# Main-solver correction after the singleton converse refutation (2026-08-31)

## Result

The production engine does not treat arbitrary `G_K`-majorized singleton states as positive.  Its
current boundary is:

- weak majorization is a necessary rejection at every `K`;
- a distinct-slot embedding in `G_K` remains a positive terminal at every `K`;
- every other majorized singleton goes through ordinary exact recursion, including at `K<=5`
  where sufficiency is a separately proved theorem rather than a production certificate.

No Fixed-Color Hall implementation is part of `radiobase.c`.  Hall remains an independent research
solver and proof check; this avoids adding a second theorem-sensitive search to the production
trust base.

## Why the first exact run appeared slow

The mass-697 core has total vertex count 729 but largest component side-sum only 65.  The historical
single `MAX_N` bound sized both universes.  A first `MAX_N=733` build, before the bounds were
separated, was manually interrupted after more than 100 seconds in `init()` without reaching the
query.

Current code uses `MAX_N` for total state size and `MAX_PART_N` for the component catalog.  With

```sh
-DMAX_K=6 -DMAX_N=733 -DMAX_PART_N=65
```

`RADIO_INIT_PROFILE` measured 0.015 CPU seconds for `init()`.  `RADIO_SPLIT_PROFILE` measured every
distinct top-level split-table and FAST initialization below its 0.001-second display resolution.

A second diagnostic disabled the then-experimental Hall precheck and left the historical cache
insertion unbounded.  A one-second macOS `sample` taken on the live process placed all 848 samples
under `cacheCantSolve`; the stack was already below the completed `canSolveB_ctx` search.  The
process was still there at the last retained inventory after 12:52 CPU, and was subsequently
interrupted.  The 32-row query had soundly contracted to the 30-row state before caching.  Thus
neither exact search nor split initialization caused the apparent hang: it was the
permutation-expanded monotone cache closure of that 30-part negative.

`RADIO_CACHE_PROFILE` on the temporary cutoff build first separated the costs exactly:

```text
RADIO_CACHE_PROFILE phase=search-end k=6 size=30 singleton=1 verdict=0 seconds=0.004
RADIO_CACHE_PROFILE phase=dominance-end k=6 query_size=30 cache_size=30 verdict=0 nodes=1000000 truncated=1 seconds=0.012
```

The ordinary engine examined 24,795 top-level split combinations and used 36,560 deterministic
work units before returning `UNSOLVABLE`.  This is why the production Hall precheck was removed:
once cache bookkeeping is finite, it provides no material speed benefit on the decisive state.

## Exact majorization-bounded dominance insertion

The arbitrary cutoff was a sound emergency fix but is no longer the default. Negative upward
closure now tracks the star expansion of the chosen harder components together with every
untouched original component. This is the easiest possible completion of that cache prefix. If it
violates a `G_k` prefix, every coordinatewise-harder completion violates Singleton Majorization
Necessity too, so the whole branch is irrelevant to production lookup and is skipped.

That bound alone was insufficient: an unbounded diagnostic with the new prefix test still hit its
60-CPU-second cap. The recursion was separately choosing each occurrence of an equal component,
although those choices have the same cache edge and remaining multiset. Choosing one canonical
representative per equal value removes this duplicate factorial expansion. With both corrections,
the unbounded mass-683 negative insertion completes in 30 recursive nodes, rejects 203 branches at
the majorization boundary, and takes below the profiler's 0.001-second resolution. The unbounded
positive `j=13` closure also completes: 2,602,239 nodes in 0.054 CPU seconds.

`RADIO_CACHE_INSERT_NODE_LIMIT` therefore defaults to zero. A nonzero value remains available for
controlled diagnostics; if it truncates, every retained consequence is still sound and
`cache=partial:N/N` remains an explicit annotation.

The production regression checks both directions without truncation: the feasible 32-part `j=13`
fact and infeasible 30-part mass-683 core remain exact trie hits. The older mass-697 and padded forms
are rejected as upward consequences of the smaller negative. A separate exhaustive `K=3` regression
compares the cache against an independent coordinatewise perfect-matching definition for 255
normalized non-unit seeds and all 65,025 in-bound seed/query pairs; every relation agrees.

## Cache trust epoch

Older positive cache facts are unsafe even when the queried state is not singleton: an ancestor
may have been accepted because a recursive singleton leaf used either the false unrestricted
converse or the now-retired production shortcut for the separately proved `K<=5` converse.
Current solver output emits, and oracle journals begin with,

```text
# radio-cache-semantics=singleton-majorization-necessity-only-v1
```

`parse_file`, `radio_oracle.c`, and the Pareto census's exact loader replay positives only after
that marker.  Unmarked or older-epoch input is negative-only.  Snapshot v4 begins the same semantic
epoch and rejects v1--v3 snapshots even under `restore-any`; its geometry includes `MAX_PART_N`.

## Reproduction and checks

The main regression is:

```sh
tools/singleton_main_solver_regression.sh
```

Its optimized build reports:

```text
SINGLETON_MAIN_SOLVER_REGRESSION verdict=FALSE work=36560 cache_nodes=30 cache_majorization_prunes=203 low_k_exact=YES ignored_untrusted_positive=2
```

It checks that a nonembedded majorized `K=2` state consumes exact-recursion work, canonical `G_6`,
the feasible `j=13` transfer state, the exact mass-683 negative and its
mass-697/full-padded upward consequences, rejection of an unmarked singleton positive, rejection of an unmarked tainted
nonsingleton ancestor, and exact unbounded positive/negative cache insertion.

The independent cache-closure regression is:

```sh
tools/cache_upward_closure_regression.sh
```

It reports `seeds=255 queries=65025 result=PASS`.

The separate cache-epoch regression is:

```sh
tools/cache_semantics_regression.sh
```

Additional passing checks on the working tree were `tools/test_radio_refute.sh`,
`tools/work_budget_regression.sh`, `tools/search_context_regression.sh`,
`tools/provenance_regression.sh`, and `tools/test_pareto_prefix_census.sh`.  An ASan+UBSan build of
the main and normalized cache-closure regressions (Apple leak detection disabled because that
platform does not support it) completed with no sanitizer finding. A small oracle v4 snapshot
round trip restored identical cache structure statistics.
