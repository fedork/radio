# The Two-Part Reduction, and the Two-Part Frontier Theorem for `(2,1)`

Written up 2026-08-03. Two results, one trivial and one not.

The first says that the whole of `n(k,m)` is determined by the **two-part frontier** one
level down — so "characterise the two-part mixed frontier" is not one obstacle among
several, it is the entire remaining problem. The second characterises the first
non-singleton two-part family exactly, and reproves `n(k,3) = 2^k - k` as a corollary.

## The two-part frontier

For `s, t >= 1` and `k >= 0` define

> `g_k(s,t; p) = max { q : Sb(p:s, q:t) is solvable in k }`.

By Subgraph Monotonicity ([subgraph-monotonicity.md](subgraph-monotonicity.md)) the solvable
set is downward closed in `(p,q)`, so `g_k(s,t; ·)` is non-increasing and the solvable region
is exactly the region under a staircase. Measured staircases are in
[`../../data/pareto_2part.csv`](../../data/pareto_2part.csv).

## Theorem 1 (Two-Part Reduction Identity)

> For every `k >= 0` and `m >= 2`,
>
> ```
> n(k+1, m) = max        max { p + q : Sb(p:(m-b), q:b) solvable in k,
>            0 <= b <= m                p <= n(k,b),  q <= n(k,m-b) }
> ```
>
> where the `b = 0` and `b = m` terms degenerate to `2 · n(k,m)`.

### Proof

