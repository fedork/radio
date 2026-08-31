# Exact `m=5` as a Pareto-assembly calibration

Li--Wu--Triesch prove the exact piecewise frontier for `K_(5,n)`, equivalently `Sb(n:5)`.
This note does not replace their proof.  It recomputes a proposed lower construction in the
corrected A/B/C/D assembly coordinates and isolates a one-dimensional D problem.  **Correction
2026-08-30:** the arbitrary singleton-majorization terminals used in that reconstruction are not
certificates because the universal converse is false.  The published theorem remains unconditional, as do the local
upper-bound reduction and any branches explicitly exactified to canonical/distinct-slot leaves.
The assembly is therefore a diagnostic calibration rather than an independent reconstruction.
See [the primary-source summary](../literature.md#li-wu-triesch-2018).

Throughout, put

    t = k-2,        P = 2^t,        Q = binomial(t-2,2).

The already exact lower-height frontiers used below are

    n(t,1)   = P,
    n(t,2)   = P-1,
    n(t,3)   = P-t,
    n(t+1,4) = 2P-2t.

In these coordinates the published theorem is

    n(k,5) = 4P-3t-Q-1,    4 <= k <= 8,
             4P-3t-Q,      9 <= k <= 10,
             4P-3t-Q+1,    k >= 11.                         (1)

The last line is a single proved range quantified over every `k>=11`.  It excludes any later
transition in the **numerical frontier**.  It does not assert uniqueness of an optimal strategy,
continued identity of every internal partition, or an aligned per-coin `AABD` profile.

The `k=3` theorem value is correct in the paper's symmetric graph notation, but `K_(5,3)` is
stored here after swapping its shores, so the normalized one-part calibration starts at `k=4`.

## The old `3+2` branch

Choose the assembly heights

    (alpha,beta,gamma) = (3,2,2)

and the exact lower states

    a = n(k-1,3) = 2P-t-1,
    b = c = n(t,2) = P-1.

The old construction takes

    d_old = P-2t-Q+1.

Its hard branch at level `t` is

    Sb(d_old:2, (P-1):1, (P-t):2) @t,                         (2)

and the parent width is

    a+b+d_old = 4P-3t-Q-1.                                   (3)

The atom construction in [the ground-up derivation](../conjectures.md#ground-up-atom-induction-the-outer-states-are-black-boxes)
proves (2) uniformly for `k>=7`.  It is the `BBBD@G_(k-2)` lower construction.  Equation (3)
reaches the exact frontier through `k=8`, remains a valid construction afterwards, and is one and
two coins short of (1) in the next two regimes respectively.

## The new `4+1` branch

Now choose

    (alpha,beta,gamma) = (4,3,1)

and

    a = n(k-1,4) = 2P-2t,
    b = n(t,3)   = P-t,
    c = n(t,1)   = P,
    a-c          = P-2t.                                      (4)

Thus `C` fits inside `A` throughout the theorem's new regime.  For a free width `d`, the parent
candidate is

    N = a+b+d = 3P-3t+d.                                      (5)

The first test selects `[a:4]`.  Its pure outcomes are `Sb(a:4)@k-1` and
`Sb(b+d:1)@k-1`; the mixed outcome is

    Sb(a:1, b+d:4) @k-1.

In that mixed outcome, select `[a-c:0]` from the first part and `[b:3]` from the second.
Direct rectangle algebra gives the three level-`t` outcomes

    outcome 2: Sb(b:3),
    outcome 0: Sb(c:1,d:1),
    outcome 1: R_t(d) = Sb(d:3,b:1,a-c:1).                     (6)

The first is the exact `m=3` construction.  The second embeds in the first two distinct rows
`(P,P-1)` of `G_t` whenever `d<=P-1` and is therefore unconditionally solvable.  Thus the only
synchronized obligation after the two outer tests is

    R_t(d) = Sb(d:3, (P-t):1, (P-2t):1) @t.                    (7)

This is exactly the proposed variable-D slice, with no A/B/C witness-tree alignment involved.

### D-slice upper bound and unsupported relaxed matches

The proposed matching value for the branch (7) is

    d*(t) = P-Q,       t=7,8,                                 (8)
            P-Q+1,     t>=9.

Equivalently, (8) covers parent levels `k=9,10` and `k>=11`.  At the transition base `k=8`
(`t=6`), exact local assembly gives the tied value `d*=P-Q-1=57`.

**Relaxed-terminal derivation, not achievability.**  The structurally checked `k=9` file contains the literal state
`Sb(118:3,121:1,114:1)@7`; see
[`majorized_481_5_at9.tree`](../../witnesses/majorized_481_5_at9.tree).  The relaxed regression
checks `d=241` at `t=8` and the tied `t=6` base.  For every `t>=9`, the uniform
derivation below reaches singleton states majorized by `G`; this does not finish the lower bound.
Theorems 2 and 3 of Li--Wu--Triesch independently prove the parent
frontier by their published construction, but this note has not established that their internal
state is exactly (7).

**Sharpness.**  Suppose `R_t(d*(t)+1)` were solvable.  For `t>=7`, both untouched outer branches
would still be solvable:

    d*(t)+1 <= P-1,        b+d*(t)+1 <= 2P.

Composing that D solution with the two tests above and the exact `m=4` and `m=3` branches would
solve `Sb(n(k,5)+1:5)@k`, contradicting the published exact upper bound.  Subgraph monotonicity then
excludes every larger `d`.  Thus the global theorem supplies the displayed upper bound for the
local D slice.  Equality still needs an unconditional construction of `R_t(d*(t))`.

### A majorization-terminal two-test template for `t>=9`

The eventual `+1` is not only a consequence read backwards from the published answer.  Set

    H=P/2,        J=P/4,        d=P-Q+1.

In `R_t(d)`, aligned in the part order shown in (7), make the test

    [H-t:2, H:1, H-2t:0].

Its three outcomes are

    U2 = Sb(H:1,(H-t):2),
    S  = Sb((H-Q+t+1):2,(H-t):1,(H-t):1,(H-2t):1),
    U0 = Sb(H:1,(H-Q+t+1):1).

`U0` is weakly majorized by `G_(t-1)`.  Test `U2` with

    [J:0,(J-1):1],

and test the four displayed parts of `S` with

    [(J-1):1,J:1,(J-t):0,(J-2t+1):0].

Every resulting part is a singleton.  The `U2` leaves are subprofiles of the first three entries of
`G_(t-2)`.  The three `S` leaves have width sequences

    (J,J-1),
    (J,J-1,J-Q+t+2),
    (J-1,J-Q+t+2,J-t,J-t,J-2t+1).                 (9)

For `r=t-2`, the first five entries of `G_r` are

    (J,J-1,J-t+1,J-t+1,J-Q-t+1).

The last sequence in (9), viewed as deficits from `J`, has multiset

    {1,Q-t-2,t,t,2t-1},

whereas the five reference deficits are

    {0,1,t-1,t-1,Q+t-1}.

Their total deficits agree.  For `t>=9`, `Q-t-2>=t`; the first four sorted deficit sums of the
candidate are therefore at least the corresponding reference sums.  The three-entry leaf is also
majorized because `Q>=2t+1`, and `U0` because `Q>=t+2`.  These inequalities all hold from `t=9`,
but weak majorization cannot complete the construction.  Where the decisive
five-part leaf is subsequently replaced by an explicit canonical tree, that finite instance is
unconditional.

The same test dimensions are legal at `t=7,8`, but the three-entry `S` leaf already fails there:
`J-Q+t+2 > J-t+1`.  Thus this one template exposes the real threshold.  The two earlier values need
their separate finite constructions rather than a fictitious uniform continuation.

### Exactifying the decisive majorized leaf

The five-part leaf on the last line of (9) can be written independently of the outer construction.
Put `r=t-2`, `A_r=2^r`, and

    B_r = A_r-1,
    X_r = A_r-r-2,
    E_r = A_r+r+4-binomial(r,2),
    Y_r = A_r-2r-3.

Then the leaf is

    P_r = sort(B_r,X_r,X_r,E_r,Y_r) @r.

At the first eventual case, parent `k=11`, this is

    P_7 = (127,119,119,118,111) @7.

There are three distinct terminal notions here.  Weak majorization verifies the necessary prefix
condition for `P_7`, but is not by itself a positive certificate.  An **embedded** terminal requires
its sorted widths to fit coordinatewise into distinct
slots of `G_s`.  An **exact** terminal requires the widths to be a literal sub-multiset of `G_s`.
Exact implies embedded, and both properties are hereditary on deleting parts, so the recursive
partial-state exclusions remain sound.

The complete specialized recurrence gives a sharp answer for `P_7`:

| terminal required | at most 2 further tests | 3 further tests |
|---|---:|---:|
| embedded in distinct `G_s` slots | no | yes |
| exact sub-multiset of `G_s` | no | yes |

The positive exact tree is
[`canonical_m5_leaf_p7_at7.tree`](../../witnesses/canonical_m5_leaf_p7_at7.tree); its 19 nodes,
six splits, and 13 canonical leaves are re-derived by `tools/check_witness.py`.  The two negative
depth-two searches and both positive depth-three searches are locked by
`tools/singletonization_regression.sh`.  Thus **three is the minimum for this concrete leaf**.  It
does not mean that the whole `m=5` strategy uses three levels, nor that one depth works uniformly
for every `r`.

As finite construction data, the same regression exactifies `P_8` and `P_9` in three tests.  At
`P_10` it exhaustively rejects depth three and verifies a depth-four exact tree.  This is the first
observed change in the exactification depth, not a new transition in the already settled parent
frontier and not an eventual depth formula.

The uniform exact question has a different answer.  Fix an extra depth `d`, put `s=r-d` and
`N=2^d`, and measure each component from `N A_s`.  The component deficits are

    Delta(B_r)=1,
    Delta(X_r)=r+2,
    Delta(Y_r)=2r+3,
    Delta(E_r)=binomial(r,2)-r-4.

The first four distinct atom values of `G_s` are

    A_s=2^s,
    B_s=A_s-1,
    C_s=A_s-s-1,
    D_s=A_s-1-s(s+1)/2.

For fixed `d` and sufficiently large `r`, every component of `P_r` exceeds `(N-1)A_s`, so an
exact inventory has exactly `N` positive pieces.  An exact `E_r` inventory must use exactly one
`D_s`; lower atoms have cubic-or-larger deficit and two `D_s` atoms already have too large a
quadratic coefficient.  If it
also uses `q` copies of `C_s` and `p` copies of `B_s`, coefficient comparison forces

    q=d-2,        p=(d-6)(d+1)/2.

Hence every fixed `d<=5` eventually requires a negative number of B atoms and cannot give a uniform
exact fit.  The first arithmetically possible depth is `d=6`.  At that depth the four individual
component inventories are, with exponents denoting multiplicity,

    B_r = A_s^63 B_s,
    X_r = A_s^56 B_s^7 C_s,
    Y_r = A_s^49 B_s^13 C_s^2,
    E_r = A_s^59 C_s^4 D_s.

`tools/m5_assembly.py` checks these 64-piece identities at every covered symbolic level where
`A_s,B_s,C_s,D_s` are positive atoms.  It also locks the first elementary failure of depth three:
for `r=10`, `Y_10=1001` would need eight `G_7` atoms with total deficit 23, but the available small
deficits are `0,1,8` and `p+8q=23`, `p+q<=8`, has no solution.

The depth-six identities are only **per-component inventories**.  They do not assign those atoms to
common outcome columns and therefore do not prove a synchronized six-test tree.  The correct current
statement is: six is the candidate minimum uniform **exact** depth; exact sufficiency and the minimum
uniform embedded depth are open.  In particular, the paper neither needs nor proves this stronger
atomization.

### Recomputed indices

The selected width in the singleton component on the second outer test is

    a-c = 2^(k-2)-2(k-1)+2 = P-2t.                            (10)

Some displayed descendants around equations (69)--(70) of the paper replace `k-1` by `k-2`,
which would make that width two units too large.  Equation (10) follows directly from (4), and the
exact `k=9` hard branch contains 114, not 116.  The theorem statements and final piecewise formula
are consistent; the intermediate rectangles must be recomputed rather than copied.

## The envelope and the atom arithmetic

The exact assembly enumeration over source-carrying Pareto inputs gives the following qualitative
transition, locked by `tools/singletonization_regression.sh`:

- the `(3,2,2)` branch is the sole winner at parent levels `k=4..7`;
- `(3,2,2)` and `(4,3,1)` tie at `k=8`; and
- `(4,3,1)` is the sole winner at `k=9`.

Default-terminal construction checks of (7) give `d=241` and `d=492` at `k=10` and `k=11`,
matching the published maxima 985 and 2001 conditionally.  The published theorem is the
unconditional source for both achievability and optimality.

There is a compact mass interpretation.  At `G_t`, let

    A=P,        B=P-1,        D=P-1-t-binomial(t,2).

Then

    BBBD = 4P-3t-Q-1,
    ABBD = BBBD+1,
    AABD = BBBD+2.                                            (11)

Consequently the three lines of (1) have masses `BBBD`, `ABBD`, and `AABD`.  Equation (11) is
arithmetic only.  The published strategies and the committed 481 tree are not presently proved to
be symmetric, non-wasteful aligned realizations of the latter two words.  An atom-mass identity must
not be promoted to an atom-profile construction without such a tree.

## What this calibration establishes

The known exact `m=5` result supports the general assembly mechanism but changes its state space:

1. Conditional on an unconditional solution of the D state, A/B/C can be used as black boxes;
   only their outer dimensions enter (7).
2. Maximizing D for each outer triple remains the correct local problem.
3. A global construction must retain competing outer triples and take their envelope; one repeated
   height choice is not enough.
4. In the relaxed terminal model, `d*` can change symbolic regime because a leaf-majorization
   inequality changes truth value; here that happens exactly between `t=8` and `t=9`.
5. None of this proves that every height has an eventual aligned atom regime or that the assembly
   exhausts every unrestricted strategy.

The executable arithmetic and two-test rectangle algebra are in `tools/m5_assembly.py`.
`tools/check_tables.py` verifies its identities
through `k=64` and checks every recorded exact `m=5` datum against the published theorem.  The exact
finite construction controls and their independently checked branch trees are part of
`tools/singletonization_regression.sh`.

## Research status

This calibration is complete.  The broader excess-`q` assembly programme was parked on 2026-08-16.
Nothing above proves that the assembly family is globally exhaustive, that a sufficiently large
normalization always stabilizes, or that a fixed outer triple has one uniform D formula.  The note
remains the known-answer test that any future general construction must pass.
