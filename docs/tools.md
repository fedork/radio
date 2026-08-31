# The solver and its drivers

## Layout

`radiobase.c` is the entire engine. Every other `.c` at the repo root is a `main()` wrapped around
it, selected by `#include "radiobase.c"`. There is no build system; each driver is a single compiler
invocation made through the provenance builder.

```
tools/build_radio.py -O3 -DMAX_K=<k> -DMAX_N=<total-n> \
    -DMAX_PART_N=<largest-component-n> <driver>.c -o <driver>
```

`MAX_K`, `MAX_N` and `MAX_PART_N` size static tables at compile time, so **they must be set for the
problem you are running**. `MAX_N` is the largest sum of all component side-sums in a state;
`MAX_PART_N` is the largest `n1+n2` of one component and sizes the quadratic component catalog.
It defaults to `MAX_N` for compatibility. Set it explicitly for long states made of small
components: the mass-683 singleton core needs `MAX_N=713` but only `MAX_PART_N=65`.

## Build and run provenance

Every `radiobase.c` process writes a `# radio-provenance-v1` block to stdout before allocating
tables or reading a cache. It contains the Git commit, source-dependency hashes, exact compiler
argument vector, compiler version, deterministic build ID, exact run argument vector, UTC time,
working directory, OS/kernel/architecture, CPU, physical RAM, locale and declared resource caps.
Arguments are stored as indexed fields, so spaces and escaping never erase their boundaries. The
runtime environment is deliberately an allow-list; credentials and unrelated variables are never
dumped.

`tools/build_radio.py` also writes `<binary>.provenance`, including the final binary SHA-256. Direct
`clang` builds remain possible for archaeology, but the engine marks them
`provenance_complete=no`; strict validation rejects them:

```
tools/check_provenance.py out.txt binary.provenance
```

Standalone programs such as `radio_verify`, which do not include the engine, retain the same raw
output contract when launched as
`tools/run_with_provenance.py ./radio_verify ...`. `parse_out.sh` preserves all comment metadata in
checkpoints, and `parse_file` ignores and re-echoes it while replaying facts. `tools/artifacts.sh push` refuses a
new solver log which lacks a complete block; `RADIO_ALLOW_LEGACY_PROVENANCE=1` exists only for a
pre-banner historical file whose limitation is explicitly recorded in `docs/data.md`.

## Independent negative verifier

`radio_verify.c` shares no search code with `radiobase.c`. Build it with pthread support:

```
tools/build_radio.py -O3 -pthread radio_verify.c -o radio_verify
VERIFY_THREADS=8 tools/run_with_provenance.py ./radio_verify LOG [MAX_K]
```

Ordinary verification puts eligible facts from every `k` in one dynamic worker queue. A fact at
`k` reads only the frozen fact index at `k-1`, so runtime order is irrelevant: acceptance still
means that every local obligation passed, and well-founded induction supplies the proof. Each
worker owns its recursion state, memo and lazy live/pair tables. The default per-worker memo shrinks
with worker count so aggregate direct-memo capacity stays near the single-thread `2^24` entries;
override it with `VERIFY_MEMO_BITS` only for a measured comparison.

Parent parts are enumerated by descending segment mass, then descending long side. This is only a
Cartesian-product traversal order; it does not omit assignments. It reduced a twenty-root spread
sample of the dominant run9 k=7 four-part level from 41,945,991 to 5,336,038 recursion nodes. Pass
`group_order=0` after `MAX_K` for the former canonical-long-side control; modes 1 and 2 retain the
mass-ascending and fewest-options controls.

The default static dominance index keeps the canonical fact array unchanged and builds a separate
read-only `(part count,largest segment product,total mass)` permutation. Its hot structure-of-arrays
columns contain total mass plus packed sorted n, m and top-four segment-product profiles. Those are
sound necessary componentwise conditions; only the survivors touch the full `Fact` and run the
exact injection matcher.

Large levels now add an immutable kd hierarchy over the 21 necessary profile coordinates. Each
node stores componentwise minima; a failed minimum skips the whole subtree, while a fitting leaf
still falls through to the unchanged packed filters and exact injection matcher. The tree is also
used for same-level redundancy checks, excluding the query itself. Levels below 65,536 facts keep
the product-only loop, avoiding a regression on the smaller level that dominates Sa(113). On the
retained hard run9 k=7 root, the tree reduced the block build's 5.509 billion individual fact
probes to 431.317 million while preserving the exact 4,644,469-node proof and memo counts; verifier
wall fell from 11.70 to 4.20 seconds before wider pair pruning. A 32-fact leaf build adds 26.9 MiB
and takes about 4.3 seconds once per full run9 load.

Pairwise forward checking now covers option lists through 512 entries. Its rows are worker-local
and capped at 128 MiB per worker; a refused allocation only disables that optimization. The five-
root control fell from 9,158,686 nodes / 21.00 seconds under 256-fact blocks to 4,690,828 nodes /
5.34 seconds under the kd tree plus wider pairs. `VERIFY_LEGACY_INDEX` restores the oldest
`(part count,largest n,total mass)` scan. Explicit product-profile/sort macros or
`VERIFY_NO_BLOCK_PARETO` give the product-only control; `VERIFY_BLOCK_PARETO` selects the former
fixed-block hierarchy; `VERIFY_INDEX_STATS` reports the selected hierarchy and per-fact filters.
Design, failed layouts, tuning and controls are in
[`../evidence/verifier_product_index_2026-08-17.txt`](../evidence/verifier_product_index_2026-08-17.txt)
and
[`../evidence/verifier_block_pareto_2026-08-17.txt`](../evidence/verifier_block_pareto_2026-08-17.txt),
with the production hierarchy in
[`../evidence/verifier_kd_index_2026-08-18.txt`](../evidence/verifier_kd_index_2026-08-18.txt).

With the mass-descending part order, the full 120,302-record Sa(113) colored replay reported
330,226,371 nodes / 25.10 seconds on twelve workers. **Do not treat that colored replay as proof.**
On 2026-08-18 the solver-core refuter below exposed nine uncovered splits in the same bundle, while
the complete 304,105-record normalized input closed with zero gaps. The independent checker's
coloring/replay discrepancy remains to be diagnosed; its benchmark remains useful as a performance
measurement, not as evidence that the pruned support set is closed.

The durable input/output format is text, and is deliberately simpler than a raw log:

```
radio-negative-certificate-v1
# comments are allowed
meta source-sha256 ba63...
root 9 Sb(112:81)
fact 8 Sb(53:52,44:44)
```

`root` records are claims to prove; `fact` records are their support database. Parts are
canonicalized and mass is recomputed, never trusted from an annotation. Unknown records are fatal
after the header. `CERT_ONLY=1 CERT_OUT=FILE` normalizes a raw log without verifying it. The current
in-memory representation still limits each coordinate to 255; that is an implementation limit,
not part of the text grammar.

## Frozen solver-core refuter

`radio_refute.c` is the fast validation path when sharing the production solver core is acceptable:

```
tools/make_refute_level_certificate.py complete.cert --level 7 -o level-7.cert
tools/build_radio.py -O3 -pthread -DMAX_K=10 -DMAX_N=194 \
  radio_refute.c -o radio_refute
REFUTE_THREADS=16 REFUTE_PROGRESS_SECONDS=60 \
  tools/run_with_provenance.py ./radio_refute level-7.cert
```

The preferred `radio-negative-level-certificate-v2` input is one text file per audited level, with
sections in load order: a certificate-local `part ID -> n:m` dictionary, complete k-1 support,
checked split-part hints, then level-k claims. Only support enters the production compact dominance
trie; compact offset/length claims remain worker targets. The converter is bounded-memory and
deterministic. Legacy full v1 certificates remain accepted for regression.

The split hints list every nonunit root part with its occurrence count and determine serial
preparation order. The verifier checks the list exactly against the claims, then independently
derives every local split and theorem filter; hints cannot omit work. It asks isolated local
outcomes directly at k-1 instead of inheriting the mutable solver's multi-level metadata walk, so
the only result cache loaded for a level-k phase is k-1. It then freezes cache and split structures.
Each worker owns a `radio_search_context`. An audited root bypasses its own level-k cache entry and
enumerates the solver's exhaustive pass; children are theorem/CACHE_ONLY k-1 queries. A cache miss
is an uncovered split, never permission to recurse, learn or write a fact. Cache allocation counts
and a full split-table checksum must be identical after the worker epoch.

Full-star majorization checks only the endpoint of each equal-value star run. The prefix-difference
increments are non-decreasing because `G_k` is sorted, so this is exactly equivalent to testing
every expanded prefix; `tools/star_majorization_regression.c` locks the equivalence. The frozen
verifier compiles the worker L1 out by default after a matched k7 A/B measured an 11.3% CPU
regression. `-DRADIO_REFUTE_ENABLE_L1` restores that rejected control; ordinary mutable solver
builds retain L1.

`REFUTE_MIN_K`/`MAX_K`, `REFUTE_MIN_PARTS`/`MAX_PARTS`, and
`REFUTE_STRIDE`/`OFFSET` select a deterministic batch. Runtime order needs no level barrier because
every dependency decreases k. Progress reports cache loading, serial freeze work, completed and
per-level targets, accepted-prefix throughput, ETA, and the three oldest active roots. This tool is
not an independent proof implementation; it is deliberately the solver's read-only, refute-only
baseline. The completed old full-v1 replay is in
[`../evidence/verifier_frozen_trie_2026-08-18.txt`](../evidence/verifier_frozen_trie_2026-08-18.txt);
the level format, majorization proof and k7 controls are in
[`../evidence/verifier_level_v2_2026-08-18.txt`](../evidence/verifier_level_v2_2026-08-18.txt).
`tools/test_radio_refute.sh` covers legacy and level inputs, serial/parallel agreement, corrupt
split hints and dictionary order, endpoint-majorization equivalence, the L1 control, and a
fail-closed false claim.

The same refuter performs top-down citation coloring behind a compile flag:

```
tools/build_radio.py -O3 -pthread -DMAX_K=10 -DMAX_N=194 \
  -DRADIO_REFUTE_ENABLE_COLORING radio_refute.c -o radio_refute_color
REFUTE_THREADS=16 tools/run_with_provenance.py \
  ./radio_refute_color level-9.cert selected-level-8.txt
tools/make_refute_level_certificate.py complete.cert --level 8 \
  --selection selected-level-8.txt -o level-8.cert
```

