# Open predictions

This page tracks open predictions and the experiments that refute them.  A prediction you cannot
falsify is not worth recording; once a row is settled, its fact moves to `data/*.csv` and the failed
model remains here so it is not rediscovered.

The formulas are stored executably in `data/conjectures.csv`; `tools/check_tables.py`
re-evaluates each one against every proven datum on every run. Proof status per lemma is in
[theorems/special-cases.md](theorems/special-cases.md).

## Two models of the frontier

**Closed forms.** `n(k,m) = c * 2^k + (quadratic in k)`, one row per `m`. See the generated
table in [theorems/special-cases.md](theorems/special-cases.md).

**Dyadic profiles.** `n(k,m)` equals a fixed multiset of atoms drawn from the dyadic blocks
of `G_{k-q}`, for an `m`-dependent offset `q`. Written as letter strings where `A` is the
first dyadic block of `G_{k-q}`, `B` the second, and so on:

| m | profile | q |
|---|---|---|
| 1 | `A` | 0 |
| 2 | `B` | 0 |
| 3 | `AC` | 1 |
| 4 | `AACC` | 2 |
| 5 | `BBBD` | 2 |
| 6 | ~~`BBCD`~~ **refuted at k=10** | 2 |
| 7 | `ABBBBCDD` | 3 |
| 8 | `AAACCCDD` | 3 |
| 9 | `AAAAAABBCCCCCCDE` | 4 |
| 10 | `AAAAABBCCCCCCCEE` | 4 |

Profile length is constrained to a power of two and the alphabet to the first `m` dyadic
slots. For `m = 11` the shortest exact fit found needs length 64 at `q = 5`, and the string
was not recorded - a gap worth closing, since it is the only thing blocking an independent
check of `n(9,11)`.

Both models reproduce **every** proven value for `k = 1..8`, `m = 1..10`.  They are no longer
globally viable models: both give 976 from their `m=6` row at `k=10`, while the exact maximum is
973.  The disagreement below remains useful for distinguishing their still-open `m=9,10` rows,
but neither model should be extrapolated wholesale.

## The remaining m=9,10 discriminating experiment

The two models agree everywhere data exists and diverge at `k = 9`:

| m | closed form | dyadic profile | | |
|---|---|---|---|---|
| 1-8 | 512, 511, 503, 496, 480, 473, 457, 447 | identical | agree | |
| **9** | **431** | **432** | disagree by 1 | |
| **10** | **414** | **416** | disagree by 2 | |

So:

> **If `Sb(432 : 9)` is solvable in 9 tests, the closed-form model is dead.**

The test is asymmetric and that matters for how to spend compute:

- **Finding** a canonical witness tree for `Sb(432:9)` proves the profile model right
  outright, via the Singleton Majorization Theorem, with no solver trust required. This is
  cheap - the same tool already produced trees for `496:4`, `480:5` and `473:6` at `k=9`.
- **Failing** to find one proves nothing. Ruling out 432 requires exhaustive search, which
  is expensive.

`m = 10` gives an independent replication of the same question: 416 versus 414.

Run it with:

```
./run_radio_canon_search_generic.sh 4 9 432 9
./run_radio_canon_search_generic.sh 4 9 416 10
```

Note that earlier `k=9` probes used `radio_full`, which enumerates *every* top-level split -
far more work than needed here. Reach for the canonical search first; see
[tools.md](tools.md).

## Unverified predictions for k = 9

Where the models agree, they still predict:

| m | 7 | 8 |
|---|---|---|
| n(9,m) | 457 | 447 |

Both are natural canonical-search targets and would extend the artifact-backed part of the
`k=9` column from `m <= 6` to `m <= 8`:

```
./run_radio_canon_search_generic.sh 4 9 457 7
./run_radio_canon_search_generic.sh 4 9 447 8
```

## Conjecture (u1) - the antidiagonal conjecture

> If `Sb(n1 : n2)` is solvable in `k` for `n1 >= n2`, then so is `Sb((n1+1) : (n2-1))`.

