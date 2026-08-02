<!--
Imported 2026-08-02 from the Google Docs export, punctuation unescaped.

KNOWN WORK REMAINING - see docs/research-plan.md item P5:
  - the K=8 table below is STALE at m=10..17; correct values are in data/pareto_sb.csv
  - lemma numbering: (7) duplicates (5); "(7) holds true k up to 8" refers to (u1)
  - <TODO> sections: Terminology, Unit Group Triviality Lemma, Insights, Refuted lemmas
  - lemma (10) has been corrected here already: k(k-1)/2, not k(k-5)/2
  - add lemma (12) for m=8 and the G_k = sum-of-binomials closed form
  - the Sa table should mark k<=9 proven vs k=10 constructed
Do not treat any number in this file as authoritative; data/*.csv is.
-->

**Introduction**

A specific case of quantity group testing is considered: given a group on *n* coins 2 of which are known to be defective and a test procedure that gives the number of defective coins in a subset of coins (0,1 or 2 in this case) determine the largest *n* for which both coins can be detected with at most *k* tests. Best solutions for *k* up to 10 are found and proven by exhaustive search using a computer program provided with additional intermediate results and lemmas.

**Terminology and notation**

Sa, Sb, Sbb, k, zero group, unit group, etc\<TODO>

**Results**

**Maximum solvable Sa states**

The following are maximum values of *n* for a given *k* such that Sa(*n*) can be solved in *k* tests. Results for k up to 9 are proven to be the best possible results by exhaustive search using the provided Program. Best results for k up to 7 have been previously published (without proof of them being best), so results for k>=8 are new as well as proof for non-trivial values of k are new.

| k | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
| :---- | :---- | :---- | :---- | :---- | :---- | :---- | :---- | :---- | :---- | :---- | :---- |
| Max n | 2 | 2 | 3 | 5 | 8 | 13 | 22 | 38 | 65 | 112 | 192 |

**Maximum solvable Sb states with size 1**

The following table enumerates all *max* values of single-group Sb states for k up to 8 ( Sb(n1 : n2) is considered to be *max* iff Sb(n1 : n2)  can be solved in k tests, but both Sb((n1+1) : n2) and Sb(n1 : (n2+1)) cannot. Therefore the set of these values for a given k forms a pareto front such that all values on or below the line are solvable in k and all values above it are not. The following table lists the maximum value of n1 for given k and n2 where n1>=n2 and Sb(n1 : n2) is solvable in k.

| n2\\K | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
| ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- | ----- |
| **1** | 2 | 4 | 8 | 16 | 32 | 64 | 128 | 256 |
| **2** |  | 3 | 7 | 15 | 31 | 63 | 127 | 255 |
| **3** |  |  | 5 | 12 | 27 | 58 | 121 | 248 |
| **4** |  |  | 4 | 10 | 24 | 54 | 116 | 242 |
| **5** |  |  |  | 9 | 22 | 50 | 109 | 231 |
| **6** |  |  |  | 7 | 19 | 46 | 104 | 225 |
| **7** |  |  |  |  | 17 | 42 | 97 | 214 |
| **8** |  |  |  |  | 15 | 38 | 91 | 206 |
| **9** |  |  |  |  | 14 | 36 | 87 | 198 |
| **10** |  |  |  |  | 12 | 33 | 82 | 182 |
| **11** |  |  |  |  | 11 | 31 | 77 | 176 |
| **12** |  |  |  |  |  | 29 | 73 | 170 |
| **13** |  |  |  |  |  | 27 | 69 | 165 |
| **14** |  |  |  |  |  | 25 | 66 | 159 |
| **15** |  |  |  |  |  | 24 | 63 | 153 |
| **16** |  |  |  |  |  | 22 | 60 | 148 |
| **17** |  |  |  |  |  | 21 | 58 | 142 |
| **18** |  |  |  |  |  | 20 | 55 | 139 |
| **19** |  |  |  |  |  | 19 | 53 | 135 |
| **20** |  |  |  |  |  |  | 51 | 130 |
| **21** |  |  |  |  |  |  | 49 | 126 |
| **22** |  |  |  |  |  |  | 47 | 122 |
| **23** |  |  |  |  |  |  | 45 | 118 |
| **24** |  |  |  |  |  |  | 43 | 115 |
| **25** |  |  |  |  |  |  | 41 | 111 |
| **26** |  |  |  |  |  |  | 40 | 108 |
| **27** |  |  |  |  |  |  | 38 | 105 |
| **28** |  |  |  |  |  |  | 37 | 102 |
| **29** |  |  |  |  |  |  | 36 | 100 |
| **30** |  |  |  |  |  |  | 35 | 97 |
| **31** |  |  |  |  |  |  | 34 | 94 |
| **32** |  |  |  |  |  |  | 33 | 92 |
| **33** |  |  |  |  |  |  | 32 | 89 |
| **34** |  |  |  |  |  |  |  | 87 |
| **35** |  |  |  |  |  |  |  | 85 |
| **36** |  |  |  |  |  |  |  | 83 |
| **37** |  |  |  |  |  |  |  | 81 |
| **38** |  |  |  |  |  |  |  | 79 |
| **39** |  |  |  |  |  |  |  | 77 |
| **40** |  |  |  |  |  |  |  | 76 |
| **41** |  |  |  |  |  |  |  | 74 |
| **42** |  |  |  |  |  |  |  | 72 |
| **43** |  |  |  |  |  |  |  | 71 |
| **44** |  |  |  |  |  |  |  | 69 |
| **45** |  |  |  |  |  |  |  | 68 |
| **46** |  |  |  |  |  |  |  | 66 |
| **47** |  |  |  |  |  |  |  | 65 |
| **48** |  |  |  |  |  |  |  | 64 |
| **49** |  |  |  |  |  |  |  | 62 |
| **50** |  |  |  |  |  |  |  | 61 |
| **51** |  |  |  |  |  |  |  | 60 |
| **52** |  |  |  |  |  |  |  | 59 |
| **53** |  |  |  |  |  |  |  | 58 |
| **54** |  |  |  |  |  |  |  | 57 |
| **55** |  |  |  |  |  |  |  | 56 |
|  |  |  |  |  |  |  |  |  |

![][image1]

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
(7) **Sb(2^k-1:2, 2^k-k:1)**

(8) Sb(2^k-2k+2 : 4) solvable in k   
…..

3k-2k \= i=0k-12il=0ik!/(l!(k-l)!) where l=0ik!/(l!(k-l)!) is partial sum of binomial sequence

**Proposed unproven lemmas:**

**(u1)** if Sb(n1 : n2) for any n1>=n2 can be solved in k then Sb( (n1+1) : (n2-1) ) can also be solved in k.

(7) holds true k up to 8 and is expected to also hold for any k, but the author was unable to find a rigorous proof. 

(9) Sb(2^k-k(k-3)/2-5 : 5) solvable in k  
(10) Sb(2^k-k(k-1)/2-3 : 6) solvable in k  
(11) Sb(2^k-k^2+4k-10 : 7) solvable in k

**Proposed and refuted lemmas:**  
 \<TODO>

**Solution for 192 coins in 10 tests:**

The full witness tree is `witnesses/sa192_k10_a.tree` in the repository, with a second,
slightly smaller witness in `witnesses/sa192_k10_b.tree`. Both are verified by
`tools/check_witness.py`: every split is re-derived from the recorded test, every reference
is checked to dominate its child after unit-group stripping, and the information bound is
checked at every node. Reproduced in an appendix for the final version.
