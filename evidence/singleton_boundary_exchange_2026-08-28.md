# Singleton boundary-exchange diagnostics (2026-08-28)

## Exact flip and swap conditions

For a fixed feasible coloring `A/B`, let

    r(S)=H(|S|)+H(|S intersection A|)+H(|S intersection B|),
    slack(S)=r(S)-x(S),

and put `h_s=H(s)-H(s-1)`.  If `S` has old color counts `(p,q)`, flipping a contained
`B` row to `A` changes its rank by

    h_(p+1)-h_q.

For a swap `u in A`, `v in B`, sets containing both or neither are unchanged.  A set containing
`v` but not `u` has the displayed change; a set containing `u` but not `v` changes by

    h_(q+1)-h_p.

These identities are direct differences of the Fixed-Color Hall rank.  They show that a failed
`B->A` flip has an oppositely imbalanced blocker, and that the two blocker families for a failed
swap have opposite imbalance.  The theorem note combines them with the Tight Pascal-Band Lemma.

## The fixed-boundary family is not a delta-matroid

At `K=3`, use

    x=(3,2^11,1,1),             h=G_2=(4,3,1,1).

Index the width-three row by 0, the eleven width-two rows by 1 through 11, and the unit rows by
12 and 13.  The two feasible `A`-sets

    P={0,1,2,3},                Q={1,2,3,4,5}

violate symmetric exchange at `e=0`.  Since `P triangle Q={0,4,5}`, the only candidates are
`P triangle {0}`, `P triangle {0,4}`, and `P triangle {0,5}`.  The first leaves only three `A`
rows, making the full-set Hall capacity 26 below total mass 27.  Each of the other two leaves the
opposite color with mass 19, above the pure capacity `2H(10)=18`.

The new labelled-mask case mode verifies the whole feasible family and returns

    BOUNDARY_DELTA_CASE k=3 state=(3,2,2,2,2,2,2,2,2,2,2,2,1,1)
      feasible_colorings=13442 delta=NO x=15 y=62 e=0

Thus the theorem that expands a jump-system coordinate into many Boolean elements cannot be
restricted to the all-in/all-out block boundary used by row colorings.

## Row-floor and closest-boundary rules fail at K=4

The landscape census now classifies failed colorings by the recipient-side row count.  Through
`K=3`, all 325 failures above the necessary `2^(K-1)`-row floor have a one-row repair, while all
23 failures at the floor need a swap.  This dichotomy is already false at `K=4`.

In the first 5,000 full-mass `K=4` states there are 45,504 failed colorings.  Of these, 33,410
have a one-row repair and 12,094 need distance two.  The recipient side is at its eight-row floor
for 11,578 failures and above it for 33,926; 516 of the above-floor failures still have no direct
flip.  The first is

    x=(16,15,11,8,7,6,5,4,1^9),       transfer 8->6,
    A=(15,8,5,4,1^4),                  B=(16,11,7,6,1^5).

Two deterministic crossing swaps were then tested: swap the marked recipient with the closest
smaller non-donor row beyond the selected tight `A` prefix, or swap the largest such outside row
with the least selected `B` row above it.  They cover every no-flip failure through `K=3`, but
miss 170 failures in the same 5,000-state `K=4` window.  The first miss is

    x=(16,15,11,8,7,6,3,3,3,3,1^6),   transfer 8->6,
    A=(15,8,3,3,3,3,1,1),              B=(16,11,7,6,1,1,1,1).

It is repaired, up to globally exchanging the two color names, by swapping the marked donor 8
with the row 11.  This proves that neither transfer endpoint nor the closest value crossing can be
fixed in advance.  The surviving local target is existential: some `B` row inside the dangerous
cut must flip, or some such row must swap with an `A` row outside it.

## Reproduction

The boundary/delta build was
`cb48a5d92eccb9f3cbdf76b30c1b0cd2a31ad7cdb6126bbfebea6144512db25f`:

    CC=clang++ tools/build_radio.py -O3 -std=c++20 -Wall -Wextra -pedantic \
        tools/singleton_pair_coloring_census.cpp -o /tmp/singleton-boundary-final
    tools/run_with_provenance.py /tmp/singleton-boundary-final \
        --boundary-delta-case 3 3 2 2 2 2 2 2 2 2 2 2 2 1 1
    tools/run_with_provenance.py /tmp/singleton-boundary-final \
        --adjacent-fiber-landscape-census 3 0
    tools/capped_run.sh --seconds 180 --rss-gb 1 --label boundary-rules-k4 -- \
        tools/run_with_provenance.py /tmp/singleton-boundary-final \
        --adjacent-fiber-landscape-census 4 5000 0

The `K=3` run is exhaustive.  The `K=4` run is an exact initial window, not a complete level.  It
completed in 55 wall seconds at 0.01 GB reported peak RSS.

## Strict band descent is false; positive mass gives an acyclic target

The exact blocker mode `--boundary-blockers` distinguishes two failures of a crossing move: the
new coloring may violate an original-demand Hall inequality, or it may remain feasible for the
original state while exposing another tight separator for the transferred state.  For a rank-loss
blocker with old color counts `(a,b)`, the lost columns have heights in the closed interval between
`a` and `b`; the mode compares that interval with the open Pascal band of the target separator.

The delta-matroid counterexample above also refutes strict band descent.  Use transfer `3->1` and

    A=(3,2,2,2),       B=(2^8,1,1).

Its selected target cut is `(p,q)=(0,10)`.  Swapping the width-three donor with a selected
width-two row is infeasible, and its blocker changes counts `(1,9)->(0,10)`.  The blocker band and
target band are both

    levels=3, columns=4, width=10, p=0, q=10.

