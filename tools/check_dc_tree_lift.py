#!/usr/bin/env python3
"""Decide whether a projected (D,C+D) tree has an exact A--D lift.

The projected tree fixes the selected D count, selected C+D count, and selected
height for every part.  For an exact profile, only the selected B count remains
free.  This checker exhausts that finite interval at every internal node and
re-derives all exact children.  It deliberately shares no search code with the
C++ producer.

The result is scoped to the serialized projected skeleton.  A NO does not say
that another projected tree, or a deeper tree, cannot lift.

Partial-child pruning is sound by subgraph monotonicity: if a partial child
already fails the necessary full-star condition (or the projected recursion),
adjoining the remaining disjoint parts cannot make that substate solvable.
"""

from __future__ import annotations

import argparse
import functools
import itertools
import math
import re
import sys
from dataclasses import dataclass
from pathlib import Path


Profile = tuple[int, int, int, int]  # A, B, C, D
Part = tuple[Profile, int]
State = tuple[Part, ...]
DCPart = tuple[int, int, int]  # D, C+D, height
DCState = tuple[DCPart, ...]
DCSplit = tuple[int, int, int]  # selected D, selected C+D, selected height

TREE_NODE_RE = re.compile(r"^dc_tree_node\s+(.*)$")
TREE_CERT_RE = re.compile(r"^dc_tree_certificate\s+(.*)$")
DC_PART_RE = re.compile(r"^\((\d+),(\d+)\):(\d+)$")


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


def parse_dc_parts(text: str) -> tuple[DCPart, ...]:
    if text == "-":
        return ()
    parts: list[DCPart] = []
    for item in text.split(",("):
        if not item.startswith("("):
            item = "(" + item
        match = DC_PART_RE.fullmatch(item)
        if not match:
            raise ValueError(f"malformed projected part {item!r}")
        parts.append(tuple(map(int, match.groups())))
    return tuple(parts)


def deficit(profile: Profile) -> tuple[int, int, int]:
    _, b, c, d = profile
    return d, c + d, b + c + d


def normalize(parts: list[Part] | tuple[Part, ...]) -> State:
    return tuple(
        sorted(
            (part for part in parts if part[1] > 0),
            key=lambda part: (deficit(part[0]), -part[1]),
        )
    )


def normalize_dc(parts: list[DCPart] | tuple[DCPart, ...]) -> DCState:
    return tuple(
        sorted(
            (part for part in parts if part[2] > 0),
            key=lambda part: (part[0], part[1], -part[2]),
        )
    )


def project_part(part: Part) -> DCPart:
    profile, height = part
    _, _, c, d = profile
    return d, c + d, height


def project(state: State) -> DCState:
    return normalize_dc([project_part(part) for part in state])


def refine(profile: Profile) -> Profile:
    a, b, c, d = profile
    return 2 * a + b, b + c, c + d, d


def lift(profile: Profile, atoms: int) -> Profile:
    value = profile
    size = sum(value)
    while size < atoms:
        value = refine(value)
        size *= 2
    if size != atoms:
        raise ValueError(f"profile of size {sum(profile)} does not refine to {atoms} atoms")
    return value


def ordered_profiles(atoms: int) -> list[Profile]:
    profiles: list[Profile] = []
    for a in range(atoms + 1):
        for b in range(atoms - a + 1):
            for c in range(atoms - a - b + 1):
                d = atoms - a - b - c
                profiles.append((a, b, c, d))
    profiles.sort(key=deficit)
    return profiles


def reference_profiles(atoms: int) -> tuple[Profile, ...]:
    if atoms < 1 or atoms & (atoms - 1):
        raise ValueError("profile atom count must be a positive power of two")
    units: tuple[Profile, ...] = (
        (1, 0, 0, 0),
        (0, 1, 0, 0),
        (0, 0, 1, 0),
        (0, 0, 1, 0),
        (0, 0, 0, 1),
        (0, 0, 0, 1),
    )
    return tuple(lift(profile, atoms) for profile in units)


def eventually_nonnegative(value: tuple[int, int, int]) -> bool:
    return next((coefficient > 0 for coefficient in value if coefficient), True)


