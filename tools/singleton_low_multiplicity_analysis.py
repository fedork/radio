#!/usr/bin/env python3
"""Analyze the exact K=4 parent-to-solution relation below four child orbits.

Input logs must come from singleton_split_multiplicity_census with orbit-limit
four and report-low enabled.  The analysis deliberately uses only exact
child-shape orbits; the census log remains the source for every state and cut.
"""

from __future__ import annotations

import argparse
import ast
import bisect
import re
from collections import Counter, defaultdict, deque
from pathlib import Path


Partition = tuple[int, ...]
ChildTriple = tuple[Partition, Partition, Partition]


def load(path: Path) -> dict[Partition, dict[str, object]]:
    parents: dict[Partition, dict[str, object]] = {}
    current: Partition | None = None
    for line in path.read_text().splitlines():
        if line.startswith("LOW_PARENT "):
            match = re.fullmatch(
                r"LOW_PARENT parent=(\(.*\)) child_orbits=(\d+) "
                r"allocation_orbits=(\d+)",
                line,
            )
            assert match is not None
            current = ast.literal_eval(match.group(1))
            parents[current] = {
                "multiplicity": int(match.group(2)),
                "cuts": int(match.group(3)),
                "solutions": [],
            }
        elif line.startswith("LOW_SOLUTION "):
            assert current is not None
            children = line.split(" children=", 1)[1].split(
                " allocation_orbits=", 1
            )[0]
            solutions = parents[current]["solutions"]
            assert isinstance(solutions, list)
            solutions.append(ast.literal_eval(children))
    return parents


def base(k: int) -> Partition:
    values = [1]
    for _ in range(k):
        next_values = [0] * (2 * len(values))
        for index, value in enumerate(values):
            next_values[index] += value
            next_values[2 * index] += value
            next_values[2 * index + 1] += value
        values = sorted(next_values, reverse=True)
    return tuple(values)


def padded(partition: Partition, size: int) -> Partition:
    return partition + (0,) * (size - len(partition))


def dominates(upper: Partition, lower: Partition, size: int) -> bool:
    upper_sum = 0
    lower_sum = 0
    for upper_part, lower_part in zip(padded(upper, size), padded(lower, size)):
        upper_sum += upper_part
        lower_sum += lower_part
        if upper_sum < lower_sum:
            return False
    return True


def transfer_distance(first: Partition, second: Partition, size: int) -> int:
    return sum(
        abs(left - right)
        for left, right in zip(padded(first, size), padded(second, size))
    ) // 2


def child_lift_cost(source: ChildTriple, target: ChildTriple) -> int | None:
    costs: list[int] = []
    for oriented in (target, (target[2], target[1], target[0])):
        if all(dominates(left, right, 27) for left, right in zip(source, oriented)):
            costs.append(
                sum(
                    transfer_distance(left, right, 27)
                    for left, right in zip(source, oriented)
                )
            )
    return min(costs) if costs else None


