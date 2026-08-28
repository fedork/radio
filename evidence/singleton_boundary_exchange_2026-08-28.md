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

The final provenance build is
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
