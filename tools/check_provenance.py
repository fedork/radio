#!/usr/bin/env python3
"""Validate or extract the provenance block in a solver log/build sidecar.

    tools/check_provenance.py out.txt [...]
    tools/check_provenance.py --allow-incomplete old-or-direct-build.txt
    tools/check_provenance.py --extract out.txt

The default is intentionally strict: an incomplete block fails just like a missing one.  This is
the gate to run before promoting a raw output to durable evidence.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path
from typing import TextIO


BEGIN = "# radio-provenance-v1 begin"
END = "# radio-provenance-v1 end"
HEX256 = re.compile(r"[0-9a-f]{64}")
COMMIT = re.compile(r"(?:[0-9a-f]{40}|[0-9a-f]{64})")
MAX_PREFIX_BYTES = 1024 * 1024


def read_block(stream: TextIO) -> list[str]:
    block: list[str] = []
    active = False
    scanned = 0
    for raw in stream:
        scanned += len(raw.encode("utf-8", errors="replace"))
        line = raw.rstrip("\n")
        if not active:
            if line == BEGIN:
                active = True
                block.append(line)
            elif scanned >= MAX_PREFIX_BYTES:
                break
            continue
        block.append(line)
        if line == END:
            return block
        if scanned >= MAX_PREFIX_BYTES:
            break
    return block


def fields(block: list[str]) -> tuple[dict[str, str], list[str]]:
    result: dict[str, str] = {}
    errors: list[str] = []
    if not block:
        return result, ["missing radio-provenance-v1 block"]
    if block[0] != BEGIN:
        errors.append("malformed provenance beginning")
    if block[-1] != END:
        errors.append("unterminated provenance block")
    for line in block[1:-1]:
        if not line.startswith("# ") or "=" not in line:
            errors.append(f"malformed provenance line: {line!r}")
            continue
        key, value = line[2:].split("=", 1)
        if key in result:
            errors.append(f"duplicate field {key}")
        result[key] = value
    return result, errors


def integer(value: str | None, key: str, errors: list[str]) -> int:
    if value is None or not value.isdigit():
        errors.append(f"{key} is missing or not a non-negative integer")
        return 0
    return int(value)


def validate(meta: dict[str, str], *, allow_incomplete: bool) -> list[str]:
    errors: list[str] = []
    artifact = meta.get("artifact")
    if artifact not in {"solver-output", "command-output", "build-sidecar"}:
        errors.append(f"unknown or missing artifact type: {artifact!r}")
    complete = meta.get("provenance_complete")
    if complete not in {"yes", "no"}:
        errors.append("provenance_complete must be yes or no")
    elif complete != "yes" and not allow_incomplete:
        errors.append("provenance is explicitly incomplete")

    build_id = meta.get("build_id", "")
    if complete == "yes" and not HEX256.fullmatch(build_id):
        errors.append("complete provenance needs a 64-hex-digit build_id")
    commit = meta.get("git_commit", "")
    if complete == "yes" and not COMMIT.fullmatch(commit):
        errors.append("complete provenance needs a hexadecimal git_commit")

    for key in ("build_utc", "build_host", "build_uname", "build_cwd", "compiler_version",
                "compiler_executable_sha256", "build_tool_sha256", "provenance_injection",
                "git_identity_source", "git_source_dirty", "git_worktree_dirty"):
        if not meta.get(key) or (complete == "yes" and meta[key] == "unknown"):
            errors.append(f"complete provenance needs {key}")
    for key in ("compiler_executable_sha256", "build_tool_sha256"):
        if complete == "yes" and not HEX256.fullmatch(meta.get(key, "")):
            errors.append(f"complete provenance needs a 64-hex-digit {key}")

    argc = integer(meta.get("compile_arg_count"), "compile_arg_count", errors)
    if complete == "yes" and argc == 0:
        errors.append("complete provenance has no compiler arguments")
    for i in range(argc):
        if f"compile_arg[{i}]" not in meta:
            errors.append(f"missing compile_arg[{i}]")

    nenv = integer(meta.get("build_env_count"), "build_env_count", errors)
    for i in range(nenv):
        if f"build_env[{i}].name" not in meta:
            errors.append(f"missing build_env[{i}].name")
        if f"build_env[{i}].value" not in meta:
            errors.append(f"missing build_env[{i}].value")

    nsource = integer(meta.get("source_count"), "source_count", errors)
    if complete == "yes" and nsource == 0:
        errors.append("complete provenance has no source manifest")
    for i in range(nsource):
        path = meta.get(f"source[{i}].path")
        digest = meta.get(f"source[{i}].sha256", "")
        if path is None:
            errors.append(f"missing source[{i}].path")
        if not HEX256.fullmatch(digest):
            errors.append(f"source[{i}].sha256 is missing or malformed")

    if artifact == "build-sidecar":
        if not HEX256.fullmatch(meta.get("binary_sha256", "")):
            errors.append("build sidecar needs binary_sha256")
    elif artifact in {"solver-output", "command-output"}:
        for key in ("run_utc", "runtime_argv_source", "runtime_argv_complete", "runtime_cwd",
                    "runtime_host", "runtime_os",
                    "runtime_kernel_release", "runtime_kernel_version", "runtime_arch",
                    "runtime_logical_cpus", "runtime_physical_memory_bytes", "runtime_pointer_bits",
                    "runtime_cpu_model", "runtime_rlimit.CPU_seconds.soft",
                    "runtime_rlimit.address_space_bytes.soft"):
            if key not in meta:
                errors.append(f"solver output is missing {key}")
        if artifact == "solver-output":
            for key in ("define.MAX_K", "define.MAX_N"):
                if key not in meta:
                    errors.append(f"solver output is missing {key}")
        run_argc = integer(meta.get("run_arg_count"), "run_arg_count", errors)
        for i in range(run_argc):
            if f"run_arg[{i}]" not in meta:
                errors.append(f"missing run_arg[{i}]")
        if complete == "yes" and meta.get("runtime_argv_complete") != "yes":
            errors.append("complete solver provenance did not capture runtime argv")
    return errors


def open_input(name: str) -> tuple[TextIO, bool]:
    if name == "-":
        return sys.stdin, False
    return Path(name).open(encoding="utf-8", errors="replace"), True


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--allow-incomplete", action="store_true")
    parser.add_argument("--extract", action="store_true",
                        help="print the first provenance block instead of a status line")
    parser.add_argument("files", nargs="+", help="solver logs/build sidecars; - means stdin")
    args = parser.parse_args()
    failed = 0
    for name in args.files:
        stream, close = open_input(name)
        try:
            block = read_block(stream)
        finally:
            if close:
                stream.close()
        meta, errors = fields(block)
        if not errors:
            errors.extend(validate(meta, allow_incomplete=args.allow_incomplete))
        if args.extract:
            if not errors:
                print("\n".join(block))
        elif errors:
            print(f"{name}: INVALID")
            for error in errors:
                print(f"  {error}")
        else:
            print(f"{name}: provenance OK build_id={meta['build_id']} commit={meta['git_commit']}")
        failed += bool(errors)
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
