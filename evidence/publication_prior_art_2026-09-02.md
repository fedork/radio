# Publication prior-art audit, 2026-09-02

This is the reproducible record of the publication search begun from
[`docs/publication-handover.md`](../docs/publication-handover.md). The decisive correction from
the second pass is positive, not negative: Gargano--Montuori--Setaro--Vaccaro 1992 publishes an
exact `Sb(32:32)` decision tree and also states that `K_{21,17}` is 3-optimal. Together with
Aigner's earlier complete-graph constructions, these give published constructions for `Sa(64)`
in eight tests and `Sa(38)` in seven tests. The local trees are independent certificates, but
they are not the first published achievability proofs.

No finite source located in the corpus below states or implies `Sa(65)`, `Sa(112)`, or `Sa(192)`,
and none proves the upper boundary `Sa(38) <= 7`. That is a **corpus-scoped conclusion**, not a
proof that an unindexed or inaccessible publication does not exist. The remaining access gaps
are listed explicitly at the end. The Google Scholar cited-by pass and Hwang 1989 are closed;
the inaccessible book pages and Christen originals are retained as disclosed limitations, not
active search tasks.

## Finite results found in primary sources

### Aigner 1986

Source: M. Aigner, “Search problems on graphs,” *Discrete Applied Mathematics* 14 (1986),
215--230, [DOI 10.1016/0166-218X(86)90026-0](https://doi.org/10.1016/0166-218X(86)90026-0).

- Corollaries 2, 3 and 4 give the exact complete-bipartite formulas for `m=2`, `m=3` **and
  `m=4`**. In particular, Corollary 4 prints the correct
  `c(K_{4,n}) <= k <=> n <= 2^k - 2k + 2` for `k >= 3`. The later typo is confined to
  Aigner's 1988 Exercise 3.3.2; Li--Wu--Triesch 2018 is an independent later proof, not the
  first published proof of the `m=4` formula.
- Figure 4 is an exact table of `c(K_{m,n})` for the displayed small pairs. It prints
  `c(K_{6,7})=4` and `c(K_{6,8})=5`, so the frontier cell `n(4,6)=7` was already published.
  It also supplies numerous finite constructions that are below later frontiers.
- Page 226 defines `h(k)` by `c(K_n) <= k <=> n <= h(k)`, exactly the present `Sa(k)`.
  Figure 5 prints `3,5,8,13,22,37` for `k=2,...,7` and calls 37 the correct final value.
  Thus the exact complete-graph thresholds through six tests are published; the seven-test
  upper claim is false.
- The same page records `c(K_{9,13})=5` and `c(K_{15,22})=6`. These are construction inputs,
  not complete fixed-`m` frontier theorems.

The full page images and extracted text were checked, not merely the abstract or metadata.

### Aigner 1988

Source: M. Aigner, *Combinatorial Search* (Wiley--Teubner, 1988); local Chapter 3 scan and
[indexed Google Books record](https://books.google.com/books?id=JyRDAQAAIAAJ).

- Section 3.3 supplies the exact graph translation, Subgraph Monotonicity, the canonical
  singleton sequence `N(k)`, and the open converse after Proposition 3.25.
- Exercise 3.3.1 gives exact `m=3`; Exercise 3.3.2 misprints the `m=4` formula as
  `2^k-k+2`, even though the 1986 journal paper has the correct `2^k-2k+2`; Exercise 3.3.3
  records `M(K_{5,9})=4` and `M(K_{9,14})=5`.
- Chapter 2, p. 102, defines the same complete-graph threshold as `m^(2)(k)` and repeats
  `3,5,8,13,22,37` in Figure 2.13.

The supplied scan does not contain Chapter 2. The p. 102 statements were checked in indexed
source text, so a page-by-page Chapter 2 inspection remains an access task.

### Zhang--Berger--Massey 1987

Source: Z. Zhang, T. Berger and J. L. Massey, “Some families of zero-error block codes for the
two-user binary adder channel with feedback,” *IEEE Transactions on Information Theory* 33(5)
(1987), 613--619, [DOI 10.1109/TIT.1987.1057358](https://doi.org/10.1109/TIT.1987.1057358),
[author-hosted PDF](https://www.isiweb.ee.ethz.ch/archive/massey_pub/pdf/BI427.pdf).

Full-feedback uniquely decodable pairs are exactly `Sb` strategies under the standard coding
translation. The paper explicitly gives `(5,3)` in three uses, `(5,9)` in four, `(8,14)` in
five, and `(5,45)` in six, as well as infinite recurrence families. These are genuine finite
prior constructions. Among the displayed points, `(5,3)` and `(5,9)` lie on local frontiers, but the
paper does not supply all adjacent impossibility results needed for a frontier table; the other
listed points lie below the present frontiers.

### Gargano--Montuori--Setaro--Vaccaro 1992

Source: L. Gargano, V. Montuori, G. Setaro and U. Vaccaro, “An improved algorithm for
quantitative group testing,” *Discrete Applied Mathematics* 36 (1992), 299--306,
[DOI 10.1016/0166-218X(92)90260-H](https://doi.org/10.1016/0166-218X(92)90260-H).

- Lemma 2.3 states `T(32,32)=7`; the appendix, Figures 1--3, is the complete decision tree.
  Equality follows from the information bound as well as from the paper's statement that
  `K_{32,32}` is 3-optimal.
- The conclusion on p. 305 says that analysis of the appendix algorithm also makes
  `K_{14,9}` and `K_{21,17}` 3-optimal. Hence `T(14,9)=5` and `T(21,17)=6` exactly.
- These exact costs do **not** alone prove the maxima `n(5,9)=14` or `n(6,17)=21`: a larger
  neighboring bipartite graph can still fit under the same ternary information bound. They
  publish the positive half of those local frontier points.
- A footnote on p. 300 says a referee reported that Christen announced an unpublished result
  similar to this algorithm at the 1986 SIAM Conference on Discrete Mathematics. This is not a
  publication, but it is a historical-priority caveat and makes the unobtained talk worth checking.

Two immediate complete-graph consequences were not called out as separate theorems in the
paper, but are obtained by its constructions and the standard first-test decomposition:

1. Split 38 coins into parts of 21 and 17. The mixed outcome is `Sb(21:17)`, solvable in six
   further tests by Gargano et al.; either pure outcome is a complete graph on at most 21
   vertices, solvable in six further tests by Aigner's published `h(6)=22`. Therefore
   `Sa(38)` is solvable in seven tests.
2. Split 64 coins into two parts of 32. The mixed outcome is `Sb(32:32)`, solvable in seven
   further tests by the appendix tree; either pure outcome `Sa(32)` is solvable in seven
   further tests by Aigner. Therefore `Sa(64)` is solvable in eight tests.

Consequently, the repository's checked `sa38_k7.tree` is an independent certificate and the
local 38/39 boundary establishes the exact correction to Aigner, but the *achievability* of 38
was already implicit in published constructions. At eight tests, the prior construction reaches
64; the repository's exact `Sa(8)=65` improves it by one.

### Later exact theorem

Li--Wu--Triesch 2018 proves the exact piecewise `m=5` frontier and reproves the `m=4` formula:
S. Li, X. Wu and E. Triesch, “A ternary search problem on two disjoint sets,” *Discrete Applied
Mathematics* 251 (2018), 221--235,
[DOI 10.1016/j.dam.2018.05.026](https://doi.org/10.1016/j.dam.2018.05.026).
The first publication priority for `m=4`, however, is Aigner 1986 Corollary 4.

## Cell-honest effect on the claimed finite results

| repository result | publication status after this audit |
|---|---|
| `Sa(k)` through `k=6` | exact values published by Aigner 1986 |
| `Sa(7)=38` | 38-achievability follows from Gargano's published `Sb(21:17)@6`; no prior upper proof or explicit corrected threshold was located |
| `Sa(8)=65` | Gargano's `Sb(32:32)@7` gives the prior lower bound 64; no source located reaches 65 |
| `Sa(9)=112`, `Sa(10)=192` | no finite statement or construction reaching either value located in the searched corpus |
| fixed `m=2,3,4` frontiers | exact formulas published by Aigner 1986 |
| fixed `m=5` frontier | exact piecewise formula published by Li--Wu--Triesch 2018 |
| `n(4,6)=7` | exact cell already printed in Aigner 1986 Figure 4 |
| `n(5,9)=14` | prior constructions at the endpoint (Aigner 1988; Zhang--Berger--Massey 1987; Gargano 1992); the endpoint's maximality was not located |
| `n(6,17)=21` | prior construction at the endpoint (Gargano 1992); the endpoint's maximality was not located |
| `Sb(32:32)@7` | exact published tree, Gargano 1992; it is below the local `n(7,32)=33` frontier |
| remaining `m>=6` cells through `k=8`, and `n(9,6)=473` | no published maxima located; individual older lower constructions must still be credited where they occur |

This table deliberately separates “a strategy for the endpoint” from “the endpoint is maximal.”
A 3-optimal graph has minimum cost equal to its information lower bound, but that does not rule
out a larger graph with the same cost.

## Backward source closure

The following same-model sources were read in full or through the relevant complete sections:

- F. K. Hwang, “A Tale of Two Coins” (1987),
  [DOI 10.2307/2322412](https://doi.org/10.2307/2322412): the model-Q Fibonacci construction,
  but no later finite threshold table.
- F. K. Hwang, “Updating a Tale of Two Coins” (1989), pp. 259--265,
  [DOI 10.1111/j.1749-6632.1989.tb16406.x](https://doi.org/10.1111/j.1749-6632.1989.tb16406.x):
  the complete seven-page article was obtained through Oxford and inspected page by page. Model Q
  on pp. 264--265 is exactly the unrestricted adaptive two-irregular model with feedback equal to
  0, 1, or 2 irregulars in the tested set. Page 265 calls the halving procedure best only in the
  one-pair class and Christen's Fibonaccian procedure best only in the two-pair class. It then
  attributes to Christen's 1986 talk cardinality-reduction constructions by factors 7, 12, 20,
  and 33 in 4, 5, 6, and 7 tests, respectively, and reports “the best result so far for model Q”
  as the asymptotic upper bound `T_Q(n) <= 1.3877 log_2 n`. These are constructions and
  restricted-class optima, not exact finite `Sa` or `Sb` maxima. The inspected PDF has SHA-256
  `f2a6bc3359266635d8d3be8f917fc3f5639bc34a5dd57f7bd30655375a8e9d4f` and is retained outside
  the repository.
- F. H. Hao, “The optimal procedures for quantitative group testing” (1990),
  [DOI 10.1016/0166-218X(90)90022-5](https://doi.org/10.1016/0166-218X(90)90022-5): product
  inequalities and limit existence; its finite input `T(15,22)=6` is cited from Aigner.
- C. A. Christen, “Search problems: one, two or many rounds” (1994),
  [DOI 10.1016/0012-365X(94)00106-S](https://doi.org/10.1016/0012-365X(94)00106-S): repeats
  `T_A(32,32)=7` and surveys the older asymptotic coefficients. The quantities 7, 12 and 20 in
  its recursive asymptotic bounds are not claims that `T(7,7)=4`, `T(12,12)=5`, or
  `T(20,20)=6`.
- Jiang--Polyanskii--Vorobyev 2019,
  [proceedings PDF](https://www.lebesgue.fr/sites/default/files/proceedings_WCC/WCC_2019_paper_65.pdf):
  reviews the exact same historical chain and adds an explicit asymptotic construction. It cites
  Belokopytov--Luzgin's very large family
  `K_{2^(235n+61),2^(235n+61)}` in `312n+123` tests; this has no small-`k` frontier consequence.
- Florin--Ho--Jiang 2022,
  [DOI 10.1109/TIT.2021.3137965](https://doi.org/10.1109/TIT.2021.3137965): proves the exact
  equivalence between `Sb(n:n)` strategies and full-feedback binary-adder codes, reviews the
  same finite sources, and settles the asymptotic rate rather than the small finite table.
- Bshouty 2009,
  [COLT PDF](https://www.learningtheory.org/colt2009/papers/004.pdf): “optimal” there is a
  constant-factor guarantee for the general spring-scale problem, not a finite exact result.
- Karimi--Kazemi--Heidarzadeh--Sprintson 2018,
  [arXiv:1805.02977](https://arxiv.org/abs/1805.02977): average-case and a broader weight model
  permitting one weight-2 coin, not the present distinct-two-defective worst case.

The two modern same-model papers' bibliographies return to Aigner, Christen, Hao, Gargano,
Belokopytov and the feedback-coding papers; they did not reveal another finite exact table.

## Forward citations and broad discovery

Eight seed records were used: Aigner 1986, Hwang 1987, Hwang 1989, Andreae 1989, Hao 1990,
Gargano 1992, Christen 1994 and Li--Wu--Triesch 2018.

### OpenAlex

The API counts on 2026-09-02 were respectively 36, 31, 3, 3, 11, 16, 3 and 0. Deduplicating
all returned forward citations gave 69 works. The query form is:

```text
https://api.openalex.org/works?per-page=200&filter=cites:W1977708615
```

with seed IDs `W1977708615`, `W2032413558`, `W2136029256`, `W1974102038`, `W2081703448`,
`W2066296454`, `W1972805208`, and `W2807652950`. All 69 titles were screened; the same-model
candidates were then checked in primary text. The other hits were binary-outcome group testing,
parity/balance/underweight models, nonadaptive QGT, noisy or probabilistic variants, graph
reconstruction, or general coding work.

Four independent OpenAlex discovery searches were exhausted rather than stopped at the first page:

```text
"quantitative group testing"               138 records
"ternary search" graph                     216 records
"coin weighing" "spring scale"             56 records
"binary adder channel" feedback             39 records
```

Their deduplicated union contained 424 records. Title screening and primary-text follow-up found
the sources above, plus near-misses recorded below, but no additional finite `Sa` threshold or
complete `Sb` frontier.

Crossref `query.bibliographic` searches with the same four concept families were also used as a
noisy discovery supplement. Crossref's token matching reported millions of nominal hits, so only
the first 1,000 relevance-ranked records per query were screened and this is **not** counted as an
exhausted database. The useful additions it surfaced were the Zhang--Berger--Massey, Kramer,
Deppe--Lebedev, Bshouty and Karimi items already classified above or below.

### Semantic Scholar and publisher citation lists

Semantic Scholar returned 38, 33, 4, 2, 11, 16, 2 and 0 forward-citation records for the same
eight DOI seeds (106 records before cross-seed deduplication). These sets were title-screened and
the technically relevant additions checked. The reproducible endpoint is:

```text
https://api.semanticscholar.org/graph/v1/paper/DOI:10.1016%2F0166-218X%2886%2990026-0/citations?limit=1000&fields=title,year,externalIds
```

The complete ACM forward-citation list for Aigner's book (43 entries) was separately reviewed.
It contains no resolution of the singleton-majorization converse, but is known to be incomplete:
Li--Wu--Triesch cite the book and do not appear there.

### Google Scholar cited-by pages

The signed-in in-app Google Scholar UI was used on 2026-09-02. Every accessible cited-by page was
walked to its terminal page and every displayed title was screened:

| seed | pages inspected | raw records | terminal condition |
|---|---:|---:|---|
| Aigner, “Search problems on graphs” (1986) | 1--6 | 54 | page 6 had four records and no next page |
| Aigner, *Combinatorial Search* (1988) | 1--18 | 180 | page 19 was empty and no next page was exposed |
| Gargano et al. (1992) | 1--3 | 30 | no next page after page 3 |
| Li--Wu--Triesch (2018) | 0 | 0 | the exact record exposed no cited-by link |

Scholar displayed an approximate count near 350 for the Aigner book while exposing only 180
records. The 180-record result is therefore exhaustive only for the pages Scholar made
navigable, not a claim that every item behind the approximate count was returned. The four
walks produced 264 raw rows and 227 records after normalized-title deduplication. The complete
deduplicated title, bibliographic-line, and source-page manifest is retained in
[`google_scholar_cited_by_2026-09-02.tsv`](google_scholar_cited_by_2026-09-02.tsv).

Every title in that manifest was screened. Works possibly touching the target model were then
checked against primary text already gathered in this audit: Aigner, Andreae, Hao, Gargano,
Christen 1994, Bshouty, Karimi et al., Jiang--Polyanskii--Vorobyev, Florin--Ho--Jiang,
Hwang--Lee, Li--Wu--Triesch, and the feedback-coding papers. Damaschke 1994 was additionally
checked at its primary-text abstract on p. 101 and uses binary defective-edge feedback, not a
0/1/2 count. Nested-only, nonadaptive, noisy, probabilistic, parity, balance, underweight,
binary-edge, and general graph-reconstruction titles were excluded on their stated model.
No newly surfaced work supplies an exact finite `Sa` or `Sb` maximum or a construction beyond
those already credited above.

Two exact-title Scholar checks were also used to pursue the inaccessible Christen sources.
Christen's 1980 Publication 341 record had 16 cited-by results; both pages were screened and no
hidden report copy appeared. The 1986 SIAM-talk record had three cited-by results--Hwang 1989,
Gargano 1992, and Christen 1994--and no program, abstract, manuscript, or proceedings copy.
These checks strengthen the trail but do not substitute for the originals.

## Exact-string, OEIS and book searches

The official [`oeis/oeisdata`](https://github.com/oeis/oeisdata) export timestamped
`2026-09-02T03:01:04-04:00` contains no exact match for the complete project sequence or the
tails `22,38,65,112,192`, `38,65,112,192`, or `65,112,192`. A broad web search for
`3,5,8,13,22,38` does find OEIS A297497, but that is an unrelated recursively generated triangle;
it is not this search threshold.

Du and Hwang, *Combinatorial Group Testing and Its Applications*, second edition (2000), was
checked through the publisher's [DOI 10.1142/4252](https://doi.org/10.1142/4252) reader and its
[Google Books record](https://books.google.com/books?id=nD5kDQAAQBAJ). The publisher reader
required a personal or institutional entitlement. Its UK federation offered no usable Oxford
institution entry, and the US InCommon list did not include Las Positas College.

Google Books exposed pp. 211, 214, 217--218, 220, 222--224, and 226--231; pp. 212--213,
215--216, 219, 221, and 225 remained unavailable. Page 220 is the relevant sequential
`(2,n)` discussion. It attributes `g(2,n) <= log_phi n` to Christen, the limiting constant at
most approximately `2.27 log_3 n` to Hao, and the improved asymptotic upper bound to Gargano et
al.; it gives no exact finite `Sa` or `Sb` table. Pages 217--218 concern nonadaptive detecting
matrices, while pp. 222--230 move through other two-irregular feedback models. The bibliography
on pp. 230--231 includes Aigner 1986, Christen 1980/1983, Gargano 1992, Hao 1990, and Hwang
1987/1989--the same historical chain already inspected here.

The searchable text was also queried for the complete sequence and its tails, `T(32,32)`,
`32,32`, `21,17`, `14,9`, `K32,32`, `Gargano`, and `threshold function`. Reproduce by replacing
`QUERY` here:

```text
https://books.google.com/books?jscmd=SearchWithinVolume2&q=QUERY&vid=nD5kDQAAQBAJ
```

This is a relevant-page inspection with seven intervening pages still missing, not a complete
page-by-page copy of pp. 211--230. Because p. 220 contains the target sequential subsection and
only summarizes asymptotic results, the missing pages are now a residual coverage gap rather
than a submission blocker, provided publication claims remain corpus-scoped.

## Near-misses explicitly excluded

- Chang--Hwang 1980/1981, Chang--Hwang--Lin 1982, and Deppe--Lebedev 2013 use a binary
  positive/negative group-test oracle, not the exact-count oracle.
- Andreae's broader graph search papers are relevant context but add no located finite complete
  graph or complete-bipartite threshold.
- Damaschke's “A tight upper bound for group testing in graphs” (1994) asks binary questions of
  whether a tested vertex set contains the single defective edge; despite its title and citation
  path, it is not the two-defective exact-count model.
- The “two counterfeit coins with two-arms balance,” parity-check, complementary-weight,
  underweight, multi-arm, and unreliable-test papers have different feedback.
- Nonadaptive quantitative group testing, nested-only plans, expected-test objectives, noisy or
  semi-quantitative models, and general additive graph reconstruction do not establish these
  unrestricted adaptive worst-case values.
- Kramer's 1997 one-page “sequential strategy” for the feedback adder channel is a
  capacity/variable-sequential result, not a fixed-block zero-error `Sb` value.

## Supporting-theorem attribution

Searches of Aigner 1986, the complete available Chapter 3 scan of Aigner 1988, Andreae 1989,
and the indexed 1988 book text found no general Unit-Group Elimination equivalence or
edge-injective vertex-map pullback lemma. Aigner and Andreae use related special constructions,
not the general statements proved here. This remains a scoped negative result; the finite-value
audit is much stronger than the attribution audit for these elementary lemmas.

## Disclosed inaccessible sources; search closed

The exhaustive Scholar page walk and Hwang 1989 are closed. On 2026-09-02 the project also chose
to close the access search rather than pursue an open-ended archival hunt. The following remain
limitations on the audited corpus, not pending work:

1. Christen, *A Fibonaccian algorithm for the detection of two elements*, Publ. 341,
   Université de Montréal (1980), and the 1986 SIAM conference presentation “Adaptive versus
   non-adaptive quantitative detection.” Scholar versions and all 16/3 cited-by results exposed
   no primary copy. Hwang 1989 now states their relevant asymptotic consequences, but the
   originals would be necessary before making a claim about everything they contain. No such
   claim is made.
2. The seven unavailable pages interleaved in Du--Hwang pp. 211--230, plus a full page-by-page
   Chapter 2 copy if obtainable. The available target subsection and bibliography make this a
   disclosed completeness limitation, not a reason to hold the paper.

Belokopytov's 1987/1989 papers remain desirable for asymptotic attribution, but the later full
texts state their consequences precisely enough to show that they do not settle the small finite
values at issue here.

## Safe publication language after this pass

- Do **not** call the local `Sa(38)` tree the first construction, or call `Sb(21:17)` or
  `Sb(32:32)` new. Cite Gargano 1992.
- It is safe to say that the local 38/39 boundary corrects Aigner's exact seven-test value while
  independently certifying a construction already implicit in Gargano.
- It is safe to say that Aigner 1986, not Li--Wu--Triesch 2018, first proves exact `m=4`.
- For `Sa(65)`, `Sa(112)`, `Sa(192)`, the remaining `m>=6` maxima, and the Aigner-converse
  counterexample, say “no prior result was located in the audited corpus,” not “first ever.”
  Scholar non-discovery and the partial book inspection narrow the residual risk but never turn
  database non-discovery into proof of worldwide priority.
