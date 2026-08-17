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

An end-to-end verifier replay of the new 3.17-million-fact run9 DAG has not been performed. It would
further reduce the trusted implementation from the solver to the independent checker, but it is
expected to require substantial enumeration and is not a prerequisite for the repository's
`proven-exhaustive` classification. The current result rests on a retained, fully provenanced,
current proof-safe exhaustive run plus the independently checked positive witness.

## Superseded 2023 route

The 2023 run reached the same conclusion after 4,079,185 solve seconds and roughly 90 GB of virtual
memory. It cannot source the claim: the same build produced 37 provably false negatives, the run was
resumed from caches whose earlier logs were not all retained, and there is no syntactic marker that
separates its wrong negatives. Its raw logs remain archived as historical cost evidence.

The former plan to rehabilitate that corpus by painting the reachable DAG, proving sixteen missing
k=8 facts and independently checking each level is therefore closed. It solved the wrong archival
problem once a clean cold derivation became available.
