#!/usr/bin/env python3
"""Prototype the synchronized-majorization hierarchy R_d.

R_0(S, k) is full-star majorization.  R_d, d>0, asks for one legal
rectangle split whose three children satisfy R_{d-1}.  This is a research
prototype: it favors transparent independent logic over solver integration.
"""

from __future__ import annotations

import argparse
import functools
import re
import time
from dataclasses import dataclass

Part = tuple[int, int]
State = tuple[Part, ...]


def normalize(parts: list[Part] | tuple[Part, ...]) -> State:
    out = []
    for x, y in parts:
        if x <= 0 or y <= 0:
            continue
        out.append((max(x, y), min(x, y)))
    return tuple(sorted(out, reverse=True))


@functools.cache
def base(k: int) -> tuple[int, ...]:
    g = (1,)
    for _ in range(k):
        nxt = [0] * (2 * len(g))
        for i, value in enumerate(g):
            nxt[i] += value
            nxt[2 * i] += value
            nxt[2 * i + 1] += value
        g = tuple(sorted(nxt, reverse=True))
    return g


@functools.cache
def r0(state: State, k: int) -> bool:
    if sum(n * m for n, m in state) > 3**k:
        return False
    profile = sorted((n for n, m in state for _ in range(m)), reverse=True)
    g = base(k)
    left = right = 0
    for i, value in enumerate(profile[: len(g)]):
        left += value
        right += g[i]
        if left > right:
            return False
    return True


def children(part: Part, split: Part) -> tuple[State, State, State]:
    n, m = part
    a, b = split
    return (
        normalize(((n - a, m - b),)),
        normalize(((a, m - b), (n - a, b))),
        normalize(((a, b),)),
    )


@functools.cache
def options(part: Part) -> tuple[tuple[Part, tuple[State, State, State]], ...]:
    n, m = part
    out = []
    for a in range(n + 1):
        for b in range(m + 1):
            ch = children(part, (a, b))
            masses = [sum(x * y for x, y in state) for state in ch]
            # Prefer balanced options; use coordinates only as deterministic tie-breakers.
            score = (max(masses), max(masses) - min(masses), sum(v * v for v in masses), a, b)
            out.append((score, (a, b), ch))
    out.sort()
    return tuple((split, ch) for _, split, ch in out)


@dataclass
class Counters:
    calls: int = 0
    assignments: int = 0
    prefix_rejects: int = 0


COUNTERS = Counters()
WITNESS: dict[tuple[State, int, int], tuple[Part, ...]] = {}


@functools.cache
def relax(state: State, k: int, depth: int) -> bool:
    COUNTERS.calls += 1
    if not r0(state, k):
        return False
    if depth == 0 or not state:
        return True
    if k == 0:
        return True  # r0 already implies at most one edge.
    if all(m == 1 for _, m in state):
        return True  # R_0 is exact for singleton states.

    # R_depth is subgraph-monotone.  Therefore each contribution from one parent
    # part must itself satisfy R_{depth-1} in every outcome.  This local filter is
    # necessary and usually much cheaper than entering the Cartesian product.
    candidate = []
    for original, part in enumerate(state):
        keep = []
        for split, ch in options(part):
            if all(relax(child, k - 1, depth - 1) for child in ch):
                keep.append((split, ch))
        if not keep:
            return False
        candidate.append((original, part, keep))

    candidate.sort(key=lambda item: (len(item[2]), -(item[1][0] * item[1][1]), item[1]))
    selected: list[Part] = [(0, 0)] * len(candidate)
    selected_by_original: list[Part] = [(0, 0)] * len(candidate)
    partial: list[list[Part]] = [[], [], []]

    def dfs(i: int) -> bool:
        if i == len(candidate):
            # Every prefix, including this complete assignment, was checked below.
            return True
        original, part, choices = candidate[i]
        previous_split: Part | None = None
        if i and candidate[i - 1][1] == part:
            previous_split = selected[i - 1]
        for split, local_children in choices:
            # Identical components are interchangeable.
            if previous_split is not None and split < previous_split:
                continue
            COUNTERS.assignments += 1
            old_lengths = [len(x) for x in partial]
            for outcome in range(3):
                partial[outcome].extend(local_children[outcome])
            child_states = [normalize(x) for x in partial]
            ok = all(relax(child, k - 1, depth - 1) for child in child_states)
            if ok:
                selected[i] = split
                selected_by_original[original] = split
                if dfs(i + 1):
                    WITNESS[(state, k, depth)] = tuple(selected_by_original)
                    return True
            else:
                COUNTERS.prefix_rejects += 1
            for outcome in range(3):
                del partial[outcome][old_lengths[outcome] :]
        return False

    return dfs(0)


