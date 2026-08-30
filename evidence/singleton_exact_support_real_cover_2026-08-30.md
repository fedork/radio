# Real Hall-cover counterexamples and the primitive-lattice gap (2026-08-30)

## Question separated into two genuinely different real problems

For `h=G_(K-1)`, let `H` be its saturated prefix function and, for a padded
row coloring `E=A disjoint-union B`, put

    f_A(S)=H(|S|)+H(|S intersection A|)+H(|S intersection B|).

The integer bases of `f_A` are exactly the full-mass demands feasible for that
fixed coloring.  Their union has the correct convex hull, but the union of the
corresponding **real** base polytopes need not be convex.

Already at `K=2`, with `h=(2,1)`, the real state

    x=(4,3,2/3,2/3,2/3) <=_w G_2=(4,3,1,1)                 (ER1)

has no Hall coloring.  Full mass forces at least two rows of each color.  By
symmetry put the row four in `A`.  The row three cannot also be in `A`, since
`7>2H(2)=6`.  If `A` has two rows, the two sides are

    A=(4,2/3),             B=(3,2/3,2/3),

and `(p,q)=(1,3)` has demand `25/3>8`.  If `A` has three rows, the two sides
are

    A=(4,2/3,2/3),         B=(3,2/3),

and `(p,q)=(3,1)` has the same demand and capacity.  This proves (ER1)
without computation.

Scaling by three turns (ER1) into the integer hole

    (12,9,2,2,2) <=_w (12,9,3,3)

for the non-Pascal child profile `3G_1=(6,3)`.  Thus the desired property is
not homogeneous.  A proof for primitive `G_r` must use its unit lattice, not
only the ratios of its Pascal entries.

## Exact support does not repair real coverage

The counterexample has five positive rows, while `G_2` has four.  Put

    m=2^(K-1),       |E|=2m,       M=3^(K-1).

For `|A|=|E-A|=m`, let `B(f_A)` denote the real base polytope.  Every one of
its coordinates is at least one, because

    f_A(E)-f_A(E-i)=3M-(M+(M-1)+M)=1.

It is contained in the permutahedron of `G_K` by the balanced-rank identity.
Conversely, the union over all balanced `A` contains every permutation of
`G_K`, by permuting the canonical cut.  This initially suggested the following
clean sufficient strengthening.

> **Failed Exact-Support Real Cover statement.** For `h=G_(K-1)` on `2^K` labelled
> rows,
>
>     union_(|A|=m) B(f_A) = Perm(G_K).                       (ER2)

Equivalently, the displayed union would be convex.  If (ER2) held, every integral
exact-support parent has a Hall coloring; the integral matching theorem then
gives an integral cut.  The proved Minimum-Support Reduction coalesces any
full-mass integral parent to exactly `2^K` positive rows and lifts its cut.
Hence (ER2) at every level would prove the full Singleton Majorization Converse.

The restriction to exact support is essential to this implication.  The
integer coalescence proof uses the unit tail of `G_K`; it has no homogeneous
real analogue, which is exactly why (ER1) can occur.

At `K=2`, (ER2) is a theorem.  Write

    a=(a1>=a2>=a3>=a4)>0,       a<=_w(4,3,1,1),       sum a_i=9.

Color `{a1,a4}` against `{a2,a3}`.  The one-row inequalities follow from
`a1<=4`; the two pure inequalities are `a1+a4<=6` and `a2+a3<=6`.  The first
follows because its failure would give `a4>2` and then total mass greater than
ten; the second follows from `a1>=9/4` and the tail inequality `a4>=1`.  The
`(1,1)` inequality is `a1+a2<=7`, the two three-row inequalities follow from
`a3,a4>=1`, and the full inequality is equality at nine.  These are all Hall
pairs for `h=(2,1)`.

The statement fails at `K=3`.  The smallest grid counterexample found is

    a=(8,7,4,(8/5)^5) <=_w G_3=(8,7,4,4,1,1,1,1).             (ER4)

Every full-mass coloring has four rows on each side.  Among the first three
rows, one side contains two and the other contains one.  Take the whole side
containing two head rows together with the remaining head row on the other
side.  Its demand is

    8+7+4+2(8/5)=111/5,

whereas its `(4,1)` Hall capacity is

    H(5)+H(4)+H(1)=9+9+4=22.

Thus every coloring has excess at least `1/5`.  Coloring
`A=(8,4,8/5,8/5)`, `B=(7,8/5,8/5,8/5)` shows that the minimum possible
maximum excess is exactly `1/5`.

