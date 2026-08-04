# Status

**Read this first.** Where everything stands, and what will silently ruin your work if you
don't know it. Last refreshed **2026-08-04**.

This page says where things *stand*. For what happened and why, see
[journal.md](journal.md); for what to do next, [research-plan.md](research-plan.md).

## Active traps

Each of these has already caused, or was one step from causing, a wrong result.

| trap | why it matters |
|---|---|
| **Never warm-start a *negative* result from `cache-2025:parsed_260.txt`.** | It contains the 16 `Sa(193)` verdicts under suspicion. Loading it re-reads the old answers and "confirms" them. It cannot be filtered: the cache spans 2023–2025 and does not record which build wrote each line. Fine for *finding* solutions — a poisoned negative only slows a search, never corrupts it, because any solution found is re-verified as a tree. |
| **Never promote a 2023-era negative above `legacy`.** | That build emits false negatives — 37 known, ~0.27%, with **no syntactic marker**. `Sb(143:17)` in 8 was declared unsolvable after 10 passes and 4 days, and is wrong. See [`../evidence/refuted_2023_negatives.txt`](../evidence/refuted_2023_negatives.txt). |
| **Do not "upgrade" the paper's `k ≤ 9` optimality claim to `k = 10`.** | The claim as written is exactly right. `Sa(10) = 192` maximality rests on the suspect 2023 run. |
| **`out26_1.txt` / `out26_2.txt` exist twice under the same names.** | ~130-byte stubs in `fullsolve-2026`; the 905 MB / 51 MB originals in `sa193-2023`. Only the latter are evidence. Pulling the wrong tag yields nothing, silently. |
| **A missing `can't solve` line does not mean unsolvable.** | `canSolveB` returns a tri-state and gives up with `MAYBE` on a deadline, printing nothing. Absence of a verdict is not a verdict. (Briefly narrowed on 2026-08-04 when deadlines were disabled; that change was reverted the same day — disabling them trapped a real run for six hours. A printed `can't solve` is exhaustive either way, since it is emitted only when `!skipped_some`.) |
| **Fits with fewer than ~4 data points are meaningless.** | The Pareto data thins out fast: m ≥ 33 has a single k value. A profile or closed form fitted there is unconstrained. |
| **Never add "move a coin to the larger side" to `compare_solvability`.** | Conjecture (u1) is unproven, and its multi-part form is outright **false**: `Sb(15:2, 5:4)` is solvable in 4, `Sb(15:2, 6:3)` is not, despite lower mass. Wired into the cache as a dominance rule it would manufacture false negatives — the exact failure mode that makes the 2023 corpus unusable. Only *componentwise* part dominance is sound; see [theorems/subgraph-monotonicity.md](theorems/subgraph-monotonicity.md). |

## Goals

| | goal | state |
|---|---|---|
| **H1** | Publish | Draft in `paper/`. K≤8 table and both theorems are solid; `<TODO>` sections and the stale K=8 column remain. See P5. |
| **H2** | The K=9 Sb column | **Main open front.** Six lower bounds at small m; `legacy` bounds at m=65..96; the band **m = 7..64 is entirely blank**. |
| **H3** | Is `Sa = 192` maximal at k=10? | **Open.** A 2023 run says yes over ~47 days, but that corpus is unreliable. Needs a cold re-run of **all 16** states — conjecture (u1) would collapse them to one, but it is unproved and 2026-08-03 closed the two obvious routes to it. |
| **H4** | Structural theory | Active and promising — see below. |

## What is established

Facts live in `data/*.csv` with per-cell `bound`, `status` and `source`;
[results.md](results.md) narrates them. Summary:

- **Sb Pareto frontier, K = 1..8** — proven maximal, artifact-backed. 129 of 130 cells have
  both certifying log lines located; the exception is the trivial `k=8, m=1` dichotomy. The
  30 KB of certifying lines is committed at
  [`../evidence/pareto_certification_k1_8.txt`](../evidence/pareto_certification_k1_8.txt),
  so the provenance survives the 1.8 GB of logs.
- **Sa sequence, k = 1..9** — proven maximal. `Sa(192)` in 10 is a verified construction.
- **Three theorems** — Singleton Majorization, Unit-Group Elimination and Subgraph
  Monotonicity, all proved. The third is elementary but was load-bearing and unwritten: it is
  what the result cache's downward/upward closure and the whole `sbb_greater` relation rest
  on, and what lets a negative certificate store antichains instead of closures.
- **15 verified witness trees** — `Sa(38)` through `Sa(192)`, plus canonical trees for
  `Sb(248:3)@8`, `Sb(496:4)@9`, `Sb(480:5)@9`, `Sb(473:6)@9`, and a two-sided-only
  `Sb(480:5)@9`. All pass
  `tools/check_witness.py`, which re-derives every step without consulting the solver.
- **16 exhaustive multi-part enumerations** — `data/exhaustive_multipart.csv`, including one
  proven negative.

## What is refuted

