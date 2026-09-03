# A cleanroom-checkable `K=8` Pareto frontier certificate, 2026-09-03

Task: extract certified witness trees and a refutation certificate from "the recent Pareto-8 run",
in a form the current verifier of record — the cleanroom checker `tools/cleanroom` — accepts.

Outcome in one line: the refutation certificate exists and its load-bearing top level verifies with
zero gaps, but it comes from `out_k8.txt`, **not** from the recent census run, which cannot supply
one at all. Certified trees do not come from any solver log; they come from canonical search, and
the cleanroom cannot check them because it has no positive mode.

## The recent Pareto-8 run is not a certificate source

The only recent `k=8` run is the Pareto-*prefix choice census*
(`pareto-census-k8-2026-08-19`, local copy `.artifacts/pareto-census-k8/out.txt`, 94,447,895 bytes,
`tools/check_provenance.py` OK, `build_id=d9a89e30…`, commit `54486d6`). It is not a frontier solve
and its solver verdicts stop well below `k=8`:

| | k=3 | k=4 | k=5 | k=6 | k=7 | k=8 |
|---|---|---|---|---|---|---|
| `can't solve` | 1 | 28,779 | 502,220 | 21,120 | 0 | 0 |
| `can solve` | 84 | 22,912 | 22,021 | 775 | 0 | 0 |

There is no `k=7` or `k=8` negative in it, so it contains no frontier maximality claim to certify.
It is also not closed. Normalizing it with `CERT_ONLY=1 CERT_OUT=… ./radio_verify` yields 552,120
facts and **0 explicit roots**; auditing its `k<=4` levels gives

```
RESULT_LEVEL k=4 completed=28779 verified=249 gaps=28530 contradicted=0
TOTAL verified 249, gaps 28531
```

The cause is structural, not a transcription bug: the census ran from a preloaded 934 MB cache
(`input.tar.zst`, `exact.cache` + `dominance.cache`), and cache-resident facts were never
reprinted, so level `k-1` cannot support level `k`.

The seed cache is available locally and holds 11,476,223 negatives (`k=1..8`), but it is **not** a
substitute. Transcribing its distilled `- b n1 m1 … t pairs n k` records into certificate facts
requires dropping or keeping unit `(1:1)` parts, and dropping them is unsound: Unit-Group
Elimination gives `S = R + units` solvable in `k` iff `mass(S) <= 3^k` and `R` solvable in `k`, so
"`R` unsolvable" is strictly **stronger** than "`R + units` unsolvable" — `R` can be solvable while
`R + units` fails the information bound. A first attempt at this conversion was discarded for
exactly that reason.

## `out_k8.txt` is the run that carries the frontier

`out_k8.txt` (1,329,497,429 bytes, 2026-05-12) is a **cold single-session** `K=8` sweep: no cache
load line anywhere in its preamble, and it terminates normally on `can solve Sb(256:1) … in 8`.

| | k=1 | k=2 | k=3 | k=4 | k=5 | k=6 | k=7 | k=8 |
|---|---|---|---|---|---|---|---|---|
| `can't solve` | 1 | 9 | 633 | 842,545 | 7,845,253 | 2,520,118 | 6,852 | 54 |
| `can solve` with `[...]` | 1 | 13 | 1,184 | 47,910 | 130,172 | 4,389 | 779 | 201 |

The 54 `k=8` negatives are exactly the frontier refutations `Sb(57:55)`, `Sb(58:54)`, …,
`Sb(249:3)`, `Sb(256:2)` — one per `m=2..55` — and each is `n1+1` for the corresponding
`proven-exhaustive` row of `data/pareto_sb.csv`. Every one of the 54 matches the table. (`m=1` has
no refutation because its `2^k` upper bound is Lemma 1, the dichotomy argument.)

`out_k8.txt` **fails** `tools/check_provenance.py` — it predates the provenance banner — so it
cannot be archived as durable raw evidence. That does not weaken a certificate derived from it: the
cleanroom shares no solver code and re-derives every split from Singleton Majorization Necessity,
Unit-Group Elimination, Subgraph Monotonicity and the split semantics. The seed log is a *hint
list*; the certificate plus its zero-gap audit is the evidence.

## The certificate

`tools/log_to_v1_cert.sh` reduces the log to a `radio-negative-certificate-v1` in one streaming
pass (443,851,829 bytes, **11,215,465** claims: 54 `root` at `k=8`, 11,215,411 `fact` below).
`LC_ALL=C sort -u` removed nothing, so the solver had already emitted each state once in a
canonical part order. Parts are written **verbatim**; the checker applies CANON and UNIT itself and
recomputes every mass, so no annotation can enter the proof and the unit-stripping trap above
cannot recur.