def eventual_threshold(value: tuple[int, int, int]) -> int:
    if not eventually_nonnegative(value):
        raise ValueError(f"threshold requested for eventually negative value {value}")
    for base in range(3, 1_000_000):
        if (
            value[0] * math.comb(base, 2) + value[1] * base + value[2] >= 0
            and value[0] * base + value[1] >= 0
        ):
            return base
    raise ValueError("eventual threshold search exceeded independent bound")


def profile_text(profile: Profile) -> str:
    return "".join(letter * count for letter, count in zip("ABCD", profile))


def state_text(state: State) -> str:
    return ",".join(f"{profile_text(profile)}:{height}" for profile, height in state) or "-"


@dataclass(frozen=True)
class Node:
    node_id: int
    parent: int
    outcome: int
    level: int
    state: DCState
    split: tuple[DCSplit, ...] | None


def read_tree(path: Path) -> tuple[int, dict[int, Node]]:
    nodes: dict[int, Node] = {}
    summary: dict[str, str] | None = None
    for raw_line in path.read_text().splitlines():
        line = raw_line.strip()
        node_match = TREE_NODE_RE.match(line)
        if node_match:
            fields = parse_fields(node_match.group(1))
            node_id = int(fields["id"])
            if node_id in nodes:
                raise ValueError(f"duplicate projected tree node {node_id}")
            has_leaf = fields.get("leaf") == "YES"
            has_split = "split" in fields
            if has_leaf == has_split:
                raise ValueError(f"node {node_id} must have exactly one of leaf or split")
            state = parse_dc_parts(fields["state"])
            if state != normalize_dc(state):
                raise ValueError(f"projected state at node {node_id} is not normalized")
            split = parse_dc_parts(fields["split"]) if has_split else None
            if split is not None and len(split) != len(state):
                raise ValueError(f"projected split arity mismatch at node {node_id}")
            nodes[node_id] = Node(
                node_id=node_id,
                parent=int(fields["parent"]),
                outcome=int(fields["outcome"]),
                level=int(fields["level"]),
                state=state,
                split=split,
            )
            continue
        summary_match = TREE_CERT_RE.match(line)
        if summary_match:
            if summary is not None:
                raise ValueError("certificate contains more than one projected tree summary")
            summary = parse_fields(summary_match.group(1))
    if summary is None:
        raise ValueError("missing projected tree summary")
    if summary.get("version") != "1":
        raise ValueError("unsupported projected tree version")
    expected_nodes = int(summary["nodes"])
    if set(nodes) != set(range(expected_nodes)):
        raise ValueError("projected tree node ids are not contiguous")
    root = int(summary["root"])
    if root not in nodes or nodes[root].parent != -1 or nodes[root].outcome != -1:
        raise ValueError("invalid projected tree root")
    return int(summary["profile_atoms"]), nodes


