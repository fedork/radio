# The completed computational certificate for `Sa(193)`

The certificate design began 2026-08-04. Proof-safe cold run9 completed it on 2026-08-16;
`Sa(10)=192` is now established. This page records exactly what supports that claim and what
remains optional strengthening.

## The reduction

`Sa(n)` in `k` splits into a taken group of `n1` and the rest, needing `Sa(n1)` in `k-1` and
`Sb(n1:n-n1)` in `k-1`. Since the proven k=9 maximum is `Sa(112)`,

> `Sa(193)` is unsolvable in 10 iff all sixteen `Sb(n1:193-n1)` are unsolvable in 9,
> for `n1=97..112`.

A verified witness tree independently proves `Sa(192)` solvable. The negative side therefore
consists exactly of those sixteen root refutations.

## The proof-source run

Run9 started at 2026-08-12 03:21:12 UTC from an empty cache and stayed in one solver session. It
first passed the known positive control:

```text
result CONTROL Sa(192) in 10 = SOLVABLE  (479.2 s)
```

It then printed an exhaustive negative for every root and finished with:

```text
result Sa(193) in 10 = UNSOLVABLE  (419353.1 s)
```

The sixteen verbatim root lines are committed in
[`../evidence/sa193_unsolvable_in_10.txt`](../evidence/sa193_unsolvable_in_10.txt). Their raw source
is `sa193-cold-2026-08-16:run9_out_sa193.txt` in the private artifact store.

| property | run9 value |
|---|---|
| source commit | `e7fa747264476461a234bf78e49762ee77ad8d8d` |
| build ID | `219a8753a3caf79cf7a160cb220a7305b8d914d1bfd8989d52861d1cc1407de4` |
| raw SHA-256 | `ba635d9141601ebb643ed4f102703deb112fc3e8260f4936e8545fe44a300cf4` |
| raw size | 365,340,502 bytes; 3,174,576 lines |
| solver CPU | 419,353.1 seconds |
| wrapper wall / peak RSS | 419,849 seconds / 1.32 GB |
| top-level roots | 16 of 16 |
| cache origin | none, cold |
| audited contradictions | 0 |
| rb-tainted contractions | 0 suppressed |

Exit code 1 is the driver's expected code for a definitive UNSOLVABLE result; the wrapper completed
normally. `tools/check_provenance.py` accepts the raw log. `tools/extract_evidence.py audit`
interprets 857 comparable lines, 848 distinct comparable states and no contradiction.

## Why run9 is proof-safe and run3/run8 are not

The old implicit-prefix contraction could combine incorrectly with a suffix-reachability rejection:
it might cache a shorter negative that was actually solvable, contaminating later search. Run3 and
run8 predate fix `75814a7`, so their independently matching UNSOLVABLE results are retained only as
performance comparisons.

Run9 contains the fix. Once `rb_dead` actually rejects a partial assignment, that invocation may
cache the exact full negative but cannot materialize an implicit shorter one. Each affected event
would print `contraction=rb-suppressed:<size>`; none occurred in the completed run. The fix was
nevertheless present and its forced regression remained part of the build.

Cold single-session execution also avoids the other historical defect: a resumed cache may depend
on facts whose producing logs were never retained. Run9 inherited no cache, so the raw session is
closed by construction.

## What was retained

The `sa193-cold-2026-08-16` release contains:

- `run9_out_sa193.txt.zst`: the proof source;
- `run8_out_sa193.txt.zst`: the matched pre-fix performance comparator;
- `sa193-cold-metadata.tar.zst`: run3/run8/run9 source bundles, frozen binaries, provenance
  sidecars, run metadata, memory profiles, final status/stderr/watchdog files and checksums.

Final EBS-only sidecars were first copied to `run3/final/`, `run8/final/` and `run9/final/` in the
private S3 bucket. The shared checksum manifest is `final/sa193-cold-sidecars.sha256`. The release
manifest records raw hashes and byte counts, and `data/artifacts.csv` makes every cited asset
resolvable offline.

