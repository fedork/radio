# Exact adaptive quantitative group testing with two defectives

<!--
Working manuscript. Numerical tables are generated from data/*.csv and must not be edited by
hand. The remaining publication placeholder is the public artifact DOI.
-->

## Abstract

Exactly two of `n` items are defective. An adaptive test selects a subset and reports whether it
contains zero, one, or two defectives. We determine the largest solvable population for every test
budget through ten, obtaining

`2, 2, 3, 5, 8, 13, 22, 38, 65, 112, 192`,

and determine the complete two-set Pareto frontier through eight tests, together with the exact
cell `n(9,6)=473`. The ten-test upper bound is represented by a finite 2,846,568-claim
certificate that has been checked by an implementation sharing no code with the search program.

The structural result is sharper. Aigner proved that every solvable singleton state is weakly
majorized by an explicit Pascal-type state `G_K` and asked whether the converse holds. We prove
the converse for every `K<=5`, give a short integral counterexample at `K=6`, and construct a
counterexample for every `K>=6`. Thus six tests are the exact first failure level of the
majorization characterization.

## 1. Introduction

The problem is a small but unusually exact instance of adaptive quantitative group testing. Each
test has three possible answers, yet the hypotheses are pairs of items rather than independent
ternary messages. That constraint makes the information bound `3^K` necessary but rarely
sufficient and makes finite optimality substantially harder than construction.

This paper has three contributions:

1. It classifies the singleton-majorization question exactly by test depth: the converse holds
   through `K=5` and fails at every `K>=6`.
2. It determines the complete-graph threshold through ten tests and corrects Aigner's published
   seven-test maximum from 37 to 38. The construction for 38 was already implicit in Gargano et
   al.; the new content at that level is the matching upper boundary.
3. It gives the complete exact `Sb` frontier through eight tests and a separately certified
   `Sa(193)` impossibility proof. Positive results are supplied as explicit strategy trees, while
   negative results are tied to exhaustive records or finite certificates.

The finite-value priority statements below are deliberately corpus-scoped. The search record
includes exhaustive navigable Google Scholar walks for the core sources, complete OpenAlex and
Semantic Scholar seed results, broad discovery searches, primary-text checks, and explicit
unavailable-source limitations; see the
[prior-art audit](../evidence/publication_prior_art_2026-09-02.md).

## 2. Model and notation

A state is a finite set of surviving unordered defective pairs. In Aigner's graph formulation,
items are vertices and possible pairs are edges. A test selects a vertex set; an edge then returns
the number of its endpoints in that set. A strategy is a ternary decision tree, and a state is
*solvable in `K` tests* if every surviving edge is isolated by depth at most `K`.

We use two structured state families:

- `Sa(n)` is the complete graph on `n` items, with mass `binom(n,2)`.
- `Sb(n_1:m_1, ..., n_r:m_r)` is the disjoint union of complete bipartite graphs
  `K_{n_i,m_i}` and has mass `sum_i n_i m_i`. Orientation and part order are immaterial.

One test taking `a_i` and `b_i` vertices from the two sides of part `i` has children

```text
outcome 2: Sb(a_i:b_i),
outcome 0: Sb(n_i-a_i:m_i-b_i),
outcome 1: Sb(a_i:m_i-b_i, n_i-a_i:b_i),
```

with empty parts removed and the contributions from all parent parts united. Any solvable state
has mass at most `3^K`. Deleting candidate edges cannot make a state harder, so a strategy for a
graph also solves every subgraph.

Let `A(K)` be the largest `n` for which `Sa(n)` is solvable in `K` tests. Let `n(K,m)` be the
largest `n>=m` for which `Sb(n:m)` is solvable in `K` tests. An `Sb` state is *singleton* when
every short side equals one, and we identify it with its nonincreasing long-side sequence
`a=(a_1,a_2,...)`. We write `a <=_w b` for weak majorization: every prefix sum of `a` is at most
the corresponding prefix sum of `b`.

## 3. Prior work

[Aigner (1986)](https://doi.org/10.1016/0166-218X(86)90026-0) introduced the graph formulation,
proved the exact fixed-`m=2,3,4` formulas, tabulated small complete bipartite cases, and published
the complete-graph thresholds through seven tests as `3,5,8,13,22,37`. His 1988 monograph
repeats that table and, after proving singleton-majorization necessity, explicitly asks whether
the converse holds.

[Gargano et al. (1992)](https://doi.org/10.1016/0166-218X(92)90260-H) publish exact strategies
for `Sb(14:9)@5` and `Sb(21:17)@6` and the full seven-test `Sb(32:32)` tree. Combining the
`21:17` strategy with Aigner's six-test complete-graph threshold constructs `Sa(38)@7`; the
`32:32` tree constructs `Sa(64)@8`. Accordingly, our 38-coin tree is an independent certificate,
not the first construction, while the 38/39 boundary corrects the earlier claimed maximum. The
exact value 65 improves the located eight-test construction by one.

[Zhang, Berger and Massey (1987)](https://doi.org/10.1109/TIT.1987.1057358) give finite
full-feedback binary-adder codes equivalent to several `Sb` constructions. Hwang's
[1987](https://doi.org/10.2307/2322412) and
[1989](https://doi.org/10.1111/j.1749-6632.1989.tb16406.x) accounts, Hao's
[product construction](https://doi.org/10.1016/0166-218X(90)90022-5), and
[Christen's round survey](https://doi.org/10.1016/0012-365X(94)00106-S) describe the older
finite inputs and asymptotic developments. [Li, Wu and Triesch
(2018)](https://doi.org/10.1016/j.dam.2018.05.026) prove the exact piecewise `m=5` frontier.
Later work determines the sharp asymptotic rate
([Florin, Ho and Jiang 2022](https://doi.org/10.1109/TIT.2021.3137965)); it does not settle the
small finite maxima considered here.

## 4. The singleton-majorization boundary

Set `G_0=(1)`. If `G_{K-1}=(h_1,...,h_s)`, define the zero-padded sequences

```text
L=(h_1,0,h_2,0,...,h_s,0),
M=(h_1,...,h_s,0,...,0),
R=(0,h_1,0,h_2,...,0,h_s),
```

and let `G_K` be the nonincreasing rearrangement of `L+M+R`. For example,
`G_2=(4,3,1,1)` and `G_3=(8,7,4,4,1,1,1,1)`. This is Aigner's canonical singleton strategy;
its mass is `3^K`.

> **Theorem 1 (exact majorization boundary).** A singleton state solvable in `K` tests is weakly
> majorized by `G_K`. Conversely, every `G_K`-majorized singleton state is solvable when
> `K<=5`. The converse is false for every `K>=6`.

For necessity, index the possible test transcripts by `{0,1,2}^K`. Join two transcripts when,
at their first differing coordinate, their symbols are 0 and 2. Hypotheses belonging to one
singleton row must form a stable set in this transcript-conflict graph. Its maximum `t`-colorable
induced subgraph has size equal to the sum of the `t` largest entries of `G_K`, proving every
majorization inequality.

At `K=6`, using `x^r` for `r` copies of `x`, consider

```text
P = (64,63,57^2,42^4,22^7,8^15,7^2,1^32).
```

The state has 64 positive rows, mass `3^6`, and `P <=_w G_6`. It nevertheless has no legal first
split into three `G_5`-majorized children. Tightness at prefix ranks 15 and 32 restricts the row
color counts to two symmetric transitions. Every intervening row must contribute positive mass
to the mixed child, after which one pure child requires 64 units from nine rows that can retain at
most 63. Both transitions are impossible. The complete inequality argument is the
[Tight-Band Capacity Obstruction](../docs/theorems/tight-band-capacity.md); a separate direct-row
enumerator reproduces the failure.

An explicit dyadic balanced-band construction repeats the same obstruction for every higher
depth. At the other side of the boundary, exact prefix-cylinder certificates and an uncapped Hall
check cover all `G_5`-majorized exact-support parents; reduction to smaller support completes the
`K<=5` direction. Thus `K=6` is not merely the first found counterexample but the proved first
failure level. Full statements, reductions, and verification records are collected in the
[singleton-majorization theorem](../docs/theorems/singleton-majorization.md).

## 5. Exact finite thresholds

### 5.1 Complete-graph states

The following are the exact values of `A(K)` through ten tests. Every bare entry is a maximum,
not merely a construction.

<!-- generated:pareto_sa -->
| k | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| max n | 2 | 2 | 3 | 5 | 8 | 13 | 22 | 38 | 65 | 112 | 192 |

Parenthesised means lower bound only. Evidence per row in `data/pareto_sa.csv`.
<!-- /generated -->

The value `A(7)=38` needs careful attribution. Aigner printed 37 as the exact value, while
Gargano et al.'s published `Sb(21:17)@6` strategy already makes 38 achievable. Our independent
tree proves the positive half directly, and the exhaustive rejection of 39 supplies the missing
upper half. Similarly, Gargano et al. reach 64 at eight tests; the exact value `A(8)=65` is a
one-item improvement over that located construction. No source in the audited corpus reaches
65, 112, or 192.

For the ten-test upper bound, `A(9)=112` reduces `Sa(193)` to the sixteen possible first-test
mixed branches

```text
Sb(n:193-n),  97 <= n <= 112,
```

each with nine tests remaining. A cold, proof-safe search rejected all sixteen in one session
after passing an `Sa(192)` control. The raw run used 419,353.1 CPU seconds and 1.32 GB peak RSS;
its provenance and root verdicts are retained in the
[`Sa(193)` record](../evidence/sa193_unsolvable_in_10.txt). Two explicit trees independently
prove `Sa(192)` achievable.

### 5.2 Complete-bipartite states

For fixed `K`, the values `n(K,m)` form a Pareto frontier. The table is complete and exact through
`K=8`. At `K=9`, only `m=1,...,6` are settled; the band `m=7,...,64` is open. At `K=10`, the table
contains the published exact `m=5` value and an unconditional upper bound at `m=6`.

<!-- generated:pareto_sb -->
| m\k | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
|---|---|---|---|---|---|---|---|---|---|---|
| **1** | 2 | 4 | 8 | 16 | 32 | 64 | 128 | 256 | 512 |  |
| **2** |  | 3 | 7 | 15 | 31 | 63 | 127 | 255 | 511 |  |
| **3** |  |  | 5 | 12 | 27 | 58 | 121 | 248 | 503 |  |
| **4** |  |  | 4 | 10 | 24 | 54 | 116 | 242 | 496 |  |
| **5** |  |  |  | 9 | 22 | 50 | 109 | 231 | 481 | 985 |
| **6** |  |  |  | 7 | 19 | 46 | 104 | 225 | 473 | ≤973 |
| **7** |  |  |  |  | 17 | 42 | 97 | 214 |  |  |
| **8** |  |  |  |  | 15 | 38 | 91 | 206 |  |  |
| **9** |  |  |  |  | 14 | 36 | 87 | 198 |  |  |
| **10** |  |  |  |  | 12 | 33 | 82 | 189 |  |  |
| **11** |  |  |  |  | 11 | 31 | 77 | 182 |  |  |
| **12** |  |  |  |  |  | 29 | 73 | 174 |  |  |
| **13** |  |  |  |  |  | 27 | 69 | 168 |  |  |
| **14** |  |  |  |  |  | 25 | 66 | 161 |  |  |
| **15** |  |  |  |  |  | 24 | 63 | 155 |  |  |
| **16** |  |  |  |  |  | 22 | 60 | 150 |  |  |
| **17** |  |  |  |  |  | 21 | 58 | 144 |  |  |
| **18** |  |  |  |  |  | 20 | 55 | 139 |  |  |
| **19** |  |  |  |  |  | 19 | 53 | 135 |  |  |
| **20** |  |  |  |  |  |  | 51 | 130 |  |  |
| **21** |  |  |  |  |  |  | 49 | 126 |  |  |
| **22** |  |  |  |  |  |  | 47 | 122 |  |  |
| **23** |  |  |  |  |  |  | 45 | 118 |  |  |
| **24** |  |  |  |  |  |  | 43 | 115 |  |  |
| **25** |  |  |  |  |  |  | 41 | 111 |  |  |
| **26** |  |  |  |  |  |  | 40 | 108 |  |  |
| **27** |  |  |  |  |  |  | 38 | 105 |  |  |
| **28** |  |  |  |  |  |  | 37 | 102 |  |  |
| **29** |  |  |  |  |  |  | 36 | 100 |  |  |
| **30** |  |  |  |  |  |  | 35 | 97 |  |  |
| **31** |  |  |  |  |  |  | 34 | 94 |  |  |
| **32** |  |  |  |  |  |  | 33 | 92 |  |  |
| **33** |  |  |  |  |  |  |  | 89 |  |  |
| **34** |  |  |  |  |  |  |  | 87 |  |  |
| **35** |  |  |  |  |  |  |  | 85 |  |  |
| **36** |  |  |  |  |  |  |  | 83 |  |  |
| **37** |  |  |  |  |  |  |  | 81 |  |  |
| **38** |  |  |  |  |  |  |  | 79 |  |  |
| **39** |  |  |  |  |  |  |  | 77 |  |  |
| **40** |  |  |  |  |  |  |  | 76 |  |  |
| **41** |  |  |  |  |  |  |  | 74 |  |  |
| **42** |  |  |  |  |  |  |  | 72 |  |  |
| **43** |  |  |  |  |  |  |  | 71 |  |  |
| **44** |  |  |  |  |  |  |  | 69 |  |  |
| **45** |  |  |  |  |  |  |  | 68 |  |  |
| **46** |  |  |  |  |  |  |  | 66 |  |  |
| **47** |  |  |  |  |  |  |  | 65 |  |  |
| **48** |  |  |  |  |  |  |  | 64 |  |  |
| **49** |  |  |  |  |  |  |  | 62 |  |  |
| **50** |  |  |  |  |  |  |  | 61 |  |  |
| **51** |  |  |  |  |  |  |  | 60 |  |  |
| **52** |  |  |  |  |  |  |  | 59 |  |  |
| **53** |  |  |  |  |  |  |  | 58 |  |  |
| **54** |  |  |  |  |  |  |  | 57 |  |  |
| **55** |  |  |  |  |  |  |  | 56 |  |  |
| **65** |  |  |  |  |  |  |  |  | ≥112 |  |
| **66** |  |  |  |  |  |  |  |  | ≥112 |  |
| **67** |  |  |  |  |  |  |  |  | ≥112 |  |
| **68** |  |  |  |  |  |  |  |  | ≥112 |  |
| **69** |  |  |  |  |  |  |  |  | ≥112 |  |
| **70** |  |  |  |  |  |  |  |  | ≥112 |  |
| **71** |  |  |  |  |  |  |  |  | ≥112 |  |
| **72** |  |  |  |  |  |  |  |  | ≥112 |  |
| **73** |  |  |  |  |  |  |  |  | ≥112 |  |
| **74** |  |  |  |  |  |  |  |  | ≥112 |  |
| **75** |  |  |  |  |  |  |  |  | ≥112 |  |
| **76** |  |  |  |  |  |  |  |  | ≥112 |  |
| **77** |  |  |  |  |  |  |  |  | ≥112 |  |
| **78** |  |  |  |  |  |  |  |  | ≥112 |  |
| **79** |  |  |  |  |  |  |  |  | ≥112 |  |
| **80** |  |  |  |  |  |  |  |  | ≥112 |  |
| **81** |  |  |  |  |  |  |  |  | 82–111 |  |
| **82** |  |  |  |  |  |  |  |  | 83–110 |  |
| **83** |  |  |  |  |  |  |  |  | 84–109 |  |
| **84** |  |  |  |  |  |  |  |  | 85–108 |  |
| **85** |  |  |  |  |  |  |  |  | 86–107 |  |
| **86** |  |  |  |  |  |  |  |  | 87–106 |  |
| **87** |  |  |  |  |  |  |  |  | 88–105 |  |
| **88** |  |  |  |  |  |  |  |  | 89–104 |  |
| **89** |  |  |  |  |  |  |  |  | 90–103 |  |
| **90** |  |  |  |  |  |  |  |  | 91–102 |  |
| **91** |  |  |  |  |  |  |  |  | 92–101 |  |
| **92** |  |  |  |  |  |  |  |  | 93–100 |  |
| **93** |  |  |  |  |  |  |  |  | 94–99 |  |
| **94** |  |  |  |  |  |  |  |  | 95–98 |  |
| **95** |  |  |  |  |  |  |  |  | ≤97 |  |
| **96** |  |  |  |  |  |  |  |  | ≤96 |  |

A bare number is a proven maximum. `≥n` is a lower bound (a solution exists, maximality open), `≤n` an upper bound (exhaustively refuted above), `a–b` a two-sided bracket. Per-cell status and evidence are in `data/pareto_sb.csv`.
<!-- /generated -->

Blank cells are not established. A bare number is an exact maximum; `>=`, `<=`, and ranges denote
lower bounds, upper bounds, and two-sided brackets respectively. The per-cell evidence is in
[`pareto_sb.csv`](../data/pareto_sb.csv).

> **Theorem 2 (finite frontiers).** The complete-graph values in the first table are exact through
> `K=10`. Every complete-bipartite cell in the second table is exact through `K=8`, and
> `n(9,6)=473`.

The fixed small-side formulas provide useful checks and attribution:

```text
n(K,1) = 2^K,
n(K,2) = 2^K - 1,
n(K,3) = 2^K - K,
n(K,4) = 2^K - 2K + 2.
```

Aigner proves the `m=2,3,4` formulas; the `m=4` formula is correct in his 1986 paper but is
misprinted in the 1988 exercise. If `F(K)=2^K-K(K-3)/2-5`, Li--Wu--Triesch prove

```text
n(K,5) = F(K)       for 3 <= K <= 8,
         F(K) + 1   for 9 <= K <= 10,
         F(K) + 2   for K >= 11.
```

Aigner's small table also contains the exact isolated cell `n(4,6)=7`. Individual constructions
from Zhang--Berger--Massey and Gargano et al. coincide with several other positive endpoints, but
do not by themselves provide the adjacent negative needed for maximality.

At nine tests, a canonical tree proves `Sb(473:6)` and an exact retained search rejects
`Sb(474:6)`. The formerly proposed continuation

`n(K,6)=2^K-K(K-1)/2-3`

matches `K=4,...,9` but is false at ten: it predicts 976, whereas exhaustive search proves the
unconditional upper bound `n(10,6)<=973`. No unconditional 973 construction is presently known;
a relaxed tree ending in arbitrary majorized singleton states is not a proof in view of Theorem 1.

## 6. Certificates and independent checking

The computation separates the two directions of every exact result.

For achievability, a witness tree records the tested subset at every internal node. The checker
re-derives all three children and accepts a terminal only when it is a substate of the explicit
`G_K` strategy or has another proved base case. In particular, the trees for `Sa(38)`, `Sa(65)`,
`Sa(112)`, and `Sa(192)` are proof objects independent of the search program. Arbitrary weak
majorization is intentionally not accepted as a terminal rule.

For the `Sa(193)` upper bound, the retained cold run was converted to an eight-level certificate
from levels nine through two. It contains exactly 2,846,568 negative claims. At each level, every
possible first split is rejected by the information bound or cites a child claim at the next
level; the level-two support is empty, so the chain terminates in direct refutations. A structural
checker verifies that adjacent levels match exactly.

The semantic audit has two implementations. The frozen refuter reuses the production solver core.
Separately, `tools/cleanroom` was written from the mathematical specification, shares no source
with the solver, reconstructs the split space, and verified all 2,846,568 claims with zero gaps.
The certificate format and exact level counts are documented in
[`sa193-certificate.md`](../docs/sa193-certificate.md), and the independent run is recorded in
[`cleanroom_verifier_2026-09-01.txt`](../evidence/cleanroom_verifier_2026-09-01.txt).

The `K<=8` bipartite frontier is backed cell by cell by positive and negative boundary records.
No negative from the unreliable 2023 solver era is used as proof. The source-of-truth CSV records
whether each entry is a maximum, a lower bound, or an upper bound and identifies its retained or
published source.

## 7. Reproducibility and scope

The reproduction package contains the trimmed negative certificate, the structural and semantic
checkers, both `Sa(192)` witness trees, the relevant evidence records, and exact hashes. It has
been extracted in a clean directory and exercised end to end. A public archival DOI will replace
this sentence before submission.

The literature audit found no earlier result reaching `A(8)=65`, `A(9)=112`, or `A(10)=192`, no
published upper boundary correcting `A(7)`, and no resolution of Aigner's singleton-majorization
question in the audited corpus. This is not asserted as proof of worldwide absence. Scholar limits
the number of navigable results for some records, Christen's 1980 report and 1986 talk remain
unavailable, and seven interleaved pages of the relevant Du--Hwang chapter could not be read. The
available target subsection and the later sources expose only the already credited asymptotic
chain. Exact search coverage and exclusions are preserved in the
[audit record](../evidence/publication_prior_art_2026-09-02.md).

## 8. Conclusion

The exact-count oracle has enough structure to make weak majorization a sharp characterization
through five tests, but not beyond. The first obstruction appears at six tests as a thin integral
hole between two tight Pascal ranks, and the same mechanism persists at every higher depth. On the
finite side, explicit witnesses and independently checkable negative certificates extend the
complete-graph threshold through ten tests and the complete-bipartite frontier through eight.
Together these results turn a finite search into a structural boundary theorem with a reproducible
computational component.
