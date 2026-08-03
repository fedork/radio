#!/usr/bin/env python3
"""Extract the evidence for a claim from raw solver logs, and audit logs for contradictions.

The raw corpus is ~26 GB across several machines and archives. The part that actually
certifies the published tables is about 30 KB: two log lines per frontier cell. This tool
finds those lines so the provenance can live in git while the logs live wherever is cheap -
or nowhere.

It also runs the audit that matters when folding in an old or unfamiliar log: does it
contradict what is already proven, or itself?

Subcommands
-----------
certify   For every `status=proven-exhaustive` row in data/*.csv, locate the can-solve line
          at n and the can't-solve line at n+1. Reports any obligation it cannot meet.
          Writes an evidence file with `--out`.

audit     Check logs and parsed caches against data/*.csv and against themselves:
            * a state claimed solvable beyond a proven maximum
            * a state claimed unsolvable at or below a proven maximum
            * the same state both solvable in k and unsolvable in some k' >= k
            * a solvable claim violating the information bound, mass > 3^k
          Any hit means either the log or the table is wrong; both are worth knowing.

Both accept plain logs and `parse_out.sh`-style caches, and read `.zst`/`.gz` transparently.
Zip archives are not read directly - pipe entries in with `unzip -p`, or pass `-` for stdin.

Usage
-----
  tools/extract_evidence.py certify out_k7.txt out_k8.txt --out evidence/cert.txt
  tools/extract_evidence.py audit parsed_260.txt
  unzip -p archive.zip 'radio/out*.txt' | tools/extract_evidence.py audit -
"""
from __future__ import annotations

import argparse
import csv
import gzip
import os
import re
import subprocess
import sys
from typing import Dict, Iterator, List, Tuple

LOG_SB = re.compile(rb"^(?:result\s+)?can('t)? solve Sb\((\d+):(\d+)\)\[(\d+),\d+\]"
                    rb"(?: size=\S+)? in (\d+)\b")
LOG_SA = re.compile(rb"^(?:result\s+)?can('t)? solve Sa\((\d+)\) in (\d+)\b")
RATIO = re.compile(rb"^result in (\d+) ratio = (\d+)/(\d+) solvability [\d.]+ Sb\(([^)]*)\)")


def open_log(path: str):
    if path == "-":
        return sys.stdin.buffer
    if path.endswith(".zst"):
        return subprocess.Popen(["zstd", "-dc", path], stdout=subprocess.PIPE).stdout
    if path.endswith(".gz"):
        return gzip.open(path, "rb")
    return open(path, "rb")


def parts_of(fields: List[bytes]) -> Tuple[Tuple[int, int], ...]:
    """Parts of a `parse_out.sh` cache line: `± b n1 n2 [n3 n4 ...] t pairs n k`."""
    out = []
    for i in range(2, len(fields) - 4, 2):
        a, b = int(fields[i]), int(fields[i + 1])
        if a and b:
            out.append((max(a, b), min(a, b)))
    return tuple(sorted(out, reverse=True))


def scan(paths: List[str]) -> Iterator[Tuple[str, tuple]]:
    """Yield ('log'|'cache'|'ratio', payload) for every interpretable line."""
    for path in paths:
        with open_log(path) as fh:
            for raw in fh:
                if raw[:1] in (b"+", b"-"):                      # parse_out.sh cache
                    f = raw.split()
                    if len(f) < 4:
                        continue
                    sign = f[0] == b"+"
                    if f[1] == b"a":
                        yield "cache", (path, sign, ("a", int(f[2])), int(f[3]), None)
                    elif f[1] == b"b" and len(f) >= 8:
                        yield "cache", (path, sign, ("b", parts_of(f)), int(f[-1]), int(f[-3]))
                    continue
                if not raw.startswith((b"can", b"result")):
                    continue
                m = LOG_SB.match(raw)
                if m:
                    n1, n2 = int(m.group(2)), int(m.group(3))
                    yield "log", (path, not m.group(1), ("b", ((max(n1, n2), min(n1, n2)),)),
                                  int(m.group(5)), int(m.group(4)), raw.decode(errors="replace").rstrip())
                    continue
                m = LOG_SA.match(raw)
                if m:
                    yield "log", (path, not m.group(1), ("a", int(m.group(2))),
                                  int(m.group(3)), None, raw.decode(errors="replace").rstrip())
                    continue
                m = RATIO.match(raw)
                if m:
                    yield "ratio", (path, int(m.group(1)), int(m.group(2)), int(m.group(3)),
                                    m.group(4).decode())


def load_tables(data: str):
    sb, sa = {}, {}
    for r in csv.DictReader(open(os.path.join(data, "pareto_sb.csv"))):
        if r["bound"] == "max":
            sb[(int(r["k"]), int(r["m"]))] = (int(r["n1"]), r["status"])
    for r in csv.DictReader(open(os.path.join(data, "pareto_sa.csv"))):
        if r["bound"] == "max":
            sa[int(r["k"])] = (int(r["n"]), r["status"])
    return sb, sa


