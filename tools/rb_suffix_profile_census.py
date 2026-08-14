#!/usr/bin/env python3
"""Measure actual rb_dead calls and rejections by suffix on the complete small census.

The build forces reachability to arm after the first accepted prefix.  This is a diagnostic of the
counterfactual proactive policy, not the production trigger.  Every solver invocation is cold so
cache history from one state cannot affect another state's suffix profile.
"""

from __future__ import annotations

import argparse
from collections import defaultdict
import itertools
from pathlib import Path
import re
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[1]
FIELD_RE = re.compile(r"(\w+)=([^ ]+)")


def fields(line: str) -> dict[str, str]:
    return dict(FIELD_RE.findall(line))


def build_solver(destination: Path, trigger: int) -> None:
    command = [
        sys.executable,
        str(ROOT / "tools" / "build_radio.py"),
        "-O3",
        "-DMAX_K=3",
        "-DMAX_N=40",
        f"-DRB_TRIGGER={trigger}",
        "-DRADIO_RB_PROFILE_DIAGNOSTIC",
        "radio_one.c",
        "-o",
        str(destination),
    ]
    completed = subprocess.run(command, cwd=ROOT, text=True, capture_output=True)
    if completed.returncode:
        sys.stderr.write(completed.stdout)
        sys.stderr.write(completed.stderr)
        raise SystemExit(completed.returncode)


def parse_profile(stderr: str) -> dict[str, object] | None:
    profile: dict[str, object] | None = None
    suffixes: dict[int, dict[str, int]] = {}
    ended = False

    for line in stderr.splitlines():
        if line.startswith("RB_PROFILE_BEGIN "):
            if profile is not None:
                raise ValueError("multiple reachability profiles in one cold query")
            raw = fields(line)
            profile = {
                key: int(value) if key != "state" else value
                for key, value in raw.items()
            }
        elif line.startswith("RB_PLIABILITY ") and profile is not None:
            profile["pliability"] = {key: int(value) for key, value in fields(line).items()}
        elif line.startswith("RB_PLIABILITY_SUFFIX ") and profile is not None:
            row = {key: int(value) for key, value in fields(line).items()}
            suffixes.setdefault(row["index"], {}).update(row)
        elif line.startswith("RB_PROFILE_SUFFIX ") and profile is not None:
            row = {key: int(value) for key, value in fields(line).items()}
            suffixes.setdefault(row["index"], {}).update(row)
        elif line.startswith("RB_PROFILE_END ") and profile is not None:
            profile.update({key: int(value) for key, value in fields(line).items()})
            ended = True

    if profile is None:
        return None
    if not ended or "pliability" not in profile:
        raise ValueError("incomplete reachability profile")
    profile["suffixes"] = suffixes
    return profile


def census(solver: Path, limit: int | None) -> tuple[list[dict[str, object]], int]:
    part_types = [(n, m) for n in range(2, 6) for m in range(1, n + 1)]
    profiles: list[dict[str, object]] = []
    no_profile = 0
    seen = 0

    for state in itertools.combinations_with_replacement(part_types, 4):
        mass = sum(n * m for n, m in state)
        if not 22 <= mass <= 27:
            continue
        if limit is not None and seen >= limit:
            break
        seen += 1
        command = [str(solver), "3"]
        for n, m in state:
            command.extend((str(n), str(m)))
        completed = subprocess.run(command, cwd=ROOT, text=True, capture_output=True)
        if completed.returncode not in (0, 1):
            sys.stderr.write(completed.stdout)
            sys.stderr.write(completed.stderr)
            raise SystemExit(f"solver exited {completed.returncode} for {state}")
        try:
            profile = parse_profile(completed.stderr)
        except ValueError as error:
            raise SystemExit(f"{error}: {state}") from error
        if profile is None:
            no_profile += 1
        else:
            profiles.append(profile)

    return profiles, no_profile


def ratio(numerator: int, denominator: int) -> str:
    return f"{100.0 * numerator / denominator:.2f}%" if denominator else "n/a"


