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

Proving (PA) uniformly for the Pascal/dyadic prefix functions would prove an explicit no-exchange
partition algorithm for every parent having at least `2*3^(K-1)` nonzero rows.  It does not address
the lower-support regime, which contains all known strict-alternation counterexamples.

## Exact checks

`tools/singleton_pair_coloring_census.cpp` now has two relevant modes.  Build and run with

    CC=clang++ tools/build_radio.py -O3 -std=c++20 -Wall -Wextra -pedantic \
        tools/singleton_pair_coloring_census.cpp -o /tmp/singleton-pair-census-padded
    tools/run_with_provenance.py /tmp/singleton-pair-census-padded \
        --padded-three-census 4
    tools/run_with_provenance.py /tmp/singleton-pair-census-padded \
        --padded-prefix-check 12

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

The final source build was
`a895141b33ec5893167eb3e49a33dad0a11d2016ebd1e43ceeff2471907a67c3`.  The provenance-wrapped
complete `K=3,4` censuses took 5.4 wall seconds together; the twelve arithmetic checks took
2.1 wall seconds together on the recorded M4 Pro.  These finite checks support (PA) but do not
prove it for arbitrary `K`.