def cmd_certify(args) -> int:
    sb, sa = load_tables(args.data)
    want: Dict[tuple, str] = {}
    for (k, m), (n, st) in sb.items():
        if st != "proven-exhaustive":
            continue
        want[("b", ((n, m),), k)] = f"n({k},{m}) = {n} achievable"
        want[("b", ((n + 1, m),), k)] = f"n({k},{m}) = {n} maximal"
    for k, (n, st) in sa.items():
        if st != "proven-exhaustive":
            continue
        want[("a", n, k)] = f"Sa({n}) in {k}"
        want[("a", n + 1, k)] = f"Sa({n + 1}) not in {k}"

    found: Dict[tuple, Tuple[str, str]] = {}
    for kind, p in scan(args.logs):
        if kind != "log":
            continue
        path, ok, state, k, _pairs, line = p
        key = (state[0], state[1], k)
        if key in want and key not in found:
            found[key] = (path, line)

    print(f"obligations : {len(want)}")
    print(f"located     : {len(found)}")
    missing = sorted(set(want) - set(found), key=str)
    if missing:
        print(f"MISSING     : {len(missing)}")
        for k in missing[:20]:
            print(f"   {want[k]}")
    if args.out and found:
        with open(args.out, "w") as out:
            out.write(f"# Evidence extract, {len(found)}/{len(want)} obligations.\n"
                      f"# Sources: {', '.join(sorted({p for p, _ in found.values()}))}\n"
                      f"# Regenerate: tools/extract_evidence.py certify ...\n#\n")
            for key in sorted(found, key=str):
                out.write(found[key][1] + "\n")
        print(f"wrote {args.out} ({os.path.getsize(args.out):,} bytes)")
    return 0 if not missing else 1


def cmd_audit(args) -> int:
    sb, sa = load_tables(args.data)
    errs: List[str] = []
    best_pos: Dict[tuple, int] = {}
    worst_neg: Dict[tuple, int] = {}
    n = 0
    for kind, p in scan(args.logs):
        if kind == "ratio":
            path, k, solv, tot, state = p
            if solv == 0:
                print(f"  exhaustively UNSOLVABLE in {k}: Sb({state})   [{path}]")
            continue
        if kind not in ("log", "cache"):
            continue
        n += 1
        path, ok, state, k, pairs = p[0], p[1], p[2], p[3], p[4]
        if state[0] == "b":
            if pairs is not None and ok and pairs > 3 ** k:
                errs.append(f"information bound: Sb{list(state[1])} claimed solvable in {k} "
                            f"with mass {pairs} > 3^{k}   [{path}]")
            if len(state[1]) == 1:
                a, m = state[1][0]
                if (k, m) in sb:
                    mx, st = sb[(k, m)]
                    if ok and a > mx:
                        errs.append(f"Sb({a}:{m}) claimed solvable in {k}, but n({k},{m})={mx} "
                                    f"is proven maximal   [{path}]")
                    if not ok and a <= mx:
                        errs.append(f"Sb({a}:{m}) claimed unsolvable in {k}, but n({k},{m})={mx} "
                                    f"is proven   [{path}]")
        else:
            v = state[1]
            if k in sa:
                mx, st = sa[k]
                if ok and v > mx:
                    errs.append(f"Sa({v}) claimed solvable in {k}, proven max {mx}   [{path}]")
                if not ok and v <= mx:
                    errs.append(f"Sa({v}) claimed unsolvable in {k}, proven max {mx}   [{path}]")
        key = state
        if ok:
            if key not in best_pos or k < best_pos[key]:
                best_pos[key] = k
        else:
            if key not in worst_neg or k > worst_neg[key]:
                worst_neg[key] = k
    both = best_pos.keys() & worst_neg.keys()
    for key in both:
        if worst_neg[key] >= best_pos[key]:
            errs.append(f"self-contradiction: {key} solvable in {best_pos[key]} "
                        f"but unsolvable in {worst_neg[key]}")
    print(f"lines interpreted   : {n:,}")
    print(f"distinct states     : {len(best_pos.keys() | worst_neg.keys()):,}")
    print(f"states with both    : {len(both):,}")
    print(f"CONTRADICTIONS      : {len(errs)}")
    for e in errs[:25]:
        print(f"   {e}")
    return 0 if not errs else 1


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--data", default=os.path.join(os.path.dirname(__file__), "..", "data"))
    sub = ap.add_subparsers(dest="cmd", required=True)
    c = sub.add_parser("certify"); c.add_argument("logs", nargs="+"); c.add_argument("--out")
    c.set_defaults(fn=cmd_certify)
    a = sub.add_parser("audit"); a.add_argument("logs", nargs="+")
    a.set_defaults(fn=cmd_audit)
    args = ap.parse_args()
    return args.fn(args)


if __name__ == "__main__":
    sys.exit(main())
