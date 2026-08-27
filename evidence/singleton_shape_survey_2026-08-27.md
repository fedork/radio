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
`G_(K-1)`.  Thus a reported success is not based on the open converse.

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

## Aborted higher-level attempt

A direct `--uniform 5 1000 314159265` run was stopped by its 60-CPU-second
limit before producing even a batch verdict.  It is an abort and supplies no
`K=5` evidence.  Direct assignment enumeration is the wrong high-level method;
the fixed-color Hall projection used by the row-coloring census is much faster.
