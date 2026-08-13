#!/usr/bin/env python3
"""Summarize a completed ``pareto_prefix_census`` log.

Only tab-separated ``CENSUS`` records are read; ordinary solver diagnostics and embedded
provenance are ignored.  In addition to raw cut multiplicity, the report quotients full-solution
cuts by the exact automorphisms of a canonical state: global test complementation, permutation of
identical components, and shore exchange inside square components.
"""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import csv
import hashlib
import json
from pathlib import Path
import statistics
import sys
from typing import Iterable


Pair = tuple[int, int]


def fields(line: str) -> tuple[str, dict[str, str]] | None:
    if not line.startswith("CENSUS\t"):
        return None
    words = line.rstrip("\n").split("\t")
    data: dict[str, str] = {}
    for word in words[2:]:
        if "=" in word:
            key, value = word.split("=", 1)
            data[key] = value
    return words[1], data


def pairs(value: str) -> tuple[Pair, ...]:
    if value == "-" or not value:
        return ()
    return tuple(tuple(map(int, word.split(":"))) for word in value.split(","))  # type: ignore[return-value]


def semantic_state(parts: Iterable[Pair]) -> tuple[Pair, ...]:
    """Canonicalize independently of radiobase's internal Sbb numbering."""
    return tuple(sorted((max(n, m), min(n, m)) for n, m in parts if n * m > 1))


def mass(parts: Iterable[Pair]) -> int:
    return sum(n * m for n, m in parts)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def unit_extensions(units: int, masses: tuple[int, int, int], cap: int) -> int:
    """Count labelled top-level cuts of removed ``1:1`` components.

    A unit component has one cut producing outcome 0, two shore choices producing outcome 1,
    and one cut producing outcome 2.  Unit-Group Elimination permits any allocation that fits in
    the three child information slacks.
    """
    total = 0
    slacks = tuple(cap - value for value in masses)
    for outcome0 in range(units + 1):
        for outcome1 in range(units - outcome0 + 1):
            outcome2 = units - outcome0 - outcome1
            counts = (outcome0, outcome1, outcome2)
            if any(count > slack for count, slack in zip(counts, slacks)):
                continue
            # Multinomial choice of labelled units, with two shore orientations in outcome 1.
            ways = 1
            remaining = units
            for count in counts[:2]:
                numerator = 1
                denominator = 1
                for step in range(count):
                    numerator *= remaining - step
                    denominator *= step + 1
                ways *= numerator // denominator
                remaining -= count
            total += ways * (2 ** outcome1)
    return total


def require(condition: bool, path: Path, message: str) -> None:
    if not condition:
        raise ValueError(f"{path}: {message}")


def component_bijections(endpoint: tuple[Pair, ...], seed: tuple[Pair, ...]) -> int:
    """Count component bijections under which ``endpoint`` contains ``seed``.

    Equal canonical components deliberately remain separate positions here.  Once a canonical
    upgrade is transported back to a labelled four-lineage record, those assignments can lead to
    different lifted cuts even though they describe one unlabelled endpoint state.
    """
    if len(endpoint) != len(seed):
        return 0

    def visit(index: int, used: int) -> int:
        if index == len(seed):
            return 1
        sn, sm = seed[index]
        return sum(
            visit(index + 1, used | (1 << j))
            for j, (en, em) in enumerate(endpoint)
            if not (used & (1 << j)) and en >= sn and em >= sm
        )

    return visit(0, 0)


def dominates(endpoint: tuple[Pair, ...], seed: tuple[Pair, ...]) -> bool:
    return component_bijections(endpoint, seed) > 0


def shore_oriented_embeddings(endpoint: tuple[Pair, ...], seed: tuple[Pair, ...]) -> int:
    """Count component bijections and both valid labelled-shore orientations.

    This intentionally does not quotient square-shore or identical-component automorphisms.  It
    measures how many ways a canonical endpoint can be transported back to a labelled lineage;
    the split-class statistics separately measure inequivalent canonical endpoint cuts.
    """
    if len(endpoint) != len(seed):
        return 0

    def visit(index: int, used: int) -> int:
        if index == len(seed):
            return 1
        sn, sm = seed[index]
        total = 0
        for j, (en, em) in enumerate(endpoint):
            if used & (1 << j):
                continue
            orientations = int(en >= sn and em >= sm) + int(em >= sn and en >= sm)
            if orientations:
                total += orientations * visit(index + 1, used | (1 << j))
        return total

    return visit(0, 0)


def nearest(values: Iterable[int], quantile: float) -> int:
    ordered = sorted(values)
    if not ordered:
        return 0
    index = max(0, min(len(ordered) - 1, int(quantile * len(ordered) + 0.999999) - 1))
    return ordered[index]


def distribution(values: Iterable[int]) -> dict[str, int | float]:
    vals = list(values)
    if not vals:
        return {"min": 0, "median": 0, "p90": 0, "max": 0}
    return {
        "min": min(vals),
        "median": statistics.median(vals),
        "p90": nearest(vals, 0.9),
        "max": max(vals),
    }


def histogram(values: Iterable[int]) -> dict[int, int]:
    return dict(sorted(Counter(values).items()))


def normalize_identical(parts: tuple[Pair, ...], cuts: tuple[Pair, ...]) -> tuple[Pair, ...]:
    out: list[Pair] = []
    i = 0
    while i < len(parts):
        j = i + 1
        while j < len(parts) and parts[j] == parts[i]:
            j += 1
        group: list[Pair] = []
        n, m = parts[i]
        for a, b in cuts[i:j]:
            cut = (a, b)
            if n == m:
                cut = min(cut, (b, a))
            group.append(cut)
        out.extend(sorted(group))
        i = j
    return tuple(out)


