#!/usr/bin/env python3
"""Measure how well a rule can pick a winning split, on the choice-census corpora.

This reproduces the 2026-08-18 journal measurements.  It answers two separate questions and
deliberately keeps them apart:

  * how many candidate splits survive a *sound necessary* condition (a filter), and
  * where the true winner lands when candidates are *ordered* by a scalar feature.

Winner labels come from the census itself.  ``CENSUS FULL_WIN`` is the exact, complete verdict
set for each endpoint -- the enumerator's own prunes are all necessary conditions and the leaf
test is exact -- so a candidate is a winner precisely when it appears there.  No solver call is
needed to label anything, and every filter below must therefore keep 100% of winners.  A recall
below 100% is a bug in this tool (or a disagreement worth chasing), not a selectivity result.

Why the census's own ``complete=`` count is not used as the candidate count: ``enumerate_rec``
prunes with ``CACHE_ONLY`` lookups against a warm dominance cache, so it depends on cache
history rather than on the state.  Everything here re-derives the candidate set from scratch
using only cache-free conditions.

Filters, all sound:

  cap        every child's information mass is at most ``3**(k-1)``
  frontier   each of the four rectangles a component cut induces lies on or below the proven
             one-part frontier at the child level (Subgraph Monotonicity)
  r0         full-star majorization of each child against ``G_k`` (Singleton Majorization)
  pair       every *cross-part* pair of each child's components is jointly solvable at the
             child level.  This is the condition the solver does NOT have: ``radiobase.c``'s
             ``s[4]``/``s[5]`` loop tests one part, and the two mixed rectangles of that same
             part, but never two different parts together.
  r1         the depth-1 relaxation ``R_1`` of tools/bundled_majorization.py

``pair`` needs an exact small-state oracle, built by the ``table`` mode with the independent
``tools/refsolve.py`` rather than with the C solver, so filter agreement is evidence.

Usage:
    tools/split_choice_rules.py single  CENSUS.out
    tools/split_choice_rules.py table   K [--parts 2] [--out FILE]
    tools/split_choice_rules.py ladder  CENSUS.out [--parts 4] [--limit N] [--table FILE]
    tools/split_choice_rules.py rank    CENSUS.out [--parts 4] [--limit N]

CENSUS.out may be a partial log: only endpoints whose ``FULL_SUMMARY`` is present and whose
winner count matches it are used.
"""

from __future__ import annotations

import argparse
import ast
import csv
import importlib.util
import json
import statistics
import sys
import time
from collections import Counter, defaultdict
from itertools import combinations, combinations_with_replacement
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def _load(name: str):
    """Import a sibling tool by path, registering it so dataclasses resolve."""
    spec = importlib.util.spec_from_file_location(name, ROOT / "tools" / f"{name}.py")
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


apc = _load("analyze_pareto_prefix_census")
bm = _load("bundled_majorization")


def proven_frontier() -> dict[tuple[int, int], int]:
    front = {}
    with (ROOT / "data" / "pareto_sb.csv").open(newline="") as handle:
        for row in csv.DictReader(handle):
            if row["bound"] == "max" and row["status"].startswith("proven"):
                front[int(row["k"]), int(row["m"])] = int(row["n1"])
    return front


