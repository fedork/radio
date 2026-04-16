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

