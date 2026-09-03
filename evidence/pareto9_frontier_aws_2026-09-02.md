# K=9 full Pareto-frontier AWS walk, launched 2026-09-02

This is the operational record for the unbounded full `K=9` `Sb` frontier walk requested on
2026-09-02 local time (launched 2026-09-03 UTC). It is a running computation, not yet a source for
new entries in `data/pareto_sb.csv`.

## What runs

Commit `546d9a16f0fc9defd13a07f08b5a6e0430669b65` adds a generic frontier driver and the durable AWS
pipeline. `radio_pareto.c --bootstrap-diagonal 9 55 55 CACHE` first verifies the supplied
`Sb(55:55)` seed, then alternates the two shores until the first exact negative. From that bracket
it follows the staircase to `m=1`, emitting a `PARETO CELL` only after a solvable state and its
first unsolvable right neighbor have both been obtained. An unexpected tri-state `MAYBE` aborts
the run; it is never interpreted as a positive verdict.

The driver is built with `MAX_K=9`, `MAX_N=514`, and `MAX_PART_N=514`. The value 514 is required
because the final queried upper neighbor is `Sb(512:2)`; the `m=1` upper neighbor is discharged by
the exact dichotomy bound instead of being queried. `tools/pareto_walk_regression.sh` checks the
same diagonal-bootstrap and direct-staircase paths against the complete exact `K=3` frontier,
including `(m,n)=(1,8),(2,7),(3,5),(4,4)`.

## Immutable input selected at launch

The pipeline copied a stable revision of the live `run10/sa193.checkpoint`, checking its S3 object
identity both before and after download, and preserved the exact compressed bytes under the new
run prefix. The selected input is:

| field | value |
|---|---|
| source | `s3://radio-sa193-393287594714/run10/sa193.checkpoint` |
| S3 ETag | `"e129a0998fef994159a2b1d8c5e92d6e-11"` |
| S3 size | 88,202,383 bytes |
| S3 last modified | 2026-09-03 00:59:54 UTC |
| SHA-256 | `fe85b0df45cbed0ae0d5b49a4f2c446695470d9ca5c429fd0b12a68a3577616d` |
| facts / lines | 2,266,369 |
| semantics | `singleton-majorization-necessity-only-v1` |
| preserved copy | `pareto9-frontier/20260903T011520Z/input/input-sa193.cache.zst` |

`tools/check_provenance.py` accepted that checkpoint before launch and again on the AWS host. This
is the current necessity-only cache epoch, not either retired mixed oracle snapshot. The live
`Sa(193)` job can continue replacing its own checkpoint without changing this run's frozen input.

After launch, a possible `94:94` or `95:95` start was considered. No restart was made. The checked
`Sa(192)` tree explicitly solves `Sb(112:80)@9`, so subgraph monotonicity makes `Sb(55:55)@9` an
unconditional seed. The table's `Sb(95:94)` positive is only `legacy` with its source log missing,
and neither `Sb(94:94)` nor `Sb(95:95)` has a retained unconditional witness. More importantly,
the lower start changes only bootstrap cost: the driver advances outward and finds the same first
unsolvable diagonal bracket before emitting any frontier cell. Keeping the already-live run avoids
discarding initialization for an optimization that cannot change the result.

## AWS deployment

| field | value |
|---|---|
| run ID | `20260903T011520Z` |
| instance | `i-007f24b8cbc1fb060`, on-demand `r7iz.xlarge`, `us-west-2b` |
| launch time | 2026-09-03 01:15:26 UTC |
| S3 prefix | `s3://radio-sa193-393287594714/pareto9-frontier/20260903T011520Z/` |
| source commit | `546d9a16f0fc9defd13a07f08b5a6e0430669b65` |
| source bundle SHA-256 | `b0a187f9fd8caf8bc442307a653145b775f136949f5557554e800cdf6639ec18` |
| production build ID | `8f56489277cf155e4c966b8fc8b7c52ad6ad67dbe14fe862cfe37ca2052c05a5` |
| production binary SHA-256 | `e4fe27cfe98fca5a2004b8cd57b7e1256b8530346a5d87c3d8ac221164443a0b` |
| wall-time bound | none (`tools/capped_run.sh --seconds 0`) |
| resident-memory cap | 24 GiB |
| disk | encrypted 50-GiB gp3 `vol-0e3bbd1272ce7069e`, `DeleteOnTermination=false` |

On-demand capacity is intentional: the individual exact queries have no intra-query checkpoint,
so a Spot interruption could discard a long unique derivation. The supervisor uploads `STATUS`
every ten minutes and the accumulated cache and compressed raw log hourly. S3 upload failures are
retried and do not kill a healthy solver; a failed final upload leaves the instance and persistent
volume available for recovery rather than tearing them down.

SNS topic `radio-sa193-progress` had a confirmed `fedor@retellai.com` subscription at launch. It
sends mail when the run starts, when diagonal bootstrap finishes, for each batch of newly closed
frontier cells, every six hours while otherwise quiet, on checkpoint-upload warnings, and on final
completion or failure. AWS/SNS CloudWatch recorded one `NumberOfMessagesPublished` in the
2026-09-03 01:15 UTC bucket, confirming the initial start notification reached SNS.

The production binary sidecar passes `tools/check_provenance.py` and records the exact source
commit and compile arguments above. A live SSM inventory after launch found exactly one wrapper and
one `radio_pareto_k9 --bootstrap-diagonal 9 55 55 input-sa193.cache` process on this instance. The
separate `run10` `Sa(193)` process remained alive on `i-0318c3349a0df835b` and was not modified.
The first durable solving snapshot, written 2026-09-03 01:26:11 UTC, found the solver alive after
ten minutes at 2.45 GiB RSS with zero decisions and 3,628 raw-log bytes. The log then contained only
the complete runtime provenance block, so zero decisions means cache replay/initialization had not
yet reached `PARETO START`; it is not a negative result or a stalled-state diagnosis.

Read durable progress with:

```sh
tools/pareto9_status.sh 20260903T011520Z
```

The status helper also performs a read-only SSM scan of the live raw log and prints ten distinct
slow facts, ranked by `max(completed took, highest progress elapsed)`. Completed verdicts and
progress-only records are labelled separately: `NO_FINAL_VERDICT` must not be read as a negative.
The units are the clocks actually present in the log, not reconstructed wall time. `took` is
inclusive process CPU; under the production work-budget build, `elapsed` is accepted split-prefix
work divided by 20,000,000. Both include recursive descendant work. The parser and ordering are
locked by `tools/pareto_slowest_facts_regression.sh`.

Do not promote a cell from the progress display alone. After completion, require the final raw log
to pass `tools/check_provenance.py`, retain its exact positive/negative line pair for every cell,
reconcile all emitted cells, and only then update `data/pareto_sb.csv` with a durable source.