def summarize(profiles: list[dict[str, object]], no_profile: int) -> None:
    rows: list[dict[str, int]] = []
    state_rows: list[tuple[dict[str, object], list[dict[str, int]]]] = []
    for profile in profiles:
        slack = int(profile["slack"])
        suffixes = profile["suffixes"]
        assert isinstance(suffixes, dict)
        current_rows: list[dict[str, int]] = []
        for raw in suffixes.values():
            if "calls" not in raw:
                continue
            row = dict(raw)
            row["slack"] = slack
            row["flex"] = slack - row["excess"]
            rows.append(row)
            current_rows.append(row)
        state_rows.append((profile, current_rows))

    total_calls = sum(row["calls"] for row in rows)
    total_pruned = sum(row["pruned"] for row in rows)
    states_with_prune = sum(int(profile["pruned"]) > 0 for profile in profiles)
    improved = []
    for profile in profiles:
        pliability = profile["pliability"]
        assert isinstance(pliability, dict)
        if pliability["slack_excess_head"] < pliability["coarse_head"]:
            improved.append(str(profile["state"]))
    print(f"profiles={len(profiles)} no_profile={no_profile} "
          f"states_with_prune={states_with_prune}")
    print(f"suffix_rows={len(rows)} calls={total_calls} pruned={total_pruned} "
          f"prune_rate={ratio(total_pruned, total_calls)}")
    print(f"full_slack_excess_improvements={len(improved)} "
          f"examples={','.join(improved[:3]) if improved else '-'}")

    for certificate in ("exact", "theorem", "coarse", "slack_excess"):
        certified = [row for row in rows if row.get(certificate, 0)]
        calls = sum(row["calls"] for row in certified)
        pruned = sum(row["pruned"] for row in certified)
        print(f"{certificate}_certified rows={len(certified)} calls={calls} pruned={pruned}")

    by_slack: dict[int, list[int]] = defaultdict(lambda: [0, 0, 0, 0])
    by_flex: dict[int, list[int]] = defaultdict(lambda: [0, 0, 0])
    for profile in profiles:
        bucket = by_slack[int(profile["slack"])]
        bucket[0] += 1
        bucket[1] += int(profile["pruned"]) > 0
    for row in rows:
        slack_bucket = by_slack[row["slack"]]
        slack_bucket[2] += row["calls"]
        slack_bucket[3] += row["pruned"]
        flex_bucket = by_flex[row["flex"]]
        flex_bucket[0] += 1
        flex_bucket[1] += row["calls"]
        flex_bucket[2] += row["pruned"]

    print("slack: profiles/with-prune/calls/pruned/rate")
    for slack, (count, with_prune, calls, pruned) in sorted(by_slack.items()):
        print(f"{slack}: {count}/{with_prune}/{calls}/{pruned}/{ratio(pruned, calls)}")

    print("flex=sigma-D: rows/calls/pruned/rate")
    for flex, (count, calls, pruned) in sorted(by_flex.items()):
        print(f"{flex}: {count}/{calls}/{pruned}/{ratio(pruned, calls)}")
    pruned_rows = [row for row in rows if row["pruned"]]
    largest_pruned_flex = max((row["flex"] for row in pruned_rows), default=None)
    largest_examples = []
    if largest_pruned_flex is not None:
        for profile, current in state_rows:
            for row in current:
                if row["pruned"] and row["flex"] == largest_pruned_flex:
                    largest_examples.append(f"{profile['state']}@{row['index']}")
    print(f"largest_flex_with_prune="
          f"{largest_pruned_flex if largest_pruned_flex is not None else 'none'} "
          f"examples={','.join(largest_examples[:3]) if largest_examples else '-'}")

    print("state rule min(sigma-D)>=threshold: states/with-prune/calls/pruned")
    for threshold in (0, 1, 2):
        selected = [
            (profile, current)
            for profile, current in state_rows
            if current and min(row["flex"] for row in current) >= threshold
        ]
        calls = sum(row["calls"] for _, current in selected for row in current)
        pruned = sum(row["pruned"] for _, current in selected for row in current)
        with_prune = sum(int(profile["pruned"]) > 0 for profile, _ in selected)
        print(f"{threshold}: {len(selected)}/{with_prune}/{calls}/{pruned}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--solver", type=Path, help="reuse an already-built diagnostic radio_one")
    parser.add_argument("--trigger", type=int, default=1)
    parser.add_argument("--limit", type=int)
    args = parser.parse_args()

    if args.trigger < 1 or (args.limit is not None and args.limit < 1):
        parser.error("trigger and limit must be positive")

    if args.solver is not None:
        profiles, no_profile = census(args.solver.resolve(), args.limit)
        summarize(profiles, no_profile)
        return

    with tempfile.TemporaryDirectory(prefix="radio-rb-suffix-profile-") as directory:
        solver = Path(directory) / "radio_profile"
        build_solver(solver, args.trigger)
        profiles, no_profile = census(solver, args.limit)
        summarize(profiles, no_profile)


if __name__ == "__main__":
    main()
