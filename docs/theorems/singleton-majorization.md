# Singleton majorization: proved necessity and the open converse

## Status (corrected 2026-08-26)

For a singleton state

`Sb(a1:1, a2:1, ..., an:1)`, with `a1 >= a2 >= ...`,

the implication

`solvable in K tests  =>  (a1,a2,...) <=_w G_K`

is proved below.  The converse is strongly supported but is **not proved in this repository**.
Aigner proved the same necessary direction and explicitly left the converse open in 1988; see
[the scan note](../aigner-1988-scan.md).

The former proof in this file used a purported Three-Way Majorization Decomposition Lemma.  That
lemma is false, and even a correct unconstrained polymatroid decomposition would not enforce the
legal row condition that a singleton row cannot feed both pure children.  The exact remaining
statement is the Row-Coloring Lemma below.  It is special to `G_K`; generic base sequences have
counterexamples.

## Definitions

We write all sequences in nonincreasing order and pad them with trailing zeros when needed.

### Weak majorization
For two nonincreasing sequences

`x = (x1, x2, ...)`, `y = (y1, y2, ...)`,

we write

`x <=_w y`

if for every `t >= 1`,

`sum_{i=1..t} xi <= sum_{i=1..t} yi`.

### Singleton base sequence
Define `G_0 = (1)`.

For `K >= 1`, if

`G_{K-1} = (h1 >= h2 >= ... >= hm)`,

then define three zero-padded sequences of length `2m`:

- `L_K = (h1, 0, h2, 0, ..., hm, 0)`
- `M_K = (h1, h2, ..., hm, 0, ..., 0)`
- `R_K = (0, h1, 0, h2, ..., 0, hm)`

and define

`G_K := sort(L_K + M_K + R_K)`

where the sum is coordinatewise and `sort` means reorder in nonincreasing order.

This gives:

- `G_0 = (1)`
- `G_1 = (2, 1)`
- `G_2 = (4, 3, 1, 1)`
- `G_3 = (8, 7, 4, 4, 1, 1, 1, 1)`

etc.

---

## Why the former decomposition lemma is false

At `K=2`, take the three staggered copies of `G_1=(2,1)` used in the old argument:

```
u = (2,0,1,0)
v = (2,1,0,0)
w = (0,2,0,1).
```

The vectors

```
x1 = (2,1,0,0)
x2 = (0,2,1,0)
x3 = (1,0,2,0)
```

are each weakly majorized by the corresponding copy and sum to `(3,3,3,0)`.  But

```
3+3+3 = 9 > 4+3+1 = 8,
```

so `(3,3,3)` is not weakly majorized by `G_2=(4,3,1,1)`.  Thus the stated
coordinate-sum lemma was not a standard polymatroid theorem.

There is a second, independent defect in using an unconstrained decomposition for sufficiency.
For one star row, the tested centre is either outside or inside the test.  The row can therefore
feed outcomes `{0,1}` or outcomes `{1,2}`, but never both pure outcomes `0` and `2`.  In the old
notation every legal split must satisfy

`l_i r_i = 0` for every row `i`.

An arbitrary three-way polymatroid decomposition does not impose this condition.

## The transcript conflict graph

Let `Q_K` have vertex set `{0,1,2}^K`.  Two words are adjacent when, at their first differing
coordinate, their symbols are `0` and `2` in some order.

> **Transcript lemma.** A set of transcripts can be assigned to one singleton row if and only if
> it is a stable set of `Q_K`.

To see this, consider a node at which the row's hypotheses still have a common transcript prefix.
If the centre of the star is outside the test, those hypotheses may split only between outcomes
`0` and `1`; if the centre is inside, they may split only between `1` and `2`.  This proves the
forward implication.  Conversely, the absence of a `0`/`2` split at every common-prefix node tells
us which of these two choices to make, and the leaves can then be selected independently.  Applying
this row by row constructs the test.

Consequently, a singleton state with row sizes `a_1,...,a_n` is solvable in `K` tests if and only
if some induced subgraph of `Q_K` has a proper coloring whose nonzero color-class sizes are
`a_1,...,a_n`.

The graph has the exact recursion

`Q_K = Q_{K-1} disjoint-union (Q_{K-1} join Q_{K-1})`.

The three copies are the transcripts starting in `1`, `0`, and `2`: the `0` and `2` copies are
completely joined, while the `1` copy has no edges to either.

### A stronger chromatic-symmetric-function target (2026-08-27)

Write the chromatic symmetric function as

    X_(Q_K) = sum_lambda c_K(lambda) m_lambda.

Stanley's monomial expansion says that `c_K(lambda)` counts semi-ordered stable partitions of
`Q_K` of type `lambda`.  Thus the desired converse follows from the formally stronger inequalities

    lambda >= mu  =>  c_K(lambda) <= c_K(mu).                    (SN)

In current terminology, (SN) says that `Q_K` is **strongly nice**; strongly nice immediately
implies nice, meaning that the stable-partition support is a dominance ideal.  This packages the
existence problem globally: it asks for monotonicity of the number of decompositions, rather than
selecting one decomposition by a local rule.

The graph recursion gives an exact coefficient recurrence.  For a fixed labelled color of size
`w`, its counts in the left/mixed/right copies must be

    (w-x,x,0) or (0,w-x,x).

After assigning every color, the three child coefficients multiply, and summing all assignments
gives `c_K`.  The provenance-built `tools/singleton_strong_niceness.cpp` evaluates this recurrence
without invoking the open converse.  It verifies (SN) for `K=3` on all 1,206 possible supported
types and all 463,886 comparable ordered pairs (4,740,395 recursive allocation nodes).  This is
another finite theorem only, not an induction proof.

Labelling every coin by its unique transcript makes this recurrence genuinely bijective at the
decorated level: restricting a parent coloring to the three first-symbol copies recovers its three
child colorings uniquely, and compatible child colorings recombine uniquely.  It does not make the
projection to the row-size vector injective.  In coefficient language a successful counting proof
must therefore be coefficientwise.  The clean sufficient target for one Robin-Hood transfer is an
injection

    colorings of type a  -->  colorings of type a-e_i+e_j.                 (INJ)

Trying to build (INJ) recursively identifies the same structural obstruction as the Hall proof.
If the donor has more occurrences than the recipient in the mixed child, or in a pure child that
the recipient may legally use, child-level strong niceness performs the transfer.  The unresolved
case has donor and recipient on opposite pure sides, with no donor excess in the mixed child.  A
two-color Kempe exchange need not work—the induced-claw example below already proves that—and an
injective construction must route through a third color or a global augmenting chain.  That is
strictly stronger than merely finding one feasible coloring for the target state, and its stopped
augmenting paths would have to overcome the same opposite-color tight separators isolated by the
Pascal Adjacent-Fiber Lemma.

Exact arbitrary-precision probes at `K=4` have not refuted (INJ): the coefficient rises on the
first canonical balancing transfer, and one exact ten-transfer walk is monotone.  A longer request
hit its cap after those ten completed steps.  These are narrow finite probes, not evidence for all
5,997,038 states or all dominance covers; commands and exact coefficients are in the
[strong-niceness census record](../../evidence/singleton_strong_niceness_2026-08-27.md).

Padding to `N=3^K` labelled rows removes permutation quotients but does not turn this recurrence
into a cardinality proof.  Let `S_K` be the full-mass labelled vectors majorized by `G_K`.  Its
cardinality has the exact partition sum

    |S_K| = sum_(lambda partition N, lambda<=G_K)
                N! / ((N-length(lambda))! product_(v>=1) multiplicity_v(lambda)!).       (COUNT)

Let `D_K` instead be the triples of padded child vectors `(l,m,r)`, each majorized by `G_(K-1)`,
with `l_i r_i=0`, and map a triple to `a=l+m+r`.  This map has image contained in `S_K`, but it is
many-to-one with nonconstant fibers.  Already for `K=1`, `S_1` consists of the six labelled
permutations of `(2,1,0)` and `(1,1,1)`, hence has size seven.  The three unit children have
`3*3*2=18` legal labelled placements: every `(2,1,0)` output has two preimages, while `(1,1,1)`
has six.

Consequently, counting child triples computes a sum of decomposition coefficients, whereas the
converse asks for the size of their support.  Proving that the number of *distinct* outputs equals
`|S_K|` would indeed prove the Row-Coloring Lemma, because the image is already contained in
`S_K`; however, determining that distinct-output count is exactly the assertion that every relevant
coefficient is nonzero.  A uniform division by a fiber size is impossible even at `K=1`.

There is no immediate closure theorem for the displayed graph recursion.  Strong niceness is
closed under disjoint union, but not under graph join in general; moreover `Q_3` contains an induced
claw, so the hereditary claw-free characterization does not apply.  The useful new proof target is
therefore the special operator

    T(G) = G disjoint-union (G join G):

prove directly that the particular sequence `Q_K=T(Q_(K-1))` preserves strong niceness, or at least
preserves the dominance-ideal support.  The coefficient recurrence is a clean setting in which to
try the same two-coordinate balancing transfer as in the Row-Coloring Lemma.

## Proven necessity

For a graph `Q`, let `alpha_t(Q)` be the maximum number of vertices in an induced subgraph that is
colorable with at most `t` colors, and set `alpha_0=0`.  Write

`H_K(t) = sum_{i=1..t} G_K[i]`,

with `H_K` constant after the end of `G_K`.

> **Proposition.** `alpha_t(Q_K)=H_K(t)` for all `K,t`.

*Proof.* The claim is trivial for `K=0`.  In the middle copy we may take at most
`alpha_t(Q_{K-1})` vertices.  In the join of the two outer copies, the color sets must be disjoint;
if they use `p` and `q` colors with `p+q=t`, they contribute at most

`alpha_p(Q_{K-1}) + alpha_q(Q_{K-1})`.

By induction this is `H(p)+H(q)`, where `H=H_{K-1}`.  Since the increments of `H` are the
nonincreasing entries of `G_{K-1}`, this sum is maximized when `p,q` are as equal as possible.
Therefore

`alpha_t(Q_K) = H(t) + H(ceil(t/2)) + H(floor(t/2))`.

The duplicated outer sequence has prefix sum `H(ceil(t/2))+H(floor(t/2))`, and the middle sequence
has prefix sum `H(t)`.  Their coordinatewise sum is already nonincreasing and is exactly `G_K`, so
the right-hand side is `H_K(t)`. ∎

If a singleton state is solvable, the union of its `t` largest color classes is a `t`-colorable
induced subgraph of `Q_K`.  Hence every prefix of `a` is at most `H_K(t)`.  This proves

> **Singleton Majorization Necessity Theorem.** Every singleton state solvable in `K` tests has
> `a <=_w G_K`.

This proof does not use the false decomposition lemma.

## The exact remaining Row-Coloring Lemma

Put `h=G_{K-1}` and let `H(t)` be its saturated prefix-sum function.  Color every parent row either
`A` or `B`; an `A` row is allowed to feed the left pure child and the mixed child, while a `B` row
is allowed to feed the mixed child and the right pure child.  Let `A_p` and `B_q` be the sums of the
`p` largest `A` rows and the `q` largest `B` rows.

> **Fixed-Color Hall Lemma.** For this fixed row coloring, a legal first split whose three child
> sequences are weakly majorized by `h` exists if and only if, for every `p,q>=0`,
>
> `A_p + B_q <= H(p+q) + H(p) + H(q).`  (C)

*Proof.* Let `h'` be the conjugate partition of `h`.  For each child make one column of capacity
`h'_j` for every `j`.  A row may use a column at most once.  An `A` row is joined to the left and
mixed columns; a `B` row is joined to the mixed and right columns.  A set consisting of `p` `A`
rows and `q` `B` rows sees total capacity

