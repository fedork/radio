#!/usr/bin/env python3
"""Reproduce the complete small-state rb suffix-pliability census.

Enumerate every four-part multiset over 2 <= n <= 5, 1 <= m <= n with total pair mass 22..27,
matching the independent exact census described in evidence/work_budget_rb_root_2026-08-13.txt.
This tool classifies only theorem-filtered mass reachability; it does not recursively solve states.
"""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import itertools
from pathlib import Path
import re
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[1]
FIELD_RE = re.compile(r"(\w+)=([^ ]+)")


def build_probe(destination: Path) -> None:
    command = [
        sys.executable,
        str(ROOT / "tools" / "build_radio.py"),
        "-O3",
        "-DMAX_K=3",
        "-DMAX_N=40",
        "tools/rb_root_probe.c",
        "-o",
        str(destination),
    ]
    completed = subprocess.run(command, cwd=ROOT, text=True, capture_output=True)
    if completed.returncode:
        sys.stderr.write(completed.stdout)
        sys.stderr.write(completed.stderr)
        raise SystemExit(completed.returncode)


def analyze(probe: Path) -> tuple[list[dict[str, int]], int]:
    part_types = [(n, m) for n in range(2, 6) for m in range(1, n + 1)]
    rows: list[dict[str, int]] = []
    parent_theorem_dead = 0

    for state in itertools.combinations_with_replacement(part_types, 4):
        mass = sum(n * m for n, m in state)
        if not 22 <= mass <= 27:
            continue
        command = [str(probe), "3"]
        for n, m in state:
            command.extend((str(n), str(m)))
        completed = subprocess.run(command, cwd=ROOT, text=True, capture_output=True)
        if completed.returncode:
            sys.stderr.write(completed.stdout)
            sys.stderr.write(completed.stderr)
            raise SystemExit(f"probe exited {completed.returncode} for {state}")
        line = next(
            (item for item in completed.stdout.splitlines() if item.startswith("RB_PLIABILITY ")),
            None,
        )
        if line is None:
            if "reason=parent-star-majorization" not in completed.stdout:
                raise SystemExit(f"missing pliability result for unrecognized reason: {state}")
            parent_theorem_dead += 1
            continue
        rows.append({key: int(value) for key, value in FIELD_RE.findall(line)})

    return rows, parent_theorem_dead


def print_summary(rows: list[dict[str, int]], parent_theorem_dead: int) -> None:
    print(f"states={len(rows) + parent_theorem_dead} parent_theorem_dead={parent_theorem_dead} "
          f"rb_analyzed={len(rows)}")
    print(f"root_rb_dead={sum(not row['root_pliable'] for row in rows)}")
    print(f"exact_no_callable_prune="
          f"{sum(row['potential_call_suffixes'] == 0 for row in rows)}")
    print(f"theorem_proves_no_callable_prune={sum(row['theorem_head'] <= 1 for row in rows)}")
    print(f"coarse_proves_no_callable_prune={sum(row['coarse_head'] <= 1 for row in rows)}")
    for field in ("exact_head", "theorem_head", "coarse_head"):
        counts = dict(sorted(Counter(row[field] for row in rows).items()))
        print(field, counts)

    by_slack: dict[int, list[int]] = defaultdict(lambda: [0, 0, 0])
    for row in rows:
        bucket = by_slack[row["slack"]]
        bucket[0] += 1
        bucket[1] += row["potential_call_suffixes"] == 0
        bucket[2] += row["theorem_head"] <= 1
    print("slack: analyzed/exact-no-call/theorem-no-call")
    for slack in sorted(by_slack):
        analyzed, exact, theorem = by_slack[slack]
        print(f"{slack}: {analyzed}/{exact}/{theorem}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--probe", type=Path, help="reuse an already-built rb_root_probe")
    args = parser.parse_args()

    if args.probe is not None:
        rows, parent_theorem_dead = analyze(args.probe.resolve())
        print_summary(rows, parent_theorem_dead)
        return

    with tempfile.TemporaryDirectory(prefix="radio-rb-pliability-") as directory:
        probe = Path(directory) / "rb_root_probe"
        build_probe(probe)
        rows, parent_theorem_dead = analyze(probe)
        print_summary(rows, parent_theorem_dead)


if __name__ == "__main__":
    main()
