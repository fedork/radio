#!/usr/bin/env python3
"""Extract singleton parents with a unique normalized first split through K=3.

Child uniqueness counts normalized (L,M,R) types modulo the genuine L/R
outcome symmetry.  Cut uniqueness additionally requires one multiset of row
triples (l_i,m_i,r_i), modulo equal rows and equal child parts.
"""

from __future__ import annotations

from collections import defaultdict
from dataclasses import dataclass
from typing import Sequence

from singleton_cut_splice_survey import dominated_partitions
from singleton_identical_children_census import (
    Partition,
    Row,
    normalized_allocations,
    profile,
    recombinations,
)


ChildTriple = tuple[Partition, Partition, Partition]


@dataclass(frozen=True)
class UniqueParent:
    parent: Partition
    children: ChildTriple
    allocation_orbits: int


def swap_rows(rows: Sequence[Row]) -> tuple[Row, ...]:
    return tuple(
        sorted(
            ((right, mixed, left) for left, mixed, right in rows),
            reverse=True,
        )
    )


def allocation_orbit_count(parent: Partition, children: ChildTriple) -> int:
    left, mixed, right = children
    allocations = normalized_allocations(parent, left, mixed, right)
    if left == right:
        allocations = {min(rows, swap_rows(rows)) for rows in allocations}
    return len(allocations)


def survey(k: int) -> tuple[set[Partition], list[UniqueParent]]:
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
    unique = []
    for parent, child_orbits in solutions.items():
        if len(child_orbits) != 1:
            continue
        children = next(iter(child_orbits))
        unique.append(
            UniqueParent(
                parent=parent,
                children=children,
                allocation_orbits=allocation_orbit_count(parent, children),
            )
        )
    return parents, sorted(unique, key=lambda item: item.parent)


def has_repeated_child(children: ChildTriple) -> bool:
    left, mixed, right = children
    return left == mixed or left == right or mixed == right


def main() -> None:
    expected = {1: (2, 2), 2: (4, 3), 3: (9, 6)}
    previous_unique = {(1,)}
    print("UNIQUE-SPLIT SURVEY")
    for k in range(1, 4):
        parents, unique = survey(k)
        cut_unique = [item for item in unique if item.allocation_orbits == 1]
        assert (len(unique), len(cut_unique)) == expected[k]
        assert all(has_repeated_child(item.children) for item in unique)
        assert all(
            child in previous_unique
            for item in unique
            for child in item.children
        )

        reverse: dict[ChildTriple, list[Partition]] = defaultdict(list)
        for item in unique:
            reverse[item.children].append(item.parent)
        collisions = {
            children: sorted(fiber)
            for children, fiber in reverse.items()
            if len(fiber) > 1
        }

        print(
            f"K={k} parent_types={len(parents)} child_unique={len(unique)} "
            f"cut_unique={len(cut_unique)} distinct_unique_triples={len(reverse)}"
        )
        for item in unique:
            marker = "CUT-UNIQUE" if item.allocation_orbits == 1 else "CHILD-ONLY"
            print(
                f"  {marker} parent={item.parent} children={item.children} "
                f"allocation_orbits={item.allocation_orbits}"
            )
        for children, fiber in sorted(collisions.items()):
            print(f"  COLLISION children={children} parents={fiber}")

        previous_unique = {item.parent for item in unique}


if __name__ == "__main__":
    main()
