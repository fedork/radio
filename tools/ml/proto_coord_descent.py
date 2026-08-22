#!/usr/bin/env python3
"""Prototype and test, against the real persistent oracle, the coordinate-descent / large-
neighborhood-search idea from the 2026-08-22 "this is a packing problem" conversation:

The mass/cap constraint is a multiple-choice knapsack (solved exactly and cheaply by
`exact_candidates`'s (S,X) DP already) -- that part isn't the open problem. What's open is that
the actual quality signal (does this specific combination lead to three solvable children) is a
genuinely joint, non-separable function of all parts together, which is exactly why the per-part
"deficit" order (evidence/deficit_order_and_bestfirst_2026-08-22.txt) landed 1,000x-2,700x behind
the pooled recursive-V model: a separable proxy can't capture a non-separable objective.

Coordinate descent sidesteps this by using the EXPENSIVE, ACCURATE pooled score directly, but only
ever evaluating it on a SMALL BLOCK of parts at a time, holding the rest fixed.

**1-part-at-a-time descent was tried first and fails outright** (see the SUMMARY at the bottom of
a run, or the evidence file): with only one part free, the AND-of-three-children objective's
landscape is rugged enough that greedy descent from a random start gets stuck in a local optimum
well below the true winner's own score, and more restarts (tested to 300, ~19,400 evaluations)
plateau below it rather than closing the gap. **2-parts-at-a-time (block coordinate descent)**
fixes this: holding the other N-2 parts fixed and jointly searching one PAIR's cross product
(still small -- up to ~60x60 -- and cheap) gives the descent enough reach to escape the 1-block
local optima. Measured offline before any oracle calls: 2-block descent matched a known census
winner exactly on the very first restart (1,745 evaluations) with one random seed, and reached the
winner's own score within 30 restarts (54,453 evaluations, 4.3s) with another -- both far cheaper
than scoring the full 7,666-candidate R_0-survivor list, and worlds better than 1-block descent's
complete failure (0/30 real oracle-verified successes, evidence below).

  1. Get any one feasible combination cheaply (rejection sampling over each part's own
     Pareto-admissible option list, checking only the mass/cap arithmetic -- no R_0, no oracle,
     no full enumeration).
  2. Repeat: pick a PAIR of parts, hold the rest fixed, and jointly search that pair's own
     admissible-option cross product (small) for the choice maximizing the full pooled
     recursive-V score of the resulting whole candidate (R_0-checked, the same score that gave
     rank 1/13/85/687 in the tier sample). Cycle through all C(P,2) pairs; stop when a full round
     produces no improvement (a local optimum) or a round cap is hit.
  3. Check the local optimum against the real oracle. If it's not a genuine winner, restart from a
     new random feasible point (a handful of restarts) rather than getting stuck in one basin.

Needs an SSM tunnel to the oracle-serve instance on 127.0.0.1:7777 (see docs/aws-run.md).
"""
import itertools
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
from proto_deficit_bestfirst import part_options, raw_children


def totals(parts, take):
    S = sum(a * b for a, b in take)
    X = sum(a * (m - b) + (n - a) * b for (n, m), (a, b) in zip(parts, take))
    return S, X


def feasible(S, X, C, capc):
    return S <= capc and X <= capc and 0 <= C <= capc


def pooled_score(V, sel, mix, comp, k_child):
    if not (bm.r0(sel, k_child) and bm.r0(mix, k_child) and bm.r0(comp, k_child)):
        return None
    fs = rv.feat(list(sel), k_child); fm = rv.feat(list(mix), k_child); fc = rv.feat(list(comp), k_child)
    vs, vm, vc = V(np.array([fs, fm, fc], np.float32))
    return min(vs, vm, vc)


def random_feasible_start(rng, parts, per_part_opts, mass, capc, max_tries=20000):
    for _ in range(max_tries):
        take = [per_part_opts[i][rng.integers(len(per_part_opts[i]))] for i in range(len(parts))]
        S, X = totals(parts, take)
        C = mass - S - X
        if feasible(S, X, C, capc):
            return take
    return None


def coordinate_descent_1block(rng, parts, per_part_opts, mass, capc, k_child, V, max_rounds, log):
    """1-part-at-a-time descent. Kept for the record: fails outright (0/30 real oracle-verified
    successes on the easiest of the 4 tier-sample endpoints, and 300 restarts / ~19,400
    evaluations plateau below the known winner's own score) -- see the module docstring and
    evidence/coord_descent_2026-08-22.txt. Superseded by the 2-block version below."""
    take = random_feasible_start(rng, parts, per_part_opts, mass, capc)
    if take is None:
        log("  no feasible random start found")
        return None, 0
    evals = 0
    sel, mix, comp = raw_children(parts, take)
    cur = pooled_score(V, sel, mix, comp, k_child)
    if cur is None:
        cur = -1e9  # start is mass-feasible but not R_0-admissible; descent will move away from it
    for rnd in range(max_rounds):
        improved = False
        order = list(range(len(parts)))
        rng.shuffle(order)
        for j in order:
            best_opt = take[j]; best_score = cur
            base = list(take)
            for (a, b) in per_part_opts[j]:
                if (a, b) == take[j]:
                    continue
                cand = base[:j] + [(a, b)] + base[j + 1:]
                S, X = totals(parts, cand)
                C = mass - S - X
                if not feasible(S, X, C, capc):
                    continue
                sel, mix, comp = raw_children(parts, cand)
                sc = pooled_score(V, sel, mix, comp, k_child)
                evals += 1
                if sc is not None and sc > best_score:
                    best_score = sc; best_opt = (a, b)
            if best_opt != take[j]:
                take[j] = best_opt; cur = best_score; improved = True
        if not improved:
            break
    return take, evals


