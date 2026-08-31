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

These timings show that the main solver is practical for targeted windows and as an independent
validator of candidates.  They do **not** make it the right exhaustive engine for all
9,960,648,265 distance-14 parents.  At the measured 10,000-state window rate, a serial full shell
would still take weeks, whereas the specialized Hall census processes roughly a million states per
second on the same laptop.  Use Hall for exhaustive discovery, then the ordinary solver and the
tight-band extractor as independent checks of every hole it emits.