Kept on record so it is not re-derived: the `m=11` closed form (violates monotonicity in m),
the hand-typed `409?` as a *derivation* (though see below — it is *consistent* with the
profile model), and 31 verdicts from the 2023 build.

## Where H4 stands — the most active thread

For fixed m, `n(k,m)` appears to be a fixed multiset of atoms drawn from the base sequence
`G_{k−q}` (a "profile"). Established this session:

- Profiles form **refinement classes** under `A→aa, B→ab, C→bc, D→cd`. Refinement preserves
  the value function and doubles the length, so choosing a larger `q` adds **no** expressive
  power. Verified against the spreadsheet's own columns.
- **`length = 2^q` is a refinement invariant** — a property of the whole class, not of the
  shortest representative. Holds for every m ≤ 13.
- Given that constraint the profile is **unique** for m ≤ 9 and m = 11..13, hence so is its
  `n(9,m)` prediction: `512, 511, 503, 496, 480, 473, 457, 447, 432`, then `414..416` (m=10,
  two fits), `410`, `395`, `388`.
- The journal's "m=11 first large jump to length 64" **dissolves** if the single
  exactly-diagonal cell `Sb(11:11)` is treated as pre-stabilised; m=11 then fits at length 16
  like its neighbours. Not airtight — m=4 includes *its* diagonal cell and fits fine.