`sum_j min(p,h'_j) + sum_j min(p+q,h'_j) + sum_j min(q,h'_j)`

`= H(p)+H(p+q)+H(q)`.

The integral bipartite `b`-matching theorem says that all row demands can be assigned exactly when
no row subset exceeds its neighborhood capacity.  Among subsets with `p,q` rows, the largest demand
is `A_p+B_q`, giving (C).  Reading the assigned columns in each child gives a `0`-`1` matrix whose
row sums are that child's sequence and whose column sums are bounded by `h'`; the usual same Hall
criterion is exactly weak majorization by `h`.  The support restrictions make the split legal. ∎

The full converse is therefore equivalent to the following purely two-color statement.

> **Row-Coloring Lemma (open).** If `a <=_w G_K`, the rows of `a` can be colored `A/B` so that
> (C) holds for every `p,q`.

Indeed, the Fixed-Color Hall Lemma produces three `G_{K-1}`-majorized children, and induction would
finish the strategy.  Conversely, the first test of any strategy supplies exactly such a coloring.

The parent majorization inequalities are only the balanced slice of (C):

`H_K(t) = H(t)+H(ceil(t/2))+H(floor(t/2)).`

The missing proof must show that the rows can be colored so that all *off-diagonal* pairs `(p,q)`
also hold.  This is where strict alternation and simple load balancing fail.

The base has the elementary `2`-flat properties

- `h_1=h_2+1`;
- `h_{2j-1}=h_{2j}` for every `j>=2` (with zero padding);
- equivalently, every column height of `h` is even except for one column of height one.

These properties alone are **not sufficient**.  Take

`h=(6,5,1,1)`, whose induced parent base is `g=(12,11,6,6,1,1,1,1)`, and

`a=(12,11,6,2,2,2,2,2) <=_w g`.

If a coloring satisfied (C), the full-set inequality would force four rows of each color, because
`sum(a)=3 sum(h)=39` and `H(s)=13` only for `s>=4`.  Put row 12 in `A` by symmetry.  Row 11 must
then be in `B`, since putting both in `A` violates `(p,q)=(2,0)` by `23>2H(2)=22`.  If row 6 is
in `A`, the four rows of `A` are `(12,6,2,2)` and `(p,q)=(4,1)` fails by

`22+11=33 > H(5)+H(4)+H(1)=13+13+6=32`.

If row 6 is in `B`, the symmetric `(1,4)` inequality fails by the same amount.  Thus even the
`2`-flat shape and column parity do not imply the lemma.

For comparison, generic bases fail even more easily: with `h=(6,1)`, the induced parent base is
`(12,7,1,1)`, and `(12,3,3,3)` is majorized by it but has no legal decomposition.  A proof for
`G_{K-1}` must therefore use the full hierarchy of Pascal multiplicities, not only local equality
patterns or parity.

### A sharper bottom-up target

The conjugate partition of `G_r` has a particularly rigid form:

`(G_r)' = (2^(r-j) repeated binomial(r,j) times, 0<=j<=r).`

Equivalently, if `c` runs through the column capacities of `h'=G_(K-1)'`, then the column
capacities of `G_K'` are the multiset

`{2c : c in h'} multiset-union {c : c in h'}.`                    (D)

This is just Pascal's identity: doubling a capacity `2^(K-1-j)` contributes
`binomial(K-1,j)` copies at level `j`, while retaining it contributes the other
`binomial(K-1,j)` copies at level `j+1`.

Formula (D) gives another exact version of the coloring problem.  Parent majorization says, by
the bipartite degree criterion, that there is a `0`-`1` matrix with row degrees `a` and column
capacities consisting of one doubled column `2c` and one single column `c` for every child column.
The desired legal decomposition is equivalent to choosing such a matrix and coloring its rows
`A/B` so that every doubled column contains at most `c` rows of each color.  The doubled column
then separates into the left and right pure columns, while the single column becomes the mixed
column.  Crucially, one color is chosen for the whole row, not separately for its incidences.
An ordinary balanced edge-coloring would therefore be insufficient.

Thus every child column has power-of-two capacity, with multiplicities given by a row of Pascal's
triangle.  This gives a precise way to formulate the proposed “many identical transfer targets”
idea.

#### Pascal-first Boolean columns and a tight-band lemma (2026-08-28)

The multiplicities in (D) can be given canonical labels instead of being treated as anonymous
copies.  Index the columns of `(G_r)'` by the Boolean lattice `2^[r]`, with

    c(T)=2^(r-|T|),                 T subset [r].

There are `binomial(r,j)` sets of rank `j`, so this is exactly the conjugate partition above.
After adjoining a new first-test coordinate `*`, every child column `T subset [r]` gives the two
parent columns

    T                 of capacity 2c(T),
    T union {*}       of capacity  c(T).

The first is the doubled pure column and the second is the mixed column.  Thus Pascal's identity
is built into the first split: parent columns omitting `*` divide into equal left/right halves,
while columns containing `*` become the mixed child.

For a full-mass demand vector `a<=_w G_(r+1)`, Gale--Ryser gives a saturated simple incidence
matrix from rows to these parent columns.  A legal first cut is equivalent to choosing such a
matrix together with one sign for every row so that, in every doubled column `T`, at most `c(T)`
incident rows have either sign.  Saturation then makes the split exactly `c(T)+c(T)`; the single
column `T union {*}` supplies the mixed incidence.  Conversely, combine the paired pure columns
of any legal cut and retain its mixed columns.  This proves the Boolean-column formulation without
using the open converse.  The unresolved content is the **row coherence**: one sign must serve all
doubled-column incidences of a row.

This formulation exposes more structure in a tight Hall separator.  Put `h=G_r`, let `H` be its
saturated prefix function, and fix a coherent allocation for a feasible coloring `A/B`.  Suppose
`X subset A`, `Y subset B` is tight, and write

    p=|X|,       q=|Y|,       t=p+q,
    a(X union Y)=H(p)+H(q)+H(t).                              (TB1)

> **Tight Pascal-Band Lemma.** In every child column `T` of capacity `c=c(T)`, the tight set uses
> exactly `min(p,c)` left incidences, `min(t,c)` mixed incidences, and `min(q,c)` right
> incidences.  Consequently every row of `X` has degree at least `h_p+h_t`, every row of `Y` has
> degree at least `h_q+h_t`, every row of `A-X` has degree at most `h_(p+1)+h_(t+1)`, and every
> row of `B-Y` has degree at most `h_(q+1)+h_(t+1)`.

Indeed, each displayed per-column count is an upper bound.  Their sum over all columns is the
right side of (TB1), so equality of the total forces equality in every column separately.  If
`c>=p`, all `p` rows of `X` therefore use the left column; if `c<=p`, that column is filled by
`X` and is unavailable outside it.  The number of child columns with `c>=s` is `h_s`, with
`h_s=0` beyond the padded length of `h`.  Applying
the same argument to the mixed and right columns gives all four degree bounds.

Now consider a Robin--Hood transfer from a donor `i in A-X` to a recipient `j in Y`, with
`a_i>=a_j+2`.  The bounds imply

    h_(p+1)-h_q >= 2 + h_t-h_(t+1).                           (TB2)

For the Pascal columns this has the exact interpretation

    #{T : p<c(T)<q} >= 2 + #{T : c(T)=t}.                    (TB3)

Thus a dangerous opposite-color tight separator necessarily has `q>=p+2` and crosses at least
two child columns in the open count interval `(p,q)`.  For `r>=2`, that band contains an internal
power-of-two capacity with repeated binomial multiplicity; the only multiplicity-one capacities
are the two extreme Boolean ranks, and they cannot supply (TB3) by themselves.  The symmetric
statement holds when the donor is in `B-Y` and the recipient in `X`.

This is the first general reason that duplicated Pascal targets must occur at every genuine
transfer obstruction.  It is stronger than the generic observation that an opposite-color tight
set is the only obstruction: the set saturates the whole dyadic hierarchy term by term, and the
donor-recipient gap forces a nontrivial repeated rank between its two color counts.

The Boolean labels do **not** finish the proof by themselves.  Static Hall capacity has already
summed over all equal columns, so an arbitrary stopped search need not be closed under Boolean
shadows.  Nor can one demand nested or laminar doubled supports.  At `K=3`, the legal state
`(8,7,4,1^8)` forces combined pure-degree profile `(4,4,4,1^6)` or `(4,4,3,1^7)` for doubled
support sizes `8,4,4,2`.  A laminar family cannot realize the first profile because its size-two
member contains at most two points common to all four supports.  For the second, the two
degree-four points force the size-two support into both size-four supports; laminarity makes those
equal, giving at least four points of degree at least three instead of three.  Crossing supports
are already essential at this smallest nontrivial level.

A fixed-color tight separator cannot be defeated by incidence switches that preserve every row's
orientation: the Fixed-Color Hall Lemma says exactly that the transferred demand is infeasible in
that fiber.  Thus an earlier formulation of the proposed plateau descent--complete the transfer
while preserving all signs--was internally impossible.  The residual search has to move among
**feasible colorings of the original state**, rerouting pure incidences while one or more whole
rows change sign, until it finds a coloring with separating margin at least one.

Equal rows should not be marked prematurely.  A transfer between two values of a partition may
choose any donor and recipient rows having those values.  In particular, if some donor-valued and
recipient-valued rows share a color, choosing that pair gives separator margin at least their
value gap: replacing the recipient by the donor preserves all three Hall cardinalities.  Only a
coloring that segregates the two value classes can be dangerous.

The corrected Pascal-first target is a **global color-exchange augmentation lemma**.  Among all
feasible colorings of the original state, maximize the minimum donor-recipient separating margin;
among equal optima, choose a minimum tight dyadic band.  If the margin is zero, (TB3) supplies a
repeated internal Pascal rank crossing that band.  The desired exchange must reroute the pure
incidences of a globally chosen set of rows through the duplicated columns and either raise the
margin to one or produce another feasible coloring with a strictly smaller minimizing band.

The finite data suggests that the required global choice may nevertheless have a very small final
support, once all irrelevant labels are removed.  The correct quotient identifies equal donor and
recipient rows and also exchanges the two pure sides globally.  In that quotient, an exact census
finds no failed coloring at `K=1,2`.  At `K=3`, 348 feasible colorings fail their selected transfer;
325 have a successful coloring at row distance one and the remaining 23 at distance two.  The
first genuine one-row failure is

    a=(8,6,5,3,2,1,1,1),       transfer 5 -> 2,
    A=(5,3,1,1),                B=(8,6,2,1).

An exact prefix census of 10,000 `K=4` states contains 94,936 failed normalized colorings: 74,090
reach success at distance one and 20,846 at distance two.  None needs more.  More specifically,
every failed coloring in all checked landscapes reaches success by one row flip or one
opposite-color swap.  This motivates the stronger **Two-Row Color-Exchange Lemma**: every
failed feasible coloring has another feasible coloring, successful for the transfer, obtained by
one flip or one swap (up to equal-row identity and global color complementation).  It would finish
the Adjacent-Fiber step immediately.  The result is exhaustive only through `K=3`; the `K=4`
prefix is finite evidence, not a proof.  A proof still has to derive the exchange from the
duplicated rank forced by (TB3), rather than merely counting its capacity.  Exact definitions,
counts and reproduction commands are in the
[coloring-landscape record](../../evidence/singleton_coloring_landscape_2026-08-28.md).