There is an all-level family, so this is not a low-dimensional accident.  Put

    m=2^(K-1),     M=3^(K-1),     g=G_K,
    t=1+K/(m+1),
    a=(g_1,...,g_(m-1),t^(m+1)).                              (ER5)

The last `m+1` coordinates in (ER5) are the average of the corresponding tail
of `g`; hence (ER5) is a positive exact-support point majorized by `g`.  To
verify the displayed average, let `h=G_(K-1)`.  Its last `m/2` entries are one,
`h_(m/2)=K`, and therefore

    H_K(m-1)=H(m-1)+H(m/2)+H(m/2-1)=3M-m-K-1.

The remaining mass is `m+K+1` on `m+1` rows, giving `t`.

In any balanced coloring one side contains `u>=m/2` of the first `m-1` rows;
put `v=m-1-u<=m/2-1`.  Select that whole side and the `v` head rows on the
other side.  Its excess over the `(m,v)` Hall capacity is

    E(v)=H_K(m-1)+(v+1)t-2M-H(v).

For `v<m/2-1`, moving to `v+1` changes the excess by
`t-h_(v+1)<0`, since `h_(v+1)>=h_(m/2)=K>t`.  Its minimum is consequently
at `v=m/2-1`, where

    E_min=(m/2)(t-1)-1=((K-2)m-2)/(2(m+1)).                   (ER6)

This is positive for every `K>=3`.  It already equals `23/17>1` at `K=5`.
Therefore neither exact support nor a universal additive-defect-below-one
version can rescue the real-cover strategy.

Multiplying (ER5) by `m+1` gives an integer hole for the *scaled* child profile
`(m+1)G_(K-1)`.  It does not refute the primitive integer Singleton
Majorization Converse; it proves that any proof of that converse must use the
primitive lattice essentially.

## The primitive lattice repairs the final tight band

The same family contains a small positive integer theorem.  The final `m+1`
rows of `G_K` are

    (K+1,1^m).                                                 (ER7)

At the tight rank `m-1`, concavity gives exactly the two orientation counts

    I(m-1)={m/2-1,m/2}.

Thus the head-light side of any legal head-band allocation has `m/2+1` tail
slots, while the other side has `m/2`.

> **Integral Final-Band Extension Lemma.** Suppose an exact-support integral
> parent is tight after its first `m-1` rows.  Every legal allocation of that
> head band extends across its final `m+1` rows.

Indeed, after placing one coin in every tail row there are exactly `K` coins
left.  Consequently at most `K` tail rows are non-unit, and
`K<=m/2+1`.  Put all non-unit tail rows, plus enough unit rows, on the
head-light side.  Tight-skeleton factorization concatenates the given head
allocation with the canonical tail (ER7), oriented so `K+1` lies on that
side.  Now move its `K` excess coins to the desired non-unit rows without
changing colors.  Each move is a Robin--Hood transfer within one color.  For
every fixed pair of color counts, the maximum selected demand can only
decrease, so all fixed-color Hall inequalities remain valid.

In particular, keeping the first `m-1` rows of `G_K` fixed and replacing
(ER7) by any `m+1` positive integral rows of the same mass always gives a
colorable parent.  This is the **Integral Tail-Unsmoothing Corollary**.

For example, at `K=5` the fractional bad tail is `(22/17)^17`, whereas its
most even integral replacement is `(2^5,1^12)`.  All five rows of size two
fit on the head-light side.  Equivalently, transform the canonical tail
`(6,1^16)` by four same-color transfers.  Direct enumeration confirms this
construction through `K=6`, but the preceding argument is all-level.

The lemma does not produce the required legal allocation of an arbitrary head
band and therefore does not prove Row-Coloring.  It does show that on this
tight face the tail is never the integer obstruction.  The open work is to
extend the incoming orientation through earlier Pascal bands; the real
relaxation loses exactly the finite-support fact that makes the final band
automatic.

## Exact rational-grid evidence

`tools/singleton_solution_fiber_dag.py` now has two exact scaling modes.  A
denominator-`d` point is stored as an integer partition under `dG_K`, while
the Hall profile is `dG_(K-1)`.  Dividing all inequalities by `d` makes this
an exact rational test of the real union, not an appeal to the open integer
converse.

The padded command

    python3 tools/singleton_solution_fiber_dag.py --rational-grid 2 5

