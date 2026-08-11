# The `Sa(193)` AWS run — how to check on it, and how to stop it

This page is the operational handle for the cold runs hosted on the dedicated instance; the
findings go to [journal.md](journal.md) as usual.

## What is running

| | |
|---|---|
| instance | `i-0005d74f985c52ae1`, `r7iz.4xlarge` (16 vCPU, 128 GB), us-west-2b, on-demand |
| active solvers | `run3`: live incumbent; `run7`: diagnosed-obsolete progress-gated scheduler at `e648e83`, retained pending archival; both `MAX_K=10 MAX_N=193` |
| account | 393287594714 (shared production — everything tagged `Project=radio-sa193`) |
| bucket | `s3://radio-sa193-393287594714/` |
| notifications | SNS `radio-sa193-progress` -> fedor@retellai.com |
| memory guards | `run3` 40 GiB + `run7` 60 GiB = 100 GiB on the 123 GiB host |
| completion | a 20-minute final-upload grace period, then instance-initiated stop once both solvers are gone |

Each run remains internally serialized: one process and cache handle all sixteen top-level states
in sequence.  The two builds run side by side on separate cores and have separate directories,
binary names, watchdogs and S3 prefixes; none loads another run's cache. Run7 was launched as a
matched-host comparison, but its scheduler is now known to stall, so its timing is diagnostic rather
than a performance baseline.

| prefix | build | started UTC | state / binary |
|---|---|---|---|
| `run3/` | A+B + full-star majorization (`3cf1406`) | 2026-08-10 00:00:04 | active; `/root/run3/radio_sa193_v3` |
| `run4/` | level-lazy tables + compact last-segment Pareto cache (`6af384e`) | 2026-08-11 01:37:20 | stopped; old scheduler; `/root/run4/radio_sa193_v4` |
| `run5/` | compact cache + bounded exact-state L1 (`290a892`) | 2026-08-11 05:05:13 | stopped; old scheduler; `/root/run5/radio_sa193_v5` |
| `run6/` | broken zero-progress deadline/prefix-poll experiment (`c13b5d3`) | 2026-08-11 05:45:09 | stopped and archived; `/root/run6/radio_sa193_v6` |
| `run7/` | progress-gated pass-2 `NO_DEADLINE` handoff (`e648e83`) | 2026-08-11 15:45:30 | active but obsolete; `/root/run7/radio_sa193_v7` |

All rows name frozen binaries, not aliases for current `main`. The original run4/run5 snapshots did
not by themselves prove a livelock, but run7 later reproduced the same information-tight 14-part
child beneath a finite parent and remained there for 19,859 CPU seconds after 270.7 billion admitted
prefixes. Its pass-2 `NO_DEADLINE` handoff and negative-progress gate remove the finite escape when
the compact cache adds no fact. `c13b5d3` represents the opposite bad extreme: it could poll before a
complete child was tried. Definitive printed verdicts from both builds remain sound, but neither is a
timing baseline. The exact frames, final state machine, regression hashes and disposition are in
[`../evidence/deadline_stall_2026-08-10.txt`](../evidence/deadline_stall_2026-08-10.txt).
Archive run7's current raw segment and checkpoint before stopping it. A replacement should use
`45c34fd` or later, but starting another multi-hour run requires an explicit check-in.

The predecessor run (2026-08-03, `i-0b8ca7169585b7cc1`) failed — deadlines had been removed and it
sank 43 minutes into one 13-part k=5 node — and was terminated.

### Following it without logging in

`tools/sa193_watchdog.sh` emails on every milestone plus a 6-hour heartbeat. The progress metric is
the only one that means anything here: `Sa(193)` in 10 is unsolvable **iff all sixteen**
`Sb(n1:193-n1)` fail in 9, for `n1 = 97..112`, so each is 1/16 of the job and each prints a line.
Verdict counts and elapsed time are not progress; "3 of 16" is.

