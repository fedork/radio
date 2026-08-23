#!/usr/bin/env python3
"""Prototype and test, against the real persistent oracle, the "concentric round expansion"
search design from the 2026-08-22 conversation.

DESIGN BEING TESTED (single level only -- see the module docstring's last section for what is
NOT tested here). Instead of the current solver's nested nothing-shared-across-segments walk with
an arbitrary CPU-unit cutoff:

  - Split a state's P segments (parts) into P-1 "concentric" segments and 1 "last" segment.
  - Each concentric segment's own admissible option list is pre-sorted by a cheap per-part proxy
    (the deficit signal from evidence/deficit_order_and_bestfirst_2026-08-22.txt -- real, if
    modest, and free at this point since the per-part table is already built for R_0 admissibility).
  - Round r tries every NEW joint combination of the concentric segments' own top-R_i(r) prefixes
    (skipping combinations already tried in an earlier round). R_i(r) grows geometrically with a
    per-segment factor g = G^(1/(P-1)) for a target TOTAL per-round work-growth factor G (default
    2, matching the existing CPU-quantum doubling's spirit) -- the fix for the "growth is also to
    the power of segment count" blowup identified in conversation.
  - For each such combination, the LAST segment is walked IN FULL, but scored with the real,
    accurate pooled recursive-V score (not a proxy) -- cheap here because only that one segment's
    own (small) option list is free, exactly the coordinate-descent "single free block" trick
    (tools/ml/proto_coord_descent.py), restricted to feasible options (same joint mass/cap filter).
  - The metric that matters here is real ORACLE calls (the expensive resource), and at what ROUND
    a success occurs -- the "at what round did we stop" number is the claimed non-arbitrary,
    interpretable stopping criterion this design is meant to provide instead of a CPU-unit count.

Compared against three baselines already measured on the same 4 real endpoints: natural/blind
order (rank_natural), deficit order alone (rank_deficit), and the full pooled-model scan
(rank_learned) -- see evidence/deficit_order_and_bestfirst_2026-08-22.txt.

NOT TESTED HERE: propagating the round/radius DOWN into the recursive verification of the three
children themselves (the "propagate effort to the level below" half of the design). That needs
simulating multiple levels of the AND-OR tree with real oracle calls at each level -- a
substantially bigger undertaking, deferred until the single-level core idea is shown to earn its
keep (see the SUMMARY printed by this script).

Needs an SSM tunnel to the oracle-serve instance on 127.0.0.1:7777 (see docs/aws-run.md).
"""
import itertools
import math
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
from proto_deficit_bestfirst import part_options, raw_children, part_deficit
from proto_coord_descent import totals, feasible, pooled_score