class LiftChecker:
    def __init__(self, atoms: int, nodes: dict[int, Node]) -> None:
        self.atoms = atoms
        self.nodes = nodes
        self.reference = reference_profiles(atoms)
        self.children: dict[int, dict[int, int]] = {}
        for node in nodes.values():
            if node.parent == -1:
                continue
            siblings = self.children.setdefault(node.parent, {})
            if node.outcome in siblings:
                raise ValueError(
                    f"projected parent {node.parent} repeats outcome {node.outcome}"
                )
            siblings[node.outcome] = node.node_id
        for node in nodes.values():
            expected = set() if node.split is None else {0, 1, 2}
            if set(self.children.get(node.node_id, {})) != expected:
                raise ValueError(f"malformed children below projected node {node.node_id}")

        self.state_calls = 0
        self.pairings = 0
        self.cut_assignments = 0
        self.prefix_rejects = 0
        self.leaf_checks = 0
        self.witness: dict[tuple[int, State], tuple[tuple[Profile, int], ...]] = {}

    @functools.lru_cache(maxsize=None)
    def full_star(self, state: State) -> bool:
        expanded: list[Profile] = []
        for profile, height in state:
            expanded.extend([profile] * height)
        expanded.sort(key=deficit)
        if len(expanded) > len(self.reference):
            return False
        left = [0, 0, 0]
        right = [0, 0, 0]
        for index, profile in enumerate(expanded):
            for coefficient in range(3):
                left[coefficient] += deficit(profile)[coefficient]
                right[coefficient] += deficit(self.reference[index])[coefficient]
            difference = tuple(left[i] - right[i] for i in range(3))
            if not eventually_nonnegative(difference):
                return False
        return True

    @functools.lru_cache(maxsize=None)
    def exact_cuts(self, part: Part, projected_split: DCSplit) -> tuple[Profile, ...]:
        parent, height = part
        selected_d, selected_cd, selected_height = projected_split
        if not 0 <= selected_height <= height:
            return ()
        selected_c = selected_cd - selected_d
        if selected_c < 0:
            return ()
        total = refine(parent)
        cuts: list[Profile] = []
        for selected_b in range(self.atoms - selected_cd + 1):
            selected_a = self.atoms - selected_b - selected_cd
            selected = (selected_a, selected_b, selected_c, selected_d)
            if all(take <= available for take, available in zip(selected, total)):
                cuts.append(selected)
        return tuple(cuts)

    def split_part(
        self, part: Part, selected: Profile, selected_height: int
    ) -> tuple[State, State, State]:
        parent, height = part
        total = refine(parent)
        if not 0 <= selected_height <= height or any(
            take > available for take, available in zip(selected, total)
        ):
            raise ValueError("internal exact-cut generation error")
        complement = tuple(available - take for take, available in zip(selected, total))
        if sum(complement) != self.atoms:
            raise ValueError("exact complement has the wrong profile size")
        return (
            normalize([(selected, selected_height)]),
            normalize([(selected, height - selected_height), (complement, selected_height)]),
            normalize([(complement, height - selected_height)]),
        )

    @staticmethod
    def add_states(left: State, right: State) -> State:
        return normalize(left + right)

    def projected_pairings_for(
        self,
        state: State,
        projected_state: DCState,
        projected_split: tuple[DCSplit, ...],
    ) -> tuple[tuple[DCSplit, ...], ...]:
        exact_groups: dict[DCPart, list[int]] = {}
        for index, part in enumerate(state):
            exact_groups.setdefault(project_part(part), []).append(index)
        projected_groups: dict[DCPart, list[DCSplit]] = {}
        for projected_part, split in zip(projected_state, projected_split):
            projected_groups.setdefault(projected_part, []).append(split)
        if set(exact_groups) != set(projected_groups):
            return ()
        assignments: list[list[DCSplit | None]] = [[None] * len(state)]
        for projected_part in sorted(exact_groups):
            indices = exact_groups[projected_part]
            splits = projected_groups[projected_part]
            if len(indices) != len(splits):
                return ()
            permutations = sorted(set(itertools.permutations(splits)))
            next_assignments: list[list[DCSplit | None]] = []
            for current in assignments:
                for permutation in permutations:
                    updated = current.copy()
                    for index, split in zip(indices, permutation):
                        updated[index] = split
                    next_assignments.append(updated)
            assignments = next_assignments
        return tuple(
            tuple(split for split in assignment if split is not None)
            for assignment in assignments
        )

    def projected_pairings(
        self, state: State, node: Node
    ) -> tuple[tuple[DCSplit, ...], ...]:
        assert node.split is not None
        return self.projected_pairings_for(state, node.state, node.split)

    @functools.lru_cache(maxsize=None)
    def solve(self, node_id: int, state: State) -> bool:
        self.state_calls += 1
        node = self.nodes[node_id]
        if project(state) != node.state:
            raise ValueError(f"exact state does not project to node {node_id}")
        if not self.full_star(state):
            return False
        if node.split is None:
            self.leaf_checks += 1
            answer = all(height == 1 for _, height in state)
            if not answer:
                raise ValueError(f"projected leaf {node_id} is not exact-singleton")
            return True

        pairings = self.projected_pairings(state, node)
        child_ids = self.children[node_id]
        for pairing in pairings:
            self.pairings += 1
            choices: list[tuple[int, Part, DCSplit, tuple[Profile, ...]]] = []
            for index, (part, projected_split) in enumerate(zip(state, pairing)):
                cuts = self.exact_cuts(part, projected_split)
                if not cuts:
                    choices = []
                    break
                choices.append((index, part, projected_split, cuts))
            if not choices:
                continue
            choices.sort(
                key=lambda item: (
                    len(item[3]),
                    project_part(item[1]),
                    deficit(item[1][0]),
                )
            )
            selected_by_index: list[tuple[Profile, int] | None] = [None] * len(state)

            def assign(index: int, partial: tuple[State, State, State]) -> bool:
                if index == len(choices):
                    outcome_order = sorted(
                        range(3),
                        key=lambda outcome: self.nodes[child_ids[outcome]].level,
                        reverse=True,
                    )
                    for outcome in outcome_order:
                        if not self.solve(child_ids[outcome], partial[outcome]):
                            return False
                    key = (node_id, state)
                    self.witness[key] = tuple(
                        selected for selected in selected_by_index if selected is not None
                    )
                    return True

                original, part, projected_split, cuts = choices[index]
                selected_height = projected_split[2]
                for selected in cuts:
                    self.cut_assignments += 1
                    local = self.split_part(part, selected, selected_height)
                    next_children = tuple(
                        self.add_states(partial[outcome], local[outcome])
                        for outcome in range(3)
                    )
                    if any(not self.full_star(child) for child in next_children):
                        self.prefix_rejects += 1
                        continue
                    selected_by_index[original] = (selected, selected_height)
                    if assign(index + 1, next_children):
                        return True
                    selected_by_index[original] = None
                return False

            if assign(0, ((), (), ())):
                return True
        return False


