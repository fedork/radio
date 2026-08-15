#!/usr/bin/env python3
"""Independently verify a coinductive (D,C+D) losing-kernel certificate.

The producer is tools/search_atom_profiles.cpp.  This checker deliberately reimplements the
projected refinement/cut algebra and does not trust the producer's bounded-depth memo table.

A listed core denotes its upward closure under adjoining state parts.  For every non-axiomatic
core S, the checker exhausts every synchronized test and verifies that some outcome contains a
listed core (or an immediate lineage/full-star obstruction).  Restricting a strategy to a
substate proves that adjoining more parts cannot repair such an obstruction, so this finite
closure is an all-depth negative certificate.
"""

from __future__ import annotations

import argparse
import functools
import itertools
import math
import re
import sys
from pathlib import Path

Part = tuple[int, int, int]  # D, C+D, height
State = tuple[Part, ...]
Children = tuple[State, State, State]

HEADER_RE = re.compile(r"^dc_kernel_certificate\s+(.*)$")
STATE_RE = re.compile(r"^dc_kernel_state state=(.*)$")
TREE_NODE_RE = re.compile(r"^dc_tree_node\s+(.*)$")
TREE_CERT_RE = re.compile(r"^dc_tree_certificate\s+(.*)$")
PART_RE = re.compile(r"^\((\d+),(\d+)\):(\d+)$")


def parse_fields(text: str) -> dict[str, str]:
    fields: dict[str, str] = {}
    for item in text.split():
        if "=" not in item:
            raise ValueError(f"malformed certificate field {item!r}")
        key, value = item.split("=", 1)
        if key in fields:
            raise ValueError(f"duplicate certificate field {key!r}")
        fields[key] = value
    return fields


def normalize(parts: list[Part] | tuple[Part, ...]) -> State:
    return tuple(sorted((part for part in parts if part[2] > 0), key=lambda p: (p[0], p[1], -p[2])))


def parse_state(text: str) -> State:
    if text == "-":
        return ()
    parts: list[Part] = []
    for item in text.split(",("):
        if not item.startswith("("):
            item = "(" + item
        match = PART_RE.fullmatch(item)
        if not match:
            raise ValueError(f"malformed projected part {item!r}")
        parts.append(tuple(map(int, match.groups())))
    state = normalize(parts)
    if tuple(parts) != state:
        raise ValueError(f"state is not in canonical order: {text}")
    return state


def parse_split(text: str) -> tuple[Part, ...]:
    parts: list[Part] = []
    for item in text.split(",("):
        if not item.startswith("("):
            item = "(" + item
        match = PART_RE.fullmatch(item)
        if not match:
            raise ValueError(f"malformed projected split {item!r}")
        parts.append(tuple(map(int, match.groups())))
    return tuple(parts)


def state_text(state: State) -> str:
    return ",".join(f"({d},{cd}):{height}" for d, cd, height in state) or "-"


def add_states(left: State, right: State) -> State:
    return normalize(left + right)


def ordered_profiles(atoms: int) -> list[tuple[int, int, int, int]]:
    profiles: list[tuple[int, int, int, int]] = []
    for a in range(atoms + 1):
        for b in range(atoms - a + 1):
            for c in range(atoms - a - b + 1):
                d = atoms - a - b - c
                profiles.append((a, b, c, d))
    profiles.sort(key=lambda profile: (profile[3], profile[2] + profile[3], sum(profile[1:])))
    return profiles


def check_rank_boundary(header: dict[str, str], root: State) -> State:
    atoms = int(header["profile_atoms"])
    if atoms != 16:
        raise ValueError("this kernel schema currently certifies the sixteen-atom boundary")
    first = int(header["candidate_rank_first"])
    last = int(header["candidate_rank_last"])
    next_rank = int(header["next_rank"])
    profiles = ordered_profiles(atoms)
    if not (1 <= first <= last < next_rank <= len(profiles)):
        raise ValueError("invalid candidate rank interval")
    candidate_band = profiles[first - 1 : last]
    actual_band = [
        rank for rank, (_, _, c, d) in enumerate(profiles, 1) if (c, d) == (0, 2)
    ]
    if actual_band != list(range(first, last + 1)) or next_rank != last + 1:
        raise ValueError("header does not identify the complete two-D/no-C rank band")
    if len(candidate_band) != last - first + 1 or any(
        (c, d) != (0, 2) for _, _, c, d in candidate_band
    ):
        raise ValueError("candidate rank interval is not exactly the two-D/no-C band")
    if profiles[first - 1] != (14, 0, 0, 2) or profiles[next_rank - 1] != (13, 0, 1, 2):
        raise ValueError("unexpected sixteen-atom rank boundary profiles")
    expected_root = normalize([(0, 1, 1), (0, 2, 2), (2, 2, 3)])
    if root != expected_root:
        raise ValueError("kernel root does not represent the candidate rank band")
    return normalize([(0, 1, 1), (0, 2, 2), (2, 3, 3)])


