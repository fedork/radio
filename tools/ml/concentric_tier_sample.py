#!/usr/bin/env python3
"""Scale up the concentric-round-search validation from n=4 to a real sample.

evidence/concentric_round_search_2026-08-22.txt validated the design on exactly 4 endpoints (all
4 succeeded, including both where block coordinate descent failed). That is not enough to trust
as a general result. This script samples ADDITIONAL endpoints from the same k7-census tiers
(2-winner: hardest-mass-biased, since 131 candidates make that a real bias; 4-winner: unbiased,
since only 22 exist total) and runs the same real-oracle-verified concentric round search on each,
using "deficit" as the outer-segment order -- picked pragmatically (marginally ahead in aggregate
on the n=4 comparison, simple, no need to special-case levels the way radiobase.c's own BY_MAGIC3
selection does) rather than because it was shown to be the right choice; the segment-order
question is explicitly not what this run is trying to settle.

Needs an SSM tunnel to the oracle-serve instance on 127.0.0.1:7777 (see docs/aws-run.md).
"""
import sys
import time

import numpy as np

sys.path.insert(0, "/Users/fedor/radio")
sys.path.insert(0, "/Users/fedor/radio/tools")
sys.path.insert(0, "/Users/fedor/radio/tools/ml")
from cut_ranker import four_part
from cut_ranker_data import CORPORA, bound_table
import proto_concentric_rounds as pcr
import recursive_value as rv
from tools.oracle_tcp_client import TCPOracle

ALREADY_TESTED = {"U000368", "U000535", "U000607", "U000068"}


def main():
    corpus = sys.argv[1] if len(sys.argv) > 1 else "k7"
    n_per_tier = int(sys.argv[2]) if len(sys.argv) > 2 else 6
    max_rounds = int(sys.argv[3]) if len(sys.argv) > 3 else 22

    C = CORPORA[corpus]
    forced = four_part(corpus, forced_only=True)
    tier2 = [e for e in forced if len(C["wins"][e]) == 2 and e not in ALREADY_TESTED]
    tier4 = [e for e in forced if len(C["wins"][e]) == 4 and e not in ALREADY_TESTED]

    rng = np.random.default_rng(20260822)

    def hardest(pool, n):
        by_mass = sorted(pool, key=lambda e: -C["meta"][e]["mass"])
        top = by_mass[:max(n * 3, n)]
        idx = rng.choice(len(top), min(n, len(top)), replace=False)
        return [top[i] for i in idx]

    sample2 = [("2-winner", e) for e in hardest(tier2, n_per_tier)]
    sample4 = [("4-winner", e) for e in hardest(tier4, n_per_tier)]
    endpoints = sample2 + sample4
    print(f"corpus {corpus}: tier2 pool {len(tier2)} (excl. already-tested), "
          f"tier4 pool {len(tier4)}; sampling {len(endpoints)} new endpoints", file=sys.stderr)

    t0 = time.time()
    print("training recursive V on k<=6 matched-sampler ...", file=sys.stderr)
    lr, gb = rv.experiment_1([4, 5, 6], 7)
    V = lambda X: lr.predict_proba(X)[:, 1]
    print(f"  done [{time.time()-t0:.0f}s]", file=sys.stderr)

    oracle = TCPOracle(timeout=60)
    results = []
    for tier, name in endpoints:
        k = 5
        k_child = k - 1
        capc = 3 ** k_child
        parts = list(C["states"][name])
        mass = C["meta"][name]["mass"]
        assert mass == sum(n_ * m_ for n_, m_ in parts)
        mmax = max(max(p) for p in parts)
        tab = bound_table(k_child, mmax)
        pcr.tab_mmax = (tab, mmax)

        print(f"\n=== {tier} {name}: parts={parts} mass={mass}/{3**k} "
              f"known_winners={len(C['wins'][name])} ===", file=sys.stderr)
        t1 = time.time()
        take, rnd, pevals, ocalls = pcr.concentric_round_search(
            parts, mass, capc, k_child, V, oracle, outer_order="deficit", max_rounds=max_rounds,
            log=lambda s: print(s, file=sys.stderr))
        status = "SUCCESS" if take else "FAILED"
        dt = time.time() - t1
        print(f"{tier} {name}: {status} round={rnd} pooled_evals={pevals} oracle_calls={ocalls} "
              f"split={take}  [{dt:.0f}s]", file=sys.stderr)
        results.append(dict(tier=tier, name=name, mass=mass, status=status, round=rnd,
                             pooled_evals=pevals, oracle_calls=ocalls, elapsed=dt))

    print("\n\n=== SUMMARY ===")
    for tier in ("2-winner", "4-winner"):
        rs = [r for r in results if r["tier"] == tier]
        n_success = sum(1 for r in rs if r["status"] == "SUCCESS")
        print(f"\n{tier}: {n_success}/{len(rs)} succeeded")
        for r in rs:
            print(f"  {r['name']}: {r['status']}  round={r['round']}  "
                  f"oracle_calls={r['oracle_calls']}  [{r['elapsed']:.0f}s]")
        succ = [r for r in rs if r["status"] == "SUCCESS"]
        if succ:
            calls = np.array([r["oracle_calls"] for r in succ])
            rounds = np.array([r["round"] for r in succ])
            print(f"  oracle_calls: median={int(np.median(calls))}  "
                  f"min={calls.min()}  max={calls.max()}")
            print(f"  round: median={int(np.median(rounds))}  min={rounds.min()}  max={rounds.max()}")

    print(f"\n[{time.time()-t0:.0f}s total]")


if __name__ == "__main__":
    main()
