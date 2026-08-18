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

With ``--selection``, level-k claims are restricted to the cited support records emitted by a
successful coloring audit of level k+1.  Selection indices are checked against both the complete
source level and the human-readable state copied into the selection file; level-(k-1) support
remains complete so the resulting certificate is independently replayable.
"""

from __future__ import annotations

import argparse
import hashlib
import re
import sys
from collections import Counter
from dataclasses import dataclass
from pathlib import Path
from typing import Iterator, TextIO


V1_HEADER = "radio-negative-certificate-v1"
V2_HEADER = "radio-negative-level-certificate-v2"
COLOR_SELECTION_HEADER = "radio-negative-color-selection-v1"
RECORD = re.compile(r"^(?:fact|root)\s+(\d+)\s+Sb\(([^)]*)\)\s*(?:#.*)?$")
COLOR_USE = re.compile(r"^use\s+(\d+)\s+Sb\(([^)]*)\)\s*(?:#.*)?$")
PART = re.compile(r"^(\d+):(\d+)$")
MAX_PARTS = 40

Part = tuple[int, int]
State = tuple[Part, ...]


@dataclass(frozen=True)
class ColorSelection:
    parent_level: int
    selected_level: int
    source_claims: int
    audited: int
    support: int
    used: int
    citation_hits: int
    states: dict[int, State]


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


def read_color_selection(path: Path) -> ColorSelection:
    records: list[tuple[int, str]] = []
    with path.open(encoding="utf-8") as source:
        for lineno, line in enumerate(source, 1):
            text = line.strip()
            if text and not text.startswith("#"):
                records.append((lineno, text))
    if not records:
        raise ValueError(f"{path}: empty color selection")
    header_lineno, header = records[0]
    if header != COLOR_SELECTION_HEADER:
        raise ValueError(
            f"{path}: line {header_lineno}: expected {COLOR_SELECTION_HEADER}"
        )

    labels = (
        "parent-level",
        "selected-level",
        "source-claims",
        "audited",
        "support",
        "used",
        "citation-hits",
    )
    values: dict[str, int] = {}
    cursor = 1
    for label in labels:
        if cursor >= len(records):
            raise ValueError(f"{path}: missing {label} record")
        lineno, text = records[cursor]
        fields = text.split()
        if len(fields) != 2 or fields[0] != label or not fields[1].isdigit():
            raise ValueError(f"{path}: line {lineno}: expected '{label} NUMBER'")
        values[label] = int(fields[1])
        cursor += 1

    states: dict[int, State] = {}
    previous_index = 0
    while cursor < len(records):
        lineno, text = records[cursor]
        match = COLOR_USE.fullmatch(text)
        if match is None:
            raise ValueError(f"{path}: line {lineno}: unknown color-selection record")
        index = int(match.group(1))
        if index <= previous_index:
            raise ValueError(
                f"{path}: line {lineno}: use indices must be strictly increasing"
            )
        states[index] = canonical_state(match.group(2), lineno=lineno)
        previous_index = index
        cursor += 1

    parent_level = values["parent-level"]
    selected_level = values["selected-level"]
    source_claims = values["source-claims"]
    audited = values["audited"]
    support = values["support"]
    used = values["used"]
    citation_hits = values["citation-hits"]
    if parent_level < 1 or selected_level != parent_level - 1:
        raise ValueError(f"{path}: selected level must be parent level minus one")
    if source_claims < 1 or audited != source_claims:
        raise ValueError(f"{path}: invalid source-claims/audited counts")
    if used > support or len(states) != used:
        raise ValueError(f"{path}: used count does not match support/use records")
    if citation_hits < used:
        raise ValueError(f"{path}: citation-hits cannot be smaller than used")
    if states and max(states) > support:
        raise ValueError(f"{path}: use index exceeds support count")
    return ColorSelection(
        parent_level=parent_level,
        selected_level=selected_level,
        source_claims=source_claims,
        audited=audited,
        support=support,
        used=used,
        citation_hits=citation_hits,
        states=states,
    )


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
    selection: ColorSelection | None = None,
    selection_path: Path | None = None,
    selection_sha256: str | None = None,
) -> None:
    if selection is not None and selection.selected_level != level:
        raise ValueError(
            f"selection targets level {selection.selected_level}, not requested level {level}"
        )
    support_level = level - 1
    support_count = support_refs = claim_count = claim_refs = 0
    source_level_count = 0
    matched_selection: set[int] = set()
    all_parts: set[Part] = set()
    root_uses: Counter[Part] = Counter()
    for record_level, state in iter_v1(source_path):
        if record_level == support_level:
            support_count += 1
            support_refs += len(state)
            all_parts.update(state)
        elif record_level == level:
            source_level_count += 1
            if selection is not None:
                expected = selection.states.get(source_level_count)
                if expected is None:
                    continue
                if state != expected:
                    raise ValueError(
                        f"selection use {source_level_count} state does not match source"
                    )
                matched_selection.add(source_level_count)
            claim_count += 1
            claim_refs += len(state)
            all_parts.update(state)
            root_uses.update(part for part in state if part != (1, 1))
    if selection is not None:
        if source_level_count != selection.support:
            raise ValueError(
                f"selection support count {selection.support} does not match "
                f"source level-{level} count {source_level_count}"
            )
        missing = selection.states.keys() - matched_selection
        if missing:
            raise ValueError(f"selection records not found in source: {sorted(missing)[:5]}")
    if not claim_count:
        if selection is not None and selection.used == 0:
            raise ValueError("color selection is empty; the top-down chain is complete")
        raise ValueError(f"input has no level-{level} claims")

    sorted_parts = sorted(all_parts, key=part_sort_key)
    ids = {part: index for index, part in enumerate(sorted_parts, 1)}
    split_hints = sorted(root_uses, key=lambda p: (-root_uses[p], ids[p]))

    out.write(f"{V2_HEADER}\n")
    out.write(f"# source {source_path.name} sha256={source_sha256}\n")
    if selection is not None:
        assert selection_path is not None and selection_sha256 is not None
        out.write(
            f"# selection {selection_path.name} sha256={selection_sha256} "
            f"parent-level={selection.parent_level} used={selection.used}\n"
        )
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
    source_level_index = 0
    for record_level, state in iter_v1(source_path):
        if record_level == level:
            source_level_index += 1
            if selection is not None and source_level_index not in selection.states:
                continue
            emit_state(out, "claim", state, ids)


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path, help="normalized v1 certificate")
    parser.add_argument("--level", type=int, required=True, help="level k to package")
    parser.add_argument(
        "--selection",
        type=Path,
        help="color selection emitted by a successful level-(k+1) audit",
    )
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
    if args.selection is not None:
        if args.selection.resolve() == args.input.resolve():
            print("certificate input and selection paths must differ", file=sys.stderr)
            return 2
        if args.output is not None and args.output.resolve() == args.selection.resolve():
            print("selection and output paths must differ", file=sys.stderr)
            return 2
    try:
        source_sha256 = input_sha256(args.input)
        selection = read_color_selection(args.selection) if args.selection is not None else None
        selection_sha256 = input_sha256(args.selection) if args.selection is not None else None
        if args.output is None:
            write_level(
                sys.stdout,
                level=args.level,
                source_path=args.input,
                source_sha256=source_sha256,
                selection=selection,
                selection_path=args.selection,
                selection_sha256=selection_sha256,
            )
        else:
            with args.output.open("w", encoding="utf-8", newline="\n") as out:
                write_level(
                    out,
                    level=args.level,
                    source_path=args.input,
                    source_sha256=source_sha256,
                    selection=selection,
                    selection_path=args.selection,
                    selection_sha256=selection_sha256,
                )
    except (OSError, UnicodeError, ValueError) as exc:
        print(f"{args.input}: {exc}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
