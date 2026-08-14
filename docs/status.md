# Status

**Read this first.** Where everything stands, and what will silently ruin your work if you
don't know it. Last refreshed **2026-08-13** (proof-safe cold AWS `run9` and the resumed k=8
Pareto-prefix census are running beside the retained `run3`/`run8` performance baselines).

This page says where things *stand*. For what happened and why, see
[journal.md](journal.md); for what to do next, [research-plan.md](research-plan.md).

## Active traps

Each of these has already caused, or was one step from causing, a wrong result.

| trap | why it matters |
|---|---|
| **Never warm-start a *negative* result from `cache-2025:parsed_260.txt`.** | It contains the 16 `Sa(193)` verdicts under suspicion. Loading it re-reads the old answers and "confirms" them. It cannot be filtered: the cache spans 2023–2025 and does not record which build wrote each line. Fine for *finding* solutions — a poisoned negative only slows a search, never corrupts it, because any solution found is re-verified as a tree. |
| **Never promote a 2023-era negative above `legacy`.** | That build emits false negatives — 37 known, ~0.27%, with **no syntactic marker**. `Sb(143:17)` in 8 was declared unsolvable after 10 passes and 4 days, and is wrong. See [`../evidence/refuted_2023_negatives.txt`](../evidence/refuted_2023_negatives.txt). |
| **A solver log without complete embedded provenance is not new durable evidence.** | Historical outputs cannot identify which bugs and optimizations their binaries contained. New builds go through `tools/build_radio.py`; every raw output must contain `radio-provenance-v1` and pass `tools/check_provenance.py`. Direct compiler builds explicitly say `provenance_complete=no`. Standalone utilities run through `tools/run_with_provenance.py`. The artifact uploader enforces this, with a conspicuous legacy-only override. |
| **Do not use a negative derived by `run3` or `run8` as proof.** | Joint suffix reachability (`rb_dead`) was sound for rejecting the full state but incompatible with the older implicit-prefix contraction: it could cache a shorter negative that was actually solvable, then contaminate later searches. Forced counterexample: `Sb(5:3,2:2,2:2,2:2)@3` is unsolvable while its inferred `Sb(5:3)@3` negative is false. Builds containing `efadab0` but predating fix `75814a7` have no marker distinguishing affected contractions; this includes frozen run3 and run8. Cold `run9` suppresses contraction after an actual reachability rejection. Older exact lines independently rechecked without contraction may still be valid, and positive witnesses remain independently checkable. |
| **Do not "upgrade" the paper's `k ≤ 9` optimality claim to `k = 10`.** | The claim as written is exactly right. `Sa(10) = 192` maximality rests on the suspect 2023 run. |
| **`out26_1.txt` / `out26_2.txt` exist twice under the same names.** | ~130-byte stubs in `fullsolve-2026`; the 905 MB / 51 MB originals in `sa193-2023`. Only the latter are evidence. Pulling the wrong tag yields nothing, silently. |
| **A missing `can't solve` line does not mean unsolvable.** | `canSolveB` returns a tri-state and gives up with `MAYBE` on a deadline, printing nothing. Absence of a verdict is not a verdict. (Briefly narrowed on 2026-08-04 when deadlines were disabled; that change was reverted the same day — disabling them trapped a real run for six hours. In a proof-safe cache, a printed `can't solve` is exhaustive because it is emitted only when `!skipped_some`; the separate `rb_dead` trap explains why run3/run8 caches are not proof-safe.) |
| **Do not restore either old deadline extreme.** | `c13b5d3` could return before trying a complete child; `e648e83` required a new negative cache fact and then handed pass 2 an unbounded child. Run7 demonstrated the latter failure for 20,460 CPU seconds in a finite-parent k=5 state after 280,116,882,707 prefixes. Current policy permits zero-progress `MAYBE`, never refills an expired parent, keeps the reliable one/two-segment spine on the shared budget, and probes longer states with a geometrically increasing local slice. See [`../evidence/deadline_stall_2026-08-10.txt`](../evidence/deadline_stall_2026-08-10.txt). |
| **`tools/capped_run.sh --rss-gb` cannot bound a long solver run on this machine.** | The result-cache trie grows unboundedly as it solves, and macOS swaps it out rather than keeping it resident, so RSS reads 0.2 GB while 27 GB sits in swap and the cap never fires. A k=8-rooted mapping run reached `VSZ 424 GB` and 6,395 swapins per 45 s, managing 2 of 35 roots in 9 h 20 m. Use the local supervisor, which guards macOS `top`'s documented physical-footprint field, plus `vm_stat` swapins. `vmmap -summary` is useful for one-off attribution but can itself hang indefinitely. The 2026-08-10 local `Sa(193)` trial independently reproduced the RSS gap: 2.77 GB peak RSS versus 7.1 GB footprint. |
| **Do not apply old oracle footprint estimates to the new cache.** | The pre-2026-08-10 pointer trie needed 4.04 GB at `MAX_N=132` and about 20 GB at `MAX_N=262`; those measurements remain explanations of old failed runs, not predictions for current `main`. The deployed last-segment cache is 11.2x smaller on the `MAX_N=193` checkpoint, but a full `MAX_N=262` oracle has not been measured. Cap and inventory any new mapping run rather than assuming either figure. |
| **Benchmark against `radiobase.c`, not against a tool you wrote.** | Three times on 2026-08-09 a headline number came from comparing against the wrong baseline: a 40-state sample, a holdout that shared its *construction* with the training set, and a cold validation measured against a warm benchmark. The last one led to patching a "race" that did not exist. A per-part filter measured at 15.3x turned out to be already implemented by the per-split `s[4]` / `s[5]` loop in `canSolveB`, and a "200x" combined figure collapsed to ~5-13x. Before quoting a speedup, find the existing check that already does it. |
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
- **Three theorems plus the lift-box lemma** — Singleton Majorization, Unit-Group Elimination and
  Subgraph Monotonicity are proved; so is the new geometric core of recursive Pareto lifting. The
  latter is a search-region lemma, not yet a full construction. Subgraph Monotonicity is elementary
  but was load-bearing and unwritten: it is
  what the result cache's downward/upward closure and the whole `sbb_greater` relation rest
  on, and what lets a negative certificate store antichains instead of closures.
