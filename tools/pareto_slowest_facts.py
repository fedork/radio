#!/usr/bin/env python3
"""Rank solver facts by their largest completed or in-progress inclusive time."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path
from typing import Iterable, TextIO


COMPLETED_RE = re.compile(
    r"^(?P<verdict>can solve|can't solve)(?: size=\d+/\d+)? "
    r"(?P<state>S[ab]\([^)]*\))\[[^]]*\] in (?P<k>\d+)\b.*?"
    r"\btook (?P<took>\d+(?:\.\d+)?)\b"
)
PROGRESS_RE = re.compile(
    r"^still solving in (?P<k>\d+)\b.*? "
    r"(?P<state>S[ab]\([^)]*\))\[[^]]*\].*?"
    r"\belapsed (?P<elapsed>\d+(?:\.\d+)?)/"
)


def input_lines(paths: list[str]) -> Iterable[str]:
    if not paths:
        yield from sys.stdin
        return
    for name in paths:
        with Path(name).open(encoding="utf-8", errors="replace") as stream:
            yield from stream


def update_max(fact: dict[str, object], field: str, value: float) -> None:
    previous = fact[field]
    if previous is None or value > previous:
        fact[field] = value


def parse(lines: Iterable[str]) -> dict[tuple[int, str], dict[str, object]]:
    facts: dict[tuple[int, str], dict[str, object]] = {}
    for line in lines:
        match = COMPLETED_RE.match(line)
        if match:
            key = (int(match["k"]), match["state"])
            fact = facts.setdefault(
                key, {"completed": None, "elapsed": None, "verdict": None}
            )
            update_max(fact, "completed", float(match["took"]))
            verdict = "SOLVABLE" if match["verdict"] == "can solve" else "UNSOLVABLE"
            old_verdict = fact["verdict"]
            if old_verdict is not None and old_verdict != verdict:
                raise ValueError(f"contradictory completed verdicts for {match['state']}@{match['k']}")
            fact["verdict"] = verdict
            continue

        match = PROGRESS_RE.match(line)
        if match:
            key = (int(match["k"]), match["state"])
            fact = facts.setdefault(
                key, {"completed": None, "elapsed": None, "verdict": None}
            )
            update_max(fact, "elapsed", float(match["elapsed"]))
    return facts


def seconds(value: float | None) -> str:
    if value is None:
        return "-"
    if value < 1:
        return f"{value:.3f}"
    if value.is_integer():
        return str(int(value))
    return f"{value:.3f}".rstrip("0").rstrip(".")


def render(facts: dict[tuple[int, str], dict[str, object]], limit: int, stream: TextIO) -> None:
    ranked = []
    for (k, state), fact in facts.items():
        completed = fact["completed"]
        elapsed = fact["elapsed"]
        score = max(value for value in (completed, elapsed) if value is not None)
        basis = "completed" if completed is not None and completed >= (elapsed or -1) else "elapsed"
        ranked.append((score, k, state, basis, fact))
    ranked.sort(key=lambda row: (-row[0], -row[1], row[2]))

    print(
        "  slowest facts       rank=max(completed took, highest progress elapsed); "
        "inclusive solver-clock seconds",
        file=stream,
    )
    print(
        "  timing note         took is process CPU; progress elapsed is deterministic "
        "work-equivalent time, not audited wall time",
        file=stream,
    )
    for rank, (score, k, state, basis, fact) in enumerate(ranked[:limit], 1):
        verdict = fact["verdict"] or "NO_FINAL_VERDICT"
        print(
            f"    {rank:2d}. {seconds(score):>7}s {basis:<9} {verdict:<16} "
            f"{state}@{k} (took={seconds(fact['completed'])}, "
            f"elapsed={seconds(fact['elapsed'])})",
            file=stream,
        )


def main() -> int:
    parser = argparse.ArgumentParser(
        description="rank distinct solver facts by max(completed took, progress elapsed)"
    )
    parser.add_argument("paths", nargs="*", help="raw solver logs; read stdin when omitted")
    parser.add_argument("--limit", type=int, default=10)
    args = parser.parse_args()
    if args.limit < 1:
        parser.error("--limit must be positive")
    try:
        facts = parse(input_lines(args.paths))
    except (OSError, ValueError) as error:
        print(f"pareto_slowest_facts: {error}", file=sys.stderr)
        return 1
    render(facts, args.limit, sys.stdout)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
