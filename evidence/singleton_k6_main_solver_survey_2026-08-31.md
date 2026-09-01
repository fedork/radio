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
search or wall-clock deadline.  Resource exhaustion is an abort, never a verdict.

The first stage used 10,000,000 ranks; subsequent stages use deterministic 3,000,000-rank windows.
Only a successfully completed stage with its exact query count and zero `MAYBE` advances
`NEXT_RANK`; its provenance-bearing exception log is uploaded before the checkpoint.  Thus
interruption loses at most the active stage.  The
durable prefix is
`s3://radio-sa193-393287594714/run10/k6-main-survey/`, and
`tools/singleton_k6_survey_status.sh [--watch]` displays its heartbeat.

The first stage, ranks 0 through 9,999,999, completed and checkpointed at 2026-09-01 00:20:41 UTC:

```text
queries=10000000 solvable=9999999 unsolvable=1 maybe=0 total_ms=1404888
```

The only negative is the known rank-55,096 hole.  The retained exception log passes
`tools/check_provenance.py` and has SHA-256
`6fc5a3d888d40995085eca66d62dd363c00273bc80efd8621c4d0b486e7a6ba3`.  This stage ran with an
8-GiB oracle address-space limit and 10-GiB cgroup ceiling.  Because it approached the former,
subsequent stages run with 16-GiB address space and a 20-GiB cgroup ceiling.  The change was made at
the durable 10,000,000 checkpoint; a just-started second-stage process was replaced, but no
completed stage or retained verdict was lost.  The new process's live limits were read back as
17,179,869,184 and 21,474,836,480 bytes respectively.

The first attempt at ranks 10,000,000 through 19,999,999 exposed much faster child-cache growth
than the first rank band: after roughly two million emitted queries it was already at 6.5 GiB RSS.
It was deliberately stopped without a verdict or checkpoint and restarted from the durable
10,000,000 boundary in 3,000,000-rank stages.  This is checkpoint sizing, not a search cutoff; each
query remains unlimited.

The replacement runner writes an atomic emitted-query counter every 100,000 states and uploads a
heartbeat every minute.  Status now gives exact rank counts to that granularity, stage and overall
percentages, current throughput, stage ETA, full ETA at the current stage's rate, human-readable
RSS, and the VM ceiling.  Its first live heartbeat showed 300,000 / 3,000,000 (10.00%), 4,918
states/s, a 9m09s stage ETA, 0.103% overall and a 23d10h full ETA; the latter is explicitly a
changing-band projection, not a promised completion date.

The progress update is commit `b62616edf382dfad5e24bb74737cd51d70665604`; its source bundle has
SHA-256 `b0c44b8eca35715157afd573e78462c21f1f5339f7c26baf3e50c12be71f0121`.
The AWS ranker build ID is
`f13f247618cc82cc98f2d1755e40fdd45a2c9c1571af7c6606a5f424e4f4bd7d` and the rebuilt oracle ID is
`e8386449693d83ba7a117ee0fc152e6ccae85b25e56f57838fc34c758ffa66d2`.

## Integrated C iteration

The initial AWS runner used two processes: the C++ ranker serialized every 64-row state as text,
and `radio_oracle.c` parsed it before calling the production solver.  A matched cold local window
showed that this was not the speed bottleneck: 100,000 ranks took 17.78 wall / 17.37 user seconds
through the pipe versus 18.00 wall / 17.33 user seconds in one process.  Integration is therefore a
simplicity and observability improvement, not a claimed performance optimization.

`radio_singleton_k6_survey.c` is the replacement.  It is a thin C driver including `radiobase.c`:
ordinary depth-first generation, subtree counts only for checkpoint skipping, `getSbb(n,1)`, then
direct `canSolveB(...,NO_DEADLINE)`.  There is no Hall code, parser, pipe, second process, reordering
or buffered state corpus.  Its progress counter advances after a solver verdict, so the displayed
count is now classified states rather than emitted states.

The regression compares seven `K=3` states and their order against the independent C++ ranker,
checks all seven exact recursive positives, and reproduces the rank-55,096 `K=6` negative.  A local
100,000-state control returns 99,999 positives, exactly that negative and zero `MAYBE`.

The preceding two-process ranks 10,000,000 through 12,999,999 completed with 3,000,000 positives,
zero negatives and zero `MAYBE` in 882.066 solver CPU seconds.  Its provenance-checked S3 log has
SHA-256 `f3650528f14f519753de15b01b5b2ff58eb8454a81fcbe8a72353aad66ce7d63`.
The wrapper then stopped after preserving the checkpoint because its between-stage display tried to
compute a percentage with a zero-length transient stage.  No verdict was lost.  The first
integrated service start separately exposed a 658-ms startup race: under `pipefail`, the status
sampler treated `ps` finding no newly scheduled process yet as fatal.  It ran no query.  Both status
paths are now explicit and race-safe.

The integrated AWS binary is source commit `d295b174984a0a91256622dc39c34c5ae17e1632`, build ID
`5e4d58f646ff4f5b63f4bd8c63acddc21710ac0a685a12aa48f0b449ec4e38d5`; its source bundle SHA-256
is `bf7762d8c96242a812baa0035dce0ba7333d261e28e91965191d42dabec33eed`.  The startup-race-safe
runner is commit `9684bdb`.  A provenance-checked remote smoke test reproduced the known hole before
the service resumed from rank 13,000,000.  Process inspection confirms one `k6-survey` process on
CPU 0 and no ranker or oracle process.

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