def cut_class(parts: tuple[Pair, ...], cuts: tuple[Pair, ...]) -> tuple[Pair, ...]:
    direct = normalize_identical(parts, cuts)
    complement = tuple((n - a, m - b) for (n, m), (a, b) in zip(parts, cuts))
    complement = normalize_identical(parts, complement)
    return min(direct, complement)


def analyze(path: Path, pareto_path: Path | None = None) -> dict[str, object]:
    records: defaultdict[str, list[dict[str, str]]] = defaultdict(list)
    with path.open(errors="replace") as handle:
        for line in handle:
            record = fields(line)
            if record:
                records[record[0]].append(record[1])

    if len(records["END"]) != 1:
        raise ValueError(f"{path}: expected one completed CENSUS END record")

    require(len(records["BEGIN"]) == 1, path, "expected one CENSUS BEGIN record")
    require(len(records["PREFIX_SUMMARY"]) == 1, path, "missing PREFIX_SUMMARY")
    require(len(records["TARGET_SUMMARY"]) == 1, path, "missing TARGET_SUMMARY")
    require(len(records["UPGRADE_SUMMARY"]) == 1, path, "missing UPGRADE_SUMMARY")

    root_summaries = records["ROOT_SUMMARY"]
    prefix_summary = records["PREFIX_SUMMARY"][0]
    lineages = records["LINEAGE"]
    root_k = int(records["END"][0]["root_k"])
    residual_k = int(records["END"][0]["residual_k"])
    all_roots = {record["root"] for record in root_summaries}
    require(len(root_summaries) == len(all_roots), path, "duplicate root summaries")
    require(int(records["BEGIN"][0]["roots"]) == len(all_roots), path,
            "BEGIN root total mismatch")

    frontier: dict[tuple[int, int], int] = {}
    if pareto_path is not None:
        with pareto_path.open(newline="") as handle:
            for row in csv.DictReader(handle):
                if row["bound"] == "max" and row["status"].startswith("proven"):
                    frontier[int(row["k"]), int(row["m"])] = int(row["n1"])
        expected_roots = {
            f"{n}:{m}" for (k, m), n in frontier.items() if k == root_k
        }
        require(all_roots == expected_roots, path,
                "census roots differ from the proven Pareto table")

    def on_front(level: int, u: int, v: int) -> bool:
        u, v = max(u, v), min(u, v)
        return v > 0 and frontier.get((level, v)) == u

    def frontier_deficit(level: int, u: int, v: int) -> int | None:
        u, v = max(u, v), min(u, v)
        if v == 0:
            return None
        if not frontier:
            return 0
        require((level, v) in frontier, path,
                f"missing proven k={level}, m={v} frontier point")
        deficit = frontier[level, v] - u
        require(deficit >= 0, path,
                f"solvable component {u}:{v}@{level} crosses its proven frontier")
        return deficit

    targets = {record["id"]: record for record in records["TARGET"]}
    endpoints = {record["id"]: record for record in records["ENDPOINT"]}
    require(len(targets) == len(records["TARGET"]), path, "duplicate target IDs")
    require(len(endpoints) == len(records["ENDPOINT"]), path, "duplicate endpoint IDs")
    target_states = {key: semantic_state(pairs(record["state"]))
                     for key, record in targets.items()}
    endpoint_states = {key: semantic_state(pairs(record["state"]))
                       for key, record in endpoints.items()}
    target_units = {key: int(record.get("units", "0")) for key, record in targets.items()}
    endpoint_units = {key: int(record.get("units", "0")) for key, record in endpoints.items()}
    require(all(value >= 0 for value in target_units.values()), path,
            "negative target unit reserve")
    require(all(value >= 0 for value in endpoint_units.values()), path,
            "negative endpoint unit reserve")
    if frontier:
        for kind, states in (("target", target_states), ("endpoint", endpoint_states)):
            for key, state in states.items():
                for n, m in state:
                    require(frontier.get((residual_k, m), -1) >= n, path,
                            f"{kind} {key}: component {n}:{m} exceeds the proven "
                            f"one-part k={residual_k} frontier")
    target_signatures = {
        key: (state, target_units[key]) for key, state in target_states.items()
    }
    endpoint_signatures = {
        key: (state, endpoint_units[key]) for key, state in endpoint_states.items()
    }
    require(len(set(target_signatures.values())) == len(target_signatures), path,
            "duplicate canonical target/unit states")
    require(len(set(endpoint_signatures.values())) == len(endpoint_signatures), path,
            "duplicate canonical endpoint/unit states")
    ordered_endpoints = sorted(
        endpoint_states.items(),
        key=lambda item: (endpoint_units[item[0]], mass(item[1])),
    )
    for i, (smaller_id, smaller) in enumerate(ordered_endpoints):
        for larger_id, larger in ordered_endpoints[i + 1:]:
            if endpoint_units[larger_id] == endpoint_units[smaller_id] \
                    and dominates(larger, smaller):
                raise ValueError(
                    f"{path}: endpoint antichain violation: {larger_id} dominates {smaller_id}"
                )

    first_keys: set[tuple[str, Pair]] = set()
    nondegenerate_first_keys: set[tuple[str, Pair]] = set()
    first_frontier_deficits: list[int] = []
    first_frontier_deficits_by_root: defaultdict[str, list[int]] = defaultdict(list)
    for index, record in enumerate(records["FIRST"], 1):
        root = pairs(record["root"])
        take = pairs(record["take"])
        mixed = pairs(record["mixed"])
        require(len(root) == len(take) == 1, path, f"first {index}: malformed cut")
        require(record["root"] in all_roots, path, f"first {index}: unknown root")
        require(int(record["k"]) == root_k, path, f"first {index}: wrong level")
        n, m = root[0]
        a, b = take[0]
        require(0 <= a <= n and 0 <= b <= m, path, f"first {index}: cut out of range")
        require(mixed == ((a, m - b), (n - a, b)), path,
                f"first {index}: wrong labelled mixed child")
        require(semantic_state(mixed) == semantic_state(pairs(record["canonical"])), path,
                f"first {index}: wrong canonical mixed child")
        require(int(record["strict"]) in (0, 1), path, f"first {index}: invalid strict flag")
        key = (record["root"], take[0])
        require(key not in first_keys, path, f"first {index}: duplicate labelled cut")
        first_keys.add(key)
        if all(u * v for u, v in mixed):
            nondegenerate_first_keys.add(key)
        if frontier:
            expected_strict = on_front(root_k - 1, a, b) or on_front(
                root_k - 1, n - a, m - b
            )
            require(int(record["strict"]) == expected_strict, path,
                    f"first {index}: incorrect strict flag")
            deficits = [
                value for value in (
                    frontier_deficit(root_k - 1, a, b),
                    frontier_deficit(root_k - 1, n - a, m - b),
                ) if value is not None
            ]
            require(bool(deficits), path, f"first {index}: no nonempty pure child")
            best_deficit = min(deficits)
            first_frontier_deficits.append(best_deficit)
            first_frontier_deficits_by_root[record["root"]].append(best_deficit)

    lineage_counts: Counter[str] = Counter()
    lineage_opposed: Counter[str] = Counter()
    lineage_strict: Counter[str] = Counter()
    lineage_keys: set[tuple[str, Pair, tuple[Pair, ...]]] = set()
    lineage_by_first: Counter[tuple[str, Pair]] = Counter()
    second_frontier_deficits: list[int] = []
    second_frontier_deficits_by_root: defaultdict[str, list[int]] = defaultdict(list)
    for index, record in enumerate(lineages, 1):
        root = pairs(record["root"])
        first = pairs(record["first"])
        two = pairs(record["two"])
        second = pairs(record["second"])
        four = pairs(record["four"])
        require(len(root) == len(first) == 1, path, f"lineage {index}: malformed first cut")
        require(len(two) == len(second) == 2, path, f"lineage {index}: malformed second cut")
        require(len(four) == 4, path, f"lineage {index}: malformed four-lineage child")
        require(record["root"] in all_roots, path, f"lineage {index}: unknown root")
        require(int(record["k"]) == residual_k, path, f"lineage {index}: wrong level")
        require(int(record["opposed"]) in (0, 1), path,
                f"lineage {index}: invalid opposed flag")
        require(int(record["strict_first"]) in (0, 1), path,
                f"lineage {index}: invalid strict-first flag")
        n, m = root[0]
        a, b = first[0]
        require(0 <= a <= n and 0 <= b <= m, path, f"lineage {index}: first cut out of range")
        require(two == ((a, m - b), (n - a, b)), path, f"lineage {index}: wrong two child")
        expected_four: list[Pair] = []
        for (u, v), (x, y) in zip(two, second):
            require(0 <= x <= u and 0 <= y <= v, path, f"lineage {index}: second cut out of range")
            expected_four.extend(((x, v - y), (u - x, y)))
        require(tuple(expected_four) == four, path, f"lineage {index}: wrong four child")
        target = record["target"]
        require(target in targets, path, f"lineage {index}: unknown target {target}")
        displayed = semantic_state(pairs(record["canonical"]))
        require(displayed == semantic_state(four), path, f"lineage {index}: wrong canonical child")
        require(displayed == semantic_state(pairs(targets[target]["state"])), path,
                f"lineage {index}: target state mismatch")
        raw_units = sum(u * v == 1 for u, v in four)
        raw_empty = sum(u * v == 0 for u, v in four)
        tracks_units = "units" in targets[target]
        if tracks_units:
            require(target_units[target] == raw_units, path,
                    f"lineage {index}: target unit reserve mismatch")
        if "empty_lineages" in targets[target]:
            require(int(targets[target]["empty_lineages"]) == raw_empty, path,
                    f"lineage {index}: target empty-lineage count mismatch")
        target_core_mass = mass(displayed)
        if "core_mass" in targets[target]:
            require(int(targets[target]["core_mass"]) == target_core_mass, path,
                    f"lineage {index}: target core mass mismatch")
        recorded_mass = target_core_mass + (raw_units if tracks_units else 0)
        require(int(targets[target]["mass"]) == recorded_mass, path,
                f"lineage {index}: target total mass mismatch")
        key = (record["root"], first[0], second)
        require(key not in lineage_keys, path, f"lineage {index}: duplicate labelled cuts")
        lineage_keys.add(key)
        lineage_by_first[record["root"], first[0]] += 1
        if frontier:
            (n0, m0), (n1, m1) = two
            (a0, b0), (a1, b1) = second
            expected_opposed = (
                on_front(root_k - 2, a0, b0)
                and on_front(root_k - 2, n1 - a1, m1 - b1)
            ) or (
                on_front(root_k - 2, n0 - a0, m0 - b0)
                and on_front(root_k - 2, a1, b1)
            )
            require(int(record["opposed"]) == expected_opposed, path,
                    f"lineage {index}: incorrect opposed flag")
            opposed_deficits: list[int] = []
            for left, right in (
                ((a0, b0), (n1 - a1, m1 - b1)),
                ((n0 - a0, m0 - b0), (a1, b1)),
            ):
                values = [
                    frontier_deficit(root_k - 2, *left),
                    frontier_deficit(root_k - 2, *right),
                ]
                if all(value is not None for value in values):
                    opposed_deficits.append(sum(value for value in values if value is not None))
            if opposed_deficits:
                best_deficit = min(opposed_deficits)
                second_frontier_deficits.append(best_deficit)
                second_frontier_deficits_by_root[record["root"]].append(best_deficit)
        lineage_counts[target] += 1
        lineage_opposed[target] += int(record["opposed"])
        lineage_strict[target] += int(record["strict_first"])

    for target, record in targets.items():
        require(int(record["occurrences"]) == lineage_counts[target], path,
                f"{target}: occurrence count mismatch")
        require(int(record["opposed_occurrences"]) == lineage_opposed[target], path,
                f"{target}: opposed count mismatch")
        require(int(record["strict_first_occurrences"]) == lineage_strict[target], path,
                f"{target}: strict count mismatch")

    second_summaries: dict[tuple[str, Pair], dict[str, str]] = {}
    for record in records["SECOND_SUMMARY"]:
        key = (record["root"], pairs(record["first"])[0])
        require(key in first_keys, path, f"second summary for unknown first cut {key}")
        require(key not in second_summaries, path, f"duplicate second summary {key}")
        second_summaries[key] = record
        require(int(record["winners"]) == lineage_by_first[key], path,
                f"{key}: second-winner summary mismatch")
    require(set(second_summaries) == nondegenerate_first_keys, path,
            "second-summary set differs from nondegenerate first cuts")

    upgrades: defaultdict[str, list[tuple[str, int, int, int]]] = defaultdict(list)
    upgrade_pairs: set[tuple[str, str]] = set()
    for record in records["UPGRADE"]:
        seed = record["seed"]
        endpoint = record["endpoint"]
        require(seed in targets, path, f"upgrade names unknown seed {seed}")
        require(endpoint in endpoints, path, f"upgrade names unknown endpoint {endpoint}")
        seed_state = target_states[seed]
        endpoint_state = endpoint_states[endpoint]
        require(target_units[seed] == endpoint_units[endpoint], path,
                f"upgrade {seed}->{endpoint} changes the unit reserve")
        require(dominates(endpoint_state, seed_state), path,
                f"upgrade {seed}->{endpoint} does not dominate")
        delta = int(record["delta_mass"])
        require(delta == mass(endpoint_state) - mass(seed_state), path,
                f"upgrade {seed}->{endpoint} has wrong mass delta")
        require((seed, endpoint) not in upgrade_pairs, path,
                f"duplicate upgrade edge {seed}->{endpoint}")
        bijections = component_bijections(endpoint_state, seed_state)
        embeddings = shore_oriented_embeddings(endpoint_state, seed_state)
        require(bijections > 0, path, f"upgrade {seed}->{endpoint} has no alignment")
        recorded_bijections = record.get("component_bijections", record.get("alignments"))
        if recorded_bijections is not None:
            require(int(recorded_bijections) == bijections, path,
                    f"upgrade {seed}->{endpoint} has wrong component-bijection count")
        if "shore_embeddings" in record:
            require(int(record["shore_embeddings"]) == embeddings, path,
                    f"upgrade {seed}->{endpoint} has wrong shore-embedding count")
        upgrade_pairs.add((seed, endpoint))
        upgrades[seed].append((endpoint, delta, bijections, embeddings))

    structured_targets = {
        record["target"] for record in lineages
        if int(record["strict_first"]) and int(record["opposed"])
    }
    structured_roots = {
        record["root"] for record in lineages
        if int(record["strict_first"]) and int(record["opposed"])
    }
    nondegenerate_roots = {root for root, _ in nondegenerate_first_keys}
    exact_opposed_roots = {
        root for root, values in second_frontier_deficits_by_root.items()
        if min(values) == 0
    }
    strict_roots = {
        record["root"] for record in records["FIRST"] if int(record["strict"])
    }
    first_by_root = Counter(record["root"] for record in records["FIRST"])
    roots_by_target: defaultdict[str, set[str]] = defaultdict(set)
    structured_roots_by_target: defaultdict[str, set[str]] = defaultdict(set)
    for record in lineages:
        roots_by_target[record["target"]].add(record["root"])
        if int(record["strict_first"]) and int(record["opposed"]):
            structured_roots_by_target[record["target"]].add(record["root"])
    strict_by_root = Counter(
        record["root"] for record in records["FIRST"] if int(record["strict"])
    )
    lineages_by_root = Counter(record["root"] for record in lineages)
    opposed_by_root = Counter(record["root"] for record in lineages if int(record["opposed"]))
    strict_opposed_by_root = Counter(
        record["root"] for record in lineages
        if int(record["strict_first"]) and int(record["opposed"])
    )
    for record in root_summaries:
        root = record["root"]
        require(int(record["winners"]) == first_by_root[root], path,
                f"{root}: first-winner summary mismatch")
        require(int(record["strict_winners"]) == strict_by_root[root], path,
                f"{root}: strict-winner summary mismatch")
    require(int(prefix_summary["first_winners"]) == len(records["FIRST"]), path,
            "prefix first-winner total mismatch")
    require(int(prefix_summary["second_winners"]) == len(lineages), path,
            "prefix second-winner total mismatch")

    full_states = {record["id"]: pairs(record["state"]) for record in records["FULL_STATE"]}
    full_units = {
        record["id"]: int(record.get("units", "0")) for record in records["FULL_STATE"]
    }
    require(len(full_states) == len(records["FULL_STATE"]), path, "duplicate full-state IDs")
    require(set(full_states) == set(endpoints), path, "full-state/endpoint ID sets differ")
    for endpoint, state in full_states.items():
        require(semantic_state(state) == semantic_state(pairs(endpoints[endpoint]["state"])),
                path, f"{endpoint}: full state differs from endpoint")
        require(full_units[endpoint] == endpoint_units[endpoint], path,
                f"{endpoint}: full state changes endpoint unit reserve")

    raw_winners = Counter(record["id"] for record in records["FULL_WIN"])
    split_classes: defaultdict[str, set[tuple[Pair, ...]]] = defaultdict(set)
    class_multiplicity: defaultdict[str, Counter[tuple[Pair, ...]]] = defaultdict(Counter)
    raw_splits: defaultdict[str, set[tuple[Pair, ...]]] = defaultdict(set)
    tight_winners = 0
    tight_by_endpoint: Counter[str] = Counter()
    unit_extended_winners: Counter[str] = Counter()
    child_slacks: list[int] = []
    child_spreads: list[int] = []
    child_cap = 3 ** (residual_k - 1)
    for index, record in enumerate(records["FULL_WIN"], 1):
        endpoint = record["id"]
        require(endpoint in full_states, path, f"full winner {index}: unknown endpoint {endpoint}")
        take = pairs(record["take"])
        state = full_states[endpoint]
        require(len(take) == len(state), path, f"full winner {index}: wrong cut arity")
        selected: list[Pair] = []
        mixed: list[Pair] = []
        complement: list[Pair] = []
        for (n, m), (a, b) in zip(state, take):
            require(0 <= a <= n and 0 <= b <= m, path, f"full winner {index}: cut out of range")
            selected.append((a, b))
            mixed.extend(((a, m - b), (n - a, b)))
            complement.append((n - a, m - b))
        raw_children = (selected, mixed, complement)
        expected_children = tuple(semantic_state(child) for child in raw_children)
        actual_children = tuple(
            semantic_state(pairs(record[key])) for key in ("selected", "mixed", "complement")
        )
        require(expected_children == actual_children, path,
                f"full winner {index}: child state mismatch")
        masses = tuple(map(int, record["masses"].split("/")))
        # The enumerator's information mass includes singleton rectangles (one possible pair),
        # although canSolveB then erases them by the Unit Group Triviality Lemma.
        require(masses == tuple(mass(child) for child in raw_children), path,
                f"full winner {index}: child mass mismatch")
        require(max(masses) <= child_cap, path, f"full winner {index}: information cap exceeded")
        units = full_units[endpoint]
        expected_extensions = unit_extensions(units, masses, child_cap)
        require(expected_extensions > 0, path,
                f"full winner {index}: reserved units cannot fit in its children")
        recorded_extensions = int(record.get("unit_extensions", "1"))
        require(recorded_extensions == expected_extensions, path,
                f"full winner {index}: wrong unit-extension count")
        unit_extended_winners[endpoint] += recorded_extensions
        require(take not in raw_splits[endpoint], path,
                f"full winner {index}: duplicate labelled cut")
        raw_splits[endpoint].add(take)
        cut_orbit = cut_class(full_states[endpoint], take)
        split_classes[endpoint].add(cut_orbit)
        class_multiplicity[endpoint][cut_orbit] += 1
        slack = child_cap - max(masses)
        child_slacks.append(slack)
        child_spreads.append(max(masses) - min(masses))
        if slack == 0:
            tight_winners += 1
            tight_by_endpoint[endpoint] += 1

    full_summaries = {record["id"]: record for record in records["FULL_SUMMARY"]}
    require(len(full_summaries) == len(records["FULL_SUMMARY"]), path,
            "duplicate full summaries")
    require(not (set(full_summaries) - set(endpoints)), path, "full summary for unknown endpoint")
    missing_maps = sorted(set(endpoints) - set(full_summaries))
    zero_winner = sorted(endpoint for endpoint in endpoints if raw_winners[endpoint] == 0)
    if missing_maps or zero_winner:
        raise ValueError(
            f"{path}: incomplete full maps: missing={missing_maps[:3]} zero={zero_winner[:3]}"
        )
    for endpoint, record in full_summaries.items():
        require(int(record["winners"]) == raw_winners[endpoint], path,
                f"{endpoint}: full winner summary mismatch")
        if "unit_extended_winners" in record:
            require(int(record["unit_extended_winners"]) == unit_extended_winners[endpoint], path,
                    f"{endpoint}: unit-extended winner summary mismatch")

    structured_endpoints = {
        endpoint
        for seed in structured_targets
        for endpoint, _, _, _ in upgrades.get(seed, ())
    }
    eligible_targets = {key for key, value in targets.items() if int(value["eligible"])}
    four_targets = {
        key for key, state in target_states.items() if len(state) == 4
    }
    structured_eligible_targets = structured_targets & eligible_targets
    missing_upgrades = sorted(eligible_targets - set(upgrades))
    require(not missing_upgrades, path, f"eligible seeds without endpoints: {missing_upgrades[:3]}")
    linked_endpoints = {
        endpoint for choices in upgrades.values() for endpoint, _, _, _ in choices
    }
    require(linked_endpoints == set(endpoints), path,
            "endpoint set differs from upgrade-edge destinations")
    seed_summaries = {record["seed"]: record for record in records["UPGRADE_SEED_SUMMARY"]}
    require(len(seed_summaries) == len(records["UPGRADE_SEED_SUMMARY"]), path,
            "duplicate upgrade-seed summaries")
    require(set(seed_summaries) == eligible_targets, path,
            "upgrade-seed summary set differs from eligible targets")
    for seed, record in seed_summaries.items():
        require(int(record["endpoints"]) == len(upgrades[seed]), path,
                f"{seed}: upgrade endpoint count mismatch")
    already_maximal = {
        seed
        for seed, choices in upgrades.items()
        if any(delta == 0 for _, delta, _, _ in choices)
    }
    endpoint_roots: defaultdict[str, set[str]] = defaultdict(set)
    structured_endpoint_roots: defaultdict[str, set[str]] = defaultdict(set)
    seeds_by_endpoint: Counter[str] = Counter()
    for seed, choices in upgrades.items():
        for endpoint, _, _, _ in choices:
            seeds_by_endpoint[endpoint] += 1
            endpoint_roots[endpoint].update(roots_by_target[seed])
            structured_endpoint_roots[endpoint].update(structured_roots_by_target[seed])
    structured_endpoints_by_root: defaultdict[str, set[str]] = defaultdict(set)
    for endpoint, roots in structured_endpoint_roots.items():
        for root in roots:
            structured_endpoints_by_root[root].add(endpoint)
    target_summary = records["TARGET_SUMMARY"][0]
    upgrade_summary = records["UPGRADE_SUMMARY"][0]
    end_summary = records["END"][0]
    require(int(target_summary["targets"]) == len(targets), path, "target total mismatch")
    # The original four-part-only prototype called its eligible set ``eligible_four``.  Final
    # corpora also upgrade canonical two- and three-part degeneracies, and name the count
    # ``upgrade_seeds``.  Accepting the old spelling keeps the analyzer useful on tuning logs.
    summary_eligible = target_summary.get(
        "upgrade_seeds", target_summary.get("eligible_four", "-1")
    )
    require(int(summary_eligible) == len(eligible_targets), path,
            "eligible-target total mismatch")
    if "canonical_four" in target_summary:
        require(int(target_summary["canonical_four"]) == len(four_targets), path,
                "canonical-four target total mismatch")
    if "empty_core" in target_summary:
        require(int(target_summary["empty_core"]) == sum(not state for state in target_states.values()),
                path, "empty-core target total mismatch")
    require(int(target_summary["degenerate"]) == len(targets) - len(four_targets), path,
            "degenerate-target total mismatch")
    require(int(upgrade_summary["endpoints"]) == len(endpoints), path,
            "endpoint total mismatch")
    require(int(upgrade_summary["eligible_seeds"]) == len(eligible_targets), path,
            "upgrade eligible-seed total mismatch")
    require(int(upgrade_summary["visited"]) == int(end_summary["upgrade_nodes"]), path,
            "upgrade visited-node total mismatch")
    require(int(end_summary["targets"]) == len(targets), path, "END target total mismatch")
    require(int(end_summary["endpoints"]) == len(endpoints), path, "END endpoint total mismatch")

    exact_summary = records.get("EXACT_ORACLE_SUMMARY", [])
    exact = exact_summary[0] if exact_summary else {"facts": "0", "hits": "0", "misses": "0"}
    endpoint_core_masses = {key: mass(value) for key, value in endpoint_states.items()}
    endpoint_masses = {
        key: endpoint_core_masses[key] + endpoint_units[key] for key in endpoints
    }
    for key, record in endpoints.items():
        require(int(record["mass"]) == endpoint_masses[key], path,
                f"{key}: endpoint mass mismatch")
        if "core_mass" in record:
            require(int(record["core_mass"]) == endpoint_core_masses[key], path,
                    f"{key}: endpoint core mass mismatch")
    endpoint_cap = 3 ** residual_k
    target_parts = {key: len(value) for key, value in target_states.items()}
    endpoint_parts = {key: len(value) for key, value in endpoint_states.items()}
    per_dimension: dict[int, dict[str, object]] = {}
    for part_count in sorted(set(target_parts.values()) | set(endpoint_parts.values())):
        dimension_targets = {
            key for key in eligible_targets if target_parts[key] == part_count
        }
        dimension_endpoints = {
            key for key in endpoints if endpoint_parts[key] == part_count
        }
        dimension_edges = [
            (seed, endpoint, delta, bijections, embeddings)
            for seed in dimension_targets
            for endpoint, delta, bijections, embeddings in upgrades[seed]
        ]
        per_dimension[part_count] = {
            "targets": len(dimension_targets),
            "target_unit_reserve_histogram": histogram(
                target_units[key] for key in dimension_targets
            ),
            "already_maximal_seeds": len(dimension_targets & already_maximal),
            "upgrade_edges": len(dimension_edges),
            "upgrade_delta_mass": distribution(
                delta for _, _, delta, _, _ in dimension_edges
            ),
            "component_bijections": sum(
                bijections for _, _, _, bijections, _ in dimension_edges
            ),
            "shore_oriented_embeddings": sum(
                embeddings for _, _, _, _, embeddings in dimension_edges
            ),
            "endpoints": len(dimension_endpoints),
            "endpoint_unit_reserve_histogram": histogram(
                endpoint_units[key] for key in dimension_endpoints
            ),
            "raw_full_winners": sum(raw_winners[key] for key in dimension_endpoints),
            "unit_extended_full_winners": sum(
                unit_extended_winners[key] for key in dimension_endpoints
            ),
            "automorphism_quotient_classes": sum(
                len(split_classes[key]) for key in dimension_endpoints
            ),
            "tight_full_winners": sum(tight_by_endpoint[key] for key in dimension_endpoints),
        }
    richest_endpoints = [
        {
            "id": key,
            "state": endpoints[key]["state"],
            "units": endpoint_units[key],
            "core_mass": endpoint_core_masses[key],
            "mass": endpoint_masses[key],
            "information_slack": endpoint_cap - endpoint_masses[key],
            "raw_winners": raw_winners[key],
            "unit_extended_winners": unit_extended_winners[key],
            "classes": len(split_classes[key]),
            "tight_winners": tight_by_endpoint[key],
        }
        for key in sorted(
            endpoints,
            key=lambda item: (
                -raw_winners[item], -len(split_classes[item]), endpoints[item]["state"]
            ),
        )[:10]
    ]
    raw_solutions_per_seed = {
        seed: sum(raw_winners[endpoint] for endpoint, _, _, _ in upgrades[seed])
        for seed in eligible_targets
    }
    unit_extended_solutions_per_seed = {
        seed: sum(unit_extended_winners[endpoint] for endpoint, _, _, _ in upgrades[seed])
        for seed in eligible_targets
    }
    endpoint_embedding_cut_pairs_per_seed = {
        seed: sum(
            unit_extended_winners[endpoint] * embeddings
            for endpoint, _, _, embeddings in upgrades[seed]
        )
        for seed in eligible_targets
    }
    solution_classes_per_seed = {
        seed: sum(len(split_classes[endpoint]) for endpoint, _, _, _ in upgrades[seed])
        for seed in eligible_targets
    }
    single_endpoint_single_class = sum(
        len(upgrades[seed]) == 1
        and len(split_classes[upgrades[seed][0][0]]) == 1
        for seed in eligible_targets
    )

    result: dict[str, object] = {
        "path": str(path),
        "input_sha256": sha256(path),
        "pareto_csv_sha256": sha256(pareto_path) if pareto_path is not None else None,
        "analyzer_sha256": sha256(Path(__file__).resolve()),
        "python_version": sys.version.split()[0],
        "root_k": root_k,
        "residual_k": residual_k,
        "roots": len(all_roots),
        "first_winners": len(records["FIRST"]),
        "strict_first_winners": sum(int(record["strict"]) for record in records["FIRST"]),
        "first_winners_per_root": distribution(first_by_root[root] for root in all_roots),
        "strict_first_winners_per_root": distribution(strict_by_root[root]
                                                       for root in all_roots),
        "roots_with_strict_first": len(strict_roots),
        "roots_without_strict_first": sorted(all_roots - strict_roots),
        "first_frontier_deficit": distribution(first_frontier_deficits),
        "first_frontier_deficit_histogram": histogram(first_frontier_deficits),
        "minimum_first_frontier_deficit_by_root": {
            root: min(values)
            for root, values in sorted(first_frontier_deficits_by_root.items())
        },
        "roots_without_exact_frontier_first": sorted(
            root for root, values in first_frontier_deficits_by_root.items()
            if min(values) > 0
        ),
        "second_winners": len(lineages),
        "second_unique_states": int(prefix_summary.get("second_unique_states", "0")),
        "second_memo_hits": int(prefix_summary.get("second_memo_hits", "0")),
        "opposed_second_winners": sum(int(record["opposed"]) for record in lineages),
        "strict_opposed_lineages": sum(
            int(record["strict_first"]) and int(record["opposed"]) for record in lineages
        ),
        "lineages_per_root": distribution(lineages_by_root[root] for root in all_roots),
        "opposed_lineages_per_root": distribution(opposed_by_root[root] for root in all_roots),
        "strict_opposed_lineages_per_root": distribution(strict_opposed_by_root[root]
                                                           for root in all_roots),
        "roots_with_strict_opposed_lineage": len(structured_roots),
        "roots_without_strict_opposed_lineage": sorted(all_roots - structured_roots),
        "second_opposed_frontier_deficit": distribution(second_frontier_deficits),
        "second_opposed_frontier_deficit_histogram": histogram(second_frontier_deficits),
        "minimum_second_opposed_frontier_deficit_by_root": {
            root: min(values)
            for root, values in sorted(second_frontier_deficits_by_root.items())
        },
        "nondegenerate_roots_without_exact_opposed_second": sorted(
            nondegenerate_roots - exact_opposed_roots
        ),
        "targets": len(targets),
        "upgrade_eligible_targets": len(eligible_targets),
        "canonical_four_targets": len(four_targets),
        "degenerate_targets": len(targets) - len(four_targets),
        "target_parts_histogram": histogram(target_parts.values()),
        "target_unit_reserve_histogram": histogram(target_units.values()),
        "strict_opposed_upgrade_targets": len(structured_targets & eligible_targets),
        "strict_opposed_four_targets": len(structured_targets & four_targets),
        "roots_per_target": distribution(bin(int(record["root_mask"], 16)).count("1")
                                         for record in targets.values()),
        "multi_root_targets": sum(bin(int(record["root_mask"], 16)).count("1") > 1
                                  for record in targets.values()),
        "lineages_per_target": distribution(lineage_counts[target] for target in targets),
        "upgrade_nodes": int(records["END"][0]["upgrade_nodes"]),
        "endpoints": len(endpoints),
        "endpoint_parts_histogram": histogram(endpoint_parts.values()),
        "endpoint_unit_reserve_histogram": histogram(endpoint_units.values()),
        "per_dimension": per_dimension,
        "upgrade_edges": sum(map(len, upgrades.values())),
        "already_maximal_seeds": len(already_maximal),
        "upgraded_seeds": len(eligible_targets) - len(already_maximal),
        "endpoints_per_seed": distribution(len(upgrades[seed]) for seed in eligible_targets),
        "multi_endpoint_seeds": sum(len(upgrades[seed]) > 1 for seed in eligible_targets),
        "seeds_per_endpoint": distribution(seeds_by_endpoint[key] for key in endpoints),
        "multi_seed_endpoints": sum(seeds_by_endpoint[key] > 1 for key in endpoints),
        "upgrade_delta_mass": distribution(
            delta for choices in upgrades.values() for _, delta, _, _ in choices
        ),
        "upgrade_delta_mass_histogram": histogram(
            delta for choices in upgrades.values() for _, delta, _, _ in choices
        ),
        "endpoints_per_seed_histogram": histogram(
            len(upgrades[seed]) for seed in eligible_targets
        ),
        "upgrade_component_bijections": sum(
            bijections
            for choices in upgrades.values()
            for _, _, bijections, _ in choices
        ),
        "component_bijections_per_edge": distribution(
            bijections
            for choices in upgrades.values()
            for _, _, bijections, _ in choices
        ),
        "upgrade_shore_oriented_embeddings": sum(
            embeddings
            for choices in upgrades.values()
            for _, _, _, embeddings in choices
        ),
        "shore_oriented_embeddings_per_edge": distribution(
            embeddings
            for choices in upgrades.values()
            for _, _, _, embeddings in choices
        ),
        "raw_endpoint_solutions_per_seed": distribution(raw_solutions_per_seed.values()),
        "unit_extended_endpoint_solutions_per_seed": distribution(
            unit_extended_solutions_per_seed.values()
        ),
        "endpoint_embedding_cut_pairs_per_seed": distribution(
            endpoint_embedding_cut_pairs_per_seed.values()
        ),
        "solution_classes_per_seed": distribution(solution_classes_per_seed.values()),
        "single_endpoint_single_class_seeds": single_endpoint_single_class,
        "structured_endpoints": len(structured_endpoints),
        "structured_eligible_targets": len(structured_eligible_targets),
        "structured_already_maximal_seeds": len(structured_eligible_targets & already_maximal),
        "structured_upgraded_seeds": len(structured_eligible_targets - already_maximal),
        "structured_upgrade_delta_mass": distribution(
            delta
            for seed in structured_eligible_targets
            for _, delta, _, _ in upgrades[seed]
        ),
        "roots_per_endpoint": distribution(len(endpoint_roots[key]) for key in endpoints),
        "structured_roots_per_endpoint": distribution(
            len(structured_endpoint_roots[key]) for key in structured_endpoints
        ),
        "structured_endpoints_per_root": distribution(
            len(structured_endpoints_by_root.get(root, ())) for root in all_roots
        ),
        "roots_without_structured_endpoint": sorted(
            all_roots - set(structured_endpoints_by_root)
        ),
        "endpoint_mass": distribution(endpoint_masses.values()),
        "endpoint_information_slack": distribution(
            endpoint_cap - value for value in endpoint_masses.values()
        ),
        "endpoint_information_slack_histogram": histogram(
            endpoint_cap - value for value in endpoint_masses.values()
        ),
        "raw_full_winners": sum(raw_winners.values()),
        "unit_extended_full_winners": sum(unit_extended_winners.values()),
        "raw_winners_per_endpoint": distribution(raw_winners.values()),
        "raw_winners_per_endpoint_histogram": histogram(raw_winners.values()),
        "automorphism_quotient_classes": sum(len(value) for value in split_classes.values()),
        "classes_per_endpoint": distribution(len(split_classes[key]) for key in endpoints),
        "classes_per_endpoint_histogram": histogram(
            len(split_classes[key]) for key in endpoints
        ),
        "solution_orbit_size_histogram": histogram(
            count for classes in class_multiplicity.values() for count in classes.values()
        ),
        "single_class_endpoints": sum(len(split_classes[key]) == 1 for key in endpoints),
        "tight_full_winners": tight_winners,
        "tight_winners_per_endpoint": distribution(
            tight_by_endpoint[key] for key in endpoints
        ),
        "full_winner_child_slack": distribution(child_slacks),
        "full_winner_child_slack_histogram": histogram(child_slacks),
        "full_winner_child_spread": distribution(child_spreads),
        "richest_endpoints": richest_endpoints,
        "full_complete_candidates": sum(int(record["complete"]) for record in full_summaries.values()),
        "complete_candidates_per_endpoint": distribution(
            int(record["complete"]) for record in full_summaries.values()
        ),
        "full_prefixes": sum(int(record["prefixes"]) for record in full_summaries.values()),
        "upgrade_successor_queries": int(upgrade_summary.get("successor_queries", "0")),
        "upgrade_information_rejected": int(upgrade_summary.get(
            "information_rejected", "0"
        )),
        "upgrade_component_frontier_rejected": int(upgrade_summary.get(
            "component_frontier_rejected", "0"
        )),
        "representation_blocked": int(records["UPGRADE_SUMMARY"][0].get(
            "representation_blocked", "0"
        )),
        "exact_oracle_facts": int(exact["facts"]),
        "exact_oracle_hits": int(exact["hits"]),
        "exact_oracle_misses": int(exact["misses"]),
    }
    return result


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("log", type=Path)
    parser.add_argument("--pareto-csv", type=Path)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    try:
        result = analyze(args.log, args.pareto_csv)
    except (OSError, ValueError) as exc:
        parser.error(str(exc))
    if args.json:
        print(json.dumps(result, indent=2, sort_keys=True))
    else:
        for key, value in result.items():
            print(f"{key}\t{value}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
