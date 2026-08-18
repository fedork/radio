# Bulk artifacts: what exists, what is archived, and what was deliberately dropped

Raw solver logs are the evidence behind the `proven-exhaustive` rows in `data/pareto_sb.csv`
and `data/pareto_sa.csv`. They are far too large for git, so the valuable subset is archived
to tagged releases on the private repo **`fedork/radio-data`** and indexed here. This page
lives in the public repo so the knowledge of what exists survives the bytes.

Compression is `zstd -19`, lossless, about **10%** of raw.

## The archiving decision (2026-08-03)

Not everything was kept, and the reason is the audit rather than the size.

**Archived** — the 2026 artifacts, including the proof-safe cold `Sa(193)` run and its matched
performance comparator; the accumulated cache; the `pareto9` chain; and from the 2023 snapshot
only `out26_2`/`out26_3` (the historical `Sa(193)` record) plus the `print*` and `full*` files
that the committed witness trees and exhaustive results were extracted from. About 4.1 GB raw,
roughly 460 MB stored.

**Not archived** — `radio.zip` as a whole (3.56 GB). Two reasons:

1. Its ~18 GB of `out*` logs come from the build with **37 provably false negatives**
   ([`../evidence/refuted_2023_negatives.txt`](../evidence/refuted_2023_negatives.txt)). They
   cannot be cited for anything, so the provenance argument that justifies archiving the 2026
   logs does not apply to them.
2. It exceeds GitHub's 2 GB per-asset limit and would need splitting.

Everything of value has been extracted from it — the historical `Sa(193)` lines, 13 verified witness
trees, 16 exhaustive enumerations. Keep the zip on local or external storage as a cold
backstop; it is a record of what has been *attempted*, which is worth something when deciding
what to re-run, but it is not evidence.

## Fetching

```
tools/artifacts.sh list                 # tags in the store
tools/artifacts.sh show <tag>           # manifest: sha256, raw size, source, host, date
tools/artifacts.sh pull <tag> [dest]    # download, verify sha256, decompress
tools/artifacts.sh verify <tag>         # round-trip check without keeping the files
```

`pull` and `verify` fail loudly on a sha256 or byte-count mismatch, so a corrupted asset
cannot be used silently.

`data/artifacts.csv` maps every tag to its assets and their contents, and is what makes a
`tag:path` source in `data/*.csv` checkable offline. One hazard it records explicitly:
**`out26_1.txt` / `out26_2.txt` exist twice** - as ~130-byte stubs in `fullsolve-2026`, and as
the 905 MB / 51 MB files of the same name in `sa193-2023`. Only the latter are evidence.

## Credentials

This repo is owned by the GitHub account **`fedork`**, which is not the machine's default
GitHub login (`fedorkar`). Both tools are pointed at `fedork` **per repo**, leaving global
settings alone:

- **git** - already configured: `core.sshCommand` in `.git/config` selects
  `~/.ssh/id_ed25519_personal`, which authenticates as `fedork`. Nothing to do.
- **gh** - uses an isolated config directory at `.gh/` (gitignored). Populate it once:

  ```
  GH_CONFIG_DIR="$PWD/.gh" gh auth login
  ```

  `tools/artifacts.sh` sets `GH_CONFIG_DIR` automatically when `.gh/` exists, and refuses to
  run with a clear message when it does not. For ad-hoc `gh` commands in this repo, set the
  variable yourself; the global `gh` login stays on `fedorkar`.

Note that `gh auth switch` would have changed the *global* active account, which is why the
config-directory approach is used instead.

Then:

```
gh repo create fedork/radio-data --private
tools/artifacts.sh push k8-2026-05-12 out_k8.txt
```

Override the store location with `RADIO_DATA_REPO` if it should live elsewhere.

## Inventory

Sizes are raw. Divide by roughly 10 for the stored size.

### Archived — cited by data/*.csv