It compresses to 47,157,311 bytes at `zstd -3` (10.6x), so the natural durable home is a
`pareto8-certificate-2026-09-03` release via `tools/artifacts.sh push` plus a
[data.md](../docs/data.md) row — the same shape as `sa193-certificate-2026-08-19`, which held
2,846,568 claims in 15.6 MB. Not pushed yet: the chain is not complete until `k=5` verifies, and
the release should carry the audit logs for every level.

### Audit results (cleanroom `radio_cleanroom`, 10 threads, Apple M4 Pro)

| level | claims | support | verified | gaps | notes |
|---|---|---|---|---|---|
| k=1 | 1 | 0 | 1 | **0** | |
| k=2 | 9 | 1 | 9 | **0** | |
| k=3 | 633 | 9 | 633 | **0** | |
| k=4 | 842,545 | 633 | 842,545 | **0** | 1.787e9 cells, 17.2 s |
| k=5 | 7,845,253 | 842,545 | *running on AWS* | **0** in first 7,786 | see cost below |
| k=6 | 2,520,118 | 7,845,253 | 2,520,118 | **0** | build 362.7 s, audit 886.4 s, 92.6e9 cells |
| k=7 | 6,852 | 2,520,118 | 6,852 | **0** | build 83.2 s, audit 0.41 s |
| **k=8** | **54** | 6,852 | **54** | **0** | `dead_options=148018/148018` |

The `k=8` level certificate is committed as
[`pareto8_level8_2026-09-03.cert`](pareto8_level8_2026-09-03.cert) — 107,590 bytes, 1,226 parts,
6,852 support facts, 54 claims — so the frontier refutations are re-checkable from the repository
alone:

```
tools/cleanroom/target/release/radio_cleanroom audit evidence/pareto8_level8_2026-09-03.cert
TOTAL verified 54, gaps 0, cells 0
```

The `k=8` line is the load-bearing one: all 54 frontier refutations verify, and *every* legal first
split of *every* root is discharged by the `k=7` support. The `k<=4` block closes on only 633
`k=3` facts. `k=7` closes against `k=6`. So `out_k8.txt` is genuinely closed wherever it has been
checked, and no gap or contradiction has appeared anywhere.

Only `k=5` remains, and it is the one expensive level.

### `k=6` closed locally, complete and clean

A uniform 1/1000 sample first (2,478 claims/s, 0 gaps), then the whole level:

```
BUILD  k=6 facts=7845253 redundant=2188688 closure_tuples=80764271 closure_nodes=103644156 wall_s=362.696
RESULT_LEVEL k=6 completed=2520118 verified=2520118 gaps=0 contradicted=0 cells=92620060382 wall_s=886.413 rate=2843/s
```

**All 2,520,118 claims verified, zero gaps**, in 362.7 s of support build plus 886.4 s of audit at
10 threads, peak RSS 3.50 GB. 2,188,688 of the 7,845,253 `k=5` support facts are discarded as
redundant at load, which is why a level with three times `k=5`'s support is a hundred times
cheaper per claim.

### The `k=5` audit run

| field | value |
|---|---|
| run ID | `20260903T163559Z` |
| instance | `i-0f000d13c1fe92cf1`, on-demand `c8a.24xlarge` (96 vCPU), `us-west-2b` |
| launched | 2026-09-03 16:36:31 UTC, 20 h hard stop |
| source commit | `42ce13d97cf379a56cf0ec77aacfadbeddf49f41` |
| source bundle SHA-256 | `3c850d665f1100ff19e34be0b6554fce892db5f69fd5c2507888d6d7f3908afb` |
| audited artifact | `pareto8-k1to8.cert.zst`, 47,157,311 bytes |
| artifact SHA-256 | `f335c6b4b21e696e6a936b8166c6474a7bdeaa35904162dba3116e16f9e2abda` |
| gate | `--expect-claims 11215465`, so the run fails unless it accounts for every claim |
| S3 prefix | `s3://radio-sa193-393287594714/cleanroom-verify/20260903T163559Z/` |

It audits the flat certificate end to end, so it re-derives `k=1..4` and `k=6..8` as well as `k=5`.
That makes it an independent replay of the local results on different hardware, and the shared
levels agree exactly: `k=4` reported the identical 1,787,253,880 cells in 2.201 s against 17.243 s
locally (7.8x on 96 vs 10 threads). Cell counts are hardware-independent; the seconds are not.

Read progress with `tools/cleanroom_ec2_status.sh`. One operational note: the `eta_s` field is
computed from the last 300-second window and swings between about 4,000 and 38,000 depending on
whether a few very expensive claims landed in that window. `rate_total` is the stable figure and
climbed 155 -> 438 claims/s as the run cleared the widest claims first.