`Sb(n:m)` is solvable in `k+1` iff some first test works. A test on a one-part state is a
split `[a:b]`, and its three children are `Sb(a:b)`, `Sb((n-a):(m-b))` and the mixed child
`Sb(a:(m-b), (n-a):b)` ([../problem.md](../problem.md#what-one-test-does)). The first two are
**single parts**, so they are solvable in `k` iff `a <= n(k,b)` and `n-a <= n(k,m-b)`. Write
`p = a` and `q = n - a`; then `n = p + q` and the three conditions are exactly those in the
statement. Maximising over `a` and `b` gives the largest reachable `n`. ∎

Nothing here is deep. What it buys is a precise statement of where the difficulty lives:

- the outcome-2 and outcome-0 obligations are **one-part** facts, already tabulated;
- the whole of the remaining content is the **two-part** condition;
- and the two caps are **crossed** — `p` is the n-side of the `(m-b)` part in the mixed
  child but is capped by `n(k,b)`. That crossing is what makes the optimum sit off the
  corner of the two-part staircase.

### Verified

Recomputed from measured two-part staircases against `data/pareto_sb.csv`, which was produced
by a different route: **20 of 20 exact**, checked by `tools/check_tables.py` on every `(k,m)`
where the staircase table is complete and the one-part maxima are proven — `k=4→5` for
`m = 2..7`, `k=5→6` for `m = 2..10`, `k=6→7` for `m = 2..6`. Since the two sides come from
independent computations, the agreement cross-validates both tables. `tools/refsolve.py
check-reduction` reproduces it from the independent solver for `k <= 3`.

The optimum is usually attained by several `b`; the **smallest** one is `1` for `m = 2,3,4`,
`2` for `m = 5..8` and `3` for `m = 9,10`, identically at every `k` measured. `m = 6` is the
exception to the run of consecutive optima: only `b ∈ {2,4}` attain it, never `b = 3` — the
same gap recorded in the split windows in
[../conjectures.md](../conjectures.md#scalable-constructions-for-m5-and-m6-2026-08-03).

## Theorem 2 (Two-Part Frontier Theorem, family `(2,1)`)

> For every `k >= 1`, `Sb(p:2, q:1)` is solvable in `k` if and only if
>
> ```
> p <= 2^k - 1,      q <= 2^k,      p + q <= 2^(k+1) - k - 1.
> ```

Write `P_1(j) = 2^j`, `P_2(j) = 2^(j+1) - 1`, `P_3(j) = 3·2^j - j - 2` for the first three
prefix sums of the singleton base sequence `G_j`
([singleton-majorization.md](singleton-majorization.md)); these follow from the closed form
`G_j = (2^j, 2^j - 1, 2^j - 1 - j, ...)`.

### Necessity

`p <= n(k,2) = 2^k - 1` and `q <= n(k,1) = 2^k` are Subgraph Monotonicity plus Lemmas 1 and 3
([special-cases.md](special-cases.md)). For the sum, induct on `k`; the base `k=1` is the
three states of mass `<= 3`, checked directly.

Let a strategy solve `Sb(p:2, q:1)` in `k`, with first test splitting the `(p,2)` part by
`[a:b]` and the `(q,1)` part by `[c:d]`. Complementing the tested set swaps outcomes `0` and
`2` and fixes `1`, so `b=2` is the mirror of `b=0`; three cases remain.

**Case `b = 1`.** The `(p,2)` part becomes singletons in all three children, so every child is
a singleton state and the Singleton Majorization Theorem applies.

- `d = 1`: children are `{a, c}`, `{p-a}`, `{a, p-a, q-c}`. From the mixed child
  `a + (p-a) + (q-c) <= P_3(k-1)`; from the outcome-2 child `c <= P_1(k-1) = 2^(k-1)`. Hence
  `p + q = [a + (p-a) + (q-c)] + c <= P_3(k-1) + 2^(k-1) = 2^(k+1) - k - 1`.
- `d = 0`: children are `{a}`, `{p-a, q-c}`, `{a, p-a, c}`. From the mixed child
  `a + (p-a) + c <= P_3(k-1)`; from the outcome-0 child `q-c <= 2^(k-1)`. Same total.

**Case `b = 0`.** The `(p,2)` part stays a double.

- `d = 1`: children are `Sb(c:1)`, `Sb((p-a):2)`, `Sb(a:2, (q-c):1)`. The induction hypothesis
  on the mixed child gives `a + (q-c) <= 2^k - k`, and the other two give `p-a <= 2^(k-1) - 1`
  and `c <= 2^(k-1)`. Summing: `p + q <= (2^k - k) + (2^(k-1) - 1) + 2^(k-1) = 2^(k+1) - k - 1`.
- `d = 0`: children are nil, `Sb((p-a):2, (q-c):1)`, `Sb(a:2, c:1)`. The induction hypothesis
  applies to both, giving `p + q <= 2(2^k - k) = 2^(k+1) - 2k`, which is at most
  `2^(k+1) - k - 1` for `k >= 1`.

Every case yields the same bound, and it is attained, so it is exact. ∎

### Sufficiency

Take the `b = 1, d = 1` split, which is determined by `u = a`, `v = p - a`, `c` and
`w = q - c`. Its three children are the singleton states `{u,c}`, `{v}` and `{u,v,w}`, so by
Singleton Majorization it suffices to choose

```
c = min(q, 2^(k-1)),              w = q - c
u = min(p, 2^(k-1) - [c = 2^(k-1)]),  v = p - u
```

rebalancing `u, v` (swap, or move the excess) if `v > 2^(k-1)` or if `v + w > 2^k - 1`. The
sum constraint is what makes the mixed child fit: `u + v + w = p + q - c`, and when
`q >= 2^(k-1)` that is `p + q - 2^(k-1) <= P_3(k-1)` exactly when `p + q <= 2^(k+1) - k - 1`.

**Checked as arithmetic, not by search:** for all `k = 1..10` and all **1,399,926** pairs
`(p,q)` in the claimed region, this recipe yields a split whose three children are weakly
majorized by `G_{k-1}`. Zero failures.

### Verified

Exhaustively, by `tools/refsolve.py` — an independent solver sharing no code with
`radiobase.c` — for `k = 1..5`: 1,741 pairs, **0 disagreements**. Reproduce with
`tools/refsolve.py check-2part 5`. Confirmed at `k = 6` by the full staircase in
`data/pareto_2part.csv`, and at `k = 7` at the boundary: `Sb(127:2, 121:1)` is solvable
(`p+q = 248 = 2^8 - 8`) and `Sb(127:2, 122:1)` is not.

### Corollary: `n(k,3) = 2^k - k`

Apply Theorem 1 with `m = 3`. Only `b = 1` and its mirror `b = 2` are available, and
`b = 1` gives the mixed child `Sb(p:2, q:1)` with caps `p <= n(k,1) = 2^k` and
`q <= n(k,2) = 2^k - 1`. Theorem 2 then makes the binding constraint `p + q <= 2^(k+1) - k - 1`,
which the caps do not tighten for `k >= 1`. The degenerate `b = 0` route gives only
`2·n(k,3)`, which is smaller. Hence `n(k+1,3) = 2^(k+1) - (k+1)`. ∎

This is Lemma 6, already proved by a different argument in
[special-cases.md](special-cases.md#lemma-6). The point is not the result but that the route
through the two-part frontier reaches it — the same route is what `m = 4, 5, 6` need, and
there the one-part lemmas are still unproved.

## What the next family would buy

| family `(s,t)` | is the mixed child of | settles |
|---|---|---|
| `(2,1)` | `m = 3`, `b = 1` | `n(k,3)` — **done, above** |
| `(2,2)` | `m = 4`, `b = 2` | `n(k,4)` = Lemma 8, unproved |
| `(3,2)` | `m = 5`, `b = 2` | `n(k,5)` = Lemma 9, unproved |
| `(4,2)` | `m = 6`, `b = 2` | `n(k,6)` = Lemma 10, unproved |

Measurements and conjectured forms for the last three are in
[../conjectures.md](../conjectures.md#the-two-part-frontier-programme-2026-08-03).
