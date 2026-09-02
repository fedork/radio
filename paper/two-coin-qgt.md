<!--
Imported 2026-08-02 from the Google Docs export, punctuation unescaped.

KNOWN WORK REMAINING - see docs/research-plan.md item P5:
  - <TODO> sections: Terminology, Unit Group Triviality Lemma, Insights
  - add lemma (12) for m=8
  - THE DRAFT PREDATES THE STRONGEST RESULTS AND DOES NOT YET CONTAIN THEM:
      * singleton majorization by G_K is necessary but NOT sufficient - the K=6
        counterexample and the infinite family for every K>=6, which answer negatively the
        converse Aigner 1988 left open after Prop. 3.25;
      * the converse IS true for K<=5 (exhaustive, 2026-08-31);
      * the certified-verification architecture: the 2,846,568-claim certificate and its
        independent re-verification by tools/cleanroom.
    A restructure around these, rather than more polishing of the sections below, is the
    remaining work. See docs/publishable-claims.md for the claim inventory and its
    still-outstanding novelty audit.
  - fixed 2026-09-02: the hand-typed Sb frontier is now a generated block; the (7)/(5)
    numbering collision, the dead image reference and the mangled G_k formula are resolved.
Do not treat any number in this file as authoritative; data/*.csv is.
-->

**Introduction**

A specific case of quantity group testing is considered: given a group on *n* coins, two of which are known to be defective, and a test procedure that reports the number of defective coins in a tested subset (0, 1 or 2), determine the largest *n* for which both coins can be detected with at most *k* adaptive tests. The computation proves the `Sa` sequence through *k*=10 and extends the exact and constructive record for the two-set `Sb` problem. Every computational claim is separated into a verified lower witness and, where available, an exhaustive or published upper bound.

**Relation to prior work**

Aigner's graph formulation identifies `Sa(n)` with search on the complete graph `K_n` and
`Sb(n:m)` with search on `K_{m,n}`; his canonical sequence is the predecessor of the `G_k`
sequence used below ([Aigner 1986](https://doi.org/10.1016/0166-218X(86)90026-0)).  The exact
`m=4` result and the piecewise exact `m=5` result are due to Li, Wu and Triesch
([2018](https://doi.org/10.1016/j.dam.2018.05.026)).  Hao's product inequality
([1990](https://doi.org/10.1016/0166-218X(90)90022-5)) and the explicit recursive algorithm of
Gargano, Montuori, Setaro and Vaccaro
([1992](https://doi.org/10.1016/0166-218X(92)90260-H)) provide the scalable asymptotic context.
Jiang, Polyanskii and Vorobyev give an explicit near-optimal mixed construction
([2019](https://www.lebesgue.fr/sites/default/files/proceedings_WCC/WCC_2019_paper_65.pdf));
Florin, Ho and Jiang determine the sharp leading asymptotic rate
([2022](https://doi.org/10.1109/TIT.2021.3137965)). Neither result is a finite fixed-`m` Pareto
maximum.
Hwang's survey ([1987](https://doi.org/10.2307/2322412)) is a concise historical introduction
to the two-coin quantitative model.  Full source notes and cautions are maintained in
`../docs/literature.md`.

**Terminology and notation**

Sa, Sb, Sbb, k, zero group, unit group, etc\<TODO>

**Results**

**Maximum solvable Sa states**

The following are maximum values of *n* for a given *k* such that Sa(*n*) can be solved in *k*
tests. Every entry through *k*=10 is proven maximal. For *k*=1 through 9, retained exhaustive logs
contain the positive boundary and the rejection immediately above it. For *k*=10, an independently
checked witness proves Sa(192) achievable and a proof-safe cold exhaustive run rejects Sa(193).

<!-- generated:pareto_sa -->
| k | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| max n | 2 | 2 | 3 | 5 | 8 | 13 | 22 | 38 | 65 | 112 | 192 |

Parenthesised means lower bound only. Evidence per row in `data/pareto_sa.csv`.
<!-- /generated -->

For the last upper bound, the proven *k*=9 maximum Sa(112) reduces Sa(193) to the sixteen states
`Sb(n1:193-n1)` with `97<=n1<=112`. Cold run9 rejected all sixteen in one uninterrupted session,
after its Sa(192) positive control passed. It returned UNSOLVABLE in 419353.1 CPU seconds with
1.32 GB peak RSS. The raw, fully provenanced source is archived as
`sa193-cold-2026-08-16:run9_out_sa193.txt`; the sixteen root lines and hashes are committed in
`../evidence/sa193_unsolvable_in_10.txt`. The matching 2023 result is not used because that build
produced known false negatives.

**Maximum solvable Sb states with size 1**

Write `n(k,m)` for the largest `n1` with `Sb(n1 : m)` solvable in `k` tests, where `n1 >= m`.
For each `k` these values form a Pareto frontier: every state on or below the line is solvable
in `k`, every state above it is not. The frontier is complete and proven maximal for
`k <= 8`. At `k = 9` only `m = 1..6` are settled, of which `m = 1..5` are published results
(Aigner 1986 for `m = 2, 3`; Li--Wu--Triesch 2018 for `m = 4, 5`) and `m = 6` is established
here; the band `m = 7..64` is open. At `k = 10` only `m = 5` (published) and an upper bound at
`m = 6` are known.

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

Blank means not established; `≤` marks an upper bound without a matching construction.
Evidence per cell is in `data/pareto_sb.csv`; this block is rendered by
`tools/check_tables.py --render` and must never be edited by hand.

**“Unit Group Triviality Lemma”** 

Unit group (1:1) \<TODO>

**Insights into relative solvability of various states** 

\<tables>

**Special case analysis**

**Sb(2^k:1) (1)**  
For Sb(n:1), since one of the coins is already known in this case, the solution is reduced to a single-coin problem for which the optimal strategy is dichotomy. Therefore it is easy to see that maximum n is equal to 2^k for a given k. 

***(2)*** **Sb(2^k:1, 2^k-1:1)**  
***(2.1)*** For k=0 this case becomes Sb(1:1,0:1), removing zero group reduces it to Sb(1:1) which is solvable in k=0

***(2.2)*** If ***(2)*** is solvable in (k-1) then it is also solvable in k with [2^(k-1) : 1, 2^(k-1)-1:0] producing Sb(2^(k-1):1) (solvable in k-1 per ***(1)***) for both 0 and 2 test results and Sb(2^(k-1):1, 2^(k-1)-1:1) **(2)**

From ***(2.1)*** and ***(2.2)*** by induction it follows that ***(2)*** is true for any k>=0

***(3) Sb(2^k-1:2)***   
For Sb(n:2) the maximum n is equal to 2^k-1 for a given k and the optimally solved with [2^(k-1):1]=>  
		Sb(2^(k-1) : 1) (solvable per ***(1)***) ,    
Sb((2^(k-1) -1) : 1) (solvable in k-1 implied from previous) and   
Sb(Sb(2^(k-1) : 1 , (2^(k-1) -1) : 1) (solvable per (2));

***(4)*** **Sb(2^k:1, 2^k-1:1, 2^k-k-1:1)** is solvable in k with [2^(k-1):0, 2^(k-1):1, 2^(k-1)-1:1] \=>  
Sb(2^(k-1):1, 2^(k-1):1) (solvable per (2)),  
Sb(2^(k-1):1) (solvable per (1)) and  
Sb(2^(k-1):1, 2^(k-1):1, 2^(k-1) - (k-1) - 1:1) (since (2^k - k -1) - (2^(k-1)-1) \= 2^(k-1) - k \= 2^(k-1) - (k - 1) - 1) which is solvable per (4) by induction.

***(5)*** **Sb(2^k-1:2, 2^k-k:1)** is solvable with [2^(k-1)-1:1, 2^(k-1):1] \=>  
	Sb(2^(k-1)-1:1, 2^(k-1):1) (solvable per (2)),  
	Sb(2^(k-1):1) (solvable per (1)) and  
	Sb(2^(k-1):1, 2^(k-1)-1:1, 2^(k-1) - k:1) which is solvable per (4) since 2^(k-1) - k \= 2^(k-1) - (k-1) - 1

***(6)*** **Sb(2^k-k:3)** is solvable in k with [2^(k-1)-1:1] \=>  
Sb(2^(k-1)-1:1) (solvable per (1))  
	Sb(2^(k-1)-k:2) (solvable per (3) since 2^(k-1)-k \= 2^(k-1)-(k-1)-1 ), and  
	Sb(2^(k-1)-1:2, 2^(k-1)-(k-1):1) solvable per (5)  

(8) `n(k,4)=2^k-2k+2` exactly for `k>=3` (Li--Wu--Triesch, Corollary 3).  The local
construction remains useful as an independent lower proof; the published theorem supplies the
upper bound.

Entries of `G_k` come in dyadic blocks of sizes `1, 1, 2, 4, ..., 2^(k-1)`, and every entry in
block `r` (zero-indexed) equals the partial binomial sum `sum_{i=0}^{k-r} C(k, i)`; the block
sizes double, and the total is `sum G_k = 3^k`. Verified against the defining recurrence for
`k <= 12` in `tools/cleanroom`'s test suite.

**Proposed unproven lemmas:**

**(u1)** if Sb(n1 : n2) for any n1>=n2 can be solved in k then Sb( (n1+1) : (n2-1) ) can also be solved in k.

**(u1)** is verified on all 130 proven cells for `k = 1..8`, and exhaustively for every
one-part state at `k <= 5`, but no proof is known; two natural proof routes were refuted on
2026-08-03. Its multi-part analogue is outright false: `Sb(15:2, 5:4)` is solvable in 4 while
`Sb(15:2, 6:3)` is not, despite the lower mass.

(9) The former formula `F(k)=2^k-k(k-3)/2-5` is exact only through `k=8`.
Li--Wu--Triesch, Theorems 1--3 and Remark 1, prove

```
n(k,5) = F(k)       for 3 <= k <= 8,
         F(k) + 1   for 9 <= k <= 10,
         F(k) + 2   for k >= 11.
```

In particular `n(9,5)=481`, not 480.  The published theorem proves achievability, and
`evidence/sb_m5_k9_frontier.txt` independently records the exact rejection of 482.  The file
`witnesses/majorized_481_5_at9.tree` is conditional on the open singleton-majorization converse.
The observed root changes from type `3+2` through `k=8` to type `4+1` at `k=9`, matching the
published construction.
(11) Sb(2^k-k^2+4k-10 : 7) solvable in k

**Proposed and refuted lemmas:**  

(10) `Sb(2^k-k(k-1)/2-3 : 6)` is solvable in `k`.  It matches `k=4..9` but is false at
`k=10`: it predicts 976, whereas exhaustive search gives the unconditional upper bound 973.  See
`../docs/theorems/special-cases.md` and `../evidence/sb_m6_k10_frontier.txt`.

**Solution for 192 coins in 10 tests:**

The full witness tree is `witnesses/sa192_k10_a.tree` in the repository, with a second,
slightly smaller witness in `witnesses/sa192_k10_b.tree`. Both are verified by
`tools/check_witness.py`: every split is re-derived from the recorded test, every reference
is checked to dominate its child after unit-group stripping, and the information bound is
checked at every node. Maximality is supplied separately by the cold `Sa(193)` refutation above.
The witness should be reproduced in an appendix for the final version.
