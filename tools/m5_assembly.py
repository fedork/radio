#!/usr/bin/env python3
"""Symbolic arithmetic for the exact m=5 Pareto-assembly calibration.

Li--Wu--Triesch prove the exact piecewise value of n(k,5).  This module rewrites
their two construction types in the corrected A/B/C/D assembly notation used by
this repository.  It also verifies the singleton-majorization inequalities for a
formerly conditional eventual D-branch construction; the converse needed for that inference is
false.  The words BBBD, ABBD and AABD below
remain mass identities, not assertions that a symmetric aligned atom tree exists.

Run without arguments for the transition levels, or name levels explicitly:

    tools/m5_assembly.py
    tools/m5_assembly.py 8 9 10 11
"""
from __future__ import annotations

import argparse
from dataclasses import dataclass
from math import comb
from typing import Tuple


Part = Tuple[int, int]
State = Tuple[Part, ...]


def canonical_state(parts: Tuple[Part, ...]) -> State:
    return tuple(sorted((n, m) for n, m in parts if n and m))


def split_state(parts: State, cuts: State) -> Tuple[State, State, State]:
    """Apply one synchronized rectangle test; return outcomes 2, 1 and 0."""
    if len(parts) != len(cuts):
        raise ValueError("a synchronized cut needs one rectangle per state part")
    out2 = []
    out1 = []
    out0 = []
    for (n, m), (x, y) in zip(parts, cuts):
        if not (0 <= x <= n and 0 <= y <= m):
            raise ValueError(f"illegal cut {(x, y)} for part {(n, m)}")
        out2.append((x, y))
        out1.extend(((x, m - y), (n - x, y)))
        out0.append((n - x, m - y))
    return (canonical_state(tuple(out2)), canonical_state(tuple(out1)),
            canonical_state(tuple(out0)))


def singleton_prefix(level: int, count: int) -> Tuple[int, ...]:
    """The first ``count`` entries of G_level, without materializing all 2^level entries."""
    values = []
    for block in range(level + 1):
        value = sum(comb(level, i) for i in range(level - block + 1))
        multiplicity = 1 if block == 0 else 2 ** (block - 1)
        values.extend([value] * min(multiplicity, count - len(values)))
        if len(values) == count:
            return tuple(values)
    raise ValueError(f"G_{level} has fewer than {count} entries")


def singleton_majorized(state: State, level: int) -> bool:
    """Necessary singleton-majorization test for a state whose parts have height one."""
    if any(m != 1 for _, m in state):
        return False
    widths = sorted((n for n, _ in state), reverse=True)
    reference = singleton_prefix(level, len(widths))
    return all(sum(widths[:i]) <= sum(reference[:i]) for i in range(1, len(widths) + 1))


def decisive_majorized_leaf(r: int) -> Tuple[int, ...]:
    """The five-part leaf which makes the eventual D template non-canonical.

    Here ``r=t-2=k-4`` and the entries are returned in nonincreasing order.
    The first instance in the published eventual regime is
    ``(127,119,119,118,111)`` at ``r=7``.
    """
    if r < 3:
        raise ValueError("the decisive leaf notation requires r>=3")
    scale = 2**r
    b = scale - 1
    x = scale - r - 2
    e = scale + r + 4 - comb(r, 2)
    y = scale - 2 * r - 3
    return tuple(sorted((b, x, x, e, y), reverse=True))


def uniform_exact_inventory_coefficients(depth: int) -> Tuple[int, int]:
    """Return the forced C and B counts beside one D in an eventual exact E fit.

    Comparing quadratic and linear deficit coefficients at fixed extra depth
    forces ``q=depth-2`` C atoms and ``p=(depth-6)(depth+1)/2`` B atoms.  This is
    a necessary eventual inventory calculation, not a synchronized-tree proof.
    """
    if depth < 0:
        raise ValueError("depth must be nonnegative")
    return depth - 2, (depth - 6) * (depth + 1) // 2


