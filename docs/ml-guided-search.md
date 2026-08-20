# A recursive learned predictor: design note

Written 2026-08-20, after the flat-feature ranker topped out; revised the same day once the value
model and the oracle were measured, and again the same day once the recursive predictor itself was
built and measured. Status: **the substrate, the level-held-out value model, and the recursive
cut-scorer are all built and measured; wiring a prototype into the solver's split loop is the
remaining step.** Every number below has a source in `evidence/`; anything proposed rather than
measured says so.

## Read this first if you are picking the thread up

The goal is a *fast solver*: use a learned predictor to order the search so the exact solver reaches
verdicts sooner. Five things are already in place; wiring one of them into the solver is not.

**Built and measured.**

| what | where | measured |
|---|---|---|
| warm oracle, stdin protocol | `radio_oracle.c`, `tools/oracle_client.py` | 0.11 ms/query; verdicts identical to `radio_one` on 2,200 states |
| full-corpus snapshot | `s3://radio-sa193-393287594714/oracle-prime/20260820T165448Z/cache.snap.zst` | 21.9M facts, restores in **32.8 s** at 2.41 GB |
| learned cut ranker | `tools/ml/cut_ranker.py` | median rank **76 of 54,014** with `R_0`; 428x better than blind |
| level-transfer value model | `tools/ml/value_level_transfer.py`, extended by `tools/ml/recursive_value.py` | **AUC 0.99+** transfers train-k<=6/test-k=7, not just the original k=4->k=5 pair |
| recursive cut scorer | `tools/ml/recursive_value.py` | scoring a split by `min(V(child))`, with **zero split-label supervision**, reaches **120x** selectivity vs a directly-supervised flat ranker's 130.5x, on the identical 153 real forced k7 endpoints |
| corpus analysis | `tools/analyze_single_solution_cuts.py` | the forced-cut structure of both censuses |

**Not done: wiring a prototype into the solver's split loop.** The recursive scorer above is
measured offline (selectivity against the same sampled stage-2 candidates `cut_ranker.py` uses),
not yet in front of `canSolveB`'s actual split loop, and only at children-at-k=6 (scoring real k7
endpoint splits) — not yet children-at-k=7. Full results, including two traps that broke the data
pipeline before either question could be answered, in
[../evidence/recursive_value_2026-08-20.txt](../evidence/recursive_value_2026-08-20.txt).

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

- Finding a **witness** is risk-free under guidance. Whatever tree a learned policy finds is checked
  by `tools/check_witness.py` against the Singleton Majorization Theorem, and per this repo's
  evidence hierarchy such a tree stands *even if the solver is wrong*. A wrong prediction costs
  time, never correctness.
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

**What's left of step 3**: the DP top-k decoder over the factored per-part policy, and actually
putting either in front of `canSolveB`'s split loop — still not done, still gated on judging it by
end-to-end CPU seconds on a known-hard instance, not on AUC or on the offline selectivity numbers
above. The sound filters remain very cheap (per-part Pareto bound ~9-12x at full recall for a table
lookup, `R_0` another 8x); a learned component in the actual loop still has to beat that on cost.

Failure is a result here, and one showed up mid-experiment: the fixed [0.70,1.02]-of-cap mass band
that worked at k=4/k=5 sampled **zero solvable states out of 300 at k=7** — it does not transfer,
because the per-part solo Pareto maximum grows almost as fast as the cap does. The band must be
bisected per level against the oracle before drawing a training sample; see the evidence file and
the trap table.
