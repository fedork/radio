# Singleton Row-Coloring finite census (2026-08-26)

## Question and exact test

For `h=G_(K-1)` let `H` be its saturated prefix function.  A coloring of a sorted
parent partition `a` into subsequences `A,B` is legal exactly when

    A_p + B_q <= H(p+q) + H(p) + H(q)                 for every p,q.       (C)

It is enough to test full mass `sum(a)=3^K`: append artificial unit rows to an
underfull state and delete them after coloring.  The utility
`tools/singleton_pair_coloring_census.cpp` recursively enumerates every full-mass
integer partition weakly majorized by `G_K`, quotients colorings by permutations
of equal rows, and checks (C) directly.  It is provenance-built and run as

    CC=clang++ tools/build_radio.py -O3 -std=c++20 -Wall -Wextra -pedantic \
        tools/singleton_pair_coloring_census.cpp -o /tmp/singleton-pair-census
    tools/run_with_provenance.py /tmp/singleton-pair-census 4

The independent completion-count recursion used by `--uniform` returns the same
`k=4` universe size as exhaustive generation.

## Complete results

| level | full-mass dominated partitions | unrestricted coloring | adjacent-pair coloring |
|---|---:|---:|---:|
| `K=3` | 1,206 | 1,206 | 1,206 |
| `K=4` | 5,997,038 | 5,997,038 | 5,996,122 |

Thus the Row-Coloring Lemma is computationally verified through `K=4`, but the
stronger rule that every adjacent pair can be split between the colors is false.
Its first `K=4` counterexample is

    (16,15,11,11,5,5,5,1^13).

It has unrestricted colorings, for example

    A=(16,11,5,5,1,1,1,1),
    B=(15,11,5,1,1,1,1,1,1,1,1,1).

Every adjacent-pair coloring distributes the final twelve unit rows evenly; the
displayed legal coloring must distribute those tail rows unevenly.  This is why
adding the artificial unit rows does not rescue alternation, although deleting
them before reasoning can conceal the obstruction.

The exact normalized search was unexpectedly shallow.  Across all 5,997,038
`K=4` states it visited 47,389,246 recursive nodes, versus 47,389,224 nodes for
one direct block path per state.  Only 22 states needed one failed branch; no
state needed a longer repair.  This observation led to the forward rules below.

## Rules tested

Write the distinct row widths as `v_1>...>v_s`, with multiplicities `m_j`, and
let `x_j` be the number of the `m_j` equal rows colored A.  Treating a value block
at once is not a claim that equal rows have different combinatorics from nearby
rows: `(x_j)` is exactly the normalization of all row colorings by permutations
of indistinguishable rows.

The most successful forward rule was:

1. put a largest row in A to fix A/B complementation;
2. process value blocks in decreasing order;
3. among allocations of the current block that preserve every currently exposed
   inequality (C), prefer the allocation minimizing the current total-mass
   difference (ties favor A);
4. before committing, require that the next lower value block has at least one
   allocation preserving (C).

This one-block-lookahead rule uses no recoloring or exchange.  It succeeds on all
5,997,038 `K=4` states.  Plain block balancing succeeds on 5,997,016 and fails on
22.  Merely choosing the minimum-A or maximum-A one-block-extendable allocation
is not enough: those variants succeed on 5,973,904 and 5,985,556 states,
respectively.  Thus local extendability alone is not the observed invariant; the
central/balanced choice matters.

A simpler attempt reserves enough uncolored rows to bring both final colors to
`|G_(K-1)|=2^(K-1)` rows.  Full mass makes that cardinality necessary because
`H(p)` reaches `3^(K-1)` only at the last row of `G_(K-1)`.  This rule succeeds on
the complete `K=4` census, explaining all 22 plain-block failures there, but it is
not universal at `K=5`.

## Higher-level stress tests, not proofs

Dynamic programming counts exactly

    38,378,683,542,323

full-mass partitions weakly majorized by `G_5`.  The `--uniform` mode samples this
finite universe with exact completion-count weights.  On 10,000,000 independent
samples (seed `314159265`):

| rule | successes |
|---|---:|
| plain balanced value blocks | 9,999,988 / 10,000,000 |
| balanced blocks plus final-row reservation | 9,999,994 / 10,000,000 |
| balanced blocks plus one-block lookahead | 10,000,000 / 10,000,000 |

The last line is evidence for a conjecture, not a proof and not an exhaustive
`K=5` result.  A first counterexample to the row-reservation shortcut is

    (31,29,23,22,21,21,12,12,12,7,7,6,5,5,5,5,5,1^15).

The shortcut reaches

    A=(31,22,21,12,7,7),
    B=(29,23,21,12,12,6)

and no allocation of the five rows of width 5 preserves (C): the two most balanced
allocations already fail at `(p,q)=(9,5)` and `(6,9)` by one.  One-block lookahead
instead reverses the preceding width-6 row.  An exact completed coloring is

    A=(31,22,21,12,7,7,6,5,1,1,1,1,1,1,1,1),
    B=(29,23,21,12,12,5,5,5,5,1,1,1,1,1,1,1).

A separate dominance-transfer walk tested 100,000 `K=6` states (seed
`271828182`); plain block balancing and one-block lookahead both succeeded.  This
sample is correlated and is only a targeted dominance-cover stress test.

## Proof target exposed by the census

The finite data suggest the following **Block-Extension Conjecture**.  For
`h=G_(K-1)`, the balanced one-block-lookahead rule above never gets stuck on a
full-mass `a <=_w G_K`.  A proof needs two statements:

1. at every block there is a locally legal allocation that admits the next block;
2. choosing the most mass-balanced such allocation preserves statement 1 for the
   following step.

Both must use the Pascal/dyadic structure of `G_(K-1)`.  They are false for the
generic bases already recorded in the theorem note.  The census suggests studying
the projection of the normalized feasible-coloring set onto two consecutive block
counts `(x_j,x_(j+1))`; a discrete-convex or interval theorem for those projections
would explain the observed absence of longer backtracking.

One tempting graph-recursion shortcut also fails.  Although

    Q_K = Q_(K-1) disjoint-union (Q_(K-1) join Q_(K-1)),

the self-join need not itself be nice.  Already for shape `g=(2,1)`, the partition
`(2,2,2)` is dominated by two sorted copies `(2,2,1,1)` but cannot be divided into
two subsequences each dominated by `(2,1)`.  The middle component is therefore not
separable from the two pure components; that failed factorization is the original
mixed-child difficulty in another form.

No general proof follows from this census.  The Singleton Majorization converse and
the Row-Coloring Lemma remain open.
