#!/usr/bin/env python3
"""Test symmetric-child restrictions on every full-mass singleton type through K=3.

For fixed normalized child types L, M, R, a legal parent recombination is a partial
matching from the mixed parts M to the disjoint collection of pure parts L union R.
Matched values are added; unmatched values become separate parent rows.  This is
exactly the row condition l_i r_i=0, modulo row permutations.
"""

from __future__ import annotations

import itertools
from functools import lru_cache
from time import monotonic
from typing import Sequence

from singleton_cut_splice_survey import canonical_chains, dominated_partitions


Partition = tuple[int, ...]


def profile(k: int) -> Partition:
    return tuple(sorted(map(len, canonical_chains(k)), reverse=True))


def transfer_distance(left: Sequence[int], right: Sequence[int]) -> int:
    width = max(len(left), len(right))
    return sum(
        abs(
            (left[index] if index < len(left) else 0)
            - (right[index] if index < len(right) else 0)
        )
        for index in range(width)
    ) // 2


def majorizes(left: Sequence[int], right: Sequence[int]) -> bool:
    left_sum = 0
    right_sum = 0
    for index in range(max(len(left), len(right))):
        left_sum += left[index] if index < len(left) else 0
        right_sum += right[index] if index < len(right) else 0
        if left_sum < right_sum:
            return False
    return True


@lru_cache(maxsize=None)
def recombinations(
    left: Partition, mixed: Partition, right: Partition
) -> frozenset[Partition]:
    pure = tuple(sorted(left + right, reverse=True))

    @lru_cache(maxsize=None)
    def visit(
        mixed_index: int, available: Partition, formed: Partition
    ) -> frozenset[Partition]:
        if mixed_index == len(mixed):
            return frozenset((tuple(sorted(formed + available, reverse=True)),))

        value = mixed[mixed_index]
        results = set(
            visit(
                mixed_index + 1,
                available,
                tuple(sorted(formed + (value,), reverse=True)),
            )
        )
        previous = None
        for index, pure_value in enumerate(available):
            if pure_value == previous:
                continue
            previous = pure_value
            remainder = available[:index] + available[index + 1 :]
            results.update(
                visit(
                    mixed_index + 1,
                    remainder,
                    tuple(sorted(formed + (value + pure_value,), reverse=True)),
                )
            )
        return frozenset(results)

    return visit(0, pure, ())


def forms_dominance_chain(children: tuple[Partition, Partition, Partition]) -> bool:
    return any(
        majorizes(first, second) and majorizes(second, third)
        for first, second, third in itertools.permutations(children)
    )


def census(
    k: int,
) -> tuple[set[Partition], set[Partition], set[Partition], set[Partition], int]:
    children = dominated_partitions(3 ** (k - 1), profile(k - 1))
    parents = set(dominated_partitions(3**k, profile(k)))
    equal: set[Partition] = set()
    adjacent: set[Partition] = set()
    chain: set[Partition] = set()
    triples = 0

    for left in children:
        for mixed in children:
            for right in children:
                triples += 1
                child_triple = (left, mixed, right)
                distances = (
                    transfer_distance(left, mixed),
                    transfer_distance(left, right),
                    transfer_distance(mixed, right),
                )
                is_equal = min(distances) == 0
                is_adjacent = min(distances) <= 1
                is_chain = forms_dominance_chain(child_triple)
                if not (is_equal or is_adjacent or is_chain):
                    continue
                outputs = parents.intersection(recombinations(left, mixed, right))
                if is_equal:
                    equal.update(outputs)
                if is_adjacent:
                    adjacent.update(outputs)
                if is_chain:
                    chain.update(outputs)
    return parents, equal, adjacent, chain, triples


def verify_counterexample() -> None:
    target = (8, 3, 3, 3, 3, 3, 3, 1)
    left = (4, 3, 1, 1)
    mixed = (4, 2, 2, 1)
    right = (3, 3, 2, 1)
    assert target in recombinations(left, mixed, right)
    assert majorizes(profile(3), target)
    assert majorizes(left, mixed) and majorizes(mixed, right)
    assert transfer_distance(left, mixed) == 1
    assert transfer_distance(mixed, right) == 1


def expected_k3_exceptions() -> set[Partition]:
    exceptions = {
        (8, 5, 5) + (1,) * 9,
        (8,) + (3,) * 6 + (1,),
        (7, 6, 6, 2) + (1,) * 6,
        (7, 6, 6) + (1,) * 8,
        (6, 6, 6) + (1,) * 9,
    }
    exceptions.update(
        (7,) + (2,) * twos + (1,) * (20 - 2 * twos)
        for twos in range(11)
    )
    return exceptions


def main() -> None:
    started = monotonic()
    verify_counterexample()
    expected = {
        1: (2, 2),
        2: (15, 15),
        3: (1_206, 1_190),
    }
    print("IDENTICAL-CHILDREN CENSUS")
    for k in range(1, 4):
        parents, equal, adjacent, chain, triples = census(k)
        assert (len(parents), len(equal)) == expected[k]
        assert adjacent == parents
        assert chain == parents
        print(
            f"K={k} child_triples={triples} parent_types={len(parents)} "
            f"equal_pair={len(equal)} adjacent_pair={len(adjacent)} "
            f"dominance_chain={len(chain)}"
        )
        if k == 3:
            missing = sorted(parents - equal, reverse=True)
            assert set(missing) == expected_k3_exceptions()
            print(f"K=3 no_equal_pair={len(missing)}")
            for target in missing:
                print(f"  {target}")
    print("counterexample=(8,3,3,3,3,3,3,1) status=VALID")
    print("counterexample_children=((4,3,1,1),(4,2,2,1),(3,3,2,1))")
    print(f"seconds={monotonic() - started:.3f}")


if __name__ == "__main__":
    main()
