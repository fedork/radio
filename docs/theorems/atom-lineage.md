# Atom-lineage obstruction for aligned profile constructions

This note concerns the explicitly restricted, non-wasteful aligned profile model used by
`tools/search_atom_profiles.cpp`.  It is not an unrestricted group-testing impossibility theorem.
Within that model it replaces bounded-depth evidence by an all-depth symbolic obstruction.

## Model

Fix a power-of-two normalization size `N`.  A width profile

    p = (p_A,p_B,p_C,p_D),        |p|=N,

means `p_A A_r + p_B B_r + p_C C_r + p_D D_r`.  One synchronized test refines

    R(p) = (2p_A+p_B, p_B+p_C, p_C+p_D, p_D)

and selects an `N`-atom subprofile `x <= R(p)`; the complementary profile is `R(p)-x`.  A state
part `(p:h)` uses the usual height cut `y`, so its mixed child contains `(x:h-y)` and
`(R(p)-x:y)`, omitting zero-height parts.

For `N=2^s`, the singleton reference consists of `A,B,C,C,D,D` refined `s` times.  Its first six
profiles contain respectively

    0, 0, 0, 0, 1, 1

copies of `D`.

## D-lineage lemma

For a profile state `S`, define the *unweighted* D-lineage count

    L_D(S) = sum over parts p_D.

Do not multiply `p_D` by the part height.  Then every mixed child `S_1` satisfies

    height(S_1) = height(S),        L_D(S_1) <= L_D(S).              (1)

Indeed, `R(p)_D=p_D`.  The selected and complementary profiles partition those `p_D` atoms.  If
both mixed pieces have positive height their D counts add back to `p_D`; if one has zero height,
some lineages can disappear but none can be created.  Summing over parts proves (1).

An eventual singleton leaf of height `h` needs at least

    max(0,h-4)                                                       (2)

D lineages.  This is just the leading, `binom(r,2)`, coefficient of full-star majorization at the
complete prefix of length `h`.

Now follow the mixed outcome at every node of any proposed finite strategy.  Its height never
changes.  Equations (1) and (2) prove:

> **D-lineage obstruction.** An aligned profile state `S` is impossible at every synchronized
> depth if `L_D(S) < max(0,height(S)-4)`.

Equivalently, these states form a closed losing set: for every legal test, nature can choose the
mixed child and remain in the set.  This is the finite coinductive certificate that the earlier
bounded-depth search was missing.

## Finite-depth mixed-supply lemma

The same argument retains all three deficit coordinates.  Define the unweighted supply

    Sigma(S) = sum over parts (p_D, p_C+p_D, p_B+p_C+p_D) = (D,V,W).

Again, do not multiply a part by its height.  Refinement acts by the triangular map

    T(D,V,W) = (D,V+D,W+V).

Selected and complementary profiles partition `T(p)`.  A zero-height mixed piece can delete
supply but cannot add it, so every mixed child satisfies `Sigma(S_1) <= T(Sigma(S))`
componentwise.  After following `t` mixed outcomes,

    Sigma(S_t) <= (D, V+tD, W+tV+binom(t,2)D).                    (2a)

Let `Q_h` be the sum of the deficit triples of the first `h` singleton reference profiles.  A
singleton leaf of height `h` must have total deficit lexicographically at least `Q_h`.  Therefore:

> **Mixed-supply bound.** If the right side of (2a) is lexicographically smaller than `Q_h`, a
> height-`h` state cannot finish within `t` more synchronized tests.

This is a finite-depth obstruction.  Its leading-coordinate failure is independent of `t` and is
exactly the D-lineage theorem; lower-coordinate failures can disappear when more depth is allowed.
The compiled search uses this bound before recursion, and the independent lineage checker verifies
the local triangular inequality on every enumerated cut at the 8- and 16-atom controls.

The same calculation gives a stronger *transition* constraint.  If the first mixed child loses

    ell = T(Sigma(S)) - Sigma(S_1) = (ell_D,ell_V,ell_W),

then `ell` is nonnegative and additive over the state parts.  Propagating the child optimistically
for the remaining `t-1` tests gives

    Sigma(S_t) <= T^t(Sigma(S)) - T^(t-1)(ell),                  (2b)

