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

## Structural threads (from the journal)

Carried over from [journal.md](journal.md), unresolved:

- **The missing canonicalization generator.** The two `496:4` witness-derived matrices have
  disjoint orbits of size 4096 under the current prefix-rotation generators, and different
  final branch-signature multisets. A second generator is needed and no candidate preserving
  the right invariants has been found. The depth-2 block clue is a conservative 3-block
  rewrite `{AACC, AB, BC} <-> {AA, AC, BBCC}`.
- **Whether `473:6 @9` scales.** Verifying the witness tree (done, see
  [results.md](results.md)) settles the tree/state level only. Whether it lifts to a valid
  compact atomic decomposition matrix under one-level scaling is untouched.
- **Proving rather than fitting the closed forms.** Lemmas 1-5 have real inductive proofs.
  Nothing beyond `m = 4` has a matching upper bound.
