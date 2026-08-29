#!/usr/bin/env python3
"""Analyze rigid-child coverage and the low-multiplicity K=3 split layers.

This complements singleton_split_multiplicity_census.cpp.  The compiled census
scales to K=4; this small exact Python survey keeps the complete K=3 child-orbit
index so it can inspect rigid-child coverage and the local dominance graph.
"""

from __future__ import annotations

from collections import Counter, defaultdict, deque

from singleton_cut_splice_survey import dominated_partitions
from singleton_identical_children_census import profile, recombinations
from singleton_unique_split_survey import allocation_orbit_count, survey


Partition = tuple[int, ...]
ChildTriple = tuple[Partition, Partition, Partition]


def solution_index(k: int) -> tuple[set[Partition], dict[Partition, set[ChildTriple]]]:
    child_types = dominated_partitions(3 ** (k - 1), profile(k - 1))
    parents = set(dominated_partitions(3**k, profile(k)))
    solutions: dict[Partition, set[ChildTriple]] = defaultdict(set)
    for left in child_types:
        for mixed in child_types:
            for right in child_types:
                triple = min((left, mixed, right), (right, mixed, left))
                for parent in parents.intersection(recombinations(left, mixed, right)):
                    solutions[parent].add(triple)
    assert set(solutions) == parents
    return parents, solutions


def downward_neighbors(parent: Partition, parents: set[Partition]) -> set[Partition]:
    """Distinct normalized one-unit Robin--Hood neighbors below parent."""
    result: set[Partition] = set()
    values = parent + (0,)
    for donor_index, donor in enumerate(parent):
        if donor <= 1:
            continue
        for recipient_index in range(donor_index + 1, len(values)):
            recipient = values[recipient_index]
            if donor < recipient + 2:
                continue
            changed = list(parent)
            changed[donor_index] -= 1
            if recipient_index < len(parent):
                changed[recipient_index] += 1
            else:
                changed.append(1)
            normalized = tuple(sorted((value for value in changed if value), reverse=True))
            if normalized in parents and normalized != parent:
                result.add(normalized)
    return result


def main() -> None:
    parents, solutions = solution_index(3)
    rigid_children = {item.parent for item in survey(2)[1]}

    best_rigid_count: dict[Partition, int] = {}
    for parent, triples in solutions.items():
        best_rigid_count[parent] = max(
            sum(child in rigid_children for child in triple)
            for triple in triples
        )
    best_histogram = Counter(best_rigid_count.values())
    assert best_histogram == {0: 1, 1: 60, 2: 331, 3: 814}
    no_rigid = {parent for parent, count in best_rigid_count.items() if count == 0}
    assert no_rigid == {(3,) * 9}

    orbit_histogram = Counter(len(triples) for triples in solutions.values())
    assert tuple(orbit_histogram[count] for count in (1, 2, 3)) == (9, 19, 6)

    low_cut_histogram: Counter[int] = Counter()
    low_cut_details: list[tuple[Partition, int, tuple[int, ...]]] = []
    for parent, triples in solutions.items():
        if len(triples) > 3:
            continue
        cuts_by_child = tuple(
            allocation_orbit_count(parent, triple) for triple in sorted(triples)
        )
        total_cuts = sum(cuts_by_child)
        low_cut_histogram[total_cuts] += 1
        if total_cuts <= 3:
            low_cut_details.append((parent, len(triples), cuts_by_child))
    assert tuple(low_cut_histogram[count] for count in (1, 2, 3)) == (6, 4, 8)

    adjacency = {parent: downward_neighbors(parent, parents) for parent in parents}
    upward: dict[Partition, set[Partition]] = {parent: set() for parent in parents}
    undirected: dict[Partition, set[Partition]] = {parent: set() for parent in parents}
    for parent, children in adjacency.items():
        for child in children:
            upward[child].add(parent)
            undirected[parent].add(child)
            undirected[child].add(parent)

    rigid_parents = {parent for parent, triples in solutions.items() if len(triples) == 1}
    assert all(len(upward[parent]) <= 1 for parent in rigid_parents)
    low_up_degree = {parent for parent in parents if len(upward[parent]) <= 1}
    assert len(low_up_degree) == 14
    assert len(low_up_degree - rigid_parents) == 5

    distance = {parent: 0 for parent in rigid_parents}
    queue = deque(rigid_parents)
    while queue:
        parent = queue.popleft()
        for neighbor in undirected[parent]:
            if neighbor not in distance:
                distance[neighbor] = distance[parent] + 1
                queue.append(neighbor)
    distance_two = Counter(
        distance[parent] for parent, triples in solutions.items() if len(triples) == 2
    )
    distance_three = Counter(
        distance[parent] for parent, triples in solutions.items() if len(triples) == 3
    )
    assert distance_two == {1: 15, 2: 4}
    assert distance_three == {1: 4, 2: 1, 3: 1}

    print("RIGID-CHILD AND LOW-MULTIPLICITY SURVEY K=3")
    print(
        "parents=1206 child_orbits_1=9 child_orbits_2=19 "
        "child_orbits_3=6 child_orbits_ge4=1172"
    )
    print(
        "best_rigid_children_0=1 best_rigid_children_1=60 "
        "best_rigid_children_2=331 best_rigid_children_3=814"
    )
    print(f"no_rigid_child={next(iter(no_rigid))}")
    print("cut_orbits_1=6 cut_orbits_2=4 cut_orbits_3=8")
    print(
        "rigid_upward_unit_degree_le1=9/9 "
        "all_upward_unit_degree_le1=14 nonrigid_among_them=5"
    )
    print("child_orbits_2_distance_from_rigid=1:15,2:4")
    print("child_orbits_3_distance_from_rigid=1:4,2:1,3:1")
    print("CUT-LAYERS-AT-MOST-3")
    for parent, child_orbits, cuts_by_child in sorted(low_cut_details):
        print(
            f"  parent={parent} child_orbits={child_orbits} "
            f"cuts_by_child={cuts_by_child} total_cuts={sum(cuts_by_child)}"
        )


if __name__ == "__main__":
    main()
