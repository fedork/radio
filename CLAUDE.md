# Working in this repo

Research code for two-defective quantity group testing: how many coins can be resolved in
`k` tests when each test reports 0, 1 or 2 defectives in the tested subset. Read
[docs/problem.md](docs/problem.md) first if the notation `Sa(n)` / `Sb(n1:m1, ...)` is not
already familiar.

**[docs/README.md](docs/README.md) is the map. [docs/research-plan.md](docs/research-plan.md)
says what to work on.**

## The one rule that matters

> Every number is a claim. Claims carry a status and a source, or they do not get written
> down.

This repo has already shipped two errors from ignoring it: a Pareto column that went stale
in three of its four copies, and a lemma whose `k(k-1)/2` became `k(k-5)/2` in transcription,
asserting that unsolvable states were solvable. Both are now caught by
`tools/check_tables.py`. The discipline below exists to keep it that way.

## Facts

`data/pareto_sb.csv`, `data/pareto_sa.csv` and `data/conjectures.csv` are the single source
of truth. Status vocabulary and column meanings: [docs/README.md](docs/README.md).

- **Never copy a table into prose.** Reference the CSV. If a table must be readable inline,
  wrap it in `<!-- generated:NAME -->` / `<!-- /generated -->` and produce it with
  `tools/check_tables.py --render`.
- **Distinguish `max` from `lower`.** "473 is solvable in 9" and "473 is the maximum at
  k=9, m=6" are different claims and only the first is established.
- **Supersede in place.** Replace a wrong statement; never leave it beside its correction.

## Evidence

Prefer, in order:

1. A **canonical witness tree** whose leaves are sub-multisets of `G_k`. This is a proof
   resting on the Singleton Majorization Theorem alone - the solver could be wrong and the
   result would stand. Produce with `radio_canon_search_generic`, verify with
   `tools/check_witness.py`, commit to `witnesses/`.
2. A **solver log line pair** - `can solve Sb(n:m) in k` plus `can't solve Sb(n+1:m) in k` -
   located in a retained, archived artifact.
3. A solver run whose output was not kept. This is worth little; it is how the K=9 frontier
   was lost.

Run `tools/check_witness.py` on any new tree **before** recording the result. Run
`tools/check_tables.py` after touching anything in `data/` or the docs.

Note that a missing `can't solve` line does **not** mean unsolvable. `canSolveB` returns a
tri-state and gives up with `MAYBE` when it hits a deadline, printing nothing.

## Be skeptical of what you read here

Prior documents, spreadsheets and outputs are *claims*, including this file. In one session
of checking, the tab labelled `INCORRECT pareto` turned out to hold the only correct copy of
the table, two unlabelled copies were stale, and a hand-typed `409?` had no derivation behind
it at all. Verify before relying, and when something does not reconcile, say so rather than
picking whichever source is more convenient.

## Building and running

No build system. Each driver is one `clang` invocation; `MAX_K` and `MAX_N` size static
tables at compile time and must match the problem.

```
clang -O3 -DMAX_K=<k> -DMAX_N=<n1+n2> <driver>.c -o <driver>
```

Prefer the wrappers, which compute both and compile for you:

```
./run_radio_canon_search_generic.sh <target_k> <k> <n1> <m> ...   # start here for k=9 work
./run_radio_full.sh [cache] <k> <n1> <m> ...                      # much more expensive
```

Driver selection, engine internals, and cache files: [docs/tools.md](docs/tools.md). The
short version - reach for `radio_canon_search_generic` first, because it yields a
checkable proof and is far cheaper than `radio_full`, which enumerates every top-level
split and was responsible for several runs that were killed without a verdict.

## Long-running jobs

Background runs with an explicit time cap (30-60 minutes) are fine without asking; report
what happened, including cost and where it stalled. Anything longer, check in first. A
search that finds nothing is a result worth recording in
[docs/journal.md](docs/journal.md) along with its cost - otherwise it gets attempted again.

## Artifacts

Solver output is large and is **never committed**. The corpus is ~2.1 GB raw, ~9% of that
under `zstd -19`.

- Over ~1 MB: archive with `tools/artifacts.sh push <tag> <file>` and add a row to
  [docs/data.md](docs/data.md).
- **Archive before deleting.** The K=9 Pareto walk was lost exactly this way, and
  `run_pareto9.sh` cannot restart without it.
- Archive raw output, not `parse_out.sh` output. The distilled form drops the `with [...]`
  witnesses, which are what witness-tree reconstruction needs later.

This repo is owned by the GitHub account `fedork`, not the machine's default login. Both
git and `gh` are pointed there per repo, without touching global settings: `git` via
`core.sshCommand` in `.git/config`, and `gh` via an isolated `GH_CONFIG_DIR` at `.gh/`.
`tools/artifacts.sh` handles the latter itself. Details in [docs/data.md](docs/data.md).
Do not run `gh auth switch` - it changes the global active account.

## Git

- `*.txt` is ignored, deliberately. `witnesses/*.tree` and `data/**/*.csv` are explicitly
  re-admitted - if you add a category of small, valuable file, re-admit it too, and say why
  in the same commit.
- Branch off `main`; do not push without being asked.
- Commit in reviewable chunks. Explain *why* in the message; the diff already shows what.

## Keeping this file honest

This file is maintained the same way as the research record.

- When you learn something durable about how to work here - a tool that is the wrong choice,
  a check worth running, a trap in the engine - add it here, dated where the timing matters.
- When a rule here turns out to be wrong, **replace it**. Do not append a contradiction.
- When a rule stops being needed because a check now enforces it mechanically, say so and
  point at the check. Automation beats instruction.
- Keep it short enough to be read in full. If it is growing past roughly two screens, the
  detail belongs in `docs/` with a pointer from here.

Changes to this file are part of the work, not overhead: an instruction that would have
prevented a mistake is worth more than the fix for that mistake.