Every retained negative Pareto point carries its original support-record index. Split preparation
and each worker mark private dense bitsets, avoiding shared writes on billions of cache hits; the
driver ORs them after a zero-gap epoch. The readable selection includes ordered source indices and
`Sb(...)` text. The converter requires `audited == source-claims`, checks each index and state
against the complete v1 level, keeps complete next support and regenerates dictionary/split hints.
`used 0` ends the chain. A coloring binary accepts only level-v2 input, refuses an existing output
path and emits no valid selection after a gap. Eager split preparation is conservatively included,
so the result is sound reachability compression, not a claim of minimum support. The test script
also locks byte-identical one/two-worker selection, a nonempty two-level handoff, terminal zero and
tamper/overwrite rejection. Design, sanitizer controls and the dominant k7 A/B are in
[`../evidence/verifier_coloring_citations_2026-08-18.txt`](../evidence/verifier_coloring_citations_2026-08-18.txt).

The older independent checker also has top-down coloring, but its Sa(113) output has nine known
gaps under the solver-core refuter and must not be used as proof. Its historical invocation was:

```
TOPDOWN=9 ROOTS=roots.cert MINIMIZE_BEFORE_COLOR=1 CERT_OUT=colored.cert \
  VERIFY_THREADS=8 tools/run_with_provenance.py ./radio_verify LOG 9
```

`MINIMIZE_BEFORE_COLOR` first replaces each support level below the roots by its same-level minimal
antichain under Subgraph Monotonicity. `ROOTS` may be either a text certificate or raw solver lines;
all records in that file are treated as roots. `ROOTS` is additive: outside top-down coloring it
does not stop facts in the main input from also becoming verification targets, so a one-root
microbenchmark must filter the main-input source explicitly. Without explicit roots, every fact at
`TOPDOWN` is a root and therefore cannot be colored away. Worker-local caches are discarded between coloring
passes, preserving the citation accounting that shared cached live tables previously suppressed.
The checker refuses `CERT_OUT` after a partial or filtered verification; use `CERT_ONLY` when the
intent is normalization rather than a proof replay.

