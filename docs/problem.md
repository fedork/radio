# The problem and its notation

## Setting

Two-defective quantity group testing. There are `n` coins, exactly two of which are
radioactive. A **test** selects a subset and reports how many of the two defectives it
contains: `0`, `1`, or `2`. The question is the largest `n` for which both defectives can
always be identified within `k` tests.

Because each test has three outcomes, `k` tests produce at most `3^k` distinct outcome
sequences. Any state with more than `3^k` surviving candidate pairs is therefore
unsolvable in `k`. This is the **information bound**, and it is the one hard constraint
that every result here respects.

## State types

A state records what is still possible about the location of the two defectives.

**`Sa(n)`** - `n` indistinguishable coins, both defectives somewhere among them, nothing
else known. Candidate pairs: `C(n,2) = n(n-1)/2`.

**`Sb(n1:m1, n2:m2, ...)`** - a disjoint union of alternatives. In alternative `i`, one
defective lies in a group of `n_i` coins and the other in a *disjoint* group of `m_i`
coins. Candidate pairs: `sum_i n_i * m_i`. This quantity is called the **mass** of the
state and is what the information bound applies to.

Orientation is irrelevant: `Sb(a:b) = Sb(b:a)`. Order of parts is irrelevant. Parts with a
zero side are empty and are dropped. Throughout, each part is written `n_i >= m_i` and
parts are listed in descending order.

An `Sb` state is called a **singleton state** if every part has `m_i = 1`, and a
**unit group** is a part `1:1` - a part with no remaining ambiguity at all.

## What one test does

A test on an `Sb` state is described by a **split vector** `[a1:b1, a2:b2, ...]`, one entry
per part, meaning "take `a_i` coins from the `n_i` side and `b_i` from the `m_i` side".
The three outcomes give three child states:

| outcome | meaning | child |
|---|---|---|
| `2` | both defectives in the taken set | `Sb(a_i : b_i)` |
| `0` | neither taken | `Sb((n_i - a_i) : (m_i - b_i))` |
| `1` | exactly one taken | `Sb(a_i : (m_i - b_i), (n_i - a_i) : b_i)` |

The mixed branch **doubles the number of parts**. That is the source of the combinatorial
explosion: a depth-`k` search over an initially one-part state can reach states with up to
`2^k` parts.

For `Sa(n)`, a test takes `c` of the `n` coins and yields `Sa(c)`, `Sb(c : n-c)`, and
`Sa(n-c)` for outcomes 2, 1, 0 respectively.

## Solvability

A state is **solvable in `k`** if some test exists whose three children are all solvable in
`k-1`. The base case is a state with at most one candidate pair. A **witness tree** is an
explicit record of such a strategy; see `witnesses/README.md`.

## The two quantities that are tabulated

- `Sa(k)` - the largest `n` with `Sa(n)` solvable in `k`. See `data/pareto_sa.csv`.
- `n(k,m)` - the largest `n1` with `Sb(n1 : m)` solvable in `k`. For fixed `k` these form a
  Pareto frontier over the two group sizes. See `data/pareto_sb.csv`.

Both are non-decreasing in `k`. `n(k,m)` is non-increasing in `m`, because `Sb(n:m+1)`
solvable implies `Sb(n:m)` solvable - the latter is a substate.

## The singleton base sequence

`G_k` is an explicit solvable singleton state that weakly majorizes every singleton state
solvable in `k` tests. It is written as a nonincreasing sequence of group sizes and defined by
`G_0 = (1)` and a three-way recurrence given in
[theorems/singleton-majorization.md](theorems/singleton-majorization.md).

```
G_0 = (1)
G_1 = (2, 1)
G_2 = (4, 3, 1, 1)
G_3 = (8, 7, 4, 4, 1, 1, 1, 1)
G_4 = (16, 15, 11, 11, 5, 5, 5, 5, 1 x 8)
G_5 = (32, 31, 26, 26, 16 x 4, 6 x 8, 1 x 16)
```

Entries come in dyadic blocks of sizes 1, 1, 2, 4, 8, ..., and the value in block `r` has
the closed form

```
G_k[block r] = sum_{i=0}^{k-r} C(k, i)
```

a partial sum of binomial coefficients. Block `0` gives `2^k`, block `1` gives `2^k - 1`,
block `2` gives `2^k - 1 - k`, and so on. This identity is verified for `k <= 12` and is
what `make_u_freq` in `radio_canon_search_generic.c` computes.

Naming convention used in the profile work: uppercase letters `A, B, C, D, ...` denote the
leading atom of each successive dyadic block of `G_k`, so `A = 2^k`, `B = 2^k - 1`,
`C = 2^k - 1 - k`, and a "profile" like `BBCD` means `B + B + C + D` evaluated in some
`G_{k-q}`.

## Why singleton states matter

Weak majorization by `G_k` is a proved necessary condition for singleton solvability, but it is
not sufficient.  The exact-support full-mass state
`(64,63,57^2,42^4,22^7,8^15,7^2,1^32)` is majorized by `G_6` and unsolvable in six tests; see the
[counterexample proof](../evidence/singleton_k6_counterexample_2026-08-30.md).  The important unconditional terminal
is narrower: any state whose rows fit into distinct rows of the explicit solvable state `G_k` is
solvable by deleting edges.  This is what makes a canonical witness tree—one whose every leaf is a
sub-multiset or distinct-slot substate of `G_k`—a self-contained proof rather than a solver
transcript.  A leaf certified only by arbitrary weak majorization is not a proof and needs a
separate exact strategy.