where

    T^(t-1)(ell) =
      (ell_D,
       ell_V+(t-1)ell_D,
       ell_W+(t-1)ell_V+binom(t-1,2)ell_D).

Consequently a partial global cut can be discarded as soon as the right side of (2b) falls below
`Q_h`: unchosen parts can incur zero further loss but cannot compensate for loss already incurred.
This is the **propagated-loss bound**.  In particular, equality with `Q_h` in the leading `j`
coordinates forces the corresponding propagated losses to vanish.  The C++ recursion applies
this lexicographic test both to each local cut and after every added state part; it is a necessary
condition only and does not turn a bounded negative into an all-depth result.

There is also a height-sensitive version of (2a).  Preserve the ancestry of each state part
`p:h` while following `j` mixed outcomes.  To finish as `h` singleton lineages requires
`h<=2^j`, because one test can at most double the number of descendants.  Moreover, each terminal
singleton profile contains `N` atoms, so none of its three deficit coordinates exceeds `N`.
Consequently the total supply descending from that part is bounded componentwise by

    (p_D,
     min(hN, p_C+p_D+j p_D),
     min(hN, p_B+p_C+p_D+j(p_C+p_D)+binom(j,2)p_D)).             (2c)

Sum (2c) over the original parts and compare it lexicographically with `Q_h`.  A state that can
finish within `t` tests must pass this comparison for at least one `j` in `0..t`.  This remains an
optimistic necessary bound: it ignores whether the capped supplies can actually be partitioned
among the `h` leaves.  The independent Python implementation recomputes it rather than consuming
the C++ result.

## Height-6 consequence

For the `(alpha,beta,gamma)=(4,3,2)` hard branch, the two fixed profiles have no D atoms:

    (A^5 B^2 C:1), (A^3 B^3 C^2:2), (D-germ:3).                    (3)

Thus every D germ with `p_D<=1` is impossible at all depths.  At eight atoms these are exactly
ranks 1 through 81 in eventual deficit order.  This includes the finite-width accounting
`ABBBBBCD` at rank 59 and the old `AAAABBCD` at rank 56.  Increasing the depth or merely refining
either one cannot repair it: refinement preserves its single D lineage.

The first survivor is rank 82, `A^6D^2`.  The exact aligned recursion finds a three-level tree for

    Sb(A^5B^2C:1, A^3B^3C^2:2, A^6D^2:3),                          (4)

and the independently implemented checker verifies all 19 nodes.  The largest terminal threshold
is root base `r>=12`.  Consequently (4) is the widest D germ among all 165 A--D eight-atom profiles,
with no synchronized-depth qualifier: every wider germ has the all-depth obstruction above.

Attaching the already constructed outer A/B/C branches in the working four-segment assembly gives
the conditional parent profile

    A^21 B^6 C^3 D^2 @ G_(k-5),

whose width is

    2^k - 2 binom(k-5,2) - 5(k-5) - 11
      = 2^k - k^2 + 6k - 16,                                      (5)

valid from the symbolic hard-branch threshold `k>=17`.  Equation (5) is an aligned-family lower
construction, not a global Pareto maximum over other height triples or normalization sizes.

## Sixteen-atom two-coordinate kernel

The 165 profiles above are all A--D profiles of length eight, not all excessive-q profiles.  At
length 16 there are 969 A--D profiles.  The same lineage theorem excludes ranks 1 through 289; the
refined `ABBBBBCD` class is rank 191, and the first profile not decided by D count alone is rank 290,
`A^14D^2`.

The two-coordinate projection

    pi(p) = (p_D,p_C+p_D)

is a sound over-approximation: it retains the first two eventual deficit coefficients, permits
every projected aligned cut, and deliberately drops the final coefficient at terminal comparison.
Writing a projected part as `(u,v:h)`, refinement sends its total profile to `(u,u+v)`.  Thus the
projected local cut algebra is closed and finite.

Ranks 290 through 304 all have `p_D=2,p_C=0`; their differing A/B counts disappear under `pi`.
Together with the two fixed branches, every one has the same projected root

    S_290 = ((0,1):1), ((0,2):2), ((2,2):3).                      (6)