So an arbitrary alternating blocker chain has no strictly decreasing band potential.  In the same
case, flipping any selected width-two row is feasible and immediately supports the transfer.

This suggested testing a global but much simpler rule.  Among all feasible crossing flips and
swaps across the selected separator, maximize the increase in the total mass of the side receiving
the selected `B` row.  For material recipients, the exact quotient census found:

| window | failed colorings | no successful maximum | failed tied maximum |
|---|---:|---:|---:|
| complete `K=3` | 348 | 0 | 0 |
| first 5,000 `K=4` states | 45,504 | 0 | 0 |
| 1,000 states after skip 1,000,000 | 78,487 | 0 | not measured |
| 200 states after skip 3,000,000 | 41,842 | 0 | not measured |
| 200 states after skip 5,000,000 | 18,137 | 0 | not measured |

In the complete `K=3` run and first `K=4` window, every maximizing crossing neighbor succeeds,
not merely one choice among ties, and the minimum maximum mass gain is `1`.  Across all displayed
windows, all 183,970 failed colorings have a successful maximum-gain crossing neighbor.  The late
windows are exact on their stated ranges, not a complete `K=4` census.  A 1,000-state run at skip
3,000,000 timed out after 184 wall seconds and is an abort, not evidence; the completed 200-state
replacement took 96 seconds.

The weaker positive-gain statement is enough for a proof.  Repeatedly take any original-state
feasible crossing move that strictly increases the receiving side's mass.  A move of either marked
endpoint finishes by putting donor and recipient together.  Every other nonterminal move preserves
their orientation and strictly increases an integer mass bounded by the pure Hall inequality, so
the process cannot cycle.  The exact remaining claim is therefore the **Positive Pascal Crossing
Lemma**: such a positive feasible crossing move always exists.  It is still open.

## Tight-set core removes the reverse blocker

The dangerous `x`-tight sets form a lattice.  Write `C` for their intersection and `U` for their
union.  A one-flip/one-swap common coloring must move a `B` row `v in C` to `A`; a swap must move an
`A` row `u notin U` back to `B`.  For each such `v`, flip it provisionally and intersect all row
sets on which the flipped coloring violates the original demand.  Call that intersection `P_v`.

There is no second, oppositely imbalanced blocker family for a **positive** swap.  If a set `T`
contains `u` but not `v`, its rank after swapping colors equals the old rank of `T-u+v`.  Since
`x_v>x_u`, old feasibility of `T-u+v` leaves at least `x_v-x_u` slack for `T` after the swap.
Consequently the swap is feasible for `x` exactly when `u in P_v`.  If also `v in C` and
`u notin U`, the same swap is feasible for the transferred demand.  This reduces the open proof to
one global intersection statement: for some `v in C`, either its flip has no blocker, or `P_v-U`
contains a smaller `A` row.

`--boundary-blockers` now prints the dangerous lattice core/hull and the intersection/union of all
blockers for every tested move.  In the first `K=4` no-flip case

    x=(16,15,11,8,7,6,5,4,1^9),       transfer 8 -> 6,
    A=(15,8,5,4,1^4),                  B=(16,11,7,6,1^5),

there is one dangerous tight set, with

    C=U={15_A,16_B,11_B,7_B,6_B}.

Flipping `v=11` has 24 labelled blockers whose common intersection is

    P_11={15_A,8_A,5_A,4_A,16_B,11_B}.

Thus each of `u=8,5,4` lies in `P_11-U` and is smaller than 11; all three swaps are feasible and
finish the transfer.  In the closest-boundary counterexample

    x=(16,15,11,8,7,6,3^4,1^6),

the same dangerous core occurs, but the common blocker intersection for `v=11` is

    P_11={15_A,8_A,16_B,11_B}.

It selects the nonlocal swap `8_A <-> 11_B` directly.  These are exact labelled-mask case checks,
not a census proof of the intersection statement.

The final build for the complete `K=3` and first-5,000 `K=4` maximum/tie/positive-gain checks is
`fa329a545ac76dca7dda565267c854962707ed331e393d111ae9303346c3e46e`:

    CC=clang++ tools/build_radio.py -O3 -std=c++20 -Wall -Wextra -pedantic \
        tools/singleton_pair_coloring_census.cpp -o /tmp/singleton_boundary_positive_final
    tools/run_with_provenance.py /tmp/singleton_boundary_positive_final \
        --adjacent-fiber-landscape-census 3 0
    tools/capped_run.sh --seconds 180 --rss-gb 1 --label boundary-positive-k4 -- \
        tools/run_with_provenance.py /tmp/singleton_boundary_positive_final \
        --adjacent-fiber-landscape-census 4 5000 0

The final run took 56 wall seconds at 0.01 GB reported peak RSS.  The detailed blocker command is

    tools/run_with_provenance.py /tmp/singleton_boundary_positive_final \
        --boundary-blockers 3 3 1 13 3 2 2 2 2 2 2 2 2 2 2 2 1 1

The lattice/blocker-core extension was built as
`322b29b4bb6d29da2a36f410b69dcdba0a5b8b6cb0026862d90a7d9cb5d39036`.  Its two exact case commands
are

    CC=clang++ tools/build_radio.py -O3 -std=c++20 -Wall -Wextra -pedantic \
        tools/singleton_pair_coloring_census.cpp -o /tmp/singleton-core-work2
    tools/run_with_provenance.py /tmp/singleton-core-work2 --boundary-blockers 4 8 6 4 \
        16 15 11 8 7 6 5 4 1 1 1 1 1 1 1 1 1
    tools/run_with_provenance.py /tmp/singleton-core-work2 --boundary-blockers 4 8 6 1 \
        16 15 11 8 7 6 3 3 3 3 1 1 1 1 1 1
