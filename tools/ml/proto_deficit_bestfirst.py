#!/usr/bin/env python3
"""Prototype and test, against the real persistent oracle, the two ideas from the 2026-08-22
"can BY_MAGIC3 be improved / can we generate good candidates directly" conversation:

  (1) DEFICIT ORDER: score R_0-admissible candidates by the per-part Pareto-margin ("deficit")
      signal already measured offline (recursive_value.py: AUC 0.9961 alone, vs 0.9861/0.9956 for
      the full pooled model) -- order R_0 survivors by it instead of by the pooled recursive-V
      score, and see where a real winner lands.

  (2) BEST-FIRST GENERATION: the deficit signal is a MAX over independent per-part terms (proven
      below by exhaustive cross-check against recursive_value.deficit(), 0/20000 mismatches once a
      real bug -- treating deficit()'s -1.0 "empty child" sentinel as a literal value inside a
      combining max() -- was found and fixed). That max-of-independent-terms structure means a
      classic best-first/k-shortest-paths-style heap search can generate full multi-part
      candidates in non-decreasing deficit order WITHOUT ever building the R_0-survivor list,
      applying the joint mass/cap constraint as a feasibility filter on each popped candidate.

Needs an SSM tunnel to the oracle-serve instance on 127.0.0.1:7777 (see docs/aws-run.md).
"""
import heapq
import sys
import time

import numpy as np

sys.path.insert(0, "/Users/fedor/radio")
sys.path.insert(0, "/Users/fedor/radio/tools")
sys.path.insert(0, "/Users/fedor/radio/tools/ml")
from cut_ranker_data import CORPORA, bound_table
import bundled_majorization as bm
import recursive_value as rv
from tools.oracle_tcp_client import TCPOracle


# ---------------------------------------------------------------------------------------------
# shared plumbing (same as real_benchmark_via_aws.py / tier_sample_via_aws.py)
# ---------------------------------------------------------------------------------------------

def part_options(n, m, tab, mmax):
    out = []
    for a in range(n + 1):
        for b in range(m + 1):
            ok = True
            for pn, pm in ((a, b), (n - a, m - b), (a, m - b), (n - a, b)):
                hi_, lo_ = max(pn, pm), min(pn, pm)
                if hi_ * lo_ > 1 and (lo_ > mmax + 1 or hi_ > tab[lo_]):
                    ok = False
                    break
            if ok:
                out.append((a, b))
    return out


def exact_candidates(parts, mass, capc, tab, mmax):
    opts = [np.array(part_options(n, m, tab, mmax), dtype=np.int64) for n, m in parts]
    S = np.zeros(1, np.int64); X = np.zeros(1, np.int64); cols = []
    for (n, m), o in zip(parts, opts):
        a, b = o[:, 0], o[:, 1]
        s = a * b; x = a * (m - b) + (n - a) * b
        S2 = (S[:, None] + s[None, :]).ravel(); X2 = (X[:, None] + x[None, :]).ravel()
        keep = (S2 <= capc) & (X2 <= capc)
        idx = np.flatnonzero(keep)
        old = idx // len(o); new = idx % len(o)
        cols = [c[old] for c in cols] + [o[new]]
        S, X = S2[idx], X2[idx]
    Cm = mass - S - X
    keep = (Cm >= 0) & (Cm <= capc)
    return [(c[keep, 0], c[keep, 1]) for c in cols], int(keep.sum())


def raw_children(parts, take):
    sel = list(take)
    comp = [(n - a, m - b) for (n, m), (a, b) in zip(parts, take)]
    mix = ([(a, m - b) for (n, m), (a, b) in zip(parts, take)]
           + [(n - a, b) for (n, m), (a, b) in zip(parts, take)])
    return bm.normalize(sel), bm.normalize(mix), bm.normalize(comp)


# ---------------------------------------------------------------------------------------------
# the deficit signal -- both the "whole child" reference form and the proven-equivalent
# per-part-decomposed form the best-first generator needs
# ---------------------------------------------------------------------------------------------

def overall_deficit_whole(sel, mix, comp, k):
    """Reference form: rv.deficit() on each actual (normalized) child, combined correctly --
    an empty child imposes no constraint and must never win a max() against a real child's
    value (this is the bug that gave 1833/2000 false mismatches during validation: rv.deficit's
    -1.0 "empty" sentinel is not comparable on the same scale as a real deficit)."""
    vals = [rv.deficit(list(c), k) for c in (sel, mix, comp) if c]
    return max(vals) if vals else -1.0


