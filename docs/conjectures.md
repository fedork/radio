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