Emails fire on: run started, another of the sixteen done, the control reporting, the final answer,
the solver process dying, and every 6 hours otherwise. Every AWS call in the watchdog is
failure-tolerant — a broken report must never kill the run.

### Why the control runs first

`radio_sa193` asks `Sa(192)` in 10 before `Sa(193)`, and **aborts** if it does not come back
solvable. `Sa(192)` has a verified witness tree, so this catches a broken engine before the negative
is produced — which is not a formality, given the 2023 corpus holds 37 provably false negatives with
no syntactic marker, and given an engine change trapped the last run. It is also not wasted work:
`Sa(192)` and `Sa(193)` share almost everything, so the control warms the cache for the real query.

## Checking on it

```
tools/sa193_status.sh --compare --watch
```

This prints active `run3` and `run7` together.  `--prefix run7` selects only the diagnosed old run and
`--all` also includes the stopped/historical prefixes.  Each live `STATUS` gets refreshed every 10 minutes
with verdict count, cache size and RSS.  Also in the bucket:

| key | what |
|---|---|
| `run3/STATUS`, `run7/STATUS` | latest active status snapshots |
| `runN/sa193.checkpoint` | same-run restart checkpoint, refreshed hourly |
| `runN/seg-*/out_sa193.txt.zst` | immutable per-segment raw log, refreshed hourly and finalized at exit |
| `runN/seg-*/memprofile.csv` | time/RSS/verdict profile used for the comparison |
| `bench/r7iz.4xlarge.txt` | the k=9 ladder benchmark used to size this |

Is it alive?

```
aws-vault exec default -- aws ec2 describe-instances \
  --filters Name=tag:Project,Values=radio-sa193 Name=instance-state-name,Values=running \
  --query 'Reservations[].Instances[].[InstanceId,InstanceType,LaunchTime]' --output text
```

## Stopping it

Do not terminate the instance while any cold session is valuable.  To stop it while preserving
the EBS volume and all run directories:

```
aws-vault exec default -- aws ec2 stop-instances --instance-ids i-0005d74f985c52ae1
```

The active idle guard does this automatically only after both named solvers have gone, then
waits 20 minutes so their watchdogs can finalize S3 artifacts.  A manual stop is an interruption,
not a proof; S3 checkpoints are at most about one hour stale and the full EBS logs remain intact.

## Resuming

Relaunch the desired build and pass **that prefix's own** checkpoint as the driver's leading argument.
**The checkpoint is sound to warm-start a negative from**, unlike `cache-2025:parsed_260.txt`
— see the trap in [status.md](status.md). Every checkpoint carries a header naming the build,
the state and the generation time, and `parse_file` skips `#` lines, so the two can never be
confused. Only warm-start from a file that has that header.

## Why these numbers

- **r7iz.4xlarge.** The Graviton option (`x2gd`, half the price for the same 128 GB) is
  unavailable: the ARM vCPU quota on this account is **0**. The x86 quota is 5000 with ~1372 in
  use, so there is room to go bigger if memory demands it.
- **128 GB.** The pre-compact engine measured 2.32 KB of trie per insert at this geometry; the 2023 run
  reached ~90 GB. Note this also bounds how large a warm cache can be loaded: the filtered
  `out_k8.txt` facts that could inject into `Sb(74:40, 41:38)` number 11,375,981, which at that
  rate is ~25 GB of trie before the search starts. Filter harder, or size the instance for it.
  The current engine visits about half as many states, so the estimate is 40–60 GB with 90 GB
  pessimistic.  The active comparison caps the two concurrent solvers at 40 + 60 GiB, which
  reserves about 23 GiB for the OS and file cache even if both reach their caps.
- **On-demand, not spot.** Spot is ~$0.54/hr against $1.49, but the run is single-threaded and
  we are paying for RAM; the saving is not worth interruption handling on an unattended run.
- **AWS is slower than the laptop.** The k=9 ladder takes 391 s on r7iz against 261 s on the
  M4 Pro. We are here for the 128 GB, not the cores.
