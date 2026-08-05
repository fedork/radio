# The `Sa(193)` AWS run — how to check on it, and how to stop it

Started 2026-08-03 from commit `69ae856`. This page is the operational handle; the findings go
to [journal.md](journal.md) as usual.

## What is running

Launched **2026-08-05** from commit `0a468ca`, `tools/sa193_launch.sh --days 7`.

| | |
|---|---|
| instance | `i-0005d74f985c52ae1`, `r7iz.4xlarge` (16 vCPU, 128 GB), us-west-2b, on-demand |
| what | `radio_sa193`, `MAX_K=10 MAX_N=193` — the `Sa(192)` control, then `canSolveA(193, 10)` |
| account | 393287594714 (shared production — everything tagged `Project=radio-sa193`) |
| bucket | `s3://radio-sa193-393287594714/` |
| notifications | SNS `radio-sa193-progress` -> fedor@retellai.com |
| cap | 7 days, self-terminating; ~$250 of instance time |

**Serialized on purpose.** One process, one cache, all sixteen top-level states in sequence. Sixteen
parallel cold jobs would each rebuild the shared low-k work; the 2023 run's later states were only
affordable because of that reuse.

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
aws-vault exec default -- aws s3 cp s3://radio-sa193-393287594714/run/STATUS -
```

`STATUS` gets a line every 10 minutes with verdict count, cache size and RSS. Also in the bucket:

| key | what |
|---|---|
| `run/STATUS` | progress log, appended every 10 min |
| `run/112_80.cache`, `run/112_81.cache` | checkpoints, refreshed every 10 min |
| `run/out_112_8*.txt.zst` | raw logs, refreshed hourly and at the end |
| `bench/r7iz.4xlarge.txt` | the k=9 ladder benchmark used to size this |

Is it alive?

```
aws-vault exec default -- aws ec2 describe-instances \
  --filters Name=tag:Project,Values=radio-sa193 Name=instance-state-name,Values=running \
  --query 'Reservations[].Instances[].[InstanceId,InstanceType,LaunchTime]' --output text
```

## Stopping it

```
aws-vault exec default -- aws ec2 terminate-instances --instance-ids i-0b8ca7169585b7cc1
```

Safe at any time — the checkpoint in S3 is at most 10 minutes stale, and a restart re-runs only
the top-level call while every completed sub-state is a cache hit.

## Resuming

Relaunch with the same user-data and pass the checkpoint as `radio_one`'s leading argument.
**The checkpoint is sound to warm-start a negative from**, unlike `cache-2025:parsed_260.txt`
— see the trap in [status.md](status.md). Every checkpoint carries a header naming the build,
the state and the generation time, and `parse_file` skips `#` lines, so the two can never be
confused. Only warm-start from a file that has that header.

## Why these numbers

- **r7iz.4xlarge.** The Graviton option (`x2gd`, half the price for the same 128 GB) is
  unavailable: the ARM vCPU quota on this account is **0**. The x86 quota is 5000 with ~1372 in
  use, so there is room to go bigger if memory demands it.
- **128 GB.** For job (1) only. Measured 2.32 KB of trie per insert at this geometry; the 2023 run
  reached ~90 GB. Note this also bounds how large a warm cache can be loaded: the filtered
  `out_k8.txt` facts that could inject into `Sb(74:40, 41:38)` number 11,375,981, which at that
  rate is ~25 GB of trie before the search starts. Filter harder, or size the instance for it.
  The current engine visits about half as many states, so the estimate is 40–60 GB with 90 GB
  pessimistic. `capped_run.sh --rss-gb 110` kills cleanly *before* the OOM killer, so a memory
  overrun preserves the checkpoint instead of losing it — resume on a larger instance.
- **On-demand, not spot.** Spot is ~$0.54/hr against $1.49, but the run is single-threaded and
  we are paying for RAM; the saving is not worth interruption handling on an unattended run.
- **AWS is slower than the laptop.** The k=9 ladder takes 391 s on r7iz against 261 s on the
  M4 Pro. We are here for the 128 GB, not the cores.
