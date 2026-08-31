# Singleton solution-fiber DAG (`K=3`, 2026-08-30)

**Superseded universal conclusion (2026-08-30).**  The exact `K=3` fiber counts and connectivity
results below remain valid.  Pascal-Shuffle Coverage and universal transfer closure are false:
the final unit transfer into the explicit `K=6` counterexample takes a nonempty first-cut fiber to
an empty one.  See [the counterexample](singleton_k6_counterexample_2026-08-30.md).

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

## The complete first Lorenz shell

The local pattern is now proved at every level and classified at every immediate dominance cover.
Write

    v_d=sum_(s=d..K) binomial(K,s),
    G_K=(v_0,v_1,v_2^2,v_3^4,...,v_K^(2^(K-1))).

For `1<=d<=K-1`, transfer one coin from the last `v_d` row to the first `v_(d+1)` row and call the
result `X_(K,d)`.  These are exactly the states at Lorenz area

    D(x)=sum_t(H_(G_K)(t)-H_x(t))=1.

Let `h=G_(K-1)`.  If the top `2s` parent rows contain `p` rows of color `A`, their instance of the
Fixed-Color Hall inequality and the Pascal prefix identity imply

    2H_h(s)-epsilon <= H_h(p)+H_h(2s-p),

where `epsilon=1` only at the transferred prefix.  The loss from the balanced maximum when
`p!=s` is at least `h_s-h_(s+1)`.  At `s=2^(e-1)` this jump is exactly

    binomial(K-1,e-1).

Thus one unit of Lorenz slack can unbalance only the first dyadic boundary.  All deeper jumps cost
at least two.  Taking differences between successive dyadic prefix counts gives the exact fibers:

| parent | feasible coloring orbits | inherited from `G_K` | new |
|---|---:|---:|---:|
| `G_K` | 1 | -- | -- |
| `X_(K,1)`, `K>=3` | 3 | 2 | 1, namely `(PB)` |
| `X_(K,d)`, `2<=d<=K-1` | 4 | 4 | 0 |

Existence of the inherited orbits is constructive.  Choose canonical donor and recipient chains
of the desired first-test colors and move the canonical bottom cell.  That word starts with the
mixed symbol `1`, so both row colors remain fixed.  At `d=1`, `(PB)` supplies the sole third
coloring.  Since area strictly increases on every normalized Robin--Hood edge, an area-one state
has no possible predecessor other than `G_K`; hence this also proves that `(PB)` is the unique
noncanonical coloring source in the whole first shell.  The full proof is in the theorem note.

Complete allocation enumeration independently gives a stable finer pattern through `K=5`:

| level(s) | cut orbits over `G_K` | over `X_(K,1)` | over each deeper `X_(K,d)` |
|---|---:|---:|---:|
| `K=3,4,5` | 1 | 4 = 3 inherited + 1 new | 6 = 6 inherited |

This allocation count is finite evidence, not an all-level theorem.  Its recursive interpretation
is useful: a transfer at boundary `d>=2` places the one-unit defect at boundary `d-1` in one child;
the top boundary instead permits the local Pascal reassociation.

## Greedy-source reduction and higher Pascal phases

The one-unit conclusion does not extend to accumulated slack.  There is an exact global reduction
of every possible coloring source to a small family of rigid Pascal recombinations.

For a fixed padded `A/B` coloring, define the integral polymatroid rank

    f(S)=H(|S|)+H(|S intersection A|)+H(|S intersection B|).

Condition (C) says exactly that the colored row demands form an integer base of `f`.  Repeatedly
make any feasible headward exchange from a no-smaller row to a no-larger row.  The sum of squares
strictly increases.  When no exchange remains, the polymatroid greedy criterion says, with
`h=G_(K-1)`, that row `i` has size

    h_i+h_(r_i),

where `r_i` is its occurrence number within its color.  The sums must occur in nonincreasing
order.  Exactly `2^(K-1)` occurrences of each color are positive, so the terminal has `2^K`
rows.  Conversely, every balanced word with nonincreasing sums has the explicit allocation

    A row i: (h_(r_i),h_i,0),       B row i: (0,h_i,h_(r_i)),

and hence three children equal to `h`.  Reversing the ascent proves:

> Every feasible coloring descends by literal color-preserving transfers from a self-sorted
> Pascal greedy shuffle.  In particular every coloring-source orbit is one of these shuffles.