DCChildren = tuple[DCState, DCState, DCState]
DCOption = tuple[DCSplit, DCChildren]


class DCSolver:
    """Exact bounded-depth solver for the two-coordinate relaxation."""

    def __init__(self, atoms: int) -> None:
        self.atoms = atoms
        self.reference = tuple(
            (profile[3], profile[2] + profile[3])
            for profile in reference_profiles(atoms)
        )
        self.calls = 0
        self.split_assignments = 0
        self.winning_splits_emitted = 0

    @functools.lru_cache(maxsize=None)
    def full_star(self, state: DCState) -> bool:
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

    @staticmethod
    def lineage_possible(state: DCState) -> bool:
        height = sum(part[2] for part in state)
        return sum(part[0] for part in state) >= max(0, height - 4)

    @staticmethod
    def add_states(left: DCState, right: DCState) -> DCState:
        return normalize_dc(left + right)

    @functools.lru_cache(maxsize=None)
    def local_options(self, part: DCPart) -> tuple[DCOption, ...]:
        d, cd, height = part
        refined_d = d
        refined_cd = d + cd
        refined_non_cd = 2 * self.atoms - refined_cd
        options: list[DCOption] = []
        for selected_d in range(d + 1):
            for selected_cd in range(selected_d, self.atoms + 1):
                selected_c = selected_cd - selected_d
                if selected_c > cd or self.atoms - selected_cd > refined_non_cd:
                    continue
                complement_d = refined_d - selected_d
                complement_cd = refined_cd - selected_cd
                if not complement_d <= complement_cd <= self.atoms:
                    continue
                for selected_height in range(height + 1):
                    split = (selected_d, selected_cd, selected_height)
                    children = (
                        normalize_dc([(selected_d, selected_cd, selected_height)]),
                        normalize_dc(
                            [
                                (selected_d, selected_cd, height - selected_height),
                                (complement_d, complement_cd, selected_height),
                            ]
                        ),
                        normalize_dc(
                            [(complement_d, complement_cd, height - selected_height)]
                        ),
                    )
                    options.append((split, children))
        return tuple(options)

    @functools.lru_cache(maxsize=None)
    def viable_local(self, part: DCPart, depth: int) -> tuple[DCOption, ...]:
        return tuple(
            option
            for option in self.local_options(part)
            if all(self.solve(child, depth - 1) for child in option[1])
        )

    def winning_splits(self, state: DCState, depth: int):
        candidates: list[tuple[int, DCPart, tuple[DCOption, ...]]] = []
        for original, part in enumerate(state):
            options = self.viable_local(part, depth)
            if not options:
                return
            candidates.append((original, part, options))
        candidates.sort(key=lambda item: (len(item[2]), item[1]))
        selected_sorted: list[DCSplit | None] = [None] * len(candidates)
        selected_original: list[DCSplit | None] = [None] * len(candidates)

        def assign(index: int, partial: DCChildren):
            if index == len(candidates):
                self.winning_splits_emitted += 1
                yield tuple(split for split in selected_original if split is not None)
                return
            original, part, options = candidates[index]
            for split, local in options:
                if index > 0 and candidates[index - 1][1] == part:
                    previous = selected_sorted[index - 1]
                    assert previous is not None
                    if split < previous:
                        continue
                self.split_assignments += 1
                next_children = tuple(
                    self.add_states(partial[outcome], local[outcome])
                    for outcome in range(3)
                )
                if not all(self.solve(child, depth - 1) for child in next_children):
                    continue
                selected_sorted[index] = split
                selected_original[original] = split
                yield from assign(index + 1, next_children)  # type: ignore[arg-type]
                selected_sorted[index] = None
                selected_original[original] = None

        yield from assign(0, ((), (), ()))

    @functools.lru_cache(maxsize=None)
    def solve(self, state: DCState, depth: int) -> bool:
        self.calls += 1
        if not self.lineage_possible(state) or not self.full_star(state):
            return False
        if not state or all(height == 1 for _, _, height in state):
            return True
        if depth == 0:
            return False
        return next(self.winning_splits(state, depth), None) is not None


