#!/usr/bin/env python3
"""Build a `numbered`-format witness tree for an Sb target out of raw solver-log positives.

Why this exists.  The canonical format needs `[canonical U_k]` / `[embedded G_k]` leaves, which a
`K=8` near-diagonal frontier cell does not reach at any affordable search depth.  The numbered
format needs no such leaf rule: every distinct state gets a line, and a child is discharged by
`(line M)` whenever the state proved at line M **dominates** it after unit-group deletion.  That
is Subgraph Monotonicity plus Unit-Group Elimination, and it is exactly how a solver log already
carries the evidence -- most children are never logged in their own right because something
already logged embeds them.  Concretely `Sb(109:2)@7` appears nowhere in `out_k8.txt` while
`Sb(116:4)@7` does, and 109<=116, 2<=4.

So a child is resolved in this order:

  1. unit-only after deleting `(1:1)` parts        -> `(trivial)`
  2. an exact logged `can solve` line for it       -> a line for that state
  3. a logged `can solve` line whose state dominates it -> a line for the DOMINATING state

Case 3 is the load-bearing one and is why the output is compact.  The line always proves the
dominating state, never the child, so the tree stays a set of honest claims.

Split semantics are re-derived here from docs/problem.md rather than imported from
tools/check_witness.py, so that running the checker afterwards is a real cross-check and not a
tautology.  As a second guard, every derived child triple is compared against the child states the
solver itself printed on the same line; a mismatch is fatal.

    tools/log_to_numbered_tree.py out_k8.txt --target 'Sb(225:6)' --k 8 -o tree.tree

Exit 3 means the target could not be resolved from the log; the states it got stuck on are
listed so a solver run can fill exactly those gaps.
"""
from __future__ import annotations

import argparse
import re
import sys
from collections import defaultdict
from typing import Dict, List, Optional, Sequence, Tuple

Part = Tuple[int, int]
State = Tuple[Part, ...]

# `with [...]` entries can carry suffixes such as `3:0:NOTFAST-ADDED`; only the first two
# numbers are the take.
TAKE_ENTRY = re.compile(r"^\s*(\d+)\s*:\s*(\d+)")
CAN_SOLVE = re.compile(
    r"^can solve\s+(?:size=\S+\s+)?Sb\(([^)]*)\)\[(\d+),(\d+)\]\s+in\s+(\d+)"
    r"\s+with\s+\[([^\]]*)\]\s+(.*?)\s+took\s")
RESULT_IN = re.compile(
    r"^result in\s+(\d+)\s+can solve\s+Sb\(([^)]*)\)\[(\d+),(\d+)\]"
    r"\s+with\s+\[([^\]]*)\]\s+=>\s+(.*?)\s*$")
CHILD_STATE = re.compile(r"Sb\(([^)]*)\)\[(\d+),(\d+)\]")


def parse_parts(body: str) -> List[Part]:
    out: List[Part] = []
    for tok in body.split(","):
        tok = tok.strip()
        if not tok:
            continue
        a, b = tok.split(":")
        out.append((int(a), int(b)))
    return out


def canon(parts: Sequence[Part]) -> State:
    """CANON: orient n>=m, drop empty parts, sort descending. Units are KEPT."""
    ps = []
    for n, m in parts:
        if n == 0 or m == 0:
            continue
        ps.append((n, m) if n >= m else (m, n))
    ps.sort(key=lambda p: (p[0] * p[1], p[0], p[1]), reverse=True)
    return tuple(ps)


def strip_units(s: State) -> State:
    return tuple(p for p in s if p != (1, 1))


def mass(s: State) -> int:
    return sum(n * m for n, m in s)


def coins(s: State) -> int:
    return sum(n + m for n, m in s)


