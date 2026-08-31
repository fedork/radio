# Padded three-block reduction (2026-08-27)

## Exact reduction

Put `M=3^(K-1)`, `h=G_(K-1)`, and let `H` be the saturated prefix function of `h`.
Pad a full-mass parent state to `3M` labelled rows.  Partition the slots into blocks `A,B,C`
of size `M`; rows in `A` may use left and mixed, rows in `B` may use mixed and right, and rows
in `C` may use only mixed.  If `A_p,B_q,C_r` are the sums of the largest indicated numbers of
rows in the blocks, integral bipartite matching gives the exact condition

    A_p+B_q+C_r <= H(p)+H(q)+H(p+q+r)                    (T)

for every `0<=p,q,r<=M`.  The two whole-block inequalities saying that `A` and `B` each have
mass at least `M` are the complementary cases `(0,M,M)` and `(M,0,M)` of (T); they are not
sufficient by themselves.

The mixed-only block can be canonicalized.  Suppose a feasible partition has rows `x in C` and
`y in A` with `a_x>=a_y`.  Swap their block memberships.  A Hall set containing `x` but not `y`
gains pure-left capacity.  For a set containing `y` but not `x`, replace `y` by `x` before the
swap: it has the same old capacity as the original set has after the swap and at least as much
demand.  Old feasibility therefore proves new feasibility.  The same argument applies to `B`.
Repeated exchanges show that, without loss of generality, `C` is the `M` lightest padded rows.

Those rows are all zero or one: otherwise the first `2M+1` rows would already have mass at least
`2(2M+1)>3M`.  Write

    C=(1^c,0^(M-c)),             E=M-c.

For fixed `p,q`, minimizing the right side of (T) over `r` gives

    J_c(p+q)=min(H(p+q), H(p+q+c)-c)=min(H(p+q),M-c).

Indeed, for `r<=c` the increments of `H(t+r)-r` are `h_(t+r)-1`, a nonincreasing sequence, so
its minimum is at an endpoint; for `r>=c`, `H(t+r)-c` is nondecreasing.  The final equality uses
the fact that every positive row of `h` is at least one.  Thus, after deleting `C`, the exact
condition on the remaining `2M` rows is

    A_p+B_q <= H(p)+H(q)+min(H(p+q),E).                 (U)

This is the old two-color Hall problem with the mixed child truncated from mass `M` to mass `E`.

## What parent majorization supplies automatically

Assume the parent has at least `2M` nonzero rows.  Then it has exactly `2M+c` nonzero rows and,
for the prefix `P(t)` of the remaining `2M` positive rows,

    P(t) <= U_E(t)
         := min(H_K(t),E+t),
    H_K(t)=H(t)+H(ceil(t/2))+H(floor(t/2)).              (V)

The first bound is parent majorization.  The second follows because every one of the
`2M+c-t` unselected positive rows has mass at least one.  These bounds already imply every
balanced cut:

    U_E(t) <= min(H(t),E)+H(ceil(t/2))+H(floor(t/2)).

If `H(t)<=E`, use parent majorization; otherwise use `E+t` and
`H(ceil(t/2))+H(floor(t/2))>=t`.

Now alternate the remaining rows, putting odd ranks in `A` and even ranks in `B`.  Pairing selected
and unselected ranks gives

    2(A_p+B_q) <= P(2p)+P(2q),                    q>=p,
    2(A_p+B_q) <= P(2q+1)+P(2p-1),                p>q.       (W)

The first family is automatic.  Since

    U_E(2s)=2H(s)+min(H(2s),E-2(H(s)-s)),

concavity of `H` gives

    (U_E(2p)+U_E(2q))/2
      <= H(p)+H(q)+min(H(p+q),E).

Only the side receiving the larger row of each alternating pair remains.  A sufficient purely
arithmetic statement is therefore

    floor((U_E(2q+1)+U_E(2p-1))/2)
      <= H(p)+H(q)+min(H(p+q),E),                  p>q.       (PA)

(PA) is a sufficient condition for this explicit no-exchange partition algorithm.  The finite
checks below first suggested that it might hold uniformly, but the exact `K=19` construction below
refutes it even in the high-support regime.

## Exact checks

