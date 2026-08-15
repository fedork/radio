#!/usr/bin/env python3
"""Verify a symbolic aligned-profile construction tree independently of its producer."""

from __future__ import annotations

import argparse
import math
import shlex
from dataclasses import dataclass
from pathlib import Path


LETTERS = "ABCD"
PROFILE_ATOMS = 0
MAX_HEIGHT = 6

Profile = tuple[int, int, int, int]
Part = tuple[Profile, int]
State = tuple[Part, ...]


def parse_profile(word: str) -> Profile:
    counts = tuple(word.count(letter) for letter in LETTERS)
    if PROFILE_ATOMS == 0:
        raise RuntimeError("profile size was not initialized from the certificate summary")
    if len(word) != PROFILE_ATOMS or sum(counts) != PROFILE_ATOMS:
        raise ValueError(f"invalid profile {word!r}")
    if "".join(letter * count for letter, count in zip(LETTERS, counts)) != word:
        raise ValueError(f"noncanonical profile spelling {word!r}")
    return counts  # type: ignore[return-value]


def profile_text(profile: Profile) -> str:
    return "".join(letter * count for letter, count in zip(LETTERS, profile))


def deficit(profile: Profile) -> tuple[int, int, int]:
    _, b, c, d = profile
    return d, c + d, b + c + d


def refine(profile: Profile) -> tuple[int, int, int, int]:
    a, b, c, d = profile
    return 2 * a + b, b + c, c + d, d


def lift(profile: Profile) -> Profile:
    value = profile
    atoms = sum(value)
    while atoms < PROFILE_ATOMS:
        value = refine(value)
        atoms *= 2
    if atoms != PROFILE_ATOMS:
        raise ValueError("profile does not lift to configured normalization")
    return value


def all_profiles() -> list[Profile]:
    out = []
    for a in range(PROFILE_ATOMS + 1):
        for b in range(PROFILE_ATOMS - a + 1):
            for c in range(PROFILE_ATOMS - a - b + 1):
                d = PROFILE_ATOMS - a - b - c
                out.append((a, b, c, d))
    return out


def normalize(parts: list[Part]) -> State:
    kept = [(profile, height) for profile, height in parts if height]
    if sum(height for _, height in kept) > MAX_HEIGHT:
        raise ValueError("state height exceeds six")
    kept.sort(key=lambda part: (deficit(part[0]), -part[1]))
    return tuple(kept)


def parse_state(encoded: str) -> State:
    if encoded == "-":
        return ()
    parts = []
    for item in encoded.split(","):
        word, height_text = item.split(":", 1)
        height = int(height_text)
        if height <= 0:
            raise ValueError("serialized states may not contain zero-height parts")
        parts.append((parse_profile(word), height))
    state = tuple(parts)
    if state != normalize(parts):
        raise ValueError(f"state is not normalized: {encoded}")
    return state


def parse_split(encoded: str) -> tuple[tuple[Profile, int], ...]:
    out = []
    for item in encoded.split(","):
        word, height_text = item.split(":", 1)
        out.append((parse_profile(word), int(height_text)))
    return tuple(out)


def add_states(left: State, right: State) -> State:
    return normalize(list(left) + list(right))


def split_part(part: Part, selected: Profile, selected_height: int) -> tuple[State, State, State]:
    parent, height = part
    if not 0 <= selected_height <= height:
        raise ValueError("selected height lies outside its parent")
    total = refine(parent)
    if any(take > available for take, available in zip(selected, total)):
        raise ValueError(
            f"selected profile {profile_text(selected)} is not contained in refined "
            f"{profile_text(parent)}"
        )
    complement = tuple(available - take for take, available in zip(selected, total))
    if sum(complement) != PROFILE_ATOMS:
        raise ValueError("complement does not contain eight atoms")
    complement = complement  # type: ignore[assignment]
    both = normalize([(selected, selected_height)])
    mixed = normalize(
        [(selected, height - selected_height), (complement, selected_height)]  # type: ignore[list-item]
    )
    neither = normalize([(complement, height - selected_height)])  # type: ignore[list-item]
    return both, mixed, neither


