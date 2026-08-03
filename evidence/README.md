# Certification excerpts

The raw solver corpus is around 26 GB spread across machines and archives. The part of it
that actually certifies the published tables is about 30 KB — two log lines per frontier
cell. Those lines live here, in git, so the provenance outlives the logs.

A `witnesses/*.tree` file proves a state *is* solvable and can be re-checked from first
principles. The files here are different: they are verbatim excerpts of solver output,
recording **negative** results and exhaustive searches, which cannot be re-derived cheaply.
They are evidence, not proof — they rest on the solver being correct.

| file | certifies |
|---|---|
| `pareto_certification_k1_8.txt` | every `proven-exhaustive` cell of the Sb frontier for K=1..8, and the Sa sequence for k=1..9. 276 obligations, 276 located. |
| `sa193_unsolvable_in_10.txt` | `Sa(193)` is not solvable in 10, hence `Sa(10) = 192` is maximal |

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

It is the most expensive single fact in the project and the least reproducible.

`Sa(n)` in `k` splits into a taken group of `n1` and the rest, needing `Sa(n1)` in `k-1` and
`Sb(n1 : n-n1)` in `k-1`. Since `Sa(n1)` in 9 forces `n1 <= 112`, deciding `Sa(193)` in 10
comes down to sixteen states — `Sb(n1 : 193-n1)` for `n1 = 97..112`. All sixteen were
exhaustively refuted:

- `Sb(112:81)` alone took **1,725,456 s** (~20 days, 12 passes), in `radio/out26_2.txt`
- the remaining fifteen plus the `Sa(193)` verdict took **2,353,729 s** (~27 days), in
  `radio/out26_3.txt`, which had `Sb(112:81)` already warm in its loaded cache

That is roughly 47 days of solve time, and the original run needed about 90 GB of virtual
memory. Recomputation is not a realistic check. These lines are the record, and the sixteen
`Sb` verdicts are also recorded as `bound=upper` rows in `data/pareto_sb.csv`, where they
constrain the K=9 frontier for `m = 81..96`.

Both source files were recovered on 2026-08-02 from `radio.zip` in `~/radio_old`, a
2023-10-18 snapshot. Archive them before that disk is reused — see [../docs/data.md](../docs/data.md).
