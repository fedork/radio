#!/usr/bin/env python3
"""Build a learning-to-rank dataset for state -> winning cut.

FRAMING.  This is not a 624-example problem.  The unit is (state, candidate cut), the label is
"is this cut a winner", and the census enumerates *every* winner of every endpoint, so unrecorded
cap-feasible cuts are clean negatives.  That gives ~27k positives and effectively unlimited
negatives (~1e7 cap-feasible cuts per state).  The scarce axis is STATES (2,503), which is what
bounds generalization -- so every split below is grouped by state, and the headline test is
cross-level, since using this at k=9 means extrapolating off the levels we trained on.

DENOMINATOR.  Selectivity is only meaningful against a stated candidate set.  Two are built:
  stage1 = information cap on all three children
  stage2 = stage1 + every part of every child within the proven per-part Pareto bound at k-1
           (a sound, cheap necessary condition read from data/pareto_sb.csv)
The repo's existing 16.1x / 140.4x figures are quoted against cap + four-rectangle frontier, which
sits between these two, so stage2 is the fairer comparison.
"""
import csv, pickle, sys
from collections import Counter, defaultdict
import numpy as np

sys.path.insert(0, "/Users/fedor/radio/tools")
from analyze_single_solution_cuts import classify, children, cut_class
from analyze_pareto_prefix_census import mass, semantic_state

RNG = np.random.default_rng(20260820)

# ---- proven per-part bound: max n1 for a single part (n1:m) solvable in k -------------------
MAXN1 = defaultdict(dict)
with open("/Users/fedor/radio/data/pareto_sb.csv") as fh:
    for row in csv.DictReader(fh):
        try:
            k, m, n1 = int(row["k"]), int(row["m"]), int(row["n1"])
        except (ValueError, KeyError):
            continue
        if row.get("bound") == "max":
            MAXN1[k][m] = max(MAXN1[k].get(m, 0), n1)

def bound_table(k, mmax):
    """table[m] = largest n1 with (n1:m) solvable in k; 0 means impossible."""
    t = np.zeros(mmax + 2, dtype=np.int64)
    for m, n1 in MAXN1.get(k, {}).items():
        if m <= mmax + 1:
            t[m] = n1
    return t

def _load_corpora(k8_path, k7_path):
    """Parse both census logs. Cached to CENSUS_CACHE if set, since parsing k=8 takes ~20 s."""
    import os
    cache = os.environ.get("CENSUS_CACHE")
    if cache and os.path.exists(cache):
        return pickle.load(open(cache, "rb"))
    from analyze_single_solution_cuts import load
    out = (load(pathlib.Path(k8_path)), load(pathlib.Path(k7_path)))
    if cache:
        pickle.dump(out, open(cache, "wb"))
    return out


import pathlib, os
K8 = os.environ.get("K8_LOG", ".artifacts/pareto-census-k8/out.txt")
K7 = os.environ.get("K7_LOG", ".artifacts/pareto-census-k7/pareto_census_k7.out")
hi, lo = _load_corpora(K8, K7)

def prep(b):
    rk, states, meta, wins = b
    return dict(rk=rk, cap=3 ** rk, capc=3 ** (rk - 1), states=states, meta=meta, wins=wins,
                cls=classify(states, wins))

CORPORA = {"k8": prep(hi), "k7": prep(lo)}


def part_arrays(parts, B):
    """B random cuts, as per-part arrays."""
    return [(RNG.integers(0, n + 1, B), RNG.integers(0, m + 1, B)) for n, m in parts]


def child_masses(parts, ab):
    S = np.zeros(len(ab[0][0]), dtype=np.int64)
    X = np.zeros_like(S)
    for (n, m), (a, b) in zip(parts, ab):
        S += a * b
        X += a * (m - b) + (n - a) * b
    return S, X


