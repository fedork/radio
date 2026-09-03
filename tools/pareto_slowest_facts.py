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


def compact_completed(line: str) -> str:
    text = line.rstrip()
    witness = text.find(" with ")
    if witness < 0:
        return text
    took = text.find(" took ", witness)
    tail = text[took:] if took >= 0 else ""
    return f"{text[:witness]}  [+witness]{tail}"


def compact_progress(line: str, k: int) -> str:
    text = line.rstrip()
    prefix = f"still solving in {k} "
    if text.startswith(prefix):
        text = text[len(prefix) :]
    return re.sub(r" trying .*? elapsed ", "  elapsed ", text, count=1)


def parse(
    lines: Iterable[str],
) -> tuple[
    dict[tuple[int, str], dict[str, object]],
    dict[int, dict[str, tuple[int, str]]],
    int,
]:
    facts: dict[tuple[int, str], dict[str, object]] = {}
    activities: dict[int, dict[str, tuple[int, str]]] = {}
    line_number = 0
    for line_number, line in enumerate(lines, 1):
        match = COMPLETED_RE.match(line)
        if match:
            k = int(match["k"])
            key = (k, match["state"])
            fact = facts.setdefault(
                key, {"completed": None, "elapsed": None, "verdict": None}
            )
            update_max(fact, "completed", float(match["took"]))
            verdict = "SOLVABLE" if match["verdict"] == "can solve" else "UNSOLVABLE"
            old_verdict = fact["verdict"]
            if old_verdict is not None and old_verdict != verdict:
                raise ValueError(f"contradictory completed verdicts for {match['state']}@{match['k']}")
            fact["verdict"] = verdict
            activities.setdefault(k, {})["done"] = (
                line_number,
                compact_completed(line),
            )
            continue

        match = PROGRESS_RE.match(line)
        if match:
            k = int(match["k"])
            key = (k, match["state"])
            fact = facts.setdefault(
                key, {"completed": None, "elapsed": None, "verdict": None}
            )
            update_max(fact, "elapsed", float(match["elapsed"]))
            activities.setdefault(k, {})["solving"] = (
                line_number,
                compact_progress(line, k),
            )
    return facts, activities, line_number


def seconds(value: float | None) -> str:
    if value is None:
        return "-"
    if value < 1:
        return f"{value:.3f}"
    if value.is_integer():
        return str(int(value))
    return f"{value:.3f}".rstrip("0").rstrip(".")


def render_stack(
    activities: dict[int, dict[str, tuple[int, str]]],
    total_lines: int,
    stream: TextIO,
) -> None:
    picked: dict[int, tuple[int, str, str]] = {}
    for k, kinds in activities.items():
        kind, (line_number, text) = max(kinds.items(), key=lambda item: item[1][0])
        picked[k] = (line_number, kind, text)
    if not picked:
        print("  current stack       unavailable (no solver activity lines)", file=stream)
        return

    current_k = max(picked.items(), key=lambda item: item[1][0])[0]
    root_k = max(picked)
    print(
        f"  current stack       latest activity per level, current k={current_k} up to root k={root_k}",
        file=stream,
    )
    print(
        "  stack note          newest progress or completed line at each level; old enclosing lines may be stale",
        file=stream,
    )
    for k in range(root_k, current_k - 1, -1):
        if k not in picked:
            continue
        line_number, kind, text = picked[k]
        stale = " (stale)" if total_lines - line_number > 200_000 else ""
        print(f"    k={k} [{kind}]{stale} {text}", file=stream)


def render(
    facts: dict[tuple[int, str], dict[str, object]],
    activities: dict[int, dict[str, tuple[int, str]]],
    total_lines: int,
    limit: int,
    stream: TextIO,
) -> None:
    render_stack(activities, total_lines, stream)
    print(file=stream)
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

    completed_ranked = [
        (fact["completed"], k, state, fact)
        for (k, state), fact in facts.items()
        if fact["completed"] is not None
    ]
    completed_ranked.sort(key=lambda row: (-row[0], -row[1], row[2]))
    print(file=stream)
    print(
        "  slowest completed   rank=final took only; inclusive process CPU seconds",
        file=stream,
    )
    for rank, (took, k, state, fact) in enumerate(completed_ranked[:limit], 1):
        print(
            f"    {rank:2d}. {seconds(took):>7}s {fact['verdict']:<16} "
            f"{state}@{k} (highest_elapsed={seconds(fact['elapsed'])})",
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
        facts, activities, total_lines = parse(input_lines(args.paths))
    except (OSError, ValueError) as error:
        print(f"pareto_slowest_facts: {error}", file=sys.stderr)
        return 1
    render(facts, activities, total_lines, args.limit, sys.stdout)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
