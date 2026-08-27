# Transcript-graph strong-niceness census (2026-08-27)

## Why this implies the desired support statement

For the transcript conflict graph `Q_K`, write

    X_(Q_K) = sum_lambda c_K(lambda) m_lambda.

Stanley's monomial expansion identifies `c_K(lambda)` with the number of semi-ordered stable
partitions of type `lambda`.  A graph is strongly nice when

    lambda >= mu  =>  c_K(lambda) <= c_K(mu).

Strong niceness therefore implies that the stable-partition support is a dominance ideal, which is
the Singleton Majorization Converse.  Definitions and the implication are also stated in
[Li--Li--Yang--Zhang](https://arxiv.org/abs/2408.15074).

## Exact recurrence and result

The graph recursion is

    Q_K = Q_(K-1) disjoint-union (Q_(K-1) join Q_(K-1)).

For a fixed labelled color used `w` times, its counts in the three copies must be
`(w-x,x,0)` or `(0,w-x,x)`.  `tools/singleton_strong_niceness.cpp` enumerates these allocations;
at a complete allocation it multiplies the three recursively computed child coefficients.  It
does not assume that majorization is sufficient.  Child majorization is used only as the proved
necessary zero test.

The utility was built with

    CC=clang++ tools/build_radio.py -O3 -Wall -Wextra \
        tools/singleton_strong_niceness.cpp -o /tmp/singleton_strong_niceness

at build id
`d2f94ba6740cd8bcabb7e461ebd655bebc316d00fd8875fb65802cd235a5210a`.  The provenance-wrapped,
five-second capped command

    tools/capped_run.sh --seconds 300 --rss-gb 4 --label strong-niceness-k3 -- \
        tools/run_with_provenance.py /tmp/singleton_strong_niceness 3

returned

    STRONG_NICENESS_PASS k=3 partitions=1206 comparable_pairs=463886
    NODES k=1 value=4
    NODES k=2 value=249
    NODES k=3 value=4740395

The same executable returns 1 and 2 supported partitions for `K=0,1`, 15 for `K=2`, and passes all
94 comparable ordered pairs at `K=2`.  Since a coefficient is at most `27!`, the `K<=3` utility's
unsigned 128-bit arithmetic is exact.

This is exhaustive finite evidence, not a proof for arbitrary `K`.

## Why ordinary closure results do not finish the induction

A recent multiplication theorem proves that strongly nice symmetric functions with nonnegative
monomial coefficients are closed under ordinary multiplication, hence strongly nice graphs are
closed under disjoint union.  The same paper gives nice-but-not-strongly-nice graph joins, so join
has no corresponding general closure theorem; see
[Zhang, Theorem 4.1 and Section 5](https://arxiv.org/html/2608.16613v1#S4.SS1).

Moreover `Q_3` contains an induced claw.  Therefore the theorem that a graph and all its induced
subgraphs are strongly nice exactly when it is claw-free does not apply to the transcript family.
The still-open target is closure for the special combined operator

    T(G)=G disjoint-union (G join G),

or only the weaker assertion that `T` preserves the principal dominance-ideal support occurring in
this recursion.