def children_of(state: State, split: tuple[tuple[Profile, int], ...]) -> tuple[State, State, State]:
    if len(state) != len(split):
        raise ValueError("split arity does not match state arity")
    children: list[State] = [(), (), ()]
    for part, (selected, selected_height) in zip(state, split):
        local = split_part(part, selected, selected_height)
        children = [add_states(children[index], local[index]) for index in range(3)]
    return children[0], children[1], children[2]


def reference_profiles(levels: int) -> tuple[Profile, ...]:
    unit = {
        letter: tuple(1 if index == position else 0 for index in range(4))
        for position, letter in enumerate(LETTERS)
    }
    out = []
    for letter in "ABCCDD":
        value = unit[letter]
        for _ in range(levels):
            value = refine(value)  # type: ignore[arg-type]
        out.append(value)
    return tuple(out)  # type: ignore[return-value]


REFERENCE: tuple[Profile, ...] = ()


def eventual_nonnegative(value: tuple[int, int, int]) -> bool:
    return next((coefficient > 0 for coefficient in value if coefficient), True)


def evaluate(value: tuple[int, int, int], base: int) -> int:
    return value[0] * math.comb(base, 2) + value[1] * base + value[2]


def eventual_threshold(value: tuple[int, int, int]) -> int:
    if not eventual_nonnegative(value):
        raise ValueError(f"eventually negative deficit difference {value}")
    for base in range(3, 1_000_000):
        forward_difference = value[0] * base + value[1]
        if evaluate(value, base) >= 0 and forward_difference >= 0:
            return base
    raise ValueError("threshold search exceeded independent bound")


def leaf_threshold(state: State) -> int:
    if any(height != 1 for _, height in state):
        raise ValueError("a construction leaf is not singleton")
    expanded = sorted((profile for profile, _ in state), key=deficit)
    threshold = 3
    for left, right in zip(expanded, expanded[1:]):
        difference = tuple(a - b for a, b in zip(deficit(right), deficit(left)))
        threshold = max(threshold, eventual_threshold(difference))
    left_sum = [0, 0, 0]
    right_sum = [0, 0, 0]
    for index, profile in enumerate(expanded):
        for coefficient in range(3):
            left_sum[coefficient] += deficit(profile)[coefficient]
            right_sum[coefficient] += deficit(REFERENCE[index])[coefficient]
        difference = tuple(left_sum[i] - right_sum[i] for i in range(3))
        if not eventual_nonnegative(difference):
            raise ValueError(f"leaf fails eventual singleton majorization at prefix {index + 1}")
        threshold = max(threshold, eventual_threshold(difference))
    return threshold


@dataclass(frozen=True)
class Node:
    node_id: int
    parent: int
    outcome: int
    level: int
    state: State
    split: tuple[tuple[Profile, int], ...] | None
    threshold: int | None


def fields_from_line(line: str) -> dict[str, str]:
    fields = {}
    for token in shlex.split(line):
        if "=" in token:
            key, value = token.split("=", 1)
            fields[key] = value
    return fields


