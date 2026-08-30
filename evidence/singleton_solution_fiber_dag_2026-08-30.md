# Singleton solution-fiber DAG (`K=3`, 2026-08-30)

## Question and the correct graph

Start at the canonical state `G_K` and repeatedly make a normalized unit Robin--Hood transfer

    y=x-e_i+e_j,                 x_i>=x_j+2.

The recipient may be one of the zero rows in the padding to `3^K` rows.  The normalized parent
states form a directed acyclic graph, not a tree: a state may have several predecessors.  The
question in this experiment is whether the *solution fiber* above a parent changes locally along
an edge, and whether locally transported solutions starting at the canonical cut still project
onto every parent state.

Two exact fibers were surveyed.

1. A **coloring orbit** is a row bipartition `A/B` satisfying the Fixed-Color Hall inequalities,
   modulo global `A<->B` and permutations of equal rows.  Every such orbit admits a legal first
   cut, but its allocation is not fixed.
2. An **allocation orbit** is the complete normalized multiset of row triples `(l_i,m_i,r_i)`,
   modulo `L<->R` and permutations of equal parent rows.  Every row obeys `l_i r_i=0`; all three
   child sequences have mass `3^(K-1)` and are majorized by `G_(K-1)`.

For a coloring, literal transport keeps the marked donor and recipient rows in their current
colors while moving the coin.  For an allocation, literal transport is stricter: move the coin
from donor to recipient in the same one of `L,M,R`, keep every other row triple fixed, and require
the resulting allocation to remain legal.  Each parent edge therefore carries a finite bipartite
relation between its source and target solution fibers.  A source orbit with no image **dies** on
that edge; a target orbit with no preimage on that edge is **born** there.

This literal allocation relation is stronger than the earlier Adjacent-Fiber relation.  A common
Hall coloring permits the two endpoint allocations to be rebuilt independently; literal transport
does not.

## Complete coloring-fiber result

`tools/singleton_solution_fiber_dag.py` exhausts all 1,206 full-mass normalized states and all
8,916 normalized unit transfers at `K=3`.  There are 31,498 feasible coloring orbits in total.

| quantity | exact value |
|---|---:|
| parent edges with a nonempty transport relation | 8,916 / 8,916 |
| edges with no dying coloring orbit | 8,632 |
| edges with no newly born coloring orbit | 5,123 |
| edges with neither births nor deaths | 4,841 |
| total transport links | 479,832 |
| coloring orbits reachable from the unique `G_3` orbit | 31,242 / 31,498 |
| parent states hit by that canonical component | 1,206 / 1,206 |

Thus edge fibers are not monotone: 284 edges kill one or two coloring orbits, and 3,793 edges
create at least one orbit relative to that particular predecessor.  Nevertheless every parent
edge retains at least one orbit, and the single canonical source reaches a solution above every
parent state.

Exactly one noncanonical coloring orbit has no preimage from *any* immediate predecessor.  It is

    parent = (8,6,5,4,1,1,1,1),
    A=(8,6,1,1),       B=(5,4,1,1).

The canonical source misses 256 coloring orbits across 74 states.  Starting instead at the one
new orbit above and following the same transport relation reaches 30,931 orbits: all 256 missed
by the canonical source and 30,675 also reached canonically.  Consequently the whole coloring
solution DAG has exactly two source orbits, and their descendant sets cover all 31,498 vertices.

## Complete allocation-fiber result

`tools/singleton_allocation_fiber_dag.cpp` independently enumerates complete cuts parent-first.
Its 47,165,174 recursive search nodes produce 2,125,199 oriented complete allocations and
1,063,464 normalized allocation orbits.  The last two totals agree exactly with an unrestricted
run of the older independent `tools/singleton_split_multiplicity_census.cpp`.

| quantity | exact value |
|---|---:|
| parent edges with a literal allocation transport | 8,916 / 8,916 |
| edges with no dying allocation orbit | 8,000 |
| edges with no newly born allocation orbit | 2,544 |
| edges with neither births nor deaths | 1,920 |
| total literal allocation links | 26,135,976 |
| allocation orbits reachable from the unique cut of `G_3` | 1,063,144 / 1,063,464 |
| parent states hit by that canonical component | 1,206 / 1,206 |

The same unique phase source appears.  The target parent now has four allocation orbits, three
inherited from `G_3` and the one new cut

    (0,4,4), (0,3,3), (4,1,0), (3,1,0),
    (1,0,0), (1,0,0), (0,0,1), (0,0,1).

All three children of this cut are exactly `G_2=(4,3,1,1)`.  The new source reaches 1,059,979
allocation orbits.  It covers all 320 orbits missed by the canonical source and overlaps the
canonical component on 1,059,659.  The union is the full set of 1,063,464 allocation orbits.
Hence the complete cut DAG, like the coarser coloring DAG, has exactly two source orbits and its
two branches merge almost completely.

This is the useful low-level shape: the projection from solutions to parents is surjective and
every parent edge lifts, but it is not a graph covering or an injection.  Fiber vertices are born,
die, branch and merge even though the canonical component never loses parent-level coverage.

## A genuine Pascal phase birth at every level

The exceptional source has a uniform proof.  Let `K>=3`, put

    h=G_(K-1),        U=2^(K-1),        w=U-K.

The first four entries of `h` are `(U,U-1,w,w)`, and the first four rows of the canonical
decomposition of `G_K` are

    (U,U,0), (0,U-1,U), (U-1,w,0), (0,w,U-1).

