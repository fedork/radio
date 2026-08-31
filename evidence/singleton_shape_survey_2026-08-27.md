# Singleton shape-preserving cut survey (2026-08-27)

## Question

For a full-mass singleton partition `a<=_w G_K`, write

    E_a(t) = sum_i max(a_i-t,0)
    theta(a) = E_a(1)/E_(G_K)(1)

and let `s` be the number of parent rows split between the mixed child and one
pure child.  The exact identity

    sum_(children C) E_C(1) = E_a(1)-s

suggests preserving the normalized coalescence shape on average by taking

    s approximately 2^(K-1) theta(a).

A unit row cannot split.  The surveyed scalar target is therefore the floor or
ceiling of

    min(2^(K-1) theta(a), #{i:a_i>=2}).

`tools/singleton_shape_survey.cpp` reconstructs first cuts directly.  It checks
that every row feeds either `{left,mixed}` or `{mixed,right}`, that every child
has mass `3^(K-1)`, and that all three child partitions are weakly majorized by
`G_(K-1)`.  Thus a reported success does not assume the now-refuted converse.

## Build and exact scalar results

The utility was provenance-built with

    CC=clang++ tools/build_radio.py -O3 -std=c++20 -Wall -Wextra -pedantic \
        tools/singleton_shape_survey.cpp -o /tmp/singleton-shape-survey

The exact commands are

    tools/run_with_provenance.py /tmp/singleton-shape-survey --census 3
    tools/run_with_provenance.py /tmp/singleton-shape-survey --census 4

and returned

    SHAPE_SURVEY mode=census k=3 checked=1206 nodes=52661 max_nodes=2362 result=NO_FAILURE
    SHAPE_SURVEY mode=census k=4 checked=5997038 nodes=16639423162 max_nodes=6578091 result=NO_FAILURE

The `K=3` run took 0.03 user CPU seconds.  The `K=4` run took 1,854.89 user CPU
seconds and 1,867.57 wall seconds on the recorded M4 Pro.
Before the exact `K=4` run, a uniform 100,000-state sample from the exact
5,997,038-state universe also found no failure.

This is exhaustive verification only through `K=4`, not a proof of the scalar
rule or of the Singleton Majorization converse.

## Why the nonunit cap is intrinsic

If a parent row `u=x+y` is split, define

    j_(x,y)(t) = min(x,t)+min(y,t)-min(u,t).

Then at every threshold

    sum_(children C) E_C(t) = E_a(t)-J_a(t).

The maximum contribution of row `u` is `min(t,max(u-t,0))`, so

    J_a(t) <= E_a(t)-E_a(2t).

At `t=1` this becomes `s<=#{i:a_i>=2}`.  The unit-row obstruction is the first
coordinate of the full hinge-profile capacity, rather than a special case.

## Literal full-profile similarity fails

Let

    B_K(t)=E_(G_K)(t)-3E_(G_(K-1))(t).

The literal all-threshold proposal asks `J_a(t)` to be the floor or ceiling of

    min((E_a(t)/E_(G_K)(t)) B_K(t), E_a(t)-E_a(2t))

simultaneously.  The exact command

    tools/run_with_provenance.py /tmp/singleton-shape-survey --profile-census 3

refutes it at the 61st state:

    a=(8,5,5,5,1,1,1,1).

The targets include `J_a(1)=4` and `J_a(2)=6`.  A legal scalar-target cut has

    L=(4,3,1,1)
    M=(4,2,2,1)
    R=(4,3,1,1),

for which `J_a(1)=4` but `J_a(2)=7`.  Exhaustive search finds no cut satisfying
all independently rounded profile coordinates.  The full hinge profile is still
the intrinsic shape, but its coordinates have coupled lattice constraints.

The finalized diagnostic line is

    PROFILE threshold:lower-upper/first_feasible 1:4-4/4 2:6-6/7 3:8-8/8 4:7-7/7 5:3-3/3 6:2-2/2 7:1-1/1

## Feasible split-count intervals

The `--interval-census` mode computes every feasible number `s` of genuinely split rows in one
memoized traversal.  With build id
`a03d47456d5b6689d24dc9d9337c293415604e2be5a89c959dc65d1311230818`, the command

    tools/run_with_provenance.py /tmp/singleton-shape-survey --interval-census 3

returned no non-interval fiber among all 1,206 states.  It visited 4,740,395 recursive states.
The exact endpoint comparison was:

    minimum_equals_hinge=1190
    maximum_equals_splittable=955
    maximum_equals_mixed_bound=1177
    both_scalar_roundings_feasible=1185

Here the hinge lower bound is obtained by asking, at every threshold, how many largest row
capacities are needed to supply `E_a(t)-3E_h(t)`.  The mixed upper bound notes that every row wider
than the child width must split and must put at least its excess in the mixed child.

The misses are small but structural.  For

    a=(8,2,2,2,2,2,2,2,2,2,1)

the hinge bound is one split but the feasible interval is `2..6`: after `8=4+4`, the three residual
branch masses have a parity obstruction, and splitting one `2` repairs it.  At the other endpoint,

    a=(8,7,2,2,2,2,2,2)

only `s=4` is feasible although all eight rows can individually split.  The `8` and `7` already
consume at least seven of the mixed child's nine units, leaving room for only two further split
rows.  Finally,

    a=(8,6,5,3,2,1,1,1)

misses even that sharpened mixed bound by one: splitting all five nonunit rows forces mixed pieces
`(4,2,1,1,1)`, while the pure remainders contain three `4`s, so one pure child violates the top-two
cap `4+3=7`.

Thus the split-count fiber happens to be an interval at `K=3`, but its endpoints already require
integer packing information beyond the hinge inequalities.  A ten-state `K=4` interval sample was
stopped after 60 CPU seconds without a verdict; computing an entire fiber is much harder than
finding the scalar target and is not currently a promising induction route.

## Aborted higher-level attempt

A direct `--uniform 5 1000 314159265` run was stopped by its 60-CPU-second
limit before producing even a batch verdict.  It is an abort and supplies no
`K=5` evidence.  Direct assignment enumeration is the wrong high-level method;
the fixed-color Hall projection used by the row-coloring census is much faster.

## Per-row atom-sized pieces fail at K=3 (2026-08-28)

The `--atom-census` mode requires every parent row's image to contain a positive piece whose
width occurs in `G_(K-1)`.  An intact row is represented by its positive piece and a zero piece;
zero is not an atom width.  The stronger `--larger-atom-census` mode requires the largest piece
to have a canonical child-row width.  With build id
`2140914d270dd46b4698a9a8bf352df8bf28f57d2f9945915d67fed56e96acfe`, the commands

    tools/run_with_provenance.py /tmp/singleton-atom-survey --atom-census 3
    tools/run_with_provenance.py /tmp/singleton-atom-survey --larger-atom-census 3

both stop at the fourth descending state:

    SHAPE_SURVEY mode=atom-census k=3 checked=4 nodes=33 max_nodes=10 result=FAIL state=(8,7,4,2,2,2,1,1)
    SHAPE_SURVEY mode=larger-atom-census k=3 checked=4 nodes=31 max_nodes=9 result=FAIL state=(8,7,4,2,2,2,1,1)

This failure has a direct proof, independent of the search.  The state is majorized by
`G_3=(8,7,4,4,1,1,1,1)`.  The widths occurring in `G_2` are `{4,3,1}`.  The child width cap forces
`8=4+4`; the atom rule and the cap make the mixed contribution from `7` at least three; and each
of the three width-two rows must be `1+1`.  Every genuine split has one mixed piece, so those rows
alone force mixed mass at least `4+3+1+1+1=10`, exceeding the branch mass `3^2=9`.

The unrestricted state command returns the ordinary legal children

    L=(4,3,1,1) M=(4,3,1,1) R=(4,2,2,1).

One row-level realization is

    8 -> (4,4,0),  7 -> (0,3,4),  4 -> (3,1,0),  2 -> (1,1,0),
    2 -> (0,0,2),  2 -> (0,0,2),  1 -> (1,0,0),  1 -> (0,0,1).

This witness uses the freedom to leave two noncanonical width-two rows intact.  The counterexample
does not address a restriction applying only to genuinely split rows, but it
rules out the proposed condition on every row and, a fortiori, its larger-piece version.
