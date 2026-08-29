#!/usr/bin/env python3
"""Survey a cut-and-splice normal form for low-K singleton colorings.

The canonical Pascal coloring partitions the words of {0,1,2}^K into chains.
This program cuts those chains into contiguous rank intervals and asks whether
every majorized target type can be obtained by making each new chain from one
interval, or from two compatible intervals.  The exact-cover search uses the
actual ternary words, so every reported cover is a proper coloring of Q_K.
"""

from __future__ import annotations

import argparse
import random
from collections import Counter
from dataclasses import dataclass
from time import monotonic
from typing import Sequence


@dataclass(frozen=True)
class Fragment:
    mask: int
    source: int
    depth: int
    low_rank: int
    high_rank: int


@dataclass(frozen=True)
class Candidate:
    mask: int
    pieces: int


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
    return [list(chain) for chain in chains]


def chain_depth(index: int) -> int:
    return 0 if index == 0 else index.bit_length()


def word_rank(word: str) -> int:
    """Binary Pascal rank: inner symbol 1 is zero, either outer symbol is one."""
    return int("".join("0" if symbol == "1" else "1" for symbol in word), 2)


def conflict(left: str, right: str) -> bool:
    for a, b in zip(left, right):
        if a != b:
            return {a, b} == {"0", "2"}
    return False


def popcount(value: int) -> int:
    return bin(value).count("1")


def dominated_partitions(total: int, profile: Sequence[int]) -> list[tuple[int, ...]]:
    prefix: list[int] = []
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


def is_dominated(target: Sequence[int], profile: Sequence[int]) -> bool:
    target_sum = 0
    profile_sum = 0
    for index in range(max(len(target), len(profile))):
        target_sum += target[index] if index < len(target) else 0
        profile_sum += profile[index] if index < len(profile) else 0
        if target_sum > profile_sum:
            return False
    return target_sum == profile_sum


def fragments(k: int) -> tuple[list[str], list[Fragment]]:
    chains = canonical_chains(k)
    words = sorted(word for chain in chains for word in chain)
    word_index = {word: index for index, word in enumerate(words)}
    result: list[Fragment] = []
    for source, chain in enumerate(chains):
        ordered = sorted(chain, key=word_rank)
        for begin in range(len(ordered)):
            mask = 0
            for end in range(begin, len(ordered)):
                mask |= 1 << word_index[ordered[end]]
                result.append(
                    Fragment(
                        mask=mask,
                        source=source,
                        depth=chain_depth(source),
                        low_rank=word_rank(ordered[begin]),
                        high_rank=word_rank(ordered[end]),
                    )
                )
    return words, result


def union_is_chain(mask: int, words: Sequence[str]) -> bool:
    members = [words[index] for index in range(len(words)) if mask & (1 << index)]
    return all(
        not conflict(members[i], members[j])
        for i in range(len(members))
        for j in range(i)
    )


def build_candidates(
    k: int,
    variant: str,
) -> tuple[list[str], list[Candidate], int]:
    words, pieces = fragments(k)
    best: dict[int, int] = {piece.mask: 1 for piece in pieces}

    if variant == "intervals-only":
        candidates = [Candidate(mask, count) for mask, count in best.items()]
        candidates.sort(key=lambda candidate: (candidate.pieces, candidate.mask))
        return words, candidates, len(pieces)

    for left_index, left in enumerate(pieces):
        for right in pieces[left_index + 1 :]:
            if left.source == right.source:
                continue
            if left.depth == right.depth and variant != "unrestricted":
                continue

            separated = (
                left.high_rank < right.low_rank or right.high_rank < left.low_rank
            )
            if variant in {
                "normal",
                "depth-gap-two",
                "adjacent-depth",
                "shallower-below",
                "deeper-below",
            } and not separated:
                continue
            if variant == "depth-gap-two" and abs(left.depth - right.depth) > 2:
                continue
            if variant == "adjacent-depth" and abs(left.depth - right.depth) != 1:
                continue
            if variant in {"shallower-below", "deeper-below"}:
                shallow, deep = (
                    (left, right) if left.depth < right.depth else (right, left)
                )
                correctly_ordered = (
                    shallow.high_rank < deep.low_rank
                    if variant == "shallower-below"
                    else deep.high_rank < shallow.low_rank
                )
                if not correctly_ordered:
                    continue

            mask = left.mask | right.mask
            if union_is_chain(mask, words):
                best[mask] = min(best.get(mask, 2), 2)

    candidates = [Candidate(mask, count) for mask, count in best.items()]
    candidates.sort(key=lambda candidate: (candidate.pieces, candidate.mask))
    return words, candidates, len(pieces)


