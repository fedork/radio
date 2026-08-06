#!/usr/bin/env python3
"""Prototype the negative certificate: reduce a solver log to a checkable fact set, verify it.

Design in docs/certificate.md. This is the experiment that replaces its estimates with numbers.
It is Python and therefore slow; a production verifier would be C. The point is to measure the
reduction ratios and the verification cost on real logs at small k before anything expensive
depends on them.

The trust base is exactly: Singleton Majorization, Unit-Group Elimination, Subgraph
Monotonicity, and the split semantics of docs/problem.md. Nothing about the solver - not its
orderings, not FAST, not deadlines, not its cache.

    tools/certify.py <log> [--maxk K] [--budget SECONDS]

Reports, per k: facts loaded, facts after subsumption, facts verified, facts reachable from the
level above, and the time each stage took.
"""
from __future__ import annotations

import re
import sys
import time
from collections import defaultdict
import multiprocessing as mp

POW3 = [3 ** i for i in range(16)]

NEG = re.compile(r"^can't solve (?:size=\S+ )?Sb\((.*?)\)\[(\d+),(\d+)\] in (\d+)")


# ---------------------------------------------------------------- the three theorems

def G(k: int) -> list[int]:
    """The singleton base sequence G_k, non-increasing.

    Same recurrence as init_singleton_majorization() in radiobase.c: from G_{k-1}, each entry h
    at index i contributes h to positions i, 2i and 2i+1, then sort descending. Checked against
    the values recorded in the journal: G_1=(2,1), G_2=(4,3,1,1), G_3=(8,7,4,4,1,1,1,1)."""
    cur = [1]
    for _ in range(k):
        nxt = [0] * (len(cur) * 2)
        for i, h in enumerate(cur):
            nxt[i] += h
            nxt[i * 2] += h
            nxt[i * 2 + 1] += h
        cur = sorted(nxt, reverse=True)
    return cur


_gcache: dict[int, list[int]] = {}


def gprefix(k: int) -> list[int]:
    if k not in _gcache:
        g = sorted(G(k), reverse=True)
        pref, s = [], 0
        for x in g:
            s += x
            pref.append(s)
        _gcache[k] = pref
    return _gcache[k]


def canon(parts) -> tuple:
    """Unit-Group Elimination: (1:1) parts never affect solvability, so drop them.
    Orient each part n>=m and sort descending."""
    out = [(max(n, m), min(n, m)) for n, m in parts if n > 0 and m > 0]
    out = [p for p in out if p != (1, 1)]
    out.sort(reverse=True)
    return tuple(out)


def mass(s) -> int:
    return sum(n * m for n, m in s)


def maj_refutes(s, k: int) -> bool:
    """Refute s using the SINGLETON SUB-MULTISET, not only the all-singleton case.

    Singleton Majorization decides all-singleton states exactly: unsolvable iff the n-sides are
    not weakly majorized by G_k. Combined with Subgraph Monotonicity that extends to any state -
    the singleton parts form a subgraph, so if *they* violate majorization the whole state is
    unsolvable. radiobase.c does exactly this (the `singleton_size > 0` branch of canSolveB) and
    returns FALSE without printing, which is why such facts never appear in a log.

    An earlier version only handled the all-singleton case and therefore could not reproduce
    25% of the k=4 facts in a frontier walk - they were not a closure gap at all, just a rule
    the verifier was missing."""
    if not s:
        return False
    # Downgrade EVERY part to its own singleton: (n:1) <= (n:m) componentwise, so the downgraded
    # state injects into s and Subgraph Monotonicity carries unsolvability upward. Strictly stronger
    # than using only the parts already at m==1.
    ns = sorted((n for n, _ in s), reverse=True)
    pref = gprefix(k)
    run = 0
    for i, n in enumerate(ns):
        run += n
        # Past len(G_k) the bound is the CONSTANT sum G_k = 3^k - the theorem pads with trailing
        # zeros - not a violation. An earlier version returned True there, which over-refutes; it
        # fired 79 times in one k=4 level once every part was being downgraded.
        if run > (pref[i] if i < len(pref) else pref[-1]):
            return True
    return False


# ---------------------------------------------------------------- subsumption index

