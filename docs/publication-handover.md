# Publication track: handover

Written 2026-09-02 to start a dedicated session. Read this, then
[novelty-audit.md](novelty-audit.md), then [publishable-claims.md](publishable-claims.md).
Everything below is either already committed or explicitly flagged as undone.

## The one-line state

The mathematics and the verification are done and independently checked. **What blocks
submission is prior art and a public deposit, not proving or computing anything.**

## What the paper is, and the reframing that happened

The obvious headline is `Sa(10) = 192` — 192 coins resolvable in 10 tests, 193 not, extending
`2, 2, 3, 5, 8, 13, 22, 38, 65, 112, 192`. **That is probably not the strongest contribution.**

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
- **Paper draft mechanics.** The hand-typed `Sb` table is now a generated block; the numbering
  collision, dead image reference and mangled formula are fixed.
- **Two record errors corrected**, both of the class that would embarrass in submission:
  Subgraph Monotonicity was claimed as never-stated when it is Aigner eq. (3.10); and the
  certificate doc said it re-checks `Sa(193)` in 9 tests rather than 10.

## What blocks submission

1. **Novelty audit** — [novelty-audit.md](novelty-audit.md) asks five questions, records what is
   settled, and says what each possible answer changes. Current state:
   - **Q1 (is the `Sa` sequence published?)** Aigner did *not* have it — after Cor. 3.29 he
     speculates that "it is quite possible that `K_5` and `K_8` are the only 3-optimal graphs",
     which is not how one writes having computed the values. **Outstanding:** OEIS
     (`2, 3, 5, 8, 13, 22, 38, 65, 112` — Cloudflare-blocked from the agent environment), Du &
     Hwang's *Combinatorial Group Testing*, and Aigner Chapter 2's table 2.13, which Ch. 3 cites
     for `M(K_3) = 2` and which the available scan does not contain.
   - **Q2 (is the converse still open?)** Aigner poses it as a conjecture (quote above). The
     complete ACM forward-citation list for the book was reviewed: **no resolution**, nearest
     neighbours all a different oracle. But that list is demonstrably incomplete — Li--Wu--Triesch
     2018 cite Aigner and are absent. **Outstanding:** a Google Scholar "cited by" pass on the
     book and on Aigner 1986, and the nice-graph literature, where the equivalent statement is
     that `Q_6` is not nice.
   - **Q3 (`Sb` frontier).** `m = 1..5` are published (Aigner 1986/1988 for 2,3; Li--Wu--Triesch
     2018 for 4,5). Only `m >= 6` could be ours. **Outstanding:** any published table for
     `m >= 6`.
   - **Q4 (attribution spot-checks).** The `G_k` closed form is **Aigner's definition** (3.12) —
     cite, do not claim. Unit-Group Elimination and the Vertex-Splitting Pullback Lemma are still
     unchecked against Ch. 2.
2. **Public deposit.** `fedork/radio-data` is private, so every artifact link in the repo 404s
   for a referee. The package above needs a Zenodo (or equivalent) DOI, cited from the paper.
   Needs an account; nothing else.

## Findings a new session should not have to rediscover

- **Aigner Exercise 3.3.2 is an erratum.** It states `M(K_{4,n}) <= k <=> n <= 2^k - k + 2`.
  At `k=2` that asserts 16 edges resolved by 2 ternary tests against an information bound of 9;
  at `k=3` it gives 7 where exhaustive search gives 4. Li--Wu--Triesch's `2^k - 2k + 2` matches
  every verified cell. Cite Aigner for the first statement of the case, note the erratum, cite
  Li--Wu--Triesch for the proof.
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

1. Finish the novelty audit (needs a browser and, for Ch. 2, the book).
2. Deposit the package; get the DOI.
3. Restructure the paper around the counterexample. This is a framing decision and was
   deliberately left to a human — the draft header records the gap rather than guessing.
4. P5 cleanup finishes with the remaining `<TODO>` sections.
