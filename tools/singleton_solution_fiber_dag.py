#!/usr/bin/env python3
"""Survey feasible first-cut coloring fibers along singleton unit transfers.

For a full-mass normalized state x <=_w G_k, a fiber vertex is a feasible
row bipartition A/B modulo the global A<->B symmetry and permutations of
equal rows.  Feasibility is the exact Fixed-Color Hall criterion, so every
fiber vertex admits at least one legal first cut.

For a normalized Robin--Hood transfer x -> y, an edge between fiber vertices
means that the donor and recipient rows keep their colors while one coin is
moved.  Thus the bipartite edge relation records the most literal local
transport of a first-cut coloring.  The program exhausts the complete k<=3
state DAG and reports births, deaths, mergers, and forward reachability from
the G_k fiber.  It also reports components after every relation pair is made
undirected.  It can survey a downward-closed Lorenz-area ideal at a higher
level; source vertices found there have every possible predecessor present
and therefore are genuine sources of the full DAG.

The rational-grid modes scale both the parent and child profiles by a common
denominator.  Dividing a reported state by that denominator therefore tests
real, rather than only integral, coverage by the fixed-color Hall polytopes.
The exact-support mode retains exactly 2^K positive rows; the unrestricted
mode allows all 3^K padded row slots.  Missing-state summaries include the
minimum scaled Hall defect; --list-rational-holes prints every hole.
"""

from __future__ import annotations

import argparse
import heapq
from collections import Counter, defaultdict
from dataclasses import dataclass
from itertools import combinations, product
from typing import Iterable


State = tuple[int, ...]
Coloring = tuple[int, ...]  # A-row multiplicities, indexed by row value.
GreedyBlock = tuple[int, int, int]  # row value, multiplicity, A multiplicity.


def singleton_base(k: int) -> State:
    cur = [1]
    for _ in range(k):
        nxt = [0] * (2 * len(cur))
        for index, value in enumerate(cur):
            nxt[index] += value
            nxt[2 * index] += value
            nxt[2 * index + 1] += value
        cur = sorted(nxt, reverse=True)
    return tuple(cur)


def show(state: State) -> str:
    return "(" + ",".join(map(str, state)) + ")"


def show_coloring(state: State, coloring: Coloring, largest: int) -> str:
    totals = multiplicities(state, largest)
    a: list[int] = []
    b: list[int] = []
    for value in range(largest, 0, -1):
        a.extend([value] * coloring[value])
        b.extend([value] * (totals[value] - coloring[value]))
    return f"A={show(tuple(a))} B={show(tuple(b))}"


def enumerate_states(
    profile: State,
    minimum_rows: int = 0,
    maximum_rows: int | None = None,
) -> list[State]:
    total = sum(profile)
    maximum = profile[0]
    prefix = [0]
    for value in profile:
        prefix.append(prefix[-1] + value)

    states: list[State] = []

    def visit(remaining: int, previous: int, used: int, parts: list[int]) -> None:
        if remaining == 0:
            if len(parts) >= minimum_rows:
                states.append(tuple(parts))
            return
        if maximum_rows is not None:
            available = maximum_rows - len(parts)
            if available <= 0 or remaining > available * previous:
                return
        required = max(0, minimum_rows - len(parts))
        if remaining < required:
            return
        for value in range(min(previous, remaining, maximum), 0, -1):
            next_used = used + value
            count = len(parts) + 1
            bound = prefix[min(count, len(profile))]
            if next_used > bound:
                continue
            if maximum_rows is not None:
                available_after = maximum_rows - count
                remaining_after = remaining - value
                if remaining_after > available_after * value:
                    continue
            required_after = max(0, minimum_rows - count)
            if remaining - value < required_after:
                continue
            parts.append(value)
            visit(remaining - value, value, next_used, parts)
            parts.pop()

    visit(total, maximum, 0, [])
    return states


def multiplicities(state: State, largest: int) -> Coloring:
    counts = [0] * (largest + 1)
    for value in state:
        counts[value] += 1
    return tuple(counts)


class Hall:
    def __init__(self, child_profile: State):
        self.child_rows = len(child_profile)
        self.prefix = [0]
        for value in child_profile:
            self.prefix.append(self.prefix[-1] + value)

    def h(self, count: int) -> int:
        return self.prefix[min(count, self.child_rows)]

    def feasible(self, a_counts: Coloring, totals: Coloring) -> bool:
        a_values: list[int] = []
        b_values: list[int] = []
        for value in range(len(totals) - 1, 0, -1):
            a_values.extend([value] * a_counts[value])
            b_values.extend([value] * (totals[value] - a_counts[value]))

        pa = [0]
        pb = [0]
        for value in a_values:
            pa.append(pa[-1] + value)
        for value in b_values:
            pb.append(pb[-1] + value)
        for p, demand_a in enumerate(pa):
            for q, demand_b in enumerate(pb):
                if demand_a + demand_b > self.h(p + q) + self.h(p) + self.h(q):
                    return False
        return True

    def maximum_excess(self, a_counts: Coloring, totals: Coloring) -> int:
        """Return the largest demand-minus-capacity Hall violation."""
        a_values: list[int] = []
        b_values: list[int] = []
        for value in range(len(totals) - 1, 0, -1):
            a_values.extend([value] * a_counts[value])
            b_values.extend([value] * (totals[value] - a_counts[value]))

        pa = [0]
        pb = [0]
        for value in a_values:
            pa.append(pa[-1] + value)
        for value in b_values:
            pb.append(pb[-1] + value)
        return max(
            demand_a + demand_b - self.h(p + q) - self.h(p) - self.h(q)
            for p, demand_a in enumerate(pa)
            for q, demand_b in enumerate(pb)
        )


def complement(coloring: Coloring, totals: Coloring) -> Coloring:
    return tuple(total - a for total, a in zip(totals, coloring))


def canonical(coloring: Coloring, totals: Coloring) -> Coloring:
    other = complement(coloring, totals)
    return min(coloring, other)


def feasible_fiber(state: State, largest: int, hall: Hall) -> frozenset[Coloring]:
    totals = multiplicities(state, largest)
    ranges = [range(count + 1) for count in totals[1:]]
    result: set[Coloring] = set()
    for choice in product(*ranges):
        oriented = (0,) + choice
        key = canonical(oriented, totals)
        if oriented != key:
            continue
        if hall.feasible(oriented, totals):
            result.add(oriented)
    return frozenset(result)


