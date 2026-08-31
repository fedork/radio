# Singleton Adjacent-Fiber census (2026-08-27)

**Superseded universal status (2026-08-30).**  The exact `K<=4` census below remains valid, but
Adjacent-Fiber is false.  The final unit transfer into the
[`K=6` counterexample](singleton_k6_counterexample_2026-08-30.md) takes a feasible first-cut fiber
to an empty one.

## Exact predicate

Let `x` be a full-mass partition majorized by `G_K`, and choose a normalized Robin--Hood
transfer

    y=x-e_i+e_j,                 x_i>=x_j+2.

The recipient may be one of the padded zero rows.  For a coloring `sigma=(A,B)` feasible for
`x`, put

    r_sigma(S)=H(|S|)+H(|S intersection A|)+H(|S intersection B|)

and define its separating-set margin

    m_sigma(i,j)=min_(j in S, i not in S) (r_sigma(S)-x(S)).

The same coloring is feasible for `y` exactly when `m_sigma(i,j)>=1`.  The open Pascal
Adjacent-Fiber Lemma says that some such coloring exists for every transfer.

The `--adjacent-fiber-census` mode of
`tools/singleton_pair_coloring_census.cpp` tests this statement directly.  It distinguishes the
marked donor and recipient rows while quotienting only permutations of the other equal rows.
Complementation forces the donor into `A`.  For every feasible coloring of `x`, the maximum demand
of a separating set with each pair of color cardinalities is computed by forced inclusion of the
recipient and forced exclusion of the donor.  As an internal independent check, the tool also
constructs the transferred coloring and verifies

    margin>=1  if and only if  transferred coloring satisfies every Hall inequality.

For `K<=3` the mode enumerates every feasible coloring and obtains the exact maximum margin.  At
`K=4` it searches the same-color fiber first.  A same-color certificate automatically has margin
at least two: replacing the recipient by the at-least-two-heavier donor preserves all three set
cardinalities.  If that fiber has no certificate, the mode exhausts both it and the entire
opposite-color fiber.  Thus the `K=4` run is an exact existence and same-color test, proves a
certificate margin of at least two for every transfer, and obtains the exact opposite-color
optimum for every transfer that actually needs it.  It does not maximize margins larger than two
for the ordinary same-color successes, so its reported successful margins are capped at two.

One transfer type represents all choices of labelled donor and recipient having the indicated
values; equal-row symmetry makes those choices equivalent.  Distinct donor-value/recipient-value
pairs leading to the same normalized target remain distinct tests.

## Results

| `K` | full-mass states | transfer types | common coloring | same-color common coloring | margin information |
|---:|---:|---:|---:|---:|---:|
| 1 | 2 | 1 | 1 | 1 | exact 2 |
| 2 | 15 | 33 | 33 | 33 | exact 2..4 |
| 3 | 1,206 | 8,916 | 8,916 | 8,916 | exact 2..8 |
| 4 | 5,997,038 | 141,690,676 | 141,690,676 | 141,689,787 | certificate margin at least 2; capped at 2 |

Thus the Pascal Adjacent-Fiber Lemma is exhaustively true through `K=4`.  This is stronger than
the previously recorded state-by-state Row-Coloring census: it requires a common coloring for
both endpoints of every normalized unit transfer.  It is still finite evidence, not an induction
or a proof for arbitrary `K`.

The same-color strengthening is false.  Its first failure in the enumeration is

    x=(16,15,11,9,7,5,5,5,1^8),       donor=11, recipient=9.

No feasible coloring of `x` puts the two marked rows on the same side.  An opposite-color common
coloring is

    A=(15,11,5,5,1^4),
    B=(16,9,7,5,1^4).

After the transfer the marked values become 10 and 10 on their respective sides, and the same
coloring remains Hall-feasible.  Exactly 889 of the `K=4` transfer types require the search to
leave the same-color fiber before finding a certificate.  This rules out the shortcut “globally
choose a feasible coloring that keeps donor and recipient together”; genuine opposite-color
separator elimination is already necessary at `K=4`.

The diagnostic rerun classifies all 889, not merely the first one.  Every hard transfer has

    x=(16,15,11,9,lambda),       donor=11, recipient=9,

where `lambda` is a partition of 30 and the displayed full sequence is majorized by `G_4`.
Conversely every such tail occurs.  Equivalently its tail prefixes obey

    lambda_1+...+lambda_t <= (7,12,17,22,23,24,...,30)_t.

There are exactly 889 such tails in the same partition enumeration used by the full census.  In
all 889 cases the same-color fiber is empty, the exact opposite-color optimum is two, and the
chosen optimum has a unique minimizing separator with `(p,q)=(1,2)`, demand 40 and capacity 42.
The same-color obstruction already follows from the first four rows.  Rows 16 and 15 must have
opposite colors because `16+15>2H(2)=30`.  If 11 and 9 had the same color, one of 16 and 15 would
join them and the other would be alone, but the resulting `(3,1)` set has demand 51 and capacity

    H(4)+H(3)+H(1)=22+19+8=49.

Thus the exact computation exposes a single local obstruction family rather than 889 unrelated
exceptions.

## A proved obstruction-and-crossing family at every level

The local pattern is not confined to `K=4`.  Put

    U=2^(K-1),       M=3^(K-1),       d=2U-K-1,

so the first four rows of `G_(K-1)` are `(U,U-1,U-K,U-K)`.  For every `K>=4` and

    2U-2K+1 <= r <= 2U-K-3,