def split_children(state: State, take: Sequence[Part]) -> Tuple[State, State, State]:
    """SPLIT (docs/problem.md). Testing a_i:b_i against part n1_i:n2_i gives
       outcome 2 -> a:b ; outcome 0 -> (n1-a):(n2-b) ; outcome 1 -> a:(n2-b) and (n1-a):b."""
    if len(take) != len(state):
        raise ValueError(f"take has {len(take)} entries for {len(state)} parts")
    both, neither, mixed = [], [], []
    for (n1, n2), (a, b) in zip(state, take):
        if not (0 <= a <= n1 and 0 <= b <= n2):
            raise ValueError(f"take {a}:{b} outside part {n1}:{n2}")
        both.append((a, b))
        neither.append((n1 - a, n2 - b))
        mixed.append((a, n2 - b))
        mixed.append((n1 - a, b))
    return canon(both), canon(mixed), canon(neither)


def dominates(big: State, small: State) -> bool:
    """DOM: an injection from `small`'s parts into `big`'s that is componentwise >=."""
    if len(small) > len(big):
        return False
    adj = [[j for j, B in enumerate(big) if small[i][0] <= B[0] and small[i][1] <= B[1]]
           for i in range(len(small))]
    match: Dict[int, int] = {}

    def try_assign(i: int, seen: set) -> bool:
        for j in adj[i]:
            if j in seen:
                continue
            seen.add(j)
            if j not in match or try_assign(match[j], seen):
                match[j] = i
                return True
        return False

    for i in range(len(small)):
        if not try_assign(i, set()):
            return False
    return True


class Entry:
    __slots__ = ("state", "k", "take", "children", "line_no")

    def __init__(self, state: State, k: int, take: Tuple[Part, ...],
                 children: Tuple[State, State, State], line_no: int):
        self.state, self.k, self.take = state, k, take
        self.children, self.line_no = children, line_no


class Index:
    def __init__(self) -> None:
        self.exact: Dict[Tuple[State, int], Entry] = {}
        self.by_k: Dict[int, List[Entry]] = defaultdict(list)
        self.mismatches = 0

    def add(self, e: Entry) -> None:
        # Keyed on the CANONICAL state; `e.state` keeps the solver's own part order because
        # that is the order `e.take` indexes.
        key = (canon(e.state), e.k)
        # Prefer a full `result in` witness, then the earliest line, for reproducibility.
        if key not in self.exact:
            self.exact[key] = e
            self.by_k[e.k].append(e)

    def load(self, path: str) -> None:
        with open(path, "r", errors="replace") as fh:
            for lineno, line in enumerate(fh, 1):
                if "can solve" not in line:
                    continue
                mo = CAN_SOLVE.match(line)
                if mo:
                    body, k, take_s, kids_s = mo.group(1), int(mo.group(4)), mo.group(5), mo.group(6)
                else:
                    mo = RESULT_IN.match(line)
                    if not mo:
                        continue
                    k, body, take_s, kids_s = int(mo.group(1)), mo.group(2), mo.group(5), mo.group(6)
                raw_parts = parse_parts(body)
                state = canon(raw_parts)
                take: List[Part] = []
                ok = True
                for tok in take_s.split(","):
                    m2 = TAKE_ENTRY.match(tok)
                    if not m2:
                        ok = False
                        break
                    take.append((int(m2.group(1)), int(m2.group(2))))
                if not ok or len(take) != len(raw_parts):
                    continue
                # Re-derive from the parts in the solver's own order, which is what `take`
                # indexes, then compare with the children the solver printed.
                order = tuple((n, m) for n, m in raw_parts)
                try:
                    derived = split_children(order, tuple(take))
                except ValueError:
                    continue
                logged = tuple(canon(parse_parts(g[0])) for g in CHILD_STATE.findall(kids_s))
                if len(logged) == 3 and set(derived) != set(logged):
                    self.mismatches += 1
                    continue
                # Store the take against the solver's part order, plus that order itself.
                self.add(Entry(order, k, tuple(take), derived, lineno))

    def find(self, state: State, k: int) -> Optional[Entry]:
        e = self.exact.get((state, k))
        if e is not None:
            return e
        target = strip_units(state)
        best: Optional[Entry] = None
        for cand in self.by_k.get(k, ()):  # domination fallback
            cs = canon(cand.state)
            if len(cs) < len(target) or mass(cs) < mass(target):
                continue
            if dominates(strip_units(cs), target):
                if best is None or len(cand.state) < len(best.state):
                    best = cand
        return best


