# Production-engine survey of the `K=6` distance-14 shell

Status: **diagnostic survey and engineering benchmark**, not an exhaustive shell result.  The
complete distance-13 theorem still comes from the independent Fixed-Color Hall census.  The purpose
here is to determine whether the ordinary recursive solver can cheaply inspect additional `K=6`
parents without importing Hall search into its trust base.

## Why the parent cache should be disabled

All parents in one exact-support shell have 64 positive singleton rows and mass 729.  If one such
sorted sequence componentwise dominates another, equality of the total masses forces equality in
every coordinate.  Thus the level-6 dominance trie cannot reuse a verdict across two distinct
shell parents.  Its child facts at levels at most five can still be highly reusable.

This distinction matters in practice.  A clean build of the ordinary oracle at commit `9e9e25a`,
with its normal cache policy, exhausted the 30-bit branch-handle space while inserting positive
downward closures after only six top-level `K=6` positives.  The process stopped after 24.6 seconds;
the failure was cache insertion, not solving.  Build ID:
`aa7a9d35bc9437be76e06af6c46a58793d0f2214abba47f2910a00921e4984c9`.
This diagnostic drove local swap use to 22.6 GiB before terminating; no process was left running.

`RADIO_CACHE_DISABLED_LEVEL=6` now bypasses both exact-L1 and dominance-trie lookup/retention at
level six while leaving every lower level unchanged.  This changes only cache hits and allocation,
not the recursive search or its verdict.  The selected level is printed in the provenance banner.
`tools/cache_disabled_level_regression.sh` checks that the selected parent level stays absent while
an enabled child level retains its fact.

## Reproducible ranked windows

`singleton_pair_coloring_census.cpp --transfer-shell-oracle-input` uses the already verified
completion-count DP to emit any contiguous deterministic rank window in the ordinary oracle's
stdin protocol.  The convenience wrapper builds both sides with the exact `K=6` geometry and runs
the oracle under a one-hour/8-GiB cap by default:

```sh
tools/singleton_k6_main_solver_survey.sh DISTANCE SKIP LIMIT [PER_QUERY_BUDGET_SECONDS]
```

For example, the clean committed command

```sh
tools/singleton_k6_main_solver_survey.sh 14 5000000000 10000 1
```

classified exact shell ranks 5,000,000,000 through 5,000,009,999 as follows:

```text
queries=10000 solvable=10000 unsolvable=0 maybe=0 total_ms=1709
```

The full wrapper, including two compilations and the monitor's five-second polling quantum, took
9.34 wall / 4.79 user seconds.  The solver output has build ID
`9e6da219e2d0f7106c3d6ca219247cb094b1cef061c7f14bb122662545a5681c`,
commit `03c56ea285ca21c0978358e478ab7f6487978769`, clean source/worktree flags, and
`define.RADIO_CACHE_DISABLED_LEVEL=6`.

A source-clean build from the same commit and cache mode reproduced the three decisive controls
with an unlimited per-query budget (build ID
`0484e2a6bc3080931e5b00cd7f420091e201922084821440ad5c506b8cb4a369`):

```text
canonical G_6     SOLVABLE     0.0 ms
transfer j=13     SOLVABLE     2.9 ms
padded j=14       UNSOLVABLE   5.0 ms
```

The inverse ranker places the known padded `j=14` hole at zero-based shell rank 55,096.  A local
production-oracle run of ranks 0 through 99,999 returned 99,999 solvable states, that one
unsolvable state, and no `MAYBE` in 16.829 solver CPU seconds.  Extending the same window through
rank 999,999 again found exactly that one hole and no `MAYBE`:

```text
queries=1000000 solvable=999999 unsolvable=1 maybe=0 total_ms=95720
```

The latter took 110.20 wall seconds and peaked at 0.87 GiB RSS.  It was a source-clean but
worktree-dirty diagnostic build at `ad5d8f8`, build ID
`8aca49f9ea71b7dc0040e7c42bb350c6ee288b1e2964da85473e8dbd84744dac`; it is not being promoted as
a durable exhaustive result.  The ranker regression round-trips emitted states through the inverse
rank command, including rank 55,096.

## Full independent AWS census

At Fedor's request, the production solver started an unlimited-budget census of all
9,960,648,265 distance-14 parents at 2026-08-31 23:55:45 UTC.  It shares the on-demand
`r7iz.xlarge` instance `i-0318c3349a0df835b` with the cold `Sa(193)` run, but is pinned to logical
CPU 0 while `Sa(193)` occupies the other physical core.  Each query uses budget zero: there is no
search or wall-clock deadline.  A 10-GiB systemd memory ceiling and an 8-GiB oracle address-space
ceiling turn resource exhaustion into an abort, never a verdict.

The run is divided into deterministic 10,000,000-rank stages.  Only a successfully completed stage
with its exact query count and zero `MAYBE` advances `NEXT_RANK`; its provenance-bearing exception
log is uploaded before the checkpoint.  Thus interruption loses at most the active stage.  The
durable prefix is
`s3://radio-sa193-393287594714/run10/k6-main-survey/`, and
`tools/singleton_k6_survey_status.sh [--watch]` displays its heartbeat.  The first stage includes
the known hole, so it also acts as an immediate negative control.

The source bundle is commit `30021dd1eb30145264651e0bc374f6270b7ac07b`, SHA-256
`eccc0d00ca3cbd0d0c10226ce4cf279242b283e796d5813003ded533f7d292c5`.  The AWS ranker build ID is
`90a97f3ec63f534b851de5ca71d5c1ac3bbcb329b9246eec8a3fe7194d4be09a`; the production oracle build
ID is `b902d45bac2c6e1e2af53ad4d934b0bc5f162269db093cca542e865dfd8d540f`.  The first build-only
attempt used AWS Clang 15, which rejects an existing C++20 structured-binding lambda capture; it
failed before any search process existed.  Rebuilding only the standalone C++ ranker with GCC
11.5 succeeded, while the C oracle stayed on Clang.

The host's original Sa cloud-init script powers off after Sa ends.  A narrow guard now defers that
one shutdown while the census service is active, and a separate systemd monitor invokes the real
shutdown only after both intended jobs have ended.  This prevents sharing the instance from
silently imposing Sa's shorter lifetime on the census while still avoiding an indefinitely idle
host.

These timings make the production solver a practical, independent exhaustive check, although not
the fastest discovery engine: the specialized Hall census remains hundreds of times faster.  Keep
the two results separate because their implementations and proof burdens are intentionally
different.
