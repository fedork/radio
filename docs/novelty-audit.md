# Novelty audit: what to find, where, and what each answer changes

Opened 2026-09-02. This is the blocking item for [publishable-claims.md](publishable-claims.md):
that inventory lists candidate claims "because it has a proof object or a retained
computational record; that is not by itself a claim of priority."  Nothing here can be settled
from inside the repo. Each question below states what to look for, where, and — the part that
matters — **what changes depending on the answer**, so a negative result is as useful as a
positive one and gets recorded either way.

Fill in the verdict lines as they are answered. A question answered "already published" is a
success: it converts a risky claim into a citation and removes it from the paper's novelty
surface before a referee does it for us.

Known-good source map: [literature.md](literature.md) records the primary sources read directly,
with DOIs and an explicit imported-vs-own ledger. The exhaustive indexed search and its four
remaining access gaps are recorded in
[`publication_prior_art_2026-09-02.md`](../evidence/publication_prior_art_2026-09-02.md).

---

## Q1. The `Sa` sequence — how much of it is already in print?

**The values.** `Sa(k)` for `k = 0..10` is `2, 2, 3, 5, 8, 13, 22, 38, 65, 112, 192`
(`data/pareto_sa.csv`). `Sa(k)` = the largest `n` such that two defectives among `n` coins can
always be identified in `k` adaptive tests, each test reporting how many of the two lie in the
queried subset.

**What to find.** The largest `k` for which a published source states the value.

**Where to look, in order:**

> **VERDICT 2026-09-02, corrected: Aigner published this threshold through `k=7`, but his last
> value is wrong.** Aigner 1986, p. 226, defines `h(k)` by `c(K_n)<=k` iff `n<=h(k)`, exactly the
> present `Sa(k)`. Figure 5 prints `3,5,8,13,22,37` for `k=2..7` and explicitly calls `h(7)=37`
> correct. Aigner 1988, p. 102, independently defines `m^(2)(k)` as the largest population
> solvable in `k` tests and repeats the same values in Figure 2.13. Gargano et al. 1992 then
> publish exact `Sb(21:17)@6`, which with Aigner's six-test threshold 22 implies `Sa(38)@7`; their full
> `Sb(32:32)@7` appendix tree similarly implies `Sa(64)@8`. The local `sa38_k7.tree` is an
> independent construction, while the audited 38/39 boundary establishes the corrected exact
> value. The correction is therefore the missing **upper boundary at 38**, not first achievability.
>
> **OEIS verdict:** the official `oeis/oeisdata` export dated 2026-09-02 contains neither the
> complete sequence nor any of the long tails ending in `38,65,112,192`. **Du--Hwang verdict,
> scoped:** searchable second-edition text exposes the relevant additive-model summary but no
> `Sa` table or matching sequence. A page-by-page copy is still wanted. Commands, source links,
> exact scope and limitations are retained in
> [the prior-art search record](../evidence/publication_prior_art_2026-09-02.md).
>
> **Citation-corpus verdict:** all 69 unique OpenAlex forward citations to eight seed works,
> 106 Semantic Scholar seed-citation records before cross-seed deduplication, and 424 broad
> OpenAlex discovery records were screened. The same-model candidates were checked in primary
> text; none states 65, 112 or 192, or supplies the 38 upper boundary. **Still to do:** a Google
> Scholar export for independent coverage, plus the three inaccessible source groups listed below.

1. **OEIS — checked 2026-09-02.** Exact searches of the dated official Git export found no
   match. Repeat before submission if the audit remains open long enough for the export to change.
2. **Aigner 1986/1988 — checked 2026-09-02.** Both sources publish the threshold through seven
   tests; both give the now-refuted last value 37. Cite both when presenting the correction.
3. **Gargano et al. 1992 — checked 2026-09-02.** The paper publishes the finite constructions
   that imply 38 in seven tests and 64 in eight; it does not state the corrected `Sa` threshold.
4. **Du & Hwang, *Combinatorial Group Testing and Its Applications*, 2nd ed. (World
   Scientific, 2000) — searchable preview checked 2026-09-02.** No exact sequence was located;
   retain the full-book pass as a coverage check.
5. **Forward citations — public API pass complete.** OpenAlex and Semantic Scholar seed sets
   are screened. A Google Scholar export remains useful because database coverage differs.

**What each answer changes.**

