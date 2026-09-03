# Singleton majorization: exact through `K=5`, false for every `K>=6`

## Status (resolved negatively 2026-08-30)

For a singleton state

`Sb(a1:1, a2:1, ..., an:1)`, with `a1 >= a2 >= ...`,

the implication

`solvable in K tests  =>  (a1,a2,...) <=_w G_K`

is proved below.  The converse is **false**.  The full-mass exact-support state

    (64,63,57^2,42^4,22^7,8^15,7^2,1^32) <=_w G_6

has no legal first split into three `G_5`-majorized children and is therefore not solvable in six
tests.  The proof uses two tight Pascal ranks and one integral unit of mixed mass per intervening
row; see [the counterexample proof](../../evidence/singleton_k6_counterexample_2026-08-30.md).
Aigner proved the necessary direction and explicitly left the converse open in 1988; this
counterexample answers that question negatively for the present formulation. A corpus-scoped
prior-art audit found no published resolution in the navigable citation graph or the primary
sources it exposed; the unavailable-source limitations remain explicit, so no worldwide-first
claim is made. See the [audit record](../../evidence/publication_prior_art_2026-09-02.md) and
[scan note](../aigner-1988-scan.md).

The failure is not isolated.  The proved
[Dyadic Balanced-Band Family](tight-band-capacity.md#a-counterexample-at-every-k6) constructs a
full-mass exact-support `G_K`-majorized parent with no legal first cut for **every `K>=6`**.
For `K=2m` or `2m+1`, it balances the canonical band between ranks `2^(m+1)` and
`2^(m+2)`; the two tight endpoints force symmetric pure-capacity violations.  The first new member
is

    (128,127,120^2,99^4,64^7,32,31^16,8^32,1^64) <=_w G_7.

An independent direct-row solver exhausts this `K=7` state as well.  The construction, infinite
proof, 20-instance machine survey through `K=15`, and provenance are in the
[dyadic-family record](../../evidence/singleton_dyadic_counterexample_family_2026-08-31.md).
A complete prefix-cylinder census proves the converse for all 1,431,800,647,444 exact-support
parents at `K=5`; Minimum-Support Reduction and the complete `K=4` result extend this to every
`G_5`-majorized singleton state.  Hence `K=6` is the proved smallest failure level; see the
[complete `K=5` record](../../evidence/singleton_k5_prefix_cylinder_2026-08-31.md).

There is an even smaller counterexample:

    (64,63,57^2,42^4,22^7,8^15).

It has mass 683 and support 30.  Tight ranks 15 and 30 give six endpoint transitions, all blocked
by the Tight-Band Capacity Obstruction.  Deleting one of its fifteen 8s makes the prefix first-cut
feasible.  The mass-697 and full-mass forms remain useful because extending to rank 32 reduces the
human proof to two symmetric cases.

An independent clean-room direct-row solver now exhausts both forms with child slack handled
generically, reproduces the last feasible transfer cut, and agrees with an unquotiented oracle on
its tiny controls.  This is an implementation check, not a replacement for the analytic proof; see
the [verification record](../../evidence/singleton_direct_split_cleanroom_2026-08-31.md).

The rank-15/32 argument is now an instance of the proved
[Tight-Band Capacity Obstruction](tight-band-capacity.md).  Its inequality-only extractor and an
independent direct-row enumeration also show that `(8^15,7^2)` is the unique first-cut hole among
all 176 dominated 17-row bands on this fixed face.  A complete inequality census finds no
two-anchor capacity certificate on any of the 613,689,090 eligible `K=5` band instances, while an
exact prefix-cap optimization proves that distance 14 is globally minimal among all `K=6` parents
certified by this obstruction.  A subsequent exact Fixed-Color Hall census exhausts all
5,189,450,419 exact-support `K=6` parents through distance 13 and finds a first cut for every one.
Thus 14 is globally minimal for an exact-support **no-first-cut** hole, not merely for this
certificate class.  The complete `K=5` theorem upgrades this to minimum distance among recursively
unsolvable exact-support `K=6` parents as well; see the
[transfer-shell record](../../evidence/singleton_transfer_shell_census_2026-08-31.md) and
[complete `K=5` record](../../evidence/singleton_k5_prefix_cylinder_2026-08-31.md).

The former proof in this file used a purported Three-Way Majorization Decomposition Lemma.  That
lemma is false, and even a correct unconstrained polymatroid decomposition would not enforce the
legal row condition that a singleton row cannot feed both pure children.  The exact Row-Coloring
reformulation below is also false, now for the genuine Pascal profile at `K=6` rather than only for
generic bases.

The full-mass exact-support form has the minimum `2^6` positive rows required at mass `3^6`, so the
proved Minimum-Support Reduction is sharp but cannot prove sufficiency.  The proved Two-Anchor Reduction maps it to the
explicit residual hole

    (62,61,55^2,40^4,20^7,6^15,5^2)

below `(62,61,55^2,40^4,20^8,5^16)`.  Hence the formerly proposed Balanced Residual Coloring
Lemma is false as well.

The prior finite searches remain correct within their stated ranges: the converse holds through
`K=4`; the new prefix-cylinder census closes `K=5` as well.  They did not cover the `K=6` hole.
The state is reached from `G_6` by fourteen unit
Robin--Hood transfers; the first thirteen intermediate bands retain a majorized first cut, and the
last transfer destroys the entire cut fiber.  Exact exhaustion now strengthens this path statement:
no exact-support parent anywhere in the complete distance-13 ball lacks a majorized first cut.
This supplies the phase change that the transfer surveys were seeking and shows why a global
downward-closure proof cannot exist.  At `K=5`, all 1,431,800,647,444 exact-support parents are
recursively solvable: prefix cylinders cover 1,431,650,734,151, and exact Hall search closes the
remaining 149,913,293.

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

### A stronger chromatic-symmetric-function target (refuted at `K=6`)

Write the chromatic symmetric function as

    X_(Q_K) = sum_lambda c_K(lambda) m_lambda.

Stanley's monomial expansion says that `c_K(lambda)` counts semi-ordered stable partitions of
`Q_K` of type `lambda`.  Thus the desired converse follows from the formally stronger inequalities

    lambda >= mu  =>  c_K(lambda) <= c_K(mu).                    (SN)

In current terminology, (SN) says that `Q_K` is **strongly nice**; strongly nice immediately
implies nice, meaning that the stable-partition support is a dominance ideal.  This packages the
existence problem globally: it asks for monotonicity of the number of decompositions, rather than
selecting one decomposition by a local rule.

The `K=6` state in the status section is a partition dominated by the canonical coloring type
`G_6` that does not occur.  Therefore `Q_6` is not nice and (SN) is false at `K=6`.  The material
below is retained because the coefficient recurrence and the lower-level finite theorems remain
correct, not as a live route to a universal proof.

The graph recursion gives an exact coefficient recurrence.  For a fixed labelled color of size
`w`, its counts in the left/mixed/right copies must be

    (w-x,x,0) or (0,w-x,x).

After assigning every color, the three child coefficients multiply, and summing all assignments
gives `c_K`.  The provenance-built `tools/singleton_strong_niceness.cpp` evaluates this recurrence
without assuming the converse.  It verifies (SN) for `K=3` on all 1,206 possible supported
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
the recipient may legally use, child-level strong niceness performs the transfer.  The
then-unresolved case put donor and recipient on opposite pure sides, with no donor excess in the mixed child.  A
two-color Kempe exchange need not work—the induced-claw example below already proves that—and an
injective construction would have had to route through a third color or a global augmenting chain.
The final transfer into the `K=6` counterexample proves that no such universal injection exists.

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
`|S_K|` would indeed prove the Row-Coloring conjecture, because the image is already contained in
`S_K`; however, determining that distinct-output count is exactly the assertion that every relevant
coefficient is nonzero.  A uniform division by a fiber size is impossible even at `K=1`.

There is no closure theorem for the displayed graph recursion.  Strong niceness is
closed under disjoint union, but not under graph join in general; moreover `Q_3` contains an induced
claw, so the hereditary claw-free characterization does not apply.  The formerly proposed target
was the special operator

    T(G) = G disjoint-union (G join G):

and preservation of strong niceness, or even dominance-ideal support, along the particular sequence
`Q_K=T(Q_(K-1))`.  The `Q_6` counterexample refutes both preservation claims.

Even support-level closure under this special operator is false for a generic seed.  Let `E_3` be
the empty three-vertex graph.  The graph `T(E_3)` is nice with maximum stable-partition profile
`(6,3)`, but `T(T(E_3))` has maximum profile `(12,9,3,3)` and has no stable partition of the
dominated type `(12,9,2,2,2)`.  Indeed, a block of size 12 forces `6+6` across the mixed copy and
one pure copy; the block of size 9 then forces the remaining `3+6` across the mixed and other pure
copy.  Three blocks of size two cannot fill the two remaining pure capacities of three without
using the already-full mixed copy or crossing both joined pure copies.  Thus an induction for the
particular seed `Q_0=K_1` must retain its complete Pascal profile; the formal operator alone does
not preserve niceness.  The construction and the short verification that `T(E_3)` is nice are in
the [Pascal-switch record](../../evidence/singleton_balanced_hh_switch_2026-08-29.md).

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

## The Fixed-Color Hall criterion and the refuted Row-Coloring conjecture

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

## Exact Prefix-Cylinder Extension Lemma

The Fixed-Color Hall Lemma also gives a useful exact **positive** certificate.  It is the analytic
content of the prefix-cylinder computation that closes `K=5`.

Let `g=(g_1,...,g_N)` be a parent majorization bound, with prefix function `P`.  Fix a
nonincreasing parent prefix

    x=(x_1,...,x_t),                 S=sum_i x_i,

and integers `z=N-t`, `R` and `M`, where `R` is the remaining mass and `M<=x_t` bounds the next
row (with the evident omission when `t=0`).
Let `C(x;R,M)` be the set of all positive nonincreasing suffixes

    b=(b_1,...,b_z),   M>=b_1>=...>=b_z>=1,

such that `sum b_i=R` and

    S + sum_{i=1..j} b_i <= P(t+j)       for 1<=j<=z.                 (PC1)

Assume this set is nonempty.  Color the rows of `x` by `A/B`, and fix a word
`sigma in {A,B}^z` which colors the suffix by rank.  Write `u,v` for the numbers of fixed-prefix
rows colored `A,B`, write `u_sigma,v_sigma` for the two letter counts in `sigma`, and let
`alpha_p,beta_q` be the sums of the largest `min(p,u)` and `min(q,v)` fixed rows of the indicated
color.  Here "valid" means `0<=p<=u+u_sigma` and `0<=q<=v+v_sigma`.  For valid `p,q`, put

    r=(p-u)_+,   s=(q-v)_+.

For `b in C(x;R,M)`, let `T_sigma(r,s;b)` be the sum of the first `r` suffix rows marked `A` and
the first `s` suffix rows marked `B`, and define the exact support value

    U_sigma(r,s) = max_{b in C(x;R,M)} T_sigma(r,s;b).                (PC2)

> **Exact Prefix-Cylinder Extension Lemma.** Every completion `(x,b)`, with
> `b in C(x;R,M)`, satisfies the Fixed-Color Hall inequalities under this one fixed prefix
> coloring and suffix word if and only if, for every valid `p,q`,
>
> `alpha_p + beta_q + U_sigma((p-u)_+,(q-v)_+)`
>
> `<= H(p+q)+H(p)+H(q).`                                             (PC3)
>
> Consequently (PC3) is sufficient for every completion in the cylinder to have a legal first
> split into three children weakly majorized by `h`.  If every `h`-majorized singleton state is
> solvable in the remaining tests, then every completion in the cylinder is recursively solvable.

*Proof.* Since every fixed-prefix row is at least every suffix row, the `p` largest `A` rows of a
completion consist, up to immaterial ties, of the largest `min(p,u)` fixed `A` rows followed by the
first `(p-u)_+` suffix rows marked `A`; similarly on side `B`.  Thus the left side of the
Fixed-Color Hall inequality is exactly

    alpha_p + beta_q + T_sigma((p-u)_+,(q-v)_+;b).

Taking its maximum over all suffixes in the cylinder gives (PC3).  Hence (PC3) is equivalent to
the same fixed coloring satisfying every Hall inequality for every completion.  The final claim is
the Fixed-Color Hall Lemma. ∎

The word "exact" here has a precise scope: (PC3) is necessary and sufficient for this **one
uniform coloring scheme** to cover the whole cylinder.  It is not necessary for a cylinder whose
different completions are allowed unrelated colorings.

### Exact finite recurrence

The support values in (PC2) require no relaxation or enumeration of complete parents.  Let
`F_j(R',M',r,s)` denote the maximum selected mass after the first `j` suffix positions have been
fixed, with remaining mass `R'`, next-row bound `M'`, and `r,s` requested rows still to select.
At position `j`, range over the integers `w` satisfying

    1 <= w <= M',
    z-j-1 <= R'-w <= (z-j-1)w,
    S+(R-R')+w <= P(t+j+1).                                         (PC4)

If `sigma_(j+1)=A` and `r>0`, add `w` and replace `r` by `r-1`; do the analogous operation for
`B,s`.  Then

    F_j(R',M',r,s)
      = max_w { selected(w) + F_(j+1)(R'-w,w,r',s') },               (PC5)

with terminal value zero exactly when `j=z`, `R'=0` and `r=s=0`, and minus infinity otherwise.
Induction on the remaining positions proves

    U_sigma(r,s)=F_0(R,M,r,s).                                      (PC6)

Future prefix constraints are enforced by the later instances of (PC4).  Thus (PC5) is an exact
finite integer recurrence for the lemma's coefficients.  The `K=5` census specializes `sigma` to
the two alternating words and evaluates (PC5) by memoization.

### A closed-form sufficient corollary

For `0<=j<=z`, put

    Q(j)=min(P(t+j)-S, R-(z-j), jM).                                 (PC7)

Any `j` suffix rows have sum at most the sum of the largest `j`; the three terms in (PC7) bound
that prefix respectively by parent majorization, positivity of the remaining `z-j` rows, and the
row maximum.  Therefore

    U_sigma(r,s) <= Q(r+s).                                         (PC8)

It follows that the entirely explicit inequalities

    alpha_p + beta_q + Q((p-u)_+ +(q-v)_+)
      <= H(p+q)+H(p)+H(q)                                           (PC9)

are sufficient to cover the cylinder.  This **Three-Bound Prefix Corollary** is weaker than the
sharp lemma because it forgets the positions and colors of the selected tail rows.  It is often a
quick human certificate; the exact support values (PC2)--(PC6) were needed for the efficient
complete `K=5` proof.

The full converse was therefore equivalent to the following purely two-color statement.

> **Row-Coloring Conjecture (false at `K=6`).** If `a <=_w G_K`, the rows of `a` can be colored `A/B` so that
> (C) holds for every `p,q`.

Indeed, the Fixed-Color Hall Lemma produces three `G_{K-1}`-majorized children, and induction would
finish the strategy.  Conversely, the first test of any strategy supplies exactly such a coloring.

The parent majorization inequalities are only the balanced slice of (C):

`H_K(t) = H(t)+H(ceil(t/2))+H(floor(t/2)).`

The `K=6` counterexample at the top of this note satisfies every balanced parent inequality but no
coloring satisfies all off-diagonal pairs.  Equivalently, no legal first allocation exists.  Here
is the short obstruction.  For `h=G_5`, let `H` be its prefix function.  The parent is tight at
ranks 15 and 32, forcing the color counts `(p,q)` at those ranks to lie in

    {7,8} x {16}.

The 17 intervening rows have values `(8^15,7^2)`.  Tightness forces their mixed pieces to total
`H(32)-H(15)=22`; if one were zero, 31 mixed pieces would have mass
`H(15)+22=243>H(31)=242`.  Thus every band row sends at least one integral coin to the mixed child.
In either possible count transition, one pure child consequently needs 64 coins from nine rows,
which can retain at most 63.
This contradiction proves:

> **Singleton Majorization Counterexample Theorem.** Singleton majorization is necessary but not
> sufficient.  The state `(64,63,57^2,42^4,22^7,8^15)` is majorized by `G_6` and is unsolvable in
> six tests.  Adding `(7^2,1^32)` gives the full-mass exact-support version displayed above.

The full calculation, original direct/Hall enumerations, clean-room direct-row check and
transfer-path boundary are in the
[counterexample record](../../evidence/singleton_k6_counterexample_2026-08-30.md).
The general two-anchor theorem and complete fixed-face survey are in the
[Tight-Band Capacity note](tight-band-capacity.md).

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
assuming the converse.  The `K=6` counterexample shows that the demanded **row coherence** need not
exist: one sign must serve all doubled-column incidences of a row, and the two tight ranks make
those requirements incompatible.

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

The Boolean labels did **not** finish the hoped-for proof.  Static Hall capacity has already
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
opposite-color swap.  This motivated the stronger **Two-Row Color-Exchange Conjecture**: every
failed feasible coloring has another feasible coloring, successful for the transfer, obtained by
one flip or one swap (up to equal-row identity and global color complementation).  It would have
finished the Adjacent-Fiber step immediately, but the final `K=6` transfer has no feasible target
coloring and refutes it.  The lower-level finite results remain exact.  Exact definitions,
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

This analogy did not provide a proof.  The feasible colorings of a fixed state are the boundary slice

    P(rho) intersection {z: |z_i|=a_i for every i}.

Flipping row `i` changes `z_i` by `2a_i`, whereas bisubmodular delta exchange only guarantees unit
moves of support one or two through intermediate points with smaller absolute coordinates.  An
intersection with the fixed-absolute-value slice need not inherit the exchange axiom.  The proposed
**Pascal fixed-absolute exchange lemma** asserted that, for the Pascal `H`, the unit exchange chain
could be compressed to a boundary move consisting of one row flip or one row swap that makes the
transfer margin positive.  The final transfer into the `K=6` hole refutes that universal assertion.

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
Conversely, every such crossing move gains the amount in (CG) on this particular cut.  It may,
however, expose an oppositely imbalanced blocker elsewhere.

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

The formerly proposed simpler monotone target oriented the failed coloring so that the donor is in `A` and
the recipient in `B`, and select a tight transfer separator `S=X union Y`.  Call a crossing move
**positive** when it is feasible for the original demand and either

1. flips a positive-width row `v in Y` from `B` to `A`; or
2. swaps `v in Y` with `u in A-X` and has `x_v>x_u`.

Equivalently, the move strictly increases the total mass colored `A`.  The proposed local claim was:

> **Positive Pascal Crossing Conjecture (false universally).** Every failed material-row coloring has a tight
> transfer separator across which some positive crossing flip or swap is feasible.

This statement would prove the Adjacent-Fiber property; the crossing move need not solve the transfer
immediately.  If it moves the recipient or the donor (but not both, since the donor is at least two
larger), the two marked rows acquire the same color and the same-color argument finishes.  Otherwise
their orientation stays `A/B`.  If the new coloring still fails, repeat from one of its tight
separators.  Each nonterminal step strictly increases the integer `A`-mass, while the pure Hall
inequality bounds that mass by `2*3^(K-1)`, so cycling is impossible and the process terminates at a
common coloring.  The last transfer into the `K=6` counterexample has no feasible target coloring
at all, so the claimed universal termination and hence the conjecture are false.  A zero recipient is easier: its padded zero row may be assigned the donor's color
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

> **Core--Blocker Escape Conjecture (false universally).** For some `v in C intersection B`, either
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
flip case is the same argument without `u`.  Thus the Core--Blocker Escape Conjecture would imply
Adjacent-Fiber in one recoloring.

This is a sharp **local strengthening**: compute the dangerous tight-set lattice, scan its
`B`-core, and either take a feasible flip or intersect that flip's blockers and take a smaller
outside-`A` row.  It uses no alternation, endpoint special case, exchange cycle, or arbitrary
separator.  Its universal existence is false: the final transfer into the `K=6` hole has no target
coloring.  The local implication and finite lower-level evidence remain valid.
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
Hence, after sorting within colors, the Core--Blocker Escape Conjecture has a one-dimensional staircase:
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
strong local conjecture had an entirely sorted-prefix formulation.  The `K=6` hole proves that its
failed staircase inequalities need not sum to a parent-prefix violation.

Uncrossing has the following quantitative form, retained as a description of why that route stops.  Let
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
signed nesting `X subset P`, `Q subset Y`.  The hoped-for next step was to use the duplicated
columns guaranteed by (TB3), across all `v in C intersection B`, to force one blocker's common
intersection past `U` or a violated parent prefix.  The counterexample proves that this conclusion
does not follow universally.

The finite evidence remains substantial.  Through `K=3` and in the first 5,000 `K=4` states,
every maximum-`A`-mass crossing neighbor is successful for the transfer, every tie at the maximum
is successful, and the smallest observed maximum gain is one.  Three further disjoint windows also
have a successful maximum-gain crossing neighbor.  These observations do not prove the
Core--Blocker Escape Conjecture.

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

Consequently, the Row-Coloring conjecture would have followed at once if `F_h` were an **integral
polymatroid**.  Its rank inequalities would then be exactly

`sum_(i in S) a_i <= H_K(|S|),`

which, by symmetry, are the parent majorization inequalities.  Equivalently, it is enough to prove
the discrete augmentation/exchange axiom for legal allocations.  This is the cleanest bottom-up
version of the coin-transfer proposal: an added or transferred unit is routed along an augmenting
path, possibly recoloring several rows, until it reaches one of the identical dyadic columns.
At full mass this is the same M-convex exchange property used for supports of generalized
permutahedra; see
[Matherne--Morales--Selover, Section 1.3.1](https://arxiv.org/html/2201.07333v4#S1.SS3.SSS1).

The `K=6` lattice hole proves that `F_(G_5)` is not an integral polymatroid and that the global
augmentation claim is false.  The examples below record earlier failures of still stronger greedy
rules.  Thus the Pascal multiplicities and power-of-two capacities do not repair the generic
failure.

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

The formerly proposed algorithmic target was an **adaptive balanced-support augmentation**.  Insert
the paired pure columns in decreasing capacity, rerouting earlier incidences along alternating
paths when a side lacks `c_j` usable rows; a row may change orientation only after its pure
incidences have been rerouted.  Once all pairs are placed and the residual remains majorized by
`h`, Gale--Ryser finishes the mixed child.  Choosing, say, the shortest lexicographically first
augmenting path would have made this a universal deterministic algorithm.  A failed augmentation
exposes an opposite-orientation tight cut.  The `K=6` hole proves that the Pascal multiplicities do
not always let that cut be crossed.

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

The first-coordinate shadow was a clean **Scalar Shape-Balance Conjecture**: require only

    J_a(1) in floor/ceiling(min(2^(K-1) theta(a), #{i:a_i>=2})).

This is stronger than Row-Coloring and therefore is also false universally.  An exact reconstruction
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

### Global signed lifting and the lattice hole at `K=6`

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
characterizations.  Bisubmodularity itself does not imply orthant saturation: the generic bases
above, and now the Pascal `K=6` base, have uncolorable parent demands.

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

Consequently the Row-Coloring conjecture was exactly the following **Pascal Orthant-Saturation
Conjecture**:

    M_h = conv(M_h) intersection Z_+^E,              for h=G_(K-1).              (F)

The `K=6` state `(64,63,57^2,42^4,22^7,8^15,7^2,1^32)` lies in the right side of (F) but not
in `M_(G_5)`.  It is therefore an explicit primitive integer lattice hole.  The complete `K<=4`
census remains a proof that the lower-level Pascal folds have no holes.

### An equivalent balanced-column realization

There is a second exact formulation in which the Pascal structure is present before any coloring
is chosen.  Let `c=h'` be the conjugate child partition.  Pascal identity (D) says that the
conjugate parent capacities consist, for every `c_t`, of one **doubled column** of degree `2c_t`
and one **single column** of degree `c_t`.  For a full-mass demand `x`, Gale--Ryser gives

    x <=_w G_K

if and only if there is a `0`--`1` row/column incidence matrix with row degrees `x` and exactly
those doubled and single column degrees.

The Row-Coloring conjecture was equivalent to asking for such a matrix together with a coloring of its
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

> **Balanced Pascal Realization Conjecture (false at `K=6`, equivalent form).** Every full-mass row-degree
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

### A complete switch graph and a refuted short descent

The equivalent realization problem has a canonical finite search space.  Fix the exact-support
row margins, the labelled Pascal column margins, and an equal row bisection.  A **row switch**
exchanges the colors of one row on each side.  An **incidence switch** replaces a binary submatrix
`[[1,0],[0,1]]` by `[[0,1],[1,0]]`, or conversely.  Both preserve all required margins.

> **Switch-Graph Completeness Lemma.**  All pairs `(incidence realization, equal row bisection)`
> with the fixed margins lie in one graph under row and incidence switches.

*Proof.*  Equal bisections are connected by opposite-color row switches.  Binary matrices with
fixed row and column sums are connected by `2x2` interchanges: decompose the symmetric difference
of two matrices into alternating bipartite cycles and eliminate the cycles by the standard
interchange induction.  Applying the two transformations successively proves connectivity. ∎

Consequently exhaustive switch-graph search from any Gale--Ryser realization is an exact decision
algorithm: it reaches a quota-balanced vertex if and only if the desired first split exists.  This
does not prove that such a vertex exists for every majorized parent, but it identifies precisely
what a constructive proof may switch without committing to one coloring or one realization.

A much shorter deterministic search now has substantial finite evidence.  Start with the
canonical bipartite Havel--Hakimi realization (largest residual rows first, original row order for
ties) and the alternating row bisection.  At rank `ell`, sort the squared column imbalances and sum
the `binomial(K-1,ell)` smallest; let `Phi` be the sum over internal ranks.  Then `Phi=0` is exactly
the binomial balance quota.  Choose a strict row or incidence switch giving the smallest `Phi`,
with the source-order scan breaking ties.  If none exists, take the first `Phi`-neutral row or
incidence switch in the fixed scan order--including a same-color incidence switch--that exposes a
strict switch, and then take that strict switch.

> **Canonical Two-Move Pascal-Switch Conjecture (false at `K=6`).**  This descent never gets
> stuck at positive `Phi`.

If it were true, every one or two switches would lower a nonnegative integer, so the algorithm
would terminate at a legal split.  The `K=6` hole has no zero-`Phi` vertex and therefore refutes
the conjecture.  The rule passes all 408,776 exact-support `K=4` parents,
the first 500,000 `K=5` parents, disjoint higher `K=5` windows and the stated `K=5,6` random,
walk and adversarial probes.  These higher-level runs are finite evidence only.

Two boundary examples explain why both freedoms are real.  At `K=5`, the canonical matrix for

    (32,31,26,26,16^3,4^15,2^10)

has no legal row bisection (there is a short rank-count proof), but one incidence switch between a
rank-1 and rank-2 column makes it balanced.  Strict descent alone first fails at

    (32,31,26,26,16^3,9,6^9,2^2,1^13):

a neutral row switch followed by a strict row switch reaches zero.  Thus the new conjecture is not
the already-refuted canonical-coloring rule, and it does not assume that every step improves.
Exact supports, traces, complete counts, reproduction commands and the counterexample to the
tempting one-discrepancy invariant are in the
[Pascal-switch record](../../evidence/singleton_balanced_hh_switch_2026-08-29.md).

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
fixed `t`.  Hence the Balanced Pascal Realization conjecture is exactly:

> **Boolean Coordinate-Exposure Conjecture (false at `K=6`, equivalent form).** Realize the row degrees `x` on
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

The relabelling freedom cannot be replaced by one fixed Boolean address on the rows.  A tempting
simultaneous-halving strengthening labels the `2^K` rows by bit strings and requires the column
`S` to be a subcube `{x:x|S=p_S}`.  It already fails for the canonical target
`G_2=(4,3,1,1)`.  With four rows there are one universal column, two coordinate half-columns and
one singleton column.  A degree-four row must lie in both halves and receive the singleton; its
two square-neighbors then each lie in one half and have degree at least two, forcing
`(4,2,2,1)` instead.  Thus recursive coordinate exposure must allow branch-dependent relabelling;
it cannot be frozen into one global row cube.

Consequently the Boolean labels do not themselves impose any lower- or upper-shadow relation
between adjacent ranks.  Such a relation can be destroyed by independently permuting the labels
inside one rank without changing the incidence matrix, coloring, or legality.  A
Kruskal--Katona/LYM argument would become legitimate only after an augmenting construction produced
a distinguished *coupled* labelling and proved that its reached sets were shadow-closed.  No such
construction can exist for every dominated parent, so assuming shadow closure would merely add a
false stronger hypothesis.

What survives relabelling, and is therefore intrinsic, is the complete Pascal package: rank
`ell` has degree `2^(K-ell)`, deletion quota `binomial(K-1,ell)`, contraction allowance
`binomial(K-1,ell-1)`, and the deletion columns at rank `ell` can be paired arbitrarily with the
contraction columns at rank `ell+1`.  The `K=6` hole shows that combining these data with
the switch-minimal defect relations (BR0)--(BR4) need not produce a violated parent prefix.
A scalar sum over ranks, or Boolean shadows supplied only by relabelling, is insufficient.

This formulation does not assume that a previously selected coloring can be repaired locally.
It is therefore an exact restatement of the refuted Row-Coloring conjecture, unlike the stronger
local targets below.  The counterexample has no switch sequence ending at a balanced realization.

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
single column at that capacity, and rows obey (BR3).  The `K=6` hole shows that these
per-capacity defects need not yield another switch or a violated parent prefix.  Conditions
(BR1)--(BR4) remain necessary structure of a switch-minimal hole; the counterexample supplies the
concrete object in which their formerly hoped-for conclusion fails.

An equivalent transfer form of the now-refuted universal full-mass claim was the following.  If
`x in M_h` and `x_i>=x_j+2`, then

    x-e_i+e_j in M_h,                                                        (T)

where the signed lift and its row coloring may change everywhere.  Starting from a permutation
of `G_K`, repeated unit Robin-Hood transfers generate every full-mass integer vector majorized
by `G_K`; (T) would therefore prove (F), and artificial unit rows handle smaller mass.

As universal full-mass statements, (T) and (F) are equivalent: (F) immediately puts the more
balanced neighbor back in `M_h`, while (T) reaches every dominated vector from `G_K`.  The next
statement is different.  Requiring the two endpoints of every transfer to share one feasible
coloring is a genuine strengthening of (T).  Both statements are false: along (K6-4) in the
counterexample record, the final Robin--Hood transfer leaves `M_(G_5)` altogether.

This was the precise global form of the bottom-up coin-transfer proposal.  In the column model,
remove one incidence from donor row `i` and seek an alternating row/column path that installs it
at recipient `j`, possibly flipping several whole-row orientations.  If the path search stops,
its reached rows and doubled/single columns give a Hall cut.  The hoped-for Pascal-specific step
was to symmetrize that cut into a violated parent rank inequality.  The `K=6` cut is the explicit
counterexample: it respects every parent rank inequality, so no such universal symmetrization is
possible.

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

Thus only an opposite-color tight separator can obstruct a transfer.  This isolated the following
particularly economical sufficient statement, now refuted.

> **Pascal Adjacent-Fiber Conjecture (false at `K=6`).** If `x in M_h`, `h=G_(K-1)`, and
> `y=x-e_i+e_j` is a Robin-Hood transfer, then some row coloring is feasible for both `x` and `y`.

The coloring may depend on the chosen transfer; asking one coloring to support every outgoing
transfer is stronger and is not needed.  The Adjacent-Fiber statement would prove (T), hence (F), by
walking from a permutation of `G_K` through unit Robin-Hood transfers.  In tight-set language, its
failed content was the claim that among the feasible colorings of `x`, one can eliminate every tight set
that separates this particular opposite-color pair.  A proposed proof would choose a coloring
maximizing the minimum separator slack and use bisubmodular uncrossing to recolor across a minimal
tight separator.  The final edge of (K6-4) proves that such a recoloring step cannot always exist.

An exact census now verifies this common-coloring statement for every full-mass state and every
normalized donor-value/recipient-value transfer through `K=4`: all 141,690,676 `K=4` transfer
types pass.  For `K<=3`, exhaustive maximization gives best separator margin at least two.  At
`K=4` every transfer also has a certificate of margin at least two.  Transfers without a
same-color certificate have their opposite-color fiber exhaustively maximized; ordinary
same-color successes stop at their first certificate, so maxima above two are not claimed.  This
is an exact lower-level theorem, but the universal statement fails two levels later.

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
at the first obstructing Pascal plateau absorbs the unit.  It originally suggested a sharper
lemma: starting from a minimal opposite-color tight separator, descend through the dyadic/Pascal
capacity blocks until reaching the first plateau with an unused identical target, reroute the unit
inside that plateau, and propagate the displacement back along the block chain.  The explicit
family proves the local reroute, but the `K=6` hole shows that global termination or a violated
parent prefix cannot be guaranteed.

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

### The Pascal rank poset and the Griggs-dominance connection (2026-08-29)

The poset formulation identifies the problem more sharply than the Greene--Kleitman prefix
calculation alone.  Let `V` be the three-element poset with `1<0`, `1<2`, and `0` incomparable
with `2`, and let `P_K` be its `K`-fold lexicographic power.  Then `Q_K` is the incomparability
graph of `P_K`: two words are incomparable precisely when their first differing symbols are `0`
and `2`.  Stable color classes of `Q_K` are therefore chains of `P_K`.

Replace a symbol `1` by the bit zero and either outer symbol `0,2` by the bit one.  If the resulting
binary word represents the integer `r`, then the rank of the transcript is exactly

    rho(w)=r,                         0<=r<2^K.                 (PR1)

Indeed,

    P_K = P_(K-1) ordinal-sum (P_(K-1) parallel-union P_(K-1)):  (PR2)

the first-symbol-`1` copy occupies the lower half of the ranks, and the two outer copies occupy the
upper half.  Every `1` bit of `r` may independently be labelled by the outer symbol `0` or `2`,
whereas every zero bit is forced to be the symbol `1`.  Hence the rank number is

    R_K(r)=2^popcount(r).                                      (PR3)

This is the Pascal structure in rank order, before sorting any row or column profile.

The adjacent-rank graph is equally explicit.  Write the binary expansion of `r` as

    u 0 1^m,

so `r+1` is `u 1 0^m`, and put `c=popcount(u)`.  The cover graph between these ranks is the
disjoint union of `2^c` copies of

    K_(2^m,2).                                                  (PR4)

The component records the outer labels on the one-bits of `u`; after the carry, the new one-bit
has two outer labels and the `m` reset bits have none.  Formula (PR4) proves the normalized
matching inequality: if a lower-rank subset meets `d` components, it has at most `d 2^m` elements
and its upper neighborhood has exactly `2d` elements.  Thus `P_K` is a normalized-matching poset.

It also has a nested chain decomposition.  Inductively index the child chains `C_i` so that `C_i`
meets local rank `r` exactly when `i<=R_(K-1)(r)`.  In the two upper copies give the copies of
`C_i` global labels `2i-1,2i`.  For every global label `j<=2^(K-1)`, concatenate lower chain
`C_j` with the upper chain carrying label `j`; labels above `2^(K-1)` consist only of their upper
chain.  The resulting chain `D_j` meets a lower rank exactly when `j<=R_(K-1)(r)` and the
corresponding upper rank exactly when `j<=2R_(K-1)(r)`.  Its rank support is therefore contained
in that of every earlier chain, proving nesting.

The sorted multiset of rank numbers is

    2^ell repeated binomial(K,ell) times,       0<=ell<=K.

Its conjugate is `G_K`, because equivalently

    (G_K)' = (2^(K-j) repeated binomial(K,j) times, 0<=j<=K).

Consequently the nested-chain partition of `P_K` has shape `G_K`, not merely the same extremal
prefix sums.

For full mass, the Singleton Majorization Converse was therefore exactly the following special
case of the generalized Griggs dominance assertion:

> Every integer partition dominated by the nested-chain partition of the normalized-matching
> poset `P_K` occurs as the list of lengths of another chain partition of `P_K`.

Shahriari states this dominance assertion for arbitrary finite normalized-matching posets in his
[2008 chain-partition survey](https://2pcc.tcs.uj.edu.pl/archive/2pcc-Shariar-Shariari.pdf), while
Stanley's `nice`-graph language gives the equivalent incomparability-graph statement.  The broad
assertion is a conjectural template, not a theorem that can be imported here; even the Boolean-
lattice specialization is the long-standing Griggs dominance conjecture, still treated as such in
[Li--Qiu--Yang--Zhang (2024)](https://arxiv.org/abs/2408.13127) and in the current
[nice-graph literature](https://arxiv.org/abs/2608.16613).  The `K=6` state above shows that even
the recursively series-parallel poset `P_6` fails this dominance assertion.  Thus the connection
explains both why Greene--Kleitman extremality stops at necessity and how the counterexample fits
the surrounding chain-partition language.

#### The exact carry-compatible incidence target

The poset ranks separate the already-solved margin problem from the missing integral lift.  Given
a proposed chain-size partition `a`, make a zero--one matrix `M` whose rows are proposed chains and
whose columns are the ordered ranks `0,...,2^K-1`; put `M_(i,r)=1` when chain `i` is to use one
element of rank `r`.  Its margins must be

    row sums a_i,                 column sums R_K(r).            (PR5)

Since the conjugate of the sorted column-sum sequence is `G_K`, Gale--Ryser says that a matrix
with margins (PR5) exists exactly when `a<=_w G_K`.  Thus weak majorization solves the abstract
rank-incidence problem completely.

Call such a matrix **carry-compatible** if the one-entries in every rank can be bijected with the
actual elements of that rank so that the elements assigned to each matrix row form a chain of
`P_K`.  The full-mass converse was equivalent to:

> **Carry-Compatible Gale--Ryser Conjecture (false at `K=6`).** For every `a<=_w G_K` of mass `3^K`, at least one
> matrix with margins (PR5) is carry-compatible.

This was not an extra conjecture: a chain partition gives such a matrix by recording ranks, and a
carry-compatible matrix lifts back to the chain partition.  The `K=6` hole has the required
Gale--Ryser margins but admits no carry-compatible realization.

The compatibility recursion is precisely Pascal deletion/contraction.  The lower-half columns
form one `P_(K-1)` instance.  In the upper half, one row orientation must be used at every upper
rank occupied by that row, because the first outer symbol must remain `0` or remain `2`.  Every
upper column of size `2R_(K-1)(r)` must therefore be bisected by one common row coloring; after the
bisection its two halves are independent `P_(K-1)` instances.  All lower elements lie below all
upper elements, so the lifted lower and upper pieces in one row concatenate automatically.  This
is exactly the Balanced Pascal Realization conjecture in rank order.

One must choose the matrix jointly with the lift.  It is false that every Gale--Ryser matrix with
the correct margins is carry-compatible.  At `K=4`, use five rows `u,v,a,b,c`.  Give the four
upper-rank columns the neighborhoods

    rank 8:   {u,v},
    rank 9:   {u,v,a,b},
    rank 10:  {u,v,b,c},
    rank 12:  {u,v,c,a},                                    (PR6)

and give every incidence in every other rank a fresh row.  These columns have the required sizes
`2,4,4,4`; altogether the row-degree partition is

    (4,4,2,2,2,1^67) <=_w G_4.

Any lift would have to bisect all four neighborhoods by the first outer symbol.  The first
equation makes `u,v` opposite.  The next three then require successively `a,b` opposite, `b,c`
opposite, and `c,a` opposite, an impossible odd cycle.  Nevertheless the row-degree partition
itself is solvable: split one length-16 chain of the canonical `G_4` decomposition into chains of
sizes `4,4,2,2,2,1,1`, and split every other canonical chain into singletons.  Thus (PR6) refutes
only arbitrary-matrix lifting, not the converse.

The former exact target was to construct **some** rank-incidence realization whose dyadic carry
hyperedges have recursive common bisections.  The `K=6` state proves that even this joint choice may
not exist; counting margins, sorting ranks, and arbitrary matrix lifting explain only necessity.

### A canonical direct-transfer lemma and the failed global schedule

The nested decomposition has a stronger local property that is special to the Pascal construction.
Index its chains by `C_j`, `0<=j<2^K`, and put

    d(0)=0,                 d(j)=1+floor(log_2 j) for j>0.

Read the outer symbols of a word from left to right, write `0` for outer symbol `0` and `1` for
outer symbol `2`, and regard the resulting string as least-significant-bit first.  Unwinding the
recursive chain construction shows that `C_j` contains exactly one word over every binary rank
skeleton having at least `d(j)` one-bits: its successive outer labels are the bits of `j`, padded
by zeroes.  Consequently

    |C_j|=L_(d(j)),          L_d=sum_(s=d..K) binomial(K,s).     (PR7)

This description gives an exchange with no recoloring at all.

> **Canonical Bottom-Cell Transfer Lemma.** If `|C_j|>|C_q|`, one element can be moved from
> `C_j` to `C_q` while both sets remain chains.

*Proof.*  Write `d=d(j)<d(q)`.  In `C_j` take the word whose binary rank skeleton is
`0^(K-d)1^d`, namely rank `2^d-1`.  Its first `K-d` actual symbols are all `1`.  Every word of
`C_q` has more than `d` outer symbols, so it must have an outer symbol somewhere among those first
`K-d` positions.  At the first such position the two words differ as `1` versus `0` or `2`, hence
they are comparable before any possible `0`/`2` conflict.  The selected word is therefore
comparable with every member of `C_q`.  Deleting it from `C_j` and adjoining it to `C_q` preserves
both chains. ∎

Thus every first unit Robin--Hood transfer out of the canonical `G_K` coloring can be implemented
by moving one vertex directly.  Empty recipient classes, supplied by padding to `3^K` color
slots, are easier still.  This proves the precise local form of the intuition that the longer
Pascal chains contain compatible transfer targets.

The complete compatibility set also has a Pascal form.  Let a word `x` from a depth-`d` canonical
chain have outer positions

    0<=t_0<...<t_(s-1)<K

and outer-label bits `b_0,...,b_(s-1)`.  Consider receiver chains of a shorter length, hence of
code depth `e>d`.  Put

    m_e(x)=#{r : t_r-r<=K-e}.                                  (PR8)

The counted indices form an initial interval because `t_r-r`, the number of inner symbols before
the `r`th outer symbol, is nondecreasing.  A depth-`e` receiver code `q` is compatible with `x` if
and only if its binary code, padded by zeroes, agrees with `b_r` for every `r<m_e(x)`.  Indeed, a
first disagreeing outer label at occurrence `r` creates a conflict exactly when a receiver word
can copy the rank skeleton through position `t_r` and still place enough outer symbols to reach
depth `e`; that is precisely `t_r-r<=K-e`.

For `m_e(x)<e`, the compatible receivers are therefore one dyadic cylinder of size

    2^(e-1-m_e(x));                                             (PR9)

for `m_e(x)>=e` there are none, since the required leading bit of a depth-`e` code is one whereas
the depth-`d` donor code is already padded by zero.  The bottom cell used in the lemma has
`t_r=K-d+r`, so `m_e(x)=0` for every `e>d`: its cylinder is the entire receiver block.  As
successive cells are exposed, the allowable block halves only at the explicit thresholds (PR8).
This is the rigorous form of “pick one of the many identical targets with the right coloring.”

There remains a real list-coloring interaction.  Individual compatibility with one receiver is
not closed under union: already at `K=2`, the words `10` and `12` are each compatible with the
canonical singleton chain `{02}`, but they conflict with each other.  Thus (PR8)--(PR9) turn the
global schedule into a laminar dyadic receiver-list problem, but one must also keep the incoming
words assigned to each receiver mutually compatible.  Ignoring that second condition would be a
new form of the same arbitrary-incidence lifting error exposed by (PR6).

The property is not automatically preserved.  At `K=3`, use the canonical chains

    C_1=(112,121,120,211,210,201,200),
    C_2=(102,012,021,020),
    C_3=(122,212,221,220).

The word `112` can be moved directly from `C_1` to `C_3`.  Afterwards the donor has size six and
`C_2` has size four, but no second direct transfer between that pair exists: `121,120` each
conflict with `102`, while `211,210,201,200` each conflict with `012`.  Hence iterating the lemma
arbitrarily does not prove dominance closure.  The choice of the earlier recipient changes which
later moves remain available; this is exactly where a global schedule, rather than a local
exchange rule, is needed.

The resulting sufficient statement was clean but is false universally.

> **Canonical Monotone-Transfer Conjecture (false at `K=6`).** Pad the canonical `G_K` coloring of `Q_K` by empty
> classes.  For every full-mass partition `a<=_w G_K`, there is a path from that coloring to one
> of type `a` in which every step moves one vertex from a class of size at least two more than the
> recipient, and that vertex is already nonadjacent to the entire recipient class.

This conjecture would prove the Singleton Majorization Converse and would realize the proposed
“no cyclic reassignment” idea literally: the schedule is global, but each scheduled operation is
a single direct move.  It is stronger than the required existence theorem, so it must not be used
as an equivalent reformulation.  The unreachable `K=6` target refutes it; the first-move lemma and
the complete `K=3` reachability census remain valid local results.

There is exact finite evidence for the new statement.  At `K=3`, all 1,206 full-mass dominated
types are reachable by such monotone direct moves.  A forward pass retaining one coloring of each
type for which every current size gap still has a direct move reaches 1,201 types; targeted direct
paths reach the remaining five.  This is a constructive census, not an inference from the earlier
Row-Coloring census.  The source and exact output are in the
[monotone-transfer census record](../../evidence/singleton_monotone_transfer_census_2026-08-29.md).

Before the counterexample, the proposed next obligation was to show that **some** admissible direct
move preserves reachability to a target `a`.  The `K=6` hole shows that this cannot hold for every
dominated target.  The duplicated-recipient structure still explains the successful lower-level
paths but does not supply universal reachability.

### The solution-fiber DAG and a Pascal phase birth (2026-08-30)

There is a complementary way to retain exactly the compatibility that a one-representative walk
forgets.  Above every normalized parent `x<=_w G_K`, keep the whole finite fiber of legal first
cuts.  Above a parent transfer `x->y`, join two cuts when one coin moves from the marked donor row
to the marked recipient row in the same child coordinate and every other row allocation remains
fixed.  Call this the **literal allocation-transport relation**.  It is stronger than
Adjacent-Fiber: a common Hall coloring may rebuild both endpoint allocations, whereas a literal
transport may not.

The complete `K=3` relation is unexpectedly coherent.  Across all 1,206 parents, 8,916 parent
edges and 1,063,464 normalized allocation orbits, every parent edge has a literal lift.  The unique
cut of `G_3` reaches 1,063,144 allocation orbits and, more importantly, at least one orbit above
every parent.  Thus the following sufficient strengthening holds exactly at `K=3`.

> **Canonical Allocation-Transport Conjecture (false at `K=6`).** For every full-mass `a<=_w G_K`, some legal cut
> above `a` is reachable from the canonical cut above `G_K` by literal allocation transports.

The conjecture would prove the converse, but it is not equivalent to it and the `K=6` target has
no cut to reach.  Nor does edgewise
nonemptiness prove it: 916 of the `K=3` parent edges kill at least one source allocation orbit, so
independently selected edge certificates need not compose.  The correct inductive object is the
reachable subfiber `R(x)`, not one chosen cut.

The full solution fiber cannot itself be generated from `G_K`.  There is a uniform, proved Pascal
phase birth.  For `K>=3`, put `U=2^(K-1)`, `w=U-K`, and `h=G_(K-1)`.  The canonical head rows are

    (U,U,0), (0,U-1,U), (U-1,w,0), (0,w,U-1).

Transfer one parent coin from the second row to the third and replace these four allocations by

    (0,U,U), (0,U-1,U-1), (U,w,0), (U-1,w,0).                (PB)

The new parent has head `(2U,2U-2,2U-K,2U-K-1)`.  All three children remain exactly `h`: the mixed
parts are unchanged, the two pure heads are both `(U,U-1)`, and the paired Pascal tail is untouched.
Thus (PB) is a legal cut for every `K>=3`.

This cut has no literal predecessor.  The new parent has Lorenz slack one against `G_K` only at
prefix two, so `G_K` is its unique more-head-heavy one-unit predecessor.  In any cut of `G_K`, the
top two parent rows must use opposite pure sides, since

    2U+(2U-1) > 2 H_h(2)=4U-2.

But (PB) puts the unchanged top row and the shrunken donor on the same pure side, which literal
transport cannot do.  This is a true phase change: a previously violated pure-side inequality
becomes equality and a four-row Pascal reassociation creates a new solution.

There is now a complete theorem for the whole first dominance shell, not only this one edge.  Put

    v_d=sum_(s=d..K) binomial(K,s),

so the canonical profile is

    G_K=(v_0,v_1,v_2^2,v_3^4,...,v_K^(2^(K-1))).             (PB1)

For `1<=d<=K-1`, let `X_(K,d)` be obtained by transferring one coin from the last `v_d` row to
the first `v_(d+1)` row.  These are exactly the `K-1` normalized states whose Lorenz-area deficit

    D(x)=sum_(t>=1) (H_K(t)-H_x(t))

equals one.  A coloring orbit is normalized by equal-row permutations and global side exchange.

> **First-Shell Pascal Phase Theorem.** For every `K>=3`, the feasible Hall-coloring fibers are:
>
> - `G_K` has one orbit;
> - `X_(K,1)` has three orbits, two literally inherited from `G_K` and the (PB) orbit;
> - every `X_(K,d)` with `2<=d<=K-1` has four orbits, all literally inherited from `G_K`.
>
> Consequently the Hall-coloring solution DAG restricted to `D<=1` has exactly two source
> orbits: the canonical orbit and (PB).  In particular no deeper Pascal block boundary creates a
> new coloring phase.

*Proof.*  Let `h=G_(K-1)` and let `H` be its saturated prefix function.  In any feasible coloring
of one of these parents, consider the top `t=2s` rows and let `p` of them have color `A`.  They are
the `p` largest `A` rows and the `2s-p` largest `B` rows, so (C) and the Pascal prefix identity give

    H_K(2s)-epsilon <= H(2s)+H(p)+H(2s-p),
    H_K(2s)          = H(2s)+2H(s),                           (PB2)

where `epsilon` is one only at the transferred boundary and zero elsewhere.  Concavity of `H`
maximizes the last two terms at `p=s`.  If `p!=s`, their loss from the balanced maximum is at least

    h_s-h_(s+1).                                                (PB3)

At the dyadic boundary `s=2^(e-1)`, this is exactly the adjacent Pascal jump

    h_s-h_(s+1)=binomial(K-1,e-1),                              (PB4)

with zero padding after the last child row.  Hence every dyadic prefix is exactly bisected except
possibly the transferred prefix.  At that prefix an imbalance is possible only for `e=1`, where
the loss is one; for every `e>=2` it is at least two.

For `G_K`, exact bisection at the successive dyadic endpoints forces the top two distinct rows to
be opposite and every later equal-value block to split equally.  This is the unique normalized
canonical coloring.  For `X_(K,d)`, `d>=2`, all endpoints are still bisected.  The color of the
modified donor row may be chosen in two ways and compensated inside its remaining equal block;
independently the modified recipient row may be chosen in two ways.  Thus there are at most four
orbits.  All four exist literally: choose canonical donor and recipient chains of the prescribed
colors and apply the Canonical Bottom-Cell Transfer Lemma.  Its moved word begins with the mixed
symbol `1`, so the first-test row colors and every other class are unchanged.

For `d=1`, normalize the largest row to color `A`.  At prefix two, (PB2)--(PB4) allow either one
or two `A` rows.  Prefix four is again exactly bisected.  If the top two rows are opposite, the
modified recipient and its equal mate may be oriented in two ways, and both are the literal
bottom-cell transfers from `G_K`.  If the top two are together, the next two are forced together
on the other side, giving at most one further orbit; construction (PB) supplies it.  This proves
the fiber counts.  Finally `D` strictly increases along every Robin--Hood edge.  Every area-one
parent therefore has only the area-zero parent `G_K` as a possible predecessor.  The two inherited
orbits (or all four at deeper boundaries) have such predecessors, while (PB) cannot: its top two
rows are together and the unique `G_K` coloring separates them.  This proves the source claim. ∎

The same statement for **complete allocation fibers** is verified, not proved, through `K=5`.
At each of `K=3,4,5`, `G_K` has one cut orbit, `X_(K,1)` has four (three inherited and (PB)), and
each deeper `X_(K,d)` has six, all inherited.  The extra allocation choices record which Pascal
child receives the descending unit defect.  Thus the first-shell data support a useful recursive
picture: a defect at a deeper boundary descends one Pascal level inside a child; only a defect at
the first jump can be absorbed by the local four-row reassociation.

That last sentence applies only to **one unit** of slack.  Several units can meet the exact price
of a deeper Pascal imbalance and create a genuinely new phase.  The right framework for all such
phases is an integral-polymatroid greedy reduction.

Fix a coloring of padded labelled rows by `A/B` and put

    f(S)=H(|S|)+H(|S intersection A|)+H(|S intersection B|).    (PG1)

The Fixed-Color Hall inequalities say exactly that the demand vector `x` is an integer base of
`f`: `x(S)<=f(S)` for every row set and `x(E)=f(E)=3^K`.  Order the rows by nonincreasing demand.
If an integer base admits no feasible headward exchange `x+e_i-e_j` with `i<j`, the standard
polymatroid greedy criterion makes it the greedy base for this order.  Writing `h=G_(K-1)`, its
coordinates are therefore

    x_i=h_i+h_(r_i),                                           (PG2)

where `r_i` is the occurrence number of the color of row `i`, and `h_i=0` after the `2^(K-1)`th
positive child row.  Since the coordinates are already ordered, the sums in (PG2) must be
nonincreasing.

Conversely, any balanced `A/B` word for which the sums (PG2) are nonincreasing gives a legal cut:
give row `i` mixed part `h_i` and give it pure part `h_(r_i)` on the side named by its color.  The
mixed parts and each color's pure parts are exactly three copies of `h`.  Call this a
**self-sorted Pascal greedy shuffle**.

> **Pascal Greedy-Source Reduction.** Every feasible full-mass coloring is reachable, by literal
> color-preserving Robin--Hood transports, from a self-sorted Pascal greedy shuffle.  Every source
> orbit of the Hall-coloring solution DAG is therefore one of these shuffles.  Each shuffle has
> exactly `2^K` positive parent rows and has an explicit cut with all three children equal to
> `G_(K-1)`.

*Proof.*  Starting from a feasible colored demand vector, perform any feasible headward unit
exchange from a no-smaller row to a no-larger row and resort.  The sum of squared row demands
strictly increases, so the process terminates.  At termination the integral-base local-optimality
criterion for a strictly decreasing row-weight vector gives the greedy formula (PG2).  A zero
coordinate cannot precede a later positive coordinate.  After the mixed sequence is exhausted,
this forces both colors to use all `2^(K-1)` positive pure increments before any zero, so there are
exactly `2^K` positive rows and the word is balanced.  Reversing the exchanges gives the asserted
literal transports.  The displayed allocation proves the converse construction. ∎

This is the precise restricted isomorphism suggested by the rigid-state surveys: not every parent
is canonically isomorphic to its cuts, but every point at which a new solution component can enter
is the sorted recombination of three canonical children, encoded by one balanced binary word.
It also makes source classification tiny.  A dynamic program only has to retain the current word
position, its two occurrence counts, and the equal-sum blocks of (PG2).  A greedy shuffle can have
an incoming edge only by separating two equal parent rows: for unequal rows the earlier greedy
prefix is tight and forbids a headward exchange.  Testing all equal-row pairs by (C) is therefore
an exact global source test, independent of the number of dominated parent partitions.

The first new family explains why the first-shell theorem cannot be extrapolated.  Let
`s=2^(e-1)` and

    c_e=h_s-h_(s+1)=binomial(K-1,e-1).                          (PG3)

Transfer `c_e` coins from the last row of the `v_e` block of `G_K` to the first row of the
`v_(e+1)` block.  The two rows remain in order precisely when `K>=2e`; their new gap is

    binomial(K-1,e)-binomial(K-1,e-1).                          (PG4)

At `K=2e` they tie and the corresponding greedy colorings inherit through an equal-row exchange.
For `e>=2` and `K>=2e+1`, they are distinct and there are two source colorings with an imbalance
of one at the top `2^e` rows.  Explicitly, begin with the canonical greedy word.  In the size-`s`
block and following size-`2s` block, replace respectively

    A^(s/2) B^(s/2),       A^s B^s

by either

    A^(s/2) B^(s/2-1) A,  B A^(s-1) B^s,

or

    A^(s/2-1) B^(s/2+1),  A^(s+1) B^(s-1).                    (PG5)

The occurrence counts realign after those two blocks.  Formula (PG2) shows that only the two
boundary rows change, by `-c_e,+c_e`, so all three children remain `h`.  The strict inequality in
(PG4) keeps the modified rows out of every equal block.  Every other equal block lies inside flat
mixed and pure blocks of `h`, where changing the order has zero exchange capacity.  Hence no
headward unit exchange exists and both colorings are source orbits.  For `e=1`, the two
orientations coalesce under global side exchange and recover (PB).

Thus higher phases appear at the exact Pascal prices, not arbitrarily.  The `e=2` family is born
at `K=5` with area `binomial(4,1)=4`; the `e=3` family is born at `K=7` with area
`binomial(6,2)=15`, and so on.  Exact greedy-shuffle/source enumeration gives:

| `K` | self-sorted greedy coloring orbits | source orbits | source-area multiplicities |
|---:|---:|---:|---|
| 2 | 2 | 1 | `0` |
| 3 | 2 | 2 | `0, 1` |
| 4 | 5 | 2 | `0, 1` |
| 5 | 6 | 6 | `0, 1, 4^2, 5, 19` |
| 6 | 25 | 6 | `0, 1, 5^2, 6, 23` |
| 7 | 25 | 25 | `0, 1, 6^2, 7, 15^2, 16^2, 21^4, 22^2, 27, 42, 60^2, 61^2, 66^2, 67, 87` |
| 8 | 227 | 30 | `0, 1, 7^2, 8, 21^2, 22^2, 28^4, 29^2, 31, 52, 84^2, 85^2, 91^2, 92, 115, 224^2, 225, 241, 427` |

These are exact finite classifications of **all possible coloring sources** at the displayed
levels, not samples of the parent corpus.  For example, at `K=5` the two area-four source
colorings lie over

    (32,31,26,22,20,16^3,6^8,1^16),

and further sources occur at areas five and nineteen.  An independent downward-closed fiber
census through area five finds exactly the first four noncanonical sources predicted by this
classification: 267 parents, 866 transfers, 5,089 coloring orbits and 19,113 literal links, with
every transfer nonempty and every parent reached by the canonical component.

The reduction does not prove that every partition dominated by `G_K` is feasible.  It gives an
exactly equivalent global target with no arbitrary starting assignment:

> **Pascal-Shuffle Coverage Conjecture (false at `K=6`).** The projections of the color-preserving downward
> exchange cones of all self-sorted Pascal greedy shuffles cover every full-mass partition
> dominated by `G_K`.

One direction is the explicit shuffle construction and preservation of (C); the other is the
Greedy-Source Reduction.  This formulation completes the proposed phase-change analysis: the
phase anchors are now explicit and recursively Pascal-shaped, while coverage of the gaps between
their cones was the remaining content of this route.  The `K=6` hole lies outside every such cone.

#### Bidirectional exchanges collapse every coloring phase (2026-08-30)

The source proliferation is entirely caused by orienting every exchange toward the tail.  If a
unit may move in either direction, there is an all-level connectivity theorem.  This theorem is
about Hall colorings: the allocation witnessing Hall feasibility may be rebuilt after a move.  It
does not assert connectivity of the stronger complete-allocation relation.

Pad to `N=3^K` labelled row slots, put `m=2^(K-1)`, and fix a coloring
`E=A disjoint-union B`.  The feasible full-mass demand vectors are the integer bases of

    f_A(S)=H(|S|)+H(|S intersection A|)+H(|S intersection B|).       (BG1)

The full-set inequality forces `|A|,|B|>=m`: the parent mass is `3^K`, while each of the three
terms in (BG1) is at most `3^(K-1)`.  Choose any `m` slots of each color and put the canonical
`G_K` rows of that color in those slots, leaving every other slot zero.  The result `g^A` is a
permutation of `G_K` and is an integer base of the same `f_A`, witnessed by the canonical cut.

> **Bidirectional Fiber Connectivity Theorem.** Every feasible padded labelled Hall coloring is
> connected to a colored permutation of `G_K` by color-preserving unit exchanges whose
> intermediate parents are full-mass and majorized by `G_K`.  Consequently, after row
> normalization and global side exchange, the complete Hall-coloring solution graph is one
> undirected component.

*Proof.*  Integer bases of an integral polymatroid form an M-convex set.  Its unit-exchange graph
is connected, so the given demand vector and `g^A` are joined by exchanges `x-e_i+e_j` that remain
bases of (BG1).  Every intermediate base is still a valid parent: if `|S|=t` and
`p=|S intersection A|`, then concavity of `H` gives

    x(S) <= f_A(S)
         <= H(t)+H(ceil(t/2))+H(floor(t/2))
          = H_K(t).                                             (BG2)

Thus its decreasing rearrangement is majorized by `G_K`.  Finally every colored permutation
`g^A` normalizes to the canonical `G_K` coloring.  Reversing exchanges when necessary joins all
feasible coloring orbits to that one orbit. ∎

For example, the new directed source at `K=3` joins an inherited coloring over the **same parent**
in one labelled-row move.  Transfer one coin from its color-`A` row of width six to its color-`B`
row of width five.  The sorted parent remains `(8,6,5,4,1^4)`, while

    A=(8,6,1,1), B=(5,4,1,1)

becomes the inherited coloring

    A=(8,5,1,1), B=(6,4,1,1).

If normalized self-moves are suppressed and every edge must change the sorted parent, the same
connection is a three-edge detour:

    (8,7,4,4,1^4) -> (7,7,5,4,1^4) -> (7,6,6,4,1^4)
                    -> (8,6,5,4,1^4),

where the row-width moves are `8->4`, `7->5`, and then the reverse move `6->7`.  The final coloring
is the (PB) source.

Exact undirected enumeration agrees.  The complete `K=3` graph has one component containing all
31,498 coloring orbits.  The complete `K=4`, `D<=14` ideal likewise has one component containing
all 60,486 orbits.  The truncated `K=5`, `D<=5` ideal has one component of size 5,088 plus one
isolated area-five source orbit; that source is on the truncation boundary, so its outgoing
detour lies outside the ideal.  The large component nevertheless projects onto all 267 surveyed
parents.  The theorem says the isolated orbit reconnects when the full graph is retained.

This removes phase bookkeeping but does **not** prove existence.  Let `F_K` be the union of the
integer bases of (BG1) over all row colorings, and let `B_K` be all padded full-mass integer vectors
majorized by `G_K`.  We know

    F_K subset B_K,                 conv(F_K)=conv(permutations of G_K).

The convex-hull equality holds because `F_K` contains every permutation of `G_K`, while those
permutations are exactly the vertices of the ambient permutahedron.  The Row-Coloring conjecture
was precisely the lattice-saturation statement `F_K=B_K`; the `K=6` counterexample proves the
inclusion strict.  Connectivity of `F_K` cannot rule out a missing interior lattice point.  The
generic profile `h=(6,1)` already demonstrated the distinction:
all of its existing colored bases have the same bidirectional connectivity, but the ambient point
`(12,3,3,3)<=_w(12,7,1,1)` is not colorable.

Thus allowing both directions gives a cleaner global picture: the normalized colored graph is
connected, while the corresponding labelled union is vertex-spanning and has the right convex
hull.  It nevertheless has a lattice hole.  In particular, `F_6` is not M-convex.  Reverse moves
connect existing feasible colorings; they cannot create a coloring over a point outside `F_6`.

#### Real convexity fails even at exact support (2026-08-30)

Convexity of the union of **all padded real** fixed-color base polytopes is false even for the
Pascal profile at `K=2`.  Put `h=(2,1)`.  Then

    x=(4,3,2/3,2/3,2/3) <=_w (4,3,1,1),

but `x` has no Hall coloring.  Full mass forces at least two rows of each color.  Put row four in
`A`; row three must be in `B`, since `7>2H(2)=6`.  If `A` has two rows, `(p,q)=(1,3)` has demand
`25/3>8`; if it has three rows, `(3,1)` gives the same violation.  Scaling by three gives the
integer hole `(12,9,2,2,2)` for the non-Pascal scaled child `3G_1=(6,3)`.  Thus no homogeneous
real-polytope or scale-invariant argument can prove the primitive integer theorem.

The obstruction uses five positive rows, while `G_2` has four.  It is therefore natural, but also
false, to hope that exact support repairs real coverage.  On exactly `2^K=2m` labelled rows put

    U_K^min = union_(|A|=m) B(f_A).

This union lies in `Perm(G_K)` and contains all of its vertices.  Nevertheless it need not fill
that permutahedron.  Already

    (8,7,4,(8/5)^5) <=_w G_3=(8,7,4,4,1,1,1,1)              (BG3)

has no balanced Hall coloring.  Any four-versus-four coloring splits the first three rows `2+1`;
the whole side containing two, together with the remaining head row, has mass `111/5`, above its
`(4,1)` capacity `H(5)+H(4)+H(1)=22`.

This has an all-level form.  Let `M=3^(K-1)`, `m=2^(K-1)`, `g=G_K`, and flatten the final `m+1`
coordinates of `g` to their average:

    a=(g_1,...,g_(m-1),t^(m+1)),       t=1+K/(m+1).           (BG4)

Then `a<=_w g` and has exact support.  In every balanced coloring one side contains at least
`m/2` of the first `m-1` rows.  Taking that whole side plus the remaining head rows on the other
side gives Hall excess at least

    ((K-2)m-2)/(2(m+1)).                                      (BG5)

This is positive for every `K>=3` and already exceeds one at `K=5`.  Multiplication by `m+1`
gives an integer hole for the scaled profile `(m+1)G_(K-1)`, but not for the primitive profile.
Consequently neither a real exact-support cover nor a universal real defect-below-one theorem could
have proved the integer Singleton Majorization Converse.  The later `K=6` example shows that the
primitive unit lattice itself also has a hole.

The primitive lattice repairs the entire final band on this tight face.  The last `m+1` rows of
`G_K` are `(K+1,1^m)`, and `I(m-1)={m/2-1,m/2}`.  Hence the head-light side of every legal
allocation of a tight first `m-1` rows has `m/2+1` tail slots.  Any positive integral tail has only
`K` coins above its unit baseline and therefore at most `K<=m/2+1` non-unit rows.  Assign all of
them to the head-light side, concatenate the canonical tail allocation, and obtain the desired
tail by same-color Robin--Hood transfers.  Such transfers only decrease the largest demand at
every fixed pair of color counts.  This proves the **Integral Final-Band Extension Lemma**: every
legal head-band allocation extends across an arbitrary integral tail on the tight `m-1` face.  It
does not supply that head allocation, so it is a genuine partial result rather than the converse.

The rational census locates the first face exactly.  At `K=3`, denominators one through four have
no hole on 160, 3,997, 34,704 and 179,482 normalized states.  Denominator five has 56 holes among
675,341 states, beginning with `(40,35,20,8^5)`; all 56 lie on the face
`(x,y,z,8^5)`, `x+y+z=95`, and have scaled Hall defect one.  Exact support **does** give real
coverage at `K=2`, by the direct largest-with-smallest coloring proof in the evidence record.

The failed real statement still has a useful rounding formulation.  Linearly interpolate `H`; for
a fixed exact-support parent `a`, let `Y(a)` contain the vectors `y in [0,1]^E`, `y(E)=m`,
satisfying

    a(S) <= r_y(S):=H(|S|)+H(y(S))+H(|S|-y(S))               (BG6)

for every `S`.  Integral `y` are exactly Hall colorings, while the all-half vector is feasible by
parent majorization.  The set `Y(a)` is convex because each constraint is a superlevel set of a
concave function of `y(S)`.  For fixed `y`, `r_y` is submodular.  Hence active sets uncross: if
`X,Z` are tight, modularity of `a` and submodularity of `r_y` force `X union Z` and
`X intersection Z` to be tight as well.  The counterexamples (BG3)--(BG5) show that this
uncrossing cannot force an integral `y` for real `a`.

This does not make `Y(a)` an integral generalized polymatroid.  For canonical `G_3`,
the least allowed integral color count is one on rows `{8,7}`, zero on a unit row, and zero on
`{8,7,1}`; the lower-bound function is therefore not supermodular.  The proposed integer-aware
version asked whether `Y(a)` contains a zero--one point when `a` is integral for the primitive
Pascal profile.  The `K=6` counterexample answers no.  Definitions, the real counterexample proof,
the short `K=2` theorem, exact
commands and all grid counts are in the
[exact-support real-cover record](../../evidence/singleton_exact_support_real_cover_2026-08-30.md).

At `K=3` this is the only noncanonical source orbit.  It lies above
`(8,6,5,4,1^4)` and reaches 1,059,979 allocation orbits, including all 320 missed by the canonical
source; the two descendant sets overlap on 1,059,659.  Their union is the entire cut corpus.  The
coarser Hall-coloring DAG has the same two-source shape.

There is also an exact downward-closed `K=4` boundary survey.  Because `D` strictly increases on
every parent edge, the parents with `D<=14` contain every predecessor of each of their vertices.
They comprise 2,852 parent states and 26,067 transfers.  Their 60,486 Hall-coloring orbits and
871,752 complete allocation orbits have respectively 719,077 and 9,969,849 literal links.  Every
edge lifts; the canonical components hit all 2,852 parents.  In both relations (PB) is still the
only noncanonical source.  The allocation component reaches 784,351 cut orbits; the phase
component reaches 434,873, covers all 87,401 cuts missed canonically, and overlaps it in 347,472.
This is a complete theorem about the bounded ideal, not evidence that no further source occurs at
larger `D`.

The source-classification part of the earlier target is resolved for Hall colorings by the
Greedy-Source Reduction: higher Pascal phases really do occur and are not all descendants of
(PB).  The proposed remaining target was coverage by the downward cones of the explicit phase
anchors.  The `K=6` state is a hole in that projection and refutes Pascal-Shuffle Coverage, while
the surveyed bounded ideals remain exact finite theorems.  The exact definitions, counts, all-level phase constructions, programs and
reproduction commands are in the
[solution-fiber DAG record](../../evidence/singleton_solution_fiber_dag_2026-08-30.md).

### A low-level cut-and-splice normal form (2026-08-29)

The direct-transfer path may be stronger than necessary.  There is a simpler global construction
visible in the canonical chains.  Order every `C_j` by Pascal rank and call a consecutive subset a
**canonical interval**.  Cut the canonical chains into such intervals and use each interval once.
The following finite statement is exact.

> **Two-Interval Splicing Theorem (`K<=3`, exhaustive).** Every full-mass
> `a<=_w G_K`, for `K<=3`, has a chain partition of type `a` in which every target chain is either
> one canonical interval or the union of two canonical intervals satisfying all three conditions:
>
> 1. their source chains have different code depths;
> 2. their Pascal-rank ranges are disjoint; and
> 3. their union is a chain.

The proof is an exact-cover enumeration on the actual words of `{0,1,2}^K`.  It independently
enumerates the 2, 15 and 1,206 dominated types at `K=1,2,3`, constructs all permitted interval
unions, and covers every word exactly once with the requested part sizes.  At `K=3`, 821 candidates
suffice and all 1,206 targets pass.  This is a finite computer proof, not an induction in `K`; the
source and full restriction survey are in the
[cut-and-splice record](../../evidence/singleton_cut_splice_survey_2026-08-29.md).

Rank separation makes condition 3 especially small.  If interval `I` is wholly below interval
`J`, then `I union J` is a chain exactly when the top endpoint of `I` is below the bottom endpoint
of `J`; transitivity handles every other cross-pair.  Thus a target chain follows one canonical
chain, crosses one endpoint edge, and follows a second canonical chain.  It never needs an
arbitrary collection of recolored cells.

Splicing is genuinely doing work.  Cuts without joins cover only 11 of 15 types at `K=2` and 591
of 1,206 at `K=3`.  On the other hand, same-depth splices and rank-interleaved splices are never
needed at `K=3`.  Orienting every splice from smaller to larger source depth therefore gives a
directed acyclic source graph.  This realizes the no-cycle intuition at the level of the final
partition, without asserting a sequence of unit transfers.

Two tempting further restrictions are false.  Allowing only adjacent source depths misses
`(8,3,3,3,3,3,3,1)`, even though allowing depth gap at most two covers all `K=3` targets.  Also,
neither fixed rank direction works: forcing the shallower interval always below the deeper one
misses `(8,7,3,3,3,1,1,1)`, while the reverse convention misses
`(8,7,4,3,2,1,1,1)`.  The global matching must choose the rank direction of each splice.

This suggests another clean sufficient statement.

> **Pascal Two-Interval Splicing Conjecture (false at `K=6`).** The displayed normal form exists for every `K` and
> every full-mass `a<=_w G_K`.

Unlike the Carry-Compatible Gale--Ryser Lemma, this is a strengthening rather than an equivalent
reformulation.  Unlike the Canonical Monotone-Transfer Conjecture, it asks only for the final
partition and supplies no direct-move history.  Its prospective proof is a coupled cutting and
endpoint-matching theorem: choose interval endpoints in the canonical Pascal chains so that the
resulting fragments of the requested lengths can be paired across distinct depths.  The strict
depth orientation removes cyclic dependencies, while majorization should provide the Hall
inequalities for the endpoint matching.  Since the normal form would be a chain partition, the
`K=6` counterexample proves that it cannot always exist.

At `K=4` the normal form has 456 intervals and 20,542 candidates.  Five deliberately varied
majorized targets have exact covers; among 100 seeded Robin--Hood-walk targets, 85 have covers and
15 hit a 20,000-node cap, with no exhaustive failure.  This is exploratory evidence only and does
not establish the conjecture at `K=4`.

### Why two identical children are not always available (2026-08-29)

A natural stronger decomposition would require at least two of the three sorted child sequences
to be identical.  This holds for every full-mass target through `K=2`, but it is false at `K=3`,
even when equality is only up to row permutation.

Take

    a=(8,3,3,3,3,3,3,1) <=_w G_3.

Every child has mass nine and lies below `G_2=(4,3,1,1)`, so every child part is at most four.  The
parent row of size eight must split as `4+4` between the mixed child and one pure child, say `M`
and `L`.  No other parent row can make a part four.  Hence any equal pair must be `L=M`, with common
type `(4,q)`.  The child prefix bounds leave only

    q=(3,1,1), (2,2,1), (2,1,1,1), or (1,1,1,1,1).

The remaining components must form six parent rows of size three and one of size one.  Thus every
two must pair with a one and exactly one one remains unmatched.  For `q=(3,1,1)`, the third child
must contain at least three twos, but the mixed remainder has only two ones.  For `q=(2,2,1)`, the
two left twos compete for the sole mixed one.  For `q=(2,1,1,1)`, the left two and at least three
right twos compete for three mixed ones.  The all-one remainder would force six parts of size at
least two into the third child's mass nine.  All four cases are impossible because pure parts can
pair only with mixed parts, never with each other.

This is not an unsolvable state.  It has the explicit valid split

    L=(4,3,1,1),       M=(4,2,2,1),       R=(3,3,2,1),

obtained by matching `L4+M4`, twice `L1+M2`, and `R2+M1`, and leaving
`L3,R3,R3,R1` unmatched.  The three child types are distinct.

This valid split is not singular.  There are eight ordered child-type triples producing the
counterexample, four after the genuine `L<->R` outcome symmetry, and six normalized row-allocation
orbits after also quotienting equal child parts and equal parent rows.  For example the inequivalent
triple `(3,2,2,2)`, `(4,1^5)`, `(4,2,2,1)` recombines by matching the two-bearing pure parts to the
five mixed ones and matching the two fours.  Thus the failure of identical children is structural,
not an artifact of a unique exceptional decomposition.

An exact partial-matching census finds 16 such exceptions among the 1,206 `K=3` parent types:
1,190 admit an identical-child split.  A nearby statement survives completely at this level:
every `K=3` target has a valid split with two children equal or one Robin--Hood transfer apart, and
every target has one whose three child types form a dominance chain.  In the displayed split the
chain is `L>M>R`, with both steps unit transfers.  These weaker statements are finite observations,
not general theorems.  Method, the full exception family and reproduction source are in the
[identical-child census](../../evidence/singleton_identical_children_census_2026-08-29.md).

### The rigid split corpus and a multiplicity filtration (2026-08-29)

The multiplicity can be removed exactly at low levels.  Call a parent **child-unique** if it has
one normalized child-type triple `(L,M,R)` modulo `L<->R`, and **cut-unique** if that triple also
has one normalized row-allocation multiset.  Complete counts are

| `K` | all parents | child-unique | cut-unique |
|---:|---:|---:|---:|
| 1 | 2 | 2 | 2 |
| 2 | 15 | 4 | 3 |
| 3 | 1,206 | 9 | 6 |
| 4 | 5,997,038 | 30 | 8 |

Write the four child-unique `K=2` states as

    A=(1^9), B=(4,1^5), C=(4,2,2,1), D=G_2.

The nine unique `K=3` child triples are represented by

    AAA, ABB, DCD, BDB, CDC, CDD, BDD, CDD, DDD.              (RU1)

The repeated `CDD` corresponds to two different parents.  In the same order the parents are

    (1^27), (8,1^19), (8,5^3,1^4), (8,7,1^12), (8,7,2^6),
    (8,7,3^3,1^3), (8,7,4,1^8), (8,7,4,2^3,1^2), G_3.       (RU2)

The cut counts are respectively `1,1,1,1,1,2,2,4,1`.  Thus the six count-one rows are the fully
rigid corpus.  The interior example `(8,5^3,1^4)` is especially clean: its forced children are
`(D,C,D)`, and its forced normalized cut is

    (0,4,4), (4,1,0), (3,2,0), (0,2,3),
    (1,0,0)^2, (0,0,1)^2.

This finite `K=3` corpus gives a precise form to the proposed parent/children resemblance.  Every
child of a child-unique `K=3` parent is itself one of the four child-unique `K=2` states, and every
unique triple has a repeated child.  Rigidity is therefore hereditary and symmetric through
`K=3`, even though two identical children are false for general parents.

It is not yet a literal isomorphism.  The `CDD` collision maps one child triple to two parents, and
those parents have two and four cuts.  At `K=2`, the all-`(2,1)` triple similarly maps to both
`(4,2,2,1)` and `G_2`, with two cuts and one.  Child types lose exactly this cut information.  The
fully cut-unique subcorpus removes the ambiguity locally, but no general reconstruction formula is
proved.

In the partial-matching language, the missing datum is explicit: the child parts are weighted
vertices, a cut matches mixed vertices to pure vertices, and the parent rows are matched sums plus
unmatched weights.  Hence the natural rigid isomorphism is
`parent <-> (child triple, unique weighted matching)`, not `parent <-> child triple` alone.

The natural Rigidity-Heredity Conjecture said that every child-unique parent has three child-unique
children and a repeated child type.  The exact parent-first `K=4` census refutes both clauses.  A
single counterexample is

    a=(16,15,9^3,5^3,1^8),

whose forced child triple is

    (8,6,5,4,1^4), (8,7,4,3,2,1^3), G_3.

The three children are distinct.  The first two each have exactly two child orbits at `K=3`, while
only `G_3` is child-unique.  This parent has two allocation orbits.

The failure exposes a weaker recursive filtration.  Among the 30 child-unique `K=4` parents,
exactly 5, 15 and 10 have respectively one, two and three child-unique children; every other
forced child has exactly two child orbits.  Only 13 of the 30 forced triples repeat a child.  Thus
the surviving finite statement is

> **Multiplicity-Filtration Conjecture.** A child-unique parent has at least one child-unique
> child, and all of its children have at most two child orbits.

It is verified only through `K=4`.  The exact child-orbit layers one, two, three and at least four
at `K=4` contain respectively `30,123,106,5,996,779` parents.  The corresponding allocation-orbit
layers one, two and three contain `8,19,32`.  Extending the survey beyond singular fibers is
therefore genuinely informative: it reveals the first bifurcation of rigidity rather than merely
adding noisy solutions.

Rigid children are not universal in the full parent corpus.  The state `(3^9)<=_w G_3` has no
split containing any of the four rigid `K=2` children.  A row of size three cannot make the part
four present in `B,C,D`, so the only candidate is `A=(1^9)`.  If `A` is mixed, the two pure
children would each need odd mass nine from remainders of size two; if `A` is pure, it occupies
all rows and legality leaves the opposite pure child empty.  Hence even one rigid child can fail.

The complete multiplicity census, its parent-first proof method, measured cost and low-layer
structure are in the
[multiplicity record](../../evidence/singleton_split_multiplicity_census_2026-08-29.md).  The
original `K<=3` corpus remains in the
[unique-split survey](../../evidence/singleton_unique_split_survey_2026-08-29.md).

### Tight-skeleton factorization and minimum-support reduction (2026-08-29)

The parent--solution relation has an exact Pascal product on tight dyadic faces.  Let
`h=G_(K-1)` with prefix function `H`, and suppose a parent's first `t` rows are tight:

    sum_(i<=t) a_i = H_K(t).

In any legal split, let `p` of those rows face the left pure child and `q=t-p` face the right.
Their child contributions have total mass at most

    H(p)+H(t)+H(q) <= H_K(t).

The parent equality forces equality throughout.  When `t=2^j` with `j>=1`, the Pascal profile has
a strict drop after `t/2`, so concavity makes `p=q=t/2` the unique maximizer up to exchanging the
pure sides.  The top rows therefore saturate child prefixes of sizes `t/2,t,t/2`; the remaining
rows fill the contracted suffix profiles.  Conversely, legal allocations of those disjoint prefix
and suffix problems concatenate.  The endpoint `t=1` similarly forces pure counts `0,1`.  Call
this the **Dyadic Tight-Prefix Factorization Lemma**.

At `K=4,t=4`, the head capacities are

    (8,7), (8,7,4,4), (8,7),

and the tail capacities are

    (4,4,1^4), (1^4), (4,4,1^4).

Thus the pure head shapes and mixed tail shape are fixed.  Child-shape choices form a literal
head--tail Cartesian product.  The low head layers have sizes `3,4,1` and the low tail layers
`7,19,6`; convolution gives exactly `21,85,25` full parents of multiplicity one, two and three.
This accounts for 131 of the 259 low-multiplicity `K=4` parents.  More generally, 228 of the 259
have some tight prefix at `t=1,2,4,8`.

The dyadic statement is the one-state case of a more general exact factorization.  Define

    I(t)=argmax_p (H(p)+H(t-p)).

At any tight parent prefix `t`, every legal allocation has some number `p_t in I(t)` of
left-oriented rows and saturates child prefixes `p_t,t,t-p_t`.  Hence consecutive tight ranks
`u<v` fill the contracted bands

    h[p_u:p_v],       h[u:v],       h[u-p_u:v-p_v].             (TS)

Conversely, a monotone path of such counts and legal allocations of all its bands concatenate to
a legal global allocation.  At the raw oriented-allocation level, the number of global solutions
is therefore a sum over count paths of products of local band multiplicities.  This is the
**Pascal Tight-Skeleton Factorization Theorem**.  The dyadic product above occurs when `I(t)` is a
singleton.

The count path is essential, but the proposed **Positive-Band Extension Conjecture is false**.
At `K=6`, the band `[15,32)` has canonical profile `(22,7^16)`, while the dominated equal-mass
refinement `(8^15,7^2)` extends neither endpoint transition `7->16` or `8->16`.  This is exactly
the band inside the global counterexample.  The later
[Tight-Band Capacity Obstruction](tight-band-capacity.md) proves this failure by two scalar
capacity inequalities and classifies the entire 176-state fixed face.

There is no longer a separate arbitrary-row tail conjecture.  The **Half-Unit Coalescence Lemma**
says that if a capacity partition has at least as many unit parts as non-unit parts, merging the two
smallest parts of any dominated refinement preserves dominance.  Every `G_K` and every suffix of
it has this property.  Repeatedly coalesce a full parent to exactly `2^K` rows; after splitting the
coalesced state, undo a merge by orienting both original rows alike and dividing the merged pure
amount between them.  This merely refines each child part, so child majorization is preserved.
Hence:

> **Minimum-Support Reduction Theorem.**  The Row-Coloring property for all full parents at level `K`
> follows from its restriction to parents with exactly `2^K` positive rows.

The terminal positive-band case still implies arbitrary suffix extension when its head allocation
exists.  The full-band case `[0,2^K]` is exactly the minimum-support problem; the `K=6`
counterexample shows that this full-band statement is false.
In that minimum-support case every row must send a positive pure part and the two orientations have
exactly `2^(K-1)` rows each.  Removing one pure anchor coin from every row contracts the three
child capacities to `(G_(K-1)-1),G_(K-1),(G_(K-1)-1)` and the parent capacity to `G_K-1`; this is
the exact anchored residual core.  The short conjugate-partition proof of coalescence and the rank
identity for this contraction are in the tight-skeleton record.

Positive-Band Extension passes a complete exact census through `K=4`: all 1,722,516 band-state
instances across 136 rank bands extend every incoming count.  The `K=4` full band contains only
408,776 exact-support parents, versus 5,997,038 unrestricted parents; 63,329 of the exact-support
states have no internal tight prefix.  The old direct tail census also passes all 1,422,304
state--incoming-count instances across 37 cases, independently checking the now-proved reduction.
These census results are finite lower-level theorems; the general statement fails at `K=6`.

The path variable cannot be normalized away.  In the `K=4` band `[5,9)`, the state `(4^4)` fails
the same-orientation transitions `2->4` and `3->5` but succeeds under the switching transitions
`2->5` and `3->4`.  Nor may prefix capacities be arbitrarily refined: the nine-row state
`(16,15,11,11,4^5)` is dominated by `(16,15,11,11,5^4)` but cannot fill the corresponding prefix
child capacities.  Even exact-head strict alternation fails at `K=5` on
`(32,31,26,26,16^3,11^2,6^7)`.  More strongly, strict alternation fails at `K=6` on the 64-row
parent `(63^2,57^2,42^3,23^5,22^5,3^44,2^3)`, even though it is strictly below every internal
prefix of `G_6`.  The new band hole goes further: even the fully adaptive global Pascal count path
can have no outgoing transition.

Exact proofs, counterexamples, census method and reproduction are in the
[tight-skeleton record](../../evidence/singleton_pascal_tight_skeleton_2026-08-29.md); the original
low-fiber product is in the
[factorization record](../../evidence/singleton_low_multiplicity_factorization_2026-08-29.md).

### Two-anchor Pascal residual reduction (2026-08-29)

The full exact-support band has a second deterministic contraction.  Put

    n=2^(K-1),       M=3^(K-1),       h=G_(K-1),
    c=(h_1-1,...,h_n-1),

retaining the trailing zeros of `c`, and let `C` be its saturated prefix function.  Then

    H(t)=C(t)+min(t,n).                                      (TA1)

Delete from `G_K'` its universal column of height `2n` and one column of height `n`.  The remaining
parent profile `J` has

    J'={2d,d : d in c'},
    Jprefix(t)=C(t)+max_p(C(p)+C(t-p)).                       (TA2)

Equivalently,

    J=(G_K[1:n]-2, G_K[n+1:2n]-1),

with zeros removed.  In Boolean-column labels this deletes exactly `empty` and one singleton
`{*}`; all other subsets remain.  For `K=4`, for example,

    c=(7,6,3,3,0^4),       J=(14,13,9,9,3^4).

> **Two-Anchor Reduction Theorem.**  Let
> `a=(a_1>=...>=a_(2n)>0)<=_w G_K` have full mass.  Then `a_n>=2`, and the sorted residual
>
>     b=sort(a_1-2,...,a_n-2,a_(n+1)-1,...,a_(2n)-1)          (TA3)
>
> is nonnegative, has at most `2n` positive rows and satisfies `b<=_w J`.  If `b` has a coloring
> `A/B`, with at most `n` positive rows per color, satisfying
>
>     B^A_p+B^B_q <= C(p)+C(p+q)+C(q)                        (TA4)
>
> for all `p,q`, then `a` has a legal first split into three `h`-majorized children.

*Proof.*  Realize `a` against the conjugate columns of `G_K` by Gale--Ryser.  The degree-`2n`
column meets every row.  Delete it.  Bipartite Havel--Hakimi allows a remaining maximum-degree
column of height `n` to meet the `n` largest residual row degrees.  Equal-height columns are
interchangeable, so label the selected one `{*}`.  Deleting it proves `a_n>=2` and leaves exactly
(TA3) against the columns (TA2), hence `b<=_w J`.

By the Fixed-Color Hall Lemma, (TA4) allocates `b` to three residual children majorized by `c`.
Pad both color classes with zero-residual rows until each contains exactly `n` original rows.  Add
one pure anchor to every row on its orientation side, and add one mixed anchor to each of the
original top `n` rows.  A pure child prefix increases from at most `C(t)` by `min(t,n)`.  The top
`t` entries of the mixed child also increase by at most `min(t,n)`, regardless of which residual
rows received those anchors.  Equation (TA1) therefore bounds all three lifted children by `h`.
Every row uses only its chosen pure side and the mixed side, so the lift is legal. ∎

This isolates one smaller sufficient target.

> **Balanced Residual Coloring Conjecture (false at `K=6`).**  Every full-mass `b<=_w J` with at most `2n`
> positive rows has a coloring satisfying (TA4), with at most `n` positive rows of each color.

The conjecture was a sufficient strengthening, not an equivalent reformulation.  It already fails
on the two-anchor image of the `K=6` hole:

    b=(62,61,55^2,40^4,20^7,6^15,5^2)
      <=_w (62,61,55^2,40^4,20^8,5^16)=J.

Its residual child profile is `c=(31,30,25^2,15^4,5^8)`, and no capped coloring satisfies (TA4).
The finite residual censuses below remain correct.

The complete residual census supports the conjecture through `K=4`: all 73 residual states at `K=3`
and all 160,492 at `K=4` pass, with no coloring failures.  The deterministic two-anchor images of
all 160 and 408,776 exact-support parents also pass.  A still stronger longest-half rule--exactly
the largest `n` rows use the mixed child and all three children have exact support--passes all
exact-support parents through `K=4` and the strict-interior `K=6` alternation counterexample, but
is also refuted by the same `K=6` state, since it would supply a legal first split.

No scalar selection rule explains the result.  Greedy balance fails on 403 residual `K=4` states.
Globally minimum mass difference, even after imposing both necessary color-row bounds, fails on
1,067 states.  For

    b=(14,13,9,5,4,4,4,2,2),

all three admissible difference-one partitions violate Hall by one; for example,
`(14,9,4,2)|(13,5,4,4,2)` fails at `(p,q)=(3,5)`.  The legal split
`(14,5,4,4)|(13,9,4,2,2)` is less balanced.  Coalescing the two smallest residual rows fails on
9,804 two-anchor images.  Nor is the residual ideal a fixed product below the color classes of
`J`: `(6,3,3,3)` is a counterexample already at `K=3`.

A sharper transfer pattern survives.  Every one of the 160,491 noncanonical residual `K=4` states
has an upward one-unit predecessor admitting a feasible coloring in which the transfer endpoints
share a color; the first admissible predecessor works except for 77 states, and the second works
for all 77.  This is not yet a path induction, because independently selected predecessor
colorings need not be compatible along the entire path.  It suggests an endpoint-rich residual
augmentation lemma rather than a fixed assignment rule.  Proof details, exact counts, sampled
higher-level checks, counterexamples and commands are in the
[two-anchor residual record](../../evidence/singleton_two_anchor_residual_2026-08-29.md).

The residual finite pattern is a pure-anchor filtration.  Of the 259 low parents, 258 admit a
solution with a child-unique pure child.  The exception `(16,15,9^3,5,3^4,1^6)` has a two-orbit
pure child.  Every low parent also has an economical deformation from a closest rigid ancestor:
some resulting child triple is componentwise below the ancestor's forced triple, with total child
transfer distance no larger than the parent distance.  This accounts for 592 of 594 solution
orbits.  However only 176 parents are reachable from the rigid corpus by unit transfers while
remaining inside the low layer.  The other 83 require intermediate high-multiplicity states, so
the boundary is not a transfer-closed proof domain.

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

### Complete `K<=4` census and a refuted forward rule

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

The census suggested a more flexible **Block-Extension rule**.  Write the distinct row
values as `v_1>...>v_s`, with multiplicities `m_j`, and process these value blocks in descending
order.  For each block choose, among the allocations that preserve all currently exposed
inequalities (C) and permit at least one legal allocation of the next lower block, the allocation
that minimizes the current A/B total-mass difference.  Fix complementation by putting a largest
row in A and favor A on an exact tie.  This is a forward pass with one-block lookahead and no
recoloring.  It succeeds on the complete `K<=4` census, on 10,000,000 uniformly sampled states
from the exact 38,378,683,542,323-state full-mass `K=5` universe, and on a separate 100,000-state
`K=6` dominance-transfer sample.  The exact `K=6` hole has no coloring at all and therefore
refutes the universal rule; the sampled successes remain only lower-level diagnostics.

A later shell mode makes one bounded higher-level statement exhaustive.  It enumerates every
full-mass exact-support parent through transfer distance 14 at `K=5` and distance 13 at `K=6`,
using one-block lookahead only as a sound positive filter and exact normalized Hall search on every
failure.  All 311,082,023 `K=5` parents and all 5,189,450,419 `K=6` parents have a majorized first
cut.  Together with the distance-14 hole, the second result proves global minimum transfer distance
within the exact-support no-first-cut problem.  It remains a first-cut theorem at `K=6`, not a
recursive solvability census; complete counts, sharding checks and provenance are in the
[transfer-shell record](../../evidence/singleton_transfer_shell_census_2026-08-31.md).

The earlier higher-level samples are not proofs.  Simpler variants already fail: plain balanced
blocks miss 22 `K=4` states and 12 of the ten million `K=5` samples; reserving enough final rows
for both colors fixes all `K=4` cases but misses six of those `K=5` samples.  The first such `K=5` failure
needs the width-6 choice to anticipate the following width-5 block.  The exact counts,
counterexamples, commands and proposed two-block proof obligation are in
[the census record](../../evidence/singleton_row_coloring_census_2026-08-26.md).

Grouping equal rows here is only the normalization of row colorings by permutations of
indistinguishable rows.  It does not establish that equality has a special theorem unavailable to
nearby widths.  The formerly proposed one-block-extension theorem is false, since the `K=6` state
has no coloring at all.

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

## Equivalent graph and symmetric-function formulation

The canonical recursive strategy colors all of `Q_K` with color-class partition `G_K`, while the
necessity theorem shows that `G_K` dominates every other coloring type.  Therefore the converse was
equivalent to `Q_K` being **nice** in Stanley's sense: whenever a coloring type `lambda` occurs,
every partition dominated by `lambda` also occurs.  Equivalently, the support of the chromatic
symmetric function of `Q_K`, in sufficiently many variables, must contain every lattice point of
the permutahedron of `G_K`.  The terminology and this support interpretation are summarized in
[Matherne--Morales--Selover, Section 2.6](https://arxiv.org/html/2201.07333v4#S2.SS6).

The `K=6` counterexample is a dominated coloring type absent from `Q_6`, so `Q_6` is not nice.
The reformulation remains useful terminology for the negative result.

It is enough to prove the full-mass case.  If `a` has mass `M<3^K`, append `3^K-M` artificial unit
rows.  The completed partition remains majorized by `G_K`: after the original rows, its prefixes
increase by one, while every remaining nonzero entry of `G_K` is at least one.  A coloring of the
completed state restricts to one of the original state after deleting the artificial rows.

## Unconditional canonical consequence

The state `G_K` itself has Aigner's explicit recursive strategy.  Therefore any singleton state
whose rows form a sub-multiset of `G_K` is solvable by Subgraph Monotonicity.  More generally, the
same is true when the sorted row widths fit coordinatewise into distinct `G_K` rows.  These
`[canonical U_K]` and `[embedded G_K]` witness terminals remain unconditional.  An arbitrary
`[majorized G_K]` terminal is not a valid certificate: the universal rule that once justified it
is false, though any particular terminal may still have a separate strategy.

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
`make_u_freq` computes in `radio_canon_search_generic.c:121`.  It makes the proved necessary test
explicit; the `K=6` counterexample shows that the same prefix inequalities are not sufficient.

## Consequence used throughout this project

If the parts of a singleton state form a **sub-multiset** of `G_k`, delete the unused star rows
from Aigner's explicit strategy for `G_k`.  The state is therefore solvable by Subgraph
Monotonicity, without using any converse.  The same deletion argument works when the sorted
parts fit coordinatewise into distinct `G_k` rows.  A witness tree all of whose leaves have one of
these two forms is a complete proof independent of the solver.  `tools/check_witness.py` checks
these forms separately; it marks an arbitrary weak-majorization terminal as unsupported because
weak majorization alone is now known not to be a certificate.

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
converse refuted above.  The false Three-Way Decomposition Lemma also prevents replacing
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