def depth_six_exact_inventories(r: int) -> dict[str, Tuple[int, ...]]:
    """Candidate 64-piece exact inventories for the decisive leaf's components.

    The identities are meaningful as positive G_s atoms for ``s=r-6>=3``.
    They prove the scalar per-component fit only; assigning the atoms to one
    synchronized six-test tree is a separate open packing problem.
    """
    if r < 9:
        raise ValueError("the positive depth-six atom identities require r>=9")
    s = r - 6
    a = 2**s
    b = a - 1
    c = a - s - 1
    d = a - 1 - s * (s + 1) // 2
    return {
        "B": (a,) * 63 + (b,),
        "X": (a,) * 56 + (b,) * 7 + (c,),
        "Y": (a,) * 49 + (b,) * 13 + (c,) * 2,
        "E": (a,) * 59 + (c,) * 4 + (d,),
    }


def depth_three_y_obstruction() -> bool:
    """Return whether the first failed depth-three Y inventory is certified.

    At ``r=10`` there would be eight atoms of ``G_7`` summing to
    ``Y_10=1001``.  Relative to eight A-atoms their deficit is 23.  The only
    atom deficits below 23 are 0, 1 and 8, but ``p+8q=23`` with
    ``p+q<=8`` has no nonnegative solution.
    """
    deficits = tuple(2**7 - value for value in singleton_prefix(7, 5))
    if deficits != (0, 1, 8, 8, 29):
        raise ValueError(f"unexpected first G_7 deficits {deficits}")
    return not any(p + 8 * q == 23 and p + q <= 8
                   for p in range(9) for q in range(9))


def eventual_d_template_works(t: int) -> bool:
    """Check the uniform two-test D construction for the +1 regime.

    The target is R_t(2^t-binomial(t-2,2)+1).  The template is legal already at
    t=7,8 but one singleton outcome fails majorization there.  The necessary
    inequalities succeed for every t>=9, exactly where the published frontier gains its second coin.  All
    terminal states are checked directly against G_(t-1) or G_(t-2).
    """
    if t < 7:
        raise ValueError("the eventual D template is compared from t=7")
    scale = 2**t
    half = scale // 2
    quarter = scale // 4
    q = comb(t - 2, 2)
    d = scale - q + 1

    root = ((d, 3), (scale - t, 1), (scale - 2 * t, 1))
    first_cut = ((half - t, 2), (half, 1), (half - 2 * t, 0))
    first = split_state(root, first_cut)

    u2 = canonical_state(((half, 1), (half - t, 2)))
    u0 = canonical_state(((half, 1), (half - q + t + 1, 1)))
    hard = canonical_state(((half - q + t + 1, 2),
                            (half - t, 1), (half - t, 1), (half - 2 * t, 1)))
    if first != (u2, hard, u0):
        raise ValueError(f"t={t}: eventual D first-test algebra mismatch")

    u2_parts = ((half, 1), (half - t, 2))
    u2_cut = ((quarter, 0), (quarter - 1, 1))
    u2_leaves = split_state(u2_parts, u2_cut)

    hard_parts = ((half - q + t + 1, 2),
                  (half - t, 1), (half - t, 1), (half - 2 * t, 1))
    hard_cut = ((quarter - 1, 1), (quarter, 1),
                (quarter - t, 0), (quarter - 2 * t + 1, 0))
    hard_leaves = split_state(hard_parts, hard_cut)

    decisive = tuple(sorted((n for n, m in hard_leaves[1] for _ in range(m)), reverse=True))
    if decisive != decisive_majorized_leaf(t - 2):
        raise ValueError(f"t={t}: decisive singleton leaf mismatch {decisive}")

    leaves = ((u0, t - 1),
              *((leaf, t - 2) for leaf in u2_leaves),
              *((leaf, t - 2) for leaf in hard_leaves))
    return all(singleton_majorized(leaf, level) for leaf, level in leaves)


@dataclass(frozen=True)
class AssemblyCandidate:
    name: str
    alpha: int
    beta: int
    gamma: int
    a: int
    b: int
    c: int
    d: int
    branch: Tuple[Part, ...]

    @property
    def parent_width(self) -> int:
        return self.a + self.b + self.d


def correction(k: int) -> int:
    """The c(k) constant in the published exact formula."""
    if k < 3:
        raise ValueError("the published m=5 formula starts at k=3")
    if k <= 8:
        return 5
    if k <= 10:
        return 6
    return 7


def theorem_quadratic(k: int) -> int:
    """The paper's (k-4)(k-5)/2 term, including its k=3 value."""
    return (k - 4) * (k - 5) // 2


