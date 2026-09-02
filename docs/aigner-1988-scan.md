# Aigner 1988 scan: relevant extraction

Source inspected 2026-08-24: Martin Aigner, *Combinatorial Search* (Wiley--Teubner, 1988), supplied scan `Aigner.pdf`.  The continuous chapter in the scan is Chapter 3, book pp. 123--191 (PDF pp. 5--73); it is **not** the book's Chapter 2, “Weighing Problems.”  PDF pp. 74--76 separately contain answers to recommended exercises on book pp. 344--346. On 2026-09-02 the searchable Google Books copy supplied the load-bearing Chapter 2 definition and Figure 2.13; exact scope and reproducible queries are in [the publication prior-art record](../evidence/publication_prior_art_2026-09-02.md).

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

The supplied PDF supports the graph/model discussion and the exact small-`m` citations already used in [literature.md](literature.md). It does not supply the Chapter 2 balance/spring-scale exposition. The searchable book copy now supplies the threshold definition and Figure 2.13, but not a page-by-page Chapter 2 inspection.

## Additions from the full Chapter 3 scan, 2026-09-02

Read from `Aigner.pdf` (book pp. 119-194, Chapter 3 only).

- **The converse conjecture, verbatim** (book p. 150, right after the proof of Prop. 3.25):
  "It is tempting to conjecture that the converse to **3.25** also holds: `A <= N(k) ==>
  M(A) <= k`. If true, this would provide a beautiful characterization of star forests with
  cost `k`, but so far only partial results are known."  This is the sentence the `K=6`
  counterexample refutes, and the one a paper must quote.
- **`N(k)` is defined by the closed form**, display (3.12), book p. 147:
  `n_1 = 2^k`, and `n_{2^{i-1}+1} = ... = n_{2^i} = C(k,0) + ... + C(k,k-i)`. Lemma 3.23 adds
  `sum n_i = 3^k` and the two halving recursions. `N(3) = (8,7,4,4,1,1,1,1)` is printed and
  equals `G_3`. **The dyadic-block partial-binomial-sum form is therefore Aigner's, not ours.**
- **Corollary 3.29** (book p. 152): `M(K_n) >= k+1` whenever `C(n,2) > lambda * 3^(k-1)`,
  `lambda = 3/(2 sqrt 3 - 2)`. The remark after it says "it is quite possible that `K_5` and
  `K_8` are the only 3-optimal graphs." That speculation is compatible with having a short
  exact threshold table and does not support the former inference that Aigner lacked one.
- **Exercise 3.3.1**: `M(K_{3,n}) <= k <=> n <= 2^k - k`. Matches our `n(k,3)` exactly.
- **Exercise 3.3.2**: `M(K_{4,n}) <= k <=> n <= 2^k - k + 2`. **The printed formula is an
  erratum.** At `k=2` it claims `M(K_{4,4}) <= 2`, i.e. 16 edges in 2 ternary tests, against
  the information bound `16 > 9`; and it gives 7 at `k=3` where exhaustive search gives 4. The
  intended statement is Li--Wu--Triesch's Corollary 3, `n(k,4) = 2^k - 2k + 2`, which matches
  every verified cell. Cite Aigner for the first statement of the case, note the erratum, cite
  Li--Wu--Triesch for the proof.
- **Exercise 3.3.3**: `M(K_{5,9}) = 4` and `M(K_{9,14}) = 5` - both agree with our `n(4,5)=9`
  and `n(5,9)=14`.
- **Proposition 3.32** (attributed to Andreae 1988b): for forests of max degree `<= r`, the
  exact gap between `M(F)` and the information-theoretic bound. Adjacent to, but not the same
  as, the star-forest question.

## Additions from indexed Chapter 2 text, 2026-09-02

- Book p. 102 defines `m^(2)(k)` as the largest `n` for which the two-defective spring-scale
  problem has cost at most `k`. Under the book's Chapter 3 graph translation this is exactly
  this repository's `Sa(k)`.
- Figure 2.13 prints `m^(2)(k)=3,5,8,13,22,37` for `k=2..7`. Aigner 1986 Figure 5 gives the
  same table in the graph notation `h(k)` and explicitly calls 37 the correct seven-test value.
- The verified [`sa38_k7.tree`](../witnesses/sa38_k7.tree) is therefore an unconditional
  constructive refutation of Aigner's printed 37. The 2026 positive/negative boundary record in
  [`pareto_sa.csv`](../data/pareto_sa.csv) separately establishes that the corrected exact value
  is 38.

The Chapter 2 result is based on Google's indexed source text rather than the supplied PDF. Do not
claim that the whole chapter was read; retain the page-by-page copy as a bibliography task.
