# Publication prior-art audit, 2026-09-02

This note records the reproducible part of the publication search begun from
[`docs/publication-handover.md`](../docs/publication-handover.md).  It is a search record, not a
proof that unlocated prior art does not exist.  The Google Scholar cited-by pass remains open
because no interactive browser session was available.

## Aigner's complete-graph threshold

The model and threshold are identical to this repository's `Sa(k)`:

- Aigner 1986, p. 226, defines `h(k)` by `c(K_n) <= k` iff `n <= h(k)`.
- The same page's Figure 5 prints `h(k) = 3, 5, 8, 13, 22, 37` for `k = 2,...,7` and calls
  `h(7)=37` the correct value.  Source:
  [DOI 10.1016/0166-218X(86)90026-0](https://doi.org/10.1016/0166-218X(86)90026-0).
- Aigner 1988, p. 102, defines `m^(2)(k)` as the largest `n` for which two defectives among
  `n` coins can be found in `k` spring-scale tests.  Figure 2.13 repeats
  `3, 5, 8, 13, 22, 37` for `k = 2,...,7`.  Source:
  [Google Books record](https://books.google.com/books?id=JyRDAQAAIAAJ); the indexed source
  text is reproducible with:

  ```sh
  curl -sS 'https://books.google.com/books?jscmd=SearchWithinVolume2&q=jump%20points&vid=JyRDAQAAIAAJ'
  curl -sS 'https://books.google.com/books?jscmd=SearchWithinVolume2&q=37&vid=JyRDAQAAIAAJ'
  ```

The repository's [`sa38_k7.tree`](../witnesses/sa38_k7.tree) is an unconditional constructive
refutation of the printed `37`: `tools/check_witness.py` verifies a complete `Sa(38)` strategy
without trusting the solver.  The exact upper boundary at 38 is separately recorded in
[`pareto_sa.csv`](../data/pareto_sa.csv) from the audited 2026 solver artifacts.  Thus the safe
publication statement is already stronger than “the sequence was absent from Aigner”: the project
corrects Aigner's published seven-test value.  No later correction was located in the sources and
searches below, but priority for the correction remains provisional until the cited-by audit is
complete.

## OEIS

The official [`oeis/oeisdata`](https://github.com/oeis/oeisdata) export timestamp was
`2026-09-02T03:01:04-04:00`.  GitHub code search of that export returned no matches for the complete
project sequence or any of these long tails:

```text
2,2,3,5,8,13,22,38,65,112,192
22,38,65,112,192
38,65,112,192
65,112,192
```

The check used the repository-specific exact queries below; all four returned `total_count = 0`:

```sh
GH_CONFIG_DIR=.gh gh api -X GET search/code \
  -f q='"2,2,3,5,8,13,22,38,65,112,192" repo:oeis/oeisdata'
```

and the same command with each displayed tail.  This settles only whether the sequence occurs in
that dated OEIS export; it does not settle publication elsewhere.

## Du--Hwang and the exact finite frontiers

The searchable text of the second edition of Du and Hwang, *Combinatorial Group Testing and Its
Applications* (2000), was checked through its
[Google Books record](https://books.google.com/books?id=nD5kDQAAQBAJ).  Section 11.2's indexed text
summarizes Christen's golden-ratio construction, Hao's two-disjoint-set formulation, and the later
asymptotic improvements.  Exact searches for `22 38 65 112`, `65 112 192`, `threshold function`,
and `K38` produced no matching threshold table.  The indexed occurrences of `Aigner` and `Hao` on
pp. 220 and 230 lead to bounds/constructions and references, not an `Sa` table or a fixed-`m >= 6`
frontier.

The indexed searches are reproducible by substituting each URL-encoded term for `QUERY` here:

```sh
curl -sS 'https://books.google.com/books?jscmd=SearchWithinVolume2&q=QUERY&vid=nD5kDQAAQBAJ'
```

This is useful negative evidence, not a substitute for a page-by-page copy of the book: Google
Books exposes searchable snippets rather than every page.  The full-book inspection remains open.

## Citation and nice-graph checks

The [OpenAlex forward-citation result](https://api.openalex.org/works?filter=cites:W1977708615&per-page=100)
for Aigner 1986 was title-screened for work on the same
complete-graph threshold.  The technically adjacent primary sources already in
[`literature.md`](../docs/literature.md)—Andreae 1989, Hao 1990, and Li--Wu--Triesch 2018—were
searched directly.  None states `Sa(7)=38` or a later complete-graph threshold table.  This is not a
replacement for Google Scholar because citation databases have different coverage.

The full searchable text of the two current papers named in the nice-graph audit was checked for
`Aigner`, `combinatorial search`, `quantitative group testing`, `ternary search`, `transcript
graph`, `star forest`, and `N(k)`:

- Li--Li--Yang--Zhang,
  [*Strongly nice property and Schur positivity of graphs*](https://arxiv.org/abs/2408.15074).
- Zhang,
  [*Three Infinite Families Separating Schur Positivity, the Strongly Nice Property, and the Nice
  Property*](https://arxiv.org/abs/2608.16613).

Neither paper mentions the transcript family `Q_K` or the search/majorization question.  They do
confirm that niceness is current terminology and that the current literature studies other
explicit separating families.  Absence from these two papers does not settle the older or broader
nice-graph literature.

## Supporting-theorem attribution

Searches of Aigner 1986, the available Chapter 3 scan of Aigner 1988, Andreae 1989, and the indexed
1988 book text found no general statement of either the Unit-Group Elimination equivalence or the
edge-injective vertex-map pullback lemma.  Aigner and Andreae do use isolated edges inside
particular constructions; Aigner also replaces vertices by groups in a binary-search construction.
Those are related techniques, not the general statements proved here.  Treat this as a scoped
negative result: broader graph-search and decision-tree literature still needs checking before the
paper calls either theorem new.

## Remaining blocking searches

- Google Scholar cited-by passes for Aigner 1986, Aigner 1988, and Li--Wu--Triesch 2018.
- A page-by-page copy of Du--Hwang 2000, especially the additive-model chapter.
- The older Christen reports and the unobtained Belokopytov papers listed in
  [`literature.md`](../docs/literature.md).
- A broader citation search around Stanley's nice graphs and generalized Griggs chain partitions.
