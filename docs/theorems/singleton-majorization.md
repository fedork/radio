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

An exact breakpoint checker verifies (P4) for every integer `E` through `K=12`.  Complete state
censuses find no padded-alternation failure with a nonzero canonical tail at `K=3,4`; all failures
have far fewer than `2M` nonzero rows.  This is a proved reduction plus finite evidence, not a
uniform proof of (P4).  A proof of (P4) from the dyadic/Pascal formula for `H` would settle the
entire high-support regime by an explicit single pass.  The derivation, exchange proof, census
counts and commands are in the
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

A still more concrete sufficient statement is the following global transfer lemma.  If
`x in M_h` and `x_i>=x_j+2`, then

    x-e_i+e_j in M_h,                                                        (T)

where the signed lift and its row coloring may change everywhere.  Starting from a permutation
of `G_K`, repeated unit Robin-Hood transfers generate every full-mass integer vector majorized
by `G_K`; (T) would therefore prove (F), and artificial unit rows handle smaller mass.

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
