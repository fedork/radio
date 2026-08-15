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
is root base `r>=13`.  Consequently (4) is the widest D germ among all 165 A--D eight-atom profiles,
with no synchronized-depth qualifier: every wider germ has the all-depth obstruction above.

Attaching the already constructed outer A/B/C branches in the working four-segment assembly gives
the conditional parent profile

    A^21 B^6 C^3 D^2 @ G_(k-5),

whose width is

    2^k - 2 binom(k-5,2) - 5(k-5) - 11
      = 2^k - k^2 + 6k - 16,                                      (5)

valid from the symbolic hard-branch threshold `k>=18`.  Equation (5) is an aligned-family lower
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
- the remaining arbitrary-excess-`q` maximization now begins at the 32-atom slice, where a profile
  wider than the refinement of rank 305 may still exist.

## Mechanical verification

`tools/atom_profile_regression.sh` builds both the 8- and 16-atom variants.  It invokes
`tools/check_atom_profile_certificate.py`, which independently enumerates all local refinement/cut
transitions (174,069 cases at eight atoms and 5,540,319 at sixteen), and
`tools/check_atom_profile_tree.py`, which independently re-derives every state, split, terminal
majorization inequality, and threshold in the positive trees.  Neither checker shares code with
the C++ search.  The combined durable eight-atom proof object is
`evidence/atom_profile_height6_ad8.cert`.

For the new two-coordinate result, `tools/check_dc_kernel_certificate.py` independently replays
the projected algebra.  It checks all 242 minimal cores, 641,741 partial global cut assignments,
their upward-substate closure, and all 25 nodes of the first rank-305 projected tree.  The durable
object is `evidence/atom_profile_height6_dc16.cert`.  The checker does not trust the producer's
search depth or memo answers.

`tools/check_dc_tree_lift.py` then reimplements the omitted-coordinate intervals and proves that
this first skeleton has no exact lift.  Its all-skeleton mode produced the alternative exact tree
in `evidence/atom_profile_height6_rank305.cert`; `tools/check_atom_profile_tree.py` independently
re-derives every one of its profiles, splits, children, leaf inequalities, and threshold.  The
ordinary regression verifies both the fixed-skeleton negative and the retained positive tree;
rerunning the two-minute all-skeleton discovery is optional because the tree itself is the proof.
