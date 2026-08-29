#!/usr/bin/env python3
"""Construct monotone direct-recoloring witnesses for every K=3 singleton type.

A color class is a chain in the lexicographic V-poset.  A direct recoloring
moves one word from a class of size at least two more than the recipient to a
recipient with which that word has no conflict.  Thus every step implements a
unit Robin--Hood transfer without a Kempe exchange.

The main pass keeps one especially useful (unit-ready) coloring per attained
integer partition.  Five partitions are missed by that normalization; small
targeted searches construct them directly.  The final comparison is against
all full-mass partitions dominated by G_3.
"""

from __future__ import annotations

from collections import deque
from typing import Sequence


K = 3


def canonical_chains(k: int) -> list[list[str]]:
    chains: list[tuple[str, ...]] = [("",)]
    for _ in range(k):
        old = chains
        width = len(old)
        upper: list[tuple[str, ...]] = []
        for chain in old:
            upper.append(tuple("0" + word for word in chain))
            upper.append(tuple("2" + word for word in chain))
        lower = [tuple("1" + word for word in chain) for chain in old]
        chains = [lower[j] + upper[j] for j in range(width)] + upper[width:]
    padded = [list(chain) for chain in chains]
    padded.extend([] for _ in range(3**k - len(padded)))
    return padded


def conflict(x: str, y: str) -> bool:
    """Whether x,y are adjacent in Q_K (first difference is 0 versus 2)."""
    for left, right in zip(x, y):
        if left != right:
            return {left, right} == {"0", "2"}
    return False


def safe(word: str, recipient: Sequence[str]) -> bool:
    return all(not conflict(word, other) for other in recipient)


def coloring_type(chains: Sequence[Sequence[str]]) -> tuple[int, ...]:
    return tuple(size for size in sorted(map(len, chains), reverse=True) if size)


def majorizes(parent: Sequence[int], child: Sequence[int]) -> bool:
    parent_sum = 0
    child_sum = 0
    for index in range(max(len(parent), len(child))):
        parent_sum += parent[index] if index < len(parent) else 0
        child_sum += child[index] if index < len(child) else 0
        if parent_sum < child_sum:
            return False
    return True


def unit_ready(chains: Sequence[Sequence[str]]) -> bool:
    """Every ordered size gap of at least two has some direct safe move."""
    for donor in chains:
        for recipient in chains:
            if len(donor) >= len(recipient) + 2:
                if not any(safe(word, recipient) for word in donor):
                    return False
    return True


def move(
    chains: Sequence[Sequence[str]], donor: int, recipient: int, word: str
) -> list[list[str]]:
    result = [list(chain) for chain in chains]
    result[donor].remove(word)
    result[recipient].append(word)
    return result


def dominated_partitions(total: int, profile: Sequence[int]) -> list[tuple[int, ...]]:
    prefix = []
    running = 0
    for part in profile:
        running += part
        prefix.append(running)

    result: list[tuple[int, ...]] = []

    def visit(remainder: int, previous: int, parts: list[int], used: int) -> None:
        if remainder == 0:
            result.append(tuple(parts))
            return
        for part in range(min(previous, remainder, profile[0]), 0, -1):
            next_used = used + part
            index = len(parts)
            bound = prefix[index] if index < len(prefix) else prefix[-1]
            if next_used <= bound:
                visit(remainder - part, part, parts + [part], next_used)

    visit(total, profile[0], [], 0)
    return result


def unit_ready_representatives(
    initial: list[list[str]],
) -> dict[tuple[int, ...], list[list[str]]]:
    initial_type = coloring_type(initial)
    representatives = {initial_type: initial}
    pending = deque([initial_type])

    while pending:
        current_type = pending.popleft()
        current = representatives[current_type]
        for donor_index, donor in enumerate(current):
            for recipient_index, recipient in enumerate(current):
                if len(donor) < len(recipient) + 2:
                    continue
                for word in donor:
                    if not safe(word, recipient):
                        continue
                    candidate = move(current, donor_index, recipient_index, word)
                    candidate_type = coloring_type(candidate)
                    if candidate_type in representatives or not unit_ready(candidate):
                        continue
                    representatives[candidate_type] = candidate
                    pending.append(candidate_type)
    return representatives


def normalized(chains: Sequence[Sequence[str]]) -> tuple[tuple[str, ...], ...]:
    return tuple(
        sorted(
            (tuple(sorted(chain)) for chain in chains),
            key=lambda chain: (-len(chain), chain),
        )
    )


def direct_path(
    initial: list[list[str]], target: tuple[int, ...], node_limit: int = 300_000
) -> list[tuple[int, int, str]] | None:
    """Find a monotone direct-recoloring path to one target type."""
    start = normalized(initial)
    seen: set[tuple[tuple[str, ...], ...]] = set()
    nodes = 0

    def search(
        current: tuple[tuple[str, ...], ...]
    ) -> list[tuple[int, int, str]] | None:
        nonlocal nodes
        nodes += 1
        if coloring_type(current) == target:
            return []
        if nodes > node_limit or current in seen:
            return None
        seen.add(current)

        candidates = []
        for donor_index, donor in enumerate(current):
            for recipient_index, recipient in enumerate(current):
                if len(donor) < len(recipient) + 2:
                    continue
                for word in donor:
                    if not safe(word, recipient):
                        continue
                    candidate = normalized(
                        move(current, donor_index, recipient_index, word)
                    )
                    candidate_type = coloring_type(candidate)
                    if not majorizes(candidate_type, target):
                        continue
                    width = max(len(candidate_type), len(target))
                    distance = sum(
                        abs(
                            (candidate_type[j] if j < len(candidate_type) else 0)
                            - (target[j] if j < len(target) else 0)
                        )
                        for j in range(width)
                    )
                    candidates.append(
                        (
                            distance,
                            candidate_type,
                            donor_index,
                            recipient_index,
                            word,
                            candidate,
                        )
                    )
        candidates.sort(key=lambda item: item[:5])
        for _, _, donor_index, recipient_index, word, candidate in candidates:
            suffix = search(candidate)
            if suffix is not None:
                return [(donor_index, recipient_index, word)] + suffix
        return None

    return search(start)


def validate_coloring(chains: Sequence[Sequence[str]]) -> None:
    words = [word for chain in chains for word in chain]
    assert len(words) == 3**K
    assert len(set(words)) == 3**K
    for chain in chains:
        assert all(
            not conflict(chain[i], chain[j])
            for i in range(len(chain))
            for j in range(i)
        )


def main() -> None:
    initial = canonical_chains(K)
    validate_coloring(initial)
    profile = coloring_type(initial)
    targets = set(dominated_partitions(3**K, profile))

    representatives = unit_ready_representatives(initial)
    attained = set(representatives)
    missing = sorted(targets - attained, reverse=True)

    direct_witnesses = {}
    for target in missing:
        path = direct_path(initial, target)
        assert path is not None, f"no direct path found for {target}"
        direct_witnesses[target] = path
        attained.add(target)

    assert attained == targets
    print(f"K={K}")
    print(f"canonical profile={profile}")
    print(f"dominated full-mass types={len(targets)}")
    print(f"unit-ready representative pass={len(representatives)}")
    print(f"targeted direct paths={len(direct_witnesses)}")
    for target, path in direct_witnesses.items():
        print(f"  {target}: {len(path)} moves")
    print("ALL DOMINATED TYPES REACHED BY MONOTONE DIRECT RECOLORINGS")


if __name__ == "__main__":
    main()
