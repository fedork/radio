# Bulk artifacts: what exists and how to get it

Raw solver logs are the evidence behind every `proven-exhaustive` row in
`data/pareto_sb.csv`. They are far too large for git (~2.1 GB) but losing them means losing
the provenance, so they are archived separately. **This page is the index** - it lives in
the repo so the knowledge of what exists survives even when the bytes do not.

Compression is `zstd -19`, lossless, about **9%** of raw. Nothing is distilled or filtered:
`out_k8.txt` is 99.5% result lines, so there is no noise worth stripping, and the
`with [...]` split witnesses that `parse_out.sh` discards are exactly what witness-tree
reconstruction needs later.

## Status: the store is not yet stood up

> `gh` and SSH on this machine authenticate as **`fedorkar`**, but `fedork/radio`
> belongs to the account **`fedork`**, where `fedorkar` has `pull` only and no `push`.
> The store repo therefore has not been created, and **local commits cannot be pushed to
> origin either**. Resolve by authenticating as `fedork`, or by adding `fedorkar` as a
> collaborator. See [research-plan.md](research-plan.md) item P1.

Once credentials are sorted:

```
gh repo create <owner>/radio-data --private
export RADIO_DATA_REPO=<owner>/radio-data      # tools/artifacts.sh defaults to fedork/radio-data
tools/artifacts.sh push k8-2026-05-12 out_k8.txt
```

## Inventory

Everything below is currently **only** on the machine that produced it. Sizes are raw;
divide by roughly 11 for the archived size.

### Primary evidence - cited by data/pareto_sb.csv and data/pareto_sa.csv

| file | size | date | what it establishes | tag |
|---|---|---|---|---|
| `out_k8.txt` | 1.2 G | 2026-05-12 | K=8 Pareto frontier, m=2..55, both can- and can't-solve lines. Supersedes the older K=8 column at m=10..17. | `k8-2026-05-12` |
| `out_radio_1.txt` | 521 M | 2026-04-27 | Sa ladder: max n proven for k=1..9, plus the `Sa(192)` in 10 construction. Killed while working on `Sa(193)`. | `sa-2026-04-27` |
| `out_k7.txt` | 6.6 M | 2026-03-24 | K=7 frontier | `k7-2026-03-24` |

### K=9 Pareto walk - partial, and the reason `run_pareto9.sh` is broken

| file | size | date | what it contains | tag |
|---|---|---|---|---|
| `pareto9_short.txt` | 73 M | 2026-03-14 | no completed frontier steps | `pareto9-2026-03` |
| `pareto9_short_2.txt` | 158 M | 2026-03-15 | 19 frontier points: `Sb(320..338 : 17)` solvable in 9 | `pareto9-2026-03` |
| `pareto9_short_3.txt` | 97 M | 2026-03-16 | `Sb(339:17)` in 9, still undecided after 11+ hours | `pareto9-2026-03` |

The rest of the K=9 walk - everything with `m >= 18`, produced by `radioSbPareto.c` from the
diagonal downward - **is gone**. `run_pareto9.sh` chains through `parsed_260.txt` and a
`pareto9_N.txt` sequence, all matched by the `*.txt` ignore rule and since deleted. The
script cannot restart until that cache is rebuilt.

### Full-solve outputs (`radio_full`, every top-level split)

| file | size | date | state | tag |
|---|---|---|---|---|
| `out_77_39.txt` | 13 M | 2026-03-24 | `Sb(77:39)` in 8 | `fullsolve-misc` |
| `out_409_11.txt` | 5.1 M | 2026-05-15 | `Sb(409:11)` in 9 - **killed, no verdict** | `fullsolve-misc` |
| `out_410_11.txt` | 5.0 M | 2026-05-15 | `Sb(410:11)` in 9 - **killed, no verdict** | `fullsolve-misc` |
| `out_225_5_184_6.txt` | 1.3 M | 2026-05-15 | `Sb(225:5, 184:6)` in 8 - killed | `fullsolve-misc` |
| `out_231_4_242_2_full.txt` | 1.1 M | 2026-04-06 | `Sb(231:4, 242:2)` in 8 | `fullsolve-misc` |
| `out_55_18.txt` | 1.1 M | 2026-03-24 | `Sb(55:18)` in 7 | `fullsolve-misc` |
| `out_110_3_115_2_121_1_full.txt` | 872 K | 2026-04-06 | `Sb(110:3, 115:2, 121:1)` in 7 | `fullsolve-misc` |
| `out_45_23.txt` | 660 K | 2026-03-24 | `Sb(45:23)` in 7 | `fullsolve-misc` |
| `out_53_2_52_2_57_1_57_1_full.txt` | 364 K | 2026-04-06 | `Sb(53:2, 52:2, 57:1, 57:1)` in 6 | `fullsolve-misc` |
| `out_109_5.txt`, `out_54_4*.txt`, `out_53_4.txt` | < 50 K | 2026-03 | small probes | `fullsolve-misc` |

The 2026-05-15 runs were probing `Sb(409:11)` and `Sb(410:11)` at k=9 against a
hand-guessed value of `409?` for `n(9,11)`. That guess has no support - see
[theorems/special-cases.md](theorems/special-cases.md#refuted). These runs also used
`radio_full`, which is far more expensive than needed; the canonical search is the right
tool. See [tools.md](tools.md).

### Elsewhere

There are further run outputs on other machines that have never been collected. Add them
to a tag and list them here when they surface.

## Committed instead of archived

Small enough to live in git, and too valuable to risk:

- `witnesses/*.tree` - the six verified proof trees.
- `data/pareto_*.csv`, `data/conjectures.csv` - the source-of-truth tables.
- `data/sheets/` - 46 tabs from the three research spreadsheets, values and formulas, with
  the `.xlsx` originals. Imported 2026-08-02; note that the tab named `INCORRECT pareto` in
  `wbA_profiles` in fact holds the *correct* K<=8 table, while `pareto only` in
  `wbB_pareto7` and the embedded grid in `wbA_profiles/formulas` hold the superseded K=8
  column. Do not read numbers out of these; they are kept as source material.

## Rules

1. **Never commit a file over ~1 MB of solver output.** Archive it and add a row above.
2. **Archive before deleting.** The K=9 walk was lost this way.
3. Raw output, not distilled. `parse_out.sh` output can be regenerated; witness data cannot.
4. When a row here is cited as the source for a `data/*.csv` fact, the tag must exist.
