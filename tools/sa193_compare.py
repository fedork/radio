#!/usr/bin/env python3
"""Compare the slowest exact calls from the less-advanced of two live Sa(193) logs.

Per-verdict `took` covers only the activation that finally returned a verdict.  A reset in the
`elapsed` field of earlier progress lines exposes abandoned activations of the same exact state.
Their last observed elapsed values give a conservative attempt-sum floor; historical logs do not
record the exact point at which a MAYBE returned. New deterministic-budget verdicts carry their
exact work and calibration rate, while historical verdicts fall back to process-CPU seconds.
Exclusive CPU time remains an estimate.
"""

from __future__ import annotations

import argparse
import dataclasses
import datetime as dt
import heapq
import math
import re
from pathlib import Path
from typing import Iterable, Iterator


VERDICT_RE = re.compile(
    r"^(can solve|can't solve) "
    r"(?:size=\d+/\d+ )?"
    r"(Sb\(.*?\)\[\d+,\d+\]) in (\d+)"
    r".*? took (\d+(?:\.\d+)?) totalsplits=(\d+)"
)
PROGRESS_RE = re.compile(
    r"^still solving in (\d+) .*?"
    r"(Sb\(.*?\)\[\d+,\d+\]) trying .*? elapsed (\d+)/(\d+)"
)
WORK_RE = re.compile(r" work=(\d+) rate=(\d+)")
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
    effort_seconds: float | None = None
    estimated_self: float | None = None
    prior_attempt_floor: float = 0.0
    attempt_count: int = 1

    @property
    def key(self) -> tuple[str, int]:
        return self.state, self.level

    @property
    def observed_attempt_seconds(self) -> float:
        return self.final_attempt_effort + self.prior_attempt_floor

    @property
    def final_attempt_effort(self) -> float:
        return self.seconds if self.effort_seconds is None else self.effort_seconds

    @property
    def uses_work_budget(self) -> bool:
        return self.effort_seconds is not None

    @property
    def has_abandoned_attempt(self) -> bool:
        return self.prior_attempt_floor > 0


@dataclasses.dataclass
class Progress:
    state: str
    level: int
    elapsed: float

    @property
    def key(self) -> tuple[str, int]:
        return self.state, self.level


@dataclasses.dataclass
class PendingAttempts:
    """Constant-space summary of visible progress episodes for one exact state."""

    earlier_floor: float = 0.0
    current_elapsed: float = 0.0
    current_level_epoch: int = 0
    episodes: int = 0

    def observe(self, elapsed: float, level_epoch: int) -> None:
        if self.episodes == 0:
            self.episodes = 1
            self.current_elapsed = elapsed
            self.current_level_epoch = level_epoch
            return
        # A same-level verdict cannot be emitted recursively beneath this activation because every
        # child has k-1.  It therefore proves that the prior episode returned, even if a later
        # episode's first visible elapsed value happens not to be smaller.
        if level_epoch != self.current_level_epoch or elapsed <= self.current_elapsed:
            self.earlier_floor += self.current_elapsed
            self.episodes += 1
        self.current_elapsed = elapsed
        self.current_level_epoch = level_epoch


@dataclasses.dataclass
class AttemptTracker:
    """Attach visible abandoned-attempt floors to later definitive verdicts."""

    pending: dict[tuple[str, int], PendingAttempts] = dataclasses.field(default_factory=dict)
    level_epochs: dict[int, int] = dataclasses.field(default_factory=dict)

    def observe(self, progress: Progress) -> None:
        epoch = self.level_epochs.get(progress.level, 0)
        self.pending.setdefault(progress.key, PendingAttempts()).observe(progress.elapsed, epoch)

    def annotate(self, verdict: Verdict) -> Verdict:
        pending = self.pending.pop(verdict.key, None)
        if pending is not None:
            # If the definitive activation itself emitted progress, its `took` includes the last
            # episode. Otherwise that episode was abandoned too. For historical logs, integer
            # `elapsed` and `took` are floors of the same CPU clock. Deterministic-budget verdicts
            # use their exact work/rate effort, matching the virtual elapsed field.
            same_level_boundary = (
                self.level_epochs.get(verdict.level, 0) != pending.current_level_epoch
            )
            final_episode_is_visible = (
                not same_level_boundary
                and verdict.final_attempt_effort >= pending.current_elapsed
            )
            verdict.prior_attempt_floor = pending.earlier_floor
            verdict.attempt_count = pending.episodes
            if not final_episode_is_visible:
                verdict.prior_attempt_floor += pending.current_elapsed
                verdict.attempt_count += 1
        self.level_epochs[verdict.level] = self.level_epochs.get(verdict.level, 0) + 1
        return verdict


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
    work_match = WORK_RE.search(line)
    effort_seconds = None
    if work_match is not None:
        work, rate = (int(value) for value in work_match.groups())
        if rate > 0:
            effort_seconds = work / rate
    return Verdict(
        state=state,
        level=int(level),
        outcome="yes" if outcome == "can solve" else "no",
        seconds=float(seconds),
        splits=int(splits),
        line_no=line_no,
        effort_seconds=effort_seconds,
    )


