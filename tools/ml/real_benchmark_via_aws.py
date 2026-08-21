#!/usr/bin/env python3
"""Re-run this thread's real-oracle ordering benchmark (real_benchmark_generic.py /
real_benchmark_beam.py), but against the persistent oracle-serve instance instead of a cold local
radio_oracle -- same method (exact stage-2 candidates, R_0 filter, recursive-V order, walk until
success), routed through tools/oracle_tcp_client.py over an SSM tunnel.

Deliberately does NOT use the `enumerate` command here: enumerate's raw mixed-radix walk has no
incremental pruning (checked_summary only checks the cap/R_0 bound at each leaf, not on partial
sub-trees -- see radio_oracle.c's own comment), so at k=7 with wider parts its raw space runs into
the tens of billions and a single call can take 10+ minutes with no result (measured 2026-08-22,
killed unfinished). Walking a stage-2 (cap + per-part-Pareto) candidate set with plain per-
candidate queries is the proven-fast path for THIS purpose; enumerate remains the right tool for
"give me the complete exact winner set" on smaller states (k<=6, as already validated).

Needs an SSM tunnel to the oracle-serve instance already open on 127.0.0.1:7777.
"""
import sys, time
import numpy as np

sys.path.insert(0, "/Users/fedor/radio")
sys.path.insert(0, "/Users/fedor/radio/tools")
sys.path.insert(0, "/Users/fedor/radio/tools/ml")
from cut_ranker_data import bound_table
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


def main():
    state_str, K = sys.argv[1], int(sys.argv[2])
    # NOTE for census-derived states: CORPORA["k7"]/["k8"]'s "endpoints" are residual states
    # reached AFTER some root-level splits -- their real parent k (C["rk"]) is 5 and 6
    # respectively, NOT literally 7/8. Pass the correct residual k as K here, or every solver
    # call answers a different, easier question than the endpoint's real claim (caught live
    # 2026-08-22: a first attempt used K=7 for a "k7" endpoint whose real k was 5).
    cand_cap = int(sys.argv[3]) if len(sys.argv) > 3 else 20000
    PARTS = [tuple(int(x) for x in p.split(":")) for p in state_str.split(",")]
    KC = K - 1
    CAPC = 3 ** KC
    M = sum(n * m for n, m in PARTS)
    print(f"Sb state: {PARTS}  k={K}  parts={len(PARTS)}  mass={M}  cap={3**K}  capc={CAPC}",
          file=sys.stderr)

    t0 = time.time()
    print("training recursive V on k<=6 matched-sampler ...", file=sys.stderr)
    lr, gb = rv.experiment_1([4, 5, 6], 7)
    V = lambda X: lr.predict_proba(X)[:, 1]
    print(f"  done [{time.time()-t0:.0f}s]", file=sys.stderr)

    mmax = max(max(p) for p in PARTS)
    tab = bound_table(KC, mmax)
    ab, ncand = exact_candidates(PARTS, M, CAPC, tab, mmax)
    print(f"exact stage-2 candidates: {ncand:,}  [{time.time()-t0:.1f}s]", file=sys.stderr)

    rng = np.random.default_rng(20260822)
    if ncand > cand_cap:
        idx = rng.choice(ncand, size=cand_cap, replace=False)
        ab = [(a[idx], b[idx]) for a, b in ab]
        ncand = cand_cap
        print(f"  subsampled to {ncand:,}", file=sys.stderr)

    r0_pass = np.zeros(ncand, dtype=bool)
    sel_f, mix_f, comp_f = [], [], []
    sel_list, mix_list, comp_list = [], [], []
    for i in range(ncand):
        take = tuple((int(a[i]), int(b[i])) for a, b in ab)
        sel, mix, comp = raw_children(PARTS, take)
        r0_pass[i] = bm.r0(sel, KC) and bm.r0(mix, KC) and bm.r0(comp, KC)
        sel_f.append(rv.feat(list(sel), KC)); mix_f.append(rv.feat(list(mix), KC))
        comp_f.append(rv.feat(list(comp), KC))
        sel_list.append(sel); mix_list.append(mix); comp_list.append(comp)
    print(f"R_0 survivors: {int(r0_pass.sum()):,} of {ncand:,}  [{time.time()-t0:.1f}s]",
          file=sys.stderr)

    surv = np.flatnonzero(r0_pass)
    vs = V(np.array([sel_f[i] for i in surv], np.float32))
    vm = V(np.array([mix_f[i] for i in surv], np.float32))
    vc = V(np.array([comp_f[i] for i in surv], np.float32))
    score = np.minimum(np.minimum(vs, vm), vc)
    order_learned = surv[np.argsort(-score)]
    cands = [tuple((int(a[i]), int(b[i])) for a, b in ab) for i in range(ncand)]

    oracle = TCPOracle(timeout=60)

    def run_order(order, label, cap_tries):
        tried = 0
        for idx in order:
            tried += 1
            take = cands[idx]
            sel, mix, comp = sel_list[idx], mix_list[idx], comp_list[idx]
            ok = True
            for child in (sel, mix, comp):
                if not child:
                    continue
                v = oracle.ask(KC, list(child))
                if v != "SOLVABLE":
                    ok = False
                    break
            if ok:
                print(f"  [{label}] SUCCESS at candidate #{tried}: split={take}  "
                      f"[{time.time()-t0:.0f}s]", file=sys.stderr)
                return tried, take
            if tried % 100 == 0:
                print(f"  [{label}] {tried} tried, {time.time()-t0:.0f}s elapsed", file=sys.stderr)
            if tried >= cap_tries:
                print(f"  [{label}] gave up after {cap_tries}", file=sys.stderr)
                return None, None
        print(f"  [{label}] exhausted all {tried} without success", file=sys.stderr)
        return None, None

    print("\n--- LEARNED ORDER (R_0 survivors, sorted by recursive V), via AWS oracle ---",
          file=sys.stderr)
    tried_learned, found_learned = run_order(order_learned, "learned", cap_tries=20000)

    print(f"\n=== SUMMARY for Sb({state_str})@{K} ({len(PARTS)} parts) ===")
    print(f"stage-2 candidates: {ncand:,}   R_0 survivors: {int(r0_pass.sum()):,}")
    print(f"learned order (R_0 -> recursive V), via persistent AWS oracle:  "
          f"{tried_learned} candidates tried, found {found_learned}")
    print(f"[{time.time()-t0:.0f}s total]")


if __name__ == "__main__":
    main()
