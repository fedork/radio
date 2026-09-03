# Publication track: handover

Written 2026-09-02 to start a dedicated session. Read this, then
[novelty-audit.md](novelty-audit.md), then [publishable-claims.md](publishable-claims.md).
Everything below is either already committed or explicitly flagged as undone.

## The one-line state

The mathematics and verification are done and independently checked. The Google Scholar citation
pass and Hwang 1989 are closed, and the manuscript has been restructured around the counterexample
with corrected priority language. **The only remaining external-access submission blocker is a
public deposit.** Final author and venue editing remains. The inaccessible Christen originals and
seven missing Du--Hwang pages are documented corpus limitations, not active tasks.

## What the paper is, and the reframing that happened

The obvious headline is `Sa(10) = 192` — 192 coins resolvable in 10 tests, 193 not, extending
`2, 2, 3, 5, 8, 13, 22, 38, 65, 112, 192`. The prior-art audit has made that contribution more
interesting: Aigner 1986 Figure 5 and Aigner 1988 Figure 2.13 both print the identical threshold
through seven tests as `3,5,8,13,22,37`, and the former explicitly calls 37 correct. The checked
`Sa(38)` tree constructively refutes that published value and the audited boundary pair establishes
38 exactly. However, Gargano--Montuori--Setaro--Vaccaro 1992 already publish
`Sb(21:17)@6`; with Aigner's published six-test complete-graph threshold 22, a 21/17 first split gives a published construction for
`Sa(38)`. Their appendix's exact `Sb(32:32)@7` tree likewise gives `Sa(64)@8`. Thus the local
novelty is the **upper boundary and exact correction at 38**, not the first construction; at eight
tests the local exact 65 improves the published construction by one. No source in the completed
indexed corpus states 65, 112 or 192. **Even so, this is probably not the strongest contribution.**

Aigner 1988, *Combinatorial Search*, p. 150, immediately after proving Prop. 3.25 (necessity of
majorization by `N(k)` for star forests):