def validate_balanced_k3_example() -> None:
    cover = (
        ("010", "001", "002"),
        ("102", "012", "000"),
        ("100", "011", "022"),
        ("110", "101", "202"),
        ("210", "201", "200"),
        ("112", "121", "222"),
        ("111", "021", "020"),
        ("122", "212", "221"),
        ("120", "211", "220"),
    )
    chains = canonical_chains(3)
    owner = {word: source for source, chain in enumerate(chains) for word in chain}
    assert sorted(word for target_chain in cover for word in target_chain) == sorted(owner)
    for target_chain in cover:
        assert all(
            not conflict(target_chain[i], target_chain[j])
            for i in range(len(target_chain))
            for j in range(i)
        )
        sources = sorted({owner[word] for word in target_chain})
        assert len(sources) <= 2
        intervals = []
        for source in sources:
            ordered = sorted(chains[source], key=word_rank)
            positions = sorted(
                ordered.index(word)
                for word in target_chain
                if owner[word] == source
            )
            assert positions == list(range(positions[0], positions[-1] + 1))
            ranks = [word_rank(ordered[position]) for position in positions]
            intervals.append((min(ranks), max(ranks)))
        if len(sources) == 2:
            assert chain_depth(sources[0]) != chain_depth(sources[1])
            assert (
                intervals[0][1] < intervals[1][0]
                or intervals[1][1] < intervals[0][0]
            )


def exact_cover(
    word_count: int,
    candidates: Sequence[Candidate],
    target: Sequence[int],
    node_limit: int | None = None,
) -> tuple[str, int]:
    sizes = sorted(set(target), reverse=True)
    size_index = {size: index for index, size in enumerate(sizes)}
    initial_counts = tuple(Counter(target)[size] for size in sizes)
    by_word_size: list[list[list[Candidate]]] = [
        [[] for _ in sizes] for _ in range(word_count)
    ]
    for candidate in candidates:
        size = popcount(candidate.mask)
        if size not in size_index:
            continue
        slot = size_index[size]
        mask = candidate.mask
        while mask:
            bit = mask & -mask
            by_word_size[bit.bit_length() - 1][slot].append(candidate)
            mask ^= bit

    full_mask = (1 << word_count) - 1
    failed: set[tuple[int, tuple[int, ...]]] = set()
    nodes = 0
    limited = False

    def search(unused: int, counts: tuple[int, ...]) -> bool:
        nonlocal nodes, limited
        nodes += 1
        if node_limit is not None and nodes > node_limit:
            limited = True
            return False
        if unused == 0:
            return all(count == 0 for count in counts)
        state = (unused, counts)
        if state in failed:
            return False

        best_options: list[tuple[int, Candidate]] | None = None
        remaining_words = unused
        while remaining_words:
            bit = remaining_words & -remaining_words
            word = bit.bit_length() - 1
            options: list[tuple[int, Candidate]] = []
            for slot, count in enumerate(counts):
                if count == 0:
                    continue
                options.extend(
                    (slot, candidate)
                    for candidate in by_word_size[word][slot]
                    if candidate.mask & unused == candidate.mask
                )
            if not options:
                failed.add(state)
                return False
            if best_options is None or len(options) < len(best_options):
                best_options = options
            remaining_words ^= bit

        assert best_options is not None
        best_options.sort(key=lambda item: (item[1].pieces, item[1].mask, item[0]))
        for slot, candidate in best_options:
            next_counts = list(counts)
            next_counts[slot] -= 1
            if search(unused ^ candidate.mask, tuple(next_counts)):
                return True
            if limited:
                return False
        failed.add(state)
        return False

    passed = search(full_mask, initial_counts)
    return ("PASS" if passed else "LIMIT" if limited else "FAIL"), nodes


def survey(k: int, variant: str) -> tuple[str, tuple[int, ...] | None, int, int, float]:
    words, candidates, _ = build_candidates(k, variant)
    profile = tuple(sorted(map(len, canonical_chains(k)), reverse=True))
    targets = dominated_partitions(3**k, profile)
    nodes = 0
    started = monotonic()
    for target in targets:
        status, used = exact_cover(len(words), candidates, target)
        nodes += used
        if status != "PASS":
            return status, target, len(candidates), nodes, monotonic() - started
    return "PASS", None, len(candidates), nodes, monotonic() - started


