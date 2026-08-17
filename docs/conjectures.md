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
| 5 | ~~`BBBD`~~ **refuted as optimal at k=9** | 2 |
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
globally viable models.  At `m=5` both predict 480 at `k=9`, while the exact maximum is 481;
at `m=6` both give 976 at `k=10`, while the exact maximum is 973.  Their disagreement remains
useful for distinguishing the still-open *row-wise* `m=9,10` extrapolations, but neither model
should be extrapolated wholesale.

## The remaining m=9,10 discriminating experiment

The two remaining row-wise extrapolations diverge at `k = 9`:

| m | closed form | dyadic profile | difference |
|---|---:|---:|---:|
| **9** | **431** | **432** | 1 |
| **10** | **414** | **416** | 2 |

So:

> **If `Sb(432 : 9)` is solvable in 9 tests, the `m=9` closed-form row is dead.**

The test is asymmetric and that matters for how to spend compute:

- **Finding** a canonical witness tree for `Sb(432:9)` proves the profile prediction achievable
  and refutes the 431 equality outright, via the Singleton Majorization Theorem, with no solver
  trust required.  It would not by itself prove 432 maximal.  This is cheap - the same tool
  already produced trees for `496:4`, `480:5` and `473:6` at `k=9`.
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

Both are natural canonical-search targets and would extend the exact/witness-backed prefix of
the `k=9` column from `m <= 6` to `m <= 8`:

```
./run_radio_canon_search_generic.sh 4 9 457 7
./run_radio_canon_search_generic.sh 4 9 447 8
```

## Exact m=5 transition and the structural break (updated 2026-08-16)

Li--Wu--Triesch prove the exact piecewise answer.  If

    F(k) = 2^k - k(k-3)/2 - 5,

then

    n(k,5) = F(k)       for 3 <= k <= 8,
             F(k) + 1   for 9 <= k <= 10,
             F(k) + 2   for k >= 11.