### `k=5` is the cost centre

Measured directly, not estimated:

```
PROGRESS k=5 elapsed_s=305.3 completed=7786/7845253 percent=0.0992 verified=7786 gaps=0
             cells=20704807273 rate_total=25.51/s eta_s=307270
```

**25.51 claims/s → 307,270 s ≈ 3.6 days wall at 10 threads ≈ 853 thread-hours.**

The cost is intrinsic to the level's shape, not a checker defect. `k=5` claims are wide:

| np | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
|---|---|---|---|---|---|---|---|---|
| claims | 13 | 601 | 19,819 | 164,333 | 771,136 | 2,830,015 | 3,533,480 | 525,856 |

6.9M of the 7.85M claims have 6–8 parts, and the audit enumerates every legal split of each. That
is 2.66M cells per claim against 2,121 per claim at `k=4` — a 1,250x jump. This is the same wall
the `Sa(193)` chain hit at its `k=7` level, which is why `tools/cleanroom_verify_chain.sh` excludes
`k=7` locally by default.

There is no cheap trim. Restricting the certificate to the facts reachable from the 54 roots would
help, but computing that reachable set requires citation tracing, which only the retired
`radio_verify`/`radio_refute` coloring path provides — and the `Sa(193)` measurements put coloring
at +8.0% cost for a −4.14% claim reduction. Load-time redundancy elimination is already applied by
the checker itself (`redundant=3300` of 6,852 at `k=8`; `redundant=728221` of 2,520,118 at `k=7`).

## Certified trees

Five `K=8` frontier maxima now have unconditional canonical witness trees, verified by
`tools/check_witness.py` with no `[majorized G_k]` terminals:

| cell | file | nodes | splits | terminals |
|---|---|---|---|---|
| `Sb(256:1)@8` | `witnesses/canon_256_1_at8.tree` | 1 | 0 | 1 |
| `Sb(255:2)@8` | `witnesses/canon_255_2_at8.tree` | 4 | 1 | 3 |
| `Sb(248:3)@8` | `witnesses/canon_248_3_at8.tree` | 23 | 7 | 16 |
| `Sb(242:4)@8` | `witnesses/canon_242_4_at8.tree` | 335 | 110 | 225 |
| `Sb(231:5)@8` | `witnesses/canon_231_5_at8.tree` | 1,364 | 450 | 914 |

`canon_248_3_at8.tree` was already committed; the other four are new. All were produced by
`radio_canon_search_generic` (`canon8n`, `MAX_K=8 MAX_N=260 MAX_STATE_SIZE=1024
MAX_TREE_NODES=400000 MAX_MEMO=4000000`), *not* extracted from a log — see the next section for why
that matters.

A `target_k=3` sweep over `m=1,…,10,15,20,30,40,55` under a 120 CPU-second per-cell cap gives:

- `TREE`/`TERMINAL`: `m=1,2,3,4,5`
- genuine `NO_CANONICAL_TREE` exhaustion: `m=6` (`Sb(225:6)`), `m=8` (`Sb(206:8)`); and `m=55`
  (`Sb(56:55)`) under a separate 900-second cap, `REACH: 1759897 tested, 962372 pruned`
- hit the cap, therefore **unknown**: `m=7,9,10,15,20,30,40`

A CPU cap is not evidence of absence, any more than `out of nodes` is. So canonical coverage stops
at `m=5` with three proved holes above it, mirroring `K=9` where trees exist only for `m=4,5,6`.

### Domination closes the other fifty cells, with no solver run

The numbered format needs no `G_k` leaf. It discharges a child with `(line M)` whenever line M's
state **dominates** it after unit-group deletion — Subgraph Monotonicity plus Unit-Group
Elimination — and that is how a solver log already carries the evidence. Most children are never
logged in their own right because something already logged embeds them: `Sb(109:2)@7` appears
nowhere in `out_k8.txt` while `Sb(109:5)@7` does, and `109<=109, 2<=5`.

`tools/log_to_numbered_tree.py` exploits exactly that, resolving each child as (1) trivial when
unit-only, (2) an exact logged positive, or (3) a logged positive that dominates it. Run over
`out_k8.txt` for the fifty cells `m=6..55`:

```
indexed 184649 distinct logged positives (0 child-mismatch lines rejected)
built 50, gaps 0
```

Every cell resolved, in **87–183 lines** each, and `tools/check_witness.py` verifies all fifty
unconditionally. **No solver fill-in was needed.** The tool re-derives split semantics from
`docs/problem.md` rather than importing them from the checker, so running the checker afterwards is
a genuine cross-check; as a second guard it compares every derived child triple against the child
states the solver printed on the same line, and all 184,649 logged positives agreed.