A single scalar lineage count does not close (6): a cut can trade a projected C gain in the mixed
child against a prefix failure in a pure child.  The exact invariant is instead a finite
coinductive kernel.  Let `I` contain every projected state that already fails D lineage or
two-coordinate full-star majorization, and let `K` be the 242 minimal states retained in
`evidence/atom_profile_height6_dc16.cert`.  The independently checked property is

    for every S in K and every legal synchronized cut of S,
    some outcome contains a state in K or I as a substate.          (7)

The upward closure is important: adjoining more branches cannot make an unsolvable restricted
substate solvable.  Hence (7) makes the upward closure of `K union I` a closed losing set.  Since
`S_290` is in `K`, it is impossible at every synchronized depth.  The projected model is an
over-approximation, so this excludes the full A--D profiles at ranks 290--304 as well.  The kernel
was discovered as a repeated pair of bounded-search layers, but the proof no longer depends on
that depth: the checker exhausts the transition closure directly.

The omitted coordinate also has a closed exact lift rule.  Let the parent profile be
`p=(a,b,c,d)`, let a projected cut select `(u,v)=(x_D,x_C+x_D)`, and write `j=x_B` for its only
remaining freedom.  Then

    x=(N-v-j, j, v-u, u),
    max(0, N-v-(2a+b)) <= j <= min(N-v, b+c).                       (8)

The projected legality conditions already handle C and D; (8) is exactly the remaining A/B
containment in `R(p)`.  Thus a projected skeleton lifts by choosing one integer from this interval
for each part at each node.  Every choice forces the complement and all three exact children, so
the problem is a finite recursive constraint system rather than an unstructured profile search.

Rank 305, `A^13CD^2`, adds exactly one C atom and changes only the last root part to `((2,3):3)`.
It lies outside the losing kernel.  A separately retained 25-node projected tree solves it at
depth three, so the `(D,C+D)` abstraction is sharp at this boundary.  This positive is only an
abstraction witness: it does not assign the dropped B/C/D deficit coefficient.  That particular
skeleton does **not** lift through the omitted coordinate; `tools/check_dc_tree_lift.py` exhausts
every legal hidden-coordinate assignment on it.  This is not an obstruction, because a projected
state can have several inequivalent winning splits.

Enumerating those projected splits lazily and lifting each one immediately finds a different exact
tree.  Its root is

    (A^12B^3C:1), (A^9B^5C^2:2), (A^13CD^2:3),

and its first selected profiles are respectively

    (A^16:1), (A^8B^6C^2:0), (A^14C^2:2).                         (9)

The independently checked proof object has 19 nodes and all leaves satisfy exact three-coordinate
majorization for root base `r>=6`.  Since ranks 1--304 already have all-depth exclusions, rank 305
is therefore the exact widest sixteen-atom D germ, again with no synchronized-depth qualifier.

At this normalization the already constructed outer quantities are

    c=A^15B,   a-c=A^9B^5C^2,   b=A^12B^3C,

so attaching the new germ gives the conditional parent profile

    A^49B^9C^4D^2 @ G_(k-6)
      = R^2(A^7B^7D^2) @ G_(k-6).

Its width is

    2^k - 2 binom(k-6,2) - 6(k-6) - 15
      = 2^k - k^2 + 7k - 21,                                    (10)

and the hard-tree threshold `r>=6` gives `k>=12`.  As in (5), this is conditional on the working
outer assembly and is a lower construction inside the aligned height triple, not a global Pareto
maximality theorem.

So the symbolic result is sharp but scoped:

- the one-D proposal is refuted at *all* depths and under every pure refinement;
- the eight- and sixteen-atom A--D optima are proved exactly;
- the new 16-atom ranks 290--304 are also all-depth negative;
- the 32-atom slice is reduced to one unresolved profile, but larger normalization sizes remain
  open.

## Thirty-two-atom boundary kernel

At length 32 there are 6,545 A--D profiles.  D lineage excludes ranks 1--1089.  The next three
complete projection bands all have two D atoms:

    ranks 1090--1120: p_C=0, root ((0,1):1),((0,2):2),((2,2):3),
    ranks 1121--1150: p_C=1, root ((0,1):1),((0,2):2),((2,3):3),
    ranks 1151--1179: p_C=2, root ((0,1):1),((0,2):2),((2,4):3).  (11)

