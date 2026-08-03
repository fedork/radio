#!/usr/bin/env python3
"""Check the source-of-truth tables in data/ for internal consistency.

Runs in a second and needs nothing but the CSVs, so it is cheap to run on every change.
It exists because this project has already shipped two errors of exactly the kind it
catches: a Pareto column that went stale in three of its four copies, and a lemma whose
`k(k-1)/2` was transcribed as `k(k-5)/2`, which claimed unsolvable states were solvable.

Checks performed:

  monotone in m   n(k,m) >= n(k,m+1). Sb(n:m+1) solvable implies Sb(n:m) solvable, since
                  the latter is a substate, so the frontier cannot rise with m.
  monotone in k   n(k,m) <= n(k+1,m). An extra test never hurts.
  (u1)            n(k,m-1) >= n(k,m) + 1. Conjectured, not proved - reported separately.
  info bound      n*m <= 3^k. k ternary tests distinguish at most 3^k cases.
  Sa consistency  Sa(k) = max over n1 <= Sa(k-1) of n1 + (largest n2 with Sb(n1:n2) in k-1),
                  checked as an upper bound wherever the K=k-1 frontier is known.
  formulas        every closed form in conjectures.csv reproduces every known max value of
                  its row at or above its fits_from_k.
  provenance      every row carries a status from the accepted vocabulary, and every
                  status that claims evidence names a source.
  rendered docs   any markdown block delimited by `<!-- generated:NAME -->` and
                  `<!-- /generated -->` still matches what the CSVs produce. Docs are meant
                  to be readable, so the tables do appear in prose - but they are generated,
                  never hand-copied, which is what keeps a stale copy from surviving.

Usage:  tools/check_tables.py [--data DIR] [--render]
        --render rewrites the generated blocks in place instead of checking them.
Exit status is nonzero if any check fails. Conjecture violations are warnings, not
failures, unless the conjecture is contradicted by a proven value.
"""
from __future__ import annotations

import argparse
import csv
import glob
import os
import re
import sys
from typing import Dict, List

STATUSES = {"proven-exhaustive", "proven-theorem", "witness", "solver-lower",
            "legacy", "conjecture", "refuted"}
BOUNDS = {"max", "lower", "upper"}
NEEDS_SOURCE = {"proven-exhaustive", "proven-theorem", "witness", "solver-lower"}


def singleton_base(k: int) -> List[int]:
    cur = [1]
    for _ in range(k):
        nxt = [0] * (2 * len(cur))
        for i, h in enumerate(cur):
            nxt[i] += h
            nxt[2 * i] += h
            nxt[2 * i + 1] += h
        cur = sorted(nxt, reverse=True)
    return cur


def dyadic_letters(k: int) -> Dict[str, int]:
    """Letter -> value of the leading atom of each dyadic block of G_k (A, B, C, DDDD...)."""
    g = singleton_base(k) if k >= 0 else []
    starts = [0, 1, 2, 4, 8, 16, 32, 64, 128]
    return {chr(65 + i): (g[s] if s < len(g) else 0) for i, s in enumerate(starts)}


MARK = re.compile(r"(<!-- generated:([a-z_]+) -->\n)(.*?)(<!-- /generated -->)", re.S)


def render_pareto_sb(sb: List[dict]) -> str:
    """The Pareto frontier as a readable grid, showing what is actually known per cell."""
    cells: Dict[int, Dict[int, Dict[str, int]]] = {}
    for r in sb:
        c = cells.setdefault(int(r["m"]), {}).setdefault(int(r["k"]), {})
        n, b = int(r["n1"]), r["bound"]
        if b == "max":
            c["max"] = n
        elif b == "lower":
            c["lower"] = max(n, c.get("lower", 0))
        elif b == "upper":
            c["upper"] = min(n, c.get("upper", 10 ** 9))
    ks = sorted({int(r["k"]) for r in sb})
    out = ["| m\\k | " + " | ".join(str(k) for k in ks) + " |",
           "|---" * (len(ks) + 1) + "|"]
    for m in sorted(cells):
        row = []
        for k in ks:
            c = cells[m].get(k)
            if not c:
                row.append("")
            elif "max" in c:
                row.append(str(c["max"]))
            elif "lower" in c and "upper" in c:
                row.append(f"{c['lower']}–{c['upper']}")
            elif "lower" in c:
                row.append(f"≥{c['lower']}")
            else:
                row.append(f"≤{c['upper']}")
        out.append(f"| **{m}** | " + " | ".join(row) + " |")
    out.append("")
    out.append("A bare number is a proven maximum. `≥n` is a lower bound (a solution exists, "
               "maximality open), `≤n` an upper bound (exhaustively refuted above), `a–b` a "
               "two-sided bracket. Per-cell status and evidence are in `data/pareto_sb.csv`.")
    return "\n".join(out) + "\n"