def parse_progress(line: str) -> Progress | None:
    match = PROGRESS_RE.match(line)
    if not match:
        return None
    level, state, elapsed, _deadline = match.groups()
    return Progress(state=state, level=int(level), elapsed=float(elapsed))


def iter_verdicts(path: Path) -> Iterator[Verdict]:
    estimator = SelfEstimator()
    attempts = AttemptTracker()
    with path.open("r", encoding="utf-8", errors="replace") as source:
        for line_no, line in enumerate(source, 1):
            progress = parse_progress(line)
            if progress is not None:
                attempts.observe(progress)
                continue
            verdict = parse_verdict(line, line_no)
            if verdict is not None:
                yield estimator.observe(attempts.annotate(verdict))


def scan(path: Path, label: str, top: int) -> Summary:
    summary = Summary(label=label, path=path)
    heap: list[tuple[float, int, Verdict]] = []
    for verdict in iter_verdicts(path):
        summary.verdicts += 1
        if verdict.level == 9:
            summary.k9_verdicts += 1
            mass = MASS_RE.search(verdict.state)
            first = FIRST_PART_RE.match(verdict.state)
            if mass and first and int(mass.group(2)) == 193:
                n1 = int(first.group(1))
                if 97 <= n1 <= 112:
                    summary.top_done.add(n1)

        item = (verdict.observed_attempt_seconds, verdict.line_no, verdict)
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
    for verdict in iter_verdicts(path):
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


def compact_floor(value: float) -> str:
    """Compact a lower bound without rounding it upward."""

    if value >= 1_000_000:
        return f"{math.floor(value / 10_000) / 100:.2f}m"
    if value >= 1_000:
        return f"{math.floor(value / 10) / 100:.2f}k"
    if value >= 10:
        return str(math.floor(value))
    floored = math.floor(value * 1_000) / 1_000
    return f"{floored:.3f}".rstrip("0").rstrip(".")


def short_state(state: str, width: int = 70) -> str:
    if len(state) <= width:
        return state
    return state[: width - 1] + "…"


def timing_pair(verdict: Verdict) -> str:
    attempt_time = compact_number(verdict.observed_attempt_seconds)
    if verdict.has_abandoned_attempt:
        attempt_time = f"≥{compact_floor(verdict.observed_attempt_seconds)}({verdict.attempt_count}a)"
    self_time = "-" if verdict.estimated_self is None else compact_number(verdict.estimated_self)
    return f"{attempt_time}/{self_time}"


def ratio(value: float | None, peer_value: float | None) -> str:
    if value is None or peer_value is None or value < 0 or peer_value <= 0:
        return "-"
    return f"{value / peer_value:.2f}x"


def attempt_ratio(verdict: Verdict, peer: Verdict) -> str:
    result = ratio(verdict.observed_attempt_seconds, peer.observed_attempt_seconds)
    if result != "-" and (
        verdict.has_abandoned_attempt
        or peer.has_abandoned_attempt
        or verdict.uses_work_budget != peer.uses_work_budget
    ):
        return f"~{result}"
    return result


def format_summary(a: Summary, b: Summary) -> str:
    behind, peer = sorted((a, b), key=lambda item: item.progress_key)
    matches = find_matches(peer.path, {verdict.key for verdict in behind.top_slow})
    now = dt.datetime.now(dt.timezone.utc).replace(microsecond=0).isoformat()
    lines = [
        f"compare {now}  behind={behind.label} "
        f"({len(behind.top_done)}/16 roots, {behind.verdicts} verdicts)",
        f"slow states (attempt-effort≥/~CPU-self-final) from {behind.label}"
        f"       {behind.label:>15} {peer.label:>15}    a×/~s×",
    ]
    for verdict in behind.top_slow:
        other = matches.get(verdict.key)
        state = short_state(verdict.state)
        if other is None:
            comparison = f"{timing_pair(verdict):>15} {'-':>15} {'-':>11}"
        else:
            ratios = (
                f"{attempt_ratio(verdict, other)}/"
                f"{ratio(verdict.estimated_self, other.estimated_self)}"
            )
            comparison = (
                f"{timing_pair(verdict):>15} "
                f"{timing_pair(other):>15} {ratios:>11}"
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
