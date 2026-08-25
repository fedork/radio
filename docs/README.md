# Documentation

Two-defective quantity group testing: how large a coin set can be resolved in `k` tests.

## Start here

| | |
|---|---|
| [problem.md](problem.md) | Definitions, notation, the information bound, the base sequence `G_k` |
| [results.md](results.md) | What is established, with provenance |
| [literature.md](literature.md) | Primary-source map, exact imported theorems, scalable algorithms, citation notes |
| [publishable-claims.md](publishable-claims.md) | Conservative inventory of paper-worthy results, proof standards and exclusions |
| [conjectures.md](conjectures.md) | What is predicted, and the experiment that would settle each |
| [research-plan.md](research-plan.md) | Goals and what to work on next |
| [tools.md](tools.md) | The solver, its drivers, and the verification tools |
| [parallel-solver.md](parallel-solver.md) | Frozen-cache epochs, worker ownership and limited-width batching plan |
| [data.md](data.md) | Index of bulk artifacts and how to fetch them |
| [journal.md](journal.md) | Running log. History, not fact. |
| [theorems/](theorems/) | Proofs |
| [theorems/m5-pareto-assembly.md](theorems/m5-pareto-assembly.md) | Exact `m=5` theorem translated into the competing `3+2` / `4+1` assembly branches and a sharp symbolic D slice |
| [theorems/recursive-pareto-lift.md](theorems/recursive-pareto-lift.md) | Proved lift-box lemma and the open recursive Pareto-lift programme |
| [theorems/atom-lineage.md](theorems/atom-lineage.md) | All-depth D-lineage/kernel obstructions, exact eight-/sixteen-atom height-6 optima, and the one-rank 32-atom gap |

## How facts are recorded

The files under `data/` are the single source of truth. Everything else references them.

- `data/pareto_sb.csv` - the `Sb` Pareto frontier: `k, m, n1, bound, status, source, note`
- `data/pareto_sa.csv` - the `Sa` sequence
- `data/conjectures.csv` - closed forms and dyadic profiles, stored executably
- `data/exhaustive_multipart.csv` - complete `all_solutions` enumerations of multi-part states
- `data/artifacts.csv` - the archive: `tag, asset, contains, era, audited, note`

A `source` must be **resolvable**, in one of three forms:

| form | example |
|---|---|
| a file in the repo | `witnesses/canon_473_6_at9.tree` |
| a theorem reference | `lemma-6`, `docs/theorems/special-cases.md#lemma-1` |
| `tag:path` into the archive | `k8-2026-05-12:out_k8.txt` |

A bare log filename is **not** acceptable - nobody can find it later. `tools/check_tables.py`
rejects unresolvable sources and unknown tags.

Every row carries a **bound** and a **status**:

`bound` is `max` (proven maximal), `lower` (a solution exists, maximality open) or `upper`
(exhaustively refuted above this value). The distinction matters and is easy to lose in prose:
"457 is solvable in 9" and "457 is the maximum" are different claims.

| status | meaning |
|---|---|
| `proven-exhaustive` | retained evidence contains a complete rejection at `n+1` and either a positive verdict or independently checked witness at `n` |
| `proven-theorem` | follows from a proved theorem or lemma, which `source` names |
| `witness` | a machine-checked witness tree exists; lower bound only |
| `solver-lower` | the solver reported a solution, not independently recheckable |
| `legacy` | asserted by an earlier run whose artifact was not retained |
| `conjecture` | model prediction, no proof |
| `refuted` | tested and false; kept so it is not re-derived |

Anything claiming evidence must name a `source`; `tools/check_tables.py` enforces this.

### Rules

1. **Never copy a table.** Reference `data/*.csv`. Where a table needs to be readable in
   prose, wrap it in `<!-- generated:NAME -->` / `<!-- /generated -->` and produce it with
   `tools/check_tables.py --render`. The checker fails if a generated block drifts.
2. **Never state a number without a status.** "n(9,7) = 457" is not established; "457 is a
   conjectured value with no witness yet" is accurate.
3. **Verify before recording.** `tools/check_witness.py` on any new tree.
4. **Update in place.** When something is superseded, replace it. Do not stack a
   contradicting note beside the old one - that is how four copies of the K=8 column came to
   exist with three of them stale.

## Checks

```
tools/check_tables.py                    # invariants, formulas, generated blocks
tools/check_witness.py witnesses/*.tree  # re-derive every proof from first principles
```

Both are fast, dependency-free, and worth running on every change.

## Why the discipline

Two errors that had already made it into circulating documents, both now caught
automatically:

- The K=8 column existed in four places and three were stale at `m = 10..17`. The only
  correct copy sat in a spreadsheet tab labelled `INCORRECT pareto`.
- Lemma (10) was transcribed with `k(k-5)/2` in place of `k(k-1)/2`, which asserted that
  unsolvable states were solvable.
