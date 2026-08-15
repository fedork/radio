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

## Why larger normalization is still open

The 165 profiles above are all A--D profiles of length eight, not all excessive-q profiles.  At
length 16 there are 969 A--D profiles.  The same lineage theorem excludes ranks 1 through 289; the
refined `ABBBBBCD` class is rank 191, and the first survivor is rank 290, `A^14D^2`.

The two-coordinate abstraction `(p_D,p_C+p_D)` is a sound over-approximation: it retains the first
two eventual deficit coefficients and allows every projected aligned cut, while dropping the final
coefficient at terminal comparison.  Therefore an abstract `NO` is an exact bounded-depth
exclusion.  It rejects the 16-atom rank-290 state through depth 5 in the retained regression.  A
60-second exact scan reached the next `(D,C+D)` band at rank 305 without a verdict, and the known
eight-atom positive refines to the constructible profile `A^12C^2D^2` at rank 319.  Whether a new
16-atom germ in ranks 290--318 wins at greater depth is open.

So the symbolic result is sharp but scoped:

- the one-D proposal is refuted at *all* depths and under every pure refinement;
- the eight-atom A--D optimum is proved exactly;
- maximizing over genuinely new profiles introduced by larger `q` remains the next problem.

## Mechanical verification

`tools/atom_profile_regression.sh` builds both the 8- and 16-atom variants.  It invokes
`tools/check_atom_profile_certificate.py`, which independently enumerates all local refinement/cut
transitions (174,069 cases at eight atoms and 5,540,319 at sixteen), and
`tools/check_atom_profile_tree.py`, which independently re-derives every state, split, terminal
majorization inequality, and threshold in the positive trees.  Neither checker shares code with
the C++ search.  The combined durable eight-atom proof object is
`evidence/atom_profile_height6_ad8.cert`.