def has_feasible_coloring(state: State, largest: int, hall: Hall) -> bool:
    """Return after the first feasible coloring, without materializing its fiber."""
    totals = multiplicities(state, largest)
    ranges = [range(count + 1) for count in totals[1:]]
    for choice in product(*ranges):
        oriented = (0,) + choice
        if oriented != canonical(oriented, totals):
            continue
        if hall.feasible(oriented, totals):
            return True
    return False


def has_balanced_feasible_coloring(state: State, largest: int, hall: Hall) -> bool:
    """Test a 2m-row state by its balanced labelled row subsets.

    Full mass forces m rows of each color.  Fixing the first row in A quotients
    global color reversal and is substantially faster on rational grids than
    scanning every value-multiplicity vector.
    """
    rows = len(state)
    side_rows = hall.child_rows
    if rows != 2 * side_rows:
        raise ValueError("balanced coloring requires exactly twice the child support")
    totals = multiplicities(state, largest)
    for chosen in combinations(range(1, rows), side_rows - 1):
        a_counts = [0] * (largest + 1)
        a_counts[state[0]] += 1
        for index in chosen:
            a_counts[state[index]] += 1
        if hall.feasible(tuple(a_counts), totals):
            return True
    return False


def minimum_coloring_excess(
    state: State,
    largest: int,
    hall: Hall,
    exact_support: bool,
) -> int:
    """Minimize the maximum scaled Hall excess over colorings."""
    totals = multiplicities(state, largest)
    best: int | None = None
    if exact_support:
        rows = len(state)
        side_rows = hall.child_rows
        if rows != 2 * side_rows:
            raise ValueError("exact-support defect requires balanced row counts")
        for chosen in combinations(range(1, rows), side_rows - 1):
            a_counts = [0] * (largest + 1)
            a_counts[state[0]] += 1
            for index in chosen:
                a_counts[state[index]] += 1
            excess = hall.maximum_excess(tuple(a_counts), totals)
            best = excess if best is None else min(best, excess)
    else:
        ranges = [range(count + 1) for count in totals[1:]]
        for choice in product(*ranges):
            oriented = (0,) + choice
            if oriented != canonical(oriented, totals):
                continue
            excess = hall.maximum_excess(oriented, totals)
            best = excess if best is None else min(best, excess)
    if best is None:
        raise AssertionError("state had no coloring choices")
    return best


def inspect_rational_grid(
    k: int,
    maximum_denominator: int,
    exact_support: bool,
    list_holes: bool,
) -> None:
    """Exhaust normalized rational grids for real Hall-polytope coverage.

    A denominator-d grid point x is represented by the integer state d*x.
    Scaling the child profile by d scales every Hall inequality by d, so an
    empty fiber exactly certifies that x is outside every real fixed-color
    base polytope.
    """
    if maximum_denominator < 1:
        raise ValueError("maximum denominator must be positive")
    padded_rows = 3**k
    support = 2**k
    for denominator in range(1, maximum_denominator + 1):
        parent = tuple(denominator * value for value in singleton_base(k))
        child = tuple(denominator * value for value in singleton_base(k - 1))
        if exact_support:
            states = enumerate_states(parent, support, support)
        else:
            states = enumerate_states(parent, 0, padded_rows)
        hall = Hall(child)
        missing: list[State] = []
        for state in states:
            feasible = (
                has_balanced_feasible_coloring(state, parent[0], hall)
                if exact_support
                else has_feasible_coloring(state, parent[0], hall)
            )
            if not feasible:
                missing.append(state)
        mode = "exact-support" if exact_support else "padded"
        first = show(missing[0]) if missing else "none"
        defects = Counter(
            minimum_coloring_excess(state, parent[0], hall, exact_support)
            for state in missing
        )
        defect_summary = (
            ",".join(f"{defect}:{count}" for defect, count in sorted(defects.items()))
            if defects
            else "none"
        )
        print(
            f"RATIONAL_HALL_GRID k={k} denominator={denominator} mode={mode} "
            f"states={len(states)} missing={len(missing)} first_missing={first} "
            f"scaled_defects={defect_summary}"
        )
        if list_holes:
            for state in missing:
                defect = minimum_coloring_excess(
                    state, parent[0], hall, exact_support
                )
                print(
                    f"RATIONAL_HALL_HOLE k={k} denominator={denominator} "
                    f"mode={mode} state={show(state)} scaled_defect={defect}"
                )


def transfers(state: State, padded_rows: int) -> Iterable[tuple[int, int, State]]:
    values = sorted(set(state), reverse=True)
    recipients = list(values)
    if len(state) < padded_rows:
        recipients.append(0)
    for donor in values:
        for recipient in recipients:
            if donor < recipient + 2:
                continue
            changed = list(state)
            changed.remove(donor)
            if donor > 1:
                changed.append(donor - 1)
            if recipient > 0:
                changed.remove(recipient)
            changed.append(recipient + 1)
            target = tuple(sorted(changed, reverse=True))
            yield donor, recipient, target


def dominance_area(profile: State, state: State, padded_rows: int) -> int:
    """Return the integral area between the Lorenz curves of profile and state."""
    if len(profile) > padded_rows or len(state) > padded_rows:
        raise ValueError("state has more rows than its padding")
    padded_profile = profile + (0,) * (padded_rows - len(profile))
    padded_state = state + (0,) * (padded_rows - len(state))
    if sum(padded_profile) != sum(padded_state):
        raise ValueError("dominance area requires equal total mass")
    return sum(
        index * (value - base)
        for index, (base, value) in enumerate(zip(padded_profile, padded_state))
    )


def enumerate_area_ideal(profile: State, padded_rows: int, maximum_area: int) -> list[State]:
    """Enumerate the normalized states below profile of area at most maximum_area."""
    if maximum_area < 0:
        raise ValueError("maximum area must be nonnegative")
    pending: list[tuple[int, State]] = [(0, profile)]
    area_by_state = {profile: 0}
    while pending:
        area, state = heapq.heappop(pending)
        if area_by_state[state] != area:
            continue
        for _, _, target in transfers(state, padded_rows):
            target_area = dominance_area(profile, target, padded_rows)
            if target_area <= area:
                raise AssertionError(
                    f"Robin--Hood transfer did not increase area: {state} -> {target}"
                )
            if target_area > maximum_area or target in area_by_state:
                continue
            area_by_state[target] = target_area
            heapq.heappush(pending, (target_area, target))
    return sorted(area_by_state, key=lambda state: (area_by_state[state], state))


