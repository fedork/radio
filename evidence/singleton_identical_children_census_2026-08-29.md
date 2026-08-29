# Identical-child split census (`K<=3`, 2026-08-29)

## Proposed strengthening

For every full-mass singleton state `a<=_w G_K`, ask for a legal first-test split whose three
normalized child sequences `L,M,R` contain an identical pair.  Equality here is only after sorting
and deleting zero rows.  This is weaker than labelled equality and is the interpretation most
favorable to the proposal.

The statement is true through `K=2` but false at `K=3`.

## Exact method

For fixed normalized child types `L,M,R`, legal row recombination has a simple exact description.
Put all pure-child parts in the disjoint multiset `L union R`.  Each mixed-child part may be
matched to at most one pure part, in which case their values add to form one parent row; every
unmatched part forms its own row.  This is precisely the condition that a row may use
`{left,mixed}` or `{mixed,right}`, but not both pure children.  Row labels can then be restored
arbitrarily, so no normalized recombinations are lost.

`tools/singleton_identical_children_census.py` enumerates every majorized child type, every ordered
child triple, and every such partial matching.  It compares the resulting support with an
independent enumeration of all full-mass parent partitions dominated by `G_K`.

| level | parent types | types having an equal-child split |
|---:|---:|---:|
| 1 | 2 | 2 |
| 2 | 15 | 15 |
| 3 | 1,206 | 1,190 |

The 16 exceptions at `K=3` are

- `(8,5,5,1^9)` and `(8,3^6,1)`;
- `(7,6,6,2,1^6)` and `(7,6,6,1^8)`;
- `(7,2^t,1^(20-2t))` for `0<=t<=10`; and
- `(6,6,6,1^9)`.

## A short counterexample proof

Take

    a=(8,3,3,3,3,3,3,1) <=_w G_3.

Every child has mass nine and is majorized by `G_2=(4,3,1,1)`, so every child part is at most
four.  The width-eight parent row must therefore split as `4+4` between the mixed child and one
pure child.  Assume these are `M` and `L`.  No other parent row can create a child part four, so if
two normalized children are equal, they must be `L=M`.

After deleting their common leading four, write their common mass-five remainder as `q`.  The
`G_2` prefix inequalities leave only

    q in {(3,1,1), (2,2,1), (2,1,1,1), (1,1,1,1,1)}.

All remaining child parts must recombine into six rows of size three and one row of size one.
Hence every part three is unmatched, every part two is paired with a part one, and exactly one
part one is unmatched.  If `n_j` counts all remaining child parts of size `j`, necessarily

    n_3+n_2=6,                 n_1=n_2+1.                 (IC1)

These equations give the following complete cases for the third child `R`.

| common remainder `q` | possible `R` from (IC1) | legal-row obstruction |
|---|---|---|
| `(3,1,1)` | `(3,2,2,2)` or `(2,2,2,2,1)` | at least three `R`-twos need `M`-ones, but `M` has two |
| `(2,2,1)` | `(3,3,1,1,1)`, `(3,2,1^4)`, or `(2,2,1^5)` | two `L`-twos need the unique `M`-one |
| `(2,1,1,1)` | `(3,2,2,2)` or `(2,2,2,2,1)` | at least four pure twos compete for only three `M`-ones |
| `(1^5)` | none | (IC1) would require six parts of size at least two in mass nine |

The phrase “need `M`-ones” uses the legal-row condition: a pure `L` or `R` part can pair only with
a mixed part, never with the other pure child.  Every case is impossible, proving that this
majorized state has no split with two identical normalized children.

The state itself has a valid split; the obstruction is only to the added symmetry.  One explicit
triple is

    L=(4,3,1,1),       M=(4,2,2,1),       R=(3,3,2,1).

Match `L4+M4`, the two `L1+M2`, and `R2+M1`; leave `L3,R3,R3,R1` unmatched.  The resulting parent
is exactly `(8,3^6,1)`, and all three children are majorized by `G_2`.

## A surviving nearby statement

The failure is only one unit away from the proposed symmetry.  In the displayed split,

    L > M > R

is a dominance chain, and each consecutive pair differs by one Robin--Hood transfer.  The exact
census finds the same weaker phenomenon for every one of the 1,206 `K=3` parent types:

- every type has a valid split with at least two children equal or one transfer apart; and
- every type has a valid split whose three child types form a dominance chain.

These are exhaustive finite observations, not general theorems.  The adjacent-pair version is the
more direct repair of the original proposal and connects naturally to the existing Pascal
Adjacent-Fiber route.

## Reproduction

From the repository root:

```text
tools/singleton_identical_children_census.py
```

The dependency-free run examines all 3,375 ordered triples of the 15 `K=2` child types and takes
about 2.4 seconds locally.