A projected depth-32 search exposed identical false layers at depths three and four.  Minimizing
their viable states under multiset inclusion gives the 504-core kernel retained in
`evidence/atom_profile_height6_dc32.cert`.  The independently checked closure property is again
(7), now for all 504 cores.  Its upward closure contains every root in (11), so the projection
excludes ranks 1090--1179 at every depth.  Together with D lineage, every rank through 1179 is
therefore impossible in the exact aligned model.

The next profile is rank 1180, `A^27C^3D^2`, whose projected last part is `((2,5):3)` and is not
covered by this kernel.  Rank 1181, `A^26BC^3D^2`, is the pure refinement
`R(A^13CD^2)` and hence inherits the exact rank-305 construction.  Consequently the exact
32-atom optimum has been narrowed to a one-rank alternative: rank 1180 if that state is
constructible, otherwise rank 1181.

The propagated-loss bound makes the rank-1180 depth-three product finite enough to exhaust.  Its
root has exact leading-coordinate tightness, so every first mixed transition must lose no D or V
supply and at most four W supply.  The complete aligned product then returns `NO` in 6.7 solver
seconds in the measured run; `tools/atom_profile_regression.sh` reproduces the exhaustive bounded
verdict.  The separately implemented Python all-skeleton search independently reaches the same
`NO` by enumerating every winning two-coordinate skeleton and every exact hidden-coordinate lift.
Thus no rank-1180 aligned tree has depth at most three.  This is not an all-depth exclusion.  Exact
depth-four runs using both complete-product and outer-prefix order reached their explicit ten-minute
CPU caps without a verdict.

A finite root-frontier calculation now replaces those undirected depth-four attempts.  Sound
symbolic and projected filters leave 7,266 oriented first tests.  Solving both pure-outcome
children exactly leaves 6,712 tests and 1,826 distinct mixed children.  The only positive-`V`-loss
classes are `(ell_D,ell_V,ell_W)=(0,2,10),(0,2,11),(0,2,12)`, comprising 16 tests and eight distinct
mixed children.  Exact depth-three recursion proves all eight children negative, independently in
the C++ and Python implementations.  Therefore every possible depth-four tree must satisfy

    ell_D=ell_V=0,       1<=ell_W<=14.                            (11a)

The remaining exact frontier has 6,696 oriented first tests and 1,818 distinct mixed children in
fourteen scalar W-loss classes.  This is a bounded reduction, not a depth-four verdict: rank 1180,
the 32-atom optimum, and eventual constructibility all remain open.

## Scalar form of the remaining D optimization

The outer A/B/C branches can now be eliminated symbolically.  Put `N=2^s`, with `s>=3`.  D
lineage requires at least two D atoms, while any larger D count loses in the leading eventual
coefficient, so write the only potentially optimal D germ as

    D_(s;b,c) = A^(N-b-c-2) B^b C^c D^2.

At the same normalization the three already constructed outer profiles are

    A^(N-1)B,
    A^(N-2s+1) B^(2s-3) C^2,
    A^(N-s) B^(s-1) C.

Thus attaching `D_(s;b,c)` gives the parent profile

    A^(4N-3s-b-c-2) B^(3s+b-3) C^(c+3) D^2 @ G_(k-s-2),        (12)

and purely algebraic evaluation gives the width germ

    2^k - k^2 + (2s-c)k - s^2 - 3s + c(s+1) - b + 2.           (13)

No witness internals occur in (12)--(13).  For a fixed normalization and `k>s+1`, eventual width
order says to minimize `c` first and then `b`; constructibility of that two-parameter D state is
the only remaining synchronized obligation.

For the height-6 root, its unweighted supply and the six-item terminal requirement are

    Sigma(S) = (2, c+5, 3s+b+c+1),
    Q_6      = (2, 2s+4, s^2+3s+5).                              (14)

At depth three, (2a) and (14) force `c>=2s-7`.  If equality holds, the last coordinate further
requires `b>=s^2-8s+11`.  These are necessary depth-three bounds, not all-depth bounds.  At
`s=5`, the first one forces `c>=3`, agreeing with the stronger all-depth kernel; the second places
no restriction on `b`.  The exact product search nevertheless excludes the boundary `b=0,c=3`
through depth three.  The exact 32-atom all-depth question is consequently still the single step
`b=1 -> 0`:

    b=1: A^26BC^3D^2, constructible by refinement, parent width
         2^k-k^2+7k-21;
    b=0: A^27C^3D^2, unresolved, formal parent
         A^108B^12C^6D^2 @ G_(k-7), width 2^k-k^2+7k-20.         (15)

