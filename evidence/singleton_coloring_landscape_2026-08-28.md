# Singleton transfer-coloring landscape (2026-08-28)

## Why the residual problem must change colorings

For a full-mass state `x<=_w G_K`, fix a Robin--Hood value transfer `d->r`, `d>=r+2`.
For a feasible row coloring `sigma`, let

    m_sigma(d,r)=min_(recipient j in S, donor i not in S)
        (H(|S|)+H(|S intersection A|)+H(|S intersection B|)-x(S)).

The same coloring survives the transfer exactly when this margin is at least one.  If it is zero,
the tight set is a Fixed-Color Hall obstruction.  Therefore no incidence reroute preserving every
row's orientation can complete the transfer; the search must move to another feasible coloring of
`x`.

Equal rows are not intrinsically marked by a transfer of partition values, and the two pure sides
are globally interchangeable.  The diagnostic first enumerates marked donor/recipient
realizations, then quotients realizations having the same unordered pair of colored value
multisets, retaining the best choice of equal-row identities and global `A/B` orientation.  Its
row distance is the minimum number of rows whose colors differ, also minimized over complementing
one whole coloring.

For every failed coloring it computes the nearest successful coloring.  It also tests a proposed
monotone local descent: a neighbor improves if it raises the separating margin, or preserves the
margin while decreasing lexicographically

    (number of crossed dyadic levels,
     number of crossed Pascal columns,
     q-p, p, q)

for the best minimum-margin separator.

## Exact `K<=3` result

The complete enumeration gives:

| `K` | states | transfers | feasible normalized colorings | failed colorings | nearest-success distances |
|---:|---:|---:|---:|---:|---:|
| 1 | 2 | 1 | 1 | 0 | none |
| 2 | 15 | 33 | 88 | 0 | none |
| 3 | 1,206 | 8,916 | 237,617 | 348 | 325 at distance 1; 23 at distance 2 |

Thus changing one row is false even at `K=3`, but every failed coloring there has a successful
coloring within two row changes.  The first coloring whose nearest success is at distance two is

    x=(8,6,5,3,2,1,1,1),       transfer 5->2,
    A=(5,3,1,1),                B=(8,6,2,1).

Its margin-zero separator has `(p,q)=(0,3)`.  A successful coloring is obtained, for example, by
exchanging the rows of values 6 and 3.

## Disjoint `K=4` windows support two-row color exchange

The first 10,000 full-mass partitions in the same exact descending enumeration contain 282,690
normalized transfer types and 46,600,920 feasible normalized colorings.  Of these, 94,936
colorings fail their selected transfer.  Their nearest-success distances are recorded below in
the final run output:

| distance | failed colorings |
|---:|---:|
| 1 | 74,090 |
| 2 | 20,846 |

Thus the one-row rule fails again, but every failed coloring in this exact prefix has a successful
coloring within two row changes.  The first coloring attaining distance two is

    x=(16,15,11,11,5,5,5,4,2,1^7),       transfer 4->2,
    A=(15,11,5,4,1,1,1,1),
    B=(16,11,5,5,2,1,1,1).

It has margin zero at `(p,q)=(3,5)` and no successful one-row recoloring.  A two-row change works.
In fact, every failure in the table reaches success either by flipping one row or by swapping two
opposite-color rows; no same-direction two-flip move is needed.  This motivates the **Two-Row
Color-Exchange Lemma**: after the equal-row/global-complement quotient, every failed feasible
coloring has a successful feasible coloring obtainable by one flip or one swap.  That statement
would imply the Adjacent-Fiber Lemma directly.  The present computation proves it only through
`K=3` and on the stated `K=4` prefix.

To check that this was not an artifact of the head-heavy initial segment, the same final binary
also enumerated three disjoint 1,000-state windows after skipping 1,000,000, 3,000,000, and
5,000,000 states.  The four windows together give:

| `K=4` states | transfers | feasible normalized colorings | failed colorings | distance 1 | distance 2 |
|---:|---:|---:|---:|---:|---:|
| 13,000 | 369,300 | 261,315,748 | 377,873 | 352,471 | 25,402 |

Every distance-two failure has an opposite-color swap repair.  The windows after 3,000,000 and
5,000,000 states in fact have no distance-two failures at all.  These are exact disjoint windows,
not random samples, but they are still only 13,000 of the 5,997,038 `K=4` states.

This move shape agrees with the support-one-or-two delta-exchange directions for integer points of
an integral bisubmodular polyhedron.  It does not follow formally from that theorem: encoding a
coloring as `z_i=+/-x_i` restricts the bisubmodular polyhedron to the nonconvex boundary
`|z_i|=x_i`, while an elementary delta exchange changes coordinates by one.  Showing that the
Pascal capacities compress such an interior unit-exchange chain to a boundary flip or swap is the
remaining theoretical step.

## Reproduction

The final provenance build is
`559585c2f01d0c51942cdf27a6013160df14c3d8ee0d160c4746a7bfa618a638`.

    CC=clang++ tools/build_radio.py -O3 -std=c++20 -Wall -Wextra -pedantic \
        tools/singleton_pair_coloring_census.cpp -o /tmp/singleton-pair-landscape-final3
    tools/run_with_provenance.py /tmp/singleton-pair-landscape-final3 \
        --adjacent-fiber-landscape-census 1 0
    tools/run_with_provenance.py /tmp/singleton-pair-landscape-final3 \
        --adjacent-fiber-landscape-census 2 0
    tools/run_with_provenance.py /tmp/singleton-pair-landscape-final3 \
        --adjacent-fiber-landscape-census 3 0
    tools/run_with_provenance.py /tmp/singleton-pair-landscape-final3 \
        --adjacent-fiber-landscape-census 4 10000 0
    tools/run_with_provenance.py /tmp/singleton-pair-landscape-final3 \
        --adjacent-fiber-landscape-census 4 1000 1000000
    tools/run_with_provenance.py /tmp/singleton-pair-landscape-final3 \
        --adjacent-fiber-landscape-census 4 1000 3000000
    tools/run_with_provenance.py /tmp/singleton-pair-landscape-final3 \
        --adjacent-fiber-landscape-census 4 1000 5000000

The `K<=3` runs are exhaustive.  The `K=4` runs are exact censuses of the stated disjoint windows,
not random samples and not a complete `K=4` result.  The diagnostic enumerates complete
feasible-coloring sets for each included transfer; it does not infer absent local moves from a
greedy search.  The four `K=4` runs were run concurrently; the longest finished in under five wall
minutes and all remained below 50 MB RSS.