def exact_width(k: int) -> int:
    """Published exact n(k,5)."""
    return 2**k - 3 * k - theorem_quadratic(k) + correction(k)


def old_width(k: int) -> int:
    """The former BBBD formula, exact only through k=8."""
    if k < 4:
        raise ValueError("the normalized Sb(n:5) calibration starts at k=4")
    return 2**k - 3 * k - theorem_quadratic(k) + 5


def old_candidate(k: int) -> AssemblyCandidate:
    """The (alpha,beta,gamma)=(3,2,2) / 3+2 construction."""
    t = k - 2
    scale = 2**t
    q = comb(t - 2, 2)
    a = 2 ** (k - 1) - (k - 1)       # n(k-1,3)
    b = c = scale - 1                 # n(k-2,2)
    d = scale - 2 * t - q + 1
    branch = tuple(p for p in ((d, 2), (b, 1), (a - c, 2)) if p[0] and p[1])
    return AssemblyCandidate("3+2", 3, 2, 2, a, b, c, d, branch)


def new_candidate(k: int) -> AssemblyCandidate:
    """The (alpha,beta,gamma)=(4,3,1) / 4+1 exact-width candidate.

    The paper constructs this candidate from k=9 onward.  Exact finite assembly
    search also constructs it at k=8, where it ties the old branch.  For k<8 the
    returned dimensions are only the arithmetic dimensions required to reach the
    exact parent width; constructibility is not asserted.
    """
    t = k - 2
    scale = 2**t
    a = 2 ** (k - 1) - 2 * (k - 1) + 2  # n(k-1,4)
    b = scale - t                         # n(k-2,3)
    c = scale                             # n(k-2,1)
    d = exact_width(k) - a - b
    branch = tuple(p for p in ((d, 3), (b, 1), (a - c, 1)) if p[0] and p[1])
    return AssemblyCandidate("4+1", 4, 3, 1, a, b, c, d, branch)


def atom_masses(k: int) -> Tuple[int, int, int]:
    """Return the arithmetic masses BBBD, ABBD and AABD at G_(k-2)."""
    t = k - 2
    a = 2**t
    b = a - 1
    d = a - 1 - t - comb(t, 2)
    return 3 * b + d, a + 2 * b + d, 2 * a + b + d


