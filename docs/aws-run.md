# The `Sa(193)` AWS runs — final solver record and remaining follow-ups

This page is the operational record for the completed cold runs and the remaining follow-ups on
the dedicated instance; the findings go to [journal.md](journal.md) as usual.

## What is running

| | |
|---|---|
| instance | `i-0005d74f985c52ae1`, `r7iz.4xlarge` (16 vCPU, 128 GB), us-west-2b, on-demand |
| active Sa solvers | none — run3, run8 and run9 all completed 16/16 roots |
| active jobs | `pareto_k8_aws`, the separate k=8 Pareto-prefix census; `run9_verify`, the independent run9 certificate coloring/replay |
| account | 393287594714 (shared production — everything tagged `Project=radio-sa193`) |
| bucket | `s3://radio-sa193-393287594714/` |
| notifications | SNS `radio-sa193-progress` -> fedor@retellai.com |
| active memory guards | 20 GiB for the census; 80 GiB for the verifier; neither process may consume the other's allowance |
| completion | both supervisors finalize their S3 artifacts; the idle guard may stop the instance only after `pareto_k8_aws` and `run9_verify` are gone |

Each Sa run was internally serialized: one process and cache handled all sixteen top-level states in
sequence. Every run was isolated by directory, binary, cache, raw log, watchdog and S3 prefix.
Run3/run8 are performance baselines only: their builds predate the suffix-reachability/contraction
fix. Run9 is the proof source.

| prefix | build | started UTC | state / binary |
|---|---|---|---|
| `run3/` | A+B + full-star majorization (`3cf1406`) | 2026-08-10 00:00:04 | completed; performance only; 479020.9 CPU s, 25.57 GB peak RSS |
| `run4/` | level-lazy tables + compact last-segment Pareto cache (`6af384e`) | 2026-08-11 01:37:20 | stopped; old scheduler; `/root/run4/radio_sa193_v4` |
| `run5/` | compact cache + bounded exact-state L1 (`290a892`) | 2026-08-11 05:05:13 | stopped; old scheduler; `/root/run5/radio_sa193_v5` |
| `run6/` | broken zero-progress deadline/prefix-poll experiment (`c13b5d3`) | 2026-08-11 05:45:09 | stopped and archived; `/root/run6/radio_sa193_v6` |
| `run7/` | progress-gated pass-2 `NO_DEADLINE` handoff (`e648e83`) | 2026-08-11 15:45:30 | stopped 2026-08-11 22:13:30; archived; `/root/run7/radio_sa193_v7` |
| `run8/` | compact cache + bounded long-state probes (`9395218`) | 2026-08-11 22:46:06 | completed; performance only; 412561.4 CPU s, 1.32 GB peak RSS |
| `run9/` | suppress rb-tainted implicit contractions (`e7fa747`) | 2026-08-12 03:21:12 | **completed proof source**; 419353.1 CPU s, 1.32 GB peak RSS |

At **2026-08-17 00:52 UTC**, the remaining `pareto_k8_aws` process had run for 2d23h at one
full core and 8,892,056 KiB RSS. Its 44,833,189-byte output contained all 815
`CENSUS SECOND_SUMMARY` blocks, the closed prefix summary and 1,688 targets. It had not yet emitted
an endpoint or full-state record, so the upgrade/full-state phase remains unfinished. The host had
113 GiB available, no swap and 194 GB free disk. Do not stop the instance. The S3 `STATUS` object
for this census is still the launch snapshot; inspect the live output or final artifact rather than
using that stale object as a health signal.

At **2026-08-17 16:34:44 UTC**, the independent run9 verifier launched from commit `8856509`.
It reproduced the 3,126,190-record sanitized certificate byte-for-byte and completed pre-color
antichain reduction. The dominant `k=7` level fell only from 2,576,885 to 2,507,270 facts in 713.01
seconds. Coloring then verified all sixteen `k=9` roots and all 2,151 minimal `k=8` targets, which
cited 2,506,515 `k=7` facts—99.97% of that level. It is now coloring that full-scale `k=7` batch.
Fourteen nice-level-10 workers are pinned to CPUs 0--13, leaving two vCPUs for the one-core census
and the host; both jobs retain full CPU and the host has no swap. The verifier will replay the
resulting closed bundle and upload compressed results under `run9-verifier/20260817T163700Z/`.
Follow it with
`tools/run9_verifier_status.sh`; launch hashes and the first snapshot are in
[`../evidence/run9_verifier_aws_2026-08-17.txt`](../evidence/run9_verifier_aws_2026-08-17.txt).

