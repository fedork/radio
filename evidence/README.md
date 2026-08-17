# Certification excerpts

The raw solver corpus is around 26 GB spread across machines and archives. The part of it
that actually certifies the published tables is about 30 KB — two log lines per frontier
cell. Those lines live here, in git, so the provenance outlives the logs.

A `witnesses/*.tree` file proves a state *is* solvable and can be re-checked from first
principles. Most files here are different: they are verbatim excerpts of solver output,
recording **negative** results and exhaustive searches, which cannot be re-derived cheaply.
They are evidence, not proof — they rest on the solver being correct. The `atom_profile_*.cert`
files are the exception: they are finite symbolic proof objects replayed by independent checkers.

| file | certifies |
|---|---|
| `pareto_certification_k1_8.txt` | every `proven-exhaustive` cell of the Sb frontier for K=1..8, and the Sa sequence for k=1..9. 276 obligations, 276 located. |
| `sa193_unsolvable_in_10.txt` | the sixteen proof-safe cold run9 root refutations, establishing that `Sa(193)` is not solvable in 10 and hence `Sa(10)=192` is maximal |

Symbolic aligned-profile proof objects:

| file | certifies |
|---|---|
| `atom_profile_height6_ad8.cert` | the D-lineage exclusion through eight-atom rank 81 and the exact rank-82 construction |
| `atom_profile_height6_dc16.cert` | the two-coordinate exclusion through sixteen-atom rank 304 and the first projected rank-305 tree |
| `atom_profile_height6_rank305.cert` | an exact alternative rank-305 tree, establishing the sixteen-atom optimum |
| `atom_profile_height6_dc32.cert` | the 504-core two-coordinate exclusion of 32-atom ranks 1090--1179; together with D lineage this excludes every rank through 1179 |

Engineering investigations are retained separately because they explain solver policy rather than
certify a frontier cell:

| file | records |
|---|---|
| `deadline_stall_2026-08-10.txt` | why finite descendants must share an absolute allowance and long-state probes must deepen monotonically; CPU-clock incarnation used by the frozen Sa(193) runs |
| `work_budget_rb_root_2026-08-13.txt` | calibration and regression of deterministic accepted-prefix budgets; exact meaning, measured power, and rejected eager use of `rb_dead(0,0,0,0)` |
| `rb_pliability_2026-08-13.txt` | exact hereditary suffix pliability and the first absolute-slack/tail-length certificates |
| `rb_slack_profile_2026-08-14.txt` | full-slack q/D refinement, actual suffix call/prune census, real controls and rejected exact cutoff |
| `sa193_run_comparison_2026-08-16.txt` | final run3/run8/run9 costs, hashes, classifications and the exact run8/run9 fact-set comparison |
| `verifier_pipeline_benchmark_2026-08-17.txt` | complete Sa(66) thread/affinity sweep and Sa(113) sanitize, minimalize/color, replay and 8/14/16-worker comparison on the AWS run9 hardware class |

## Regenerating and auditing

```
tools/extract_evidence.py certify out_k7.txt out_radio_1.txt out_k8.txt \
    --out evidence/pareto_certification_k1_8.txt

tools/extract_evidence.py audit <any log or parsed cache>
```

`certify` reports any obligation it cannot meet, so a cell losing its evidence is visible
rather than silent. `audit` is what to run before trusting an unfamiliar log: it checks for
claims that contradict the proven tables, self-contradictions, and information-bound
violations.

## Why `Sa(193)` gets its own file

It is the most expensive current proof artifact in the project and closes the `Sa` sequence
through `k=10`.

`Sa(n)` in `k` splits into a taken group of `n1` and the rest, needing `Sa(n1)` in `k-1` and
`Sb(n1 : n-n1)` in `k-1`. Since `Sa(n1)` in 9 forces `n1 <= 112`, deciding `Sa(193)` in 10
comes down to sixteen states — `Sb(n1 : 193-n1)` for `n1=97..112`. Proof-safe cold run9
exhaustively refuted all sixteen in **419353.1 CPU seconds** and **419849 wall seconds**, with
**1.32 GB peak RSS**. Its `Sa(192)` positive control passed first, and the whole run stayed in
one session from an empty cache.

The raw log is archived as `sa193-cold-2026-08-16:run9_out_sa193.txt`. Its committed excerpt
records all sixteen root lines, full source/build identity, raw hash and size. The corresponding
`bound=upper`, `status=proven-exhaustive` rows constrain the K=9 frontier for `m=81..96`.

The recovered 2023 run needed roughly 47 solve-days and about 90 GB of virtual memory, but its
build produced known false negatives. It remains archived as historical cost evidence and no
longer carries the mathematical claim. See [../docs/data.md](../docs/data.md).