class Corpus:
    """The endpoint layer of one choice census, with exact winner labels."""

    def __init__(self, path: Path):
        records = defaultdict(list)
        with path.open(errors="replace") as handle:
            for line in handle:
                parsed = apc.fields(line)
                if parsed:
                    records[parsed[0]].append(parsed[1])
        if not records["BEGIN"]:
            raise SystemExit(f"{path}: no CENSUS BEGIN record")
        self.path = path
        self.residual_k = int(records["BEGIN"][0]["residual_k"])
        self.root_k = int(records["BEGIN"][0]["root_k"])
        # The endpoint state itself lives at `residual_k` and is bounded by 3**residual_k, but a
        # *candidate split* is judged by its children, which live one level down.  The cap that
        # filters candidates is therefore the child bound.  Using the parent's bound here silently
        # admits candidates no child could satisfy; `r0` then re-imposes the right cap, so only the
        # pre-majorization columns look wrong, which is easy to miss.
        self.state_cap = 3 ** self.residual_k
        self.child_k = self.residual_k - 1
        self.cap = 3 ** self.child_k
        self.states = {r["id"]: apc.pairs(r["state"]) for r in records["FULL_STATE"]}
        summaries = {r["id"]: r for r in records["FULL_SUMMARY"]}
        self.summaries = summaries
        self.classes: dict[str, set] = defaultdict(set)
        self.winners: dict[str, set] = defaultdict(set)
        raw = Counter()
        for record in records["FULL_WIN"]:
            key = record["id"]
            if key not in self.states:
                continue
            take = apc.pairs(record["take"])
            self.classes[key].add(apc.cut_class(self.states[key], take))
            self.winners[key].add(take)
            raw[key] += 1
        # A partial log's last endpoint can have an unterminated map; require agreement.
        self.usable = sorted(
            key for key, record in summaries.items()
            if key in self.states and int(record["winners"]) == raw[key]
        )
        self.raw = raw

    def single(self, parts: int | None = None) -> list[str]:
        out = [k for k in self.usable if len(self.classes[k]) == 1]
        if parts is not None:
            out = [k for k in out if len(self.states[k]) == parts]
        return out


class Filters:
    def __init__(self, corpus: Corpus, table: dict | None = None):
        self.c = corpus
        self.front = proven_frontier()
        self.table = table or {}

    def rect_ok(self, u: int, v: int) -> bool:
        if u < v:
            u, v = v, u
        if v == 0 or u * v <= 1:
            return True
        limit = self.front.get((self.c.child_k, v))
        return True if limit is None else u <= limit

    def cut_ok(self, n: int, m: int, a: int, b: int) -> bool:
        return (self.rect_ok(a, b) and self.rect_ok(n - a, m - b)
                and self.rect_ok(a, m - b) and self.rect_ok(n - a, b))

    def pair_ok(self, state) -> bool:
        for x, y in combinations(state, 2):
            if self.table.get(bm.normalize([x, y])) is False:
                return False
        return True

    def candidates(self, parts):
        """Every cap+frontier feasible split, with its three normalized children."""
        size = len(parts)
        out = []

        def walk(i, take, m0, m1, m2):
            if i == size:
                selected, mixed, complement = [], [], []
                for (n, m), (a, b) in zip(parts, take):
                    selected.append((a, b))
                    complement.append((n - a, m - b))
                    mixed.append((a, m - b))
                    mixed.append((n - a, b))
                out.append((tuple(take), bm.normalize(selected),
                            bm.normalize(mixed), bm.normalize(complement)))
                return
            n, m = parts[i]
            for a in range(n + 1):
                for b in range(m + 1):
                    s, x = a * b, a * (m - b) + (n - a) * b
                    c = (n - a) * (m - b)
                    if m0 + s > self.c.cap or m1 + x > self.c.cap or m2 + c > self.c.cap:
                        continue
                    if not self.cut_ok(n, m, a, b):
                        continue
                    take.append((a, b))
                    walk(i + 1, take, m0 + s, m1 + x, m2 + c)
                    take.pop()

        walk(0, [], 0, 0, 0)
        return out

    def r0(self, children) -> bool:
        return all(bm.r0(child, self.c.child_k) for child in children)

    def pairs(self, children) -> bool:
        return all(self.pair_ok(child) for child in children)

    def r1(self, children) -> bool:
        return all(bm.relax(child, self.c.child_k, 1) for child in children)


def margin(state, k: int) -> int:
    """Slack in the tightest star-majorization prefix inequality."""
    profile = sorted((n for n, m in state for _ in range(m)), reverse=True)
    g = bm.base(k)
    left = right = 0
    best = None
    for i, value in enumerate(profile[:len(g)]):
        left += value
        right += g[i]
        if best is None or right - left < best:
            best = right - left
    return 0 if best is None else best