def validate(k: int) -> None:
    """Check every symbolic identity used by the calibration at one level."""
    if k < 4:
        raise ValueError("require k>=4")
    t = k - 2
    scale = 2**t
    q = comb(t - 2, 2)

    old = old_candidate(k)
    if old.parent_width != old_width(k):
        raise ValueError(f"k={k}: old assembly width mismatch")
    expected_old_branch = ((scale - 2 * t - q + 1, 2),
                           (scale - 1, 1),
                           (scale - t, 2))
    if old.branch != expected_old_branch:
        raise ValueError(f"k={k}: old hard-branch mismatch {old.branch}")
    old_root = split_state(((old.parent_width, 5),), ((old.a, 3),))
    expected_old_root = (canonical_state(((old.a, 3),)),
                         canonical_state(((old.a, 2), (old.b + old.d, 3))),
                         canonical_state(((old.b + old.d, 2),)))
    if old_root != expected_old_root:
        raise ValueError(f"k={k}: old root rectangle algebra mismatch")
    old_second = split_state(((old.a, 2), (old.b + old.d, 3)),
                             ((old.a - old.c, 0), (old.b, 2)))
    expected_old_second = (canonical_state(((old.b, 2),)),
                            canonical_state(old.branch),
                            canonical_state(((old.c, 2), (old.d, 1))))
    if old_second != expected_old_second:
        raise ValueError(f"k={k}: old second-test rectangle algebra mismatch")

    new = new_candidate(k)
    if new.parent_width != exact_width(k):
        raise ValueError(f"k={k}: new assembly width mismatch")
    expected_new_branch = tuple(
        p for p in ((new.d, 3), (scale - t, 1), (scale - 2 * t, 1)) if p[0] and p[1]
    )
    if new.branch != expected_new_branch:
        raise ValueError(f"k={k}: new hard-branch mismatch {new.branch}")
    if new.c > new.a:
        raise ValueError(f"k={k}: new C width {new.c} does not fit A width {new.a}")
    new_root = split_state(((new.parent_width, 5),), ((new.a, 4),))
    expected_new_root = (canonical_state(((new.a, 4),)),
                         canonical_state(((new.a, 1), (new.b + new.d, 4))),
                         canonical_state(((new.b + new.d, 1),)))
    if new_root != expected_new_root:
        raise ValueError(f"k={k}: new root rectangle algebra mismatch")
    new_second = split_state(((new.a, 1), (new.b + new.d, 4)),
                             ((new.a - new.c, 0), (new.b, 3)))
    expected_new_second = (canonical_state(((new.b, 3),)),
                            canonical_state(new.branch),
                            canonical_state(((new.c, 1), (new.d, 1))))
    if new_second != expected_new_second:
        raise ValueError(f"k={k}: new second-test rectangle algebra mismatch")

    expected_d = scale - q + (-1 if k <= 8 else 0 if k <= 10 else 1)
    if new.d != expected_d:
        raise ValueError(f"k={k}: new D formula gives {new.d}, expected {expected_d}")

    bbbd, abbd, aabd = atom_masses(k)
    if (bbbd, abbd, aabd) != (old_width(k), old_width(k) + 1, old_width(k) + 2):
        raise ValueError(f"k={k}: atom-mass identities disagree")
    expected_exact_mass = bbbd if k <= 8 else abbd if k <= 10 else aabd
    if expected_exact_mass != exact_width(k):
        raise ValueError(f"k={k}: regime mass does not equal the exact theorem")

    if k in (9, 10) and eventual_d_template_works(t):
        raise ValueError(f"k={k}: eventual +1 D template should fail before k=11")
    if k >= 11 and not eventual_d_template_works(t):
        raise ValueError(f"k={k}: eventual +1 D template failed")

    # At any fixed depth <=5, the forced eventual E inventory would need a
    # negative number of B atoms.  Depth six is the first scalar possibility.
    for depth in range(6):
        _, b_count = uniform_exact_inventory_coefficients(depth)
        if b_count >= 0:
            raise ValueError(f"depth={depth}: expected negative eventual B count")
    if uniform_exact_inventory_coefficients(6) != (4, 0):
        raise ValueError("depth-six eventual inventory coefficients changed")
    if not depth_three_y_obstruction():
        raise ValueError("the r=10 depth-three Y obstruction disappeared")

    # From r=9 onward A_s,B_s,C_s,D_s are positive entries of G_s.  Check all
    # four 64-piece identities against the exact decisive-leaf widths.
    r = k - 4
    if r >= 9:
        inventory = depth_six_exact_inventories(r)
        scale_r = 2**r
        targets = {
            "B": scale_r - 1,
            "X": scale_r - r - 2,
            "Y": scale_r - 2 * r - 3,
            "E": scale_r + r + 4 - comb(r, 2),
        }
        base_prefix = set(singleton_prefix(r - 6, 5))
        for name, atoms in inventory.items():
            if len(atoms) != 64 or sum(atoms) != targets[name]:
                raise ValueError(f"r={r}: depth-six {name} inventory mismatch")
            if any(atom not in base_prefix for atom in atoms):
                raise ValueError(f"r={r}: depth-six {name} uses a non-atom")


def format_candidate(candidate: AssemblyCandidate) -> str:
    branch = ",".join(f"{n}:{m}" for n, m in candidate.branch)
    return (f"{candidate.name} heights=({candidate.alpha},{candidate.beta},"
            f"{candidate.gamma}) A={candidate.a} B={candidate.b} C={candidate.c} "
            f"D={candidate.d} parent={candidate.parent_width} branch={branch}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("k", nargs="*", type=int, default=[8, 9, 10, 11])
    parser.add_argument("--check-through", type=int, default=64,
                        help="validate every level from 4 through this value")
    args = parser.parse_args()

    if args.check_through < 4:
        parser.error("--check-through must be at least 4")
    for k in range(4, args.check_through + 1):
        validate(k)

    for k in args.k:
        validate(k)
        bbbd, abbd, aabd = atom_masses(k)
        print(f"k={k} exact={exact_width(k)} atom_masses=BBBD:{bbbd},ABBD:{abbd},AABD:{aabd}")
        print("  " + format_candidate(old_candidate(k)))
        print("  " + format_candidate(new_candidate(k)))
    print(f"validated m=5 assembly identities for k=4..{args.check_through}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