def main() -> int:
    global PROFILE_ATOMS, REFERENCE
    parser = argparse.ArgumentParser()
    parser.add_argument("tree", type=Path)
    args = parser.parse_args()
    lines = args.tree.read_text().splitlines()

    summaries = [
        fields_from_line(line)
        for line in lines
        if line.startswith("atom_profile_tree_certificate ")
    ]
    if len(summaries) != 1:
        raise ValueError(f"expected one tree certificate summary, found {len(summaries)}")
    summary = summaries[0]
    if summary.get("version") != "1":
        raise ValueError("unsupported tree certificate version")
    PROFILE_ATOMS = int(summary["profile_atoms"])
    if PROFILE_ATOMS < 8 or PROFILE_ATOMS > 16 or PROFILE_ATOMS & (PROFILE_ATOMS - 1):
        raise ValueError(f"unsupported profile atom count {PROFILE_ATOMS}")
    levels = PROFILE_ATOMS.bit_length() - 1
    if int(summary["normalization_levels"]) != levels:
        raise ValueError("normalization level does not match profile atom count")
    REFERENCE = reference_profiles(levels)

    nodes: dict[int, Node] = {}
    for line in lines:
        if not line.startswith("atom_profile_tree_node "):
            continue
        fields = fields_from_line(line)
        node_id = int(fields["id"])
        if node_id in nodes:
            raise ValueError(f"duplicate node id {node_id}")
        has_split = "split" in fields
        has_leaf = "leaf_threshold" in fields
        if has_split == has_leaf:
            raise ValueError(f"node {node_id} must have exactly one of split/leaf_threshold")
        nodes[node_id] = Node(
            node_id=node_id,
            parent=int(fields["parent"]),
            outcome=int(fields["outcome"]),
            level=int(fields["level"]),
            state=parse_state(fields["state"]),
            split=parse_split(fields["split"]) if has_split else None,
            threshold=int(fields["leaf_threshold"]) if has_leaf else None,
        )

    root_id = int(summary["root"])
    if int(summary["nodes"]) != len(nodes):
        raise ValueError("summary node count disagrees with serialized nodes")
    if root_id not in nodes or nodes[root_id].parent != -1 or nodes[root_id].outcome != -1:
        raise ValueError("invalid root linkage")

    positive_candidates = [
        fields_from_line(line)
        for line in lines
        if line.startswith("height6_candidate ") and " answer=YES " in f" {line} "
    ]
    if len(positive_candidates) > 1:
        raise ValueError("tree output contains more than one positive height-6 candidate")
    if positive_candidates:
        candidate = positive_candidates[0]
        d_profile = parse_profile(candidate["D"])
        ordered = sorted(all_profiles(), key=deficit)
        if int(candidate["rank"]) != ordered.index(d_profile) + 1:
            raise ValueError("printed candidate rank disagrees with independent profile order")
        expected_root = normalize(
            [
                (lift((5, 2, 1, 0)), 1),
                (lift((3, 3, 2, 0)), 2),
                (d_profile, 3),
            ]
        )
        if nodes[root_id].state != expected_root:
            raise ValueError("positive candidate line does not describe the serialized root tree")

    by_parent: dict[int, dict[int, Node]] = {}
    for node in nodes.values():
        if node.node_id == root_id:
            continue
        if node.parent not in nodes:
            raise ValueError(f"node {node.node_id} has missing parent {node.parent}")
        outcomes = by_parent.setdefault(node.parent, {})
        if node.outcome in outcomes:
            raise ValueError(f"parent {node.parent} repeats outcome {node.outcome}")
        outcomes[node.outcome] = node

    seen: set[int] = set()
    maximum_root_threshold = 3

    def verify(node: Node) -> None:
        nonlocal maximum_root_threshold
        if node.node_id in seen:
            raise ValueError(f"node {node.node_id} is reachable twice")
        seen.add(node.node_id)
        if node.split is None:
            if by_parent.get(node.node_id):
                raise ValueError(f"leaf {node.node_id} has children")
            expected = leaf_threshold(node.state)
            if node.threshold != expected:
                raise ValueError(
                    f"leaf {node.node_id} threshold: expected {expected}, got {node.threshold}"
                )
            maximum_root_threshold = max(maximum_root_threshold, node.level + expected)
            return

        children = by_parent.get(node.node_id, {})
        if set(children) != {0, 1, 2}:
            raise ValueError(f"internal node {node.node_id} does not have outcomes 0,1,2")
        expected_states = children_of(node.state, node.split)
        for outcome in range(3):
            child = children[outcome]
            if child.level != node.level + 1:
                raise ValueError(f"child {child.node_id} has the wrong level")
            if child.state != expected_states[outcome]:
                raise ValueError(
                    f"child {child.node_id} state disagrees with parent {node.node_id} outcome {outcome}"
                )
            verify(child)

    verify(nodes[root_id])
    if seen != set(nodes):
        raise ValueError(f"unreachable nodes: {sorted(set(nodes) - seen)}")
    if maximum_root_threshold != int(summary["root_base_threshold"]):
        raise ValueError("summary root threshold disagrees with independently checked leaves")

    print(
        f"atom profile tree verified: {len(nodes)} nodes, root base >= "
        f"{maximum_root_threshold}, root state has {sum(h for _, h in nodes[root_id].state)} rows"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
