# Two-anchor Pascal residual reduction

**Superseded status (2026-08-30).**  The reduction theorem in this note is proved, but the
Balanced Residual Coloring conjecture is false.  The `K=6` singleton counterexample maps to the
explicit residual hole `(62,61,55^2,40^4,20^7,6^15,5^2)`; see
[the counterexample](singleton_k6_counterexample_2026-08-30.md).  The exact `K<=4` censuses below
remain finite theorems.

## The two forced Pascal columns

Fix `K>=2` and put

    n=2^(K-1),             M=3^(K-1),
    h=G_(K-1),             c=(h_1-1,...,h_n-1).

Trailing zeros are retained in `c`, and `C(t)` denotes its saturated prefix function.  Thus

    H(t)=C(t)+min(t,n),

where `H` is the prefix function of `h`.  The conjugate columns of `h` are indexed by subsets of
`[K-1]`; their capacities are

    2^(K-1-|T|),           T subset [K-1].

The unique full-height column is `T=empty`.  Deleting it subtracts one from every row and leaves
exactly the conjugate of `c`.

The parent conjugate has, for every child capacity `d`, one doubled column of capacity `2d` and
one single column of capacity `d`.  In particular it contains a universal column of height `2n`
and a column of height `n`.  Delete those two columns.  The remaining parent profile `J` has
conjugate

    J'={2d,d : d in c'}.

Equivalently,

    Jprefix(t)=C(t)+max_p(C(p)+C(t-p)),

or, directly in row notation,

    J=(G_K[1:n]-2, G_K[n+1:2n]-1),

with zeros removed.  For example,

    K=3: c=(3,2,0,0),             J=(6,5,2,2),
    K=4: c=(7,6,3,3,0,0,0,0),    J=(14,13,9,9,3,3,3,3).

In Boolean labels the residual parent columns are all subsets of `[K]` except `empty` and one
designated singleton `{*}`.  Thus the contraction is not an arbitrary subtraction: it removes the
universal pure column and its matching universal mixed column from the Pascal recursion.

## Two-Anchor Reduction Theorem

Let `a=(a_1>=...>=a_(2n)>0)` have mass `3M` and satisfy `a<=_w G_K`.  Define the labelled residual
degrees

    a_i-2,       1<=i<=n,
    a_i-1,       n<i<=2n,

and let `b` be their nonincreasing rearrangement, with zeros retained or discarded as convenient.

> **Two-Anchor Reduction Theorem.**  The residual is nonnegative, has mass `3(M-n)`, has at most
> `2n` positive rows, and satisfies `b<=_w J`.  Moreover, if the rows of `b` can be colored `A/B`,
> with at most `n` positive rows of each color, so that
>
>     B^A_p+B^B_q <= C(p)+C(p+q)+C(q)                    (BR)
>
> for every `p,q`, then `a` has a legal first split into three children majorized by `h`.

*Proof.*  Since `a<=_w G_K` with equal total mass, Gale--Ryser realizes `a` as row degrees against
the conjugate columns of `G_K`.  The column of degree `2n` meets every row; delete it, subtracting
one from every row.  Among the remaining columns, choose one of maximum degree `n`.  Bipartite
Havel--Hakimi (the Ryser reduction) allows this column to meet the `n` largest remaining row
degrees.  Equal-height columns are interchangeable, so label the selected one `{*}`.  Consequently
those degrees are positive, so `a_n>=2`.  Delete this column as well.  Its row degrees are precisely
the displayed residuals, and its remaining column sequence is `J'`.  Gale--Ryser therefore gives
`b<=_w J`.  The mass and support statements are immediate.

Now assume (BR).  The Fixed-Color Hall Lemma allocates `b` to residual left, mixed and right
children `l,m,r`, each majorized by `c`; `A` rows use only left+mixed and `B` rows only
mixed+right.  Pad the two color classes with zero-residual rows until each contains exactly `n`
of the original `2n` rows.

Give every row one pure anchor on its orientation side.  Give each of the original top `n` rows
one mixed anchor.  A pure residual child, padded to its `n` orientation slots, has top-`t` sum at
most `C(t)`; after adding all `n` pure anchors this is at most

    C(t)+min(t,n)=H(t).

