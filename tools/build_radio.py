#!/usr/bin/env python3
"""Build a radio executable with self-identifying provenance.

Usage mirrors the compiler after the script name:

    tools/build_radio.py -O3 -DMAX_K=10 -DMAX_N=193 radio_sa193.c -o radio_sa193

The selected compiler is ``$CC`` (default: clang).  The script asks the compiler for the local
dependency set, hashes every source dependency, injects a generated metadata header, and writes a
``<binary>.provenance`` sidecar after a successful link.  ``radiobase.c`` prints the embedded copy
at process start, so the raw stdout remains self-contained if the binary or sidecar is later lost.

Only metadata that affects reproducibility is captured.  In particular, this never serializes the
whole environment: build machines commonly carry cloud and Git credentials there.
"""

from __future__ import annotations

import datetime as dt
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import socket
import subprocess
import sys
import tempfile
from typing import Iterable, Sequence


ROOT = Path(__file__).resolve().parent.parent
SCHEMA = "radio-provenance-v1"
BUILD_ENV_ALLOWLIST = (
    "CC", "PATH", "SDKROOT", "DEVELOPER_DIR", "MACOSX_DEPLOYMENT_TARGET", "CPATH",
    "C_INCLUDE_PATH", "CPLUS_INCLUDE_PATH", "OBJC_INCLUDE_PATH", "LIBRARY_PATH",
    "LD_LIBRARY_PATH", "DYLD_LIBRARY_PATH", "SOURCE_DATE_EPOCH", "ZERO_AR_DATE", "LANG",
    "LC_ALL", "TZ", "RADIO_SOURCE_COMMIT",
)


class BuildError(RuntimeError):
    pass


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def run(cmd: Sequence[str], *, cwd: Path, check: bool = True) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        list(cmd), cwd=cwd, check=check, text=True, stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )


def git(args: Sequence[str], *, check: bool = True) -> subprocess.CompletedProcess[str]:
    try:
        return run(["git", "-C", str(ROOT), *args], cwd=ROOT, check=check)
    except FileNotFoundError:
        if check:
            raise BuildError("git is unavailable")
        return subprocess.CompletedProcess(["git", *args], 127, "", "git is unavailable")


def output_path(args: Sequence[str], cwd: Path) -> Path:
    out: str | None = None
    skip = False
    for i, arg in enumerate(args):
        if skip:
            skip = False
            continue
        if arg == "-o":
            if i + 1 >= len(args):
                raise BuildError("-o has no output path")
            out = args[i + 1]
            skip = True
        elif arg.startswith("-o") and len(arg) > 2:
            out = arg[2:]
    if out is None:
        raise BuildError("an explicit -o OUTPUT is required so the provenance sidecar has a name")
    path = Path(out)
    return path if path.is_absolute() else cwd / path


def args_without_output(args: Sequence[str]) -> list[str]:
    result: list[str] = []
    skip = False
    for i, arg in enumerate(args):
        if skip:
            skip = False
            continue
        if arg == "-o":
            if i + 1 >= len(args):
                raise BuildError("-o has no output path")
            skip = True
            continue
        if arg.startswith("-o") and len(arg) > 2:
            continue
        result.append(arg)
    return result


def compiler_command() -> list[str]:
    words = shlex.split(os.environ.get("CC", "clang"))
    if not words:
        raise BuildError("CC expands to an empty command")
    resolved = shutil.which(words[0])
    if resolved is None:
        raise BuildError(f"compiler not found: {words[0]}")
    words[0] = str(Path(resolved).resolve())
    return words