Run8 and run9 have 3,160,113 identical signed parsed facts, no opposite-sign state verdict, and only
6,536 / 7,071 run-specific facts. Those measurements and all three final run totals are committed in
[`../evidence/sa193_run_comparison_2026-08-16.txt`](../evidence/sa193_run_comparison_2026-08-16.txt).
Agreement is a consistency check, not a transfer of proof status from run9 to run8.

## Independent verifier status

`radio_verify.c` is a separate checker sharing no solver search code. It treats a negative proof as
a k-stratified DAG of refuted facts and verifies that every split has a refuted child one level
down, using only the split semantics and the proved Singleton Majorization, Unit-Group Elimination
and Subgraph Monotonicity theorems. It has verified the full `Sa(113)` k=9 ladder—304,105 negative
facts across k=2..8—with zero unverified.

Two end-to-end run9 coloring attempts were stopped on 2026-08-18 after establishing that those
checker builds were not economical at this scale. The instrumented sixteen-core run completed only
119,649/2,505,858 k=7 targets in 11,460.1 seconds, despite 105,605,161,144 recursion nodes and zero
unresolved/budget outcomes; the older fourteen-worker run had spent about 12h28m at the same
barrier. Neither produced a colored certificate or replay verdict. This does not change the
repository's `proven-exhaustive` classification, which rests on the retained, fully provenanced,
current proof-safe exhaustive run plus the independently checked positive witness. Final diagnostic
records are in
[`../evidence/run9_verifier_aws_2026-08-17.txt`](../evidence/run9_verifier_aws_2026-08-17.txt) and
[`../evidence/verifier_progress_2026-08-17.txt`](../evidence/verifier_progress_2026-08-17.txt).

Full coloring remains deferred. The independent checker searches split space for every negative
fact; top-down coloring decides which such searches to run but does not make them cheap. Its later
optimized ordinary audit was stopped after confirming that repeated search still cost too much.
The level-local solver-core refuter below is the practical baseline; a future independent
certificate should have the solver record a compact coverage proof—ranges or subboxes of tests
annotated with the rejecting outcome and cited lower-level fact—so the checker validates coverage
and citations instead of reconstructing the proof by search. The readable text envelope remains
appropriate; binary packing should remain an internal indexing choice until parsing or storage is
measured as the bottleneck.

### Parallel checker and durable certificate prototype (2026-08-16)

The checker now has a pthread path whose workers share only frozen fact levels. Ordinary verification
puts facts from every level in one dynamic queue: there is no computational level barrier, because
the final result accepts only if every local check succeeds and the dependency relation always
decreases `k`. Top-down coloring retains the necessary barrier from `k` to `k-1`, since one level's
citations define the next level's targets.

The durable format is human-readable text, not binary:

```text
radio-negative-certificate-v1
root 9 Sb(112:81)
fact 8 Sb(53:52,44:44)
```

Mass is derived, records are canonicalized, and unknown lines are rejected. Binary packing remains
an internal indexing choice. On the retained `fullsolve-2026:out_k7.txt` corpus, normalization
reduced 6,910,223 raw bytes to 1,908,729 text bytes and 194,131 bytes under `zstd -19`; the readable
representation is therefore not a meaningful storage penalty.

The format also passed a parse-only run9 gate: the old and new parsers both extracted 3,126,190
canonical negative records; normalization took 2.74 wall seconds and 457 MB peak RSS. The readable
file is 106,011,566 bytes, 7,194,721 under `zstd -19`, and a read/write round trip was byte-identical
with SHA-256 `3ad5877a2ffa3bcf04c3403a147ae075e406b4313cce83eb0761fdd563725116`.
This established the transport format; the subsequent frozen replay is recorded below.

For frozen replay, v1 is now the complete source corpus rather than the execution layout.
`tools/make_refute_level_certificate.py` deterministically produces one self-contained
`radio-negative-level-certificate-v2` file per audited level:

```text
radio-negative-level-certificate-v2
level 7
parts 1052
part <id> <n>:<m>
...
support 6 388317 1468377
fact <part-id> ...
...
split-hints 772
split <part-id> uses <occurrences>
...
claims 7 2576885 10121321
claim <part-id> ...
```

