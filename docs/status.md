# Status

**Read this first.** Where everything stands, and what will silently ruin your work if you
don't know it. Last refreshed **2026-08-10** (exact `k=10,m=6` frontier; two live runs remain).

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
| **`tools/capped_run.sh --rss-gb` cannot bound a long solver run on this machine.** | The result-cache trie grows unboundedly as it solves, and macOS swaps it out rather than keeping it resident, so RSS reads 0.2 GB while 27 GB sits in swap and the cap never fires. A k=8-rooted mapping run reached `VSZ 424 GB` and 6,395 swapins per 45 s, managing 2 of 35 roots in 9 h 20 m. Bound by `MAX_N` and cell selection at *compile* time; to detect it live, watch `vm_stat` swapins, not RSS. |
| **The k≤7 oracle does not fit in 24 GB at full coin range.** | The cache trie scales ~`MAX_N²`: 4.04 GB at `MAX_N=132`, ~20 GB at `MAX_N=262`. A k=8-rooted run spent 3 h 50 m loading and never finished. Loading that looks like it is "decelerating" is swapping. Mapping a *known* k=5 state needs only k=4 solving, so obtain states some other way and skip the large oracle entirely. |
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
- **Three theorems** — Singleton Majorization, Unit-Group Elimination and Subgraph
  Monotonicity, all proved. The third is elementary but was load-bearing and unwritten: it is
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

Artifact store `fedork/radio-data` (private): 7 tags, 12 assets plus a manifest per tag,
367 MB stored, `check-index` green.
Deliberately **not** archived: ~18 GB of unreliable 2023 `out*` — see the decision in
[data.md](data.md).

Git and `gh` are pointed at the `fedork` account **per repo**, leaving global config alone.
Do not run `gh auth switch`.

## Running now

**Two cold runs are live on one instance** `i-0005d74f985c52ae1` (`r7iz.4xlarge`, 16 vCPU, 123 GB).

| | `run/` — original | `run2/` — A+B |
|---|---|---|
| build | 2026-08-05, pre-A+B | `efadab0` (2026-08-09) |
| started | 2026-08-05 ~13:44 UTC | 2026-08-09 ~19:40 UTC |
| at last check | 4 d 03 h, **0 of 16**, 2.56 M verdicts, 6.86 GB | just started |
| memory cap | 110 GB | 40 GB |
| binary | `radio_sa193` | `radio_sa193_ab` |

Watch both with `tools/sa193_status.sh --both [--watch]`. Notification subjects for the second run
are prefixed `[run2]`. Separate binary names matter: the original launcher locates its solver with
`pgrep -x radio_sa193 | head -1`, which with two runs would aim the new watchdog at the incumbent.

**The comparison to read** is `verdicts by level` at matched wall clock, specifically the k=6 line -
that level is 97% of the incumbent's CPU and is exactly what A+B targets. Do not expect the measured
2.75x: both A+B benchmarks were run *warm*, and the reachability prune arms on candidate **count**,
which accumulates far more slowly cold because every child solve is uncached.

The incumbent is not stuck. Its k=7 state `Sb(33:16,32:15,45:10,23:19)` was at `left=331/578`; per
the project owner, based on prior experience it is a few hours from done.

**Two risks.** The incumbent's user-data ends in `shutdown -h now`, so if it finishes or trips a cap
the instance stops and takes `run2` with it. And `run2`'s 40 GB cap is a guess - 2023 reached ~90 GB.

## The `Sa(193)` cold run is live

`i-0005d74f985c52ae1`, started 2026-08-05, serialized single process, **completely cold**
(`cache=(none, cold)`). The `Sa(192)` control passed in 2,209 s. Details and how to follow it without
logging in: [aws-run.md](aws-run.md); `tools/sa193_status.sh`.

At 47 h: `0 of 16` top-level states, ~2.1 M verdicts, **5.8 GB** peak against a 110 GB guard.

**Where the time goes (2026-08-07):** `self(k) = inclusive(k) - inclusive(k-1)`, valid because `took`
is inclusive and each state is computed exactly once (2,065,670 distinct `(state,k)`, zero duplicates).
**k=6 is 93.2% of CPU**, k=5 is 2.4%, k=4 is 0.2%; k>=7 is unknowable mid-run because their ancestors
have not completed. One k=6 state at mass 728 of 729 took 16,603 s — **9.9% of the whole run** — and the
92 k=6 states over 60 s are ~96% of all CPU. The near-saturated 8-part k=6 states are the whole cost.
`tools/sa193_status.sh` reports this every cycle.