def greedy_pascal_orbits(k: int) -> dict[tuple[GreedyBlock, ...], str]:
    """Return all self-sorted greedy A/B shuffles, with one representative word.

    For h=G_(k-1), position i of a greedy polymatroid base receives mixed
    part h_i and pure part h_r, where r is that color's occurrence number.
    A source coloring must have these sums in nonincreasing order.  Equal
    sums are collapsed to their total and A multiplicities, which is exactly
    the coloring-orbit quotient.  The unique largest row is fixed in A to
    quotient global complementation.
    """
    if k < 1:
        raise ValueError("level must be positive")
    child = singleton_base(k - 1)
    rows = len(child)
    partial: dict[tuple[int, tuple[GreedyBlock, ...]], str] = {(0, ()): ""}
    for index in range(2 * rows):
        following: dict[tuple[int, tuple[GreedyBlock, ...]], str] = {}
        for (a_used, blocks), word in partial.items():
            b_used = index - a_used
            for in_a in (True, False):
                if index == 0 and not in_a:
                    continue
                used = a_used if in_a else b_used
                if used >= rows:
                    continue
                value = (child[index] if index < rows else 0) + child[used]
                if blocks and value > blocks[-1][0]:
                    continue
                a_next = a_used + int(in_a)
                if blocks and value == blocks[-1][0]:
                    old_value, old_total, old_a = blocks[-1]
                    next_blocks = blocks[:-1] + (
                        (old_value, old_total + 1, old_a + int(in_a)),
                    )
                else:
                    next_blocks = blocks + ((value, 1, int(in_a)),)
                key = (a_next, next_blocks)
                following.setdefault(key, word + ("A" if in_a else "B"))
        partial = following
    result = {
        blocks: word
        for (a_used, blocks), word in partial.items()
        if a_used == rows
    }
    if len(result) != len(partial):
        raise AssertionError("greedy shuffle did not finish with balanced colors")
    return result


def greedy_block_data(
    blocks: tuple[GreedyBlock, ...], largest: int
) -> tuple[State, Coloring]:
    state = tuple(value for value, total, _ in blocks for _ in range(total))
    a_counts = [0] * (largest + 1)
    for value, _, a_total in blocks:
        a_counts[value] = a_total
    totals = multiplicities(state, largest)
    return state, canonical(tuple(a_counts), totals)


def weakly_majorized(state: State, profile: State) -> bool:
    used = 0
    bound = 0
    for index, value in enumerate(state):
        used += value
        if index < len(profile):
            bound += profile[index]
        if used > bound:
            return False
    return sum(state) == sum(profile)


def coloring_predecessor(
    k: int, state: State, coloring: Coloring
) -> tuple[int, int, State, Coloring] | None:
    """Find an exact color-preserving headward unit move, if one exists."""
    profile = singleton_base(k)
    largest = profile[0]
    totals = multiplicities(state, largest)
    hall = Hall(singleton_base(k - 1))
    values = sorted(set(state), reverse=True)

    for head in values:
        for tail in values:
            if head < tail:
                continue
            for head_in_a in (True, False):
                head_count = coloring[head] if head_in_a else totals[head] - coloring[head]
                if not head_count:
                    continue
                for tail_in_a in (True, False):
                    tail_count = coloring[tail] if tail_in_a else totals[tail] - coloring[tail]
                    if not tail_count:
                        continue
                    if (head, head_in_a) == (tail, tail_in_a) and head_count < 2:
                        continue
                    if head + 1 > largest:
                        continue

                    predecessor = list(state)
                    predecessor.remove(head)
                    predecessor.remove(tail)
                    predecessor.append(head + 1)
                    if tail > 1:
                        predecessor.append(tail - 1)
                    predecessor_state = tuple(sorted(predecessor, reverse=True))
                    if not weakly_majorized(predecessor_state, profile):
                        continue

                    predecessor_a = list(coloring)
                    if head_in_a:
                        predecessor_a[head] -= 1
                        predecessor_a[head + 1] += 1
                    if tail_in_a:
                        predecessor_a[tail] -= 1
                        if tail > 1:
                            predecessor_a[tail - 1] += 1
                    predecessor_totals = multiplicities(predecessor_state, largest)
                    predecessor_key = canonical(
                        tuple(predecessor_a), predecessor_totals
                    )
                    if hall.feasible(predecessor_key, predecessor_totals):
                        return head, tail, predecessor_state, predecessor_key
    return None


@dataclass(frozen=True)
class FiberRelation:
    pairs: frozenset[tuple[Coloring, Coloring]]
    same_pairs: frozenset[tuple[Coloring, Coloring]]

    def forward(self) -> dict[Coloring, frozenset[Coloring]]:
        result: dict[Coloring, set[Coloring]] = defaultdict(set)
        for source, target in self.pairs:
            result[source].add(target)
        return {key: frozenset(value) for key, value in result.items()}


def edge_relation(
    state: State,
    target: State,
    donor: int,
    recipient: int,
    largest: int,
    source_fiber: frozenset[Coloring],
    target_fiber: frozenset[Coloring],
) -> FiberRelation:
    source_totals = multiplicities(state, largest)
    target_totals = multiplicities(target, largest)
    pairs: set[tuple[Coloring, Coloring]] = set()
    same_pairs: set[tuple[Coloring, Coloring]] = set()

    for source_key in source_fiber:
        orientations = {source_key, complement(source_key, source_totals)}
        for oriented in orientations:
            donor_sides = []
            if oriented[donor] > 0:
                donor_sides.append(True)
            if source_totals[donor] - oriented[donor] > 0:
                donor_sides.append(False)

            if recipient == 0:
                recipient_sides = [True, False]
            else:
                recipient_sides = []
                if oriented[recipient] > 0:
                    recipient_sides.append(True)
                if source_totals[recipient] - oriented[recipient] > 0:
                    recipient_sides.append(False)

            for donor_in_a in donor_sides:
                for recipient_in_a in recipient_sides:
                    moved = list(oriented)
                    if donor_in_a:
                        moved[donor] -= 1
                        if donor > 1:
                            moved[donor - 1] += 1
                    if recipient_in_a:
                        if recipient > 0:
                            moved[recipient] -= 1
                        moved[recipient + 1] += 1
                    target_key = canonical(tuple(moved), target_totals)
                    if target_key not in target_fiber:
                        continue
                    pair = (source_key, target_key)
                    pairs.add(pair)
                    if donor_in_a == recipient_in_a:
                        same_pairs.add(pair)

    return FiberRelation(frozenset(pairs), frozenset(same_pairs))


