# Singleton Adjacent-Fiber census (2026-08-27)

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
