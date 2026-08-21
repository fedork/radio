#!/usr/bin/env python3
"""Extend the level-transfer value model to a genuine multi-level holdout, and use it
RECURSIVELY: score a candidate split by the trained V of its three children, one level
down, instead of by the parent's own shape.

Two experiments, run in order; the second only means anything if the first separates.

1. LEVEL HOLDOUT (extends evidence/value_level_transfer_2026-08-20.txt from one pair,
   k=4->k=5, to a genuine ladder). Matched sampler, oracle labels, permuted-label control.
   Data: /tmp/rec/labeled2_k{4,5,6,7}.txt, produced by /tmp/rec/label_oracle.py against
   /tmp/rec/oracle_k7 (cold, no cache -- see docs/ml-guided-search.md).

2. RECURSIVE CUT SCORING. The design note's diagnosis: the flat ranker predicts the
   OR-choice from the parent's shape alone and stalls at median rank 76 of 54,014
   (evidence/learned_cut_ranker_2026-08-20.txt). "What determines the winner lives one
   level down." So: for every stage-2 candidate cut of a real census endpoint, compute its
   three actual AND-OR children (selected/mixed/complement, via
   analyze_single_solution_cuts.children) and score the cut by
   min(V(selected), V(mixed), V(complement)) -- the AND semantics, used only to ORDER, never
   to prune. Compare against cut_ranker's flat-shape ranker on the identical stage-2
   candidate set and cost metric, so the two numbers are directly comparable.

   This is also the first honest test of the sampler/real-recursion distribution mismatch
   the evidence file flags: V is trained on synthetic matched states, evaluated on children
   that are actual splits of real forced census endpoints -- a distribution neither the
   the training states nor `analyze_single_solution_cuts` were built to match.

3. WORST CASE FIRST, THEN ORDER (2026-08-21, prompted by a user challenge to experiment 2:
   stratifying experiment 2's own result by exact candidate-set size showed the recursive
   ranker's median rank roughly DOUBLES on the hardest third of endpoints, 146 -> 331, and its
   worst case is 16,886 of up to 130,262 -- barely better than blind on the single hardest
   instance. A ranking, however good on typical cases, can never certify "no solution exists":
   only a SOUND filter can, by shrinking the set that must be exhaustively tried. So: apply
   `R_0` (full-star majorization, `tools/bundled_majorization.r0`, proved in
   docs/theorems/singleton-majorization.md) to every candidate's three children FIRST -- this
   is the real, theorem-backed worst-case bound, entirely independent of any learned model --
   and only THEN rank what survives with the recursive V scorer. Composing them turns out to
   fix both problems: the R_0-survivor count shrinks MORE on the hardest tier, not less (4.7x
   -> 10.6x), and the recursive rank WITHIN the survivors stops degrading with hardness at all
   (correlation 0.001, against 0.129 pre-filter). Full numbers in
   evidence/recursive_value_2026-08-20.txt section 4.

   Two guarantees that must be kept separate: the R_0-survivor count is the real cutoff --
   exhausting it with no success is a sound proof of unsolvability. The rank-within-survivors
   is the unsound, ordering-only signal -- it speeds up finding a witness when one exists, and
   must NEVER be used to stop early or declare unsolvable.
"""
import sys, time
from pathlib import Path
from collections import defaultdict
import csv
import numpy as np
from sklearn.linear_model import LogisticRegression
from sklearn.ensemble import HistGradientBoostingClassifier
from sklearn.preprocessing import StandardScaler
from sklearn.pipeline import make_pipeline
from sklearn.metrics import roc_auc_score

sys.path.insert(0, "/Users/fedor/radio/tools")
sys.path.insert(0, "/Users/fedor/radio/tools/ml")
from analyze_single_solution_cuts import classify, children as split_children
from cut_ranker_data import CORPORA, bound_table, part_arrays, child_masses, pareto_ok
from cut_ranker import four_part

# ---- shared with value_level_transfer.py: pooled, permutation- and part-count-invariant
# features. Duplicated rather than imported because that module runs its own load()/print()
# at import time. Keep the two in sync by hand if either changes.
MAXN1 = defaultdict(dict)
for row in csv.DictReader(open("/Users/fedor/radio/data/pareto_sb.csv")):
    try:
        k, m, n1 = int(row["k"]), int(row["m"]), int(row["n1"])
    except (ValueError, KeyError):
        continue
    if row.get("bound") == "max":
        MAXN1[k][m] = max(MAXN1[k].get(m, 0), n1)


