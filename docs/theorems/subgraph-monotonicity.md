# The Graph Reformulation and the Subgraph Monotonicity Theorem

Written up 2026-08-03; **the attribution below was corrected 2026-09-02.** The theorem is
elementary and it is **published**: Aigner 1988, *Combinatorial Search*, equation (3.10),
book p. 145, states `H subset-of G => M(H) <= M(G)` (see
[../aigner-1988-scan.md](../aigner-1988-scan.md)). This page's earlier claim that it "had
never been stated" was wrong and must not be repeated in any write-up - cite Aigner. What
follows is a self-contained proof in this project's graph reformulation, kept because the
certificate and cache arguments below depend on the exact form of the corollaries, not
because the result is new. It is
what the solver's entire result cache rests on — `cacheCanSolve` closes downward and
`cacheCantSolve` closes upward over the relation built in
`sbb_lesser` / `sbb_greater`. It is also the lemma a negative certificate needs in order to
store only minimal elements rather than whole closures.

## The graph reformulation

Identify a state with a **graph** on the coin set: vertices are coins, and an edge `{x,y}`
means "the two defectives could be `x` and `y`". A test is a vertex subset `S`, and it
reports `|e ∩ S| ∈ {0,1,2}` for the true edge `e`. The three children of `G` under `S` are

| outcome | child |
|---|---|
| `2` | `G[S]` — the subgraph induced on `S` |
| `0` | `G[V \ S]` |
| `1` | the crossing subgraph: edges with exactly one endpoint in `S` |

`Sa(n)` is the complete graph `K_n`, and `Sb(n1:m1, …)` is the disjoint union of complete
bipartite graphs `K_{n_i,m_i}` on pairwise disjoint vertex sets. **That class is closed
under the operation**: for a part with sides `N ⊔ M`, writing `N_1 = N ∩ S`, `N_0 = N \ S`
and likewise for `M`, the three children contribute `N_1 × M_1`, `N_0 × M_0`, and
`N_1 × M_0 ∪ N_0 × M_1` — which is exactly the split-vector table in
[../problem.md](../problem.md#what-one-test-does), with the mixed branch doubling the part
count because it contributes two parts.

Mass is the edge count. A state is solvable in `0` iff it has at most one edge.

## Theorem (Subgraph Monotonicity)

> Let `G' ⊆ G` be graphs on the same vertex set, with `E(G') ⊆ E(G)`. If `G` is solvable in
> `k`, then so is `G'`.

### Proof

Induction on `k`. For `k = 0`, `G` solvable means `|E(G)| ≤ 1`, hence `|E(G')| ≤ 1`.

For `k ≥ 1`, let `S` be the first test of a strategy solving `G`, so `G[S]`, `G[V\S]` and
the crossing subgraph of `G` are each solvable in `k-1`. Test the same `S` on `G'`. Each
child of `G'` is a subgraph of the corresponding child of `G`, on the same vertex set, so by
the induction hypothesis each is solvable in `k-1`. ∎

The strategy is literally unchanged: a decision tree that separates every edge of `G`
separates every edge of any subgraph. Only the *existence* of a strategy is asserted; the
one exhibited need not be optimal for `G'`.

## Corollaries used by the solver

1. **Sub-multiset.** Deleting a part, or any subset of coins, yields a subgraph. So
   `Sb(S')` is solvable in `k` whenever `S' ⊆ S` as a multiset of parts and `Sb(S)` is.

2. **Componentwise part dominance.** If `n_i' ≤ n_i` and `m_i' ≤ m_i` for every part, then
   `K_{n_i',m_i'}` embeds in `K_{n_i,m_i}`, so the whole state is a subgraph. This is
   exactly the relation `sbb_greater` enumerates in `radiobase.c`: all `j` with
   `n1_j ≥ n1_i` **and** `n2_j ≥ n2_i`), and it is therefore sound.

3. **Certificates may store antichains.** A downward-closed set of solvable states is
   determined by its maximal elements and an upward-closed set of unsolvable states by its
   minimal elements, so a certificate need record only those, with this theorem as the
   inference rule.

## What it does *not* give

The dominance above is componentwise, and it relates neither of the following pairs, both of
which have the same coin count:

- `Sb(112:81)` and `Sb(111:82)` — the sixteen `Sa(193)` states are pairwise incomparable
  under it, which is why all sixteen need refuting independently.
- `Sb(a:b)` and `Sb(a+1:b-1)` — see
  [../conjectures.md](../conjectures.md#the-antidiagonal-conjecture-c). Moving a coin from
  the small side to the large side removes `a` edges and adds `b-1`, so neither graph
  contains the other. That conjecture is **not** an instance of this theorem, and the
  temptation to add it to `compare_solvability` as though it were is recorded as a trap in
  [../status.md](../status.md).
