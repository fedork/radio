#!/usr/bin/env python3
"""Render compact Sa(193) per-level CPU totals, including visible active calls."""

from __future__ import annotations

import re
import sys


TIME_LINE_RE = re.compile(r"^    k=(\d+)")
ACTIVE_RE = re.compile(r"^    k=(\d+) \[solving\].*? elapsed (\d+)/\d+")


def compact_number(value: float) -> str:
    if value >= 1_000_000:
        return f"{value / 1_000_000:.2f}M"
    if value >= 1_000:
        return f"{value / 1_000:.1f}k"
    return f"{value:.0f}"


def parse_status(status: str) -> tuple[dict[int, float], dict[int, float]]:
    """Return completed inclusive totals and current visible elapsed time by level.

    The watchdog table uses fixed-width Unicode figure-space fields. Parse by those field widths,
    not whitespace: grouping separators inside large verdict counts are also figure spaces.
    """

    inclusive: dict[int, float] = {}
    active: dict[int, float] = {}
    in_stack = False
    in_times = False
    for line in status.splitlines():
        if line.startswith("  latest activity per level"):
            in_stack = True
            continue
        if line.startswith("  time by level"):
            in_stack = False
            in_times = True
            continue
        if in_stack:
            match = ACTIVE_RE.match(line)
            if match:
                active[int(match.group(1))] = float(match.group(2))
        if in_times:
            match = TIME_LINE_RE.match(line)
            if not match:
                continue
            level = int(match.group(1))
            # After k=<level>: verdicts width 11, inclusive width 10, self width 10, CPU% width 7.
            rest = line[match.end() :]
            inclusive_field = rest[11:21].replace("\u2007", "").strip()
            if inclusive_field.endswith("s"):
                inclusive[level] = float(inclusive_field[:-1])
    return inclusive, active


def format_level_times(status: str) -> str:
    inclusive, active = parse_status(status)
    if not inclusive:
        return ""
    tokens: list[str] = []
    for level in sorted(inclusive, reverse=True):
        augmented = inclusive[level] + active.get(level, 0.0)
        lower = inclusive.get(level - 1, 0.0) + active.get(level - 1, 0.0)
        self_time = augmented - lower
        self_text = compact_number(self_time) if self_time >= 0 else "-"
        tokens.append(f"k{level} {compact_number(augmented)}/{self_text}")
    first = " ".join(tokens[:4])
    second = " ".join(tokens[4:])
    output = f"       CPU by level i/s (+active): {first}"
    if second:
        output += f"\n                                  {second}"
    return output


def main() -> int:
    output = format_level_times(sys.stdin.read())
    if output:
        print(output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
