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

## Conjecture (u1)

`n(k, m-1) >= n(k, m) + 1`. Holds on all 130 proven cells. Stated in
[theorems/special-cases.md](theorems/special-cases.md#conjecture-u1). Unproved.

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

Both constructions bottom out in a 2-part mixed state whose frontier is uncharacterised. That
is where the work now is.

## One-sided n-splits: every observed instance is explained, none is necessary

Revising the earlier empirical claim. Across the frontier enumerations above — six states,
three values of k, for both m=5 and m=6 — there are **zero** working splits with the n-side left
whole. Descending one level into `Sb(50:4, 54:2)@6`, the mixed child of `Sb(104:6)@7`, all four
of its working splits are two-sided as well.

The five instances in the committed canonical trees are **all in `canon_473_6_at9`**, and all are
no-ops. `Sb(7:1)` split `[7:1]` sends the whole part to outcome 2 and leaves the other two
branches empty — the state is unchanged, one level deeper. The reason is the search's
termination rule, not the problem: `7` is not an atom of `G_4` but is an atom of `G_3`, so
burning a level makes the state canonical and lets `radio_canon_search_generic` stop. A real
split such as `[4:1]` yields `Sb(4:1)@3` and `Sb(3:1)@3` with no level wasted.

So the same tree that fails the profile test and wastes 7 paths is also the only one using
one-sided splits, and for the same underlying reason: it is a valid but wasteful witness.

**Status: not safe to assume, but no counterexample survives inspection.** Excluding one-sided
n-splits costs nothing on any state examined so far, at any k, for m <= 6. What is missing is
the 2-part frontier — until that is characterised, "safe" is unproven rather than supported by
the near-square exceptions I previously cited, which are now explained away.

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