class KernelChecker:
    def __init__(self, atoms: int, cores: set[State]) -> None:
        self.atoms = atoms
        self.cores = cores
        self.reference = self.make_reference(atoms)
        self.local_transition_count = 0
        self.assignment_count = 0
        self.axiom_cores = 0
        self.cyclic_cores = 0

    @staticmethod
    def make_reference(atoms: int) -> tuple[tuple[int, int], ...]:
        if atoms < 1 or atoms & (atoms - 1):
            raise ValueError("profile_atoms must be a positive power of two")
        levels = int(math.log2(atoms))
        # A, B, C, C, D, D at one atom.  Refinement fixes D and sends CD -> CD+D.
        reference = [(0, 0), (0, 0), (0, 1), (0, 1), (1, 1), (1, 1)]
        for _ in range(levels):
            reference = [(d, cd + d) for d, cd in reference]
        return tuple(reference)

    def validate_state(self, state: State) -> None:
        if not state:
            raise ValueError("the empty state cannot be a losing core")
        if sum(height for _, _, height in state) > 6:
            raise ValueError(f"core exceeds height six: {state_text(state)}")
        for d, cd, height in state:
            if not (0 <= d <= cd <= self.atoms and 1 <= height <= 6):
                raise ValueError(f"invalid core part in {state_text(state)}")

    @functools.lru_cache(maxsize=None)
    def lineage_possible(self, state: State) -> bool:
        height = sum(part[2] for part in state)
        lineages = sum(part[0] for part in state)
        return lineages >= max(0, height - 4)

    @functools.lru_cache(maxsize=None)
    def full_star(self, state: State) -> bool:
        expanded: list[tuple[int, int]] = []
        for d, cd, height in state:
            expanded.extend([(d, cd)] * height)
        expanded.sort()
        if len(expanded) > len(self.reference):
            return False
        left_d = left_cd = right_d = right_cd = 0
        for index, (d, cd) in enumerate(expanded):
            left_d += d
            left_cd += cd
            right_d += self.reference[index][0]
            right_cd += self.reference[index][1]
            if left_d < right_d or (left_d == right_d and left_cd < right_cd):
                return False
        return True

    @functools.lru_cache(maxsize=None)
    def immediate_obstruction(self, state: State) -> bool:
        return not self.lineage_possible(state) or not self.full_star(state)

    @functools.lru_cache(maxsize=None)
    def contains_core(self, state: State) -> bool:
        for size in range(1, len(state) + 1):
            for indices in itertools.combinations(range(len(state)), size):
                if normalize([state[index] for index in indices]) in self.cores:
                    return True
        return False

    def losing_substate(self, state: State) -> bool:
        return self.contains_core(state) or self.immediate_obstruction(state)

    @functools.lru_cache(maxsize=None)
    def local_options(self, part: Part) -> tuple[Children, ...]:
        d, cd, height = part
        refined_d = d
        refined_cd = d + cd
        refined_non_cd = 2 * self.atoms - refined_cd
        options: list[Children] = []
        for selected_d in range(d + 1):
            for selected_cd in range(selected_d, self.atoms + 1):
                selected_c = selected_cd - selected_d
                if selected_c > cd:
                    continue
                if self.atoms - selected_cd > refined_non_cd:
                    continue
                complement_d = refined_d - selected_d
                complement_cd = refined_cd - selected_cd
                if not (complement_d <= complement_cd <= self.atoms):
                    continue
                for selected_height in range(height + 1):
                    both = normalize([(selected_d, selected_cd, selected_height)])
                    mixed = normalize(
                        [
                            (selected_d, selected_cd, height - selected_height),
                            (complement_d, complement_cd, selected_height),
                        ]
                    )
                    neither = normalize(
                        [(complement_d, complement_cd, height - selected_height)]
                    )
                    options.append((both, mixed, neither))
        self.local_transition_count += len(options)
        return tuple(options)

    def children_for_split(self, part: Part, split: Part) -> Children:
        d, cd, height = part
        selected_d, selected_cd, selected_height = split
        refined_cd = d + cd
        refined_non_cd = 2 * self.atoms - refined_cd
        if not (0 <= selected_d <= d and selected_d <= selected_cd <= self.atoms):
            raise ValueError(f"illegal selected projected profile {split}")
        if selected_cd - selected_d > cd:
            raise ValueError(f"selected C count exceeds refinement in {split}")
        if self.atoms - selected_cd > refined_non_cd:
            raise ValueError(f"selected non-CD count exceeds refinement in {split}")
        complement_d = d - selected_d
        complement_cd = refined_cd - selected_cd
        if not (complement_d <= complement_cd <= self.atoms):
            raise ValueError(f"illegal complementary projected profile for {split}")
        if not (0 <= selected_height <= height):
            raise ValueError(f"illegal selected height in {split}")
        return (
            normalize([(selected_d, selected_cd, selected_height)]),
            normalize(
                [
                    (selected_d, selected_cd, height - selected_height),
                    (complement_d, complement_cd, selected_height),
                ]
            ),
            normalize([(complement_d, complement_cd, height - selected_height)]),
        )

    def core_is_closed(self, state: State) -> bool:
        if self.immediate_obstruction(state):
            self.axiom_cores += 1
            return True
        if all(height == 1 for _, _, height in state):
            raise ValueError(f"terminal-good state listed as losing: {state_text(state)}")

        candidates: list[tuple[Children, ...]] = []
        for part in state:
            viable: list[Children] = []
            for option in self.local_options(part):
                if any(self.losing_substate(child) for child in option):
                    continue
                viable.append(option)
            if not viable:
                self.cyclic_cores += 1
                return True
            candidates.append(tuple(viable))
        candidates.sort(key=len)

        empty_children: Children = ((), (), ())

        def has_uncovered_assignment(index: int, partial: Children) -> bool:
            if index == len(candidates):
                return True
            for option in candidates[index]:
                self.assignment_count += 1
                next_children = tuple(
                    add_states(partial[outcome], option[outcome]) for outcome in range(3)
                )
                if any(self.losing_substate(child) for child in next_children):
                    continue
                if has_uncovered_assignment(index + 1, next_children):
                    return True
            return False

        if has_uncovered_assignment(0, empty_children):
            return False
        self.cyclic_cores += 1
        return True

    def check(self) -> None:
        for state in self.cores:
            self.validate_state(state)
        for index, state in enumerate(sorted(self.cores, key=lambda s: (len(s), s)), 1):
            if not self.core_is_closed(state):
                raise ValueError(
                    f"core has a test with no certified losing outcome: {state_text(state)}"
                )
            if index % 500 == 0:
                print(f"checked {index}/{len(self.cores)} kernel states", file=sys.stderr)

    def check_tree(
        self, tree_header: dict[str, str], nodes: dict[int, dict[str, object]]
    ) -> None:
        expected_nodes = int(tree_header["nodes"])
        root_id = int(tree_header["root"])
        if int(tree_header["version"]) != 1:
            raise ValueError("unsupported DC tree version")
        if int(tree_header["profile_atoms"]) != self.atoms:
            raise ValueError("DC tree and kernel use different profile atom counts")
        if set(nodes) != set(range(expected_nodes)):
            raise ValueError("DC tree node ids are not contiguous or disagree with header")
        if root_id not in nodes:
            raise ValueError("DC tree root is missing")
        root = nodes[root_id]
        if root["parent"] != -1 or root["outcome"] != -1 or root["level"] != 0:
            raise ValueError("malformed DC tree root linkage")

        children_by_parent: dict[int, dict[int, int]] = {}
        for node_id, node in nodes.items():
            parent = int(node["parent"])
            outcome = int(node["outcome"])
            if node_id == root_id:
                continue
            if parent not in nodes or outcome not in (0, 1, 2):
                raise ValueError(f"malformed linkage at DC tree node {node_id}")
            outcomes = children_by_parent.setdefault(parent, {})
            if outcome in outcomes:
                raise ValueError(f"duplicate outcome {outcome} below DC tree node {parent}")
            outcomes[outcome] = node_id

        for node_id, node in nodes.items():
            state = node["state"]
            assert isinstance(state, tuple)
            self.validate_tree_state(state)
            if node.get("leaf"):
                if node_id in children_by_parent:
                    raise ValueError(f"leaf DC tree node {node_id} has children")
                if self.immediate_obstruction(state) or not all(
                    height == 1 for _, _, height in state
                ):
                    raise ValueError(f"DC tree leaf is not terminal-good: {state_text(state)}")
                continue

            split = node.get("split")
            if not isinstance(split, tuple) or len(split) != len(state):
                raise ValueError(f"split arity mismatch at DC tree node {node_id}")
            linked = children_by_parent.get(node_id, {})
            if set(linked) != {0, 1, 2}:
                raise ValueError(f"internal DC tree node {node_id} lacks three outcomes")
            computed: list[State] = [(), (), ()]
            for part, local_split in zip(state, split):
                local = self.children_for_split(part, local_split)
                computed = [add_states(computed[i], local[i]) for i in range(3)]
            for outcome, expected_state in enumerate(computed):
                child = nodes[linked[outcome]]
                if child["level"] != int(node["level"]) + 1:
                    raise ValueError(f"wrong level below DC tree node {node_id}")
                if child["state"] != expected_state:
                    raise ValueError(
                        f"wrong outcome {outcome} below DC tree node {node_id}: "
                        f"expected {state_text(expected_state)}, "
                        f"got {state_text(child['state'])}"
                    )

    def validate_tree_state(self, state: State) -> None:
        if sum(height for _, _, height in state) > 6:
            raise ValueError(f"DC tree state exceeds height six: {state_text(state)}")
        for d, cd, height in state:
            if not (0 <= d <= cd <= self.atoms and 1 <= height <= 6):
                raise ValueError(f"invalid DC tree part in {state_text(state)}")


