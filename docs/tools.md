# The solver and its drivers

## Layout

`radiobase.c` (~1560 lines) is the entire engine. Every other `.c` at the repo root is a
`main()` wrapped around it, selected by `#include "radiobase.c"`. There is no build system;
each driver is a single `clang` invocation.

```
clang -O3 -DMAX_K=<k> -DMAX_N=<n> <driver>.c -o <driver>
```

`MAX_K` and `MAX_N` size static tables at compile time, so **they must be set for the
problem you are running**. `MAX_N` is the largest total coin count `n1 + n2` any state will
reach, not the largest single group. Two wrapper scripts compute both for you and are the
preferred entry points.

## Engine internals worth knowing

| what | where |
|---|---|
| `(n1:n2)` pair -> `sbb` integer id; split tables built lazily | `init()`, `ensure_splits()` at `radiobase.c:1227` |
| Four stored split orderings (`BY_SP0/1/2`, `BY_MAGIC3`); the `_DESC` three are derived by reversed subscript | `ensure_splits`, `ORDER_BASE` |
| Result cache: a trie over sorted `sbb` ids, closed downward for can-solve and upward for can't-solve | `cacheCanSolve` `:206`, `cacheCantSolve` `:276` |
| Main search: tri-state `TRUE`/`FALSE`/`MAYBE`, FAST-restricted first pass, exhaustive second pass, deadlines | `canSolveB` |
| Unit-group stripping before search | `radiobase.c:538` |
| Exact decision for singleton states via majorization against `G_k` | `radiobase.c:464-506` |
| `Sa` recursion | `canSolveA` `:1011` |
| Enumerate *all* top-level splits plus a solvability matrix | `all_solutions` `:1295` |
| Warm the cache from a previous run's parsed output | `parse_file` `:1431` |

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
clang -O3 -DMAX_K=9 -DMAX_N=485 -DMAX_STATE_SIZE=1024 radio_canon_search_generic.c -o canon
( ulimit -v 6000000; ulimit -t 900; ./canon 3 9 473 6 )
```

`MAX_STATE_SIZE`, `MAX_TREE_NODES` and `MAX_MEMO` are now `-D`-overridable. A state at depth
`d` has at most `2^(k-d)` parts, so 1024 is generous for `k <= 9` and cuts a node from 61 KB to
12 KB. Note `timeout` is not installed here; `ulimit -t` is the portable CPU-seconds bound.

### Capping the other drivers

Those compile-time bounds exist only in `radio_canon_search_generic`. `radio`, `radio_one`,
`radio_full` and the Pareto walkers grow an unbounded result-cache trie instead — the 2023
`Sa(193)` run reached ~90 GB that way — and `ulimit -t` bounds CPU, not memory. For those:

```
tools/capped_run.sh --seconds 3600 --rss-gb 16 --label sa113 -- ./radio_k9 > out.txt
```

Enforces a wall-clock cap and an RSS cap, reports peak RSS and wall time on stderr (so it
survives redirecting the driver's stdout), and exits 124 on timeout / 137 on memory kill.
Wall time is quantised to `--poll` (default 5s); the drivers' own `took N` lines are exact.

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
  of a line is not.
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
that does not depend on the solver being correct; a solver log is not.

## Split-heuristic research tools

The 2026-08-09 long-state experiments use exact solvability of small child subsets. These are lab
tools, not yet part of `radiobase.c`:

| tool | purpose |
|---|---|
| `tools/tripletab.c` | build every solver-solvable three-part state at one child level, gated by the exact pair table |
| `tools/quadtab.c` | build every solver-solvable four-part state, gated by pair and triple tables |
| `tools/subset_census.c` | count per-part, pair and triple survivors on complete k=5 four-part state lists |
| `tools/filter_triples.c` | apply triple and optional quad tables to an existing split-feature/label dataset |
| `tools/label_split_features.c` | join `WIN ... state=... take=...` logs to feature rows without relying on row samples |
| `tools/sample_subsets.c` | sample pair/triple rejection and lookup cost on k=6 states too large to enumerate |

Example table builds (the stated `MAX_N` includes all parts in a table entry):

```
clang -O3 -DMAX_K=4 -DMAX_N=96 tools/tripletab.c -o /tmp/tripletab4
/tmp/tripletab4 4 16 pairs_k4.txt > /tmp/triples_k4.txt

clang -O3 -DMAX_K=4 -DMAX_N=128 tools/quadtab.c -o /tmp/quadtab4
/tmp/quadtab4 4 16 pairs_k4.txt /tmp/triples_k4.txt > /tmp/quads_k4.txt

clang -O3 -DMAX_K=5 -DMAX_N=100 tools/tripletab.c -o /tmp/tripletab5
/tmp/tripletab5 5 32 pairs_k5.txt > /tmp/triples_k5.txt
```

The tables' positive and negative entries are exhaustive according to the current C solver. That is
enough for a fallback-safe heuristic experiment, but it is not an independent certificate. Do not
let a table negative prune an exhaustive proof search until the table has an adequate audit.