The sections are deliberately in load order. IDs are local to the file and map through explicit
`n:m` definitions; they are not the solver's internal `sbb` values. Definitions are in ascending
total-mass/long-side order, so descending IDs give the canonical state order; the verifier checks
both sides of that contract. The support and claim headers carry both record and part-reference
counts, permitting exact compact allocation. Split hints list every nonunit claim part and its
occurrence count, ordered by the producer as a performance hint.
The verifier rejects a missing, extra or miscounted hint and re-derives all split geometry, so the
hint section cannot remove a test. For a level-k phase, only support k-1 enters the dominance trie;
level-k records are targets and support storage is released after freezing.

The generated run9 k7 file is 63,781,183 readable bytes and 7,983,524 bytes under `zstd -19`. Its
compressed size is slightly larger than complete v1 despite containing only two levels: numeric IDs
compress less repetitively than literal `n:m`. That is acceptable—the format is for bounded phase
loading, compact in-memory claims and distribution, not archive compression. Exact hashes and
counts are in
[`../evidence/verifier_level_v2_2026-08-18.txt`](../evidence/verifier_level_v2_2026-08-18.txt).

A separately bounded top-layer coloring then verified the sixteen run9 `k=9` roots in 0.23 seconds
on eight workers and cited all 2,545 canonical `k=8` facts. It intentionally stopped there: no
`k=8` or lower fact was checked, and the support levels had not yet been minimalized. For this
particular log the sixteen roots happen to be exactly all canonical `k=9` facts, but the explicit
root records remain necessary format semantics—the small corpus demonstrates that top-level logs
are not generally root-only.

That corpus supplies 62,366 independently verified facts. The same O3 build returned zero gaps and
exactly 97,483,464 recursion nodes at every tested width; wall time was 14.13 seconds at one worker,
5.22 at four, 3.24 at eight and 2.79 at sixteen. Eight is the economical width on this small
workload: sixteen buys only another 14% wall reduction while materially increasing duplicate memo
and lazy-table work.

Pre-color same-level minimalization and explicit roots are both implemented. For one nontrivial
`k=6` root from that corpus, minimalization followed by coloring produced 373 support facts and a
9,897-byte certificate; one- and four-worker outputs were byte-identical and replayed with zero
gaps. Treating all 779 logged `k=6` facts as roots instead produced 38,275 support facts. Coloring
can discard unused descendants, but a supplied top-level target is a root by definition, so the
format must distinguish roots from incidental top-level facts.

Commands, hashes, the complete scaling table and sanitizer results are retained in
[`../evidence/radio_verify_parallel_2026-08-16.txt`](../evidence/radio_verify_parallel_2026-08-16.txt).
This remains the completed small-corpus prototype. The attempted full run9 coloring/replay is now a
closed performance diagnostic, not an active proof run.

### Packed dominance and adaptive block index (2026-08-17)

The verifier's static support lookup now uses a separate product-ordered columnar index while
leaving canonical facts and the readable certificate format unchanged. Sorted n, m and segment
products are necessary filters only; exact component injection still decides every citation. On a
provenance-complete hard run9 k=7 control, the legacy and production paths returned identical
4,644,469-node proofs and memo counts, while verifier wall fell from 209.63 to 33.24 seconds. The
extra columns raised measured peak RSS from 0.41 to 0.53 GB on the full 3.126-million-record input.

The default now adds a second bounded layer only on levels with at least 65,536 facts. Full
256-fact primary-key blocks retain Pareto-minimal mass/product profiles and are skipped only when no
summary point fits; every positive summary still reaches the original exact checks. On the same
hard root, a clean contemporaneous product-only/block A/B preserved all proof and memo counts while
reducing 39.16 to 11.70 seconds, at 0.53 versus 0.57 GB peak RSS. An ungated version slightly slowed
the complete Sa(113) replay, which is why smaller levels retain the product-only loop.