returns

    denominator  states  missing  first missing
              1      15        0  none
              2     206        0  none
              3   1,319        2  (12,9,2,2,2)
              4   5,803        6  (16,12,3,3,2)
              5  19,891       23  (20,15,4,4,2)

and takes 1.14 wall seconds on the recorded local machine.  This independently
reproduces (ER1).

The exact-support commands

    python3 tools/singleton_solution_fiber_dag.py --exact-rational-grid 2 6
    python3 tools/singleton_solution_fiber_dag.py --exact-rational-grid 3 5 \
      --list-rational-holes

return no missing coloring through the first listed grids and then expose
(ER4):

    K=2: 4, 16, 38, 77, 132, 214 states at denominators 1,...,6;
    K=3: 160, 3,997, 34,704, 179,482 states and no holes at denominators 1,...,4;
         675,341 states, 56 holes, first (40,35,20,8,8,8,8,8), at denominator 5.

The `K=2` run takes 0.08 wall seconds.  The `K=3` run through denominator four
takes 15.19 wall seconds, and the final run through denominator five takes
64.65 wall seconds on the recorded local machine.  Every one of the 56 new
holes has the form

    (x,y,z,8,8,8,8,8),      x+y+z=95,

and has minimum scaled Hall defect one.  This is exactly the normalized
`1/5`-defect face described by (ER4), not 56 unrelated exceptions.

## A fractional-coloring polytope

Let `bar H` be the piecewise-linear interpolation of `H`.  For a fixed
exact-support parent `a`, define

    Y(a)={y in [0,1]^E : y(E)=m and
          a(S)<=r_y(S) for every S subset E},

where

    r_y(S)=bar H(|S|)+bar H(y(S))+bar H(|S|-y(S)).             (ER3)

An integral `y` is precisely a Hall coloring.  The all-half point belongs to
`Y(a)`, because (ER3) then becomes the parent majorization rank.  Moreover,
`Y(a)` is convex: for fixed `S`, the last two terms are a concave symmetric
function of the single linear form `y(S)`, and a superlevel set of a concave
function is convex.  Thus the failed statement (ER2) was equivalently the
assertion that every one of these nonempty Pascal rounding sets contains a
zero--one point.  State (ER4) shows directly that this assertion is false for
real `a`.

There is a useful uncrossing fact.  For fixed `y`, `r_y` is submodular: each
term in (ER3) is a concave function of a nonnegative modular weight.  If `X`
and `Z` are active (`a(X)=r_y(X)` and `a(Z)=r_y(Z)`), modularity of `a`,
submodularity of `r_y`, and feasibility give equality throughout

    r_y(X)+r_y(Z) >= r_y(X union Z)+r_y(X intersection Z)
                    >= a(X union Z)+a(X intersection Z)
                     =a(X)+a(Z).

Hence `X union Z` and `X intersection Z` are active as well.  This uncrossing
fact remains valid, but (ER4)--(ER6) prove that it cannot force real rounding.
It can only be useful in an argument that additionally exploits integral
parent demands.

The most immediate generalized-polymatroid shortcut is false.  If `L(S)` is
the least integral value allowed for `y(S)`, then `L` need not be
supermodular.  For the canonical `G_3` parent, the set of rows `{8,7}` has
`L=1`, a unit row has `L=0`, but `{8,7,1}` has `L=0`: the added unit permits
that three-row set to be monochromatic at exact capacity sixteen.  Thus
`1+0>0+0`.  The proof must use the active-set lattice in (ER3), not declare
the raw interval endpoints paramodular.

The remaining sharp question is the primitive integer one: when `a` itself
is integral and majorized by `G_K`, must `Y(a)` contain a zero--one point?
That is just the exact-support Row-Coloring Lemma in an integer-aware
rounding form.  The real counterexamples show why a proof cannot discard the
integrality of `a` while rounding `y`.  Active-set uncrossing identifies
structure an integer argument may use, but it is not itself an escape lemma.

Finally, ordinary Lorentzianity of the full chromatic symmetric function is
not this escape route.  `Q_3` contains an induced claw, while the support
conditions for a Lorentzian chromatic symmetric function force the graph to
be claw-free; see the conclusion of
[Borowiecki--Mészáros, *Symmetric Lorentzian Polynomials*](https://borowiecki.dev/pdf/2510.07819).
The exact-support slice could still have a separate discrete-convex
description, but the full `X_(Q_K)` cannot supply it through Lorentzianity.