def features(parts, take, children, cap, child_k, front):
    selected, mixed, complement = children
    masses = [sum(n * m for n, m in child) for child in children]
    deficits = []
    all_or_nothing = 0
    for (n, m), (a, b) in zip(parts, take):
        if a in (0, n) or b in (0, m):
            all_or_nothing += 1
        for p, q in ((a, b), (n - a, m - b), (a, m - b), (n - a, b)):
            u, v = (p, q) if p >= q else (q, p)
            if v and u * v > 1 and (child_k, v) in front:
                deficits.append(front[child_k, v] - u)
    margins = [margin(child, child_k) for child in children]
    return {
        "take": take,
        "slack": cap - max(masses),
        "spread": max(masses) - min(masses),
        "dev": sum(abs(a * m - b * n) for (n, m), (a, b) in zip(parts, take)),
        "minmargin": min(margins),
        "summargin": sum(margins),
        "mixmargin": margins[1],
        "mixmass": masses[1],
        "mixparts": len(mixed),
        "mixdistinct": len(set(mixed)),
        "alldistinct": len(set(selected) | set(mixed) | set(complement)),
        "worstdef": min(deficits) if deficits else 10 ** 6,
        "sumdef": sum(deficits),
        "aon": all_or_nothing,
    }


ORDERINGS = {
    "slack asc (tightness)": lambda c: (c["slack"],),
    "dev asc": lambda c: (c["dev"],),
    "dev then slack": lambda c: (c["dev"], c["slack"]),
    "spread asc": lambda c: (c["spread"],),
    "minmargin desc": lambda c: (-c["minmargin"],),
    "minmargin asc": lambda c: (c["minmargin"],),
    "summargin desc": lambda c: (-c["summargin"],),
    "mixmargin desc": lambda c: (-c["mixmargin"],),
    "mixmass asc": lambda c: (c["mixmass"],),
    "mixparts asc": lambda c: (c["mixparts"],),
    "mixdistinct asc": lambda c: (c["mixdistinct"],),
    "alldistinct asc": lambda c: (c["alldistinct"],),
    "worstdef desc": lambda c: (-c["worstdef"],),
    "worstdef asc": lambda c: (c["worstdef"],),
    "sumdef desc": lambda c: (-c["sumdef"],),
    "aon desc": lambda c: (-c["aon"],),
    "aon asc": lambda c: (c["aon"],),
}


def cmd_single(args) -> int:
    corpus = Corpus(args.census)
    single = corpus.single()
    print(f"{corpus.path.name}: root_k={corpus.root_k} residual_k={corpus.residual_k} "
          f"state_cap={corpus.state_cap} child_k={corpus.child_k} child_cap={corpus.cap}")
    print(f"endpoints with a usable full map: {len(corpus.usable)} "
          f"(of {len(corpus.states)} FULL_STATE records)")
    print(f"single automorphism class:        {len(single)} "
          f"({100 * len(single) / max(1, len(corpus.usable)):.1f}%)")
    print(f"part counts, all usable:  {dict(sorted(Counter(len(corpus.states[k]) for k in corpus.usable).items()))}")
    print(f"part counts, single-class: {dict(sorted(Counter(len(corpus.states[k]) for k in single).items()))}")
    print(f"raw labelled winners at single-class endpoints: "
          f"{dict(sorted(Counter(corpus.raw[k] for k in single).items()))}")
    classes = [len(corpus.classes[k]) for k in corpus.usable]
    print(f"classes per endpoint: median {statistics.median(classes):.0f} max {max(classes)}")
    census_complete = [int(corpus.summaries[k]["complete"]) for k in single]
    print(f"census `complete=` at single-class endpoints (a CACHE-DEPENDENT effort count, "
          f"not the size of the choice problem): median {statistics.median(census_complete):.0f} "
          f"max {max(census_complete)}")
    return 0


