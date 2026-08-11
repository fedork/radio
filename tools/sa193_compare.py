#!/usr/bin/env python3
"""Build a compact, exact comparison of two live Sa(193) raw logs.

Per-verdict `took` is inclusive CPU time.  The historical logs do not contain call boundaries
needed to assign exclusive time to individual verdicts, but aggregate self time by level is exact
when non-negative: inclusive(k) - inclusive(k-1).  Keep those two measurements separate rather
than manufacturing per-state precision.
"""

from __future__ import annotations

import argparse
import dataclasses
import datetime as dt
import heapq
import re
from pathlib import Path
from typing import Iterable


VERDICT_RE = re.compile(
    r"^(can solve|can't solve) "
    r"(?:size=\d+/\d+ )?"
    r"(Sb\(.*?\)\[\d+,\d+\]) in (\d+)"
    r".*? took (\d+(?:\.\d+)?) totalsplits=(\d+)"
)
MASS_RE = re.compile(r"\[(\d+),(\d+)\]$")
FIRST_PART_RE = re.compile(r"^Sb\((\d+):")


@dataclasses.dataclass(frozen=True)
class Verdict:
    state: str
    level: int
    outcome: str
    seconds: float
    splits: int
    line_no: int

    @property
    def key(self) -> tuple[str, int]:
        return self.state, self.level


@dataclasses.dataclass
class Summary:
    label: str
    path: Path
    verdicts: int = 0
    k9_verdicts: int = 0
    top_done: set[int] = dataclasses.field(default_factory=set)
    inclusive: dict[int, float] = dataclasses.field(default_factory=dict)
    counts: dict[int, int] = dataclasses.field(default_factory=dict)
    top_slow: list[Verdict] = dataclasses.field(default_factory=list)

    @property
    def progress_key(self) -> tuple[int, int, int]:
        # Root completions are the real progress. K=9 and total verdict counts are only tie-breakers
        # while two runs sit inside the same root count.
        return len(self.top_done), self.k9_verdicts, self.verdicts

    def self_seconds(self, level: int) -> float | None:
        if level not in self.inclusive:
            return None
        value = self.inclusive[level] - self.inclusive.get(level - 1, 0.0)
        # An unfinished ancestor can leave lower-level time without its enclosing `took` line.
        return value if value >= 0 else None


def parse_verdict(line: str, line_no: int) -> Verdict | None:
    match = VERDICT_RE.match(line)
    if not match:
        return None
    outcome, state, level, seconds, splits = match.groups()
    return Verdict(
        state=state,
        level=int(level),
        outcome="yes" if outcome == "can solve" else "no",
        seconds=float(seconds),
        splits=int(splits),
        line_no=line_no,
    )


def scan(path: Path, label: str, top: int) -> Summary:
    summary = Summary(label=label, path=path)
    heap: list[tuple[float, int, Verdict]] = []
    with path.open("r", encoding="utf-8", errors="replace") as source:
        for line_no, line in enumerate(source, 1):
            verdict = parse_verdict(line, line_no)
            if verdict is None:
                continue
            summary.verdicts += 1
            summary.counts[verdict.level] = summary.counts.get(verdict.level, 0) + 1
            summary.inclusive[verdict.level] = (
                summary.inclusive.get(verdict.level, 0.0) + verdict.seconds
            )
            if verdict.level == 9:
                summary.k9_verdicts += 1
                mass = MASS_RE.search(verdict.state)
                first = FIRST_PART_RE.match(verdict.state)
                if mass and first and int(mass.group(2)) == 193:
                    n1 = int(first.group(1))
                    if 97 <= n1 <= 112:
                        summary.top_done.add(n1)

            item = (verdict.seconds, line_no, verdict)
            if len(heap) < top:
                heapq.heappush(heap, item)
            elif item[:2] > heap[0][:2]:
                heapq.heapreplace(heap, item)

    summary.top_slow = [item[2] for item in sorted(heap, reverse=True)]
    return summary