def best_fit_cost(parent: Partition, child: Partition) -> int | None:
    """Minimum residual mass when child parts use the smallest fitting rows."""
    rows = sorted(parent)
    cost = 0
    for part in sorted(child, reverse=True):
        index = bisect.bisect_left(rows, part)
        if index == len(rows):
            return None
        cost += rows.pop(index) - part
    return cost


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("k3_log", type=Path)
    parser.add_argument("k4_log", type=Path)
    args = parser.parse_args()

    k3 = load(args.k3_log)
    k4 = load(args.k4_log)
    assert len(k3) == 34
    assert len(k4) == 259
    assert Counter(item["multiplicity"] for item in k4.values()) == {
        1: 30,
        2: 123,
        3: 106,
    }

    rigid_k3 = {
        parent for parent, item in k3.items() if item["multiplicity"] == 1
    }
    rigid_k4 = {
        parent for parent, item in k4.items() if item["multiplicity"] == 1
    }

    child_occurrences: Counter[Partition] = Counter()
    total_solutions = 0
    total_cuts = 0
    for item in k4.values():
        solutions = item["solutions"]
        assert isinstance(solutions, list)
        total_solutions += len(solutions)
        total_cuts += int(item["cuts"])
        for solution in solutions:
            child_occurrences.update(solution)
    distinct_child_layers = Counter(
        int(k3.get(child, {"multiplicity": 4})["multiplicity"])
        for child in child_occurrences
    )
    assert total_solutions == 594
    assert total_cuts == 1741
    assert len(child_occurrences) == 48
    assert distinct_child_layers == {1: 9, 2: 18, 3: 5, 4: 16}

    best_pure_layer: dict[int, Counter[int]] = defaultdict(Counter)
    pure_anchor_exceptions: list[Partition] = []
    for parent, item in k4.items():
        solutions = item["solutions"]
        assert isinstance(solutions, list)
        best = min(
            min(
                int(k3.get(solution[0], {"multiplicity": 4})["multiplicity"]),
                int(k3.get(solution[2], {"multiplicity": 4})["multiplicity"]),
            )
            for solution in solutions
        )
        multiplicity = int(item["multiplicity"])
        best_pure_layer[multiplicity][best] += 1
        if best != 1:
            pure_anchor_exceptions.append(parent)
    assert best_pure_layer == {
        1: Counter({1: 30}),
        2: Counter({1: 122, 2: 1}),
        3: Counter({1: 106}),
    }
    assert pure_anchor_exceptions == [
        (16, 15, 9, 9, 9, 5, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1)
    ]

    parent_base = base(4)
    parent_prefix = []
    running = 0
    for part in parent_base:
        running += part
        parent_prefix.append(running)

    def tight_dyadic_prefixes(parent: Partition) -> tuple[int, ...]:
        running_parent = 0
        result = []
        for index, part in enumerate(parent[:16]):
            running_parent += part
            rows = index + 1
            if rows in (1, 2, 4, 8) and running_parent == parent_prefix[index]:
                result.append(rows)
        return tuple(result)

    dyadic_coverage = Counter()
    for parent, item in k4.items():
        dyadic_coverage[
            (int(item["multiplicity"]), bool(tight_dyadic_prefixes(parent)))
        ] += 1
    assert dyadic_coverage == {
        (1, True): 28,
        (1, False): 2,
        (2, True): 116,
        (2, False): 7,
        (3, True): 84,
        (3, False): 22,
    }

    def has_internal_tight_prefix(parent: Partition) -> bool:
        running_parent = 0
        for index, part in enumerate(parent[:15]):
            running_parent += part
            if running_parent == parent_prefix[index]:
                return True
        return False

    tight_skeleton_coverage = Counter(
        (int(item["multiplicity"]), has_internal_tight_prefix(parent))
        for parent, item in k4.items()
    )
    assert tight_skeleton_coverage == {
        (1, True): 29,
        (1, False): 1,
        (2, True): 122,
        (2, False): 1,
        (3, True): 102,
        (3, False): 4,
    }
    no_tight_skeleton = {
        parent for parent in k4 if not has_internal_tight_prefix(parent)
    }
    assert no_tight_skeleton == {
        (15, 15) + (1,) * 51,
        (15,) + (1,) * 66,
        (14,) + (1,) * 67,
        (13,) * 4 + (1,) * 29,
        (2,) + (1,) * 79,
        (1,) * 81,
    }

    tight_four = {
        (parent[:4], parent[4:]): item
        for parent, item in k4.items()
        if sum(parent[:4]) == 53
    }
    heads = sorted({head for head, _ in tight_four})
    tails = sorted({tail for _, tail in tight_four})
    assert len(tight_four) == 131
    assert len(heads) == 8
    assert len(tails) == 32

    head_degree = Counter(
        head for head, tail in tight_four
    )
    unit_heads = {head for head in heads if head_degree[head] == len(tails)}
    assert len(unit_heads) == 3
    reference_head = min(unit_heads)
    tail_multiplicity = {
        tail: int(tight_four[reference_head, tail]["multiplicity"])
        for tail in tails
    }
    assert Counter(tail_multiplicity.values()) == {1: 7, 2: 19, 3: 6}
    rigid_tails = {tail for tail, value in tail_multiplicity.items() if value == 1}
    head_multiplicity = {}
    for head in heads:
        values = {
            int(tight_four[head, tail]["multiplicity"])
            for tail in rigid_tails
            if (head, tail) in tight_four
        }
        assert len(values) == 1
        head_multiplicity[head] = values.pop()
    assert Counter(head_multiplicity.values()) == {1: 3, 2: 4, 3: 1}
    for head in heads:
        for tail in tails:
            product = head_multiplicity[head] * tail_multiplicity[tail]
            if product <= 3:
                assert (head, tail) in tight_four
                assert int(tight_four[head, tail]["multiplicity"]) == product
            else:
                assert (head, tail) not in tight_four
    tight_four_layers = Counter(
        int(item["multiplicity"]) for item in tight_four.values()
    )
    assert tight_four_layers == {1: 21, 2: 85, 3: 25}

    nearest_coverage: Counter[int] = Counter()
    nearest_totals: Counter[int] = Counter()
    for parent, item in k4.items():
        multiplicity = int(item["multiplicity"])
        solutions = item["solutions"]
        assert isinstance(solutions, list)
        ancestors = [
            rigid for rigid in rigid_k4 if dominates(rigid, parent, 81)
        ]
        parent_distance = min(
            transfer_distance(rigid, parent, 81) for rigid in ancestors
        )
        nearest = [
            rigid
            for rigid in ancestors
            if transfer_distance(rigid, parent, 81) == parent_distance
        ]
        covered = 0
        economical = False
        for solution in solutions:
            costs = []
            for rigid in nearest:
                rigid_solutions = k4[rigid]["solutions"]
                assert isinstance(rigid_solutions, list)
                assert len(rigid_solutions) == 1
                cost = child_lift_cost(rigid_solutions[0], solution)
                if cost is not None:
                    costs.append(cost)
            if costs:
                covered += 1
                economical |= min(costs) <= parent_distance
        assert economical
        nearest_coverage[multiplicity] += covered
        nearest_totals[multiplicity] += len(solutions)
    assert nearest_totals == {1: 30, 2: 246, 3: 318}
    assert nearest_coverage == {1: 30, 2: 246, 3: 316}

    edges: dict[Partition, list[Partition]] = defaultdict(list)
    parents = list(k4)
    for upper in parents:
        for lower in parents:
            if (
                upper != lower
                and transfer_distance(upper, lower, 81) == 1
                and dominates(upper, lower, 81)
            ):
                edges[upper].append(lower)
    reachable = set(rigid_k4)
    queue = deque(rigid_k4)
    while queue:
        parent = queue.popleft()
        for child in edges[parent]:
            if child not in reachable:
                reachable.add(child)
                queue.append(child)
    assert len(reachable) == 176

    g4 = base(4)
    g3 = base(3)
    g4_solutions = k4[g4]["solutions"]
    assert isinstance(g4_solutions, list) and g4_solutions == [(g3, g3, g3)]
    rigid_costs = {
        child: best_fit_cost(g4, child) for child in rigid_k3
    }
    rigid_costs = {
        child: cost for child, cost in rigid_costs.items() if cost is not None
    }
    best_fit_anchor = min(rigid_costs, key=lambda child: rigid_costs[child])
    assert best_fit_anchor == (8, 5, 5, 5, 1, 1, 1, 1)
    assert best_fit_anchor not in g4_solutions[0]

    print("SINGLETON LOW-MULTIPLICITY ANALYSIS K=4")
    print(
        "parents=259 solutions=594 allocation_orbits=1741 "
        "distinct_child_shapes=48"
    )
    print("distinct_children_by_layer=1:9,2:18,3:5,>=4:16")
    print("rigid_pure_anchor=258/259 exceptional_parent=" + str(
        pure_anchor_exceptions[0]
    ))
    print("dyadic_tight_prefix=228/259 no_dyadic=31")
    print("any_internal_tight_prefix=253/259 no_internal_tight=6")
    print(
        "tight_prefix_4_parents=131 head_layers=1:3,2:4,3:1 "
        "tail_layers=1:7,2:19,3:6 full_layers=1:21,2:85,3:25"
    )
    print(
        "nearest_rigid_deformation_solution_orbits="
        f"{sum(nearest_coverage.values())}/{sum(nearest_totals.values())} "
        "economical_parent_coverage=259/259"
    )
    print("unit_transfer_paths_inside_low_layer=176/259 missing=83")
    print(
        "sorted_best_fit_fails_on_G4 candidate=" + str(best_fit_anchor) +
        " actual=" + str(g4_solutions[0])
    )


if __name__ == "__main__":
    main()
