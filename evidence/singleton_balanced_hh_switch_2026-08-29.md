# Balanced Havel--Hakimi and Pascal-switch survey (2026-08-29)

## Status and conclusion

This note does **not** prove the Singleton Majorization Converse.  It turns the proposed
"keep halving" construction into one precise, finite switch algorithm and separates three facts:

1. the full incidence-and-coloring switch graph is a complete decision space (proved);
2. one canonical Havel--Hakimi realization is not always colorable (exact `K=5`
   counterexample);
3. a deterministic descent using row swaps and `2x2` incidence switches survives the complete
   exact-support `K<=4` corpus and the stated `K=5,6` probes, but its general escape lemma remains
   open.

Thus repeated halving is still a workable global route.  The remaining proof is no longer an
assignment formula: it is an augmenting-path theorem saying that a stopped Pascal switch search
forces a violated parent prefix.

## Canonical realization and quota energy

Fix an exact-support full-mass parent

    a=(a_1>=...>=a_(2^K)>0) <=_w G_K.

The conjugate parent columns have rank `ell`, degree `2^(K-ell)`, and multiplicity
`binomial(K,ell)`.  Construct a labelled zero--one incidence matrix by bipartite Havel--Hakimi:
process columns in nonincreasing degree order and attach the next column to the rows with largest
current residual degree, breaking ties by original row index.  Gale--Ryser/Ryser reduction proves
that this finishes with row sums `a`.

Initially color odd-indexed rows `A` and even-indexed rows `B`.  At rank `ell`, put

    b_ell=binomial(K-1,ell).

For each column `C` of even degree `d`, let

    delta(C)=|C intersection A|-d/2.

Sort `delta(C)^2` inside a rank and define

    Phi=sum_(ell=1..K-1) (the b_ell smallest squared deviations at rank ell).

Then `Phi=0` exactly when every rank has the required number of bisected columns.  Those columns
are labelled doubled/pure; the remaining `binomial(K-1,ell-1)` columns are single/mixed.  The
Balanced-Columns equivalence then gives a legal first split.

The tested descent preserves all margins and the exact row bisection:

- swap the colors of one `A` row and one `B` row; or
- apply a degree-preserving `2x2` incidence switch
  `[[1,0],[0,1]] <-> [[0,1],[1,0]]`.

Take the strict move giving the smallest `Phi`, with the fixed source-order scan breaking ties.
If none exists, scan `Phi`-neutral row swaps and then incidence switches (including same-color
incidence switches), and take the first one that exposes a strict move.  Every one or two moves
therefore lowers the nonnegative integer `Phi`.

> **Canonical Two-Move Pascal-Switch Conjecture.**  Starting from the canonical realization and
> alternating coloring above, this procedure never reaches positive `Phi` with no strict move and
> no neutral-then-strict move.

This conjecture would prove the converse after the proved Minimum-Support Reduction and induction
on `K`.  It is a sufficient strengthening, not an equivalent reformulation.

## A proved complete search space

There is a useful theorem behind the heuristic.

> **Switch-Graph Completeness Lemma.**  Fix the row margins `a`, the labelled Pascal column
> margins, and `2^(K-1)` rows of each color.  All pairs `(incidence realization, row bisection)`
> lie in one graph under arbitrary row-color swaps and arbitrary `2x2` incidence switches.
> Consequently breadth-first search from the canonical pair finds a quota-balanced realization
> if and only if one exists.

*Proof.*  Any two equal-size row bisections differ by a sequence of swaps between an `A` row and a
`B` row.  Independently, the classical interchange proof for binary matrices says that all
zero--one matrices with fixed row and column sums are connected by `2x2` switches: the symmetric
difference of two realizations is an alternating Eulerian bipartite graph, and its alternating
cycles are eliminated by interchanges.  First change the coloring and then the matrix.  Both
operations preserve the prescribed margins, so their product graph is connected.  Quota balance
is exactly `Phi=0`.  This proves the claim.  ∎

This is already a universal exact algorithm for each finite state.  What is open is the theorem
that every Pascal-majorized state has a successful vertex, or, more constructively, that the short
descent above always reaches one.

## The canonical matrix itself is false at `K=5`

An exact counterexample to the rule "construct the canonical matrix and only color its rows" is

    a*=(32,31,26,26,16,16,16,4^15,2^10).

It has mass `243`, exact support `32`, and is majorized by `G_5`.  Its nontrivial canonical
supports are as follows (rows are one-based):

    rank 1, degree 16:
      1..16
      1..10,17..22
      1..7,11..19
      1..13,20..22
      1..7,14..22

    rank 2, degree 8:
      {1,..,7,j},  j=23,..,32

    rank 3, degree 4:  ten copies of {1,2,3,4}
    rank 4, degree 2:  five copies of {1,2}