class GuidedLiftChecker(LiftChecker):
    """Search every projected skeleton while retaining the exact third coordinate."""

    def __init__(self, atoms: int) -> None:
        super().__init__(atoms, {})
        self.dc = DCSolver(atoms)
        self.projected_splits = 0

    def leaf_threshold(self, state: State) -> int:
        if any(height != 1 for _, height in state):
            raise ValueError("guided construction leaf is not singleton")
        expanded = sorted((profile for profile, _ in state), key=deficit)
        threshold = 3
        for left, right in zip(expanded, expanded[1:]):
            difference = tuple(
                deficit(right)[index] - deficit(left)[index] for index in range(3)
            )
            threshold = max(threshold, eventual_threshold(difference))
        left_sum = [0, 0, 0]
        right_sum = [0, 0, 0]
        for index, profile in enumerate(expanded):
            for coefficient in range(3):
                left_sum[coefficient] += deficit(profile)[coefficient]
                right_sum[coefficient] += deficit(self.reference[index])[coefficient]
            difference = tuple(left_sum[i] - right_sum[i] for i in range(3))
            threshold = max(threshold, eventual_threshold(difference))
        return threshold

    def emit_tree(self, root: State, depth: int) -> None:
        next_id = 0
        root_threshold = 3

        def visit(
            state: State, remaining: int, parent: int, outcome: int, level: int
        ) -> None:
            nonlocal next_id, root_threshold
            node_id = next_id
            next_id += 1
            prefix = (
                f"atom_profile_tree_node id={node_id} parent={parent} outcome={outcome} "
                f"level={level} state={state_text(state)}"
            )
            split = self.witness.get((remaining, state))
            if split is None:
                threshold = self.leaf_threshold(state)
                root_threshold = max(root_threshold, level + threshold)
                print(f"{prefix} leaf_threshold={threshold}")
                return
            print(
                f"{prefix} split="
                + ",".join(
                    f"{profile_text(profile)}:{height}" for profile, height in split
                )
            )
            children: tuple[State, State, State] = ((), (), ())
            for part, (selected, selected_height) in zip(state, split):
                local = self.split_part(part, selected, selected_height)
                children = tuple(
                    self.add_states(children[child_outcome], local[child_outcome])
                    for child_outcome in range(3)
                )
            for child_outcome in range(3):
                visit(
                    children[child_outcome],
                    remaining - 1,
                    node_id,
                    child_outcome,
                    level + 1,
                )

        visit(root, depth, -1, -1, 0)
        print(
            f"atom_profile_tree_certificate version=1 root=0 nodes={next_id} "
            f"root_base_threshold={root_threshold} profile_atoms={self.atoms} "
            f"normalization_levels={self.atoms.bit_length() - 1}"
        )

    @functools.lru_cache(maxsize=None)
    def solve_guided(self, state: State, depth: int) -> bool:
        self.state_calls += 1
        if not self.full_star(state):
            return False
        projected_state = project(state)
        if not self.dc.solve(projected_state, depth):
            return False
        if not state or all(height == 1 for _, height in state):
            self.leaf_checks += 1
            return True
        if depth == 0:
            return False

        for projected_split in self.dc.winning_splits(projected_state, depth):
            self.projected_splits += 1
            for pairing in self.projected_pairings_for(
                state, projected_state, projected_split
            ):
                self.pairings += 1
                choices: list[tuple[int, Part, DCSplit, tuple[Profile, ...]]] = []
                for index, (part, local_projection) in enumerate(zip(state, pairing)):
                    cuts = self.exact_cuts(part, local_projection)
                    if not cuts:
                        choices = []
                        break
                    choices.append((index, part, local_projection, cuts))
                if not choices:
                    continue
                choices.sort(
                    key=lambda item: (
                        len(item[3]),
                        project_part(item[1]),
                        deficit(item[1][0]),
                    )
                )
                selected_by_index: list[tuple[Profile, int] | None] = [None] * len(state)

                def assign(index: int, partial: tuple[State, State, State]) -> bool:
                    if index == len(choices):
                        for outcome in (1, 0, 2):
                            if not self.solve_guided(partial[outcome], depth - 1):
                                return False
                        self.witness[(depth, state)] = tuple(
                            selected
                            for selected in selected_by_index
                            if selected is not None
                        )
                        return True

                    original, part, local_projection, cuts = choices[index]
                    selected_height = local_projection[2]
                    for selected in cuts:
                        self.cut_assignments += 1
                        local = self.split_part(part, selected, selected_height)
                        next_children = tuple(
                            self.add_states(partial[outcome], local[outcome])
                            for outcome in range(3)
                        )
                        if any(not self.full_star(child) for child in next_children):
                            self.prefix_rejects += 1
                            continue
                        selected_by_index[original] = (selected, selected_height)
                        if assign(index + 1, next_children):
                            return True
                        selected_by_index[original] = None
                    return False

                if assign(0, ((), (), ())):
                    return True
        return False


