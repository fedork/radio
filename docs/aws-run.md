# The `Sa(193)` AWS runs — final solver record and remaining census

This page is the operational record for the completed cold runs, the concluded verifier
diagnostics and the remaining census on the original shared instance; the findings go to
[journal.md](journal.md) as usual.

## What is running

| | |
|---|---|
| shared instance | `i-0005d74f985c52ae1`, `r7iz.4xlarge` (16 vCPU, 128 GB), us-west-2b, on-demand |
| dedicated verifier | terminated: `i-0cb3783e937115ff1`, run `20260818T074026Z`; complete frozen solver-core replay is release-verified |
| active Sa solvers | none — run3, run8 and run9 all completed 16/16 roots |
| active jobs | shared host only: `pareto_k8_aws` |
| account | 393287594714 (shared production — everything tagged `Project=radio-sa193`) |
| bucket | `s3://radio-sa193-393287594714/` |
| notifications | SNS `radio-sa193-progress` -> fedor@retellai.com |
| active memory guards | shared host: 20 GiB census |
| completion | verifier finalized to S3 and GitHub; do not stop the census host before its upload is checked |

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

At **2026-08-18 14:21:54 UTC**, the remaining `pareto_k8_aws` process had run for 391,269 seconds at
one full core and 9,041.7 MiB RSS. Its output contained all 815 `CENSUS SECOND_SUMMARY` blocks, 48 of
70 freshly recomputed blocks, the closed prefix summary, 1,688 targets, 1,893 endpoints and 1,070
full-state records. It had not yet emitted a final `CENSUS END` record. The host had 113.5 GiB
available, no swap and 193.1 GiB free disk. Do not stop the instance. The S3 `STATUS` object for
this census is still the launch snapshot; use `tools/pareto_census_status.sh` or the final artifact
rather than that stale object.

At **2026-08-17 16:34:44 UTC**, the independent run9 verifier launched from commit `8856509`.
It reproduced the 3,126,190-record sanitized certificate byte-for-byte and completed pre-color
antichain reduction. The dominant `k=7` level fell only from 2,576,885 to 2,507,270 facts in 713.01
seconds. Coloring then verified all sixteen `k=9` roots and all 2,151 minimal `k=8` targets, which
cited 2,506,515 `k=7` facts—99.97% of that level. Its frozen display had no intra-level cursor. On
2026-08-18 at 05:02:49 UTC its wrapper was sent TERM after about 12h28m in coloring; cleanup returned
exit 130 and uploaded final diagnostics under `run9-verifier/20260817T163700Z/`. No colored bundle
or replay result exists. The census retained its core throughout and remains live. Launch hashes,
the first snapshot and final disposition are in
[`../evidence/run9_verifier_aws_2026-08-17.txt`](../evidence/run9_verifier_aws_2026-08-17.txt).

At **2026-08-18 01:49:11 UTC**, a second, progress-reporting replay began on dedicated on-demand
instance `i-01f8c56b7a53a1178`. Run `20260818T014906Z` uses clean commit `f170ded`, sixteen physical
cores on a `c8a.4xlarge`, a 24 GiB RSS guard, twelve-hour phase caps and an independent 25-hour hard
stop. Its source bundle SHA-256 is
`4d4d952f06ce59cf157c0ed4c7a57d6d18c0d3799152bb1deccf1e8900fd1661`. It passed the remote
regression, hash checks, normalization and byte round-trip. The k=7 minimal level exactly matched
the first run's 2,507,270 facts but completed in 77.17 rather than 713.01 seconds; the comparison
combines the new verifier index, two more workers and physical-core hardware. Its k=7 coloring
batch contains 2,505,858 cited targets. The first 108,083 three-part targets ran at 144--191/s, but
the first four complete four-part intervals fell to 0.467--0.550/s while node cursors kept
advancing. The run was intentionally stopped at 05:02:58 UTC rather than spend more on an approach
that was already slower than the proof-producing solver. Its final k=7 snapshot had completed
119,649/2,505,858 targets in 11,460.1 seconds: 10,312/2,396,521 four-part targets, 105,605,161,144
nodes, zero unresolved/budget outcomes and a 0.450/s last-window rate. Cleanup returned exit 130,
uploaded the final manifest, and produced no partial colored certificate. The compressed normalized
input and color log were streamed back and matched that manifest. Instance
`i-01f8c56b7a53a1178` was then terminated, deleting its `DeleteOnTermination` root volume. Its
approximate on-demand compute cost from launch through finalization was **$2.79**. Exact identifiers,
bootstrap lessons, snapshots and final hashes are in
[`../evidence/verifier_progress_2026-08-17.txt`](../evidence/verifier_progress_2026-08-17.txt).

