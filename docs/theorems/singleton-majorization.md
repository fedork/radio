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
