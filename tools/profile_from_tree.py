#!/usr/bin/env python3
"""Derive the base-sequence profile of a solution from its witness tree.

Background. For fixed `m`, `n(k,m)` appears to equal a fixed multiset of atoms drawn from
the base sequence `G_t` — a "profile". That was found by *fitting* numbers. This tool derives
it instead, from a verified witness tree, and in doing so gives the profile a mechanism:

    Fix one m-side coin y. Over the k-t tests lying above normalisation level t, y is either
    inside the tested set or outside it — so y has exactly 2^(k-t) root-to-level-t paths.
    Each path ends holding one chunk of the n-side, and that chunk must be resolvable in the
    t remaining tests while paired with y, so its size is an atom of G_t.

    The profile is therefore the multiset of chunk sizes along one coin's 2^(k-t) paths.

Three consequences, all checkable and all confirmed on the committed trees:

  length = 2^q      with q = k-t. Not a coincidence: it counts binary paths.
  refinement        lowering t by one doubles the path count, which is exactly why
                    `length = 2^q` is invariant across a refinement class.
  m-fold census     the whole tree's leaf-atom census at level t is m copies of the profile,
                    hence every count is divisible by m, and the total is m * 2^(k-t).

Two ways a real solution can fail to exhibit a profile, and both occur:

  empty paths       a path where y ends up with no n-side coins. The total atom count then
                    falls short of m * 2^(k-t). `Sb(473:6)@9` wastes 7 slots this way.
  asymmetry         the census is not divisible by m, so the coins do not share a common
                    decomposition. This is the "multiple-of-m atom-count sanity check" the
                    journal records as failing for 473:6; it fails for 2 of the 9 480:5
                    solutions too.

So a profile describes a *symmetric, non-wasteful* solution, not every solution.

Usage:  tools/profile_from_tree.py witnesses/canon_480_5_at9.tree [...]
        tools/profile_from_tree.py --expect BBBD witnesses/canon_480_5_at9.tree
"""
from __future__ import annotations

import argparse
import re
import sys
from collections import Counter
from math import comb

NODE = re.compile(r"^\s*(.*?)\s+@(\d+)\s+(?:\[canonical U_(\d+)\]|--\[(.*?)\]-->)\s*$")


def atom(r: int, j: int) -> int:
    """Value of dyadic block r of G_j: a partial binomial sum."""
    return 0 if (j < 0 or r > j) else sum(comb(j, i) for i in range(0, j - r + 1))


def block_of(value: int, j: int):
    for r in range(j + 1):
        if atom(r, j) == value:
            return r
    return None


def refine(coeffs, steps: int):
    """Refine a block-multiplicity vector `steps` levels down. A -> aa, X_i -> x_{i-1} x_i."""
    c = list(coeffs)
    for _ in range(steps):
        o = Counter()
        for r, n in enumerate(c):
            if not n:
                continue
            if r == 0:
                o[0] += 2 * n
            else:
                o[r - 1] += n
                o[r] += n
        c = [o.get(r, 0) for r in range(max(o) + 1)] if o else [0]
    return c


def parse_parts(text: str):
    out = []
    for p in text.split(","):
        p = p.strip()
        if p:
            a, b = (int(x) for x in p.split(":"))
            if a and b:
                out.append((max(a, b), min(a, b)))
    return out


def load(path: str):
    seq = []
    for line in open(path):
        if line.lstrip().startswith("#"):
            continue
        m = NODE.match(line.rstrip())
        if m:
            seq.append((parse_parts(m.group(1)), int(m.group(2)), m.group(3) is not None))
    pos, out = 0, []

    def build():
        nonlocal pos
        st, k, canon = seq[pos]
        pos += 1
        return (st, k, canon, [] if canon else [build() for _ in range(3)])

    while pos < len(seq):
        out.append(build())
    return out


def leaves_of(node):
    acc = []

    def walk(n):
        st, k, canon, kids = n
        if canon and st:
            acc.append((st, k))
        for c in kids:
            walk(c)

    walk(node)
    return acc


def analyse(tree):
    leaves = leaves_of(tree)
    k = tree[1]
    t = min(d for _, d in leaves)
    census, count = Counter(), 0
    for state, d in leaves:
        for a, mult in state:
            if mult != 1:
                return None                      # not a singleton leaf
            r = block_of(a, d)
            if r is None:
                return None                      # not an atom of G_d
            for rr, n in enumerate(refine([0] * r + [1], d - t)):
                if n:
                    census[atom(rr, t)] += n
                    count += n
    m = min(mult for state, _ in leaves for _, mult in state) if leaves else 0
    m = max(sum(mult for _, mult in tree[0]), 0)  # multiplicity of the root's m-side
    return dict(k=k, t=t, m=m, census=census, count=count,
                expected_count=m * 2 ** (k - t),
                symmetric=all(v % m == 0 for v in census.values()) if m else False)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("trees", nargs="+")
    ap.add_argument("--expect", help="profile letters at level k-q, e.g. BBBD")
    ap.add_argument("--q", type=int, default=2, help="offset for --expect (default 2)")
    args = ap.parse_args()

    bad = 0
    for path in args.trees:
        print(f"\n{path}")
        for i, tree in enumerate(load(path), 1):
            a = analyse(tree)
            if a is None:
                print(f"  solution {i}: not a canonical singleton-leaf tree")
                bad += 1
                continue
            waste = a["expected_count"] - a["count"]
            per = {v: c // a["m"] for v, c in sorted(a["census"].items(), reverse=True)} \
                if a["symmetric"] else None
            print(f"  solution {i}: m={a['m']} k={a['k']} normalised to level {a['t']}")
            print(f"    census   {dict(sorted(a['census'].items(), reverse=True))}")
            print(f"    atoms    {a['count']} of m*2^(k-t) = {a['expected_count']}"
                  + (f"   {waste} EMPTY PATHS" if waste else "   no waste"))
            if per:
                print(f"    profile  {per}  (length {sum(per.values())} = 2^{a['k']-a['t']})")
            else:
                print(f"    profile  none - census not divisible by m={a['m']} (ASYMMETRIC)")
                bad += 1
            if args.expect and per:
                lvl = a["k"] - args.q
                coeffs = [args.expect.count(chr(65 + r)) for r in range(8)]
                want = Counter()
                for rr, n in enumerate(refine(coeffs, lvl - a["t"])):
                    if n:
                        want[atom(rr, a["t"])] += n
                ok = Counter(per) == want
                print(f"    expect   {dict(sorted(want.items(), reverse=True))}"
                      f"  from {args.expect}@G_{lvl}  -> {'MATCH' if ok else 'DIFFERS'}")
                if not ok:
                    bad += 1
    print(f"\n{'all solutions carry a profile' if not bad else f'{bad} without a clean profile'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