def make_dependencies(cc: Sequence[str], args: Sequence[str], cwd: Path) -> list[Path]:
    # -MM keeps system headers out: libc's exact implementation is represented by compiler version
    # plus build/runtime OS, while the repository sources are content-addressed individually.
    cmd = [*cc, *args_without_output(args), "-MM", "-MT", "radio-provenance-target"]
    result = run(cmd, cwd=cwd, check=False)
    if result.returncode:
        raise BuildError(
            "compiler dependency scan failed; refusing an allegedly complete build:\n"
            + result.stderr.strip()
        )
    logical = result.stdout.replace("\\\n", " ")
    if ":" not in logical:
        raise BuildError(f"could not parse compiler dependency output: {result.stdout!r}")
    dep_text = logical.split(":", 1)[1]
    try:
        names = shlex.split(dep_text)
    except ValueError as exc:
        raise BuildError(f"could not parse compiler dependency list: {exc}") from exc
    deps: dict[str, Path] = {}
    for name in names:
        path = Path(name)
        path = path if path.is_absolute() else cwd / path
        path = path.resolve()
        if not path.is_file():
            raise BuildError(f"compiler dependency does not exist: {path}")
        deps[str(path)] = path
    if not deps:
        raise BuildError("compiler dependency scan returned no source files")
    return [deps[k] for k in sorted(deps)]


def display_path(path: Path) -> str:
    try:
        return path.relative_to(ROOT).as_posix()
    except ValueError:
        return str(path)


def is_repository_checkout() -> bool:
    top = git(["rev-parse", "--show-toplevel"], check=False)
    if top.returncode:
        return False
    try:
        return Path(top.stdout.strip()).resolve() == ROOT
    except OSError:
        return False


def commit_identity() -> tuple[str, str]:
    supplied = os.environ.get("RADIO_SOURCE_COMMIT", "").strip()
    if supplied:
        if not re.fullmatch(r"(?:[0-9a-fA-F]{40}|[0-9a-fA-F]{64})", supplied):
            raise BuildError("RADIO_SOURCE_COMMIT must be a full 40- or 64-digit commit id")
        supplied = supplied.lower()
    if is_repository_checkout():
        result = git(["rev-parse", "HEAD"], check=False)
        head = result.stdout.strip().lower() if result.returncode == 0 else ""
        if not re.fullmatch(r"(?:[0-9a-f]{40}|[0-9a-f]{64})", head):
            raise BuildError("could not resolve a full commit id for this checkout")
        if supplied and supplied != head:
            raise BuildError(
                f"RADIO_SOURCE_COMMIT={supplied} conflicts with checkout HEAD={head}"
            )
        return head, "git-checkout"
    if supplied:
        return supplied, "supplied-source-archive"
    return "unknown", "unavailable"


def dirty_state(deps: Sequence[Path]) -> tuple[str, str]:
    if not is_repository_checkout():
        return "source-archive", "source-archive"
    worktree = git(["status", "--porcelain", "--untracked-files=all"], check=False)
    worktree_dirty = "unknown" if worktree.returncode else ("yes" if worktree.stdout else "no")

    rels: list[str] = []
    for path in deps:
        try:
            rels.append(path.relative_to(ROOT).as_posix())
        except ValueError:
            return "external-source", worktree_dirty
    for rel in rels:
        if git(["ls-files", "--error-unmatch", "--", rel], check=False).returncode:
            return "yes", worktree_dirty
    unstaged = git(["diff", "--quiet", "HEAD", "--", *rels], check=False)
    staged = git(["diff", "--cached", "--quiet", "HEAD", "--", *rels], check=False)
    return ("yes" if unstaged.returncode or staged.returncode else "no"), worktree_dirty


def c_string(value: str) -> str:
    """A byte-exact C string literal, independent of source-file encoding."""
    out = ['"']
    for byte in value.encode("utf-8"):
        if byte == 0x22:
            out.append(r'\"')
        elif byte == 0x5C:
            out.append(r"\\")
        elif 0x20 <= byte <= 0x7E:
            out.append(chr(byte))
        else:
            out.append(f"\\{byte:03o}")
    out.append('"')
    return "".join(out)


def escaped_value(value: str) -> str:
    """Match radiobase.c's line-oriented value encoding."""
    out: list[str] = []
    for byte in value.encode("utf-8"):
        if byte == 0x5C:
            out.append(r"\\")
        elif byte == 0x0A:
            out.append(r"\n")
        elif byte == 0x0D:
            out.append(r"\r")
        elif byte == 0x09:
            out.append(r"\t")
        elif 0x20 <= byte <= 0x7E:
            out.append(chr(byte))
        else:
            out.append(f"\\x{byte:02x}")
    return "".join(out)