def render_pareto_sa(sa: List[dict]) -> str:
    rows = sorted(sa, key=lambda r: int(r["k"]))
    out = ["| k | " + " | ".join(r["k"] for r in rows) + " |",
           "|---" * (len(rows) + 1) + "|",
           "| max n | " + " | ".join(r["n"] if r["bound"] == "max" else f"({r['n']})"
                                     for r in rows) + " |",
           "",
           "Parenthesised means lower bound only. Evidence per row in `data/pareto_sa.csv`."]
    return "\n".join(out) + "\n"


def render_conjectures(cj: List[dict]) -> str:
    out = ["| m | closed form | fits from | status |", "|---|---|---|---|"]
    for r in sorted((r for r in cj if r["model"] == "closed-form"), key=lambda r: int(r["m"])):
        f = r["formula"].replace("**", "^").replace("//", "/")
        out.append(f"| {r['m']} | `{f}` | k >= {r['fits_from_k'] or '?'} | {r['status']} |")
    out.append("")
    out.append("Formulas are stored executably in `data/conjectures.csv` and checked against "
               "every proven datum by `tools/check_tables.py`.")
    return "\n".join(out) + "\n"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--data", default=os.path.join(os.path.dirname(__file__), "..", "data"))
    ap.add_argument("--render", action="store_true",
                    help="rewrite generated doc blocks instead of checking them")
    args = ap.parse_args()
    d = args.data

    sb = list(csv.DictReader(open(os.path.join(d, "pareto_sb.csv"))))
    sa = list(csv.DictReader(open(os.path.join(d, "pareto_sa.csv"))))
    cj = list(csv.DictReader(open(os.path.join(d, "conjectures.csv"))))
    apath = os.path.join(d, "artifacts.csv")
    arts = list(csv.DictReader(open(apath))) if os.path.exists(apath) else []
    tags = {r["tag"] for r in arts}

    errs: List[str] = []
    warns: List[str] = []

    # provenance -----------------------------------------------------------------
    for name, rows, keyf in (("pareto_sb", sb, lambda r: f"k={r['k']} m={r['m']}"),
                             ("pareto_sa", sa, lambda r: f"k={r['k']}"),
                             ("conjectures", cj, lambda r: f"m={r['m']} {r['model']}")):
        for r in rows:
            if r["status"] not in STATUSES:
                errs.append(f"{name} {keyf(r)}: unknown status {r['status']!r}")
            if r["status"] in NEEDS_SOURCE and not r.get("source", "").strip():
                errs.append(f"{name} {keyf(r)}: status {r['status']} claims evidence "
                            f"but names no source")
            if "bound" in r and r["bound"] not in BOUNDS:
                errs.append(f"{name} {keyf(r)}: unknown bound {r['bound']!r}")
            # Every source must be resolvable: a file in the repo, a theorem reference, or
            # `tag:path` naming an archive tag that data/artifacts.csv knows about. A bare
            # log filename is not good enough - nobody can find it later.
            src = (r.get("source") or "").strip()
            if src and not src.startswith(("lemma", "definition", "docs/")):
                for piece in src.split("+"):
                    if ":" in piece and not piece.startswith(("witnesses/", "evidence/", "data/")):
                        tag = piece.split(":", 1)[0]
                        if tags and tag not in tags:
                            errs.append(f"{name} {keyf(r)}: source names unknown archive "
                                        f"tag {tag!r}")
                    else:
                        # resolve repo-relative, from this script's location, so that
                        # --data can point at a scratch copy for testing
                        p = os.path.join(os.path.dirname(__file__), "..",
                                         piece.split("#")[0])
                        if not os.path.exists(p):
                            errs.append(f"{name} {keyf(r)}: source {piece!r} is neither a "
                                        f"file in the repo nor tag:path")

    # the proven Sb frontier -----------------------------------------------------
    maxv: Dict[int, Dict[int, int]] = {}
    for r in sb:
        if r["bound"] == "max":
            maxv.setdefault(int(r["k"]), {})[int(r["m"])] = int(r["n1"])
    lower: Dict[int, Dict[int, int]] = {}
    upper: Dict[int, Dict[int, int]] = {}
    for r in sb:
        k, m, n = int(r["k"]), int(r["m"]), int(r["n1"])
        if r["bound"] == "upper":
            cur = upper.setdefault(k, {})
            if m not in cur or n < cur[m]:      # keep the tightest upper bound
                cur[m] = n
        else:
            cur = lower.setdefault(k, {})
            if m not in cur or n > cur[m]:      # keep the strongest lower bound
                cur[m] = n

    # a lower bound may never exceed an upper bound for the same cell
    for k, col in sorted(upper.items()):
        for m, hi in sorted(col.items()):
            lo = lower.get(k, {}).get(m)
            if lo is not None and lo > hi:
                errs.append(f"k={k} m={m}: lower bound {lo} exceeds upper bound {hi}")
            if m in maxv.get(k, {}) and maxv[k][m] > hi:
                errs.append(f"k={k} m={m}: proven maximum {maxv[k][m]} exceeds upper bound {hi}")

    for k, col in sorted(maxv.items()):
        for m, n in sorted(col.items()):
            if n * m > 3 ** k:
                errs.append(f"k={k} m={m}: n*m = {n * m} exceeds 3^{k} = {3 ** k}")
            if m + 1 in col and col[m + 1] > n:
                errs.append(f"k={k}: n({m + 1})={col[m + 1]} > n({m})={n}, "
                            f"violates monotonicity in m")
            if k + 1 in maxv and m in maxv[k + 1] and maxv[k + 1][m] < n:
                errs.append(f"m={m}: n(k={k + 1})={maxv[k + 1][m]} < n(k={k})={n}, "
                            f"violates monotonicity in k")
            if m - 1 in col and col[m - 1] < n + 1:
                warns.append(f"k={k} m={m}: (u1) would need n({m - 1}) >= {n + 1}, "
                             f"table has {col[m - 1]}")

    # Cross-constraints between bounds. Note that two lower bounds say nothing about each
    # other - n(k,87) >= 88 and n(k,88) >= 89 are perfectly compatible, because the true
    # values may both be far higher. The real constraint is that a lower bound at m+1 cannot
    # exceed a ceiling at m, since n(k,m+1) <= n(k,m).
    for k, col in sorted(lower.items()):
        for m, lo in sorted(col.items()):
            ceil_prev = None
            if m - 1 in upper.get(k, {}):
                ceil_prev = ("upper bound", upper[k][m - 1])
            elif m - 1 in maxv.get(k, {}):
                ceil_prev = ("proven maximum", maxv[k][m - 1])
            if ceil_prev and lo > ceil_prev[1]:
                errs.append(f"k={k}: lower bound n({m})>={lo} exceeds the {ceil_prev[0]} "
                            f"n({m - 1})<={ceil_prev[1]}, but n is non-increasing in m")
            if lo * m > 3 ** k:
                errs.append(f"k={k} m={m}: lower bound {lo} needs mass {lo * m} > 3^{k}")

    # Sa upper bound from the Sb frontier -----------------------------------------
    sa_max = {int(r["k"]): int(r["n"]) for r in sa if r["bound"] == "max"}
    for r in sa:
        k, n = int(r["k"]), int(r["n"])
        if k - 1 not in maxv or k - 1 not in sa_max:
            continue
        # Only meaningful when the K=k-1 Sb column is complete over the reachable range.
        # For k=10 it is not (K=9 has proven maxima only up to m=1), and applying it anyway
        # produces a nonsense ceiling.
        if not all(m in maxv[k - 1] for m in range(1, sa_max[k - 1] + 1)):
            continue
        # Sa(n) in k splits into n1 taken / n-n1 not, needing Sa(n1) in k-1 and
        # Sb(n1 : n-n1) in k-1. So n <= max over feasible n1 of n1 + n2max(n1).
        col = maxv[k - 1]
        best = 0
        for n1 in range(1, sa_max[k - 1] + 1):
            n2 = max((m for m, v in col.items() if v >= n1), default=0)
            best = max(best, n1 + n2)
        if best and n > best and r["bound"] == "max":
            errs.append(f"Sa k={k}: claimed {n} exceeds the bound {best} implied by the "
                        f"K={k - 1} Sb frontier")

    # formulas --------------------------------------------------------------------
    checked = 0
    for r in cj:
        m, model, formula, status = int(r["m"]), r["model"], r["formula"], r["status"]
        if status == "refuted":
            continue
        for k, col in sorted(maxv.items()):
            if m not in col:
                continue
            if r["fits_from_k"] and k < int(r["fits_from_k"]):
                continue
            if model == "closed-form":
                got = eval(formula, {"__builtins__": {}}, {"k": k})
            elif model == "dyadic-profile":
                prof, q = formula.split("@")
                q = int(q.split("-")[1].rstrip("]"))
                letters = dyadic_letters(k - q)
                got = sum(letters[c] for c in prof)
            else:
                errs.append(f"conjectures m={m}: unknown model {model!r}")
                break
            checked += 1
            if got != col[m]:
                errs.append(f"conjectures m={m} [{model}] at k={k}: predicts {got} "
                            f"but the proven maximum is {col[m]}")

    # generated blocks in the docs --------------------------------------------------
    renderers = {"pareto_sb": lambda: render_pareto_sb(sb),
                 "pareto_sa": lambda: render_pareto_sa(sa),
                 "conjectures": lambda: render_conjectures(cj)}
    root = os.path.normpath(os.path.join(d, ".."))
    rendered = stale = 0
    for path in sorted(glob.glob(os.path.join(root, "**", "*.md"), recursive=True)):
        if os.sep + ".venv" + os.sep in path:
            continue
        text = open(path).read()
        if "<!-- generated:" not in text:
            continue

        def sub(mo):
            nonlocal rendered, stale
            name, body = mo.group(2), mo.group(3)
            if name not in renderers:
                errs.append(f"{os.path.relpath(path, root)}: unknown generated block {name!r}")
                return mo.group(0)
            fresh = renderers[name]()
            rendered += 1
            if fresh != body:
                stale += 1
                if not args.render:
                    errs.append(f"{os.path.relpath(path, root)}: generated block "
                                f"'{name}' is stale - rerun with --render")
            return mo.group(1) + fresh + mo.group(4)

        new = MARK.sub(sub, text)
        if args.render and new != text:
            open(path, "w").write(new)

    # report -----------------------------------------------------------------------
    ncells = sum(len(c) for c in maxv.values())
    print(f"pareto_sb.csv  {len(sb)} rows, {ncells} proven maxima, k={min(maxv)}..{max(maxv)}")
    print(f"pareto_sa.csv  {len(sa)} rows, {len(sa_max)} proven maxima")
    print(f"conjectures    {len(cj)} models, {checked} formula/datum agreements checked")
    print(f"doc blocks     {rendered} generated, "
          f"{'%d rewritten' % stale if args.render else '%d stale' % stale}")
    for w in warns:
        print(f"  WARN  {w}")
    for e in errs:
        print(f"  FAIL  {e}")
    print("\n" + ("all checks passed" if not errs else f"{len(errs)} FAILURE(S)"))
    return 0 if not errs else 1


if __name__ == "__main__":
    sys.exit(main())
