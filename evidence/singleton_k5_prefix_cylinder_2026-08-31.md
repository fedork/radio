# Complete `K=5` singleton converse by prefix cylinders

**Status: exhaustive theorem, 2026-08-31.**  Every singleton state majorized by `G_5` is
solvable in five tests.  Consequently the `K=6` counterexample is at the minimum possible failure
level.

The computation exhausts all

    1,431,800,647,444

positive, nonincreasing, full-mass, exact-support parents majorized by `G_5`.  Prefix certificates
cover 1,431,650,734,151 of them; an uncapped exact Fixed-Color Hall search checks the remaining
149,913,293 individually and finds no hole.  Since Singleton Majorization is already sufficient at
`K=4`, every first cut found here has recursively solvable children.  Minimum-Support Reduction and
unit padding then extend the exact-support result to every `G_5`-majorized singleton state.

## Prefix-cylinder certificate

The general statement and proof are now recorded as the
[Exact Prefix-Cylinder Extension Lemma](../docs/theorems/singleton-majorization.md#exact-prefix-cylinder-extension-lemma).
In particular, the test below is necessary and sufficient for the one fixed prefix coloring and
alternating tail word to work uniformly over the entire cylinder; it is not merely a sampled or
one-sided estimate.  The lemma also gives a weaker closed-form Three-Bound Prefix Corollary.

Fix a generated parent prefix `x=(a_1,...,a_t)` of mass `S`.  Its completions have `z=32-t`
positive rows, remaining mass `R`, maximum next value `M`, and every completed prefix is bounded by
the corresponding prefix of `G_5`.  Color the fixed rows `A/B`; color the unknown suffix by a fixed
alternating rank pattern, trying both starting colors.

For a Fixed-Color Hall inequality `(p,q)`, put

    r=max(0,p-|A|),  s=max(0,q-|B|).

The unknown contribution is the sum of the first `r` suffix rows colored `A` and the first `s`
colored `B`.  The program computes its **exact maximum over every legal completion of the parent
prefix** by the memoized recurrence

    F(i,R,M,e,r,s) = max_v (selected(e,r,s)*v
                              + F(i+1,R-v,v,1-e,r',s')).

The choices `v` are exactly the positive sorted values that respect mass and the next `G_5`
prefix.  The base case requires exact final mass and support.  Hence this is a finite integer DP,
not a relaxation or sample.

If, for every `(p,q)`,

    fixed_A(p) + fixed_B(q) + F(t,R,M,e,r,s)
        <= H(p+q)+H(p)+H(q),

then every completion in the cylinder has a Fixed-Color Hall coloring and therefore a legal first
cut.  The proof is uniform over the cylinder: a completion may attain a different maximum for each
inequality, but each displayed upper bound holds for all completions.

The prefix-color search quotients only equal fixed rows and uses the same first-block A/B
complementation as the existing exact search.  Certificates are attempted at prefix depths
`4,6,...,16`.  Each attempt is allowed at most 16 coloring nodes.  This cap is harmless for
correctness: a capped failure merely descends farther in the partition tree.  At a complete parent,
the existing `GeneralSearch` is uncapped and exact.

An independent suffix-count DP counts every completion below a parent prefix.  A successful
cylinder therefore skips a known exact number of raw parents.  Fourteen threads receive disjoint
contiguous raw-rank intervals; each shard is accepted only if its prefix-covered count plus exact
leaf count equals the assigned interval size, and the aggregate is accepted only if all intervals
sum to the independently known exact-support total.

## Complete result

The clean completion command was

```sh
tools/capped_run.sh --seconds 7200 --rss-gb 16 \
  --label singleton-k5-prefix16-complete -- \
  tools/run_with_provenance.py /tmp/singleton-prefix-census-bench \
  --prefix-cylinder-parallel 5 14 16 2 4 16
```

It reported

```text
PREFIX_CYLINDER_PARALLEL_CENSUS K=5 workers=14 complete=YES verified=YES
  total_states=1431800647444 covered_states=1431650734151
  tested_leaves=149913293 generation_nodes=11887816744
  certificate_node_limit=16 stride=2 minimum_prefix=4 maximum_prefix=16
  certificate_attempts=7392630195 certificate_nodes=85340765606
  certificate_cutoffs=512760772 cylinders=6879849886
  leaf_search_nodes=1874543168 tail_maximum_memo_states=39974682
  tail_maximum_calls=1361209586230 hole=() seconds=4484.15
```

The depth decomposition was

| prefix depth | cylinders | covered parents |
|---:|---:|---:|
| 4 | 15,381 | 77,637,440,748 |
| 6 | 447,511 | 240,630,673,320 |
| 8 | 8,265,625 | 408,470,765,426 |
| 10 | 97,689,098 | 397,277,077,209 |
| 12 | 618,882,242 | 227,720,892,012 |
| 14 | 2,072,211,911 | 69,862,167,963 |
| 16 | 4,082,338,118 | 10,051,717,473 |

Thus the uniform prefix proof covers 99.989529737% of the exact-support space.  The exact fallback
handles the remaining 0.010470263%, averaging 12.504 exact search nodes per parent.  The completed
run took 4,489 wrapper wall seconds (74m49s) and peaked at 1.31 GB RSS.

An earlier static 12-shard run with the identical certificate parameters timed out after 3,604 wall
seconds (60m04s), peak RSS 1.19 GB, after five broad shards had completed.  It emitted no aggregate
verdict and is an abort, not evidence.  Fourteen narrower shards completed without changing the
algorithm or proof parameters.

## Controls and provenance

The checked regression uses the same 16-node certificate cap:

- `K=3`: all 160 exact-support parents are prefix-certified;
- `K=4`: 408,772 of 408,776 are prefix-certified and the uncapped exact fallback closes the other
  four.

The `K=4` decomposition is deliberately locked in
`tools/singleton_transfer_shell_regression.sh`.  The exact completion counter independently agrees
with the established totals at every controlled level.

An AddressSanitizer/UndefinedBehaviorSanitizer build repeated both prefix controls with no finding;
its build ID is `c3b62f39980f3e0959f059f33e6e955bbef745b69b6c47994e89d85cb47c9577`.

The completion run used source SHA-256
`beda97d34c08810bb24d0518c07f223d632f711e7ebb1b1235143c7cfbbbad34`, build ID
`5640d9ad463066aaf5b0f1530c959f0804185b74c2fc45e7585e4df87bce9c75`, and binary SHA-256
`503c6f4abdc4eb0b49e778a287bb49f920ce30047737f3157a309030c9f7a269`.  The build was made through
`tools/build_radio.py`; the run banner recorded complete build and runtime provenance.  The source
hash matches `tools/singleton_pair_coloring_census.cpp` in the resulting commit.

## Consequence and scope

This proves the Singleton Majorization Converse at `K=5`; it does not make majorization sufficient
at higher levels.  The dyadic no-first-cut family gives counterexamples for every `K>=6`, so `K=6`
is now the proved first failure level.

At `K=6`, transfer distance 14 was already proved minimum for a no-first-cut exact-support hole.
The new `K=5` theorem also removes the former recursive-minimality caveat: a closer `K=6` parent
cannot be recursively unsolvable through an unsolvable majorized `K=5` child.  It may still be
useful to classify other distance-14 holes, minimize the non-unit core under other orders, and seek
a compact laminar/Hall-dual criterion explaining why majorization is exact through `K=5` and first
fails on the thin balanced-band face at `K=6`.