The rank quotas are respectively `4,6,4,1`.

Here is a short proof that no row bisection colors this fixed matrix.  Rank 4 makes rows 1 and 2
opposite.  Rank 3 then makes rows 3 and 4 opposite, so the first four rows contain exactly two
`A` rows.  Let `r` be the number of `A` rows among rows 1--7, and let `t` be the number among rows
23--32.  Rank 2 can meet its quota only in one of two cases:

    r=3 and t>=6,       or       r=4 and t<=4.                 (1)

Put `b,c,d,e,f` for the `A` counts in the five triples 8--10, 11--13, 14--16,
17--19 and 20--22.  The five rank-1 `A` counts are

    r+b+c+d,  r+b+e+f,  r+c+d+e,  r+b+c+f,  r+d+e+f.          (2)

At least four must equal eight.  This small calculation can be done without a computer.  Subtract
`r` and put `s=8-r`.  The five left sides after that subtraction are

    b+c+d,  b+e+f,  c+d+e,  b+c+f,  d+e+f.                  (3)

Their omitted pairs form a five-cycle, so all five choices of a possibly exceptional equation are
equivalent.  If, for example, the first equation is omitted and the other four equal `s`,
subtraction gives `c=e=f` and `b=d=s-2c`.  The bounds `0<=b,c,d,e,f<=3` then give
`c=1,2`.  Hence `b+c+d+e+f=2s-c`: it is `9` or `8` when `r=3`, and `7` or `6` when `r=4`.
The five-cycle symmetry covers every omitted equation (and all five cannot be equal, since that
would require `3b=s`).  Therefore

    r=3  =>  #A in rows 1--22 is 11 or 12,
    r=4  =>  #A in rows 1--22 is 10 or 11.

There are sixteen `A` rows in total.  Thus the first case gives `t=5` or `4`, contradicting (1),
and the second gives `t=6` or `5`, also contradicting (1).

The exact grouped-color search independently returns `found=NO`, `aborted=NO` after 15,139 nodes.
This is not a counterexample to singleton majorization.  With the alternating coloring, switch
columns 4 and 7 on rows 12 and 23.  The quota-energy profile changes from `(0,1,0,0)` to zero.
Column 4 is rank 1 and is allowed to absorb the defect; column 7 is rank 2 and becomes
balanced.  This single incidence switch is a valid balanced realization and hence a legal cut.

This example is the cleanest evidence that Pascal multiplicities are buffers, not a reason to
freeze one realization.

## Strict descent is also too rigid

The first parent in the prefix census at which no row swap or incidence switch strictly lowers
`Phi` is

    (32,31,26,26,16^3,9,6^9,2^2,1^13).

Its initial energy profile is `(1,0,0,0)`.  A neutral color swap of rows 5 and 8 leaves the energy
one; a strict color swap of rows 9 and 14 then makes it zero.  Thus cyclic reassignment is not
needed here, but one globally chosen setup move is.  This is why the tested algorithm permits one
neutral edge before every strict decrease.

## Exact and sampled results

`tools/singleton_balanced_hh_census.cpp` independently enumerates sorted exact-support parents,
constructs the canonical matrix, and checks the switch rule.  For `K<=4` it also exhausts all
normalized row bisections of the original canonical matrix.

| corpus | result | relevant diagnostics |
|---|---|---|
| complete `K=3` exact-support corpus | 160 / 160 pass | alternating coloring already works |
| complete `K=4` exact-support corpus | 408,776 / 408,776 pass | 69,664 need one row swap; no incidence or neutral move; maximum initial energy 2 |
| first 500,000 `K=5` exact-support parents | pass | 190,975 moves; 347 incidence switches; 12 neutral moves; maximum two moves per parent |
| `K=5`, 100,000-state windows after 5,000,000 and 50,000,000 parents | pass | maximum two and one moves respectively |
| `K=5`, 100,000 multinomial samples and 100,000 Robin--Hood-walk states | pass | maximum two and one moves respectively |
| `K=5`, 500,000-iteration difficulty hill climb | no failure | best state needed two moves from initial energy 3 |
| `K=6`, 100,000-state Robin--Hood walk | pass | 43,513 moves, 417 incidence switches, maximum three moves; maximum initial energy 4 |
| `K=6`, 100,000-iteration difficulty hill climb | no failure | best state needed three moves from initial energy 5 |

The sampled rows may repeat and are not claims about a uniformly sampled finite universe.  The
only complete theorem in the table is through `K=4`.