The `K=8` frontier therefore has an unconditional achievability witness for **every** cell
`m=1..55`, and the standing lesson is: when a positive looks absent from a log, check domination
before starting a solver.

Because `Sb(256:1)@8` now has a tree, the `8,1,256` row's note in `data/pareto_sb.csv` was changed
in place from "achievability logged in out_k8.txt" to cite the witness instead.

## What is missing

1. **The cleanroom has no positive mode at all.** `radio_cleanroom` exposes exactly two
   subcommands, `audit` (negative certificates, `level-v2` or flat `v1`) and `selftest`. Nothing in
   `src/` verifies a tree. So "certified trees compatible with the cleanroom" does not currently
   exist as a thing: trees are checked by `tools/check_witness.py`, a separate Python checker with
   its own trust base. Making the cleanroom cover both sides means adding a `tree` subcommand — the
   `G_k`/`U_k` terminal logic it would need is already in `src/gk.rs`.
2. **`tools/extract_witness_tree.py` output is verifiable by nothing** — it renders a third format
   (`- #N <state> in k: <status>` plus `source:` lines) that `check_witness.py` does not parse.
   Its docstring now says so and points at `tools/log_to_numbered_tree.py`. Navigation tool, not
   evidence.
3. **`k=5` of the chain is unverified** — running on AWS as of this record
   (`tools/cleanroom_ec2_status.sh`, run `20260903T163559Z`, `c8a.24xlarge`, ~10.4 h projected,
   zero gaps through the first 108,420 claims). It is the single remaining obstacle to a complete
   induction; every other level is closed.
4. **No provenanced raw `K=8` log exists.** `out_k8.txt` cannot be archived under the current rule.
   The certificate can be, and is the more useful artifact, but a clean re-run is the only way to
   get an archivable raw log.
5. **`radio_print.c` cannot use domination through the cache.** Given the positives-only cache it
   reported `found no solutions` for `Sb(225:6)@8`, because its `CACHE_ONLY` child check wants an
   exact cached positive and `Sb(109:2)@7` is only present by embedding in `Sb(109:5)@7`. Its own
   `is_inferior` does domination between *lines*, but not against the cache. Its bounds are now
   `-D` overridable and it takes an Sb target (`<cache> k n1 m`), so the remaining gap is the
   cache lookup itself; `tools/log_to_numbered_tree.py` sidesteps it entirely.

## Sizing a clean `K=8` solver re-run, for comparison

Summing `took` over the `k=8` verdict lines in `out_k8.txt`: **950,852 CPU s = 11.01 days**. That is
2.3x the entire proof-safe `Sa(193)` cold run9 (419,353.1 CPU s, 4.85 days), because this is a
54-cell frontier sweep rather than 16 roots. A clean `K=8` frontier run is therefore *not* the cheap
job it sounds like next to the live `K=9` walk, and per `docs/aws-run.md` a unique cold derivation
stays On-Demand.

Auditing the certificate we already have is the far better buy. Scaling the measured local rate:

| host | vCPU | projected `k=5` wall |
|---|---|---|
| local (M4 Pro, 10 threads) | 10 | 85.4 h |
| `c8a.8xlarge` (the existing launcher default) | 32 | 26.7 h |
| `c8a.24xlarge` | 96 | 8.9 h |
| `c8a.48xlarge` | 192 | 4.4 h |

These are scalings of one local measurement, not measurements on a `c8a`; per-core throughput on
Zen 5 will differ. `tools/cleanroom_ec2_remote.sh` already runs a `--stride 5000` smoke of the
heaviest level before committing to the full audit, which is the right way to calibrate. The
checker's `--stride`/`--offset` sharding also means `k=5` can be split across hosts.

## Reproduction

```
tools/log_to_v1_cert.sh out_k8.txt k8_full.cert 8 <tmpdir>
tools/make_refute_level_certificate.py k8_78.cert --level 8 -o k8-level8.cert
tools/cleanroom/target/release/radio_cleanroom audit --threads 10 k8-level8.cert
tools/build_radio.py -O3 -DMAX_K=8 -DMAX_N=260 -DMAX_STATE_SIZE=1024 \
    -DMAX_TREE_NODES=400000 -DMAX_MEMO=4000000 radio_canon_search_generic.c -o canon8n
tools/sweep_canon_frontier.sh ./canon8n 8 120 3 <outdir> 1 2 3 4 5
tools/canon_out_to_tree.py <outdir>/canon_231_5_at8.out -o witnesses/canon_231_5_at8.tree \
    --command './canon8n 3 8 231 5'
tools/check_witness.py witnesses/*.tree
```

New tools added with this record: `tools/log_to_v1_cert.sh`, `tools/canon_out_to_tree.py`,
`tools/sweep_canon_frontier.sh`.