def define(name: str, value: str) -> str:
    return f"#define {name} {c_string(value)}\n"


def string_array(name: str, values: Iterable[str]) -> str:
    vals = list(values)
    body = ",\n    ".join(c_string(v) for v in vals) if vals else "NULL"
    return f"static const char *const {name}[{max(1, len(vals))}] = {{\n    {body}\n}};\n"


def generated_header(meta: dict[str, object]) -> str:
    sources = meta["sources"]
    assert isinstance(sources, list)
    compile_argv = meta["compile_argv"]
    assert isinstance(compile_argv, list)
    build_env = meta["build_env"]
    assert isinstance(build_env, list)
    return "".join([
        "/* generated by tools/build_radio.py; do not retain or edit */\n",
        "#define RADIO_BUILD_PROVENANCE_AVAILABLE 1\n",
        f"#define RADIO_BUILD_PROVENANCE_COMPLETE {1 if meta['provenance_complete'] == 'yes' else 0}\n",
        define("RADIO_BUILD_ID", str(meta["build_id"])),
        define("RADIO_GIT_COMMIT", str(meta["git_commit"])),
        define("RADIO_GIT_IDENTITY_SOURCE", str(meta["git_identity_source"])),
        define("RADIO_GIT_SOURCE_DIRTY", str(meta["git_source_dirty"])),
        define("RADIO_GIT_WORKTREE_DIRTY", str(meta["git_worktree_dirty"])),
        define("RADIO_BUILD_UTC", str(meta["build_utc"])),
        define("RADIO_BUILD_HOST", str(meta["build_host"])),
        define("RADIO_BUILD_UNAME", str(meta["build_uname"])),
        define("RADIO_BUILD_CWD", str(meta["build_cwd"])),
        define("RADIO_COMPILER_VERSION", str(meta["compiler_version"])),
        define("RADIO_COMPILER_SHA256", str(meta["compiler_executable_sha256"])),
        define("RADIO_BUILD_TOOL_SHA256", str(meta["build_tool_sha256"])),
        define("RADIO_PROVENANCE_INJECTION", str(meta["provenance_injection"])),
        f"#define RADIO_BUILD_ARGC {len(compile_argv)}\n",
        string_array("radio_provenance_build_argv", (str(x) for x in compile_argv)),
        f"#define RADIO_BUILD_ENV_COUNT {len(build_env)}\n",
        string_array("radio_provenance_build_env_names", (str(x["name"]) for x in build_env)),
        string_array("radio_provenance_build_env_values", (str(x["value"]) for x in build_env)),
        f"#define RADIO_SOURCE_COUNT {len(sources)}\n",
        string_array("radio_provenance_source_paths", (str(x["path"]) for x in sources)),
        string_array("radio_provenance_source_sha256", (str(x["sha256"]) for x in sources)),
    ])


def sidecar(meta: dict[str, object], binary_hash: str) -> str:
    lines = [f"# {SCHEMA} begin"]
    scalar_keys = [
        "artifact", "provenance_complete", "build_id", "git_commit", "git_identity_source",
        "git_source_dirty", "git_worktree_dirty", "build_utc", "build_host", "build_uname",
        "build_cwd", "compiler_version", "compiler_executable_sha256", "build_tool_sha256",
        "provenance_injection",
    ]
    for key in scalar_keys:
        lines.append(f"# {key}={escaped_value(str(meta[key]))}")
    compile_argv = meta["compile_argv"]
    assert isinstance(compile_argv, list)
    lines.append(f"# compile_arg_count={len(compile_argv)}")
    lines.extend(f"# compile_arg[{i}]={escaped_value(str(arg))}"
                 for i, arg in enumerate(compile_argv))
    build_env = meta["build_env"]
    assert isinstance(build_env, list)
    lines.append(f"# build_env_count={len(build_env)}")
    for i, item in enumerate(build_env):
        lines.append(f"# build_env[{i}].name={escaped_value(str(item['name']))}")
        lines.append(f"# build_env[{i}].value={escaped_value(str(item['value']))}")
    sources = meta["sources"]
    assert isinstance(sources, list)
    lines.append(f"# source_count={len(sources)}")
    for i, source in enumerate(sources):
        lines.append(f"# source[{i}].path={escaped_value(str(source['path']))}")
        lines.append(f"# source[{i}].sha256={source['sha256']}")
    lines.append(f"# binary_sha256={binary_hash}")
    lines.append(f"# {SCHEMA} end")
    return "\n".join(lines) + "\n"


