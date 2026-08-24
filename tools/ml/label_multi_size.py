#!/usr/bin/env python3
"""Generate and label matched corpora at several PART COUNTS, not just the original 4.

The existing labeled_k{4,5,6,7}.txt (data/ml_order/) is exclusively 4-part states -- the model
trained on it only matches canSolveB_ctx's own size=4 decompositions, not the size=1/2/1 shape a
leaf query like Sb(112:80) actually produces at its own level 0. This generates the missing
shapes so a single combined model (nparts as a real feature) and separate per-size models can
both be trained and compared, rather than assuming which is right.

Same methodology as /tmp/rec/calibrate_and_label2.py (not committed, session-only): per (k,
nparts), bisect the mass band for a ~50% solvable split via the warm oracle, then draw 300
states from that band and label them for real via the same oracle.

Usage: tools/ml/label_multi_size.py <oracle_binary> [--out-dir data/ml_order]
"""
import argparse
import subprocess
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from tools.oracle_client import Oracle
from tools.oracle_tcp_client import TCPOracle
from contextlib import contextmanager


@contextmanager
def _tcp_oracle(host, port):
    o = TCPOracle(host=host, port=port, timeout=180)
    o.ready = o.stats()  # cheap round-trip that also confirms the server is actually reachable
    yield o

REPO = Path(__file__).resolve().parents[2]
GEN = [sys.executable, str(REPO / "tools/ml/value_gen_states.py")]
WIDTH = 0.08

# (nparts, m_lo, m_hi): wider m-range for fewer parts, since fewer parts must carry more mass
# each -- matches the shapes canSolveB_ctx's own recursion actually produces (a size=1 leaf like
# Sb(112:80) has m=80; the original size=4 battery states have m in [6,14]).
SHAPES = {
    1: (1, 150),
    2: (1, 80),
    3: (2, 25),
    4: (2, 7),   # reproduces the original 4-part generator's own range exactly
}


WIDTHCAP_MAX = 185  # margin under the labeling oracle's compiled MAX_N (see --max-n); overridden
                     # in main() from args.max_n


def gen(k, n, lo, hi, nparts, m_lo, m_hi):
    out = subprocess.run(GEN + [str(k), str(n), str(lo), str(hi), str(nparts), str(m_lo), str(m_hi),
                                 str(WIDTHCAP_MAX)],
                          capture_output=True, text=True).stdout
    lines = [l for l in out.splitlines() if l.strip()]
    states = []
    for l in lines:
        v = [int(x) for x in l.split()]
        states.append([(v[i], v[i + 1]) for i in range(0, len(v), 2)])
    return states, lines


def label(o, k, states):
    return [{"SOLVABLE": 0, "UNSOLVABLE": 1, "MAYBE": 2}[o.ask(k, parts)] for parts in states]


