#!/usr/bin/env python3
"""Convert a radio-negative-level-certificate-v2 chain into loadable cache facts.

The sa193 certificate proves unsolvability via a compact per-level `part`/`claim`/`fact` format
(see check_level_chain.py), not the plain-text "can't solve Sb(...) in k ..." lines a raw solver
log emits. radio_oracle.c's `load` command (radiobase.c's parse_file) expects the distilled cache
line format parse_out.sh produces from those raw logs:

    - b n1 m1 n2 m2 ... t <pairs> <n> <k>

where `pairs` = sum(n_i * m_i) (radiobase.c: sb_pairs[sbb] = n1*n2) and `n` = sum(n_i + m_i); `n`
is parsed but not used by cache() itself. `+`/`-` is solvable/unsolvable.

Only `claim` records are emitted (each level's own unsolvability claims) -- `fact` records are
each level's *support*, which by the chain's own construction equals the claim set one level down,
so emitting both would just duplicate every fact except the top level's.

SAFETY. cacheCantSolve (radiobase.c) is permutation-robust by construction -- at each recursion
level it rotates every remaining part into the pivot position in turn ("other part permutations in
this call may still add information"), unlike cacheCanSolve which assumes sort1's descending
order. Since every emitted fact here is a *negative* (this is a certificate of unsolvability), the
positive-path ordering assumption never applies and no additional sorting is needed here --  but
see validate_cert_load.py, which checks this claim empirically rather than trusting the reading of
the C source alone.

Usage:
    zstd -dc sa193-k4.cert.zst | tools/cert_to_cache.py --level 4 > cache_k4.txt
    tools/cert_to_cache.py sa193-k2.cert sa193-k3.cert ... > cache_all.txt   # auto-detects level
"""
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from check_level_chain import Level  # noqa: E402


def emit(level: Level, out) -> int:
    n = 0
    for state in level.claims:
        pairs = sum(a * b for a, b in state)
        sidesum = sum(a + b for a, b in state)
        parts = " ".join(f"{a} {b}" for a, b in state)
        out.write(f"- b {parts} t {pairs} {sidesum} {level.level}\n")
        n += 1
    return n


def load_cert_text(path: Path) -> str:
    if path.suffix == ".zst":
        return subprocess.run(["zstd", "-dc", str(path)], capture_output=True, text=True,
                               check=True).stdout
    return path.read_text()


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("certs", type=Path, nargs="*",
                     help=".cert or .cert.zst files; omit to read one plain .cert from stdin")
    args = ap.parse_args()

    total = 0
    if args.certs:
        for path in args.certs:
            text = load_cert_text(path)
            tmp = Path(f"/tmp/_cert_to_cache_{path.stem}.cert")
            tmp.write_text(text)
            try:
                level = Level(tmp)
                n = emit(level, sys.stdout)
                print(f"  {path.name}: level {level.level}, {n} claims", file=sys.stderr)
                total += n
            finally:
                tmp.unlink(missing_ok=True)
    else:
        tmp = Path("/tmp/_cert_to_cache_stdin.cert")
        tmp.write_text(sys.stdin.read())
        try:
            level = Level(tmp)
            n = emit(level, sys.stdout)
            print(f"  stdin: level {level.level}, {n} claims", file=sys.stderr)
            total += n
        finally:
            tmp.unlink(missing_ok=True)
    print(f"total: {total} cache facts", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
