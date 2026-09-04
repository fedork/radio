# run10 completed the cold `Sa(193)` re-derivation; the host was powered off 63 seconds later

Recovered 2026-09-04 from the stopped instance's root volume. The run was believed to have died at
13 of 16 top-level states because that is what its last durable `STATUS` object said. It had in
fact finished.

## The result

```
result CONTROL Sa(192) in 10 = SOLVABLE  (389.9 s)
...
can't solve Sb(97:96)[9312,193] in 9 took 108 totalsplits=412 pass=1 fast_solve=0 probe=shared work=1048665549 rate=20000000
can't solve Sa(193) in 10 took 301127
result Sa(193) in 10 = UNSOLVABLE  (301127.6 s)
=== done. MAYBE is not a refutation - it means a deadline fired, not that 193 is hard.
```

All sixteen top-level `Sb(n1:n2)` states with `n1+n2=193` carry a printed `can't solve ... in 9`
line, so the root layer is exhaustive. `radio_sa193.c:59` returns `r == TRUE ? 0 : r == FALSE ? 1 :
2`, so the wrapper's `exit 1` is the negative-verdict success path, not a failure:

```
[capped_run] sa193: completed | exit 1 | wall 301536s (5025m36s) | peak RSS 1.05 GB
```

This is a **second cold derivation agreeing with run9**, not an independent implementation: same
engine lineage, a different build (`9e9e25a`, post-refutation necessity-only cache semantics) and a
different cache history. It does not add implementation independence to the certificate of record;
it does show the current build reproduces the verdict from cold, 28.19% cheaper.

| | run9 (proof source) | run10 |
|---|---|---|
| result | `Sa(193) in 10 = UNSOLVABLE` | `Sa(193) in 10 = UNSOLVABLE` |
| process CPU | 419,353.1 s (4.85 d) | **301,127.6 s (3.48 d)** |
| peak RSS | 1.32 GB | 1.05 GB |
| raw log | 365,340,502 B | 470,935,285 B |
| instance | `r7iz.4xlarge`, 16 vCPU, shared with run8 | `r7iz.xlarge`, 4 vCPU, alone |

## Identity and archive

```
raw log        470,935,285 bytes, 3,335,084 lines
sha256         5622c3f39a68291138a91703f733ddffc49ee859d3ac17c66fd2d933d91215d8
provenance     OK  build_id=54419a4988bb53065d8855cd66d09e6f133896816aecdea635692c0ef33a7492
               commit=9e9e25ae1d5063207322071b9d4d4d626fc6e965
compile args   clang-15 -O3 -DMAX_K=10 -DMAX_N=193 -DMAX_PART_N=193 radio_sa193.c
audit          tools/extract_evidence.py audit: 0 contradictions
stderr         1,004,596 bytes, sha256 3c277b77dbab084988c21d9b5948865669899e735780dea39b2524a7071d7b1b
checkpoint     3,329,879 lines, 131,464,549 bytes, header "... segment 20260831T232959Z, final, ..."
```

Both the on-instance `sha256` and an independent local `shasum -a 256` of the decompressed S3
object agree, so the archived copy is byte-exact. Locations:

- `s3://radio-sa193-393287594714/run10/seg-20260831T232959Z/` — `out_sa193.txt.zst` (44,332,227 B
  at `zstd -19`), `sa193.checkpoint`, `sa193.err.txt`, `memprofile.csv`, `run10-metadata.tgz`
- release `sa193-cold-run10-2026-09-04` — raw log, stderr, metadata bundle
- `snap-03f0ad37ce7ec286d`, a snapshot of root volume `vol-020083ad3e3df88c4` taken before the
  instance was restarted, while it was still the only copy. The instance was terminated on
  2026-09-04 after the release verified, and the volume went with it, so this snapshot is the last
  image of the run's working directory

## The sixteen roots

Process CPU seconds, final pass, root split count. Every root is `can't solve`.

| root | took | pass | splits | | root | took | pass | splits |
|---|---|---|---|---|---|---|---|---|
| `Sb(112:81)` | 100,723 | 5 | 903 | | `Sb(104:89)` | 5,274 | 2 | 522 |
| `Sb(111:82)` | 64,173 | 5 | 790 | | `Sb(103:90)` | 3,944 | 2 | 484 |
| `Sb(110:83)` | 42,252 | 4 | 781 | | `Sb(102:91)` | 2,852 | 2 | 455 |
| `Sb(109:84)` | 28,163 | 4 | 693 | | `Sb(101:92)` | 2,002 | 1 | 497 |
| `Sb(108:85)` | 19,168 | 3 | 681 | | `Sb(100:93)` | 1,317 | 1 | 471 |
| `Sb(107:86)` | 13,392 | 3 | 621 | | `Sb(99:94)` | 789 | 1 | 448 |
| `Sb(106:87)` | 9,555 | 3 | 566 | | `Sb(98:95)` | 390 | 1 | 429 |
| `Sb(105:88)` | 7,017 | 3 | 510 | | `Sb(97:96)` | 108 | 1 | 412 |