Their parent sizes are `(2U,2U-1,2U-K-1,2U-K-1)`.  Transfer one coin from the second row to the
third.  The new parent `X_K` has head

    (2U,2U-2,2U-K,2U-K-1).

Replace the four displayed allocation rows by

    (0,U,U), (0,U-1,U-1), (U,w,0), (U-1,w,0),              (PB)

and leave the canonical tail unchanged.  Every row is legal.  The mixed contributions are still
`h`; the two pure children still have head `(U,U-1)`, and the paired Pascal tail is unchanged.
Thus all three children are exactly `h`.  This proves that `(PB)` is a legal cut at every `K>=3`.

It is genuinely new under literal transport.  In every feasible cut of `G_K`, its top two parent
rows must lie on opposite pure sides, because putting them together would demand

    2U+(2U-1)=4U-1 > 2 H_h(2)=4U-2.

In `(PB)`, the unchanged top row and the shrunken donor lie on the same pure side.  A literal
transport cannot change either orientation, so no cut of `G_K` maps to `(PB)`.

Moreover `G_K` is the only more-head-heavy normalized one-coin predecessor of `X_K`.  Relative to
`G_K`, the Lorenz-prefix slack of `X_K` is one at prefix two and zero at every other prefix.  An
inverse unit transfer raises the Lorenz curve on one interval of prefixes; staying below `G_K`
forces that interval to be the singleton `{2}`, hence forces the reverse third-to-second transfer.
So `(PB)` is a true source vertex of the literal solution DAG at every `K>=3`, not a normalization
artifact.

Exact coloring enumeration gives the same local fiber pattern at `K=3,4,5`: `G_K` has one orbit,
`X_K` has three, two are inherited and `(PB)` supplies the unique new orbit.  The respective heads
are

    K=3: (8,6,5,4),
    K=4: (16,14,12,11),
    K=5: (32,30,27,26).

This is a concrete meaning of **phase change**: a tight pure-side Hall inequality forbids the top
two rows from sharing a side; after the unit transfer it becomes equality, and a four-row Pascal
reassociation creates a new exact `h,h,h` decomposition.

## Consequence for a proof strategy

The experiment supports the user's global-schedule idea, but it separates two possible claims.

* “Every solution is obtained from the canonical cut” is false at every `K>=3`, by `(PB)`.
* The sufficient statement is that the literal-transport component of the canonical cut projects
  onto every parent state.  This is exactly true at `K=3`, even though it misses 320 individual
  cuts.

Call the latter the **Canonical Allocation-Transport Conjecture**.  It is stronger than the
Singleton Majorization Converse, because it asks for an entire monotone path whose every step
changes only the transported coin.  It is not implied by edgewise nonempty fibers: 916 of the
`K=3` edges kill some source cuts, so independently choosing one common cut per edge need not
compose along a path.

A minimal-counterexample proof now has a precise target.  Let `R(x)` be the allocations over `x`
reachable from the canonical cut.  If a first parent with `R(x)=empty` existed, every incoming
edge would have a nonempty full transport relation but all of its source endpoints would avoid the
reachable fibers of the predecessors.  The Pascal phase calculation shows what a new source of
the full solution relation looks like.  A proof must show either that all such source phases are
explicit Pascal reassociations and harmless to parent coverage, or directly that some incoming
relation always meets a reachable predecessor fiber.  This retains the whole fiber and avoids the
earlier mistake of selecting incompatible edge certificates independently.

## Reproduction and scope

```text
tools/singleton_solution_fiber_dag.py 1
tools/singleton_solution_fiber_dag.py 2
tools/singleton_solution_fiber_dag.py 3
tools/singleton_solution_fiber_dag.py --phase-edge 3
tools/singleton_solution_fiber_dag.py --phase-edge 4
tools/singleton_solution_fiber_dag.py --phase-edge 5

CC=clang++ tools/build_radio.py -std=c++20 -O3 -Wall -Wextra -pedantic \
  tools/singleton_allocation_fiber_dag.cpp \
  -o /tmp/singleton_allocation_fiber_dag

tools/capped_run.sh --seconds 1800 --rss-gb 4 \
  --label singleton-allocation-fiber-k3 -- \
  tools/run_with_provenance.py /tmp/singleton_allocation_fiber_dag 3 0
```

The final allocation build id is
`1f29d8c908bc30fc42bfd06ab6c18f1e73445f1fd771da87f54ec5556a2c488e`.
It was built from clean commit `aa0a61198161a054d4cb7cb7f39e717585464416`.
The complete component run took 51 wrapper wall seconds, 49.5431 in-process seconds and 0.10 GB
peak RSS on the recorded Apple M4 Pro.  The earlier full-detail run without the second component
propagation took 31 wall seconds.  The exact coloring run takes about four wall seconds.
An address/undefined-sanitized build
`be0d4f6b02fa3b9761f8b29bbbb48c6349e44f179764878ec44c4e118fe63df5`
reproduced the complete `K=3` counts without a diagnostic in 412.236 in-process / 414 wrapper wall
seconds at 1.86 GB peak RSS.

All exhaustive global claims in this note stop at `K=3`.  The proved phase-birth construction is
uniform in `K`, and only its small local fiber counts were checked through `K=5`.  Neither survey
proves the Canonical Allocation-Transport Conjecture or the Singleton Majorization Converse for
arbitrary `K`.
