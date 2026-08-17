# Working in this repo

Research code for two-defective quantity group testing: how many coins can be resolved in
`k` tests when each test reports 0, 1 or 2 defectives in the tested subset.

This file is the canonical agent brief, in the vendor-neutral `AGENTS.md` location.
`CLAUDE.md` is a symlink to it. To support another tool, add another symlink — do not copy
the content, or the copies will diverge, which is the exact failure this repo is organised
against.

## Start here, in this order

1. **[docs/status.md](docs/status.md)** — where everything stands right now, and the active
   traps. Read this even for a small task; several of the traps silently invalidate work.
2. **[docs/research-plan.md](docs/research-plan.md)** — what to work on and why.
3. [docs/problem.md](docs/problem.md) — only if the notation `Sa(n)` / `Sb(n1:m1, …)` is
   unfamiliar. [docs/README.md](docs/README.md) is the full map.

Then, before changing anything:

```
tools/check_tables.py                    # facts, formulas, generated blocks, sources
tools/check_witness.py witnesses/*.tree  # re-derive every proof from first principles
```

Both are seconds and dependency-free. If either is red on arrival, fix that first and say so.

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

The finite search budget is a shared absolute bound, not permission for a descendant to manufacture
more work.  The default clock is deterministic: one unit per accepted split prefix across the whole
recursive tree, calibrated at 20,000,000 units per nominal second.  A finite state may return `MAYBE`
with zero new cache facts; exhaustive pass 2 must remain finite.  One- and two-segment states keep the
shared parent budget because they form the reliable constructive spine; longer states start with a
40,000,000-unit speculative-child quantum and double it after an unresolved full pass.  This monotone
allowance prevents same-budget spinning without storing a preferred split.  Compile with
`-DRADIO_CPU_BUDGET` only for a controlled comparison with the historical process-CPU scheduler.
Determinism here assumes the same binary, query and cache history; FAST promotion makes history part
of the input even though hardware speed no longer chooses the finite stopping point.
`tools/deadline_regression.c` and `tools/work_budget_regression.sh` lock the invariants (revised
2026-08-13); measured context is in `evidence/deadline_stall_2026-08-10.txt` and
`evidence/work_budget_rb_root_2026-08-13.txt`.

## Be skeptical of what you read here

Prior documents, spreadsheets and outputs are *claims*, including this file. In one session
of checking, the tab labelled `INCORRECT pareto` turned out to hold the only correct copy of
the table, two unlabelled copies were stale, and a hand-typed `409?` had no derivation behind
it at all. Verify before relying, and when something does not reconcile, say so rather than
picking whichever source is more convenient.

## Building and running

No build system. Each driver is one compiler invocation; `MAX_K` and `MAX_N` size static tables at
compile time and must match the problem. Always invoke the compiler through the provenance builder:

```
tools/build_radio.py -O3 -DMAX_K=<k> -DMAX_N=<n1+n2> <driver>.c -o <driver>
```

It embeds the commit, exact compiler arguments, compiler identity and source hashes in the binary;
`radiobase.c` adds exact run arguments and a safe execution-environment fingerprint to stdout before
initialization. It also writes `<driver>.provenance`. A direct compiler invocation still works, but
its output says `provenance_complete=no` and is not durable evidence. Validate retained output with
`tools/check_provenance.py <log>`. Standalone utilities which do not include `radiobase.c` must run
through `tools/run_with_provenance.py`.

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

**Cap memory, not just time, and keep a live inventory.** `radio_canon_search_generic`
allocates 1.26 GB per pool chunk and never frees one (see [docs/tools.md](docs/tools.md)); an
exhausting search grows without bound. Run it as

```
tools/build_radio.py -O3 -DMAX_K=9 -DMAX_N=485 -DMAX_STATE_SIZE=1024 \
      -DMAX_TREE_NODES=400000 -DMAX_MEMO=4000000 radio_canon_search_generic.c -o canon
( ulimit -t 900; ./canon 3 9 473 6 )
```

Bound memory at **compile time**, not with `ulimit`: `ulimit -v` is unsupported on macOS and
fails silently, and `timeout` is not installed either — `ulimit -t` (CPU seconds) is the one
portable runtime bound. `MAX_TREE_NODES x sizeof(TreeNode) + MAX_MEMO x 192` is the real
ceiling; the numbers above give about 5.7 GB.

When the pool is exhausted the search prints `out of nodes` and exits 2. **That is an abort,
not a proof of absence** — do not read it as `NO_CANONICAL_TREE`.

Compile-time bounds only exist for `radio_canon_search_generic`. The other drivers grow an
unbounded result-cache trie, and `ulimit -t` caps CPU, not memory — so for those, wrap the run:

```
tools/capped_run.sh --seconds 3600 --rss-gb 16 --label sa113 -- ./radio_k9 > out.txt
```

It enforces the wall-clock cap and a **resident-set** cap, reports peak RSS and wall time, and
exits 124 on timeout / 137 on memory kill.  That is an effective heap bound on the Linux/AWS
runs where the anonymous heap stays resident.  It is **not** a total-memory bound on macOS:
swapped anonymous pages disappear from RSS.  For a long local run use
`tools/sa193_local_supervisor.sh`, whose guard reads macOS `top`'s documented physical-footprint
field, plus `vm_stat` swap activity; see the active trap in [docs/status.md](docs/status.md).
`vmmap -summary` is useful for one-off attribution but can itself hang for many minutes on a busy
solver, so never make an unbounded `vmmap` call the live guard.  Use the wrapper for anything you
would otherwise leave unattended, but do not mistake its macOS RSS cap for an allocation cap.

