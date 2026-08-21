#!/usr/bin/env python3
"""Systematic sample of the k7 census's 2-winner and 4-winner tiers, using the persistent
oracle-serve instance's `enumerate` command for exact ground truth instead of walking candidates
one at a time against a solver (this thread's earlier method, which cost 40-60+ minutes per
state). One `enumerate` call gets the COMPLETE winner set and the exact R_0-admissible count in
one round trip; ranking the recursive-V scorer against that ground truth is then pure local
Python, no further oracle calls needed.

**NOT YET PRACTICAL AS WRITTEN.** `enumerate` was measured 2026-08-22 to run 10+ minutes with no
result on an ordinary k7 4-part census endpoint (killed, not timed out -- it was making real
progress, just too slowly): its raw mixed-radix walk checks the cap/R_0 bound only at the deepest
leaf, never on a partial sub-tree, so cost is the RAW combinatorial size regardless of how
selective the bound is (see radio_oracle.c's own comment on this, and
evidence/real_benchmark_via_aws_oracle_2026-08-22.txt). This script is kept because the APPROACH
is right and the code is correct for where `enumerate` is actually fast (validated through k<=6);
using it at k=7 needs either an incrementally-pruned `enumerate` (not built) or, for now,
tools/ml/real_benchmark_via_aws.py's method (stage-2 candidates + R_0 + recursive-V order + plain
per-candidate queries) instead -- also note that CORPORA["k7"]'s endpoints' real parent level is
`C["rk"]` (5), not literally 7; querying at the corpus's label instead of `C["rk"]` gives
internally-consistent, wrong answers with no warning.

Needs an SSM tunnel to the oracle-serve instance already open on 127.0.0.1:7777 -- see
docs/aws-run.md for the current instance and tunnel command.
"""
import sys, time
import numpy as np

sys.path.insert(0, "/Users/fedor/radio")
sys.path.insert(0, "/Users/fedor/radio/tools")
sys.path.insert(0, "/Users/fedor/radio/tools/ml")
from cut_ranker import four_part
from cut_ranker_data import CORPORA, bound_table
import bundled_majorization as bm
import recursive_value as rv
from tools.oracle_tcp_client import TCPOracle


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


def exact_stage2_candidates(parts, mass, capc, tab, mmax):
    """Same construction as exact_topk.py -- cap+per-part-Pareto-bound candidates, exact, no
    sampling."""
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


