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
| Ten heuristic split orderings (`BY_MAGIC3`, `BY_SP0/1/2`, descending variants) | `radiobase.c:1130-1282` |
| Result cache: a trie over sorted `sbb` ids, closed downward for can-solve and upward for can't-solve | `cacheCanSolve` `:206`, `cacheCantSolve` `:276` |
| Main search: tri-state `TRUE`/`FALSE`/`MAYBE`, deadlines, two passes (`fast_solve` then full) | `canSolveB` `:516` |
| Unit-group stripping before search | `radiobase.c:538` |
| Exact decision for singleton states via majorization against `G_k` | `radiobase.c:464-506` |
| `Sa` recursion | `canSolveA` `:1011` |
| Enumerate *all* top-level splits plus a solvability matrix | `all_solutions` `:1295` |
| Warm the cache from a previous run's parsed output | `parse_file` `:1431` |

Two behaviours to be aware of:

- **`MAYBE` is a real answer.** Deadlines cause `canSolveB` to give up and return `MAYBE`
  rather than `FALSE`. A `can't solve` line in the output is a genuine negative; the absence
  of a line is not.
- **The search self-tunes.** On success, `canSolveB` writes `s[FAST] = 1` back into the split
  table (`radiobase.c:929`, printed as `NOTFAST-ADDED`). Search order therefore differs
  between runs, and a rerun is not guaranteed to explore in the same order.

## Choosing a driver

| driver | use it for | cost |
|---|---|---|
| `radio_canon_search_generic.c` | **Prefer this for new `k=9` results.** Finds a tree that terminates in canonical `U_k` singleton states, which is a self-contained proof. Produced the `473:6`, `480:5`, `496:4` trees. | minutes |
| `radio_one.c` | One question: is this state solvable in `k`? | varies wildly |
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

Example - reproduce a committed witness:

```
./run_radio_canon_search_generic.sh 3 8 248 3     # matches witnesses/canon_248_3_at8.tree
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
