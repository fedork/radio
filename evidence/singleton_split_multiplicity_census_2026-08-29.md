# Singleton split multiplicity through `K=4` (2026-08-29)

## Scope and definitions

For a full-mass parent `a<=_w G_K`, a **child orbit** is a normalized child triple `(L,M,R)`
modulo the genuine `L<->R` symmetry; the mixed child remains distinguished.  An **allocation
orbit** is a normalized multiset of row triples `(l_i,m_i,r_i)`, again modulo `L<->R` and
permutations of equal parent rows.  Every row obeys `l_i r_i=0` and every child has mass
`3^(K-1)` and is weakly majorized by `G_(K-1)`.

The earlier words **child-unique** and **cut-unique** mean respectively one child orbit and one
allocation orbit.

## Exact method

`tools/singleton_split_multiplicity_census.cpp` works parent-first.  For a parent row of value `a`
it enumerates exactly the row choices

    (p,a-p,0) or (0,a-p,p).

Choices on equal parent rows are nondecreasing, which quotients their permutations.  Each child is
stored by its value multiplicities.  A partial branch is rejected as soon as one child exceeds a
prefix of `G_(K-1)` or its required mass.  At a leaf, the program canonicalizes `L<->R` and stores
both the child triple and the row-allocation multiset.

The production census uses an orbit limit of four.  It stops a parent when a fourth child orbit is
found.  Consequently the child counts one, two and three are exact; the remaining bucket is
`>=4`.  Parents in the first three buckets are exhausted completely, so their allocation counts
one, two and three are exact as well.

As controls, the compiled implementation reproduces the independent Python partial-matching
census at every `K<=3`, including the complete forced triples and cut counts.  Its `K=4` parent
enumerator independently reproduces the previously established 5,997,038-state full-mass corpus.

## Exact multiplicity layers

| `K` | all parents | 1 child orbit | 2 | 3 | `>=4` | 1 allocation orbit | 2 | 3 |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 2 | 2 | 0 | 0 | 0 | 2 | 0 | 0 |
| 2 | 15 | 4 | 5 | 2 | 4 | 3 | 3 | 4 |
| 3 | 1,206 | 9 | 19 | 6 | 1,172 | 6 | 4 | 8 |
| 4 | 5,997,038 | 30 | 123 | 106 | 5,996,779 | 8 | 19 | 32 |

Thus the `K=4` small-child-multiplicity corpus has only 259 parents.  It is a very thin boundary
stratum: about 0.00432% of the full corpus.  The fully cut-unique corpus has eight members.

The complete `K=4` run visited 1,765,546,548 row-allocation search nodes and 30,162,788 complete
allocations.  On an Apple M4 Pro it took 96.5534 seconds of in-process elapsed time under a one-hour,
4-GiB capped wrapper; the wrapper reported 101 wall seconds and exit zero.  The retained temporary
log passed `tools/check_provenance.py` with build id
`abc2dd99be783536c9d20bd91f765246016b84c92aa009d8b5c33f82a55d025b`.  The log is small and
reproducible, so it was not archived as a release artifact.

## Strict rigidity heredity is false at `K=4`

The `K<=3` corpus suggested that a child-unique parent must have three child-unique children and a
repeated child.  Both conclusions fail at `K=4`.

A single clean counterexample is

    a=(16,15,9^3,5^3,1^8).

It has one child orbit, namely

    (8,6,5,4,1^4), (8,7,4,3,2,1^3), G_3,

and two allocation orbits.  The three children are distinct.  The first two children each have
exactly two child orbits at `K=3`, while only `G_3` is child-unique.  Thus the former
Rigidity-Heredity Conjecture and its forced-repetition clause are both refuted.

The failure is controlled rather than arbitrary.  Among the 30 child-unique `K=4` parents, the
number of rigid `K=3` children in the forced triple is distributed as follows:

| rigid children | child-unique `K=4` parents |
|---:|---:|
| 1 | 5 |
| 2 | 15 |
| 3 | 10 |

Exactly 13 of the 30 forced triples repeat a child, so 17 are all-distinct.  Nevertheless, every
forced child belongs to the one- or two-orbit `K=3` layer, and every forced triple contains at
least one rigid child.  This motivates the corrected finite pattern

    parent multiplicity 1  =>  every child multiplicity <=2 and some child multiplicity =1.

It is verified only through `K=4`; call it the **Multiplicity-Filtration Conjecture** if pursued.

There is also a conceptual reason strict heredity was too strong.  Parent child-uniqueness fixes
the three child **shapes at the first split**.  Whether one of those shapes has one or several
splits of its own is a question one level lower and does not manufacture a second first split of
the parent.  The `K<=3` inheritance was therefore additional Pascal boundary structure, not a
formal consequence of unique factorization.  A literal recursive isomorphism would have to retain
decorated child decomposition trees, not only the three child partitions and the current weighted
matching.

