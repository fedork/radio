# Special-case constructions (lemmas 1-11)

Closed forms for `n(k,m)`, the largest `n1` with `Sb(n1 : m)` solvable in `k`, for small
`m`. Each is marked with what is actually established.

The machine-readable versions live in `data/conjectures.csv` and are checked against every
proven datum by `tools/check_tables.py`. **Do not copy a formula out of this file into
another document** - reference it. Lemma 10 below was corrupted exactly that way once.

Status key: **proved** = full argument given here; **construction** = the split is exhibited
and gives a valid lower bound, but no matching upper bound is proved; **conjecture** =
numerical fit only.

<!-- generated:conjectures -->
| m | closed form | fits from | status |
|---|---|---|---|
| 1 | `2^k` | k >= 1 | proven-theorem |
| 2 | `2^k - 1` | k >= 2 | proven-theorem |
| 3 | `2^k - k` | k >= 3 | proven-theorem |
| 4 | `2^k - 2*k + 2` | k >= 3 | conjecture |
| 5 | `2^k - k*(k-3)/2 - 5` | k >= 4 | conjecture |
| 6 | `2^k - k*(k-1)/2 - 3` | k >= 4 | refuted |
| 6 | `2^k - k*k + 7*k - 21` | k >= 9 | conjecture |
| 7 | `2^k - k*k + 4*k - 10` | k >= 5 | conjecture |
| 8 | `2^k - k*k + 2*k - 2` | k >= 5 | conjecture |
| 9 | `(31*2^k)/32 - k*k + 2*k - 2` | k >= 5 | conjecture |
| 10 | `(15*2^k)/16 - k*k + 2*k - 3` | k >= 5 | conjecture |
| 11 | `(33*2^k)/32 - 7*k*k/2 + 51*k/2 - 62` | k >= 5 | refuted |

Formulas are stored executably in `data/conjectures.csv` and checked against every proven datum by `tools/check_tables.py`.
<!-- /generated -->

---

## (1) `Sb(2^k : 1)` - proved, both directions

One defective is already localised to a single coin, so the problem reduces to finding one
defective among `n`. Each test either contains that coin's group or not, giving two
informative outcomes, so `k` tests distinguish at most `2^k` positions, and binary search
attains it. Hence `n(k,1) = 2^k` exactly.

## (2) `Sb(2^k : 1, 2^k - 1 : 1)` - proved by induction

*Base.* `k = 0` gives `Sb(1:1, 0:1)`; dropping the zero group leaves `Sb(1:1)`, solvable in 0.

*Step.* If solvable in `k-1`, then in `k` use the split `[2^(k-1) : 1, 2^(k-1) - 1 : 0]`.
Outcomes 0 and 2 each yield `Sb(2^(k-1) : 1)`, solvable in `k-1` by (1); outcome 1 yields
`Sb(2^(k-1) : 1, 2^(k-1) - 1 : 1)`, solvable in `k-1` by hypothesis.

## (3) `Sb(2^k - 1 : 2)` - proved

Split `[2^(k-1) : 1]`, giving `Sb(2^(k-1) : 1)` by (1), `Sb(2^(k-1) - 1 : 1)` by (1), and
`Sb(2^(k-1) : 1, 2^(k-1) - 1 : 1)` by (2). So `n(k,2) = 2^k - 1`.

## (4) `Sb(2^k : 1, 2^k - 1 : 1, 2^k - k - 1 : 1)` - proved by induction

Split `[2^(k-1) : 0, 2^(k-1) : 1, 2^(k-1) - 1 : 1]`, giving
`Sb(2^(k-1) : 1, 2^(k-1) : 1)` by (2), `Sb(2^(k-1) : 1)` by (1), and
`Sb(2^(k-1) : 1, 2^(k-1) : 1, 2^(k-1) - (k-1) - 1 : 1)` by induction, using
`(2^k - k - 1) - (2^(k-1) - 1) = 2^(k-1) - k`.

## (5) `Sb(2^k - 1 : 2, 2^k - k : 1)` - proved

Split `[2^(k-1) - 1 : 1, 2^(k-1) : 1]`, giving `Sb(2^(k-1) - 1 : 1, 2^(k-1) : 1)` by (2),
`Sb(2^(k-1) : 1)` by (1), and `Sb(2^(k-1) : 1, 2^(k-1) - 1 : 1, 2^(k-1) - k : 1)` by (4).

## (6) `Sb(2^k - k : 3)` - construction