def read_certificate(
    path: Path,
) -> tuple[dict[str, str], set[State], dict[str, str] | None, dict[int, dict[str, object]]]:
    header: dict[str, str] | None = None
    cores: set[State] = set()
    tree_header: dict[str, str] | None = None
    tree_nodes: dict[int, dict[str, object]] = {}
    for raw_line in path.read_text().splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        header_match = HEADER_RE.match(line)
        if header_match:
            if header is not None:
                raise ValueError("certificate contains more than one kernel header")
            header = parse_fields(header_match.group(1))
            continue
        state_match = STATE_RE.match(line)
        if state_match:
            state = parse_state(state_match.group(1))
            if state in cores:
                raise ValueError(f"duplicate kernel state {state_text(state)}")
            cores.add(state)
            continue
        tree_node_match = TREE_NODE_RE.match(line)
        if tree_node_match:
            fields = parse_fields(tree_node_match.group(1))
            node_id = int(fields["id"])
            if node_id in tree_nodes:
                raise ValueError(f"duplicate DC tree node {node_id}")
            node: dict[str, object] = {
                "parent": int(fields["parent"]),
                "outcome": int(fields["outcome"]),
                "level": int(fields["level"]),
                "state": parse_state(fields["state"]),
            }
            if fields.get("leaf") == "YES":
                node["leaf"] = True
            elif "split" in fields:
                node["split"] = parse_split(fields["split"])
            else:
                raise ValueError(f"DC tree node {node_id} has neither leaf nor split")
            tree_nodes[node_id] = node
            continue
        tree_cert_match = TREE_CERT_RE.match(line)
        if tree_cert_match:
            if tree_header is not None:
                raise ValueError("certificate contains more than one DC tree header")
            tree_header = parse_fields(tree_cert_match.group(1))
            continue
        raise ValueError(f"unrecognized certificate line: {line!r}")
    if header is None:
        raise ValueError("missing dc_kernel_certificate header")
    if (tree_header is None) != (not tree_nodes):
        raise ValueError("incomplete DC tree certificate")
    return header, cores, tree_header, tree_nodes


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("certificate", type=Path)
    args = parser.parse_args()
    try:
        header, cores, tree_header, tree_nodes = read_certificate(args.certificate)
        required = {
            "version": "1",
            "model": "power_of_two_atom_aligned",
            "fixed_layers": "3,4",
            "implicit_axioms": "d_lineage,full_star",
            "minimized": "multiset",
            "closure": "upward_substate",
            "verdict": "ALL_DEPTH_NO",
        }
        for key, expected in required.items():
            if header.get(key) != expected:
                raise ValueError(f"expected {key}={expected}, got {header.get(key)!r}")
        atoms = int(header["profile_atoms"])
        expected_count = int(header["core_states"])
        if len(cores) != expected_count:
            raise ValueError(f"header says {expected_count} cores, found {len(cores)}")
        root = parse_state(header["root"])
        if root not in cores:
            raise ValueError("certificate root is not a listed core")
        expected_tree_root = check_rank_boundary(header, root)

        checker = KernelChecker(atoms, cores)
        checker.check()
        if tree_header is not None:
            checker.check_tree(tree_header, tree_nodes)
            actual_tree_root = tree_nodes[int(tree_header["root"])]["state"]
            if actual_tree_root != expected_tree_root:
                raise ValueError("DC tree root is not the first post-kernel rank")
        print(
            "verified DC losing kernel: "
            f"atoms={atoms} cores={len(cores)} axioms={checker.axiom_cores} "
            f"cyclic={checker.cyclic_cores} local_options={checker.local_transition_count} "
            f"assignments={checker.assignment_count} root={state_text(root)}"
            + (f" dc_tree_nodes={len(tree_nodes)}" if tree_nodes else "")
        )
        return 0
    except (KeyError, OSError, ValueError) as error:
        print(f"DC kernel certificate error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