def cmd_table(args) -> int:
    """Exact small-state oracle at level K, from the independent reference solver."""
    refsolve = _load("refsolve")
    front = proven_frontier()
    cap = 3 ** args.k
    singles = [(u, v) for v in range(1, cap + 1) for u in range(v, cap + 1)
               if u * v <= cap and (args.k, v) in front and u <= front[args.k, v]]
    print(f"k={args.k} cap={cap}: {len(singles)} individually-solvable components", flush=True)
    bad = 0
    for v in range(1, 9):
        if (args.k, v) not in front:
            continue
        n = front[args.k, v]
        if not refsolve.solvable(refsolve.norm([(n, v)]), args.k):
            print(f"  FRONTIER MISS {n}:{v}")
            bad += 1
        if refsolve.solvable(refsolve.norm([(n + 1, v)]), args.k):
            print(f"  FRONTIER OVERSHOOT {n + 1}:{v}")
            bad += 1
    if bad:
        raise SystemExit("refsolve disagrees with the proven one-part frontier; not writing a table")
    print("one-part frontier self-check against refsolve: OK", flush=True)

    start = time.time()
    table: dict = {}
    # Identical components must be included: a child routinely repeats a part, and the mixed
    # child especially so.  Plain `combinations` would silently leave those states out of the
    # table, which makes every lookup on them a miss and quietly weakens the filter.
    for combo in combinations_with_replacement(singles, args.parts):
        if sum(u * v for u, v in combo) > cap:
            continue
        state = bm.normalize(list(combo))
        if state in table:
            continue
        table[state] = refsolve.solvable(state, args.k)
        if len(table) % 2000 == 0:
            print(f"  {len(table)} states, {time.time() - start:.0f}s", flush=True)
    unsolvable = sum(1 for v in table.values() if not v)
    print(f"{args.parts}-part states at k={args.k}: {len(table)}  "
          f"solvable {len(table) - unsolvable}  unsolvable {unsolvable}  "
          f"({time.time() - start:.0f}s)")
    out = args.out or ROOT / "tools" / "testdata" / f"exact_{args.parts}part_k{args.k}.json"
    Path(out).write_text(json.dumps(
        {"k": args.k, "parts": args.parts, "table": {repr(k): v for k, v in table.items()}}))
    print(f"wrote {out}")
    return 0


def load_table(path: Path) -> dict:
    blob = json.loads(Path(path).read_text())
    return {ast.literal_eval(k): v for k, v in blob["table"].items()}


def cmd_ladder(args) -> int:
    corpus = Corpus(args.census)
    table = load_table(args.table) if args.table else {}
    filters = Filters(corpus, table)
    endpoints = corpus.single(args.parts)[:args.limit]
    if not table:
        print("no --table given: the cross-part pair column is skipped", file=sys.stderr)
    totals = Counter()
    print(f"{'endpoint':10} {'cap+front':>10} {'+r0':>8} {'+pair':>8} {'+both':>8} "
          f"{'win':>4} {'kept':>5}")
    start = time.time()
    for key in endpoints:
        parts = corpus.states[key]
        wins = corpus.winners[key]
        counts = Counter()
        kept = 0
        for take, *children in filters.candidates(parts):
            counts["base"] += 1
            r0 = filters.r0(children)
            pair = filters.pairs(children) if table else False
            if r0:
                counts["r0"] += 1
            if table and pair:
                counts["pair"] += 1
            if r0 and (pair if table else True):
                counts["both"] += 1
                if take in wins:
                    kept += 1
        print(f"{key:10} {counts['base']:10d} {counts['r0']:8d} {counts['pair']:8d} "
              f"{counts['both']:8d} {len(wins):4d} {kept:5d}", flush=True)
        totals.update(counts)
        totals["win"] += len(wins)
        totals["kept"] += kept
        if args.seconds and time.time() - start > args.seconds:
            print("(time budget reached)")
            break

    def ratio(stage):
        return totals["base"] / totals[stage] if totals[stage] else float("inf")

    print(f"\nsound cache-free filter ladder, {args.parts}-part single-class endpoints:")
    print(f"  cap + four-rectangle frontier   {totals['base']}")
    print(f"  + r0 full-star majorization     {totals['r0']}   ({ratio('r0'):.1f}x)")
    if table:
        print(f"  + cross-part pair solvability   {totals['pair']}   ({ratio('pair'):.1f}x)")
        print(f"  + both                          {totals['both']}   ({ratio('both'):.1f}x)")
        independent = ratio("r0") * ratio("pair")
        print(f"  (independent would be {independent:.1f}x; measured "
              f"{'super' if ratio('both') > independent else 'sub'}-multiplicative)")
    print(f"  winners kept {totals['kept']}/{totals['win']} -- every filter is a necessary "
          f"condition, so anything below 100% is a bug")
    if totals["both"]:
        print(f"  precision {100 * totals['kept'] / totals['both']:.3f}%   "
              f"candidates per winner {totals['both'] / max(1, totals['kept']):.0f}")
    return 0