Final Sa sidecars that existed only on EBS were copied to immutable `run3/final/`, `run8/final/`
and `run9/final/` prefixes under SSM command `4dfc8613-78aa-4b81-a122-895e9675bf54`; the shared
checksum manifest is `final/sa193-cold-sidecars.sha256`. The public index points at the durable
private release `sa193-cold-2026-08-16`.

All rows name frozen binaries, not aliases for current `main`. The original run4/run5 snapshots did
not by themselves prove a livelock, but run7 later reproduced the same information-tight 14-part
child beneath a finite parent. Its final snapshot had spent 20,460 CPU seconds and admitted
280,116,882,707 prefixes in that activation without returning the `Sa(192)` control. Its pass-2
`NO_DEADLINE` handoff and negative-progress gate remove the finite escape when the compact cache adds
no fact. `c13b5d3` represents the opposite bad extreme: it could poll before a complete child was
tried. Those scheduler failures return `MAYBE`, but the separate pre-`75814a7` contraction bug means
their negative caches are not proof-safe either; their retained timing remains diagnostic.
The exact frames, final state machine, regression hashes and disposition are in
[`../evidence/deadline_stall_2026-08-10.txt`](../evidence/deadline_stall_2026-08-10.txt).

Run7's final raw segment is
`run7/seg-seg-20260811154530Z-e648e83/out_sa193.txt.zst`: 104,936 lines and 11,065,274 bytes after
decompression, SHA-256 `a79d31d9b11bf97679451087b90978f7fdc3b8874847bda2ebca305142ddb72c`.
The final checkpoint is `run7/sa193.checkpoint`, SHA-256
`b7e63923275caa4d486fbefd5cd912cf80d8a530c36372fbf14cece2b8cae545`. Because the run predates
embedded provenance, its exact source archive, frozen binary, `run.meta`, final memory profile,
stderr and watchdog log are retained under `run7/` too. All were streamed back and hash-verified;
the compressed source and raw-log streams also passed `zstd -t`.

Run8 is that replacement. Its embedded provenance names full commit
`9395218dcbdd90d8f6a208b15da1878ff75f6ee1`, `-O3 -DMAX_K=10 -DMAX_N=193`, no cache argument and
the enabled `Sa(192)` control. The binary SHA-256 is
`d9ae6e5feea4700be742504e345e2af09c910d790330b37457755cd89d4ac950`; the exact source archive is
`run8/source/radio-9395218.tar.zst`, SHA-256
`38837a2fb0f66036733d139301c0d2b9378e437be22e6266477fad50fb31ea69`. The source archive, frozen
binary, provenance sidecar and `run.meta` were streamed back from S3 and hash-checked immediately
after launch. Launch and independent survival checks are SSM commands
`0d7a7f49-1033-4429-844f-df87860cbe4f` and `40e28f6c-397b-422b-bb20-55463cdc347c`.
Its mandatory cold control returned `SOLVABLE` in 471.6 CPU seconds; the run then completed all
sixteen roots and returned UNSOLVABLE in 412561.4 CPU seconds. Run3's same-host control was
540.7 seconds. Run8 remains performance-only because its build predates the contraction fix.

Run9 is the proof-safe replacement, built from full commit
`e7fa747264476461a234bf78e49762ee77ad8d8d`. Commit `75814a7` within it records an invocation as
tainted once `rb_dead` actually rejects a partial assignment; an eventual full negative remains
cacheable, but the implicit shorter negative is suppressed and printed as
`contraction=rb-suppressed:<size>`. The forced regression exhibits the real failure:
`Sb(5:3,2:2,2:2,2:2)@3` is negative while the formerly inferred `Sb(5:3)@3` is positive.

