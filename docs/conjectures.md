# Open predictions

Nothing on this page is established. Every claim here comes with the experiment that would
settle it, because a prediction you cannot falsify is not worth recording.

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
| 6 | `BBCD` | 2 |
| 7 | `ABBBBCDD` | 3 |
| 8 | `AAACCCDD` | 3 |
| 9 | `AAAAAABBCCCCCCDE` | 4 |
| 10 | `AAAAABBCCCCCCCEE` | 4 |

Profile length is constrained to a power of two and the alphabet to the first `m` dyadic
slots. For `m = 11` the shortest exact fit found needs length 64 at `q = 5`, and the string
was not recorded - a gap worth closing, since it is the only thing blocking an independent
check of `n(9,11)`.

Both models reproduce **every** proven value for `k = 1..8`, `m = 1..10`.

## The discriminating experiment

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

## Scalable constructions for m=5 and m=6 (2026-08-03)

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

**Two recursions, exact on every available k (5..9):**

    n(k,5) = n(k-1,2) + n(k-1,6)
    n(k,6) = n(k-1,4) + n(k-1,5)

The second is realised directly by a split: `b = 2`, `a = n(k-1,5)`, giving
`n(k-1,5):2` on outcome 2 and `n(k-1,4):4` **saturated** on outcome 0. The `473:6@9` witness
uses the mirror form `[242:4]`, i.e. `a = n(8,4) = 242`, `n-a = n(8,5) = 231`.

The first is a numerical identity only — `m=6 > 5` cannot appear as a child of an m=5 part, and
the split that would saturate it (`a = n(k-1,2)`) is *not* in the working set, so it is not
realised structurally.

