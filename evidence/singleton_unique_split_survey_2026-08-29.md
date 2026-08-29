# Unique singleton split survey (`K<=3`, 2026-08-29)

## Definitions

A full-mass parent has a **child-unique** split if exactly one normalized ordered child-type triple
`(L,M,R)` can produce it, modulo the genuine outcome symmetry `L<->R`.  The mixed child remains
distinguished.

It is **cut-unique** if, in addition, that child triple has exactly one normalized multiset of row
triples `(l_i,m_i,r_i)`, modulo equal parent rows, equal child parts, and `L<->R`.

Thus cut uniqueness is the finer notion.  Neither definition counts the possibly multiple
transcript-chain realizations internal to a fixed child state.

## Exact method

`tools/singleton_unique_split_survey.py` uses the exact partial-matching formulation from the
identical-child census.  For every ordered triple of full-mass child partitions, each mixed part is
left alone or matched to one part from the disjoint pure multiset `L union R`.  It indexes every
resulting parent by its child-type orbit.  Only parents with one such orbit proceed to a second
enumeration of normalized row triples.

The parent corpus is independently enumerated from majorization by `G_K`.  The survey is exhaustive
through `K=3`.

| level | all parent types | child-unique | cut-unique | distinct triples used by child-unique parents |
|---:|---:|---:|---:|---:|
| 1 | 2 | 2 | 2 | 1 |
| 2 | 15 | 4 | 3 | 3 |
| 3 | 1,206 | 9 | 6 | 8 |

## The complete `K=3` rigid corpus

Name the four child-unique `K=2` types

    A=(1^9),       B=(4,1^5),       C=(4,2,2,1),       D=(4,3,1,1)=G_2.

Then the entire child-unique `K=3` corpus is:

| parent | unique child triple representative | normalized cut orbits |
|---|---|---:|
| `(1^27)` | `(A,A,A)` | 1 |
| `(8,1^19)` | `(A,B,B)` | 1 |
| `(8,5,5,5,1^4)` | `(D,C,D)` | 1 |
| `(8,7,1^12)` | `(B,D,B)` | 1 |
| `(8,7,2^6)` | `(C,D,C)` | 1 |
| `(8,7,3,3,3,1^3)` | `(C,D,D)` | 2 |
| `(8,7,4,1^8)` | `(B,D,D)` | 2 |
| `(8,7,4,2,2,2,1,1)` | `(C,D,D)` | 4 |
| `(8,7,4,4,1^4)=G_3` | `(D,D,D)` | 1 |

The six rows with cut count one are the requested completely singular corpus.

For example, the non-extreme state `(8,5,5,5,1^4)` has the unique cut

    (0,4,4), (4,1,0), (3,2,0), (0,2,3),
    (1,0,0), (1,0,0), (0,0,1), (0,0,1),

up to the stated symmetries.  Its children are `(D,C,D)`.  This is a particularly useful rigid
interior example: it is neither `G_3` nor the all-unit state, yet both its children and its cut are
forced.

## Structural signal

Three facts survive after removing multiplicity:

1. **Hereditary child rigidity.** Every child appearing in a child-unique `K=3` parent is one of
   the four child-unique `K=2` states `A,B,C,D`.
2. **Forced repetition.** Every unique child triple has at least two identical children.  The
   identical-child statement is false for arbitrary parents, but it is exact on the rigid corpus.
3. **A small ambiguity fiber.** Child types alone are not quite an isomorphism.  The same triple
   `(C,D,D)` produces two child-unique parents, with respectively two and four normalized cuts.
   This is precisely where the child map loses injectivity.  At `K=2` the analogous triple
   `((2,1),(2,1),(2,1))` produces both `(4,2,2,1)` and `G_2`; the first has two cuts and the second
   one.

The cut-unique corpus removes these ambiguous representations.  Its six members are therefore the
cleanest finite candidates for a parent/children correspondence, although the survey does not yet
supply a formula reconstructing the parent from the children.

The partial-matching model identifies the missing coordinate precisely.  A cut is a weighted
matching from mixed-child parts to pure-child parts; its matched edge weights are sums and its
unmatched vertex weights remain parent rows.  Thus `(children,matching)` always reconstructs the
parent.  On the six cut-unique states the converse reconstruction is unique as well.  The most
literal prospective “isomorphism” is therefore unique factorization of the parent into a rigid
child triple plus this weighted matching, not an identification with the three child partitions
alone.

This motivates a **Rigidity-Heredity Conjecture**: if a parent has a unique normalized child-type
orbit, then all three children are themselves child-unique and at least two of them coincide.  It
holds through `K=3`.  A proof would reduce candidate rigid `K=4` triples from all `1206^3` ordered
triples to the `9^3` triples on the rigid spine, but using that reduction in an exhaustive proof
before proving heredity would be circular.

## Why `K=4` needs a different census

A direct extension would inspect 1,754,049,816 ordered triples of the 1,206 `K=3` child types.
That is the wrong enumeration.  Since only uniqueness matters, a `K=4` implementation should work
parent-first and stop as soon as it finds a second child-type orbit; only the rare survivors need
complete row-allocation counting.  The existing `K=4` corpus has 5,997,038 parents, so a
stop-after-two compiled search is plausible, while the naive triple product is not.

## Reproduction

From the repository root:

```text
tools/singleton_unique_split_survey.py
```

The dependency-free exact `K<=3` run takes about 3.3 seconds locally.