Run9's embedded build ID is
`219a8753a3caf79cf7a160cb220a7305b8d914d1bfd8989d52861d1cc1407de4`; binary SHA-256 is
`4df4194f9201147b07199266fd66b35970e953dbddc7b799af3dcf60f019dac6`. The exact source archive
`run9/source/radio-e7fa747.tar.zst` is 1,214,189 bytes with SHA-256
`b6fd7d8bb76fbc6e020ffb0cc8d1a45ef3d618d9e3e3280bfc329801c74c1536`. Its binary, provenance
sidecar, archive, `run.meta`, and hash manifest were all streamed back from S3 and independently
matched. Launch, survival, and preservation SSM commands are
`f766ce6b-1a8b-45a1-b929-2a65420899ee`, `78883034-52fd-42c7-a83d-3b5eae9eff46`, and
`7f9f7920-854e-4148-80d3-57c2588dce7a`.
Its mandatory cold control returned `SOLVABLE` in 479.2 CPU seconds. The run then completed all
sixteen roots and returned `UNSOLVABLE (419353.1 s)` with zero suppression markers. The finalized
raw log has 3,174,576 lines, 365,340,502 bytes and SHA-256
`ba635d9141601ebb643ed4f102703deb112fc3e8260f4936e8545fe44a300cf4`; it is archived as
`sa193-cold-2026-08-16:run9_out_sa193.txt`. Run8's paired raw log is in the same release. Exact
fact-set and performance comparisons are in
[`../evidence/sa193_run_comparison_2026-08-16.txt`](../evidence/sa193_run_comparison_2026-08-16.txt).

Run8's solver remains exactly that frozen build. Its comparison helper alone was updated after launch,
first to `4cd002e` for estimated self time and then to `58e3457` for visible-attempt aggregation.
The current helper SHA-256 is
`61af8b9512dec07fbcc621ff76bdb3386556c83d0bd515a7815093ab4ad6dd52`; the launch and intermediate
helpers are retained under `run8/monitor-updates/`, and `run.meta` records every transition and
hash. The update SSM commands are `73df22ca-30a2-4d18-b357-896d1e772e82` and
`4d0d79db-43cb-4800-bb23-9b2843a5fb2a`. No solver, cache, deadline, watchdog process or raw output
was changed; the frozen binary remains
`d9ae6e5feea4700be742504e345e2af09c910d790330b37457755cd89d4ac950`.

The predecessor run (2026-08-03, `i-0b8ca7169585b7cc1`) failed — deadlines had been removed and it
sank 43 minutes into one 13-part k=5 node — and was terminated.

### How the Sa runs were followed

`tools/sa193_watchdog.sh` emailed on every milestone plus a 6-hour heartbeat. The progress metric
was the only one that meant anything: `Sa(193)` in 10 is unsolvable **iff all sixteen**
`Sb(n1:193-n1)` fail in 9, for `n1 = 97..112`, so each is 1/16 of the job and each prints a line.
Verdict counts and elapsed time are not progress; "3 of 16" is.

Emails fired on: run started, another of the sixteen done, the control reporting, the first/new
rb-tainted contraction suppression, the final answer, the solver process dying, and every 6 hours
otherwise. Every AWS call in the watchdog is
failure-tolerant — a broken report must never kill the run.

### Why the control runs first

`radio_sa193` asks `Sa(192)` in 10 before `Sa(193)`, and **aborts** if it does not come back
solvable. `Sa(192)` has a verified witness tree, so this catches a broken engine before the negative
is produced — which is not a formality, given the 2023 corpus holds 37 provably false negatives with
no syntactic marker, and given an engine change trapped the last run. It is also not wasted work:
`Sa(192)` and `Sa(193)` share almost everything, so the control warms the cache for the real query.

## Checking the retained runs and live census

```
tools/sa193_status.sh --compare --baseline run8 --candidate run9
```

This now prints final rows for run8 and run9, followed by their exact-call comparison. Without
overrides the historical default remains run3/run8; `--all` prints the verbose history. During the
runs, the comparison chose
the run with fewer completed roots (then fewer verdicts), takes its six slowest completed exact
states and joins them to the selected peer by printed `(state,k)`. It groups a state's progress lines into
attempts whenever `elapsed` resets or another same-level verdict proves that the activation
returned. The final verdict's inclusive `took` is counted once; each abandoned visible attempt
contributes its last observed elapsed value. Because historical logs do not print the exact time of
a `MAYBE` return, `≥` marks the resulting attempt-sum floor, `(2a)` is the number of visible attempts,
and its ratio is prefixed `~`. Short abandoned attempts with no progress line remain unknowable.