def main(argv: Sequence[str]) -> int:
    if not argv or argv[0] in {"-h", "--help"}:
        print(__doc__.strip())
        return 0 if argv else 64

    cwd = Path.cwd().resolve()
    args = list(argv)
    out = output_path(args, cwd)
    cc = compiler_command()
    compiler = run([*cc, "--version"], cwd=cwd, check=False)
    if compiler.returncode:
        raise BuildError(f"could not identify compiler: {compiler.stderr.strip()}")
    compiler_version = compiler.stdout.strip() or compiler.stderr.strip()
    compiler_hash = sha256(Path(cc[0]))
    deps = make_dependencies(cc, args, cwd)
    sources = [{"path": display_path(path), "sha256": sha256(path)} for path in deps]
    commit, identity_source = commit_identity()
    source_dirty, worktree_dirty = dirty_state(deps)
    complete = commit != "unknown"
    compile_argv = [*cc, *args]
    build_env = [{"name": name, "value": os.environ[name]}
                 for name in BUILD_ENV_ALLOWLIST if name in os.environ]
    build_tool_hash = sha256(Path(__file__).resolve())
    provenance_injection = "tools/build_radio.py forced-include schema 1"
    identity = {
        "schema": SCHEMA,
        "git_commit": commit,
        "sources": sources,
        "compiler_version": compiler_version,
        "compiler_executable_sha256": compiler_hash,
        "compile_argv": compile_argv,
        "build_env": build_env,
        "provenance_injection": provenance_injection,
        "build_tool_sha256": build_tool_hash,
    }
    build_id = hashlib.sha256(
        json.dumps(identity, sort_keys=True, separators=(",", ":")).encode("utf-8")
    ).hexdigest()
    meta: dict[str, object] = {
        "artifact": "build-sidecar",
        "provenance_complete": "yes" if complete else "no",
        "build_id": build_id,
        "git_commit": commit,
        "git_identity_source": identity_source,
        "git_source_dirty": source_dirty,
        "git_worktree_dirty": worktree_dirty,
        "build_utc": dt.datetime.now(dt.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "build_host": socket.gethostname(),
        "build_uname": " ".join(os.uname()),
        "build_cwd": str(cwd),
        "compiler_version": compiler_version,
        "compiler_executable_sha256": compiler_hash,
        "build_tool_sha256": build_tool_hash,
        "provenance_injection": provenance_injection,
        "compile_argv": compile_argv,
        "build_env": build_env,
        "sources": sources,
    }

    out.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="radio-build-provenance-") as tmp:
        header = Path(tmp) / "radio_build_provenance.h"
        header.write_text(generated_header(meta), encoding="utf-8")
        completed = subprocess.run([*cc, "-include", str(header), *args], cwd=cwd)
        if completed.returncode:
            return completed.returncode

    binary_hash = sha256(out)
    sidecar_path = Path(str(out) + ".provenance")
    tmp_sidecar = Path(str(sidecar_path) + ".tmp")
    tmp_sidecar.write_text(sidecar(meta, binary_hash), encoding="utf-8")
    os.replace(tmp_sidecar, sidecar_path)
    print(
        f"built {out} build_id={build_id} provenance_complete={'yes' if complete else 'no'}\n"
        f"metadata {sidecar_path}",
        file=sys.stderr,
    )
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main(sys.argv[1:]))
    except BuildError as exc:
        print(f"build_radio.py: {exc}", file=sys.stderr)
        sys.exit(2)