def pareto_ok(parts, ab, k_child, mmax):
    """Sound necessary condition: every part of every child within the proven per-part bound."""
    tab = bound_table(k_child, mmax)
    ok = np.ones(len(ab[0][0]), dtype=bool)
    for (n, m), (a, b) in zip(parts, ab):
        for (pn, pm) in ((a, b), (n - a, m - b), (a, m - b), (n - a, b)):
            hi_ = np.maximum(pn, pm)
            lo_ = np.minimum(pn, pm)
            live = (hi_ * lo_) > 1                      # empty and unit parts are always fine
            lo_c = np.clip(lo_, 0, mmax + 1)
            ok &= ~(live & (hi_ > tab[lo_c]))
    return ok


def features(parts, ab, capc, M, mmax):
    """Scale-normalized, permutation- and complement-invariant features for a batch of cuts."""
    B = len(ab[0][0])
    S, X = child_masses(parts, ab)
    C = M - S - X
    Smax = np.maximum(S, C).astype(float)
    Smin = np.minimum(S, C).astype(float)
    Xf = X.astype(float)

    cols = {
        "X_over_cap": Xf / capc,
        "max_pure_over_cap": Smax / capc,
        "min_pure_over_cap": Smin / capc,
        "margin_over_cap": (Xf - Smax) / capc,
        "pure_gap_over_cap": (Smax - Smin) / capc,
        "parent_occupancy": np.full(B, M / (3.0 * capc)),
    }

    # per-part shape of the cut; complement flips t->1-t and u->1-u together, so |t-u| and
    # |t-1/2| are invariant under it.
    tu = []
    for (n, m), (a, b) in zip(parts, ab):
        t = a / n if n else np.zeros(B)
        u = b / m if m else np.zeros(B)
        tu.append((t, u))
    diag = np.stack([np.abs(t - u) for t, u in tu])
    offhalf = np.stack([np.abs(t - 0.5) for t, u in tu])
    offhalf_u = np.stack([np.abs(u - 0.5) for t, u in tu])
    cols["diag_mean"] = diag.mean(0)
    cols["diag_max"] = diag.max(0)
    cols["diag_min"] = diag.min(0)
    cols["offhalf_mean"] = offhalf.mean(0)
    cols["offhalf_max"] = offhalf.max(0)
    cols["offhalf_u_mean"] = offhalf_u.mean(0)

    sliver = np.zeros(B, dtype=np.int64)
    whole = np.zeros(B, dtype=np.int64)
    for (n, m), (a, b) in zip(parts, ab):
        s = a * b
        c = (n - a) * (m - b)
        whole += ((a == n) & (b == m)) | ((a == 0) & (b == 0))
        sliver += ((s == 0) | (c == 0)) & ~(((a == n) & (b == m)) | ((a == 0) & (b == 0)))
    cols["sliver_frac"] = sliver / len(parts)
    cols["whole_frac"] = whole / len(parts)

    # structure of each child: live part count, biggest rectangle, worst aspect.
    def child_stats(pairs, tag):
        live = np.zeros(B, dtype=np.int64)
        big = np.zeros(B, dtype=np.int64)
        big2 = np.zeros(B, dtype=np.int64)
        asp = np.zeros(B)
        for pn, pm in pairs:
            h = np.maximum(pn, pm).astype(np.int64)
            l = np.minimum(pn, pm).astype(np.int64)
            pm_ = h * l
            islive = pm_ > 1
            live += islive
            newbig = np.maximum(big, pm_)
            big2 = np.maximum(big2, np.minimum(big, pm_))
            big = newbig
            asp = np.maximum(asp, np.where(l > 0, h / np.maximum(l, 1), 0.0))
        cols[f"{tag}_parts"] = live / 8.0
        cols[f"{tag}_bigrect"] = big / capc
        cols[f"{tag}_top2"] = (big + big2) / capc
        cols[f"{tag}_aspect"] = np.minimum(asp, 40.0) / 40.0

    sel_pairs = [(a, b) for (a, b) in ab]
    comp_pairs = [(n - a, m - b) for (n, m), (a, b) in zip(parts, ab)]
    mix_pairs = ([(a, m - b) for (n, m), (a, b) in zip(parts, ab)]
                 + [(n - a, b) for (n, m), (a, b) in zip(parts, ab)])
    # symmetrize selected/complement so the feature set is complement-invariant
    swap = S > C
    def sym(pairsA, pairsB):
        return [(np.where(swap, pb[0], pa[0]), np.where(swap, pb[1], pa[1]))
                for pa, pb in zip(pairsA, pairsB)]
    child_stats(sym(sel_pairs, comp_pairs), "pureA")
    child_stats(sym(comp_pairs, sel_pairs), "pureB")
    child_stats(mix_pairs, "mixed")

    names = sorted(cols)
    return names, np.stack([cols[n] for n in names], axis=1).astype(np.float32)