**And within k=6 it is 166 states (2026-08-08).** Work per verdict is bimodal with an empty gap:
249,913 verdicts below 1e8 splits hold 0.31% of k=6, while 164 at 1e10-1e12 hold 99.63%. The gap is
structural — a mixed child doubles the part count while the other two preserve it, so k=6 states exist
at 4 parts and 8 parts and nowhere between, and the two cost modes are those two shapes. **99.57% of
k=6 work is one cell: 8 parts at mass >= 0.99 of 3^6**, i.e. ~90% of the entire run in 166 states.
They have no cheap refutation — the counting bound is vacuous at saturation and majorization is not
close — so any optimisation that matters must attack 8-part near-saturated states specifically. The
memory profile (`run/seg-*/memprofile.csv`) shows essentially the whole footprint was allocated in a
40-minute window during the control, and +900k verdicts since cost +0.48 GB — so **memory is not the
binding constraint after all**, contrary to the earlier expectation drawn from 2023's ~90 GB. Wall
clock is: there is no time bound anywhere below a pass-2 node, because
`child_deadline = pass<2 ? deadline : NO_DEADLINE` and such a node bumps its deadline 10 s rather than
returning `MAYBE`. Weeks is the expectation, not a symptom.

The run's binary **predates** the majorization corollary below; not worth restarting for a 3-16%
pruning gain against 27 h of accumulated cache.

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

`radiobase.c` now applies this full star-expansion majorization before the cache; the independent C
verifier and Python certificate prototype apply the same lemma separately. This **corrects** the
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

It does **not** yet improve the long-state solver. `radiobase.c` already invokes `R_0` on every
partial child before its cache lookup, so a separate `R_1` pass duplicates the existing prefix walk.
On the 42.7-second four-part positive `Sb(29:6,19:9,13:12,36:3)` in 6, the first cheap `R_1` witness
has two exactly unsolvable children and even passes `R_2`; `R_1` feasibility therefore does not
distinguish it from the real winning split. On a residual four-part negative, isolated `R_2` took
6.3 Python CPU seconds and still passed, while the warmed exact solver refuted it in 0.1 seconds;
`R_3` hit a 30-second cap. Production code is therefore unchanged. The useful target, if P6
continues, is a genuinely cheap approximation to deeper synchronization for **ordering**, not an
unconditional hierarchy pre-pass.

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

0. **Watch the two runs** (`tools/sa193_status.sh --both`). If `run2` overtakes the incumbent on
   k=6 verdicts at matched wall clock, the incumbent can be killed; until then leave it running.
   The 2026-08-09 journal entry has the full launch detail. Both remote runs predate full star
   expansion and do not benefit from it; do not restart either without a separate matched decision.

The pair/triple/quad deployment and limited-discrepancy FAST passes remain **rejected**. Their offline
facts are real, but the warm upward-closed prefix cache already contains the subset information; the
former added zero marginal rejections on the A+B monster, and the latter regressed negatives. Full
star expansion is different: it is an arbitrary-part-count global theorem and is now deployed.

1. For further P6 work, use `Sb(29:6,19:9,13:12,36:3)` in 6 as the residual positive control. A new
   bundled proposal must order the real winning split earlier under the same warm k<=5 cache; merely
   finding an `R_1` or `R_2` witness is already known not to do that. Keep any deeper check bounded
   and fallback-safe.
2. `./run_radio_canon_search_generic.sh 4 9 457 7` and `... 447 8` — unique forced predictions
   of the profile model; minutes each, and a hit is a self-verifying proof.
3. Read the m=5 profile off `witnesses/canon_480_5_at9.tree`. This would turn the `2^q`
   invariant from a fit into a derivation, and needs no new compute.
4. `... 432 9` — discriminates the profile model (432) from the closed form (431).
5. The **Extremal Split Lemma** — the whole remaining gap in conjecture (u1), and the only item
   here needing no compute at all. An exchange argument is the natural shape; the surviving
   obligations are listed in
   [conjectures.md](conjectures.md#where-the-proof-gets-stuck).
