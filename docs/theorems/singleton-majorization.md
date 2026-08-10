# Singleton Majorization Theorem

## Recommended names

### Theorem name
**Singleton Majorization Theorem**

This is the main statement:

> A singleton state `Sb(a1:1, a2:1, ..., an:1)` with `a1 >= a2 >= ...` is solvable in `K` tests if and only if the sequence `(a1,a2,...)` is weakly majorized by the singleton base sequence `G_K`.

### Key lemma name
**Three-Way Majorization Decomposition Lemma**

This is the structural lemma used in the induction step. It is the right named object to cite later.

An alternative shorter name would be **Base-Sequence Splitting Lemma**, but the first name is more precise.

---

## Definitions

We write all sequences in nonincreasing order and pad them with trailing zeros when needed.

### Weak majorization
For two nonincreasing sequences

`x = (x1, x2, ...)`, `y = (y1, y2, ...)`,

we write

`x <=_w y`

if for every `t >= 1`,

`sum_{i=1..t} xi <= sum_{i=1..t} yi`.

### Singleton base sequence
Define `G_0 = (1)`.

For `K >= 1`, if

`G_{K-1} = (h1 >= h2 >= ... >= hm)`,

then define three zero-padded sequences of length `2m`:

- `L_K = (h1, 0, h2, 0, ..., hm, 0)`
- `M_K = (h1, h2, ..., hm, 0, ..., 0)`
- `R_K = (0, h1, 0, h2, ..., 0, hm)`

and define

`G_K := sort(L_K + M_K + R_K)`

where the sum is coordinatewise and `sort` means reorder in nonincreasing order.

This gives:

- `G_0 = (1)`
- `G_1 = (2, 1)`
- `G_2 = (4, 3, 1, 1)`
- `G_3 = (8, 7, 4, 4, 1, 1, 1, 1)`

etc.

---

## Theorem

Let

`a = (a1 >= a2 >= ... >= an > 0)`.

Then the singleton state

`Sb(a1:1, a2:1, ..., an:1)`

is solvable in `K` tests if and only if

`a <=_w G_K`.

---

## Three-Way Majorization Decomposition Lemma

Let `u, v, w` be finite nonnegative sequences. Then

`x^↓ <=_w (u + v + w)^↓`

if and only if there exist nonnegative vectors `x^(1), x^(2), x^(3)` of the same length as `x` such that

- `x = x^(1) + x^(2) + x^(3)` coordinatewise,
- `(x^(1))^↓ <=_w u`,
- `(x^(2))^↓ <=_w v`,
- `(x^(3))^↓ <=_w w`.

### Note
This is the standard symmetric-polymatroid decomposition fact behind the induction step.

---

## Proof of the Singleton Majorization Theorem

We proceed by induction on `K`.

### Base case: `K = 0`

The only nonzero singleton state solvable in `0` tests is `Sb(1:1)`.

Since `G_0 = (1)`, the condition `a <=_w G_0` means

- `a1 <= 1`
- `a1 + a2 <= 1`
- `a1 + a2 + a3 <= 1`
- etc.

Hence the only nonzero possibility is `a = (1)`.

So the theorem holds for `K = 0`.

### Induction step

Assume the theorem holds for `K - 1`. We prove it for `K`.

#### Sufficiency
Assume

`a <=_w G_K`.

By definition,

`G_K = sort(L_K + M_K + R_K)`.

Applying the Three-Way Majorization Decomposition Lemma with `u = L_K`, `v = M_K`, `w = R_K`, there exist nonnegative vectors

- `l = (li)`
- `m = (mi)`
- `r = (ri)`

such that

- `ai = li + mi + ri` for each `i`,
- `l^↓ <=_w L_K`,
- `m^↓ <=_w M_K`,
- `r^↓ <=_w R_K`.

After deleting zeros and re-sorting, each of `L_K, M_K, R_K` becomes exactly `G_{K-1}`. Therefore

- `l^↓ <=_w G_{K-1}`
- `m^↓ <=_w G_{K-1}`
- `r^↓ <=_w G_{K-1}`.

Now define one test on the singleton state row by row as follows:

- split row `i` into three consecutive parts of sizes `li`, `mi`, `ri`
- test the union of the left and mixed parts