Run these one at a time, and `pgrep -f radio_canon` before launching another. On 2026-08-03 three
concurrent runs reached 28+21+12 GB and pushed the machine into heavy swap, because two of them
were leftovers I had stopped tracking. **Before launching anything, list what is already
running; before finishing a turn, account for every process you started.**

A yielded tool cell is not proof that every child it launched has exited.  On macOS, stdin Python
searches can appear as `Python -`, not `python3 -`; one such orphan consumed about 65 CPU minutes on
2026-08-16 before it was noticed.  Include the actual utility name and both Python spellings in the
final process inventory whenever a one-off search script was used.

## Artifacts

Solver output is large and is **never committed**. The store is the private repo
`fedork/radio-data`, as tagged releases; the index is [docs/data.md](docs/data.md).

```
tools/artifacts.sh list | show <tag> | push <tag> <file>... | pull <tag> [dest] | verify <tag>
```

- Over ~1 MB: `push` it and add a row to [docs/data.md](docs/data.md). A tag cited as a
  `source` in `data/*.csv` must exist.
- **Archive before deleting.** The K=9 Pareto walk was lost exactly this way.
- Archive raw output, not `parse_out.sh` output. The distilled form drops the `with [...]`
  witnesses, which are what witness-tree reconstruction needs later.
- New raw solver output must pass `tools/check_provenance.py` before archival. `artifacts.sh push`
  enforces this; its legacy override is only for a documented pre-banner historical artifact.
- **Not everything deserves archiving.** Logs from a build known to emit false negatives are
  not evidence; extract what is verifiable and let the bulk sit on local storage. See the
  archiving decision in [docs/data.md](docs/data.md).

This repo is owned by the GitHub account `fedork`, not the machine's default login. Both
git and `gh` are pointed there per repo, without touching global settings: `git` via
`core.sshCommand` in `.git/config`, and `gh` via an isolated `GH_CONFIG_DIR` at `.gh/`.
`tools/artifacts.sh` handles the latter itself. Details in [docs/data.md](docs/data.md).
Do not run `gh auth switch` - it changes the global active account.

## Git

- `*.txt` is ignored, deliberately. `witnesses/*.tree` and `data/**/*.csv` are explicitly
  re-admitted - if you add a category of small, valuable file, re-admit it too, and say why
  in the same commit.
- Keep the primary repo checkout on `main`; do not switch branches in it. When isolation is useful,
  work in a per-chat scratch worktree or clone based on current `origin/main`, then integrate the
  coherent, checked result directly to `main`. Push to `main` when the work is ready and fetch from
  `origin/main` when convenient; never include unrelated working-tree changes.
- Commit in reviewable chunks. Explain *why* in the message; the diff already shows what.

## Before you finish a session

The next session starts cold and sees only this file. Everything it needs must be written
down, or the work is half-done.

1. **Journal what was learned** — append a dated entry to
   [docs/journal.md](docs/journal.md): findings, decisions *and the reason for them*,
   retractions, and dead ends. A failed or abandoned attempt is a result; record it with its
   **measured cost**, or someone will spend the same weeks again.
2. **Refresh [docs/status.md](docs/status.md)** — it is the first thing the next session
   reads. Update the state of each goal and, especially, the trap list.
3. **Re-read your own changes for statements that are now stale.** This is the failure mode
   that recurs most: in one session three separate documents were left asserting things the
   same session had disproved, despite the supersede-in-place rule being written down. New
   knowledge invalidates old sentences somewhere else — go and find them.
4. **Run both checks**, plus `tools/check_docs.py`, and commit.

Cost figures worth having to hand, because they set the scale of what is worth proposing:
the suspect 2023 `Sa(193)` run took ~47 solve-days in ~90 GB of virtual memory; the proof-safe
2026 cold run took 419,353.1 CPU seconds (4.85 days) and 1.32 GB peak RSS. The K=9 near-diagonal walk
took 14 months of wall clock to move from `m=96` to `m≈81`; a single wrong verdict,
`Sb(143:17)`, consumed 4 days. Canonical searches, by contrast, run in minutes.

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

## Log-era reliability (added 2026-08-02)

Solver output is not uniformly trustworthy, and the difference is by *era*, not by how
expensive the run looks.

- **2026 artifacts** (`out_k7.txt`, `out_radio_1.txt`, `out_k8.txt`): audited clean, 0
  contradictions in 2,723 single-part verdicts. These back the K<=8 table.
- **2023 corpus** (everything in `radio.zip`): **37 provably false negatives**, ~0.27% of its
  single-part negatives. Recorded in `evidence/refuted_2023_negatives.txt`. It includes the
  `K=8, m=10..17` band that the 2026-05-12 recomputation corrected.

There is **no syntactic marker** separating good from bad. `Sb(143:17)` in 8 was declared
unsolvable after 10 passes and 4 days of exhaustive search, and is wrong. Cheap
`fast_solve=1, totalsplits=0` lines are *mostly* the bad ones but 569 such lines are correct,
so the signature is useless as a filter.

Practical consequence: **run `tools/extract_evidence.py audit <file>` before trusting any
unfamiliar log**, and never promote a 2023-era negative above `legacy`. Positive results are
different - a witness tree can be re-verified from first principles, so achievability claims
stand on their own regardless of era.
