# A recursive learned predictor: design note

Written 2026-08-20, after the flat-feature ranker topped out; revised the same day once the value
model and the oracle were measured, and again the same day once the recursive predictor itself was
built and measured; revised again 2026-08-21 once a user challenge showed the recursive scorer
degrades on the hardest endpoints, and composing it with a sound filter fixed that; revised once
more the same day after that composition was validated end-to-end with real solver calls, on a
real documented hard benchmark, not an offline proxy. Status: **the substrate, the level-held-out
value model, the recursive cut-scorer, and its composition with a sound worst-case bound are all
built, measured, and now validated end-to-end via genuine `canSolveB` calls; wiring a prototype
into `radiobase.c`'s own split loop is the remaining step.** Every number below has a source in
`evidence/`; anything proposed rather than measured says so.

## Read this first if you are picking the thread up

The goal is a *fast solver*: use a learned predictor to order the search so the exact solver reaches
verdicts sooner. Nine things are already in place; wiring one of them into the solver is not.

**Built and measured.**

| what | where | measured |
|---|---|---|
| warm oracle, stdin protocol | `radio_oracle.c`, `tools/oracle_client.py` | 0.11 ms/query; verdicts identical to `radio_one` on 2,200 states |
| full-corpus snapshot | `s3://radio-sa193-393287594714/oracle-prime/20260820T165448Z/cache.snap.zst` | 21.9M facts, restores in **32.8 s** at 2.41 GB |
| learned cut ranker | `tools/ml/cut_ranker.py` | median rank **76 of 54,014** with `R_0`; 428x better than blind |
| level-transfer value model | `tools/ml/value_level_transfer.py`, extended by `tools/ml/recursive_value.py` | **AUC 0.99+** transfers train-k<=6/test-k=7, not just the original k=4->k=5 pair |
| recursive cut scorer | `tools/ml/recursive_value.py` | scoring a split by `min(V(child))`, with **zero split-label supervision**, reaches **120x** selectivity vs a directly-supervised flat ranker's 130.5x, on the identical 153 real forced k7 endpoints |
| **worst case (sound) + order (learned)**, composed | `tools/ml/recursive_value.py` (Experiment 3) | `R_0` gives a real, theorem-backed cutoff — median 6,892 / worst 16,547 survivors, vs stage-2's up to 130,262 — that shrinks **more** on the hardest endpoints (10.6x vs 4.7x), and once applied first, the recursive ranker's hard-case degradation (median rank doubling, worst near-blind) disappears entirely (correlation with hardness drops from 0.129 to 0.001) |
| **end-to-end, real solver calls, no proxy** | `/tmp/rec/real_benchmark.py` (not committed; self-contained, reproducible) | on this repo's own "residual positive control" `Sb(29:6,19:9,13:12,36:3)@6` (documented: 37,899 top-level splits, 26.6-33 CPU s under the current default order), the composed pipeline finds an independently-verified working split after **67** real, oracle-checked top-level candidates — a **566x** reduction — while the same `R_0` survivors in natural order had not found one after 1,340 tries |
| **same, across 3/4/8-part states** | `/tmp/rec/real_benchmark_generic.py`, `/tmp/rec/real_benchmark_beam.py` | candidates-to-success is **43 / 67 / 52** — remarkably stable across a 3-8 part range and 9 orders of magnitude of raw search space, using the SAME value model, untouched. What actually needed to change by part count was candidate *generation*: exact DP enumeration broke (OOM) at 8 parts and needed a width-bounded beam variant, tried 5 times before succeeding — the scorer needed zero changes |
| **native `enumerate` oracle command — every winning split, exactly, in seconds** | `radio_oracle.c` (additive; `docs/tools.md` protocol) | replaces the Python-driven enumeration entirely: exact match to two documented ground truths (2 winners of 1,212,971,760; the residual control's complete list, 6 winners + 2 inconclusive) in 40s-7min, where the Python-plus-oracle pipeline took 40-60+ minutes for a single found witness. Sound `R_0` pre-filter before ever calling `canSolveB`; scoped to raw-space cost so not yet usable past ~4-5 parts. **2026-08-22: confirmed not usable on an ordinary k=7 4-part census endpoint either** — a call ran 10+ minutes with no result (killed) because admissibility is only checked at the deepest leaf of the raw walk, not on partial sub-trees; use the existing stage-2/`R_0`/recursive-V pipeline for k=7 ordering instead. See [../evidence/oracle_enumerate_2026-08-21.txt](../evidence/oracle_enumerate_2026-08-21.txt), [../evidence/real_benchmark_via_aws_oracle_2026-08-22.txt](../evidence/real_benchmark_via_aws_oracle_2026-08-22.txt) |
| **ordering pipeline re-validated via the persistent oracle** | `tools/oracle_tcp_client.py`, `tools/ml/real_benchmark_via_aws.py` | on a real, deliberately-hardest k7-census endpoint (mass 90.9% of cap, 2 known winners), learned order (`R_0` then recursive-V) succeeds at candidate **#1** of 2,626 `R_0` survivors; natural order on the identical subsample needs **#2,970** — same exact winning split either way, matching a known census winner, independently re-verified. Whole test (training + both orders) under 5 minutes wall-clock, vs 40-60+ min/state with the old cold-local-oracle method. Caught and fixed a real bug first: census "kN" corpora are rooted at k=N but their *endpoints*' actual parent level is `C["rk"]` (5 for the "k7" corpus, not 7) — querying at the wrong k gives internally-consistent but wrong answers. See [../evidence/real_benchmark_via_aws_oracle_2026-08-22.txt](../evidence/real_benchmark_via_aws_oracle_2026-08-22.txt) |
| **systematic 16-endpoint tier sample** | `tools/ml/tier_sample_via_aws.py` | `rank_learned` small and complete for all 16 (2-winner median 7/max 104, 4-winner median 83/max 687, pools of 31k-85k); `rank_natural` only fully resolved 4/16 within an 8,000-try cap (68.9x-6,041x measured, rest are `>=`11.6x-1,600x lower bounds) — reported as two separate facts, not one blended median, since averaging only the resolved cases would hide the harder-to-measure majority. See [../evidence/tier_sample_via_aws_2026-08-22.txt](../evidence/tier_sample_via_aws_2026-08-22.txt) |
| **per-part "deficit" order — a cheap `BY_MAGIC3` replacement candidate** | `tools/ml/proto_deficit_bestfirst.py` | the per-part Pareto-margin signal already scores AUC 0.9961 alone offline (nearly matching the pooled model) and costs nothing extra — it's already computed at split-table-build time in `radiobase.c`. Tested as an order on the same 4 real endpoints: real, complete, 2.2x-4.7x better than blind/natural order where comparable — but 1,000x-2,700x worse than the pooled recursive-V order, because 73-93% of every endpoint's mass-feasible candidates already sit at the single worst admissible deficit value (near-cap-mass states force some part to its own boundary to hit the exact total) — the signal saturates within one hard state's own survivor set even though it discriminates well across a population of states. A best-first *generator* built on the same signal fails outright for the identical reason (200,000 pops, zero feasible candidates on the easiest of the 4 endpoints) — not a search-algorithm problem, the score itself has no room left near its own boundary. See [../evidence/deficit_order_and_bestfirst_2026-08-22.txt](../evidence/deficit_order_and_bestfirst_2026-08-22.txt) |
| **block coordinate descent — a real "packing problem" framing** | `tools/ml/proto_coord_descent.py` | the mass/cap constraint is a solved multiple-choice knapsack; the open problem is that "does this combination work" is a non-separable joint function of the parts, so descent uses the real pooled score directly, on a small free block of parts at a time. 1-part-at-a-time fails outright (0/30 real successes, easiest endpoint). 2-parts-at-a-time works with real oracle-verified splits (one matching a known census winner, one a genuinely new split) on the two endpoints where the pooled model itself ranked easily (rank 1, 13) — but fails decisively, not just slowly, on the two where the pooled model struggled (rank 85, 687): 150 restarts, up to 774,712 evaluations (225x the endpoint's own R_0-survivor count), zero successes. Not cheaper than direct full-list scoring at the k7 scale tested even where it succeeds. See [../evidence/deficit_order_and_bestfirst_2026-08-22.txt](../evidence/deficit_order_and_bestfirst_2026-08-22.txt) section 6 |
| **concentric round expansion — succeeds where coordinate descent failed, now at n=10** | `tools/ml/proto_concentric_rounds.py`, `tools/ml/concentric_tier_sample.py` | replaces `canSolveB_ctx`'s arbitrary CPU-unit cutoff with an interpretable one: split a state's segments into concentric ones (radius grown together each round, per-segment growth factor `G^(1/(P-1))` derived from first principles to avoid a naive `G^P` blowup) and a "last" segment always walked in full with the real pooled score. Tested with real oracle verification on 4 tier-sample endpoints (**all 4 succeed**, including both where block coordinate descent failed decisively), then scaled to 6 more freshly-sampled endpoints (**all 6 also succeed**) for **10/10 overall**: oracle-call cost median 2,154 (range 886-6,971), round of success median 17 (range 16-18) — a strikingly tight round band despite a ~700x spread in pooled-model difficulty (rank 1 to 687), now backed by a real sample rather than 4 anecdotes. Exhaustive-within-radius search can't get stuck in the wrong basin the way restart-based local search can. Benchmarked segment order (deficit vs. the actual `BY_MAGIC3` production heuristic, ported faithfully into Python) at n=4: no clean winner (magic3 wins one endpoint, deficit wins two, one ties) — segment order is a real but modest (~20-80%) efficiency lever, not what makes the method robust. Not yet tested: propagating the round/radius down into the recursive verification of the children themselves (the multi-level half of the design), and the segment-order comparison at the n=10 scale. See [../evidence/concentric_round_search_2026-08-22.txt](../evidence/concentric_round_search_2026-08-22.txt) |
| corpus analysis | `tools/analyze_single_solution_cuts.py` | the forced-cut structure of both censuses |

**Not done: wiring a prototype into `radiobase.c`'s own split loop.** Every result above except
the last used sampled or exactly-enumerated stage-2 candidates scored offline; the last one (real
benchmark, real oracle calls) closes that gap for one state by using the real solver as ground
truth for every claim, without touching production code — but it is still not `canSolveB`'s
actual loop, it is one state (n=1), the win has only been shown at children-at-k=5 (this
benchmark) and children-at-k=6 (the census experiments) — not yet children-at-k=7 or the real k=9
H2 frontier — and the 46-minute wall-clock of that Python harness is not the number that matters
(`r0`/`feat` are cheap enough that a C port should score millions of candidates in well under a
second; 67-vs-37,899 candidates is the number to carry forward, not the harness's own runtime).
Full results, including two traps that broke the data pipeline before either question could be
answered, in [../evidence/recursive_value_2026-08-20.txt](../evidence/recursive_value_2026-08-20.txt);
the worst-case-first composition and the hard-case stratification that motivated it are in
[../evidence/recursive_value_worst_case_2026-08-21.txt](../evidence/recursive_value_worst_case_2026-08-21.txt);
the real-benchmark validation is in
[../evidence/real_benchmark_residual_control_2026-08-21.txt](../evidence/real_benchmark_residual_control_2026-08-21.txt).

**Start the oracle before anything else.** It removes the reason the earlier experiments were
awkward — labels used to cost 200 ms and a process each:

```
aws s3 cp s3://radio-sa193-393287594714/oracle-prime/20260820T165448Z/cache.snap.zst .
zstd -d cache.snap.zst
tools/build_radio.py -O3 -DMAX_K=9 -DMAX_N=300 radio_oracle.c -o radio_oracle_k9_n300
./radio_oracle_k9_n300 --restore-any=cache.snap --journal=oracle-journal.txt
```

`restore-any` is needed because that snapshot was built by Linux clang; the geometry is checked
either way. Journal every session so the next one starts warmer.

## What the measurements already rule in and out

* **A learned ranker works, as an ordering.** 428x better than blind at ranking winners, and it
  transfers across levels. It is **not** a filter: worst case 6.5x, no recall guarantee, so it can
  order a search but never prune one.
* **Top-5 guaranteed is out of reach this way.** Exact ranks over the full candidate set: median 76
  and worst 1,533 with `R_0` applied first, against the 5 a guarantee needs. ~15x short on the
  median, ~300x on the tail. See [../evidence/learned_cut_ranker_2026-08-20.txt](../evidence/learned_cut_ranker_2026-08-20.txt).
* **Data is not the constraint anywhere.** Both the ranker and the value model are flat from ~26
  training states, and plain logistic regression beats gradient boosting in both. The feature set
  binds.
* **The cheap sound filters are the thing to beat.** Per-part Pareto bound from `pareto_sb.csv`:
  ~9-12x at full recall for one array lookup. `R_0` full-star majorization: another 8x. A learned
  component earns its place only on end-to-end CPU against those.
* **Most of the learned gain sits where the sound bounds already decide.** On the 1,751 of 2,200
  states they leave undecided, the value model is 0.9596 against 0.9372 for mass alone. Real, much
  smaller, and that is the honest number to carry forward.
* **Recursion works, and standalone AUC does not predict which model survives it.** Scoring a split
  by `min(V(child))` with a value model that never saw a single split label reaches 120x selectivity
  against a flat ranker trained directly on winning splits (130.5x) — on the same real, held-out
  census endpoints. But the *better*-AUC model (gradient boosting, 0.996 vs logistic's 0.986 at the
  k=7 holdout) collapses to 2.3x once composed and shifted onto real data; only the end-to-end,
  composed metric caught this. Measured 2026-08-20; see
  [../evidence/recursive_value_2026-08-20.txt](../evidence/recursive_value_2026-08-20.txt).
* **The recursive ranker alone degrades on the hardest endpoints — composing it with `R_0` fixes
  that, it doesn't just paper over it.** Stratified by exact candidate-set size, the recursive
  ranker's median rank roughly doubles on the hardest third (146 -> 331) and its worst case is
  barely better than blind (16,886 of up to 130,262). Apply `R_0` first — the real, sound,
  theorem-backed filter, not a heuristic — and the degradation vanishes: rank medians go flat
  across hardness tiers (18/12/13) and the hardness-vs-rank correlation drops from 0.129 to 0.001.
  `R_0` itself shrinks the hardest tier *more*, not less (10.6x vs 4.7x). Two lessons: a ranking can
  never certify a negative, only a sound filter can — and the "hard cases resist learning" framing
  was measuring the wrong stage's output. Measured 2026-08-21; see
  [../evidence/recursive_value_worst_case_2026-08-21.txt](../evidence/recursive_value_worst_case_2026-08-21.txt).
* **This isn't just an offline artifact — it beats a real, previously-unbeaten benchmark using the
  real solver.** This repo already keeps `Sb(29:6,19:9,13:12,36:3)@6` on record specifically
  because a prior split-ordering proposal failed it. The composed pipeline (`R_0` then recursive
  V), tested by asking a genuine warm oracle — full `canSolveB` recursion, no shortcuts — whether
  each ordered candidate's three children are solvable, finds an independently-verified working
  split after 67 top-level candidates against the documented 37,899. n=1, and not yet inside
  `radiobase.c`, but this is the first result in the thread that isn't measured against a sampled
  or exactly-enumerated proxy. Measured 2026-08-21; see
  [../evidence/real_benchmark_residual_control_2026-08-21.txt](../evidence/real_benchmark_residual_control_2026-08-21.txt).
* **State length changes what's needed, but not where you'd guess.** Tested at 3, 4, and 8 parts
  (real documented states, real oracle calls): candidates-to-success is 43 / 67 / 52 —
  strikingly stable, using the SAME value model, unretrained, across a 3-8 part range and 9 orders
  of magnitude of raw search space. What actually breaks by part count is *generating* the
  candidates, not scoring them: exact DP enumeration (clean at 3-4 parts) OOM'd outright at 8, and
  needed a width-bounded beam variant — a cheap version of the DP top-k decoder the design already
  called for — tried at two widths before one found anything. 1- and 2-part states weren't tested:
  this repo already solves those exactly, no search needed. Measured 2026-08-21; see
  [../evidence/real_benchmark_by_part_count_2026-08-21.txt](../evidence/real_benchmark_by_part_count_2026-08-21.txt).

## Why the flat ranker stalled, and why recursion is the fix

`solvable(S, k)` is an AND-OR recursion:

```
solvable(S, k)  =  OR over splits c  of  AND over the three children  solvable(child(S,c), k-1)
solvable(S, 0)  =  mass(S) <= 1
```

The [learned ranker](../evidence/learned_cut_ranker_2026-08-20.txt) predicts the OR-choice from the
parent's *shape* alone. It reaches median rank 76 of 54,014 candidates and stops there, and the
reason is structural rather than statistical: the learning curve is flat from 26 training states, so
it is not short of data. What determines the winner lives one level down, and the
[R_d ladder](status.md) already measured the price of looking there — `R_0` 8x, `R_1` a further 5.8x
at 30-80 s per endpoint, `R_d` converging to exact solving. A predictor that never recurses cannot
climb that ladder.

So the object to learn is not "which split wins" but the recursion itself: a value `V(S,k)`
estimating solvability, and a policy `pi(c | S,k)` ordering the OR-branches, applied at every level.

## The one asymmetry that decides where to point this

**Achievability is safe; unsolvability is not.**

- Finding a **canonical/distinct-slot witness** is risk-free under guidance. Whatever tree a learned
  policy finds is checked by `tools/check_witness.py` against the explicit `G_k` strategy and
  Subgraph Monotonicity, and such a tree stands *even if the solver is wrong*. A majorized-only
  tree is not an achievability proof because singleton-majorization sufficiency is false.
- Proving **unsolvability** requires exhausting the OR-branches. A learned value can reorder them or
  allocate budget, but any pruning by it is unsound and would silently manufacture false negatives —
  the exact failure the 2023 corpus already shipped 37 times.

That settles the target: **point this at the k=9 achievability frontier**, where the 14-month
near-diagonal walk lives, not at refutations. Guidance there is free of correctness risk.

## The training corpus already exists

This is the part that makes the project cheap. Nothing needs to be generated from scratch.

| source | what it gives | size |
|---|---|---|
| [`sa193-certificate-2026-08-19`](data.md) | `(state, k) -> unsolvable` **spanning k=2..9**, with checked split hints | 2,846,568 claims |
| [`pareto-census-k8-2026-08-19`](data.md) and the k=7 census | every winning split of every endpoint — complete policy labels | 57,890 winners |
| census `input.tar.zst` in S3 | `exact.cache` and `dominance.cache` | 11.6M oracle facts |
| the C solver | on-demand labels for active learning | unbounded |

The certificate chain is the valuable one and is exactly the right shape: it is already *per level*,
already normalized, already verified with zero gaps. Its bias must be stated — it is all negatives
from one refutation lineage, so positives have to come from the censuses and the witness trees.

## Architecture, and the two constraints that pick it

A state is a multiset of parts, so the encoder must be **permutation-invariant**; the frontier is at
a different `k` than the training data, so it must be **scale-free**. DeepSets or a small set
transformer over parts satisfies the first. For the second, normalize lengths by `sqrt(3^k)` and
masses by `3^k` — which is not a guess: the two censuses have
[the same normalized shape distribution](../evidence/single_solution_cuts_2026-08-20.txt) (part size
0.211 of cap at k=8 against 0.206 at k=7), and the flat ranker already transfers k=7 -> k=8. The
current 26-feature vector hard-codes four parts, and that alone blocks universality.

**The action space needs factoring.** There are `prod (n_i+1)(m_i+1)` splits — about 1e9 for a k=8
endpoint. Two consequences:

1. No softmax over actions. Factor the policy per part: `pi(c|S) = prod_i pi_i(a_i,b_i | S, part_i)`,
   each per-part head conditioned on the global state embedding.
2. **No per-candidate network evaluation.** Scoring 1e6 candidates with a net is hopeless; scoring
   the `sum_i (n_i+1)(m_i+1)` ~ 1e3 per-part options once is trivial. This is the difference between
   feasible and not.

Then compose exactly rather than by sampling: the three child masses are sums of per-part
contributions, so the same `(S, X)` DP used to
[count the candidate space exactly](../evidence/learned_cut_ranker_2026-08-20.txt) also does
**exact top-k decoding** under a factored policy, with the information cap enforced inside the DP.
One network pass per part, then a DP, gives the k best feasible splits with no enumeration.

## Methods worth borrowing, by name

- **Learning to branch.** The closest published analogue: in MILP branch-and-bound a cheap model is
  trained to imitate *strong branching*, an oracle too expensive to run everywhere (Khalil et al.
  2016; Gasse et al. 2019 use a GNN). Here strong branching is the exact solve of the children. The
  correspondence is near-exact and the evaluation protocol — measure end-to-end solve time, not
  prediction accuracy — is the one to copy.
- **Proof Number Search.** The classical AND-OR tree algorithm, and the natural host: it already
  maintains proof/disproof numbers as difficulty estimates per node, and replacing the uniform
  initialization with a learned estimate is a well-trodden upgrade. Fits this problem better than
  MCTS, which assumes a stochastic game.
- **Expert iteration / AlphaZero-style bootstrapping.** Run the guided search, keep the splits that
  worked, retrain, repeat. The "expert" is the search itself, not self-play.
- **Bellman consistency as a loss.** `V(S,k) ~ max_c min_child V(child,k-1)` is trainable without any
  oracle beyond the base case, which is how you would push past the levels the solver can reach.
  Also the most likely thing to diverge or self-confirm; needs oracle anchoring at every level.
- **Curriculum across k.** Train where the solver is cheap, extrapolate upward. The k=7 -> k=8
  transfer already measured is the evidence that this direction works at all.

## First experiment, and how it should be judged

Smallest thing carrying real signal — **done 2026-08-20**, full numbers and two data-pipeline traps
in [../evidence/recursive_value_2026-08-20.txt](../evidence/recursive_value_2026-08-20.txt):

1. ~~Train `V(S,k)` as a DeepSets over parts on the certificate chain plus census positives~~ — not
   what was run. The certificate-chain-plus-census design was already refuted before this note was
   written (disjoint mass bands, see the trap table); the pooled sum/mean/max/min/std/median
   features already used by `value_level_transfer.py` are permutation- and part-count-invariant
   without a DeepSets network, and no torch is installed in `.venv`. `tools/ml/recursive_value.py`
   reuses that feature set.
2. Hold out a **level**: train `k<=6`, test `k=7`, matched sampler, oracle-labelled, permuted
   control. **Done — it separates.** AUC 0.986 (logistic) / 0.996 (boosted) vs a 0.482 permuted
   control, though the sound per-part deficit alone is now at 0.996 too — the learned edge has
   narrowed to the 131-of-273 states neither sound filter decides (boosted 0.971 vs mass 0.843
   there).
3. Since that separated: score a candidate split by `min(V(child))` one level down, on real forced
   census endpoints. **Done.** Logistic V, with zero split-label supervision, reaches 120x
   selectivity against a directly-supervised flat ranker's 130.5x on the identical population.
   Gradient-boosted V — despite *higher* standalone AUC — collapses to 2.3x recursively; standalone
   AUC did not predict this, only the composed metric did.
4. Stratifying step 3 by exact candidate-set size (2026-08-21, prompted by a user challenge that a
   scorer good on "easy" cases is not the interesting claim) showed real degradation on the hardest
   third — median rank roughly doubles, worst case near-blind. **Fixed by composing with `R_0`
   first**, not by more data or a bigger model: `R_0` is a sound filter (never drops a true winner,
   proved), gives a real worst-case cutoff (median 6,892 / worst 16,547 survivors, down from up to
   130,262), shrinks the hardest tier *more* than the easiest (10.6x vs 4.7x), and once applied the
   recursive ranker's hardness-correlated degradation disappears (0.129 -> 0.001). This is the
   Proof-Number-Search analogy above made concrete: `R_0`'s survivor count is the proof/disproof-
   number-like completeness bound; `V` is the learned estimate ordering what's left. See
   [../evidence/recursive_value_worst_case_2026-08-21.txt](../evidence/recursive_value_worst_case_2026-08-21.txt).
5. Judging it end-to-end, on a known-hard instance, with real solver calls instead of an offline
   proxy (2026-08-21, same day): this repo's own "residual positive control"
   `Sb(29:6,19:9,13:12,36:3)@6` — kept on record because it already rejected one split-ordering
   proposal — documented at 37,899 top-level splits under the current default order. Asking a real
   warm oracle (`canSolveB`, no shortcuts) whether each `R_0`-survivor's three children are
   solvable, in recursive-V order, finds an independently-verified working split after **67**.
   566x, on the actual acceptance test this thread was always going to be judged against, not on
   AUC and not on a sampled candidate set. See
   [../evidence/real_benchmark_residual_control_2026-08-21.txt](../evidence/real_benchmark_residual_control_2026-08-21.txt).

**What's left**: confirm this survives a C port (the 46-minute wall-clock above is unoptimized
Python scoring 4.4M candidates; `r0`/`feat` are cheap enough that this should not be the bottleneck
once ported), broaden past n=1, extend to children-at-k=7/k=8 and the real k=9 H2 frontier, the DP
top-k decoder over a factored per-part policy so candidates need not be enumerated at all, and
actually putting it in front of `canSolveB`'s split loop as a genuine (sound-preserving,
reordering-only) patch. The sound filters remain very cheap on their own (per-part Pareto bound
~9-12x at full recall for a table lookup, `R_0` another 8x); a learned component in the actual loop
still has to beat the composed pipeline on cost, not just beat blind.

Failure is a result here, and one showed up mid-experiment: the fixed [0.70,1.02]-of-cap mass band
that worked at k=4/k=5 sampled **zero solvable states out of 300 at k=7** — it does not transfer,
because the per-part solo Pareto maximum grows almost as fast as the cap does. The band must be
bisected per level against the oracle before drawing a training sample; see the evidence file and
the trap table.