The eight fully cut-unique `K=4` parents are especially structured:

| parent | forced children |
|---|---|
| `G_4=(16,15,11^2,5^4,1^8)` | `(G_3,G_3,G_3)` |
| `(16,15,11^2,1^28)` | `((8,7,1^12),G_3,(8,7,1^12))` |
| `(16,15,1^50)` | `((8,1^19),(8,7,1^12),(8,1^19))` |
| `(16,1^65)` | `((1^27),(8,1^19),(8,1^19))` |
| `(14^3,11,5^4,1^8)` | `(G_3,(7,6,6,4,1^4),G_3)` |
| `(14^3,11,1^28)` | `((8,7,1^12),(7,6,6,4,1^4),(8,7,1^12))` |
| `(14^3,1^39)` | `((8,1^19),(7,6,6,1^8),(8,7,1^12))` |
| `(1^81)` | `((1^27),(1^27),(1^27))` |

They form two short Pascal-headed arms, beginning with `16` and with `14^3`, plus the all-unit
endpoint.  This is a useful description of the singular boundary, not a proved recursive
classification.

## What the two- and three-orbit layers add

For a parent in an exact low layer, minimize over its child triples the largest child
multiplicity.  Child multiplicity `>=4` is retained as one capped class.  At `K=4` the exact result
is:

| parent child orbits | admits three rigid children | best maximum child multiplicity 2 | 3 | `>=4` |
|---:|---:|---:|---:|---:|
| 1 | 10 | 20 | 0 | 0 |
| 2 | 40 | 80 | 3 | 0 |
| 3 | 20 | 78 | 6 | 2 |

The columns are exclusive: for example the middle entry 80 counts parents whose best triple has
maximum child multiplicity exactly two, not at most two.

This is the main reason the expanded survey is useful.  The singular layer alone falsely suggests
strict heredity.  The next layer reveals that rigidity normally bifurcates into two child orbits,
and it supplies a plausible filtration for induction.  It is still a structural microscope, not a
proof route for the converse: 5,996,779 `K=4` parents lie outside these layers.

The subsequent complete parent--solution analysis identifies an exact dyadic tight-prefix product
inside this microscope; see the
[low-multiplicity factorization record](singleton_low_multiplicity_factorization_2026-08-29.md).

## Rigid children are not universal

The separate exact `K=3` survey in `tools/singleton_rigid_child_survey.py` asks for the largest
number of rigid `K=2` children available in any split.  Among all 1,206 parents the counts for zero,
one, two and three rigid children are respectively

    1, 60, 331, 814.

The unique parent admitting no rigid child at all is

    (3^9).

There is also a short proof.  Three cannot produce a child part four, so the only possible rigid
child is `(1^9)`.  If it is the mixed child, every parent row leaves a pure remainder two; the two
pure children would each need odd mass nine from parts of size two.  If `(1^9)` is a pure child,
it uses all nine parent rows, and legality prevents the opposite pure child from receiving
anything.  Both cases are impossible.

Therefore neither “at least one rigid child” nor “at least two rigid children” can hold for every
parent.  The weaker statement that a **child-unique** parent has at least one rigid child survives
through `K=4`.

## Shape signal and its limit

At `K=3`, all nine child-unique parents have at most one distinct majorization-preserving
one-unit move toward the head.  This confirms that they lie on a thin boundary spine.  It is not a
characterization: 14 parents have at most one such upward neighbor, and five of those are
non-rigid.

The 19 two-orbit parents have graph-distance distribution `15` at distance one and `4` at distance
two from the rigid set in the unit-transfer graph.  The six three-orbit parents distribute as
`4,1,1` at distances one, two and three.  Thus the near-rigid corpus is mostly, but not entirely,
the immediate shell.  Neither prefix tightness nor one-unit branching alone separates rigidity.
The recursive multiplicity of the forced children is the sharper signal exposed by `K=4`.

## Reproduction

```text
CC=clang++ tools/build_radio.py -std=c++20 -O3 \
  tools/singleton_split_multiplicity_census.cpp \
  -o /tmp/singleton_split_multiplicity_census

tools/run_with_provenance.py \
  /tmp/singleton_split_multiplicity_census 3 0 0 4

tools/capped_run.sh --seconds 3600 --rss-gb 4 --label singleton-k4-layered -- \
  tools/run_with_provenance.py \
  /tmp/singleton_split_multiplicity_census 4 0 0 4

tools/singleton_rigid_child_survey.py
```