An adversarial 338,984-iteration Python hill climb also refuted a tempting diagnostic: canonical
columns do **not** always have imbalance at most one.  The parent

    (23,22,22,22,18,17,14,11,11,10,10,7,7,6,6,5,5,5,3,3,2^4,1^8)

has a rank-3 canonical column with imbalance two.  Its quota energy is already zero, so the large
imbalance lies in an allowed buffer.  The search took 61.2 seconds.  This distinction--quota
defect rather than maximum column discrepancy--must be retained in any proof.

## Three other global shortcuts rejected

The special cograph operator alone does not preserve a principal dominance ideal.  Let `E_3` be
the empty graph on three vertices and `T(G)=G disjoint-union (G join G)`.  Every partition of nine
dominated by `(6,3)` occurs in `T(E_3)`: equivalently, every part of a partition of nine with
largest part at most six can be split among three bins of mass three with no row using all three.
For a direct construction, treat largest parts `6,5,4` as `3+3`, `3+2`, `3+1` and fill the
remaining bin capacities; if every part is at most three, sequentially fill three bins, and no
part can cross two boundaries.

Nevertheless `T(T(E_3))` has maximum profile `(12,9,3,3)`, while

    (12,9,2,2,2) <=_w (12,9,3,3)

does not occur.  A size-12 row must use `6+6` in the mixed copy and one pure copy.  The size-9 row
must then use the remaining `3+6` in the mixed and other pure copy.  The mixed copy is full and
each pure copy has capacity three left.  Three rows of size two cannot fill those two capacities
without either using the full mixed copy or illegally crossing both pure copies.  Hence even
`T` applied to a `T`-generated nice cograph need not remain nice.  A proof for `Q_K=T^K(K_1)` must
retain the exact Pascal seed hierarchy, not only the formal graph operator.

A second full-depth greedy survey processed the actual ranks of `P_K`, matching each rank into the
compatible rows of greatest residual demand.  Sequential assignment already fails for
`(3,2,2,2)@K=2`; maximum-weight matching repairs `K=2` but fails at
`(8,5,4,4,3,1,1,1)@K=3`, even after a longest-future-chain urgency check.  Thus "keep halving"
cannot mean committing each rank irrevocably.  The incidence switches above are precisely the
required backward correction.

Nor can all recursive bisections be frozen into one global Boolean address on the rows.  In that
stronger model, label the `2^K` rows by bit strings and require the column indexed by
`S subset [K]` to be the subcube `{x:x|S=p_S}` for one chosen pattern `p_S`.  It already misses
`G_2=(4,3,1,1)`.  There are four rows, one universal column, two coordinate half-columns and one
singleton column.  A degree-four row must be in both halves and receive the singleton.  The two
rows adjacent to it in the square then each receive the universal column and exactly one half,
forcing degrees at least two, so the profile is `(4,2,2,1)`, not `(4,3,1,1)`.  Recursive halving
must permit branch-dependent relabelling; it cannot be represented by one fixed row hypercube.

## Reproduction

```text
CC=clang++ tools/build_radio.py -std=c++20 -O3 -Wall -Wextra -pedantic \
  tools/singleton_balanced_hh_census.cpp \
  -o /tmp/singleton_balanced_hh_census

tools/run_with_provenance.py /tmp/singleton_balanced_hh_census 4
tools/run_with_provenance.py /tmp/singleton_balanced_hh_census 5 500000
tools/run_with_provenance.py /tmp/singleton_balanced_hh_census 6 walk 100000 29006
tools/run_with_provenance.py /tmp/singleton_balanced_hh_census 5 hill 500000 29029
tools/run_with_provenance.py /tmp/singleton_balanced_hh_census 6 hill 100000 29030
```

The fixed-canonical counterexample and its one-switch repair are reproduced by `canonical-state`
and `state` modes using the expanded form of `a*` above.  Complete `K=4` output is locked by
internal regression counts.  Clean committed-source build
`37f2edf7635cf35e18f1843702959905720bfc55d75480b4e1bfcba260ade110`
reproduces the complete `K=4` run, the first 500,000 `K=5` parents and both exact boundary cases.
Identical-source builds `53ccceb84295839a9a68d8b92912ec4ee9f0770f9b14b788566f71cdc4ab223e`
and `ade0deedd3cd2c952f9f71d1ea92a1a9097e717da51f23e86e6b481a5b40b668` produced the hill probes
and the stated `K=6` walk respectively.  Address/undefined sanitizers pass the complete `K=3` run
under clean build `50751955c54ffa2a0b9fad77a6fc347ccd0a4f08c0906f6dd1a3bd73cf7d035f`.
