# Parallel solver design

Status: prerequisite implementation, not yet a thread-safe parallel solver.  `canSolveB_ctx`
now gives each recursive search an explicit work clock, exact L1 and joint-reachability workspace.
The existing `canSolveB` entry point is an exact compatibility wrapper around one default context.
The 1,038-answer serial regression is byte-identical, both work/CPU budget regressions pass, and
Address/UndefinedBehaviorSanitizer passes cover the moved reachability storage.

## Scheduling decision

Keep the cheap witness-oriented pass depth first.  Parallelize only the exhaustive tail by turning
coarse split prefixes into a bounded queue.  This avoids imposing a level-synchronous barrier on the
recursive proof: workers may close states at different `k` values, and order is irrelevant once all
published answers are definitive.

The queue is deliberately limited-width.  It should contain enough independent prefixes to occupy
the selected cores, but should not materialize the full breadth-first frontier.  A worker continues
ordinary depth-first search within one prefix.  An unresolved task returns `MAYBE` plus its cursor;
it does not publish a fact.  The coordinator may subdivide it or assign a larger deterministic
accepted-prefix allowance at the next batch boundary.

## Ownership

| owner | state | mutation policy |
|---|---|---|
| process | `sbb` universe, powers, majorization bases | initialized once, then immutable |
| cache epoch | normalized positive/negative antichains and their range indexes | immutable while workers run |
| split epoch | split geometry, order arrays and frozen admissibility/FAST metadata | immutable while workers run |
| worker | recursion stack, accepted-prefix clock, exact L1, reachability DP, task output | private; discarded or reused after the batch |
| coordinator | bounded task queue and definitive-result publication | sole writer between epochs |

The first worker-owned row is implemented.  The two epoch rows are not.  In current `main`, the
dominance trie and its arenas mutate on every newly proved answer; split tables are built lazily and
their `s[4]`, `s[5]` and `FAST` fields learn in place; `sbb_to_min_k` is also mutable.  Concurrent
recursive calls would therefore have C data races even though their `radio_search_context` objects
were distinct.  Do not put a thread pool around `canSolveB_ctx` yet.

## Epoch and publication protocol

1. Run the ordinary depth-first heuristic pass against the current cache.  A quick witness ends the
   query without queue construction.
2. Normalize each relevant per-`k` result antichain and build an immutable lookup image.  Canonical
   facts remain the source; auxiliary range keys are disposable indexes.
3. Freeze the split catalog needed by the batch.  Separate immutable cut geometry from learned
   per-cut bounds, or copy the latter into a worker overlay.  No lock may be held across recursion.
4. Enumerate deterministic coarse prefixes from exhaustive pass 2 until the bounded queue is full.
   Each task records the canonical root, pass, prefix depth, selected cut ids, cumulative three-child
   states, and an accepted-prefix allowance.  A serial cursor regression must prove that the union
   of tasks is exactly the old search interval with neither holes nor duplicates.
5. Workers read the epoch and write only private exact/hot overlays.  They may return `TRUE`,
   `FALSE`, or `MAYBE`; only the first two are candidates for publication.
6. At the batch boundary, the coordinator rechecks canonical identities, rejects contradictory
   signs, Pareto-normalizes definitive facts, publishes one new epoch, and deterministically orders
   durable log lines.  `MAYBE` tasks are resumed, subdivided, or enlarged without becoming facts.

This is a combined shared-memory/MapReduce shape: immutable indexes are shared within a machine,
while task and result records are self-contained enough to send to another machine later.  The
same batch boundary supplies restartability for Spot instances; a unique uncheckpointed proof still
belongs on On-Demand capacity.

## Cache layout and denormalization

Durable storage stays canonical and human-readable.  Do not expand the full implied closure: it
multiplies memory, publication work and validation surface.  Partial denormalization belongs in
rebuildable epoch indexes and worker-local hot caches.

The first frozen-index candidates should be measured in this order:

1. exact state hash/L1;
2. part count and total mass;
3. sorted segment product profile, followed by independent long-side and short-side profiles;
4. the existing exact component-injection/dominance test.

This reflects the successful verifier experiment without assuming its negative-only scan transfers
unchanged to the solver.  Positive and negative monotonicity run in opposite directions, so one
physical ordering may require two range views or a compact block summary.  Preserve the deployed
last-segment Pareto fronts as the mutable serial baseline until an A/B on the expensive current
`k=7` layer shows that an epoch index wins end to end.

## Next implementation gate

The next patch should introduce a read-only result-cache view plus a worker-local result overlay,
then split immutable cut geometry from mutable cut metadata.  After that:

- extract a resumable serial pass-2 prefix cursor;
- compare its exact verdicts and deterministic work with the current engine on
  `tools/split_regression.c` and the fixed warm four-part control;
- run two workers under ThreadSanitizer on a small closed corpus;
- benchmark widths 1/2/4/... on a measured `k=6` or `k=7` bottleneck, reporting wall time, total
  CPU, peak memory, work units, cache facts published and duplicate work.

The first parallel benchmark is successful only if it preserves definitive answers, keeps memory
bounded by the chosen queue/overlay limits, and reduces wall time without an unacceptable rise in
total accepted-prefix work.  A language rewrite is not the next experiment: the ownership and
batch boundaries must be made correct in C first, after which another implementation can reuse the
same task and epoch contracts.