> "It is tempting to conjecture that the converse to **3.25** also holds: `A <= N(k) ==>
> M(A) <= k`. If true, this would provide a beautiful characterization of star forests with
> cost `k`, but so far only partial results are known."

This project **refutes that conjecture**: the converse holds for `K <= 5` (exhaustive) and is
false for every `K >= 6`, with an explicit first counterexample and an infinite family. A
negative answer to a conjecture posed in a standard monograph outranks a sequence value. The
suggested shape is therefore: lead with the counterexample, carry `Sa` through `k=10` and the
`Sb` frontier for `k <= 8` as the certified computational results, and present the verification
architecture as method.

`Sa(10)=192` is also *equivalent* to `M(K_193) > 10` in Aigner's notation — worth saying
explicitly, since it connects the computation to his framework.

## What is done

- **The result.** `Sa(k)` proven maximal for `k = 0..10`; `Sb` frontier proven for `k <= 8`
  (130 cells); `n(9,6) = 473`. Status vocabulary and per-cell evidence in `data/*.csv`.
- **Independent verification.** `tools/cleanroom` (Rust, zero dependencies, shares no code with
  the solver) closes all 2,846,568 certificate claims with zero gaps. Verified twice, at two
  commits with materially different index construction, reaching an identical candidate-cell
  count of 3,252,096,103,282. It agrees with the production engine on total work to 0.46% over
  3.2 trillion cells **and** on citations specifically (1.180 trillion at k=7 against a
  documented 1.18 trillion). Record: `evidence/cleanroom_verifier_2026-09-01.txt`.
- **Reproduction package.** `tools/make_repro_package.sh` builds a 15.7 MB tarball — certificate,
  both checkers, both witnesses, evidence — validated by extracting to a clean directory and
  running it end to end. This is deposit-ready.
- **Paper structure.** The manuscript now leads with the exact singleton-majorization boundary,
  includes the `K=6` counterexample and infinite family, incorporates the corrected prior-art
  record and independent certificate checks, and retains generated `Sa`/`Sb` tables. The imported
  draft's placeholders, numbering collision, speculative formulas and stale framing are gone.
- **Two record errors corrected**, both of the class that would embarrass in submission:
  Subgraph Monotonicity was claimed as never-stated when it is Aigner eq. (3.10); and the
  certificate doc said it re-checks `Sa(193)` in 9 tests rather than 10.

## Submission checklist

1. **Novelty audit — closed.** [novelty-audit.md](novelty-audit.md) now records the
   completed 69-work OpenAlex forward-citation union, 106 Semantic Scholar seed records, 424
   broad discovery records, backward source closure, and exact-string searches. Current state:
   - **Q1 (is the `Sa` sequence published?)** Aigner published it through `k=7`, but with the
     false last value 37. Gargano 1992's `Sb(21:17)@6` already implies 38-achievability;
     the repository independently certifies it and proves the missing upper boundary. Gargano's
     `Sb(32:32)@7` similarly gives the prior eight-test lower bound 64. The official OEIS Git
     export has no match. The in-app Scholar pass exhausted 264 displayed rows (227 after
     normalized-title deduplication), and the available Du--Hwang target subsection and
     bibliography exposed only the known asymptotic chain. No additional finite threshold was
     found. The seven unavailable Du--Hwang pages remain a disclosed corpus limitation.
     Reproducible searches are in
     [the audit record](../evidence/publication_prior_art_2026-09-02.md).
   - **Q2 (is the converse still open?)** Aigner poses it as a conjecture (quote above). The
     complete ACM forward-citation list for the book was reviewed: **no resolution**, nearest
     neighbours all a different oracle. But that list is demonstrably incomplete — Li--Wu--Triesch
     2018 cite Aigner and are absent. OpenAlex and Semantic Scholar forward citations, plus two
     current nice/strongly-nice papers, have now been searched and expose no resolution.
     The Scholar cited-by pages are now exhausted as far as Scholar exposed them; the Aigner-book
     result stopped at 180 displayed records despite an approximate count near 350. A broader
     nice-graph check remains advisable before an unqualified priority claim.
   - **Q3 (`Sb` frontier).** Exact formulas for `m=2,3,4` are already Aigner 1986; `m=5` is
     Li--Wu--Triesch 2018. Aigner Figure 4 also publishes `n(4,6)=7`. Zhang--Berger--Massey
     and Gargano publish individual finite constructions, notably `Sb(21:17)@6` and the full
     `Sb(32:32)@7` tree, without supplying the neighboring negatives needed for a frontier.
     Hwang 1989 has now been read in full: its exact-count Model Q section reports asymptotic
     constructions and restricted-class optima, not an exact finite maximum. The Scholar pass
     found no additional finite frontier. The older Christen report/talk remain unavailable,
     and seven interleaved Du--Hwang pages remain missing. Treat every older endpoint construction
     as prior even when maximality is local.
   - **Q4 (attribution spot-checks).** The `G_k` closed form is **Aigner's definition** (3.12) —
     cite, do not claim. A scoped check found no general Unit-Group Elimination or Vertex-Splitting
     Pullback statement in Aigner 1986/1988 or Andreae 1989, but broader literature checking is
     still required before calling either new.
2. **Public deposit.** `fedork/radio-data` is private, so every artifact link in the repo 404s
   for a referee. The package above needs a Zenodo (or equivalent) DOI, cited from the paper.
   Needs an account; nothing else.

## Findings a new session should not have to rediscover

- **Aigner 1988 Exercise 3.3.2 is an erratum, but Aigner 1986 is correct.** The exercise states
  `M(K_{4,n}) <= k <=> n <= 2^k - k + 2`.
  At `k=2` that asserts 16 edges resolved by 2 ternary tests against an information bound of 9;
  at `k=3` it gives 7 where exhaustive search gives 4. Li--Wu--Triesch's `2^k - 2k + 2` matches
  every verified cell, but Aigner 1986 Corollary 4 already prints and proves that correct formula.
  Cite Aigner 1986 for priority, distinguish the book typo, and cite Li--Wu--Triesch as an
  independent later proof.
- **Aigner's seven-test complete-graph value is also wrong.** Aigner 1986 Figure 5 defines the
  same threshold as `Sa`, prints 37, and calls it correct; Aigner 1988 Figure 2.13 repeats it.
  Gargano 1992 already makes 38 achievable via `Sb(21:17)@6`; `witnesses/sa38_k7.tree` is an
  independent solver-free construction, while the 2026 boundary record makes 38 exact. The
  publishable correction is the exact upper boundary, not first achievability.
- **Gargano 1992 contains more finite prior art than its abstract suggests.** Besides the full
  `Sb(32:32)@7` appendix tree, its conclusion gives exact costs for `Sb(14:9)@5` and
  `Sb(21:17)@6`. The latter implies `Sa(38)@7`; the former two-set results do not by themselves
  prove fixed-`m` maximality.
- **Exercises 3.3.1 and 3.3.3 independently confirm our cells**: `n(k,3) = 2^k - k`, and
  `M(K_{5,9}) = 4`, `M(K_{9,14}) = 5` matching `n(4,5) = 9` and `n(5,9) = 14`.
- **Pre-empt the near-miss literature.** "Two counterfeit coins with two-arms balance"
  (Liu--Zhang--Nie 2005; Wen-An--Zan-Kan 2004) sounds like this problem but uses a three-way
  comparison oracle, not a count of defectives in the queried set. Say so in related work.
- **The nice-graph neighbourhood is active.** The Stanley--Gasharov conjecture was disproved in
  2024--25 (Prajapati; independently Matherne--Morales), with infinite counterexample families
  following (`arXiv:2607.27166`). Since the `K=6` result is equivalent to `Q_6` not being nice,
  this is both a citation opportunity and a reason not to sit on the result.
- **Describe the trust base precisely, and honestly.** The refuter (`radio_refute.c`) shares the
  solver core, so its zero-gap replays are solver-core validation. Separately, its citation
  lookup is *positional* by design (`checkCacheTrie_ctx`, radiobase.c:1330) and therefore
  incomplete: a deliberate completeness/cost trade that is right for a solver but makes its gap
  *reports* unreliable. It produced nine false gaps on a certificate bundle that is actually
  fine, which stood as a documented prohibition until 2026-09-02. This is a good illustration of
  why the independent checker was worth building — but it must be framed as a design trade, not
  a bug.

## Deliberately excluded from any submission

Carried over from [publishable-claims.md](publishable-claims.md): any global fixed-`m` formula
for `m >= 6`; any claim that the `k=9` `Sb` frontier is known (`m = 7..64` is open); the
antidiagonal conjecture (u1) and profile fits for `m >= 7`; legacy positive records without
retained source output; and the asymptotic constant, which Florin--Ho--Jiang settled.

## Suggested order

1. Deposit the package and insert its DOI in the manuscript. This is the only external-access
   submission blocker.
2. Perform final author and target-venue editing on the restructured manuscript. The theory-first
   narrative, priority corrections, finite tables, and verification section are now present.
3. Audit the short and infinite-family proofs independently and decide how much of the large
   `K<=5` computational proof belongs in the main paper versus supplementary material.

The access search is deliberately closed. Do not restart the Christen or Du--Hwang hunt unless a
referee identifies a specific source or the paper's claim scope changes; keep claims corpus-scoped.
