# Literature map

Primary literature checked against this project's model.  In the graph notation used by
Aigner and Li--Wu--Triesch, our state `Sb(n:m)` is the complete bipartite graph `K_{m,n}` and
their search number `c(K_{m,n})` is the minimum number of adaptive ternary tests.  Thus their
largest `n` with `c(K_{m,n}) <= k` is exactly our `n(k,m)`.

The eleven sources below and the scoped Aigner book scan were read directly, not inferred from
abstracts or secondary citations. Bibliographic links point to the version of record; local
download paths are deliberately not committed.

## Primary sources read

### Hwang 1987

F. K. Hwang, “A Tale of Two Coins,” *American Mathematical Monthly* 94(2) (1987),
121–129. [JSTOR 2322412](https://www.jstor.org/stable/2322412),
[DOI 10.2307/2322412](https://doi.org/10.2307/2322412).

- A readable historical survey of two-defective search models.  Its “model Q” is quantitative
  group testing, and its Fibonaccian construction is an ancestor of the canonical singleton
  sequence used here.
- Best use in a paper: motivation and terminology, not a source for the fixed-`m`, `k=9`
  frontier.  A short quotation candidate from p. 121 is “immensely more difficult to search
  optimally for two objects.”

### Aigner 1986

M. Aigner, “Search problems on graphs,” *Discrete Applied Mathematics* 14 (1986),
215–230. [DOI 10.1016/0166-218X(86)90026-0](https://doi.org/10.1016/0166-218X(86)90026-0).

- The foundational formulation: `Sa(n)` is search on `K_n`, `Sb(n:m)` on `K_{m,n}`, and a
  subset test partitions the candidate-edge graph into the 0/1/2 outcome graphs.
- Introduces the canonical sequence `N(k)`; for example
  `N(3)=(8,7,4,4,1,1,1,1)`, the direct predecessor of this repository's `G_k`.
- Gives the exact complete-bipartite results for `m=2` and `m=3`.  These upgrade the `k=9`
  values 511 and 503 from lower bounds to published maxima.

### Aigner 1988 (scoped book scan)

M. Aigner, *Combinatorial Search* (Wiley--Teubner, 1988). The supplied scan is Chapter 3 and
the relevant answers-to-exercises pages, rather than the book's Chapter 2 on weighing. Its
directly checked details are in [aigner-1988-scan.md](aigner-1988-scan.md).

- Section 3.3 explicitly identifies quantity testing with ternary edge search, derives the
  three `K_{m,n}` child graphs, and gives the historical singleton sequence `N(k) = G_k`.
- Corollary 3.26 gives the exact `m=2` result, while the answer to Exercise 3.3.1 gives the
  exact `m=3` result. This independently cross-checks the corresponding Aigner 1986 claims.
- Proposition 3.25 is only a necessary majorization condition. Its stated open converse is a
  useful guard against overstating what this older source proves.

### Andreae 1989

T. Andreae, “A ternary search problem on graphs,” *Discrete Applied Mathematics* 23 (1989),
1–10. [DOI 10.1016/0166-218X(89)90030-9](https://doi.org/10.1016/0166-218X(89)90030-9).

- Extends the adaptive graph-search problem to broad graph classes, especially forests, and
  places Aigner's exact small complete-bipartite cases in a wider near-information-bound
  theory.
- Useful for related-work context; it does not determine the `m=5` frontier or supply a direct
  solver shortcut for the present fixed-`m` states.

### Hao 1990

F. H. Hao, “The optimal procedures for quantitative group testing,” *Discrete Applied
Mathematics* 26 (1990), 79–86.
[DOI 10.1016/0166-218X(90)90022-5](https://doi.org/10.1016/0166-218X(90)90022-5).

- Uses `T(n)` for two defectives in one set and `T(m,n)` for one in each of two sets.
- The composition inequality `T(ab,cd) <= T(a,c)+T(b,d)` and the convergence results for
  `T(n,n)/ln n` and `T(n)/ln n` are the cleanest bridge from exact finite strategies to
  scalable constructions.
- Relevant to a scalable implementation: product constructions can turn a verified small
  strategy into a recursive upper bound, but they do not by themselves prove a finite Pareto
  maximum.

### Gargano--Montuori--Setaro--Vaccaro 1992

L. Gargano, V. Montuori, G. Setaro and U. Vaccaro, “An improved algorithm for quantitative
group testing,” *Discrete Applied Mathematics* 36 (1992), 299–306.
[DOI 10.1016/0166-218X(92)90260-H](https://doi.org/10.1016/0166-218X(92)90260-H).

- Gives an explicit recursive algorithm and the concrete benchmark `T(32,32)=7`.
- Its main asymptotic consequence is
  `T(n) <= (7/log_3(32)) log_3(n) + O(1) = 2.18... log_3(n) + O(1)`.
- The appendix's multipart decision procedure is useful both as a source of solver regression
  cases and as a publication reference for a scalable constructive algorithm.

### Christen 1994

C. A. Christen, “Search problems: one, two or many rounds,” *Discrete Mathematics* 136
(1994), 39–51.
[DOI 10.1016/0012-365X(94)00106-S](https://doi.org/10.1016/0012-365X(94)00106-S).

- A survey organized by adaptivity: fully sequential, a bounded number of rounds, and
  non-adaptive search.  It is a useful citation for why the number of rounds is a genuine
  model parameter.
- Relevant as context for our adaptive solver and for explaining why the non-adaptive Sidon
  reformulation is not equivalent once `m>=3`; it has no direct exact `m=5` result.

### Li--Wu--Triesch 2018

S. Li, X. Wu and E. Triesch, “A ternary search problem on two disjoint sets,” *Discrete
Applied Mathematics* 251 (2018), 221–235.
[DOI 10.1016/j.dam.2018.05.026](https://doi.org/10.1016/j.dam.2018.05.026).

This is the decisive source for the correction in this repository.

- Corollary 3 proves `n(k,4)=2^k-2k+2` for `k>=3`, so the old “lemma 8” is a theorem, not a
  conjecture.
- Theorems 1–3 and Remark 1 prove the exact piecewise formula

  ```text
  n(k,5) = 2^k - 3k - (k-4)(k-5)/2 + c(k),
  c(k) = 5  for 3 <= k <= 8,
         6  for 9 <= k <= 10,
         7  for k >= 11.
  ```

  Equivalently, if `F(k)=2^k-k(k-3)/2-5` is the former formula, the exact answer is
  `F(k)` through `k=8`, `F(k)+1` at `k=9,10`, and `F(k)+2` from `k=11` onward.  In
  particular `n(9,5)=481`, `n(10,5)=985`, and `n(11,5)=2001`.
- The construction changes type at precisely the solver-observed break.  Theorem 1 starts
  with a `3+2` test for `k=3..8`; Theorem 2 starts with `4+1` for `k=9,10`; Theorem 3
  retains the first two tests but adds another recursive stage from `k=11`.
- Theorem 3 is quantified for every `k>=11`, so it excludes any later change in the numerical
  frontier.  It supplies a uniform construction and matching bound, not uniqueness of the optimal
  strategy or an aligned per-coin `AABD` profile.
- Publication caution: the theorem statements and final piecewise result are internally
  consistent and independently confirmed at `k=9` here, but displayed intermediate equations
  (69)–(70) contain apparent index/off-by-one inconsistencies.  Equation (69), for example,
  writes `k-2` in its last singleton width although the immediately preceding selected test
  uses `k-1`.  Recompute numerical specializations rather than copying those displays.  The
  corrected rectangle algebra and exact D-slice consequence are in
  [the m=5 Pareto-assembly calibration](theorems/m5-pareto-assembly.md).

### Jiang--Polyanskii--Vorobyev 2019

Z. Jiang, N. Polyanskii and I. Vorobyev, “A near-optimal algorithm for adaptive searching of
two counterfeit coins,” *Workshop on Coding and Cryptography* (2019).
[Proceedings PDF](https://www.lebesgue.fr/sites/default/files/proceedings_WCC/WCC_2019_paper_65.pdf).

- Gives an **explicit** asymptotic construction for the same ternary edge-search model.  Its
  mixed strategy over disjoint copies of `K_{n,n}` gives
  `N_wc(K_{n,n}) <= 1.2938 log_2(n)(1+o(1))`.
- The reusable structural idea is to mix many bipartite instances, then resolve a bounded
  matching of residual candidate edges.  This is useful conceptual precedent for this
  repository's multipart constructions, but it is not a finite fixed-`m` `Sb` frontier proof.
- The paper conjectured the smaller `1.2662` leading constant; the next source establishes the
  sharp asymptotic rate.

### Florin--Ho--Jiang 2022

S. H. Florin, M. H. Ho and Z. Jiang, “On the Binary Adder Channel With Complete Feedback, With
an Application to Quantitative Group Testing,” *IEEE Transactions on Information Theory* 68(5)
(2022), 2839–2856. [DOI 10.1109/TIT.2021.3137965](https://doi.org/10.1109/TIT.2021.3137965).

- Establishes the exact asymptotic rate for the adaptive one-set problem:
  `Sa(n)` needs `1.26624... log_2(n)(1+o(1))` tests.  This settles the leading constant left
  open by the 2019 explicit construction.
- Its Theorem 10 gives a direct bijection between strategies with one defective in each of two
  disjoint equal-size sets and uniquely decodable binary-adder-channel codes with complete
  feedback.  This is a clean modern source for the bipartite model's coding-theoretic analogue.
- These are asymptotic results, not finite exact values of `Sa(n)` or fixed-`m` `Sb(n:m)`.
  They therefore do not change the project tables or the evidence behind their maxima.

### Zhang--Berger--Massey 1987

Z. Zhang, T. Berger and J. L. Massey, “Some Families of Zero-Error Block Codes for the Two-User
Binary Adder Channel with Feedback,” *IEEE Transactions on Information Theory* 33(5) (1987),
613–619. [DOI 10.1109/TIT.1987.1057358](https://doi.org/10.1109/TIT.1987.1057358).

- An early coding-theoretic treatment of the full-feedback binary-adder channel. Its Fibonacci
  and difference-equation code families are direct equivalents of adaptive bipartite strategies;
  the paper's best displayed symmetric full-feedback rate is about `0.717` bits per user per use.
- The later WCC paper translates its refined construction to the `1.3954 log_2(n)(1+o(1))`
  explicit worst-case bound. It is historical construction context only: it supplies neither a
  finite fixed-`m` maximum nor the now-known optimal asymptotic constant.

### Hwang--Lee 2001

F. K. Hwang and J. S. Lee, “Optimal quantitative group testing on cycles and paths,” *Discrete
Applied Mathematics* 113 (2001), 299–302.
[DOI 10.1016/S0166-218X(01)00188-3](https://doi.org/10.1016/S0166-218X(01)00188-3).

- Uses exactly the same ternary induced-subgraph test, but for candidate graphs that are unions
  of paths and cycles. It determines their optimal test counts up to a one-test cycle effect.
- This is useful confirmation that the repository's graph formulation is standard. It does not
  address `K_n`, `K_{m,n}`, or the present complete-graph/bipartite frontiers.

## Consequences imported into this project

| claim | prior record | corrected status | source |
|---|---|---|---|
| `n(9,2)=511` | proved construction/lower bound | exact maximum | Aigner 1986 |
| `n(9,3)=503` | proved construction/lower bound | exact maximum | Aigner 1986 |
| `n(9,4)=496` | verified witness/lower bound | exact maximum | Li--Wu--Triesch 2018 |
| `n(9,5)=481` | conjectured 480, witnessed 480 | exact maximum; 481 witnessed here | Li--Wu--Triesch 2018 plus local exact replay |
| old `m=5` closed form / `BBBD` profile | conjectured equality | refuted as equality at `k=9`; still a lower construction | same |
| `n(9,6)=473` | verified lower bound | exact maximum after retained 473/474 replay | local exact replay |

The local replay is retained in `evidence/sb_m5_k9_frontier.txt`, its independently checked
tree in `witnesses/majorized_481_5_at9.tree`, and the root-type scan in
`evidence/sb_m5_k9_root_transition.txt`.  The adjacent exact `m=6` boundary is in
`evidence/sb_m6_k9_frontier.txt`.

## Still worth obtaining

These are cited by the papers above but have not yet been checked directly here:

- The *full* Chapter 2, “Weighing Problems,” of Aigner's *Combinatorial Search* (1988): the
  newly supplied scan begins instead at Chapter 3. Chapter 3 is enough for the graph framework
  and exact `m=2,3` cross-check; Chapter 2 would add the original balance/spring-scale account.
- C. A. Christen's earlier Fibonaccian-search reports and the original papers behind Hwang's
  survey: useful only if the historical development becomes part of the paper.  Search Oxford
  SOLO by title/author or request scans through Bodleian document delivery.
- D.-Z. Du and F. K. Hwang, *Combinatorial Group Testing and Its Applications*: useful for a
  modern group-testing overview, but lower priority than the primary exact papers already in
  hand.
- A. Ya. Belokopytov and V. N. Luzgin, “Block transmission of information in a summing
  multiple access channel with feedback,” *Problems of Information Transmission* 23(4) (1987),
  347–351: WCC 2019 attributes the then-best explicit asymptotic constant `1.3277` to it.
  Obtain the primary paper before treating that attribution as a project source.
- A. Ya. Belokopytov, “On the zero error feedback capacity region of the binary adder channel,”
  *Problems of Control and Information Theory* 18(2) (1989), 125–133: WCC 2019 attributes the
  nonconstructive `1.2662` worst-case bound to it. Florin--Ho--Jiang now gives the exact modern
  result, but the original is worth obtaining for the historical proof trail.
- E. Karimi, F. Kazemi, A. Heidarzadeh and A. Sprintson, “A Simple and Efficient Strategy for
  the Coin Weighing Problem with a Spring Scale,” *ISIT* (2018), 1730–1734,
  [arXiv:1805.02977](https://arxiv.org/abs/1805.02977): relevant only as an **average-case**
  study of a broader `d=k=2` weight model, which permits a weight-2 coin. It is not a comparison
  bound for this repository's two distinct-defective, worst-case model.

For the current theorem and implementation work, no paywalled article is blocking progress:
the primary exact `m=5` paper and the main scalable-algorithm papers have all been obtained.
