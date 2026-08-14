# Research plan

Living document. Update it when a goal is met or reprioritised; do not accumulate stale
entries. Last revised 2026-08-14.

## High-level goals

**H1 - Publish.** Get the two-coin quantity group testing results out: the `Sa` sequence
proven optimal for `k <= 9` with `Sa(192)` in 10 as a verified construction, the `Sb` Pareto
frontier proven for `k <= 8`, and the two theorems. The draft in `paper/` is close. Its claim
of optimality through `k <= 9` is exactly right - do not "upgrade" it to 10 without redoing
the `Sa(193)` computation.

**H2 - The K=9 Sb column.** Six lower bounds at small `m`, plus `legacy` bounds at
`m = 65..96` recovered from the Sa work. The band `m = 7..64` is entirely blank. This is the
main open front.

**H3 - Is `Sa = 192` maximal at k=10?** Still open, and now better understood. A 2023 run
refuted all 16 states `Sb(n1 : 193-n1)` in 9 over ~47 days, and that log was recovered on
2026-08-02 - but the same corpus contains 37 provably false negatives with no syntactic
marker distinguishing them, so the verdict cannot be accepted. See
[results.md](results.md#sa10-192-achievable-maximality-not-established). Settling it means
re-running those 16 states on a current build — and note that a warm start from
`parsed_260.txt` is **not** a shortcut: the cache contains the 16 verdicts under suspicion, so
loading it would confirm them circularly, and it cannot be filtered by build era. See the
warm-start warning in [data.md](data.md#warm-starting-from-parsed_260txt-two-traps). A sound
negative needs a cold run, which is why this stays expensive.

Cold `run9` is the current proof attempt. Run3 and run8 remain useful performance baselines but
cannot settle H3: before `75814a7`, suffix reachability could promote a full-state obstruction to a
false implicit shorter negative and poison the cache. Run9 began from an empty cache with that
interaction disabled and reports every suppressed contraction.

Run9 is a frozen process-CPU-budget binary and remains untouched. New builds preserve its shared
finite-bound/geometric-deepening policy but schedule finite probes by deterministic accepted-prefix
work, calibrated at 20,000,000 units per nominal second. This removes hardware/load timing from the
stopping point for a fixed binary, query and cache history; it does not make a warm and cold search
equivalent or change what constitutes a proof. The root `rb_dead(0,0,0,0)` relaxation was tested
separately and was too weak to enable eagerly, so the existing measured-cost trigger remains. See
[`../evidence/work_budget_rb_root_2026-08-13.txt`](../evidence/work_budget_rb_root_2026-08-13.txt).
The follow-up hereditary-pliability test now gives an exact suffix cutoff after the DP is built and
a cheap sufficient cutoff from absolute slack, tail excess and retained pure corners.  The cheap
bound is sound but captures only 29 of 65 complete no-call cases in the 243 parent-theorem survivors
of the small census; it does not justify an a-priori length trigger.  The per-depth measurement is
complete: full absolute slack strengthens the q/D theorem for 11 partial tails, and actual rejection
rates fall sharply with both slack and `slack-D`, but an exact cutoff that removed 77.89% of lookups
on the hard positive produced no stable CPU speedup.  Keep the measured-cost trigger and the cutoff
diagnostic-only. See
[`../evidence/rb_pliability_2026-08-13.txt`](../evidence/rb_pliability_2026-08-13.txt) and
[`../evidence/rb_slack_profile_2026-08-14.txt`](../evidence/rb_slack_profile_2026-08-14.txt).

**H4 - Structural theory.** Prove or refute fixed-`m` families rather than fitting them.  The
`m=6` closed form and `BBCD` profile are now refuted by the exact `n(10,6)=973` frontier.  The
next question is the large-`k` two-bundle construction/obstruction behind its successful `2+4`
root, not whether the particular `473:6@9` witness scales.

## Proximate goals

### P1 - Make the record trustworthy *(done 2026-08-03)*

The repo reorganisation: source-of-truth tables with per-cell provenance, verified witness
trees, mechanical invariant checks, agent instructions, artifact archival.

**Done when** all witness trees pass `tools/check_witness.py`, `tools/check_tables.py` is
green, and every artifact cited by `data/*.csv` is fetchable by tag.

Delivered: source-of-truth CSVs with per-cell `bound`/`status`/`source`; 13 verified witness
trees; four checking tools; the artifact store `fedork/radio-data` (10 tags, round-trip
verified) with the archiving decision recorded in [data.md](data.md) including what was
deliberately not kept; the vendor-neutral `AGENTS.md` brief with a session-end protocol;
[status.md](status.md) as the cold-start snapshot. Merged to `main` and pushed 2026-08-03.

Credentials are per-repo and leave global config alone: `core.sshCommand` for git,
`GH_CONFIG_DIR=.gh` for `gh`, both as `fedork`.

Durability extension (2026-08-11): new solver output is self-identifying. The canonical builder
embeds commit/source hashes/compiler arguments and emits a binary sidecar; runtime stdout adds exact
arguments, host/OS/CPU/RAM and resource limits. Checkpoints preserve the block and archival rejects
missing/incomplete provenance unless a historical override is explicit.

### P2 - Extend the K=9 column constructively

Canonical search for `m = 7` (predicted 457) and `m = 8` (predicted 447), where both models
agree. Each success is a proof standing on the Singleton Majorization Theorem alone.

```
./run_radio_canon_search_generic.sh 4 9 457 7
./run_radio_canon_search_generic.sh 4 9 447 8
```

**Done when** a validated tree is committed for each, or the attempt is recorded in
[journal.md](journal.md) with its cost and where it stalled.

### P3 - Kill one of the two models

`Sb(432:9)` in 9 decides it: the dyadic-profile model says solvable, the closed form says
the maximum is 431. `Sb(416:10)` replicates the question independently. Details and the
asymmetry of the test in [conjectures.md](conjectures.md#the-discriminating-experiment).

**Done when** one model is refuted, or both searches are recorded as inconclusive with
their cost. Also worth recovering: the length-64 `m = 11` profile string, which the journal
mentions but never wrote down, and which would let `n(9,11)` be predicted at all.

### P4 - Fill the K=9 band m = 7..64

Superseded in part: the 16 `Sa(193)` states are decided, and `parsed_260.txt` has been
recovered, so `run_pareto9.sh` can restart. What remains is the blank band.

Note what the recovered data shows about cost: the near-diagonal walk moved from `m = 96`
down to about `m = 81` over 14 months of wall clock across 81 chained runs, and single states
in that region took weeks each. A frontier walk downward from the diagonal will not reach
`m = 64` on any useful timescale. Approaching the band from small `m` upward with the
canonical search (P2/P3) is the only cheap direction, and P2 should be read as the first step
of P4 rather than as separate work.

**Done when** there is a measured cost model for the band and a decision about whether to
pursue it at all.

### P5 - Correct and finish the paper

- Replace the stale K=8 column (`m = 10..17`) with the values in `data/pareto_sb.csv`.
- Fix lemma (10): `k(k-1)/2`, not `k(k-5)/2`.
- Fill the `<TODO>` sections - Terminology, the Unit Group Triviality Lemma (now proved in
  [theorems/unit-group-elimination.md](theorems/unit-group-elimination.md)), Insights,
  Refuted lemmas.
- Add lemma (12) for `m = 8`, and the `G_k = sum of binomials` closed form.
- Present `Sa(192)` in 10 as a construction with a verified witness tree, and say plainly that
  maximality at k=10 is open. Do not repeat the 2023 `Sa(193)` verdict as established.
- Fix the numbering collision: `(7)` is currently a duplicate of `(5)`, and the sentence
  "(7) holds true k up to 8" plainly refers to `(u1)`.
- State the `k <= 9` / `k = 10` distinction in the `Sa` table itself, not only in prose.

**Done when** the draft passes `tools/check_tables.py` with no stale generated blocks and
contains no number absent from `data/*.csv`.

### P6 - Full star expansion and synchronized majorization for long states

**Delivered 2026-08-09.** The useful child-profile feature was not merely an ordering score. The
[Vertex-Splitting Pullback Lemma](theorems/singleton-majorization.md#vertex-splitting-pullback-lemma-2026-08-09)
proves that every part `(n:m)`, `n>=m`, may be lifted to `m` disjoint singleton stars `(n:1)`.
Therefore the mass-preserving block profile of every solvable state must be weakly majorized by
`G_k`. On an exact-L1 miss, `radiobase.c` applies this sound filter before its dominance-trie
lookup; `radio_verify.c` and `tools/certify.py` implement it independently.

This supersedes the value-order experiment. Against the same warm lower cache, the hard positive
`Sb(15:3,14:3,17:2,8:4,11:2,10:2,19:1,15:1)` in 5 moved from a 300-second timeout to 5.3 CPU seconds.
The exact A+B negative monster is refuted at the root by `714 > 705` at prefix 40 of its expanded
profile, so its previous 237.4-second / 8.1-billion-candidate search is no longer a search problem.
Broad replay and independent-verifier results are in the latest [journal entry](journal.md).

Reproduce the two controls after compiling `radio_one.c` at the stated bounds:

```
tools/capped_run.sh --seconds 300 --rss-gb 8 --label star-hard -- \
  ./radio_one /tmp/k6lab/warm_k4.txt 5 15 3 14 3 17 2 8 4 11 2 10 2 19 1 15 1
tools/capped_run.sh --seconds 300 --rss-gb 8 --label star-monster -- \
  ./radio_one /tmp/k6lab/warm_k5.txt 6 18 8 22 6 15 8 13 9 23 4 23 2 21 2 17 2
```

The old pair/triple/quad deployment remains rejected: its information is already present in the warm
upward-closed prefix cache. The full star filter is different—it is a global arbitrary-part-count
theorem and directly proves 40.6% of the recorded k=5 negatives.

**The synchronized theory is now characterised, but not deployed.** Define `R_0` as full-star
majorization and `R_d` by requiring one legal synchronized split whose three children pass
`R_{d-1}`. These conditions are nested and sound, and `R_k` is exact solvability. The first level is
an additive hinge-capacity problem. The full theorem, a worked `Sb(16:1,12:2)` ladder, and a
width-two counterexample to any single-base majorization rule are in
[the theorem note](theorems/singleton-majorization.md#the-synchronized-majorization-hierarchy-2026-08-09).

Do not add `R_1` as a production pre-pass. The current recursive prefix checks already enforce its
child inequalities, and on the residual four-part positive the first `R_1` witness is dead and
still passes `R_2`; mere feasibility does not distinguish it from the true winning split. Direct
deeper checks also lose to the warmed exact cache on the tested residual negative.

One theorem-level use *is* now deployed in split-table construction (2026-08-10): each local cut
whose one-part child already fails the information bound or `R_0` is omitted from that level's
table. By subgraph monotonicity no completion with the other parts could rescue it. This is the
same cheap necessary test the recursive loop would eventually perform, not an `R_1` search.
Tables are coarse-grained lazy per `(k,sbb)`, exact-sized and contiguous; suffix parts allocate no
table until search reaches them. On the fixed warm positive control, the identical winning split
and top-level counter were found in 32 rather than 43 solver seconds (49.91 rather than 62.05 wall
seconds including cache load), while the warm negative stayed at 0.08-0.09 solver seconds. The
reproducible regression and allocation measurements are in the 2026-08-10 journal entry.

If P6 continues, the ordering benchmark remains
`Sb(29:6,19:9,13:12,36:3)` in 6 and the only promising use is a **bounded, fallback-safe ordering
signal** that approximates deeper synchronization without recursively solving it.  The current
exact-L1 baseline is 26.6 solve seconds with 37,899 top-level splits; compare both numbers, because
a cache-only speed change and an ordering improvement answer different questions.

**Recursive Pareto-lift track (2026-08-12).** A second bounded ordering signal now has a proved
geometric core.  If a lower aligned template `T` uses cut `s`, every lineage-preserving parent cut
lies in the box `s <= X <= s+(P-T)`; its four rectangles componentwise contain the corresponding
lower rectangles.  Searching `L1` shells around the proportional lift and preserving the lower
split's outcome proportions found a new solution of the four-part k=7 control at structural rank 5
in 15 wall seconds, versus 65 wall / 57 solver seconds for ordinary search with the same warm cache.
An adjacent lower point also succeeds, but only under the correct inherited assignment of two equal
components.  See [the theorem note](theorems/recursive-pareto-lift.md) and the fully provenanced
`pareto-lift-2026-08-12` artifacts.

This is not ready for `radiobase.c`.  A literal second recursive step exhausted the complete lower
split's lift box, and a greedy componentwise Pareto upgrade followed by its first solving split also
failed.  The k=7 exhaustive choice corpus is complete; the k=8 remainder is running on the shared
AWS host from 621 of 815 summary-closed prefix blocks.  Let that finish before adding another fitted
heuristic, then analyze the retained antichain of fixed-dimension Pareto upgrades and inequivalent
solving splits for a lineage-preserving choice property.  Only after that corpus is understood
should the minimal production form be tried: a transient lineage-aware hint, one or two templates,
a bounded pass, and unchanged fallback.  Split hints must influence order only; they are not cache
facts.  Use `tools/pareto_census_status.sh` for a one-shot progress query.

**Separate theoretical m=6 thread (updated 2026-08-10).** The first large-`k` classification is
complete: `n(10,6)=973`, with exact rejection at 974 and an independently verified
singleton-majorized tree at 973.  This refutes both the closed form and `BBCD` profile, which
predict 976.  The working root is `[477:2]`, with mixed child `Sb(496:2,477:4)@9`; it avoids the
`Z_7` kernel rather than repairing it.  Next, work forward from this **state family**, not from its
particular stored subtree:

1. classify the working root splits at 973 and express the `m=4 + m=2` mixed-child boundary in
   deficit coordinates;
2. decide the proposed one-level lift, whose only new hard child is
   `Sb(503:1,495:2,478:3)@9` (the first five-minute run was inconclusive, and the literal scaled
   continuation is refuted by the exact negative `Sb(247:1,247:1,240:2,231:2)@8`);
3. derive a parametric lower construction or synchronized upper obstruction for that two-bundle
   frontier, then recover the low-`k` degeneracies backwards.

Do not infer a constant `-3` correction from one value.  Reproduction, correctness argument and
costs are in the latest journal entry and `docs/tools.md`.

**Conditional excess-q construction track (2026-08-14).** Under the explicitly unproved working
assumption that every fixed labelled A/B/C/D pattern eventually reaches a stable atomic-leaf regime,
the corrected diagram gives a concrete branch rather than a missing map.  For
`A=(a:alpha)@k-1` and `B=(b:beta),C=(c:gamma)@k-2`, maximize `d` in
`Sb(d:beta,b:alpha-beta,c:m-alpha-gamma,a-c:gamma)@k-2`, then score the candidate `a+b+d`.
The direct finite mode recovers the proven `m=10` widths at parent levels 5 through 7 and shows why
all Pareto triples must be retained: the repeated `(6,4,3)` height choice falls one short at level 7,
while B-height 5 reaches the frontier.  Next, automate enumeration of admissible A/B/C triples,
rank them with sound D upper bounds, and evaluate exact slices until every still-competitive triple
is decided or explicitly inconclusive.  In parallel, use guarded mixed-frontier pieces to seek an
eventual formula for `d*` and an induction under atom refinement.  The exact generic piece checks
through residual level 11 remain finite evidence only, and this track changes no Pareto datum;
definitions and controls are in
[conjectures.md](conjectures.md#excess-q-pareto-assembly-as-a-variable-d-slice-working-hypothesis-2026-08-14).

## Ordering

P1 first and quickly. The 2023 corpus spent months of compute and, until 2026-08-02, existed
in exactly one place: a zip on one disk. It is unreliable but not worthless - it is the only
record of what has been attempted, and re-deriving from scratch is what costs months.

Then P5 and P2 in parallel - P5 is writing, P2 is compute, so they do not contend. P3 follows
P2, reusing the same tooling and the same feel for which `target_k` values work. P4 is now
more a costing exercise than a plan.

P6 now has two distinct tracks while proof-safe cold `run9` and the retained run3/run8 baselines
continue remotely. The solver track's bounded ordering approximation is deployed in run8/run9:
long candidates receive a geometric local probe while the one/two-segment constructive spine keeps
the shared parent budget.
It reproduced the cold `Sa(192)` path locally in 376.293 CPU seconds, and the remote control has now
passed in 471.6 CPU seconds in run8 and 479.2 in proof-safe run9. Use run8 as the
matched baseline for run9's natural `Sa(193)` progress, visible-attempt cost and memory, but use only
run9 for a new negative claim. Do not add split history before that comparison matures.
The theory track now has two compatible large-k targets: work from the exact `k=10,m=6` mixed-child
boundary toward a parametric construction/obstruction, and test whether recursive Pareto lifting
stabilises once low-k degeneration disappears.  In both cases retain alternative Pareto upgrades
and split classes, then recover small k backwards. Do not use witness-tree shape as evidence that
either track has a unique continuation.

The result-cache prerequisite for H3 is now delivered.  Last-segment positive/negative Pareto fronts
reduce the measured k=5..7 checkpoint storage 11.2x; with the exact-state L1, the full `Sa(192)`
control passes at 0.35 GB peak RSS and 711.7 CPU seconds, with no remaining measured premium over the
734.5-second pre-compaction control. AWS `run7` and the same-chain local continuation used the
now-obsolete `e648e83` progress-gated pass-2 scheduler and were retired on 2026-08-11 after their raw
segments and closed checkpoints were preserved. Run3 remains the untouched live incumbent; run8 is
the cold `9395218` bounded-probe baseline, and run9 is the cold `e7fa747` proof run. Run9 has a
60 GiB individual cap and is stopped first if all live solvers reach 108 GiB combined RSS. Neither
`c13b5d3` nor `e648e83` is a valid performance baseline for the new scheduler. H3 still sits
awkwardly—the answer is probably 192, the evidence is probably right, and neither "probably" belongs
in a paper. Resume only from a run's own output and retain every raw segment; compact does not mean
bounded.