def build(corpus_name, per_endpoint=1200, max_draws=200):
    C = CORPORA[corpus_name]
    capc = C["capc"]
    k_child = C["rk"] - 1
    mmax = max(max(max(p) for p in s) for s in C["states"].values())
    rows, labels, groups, stages = [], [], [], []
    names = None
    endpoints = [e for e in C["states"] if C["meta"][e]["parts"] == 4]
    for gi, e in enumerate(endpoints):
        parts = C["states"][e]
        M = C["meta"][e]["mass"]
        winset = {tuple(t) for t in C["wins"][e]}
        # ---- positives: one representative per winning class
        pos = [rep for rep in C["cls"][e].values()]
        if not pos:
            continue
        ab_pos = [(np.array([p[i][0] for p in pos]), np.array([p[i][1] for p in pos]))
                  for i in range(len(parts))]
        nm, fp = features(parts, ab_pos, capc, M, mmax)
        names = names or nm
        st2p = pareto_ok(parts, ab_pos, k_child, mmax)
        rows.append(fp); labels.append(np.ones(len(pos), dtype=np.int8))
        groups.append(np.full(len(pos), gi)); stages.append(np.where(st2p, 2, 1).astype(np.int8))
        # ---- negatives: uniform over the split space, kept if cap-feasible and not a winner
        got = 0
        for _ in range(max_draws):
            if got >= per_endpoint:
                break
            B = 4096
            ab = part_arrays(parts, B)
            S, X = child_masses(parts, ab)
            Cm = M - S - X
            keep = (S <= capc) & (X <= capc) & (Cm >= 0) & (Cm <= capc)
            if not keep.any():
                continue
            idx = np.flatnonzero(keep)[: per_endpoint - got]
            abk = [(a[idx], b[idx]) for a, b in ab]
            # drop any sampled cut that is actually a winner
            cuts = list(zip(*[(a.tolist(), b.tolist()) for a, b in abk]))
            isw = np.array([tuple(zip(cuts[i][0], cuts[i][1])) in winset
                            for i in range(len(idx))]) if False else np.zeros(len(idx), bool)
            for j in range(len(idx)):
                cand = tuple((int(abk[p][0][j]), int(abk[p][1][j])) for p in range(len(parts)))
                if cand in winset:
                    isw[j] = True
            if isw.any():
                sel = ~isw
                abk = [(a[sel], b[sel]) for a, b in abk]
            n_here = len(abk[0][0])
            if not n_here:
                continue
            _, fn = features(parts, abk, capc, M, mmax)
            st2 = pareto_ok(parts, abk, k_child, mmax)
            rows.append(fn); labels.append(np.zeros(n_here, dtype=np.int8))
            groups.append(np.full(n_here, gi)); stages.append(np.where(st2, 2, 1).astype(np.int8))
            got += n_here
    return (names, np.concatenate(rows), np.concatenate(labels),
            np.concatenate(groups), np.concatenate(stages))


if __name__ == "__main__":
    out = {}
    for name in ("k7", "k8"):
        names, Xf, y, g, st = build(name)
        print(f"{name}: {Xf.shape[0]:,} rows  {int(y.sum()):,} positives  "
              f"{len(set(g.tolist()))} states  stage2 share {100*(st==2).mean():.1f}%  "
              f"stage2 positives {int(((st==2)&(y==1)).sum()):,}/{int(y.sum()):,}")
        out[name] = dict(names=names, X=Xf, y=y, g=g, stage=st)
    pickle.dump(out, open("/tmp/ml/data.pkl", "wb"))
    print("features:", len(out["k8"]["names"]))
    print(", ".join(out["k8"]["names"]))