- **The profile now has a mechanism, derived from the trees rather than fitted
  (2026-08-03).** Fix an m-side coin; over the `k-t` tests above normalisation level `t` it is
  taken or not, giving exactly `2^(k-t)` paths, each ending in one n-side chunk that must be
  an atom of `G_t`. The profile is that multiset. So `length = 2^q` counts paths, refinement
  invariance is automatic, and the whole-tree census is `m` copies of it. Verified on 11 of
  14 committed solutions; `tools/profile_from_tree.py`. Full argument in
  [conjectures.md](conjectures.md#the-profile-has-a-mechanism-derived-2026-08-03).
- **Invariants (2026-08-03):** across the 9 solutions of `Sb(480:5)@9` only total mass and the
  atom count `m·2^(k-t)` are invariant; the atom census takes **3 distinct values**. So no
  invariant selects "the" solution — the space splits into genuinely inequivalent classes, which
  reframes the canonicalisation programme: the profile is an invariant of the
  *symmetric, non-wasteful* class, not of the state.
- **The canonical searches report one tree per *top split*, not all trees.** `search_state`
  returns on the first successful subtree and memoises it. `Sb(473:6)@9` has exactly one working
  top split (`[242:4]`), so each mode returns exactly one tree — yet restricted and unrestricted
  return *different* subtrees under it, proving several exist. Do not read "1 tree" as "one
  solution", and do not read it as evidence that no symmetric tree exists.
- **m=6's extra depth is forced.** `target_k=4` on `Sb(473:6)@9` returns `NO_CANONICAL_TREE`:
  no solution has all leaves canonical above depth 3, against depth 5 for m=5 and 6 for m=3,4.
  So `q` jumps 4 -> 6 (profile length 16 -> 64), skipping 32 — the same gap seen at m=11.
- **`Sb(473:6)@9` does not exhibit the profile** — asymmetric, and it wastes 7 paths. This
  answers the journal's headline open question negatively *for that witness*; whether a
  symmetric one exists is open.
- **Reframed obstruction:** `q(m)` is not arbitrary, since `q = k - t`. The real question is
  how deep a solution must go before every leaf is a singleton.
- **n-side splits — ANSWERED (2026-08-03): one-sided is not necessary.** Two-sided-only
  canonical trees exist for both `Sb(480:5)@9` (5 trees) and `Sb(473:6)@9` (1 tree), verified.
  The latter is decisive: the unrestricted witness uses 5 one-sided splits and all are
  avoidable, on two states at one k. Earlier notes below are superseded.
- **Caveat: orientation flips.** A part is stored `n >= m` and a child can invert that, at which
  point the fixed-m-side path model miscounts. The two-sided `473:6` tree has 1 flip, so its
  waste and symmetry figures are not meaningful. Trees with 0 flips — the unrestricted `473:6`
  and all nine `480:5` — are unaffected. `profile_from_tree.py` now detects and refuses.
- *(superseded)* **n-side splits — unresolved.** Zero working splits leave the n-side whole in
  full enumerations of six frontier states (m=5,6 at k=5,6,7) or in the mixed child one level
  down. The 34 instances in the unrestricted `Sa` trees are **genuine** but avoidable: every
  state tested exhaustively also admits a fully two-sided split, though those are rare (76 of
  5440 in one case). The 5 in the canonical trees are no-ops. **The open question is
  compositional** — local two-sidedness at every node does not imply a complete two-sided tree,
  since taking the two-sided split changes the children. `radio_canon_twosided.c` now exists
  for this: `TWO_SIDED_ONLY=1` rejects any split leaving a non-nil part's n-side whole.
  **First result: m=5 at k=9 survives the restriction** — 5 verified trees in
  `witnesses/canon_480_5_at9_twosided.tree`, 0 one-sided splits in 474 entries. The m=6 run
  was killed for memory before finishing, so m=6 is still open.
- **Caveat on `canon_*.tree` evidence.** `radio_canon_search_generic` does *not* prohibit
  one-sided splits (verified in source: n-side spans `[0,n]`, m-side enumeration is a bijection,
  the mirror filter is top-level dedup only). But it *does* require every leaf to be a
  `G_k`-atom singleton state, so conclusions about tree interiors from those trees are
  conditional on that hypothesis.
- **Scalable recursions:** `n(k,5) = n(k-1,2) + n(k-1,6)` (numerical only) and
  `n(k,6) = n(k-1,4) + n(k-1,5)` (realised by a split, outcome-0 saturated). Exact for k=5..9.
  See [conjectures.md](conjectures.md#scalable-constructions-for-m5-and-m6-2026-08-03).
- **Ruled out:** the non-adaptive reformulation. Each test returns
  `[x∈S] + [y∈S]`, so non-adaptive solving is a Sidon condition
  `(U−U) ∩ (V−V) = {0}` — exact for m ≤ 2, but strictly weaker from m = 3 (k=4, m=5: 6 vs 9).
  Adaptivity is essential; the Sidon picture is an intuition device, not a reduction.
- **Conjecture (u1) — mapped, not proved (2026-08-03).** `Sb(n1:n2)` solvable in k, n1 ≥ n2,
  implies `Sb((n1+1):(n2-1))`; equivalently the frontier decreases *strictly* in m. Now
  exhaustive over every one-part state at k ≤ 5, on top of the 130 proven cells. Two routes are
  **refuted**: the multi-part generalisation is false (`Sb(15:2,5:4)` solvable in 4,
  `Sb(15:2,6:3)` not), which kills every induction that rewrites a strategy part by part; and no
  mass-based coin-move lemma exists (`Sb(8:1,2:1)` solvable in 3, `Sb(9:1)` not, at strictly
  lower mass — by Singleton Majorization, not by solver). What remains is one lemma: *the winning
  split minimising `p−q` survives the coin move*, 187/187 at k ≤ 5 plus 28 at k = 6.
  [conjectures.md](conjectures.md#conjecture-u1---the-antidiagonal-conjecture).
- **A second solver exists.** `tools/refsolve.py`, written from [problem.md](problem.md) alone,
  no shared code with `radiobase.c`, reproduces the proven columns for k = 1..6 exactly. Slow —
  k ≤ 6 only — but auditable, which is what settles structural questions.

## Infrastructure

Working and worth trusting: `tools/check_tables.py`, `tools/check_witness.py`,
`tools/extract_evidence.py` (`certify` / `audit`), `tools/artifacts.sh`
(`push`/`pull`/`verify`/`check-index`), `tools/check_docs.py`, `tools/refsolve.py`.

Artifact store `fedork/radio-data` (private): 7 tags, 12 assets plus a manifest per tag,
367 MB stored, `check-index` green.
Deliberately **not** archived: ~18 GB of unreliable 2023 `out*` — see the decision in
[data.md](data.md).

Git and `gh` are pointed at the `fedork` account **per repo**, leaving global config alone.
Do not run `gh auth switch`.

## Running now

**Nothing is running.** The first AWS attempt (2026-08-04) was killed after six hours with no
verdict — removing deadlines trapped it in an intractable subtree. Deadlines are restored;
3,100,961 verdicts of the control survive as a checkpoint in
`s3://radio-sa193-393287594714/run/112_80.cache`, so a relaunch resumes rather than repeats.
See [aws-run.md](aws-run.md) and the 2026-08-04 journal entry.

## The Sa(193) certificate

Design in [certificate.md](certificate.md), nothing built yet. The object is sixteen k=9
refutations; the certificate is the set of refuted facts (the existing `parse_out.sh` format plus
a provenance header), verified level by level in `k`. Estimated order 300-600 MB shipped, and
`radio_allsol.c` is already most of the `SPLITS` checker. The one real problem is that the
solver's cache materialises the upward closure, so the log is **not closed** — three options,
decided by measurement not preference.

## Immediate next steps

1. `./run_radio_canon_search_generic.sh 4 9 457 7` and `... 447 8` — unique forced predictions
   of the profile model; minutes each, and a hit is a self-verifying proof.
2. Read the m=5 profile off `witnesses/canon_480_5_at9.tree`. This would turn the `2^q`
   invariant from a fit into a derivation, and needs no new compute.
3. `... 432 9` — discriminates the profile model (432) from the closed form (431).
4. The **Extremal Split Lemma** — the whole remaining gap in conjecture (u1), and the only item
   here needing no compute at all. An exchange argument is the natural shape; the surviving
   obligations are listed in
   [conjectures.md](conjectures.md#where-the-proof-gets-stuck).