This makes the proposed parent/decomposition isomorphism exact at phase anchors.  A dynamic
program enumerates the shuffles without enumerating parent partitions.  Greedy tight prefixes
forbid a predecessor using unequal target rows; testing all equal-row separations by (C) then
classifies the genuine sources exactly.

| `K` | greedy-shuffle orbits | source orbits | source-area multiplicities |
|---:|---:|---:|---|
| 2 | 2 | 1 | `0` |
| 3 | 2 | 2 | `0, 1` |
| 4 | 5 | 2 | `0, 1` |
| 5 | 6 | 6 | `0, 1, 4^2, 5, 19` |
| 6 | 25 | 6 | `0, 1, 5^2, 6, 23` |
| 7 | 25 | 25 | `0, 1, 6^2, 7, 15^2, 16^2, 21^4, 22^2, 27, 42, 60^2, 61^2, 66^2, 67, 87` |
| 8 | 227 | 30 | `0, 1, 7^2, 8, 21^2, 22^2, 28^4, 29^2, 31, 52, 84^2, 85^2, 91^2, 92, 115, 224^2, 225, 241, 427` |

The first higher family has a closed form.  At dyadic boundary `s=2^(e-1)`, the price of one
color-count imbalance is

    c_e=h_s-h_(s+1)=binomial(K-1,e-1).

Move `c_e` coins between the adjacent `v_e` and `v_(e+1)` boundary rows of `G_K`.  The new row gap
is `binomial(K-1,e)-binomial(K-1,e-1)`.  At `K=2e` it is zero, and the would-be phase inherits by
separating those equal rows.  For `K>=2e+1` it is positive and two self-sorted shuffles give source
colorings.  Thus the `e=2` phases first occur at `K=5`, area four, and the `e=3` phases first occur
at `K=7`, area fifteen.  The theorem note gives the two explicit block words and proof.

At `K=5`, the two area-four sources lie over

    (32,31,26,22,20,16^3,6^8,1^16).

An independent exhaustive ideal through `D=5` confirms the greedy classification locally:

| quantity | `K=5`, `D<=5` |
|---|---:|
| parent states | 267 |
| parent transfers | 866 |
| coloring orbits | 5,089 |
| literal links | 19,113 |
| nonempty parent edges | 866 |
| noncanonical sources | 4: areas `1,4,4,5` |
| parents reached from the canonical coloring | 267 |

The area-nineteen source lies outside this ideal but is classified globally by the greedy-source
theorem and exact predecessor test.  Restricting to dominance-cover edges is insufficient even
inside `D<=5`: that sub-DAG reaches only 266 of the 267 parents.  The missed parent is

    (32,31,26,25,16^4,7,6^7,1^16).

It is the direct transfer of one coin from the last width-26 row of `G_5` to the first width-6
row, hence has area five.  The long edge carries a canonical coloring, but no chain of area-one
literal transports from the canonical source reaches any coloring above it.  Long one-coin moves
are therefore not merely shortcuts in the solution relation.

The now-refuted remaining statement was the **Pascal-Shuffle Coverage Conjecture**: the parent projections of the
downward exchange cones from all self-sorted shuffles cover every partition dominated by `G_K`.
By the reduction above this was equivalent to the Row-Coloring conjecture.  It is a much sharper version
of the original phase-change idea, but the `K=6` hole now refutes it universally.

## Exact downward-closed `K=4` neighborhood

Padding to 81 rows, `D(x)` is also

    D(x)=sum_i (i-1)(x_i-(G_4)_i).

Every normalized Robin--Hood move strictly increases it.  Therefore `{x:D(x)<=B}` is a genuine
order ideal: all predecessors of every surveyed fiber vertex are present.  Source claims inside
the ideal cannot be artifacts of truncation.

The complete `B=14` result is:

| quantity | coloring relation | complete-allocation relation |
|---|---:|---:|
| parent states | 2,852 | 2,852 |
| parent transfers | 26,067 | 26,067 |
| fiber orbits | 60,486 | 871,752 |
| literal transport links | 719,077 | 9,969,849 |
| nonempty parent edges | 26,067 | 26,067 |
| fiber orbits reached canonically | 52,728 | 784,351 |
| parent states reached canonically | 2,852 | 2,852 |
| noncanonical source orbits | 1 | 1 |

