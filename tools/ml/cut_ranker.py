#!/usr/bin/env python3
"""Train and evaluate a learned ranker for state -> winning cut.

Reports SELECTIVITY: search candidates in model-score order until a winner is hit; cost is the
number tried.  A blind order costs about (N+1)/(P+1).  The ratio of those is what the model buys.
Needs numpy + scikit-learn in .venv -- see tools/ml/README.md.  Every claim it prints is grouped by
state, quoted against a stated candidate set, and shipped with a permuted-label control.
"""
import sys, time
from pathlib import Path
import numpy as np
from sklearn.linear_model import LogisticRegression
from sklearn.ensemble import HistGradientBoostingClassifier
from sklearn.preprocessing import StandardScaler
from sklearn.pipeline import make_pipeline
from sklearn.model_selection import GroupKFold

sys.path.insert(0, str(Path(__file__).resolve().parent))
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from cut_ranker_data import CORPORA, features, pareto_ok, part_arrays, child_masses

NAMES = sorted(["X_over_cap","max_pure_over_cap","min_pure_over_cap","margin_over_cap",
                "pure_gap_over_cap","parent_occupancy","diag_mean","diag_max","diag_min",
                "offhalf_mean","offhalf_max","offhalf_u_mean","sliver_frac","whole_frac",
                "pureA_parts","pureA_bigrect","pureA_top2","pureA_aspect",
                "pureB_parts","pureB_bigrect","pureB_top2","pureB_aspect",
                "mixed_parts","mixed_bigrect","mixed_top2","mixed_aspect"])


def sample_state(corpus, e, n_neg):
    """Positives (one per winning class) and stage-2 negatives for one endpoint."""
    C = CORPORA[corpus]
    capc, k_child = C["capc"], C["rk"] - 1
    mmax = max(max(max(p) for p in s) for s in C["states"].values())
    parts, M = C["states"][e], C["meta"][e]["mass"]
    pos = list(C["cls"][e].values())
    ab_pos = [(np.array([p[i][0] for p in pos]), np.array([p[i][1] for p in pos]))
              for i in range(len(parts))]
    _, fp = features(parts, ab_pos, capc, M, mmax)
    negs, got = [], 0
    for _ in range(4000):
        if got >= n_neg:
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
        j = np.flatnonzero(ok)[: n_neg - got]
        abk = [(a[j], b[j]) for a, b in abk]
        _, fn = features(parts, abk, capc, M, mmax)
        negs.append(fn); got += len(j)
    return fp, (np.concatenate(negs) if negs else np.zeros((0, len(NAMES)), np.float32))


def four_part(corpus, forced_only=False):
    C = CORPORA[corpus]
    return sorted(e for e in C["states"] if C["meta"][e]["parts"] == 4
                  and (not forced_only or len(C["cls"][e]) == 1))


def training_set(corpus, per_state=600):
    X, y = [], []
    for e in four_part(corpus):
        fp, fn = sample_state(corpus, e, per_state)
        X.append(fp); y.append(np.ones(len(fp), np.int8))
        if len(fn):
            X.append(fn); y.append(np.zeros(len(fn), np.int8))
    return np.concatenate(X), np.concatenate(y)


def evaluate(model, tests, idx=None):
    cost, N = [], []
    for fp, fn in tests:
        if len(fn) < 1000:
            continue
        a, b = (fp[:, idx], fn[:, idx]) if idx is not None else (fp, fn)
        sp, sn = model.predict_proba(a)[:, 1], model.predict_proba(b)[:, 1]
        cost.append(int((sn >= sp.max()).sum()) + 1); N.append(len(fn))
    cost, N = np.array(cost, float), np.array(N, float)
    return cost, ((N + 1) / 2.0) / cost


def line(tag, cost, ratio):
    print(f"  {tag:38s} median cost {np.median(cost):6.0f}   median {np.median(ratio):7.1f}x"
          f"   worst {ratio.min():6.1f}x   censored {100*(cost==1).mean():3.0f}%")


def main():
    t0 = time.time()
    print("training on the k=7 corpus only ...")
    Xtr, ytr = training_set("k7")
    print(f"  {Xtr.shape[0]:,} rows, {int(ytr.sum()):,} positives  [{time.time()-t0:.0f}s]")

    forced8 = four_part("k8", forced_only=True)
    sample = forced8[:: max(1, len(forced8) // 120)][:120]
    tests = [sample_state("k8", e, 6000) for e in sample]
    print(f"\nheld-out test: {len(tests)} FORCED k=8 states, 6000 stage-2 candidates each")
    print("(different state AND different level from anything trained on)\n")

    lr = make_pipeline(StandardScaler(), LogisticRegression(max_iter=2000, class_weight="balanced"))
    lr.fit(Xtr, ytr)
    line("logistic regression, 26 features", *evaluate(lr, tests))

    gb = HistGradientBoostingClassifier(max_iter=200, learning_rate=0.1, max_depth=6,
                                        l2_regularization=1.0, random_state=0)
    gb.fit(Xtr, ytr)
    line("gradient boosting, 26 features", *evaluate(gb, tests))

    rs = np.random.default_rng(5)
    yp = ytr.copy(); rs.shuffle(yp)
    ctl = HistGradientBoostingClassifier(max_iter=200, learning_rate=0.1, max_depth=6,
                                         l2_regularization=1.0, random_state=0)
    ctl.fit(Xtr, yp)
    line("PERMUTED-LABEL CONTROL (must be ~1x)", *evaluate(ctl, tests))

    print("\nhow small can the rule be?")
    for tag, feats in (
        ("1 feature  diag_mean", ["diag_mean"]),
        ("3 features diag,pure_gap,max_pure",
         ["diag_mean", "pure_gap_over_cap", "max_pure_over_cap"]),
        ("5 features +diag_max,pureB_top2",
         ["diag_mean", "pure_gap_over_cap", "max_pure_over_cap", "diag_max", "pureB_top2"]),
        ("8 features +offhalf,margin,mixed_big",
         ["diag_mean", "pure_gap_over_cap", "max_pure_over_cap", "diag_max", "pureB_top2",
          "offhalf_mean", "margin_over_cap", "mixed_bigrect"]),
    ):
        idx = [NAMES.index(f) for f in feats]
        m = make_pipeline(StandardScaler(),
                          LogisticRegression(max_iter=2000, class_weight="balanced"))
        m.fit(Xtr[:, idx], ytr)
        line(tag, *evaluate(m, tests, idx))

    print("\nlearning curve — how many training states are actually needed?")
    states = four_part("k7")
    for frac in (0.05, 0.1, 0.25, 0.5, 1.0):
        keep = set(np.random.default_rng(3).choice(
            len(states), max(2, int(len(states) * frac)), replace=False).tolist())
        X, y = [], []
        for i, e in enumerate(states):
            if i not in keep:
                continue
            fp, fn = sample_state("k7", e, 600)
            X.append(fp); y.append(np.ones(len(fp), np.int8))
            if len(fn):
                X.append(fn); y.append(np.zeros(len(fn), np.int8))
        m = make_pipeline(StandardScaler(),
                          LogisticRegression(max_iter=2000, class_weight="balanced"))
        m.fit(np.concatenate(X), np.concatenate(y))
        cost, ratio = evaluate(m, tests)
        print(f"  {int(len(states)*frac):4d} training states   median {np.median(ratio):7.1f}x")
    print(f"\n[{time.time()-t0:.0f}s]")


if __name__ == "__main__":
    main()
