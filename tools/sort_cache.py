#!/usr/bin/env python3
"""Reorder a solver cache file so replaying it is cheaper.

Both cache inserts expand a closure rather than storing one node: a positive fact is propagated
down to every state it dominates, a negative fact up to every state that dominates it. So the
order facts arrive in decides how much of that work is wasted.

  * insert the LARGEST solvable states first  -- each subsumes every smaller one
  * insert the SMALLEST unsolvable states first -- each subsumes every larger one

Measured on a 99,672-fact sample of the archived census cache (98% negative): 304 facts/s in the
file's own discovery order against 685 facts/s sorted, a 2.25x speedup for a pure reordering.

Note what this does NOT fix. Only 1.4% of those inserts were redundant, so the cost is real closure
work, not duplicate facts -- reordering wins by making subsumption happen early, and no amount of
it turns an 8.9-hour replay into a short one. A structural snapshot of the trie is the actual fix.

    tools/sort_cache.py in.cache out.cache [--max-k K] [--max-n N]
"""

from __future__ import annotations

import argparse
from pathlib import Path


def key_of(line: str):
    """(sign, pairs, k) for a fact line, or None if it is not one."""
    if not line or line[0] not in "+-":
        return None
    w = line.split()
    try:
        t = w.index("t")
    except ValueError:
        return None
    if len(w) < t + 4:
        return None
    try:
        return (w[0] == "+", int(w[t + 1]), int(w[t + 3]))
    except ValueError:
        return None


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("src", type=Path)
    ap.add_argument("dst", type=Path)
    ap.add_argument("--max-k", type=int, default=None, help="drop facts above this k")
    ap.add_argument("--max-n", type=int, default=None, help="drop facts wider than this side sum")
    args = ap.parse_args()

    pos, neg, dropped, other = [], [], 0, 0
    with args.src.open() as fh:
        for line in fh:
            if line.startswith("#"):
                continue
            k = key_of(line.rstrip("\n"))
            if k is None:
                other += 1
                continue
            sign, pairs, kk = k
            if args.max_k is not None and kk > args.max_k:
                dropped += 1
                continue
            if args.max_n is not None:
                w = line.split()
                sides = sum(int(x) for x in w[2:w.index("t")])
                if sides > args.max_n:
                    dropped += 1
                    continue
            (pos if sign else neg).append((pairs, line))

    pos.sort(key=lambda r: -r[0])       # biggest solvable first
    neg.sort(key=lambda r: r[0])        # smallest unsolvable first
    with args.dst.open("w") as out:
        out.write(f"# reordered by tools/sort_cache.py from {args.src.name}: "
                  f"{len(pos)} positive (mass descending), {len(neg)} negative (mass ascending)\n")
        for _, line in pos:
            out.write(line)
        for _, line in neg:
            out.write(line)
    print(f"{args.src} -> {args.dst}: {len(pos)} positive, {len(neg)} negative, "
          f"{dropped} dropped by limits, {other} non-fact lines skipped")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