In both relations the sole extra source is `(PB)`.  In the allocation relation its descendant set
contains 434,873 cuts.  It covers all 87,401 cuts missed by the canonical component and overlaps
that component on 347,472.  Already at area two an edge out of `G_4` creates a cut relative to that
one predecessor, but an alternate incoming edge inherits it.  This is the first concrete reason
the parent graph must be treated as a DAG rather than a selected transfer tree.

The result is exact only for `D<=14`.  It proves neither that there are no later phase sources nor
that the canonical component hits every one of the 5,997,038 `K=4` parents.  It does show that
multiple interacting unit defects produce extensive birth/death churn without producing a second
new source anywhere in this complete initial ideal.

## Bidirectional collapse of the coloring phases

Making every legal coloring-transport link undirected removes the source obstruction completely
in the full graph.  This has an all-level proof, not merely a census explanation.  For a fixed
coloring `E=A disjoint-union B`, the feasible padded demand vectors are the integer bases of

    f_A(S)=H(|S|)+H(|S intersection A|)+H(|S intersection B|).

A full-mass base forces at least `2^(K-1)` row slots of each color.  Put the canonical rows of each
color into any that many slots and put zero in the rest.  This gives a colored permutation of
`G_K` in the same base polytope.  The integral-polymatroid exchange theorem connects any two
integer bases by color-preserving unit exchanges.  Every intermediate remains a majorized parent,
because for `|S|=t` and `p=|S intersection A|`, concavity gives

    f_A(S) <= H(t)+H(ceil(t/2))+H(floor(t/2))=H_K(t).

Hence every **existing** Hall coloring connects bidirectionally to the canonical coloring after
normalizing rows.  The quotient retains self-links induced by a move `d->d-1`: labelled rows swap
their adjacent widths, so the sorted parent is unchanged but its coloring can change.  The special
`1->0` move similarly transfers a unit to a padded zero slot.  The theorem permits the Hall
allocation to be rebuilt at each vertex; it is not a theorem about the stronger
complete-allocation transport graph.

The new `UNDIRECTED_GRAPH` output checks the predicted quotient connectivity:

| corpus | parents | coloring orbits | internal links | components | canonical component | parent projection |
|---|---:|---:|---:|---:|---:|---:|
| complete `K=2` | 15 | 42 | 34 | 1 | 42 | 15 / 15 |
| complete `K=3` | 1,206 | 31,498 | 54,211 | 1 | 31,498 | 1,206 / 1,206 |
| `K=4`, `D<=14` | 2,852 | 60,486 | 75,100 | 1 | 60,486 | 2,852 / 2,852 |
| `K=5`, `D<=5` | 267 | 5,089 | 8,687 | 2 | 5,088 | 267 / 267 |

The second `K=5` component is a single orbit: the unique source at area five has no incoming link,
and every outgoing edge leaves the truncated ideal.  The all-level theorem says it reconnects by
a detour at larger area in the full graph.  The smaller Pascal phase connects to an inherited
coloring over the same parent in one move:

    A=(8,6,1,1), B=(5,4,1,1) --6->5--> A=(8,5,1,1), B=(6,4,1,1).

The widths swap, so the normalized parent stays `(8,6,5,4,1^4)`.  If such normalized self-moves
are suppressed, the explicit connection is instead the three-edge detour:

    (8,7,4,4,1^4) --8->4--> (7,7,5,4,1^4)
                     --7->5--> (7,6,6,4,1^4)
             --reverse 6->7--> (8,6,5,4,1^4),

whose endpoint is exactly that directed (PB) source.

This is a simplification, but not the converse.  If `F_K` is the union of all fixed-color integer
bases and `B_K` is the integer permutahedron majorized by `G_K`, then `F_K subset B_K`, every
permutation of `G_K` lies in `F_K`, and therefore `conv(F_K)=conv(B_K)`.  Row-Coloring is exactly
the assertion that this labelled, vertex-spanning set has no missing lattice points.  Separately,
the theorem proves that its normalized colored graph is connected.  The generic counterexample
`h=(6,1)`, `a=(12,3,3,3)` shows why these two facts cannot fill a hole.