def parse_state(words: list[str]) -> State:
    if len(words) % 2:
        raise ValueError("state needs n m pairs")
    return normalize([(int(words[i]), int(words[i + 1])) for i in range(0, len(words), 2)])


def report_one(state: State, k: int, depth: int) -> bool:
    before = Counters(**vars(COUNTERS))
    started = time.process_time()
    answer = relax(state, k, depth)
    elapsed = time.process_time() - started
    print(f"R_{depth}={'YES' if answer else 'NO'} k={k} state={state}")
    if answer and depth:
        print(f"split={WITNESS.get((state, k, depth))}")
    info = relax.cache_info()
    print(
        f"cpu={elapsed:.6f}s calls={COUNTERS.calls - before.calls} "
        f"assignments={COUNTERS.assignments - before.assignments} "
        f"prefix_rejects={COUNTERS.prefix_rejects - before.prefix_rejects} "
        f"memo={info.currsize} hits={info.hits}"
    )
    return answer


def cmd_solve(args: argparse.Namespace) -> int:
    state = parse_state(args.state)
    return 0 if report_one(state, args.k, args.depth) else 1


def cmd_ladder(args: argparse.Namespace) -> int:
    state = parse_state(args.state)
    for depth in range(args.k + 1):
        report_one(state, args.k, depth)
    return 0


def read_pair_table(path: str) -> tuple[list[Part], set[tuple[int, int, int, int]]]:
    """Read complete `pairtab` output: exact four-integer lines are its solvable pairs."""
    solved = set()
    parts = set()
    pattern = re.compile(r"\d+ \d+ \d+ \d+\s*")
    with open(path, encoding="utf-8") as stream:
        for line in stream:
            if not pattern.fullmatch(line):
                continue
            values = tuple(map(int, line.split()))
            solved.add(values)
            parts.add(values[:2])
            parts.add(values[2:])
    # pairtab enumerates m outside n; retain that order to reconstruct its i<=j universe.
    return sorted(parts, key=lambda p: (p[1], p[0])), solved


def cmd_census_pairs(args: argparse.Namespace) -> int:
    parts, solved = read_pair_table(args.table)
    raw: list[tuple[State, bool]] = []
    canonical: dict[State, bool] = {}
    for i, left in enumerate(parts):
        for right in parts[i:]:
            exact = (*left, *right) in solved
            state = normalize((left, right))
            raw.append((state, exact))
            previous = canonical.setdefault(state, exact)
            if previous != exact:
                raise ValueError(f"pair table contradicts itself after orientation: {state}")

    print(
        f"parts={len(parts)} raw={len(raw)} raw_solvable={sum(x for _, x in raw)} "
        f"canonical={len(canonical)} canonical_solvable={sum(canonical.values())}"
    )
    for depth in range(args.k + 1):
        started = time.process_time()
        raw_false = sum(not relax(state, args.k, depth) for state, exact in raw if exact)
        raw_reject = sum(not relax(state, args.k, depth) for state, exact in raw if not exact)
        canonical_false = sum(
            not relax(state, args.k, depth) for state, exact in canonical.items() if exact
        )
        canonical_reject = sum(
            not relax(state, args.k, depth) for state, exact in canonical.items() if not exact
        )
        print(
            f"R_{depth}: raw_negative_rejections={raw_reject} raw_false_rejections={raw_false} "
            f"canonical_negative_rejections={canonical_reject} "
            f"canonical_false_rejections={canonical_false} "
            f"cpu={time.process_time() - started:.6f}s"
        )
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)

    solve_parser = commands.add_parser("solve", help="check one R_depth relaxation")
    solve_parser.add_argument("k", type=int)
    solve_parser.add_argument("depth", type=int)
    solve_parser.add_argument("state", nargs="+")
    solve_parser.set_defaults(function=cmd_solve)

    ladder_parser = commands.add_parser("ladder", help="check every depth from 0 through k")
    ladder_parser.add_argument("k", type=int)
    ladder_parser.add_argument("state", nargs="+")
    ladder_parser.set_defaults(function=cmd_ladder)

    census_parser = commands.add_parser(
        "census-pairs", help="compare every hierarchy depth with complete pairtab output"
    )
    census_parser.add_argument("k", type=int)
    census_parser.add_argument("table")
    census_parser.set_defaults(function=cmd_census_pairs)

    args = parser.parse_args()
    if hasattr(args, "depth") and not 0 <= args.depth <= args.k:
        parser.error("depth must be between 0 and k")
    if args.k < 0:
        parser.error("k must be nonnegative")
    return args.function(args)


if __name__ == "__main__":
    raise SystemExit(main())
