#!/usr/bin/env python3
"""Build one self-contained level certificate for radio_refute.

The input is a normalized ``radio-negative-certificate-v1`` file.  The output is a strict,
human-readable ``radio-negative-level-certificate-v2`` file ordered exactly as the frozen
refuter consumes it:

    part dictionary
    complete level-(k-1) support
    checked split-part hints for level k
    level-k claims to audit

Split hints name the root parts whose complete local split geometry should be prepared.  They are
ordered by decreasing occurrence count, but they are not evidence: the refuter re-derives every
split and verifies that the hint set and counts agree with the claims.
"""

from __future__ import annotations

import argparse
import hashlib
import re
import sys
from collections import Counter
from pathlib import Path
from typing import Iterator, TextIO


V1_HEADER = "radio-negative-certificate-v1"
V2_HEADER = "radio-negative-level-certificate-v2"
RECORD = re.compile(r"^(?:fact|root)\s+(\d+)\s+Sb\(([^)]*)\)\s*(?:#.*)?$")
PART = re.compile(r"^(\d+):(\d+)$")
MAX_PARTS = 40

Part = tuple[int, int]
State = tuple[Part, ...]


def canonical_part(n: int, m: int, *, lineno: int) -> Part:
    if n < 1 or m < 1:
        raise ValueError(f"line {lineno}: part sides must be positive")
    return (n, m) if n >= m else (m, n)


def canonical_state(body: str, *, lineno: int) -> State:
    if not body:
        raise ValueError(f"line {lineno}: empty state")
    parts: list[Part] = []
    for raw in body.split(","):
        match = PART.fullmatch(raw.strip())
        if match is None:
            raise ValueError(f"line {lineno}: malformed part {raw!r}")
        parts.append(canonical_part(int(match.group(1)), int(match.group(2)), lineno=lineno))
    if len(parts) > MAX_PARTS:
        raise ValueError(f"line {lineno}: more than {MAX_PARTS} parts")
    # This is the stable certificate order corresponding to radiobase's sbb order: total pair
    # mass first, then the long side.  State semantics are a multiset, so this changes no claim.
    parts.sort(key=lambda p: (p[0] * p[1], p[0], p[1]), reverse=True)
    return tuple(parts)


def input_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as raw:
        for chunk in iter(lambda: raw.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def iter_v1(path: Path) -> Iterator[tuple[int, State]]:
    header_seen = False
    with path.open(encoding="utf-8") as source:
        for lineno, line in enumerate(source, 1):
            text = line.strip()
            if not text or text.startswith("#") or text.startswith("meta "):
                continue
            if not header_seen:
                if text != V1_HEADER:
                    raise ValueError(f"line {lineno}: expected {V1_HEADER}")
                header_seen = True
                continue
            match = RECORD.fullmatch(text)
            if match is None:
                raise ValueError(f"line {lineno}: unknown certificate record")
            level = int(match.group(1))
            if level < 1:
                raise ValueError(f"line {lineno}: invalid level {level}")
            yield level, canonical_state(match.group(2), lineno=lineno)
    if not header_seen:
        raise ValueError(f"missing {V1_HEADER} header")


def part_sort_key(part: Part) -> tuple[int, int, int]:
    return part[0] * part[1], part[0], part[1]


def emit_state(out: TextIO, tag: str, state: State, ids: dict[Part, int]) -> None:
    out.write(tag)
    for part in state:
        out.write(f" {ids[part]}")
    out.write("\n")


def write_level(
    out: TextIO,
    *,
    level: int,
    source_path: Path,
    source_sha256: str,
) -> None:
    support_level = level - 1
    support_count = support_refs = claim_count = claim_refs = 0
    all_parts: set[Part] = set()
    root_uses: Counter[Part] = Counter()
    for record_level, state in iter_v1(source_path):
        if record_level == support_level:
            support_count += 1
            support_refs += len(state)
            all_parts.update(state)
        elif record_level == level:
            claim_count += 1
            claim_refs += len(state)
            all_parts.update(state)
            root_uses.update(part for part in state if part != (1, 1))
    if not claim_count:
        raise ValueError(f"input has no level-{level} claims")

    sorted_parts = sorted(all_parts, key=part_sort_key)
    ids = {part: index for index, part in enumerate(sorted_parts, 1)}
    split_hints = sorted(root_uses, key=lambda p: (-root_uses[p], ids[p]))

    out.write(f"{V2_HEADER}\n")
    out.write(f"# source {source_path.name} sha256={source_sha256}\n")
    out.write(f"level {level}\n")
    out.write(f"parts {len(sorted_parts)}\n")
    for part in sorted_parts:
        out.write(f"part {ids[part]} {part[0]}:{part[1]}\n")

    out.write(f"support {support_level} {support_count} {support_refs}\n")
    if support_count:
        for record_level, state in iter_v1(source_path):
            if record_level == support_level:
                emit_state(out, "fact", state, ids)

    out.write(f"split-hints {len(split_hints)}\n")
    for part in split_hints:
        out.write(f"split {ids[part]} uses {root_uses[part]}\n")

    out.write(f"claims {level} {claim_count} {claim_refs}\n")
    for record_level, state in iter_v1(source_path):
        if record_level == level:
            emit_state(out, "claim", state, ids)


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path, help="normalized v1 certificate")
    parser.add_argument("--level", type=int, required=True, help="level k to package")
    parser.add_argument("-o", "--output", type=Path, help="output file (default: stdout)")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    if args.level < 1:
        print("--level must be positive", file=sys.stderr)
        return 2
    if args.output is not None and args.output.resolve() == args.input.resolve():
        print("input and output paths must differ", file=sys.stderr)
        return 2
    try:
        source_sha256 = input_sha256(args.input)
        if args.output is None:
            write_level(
                sys.stdout,
                level=args.level,
                source_path=args.input,
                source_sha256=source_sha256,
            )
        else:
            with args.output.open("w", encoding="utf-8", newline="\n") as out:
                write_level(
                    out,
                    level=args.level,
                    source_path=args.input,
                    source_sha256=source_sha256,
                )
    except (OSError, UnicodeError, ValueError) as exc:
        print(f"{args.input}: {exc}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
