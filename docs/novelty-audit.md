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

Known-good starting point: [literature.md](literature.md) already has 12 primary sources read
directly, with DOIs, and an explicit imported-vs-own ledger. The five sources it names as still
wanted are exactly the ones most likely to answer Q1–Q3.

---

## Q1. The `Sa` sequence — how much of it is already in print?

**The values.** `Sa(k)` for `k = 0..10` is `2, 2, 3, 5, 8, 13, 22, 38, 65, 112, 192`
(`data/pareto_sa.csv`). `Sa(k)` = the largest `n` such that two defectives among `n` coins can
always be identified in `k` adaptive tests, each test reporting how many of the two lie in the
queried subset.

**What to find.** The largest `k` for which a published source states the value.

**Where to look, in order:**

1. **OEIS.** Search `2, 3, 5, 8, 13, 22, 38, 65, 112`, and separately the tail
   `22, 38, 65, 112, 192` (early terms often differ by indexing convention). I could not do
   this — oeis.org serves a Cloudflare challenge to this environment. If the sequence is
   present, its references and comments answer most of Q1 immediately.
2. **Aigner 1988, *Combinatorial Search*, Ch. 3.** The repo's scan
   ([aigner-1988-scan.md](aigner-1988-scan.md)) is scoped to Ch. 3 and captures Props. 3.24,
   3.25, Cor. 3.26 and the answer to Ex. 3.3.1. **Check specifically for a table of `M(K_n)`
   values, or worked small cases,** which a scoped scan could easily have skipped. Also check
   Ch. 2, which the repo has not read at all.
3. **Du & Hwang, *Combinatorial Group Testing and Its Applications*, 2nd ed. (World
   Scientific, 2000).** The standard reference work. If small-case tables for the two-defective
   quantitative model exist anywhere, they are most likely here.
4. **Forward citations of Aigner 1986** (`10.1016/0166-218X(86)90026-0`) and **Li–Wu–Triesch
   2018** (`10.1016/j.dam.2018.05.026`) in Google Scholar. This is the highest-yield single
   technique for the whole audit: anyone computing these values would cite one of them.

**What each answer changes.**

| finding | consequence |
|---|---|
| the sequence is in OEIS with references | follow them; likely resolves Q1 and Q3 both |
| published through `k = 9` (i.e. 112 is known) | only `Sa(10) = 192` is ours. The paper's computational claim narrows to one value plus the certified method — this is the most likely outcome and is fine |
| published only to `k = 5` or so | the whole upper range is ours; the computational contribution is substantially larger |
| `Sa(10) = 192` already published | the computational headline is gone. The counterexample (Q2) becomes the paper, and the verification architecture becomes the methodological contribution |

---

## Q2. The singleton-majorization converse — is it still open? **(highest stakes)**

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

**The claim.** `n(k,m)` complete and proven maximal for `k = 1..8`, 130 cells
(`data/pareto_sb.csv`, evidence in `evidence/pareto_certification_k1_8.txt`).

**Already known to be published**, per the ledger in `literature.md`: `m=1` (trivial
dichotomy), `m=2` and `m=3` (Aigner 1986/1988), `m=4` and `m=5` (Li–Wu–Triesch 2018). So the
open question is only **`m >= 6` for `k <= 8`**, plus `n(9,6) = 473`.

**What to find.** Any published table or theorem giving `n(k,m)` for `m >= 6`.

**Where to look:** Du & Hwang's book again; Christen's Fibonaccian search reports (named in
`literature.md`, unobtained); forward citations of Li–Wu–Triesch, whose own related-work
section is the best short survey of exactly this frontier.

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
- **The `G_k` closed form** (dyadic blocks of partial binomial sums) — plausibly folklore.

For each: either find the citation, or state in the paper that no published statement was
found, having actually looked.

---

## Q5. Framing check — what is this problem called elsewhere?

Worth an hour because it affects discoverability and referee selection. The model appears under
several names: *quantitative group testing*, *coin weighing* with a counting oracle, *search
with a quantitative oracle*, and in Aigner's formulation *ternary edge search on graphs*. Find
which community currently owns it and which journal published the nearest recent work
(Li–Wu–Triesch went to *Discrete Applied Math*, which is the obvious first target).

---

## Recording the answers

Add each verdict to [literature.md](literature.md)'s imported-vs-own ledger, not here, so there
is one place that says what is ours. Then update this file's question with a one-line verdict
and the citation. Per the repo's supersede-in-place rule, if an answer contradicts a claim
elsewhere in `docs/`, fix that claim in the same commit.
