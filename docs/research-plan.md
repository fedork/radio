# Research plan

Living document. Update it when a goal is met or reprioritised; do not accumulate stale
entries. Last revised 2026-08-02.

## High-level goals

**H1 - Publish.** Get the two-coin quantity group testing results out: the `Sa` sequence
proven optimal for `k <= 9`, the `Sb` Pareto frontier proven for `k <= 8`, the `Sa(192)`
in 10 construction, and the two theorems. The draft in `paper/` is close; the mathematics is
further along than the write-up.

**H2 - The K=9 Sb column.** Currently six entries, all but one a lower bound. This is the
gateway to almost everything else, including H3.

**H3 - Is `Sa = 192` optimal at k=10?** Reduces to 16 near-diagonal K=9 states. A positive
answer (193 achievable) would be a better headline; a negative answer completes the table.

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

- **Credentials.** `gh`/SSH authenticate as `fedorkar`; `fedork/radio` grants that account
  pull only. Nothing can be pushed - not the reorganisation branch, not the artifact store.
  Resolve before anything else, since until then all this work exists on one disk.
- Create the artifact store and upload the 2.1 GB corpus. See [data.md](data.md).
- Import and correct the paper draft.

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

### P4 - Rebuild the K=9 frontier, aimed at Sa(193)

The 16 states `Sb(n1 : 193-n1)` in 9 for `n1 = 97..112`. Any one of them solvable settles
`Sa(193)` affirmatively; all sixteen unsolvable proves `Sa(10) = 192`.

Prerequisite: `run_pareto9.sh` cannot restart, because `parsed_260.txt` and its
`pareto9_N.txt` chain were deleted. Rebuild that cache from the surviving
`pareto9_short_*.txt` and `out_k8.txt` via `parse_out.sh` before resuming any frontier walk.

**Done when** each of the 16 is decided, or the undecided remainder is scoped with measured
cost estimates.

### P5 - Correct and finish the paper

- Replace the stale K=8 column (`m = 10..17`) with the values in `data/pareto_sb.csv`.
- Fix lemma (10): `k(k-1)/2`, not `k(k-5)/2`.
- Fill the `<TODO>` sections - Terminology, the Unit Group Triviality Lemma (now proved in
  [theorems/unit-group-elimination.md](theorems/unit-group-elimination.md)), Insights,
  Refuted lemmas.
- Add lemma (12) for `m = 8`, and the `G_k = sum of binomials` closed form.
- Fix the numbering collision: `(7)` is currently a duplicate of `(5)`, and the sentence
  "(7) holds true k up to 8" plainly refers to `(u1)`.
- State the `k <= 9` / `k = 10` distinction in the `Sa` table itself, not only in prose.

**Done when** the draft passes `tools/check_tables.py` with no stale generated blocks and
contains no number absent from `data/*.csv`.

## Ordering

P1 first and quickly - until credentials are fixed, everything is one disk failure from
gone. Then P2 and P5 in parallel: P2 is compute-bound and P5 is writing, so they do not
contend. P3 follows P2 because it reuses the same tooling and the same intuition about what
`target_k` values work. P4 is the largest compute commitment and should not start until the
cheaper K=9 results have calibrated how expensive that regime really is.