For the mixed child, adding anchors on an arbitrary `n`-row subset increases every top-`t` sum by
at most `min(t,n)`, so the same inequality proves majorization by `h`.  All three child masses
become `M`.  Each row uses one pure side and possibly the mixed side, never both pure sides, so the
split is legal.  This proves the theorem. ∎

The theorem makes one strong normalization automatic: every one of the longest `n` parent rows is
genuinely split, because it receives both a pure and a mixed anchor.  It does **not** force the
bottom `n` rows to avoid the mixed child.

Together with the proved Minimum-Support Reduction, the full singleton converse would follow by
induction from the following smaller statement.

> **Balanced Residual Coloring Conjecture (false at `K=6`).**  Every full-mass `b<=_w J` with at most `2n`
> positive rows has a coloring satisfying (BR), with at most `n` positive rows of each color.

This is a sufficient strengthening, not an established equivalent reformulation: the deterministic
top-half mixed anchors have not been proved necessary for an arbitrary legal cut.

## Complete finite evidence

`tools/singleton_pascal_interval_census.cpp` enumerates the residual parent independently, groups
equal row values only by a symmetry quotient, and checks all two-parameter Hall inequalities.
The complete residual universe is:

| level | residual profile `J` | states with support at most `2n` | exact failures | search nodes |
|---:|---|---:|---:|---:|
| 3 | `(6,5,2,2)` | 73 | 0 | 303 |
| 4 | `(14,13,9,9,3^4)` | 160,492 | 0 | 1,140,358 |

Applying the deterministic two-anchor subtraction only to exact-support original parents gives a
larger multiset count because different parents may have the same residual:

| level | exact-support parents | failures after two anchors | search nodes |
|---:|---:|---:|---:|
| 3 | 160 | 0 | 678 |
| 4 | 408,776 | 0 | 2,929,065 |

An unrestricted residual allocation, without the color-row cap, also succeeds on all 102 and
312,755 full residual states at `K=3,4`; the capped result above is the one needed for the lift.

There is finite evidence for a stronger **Longest-Half Mixed Conjecture**: orient exactly `n` rows
to each pure side, give every parent row a positive pure piece, require exactly the largest `n`
rows to have positive mixed pieces, and require the bottom `n` rows to be pure-only.  Thus all
three children have exact support `n`.  It succeeds on all 160 exact-support
parents at `K=3` and all 408,776 at `K=4`; the latter search uses 39,086,058 nodes, with a maximum
of 51,405.  Merely requiring all three children to have exact support also passes, using
19,587,981 nodes at `K=4`.  The longest-half rule additionally succeeds on the strict-interior
`K=6` alternation counterexample

    (63^2,57^2,42^3,23^5,22^5,3^44,2^3),

but this is one instance, not a proof.  A 20-state `K=5` walk passed after 227,641,556 search
nodes; a requested 1,000-state walk reached its 120-second cap without a batch verdict and is an
abort, not evidence either way.

## What the census rules out

The two-anchor reduction is useful precisely because several simpler residual rules are false.

- Requiring one pure child to equal the canonical `h` fails at `K=3` for
  `(8,7,2,2,2,2,2,2)`.
- A fixed cross-alternation of top and bottom halves fails on 6 of the 160 exact `K=3` parents;
  fixed cross-threshold rules fail on 9.
- Binary separation under two duplicate copies of a child profile is false: for
  `h=G_2=(4,3,1,1)`, `(4,2^7)<=_w(h,h)` cannot be partitioned into two `h`-majorized
  subsequences because neither side can have mass nine.
- After two anchors, repeatedly merging the two smallest residual rows is not dominance-safe.
  It fails on 9,804 exact `K=4` parents, first at
  `(16,15,11,11,5,5,4,4,3,1^7)`.  The residual
  `(14,13,9,9,3,3,2,2,2)<=_w(14,13,9,9,3^4)` becomes invalid when two terminal twos merge to a
  four.
- Plain balanced value-block assignment misses 403 of the 160,492 residual `K=4` states and 515
  of the 408,776 two-anchor images.  One-block lookahead happens to pass both complete `K=4`
  corpora, but it fails in a `K=5` sample.