| finding | consequence |
|---|---|
| the sequence later enters OEIS with references | follow them; likely resolves Q1 and Q3 both |
| a later source already constructs 38 | **found:** cite Gargano; the local result remains the exact upper correction and an independent certificate |
| published through `k = 9` (i.e. 112 is known) | only `Sa(10) = 192` is ours. The paper's computational claim narrows to one value plus the certified method — this is the most likely outcome and is fine |
| no source reaches 65 or above | current corpus result: the locally new finite tail begins at the one-step improvement 64 to 65, subject to the remaining access gaps |
| `Sa(10) = 192` already published | the computational headline is gone. The counterexample (Q2) becomes the paper, and the verification architecture becomes the methodological contribution |

---

## Q2. The singleton-majorization converse — is it still open? **(highest stakes)**

> **VERDICT 2026-09-02, part 1: Aigner poses it as an open conjecture. Verbatim, book p. 150,
> immediately after the proof of Prop. 3.25:**
>
> > "It is tempting to conjecture that the converse to **3.25** also holds:
> > `A <= N(k) ==> M(A) <= k`. If true, this would provide a beautiful characterization of star
> > forests with cost `k`, but so far only partial results are known."
>
> So the framing is sound: Aigner states the converse, calls the characterization it would give
> "beautiful", and says only partial results were known. The `K=6` counterexample refutes
> exactly this conjecture, and the `K<=5` result is exactly the "partial results" boundary made
> sharp.
>
> **VERDICT 2026-09-02, part 2: the ACM forward-citation list for the book contains no
> resolution.** All 43 entries of `dl.acm.org/action/ajaxShowCitedBy?doi=10.5555/61992` were
> reviewed. They are overwhelmingly other search models - alphabetic codes, plurality, majority,
> k-equal, two-sided search, weighted-graph reconstruction, competitive group testing. The three
> nearest are all a different oracle or a different problem:
>
> - **Gerzen 2009a/2009b**, "Searching for an edge in a graph with restricted test sets" and
>   "Edge search in graphs with restricted test sets" (*Discrete Math* 309:6, 309:20). These are
>   Aigner's section 3.4, the *restricted* test-set variant, not the unrestricted ternary
>   question of 3.3.
> - **Gerzen 2011**, "On a group testing problem: characterization of graphs with 2-complexity
>   c_2 and maximum number of edges" (*DAM* 159:17, `10.1016/j.dam.2011.06.026`). Binary group
>   testing - a test reports whether a subset contains at least one defective - not the counting
>   oracle.
> - **Liu--Zhang--Nie 2005 and Wen-An--Zan-Kan 2004**, "two counterfeit coins with two-arms
>   balance" (`10.1016/j.dam.2005.03.009`, `10.1016/S0166-218X(03)00343-3`). Two defectives, but
>   the oracle is a three-way balance comparison, not a count of defectives in the queried set.
>   Worth a sentence in the paper's related work so the resemblance is pre-empted.
>
> **The wider citation pass is now complete for the public APIs.** The 69-work deduplicated
> OpenAlex union and 106 Semantic Scholar seed records for eight core sources were screened;
> neither exposes a resolution. The ACM list remains demonstrably incomplete because
> Li--Wu--Triesch 2018 cite Aigner and do not appear in it. A Google Scholar export remains an
> independent-coverage check, not a substitute for the completed public citation graph.
> **Still to do:** that export and broader nice-graph literature, where the equivalent statement
> is that `Q_6` is not nice. Note the neighbourhood is currently active -
> the Stanley--Gasharov conjecture (every claw-free graph is Schur-positive) was disproved in
> 2024-25 by Prajapati and independently by Matherne--Morales, with infinite counterexample
> families following (`arXiv:2607.27166`), so someone could plausibly touch `Q_K` soon.
>
> **VERDICT 2026-09-02, part 3: the two current nice/strongly-nice papers named by the audit do
> not contain this result.** Full-text searches of Li--Li--Yang--Zhang (`arXiv:2408.15074`) and
> Zhang (`arXiv:2608.16613`) found no Aigner/search/transcript/star-forest connection and neither
> paper mentions `Q_K`. They confirm that *nice* is the right current vocabulary, but they study
> other explicit graph families. This narrows the search; it does not replace the broader citation
> pass. Search terms and links are in
> [the audit record](../evidence/publication_prior_art_2026-09-02.md).