def coordinate_descent(rng, parts, per_part_opts, mass, capc, k_child, V, max_rounds, log):
    """2-block descent: jointly re-optimize one PAIR of parts at a time, holding the rest fixed.
    The pair's own admissible-option cross product is still small (up to ~60x60), so this stays
    cheap, but the larger neighborhood gives the descent enough reach to escape the local optima
    that trap the 1-block version (see coordinate_descent_1block above)."""
    take = random_feasible_start(rng, parts, per_part_opts, mass, capc)
    if take is None:
        log("  no feasible random start found")
        return None, 0
    evals = 0
    sel, mix, comp = raw_children(parts, take)
    cur = pooled_score(V, sel, mix, comp, k_child)
    if cur is None:
        cur = -1e9
    pairs = list(itertools.combinations(range(len(parts)), 2))
    for rnd in range(max_rounds):
        improved = False
        rng.shuffle(pairs)
        for (i, j) in pairs:
            best_opt = (take[i], take[j]); best_score = cur
            base = list(take)
            for oi in per_part_opts[i]:
                for oj in per_part_opts[j]:
                    if oi == take[i] and oj == take[j]:
                        continue
                    cand = list(base); cand[i] = oi; cand[j] = oj
                    S, X = totals(parts, cand)
                    C = mass - S - X
                    if not feasible(S, X, C, capc):
                        continue
                    sel, mix, comp = raw_children(parts, cand)
                    sc = pooled_score(V, sel, mix, comp, k_child)
                    evals += 1
                    if sc is not None and sc > best_score:
                        best_score = sc; best_opt = (oi, oj)
            if best_opt != (take[i], take[j]):
                take[i], take[j] = best_opt; cur = best_score; improved = True
        if not improved:
            break
    return take, evals


def main():
    names = sys.argv[1:] if len(sys.argv) > 1 else ["U000368", "U000535", "U000607", "U000068"]
    C = CORPORA["k7"]
    oracle = TCPOracle(timeout=60)
    t0 = time.time()

    print("training recursive V on k<=6 matched-sampler ...", file=sys.stderr)
    lr, gb = rv.experiment_1([4, 5, 6], 7)
    V = lambda X: lr.predict_proba(X)[:, 1]
    print(f"  done [{time.time()-t0:.0f}s]", file=sys.stderr)

    rng = np.random.default_rng(20260822)
    results = {}
    for name in names:
        k = 5
        k_child = k - 1
        capc = 3 ** k_child
        parts = list(C["states"][name])
        mass = C["meta"][name]["mass"]
        assert mass == sum(n * m for n, m in parts)
        mmax = max(max(p) for p in parts)
        tab = bound_table(k_child, mmax)
        per_part_opts = [part_options(n, m, tab, mmax) for n, m in parts]

        print(f"\n=== {name}: parts={parts} mass={mass}/{3**k} "
              f"per-part option counts={[len(o) for o in per_part_opts]} ===", file=sys.stderr)

        t1 = time.time()
        total_evals = 0
        found = None
        restarts_used = 0
        MAX_RESTARTS = 30
        for restart in range(MAX_RESTARTS):
            restarts_used = restart + 1
            take, evals = coordinate_descent(rng, parts, per_part_opts, mass, capc, k_child, V,
                                              max_rounds=10, log=lambda s: print(s, file=sys.stderr))
            total_evals += evals
            if take is None:
                continue
            sel, mix, comp = raw_children(parts, take)
            ok = True
            for child in (sel, mix, comp):
                if not child:
                    continue
                v = oracle.ask(k_child, list(child))
                if v != "SOLVABLE":
                    ok = False
                    break
            print(f"  restart {restart+1}: local optimum split={take}  evals_so_far={total_evals}  "
                  f"oracle_verdict={'SOLVABLE' if ok else 'not all solvable'}  "
                  f"[{time.time()-t1:.0f}s]", file=sys.stderr)
            if ok:
                found = take
                break
        results[name] = dict(found=found, evals=total_evals, restarts=restarts_used,
                              elapsed=time.time() - t1)
        print(f"  {name}: {'SUCCESS' if found else 'FAILED'} after {restarts_used} restarts, "
              f"{total_evals} pooled-score evaluations, {time.time()-t1:.0f}s", file=sys.stderr)

    print("\n\n=== SUMMARY ===")
    for name, r in results.items():
        print(f"{name}: {'SUCCESS' if r['found'] else 'FAILED'}  restarts={r['restarts']}  "
              f"pooled_score_evals={r['evals']}  split={r['found']}  [{r['elapsed']:.0f}s]")
    print(f"\n[{time.time()-t0:.0f}s total]")


if __name__ == "__main__":
    main()
