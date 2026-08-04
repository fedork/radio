# The `Sa(193)` AWS run — how to check on it, and how to stop it

Started 2026-08-03 from commit `69ae856`. This page is the operational handle; the findings go
to [journal.md](journal.md) as usual.

## What is running

| | |
|---|---|
| instance | `i-0b8ca7169585b7cc1`, `r7iz.4xlarge` (16 vCPU, 128 GB), us-west-2c, **on-demand** |
| account | 393287594714 (shared production account — everything is tagged `Project=radio-sa193`) |
| bucket | `s3://radio-sa193-393287594714/` |
| IAM | role + instance profile `radio-sa193-ec2`, write access to that one bucket and nothing else |

Two states, in this order:

1. **`Sb(112:80)` in 9 — a positive control.** This is the `Sa(192)` construction, already proven
   solvable by verified witness trees. The engine changed a great deal on 2026-08-03, so if this
   does not come back `can solve` the run **stops itself** and the negative below is not to be
   trusted. Cap 12 h.
2. **`Sb(112:81)` in 9 — the actual question.** One of the sixteen states behind
   [H3](status.md). Warm-started from the control's own output. Cap 60 h.

The instance terminates itself when done, or when a cap trips. Total cap 72 h, so the run
cannot cost more than about **$107** whatever happens. That bound *is* the budget guard —
tag-filtered AWS Budgets need cost-allocation tags activated, which takes ~24 h.

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
- **128 GB.** Measured 2.32 KB of trie per insert at this geometry; the 2023 run reached ~90 GB.
  The current engine visits about half as many states, so the estimate is 40–60 GB with 90 GB
  pessimistic. `capped_run.sh --rss-gb 110` kills cleanly *before* the OOM killer, so a memory
  overrun preserves the checkpoint instead of losing it — resume on a larger instance.
- **On-demand, not spot.** Spot is ~$0.54/hr against $1.49, but the run is single-threaded and
  we are paying for RAM; the saving is not worth interruption handling on an unattended run.
- **AWS is slower than the laptop.** The k=9 ladder takes 391 s on r7iz against 261 s on the
  M4 Pro. We are here for the 128 GB, not the cores.
