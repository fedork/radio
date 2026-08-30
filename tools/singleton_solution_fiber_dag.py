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
the G_k fiber.
"""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
from dataclasses import dataclass
from itertools import product
from typing import Iterable


State = tuple[int, ...]
Coloring = tuple[int, ...]  # A-row multiplicities, indexed by row value.


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


def enumerate_states(profile: State) -> list[State]:
    total = sum(profile)
    maximum = profile[0]
    prefix = [0]
    for value in profile:
        prefix.append(prefix[-1] + value)

    states: list[State] = []

    def visit(remaining: int, previous: int, used: int, parts: list[int]) -> None:
        if remaining == 0:
            states.append(tuple(parts))
            return
        for value in range(min(previous, remaining, maximum), 0, -1):
            next_used = used + value
            count = len(parts) + 1
            bound = prefix[min(count, len(profile))]
            if next_used > bound:
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
    novel_states = []
    novel_orbits = 0
    for state in states:
        if state == profile:
            continue
        novel = set(fibers[state]) - inherited_by_any[state]
        if novel:
            novel_states.append((len(novel), state, len(fibers[state])))
            novel_orbits += len(novel)

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
    args = parser.parse_args()
    if args.phase_edge is not None:
        inspect_phase_edge(args.phase_edge)
        return
    if not 1 <= args.k <= 3:
        parser.error("the exact implementation currently supports 1 <= k <= 3")
    run(args.k, args.examples)


if __name__ == "__main__":
    main()