- Globally minimizing the final color-mass difference also fails.  For
  `b=(14,13,9,5,4,4,4,2,2)`, impose both necessary row bounds: four through eight positive rows
  per color.  Up to exchanging colors, its three admissible optimum partitions are

      (14,9,4,2)   | (13,5,4,4,2),
      (13,9,5,2)   | (14,4,4,4,2),
      (13,9,4,2)   | (14,5,4,4,2).

  All three violate Hall at `(p,q)=(3,5)`: demand `55` against capacity `54`.  The legal coloring
  `(14,5,4,4)|(13,9,4,2,2)` has difference three.  There are 1,067 such optimum-balance failures
  in the complete residual `K=4` universe and 2,499 among the two-anchor parent images.
- A fixed product below color classes of the canonical residual boundary is false already at
  `K=3`.  The boundary `J=(6,5,2,2)` has one normalized feasible coloring, but
  `b=(6,3,3,3)` is not a union of independent majorizations of those two color classes.  It is
  nevertheless legal with coloring `(6,3)|(3,3)`.

The first sampled `K=5` failure of both plain balance and one-block lookahead is

    b=(30,29,24,24,14^4,4^6,3,3,2).

One exact coloring is

    A=(30,24,14,14,4,4,3,3,2),
    B=(29,24,14,14,4,4,4,4).

It requires a coordinated correction across the width-four, width-three and width-two blocks;
there is no single exceptional value class to special-case.

## A sharper transfer clue, still not an induction

For a noncanonical residual state `b`, call `x` an upward predecessor when `x` is obtained by
moving one unit from a later row of `b` to an earlier row and still satisfies `x<=_w J`.  If `x`
has a feasible coloring in which those two marked rows share a color, the forward Robin--Hood
transfer to `b` preserves every Hall inequality in that color.

An exact predecessor census gives:

| level | noncanonical states | states with an endpoint-coherent predecessor | first-candidate misses | maximum candidates tried |
|---:|---:|---:|---:|---:|
| 3 | 72 | 72 | 2 | 2 |
| 4 | 160,491 | 160,491 | 77 | 2 |

At `K=4`, 160,414 states use the first lexicographically admissible predecessor; every one of the
remaining 77 uses the second.  The first miss is

    b=(14,13,8,8,5,3,3,3):

the reverse move on values `(8,8)` is endpoint-incoherent, while `(8,5)` works.  Sixty-one of the
77 first misses use equal values and sixteen do not, so “avoid equal rows” is not the rule.

This observation does not yet define compatible colorings along a whole path from `J`: the
coloring selected for a predecessor by one target need not be the coloring inherited from its own
predecessor.  The honest next transfer target is therefore an endpoint-rich residual augmentation
lemma, or a global potential that selects compatible predecessor colorings.  The one-step census
is evidence for that target, not an induction proof.

## Reproduction

```text
CC=clang++ tools/build_radio.py -std=c++20 -O3 -Wall -Wextra -pedantic \
  tools/singleton_pascal_interval_census.cpp \
  -o /tmp/singleton_pascal_interval_census

tools/run_with_provenance.py /tmp/singleton_pascal_interval_census \
  4 anchored-residual-balanced-coloring
tools/run_with_provenance.py /tmp/singleton_pascal_interval_census \
  4 two-anchor-balanced-coloring
tools/run_with_provenance.py /tmp/singleton_pascal_interval_census \
  4 anchored-residual-same-color-predecessor
tools/run_with_provenance.py /tmp/singleton_pascal_interval_census \
  4 exact-children-full-band
tools/run_with_provenance.py /tmp/singleton_pascal_interval_census \
  4 prefix-mixed-full-band
tools/run_with_provenance.py /tmp/singleton_pascal_interval_census \
  3 anchored-residual-boundary-product
```

The final optimized provenance build id is
`e42509888792c2be603e4e08c79c06d740b174405e2d7ccd334826f7729298eb`.  The complete `K=3`
residual-coloring, predecessor and longest-half modes also pass address/undefined sanitizers under
build id `5f62ce7fa7ef3a413b4c563aba2cb7afe23de6dee127ff86aff4ecb2b4452528`.  All complete positive
runs exit zero; modes whose purpose is to reproduce a stated counterexample return one.