def cmd_rank(args) -> int:
    corpus = Corpus(args.census)
    filters = Filters(corpus)
    front = filters.front
    endpoints = corpus.single(args.parts)[:args.limit]
    ranks = defaultdict(list)
    fractions = defaultdict(list)
    sizes = []
    start = time.time()
    for key in endpoints:
        parts = corpus.states[key]
        wins = corpus.winners[key]
        rows = []
        for take, *children in filters.candidates(parts):
            if not filters.r0(children):
                continue
            rows.append(features(parts, take, children, corpus.cap, corpus.child_k, front))
        if not rows or not any(r["take"] in wins for r in rows):
            print(f"  !! {key}: no winner among r0 survivors", file=sys.stderr)
            continue
        sizes.append(len(rows))
        for name, key_fn in ORDERINGS.items():
            order = sorted(rows, key=key_fn)
            rank = min(i for i, row in enumerate(order, 1) if row["take"] in wins)
            ranks[name].append(rank)
            fractions[name].append(rank / len(rows))
        if args.seconds and time.time() - start > args.seconds:
            print("(time budget reached)", file=sys.stderr)
            break
    if not sizes:
        raise SystemExit("no endpoints measured")
    print(f"\n{corpus.path.name}: {len(sizes)} single-class {args.parts}-part endpoints")
    print(f"r0-feasible candidates: median {statistics.median(sizes):.0f} "
          f"p90 {sorted(sizes)[int(0.9 * (len(sizes) - 1))]} max {max(sizes)} total {sum(sizes)}")
    print(f"\n{'ordering':24} {'medRank':>8} {'p90':>8} {'worst':>8} {'medFrac':>8} "
          f"{'top10':>6} {'top1%':>6}")
    for name in sorted(ORDERINGS, key=lambda n: statistics.median(fractions[n])):
        values = sorted(ranks[name])
        n = len(values)
        print(f"{name:24} {statistics.median(values):8.0f} "
              f"{values[int(0.9 * (n - 1))]:8d} {max(values):8d} "
              f"{statistics.median(fractions[name]):8.3f} "
              f"{sum(1 for v in values if v <= 10):6d} "
              f"{sum(1 for f in fractions[name] if f <= 0.01):6d}")
    print("\nRandom ordering gives medFrac ~0.5/(number of winners).  A rule needs medFrac near 0,"
          "\nnot merely below 0.5; see the refutation trap in docs/status.md.")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = parser.add_subparsers(dest="command", required=True)

    p = sub.add_parser("single", help="report the single-solution layer of a census")
    p.add_argument("census", type=Path)
    p.set_defaults(func=cmd_single)

    p = sub.add_parser("table", help="build an exact small-state oracle with refsolve")
    p.add_argument("k", type=int)
    p.add_argument("--parts", type=int, default=2)
    p.add_argument("--out", type=Path)
    p.set_defaults(func=cmd_table)

    p = sub.add_parser("ladder", help="measure sound cache-free filter selectivity")
    p.add_argument("census", type=Path)
    p.add_argument("--parts", type=int, default=4)
    p.add_argument("--limit", type=int, default=25)
    p.add_argument("--table", type=Path, help="exact 2-part oracle from `table` mode")
    p.add_argument("--seconds", type=float, default=0.0)
    p.set_defaults(func=cmd_ladder)

    p = sub.add_parser("rank", help="measure where scalar orderings put the winner")
    p.add_argument("census", type=Path)
    p.add_argument("--parts", type=int, default=4)
    p.add_argument("--limit", type=int, default=200)
    p.add_argument("--seconds", type=float, default=0.0)
    p.set_defaults(func=cmd_rank)

    args = parser.parse_args()
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
