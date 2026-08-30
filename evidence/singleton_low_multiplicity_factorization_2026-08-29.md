# Low-multiplicity parent--solution factorization (2026-08-29)

## Scope

This record examines the complete `K=4` corpus with one, two or three normalized child-shape
orbits.  There are 259 such parents and 594 parent--child orbits.  Their cuts have 1,741 normalized
row-allocation orbits.  The source is the exact parent-first census with orbit limit four, so no
state in these three layers is truncated.

The main outcome is not a sorted assignment rule.  It is an exact factorization at saturated
dyadic Pascal prefixes.

## Dyadic tight-prefix saturation

Let `h=G_(K-1)` and let `H` be its saturated prefix-sum function.  Recall

    H_K(t)=H(t)+H(ceil(t/2))+H(floor(t/2)).

Consider a legal split of a sorted parent `a<=_w G_K`, and suppose its first `t` rows are tight:

    sum_(i<=t) a_i = H_K(t).                                  (TP1)

Among these `t` rows let `p` be oriented toward the left pure child and `q=t-p` toward the right.
Their contributions to the left, mixed and right children are subsets of respectively `p,t,q`
child parts.  Majorization therefore bounds their total mass by

    H(p)+H(t)+H(q) <= H_K(t).                                 (TP2)

The left side is exactly the mass in (TP1), so every inequality in (TP2) is equality.  Thus `p`
maximizes `H(p)+H(t-p)`, and the three contribution subsets saturate child prefixes of sizes
`p,t,q`.

Now take `t=2^j` with `j>=1`.  The Pascal profile `h` has a strict drop after `t/2`: its constant
blocks end at powers of two.  Concavity and symmetry consequently make `p=q=t/2` the unique
maximizer, up to the global exchange of the pure sides.  Hence every legal split must give the top
`t` parent rows exactly the child-prefix quotas

    left: h[1:t/2],    mixed: h[1:t],    right: h[1:t/2].      (TP3)

The remaining rows fill the three complementary suffix profiles.  Conversely, any legal head
allocation and legal tail allocation with these exact profiles concatenate to a legal global
allocation.  This is the usual tight-face product of a majorization polymatroid: a vector of mass
`H(s)` dominated by the first `s` capacities, for `s=t/2` or `s=t`, and a vector dominated by the
contracted suffix combine to a vector dominated by `h`.  Row legality is preserved because the
two parent row sets are disjoint.

This proves the **Dyadic Tight-Prefix Factorization Lemma**.  It is special to the Pascal profile:
the strict drop at the dyadic midpoint fixes the pure row counts.  For a nondyadic tight prefix the
maximizer can be nonunique; at `K=4`, for example, `t=6` permits `p=2,3,4`.
The endpoint `t=1` has the analogous forced counts `0,1` up to exchanging the pure sides.

The lemma gives a concrete construction:

1. scan the parent for a tight dyadic prefix;
2. split there into the forced Pascal head and its contracted tail;
3. solve the two smaller, generally asymmetric capacity problems;
4. concatenate their child contributions.

It does not by itself prove the full converse, because the two contracted problems are not both
instances of the original symmetric `G_j` statement.  The correct strengthening must retain the
exact parent-rank interval and the pure-row count at each tight endpoint.  Allowing arbitrary row
refinements of a prefix capacity is false: `(16,15,11,11,4^5)` is dominated by
`(16,15,11,11,5^4)` but cannot fill the corresponding prefix child capacities.  The subsequent
[tight-skeleton record](singleton_pascal_tight_skeleton_2026-08-29.md) proves the general
factorization over all tight ranks, proves that two-smallest-row coalescence reduces the full
problem to exactly `2^K` rows, formulates the remaining count-path extension conjecture, and
verifies it through `K=4`.

## The exact `K=4,t=4` product

For `K=4,t=4`, the parent prefix has mass 53.  The forced child head capacities are

    left=(8,7),  mixed=(8,7,4,4),  right=(8,7),

and the tail capacities are

    left=(4,4,1^4),  mixed=(1^4),  right=(4,4,1^4).

The pure head shapes are forced to `(8,7)`, and the mixed tail is forced to `(1^4)`.  Therefore
the normalized child-shape choices are a literal Cartesian product:

    head H + tail T  ->  ( (8,7)+L(T), M(H)+(1^4), (8,7)+R(T) ).    (TP4)

Here `+` means sorted concatenation, not coordinatewise addition.

Within the low-multiplicity corpus there are eight head states and 32 tail states.  Their local
orbit layers are

| local problem | multiplicity 1 | 2 | 3 |
|---|---:|---:|---:|
| head | 3 | 4 | 1 |
| tail | 7 | 19 | 6 |

Every head--tail pair whose product is at most three occurs, and its full child multiplicity is
the product.  Thus this single factorization accounts for 131 parents:

    multiplicity 1: 3*7             = 21,
    multiplicity 2: 3*19 + 4*7      = 85,
    multiplicity 3: 3*6  + 1*7      = 25.

