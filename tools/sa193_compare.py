#!/usr/bin/env python3
"""Compare the slowest exact calls from the less-advanced of two live Sa(193) logs.

Per-verdict `took` is inclusive CPU time. Historical logs cannot assign exclusive time to an
individual verdict, so report only measurements the exact-state join actually supports.
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


@dataclasses.dataclass
class Verdict:
    state: str
    level: int
    outcome: str
    seconds: float
    splits: int
    line_no: int
    estimated_self: float | None = None

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
    top_slow: list[Verdict] = dataclasses.field(default_factory=list)

    @property
    def progress_key(self) -> tuple[int, int, int]:
        # Root completions are the real progress. K=9 and total verdict counts are only tie-breakers
        # while two runs sit inside the same root count.
        return len(self.top_done), self.k9_verdicts, self.verdicts


@dataclasses.dataclass
class SelfEstimator:
    """Estimate a call's self time from the post-order verdict stream.

    Between consecutive level-k verdicts, level-(k-1) `took` values are the completed direct-child
    work in the usual depth-first case. MAYBE calls and cache effects make this an estimate, and the
    first verdict at each level has no left boundary, so leave that one unknown.
    """

    inclusive_by_level: dict[int, float] = dataclasses.field(default_factory=dict)
    lower_at_previous: dict[int, float] = dataclasses.field(default_factory=dict)

    def observe(self, verdict: Verdict) -> Verdict:
        lower_total = self.inclusive_by_level.get(verdict.level - 1, 0.0)
        previous = self.lower_at_previous.get(verdict.level)
        estimated_self = None if previous is None else verdict.seconds - (lower_total - previous)
        self.lower_at_previous[verdict.level] = lower_total
        self.inclusive_by_level[verdict.level] = (
            self.inclusive_by_level.get(verdict.level, 0.0) + verdict.seconds
        )
        verdict.estimated_self = estimated_self
        return verdict


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
    estimator = SelfEstimator()
    with path.open("r", encoding="utf-8", errors="replace") as source:
        for line_no, line in enumerate(source, 1):
            verdict = parse_verdict(line, line_no)
            if verdict is None:
                continue
            verdict = estimator.observe(verdict)
            summary.verdicts += 1
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
    estimator = SelfEstimator()
    with path.open("r", encoding="utf-8", errors="replace") as source:
        for line_no, line in enumerate(source, 1):
            verdict = parse_verdict(line, line_no)
            if verdict is None:
                continue
            verdict = estimator.observe(verdict)
            if verdict.key in keys:
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


def timing_pair(verdict: Verdict) -> str:
    self_time = "-" if verdict.estimated_self is None else compact_number(verdict.estimated_self)
    return f"{compact_number(verdict.seconds)}/{self_time}"


def ratio(value: float | None, peer_value: float | None) -> str:
    if value is None or peer_value is None or value < 0 or peer_value <= 0:
        return "-"
    return f"{value / peer_value:.2f}x"


def format_summary(a: Summary, b: Summary) -> str:
    behind, peer = sorted((a, b), key=lambda item: item.progress_key)
    matches = find_matches(peer.path, {verdict.key for verdict in behind.top_slow})
    now = dt.datetime.now(dt.timezone.utc).replace(microsecond=0).isoformat()
    lines = [
        f"compare {now}  behind={behind.label} "
        f"({len(behind.top_done)}/16 roots, {behind.verdicts} verdicts)",
        f"slow calls (CPU incl/~self) from {behind.label}"
        f"          {behind.label:>13} {peer.label:>13}    i×/~s×",
    ]
    for verdict in behind.top_slow:
        other = matches.get(verdict.key)
        state = short_state(verdict.state)
        if other is None:
            comparison = f"{timing_pair(verdict):>13} {'-':>13} {'-':>10}"
        else:
            ratios = (
                f"{ratio(verdict.seconds, other.seconds)}/"
                f"{ratio(verdict.estimated_self, other.estimated_self)}"
            )
            comparison = (
                f"{timing_pair(verdict):>13} "
                f"{timing_pair(other):>13} {ratios:>10}"
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