consider the full-mass state

    x=(2U,2U-1,d,r,1^T),
    T=3M-(2U+(2U-1)+d+r),

with transfer `d -> r`.  The interval contains exactly `K-3` integers.  The state is majorized by
`G_K`: its first three prefixes equal the canonical ones, its fourth is no larger, and thereafter
each added unit is covered by a remaining positive canonical row until the canonical mass
saturates.

No feasible coloring puts `d` and `r` together.  The top two rows must be opposite because

    2U+(2U-1) > 2H(2)=4U-2.

If `d,r` shared a color, the four top rows would have color counts `(3,1)`, while

    2U+(2U-1)+d+r > H(4)+H(3)+H(1)=8U-3K-2

is exactly the lower bound `r>2U-2K` above.

Nevertheless one crossing coloring works before and after the transfer.  Color `2U-1,d` as `A`
and `2U,r` as `B`.  Write `s=r-U+1`.  Allocate the four nonunit rows to left, mixed and right as

    2U-1 -> (U, U-1, 0),
    d    -> (U-1, U-K, 0),
    2U   -> (0, U, U),
    r    -> (0, s, U-1).

Use `M-(2U-1)` unit rows in the left child, `M-2U+K-r` in the mixed child, and
`M-(2U-1)` in the right child.  These counts are nonnegative and sum to `T`.  The three children
have mass `M` and shapes

    left  = (U,U-1,1,...,1),
    mixed = (U,U-1,U-K,s,1,...,1),
    right = (U,U-1,1,...,1).

They are majorized by `G_(K-1)`: the displayed nonunit prefixes fit its first four rows, every
remaining canonical row is at least one, and all three children have mass `M`.  After the transfer,
keep the coloring and every allocation except replace the mixed contributions `U-K,s` by
`U-K-1,s+1`.  Since `s<=U-K-2`, those two entries still fit under the identical pair
`U-K,U-K`.  This proves a common coloring uniformly, with no search and no cyclic reassignment.

More strongly, the same construction works for every intermediate transfer of at most `d-r`
units: the two mixed contributions move toward one another underneath the identical pair.  Hence
the separating margin is at least `d-r`; the set consisting of the largest unmarked `A` row and
the largest two `B` rows has slack exactly

    H(1)+H(2)+H(3)-((2U-1)+2U+r)=d-r.

So the margin is exactly the donor-recipient gap.  This is a concrete Pascal augmenting step: the
two identical `U-K` targets absorb the transfer.  The formerly proposed next step was proving that
every minimal opposite-color separator at an arbitrary Pascal level can be eliminated by changing to a suitable
feasible coloring.  A fixed-color tight separator cannot itself be crossed while preserving row
orientations--that is exactly what the Fixed-Color Hall Lemma forbids.  The later
[coloring-landscape analysis](singleton_coloring_landscape_2026-08-28.md), after quotienting equal
rows and global side complementation, instead supports a Two-Row Color-Exchange Lemma through all
`K<=3` cases and four disjoint `K=4` windows totaling 13,000 states.  The `K=6` hole refutes that
stronger local statement universally.

The complete `K=4` run visited 1,173,872,133 search nodes and 141,770,271 complete feasible
colorings.  It completed in 353 wall seconds with reported peak RSS below 0.01 GB under
`tools/capped_run.sh`.  It found no Adjacent-Fiber counterexample.

## Reproduction

The provenance build is
`b3809eae9bc918025b7c01ac262fed43372ae50f78c581bfe20514bce5f63ece`.

    CC=clang++ tools/build_radio.py -O3 -std=c++20 -Wall -Wextra -pedantic \
        tools/singleton_pair_coloring_census.cpp -o /tmp/singleton-pair-census-adjacent-v2
    tools/run_with_provenance.py /tmp/singleton-pair-census-adjacent-v2 \
        --adjacent-fiber-census 1 0 exact
    tools/run_with_provenance.py /tmp/singleton-pair-census-adjacent-v2 \
        --adjacent-fiber-census 2 0 exact
    tools/run_with_provenance.py /tmp/singleton-pair-census-adjacent-v2 \
        --adjacent-fiber-census 3 0 exact
    tools/capped_run.sh --seconds 1800 --rss-gb 4 --label adjacent-fiber-k4-v2 -- \
        tools/run_with_provenance.py /tmp/singleton-pair-census-adjacent-v2 \
        --adjacent-fiber-census 4

All four raw outputs pass `tools/check_provenance.py`.  The `K<=3` runs took less than one wall
second together on the recorded M4 Pro.  The complete `K=4` wrapper exited zero rather than by its
time or memory cap.

The hard-family diagnostic build is
`fdb42be9f6e301adb279582600397400fbee9a6648dd24a0de5b3355b620e415`.  Its complete `K=4`
rerun took 358 wall seconds, printed all 889 hard records plus the aggregate classification above,
and again visited 1,173,872,133 nodes.  The later single-case mode was built as
`f18ca95eb7b7cd6df8d25df3d35594490dc9392193df3c98c8e5d976dc76dc88`; exact probes of the
all-unit-tail family at `K=5` for `26->24` and `26->23`, and at `K=6` for `57->55`, agree with
the proved formula.  Every retained local output passes `tools/check_provenance.py`.  These probes
are checks of the algebraic construction, not evidence needed for its proof.