At **2026-08-18 05:53:00 UTC**, ordinary audit run `20260818T055255Z` launched on
on-demand `c8a.4xlarge` instance `i-066a6cd0b7f66d581`. Its clean source commit is
`cbc3eade963ca93e9986be614f6c91557c762fda`; source-bundle SHA-256 is
`9d89857e4449b51f7d0283d9bf178d4bb96b6fd66c9f4912fb2ef8cd670e0a07`; and its live prefix is
`s3://radio-sa193-393287594714/run9-verifier-progress/20260818T055255Z/`. This run deliberately
does no coloring. Remote regression, raw-source hash, normalization and byte round-trip passed.
It then entered a deterministic 9,995-fact sample across the k=7 four-part level. At the first
complete minute it had verified 198 facts with zero gaps at 3.300/s, 1,472% CPU, 1,359.6 MiB RSS
and zero swap. That one-minute projection is about 8.4 days and is not the gate result: canonical
cost varies, so the pipeline waits for the complete sample and automatically refuses the full k=7
phase if its measured projection exceeds seven days. If accepted, k=7, k<=6 and k=8..9 become
separately retained checkpoints. The seven-day phase cap, nine-day host stop and 24-GiB RSS guard
bound the attempt. This frozen source explicitly used the former canonical-n part order. It
ultimately verified all 9,995 sampled facts with zero gaps in 23,697,303,379 nodes / 1,627.30
seconds and projected 390,552 seconds (4.52 days), passing the gate. Once it entered the now-
superseded full phase, its capped wrapper was sent TERM. The supervisor finalized with exit 130;
every final-manifest object was streamed back and hash-checked. Instance `i-066a6cd0b7f66d581`
was terminated at 06:23:18 UTC, deleting its `DeleteOnTermination=true` root volume. Approximate
on-demand compute cost was $0.44.

At **2026-08-18 06:24:35 UTC**, the clean mass-descending replacement launched as run
`20260818T062429Z` on instance `i-0b81cd58d3ba14f0c`. Its source commit is `5869a46`; source-bundle
SHA-256 is `1cb324193d79fbd788c81961f4c35437b8096a2577dfbd61037ceda856b36e19`; and its live prefix is
`s3://radio-sa193-393287594714/run9-verifier-progress/20260818T062429Z/`. Regression, raw hash,
normalization and byte round-trip passed. The identical 9,995-fact sample closed with zero gaps in
3,197,377,218 nodes / 341.32 seconds—7.41x fewer nodes and 4.77x less wall than the canonical-order
run. Its k=7 four-part projection is 81,917 seconds (22.75 hours), safely below the seven-day gate,
and full k=7 verification began at 06:31:23 UTC. The live `BATCH_START` includes
`group_order=3`. The run was stopped deliberately at 07:08:07 UTC after the full phase reached
251,131/2,576,885 k=7 claims, including 73,045 four-part claims, with zero gaps in 2,160 seconds and
48,049,145,431 nodes. Its capped wrapper finalized with exit 130. Every object named by
`final.sha256`, including the normalized certificate and raw source, was streamed back and matched;
instance `i-0b81cd58d3ba14f0c` was then terminated. The result is diagnostic: even after the ordering
improvement, this checker repeated more work than the proof-producing solver.

At **2026-08-18 07:40:32 UTC**, frozen solver-core refuter run `20260818T074026Z` launched on
dedicated on-demand `c8a.4xlarge` instance `i-0cb3783e937115ff1`. Its clean source is commit
`e0402900f4a74853ac44344aa8080c41ce0688fe`; the 1,620,907-byte source bundle has SHA-256
`37bfe28092e394c09bf7cc136c95dbf102f5d78525b35e3110bdef126f9c43e8` and is under
`s3://radio-sa193-393287594714/run9-frozen-refute/20260818T074026Z/`. The run reuses the hash-checked
3,126,190-record normalized input from the concluded ordinary audit. It serially builds the compact
negative trie and required split metadata, then gives sixteen workers immutable solver tables and
worker-local search contexts. Its 9,995-fact gate closed with zero gaps in 81.200 wall and
1,293.979 CPU seconds, projecting the k=7 four-part level at 19,488 wall / 310,555 CPU seconds.
That is 5.41 hours and 74.06% of the complete cold solver's CPU, so both gates passed and full k=7
began at 07:47:30 UTC. It completed all three retained checkpoints with zero gaps: k=7 verified
2,576,885 claims in a 19,811.819-second worker epoch using 316,683.839 CPU seconds; k<=6 verified
546,744 in 129.665 wall / 2,072.754 CPU seconds; and k=8..9 verified 2,561 in 0.914 wall /
14.578 CPU seconds. The capped phases, including three serial cache loads, took 20,113, 427 and
305 wall seconds and peaked at 1.24, 1.14 and 1.11 GB RSS. Thus the full verification phase took
20,845 seconds (5h47m25s), while worker epochs used 318,771.171 CPU seconds—76.015% of run9's
complete 419,353.1-second solver cost. The measured result agrees closely with the gate projection.