def root_state(atoms: int, rank: int) -> State:
    profiles = ordered_profiles(atoms)
    if not 1 <= rank <= len(profiles):
        raise ValueError(f"rank {rank} lies outside the {atoms}-atom profile list")
    return normalize(
        [
            (lift((5, 2, 1, 0), atoms), 1),
            (lift((3, 3, 2, 0), atoms), 2),
            (profiles[rank - 1], 3),
        ]
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("certificate", type=Path)
    parser.add_argument("--rank", type=int, default=305)
    parser.add_argument("--all-skeletons", action="store_true")
    parser.add_argument("--depth", type=int, default=3)
    parser.add_argument("--emit-tree", action="store_true")
    parser.add_argument("--expect", choices=("YES", "NO"))
    args = parser.parse_args()
    try:
        atoms, nodes = read_tree(args.certificate)
        state = root_state(atoms, args.rank)
        if args.depth < 0:
            raise ValueError("depth must be nonnegative")
        if args.all_skeletons:
            checker = GuidedLiftChecker(atoms)
            answer = checker.solve_guided(state, args.depth)
        else:
            checker = LiftChecker(atoms, nodes)
            root_id = next(node.node_id for node in nodes.values() if node.parent == -1)
            answer = checker.solve(root_id, state)
        verdict = "YES" if answer else "NO"
        if args.expect is not None and verdict != args.expect:
            raise ValueError(f"expected exact lift {args.expect}, got {verdict}")
        candidate = ordered_profiles(atoms)[args.rank - 1]
        candidate_text = "".join(letter * count for letter, count in zip("ABCD", candidate))
        scope = "all_skeletons" if args.all_skeletons else "fixed_skeleton"
        print(
            f"projected tree exact lift: rank={args.rank} candidate={candidate_text} "
            f"answer={verdict} scope={scope} depth={args.depth} nodes={len(nodes)} "
            f"states={checker.state_calls} "
            f"cut_assignments={checker.cut_assignments} "
            f"prefix_rejects={checker.prefix_rejects} leaf_checks={checker.leaf_checks}"
            + (
                f" projected_splits={checker.projected_splits} "
                f"dc_states={checker.dc.solve.cache_info().currsize}"
                if isinstance(checker, GuidedLiftChecker)
                else f" memo={checker.solve.cache_info().currsize} pairings={checker.pairings}"
            )
        )
        if args.emit_tree:
            if not answer or not isinstance(checker, GuidedLiftChecker):
                raise ValueError("--emit-tree requires a positive --all-skeletons search")
            print(
                f"height6_candidate rank={args.rank} D={candidate_text} "
                f"answer=YES depth={args.depth}"
            )
            checker.emit_tree(state, args.depth)
        return 0
    except (KeyError, OSError, StopIteration, ValueError) as error:
        print(f"projected tree lift error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