def concentric_round_search(parts, mass, capc, k_child, V, oracle, growth_G=2.0, R0=1,
                             max_rounds=30, outer_order="deficit", log=print):
    P = len(parts)
    outer = list(range(P - 1))
    last = P - 1

    raw_opts = [part_options(n, m, tab_mmax[0], tab_mmax[1]) for n, m in parts]
    if outer_order == "deficit":
        sorted_opts = [
            sorted(raw_opts[i], key=lambda ab: part_deficit(parts[i][0], parts[i][1], ab[0], ab[1], k_child))
            for i in range(P)
        ]
    else:  # "blind" -- natural enumeration order, no per-part scoring at all
        sorted_opts = raw_opts

    sizes = [len(sorted_opts[i]) for i in outer]
    g = growth_G ** (1.0 / max(len(outer), 1))

    tried_combos = set()
    total_pooled_evals = 0
    total_oracle_calls = 0
    t0 = time.time()
    R = [0] * len(outer)
    for rnd in range(1, max_rounds + 1):
        new_R = [min(sizes[i], max(R[i] + 1, math.ceil(R0 * g ** rnd))) for i in range(len(outer))]
        if new_R == R and all(r >= s for r, s in zip(R, sizes)):
            log(f"    round {rnd}: fully exhausted all segments, no success  "
                f"[{time.time()-t0:.0f}s]")
            break
        R = new_R
        ranges = [range(R[i]) for i in range(len(outer))]
        n_new_this_round = 0
        for combo_idx in itertools.product(*ranges):
            if combo_idx in tried_combos:
                continue
            tried_combos.add(combo_idx)
            n_new_this_round += 1
            outer_choice = [sorted_opts[outer[i]][combo_idx[i]] for i in range(len(outer))]

            S_out = sum(a * b for a, b in outer_choice)
            X_out = sum(a * (parts[outer[i]][1] - b) + (parts[outer[i]][0] - a) * b
                        for i, (a, b) in enumerate(outer_choice))

            feasible_last = []
            for (a, b) in sorted_opts[last]:
                take = outer_choice + [(a, b)]
                S, X = totals(parts, take)
                C = mass - S - X
                if feasible(S, X, C, capc):
                    feasible_last.append((a, b))
            if not feasible_last:
                continue

            scored = []
            for (a, b) in feasible_last:
                take = outer_choice + [(a, b)]
                sel, mix, comp = raw_children(parts, take)
                sc = pooled_score(V, sel, mix, comp, k_child)
                total_pooled_evals += 1
                if sc is not None:
                    scored.append((sc, (a, b)))
            scored.sort(key=lambda t: -t[0])

            for sc, (a, b) in scored:
                take = outer_choice + [(a, b)]
                sel, mix, comp = raw_children(parts, take)
                ok = True
                for child in (sel, mix, comp):
                    if not child:
                        continue
                    v = oracle.ask(k_child, list(child))
                    total_oracle_calls += 1
                    if v != "SOLVABLE":
                        ok = False
                        break
                if ok:
                    log(f"    round {rnd}: SUCCESS  R={R}  split={take}  "
                        f"pooled_evals={total_pooled_evals}  oracle_calls={total_oracle_calls}  "
                        f"[{time.time()-t0:.0f}s]")
                    return take, rnd, total_pooled_evals, total_oracle_calls

        log(f"    round {rnd}: R={R}  {n_new_this_round} new combos  "
            f"pooled_evals_so_far={total_pooled_evals}  oracle_calls_so_far={total_oracle_calls}  "
            f"[{time.time()-t0:.0f}s]")

    return None, max_rounds, total_pooled_evals, total_oracle_calls


tab_mmax = None  # set per-endpoint in main()


def main():
    names = sys.argv[1:] if len(sys.argv) > 1 else ["U000368", "U000535", "U000607", "U000068"]
    C = CORPORA["k7"]
    oracle = TCPOracle(timeout=60)
    t0 = time.time()

    print("training recursive V on k<=6 matched-sampler ...", file=sys.stderr)
    lr, gb = rv.experiment_1([4, 5, 6], 7)
    V = lambda X: lr.predict_proba(X)[:, 1]
    print(f"  done [{time.time()-t0:.0f}s]", file=sys.stderr)

    global tab_mmax
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
        tab_mmax = (tab, mmax)

        print(f"\n=== {name}: parts={parts} mass={mass}/{3**k} ===", file=sys.stderr)
        endpoint_result = {}
        for order in ("deficit", "blind"):
            print(f"  --- outer segment order: {order} ---", file=sys.stderr)
            take, rnd, pevals, ocalls = concentric_round_search(
                parts, mass, capc, k_child, V, oracle,
                outer_order=order, log=lambda s: print(s, file=sys.stderr))
            endpoint_result[order] = dict(found=take, round=rnd, pooled_evals=pevals,
                                           oracle_calls=ocalls)
        results[name] = endpoint_result

    print("\n\n=== SUMMARY ===")
    for name, r in results.items():
        print(f"{name}:")
        for order, res in r.items():
            status = "SUCCESS" if res["found"] else "FAILED"
            print(f"  [{order:8s}] {status}  round={res['round']}  "
                  f"pooled_evals={res['pooled_evals']}  oracle_calls={res['oracle_calls']}")
    print(f"\n[{time.time()-t0:.0f}s total]")


if __name__ == "__main__":
    main()