This is not merely a numerical correction.  Their construction changes its first test from
type `3+2` through `k=8` to type `4+1` at `k=9,10`; from `k=11` the first two tests remain,
but another recursive stage supplies the second extra coin.  See
[literature.md#li-wu-triesch-2018](literature.md#li-wu-triesch-2018).

The construction now has an exact translation into the corrected assembly.  Put `t=k-2`,
`P=2^t`, and `Q=binomial(t-2,2)`.  The new height choice is
`(alpha,beta,gamma)=(4,3,1)`, with

    a=2P-2t,  b=P-t,  c=P,  a-c=P-2t.

After its two outer tests, the sole hard outcome is

    R_t(d) = Sb(d:3,(P-t):1,(P-2t):1) @t.

The construction and published upper bound together make this D slice exact:
`d*=P-Q` for `t=7,8`, and `d*=P-Q+1` for `t>=9`.  Exact local assembly also gives the tied
base `d*=P-Q-1=57` at `t=6`.  The proof, including the recomputed off-by-one index in the paper's
displayed descendants, is in
[the m=5 Pareto-assembly calibration](theorems/m5-pareto-assembly.md).
In particular, the eventual branch has a self-contained two-test reduction to singleton leaves.
One of its `G_(t-2)` prefix inequalities fails at `t=7,8` and all of them hold from `t=9`, so the
second regime change is visible inside D rather than merely fitted to the theorem values.

The local exact replay sees the same transition.  For `Sb(481:5)@9`, every capacity-feasible
`3+2` root is negative: `[a:3]` fails for all 23 values `a=226..248`.  In the `4+1` class,
`[a:4]` fails for `a=225..239` and succeeds for `a=240,241,242`.  A `5+0` split cannot fit
481 because its two `k=8`, `m=5` pure branches total at most `2*231=462`.  Thus every feasible
root is `4+1`, up to complement.  The compact verified witness begins with `[239:1]`, the
complement of the paper's `[242:4]`.  Full diagnostic details are retained in
`evidence/sb_m5_k9_root_transition.txt`; the 481/482 boundary and proof tree are in
`evidence/sb_m5_k9_frontier.txt` and `witnesses/majorized_481_5_at9.tree`.

There is a useful atom arithmetic behind the three regimes.  At normalization `t=k-2`, let

    A=2^t,  B=A-1,  D=A-1-t-binomial(t,2).

Then `3B+D=F(k)`, replacing one `B` by `A` adds one, and replacing two adds two:

    BBBD = F(k),    ABBD = F(k)+1,    AABD = F(k)+2.

At `k=9`, `(A,B,D)=(128,127,99)`, so these masses are 480, 481 and 482, and the exact answer
selects the middle one.  This is an arithmetic interpretation of the published piecewise
formula, **not yet a tree-derived symmetric per-coin profile** for the 481 witness: its leaves
are arbitrary sequences majorized by `G_k`, and `profile_from_tree.py` cannot infer an atom
census from those compressed leaves.  The old `BBBD` construction remains valid for `k>=7`,
but its claim to optimality is refuted.

## Conjecture (u1) - the antidiagonal conjecture

> If `Sb(n1 : n2)` is solvable in `k` for `n1 >= n2`, then so is `Sb((n1+1) : (n2-1))`.

Equivalently `n(k, m-1) >= n(k, m) + 1`: the frontier decreases **strictly** in `m`. Formal
statement in [theorems/special-cases.md](theorems/special-cases.md#conjecture-u1). Still
unproved, but 2026-08-03 mapped it out: the evidence is much stronger than the 130 cells,
two natural proof routes are now **refuted**, and what is left is a single crisp lemma.

### Why it is worth proving

`Sa(193)` in 10 reduces to sixteen states `Sb(n1 : 193-n1)`, `n1=97..112`
([results.md](results.md#sa10--192-proven-maximal)). They are pairwise incomparable under
subgraph monotonicity, so the completed cold proof had to refute all sixteen. Under (u1) they
would chain: refuting the most lopsided `Sb(112:81)` would refute the other fifteen and shrink
this class of negative certificate 16-fold. H3 is now settled without the conjecture, but the
reduction remains valuable for future antidiagonal frontiers.

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
tools/build_radio.py -O3 -DMAX_K=4 -DMAX_N=40 radio_one.c -o /tmp/r4
/tmp/r4 4 15 2 6 3
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

**Two numerical identities on the old data, now both refuted at their first new datum:**

    n(k,5) = n(k-1,2) + n(k-1,6)
    n(k,6) = n(k-1,4) + n(k-1,5)

The first identity is exact only for `k=5..8`.  At `k=9` its right side is
`n(8,2)+n(8,6)=255+225=480`, while the exact left side is 481.  It was numerical only:
`m=6 > 5` cannot appear as a child of an `m=5` part, and the split that would saturate it
(`a=n(k-1,2)`) is not in the working set.

The second identity is exact for `k=5..9` and on that finite range is realised directly by a
split: `b = 2`,
`a = n(k-1,5)`, giving
`n(k-1,5):2` on outcome 2 and `n(k-1,4):4` **saturated** on outcome 0. The `473:6@9` witness
uses the mirror form `[242:4]`, i.e. `a = n(8,4) = 242`, `n-a = n(8,5) = 231`.

The apparent `m=6` recursion fails at its first extrapolation.  With the corrected exact
`n(9,5)=481`, it predicts `496+481=977` at `k=10`, but exact synchronized search proves
`n(10,6)=973`.  A verified tree uses root `[477:2]`, with children

    Sb(477:2),  Sb(496:2,477:4),  Sb(496:4).

Thus the `m=4` pure child remains saturated while the other width retreats from the true
`m=5` frontier 481 to 477, a loss of four.  Separately, the old `BBCD`/closed-form model still
predicts 976; its continuation through `Sb(496:2,480:4)` reaches the impossible `Z_7` kernel.
The new mixed state avoids it.  This establishes a break, **not** the replacement formula
`n(k-1,4)+n(k-1,5)-4`.  Its natural `k=11` lift reduces to
`Sb(503:1,495:2,478:3)@9`; a five-minute exact run was inconclusive.  The two-part/mixed-state
frontier, rather than either fitted witness, is where the work now is.  In particular, literally
scaling the witness's next split produces the exactly unsolvable residual
`Sb(247:1,247:1,240:2,231:2)@8`; that continuation is refuted even though the parent state remains
open.

## Excess-q Pareto assembly as a variable-D slice (working hypothesis, 2026-08-14)

**Status — parked 2026-08-16.**  The reduction and all results below are retained, but this is no
longer an active construction programme.  The exact `m=5` calibration confirms the local geometry
and black-box use of A/B/C, while showing that a global answer already needs competing outer
families and a piecewise synchronized D frontier.  The height-6 work supplies strong conditional
constructions and obstructions inside one aligned family, but neither proves the sufficiently-large-
`q` postulate nor exhausts unrestricted strategies.

Reopening requires a genuinely global bridge: a theorem that the outer family enumeration is
complete, a refinement-stable exact recurrence for the guarded mixed-child antichain, or an
all-depth construction/obstruction connecting successive normalizations.  More finite ranks,
bounded-depth exclusions, or fitted atom words alone do not address the missing implication.

The corrected diagram supplied later on 2026-08-14 makes the geometry explicit and supersedes the
initial attachment.  Write the chosen one-part Pareto states as

    A=(a:alpha) @ k-1,
    B=(b:beta), C=(c:gamma) @ k-2.

A is the upper-left rectangle, B the upper-middle rectangle, C lies below the right end of A, and D
is the upper-right segment of the magenta four-segment staircase.  For total parent height `m`, that
staircase is exactly

    S_D(d) = Sb(d:beta,
                b:alpha-beta,
                c:m-alpha-gamma,
                a-c:gamma) @ k-2.                 (1)

Zero-width or zero-height parts are omitted.  The drawn non-overlap conditions are
`beta<=alpha`, `alpha+gamma<=m`, and `c<=a`; the proposed additional admissibility condition
`m<=2a` is retained as a working hypothesis rather than derived from the picture.  Once A/B/C and
`m` are chosen, `d=width(D)` is the only free dimension, and the parent candidate is

    N = a+b+d.                                    (2)

Here A/B/C/D name diagram components, not the dyadic atom types denoted by the same letters elsewhere.

The working assumption is deliberately stronger than anything proved here:

> For every fixed total height and labelled placement (1), some sufficiently large `q` reaches a
> stable atomic-leaf construction regime.

No threshold or proof of existence is claimed.  Lineages remain labelled while taking this limit;
ordinary normalization of equal-looking rectangles must not erase which diagram branch they came
from.

### The exact D problem, then its deficit form

Put `s=k-2`.  The hard part of the proposed assembly is now the concrete one-dimensional frontier

    d*(A,B,C,m) = max { d : S_D(d) is solvable in s }.       (3)

It is enough to consider `0<=d<=2^s`: the variable part contains `Sb(d:1)` as a substate, whose exact
maximum in `s` tests is `2^s`.  Subgraph monotonicity makes feasibility downward closed in `d`, so an
exact adjacent negative/positive pair certifies (3).  Formula (2), followed by maximization over the
permitted Pareto triples and height allocations, is precisely the proposed candidate generator.

To seek a scale-free expression rather than scan every `d`, write

    D = (2^s-delta : beta).

Every viable wide-side cut has the form

    y = 2^(s-1)-u,       (2^s-delta)-y = 2^(s-1)-v,       u+v=delta.

If the selected narrow-side height is `x`, D contributes

    outcome 2: (2^(s-1)-u : x)
    outcome 0: (2^(s-1)-v : beta-x)
    outcome 1: (2^(s-1)-u : beta-x), (2^(s-1)-v : x).

Thus maximizing D is exactly minimizing `delta`.  Fix synchronized cuts `sigma` of the other three
parts in (1), and a height cut `x` of D.  Let `E_2(sigma)`, `E_1(sigma)`, and `E_0(sigma)` be those
fixed parts' contributions to the three children.  For any chosen constructibility predicate (`C_r`
for bounded singletonization search, or `C_s` for exact solvability), define:

- `U_2` as the least `u` making the outcome-2 child constructible;
- `U_0` as the least `v` making the outcome-0 child constructible; and
- `M` as the Pareto-minimal pairs `(p,q)` making the mixed child constructible.

Constructibility is upward closed in deficits by subgraph monotonicity.  Consequently the exact
answer for these fixed cuts is

    delta*(sigma,b) = min_{(p,q) in M} [max(p,U_2) + max(q,U_0)].

The proof is immediate in both directions.  Every feasible `(u,v)` must exceed both pure thresholds
and dominate some minimal mixed pair.  Conversely, for any `(p,q)` in `M`, choosing
`u=max(p,U_2)` and `v=max(q,U_0)` satisfies all three children.  Minimize this expression over
`sigma` and `x`, inside the legal box `0<=u,v<=2^(s-1)`; an empty feasible set has value infinity.
Zero-height terms at `x=0` or `x=beta` are simply omitted.

This identifies the irreducible object: it is the **two-dimensional mixed-child deficit antichain**
`M`, not a single majorization capacity.  A scalar formula for D can exist only after that antichain
has acquired a stable describable form.

Every finite antichain can be split into maximal guarded slope-one pieces

    L <= p <= R,       q=C-p.

For one such piece, put

    I = [min(U_2,C-U_0), max(U_2,C-U_0)].

The piece contributes

    max(C,U_2+U_0) + distance([L,R], I)

to `delta_D`.  This is exact: without the guard, the convex objective is constant on `I`; moving to
the nearest allowed integer endpoint raises it by exactly the interval distance.  Taking the minimum
over pieces, fixed-part cuts and D height cuts gives D without enumerating the individual antichain
points.  If `L,R,C` eventually have polynomial descriptions in the residual level, this is already a
scalable max/min expression.

In the diagram's levels, write

    width(A) = 2^(k-1)-delta_A,
    width(B) = 2^(k-2)-delta_B,
    width(D) = 2^(k-2)-delta_D.

Then each parent candidate is

    2^k - (delta_A + delta_B + delta_D).

So candidate comparison is also deficit minimization.  Enumerate the permitted A/B/C choices,
compute the three fixed parts in (1), maximize D, then retain the smallest total deficit for each
total height before Pareto filtering.

### Finite exact check and present boundary

`tools/search_singletonization.cpp assembly` executes (1) directly and scans `d` downward with one
retained exact memo.  Its `assembly-enumerate` mode now reads only source-carrying, proven-maximum
rows from `data/pareto_sb.csv`, rejects a level containing unresolved rows, and enumerates every
ordered A/B/C triple satisfying the three drawn inequalities plus the explicitly unproved
`m<=2a` condition.  For each triple it binary-searches the full-star necessary condition to obtain
a sound upper bound on `d`, ranks by `a+b+d`, and then runs the exact residual recurrence only while
a triple can still tie the incumbent.  Larger `d` values are excluded by full-star majorization;
smaller skipped values cannot tie by arithmetic.  Thus `optimization_complete=YES` certifies the
widest member of this specified assembly family even when `all_triple_dmax_exact=NO` says that some
irrelevant losing slices were not solved to their individual boundaries.  `assembly-rank` emits the
entire static ranking and exits before exact search, so a difficult next-level slice cannot hide the
completed enumeration.

The exact `m=10` regression (source: `tools/singletonization_regression.sh`, with Pareto inputs
sourced row-by-row by `data/pareto_sb.csv`) gives:

- parent level 5: 68 admissible triples, 3 surviving the full-star bound, and exact family optimum
  12, attained by 2 triples;
- parent level 6: 133 admissible triples, 21 surviving the bound, and exact family optimum 33,
  attained by 4 triples; and
- parent level 7: 165 admissible triples, 37 surviving the bound, and exact family optimum 82,
  attained uniquely by the displayed triple below.

Every winning branch tree is independently checked.  In particular, the earlier hand controls are
recovered:

- at parent level 5, `A=(7:6)`, `B=(4:4)`, `C=(5:3)` give `d*=1` and candidate 12;
- at parent level 6, `A=(19:6)`, `B=(10:4)`, `C=(12:3)` give `d*=4` and candidate 33; and
- at parent level 7, a frontier-reaching triple is `A=(46:6)`, `B=(22:5)`, `C=(27:3)`, giving
  `S_D(14)=Sb(27:1,22:1,19:3,14:5)@5`, `d*=14`, and candidate 82.

These reproduce the proven Pareto rows 12, 33, and 82.  They validate the corrected geometry, the
finite D maximization, and completeness of the enumerated *working family* at those inputs—not the
`m<=2a` hypothesis, the eventual-`q` assumption, or completeness among arbitrary constructions.

At parent level 8 and `m=10`, `assembly-rank` completes without entering the hard exact recurrence:
165 triples are admissible and 37 survive the full-star bound; the largest necessary candidate
bound is 195 while the proven parent maximum is 189.  One especially simple width-189 target,
`Sb(50:4,39:6)@6`, is exactly negative.  These are finite regression facts from
`tools/singletonization_regression.sh`; the complete level-8 assembly optimum remains unresolved.

The generic `slice` mode is the same finite D problem after the fixed three parts have been formed:
it adds `(2^s-delta:h)` to arbitrary fixed residual parts and scans deficits upward.  Its independent
synchronization regression uses the adjacent pair from
[the Singleton Majorization note](theorems/singleton-majorization.md#why-there-is-no-single-width-two-base-sequence)
after applying subgraph monotonicity.
The note supplies the variable-width-11 negative and a variable-width-10 positive superstate; by
subgraph monotonicity the fixed state `(11:2,9:2,3:2)@4` has exact D-maximum 10.  Full-star
majorization still permits 11, but exact synchronization rejects it.  The positive tree is checked by
`tools/check_witness.py`; `tools/singletonization_regression.sh` locks the pair and abort semantics.

The `mixed-frontier` mode computes the second-order object `M` needed to solve a chosen slice
recursively.  For the same generic fixed parts `(9:2,3:2)@4`, its
complete exact frontier for two variable height-two parts is

    {(2,10), (3,8), (4,6), (6,4), (8,3), (10,2)}.

In particular `(4,6)` is feasible while the equally costly balanced pair `(5,5)` is not.  This is the
two-coordinate synchronization notch that a scalar total deficit would erase.  As an illustrative
combination, imposing the synthetic pure thresholds `U_2=U_0=5` makes the piece optimizer return
`delta_D=11`: synchronization costs one unit beyond the unattainable scalar value 10.  Those pure
thresholds are a generic regression choice, not a value derived from assembly (1).

Two unforced families show what “stabilization” should mean computationally.  The regression performs
complete exact scans and verifies every positive tree:

- for residual levels `3<=s<=11`, variable heights `(1,2)` give one piece
  `0<=u<=s`, `v=s+1-u`;
- for `4<=s<=11`, variable heights `(2,2)` give the integer minimal boundary of
  `u,v>=1` and `u+v>=2s+indicator(min(u,v)<=2)`, compressed into three pieces; and
- at level `s=6`, heights `(1,4)` already require three guarded pieces:
  `u=0..2, v=17-u`; `u=4..7, v=18-u`; and `u=9, v=19-u`.

These are finite exact facts sourced by `tools/singletonization_regression.sh`, not proofs of the
displayed formulae outside their checked ranges.  They also show that the antichain need not stabilize
as a list: the `(2,2)` frontier has `2s-1` points.  What can stabilize is a finite guarded-piece
description whose endpoints and sums depend on `s`.

The corrected diagram removes the supposed missing A/B/C map: the three fixed parts are already
given by (1), and the finite triple enumerator now exists.  If the track is reopened, the unfinished
finite engineering task is to schedule the still-competitive level-8 slices without letting one
hard negative block every later triple, while retaining an exact proof obligation for each skipped
query.  The missing scale-free mathematics is a proof that eventual dyadic-polynomial formulae for
`d*`—or for its guarded mixed-frontier pieces—survive atom refinement.

### Ground-up atom induction: the outer states are black boxes

The construction does **not** need the internal solution trees of A, B, or C.  They occur in
separate adaptive branches, so their witnesses can be attached independently after the two outer
tests have selected a branch.  The four-segment state (1) depends only on the six outer scalars
`a,alpha,b,beta,c,gamma`.  In particular, `a-c` is an arithmetic width difference, not a remainder
of A's witness tree.  Keeping the A/B/C/D labels while forming (1) is bookkeeping for the diagram;
it is not an atom-by-atom alignment condition.

This makes induction on total height possible.  For fixed `m`, every admissible choice has
`alpha,beta,gamma<m`; the already constructed one-part widths at those heights supply `a,b,c`, and
only the synchronized D branch is new.  Use `A_r,B_r,C_r,D_r` below for the first four atom values
of `G_r` (not the diagram components), with refinement

    A_{r+1}=2 A_r,
    B_{r+1}=A_r+B_r,
    C_{r+1}=B_r+C_r,
    D_{r+1}=C_r+D_r.                              (4)

The first nontrivial height steps then work directly in atoms, conditional only on the proposed
outer assembly.

**Height 4.**  Put `s=k-2`, `r=s-1`, and choose
`(alpha,beta,gamma)=(2,2,1)`.  The known height-1 and height-2 constructions give

    a=B_{s+1},  b=B_s,  c=A_s,  d=2 C_r.

The hard branch is

    Sb(2 C_r:2, A_s:1, B_s:1) @ s.

One test splits the two copies of `C_r` evenly and uses the decompositions
`A_s=A_r+A_r`, `B_s=A_r+B_r`.  Its two pure children are the singleton state
`(A_r,C_r)` and its mixed child is `(A_r,B_r,C_r,C_r)`, a prefix of `G_r`.  Hence the branch is
solvable, while (4) gives

    a+b+d = 4 A_r+2 B_r+2 C_r = R(AACC).

Thus the ground-up assembly produces the `AACC@G[k-2]` lower-bound construction (for `r>=2`; the
smaller established cases remain finite base cases).  This is an achievability statement, not a new
maximality proof.

**Height 5.**  Put `s=k-2`, `r=s-2`, and choose
`(alpha,beta,gamma)=(3,2,2)`.  The height-2 and height-3 constructions give

    a=A_s+C_s,  b=c=B_s,
    a-c=A_{s-1}+C_{s-1},
    d=B_{s-1}+D_{s-1}.

At level `r` the hard branch therefore has the three width profiles

    (ABCD:2), (AAAB:1), (AABC:2).                 (5)

There is a uniform two-test construction for (5).  On the first test take respectively
`(AB:2)`, `(AB:0)`, and `(AC:1)`.  One pure child is the first two atoms of `G_{r+1}`.  The other
pure child is `(AB:2,AC:1)` and is resolved by taking `(B:1,A:1)`.  The mixed child is

    (AB:1, AB:1, AC:1, CD:2).

Resolve it by taking `B:0` from the first `AB`, `A:0` from the second, `A:1` from `AC`, and
`h:1` from `CD`, where `h=floor((C_r+D_r)/2)` and
`h'=C_r+D_r-h`.  Its singleton children are majorized by `G_r`: the only non-immediate case is
`(A_r,B_r,C_r,h,h')`, for which `h'<=C_r` and `h+h'=C_r+D_r`, so its first five prefix sums are
bounded by `(A_r,B_r,C_r,C_r,D_r)`.  This proves (5) for `r>=3`.  Finally,

    a+b+d = 3 B_s+D_s,

so the construction has profile `BBBD@G[k-2]`, equivalently the old height-5 formula, for
all `k>=7`.  This proves a lower bound only.  Equality is now **refuted** at `k=9`, where the
published exact answer and independent replay gain one coin via the `4+1` regime above.

The complete height-5 calibration therefore retains both outer families rather than replacing this
proof.  Exact assembly chooses `(3,2,2)` through `k=7`, ties it with `(4,3,1)` at `k=8`, and
chooses `(4,3,1)` at `k=9`; the latter branch then realizes the published exact formula for every
larger `k`.  Its D maximum is piecewise even though the outer height triple is unchanged.  See
[the exact reconstruction](theorems/m5-pareto-assembly.md) and the locked finite controls in
`tools/singletonization_regression.sh`.

**Height 6 is exactly the first synchronization obstruction.**  The repeated finite winner uses
`(alpha,beta,gamma)=(4,3,2)`.  With `r=k-4`, the refuted `BBCD` continuation would require the hard
branch

    Sb(ABBD:3, AABC:1, ABCC:2) @ r+2.             (6)

It works through the finite `k<=9` data but fails at `k=10`: at `G_6`, its D word `ABBD` has width
232 and would give the impossible parent width 976.  The exact height-6 frontier instead uses
`d=229`, giving the verified hard state `Sb(229:3,241:2,248:1)@8` and parent width 973.  After one
more refinement, at `G_5`, the failed D word is `AAAABBCD` (width 232), whereas one eight-atom
accounting of the exact width 229 is `ABBBBBCD`: three `A` atoms have been replaced by three `B`
atoms.  This is a precise finite synchronization loss of three, not evidence for a constant
correction at later levels.

At the same `G_5` normalization, the outer A and C widths have profiles
`A^10 B^4 C^2` and `A^7 B`; hence `a-c=A^3 B^3 C^2` is nonnegative.  This supports—but does not
prove—the working expectation that C fits inside A after sufficient refinement.  The resulting
parent accounting `A^16 B^11 C^4 D` is likewise only an outer width decomposition; it is not claimed
to be a symmetric profile derived from the stored witness tree.

The successful first split of the finite hard branch does have an exact atom description one level
lower.  Refine its three width words from `G_r` to `G_(r-1)`.  The refined totals and the selected
eight-atom subwords are

    A^5 B^2 C   -> A^12 B^3 C    : select A^4 B^3 C  at height 0,
    A^3 B^3 C^2 -> A^9 B^5 C^2  : select A^7 B      at height 2,
    A B^5 C D   -> A^7 B^6 C^2 D: select A^3 B^3 C D at height 1.       (7)

The complements are respectively `A^8`, `A^2 B^4 C^2`, and `A^4 B^3 C`.  At `r=5`, (7) evaluates
to the stored split `[120:0,127:2,109:1]`.  Its mixed child is

    (A^4 B^3 C:1)^2, (A^2 B^4 C^2:2), (A^3 B^3 C D:2).                (8)

This exposes exactly why repeating the finite split is not an induction.  At `r=6`, (7) evaluates
to `[247:0,255:2,231:1]`, and (8) becomes
`Sb(247:1,247:1,240:2,231:2)@8`, which is exactly unsolvable.  The source is
`evidence/m6_k11_scaled_attempt.txt`.  Thus the literal refinement of the `k=10` split fails at the
next level.  This does **not** refute the parent `Sb(503:1,495:2,478:3)@9` or another unrestricted
first split.  The all-depth lineage argument below now separately rules out eventual reuse of the
same one-D germ inside the aligned profile model.

There is nevertheless a finite optimizer at each fixed power-of-two normalization in the explicitly
restricted aligned model.  Represent every width by `p=(p_A,p_B,p_C,p_D)` with `|p|=N`.  One
refinement is

    R(p)=(2p_A+p_B, p_B+p_C, p_C+p_D, p_D).

A legal aligned cut chooses `x<=R(p)` with `|x|=N`; its complement is `R(p)-x`, and the usual height
cut produces the three child states.  At `N=8`, the terminal reference profiles—the first six atoms
of `G_(r+3)`, refined to `G_r`—are

    A^8, A^7B, A^4B^3C, A^4B^3C, AB^3C^3D, AB^3C^3D.                 (9)

No numerical scale is needed to order candidates.  The deficit of `p` is

    p_D binom(r,2) + (p_C+p_D)r + (p_B+p_C+p_D),                    (10)

so eventual width is decreasing lexicographic order of
`(p_D,p_C+p_D,p_B+p_C+p_D)`.

The recursion now has an all-depth symbolic obstruction.  Let `L_D(S)` be the sum of `p_D` over
state parts, **without** multiplying by their heights.  Refinement preserves each D atom and the
mixed outcome only partitions those atoms, while preserving total height.  Following mixed outcomes
therefore proves that every constructible height-`h` state needs

    L_D(S) >= max(0,h-4).                                           (11)

At height 6 the two fixed profiles in the hard branch contain no D, so every D germ with at most one
D atom is impossible at every depth.  For eight atoms these are exactly ranks 1--81.  Thus the old
`AAAABBCD` germ (rank 56) and the finite 229 accounting `ABBBBBCD` (rank 59) do **not** stabilize in
this model; increasing `q` by pure refinement cannot repair their single D lineage.

Rank 82, `A^6D^2`, is the first survivor, and exact recursion finds a three-level construction.  An
independent 19-node tree checker gives root-base threshold `r>=12`.  Hence it is the exact widest
A--D eight-atom D germ at *all* depths, not merely at depth 3.  Attaching the already constructed
outer branches gives the conditional parent profile `A^21B^6C^3D^2@G_(k-5)` and width

    2^k-k^2+6k-16,       k>=17.                                    (12)

That eight-atom answer is not stable under arbitrary excessive `q`.  At 16 atoms the same lineage
certificate excludes ranks 1--289 (including the refined 229 class at rank 191), and a finite
242-core coinductive kernel in the sound `(p_D,p_C+p_D)` abstraction excludes ranks 290--304 at
every depth.  The first retained projected tree for rank 305, `A^13CD^2`, has no exact lift, but an
all-skeleton product search finds a different 19-node exact tree.  Its root-base threshold is
`r>=6`, so rank 305 is the exact sixteen-atom optimum and gives the conditional parent profile

    A^49B^9C^4D^2@G[k-6] = R^2(A^7B^7D^2)@G[k-6]

with width

    2^k-k^2+7k-21,       k>=12.                                  (13)

The compact `A^7B^7D^2@G[k-4]` spelling is the old spreadsheet's `p6'` row: it reproduces 473 at
`k=9`, the exact 973 at `k=10`, and predicts 1983 at the still-open `k=11`.  The construction needs
the twice-refined 64-atom realization; the compact spelling alone does not lower its proved atomic
depth.  At 32 atoms, D lineage and a checked 504-core `(D,C+D)` kernel exclude ranks 1--1179 at
every depth.  Rank 1181, `A^26BC^3D^2`, is the pure refinement of the rank-305 construction, leaving
only the wider rank 1180, `A^27C^3D^2`, unresolved in that slice.  Exact propagated-loss search now
excludes that profile through depth three.  At depth four, solving the two pure outcomes exactly
leaves 1,818 distinct mixed children, and loss-sliced guided cover exhausts all of them.  Hence no
aligned tree of depth at most four exists, but eventual constructibility is still undecided.  Thus
arbitrary excessive `q` remains open.  Within this one slice, the unresolved symbolic decision is a
single profile requiring either a depth-at-least-five construction or an all-depth obstruction,
rather than a band; it is recorded but not scheduled.  Full
proof and scope are in
[the atom-lineage note](theorems/atom-lineage.md).

There is now a scale-free objective for that decision.  At normalization `N=2^s`, write the D germ
as `A^(N-b-c-2)B^bC^cD^2`.  Attaching the three known outer branches gives width

    2^k-k^2+(2s-c)k-s^2-3s+c(s+1)-b+2.

Thus eventual maximization at fixed `s` is lexicographic: minimize `c`, then `b`.  For `s=5`, the
all-depth projected kernel fixes `c=3`; the checked construction has `b=1`, while `b=0` is the
unique wider all-depth candidate still open.  Its formal parent is `A^108B^12C^6D^2@G[k-7]` and
its width is `2^k-k^2+7k-20`; exact cover shows that any construction for it must have depth at
least five.  The user's sufficiently-large-`q` postulate licenses the eventual comparison; it does
not supply this missing construction.  This row is deliberately not added to
`data/conjectures.csv`: no starting `k` or construction is known, and the next normalization could
change the optimum again.

The scalar supply calculation can be completed at every finite depth.  If `t` mixed transitions
remain, it requires

    c >= max(0,2s-1-2t),

and, when the unclamped right side is positive and the middle coordinate ties,

    b >= max(0,s^2-2s-2st-t+t^2+5).

At `t=3`, the pure-refinement boundary from the checked sixteen-atom germ has
`c=2s-7,b=(s-4)^2`; the necessary lower boundary permits only five persistent B savings.  Saving
one starts with rank 1180 at 32 atoms, savings two through four first appear at 64 atoms, and saving
five first appears at 128 atoms.  This reduces the whole depth-three boundary to five candidate
tracks.  It does not solve arbitrary excessive `q`: once `t>=s`, even `b=c=0` passes the scalar
supply relaxation, so an all-depth conclusion needs a synchronized closed kernel or recurrence.

At depth four, exact solution of both pure children shows that any surviving first mixed transition
must preserve all D and `C+D` supply and lose between one and fourteen units of `B+C+D`.  This leaves
6,696 oriented first tests and 1,818 distinct hard children in fourteen loss classes.  Guided exact
cover returns `NO` on every child, proving the bounded depth-four exclusion.  Both the reduction and
the exact recursion use only outer dimensions; they do not inspect the solved internals of the A/B/C
branches.  Scalar supply itself cannot extend this result to all depths; a future resolution would
need a synchronized closed kernel/recurrence or a deeper positive tree.

`tools/search_atom_profiles.cpp` implements both normalizations and the two-coordinate abstraction;
`tools/check_dc_tree_lift.py` supplies the exact-coordinate product search.
`tools/atom_profile_regression.sh` invokes independent checkers for the D-lineage certificate, the
two-coordinate kernel, the failed first lift, and all retained positive trees;
`tools/check_atom_profile_cover_log.py` replays the depth-four loss partition and accounting.  This
is the concrete version of the abstract `H` state below and still uses only the outer profiles of
A/B/C.

### Minimal state for a height-first D optimizer

No routed catalog for A/B/C is needed.  A scale-free implementation needs only two kinds of entry:

    P[m,q] = outer width germ (and one proof pointer) for the constructed one-part state;
    H[q; (w_1:h_1),...,(w_j:h_j)] = exact constructibility of a labelled mixed state.

Here a width germ may be stored either as an atom-count vector after refinement to a common `G_t`,
or equivalently as its binomial-deficit polynomial.  For each new `m`, refine the already known
`P[alpha,q-1]`, `P[beta,q-2]`, and `P[gamma,q-2]` to the common base, form the three fixed widths
`b,c,a-c`, and ask `H` for the largest D germ.  The parent comparison is then the scalar deficit
comparison `delta_A+delta_B+delta_D`; the A/B/C proof pointers are used only after a winner has been
chosen.

If the cell interpretation requires whole atoms, `c` fitting in `a` means only that the scalar
difference `a-c` has a nonnegative atom decomposition at the selected refinement.  It does not mean
that C must select cells from a previously fixed decomposition of A.  Likewise, if the maximal
integer D has no decomposition with exactly `2^(q-2)` atoms, a larger normalization may expose a
new profile.  That is not the same as purely refining the old profile: the 229 value first acquires
an eight-atom accounting one level below its failed four-atom attempt, but its single D lineage now
proves that this refinement class cannot stabilize.

The computational stabilization test is therefore

    P[m,q+1] = R(P[m,q])

together with refinement-compatible guarded pieces for the D mixed-child frontier.  Equality of
numeric widths or atom counts alone is only discovery evidence.  A scalable proof still requires a
symbolic split template (as above for heights 4 and 5), or an induction showing that the guarded
frontier description itself refines.

The rank-1180 depth-four closure makes the required induction precise.  Let `I` be the immediate
lineage/majorization failures and let `up(K union I)` mean every state containing a listed losing
core as a substate.  A finite exact antichain `K` is an all-depth D certificate exactly when

    for every S in K and every legal test x,
    some child H_o(S,x) lies in up(K union I).

If the track is reopened, this is the appropriate representation for `H`: minimize the bounded
exact-negative memo into candidate cores, check the displayed closure without a depth parameter,
and add uncovered children until it closes or leaves an explicit transition graph for constructive
search.  If rank 1180 lies
in the final upward closure,
the `-21` family wins the 32-atom slice at all depths.  If not, the uncovered transitions are the
only candidates for a depth-at-least-five `-20` construction.  Either outcome reuses only outer
profiles and proof pointers; no inner A/B/C catalog is introduced.

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
  `Sb(496:2,477:4)@9`, so the old `BBCD`/closed-form value 976 is dead.  The open problem is a
  parametric construction or obstruction for this `m=2 + m=4` mixed-state frontier; the parked
  aligned assembly is not presumed to supply it.  Do not fit
  the 973 witness's later subtree or infer either a constant three-unit correction to the old
  formula or a four-unit correction to the finite recurrence from one level.
- **Proving rather than fitting the closed forms.** Lemmas 1-5 have real inductive proofs;
  Aigner and Li--Wu--Triesch supply published matching upper bounds through `m=5`.  No general
  exact formula beyond `m=5` is known, although the retained replay makes the individual
  `n(9,6)=473` and `n(10,6)=973` boundaries exact.