A newly colored Sa(113) certificate with 9 roots and 120,293 support facts then replayed under the
production build: the checker reported all 120,302 records closed and exactly 2,491,283,058 nodes
in 140.28 seconds on eight workers. The frozen solver-core refuter later exposed nine uncovered
splits in this colored support set, so retain the result only as a performance measurement. A full
run9 parse/write pass remained byte-identical, so the index itself is still strictly an internal
representation change. See
[`../evidence/verifier_product_index_2026-08-17.txt`](../evidence/verifier_product_index_2026-08-17.txt).
Block design, failed layouts and final controls are in
[`../evidence/verifier_block_pareto_2026-08-17.txt`](../evidence/verifier_block_pareto_2026-08-17.txt).

### Immutable kd dominance hierarchy (2026-08-18)

The fixed blocks were still spatially broad: a single hard k=7 proof admitted 5.509 billion fact
positions. The production index now recursively partitions the same sound mass, sorted-product and
independently sorted n/m profiles. Each immutable node carries componentwise minima. A failed lane
rejects every descendant; a fitting 32-fact leaf still reaches the old packed tests and exact
injection matcher, so the hierarchy cannot create a citation. The same query accelerates
same-level minimalization while explicitly excluding the fact being tested.

On the exact hard root, the hierarchy preserved the 4,644,469-node proof and memo counts while
reducing fact probes to 431.317 million and verifier wall to 4.20 seconds. Extending bounded
pairwise forward checking through 512 options reduced the five-root control from 9,158,686 to
4,690,828 recursion nodes and from 21.00 to 5.34 seconds. Pair rows have a 128-MiB-per-worker
fail-open ceiling. Complete run9 k=6 and k=7 antichain passes reproduced 229,341 and 2,507,270
minimal facts in 3.8 and 49.8 seconds locally. The complete 120,302-record Sa(113) replay retained
its established 2,491,283,058 nodes and reported zero gaps under the then-current group order; that
colored-support verdict is now superseded.

A later missing control changed only the order in which parent parts are enumerated: descending
segment mass, then descending long side. On twenty roots spread across the dominant run9 k=7
four-part level it reduced 41,945,991 canonical-order nodes to 5,336,038. The complete Sa(113)
colored replay reported all 120,302 records closed in 330,226,371 nodes and 25.10 seconds on twelve
workers instead of 2,491,283,058 nodes and 119.19 seconds. These remain valid traversal-cost
comparisons, not proof acceptance. This permutation is now the independent checker's default; the
former order and two failed alternatives remain selectable controls.

These measurements justified a bounded ordinary audit of the existing run9 facts, not a return to
coloring. Its sample verified all 9,995 facts with zero gaps in 341.32 seconds, but the partial full
phase still repeated more work than the solver and was stopped at 251,131/2,576,885 k=7 claims.
The replacement below is the solver-core baseline. Explicit split-space coverage remains the
preferable independent certificate design. Full measurements and the index soundness argument are in
[`../evidence/verifier_kd_index_2026-08-18.txt`](../evidence/verifier_kd_index_2026-08-18.txt).

### Frozen solver-core refutation (2026-08-18)

The completed AWS replay below used the original implementation, which loaded the full readable
negative certificate into the production compact trie. Current `radio_refute.c` instead prefers one
level-v2 file: only k-1 support enters the trie, while level-k claims remain compact targets. It
uses the checked root-part hints to prepare independently derived split tables and asks local
viability directly at k-1 before publishing an immutable epoch. Each worker owns its search
context. The audited root bypasses its own cached claim and runs only exhaustive pass 2; all
children are theorem or frozen k-1 cache queries. An uncovered split fails closed instead of
launching recursive solving, and no worker publishes a fact. Cache allocation counts and a
complete split-table checksum must remain unchanged.