def print_exact_survey() -> None:
    validate_balanced_k3_example()
    print("CUT-AND-SPLICE EXACT SURVEY")
    expected_type_counts = {1: 2, 2: 15, 3: 1_206}
    print("balanced_example=(3^9) status=VALID")
    for k in range(1, 4):
        profile = tuple(sorted(map(len, canonical_chains(k)), reverse=True))
        targets = dominated_partitions(3**k, profile)
        assert len(targets) == expected_type_counts[k]
        print(f"K={k} profile={profile} dominated_types={len(targets)}")

    expected_normal_candidates = {1: 5, 2: 46}
    for k in range(1, 3):
        status, target, count, nodes, elapsed = survey(k, "normal")
        assert status == "PASS" and target is None
        assert count == expected_normal_candidates[k]
        print(
            f"K={k} variant=normal candidates={count} status={status} "
            f"nodes={nodes} seconds={elapsed:.3f}"
        )

    expected_interval_support = {1: 2, 2: 11, 3: 591}
    expected_first_uncovered = {
        1: None,
        2: (4, 2, 2, 1),
        3: (8, 7, 4, 3, 2, 1, 1, 1),
    }
    for k in range(1, 4):
        words, candidates, _ = build_candidates(k, "intervals-only")
        profile = tuple(sorted(map(len, canonical_chains(k)), reverse=True))
        targets = dominated_partitions(3**k, profile)
        passed = 0
        first_uncovered: tuple[int, ...] | None = None
        for target in targets:
            status, _ = exact_cover(len(words), candidates, target)
            if status == "PASS":
                passed += 1
            elif first_uncovered is None:
                first_uncovered = target
        assert passed == expected_interval_support[k]
        assert first_uncovered == expected_first_uncovered[k]
        print(
            f"K={k} intervals_only_pass={passed}/{len(targets)} "
            f"first_uncovered={first_uncovered}"
        )

    variants = (
        "unrestricted",
        "different-depth",
        "normal",
        "depth-gap-two",
        "adjacent-depth",
        "shallower-below",
        "deeper-below",
    )
    expected_variants = {
        "unrestricted": ("PASS", 839, None),
        "different-depth": ("PASS", 827, None),
        "normal": ("PASS", 821, None),
        "depth-gap-two": ("PASS", 752, None),
        "adjacent-depth": ("FAIL", 474, (8, 3, 3, 3, 3, 3, 3, 1)),
        "shallower-below": ("FAIL", 698, (8, 7, 3, 3, 3, 1, 1, 1)),
        "deeper-below": ("FAIL", 211, (8, 7, 4, 3, 2, 1, 1, 1)),
    }
    for variant in variants:
        status, target, count, nodes, elapsed = survey(3, variant)
        assert (status, count, target) == expected_variants[variant]
        suffix = "" if target is None else f" first_uncovered={target}"
        print(
            f"K=3 variant={variant} candidates={count} status={status} "
            f"nodes={nodes} seconds={elapsed:.3f}{suffix}"
        )


def random_walk_targets(k: int, count: int, seed: int) -> list[tuple[int, ...]]:
    rng = random.Random(seed)
    profile = tuple(sorted(map(len, canonical_chains(k)), reverse=True))
    targets: list[tuple[int, ...]] = []
    slots = 3**k
    for _ in range(count):
        current = list(profile) + [0] * (slots - len(profile))
        for _ in range(rng.randrange(1, 61)):
            pairs = [
                (donor, recipient)
                for donor in range(slots)
                for recipient in range(slots)
                if current[donor] >= current[recipient] + 2
            ]
            if not pairs:
                break
            donor, recipient = rng.choice(pairs)
            current[donor] -= 1
            current[recipient] += 1
            current.sort(reverse=True)
        targets.append(tuple(part for part in current if part))
    return targets


def print_k4_random_samples(count: int, seed: int, node_limit: int) -> None:
    words, candidates, _ = build_candidates(4, "normal")
    passed = 0
    limited = 0
    failed = 0
    nodes = 0
    started = monotonic()
    for index, target in enumerate(random_walk_targets(4, count, seed)):
        status, used = exact_cover(
            len(words), candidates, target, node_limit=node_limit
        )
        nodes += used
        if status == "PASS":
            passed += 1
        elif status == "LIMIT":
            limited += 1
            print(f"K=4 random_index={index} status=LIMIT target={target}")
        else:
            failed += 1
            print(f"K=4 random_index={index} status=FAIL target={target}")
    print(
        f"K=4 random_seed={seed} samples={count} node_limit={node_limit} "
        f"pass={passed} limit={limited} fail={failed} nodes={nodes} "
        f"seconds={monotonic() - started:.3f}"
    )


def print_k4_samples() -> None:
    k = 4
    words, candidates, interval_count = build_candidates(k, "normal")
    profile = tuple(sorted(map(len, canonical_chains(k)), reverse=True))
    samples = (
        profile,
        (16, 15, 11, 9, 7, 5, 5, 5) + (1,) * 8,
        (5,) * 16 + (1,),
        (4,) * 20 + (1,),
        (3,) * 27,
    )
    print("K=4 NORMAL-FORM SAMPLES")
    print(
        f"profile={profile} intervals={interval_count} candidates={len(candidates)}"
    )
    for target in samples:
        assert is_dominated(target, profile)
        started = monotonic()
        status, nodes = exact_cover(len(words), candidates, target, node_limit=500_000)
        assert status == "PASS"
        print(
            f"target={target} status={status} nodes={nodes} "
            f"seconds={monotonic() - started:.3f}"
        )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--k4-samples",
        action="store_true",
        help="also test five selected majorized K=4 targets",
    )
    parser.add_argument(
        "--k4-random",
        type=int,
        default=0,
        metavar="COUNT",
        help="also test COUNT seeded K=4 Robin--Hood-walk targets",
    )
    parser.add_argument("--seed", type=int, default=20260829)
    parser.add_argument("--node-limit", type=int, default=20_000)
    args = parser.parse_args()
    print_exact_survey()
    if args.k4_samples:
        print_k4_samples()
    if args.k4_random:
        print_k4_random_samples(args.k4_random, args.seed, args.node_limit)


if __name__ == "__main__":
    main()