Then the three possible outcomes produce exactly the three singleton child states

- left child: `l^↓`
- mixed child: `m^↓`
- right child: `r^↓`

Each is weakly majorized by `G_{K-1}`, so by the induction hypothesis each is solvable in `K - 1` tests.

Therefore the parent state is solvable in `K` tests.

#### Necessity
Assume now that

`Sb(a1:1, ..., an:1)`

is solvable in `K` tests.

Take its first test. For each row `i`, let

- `li` be the number of coins that go to the left child,
- `mi` the number of coins that go to the mixed child,
- `ri` the number of coins that go to the right child.

Then

`ai = li + mi + ri`

for every `i`.

Let

- `L = l^↓`
- `M = m^↓`
- `R = r^↓`

These are exactly the three child singleton states.

Since the parent is solvable in `K` tests, each child is solvable in `K - 1` tests. By the induction hypothesis,

- `L <=_w G_{K-1}`
- `M <=_w G_{K-1}`
- `R <=_w G_{K-1}`.

Equivalently, after zero-padding to the common length,

- `L <=_w L_K`
- `M <=_w M_K`
- `R <=_w R_K`.

Applying the Three-Way Majorization Decomposition Lemma in the forward direction to

`a = l + m + r`,

we obtain

`a^↓ <=_w sort(L_K + M_K + R_K) = G_K`.

Since `a` was already nonincreasing,

`a <=_w G_K`.

This proves necessity.

Thus both directions hold for all `K >= 0`, and the theorem follows.

---

## Corollary

For each `K`, the solvable singleton `Sb` states are exactly the finite nonincreasing sequences weakly majorized by `G_K`.

In particular, `G_K` is the unique maximal solvable singleton sequence for `K` tests.

---

## Status note

This proof is clean and project-usable provided we accept the Three-Way Majorization Decomposition Lemma as a standard polymatroid/majorization decomposition fact.

If later needed, this note can be extended with a standalone proof of that lemma or a precise external citation.


---

## Closed form for `G_k`

The recurrence above is equivalent to an explicit formula. Entries of `G_k` come in dyadic
blocks of sizes 1, 1, 2, 4, 8, ..., and every entry in block `r` (zero-indexed) equals a
partial sum of binomial coefficients:

```
G_k[block r] = sum_{i=0}^{k-r} C(k, i)
```

So block 0 is `2^k`, block 1 is `2^k - 1`, block 2 is `2^k - 1 - k`, block 3 is
`2^k - 1 - k(k+1)/2`, and so on. Verified against the recurrence for `k <= 12`. This is what
`make_u_freq` computes in `radio_canon_search_generic.c:121`, and it makes the theorem's
criterion fully explicit: no recursion is needed to test a candidate singleton state.

## Consequence used throughout this project

If the parts of a singleton state form a **sub-multiset** of `G_k`, its prefix sums are
dominated termwise by those of `G_k`, so it is weakly majorized by `G_k` and hence solvable
in `k`. A witness tree all of whose leaves are such states is therefore a complete proof,
independent of the solver. `tools/check_witness.py` checks exactly this condition.

## Vertex-Splitting Pullback Lemma (2026-08-09)

Subgraph Monotonicity is not the only way to transfer a strategy between graphs. A graph may also be
made easier by **splitting a vertex into clones**.

> Let `H` and `G` be graphs and let `pi: V(H) -> V(G)` be a vertex map whose induced map on edges is
> injective: distinct edges `{x,y}` of `H` have distinct images `{pi(x),pi(y)}` in `G`. If `G` is
> solvable in `k` tests, then `H` is solvable in `k` tests.

*Proof.* At every node of a strategy for `G`, replace its tested vertex set `T` by the preimage
`pi^-1(T)`. An edge of `H` then gives exactly the same `0/1/2` response as its image edge in `G`, at
this node and recursively at every node it reaches. The strategy for `G` assigns distinct transcripts
to distinct image edges. Edge-injectivity therefore gives distinct transcripts to distinct edges of
`H`. ∎

The vertex map need not be injective: several clones in `H` may map to one vertex of `G`. This is why
the lemma is not a restatement of Subgraph Monotonicity.