This is intentionally not independent of the solver. It answers the practical baseline question:
can the original exhaustive refutation be replayed read-only and in parallel without costing more
than the solver? A full normalized Sa(113) control closed 304,105/304,105 in 10.232 verification
seconds on twelve M4 Pro workers. The colored 120,302-record subset exposed nine gaps, which is why
run9 uses all 3,126,190 normalized claims. The deterministic local run9 gate closed 9,995/9,995 in
126.337 verification seconds and projects 8.43 hours on twelve local workers. AWS run
`20260818T074026Z` closed the same gate in 81.200 wall / 1,293.979 CPU seconds, projecting 19,488
wall / 310,555 CPU seconds. Both its 24-hour wall gate and 419,353-CPU-second solver-cost gate
passed. The completed three-phase replay verified all 3,126,190 claims with zero gaps in 20,845
capped wall seconds; its worker epochs used 318,771.171 CPU seconds, 76.015% of the complete cold
solver. Peak RSS was 1.24 GB. The exact output is archived in the private release
[`sa193-frozen-refute-2026-08-18`](https://github.com/fedork/radio-data/releases/tag/sa193-frozen-refute-2026-08-18).
Details are in
[`../evidence/verifier_frozen_trie_2026-08-18.txt`](../evidence/verifier_frozen_trie_2026-08-18.txt).

The former “instrument then try trie/exact before theorem” priority is superseded by direct k7
controls. The 34.7% profile showed theorem heat, not that an extra lookup would hit. Within each
constant star run the majorization prefix difference is discrete convex, so checking its endpoint
is exactly equivalent to checking every expanded prefix. Two matched controls measured 28.69% less
worker CPU. Separately, compiling L1 out reduced worker CPU another 11.29%; it is now off only in
the frozen verifier, not the mutable solver. The k7 level file reduced cache construction locally
from 263.457 to about 3.1 seconds and the checked one-step split preparation reduced admitted sample
prefixes by 5.915%. The final 12-worker gate closed 9,995/9,995 in 79.672 wall / 934.528 CPU seconds
and projects 5.30 local wall hours for the four-part k7 band. Exact controls are in
[`../evidence/verifier_level_v2_2026-08-18.txt`](../evidence/verifier_level_v2_2026-08-18.txt).

The next bounded decision is a same-hardware AWS k7 gate, followed—only if its solver-cost gate
passes—by a complete k7 replay. Per-root reachability telemetry/hints remain plausible. A global
explicit-fact hash is deferred because no measured exact-hit case justifies paying another probe on
every child. Explicit split-space coverage remains the route to an independently asymptotically
cheaper checker.

### Live completed-target telemetry (2026-08-17)

The absence of an intra-level cursor in the first full run9 replay made a healthy 2.5-million-target
`k=7` batch indistinguishable from a pathological tail. The verifier now optionally reports actual
completed targets, not the batch size or the atomic claim cursor. Every interval also includes
recent/cumulative/EWMA completion rates, completed recursion nodes, progress by level and part
count, and the three oldest active facts with age and a node cursor. This separates three cases that
previously looked identical: steady work, a harder region of the canonical order, and a few stuck
tail facts.

The three ETA fields are deliberately labelled projections. Fact costs are heterogeneous, so
`eta_total_s` extrapolates all work so far, `eta_window_s` uses only the latest interval, and
`eta_ewma_s` smooths recent intervals; none is a promise. The window estimate reacts immediately
when canonical order enters a new part-count or mass region, while the other two deliberately lag.
A growing active age with an advancing node cursor means deep enumeration, while a growing
age with a flat cursor points toward support-index work inside one recursion node. This is enough to
identify the state and phase for a focused profile without enabling the high-overhead per-candidate
index counters in the production proof run.

The final local Sa(113) gate and the new dedicated-EC2 launch contract are recorded in
[`../evidence/verifier_progress_2026-08-17.txt`](../evidence/verifier_progress_2026-08-17.txt).

## Superseded 2023 route

The 2023 run reached the same conclusion after 4,079,185 solve seconds and roughly 90 GB of virtual
memory. It cannot source the claim: the same build produced 37 provably false negatives, the run was
resumed from caches whose earlier logs were not all retained, and there is no syntactic marker that
separates its wrong negatives. Its raw logs remain archived as historical cost evidence.

The former plan to rehabilitate that corpus by painting the reachable DAG, proving sixteen missing
k=8 facts and independently checking each level is therefore closed. It solved the wrong archival
problem once a clean cold derivation became available.
