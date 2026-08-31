# Aigner 1988 scan: relevant extraction

Source inspected 2026-08-24: Martin Aigner, *Combinatorial Search* (Wiley--Teubner, 1988), supplied scan `Aigner.pdf`.  The continuous chapter in the scan is Chapter 3, book pp. 123--191 (PDF pp. 5--73); it is **not** the book's Chapter 2, “Weighing Problems.”  PDF pp. 74--76 separately contain answers to recommended exercises on book pp. 344--346.

## Direct relation to this project

Section 3.3, “Searching For an Edge: Ternary Variant,” makes the exact translation used by this repository.  A candidate defective pair is an unknown graph edge; testing coin set `A` reports whether the edge has two, one, or no endpoints in `A` (book p. 145, PDF p. 27).  Therefore:

- `Sa(n)` is the ternary edge-search problem on `K_n`.
- `Sb(n:m)` is the problem on `K_{m,n}`.
- A test taking `i` vertices from the `m` side and `j` from the `n` side has outcome graphs
  `K_{i,j}`, `K_{i,n-j} + K_{m-i,j}`, and `K_{m-i,n-j}` for outcomes 2, 1, and 0 respectively (equation (3.11), book p. 145, PDF p. 27).

This independently matches the state-transition table in [problem.md](problem.md#what-one-test-does) and the graph formulation in [theorems/subgraph-monotonicity.md](theorems/subgraph-monotonicity.md).

## Material worth citing or reusing

- **Subgraph monotonicity.** Equation (3.10), book p. 145 (PDF p. 27), states `H ⊆ G => M(H) <= M(G)`.  This is the published instance of the monotonicity principle proved in [theorems/subgraph-monotonicity.md](theorems/subgraph-monotonicity.md).
- **The singleton base sequence.** Aigner's star-forest sequence `N(k)` has the same entries as this project's `G_k`: `(1)`, `(2,1)`, `(4,3,1,1)`, `(8,7,4,4,1,1,1,1)`, ... (book p. 147, PDF p. 29).  Proposition 3.24 proves `M(N(k)) = k` by a three-way recursive test (book p. 148, PDF p. 30).  This is the historical source of the unconditional canonical singleton construction.
- **Majorization's historical boundary.** Proposition 3.25 proves only the necessary direction: a star forest solvable in `k` is weakly dominated by `N(k)` (book pp. 149--150, PDF pp. 31--32).  The next paragraph explicitly says the converse was then open.  It is now refuted here by an exact-support `K=6` counterexample; the former purported proof also used a false decomposition lemma.
- **Exact `m=2`.** Corollary 3.26 gives `M(K_{2,n}) = ceil(log_2(n+1))` (book p. 150, PDF p. 32), equivalent to `n(k,2) = 2^k - 1`.
- **Exact `m=3`.** The answer to Exercise 3.3.1 proves `M(K_{3,2^k-k}) <= k` and `M(K_{3,2^k-k+1}) >= k+1` (book p. 345, PDF p. 75), equivalent to `n(k,3) = 2^k-k` for `k >= 2`.

The scan supports the graph/model discussion and the exact small-`m` citations already used in [literature.md](literature.md).  It does not supply the Chapter 2 balance/spring-scale exposition or the results referred to there as 2.10 and 2.13.
