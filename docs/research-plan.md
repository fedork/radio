# Research plan

Living document. Update it when a goal is met or reprioritised; do not accumulate stale
entries. Last revised 2026-08-02.

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

**H4 - Structural theory.** Prove the closed-form family rather than fitting it. Resolve the
canonical decomposition matrix questions: the missing second generator, and whether
`473:6 @9` lifts to a scalable compact matrix.

## Proximate goals

### P1 - Make the record trustworthy *(in progress)*

The repo reorganisation: source-of-truth tables with per-cell provenance, verified witness
trees, mechanical invariant checks, agent instructions, artifact archival.

**Done when** all witness trees pass `tools/check_witness.py`, `tools/check_tables.py` is
green, and every artifact cited by `data/*.csv` is fetchable by tag.

Outstanding:

- Push the branch. Until then the reorganisation exists on one disk.

Done: credentials (repo-local `core.sshCommand` for git, `GH_CONFIG_DIR=.gh` for `gh`, both
as `fedork`); the artifact store `fedork/radio-data` with seven tags, round-trip verified;
the archiving decision recorded in [data.md](data.md) including what was deliberately not
kept.

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

## Ordering

P1 first and quickly. The 2023 corpus spent months of compute and, until 2026-08-02, existed
in exactly one place: a zip on one disk. It is unreliable but not worthless - it is the only
record of what has been attempted, and re-deriving from scratch is what costs months.

Then P5 and P2 in parallel - P5 is writing, P2 is compute, so they do not contend. P3 follows
P2, reusing the same tooling and the same feel for which `target_k` values work. P4 is now
more a costing exercise than a plan.

H3 sits awkwardly: the answer is probably 192, the evidence is probably right, and neither
"probably" belongs in a paper. Since the draft only claims optimality through k=9, nothing is
blocked - so re-running the 16 states is worth doing when spare compute exists, not before.