- **16 verified witness trees** — `Sa(38)` through `Sa(192)`, plus recursive trees for
  `Sb(248:3)@8`, `Sb(496:4)@9`, `Sb(480:5)@9`, `Sb(473:6)@9`, and a two-sided-only
  `Sb(480:5)@9`, and the singleton-majorized proof of the exact frontier
  `Sb(973:6)@10`. All pass
  `tools/check_witness.py`, which re-derives every step without consulting the solver.
- **16 exhaustive multi-part enumerations** — `data/exhaustive_multipart.csv`, including one
  proven negative.

## What is refuted

Kept on record so it is not re-derived: the `m=6` closed form and `BBCD` profile (both predict
`n(10,6)=976`, while the exact maximum is 973), the `m=11` closed form (violates monotonicity in
m), the hand-typed `409?` as a *derivation* (though see below — it is *consistent* with the
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
- **The apparent m=6 recursion is refuted at its first extrapolation.**
  `n(k,6) = n(k-1,4) + n(k-1,5)` is exact for `k=5..9` but predicts 976 at `k=10`; the exact
  value is 973.  The verified construction still uses a `2+4` root and saturates the `m=4`
  pure child, but backs the other width down from 480 to 477.  This is one datum, not a new
  constant-correction formula.  See
  [conjectures.md](conjectures.md#finite-m5m6-recurrences-and-the-k10-break-2026-08-10).
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
(`push`/`pull`/`verify`/`check-index`), `tools/check_docs.py`, `tools/refsolve.py`, and the
fixed-small-m exact recurrence `tools/search_singletonization.cpp`.

Artifact store `fedork/radio-data` (private): 12 tags, 33 assets plus a manifest per tag,
about 394 MB stored, `check-index` green.
Deliberately **not** archived: ~18 GB of unreliable 2023 `out*` — see the decision in
[data.md](data.md).

Git and `gh` are pointed at the `fedork` account **per repo**, leaving global config alone.
Do not run `gh auth switch`.

## Running now

Three AWS solvers are on `i-0005d74f985c52ae1` (`r7iz.4xlarge`, 16 vCPU, 123 GB). Snapshot from
**2026-08-12 03:32 UTC**:

| prefix / build | freshness | last reported state |
|---|---|---|
| `run/` — original | stale; solver gone | 2,568,394 verdicts, 0 of 16 |
| `run2/` — A+B | stale; solver gone | 1,897,635 verdicts, 5.72 GB, 0 of 16 |
| `run3/` — A+B + full-star majorization | **alive; performance only** | 1.71 M verdicts, **25.51 GB**, 1 of 16 |
| `run4/` — compact cache at frozen commit `6af384e` | stopped, archived; old scheduler | 103,773 verdicts, **0.29 GB**, control never returned |
| `run5/` — compact cache + exact L1 at frozen commit `290a892` | stopped, archived; old scheduler | 103,769 verdicts, **0.29 GB**, control never returned |
| `run6/` — broken deadline experiment at `c13b5d3` | stopped, archived | control SOLVABLE in 922.0 s; 618,816 raw lines, 1.37 GB peak RSS |
| `run7/` — progress-gated pass-2 dive at `e648e83` | stopped and archived; obsolete scheduler | 104,931 verdicts; control never returned; **0.29 GB** peak RSS |
| `run8/` — compact cache + bounded probes at `9395218` | **alive; performance only** | 639.6 K verdicts, **1.15 GB**, 0 of 16 |
| `run9/` — rb-safe contraction at `e7fa747` | **fresh and alive; proof run** | control SOLVABLE in **479.2 s**; 165.7 K verdicts, **0.60 GB**, 0 of 16 |

Use `tools/sa193_status.sh --compare --baseline run8 --candidate run9 [--watch]` for the matched
run8/run9 comparison. The default remains the longer run3/run8 comparison; `--all` prints the stopped
historical sessions and must not be read as proof that those processes remain alive.
`run3`'s `Sa(192)` control took 540.7 s.  It has completed one of the 16 top-level states and remains
dominated by k=7 while exploring `Sb(111:82)@9`.

Run8 started cold at 2026-08-11 22:46:06 UTC with the `Sa(192)` control enabled. Its full embedded
commit is `9395218dcbdd90d8f6a208b15da1878ff75f6ee1`. Its 60 GiB wrapper and run3's 40 GiB wrapper
formed the original two-run envelope; run9's combined guard now caps all three solvers at 108 GiB.
The source archive, frozen binary, sidecar and `run.meta` are retained under `run8/`; hashes and SSM
command IDs are in [aws-run.md](aws-run.md).

Run9 started cold at 2026-08-12 03:21:12 UTC from `e7fa747264476461a234bf78e49762ee77ad8d8d`.
It changes only the unsound interaction: once `rb_dead` actually rejects a partial assignment, that
invocation retains an exact full negative but cannot materialize an implicit shorter negative. Each
such event prints `contraction=rb-suppressed:<size>`; its five-minute status reports the count and
latest state. The exact source archive, binary, build sidecar and launch metadata were hash-verified
through S3. Run9 has a 60 GiB individual guard; a separate **108 GiB combined-solver guard stops
run9 first**, preserving run3/run8 and about 15 GiB of host headroom. The idle guard now names all
three solvers.

Run8's mandatory remote happy-path gate passed:
`result CONTROL Sa(192) in 10 = SOLVABLE (471.6 s)`.
This is 0.872x run3's 540.7-second control on the same host. It validates this execution sufficiently
to continue into `Sa(193)`; it is not evidence about the final negative yet.

Run9's independent cold control also passed:
`result CONTROL Sa(192) in 10 = SOLVABLE (479.2 s)`. At 03:32 UTC the solver had continued into
`Sa(193)`, all three solver processes and run9's wrapper/watchdog/joint/idle guards were alive, and
95 GiB remained available with no swap. No tainted contraction had yet been suppressed. This gates
the execution, not the final maximality claim.

The run8 watchdog scans the run3 and run8 raw prefixes every five minutes with bounded state. It chooses the
run that is behind by completed roots and verdict count, ranks its six slowest completed exact
states, and joins `(state,k)` keys in the peer log. A verdict's `took` covers only its final
activation, so the ranking and first timing column now add the last visible `elapsed` value from
each earlier progress episode. Historical `MAYBE` returns have no exact timestamp: `≥` marks this
attempt-sum floor, `(2a)` means two visible attempts, and a ratio involving a floor is prefixed `~`.
Per-call `~self-final` still subtracts all k-1 verdict time since the previous k verdict and applies
only to the final activation. `MAYBE` and cache effects make it approximate, and the first call at
each level has no left boundary. The compact view keeps the recursive stack and a per-run level
profile, but no side-by-side level comparison. The profile adds the visible elapsed time of current
`still solving` frames before taking level differences.

The memory profile is no longer the benign one inferred from the original build: `run3` has reached
25.50 GB with 1.64 M verdicts.  The stale `run2` snapshot is not a matched-time comparison,
so it does not by itself identify the cause; it does show that memory per verdict cannot be assumed
stable across the changed search shape.  `run3` still has the old unbounded dense result trie; the
current compact representation remains unbounded but is over an order of magnitude smaller on the
retained checkpoint.  The level-lazy split-table and cache changes are not deployed in `run3`.
Do not restart a cold run solely to pick them up.

Run7 resolves the ambiguity left by run4/run5. At retirement it was in exhaustive pass 2 of their
same information-tight 14-part k=5 state after 280,116,882,707 prefixes. The call had a finite
parent, but `e648e83`'s pass-2 rule made the child `NO_DEADLINE`; its required negative-cache count
could not advance when the cache was saturated. The 20,460-second activation was therefore not a
healthy mandatory dive. Run6 represents the opposite failure: its poll could return before a
complete child was tried. Printed definitive verdicts from either build remain sound, but neither
timing profile is a baseline.

Commit `45c34fd` replaces both extremes without remembering a split.  Finite children share an
absolute parent bound and may return `MAYBE` with no mutation.  One/two-segment states retain that
shared allowance; longer states start with two-second speculative children and double the quantum
after an unresolved exhaustive pass.  The exact stuck k=5 state now returns `MAYBE` in 1.000 CPU
second with zero new negatives.  A normal genuinely cold `Sa(192)` control completed in 376.293 CPU
seconds (382 s wall, 0.18 GiB peak RSS), following `[48:32]`, `[16:15,45:23]`, then
`[17:8,27:13,10:8,12:0]`.  The 1,038-answer regression and ASan+UBSan gates match prior main.

Raw validation and rejected-experiment logs are archived as `bounded-probe-2026-08-11` and
`bounded-probe-rejected-2026-08-11`.  Full control flow, build IDs and the two-stage correction are
in [`../evidence/deadline_stall_2026-08-10.txt`](../evidence/deadline_stall_2026-08-10.txt).
Run3 remains untouched. Run7 and the local `e648e83` continuation predate embedded
`radio-provenance-v1`; both were stopped only after their source/launch metadata and raw segments
were preserved. Run7's finalized S3 raw stream matches the retained EBS file exactly: 104,936 lines,
11,065,274 bytes and SHA-256
`a79d31d9b11bf97679451087b90978f7fdc3b8874847bda2ebca305142ddb72c`. Its final checkpoint,
source archive, frozen binary, final profile, stderr, watchdog log and `run.meta` are under `run7/`.

The completed common prefix does give a clean throughput warning.  Matching negative `Sb` verdicts
by exact printed state and level found 98,253 common calls.  Across the k=8, k=7 and k=6 groups,
`run4`'s summed inclusive per-call `took` values were respectively 1.246x, 1.247x and 1.264x
`run3`'s.  The slowest six completed `run4` refutations were all one-part k=8 states and each was
slower by 19-33%, even though its local `totalsplits` count was 25-30% smaller.  At k=7 and k=6 the
aggregate split counts are essentially identical, so reduced split construction is not hiding the
cost.  Treat this as an approximately 25% cost for the *whole compact build on this natural cold
path*, not as an isolated Pareto-front lookup measurement: `took` is inclusive and the two builds
also differ in split storage/filtering and code layout.  Exact states and snapshot hashes are in the
2026-08-10 journal entry.

That measurement motivated an isolated profile and is no longer the lookup-cost expectation for
current `main`.  A bounded exact-state L1 now probes before repeated bundled majorization and
the compact dominance trie.  On the fixed warm four-part control it takes **26.6** solve seconds,
versus 42.6 for the first compact build and 33.0 before compaction, with the identical witness and
37,899 local splits.  The full `Sa(192)` gate is SOLVABLE in **711.7 CPU seconds**, versus 819.9 for
the first compact build and 734.5 before compaction, at 0.35 GB peak RSS.  All 3,379,067 verdict
bytes in the combined-checkpoint regression match compact baseline c146d9d exactly.  This fixes the
measured tax for current runs; frozen `run4` does not contain it, while stopped `run5`/`run6`/`run7`
do.

The proposed matched state `Sb(48:48,64:33)@8` now has definitive matching refutations. Run3 first
logged an abandoned episode through 2,602 seconds, then much later returned `FALSE` in a 14-second
warm retry; run8 used an approximately 999-second bounded probe and returned `FALSE` on a
1,181-second retry. Both definitive calls report the same 53,834 admitted split combinations. The
observable attempt floors are therefore `run8 ≥ 2,152 s` and `run3 ≥ 2,616 s`, not the misleading
final-activation ratio `1181/14 = 84.36`. More importantly, the enclosing `Sb(112:81)@9` reached the
run8 refutation and entered pass 2 at 4,881 CPU seconds; run3 reached the same refutation immediately
before its 155,329-second root verdict. That is a 31.8x earlier phase transition, not yet an
end-to-end root comparison: run8 remained in pass 2 at this snapshot. Exact raw line numbers and
the lower-bound reconstruction are in the 2026-08-12 journal entry.

A current-main local trial (`713b7d6`, M4 Pro / 24 GB) was stopped deliberately rather than left
to swap.  The cold `Sa(192)` control passed in **734.5 CPU seconds**.  In 1,152 wall seconds the
process emitted 232,725 verdicts, but after entering `Sa(193)` its `vmmap` footprint reached
**7.1 GB**, of which **5.9 GB was swapped**, while the wrapper reported only 2.77 GB peak RSS.
CPU utilisation had fallen to about 44% and no top-level k=9 state had completed.  This is an
abort, not a refutation.  It shows that the level-lazy split tables do not by themselves make a
full 24 GB local run practical.  This is now the pre-compaction baseline, not the operational state
of current `main`.
The raw log is archived as `sa193-local-2026-08-10:out_sa193.txt` and its same-run parsed checkpoint
remains locally at `/Users/fedor/radio-runs/sa193-local-713b7d6-trial1/sa193.checkpoint`.

Checkpoint replay now explains the footprint.  The k=5..7 cache roots reserve **5.20 GiB** for
90.7 million live transitions; the 232,725 exact facts occupy only 8.0 MiB in parsed form.  Two
different multipliers are involved: eager dominance-closure materialisation (especially positive
k=5/6 facts) and sparse dense arrays (especially negative k=7).  The exact shape counts and discarded
intermediate layout models remain in
[`../evidence/cache_shape_sa193_local.txt`](../evidence/cache_shape_sa193_local.txt).

The local design is now deployed: maximal-positive and minimal-negative Pareto fronts store the last
part at every exact prefix, with tagged uint32 child descriptors and inline singleton fronts.  The
same k=5..7 replay requests **499,877,916 bytes**, down **11.174x / 91.05%** from 5,585,395,760;
the process peaks at 0.60 GB RSS.  Across 4,164,958 exact, mutated and deterministic random queries,
every old verdict is preserved and 4,622 `MAYBE` answers become sound positive-front hits.  The full
`Sa(192)` control now remains SOLVABLE in 711.7 CPU seconds after adding the 2 MiB exact-state L1,
versus 819.9 for the first compact build and 734.5 before compaction; peak RSS is **0.35 GB**.  This
makes a bounded local `Sa(193)` continuation credible without retaining the earlier CPU premium; it
does not bound future cache growth or prove that the full refutation fits.
Implementation measurements and commands are in
[`../evidence/cache_last_front_2026-08-10.txt`](../evidence/cache_last_front_2026-08-10.txt).

The genuinely cold local segment at commit `7ceb59d` ran from 2026-08-11 01:12:42 to 05:47:30 UTC
in `/Users/fedor/radio-runs/sa193-local-front-7ceb59d-cold2`.  Its control passed in 807.7 seconds;
it then emitted 485,337 log lines while holding about a 1.3 GiB physical footprint.  It was stopped
under the first deadline diagnosis.  Run7 later confirmed that the inherited progress-gated
pass-2 policy can indeed lose its finite escape, although the original run4/run5 snapshots alone
had not proved that.  The supervisor exited 143 and generated a
17 MiB final same-run checkpoint (SHA-256
`3b8622f4d1cc342f28c93626e6554d2c7ca8da8ff0582c993ceeca6e19c73ae2`).  This is an interrupted
segment, not an `Sa(193)` verdict.

A corrected same-chain continuation ran in
`/Users/fedor/radio-runs/sa193-local-depth-e648e83-resume3` from 2026-08-11 15:49:16 to
22:12:33 UTC on commit `e648e83`. It loaded the 931,075-line checkpoint folded from every earlier
segment, reproduced the positive control from cache, and added 188,172 raw lines before deliberate
retirement. Its scheduler is obsolete; do not use its slow-state timing as evidence about `45c34fd`.
The old supervisor's `vmmap` probe itself hung for 19 minutes, freezing both its memory guard and
checkpoint cadence; merely bounding it showed that it still timed out consistently.  The guard now
reads macOS `top`'s documented physical-footprint field, with its own 20-second timeout, and its
first sample succeeded.  Resumed checkpoints fold inherited facts into each new file, so another
restart does not silently forget an earlier segment.  A final proof must retain all raw logs; the
continuation alone is not a closed derivation.

Operational guard outcome: while adding future provenance, the primary supervisor script was edited
in place despite the documented Bash re-read trap. As expected, the primary exited without its
completion marker after solver 19088 was terminated. Independent recovery guard 28814 then folded
the exact inherited checkpoint plus the final raw segment into `sa193.recovery.checkpoint` and
exited. The recovered file has 1,118,898 lines, 42,433,056 bytes and SHA-256
`ba6ba91fdad83681b36a1b79c126060dc1607857cdf57ed34b18bb3ca65e2f7a`; its body matches a fresh
`source checkpoint + parse_out.sh(raw)` reconstruction byte for byte. PIDs 19088, 19059 and 28814
are all gone. Every raw segment and checkpoint remains on disk; nothing was deleted.
The four raw segments, closed checkpoint and launch/runtime sidecars are also archived under
`sa193-local-chain-2026-08-11`; the explicit legacy-provenance classification is recorded in
[data.md](data.md).

The sibling `cold1` directory is an empty launcher failure: a managed one-shot shell reaped both
descendants despite `nohup`; it contains no solver result and must never be used as a checkpoint.

Historical lesson, retained without treating the old processes as live: before full-star
majorization, almost all measured CPU was in near-saturated 8-part k=6 states.  Full-star
majorization removes that mode strongly enough that `run3` is instead dominated by k=7.  Optimising
only the old k=6 monsters is therefore no longer a complete plan for the current engine.

## The Sa(193) certificate

Design in [certificate.md](certificate.md). The object is sixteen k=9 refutations; the certificate
is the set of refuted negative facts in the existing `parse_out.sh` format plus a provenance
header, verified level by level in `k`.

**Built and working: `radio_verify.c`.** An independent checker sharing no code with the solver.
It verifies the whole `Sa(113)` k=9 ladder — 304,105 negative facts across k=2..8 — with **0
unverified**, single-threaded. Trust base is three theorems plus the split semantics; ~700 lines.

**The 2023 corpus is nearly a certificate already, and the gap is localised.** `sa193-2023`
contains all sixteen `can't solve Sb(n1:193-n1) in 9`. Checked top-down against itself plus the
2026 `out_k8.txt`:

- **k=9: each of the sixteen fails on exactly ONE split** — 32 recursion nodes total. The survivor
  is always the near-balanced one, and it needs a single two-part k=8 fact: for `Sb(112:81)` that is
  **`Sb(74:40, 41:38)` at k=8**, which nothing among the 1,879 logged 2023 k=8 facts dominates.
- **k=4: 940 facts, 0 unverified. k=5: 4,859 of 4,859 sampled, 0 unverified.**
- k=7 — 3.1 M facts, P=4, ~558 options per part — is the term that decides feasibility. Not yet
  measured to completion.

So the question is no longer "re-run 47 days"; it is "prove sixteen k=8 two-part states and check
the rest". See the 2026-08-04 journal entry.

Two design claims that stood here are **withdrawn**:

- *"The log is not closed."* Qualified, not withdrawn: **a cold single-session run is closed; a
  resumed run is not.** The k=9 ladder (one cold session) verifies at 0 unverified. The 2023
  `Sa(193)` run was resumed for months from warm caches whose logs were not archived, so ~5% of its
  k=5 facts and all sixteen of its k=9 facts cite children that were never logged. **Constraint on
  the re-run: keep every session's output, or start cold and never resume.** The fix is not
  breadcrumbs but **on-demand derivation** — the checker proves cheap missing facts itself, which
  closed the k=5 gap completely and shrinks the artifact to facts that are expensive to re-derive.
- *"Verification is cheaper than the proof."* It is not — removing search removes constant
  factors, not the enumeration. k=4's 216,580 facts cost 91 s against 1,521 s for the whole
  `Sa(113)` solve. The value is the trust base, arbitrary parallelism, one-level memory
  residency, and spot-checkability.

### What the certificate needs next, in order

1. **Prove the sixteen k=8 facts with the solver** — `./radio_one <cache> 8 74 40 41 38` and its
   fifteen siblings. The only genuinely new compute, sixteen independent jobs, **still unsized**.
   Cold is hopeless: even the single part `Sb(74:41)` at k=8 does not resolve in 10 minutes.
   Warm-start from `k8-2026-05-12:out_k8.txt`, 2026-era and audited clean — the prohibition is
   specific to `cache-2025:parsed_260.txt` and its sixteen suspect verdicts. Beware the cache size:
   the `out_k8.txt` facts that could inject into `Sb(74:40, 41:38)` number 11,375,981, about 25 GB
   of trie before the search begins.

   **Deriving them in the verifier does not work (tested 2026-08-05).** `Sb(74:40, 41:38)` fails on
   its first split in 227 nodes: all three children are k=7 states absent from the corpus and
   dominated by nothing in it. Deriving it needs k=7 facts that must themselves be derived, and
   the recursion is the original search.
2. **Verify the painted k=7 sub-DAG** — 16,347 facts, order 100-300 core-hours, hours on 24 cores.
   Parallelise here: facts are independent and levels are independent, and the resident set is one
   level rather than the certificate.
3. **Re-ask whether painting shrinks at k=7 -> k=6.** Preferring already-painted witnesses gave
   2.07x in speed and **no** size reduction at k=9 -> k=8, because 1,910 of 1,932 k=8 facts are
   cited and there is nothing to trade against. At k=7 -> k=6, 16,347 facts fan out into a 2.5 M
   level, where slack is far likelier.
4. **Minimalize each level** — measured at only **1.84x** (46.4% of the k=6 level is redundant, 45.6%
   of the `np=4` bucket that dominates). Deprioritised accordingly; it is not the lever the
   upward-closure argument suggested.

Ranked by measured effect at k=7: **top-down painting 190x**, columnar dominance index >=8.1x,
minimalization 1.84x. Cost scales with **part count**, not fact count, but the realised part count
on the actual `Sa(193)` logs never exceeds 10 — so the binding constraint was never the exponent,
it was how many facts the proof actually reaches.

## Where P6 stands — full star expansion is the structural rule

The long-state work has a theorem-backed improvement, not another fitted heuristic. The
**Vertex-Splitting Pullback Lemma** says that `(n:m)`, `n>=m`, may be lifted to `m` disjoint copies of
`(n:1)` by cloning the wide shore. Pulling tests back to all clones preserves edge transcripts, so
every solvable state must have its mass-preserving profile

    sort(n_1 repeated m_1 times, n_2 repeated m_2 times, ...) <=_w G_k.

On an exact-L1 miss, `radiobase.c` applies this full star-expansion majorization before the
dominance trie; the independent C verifier and Python certificate prototype apply the same lemma
separately. This **corrects** the
earlier claim that the `majtight>1` cutoff was heuristic-only. The cutoff is sound; its continuous
value below 1 remains only an ordering score.

Measured effect: the hard 8-part positive moved from a matched 300-second timeout to **5.3 CPU
seconds**. The A+B monster is now refuted at the root by the prefix certificate `714>705`, instead of
237.4 CPU seconds / 8.1 billion candidate evaluations. The filter directly proves 822,537 of
2,024,705 recorded k=5 negatives and hits zero of 20,780 positives. All 62,366 negative facts in
`out_k7.txt` still verify independently with zero gaps. Full proof and benchmarks are in the latest
[journal entry](journal.md).

The condition is not sufficient: `Sb(16:1,12:2)` passes it but is unsolvable in 4. The residual is
now formalised by the
[synchronized-majorization hierarchy](theorems/singleton-majorization.md#the-synchronized-majorization-hierarchy-2026-08-09).
`R_0` is full star expansion; `R_d` requires one legal rectangle split whose children pass
`R_{d-1}`. The relaxations are nested and sound, and `R_k` is exact solvability. `R_1` also has an
additive hinge-vector formulation, so it can be checked without sorting child profiles.

The hierarchy is structurally sharp on the complete current-solver k=4 pair universe. Among its 238
canonical negatives, `R_0,R_1,R_2,R_3` reject respectively 68, 150, 229 and all 238, with zero
rejections among 1,247 canonical positives. `Sb(16:1,12:2)` itself passes `R_0,R_1,R_2` and fails
`R_3`. `tools/bundled_majorization.py` and `tools/pairtab.c` reproduce the census.

Deeper synchronization does **not** yet improve the long-state solver. On an exact-L1 miss,
`radiobase.c` already invokes `R_0` on every partial child before its dominance-trie lookup, so a
separate `R_1` pass duplicates the existing prefix walk. On the residual four-part positive
`Sb(29:6,19:9,13:12,36:3)` in 6, the first
cheap `R_1` witness has two exactly unsolvable children and even passes `R_2`; `R_1` feasibility
therefore does not distinguish it from the real winning split. On a residual four-part negative,
isolated `R_2` took 6.3 Python CPU seconds and still passed, while the warmed exact solver refuted it
in 0.1 seconds; `R_3` hit a 30-second cap.

### Recursive Pareto lifting: a sharp first step, open recursion (2026-08-12)

The proposed long-state construction now has an exact local lemma.  For aligned lower template
`T <= P` and lower cut `s`, every lineage-preserving lift is in

```
s <= X <= s + (P - T).
```

Every lower selected/complement/mixed rectangle is then a componentwise substate of its parent
counterpart.  This proves the search box, not the enlarged children: they still require exact
verification.  Full proof and the distinction are in
[recursive-pareto-lift.md](theorems/recursive-pareto-lift.md).

The rigid prefix premise checks out empirically.  Across all 32 one-part k=7 Pareto roots, a winning
first cut can be chosen from the corresponding k=6 frontier; in all 31 cases with a nontrivial
two-part continuation, opposing lower-front lineages give a winning second cut.  The new probe then
lifts the resulting four-part template by preserving its three outcome proportions.  On
`Sb(45:10,33:15,32:14,23:20)@7`, it found a new solution at radius 8, structural rank 5, in 15 wall
seconds.  Ordinary search under the same warm cache took 65 wall / 57 solver seconds and admitted
155,795 top-level splits.  An adjacent lower-front point also found a different solution, but
swapping its two equal `19:9` lineages did not succeed within the same radius: inherited component
identity matters.

Greedy full recursion fails at the next low-k node.  The direct lower split's complete 774,144-point
lift box had no cache-open candidate.  A unique greedy Pareto upgrade produced 19 cache-open
candidates in its smaller box, but strict 200 ms-per-child probes accepted none.  Therefore one lower witness,
one maximal upgrade and its first split are not a construction.  The open object is a choice theorem
over an antichain of Pareto upgrades and inequivalent solving splits, preferably tested first at
larger k where degeneration is weaker.  The implementation remains the standalone
`tools/pareto_lift_probe.c`; no production search order or cache semantics changed.  Fully
provenanced positive-path logs are archived as `pareto-lift-2026-08-12`.

The exhaustive choice-corpus driver is now built and its k=7 run is complete: 32 roots, 450 winning
first cuts, 2,956 labelled second-cut lineages, 563 canonical targets, 819 upgrade nodes, 610
fixed-dimension Pareto endpoints, 7,396 raw endpoint winners and 3,227 exact automorphism classes.
No seed was blocked by the representation limit.  The larger k=8 run was interrupted by an IDE
restart after 4 h 57 m at 7.043 GB peak physical footprint.  Its durable input contains 621 of 815
summary-closed first-cut blocks, including 17 of the 70 blocks absent from the older checkpoints;
the unfinished next block is deliberately ignored and replayed.

That remainder is now detached on the shared AWS host as `pareto_k8_aws`, built from `54486d6` with
build ID `d9a89e3002d69f7879a214fbc78452c257a1c05ac9c51a4ecee55c62432af3cf`.  It has a 20 GiB
individual RSS cap and is the victim of a four-solver 108 GiB combined guard; the host idle guard
also tracks it.  Its supervisor analyzes and uploads the final corpus automatically.  Query it once,
without starting a local watch loop, with `tools/pareto_census_status.sh`.

One safe `R_0` deployment landed on 2026-08-10: split-table construction omits a local cut when one
of that part's child substates already fails counting or full-star majorization at `k-1`. Subgraph
monotonicity proves that later parts cannot rescue it. Tables are now exact-sized contiguous blocks
keyed by `(k,sbb)`, and suffix tables remain absent until depth-first search reaches them. Against
the same warm cache, the positive above retained the identical winning split and top-level
`totalsplits=37899` while moving from 43 to 32 solver seconds; its requested persistent split memory
moved from 1,407,276 to 261,560 bytes. The exact negative control stayed at 0.08-0.09 seconds. This is
allocation and a redundant local necessary check, not an unconditional hierarchy pre-pass. The
remaining theoretical target is a genuinely cheap approximation to deeper synchronization for
**ordering**.

### Theoretical m=6 track: exact k=10 break beyond the fitted continuation (2026-08-10)

The `473:6@9` witness must not be treated as a canonical scalable object. Exhaustive data support
only its early trunk: `Sb(110:3,115:2,121:1)@7` has two working splits, one outcome-complement pair.
Its mixed child `Sb(53:2,52:2,57:1,57:1)@6` has 12 working splits, or three genuine symmetry
classes. The stored tree chooses one; the ambiguity is real from that point onward.

Extending the forced arithmetic prefix gives the parametric four-part kernel

    Z_t = Sb((D_t+2t-1):2, (A_t-2t):2, C_t:1, C_t:1),

where `A_t,C_t,D_t` are the first, third and fourth dyadic atom values of `G_t`. Its six-row
full-star mass is exactly one below the top-six capacity. This forces any `R_1`-feasible next test
into row-count patterns `(2,6,4)`, `(3,6,3)` or `(4,6,2)`, with one unit of total child slack.

The independent hierarchy checker exhausts every such first test without using a witness
continuation. `Z_6` passes `R_4`, as it must. `Z_7` has 356 `R_1`-feasible raw splits / 84 distinct
normalized child triples and fails `R_4`; `Z_8` has 424 / 101 and also fails `R_4`. Since every
solvable state satisfies every `R_d`, both kernels are unsolvable. Reproduce with
`tools/bundled_majorization.py m6-kernel <t> 4` (21.2 and 57.5 CPU seconds in the recorded runs).

The missing root classification is now settled at the first new level.  The exact small-m
synchronized search exhausts every strategy for `Sb(974:6)@10`, while a 115-node tree proves
`Sb(973:6)@10`; hence **`n(10,6)=973`**.  The old formula and `BBCD` profile both predict 976 and
are refuted.  At the successful root `[477:2]`, the children are `Sb(477:2)`,
`Sb(496:2,477:4)`, and `Sb(496:4)`: it keeps the saturated `m=4` side but avoids the dead
`Z_7` continuation by giving up three units on the other width.  The centered `3+3` alternatives
`Sb(488:3,488:3)@9` (total 976) and `Sb(487:3,486:3)@9` (total 973) are both exactly unsolvable;
this does not classify every working root at 973.

The proof sources are `evidence/sb_m6_k10_frontier.txt` and
`witnesses/majorized_973_6_at10.tree`.  `tools/search_singletonization.cpp` is independent of the
old fitted witness: at full depth its recurrence is exact solvability, it enumerates the complete
short deficit interval for each cut, and its positive tree is re-derived by `check_witness.py`.
The next large-k target is the two-bundle mixed frontier behind the working `2+4` root.  A tempting
`-3` lift to `k=11` reduces to `Sb(503:1,495:2,478:3)@9`, but its first five-minute exact run timed
out without a verdict.  The literal scaled next split is nevertheless dead: it produces the
exactly unsolvable residual `Sb(247:1,247:1,240:2,231:2)@8` (277.622 s).  A different first split
of the parent remains possible.  Do not promote that one-point correction to a formula.

## Immediate next steps

0. **Keep watching proof-safe cold `run9` beside run8.** Its positive `Sa(192)` control passed in
   479.2 CPU seconds; it should now run to a verdict unless its individual/combined memory guard or another
   concrete health signal fails. Use `tools/sa193_status.sh --compare --baseline run8 --candidate
   run9 --watch`. Run3/run8 remain valuable matched performance histories, but their negative caches
   predate the contraction fix and cannot establish H3.

1. **Evaluate run9 by scheduler progress, suppression telemetry and memory, not raw verdict count
   alone.** Compare completed roots, active stacks, matched attempt floors, `~self-final`, RSS, and
   `rb-tainted contractions`. A `≥` attempt sum is only a visible-work floor; missing
   verdicts remain `MAYBE`, not negatives, and no obsolete checkpoint should be used as a
   cross-build proof source.

The pair/triple/quad deployment and limited-discrepancy FAST passes remain **rejected**. Their offline
facts are real, but the warm upward-closed prefix cache already contains the subset information; the
former added zero marginal rejections on the A+B monster, and the latter regressed negatives. Full
star expansion is different: it is an arbitrary-part-count global theorem and is now deployed.

2. For further P6 work, use `Sb(29:6,19:9,13:12,36:3)` in 6 as the residual positive control. A new
   bundled proposal must order the real winning split earlier under the same warm k<=5 cache; merely
   finding an `R_1` or `R_2` witness is already known not to do that. Current `main` takes 26.6
   solve seconds and 37,899 top-level splits after the exact-L1 change. Keep any deeper check
   bounded and fallback-safe.  For the separate recursive Pareto-lift track, move upward in k before
   adding solver code: retain several parent-conditioned Pareto upgrades and inequivalent splits,
   preserve lineage labels through equal components, and measure whether one branch survives at the
   next recursive node.  One greedy low-k path is already known to fail.
3. `./run_radio_canon_search_generic.sh 4 9 457 7` and `... 447 8` — unique forced predictions
   of the profile model; minutes each, and a hit is a self-verifying proof.
4. Read the m=5 profile off `witnesses/canon_480_5_at9.tree`. This would turn the `2^q`
   invariant from a fit into a derivation, and needs no new compute.
5. `... 432 9` — discriminates the profile model (432) from the closed form (431).
6. The **Extremal Split Lemma** — the whole remaining gap in conjecture (u1), and the only item
   here needing no compute at all. An exchange argument is the natural shape; the surviving
   obligations are listed in
   [conjectures.md](conjectures.md#where-the-proof-gets-stuck).
