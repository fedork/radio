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

An end-to-end verifier replay of the new 3.17-million-fact run9 DAG is running on AWS but has not
yet returned a verdict. It further reduces the trusted implementation from the solver to the
independent checker, but it is not a prerequisite for the repository's `proven-exhaustive`
classification. The current result rests on a retained, fully provenanced, current proof-safe
exhaustive run plus the independently checked positive witness. Launch state and the bounded
one-shot progress command are recorded in
[`../evidence/run9_verifier_aws_2026-08-17.txt`](../evidence/run9_verifier_aws_2026-08-17.txt).

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
This establishes the transport format, not the still-pending proof replay.

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
This remains the completed small-corpus prototype; the separate full run9 coloring/replay is now
live and incomplete on AWS.

## Superseded 2023 route

The 2023 run reached the same conclusion after 4,079,185 solve seconds and roughly 90 GB of virtual
memory. It cannot source the claim: the same build produced 37 provably false negatives, the run was
resumed from caches whose earlier logs were not all retained, and there is no syntactic marker that
separates its wrong negatives. Its raw logs remain archived as historical cost evidence.

The former plan to rehabilitate that corpus by painting the reachable DAG, proving sixteen missing
k=8 facts and independently checking each level is therefore closed. It solved the wrong archival
problem once a clean cold derivation became available.