def find_matches(path: Path, keys: set[tuple[str, int]]) -> dict[tuple[str, int], Verdict]:
    matches: dict[tuple[str, int], Verdict] = {}
    if not keys:
        return matches
    with path.open("r", encoding="utf-8", errors="replace") as source:
        for line_no, line in enumerate(source, 1):
            verdict = parse_verdict(line, line_no)
            if verdict is not None and verdict.key in keys:
                matches[verdict.key] = verdict
    return matches


def compact_number(value: float) -> str:
    if value >= 1_000_000:
        return f"{value / 1_000_000:.2f}m"
    if value >= 1_000:
        return f"{value / 1_000:.1f}k"
    if value >= 10:
        return f"{value:.0f}"
    return f"{value:.3f}".rstrip("0").rstrip(".")


def short_state(state: str, width: int = 70) -> str:
    if len(state) <= width:
        return state
    return state[: width - 1] + "…"


def format_summary(a: Summary, b: Summary) -> str:
    behind, peer = sorted((a, b), key=lambda item: item.progress_key)
    matches = find_matches(peer.path, {verdict.key for verdict in behind.top_slow})
    now = dt.datetime.now(dt.timezone.utc).replace(microsecond=0).isoformat()
    lines = [
        f"compare {now}  behind={behind.label} "
        f"({len(behind.top_done)}/16 roots, {behind.verdicts} verdicts)",
        "call=inclusive CPU; self=aggregate CPU by level",
    ]

    levels = sorted(
        {verdict.level for verdict in behind.top_slow}
        | set(behind.inclusive)
        | set(peer.inclusive),
        reverse=True,
    )
    # Keep the table small and focused on levels that have measurable self time or selected calls.
    selected_levels = {verdict.level for verdict in behind.top_slow}
    levels = [
        level
        for level in levels
        if level in selected_levels
        or behind.self_seconds(level) is not None
        or peer.self_seconds(level) is not None
    ][:8]
    lines.append(f"level       {behind.label:>17}       {peer.label:>17}")
    lines.append("          inclusive / self      inclusive / self")
    for level in levels:
        b_inclusive = compact_number(behind.inclusive.get(level, 0.0))
        p_inclusive = compact_number(peer.inclusive.get(level, 0.0))
        b_self_value = behind.self_seconds(level)
        p_self_value = peer.self_seconds(level)
        b_self = "-" if b_self_value is None else compact_number(b_self_value)
        p_self = "-" if p_self_value is None else compact_number(p_self_value)
        lines.append(
            f"k={level:<2} {b_inclusive:>10} / {b_self:<7} "
            f"{p_inclusive:>10} / {p_self:<7}"
        )

    lines.append("")
    lines.append(
        f"slow calls selected from {behind.label}"
        f"                    {behind.label:>8} {peer.label:>8}   ratio"
    )
    for verdict in behind.top_slow:
        other = matches.get(verdict.key)
        state = short_state(verdict.state)
        if other is None:
            comparison = f"{compact_number(verdict.seconds):>8} {'-':>8} {'-':>7}"
        else:
            ratio = "-" if other.seconds == 0 else f"{verdict.seconds / other.seconds:.2f}x"
            comparison = (
                f"{compact_number(verdict.seconds):>8} "
                f"{compact_number(other.seconds):>8} {ratio:>7}"
            )
            if other.outcome != verdict.outcome:
                comparison += "  OUTCOME-MISMATCH"
        lines.append(f"k={verdict.level} {state:<70} {comparison}")
    return "\n".join(lines)


def main(argv: Iterable[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path)
    parser.add_argument("peer_log", type=Path)
    parser.add_argument("--label", default="run")
    parser.add_argument("--peer-label", default="peer")
    parser.add_argument("--top", type=int, default=6)
    args = parser.parse_args(argv)
    if args.top < 1 or args.top > 20:
        parser.error("--top must be between 1 and 20")
    for path in (args.log, args.peer_log):
        if not path.is_file():
            parser.error(f"no such log: {path}")

    first = scan(args.log, args.label, args.top)
    second = scan(args.peer_log, args.peer_label, args.top)
    print(format_summary(first, second))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
