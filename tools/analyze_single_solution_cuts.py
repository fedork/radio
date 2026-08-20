#!/usr/bin/env python3
"""Compare the single-solution endpoint cuts of two prefix censuses, with controls.

An endpoint of ``pareto_prefix_census`` is a maximal residual state; the census records every
winning split of it as a ``FULL_WIN``.  Quotienting those splits by the exact automorphisms of the
state (identical-component permutation, square-shore exchange, global complementation) gives the
*classes*.  An endpoint with exactly one class has essentially one way to be solved, and those
forced cuts are what this script is about.

Two things this exists to answer:

* Does anything cheap separate the forced endpoints from the rest?  Every comparison below is run
  against a control population — the multi-solution endpoints, over *all* their classes, since
  picking one representative silently biases the comparison.
* Is there a connection between levels?  A ``root_k=8`` census has endpoints at residual `k=6`
  whose children live at `k=5` — exactly the level of a ``root_k=7`` census's own endpoints.  So
  the two corpora meet, and we can ask whether a forced k=8 cut lands on a state the k=7 census
  already knows.

Equivalence semantics are imported from ``analyze_pareto_prefix_census`` rather than reimplemented,
so a change there cannot silently desynchronize this.  Run:

    tools/analyze_single_solution_cuts.py --k8 <k8.out> --k7 <k7.out>
"""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import json
import math
from pathlib import Path
import random
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))

from analyze_pareto_prefix_census import (  # noqa: E402
    Pair,
    cut_class,
    fields,
    mass,
    pairs,
    semantic_state,
)

CHILD_NAMES = ("selected", "mixed", "complement")


def children(parts: tuple[Pair, ...], take: tuple[Pair, ...]):
    """The three outcome children of a split, as canonical semantic states.

    outcome 2 -> (a:b); outcome 0 -> (n-a:m-b); outcome 1 -> (a:m-b) and (n-a:b).
    Verified against the census's own selected/mixed/complement fields.
    """
    selected = list(take)
    complement = [(n - a, m - b) for (n, m), (a, b) in zip(parts, take)]
    mixed: list[Pair] = []
    for (n, m), (a, b) in zip(parts, take):
        mixed.append((a, m - b))
        mixed.append((n - a, b))
    return semantic_state(selected), semantic_state(mixed), semantic_state(complement)


def load(path: Path):
    states: dict[str, tuple[Pair, ...]] = {}
    meta: dict[str, dict[str, int]] = {}
    wins: defaultdict[str, list[tuple[Pair, ...]]] = defaultdict(list)
    residual_k = None
    with path.open() as handle:
        for line in handle:
            parsed = fields(line)
            if parsed is None:
                continue
            kind, data = parsed
            if kind == "BEGIN":
                residual_k = int(data["residual_k"])
            elif kind == "FULL_STATE":
                states[data["id"]] = pairs(data["state"])
                meta[data["id"]] = {k: int(data[k]) for k in ("parts", "units", "mass")}
            elif kind == "FULL_WIN":
                wins[data["id"]].append(pairs(data["take"]))
    if residual_k is None:
        raise SystemExit(f"{path}: no CENSUS BEGIN record")
    return residual_k, states, meta, wins


def classify(states, wins):
    out: dict[str, dict[tuple[Pair, ...], tuple[Pair, ...]]] = {}
    for endpoint, parts in states.items():
        found: dict[tuple[Pair, ...], tuple[Pair, ...]] = {}
        for take in wins.get(endpoint, ()):
            found.setdefault(cut_class(parts, take), take)
        out[endpoint] = found
    return out


def orient(part: Pair, cut: Pair) -> tuple[Pair, Pair]:
    """Write the part with n>=m, carrying its cut along, so shapes are comparable."""
    (n, m), (a, b) = part, cut
    return ((m, n), (b, a)) if n < m else ((n, m), (a, b))


def cut_kind(part: Pair, cut: Pair) -> str:
    """Archetype of a single part's cut.  Categories are exclusive, tested in this order."""
    (n, m), (a, b) = orient(part, cut)
    if (a, b) == (n, m) or (a, b) == (0, 0):
        return "whole-part"
    if a * b == 0 or (n - a) * (m - b) == 0:
        return "sliver"           # contributes nothing to one of the two pure outcomes
    if a * m == b * n:
        return "proportional"     # exactly diagonal
    return "generic"