@dataclass(frozen=True)
class Edge:
    source: State
    donor: int
    recipient: int
    target: State
    relation: FiberRelation


@dataclass(frozen=True)
class UndirectedSummary:
    components: int
    largest_component: int
    canonical_vertices: int
    canonical_states: int
    source_components: int
    internal_links: int


def normalized_self_exchange_pairs(
    state: State,
    fiber: frozenset[Coloring],
) -> frozenset[tuple[Coloring, Coloring]]:
    """Return color changes induced by a unit move with unchanged row multiset.

    A move d -> d-1 swaps the two row widths after sorting.  The special move
    1 -> 0 transfers the unit to a padded zero slot and can likewise change
    the positive row's color.  The directed parent DAG omits both as loops,
    but the labelled bidirectional exchange graph must retain them.
    """
    largest = len(next(iter(fiber))) - 1
    totals = multiplicities(state, largest)
    pairs: set[tuple[Coloring, Coloring]] = set()
    for donor in set(state):
        recipient = donor - 1
        if recipient > 0 and not totals[recipient]:
            continue
        if recipient == 0 and len(state) == sum(state):
            continue
        relation = edge_relation(
            state,
            state,
            donor,
            recipient,
            largest,
            fiber,
            fiber,
        )
        pairs.update(relation.pairs)
    return frozenset(pairs)


def undirected_summary(
    states: list[State],
    fibers: dict[State, frozenset[Coloring]],
    edges: list[Edge],
    canonical_vertices: Iterable[tuple[State, Coloring]],
    source_vertices: Iterable[tuple[State, Coloring]],
) -> UndirectedSummary:
    """Summarize the graph obtained by allowing every legal transfer backwards.

    Vertices are feasible normalized parent/coloring pairs.  A relation pair on
    a downward Robin--Hood edge becomes one undirected edge; hence walking it in
    reverse is precisely a legal headward transfer with both endpoint cuts
    retained.  Adjacent-width and unit-to-zero exchanges that preserve the
    normalized parent are included as internal fiber links.
    """
    vertices = [
        (state, coloring)
        for state in states
        for coloring in fibers[state]
    ]
    index = {vertex: number for number, vertex in enumerate(vertices)}
    parent = list(range(len(vertices)))
    size = [1] * len(vertices)

    def find(number: int) -> int:
        while parent[number] != number:
            parent[number] = parent[parent[number]]
            number = parent[number]
        return number

    def union(left: int, right: int) -> None:
        left_root = find(left)
        right_root = find(right)
        if left_root == right_root:
            return
        if size[left_root] < size[right_root]:
            left_root, right_root = right_root, left_root
        parent[right_root] = left_root
        size[left_root] += size[right_root]

    for edge in edges:
        for source_coloring, target_coloring in edge.relation.pairs:
            union(
                index[(edge.source, source_coloring)],
                index[(edge.target, target_coloring)],
            )

    internal_links: set[
        tuple[tuple[State, Coloring], tuple[State, Coloring]]
    ] = set()
    for state in states:
        for source_coloring, target_coloring in normalized_self_exchange_pairs(
            state, fibers[state]
        ):
            if source_coloring == target_coloring:
                continue
            source = (state, source_coloring)
            target = (state, target_coloring)
            internal_links.add(tuple(sorted((source, target))))
            union(index[source], index[target])

    component_sizes = Counter(find(number) for number in range(len(vertices)))
    canonical_roots = {
        find(index[vertex])
        for vertex in canonical_vertices
    }
    source_roots = {
        find(index[vertex])
        for vertex in source_vertices
    }
    canonical_vertex_count = sum(
        count for root, count in component_sizes.items() if root in canonical_roots
    )
    canonical_state_count = sum(
        any(
            find(index[(state, coloring)]) in canonical_roots
            for coloring in fibers[state]
        )
        for state in states
    )
    return UndirectedSummary(
        components=len(component_sizes),
        largest_component=max(component_sizes.values()),
        canonical_vertices=canonical_vertex_count,
        canonical_states=canonical_state_count,
        source_components=len(source_roots),
        internal_links=len(internal_links),
    )


def phase_score(edge: Edge, fibers: dict[State, frozenset[Coloring]]) -> tuple[float, int]:
    source_count = len(fibers[edge.source])
    target_count = len(fibers[edge.target])
    source_seen = {source for source, _ in edge.relation.pairs}
    target_seen = {target for _, target in edge.relation.pairs}
    churn = (source_count - len(source_seen)) + (target_count - len(target_seen))
    return churn / (source_count + target_count), churn


def summarize_edge(edge: Edge, fibers: dict[State, frozenset[Coloring]]) -> str:
    source_seen = {source for source, _ in edge.relation.pairs}
    target_seen = {target for _, target in edge.relation.pairs}
    return (
        f"source={show(edge.source)} move={edge.donor}->{edge.recipient} "
        f"target={show(edge.target)} fibers={len(fibers[edge.source])}->{len(fibers[edge.target])} "
        f"links={len(edge.relation.pairs)} persistent={len(source_seen)} "
        f"inherited={len(target_seen)} deaths={len(fibers[edge.source]) - len(source_seen)} "
        f"births={len(fibers[edge.target]) - len(target_seen)} "
        f"same_links={len(edge.relation.same_pairs)}"
    )