The sixteen sum to 301,119 s against the reported total of 301,127.6 s. Printed verdict lines
carrying a `took` field, by level: k=2 3, k=3 140, k=4 58,049, k=5 124,225, k=6 530,838,
k=7 2,561,962, k=8 2,581, k=9 95 (the sixteen roots plus 79 cheap `Sa(n)` lines), k=10 1 —
3,277,894 in total.

**The root cost decay collapses near the diagonal.** Roots 1-12 decay by a stable factor of
0.64-0.75, but the last four fall 1,317 -> 789 -> 390 -> 108, i.e. by 0.60, 0.49 and 0.28. A
0.729 extrapolation from root 12 predicted ~2,400 s for the tail; the true figure is 1,287 s. Any
projection of a near-diagonal `Sa` tail from the mid-range roots overshoots, because the shared
cache has already absorbed almost everything those roots need.

## Why the run looked dead: a 32-second race

Timeline, all UTC, from `wtmp`, the journal, file mtimes and the `capped_run` footer:

```
11:06:43.23  memprofile row written; watchdog's 600 s cycle ticks
11:06:44     STATUS uploaded: "13 of 16", "solver process alive"   <- last durable state
11:15:12.88  final write to out_sa193.txt: the result line
11:15:15.99  capped_run writes its footer; radio_sa193 is gone
11:16:11     radio-shared-shutdown[667410]: "Sa(193) and K=6 survey ended; stopping host"
11:16:18     systemd-shutdown: "Syncing filesystems and block devices"; journald stopped
11:16:43     <- where the watchdog's next tick, and the final archive, would have been
```

The culprit is `tools/aws_shared_job_shutdown_when_idle.sh`, left running from the 2026-08-31
episode when the `K=6` census shared this instance with the Sa run:

```bash
while pgrep -x radio_sa193 >/dev/null || systemctl is-active --quiet radio-k6-survey.service; do
    sleep 60
done
logger -t radio-shared-shutdown "Sa(193) and K=6 survey ended; stopping host"
exec /usr/sbin/shutdown -h now
```

It was started by hand over SSM, not as a unit, and nothing in `tools/` or `docs/` recorded that it
had been installed. When the census migrated to its own Spot ASG on 2026-09-01 and was stopped at
05:13 UTC, this loop's only remaining condition was `pgrep -x radio_sa193`. Its companion
`/usr/local/bin/shutdown` (still installed, mtime 2026-08-31 23:58) shadows the real binary to
defer cloud-init's shutdown while the census is active; with the census gone it is a pass-through.

The loop did exactly what it was written to do, 56 seconds after the solver exited. The damage was
**purely archival**, and it came from a cadence mismatch: `sa193_watchdog.sh` polls at
`--interval 600`, and its solver-GONE branch is the only path that uploads the complete log, the
final checkpoint, the stderr file and the "FINISHED" SNS mail. A 60-second poll racing a
600-second poll wins, so:

- the newest S3 log stayed the routine hourly `zstd -3` snapshot from 10:35Z, ending 4m 37s before
  the result line, with no truncation marker
- `sa193.err` and `/var/log/sa193.log` were never uploaded, because user-data reaches them only
  after `wait $SOLVER_WRAPPER`, which sits *behind* the foreground watchdog
- no completion mail was sent, so the run looked stalled rather than finished

Nothing was lost only because `--instance-initiated-shutdown-behavior stop` kept the volume.

## What to change before the next long run

1. **Do not leave a `pgrep`-driven shutdown loop behind after a shared-host arrangement ends.**
   Tie it to the arrangement, and record the install in the run's own notes so teardown is visible.
2. **The final archive must not depend on a polling watchdog observing the exit.** Uploading the
   complete log from the same shell that waits on the solver, before anything else can power the
   host off, removes the race entirely.
3. **Read a stopped instance's `STATUS` as the last snapshot, never as the outcome.** Here the two
   differ by the entire result.