Three further disjoint 1,000-state windows after 1,000,000, 3,000,000 and 5,000,000 enumerated
states give the same result.  Across all four `K=4` windows, every one of 377,873 failed colorings
has a one-flip or one-swap repair.  This broadens the finite evidence but does not change its
logical status: only 13,000 of the 5,997,038 `K=4` states were included in this landscape census.

There is a close but incomplete connection to standard bisubmodular exchange.  Define, on disjoint
row sets,

    rho(X,Y)=H(|X|)+H(|Y|)+H(|X|+|Y|).

Because `H` is nondecreasing and concave, each of the three cardinality terms is bisubmodular, so
`rho` is an integral bisubmodular function.  If a coloring is encoded by
`z_i=a_i` on `A` and `z_i=-a_i` on `B`, its Hall inequalities are exactly

    z(X)-z(Y) <= rho(X,Y)             for all disjoint X,Y.

Thus all integral signed demand vectors form the BS-convex set of integer points of the
bisubmodular polyhedron `P(rho)`.  Its abstract delta-exchange directions have support at most two;
see [Iwamasa, Theorem 3](https://arxiv.org/abs/2303.06320).  This is the correct structural analogue
of the observed one-flip-or-one-swap rule.

It is not yet a proof.  The feasible colorings of a fixed state are the boundary slice

    P(rho) intersection {z: |z_i|=a_i for every i}.

Flipping row `i` changes `z_i` by `2a_i`, whereas bisubmodular delta exchange only guarantees unit
moves of support one or two through intermediate points with smaller absolute coordinates.  An
intersection with the fixed-absolute-value slice need not inherit the exchange axiom.  The precise
remaining theorem is therefore a **Pascal fixed-absolute exchange lemma**: for the Pascal `H`, the
unit exchange chain can be compressed to a boundary move consisting of one row flip or one row
swap, chosen so that the transfer margin becomes positive.  The Tight Pascal-Band Lemma supplies
the repeated internal rank needed for that compression; proving that it prevents the chain from
ending in the interior is the unresolved step.

#### Exact boundary-exchange calculus (2026-08-28)

The local flip-or-swap target can be stated without any incidence choices.  Fix a feasible
coloring `A/B`, put

    r(S)=H(|S|)+H(|S intersection A|)+H(|S intersection B|),
    slack(S)=r(S)-x(S),

and write `h_s=H(s)-H(s-1)`, padded by zero.  If a row `v in B` is flipped to `A`, then for a set
`S` containing `v`, with old color counts `(p,q)`,

    r_new(S)-r(S)=h_(p+1)-h_q.                              (FX)

Sets not containing `v` are unchanged.  Hence the flip is feasible for `x` exactly when

    slack(S) >= h_q-h_(p+1)                                (FC)

for every `S` containing `v`.  In particular, every failed `B->A` flip has an `A`-heavy blocker:
the right side can be positive only across a drop of `h` with `q<=p`.

For a swap `u in A`, `v in B`, sets containing both or neither row are unchanged.  Sets containing
`v` but not `u` obey (FC), while sets containing `u` but not `v` require

    slack(S) >= h_p-h_(q+1).                                (SC)

Thus the two possible blocker families have opposite color imbalance.  In the Boolean-column
model, the right sides of (FC) and (SC) count exactly the child columns whose capacities lie in
the corresponding closed interval between the two color counts.

Now let `S=X union Y` be a tight separator for a transfer from `i in A-X` to `j in Y subset B`.
The Tight Pascal-Band Lemma gives `q>=p+2` and

    h_(p+1)-h_q >= 2+h_(p+q)-h_(p+q+1) >= 2.                (CG)

Any successful one-flip repair must therefore flip a row of `Y` from `B` to `A`.  Any successful
one-swap repair must exchange a row of `Y` with a row of `A-X`.  Indeed, every other flip or swap
leaves `r(S)` unchanged or decreases it, whereas the transferred demand on `S` is `x(S)+1`.
Conversely, every such crossing move gains the amount in (CG) on this particular cut.  The only
remaining issue is that it may expose an oppositely imbalanced blocker elsewhere.

Plain dyadic-band descent is false.  At `K=3`, for

    x=(3,2^11,1,1),       transfer 3 -> 1,
    A=(3,2,2,2),          B=(2^8,1,1),

the selected tight separator has counts `(0,10)`.  Swapping the width-three donor with a selected
width-two row is blocked by a set whose old/new counts are `(1,9)->(0,10)`; its closed Pascal
rank-loss band is exactly the original open target band, not a smaller one.  Swapping equal
width-two rows can also reproduce the same value coloring and the same separator.  Thus a proof
cannot follow an arbitrary blocker and use the band alone as a decreasing potential.  A crossing
flip of a width-two row succeeds in this example, so the failure points instead toward a global
choice over the whole cut.

There is a simpler monotone target.  Orient the failed coloring so that the donor is in `A` and
the recipient in `B`, and select a tight transfer separator `S=X union Y`.  Call a crossing move
**positive** when it is feasible for the original demand and either

1. flips a positive-width row `v in Y` from `B` to `A`; or
2. swaps `v in Y` with `u in A-X` and has `x_v>x_u`.

Equivalently, the move strictly increases the total mass colored `A`.  The remaining local claim
can be stated as follows.

> **Positive Pascal Crossing Lemma (open).** Every failed material-row coloring has a tight
> transfer separator across which some positive crossing flip or swap is feasible.

This lemma alone proves the Adjacent-Fiber Lemma; the crossing move need not solve the transfer
immediately.  If it moves the recipient or the donor (but not both, since the donor is at least two
larger), the two marked rows acquire the same color and the same-color argument finishes.  Otherwise
their orientation stays `A/B`.  If the new coloring still fails, repeat from one of its tight
separators.  Each nonterminal step strictly increases the integer `A`-mass, while the pure Hall
inequality bounds that mass by `2*3^(K-1)`, so cycling is impossible and the process terminates at a
common coloring.  A zero recipient is easier: its padded zero row may be assigned the donor's color
without affecting feasibility, and after the transfer the marked rows already share a color.

The arbitrary choice of `S` can now be removed, and with it the apparent alternating blocker
chain.  Let `D` be the family of all `x`-tight sets that contain `j` and omit `i`.  Since `r` is
submodular and `x` is modular, tight sets are closed under union and intersection.  Therefore

    C=intersection_(S in D) S,          U=union_(S in D) S

are themselves dangerous tight sets.  In particular `j in C`, `i notin U`.  Every recoloring that
works for the transfer in one flip or swap must move some `v in C intersection B` to `A`; in the
swap case it must move some `u in A-U` back to `B`.  Otherwise at least one member of `D` gains no
rank.

For `v in C intersection B`, temporarily flip `v` to `A` and let

    N_v={T : r_(A+v,B-v)(T)<x(T)}

be its family of original-demand blockers.  Every member of `N_v` contains `v`.  If `N_v` is empty,
the flip is feasible for both `x` and the transferred demand.  If `N_v` is nonempty, put

    P_v=intersection_(T in N_v) T.

There is an exact global swap rule:

> **Core--Blocker Escape Lemma (open Pascal step).** For some `v in C intersection B`, either
> `N_v` is empty, or `P_v intersection (A-U)` contains a row `u` with `x_u<x_v`.

Indeed, in the second case swap `u` and `v`.  Sets containing `v` but not `u` are safe because
`u in P_v`; sets containing both or neither are unchanged.  A set `T` containing `u` but not `v`
cannot become infeasible.  Under the old coloring its post-swap rank is exactly the old rank of
`T-u+v`, while

    x(T-u+v)=x(T)-x_u+x_v > x(T).

Old feasibility therefore gives the post-swap inequality with at least `x_v-x_u>=1` slack.  This
simple replacement argument eliminates the supposed reverse, `B`-heavy blocker family.

The same flip or swap is already feasible for the transferred demand, not merely a step in a
monotone iteration.  For a swap, an old dangerous tight set contains `v` because `v in C` and omits
`u` because `u notin U`, so (CG) gives it at least two units of new rank.  Any newly tight dangerous
set containing `u` but not `v` has the replacement slack above; a set containing neither or both
would have been an old member of `D`, contradicting `v in C` or `u notin U`; and a set containing
`v` but not `u`, with new color counts `(p,q)`, gains
`h_p-h_(q+1)>=h_(p+1)-h_q>=2`; if it were newly tight, the old coloring would be infeasible.  The
flip case is the same argument without `u`.  Thus the Core--Blocker Escape Lemma implies
Adjacent-Fiber in one recoloring.

This is a sharp **local strengthening**: compute the dangerous tight-set lattice, scan its
`B`-core, and either take a feasible flip or intersect that flip's blockers and take a smaller
outside-`A` row.  It uses no alternation, endpoint special case, exchange cycle, or arbitrary
separator.  Its existence is still unproved, and it is not the sole remaining obligation for the
Row-Coloring Lemma: it asks every failed coloring for an immediately adjacent common coloring,
whereas the exact global statement may recolor and rebuild the incidence realization arbitrarily.
The finite evidence for the earlier maximum-gain
rule also supports this stronger statement: every observed successful one-flip/one-swap neighbor
must cross every member of `D`, hence has exactly the core/hull membership above.

The intersections have a further order structure.  For fixed `v`, `P_v intersection A` is an
upper set in row mass.  If `u in P_v`, `w in A`, and `x_w>=x_u`, then every blocker contains `w`:
otherwise a blocker containing `u` but omitting `w` can replace `u` by `w`, preserving its color
counts and not decreasing its demand, to produce a blocker omitting `u`.  This contradicts
`u in P_v`.  Consequently, if *any* smaller row in `A-U` gives an escape for `v`, then so does the
largest row in `A-U` whose mass is still strictly below `x_v`.  The closest-smaller row is therefore
canonical **after** `v` has been selected from the global core; the failed closest-boundary rules
selected `v` from one arbitrary separator instead.

These upper sets are monotone as the scanned `B` row decreases.  If `v,w in B`, `x_v>=x_w`, and
both flips are blocked, then

    P_v intersection A subset P_w intersection A.                         (ST1)

To see this, map a blocker of the flip of `w` to one for `v`.  If it already contains `v`, it is
unchanged; otherwise replace `w` by `v`.  The post-flip color counts are the same and demand does
not decrease, while the blocker has exactly the same `A` rows.  Thus every `A` row common to all
`v`-blockers is common to all `w`-blockers.

The dangerous lattice has compatible prefix geometry.  `U intersection (A-{i})` is an upper set:
in a dangerous tight set, replacing an included ordinary `A` row by a heavier omitted one would
either violate old feasibility or give another dangerous tight set.  Also, if `v in C intersection
B`, every strictly heavier `B` row lies in `C` (equal rows may exchange their labelled roles).
Hence, after sorting within colors, the Core--Blocker Escape Lemma is a one-dimensional staircase:
the common `A`-prefix `P_v intersection A` grows monotonically as `v` descends through the
dangerous `B`-core, and one must prove that it crosses the ordinary `A`-prefix `U intersection A`
before its newly reached row becomes at least as large as `v`.  The marked donor is the one
possible out-of-prefix row because it is excluded from every dangerous set.

There is also an exact numerical test for one staircase step.  For the canonical candidate
`u=u(v)`, sort

    A(v,u)=(A-{u}) union {v},              B(v)=B-{v}.

Then `u in P_v` exactly when

    prefix_a(A(v,u)) + prefix_b(B(v))
        <= H(a+b)+H(a)+H(b)                                      (ST2)

for every `(a,b)`.  These are precisely the provisional-flip inequalities on sets omitting `u`;
all sets containing `u` are safe after the positive swap by the replacement argument.  Thus this
strong local conjecture can be attacked entirely with sorted prefixes: if every staircase step
fails (ST2), choose one violating pair per step and use (TB3)/(CU) to sum them into a parent-prefix
violation.  Arbitrary labelled blocker sets no longer need to be tracked.

Uncrossing now has a quantitative form that exposes the remaining work on that route.  Let
`S=X union Y in D` have counts `(p,q)`, and let a blocker `T=P union Q in N_v` have old counts
`(a,b)`, slack `delta`, and flip loss `L=h_b-h_(a+1)>delta`.  If `Delta` is the submodularity
defect of `r` on `S,T`, tightness of `S` gives

    slack(S intersection T)+slack(S union T)=delta-Delta,

so `Delta<=delta<L`.  For every child column whose capacity `c` lies in both bands

    p<c<q,                         b<=c<=a,

the `A`- and `B`-rank terms contribute exactly

    |X minus P| + |Q minus Y|

to `Delta`.  If `m` Pascal columns lie in the band intersection, then

    m (|X minus P|+|Q minus Y|) <= Delta < L.                  (CU)

In particular, when the entire blocker band lies in the target band, `m=L` and (CU) forces the
signed nesting `X subset P`, `Q subset Y`.  The missing general argument must use the duplicated
columns guaranteed by (TB3), across all `v in C intersection B`, to force one blocker's common
intersection past `U` onto a smaller `A` row, or else sum the residual nonnested defects into a
violated parent prefix inequality.  Only the `A`-heavy flip blockers remain; an alternating-cut
descent is unnecessary.

The finite evidence remains substantial.  Through `K=3` and in the first 5,000 `K=4` states,
every maximum-`A`-mass crossing neighbor is successful for the transfer, every tie at the maximum
is successful, and the smallest observed maximum gain is one.  Three further disjoint windows also
have a successful maximum-gain crossing neighbor.  These observations do not prove the
Core--Blocker Escape Lemma.

Standard delta-matroid exchange does not supply that termination.  The fixed-absolute coloring
family is not even a delta-matroid in general for Pascal `H`.  At `K=3`, take

    x=(3,2^11,1,1),

index the width-three row by `0` and the first width-two rows by `1,2,...`, and let the `A`-sets be

    P={0,1,2,3},             Q={1,2,3,4,5}.

Both colorings are feasible.  For `e=0 in P triangle Q`, symmetric exchange permits only the flip
`P triangle {0}` or the two swaps `P triangle {0,4}` and `P triangle {0,5}`.  The flip leaves only
three `A` rows, so the full-set capacity is 26 below mass 27.  Either swap leaves the other side
with mass 19, above its pure capacity `2H(10)=18`.  All three candidates are infeasible.  The
expanded-coordinate theorem for jump systems remains valid; restricting each expanded block to
be all-in or all-out is the step that destroys symmetric exchange.

The exact diagnostic also rejects simpler ways of preselecting the crossing rows.  Through `K=3`,
all 325 failed colorings whose recipient side is above its `2^(K-1)` row floor have a one-row
repair, while the 23 failures at the floor need a swap.  In the first 5,000 `K=4` states, however,
516 above-floor failures also need a swap.  Two natural closest-boundary swap prescriptions still
miss 170 failed colorings in that window.  Therefore the live statement is existence of *some*
crossing move, selected from the whole cut, not a fixed endpoint or closest-value rule.  The exact
counterexamples and reproduction commands are in the
[boundary-exchange record](../../evidence/singleton_boundary_exchange_2026-08-28.md).

Fix a number of labelled parent rows.  Let `F_h` be the set of integer demand vectors that admit a
legal allocation to left, mixed and right columns of capacities `h'`: a row uses a column at most
once and may use `{left,mixed}` or `{mixed,right}`, but not both pure sides.  The maximum demand
that can be supported on any specified `t` rows is

`max_(p+q=t) D(p,q) = H(t)+H(ceil(t/2))+H(floor(t/2)) = H_K(t).`

Consequently, the Row-Coloring Lemma would follow at once if `F_h` were an **integral
polymatroid**.  Its rank inequalities would then be exactly

`sum_(i in S) a_i <= H_K(|S|),`

which, by symmetry, are the parent majorization inequalities.  Equivalently, it is enough to prove
the discrete augmentation/exchange axiom for legal allocations.  This is the cleanest bottom-up
version of the coin-transfer proposal: an added or transferred unit is routed along an augmenting
path, possibly recoloring several rows, until it reaches one of the identical dyadic columns.
At full mass this is the same M-convex exchange property used for supports of generalized
permutahedra; see
[Matherne--Morales--Selover, Section 1.3.1](https://arxiv.org/html/2201.07333v4#S1.SS3.SSS1).

This formulation does not yet prove the exchange axiom or the balanced row orientation in (D).
The examples below show why a one-step
greedy augmentation is too strong, but they do not rule out a global augmenting-path proof.  For a
generic base `h`, `F_h` is not a polymatroid, consistently with the counterexamples above.  The
remaining structural question is whether the Pascal multiplicities and power-of-two column
capacities make it one for `h=G_r`.

### Padded pure-first allocation (2026-08-27)

Padding to `N=3^K` labelled row slots does give a cleaner exact target for a universal
construction.  Put `h=G_(K-1)`, and list the conjugate column capacities of `h` as

    h'=(c_1,...,c_s),          sum_j c_j=3^(K-1).

Let `a=(a_i:i in E)` be a full-mass parent state on the fixed slot set `E`, including its zero
rows.  A **pure-first allocation** consists of

1. an orientation `E=A disjoint-union B`;
2. for every `j`, supports `L_j subset A` and `R_j subset B` with
   `|L_j|=|R_j|=c_j`; and
3. pure degrees

       p_i=#{j:i in L_j or i in R_j}

   satisfying `p_i<=a_i`, such that the residual `m_i=a_i-p_i` is majorized by `h`.

This is exactly equivalent to a legal first cut; it is not an additional conjecture.  Given the
data above, the `L_j` and `R_j` are already incidence matrices for the two pure children.  Since
`m<=_w h` and both have mass `3^(K-1)`, Gale--Ryser supplies a `0`-`1` incidence matrix with row
degrees `m` and column degrees `(c_j)`, which is the mixed child.  Conversely, realize each child
of any legal cut by such a matrix, pair equal-capacity left and right columns, and orient every row
toward the pure child it uses; rows using no pure child may be oriented arbitrarily.  This recovers
the pure-first data.

Thus the proposed order of construction is valid: place the pure incidences first and leave the
mixed child to an ordinary degree-sequence theorem.  Padding is useful because rows never appear
or disappear, zero slots remain explicit vertices that can be oriented freely, and orientation
and pure placement are the only coupled choices.  It does not by itself choose those supports.

One tempting global choice is still too rigid.  Fix a linear alternating order of the `N` slots
and require each doubled pure support `L_j union R_j` to be a consecutive interval of length
`2c_j`; it is then automatically split evenly between `A` and `B`.  At `K=3`, however,

    h=G_2=(4,3,1,1),       h'=(4,2,2,1),
    a=(8,7,4,1^8) <=_w G_3=(8,7,4,4,1^4).

This state does have a legal cut: assign row triples `(left,mixed,right)` as

    8 -> (4,4,0),       7 -> (0,3,4),       4 -> (3,1,0),
    two 1s -> (1,0,0),  five 1s -> (0,0,1), one 1 -> (0,1,0).

The children are `(4,3,1,1)`, `(4,3,1,1)`, and `(4,1^5)`, all majorized by `h`.
Nevertheless, the four interval lengths would be `8,4,4,2`.  In every pure-first allocation, the
row of size eight must have `(p,m)=(4,4)`.  The row of size seven must have `(p,m)=(4,3)`: pure
degree is at most four, while a second mixed degree four would violate the first two inequalities
for `m<=_w h`.  The remaining mixed mass is two, and the size-four row can retain at most one of it.
Consequently the pure coverage profile must be one of

    (4,4,4,1^6)       or       (4,4,3,1^7).                    (E)

Neither is the coverage profile of intervals of lengths `8,4,4,2`.  The first would require three
points common to all four intervals, impossible because the shortest interval has length two.  In
the second, the two degree-four points must be exactly the length-two interval.  Removing it leaves
three intervals of lengths `8,4,4` with coverage `(3^3,1^7)`: their triple intersection has length
three and there are no degree-two points.  The two length-four intervals must therefore extend a
common three-point block on opposite sides.  The length-eight interval contains the block and at
least one of those two extensions, creating a degree-two point, a contradiction.

The useful algorithmic target is therefore an **adaptive balanced-support augmentation**.  Insert
the paired pure columns in decreasing capacity, rerouting earlier incidences along alternating
paths when a side lacks `c_j` usable rows; a row may change orientation only after its pure
incidences have been rerouted.  Once all pairs are placed and the residual remains majorized by
`h`, Gale--Ryser finishes the mixed child.  Choosing, say, the shortest lexicographically first
augmenting path would make this a universal deterministic algorithm.  The still-unproved step is
that an augmenting path always exists.  A failed augmentation exposes an opposite-orientation
tight cut, so proving that the Pascal multiplicities always let that cut be crossed is the same
separator-elimination problem isolated by the adjacent-fiber formulation above.

#### Equal blocks and the canonical mixed-only tail

The `3^K` padding also sharpens the row-partition version.  Put `M=3^(K-1)` and divide the padded
slots into equal blocks `A,B,C`, where `A` rows may feed left and mixed, `B` rows may feed mixed and
right, and `C` rows may feed only mixed.  If `A_p,B_q,C_r` denote the corresponding largest-row
prefix sums, the integral Hall criterion is exactly

    A_p+B_q+C_r <= H(p)+H(q)+H(p+q+r)                 (P1)

for all `0<=p,q,r<=M`.  The conditions `mass(A)>=M` and `mass(B)>=M` are only the two complementary
whole-block cases of (P1).

There is no need to search over `C`.  In any feasible partition, swapping a heavier `C` row with a
lighter `A` or `B` row preserves every Hall inequality: a set containing the heavier row gains
pure capacity, while a set containing only the lighter row is certified by replacing it with the
heavier row before the swap.  Hence `C` may always be taken to be the `M` smallest padded rows.
They are necessarily zeros and ones.  If their mass is `c` and `E=M-c`, minimizing (P1) over its
`r` coordinate gives the exact contracted condition

    A_p+B_q <= H(p)+H(q)+min(H(p+q),E).               (P2)

Thus the equal three-block formulation canonically chooses its mixed-only block and reduces back
to a two-color problem with a tail-truncated mixed child.

This reduction is especially clean when the parent has at least `2M` nonzero rows.  Its remaining
`2M` rows have prefix bound

    U_E(t)=min(H_K(t),E+t).                           (P3)

The second term is the total-mass bound after charging one unit to every unselected positive row.
It implies all balanced instances of (P2).  If the remaining rows are colored alternately by
rank, pairwise averaging also proves every off-diagonal inequality on the side receiving the
smaller row of each pair.  The other side would follow from the single arithmetic family

    floor((U_E(2q+1)+U_E(2p-1))/2)
      <= H(p)+H(q)+min(H(p+q),E),        p>q.         (P4)

An exact breakpoint checker verifies (P4) for every integer `E` through `K=12`, but (P4) is false.
At `K=19`, `E=M=3^18`, `p=513`, and `q=256`, its left side exceeds its right side by 2,431.
This is not only a failure of the envelope `U_E`: a compressed construction realizes the relevant
prefixes as an actual sorted, full-mass parent majorized by `G_19`, with exactly `2M` positive
rows.  Strict alternation therefore does not settle even the high-support regime.

The canonical-tail exchange and contraction (P1)--(P3) remain exact.  A useful high-support
subproblem keeps adjacent pairs but chooses their orientations globally.  (This cannot be a proof
for all states: adjacent-pair orientation already fails on 916 lower-support `K=4` states.)  Write
pair `i` as `x_i>=y_i`, put `d_i=x_i-y_i`, choose `epsilon_i in {+1,-1}` according to which color
receives `x_i`, and define `D_n=sum_(i<=n) epsilon_i d_i`.  If `P` is the parent prefix function,
then

    A_p+B_q = (P(2p)+P(2q)+D_p-D_q)/2.

Thus all contracted Hall inequalities become simultaneous interval bounds on the signed walk
increments `D_p-D_q`.  This formulation explains why balancing only the current total is not the
right invariant: the constraints are on every interval, not just prefixes from zero.  It is still
an existence problem, not a proof, and solving it would cover only the regime where paired rows
are justified.  The derivation, retraction, exact counterexample and commands are in the
[padded three-block record](../../evidence/singleton_padded_three_blocks_2026-08-27.md).

### A shape-preserving forest target (2026-08-27)

There is a precise version of the proposed “children of similar shape” idea.  For an integer
partition `a`, define its hinge, or coalescence, profile

    E_a(t) = sum_i max(a_i-t,0),                    t=1,2,... .

For equal-mass partitions, `a <=_w g` is equivalent to `E_a(t)<=E_g(t)` at every integer
threshold.  The discrete derivative of this profile is the conjugate row-count profile
`c_t(a)=#{i:a_i>=t}`.  Thus this is the whole majorization shape, not a new scalar score.

In a legal first cut, one parent row of width `u` is either left intact or split as `u=x+y`
between the mixed child and one pure child.  Put

    j_(x,y)(t) = min(x,t)+min(y,t)-min(u,t).

Direct cancellation gives the exact aggregate identity

    sum_(children C) E_C(t) = E_a(t)-J_a(t),
    J_a(t) = sum_(parent rows u=x+y) j_(x,y)(t).                 (G)

At `t=1`, every genuinely split row contributes one and every intact row contributes zero, so
`J_a(1)` is exactly the number of split parent rows.  More generally, the largest possible
contribution of a row `u` at threshold `t` is `min(t,max(u-t,0))`.  Hence the parent itself imposes
the sharp coordinatewise capacity

    J_a(t) <= E_a(t)-E_a(2t).                                  (H)

In particular, (H) at `t=1` is `J_a(1)<=#{i:a_i>=2}`.  The previously observed cap by the
number of nonunit rows is therefore the first coordinate of a canonical hierarchy, not an
exception for unit rows.

Let `g=G_K`, `h=G_(K-1)`, and define the canonical top-layer removal profile

    B_K(t) = E_g(t)-3E_h(t).

It is nonnegative because the canonical first cut of `G_K` has three children equal to `h`, and
(G) says that its cut profile is exactly `B_K`.  If `E_g(t)>0`, the literal same-shape target is

    J_a^*(t) = (E_a(t)/E_g(t)) B_K(t),

capped by (H) and rounded integrally.  At thresholds where `E_h(t)>0`, without the cap or rounding,
substituting this value in (G) makes the average normalized child hinge coordinate equal to the
parent's:

    (sum_C E_C(t))/(3E_h(t)) = E_a(t)/E_g(t).

The first coordinate has an especially simple form.  If `r` is the number of rows, put

    theta(a) = (3^K-r)/(3^K-2^K).

Since `B_K(1)=2^(K-1)`, same average shape asks for

    J_a(1) = 2^(K-1) theta(a),

up to floor/ceiling and the forced cap `#{i:a_i>=2}`.  Equivalently, view every row of width `u`
as a binary tree with `u` leaves and `u-1` joins.  Then `theta` is the fraction of the
`3^K-2^K` joins of the canonical forest retained by `a`, and the equation removes that same
fraction of the `2^(K-1)` joins at the top recursive layer.

The most literal **Shape-Preserving Cut** proposal would ask every full-mass `a<=_w G_K` for a
legal first cut whose three children are each majorized by `G_(K-1)` and whose cut profile `J_a`
simultaneously takes the floor or ceiling of

    min((E_a(t)/E_g(t)) B_K(t), E_a(t)-E_a(2t))

at every relevant threshold.  The statement is **false already at `K=3`**.  The 61st state in the
exact descending census is

    a=(8,5,5,5,1,1,1,1).

Its coordinate targets include `J_a(1)=4` and `J_a(2)=6`.  It has legal children, for example

    (4,3,1,1), (4,2,2,1), (4,3,1,1),

but that cut has `J_a(1)=4`, `J_a(2)=7`, and exhaustive search finds no cut meeting all rounded
targets simultaneously.  Thus the full hinge profile is the right intrinsic language, but its
coordinates have genuine lattice coupling and cannot be normalized independently.

The first-coordinate shadow remains a clean **Scalar Shape-Balance Conjecture**: require only

    J_a(1) in floor/ceiling(min(2^(K-1) theta(a), #{i:a_i>=2})).

This is still stronger than the Row-Coloring Lemma and is not proved.  An exact reconstruction
survey finds no failure among all 1,206 full-mass states at `K=3` or all 5,997,038 at `K=4`.
`tools/singleton_shape_survey.cpp` reproduces both censuses and the full-profile counterexample.
The scalar rule is therefore a plausible coarse induction invariant, not by itself a certificate:
the proof must still produce three individually `G_(K-1)`-majorized children.

A tempting local strengthening is already false.  Call a positive piece an **atom-sized piece**
when its width occurs as a row width of `G_(K-1)`, and require every parent row's one- or
two-piece image under the first cut to contain one.  At `K=3`,

    a=(8,7,4,2,2,2,1,1) <=_w G_3=(8,7,4,4,1,1,1,1)

is a counterexample.  The child atom widths are `{4,3,1}`.  The rows `8` and `7` must contribute
at least `4` and `3` respectively to the mixed child, while each of the three rows of width `2`
must split as `1+1` and contribute another one.  These five rows force mixed mass at least ten,
greater than its required mass nine.  The state nevertheless has an ordinary legal cut:

    8 -> (4,4,0),  7 -> (0,3,4),  4 -> (3,1,0),  2 -> (1,1,0),
    2 -> (0,0,2),  2 -> (0,0,2),  1 -> (1,0,0),  1 -> (0,0,1),

whose children are `(4,3,1,1)`, `(4,3,1,1)`, and `(4,2,2,1)`.  This repair leaves two
non-atom-sized rows intact; demanding that the atom-sized piece be the larger one is therefore
false as well.  This does not refute the weaker condition imposed only on genuinely split rows.
The exact atom-restricted modes of `tools/singleton_shape_survey.cpp` reproduce the obstruction.

### Global signed lifting and the no-holes target (2026-08-26)

There is a cleaner global formulation that removes value blocks and forward choices entirely.
Fix a labelled row set `E` and, for disjoint `X,Y subset E`, define

    f(X,Y) = H(|X union Y|) + H(|X|) + H(|Y|).

Define the signed Hall polyhedron

    P(f) = { z in R^E : z(X)-z(Y) <= f(X,Y) for every disjoint X,Y }.

If `A={i:z_i>=0}` and `B={i:z_i<0}`, inequalities with rows carrying the wrong sign can be
deleted from `X,Y`; this increases the left side and decreases the right side.  Therefore
`z in P(f)` is equivalent to (C) for the coloring `A/B` and row demands `|z_i|`.

The saturated prefix function `H` is nondecreasing and concave.  Hence `S -> H(|S|)` is
submodular.  Under the bisubmodular meet, the positive and negative sets become their respective
intersections; under the reduced union, each is contained in its ordinary union.  Submodularity
and monotonicity therefore prove the required inequality separately for `H(|X|)` and `H(|Y|)`.
For `H(|X union Y|)`, the supports of the meet and reduced union are contained in the ordinary
intersection and union of the two supports, so the same argument applies.  Adding the three
inequalities proves that `f` is bisubmodular.  Thus the integral signed feasible set

    Z_h = P(f) intersection Z^E

has the standard signed-exchange, or BS-convex, structure of an integral bisubmodular
polyhedron.  See [Iwamasa 2023](https://arxiv.org/abs/2303.06320) for the exchange
characterizations.  Bisubmodularity itself is not the missing proof: the generic bases above
produce the same kind of `f` and still have uncolorable parent demands.

Fold the signed set into the nonnegative orthant:

    M_h = { |z| : z in Z_h }.

Also put

    R(S) = max_(X disjoint-union Y=S) f(X,Y)
         = H(|S|)+H(ceil(|S|/2))+H(floor(|S|/2))
         = H_K(|S|).

The middle equality follows from concavity of `H`: the maximum splits `S` as evenly as
possible.  Every `x in M_h` satisfies `x(S)<=R(S)`, by taking `X` and `Y` to be the positive
and negative coordinates of its signed lift inside `S`.

In fact the convex hull is already exact:

    conv(M_h) = { x in R_+^E : x(S)<=R(S) for every S subset E }.                 (E)

For the reverse inclusion, the vertices of the cardinality-based polymatroid on the right are
coordinate permutations of `(G_K[1],...,G_K[t],0,...)`.  Each is in `M_h`: take the canonical
decomposition of `G_K` and delete its rows after `t`.  This proves (E).

Consequently the Row-Coloring Lemma is exactly the following **Pascal Orthant-Saturation
Lemma**:

    M_h = conv(M_h) intersection Z_+^E,              for h=G_(K-1).              (F)

In words, folding the integral signed Hall polyhedron creates no lattice holes.  Generic `h`
can create holes, as the explicit counterexamples above demonstrate.  The complete `K<=4`
census says that the Pascal folds have no holes at those levels.

### An equivalent balanced-column realization

There is a second exact formulation in which the Pascal structure is present before any coloring
is chosen.  Let `c=h'` be the conjugate child partition.  Pascal identity (D) says that the
conjugate parent capacities consist, for every `c_t`, of one **doubled column** of degree `2c_t`
and one **single column** of degree `c_t`.  For a full-mass demand `x`, Gale--Ryser gives

    x <=_w G_K

if and only if there is a `0`--`1` row/column incidence matrix with row degrees `x` and exactly
those doubled and single column degrees.

The Row-Coloring Lemma is equivalent to asking for such a matrix together with a coloring of its
rows `A/B` for which every doubled column is perfectly balanced:

    |N(D_t) intersection A|=|N(D_t) intersection B|=c_t.          (BR)

Indeed, merge the left and right copies of child column `t` in any legal decomposition.  A row
cannot use both pure children, so the merged column is still `0`--`1`; full mass makes its degree
`2c_t`, the mixed column has degree `c_t`, and the row orientation gives (BR).  Conversely, split
each doubled column according to the two row colors and retain each single column as the mixed
copy.  Every resulting child column has degree `c_t`, and a row uses only its chosen pure side and
the mixed side, so this is a legal decomposition.

Equivalently, pair the `c_t` rows of one color in a doubled column with its `c_t` rows of the other
color.  The doubled columns become matchings of prescribed sizes whose union is bipartite; the
single columns are unrestricted subsets of the same prescribed sizes.  Thus the exact global
problem is:

> **Balanced Pascal Realization Lemma (open, equivalent form).** Every full-mass row-degree
> sequence majorized by `G_K` has a realization by the doubled/single Pascal columns in which the
> doubled-column neighborhoods admit the common exact bisection (BR).

The binomial quotas make this still more symmetric.  Index the conjugate parent columns by their
rank `ell`.  There are

    n_ell=binomial(K,ell)

columns of the identical degree `2^(K-ell)`.  Pascal identity splits this pool into

    b_ell=binomial(K-1,ell)       balanced doubled columns,
    s_ell=binomial(K-1,ell-1)    unrestricted single columns.    (BQ)

Here an out-of-range binomial coefficient is zero.  Thus the doubled/single labels are not fixed
inside a degree pool.  The exact requirement is merely that at least `b_ell` of its `n_ell`
columns contain half `A` rows and half `B` rows.  Choosing those columns as doubled and the other
`s_ell` as single recovers (BR), and every (BR) realization clearly has these quotas.  This is an
equivalent **Binomial Balanced-Columns** form of the lemma.  It displays the Pascal structure as a
rank-by-rank allowance: all columns at `ell=0` must balance, while the allowed unbalanced count
grows from `binomial(K-1,ell-1)` and reaches the sole degree-one column at `ell=K`.

The top quota is never the obstruction.  A full-mass `x<=_w G_K` has at least `2^K` positive
rows; otherwise its whole mass would occur before the positive support of `G_K` ends, violating a
prefix inequality.  Apply the bipartite Havel--Hakimi (Ryser) reduction to the unique largest
parent column, whose degree is `2^K`: connect it to the `2^K` largest residual row degrees, subtract
one there, and the remaining row/column margins are still realizable.  Color exactly half of those
`2^K` rows `A` and half `B`, then color all other rows arbitrarily.  This gives a realization in
which the unique `ell=0` column is balanced.  Since `ell=K` has balance quota zero, a first failure
in a rank-lexicographically optimized realization must lie at an internal rank

    1<=ell<=K-1.                                                (BQ0)

### Boolean coordinate exposure, not accidental multiplicity

The quotas (BQ) are literally deletion and contraction of one Boolean coordinate.  Label the
parent columns by subsets `S subset [K]`, with

    degree(S)=2^(K-|S|).

Expose a prospective first-test coordinate `t`.  If `t notin S`, the corresponding child column
has half the parent capacity and occurs once in each pure branch, so `S` must be bisected between
the two row colors.  If `t in S`, deleting `t` leaves one child column of the same capacity, so
`S` is unrestricted and belongs to the mixed branch.  At rank `ell`, the two cases have cardinalities
`binomial(K-1,ell)` and `binomial(K-1,ell-1)`.  Conversely, because columns of one rank have
identical degrees, any choice meeting (BQ) can be labelled by the subsets omitting or containing a
fixed `t`.  Hence the Balanced Pascal Realization Lemma is exactly:

> **Boolean Coordinate-Exposure Lemma (open, equivalent form).** Realize the row degrees `x` on
> the subset columns above and expose one coordinate `t` so that every column not containing `t`
> is exactly bisected by one row coloring; columns containing `t` are unrestricted.

This is the first-test deletion/contraction operation itself, not a generic realization aided by
fortunate repeated capacities.

One tempting use of the coordinate symmetry gives no new theorem.  For a row subset containing
`p` selected `A` rows and `q` selected `B` rows, the capacity after exposing `t` is

    sum_(ell=0..K-1) binomial(K-1,ell)
        (min(p,2^(K-1-ell))+min(q,2^(K-1-ell)))
      + sum_(ell=1..K) binomial(K-1,ell-1)
        min(p+q,2^(K-ell))
      = H(p)+H(q)+H(p+q).                                      (CE1)

Maximizing over `p+q=s` balances `p,q` and gives exactly the parent prefix rank `H_K(s)`.  Thus
maximizing away the color counts discards the off-diagonal integer choice and returns the
already-proved convex-hull equality.  It cannot rule out a lattice hole.

There is also an exact limit on what the subset labels contribute.  Fix an unlabelled incidence
realization and a row coloring.  At each rank `ell`, label its columns by the `ell`-subsets of
`[K]` independently of every other rank.  The realization admits an exposure of a fixed coordinate
`t` if and only if at least `binomial(K-1,ell)` columns at every rank are bisected: label any such
columns by the subsets omitting `t` and label the remainder by the subsets containing `t`.
Conversely, an exposure supplies exactly those bisected columns.  Call this the
**Independent-Relabelling Lemma**.

Consequently the Boolean labels do not themselves impose any lower- or upper-shadow relation
between adjacent ranks.  Such a relation can be destroyed by independently permuting the labels
inside one rank without changing the incidence matrix, coloring, or legality.  A
Kruskal--Katona/LYM argument would become legitimate only after an augmenting construction produced
a distinguished *coupled* labelling and proved that its reached sets were shadow-closed.  No such
construction is currently known, so assuming shadow closure would merely add an unproved stronger
hypothesis.

What survives relabelling, and is therefore intrinsic, is the complete Pascal package: rank
`ell` has degree `2^(K-ell)`, deletion quota `binomial(K-1,ell)`, contraction allowance
`binomial(K-1,ell-1)`, and the deletion columns at rank `ell` can be paired arbitrarily with the
contraction columns at rank `ell+1`.  The remaining integral target is to combine these data with
the switch-minimal defect relations (BR0)--(BR4).  If `Phi>0`, one must turn the resulting
rank-by-rank defects and forced doubled/single nestings into a violated parent prefix; proving that
implication is the unresolved Pascal step.  A scalar sum over ranks, or Boolean shadows supplied
only by relabelling, is insufficient.

This formulation does not assume that a previously selected coloring can be repaired locally.
It is therefore an exact restatement of the Row-Coloring Lemma, unlike the stronger local targets
below.  A possible constructive proof would start with an arbitrary Gale--Ryser realization and
use degree-preserving switches, while choosing the pairings inside identical doubled columns, to
remove odd cycles from the union of the paired matchings.  What is missing is a Pascal-specific
global switching argument; merely repairing one failed coloring is not logically necessary.

The global switching space has an immediate normalization that the fixed-color route obscures.
Among all incidence realizations with the prescribed margins and all row colorings, minimize

    Phi=sum_(doubled D_t) delta_t^2,
    delta_t=|N(D_t) intersection A|-c_t.

Also minimize over which columns receive the doubled labels in each equal-degree pool.  Hence a
selected doubled column `D(c)` and an unselected single column `S(2c)` of the same actual degree
`2c` obey

    |delta_D| <= ||N(S(2c)) intersection A|-c|.                  (BR0)

Otherwise exchanging their type labels preserves the entire incidence matrix and all margins but
strictly lowers `Phi`.  In particular, if a rank pool supplies fewer than its quota of balanced
columns, every unselected column in that pool is also unbalanced; the doubled labels already mark
the columns closest to half-and-half.

For two doubled columns `D,E` of the same capacity `2c`, minimality gives

    |delta_D-delta_E|<=1.                                      (BR1)

Indeed, if `delta_D>=delta_E+2`, then `D` contains more `A` rows than `E`, while `E` contains
more `B` rows than `D`.  Choose `u in A intersection (D-E)` and
`v in B intersection (E-D)`, and interchange the two incidences.  This preserves every row and
column degree, changes the two imbalances to `delta_D-1,delta_E+1`, and strictly decreases `Phi`.
The reverse inequality is symmetric.

The same argument across capacities gives the useful Lipschitz bound

    |delta_D-delta_E|<=max(1,|c_D-c_E|).                         (BR1')

For if, say, `delta_D-delta_E` exceeds both terms on the right, then
`c_D+delta_D>c_E+delta_E` and `c_E-delta_E>c_D-delta_D`; the same opposite-color interchange
exists and lowers `Phi`.  In particular, (BR1) means that one capacity class cannot contain both
a positive and a negative defect: it has one sign, with zero possibly present at the boundary.

There is also an exact doubled/single nesting consequence.  Let `D` have capacity `2c` and let
`S` be any single column of the matching degree `c`.  If `delta_D=d>0`, then

    N(S) intersection B subset N(D) intersection B,
    |N(S) intersection A|>=d.                                  (BR2)

The first inclusion fails exactly when there are
`u in A intersection (D-S)` and `v in B intersection (S-D)`; the same incidence interchange
reduces `d` and hence `Phi`.  Such a row `u` always exists because `D` contains `c+d>c=|S|`
rows of color `A`.  The cardinality bound follows from the inclusion.  For `d<0`, the color-reversed
statement holds: `N(S) intersection A subset N(D) intersection A` and
`|N(S) intersection B|>=-d`.

Finally, minimality under flipping one whole row couples the capacity classes.  If `p_u` is the
number of doubled columns containing row `u`, then

    2 sum_(D contains u) delta_D <= p_u,       u in A,
    2 sum_(D contains u) delta_D >= -p_u,      u in B.           (BR3)

For an `A` row, a flip changes each incident term from `delta_D^2` to
`(delta_D-1)^2`; for a `B` row it changes it to `(delta_D+1)^2`.  Nonnegativity of those two
changes is exactly (BR3).  Summing the first inequality over `A` rows and the second over `B` rows
also gives the coarse global bound

    2 sum_D delta_D^2 <= sum_D c_D.                             (BR4)

Thus defects of one sign cannot simply accumulate independently across the Pascal levels; every
row meeting them must receive compensating zero or opposite-sign incidences as prescribed by
(BR3).

Thus a switch-minimal counterexample cannot have unrelated defects in the many identical Pascal
columns.  At each power-of-two capacity, all doubled-column imbalances take at most two consecutive
values of one sign, capacity levels obey (BR1'), every nonzero value forces nesting against every
single column at that capacity, and rows obey (BR3).  The
remaining exact task on this route is to combine those per-capacity defects using the binomial
multiplicities and show that either another switch exists or their aggregate violates a parent
prefix inequality.  Unlike the Core--Blocker staircase, (BR1)--(BR2) were obtained by optimizing
globally over realizations and colorings and therefore do not strengthen the desired conclusion.

An equivalent transfer form of the same universal full-mass claim is the following.  If
`x in M_h` and `x_i>=x_j+2`, then

    x-e_i+e_j in M_h,                                                        (T)

where the signed lift and its row coloring may change everywhere.  Starting from a permutation
of `G_K`, repeated unit Robin-Hood transfers generate every full-mass integer vector majorized
by `G_K`; (T) would therefore prove (F), and artificial unit rows handle smaller mass.

As universal full-mass statements, (T) and (F) are equivalent: (F) immediately puts the more
balanced neighbor back in `M_h`, while (T) reaches every dominated vector from `G_K`.  The next
statement is different.  Requiring the two endpoints of every transfer to share one feasible
coloring is a genuine strengthening of (T); no implication from global transfer closure to that
common-fiber property has been proved.

This is the precise global form of the bottom-up coin-transfer proposal.  In the column model,
remove one incidence from donor row `i` and seek an alternating row/column path that installs it
at recipient `j`, possibly flipping several whole-row orientations.  If the path search stops,
its reached rows and doubled/single columns give a Hall cut.  The remaining Pascal-specific
proof obligation is to use the power-of-two capacities and binomial multiplicities in (D) to
symmetrize that cut and turn it into a violated parent rank inequality.  Ordinary
bisubmodular exchange does not automatically do this after the absolute-value fold.

There is a sharper local version of exactly that obligation.  For a fixed coloring `A/B`, put

    r_(A,B)(S)=H(|S|)+H(|S intersection A|)+H(|S intersection B|).

The Fixed-Color Hall Lemma says that a labelled demand vector `x` is feasible for this coloring
exactly when `x(S)<=r_(A,B)(S)` for every row set `S`.  Consider one Robin-Hood transfer

    y=x-e_i+e_j,                 x_i>=x_j+2.

The same coloring is feasible for both `x` and `y` exactly when no `x`-tight set contains `j` but
not `i`.  Indeed, those and only those inequalities increase, and they increase by one.  Moreover,
if `i,j` have the same color, such a tight set is impossible: replacing `j` by `i` preserves all
three cardinalities in `r_(A,B)` but increases its demand by at least two, contradicting the
feasibility of `x`.

Thus only an opposite-color tight separator can obstruct a transfer.  This isolates the following
particularly economical sufficient statement.

> **Pascal Adjacent-Fiber Lemma (open).** If `x in M_h`, `h=G_(K-1)`, and
> `y=x-e_i+e_j` is a Robin-Hood transfer, then some row coloring is feasible for both `x` and `y`.

The coloring may depend on the chosen transfer; asking one coloring to support every outgoing
transfer is stronger and is not needed.  The Adjacent-Fiber Lemma would prove (T), hence (F), by
walking from a permutation of `G_K` through unit Robin-Hood transfers.  In tight-set language, its
entire unproved content is: among the feasible colorings of `x`, one can eliminate every tight set
that separates this particular opposite-color pair.  A plausible proof would choose a coloring
maximizing the minimum separator slack and use bisubmodular uncrossing to recolor across a minimal
tight separator.  What is still missing is the Pascal-specific recoloring step that either raises
that slack or turns the separator into a violated parent-rank inequality.

An exact census now verifies this common-coloring statement for every full-mass state and every
normalized donor-value/recipient-value transfer through `K=4`: all 141,690,676 `K=4` transfer
types pass.  For `K<=3`, exhaustive maximization gives best separator margin at least two.  At
`K=4` every transfer also has a certificate of margin at least two.  Transfers without a
same-color certificate have their opposite-color fiber exhaustively maximized; ordinary
same-color successes stop at their first certificate, so maxima above two are not claimed.  This
is strong finite evidence, not the missing recoloring proof.

The tempting stronger claim that donor and recipient can always share a color is false at `K=4`.
For

    x=(16,15,11,9,7,5,5,5,1^8),       donor=11, recipient=9,

no feasible coloring puts the two marked rows together, while the opposite-color assignment

    A=(15,11,5,5,1^4),       B=(16,9,7,5,1^4)

is feasible before and after the transfer.  Thus a proof really must eliminate opposite-color
tight separators rather than normalize them away.  Counts, exact search method, commands and
provenance are in the
[Adjacent-Fiber census record](../../evidence/singleton_adjacent_fiber_census_2026-08-27.md).

The 889 exceptional `K=4` transfers are one family, not unrelated pathologies: they are exactly
the states `(16,15,11,9,lambda)` with donor 11, recipient 9 and a majorized tail of mass 30.  The
first four rows alone make a same-color assignment impossible.  Rows 16 and 15 must be opposite by
the pure `(2,0)` inequality; if 11 and 9 were together, the four rows would have color counts
`(3,1)` and demand 51 against capacity 49.  The exact opposite-color optimum is two in every case;
the selected optimal certificate always has unique minimizing cardinalities `(1,2)`.

This obstruction and its repair persist uniformly.  Put `U=2^(K-1)`, `M=3^(K-1)` and
`d=2U-K-1`.  For every `K>=4` and

    2U-2K+1 <= r <= 2U-K-3,

the full-mass state `(2U,2U-1,d,r,1^T)` is majorized by `G_K`, but no feasible coloring puts the
marked `d,r` rows together.  There are `K-3` choices of `r`.  A common crossing coloring has
`2U-1,d` on one side and `2U,r` on the other.  Its nonunit left/mixed/right allocations are

    2U-1 -> (U, U-1, 0),       d -> (U-1, U-K, 0),
    2U   -> (0, U, U),         r -> (0, r-U+1, U-1),

with unit rows filling each child to mass `M`.  The mixed child fits under the canonical prefix
`(U,U-1,U-K,U-K)`.  After one transfer, replace its last two displayed contributions by
`U-K-1,r-U+2`; they still fit under the identical pair `U-K,U-K`.  Indeed the same construction
supports every intermediate transfer, and its separator margin is exactly `d-r`.

This proves one genuine Pascal separator crossing with no cyclic reassignment: an identical pair
at the first obstructing Pascal plateau absorbs the unit.  It also suggests a sharper remaining
lemma.  Starting from a minimal opposite-color tight separator, descend through the dyadic/Pascal
capacity blocks until reaching the first plateau with an unused identical target, reroute the unit
inside that plateau, and propagate the displacement back along the block chain.  A proof must show
that failure at every plateau sums to a violated parent prefix.  The explicit family proves the
local reroute, but the global termination/violation alternative is still open.

Simple global scalar optimization is not a substitute for (F).  Requiring at least
`2^(K-1)` rows of each color and then minimizing the final A/B mass difference passes every
full-mass state through `K=3`, but fails already at `K=4`.  The first failure in descending
partition enumeration is

    a=(16,15,9,9,9,5,5,5,1^8).

Its unique normalized mass-optimal split has totals `41/40` and puts `16,15` together, so even
the pure inequality `(p,q)=(2,0)` fails: `31>2H(2)=30`.  A legal split exists at totals `39/42`:

    A=(16,9,5,5,1,1,1,1),
    B=(15,9,9,5,1,1,1,1).

Thus the global invariant must retain the full signed rank profile, not just total mass and row
count.

A tempting attempt is to reserve half of every mixed-column capacity for each color in advance.
It is already too rigid at `h=G_2=(4,3,1,1)`, whose conjugate is `(4,2,2,1)`.  Splitting those
capacities by ceiling/floor gives effective color bases

`u=(8,4,1,1)` and `v=(7,4,1,1)`, with `sort(u union v)=G_3`.

Nevertheless `a=(8,5,5,5) <=_w G_3` cannot be partitioned into a subsequence majorized by `u`
and one majorized by `v`: the row `8` must go to `u`, no row `5` can join it, and the other three
rows have total 15 greater than `sum(v)=13`.  The flexible coloring
`A=(5,5)`, `B=(8,5)` does satisfy (C).  Hence the mixed capacity must be assigned adaptively; its
identical targets cannot be permanently pre-colored.

### Equivalent prescribed-chain problem

There is also a useful poset formulation.  Let `V` be the three-element poset with
`1<0`, `1<2`, and `0` incomparable with `2`, and let `P_K` be its `K`-fold lexicographic power.
Then `Q_K` is the incomparability graph of `P_K`: two words are incomparable precisely when their
first differing symbols are `0` and `2`.  Stable color classes of `Q_K` are therefore chains of
`P_K`, and `G_K` is its Greene--Kleitman chain shape.

For full mass, the converse says exactly that every partition dominated by this
Greene--Kleitman shape occurs as the list of lengths of a chain partition of `P_K`; for smaller
mass, take an induced subposet.  General Greene--Kleitman theory supplies the extremal prefix
numbers, which is another proof of necessity, but it does not supply all dominated chain-partition
types.  Thus this is a useful connection and a possible source of an augmenting-chain proof, not a
solution by itself.

### Why strict alternation is insufficient

The sequence

`(8,7,4,2,2,1,1,1,1) <=_w G_3`

colored by odd/even position has

`A=(8,4,2,1,1)` and `B=(7,2,1,1)`.  At `(p,q)=(5,1)`,

`A_5+B_1=23 > H_2(6)+H_2(5)+H_2(1)=9+9+4=22`.

So alternation satisfies the balanced parent prefixes but can fail an off-diagonal Hall inequality
already at `K=3`.  For example, `A=(8,4,2,1)`, `B=(7,2,1,1,1)` satisfies every inequality (C),
so this is a counterexample to the rule, not to the converse.  In the conjugate-layer flow model,
the corresponding working row orientations are `L,R,L,L,R,L,R,R,R`: the pure orientation must be
chosen globally and need not alternate.

### Complete `K<=4` census and a sharper forward conjecture (2026-08-26)

The full-mass reduction makes a complete small census practical.  The provenance-built utility
`tools/singleton_pair_coloring_census.cpp` enumerates every integer partition of `3^K` weakly
majorized by `G_K`, normalizes colorings by the multiplicities of equal rows, and checks (C)
directly.  There are 1,206 such states at `K=3` and 5,997,038 at `K=4`.  Every one has an
unrestricted coloring satisfying (C).  This is an exhaustive finite verification, not an
inductive proof.

The stronger adjacent-pair proposal fails on 916 of the `K=4` states.  Its first full-mass
counterexample is

`(16,15,11,11,5,5,5,1^13)`.

One legal coloring is

`A=(16,11,5,5,1^4)`, `B=(15,11,5,1^9)`.

Thus the tail unit rows must sometimes be distributed unevenly; keeping them in the argument is
essential, and splitting every adjacent pair is genuinely too restrictive.

The census suggests a more flexible **Block-Extension Conjecture**.  Write the distinct row
values as `v_1>...>v_s`, with multiplicities `m_j`, and process these value blocks in descending
order.  For each block choose, among the allocations that preserve all currently exposed
inequalities (C) and permit at least one legal allocation of the next lower block, the allocation
that minimizes the current A/B total-mass difference.  Fix complementation by putting a largest
row in A and favor A on an exact tie.  This is a forward pass with one-block lookahead and no
recoloring.  It succeeds on the complete `K<=4` census, on 10,000,000 uniformly sampled states
from the exact 38,378,683,542,323-state full-mass `K=5` universe, and on a separate 100,000-state
`K=6` dominance-transfer sample.

Those higher-level samples are not proofs.  Simpler variants already fail: plain balanced blocks
miss 22 `K=4` states and 12 of the ten million `K=5` samples; reserving enough final rows for both
colors fixes all `K=4` cases but misses six of those `K=5` samples.  The first such `K=5` failure
needs the width-6 choice to anticipate the following width-5 block.  The exact counts,
counterexamples, commands and proposed two-block proof obligation are in
[the census record](../../evidence/singleton_row_coloring_census_2026-08-26.md).

Grouping equal rows here is only the normalization of row colorings by permutations of
indistinguishable rows.  It does not establish that equality has a special theorem unavailable to
nearby widths.  A proof still has to show, using the full Pascal/dyadic structure of `G`, that a
one-block-extendable balanced choice always exists and preserves the same property at the next
step.  That statement remains open.

Even “put the next row on any side that keeps all inequalities currently valid” is not an
exchange-free algorithm.  For `K=3`, the partial coloring

`A=(7,4)`, `B=(4,4,2,2,2)`

satisfies (C), but a next row of size `2` can be appended to neither color without violating (C).
Nevertheless the completed sequence

`(7,4,4,4,2,2,2,2) <=_w G_3`

has a feasible coloring.  A forward construction therefore needs look-ahead, recoloring, or a
global existence argument; maintaining the inequalities locally is not enough.

Concretely, appending to `A` fails at `(p,q)=(3,5)` with `27>26`, while appending to `B`
fails already at `(1,6)` with `23>22`.  A feasible recoloring of the full sequence is
`A=(7,4,4,2)`, `B=(4,2,2,2)`.

### Why a two-class exchange proof is insufficient

In `Q_3`, the stable sets

`X={000,001,010}` and `Y={221}`

induce a claw on `X union Y`.  They cannot be repartitioned into two stable sets of sizes `2,2`.
Thus a unit transfer between a donor and recipient color may require a third color or a global
recoloring.  Any proof by dominance-cover exchanges must include that extra mechanism.

## Equivalent graph and symmetric-function targets

The canonical recursive strategy colors all of `Q_K` with color-class partition `G_K`, while the
necessity theorem shows that `G_K` dominates every other coloring type.  Therefore the converse is
equivalent to `Q_K` being **nice** in Stanley's sense: whenever a coloring type `lambda` occurs,
every partition dominated by `lambda` also occurs.  Equivalently, the support of the chromatic
symmetric function of `Q_K`, in sufficiently many variables, must contain every lattice point of
the permutahedron of `G_K`.  The terminology and this support interpretation are summarized in
[Matherne--Morales--Selover, Section 2.6](https://arxiv.org/html/2201.07333v4#S2.SS6).

This reformulation is exact but does not by itself prove the claim.  `Q_K` is a cograph, but from
`K>=3` it contains induced claws, so Stanley's theorem that a graph and all its induced subgraphs
are nice exactly when it is claw-free does not settle this family.  The claw example above also
explains why.

It is enough to prove the full-mass case.  If `a` has mass `M<3^K`, append `3^K-M` artificial unit
rows.  The completed partition remains majorized by `G_K`: after the original rows, its prefixes
increase by one, while every remaining nonzero entry of `G_K` is at least one.  A coloring of the
completed state restricts to one of the original state after deleting the artificial rows.

## Unconditional canonical consequence

The state `G_K` itself has Aigner's explicit recursive strategy.  Therefore any singleton state
whose rows form a sub-multiset of `G_K` is solvable by Subgraph Monotonicity.  More generally, the
same is true when the sorted row widths fit coordinatewise into distinct `G_K` rows.  These
`[canonical U_K]` and `[embedded G_K]` witness terminals remain unconditional and do not use the
open converse.  Only arbitrary `[majorized G_K]` terminals are conditional.

---

## Closed form for `G_k`

The recurrence above is equivalent to an explicit formula. Entries of `G_k` come in dyadic
blocks of sizes 1, 1, 2, 4, 8, ..., and every entry in block `r` (zero-indexed) equals a
partial sum of binomial coefficients:

```
G_k[block r] = sum_{i=0}^{k-r} C(k, i)
```

So block 0 is `2^k`, block 1 is `2^k - 1`, block 2 is `2^k - 1 - k`, block 3 is
`2^k - 1 - k(k+1)/2`, and so on. Verified against the recurrence for `k <= 12`. This is what
`make_u_freq` computes in `radio_canon_search_generic.c:121`.  It makes both the proved necessary
test and the conjectured sufficient test explicit; no recursion is needed to evaluate the prefix
inequalities.

## Consequence used throughout this project

If the parts of a singleton state form a **sub-multiset** of `G_k`, delete the unused star rows
from Aigner's explicit strategy for `G_k`.  The state is therefore solvable by Subgraph
Monotonicity, without using the open converse.  The same deletion argument works when the sorted
parts fit coordinatewise into distinct `G_k` rows.  A witness tree all of whose leaves have one of
these two forms is a complete proof independent of the solver.  `tools/check_witness.py` checks
these forms separately; it now marks an arbitrary weak-majorization terminal as conditional.

## Vertex-Splitting Pullback Lemma (2026-08-09)

Subgraph Monotonicity is not the only way to transfer a strategy between graphs. A graph may also be
made easier by **splitting a vertex into clones**.

> Let `H` and `G` be graphs and let `pi: V(H) -> V(G)` be a vertex map whose induced map on edges is
> injective: distinct edges `{x,y}` of `H` have distinct images `{pi(x),pi(y)}` in `G`. If `G` is
> solvable in `k` tests, then `H` is solvable in `k` tests.

*Proof.* At every node of a strategy for `G`, replace its tested vertex set `T` by the preimage
`pi^-1(T)`. An edge of `H` then gives exactly the same `0/1/2` response as its image edge in `G`, at
this node and recursively at every node it reaches. The strategy for `G` assigns distinct transcripts
to distinct image edges. Edge-injectivity therefore gives distinct transcripts to distinct edges of
`H`. ∎

The vertex map need not be injective: several clones in `H` may map to one vertex of `G`. This is why
the lemma is not a restatement of Subgraph Monotonicity.

## Corollary: full star-expansion majorization

Orient every part so `n_i >= m_i`. Define the **full star expansion**

```
Phi({(n_i:m_i)}_i) = ( n_1 repeated m_1 times,
                        n_2 repeated m_2 times, ... )^downarrow.
```

Equivalently, replace each `K_{n_i,m_i}` by `m_i` vertex-disjoint copies of `K_{n_i,1}`.

> If `Sb(n_1:m_1, ..., n_p:m_p)` is solvable in `k`, then
>
> `Phi({(n_i:m_i)}_i) <=_w G_k`.

*Proof.* For one `K_{n,m}`, name its sides `x_1,...,x_n` and `y_1,...,y_m`. In the expanded graph,
star `j` has centre `y_j` and leaves `x_(1,j),...,x_(n,j)`. Map `y_j` to `y_j` and every clone
`x_(i,j)` to `x_i`. This maps the edge `{x_(i,j),y_j}` bijectively to the original edge
`{x_i,y_j}`. Taking the disjoint union of these maps over all parts satisfies the pullback lemma, so
solvability of the original state implies solvability of its all-singleton expansion. The Singleton
Majorization Necessity Theorem now gives the displayed condition. ∎

This supersedes the 2026-08-06 one-copy downgrade `(n:m) -> (n:1)`. The full expansion contains that
sequence and another `m-1` copies of `n`, so every old violation remains a new violation; unlike the
old downgrade, it also preserves the full mass `sum n_i m_i`.

The choice of orientation is strongest. Expanding the other shore gives `n` copies of `m`; for
`n >= m`, that flatter equal-mass sequence is weakly majorized by `m` copies of `n`, so it cannot add
a violation after the displayed test passes.

In fact this is the strongest edge-bijective singleton lift of one part. The centre of any lifted
star maps to one original vertex, so its degree is at most `n`; the lifted stars must carry all `nm`
distinct edges. Among sequences with total `nm` and largest entry at most `n`, the sequence of `m`
copies of `n` weakly majorizes every other one. Any stronger structural condition must therefore
retain information that ordinary singleton majorization forgets, such as the requirement that clones
of one original vertex be tested together.

### The tail must be clamped, not treated as a violation

Sequences are **zero-padded**, so for `t > len(G_k) = 2^k` the right-hand side is the constant
`sum G_k = 3^k`. The left-hand side is at most the state's mass, which the counting bound has already
bounded by `3^k`. So **no violation can arise past `len(G_k)`**, and code that reports one there
over-refutes.

This matters specifically because of the corollary: full star expansion routinely has more than
`2^k` entries. `radio_verify.c` once had exactly this defect, and it fired **79 times** in a single
k=4 level even under the weaker one-copy downgrade. `radiobase.c` was already correct — it stops the
comparison at `min(size, len(G_k))`, which is equivalent when the counting bound has passed.

## The synchronized-majorization predicates (corrected 2026-08-26)

Full star expansion forgets only one thing: the cloned rows belonging to one rectangle cannot choose
their tests independently. That missing constraint can be restored one test at a time.

For a state `S` and `0 <= d <= k`, define the Boolean relaxation `R_d(S,k)` as follows.

- `R_0(S,k)` means `Phi(S) <=_w G_k`, including the zero-padded total-mass inequality.
- `R_d(S,k)` for `d>0` means that there is one legal synchronized rectangle split of `S` for which
  all three children `S_0,S_1,S_2` satisfy `R_{d-1}(S_j,k-1)`.

Thus `R_0` is the deployed static necessary condition.  Every strategy witnesses every `R_d` up
to its available depth, and `R_k` expands the exact decision-tree recurrence all the way to
trivial leaves.

> **Synchronized Predicate Theorem.** For `0 <= d <= k`,
>
> `solvable(S,k)  =>  R_d(S,k)`.
>
> At the final level, `R_k(S,k)` is equivalent to exact solvability in `k` tests.

*Proof.* The necessary implication follows by induction on `d`.  Use the real strategy's first
test; each child is solvable in `k-1`, so it satisfies the preceding predicate level.  Finally, an
`R_k` witness recursively supplies a legal split at all `k` levels. At depth `k`, each
leaf satisfies `R_0(S',0)`. Since `G_0=(1)`, such a leaf contains at most one possible defective
pair and needs no further test. The recursive witnesses are therefore an exact strategy. The reverse
direction is the necessary implication at `d=k`. ∎

The formerly claimed nesting `R_{d+1} => R_d` is **not proved**.  Its proposed base step
`R_1=>R_0` treated majorization of singleton children as sufficient for solving them, exactly the
open converse isolated above.  The false Three-Way Decomposition Lemma also prevents replacing
that step by a generic coordinate-sum argument.  Consequently the predicates are sound necessary
filters and the endpoint `R_k` is exact, but they should not presently be described as a nested
hierarchy.

Every `R_d` is also subgraph-monotone: restrict its witnessing splits to the subgraph and induct on
`d`. This justifies rejecting a partial split prefix as soon as one of its child prefixes fails the
required relaxation.

### Additive hinge form of `R_1`

For a nonnegative sequence `x`, write

    H_x(t) = sum_j max(x_j-t, 0).

Weak majorization `x <=_w y` is equivalent to `H_x(t) <= H_y(t)` for every `t>=0`. For integer
sequences it is enough to check integer thresholds. Define `C_k(t)=H_{G_k}(t)` and

    h_t(u,v) = min(u,v) max(max(u,v)-t, 0).

The quantity `h_t(u,v)` is exactly the hinge contribution of the full star expansion of one
rectangle `(u:v)`. If `(n:m)` is split by selecting `(a:b)`, its contributions to the three children
are

    outcome 2: h_t(a,b)
    outcome 0: h_t(n-a,m-b)
    outcome 1: h_t(a,m-b) + h_t(n-a,b).

Consequently, `R_1(S,k)` is exactly a multiple-choice integer feasibility problem: choose one
`(a_i:b_i)` for every parent part so that, for every outcome and threshold, the sum of these
contributions is at most `C_{k-1}(t)`. This formulation is additive and requires no sorting. Threshold
`t=0` is the ordinary three-child counting bound; the positive thresholds retain the shape
information that counting loses.

### Worked residual: `Sb(16:1,12:2)` in four tests

The static profile is `(16,12,12)`. It passes `R_0` because its nontrivial prefix sums
`16,28,40` are bounded by the first three prefix sums `16,31,42` of `G_4`.

For `R_1`, choose the synchronized split `[8:0,7:2]`. Its children are

    outcome 2: Sb(7:2)
    outcome 0: Sb(8:1)
    outcome 1: Sb(8:1,5:2).

Their full-star profiles `(7,7)`, `(8)`, and `(8,5,5)` all lie below `G_3`, so this witnesses
`R_1`. Each child in turn has an `R_1` continuation, so the same first split also witnesses `R_2`.
But `R_3` fails: three synchronized layers are already enough to expose the obstruction, one level
before the exact `R_4` test. Reproduce all five verdicts with

```
tools/bundled_majorization.py ladder 4 16 1 12 2
```

### Why there is no single width-two base sequence

Even if every part has width two, solvability is not downward closed under ordinary weak
majorization. The independent reference solver gives

```
tools/refsolve.py solve 4 12 2 10 2 9 2 3 2   # solvable
tools/refsolve.py solve 4 11 2 11 2 9 2 3 2   # unsolvable
```

Yet the unsolvable state's full-star profile

    (11,11,11,11,9,9,3,3)

is weakly majorized by the solvable state's profile

    (12,12,10,10,9,9,3,3):

their prefix sums are respectively `(11,22,33,44,53,62,65,68)` and
`(12,24,34,44,53,62,65,68)`. Therefore the exact width-two solvable set is not a majorization ideal.
No criterion obtained by replacing `G_k` with one fixed width-two base sequence can be exact; the
discrete synchronization choices, retained by the hierarchy, are essential.