| tag | contents | raw | audited |
|---|---|---|---|
| `k8-2026-05-12` | `out_k8.txt` — K=8 frontier, m=2..55, both can- and can't-solve lines. Supersedes the older K=8 column at m=10..17 | 1.2 G | clean |
| `sa-2026-04-27` | `out_radio_1.txt` — Sa ladder, max n proven for k=1..9 | 521 M | clean |
| `pareto9-2026-03` | `pareto9_short{,_2,_3}.txt` — 19 frontier points `Sb(320..338 : 17)` in 9; `Sb(339:17)` left undecided after 11+ h | 328 M | clean |
| `fullsolve-2026` | `out_k7.txt` (K=7 frontier) and 15 smaller full-solve / probe outputs, tarred | 37 M | clean |
| `cache-2025` | `parsed_260.txt` (19.5 M entries) + the unbroken `pareto9_36..116` chain, tarred | 822 M | clean |
| `sa193-2023` | `out26_2.txt`, `out26_3.txt` — historical `Sa(193)` run, superseded as a verdict source by the cold 2026 proof | 350 M | **suspect era; performance/history only** |
| `trees-2023` | `print*` (26 files, source of 13 verified trees) + `full_*` (18 files, source of the 16 exhaustive results), tarred | 246 M | positives only |
| `small-m-frontier-2026-08-15` | 46 raw frontier logs for every normalized `m<=6`, `k<=9` case, plus 44 forced-root logs classifying the `m=5`, `k=9` transition | 762 K | complete embedded provenance in every solver log; every boundary has `n+1 NO` and `n YES`; all positive trees rechecked; forced-root outcome ranges validated |
| `sa193-cold-2026-08-16` | proof-safe cold run9 log, matched pre-fix run8 comparator, and a metadata tar with run3/run8/run9 source, binaries, provenance, profiles and final sidecars | 710 M | run9 has complete provenance, a passed positive control and all 16 exhaustive root rejections; run8/run3 are performance-only |

"Clean" means `tools/extract_evidence.py audit` found no contradictions. `sa193-2023` comes
from the build with known false negatives — the lines are kept as a historical record and no
longer source the claim. `trees-2023` holds witness trees, which
re-verify from first principles, so its era does not matter.

### Archived — exact bounded structural verdicts

| tag | contents | raw | audited |
|---|---|---:|---|
| `rank1180-depth4-2026-08-15` | eight guided exact-cover logs partitioning rank 1180's depth-four mixed frontier into W-loss classes 1..14 | 612 K | complete provenance; `tools/check_atom_profile_cover_log.py` replays all 6,696 oriented tests, 1,818 distinct mixed children and eight scoped `NO` verdicts |

This tag proves only that `A^27C^3D^2` has no aligned tree of depth at most four in the
32-atom profile model.  It is structural evidence, not a Pareto-table maximality source and not
an all-depth exclusion.

### Archived — performance measurements, not verdict sources