**The claim.** Every singleton state solvable in `K` is weakly majorized by `G_K` (necessity;
this is Aigner's, Prop. 3.25). The converse holds for `K <= 5` (proved here, exhaustively) and
**is false for every `K >= 6`** (proved here, with an explicit first counterexample
`(64,63,57^2,42^4,22^7,8^15,7^2,1^32)` and an infinite family). See
[theorems/singleton-majorization.md](theorems/singleton-majorization.md) and
[theorems/tight-band-capacity.md](theorems/tight-band-capacity.md).

**Why it matters most.** Aigner 1988 proves only necessity and, per the repo's scan, "the next
paragraph explicitly says the converse was then open." A negative answer to a question posed in
a standard monograph is a stronger and more durable contribution than a sequence value.

**What to find.** Anything published between 1988 and now that resolves, claims to resolve, or
restates as open the converse of Prop. 3.25.

**Where to look:**

1. **Read Aigner's paragraph after Prop. 3.25 directly** (book pp. 149–150) and record its
   exact wording. The paper must quote it accurately when saying the question was open — this
   sentence is load-bearing for the whole framing.
2. **Forward citations of Aigner 1988**, filtered for star forests / majorization / `N(k)`.
3. **The "nice graph" literature.** The counterexample is equivalent to `Q_6` failing to be
   *nice* in Stanley's sense (Stanley 1998, `10.1016/S0012-365X(98)00146-0`), and the repo
   already cites Shahriari's generalized-Griggs template and current nice-graph work
   (`arXiv:2608.16613`). **Check whether niceness of these transcript graphs is stated as open,
   conjectured, or settled anywhere.** If someone has independently shown `Q_K` is not nice,
   that is the same result in different language, and we must find it before a referee does.
4. **Belokopytov & Luzgin 1987 and Belokopytov 1989**, named in `literature.md` as unobtained.
   Russian-language work in this area is the most plausible place for an overlooked resolution.

**What each answer changes.**

| finding | consequence |
|---|---|
| still open, never resolved | **this is the paper.** Lead with it; `Sa(10)=192` becomes the flagship computation supporting a theory result |
| resolved by someone else | severe. Demote to a verification/reproduction contribution and re-check whether the *infinite family* and the exact `K=5` boundary are still new |
| stated as open in recent work | excellent — cite that as evidence the question is live |

---

## Q3. The `Sb` frontier for `k <= 8` — which of the 130 cells are new?

> **VERDICT 2026-09-02, corrected after the journal and citation pass:**
>
> - **Ex. 3.3.1**: `M(K_{3,n}) <= k <=> n <= 2^k - k`. This is `n(k,3) = 2^k - k` and it matches
>   our data exactly (k=3,4,5,9 -> 5, 12, 27, 503). Attribution already correct in
>   `literature.md`.
> - **Ex. 3.3.2**: `M(K_{4,n}) <= k <=> n <= 2^k - k + 2`. **The printed formula is wrong.** At
>   `k=2` it asserts `M(K_{4,4}) <= 2`, i.e. 16 edges resolved by 2 ternary tests, which
>   violates the information bound `16 > 3^2 = 9`. It also disagrees with all our verified
>   cells (`k=3` gives 7 against the exhaustively established 4). Li--Wu--Triesch's Corollary 3,
>   `n(k,4) = 2^k - 2k + 2`, matches every cell (4, 10, 24, 54, 116, 242, 496). Almost certainly
>   a typo for `2^k - 2k + 2`. **Aigner 1986 Corollary 4 already prints and proves the correct
>   formula.** Cite the journal paper for priority, distinguish the 1988 erratum, and cite
>   Li--Wu--Triesch as an independent later proof.
> - **Ex. 3.3.3**: `M(K_{5,9}) = 4` and `M(K_{9,14}) = 5` - both match our `n(4,5)=9` and
>   `n(5,9)=14` endpoints as constructions, but a cost statement at one graph is not by itself
>   the fixed-`m` maximality claim.
>
> - **Aigner 1986 Figure 4** publishes the exact small table including
>   `c(K_{6,7})=4`, `c(K_{6,8})=5`, hence the `m=6,k=4` frontier `n(4,6)=7`.
> - **Zhang--Berger--Massey 1987** publishes finite full-feedback code pairs equivalent to
>   `Sb` constructions, including `(5,3)@3`, `(5,9)@4`, `(8,14)@5` and `(5,45)@6`.
> - **Gargano et al. 1992** publishes the complete `Sb(32:32)@7` tree and exact costs for
>   `Sb(14:9)@5` and `Sb(21:17)@6`. The last two hit locally maximal endpoints but do not prove
>   the neighboring negative, hence do not by themselves publish those frontier maxima.
>
> **Corpus verdict:** no further fixed-`m>=6` maximum was exposed by the complete public-API
> citation/discovery pass or searchable Du--Hwang text. **Still to do:** page-by-page Du--Hwang,
> Hwang 1989, Christen 1980/1986, and a Scholar export.

**The claim.** `n(k,m)` complete and proven maximal for `k = 1..8`, 130 cells
(`data/pareto_sb.csv`, evidence in `evidence/pareto_certification_k1_8.txt`).

**Already known to be published**, per the ledger in `literature.md`: `m=1` (trivial
dichotomy), exact `m=2,3,4` (Aigner 1986), and exact `m=5` (Li–Wu–Triesch 2018). In the
`m>=6` region, `n(4,6)=7` is also published and several later frontier endpoints have published
positive constructions. The remaining publication question is therefore **cell and bound
specific**, not simply “all `m>=6` are ours.”

**What to find.** Any published table or theorem giving `n(k,m)` for `m >= 6`.

**Where still to look:** the page-by-page Du--Hwang book; Christen's Fibonaccian search report
and 1986 talk; Hwang 1989; and a Google Scholar export. The public citation databases and the
backward bibliographies of the modern same-model papers are already exhausted.

**Consequence.** Whatever is found converts cells from "ours" to "cited" in the same ledger
format `literature.md` already uses. Low risk either way, but it must be done cell-honestly —
claiming a published cell would be the same class of error as the Subgraph Monotonicity
attribution mistake corrected on 2026-09-02.

---

## Q4. Attribution spot-check on the supporting theorems

The Subgraph Monotonicity page claimed for a month that the theorem "had never been stated"
while the project's own Aigner scan identified it as equation (3.10). That error was caught
only because this audit began. **Assume others exist.** The theorems to re-check against the
literature before any of them is stated as ours:

- **Unit-Group Elimination** ([theorems/unit-group-elimination.md](theorems/unit-group-elimination.md)) —
  elementary, and elementary results are exactly the ones already in a 1988 monograph.
- **Vertex-Splitting Pullback Lemma** and its star-expansion corollary — the repo notes it is
  *not* a restatement of Subgraph Monotonicity, but check Aigner Ch. 2–3 for a pullback or
  homomorphism-monotonicity principle.
- **The `G_k` closed form** (dyadic blocks of partial binomial sums) — **RESOLVED 2026-09-02:
  it is Aigner's, and it is his *definition*.** Book p. 147, display (3.12) defines `N(k)` by
  `n_1 = 2^k`, `n_2 = C(k,0)+...+C(k,k-1)`, and generally
  `n_{2^{i-1}+1} = ... = n_{2^i} = C(k,0)+...+C(k,k-i)`, with Lemma 3.23 giving
  `sum n_i = 3^k` and the two recursions. `N(3) = (8,7,4,4,1,1,1,1)` is printed there and equals
  `G_3`. So the closed form must be cited, not claimed; what is ours is only the choice to
  *define* `G_k` by the recurrence and derive the closed form as a cross-check.

> **VERDICT 2026-09-02, scoped:** no general Unit-Group Elimination equivalence or
> edge-injective vertex-map pullback statement was found in Aigner 1986, the available/full-text
> Chapter 3 material from Aigner 1988, Andreae 1989, or indexed searches across the 1988 book.
> Aigner and Andreae do use isolated edges in particular constructions, and Aigner uses a vertex
> blow-up in a binary-search construction; neither is the general theorem stated here. Broader
> decision-tree and graph-search literature remains to be checked before claiming novelty. See
> [the audit record](../evidence/publication_prior_art_2026-09-02.md).

For each: either find the citation, or state in the paper that no published statement was
found, having actually looked.

---

## Q5. Framing check — what is this problem called elsewhere?

**VERDICT 2026-09-02, partial.** The directly relevant sources use *quantitative group testing*,
*coin weighing with a spring scale*, and *ternary search on graphs*. The current neighboring graph
literature uses *nice* and *strongly nice* for the dominance property. Use all four vocabularies in
the title/abstract/keywords or related work so both communities can find the paper. The nearest
finite exact paper remains Li--Wu--Triesch in *Discrete Applied Mathematics*; journal selection is
still a human decision.

---

## Recording the answers

Add each verdict to [literature.md](literature.md)'s imported-vs-own ledger, not here, so there
is one place that says what is ours. Then update this file's question with a one-line verdict
and the citation. Per the repo's supersede-in-place rule, if an answer contradicts a claim
elsewhere in `docs/`, fix that claim in the same commit.
