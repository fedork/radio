#!/usr/bin/env python3
"""Systematic sample across the k7 census's 2-winner and 4-winner four-part endpoint tiers,
via the persistent oracle-serve instance -- the follow-up to
evidence/real_benchmark_via_aws_oracle_2026-08-22.txt's single-endpoint result, now run at the
scale originally asked for ("focus on 4-part states and among them those with 2 or 4 solutions").

Method per sampled endpoint (same as tools/ml/real_benchmark_via_aws.py, generalized to a batch
and with a mandatory natural-order control added -- see the trivial-state-trap note in the
evidence file above for why the control is not optional):

  1. exact stage-2 candidates (cap + per-part-Pareto bound), subsampled to `cand_cap` if larger.
  2. R_0-filter the subsample; score survivors by recursive-V (min over selected/mixed/complement).
  3. walk LEARNED order (R_0 survivors, sorted by score) against the real oracle until success.
  4. walk NATURAL order (the identical unfiltered subsample, its original enumeration order)
     against the real oracle until success.
  5. record both ranks; selectivity = natural_rank / learned_rank.

Uses C["rk"] (the endpoint's real parent level, 5 for "k7"/6 for "k8"), NOT the corpus's literal
name -- see the 2026-08-22 caveat in real_benchmark_via_aws.py and census_sample_via_aws_oracle.py.

Needs an SSM tunnel to the oracle-serve instance already open on 127.0.0.1:7777 -- see
docs/aws-run.md for the current instance and tunnel command.

Usage: tools/ml/tier_sample_via_aws.py [corpus=k7] [n_per_tier=8] [cand_cap=20000] [natural_cap=8000]
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


def main():
    corpus = sys.argv[1] if len(sys.argv) > 1 else "k7"
    n_per_tier = int(sys.argv[2]) if len(sys.argv) > 2 else 8
    # cand_cap default is set well above this corpus's typical true stage-2 size (~50-60k for a
    # k7 4-part endpoint) so ordinary endpoints are ranked over their FULL true candidate pool,
    # never a random subsample -- see the 2026-08-22 pilot note above run_order's call site for
    # why subsampling before checking winner-membership silently erases hard (few-winner) cases.
    cand_cap = int(sys.argv[3]) if len(sys.argv) > 3 else 150000
    natural_cap = int(sys.argv[4]) if len(sys.argv) > 4 else 8000
    t0 = time.time()

    print(f"training recursive V on k<=6 matched-sampler ...", file=sys.stderr)
    lr, gb = rv.experiment_1([4, 5, 6], 7)
    V = lambda X: lr.predict_proba(X)[:, 1]
    print(f"  done [{time.time()-t0:.0f}s]", file=sys.stderr)

    C = CORPORA[corpus]; capc = C["capc"]; rk = C["rk"]; k_child = rk - 1
    mmax = max(max(max(p) for p in s) for s in C["states"].values())
    tab = bound_table(k_child, mmax)
    forced = four_part(corpus, forced_only=True)

    tier2 = [e for e in forced if len(C["wins"][e]) == 2]
    tier4 = [e for e in forced if len(C["wins"][e]) == 4]
    print(f"corpus {corpus}: rk={rk} (real parent level, not the corpus label)  "
          f"tier2={len(tier2)} endpoints  tier4={len(tier4)} endpoints", file=sys.stderr)

    rng = np.random.default_rng(20260822)
    # deliberately favor the HARDEST (highest-mass) endpoints in each tier, not a uniform sample --
    # see the trivial-state-trap note: an arbitrary low-mass endpoint proves nothing about ordering.
    def hardest(pool, n):
        by_mass = sorted(pool, key=lambda e: -C["meta"][e]["mass"])
        top = by_mass[:max(n * 3, n)]
        idx = rng.choice(len(top), min(n, len(top)), replace=False)
        return [top[i] for i in idx]

    sample2 = [("2-winner", e) for e in hardest(tier2, n_per_tier)]
    sample4 = [("4-winner", e) for e in hardest(tier4, n_per_tier)]
    endpoints = sample2 + sample4
    print(f"sampling {len(endpoints)} endpoints, biased toward highest mass in each tier",
          file=sys.stderr)

    oracle = TCPOracle(timeout=60)
    results = []
    for tier, e in endpoints:
        parts = C["states"][e]; mass = C["meta"][e]["mass"]
        t1 = time.time()
        ab, ncand = exact_candidates(parts, mass, capc, tab, mmax)
        true_ncand = ncand

        # NOTE (found 2026-08-22 pilot): randomly subsampling the true stage-2 pool down to
        # cand_cap BEFORE checking for known winners can silently drop every literal census
        # winner from the pool by chance (retention ~cand_cap/true_ncand per winner, so with a
        # handful of winners and a ~1/3 retention rate the miss probability is not small) --
        # producing rank_learned=None/rank_natural=None that look like "no result" but are really
        # "the sampled pool never contained a solution," not evidence about the ordering. Default
        # cand_cap is set high enough that ordinary endpoints in this corpus (~50-60k true
        # candidates) are never subsampled; only a genuine outlier gets truncated, and that case
        # is flagged below rather than silently accepted.
        if ncand > cand_cap:
            idx = rng.choice(ncand, size=cand_cap, replace=False)
            ab = [(a[idx], b[idx]) for a, b in ab]
            ncand = cand_cap
            print(f"  {tier} {e}: WARNING stage2 truncated {true_ncand:,} -> {cand_cap:,} "
                  f"(raise cand_cap to avoid this)", file=sys.stderr)

        cands = [tuple((int(a[i]), int(b[i])) for a, b in ab) for i in range(ncand)]
        known_present = sum(1 for w in C["wins"][e] if w in set(cands))
        r0_pass = np.zeros(ncand, dtype=bool)
        sel_f, mix_f, comp_f = [], [], []
        sel_list, mix_list, comp_list = [], [], []
        for i, take in enumerate(cands):
            sel, mix, comp = raw_children(parts, take)
            r0_pass[i] = bm.r0(sel, k_child) and bm.r0(mix, k_child) and bm.r0(comp, k_child)
            sel_f.append(rv.feat(list(sel), k_child)); mix_f.append(rv.feat(list(mix), k_child))
            comp_f.append(rv.feat(list(comp), k_child))
            sel_list.append(sel); mix_list.append(mix); comp_list.append(comp)

        surv = np.flatnonzero(r0_pass)
        vs = V(np.array([sel_f[i] for i in surv], np.float32))
        vm = V(np.array([mix_f[i] for i in surv], np.float32))
        vc = V(np.array([comp_f[i] for i in surv], np.float32))
        score = np.minimum(np.minimum(vs, vm), vc)
        order_learned = surv[np.argsort(-score)]
        order_natural = np.arange(ncand)  # the pool's own enumeration order, unfiltered by R_0

        rank_l, split_l = run_order(oracle, cands, sel_list, mix_list, comp_list,
                                     order_learned, k_child, "learned", cand_cap, t1)
        rank_n, split_n = run_order(oracle, cands, sel_list, mix_list, comp_list,
                                     order_natural, k_child, "natural", natural_cap, t1)

        agree = (split_l == split_n) if (split_l and split_n) else None
        results.append(dict(tier=tier, e=e, mass=mass, ncand=ncand, r0=int(r0_pass.sum()),
                             rank_learned=rank_l, rank_natural=rank_n, agree=agree,
                             known_winners=len(C["wins"][e]), known_present=known_present))
        print(f"  {tier} {e}: mass={mass}/{C['cap']} stage2={ncand:,} "
              f"(known winners in pool: {known_present}/{len(C['wins'][e])}) "
              f"R0={int(r0_pass.sum()):,} rank_learned={rank_l} rank_natural={rank_n} agree={agree}  "
              f"[{time.time()-t1:.0f}s]", file=sys.stderr)

    print(f"\n=== SUMMARY, corpus={corpus} ({time.time()-t0:.0f}s total) ===")
    for tier in ("2-winner", "4-winner"):
        rs = [r for r in results if r["tier"] == tier and r["rank_learned"] is not None
              and r["rank_natural"] is not None]
        skipped = [r for r in results if r["tier"] == tier and
                   (r["rank_learned"] is None or r["rank_natural"] is None)]
        print(f"\n{tier}: n={len(rs)} usable "
              f"({len(skipped)} skipped -- learned or natural order hit its try-cap)")
        if skipped:
            print(f"  NOTE: {len(skipped)}/{len(rs)+len(skipped)} endpoints in this tier did not "
                  f"resolve within the try-cap on at least one order -- excluded from the medians "
                  f"below, not counted as zero or as success.")
        if not rs:
            continue
        rl = np.array([r["rank_learned"] for r in rs])
        rn = np.array([r["rank_natural"] for r in rs])
        sel_ratio = rn / rl
        n_agree = sum(1 for r in rs if r["agree"])
        print(f"  median rank_learned={int(np.median(rl)):,}  median rank_natural={int(np.median(rn)):,}")
        print(f"  median selectivity (natural/learned) = {np.median(sel_ratio):.1f}x  "
              f"(min {sel_ratio.min():.1f}x, max {sel_ratio.max():.1f}x)")
        print(f"  both orders found the SAME split: {n_agree}/{len(rs)}")

    print(f"\nendpoints skipped entirely (0 known winners in this corpus's own census, or "
          f"stage-2 empty): "
          f"{sum(1 for r in results if r['ncand']==0)}")


if __name__ == "__main__":
    main()