The 21 rigid parents form the especially visible `3 x 7` rectangle.  For example, the three heads

    (16,15,11,11), (16,13,13,11), (14,14,14,11)

combine independently with the same seven rigid tails.  The tail determines the two pure child
tails, the head determines the mixed child head, and the representative cut preserves this block
separation.  This is the precise isomorphism suggested by the earlier rigid survey.

For example,

    H=(16,13,13,11),  T=(5^3,3^2,1^7)

combine to the parent `(16,13,13,11,5^3,3^2,1^7)`.  The tail supplies pure children
`(8,7,4,3,2,1^3)` and `G_3`, while the head supplies the mixed head `(8,6,5,4)`; appending the
forced mixed tail `(1^4)` gives the unique triple

    (8,7,4,3,2,1^3), (8,6,5,4,1^4), G_3.

Across all 259 parents, 228 have at least one tight prefix at `t=1,2,4,8`.  The counts with and
without such an internal dyadic equality are

| parent multiplicity | with dyadic equality | without |
|---:|---:|---:|
| 1 | 28 | 2 |
| 2 | 116 | 7 |
| 3 | 84 | 22 |

Thus tight Pascal factorization explains most of the small-fiber boundary, but not all of it.
Allowing every internal tight rank, with its full plateau of possible pure-row counts, raises this
coverage to 253/259.  The remaining six states are listed in the tight-skeleton record.

## A recursive pure anchor

Only 48 distinct `K=3` child shapes occur among all 594 solutions.  Their lower multiplicities are

    layer 1: 9,  layer 2: 18,  layer 3: 5,  layer >=4: 16.

More sharply, 258 of the 259 parents admit a solution with a child-unique **pure** child.  By parent
layer, the best available pure-child multiplicity is

| parent multiplicity | pure child 1 | pure child 2 |
|---:|---:|---:|
| 1 | 30 | 0 |
| 2 | 122 | 1 |
| 3 | 106 | 0 |

The sole no-rigid-pure exception is

    (16,15,9^3,5,3^4,1^6).

It has two child orbits and admits a pure child from the two-orbit layer.  This suggests a
**Pure-Anchor Filtration** for small fibers: anchor one recursively simple pure branch, then solve
the binary residual problem on the unanchored rows.  It is a finite `K<=4` pattern, not yet a
theorem.

The anchor is not selected by ordinary best fit.  On `G_4`, assigning the rigid child parts to the
smallest fitting parent rows minimizes residual mass for `(8,5,5,5,1^4)`, but that child does not
occur in the unique solution.  The actual children are all `G_3`.  Pascal alternation, rather than
mass-sorted matching, remains essential.

## Deformation from the rigid spine, and its limit

For two equal-mass sorted partitions, use half their padded `L1` distance as unit-transfer distance.
For every low parent, choose a closest child-unique ancestor in dominance order.  Every one of the
259 parents has a solution which is componentwise dominated by that ancestor's forced child triple
(after the `L<->R` symmetry), and whose total child transfer distance is no greater than the parent
distance.  In this precise finite sense, the parent can be rounded to the nearest rigid spine and
then deformed downward without increasing transfer cost.

The nearest-spine deformation accounts for 592 of all 594 solution orbits.  The two extra orbits
occur at the unit-heavy parents `(16,14,1^51)` and `(15,15,1^51)`.

This observation does not give a unit-by-unit proof inside the small-fiber corpus.  In the directed
graph of the 259 parents, with one Robin--Hood transfer per edge, only 176 are reachable from the
30 rigid parents while staying in the corpus.  The other 83 require paths through states having at
least four solutions.  Therefore a universal transfer construction must tolerate a temporary
increase of multiplicity; the rigid boundary is not closed under the required moves.

## Reproduction

Build and export the exact low layers:

```text
CC=clang++ tools/build_radio.py -std=c++20 -O3 \
  tools/singleton_split_multiplicity_census.cpp \
  -o /tmp/singleton_split_multiplicity_census

tools/run_with_provenance.py \
  /tmp/singleton_split_multiplicity_census 3 0 0 4 1 \
  > /tmp/singleton_k3_cuts.log 2>&1

tools/capped_run.sh --seconds 1800 --rss-gb 4 --label singleton-k4-cuts -- \
  tools/run_with_provenance.py \
  /tmp/singleton_split_multiplicity_census 4 0 0 4 1 \
  > /tmp/singleton_k4_cuts.log 2>&1

tools/check_provenance.py /tmp/singleton_k3_cuts.log
tools/check_provenance.py /tmp/singleton_k4_cuts.log
tools/singleton_low_multiplicity_analysis.py \
  /tmp/singleton_k3_cuts.log /tmp/singleton_k4_cuts.log
```

The recorded complete run used build id
`b3ad4bc8a693a00ca9cbebfa6cbd8bb9e440122660a3626277b62b6ac92369fb`, visited the same
1,765,546,548 search nodes and 30,162,788 complete allocations as the original census, and took
97.7258 in-process seconds / 101 wrapper wall seconds under the 30-minute, 4-GiB cap.  Both logs
passed provenance checking.  No process remains.