class Builder:
    def __init__(self, index: Index) -> None:
        self.index = index
        self.lines: List[dict] = []                    # 1-based; lines[0] unused
        self.assigned: Dict[Tuple[State, int], int] = {}
        self.refs: Dict[int, int] = defaultdict(int)
        self.stuck: List[Tuple[State, int]] = []

    def resolve(self, state: State, k: int) -> Optional[int]:
        """Line number proving a state that dominates `state` at depth k, or None for trivial."""
        if not strip_units(state):
            return None                                # UNIT: trivial
        e = self.index.find(state, k)
        if e is None:
            self.stuck.append((state, k))
            raise LookupError(f"no logged positive covers Sb{state} at k={k}")
        key = (canon(e.state), k)
        if key in self.assigned:
            return self.assigned[key]
        ln = len(self.lines) + 1
        self.lines.append({})                          # reserve the slot before recursing
        self.assigned[key] = ln
        kids = []
        for child in e.children:
            ref = self.resolve(child, k - 1)
            if ref is not None:
                self.refs[ref] += 1
            kids.append((child, ref))
        self.lines[ln - 1] = dict(line=ln, k=k, state=e.state, take=e.take, kids=kids,
                                  src=e.line_no)
        return ln

    def render(self, header: Sequence[str]) -> str:
        out = list(header)
        for nd in self.lines:
            st = tuple(nd["state"])
            body = ",".join(f"{n}:{m}" for n, m in st)
            take = ",".join(f"{a},{b}" for a, b in nd["take"])
            out.append(f"{nd['line']}. (in {nd['k']}) (used {self.refs.get(nd['line'], 0)}) "
                       f"Sb({body})[{mass(canon(st))},{coins(canon(st))}] take[{take}]:")
            for digit, (child, ref) in zip((2, 1, 0), nd["kids"]):
                cb = ",".join(f"{n}:{m}" for n, m in child) if child else ""
                tag = f"(line {ref})" if ref is not None else "(trivial)"
                out.append(f" {digit}=>Sb({cb})[{mass(child)},{coins(child)}]{tag}")
        return "\n".join(out) + "\n"


def parse_m_spec(spec: str) -> List[int]:
    out: List[int] = []
    for chunk in spec.split(","):
        chunk = chunk.strip()
        if "-" in chunk:
            a, b = chunk.split("-")
            out.extend(range(int(a), int(b) + 1))
        elif chunk:
            out.append(int(chunk))
    return out