| tag | contents | raw | audited |
|---|---|---:|---|
| `sa193-local-2026-08-10` | `out_sa193.txt` — cold current-engine local trial; `Sa(192)` control passed, partial `Sa(193)` search was stopped at 7.1 GB `vmmap` footprint | 26 M | 0 contradictions among 751 comparable states; **aborted, no `Sa(193)` verdict** |
| `bounded-probe-2026-08-11` | cold `Sa(192)` control plus focused k=5/7/8/9 scheduler replays; all carry complete embedded build/run provenance | 15.8 M | 1,038-answer regression and sanitizer gates passed; performance/positive-path evidence only |
| `bounded-probe-rejected-2026-08-11` | universal two-second probe experiments: the enclosing k=8 pass timed out and the cold control re-entered an unbounded pass-2 dive | 57.0 M | rejected performance experiments; no negative claim |
| `sa193-local-chain-2026-08-11` | four interrupted local `Sa(193)` segments, their closed recovery checkpoint, and a metadata tar containing each `run.meta`, frozen binary, monitor/completion/stderr and recovery guard | 157 M | 0 contradictions in each raw segment; **pre-banner logs archived with the explicit legacy-provenance override**; sidecars supply commits, source/binary hashes, commands and limits; no `Sa(193)` verdict |
| `pareto-lift-2026-08-12` | ordinary four-part control, two lower-template solutions, two recursive-lift successes and their capped-run sidecars | 12.9 M | fully provenanced positive-path/performance evidence; no negative or maximality claim |
| [`verifier-pipeline-2026-08-17`](https://github.com/fedork/radio-data/releases/tag/verifier-pipeline-2026-08-17) | complete Sa(66) 1/2/4/8/14/16-worker sweep; Sa(113) normalized and colored certificates; independent 8/14/16-worker replays; exact source, scripts, logs, hashes and host topology | 83.0 M | logs are provenance-complete, but the solver-core refuter later exposed nine gaps in the colored Sa(113) support set; retain as performance evidence, not a colored proof. The contained pre-banner solver corpus is explicitly legacy input. |

### Not archived

| | raw | why |
|---|---|---|
| `radio.zip` (`~/radio_old`) | 3.56 G → 23 G | ~18 GB of unreliable 2023 `out*`; over the 2 GB asset limit. Everything of value extracted. Cold backstop only. |
| interrupted full-run9 coloring attempts (`run9-verifier/20260817T163700Z`, `run9-verifier-progress/20260818T014906Z`) | two 106-MB normalized inputs plus small diagnostics | No colored certificate or replay verdict was produced; each normalized input is a derived duplicate of the archived run9 raw proof source. Final counters and hashes are committed in `evidence/`; operational objects remain in the private S3 bucket. |
| superseded ordinary run9 verifier gates (`run9-verifier-progress/20260818T055255Z`, `.../20260818T062429Z`) | duplicated normalized input plus bounded logs | The first only established canonical-order cost; the second was stopped at 251,131/2,576,885 k=7 claims after showing that independent search still exceeded the solver's work. Final manifests were checked and measurements are committed; neither completed a proof checkpoint. |

### Extracted into git, so the bulk is no longer load-bearing

- `evidence/pareto_certification_k1_8.txt` — all 276 obligations behind the K≤8 table, 30 KB
- `evidence/sa193_unsolvable_in_10.txt` — the 16 proof-safe cold root refutations, plus exact provenance and hashes
- `evidence/sa193_run_comparison_2026-08-16.txt` — final run3/run8/run9 classification and measurements
- `evidence/refuted_2023_negatives.txt` — the 31 verdicts that corpus got wrong
- `witnesses/*.tree` — 18 verified trees, including the singleton-majorized proof of `Sb(481:5)@9`
- `data/exhaustive_multipart.csv` — the 16 `full*` enumerations
- `data/pareto_sb.csv` — 52 K=9 rows: six exact small-`m` maxima, 16 proof-safe upper bounds and retained legacy lower bounds

### Warm-starting from `parsed_260.txt`: two traps

The cache audits clean — 0 contradictions, 0 information-bound violations, 0 header
mismatches across 19,507,378 entries, and it carries **none** of the 31 known-bad verdicts in
[`../evidence/refuted_2023_negatives.txt`](../evidence/refuted_2023_negatives.txt). So a warm
start will not reintroduce any *identified* error. But:

1. **It contains the 16 historical `Sa(193)` verdicts.** Re-running those states with this cache
   loaded would read the old answers straight back and "confirm" them. This is why the successful
   H3 run started cold; the cache still cannot be used to independently reproduce that negative.

2. **Absence of known errors is not absence of errors.** Only 1,023 of 19.5 M entries are
   checkable against the proven tables at all; the rest are multi-part states or `k = 9`,
   where there is nothing to check against. The cache was accumulated from runs spanning
   2023–2025 and does **not** record which build produced each line, so 2023-era negatives
   cannot be filtered out. A warm-started result is only as sound as the cache beneath it.

The practical consequence: `parsed_260.txt` is excellent for *finding* solutions — a positive
result can always be re-verified as a witness tree, so a poisoned negative can only slow the
search, never corrupt the answer. It is **not** a safe basis for a new negative result. Cold
runs are the only sound route to new negatives. The completed run9 result followed that rule.

### Elsewhere

There may be further run outputs on other machines. Run
`tools/extract_evidence.py audit` over anything that surfaces before trusting it, then add a
row here.

## Committed instead of archived

Small enough to live in git, and too valuable to risk:

- `witnesses/*.tree` - the 18 verified proof trees.
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
5. New solver logs must contain a complete `radio-provenance-v1` block and pass
   `tools/check_provenance.py`. `tools/artifacts.sh push` enforces this. The
   `RADIO_ALLOW_LEGACY_PROVENANCE=1` override is only for a historical/pre-banner log, and its
   missing provenance must be stated in this inventory.