`tools/singleton_pair_coloring_census.cpp` now has three relevant modes.  Build and run with

    CC=clang++ tools/build_radio.py -O3 -std=c++20 -Wall -Wextra -pedantic \
        tools/singleton_pair_coloring_census.cpp -o /tmp/singleton-pair-census-padded
    tools/run_with_provenance.py /tmp/singleton-pair-census-padded \
        --padded-three-census 4
    tools/run_with_provenance.py /tmp/singleton-pair-census-padded \
        --padded-prefix-check 12
    tools/run_with_provenance.py /tmp/singleton-pair-census-padded \
        --padded-alternation-counterexample

The complete state census gives:

| `K` | all states | states with `c>0` | alternating failures with `c>0` | all alternating failures |
|---:|---:|---:|---:|---:|
| 3 | 1,206 | 66 | 0 | 8 |
| 4 | 5,997,038 | 11,309 | 0 | 11,772 |

At `K=3` the failures have 9 or 11 nonzero rows.  At `K=4` they have 16 through 21 nonzero rows.
Thus no failure in either complete census is close to the `2M` threshold (18 and 54 respectively).

The arithmetic mode checks all integer `E`, not sampled states.  For fixed `p,q`, both sides of
(PA) are piecewise linear in `E`; their only breakpoints are
`H_K(2q+1)-(2q+1)`, `H_K(2p-1)-(2p-1)`, and `H(p+q)`.  A maximum difference therefore occurs at
an endpoint or an integer adjacent to one of these breakpoints.  The mode checks those points.
It only needs `p,q<=|h|`: if either count is larger, its pure prefix has saturated at `M`, the
mixed prefix has saturated at `E`, and the support bound `E+p+q` proves (U) immediately.
Every level `K=1..12` passes.  At `K=12` it checks 2,098,176 pairs and 20,201,473 breakpoint values.

That finite pattern eventually breaks.  At `K=19`, put `M=E=3^18=387,420,489` and define
`U(t)=min(H_19(t),M+t)`.  There is a sorted state with `2M` positive rows whose compressed form is

* rows `1..513`: the corresponding `G_19` rows;
* 294 rows of value 261,964, followed by 218 rows of value 261,963;
* rows `1026..1281`: the corresponding `G_19` rows;
* row 1282: 81,226;
* 774,839,696 trailing unit rows, followed by the padding zeros.

Its total mass is `3M`, and every prefix is at most the corresponding prefix of `G_19`.  The tool
checks sortedness, total mass and all nontrivial majorization prefixes directly from this compressed
description.  Under strict alternation, however, the contracted Hall inequality at `p=513,q=256`
is

    A_513+B_256 = 276,817,774
      > 276,815,343 = H(513)+H(256)+H(769).

The excess is 2,431.  Hence both (PA) and the proposed high-support strict-alternation algorithm are
false.  This did **not by itself** refute the Row-Coloring conjecture; it said that these same rows need a different,
globally chosen orientation if the lemma is true.

For adjacent pairs `(x_i,y_i)`, let `d_i=x_i-y_i`, choose a sign `epsilon_i` according to which
color gets `x_i`, and put `D_n=sum_(i<=n) epsilon_i d_i`.  Then exactly

    A_p+B_q = (P(2p)+P(2q)+D_p-D_q)/2.

The adaptive-pair problem is therefore a signed-walk feasibility problem with simultaneous bounds
on every interval `D_p-D_q`.  This is a cleaner target for the high-support subproblem, but not a
candidate for the full lemma: arbitrary adjacent-pair orientation already fails on 916
lower-support `K=4` states.  No general feasibility proof is known even in the high-support regime.

The finite-check source build was
`a895141b33ec5893167eb3e49a33dad0a11d2016ebd1e43ceeff2471907a67c3`.  The provenance-wrapped
complete `K=3,4` censuses took 5.4 wall seconds together; the twelve arithmetic checks took
2.1 wall seconds together on the recorded M4 Pro.  The final counterexample build was
`44c17992e86ea10573820f143cc58fb8d3519edc5d6130e4d9659586725be8f7`; its provenance-wrapped
construction and verification took less than 0.1 wall second after a 2.6-second build-and-run
command on the same machine.  The `K<=12` checks are retained as a warning about extrapolating a
large finite range, not as evidence for (PA).
