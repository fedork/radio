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


if __name__ == "__main__":
    main()
