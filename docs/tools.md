# The solver and its drivers

## Layout

`radiobase.c` is the entire engine. Every other `.c` at the repo root is a `main()` wrapped around
it, selected by `#include "radiobase.c"`. There is no build system; each driver is a single compiler
invocation made through the provenance builder.

```
tools/build_radio.py -O3 -DMAX_K=<k> -DMAX_N=<n> <driver>.c -o <driver>
```

`MAX_K` and `MAX_N` size static tables at compile time, so **they must be set for the
problem you are running**. `MAX_N` is the largest total coin count `n1 + n2` any state will
reach, not the largest single group. Two wrapper scripts compute both for you and are the
preferred entry points.

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

## Engine internals worth knowing

| what | where |
|---|---|
| `(n1:n2)` pair -> `sbb` integer id; level-keyed split tables built lazily | `init`, `ensure_splits` |
| Four stored split orderings (`BY_SP0/1/2`, `BY_MAGIC3`); the `_DESC` three are derived by reversed subscript | `ensure_splits`, `ORDER_BASE` |
| Result cache: exact-prefix trie with maximal-positive/minimal-negative Pareto fronts in its last part | `cacheCanSolve`, `cacheCantSolve`, `checkCache` |
| Main search: tri-state `TRUE`/`FALSE`/`MAYBE`, FAST/exhaustive passes, shared short-state budget and geometric long-state probes | `canSolveB` |
| Joint suffix reachability; suppression of prefix contraction once it rejects | `rb_dead`, `rb_tainted_contraction` |
| Unit-group stripping before search | start of `canSolveB` |
| Exact singleton decision plus full star-expansion majorization for every state | `singleton_majorization_can_solve`, `star_expansion_majorization_can_solve` |
| `Sa` recursion | `canSolveA` |
| Enumerate *all* top-level splits plus a solvability matrix | `all_solutions` |
| Warm the cache from a previous run's parsed output | `parse_file` |

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

- **`MAYBE` is a real answer.** Deadlines cause `canSolveB` to give up and return `MAYBE`
  rather than `FALSE`. A `can't solve` line in the output is a genuine negative; the absence
  of a line is not. A finite state may now bail without adding a cache fact, but it never gives an
  unresolved descendant `NO_DEADLINE` or refills an expired parent. One- and two-segment states
  retain the shared parent allowance; states with three or more segments give each speculative
  child an initial two-second slice. An unresolved exhaustive pass doubles that local slice, so an
  unbounded root still deepens monotonically without persisting a preferred split. Finite prefix
  enumeration polls the shared deadline every 65,536 admitted prefixes. The exact run7 failure and
  the superseded progress-gated policy are recorded in
  [`../evidence/deadline_stall_2026-08-10.txt`](../evidence/deadline_stall_2026-08-10.txt).
- **The search is not byte-deterministic.** The FAST pass was briefly removed on 2026-08-03 and
  restored on 2026-08-04 after the solver sank into known-solvable branches. On a solution,
  `canSolveB` can promote a previously non-FAST option with `s[FAST] = 1`, so later search order
  depends on which states the process has already solved. Deadlines add a second wall-clock
  dependency. Compare verdicts and exact counters on controlled warm/cold baselines; do not use a
  raw output `diff` as the only regression gate.

## Choosing a driver

| driver | use it for | cost |
|---|---|---|
| `radio_canon_search_generic.c` | **Prefer this for new `k=9` results.** Finds a tree that terminates in canonical `U_k` singleton states, which is a self-contained proof. Produced the `473:6`, `480:5`, `496:4` trees. | minutes |
| `radio_one.c` | One question: is this state solvable in `k`? | varies wildly |
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

Both derive `MAX_K` / `MAX_N` and compile before running. `run_radio_full.sh` will also scan
a cache file to size the build. `target_k` in the canonical search is the depth at which the
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

## Cache files

Solver output can be fed back in to skip work already done. `parse_out.sh` converts a raw
log into the compact form `parse_file` reads:

```
cat out_run.txt | ./parse_out.sh >> cache.txt
./radio_one cache.txt 9 432 9
```

A driver takes an optional leading cache-file argument, detected by argument-count parity.

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

**Run `check_witness.py` before recording any new result.** A tree that passes is a proof
that does not depend on the solver being correct; a solver log is not.  Recursive trees may
stop either at `[canonical U_k]` atom sub-multisets or at `[majorized G_k]` arbitrary singleton
sequences.  The checker verifies the former by multiplicity and the latter by every weak-
majorization prefix.

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
| `tools/bundled_majorization.py` | evaluate the sound depth-`d` synchronized-majorization hierarchy and compare it with a complete pair table |
| `tools/search_singletonization.cpp` | exact small-m synchronized search with arbitrary singleton-majorized terminals; scan a fixed-m frontier with memo reuse |

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

The bundled hierarchy starts at full star-expansion majorization (`R_0`), adds one synchronized
rectangle split per level, and becomes exact at `R_k`. Its small regression case and complete k=4
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
them at the requested hierarchy depth. It does not inherit any continuation from a stored
witness tree.

### Exact small-m singletonization search

`search_singletonization.cpp` is the scale-free counterpart of the hierarchy prototype for states
with a small total narrow-side multiplicity.  Define `C_d(S,k)` to hold when `S` passes full-star
majorization and either is already a singleton state, or has a legal rectangle split whose three
children satisfy `C_(d-1)`.  A singleton terminal is decided exactly by the Singleton Majorization
Theorem.  At `d=k`, this is exact solvability: any strategy supplies the recurrence, and at depth
zero full-star majorization permits at most one edge.

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
tools/run_with_provenance.py /tmp/search_singletonization forced 10 10 973 6 477 2
tools/run_with_provenance.py /tmp/search_singletonization frontier 10 6 974 973
```

The first form checks one state with a bounded number of synchronized levels.  `forced` verifies a
specified one-part root split and prints its tree.  `frontier` walks downward, retains the exact memo
between adjacent `n`, stops at the first positive, and prints its tree.  Cap frontier runs with
`tools/capped_run.sh`; a memo-limit exception or external cap is an abort, never a negative verdict.
The retained `k=10,m=6` replay and independently checked tree are
`evidence/sb_m6_k10_frontier.txt` and `witnesses/majorized_973_6_at10.tree`.

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