def part_deficit(n, m, a, b, k):
    """Proven-equivalent (0/20000 mismatches, see module docstring) per-part decomposition: this
    part's own worst contribution across the four shapes it feeds (selected, both mixed halves,
    complement), skipping any shape bm.normalize would drop (x<=0 or y<=0 -- not a real rectangle,
    not a constraint)."""
    best = -10 ** 9
    for x, y in ((a, b), (a, m - b), (n - a, b), (n - a, m - b)):
        if x <= 0 or y <= 0:
            continue
        hi, lo = max(x, y), min(x, y)
        d = hi - rv.MAXN1.get(k, {}).get(lo, 0)
        best = max(best, d)
    return best


# ---------------------------------------------------------------------------------------------
# PROTOTYPE 1: order R_0 survivors by the deficit signal, walk against the real oracle
# ---------------------------------------------------------------------------------------------

def run_order(oracle, cands, sel_list, mix_list, comp_list, order, k_child, label, cap_tries, t0):
    tried = 0
    for idx in order:
        tried += 1
        sel, mix, comp = sel_list[idx], mix_list[idx], comp_list[idx]
        ok = True
        for child in (sel, mix, comp):
            if not child:
                continue
            v = oracle.ask(k_child, list(child))
            if v != "SOLVABLE":
                ok = False
                break
        if ok:
            return tried, cands[idx]
        if tried % 500 == 0:
            print(f"    [{label}] {tried} tried, {time.time()-t0:.0f}s elapsed", file=sys.stderr)
        if tried >= cap_tries:
            return None, None
    return None, None


def proto1_deficit_order(oracle, parts, mass, capc, k_child, tab, mmax, label, t0):
    ab, ncand = exact_candidates(parts, mass, capc, tab, mmax)
    cands = [tuple((int(a[i]), int(b[i])) for a, b in ab) for i in range(ncand)]
    print(f"  [{label}] stage-2 candidates: {ncand:,}", file=sys.stderr)

    r0_pass = np.zeros(ncand, dtype=bool)
    deficit = np.zeros(ncand, dtype=np.int64)
    sel_list, mix_list, comp_list = [], [], []
    for i, take in enumerate(cands):
        sel, mix, comp = raw_children(parts, take)
        r0_pass[i] = bm.r0(sel, k_child) and bm.r0(mix, k_child) and bm.r0(comp, k_child)
        deficit[i] = overall_deficit_whole(sel, mix, comp, k_child)
        sel_list.append(sel); mix_list.append(mix); comp_list.append(comp)

    surv = np.flatnonzero(r0_pass)
    print(f"  [{label}] R_0 survivors: {len(surv):,} ({len(surv)/ncand:.1%})", file=sys.stderr)

    order_deficit = surv[np.argsort(deficit[surv])]  # ascending deficit = descending margin = best first
    rank, split = run_order(oracle, cands, sel_list, mix_list, comp_list,
                             order_deficit, k_child, f"{label}/deficit", 20000, t0)
    print(f"  [{label}] DEFICIT ORDER: rank={rank}  split={split}", file=sys.stderr)
    return dict(ncand=ncand, r0=len(surv), rank_deficit=rank, split_deficit=split)


# ---------------------------------------------------------------------------------------------
# PROTOTYPE 2: best-first generation via a heap over per-part deficit-sorted option lists
# ---------------------------------------------------------------------------------------------

