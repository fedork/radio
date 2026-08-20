# A recursive learned predictor: design note

Written 2026-08-20, after the flat-feature ranker topped out. Status: **design, nothing measured
yet.** The numbers quoted from earlier work carry their own sources; everything proposed here is
untested and is labelled as such.

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

Smallest thing carrying real signal:

1. Train `V(S,k)` as a DeepSets over parts on the certificate chain plus census positives, with
   `sqrt(3^k)` normalization.
2. Hold out a **level**, not a random split: train on `k <= 6`, test on `k = 7`. Report separation
   there. A permuted-label control is mandatory — it caught nothing last time but that is the point.
3. Only if that separates: factor a policy, decode top-k with the DP, and put it in front of the
   solver's split loop.

**Judge it on end-to-end CPU seconds on a known-hard instance, not on AUC.** A predictor that is
more accurate but costs more per node than the sound filters it displaces is a loss, and the sound
filters are very cheap: the per-part Pareto bound is ~9-12x at full recall for a table lookup, `R_0`
another 8x. Anything learned has to beat that on cost, not just on quality.

Failure is a result here. If a level-held-out value model does not separate, that is worth as much as
if it does, and belongs in [journal.md](journal.md) with its measured cost.