def probe(o, k, lo, nparts, m_lo, m_hi, n=30):
    hi = lo + WIDTH
    states, _ = gen(k, n, lo, hi, nparts, m_lo, m_hi)
    if len(states) < n * 0.5:
        return None
    codes = label(o, k, states)
    solv = sum(1 for c in codes if c == 0); unsolv = sum(1 for c in codes if c == 1)
    if solv + unsolv < n * 0.5:
        return None
    return solv / (solv + unsolv)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("oracle_binary", nargs="?", default=None,
                     help="local radio_oracle binary path (ignored if --tcp is given)")
    ap.add_argument("--tcp", action="store_true",
                     help="use the persistent oracle-serve TCP server (see docs/aws-run.md) "
                          "instead of spawning a local binary -- reach it via the documented SSM "
                          "port-forward first")
    ap.add_argument("--tcp-host", default="127.0.0.1")
    ap.add_argument("--tcp-port", type=int, default=7777)
    ap.add_argument("--out-dir", default=str(REPO / "data/ml_order"))
    ap.add_argument("--levels", default="4,5,6,7")
    ap.add_argument("--sizes", default="1,2,3")  # 4 already exists; rerun by adding it here
    ap.add_argument("--budget-seconds", type=int, default=5)
    ap.add_argument("--n-target", type=int, default=300)
    ap.add_argument("--suffix", default="")  # e.g. "_hi" to avoid clobbering an existing k<=7 file
    ap.add_argument("--restore-any", default=None,
                     help="path to a cache.snap to restore (via --restore-any=, foreign identity "
                          "allowed) before labeling -- cache hits on already-known facts are free")
    ap.add_argument("--max-n", type=int, default=193,
                     help="the labeling oracle's compiled MAX_N; WIDTHCAP is kept a margin under it")
    args = ap.parse_args()
    out_dir = Path(args.out_dir)
    levels = [int(x) for x in args.levels.split(",")]
    sizes = [int(x) for x in args.sizes.split(",")]
    caches = [f"--restore-any={args.restore_any}"] if args.restore_any else []
    global WIDTHCAP_MAX
    WIDTHCAP_MAX = args.max_n - 8

    oracle_cm = (_tcp_oracle(args.tcp_host, args.tcp_port) if args.tcp
                 else Oracle(args.oracle_binary, caches=caches, budget_seconds=args.budget_seconds,
                             echo_ready=True))
    with oracle_cm as o:
        print(o.ready, file=sys.stderr); sys.stderr.flush()
        for nparts in sizes:
            m_lo, m_hi = SHAPES[nparts]
            for k in levels:
                print(f"\n=== nparts={nparts} k={k}: bisecting for a ~50% solvable band "
                      f"(m in [{m_lo},{m_hi}]) ===", file=sys.stderr); sys.stderr.flush()
                lo_bound, hi_bound = 0.02, 0.94
                best = None
                for step in range(7):
                    lo = (lo_bound + hi_bound) / 2
                    frac = probe(o, k, lo, nparts, m_lo, m_hi)
                    print(f"  step {step}: lo={lo:.3f} -> "
                          f"{'infeasible/degenerate' if frac is None else f'solved frac {frac:.2f}'}",
                          file=sys.stderr); sys.stderr.flush()
                    if frac is None:
                        hi_bound = lo
                        continue
                    if best is None or abs(frac - 0.5) < abs(best[1] - 0.5):
                        best = (lo, frac)
                    if frac > 0.5:
                        lo_bound = lo
                    else:
                        hi_bound = lo
                if best is None:
                    print(f"  nparts={nparts} k={k}: bisection never found a mixed band -- skipping",
                          file=sys.stderr)
                    continue
                lo, frac = best
                hi = lo + WIDTH
                print(f"  nparts={nparts} k={k}: chosen band [{lo:.3f},{hi:.3f}] "
                      f"(solved frac {frac:.2f} at n=30 probe)", file=sys.stderr); sys.stderr.flush()

                t0 = time.time()
                states, lines = gen(k, args.n_target, lo, hi, nparts, m_lo, m_hi)
                codes = label(o, k, states)
                out_dir.mkdir(parents=True, exist_ok=True)
                states_path = out_dir / f"states_n{nparts}_k{k}{args.suffix}.txt"
                labeled_path = out_dir / f"labeled_n{nparts}_k{k}{args.suffix}.txt"
                states_path.write_text("\n".join(lines) + "\n")
                with open(labeled_path, "w") as f:
                    for c, l in zip(codes, lines):
                        f.write(f"{c} {l}\n")
                solv = sum(1 for c in codes if c == 0); unsolv = sum(1 for c in codes if c == 1)
                maybe = sum(1 for c in codes if c == 2)
                print(f"  nparts={nparts} k={k}: final {len(states)}-state sample in "
                      f"{time.time()-t0:.1f}s -> solvable={solv} unsolvable={unsolv} maybe={maybe}",
                      file=sys.stderr); sys.stderr.flush()
        print(o.stats(), file=sys.stderr)


if __name__ == "__main__":
    main()