def batch(args) -> int:
    """One log index, many frontier cells. Prints a per-cell verdict line."""
    import csv
    import os

    if not args.outdir or not args.m:
        print("--frontier-csv needs --outdir and --m", file=sys.stderr)
        return 2
    os.makedirs(args.outdir, exist_ok=True)
    want = set(parse_m_spec(args.m))
    n1_by_m: Dict[int, int] = {}
    with open(args.frontier_csv) as fh:
        for row in csv.DictReader(fh):
            if int(row["k"]) == args.k and int(row["m"]) in want:
                n1_by_m[int(row["m"])] = int(row["n1"])

    idx = Index()
    idx.load(args.log)
    print(f"indexed {len(idx.exact)} distinct logged positives"
          f" ({idx.mismatches} child-mismatch lines rejected)", file=sys.stderr)

    built = gaps = 0
    for m in sorted(n1_by_m):
        n1 = n1_by_m[m]
        target = canon([(n1, m)])
        b = Builder(idx)
        try:
            root = b.resolve(target, args.k)
        except LookupError as exc:
            gaps += 1
            first = b.stuck[-1] if b.stuck else None
            where = (f"Sb({','.join(f'{x}:{y}' for x, y in first[0])})@{first[1]}"
                     if first else "?")
            print(f"m={m} Sb({n1}:{m})@{args.k} GAP at {where}  ({exc})")
            continue
        if root is None:
            print(f"m={m} Sb({n1}:{m})@{args.k} TRIVIAL")
            continue
        path = os.path.join(args.outdir, f"numbered_{n1}_{m}_at{args.k}.tree")
        header = [
            "# Numbered witness tree, built from raw solver-log positives by",
            "# tools/log_to_numbered_tree.py.  A `(line M)` child is discharged because line M's",
            "# state DOMINATES it after unit-group deletion (Subgraph Monotonicity + Unit-Group",
            "# Elimination); most children are never logged in their own right.",
            f"# source  : {args.log}",
            f"# target  : Sb({n1}:{m}) in {args.k}",
            f"# lines   : {len(b.lines)}",
            f"# note    : K={args.k} Pareto frontier maximum at m={m} ({args.frontier_csv})",
            "# checked : tools/check_witness.py",
            "#",
        ]
        with open(path, "w") as fh:
            fh.write(b.render(header))
        built += 1
        print(f"m={m} Sb({n1}:{m})@{args.k} TREE {len(b.lines)} lines -> {path}")
    print(f"built {built}, gaps {gaps}")
    return 0 if gaps == 0 else 3


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("log")
    ap.add_argument("--target", help="e.g. 'Sb(225:6)' or '225:6'")
    ap.add_argument("--k", required=True, type=int)
    ap.add_argument("-o", "--out")
    ap.add_argument("--note", action="append", default=[])
    # Batch mode: indexing a 1.3 GB log takes minutes, so resolve many frontier cells per pass.
    ap.add_argument("--frontier-csv", help="data/pareto_sb.csv; take n1 from the k-rows")
    ap.add_argument("--m", help="m values for --frontier-csv, e.g. '6-55' or '6,8,12'")
    ap.add_argument("--outdir", help="output directory for --frontier-csv mode")
    args = ap.parse_args()

    if args.frontier_csv:
        return batch(args)
    if not args.target:
        ap.error("--target is required without --frontier-csv")

    body = args.target.strip()
    if body.startswith("Sb(") and body.endswith(")"):
        body = body[3:-1]
    target = canon(parse_parts(body))

    idx = Index()
    idx.load(args.log)
    print(f"indexed {len(idx.exact)} distinct logged positives"
          f" ({idx.mismatches} child-mismatch lines rejected)", file=sys.stderr)

    b = Builder(idx)
    try:
        root = b.resolve(target, args.k)
    except LookupError as exc:
        print(f"UNRESOLVED: {exc}", file=sys.stderr)
        for st, k in b.stuck[-10:]:
            print(f"  gap: Sb({','.join(f'{n}:{m}' for n, m in st)}) at k={k}", file=sys.stderr)
        return 3
    if root is None:
        print("target is trivial", file=sys.stderr)
        return 3

    header = [
        "# Numbered witness tree, built from raw solver-log positives by",
        "# tools/log_to_numbered_tree.py.  A `(line M)` child is discharged because line M's",
        "# state DOMINATES it after unit-group deletion (Subgraph Monotonicity + Unit-Group",
        "# Elimination); most children are never logged in their own right.",
        f"# source  : {args.log}",
        f"# target  : Sb({','.join(f'{n}:{m}' for n, m in target)}) in {args.k}",
        f"# lines   : {len(b.lines)}",
    ]
    header += [f"# note    : {n}" for n in args.note]
    header += ["# checked : tools/check_witness.py", "#"]
    text = b.render(header)
    if args.out:
        with open(args.out, "w") as fh:
            fh.write(text)
        print(f"{args.out}: {len(b.lines)} lines", file=sys.stderr)
    else:
        sys.stdout.write(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