def canonical_rows(k: int) -> list[tuple[int, int, int]]:
    child = singleton_base(k - 1)
    rows: list[tuple[int, int, int]] = []
    for index in range(2 * len(child)):
        pure = child[index // 2]
        mixed = child[index] if index < len(child) else 0
        rows.append((pure if index % 2 == 0 else 0,
                     mixed,
                     pure if index % 2 == 1 else 0))
    return rows


def phase_cut(k: int) -> tuple[State, list[tuple[int, int, int]]]:
    """The four-row Pascal reassociation born at the canonical head edge."""
    if k < 3:
        raise ValueError("the distinct-row phase cut starts at k=3")
    child = singleton_base(k - 1)
    rows = canonical_rows(k)
    u = child[0]
    rows[0] = (0, u, u)
    rows[1] = (0, child[1], u - 1)
    rows[2] = (u, child[2], 0)
    rows[3] = (child[1], child[3], 0)
    parent = tuple(sorted((sum(row) for row in rows), reverse=True))
    children = [
        tuple(sorted((row[column] for row in rows if row[column]), reverse=True))
        for column in range(3)
    ]
    if any(sequence != child for sequence in children):
        raise AssertionError("Pascal phase cut does not reproduce three canonical children")
    if any(left and right for left, _, right in rows):
        raise AssertionError("Pascal phase cut violates row legality")
    return parent, rows


def inspect_phase_edge(k: int) -> None:
    if not 3 <= k <= 5:
        raise ValueError("exact phase-edge fiber enumeration is supported for 3 <= k <= 5")
    source = singleton_base(k)
    donor = source[1]
    recipient = source[2]
    changed = list(source)
    changed.remove(donor)
    changed.append(donor - 1)
    changed.remove(recipient)
    changed.append(recipient + 1)
    target = tuple(sorted(changed, reverse=True))
    explicit_target, rows = phase_cut(k)
    if target != explicit_target:
        raise AssertionError("explicit Pascal phase cut has the wrong parent")

    hall = Hall(singleton_base(k - 1))
    source_fiber = feasible_fiber(source, source[0], hall)
    target_fiber = feasible_fiber(target, source[0], hall)
    relation = edge_relation(
        source,
        target,
        donor,
        recipient,
        source[0],
        source_fiber,
        target_fiber,
    )
    inherited = {coloring for _, coloring in relation.pairs}
    novel = set(target_fiber) - inherited
    print(
        f"PASCAL_PHASE_EDGE k={k} source={show(source)} move={donor}->{recipient} "
        f"target={show(target)} source_fiber={len(source_fiber)} "
        f"target_fiber={len(target_fiber)} links={len(relation.pairs)} "
        f"inherited={len(inherited)} novel={len(novel)}"
    )
    print("PHASE_CUT " + ",".join(f"({l},{m},{r})" for l, m, r in rows))
    for coloring in sorted(novel):
        print("NOVEL_COLORING " + show_coloring(target, coloring, source[0]))


def inspect_greedy_sources(k: int, examples: int) -> None:
    """Classify every possible coloring-source orbit via polymatroid greediness."""
    profile = singleton_base(k)
    largest = profile[0]
    padded_rows = 3**k
    hall = Hall(singleton_base(k - 1))
    records = []
    for blocks, word in greedy_pascal_orbits(k).items():
        state, coloring = greedy_block_data(blocks, largest)
        totals = multiplicities(state, largest)
        if not hall.feasible(coloring, totals):
            raise AssertionError(f"greedy coloring is infeasible: {state}")
        predecessor = coloring_predecessor(k, state, coloring)
        records.append(
            (
                dominance_area(profile, state, padded_rows),
                state,
                coloring,
                word,
                predecessor,
            )
        )
    records.sort(key=lambda record: record[:4])
    sources = [record for record in records if record[4] is None]
    print(
        f"PASCAL_GREEDY_SOURCE_SURVEY k={k} "
        f"greedy_orbits={len(records)} source_orbits={len(sources)} "
        f"inherited_greedy_orbits={len(records) - len(sources)}"
    )
    print("SOURCE_AREA_HISTOGRAM " + " ".join(
        f"{area}:{count}"
        for area, count in sorted(Counter(record[0] for record in sources).items())
    ))
    print("GREEDY_SOURCES")
    for area, state, coloring, word, _ in sources[:examples]:
        print(
            f"  area={area} state={show(state)} word={word} "
            + show_coloring(state, coloring, largest)
        )
    if len(sources) > examples:
        print(f"  ... {len(sources) - examples} further source orbits omitted")

    inherited = [record for record in records if record[4] is not None]
    if inherited and examples:
        print("INHERITED_GREEDY_ORBITS")
        for area, state, coloring, word, predecessor in inherited[:examples]:
            if predecessor is None:
                raise AssertionError("missing predecessor record")
            head, tail, predecessor_state, _ = predecessor
            print(
                f"  area={area} state={show(state)} word={word} "
                f"reverse={head},{tail} predecessor={show(predecessor_state)} "
                + show_coloring(state, coloring, largest)
            )
        if len(inherited) > examples:
            print(f"  ... {len(inherited) - examples} further inherited orbits omitted")


def run_area_ideal(k: int, maximum_area: int, examples: int) -> None:
    """Survey an exact downward-closed initial ideal of the solution-fiber DAG."""
    profile = singleton_base(k)
    child = singleton_base(k - 1)
    largest = profile[0]
    padded_rows = 3**k
    states = enumerate_area_ideal(profile, padded_rows, maximum_area)
    state_set = set(states)
    area = {
        state: dominance_area(profile, state, padded_rows)
        for state in states
    }

    # For the levels whose full state corpus is inexpensive, independently
    # check that transfer generation really produced the entire area ideal.
    if k <= 3:
        expected = {
            state
            for state in enumerate_states(profile)
            if dominance_area(profile, state, padded_rows) <= maximum_area
        }
        if state_set != expected:
            raise AssertionError(
                f"area-ideal enumeration mismatch: generated={len(state_set)} "
                f"expected={len(expected)}"
            )

    hall = Hall(child)
    fibers: dict[State, frozenset[Coloring]] = {}
    for state in states:
        fibers[state] = feasible_fiber(state, largest, hall)
        if not fibers[state]:
            raise AssertionError(f"row-coloring counterexample {state}")

    edges: list[Edge] = []
    incoming: dict[State, list[Edge]] = defaultdict(list)
    outgoing: dict[State, list[Edge]] = defaultdict(list)
    for state in states:
        for donor, recipient, target in transfers(state, padded_rows):
            if target not in state_set:
                continue
            relation = edge_relation(
                state,
                target,
                donor,
                recipient,
                largest,
                fibers[state],
                fibers[target],
            )
            edge = Edge(state, donor, recipient, target, relation)
            edges.append(edge)
            incoming[target].append(edge)
            outgoing[state].append(edge)

    inherited_by_any: dict[State, set[Coloring]] = defaultdict(set)
    empty_edges = []
    for edge in edges:
        if not edge.relation.pairs:
            empty_edges.append(edge)
        inherited_by_any[edge.target].update(target for _, target in edge.relation.pairs)

    sources: list[tuple[State, Coloring]] = [
        (profile, coloring) for coloring in fibers[profile]
    ]
    for state in states:
        if state == profile:
            continue
        sources.extend(
            (state, coloring)
            for coloring in set(fibers[state]) - inherited_by_any[state]
        )

    reached: dict[State, set[Coloring]] = defaultdict(set)
    reached[profile].update(fibers[profile])
    for state in states:
        active = reached[state]
        if not active:
            continue
        for edge in outgoing[state]:
            forward = edge.relation.forward()
            for coloring in active:
                reached[edge.target].update(forward.get(coloring, ()))

    # Dominance area grades the normalized partition order.  Retain only its
    # cover edges to test whether long one-coin jumps are actually needed for
    # inheritance or forward reachability.
    cover_edges = [
        edge for edge in edges if area[edge.target] == area[edge.source] + 1
    ]
    cover_incoming: dict[State, list[Edge]] = defaultdict(list)
    cover_outgoing: dict[State, list[Edge]] = defaultdict(list)
    for edge in cover_edges:
        cover_incoming[edge.target].append(edge)
        cover_outgoing[edge.source].append(edge)
    cover_source_orbits: list[tuple[State, Coloring]] = [
        (profile, coloring) for coloring in fibers[profile]
    ]
    for state in states:
        if state == profile:
            continue
        cover_inherited = {
            target
            for edge in cover_incoming[state]
            for _, target in edge.relation.pairs
        }
        cover_source_orbits.extend(
            (state, coloring)
            for coloring in set(fibers[state]) - cover_inherited
        )
    cover_reached: dict[State, set[Coloring]] = defaultdict(set)
    cover_reached[profile].update(fibers[profile])
    for state in states:
        for edge in cover_outgoing[state]:
            forward = edge.relation.forward()
            for coloring in cover_reached[state]:
                cover_reached[edge.target].update(forward.get(coloring, ()))

    state_histogram = Counter(area.values())
    fiber_histogram: Counter[int] = Counter()
    source_histogram: Counter[int] = Counter()
    for state in states:
        fiber_histogram[area[state]] += len(fibers[state])
    for state, _ in sources:
        source_histogram[area[state]] += 1

    undirected = undirected_summary(
        states,
        fibers,
        edges,
        ((profile, coloring) for coloring in fibers[profile]),
        sources,
    )

    print(
        f"SOLUTION_FIBER_AREA_IDEAL k={k} maximum_area={maximum_area} "
        f"states={len(states)} fibers={sum(map(len, fibers.values()))} "
        f"transfers={len(edges)} links={sum(len(edge.relation.pairs) for edge in edges)}"
    )
    print(
        f"EDGE_SUMMARY nonempty={len(edges) - len(empty_edges)}/{len(edges)} "
        f"sources={len(sources)} nonroot_sources={len(sources) - len(fibers[profile])}"
    )
    print(
        f"CANONICAL_COMPONENT reached_states={sum(bool(reached[state]) for state in states)}/"
        f"{len(states)} reached_fibers={sum(len(reached[state]) for state in states)}/"
        f"{sum(map(len, fibers.values()))}"
    )
    print(
        f"UNDIRECTED_GRAPH components={undirected.components} "
        f"largest={undirected.largest_component} "
        f"canonical_states={undirected.canonical_states}/{len(states)} "
        f"canonical_fibers={undirected.canonical_vertices}/"
        f"{sum(map(len, fibers.values()))} "
        f"source_components={undirected.source_components} "
        f"internal_links={undirected.internal_links}"
    )
    print(
        f"COVER_SUBDAG edges={len(cover_edges)} "
        f"nonempty={sum(bool(edge.relation.pairs) for edge in cover_edges)}/"
        f"{len(cover_edges)} sources={len(cover_source_orbits)} "
        f"nonroot_sources={len(cover_source_orbits) - len(fibers[profile])} "
        f"reached_states={sum(bool(cover_reached[state]) for state in states)}/"
        f"{len(states)} reached_fibers="
        f"{sum(len(cover_reached[state]) for state in states)}/"
        f"{sum(map(len, fibers.values()))}"
    )
    print("AREA_HISTOGRAM")
    for value in sorted(state_histogram):
        reached_states = sum(bool(reached[state]) for state in states if area[state] == value)
        reached_fibers = sum(len(reached[state]) for state in states if area[state] == value)
        print(
            f"  area={value} states={state_histogram[value]} fibers={fiber_histogram[value]} "
            f"sources={source_histogram[value]} reached_states={reached_states} "
            f"reached_fibers={reached_fibers}"
        )

    print("SOURCE_ORBITS")
    for state, coloring in sorted(sources, key=lambda item: (area[item[0]], item))[:examples]:
        print(
            f"  area={area[state]} state={show(state)} fiber={len(fibers[state])} "
            + show_coloring(state, coloring, largest)
        )
    if len(sources) > examples:
        print(f"  ... {len(sources) - examples} further source orbits omitted")

    cover_missed_states = [state for state in states if not cover_reached[state]]
    if cover_missed_states:
        print("COVER_MISSED_STATES")
        for state in cover_missed_states[:examples]:
            print(
                f"  area={area[state]} state={show(state)} fiber={len(fibers[state])}"
            )
        if len(cover_missed_states) > examples:
            print(
                f"  ... {len(cover_missed_states) - examples} further states omitted"
            )

    if empty_edges:
        print("EMPTY_EDGES")
        for edge in empty_edges[:examples]:
            print("  " + summarize_edge(edge, fibers))
        if len(empty_edges) > examples:
            print(f"  ... {len(empty_edges) - examples} further empty edges omitted")

    missed_states = [state for state in states if not reached[state]]
    if missed_states:
        print("CANONICALLY_MISSED_STATES")
        for state in missed_states[:examples]:
            print(
                f"  area={area[state]} state={show(state)} fiber={len(fibers[state])}"
            )
        if len(missed_states) > examples:
            print(f"  ... {len(missed_states) - examples} further states omitted")


def run(k: int, examples: int) -> None:
    profile = singleton_base(k)
    child = singleton_base(k - 1)
    largest = profile[0]
    padded_rows = 3**k
    states = enumerate_states(profile)
    state_set = set(states)
    hall = Hall(child)
    fibers = {state: feasible_fiber(state, largest, hall) for state in states}

    if any(not fiber for fiber in fibers.values()):
        missing = next(state for state, fiber in fibers.items() if not fiber)
        raise AssertionError(f"row-coloring counterexample {missing}")

    edges: list[Edge] = []
    incoming: dict[State, list[Edge]] = defaultdict(list)
    outgoing: dict[State, list[Edge]] = defaultdict(list)
    for state in states:
        for donor, recipient, target in transfers(state, padded_rows):
            if target not in state_set:
                raise AssertionError(f"transfer left state corpus: {state} -> {target}")
            relation = edge_relation(
                state,
                target,
                donor,
                recipient,
                largest,
                fibers[state],
                fibers[target],
            )
            edge = Edge(state, donor, recipient, target, relation)
            edges.append(edge)
            incoming[target].append(edge)
            outgoing[state].append(edge)

    expected = {1: 1, 2: 33, 3: 8916}
    if k in expected and len(edges) != expected[k]:
        raise AssertionError(f"transfer regression: {len(edges)} != {expected[k]}")
    empty = [edge for edge in edges if not edge.relation.pairs]
    if empty:
        raise AssertionError("adjacent-fiber counterexample: " + summarize_edge(empty[0], fibers))

    fiber_histogram = Counter(map(len, fibers.values()))
    max_state = max(states, key=lambda state: (len(fibers[state]), state))
    source_total = target_total = bijective = exact_relation = 0
    no_birth = no_death = no_churn = no_same = 0
    edge_classes: Counter[tuple[int, int]] = Counter()
    for edge in edges:
        source_seen = {source for source, _ in edge.relation.pairs}
        target_seen = {target for _, target in edge.relation.pairs}
        deaths = len(fibers[edge.source]) - len(source_seen)
        births = len(fibers[edge.target]) - len(target_seen)
        edge_classes[(deaths, births)] += 1
        source_total += len(fibers[edge.source])
        target_total += len(fibers[edge.target])
        no_birth += births == 0
        no_death += deaths == 0
        no_churn += deaths == 0 and births == 0
        no_same += not edge.relation.same_pairs
        forward = Counter(source for source, _ in edge.relation.pairs)
        backward = Counter(target for _, target in edge.relation.pairs)
        is_bijection = (
            deaths == 0
            and births == 0
            and all(degree == 1 for degree in forward.values())
            and all(degree == 1 for degree in backward.values())
        )
        bijective += is_bijection
        exact_relation += (
            is_bijection
            and all(source == target for source, target in edge.relation.pairs)
        )

    inherited_by_any: dict[State, set[Coloring]] = defaultdict(set)
    for state, state_edges in incoming.items():
        for edge in state_edges:
            inherited_by_any[state].update(target for _, target in edge.relation.pairs)
    source_orbits = [(profile, coloring) for coloring in fibers[profile]]
    novel_states = []
    novel_orbits = 0
    for state in states:
        if state == profile:
            continue
        novel = set(fibers[state]) - inherited_by_any[state]
        if novel:
            novel_states.append((len(novel), state, len(fibers[state])))
            novel_orbits += len(novel)
            source_orbits.extend((state, coloring) for coloring in novel)

    ordered = sorted(states, key=lambda state: (sum(v * v for v in state), state), reverse=True)
    if ordered[0] != profile:
        raise AssertionError("G_k is not first in transfer topological order")

    def propagate(starts: set[Coloring]) -> tuple[dict[State, set[Coloring]], int]:
        reached: dict[State, set[Coloring]] = defaultdict(set)
        reached[profile].update(starts)
        for state in ordered:
            active = reached[state]
            if not active:
                continue
            for edge in outgoing[state]:
                forward = edge.relation.forward()
                for source in active:
                    reached[edge.target].update(forward.get(source, ()))
        reached_states = sum(bool(reached[state]) for state in states)
        return reached, reached_states

    all_reached, all_reached_states = propagate(set(fibers[profile]))
    start_results = []
    for start in sorted(fibers[profile]):
        reached, reached_states = propagate({start})
        reached_pairs = sum(len(reached[state]) for state in states)
        start_results.append((reached_states, reached_pairs, start))

    undirected = undirected_summary(
        states,
        fibers,
        edges,
        ((profile, coloring) for coloring in fibers[profile]),
        source_orbits,
    )
    expected_undirected = {1: (1, 0), 2: (1, 34), 3: (1, 54211)}
    if k in expected_undirected:
        observed = (undirected.components, undirected.internal_links)
        if observed != expected_undirected[k]:
            raise AssertionError(
                f"undirected regression: {observed} != {expected_undirected[k]}"
            )

    print(f"SOLUTION_FIBER_DAG k={k} states={len(states)} transfers={len(edges)}")
    print(
        f"FIBERS total_orbits={sum(map(len, fibers.values()))} "
        f"minimum={min(fiber_histogram)} maximum={len(fibers[max_state])} "
        f"maximum_state={show(max_state)} G={len(fibers[profile])} "
        f"all_units={len(fibers[(1,) * padded_rows])}"
    )
    print("FIBER_HISTOGRAM " + " ".join(f"{size}:{count}" for size, count in sorted(fiber_histogram.items())))
    print(
        f"EDGE_SUMMARY nonempty={len(edges) - len(empty)} no_same={no_same} "
        f"no_birth={no_birth} no_death={no_death} no_churn={no_churn} "
        f"bijections={bijective} identity_bijections={exact_relation}"
    )
    print(
        f"EDGE_INCIDENCE source_fiber_occurrences={source_total} "
        f"target_fiber_occurrences={target_total} "
        f"transport_links={sum(len(edge.relation.pairs) for edge in edges)}"
    )
    print(
        f"INCOMING_UNION states_with_novel_orbits={len(novel_states)} "
        f"novel_orbits={novel_orbits}"
    )
    print(
        f"FORWARD_ALL_STARTS reached_states={all_reached_states}/{len(states)} "
        f"reached_fiber_orbits={sum(len(all_reached[state]) for state in states)}/"
        f"{sum(map(len, fibers.values()))}"
    )
    print(
        f"UNDIRECTED_GRAPH components={undirected.components} "
        f"largest={undirected.largest_component} "
        f"canonical_states={undirected.canonical_states}/{len(states)} "
        f"canonical_fibers={undirected.canonical_vertices}/"
        f"{sum(map(len, fibers.values()))} "
        f"source_components={undirected.source_components} "
        f"internal_links={undirected.internal_links}"
    )
    unreachable = [
        (len(set(fibers[state]) - all_reached[state]), state)
        for state in states
        if set(fibers[state]) - all_reached[state]
    ]
    print(
        f"FORWARD_UNREACHABLE states={len(unreachable)} "
        f"fiber_orbits={sum(count for count, _ in unreachable)}"
    )
    novel_reached: dict[State, set[Coloring]] = defaultdict(set)
    for state in states:
        if state != profile:
            novel_reached[state].update(set(fibers[state]) - inherited_by_any[state])
    for state in ordered:
        active = novel_reached[state]
        if not active:
            continue
        for edge in outgoing[state]:
            forward = edge.relation.forward()
            for source in active:
                novel_reached[edge.target].update(forward.get(source, ()))
    novel_descendants = sum(len(novel_reached[state]) for state in states)
    unreachable_set = {
        (state, coloring)
        for state in states
        for coloring in set(fibers[state]) - all_reached[state]
    }
    novel_set = {
        (state, coloring)
        for state in states
        for coloring in novel_reached[state]
    }
    print(
        f"NOVEL_COMPONENT descendants={novel_descendants} "
        f"unreachable_covered={len(unreachable_set & novel_set)}/{len(unreachable_set)} "
        f"overlap_with_canonical={len(novel_set) - len(unreachable_set & novel_set)}"
    )
    print("START_FIBER_RESULTS")
    for reached_states, reached_pairs, start in sorted(start_results, reverse=True):
        print(
            f"  start={start} reached_states={reached_states}/{len(states)} "
            f"reached_fiber_orbits={reached_pairs}/{sum(map(len, fibers.values()))}"
        )

    print("MOST_CHURNING_EDGES")
    ranked_edges = sorted(
        edges,
        key=lambda edge: (phase_score(edge, fibers), summarize_edge(edge, fibers)),
        reverse=True,
    )
    for edge in ranked_edges[:examples]:
        print("  " + summarize_edge(edge, fibers))

    print("MOST_DESTRUCTIVE_EDGES")
    death_edges = []
    for edge in edges:
        source_seen = {source for source, _ in edge.relation.pairs}
        deaths = len(fibers[edge.source]) - len(source_seen)
        if deaths:
            death_edges.append((deaths / len(fibers[edge.source]), deaths, edge))
    for _, _, edge in sorted(death_edges, reverse=True, key=lambda item: (item[0], item[1]))[:examples]:
        print("  " + summarize_edge(edge, fibers))

    print("MOST_NOVEL_STATES")
    for novel, state, size in sorted(novel_states, reverse=True)[:examples]:
        print(
            f"  state={show(state)} fiber={size} inherited={size - novel} novel={novel} "
            f"predecessor_edges={len(incoming[state])}"
        )
        missing = set(fibers[state]) - inherited_by_any[state]
        for coloring in sorted(missing):
            print("    novel " + show_coloring(state, coloring, largest))
        for edge in incoming[state]:
            print("    predecessor " + summarize_edge(edge, fibers))

    print("MOST_FORWARD_UNREACHABLE_STATES")
    for count, state in sorted(unreachable, reverse=True)[:examples]:
        print(
            f"  state={show(state)} fiber={len(fibers[state])} "
            f"reached={len(all_reached[state])} unreachable={count}"
        )

    death_histogram: Counter[int] = Counter()
    birth_histogram: Counter[int] = Counter()
    for (deaths, births), count in edge_classes.items():
        death_histogram[deaths] += count
        birth_histogram[births] += count
    print("DEATH_HISTOGRAM " + " ".join(f"{count}:{edges}" for count, edges in sorted(death_histogram.items())))
    print("BIRTH_HISTOGRAM " + " ".join(f"{count}:{edges}" for count, edges in sorted(birth_histogram.items())))

    print("SMALL_EDGE_CLASSES")
    for (deaths, births), count in sorted(edge_classes.items(), key=lambda item: (sum(item[0]), item[0]))[:20]:
        print(f"  deaths={deaths} births={births} edges={count}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("k", nargs="?", type=int, default=3)
    parser.add_argument("--examples", type=int, default=12)
    parser.add_argument("--phase-edge", type=int)
    parser.add_argument("--area-ideal", nargs=2, type=int, metavar=("K", "AREA"))
    parser.add_argument("--greedy-sources", type=int, metavar="K")
    parser.add_argument(
        "--rational-grid", nargs=2, type=int, metavar=("K", "MAX_DENOMINATOR")
    )
    parser.add_argument(
        "--exact-rational-grid",
        nargs=2,
        type=int,
        metavar=("K", "MAX_DENOMINATOR"),
    )
    parser.add_argument(
        "--list-rational-holes",
        action="store_true",
        help="print every missing rational-grid state and its scaled Hall defect",
    )
    args = parser.parse_args()
    if args.phase_edge is not None:
        inspect_phase_edge(args.phase_edge)
        return
    if args.greedy_sources is not None:
        if not 2 <= args.greedy_sources <= 8:
            parser.error("greedy-source enumeration supports 2 <= K <= 8")
        inspect_greedy_sources(args.greedy_sources, args.examples)
        return
    if args.rational_grid is not None:
        k, maximum_denominator = args.rational_grid
        if not 1 <= k <= 2:
            parser.error("padded rational-grid enumeration supports 1 <= K <= 2")
        if maximum_denominator < 1:
            parser.error("the maximum denominator must be positive")
        inspect_rational_grid(k, maximum_denominator, False, args.list_rational_holes)
        return
    if args.exact_rational_grid is not None:
        k, maximum_denominator = args.exact_rational_grid
        if not 1 <= k <= 3:
            parser.error("exact-support rational-grid enumeration supports 1 <= K <= 3")
        if maximum_denominator < 1:
            parser.error("the maximum denominator must be positive")
        inspect_rational_grid(k, maximum_denominator, True, args.list_rational_holes)
        return
    if args.area_ideal is not None:
        k, maximum_area = args.area_ideal
        if not 1 <= k <= 5:
            parser.error("area-ideal fiber enumeration supports 1 <= K <= 5")
        run_area_ideal(k, maximum_area, args.examples)
        return
    if not 1 <= args.k <= 3:
        parser.error("the exact implementation currently supports 1 <= k <= 3")
    run(args.k, args.examples)


if __name__ == "__main__":
    main()