## Corollary: full star-expansion majorization

Orient every part so `n_i >= m_i`. Define the **full star expansion**

```
Phi({(n_i:m_i)}_i) = ( n_1 repeated m_1 times,
                        n_2 repeated m_2 times, ... )^downarrow.
```

Equivalently, replace each `K_{n_i,m_i}` by `m_i` vertex-disjoint copies of `K_{n_i,1}`.

> If `Sb(n_1:m_1, ..., n_p:m_p)` is solvable in `k`, then
>
> `Phi({(n_i:m_i)}_i) <=_w G_k`.

*Proof.* For one `K_{n,m}`, name its sides `x_1,...,x_n` and `y_1,...,y_m`. In the expanded graph,
star `j` has centre `y_j` and leaves `x_(1,j),...,x_(n,j)`. Map `y_j` to `y_j` and every clone
`x_(i,j)` to `x_i`. This maps the edge `{x_(i,j),y_j}` bijectively to the original edge
`{x_i,y_j}`. Taking the disjoint union of these maps over all parts satisfies the pullback lemma, so
solvability of the original state implies solvability of its all-singleton expansion. The Singleton
Majorization Theorem now gives the displayed condition. ∎

This supersedes the 2026-08-06 one-copy downgrade `(n:m) -> (n:1)`. The full expansion contains that
sequence and another `m-1` copies of `n`, so every old violation remains a new violation; unlike the
old downgrade, it also preserves the full mass `sum n_i m_i`.

The choice of orientation is strongest. Expanding the other shore gives `n` copies of `m`; for
`n >= m`, that flatter equal-mass sequence is weakly majorized by `m` copies of `n`, so it cannot add
a violation after the displayed test passes.

In fact this is the strongest edge-bijective singleton lift of one part. The centre of any lifted
star maps to one original vertex, so its degree is at most `n`; the lifted stars must carry all `nm`
distinct edges. Among sequences with total `nm` and largest entry at most `n`, the sequence of `m`
copies of `n` weakly majorizes every other one. Any stronger structural condition must therefore
retain information that ordinary singleton majorization forgets, such as the requirement that clones
of one original vertex be tested together.

### The tail must be clamped, not treated as a violation

Sequences are **zero-padded**, so for `t > len(G_k) = 2^k` the right-hand side is the constant
`sum G_k = 3^k`. The left-hand side is at most the state's mass, which the counting bound has already
bounded by `3^k`. So **no violation can arise past `len(G_k)`**, and code that reports one there
over-refutes.

This matters specifically because of the corollary: full star expansion routinely has more than
`2^k` entries. `radio_verify.c` once had exactly this defect, and it fired **79 times** in a single
k=4 level even under the weaker one-copy downgrade. `radiobase.c` was already correct — it stops the
comparison at `min(size, len(G_k))`, which is equivalent when the counting bound has passed.

## The synchronized-majorization hierarchy (2026-08-09)

Full star expansion forgets only one thing: the cloned rows belonging to one rectangle cannot choose
their tests independently. That missing constraint can be restored one test at a time.

For a state `S` and `0 <= d <= k`, define the Boolean relaxation `R_d(S,k)` as follows.

- `R_0(S,k)` means `Phi(S) <=_w G_k`, including the zero-padded total-mass inequality.
- `R_d(S,k)` for `d>0` means that there is one legal synchronized rectangle split of `S` for which
  all three children `S_0,S_1,S_2` satisfy `R_{d-1}(S_j,k-1)`.

Thus `R_0` is the deployed static condition. `R_1` insists that its independent singleton
strategies can at least share one legal first test; every further level postpones the singleton
relaxation by one more synchronized test.

> **Synchronized Hierarchy Theorem.** For `0 <= d < k`,
>
> `solvable(S,k)  =>  R_{d+1}(S,k)  =>  R_d(S,k)`.
>
> At the final level, `R_k(S,k)` is equivalent to exact solvability in `k` tests.

*Proof.* The first implication follows by induction on `d`. Use the real strategy's first test; each
of its children is solvable in `k-1`, so it satisfies the preceding relaxation level.