Bidirectionality therefore supersedes phase enumeration as a necessary proof mechanism.  The
surviving clean target is to show that the primitive integer set `F_K` is M-convex.  The initially
proposed real fixed-color cover is now refuted, first in padded form at `K=2` and then at exact
support at every `K>=3`; see the
[real-cover counterexample record](singleton_exact_support_real_cover_2026-08-30.md).  Thus
M-convexity is an integer-lattice assertion here, not a consequence of convexity of the real
union.

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
reachable fibers of the predecessors.  The greedy reduction now classifies every possible source
of the coarser Hall-coloring relation as a rigid canonical-child shuffle, and proves that higher
sources occur.  A proof should therefore establish Pascal-Shuffle Coverage, or directly show that
some incoming relation always meets a reachable predecessor fiber.  It must allow long unit moves:
the `K=5`, `D<=5` cover sub-DAG already misses one parent.  This retains the whole fiber and avoids
the earlier mistake of selecting incompatible edge certificates independently.

## Reproduction and scope

```text
tools/singleton_solution_fiber_dag.py 1
tools/singleton_solution_fiber_dag.py 2
tools/singleton_solution_fiber_dag.py 3
tools/singleton_solution_fiber_dag.py --phase-edge 3
tools/singleton_solution_fiber_dag.py --phase-edge 4
tools/singleton_solution_fiber_dag.py --phase-edge 5
tools/singleton_solution_fiber_dag.py --area-ideal 4 14 --examples 20
tools/singleton_solution_fiber_dag.py --area-ideal 5 5 --examples 30
for k in 2 3 4 5 6 7 8; do
  tools/singleton_solution_fiber_dag.py --greedy-sources "$k" --examples 0
done

CC=clang++ tools/build_radio.py -std=c++20 -O3 -Wall -Wextra -pedantic \
  tools/singleton_allocation_fiber_dag.cpp \
  -o /tmp/singleton_allocation_fiber_dag

tools/capped_run.sh --seconds 1800 --rss-gb 4 \
  --label singleton-allocation-fiber-k3 -- \
  tools/run_with_provenance.py /tmp/singleton_allocation_fiber_dag 3 0

tools/capped_run.sh --seconds 300 --rss-gb 4 \
  --label singleton-allocation-area-k4-d14 -- \
  tools/run_with_provenance.py /tmp/singleton_allocation_fiber_dag \
    --area 4 14 3

tools/run_with_provenance.py /tmp/singleton_allocation_fiber_dag \
  --edge 5 31 26 32 31 26 26 16 16 16 16 6 6 6 6 6 6 6 6 \
    1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
```

The final optimized allocation build id is
`61e923d2a94e106f8ccd7f50ad7ba6a01b1da1f57fdc8b230ff4708ebf55027a`, from clean commit
`83d87e66aad25ed30d030798c6d93def87af3095`.  It reproduces the complete `K=3` relation in
48.8522 in-process / 50 wrapper wall seconds at 0.10 GB peak RSS and the `K=4`, `D<=14` ideal in
14.299 / 16 seconds at 0.10 GB.  Both retained temporary logs pass `tools/check_provenance.py`.
Clean address/undefined-sanitized build
`cf5df2f2ff2dc72d6becbcb44b5e678b99a863216b67cd2dba687cadd9f4df5f` reproduces the complete
area-14 allocation result without a diagnostic in 121.67 in-process / 127 wrapper wall seconds at
0.69 GB peak RSS; its log also passes the provenance checker.

The Python source at the same clean commit has SHA-256
`b9507bf6b18ef7616bb771f78670f1504fc221e95165d3582639e4268c30563b`.  Its clean `K=4`, `D<=14`
coloring run took 61 wrapper wall seconds at 0.03 GB, and its `K=5`, `D<=5` run took 218 seconds at
0.02 GB.  Running all seven greedy-source classifications `K=2,...,8` from that source took 7.7
wall seconds in one sequential shell loop.

The bidirectional-component extension has Python source SHA-256
`1b09af6c4c35e528346a673d2edf72ed2e881b8c53ddefa90d2fdc490c50d8c3`.  Direct exact reruns took
4.84 wall seconds for complete `K=3`, 63.87 seconds for `K=4`, `D<=14`, and 223.21 seconds for
`K=5`, `D<=5`.  They produced the component and internal-link counts above.

The complete allocation-DAG claims stop at `K=3`; the allocation area ideal stops at `K=4`,
`D=14`.  The Hall-coloring ideals stop at `K=4`, `D=14` and `K=5`, `D=5`.  The
Greedy-Source Reduction and the dyadic phase family are all-level theorems, while the complete
greedy-source classifications stop at `K=8`.  None of these results proves Pascal-Shuffle
Coverage, the Canonical Allocation-Transport Conjecture or the Singleton Majorization Converse.