def dominates(a, b) -> bool:
    """a <= b in the subgraph order: an injection from a's parts into b's with each part
    componentwise no larger. Subgraph Monotonicity then gives: a unsolvable => b unsolvable."""
    if len(a) > len(b) or mass(a) > mass(b):
        return False

    def rec(i: int, used: int) -> bool:
        if i == len(a):
            return True
        an, am = a[i]
        for j, (bn, bm) in enumerate(b):
            if used >> j & 1:
                continue
            if an <= bn and am <= bm:
                if rec(i + 1, used | 1 << j):
                    return True
        return False

    return rec(0, 0)


class Index:
    """Facts at one k, bucketed so a dominance query scans few candidates."""

    def __init__(self, facts):
        self.exact = set(facts)
        self.memo: dict = {}
        self.by_len = defaultdict(list)
        for f in facts:
            self.by_len[len(f)].append((mass(f), f))
        for v in self.by_len.values():
            v.sort()

    def refuted(self, s, k: int) -> bool:
        """Is s unsolvable in k? `self` must be the fact set for THIS k, never another level."""
        hit = self.memo.get(s)
        if hit is not None:
            return hit
        r = self._refuted(s, k)
        self.memo[s] = r
        return r

    def _refuted(self, s, k: int) -> bool:
        if mass(s) > POW3[k]:
            return True                      # COUNT
        if not s:
            return False                     # solved
        if s in self.exact:
            return True
        if maj_refutes(s, k):
            return True                      # MAJ
        ms = mass(s)
        for L in range(1, len(s) + 1):        # DOM, via Subgraph Monotonicity
            for m2, f in self.by_len.get(L, ()):
                if m2 > ms:
                    break
                if dominates(f, s):
                    return True
        return False


# ---------------------------------------------------------------- SPLITS verification

_LIVE: dict = {}


def _live_splits(part, k: int, below: Index, restrict: bool):
    """Splits of one part that are not dead group-locally.

    Depends only on (part, k, restrict) - never on the state containing the part - so it is
    memoised. Measured 2026-08-04 on the k=9 ladder: 729 distinct (part,k) pairs serve 657,945
    part-slots across 148,626 facts, a reuse factor near 900x and 5,120x at k=4. That is why
    this table is a verifier-side memo and not something worth shipping in the certificate.

    `restrict` applies the complement symmetry, valid for one designated group only."""
    key = (part, k, restrict)
    hit = _LIVE.get(key)
    if hit is not None:
        return hit
    n, m = part
    opts = []
    for a in range(n + 1):
        for b in range(m + 1):
            if restrict and (a, b) > (n - a, m - b):
                continue
            if (below.refuted(canon([(a, b)]), k - 1)
                    or below.refuted(canon([(n - a, m - b)]), k - 1)
                    or below.refuted(canon([(a, m - b), (n - a, b)]), k - 1)):
                continue
            opts.append((a, b, a * b, (n - a) * (m - b), n * m - a * b - (n - a) * (m - b)))
    _LIVE[key] = opts
    return opts

def splits_ok(s, k: int, below: Index, budget_end: float):
    """Every split of s must have a child refuted at k-1.

    A split whose child exceeds 3^(k-1) is covered by COUNT, which is exactly the prefix prune
    (mass is conserved across the three children, so partial sums only grow). Returns
    (verified, used_children) or (None, ...) if the time budget ran out."""
    cap = POW3[k - 1]
    used = set()
    P = len(s)

    # Group-local rejection, the same test radiobase.c memoises in s[4]/s[5]. If ONE group's
    # split has a child that is refuted at k-1 in isolation, that split is dead whatever every
    # other group does: the full child contains this group's contribution as a sub-multiset, so
    # Subgraph Monotonicity refutes the full child too. Measured 2026-08-04 over 25 facts with
    # >=3 parts: this cuts a further 107x beyond the counting bound (408,590 leaves -> 3,824),
    # and some facts drop to zero leaves, i.e. refuted with no product enumeration at all.
    live = [_live_splits(p, k, below, gi == 0) for gi, p in enumerate(s)]
    last = [-1] * (P + 1)

    def rec(i: int, s2, s0, s1, c2, c0, c1):
        if time.time() > budget_end:
            raise TimeoutError
        # Subset narrowing on the PREFIX {0..i-1}. A refuted PARTIAL child means every
        # completion has that child refuted too, since completions only add parts and Subgraph
        # Monotonicity preserves unsolvability upward - so the whole subtree is discharged here.
        # radiobase.c does this at every prefix via its three CACHE_ONLY probes; checking only
        # at the leaf, as an earlier version did, throws the reduction away.
        if i:
            for child in (canon(c2), canon(c0), canon(c1)):
                if below.refuted(child, k - 1):
                    used.add(child)
                    return True
        if i == P:
            return False                      # a split with no refuted child: not verified
        n, m = s[i]
        # Identical-part symmetry: equal parts have identical option lists, and swapping their
        # splits permutes the children, which are multisets. So require non-decreasing option
        # index across a run of equal parts. Factorial saving where parts repeat.
        lo = last[i - 1] + 1 if i and s[i] == s[i - 1] else 0
        for oi in range(lo, len(live[i])):
            a, b, k2, k0, k1 = live[i][oi]
            if s2 + k2 > cap or s0 + k0 > cap or s1 + k1 > cap:
                continue
            last[i] = oi
            if not rec(i + 1, s2 + k2, s0 + k0, s1 + k1,
                       c2 + [(a, b)], c0 + [(n - a, m - b)],
                       c1 + [(a, m - b), (n - a, b)]):
                return False
        return True

    try:
        return rec(0, 0, 0, 0, [], [], []), used
    except TimeoutError:
        return None, used