For the base nesting step `R_1 => R_0`, apply the witnessing rectangle test to the row-star lift of
`S`. Its three children are singleton states. For a child rectangle `(u:v)`, this inherited row
orientation contributes `v` copies of `u`; the strongest orientation `Phi` contributes
`min(u,v)` copies of `max(u,v)` and weakly majorizes the inherited one. The same is true after
concatenating all child rectangles. `R_0` for the rectangle child therefore supplies a singleton
strategy for the inherited child. Combining the three strategies below the common first test solves
the row-star lift of `S`, and the Singleton Majorization Theorem gives `R_0(S,k)`.

Now induct on `d`: a witness for `R_{d+1}` has children satisfying `R_d`; by the preceding nesting
level those same children satisfy `R_{d-1}`, so the same first split witnesses `R_d`.

Finally, an `R_k` witness recursively supplies a legal split at all `k` levels. At depth `k`, each
leaf satisfies `R_0(S',0)`. Since `G_0=(1)`, such a leaf contains at most one possible defective
pair and needs no further test. The recursive witnesses are therefore an exact strategy. The reverse
direction was the first implication. ∎

Every `R_d` is also subgraph-monotone: restrict its witnessing splits to the subgraph and induct on
`d`. This justifies rejecting a partial split prefix as soon as one of its child prefixes fails the
required relaxation.

### Additive hinge form of `R_1`

For a nonnegative sequence `x`, write

    H_x(t) = sum_j max(x_j-t, 0).

Weak majorization `x <=_w y` is equivalent to `H_x(t) <= H_y(t)` for every `t>=0`. For integer
sequences it is enough to check integer thresholds. Define `C_k(t)=H_{G_k}(t)` and

    h_t(u,v) = min(u,v) max(max(u,v)-t, 0).

The quantity `h_t(u,v)` is exactly the hinge contribution of the full star expansion of one
rectangle `(u:v)`. If `(n:m)` is split by selecting `(a:b)`, its contributions to the three children
are

    outcome 2: h_t(a,b)
    outcome 0: h_t(n-a,m-b)
    outcome 1: h_t(a,m-b) + h_t(n-a,b).

Consequently, `R_1(S,k)` is exactly a multiple-choice integer feasibility problem: choose one
`(a_i:b_i)` for every parent part so that, for every outcome and threshold, the sum of these
contributions is at most `C_{k-1}(t)`. This formulation is additive and requires no sorting. Threshold
`t=0` is the ordinary three-child counting bound; the positive thresholds retain the shape
information that counting loses.

### Worked residual: `Sb(16:1,12:2)` in four tests

The static profile is `(16,12,12)`. It passes `R_0` because its nontrivial prefix sums
`16,28,40` are bounded by the first three prefix sums `16,31,42` of `G_4`.

For `R_1`, choose the synchronized split `[8:0,7:2]`. Its children are

    outcome 2: Sb(7:2)
    outcome 0: Sb(8:1)
    outcome 1: Sb(8:1,5:2).

Their full-star profiles `(7,7)`, `(8)`, and `(8,5,5)` all lie below `G_3`, so this witnesses
`R_1`. Each child in turn has an `R_1` continuation, so the same first split also witnesses `R_2`.
But `R_3` fails: three synchronized layers are already enough to expose the obstruction, one level
before the exact `R_4` test. Reproduce all five verdicts with

```
tools/bundled_majorization.py ladder 4 16 1 12 2
```

### Why there is no single width-two base sequence

Even if every part has width two, solvability is not downward closed under ordinary weak
majorization. The independent reference solver gives

```
tools/refsolve.py solve 4 12 2 10 2 9 2 3 2   # solvable
tools/refsolve.py solve 4 11 2 11 2 9 2 3 2   # unsolvable
```

Yet the unsolvable state's full-star profile

    (11,11,11,11,9,9,3,3)

is weakly majorized by the solvable state's profile

    (12,12,10,10,9,9,3,3):

their prefix sums are respectively `(11,22,33,44,53,62,65,68)` and
`(12,24,34,44,53,62,65,68)`. Therefore the exact width-two solvable set is not a majorization ideal.
No criterion obtained by replacing `G_k` with one fixed width-two base sequence can be exact; the
discrete synchronization choices, retained by the hierarchy, are essential.