The working sufficiently-large-`q` postulate licenses this eventual comparison, but does not imply
the second line of (15) is constructible.  It is the unique wider candidate left at `s=5`.  At
larger `s`, neither `c=2s-7` nor `b=0` may be assumed without another all-depth
exclusion/positive argument.  Equation (13), rather than a guessed word, is the scalable objective
for those slices.

At this boundary the two finite-depth supply budgets are especially concrete:

    Sigma(S_1180) = (2,8,19),       Q_6 = (2,14,45),
    T^3 Sigma      = (2,14,49),     T^4 Sigma = (2,16,63).        (16)

For a first-transition loss `ell`, depth three therefore requires
`ell_D=ell_V=0, ell_W<=4`.  The raw depth-four supply budget permits `ell_D=0, ell_V<=2`; if the
full two units of V slack are spent, (2b) further gives `ell_W+3ell_V<=18`, hence `ell_W<=12`.
Exact solution of the two pure children then removes every `ell_V>0` case and the `ell_W=0` case,
leaving precisely the necessary frontier (11a).  These calculations use only the outer profiles
and the candidate D germ, not the inner witness trees of A/B/C.  The 1,818 mixed children are the
finite symbolic interface for the next D-branch calculation.

## Mechanical verification

`tools/atom_profile_regression.sh` builds both the 8- and 16-atom variants and also checks the
retained 32-atom projected certificate.  It invokes
`tools/check_atom_profile_certificate.py`, which independently enumerates all local refinement/cut
transitions (174,069 cases at eight atoms and 5,540,319 at sixteen), and
`tools/check_atom_profile_tree.py`, which independently re-derives every state, split, terminal
majorization inequality, and threshold in the positive trees.  Neither checker shares code with
the C++ search.  The combined durable eight-atom proof object is
`evidence/atom_profile_height6_ad8.cert`.

For the two-coordinate results, `tools/check_dc_kernel_certificate.py` independently replays the
projected algebra.  On the 16-atom certificate it checks all 242 minimal cores, 641,741 partial
global cut assignments, their upward-substate closure, and all 25 nodes of the first rank-305
projected tree.  On the 32-atom certificate it checks 504 cores and 1,776,407 assignments, verifies
that all three complete rank bands in (11) are represented, and verifies that every one of their
roots contains a listed core.  The durable objects are `evidence/atom_profile_height6_dc16.cert`
and `evidence/atom_profile_height6_dc32.cert`.  The checker does not trust the producer's search
depth or memo answers.

`tools/check_dc_tree_lift.py` then reimplements the omitted-coordinate intervals and proves that
this first skeleton has no exact lift.  Its all-skeleton mode produced the alternative exact tree
in `evidence/atom_profile_height6_rank305.cert`; `tools/check_atom_profile_tree.py` independently
re-derives every one of its profiles, splits, children, leaf inequalities, and threshold.  The
ordinary regression verifies both the fixed-skeleton negative and the retained positive tree;
rerunning the two-minute all-skeleton discovery is optional because the tree itself is the proof.

The same regression builds the 32-atom exact engine, checks a 32-atom singleton tree, locks a sharp
mixed-supply rejection, and exhausts rank 1180 through depth three.  A small positive two-part tree
guards the complete-product search path used there; `tools/check_dc_tree_lift.py --all-skeletons`
independently repeats the bounded rank-1180 negative.  At depth four, both implementations enumerate
the same 7,266 filtered first tests, 6,712 pure-feasible tests and 1,826 mixed children.  They also
independently exhaust the same eight positive-`V`-loss children, all negative, which certifies the
reduced frontier (11a).  `tools/check_atom_profile_certificate.py`
independently checks the local triangular supply inequality while replaying 5,540,319 transitions
at 16 atoms and checks 5,814 ordered-triple/depth propagation cases against literal triangular
iteration.
`tools/check_atom_parent_formula.py` separately reconstructs (12) from the three outer profiles and
compares (13) with direct atom evaluation in 5,136 cases, including all three boundary
specializations in (15); it also independently derives every value and loss budget in (16).