Both constructions bottom out in a 2-part mixed state. That state is no longer
uncharacterised: see [the two-part frontier programme](#the-two-part-frontier-programme-2026-08-03)
below, which measures it, gives it an exact theorem for one family, and shows that the whole of
`n(k,m)` follows from it.

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

### Why m=6 has no alternatives to compare

The split window for `Sb(n(k,6):6)` at `b=2` is bounded below by `n(k,6) - n(k-1,4)` and above by
`n(k-1,2)`. Those give a window of 7, 10, 14, 19, 25 values for `k = 5..9` — wide. But the mixed
child `Sb(a:4, (n-a):2)` closes it to **two** values at `k=5,6,7`, and at `k=9` to essentially
**one**: `a = 231`, appearing as its mirror `[242:4]`.

That is why `canon_473_6_at9` is the *only* tree the canonical search returns, and why there is no
symmetric alternative to find — at the root there is nothing to choose.

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

## The two-part frontier programme (2026-08-03)

The blocker recorded above — "bottoms out in a 2-part mixed state whose frontier is
uncharacterised" — turns out to be the *whole* problem rather than one obstacle in it, and it
is now measured.

Write `g_k(s,t; p)` for the largest `q` with `Sb(p:s, q:t)` solvable in `k`. Subgraph
Monotonicity makes it non-increasing in `p`, so each `(k,s,t)` gives a staircase. Staircases
are in [`../data/pareto_2part.csv`](../data/pareto_2part.csv) (52 of them, `k = 4,5,6`);
`radio_2part.c` produces them and `tools/refsolve.py two-part` reproduces them independently
for `k <= 5`.

**The reduction is exact.** `n(k+1,m)` is the largest `p+q` over the two-part frontier at `k`,
subject to two crossed caps — proved and verified 20/20 in
[theorems/two-part-reduction.md](theorems/two-part-reduction.md), and re-checked on every run
by `tools/check_tables.py`. So a characterisation of `g` *is* a characterisation of `n(k,m)`,
and nothing else is missing.

**One family is settled.** `Sb(p:2, q:1)` is solvable in `k` iff `p <= 2^k - 1`, `q <= 2^k` and
`p + q <= 2^(k+1) - k - 1` — proved, and exhaustively verified by the independent solver for
`k <= 5`. It reproves `n(k,3) = 2^k - k`.

### `(2,2)` — conjectured, and it would prove Lemma 8

> `Sb(p:2, q:2)` is solvable in `k >= 4` iff `p, q <= 2^k - 1` and
> `p + q <= 2^(k+1) - 2k - [max(p,q) >= 2^k - 2]`.

The bracket is an indicator: the last unit of the sum is available only when **neither** side
is within `1` of `n(k,2)`. Equivalently the top sum `2·n(k,3)` is reached exactly on the window
`p in [n(k,4)+1, n(k,2)-2]`.

Verified against complete staircases at `k = 4, 5, 6` and at both window edges at `k = 7`:
`Sb(121:2,121:2)` and `Sb(117:2,125:2)` are solvable, `Sb(121:2,122:2)` and `Sb(116:2,126:2)`
are not. It is **false at `k = 2, 3`**, where it is conservative — `Sb(1:2,3:2)@2` and
`Sb(4:2,6:2)@3` are solvable and the form says otherwise. The max-sum half,
`max_p [p + g_k(2,2; p)] = n(k+1,4)`, holds from `k = 2` with no exception.

Four values of `k` is the bare minimum this repo trusts (see the fits trap in
[status.md](status.md)); the window bounds in particular are a two-parameter fit on four points.

### `(3,2)` and `(4,2)` — the two the recursions actually need

Both are measured but neither has a closed form yet. What is verified is the sharper statement
each construction needs, at the single extreme point:

| statement | meaning | verified at |
|---|---|---|
| `g_k(4,2; n(k,5)) = n(k,4)` | `Sb(n(k,5):4, n(k,4):2)` is solvable in `k` and one more coin on the 2-side is not | `k = 4,5,6,7` |
| `g_k(3,2; n(k,3)) = n(k+1,5) - n(k,3)` | the `m=5` split window reaches `n(k+1,5)` at its endpoint | `k = 4,5,6,7` |

The first is the **Mixed-Saturation Lemma** for `m = 6`: together with the two one-part facts
`n(k,4) <= n(k,4)` and `n(k,5) <= n(k,2)` it gives `n(k+1,6) >= n(k,4) + n(k,5)`, i.e. the
construction, and with the reduction identity it gives equality. Proving it for all `k` would
prove Lemma 10.

### The `m=6` mixed child has a rigid split, and it leaves the two-part class

`Sb(n(k,5):4, n(k,4):2)` has **exactly four** working top-level splits at each of `k = 4, 5, 6`
— two, plus their complements. Writing `A = n(k-1,2)`, `T = n(k-1,3)`,
`F = n(k-1,4)` and `α = n(k,5) - T`, they are `[α:1, A:2]` and `[(α+1):1, A:2]`, with children

```
out2  Sb(A:2, α:1)                  a (2,1) state, with A = n(k-1,2) saturated
out0  Sb(T:3)                       saturated at the one-part frontier
out1  Sb(α:3, T:1, (F-1):2)         three parts
```

(the second split shifts `α -> α+1` and `T -> T-1`). Exact at all three `k`:
`α = 4, 10, 23` and `(A, T, F-1) = (7,5,3), (15,12,9), (31,27,23)`.

So the descent is completely determined, `out2` is governed by the theorem above with slack,
`out0` is a saturated one-part state — and **`out1` has three parts**. The two-part class is
not closed under its own optimal descent. Each `out1` was confirmed solvable at its own budget
`k-1` (so the descent is real, not a wasted level), but the three-part family it lands in is
uncharacterised, and that is now the frontier of this line of work.
## The symbolic profile programme (2026-08-03)

A route to `n(k,m)` for general `m`, arrived at from the coin side. It subsumes the profile
model above by giving it a mechanism *and* a feasibility condition, and it is the direction
the next session should take.

### The two facts it rests on

**Parts partition the m-side.** At every node the parts' m-sides form a partition of a subset
of the original `m` coins — the root is one part holding all of `Y`, and a split sends
in-coins to outcome 2, out-coins to outcome 0, and both to outcome 1 *as two separate parts*,
never duplicating a coin. Hence **at most `m` non-nil parts at any node**. Unlike the profile
census this is immune to the orientation-flip trap below, because it is about the intrinsic
bipartition rather than the stored orientation.

**Distinct parts are vertex-disjoint** (the graph reformulation), so at a level-`q` node, where
the m-side is atomized, the alive coins hold pairwise disjoint n-side chunks and each splits
independently. Coins only share a chunk while they share a part.

### The formulation

Fix `m` and `q`, and let `t = k - q`. Write atoms as dyadic-block letters of `G_t` —
`A` = block 0, `B` = block 1, `C` = block 2, `D` = block 3.

> Find an m-side pattern — which coins are alive at which of the `3^q` nodes, and how they are
> grouped into parts — together with a letter string `P` of length `2^q`, such that `m` copies
> of `P` can be laid on the level-`q` nodes, coin `y`'s copy on `y`'s own `2^q` nodes, one
> letter per alive coin, with every node's received multiset a legal sub-multiset of `G_t`.
> Maximise the value of `P`.

**The whole condition is `t`-free.** Block multiplicities in `G_t` are `1, 1, 2, 4, 8, …`
regardless of `t`, so "sub-multiset of `G_t`" reads in letters as "at most one `A`, at most one
`B`, at most two `C`, at most four `D`, …". The m-side pattern contains no `n` either. So one
solve per `(m,q)` yields a letter string, and `n(k,m)` is that string evaluated at `t = k-q`
— *for every `k` at once*. That is the scalable formula the profile model was reaching for.

The only role of `t` is that `G_t` has `t+1` distinct blocks, so `t` must exceed the deepest
letter used. That is the precise form of the "`n` far from the diagonal, no edge effects"
assumption.

### Worked, verified, and it hits the maximum

`m = 3`, `q = 2`. All three coins carry the profile **`AABC`**; the shared nodes read
`{A,B}`, `{A,B,C}`, `{A,C}`, all legal. Evaluating at `t = k-2`:

```
2A + B + C = 2·2^(k-2) + (2^(k-2) - 1) + (2^(k-2) - k + 1) = 2^k - k = n(k,3)
```

for every `k`, from one packing solve. The `k = 5` instance is committed as
[`../witnesses/canon_27_3_at5_symbolic.tree`](../witnesses/canon_27_3_at5_symbolic.tree) and
passes `tools/check_witness.py` — a hand-derived construction, not solver output, reaching the
proven maximum `n(5,3) = 27`.

`AABC` is the forced refinement of the `AC` recorded for `m=3` in `data/conjectures.csv`. So
this route does not merely match the fitted profile table, it would **derive** it. The same
evaluation checks out for `AACC` → `n(k,4)`, `BBBD` → `n(k,5)`, `BBCD` → `n(k,6)`.

### `q` has two different jobs, and the repo has been conflating them

- **Value.** Refinement is forced (Atom Descent, Corollary 2 in
  [theorems/singleton-majorization.md](theorems/singleton-majorization.md)), so `AC` and `AABC`
  are the same number. Larger `q` adds no expressive power to the profile. This is the
  "refinement invariance" already recorded above, now with a proof.
- **Feasibility.** Larger `q` buys *packing room*, and that is what `q_min(m)` measures.
  `m = 6` has a length-4 profile yet needs `q = 6`; atomizing 6 coins takes only
  `⌈log₂ 6⌉ = 3` levels. So `q_min` is a packing threshold, and the journal's "why does `q` skip
  4 → 6 at `m=6`" is a question about packing feasibility, not about refining the m-side.

### It explains the canonical search's small-k failures

Atom Descent Corollary 1 says atomic-leaf feasibility is monotone, so a `NO_CANONICAL_TREE` at
`target_k = t` forces one at every larger `t`. The recorded failures are then all one fact,
`q_min(6) ≈ 6`, rather than a defect of the atomic-leaf hypothesis:

| state | recorded | implies |
|---|---|---|
| `Sb(46:6)@6` | fails at `target_k` 2 and 3 | largest feasible `t ≤ 1`, so `q_min ≥ 5` |
| `Sb(104:6)@7` | fails at `target_k` 3 | largest feasible `t ≤ 2`, so `q_min ≥ 5` |
| `Sb(473:6)@9` | fails at 4, works at 3 | largest feasible `t = 3`, so `q_min = 6` |

Falsifiable predictions, cheap to test: `Sb(104:6)@7` should succeed at `target_k = 1` and fail
at 2; `Sb(46:6)@6` should succeed only at `target_k = 0`; `Sb(50:5)@6` should succeed at
`target_k = 2` since `q_min(5) = 4`.

### Solved for m = 3, 4, 5 (2026-08-03)

`tools/symbolic_profile.py` implements the search. Results, each the largest-value profile the
`m` coins can pack in `q` levels:

| m | q_min | profile | `n(k,m)` |
|---|---|---|---|
| 3 | 2 | `AABC` | `2^k - k` |
| 4 | 3 | `AAAABBCC` | `2^k - 2k + 2` |
| 5 | 4 | `AAAAAAAAABBBBCCD` | `2^k - k(k-3)/2 - 5` |

Three things fall out at once, none of them put in by hand:

- **The formulas are exactly Lemmas 6, 8 and 9** — the last two previously unproved. The solver
  never sees a value of `n` or `k`.
- **`q_min = 2, 3, 4` reproduces the tree depths measured independently** from the canonical
  witnesses (the table under
  [what is actually invariant](#what-is-actually-invariant-across-the-k9-trees-2026-08-03)).
- **Each profile is the forced refinement of the fitted one** — `AC`, `AACC`, `BBBD` refined to
  `q_min` under `A→aa, B→ab, C→bc, D→cd`. So the fitted profile table is derived, not matched.

`m = 6` is consistent so far but not finished: the refinements of `BBCD` at `q = 3` and `q = 4`
are **infeasible**, as `q_min(6) = 6` predicts. `q = 5, 6` are expensive and still open.

### The value/feasibility split, demonstrated

`AACC` is **infeasible at `q=2`** — the best a 4-coin state can pack in two levels is `ABCC`,
worth `n(k,4) - 1`. Its refinement `AAAABBCC` is feasible at `q=3` and worth exactly `n(k,4)`.
So the shortest representative of a profile class need not be realisable; only a deep enough
refinement is. That is the distinction between the two jobs of `q` above, now measured rather
than argued.

The prune that kills `AACC` is a counting one, and it is the reason the search terminates: a
part with `c` coins and `v[i]` copies of letter `i` needs `c · v[i]` slots below it, there are
`3^d` level-`q` nodes below, and each holds at most `multiplicity(i)` copies. At `d=1` the part
`(2, AA)` needs 4 slots for `A` and only 3 exist.

### The constructions are real, and they scale

Instantiating a profile at `t = k-q` gives a tree that `tools/check_witness.py` verifies from
first principles. **24 of them were generated and checked, `k = 4..12` for `m = 3,4,5`**, every
one matching the closed form. So the lower bounds

```
n(k,3) >= 2^k - k        n(k,4) >= 2^k - 2k + 2        n(k,5) >= 2^k - k(k-3)/2 - 5
```

are **unconditional** at every `k` checked, from one letter string each. That is the scalable
base-sequence construction this programme was after. Three instances are committed as
`witnesses/symbolic_{1014_3,1006_4,984_5}_at10.tree` and appear in `data/pareto_sb.csv` as the
first `k = 10` entries in the `Sb` table.

### m = 6: not a search problem, and probably not a compute problem (2026-08-03)

`m=6` resisted every search thrown at it. Rather than widen the search, the descent was
*pinned* from outside, using the two-part enumeration, and the residue examined. That turned
out to be decisive about where the difficulty is.

**No counting obstruction exists.** Three necessary conditions were computed exactly from the
m-side pattern alone (no `n`, no letters), and all pass:

| condition | why | m=6, q=6 |
|---|---|---|
| `m·nA(P) <= max occupied nodes` | `A` has multiplicity 1, so every A-play needs its own node | `276 <= 325` |
| `S_1 <= m(nB+nC+nD)` | at a node with `r` coins, `r-1` play non-`A` | `59 <= 108` |
| `S_2 <= m(nC+nD)`, `S_4 <= m·nD` | at most 2 coins play `A` or `B`, at most 4 play `A..C` | both pass |

(`S_j = Σ_v max(0, r_v − j)`.) They are tight — 276 of at most 325 occupied nodes must carry an
`A` — but they do not block. So if `m=6` fails, it fails for a structural reason, not a
counting one.

**The forced descent lands exactly where the repo already looked.** The root split of
`Sb(473:6)@9` is unique, and the two-part work fixes the next level too. Instantiating the
symbolic descent at `k=9` (`t = 3`, so `A=8, B=7, C=4, D=1`) gives

```
P = A^46 B^12 C^5 D                      -> Sb(473:6)          = n(9,6)
  out2 Sb(231:2)   out0 Sb(242:4)        = n(8,5), n(8,4) saturated
  out1 Sb(231:4, 242:2)                  the Mixed-Saturation state
    out1/out2  Sb(127:2, 110:1)          FEASIBLE
    out1/out0  Sb(121:3)                 FEASIBLE
    out1/out1  Sb(110:3, 121:1, 115:2)   <- the whole remaining question
```

That last state is **already in [`../data/exhaustive_multipart.csv`](../data/exhaustive_multipart.csv)**:
`Sb(110:3, 115:2, 121:1)` in 7, solvable, with exactly **2 working splits out of 37,700,928** —
and its neighbour `Sb(111:3, 115:2, 121:1)` is recorded unsolvable. The symbolic model, which
never sees an `n` or a `k`, reconstructs a state that a 2023 exhaustive enumeration had already
singled out, sitting one coin away from impossible. Whatever else is true, the model is
tracking the real extremal structure.

**And the pinned split fails.** A symbolic solution instantiated at `k=9` must use one of those
two splits (they are mirrors). Translating `[57:2, 52:0, 64:1]` back into letters gives 3 x 3 x 1
= 9 candidate decompositions — the numeric split does not determine the letters, and different
letter multisets behave differently at other `t`. **All nine are infeasible.**

So the live hypothesis is that `m=6` has **no symmetric atomic solution**, i.e. the model is
incomplete for `m=6` rather than merely slow. Supporting it: the only atomic `q=6` witness on
record, `canon_473_6_at9.tree`, is asymmetric — census `{A:289, B:63, C:20, D:5}`, 377 atoms
with 7 empty paths, against the `6 x (A^46 B^12 C^5 D)` = `{A:276, B:72, C:30, D:6}` a symmetric
one would need.

**What is not yet closed**, and should be before this is called settled:

- the second level-2 split family (`alpha+1` instead of `alpha`) was still under test;
- "exactly 2 working splits" is a 2023-era claim resting on 37.7M *negative* verdicts, from the
  build with known false negatives. Fine as a guide, not as a proof;
- the 2023 enumeration of the Mixed-Saturation state itself, `full_231_4_242_2.txt`, **aborted**
  with `updated == 0 when caching result` — the `MAX_N` trap documented in
  [tools.md](tools.md#max_n-undersizing-is-silent-found-2026-08-03) — which is why that state
  never made it into `exhaustive_multipart.csv`. Redoing it on a correctly sized build would
  close the level-2 case properly.

If the hypothesis holds, the fix is not more compute: it is to let coins carry *different*
profiles, which costs the `t`-uniformity argument that currently forces symmetry and is the
next real modelling question.

### What is assumed

- **Optimality, not achievability, is what rests on assumptions.** Anything the model finds is
  a construction, checkable at any `k`, and does not depend on the model being right.
- **The atomic-leaf hypothesis** is needed for the upper-bound reading — that the profile found
  is the *best* one. Its evidence is that the maximum coincides with the known `n(k,m)` for
  `m = 3,4,5` across every `k` where the frontier is proven.
- **Symmetry is not an extra assumption here**, contrary to what an earlier draft of this
  section said. Every coin's profile is the root multiset by construction; and distinct
  letter-count vectors are linearly independent as functions of `t`, so two coins with equal
  totals *for all t* necessarily carry the same multiset. Asymmetric solutions exist (two of
  the nine `Sb(480:5)@9`), but they cannot be described by a single `t`-uniform formula, which
  is what makes them invisible here and harmless.

## Structural threads (from the journal)

Carried over from [journal.md](journal.md), unresolved:

- **The missing canonicalization generator.** The two `496:4` witness-derived matrices have
  disjoint orbits of size 4096 under the current prefix-rotation generators, and different
  final branch-signature multisets. A second generator is needed and no candidate preserving
  the right invariants has been found. The depth-2 block clue is a conservative 3-block
  rewrite `{AACC, AB, BC} <-> {AA, AC, BBCC}`.
- **Whether `473:6 @9` scales.** Answered negatively for the witness we have — see above.
  Whether a symmetric one exists is open.
- **Proving rather than fitting the closed forms.** Lemmas 1-5 have real inductive proofs.
  Nothing beyond `m = 4` has a matching upper bound.