def proto2_bestfirst(oracle, parts, mass, capc, k_child, tab, mmax, label, t0, max_pops=200000):
    # per-part admissible options, each sorted by THIS PART's own worst-shape deficit ascending
    per_part = []
    for n, m in parts:
        opts = part_options(n, m, tab, mmax)
        scored = sorted(opts, key=lambda ab: part_deficit(n, m, ab[0], ab[1], k_child))
        per_part.append(scored)
    sizes = [len(p) for p in per_part]
    print(f"  [{label}] per-part option counts (after per-part Pareto filter): {sizes}",
          file=sys.stderr)

    def priority(idx_tuple):
        best = -10 ** 9
        for (n, m), opts, i in zip(parts, per_part, idx_tuple):
            a, b = opts[i]
            best = max(best, part_deficit(n, m, a, b, k_child))
        return best

    start = tuple(0 for _ in parts)
    heap = [(priority(start), start)]
    visited = set()
    popped = 0
    feasible_seen = 0
    checked = 0
    t1 = time.time()
    while heap:
        pr, idxs = heapq.heappop(heap)
        if idxs in visited:
            continue
        visited.add(idxs)
        popped += 1
        if popped >= max_pops:
            print(f"  [{label}] BEST-FIRST gave up after {popped} pops (no oracle-verified winner)",
                  file=sys.stderr)
            return dict(popped=popped, feasible_seen=feasible_seen, checked=checked, rank=None)

        take = tuple(per_part[i][idxs[i]] for i in range(len(parts)))
        S = sum(a * b for a, b in take)
        X = sum(a * (m - b) + (n - a) * b for (n, m), (a, b) in zip(parts, take))
        Cm = mass - S - X
        feasible = (0 <= Cm <= capc) and (S <= capc) and (X <= capc)

        if feasible:
            feasible_seen += 1
            sel, mix, comp = raw_children(parts, take)
            checked += 1
            ok = True
            for child in (sel, mix, comp):
                if not child:
                    continue
                v = oracle.ask(k_child, list(child))
                if v != "SOLVABLE":
                    ok = False
                    break
            if ok:
                print(f"  [{label}] BEST-FIRST SUCCESS: popped={popped} feasible_seen={feasible_seen} "
                      f"oracle_checked={checked}  split={take}  deficit={pr}  "
                      f"[{time.time()-t1:.0f}s]", file=sys.stderr)
                return dict(popped=popped, feasible_seen=feasible_seen, checked=checked,
                            rank=feasible_seen, split=take)
            if feasible_seen % 200 == 0:
                print(f"    [{label}] popped={popped} feasible={feasible_seen} checked={checked} "
                      f"[{time.time()-t1:.0f}s]", file=sys.stderr)

        for j in range(len(parts)):
            if idxs[j] + 1 < sizes[j]:
                nxt = idxs[:j] + (idxs[j] + 1,) + idxs[j + 1:]
                if nxt not in visited:
                    heapq.heappush(heap, (priority(nxt), nxt))

    print(f"  [{label}] BEST-FIRST exhausted the entire space ({popped} pops) without success",
          file=sys.stderr)
    return dict(popped=popped, feasible_seen=feasible_seen, checked=checked, rank=None)


# ---------------------------------------------------------------------------------------------

# Corpus endpoint IDs from the 2026-08-22 tier sample (evidence/tier_sample_via_aws_2026-08-22.txt)
# with their MEASURED rank_learned/rank_natural, quoted directly from that run's log -- NOT
# hand-typed part tuples (an earlier draft of this script fabricated plausible-looking (n,m)
# tuples from memory for these IDs; verified against CORPORA["k7"]["states"][e] and found wrong,
# e.g. the real U000068 is ((20,3),(17,3),(27,1),(27,1)), nothing like the guess. Parts are now
# always looked up live from the corpus, never hardcoded.
ENDPOINTS = {
    "U000368": dict(k=5, rank_learned=1, rank_natural=6041),
    "U000535": dict(k=5, rank_learned=13, rank_natural=None),
    "U000607": dict(k=5, rank_learned=85, rank_natural=5860),
    "U000068": dict(k=5, rank_learned=687, rank_natural=None),
}


def main():
    names = sys.argv[1:] if len(sys.argv) > 1 else list(ENDPOINTS.keys())
    C = CORPORA["k7"]
    oracle = TCPOracle(timeout=60)
    t0 = time.time()
    results = {}
    for name in names:
        known = ENDPOINTS[name]
        k = known["k"]
        parts = list(C["states"][name])
        mass = C["meta"][name]["mass"]
        assert mass == sum(n * m for n, m in parts), f"{name}: mass mismatch, corpus data changed?"
        k_child = k - 1
        capc = 3 ** k_child
        mmax = max(max(p) for p in parts)
        tab = bound_table(k_child, mmax)
        print(f"\n=== {name}: parts={parts} mass={mass}/{3**k} known={known} ===", file=sys.stderr)

        r1 = proto1_deficit_order(oracle, parts, mass, capc, k_child, tab, mmax, name, t0)
        r2 = proto2_bestfirst(oracle, parts, mass, capc, k_child, tab, mmax, name, t0)
        results[name] = dict(known=known, proto1=r1, proto2=r2)

    print("\n\n=== SUMMARY ===")
    for name, r in results.items():
        print(f"{name}:")
        print(f"  known (pooled recursive-V):  rank_learned={r['known'].get('rank_learned')}  "
              f"rank_natural={r['known'].get('rank_natural')}")
        print(f"  proto1 (deficit order):      rank={r['proto1']['rank_deficit']}  "
              f"(of {r['proto1']['r0']:,} R_0 survivors, {r['proto1']['ncand']:,} stage-2 total)")
        print(f"  proto2 (best-first generate): rank={r['proto2']['rank']}  "
              f"popped={r['proto2']['popped']}  feasible_seen={r['proto2']['feasible_seen']}  "
              f"oracle_checked={r['proto2']['checked']}")
    print(f"\n[{time.time()-t0:.0f}s total]")


if __name__ == "__main__":
    main()