def deficit(parts, k):
    if not parts:
        return -1.0
    return max(n - MAXN1.get(k, {}).get(m, 0) for n, m in ((max(a, b), min(a, b)) for a, b in parts))


def feat(parts, k):
    C = 3.0 ** k
    R = C ** 0.5
    if not parts:
        # the empty/trivial state: always solvable (mass 0), give it the most-solvable corner
        return [0.0, 0.0] + [0.0] * 24 + [-1.0, 0.0, 1.0, 0.0, 0.0]
    n = np.array([max(a, b) for a, b in parts], float)
    m = np.array([min(a, b) for a, b in parts], float)
    area = n * m / C
    nn = n / R
    mm = m / R
    asp = n / np.maximum(m, 1)

    def pool(v):
        s = np.sort(v)
        return [v.sum(), v.mean(), s[-1], s[0], v.std(), s[len(s) // 2]]

    f = [float(len(parts)), area.sum()]
    for v in (area, nn, mm, asp):
        f += pool(v)
    d = deficit(parts, k)
    f += [d / R, float(d > 0), 1.0 - area.sum(), float(np.sort(area)[-1]),
          float(np.sort(area)[-1] / max(area.sum(), 1e-9))]
    return f


NAMES = (["nparts", "mass"]
         + [f"{v}_{p}" for v in ("area", "n", "m", "asp") for p in ("sum", "mean", "max", "min", "std", "med")]
         + ["deficit", "violated", "headroom", "maxarea", "maxshare"])


def load(k):
    X, y = [], []
    for line in open(f"/tmp/rec/labeled3_k{k}.txt"):
        w = line.split()
        rc = int(w[0])
        if rc not in (0, 1):          # drop MAYBE -- never record it as either class
            continue
        v = [int(x) for x in w[1:]]
        parts = [(v[i], v[i + 1]) for i in range(0, len(v), 2)]
        X.append(feat(parts, k))
        y.append(1 if rc == 0 else 0)
    return np.array(X, np.float32), np.array(y)


# =====================================================================================
# EXPERIMENT 1 -- level holdout
# =====================================================================================
def experiment_1(train_levels, test_level):
    Xtr_parts, ytr_parts = [], []
    for k in train_levels:
        Xk, yk = load(k)
        Xtr_parts.append(Xk); ytr_parts.append(yk)
        print(f"  train level k={k}: {len(yk)} states, {yk.mean():.1%} solvable")
    Xtr, ytr = np.concatenate(Xtr_parts), np.concatenate(ytr_parts)
    Xte, yte = load(test_level)
    print(f"  test  level k={test_level}: {len(yte)} states, {yte.mean():.1%} solvable")
    if yte.mean() in (0.0, 1.0) or len(yte) < 20:
        print("  test level degenerate or too small -- skipping"); return None

    def show(t, s):
        print(f"    {t:38s} AUC {roc_auc_score(yte, s):.4f}")

    print("  BASELINES")
    show("mass/cap (lower = solvable)", -Xte[:, NAMES.index("mass")])
    show("per-part Pareto deficit (sound)", -Xte[:, NAMES.index("deficit")])

    print(f"  LEARNED, trained on k={list(train_levels)}, tested on k={test_level}")
    lr = make_pipeline(StandardScaler(), LogisticRegression(max_iter=4000)).fit(Xtr, ytr)
    show("logistic regression", lr.predict_proba(Xte)[:, 1])
    gb = HistGradientBoostingClassifier(max_iter=250, learning_rate=0.08, max_depth=5,
                                        l2_regularization=1.0, random_state=0).fit(Xtr, ytr)
    show("gradient boosting", gb.predict_proba(Xte)[:, 1])
    rs = np.random.default_rng(9)
    yp = ytr.copy(); rs.shuffle(yp)
    ctl = HistGradientBoostingClassifier(max_iter=250, learning_rate=0.08, max_depth=5,
                                         l2_regularization=1.0, random_state=0).fit(Xtr, yp)
    show("PERMUTED-LABEL CONTROL", ctl.predict_proba(Xte)[:, 1])

    und = (Xte[:, NAMES.index("mass")] <= 1.0) & (Xte[:, NAMES.index("deficit")] <= 0)
    print(f"  HARD SUBSET: {und.sum()} of {len(yte)} undecided by info cap + per-part bound, "
          f"{yte[und].mean():.1%} solvable" if und.sum() else "  HARD SUBSET: empty")
    if und.sum() > 30 and 0 < yte[und].mean() < 1:
        show2 = lambda t, s: print(f"    {t:38s} AUC {roc_auc_score(yte[und], s[und]):.4f}")
        show2("mass/cap", -Xte[:, NAMES.index("mass")])
        show2("gradient boosting", gb.predict_proba(Xte)[:, 1])
    return lr, gb


# =====================================================================================
# EXPERIMENT 2 -- recursive cut scoring: V one level down vs the flat shape ranker
# =====================================================================================
def recursive_cut_rank(corpus_name, V, per_endpoint=6000, max_endpoints=None, min_negs=1000):
    """For every forced (single-class) four-part endpoint of `corpus_name`, take the exact
    same stage-2 candidate population cut_ranker.py uses, score each candidate by
    min(V(selected), V(mixed), V(complement)), and report the rank of the true winner --
    directly comparable to cut_ranker.py's evaluate()."""
    C = CORPORA[corpus_name]
    capc, k_child = C["capc"], C["rk"] - 1
    mmax = max(max(max(p) for p in s) for s in C["states"].values())
    tab = bound_table(k_child, mmax)
    endpoints = four_part(corpus_name, forced_only=True)
    if max_endpoints:
        endpoints = endpoints[:max_endpoints]
    costs, ratios, sizes = [], [], []
    t0 = time.time()
    for ei, e in enumerate(endpoints):
        parts, M = C["states"][e], C["meta"][e]["mass"]
        winset = {tuple(t) for t in C["wins"][e]}
        pos = list(C["cls"][e].values())
        ab_pos = [(np.array([p[i][0] for p in pos]), np.array([p[i][1] for p in pos]))
                  for i in range(len(parts))]
        # sample stage-2 negatives the same way cut_ranker.sample_state does
        negs, got = [], 0
        for _ in range(4000):
            if got >= per_endpoint:
                break
            ab = part_arrays(parts, 16384)
            S, X = child_masses(parts, ab)
            Cm = M - S - X
            keep = (S <= capc) & (X <= capc) & (Cm >= 0) & (Cm <= capc)
            if not keep.any():
                continue
            idx = np.flatnonzero(keep)
            abk = [(a[idx], b[idx]) for a, b in ab]
            ok = pareto_ok(parts, abk, k_child, mmax)
            if not ok.any():
                continue
            j = np.flatnonzero(ok)[: per_endpoint - got]
            negs.append([(a[j], b[j]) for a, b in abk]); got += len(j)
        if got < min_negs:
            continue

        def score_batch(ab_batch, B):
            sel_f, mix_f, comp_f = [], [], []
            for i in range(B):
                take = tuple((int(a[i]), int(b[i])) for a, b in ab_batch)
                sel, mix, comp = split_children(parts, take)
                sel_f.append(feat(sel, k_child))
                mix_f.append(feat(mix, k_child))
                comp_f.append(feat(comp, k_child))
            vs = V(np.array(sel_f, np.float32))
            vm = V(np.array(mix_f, np.float32))
            vc = V(np.array(comp_f, np.float32))
            return np.minimum(np.minimum(vs, vm), vc)

        sp = score_batch(ab_pos, len(pos))
        neg_scores = []
        for negb in negs:
            neg_scores.append(score_batch(negb, len(negb[0][0])))
        sn = np.concatenate(neg_scores)
        cost = int((sn >= sp.max()).sum()) + 1
        costs.append(cost); sizes.append(got + len(pos))
        ratios.append(((got + len(pos) + 1) / 2.0) / cost)
        if ei % 10 == 0:
            print(f"    {ei}/{len(endpoints)} endpoints, {time.time()-t0:.0f}s elapsed", file=sys.stderr)
    costs, ratios, sizes = np.array(costs, float), np.array(ratios, float), np.array(sizes)
    return costs, ratios, sizes


def _r0_children(parts, take):
    """The three raw (unpruned) per-part children of a split, in bundled_majorization's own
    `normalize` convention -- NOT `analyze_single_solution_cuts.children`'s `semantic_state`,
    which additionally drops n*m<=1 "unit" parts that `bm.normalize` keeps. Mixing the two
    conventions when calling `bm.r0` would silently test a state `r0` was not built against."""
    import bundled_majorization as bm
    sel = list(take)
    comp = [(n - a, m - b) for (n, m), (a, b) in zip(parts, take)]
    mix = ([(a, m - b) for (n, m), (a, b) in zip(parts, take)]
           + [(n - a, b) for (n, m), (a, b) in zip(parts, take)])
    return bm.normalize(sel), bm.normalize(mix), bm.normalize(comp)


def worst_case_then_order(corpus_name, V, cap=20000, seed=20260821):
    """Experiment 3. For every forced endpoint's EXACT stage-2 candidate set (no sampling --
    `exact_topk.exact_candidates`, capped to a uniform <=`cap` subsample keeping every literal
    winner, for tractability), apply `R_0` to each candidate's three children first -- a sound
    filter, sole source of any worst-case/cutoff guarantee -- then rank ONLY the R_0 survivors
    with the recursive V scorer. Returns per-endpoint (exact stage-2 size, estimated R_0-
    survivor count scaled to the full space, rank of the winner within the R_0 survivors), plus
    a soundness spot-check (R_0 must never drop a literal winner)."""
    import bundled_majorization as bm
    from exact_topk import exact_candidates
    rng = np.random.default_rng(seed)
    C = CORPORA[corpus_name]
    k_child = C["rk"] - 1
    mmax = max(max(max(p) for p in s) for s in C["states"].values())
    tab = bound_table(k_child, mmax)
    forced = four_part(corpus_name, forced_only=True)
    results, winners_dropped = [], 0
    t0 = time.time()
    for ei, e in enumerate(forced):
        parts = C["states"][e]
        ab, ncand = exact_candidates(C, e, tab, mmax)
        if ncand == 0:
            continue
        winset = {tuple(t) for t in C["wins"][e]}
        cuts = np.stack([np.stack([a, b], 1) for a, b in ab], 1)
        iswin = np.array([tuple(map(tuple, cuts[i])) in winset for i in range(ncand)])
        if not iswin.any():
            continue
        if ncand > cap:
            win_idx = np.flatnonzero(iswin)
            keep_other = rng.choice(np.flatnonzero(~iswin), size=cap - len(win_idx), replace=False)
            idx = np.concatenate([win_idx, keep_other])
            ab_s, iswin_s, sample_n = [(a[idx], b[idx]) for a, b in ab], iswin[idx], len(idx)
        else:
            ab_s, iswin_s, sample_n = ab, iswin, ncand

        r0_pass = np.zeros(sample_n, dtype=bool)
        sel_f, mix_f, comp_f = [], [], []
        for i in range(sample_n):
            take = tuple((int(a[i]), int(b[i])) for a, b in ab_s)
            sel, mix, comp = _r0_children(parts, take)
            r0_pass[i] = bm.r0(sel, k_child) and bm.r0(mix, k_child) and bm.r0(comp, k_child)
            sel_f.append(feat(list(sel), k_child)); mix_f.append(feat(list(mix), k_child))
            comp_f.append(feat(list(comp), k_child))

        if iswin_s.any() and not r0_pass[iswin_s].all():
            winners_dropped += int((~r0_pass[iswin_s]).sum())

        score = np.minimum(np.minimum(V(np.array(sel_f, np.float32)), V(np.array(mix_f, np.float32))),
                            V(np.array(comp_f, np.float32)))
        surv = np.flatnonzero(r0_pass)
        surv_win = iswin_s[surv]
        n_r0 = int(r0_pass.sum())
        if surv_win.any():
            surv_score = score[surv]
            rank_within_r0 = int((surv_score >= surv_score[surv_win].max()).sum())
        else:
            rank_within_r0 = n_r0 + 1
        results.append((ncand, n_r0 * (ncand / sample_n), rank_within_r0))
        if ei % 15 == 0:
            print(f"    {ei}/{len(forced)} endpoints, {time.time()-t0:.0f}s elapsed", file=sys.stderr)
    return results, winners_dropped


def main():
    print("=" * 70)
    print("EXPERIMENT 1: level-held-out value model, extended ladder")
    print("=" * 70)
    print("\n-- train k=4, test k=5 (same experiment shape as "
          "evidence/value_level_transfer_2026-08-20.txt, NOT the same numbers: this run's mass "
          "bands are bisected per level rather than fixed at [0.70,1.02], so the sample differs) --")
    experiment_1([4], 5)
    print("\n-- train k<=5, test k=6 (new) --")
    experiment_1([4, 5], 6)
    print("\n-- train k<=6, test k=7 (new -- the literal 'first experiment' spec in "
          "docs/ml-guided-search.md) --")
    r = experiment_1([4, 5, 6], 7)

    if r is None:
        print("\nlevel-7 holdout degenerate; stopping before the recursive experiment.")
        return
    lr, gb = r

    print("\n" + "=" * 70)
    print("EXPERIMENT 2: recursive cut scoring -- V(child) one level down vs flat shape")
    print("=" * 70)
    print("V trained on matched-sampler k<=6 (transfers to synthetic k=7 above); now applied")
    print("to REAL children of REAL census cuts -- a genuine distribution shift, per the")
    print("evidence file's warning. Reports the identical cost/ratio metric as cut_ranker.py.\n")

    print("-- k7 census (children at k=6, inside V's training range) --")
    for tag, model in (("logistic V", lr), ("gradient-boosted V", gb)):
        costs, ratios, sizes = recursive_cut_rank("k7", lambda X: model.predict_proba(X)[:, 1])
        if len(costs):
            print(f"  {len(costs)} forced endpoints, median candidate-set size {int(np.median(sizes)):,}")
            print(f"  recursive {tag:20s} ranker  median cost {np.median(costs):6.0f}"
                  f"   median {np.median(ratios):7.1f}x   worst {ratios.min():6.1f}x")
        else:
            print(f"  {tag}: no endpoint had enough stage-2 negatives")

    print("\n" + "=" * 70)
    print("EXPERIMENT 3: worst case first (R_0, sound), then order (recursive V, unsound)")
    print("=" * 70)
    print("R_0-survivor count is the real cutoff -- a sound proof of unsolvability if exhausted")
    print("with no success. Rank-within-survivors is ordering only: it must never be used to")
    print("stop early. Exact stage-2 enumeration, capped to a uniform <=20000 subsample.\n")
    results, dropped = worst_case_then_order("k7", lambda X: lr.predict_proba(X)[:, 1])
    print(f"\nsoundness check: winners dropped by R_0 across all sampled candidates: {dropped} "
          "(must be 0)")
    ncands = np.array([r[0] for r in results]); scaled_r0 = np.array([r[1] for r in results])
    rank_r0 = np.array([r[2] for r in results])
    print(f"\n{len(results)} endpoints. stage-2 candidates: median {int(np.median(ncands)):,} "
          f"max {ncands.max():,}")
    print(f"R_0 survivors (SOUND worst-case bound): median {int(np.median(scaled_r0)):,} "
          f"max {int(scaled_r0.max()):,}  (shrink vs stage-2: median "
          f"{np.median(ncands/np.maximum(scaled_r0,1)):.1f}x)")
    print(f"rank within R_0 survivors (recursive V order): median {int(np.median(rank_r0)):,} "
          f"worst {int(rank_r0.max()):,}")
    print("\n-- stratified by exact stage-2 candidate-set size (hardness tiers) --")
    order = np.argsort(ncands)
    ncands_s, scaled_r0_s, rank_r0_s = ncands[order], scaled_r0[order], rank_r0[order]
    n = len(ncands_s)
    for lo, hi in [(0, n // 3), (n // 3, 2 * n // 3), (2 * n // 3, n)]:
        sn, sr0, srank = ncands_s[lo:hi], scaled_r0_s[lo:hi], rank_r0_s[lo:hi]
        print(f"  stage-2 in [{sn.min():,},{sn.max():,}] (n={hi-lo}): R_0 survivors median "
              f"{int(np.median(sr0)):,} max {int(sr0.max()):,}  |  rank median "
              f"{int(np.median(srank)):,} worst {int(srank.max()):,}")
    print(f"\ncorrelation(stage-2 size, R_0-survivor count) = {np.corrcoef(ncands, scaled_r0)[0,1]:.3f}")
    print(f"correlation(stage-2 size, rank-within-R_0)    = {np.corrcoef(ncands, rank_r0)[0,1]:.3f}")


if __name__ == "__main__":
    main()
