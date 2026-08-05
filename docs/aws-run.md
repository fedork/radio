# The `Sa(193)` AWS run — how to check on it, and how to stop it

Started 2026-08-03 from commit `69ae856`. This page is the operational handle; the findings go
to [journal.md](journal.md) as usual.

## Nothing is running

The 2026-08-03 run (`i-0b8ca7169585b7cc1`, `r7iz.4xlarge`) **failed and was terminated**. It sank
43 minutes into a single 13-part k=5 node of mass 243 because deadlines had been removed; that
change was reverted the same day. Confirmed 2026-08-05: no instance carries the
`Project=radio-sa193` tag in any state, so nothing is billing.

## What the next run is for — and it is not the old job

The old plan was to re-prove `Sa(193)` from scratch: sixteen `Sb(n1:193-n1)` at k=9, ~47 days of
solve time. **That is no longer the job.** The 2023 corpus turns out to be nearly checkable
(2026-08-04, see [certificate.md](certificate.md)):

- all sixteen k=9 facts are present, and each fails on **exactly one split**;
- the survivor is always the near-balanced one, whose two single-part children are both *solvable*
  by the proven Pareto table, so each root needs exactly one **two-part k=8 fact** — for
  `Sb(112:81)` that is `Sb(74:40, 41:38)`;
- with top-down painting the k=7 level is 16,347 reachable facts, not 3,098,762 — order 100-300
  core-hours rather than 6.3 core-years.

So the next run has two parts, and they are independent:

1. **Prove the sixteen k=8 two-part facts.** The only genuinely new compute. Sixteen independent
   jobs, one per root, `radio_one <cache> 8 74 40 41 38` and its fifteen siblings. Cold is
   hopeless — even the single part `Sb(74:41)` at k=8 does not resolve in 10 minutes — so each
   needs a warm start. `out_k8.txt` is the right source: 2026-era, audited clean, and the
   warm-start prohibition is specific to `cache-2025:parsed_260.txt`. **Unsized as of 2026-08-05**;
   size one before provisioning sixteen.
2. **Verify the painted sub-DAG.** `radio_verify`, `TOPDOWN=9`, embarrassingly parallel across
   facts and across levels, so this wants cores rather than the 128 GB the old plan needed.

The instance shape below was chosen for (1)'s memory profile and is still right for it. For (2),
prefer many vCPUs; the verifier's resident set is one level, not the certificate.

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