# ---------------------------------------------------------------- driver

def main() -> int:
    args = sys.argv[1:]
    if not args:
        print(__doc__)
        return 2
    log = args[0]
    maxk = int(args[args.index('--maxk') + 1]) if '--maxk' in args else 99
    budget = float(args[args.index('--budget') + 1]) if '--budget' in args else 120.0

    neg = defaultdict(set)
    for line in open(log):
        if not line.startswith("can't"):
            continue
        m = NEG.match(line)
        if not m:
            continue
        s = canon(tuple(map(int, p.split(':'))) for p in m.group(1).split(','))
        if s:
            neg[int(m.group(4))].add(s)

    workers = int(args[args.index('--workers') + 1]) if '--workers' in args else 1
    subsume = '--no-subsume' not in args

    print(f"log: {log}   workers={workers}   subsumption={'on' if subsume else 'off'}")
    print(f"{'k':>3} {'loaded':>9} {'checked':>9} {'subsum%':>8} "
          f"{'verified':>9} {'unver':>7} {'timeout':>8} {'sec':>8}")
    prev = Index(set())
    for k in sorted(neg):
        if k > maxk:
            break
        facts = sorted(neg[k])
        t0 = time.time()
        if subsume:
            idx_self = Index(set(facts))
            checked = [f for f in facts if not _redundant(f, idx_self)]
        else:
            checked = facts
        global _K, _BELOW, _END
        _K, _BELOW = k, prev
        _END = time.time() + budget * max(1, len(checked))
        if workers > 1 and len(checked) > 200:
            ctx = mp.get_context('fork')
            with ctx.Pool(workers) as pool:
                res = pool.map(_verify_one, checked, chunksize=64)
        else:
            res = [_verify_one(f) for f in checked]
        ver = sum(1 for r in res if r is True)
        unver = sum(1 for r in res if r is False)
        tmo = sum(1 for r in res if r is None)
        print(f"{k:>3} {len(facts):>9,} {len(checked):>9,} "
              f"{100*(1-len(checked)/len(facts)) if facts else 0:>7.1f}% "
              f"{ver:>9,} {unver:>7,} {tmo:>8,} {time.time()-t0:>8.1f}")
        sys.stdout.flush()
        prev = Index(set(checked))
    return 0


_K = 0
_BELOW: Index = None      # type: ignore
_END = 0.0


def _redundant(f, idx) -> bool:
    mf = mass(f)
    for L in range(1, len(f) + 1):
        for m2, g in idx.by_len.get(L, ()):
            if m2 > mf:
                break
            if g != f and dominates(g, f):
                return True
    return False


def _verify_one(f):
    """True verified, False not verified, None out of budget. Only COUNT and MAJ short-circuit."""
    if mass(f) > POW3[_K] or maj_refutes(f, _K):
        return True
    ok, _ = splits_ok(f, _K, _BELOW, min(_END, time.time() + 30.0))
    return ok


if __name__ == '__main__':
    sys.exit(main())