`tools/test_radio_verify.sh` covers serial/parallel agreement, text parsing, minimalization,
deterministic coloring and replay, progress-option validation, plus a forced-small-block antichain
comparison. The first corpus
measurement used
[`fullsolve-2026`](https://github.com/fedork/radio-data/releases/tag/fullsolve-2026): all 62,366
facts and 97,483,464 recursion nodes were identical from one through sixteen workers; measured
wall scaling and certificate sizes are retained in
[`../evidence/radio_verify_parallel_2026-08-16.txt`](../evidence/radio_verify_parallel_2026-08-16.txt).

`tools/benchmark_verifier_pipeline.sh` exercises the complete raw-prefix -> normalized text ->
byte-identical round-trip -> minimalized/colored certificate -> independent replay path. It records
per-stage GNU Time wall/CPU/RSS data, exact hashes and provenance, and refuses completion on a
missing root, changed round-trip, unresolved fact or count mismatch. The 2026-08-17 AWS benchmark
used it for 21 Sa(66) runs and a full Sa(113) pipeline. On the representative Sa(113) certificate,
8/14/16 workers replayed the identical 2,491,817,467-node proof in 434.86/369.57/347.91 seconds:
sixteen minimizes wall, eight minimizes CPU, and fourteen is a reasonable shared-host compromise.
The full record is
[`../evidence/verifier_pipeline_benchmark_2026-08-17.txt`](../evidence/verifier_pipeline_benchmark_2026-08-17.txt)
and its durable bundle is the private
[`verifier-pipeline-2026-08-17`](https://github.com/fedork/radio-data/releases/tag/verifier-pipeline-2026-08-17)
release.

The parallel prototype deliberately rejects `TIMECAP`, diagnostic split printing and the disabled
subtree-DP experiment. Their current process-global reporting/bulk allocation is not safe or useful
to duplicate per worker.

Long batches can expose a live, completed-work cursor with `VERIFY_PROGRESS_SECONDS=N`. Each
interval reports completed/claimed/queued targets, verified/unresolved counts, cumulative/window/
EWMA throughput, cumulative recursion nodes, progress by `k` and part count, and the three oldest
active facts with their age and a coarse recursion-node cursor. `eta_total_s`, `eta_window_s` and
`eta_ewma_s` are explicit throughput projections, not deadlines: canonical task order changes the
cost mix, so use the estimates together with `PROGRESS_PARTS` and active ages. `eta_window_s` reacts
immediately to a new cost region but is noisy; total and EWMA projections retain earlier history.
`BATCH_START` and `BATCH_DONE`
remain exact machine-readable boundaries even when periodic reporting is disabled. The reporter
adds no atomic operation to dominance scans; an active worker publishes its node cursor only every
2^20 recursion nodes and all other counters update once per completed target.

The original detached full-run9 AWS pipeline is staged by `tools/run9_verifier_aws_remote.sh`. It validates
the raw log and source archive hashes, normalizes and byte-round-trips the text input, minimalizes
and colors it, replays the colored bundle, then compresses and uploads the result. Its current
one-shot progress command is:

```
tools/run9_verifier_status.sh
```

That status command uses one bounded SSM request and exits; it does not leave a local watcher. Its
leading `PROGRESS` block distinguishes the four pipeline steps and separates process health from
proof progress. The frozen binary on the shared host predates live counters and still reports only
completed levels.

New dedicated runs use `tools/run9_verifier_ec2_launch.sh`. It refuses a second active run, archives
the exact clean commit, launches an on-demand 16-vCPU `c8a.4xlarge`, runs the regression, normalizes
and byte-round-trips run9, then audits the existing certificate without coloring. A deterministic
9,995-fact k=7 four-part sample must project below seven days before the expensive phase starts.
The full audit retains k=7, k<=6 and k=8..9 as separate checkpoints; every phase has a 24-GiB RSS
guard, k=7 has a seven-day wall cap, and the host has an independent nine-day shutdown. `STATUS`
and a bounded `PROGRESS` tail reach S3 every minute. Spot is deliberately not used because the
multi-day k=7 phase has no intra-level restart cursor. Follow the latest run once, or continuously,
with:

```
tools/run9_verifier_progress_status.sh
tools/run9_verifier_progress_status.sh --watch
```

Current level-v2 solver-core deployments use different, one-shot helpers:

```
tools/run9_refute_ec2_launch.sh                 # complete uncolored per-level replay
tools/run9_refute_status.sh RUN_ID
tools/run9_color_refute_ec2_launch.sh           # top-down citation-colored replay
tools/run9_color_refute_status.sh RUN_ID
```

Both launchers require a clean tracked source/pipeline set, archive the exact commit, run the
regression and a same-host k7 gate, use sixteen workers on a dedicated on-demand `c8a.4xlarge`, and
upload live status every minute. The uncolored runner checkpoints each independent level. The
colored runner begins with all k9 claims, validates and checkpoints each selection, generates the
next filtered claims with complete lower support, and stops only at `used 0`. There is no
within-level restart cursor, so the multi-hour k7 phase is intentionally not Spot.

## Engine internals worth knowing

| what | where |
|---|---|
| `(n1:n2)` pair -> `sbb` integer id; level-keyed split tables built lazily | `init`, `ensure_splits` |
| Four stored split orderings (`BY_SP0/1/2`, `BY_MAGIC3`); the `_DESC` three are derived by reversed subscript | `ensure_splits`, `ORDER_BASE` |
| Result cache: exact-prefix trie with maximal-positive/minimal-negative Pareto fronts in its last part | `cacheCanSolve`, `cacheCantSolve`, `checkCache` |
| Main search: tri-state `TRUE`/`FALSE`/`MAYBE`, FAST/exhaustive passes, deterministic accepted-prefix budget, shared short-state allowance and geometric long-state probes | `canSolveB_ctx`; `canSolveB` is its default-context wrapper |
| Joint suffix reachability; suppression of prefix contraction once it rejects | `rb_dead`, `rb_tainted_contraction` |
| Unit-group stripping before search | start of `canSolveB` |
| Singleton necessary-prefix test, unconditional distinct-slot shortcut, and full star-expansion obstruction; every other majorized singleton uses exact recursion, including at `K<=5` | `singleton_majorization_holds`, `singleton_embedded_can_solve`, `star_expansion_majorization_can_solve` |
| `Sa` recursion | `canSolveA` |
| Enumerate *all* top-level splits plus a solvability matrix | `all_solutions` |
| Warm the cache from a previous run's parsed output | `parse_file` |

### Search contexts and parallel boundary

`radio_search_context` now owns the mutable state that naturally belongs to one recursive worker:
the accepted-prefix clock, exact L1, joint-suffix reachability allocation/counters, and negative
verdict count. Call `radio_search_context_init`, use `canSolveB_ctx`, then call
`radio_search_context_destroy`; ordinary drivers continue to use `canSolveB` and reproduce the
prior serial behavior. `tools/search_context_regression.sh` checks independent contexts under both
the deterministic work scheduler and historical CPU scheduler. `tools/split_regression.c` remains
the exact old/new verdict gate.

This API does **not** yet authorize concurrent recursive solver calls. The dominance trie/arenas,
lazy split catalog and its learned `s[4]`/`s[5]`/`FAST` fields, and `sbb_to_min_k` remain
process-global mutable state. They must become an immutable batch epoch plus worker-local overlay
before pthread workers are safe. The ownership map, publication protocol and limited-width queue
design are in [parallel-solver.md](parallel-solver.md).

### Split-table allocation

Split tables are keyed by `(parent k, sbb)`, not merely by `sbb`.  A table is either absent or
fully built; there is no cut-level/chunk-level lazy machinery.  `canSolveB` initially builds only
the first part's table and materialises a suffix table when depth-first search reaches that part.
The joint reachability accelerator is the one bulk path: when it arms, it needs every suffix and
builds the missing whole tables then.

Reachability depends on that suffix, whereas implicit size contraction claims the shorter prefix is
itself impossible. The two inferences cannot be combined: after any actual `rb_dead` rejection, a
full negative is still exact but contraction is suppressed and the verdict carries
`contraction=rb-suppressed:<candidate-size>`. `tools/rb_contraction_regression.sh` forces the path
on a state whose candidate prefix is known positive; do not remove that test or weaken the guard to
the final pass only, because iterative-deepening counters reset between passes.

Each table is one exact-sized contiguous allocation containing its cut records, four stored order
indices, and three cumulative counting arrays.  Before sizing it, `ensure_splits` removes a cut if
one of that part's three child substates, by itself, exceeds `3^(k-1)` or violates full-star
majorization at `k-1`.  This is an exhaustive-search-safe filter: the child is a subgraph of the
corresponding complete child state, and adding the other parent parts cannot make an impossible
subgraph solvable.  Unit groups are counted and then eliminated exactly as at the start of
`canSolveB`; cache verdicts and conjectural dominance rules are never used here.

Compile any driver with `-DSPLIT_STATS` to print requested bytes, geometric candidates and retained
options by level at exit.  `tools/split_regression.c` is the small deterministic old/new verdict
gate, and `tools/split_memory_probe.c` isolates persistent split-allocation requests for one warm
query; their headers contain the comparison commands.  These are regression/measurement drivers
for the C solver, not independent evidence for a mathematical verdict.

### Result-cache allocation

Each prefix-trie edge is a tagged `uint32_t` descriptor.  A front-only node with at most one point
of each sign is encoded inline; larger fronts use a small vector or front record.  A node with a
longer suffix owns a dense `uint32_t` branch array whose slots 0 and 1 store its positive and
negative front descriptors.  Positive fronts are consulted only for the query's final part;
negative fronts refute any extension of the matching prefix.

This remains an unbounded cache, but it no longer materialises last-part dominance as child nodes.
On the retained `MAX_N=193` checkpoint, k=5..7 requested storage fell from 5.59 GB to 0.50 GB.  The
initial compact version paid an 11.6% control-CPU premium.  Current `main` removes that premium with
a 65,536-entry (2 MiB at `MAX_N=193`) exact-state L1.  It stores no `MAYBE`, verifies the full
normalized state after its fingerprint, and bypasses states longer than 12 parts.  The probe sits
before repeated singleton/full-star majorization; misses still follow those theorem checks and the
same dominance trie.  `-DMEASURE_CACHE_L1` prints aggregate query/hit/store counts at exit and
`-DCACHE_L1_BITS=n` overrides the default size for controlled measurements.

On the fixed warm four-part control, the final build took 26.6 solve seconds versus 42.6 for the
first compact build and 33.0 before compaction, with the identical witness and 37,899 top-level
splits.  The full `Sa(192)` gate returned SOLVABLE in 711.7 CPU seconds and 0.35 GB peak RSS, versus
819.9 seconds for the first compact build and 734.5 seconds before compaction.  Exact layout,
query-regression and throughput measurements are in
[`../evidence/cache_last_front_2026-08-10.txt`](../evidence/cache_last_front_2026-08-10.txt).

`tools/cache_query_regression.c` verifies every exact fact in a parsed cache, then writes one byte
per targeted mutation and deterministic random query.  Compile it against two engine versions and
compare the complete byte files; do not replace this with a hash when changing cache semantics.

### Memory: `radio_canon_search_generic` will eat your machine

`TreeNode` embeds `int sb[5120]` and `int split_m[10240]`, so **one node is 61,488 bytes**. The
pool allocates 20480-node chunks — **1.26 GB each** — and `pool_used = snap` only rewinds the
counter, it never frees a chunk. With `MAX_TREE_NODES` at 4,000,000 the ceiling is ~246 GB, plus
a 1.54 GB static memo array.

A search that finds its tree quickly is cheap. One that has to *exhaust* — proving no tree exists,
or grinding through top-level splits after the first success — grows without bound. Three
concurrent runs on `473:6@9` reached 28 GB, 21 GB and 12 GB resident and drove the machine into
heavy swap.

**Always cap it.** Run under a memory limit and a wall-clock limit, one at a time:

```
tools/build_radio.py -O3 -DMAX_K=9 -DMAX_N=485 -DMAX_STATE_SIZE=1024 \
    radio_canon_search_generic.c -o canon
( ulimit -v 6000000; ulimit -t 900; ./canon 3 9 473 6 )
```

`MAX_STATE_SIZE`, `MAX_TREE_NODES` and `MAX_MEMO` are now `-D`-overridable. A state at depth
`d` has at most `2^(k-d)` parts, so 1024 is generous for `k <= 9` and cuts a node from 61 KB to
12 KB. Note `timeout` is not installed here; `ulimit -t` is the portable CPU-seconds bound.

### Capping the other drivers

Those compile-time bounds exist only in `radio_canon_search_generic`. `radio`, `radio_one`,
`radio_full` and the Pareto walkers still grow an unbounded (now compact) result-cache trie — the
pre-compaction 2023 `Sa(193)` run reached ~90 GB — and `ulimit -t` bounds CPU, not memory. For those:

```
tools/capped_run.sh --seconds 3600 --rss-gb 16 --label sa113 -- ./radio_k9 > out.txt
```

Enforces a wall-clock cap and an RSS cap, reports peak RSS and wall time on stderr (so it
survives redirecting the driver's stdout), and exits 124 on timeout / 137 on memory kill.
Wall time is quantised to `--poll` (default 5s); the drivers' own `took N` lines are exact.

On macOS this is only a **resident-set** guard, not a bound on the solver's allocated heap.
Swapped anonymous pages leave RSS: the pre-compaction 2026-08-10 local `Sa(193)` trial showed 2.77 GB peak RSS
while `vmmap -summary` measured a 7.1 GB physical footprint, 5.9 GB of it swapped.  macOS `top`'s
documented `MEM` field reports that physical footprint cheaply enough for a live guard; also retain
`vm_stat` for swap context.  `vmmap` remains useful for one-off attribution but later hung for 19
minutes on the active solver, so do not call it without a separate bound.  Linux/AWS did not show
the RSS gap because the solver heap remained resident there.

For the deliberately unbounded-time local `Sa(193)` run, use
`tools/sa193_local_supervisor.sh RUN_DIR [FOOTPRINT_GIB] [SAME_RUN_CHECKPOINT]`.  `RUN_DIR` must
contain a newly compiled `radio_sa193`.  With no third argument the run is cold; with one it is a
same-run continuation, and every generated checkpoint folds the inherited facts together with the
new segment so a later restart cannot forget its prefix.  The supervisor keeps the machine awake,
samples `top MEM` every two minutes under a 20-second probe bound, regenerates a checkpoint hourly,
and refuses to overwrite an existing attempt.  It has no time limit.  It stops on the requested
footprint ceiling, less than 10 GiB free disk, or five consecutive failures of the physical-memory
measurement.  Inspect `status.txt`, `monitor.log`, and the PID files in `RUN_DIR`.  Never edit the
script while its bash process is live; stop the supervisor first, because bash re-reads scripts.

If that rule has already been violated, do not restart or perturb the solver merely to repair the
launcher. `tools/sa193_recovery_guard.sh RUN_DIR SOLVER_PID SUPERVISOR_PID SOURCE_CHECKPOINT
FOOTPRINT_GIB` is a non-owning fallback: it remains passive while the primary lives, assumes the
same footprint cap if it disappears, and writes a separate closed recovery checkpoint after solver
exit. Copy it and `parse_out.sh` into the run directory before starting it, and point
`RADIO_PARSE_OUT` at that copied parser, so later repository edits cannot repeat the same failure.
It is recovery machinery, not a second normal supervisor.

In an ordinary terminal the supervisor may be put under `nohup`.  A managed one-shot command may
reap its whole descendant process group after returning despite `nohup`; in that environment keep
the supervisor in the foreground of a persistent execution session.  An empty log and the absence
of `completion.txt` distinguish that launcher failure from a solver exit.

### The canonical search does not always work, even unrestricted

It is a *hypothesis* about solution shape, and the hypothesis fails at small k. `Sb(46:6)` in 6
is a proven frontier point with four working splits from `radio_full`, yet
`radio_canon_search_generic` returns `NO_CANONICAL_TREE` for it at `target_k` 3 and 2, with no
restriction applied. Same for `Sb(104:6)` in 7 at `target_k=3`.

So small-k states are **not** valid cheap proxies for testing structural hypotheses with this
tool: a negative there says the state has no atomic-leaf solution, not that it has no solution.
Use `radio_full` when the question is about solutions in general.

and check for strays with `pgrep -f radio_canon` before starting another.

Two behaviours to be aware of:

- **`MAYBE` is a real answer.** Finite search budgets cause `canSolveB` to give up and return `MAYBE`
  rather than `FALSE`. A `can't solve` line in the output is a genuine negative; the absence
  of a line is not. A finite state may now bail without adding a cache fact, but it never gives an
  unresolved descendant `NO_DEADLINE` or refills an expired parent. One- and two-segment states
  retain the shared parent allowance; states with three or more segments give each speculative
  child an initial 40,000,000-unit slice (two nominal seconds). An unresolved exhaustive pass
  doubles that local slice, so an unbounded root still deepens monotonically without persisting a
  preferred split. The repository default charges one unit at each accepted split prefix and checks
  every charge; 20,000,000 units are one nominal second. The counter is absolute and process-global,
  so recursive children consume the parent's allowance. `-DRADIO_CPU_BUDGET` restores the historical
  process-CPU clock and its every-65,536-prefix poll for controlled comparisons. The state-machine
  failure and work-clock calibration are recorded in
  [`../evidence/deadline_stall_2026-08-10.txt`](../evidence/deadline_stall_2026-08-10.txt) and
  [`../evidence/work_budget_rb_root_2026-08-13.txt`](../evidence/work_budget_rb_root_2026-08-13.txt).
- **The search is not unconditionally byte-deterministic.** The FAST pass was briefly removed on 2026-08-03 and
  restored on 2026-08-04 after the solver sank into known-solvable branches. On a solution,
  `canSolveB` can promote a previously non-FAST option with `s[FAST] = 1`, so later search order
  depends on which states the process has already solved. Given the same binary, query and cache
  history, the default finite stopping point no longer depends on CPU speed or concurrent load;
  progress-heartbeat positions and actual `took` CPU still do. The CPU fallback remains timing
  dependent. Compare verdicts and exact counters on controlled warm/cold baselines; do not use a raw
  output `diff` as the only regression gate. `tools/work_budget_regression.sh` compares ordered cache
  facts from two independent cold work-budget processes.

## Choosing a driver

| driver | use it for | cost |
|---|---|---|
| `radio_canon_search_generic.c` | **Prefer this for new `k=9` results.** Finds a tree that terminates in canonical `U_k` singleton states, which is a self-contained proof. Produced the `473:6`, `480:5`, `496:4` trees. | minutes |
| `radio_oracle.c` | **Prefer this whenever you want more than a handful of verdicts.** A persistent warm-cache oracle: pays `init()` and cache replay once, then answers `<k> <n1> <m1> ...` from stdin until told to `quit`, keeping every fact it learns. Also answers `enumerate <k> <n1> <m1> ...` (2026-08-21): every winning top-level split of the state, exactly, via a sound `R_0` pre-filter before ever calling `canSolveB` — seconds to minutes through 3-4 parts, not scoped past that (raw-space cost; see [../evidence/oracle_enumerate_2026-08-21.txt](../evidence/oracle_enumerate_2026-08-21.txt)). Start it with `./run_radio_oracle.sh`; drive it from `tools/oracle_client.py`, or over the network from `tools/oracle_server.py` (a persistent AWS-hosted instance, `tools/oracle_serve_ec2_launch.sh`/`oracle_serve_status.sh`, survives local disconnection). | seconds to start, then sub-millisecond per cached query |
| `radio_one.c` | One question: is this state solvable in `k`? Pays full `init()` per process, so use `radio_oracle.c` for batches. | varies wildly |
| `radio_pareto.c` | Walk the Sb frontier for any `k` as a staircase: `<k> <n1> <n2> [cache]`. Generic replacement for `radioSbPareto.c`. Reproduces the proven k=6 column exactly; a k=8 walk is the standard heavy benchmark. | minutes to days |
| `radio_full.c` | Every top-level split plus a solvability matrix. Thorough and **much** more expensive than `radio_one` - the killed `k=9` runs used this. | hours to never |
| `radio.c` | Walks the `Sa` ladder upward, printing `can/can't solve Sa(n) in k`. Produced `out_radio_1.txt`. | days |
| `radioR.c` | Same, downward from `MAX_N`. | |
| `radioSbPareto.c`, `r_pareto_9_short.c` | Walk the Sb Pareto frontier. `radioSbPareto` starts at the diagonal and decreases `m`; `r_pareto_9_short` resumes at a given point. | days |
| `radio_print.c` | Renders a numbered witness tree (the `witnesses/sa192_*.tree` format). Filter output with `grep resultprint`. | |
| `radio_s_table.c`, `radio_test.c`, `radio_deep.c` | Ad-hoc probes. | |
| `r2.c`, `radioSb*.c`, `src/*.java` | Superseded. Kept for reference. | |

### Wrapper scripts

```
./run_radio_canon_search_generic.sh <target_k> <k> <n1> <m> [<n1> <m> ...]
./run_radio_full.sh [cache_file] <k> <n1> <m> [<n1> <m> ...]
```

Both derive the required compile-time bounds and compile before running. `run_radio_full.sh`
separately derives `MAX_N` and `MAX_PART_N`, and also scans a cache file to size the build.
`target_k` in the canonical search is the depth at which the
search stops and demands a canonical `U_k` state; 3 or 4 works well at `k = 9`.

Example - smoke test the toolchain:

```
./run_radio_canon_search_generic.sh 3 8 248 3     # proves Sb(248:3) in 8
```

The search is deterministic for a given binary, but the output is **not** byte-comparable
against a committed tree: `radio_canon_search_generic.c` changed on 2026-04-16, and the
current version finds 7 top-level trees for this state (307 nodes) where the version that
produced `witnesses/canon_248_3_at8.tree` found 2 (23 nodes). Both are valid proofs of the
same claim. The right check is that the output *verifies*, not that it matches:

```
./radio_canon_search_generic 3 8 248 3 | grep -E '@[0-9]+ (\[canonical|--\[)' > /tmp/t.tree
tools/check_witness.py /tmp/t.tree
```

## The warm oracle

`radio_one` answers one question per process, and the process cost is not always the solve. `init()` runs
**before** the argument check; unless `MAX_PART_N` is set separately, its component catalog scales
with `MAX_N`, so the same query costs
205 s built at `-DMAX_N=400` and 0.2 s at `-DMAX_N=120`. Replaying the archived caches costs minutes
more. Anything that wants thousands of verdicts — labelling a dataset, ranking splits, poking at a
frontier — has to amortise that, which is what `radio_oracle.c` is for.

```
./run_radio_oracle.sh                 # build if needed, prime, then read queries on stdin
python3 tools/oracle_client.py ./radio_oracle_k9_n300_p300 .artifacts/oracle-cache/*.cache   # smoke test
```

**Sizing is a one-time decision.** `MAX_N` need only cover the largest total state size you will
*ask* about, while `MAX_PART_N` covers its largest component. The oracle loader skips facts too
wide for either bound and reports the count, so a narrow build stays usable against a wide cache.
Default `MAX_K=9, MAX_N=300, MAX_PART_N=300` covers the archived caches (widest fact
258) and Sa(193) states (193), and inits in 37 s at 0.64 GB. **Do not infer a scaling law** —
`MAX_N=400/MAX_K=6` inits in 205 s while the larger `MAX_N=485/MAX_K=9` inits in 146 s. Cost tracks
the work actually required, not the table dimensions, so measure the configuration you intend to use.

**Priming.** Do **not** restore the historical full-corpus snapshot into a current proof/search
run. It mixes positive and negative facts produced before singleton-majorization sufficiency was
refuted, and individual positive origins cannot be separated. Start current `main` cold, or load
only provenance-separated facts whose status survived the refutation, such as the negative
`Sa(193)` certificate. The following is retained only as a performance measurement: the whole
archived corpus — 21,866,180 facts — loaded in **1.58 h** on a 32 GiB box
(measured, run `oracle-prime/20260820T165448Z`). The rate is hump-shaped, from 44,484/s down to
431/s through an expensive band and back to over a million/s once everything is subsumed, so do not
try to predict it from a prefix. `tools/sort_cache.py` reorders a cache for a further 2.25x. The
historical snapshot the run produced is
`s3://radio-sa193-393287594714/oracle-prime/20260820T165448Z/cache.snap.zst`, 667 MiB — in **32.8 s
at 2.41 GB resident**. Keep it for forensic/performance context; do not use it to prime new work.

**Stream separation.** `radiobase.c` and `canSolveB` print progress to stdout, which would corrupt a
line protocol. The oracle keeps a duplicate of the original stdout for responses and points the C
library's stdout at stderr after the provenance banner, so the banner stays with the response stream
(retained output still passes `check_provenance.py`) and all later chatter goes to stderr.

**`MAYBE` is not a refutation.** The per-query budget defaults to finite, because an unbounded query
would hang the daemon. `budget 0` removes the deadline and accepts that some states never return.
Never record a `MAYBE` as a negative — that is exactly how the 2023 corpus acquired 37 false ones.

**It grows forever.** The result cache is never freed; that is the point, so wrap an unattended
session in `tools/capped_run.sh`. Per-fact dominance insertion is exact by default. Negative upward
closure does not enter a branch whose easiest completion violates star-expansion majorization, and
equal source parts have one canonical choice. A nonzero `RADIO_CACHE_INSERT_NODE_LIMIT` is retained
only as a diagnostic override; if used, `cache=partial:N/N` means a final verdict with a sound
partial closure, not `MAYBE`.

**Snapshots.** `snapshot <path>` writes the cache structure; `restore <path>` or `--restore=<path>`
reloads it linearly instead of re-deriving every dominance closure. Snapshot v4 begins the
necessity-only singleton-majorization epoch; v1--v3 snapshots are rejected even by `restore-any`.
Compatibility within v4 is keyed on the source commit plus
`MAX_K`/`MAX_N`/`MAX_PART_N`/`MAX_SBB`/`sizeof(front_point)` — what actually fixes the layout. A geometry mismatch
is refused outright; a foreign identity with matching geometry needs the explicit `restore-any`
opt-in and warns. Keying on the *build id* was a bug: it made a Linux-built snapshot unusable on
macOS for no semantic reason.
Round-trip verified: 16,099 facts dumped in 247 ms and restored in 212 ms with identical verdicts on
300 queries. Snapshot size at that scale is 2.7 KB per input fact, which is the number the first
full-corpus run needs to check.

## Cache files

Solver output can be fed back in to skip work already done. `parse_out.sh` converts a raw
log into the compact form `parse_file` reads:

```
cat out_run.txt | ./parse_out.sh >> cache.txt
./radio_one cache.txt 9 432 9
```

A driver takes an optional leading cache-file argument, detected by argument-count parity.

Current output emits `# radio-cache-semantics=singleton-majorization-necessity-only-v1` before cache facts.
`parse_file`, the oracle loader and the Pareto exact loader accept positive facts only after that
marker; an unmarked historical file is replayed negative-only. This must happen at ingestion:
older singleton positives—including facts derived from the retired `K<=5` production shortcut—may
already have tainted nonsingleton ancestors, so filtering only singleton queries is insufficient.
Oracle journals write the marker before new facts.

For phase attribution, compile with any of `RADIO_INIT_PROFILE`, `RADIO_SPLIT_PROFILE` and
`RADIO_CACHE_PROFILE`. They time eager initialization, lazy split/FAST construction, and the
search-versus-dominance-insertion boundary respectively. The K=6 mass-683 core showed why the
separation matters: exact recursion took 0.004 seconds while the old unbounded dominance insertion
continued beyond 12.9 CPU minutes. The corrected exact insertion takes 30 nodes; see the
[main-solver correction record](../evidence/main_solver_singleton_refutation_2026-08-31.md).

Note that `parse_out.sh` keeps only the verdict, **discarding the `with [...]` split
witness**. It is enough to warm the cache but not to reconstruct a witness tree. If you may
want trees later, archive the raw output instead - see [data.md](data.md).

`run_pareto9.sh` chains runs this way, but it depends on `parsed_260.txt` and a
`pareto9_N.txt` sequence that no longer exist, so it cannot currently restart. Fixing this
is item P4 in the [research plan](research-plan.md).

## Verification tools

```
tools/check_witness.py witnesses/*.tree    # re-derive every step of a witness tree
tools/check_tables.py                      # invariants + formulas + generated doc blocks
tools/check_tables.py --render             # rewrite the generated blocks in the docs
```

Both are pure Python 3 with no dependencies. Reading the spreadsheets needs `openpyxl`;
there is a venv at `.venv` for that.

**Run `check_witness.py` before recording any new result.** A tree using only canonical or
distinct-slot terminals is a proof that does not depend on the solver being correct; a solver log
is not.  Recursive trees may
stop either at `[canonical U_k]` atom sub-multisets or, in older/diagnostic output, at
`[majorized G_k]` arbitrary singleton sequences.  The checker verifies the former by multiplicity
and the latter by every weak-majorization prefix, but reports the latter as **unsupported** because
the universal converse is false.  A structurally valid tree containing such a terminal is not an
achievability proof.

## Budget and root-reachability diagnostics

`tools/budget_probe.c` runs one query with a finite allowance and reports both actual process CPU
and accepted-prefix work.  Its millisecond argument means calibrated nominal milliseconds in the
default build and CPU milliseconds in the fallback build:

```
tools/build_radio.py -O3 -DMAX_K=5 -DMAX_N=127 \
    tools/budget_probe.c -o /tmp/budget_work
tools/build_radio.py -O3 -DMAX_K=5 -DMAX_N=127 \
    -DRADIO_CPU_BUDGET -DRADIO_MEASURE_WORK \
    tools/budget_probe.c -o /tmp/budget_cpu

/tmp/budget_work 100 5 15 3 14 3 17 2 8 4 11 2 10 2 19 1 15 1
```

Exit 0/1/2 means `TRUE`/`FALSE`/`MAYBE`. `tools/work_budget_regression.sh` is the short cold-process
determinism gate; `tools/deadline_regression.c` checks local shared-bound arithmetic under both
budget modes.

`tools/rb_root_probe.c` isolates the proposed `rb_dead(0,0,0,0)` test without recursively solving a
child or changing the production trigger:

```
tools/build_radio.py -O3 -DMAX_K=7 -DMAX_N=400 \
    tools/rb_root_probe.c -o /tmp/rb_root_probe
/tmp/rb_root_probe 7 111 3 115 2 121 1
```

It builds the same theorem-filtered per-part split tables and suffix reachability DP as `canSolveB`.
`DEAD` is a sound necessary-condition refutation: no combination of remaining part cuts can keep all
three child masses under `3^(k-1)`. `ALIVE` only means that this relaxation passes; it is not a full
solver run. The default `RB_MAXCAP=800` permits root use through k=7 and reports `UNAVAILABLE` at k=8
and above. A complete small census found modest incremental rejection after full-star majorization,
and eager construction did not improve either a hard positive or the saturated deadline state, so
ordinary search still arms the DP only after measured cost. Exact counts and timings are in
[`../evidence/work_budget_rb_root_2026-08-13.txt`](../evidence/work_budget_rb_root_2026-08-13.txt).

The probe also reports hereditary suffix pliability. `exact_head=i` means the first `i` nonunit parts
are the only possible rigid head: suffix `i` and every later suffix fit every residual capacity
triple, so `rb_dead` is provably vacuous there. `theorem_head` is the cheaper retained-corner
certificate and `coarse_head` is its original length/excess corollary. `slack_excess_head` retains
the full absolute slack in the stronger bound `2(D-q)<=slack-4`. Set `RB_PLIABILITY_VERBOSE=1` for
one line per suffix. `tools/rb_pliability_regression.sh` locks the sharp boundaries, and
`tools/rb_pliability_census.py` reproduces the complete 283-state comparison. Definitions and the
first census are in
[`../evidence/rb_pliability_2026-08-13.txt`](../evidence/rb_pliability_2026-08-13.txt).

Compile with `RADIO_RB_PROFILE_DIAGNOSTIC` to print actual calls and rejections at every suffix; it
also enables the exact/theorem report so each observed row carries its structural features.
`tools/rb_suffix_profile_census.py` builds a forced-arming (`RB_TRIGGER=1`) binary and runs every
small state cold. `RADIO_RB_PLIABLE_CUTOFF` is an opt-in lab comparison that scans the completed DP
and skips exactly certified lookups. It is sound but deliberately not a default: even after skipping
77.89% of lookups on the hard positive, paired CPU timings were noise-level. Proof, profiles and
commands are in
[`../evidence/rb_slack_profile_2026-08-14.txt`](../evidence/rb_slack_profile_2026-08-14.txt).

## Split-heuristic research tools

The 2026-08-09 long-state experiments use exact solvability of small child subsets. These are lab
tools, not yet part of `radiobase.c`:

| tool | purpose |
|---|---|
| `tools/pairtab.c` | build every solver-solvable pair of individually solvable parts at one level |
| `tools/tripletab.c` | build every solver-solvable three-part state at one child level, gated by the exact pair table |
| `tools/quadtab.c` | build every solver-solvable four-part state, gated by pair and triple tables |
| `tools/subset_census.c` | count per-part, pair and triple survivors on complete k=5 four-part state lists |
| `tools/filter_triples.c` | apply triple and optional quad tables to an existing split-feature/label dataset |
| `tools/label_split_features.c` | join `WIN ... state=... take=...` logs to feature rows without relying on row samples |
| `tools/sample_subsets.c` | sample pair/triple rejection and lookup cost on k=6 states too large to enumerate |
| `tools/fast_replay.c` | replay logged long k=5 states from one forked warm-cache image, clearing all per-target cache/self-training effects |
| `tools/budget_probe.c` | compare deterministic accepted-prefix allowances with the measured CPU fallback on one finite query |
| `tools/rb_root_probe.c` | evaluate the complete first-test mass relaxation `rb_dead(0,0,0,0)` without enabling it in ordinary search |
| `tools/rb_pliability_regression.sh` | check exact and theorem-certified hereditary suffix cutoffs at sharp slack boundaries |
| `tools/rb_pliability_census.py` | reproduce the complete small-state exact/theorem pliability comparison |
| `tools/rb_suffix_profile_regression.sh` | lock actual per-suffix call/prune accounting and the opt-in exact cutoff |
| `tools/rb_suffix_profile_census.py` | force RB in cold small states and correlate reached rejections with slack and tail excess |
| `tools/bundled_majorization.py` | evaluate the sound depth-`d` synchronized-majorization predicates and compare them with a complete pair table |
| `tools/singleton_pair_coloring_census.cpp` | exhaust full-mass singleton partitions majorized by `G_K` through `K=4`; count, rank, window and parallelize exact-support transfer shells through `K=6`, using one-block lookahead followed by exact Fixed-Color Hall search; check Row-Coloring inequalities, global Adjacent-Fiber transfers and coloring rules; inspect labelled fixed-boundary delta exchange, dangerous tight-set core/hull, exact flip-blocker intersections, and maximum/positive crossing-mass rules; probe fixed feasible colorings for positive common neighbors through sampled `K=5` transfers; sample the exact `K=5` universe; check the padded Pascal-prefix reduction and its compressed `K=19` strict-alternation counterexample |
| `tools/singleton_transfer_shell_regression.sh` | provenance-build the transfer-shell census; cross-check shell totals, fast rank-window skipping, parallel/sequential aggregates, all 160 exact-support `K=3` parents, the final `K=5` shell, and canonical / transfer-13 / counterexample `K=6` controls |
| `tools/singleton_pascal_interval_census.cpp` | exhaust exact-row contracted Pascal bands and arbitrary-row suffixes through `K=4`; check tight-prefix transitions, two-anchor residual colorings, longest-half mixed splits and same-color predecessors; reproduce the exact-support `K=6` singleton-majorization counterexample, its 14-transfer phase boundary, residual hole, forced bands, and earlier rule counterexamples |
| `tools/singleton_direct_split_cleanroom.cpp` | independently enumerate legal singleton row triples into three sorted child multisets, with no Hall code or shared cache; solve arbitrary first-cut queries, check the padded/mass-697/mass-683 `K=6` holes and the feasible one-8 deletion with slack-correct residual bounds, and classify all 176 bands on the fixed rank-15/32 face |
| `tools/singleton_direct_split_regression.sh` | provenance-build the clean-room direct-row solver and run its canonical `G_6`, `j=13`, three negative counterexample forms, feasible one-8 deletion, fixed-face, naive-oracle and exhaustive `K<=3` controls |
| `tools/singleton_main_solver_regression.sh` | check that a nonembedded majorized `K=2` singleton reaches exact recursion, plus canonical / `j=13` / mass-683 exact negative and its upward consequences, cache-taint rejection, and exact positive/majorization-bounded-negative dominance insertion |
| `tools/cache_upward_closure_regression.sh` | exhaust the normalized non-unit three-part `K=3` cache universe and compare every in-bound negative upward consequence with an independent coordinatewise perfect-matching test; locks equal-part quotienting and the majorization boundary |
| `tools/cache_semantics_regression.sh` | check that unmarked cache input is negative-only while the current semantic marker admits positive Sb and Sa facts |
| `tools/singleton_tight_band_certificate.cpp` | emit deterministic no-first-cut certificates from two tight anchors; count exact-support/band spaces, exhaust the positive-mixed-floor `K=5` faces, and minimize transfer distance over all capacity-certificate cap vectors; contains no split search or Hall code |
| `tools/singleton_tight_band_regression.sh` | provenance-build and run the inequality-only regression, complete 613,689,090-instance `K=5` certificate census, and independent direct-row controls/fixed-face classification |
| `tools/singleton_balanced_hh_census.cpp` | construct the canonical Pascal Havel--Hakimi incidence matrix; test exact canonical colorability and the deterministic row/incidence-switch descent; exhaust exact-support parents through `K=4`, enumerate `K=5` windows, run random/walk/hill probes through `K=6`, and reproduce the fixed-matrix and strict-descent counterexamples with `canonical-state` / `state` |
| `tools/singleton_monotone_transfer_census.py` | construct actual ternary-word colorings showing that every full-mass `K=3` type dominated by `G_3` is reachable from the canonical coloring by monotone direct one-vertex recolorings; distinguish the unit-ready representative pass from five targeted paths |
| `tools/singleton_solution_fiber_dag.py` | exhaust the normalized Fixed-Color Hall solution fibers and their color-preserving unit-transfer relations through `K=3`; propagate directed source cones and compute the bidirectional component projection; exhaust downward-closed Lorenz-area coloring ideals through `--area-ideal K D`; enumerate every self-sorted Pascal greedy shuffle and exactly classify all possible coloring sources through `--greedy-sources K` for `K<=8`; exhaust padded or exact-support rational Hall grids with `--rational-grid K D` and `--exact-rational-grid K D`, reporting minimum scaled defects and optionally every hole with `--list-rational-holes` |
| `tools/singleton_allocation_fiber_dag.cpp` | exhaust all normalized legal first-cut allocations and literal same-child coin transports through `K=3`; classify births/deaths, source orbits and the two forward components; exhaust bounded Lorenz-area ideals with `--area`; inspect one higher-level fiber or edge with `--state` / `--edge` through `K=5` |
| `tools/singleton_shape_survey.cpp` | reconstruct legal first cuts; test scalar/full-profile shape targets, atom-sized-piece restrictions, and feasible split-count fibers; exhausts the scalar rule through `K=4`, and gives `K=3` counterexamples to the full-profile and per-row atom restrictions |
| `tools/singleton_strong_niceness.cpp` | recursively count monomial coefficients of the transcript graph's chromatic symmetric function and exhaust strong-niceness comparisons through `K=3` |
| `tools/search_singletonization.cpp` | exact-negative small-m synchronized search with selectable relaxed-majorized, distinct-slot embedded, or exact-atom terminals; rank or exhaust all proven-Pareto four-segment assemblies, scan one chosen assembly/frontier, or solve a fixed-residual slice with memo reuse |
| `tools/optimize_mixed_frontier.py` | combine a complete two-coordinate mixed-deficit frontier with the two pure-child thresholds and recover the maximum parent D-width |
| `tools/singletonization_regression.sh` | lock complete assembly rankings/optima, corrected four-segment boundaries, the sharp m=5 leaf atomization depth, exact variable-width synchronization, and memo-exhaustion abort semantics |
| `tools/search_atom_profiles.cpp` | symbolic aligned-profile recursion at 8, 16 or 32 atoms, with all-depth D-lineage, finite-depth mixed-supply pruning, and finite `(D,C+D)` coinductive obstructions |
| `tools/check_atom_profile_certificate.py` | independently exhaust the local algebra behind a D-lineage closed losing-set certificate |
| `tools/check_atom_profile_tree.py` | independently re-derive every split, leaf inequality and threshold in a symbolic positive tree |
| `tools/check_atom_parent_formula.py` | independently derive the general `(s,b,c)` parent profile and compare its closed width formula with direct atom evaluation |
| `tools/check_atom_profile_cover_log.py` | verify the loss-class accounting and restricted-negative scope of a completed exact cover-slice log |
| `tools/check_dc_kernel_certificate.py` | independently exhaust every cut from the 16- and 32-atom projected losing kernels, validate their complete rank bands, and replay an optional boundary tree |
| `tools/check_dc_tree_lift.py` | lift a retained projected tree exactly, stream all projected skeletons with the hidden coordinate, or synthesize a projected losing kernel at any power-of-two normalization |
| `tools/atom_profile_regression.sh` | verify the height-4/5 controls, exact eight-/sixteen-atom height-6 optima, and the all-depth 32-atom exclusion through rank 1179 |
| `tools/pareto_lift_probe.c` | search the lineage-preserving lift box of a known lower-level split; diagnose whether a known parent split descends from any lower split |
| `tools/pareto_prefix_census.c` | enumerate both cuts below every one-part Pareto root, globally upgrade every effective descendant to its residual fixed-dimension Pareto antichain, and fully map every endpoint |
| `tools/analyze_pareto_prefix_census.py` | structurally validate a completed census and summarize raw, symmetry-quotiented, structured-prefix and upgrade distributions |
| `tools/pareto_census_aws_remote.sh` | verify, build and detach a resumable census bundle on the shared AWS host with individual, combined and idle guards |
| `tools/pareto_census_status.sh` | make one bounded SSM query for the current remote k=8 census; it does not watch or poll after returning |

Example table builds (the stated `MAX_N` includes all parts in a table entry):

```
tools/build_radio.py -O3 -DMAX_K=4 -DMAX_N=64 tools/pairtab.c -o /tmp/pairtab4
/tmp/pairtab4 4 16 > /tmp/pairs_k4.txt

tools/build_radio.py -O3 -DMAX_K=4 -DMAX_N=96 tools/tripletab.c -o /tmp/tripletab4
/tmp/tripletab4 4 16 pairs_k4.txt > /tmp/triples_k4.txt

tools/build_radio.py -O3 -DMAX_K=4 -DMAX_N=128 tools/quadtab.c -o /tmp/quadtab4
/tmp/quadtab4 4 16 pairs_k4.txt /tmp/triples_k4.txt > /tmp/quads_k4.txt

tools/build_radio.py -O3 -DMAX_K=5 -DMAX_N=100 tools/tripletab.c -o /tmp/tripletab5
/tmp/tripletab5 5 32 pairs_k5.txt > /tmp/triples_k5.txt
```

The tables' positive and negative entries are exhaustive according to the current C solver. That is
enough for a fallback-safe heuristic experiment, but it is not an independent certificate. Do not
let a table negative prune an exhaustive proof search until the table has an adequate audit.

### Recursive Pareto-lift probe

The lift-box experiment starts with an aligned parent `P`, a componentwise lower template `T`, and
a solving cut `s` of `T`.  It searches only

```
s <= X <= s + (P - T)
```

around the proportional coordinate lift, ranking candidates by how closely their three outcome
masses preserve the lower split's proportions.  The box containment is a theorem, but it is only a
necessary lineage condition: every proposed parent child is still checked by `canSolveB`.  Cache
negatives screen candidates, misses receive the requested strict per-child budget, and a caller
must retain ordinary search as fallback.

Build and reproduce the primary four-part example:

```
tools/build_radio.py -O3 -DMAX_K=7 -DMAX_N=192 \
    tools/pareto_lift_probe.c -o /tmp/pareto_lift_probe

/tmp/pareto_lift_probe CACHE 7 recursive 12 3000 200 \
    45 10 24 5 3 0 \
    33 15 19 9 11 5 \
    32 14 19 8 15 7 \
    23 20 15 13 7 6
```

The arguments after `recursive` are maximum `L1` radius, explicit-candidate budget per shell and
strict probe milliseconds per child, followed by `(parent_n parent_m lower_n lower_m cut_n cut_m)`
for each aligned part.  A zero lower coordinate is an intentional degeneration placeholder; its
parent interval remains free and its centre is the midpoint.

The diagnostic inverse form replaces radius with a known parent cut and enumerates lower cuts whose
lift boxes contain it:

```
/tmp/pareto_lift_probe CACHE k inverse budget probe_ms \
    parent_n parent_m lower_n lower_m parent_cut_n parent_cut_m [...]
```

Exit 0 means a split was found, 1 means the requested box/bound was exhausted, and 3 means the input
or claimed lower/parent split was not verified within its local budget.  Neither a miss nor an
inverse miss is a proof of unsolvability.  The lemma, examples and current recursive obstruction are
in [the theorem note](theorems/recursive-pareto-lift.md).

### Pareto-prefix census

`pareto_prefix_census.c` builds the choice corpus that the one-template lift probe lacks.  For every
proven one-part root in `data/pareto_sb.csv`, it retains every labelled winning first cut, every
labelled winning cut of the resulting two-lineage mixed child, and the four-lineage mixed
grandchild.  Canonical descendants are deduplicated only after their four labelled lineages have
been written.  Empty and `1:1` lineages are then removed structurally by Unit-Group Elimination;
the count of removed unit groups remains attached as a scalar capacity reserve.  The resulting
zero-, one-, two-, three-, and four-part seeds are each traversed upward in their own fixed
dimension to **every** globally maximal solvable residual state.  They are never padded with
invented parts.  Endpoint maps enumerate every winning core cut and attach the exact multiplicity
of valid labelled top-level assignments of the reserved unit groups, including the two shore
choices that produce outcome 1.  This global residual antichain is
intentionally broader than a particular parent-conditioned lift box; the raw lineage records and
component-alignment counts permit narrower lineage-preserving filters afterward.

Build with room for every represented upgrade and cap the run in the usual way:

```
tools/build_radio.py -O3 -DMAX_K=8 -DMAX_N=336 \
    tools/pareto_prefix_census.c -o /tmp/pareto_prefix_census

tools/capped_run.sh --seconds 14400 --rss-gb 16 --label pareto-census-k8 -- \
    /tmp/pareto_prefix_census DOMINANCE_CACHE data/pareto_sb.csv 8 2000000 \
    ROOT_WINNER_LOG EXACT_CACHE [SECOND_CHECKPOINT ...] \
    > census-k8.out 2> census-k8.err

tools/analyze_pareto_prefix_census.py --pareto-csv data/pareto_sb.csv census-k8.out
tools/test_pareto_prefix_census.sh
```

The optional root log supplies already-archived top-level cuts; pass `-` in that position to retain
the exact cache while independently re-enumerating the first level.  Every supplied child is
checked against the exact oracle or current solver, and a missing root is enumerated normally.  The
optional exact cache is held in a flat full-state hash rather than inserted into the dominance trie.
Exact hits are
therefore cheap and memory-bounded, while misses still fall through to `canSolveB(...,NO_DEADLINE)`.
As with the dominance trie, a current build ignores an imported positive for a nonembedded
majorized singleton and derives it by exact recursion, because older exact-state tables do not
record whether the former converse shortcut created that fact.
Only a deliberately selected cache slice should be replayed into the dominance trie for partial-
prefix pruning.  This is a research-driver hook compiled through `RADIO_EXTERNAL_EXACT_LOOKUP`; an
ordinary `radiobase.c` build has no hook, storage or lookup overhead.

At every enumerated component cut, the driver also applies an exact local necessary condition: all
four induced rectangles must lie on or below the proved one-part frontier at the child level.  A
solvable multi-part child stays solvable after deleting all other components, so a rectangle beyond
that frontier cannot be rescued by later parts.  Missing/unproved frontier entries do not prune.
On the hard `Sb(62:10,82:7)@7` second state this reduces the raw Cartesian grid from 460,152 to
25,080 choices before a solver query; an independent k=7 run retained exactly the same 450 first
cuts and 2,956 labelled second-cut lineages as the unfiltered corpus.

Long prefix censuses can resume from one or more raw `SECOND_CHECKPOINT` logs.  The importer ignores
an interrupted tail and accepts only a `FIRST`/`LINEAGE` block closed by its matching
`SECOND_SUMMARY`.  It canonicalizes and deduplicates the recorded winners, checks the summary count,
and exactly re-verifies all three children of every distinct winner under the current engine.  The
closed summary is the completeness claim, so retain the checkpoint's source snapshot and
provenance and use this only when its prefix enumerator is known to match.  A k=7 replay of all 448
closed blocks reproduced the complete labelled geometry and all 563 targets, 819 upgrade nodes and
610 endpoints.

The current detached k=8 continuation is on the shared AWS host.  Its launch helper records and
uploads the exact source/input hashes, build sidecar, run arguments and memory policy, and its
supervisor validates and analyzes a successful final output before uploading it.  Monitoring is a
one-shot command, so it is safe to invoke from an ordinary terminal without leaving another local
watch process behind:

```
tools/pareto_census_status.sh
```

The output's `CENSUS` records are stable TSV.  The analyzer refuses a log without `CENSUS END`,
reconstructs both descendant levels and every final child from the logged cuts, checks masses,
unit reserves, dominance, component-alignment multiplicities and all summary totals, and quotients
final cuts by exact state automorphisms.  It reports the upgrade and solution distributions
separately for each effective dimension, counts labelled unit-group extensions, and measures how
far every first/second choice lies below the corresponding one-part frontier at the same smaller
shore.  A completed census is still empirical current-solver evidence, not an independent negative
certificate, unless its oracle negatives have separately been proved.

The bundled predicates start at full star-expansion majorization (`R_0`), add one synchronized
rectangle split per level, and reach exact solvability at `R_k`.  Each is a sound necessary
condition, but nesting between adjacent intermediate depths is not proved. Their small regression case and complete k=4
pair census are:

```
tools/bundled_majorization.py ladder 4 16 1 12 2
tools/bundled_majorization.py census-pairs 4 /tmp/pairs_k4.txt
( ulimit -t 60; tools/bundled_majorization.py m6-kernel 7 4 )
```

The checker is independent of `radiobase.c` once it has read the optional pair-table labels. It is
intentionally a transparent research implementation, not a production pre-pass: intermediate depths
remain exponential and can approach the cost of exact search.

`m6-kernel` constructs the parametric four-part state reached by extending the tight
`m=6` prefix, exhausts every first split whose children pass full-star majorization,
deduplicates identical normalized child triples (including outcome symmetry), and checks
them at the requested predicate depth. It does not inherit any continuation from a stored
witness tree.

### Exact-negative small-m singletonization search

`search_singletonization.cpp` is the scale-free counterpart of the predicate prototype for states
with a small total narrow-side multiplicity.  Define `C_d(S,k)` to hold when `S` passes full-star
majorization and either is already a singleton state, or has a legal rectangle split whose three
children satisfy `C_(d-1)`.  In the default mode, a singleton terminal is accepted by weak
majorization; that is a relaxation, not a proved positive terminal.  Consequently a default-mode
`NO` at `d=k` is an exact nonsolvability proof, while a `YES` is unsupported.  The `embedded` and
`canonical-exact` modes give unconditional positive certificates.  At depth zero full-star
majorization permits at most one edge.

The implementation's short cut range is complete, not heuristic.  In a viable first test on
`(n:m)@k`, both wide-side pieces `a` and `n-a` are at most `2^(k-1)`; otherwise one of the three
nonempty rectangle children has a full-star row wider than the largest entry of `G_(k-1)`.  Thus a
near-top width `n=2^k-delta` has only `delta+1` possible wide cuts.  While assembling a multi-part
split, a partial child that fails `C_(d-1)` can also be rejected soundly by subgraph monotonicity.

Build and run it directly:

```
CC=clang++ tools/build_radio.py -O3 -std=c++20 -Wall -Wextra -pedantic \
    tools/search_singletonization.cpp -o /tmp/search_singletonization

tools/run_with_provenance.py /tmp/search_singletonization 9 9 488 3 488 3
tools/run_with_provenance.py /tmp/search_singletonization \
    canonical-exact 7 3 127 1 119 1 119 1 118 1 111 1
tools/run_with_provenance.py /tmp/search_singletonization \
    embedded 7 2 127 1 119 1 119 1 118 1 111 1
tools/run_with_provenance.py /tmp/search_singletonization forced 10 10 973 6 477 2
tools/run_with_provenance.py /tmp/search_singletonization frontier 10 6 974 973
tools/run_with_provenance.py /tmp/search_singletonization \
    assembly 7 5 10 46 6 22 5 27 3 15 14
tools/run_with_provenance.py /tmp/search_singletonization \
    assembly-enumerate 7 10 data/pareto_sb.csv
tools/run_with_provenance.py /tmp/search_singletonization \
    assembly-rank 8 10 data/pareto_sb.csv
tools/run_with_provenance.py /tmp/search_singletonization slice 4 4 2 5 6 11 2 9 2 3 2
tools/run_with_provenance.py /tmp/search_singletonization \
    mixed-frontier 4 4 2 2 16 16 9 2 3 2 > /tmp/mixed-frontier.out
tools/optimize_mixed_frontier.py 5 5 /tmp/mixed-frontier.out
```

The first form checks one state with a bounded number of synchronized levels.  Its default terminal
is an arbitrary singleton sequence weakly majorized by `G_s`.  Prefix it with `embedded` to require
a coordinatewise injection into distinct `G_s` slots, or `canonical-exact` to require a literal
sub-multiset of `G_s`; exact implies embedded implies majorized.  A positive in either stronger mode
is unconditional.  A negative in either stronger mode is exhaustive only for that terminal
predicate, while a full-depth negative in the default, more permissive mode refutes solvability.
The `m=5` examples bracket the first eventual five-part leaf: embedded and exact both fail through
depth two, while a committed exact tree succeeds at depth three.

`forced` verifies a specified one-part root split and prints its tree.  `frontier` walks downward,
retains the exact memo between adjacent `n`, stops at the first positive, and prints its tree.  Cap
frontier runs with `tools/capped_run.sh`; a memo-limit exception or external cap is an abort, never
a negative verdict.
The retained `k=10,m=6` replay proves the upper bound 973; the checked 973 tree is an unsupported
relaxed-terminal diagnostic:
`evidence/sb_m6_k10_frontier.txt` and `witnesses/majorized_973_6_at10.tree`.

`assembly` is the corrected diagram directly.  Write

    A=(a:alpha) @ parent_k-1,
    B=(b:beta), C=(c:gamma) @ parent_k-2.

For total parent height `m`, its magenta four-segment branch is

    Sb(d:beta, b:alpha-beta, c:m-alpha-gamma, a-c:gamma) @ parent_k-2,

with zero-width or zero-height parts omitted, and the parent candidate has width `a+b+d`.
The command scans `d` downward from `start_d` to `minimum_d` with one memo.  Geometry requires
`beta<=alpha`, `alpha+gamma<=m`, and `c<=a`.  An exact negative followed by a positive proves the
reported D maximum globally by subgraph monotonicity, even when `start_d<2^(parent_k-2)`;
otherwise `global_maximum=NO` is explicit.  The regression reproduces the proven `m=10` parent
widths 12, 33, and 82 at parent levels 5, 6, and 7 respectively, and independently checks every
positive branch tree.  These are finite construction controls, not an eventual-`q` theorem.

`assembly-enumerate` automates the outer choice.  It accepts only contiguous normalized Pareto
levels whose CSV rows are all source-carrying `max,proven-*` facts; a level containing a lower,
upper, witness-only or legacy row aborts rather than being silently treated as complete.  It
enumerates ordered A/B/C triples satisfying `beta<=alpha`, `alpha+gamma<=m`, `c<=a`, and the
separately labelled working assumption `m<=2a`.  For each triple it finds the largest `d` allowed by
full-star majorization, prints the complete resulting ranking, and then scans exact `d` values only
while that triple can still tie the best exact construction already found.  An R_0 failure excludes
larger values; arithmetic excludes a skipped lower suffix.  Consequently
`optimization_complete=YES` is an exact maximum over this specified family, while
`all_triple_dmax_exact=NO` only means that individual losing boundaries below the optimum were not
needed.  All tied winners and their residual trees are printed.

`assembly-rank` performs exactly the same validated triple enumeration and emits
`assembly_ranking_result ... complete=YES bound=R0_NECESSARY`, then exits before the first exact
query.  This separation matters at the next level: a complete static ranking remains a result even
when one residual slice exceeds an external cap.  Full-star values are upper bounds, never
construction claims.  The regression locks exact `m=10` family optima through parent level 7, the
complete level-8 ranking, rejection of the incomplete level-9 Pareto input, and a level-8 two-part
state showing that even a simple R_0-permitted target can be exactly negative.

`tools/m5_assembly.py` is the symbolic known-answer calibration.  It rewrites the published exact
`m=5` theorem as the old `(3,2,2)` and new `(4,3,1)` assembly candidates, prints their A/B/C/D
dimensions, and checks the identities through a configurable level.  It also verifies the uniform
two-test singleton-majorization template for the new D branch: rejection at `k=9,10`, followed by
success at every checked `k>=11`.

```
tools/m5_assembly.py 8 9 10 11
tools/m5_assembly.py --check-through 64
```

`tools/check_tables.py` invokes the same identities and compares every recorded exact `m=5` row
with the theorem.  `tools/singletonization_regression.sh` performs complete exact assembly
enumeration for `m=5`, parent levels 4 through 9, then independently constructs the theorem's
hard branches at parent levels 10 and 11 and verifies their emitted trees.  It also proves the
three-level exact/embedded minimum for the first eventual majorized leaf and compares the emitted
exact tree with `witnesses/canonical_m5_leaf_p7_at7.tree`.  The symbolic proof and the distinction
between atom masses and atom-profile realizations are in
[the m=5 calibration](theorems/m5-pareto-assembly.md).

`slice` adds one variable part `(2^k-delta : variable_m)` to the listed fixed parts, scans `delta`
from `start_delta` through `maximum_delta`, retains the exact memo, and stops at the first positive.
Because shrinking one component gives a subgraph, feasibility is monotone in `delta`; the first
positive therefore maximizes the variable width over the scanned interval.  Starting at zero makes
that maximum unconditional if the interval reaches a positive.  A nonzero start needs an independent
proof that the omitted smaller deficits fail (for example a full-star bound).  With `depth=k`, every
printed negative and the resulting maximum are exact; at smaller depth they refer only to `C_depth`.
The example is derived from the synchronized counterexample in
[singleton-majorization.md](theorems/singleton-majorization.md#why-there-is-no-single-width-two-base-sequence):
its negative endpoint and subgraph monotonicity show that the fixed parts `(11:2,9:2,3:2)@4`
admit variable width 10 but not 11.  Run
`tools/singletonization_regression.sh` to check this boundary, re-verify its positive tree, and confirm
that memo exhaustion exits as an abort rather than emitting a negative verdict.

`mixed-frontier` constructs

    fixed parts + (2^k-u : left_m) + (2^k-v : right_m),

scans `u` upward, and binary-searches the least feasible `v` using the same retained exact memo.
Thresholds are nonincreasing by subgraph monotonicity; every strict drop is a Pareto-minimal point.
The command prints all points and their checkable trees, then compresses consecutive points of slope
minus one into `mixed_piece u=L:R sum=C`, meaning `v=C-u` for every integer `u` in that interval.
The full legal deficit box is `0<=u,v<=2^k`; at the upper endpoint the corresponding zero-width
part is omitted.

`complete=YES` means the bounding box cannot hide another minimal point: either the `u=0` threshold
was found or the legal vertical box was exhausted, and either `v=0` was reached or the legal
horizontal box was exhausted.  `exact=YES` additionally means `depth=k`; otherwise the frontier is
exact only for the bounded singletonization predicate.  A `complete=NO` frontier contains valid
candidates but is not a global frontier, while an `exact=NO` frontier can omit solvable points
outside that restricted predicate.

Given exact pure-child thresholds `U_2,U_0`, `optimize_mixed_frontier.py` minimizes
`max(p,U_2)+max(q,U_0)` over the printed pieces and reports both the deficit and
`parent_D_width=2^(k+1)-deficit`.  It refuses either `complete=NO` or `exact=NO` input.  For a piece
`p in [L,R], q=C-p`, the unconstrained optimum is `max(C,U_2+U_0)`; restricting `p` to `[L,R]`
adds the distance from that interval to the interval joining `U_2` and `C-U_0`.  Thus a stable finite
piece description gives a constant-size D optimizer even when the antichain itself grows with `k`.

### Eventual power-of-two atom-profile search

`search_atom_profiles.cpp` is narrower than `search_singletonization.cpp`: it searches only
non-wasteful strategies whose every current width is a sum of `N` `G_r` atoms and whose cuts take
`N` of the `2N` atoms produced by refinement.  The default is `N=8`; compile with
`-DATOM_PROFILE_ATOMS=16` or `32` for the larger supported normalizations.  A profile `(a,b,c,d)` means
`a A_r+b B_r+c C_r+d D_r`.  Its eventual deficit coefficients are
`(d,c+d,b+c+d)` in the basis `(binom(r,2),r,1)`, so the program compares profiles symbolically and
prints a concrete lower threshold for every positive tree.

Several symbolic necessary tests precede exact recursion.  The all-depth D-lineage theorem rejects a
height-`h` state when the unweighted sum of its D counts is below `max(0,h-4)`; this is a closed
losing-set proof, not a depth cutoff.  The finite-depth mixed-supply bound iterates the full
triangular deficit transform along the adversarial mixed path and rejects a state when even its
optimistic supply cannot reach the terminal prefix.  Its propagated-loss refinement applies the
same transform to supply discarded by the first mixed transition, giving a sound lexicographic
budget while a global cut is assembled.  A height-aware envelope additionally caps each ancestral
part's terminal coordinate supply by its height times the atom count and requires enough levels to
split it into singleton lineages.  An exact mixed-only recursion tests the adversarial transcript
that returns outcome 1 at every node; its `NO` is a sound obstruction, while its `YES` merely
permits the full three-outcome recursion.  The `(D,C+D)` projection retains the first two deficit
coefficients and over-approximates aligned play, so its `NO` is a sound exclusion at the requested
depth.  For the 16-atom rank-290 projection, the program can also synthesize a finite coinductive
kernel: its independently checked upward closure replaces the depth qualifier entirely.
`check_dc_tree_lift.py` applies the same projected algebra at larger power-of-two normalizations;
its retained 32-atom kernel closes three adjacent projection bands.  Other negative results remain
exhaustive only inside the configured aligned model and at that depth.

Build and run it as follows:

```
CC=clang++ tools/build_radio.py -O3 -std=c++20 -Wall -Wextra -pedantic \
    tools/search_atom_profiles.cpp -o /tmp/search_atom_profiles
tools/run_with_provenance.py /tmp/search_atom_profiles height6 2
tools/run_with_provenance.py /tmp/search_atom_profiles height6-lineage-certificate
tools/run_with_provenance.py /tmp/search_atom_profiles height6-max 3 1 82

CC=clang++ tools/build_radio.py -O3 -std=c++20 -Wall -Wextra -pedantic \
    -DATOM_PROFILE_ATOMS=16 tools/search_atom_profiles.cpp -o /tmp/search_atom_profiles16
tools/run_with_provenance.py /tmp/search_atom_profiles16 height6-dc-kernel-certificate
tools/run_with_provenance.py /tmp/search_atom_profiles16 height6-dc 3 305
tools/check_dc_tree_lift.py evidence/atom_profile_height6_dc16.cert --rank 305 --expect NO
tools/check_atom_profile_tree.py evidence/atom_profile_height6_rank305.cert
# Optional two-minute rediscovery of the retained exact tree:
tools/check_dc_tree_lift.py evidence/atom_profile_height6_dc16.cert \
    --rank 305 --all-skeletons --depth 3 --expect YES --emit-tree
tools/check_dc_kernel_certificate.py evidence/atom_profile_height6_dc32.cert
# Optional ~2.5-minute rediscovery of the retained 32-atom kernel:
tools/check_dc_tree_lift.py --atoms 32 --rank 1121 --projected-only --depth 32 \
    --expect NO --emit-dc-kernel 3,4
tools/check_atom_parent_formula.py
tools/atom_profile_regression.sh
```

`height6` tests the `ABBBBBCD` refinement class; it now returns the all-depth one-D obstruction
immediately.  The two control modes reproduce the established height-4 and height-5 constructions.
`height6-max depth [first_rank [last_rank]]` orders every configured A--D profile from eventually
widest to narrowest and retains both exact and abstract memos across the inclusive rank interval.
At eight atoms, ranks 1--81 have all-depth D-lineage certificates and rank 82 `A^6D^2` has an
independently checked depth-3 relaxed tree, so the first 82 scan proves the exact all-depth optimum
of that relaxed slice.  At 16 atoms, ranks 1--289 are lineage-excluded and rank 290 is abstractly negative through
all depths by the 242-core projected kernel.  Since ranks 290--304 share that projection, all are
excluded.  The first checked rank-305 projected tree has no exact lift, but
`check_dc_tree_lift.py` streams alternative winning projected splits and finds a different exact
19-node relaxed tree.  Rank 305 is consequently the exact widest sixteen-atom D germ in that model.
The retained kernel and first projected tree are `evidence/atom_profile_height6_dc16.cert`; the relaxed positive is
`evidence/atom_profile_height6_rank305.cert`.

This does not maximize arbitrary excessive `q`: 165 is the number of A--D words of length eight,
and 969 the number at sixteen, not the whole longer-profile universe.  At 32 atoms, D lineage and
the 504-core certificate in `evidence/atom_profile_height6_dc32.cert` exclude ranks 1--1179 at all
depths.  Pure refinement makes rank 1181 feasible in the relaxed model, leaving only rank 1180 open
in that slice.  The
propagated-loss budget makes the C++ rank-1180 depth-three product exhaustive and negative.  Guided
loss-sliced cover now also excludes rank 1180 through depth four; this is a bounded result and does
not decide depth five or eventual constructibility.  The Python all-skeleton implementation
independently reproduces the depth-three negative after applying the same symbolic loss lemma.
Range slicing still prevents one hard germ from hiding completed work on later
germs.  The C++ tool's default two-million exact
memo-entry ceiling aborts explicitly rather than printing `NO`; external time and memory caps
remain appropriate for deeper runs.  The proof and scope are in
[the atom-lineage note](theorems/atom-lineage.md).

For a custom exact state, `profile-state depth WORD:height [...]` runs the same recursion and emits
a machine-checkable tree on success.  `profile-state-flat` enumerates complete global cuts at the
requested root before recursing; `profile-state-prefix` forces the ordinary outer-part prefix order
at that root.  `profile-state-guided` first enumerates every winning `(D,C+D)` projected skeleton
and then every compatible hidden B-coordinate cut; this is also only a search-order change, and a
positive still emits a full exact tree.  Append `-dc-kernel` to any of these variants and pass a
checked `dc_kernel_certificate` path before the state to use its upward-substate closure as a sound
projected rejection cache.  The ordinary recursion automatically uses complete products for
two-part states through depth three and for small supply-tight states where that order is cheaper.
`check_dc_tree_lift.py --projected-only` skips exact lifts and is the appropriate mode for discovering
a larger-normalization two-coordinate kernel.  The mixed-supply control used by the regression is

```
/tmp/search_atom_profiles32 profile-state 3 \
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB:1 \
    AAAAAAAAAAAAAAAAAAAAAAAAAAAABBCC:3
```

It is rejected immediately with supply upper bound `(0,2,11)` below the height-4 requirement
`(0,2,13)`.  This is a depth-three obstruction only.

The regression also exhausts the unique 32-atom boundary state

```
/tmp/search_atom_profiles32 profile-state-flat 3 \
    AAAAAAAAAAAAAAAAAAAAAAAAAAABBBBC:1 \
    AAAAAAAAAAAAAAAAAAAAAAABBBBBBBCC:2 \
    AAAAAAAAAAAAAAAAAAAAAAAAAAACCCDD:3
```

and expects `answer=NO`.  Its root supply bound itself passes; the negative comes from exhausting
the loss-constrained complete product, so it proves only that no aligned tree of depth at most three
exists.  The independent replay is

```
tools/check_dc_tree_lift.py --atoms 32 --rank 1180 \
    --all-skeletons --depth 3 --expect NO
```

The corresponding depth-four first transition must satisfy
the stronger frontier `ell_D=ell_V=0`, `1<=ell_W<=14`.  It can be reproduced without attempting
all mixed children:

```
/tmp/search_atom_profiles32 profile-state-pure-frontier-dc-kernel 4 \
    evidence/atom_profile_height6_dc32.cert \
    AAAAAAAAAAAAAAAAAAAAAAAAAAABBBBC:1 \
    AAAAAAAAAAAAAAAAAAAAAAABBBBBBBCC:2 \
    AAAAAAAAAAAAAAAAAAAAAAAAAAACCCDD:3
tools/check_dc_tree_lift.py --atoms 32 --rank 1180 --depth 4 \
    --pure-frontier --close-positive-v-loss
```

Both implementations start from 7,266 symbolically filtered oriented tests.  Exact solution of the
two pure children leaves 6,712 tests and 1,826 distinct mixed children; exact rejection of the
eight positive-`V`-loss children leaves 6,696 tests and 1,818 children in the fourteen W-loss
classes.  `profile-state-cover[-dc-kernel]` continues from the same materialized root hypergraph,
choosing common pure children first and then mixed children in decreasing retained `(D,V,W)`
supply.  `profile-state-cover-guided[-dc-kernel]` uses the projected-skeleton exact recursion for
those mixed children.  A long closure can be resumed by restricting the materialized root to
`ell_D=ell_V=0` and an inclusive W-loss interval:

```
/tmp/search_atom_profiles32 profile-state-cover-guided-w-range-dc-kernel 4 \
    evidence/atom_profile_height6_dc32.cert 3 14 \
    AAAAAAAAAAAAAAAAAAAAAAAAAAABBBBC:1 \
    AAAAAAAAAAAAAAAAAAAAAAABBBBBBBCC:2 \
    AAAAAAAAAAAAAAAAAAAAAAAAAAACCCDD:3
```

A negative from this mode is printed as `atom_profile_cover_slice` with
`scope=declared_root_loss_slice_only`; it is not a full-state verdict.  A positive is a genuine
full strategy and is printed with its exact tree.  `profile-state-pure-frontier` stops deliberately
after the pure outcomes and prints `mixed_outcome=UNRESOLVED`; its successful process exit is a
frontier report, not a positive construction.

The archived `rank1180-depth4-2026-08-15` runs cover `ell_W=1..14` without gaps: 6,696 oriented
tests and 1,818 distinct mixed children, all exact negative.  Together with the pure-frontier
calculation this proves that rank 1180 has no aligned tree of depth at most four.  Use
`tools/check_atom_profile_cover_log.py` with the logged interval and expected counts to verify each
slice; the checker reconstructs losses from the atom words and requires the scoped final verdict.
It deliberately does not turn this bounded result into an all-depth claim.

FAST replay is deliberately an instrumented build. Each target runs in a forked child, so cache writes,
split-table initialization and `s[FAST]=1` learning disappear with the child and the next target sees
the identical parent image:

```
tools/build_radio.py -O3 -DMAX_K=5 -DMAX_N=128 -DMEASURE_FAST_REPLAY -DNO_FAST_LEARN \
      tools/fast_replay.c -o /tmp/fast_replay
/tmp/fast_replay /tmp/warm_k4.txt /tmp/warm_k5.txt 5 7 + 17 200
```

The final arguments are target level, part count, expected sign, deterministic input stride and case
limit. The facts file uses parsed-cache `+ b ... t ... k` / `- b ... t ... k` lines. This is a search
heuristic benchmark, not independent evidence for any verdict.