The two interrupted coloring runs and the canonical-order gate were not promoted to a GitHub
release. Their 106-MB normalized inputs are derived duplicates of the durable run9 raw proof
source, and none produced a completed proof checkpoint. The small, durable measurements and hashes
are committed in `evidence/`; diagnostic objects remain under their S3 prefixes for operational
inspection. The completed frozen replay is preserved in the private release
[`sa193-frozen-refute-2026-08-18`](https://github.com/fedork/radio-data/releases/tag/sa193-frozen-refute-2026-08-18),
clearly labeled as solver-core rather than independent. Its exact S3 manifest and the release
download/decompression/SHA-256 round trip both pass. Full coloring remains deferred until the certificate carries
a compact, independently checkable proof of split-space coverage rather than requiring the checker
to rediscover it.

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

**Do not stop the shared host yet:** `pareto_k8_aws` is still running. The old-index verifier has
already exited 130 and its final diagnostic upload under `run9-verifier/20260817T163700Z/` was
hash-checked. After the census exits, its final raw output and sidecars must appear under
`pareto-census-k8/20260814T0132Z/` and be checked. Only then stop the instance while preserving the
EBS volume and run directories:

```
aws-vault exec default -- aws ec2 stop-instances --instance-ids i-0005d74f985c52ae1
```

The shared-host idle guard still recognizes both old job names but only the census remains. A manual stop
before it finishes is an interruption, not a result. The Sa and verifier diagnostic artifacts are
already final; the census artifact is not.

The frozen-refuter host no longer exists. Its supervisor stopped it after exit 0; every object in
the exact final manifest was downloaded and verified, and the complete payload was independently
round-tripped through the private GitHub release. Exact instance `i-0cb3783e937115ff1` was then
terminated on 2026-08-18. Its sole root volume, `vol-0dfe51cb88d605ffd`, had
`DeleteOnTermination=true`; a subsequent AWS lookup returned `InvalidVolume.NotFound`, confirming
deletion. This action did not address or touch shared census instance
`i-0005d74f985c52ae1`.

The dedicated coloring `c8a.4xlarge` no longer exists. It was stopped by its supervisor, its final
S3 manifest was checked, and exact instance `i-01f8c56b7a53a1178` was terminated on 2026-08-18. Its
single 30-GiB encrypted gp3 volume had `DeleteOnTermination=true` and was deleted; the normalized
run9 source remains recoverable from the durable raw proof release, and the run-specific diagnostic
objects remain in S3.

## Resuming historical Sa work

No Sa resume is needed: H3 is complete. If a diagnostic continuation is ever deliberately run,
pass **that prefix's own** checkpoint as the driver's leading argument. Run9's checkpoint is sound
to warm-start from, unlike run3/run8 and `cache-2025:parsed_260.txt`; every checkpoint carries a
header naming the build, state and generation time. Such a continuation is new engineering work,
not part of the established Sa(10) proof.

## Why these numbers were chosen, and what changes next time

- **r7iz.4xlarge.** At launch, the Graviton alternative was estimated at roughly half the price
  for the same 128 GB but was unavailable because the account's ARM vCPU quota was 0. The recorded
  x86 quota snapshot was 5000 with about 1372 in use. Those are historical launch inputs, not
  current sizing guidance.
- **128 GB.** The pre-compact engine measured 2.32 KB of trie per insert at this geometry; the 2023 run
  reached ~90 GB. Note this also bounds how large a warm cache can be loaded: the filtered
  `out_k8.txt` facts that could inject into `Sb(74:40, 41:38)` number 11,375,981, which at that
  rate is ~25 GB of trie before the search starts. Filter harder, or size the instance for it.
  Before the compact runs, the estimate was 40–60 GB with 90 GB pessimistic. The three individual
  guards did not form a safe sum, so a separate 108 GiB combined guard watched all solver RSS and
  would have terminated only the newest run9 wrapper at the ceiling. In practice run8/run9 each
  peaked at 1.32 GB; the guard never fired.
- **On-Demand for the cold proof source.** That remains the correct choice for run9: interruption
  would split the one-session derivation and make every retained segment part of the proof source.
  The old point-in-time price comparison is deliberately not retained as policy.
- **AWS is slower than the laptop.** The k=9 ladder takes 391 s on r7iz against 261 s on the
  M4 Pro. We are here for the 128 GB, not the cores.

The compact solver subsequently peaked at only 1.32 GB, and the complete Sa(113) verifier replay
peaked just below 1 GiB. Future instances must therefore be selected from the measured phase rather
than copied from the original 90 GB risk estimate: budget memory for the jobs that will actually
coexist, add explicit guard headroom, then choose cores from the measured scaling curve and inspect
physical-core/SMT topology. A representative small-corpus gate precedes a full launch.

Short, deterministic and fully restartable verifier benchmarks or engineering probes should use
Spot when suitable capacity is available. Durable inputs must already be outside the instance, and
each completed stage must upload its output. The current coloring build has no intra-level
checkpoint, so interruption during its 2.5-million-target `k=7` barrier loses the whole barrier;
that full run remains an On-Demand workload. The same is true of a unique cold proof run.

AWS currently documents that Spot capacity can be interrupted with a best-effort two-minute notice
and that neither immediate capacity nor uninterrupted completion is guaranteed. For automated
multi-type launches, use EC2 Fleet's recommended `price-capacity-optimized` strategy rather than
choosing the lowest-price pool. See the official
[Spot best practices](https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/spot-best-practices.html)
and [interruption notice](https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/spot-instance-termination-notices.html)
documentation. A missing Spot allocation or an interruption is an ordinary retry/rescheduling
event, never evidence about the computation.