Equivalently `n(k, m-1) >= n(k, m) + 1`: the frontier decreases **strictly** in `m`. Formal
statement in [theorems/special-cases.md](theorems/special-cases.md#conjecture-u1). Still
unproved, but 2026-08-03 mapped it out: the evidence is much stronger than the 130 cells,
two natural proof routes are now **refuted**, and what is left is a single crisp lemma.

### Why it is worth proving

`Sa(193)` in 10 reduces to sixteen states `Sb(n1 : 193-n1)`, `n1 = 97..112`
([results.md](results.md#sa10-192-achievable-maximality-not-established)). They are pairwise
incomparable under subgraph monotonicity, so all sixteen need refuting - about 47 days of
2023 compute, of which the fifteen easier ones were 58%. Under (u1) they chain: refuting the
most lopsided `Sb(112:81)` refutes the other fifteen, and the negative certificate shrinks
16-fold.

### Evidence

- All 130 proven cells, `k = 1..8`, no violations - checked by `tools/check_tables.py`.
- Exhaustive over **every** one-part state, not just frontier cells, for `k <= 5`:
  `tools/refsolve.py check-c 5`.
- `tools/refsolve.py` is an independent second implementation written from
  [problem.md](problem.md) alone; it reproduces the proven columns for `k = 1..6` exactly.

### It is not an instance of subgraph monotonicity

In the graph reformulation
([theorems/subgraph-monotonicity.md](theorems/subgraph-monotonicity.md)) both states live on
the same `n1 + n2` vertices: moving one coin `z` from the small side to the large side
deletes the `n1` edges from `z` to the large side and adds `n1_new - 1 = n2 - 1` edges from
`z` to the rest of the small side. Since `n1 >= n2`, mass strictly drops - but neither graph
contains the other, so the theorem does not apply and no amount of dominance bookkeeping
will produce (u1).

### Refuted: the multi-part generalisation (2026-08-03)

The obvious induction needs (u1) to hold for a part *inside* a larger state, because the
mixed child of any test has two parts. It does not.

```
Sb(15:2, 5:4)  mass 50   can solve   in 4   (split [8:1, 1:1])
Sb(15:2, 6:3)  mass 48   can't solve in 4
```

The move is intra-part with `n >= m`, and mass *decreases*. Confirmed by two independent
solvers; reproduce in under a second with either:

```
tools/refsolve.py solve 4 15 2 5 4        # can solve
tools/refsolve.py solve 4 15 2 6 3        # can't solve
clang -O3 -DMAX_K=4 -DMAX_N=40 radio_one.c -o /tmp/r4 && /tmp/r4 4 15 2 6 3
```

**Consequence, and the main structural finding:** no induction on `k` that rewrites a
strategy part by part can prove (u1). Whatever proof exists must treat the one-part state as
a whole. This also makes the pair a good regression case for the solver.

### Refuted: any mass-based move lemma

The natural framework - "moving one coin between parts preserves solvability whenever mass
does not increase" - is false even with *strict* decrease:

```
Sb(8:1, 2:1)  mass 10   solvable in 3
Sb(9:1)       mass  9   not solvable in 3
```

obtained by moving a coin off the `1`-side of `(2:1)` (losing 2 edges) onto the `8`-side of
`(8:1)` (gaining 1). Both states are singleton states, so this rests on the **Singleton
Majorization Theorem**, not on a solver run: `(8,2)` has prefix sums `8, 10` against
`G_3 = (8,7,4,4,1,1,1,1)` and passes; `(9)` fails at `9 > 8`. So there is no potential
function of that shape, and (u1) is not the shadow of a more general monotonicity.

### What remains: the Extremal Split Lemma

Only two edits of a winning split `(p,q)` of `(a:b)` leave one of the outer children
untouched, according to whether the moved coin `z` was in the test:

| | split of `(a+1 : b-1)` | outcome-0 child | outcome-2 child | mixed child |
|---|---|---|---|---|
| **A** (`z` taken) | `(p+1, q-1)` | `= C_0`, free | `(p+1 : q-1)` | `{(p+1 : b-q), (a-p : q-1)}` |
| **B** (`z` untaken) | `(p, q)` | `(a+1-p : b-1-q)` | `= C_2`, free | `{(p : b-1-q), (a+1-p : q)}` |

These are one statement, not two: edit A on `s` yields the complement, within the target, of
edit B on the complement of `s`, and complementary splits have the same three children with
outcomes 0 and 2 swapped. Under complementation `p-q` maps to `(a-b) - (p-q)`.

> **Extremal Split Lemma (conjectured).** Let `a >= b >= 2` and let `Sb(a:b)` be solvable in
> `k`. Among all winning splits let `(p*, q*)` minimise `p - q`. Then `(p*+1, q*-1)` is a
> winning split of `Sb(a+1 : b-1)`.

Verified by `tools/refsolve.py check-extremal 5`: **187 of 187** solvable states with
`b >= 2` at `k = 2..5`, no exceptions, including every awkward diagonal case. A `k=6`
spot check over the seven states below each of `n(6,2)=63`, `n(6,3)=58`, `n(6,4)=54`,
`n(6,5)=50` adds **28 more**, also without exception. It has not been tested above `k=6`;
`radio_full`'s `all_solutions` is the cheap way to push it to `k=7,8`, since that enumerates
exactly the winning-split set the lemma quantifies over.

Extremality is essential, not cosmetic. At `(10:2)` in 4 the winner `(8,1)` - maximal
`p - q` - fails: edit A gives mixed child `{(9:1)}`, unsolvable in 3 because `n(3,1) = 8`.
The minimal winners `(2,1)` and `(3,2)` both survive. Selecting by `min p` scores equally
well; `min q`, `max p` and the mass-based selectors all fail.

### Where the proof gets stuck

With `(p,q)` minimal, `C_0` is unchanged, leaving two obligations:

1. `(p+1 : q-1)` solvable in `k-1`. This is (u1) at `k-1` **when `p >= q`** - but the
   minimiser frequently has `p < q`. At `(11:11)` in 5 it is `(4,6)`, so the outcome-2 child
   goes `(6:4) -> (5:5)` and mass *increases*, 24 to 25.
2. `{(p+1 : b-q), (a-p : q-1)}` solvable in `k-1`. This is a cross-part coin move, which the
   refutation above says has no general justification.

So both surviving obligations need the extremality hypothesis in an essential way. The
natural finish is an exchange argument - assume the minimal winner fails and construct a
winner with smaller `p - q` - and it has not been found. The one handle available is that
minimality makes `(p-1, q)` a non-winner, and since `(p-1 : q)` is a subgraph of `(p : q)`
the failure must lie in its outcome-0 or mixed child.

### A frame that may be the right one

Give each coin `c` a bit at every node, `c_v = [c in S_v]`. The leaf reached by the pair
`{x,y}` is the string `r` with `r_i = x_{v_i} + y_{v_i}`. Then coin `c`'s reachable subtree
is **binary** - at node `v` it uses children `c_v` and `c_v + 1` - which gives `n1 <= 2^k` in
one line, and since leaves used by distinct small-side coins must be disjoint, `n1*n2 <= 3^k`.
This is the same object as the profile mechanism derived below from the witness trees, found
independently from the other direction, which is mild evidence it is the right one. In this
frame (u1) is a relabelling question rather than a tree-surgery question.

## The Sa growth ratio

`Sa(k) / Sa(k-1)` hovers just under `sqrt(3) ~ 1.7320`: the observed ratios are 1.5, 1.667,
1.6, 1.625, 1.692, 1.727, 1.711, 1.723, 1.714. The model `Sa(k) = floor(Sa(k-1) * sqrt(3))`
reproduces 2, 3, 5, 8, 13, 22, 38, 65, 112 and then predicts **193** at `k = 10`, one more
than the 192 that is actually constructed.

This is not evidence that 193 is achievable - the model is a heuristic, and the exact
recurrence is `Sa(k) = max over n1 <= Sa(k-1) of n1 + n2max(n1)` where `n2max(n1)` is the
`K = k-1` frontier at `n1`. But it does say the question is genuinely open rather than
obviously settled, and it identifies the 16 states listed in
[results.md](results.md#why-sa193-is-a-small-question).

## The profile has a mechanism (derived 2026-08-03)

This was a numerical fit. It is now derived from the verified witness trees, by
`tools/profile_from_tree.py`.

**The mechanism.** Fix one m-side coin `y` and a normalisation level `t`. Over the `k-t`
tests above level `t`, `y` is either inside the tested set or outside it — so `y` has exactly
`2^(k-t)` root-to-level-`t` paths. Each path ends holding one chunk of the n-side, and that
chunk must be resolvable in the `t` remaining tests while paired with `y`, so its size is an
atom of `G_t`.

> **The profile is the multiset of chunk sizes along one coin's `2^(k-t)` paths.**

Everything that looked arbitrary now follows:

- **`length = 2^q` with `q = k-t`** — it counts binary paths. Not a coincidence.
- **Refinement invariance** — lowering `t` by one doubles the paths, so `L/2^q` is fixed.
  `q` is not intrinsic; the refinement class is.
- **The atoms come from `G_t`** — forced by the terminal chunk being solvable in `t`.
- **The whole tree's census at level `t` is `m` copies of the profile**, so every count is
  divisible by `m` and the total is exactly `m · 2^(k-t)`.

### Verified

| tree | m | solutions | carry the profile |
|---|---|---|---|
| `canon_248_3_at8` | 3 | 2 | 2 — matches `AC` |
| `canon_496_4_at9` | 4 | 2 | 2 — matches `AACC` |
| `canon_480_5_at9` | 5 | 9 | 7 — matches `BBBD` |
| `canon_473_6_at9` | 6 | 1 | **0** |

For `Sb(480:5)@9` the per-coin profile reads `9x32 + 4x31 + 2x26 + 1x16 = 480` at level 5,
16 atoms `= 2^4`. The whole-tree census is `32:45, 31:20, 26:10, 16:5`, 80 cells — which is
exactly the `480:5 @ U_5` benchmark recorded independently in [journal.md](journal.md),
computed there without the connection being made.

### Two ways a real solution fails to have a profile, both observed

- **Empty paths.** A path where `y` ends with no n-side coins. The atom count then falls short
  of `m · 2^(k-t)`. `Sb(473:6)@9` wastes **7** slots.
- **Asymmetry.** The census is not divisible by `m`, so the coins do not share one
  decomposition. This is precisely the "multiple-of-m atom-count sanity check" the journal
  records as failing for 473:6 — the check was right and now has a reason.

So a profile describes a **symmetric, non-wasteful** solution. Most solutions are; not all are.

### This answers the journal's headline open question, negatively

> *"Does the canonical 473:6 @9 witness correspond to a genuinely scalable compact atomic
> decomposition family, or only to a tree/state-level artifact?"*

**That witness does not.** It is asymmetric and wastes 7 paths, so it does not exhibit the m=6
profile at all. Open: whether a symmetric `Sb(473:6)@9` solution exists. Worth a targeted
search, since `canon_473_6_at9` is the only one we have and 480:5 shows asymmetric and
symmetric solutions coexisting for the same state.

### What this does to the open question

`q(m)` is no longer an arbitrary fit parameter. Since `q = k - t`, the question becomes:
**how deep must a solution go before every leaf is a singleton?** That is a property of the
tree, and a far more tractable target than an integer pulled out of a curve fit.

## n-side splits: must both parts be non-nil? (settled 2026-08-03)

Splitting a part `n:m` by `[a:b]` yields `a:b` / `(n-a):(m-b)` / `a:(m-b)` + `(n-a):b`. The
question is whether an optimal solution ever leaves the **wide** side whole — `a = 0` or
`a = n`, the journal's *left-column-only / right-column-only* support, which it provisionally
excluded.

### For a single-part state: no, and there is a reason

Among splits that leave one side whole, both routes have an **exactly achievable** bound,
because in each case the surviving children are single parts:

| route | children (outcome 2 / 1) | best n |
|---|---|---|
| narrow whole, `b = m` | `a:m` and `(n-a):m` | `n = 2 · n(k-1, m)` |
| wide whole, `a = n` | `n:b` and `n:(m-b)` | `n = n(k-1, ⌈m/2⌉)` |

Since both are tight, comparing them is conclusive. `2·n(k-1,m) > n(k-1,⌈m/2⌉)` on **all 76
checkable `(k,m)` pairs**, so leaving the wide side whole is strictly dominated — it caps `n`
at roughly `2^(k-1)` where the other route reaches nearly `2^k`.

The margin is not uniform. It is `2.00` at `m=1` and falls to `1.09` at `k=6, m=10`, i.e. it
degrades toward the diagonal. So this is *verified*, not proved: a crossing at larger `m`
than the table reaches is not excluded.

### Empirically

| source | entries | wide side left whole |
|---|---|---|
| canonical / atomic trees (frontier states) | 425 | **0** |
| numbered `Sa` witnesses | 2226 | 34 (1.5%) |
| …of those, in a **single-part** state | — | **0** |

All 34 sit inside multi-part states, and all are near-square: median ratio `n/m = 1.12`,
none above `2.0`, the largest part being `20:10`. Against a population whose median ratio is
`2.0` and only 29% of which is below `1.5` — so they are 91% concentrated in the corner where
"wide" and "narrow" barely differ.

### Answer

- **Single-part states:** you may assume both n-side parts are non-nil. Never observed
  otherwise, and dominated by the argument above.
- **Atomic Pareto matrices:** likewise — 0 of 425. The journal's exclusion of
  left/right-column-only supports is confirmed for exactly the objects it is stated about.
- **Multi-part states in general:** no, you cannot assume it. It happens, rarely, and only for
  near-square parts — where routing across the other parts changes the trade-off. The
  degenerate corner is the same one the stabilisation doctrine already sets aside, and the same
  one that produced the `Sb(11:11)` anomaly in the m=11 profile.

Reproduce with the classification in [journal.md](journal.md#2026-08-03--n-side-splits); the
comparison uses only `data/pareto_sb.csv`.

## Finite m5m6 recurrences and the k10 break (2026-08-10)

Full split enumerations of the frontier states, from `radio_full`:

| state | k | working splits |
|---|---|---|
| `Sb(22:5)` | 5 | `b in {2,3}`, `a in [10,12]` |
| `Sb(50:5)` | 6 | `b in {2,3}`, `a in [23,27]` |
| `Sb(109:5)` | 7 | `b in {2,3}`, `a in [51,58]` |
| `Sb(19:6)` | 5 | `b in {2,3,4}`, `a in [7,12]` |
| `Sb(46:6)` | 6 | `b in {2,4}`, `a in [22,24]` |
| `Sb(104:6)` | 7 | `b in {2,4}`, `a in [50,54]` |

The `a`-windows are exactly `[n - n(k-1,3), n(k-1,3)]` for m=5, so the m=3 child is the
binding single-part constraint, and the window is nonempty because `n(k,5) <= 2·n(k-1,3)`.

**Two numerical identities, exact on the then-available range `k=5..9`:**

    n(k,5) = n(k-1,2) + n(k-1,6)
    n(k,6) = n(k-1,4) + n(k-1,5)

On that finite range the second is realised directly by a split: `b = 2`,
`a = n(k-1,5)`, giving
`n(k-1,5):2` on outcome 2 and `n(k-1,4):4` **saturated** on outcome 0. The `473:6@9` witness
uses the mirror form `[242:4]`, i.e. `a = n(8,4) = 242`, `n-a = n(8,5) = 231`.

The first is a numerical identity only — `m=6 > 5` cannot appear as a child of an m=5 part, and
the split that would saturate it (`a = n(k-1,2)`) is *not* in the working set, so it is not
realised structurally.

**2026-08-10 correction.**  The apparent `m=6` recursion fails at its first extrapolation.  It
predicts `496+480=976` at `k=10`, but exact synchronized search proves `n(10,6)=973`.  A verified
tree uses root `[477:2]`, with children

    Sb(477:2),  Sb(496:2,477:4),  Sb(496:4).

Thus the `m=4` pure child remains saturated while the other width retreats by three.  The old
continuation through `Sb(496:2,480:4)` reaches the impossible `Z_7` kernel; the new mixed state
avoids it.  This establishes a break, **not** the replacement formula
`n(k-1,4)+n(k-1,5)-3`.  Its natural `k=11` lift reduces to
`Sb(503:1,495:2,478:3)@9`; a five-minute exact run was inconclusive.  The two-part/mixed-state
frontier, rather than either fitted witness, is where the work now is.  In particular, literally
scaling the witness's next split produces the exactly unsolvable residual
`Sb(247:1,247:1,240:2,231:2)@8`; that continuation is refuted even though the parent state remains
open.

## One-sided n-splits: avoidable at every node tested, but not proven excludable

Across the frontier enumerations above — six states, three values of k, for both m=5 and m=6 —
there are **zero** working splits with the n-side left whole. Descending one level into
`Sb(50:4, 54:2)@6`, the mixed child of `Sb(104:6)@7`, all four of its working splits are
two-sided as well.

The 34 one-sided splits in the numbered `Sa` trees, which come from the *unrestricted* solver,
**are genuine occurrences** — an earlier draft of this section wrongly implied otherwise. What is
true is weaker and still useful: in all five states tested exhaustively, a fully two-sided split
also exists at that node, so the one-sided choice was *taken*, not *forced*. For example
`Sb(4:3,3:3)@3` has 10 working splits of which 6 are fully two-sided; the witness happened to pick
`[2:1,3:2]`, but `[2:1,2:3]` works and is two-sided.

Two-sided splits are however **rare**: 76 of 5440 for `Sb(3:2,3:2,4:1,4:1,3:1,3:1)@3`, 42 of 3324
for `Sb(4:2,3:2,4:1,3:1,3:1,2:1)@3`. So the restriction prunes hard — good for tractability if it
is safe, but it is a tight constraint, and local availability at every node does **not** imply a
complete two-sided tree exists: choosing the two-sided split changes the children, hence the
subproblems.

The five instances in the committed canonical trees are **all in `canon_473_6_at9`**, and all are
no-ops. `Sb(7:1)` split `[7:1]` sends the whole part to outcome 2 and leaves the other two
branches empty — the state is unchanged, one level deeper. The reason is the search's
termination rule, not the problem: `7` is not an atom of `G_4` but is an atom of `G_3`, so
burning a level makes the state canonical and lets `radio_canon_search_generic` stop. A real
split such as `[4:1]` yields `Sb(4:1)@3` and `Sb(3:1)@3` with no level wasted.

So the same tree that fails the profile test and wastes 7 paths is also the only one using
one-sided splits, and for the same underlying reason: it is a valid but wasteful witness.

### What the canonical search does and does not assume

Checked against the source, because evidence drawn from `canon_*.tree` is only as good as the
tool. `radio_canon_search_generic` does **not** prohibit one-sided splits: the n-side ranges over
all of `[0,n]`, `uneven_rank_to_value` is a bijection onto `[0,m]`, and `top_split_canonical_rep`
is applied only at the top level (line 309) where it discards `a=0` as the mirror of the `a=n` it
keeps — a deduplication, not an exclusion.

What it *does* assume is stronger and genuinely putative: `is_canonical_state` requires every leaf
to be a singleton state whose parts form a sub-multiset of the `G_k` atoms. Solutions of any other
shape are invisible to it. So conclusions about tree *interiors* drawn from canonical trees are
conditional on that hypothesis, and the "0 of 425" figure should not be read as evidence about
optimal solutions in general.

### Answered (2026-08-03): one-sided n-splits are not necessary

`radio_canon_twosided.c` is the same search with `TWO_SIDED_ONLY=1` rejecting any split that
leaves a non-nil part's n-side whole. Under that restriction:

| state | result |
|---|---|
| `Sb(480:5)@9` | 5 verified trees, `witnesses/canon_480_5_at9_twosided.tree` |
| `Sb(473:6)@9` | 1 verified tree, `witnesses/canon_473_6_at9_twosided.tree` |

`473:6` is the decisive one: the unrestricted witness uses **5** one-sided splits, and they are
all avoidable. The two-sided tree is also **less wasteful** — 2 empty paths of 384 against 7:

| | nodes | splits | leaves | one-sided | wasted paths |
|---|---|---|---|---|---|
| `canon_473_6_at9` (unrestricted) | 154 | 51 | 103 | 5 | 7 |
| `canon_473_6_at9_twosided` | 172 | 57 | 115 | **0** | see below |

So excluding one-sided n-splits appears **safe** — it costs no solutions on anything tested.
Still not a theorem: two states at one value of k.

#### Orientation flips break the waste and symmetry metrics

A part `n:m` is stored oriented `n >= m`, and a child can invert that: `Sb(10:2)` split `[9:2]`
has mixed child `(1,2)`, which `getSbb` stores as `Sb(2:1)`. The path model assumes a **fixed**
m-side, so after a flip the coin count reads wrong — here it loses `(2-1) * 2^(4-3) = 2` atoms,
exactly the apparent 384 - 382 deficit.

The two-sided tree has **1 flip in 272 non-nil children**, so its "2 wasted paths" was an
artifact and its asymmetry verdict is not meaningful either. `tools/profile_from_tree.py` now
counts flips and refuses to interpret waste or symmetry when any are present.

Unaffected, because they contain **zero** flips: the unrestricted `473:6` tree (7 genuinely
wasted paths, genuinely asymmetric) and all nine `480:5` solutions (80 of 80, seven carrying the
profile). Every earlier conclusion drawn from those stands.

A prediction of mine failed here and the failure is instructive. `Sb(7:1)@4` genuinely has no
two-sided decomposition: with `target_k=3` a singleton `Sb(x:1)@d` must be an atom of `G_d` or
split into two members of `R(d-1)`, and `R(3) = {1,4,7,8}` gives sums `{2,5,8,9,11,12,14,15,16}` —
7 is absent, and 7 is not an atom of `G_4 = {16,15,11,5,1}`. Likewise `Sb(4:1)@4`. I concluded the
two-sided search must fail. It did not: the tree it found simply never produces `Sb(7:1)@4`. The
arithmetic obstruction is real but local, and a different route avoids the state entirely.

**Full enumeration completed** (2026-08-03, normal exit, not a timeout): the two-sided search
over *every* top-level split of `Sb(473:6)@9` yields **exactly one tree**, the one committed.
The unrestricted search likewise yields exactly one. Both have the same root, `[242:4]`.

That does **not** show a symmetric tree is impossible, and the reason is a property of the search
worth stating plainly: `search_state` returns on the **first** successful subtree and memoises it,
so the enumeration produces one tree *per top-level split*, not all trees. Only one top split
works for `473:6@9`, so we see one tree per restriction mode — and we know at least two distinct
subtrees exist beneath that single root, because the restricted and unrestricted searches return
different ones.

So the count "1 tree" measures top splits, not solutions. Deciding whether `Sb(473:6)@9` admits a
symmetric tree needs a search that enumerates *subtrees*, or one that optimises the census
directly. Neither exists.

## What is actually invariant across the k=9 trees (2026-08-03)

Measured across the **9 alternative solutions** of `Sb(480:5)@9`:

| quantity | across the 9 solutions |
|---|---|
| total mass `n·m` | **invariant** (2400) — forced, every candidate pair reaches one leaf |
| atom count at the normalisation level | **invariant** (80 `= m·2^(k-t)`) — i.e. all nine waste no paths |
| root split | varies, 9 distinct (`a` from 240 to 248, `b` in {2,3,4}) |
| leaf count | varies, 34 to 42 |
| leaf-depth profile | varies, 9 distinct |
| **atom census** | **varies — 3 distinct values** |

So the census takes **three** different values among nine solutions of the *same state*:
`{32:45, 31:20, 26:10, 16:5}` for seven of them, and two one-off asymmetric censuses. The two
outliers are the root `240:4` solution (the only `b=4` one) and the root `243:2` solution.

### Consequence for the canonicalisation programme

There is **no invariant that picks out "the" solution**, because the solution space genuinely
splits into classes with different atom content. This matches the journal's finding that the two
`496:4` witnesses have different final branch-signature multisets and disjoint orbits — it is not
that the generator family is too weak, it is that the objects are inequivalent.

The right statement is narrower and it holds:

> The profile is an invariant of the **symmetric, non-wasteful** class of solutions, not of the
> state. Seven of the nine `480:5` solutions lie in that class; two do not.

So canonicalise *within* that class and treat the rest as a separate family. What makes the class
well defined is the two conditions: no empty paths (`atom count = m·2^(k-t)`) and census divisible
by `m`.

### Why the m=6 top prefix has no alternatives

The split window for `Sb(n(k,6):6)` at `b=2` is bounded below by `n(k,6) - n(k-1,4)` and above by
`n(k-1,2)`. Those give a window of 7, 10, 14, 19, 25 values for `k = 5..9` — wide. But the mixed
child `Sb(a:4, (n-a):2)` closes it to **two** values at `k=5,6,7`, and at `k=9` to essentially
**one**: `a = 231`, appearing as its mirror `[242:4]`.

That is why `canon_473_6_at9` has only one working root split. It does **not** make the whole tree
unique. The recorded descent reaches `Sb(110:3,115:2,121:1)@7`, whose exhaustive artifact has one
outcome-complement pair; ambiguity then appears at `Sb(53:2,52:2,57:1,57:1)@6`, where the artifact
records 12 working splits. These reduce to three classes under outcome complementation and exchange
of the identical `57:1` parts.
The committed witness follows only one class. Consequently the forced prefix is structural
evidence; its later fitted continuation is not evidence against a different scalable tree.

**The descent depth is forced, not incidental.** Running the search with `target_k = 4`, which
requires every leaf to be canonical at depth 4 or deeper, returns `NO_CANONICAL_TREE`. So no
solution of `Sb(473:6)@9` has all its leaves canonical above depth 3 — within the canonical
hypothesis, m=6 *must* grind two levels deeper than m=5. Required depth by m:

| m | 3 | 4 | 5 | 6 |
|---|---|---|---|---|
| min leaf depth `t` at k=9 (k=8 for m=3) | 6 | 6 | 5 | **3** |
| `q = k - t`, profile length `2^q` | 2 / 4 | 3 / 8 | 4 / 16 | **6 / 64** |

The jump from `q=4` to `q=6` skips 32 entirely — the same gap that appears in the m=11 profile
length. Whatever forces that skip is probably one phenomenon, not two.

## Structural threads (from the journal)

Carried over from [journal.md](journal.md), unresolved:

- **The missing canonicalization generator.** The two `496:4` witness-derived matrices have
  disjoint orbits of size 4096 under the current prefix-rotation generators, and different
  final branch-signature multisets. A second generator is needed and no candidate preserving
  the right invariants has been found. The depth-2 block clue is a conservative 3-block
  rewrite `{AACC, AB, BC} <-> {AA, AC, BBCC}`.
- **What replaces the refuted `m=6` profile.** The exact `k=10` maximum is 973, reached through
  `Sb(496:2,477:4)@9`, so the old `BBCD`/closed-form value 976 is dead.  The next task is a
  parametric construction or obstruction for this `m=2 + m=4` mixed-state frontier.  Do not fit
  the 973 witness's later subtree or infer a constant three-unit correction from one level.
- **Proving rather than fitting the closed forms.** Lemmas 1-5 have real inductive proofs.
  Nothing beyond `m = 4` has a matching upper bound.
