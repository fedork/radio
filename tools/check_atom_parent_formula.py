#!/usr/bin/env python3
"""Independently check the symbolic outer profile, width, and mixed-supply formulas."""

from __future__ import annotations

import math

Profile = tuple[int, int, int, int]
Supply = tuple[int, int, int]


def refine(profile: Profile) -> Profile:
    a, b, c, d = profile
    return 2 * a + b, b + c, c + d, d


def lift(profile: Profile, atoms: int) -> Profile:
    value = profile
    while sum(value) < atoms:
        value = refine(value)
    if sum(value) != atoms:
        raise ValueError("profile does not refine to the requested normalization")
    return value


def add_profiles(*profiles: Profile) -> Profile:
    return tuple(sum(profile[index] for profile in profiles) for index in range(4))  # type: ignore[return-value]


def profile_supply(profile: Profile) -> Supply:
    _, b, c, d = profile
    return d, c + d, b + c + d


def add_supplies(*supplies: Supply) -> Supply:
    return tuple(sum(supply[index] for supply in supplies) for index in range(3))  # type: ignore[return-value]


def transform_power(supply: Supply, steps: int) -> Supply:
    d, v, w = supply
    return d, v + steps * d, w + steps * v + math.comb(steps, 2) * d


def profile_value(profile: Profile, base: int) -> int:
    a, b, c, d = profile
    atom_a = 2**base
    atom_b = atom_a - 1
    atom_c = atom_b - base
    atom_d = atom_c - math.comb(base, 2)
    return a * atom_a + b * atom_b + c * atom_c + d * atom_d


def closed_width(k: int, s: int, b: int, c: int) -> int:
    return (
        2**k
        - k * k
        + (2 * s - c) * k
        - s * s
        - 3 * s
        + c * (s + 1)
        - b
        + 2
    )


def parent_profile(s: int, b: int, c: int) -> Profile:
    atoms = 2**s
    if b < 0 or c < 0 or b + c + 2 > atoms:
        raise ValueError("invalid D-germ exponents")
    outer_c = lift((7, 1, 0, 0), atoms)
    outer_a_minus_c = lift((3, 3, 2, 0), atoms)
    outer_b = lift((5, 2, 1, 0), atoms)
    d_germ = (atoms - b - c - 2, b, c, 2)
    return add_profiles(outer_c, outer_a_minus_c, outer_b, d_germ)


def main() -> int:
    cases = 0
    for s in range(3, 11):
        atoms = 2**s
        for c in range(min(2 * s + 1, atoms - 1)):
            for b in range(min(2 * s + 1, atoms - c - 1)):
                actual = parent_profile(s, b, c)
                expected = (
                    4 * atoms - 3 * s - b - c - 2,
                    3 * s + b - 3,
                    c + 3,
                    2,
                )
                if actual != expected or sum(actual) != 4 * atoms:
                    raise AssertionError(
                        f"parent profile mismatch at s={s}, b={b}, c={c}: "
                        f"{actual} != {expected}"
                    )
                for k in (s + 5, s + 12, 2 * s + 10):
                    base = k - s - 2
                    if profile_value(actual, base) != closed_width(k, s, b, c):
                        raise AssertionError(
                            f"width formula mismatch at k={k}, s={s}, b={b}, c={c}"
                        )
                    cases += 1

    known = {
        (4, 0, 1): ((49, 9, 4, 2), -21),
        (5, 0, 3): ((108, 12, 6, 2), -20),
        (5, 1, 3): ((107, 13, 6, 2), -21),
    }
    for (s, b, c), (expected_profile, expected_constant) in known.items():
        actual = parent_profile(s, b, c)
        if actual != expected_profile:
            raise AssertionError(f"known parent mismatch: {actual} != {expected_profile}")
        constant = -s * s - 3 * s + c * (s + 1) - b + 2
        if 2 * s - c != 7 or constant != expected_constant:
            raise AssertionError("known width specialization mismatch")

    # The unique unresolved 32-atom root has two exact D lineages.  Re-derive its unweighted
    # supply and the complete six-profile terminal requirement without using the C++ search.
    atom_count = 32
    rank1180_parts = (
        lift((5, 2, 1, 0), atom_count),
        lift((3, 3, 2, 0), atom_count),
        (27, 0, 3, 2),
    )
    rank1180_supply = add_supplies(*(profile_supply(part) for part in rank1180_parts))
    units: tuple[Profile, ...] = (
        (1, 0, 0, 0),
        (0, 1, 0, 0),
        (0, 0, 1, 0),
        (0, 0, 1, 0),
        (0, 0, 0, 1),
        (0, 0, 0, 1),
    )
    terminal_requirement = add_supplies(
        *(profile_supply(lift(unit, atom_count)) for unit in units)
    )
    if rank1180_supply != (2, 8, 19) or terminal_requirement != (2, 14, 45):
        raise AssertionError("32-atom rank-1180 supply derivation mismatch")
    depth3_upper = transform_power(rank1180_supply, 3)
    depth4_upper = transform_power(rank1180_supply, 4)
    if depth3_upper != (2, 14, 49) or depth4_upper != (2, 16, 63):
        raise AssertionError("32-atom rank-1180 propagated supply mismatch")

    # At depth three the first mixed transition must lose no D or V and at most four W.  At
    # depth four it must lose no D and at most two V; if all two V units are spent, propagation
    # through the final three levels leaves room for at most twelve units of immediate W loss.
    depth3_slack = tuple(
        upper - required for upper, required in zip(depth3_upper, terminal_requirement)
    )
    depth4_slack = tuple(
        upper - required for upper, required in zip(depth4_upper, terminal_requirement)
    )
    if depth3_slack != (0, 0, 4) or depth4_slack != (0, 2, 18):
        raise AssertionError("32-atom rank-1180 supply slack mismatch")
    depth4_w_budget_at_v_boundary = depth4_slack[2] - 3 * depth4_slack[1]
    if depth4_w_budget_at_v_boundary != 12:
        raise AssertionError("depth-four conditional W-loss budget mismatch")

    print(
        f"atom parent formula verified: {cases} direct evaluations and 3 boundary cases; "
        "rank-1180 supply=(2,8,19), terminal=(2,14,45), "
        "depth3_loss=(D=0,V=0,W<=4), depth4_loss=(D=0,V<=2,W<=12_at_V=2)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