def dominated_by(small, big) -> bool:
    """Injection from small's parts into big's, componentwise <=, either orientation."""
    if len(small) > len(big):
        return False
    small = sorted(small, key=lambda p: -p[0] * p[1])

    def visit(i, used):
        if i == len(small):
            return True
        sn, sm = small[i]
        for j, (bn, bm) in enumerate(big):
            if used >> j & 1:
                continue
            if ((sn <= bn and sm <= bm) or (sm <= bn and sn <= bm)) and visit(i + 1, used | 1 << j):
                return True
        return False

    return visit(0, 0)


def quantiles(values):
    v = sorted(values)
    if not v:
        return {}
    return {
        "n": len(v), "min": v[0], "p10": v[len(v) // 10], "p25": v[len(v) // 4],
        "med": v[len(v) // 2], "p75": v[3 * len(v) // 4], "p90": v[9 * len(v) // 10],
        "max": v[-1],
    }


class Corpus:
    def __init__(self, path: Path, label: str):
        self.label = label
        self.path = path
        self.rk, self.states, self.meta, self.wins = load(path)
        self.cls = classify(self.states, self.wins)
        self.cap = 3 ** self.rk
        self.cap_child = 3 ** (self.rk - 1)
        self.single = {e for e, c in self.cls.items() if len(c) == 1}
        self.multi = {e for e, c in self.cls.items() if len(c) > 1}

    def four(self, pop):
        return [e for e in pop if self.meta[e]["parts"] == 4]

    def classes_of(self, pop, all_classes: bool):
        """(endpoint, representative take) pairs.  all_classes=False takes one per endpoint."""
        for e in self.four(pop):
            reps = list(self.cls[e].values())
            for rep in (reps if all_classes else reps[:1]):
                yield e, rep


def population_report(c: Corpus) -> dict:
    return {
        "path": str(c.path),
        "residual_k": c.rk,
        "cap": c.cap,
        "cap_child": c.cap_child,
        "endpoints": len(c.states),
        "raw_winners": sum(len(v) for v in c.wins.values()),
        "classes": sum(len(v) for v in c.cls.values()),
        "single_class_endpoints": len(c.single),
        "single_class_share": round(len(c.single) / len(c.states), 4),
        "endpoints_by_parts": dict(sorted(Counter(m["parts"] for m in c.meta.values()).items())),
        "single_by_parts": dict(sorted(Counter(c.meta[e]["parts"] for e in c.single).items())),
        "raw_winners_of_single_four": dict(sorted(
            Counter(len(c.wins[e]) for e in c.four(c.single)).items())),
    }


def archetypes(c: Corpus) -> dict:
    out = {}
    for name, pop, all_cls in (("single", c.single, False), ("multi", c.multi, True)):
        kinds, per_class = Counter(), Counter()
        nclass = 0
        for e, rep in c.classes_of(pop, all_cls):
            nclass += 1
            slivers = 0
            for part, cut in zip(c.states[e], rep):
                k = cut_kind(part, cut)
                kinds[k] += 1
                slivers += k == "sliver"
            per_class[slivers] += 1
        total = sum(kinds.values())
        out[name] = {
            "classes": nclass,
            "part_cuts": total,
            "share": {k: round(kinds[k] / total, 4) for k in
                      ("whole-part", "sliver", "proportional", "generic")},
            "slivers_per_class": {d: round(v / nclass, 4) for d, v in sorted(per_class.items())},
        }
    s, m = out["single"], out["multi"]
    p1, n1 = s["share"]["sliver"], s["part_cuts"]
    p2, n2 = m["share"]["sliver"], m["part_cuts"]
    p = (p1 * n1 + p2 * n2) / (n1 + n2)
    se = math.sqrt(p * (1 - p) * (1 / n1 + 1 / n2))
    out["sliver_enrichment_z"] = round((p1 - p2) / se, 2)
    return out


def geometry(c: Corpus, rng: random.Random) -> dict:
    """Child-mass structure of winning splits, against random and cap-feasible controls."""
    win_largest, spreads, at_cap, margins = Counter(), [], 0, []
    for e, rep in c.classes_of(c.single | c.multi, True):
        ms = [mass(k) for k in children(c.states[e], rep)]
        win_largest[CHILD_NAMES[ms.index(max(ms))]] += 1
        spreads.append(max(ms) - min(ms))
        margins.append(ms[1] - max(ms[0], ms[2]))
        at_cap += max(ms) == c.cap_child
    rnd, feas = Counter(), Counter()
    for e in c.four(c.single | c.multi):
        st = c.states[e]
        for _ in range(20):
            cut = tuple((rng.randint(0, n), rng.randint(0, m)) for n, m in st)
            ms = [mass(k) for k in children(st, cut)]
            rnd[CHILD_NAMES[ms.index(max(ms))]] += 1
            if max(ms) <= c.cap_child:
                feas[CHILD_NAMES[ms.index(max(ms))]] += 1
    return {
        "winning_classes": sum(win_largest.values()),
        "winning_largest_child": dict(win_largest),
        "mixed_largest_share_winning": round(win_largest["mixed"] / sum(win_largest.values()), 4),
        "mixed_margin": quantiles(margins),
        "child_spread": quantiles(spreads),
        "classes_with_a_child_at_cap": at_cap,
        "control_random": {"n": sum(rnd.values()),
                           "mixed_largest_share": round(rnd["mixed"] / sum(rnd.values()), 4)},
        "control_cap_feasible": {"n": sum(feas.values()),
                                 "mixed_largest_share": round(feas["mixed"] / sum(feas.values()), 4)
                                 if sum(feas.values()) else None},
    }


def scale_profile(c: Corpus) -> dict:
    root = math.sqrt(c.cap)
    ns, ms, sizes, aspects = [], [], [], []
    for e in c.four(c.single | c.multi):
        for (n, m) in c.states[e]:
            n, m = max(n, m), min(n, m)
            ns.append(round(n / root, 4)); ms.append(round(m / root, 4))
            sizes.append(round(n * m / c.cap, 4)); aspects.append(round(n / m, 3))
    return {
        "n_over_sqrt_cap": quantiles(ns), "m_over_sqrt_cap": quantiles(ms),
        "part_size_over_cap": quantiles(sizes), "aspect": quantiles(aspects),
        "state_occupancy_single": quantiles(
            [round(c.meta[e]["mass"] / c.cap, 4) for e in c.four(c.single)]),
        "state_occupancy_multi": quantiles(
            [round(c.meta[e]["mass"] / c.cap, 4) for e in c.four(c.multi)]),
    }


def symmetry_control(c: Corpus) -> dict:
    table: defaultdict[str, list[int]] = defaultdict(lambda: [0, 0])
    for e in c.four(c.single | c.multi):
        counts = Counter(tuple(sorted(p, reverse=True)) for p in c.states[e])
        key = f"repeats={sum(v - 1 for v in counts.values())},squares=" \
              f"{sum(1 for n, m in c.states[e] if n == m)}"
        table[key][0] += 1
        table[key][1] += len(c.cls[e]) == 1
    return {k: {"states": v[0], "single": v[1], "rate": round(v[1] / v[0], 3)}
            for k, v in sorted(table.items())}


def shape_concentration(c: Corpus, min_seen: int = 8) -> dict:
    shape_cuts: defaultdict[Pair, Counter] = defaultdict(Counter)
    for e, rep in c.classes_of(c.single, False):
        for part, cut in zip(c.states[e], rep):
            p, cc = orient(part, cut)
            shape_cuts[p][cc] += 1
    common = [(p, cs) for p, cs in shape_cuts.items() if sum(cs.values()) >= min_seen]
    rows = []
    for p, cs in sorted(common, key=lambda x: -max(x[1].values()) / sum(x[1].values())):
        top, cnt = cs.most_common(1)[0]
        rows.append({"shape": f"{p[0]}:{p[1]}", "seen": sum(cs.values()),
                     "distinct_cuts": len(cs), "top_cut": f"{top[0]}:{top[1]}",
                     "top_share": round(cnt / sum(cs.values()), 3)})
    return {"distinct_shapes": len(shape_cuts),
            "shapes_seen_at_least": min_seen,
            "shapes_qualifying": len(common),
            "most_concentrated": rows[:15]}


def cross_level(hi: Corpus, lo: Corpus) -> dict:
    lo_states = {semantic_state(s) for s in lo.states.values()}
    lo_single = {semantic_state(lo.states[e]) for e in lo.single}
    lo_list = list(lo_states)
    lo_single_list = list(lo_single)
    lo_max_mass = max(lo.meta[e]["mass"] for e in lo.states)
    lo_max_parts = max(len(s) for s in lo.states.values())

    out = {
        "lower_endpoint_states": len(lo_states),
        "lower_single_solution_states": len(lo_single),
        "lower_max_mass": lo_max_mass,
        "lower_max_parts": lo_max_parts,
    }
    for scope, pop, all_cls in (("single_four", hi.single, False), ("all_four", hi.single | hi.multi, False)):
        ident = Counter(); dom = Counter(); eligible = 0; nkids = 0; nstates = 0
        hits = []
        for e, rep in hi.classes_of(pop, all_cls):
            nstates += 1
            for name, kid in zip(CHILD_NAMES, children(hi.states[e], rep)):
                nkids += 1
                if mass(kid) <= lo_max_mass and len(kid) <= lo_max_parts:
                    eligible += 1
                if kid in lo_states:
                    ident[name] += 1
                    hits.append({"upper": e, "child": name,
                                 "state": ",".join(f"{n}:{m}" for n, m in kid),
                                 "child_is_lower_single": kid in lo_single})
                if any(dominated_by(kid, big) for big in lo_list):
                    dom[name] += 1
        out[scope] = {
            "states": nstates, "children": nkids,
            "children_passing_necessary_conditions": eligible,
            "children_identical_to_a_lower_endpoint": sum(ident.values()),
            "identical_by_child": dict(ident),
            "children_dominated_by_a_lower_endpoint": sum(dom.values()),
            "dominated_by_child": dict(dom),
            "hits": hits[:10],
        }
    return out


def child_profile(hi: Corpus, lo: Corpus) -> dict:
    """Shape profiles of the two populations that meet at the lower level."""
    def prof(states):
        aspects, shorts, parts = [], [], Counter()
        for s in states:
            if not s:
                continue
            parts[len(s)] += 1
            shorts.append(min(min(p) for p in s))
            for n, m in s:
                n, m = max(n, m), min(n, m)
                aspects.append(round(n / m, 3))
        return {"parts_per_state": dict(sorted(parts.items())),
                "aspect": quantiles(aspects), "short_side": quantiles(shorts)}
    sel, comp, mix = [], [], []
    for e, rep in hi.classes_of(hi.single, False):
        s, x, cm = children(hi.states[e], rep)
        sel.append(s); mix.append(x); comp.append(cm)
    return {
        "lower_endpoints": prof([semantic_state(s) for s in lo.states.values()]),
        "upper_forced_selected": prof(sel),
        "upper_forced_mixed": prof(mix),
        "upper_forced_complement": prof(comp),
    }


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--k8", type=Path, required=True, help="upper (root_k=8) census log")
    ap.add_argument("--k7", type=Path, required=True, help="lower (root_k=7) census log")
    ap.add_argument("--seed", type=int, default=20260820, help="control-sampling seed")
    ap.add_argument("--json", type=Path, help="write the report as JSON")
    args = ap.parse_args()

    hi = Corpus(args.k8, "k8")
    lo = Corpus(args.k7, "k7")
    rng = random.Random(args.seed)

    report = {"seed": args.seed}
    for c in (hi, lo):
        report[c.label] = {
            "population": population_report(c),
            "cut_archetypes": archetypes(c),
            "child_geometry": geometry(c, rng),
            "scale_profile": scale_profile(c),
            "symmetry_control": symmetry_control(c),
            "shape_concentration": shape_concentration(c),
        }
    report["cross_level"] = cross_level(hi, lo)
    report["lower_level_shape_profiles"] = child_profile(hi, lo)

    text = json.dumps(report, indent=2, default=str)
    print(text)
    if args.json:
        args.json.write_text(text)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
