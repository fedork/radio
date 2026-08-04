#!/usr/bin/env python3
"""Necessary conditions on a symbolic profile, from the m-side pattern alone.

Companion to tools/symbolic_profile.py. Where that one searches, this one asks whether a
search could possibly succeed -- cheaply, and without touching the n-side.

A level-q node holding `r` coins must play `r` letters that fit `G_t`, whose dyadic blocks
have multiplicities `1, 1, 2, 4, ...`. So at most one coin there plays `A`, at most two play
`A` or `B`, at most four play `A..C`. Writing `S_j = sum_v max(0, r_v - j)` over level-q
nodes, and `n_X` for how many copies of letter `X` each coin's profile holds:

    m * nA  <=  (number of occupied nodes)      every A-play needs its own node
    S_1     <=  m * (nB + nC + nD)              r-1 coins at a node play non-A
    S_2     <=  m * (nC + nD)                   r-2 of them play C or D
    S_4     <=  m * nD                          r-4 of them play D

All four depend only on which coins are alive where, which is `n`-free and `t`-free, so the
whole thing is a small DP over multisets of part sizes. Failing any of them refutes the
profile outright; passing them proves nothing, but it says a search is not doomed and points
at where the real obstruction has to be.

Usage:
    tools/occupancy_bound.py <m> <q> <profile>      e.g. 6 6 AAA...BBBCCCCCD
    tools/occupancy_bound.py check                  the cases whose answer is known
"""
from __future__ import annotations

import sys
from functools import lru_cache

LETTERS = "ABCDEFGH"


def multiplicity(i: int) -> int:
    return 1 if i <= 1 else 1 << (i - 1)


@lru_cache(maxsize=None)
def occupied(parts: tuple, d: int) -> int:
    """Most level-q nodes the coins can reach. Parts move by their own splits only."""
    parts = tuple(sorted(p for p in parts if p > 0))
    if not parts:
        return 0
    if d == 0:
        return 1
    best = 0

    def rec(i, k2, k0, k1):
        nonlocal best
        if i == len(parts):
            best = max(best, occupied(k2, d - 1) + occupied(k0, d - 1) + occupied(k1, d - 1))
            return
        c = parts[i]
        for b in range(c + 1):
            rec(i + 1, k2 + (b,), k0 + (c - b,), k1 + (c - b, b))

    rec(0, (), (), ())
    return best


def _pareto(vs):
    out = []
    for v in sorted(set(vs)):
        if not any(all(w[i] <= v[i] for i in range(3)) and w != v for w in out):
            out.append(v)
    return tuple(out)


@lru_cache(maxsize=None)
def crowding(parts: tuple, d: int) -> tuple:
    """Pareto-minimal (S_1, S_2, S_4) over every m-side pattern."""
    parts = tuple(sorted(p for p in parts if p > 0))
    r = sum(parts)
    if d == 0:
        return ((max(0, r - 1), max(0, r - 2), max(0, r - 4)),) if r else ((0, 0, 0),)
    acc = set()

    def rec(i, k2, k0, k1):
        if i == len(parts):
            for a in crowding(k2, d - 1):
                for b in crowding(k0, d - 1):
                    for c in crowding(k1, d - 1):
                        acc.add(tuple(a[j] + b[j] + c[j] for j in range(3)))
            return
        c = parts[i]
        for b in range(c + 1):
            rec(i + 1, k2 + (b,), k0 + (c - b,), k1 + (c - b, b))

    rec(0, (), (), ())
    return _pareto(acc)


def parse_profile(s: str) -> tuple:
    counts = [0] * len(LETTERS)
    for ch in s.upper():
        counts[LETTERS.index(ch)] += 1
    while counts and counts[-1] == 0:
        counts.pop()
    return tuple(counts)


def report(m: int, q: int, prof: str, known: str = "") -> bool:
    c = parse_profile(prof) + (0, 0, 0, 0)
    nA, nB, nC, nD = c[0], c[1], c[2], c[3]
    occ = occupied((m,), q)
    lim = (m * (nB + nC + nD), m * (nC + nD), m * nD)
    front = crowding((m,), q)
    ok = [v for v in front if all(v[i] <= lim[i] for i in range(3))]
    blocked = (m * nA > occ) or not ok
    print(f"m={m} q={q} nA={nA} nB={nB} nC={nC} nD={nD}")
    print(f"   A-nodes needed {m * nA}, most reachable {occ} (of 3^{q}={3 ** q})"
          f"  -> {'BLOCKED' if m * nA > occ else 'ok'}")
    print(f"   crowding Pareto {front}, limits {lim}"
          f"  -> {'BLOCKED' if not ok else 'ok'}")
    print(f"   verdict: {'REFUTED' if blocked else 'not refuted'}"
          + (f"   (known: {known})" if known else ""))
    return blocked


KNOWN = [(3, 2, "AABC", "feasible"), (4, 2, "AACC", "infeasible"),
         (4, 3, "AAAABBCC", "feasible"), (5, 4, "AAAAAAAAABBBBCCD", "feasible"),
         (6, 6, "A" * 46 + "B" * 12 + "C" * 5 + "D", "open")]


def main() -> int:
    if len(sys.argv) == 2 and sys.argv[1] == "check":
        for m, q, prof, k in KNOWN:
            report(m, q, prof, k)
        return 0
    if len(sys.argv) < 4:
        print(__doc__)
        return 2
    return 1 if report(int(sys.argv[1]), int(sys.argv[2]), sys.argv[3]) else 0


if __name__ == "__main__":
    sys.exit(main())
