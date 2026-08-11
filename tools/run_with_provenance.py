#!/usr/bin/env python3
"""Run a standalone radio utility while making its stdout self-identifying.

Drivers that include radiobase.c emit provenance themselves and do not need this.  Use this for
standalone programs such as radio_verify:

    tools/run_with_provenance.py ./radio_verify certificate.txt > verify.out

The command must have been built by tools/build_radio.py, with its ``.provenance`` sidecar still
beside it.  The launcher verifies the binary hash, writes a complete build/runtime block, then
executes the command.  radiobase notices the wrapper marker and suppresses its own duplicate block
if this launcher is used there harmlessly.
"""

from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import os
from pathlib import Path
import platform
import resource
import socket
import struct
import subprocess
import sys
from typing import Iterable

import check_provenance


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def escape(value: str) -> str:
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


def physical_memory() -> int:
    try:
        pages = os.sysconf("SC_PHYS_PAGES")
        page_size = os.sysconf("SC_PAGE_SIZE")
        if pages > 0 and page_size > 0:
            return int(pages) * int(page_size)
    except (ValueError, OSError):
        pass
    if sys.platform == "darwin":
        result = subprocess.run(
            ["sysctl", "-n", "hw.memsize"], text=True, capture_output=True, check=False
        )
        if result.returncode == 0 and result.stdout.strip().isdigit():
            return int(result.stdout.strip())
    return 0


def cpu_model() -> str:
    if sys.platform == "darwin":
        result = subprocess.run(
            ["sysctl", "-n", "machdep.cpu.brand_string"], text=True,
            capture_output=True, check=False,
        )
        if result.returncode == 0 and result.stdout.strip():
            return result.stdout.strip()
    if sys.platform.startswith("linux"):
        try:
            for line in Path("/proc/cpuinfo").read_text(errors="replace").splitlines():
                if line.startswith(("model name", "Hardware")) and ":" in line:
                    return line.split(":", 1)[1].strip()
        except OSError:
            pass
    return platform.processor() or "unknown"


def emit(meta: dict[str, str], command: Iterable[str]) -> None:
    args = list(command)
    print(check_provenance.BEGIN)
    print("# artifact=command-output")
    # Copy the build fields byte-for-byte from the validated sidecar.  Escaping and argument
    # boundaries therefore cannot drift between the embedded and standalone paths.
    for key, value in meta.items():
        if key not in {"artifact"}:
            print(f"# {key}={value}")
    print(f"# run_utc={dt.datetime.now(dt.timezone.utc).strftime('%Y-%m-%dT%H:%M:%SZ')}")
    print("# runtime_argv_source=run_with_provenance.py argv")
    for i, arg in enumerate(args):
        print(f"# run_arg[{i}]={escape(arg)}")
    print(f"# run_arg_count={len(args)}")
    print("# runtime_argv_complete=yes")
    print(f"# runtime_cwd={escape(str(Path.cwd()))}")
    print(f"# runtime_host={escape(socket.gethostname())}")
    uts = os.uname()
    print(f"# runtime_os={escape(uts.sysname)}")
    print(f"# runtime_kernel_release={escape(uts.release)}")
    print(f"# runtime_kernel_version={escape(uts.version)}")
    print(f"# runtime_arch={escape(uts.machine)}")
    print(f"# runtime_logical_cpus={os.cpu_count() or 0}")
    print(f"# runtime_physical_memory_bytes={physical_memory()}")
    print(f"# runtime_pointer_bits={8 * struct.calcsize('P')}")
    print(f"# runtime_cpu_model={escape(cpu_model())}")
    for name, which in (
        ("CPU_seconds", resource.RLIMIT_CPU),
        ("address_space_bytes", resource.RLIMIT_AS),
        ("data_bytes", resource.RLIMIT_DATA),
        ("stack_bytes", resource.RLIMIT_STACK),
        ("open_files", resource.RLIMIT_NOFILE),
    ):
        soft, hard = resource.getrlimit(which)
        soft_value = "infinity" if soft == resource.RLIM_INFINITY else str(soft)
        hard_value = "infinity" if hard == resource.RLIM_INFINITY else str(hard)
        print(f"# runtime_rlimit.{name}.soft={soft_value}")
        print(f"# runtime_rlimit.{name}.hard={hard_value}")
    safe_env = (
        "LANG", "LC_ALL", "TZ", "OMP_NUM_THREADS", "RADIO_RUNNER", "RADIO_RUN_LABEL",
        "RADIO_LIMIT_WALL_SECONDS", "RADIO_LIMIT_RSS_GIB",
        "RADIO_LIMIT_PHYSICAL_FOOTPRINT_GIB", "RADIO_RUN_CONTEXT", "TWO_SIDED_ONLY",
        "TRACE_INDEX", "BENCH_K", "NODECAP", "TIMECAP", "MINIMAL_K", "TOPDOWN", "PASSES",
        "RADIO_PROBE_INIT",
    )
    for name in safe_env:
        if name in os.environ:
            print(f"# runtime_env.{name}={escape(os.environ[name])}")
    print(check_provenance.END, flush=True)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--metadata", help="override COMMAND.provenance")
    parser.add_argument("command", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    command = args.command
    if command and command[0] == "--":
        command = command[1:]
    if not command:
        parser.error("no command supplied")

    executable = Path(command[0])
    if not executable.is_absolute():
        if "/" in command[0]:
            executable = (Path.cwd() / executable).resolve()
        else:
            from shutil import which
            found = which(command[0])
            if found is None:
                parser.error(f"command not found: {command[0]}")
            executable = Path(found).resolve()
    metadata = Path(args.metadata) if args.metadata else Path(str(executable) + ".provenance")
    try:
        with metadata.open(encoding="utf-8", errors="replace") as f:
            block = check_provenance.read_block(f)
    except OSError as exc:
        parser.error(f"cannot read build provenance {metadata}: {exc}")
    meta, errors = check_provenance.fields(block)
    errors.extend(check_provenance.validate(meta, allow_incomplete=False))
    if meta.get("artifact") != "build-sidecar":
        errors.append("metadata is not a build sidecar")
    actual_hash = sha256(executable)
    if meta.get("binary_sha256") != actual_hash:
        errors.append("binary SHA-256 does not match its build sidecar")
    if errors:
        for error in errors:
            print(f"run_with_provenance.py: {error}", file=sys.stderr)
        return 65

    emit(meta, command)
    env = os.environ.copy()
    env["RADIO_PROVENANCE_WRAPPER_EMITTED"] = "1"
    os.execvpe(command[0], command, env)
    return 127


if __name__ == "__main__":
    sys.exit(main())