def main():
    n_per_tier = int(sys.argv[1]) if len(sys.argv) > 1 else 8
    t0 = time.time()

    print("training recursive V on k<=6 matched-sampler ...", file=sys.stderr)
    lr, gb = rv.experiment_1([4, 5, 6], 7)
    V = lambda X: lr.predict_proba(X)[:, 1]
    print(f"  done [{time.time()-t0:.0f}s]", file=sys.stderr)

    C = CORPORA["k7"]; capc = C["capc"]; rk = C["rk"]; k_child = rk - 1
    mmax = max(max(max(p) for p in s) for s in C["states"].values())
    tab = bound_table(k_child, mmax)
    forced = four_part("k7", forced_only=True)

    tier2 = [e for e in forced if len(C["wins"][e]) == 2]
    tier4 = [e for e in forced if len(C["wins"][e]) == 4]
    rng = np.random.default_rng(20260822)
    sample2 = rng.choice(len(tier2), min(n_per_tier, len(tier2)), replace=False)
    sample4 = rng.choice(len(tier4), min(n_per_tier, len(tier4)), replace=False)
    endpoints = [("2-winner", tier2[i]) for i in sample2] + [("4-winner", tier4[i]) for i in sample4]
    print(f"sampling {len(endpoints)} endpoints ({len(sample2)} from 2-winner tier, "
          f"{len(sample4)} from 4-winner tier)", file=sys.stderr)

    oracle = TCPOracle(timeout=300)
    results = []
    for tier, e in endpoints:
        parts = C["states"][e]; mass = C["meta"][e]["mass"]
        t1 = time.time()

        # exact ground truth from the AWS oracle -- one call, complete winner set.
        # NOTE: rk is the endpoint's REAL parent level (5 for the "k7" corpus), not the corpus's
        # literal name -- see the module docstring's 2026-08-22 caveat.
        winners, summary = oracle.enumerate(rk, parts)
        enum_s = time.time() - t1
        if not winners:
            print(f"  {tier} {e}: enumerate found 0 winners -- census disagreement, skipping",
                  file=sys.stderr)
            continue

        # exact stage-2 candidate set, local, no oracle calls
        ab, ncand = exact_stage2_candidates(parts, mass, capc, tab, mmax)
        cands = [tuple((int(a[i]), int(b[i])) for a, b in ab) for i in range(ncand)]

        # `enumerate`'s WINNER lines report positions in ITS OWN internally re-sorted part order
        # (radio_oracle.c: enumerate_winning_splits sort1()s a local copy before walking), not the
        # census's original part order -- comparing raw positional tuples would silently mismatch
        # even on a true match. Match on the canonical SELECTED-CHILD state instead: two
        # (part-index -> (a,b)) assignments that produce the same multiset of selected rectangles
        # are the same split regardless of index bookkeeping.
        winner_selected = {bm.normalize(w) for w in winners}

        sel_f, mix_f, comp_f = [], [], []
        sel_canon = []
        for take in cands:
            sel, mix, comp = raw_children(parts, take)
            sel_canon.append(sel)
            sel_f.append(rv.feat(list(sel), k_child)); mix_f.append(rv.feat(list(mix), k_child))
            comp_f.append(rv.feat(list(comp), k_child))
        score = np.minimum(np.minimum(V(np.array(sel_f, np.float32)), V(np.array(mix_f, np.float32))),
                            V(np.array(comp_f, np.float32)))

        winner_idx = [i for i, sel in enumerate(sel_canon) if sel in winner_selected]
        found_selected = {sel_canon[i] for i in winner_idx}
        missing = [w for w in winner_selected if w not in found_selected]

        if winner_idx:
            best_score = score[winner_idx].max()
            rank = int((score >= best_score).sum())
        else:
            rank = None

        results.append(dict(tier=tier, e=e, mass=mass, checked=summary.get("checked"),
                             admissible=summary.get("admissible"), n_winners_enum=len(winners),
                             stage2_size=ncand, missing_from_stage2=len(missing), rank=rank,
                             enum_s=enum_s))
        print(f"  {tier} {e}: enum={summary} stage2={ncand:,} rank={rank} "
              f"missing_from_stage2={len(missing)}  [{time.time()-t1:.0f}s]", file=sys.stderr)

    print(f"\n=== SUMMARY ({time.time()-t0:.0f}s total) ===")
    for tier in ("2-winner", "4-winner"):
        rs = [r for r in results if r["tier"] == tier and r["rank"] is not None]
        if not rs:
            print(f"{tier}: no usable results")
            continue
        ranks = np.array([r["rank"] for r in rs])
        sizes = np.array([r["stage2_size"] for r in rs])
        print(f"{tier}: n={len(rs)}  median stage2 size {int(np.median(sizes)):,}  "
              f"median rank {int(np.median(ranks)):,}  worst {int(ranks.max()):,}  "
              f"median selectivity {np.median((sizes/2)/np.maximum(ranks,1)):.1f}x")
    n_missing = sum(1 for r in results if r["missing_from_stage2"] > 0)
    if n_missing:
        print(f"\nNOTE: {n_missing}/{len(results)} endpoints had a winner missing from the "
              f"stage-2 candidate set (violates the per-part Pareto bound but not the raw walk) "
              f"-- those ranks are computed against the OTHER winner(s) still in stage-2.")


if __name__ == "__main__":
    main()