Split `[2^(k-1) - 1 : 1]`, giving `Sb(2^(k-1) - 1 : 1)` by (1), `Sb(2^(k-1) - k : 2)` by (3),
and `Sb(2^(k-1) - 1 : 2, 2^(k-1) - (k-1) : 1)` by (5). Lower bound only; matches the proven
maximum for every `k <= 8`.

## (8) `Sb(2^k - 2k + 2 : 4)` - construction

Matches the proven maximum for `k = 3..8`. The `k=9` value 496 has a verified witness tree,
`witnesses/canon_496_4_at9.tree`.

## (9) `Sb(2^k - k(k-3)/2 - 5 : 5)` - conjecture

Exact for `k = 4..8`. The `k=9` value 480 has a verified witness tree,
`witnesses/canon_480_5_at9.tree`.

## (10) `Sb(2^k - k(k-1)/2 - 3 : 6)` - refuted

Exact for `k = 4..8`: gives 7, 19, 46, 104, 225. The `k=9` value 473 has a verified witness
tree, `witnesses/canon_473_6_at9.tree`.

It fails at `k=10`: the formula predicts 976, while exact synchronized search proves the maximum
is 973.  The lower witness is `witnesses/majorized_973_6_at10.tree`; the exhaustive 974 rejection
is retained in `evidence/sb_m6_k10_frontier.txt`.  Thus this row was a finite fit, not a
construction valid for all `k`.

> **Correction, 2026-08-02.** An earlier write-up of this lemma had
> `2^k - k(k-5)/2 - 3`, which yields 15, 29, 58, 118, 241 - above the true frontier at every
> point, so it claimed unsolvable states were solvable. The `k(k-5)` was a transcription
> slip for `k(k-1)`; the spreadsheet source always had it right. `tools/check_tables.py`
> now rejects this class of error automatically.

## (11) `Sb(2^k - k^2 + 4k - 10 : 7)` - conjecture

Exact for `k = 5..8`.

## (12) `Sb(2^k - k^2 + 2k - 2 : 8)` - conjecture, new 2026-08-02

Exact for `k = 5..8`: gives 15, 38, 91, 206. Derived by the same fit as (11); note the
quadratic tail `-k^2 + 2k - 2` recurs in the `m = 9` and `m = 10` fits, with only the
leading coefficient moving. See [../conjectures.md](../conjectures.md).

---

## Conjecture (u1)

> If `Sb(n1 : n2)` is solvable in `k` for `n1 >= n2`, then so is `Sb((n1+1) : (n2-1))`.

Equivalently the frontier satisfies `n(k, m-1) >= n(k, m) + 1`. Verified against every cell
of the proven table (`k = 1..8`, 130 cells, no violations) by `tools/check_tables.py`, which
reports failures as warnings, and exhaustively over *every* one-part state for `k <= 5` by
`tools/refsolve.py check-c 5`. No proof is known.

It is **not** an instance of the Subgraph Monotonicity Theorem - the two states live on the
same coins and neither graph contains the other. Two natural proof routes were refuted on
2026-08-03, and the remaining gap is a single lemma about extremal splits; the full analysis,
with counterexamples and reproduction commands, is in
[../conjectures.md](../conjectures.md#conjecture-u1---the-antidiagonal-conjecture).

## On the evidential weight of these fits

Fitting `c * 2^k + a*k^2 + b*k + d` through the last four known points is four unknowns
through four equations - it always succeeds, so the fit itself is not evidence. What carries
information is where the free leading coefficient `c` lands:

- `m = 5..8`: `c = 1` exactly. A constraint satisfied with no freedom left over.
- `m = 9`: `c = 31/32 = 1 - 2^-5`, and the tail `-k^2 + 2k - 2` is *identical* to `m = 8`.
- `m = 10`: `c = 15/16 = 1 - 2^-4`, same tail, constant shifted by one.
- `m = 11`: `c = 33/32 > 1`, no structure - and **refuted**, see below.

The dyadic `c` values suggest a genuine second exponential term `2^k - 2^(k-r)` entering as
`m` grows, rather than curve-fitting noise.

## Refuted

**`m = 11` closed form.** The four-point fit gives
`(33/32) 2^k - 7k^2/2 + 51k/2 - 62`, which predicts `n(10,11) = 899` while the `m=10` fit
gives `n(10,10) = 877`. Since `Sb(n : m+1)` solvable implies `Sb(n : m)` solvable, the
frontier cannot increase with `m`, so this form is impossible. Recorded as `refuted` in
`data/conjectures.csv` so it is not re-derived. A hand-entered value of `409?` for
`n(9,11)` circulates in the spreadsheets; it is not produced by this fit (which gives 412)
nor by any other model here, and should be treated as a guess with no support.