Under each compact run row, the recursive stack was shown from the k=9 root down to the active
level, followed by that run's compact per-level `inclusive/self` profile. Visible
`still solving ... elapsed` time is added to the corresponding level before the self differences
are taken. Per-call `~self-final` subtracts all k-1 verdict time since the previous k verdict; it is
an estimate for the final activation only, and the first call at a level is shown as unknown. A
missing peer call is printed as `-`, never inferred.

| key | what |
|---|---|
| `run3/STATUS` | final performance-run snapshot |
| `run8/STATUS` | final bounded-probe snapshot |
| `run8/COMPARE` | finalized exact-state comparison against run3 |
| `run9/STATUS` | final proof-source snapshot, including zero contraction suppressions |
| `run9/COMPARE` | finalized exact-state comparison against run8 |
| `run7/STATUS` | final snapshot, explicitly reporting `solver process GONE` |
| `runN/sa193.checkpoint` | same-run restart checkpoint, refreshed hourly |
| `runN/seg-*/out_sa193.txt.zst` | immutable per-segment raw log, refreshed hourly and finalized at exit |
| `runN/seg-*/memprofile.csv` | time/RSS/verdict profile used for the comparison |
| `bench/r7iz.4xlarge.txt` | the k=9 ladder benchmark used to size this |

Is the instance alive?

```
aws-vault exec default -- aws ec2 describe-instances \
  --filters Name=tag:Project,Values=radio-sa193 Name=instance-state-name,Values=running \
  --query 'Reservations[].Instances[].[InstanceId,InstanceType,LaunchTime]' --output text
```

## Stopping it

**Do not stop it yet:** `pareto_k8_aws` and `run9_verify` are still running. After both exit, their
final raw output, certificates and sidecars must appear under `pareto-census-k8/20260814T0132Z/`
and `run9-verifier/20260817T163700Z/` and be hash-checked. Only then stop the instance while
preserving the EBS volume and all run directories:

```
aws-vault exec default -- aws ec2 stop-instances --instance-ids i-0005d74f985c52ae1
```

The active idle guard includes both job names and can stop the host only after both have gone. A
manual stop before either finishes is an interruption, not a result. The Sa artifacts are already
final; the census and independent replay artifacts are not.

## Resuming historical Sa work

No Sa resume is needed: H3 is complete. If a diagnostic continuation is ever deliberately run,
pass **that prefix's own** checkpoint as the driver's leading argument. Run9's checkpoint is sound
to warm-start from, unlike run3/run8 and `cache-2025:parsed_260.txt`; every checkpoint carries a
header naming the build, state and generation time. Such a continuation is new engineering work,
not part of the established Sa(10) proof.

## Why these numbers

- **r7iz.4xlarge.** The Graviton option (`x2gd`, half the price for the same 128 GB) is
  unavailable: the ARM vCPU quota on this account is **0**. The x86 quota is 5000 with ~1372 in
  use, so there is room to go bigger if memory demands it.
- **128 GB.** The pre-compact engine measured 2.32 KB of trie per insert at this geometry; the 2023 run
  reached ~90 GB. Note this also bounds how large a warm cache can be loaded: the filtered
  `out_k8.txt` facts that could inject into `Sb(74:40, 41:38)` number 11,375,981, which at that
  rate is ~25 GB of trie before the search starts. Filter harder, or size the instance for it.
  Before the compact runs, the estimate was 40–60 GB with 90 GB pessimistic. The three individual
  guards did not form a safe sum, so a separate 108 GiB combined guard watched all solver RSS and
  would have terminated only the newest run9 wrapper at the ceiling. In practice run8/run9 each
  peaked at 1.32 GB; the guard never fired.
- **On-demand, not spot.** Spot is ~$0.54/hr against $1.49, but the run is single-threaded and
  we are paying for RAM; the saving is not worth interruption handling on an unattended run.
- **AWS is slower than the laptop.** The k=9 ladder takes 391 s on r7iz against 261 s on the
  M4 Pro. We are here for the 128 GB, not the cores.
