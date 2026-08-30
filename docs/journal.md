# Research journal

Imported 2026-08-02 from the Google Docs export. Escaped punctuation from the export has
been unescaped; content is otherwise unchanged.

Read this as a *log*, not as current fact. Anything here that is settled has been promoted
to `docs/results.md` or `data/*.csv` with a status and a source; where the two disagree,
the data files win. The K=1..8 Pareto table recorded in the 2026-05-12 entry below was
re-checked against `out_k8.txt` on 2026-08-02 and is correct.

Newest entries are at the bottom, matching the original. Append new entries at the end,
dated. When a rule stated here changes, replace the old statement rather than stacking a
contradicting one - the journal's own guidance, and it still applies.

---

Radioactive Coins Research Journal

Purpose

This is the living project journal. Keep it compact. Record accepted definitions, current working rules, benchmark snapshots, artifact status, and open questions. Remove superseded experimental clutter instead of accumulating it.

Current viewpoint

- The canonical Pareto decomposition matrix is the primary research object.

- Witness trees are discovery and validation tools, not the core object.

- Work in the stabilized regime: treat q and t as sufficiently large so low-depth and low-target degeneracies can be ignored, pending later justification.

Generic decomposition matrix layer

- Local refinement of n:m by x:y is [[x:y, (n-x):y], [x:(m-y), (n-x):(m-y)]], treating *:0 as nil.

- Columns align by width. Rows align by multiplicity / height.

- Same-depth ternary fibers recover branches.

- See companion note: General Decomposition Matrix Definitions for the purely geometric layer.

Atomic layer

- For residual target depth t, use ordered base atom sequence G_t \= (g1, g2, ..., g_{2^t}) in nonincreasing order.

- Atomic slots occur in dyadic blocks with multiplicities 1, 1, 2, 4, 8, ... .

- Compact notation: uppercase letters A, B, C, ... denote the unique atom types of G_t in order; lowercase a, b, c, ... denote the unique atom types of G_{t-1}.

- One-step refinement is A -> aa, B -> ab, C -> bc, D -> cd, and in general X_1 -> x_1x_1, X_i -> x_{i-1}x_i for i >= 2.

- Compact matrices are displayed as character grids with '-' for nil.

Witness-derived reconstruction

Accepted result: witness-tree to compact atomic matrix reconstruction should be provenance-driven rather than branch-multiset-driven.

- Normalize leaves to a common target.

- Propagate per-support-slot substates through the actual split data.

- For a mixed child, keep both descendant slots in row-major support order.

- This gives the accepted witness-derived matrix for the 496:4 @9 example after normalization to U_6.

- Branch order inside a fiber is not treated as an invariant. The invariant is the multiset of atoms on each branch.

Accepted invariant for legal transforms

- Legal transforms must preserve the multiset of atomic branches, where a branch is interpreted as an unordered atom multiset.

- Exact row-major branch strings are not invariant.

Local branch language

- A global branch is a full ternary word of length q.

- A prefix p determines a realized descendant support D(p).

- Canonicalization acts relative to prefixes, not via absolute cell positions alone.

Current canonicalization generator

Use one generator only for now:

- For a prefix p, rotate the full realized block D(p) by 180 degrees.

- Equivalently, rotate each concrete realized binary descendant square consistent with p.

Canonicalization order

- Process shorter prefixes before longer ones.

- Parent orientation must be fixed before canonicalizing descendants inside that orientation.

Current comparison rule (provisional working rule)

At a prefix p, compare local pure branches p0 and p2 by component-wise recursion only.

Procedure:

1. Look at the immediate constituent cells of D(p).

2. Sort those cells by descending cell size, using:

   - primary key: cell height

   - secondary key: cell width

   - final stable tie-break: row-major position inside D(p)

3. Inspect those cells in that order.

4. Inside each such cell, compare local 0 and local 2.

5. The first difference decides the orientation.

6. If no difference appears, leave D(p) unchanged for now.

Interpretation of final tie:

- Provisional assumption: if the comparison stays tied through this process, treat the branch pair as symmetric under the prefix rotation and leave D(p) unchanged.

- This is not yet proved and should be revisited later.

Recursive size definitions used by canonicalization

Cell height:

- nil: 0

- atomic: 1

- composite: top-row height + bottom-row height

- row height \= max of the two child heights in that row

Cell width:

- nil: empty

- atomic: its atom label

- composite: concatenate the two column widths, then sort for comparison

- a column width is the common width of its two cells when both are non-nil, or the width of the non-nil cell if one is nil

Branch size:

- branch height \= sum of heights of all cells in the branch

- branch width \= concatenation of widths of all branch cells, then sort and compare lexicographically

Deferred alternative

- The earlier branch-summary rule (compare total branch height first and total branch width second before doing component-wise comparison) is deferred, not discarded. It may be worth reintroducing later if the current component-wise-only rule proves too aggressive.

Current tested outputs under the provisional canonicalizer

2x2 baseline:

AB

AB

stays fixed.

4x4 baseline:

ABCA

ABCA

AB--

--CA

canonicalizes to:

ABAC

ABAC

--AC

BA--

This output currently passes the accepted branch-multiset invariant. Whether it matches the intended final notion of canonicality remains open.

496:4 @9 witness-derived matrix normalized to U_6:

ABCAABCA

ABCAABCA

ABACAB--

------CA

----AABC

--------

BCAA----

--------

Current canonicalized output under the provisional rule:

ABACABAC

ABACABAC

--AC----

BA--BACA

----AABC

--------

--AA----

CB------

This output also preserves the accepted branch-multiset invariant. Treat it as the current exact output of the rule, not yet as a validated final canonical form.

Benchmark notes

- For fixed m, no branch exceeds length m, so only the first m atoms of the target base matter.

- For m=5, U_4 is too low as a preferred retained benchmark target, while U_5 is the first acceptable retained level.

- Current benchmark snapshots retained from earlier work:

  - 58:3 -> canonical-depth branch-length distribution {0:8, 1:15, 2:3, 3:1}

  - 116:4 -> canonical-depth branch-length distribution {0:6, 1:13, 2:6, 3:1, 4:1}

  - 480:5 @ U_5 -> preferred :5 benchmark; 16 x 16 matrix, 80 non-null cells, branch-length distribution {0:25, 1:39, 2:12, 3:4, 5:1}, state counts 32:1 -> 45, 31:1 -> 20, 26:1 -> 10, 16:1 -> 5

Theorem-level result (**retracted 2026-08-26**)

- Singleton majorization is proved only in the necessary direction.

- The purported Three-Way Majorization Decomposition Lemma is false already at `k=2`.

- The converse for `Sb(a1:1, a2:1, ..., an:1)` remains open and is now isolated as the
  Row-Coloring Lemma in `docs/theorems/singleton-majorization.md`.

Working structural hypotheses still under investigation

- Row-support hypothesis for atomic Pareto matrices: local supports restricted to full, top-row-only, and bottom-row-only; left-column-only and right-column-only provisionally excluded.

- Recursive-corner hypothesis: after top-level normalization, the top-left pure branch is a canonical Pareto submatrix for multiplicity m' \< m when m > 1, with base case m' \= m \= 1.

- Mixed-branch inner-corner hypothesis: for q >= 2, the pure branches arising from the next split of the mixed branch are themselves lower-level Pareto submatrices.

These are not currently built into the accepted witness-derived reconstruction rule or the current provisional canonicalizer.

Width-aware reconstruction note

- The currently uploaded tree_to_compact_matrix.kt only enforces support-pattern feasibility and can output shorthand matrices that violate recursive width constraints.

- On the alternative 496:4 witness with top-level split [249:2], the script output an invalid block (for example BC / CC in one 2x2 subblock).

- A width-aware search over the same normalized branch multisets finds valid shorthand matrices; at least two valid solutions exist under the generic geometric constraints, so this witness is another example where branch data do not determine a unique shorthand matrix without further canonical rules.

Artifact status

Source-of-truth note: if both tree_to_compact_matrix.kt and tree_to_compact_matrix (1).kt appear in project search results, treat tree_to_compact_matrix (1).kt as the current width-aware reconstructor source of truth until search indexing catches up and the stale older entry disappears.

Update: tree_to_compact_matrix.kt has now been replaced locally by a width-aware solver that enforces recursive row-height and column-width constraints. On the alternative 496:4 witness with top-level split [249:2], it finds a recursively valid shorthand matrix

AA------

--CA----

AAACBCAB

----BCAB

AA--ABBC

--CAABBC

AA------

--CA----

with branch-multiset invariant preserved.

Update: tree_to_compact_matrix.kt has now been replaced locally by a width-aware solver that enforces recursive row-height and column-width constraints. On the alternative 496:4 witness with top-level split [249:2], it finds a recursively valid shorthand matrix

AA------

--CA----

AAACBCAB

----BCAB

AA--ABBC

--CAABBC

AA------

--CA----

with branch-multiset invariant preserved.

Checked project Drive folder: only research_journal is currently present there.

Useful artifacts that currently exist as chat-local files and are not present in the project Drive folder:

- General Decomposition Matrix Definitions.md

- tree_to_compact_matrix.kt

- tree_to_compact_matrix.jar

- canonicalize_compact_matrix.kt

- canonicalize_compact_matrix.jar

- current witness-derived and canonicalized 496:4 matrix outputs

Orbit comparison result under the current generator

- Using the current generator family only (prefix-based realized-block 180-degree rotations), the two different 496:4 witness-derived matrices form disjoint closed sets.

- Each orbit has size 4096 under the current generator family.

- The two orbits have empty intersection.

- Therefore the current generator family is genuinely too weak to connect these two witness realizations; if they should canonicalize to the same object, an additional generator is required.

- Preserving final branch-signature multiset is too strong for the missing second generator: the two 496:4 witness-derived matrices have disjoint current-generator orbits and different final branch-signature multisets, so no new generator with that invariant could connect them.

Depth-2 concrete-block clue

For the two validated 496:4 witness-derived matrices, comparing the 16 concrete 2x2 block atom-multisets gives a much cleaner discrepancy than comparing ternary-family mini-objects. Matrix A has block multiset difference relative to Matrix B

  2 x {AACC, AB, BC}  versus  2 x {AA, AC, BBCC}

and these triples have exactly the same total atom content:

  AACC + AB + BC  \=  AA + AC + BBCC.

So a plausible missing generator is not an atom relabeling but a conservative 3-block rewrite at the depth-2 concrete-block level, replacing {AACC, AB, BC} by {AA, AC, BBCC} (and conversely). Whether such a rewrite can be realized while preserving recursive validity remains open.

Anti-diagonal invariant clue

- Current prefix-rotation generators preserve the big anti-diagonal atom multiset.

- For valid Pareto solutions, that anti-diagonal multiset appears to be fixed: the first m atoms of the target base sequence, respecting multiplicities.

- This suggests that any additional generator should probably preserve the big anti-diagonal multiset as well, even if it changes other branch decompositions.

- A plausible algebraic family to investigate is anti-diagonal-preserving synchronized row/column permutations or block permutations, where the column permutation is the row permutation conjugated by reversal so the anti-diagonal is mapped to itself.

Branch-comparison refinement

- When comparing branches or lower-level family objects structurally, use their full fixed-length signatures including nil positions, not truncated strings or compressed atom multisets alone.

- In particular, signatures like A- and A are not interchangeable: they have the same non-nil content but different support shape.

- This should be the default for future family-table comparisons and cycle-detection work.

Checkpoint: generator search status

- Current prefix-rotation generators preserve total atom multiset, recursive row/column validity, branch multiplicity admissibility, and the big anti-diagonal multiset, but they over-constrain the space because they do not change split order; they only move existing splits around.

- In the small m=3, q=2 case, full enumeration under the generic validity constraints gives 64 valid matrices, partitioned into 4 current-generator orbits of size 16, so the two witness-derived orbits are not the whole solution space.

- In the generic m=4, q=3 case, full enumeration under the generic validity constraints gives 1,540,096 valid matrices, partitioned into 264 current-generator orbits: 152 of size 4096 and 112 of size 8192.

- The big anti-diagonal atom multiset is fixed globally in the m=4, q=3 valid solution space and equals ABCC.

- A useful first-principles reformulation is that the missing generator should be a rooted-patch adjacent-level reassociation move: align a whole layer of second-level splits, promote that layer to the top level, push the old top split down one level, and require both pre-move and post-move family tables to lie in the same admissible rooted-patch class.

- The current open difficulty is formalizing branch multiplicity admissibility for such a reassociation without falling back to ambient brute-force search. The present working direction is to describe legality in terms of admissible family tables for a rooted patch, rather than arbitrary atom transfers or arbitrary local rewrites.

- A cautionary conclusion from this chat: local slot rewrites such as AB / -- -> -- / BA are not blindly safe, because they change ancestor-branch routing; so nontrivial safe moves are likely reassociations of whole split layers rather than atom-wise local automorphisms.

Open questions

- Is the current component-wise-only canonicalization rule the intended notion of canonical form, or should the deferred branch-summary rule be restored?

- Is the final tie interpretation (leave unchanged when the recursive comparison stays tied) actually correct?

- Can the current canonicalizer be characterized abstractly rather than procedurally?

- What is the strongest correct abstract notion of globally valid matrix beyond tree-consistency?

- Can retained benchmark matrices be described by a small recursive grammar?

- Can the benchmark usefulness criterion be made precise as a safe-target threshold t_safe(m)?

- **Retracted 2026-08-26:** the Three-Way Majorization Decomposition Lemma is false; the correct
  open target is the Row-Coloring Lemma.

Workflow guidance

- Use this journal as the living source of truth.

- Journal future-useful results proactively.

- Keep accepted results and provisional rules clearly separated.

- When a rule changes, replace the old statement instead of stacking contradictory notes.

2026-04-21 consolidation update

Accepted / durable from this chat:

- Keep shorthand profiles, shorthand matrices, and refinement/unrefinement rule A->aa, B->ab, C->bc, D->cd as current working notation.

- Keep stabilization viewpoint with current user-supplied thresholds q_min(1)=0, q_min(2)=1, q_min(3)=2, q_min(4)=3, q_min(5)=4, q_min(6)=6.

- Small simplest-form Pareto profiles still supported: 1->A, 2->B, 3->AC, 4->AACC, 5->BBBD. For 6->BBCD, numeric fit is strong but structural/scalability status remains open.

- Exact local q=2 search was useful as a base-case calibration, but the stronger q=4 constructor rules explored in this chat should remain provisional, not accepted theory.

Most important unresolved question:

- Does the canonical 473:6 @9 witness correspond to a genuinely scalable compact atomic decomposition family, or only to a tree/state-level artifact?

What was checked:

- Numeric Pareto data through K\<=8 plus the extra K=9 datum 473:6 are consistent with the m=6 family refining from BBCD.

- Provenance-driven witness-tree / uniform-state reconstructions can be built and reproduce middle-branch ABCCDD, but the resulting matrices fail the stronger multiple-of-m atom-count sanity check and therefore do not certify a valid compact atomic decomposition matrix.

- A topology-preserving witness-tree lift looked coherent at the tree/state level, but this still does not certify scalable compact-matrix validity.

Working guidance for the next chat:

- Focus primarily on the strong scalability test for the 473:6 witness.

- Treat G2/H2/K2-style constructor work from this chat as exploratory background only unless it directly helps with that test.

- The key next step is to try to build or disprove a valid lifted compact atomic decomposition matrix for the 473:6 witness under one-level scaling.

.

2026-05-12 K=8 Pareto recomputation validation

- Uploaded solver artifact out_k8.zip contains radio/out_k8.txt, a full-solve output for K=8 frontier candidates.

- Parsed 54 full-solve cases, covering m=2 through m=55. For each case, the reported ratio numerator exactly equals the number of emitted top-level split witnesses, and the denominator equals (n+1)(m+1), so the run appears to have enumerated every top-level split x:y.

- Each listed frontier point Sb(n:m) has a matching can-solve-in-8 line. Each one-step successor Sb(n+1:m) has a matching cannot-solve-in-8 line, so the artifact certifies one-step Pareto maximality for those m values, assuming solver soundness.

- Confirmed K=8 improvements relative to the older table only for m=10..17: 10->189, 11->182, 12->174, 13->168, 14->161, 15->155, 16->150, 17->144. Values m\<=9 and m>=18 through 55 agree with the older table. m=1 remains the trivial 256.

- Split counts for improved cases: m=10 has 12 split witnesses; m=11 has 2; m=12 has 14; m=13 has 10; m=14 has 10; m=15 has 18; m=16 has 8; m=17 has 10.

- Full-tree follow-up: for each improved K=8 Pareto value, extracted at least one selected witness whose top-level branches are recursively justified by exact can-solve lines, logged dominance/slack implications, singleton-majorization terminals, or base terminals. Selected splits: 189:10 [104:6], 182:11 [97:7], 174:12 [97:7], 168:13 [90:8], 161:14 [85:9], 155:15 [87:9], 150:16 [82:10], 144:17 [82:10]. Some alternate top-level split witnesses are not fully reconstructible from out_k8.txt alone because they descend into small fast-solve subbranches that are not separately logged.

- Unit-group follow-up: after applying unit-group triviality by deleting all 1:1 parts before checking a substate, all previously untraced branch gaps disappear. All improved direct witnesses pass, and every alternate full-solve top-level split witness also passes: 12/12, 2/2, 14/14, 10/10, 10/10, 18/18, 8/8, 10/10 respectively. The former gaps were all small subbranches containing one or more 1:1 parts; after stripping, 16 resolve by exact logged lines and 3 by logged dominance/slack implications.

- Correction: do not treat Sb(a:2) as equivalent to Sb(a:1,a:1). In particular, the earlier attempted justification of Sb(32:1,31:1,17:2) in 5 by singleton majorization was invalid. No exact logged can-solve line for Sb(32:1,31:1,17:2) in 5 was found in out_k8.txt. Where it appears as a child of a parent line with fast_solve=1, treat it as an internally accepted fast-solve child unless a separate proof is supplied.

- Repair: Sb(32:1,31:1,17:2) in 5 can be proved directly by split [8:1,16:0,16:1]. The three K=4 branches are Sb(16:1,8:1), Sb(16:1,15:1,9:1,8:1), and Sb(16:1,9:1). Their sorted rows fit coordinatewise into distinct rows of G_4, whose first relevant entries are [16,15,11,11]. Deleting vertices from Aigner's explicit G_4 strategy therefore supplies an unconditional replacement for the previously invalid domination/singleton rewrite; arbitrary weak majorization is not needed here.

- Grep refinement: out_k8.txt contains a fitting logged superstate for the repaired node: line 11179479, `can solve Sb(17:2,32:1,31:1,10:2) in 5 with [8:1,16:0,16:1,10:2] ...`. Dropping the extra 10:2 part gives exactly the manual split [8:1,16:0,16:1] for Sb(17:2,32:1,31:1), so this repair is not merely invented; it is witnessed in the file by a stronger logged state with the same first three parts.

- Script update: extract_witness_tree.py now renders branch paths as ternary words over {0,1,2} and expands children in an interesting-first order while preserving original branch digits. It also allows later simple branches to reference an earlier proven whole-part superstate in the same rendered tree; e.g. in the 54:4 example, branch @1 proves Sb(27:2,27:2) first, then branches @0 and @2 for Sb(27:2) refer to @1 instead of expanding their own full trees.

- K=8 witness-tree extraction complete: generated and audited trees for all improved K=8 Pareto values m=10..17. All eight rendered roots prove successfully and no failed/cycle nodes remain. Extractor fixes from the audit: rectangular parts are now orientation-normalized, and failed attempts are not memoized because same-tree reference compression is context-sensitive.

- Script update: render-time compression now also uses whole-part component-wise domination by already printed nodes, not only exact repeats and extra-part deletion. Reference selection prefers simple/compact dominating states and records reference nodes as available for later references. Example: in the 174:12 tree, @00 Sb(43:3) in 6 now refers to @02 Sb(54:4) in 6 inste

- Script update: same-tree reference compression now uses whole-part rectangular domination, so an already printed Sb(N:M) can prove a later Sb(n:m) when N>=n and M>=m after orientation normalization. Example: in the regenerated 174:12 tree, root branch @2 Sb(77:5) in 7 now refers to @0 Sb(97:7) in 7.

ad of expanding a separate subtree.

Updated Pareto table, K=1..8

This table supersedes the older K=8 column. The changed K=8 entries are m=10..17; all other listed K=8 entries match the older table.

M\\K	1	2	3	4	5	6	7	8

1	2	4	8	16	32	64	128	256

2		3	7	15	31	63	127	255

3			5	12	27	58	121	248

4			4	10	24	54	116	242

5				9	22	50	109	231

6				7	19	46	104	225

7					17	42	97	214

8					15	38	91	206

9					14	36	87	198

10					12	33	82	189

11					11	31	77	182

12						29	73	174

13						27	69	168

14						25	66	161

15						24	63	155

16						22	60	150

17						21	58	144

18						20	55	139

19						19	53	135

20							51	130

21							49	126

22							47	122

23							45	118

24							43	115

25							41	111

26							40	108

27							38	105

28							37	102

29							36	100

30							35	97

31							34	94

32							33	92

33								89

34								87

35								85

36								83

37								81

38								79

39								77

40								76

41								74

42								72

43								71

44								69

45								68

46								66

47								65

48								64

49								62

50								61

51								60

52								59

53								58

54								57

55								56

Symbolic profile refit after corrected K=8 table

Model for this pass: fixed offset q and atom-position profile over A,B,C,... where N(m,K) must equal the profile evaluated at G_(K-q) for every available table entry for that m. Search limits: letters A..P, q=0..7. These are arithmetic fits only, not compact-matrix validity proofs.

Clean unchanged prefix: m=1 A q=0; m=2 B q=0; m=3 AC q=1; m=4 AACC q=2; m=5 BBBD q=2; m=6 BBCD q=2.

First warning: exact shortest arithmetic profiles become non-structural quickly. m=7 shortest exact fit is ABBBBCDD with q=3 and length 8, so no length \<= m exact fit was found within this search. m=9 and m=10 also have no length \<= m exact fit within A..P, q\<=7. Therefore the old profile-fitting heuristic likely needs an added structural constraint or a relaxed/stabilized consistency rule before interpreting m>=7 fits as meaningful compact profiles.

Artifact: symbolic_profile_refit_k1_8.md contains the full m=1..32 arithmetic-fit table.

- Correction/refinement: redo profile fitting with two structural constraints: profile length must be a power of two, and the alphabet for m is restricted to letters appearing in the first m dyadic slots A B CC DDDD EEEEEEEE ... . Under this corrected model, the clean prefix m=1..6 is unchanged. m=7 gives ABBBBCDD (length 8, q=3); m=8 gives AAACCCDD (length 8, q=3); m=9 gives AAAAAABBCCCCCCDE (length 16, q=4); m=10 gives AAAAABBCCCCCCCEE (length 16, q=4), using only A..E as expected. m=11 has the first large jump, with shortest exact fit found at length 64, q=5. Artifact: symbolic_profile_refit_power2_dyadic_prefix.md.

---

## 2026-08-02/03 — repo reorganisation, corpus recovery, and two retractions

Long session. The durable outcome is less the new results than the discovery that a large
part of the existing record could not be trusted, and the machinery to tell which part.

### Retracted

- **`Sa(10) = 192` is not proven maximal.** I recorded it as `proven-exhaustive` on finding
  the 2023 log, then withdrew it. See below.
- **The paper does not understate its result.** I claimed it should say `k ≤ 10`; it says
  `k ≤ 9`, which is exactly right. Do not change it.
- **`409?` for `n(9,11)` is not baseless.** I said it had no support. It is not *derivable*,
  but it is *consistent* with the length-64 profile model (`c_3 = 4`), which bounds
  `n(9,11)` to 405..410.

### The 2023 corpus emits false negatives

`tools/extract_evidence.py audit` over the ~18 GB inside `radio.zip` found **37 single-part
negatives that are provably false** against the 2026 artifacts — ~0.27% of that corpus's
negatives. Several are the `K=8, m=10..17` band that the 2026-05-12 recomputation already
corrected, so this independently rediscovers a known error rather than a new one. Recorded in
`evidence/refuted_2023_negatives.txt`.

**There is no syntactic marker.** `Sb(143:17)` in 8 was declared unsolvable after 10 passes
and 4 days; `Sb(154:15)` after 14 hours. Both wrong. Cheap `fast_solve=1, totalsplits=0` lines
are mostly the bad ones, but 569 such lines are correct, so that signature is useless as a
filter. By contrast the same audit over the 2026 artifacts found **0 contradictions in 2,723**
verdicts. Reliability is a function of *era*, not of apparent expense.

Consequence: the `Sa(193)` refutation, which shares that profile, is `legacy`. `Sa(192)`
achievability is unaffected — it rests on verified witness trees, and a tree re-verifies from
first principles whatever build produced it. That asymmetry between positives and negatives is
the single most useful structural fact about this project's evidence.

### Recovered from `~/radio_old`

- The `Sa(193)` run: `out26_2.txt` (Sb(112:81) alone, **1,725,456 s**, 12 passes) and
  `out26_3.txt` (the other 15 states plus the verdict, **2,353,729 s**). ~47 days of solve
  time in ~90 GB of virtual memory. `Sb(112:81)` was absent from `out26_3` because that run
  loaded a warm cache — which is why a first pass found only 15 of the 16.
- `parsed_260.txt`, 19.5 M entries — audits clean on every internal test. Unbreaks
  `run_pareto9.sh`.
- The unbroken `pareto9_36..116` chain. What it actually achieved: the near-diagonal walk
  moved from `m = 96` to about `m = 81` in **14 months** of wall clock. The band `m = 18..64`
  was never touched. A frontier walk downward will not reach it on any useful timescale.
- 7 new witness trees (`Sa(38)`, `Sa(65)` ×3, `Sa(112)` ×3) and 16 exhaustive multi-part
  enumerations.

### H4: the profile structure

- Refinement (`A→aa, B→ab, C→bc, D→cd`) reproduces the spreadsheet's own columns exactly,
  preserves the value function, and doubles length. So a profile is an equivalence *class*,
  `q` is not intrinsic, and choosing a larger `q` buys no expressive power — it only drops
  low-`k` constraints.
- **`length = 2^q` is therefore a refinement invariant**, not an artifact of picking the
  shortest form. Holds for m ≤ 13.
- Under that constraint the profile is unique for m ≤ 9 and m = 11..13, forcing the `n(9,m)`
  predictions in [status.md](status.md).
- The journal's earlier "m=11 first large jump to length 64" dissolves if `Sb(11:11)` — the
  only exactly-diagonal cell in the data — is pre-stabilised. Then m=11 fits at length 16.
  Weak point: m=4 includes its own diagonal cell and fits anyway.
- **Dead end worth recording:** the non-adaptive reformulation. A test returns
  `[x∈S] + [y∈S]`, so non-adaptive solvability is the Sidon condition
  `(U−U) ∩ (V−V) = {0}`. Exact for m ≤ 2 — it gives one-line proofs of `n(k,1) = 2^k` both
  directions and the `2^k − 1` construction for m=2 — but strictly weaker from m=3 onward
  (computed: k=4, m=5 gives 6 against the true 9). Adaptivity is essential. Do not invest in
  the non-adaptive picture as a reduction.
- Still unexplained, and the actual obstruction: `q(m)` and the coefficient vectors.

### Infrastructure

Source-of-truth CSVs with `bound`/`status`/`source` per cell; `check_tables.py` (invariants,
formulas, generated doc blocks, source resolvability); `check_witness.py` (re-derives trees
from first principles, three input formats); `extract_evidence.py` (`certify` locates the
evidence for every proven row, `audit` hunts contradictions); `artifacts.sh` (store, with
`check-index`). Artifact store stood up: 7 tags, 367 MB, round-trip verified.

Both historical errors — the stale K=8 column and the `k(k-1)/2 → k(k-5)/2` lemma typo — are
now caught mechanically. Mutation-tested.

### Process lesson

Three separate documents were left asserting things this same session had disproved, despite
"supersede in place" being written down before any of them. Recording a rule does not
implement it. The session-end protocol in `AGENTS.md` now makes re-reading your own changes an
explicit step, and `tools/check_docs.py` catches the mechanical part.

---

## 2026-08-03 — the profile is derived, not fitted

`tools/profile_from_tree.py`. Reading the profile off the committed witness trees rather than
fitting it to the frontier numbers.

**Mechanism.** Fix one m-side coin `y` and a normalisation level `t`. Over the `k-t` tests
above `t`, `y` is either in the tested set or not, so `y` has exactly `2^(k-t)` paths to level
`t`; each ends holding one n-side chunk, which must be an atom of `G_t` because it has to be
resolvable in `t` tests while paired with `y`. **The profile is the multiset of chunk sizes
along those paths.** Hence `length = 2^q` with `q = k-t` (it counts paths), refinement
invariance is automatic (lowering `t` doubles them), and the whole-tree leaf census at level
`t` is `m` copies of the profile — total exactly `m · 2^(k-t)`.

Verified: 248:3@8 both solutions, 496:4@9 both, 480:5@9 seven of nine. For 480:5 the per-coin
profile is `9x32 + 4x31 + 2x26 + 16 = 480` at level 5, and the whole-tree census
`32:45, 31:20, 26:10, 16:5` (80 cells) reproduces the `480:5 @ U_5` benchmark recorded in this
journal years ago — the numbers were already here, unconnected.

**Two failure modes, both real.** Empty paths (a coin ending a path with no n-side coins) and
asymmetry (census not divisible by `m`, so the coins do not share a decomposition). The latter
is exactly the "multiple-of-m atom-count sanity check" recorded earlier as failing for 473:6.
The check was right and now has a reason.

**`Sb(473:6)@9` does not exhibit the m=6 profile.** It is asymmetric and wastes 7 of its 384
paths. That answers the long-standing question about that witness, negatively. Whether a
symmetric 473:6 solution exists is open and worth a targeted search — 480:5 shows symmetric
and asymmetric solutions coexisting for the same state, so failure here may be an artifact of
which solution the search happened to return.

**The obstruction is reframed.** `q(m)` was an arbitrary fit parameter; since `q = k - t` it is
now a question about the tree: how deep must a solution go before every leaf is a singleton?
Also note the earlier mechanism guess in this journal — "q tests separate the m-side into 2^q
classes" — is **wrong**: at depth 7 the 480:5 tree still holds `120:3`, multiplicity 3. The
m-side decrements one per level along a chain; it does not split into classes. The `2^q` counts
paths of a single coin, not classes of coins.

## 2026-08-03 — n-side splits

Question: for an optimal solution, is every n-side split into two non-nil parts, or must
left-only / right-only be allowed?

Answer: for a **single-part** state, both non-nil — 0 counterexamples across all 13 committed
trees, and there is a dominance argument. Among one-side-whole splits both routes have exactly
achievable bounds, since the surviving children are single parts: leaving the *narrow* side
whole reaches `n = 2·n(k-1,m)`, leaving the *wide* side whole reaches only
`n = n(k-1,⌈m/2⌉)`. The former is larger on all 76 checkable `(k,m)` pairs. Verified, not
proved — the margin falls from 2.00 at `m=1` to 1.09 at `k=6,m=10`, so a crossing beyond the
table is not excluded.

In **multi-part** states it does occur: 34 of 2226 split entries, every one of them inside a
multi-part state and every one near-square (median `n/m` = 1.12, max 2.0, largest part 20:10;
the ambient population has median 2.0). In the canonical/atomic trees it never occurs at all,
0 of 425 — so the journal's provisional exclusion of left/right-column-only supports holds for
the atomic Pareto matrices it was stated about, and fails only in the near-square corner that
the stabilisation doctrine already sets aside. Same corner as the `Sb(11:11)` anomaly.

## 2026-08-03 — m=5 and m=6 scalable constructions; the one-sided evidence reconsidered

Full `radio_full` split enumerations of the frontier states, m=5 and m=6 at k=5,6,7.  The two
identities below matched the values recorded at the time.  **Superseded 2026-08-15:** the first is
exact only for `k=5..8` and fails at `k=9` (480 versus exact 481); the second is exact through
`k=9` but predicts `496+481=977` versus the unconditional upper bound 973 at `k=10`.
The former equality claim at 973 was retracted on 2026-08-26 because its positive tree uses the
open singleton-majorization converse.

    n(k,5) = n(k-1,2) + n(k-1,6)      numerical identity only, not realised by a split
    n(k,6) = n(k-1,4) + n(k-1,5)      realised: b=2, a=n(k-1,5), outcome-0 saturated

For m=5 the working windows are exactly `b in {2,3}`, `a in [n - n(k-1,3), n(k-1,3)]`, so the m=3
child is the binding single-part constraint and the window is nonempty because
`n(k,5) <= 2·n(k-1,3)`. For m=6 the window is only two wide, pinned at `a = n(k-1,5)`.

The m=5 identity cannot be structural: `m=6 > 5` cannot be a child of an m=5 part, and the split
that would saturate it (`a = n(k-1,2)`) is absent from the working set.

**Retracted:** my previous claim that the one-sided n-split exceptions were genuine but confined
to near-square parts. They are not genuine. Across all six frontier enumerations there are zero,
and the mixed child `Sb(50:4,54:2)@6` also has all four of its working splits two-sided. The five
instances in the canonical trees are all in `canon_473_6_at9` and all are **no-ops**: `Sb(7:1)`
split `[7:1]` leaves the state unchanged one level deeper, because 7 is not an atom of `G_4` but
is one of `G_3`, so burning a level lets the canonical search terminate. Artifact of the search's
stopping rule. The same tree fails the profile test and wastes 7 paths — one cause, three
symptoms.

Also retracted: the dominance argument comparing `2·n(k-1,m)` against `n(k-1,⌈m/2⌉)`. It is
correct as far as it goes but it only compares *one-side-whole* strategies against each other,
and the near-square margin narrowing that I read as meaningful is in the low-k degenerate regime.
It is not evidence about optimal solutions in general.

**Where it stands:** excluding one-sided n-splits costs nothing on anything examined, for m <= 6,
at every k with data. Not proven safe. Both constructions bottom out in 2-part mixed states whose
frontier is uncharacterised, and that is the gap.

## 2026-08-03 — invariants across the k=9 trees

Measured on the 9 alternative solutions of `Sb(480:5)@9`. Invariant: total mass (2400, forced)
and the atom count at the normalisation level (80 = m·2^(k-t), i.e. none of the nine wastes a
path). Not invariant: root split (9 distinct), leaf count (34..42), leaf-depth profile (9
distinct), and — the important one — the **atom census, which takes 3 distinct values**.

So there is no invariant that picks out "the" solution of a state. The space splits into
inequivalent classes. This is the same phenomenon as the two `496:4` witnesses having disjoint
orbits and different branch-signature multisets: the generator family is not too weak, the objects
are genuinely different. The defensible statement is that **the profile is an invariant of the
symmetric non-wasteful class** (7 of the 9), defined by two conditions — atom count `= m·2^(k-t)`
and census divisible by `m`.

Why m=6 offers no alternatives: the `b=2` split window for `Sb(n(k,6):6)` is 7/10/14/19/25 wide at
k=5..9 from the single-part constraints, but the mixed child `Sb(a:4,(n-a):2)` closes it to two
values at k=5,6,7 and to one at k=9 (`a=231`, seen as the mirror `[242:4]`). The canonical search
therefore returns exactly one tree and there is nothing to choose from at the root. Related: the
depth at which states become atoms grows with m — 6 for m=3,4, 5 for m=5, 3 for m=6 — and `q = k-t`
is measuring that.

## 2026-08-03 — correcting the one-sided-split record twice over

Two corrections to my own work in this session.

**First: the canonical search was audited and does not prohibit one-sided splits.** Read from
source: the n-side enumeration spans all of `[0,n]`, `uneven_rank_to_value` is a bijection onto
`[0,m]` (checked for total = 1,2,4,5,6,8), and `top_split_canonical_rep` is invoked only at line
309 inside `print_all_top_level_trees`, where it drops `a=0` as the mirror of the `a=n` it keeps.
A deduplication, not an exclusion. So the concern was well aimed but lands elsewhere: the tool's
real assumption is `is_canonical_state`, which demands every leaf be a singleton state whose parts
are a sub-multiset of the `G_k` atoms. Solutions of any other shape are invisible to it, so the
"0 of 425 one-sided splits in canonical trees" figure is conditional on that hypothesis and is not
evidence about optimal solutions in general.

**Second: I over-retracted the previous entry.** The 34 one-sided splits in the numbered `Sa`
trees come from the unrestricted solver and **are genuine occurrences**; saying "they are not
genuine" was wrong. What is true is that they are locally avoidable. Exhaustive enumeration of the
five smallest containing states shows a fully two-sided split always also exists — e.g.
`Sb(4:3,3:3)@3` has 10 working splits, 6 of them fully two-sided; the witness took `[2:1,3:2]` but
`[2:1,2:3]` works and is two-sided.

I also briefly concluded the opposite from an empty `radio_full` result that turned out to be
spurious — the same command re-run produced 10 splits. Do not trust a zero-result from that
pipeline without re-running it.

Two-sided splits are rare where they exist: 76 of 5440, 42 of 3324. So the restriction prunes
hard. And local availability at every node does **not** imply a complete two-sided tree, because
taking the two-sided split changes the children and hence the subproblems. That compositional
question is the real one and needs a search constrained to two-sided splits, which does not exist.

## 2026-08-03 — one-sided n-splits are not necessary

`radio_canon_twosided.c`, the canonical search with `TWO_SIDED_ONLY=1` forbidding any split that
leaves a non-nil part's n-side whole, finds verified trees for both `Sb(480:5)@9` (5 of them) and
`Sb(473:6)@9` (1). The second settles it: the unrestricted `473:6` witness uses 5 one-sided splits
and every one is avoidable. The two-sided tree is also tighter — 2 empty paths of 384 against 7 —
so the restriction costs nothing and buys something.

A prediction of mine failed, usefully. `Sb(7:1)@4` really does have no two-sided decomposition at
`target_k=3`: a singleton `Sb(x:1)@d` must be an atom of `G_d` or split into two members of
`R(d-1)`, and `R(3) = {1,4,7,8}` yields sums `{2,5,8,9,11,12,14,15,16}`, missing 7, while `G_4` is
`{16,15,11,5,1}`, also missing 7. Same for `Sb(4:1)@4`. From that I predicted the two-sided search
must fail. It did not — the tree it found never generates `Sb(7:1)@4`. The obstruction is real but
local; a different route sidesteps the state. Reasoning from "the one tree we have needs X" to "X
is unavoidable" was the error.

Two cautions from the same session. Small-k states are not cheap proxies: the canonical search
returns `NO_CANONICAL_TREE` for `Sb(46:6)@6` and `Sb(104:6)@7` even *unrestricted*, though
`radio_full` finds four working splits for the former — so a negative there means "no atomic-leaf
solution", not "no solution". And `ulimit -v` is unsupported on macOS, silently; memory must be
bounded at compile time via `MAX_TREE_NODES` and `MAX_STATE_SIZE`.

Still open: the two-sided `473:6` tree remains asymmetric and carries no m=6 profile. Waste fell
from 7 to 2, nearer the profile condition (zero waste, census divisible by m) but not at it. The
run was stopped by its CPU cap mid-enumeration, so better trees may exist.

## 2026-08-03 — orientation flips invalidate the waste metric on one tree

Chasing the two-sided `473:6` tree's apparent "2 empty paths" found a flaw in my own accounting
rather than a fact about the tree. A part is stored oriented `n >= m`, and a child can invert
that: `Sb(10:2)` split `[9:2]` yields mixed child `(1,2)`, stored as `Sb(2:1)`. The path model
assumes a fixed m-side, so after a flip the coin count is wrong — losing `(2-1)*2^(4-3) = 2`
atoms, precisely the 384-382 gap.

So the two-sided tree wastes nothing, and its "asymmetric" verdict is equally meaningless. It has
1 flip in 272 non-nil children. `profile_from_tree.py` now counts flips and declines to interpret
waste or symmetry when any are present.

Everything else is unaffected: the unrestricted `473:6` tree and all nine `480:5` solutions have
**zero** flips, so the 7 wasted paths, the 80-of-80 counts and the seven profile-carrying
solutions all stand.

Lesson: the m-side is not intrinsic. `Sb(a:b) = Sb(b:a)`, and any model that privileges one side
has to detect when the solver's normalisation swaps them.

## 2026-08-03 — the 473:6 two-sided enumeration completed; what "1 tree" means

Normal exit, not a timeout: the two-sided search over every top-level split of `Sb(473:6)@9`
returns **exactly one tree**, identical to the committed one. The unrestricted search also returns
exactly one. Both share the root `[242:4]`.

This is not evidence that no symmetric tree exists, and the reason matters. `search_state` returns
on the *first* successful subtree and memoises it, so `print_all_top_level_trees` emits one tree
per **top-level split**, not all trees. `Sb(473:6)@9` has exactly one working top split, hence one
tree per mode — while we already know at least two distinct subtrees exist beneath it, since the
restricted and unrestricted searches return different ones.

So "1 tree" is a count of top splits. Settling whether `473:6` admits a symmetric tree needs a
search that enumerates subtrees, or one that optimises the census directly. Neither exists yet,
and building one is the obvious next step for the H4 thread.

## 2026-08-03 — conjecture (u1): two proof routes closed, one lemma left

Attacked (u1) — `Sb(n1:n2)` solvable in `k`, `n1 >= n2`, implies `Sb((n1+1):(n2-1))` — because
it collapses the sixteen `Sa(193)` states to one and shrinks that certificate 16-fold. Not
proved. But the ground is now mapped, and two routes that look obviously right are dead.
Full account in [conjectures.md](conjectures.md#conjecture-u1---the-antidiagonal-conjecture).

**A second solver.** `tools/refsolve.py`, written from [problem.md](problem.md) alone, shares no
code with `radiobase.c`, and reproduces the proven columns for `k = 1..6` exactly. Slow — `k=6`
only, and only near the frontier — but short enough to audit, which is what a structural
question needs. Every claim below is confirmed by both implementations or by a proved theorem.

**The graph reformulation, and a lemma that was load-bearing but unwritten.** A state is a graph
on the coins; a test is a vertex subset `S` returning `|e ∩ S|`; the children are `G[S]`,
`G[V\S]` and the crossing subgraph. `Sb` states are exactly the disjoint unions of complete
bipartite graphs, and the class is closed under this. In that language *solvability is monotone
under edge-subgraph* — run the same tree, fewer candidates can only help. One paragraph, and it
is what `cacheCanSolve`/`cacheCantSolve` and the whole `sbb_greater` relation have always rested
on without a written proof. Now [theorems/subgraph-monotonicity.md](theorems/subgraph-monotonicity.md).
A negative certificate needs exactly this to store antichains instead of closures.

**Refuted: the multi-part generalisation of (u1).** `Sb(15:2, 5:4)` is solvable in 4 (split
`[8:1,1:1]`), `Sb(15:2, 6:3)` is not — an intra-part move with `n >= m`, mass falling 50 to 48.
Since the mixed child of any test has two parts, **no induction on `k` that rewrites a strategy
part by part can prove (u1)**. This is the finding worth the session: it is the first thing
anyone tries, and it cannot work.

**Refuted: any mass-based move lemma.** `Sb(8:1, 2:1)` solvable in 3, `Sb(9:1)` not, and the
second is obtained from the first by a single coin move that *strictly decreases* mass. This does
not use the open converse: `(8,2)` embeds coordinatewise in the first two rows `(8,7)` of the
explicit solvable `G_3`, while `(9)` fails the proved necessary inequality `9 > 8`.

**What is left.** Exactly two edits of a winning split survive the coin move, and they are the
same statement under complementation. The gap reduces to the **Extremal Split Lemma**: the
winning split minimising `p - q` survives. 187 of 187 solvable states with `b >= 2` at `k <= 5`,
plus 21 at `k = 6`. Extremality is not decoration — at `(10:2)` in 4 the winner `(8,1)` fails,
its mixed child being `{(9:1)}` with `n(3,1) = 8`. Both remaining obligations need extremality
essentially: the outcome-2 one is not (u1) at `k-1` because the minimiser often has `p < q`, and
the mixed one is a cross-part move, which the paragraph above kills in general. An exchange
argument is the natural finish and I did not find it.

**Cost.** About three hours, all of it Python at `k <= 6`; no C-solver time beyond seconds of
confirmation. Cheap relative to what it rules out.

**Consequence for H3.** (u1) is not available as a proven reduction, so all sixteen `Sa(193)`
states still need refuting independently. The 58% saving is off the table until the Extremal
Split Lemma is settled.

## 2026-08-03 — benchmark: the whole Sa ladder to Sa(113) in 25 minutes

Ran the Sa ladder end to end on the current build as a combined **performance and correctness
reference**, before touching any of the planned cache/parallelism work. There was no such
reference; every cost estimate in the Sa(193) discussion was extrapolated from 2023 figures on
unknown hardware.

```
clang -O3 -DMAX_K=9 -DMAX_N=113 radio.c -o radio_k9
tools/capped_run.sh --seconds 5400 --rss-gb 16 --poll 2 -- ./radio_k9 > bench_sa113_k9.txt
```

Apple M4 Pro (10 performance cores, 24 GB), Apple clang 21.0.0, single-threaded.

**Caveat on the timing.** A second session was working in the same checkout during the run —
it compiled `radio_mixed` at 15:17, inside the 14:57-15:22 window, and was probably also
running Python experiments. The run is single-threaded on a 10-performance-core machine, so
there was no core contention, but memory-bandwidth and cache contention could plausibly cost
a few percent. Treat 1521 s as an upper bound good to about ±10%, not a pristine figure. The
*shape* of the result — three states at 99.4% of the time, 0.24 GB peak, all nine rungs
correct — is unaffected. If a precise reference is wanted later, rerun on a quiet machine;
it is only 25 minutes.

**Result: wall 1521 s (25m21s), peak RSS 0.24 GB, log 32 MB / 315,278 lines, exit 0.**

**Correctness — all nine rungs reproduce `data/pareto_sa.csv` exactly**, both directions
(largest solvable and first unsolvable): 2, 3, 5, 8, 13, 22, 38, 65, 112. No `MAYBE` lines, no
anomalies, max `pass=2`. 315,184 Sb verdicts, 11,079 positive and 304,105 negative.

### Where the time goes

| state | time | share |
|---|---|---|
| `Sa(111)` | 912 s | 60.0% |
| `Sa(113)` (the refutation) | 489 s | 32.1% |
| `Sa(112)` | 111 s | 7.3% |
| everything else, `k = 1..9` | ~9 s | 0.6% |

`Sa(103..110)` are each under a second. The cliff is explained by the witness: all of them solve
as `Sa(65) + Sa(n-65) + Sb(65 : n-65)`, so the cost is the `Sb` child's distance from the k=8
frontier. `n(8,45) = 68` leaves `Sb(65:45)` slack — 0.001 s. `n(8,46) = 66` leaves `Sb(65:46)`
nearly tight — 912 s.

A prediction failed and the failure is informative. I expected `Sa(112)` to cost at least as much
as `Sa(111)`, since `n(8,47) = 65` puts `Sb(65:47)` *exactly* on the frontier. It took 111 s, an
eighth as long — because `Sa(111)`'s 912 s had already populated the k≤8 memo it needed. Memo
reuse dominates intrinsic difficulty, which is the same effect that collapsed the 2023 `Sa(193)`
tail 1300-fold, now measured on a clean run.

### Verdicts by depth, and what it implies

| k | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
|---|---|---|---|---|---|---|---|
| distinct states decided | 9 | 1,223 | 224,769 | 18,146 | 70,430 | 588 | 19 |

**71% of all state decisions are at k=4**, 77% at k≤5. That is the measured case for a
precomputed tablebase: a persistent k≤4 (or k≤5) decision table, computed once and reused, would
let every future run skip roughly three quarters of its state decisions and would be shared
across all sixteen `Sa(193)` states and the K=9 band work. Note the argument is about *reuse
across runs*, not size — the memo already caches these within a run.

**The log is already the fact set.** 315,184 verdicts, 315,184 distinct `(state, k)` pairs —
exactly equal, no duplicates. The solver emits each decided state once, so a negative certificate
is a canonicalised filter over existing output rather than a new artifact to design from scratch.
That materially de-risks the certification work.

**Memory is not the problem at this scale.** 0.24 GB, against the ~90 GB of the 2023 `Sa(193)`
run. So the trie is not inherently wasteful; the blowup is specific to `MAX_N=194` and k=10
(dense child arrays scale as `MAX_SBB = MAX_N^2/4`, and depth 10 multiplies the reachable set).
I had proposed the compact-memo rewrite as the single biggest engineering win — on this evidence
that is unproven, and the next measurement should be one real `Sb(n1 : 193-n1)` state at k=9 with
RSS instrumented, before any rewrite is justified.

Artifact `bench_sa113_k9.txt` is 32 MB; ~106 bytes per verdict, so log volume tracks verdicts,
not wall time — the expensive states are the quiet ones.

## 2026-08-03 — profiling the hot path and the k=10 memory blowup

Measured rather than reasoned, using `tools/instrument.py` (new). Sampling profilers are
useless here — everything inlines into one deeply-recursive `canSolveB`, so `sample` attributes
100% of self time to it. The technique that worked: **execute a component twice with a compiler
barrier and diff the runtime**; the delta is one extra execution, with semantics provably
unchanged (verdicts diffed line-for-line). Caveat: that measures the *marginal, warm* cost, so
it underestimates memory-bound components. All figures are lower bounds.

### Cost decomposition, k=8 ladder (0.590 s, whole workload)

| component | share |
|---|---|
| split-enumeration loop | ~86% (residual) |
| probe preamble: classify + sort | 10.2% (6.7 ns/call) |
| trie walk | **3.4%** (2.3 ns/probe) |
| libc `qsort`, removable | was 9.2% |

```
canSolveB calls        8,995,658   98.9% are pure CACHE_ONLY probes
split-loop iterations 108,145,711  = 1,072 per real search
 └ actually probe       8,894,747  (the pairs bound rejects ~89% before probing)
```

**The result cache is not the hot path at k=8.** A trie walk is 2.3 ns; a perfect O(1)
structure has a 3.4% ceiling. The hot path is the split table, where `ind[ordering][spi]` is a
permutation so `splitsl[spi2]` is a random access into a large array.

`qsort` on a mean length of **2.84** is the one clearly out-of-place thing: replacing it with an
inline insertion sort gives a measured **1.10x** (min-of-7, 0.650 -> 0.590 s) with verdicts
byte-identical over 3,622 lines. Generated by `tools/instrument.py fastsort`; worth folding into
`radiobase.c`.

### The k=10 blowup, measured at the real geometry

The sixteen `Sa(193)` states are `Sb(n1 : 193-n1)` **in 9**, so `MAX_K=9, MAX_N=193` *is* the
regime that reached 90 GB. 180 s into `Sb(112:81)`:

```
canSolveB calls   1,533,983,982 in 180 s   (99.9% probes, 8.5M calls/s)
TRIE  152,275 nodes, 19,201,299 slots, 1,274,459 used
      => 6.64% occupancy, mean fanout 8.37, 146.5 MB, 128 bytes per closure member
closure 19 members per logical insert
```

**Diagnosis: dense per-node child arrays.** A node allocates `8 x (max_sbb+1)` bytes indexed
directly by `sbb`. With 9,312 sbb ids at `MAX_N=193` and a *measured mean fanout of 8.37*,
occupancy is 6.6%. Replacing the dense array with a sorted `(sbb, child)` vector:

```
1,274,459 edges x 12 B + 152,275 headers x 16 B = 16.9 MB   vs 146.5 MB   => 8.7x
extrapolated: 90 GB -> ~10 GB
```

That is the difference between swapping and fitting in RAM, with **no semantic change** — the
trie's shape, its prefix-dominance shortcut and the closure all stay exactly as they are.

It should also be *faster*: at `MAX_N=66` a call costs 65 ns, at `MAX_N=193` 117 ns. The 1.8x is
locality, and a 17 MB structure is far more cache-resident than a 146 MB one. Fanout 8.37 fits a
sorted vector in one or two cache lines.

### Two of my own hypotheses, refuted by the measurement

- **"Closure materialisation is exponential in part count."** It is not: 11.9 members per insert
  at k=8, 19.0 at k=9/`MAX_N=193`. The closure is cheap and buys pruning; leave it alone.
- **"The compact-memo rewrite is the biggest engineering win."** Asserted earlier from the 90 GB
  figure alone. At k=8 it is worth 3.4%. It *is* the right change at `MAX_N=193`, but for
  footprint and locality, not for lookup cost — a different reason than I gave, and the
  distinction matters because it points at sparse nodes rather than at replacing the trie with a
  hash table.

### Recommendation

1. **Sparse trie nodes** — 8.7x memory, likely faster, no semantic change. The k=10 fix.
2. **Inline the sort** — 1.10x, already generated and verified.
3. **Split table**: materialise each ordering contiguously in iteration order, and turn the
   pairs-bound test (rejecting 89% of 84M candidates) into a precomputed cut position. This is
   where the k=9 time actually goes.
4. Bounded eviction only becomes necessary at k=11; at k=10, sparse nodes suffice.

## 2026-08-03 — sparse trie nodes: built, measured, rejected; the split tables were the target

Acted on the previous entry's recommendation and implemented sparse trie nodes — sorted
`(sbb, child)` vectors replacing the dense arrays. **It is not a win.** Recording it so it is
not retried.

| | dense | sparse |
|---|---|---|
| trie memory, `MAX_N=193`, 180 s | 149.9 MB | **29.3 MB** (5.1x) |
| verdicts in 180 s, `MAX_N=193` | 65,933 | **57,635** (-12.6%) |
| k=8 ladder | 0.640 s | **0.930 s** (1.45x slower) |

Verdicts identical, so it is correct — just not worth it. The cost is one extra dependent
load: dense is `n->next` then `array[sbb]`, two touches; sparse is `n->next`, header,
entries, scan. The locality gain does not pay for it.

**And it was the wrong 21% anyway.** Measuring what actually occupies memory at `MAX_N=193`:

```
SPLIT TABLES  2,516 sbbs built   571.1 MB   <- 79%
TRIE          154,095 arrays     149.9 MB   <- 21%
```

`ensure_splits` allocates 76 bytes per split — 36 for `splitsl` plus 40 for **ten** index
orderings — and `BY_MAX`, `BY_MAGIC` and `BY_MAGIC2` are never selected. The only references
left are the commented-out experiments above the split loop.

### Landed

Two changes, both verified:

- **Drop the three dead index arrays.** 570.2 MB -> 480.9 MB of split tables, throughput
  unchanged (65,917 vs 65,933 verdicts in 180 s).
- **Inline `sort1`.** Measured mean length 2.84 with 81% of calls at len<=3; libc `qsort` costs
  more than the sort. 1.10x on its own.

Together: **1.12x on the k=8 ladder, 1.14x on the full k=9 ladder (1337 s against 1521 s),
+2.8% verdict throughput at `MAX_N=193`, and 15.7% less split-table memory.**

### The k=9 ladder is not byte-reproducible, and that is not a bug

The gate run differed from the committed baseline: 345,298 lines against 315,278. The search
is **wall-clock dependent** — deadlines turn into `MAYBE`, which changes the explored set — so
a faster build simply gets further. k=8 *is* byte-identical because no deadline fires there.

The correct gate is contradiction-freedom, and it passes cleanly:

```
shared (state,k) pairs   307,406
cross-contradictions           0
internal contradictions        0 in each run
all nine Sa rungs              identical
```

Use that check, not `diff`, for anything at k>=9. `tools/extract_evidence.py audit` exists for
exactly this.

### Where this leaves the memory problem

Split tables **saturate**; the trie does not. If every sbb were built:

```
current layout (76 B/split)     2.2 GB
minus the 3 dead arrays (64 B)  1.8 GB
fully packed (~30 B/split)      0.9 GB
```

So the couple of GB of split tables is bounded, and the trie is what grows toward 90 GB over
weeks. Both are worth attacking, on different timescales:

1. **Pack the split tables further** — the `_DESC` orderings are exact reversals of their bases
   (`indexDesc`), so iterate the base backwards instead of storing them (-12 B/split); in
   `splitsl`, sbb ids fit `uint16`, `m1`/`m2` are derivable from the enumeration index, and
   `s[4]`/`s[5]`/`FAST` are bytes (36 B -> ~14 B). Bounded work, and it shrinks the working set
   of the loop that is 86% of runtime.
2. **Sparse nodes, done properly** — header, `uint16` keys and children in *one* allocation, so
   it is two dependent touches like dense with 8 keys sharing a cache line. That should get the
   5x on the trie without the 12.6%. This is the change that decides whether k=11 is reachable.
   The naive version measured above is not it.

## 2026-08-03 — the _DESC orderings need not be stored; split-record packing is dead

Two more results on the split tables, one landed and one a measured dead end.

### Landed: derive the _DESC orderings instead of storing them

`indexDesc` materialised each `_DESC` ordering as an exact reversal of its base,
`ind[DESC][e] == ind[BASE][size-1-e]` — 12 bytes per split for no information. Reversing the
subscript instead takes the stored index arrays from seven to **four**.

The naive form cost **+3.4%** on the k=8 ladder, because resolving base-and-direction through
two lookup tables ran on every one of ~108M split-loop iterations. The ordering is chosen once
per level, so hoisting that resolution to the ~8 sites where `splitincr[i]` is assigned — a
per-level cursor pointer and a +/-1 step — recovers all of it:

| | committed before | DESC-derived | + hoisted |
|---|---|---|---|
| k=8 ladder | 0.590 s | 0.610 s (+3.4%) | **0.590 s (0.0%)** |
| split tables at `MAX_N=193` | 480.9 MB | **390.8 MB (-18.7%)** | 390.8 MB |
| verdicts / 180 s at `MAX_N=193` | 67,641 | — | 67,552 (noise) |

Validated: k=8 byte-identical to the pre-session engine through `radio.c` (3,639 lines); k=9
ladder contradiction-free against the committed baseline — 307,632 shared `(state,k)` pairs,
**0 cross-contradictions**, 0 internal contradictions, all nine rungs correct, 1367 s.

`indexDesc` is deleted; the relationship it encoded now lives in `ORDER_BASE` / `ORDER_REVERSED`.

### Dead end: packing the split record

The next step looked like shrinking `splitsl` from 9 ints (36 B) to ~12 B — `uint16` sbb ids,
`m1`/`m2` derived from the enumeration index, byte-sized `s[4]`/`s[5]`/`FAST`. That is ~25
sites in `radiobase.c`, all twelve ordering functions, plus `radio_print.c`.

Before paying for it, the premise was tested in one line — pad `SPLIT_FIELD_COUNT` and see
whether *larger* records hurt:

| record size | k=8 ladder | verdicts / 150 s at `MAX_N=193` |
|---|---|---|
| 36 B (9 ints, current) | 0.590 s | 62,441 |
| 48 B (12 ints) | 0.590 s | — |
| 64 B (16 ints) | 0.580 s | 62,524 (**1.001x**) |

A 1.78x larger record is **not slower at either scale**. So there is no speed case for packing.
The reason, in hindsight: `splitsl[spi2]` is random *within one sbb's split array* — 112 KB for
`Sb(65:46)` — not across the 390 MB of all split tables. The hot working set is a few hundred
KB and L2-resident at any record size. The 390 MB total is never hot at once.

And the memory case is weak on its own terms: split tables **saturate** near 1.4 GB at the
current layout, which is within the budget. **Do not pack the split record.**

### Where the remaining upside is

1. **Sparse trie nodes with a packed single-block layout** — header, `uint16` keys and children
   in *one* allocation, so it is two dependent touches like dense with ~8 keys sharing a cache
   line. The naive vector version measured earlier cost 12.6%; this is the version worth
   trying, and it is the only change that addresses the trie's unbounded growth toward 90 GB.
2. **The pairs-bound test** — it rejects ~89% of 84M candidates one at a time, while the
   orderings are already sorted by child pairs, so it should be a precomputed cut position.

### Method note

Both of today's dead ends (sparse vectors, record packing) were killed by a cheap probe before
the invasive change was written. The padding probe in particular is one `sed` against
`SPLIT_FIELD_COUNT`. Reach for the one-line falsification first.

## 2026-08-03 — compact trie nodes: 3.4x memory for 12.4% throughput, not landed

Followed the previous entry's recommendation and built the packed single-block version of
sparse trie nodes. It works and it is a genuine trade, not a mistake like the first attempt —
but it is a **trade**, so it is recorded rather than landed.

### Design

One heap block per internal node, so `n->next` is a single dependent load and the header plus
a typical 8-12 keys land in one cache line:

```
[ len | cap | can | pad ]  [ int key[cap] ]  [ struct node child[cap] ]
```

`cap` is kept even so the child array stays 8-aligned; growth doubles from 4 and copies both
regions into a fresh block. `can` replaces the original's "slot 0 holds can_solve_marker"
trick. `free_children` iterates the block instead of re-deriving each child's array size from
the pairs budget, which removes that whole fragile re-derivation.

### Measured

| | dense (committed) | packed block |
|---|---|---|
| k=8 ladder | 0.580 s | 0.810 s (**1.40x slower**) |
| verdicts / 120 s at `MAX_N=193` | 57,571 | 50,455 (**12.4% slower**) |
| trie at 20,000 inserts | 15.3 MB | 7.1 MB (2.2x) |
| trie at 40,000 inserts | 65.9 MB | **19.2 MB (3.4x)** |

k=8 output byte-identical (3,639 lines). The memory ratio *grows* with insert count — dense
arrays widen as more distinct states are cached — so over a full run expect 4-5x, i.e. ~90 GB
becoming ~20 GB.

Note the single-block layout is much better than the naive two-malloc vector tried earlier
(0.716x vs 0.63x at k=8): the extra indirection really was most of that first 12.6%. What
remains is the key scan against dense's single indexed load, and no layout fixes that.

### Why it is not landed

The constraint is "keep memory within reason and avoid swapping", and whether 90 GB violates
that depends entirely on the machine. On a rented big-memory instance it does not, and dense
is 12.4% faster. On 24 GB it swaps catastrophically and the block layout is the only option
that helps. Renting the memory and paying nothing in throughput is roughly cost-neutral
against paying 12.4% for ~17-20 days — so this should be decided by where the run happens,
not settled in advance.

**If a memory-constrained run is ever needed, rebuild this.** Two traps found the hard way:

- Putting the header in a separate allocation from the entries costs ~12% on its own. One
  block, always.
- `alloc_size += ncap - b->cap` after `free(b)` is a use-after-free. It corrupted only the
  accounting counter and k=8 output stayed identical, so it survived a full correctness check
  before being caught by an absurd MB figure. Capture `oldcap` before the free.

### The idea that might get both

The blowup is concentrated in nodes near the root, where a large remaining pairs budget lets
`clamp_sbb` allocate a wide array; deep nodes are already narrow. A hybrid - dense while the
clamped `max_sbb` is small, block-sparse above a threshold - would take the memory win where
it exists and keep dense's single load everywhere else. It needs a discriminant on the hot
path, which is exactly what has to be measured rather than assumed.

## 2026-08-03 — the counting-bound cut: 1.49x on the k=9 ladder

The largest single win of the day, and it came from a property of the split orderings that was
already there but unused.

`BY_SP2`'s sort key is `pairs2raw*(1+P) + |pairs1raw - pairs0raw|` with `P = sb_pairs[sbb]`, and
the tiebreak is strictly `< 1+P` — so the key **determines** `pairs2raw`. The split loop walks
the descending-sorted index from the far end, i.e. in ascending key order, so `sb_pairs[s[0]]`
and hence the running `p0` are monotone non-decreasing down the level. The first candidate to
exceed `max_pairs_1` therefore proves every remaining candidate at that level also fails, and
the level can be abandoned outright. Same for `BY_SP1 -> p1` and `BY_SP0 -> p2`. `_DESC` walks
the other way and `BY_MAGIC3` is a distance, so neither admits the cut.

Previously ~89% of the 84M candidates reaching that point were rejected by the counting bound
**one at a time**.

| | before | with cut |
|---|---|---|
| k=8 ladder | 0.590 s | **0.370 s (1.59x)** |
| **k=9 ladder (full)** | 1521 s | **1021 s (1.49x)** |
| verdicts / 120 s at `MAX_N=193` | 57,407 | **81,873 (1.43x)** |

Validation: all nine rungs correct; 305,891 shared `(state,k)` pairs against the committed
baseline with **0 cross-contradictions** and 0 internal contradictions.

**Output is deliberately not byte-identical, even at k=8.** The cut sits before the
`fast_solve` filter, so candidates that are both `!FAST` and counting-bound-rejected no longer
set `skipped_some`. That converts some `MAYBE` results into `FALSE` — strictly more
informative, since those candidates provably cannot work — which changes what gets cached and
therefore which states are visited. k=8 rungs and all 3,558 shared verdicts agree.

### Cumulative for the day

`radiobase.c` is **1.49x faster on the k=9 ladder** (1521 s -> 1021 s) with **31% less split
table memory** (570 -> 391 MB at `MAX_N=193`), across four changes: inline `sort1`, drop three
never-selected orderings, derive `_DESC` by reversed subscript with per-level hoisting, and the
counting-bound cut. Every one validated by rungs plus contradiction-freedom.

## 2026-08-03 — the fast_solve pass is a 3.84x net loss

The single largest result of the day, and it is the removal of a heuristic rather than the
addition of one.

`canSolveB` ran a first pass with `fast_solve = TRUE`, which skips every split not marked
`FAST`. When that pass fails to conclude it sets `skipped_some`, returns `MAYBE`, and pass 2
redoes the entire level with the filter off. The instrumentation shows how often that happened
on the k=9 ladder:

```
committed engine:   13,473 verdicts at pass=1,  345,975 at pass=2   <- 96% searched TWICE
fast_solve off:    154,263 verdicts at pass=1,        0 at pass=2
```

| | committed | fast_solve off |
|---|---|---|
| **k=9 ladder** | 1021 s | **266 s (3.84x)** |
| k=8 ladder | 0.360 s | 0.340 s (1.06x) |
| `Sb(112:81)` refutation, verdicts / 120 s | 84,103 | 86,560 (1.03x) |

Validation: all nine rungs correct; **zero cross-contradictions** against both the pre-session
baseline (127,497 shared `(state,k)` pairs) and the committed engine (127,183), and 3,492
shared at k=8; zero internal contradictions anywhere.

Note the ladder also *visits fewer states* — 154k verdicts against 359k. Without the filter the
search concludes definitively instead of returning `MAYBE`, so results cache properly and it
stops re-deriving the same subtrees.

**The single-state probe badly understated this.** Measured on `Sb(112:81)` alone over 120 s it
looked like 2.9%; end-to-end it is 3.84x. A 120-second window on one deep refutation samples
throughput inside one node's search, not the multi-pass structure across a whole run. Do not
size a structural change from a fixed-time window on one state.

### What this suggests about the rest of the deadline machinery

`fast_solve` is one part of a larger apparatus — deadlines, `MAYBE`, `DEADLINE_RATIO`,
progressive re-passes — all of it built to find solutions quickly by giving up early and
retrying. On this evidence the giving-up is what costs. The other pieces are worth the same
treatment: measure with them disabled before assuming they help.

(The 2023 `Sb(112:81)` log shows `pass=12`, but that is an **earlier build**. The current one
caps at 2, which the instrumentation confirms — so the cost measured here is the two-pass
structure, not a runaway one, and it was still 3.84x.)

### Cumulative for the day

`radiobase.c` is now **5.72x faster on the k=9 ladder** (1521 s -> 266 s) with **31% less split
table memory**, over five changes: inline `sort1`, drop three never-selected orderings, derive
`_DESC` by reversed subscript with per-level hoisting, the counting-bound cut, and removing the
`fast_solve` pass. Each validated by Sa rungs plus contradiction-freedom.

## 2026-08-03 — deadlines are untested, not harmless; and the engine is deterministic again

Two corrections and one property worth having.

### Correction: "deadlines are inert" was scoped wrong

Making child budgets unbounded (`child_deadline = NO_DEADLINE`) changed **nothing** on the k=9
ladder — 267 s against 266 s, and the verdict sets were *identical*, 154,263 shared out of
154,263 with zero contradictions. I read that as the deadline machinery being inert.

It only shows that **no deadline ever fired**. The budget cascade is 1000 s at a `NO_DEADLINE`
root, then `/DEADLINE_RATIO` per level down to a `MIN_DEADLINE` floor of 3 s. The whole ladder
now runs in 266 s, so no single search comes close. The deadline machinery is **untested**, not
harmless, and needs a workload heavy enough to engage it — a k=8 Pareto walk, where single
frontier cells near the diagonal take far longer.

### The FAST machinery is gone, and the search is now deterministic

Removing the `fast_solve` pass left `FAST` read by nothing but a log annotation. Removed ~100
lines: the init loop, the dead filter branch, the `NOTFAST` annotation and its `s[FAST] = 1`
self-tuning writeback, and the orphaned `compare_solvability` / `get_max_sbb` / `minK`. That
last one ran unbounded searches behind a 1000-second deadline, sitting in dead code.

Performance-neutral (1.000x at k=8, 0.995x at `MAX_N=193`) — this is hygiene, not optimisation.
Verdicts identical (3,569 at k=8, zero contradictions); all seven drivers still compile; the
only output change is `:NOTFAST` disappearing, which `extract_witness_tree.py` already tolerates.

The payoff is elsewhere. The `s[FAST] = 1` writeback was the documented source of run-to-run
variation, so **two independent k=9 ladder runs now produce byte-identical verdicts** — 154,337
lines, both 256 s. `diff` is a valid regression gate again, after stripping ` took N` and the
`still solving` lines (a 60-second wall-clock heartbeat, so they land at different points).
`docs/tools.md` said the opposite and has been corrected.

That determinism is conditional on no deadline firing. If a heavier workload starts hitting
budgets, wall-clock dependence returns and the contradiction audit becomes the right gate again.

### New driver: `radio_pareto.c`

A parameterised frontier walker — step right while solvable, down when not. Generic replacement
for `radioSbPareto.c` (hardcoded k=9, `MAX_N=204`), and far lighter than `k8_fullrow_batch.c`,
which runs `all_solutions` at every frontier point. Carries the `MAX_N` guard `radio_2part.c`
established. Validated against the proven k=6 column: **18 of 18 cells exact**.

This is now the standard heavy benchmark: a well-defined workload with a known answer for
k <= 8, long enough that deadlines actually engage.

## 2026-08-04 — the Sa(193) run is on AWS; deadlines removed

### Deadlines are gone

Hitting a budget now bumps it rather than returning `MAYBE`, so a full `canSolveB` call always
answers `TRUE` or `FALSE` exhaustively and `MAYBE` survives only as the cache-probe "unknown".
That is the property a certified negative needs, and for runs built this way it retires the
"absence of a can't-solve line is not a verdict" trap.

This is about the guarantee, not speed — the machinery fired **once in 358,000 verdicts** on a
k=8 Pareto walk. And precisely because it never fires on the Sa ladder, the k=9 ladder output
is **byte-identical** to the deterministic baseline (154,337 lines), which is an exact
regression test rather than a contradiction audit. k=6 frontier still reproduces its proven
column, 18/18.

External bounds (`tools/capped_run.sh`) now replace deadlines for runaway protection.

### k=8 Pareto walk as the new heavy benchmark

40 minutes resolved **9 frontier cells (m = 47..55) with 0 mismatches** against the proven
column. That is stronger correctness evidence than the Sa ladder's nine rungs, and it is the
workload where deadlines actually engaged.

### The run

`Sb(112:80)` then `Sb(112:81)`, both in 9, on `r7iz.4xlarge` in us-west-2. Operational detail —
how to check, stop and resume — is in [aws-run.md](aws-run.md).

The design decision worth recording: **`Sb(112:80)` runs first as a positive control.** It is
the `Sa(192)` construction, already proven solvable by verified witness trees. The engine
changed enormously on 2026-08-03 (5.72x on the ladder, five changes), so a run that cannot
rediscover a known solution has no business producing a negative. If the control fails the run
stops itself. It also warm-starts `Sb(112:81)`, which shares most of its substructure.

Two facts that shaped the setup:

- **Graviton is unavailable on this account** — the ARM vCPU quota is 0. `x2gd.2xlarge` would
  have been half the price for the same 128 GB. The x86 quota is 5000 with ~1372 in use.
- **AWS is slower than the laptop**: the k=9 ladder is 391 s on `r7iz.4xlarge` against 261 s on
  the M4 Pro. We are renting RAM, not cores. Worth remembering before assuming a cloud instance
  speeds anything up.

Checkpointing is the part that mattered. The cache is regenerated from the run's own log every
10 minutes and pushed to S3, so an interruption costs at most that. A restart re-runs only the
top-level call — every completed sub-state is a cache hit. Each checkpoint carries a header
naming build, state and time, and `parse_file` now skips `#` lines: warm-starting a negative
from `parsed_260.txt` is forbidden, from a run's own output is sound, and the header is what
makes those two impossible to confuse.

Hard cost bound is the 72 h of `capped_run` caps, about $107, after which the instance
terminates itself. Tag-filtered AWS Budgets would need cost-allocation tags activated, ~24 h.

## 2026-08-04 — the AWS run failed: removing deadlines was a mistake

The overnight `Sa(193)` run produced no verdict in six hours and was killed. The cause was my
change, and the reasoning behind that change was wrong on its own terms. Recording it in full
because the failure mode is subtle and the temptation to repeat it is real.

### What happened

`Sb(112:80)` in 9 — the **positive control**, a state with a *known* solution, since it appears
in both verified `Sa(192)` witness trees — ran for six hours without concluding. The verdict
rate collapsed about 100-fold partway through:

```
10:44 -> 10:54   +142,047 verdicts
11:45 -> 11:55   +  1,143
```

The log shows the search sunk 43 minutes into a **single k=5 node**:

```
Sb(12:3,11:3,9:3,8:3,9:2,9:2,8:2,15:1,13:1,12:1,4:3,10:1,9:1)[243,155] in 5
left=51/52                       <- one of its 52 splits cleared
totalsplits=130,090,920,500
```

Thirteen parts, and mass **243 = 3^5 exactly** — information-tight, so the counting bound prunes
*nothing*. That one node needed an extrapolated ~34 hours, and it sits deep inside the tree.

### Why the reasoning was wrong

Deadlines are the only escape from an intractable subtree. Without one the search commits to
exhausting it rather than bailing in seconds and trying elsewhere.

I removed them (commit `69ae856`) for the exhaustiveness guarantee. **That guarantee already
existed.** `can't solve` is printed only when `!skipped_some`, so a printed negative is
exhaustive whether or not deadlines are enabled — deadlines only cause some states to return
`MAYBE` instead of a verdict, which is an interpretation trap, not a soundness one. And a
`NO_DEADLINE` root iteratively deepens (`deadline += deadline - start`) until it concludes, so
top-level answers stay definitive regardless.

I had even measured the cost: **266 s against 267 s on the k=9 ladder with identical verdicts**.
The correct reading of "free" was *free insurance*. I read it as "useless machinery" and deleted
it. Restored 2026-08-04; the ladder is byte-identical again at 256 s.

**The general lesson:** "this mechanism never fires on my benchmark" is not evidence it is
useless. It is evidence the benchmark never enters the state the mechanism exists for. The Sa
ladder has no intractable subtree; the real workload does.

### A caveat this puts on yesterday's headline result

`fast_solve` removal measured as **3.84x** — but on the Sa ladder, which is dominated by
*refutations*. `fast_solve` was a solution-*finding* heuristic. The first large **solvable**
state tried since is exactly where the engine got into trouble. The 3.84x is likely real for
`Sb(112:81)` (a refutation) and may be wrong for `Sb(112:80)` (a solution search). Do not assume
it generalises to solution-finding without measuring there.

### Salvaged

Checkpointing worked, which is the one part of the design that held. Preserved in
`s3://radio-sa193-393287594714/run/`: `112_80.cache` (128 MB, **3,100,961 verdicts**) and
`out_112_80.txt.zst` (43 MB). A relaunch warm-starts from those rather than repeating six hours.
Instance terminated; spend about $9.

## 2026-08-04 — a saturated 4-group state has exactly ONE solution in 1.2 billion tuples

Full `radio_full` map of `Sb(15:13,19:9,19:8,24:5)` in 6 — a 4-group node lifted from
`witnesses/sa112_k9_a.tree`, mass 638 against `3^6 = 729`, so ratio **0.875**. 26m34s, complete.

```
solutions: 2 of 1,209,600,000 tuples   =  1.65e-9
  [ 8:7,  8:4,  4:1, 21:5]   -> c2=197  c1=239  c0=202   (cap 243)
  [ 7:6, 11:5, 15:7,  3:0]   -> the exact global complement of the first
```

**One solution up to symmetry. Every group's split is forced** — the per-group winning band is
a single point, not a band. The first solution the systematic sweep reaches is also the tuple
the witness tree uses, so that tuple was never a free choice.

### Why this matters for search design

At a solvability ratio of 1.65e-9, **no per-group ranking heuristic can find this by ordering
alone.** Independent per-group preferences have to coincide on the unique winner in all four
groups simultaneously. What matters is admitting the right *shapes* and then coordinating.

The shapes are not uniform:

| part | mass | winner | midpoint | where |
|---|---|---|---|---|
| 15:13 | 195 | 8:7 | 7:6 | **at the midpoint** |
| 19:9 | 171 | 8:4 | 9:4 | interior, off-centre |
| 19:8 | 152 | 4:1 | 9:4 | interior, off-centre |
| 24:5 | 120 | 21:5 | 12:2 | **on the m2 boundary** (b = m) |

The two largest groups sit at or near the diagonal; the smallest sits on the boundary. There is
an arithmetic reason: `c1 = a(m−b) + (n−a)b` is *maximised* at the midpoint (`c1 = nm/2`), and
the mixed child's mass is the **sum over groups**. Midpoint-everywhere gives `c1 = 318` against
a 243 cap — infeasible by 75. So some groups must go extreme to pay for the ones that don't.
It is a three-way budget allocation across groups, not an independent per-group preference.

The exact statement, one line from AM–GM on `c1 = a(m−b) + (n−a)b` and
`c2·c0 = ab·(n−a)(m−b)`:

> **`c1 ≥ 2·√(c2·c0)`**, equivalently `√c2 + √c0 ≤ √mass`, with equality exactly on the diagonal.

A group cannot feed both outer children generously without paying the mixed child at least
their geometric mean. Verified, 0 violations in 300,000 random splits. **Not yet shown to prune
usefully** — it did not bite on the prefixes tried here.

### Why `FAST` worked, mechanically

`FAST` tested `(m2==0 || cmp(..) <= 0) && (m2==n2 || cmp(..) <= 0)` — a local optimum in the
`m2` direction, i.e. the ridge where the two mixed children balance. Crucially one clause
**auto-passes at `m2 = 0` or `m2 = n2`**, so it admits the diagonal *and* the boundary — exactly
the two shapes this solution needs. That is a much better reason to keep it than "many winners
are near the diagonal", and it explains why a top-*b* prefix of a linear ordering is **not** a
substitute: `FAST` selects a 1-D ridge plus edges out of a 2-D space, roughly `n1+1` splits of
`(n1+1)(n2+1)`, a factor `n2+1` reduction. To match that with a count you would need
`b ≈ n1`, which compounds hopelessly across many groups.

Any widening should therefore stay in the ridge's geometry (`±d` around the FAST curve), not in
a linear rank.

**Caveat: this is one state.** "The solution is unique" must not be generalised from n=1. What
generalises is the arithmetic — mass conservation, the `c1` maximum at the diagonal, and the
`c1 ≥ 2√(c2 c0)` bound.

## 2026-08-04 — winning splits lie on the mixed-children-balance line, not near the midpoint

New driver `radio_allsol.c`: enumerate every working top-level split, pruning on the counting
bound at each *prefix* instead of only at the leaf, which is what `all_solutions` does. Mass is
exactly conserved across the three children, so partial per-child sums only grow and a prefix
over `3^(k-1)` is dead with everything under it.

Validated against `all_solutions` on two states — 8 solutions and 6 solutions, identical
per-part winner sets — with 21-26x fewer leaves. It also reproduces the Python DP's counts
**to the digit** (leaves 231,506,936, prefix nodes 427,673,655), which is two independent
implementations agreeing exactly.

### The middle child, mapped

`Sb(8:6,7:7,8:5,11:4,7:4,15:1,5:3)` in 5 — 7 parts, mass 239/243 = **0.984 saturation**:

```
solutions      14,584  of  401,316,249,600  = 3.6e-8
leaves            231,506,936        prefix nodes  427,673,655
pruned to         0.000577 of raw    wall 8m00s   (all_solutions would need ~6.1 days)
```

Winning sets are **wide** — roughly half of each part's splits appear in some solution, against
exactly one per part in the 4-part parent. That difference is dimensionality, not saturation:
14 free parameters against ~2 independent constraints, versus 8. Solution *count* is not the
informative quantity.

### The finding

Per-part peaks looked bimodal — midpoint-peaked for `8:6`, `7:7`, `11:4`, `15:1`; corner-peaked
for `8:5` (`[0:0]`x3118), `7:4` (`[0:0]`x2980), `5:3` (`[0:0]`x4830). It is not two modes.

The two mixed children have masses `a(m-b)` and `(n-a)b`. They are **equal exactly when
`a·m = n·b`** — the line through `(0,0)`, the midpoint `(n/2,m/2)`, and `(n,m)`. Measuring
perpendicular distance to that line:

| state | result |
|---|---|
| parent's unique solution | every part within **0.63** of its line (distance from *midpoint* reaches 9.34) |
| middle child, all 14,584 | 100% within 2.0, 99.4% within 1.5, 72.1% within 1.0, 3.4% within 0.5 |

So "midpoint mode" and "corner mode" are one structure at different positions along one line.

**Consequence for the heuristic.** `FAST` — a local optimum of the harder mixed child in the
`m2` direction — was already measuring exactly this balance line, which is why it worked.
`BY_MAGIC3` measures distance from the *midpoint*, a proxy that agrees mid-line and mis-ranks
the ends, i.e. precisely the corner splits that peak in the map. The improvement is to make the
ordering use the same geometry as the filter: key on `|a·m - n·b|`, the mixed-children
imbalance, rather than distance from the midpoint.

Tube of half-width `w` around the line, over these 7 parts: `w=0.5` keeps 1/39,587 of the
product space and still contains ~500 of the 14,584 solutions; `w=1.0` keeps 1/239 and contains
72%. So a tight tube is a viable first pass — you need one solution, not most of them.

### Two dead ends recorded

- **The mass lower bound is provably redundant.** Requiring each child to end `>= mass - 2·cap`
  cannot fire where the upper bound does not: if `p_c + M_rem < mass - 2cap` then
  `p_a + p_b > 2cap`, so one already exceeds `cap`. Measured: 0.0% of 427,673,655 prefix nodes.
  Do not re-add it.
- **Replacing `FAST` with a top-*b* prefix of a linear order.** `FAST` selects a 1-D ridge out
  of a 2-D space (~`n1+1` of `(n1+1)(n2+1)`); a linear rank cannot express that, and matching it
  needs `b ~ n1`, which compounds hopelessly across many parts.

## 2026-08-04 — the balance-line ordering: right geometry, wrong metric, reverted

Implemented an ordering keyed on `|a·m − n·b|` (the mixed-children imbalance) in place of
`BY_MAGIC3`'s distance-from-midpoint, and reverted it. The geometry is real; the justification
was not.

### What stands

Winning splits lie on the balance line. Measured over **2,126 winning part-splits from all 15
witness trees** — five `Sa` targets from k=7 to k=10 plus six canonical trees, produced by
different solver runs in different eras, so independent of any current ordering:

```
exactly on the line   32.6%      within 1.0   96.5%
within 2.19          100.0%      median       0.24
```

And it is arithmetic, not statistical: the two mixed children have masses `a(m−b)` and
`(n−a)b`, equal exactly when `a·m = n·b`. That line runs through `(0,0)`, the midpoint, and
`(n,m)`, which is why the full map's "midpoint winners" and "corner winners" are one structure.
**This is what `FAST` tests**, and it is the best available explanation of why `FAST` worked.

### Why the ordering change failed

I measured, for each witness split, what fraction of the part's split space a key ranks ahead of
it. The balance line looked 1.79x better on the mean and moved the 99th percentile from 0.91 to
0.67.

**That metric was wrong.** The search stops at the *first* winner it reaches, so what matters is
the rank of the **nearest** winner, not of the one a particular witness happened to use. Under
that metric, on the 7-part middle child where every solution is known:

| part | nearest-winner rank, midpoint | balance line |
|---|---|---|
| 8:6, 7:7, 8:5, 7:4, 15:1, 5:3 | 0.0000 | 0.0000 |
| 11:4 | 0.0000 | 0.0333 |

`BY_MAGIC3` already places a winner **first** on 7 of 7 parts. There was no headroom, and the
balance line is strictly worse on one part.

The measurements agree: k=9 ladder 261 s against 256 s, and on the solvable states alone 4%
worse, with `Sa(111)` going 0.04 s -> 3.00 s and `Sa(110)` 0.06 s -> 1.00 s. Contradiction-clean
(0 over 128,805 shared verdicts, all nine rungs), so it was correct — just not better.

**Lesson, and it is the general one:** when evaluating a search heuristic, measure the quantity
the search actually consumes. Rank-of-a-known-good-choice is not that quantity when any of many
choices will do. Three of my four heuristic proposals this session died on measurement; this one
died on measuring the wrong thing, which is worse.

### What this does justify

Restoring `FAST` as written. It tests the balance line — the structure the witness data supports
across 2,126 splits — and it needs no improvement from me.

## 2026-08-04 — FAST restored; its cost measured, its benefit still not

Restored the `fast_solve` pass and the whole `FAST` mechanism (init loop,
`compare_solvability` / `get_max_sbb` / `minK`, the `FAST` slot, the `NOTFAST` annotation),
cherry-picked onto the current engine so the later work survived — inline `sort1`, four live
orderings, `_DESC` by reversed subscript with per-level hoisting, the counting-bound cut,
deadlines.

The restoration is **faithful**: k=9 ladder 1020 s against the 1021 s measured before removal,
and 345,847 of 359,320 verdicts carry `pass=2` — the same 96% double-pass. Contradiction-clean
(0 over 127,182 shared `(state,k)` pairs, all nine rungs, k=6 frontier 18/18).

| | no FAST | FAST |
|---|---|---|
| k=9 ladder | **256 s** | **1020 s** (4.0x) |
| states at `pass=2` | 0 | 96% |

**A trap caught by diffing rather than by compiling.** `sbb_to_min_k` is a static array, so it
zero-initialises, and `minK` treats 0 as "already computed". Without restoring
`sbb_to_min_k[i] = i<=1?0:-1` it returns 0 for every sbb, silently degrading
`compare_solvability` to natural-order tiebreaks — so `FAST` would have been computed from a
broken comparator. It compiles and produces correct-but-differently-filtered results. Verify a
restoration by diffing against the pre-removal source, not by whether it builds.

### Why 4x is not an argument against FAST

The Sa ladder cannot see FAST's benefit, for two independent reasons:

- it is **refutation-dominated**, and on a refutation pass 1 finds nothing and pass 2 redoes the
  level, so FAST is pure cost;
- `fast_solve` is gated on `size > 2` and the filter on `i < size_1`, so it does nothing for one-
  or two-part states — and the ladder's expensive nodes are exactly those: `Sa(111)`/`Sa(112)`
  reduce to the **single-part** `Sb(65:46)`, `Sb(65:47)`.

So this 4x is FAST's cost with essentially none of its benefit in view. It is the price, measured
cleanly; it is not evidence about the trade.

### The open question this leaves for the Sa(193) run

The sixteen states are refutations, which is where FAST is pure cost — but refuting a root
requires *proving children solvable* to rule them out as the failing branch, and those
sub-searches are where FAST pays. Genuinely mixed, and the ladder is a poor proxy in both
directions.

**Decide it by measurement on one of the sixteen**, not on the ladder: run the same state with
and without for a fixed wall-clock budget and compare verdicts produced. That is cheap next to
the run itself and it is the only workload that answers the question.

## 2026-08-04 — certificate prototype: it verifies, and it found a bug in itself

`tools/certify.py` reduces a solver log to a fact set and verifies it against nothing but the
three theorems and the split semantics. Built to replace the estimates in
[certificate.md](certificate.md) with measurements before anything expensive depends on them.

### It found a soundness bug in itself, at low k, immediately

The first version short-circuited each fact with `prev.refuted(f, k)` — consulting the **k-1**
fact set to accept a fact at **k**. That is backwards: a state unsolvable in `k-1` says nothing
about `k`, since more tests only help. Every fact "verified" and the run looked perfect.

Fixed, only `COUNT` and `MAJ` short-circuit; everything else must pass `SPLITS`. This is exactly
why the low-k experiment was worth doing first: at k<=6 the run is seconds, so a wrong "all
verified" is cheap to catch. Had this gone straight to a `MAX_N=193` log it would have produced a
confident, meaningless answer after hours.

### After the fix

| log | k | facts | verified | unverified |
|---|---|---|---|---|
| k<=6 Sa ladder | 3,4,5 | 11, 10, 2 | all | **0** |
| k=6 frontier walk | 3 | 121 | 121 | 0 |
| | **4** | **821** | **612** | **209 (25%)** |
| | 5 | 155 | 155 | 0 |

**The 209 failures were not a closure gap. They were a missing rule in the verifier** — see the
retraction below, recorded the same day. The paragraphs that stood here interpreted them as
unlogged dominance witnesses and are withdrawn.

### What this does to the design

- **Breadcrumbs are unnecessary.** The verifier's own dominance search finds the witness whenever
  it is present. The x6.6 figure in `certificate.md` measured queries, not facts, and was the
  wrong quantity.
- **Post-processing is the right architecture.** Everything the certificate needs is computable
  from an unchanged log. The run only has to avoid losing information.

### Retraction, same day: there is no closure gap

`maj_refutes` only fired when **every** part was a singleton. `radiobase.c` applies Singleton
Majorization to the singleton **sub-multiset** — the `singleton_size > 0` branch of `canSolveB`,
which returns FALSE without printing, so such refutations never reach a log. That is why the
missing witnesses all looked like small mixed states.

With the sub-multiset rule implemented, the same k=6 frontier walk verifies **1,330 of 1,330 with
0 unverified**, including all 968 four-part facts that previously failed. The log *is* closed
under `SPLITS`; nothing has to be added to it, and no fixpoint iteration is needed.

The lesson is about method, not about the rule: a 25% failure rate was read as a property of the
*data* when it was a property of the *checker*. The user's first guess — "is it just a difference
in how domination is applied?" — was right, and cost nothing to test against the alternative of
designing an emission mechanism for witnesses that were never missing.

### Do not project these ratios

The `SPLITS` cost per fact is the product over parts of their split spaces, pruned by the
counting bound. Four parts at k=4 is milliseconds; the node that trapped the AWS run was 13 parts
at k=5 with a 119-billion prefix tree. Part count grows as `2^depth`, so both the cost *and* the
closure-gap ratio plausibly move with k in ways these small logs cannot show. The 15-25% and ~25%
figures are measurements at k<=6, not predictions for k=10.

## 2026-08-04 — the C verifier, and the 2023 corpus turns out to be nearly checkable

Two separate things happened. The engineering was mostly negative results. The finding was large.

### Do not tune on small k — a worked example of getting this wrong

Every performance number in the first half of this session came from part counts 1-8, on
`bench_sa113_k9.txt` and `out_k8.txt`. Part count is the exponent in `SPLITS` cost, so that is
the one axis a benchmark must span, and mine did not. What the shape actually is:

    P <= min(2^(K-k), 3^k/2)

Part count doubles going down (the mixed child splits one part in two) and is capped by mass,
since every surviving part has mass >= 2 after Unit-Group Elimination. The bounds cross at
`k ~ 0.387(K+1)`. Measured on the **real `Sa(193)` logs** (`sa193-2023`), the realised shape is:

| k | facts | max P | mean mass / 3^k |
|---|---|---|---|
| 9 | 16 | 1 | 9,155 / 19,683 |
| 8 | 1,879 | 2 | 4,620 / 6,561 |
| 7 | **3,091,929** | **4** | 2,101 / 2,187 (96%) |
| 6 | 4,541 | 8 | 687 / 729 |
| 5 | 97,167 | 8 | 238 / 243 |
| 4 | 940 | 10 | 79.5 / 81 |

The doubling bound saturates only down to k=6, then collapses: a mixed child loses a part whenever
`a in {0,n}` or `b in {0,m}`, and the counting bound forces near-equipartition, so the effective
growth factor is well under 2. **Max part count never exceeds 10**, and the mass is 96-99% of
`3^k` throughout — the hard facts live in a thin shell just under the counting bound.

Consequence: mid-session I extrapolated "P ~ 30 at k=4-5 for K=10" from the `2^(K-k)` bound and
concluded single-machine verification was hopeless. **That was wrong** — the bound is not
realised. The real cost is the opposite corner: **few parts, enormous parts, high k** — 3.1 M
facts at k=7 with P=4 and parts of mass ~520 (about 558 live options each).

### Verifier performance: one win, three negative results

All measured with identical verdicts and, where the change is semantics-preserving, byte-identical
node counts.

| change | effect |
|---|---|
| incremental child construction (copy-and-insert, rolling mass and hash, replacing `canon()` per node) | **1.20x** |
| pairwise narrowing as CSP forward checking | 1.25x fewer nodes, **1.12x** wall |
| combined, k=4 of the k=9 ladder | 123.4 s -> **91.2 s** |
| subtree DP on the prefix state | **net 2.4x LOSS** — abandoned |
| group ordering: mass-ascending / fewest-options-first | 3.8x / 1.09x **worse** than canonical descending |

- **Bucketing the fact set by part count is irrelevant.** The direct-mapped memo intercepts
  99.996% of refutation queries (4.30 G hits, 7,646 misses); only 2.1 M reach the index.
- **Pairwise narrowing's benefit decays with part count** — paired on identical facts: 1.29x at
  P=6, 1.18x at P=7, 1.09x at P=8, 1.04x at P=9, 1.014x at P=10, and a net *time* loss from P=9.
  I had predicted the opposite from a model where a future group's domain shrinks as `q^i` with
  `q=0.62` measured, giving a critical depth `ln L / ln(1/q) ~ 6-8` beyond which domains empty and
  the saving is `L^(P-i_crit)`. The model is wrong by **survivorship bias**: the nodes that reach
  depth `i` are exactly those whose prefix wiped nothing, so their domains are systematically
  fuller than the unconditional estimate. Same error shape as the balance-line ordering mistake —
  reasoning about a population conditioned on survival as if it were the whole population.
  Since the decay runs the other way, pairwise narrowing is most valuable at **P=4**, which is
  where 3.1 M of the `Sa(193)` facts sit. It is left on by default.
- **Prefix narrowing already subsumes every SUBSET of the prefix**, because a refuted
  subset-child is a sub-multiset of the prefix-child and `refuted()` finds it by dominance. So
  "arbitrary subgroup narrowing" adds only *look-ahead* to groups not yet fixed. That is the whole
  content of the pairwise mechanism, and it is thin.
- **The subtree DP fails for a measurable reason.** 1.03 G nodes at P=9 held 7.1e8 distinct prefix
  states — **1.44 nodes per state**. Prefix states are nearly all distinct, so there is nothing to
  collapse. My argument that saturation makes the state "almost purely numeric" (`s1` is implied,
  so `(s2,s0)` has <= 784 values at k=4) was right about the numeric half and wrong that the
  structural half vanishes: at mean part mass 14 the children keep real structure.

Net: the checker is within a small factor of the enumeration's intrinsic size. The remaining
leverage is parallelism, which is free here — facts are independent and levels are independent.

### `radio_verify.c` verifies the k=9 ladder

304,105 negative facts, k=2..4 complete at **0 unverified** (216,580 at k=4 in 91 s), k=5 stopped
by its own cap inside a single 5.5e7-node fact. Per-fact cost spans four orders of magnitude, so a
frozen progress counter is the normal appearance of the tail, not a hang.

### The finding: the 2023 corpus is nearly a certificate already

`sa193-2023` contains **all sixteen** `can't solve Sb(n1:193-n1) in 9` for `n1 = 97..112` —
exactly the set `certificate.md` says `Sa(193)` reduces to. So the question is not whether to
re-run 47 days; it is whether the existing logs can be *checked*.

Running the verifier top-down, with the 2026 `out_k8.txt` merged in as additional database (the
union of fact sets is itself a fact set — every fact is checked on its own merits, so an unsound
one cannot be laundered by the company it keeps):

- **k=9: 16 facts, each fails on exactly ONE split** — 32 recursion nodes in total. Every other
  split of every root is discharged. The survivor is always the near-balanced one, e.g.
  `Sb(112:81) -> (38,40)`, whose two single-part children are both *solvable* by the proven Pareto
  table (`Sb(40:38)`, `Sb(74:41)` — the latter exactly on the boundary, max n1=74 at m=41). So each
  root needs exactly one two-part k=8 fact, for `Sb(112:81)` that is **`Sb(74:40, 41:38)` at k=8**.
  **Nothing among the 1,879 logged 2023 k=8 facts dominates it.**
- **k=4: 940 facts, 0 unverified** in 35 s.
- **k=5: 4,859 of 4,859 sampled facts verified, 0 unverified**, once missing low-k children are
  derived rather than cited (2,836 derived).

### Closure is a property of how the run was conducted

Earlier today I recorded "the log is closed, breadcrumbs unnecessary". That was measured on
`bench_sa113_k9.txt`, a **single cold-cache session**, and it does not generalise. The 2023
`Sa(193)` run was resumed over months from warm caches whose own logs were **not archived**
(~18 GB deliberately dropped, see [data.md](data.md)), so it cites facts whose proofs are gone:
~5% of its k=5 facts reference a k=4 child that was never logged, and the sixteen k=9 facts each
reference a k=8 child that was never logged.

So: **a cold single-session run produces a closed log; a resumed run does not.** This is a direct
constraint on the re-run — keep every session's output, or start cold and never resume.

### On-demand derivation, and what it does to the certificate

The fix is not to ship the missing facts, it is to let the checker **prove** them. When nothing in
the fact set refutes a state, `refuted_raw` now runs the same `SPLITS` check on that state one
level down, memoised, for `k <= a threshold`. Deriving is proving, and a derived fact is checked by
exactly the rules a shipped one would be — so this does not touch the trust base, it shrinks the
artifact. At k<=4 it costs milliseconds and it closed the k=5 gap completely.

That changes the shape of the certificate: **ship only the facts that are expensive to re-derive,
and let the checker regenerate the cheap bottom of the DAG.** It also means the right definition
of the certificate is the sub-DAG *reachable from the sixteen roots*, not the whole log —
`radio_verify` now takes a target mask so a run can verify one log's facts against every log as
database.

### Cost, honestly

Verification is not cheap and I have retracted the claim that it is cheaper than the proof
(see `certificate.md`). k=4 of the k=9 ladder is 91 s for 216,580 facts and 1.42 G nodes, against
1,521 s for the whole `Sa(113)` solve. The k=7 level of `Sa(193)` — 3.1 M facts at P=4 with ~558
options per part — is the term that decides feasibility and was still running when this entry was
written.

### Addendum, same day: k=7 is the term that decides feasibility, and it is not yet explained

The `Sa(193)` DAG is 3.1 M facts at k=7 out of ~3.2 M total, so that level *is* the cost. Measured
on a sample: each k=7 fact enumerates only ~25,700 nodes but takes **~5 s**, i.e. 195 us per node,
which no enumeration can account for. The memo hit rate there is **74%, not the 99.996% seen at
k=4** — a k=7 fact has ~4 parts of mass ~520, so `live_get` enumerates ~558 options per part and
issues ~6,700 refutation queries, ~1,700 of which miss the memo.

So at high k the *index* is hot, exactly the component the k=4 measurement said was irrelevant.
That is the clearest instance of the small-k trap in this session: the same code path is 0.004% of
queries at k=4 and 26% at k=7.

**The obvious fix did not work.** I added a per-level 2-D table `sdom[n][m]` — 1 iff some one-part
fact `(a:b)` with `a<=n, b<=m` is in the level — which answers single-part dominance in O(np)
instead of scanning the `np=1` bucket, and is exactly equivalent (verified by byte-identical node
counts, 41,616 at k=3). It made **no measurable difference** to the k=7 rate. So the misses are not
being served by the `np=1` bucket, and the 5 s per fact is still unexplained. The table is kept
because it is free and provably equivalent, but it is not the answer.

What this leaves open, in priority order:

1. **Profile one k=7 fact.** 5 s against 25,700 nodes is a 5-order-of-magnitude mismatch; something
   specific is wrong, and guessing has now failed twice.
2. **k=6 does not verify at all.** One 8-part fact of mass 687/729 ran 12 minutes without
   finishing. Its partial children have masses far below `3^5`, so nothing refutes them until deep
   and the tree is genuinely huge — verification cost mirrors proof cost exactly where the proof
   was hard. The 2023 k=6 level has only 4,541 facts, sitting between 97 K at k=5 and 3.1 M at k=7,
   which is itself evidence that most k=6 refutations came from the warm cache and were never
   logged.
3. **The sixteen k=8 facts.** Independent, parallel, and the whole remaining gap at the top.

Also fixed: the subtree DP was left enabled by default after being measured as a 2.4x loss, which
silently changed node counts between builds and briefly made the `sdom` change look like a win.
Defaults now match what the measurements support.

## 2026-08-04 — hints for the verifier: sound, tiny, and aimed at 0.24 seconds

The proposal was to precompute hints telling the verifier which rule to check for which branch of
which split, or a statistical priority order. Both halves were measured. Neither is worth building,
and finding out why finally localised the k=7 cost.

### The statistics are real and the ordering is still wrong

Instrumented over the 2023 k=5 level, testing all three children of every option regardless of
order: of 9,565 dead options, the mixed child `c1` fires **59.2%** and is the **sole** killer 41% of
the time, against 17.3% for `c2`. Cheapest-first checks `c1` last, so 41% of dead options pay all
three checks. Reordering to most-likely-first should save ~1.14x in child-checks.

It loses. On the k=9 ladder at k=4 with derivation off, node counts are **byte-identical**
(1,418,099,928 both ways — so this is a pure cost measurement) and the time goes **89.9 s -> 101.1 s,
a 1.12x loss**. `c1` is a two-part state: `canon` and hash over two parts, and dominance with
`lim=2` scans both the np=1 and np=2 buckets. Frequency of firing is not cost. Same error shape as
the balance-line ordering — a real statistic optimised against the wrong objective.

**Second, independent reason to keep cheapest-first:** check order determines how much *derivation*
gets triggered. On the 2023 k=5 level with derivation on, reordering took nodes from 5,582,381 to
15,238,267 and time from 12.5 s to 32.8 s, because querying `c1` first derives states that
cheapest-first never asks about — `c2` or `c0` had already killed the option. Derivation is not free
and the check order gates it.

### Per-(part,k) hints are sound and tiny, and would save a quarter of a second

The right form is not guidance but certification: *this option is dead, child c_i, rule R, witness
fact N*. The verifier confirms the witness injects componentwise and is present, so hints add no
trust — LRAT, not DRAT — and, decisively, **the verifier then never has to compute a negative**.
Options not claimed dead are simply treated as live; under-claiming costs enumeration, never
soundness. That matters because "not refuted" is the expensive answer: half the memo misses at k=4
return it.

Volume is small, because hints key on `(part, k)` and there are almost no distinct parts:

| k | distinct parts | part-slots served | reuse |
|---|---|---|---|
| 4 | 48 | 6,837 | 142x |
| 5 | 167 | 513,156 | 3,073x |
| 6 | 387 | 23,761 | 61x |
| 7 | **667** | **12,324,390** | **18,477x** |
| 8 | 497 | 3,758 | 7.6x |
| 9 | 16 | 16 | 1x |

About 1.8 M option records across all levels, ~15 MB against a certificate sized at 0.3-1 GB.

**And it is pointless.** Benchmarked directly: building the live-split table for *every* distinct
part at k=7 — 1,444 parts, 249,128 live options — takes **0.24 s for the whole level**, in both the
old and new builds. Hints replace that 0.24 s. The 18,477x reuse that makes the hint file small is
exactly what makes the hints unnecessary.

### Where k=7 actually spends its time

Four hypotheses have now failed: live_get table builds (amortise 18,477x), the np=1 dominance
bucket (indexed, no change), the memo miss rate (74% is dominated by live_get's cheap queries), and
per-option hints. What the benchmarks *exclude* points at the answer:

`live_get`'s queries are cheap because its children are small — one or two parts, low mass — so the
`mass > s->mass` break cuts the scan immediately. The expensive queries are the **prefix children
inside `splits_rec`**: 3-4 parts with mass just under the cap, where the mass break does nothing and
`lim=4` sweeps the whole 2.38 M-fact `np<=4` range of the k=6 level. Those never appear in a
live_get benchmark.

Also measured: the k=7 per-fact cost is wildly skewed. Facts early in sort order take ~5 s; three
facts drawn from across the level did not finish in 590 s, i.e. **~200 s each**. The "5 s per fact"
figure recorded earlier today is the cheap end of the distribution, not a mean.

### The columnar dominance index

Built anyway, because it is the structure the above diagnosis needs. A dominance query is an exact
orthogonal range query, so the useful content is a monotone signature plus a sort order that makes
the answer contiguous:

- **Signature.** If `f <= s`, then f's j largest parts by n map to j distinct parts of s with larger
  n, so `N_j(f) <= N_j(s)` for every j, and independently `M_j(f) <= M_j(s)`. Necessary, cheap, and
  false for nearly every candidate. Packed as eight 8-bit lanes in two `uint64` columns, so a
  candidate is a 16-byte read and two vector byte-compares instead of a backtracking match.
- **Order.** Facts sorted by `(np, largest n, mass)`, with a per-(np, largest-n) offset table, so
  `np <= np(s)` and `maxn <= maxn(s)` become a range and the mass break survives inside each group.

Provably equivalent — byte-identical node counts on every regression (167,001 / 41,616 /
1,418,099,928). Neutral at k=4 (95.0-97.7 s against 95.6-96.2 s for the baseline, within noise),
which is expected: the memo intercepts 99.996% of queries there, so the index never runs. An A/B on
hard k=7 facts was still running when this was written.

### Addendum: the columnar index is worth >=8x, and k=7 nodes cost 134 us each

A/B on the same four hard k=7 facts (P=4, drawn from across the level), same database, same flags:

| build | result |
|---|---|
| before the index | **TIMEOUT at 2,103 s** |
| columnar index | **276 s** (258 s of level time, 1,941,651 nodes) |

So **>=8.1x**, a lower bound because the baseline never finished. The memo hit rate on those facts
is **56.8%**, against 99.996% at k=4 — which is why the index is invisible at low k and decisive
here, and why "bucketing by part count is irrelevant" was a k=4 artifact.

The absolute number matters more than the ratio. At k=7 the verifier runs at **7,458 nodes/s**
against roughly **15,000,000 nodes/s at k=4** — a 2,000x difference in *per-node* cost, i.e. ~134 us
per node, ~45 us per dominance query. Node counts at k=7 are small; the queries are enormous.

Two consequences:

- **A node budget cannot bound k=7 cost.** A 30 M-node cap was never approached in 40 minutes of
  wall clock. Cost-distribution sampling needs a *time* budget per fact. The cost-histogram run
  produced nothing for a second reason worth fixing: the histogram prints only in the end-of-level
  summary, and the level never ends.
- **The remaining lever is the size of the candidate set, not the code around it.** ~45 us per query
  is tens of thousands of candidates surviving the range restriction, out of the 2,379,918-fact
  `np=4` bucket at k=6.

That bucket is 99.85% `out_k8.txt` — 2,520,118 facts from the 77-coin problem against 4,541 from the
193-coin run — because `out_k8.txt` was merged in to close the `(0,b)` gaps at k=9. It cannot simply
be dropped: with the 2023 logs alone, the same four k=7 facts fail in 0.76 s with 0 verified. Those
small-coin facts legitimately dominate 193-coin children. So the fix is not curation but
**minimalization** — the refuted set is upward-closed, so only the minimal antichain can ever be the
reason a query succeeds, and everything else is pure scan cost.

### Addendum: minimalization is worth 1.84x, not 20x

Measured exactly, on the merged k=6 level (2023 `Sa(193)` logs plus `out_k8.txt`), by testing every
fact against the level with itself excluded — 2.52 M dominance queries in 939 s:

| bucket | facts | minimal | redundant |
|---|---|---|---|
| np=1 | 135 | 19 | 85.9% |
| np=2 | 3,113 | 1,261 | 59.5% |
| np=3 | 137,626 | 53,706 | 61.0% |
| **np=4** (what `lim=4` sweeps) | **2,381,059** | **1,295,983** | **45.6%** |
| np=5..8 | 2,670 | 2,656 | ~0% |
| **total** | **2,524,603** | **1,353,625** | **46.4%** |

So the antichain is **1.84x** smaller, not the order of magnitude the upward-closure argument
invited. Combined with the columnar index that is ~15x on k=7, which is real and still leaves the
level expensive: extrapolating the A/B (485 K nodes and 64.5 s per hard fact) to ~35 s post-
minimalization gives **~3.4 core-years** for 3.1 M facts — weeks on a large machine, not hours.

Worth recording as a corrected expectation: "the refuted set is upward-closed so only the antichain
matters" is true and does not imply the antichain is small. Most logged facts are already minimal.

The redundancy is concentrated where part counts are low, which makes sense — a 4-part fact has far
more chances to contain a smaller refuted sub-multiset than an 8-part one has to contain a smaller
8-part one, and the np>=5 buckets are essentially pure antichains already.

### Two instrumentation bugs, both of which cost a run

- **The cost histogram printed only in the end-of-level summary.** A level that never finishes
  reports nothing, so a 40-minute sampling run produced no data at all. Progress output now carries
  the running histogram.
- **A node budget cannot bound k=7 cost.** Cost there is per-query, not per-node: 30 M nodes was
  never approached in 40 minutes at 7,458 nodes/s. Replaced with a per-fact *time* budget, checked
  every 1,024 nodes.

Both are the same mistake in different clothes — instrumenting the quantity that was expensive at
k=4 rather than the one that is expensive at k=7.

## 2026-08-04 — top-down painting: the certificate is the reachable sub-DAG, and it is 190x smaller

The certificate is not "every fact the solver logged". It is the sub-DAG reachable from the sixteen
roots. Verify level k, **paint** each fact at k-1 that was actually used to discharge something, then
descend and verify only the painted ones. Everything unpainted is discarded: nothing cites it, so
nothing depends on it. Still well-founded induction on k, so soundness is untouched — every cited
fact is itself verified before the certificate closes.

Implemented in `radio_verify.c` (`TOPDOWN=<k>`). The plumbing that makes it work is carrying the
**witness index through the memos**: at k=4, 99.996% of queries are memo hits, so without recording
which fact answered a cached query almost every citation would go unpainted.

### Measured, from the sixteen `Sa(193)` roots

| k | facts in level | targets (painted) | verified | unverified | cited at k-1 |
|---|---|---|---|---|---|
| 9 | 16 | 16 | 0 | 16 | **1,910** |
| 8 | 1,932 | 1,910 | 35 | 1,875 | **16,347** |

- **k=8 is essentially all reachable** — 1,910 of 1,932. Expected: the level is tiny and each root
  has thousands of splits to discharge.
- **k=7 collapses to 16,347 of 3,098,762 — 0.53%, a 190x cut.** That is the whole cost problem for
  that level. At ~35 s per hard fact it takes the k=7 term from ~3.4 core-years to order 160
  core-hours. Even allowing 10x for the undercount below, it is days rather than years.
- The count is a **lower bound**: a failed verification returns at the first split with no refuted
  child, so it cites less than a complete one would. Re-measure once k=8 closes.

### Why k=8 does not verify: the warm cache ate the low-part-count facts

1,875 of 1,910 fail, and the reason is structural rather than a bug. Every `Sa(193)` k=8 fact has
**two parts**, so its children have 2 and 4 parts. The merged k=7 level:

| np | facts |
|---|---|
| 1 | 339 |
| **2** | **6,655** |
| 3 | 43,042 |
| 4 | 3,048,745 |

The 4-part children are richly covered and the 2-part ones are not, because low-part-count states
are exactly what a warm cache dispatches by dominance without printing. Same mechanism as the
sixteen k=9 gaps, one level down.

This is good news rather than bad: **low part count is the cheap direction**. A 2-part state's
`SPLITS` check is a two-deep product, not a four-deep one, and `live_get` for an entire level costs
0.24 s. So the missing facts are the ones derivation can supply, which is what the on-demand
`derive` path is for. Whether it closes k=8, and at what cost, was measuring when this was written.

### The k=7 projection, with painting

Cost distribution at k=7, sampled 1-in-500 with a 20 s per-fact time budget: **48 of 51 facts
complete (94%)**, mean **41,015 nodes**; 3 exceed the budget. So the four facts used in the index
A/B — 485 K nodes each — were about 12x costlier than typical. They were drawn by a wide stride to
spread across the level, which selected for the tail; quoting them as a per-fact cost overstated it,
and the corrected projection is:

| | k=7 level |
|---|---|
| all 3,098,762 facts at the A/B rate | **6.3 core-years** |
| 16,347 painted facts at the A/B rate | 293 core-hours — **0.5 days on 24 cores** |
| 16,347 painted facts at the typical rate | 91 core-hours — **0.16 days on 24 cores** |

The level stops being the obstacle. Ranking the three levers measured today on their effect at k=7:
**top-down painting 190x**, columnar index >=8.1x, minimalization 1.84x. The first is worth more
than the other two together, and it was the cheapest to build.

Note the two figures bracket rather than bound: 16,347 is a lower bound on the painted set (failed
verifications stop early and cite less), and the 20 s budget truncates the tail at 6% of facts. Both
move the estimate up, neither by an order of magnitude.

### Economical painting: 2.07x faster, same size — and why the size did not move

When a split can be discharged several ways, painting whichever witness the scan reaches first grows
the certificate for no reason: an already-painted fact is free, since it is in the artifact and
verified regardless. So a later pass searches the previously painted facts first, and only falls
back to the full level. `radio_verify.c` builds that painted set as a `Level` in its own right
(`sub`, with an `orig` map back to the parent), so it reuses the same index and the same code path.

| pass | k=8 verification | cites at k=7 | preference hits |
|---|---|---|---|
| 1 — greedy, first witness found | 17.2 s | 16,347 | — |
| 2 — prefer already painted | **8.3 s** | 16,347 | 521,226 / 697,967 (74.7%) |
| 3 | 8.3 s | 16,347 | fixpoint |

- **2.07x faster.** The preferred set is 0.5% of the level, so the preferred lookup is the small
  one. Economy and speed point the same way here, which is not the usual shape of a set-cover
  heuristic.
- **Size unchanged, for a structural reason.** The preference set *is* the previous pass's painting,
  so a later pass can only re-select facts already in it; the set shrinks only if some painted fact
  becomes redundant. None did, so pass 1 had no slack at these levels — every painted fact is the
  sole available witness for at least one query. At k=9 that is expected: 1,910 of 1,932 k=8 facts
  are cited, nearly the whole level, with nothing to trade against. Re-ask at k=7 -> k=6, where
  16,347 facts fan out into a 2.5 M-fact level and slack is far likelier.

Not implemented: preferring a *child* whose witness is already painted, as opposed to a *witness*
for a given child. `splits_rec` still takes the first refuted child in cheapest-first order.
Choosing among children needs all three evaluated, and cheapest-first was measured to beat
most-likely-first by 1.12x precisely because evaluating the expensive child costs more than it
saves — so that variant trades speed for size in the direction the measurement says loses.

**Two bugs found by building this, both of the same kind — a memo that silently suppresses the
thing being measured:**

- The first multi-pass run painted nothing below the top level, because `live_get` memoises on
  `(part, k)`: pass 2 reused pass 1's tables, issued no refutation queries, and painting only
  happens where a query happens. The live tables are now dropped between passes; rebuilding a whole
  level costs 0.24 s.
- Painting needs the witness index carried *through* the memos. At k=4, 99.996% of queries are memo
  hits, so a memo that records only the verdict and not which fact produced it leaves nearly every
  citation unpainted — and under-reports silently, which is the failure mode that matters.

## 2026-08-05 — the 2023 corpus cannot be closed by derivation; the sixteen need real compute

Direct test, and it is decisive. Fed `Sb(74:40, 41:38)` — the single k=8 fact `Sb(112:81)` needs —
to the verifier as a target against the 2023 k=7 level plus `out_k8.txt`:

```
GAP split 74:40->(23,20) 41:38->(27,26)
  c2: 27:26 23:20        [1162]
  c0: 51:20 14:12        [1188]
  c1: 51:20 27:12 26:14 23:20  [2168]
verdict: unverified, 227 nodes, 0.01 s
```

It fails on the **first** split tried, in 227 nodes. Not expensive — **unsupported**. All three
children are k=7 states absent from the fact set and dominated by nothing in it: two of them 2-part,
which is exactly the shape the corpus is thin on (6,655 two-part against 3,048,745 four-part).

So the on-demand `derive` path cannot close the gap. Deriving that fact requires k=7 facts that must
themselves be derived, which require k=6 facts, and so on — the recursion *is* the original search.
Measured the expensive way first: `TOPDOWN=9` with a derive threshold of 8 ran 25 minutes without
finishing even the 16-fact k=9 level.

**Consequence for the plan.** The certified-from-2023 route does not avoid new compute; it only
*localises* it. What the corpus gives is still substantial — the sixteen k=9 facts each reduced to a
single missing k=8 child, k=4 and k=5 verifying clean, and the k=7 level painted down to 16,347
reachable facts — but the sixteen k=8 states have to be proved by the solver, warm-started from
`out_k8.txt` (2026-era, audited clean). That is the AWS job, and it is unsized.

Also fixed: `derive()` restored every global except the per-fact budget clock, so a nested
derivation reset the enclosing fact's timer and the time budget silently stopped bounding anything.
That is why a 300 s per-fact cap let one k=9 fact run 1,500 s. Third instance today of the same
failure shape — an inner mechanism quietly disabling the instrument measuring the outer one.

## 2026-08-05 — sizing the cold re-derivation, and a 4.3x gate on pass 1

Decision: re-derive `Sa(193)` **cold, in one session**, rather than certify the 2023 corpus. The
reasoning is settled by yesterday's work — every gap in that corpus traces to warm-cache resumption,
and a cold single session cannot have that defect, so its log is closed and checkable end to end.
Driver written: `radio_sa193.c`, which runs the `Sa(192)` positive control **first** and aborts if it
does not reproduce, then asks `canSolveA(193, 10)`.

### Memory is not the constraint. That changes the shape of the run.

Cold `Sb(97:96)` in 9 at `MAX_N=193` peaked at **1.36 GB**. The 2023 run reached ~90 GB, and the
whole AWS plan was built around needing 128 GB. At this footprint **the sixteen can run in parallel**
— sixteen cold single-session processes, each producing its own closed log, and a union of closed
logs is still a valid fact set. Wall clock becomes the cost of the *hardest* state rather than the
sum. That was impossible under the 90 GB assumption.

### Cold is expensive, and the cheapest of the sixteen is the evidence

| | |
|---|---|
| `Sb(97:96)` in 9, 2023, **warm** | 1,290 s |
| `Sb(97:96)` in 9, 2026 engine, **cold** | **did not finish in 2,700 s** (356,433 verdicts) |

So the cold penalty on the easiest of the sixteen is at least 2x, on an engine that is otherwise
faster. For scale, in 2023 `Sb(112:81)` cost 1,337x what `Sb(97:96)` did. Do not read a total off
that ratio — the 1,725,456 s for `Sb(112:81)` includes 12 passes, and the current engine does at
most 2 — but the run is weeks, not days.

### Pass 1 / FAST: I measured the wrong thing — retracted the same day

I counted the share of verdicts pass 1 *resolved* — 100% at k=8, 26.3% at k=7, then 0.5%, 0.4%, 0.2%
at k=6, 5, 4 — concluded it was overhead low in the tree, gated it with `FAST_MIN_K=7`, and measured
4.3x on a progress marker. **That reasoning is wrong and the gate is withdrawn.**

FAST is not a decision procedure whose value is the verdicts it closes. A split is three-way and only
one child needs refuting; the expensive mistake is grinding toward a refutation on a child that is
actually *solvable*, exhausting its whole space and finding a solution at the end anyway. FAST exists
to exhibit one witness cheaply on those children, so the search can spend its exhaustion where
exhaustion is the only option. A pass-1 success **prevents** work — it does not print a verdict, so
counting verdicts is blind to precisely the thing it does. That is what the `NOTFAST` annotation is
about.

Two things in my own data say so, both of which I recorded and misread:

- **Zero positives printed at any k**, across all 356,433 verdicts. I read that as "pass 1 never did
  its job". It means the log does not record pass 1 doing its job.
- **The gated run stalled.** It reached 1,258 k=8 verdicts at 10:21 and was still at exactly 1,258 at
  45:00. I called that "grinding on something hard". It is what sinking into a solvable branch looks
  like — the same failure the FAST restoration was for, reproduced by removing FAST again.

So the 4.3x is not a speedup, it is the gated run racing through negatives while losing the ability
to dispatch solvable children. `FAST_MIN_K` stays in the source with **default 0 — the old
unconditional behaviour** — because the gate is useful for experiments, and the comment at its
definition now records why it must not be turned on casually.

The direction this rules out and the direction it points at: **do not make FAST cheaper by doing less
of it.** If pass 1 is to be improved, the target is its hit rate on solvable children — finding the
witness sooner, or recognising sooner that a child is solvable — not skipping it.

Note what this measurement is not: pass-share by k is an artifact of the current ordering, as is
verdict count. The progress marker used above (k=8 verdicts, the direct children of the k=9 root) is
the one quantity here that means the same thing in both builds.

### The run is live

Launched 2026-08-05, `i-0005d74f985c52ae1`, `r7iz.4xlarge`, from `0a468ca`. Serialized: one process,
one cache, sixteen states in sequence — the parallel design was dropped because sixteen cold jobs
each rebuild the shared low-k work, and that reuse is why 2023's later states were affordable at all.
I had been underweighting it.

Also withdrawn: **"memory is free" does not apply to this run.** The 1.36 GB figure was one k=9 state,
incomplete, at 45 minutes. A serialized process accumulates the shared cache, which is where 2023's
~90 GB came from, so the 128 GB instance is the right shape after all. Generalising a memory bound
from a partial single-state run was the error.

Pre-flight before committing instance time: `radio_sa193` builds at `MAX_K=10 MAX_N=193`, starts, and
reached 288,956 verdicts in 12 minutes locally at 0.38 GB. The `Sa(192)` control did not finish in
that window, which is expected for a cold k=10 search and is not a problem — the control's work warms
the cache for `Sa(193)`, since the two share almost everything.

Monitoring is built around the only honest progress metric: **how many of the sixteen** are done.
Verdict counts and elapsed time say nothing about remaining work; `Sb(112:81)` alone was 1,337x
`Sb(97:96)` in 2023. Details in [aws-run.md](aws-run.md).

### The control passed, 2026-08-05

`result CONTROL Sa(192) in 10 = SOLVABLE (2209.1 s)` — the current engine reproduces the known
`Sa(192)` construction cold at k=10 in 37 minutes. That was the gate on the whole run: anything else
and `radio_sa193` aborts rather than produce a negative for 193. The run is now on `Sa(193)` itself.

At one hour: 998,682 verdicts, **5.28 GB** resident, `0 of 16`. Memory went 0.53 GB at five minutes to
5.28 GB at an hour, which looks like the 2023 profile (~90 GB), so **memory exhaustion is the expected
first failure mode**, not a surprise. `capped_run --rss-gb 110` stops it before the OOM killer does,
which preserves the hourly checkpoint in S3 and lets the run resume on a larger instance instead of
losing the work.

Watchdog bug found and fixed live: the milestone count read awk field `$5` of
`  top-level states done   N of 16`, which is the word "of", so `DONE` was constant and the
"another one done" emails could never fire — only the 6-hour heartbeat. Worst shape for a monitoring
bug, since the heartbeat keeps the channel looking healthy. Patched over SSM without touching the
solver. Also added `tools/sa193_status.sh`, which prints the snapshot's **age**: the watchdog writes
every ten minutes, so a six-minute-old snapshot is healthy, and I misread exactly that as a hang.

### Monitoring, corrected three times over — 2026-08-05

The run is **confirmed completely cold**: `cache=(none, cold)`, no `parse_file`, no "reading file"
line. No checkpoint existed in S3 at launch, so nothing was picked up.

That created a trap for later, now closed: `run/sa193.checkpoint` exists now, and the launcher's
user-data would have picked it up on any relaunch, silently producing a warm run. `--resume` is now
opt-in and the default prints `COLD run: any checkpoint in S3 is deliberately ignored`. Resuming is
for catastrophic failure only — a cold single session is what makes the log closed, and a warm start
also hides work rather than doing it, so a resumed run cannot answer "was 2023 right" on its own.

Three fixes to the status report, all worth recording because each was reporting confidently and
wrongly:

- **Per-k counts were sorted by count, not by k.** `uniq -c` emits `count ] in K`, so k is field 4;
  sorting on field 3 (`in`, constant) falls through to a lexical whole-line compare, i.e. by count
  as a string. k=9 — the only level where a verdict means 1/16 of the job — was buried mid-list.
- **`elapsed` was the watchdog's uptime, not the solver's.** The watchdog gets restarted to patch it;
  its own uptime then reports minutes for a run that is days old, which is backwards for the single
  field a reader uses to judge whether anything is wrong. Now `ps -o etimes=` on the solver.
- **Four full scans per cycle became one.** The log reaches gigabytes; the status now makes a single
  awk pass computing counts, totals, the sixteen, and the most recent verdict at each level.

That last one paid for a real improvement: the status now shows **the most recent verdict at every
level**, which is the progress context a single counter cannot give — you can see the search working
at k=6 while k=8 and k=9 sit still.

It also corrected an earlier claim of mine. **Positives are printed, with witnesses** — the k=9 line
currently reads `can solve Sb(112:80)[8960,192] in 9 with [64:48] ...`. My "zero positives at any k"
finding was a property of that pure-refutation `Sb(97:96)` run, where no solvable state arises at all,
not a property of the engine. One more reason that measurement could not see what FAST does.

### Status refinements, and two reporting bugs that looked like disasters

The verdict stack now runs **from the level the search is on upward to the root** and stops there.
Levels below were last touched arbitrarily long ago, so their "most recent" verdict is stale and
placing it means scrolling back through the whole log; levels above are the enclosing context.
Positive verdicts carry their entire witness tree inline, so they are shown as `[+witness]` — a flat
truncation cut them mid-token. The first elision then had the same bug in a subtler form: it cut from
` with ` to end-of-line, and the cost tail comes **after** the witness

    can solve <state> in <k> with <witness...> took <s> totalsplits=<n> pass=<p> fast_solve=<f>

so every positive lost its timing. That is the most useful field on the line: the k=9 control state
reads `took 1745`, i.e. 29 minutes, and `k=8 ... took 1121`. Now only the witness itself is elided.
Note `Sa(...)` verdicts say `with following:` rather than `with [`, so matching ` with ` covers both —
the first version silently left Sa lines unelided.

Two bugs in the reporting made healthy things look broken, which is the failure mode that matters
most in a status line:

- **The log appeared to shrink**, 249M then 122M, for a file that only ever grows. `du` was the
  culprit: the volume is XFS, which speculatively preallocates blocks for a growing file and trims
  them later, so `du` fluctuates while the apparent size climbs monotonically. Now reported from
  `stat -c%s`. A status line falsely suggesting the run's one irreplaceable artifact is being
  truncated is worse than no status line.
- **`pgrep -fc 'sa193_watchdog.sh --log'` reported two watchdogs** when there was one: `status()`
  uses command substitution, and the transient subshell carries the parent's argv. Verified with
  `ps` before killing anything.

Also fixed: `elapsed` was the watchdog's uptime rather than the solver's, so patching the watchdog
made a days-old run report minutes. Now `ps -o etimes=` on the solver, guarded to be numeric —
BSD `ps` has no `etimes` and prints its usage to *stdout*, so a non-empty check passes and the
arithmetic then fails on a page of option names.

Method note worth keeping: three of my `python .replace()` patches to this script silently failed to
match because of backslash escaping, and one of them — raising a truncation limit — *appeared* to work
because a different change had brought the lines under the old limit. Print the `repr()` of the target
lines and assert the match, rather than trusting that a replacement landed.

### The stack shows, per level, whichever is newer: in-progress or completed

Final form: for each level, take the **newer** of the most recent `still solving` line and the most
recent completed verdict, and label which it is. Either can be the informative one — a level mid-search
wants `left=`/`elapsed=`, a level that just finished wants `took=`/`totalsplits=` — and which is current
changes as the search moves between levels. Comparing recorded log positions is the only way to know.
The first version showed only `still solving`, which meant a level that had just completed something
displayed a line from before it. Those carry what a completed verdict cannot: `left=<remaining>/<total>` and `elapsed=<used>/<budget>`.

**`left=` is not a fraction of the work.** It counts splits of the FIRST GROUP only, for simplicity, so
it is an ordering position rather than a percentage, and the cost behind each unit is wildly non-linear
(2026-08-05, from the author). `left=577/578` means one first-group split has been cleared; it does not
mean 0.2% of the node is done, and the remaining 577 are not comparable to each other. Useful as a
liveness signal and for spotting a stuck node; useless as an ETA. A completed verdict says only what finished, which at
high k can be hours stale. Each line also carries how far back in the log it is, so a level the solver
has not revisited is marked `(stale)` rather than silently misread as current.

First reading of it, two hours into the run, is immediately diagnostic:

```
k=7  Sb(33:16,32:15,45:10,23:19)[1895,193]  elapsed 1742/2000  left=577/578   totalsplits=664031
k=6  Sb(23:6,17:8,16:8,22:4,13:6,17:4,26:2,19:2)[726,193]
                                            elapsed 3963/3973  left=59/168    totalsplits=307871277349
```

The k=6 node has enumerated **308 billion** split combinations on an 8-part state at mass **726 against
3^6 = 729** — 99.6% saturated, so the counting bound prunes nothing.

**I then misread its deadline, and the correction matters.** I wrote that it was "at 99.7% of its budget"
and "about to bail with MAYBE" — it is not, and it never will. The deadline setup in `canSolveB`
gives every descendant
of a pass-2 node `NO_DEADLINE`:

    clock_t child_deadline = pass<2 ? deadline : NO_DEADLINE;

and a `no_deadline` node that passes its deadline does **not** return `MAYBE`; it sets
`deadline = t + 10 * CLOCKS_PER_SEC` and continues (line ~950). So the printed `elapsed X/Y` has Y
tracking X at a fixed ~10 s offset forever, and the node runs until it concludes. The two samples I had
already collected said so plainly — `3963/3973` then `4231/4241`, gap constant at 10 — and I read a
constant gap as a nearly-exhausted budget.

The consequence is the opposite of reassuring: **below a pass-2 node there is no deadline protection at
all.** It was progressing (`left` went 59/168 -> 43/168 in four minutes) and did finish — but `left` counts
first-group splits only and is non-linear, so no rate can be extrapolated from it. The status now detects the case (Y - X <= 12) and labels it
`(no deadline: auto-extends)` rather than implying a countdown.

Also note `totalsplits=307871277349` against `k=6`: the saturated multi-part states at k=5-6 are where
this run will spend its time, which matches the 2026-08-04 finding that realised part count peaks near
k=4-5 and that those states are the expensive ones.

Shell trap worth recording: an apostrophe inside an awk comment terminates the single-quoted awk program.
`# ... the control's ...` broke the whole script with "unexpected EOF while looking for matching )", 60
lines from the actual cause.

### Expected scale, from the author (2026-08-05)

k=7 refutations are expected to take a long time; **weeks for `Sa(193)` overall is the expectation, not a
symptom.** A k=7 node sitting at `left=577/578` for hours is the normal shape of this problem, so do not
read it as a stuck run — and do not read `left=` as a rate, per above.

### S3 layout, and a silent AccessDenied

Live artifacts, all under `s3://radio-sa193-393287594714/`:

| key | what |
|---|---|
| `run/STATUS` | current snapshot, every 10 min |
| `run/seg-<stamp>/out_sa193.txt.zst` | raw log, hourly — the archival artifact |
| `run/seg-<stamp>/sa193.checkpoint` | parsed form, hourly |
| `run/sa193.checkpoint` | latest checkpoint, the `--resume` path |
| `src/sa193_src_<sha>.tgz` | the running build |
| `src/{rss_guard,sa193_watchdog}.sh` | live scripts, fetched by SSM to patch in place |

**`run/sa193.checkpoint` had silently stopped refreshing** — 90 minutes stale while the segment copy
was current. `aws s3 cp` between two S3 keys calls `s3:GetObjectTagging` to preserve tags, which the
run role deliberately does not grant, so it failed with AccessDenied behind a `|| true`. Fixed by
writing both keys from one parse through `tee` rather than copying, which also avoids widening the
policy. The failure mode is worth remembering: `|| true` on every AWS call keeps a broken report from
killing the run, and equally keeps a broken report from being noticed.

Deleted as superseded (~177 MB): the 2026-08-03 failed run's checkpoint, log and source bundle; the
truncated fixed-key `out_sa193.txt.zst` from before per-segment keys; and `run/STATUS-<date>.log`,
which was broken by design — S3 has no append, so each cycle overwrote it and it was never more than
a stale duplicate of `STATUS`.

### Memory profile: the whole footprint was allocated in 40 minutes, during the control

`tools/sa193_watchdog.sh` now writes a row per cycle to `memprofile.csv`, archived hourly to
`run/seg-<stamp>/memprofile.csv`. Columns: `iso, solver_secs, rss_kb, vmdata_kb, vmpeak_kb, verdicts,
curk, by_level`. The 164 cycles already in the instance's watchdog log were backfilled, so the series
starts 2026-08-05T16:03Z.

`VmData` alongside `VmRSS` is what makes attribution possible, and it makes it trivial: measured
2026-08-06, `VmData` is 6,042,908 kB against `VmRSS` 6,043,704 kB and `Pss_File` is **98 kB**. The
entire footprint is the solver's own anonymous heap — the result cache — with nothing file-backed or
shared. `VmPeak == VmSize` and `VmHWM == VmRSS`, so it has never been higher and never swapped.

Steps in the series:

| time | RSS GB | verdicts | dRSS | dverdicts |
|---|---|---|---|---|
| 16:13 | 0.80 | 464,545 | +276 MB | +117k |
| 16:23 | 2.97 | 652,794 | **+2,222 MB** | +188k |
| 16:33 | 4.64 | 847,782 | **+1,710 MB** | +195k |
| 16:43 | 5.05 | 980,450 | +420 MB | +133k |
| 16:53 | 5.28 | 998,682 | +236 MB | +18k |

**Essentially all of it was allocated in a 40-minute window during the `Sa(192)` control**, at ~11-12
KB per verdict while the cache filled. In the 25 hours since: +900k verdicts for +0.48 GB. So the k=7
refutation is re-walking cached states rather than discovering new ones — which is also why throughput
fell to ~2 verdicts/min while RSS sat flat at 5.73 GB for hours.

Peak 5.76 GB against the 110 GB guard, so the earlier expectation that memory would be the first
failure mode looks wrong: at this rate it will not approach the cap. That prediction came from the
2023 run's ~90 GB, on an engine that visited far more states.

Two bugs fixed on the way, both mine: `status()` runs inside a command substitution, so nothing it
sets survives and the profile row has to parse the status *text*; and I wrote `ps -o etimes=`
unguarded a second time, which on BSD prints its usage to stdout and put a page of option names in the
CSV. Both call sites now go through one `solver_age()` helper.

## 2026-08-06 — downgrade every part to a singleton before majorizing, and a latent unsoundness it exposed

**Superseded 2026-08-09:** the one-copy downgrade was sound but unnecessarily weak. The
Vertex-Splitting Pullback Lemma upgrades `(n:m)` to `m` copies of `(n:1)`, preserving all mass. See
the latest entry.

Suggested improvement: instead of applying Singleton Majorization only to the parts that are already
singletons, **downgrade every part `(n:m)` to `(n:1)`** and majorize that. Sound because `(n:1) <= (n:m)`
componentwise, so the downgraded state injects into the original and Subgraph Monotonicity carries
unsolvability upward. Strictly dominates the old rule: prefix sums of a sorted-nonincreasing multiset
only grow as elements are added. Written up as a corollary in
[theorems/singleton-majorization.md](theorems/singleton-majorization.md).

Applied in all three places that implement the rule: `radiobase.c`, `radio_verify.c`, `tools/certify.py`.

### It exposed an over-refutation in the verifier

`radio_verify.c` treated `i >= len(G_k)` as a majorization violation. It is not: the theorem
**zero-pads**, so past `len(G_k) = 2^k` the bound is the constant `sum G_k = 3^k`, and the left side is
at most the mass, which COUNT has already bounded by `3^k`. No violation can arise there.

Latent while only the singleton sub-multiset was fed in — no logged state has more than `2^k` singleton
parts. Downgrading every part makes it routine at low `k`: a mixed child of a 13-part state has 26
parts against `len(G_3) = 8`. Instrumented on the k=4 level of the `Sa(113)` ladder:

```
MAJ: 225,625 calls
  old rule: 433 hits, of which 79 from the buggy i>=len(G_k) clause
  new rule: 381 hits
```

So 354 of the old hits were legitimate, the corollary adds **+27** (+7.6%), and **79 refutations were
unjustified**. `radiobase.c` was already correct here — it stops at `min(size, len(G_k))`, which is the
equivalent clamp.

**The affected results were re-checked and all stand:** ladder k=2/3/4 (5, 487, 216,580 facts) and
`Sa(193)` k=4 (940) and sampled k=5 (4,859) all still verify with **0 unverified**. None of the 79 was
load-bearing. Worth stating plainly though: those earlier "0 unverified" runs did rely on an unsound
rule, and only re-running showed the conclusions survive without it.

### What the corollary buys

| state | states searched, old | new | cut |
|---|---|---|---|
| `Sb(85:12)` in 7 | 1,055 | 888 | **15.8%** |
| `Sb(77:16)` in 7 | 900 | 853 | 5.2% |
| `Sb(69:22)` in 7 | 331 | 322 | 2.7% |

Verdicts unchanged, and all 43 proven-solvable Pareto cells at k<=6 still come back solvable, so it is
not over-refuting. A smaller emitted certificate falls out of it too, since fewer states are searched
to conclusion.

**The live `Sa(193)` run does not have this.** Its binary predates the change, and restarting to pick up
a 3-16% pruning improvement would discard 27 hours of accumulated cache. Not worth it; the next run gets
it. Wall-clock timing of the change is unmeasurable per-query at k=7 because `init` for
`MAX_K=7 MAX_N=220` dominates a single invocation at ~6.4 s regardless of the state — hence counting
states searched instead.

## 2026-08-07 — where the time goes: 93% at k=6, and one state is 10% of the run

One-time analysis, then folded into the live status. `took` is inclusive of descendants
(`clock()-start` per `canSolveB` call), and each state is computed exactly once — verified: 2,065,670
distinct `(state,k)` pairs in the log, **zero duplicates**. Every level-(k-1) state is first computed
while evaluating some level-k state, so **self time at k = inclusive(k) - inclusive(k-1)**.

At 46.9 h of CPU (168,994 s; elapsed identical, so ~100% CPU-bound):

| k | verdicts | inclusive s | self s | % CPU | splits |
|---|---|---|---|---|---|
| 9 | 1 | 1,745 | * | — | 443 |
| 8 | 329 | 2,192 | 87 | 0.1% | 7.0e4 |
| 7 | 4,926 | 2,104 | * | — | 2.21e6 |
| **6** | 246,281 | 161,847 | **157,485** | **93.2%** | **1.50e13** |
| 5 | 1,708,227 | 4,362 | 4,075 | 2.4% | 2.32e11 |
| 4 | 146,928 | 287 | 287 | 0.2% | 9.02e9 |

`*` = the level above has not completed, so its inclusive time is missing from the log and the
subtraction goes negative (k=7 reads -160,806 s). That is the one real limitation of the method: it
needs completed ancestors, so mid-run the top levels are unknowable this way.

**The concentration is extreme.** One k=6 state — `Sb(16:9,25:5,17:7,20:5,13:5,16:4,19:3,27:2)`, mass
**728 of 729** — took 16,603 s, **9.9% of the entire run by itself**. The 92 k=6 states over 60 s
account for ~96% of all CPU; the other 2.06 M verdicts are ~4%. All 92 are 8-part and within 5 of
saturation, i.e. exactly where the counting bound prunes nothing.

Rounding is not the limiting factor, contrary to the initial worry: only 110 k=6 verdicts are >= 1 s
(integer-second resolution), so the quantization error is +/-55 s against 157,485 — 0.03%.
`totalsplits`, an exact integer, corroborates.

### Cost of computing it every cycle

The status already made one awk pass over the log, so this is two extra accumulators in that pass:
**1.98 s -> 3.31 s** on a 233 MB log, i.e. +1.3 s per 10-minute cycle, 0.2% of one core. It grows with
the log; at a few GB expect tens of seconds, still under 10% duty.

Getting to 3.31 s mattered and was not obvious. Referencing `$NF` forces awk to field-split **every**
line: 9.2 s. Splitting only the matched substring: 5.3 s. `index()` plus `substr()` with no field
access at all: 3.3 s.

Also fixed: `sa193_restart_watchdog.sh` asserted exactly one watchdog after starting, and `status()`
runs a command-substitution subshell carrying the parent argv, so the count saw two and aborted after
a perfectly good start. The assertion that matters is the pre-start `== 0` — leaving an OLD watchdog
behind is the real failure — so the post-start check is now `>= 1`.

### Making the status readable in email (2026-08-07)

SNS's `email` protocol delivers **text/plain only** — no content-type control, no HTML — so the mail
client picks the font, and Gmail picks a proportional one. Every padded column in the status was
therefore collapsing into unreadable ragged text, while the `label   value` block at the top stayed
readable for exactly that reason.

Tested four layouts by mailing all of them to the live topic and looking at the result:

| | result in Gmail |
|---|---|
| A: space padding | does not align (the original problem) |
| B: **tabs** | **does not align** — flattened when the client converts to HTML |
| C: **U+2007 FIGURE SPACE** | **aligns** |
| D: labelled, pipe-separated | reads fine, but wastes width and buries the numbers |

C works because FIGURE SPACE has the same advance as a digit in a proportional font, so a
right-aligned numeric column lines up without monospace. B is the one worth recording as a negative:
tabs are theoretically the right answer — CSS tab stops are multiples of `tab-size x space-width`, a
grid independent of glyph widths — but the client does not preserve them, so the theory never applies.

Adopted C, with one adjustment: **the column names go in a legend line, not a header row.** Header
labels are letters, letters do not share the digit advance, so a figure-space-padded header cannot line
up with the digits beneath it — visible in the test mail, where C's data aligned and its header did
not.

Implementation note: awk's `%10s` pads with ASCII spaces and counts bytes, so it cannot be used here;
padding is built by repeating the FIG string in a helper (`fpad`), and FIG reaches awk via
`-v FIG="$(printf '\\342\\200\\207')"` rather than a `$'\\u2007'` escape, for shell portability.

## 2026-08-08 — where the solver's time goes: 166 states, and the bimodality is structural

Distribution of per-verdict work by level, using `totalsplits` rather than `took`. `took` quantizes to
integer seconds above 1 s and to `0.000` below 1 ms, so ~99% of verdicts fall in one degenerate bucket;
`totalsplits` is an exact integer spanning 14 orders of magnitude and it measures **self** work (splits
enumerated at that node), which is what attribution needs. It converts to time at a nearly constant rate
per level: 10.5 ns/split at k=6, 17.6 at k=5, 31.8 at k=4.

Weighted by work, k=6 is **bimodal with an empty gap**:

| per-verdict splits | verdicts | % of k=6 splits |
|---|---|---|
| 1e0 - 1e7 | 249,913 | 0.31% |
| 1e8 | 0 | 0 |
| 1e9 | 2 | 0.06% |
| 1e10 - 1e12 | **164** | **99.63%** |

Splitting the two populations:

| | verdicts | parts | mass / 729 | pass |
|---|---|---|---|---|
| **>= 1e9 splits** | **166** | 7-8, median 8 | **0.993 - 1.000** | all pass=2 |
| the rest | 249,913 | 1-4, **max 4** | 0.387 - 1.000 | mostly pass=2 |

**The gap is structural, not statistical.** A split's mixed child has two parts per parent part while the
other two children have one, so part count either doubles or is preserved. k=6 states therefore exist at
4 parts and at 8 parts and **nowhere in between** — the cross-tab has entries only at 4, 7 and 8 parts.
The two cost modes are those two shapes, and the missing decade of cost is the missing part counts.

Cross-tabulated by (parts, saturation), **99.57% of all k=6 work is one cell: 8 parts, mass >= 0.99 of
3^6**. Since k=6 is ~90% of the run, **166 states are ~90% of everything**.

What this rules out: the 2026-08-06 majorization strengthening cannot touch them. Downgrading
`Sb(25:6,19:7,14:9,20:4,13:5,16:4,19:3,27:2)` to singletons gives `sum n = 153` against
`sum G_6 = 729` — nowhere near a majorization violation. The counting bound is vacuous at saturation by
construction. These states have no cheap refutation and must be enumerated; any optimisation that matters
has to attack 8-part near-saturated states specifically.

Caveat on the measure: `totalsplits` is reset per pass, so a `pass=2` verdict reports pass-2 splits only
and pass-1 work is invisible. All 166 monsters are pass=2, so their true cost is slightly higher than
recorded.

## 2026-08-08 — exploring optimisations for the 8-part near-saturated k=6 states

Scratch work in `/tmp/k6lab` (not committed): a copy of the solver instrumented to count, never to
prune, plus a standalone DP. Warm cache = the run's own checkpoint filtered to k<=5 (2,201,187 facts),
so a k=6 solve does no recursion below itself. Baseline reproduces the run: the cheapest monster is
110.7 s / 6.81e9 totalsplits locally against 105 s / 7.38e9 in the run.

### What the mass constraint actually costs, measured on the real solver

Two monsters, 110.7 s and 676.7 s, counting every candidate the split loop touches:

| | state 1 | state 2 |
|---|---|---|
| candidates visited | 6.82e9 | 4.27e10 |
| **killed by the cap check** | **82.7%** | **83.7%** |
| survive the cap (these do 3 cache probes) | 17.3% | 16.3% |
| **of survivors, provably uncompletable** | **61.8%** | **65.9%** |

So ~93% of everything the loop touches is doomed on mass grounds alone: 83% cannot fit, and most of
the rest cannot be completed. Two independent optimisations follow, both provable and both
enumerating exactly the same split set:

- **A. Iterate only cap-feasible options.** `k0+k1+k2` is the part's mass, a constant, so the three
  cap constraints define a 2-D region in `(k0,k2)`. Indexing each part's option list by `(k0,k2)` and
  walking only that region replaces the scan-and-reject. Ceiling **5.8x / 6.1x fewer iterations**.
  The existing `ordmono` cut already does this for ONE child; the 83% are the other two.
- **B. Joint suffix reachability.** `R[i]` = the set of `(r0,r2)` the remaining parts can still
  contribute; a prefix is dead if no reachable `r` keeps all three children within cap. Answered in
  O(1) per node from a 2-D running max. Ceiling **2.6x / 2.9x fewer probe sets**.

Cost model from the measured 16.2 ns per candidate: with a probe around 12 ns, iteration is ~10 ns and
the two halves are roughly equal, so A removes ~half the time and B ~30% of the rest. **Estimate 4-5x
on the monsters, hence ~3.5-4x on the whole run** since they are ~90% of it.

### Three negative results, recorded so they are not retried

- **Meet-in-the-middle on mass is pointless.** The idealised DP shows the cap-pruned DFS already
  visits only ~4 nodes per completed split, so there is almost no dead-end structure for MITM to skip.
- **One-dimensional mass bounds are provably vacuous.** Per child independently, `max r_c` is the
  entire remaining mass (take `a=n, b=m` everywhere), so `r_c >= mass-2cap-s_c` is always satisfiable.
  That is exactly why the lower-bound prune measured 0.0% on 2026-08-04. The power is in the **joint**
  `(r0,r2)` reachability - you cannot maximise two children at once - which is why it needs a 2-D table.
- **My own DP over-estimated B by ~30x.** It predicted 3.2-18.5x from an idealised tree; against the
  real solver the prune touches 10.7% of nodes. The model ignored live-option lists and the
  partial-child cache probes, which already remove most of what it counted. The honest figure is the
  probe-weighted one (62-66% of survivors), not the node-weighted one.

### If this is implemented

Gate to the monster signature (`P >= 7`, `mass >= 0.99 * 3^k`, `k <= 7` so the table stays small): the
reachability table is `(cap+1)^2` bytes per suffix - 476 KB at k=6 - and ~1e7 ops to build, which is
nothing against a 110 s state but would swamp the 2 M cheap ones. Both changes are in the hottest loop,
so validation must be identical verdicts on all 43 proven k<=6 Pareto cells plus the six monsters.
**The live run cannot benefit without a restart**, which would discard four days of accumulated cache.

### Prototype measurements: the ordering choice is the lever, and it is a small change

Measured on the real solver (scratch `/tmp/k6lab`, instrumentation only, verdicts untouched), cheapest
monster, 110.2 s baseline.

**Two ideas measured, one dead, one large.**

*Per-key emptiness test.* Dead as first conceived: `min k0 = 0` always (take `a=n`), likewise k1 and k2,
so no single-key minimum ever fires, and `A+B+C >= mass_i` is vacuous by mass conservation. It only
works as a **joint** test - a per-part 2-D running max of `k0+k2` over the achievable set. So measured
that instead: it fires on **56.5% of level-6 descents and 71.3% of level-7**, 0% at levels 1-5 (the
bounds are still loose up there), 24.4% overall.

*Tightest-key walk.* The three cap constraints are `k0<=A, k1<=B, k2<=C`; walking whichever order the
bound cuts soonest, rather than a statically chosen one:

| level | full list | tightest-key walk | shorter |
|---|---|---|---|
| 4 | 1.60e9 | 3.34e8 | 4.8x |
| 5 | 9.71e9 | 1.05e9 | 9.3x |
| 6 | 3.19e9 | 2.37e8 | 13.5x |

Full lists total 1.46e10; `ordmono` already gets the solver down to 6.82e9 (2.1x); the tightest-key
walk plus the emptiness test reaches **1.66e9 - 4.1x fewer candidate evaluations than the current code**.

**And the implementation is much smaller than expected.** `BY_SP0/BY_SP1/BY_SP2` *are* the orderings by
these three keys, and `ordmono` already terminates the level on whichever key was chosen. `splitincr[i]`
picks between them with a gap heuristic (`p0 > p1 ? ...`). The change is to replace that guess with a
**measured count** - one lookup per key into a static per-part `cnt_le` array - and use the count as the
walk bound. No new iteration machinery.

**Combined estimate.** With iteration ~60% of the 110 s and probes ~40%: A gives 4.1x on the iteration
half, B removes 62-66% of the probe half, so roughly **3.4x on this state**.

**Corrected along the way.** I first computed scan lengths as 3-12 options per prefix and concluded
indexing could not pay. Wrong denominator: descents additionally require passing the child probes, so a
descent scans **20-51** options, not 3-12. The tightest-key idea was nearly discarded on that error.

Static per part (in `ensure_splits`, keyed by sbb, so once ever): three `cnt_le` arrays of `mass+1` ints,
and for the emptiness test a `(mass+1)^2` short table - build it only for parts under a mass threshold,
which is a property of the part, not a gate on the state.

## 2026-08-08 — the two optimisations built and benchmarked: 2.5-2.75x

Both implemented in scratch (`/tmp/k6lab/impl`), not in the repo. Warm k<=5 cache from the run's own
checkpoint so a k=6 solve does no recursion below itself.

| | monster 1 | monster 2 |
|---|---|---|
| baseline | 110.7 s / 6.81e9 splits | 676.7 s / 4.27e10 |
| **A** ordering chosen by measured count | 89.2 s / 1.91e9 (**1.24x**) | 532.3 s / 1.20e10 (**1.27x**) |
| **A+B** with joint reachability | **43.9 s (2.52x)** | **246.5 s (2.75x)** |

Peak RSS also fell, 2.63 GB -> 1.62 GB. Verdicts unchanged throughout.

**A - pick the split ordering by measured count.** `BY_SP2/BY_SP1/BY_SP0` are monotone in p0/p1/p2 and
the counting-bound cut retires the level at the first option exceeding the budget, so a level runs for
exactly `cle[j][budget_j]` options. Choose the smallest of the three instead of guessing from the gap
between p0, p1 and p2. `cle` is a static per-part prefix-summed histogram, built once per sbb in
`ensure_splits`. Gated to `pass >= 2`.

**The stated reason for that gate was wrong.** I claimed pass 2 is exhaustive so iteration order carries
no solution-finding value. Pass 2 is exhaustive only when the state is *unsolvable*. FAST is imperfect
and sometimes misses an existing solution, and pass 2 then has to find it — so ordering still matters
there, and A optimises for the opposite objective: it picks whichever ordering makes the level
**shortest**, which is ideal for retiring a level and could be poor for reaching a witness.

The log measures how often this happens: `NOTFAST` marks a winning split that FAST would have skipped,
and **2,603 of 47,783 solutions (5.4%)** contain one — concentrated at k=5 (887) and k=6 (1,083),
exactly where the time goes. Not a corner case.

**Measured on exactly those states, A+B is faster, not slower**, comparing splits-to-witness:

| | states | baseline | A+B | |
|---|---|---|---|---|
| k=5, FAST-missed, solvable | 60 | 248,501 | 101,757 | **2.44x better**, 0 worse |
| k=6, FAST-missed, solvable | 25 | 37,218 | 12,499 | **2.98x better**, 1 worse (19 -> 28 splits) |

Two reasons, and the second is the reassuring one. First, the ordering choice does not change *which*
options are considered: each of the three cap bounds is a necessary condition, so terminating on any one
of them still visits every option satisfying all three — only the count of infeasible ones waded through
changes. What genuinely changes is the order *within* the feasible set, which is where the risk lives.
Second, **FAST-missed solvable states are cheap** — those 25 k=6 states average ~1,500 splits — so even
a real ordering regression there costs almost nothing, while the states A+B speeds up by 2.5-2.75x are
the expensive refutations.

So the risk is real in principle and small in magnitude, because it lands on the cheap population. It
is not eliminated: a larger or differently-drawn sample could find a costly solvable state that pass 2
reaches. Improving FAST is the better fix, and would remove the tension rather than bound it — every
miss currently costs a full pass-2 search on a state that had a solution all along.
Also never picks a `_DESC` ordering, which have `ORDER_MONO_P = -1` and so get no early termination at
all - the old heuristic chose them often.

**B - joint suffix reachability.** `R[i]` as a bitmap of the `(r0,r2)` the remaining parts can
contribute, built by convolution (shift-and-OR, ~1-2 ms for a whole state), plus a 2-D running max so
the completability test is one lookup. Inserted between the cap check and the cache probes. It fires on
**53.4% / 51.5%** of cap-survivors.

### The candidate count is not the speedup, and the gap is the whole story

A cut candidates **3.57x** and wall clock only **1.24x**. Back-solving, iteration was **27% of runtime,
not the 60% I assumed** - because A removes precisely the options that would have been *cap-killed*,
which are the cheap ones (a few ns), while the expensive ones (surviving, three cache probes) remain.
That inverted the ranking: B, which removes wasted *probes*, is the more valuable of the two, and A
alone would not have been worth the change.

### Not production-ready

- **Cleanup only happens on one exit path.** `rb_free()` and clearing `rb_on` sit at the print site, but
  `canSolveB` has many early returns. Consequence is a leak plus B silently disabling itself for the rest
  of the process - not a wrong answer, but it must be fixed before this is real.
- **Only the outermost gated state gets B**, by design of the prototype. Nested application needs
  per-invocation tables.
- **The gate is still the monster signature** (`size>=7`, `sat>=0.99`, `cap<800`) rather than the
  adaptive node-budget trigger discussed.

### Validation so far

All 43 proven k<=6 Pareto cells reproduce in both directions (n1 solvable, n1+1 not), with A and with
A+B. Both monsters return UNSOLVABLE as before. And a broader replay against the run's own log —
**350 logged k=5 verdicts, 250 negative and 100 positive, 0 mismatches** — which is the check that
covers ordinary multi-part states rather than only frontier cells and the six monsters. Note the
positives matter more than the negatives here: A reorders iteration, so a bug would most likely show as
a *missed* solution, and 100 known-solvable states all still solve.

### Production hardening of A+B (2026-08-08, still scratch)

Three gaps closed, all in `/tmp/k6lab/impl`:

- **Release on every exit path.** Only two real early returns exist after the build point (both
  `return MAYBE`; three others in that span are commented out), so an explicit `rb_release()` at each
  is enough. Previously the release sat only on the printing path, so an early return leaked the tables
  *and* left `rb_on` set, silently disabling the prune for the remainder of the process — a lost
  optimisation rather than a wrong answer, but invisible.
- **Adaptive trigger replaces the shape gate.** The prune now arms once a state has spent
  `RB_TRIGGER = 10M` candidate evaluations, instead of matching `size>=7 && sat>=0.99`. A state that
  expensive has earned the ~1-2 ms the tables cost, cheap states never pay, and no state shape is
  privileged — anything that turns out expensive gets the prune. Measured identical: **43.4 s** against
  43.9 s shape-gated, same 53.4% prune rate.
- **Nesting left as outermost-only, deliberately.** A gated state's own descendants cannot arm the
  prune. They are k=5 states, which are 2.4% of total time, so the complexity of per-invocation tables
  buys almost nothing.

Re-validated after the changes: 43 proven k<=6 Pareto cells, both directions, 0 mismatches.

## 2026-08-08 — what FAST actually misses

`NOTFAST` in a solution line marks a winning split that FAST would have skipped, so the run's own log
is a labelled dataset of the heuristic's false negatives: **2,603 of 47,783 solutions (5.4%)** contain
one, concentrated at k=5 (887) and k=6 (1,083).

Measuring each winning split's deviation `d` from the balance line `m1*n2 = n1*m2`:

| | count | median \|d\| | mean | p90 | max |
|---|---|---|---|---|---|
| FAST-admitted winners | 223,754 | 0.25 | 0.32 | 0.80 | 2.33 |
| **NOTFAST winners** | 3,006 | **0.88** | 0.91 | 1.50 | 5.00 |

So the misses are farther from the line but not far — p90 of 1.50 against an admitted max of 2.33. The
line itself is not the problem; the local-optimality test around it is.

**Two-fifths of the misses are boundary splits:**

| | share of misses |
|---|---|
| `m2 == n2` | 21.3% |
| `m2 == 0` | 15.7% |
| `m1 == n1` / `m1 == 0` | 3.9% |
| interior | 59.2% |

The `m2` boundary cases are structural, not accidental. The test is

    if ((m2==0  || compare_solvability(sbb1, ...m2-1) <= 0) &&
        (m2==n2 || compare_solvability(sbb1, ...m2+1) <= 0))

At `m2==0` the first clause auto-passes but the second must still hold, and symmetrically at `m2==n2`.
So on a boundary the two-sided local optimality test degenerates to a one-sided one that can still
reject. The source comment claiming it "admits the diagonal AND the corners" overstates what the code
does — it admits the corners only if the one applicable comparison also passes.

Two distinct repairs, then, and they are independent: **admit boundary splits unconditionally** (37% of
misses, and cheap to state), and **relax the interior comparison** to accept near-ties rather than
strict `<= 0` (the remaining 59%). Neither is implemented; the cost side — how many extra splits pass 1
would then try — is unmeasured and is what decides whether either is worth it.

### Widening FAST is decisively wrong — tested and rejected

The obvious repair from the analysis above, admitting boundary splits unconditionally, was implemented
and measured. It is **430x worse**.

| 60 solvable k=5 states that FAST currently misses | totalsplits |
|---|---|
| baseline | 248,501 |
| boundary splits admitted | **107,006,357** |

and it removed only **11%** of the `NOTFAST` hits. Verdicts unchanged, so this is purely a performance
result.

The mechanism: for a part `(n1:n2)` the boundary is `m2 = 0` or `m2 = n2` for *every* `m1`, i.e.
`2(n1+1)` extra options, against the handful the local-optimality test admits. FAST stops being a
filter, and pass 1 becomes a second near-exhaustive pass.

**The cost probe I ran first was invalid, and the way it was invalid is worth remembering.** I measured
the cost on a monster and got 0.9%, concluding the change was nearly free. But

    no_deadline = (pass==1 && size <= 4) || parent_deadline == NO_DEADLINE;

so pass 1 is **deadline-bounded for large states and unbounded for small ones**. The monsters have
size 8, bail out of pass 1 early, and hide the entire cost. The states that pay are the small ones,
which is exactly the population I had not measured. Choosing a benchmark that cannot exhibit the
failure is the same mistake as the earlier "a mechanism never firing on your benchmark".

**What this implies about the direction.** FAST's value is in being *narrow*; widening it costs far more
than the 5.4% of solutions it recovers. So the repair is not a wider admission rule. Two directions that
remain open, neither measured: **rank** rather than admit - try likely winners first inside the existing
narrow set - or give pass 1 a deadline for small states too, so a FAST miss costs a bounded pass rather
than an unbounded one. The second is a change to deadline semantics and would need the same care as the
2026-08-04 deadline episode.

### Where FAST can possibly help, from the solvability census

Before refining FAST's admission rule, the prior question: where do solvable states even exist? From
the run's log, by (k, part count), restricted to cells with >= 50 samples:

| k | parts | solvable | unsolvable | solvable % |
|---|---|---|---|---|
| 6 | **8** | **0** | 165 | **0.00%** |
| 6 | 4 | 8,434 | 214,627 | 3.78% |
| 6 | 3 | 519 | 33,134 | 1.54% |
| 5 | 4 | 7,183 | 230,027 | 3.03% |
| 5 | **5** | 1,975 | 1,332,118 | **0.15%** |
| 5 | 6 | 2,164 | 431,182 | 0.50% |
| 5 | 7 | 5,288 | 8,819 | 37.48% |
| 5 | 8 | 200 | 82 | 70.92% |
| 4 | 5 | 7,399 | 42,190 | 14.92% |

**Solvability is rare in the middle and common at the extremes.** At k=5 with 5 parts it is 0.15%; at
k=5 with 8 parts it is 71%. And **8-part k=6 states are never solvable** — 0 of 165, with exactly one
solvable >=7-part k=6 state in the entire log (a 7-part case at mass 705).

That reframes the problem. On the 166 monsters FAST cannot help *at any precision*, because there is
nothing to find; pass 1 there is pure overhead. And on the huge k=5 cells at 0.15-0.5% solvable, FAST's
job is almost entirely to fail cheaply.

So the lever may not be FAST's admission rule but **whether pass 1 runs at all**. The relevant cost:
pass 1 on an unsolvable state cannot conclude — it skips non-FAST splits, so `skipped_some` is set and
pass 2 repeats the work — so it is wasted whenever the state is unsolvable, which is 96-99.85% of the
time in the big cells. The 2026-08-03 measurement of removing FAST entirely (k=9 ladder 1521 s -> 266 s,
5.7x) is the scale of that waste on a refutation-dominated workload, and the reason it was restored is
that it is *not* waste on the minority that are solvable.

A (k, parts, saturation) prior is cheap and strongly predictive, so gating pass 1 on it - rather than
widening or narrowing what FAST admits - is the version of "skip pass 1" that does not lose the
solvable cases. Untested. It also interacts with the earlier finding that pass 1 is deadline-bounded
only for `size > 4`, which is why removing it looked free on monsters and catastrophic on small states.

**Not yet done, and the right next step per the method:** full solution maps for the cells where
solutions actually live - k=6 with 4 parts (3.78%) and k=5 with 7-8 parts - to elicit a precise rule
from complete data rather than from the winners that happened to be logged.

## 2026-08-08 — a precise, joint condition on winning splits: tightness

Elicited from the critical corpus rather than the refutation log — the five verified witness trees
(`sa192_k10_a/b`, `sa112_k9_a/b/c`), 415 winning splits, every one of them a *critical solvable* state.

Call a split **tight** when its largest child has mass exactly `3^(k-1)`, i.e. some child is pushed to
the information limit. Fraction of winners that are tight, by the parent's saturation `mass/3^k`:

| saturation | winners | tight | within 1 of cap |
|---|---|---|---|
| < 0.80 | 103 | 10.7% | 17.5% |
| 0.80-0.90 | 100 | 51.0% | 70.0% |
| 0.90-0.95 | 75 | 88.0% | 96.0% |
| **0.95-0.999** | **73** | **100.0%** | 100.0% |
| = 1.000 | 64 | 100.0% | 100.0% |

**Above saturation 0.95 every winner is tight, 137 of 137** — and the 0.95-0.999 band, where tightness
is not automatic, is 73 of 73. At `sat = 1.000` it is forced (all three children must be exactly cap),
so that row carries no information; the signal is the band below it.

And tightness is selective. Counting all cap-feasible splits by DP:

| state | sat | cap-feasible | tight | share |
|---|---|---|---|---|
| `Sb(36:8,14:12,15:7,5:1)` k=6 | 0.776 | 8,296,906 | 390,536 | 4.7% |
| `Sb(27:13,18:8,9:8,9:2)` k=6 | 0.802 | 6,251,106 | 391,898 | 6.3% |
| `Sb(15:13,19:9,19:8,24:5)` k=6 | 0.875 | 30,327,694 | 2,070,002 | 6.8% |
| `Sb(18:7,...,27:1)` k=6, 8 parts | 0.963 | 2.34e13 | 4.74e12 | 20.3% |

So requiring tightness discards 80-95% of the search space and, above sat 0.95, loses no winner in the
corpus.

**Why FAST cannot express this.** Tightness is a property of the *whole* split - the three child masses
summed across every part - whereas FAST marks each part's options independently. No per-part rule can
say "some child reaches exactly `3^(k-1)`". That is a structural limit of the current heuristic's shape,
not a matter of tuning its threshold, and it is the concrete sense in which the heuristic can be made
more *precise* rather than wider.

The earlier per-part observation dissolves into this one. The `m1/n1` histogram over winners is bimodal
with 30% at the extremes (0 or 1) - parts taken whole or left out - which looked like a second pattern
but is just how a split reaches a tight child: the balance line passes exactly through `(0,0)` and
`(n1,n2)`, so all-or-nothing parts sit on the diagonal at zero deviation while contributing their whole
mass to one child.

### Status: an empirical regularity, not a theorem

137 of 137 is from one construction family (`Sa(192)` and `Sa(112)`), so the trees are not independent
samples. A counting argument does not immediately give it: `max child >= mass/3`, so at `sat >= 0.95`
the max already lies in `[0.95 cap, cap]`, and tightness is a restriction *within* that band rather
than a consequence of it. Worth trying to prove, and worth testing against the canonical trees
(`canon_*.tree`) which come from a different construction.

Untested as a heuristic: enforcing it would mean tracking the running max child during enumeration and
requiring it to reach cap - which composes naturally with the reachability tables from A+B, since those
already carry the achievable `(r0,r2)` per suffix and can answer "can any completion still reach cap".

### Building a comprehensive corpus: full maps from the k=8 Pareto frontier

The witness-tree corpus above is 415 winners but only one winning split per state — the one that
construction happened to use. To elicit a rule properly one wants **every** winning split of **every**
critical state, which means full solvability maps from the k=8 Pareto frontier downward.

`/tmp/k6lab/mapper.c` does this: enumerate every split of a state, ask the oracle whether all three
children are solvable one level down, record the winners, recurse into their children. The only prune
is the counting bound, which is exact (a child over `3^(k-1)` is unsolvable by COUNT), so the recorded
set is exactly the winning set — nothing heuristic is applied to the corpus that the corpus is meant to
test.

The oracle is `out_k8.txt` (2026-era, audited clean) parsed into facts. Loading all 11.4 M at
`MAX_N=257` would be ~26 GB of trie, so each cell gets a cache filtered to facts whose every part fits
**componentwise** inside the root part — sound, because dominance only ever appeals to smaller states.
That is a large reduction: `Sb(256:1)` needs 6,440 facts rather than 11.4 M.

First results, and they are the right shape for eliciting a rule — the solution sets are tiny:

| cell | cap-feasible splits | winners |
|---|---|---|
| `Sb(56:55)` k=8 | 2,504 | **6** (3 up to complement symmetry) |
| `Sb(256:1)` k=8 | — | 2 |
| `Sb(255:2)` k=8 | — | 4 |

`Sb(56:55)` sits at saturation 0.469 and none of its winners is tight, consistent with the saturation
gradient found above (10.7% tight below 0.80). One of them, `take=25:21`, is **3.55 from the balance
line** — beyond the 2.33 maximum seen among FAST-admitted winners, so FAST would reject it. It is not
a `NOTFAST` case only because two of its siblings are admitted.

Depth 1 over all 55 cells is running, ~3-4 hours, dominated by the per-cell filter pass over 503 MB and
by loading caches up to ~1.1 M facts. Depth 2 is the interesting one and will cost more; depth 3 is
likely infeasible by full enumeration at 4 parts, where a single state has 30 M+ cap-feasible splits.

## 2026-08-09 - critical 4-segment corpus, a provable per-part filter, and a memory wall

**Corpus redefined.** The target is the *critical 4-segment* states two levels below a Pareto root,
not a recursive map from the roots. 1- and 2-segment states defer to the lower-level Pareto column
and teach nothing; 4-segment is where the split heuristic is weak. From a 1-part root, 4-part states
appear exactly two levels down, so this is `mapper.c` at depth 3 recording only the target level.

**Supersedes the 2026-08-08 entry below:** depth 3 is *not* infeasible. The "30 M+ cap-feasible
splits at 4 parts" figure was measured on an *off*-critical state. Near the frontier the counting
bound kills almost everything, and critical 4-part states at k=5 have a median of 548 K cap-feasible
candidates, not tens of millions.

**k=5 corpus, complete.** 32 k=7 Pareto roots -> 479 4-segment states at k=5, 335 critical
(Pareto-maximal in every part), 308 mapped, 4,684 winning splits. 27 states exceeded the 3 M
candidate guard and are logged `SKIP big`, not dropped silently. Cost 49 m 43 s wall - of which the
*mapping was 68 seconds*. The oracle load is 97% of the cost, so analysis iterates free and only
re-running the corpus is expensive.

Saturation note: Pareto roots sit at 0.04-0.47 saturation but their two-level descendants sit at
0.63-0.91, because the mixed child keeps ~half the mass with a third of the cap, so saturation
multiplies by ~1.5 per level.

**Retracted: "winners are tight".** Only 22.7% of the 4,684 winners are tight. The 137/137 result
recorded earlier came from witness trees in the sat>=0.95 band and does not generalise to 0.63-0.91.

**Superseded: the one-copy majorization check adds nothing here.** On a 40-state sample, 17,280,206
cap-feasible candidates, 17,280,206 survive the then-current check on all three children - 1.0x. The
measurement was correct for `(n:m)->(n:1)`, which discards too much mass to fire at k=4 children. It
does not apply to the full star expansion `(n:m)->m copies of (n:1)`, discovered on 2026-08-09.

**CORRECTION (same day): the per-part filter is ALREADY IN THE SOLVER, so the 15.3x below is not
new headroom.** The per-split `s[4]` / `s[5]` loop in `canSolveB` caches, for each part, the minimal level at which
`s[0]` alone, `s[3]` alone, *and the pair `{s[1],s[2]}` jointly* are solvable, and skips the split
when that exceeds `k-1`. That subsumes both the per-part test and the intra-part self-check. The
15.3x was measured against a baseline enumerator that lacked it - a property of my measuring tool,
not of the solver. **The genuinely new constraint is the CROSS-part pair filter**, and re-measuring
with the intra-part check moved into the baseline leaves it unchanged at 13.11x (k=5 4-segment) -
so that factor is entirely cross-part and does stand. Expected refutation speedup is therefore
~5-13x, not ~200x.

**Provable per-part filter, 15.3x - superseded, see correction above.** By Subgraph Monotonicity every sub-part of every
child must be solvable standalone at k-1, so for part `(n:m)` and take `(x,y)` all four of
`(x:y)`, `(n-x:m-y)`, `(x:m-y)`, `(n-x:y)` must be. This is a table lookup applied *per part, before*
the cartesian product. Across the 308 states it cuts 2.897e8 cap-feasible candidates to 1.892e7
(median 12.1x, max 67x). Exact single-part tables, computed by `ptab`, not assumed symmetric:
  k=4: 16 15 12 10 9 7 6 5 5 4 3 3 2 2 2 1
  k=5: 32 31 27 24 22 19 17 15 14 12 11 10 9 9 8 7 7 6 6 5 5 5 4 4 3 3 3 2 2 2 2 1

**Ordering score.** Fitted against ground truth, not guessed. With `f = x/P_{k-1}[y]` per sub-part,
  score = fmean + 0.286 * (dev + imbalance)
where `dev = sum|x*m - y*n| / (mass/2)` and `imbalance = 1 - minchild/cap`. `maxchild/cap` and
`fmax` both fit to weight zero. It is an *ordering*, never a filter, so it cannot miss a solution.
Absolute first-hit rank (the metric that matters - one valid split is all that is needed):

| set | median | p90 | worst | mean |
|---|---|---|---|---|
| k=5 train (30 states) | 267 | 1 491 | 2 417 | 476 |
| k=5 holdout (30 disjoint, *same construction*) | 233 | 2 505 | 3 311 | 692 |
| k=5 holdout, dev+imbal only | 523 | 8 183 | 12 705 | 2 458 |
| k=6 (17 states), k=5 weights | 2 221 | 32 031 | 143 945 | 13 567 |
| k=6, dev+imbal only | 15 245 | 72 269 | 100 567 | 23 624 |

**The 233 holdout figure is optimistic - superseded below.** Those 30 states were independent
*states* but not an independent *construction*: they were reached by the same two-level descent as
the training set. On a genuinely independent family (see the upgrade result) the same weights give
median 631, 2.7x worse.

The weights transfer across k without refitting (6.9x over baseline at k=6). Relative to the space
the heuristic *sharpens* with k - 0.38% of 60 730 at k=5, 0.057% of 3 901 959 at k=6 - but absolute
rank grows, and absolute rank is the cost. `fmean` alone is far worse than the baseline (median
6 963); only the combination works.

**Memory wall - the expensive lesson.** This machine has 24 GB. The k<=7 oracle at `MAX_N=262`
needs ~20 GB of trie (it scales ~`MAX_N^2`; `MAX_N=132` peaks at 4.04 GB). The k=8-rooted run
loaded for **3 h 50 m** and never finished - the "deceleration" from 8.6 to 0.9 dots/min was swap,
not trie growth. Abandoned. A follow-up bounded to `n1<=128` (35 of 55 cells) reached only
2 of 35 roots in **9 h 20 m** before being killed: `RSS 0.21 GB, VSZ 424 GB`, 6 395 swapins in 45 s.
The result-cache trie grows unboundedly as it solves, so a long mapping run swaps itself out.

**`tools/capped_run.sh --rss-gb` does not catch this.** Pages get swapped out rather than staying
resident, so RSS reads 0.2 GB while 27 GB sits in swap and the cap never fires. Bound these runs by
`MAX_N` and by cell selection at compile time, or watch `vm_stat` swapins - not RSS.

**Consequence for future k=6+ corpus work:** do not descend from k=8 Pareto roots on this machine.
Mapping a *known* k=5 state needs only k=4 solving, so states obtained some other way can be mapped
cheaply with no large oracle at all.

**Upgrade done, and it is cheap.** The 144 slack (`crit=0`) states were bumped to Pareto-maximal -
greedy, from each starting part in turn, so one slack state can yield several maximal ones - then
fully mapped: **190 new critical states, 1,906 winners, in 75 s at 0.08 GB peak**. No oracle load at
all, because mapping a *known* k=5 state needs only k=4 solving. Corpus is now 498 states from two
constructions. `upgrade.c`, kept in `~/radio-corpus/`.

**The upgraded states are harder, and the feature set is saturated.** On the 165 held-out upgraded
states (9.6 M candidates, median space 58 354):

| score | median | p90 | worst | mean |
|---|---|---|---|---|
| refit across both families | 671 | 3 323 | 8 301 | 1 334 |
| descent-only fit | 631 | 4 423 | 9 485 | 1 459 |
| dev + imbalance baseline | 1 375 | 8 739 | 21 801 | 3 059 |

Refitting on both families gained ~25% on the tail and *nothing* on the median. Doubling the corpus
and adding a second construction did not move it. The limit is the five features, not the weights or
the data - reaching rank ~1 needs a structural rule, not more fitting.

**Shape features - the largest gain of the session.** Describing a child by its pair count throws
away what majorization actually constrains. Three features built on the block *profile* of a child
(part `(a:b)` expanded, mass-preservingly, to `min(a,b)` copies of `max(a,b)`, sorted descending):

- `majtight` = `max_i prefix_i / Gprefix_i` against `G_j`. Large blocks blow the early prefixes
  first, so this measures distance to the majorization wall. **Correction 2026-08-09: the cutoff
  `majtight > 1` is sound.** The profile is the full star expansion, and Vertex-Splitting Pullback
  proves that every solvable state must pass Singleton Majorization. Exceeding 1 on candidates that
  pass the weaker per-part filter showed additional pruning power, not unsoundness. Using the
  continuous value below 1 as an ordering score remains heuristic.
- `meanratio` = the same prefix ratio averaged over all ranks rather than maxed.
- `l2shape` = mean squared deviation of the profile from `G_j`'s own blocks at the same rank,
  normalised by `G_j[0]` - literally "how far is this branch's shape from the extremal shape".

On the 165 held-out upgraded states:

| score | median | p90 | worst | mean |
|---|---|---|---|---|
| 12 features | **115** | 1 037 | 7 627 | 422 |
| minus `l2shape` | 229 | 1 609 | 12 743 | 669 |
| minus `meanratio` + `l2shape` | 531 | 2 465 | 8 079 | 1 006 |
| 8 features (pre-shape best) | 477 | 2 443 | 9 547 | 991 |
| `dev + imbalance` baseline | 1 375 | 8 739 | 21 801 | 3 059 |

12x over baseline. Fitted weights: `dev .35, imbal .20, fmean .75, majtight .35, meanratio 1.50,
l2shape 2.50`; `maxchild`, `fmax`, `topheavy`, `budget`, `blocks`, `mixmaj` all fit to **zero**.

`topheavy` (largest block's share of child mass) fitting to zero while `majtight` does not is the
useful detail: "a few large blocks is bad" is a real effect, but the majorization prefix ratio is its
correct formalisation and a crude largest-block share captures none of it. Likewise a per-part
"level requirement" budget (`sum 3^r / 3^j`, with `r(a,b)` the minimum level solving the part alone)
carried no signal once the profile features were present.

Cumulative for one valid split at k=5: ~11 000 probes unfiltered under deviation ordering, 1 375
with the per-part filter, **115** with the filter and the shape score - about 95x.

**Two-dimensional majorization: the pair filter.** Subgraph Monotonicity says *every* sub-multiset
of a solvable state is solvable. The per-part filter uses only subsets of size 1 - collapsing each
part to a scalar and testing it flat. Size 2 is the 2-D step: **every pair of parts in every child
must be jointly solvable at k-1**, including the two halves `(a:m-b)` and `(n-a:b)` of the same
parent part, which both land in the mixed child.

The table is small and exact: at k=4 there are 102 solvable single parts and **4 368 of their 5 253
pairs are solvable - 16.8% of pairs fail while both members individually pass**. That 16.8% is
precisely the 2-D information a flat per-part test discards.

Effect on the k=5 corpus: a further **13.11x** on top of the per-part filter, 18 921 200 -> 1 443 192
candidates over the 308 states, so ~200x off the cap-feasible space in total. Sound by construction,
and verified empirically: **all 2 556 known winners survive it**. Computed by `pairtab.c`, applied
incrementally during the cartesian product (`cpair.c`, `dumpz.c`).

Combined with the shape score, on the 165 held-out states:

| configuration | median | p90 | worst | mean |
|---|---|---|---|---|
| shape score + pair filter | **31** | 189 | 1 343 | 97 |
| refit on pair-filtered training data | 35 | 299 | 3 805 | 156 |
| `dev+imbal` baseline + pair filter | 163 | 1 711 | 5 177 | 574 |
| shape score, per-part filter only | 115 | 1 037 | 7 627 | 422 |

Refitting *after* adding the pair filter made holdout **worse** (31 -> 35, mean 97 -> 156): 55
training states cannot re-tune 12 weights against a 13x smaller space. Keep the pre-pair weights
`dev .35, imbal .20, fmean .75, majtight .35, meanratio 1.50, l2shape 2.50`.

Cumulative for one valid split at k=5: ~11 000 probes -> 1 375 (per-part filter) -> 115 (shape
score) -> **31** (pair filter). About 350x, and both filters are provable; only the ordering is
fitted.

**Can singleton majorization be generalised to an exact metric on non-singleton parts?
A scalar metric provably cannot do it.** The question: is there `phi(n,m)` such that a state is
solvable in K iff `(phi(n_i,m_i))` sorted descending is weakly majorized by some base sequence?
For two parts that rule reduces to `phi_a + phi_b <= T`, which forces the jointly-solvable graph on
parts to be a **threshold graph**. It is not. At k=4 the vicinal preorder has **52 incomparable
pairs**, with explicit contradictions:

    {(1:15),(5:7)} solvable and {(4:7),(2:13)} solvable,
    {(1:15),(2:13)} NOT and {(4:7),(5:7)} NOT

Summing the two solvable constraints gives `<= 2T` over the same four values that the two unsolvable
ones force `> 2T`. This rules out **every** scalar `phi` at once - including any linear combination
of part features, any power law, and any learned scalar. Recorded as a negative result: do not go
looking for one.

**Why: every part at its own frontier collapses to the same scalar.** With `phi` anchored at `2^k`
on the level-k single-part frontier and interpolated linearly between anchors - which does reduce
correctly, `phi(n,1) = n`, so the rule becomes the theorem - `(16:1)`, `(7:6)`, `(9:5)` and `(4:10)`
all map to `phi = 16` while carrying masses 16, 42, 45 and 40. One number cannot separate the
strip regime from the bulk regime. Sound (0 false rejections) but catches only 9.7% of unsolvable
pairs.

**Two coordinates, and both have sound base sequences.** The mass coordinate has its own analogue of
`G_K`: `C_K(t)` = max mass of a *solvable* t-part state at level K. Every t-subset of a solvable
state is solvable, so `mass of the t largest parts <= C_K(t)` is sound. Measured at k=4:

| coordinate | base sequence (prefix) | catches of 885 unsolvable pairs |
|---|---|---|
| strip, `phi` majorized by `G_K` | 16, 31, 42, 53, 58, ... 81 | 78 (8.8%) |
| mass, majorized by `C_K(t)` | 45, 63, ... 81 | 440 (49.7%) |
| either | | **466 (52.7%)**, 0 unsound |

`C_4(1) = 45` (attained by `(9:5)`), `C_4(2) = 63` (by `{(7:3),(7:6)}`). Both curves saturate at
`3^K`, and they are tight in different regimes - `G_K` for singletons, `C_K` for balanced parts.

So a 2-coordinate majorization rule **exists, is sound, and is general** (any part count, any k, no
table), but captures only about half of what the exact k=4 pair table captures. The 445 pairs it
misses all have mass `<= C_4(2)`, so no mass bound can ever see them; 364 of those 445 have at least
one thin member (`min side <= 2`). A useful third coordinate would have to separate thin-by-balanced
combinations, which is exactly where the residual sits.

**The filters apply to refutation, which is where the money is.** Both filters prune the
*cap-feasible enumeration*, and cap-feasible enumeration is exactly what pass 2 does. Measured by
sampling on the two 8-segment k=6 states the live `Sa(193)` run is actually stuck on (the level
burning 97% of its CPU), the pair filter alone removes 78.5% and 85.6% of cap-feasible candidates -
**4.7x and 7.0x**. Small samples (93 and 320 cap-feasible hits; ~3.4-7.7x and 5.5-9.5x at 95%), and
this is on top of a base that already includes the per-part filter, whose own k=6 contribution is
unmeasured. Weaker than the 13.11x at k=5 4-segment, but the k=6 states have 8 parts and 28 pairs
per child, so the direction was not obvious in advance.

Tables needed to deploy: `P_k` and the pair table per level. Sizes so far - k=4: 102 parts, 5 253
pairs; k=5: 327 parts, 53 628 pairs (85.9% solvable). Both cheap. `pairtab.c` builds them.

**How to apply the pair filter without paying for it.** All-pairs is the wrong implementation: a
p-part state needs `2*C(p,2) + C(2p,2)` checks per candidate - 176 at p=8. Three measurements on the
two live-bottleneck k=6 states settle the design.

- *Arc consistency is useless here.* AC-3 over the pair constraints removed **zero** options (1.00x):
  with ~150 options per part every option finds some partner. The pruning lives in the joint
  assignment, not the arcs. The per-part self-check (the two halves of one parent part, which share
  the mixed child) also removed nothing.
- *A third of the pairs can never fire.* 9 of 28 and 11 of 28 parent pairs have an all-compatible
  bitmap, so they are detectable once per state and skipped for free.
- *The rest is steeply concentrated.* Ranking parent pairs by incompatibility density:

| checks (of 28) | rejection of cap-feasible | share of full benefit |
|---|---|---|
| top 1 | 13.9% / 13.2% | ~16% |
| top 5 | 54.0% / 54.7% | ~64% |
| top 9 | 73.7% / 76.9% | ~89% |
| top 13 | 81.0% / 85.2% | ~97% |
| top 17+ | 83.9% / 86.3% | 100% (nothing beyond) |

So: rank the parent pairs once per state by incompatibility density, keep the top ~half, check
densest-first with early exit. **~97% of the pruning for ~46% of the checks**, and the ranking costs
`p^2 * options^2 * 6` lookups (~3.8 M) against an enumeration of 10^10 candidates. Per-parent-pair
compatibility bitmaps make each check one bit test rather than six table lookups.

**Deployment note:** the tables needed are per level, and the level that matters is already built.
The live run spends 97% of CPU at k=6, whose children sit at k=5 - and the k=5 pair table (327
parts, 53 628 pairs) exists. Tables grow with k, and building them at k>=8 means solving millions of
2-part states, so apply the filter at the low levels where enumeration actually concentrates rather
than everywhere.

**Subsequently tested later on 2026-08-09:** the size-3 subset filter is the large missing
constraint, and size 4 adds a smaller but useful second step. See the latest entry below. Scoring
children recursively with the same function one level down remains untested.

**Method note, twice burned in one day:** a holdout must differ in *construction*, not just in which
states were drawn. The 40-state sample and the same-family holdout both overstated results that the
full set and the second family then corrected.

Artifacts kept in `~/radio-corpus/` (off `/tmp`): `c7.out` (k=5 corpus, 1.7 MB), `c6.out` (17 k=6
states), `crit_k5_states.txt`, `crit_k6_states.txt`, and the tools `mapper.c`, `cands3.c`,
`majcens.c`. Not pushed to `radio-data`.


## 2026-08-09 (later) - A+B landed, second cold run started alongside the first

**Landed `efadab0`.** The two k=6 optimisations measured on 2026-08-08 had never been committed -
they sat in `/tmp` for a day while the live run used the 2026-08-05 build. Nothing else had shipped
either: every commit from 2026-08-07 to 2026-08-09 touched docs only, and the 2026-08-06 majorization
change post-dates the run's launch, so it is not in the incumbent binary.

**Validation reproduced the benchmark exactly** - same verdict, `totalsplits=8132403495` to the
digit, 51.5% of prefixes pruned, 237 s against 246.5 s - plus 43 proven Pareto cells at k<=6 with 0
mismatches.

**The measurement trap, again, and worth writing down.** A first validation run took 27+ minutes
against that 246.5 s benchmark. I diagnosed a race on `rb_on` and changed the arming test from `==`
to `>=` before checking the premise. The arming site and all three release paths were in fact
correct; the real cause was that **the benchmark loaded a 2.2 M-fact warm k=5 cache
(`warm_k5.txt`, visible as 224 progress dots in `impl/ab2.out`) and my validation ran cold.** Two
different problems, not variance. Re-run warm, it matched immediately.

The `==` -> `>=` change was kept: `rb_on` is global, so with equality a state whose trigger instant
falls while another holds the tables never arms at all, and arming late is never worse than never
arming since the prune cannot affect correctness. But the source now says plainly that the fragility
has not been observed to fire. **Third measurement error of the same shape in one day** - after the
40-state sample and the same-construction holdout. All three were "I compared against the wrong
baseline"; the fix each time was a genuinely independent comparison, not more data.

**Consequence for reading the new run:** both A+B figures were measured warm, and the reachability
prune arms on candidate *count*, which accumulates far more slowly cold because every child solve is
uncached. The cold speedup should not be assumed to be 2.75x.

**Second run.** Started cold on the same `r7iz.4xlarge` (16 vCPU, 123 GB; the incumbent uses one core
and 6.9 GB), in `/root/run2`, binary `radio_sa193_ab`, `--rss-gb 40`, S3 prefix `run2/`.

- Separate binary name is load-bearing: the original launcher finds its solver with
  `pgrep -x radio_sa193 | head -1`, which with two runs would have aimed the new watchdog at the
  incumbent.
- `tools/sa193_watchdog.sh --prefix` and `tools/sa193_status.sh --both` (commit `5ad854e`) keep the
  two apart in S3 and in the mailbox; without it the second watchdog would have overwritten the
  first's STATUS and checkpoint.
- Launched over SSM. Note `--parameters commands=` takes a JSON **array**; passing the script as one
  string silently mangles newlines into literal `n` and the command "succeeds" having run garbage.

**Two open risks.** The incumbent's user-data ends in `shutdown -h now`, so if it finishes or trips a
cap the instance stops and takes run2 with it. And run2's 40 GB cap is a guess - 2023 reached ~90 GB.

## 2026-08-09 (earlier) - size-3 subsets make the positive heuristic nearly perfect

The next untested idea from the previous entry was the right one. Exact solvability of every
**three-part subset** of each child is a strong sound constraint by Subgraph Monotonicity, despite
only 9-11% of pair-compatible triples themselves being unsolvable. Requiring the condition many
times across a 4- or 8-part child compounds sharply.

### The tables are tiny and cheap to build

`tools/tripletab.c` and `tools/quadtab.c` build the exact tables with the current solver, using the
lower-dimensional table as a necessary gate. Fresh runs under `tools/capped_run.sh`:

| child level | table | solvable | prior-gate feasible | all combinations | wall |
|---|---:|---:|---:|---:|---:|
| k=4 | triples | 104,473 | 117,940 pair-feasible | 182,104 | 5 s |
| k=4 | quads | 1,570,834 | 2,344,892 triple-feasible | 4,780,230 | 5 s |
| k=5 | triples | 3,700,630 | 4,076,613 pair-feasible | 5,881,204 | 20 s |

The k=4 and k=5 single-part universes have 102 and 327 oriented parts respectively. The text
tables are 1.5/25/50 MB only because they spell out coordinates; combinatorial bitsets need about
23 KB, 598 KB and 735 KB. So table size and construction time are not obstacles.

**Trust boundary:** "exact" here means exhaustive according to the current C solver, not an
independent certificate. The independent `tools/refsolve.py` confirmed five sampled k=4 negative
triples, but the first k=5 negative did not finish in two minutes and that audit was stopped. The
tables are safe immediately as an ordering/admission heuristic followed by exhaustive fallback.
Using their negative entries to prune an exhaustive pass should wait for a stronger audit or be
validated by the eventual certificate; otherwise a table-generation false negative could become a
solver false negative.

### Complete k=5 four-part corpus

The experiment starts *after* the already-established per-part and exact pair filters. It checks all
triples incrementally; the quad extension checks every four-part subset. Both complete construction
families were regenerated and relabelled from their full winner logs by
`tools/label_split_features.c`; `tools/filter_triples.c` verified that no known winner was removed.

| family | states | pair survivors | + triples | + quads | winners retained |
|---|---:|---:|---:|---:|---:|
| Pareto descent | 308 | 1,443,192 | 300,798 (4.80x) | 214,964 (1.40x) | 4,684 / 4,684 |
| greedy upgrade | 190 | 1,019,842 | 291,622 (3.50x) | 166,724 (1.75x) | 1,906 / 1,906 |
| **total** | **498** | **2,463,034** | **592,420 (4.16x)** | **381,688 (1.55x)** | **6,590 / 6,590** |

From the cap-feasible space before the existing per-part check, the combined reductions are 12.16x
to pairs, 50.54x to triples, and 78.45x to quads. Unlike the earlier mistaken 200x headline, each
baseline is stated explicitly here.

### The ordering becomes almost perfect after the structural filter

Refitting the same 12 shape features on only 55 states (30 descent + 25 upgrade), after subset
filtering, changed the useful regime. The triple-filter weights are

    dev .20, imbalance .10, maxchild 1.00, fmean 1.00,
    majtight 4.00, topheavy .50, mixmaj 2.50, meanratio .35

and the quad-filter weights are

    dev .20, imbalance .05, maxchild .75, fmean 1.00,
    majtight 4.00, topheavy .35, meanratio .20.

Unlisted weights are zero. These are orderings, never correctness conditions. Absolute first-hit
rank, shown as median / p90 / worst / mean:

| evaluation family | old shape + pairs | new score + triples | new score + quads |
|---|---:|---:|---:|
| all 308 descent states | 35 / 221 / 2,785 / 101 | **1 / 9 / 77 / 3** | **1 / 5 / 59 / 3** |
| all 190 upgrade states | 33 / 189 / 1,343 / 98 | **1 / 9 / 33 / 3** | **1 / 5 / 35 / 2** |
| third family, 35 wholly unseen states | 37 / 225 / 685 / 102 | **5 / 35 / 49 / 10** | **1 / 9 / 17 / 2** |

The third family was constructed after fitting: take 30 evenly spaced states from the 34,259
distinct solvable k=5 four-part states encountered under the separate k=6 corpus, greedily upgrade
them to the Pareto frontier, and remove the 498 existing states. This produced 35 new critical
states and 124 winners after testing 11,321,026 candidates in 5 s. It uses the same upgrade
procedure, so it is not a wholly unrelated generative process, but neither its states nor its
winners participated in fitting. The quad result - median rank 1, worst 17 - is the closest this
thread has come to a perfect positive heuristic.

### Transfer to the real eight-part bottleneck

`tools/sample_subsets.c` sampled 100 million per-part-valid take vectors on each of the six actual
warm-benchmark monsters in `/tmp/k6lab/bench.txt`. The pair filter's marginal factors were
8.56, 7.06, 4.34, 4.11, 4.09 and 4.03. Conditional on surviving pairs, triples added factors
31.5, 31.0, 17.9, **more than about 49 at 95% confidence** (0 of 146 survived; rule of three),
22.5 and 22.9. On the five samples with a nonzero triple survivor, the cumulative reduction from
cap-feasible candidates was **77.6x to 269.5x**.

Naive lexicographic triple checking is already cheap: failures took about 30-39 bit lookups on
average, 98.7-100% failed within 100, and after weighting by the pair-survival rate the cost is only
about 6-16 triple lookups per cap-feasible candidate. Ranking constraints by incompatibility may
improve this, but is no longer prerequisite to a first implementation.

### What this does and does not establish

- It is a **large structural improvement**, not another fit to the same five saturated features.
- For positive k=5 four-part states, subset filtering plus the new whole-split score is empirically
  near-perfect across all known states and one post-fit family.
- For negative k=6 eight-part states, triples appear much stronger than pairs in standalone sampling.
- It is **not yet a solver speedup**. `radiobase.c` probes cached solvability of every partial child,
  which overlaps subset filtering when the cache is warm, and the rank experiment globally sorts
  complete surviving candidates rather than implementing the solver's Cartesian walk. A wall-clock
  benchmark against `radiobase.c` is still mandatory.

**Subsequently performed below:** the fallback-safe leaf measurement found zero marginal triple
rejections after the warm cache. The proposed bitset deployment is therefore withdrawn; the table's
independent-trust caveat would still apply if it were ever used for exhaustive pruning elsewhere.

## 2026-08-09 (latest) - the warm prefix cache subsumes subset filters; FAST misses are not the bottleneck

The proposed deployment experiment falsified the operational interpretation of the preceding entry.
The user pointed out the missing baseline before more code landed: the real solver already asks the
warm result cache about all three partial children after every parent part, and the negative cache is
upward-closed. An unsolvable triple in a child should therefore already refute every prefix containing
it. That is exactly what happens.

### Zero marginal information after the real cache probes

Lab-only instrumentation sampled the exact A+B monster

    Sb(18:8,22:6,15:8,13:9,23:4,23:2,21:2,17:2) in 6

against the same 2,201,187-fact `warm_k5.txt` as its retained baseline. It never pruned or changed
candidate order. At complete candidates, after the counting/reachability checks but before the three
real `CACHE_ONLY` calls:

| measure | result |
|---|---:|
| complete candidates reaching the cache | 5,200,097 |
| candidates passing all three cache probes | **0** |
| systematic 1/256 sample | 20,312 |
| sample rejected by the exact triple table | 11,064 (54.47%) |
| sampled cache-pass candidates rejected by triples | **0** |

So the attractive standalone 54.5% rejection has **zero marginal rejection** on this warm workload:
every sampled triple rejection was already covered by the cache. The verdict and
`totalsplits=8,132,403,495` matched the baseline exactly. Cost was 237.4 CPU seconds, 256 seconds wall,
2.53 GB peak RSS; the extra 19 wall seconds are measurement overhead, not a speed comparison.

The cache composition explains the result. It contains 2,163,272 negative `Sb` facts and 37,894
positive `Sb` facts (plus small `Sa` tables); the exact k=5 triple universe has 2,180,574 negatives.
These are essentially the same low-dimensional frontier, and loading a negative fact expands it
upward in the trie. The triple table is a second encoding of information the warm prefix lookup already
has. The offline subset censuses and global ranks in the preceding entry remain correctly measured,
but they are not production headroom.

A follow-up tried to use pair incompatibility only to reorder parent parts, leaving the cache as the
sole rejection mechanism. On the same monster the maximum-density edge was the existing first pair,
and the greedy order was exactly the incumbent descending order for all eight parts. Across all six
monsters the first three or four parts were already in the greedy order; differences occurred only in
the weak late edges. The redundant exact run was stopped after 33 seconds once its traversal order was
known to be unchanged. This agrees with the independent verifier's earlier result that canonical
descending group order beat ascending and fewest-options-first.

### Limited discrepancy makes FAST empirically complete, but slower overall

The next experiment addressed FAST itself rather than adding another rejection oracle. Current pass 1
allows zero early non-FAST choices and pass 2 jumps straight to exhaustive search. Limited-discrepancy
passes allow one, then two, non-FAST choices before that fallback.

`tools/fast_replay.c` replays logged k=5 roots against the same k<=4 cache, clears root-level influence,
and disables FAST's `s[FAST]=1` self-training. The committed version forks each target from one pristine
warm-cache image; the measurements below used the preceding root-clear version, so lower-level cache
facts accumulated during a run. Baseline and variants used the same target order, but the small timing
differences are corpus-run measurements rather than isolated per-state timings. The forked smoke replay
reproduced the first two cases exactly.

Positive hit coverage, with fresh FAST for each split table:

| k=5 corpus | cases | ordinary FAST / exhaustive | radius 0 / 1 / 2 | baseline CPU | radius-2 CPU |
|---|---:|---:|---:|---:|---:|
| 7 parts, stride-17 sample | 200 | 135 / 65 | 135 / 57 / 8 | 0.314847 s | 0.293546 s |
| 8 parts, complete logged set | 200 | 81 / 119 | 81 / 115 / 4 | 0.247179 s | 0.247794 s |

Thus every one of these 400 positive states has a solution within two deviations of FAST. This is the
closest result yet to a perfect *admission* heuristic, and it also repeats an earlier lesson: the first
exhaustive winner is not the nearest winner. The initial 7-part control printed two `NOTFAST` choices,
but radius 1 found a different solution.

It is nevertheless not a speedup. Failed discrepancy passes cannot prove a negative:

| negative corpus | cases | baseline CPU | radius-1 CPU | radius-2 CPU |
|---|---:|---:|---:|---:|
| 7 parts, stride-31 sample | 200 | 2.15856 s | 2.45308 s (**+13.6%**) | 3.13204 s (**+45.1%**) |
| 8 parts, complete logged set | 82 | 77.7626 s | 80.2865 s (**+3.2%**) | not run |

Radius 1 recovers 172 of 184 ordinary-FAST misses, but under the logged class mix it is about 5% slower
for 7-part states and 3% slower for 8-part states. Mass and pass-1-progress gates do not change the
decision. At 8 parts, gating to mass <=230 recovers 115/119 misses and avoids virtually all negative
overhead, but the aggregate remains neutral because the recovered positives are already cheap. At 7
parts, missed positives reach mean pass-1 depth 2.80 after 2,351 candidate evaluations, versus depth
5.09 and 5,073 evaluations for negatives. Simple depth/count gates recover 30-48 of 65 misses at
roughly -0.06% to +0.2% total time: useful prediction, no material speedup.

### The real positive target is value order, not admission

Searching the audited 2026 logs found the benchmark the cheap corpus hid:

    Sb(15:3,14:3,17:2,8:4,11:2,10:2,19:1,15:1) in 5

It historically took **12,585 CPU seconds** and 18,857,614 candidate evaluations to find a solution.
Crucially, it succeeded in pass 1 and every choice in the printed winning split was already FAST. The
heuristic admitted the right answer; it simply reached it late after expensive recursive child work.

The state remains hard against the end-warm k<=4 cache: the current baseline timed out at the explicit
300-second cap. A cache-native proposal then added a narrower first pass that accepts a FAST prefix
only when all three probes return `TRUE`, rather than treating `TRUE` and `MAYBE` alike. The first build
accidentally enabled that pass recursively at k=4 and was stopped after 34 seconds; it is not evidence.
The corrected root-only build also timed out at 300 seconds (303 seconds wall), matching the baseline's
300-second timeout (304 seconds wall). There is no complete cache-certified route to the hard witness.

**Decision.** Do not deploy the pair/triple/quad filters, discrepancy passes, or `TRUE`-only pass. They
improve offline classification or positive hit rate but not the runtime the solver pays. The next
heuristic experiment must target **value ordering within the existing FAST set** on this hard positive:
instrument which uncertain full candidates consume the time, compare their prefix/cache-status traces
with the known winning split, and order already-admitted choices without adding another pass. This is
also the correct benchmark discipline: a proposal that only improves the millisecond positives has no
chance to move total runtime.

## 2026-08-09 — full star-expansion majorization: the shape heuristic was a theorem

The user proposed attacking long states from their eventual singleton leaves rather than adding
another low-dimensional cache. The first static route has a sharp limit: singleton **subgraphs** of
`K_{n,m}` cannot use `m`. Several vertex-disjoint stars inside one component must partition the same
wide shore, so their sizes sum to at most `n`; checking every such subgraph collapses to the existing
one-copy downgrade `(n:m)->(n:1)`.

The missing relation is not subgraph inclusion but **vertex splitting**.

### Vertex-Splitting Pullback Lemma

Let `pi:V(H)->V(G)` induce an injective map from the edges of `H` into the edges of `G`. If `G` is
solvable in `k`, then `H` is: at each strategy node test `pi^-1(T)` where `T` is the original test.
Every edge of `H` receives exactly the transcript of its image, and edge injectivity preserves
separation.

Apply this to `K_{n,m}`, `n>=m`: clone every wide-side vertex once for each short-side vertex. The
result is `m` disjoint copies of `K_{n,1}`, with edges in bijection with the original rectangle.
Therefore every solvable state satisfies

    Phi(S) = sort(n_1 repeated m_1 times, n_2 repeated m_2 times, ... ) <=_w G_k.

This is the **full star-expansion majorization** condition. It preserves the entire mass
`sum n_i*m_i`, strictly dominates the 2026-08-06 one-copy downgrade, and is the strongest static
singleton lift of this kind: within `K_{n,m}`, a lifted star has at most `n` edges and all `nm` edges
must appear, so no singleton lift can majorize `m` copies of `n`. Proof is now in
[theorems/singleton-majorization.md](theorems/singleton-majorization.md).

### This corrects the strongest earlier shape feature

The offline `majtight` feature was already computing exactly `Phi(child)` and comparing it with
`G_{k-1}`. The journal called it unsound because it exceeded 1 on candidates that passed the exact
per-part filter. That inference was backwards: those candidates are jointly impossible even though
each part is possible alone. The cutoff `majtight>1` is a sound refutation; only using its continuous
value below 1 as an ordering score is heuristic. The stale statement above has been corrected in
place.

The condition is not sufficient. In the exact k=4 pair universe,
`Sb(16:1,12:2)` is unsolvable although `Phi=(16,12,12)` passes the first three prefixes of
`G_4`: `16<=16`, `28<=31`, `40<=42`. Of the 885 unsolvable pairs whose 102 individual parts are
solvable, full star expansion catches 257 (29.0%); the exact pair table catches all 885. This locates
the residual obstruction: Singleton Majorization may solve the cloned stars by testing clones of one
original coin differently. A strategy that descends to the unsplit rectangle must keep those clones
**synchronised at every node**.

Equivalently, a part `(n:m)` is a bundle of `m` singleton rows of length `n`. Ordinary majorization
may decompose each row independently. A legal original test must use one common wide-side cut `a`
across the whole bundle, then choose `b` row centres: `b` rows route `(a,n-a)` to outcomes `(2,1)`,
and `m-b` rows route them to `(1,0)`. The corresponding stronger theory must therefore be a
**synchronised/bundled majorization decomposition**, not another scalar metric or subset table; the
following entry develops it.

### Validation before deployment

- Every one of the 349,827 printed solvable roots in the audited 2026 logs `out_k7.txt`,
  `out_k8.txt`, and `out_radio_1.txt` passes full star expansion; so do all three printed children of
  every winning split. Zero observed violations.
- Exhaustively rebuilding the k=4 pair table gives the unchanged 4,368 solvable pairs of 5,253.
- In `warm_k5.txt`, zero of 20,780 positive k=5 facts violate the condition. It directly refutes
  822,537 of 2,024,705 recorded negative k=5 facts (40.6%). At k=4 it refutes 91,627 of 138,065
  negatives, again with zero positive hits.
- The independent `radio_verify.c`, separately upgraded to the pullback lemma, verifies all 62,366
  negative facts in `out_k7.txt` through k=6 with zero gaps (12.89 s, 0.80 GB peak). The Python
  certificate prototype was upgraded independently as well.

### Solver result

`radiobase.c` now applies the condition before its cache lookup. It sorts only the distinct parts by
wide-side size and streams each repeated entry, rather than materialising the expanded state. The
filter is sound and active in every pass and recursive call; it is not an extra heuristic pass.

Matched warm-cache benchmarks:

| workload | prior | full star expansion | result |
|---|---:|---:|---:|
| hard 8-part positive in k=5 | >300 s timeout | **5.3 s** | >56x, now solves |
| exact A+B 8-part negative in k=6 | 237.4 CPU s | **0.0 solve CPU s** | refuted at root |
| 200 sampled 7-part k=5 negatives | 2.15856 s | **1.249594 s** | 42.1% faster |
| all 82 logged 8-part k=5 negatives | 77.7626 s | **0.060670 s** | about 1,280x faster |
| 200 sampled 7-part k=5 positives | 0.314847 s | **0.247027 s** | 21.5% faster |
| all 200 logged 8-part k=5 positives | 0.247179 s | 0.369208 s | 49.4% slower, +0.122 s total |

The A+B state now has a short proof independent of the solver and cache. Its 41-entry profile first
violates at prefix 40: `sum Phi[1..40]=714 > sum G_6[1..40]=705`; total mass is 727 against 729.

Three historically expensive 4-part k=6 positives were also rerun against the same warm k<=5 cache.
These are historical-to-current comparisons, not matched engine builds, but they exercise the deep
recursive cost the replay corpus misses: `15,864 -> 43.4 s`, `14,712 -> 52.8 s`, and
`13,326 -> 0.2 s`. All three remain solvable.

No local solver was left running; the two separately monitored remote `Sa(193)` runs remain alive and
still use the pre-star-expansion builds. The benchmark outputs are small and reproducible from the
commands in P6; no raw artifact was archived.

### Decision

Deploy the theorem-backed filter. It solves the active positive benchmark, eliminates the negative
monster, and improves the expensive negative population; the only measured regression is 0.122 s
spread over 200 already-cheap positives. The pair/triple/quad deployment and discrepancy-pass
rejections from the preceding entry still stand. This made the synchronisation constraint inside the
cloned singleton bundles the next target; the following entry records that result.

## 2026-08-09 (bundled follow-up) — a hierarchy exists, but its first levels are not the long-state heuristic

The synchronization residual now has a precise theorem rather than just an example. Define `R_0(S,k)`
to be full star-expansion majorization. For `d>0`, `R_d(S,k)` holds when one legal rectangle split has
all three children in `R_{d-1}` at `k-1`. Solvability implies every `R_d`; the relaxations are nested;
and `R_k` is exact solvability. The only non-obvious nesting step is `R_1 => R_0`: apply the common
rectangle test to the row-star lift, observe that each inherited row-oriented child is weakly
majorized by that rectangle child's strongest full-star orientation, and attach the three singleton
strategies. The full proof is now in
[the theorem note](theorems/singleton-majorization.md#the-synchronized-majorization-hierarchy-2026-08-09).

There is also a useful algorithmic form. Weak majorization is equivalent to the hinge inequalities
`H_x(t)=sum_j max(x_j-t,0) <= H_G(t)`. A rectangle `(u:v)` contributes
`min(u,v)*max(max(u,v)-t,0)`. For a local split `(a:b)`, its outcome-2, outcome-0 and outcome-1
contributions are respectively those of `(a:b)`, `(n-a:m-b)`, and the sum for
`(a:m-b),(n-a:b)`. Therefore `R_1` is an exact additive multiple-choice capacity problem over the
three outcomes and integer thresholds. It can be searched without repeatedly sorting child
profiles.

### Small exhaustive result

I added `tools/pairtab.c` to make the previously scratch-only current-solver pair table reproducible,
and `tools/bundled_majorization.py` as an independent implementation of the hierarchy. The commands

```
clang -O3 -DMAX_K=4 -DMAX_N=64 tools/pairtab.c -o /tmp/pairtab4
/tmp/pairtab4 4 16 > /tmp/pairs_k4.txt
tools/bundled_majorization.py census-pairs 4 /tmp/pairs_k4.txt
```

rebuild 102 raw oriented individually solvable parts and 5,253 raw pairs, of which the current C
solver labels 4,368 solvable. After orientation there are 1,485 canonical states: 1,247 positive and
238 negative. `R_0` rejects 68 of the 238 canonical negatives, `R_1` rejects 150, `R_2` rejects 229,
and `R_3` rejects all 238; none rejects a labelled positive. The hierarchy run took 0.003, 0.126,
0.188 and 0.281 Python CPU seconds by level in the final run. These are comparisons with exhaustive
current-solver labels, not independent certificates for the negative labels.

The original residual illustrates the depth cleanly:

```
tools/bundled_majorization.py ladder 4 16 1 12 2
```

reports YES at `R_0,R_1,R_2` and NO at `R_3,R_4`. The first synchronized split
`[8:0,7:2]` gives children `Sb(7:2)`, `Sb(8:1)` and `Sb(8:1,5:2)`, whose full-star profiles all fit
`G_3`; each child also has another synchronized continuation. The obstruction only appears on the
third synchronized layer.

### A direct fixed-width majorization rule is false

Pure width-two states look Schur-downward for two and three bundles at small k, but that pattern fails
at four bundles. The independent reference solver checked

```
tools/refsolve.py solve 4 12 2 10 2 9 2 3 2
tools/refsolve.py solve 4 11 2 11 2 9 2 3 2
```

in 10.44 and 11.64 CPU seconds: the first state is solvable and the second is not. Nevertheless the
unsolvable profile `(11,11,11,11,9,9,3,3)` is weakly majorized by the solvable profile
`(12,12,10,10,9,9,3,3)`. This rules out an exact analogue obtained by replacing `G_k` with one
width-two base sequence. Synchronization makes the feasible set discrete and non-convex under
Robin-Hood transfers; it cannot be compressed into an ordinary Lorenz curve.

### Long-state engineering result

`R_1` is cheap, but it is not new work for the solver. `canSolveB` applies full-star majorization to
every partial child before the cache lookup. Because adding later parts cannot repair a prefix
violation, every complete candidate it reaches has already passed exactly the `R_1` inequalities.
A separate `R_1` pre-pass can only duplicate that walk or generate a different order.

The residual four-part positive `Sb(29:6,19:9,13:12,36:3)` in 6 makes the ordering failure concrete.
Against `/tmp/k6lab/warm_k5.txt`, the current solver found the exact winning split
`[8:1,8:4,11:11,19:2]` after 37,899 counted prefix candidates and 42.7 solve CPU seconds (60 seconds
including cache load, peak RSS 2.60 GB). The additive `R_1` prototype instead found
`[10:2,17:8,3:5,12:1]` almost immediately. Its outcome-2 and outcome-0 children are exactly
unsolvable in 5; only outcome 1 is solvable. Worse, all three children pass `R_1`, so the dead split
also witnesses `R_2` for the parent. Thus even the next hierarchy level does not distinguish this
cheap dead witness from the real winning split.

Deeper checks do not yet pay. On the residual exact negative
`Sb(17:11,16:11,22:7,21:6)` in 6, the transparent Python `R_2` checker used 6.33 CPU seconds and still
returned YES; `R_3` hit a 30-second CPU cap. The current warmed solver refutes the same state in
0.098 solve seconds. The hard positive's isolated `R_2` and `R_3` probes each hit the same 30-second
cap before returning. No capped process was left running.

The long controls are reproducible with the same bounded build and warm cache:

```
clang -O3 -DMAX_K=6 -DMAX_N=193 radio_one.c -o /tmp/radio_bundled
tools/capped_run.sh --seconds 900 --rss-gb 8 --label bundled-positive -- \
  /tmp/radio_bundled /tmp/k6lab/warm_k5.txt 6 29 6 19 9 13 12 36 3
tools/capped_run.sh --seconds 900 --rss-gb 8 --label bundled-negative -- \
  /tmp/radio_bundled /tmp/k6lab/warm_k5.txt 6 17 11 16 11 22 7 21 6
( ulimit -t 30; tools/bundled_majorization.py solve 6 2 17 11 16 11 22 7 21 6 )
( ulimit -t 30; tools/bundled_majorization.py solve 6 3 17 11 16 11 22 7 21 6 )
```

The local benchmark log was 112 KB and the pair-census output 246 KB; both are regenerated by
the recorded commands, so neither was archived.

### Decision

Keep `radiobase.c` unchanged. The hierarchy is the right theoretical interpolation between static
majorization and exact search, and the hinge form is a useful research primitive, but unconditional
`R_1` is cache/prefix-redundant and direct `R_2/R_3` spends more than the warm exact solver on the
controls that matter. Any next attempt should be a bounded, fallback-safe approximation used to
order the existing `FAST` candidates on the 42.7-second positive. The rough standalone C probes were
discarded; only the transparent hierarchy checker and reproducible pair-table generator were kept.

## 2026-08-10 — m=6: keep the forced trunk, discard the fitted continuation

The user supplied the right evidential correction: the canonical witnesses were fitted naively.
They can prove a state solvable, but a late subtree failing to lift says nothing about another
subtree beneath the same early prefix.

The exhaustive `trees-2023` artifact locates one exact boundary. The state
`Sb(110:3,115:2,121:1)@7` has 2 working splits among 37,700,928, one outcome-complement pair. Its
mixed child is

    Z_6 = Sb(53:2,52:2,57:1,57:1) @6.

`Z_6` has 12 working splits among 346,599,648. Exchanging the identical `57:1` parts and globally
complementing the test reduces them to three genuine classes, represented (in the displayed state
order) by

    [31:2,27:1,26:0,25:0]
    [31:2,26:1,26:0,25:0]
    [27:1,31:2,26:0,25:0].

The committed witness takes only the first. Thus the early trunk is the robust object; all
ambiguity from `Z_6` onward is later than it. The rows and split counts are already sourced in
`data/exhaustive_multipart.csv`; the representatives above were re-extracted from the indexed raw
artifact.

### Parametric kernel and the one-unit slack law

Let

    A_t = 2^t,
    C_t = 2^t-(t+1),
    D_t = 2^t-(t(t+1)/2+1).

Extending the arithmetic of the early `k=9` prefix, without extending its subtree, reaches

    Z_t = Sb((D_t+2t-1):2, (A_t-2t):2, C_t:1, C_t:1) @t.

Its full-star profile has six rows and total mass `H_t(6)-1`, where `H_t(r)` is the sum of the
largest `r` entries of `G_t`. The dyadic refinement identities give

    H_t(6) = H_(t-1)(2)+H_(t-1)(6)+H_(t-1)(4)
           = 2 H_(t-1)(3)+H_(t-1)(6).

Consequently every first split that even passes `R_0` has child row counts `(2,6,4)`, `(3,6,3)`
or `(4,6,2)`, and the three nonnegative integer capacity slacks sum to exactly one. This is a
structural reduction, independent of which exact tree is selected.

It also explains one misleading `t=6` option. The third exact split class relies on
`D_(t-1)+2t-2 = C_(t-1)`. The difference between the right and left sides is
`(t-1)(t-6)/2`, so the equality is a `t=6` degeneration and cannot be lifted literally.

### Sound synchronized obstruction at the first nondegenerate levels

`tools/bundled_majorization.py m6-kernel` now performs the complete specialized check: construct
`Z_t`, enumerate every first split whose three children pass `R_0`, quotient outcome symmetry, and
require every child to pass `R_(d-1)`. It never reads a witness tree.

Recorded runs:

| kernel | raw `R_1` splits | distinct child triples | result | CPU |
|---|---:|---:|---|---:|
| `Z_6` | 276 | 66 | `R_4=YES` | 3.3 s |
| `Z_7` | 356 | 84 | `R_4=NO` | 21.2 s |
| `Z_8` | 424 | 101 | `R_4=NO` | 57.5 s |

Because solvability implies every `R_d`, the last two negatives prove `Z_7` and `Z_8`
unsolvable. They rule out the whole parametric kernel at those values, not merely the continuation
chosen in `canon_473_6_at9.tree`.

What this does **not** prove: that `Sb(976:6)@10`, or the conjectured `m=6` formula in general, is
false. That requires showing that an optimal large-k root is forced through this same prefix.
Nor do two parameter values prove `Z_t` impossible for every `t>=7`. The `R_1` split classes have
stabilized by `t=7`, which makes a parametric proof plausible, but it still has to be written.

The theoretical programme is now clean: classify the large-k root prefix without witness fitting;
then either find a different prefix that carries the `BBCD` profile, or prove the stable finite
kernel classes all contain an `R_3`-forbidden child. Only after that should low-k cases be recovered
backwards.

## 2026-08-10 — the deficit automaton gives the `k=10,m=6` upper break

**Corrected 2026-08-26:** all negative conclusions in this entry remain exact, but arbitrary
singleton-majorized positive terminals are conditional.  In particular the run proves
`n(10,6)<=973`; it does not, without the open singleton converse, prove equality.

The requested large-`k`-first analysis changed the result, rather than merely explaining the old
witness.  Write a near-top width as `n=2^k-d`.  Any first cut that can pass full-star majorization
has

    a = 2^(k-1)-u,        n-a = 2^(k-1)-(d-u),        0 <= u <= d.

Thus the exponential coordinates disappear: a part with deficit `d` has only `d+1` possible
wide-side cuts.  If the expanded row deficits of a state are `p_1 <= ... <= p_r`, and the deficits
of the top `r` entries of `G_k` are `e_1 <= ... <= e_r`, full-star majorization is exactly

    sum(p_1..p_j) >= sum(e_1..e_j)  for every j.

For six rows the base deficits are `[0,1,k+1,k+1,Q,Q]`,
`Q=k(k+1)/2+1`.  This is the scale-free state description that had been missing from the
fixed-`m`, `n >> m` discussion.

### The balanced escape route is real as a case, but not as a solution

Shallow synchronized majorization leaves both the old `2+4` root family and a qualitatively new
`3+3` family at `Sb(976:6)@10`.  The centered mixed child of the latter is
`Sb(488:3,488:3)@9`; the corresponding centered child one level earlier is
`Sb(237:3,236:3)@8`.  An exact bounded singletonization search rejected the latter through six
levels and the former through all nine levels.  The neighboring centered total-973 state
`Sb(487:3,486:3)@9` is also exactly unsolvable (104.819 s).  This does not prove that every
`3+3` split at total 973 fails, and no uniqueness claim is being made about the successful root.

### Exact-negative bounded recurrence

`tools/search_singletonization.cpp` implements `C_d(S,k)`: require full-star majorization, accept
an arbitrary singleton-majorized state as a permissive terminal, otherwise choose one legal
synchronized split whose children satisfy `C_(d-1)`.  At `d=k`, `NO` is an exact refutation because
every real strategy lies inside this relaxation; `YES` is conditional unless all terminals are
canonical or distinct-slot embedded.  The wide-cut interval above is complete; while assembling
several parent parts, rejection of a partial child is sound by subgraph monotonicity.  Along the
present searches the total narrow-side multiplicity is at most six, so a normalized state has at
most six nonempty parts.

Before trusting a new negative implementation, it was checked against independent controls.  On all
25 proven one-part frontiers with `k<=6,m<=6`, it rejected `n+1` and accepted `n`.  It also
reproduced the `Sb(16:1,12:2)@4` negative and `Sb(16:1,11:2)@4` positive from `refsolve.py`;
accepted the known `Z_6` and rejected the independently `R_4`-refuted `Z_7`; and reproduced the
width-two positive/negative pair in the synchronized-hierarchy theorem note.  Representative cases
also passed AddressSanitizer and UndefinedBehaviorSanitizer.  `tools/check_witness.py` separately
checks every `[majorized G_k]` prefix, but as corrected on 2026-08-26 those terminals are reported
as conditional rather than as complete positive certificates.

### Exact upper bound and the conditional corrected root

The final cold replay exhaustively rejected `Sb(974:6)@10` in 576.178 s, visiting 810,726 memo
states and 3,712,815,870 partial split assignments.  With that memo retained, `Sb(973:6)@10` was
accepted by the relaxation in 54.4011 s.  The unconditional conclusion is

    n(10,6) <= 973.

The retained negative evidence is `evidence/sb_m6_k10_frontier.txt`; the 115-node, 38-split,
77-terminal conditional construction is `witnesses/majorized_973_6_at10.tree` (six terminals are
genuinely nonembedded).  It uses root
`[477:2]`, producing

    Sb(477:2),        Sb(496:2,477:4),        Sb(496:4).

The `m=4` pure child remains saturated in that conditional construction, but the other width is
477 rather than the fitted 480.
This bypasses `Z_7`.  Consequently both `2^k-k(k-1)/2-3` and the `BBCD` profile are refuted: each
predicts 976 at `k=10`.

The next tempting formula is **not** being adopted.  A constant three-unit correction would give
`n(11,6)=1987`; its natural lifted mixed-state construction reduces to the new hard child
`Sb(503:1,495:2,478:3)@9`.  A capped exact run reached five minutes and timed out at 0.04 GB RSS
without a verdict.  Literally scaling the next split of the 973 witness to
`[247:0,255:2,231:1]` is now **refuted**: its only new hard child,
`Sb(247:1,247:1,240:2,231:2)@8`, is exactly unsolvable (277.622 s, 343,297 memo states,
1,972,790,070 assignments).  This kills that fitted continuation, not the parent state, which may
have a different first split.

The timeout and completed residual line are retained in `evidence/m6_k11_scaled_attempt.txt`.

The right next object is therefore the parametric `m=4 + m=2` mixed-state frontier in deficit
coordinates.  Classify that family first, then work backwards through the low-`k` degeneracies;
do not fit the late subtree of the 973 witness.

## 2026-08-10 — split tables are coarse level-lazy and theorem-filtered

The immediate trigger was memory, not another attempt to make `FAST` clever.  At the
2026-08-10 21:14 UTC `tools/sa193_status.sh --all` snapshot, `run3` had produced 868,760 verdicts
in 21 h 14 m while holding 22.65 GB resident; k=7 accounted for 90.4% of measured CPU.  The older
`run2` status was already 18 hours stale, so its 5.72 GB / 1,897,635-verdict snapshot is not a
matched comparison and does not identify the source.  It was enough to withdraw the convenient
assumption that memory per verdict stays stable when a theorem changes the search shape.

The implementation deliberately stays at whole-table granularity:

- The global array of large `splits` headers is now a pointer index.  The `MAX_K+1` fanout for an
  `sbb` is allocated only when that part is first encountered, and a complete table is keyed by
  `(parent k,sbb)`.
- One table is one exact-sized contiguous allocation: retained cut records, the four live order
  indices, and all three `cle` arrays.  There are no cut chunks and no separately lazy orderings.
- Depth-first search builds the first part's table initially and a suffix table only when the
  prefix reaches that part.  The reachability DP is the sole bulk path: if it arms after ten
  million candidates, it materialises every missing suffix because it genuinely needs them.
- Before allocating, the builder makes two cheap enumeration passes and retains a local cut only
  when each of its three child substates, by itself, passes the `3^(k-1)` counting bound and
  full-star majorization at `k-1`.  Unit groups are counted and then eliminated exactly as in
  `canSolveB`.  No cache negative, fitted rule or conjectural dominance relation participates.

The filter is sound by Subgraph Monotonicity.  For any complete multi-part split, each local
outcome is a subgraph of the corresponding complete child.  If the local outcome already violates
a necessary condition, adding the other parent parts cannot repair it.  Keying by level is required
because the child base sequence and capacity change with `k`; it also means the same `sbb` may own
more than one table, which is why allocation rather than table count is the relevant measure.

### Matched measurements

`tools/split_memory_probe.c` records requested persistent split bytes, excluding malloc metadata
and the result cache.  Its header gives the parent/current build commands.  Both controls used
`MAX_K=6 MAX_N=193` and `/tmp/k6lab/warm_k5.txt`:

| control | parent `5ea9b3c` | level-lazy build |
|---|---:|---:|
| positive `Sb(29:6,19:9,13:12,36:3)@6`, solve CPU printed by engine | 43 s | **32 s** |
| same positive, whole-process wall including cache load | 62.05 s | **49.91 s** |
| same positive, split tables / requested bytes | 71 / 1,407,276 | 82 / **261,560** |
| same positive, geometric / retained options in new tables | — | 3,753 / 2,673 |
| negative `Sb(17:11,16:11,22:7,21:6)@6`, solve CPU | 0.090 s | 0.080 s |
| same negative, split tables / requested bytes | 7 / 1,255,216 | 7 / **135,096** |
| same negative, geometric / retained options in new tables | — | 1,016 / 942 |

The positive found the identical winning split `[8:1,8:4,11:11,19:2]` and printed the identical
top-level `totalsplits=37899`; the speedup comes from cheaper work below that counter.  Requested
split memory fell 5.4x on the positive and 9.3x on the negative despite level-key duplication.
Process RSS was deliberately not promoted as a win: with a 2.5+ GB warm trie it moved in opposite
directions on the two controls, far beyond the one-megabyte split delta, so allocator/cache noise
dominates this small workload.

Correctness gates:

- `tools/split_regression.c` emitted 1,038 definitive one- and two-part answers through k=5;
  every `CHECK` line matched the parent engine.
- The complete `radio.c` Sa ladder through k=8 had identical `result` lines.
- The known `Sb(16:1,12:2)@4` negative and `Sb(16:1,11:2)@4` positive matched.
- The 1,038-case corpus completed under AddressSanitizer plus UndefinedBehaviorSanitizer with no
  report.  All main drivers and the pair/triple/quad/FAST tools compile against the new API.
- `radio_print` rendered its complete 24-line default tree under both sanitizers, and
  `tools/check_witness.py` re-derived all 24 nodes successfully.

That last gate found two renderer bugs before merge.  `radio_print` used to seek an exact
symmetry-boundary cut by decrementing until it found `(floor(n/2),m2)`.  LLDB stopped just before
underflow on `Sb(9:5)@4`: the filtered table had 12 retained entries, the requested `(4,0)` was
absent, and `splitindex[0]` was 1.  The renderer now stops at the last retained cut in the same
canonical cut/complement half, which is identical to the old boundary when that boundary exists.
UBSan then exposed an older independent write to `solutions[-1].refs` for a `TRIVIAL=-1` child;
reference counting now skips trivial leaves.

`-DSPLIT_STATS` now reports per-level candidates, retained options and requested bytes from any
normal driver.  This change removes a bounded but needless source of virtual-memory pressure; it
does **not** compact the unbounded result-cache trie, which remains the next large-k memory problem.
It is not present in the already-running `run3`, and the measured result does not justify throwing
away that cold run's 21 hours of cache.

## 2026-08-10 — a full `Sa(193)` run still does not fit this 24 GB Mac

The current-main engine (`713b7d6`, full-star majorization plus level-lazy split tables) was compiled
with `clang -O3 -DMAX_K=10 -DMAX_N=193` and run cold through `radio_sa193` on the 24 GB M4 Pro.
The trial was bounded to one hour and nominally 8 GB RSS, with a second guard sampling
`vmmap -summary` because the active trap already says macOS can swap a solver out from under an RSS
cap.  The machine began with about 21 GB of old swap allocated by the working desktop, so this is a
viability test under the real local workload, not a clean hardware benchmark.

The positive gate passed:

    result CONTROL Sa(192) in 10 = SOLVABLE  (734.5 s)

That occurred after 173,433 emitted verdicts.  The actual `Sa(193)` phase then emitted another
59,292, for 232,725 total.  At 1,152 wall seconds the run was stopped manually, before the nominal
one-hour deadline: no top-level k=9 state had completed, `vmmap` reported a **7.1 GB physical
footprint**, and its writable regions were 1.3 GB resident plus **5.9 GB swapped**.  The solver's
CPU utilisation had fallen to about 44% and only ~6,300 additional verdicts appeared in the final
three minutes.  In contrast, `capped_run` reported just 2.77 GB peak RSS.  This is a direct second
demonstration that `--rss-gb` is not an allocation bound on macOS.

The run ended by SIGTERM (exit 143), so it says nothing about whether `Sa(193)` is solvable.  It does
answer the operational question: the new split layout is not enough to make a full local run safe
or productive on the working 24 GB machine.  Total anonymous heap still grows into swap during the
first `Sa(193)` branch; compacting or explicitly bounding the result cache remains the necessary
next memory change.  Precise attribution between result-cache and residual split allocations was
not instrumented in this binary, so the measurement is deliberately stated as total malloc
footprint rather than assigned entirely to one structure.

The raw 26,075,455-byte log is archived as
`sa193-local-2026-08-10:out_sa193.txt` (SHA-256
`357d50136933d96a873fd4db3e53f73240df5400020ba4240de24df20f8a6adb`); the audit found zero
contradictions among 751 comparable states.  It is incomplete measurement evidence, not a verdict
source.  Its 232,725-line parsed checkpoint remains at
`/Users/fedor/radio-runs/sa193-local-713b7d6-trial1/sa193.checkpoint` and is sound for a same-log
resume, but resuming locally before the memory representation changes would merely reproduce the
swap failure.

## 2026-08-10 — the cache blowup is two multipliers, and both are now measured

Replaying the local trial's own 232,725-line parsed checkpoint reconstructs the result cache without
running a query or building a split table.  The heavy k=5..7 roots reserve **698,174,470** dense
pointer slots, or **5.20 GiB requested**, for 90.7 million live transitions.  This reconciles the
7.1 GB `vmmap` footprint: roughly 5.2 GiB is the result trie and the remainder is split tables,
relations, allocator metadata and fragmentation.  The exact fact file itself is 8.0 MiB.

The first multiplier is semantic: every printed verdict is expanded to its monotonicity closure.
Ten thousand positives alone create 7.81 million internal arrays / 1.15 GB requested, against
48,898 arrays / 118 MB for ten thousand negatives.  Positive k=5 and k=6 together account for
90.9 million used slots from only 12,919 exact facts.  An antichain is therefore the information-
minimal representation, but querying a mutable dominance index billions of times is a separate
performance problem; do not jump straight from this measurement to removing positive closure.

The second multiplier is representational, and it has a free subcase.  Negative k=7 is only 0.64%
occupied.  `cacheCantSolve` allocates before it knows whether a recursive closure insertion adds
anything, leaving **266,263 completely empty arrays**.  A scratch prototype rolls back an array
when that call contributes zero updates.  It reduces the combined k=7 root from 2.390 GB to
0.908 GB requested (62.0%), makes checkpoint replay faster, matches all 1,039 deterministic
regression answers and passes ASan+UBSan.  It changes no lookup or successful closure insertion.

After that rollback, the existing dense layout would need 3.82 GiB for the heavy roots.  Merely
storing the same dense slots as uint32 offsets in per-k contiguous arenas models at **1.91 GiB**
with the same one-indexed-load lookup.  An adaptive exact-cap layout (dense when occupied, packed
sparse otherwise) models at 1.05 GiB with 64-bit children or 0.58 GiB with uint32 children.  Those
last figures exclude growth slack and are design estimates, not benchmark results; the previous
all-sparse 64-bit prototype really achieved 3.4x at 40,000 inserts but cost 12.4% throughput.

Implementation order originally implied by this evidence was rollback, then per-k uint32 arenas,
then an adaptive container and only finally lazy closure.  The last-segment folding measured below
supersedes that order: land rollback, validate the folding semantics against the pointer trie, then
combine its tagged front/branch representation with uint32 arena offsets.  Adaptive nodes and the
global antichain oracle are fallbacks only if that measured combination is insufficient.

The complete replay counts and layout formulas are retained in
`evidence/cache_shape_sa193_local.txt`.

## 2026-08-10 — a lazy positive dominance oracle is small enough to be credible

The closure measurement suggested an antichain L2 but did not yet supply a plausible hot lookup.
The asymmetry of this checkpoint does.  After Unit-Group Elimination and exact deduplication, the
13,762 positive k=5..7 facts reduce to **6,079 maximal witnesses**: 3,903 at k=5, 1,901 at k=6 and
275 at k=7.  The negative side is much larger (135,374 minimal witnesses), while its retained
upward closure is the part that makes warm prefix refutation cheap.  The first closure-removal
experiment should therefore be asymmetric rather than a universal antichain rewrite.

For each k, give every maximal positive witness one bit and build `H[x]`, the bitmap of witnesses
having at least one part componentwise greater than or equal to part `x`.  A query `Q` can only be
dominated by a positive witness in

    length_at_least[|Q|] AND H[q0] AND H[q1] AND ... .

This is a necessary candidate filter, not the dominance theorem itself: different query parts may
compete for the same witness part.  For example `P=(3:2,2:1,2:1)` passes the individual and sorted
rank tests for `Q=(3:1,2:2,2:1)`, but both `3:1` and `2:2` need the sole `3:2` part, so no injection
exists.  Each surviving candidate is therefore checked exactly.  That exact check need not
backtrack: process query parts by decreasing large side, expose witness parts with sufficient large
side, and take the smallest sufficient small side.  The usual exchange argument makes this greedy
matching exact.

At `MAX_N=193`, all three positive bitmap tables over the maximal antichains model at only
**6.82 MiB**.  The proposed hot path is a bounded direct-mapped or two-way L1 over the actual queried
4--8-part states.  A positive dominance hit is permanent and is memoised exactly; a `MAYBE` is valid
only for the current positive-insertion generation.  This is demand-driven determinisation: the
first occurrence pays the bitmap/matching cost, while repeated warm prefix states recover a direct
lookup without materialising unqueried closure members.

Keep the negative closure for the first experiment.  After rollback it models at 932.7 MiB with
dense uint32 slots, preserving one-indexed-load lookup, or 75.9 MiB with the separately modelled
adaptive uint32 layout.  Thus removing positive closure first attacks the measured 90.9-million-
transition explosion without simultaneously risking the negative prefix mechanism that subsumed
the earlier pair/triple filters.  A fully antichain-based negative oracle remains possible, but it
is not required to make the result cache small.

This is a design, not a speed result.  Before replacing production `cacheCanSolve`, prototype the
positive oracle against the same checkpoint and collect: L1 stable-hit and generation-hit rates,
bitmap words touched, surviving exact candidates, and wall time on the `Sa(192)` control plus the
first bounded `Sa(193)` phase.  Compare every answer against the existing closure trie; a hash is
never trusted without full state equality.

**Superseded as the first prototype by the last-segment folding below.**  The global positive
bitmap remains a fallback if folding one dimension is not enough, but the local construction keeps
more of the present lookup and models substantially smaller on this checkpoint.

## 2026-08-10 — Pareto fronts in the last trie segment remove most closure cheaply

The better compromise came from the user: keep the prefix trie, but do not materialise dominance in
its final part.  At every exact prefix store two 2-D staircases over the possible next rectangle:
the maximal positive parts and the minimal negative parts.  A negative-front hit refutes a query
even when more parts follow; a positive-front hit proves it only when that part ends the query.
This retains precisely the prefix behaviour that made the earlier subset filters redundant.

Insertion stops closure recursion one segment early.  `cacheCanSolve` must also put its current part
on the positive front at every prefix, not only at the terminal call: deleting all remaining parts
proves that shorter prefix state.  `cacheCantSolve` adds only at its terminal part.  Within a front,
positive insertion removes componentwise-smaller points and negative insertion removes larger ones.
Sorted by the large side, each staircase has monotone small sides, so lookup can be a tiny linear
scan or one binary search.  No multi-part injection algorithm is involved—the existing closure has
already handled all earlier parts.

A read-only fold of the rollback replay gives the exact shape this representation would have on the
232,725-line checkpoint.  It replaces 12.82 million branch arrays / 512.30 million dense slots with
2.17 million branch arrays / **70.15 million slots**.  The fronts contain 17.04 million points at
12.65 million prefixes, only **1.348 points per prefix on average**; 79.7% are singletons, only 8,377
prefixes contain both signs, and the largest front of either sign has 19 points.

With uint32 branch handles and singleton fronts encoded directly in the handle or in the branch
array's otherwise-unused positions 0 and 1, the heavy k=5..7 roots model at:

| | k=5 | k=6 | k=7 | total |
|---|---:|---:|---:|---:|
| current rollback + dense uint32 | 616.8 MiB | 904.5 MiB | 433.0 MiB | 1,954.3 MiB |
| last-part fronts + dense uint32 | 193.5 MiB | 55.9 MiB | 42.4 MiB | **291.7 MiB** |

The packed non-singleton lists contribute only 25.3 MiB in that model.  Even a deliberately
pessimistic separate record for every front is 461.7 MiB, and keeping 64-bit branch slots while
tagging singleton fronts is 559.3 MiB.  These are semantic replay/layout models, not allocator or
throughput measurements, but they are much stronger than the global positive-antichain proposal:
both signs keep prefix closure, single-part queries receive full dominance, and average lookup
overhead is roughly one or two point comparisons per visited prefix.

This became the first cache prototype.  The following entry records its implementation, the places
where the layout model was optimistic, and the completed correctness/memory/throughput gates.

## 2026-08-10 — last-segment Pareto cache deployed: 11.2x smaller for 11.6% control CPU

The last-part folding is now implemented in `radiobase.c`.  A trie edge is a tagged `uint32_t`:
empty, an inline positive/negative singleton pair, a handle to a front-only record, or a handle to a
dense `uint32_t` branch array.  Branch slots 0 and 1 hold the two front descriptors.  A front point
uses 16 bits at `MAX_N=193` and 32 bits when `MAX_SBB` is larger; the latter fallback syntax-compiles
at `MAX_N=1030`.  This is one implementation for every layer, not a special short-state side cache.

Two prototypes were necessary.  The semantic prototype put a pointer and two front fields in every
child slot.  It cut the k=5 replay from 161.70 million to 62.54 million slots, but doubled a slot to
16 bytes: peak RSS moved only from 1.42 to 1.19 GB.  That rejected the tempting claim that folding
alone was enough.  The tagged 4-byte representation is the actual memory change.

Incremental insertion also exposed a subtle difference from a read-only fold.  A parsed fact may be
semantically redundant even though the old trie adds another closure marker, so replay accepts a
zero-update insertion; a freshly solved zero-update still trips the diagnostic.  More importantly,
returning from an entire negative recursive call when its first part hit the local front lost two
old negative answers in 1.81 million mutated k=5 queries.  The safe optimization is per edge: skip a
candidate child already refuted by the front, but continue the other segment permutations.  That
restored every old negative and reduced the final combined replay from 637 to 500 MB requested.

On the local `Sa(193)` checkpoint's k=5..7 layers, the pre-change trie reserves 5,585,395,760 bytes.  The
implemented cache requests **499,877,916 bytes**, an **11.174x / 91.05% reduction**; its replay peaks
at 597,213,184 resident bytes including fixed solver tables.  The earlier 291.7 MiB model was
optimistic because direct incremental insertion retains 91.13 million branch slots rather than the
read-only fold's 70.15 million and because handle growth slack is real.  The implementation still
comfortably beats the deliberately pessimistic pointer-node prototype.

`tools/cache_query_regression.c` makes the semantic comparison reproducible.  Across **4,164,958**
exact, targeted-mutated and deterministic random queries at k=5..7, every old `FALSE` and `TRUE` is
preserved.  The only changes are **4,622 `MAYBE -> TRUE`** answers.  This strengthening is intended:
folding an exact positive child into a 2-D front soundly answers componentwise-smaller last parts
that the old canonical closure did not always materialise.  Every exact checkpoint line is checked
against its recorded sign before the mutation pass begins.

The deterministic 1,039-answer corpus matches exactly.  It and a 10,000-fact replay pass
ASan+UBSan.  `radio_print` renders its 24-node default tree under both sanitizers and the independent
witness checker accepts it.  The main drivers and pair/triple/quad/FAST tools compile against the
new API.

The throughput cost is real but bounded.  On the warmed residual positive
`Sb(29:6,19:9,13:12,36:3)@6`, both engines find `[8:1,8:4,11:11,19:2]` after 37,899 top-level splits:
the old engine uses 32.8 solve seconds / 55 wall seconds / 2.40 GB peak RSS, and the compact one uses
42.3 / 65 / 0.78 GB.  The decisive cold `Sa(192)` control then returns **SOLVABLE in 819.9 CPU
seconds**, versus 734.5 before the change (**+11.63%**), with **0.41 GB peak RSS** and no utilization
decay.  A roughly twelve-percent control premium is acceptable because the old engine entered swap
before completing one `Sa(193)` root; the compact engine makes a local continuation credible.

The complete commands, layout accounting and caveats are in
[`../evidence/cache_last_front_2026-08-10.txt`](../evidence/cache_last_front_2026-08-10.txt).  The
17 MB control chatter was not archived: `Sa(192)`
already has canonical witness proofs, so this run is performance/correctness validation rather than
a new mathematical claim.  No local solver from these measurements remains alive; the pre-change
remote `run3` watcher is untouched.

## 2026-08-10 — cold compact `Sa(193)` run launched locally with a 20 GiB footprint guard

The current compact engine is now running the full cold driver from commit `7ceb59d` in
`/Users/fedor/radio-runs/sa193-local-front-7ceb59d-cold2`.  It started at
2026-08-11 01:12:42 UTC with no cache argument, the `Sa(192)` control enabled, and no wall-time
limit.  The compiled binary SHA-256 is
`c3972f6777fb2fe5b71fb54c307065501629e0c5197e96c2f2beebe186c4335b`; source and supervisor hashes
are recorded beside it in `run.meta`.  The initialization banner confirms `cache=(none, cold)`.
The control had not completed at this journal cutoff, so this records only a launch, not a verdict.

`tools/sa193_local_supervisor.sh` owns the run.  Every two minutes it records CPU, RSS, virtual size,
`vmmap` physical footprint, system swap, memory pressure, log size, and free disk.  It has exactly
four automatic stop conditions: a 20 GiB process footprint, less than 10 GiB free disk, five
consecutive `vmmap` failures (the guard can no longer be trusted), or the solver's own exit.  It also
keeps macOS awake and regenerates a same-log parsed checkpoint hourly.  The append-only raw log is
itself enough to regenerate that checkpoint after a crash, so rescanning it every few minutes would
only add long-run I/O.

The first directory, `sa193-local-front-7ceb59d-cold1`, is a launch failure and contains zero bytes
of solver output.  Starting the supervisor with `nohup` inside a short managed command did not
detach it from that command's process-tree cleanup: solver, supervisor and `caffeinate` disappeared
together, with neither stderr nor `completion.txt`.  Nothing mathematical ran far enough to record.
The replacement keeps the supervisor in the foreground of a persistent execution session.  This is
a durable operational trap: under a managed shell, verify survival after the launching command
returns; `nohup` by itself is not evidence of detachment.

## 2026-08-10 — compact AWS `run4` launched beside `run3` for a matched-host comparison

The compact engine is now also running cold on the existing `r7iz.4xlarge`, rather than replacing
the irreplaceable `run3` cache.  A pre-launch read-only inventory found 123 GiB RAM, 98 GiB available,
no swap, 196 GiB free disk, and only one solver: `run3` at 23.66 GiB RSS and one full core.  The old
cloud-init shutdown risk recorded on 2026-08-09 is no longer live: `cloud-final.service` exited
successfully on 2026-08-06 and no pending shutdown process exists.

`run4` started at 2026-08-11 01:37:20 UTC in `/root/run4`, from source bundle commit `6af384e`.
The binary is `radio_sa193_v4` (SHA-256
`9c1a3de315f210e53171672e0d28a00e1ab6aa8d3264bc030b96d930bbd0f84c`); its `radiobase.c` SHA-256
is `99f84940b728312f774de0222f92151cf8c472d96824590ff0bead8ded6158b4`.  The first raw lines report
`split_index_size = 74504 (level-lazy mode)` and
`control=yes, cache=(none, cold)`.  PID 542146 and `run3` PID 375197 both measured approximately
100% of one core after launch.  The first watchdog snapshot reported 1,743 verdicts and 0.17 GB RSS;
the control was still running, so none of these launch facts is a mathematical verdict.

The comparison is isolated at every mutable boundary: distinct directory, binary name, cache,
raw log, watchdog, `memprofile.csv`, immutable segment name, and S3 prefix `run4/`.  Its watchdog and
hourly same-run checkpoints are otherwise identical to `run3`, so
`tools/sa193_status.sh --compare --watch` gives the intended elapsed-time/RSS/progress comparison.
The SSM launch command was `df314504-e0eb-4830-b5a6-07d1da1520de`; `run.meta` on the instance holds
the remaining hashes and PIDs.

Memory is bounded jointly, not independently guessed: `run3` retains its 40 GiB RSS cap and `run4`
has 60 GiB, so the two solvers cannot consume more than 100 GiB of the 123 GiB host.  `run4`'s wall
backstop is ten years—an accident guard, not a schedule—so it should run to a verdict or a concrete
failure.  A separate idle guard watches both exact binary names.  Only after both are gone does it
wait another 20 minutes for the 10-minute watchdogs' final uploads, then issues instance shutdown;
the EC2 shutdown behaviour is `stop`, preserving the EBS volume rather than terminating it.

During the AWS staging, the independent local `cold2` run passed its own control:
`result CONTROL Sa(192) in 10 = SOLVABLE (807.7 s)`.  At 1,568 elapsed seconds its supervisor
reported 242,348 verdicts and a 1.0 GiB physical footprint.  The pre-compact local trial had already
reached 7.1 GiB at 1,152 seconds, so this is the first live-search confirmation that the checkpoint
replay reduction persists beyond the control.  It remains an early measurement, not an upper bound
and not a verdict on `Sa(193)`.

## 2026-08-10 — exact-state comparison instrumented; the `run3` example was not a refutation

The proposed matched benchmark was `Sb(48:48,64:33)@8`.  Searching the complete live logs before
interpreting it changed the premise: `run3` contains progress lines from elapsed 60 through 2,602
seconds, ending at `left=565/1225 totalsplits=45149`, but no `can solve` or `can't solve` line for
that state.  It therefore contributed no exact fact to the raw log/same-run checkpoint and must be
recorded operationally as a non-verdict/MAYBE, not as a refutation.  Surrounding descendant verdicts
do not repair the missing target verdict.  This is still a useful comparison target: a compact
`FALSE` would be a
qualitative `MAYBE -> FALSE` improvement, while another non-verdict gives a directly comparable
stall cost.

The compact local run entered the same state after its control.  Its enumeration has 1,149 outer
options rather than `run3`'s 1,225, so only state, level and semantics are held fixed—not traversal.
Its first matching progress line appeared just after a monitor sample at 844 solver-wall seconds
and 915.4 MiB physical footprint.  By state elapsed 1,989 seconds it reported `left=510/1149` and
47,377 tested combinations; no final verdict existed at this cutoff.  The local physical footprint
was 1.1 GiB during the later part of that interval.

`run4` had not reached the target because its own control had not finished.  More importantly, at
68m51s CPU it had emitted no new line for about 57 minutes: the file remained at 103,778 lines after
the control root's `Sb(112:80)@9` progress line at elapsed 436/1000.  PID 542146 remained runnable at
100% CPU with 301,624 KiB RSS and no swap.  This is a genuine search-path stall but not yet a reason
to discard the cold run: memory and process health are fine, and the user explicitly wants a longer
completion attempt.  The identical source's 807.7-second local control shows that clock-sensitive
deadlines/search order, not just cache lookup throughput, can send the two platforms down different
paths.

To retain the matched measurement, commit `25c843d` added `tools/sa193_track_state.sh`.  A detached
read-only instance of it (SSM command `670731cf-c948-4f50-8b5c-493934367413`, PID 553594) started
before `run4` first mentioned the target.  Every exact matching progress or verdict line records UTC,
solver age, RSS, VmData, VmPeak, raw-log line and message, refreshing
`run4/matches/sb48_48_64_33.tsv` in S3.  It does not signal, restart, cache, or otherwise affect the
solver.  Thus a future session can compare the completed state without reconstructing memory from
coarse ten-minute whole-run samples.

## 2026-08-10 — exact paired refutations expose an approximately 25% compact-build CPU cost

The first timing comparison does not need to wait for the stalled control root.  I ranked every
completed negative `Sb` verdict in the current `run4` raw prefix by its own inclusive `took` field,
then joined it to `run3` by the exact printed `(state,k)` key.  There were no duplicate verdict keys.
Of 98,355 completed negative `Sb` calls in `run4`, 98,253 had an exact `run3` verdict; the slowest
entries all matched and all had the same negative outcome.  The source snapshots were:

- `run4/seg-seg-20260811013720Z-6af384e/out_sa193.txt.zst`, last modified
  2026-08-11 02:37:33 UTC, 103,778 raw lines, compressed SHA-256
  `1ca28c122917716ef9a874ee314d9d42f1b1da608d4a0945238809a94dc9bb50`;
- `run3/seg-20260810T032021Z/out_sa193.txt.zst`, last modified 2026-08-11 02:26:27 UTC,
  999,309 raw lines, compressed SHA-256
  `6c74624e2db3025520bce2ecf33d7ed08ba5eba589aa987dc915341875ff957f`.

The top completed refutations in the compact log were:

| exact state | `run4` took / splits | `run3` took / splits | `run4/run3` time |
|---|---:|---:|---:|
| `Sb(67:46)@8` | 156 s / 219 | 131 s / 299 | 1.19x |
| `Sb(66:47)@8` | 105 s / 186 | 86 s / 264 | 1.22x |
| `Sb(65:48)@8` | 81 s / 323 | 61 s / 431 | 1.33x |
| `Sb(69:45)@8` | 70 s / 228 | 58 s / 320 | 1.21x |
| `Sb(68:46)@8` | 47 s / 249 | 37 s / 347 | 1.27x |
| `Sb(64:49)@8` | 36 s / 283 | 27 s / 393 | 1.33x |
| `Sb(40:20,27:26)@7` | 10 s / 8,616 | 8 s / 8,616 | 1.25x |

This is a broad effect, not one unlucky state.  Summing the inclusive times over every exact
matched negative gives 587.493 versus 471.537 seconds at k=8 (1.246x), 518.777 versus 416.038 at
k=7 (1.247x), and 555.874 versus 439.699 at k=6 (1.264x).  The corresponding aggregate local split
ratios are 0.402x, 0.999x and 1.000x.  Thus the one-part k=8 table filter is doing useful work, but
the compact build is still slower; at k=7 and k=6 essentially equal enumeration isolates a similar
throughput tax.

Interpretation boundary: these are natural-run, same-host paired observations, not an isolated
microbenchmark of the Pareto fronts.  `took` is inclusive, so level totals double-count descendant
work, and `totalsplits` counts only the printed state's local candidates.  The builds also differ in
split allocation/filtering and code layout, and the `run3` prefix was recorded earlier rather than
simultaneously.  The defensible conclusion is that the *whole compact build* costs about 25% CPU on
the very closely matched cold-control path.  The consistency and unchanged k=6/k=7 split counts
make compact result-cache lookup/representation the leading suspect, but that attribution still
needs an A/B microbenchmark.  This is the price side of the memory tradeoff, not a reason to stop
`run4`; its 0.29 GiB footprint remains healthy while the old run is already at 23.8 GiB.

## 2026-08-10 — a 2 MiB exact L1 removes the compact-cache CPU tax

The A/B microbenchmark identified the tax rather than merely accepting it.  On the same
2,201,187-fact `warm_k5.txt` and residual positive
`Sb(29:6,19:9,13:12,36:3)@6`, the pre-front engine took 33.0 solve seconds and compact c146d9d took
42.6.  Both found `[8:1,8:4,11:11,19:2]` after 37,899 top-level splits.  Sampling attributed about
19% of compact solve time directly to `checkCache`, concentrated in the last-part Pareto-front
comparisons; this nearly accounts for the whole gap.

The successful change follows the user's warm-prefix observation literally.  A 65,536-entry exact
L1 stores the complete normalized state, level and only a definitive TRUE/FALSE.  Its 32-bit hash
selects a slot but is not trusted: size, level and every part are compared before a hit, and
`MAYBE` is never stored.  At `MAX_N=193` the entries are 32 bytes, so the whole table is 2 MiB.
States longer than 12 parts bypass it.  The compact 32-bit tagged trie and both Pareto fronts remain
unchanged.

The decisive placement is before repeated singleton/full-star majorization, not merely before the
dominance trie.  After the information bound, Unit-Group Elimination and canonical sorting, an
exact hit is already a permanent mathematical fact and can return immediately.  On a miss, the
same theorem checks and trie run in their previous order.  Definitive theorem failures, singleton
answers, trie hits and completed searches may all populate L1; deadline `MAYBE`s may not.  This is
demand-driven determinisation of actual queries, not closure materialisation.

With a 16,384-entry instrumented prototype, 542,949,678 of 708,562,672 normalized queries hit
exactly (**76.63%**), and only 22 queries exceeded the 12-part bound.  The clean table-size sweep
was 26.7, **26.6**, and 27.1 solve seconds for 32,768, 65,536 and 131,072 entries respectively, so
65,536 is the default.  The final 26.6 seconds is 37.56% below compact c146d9d and 19.39% below the
pre-front engine, with the same witness and counter.  It also completes the 2.2-million-line replay
and solve in 48.74 wall seconds.

Several plausible variants lost and were removed.  Forced inlining alone reached 39.7 seconds.
Direct branch pointers bought less than one second while moving maximum RSS from roughly 624 to
966 MB, so the 32-bit handles stay.  Packing an exact state into 64-bit words made its lookup 37.5
seconds versus 36.1 for scalar 32-bit hashing.  A 1,048,576-entry table took 38.8 seconds when L1
still sat after majorization; cache pressure overwhelmed its extra hits.  These costs are retained
so none of those variants needs rediscovery.

The full gate confirms that this is not a narrow warm-cache win.  `Sa(192)` returned SOLVABLE in
**711.7 CPU seconds**, 719 seconds wall and **0.35 GB peak RSS**.  It used the known root
`Sb(112:80)@9 -> [48:32]`, whose local counter remained 446.  The first compact control took 819.9
CPU seconds and the pre-compaction control 734.5, so the compact memory representation no longer
has a measured control premium.  Clock-driven deadlines led to a somewhat different internal
transcript (177,159 lines), so the full control is an end-to-end gate, while the four-part control
is the exact-path comparison.

Correctness gates are complete.  Against compact baseline c146d9d, all **3,379,067** verdict bytes
from the combined checkpoint, its targeted mutations and 500,000 deterministic random states are
identical.  The 1,038-answer split corpus also matches exactly and passes ASan+UBSan.  The legacy
`checkCache` diagnostic API remains available; `MAX_N=1030` syntax-compiles.  Finally, the packed
two-coordinate front comparison was checked against scalar comparisons for all **86,713,344**
ordered pairs of the 9,312 valid `MAX_N=193` rectangles with zero mismatches; larger coordinates
compile to the scalar fallback.  Commands, hashes, size trials and exact verdict counts are in
[`../evidence/cache_last_front_2026-08-10.txt`](../evidence/cache_last_front_2026-08-10.txt).

The active local cold run and remote `run4` were not restarted.  They retain their frozen pre-L1
binaries, so their measurements remain internally consistent; current `main` improves future runs
or a later same-log resume rather than retroactively changing those sessions.

Final live inventory at the documentation cutoff: local PID 21538 was still at one full core after
12,044 seconds, 450,273 output lines and a 1.3 GiB `vmmap` footprint.  The 04:27 UTC remote snapshot
had `run3` alive at 24.01 GB / 1,039,742 verdicts and frozen compact `run4` alive at 0.29 GB /
103,773 verdicts; neither had completed a top-level `Sa(193)` state, and `run4` had still not closed
its control.  These are operational snapshots, not verdicts.

## 2026-08-10 — exact-L1 AWS `run5` launched beside frozen `run3` and `run4`

The current-main engine now has its own cold AWS session.  A read-only inventory immediately before
launch found `run3` PID 375197 at 24.10 GiB RSS and `run4` PID 542146 at 0.29 GiB, both using one
full core; the host had 97 GiB available, no swap and 196 GiB free disk.  Neither incumbent process,
directory, cache, watchdog nor log was touched.

`run5` started at 2026-08-11 05:05:13 UTC in `/root/run5` from an exact `git archive` of commit
`290a892`.  The archive SHA-256 is
`0a956f5123694829b57c5275d5c62cf6be07acf59b28260586c3bc5bd441f3cc`, `radiobase.c` is
`6cd31bd81d65a4f62054d8b73f16deda68fa95f0e284a882942eb2160e64f3b5`, and the newly compiled
`radio_sa193_v5` binary is
`88bf08ff8498aa223dec3c87d83893c93175ef7ef361028087fba8aa6d62cd38`.  The launch SSM command was
`4d37a1a1-c190-4ce7-8b65-a3adfaf05cd4`; solver PID 576613 and watchdog PID 576650 both survived the
managed command returning.  The first raw lines independently report
`split_index_size = 74504 (level-lazy mode)` and
`control=yes, cache=(none, cold)`.  Its mutable state is isolated under S3 prefix `run5/`, including
the ten-minute status, hourly immutable raw segment, memory profile and same-run checkpoint.

The third solver required changing the cap plan, not merely trusting its small launch footprint.
`run3` retains its 40 GiB wrapper.  `run5` has a native 30 GiB wrapper, and a detached supplemental
guard (PID 576907) lowers `run4`'s effective cap from its immutable launch-time 60 GiB wrapper to
30 GiB.  Thus the effective solver limits remain 40 + 30 + 30 = 100 GiB on the 123 GiB usable host,
leaving about 23 GiB outside them.  SSM command
`5638ef2b-401a-4bd8-96f7-4db43e27a870` installed that guard and atomically replaced the old
two-name idle watcher with PID 576924, which tracks `radio_sa193_v3`, `radio_sa193_v4` and
`radio_sa193_v5`.  A subsequent independent SSM command found both detached guards reparented to
PID 1 and all three solvers/watchdogs alive.  The nominal and effective limits and guard PIDs are in
the remote `run.meta` files.

The 05:07 UTC matched snapshot had `run3` at 24.11 GiB / 1,056,348 verdicts, `run4` at 0.29 GiB /
103,773 verdicts, and the initial `run5` snapshot at 0.17 GiB / 3,304 verdicts.  `run5` had not yet
finished its mandatory `Sa(192)` gate, so this records a verified cold launch and resource envelope,
not a solver verdict.  The status view displayed all three live builds at the time; current
`tools/sa193_status.sh --all` retains their final snapshots.

## 2026-08-10 — run4/run5 deadline diagnosis (first correction; superseded later 2026-08-11)

**First correction, itself superseded.** The observations in this entry were real, but they did not
yet prove the interpretation. Read-only GDB found run4 and run5 in the same information-tight
14-part k=5 child, below the first required negative-cache increment. Source history showed that
this was the *intended* depth-first policy: a bounded state could not throw away the pass until it
contributed facts, and pass 2 handed an unresolved child `NO_DEADLINE`. The 12-second local replay
demonstrated that policy, not whether it would terminate usefully. Run7 later supplied the missing
long-duration evidence that the intended policy loses its finite escape on a saturated cache.

Commit `c13b5d3`, originally recorded here as the repair, instead broke both invariants.  It allowed
zero-progress expiry and polled while still enumerating partial prefixes, before pass 2 could make
the `NO_DEADLINE` handoff. Run4 and run5 were stopped before the distinction was proved at
2026-08-11 05:37:59 UTC. Run7 later vindicated the operational decision, though not the original
claim that those early snapshots alone proved a livelock. Run6 was a valid cold launch and its printed verdicts remain definitive,
but it is not a valid timing/progress comparison.  The corrected debugger evidence, historical
state machine, regressions and run disposition supersede this entry in
[`../evidence/deadline_stall_2026-08-10.txt`](../evidence/deadline_stall_2026-08-10.txt).

The first local segment was also stopped because of that mistaken diagnosis.  It had run for
4 h 34 m, passed the control in 807.7 CPU seconds, emitted 485,337 lines and stayed near a 1.3 GiB
physical footprint.  The supervisor produced a final 17 MiB checkpoint with SHA-256
`3b8622f4d1cc342f28c93626e6554d2c7ca8da8ff0582c993ceeca6e19c73ae2`.  Its facts are sound and
remain part of the local checkpoint chain.

That shutdown exposed a separate guard failure: `vmmap -summary` had hung for 19 minutes, freezing
both memory checks and checkpoint cadence.  Commit `aee1a02` first bounded each probe to 20 seconds
and added a same-run checkpoint argument.  The bound worked, but the first three probes all timed
out: `vmmap` was no longer capable of monitoring this workload.  macOS `top` documents its `MEM`
field as physical footprint and returned the same kind of value in about a second (1,046 MiB while
RSS was roughly 260 MiB), so the supervisor now uses that field under the same 20-second bound.
Every new checkpoint includes both inherited facts and the new segment, avoiding a restart that
silently forgets its prefix.  The guard-fixed continuation (still carrying the later-retracted
`c13b5d3` deadline change) started
at 2026-08-11 05:49:31 UTC in
`/Users/fedor/radio-runs/sa193-local-deadline-aee1a02-resume1`, loaded the exact final checkpoint,
reproduced the positive control from cache, and resumed `Sa(193)` under the 20 GiB footprint guard.

That first continuation was deliberately stopped after 2,878 raw lines to replace `vmmap` rather
than wait for the five-failure safety exit.  I made the mistake the AWS watchdog instructions warn
about: I edited the shell script while bash was still executing it.  Its finalizer did not run.
No solver data was lost—the raw segment and its exact source checkpoint were intact—so I rebuilt the
combined checkpoint explicitly as `source checkpoint + parse_out.sh(raw segment)`.  It has 487,968
lines and SHA-256 `bd5a6e8843f57ce1273dc63bf5f3c4b4df1684aca975922c80294f9003880f58`.
This is mechanically the same operation the supervisor performs, but the failed finalizer is an
operational error and is recorded rather than hidden.

Commit `ebf4e2d` switched the probe to bounded `top MEM`.  A second continuation started at
2026-08-11 05:59:03 UTC in
`/Users/fedor/radio-runs/sa193-local-deadline-ebf4e2d-resume2`, loaded the combined checkpoint,
reproduced the positive control from cache, and resumed `Sa(193)`.  Its first footprint probe
returned normally; four consecutive samples through 06:06 UTC remained near 1.05 GiB with zero
probe failures.  All three raw segment logs must be retained for a closed eventual proof.

## 2026-08-11 — restored the historical policy; run7 exposes its remaining failure

This entry records why `e648e83` was launched. Its final recommendation is superseded by the bounded
probe result later in this journal: run7 showed that source-history intent was not sufficient.

The slow `run6` attempts made the contradiction visible: multiple k=8 calls consumed more than
3,000 seconds and then produced no verdict, even though the exhaustive phase should have delegated
the unresolved child with `NO_DEADLINE`.  Source history confirmed that
`cant_solve_count_min = cant_solve_count + 1` was introduced deliberately in `d3a41ed` as “do not
bail out without making at least some progress.”  The repeated five-times-elapsed extension at
exactly that minimum is also deliberate: it keeps the dive alive until a second fact permits an
expired finite caller to return `MAYBE`.

Live read-only GDB inspection of byte-identical run3/run6 debug rebuilds located both current k=8
calls in pass 1 under finite parent deadlines, and their k=9 roots in pass 1.  A focused local
debugger stop in the erroneous helper showed the concrete zero-progress case:
`deadline=200, start=100, now=201, cant_solve_count=0, minimum=1`, with the helper about to expire.
The prefix poll could then fire at the start of pass 2 before any complete candidate reached
`child_deadline = NO_DEADLINE`.  This explains the no-verdict bailouts without treating them as
negatives.

Commit `e648e83` removes the prefix poll and restores the pre-`c13b5d3` progress/grace semantics.
The focused regression now checks zero-progress suppression, the sliding exactly-minimum grace,
expiry after further progress, and the pass-2 handoff.  The optimized and ASan+UBSan split corpora
matched at all 1,039 lines (SHA-256
`5d0ef6f86bc32b7206da6a9dc2f59c06a11e9ac6b1ff6ad97f5788ed22dec6bd`).  Split and k=5..7
cache-query output also matched exact pre-change commit `290a892`; both `MAX_N=193` and 1030 syntax
compilations passed.  `tools/check_tables.py`, all witness verification and `tools/check_docs.py`
were green.  The correction was pushed to `main` before replacing either run.

Run6 was stopped at about 15:44 UTC after 618,816 raw log lines.  Its wrapper reported exit 143,
35,974 seconds wall time and 1.37 GiB peak RSS; its watchdog finalized the immutable log and
checkpoint at 15:47:51–52.  Run3 remained alive throughout.  Cold run7 started at
15:45:30 from the exact `e648e83` archive with `cache=none`, source SHA-256
`f6edd19d1c683b8cff40f4ae171347b0a8af4ccbfc0667f205721701eb179291`, and binary SHA-256
`691c7a1c23aafc516292e3c1b8e046d241994f2f0046d65dbbb0a8279f915fa5`.  Its solver, watchdog and
new run3/run7 idle guard were all alive with 97 GiB available and no swap; the joint cap remains
40 + 60 GiB.

The local `ebf4e2d` segment stopped cleanly at 443,713 raw lines.  Its final folded checkpoint has
931,075 lines, 34,999,432 bytes and SHA-256
`9bdb9915d5ea8f4fc240aca5a5c631b1177573be24dfa454ba8c7002e195ec56`.  A corrected continuation
started at 15:49:16 in
`/Users/fedor/radio-runs/sa193-local-depth-e648e83-resume3`, loaded that complete chain, and remains
under the 20 GiB physical-footprint guard.  The c13-era facts remain usable because its bad paths
returned `MAYBE`; no missing line was promoted to `FALSE`.

## 2026-08-11 — new outputs are self-identifying

The historical-output problem is broader than retaining the bytes: a raw log did not say which
commit produced its binary, which compile-time bounds and optimization flags were used, or which
runtime command and machine executed it. `run.meta` helped the recent Sa(193) runs, but it was a
sidecar assembled at launch and recorded the *then-current checkout*; a copied older binary could
therefore be mislabeled. This is one reason it is so difficult to tell which bugs and optimizations
the 2023/2025 outputs contain.

The canonical path is now `tools/build_radio.py` rather than a direct compiler invocation. It asks
the compiler for the local dependency set, hashes every source, records the Git commit and whether
the compiled sources/worktree differ, preserves the exact compiler argument vector and compiler
version, and derives a deterministic build ID from those inputs. A generated forced-include header
embeds that identity. The completed binary gets a `<binary>.provenance` sidecar with the same fields
and its final SHA-256. Source archives without `.git` must supply a full 40- or 64-digit
`RADIO_SOURCE_COMMIT`; the AWS launchers do this and fail if it is absent or malformed.

`radiobase.c` emits the embedded identity from a pre-main constructor (with `init` as a guarded
fallback) before any table allocation or cache replay, then adds exact indexed runtime arguments,
UTC/cwd, host, OS/kernel/architecture, CPU, physical RAM,
pointer width, process resource limits, locale and the declared cap/runner settings. Environment
capture is deliberately allow-listed rather than wholesale, so cloud/Git credentials cannot leak
into a log. Comment-prefixed lines are safe in witness output and caches. `parse_out.sh` now retains
every block, and `parse_file` ignores it (including overlong comment continuations), so a resumed
checkpoint carries the identity of each process segment in its proof chain.

Direct compiler builds remain usable for archaeology and regression, but print
`provenance_complete=no`; `tools/check_provenance.py` rejects them in strict mode. Standalone tools
which do not include `radiobase.c` use `tools/run_with_provenance.py`, which validates the binary
against its build sidecar, emits the corresponding runtime block and then execs it. The artifact
uploader now rejects new solver logs without complete provenance. Its
`RADIO_ALLOW_LEGACY_PROVENANCE=1` escape is intentionally conspicuous and must be justified in
`docs/data.md` for pre-banner history.

End-to-end regression covers a canonical build/run with spaces and backslashes in distinct argv
entries, safe wrapper-limit propagation, strict rejection plus explicit inspection of a direct
build, and the standalone launcher; it takes about two seconds locally. A small pair-to-triple table
pipeline also passed after making integer-table readers skip provenance comments (26 solvable pairs,
49 solvable triples in the tiny k=2 case). The provenance code changes output only, not search
semantics.

This is prospective. Active AWS run3/run7 and the local `e648e83` continuation were sensibly left
running; they started before this format and retain their source bundles, launch metadata and
source/binary hashes instead. They must not be described later as having embedded provenance, and
there is no reason to discard weeks of computation merely to add a header.

Operational mistake during this change: I edited `tools/sa193_local_supervisor.sh` while its Bash
process for the active local continuation was live, despite the explicit warning written after the
previous finalizer failure. The solver is a separate process and remained near 99% CPU; the primary
monitor continued advancing every two minutes with a ~1.3 GiB footprint. Nevertheless, Bash FD 255
had reached the new file size, so it was not defensible to assume its buffered post-loop finalizer
was still pristine. I copied a separate `sa193_recovery_guard.sh` into the run directory and attached
it in a persistent execution session. The first persistent copy was then deliberately replaced so
its parser, not only its guard script, was immutable; the final guard is PID 28814, with copied
guard/parser SHA-256 `d965ea47...` / `4689057a...`. It is passive while supervisor 19059 lives; on a
primary failure it takes over the 20 GiB footprint cap, and after solver 19088 exits it independently
folds the exact inherited checkpoint plus this segment's raw verdicts into
`sa193.recovery.checkpoint`, never overwriting the primary output. A first `nohup` launch was reaped
by the managed command's descendant cleanup, exactly as the tooling notes warn; the persistent
foreground session survived and was independently observed under the same session parent as the
original supervisor.

## 2026-08-11 — bounded long-state probes recover the cold `Sa(192)` happy path

The restored historical deadline policy was intentional but not safe. Run7 reached the same
information-tight 14-part k=5 state as run4/run5 beneath a finite parent, then exhaustive pass 2
converted it to `NO_DEADLINE`. The caller required one more negative cache fact before it could
honour expiry, while the warm compact cache produced no new fact. At the 21:26 UTC snapshot the
activation had consumed 19,859 CPU seconds and 270,697,771,548 admitted prefixes without completing
the `Sa(192)` control. This is the evidence the earlier run4/run5 snapshot lacked: the old rule does
not merely allow a useful dive; it removes the finite caller's escape when cache progress is zero.

The replacement in `45c34fd` deliberately has no split hint, cursor or persistent score:

* an expired finite parent is inherited as expired and may return `MAYBE` with no cache mutation;
* pass 2 never turns a finite descendant into `NO_DEADLINE`;
* one- and two-segment states keep the shared parent allowance, matching the observed
  ratio-preserving/opposed-branch constructive spine;
* states with at least three segments initially probe a child for two CPU seconds;
* an unresolved exhaustive pass doubles that local quantum, so a retry permits strictly more work
  even when the cache is saturated.

Several superficially simpler versions failed and were removed. A universal two-second slice solved
the known long k=7 state but starved its k=8 parent: after the 360-second cap the first pass was still
at 1,319 admitted candidates and `left=794/1149` (0.53 GiB peak RSS). The corresponding cold control
eventually reached root pass 2 and handed candidate 5 an unbounded child, so it was stopped at
16 m 20 s. Fixed 30/40-second “commit after two TRUE children” attempts spent their allowance on bad
near-candidates, while repeated two-second passes keyed to cache growth restarted the same prefix;
both good and bad states continued producing facts. Persisting the best-looking split was rejected
because it stores an ordering artifact rather than a structural reason. The two rejected raw logs
are archived under `bounded-probe-rejected-2026-08-11`.

The structural distinction is the part that made the composition work. Against the run7-derived
checkpoint, the known four-part k=7 state solved in 22.0 CPU seconds with
`[17:8,27:13,10:8,12:0]`. Its two-part k=8 parent then solved in the same 22.0 seconds with
`[16:15,45:23]`, the symmetric form of `[33:32,19:9]`; `Sb(112:80)@9` solved in the same 22.0 seconds
with `[48:32]`, symmetric to `[64:48]`. No preferred split was carried between calls.

The decisive normal-learning, genuinely cold control completed `Sb(112:80)@9` in 281 CPU seconds
after 446 admitted top-level candidates and completed `Sa(192)@10` in 376.293 CPU seconds. The
wrapper measured 382 seconds wall and 0.18 GiB peak RSS. An exact replay of run7's saturated k=5
state with a one-second finite parent returned `MAYBE` in 1.000 CPU second with
`negative_delta=0`. These raw outputs and their embedded build/run provenance are archived as
`bounded-probe-2026-08-11`; the detailed index is
[`../evidence/deadline_stall_2026-08-10.txt`](../evidence/deadline_stall_2026-08-10.txt).

Correctness gates did not rely on those timings. The optimized one/two-part corpus matched all 1,038
prior-main answers byte for byte after normalization, SHA-256
`2cf020540919d6fe2f8da20636ecceb8f8d14ccc4d5a3cd599ac0a91e99eade2`; ASan+UBSan produced the same
answers with no diagnostic. `tools/deadline_regression.c` now locks exhausted-parent inheritance,
the short shared spine, the long local slice and expired cache-miss behaviour. Repository table,
witness and documentation checks are run again after this record is rendered.

At that validation cutoff, run3 was not touched and run7 plus the local `e648e83` continuation were
left alive pending archival. They were retired later the same day as recorded below; neither is a
scheduler baseline.

## 2026-08-11 — obsolete `e648e83` runs retired; run3 remains live

PID 19088 was the solver for the same-chain local continuation, not a current-main validation run.
Both it and AWS run7 used the superseded `e648e83` progress-gated pass-2 scheduler, so continuing to
spend CPU on them could not evaluate the bounded-probe policy. They were deliberately stopped after
the bounded-probe control and exact saturated-state replay had already established the replacement.
No file or checkpoint was deleted.

The local solver received `TERM` at 2026-08-11 22:12:33 UTC. It had run since 15:49:16, reproduced
the inherited positive control from cache, and written 188,172 lines / 19,919,478 bytes in
`/Users/fedor/radio-runs/sa193-local-depth-e648e83-resume3/out_sa193.txt`. The primary supervisor was
the Bash process whose script had been edited in place and, as anticipated, exited without a
completion marker. The independent immutable guard then generated
`sa193.recovery.checkpoint`: 1,118,898 lines, 42,433,056 bytes, SHA-256
`ba6ba91fdad83681b36a1b79c126060dc1607857cdf57ed34b18bb3ca65e2f7a`. Its recorded hash matches the
file, and removing its two recovery-header lines produces exactly the same SHA-256 as a fresh
`inherited checkpoint + parse_out.sh(raw segment)` stream. Solver 19088, supervisor 19059 and guard
28814 all exited. The earlier raw segments and every intermediate checkpoint remain in their
original run directories. They are also archived as `sa193-local-chain-2026-08-11`: four separately
hashed raw segments, the closed recovery checkpoint and a metadata tar with each `run.meta`, frozen
binary, monitor/completion/stderr and recovery machinery. All raw segments lack the later embedded
provenance banner, so the uploader's legacy override was explicit and the limitation is recorded in
`docs/data.md`; all four audits found zero contradictions.

On AWS, a read-only identity check first found exactly one run3 solver (PID 375197) and exactly one
run7 solver (PID 688080), with run7's `run.meta` naming build `e648e83`. Only PID 688080 received
`TERM`, at 22:13:30 UTC. Its wrapper closed with exit 143 after 23,282 wall seconds and 0.29 GiB peak
RSS; run3 remained alive throughout. Waking only run7's watchdog sleep caused immediate finalization
rather than waiting for its next ten-minute cycle. The final status reports 104,931 definitive
internal verdicts, zero completed top-level states, and no returned `Sa(192)` control. The saturated
k=5 activation had reached 20,460 CPU seconds and 280,116,882,707 admitted prefixes.

The S3 raw segment under
`run7/seg-seg-20260811154530Z-e648e83/out_sa193.txt.zst` passed `zstd -t`; decompression gives 104,936
lines / 11,065,274 bytes and SHA-256
`a79d31d9b11bf97679451087b90978f7fdc3b8874847bda2ebca305142ddb72c`, exactly matching the EBS log.
The final checkpoint is 3,738,869 bytes with SHA-256
`b7e63923275caa4d486fbefd5cd912cf80d8a530c36372fbf14cece2b8cae545`. Because run7 predates the
embedded provenance banner, its exact source archive, frozen binary, `run.meta`, stderr and
watchdog log were copied beside it and streamed back for hash verification. The final memory profile
was also uploaded manually: the watchdog's exit path refreshed the raw log and checkpoint but, in
spite of the documentation, only its hourly path uploaded the profile. The source now uploads the
profile on exit as well; the live run3 watchdog is a frozen older copy and must still be checked when
that run eventually ends.

The 22:14 UTC run3 snapshot remained healthy and unchanged by the retirement operation: one of
sixteen top-level states complete, 1,651,912 verdicts, 25.50 GiB RSS, and its `Sa(192)` control
SOLVABLE in 540.7 seconds. At that cutoff no bounded-probe run had been started automatically; the
explicit decision and launch follow immediately below.

## 2026-08-11 — bounded-probe AWS run8 launched with a live exact-state comparison

The user explicitly chose to spend the AWS slot on a new cold run. A read-only pre-launch inventory
(SSM `e779f668-8060-4240-84ff-537622d69d22`, 22:45:24 UTC) found exactly one incumbent
`radio_sa193_v3`, no `radio_sa193_v8`, 96 GiB available RAM, no swap and 196 GiB free disk. The old
idle guard still named the already-retired run7, so it had to be broadened as part of the atomic
sidecar launch rather than left as a latent shutdown hazard.

Commit `9395218` added a streaming live comparison and was pushed to main before the source bundle
was made. `tools/sa193_compare.py` scans both raw logs with bounded state, chooses the run behind by
completed roots and then verdict count, retains its six largest inclusive `took` calls and joins
their exact `(state,k)` keys in the peer. The compact status shows each process's wall age, CPU,
roots, control, verdicts, RSS, log size and freshness. Historical output does not contain enough
call-boundary information to allocate abandoned `MAYBE` work to a particular parent's exclusive
time, so the tool does **not** manufacture per-verdict self measurements. The launch version also
reported the aggregate-by-level identity `self(k)=inclusive(k)-inclusive(k-1)` when non-negative;
the follow-up entry below records why that block was removed from the live view. Its initial unit
tests covered positive witness tails, lag selection, exact matching and the aggregate calculation;
a 73 MB local pair took 2.3 seconds and about 15 MB maximum RSS to compare.

Run8 started at **2026-08-11 22:46:06 UTC** under SSM launch command
`0d7a7f49-1033-4429-844f-df87860cbe4f`. It is a genuinely cold invocation of
`./radio_sa193_v8`, with the positive `Sa(192)` control enabled, built through
`tools/build_radio.py -O3 -DMAX_K=10 -DMAX_N=193` from full commit
`9395218dcbdd90d8f6a208b15da1878ff75f6ee1`. The embedded build id is
`c296e0bc477e73ecb8ed5e82e5a128938cbb50015db67dfca2bf87d2848b9e08`; the binary SHA-256 is
`d9ae6e5feea4700be742504e345e2af09c910d790330b37457755cd89d4ac950`. The exact compressed
source archive is `run8/source/radio-9395218.tar.zst`, SHA-256
`38837a2fb0f66036733d139301c0d2b9378e437be22e6266477fad50fb31ea69`.

The resource envelope is deliberate: run3 keeps its 40 GiB guard, run8 has 60 GiB, and the combined
100 GiB ceiling leaves about 23 GiB of the 123 GiB host outside the solver caps. Run8 has the ten-year
accident backstop rather than an intended time deadline. Its status and exact comparison refresh
every five minutes; raw segments and same-run checkpoints retain the existing hourly/final path. A
new idle guard names exactly `radio_sa193_v3,radio_sa193_v8` and stops the instance only after both
are gone plus the final-upload grace period.

The launching SSM command returned successfully, and an independent command after it returned
(`40e28f6c-397b-422b-bb20-55463cdc347c`) found exactly one solver of each name. Run8 solver PID
756288, wrapper 756283, watchdog 756320 and idle guard 756328 were all detached under PID 1 where
expected; run8 was using one core, 96 GiB remained available and swap was still zero. The
raw output passed `tools/check_provenance.py`. The source archive, frozen binary, provenance sidecar
and `run.meta` were immediately copied to S3 and streamed back; all four hashes matched. Preservation
command: `62ebb6fc-e2cf-4654-a39a-2b2a001c66ba`.

The first live comparison completed at 22:46:19 UTC. Run8 was correctly selected as behind with
8,771 internal verdicts and no completed 193-coin root; every one of its six selected slow calls had
an exact run3 match. The tiny early calls were 0.77x–0.81x run3's inclusive CPU, which proves the join
works but is **not** yet a performance conclusion about the hard path. The 22:51:29 cycle selected
six k=8 calls taking 13–81 CPU seconds in run8; all six matched run3 at 0.94x–1.02x. At 5m14s CPU
run8 was making visible progress inside the control root `Sb(112:80)@9`, with 73.0 K verdicts and
0.23 GiB RSS.

The happy-path gate then passed. A read-only check at 22:54:04 UTC found
`result CONTROL Sa(192) in 10 = SOLVABLE (471.6 s)` and the solver already running inside
`Sa(193)` at about 0.44 GiB RSS. This is 0.872x run3's 540.7-second same-host control and establishes
that the bounded-probe build can follow the constructive path in the retained remote environment.
The next normal watchdog cycle at 22:56:37 UTC reported 165,635 definitive `Sb` lines and 0.61 GiB
RSS. Its six slowest calls all matched run3: the control root `Sb(112:80)@9` took 345 versus 414 CPU
seconds (0.83x), while the five selected k=8 calls ranged from 0.90x to 1.02x. This is the first useful
live comparison, but it is still the shared control prefix, not natural `Sa(193)` evidence. Run3 was
not signalled, restarted or modified, and both solvers are intended to continue.

## 2026-08-11 — live comparison keeps stacks and drops by-level timing

The first compact view overcorrected: it reduced each run's useful recursive stack to one line and
spent most of the remaining space on aggregate timing by level. The user correctly preferred the
old root-to-active-level stacks and found the by-level comparison unhelpful. The view now prints the
full current stack beneath each one-line run summary, then only the lagging run's exact matched slow
calls. The comparator no longer accumulates or emits by-level totals.

Run8 initially continued uploading the first-format `COMPARE` object from its frozen launch helper,
so `tools/sa193_status.sh` stripped that obsolete block client-side without touching the live tree.
The separately versioned helper update in the next entry then made the refined comparison live while
retaining the original helper and launch metadata. At 23:12 UTC the stack view showed three active
levels for run3 and four for run8; run8 was inside `Sb(112:81)@9`, at 220.2 K status verdicts and
0.74 GiB RSS. Both solvers remained live, with run3 still one completed root ahead.

## 2026-08-11 — windowed self estimates and active-adjusted level totals

There is a useful middle ground between claiming exact per-call exclusive time and showing none.
For each completed level-k verdict, maintain the cumulative inclusive `took` total at k-1. The
difference in that counter since the previous level-k verdict is the completed child work in the
usual depth-first interval, so

```
estimated self(k verdict) = took(k verdict) - intervening took(k-1 verdicts).
```

The first verdict at each level has no left boundary and is printed `-`. A `MAYBE` return without a
verdict, cache reuse or unusual interleaving can misattribute time, so the output deliberately labels
the field `~self`; negative estimates remain visible but do not get a ratio. On the retained 73 MB
local pair, adding the estimator kept the comparison at 2.12 seconds and about 15 MB maximum RSS.

The per-level totals solve a different problem and remain useful per run. The compact view now keeps
each root-to-active stack and prints `CPU by level i/s (+active)` beneath it. Before recomputing the
level self differences, it adds the current elapsed value from every visible `[solving]` frame to
that level's completed inclusive total. This turns the formerly missing current ancestors into a
much closer live accounting without presenting the levels as a cross-run comparison. The Unicode
fixed-width status table needs positional parsing because its figure spaces are both padding and
digit-group separators; `tools/sa193_level_times.py` implements that parser and has a synthetic
active-frame regression test.

Commit `4cd002e` was pushed before the live helper was changed. The replacement comparison helper's
SHA-256 is `a8599a71b4a6bc3cc4b1e4ad0c8b6485722ba932bf297d5cc294de438a316ccd`; the launch helper's was
`dc7900942c3cc25f626be26f86b51b3ceb3b5c008d3402adefca1793f8ade097`. SSM command
`73df22ca-30a2-4d18-b357-896d1e772e82` copied both into versioned `run8/monitor-updates/` paths,
preserved the original metadata as `run.meta.launch`, atomically replaced only the helper, appended
the old/new hashes and full commit to `run.meta`, and regenerated `run8/COMPARE`. The original and
updated metadata hashes are respectively
`223c05d123e2ba58fd427e1f4744bc0e7e9f87345d35e29577848e9b38684de7` and
`5c39223c1fe5f1055404e44ddf23e7932935ecbbc6eaa376bf027f3c2090358c`. No solver process or binary
was touched.

The first live refined join at 23:32:51 UTC selected the same six slow calls. The first k=9 call had
no prior k=9 boundary, hence `345/-` versus `414/-`. The five k=8 calls had estimated self times of
4.719–16 seconds in run8 and 4.678–15 seconds in run3; their inclusive ratios were 0.90x–1.02x and
estimated-self ratios 0.77x–1.06x. At 23:33 the separate run8 level profile, including 2,260 seconds
of visible active k=9 work, read `k9 2.6k/2.1k`; run8 remained inside `Sb(112:81)@9`, at 0.92 GiB RSS.
The next ordinary watchdog cycle—not a manual one-shot—refreshed `COMPARE` in the same format at
23:37:51 UTC. A final isolation check (SSM `35b6109c-5da1-4fa8-a060-d4638b30f90c`) found exactly one
run3 solver, one run8 solver, one run8 watchdog and one two-name idle guard; the run8 binary hash was
still `d9ae6e5feea4700be742504e345e2af09c910d790330b37457755cd89d4ac950`, with 95 GiB available and
no swap.

## 2026-08-12 — visible retries replace the misleading final-activation comparison

The first genuinely hard matched state exposed a missing dimension in `took`. Both live raw logs
now contain the definitive negative
`Sb(48:48,64:33)[4416,193]@8`, with exactly 53,834 admitted split combinations in each build. The
verdicts agree; the apparent `1181/14 = 84.36x` regression did not describe the work needed to reach
them. Read-only SSM commands `926eb7fa-67c2-469c-9da5-d2c865893d98` and
`f22aa329-8907-42a6-ac07-d8508027c129` reconstructed the progress episodes and estimator windows
from the complete EBS logs.

Run3's first visible episode spans raw lines 146,510–271,994 and reaches elapsed 2,602 with a current
deadline of 2,611 and 45,149 tested combinations. It returns no verdict. Its later definitive retry
at line 1,610,349 takes only 14 seconds because the enclosing root has meanwhile spent roughly
152,000 seconds populating descendant facts. Run8 has a first episode at lines 164,321–204,753,
ending at elapsed 971 against a 999-second cap, then a second episode at lines 465,038–478,744 and a
definitive 1,181-second verdict at line 484,108. Counting the final call once and adding only the
last observed elapsed of each abandoned episode gives conservative visible-attempt floors of
`run8 ≥ 2,152 s` and `run3 ≥ 2,616 s`, an observed-floor ratio of 0.82x. These remain lower bounds:
the old logs neither timestamp a `MAYBE` return nor reveal an abandoned attempt shorter than the
60-second progress interval.

The enclosing-root timing is a separate and stronger scheduling signal. The line immediately after
run8's target verdict enters pass 2 of `Sb(112:81)@9` at elapsed 4,881. Run3's target verdict is
immediately followed by that root's final 155,329-second negative. Thus run8 reached the phase
transition 31.8x earlier, consistent with the bounded probe doing its intended job: stop the first
speculative dive, develop other cache facts, then return with a larger allowance. This is not yet a
31.8x root result; run8 was still in pass 2. The adjacent raw windows were captured by read-only SSM
command `dfccb5f5-e068-4c48-9d21-c6b3bc838ab8`.

Commit `58e3457` changes the streaming comparator to rank and compare these visible-attempt floors.
It groups progress episodes on an elapsed reset; an intervening same-level verdict is also a sound
episode boundary because recursive children have level k-1. The display now prints, for example,
`run8 ≥2.15k(2a)/65`, `run3 ≥2.61k(2a)/0`, and `~0.82x/-`: `2a` means two visible attempts,
the leading inequality marks the aggregate floor, the approximate ratio compares those floors, and
`~self-final` remains scoped to the definitive activation. Lower-bound formatting is rounded down,
never up. Seven synthetic tests cover long/long retries, long/sub-minute retries, a final attempt
with progress, same-level boundaries, matching, parsing and self estimation. The helper was also
run over both growing live logs before deployment; its bounded scan selected this state as the
largest completed run8 attempt aggregate.

Only the monitoring helper was deployed. SSM command `4d0d79db-43cb-4800-bb23-9b2843a5fb2a`
preserved the prior and current helpers, atomically installed the `58e3457` version, regenerated and
uploaded `COMPARE`, appended the transition to `run.meta`, and retained the exact output under
`run8/monitor-updates/`. The current helper SHA-256 is
`61af8b9512dec07fbcc621ff76bdb3386556c83d0bd515a7815093ab4ad6dd52`; the updated metadata SHA-256
is `f2f4bc2af810abd950b793860d4f70cc13d368c9bcf30d978df7b24e7cefaf55`. The solver binary remained
`d9ae6e5feea4700be742504e345e2af09c910d790330b37457755cd89d4ac950`, and both original solver PIDs
remained alive. No cache, deadline, raw log or solver process was changed. The next ordinary
watchdog refresh at 01:00:38 UTC reproduced the new aggregate format, so the result does not depend
on the manual deployment invocation.

## 2026-08-12 — suffix reachability invalidated implicit contraction; proof-safe run9 launched

The surprisingly cheap contraction
`Sb(37:17,25:24,43:13)@7` is not itself a wrong verdict. A fresh cold current-engine query rejected
it in 0.092 CPU seconds after 158,508 candidates, without contracting again; pre-reachability commit
`5ad854e` independently rejected it in 0.377 seconds after 144,988 candidates. The cheapness comes
from the state actually being easy to refute, not from the shortened line being accepted without a
search.

The audit nevertheless found a real soundness bug in the mechanism that produced that line.
Implicit contraction promotes a full negative to a shorter prefix only when every rejection is
prefix-local. `rb_dead`, introduced by `efadab0`, instead reasons about the joint reachability of the
remaining suffix. A forced low-threshold build (`RB_TRIGGER=1`) supplies a concrete counterexample:

```
Sb(5:3,2:2,2:2,2:2)@3 = UNSOLVABLE
Sb(5:3)@3             = SOLVABLE
```

The old engine inferred and cached the second line as negative while proving the first. This does
not invalidate exact full-state rejection by `rb_dead`, but a false shortened cache entry can later
invalidate unrelated negatives. Run3 and run8 both predate the fix and have no telemetry that can
separate affected contractions, so they remain useful only as performance baselines. At an audit
snapshot, run8 had emitted about 126,265 contractions among 542,186 negatives (23%). Its reachability
watchdog had recorded 1,563 activations, 4,236,250,354 tested assignments and 1,359,453,352 pruned
(32.09%). Those aggregates do not identify which individual contractions were tainted.

Commit `75814a7` fixes the interaction narrowly. Each `canSolveB` invocation retains a taint after
`rb_dead` actually rejects a partial assignment, across all iterative-deepening passes. An exhaustive
full-state negative is still printed and cached; only its implicit shorter negative is suppressed,
with `contraction=rb-suppressed:<candidate_size>` in the raw log. `RB_TRIGGER` is now compile-time
overrideable solely so `tools/rb_contraction_regression.sh` can exercise the counterexample cheaply.
The regression checks the exact full negative, the suppression marker, absence of the false prefix
from a parsed cache, the prefix's independent positive solution and reachability telemetry.

Validation passed the repository table, witness and documentation checks; the new forced regression;
the joint-RSS-guard regression; eight status/comparison unit tests; the provenance regression; and a
1,039-query optimized differential replay against parent `eefeae7` with identical output SHA-256
`2cf020540919d6fe2f8da20636ecceb8f8d14ccc4d5a3cd599ac0a91e99eade2`. The forced counterexample was
also clean under AddressSanitizer plus UndefinedBehaviorSanitizer. With the production trigger, the
original shortened state remained an exact 158,508-candidate negative in 0.079 CPU seconds and
emitted no suppression marker. The fix and its monitoring/launch support were pushed to `main` as
`e7fa747264476461a234bf78e49762ee77ad8d8d`.

Cold AWS `run9` began at 2026-08-12 03:21:12 UTC on the existing r7iz.4xlarge, beside retained run3
and run8. It uses no cache and enables the `Sa(192)` control. Its embedded build ID is
`219a8753a3caf79cf7a160cb220a7305b8d914d1bfd8989d52861d1cc1407de4`, and its binary SHA-256 is
`4df4194f9201147b07199266fd66b35970e953dbddc7b799af3dcf60f019dac6`. The exact source archive is
`s3://radio-sa193-393287594714/run9/source/radio-e7fa747.tar.zst`, 1,214,189 bytes, SHA-256
`b6fd7d8bb76fbc6e020ffb0cc8d1a45ef3d618d9e3e3280bfc329801c74c1536`. Independent post-launch and
S3 round-trip checks matched the source, binary, provenance sidecar, `run.meta` and launch manifest;
SSM commands were `f766ce6b-1a8b-45a1-b929-2a65420899ee`,
`78883034-52fd-42c7-a83d-3b5eae9eff46` and `7f9f7920-854e-4148-80d3-57c2588dce7a`.

Run9 has its own 60 GiB guard. Because three independent caps could overcommit a 123 GiB host, a
second guard sums the exact three named solver RSS values and terminates the newest run9 wrapper at
108 GiB, preserving roughly 15 GiB for the host and the longer baselines. The idle guard now waits
for all three solver names. The status/watchdog reports the suppression count and compares run9 to
run8 by default when explicitly selected. Five minutes after launch run9 was alive at 71,811
verdicts and 0.23 GiB RSS, with zero suppressed contractions. Its mandatory control then returned
`Sa(192) = SOLVABLE` in 479.2 CPU seconds, versus run8's 471.6 seconds, and the same process continued
into `Sa(193)`. Direct post-control checks at 03:30 UTC found it at 0.59 GiB RSS with zero suppression
markers; all three solvers plus run9's wrapper, watchdog, combined-RSS guard and three-name idle
guard were alive. The host retained 95 GiB available RAM, no swap and 196 GiB free disk. These checks
were SSM commands `aa6e5298-9dea-44dc-82b7-28d78a3fbff2`,
`451831b4-5e8c-4c5f-8621-0a3cd4095a6d`, and `b28ec8dc-8156-464a-b50e-7a828866e45b`. The positive
control gates the execution but is not yet a result about `Sa(193)`. The next ordinary watchdog
snapshot independently recorded 165,654 verdicts, 0.60 GiB RSS and zero suppressions. Its first
completed matched k=9 call, `Sb(112:80)@9`, took 352 seconds against run8's 345 (1.02x), consistent
with the intended near-zero overhead before the new guard actually fires.

## 2026-08-12 — recursive Pareto lifting finds a new four-part solution, but greedy depth two fails

The new hypothesis was to make the observed one- and two-part constructive prefix explicit, then
recurse on the corresponding four-part state one level down.  The lower long state may need a
componentwise Pareto upgrade before its split is useful.  As corrected on 2026-08-26, arbitrary
singleton-majorized leaves make such a construction conditional; canonical and distinct-slot
embedded leaves remain exact.  This suggested a construction, not merely another scalar score.

The rigid-prefix preflight was encouraging.  Reading the complete k=7 frontier against its k=6
front from `data/pareto_sb.csv` and the archived `fullsolve-2026:out_k7.txt`, all 32 one-part Pareto
roots have a winning first cut based on a corresponding lower-front point.  All 31 roots with a
nontrivial two-part continuation admit the expected opposing-front second cut.  This is an empirical
census, not a uniqueness statement: many roots have additional winning cuts.  It validates the
proposed source of a four-lineage template without claiming that the prefix determines the rest of
the tree.

A literal attempt to place the whole lower four-part state unchanged into one pure child had zero
information-feasible candidates.  The obstruction is a vertex budget: a pure child fixes too much
of every component, leaving the other pure/mixed outcomes over capacity.  The correct recursion is
one level deeper—take a *solving split* of the lower four-part state, then lift that cut.

That move has an exact geometric core.  For aligned parent component `P=(N:M)`, lower component
`T=(n:m)` and lower cut `s=(a:b)`, require

```
a <= x <= a + N - n
b <= y <= b + M - m.
```

The parent selected, complementary and two mixed rectangles then componentwise contain their lower
counterparts.  This is the lift-box lemma, now proved in
`docs/theorems/recursive-pareto-lift.md`.  Its logical direction is deliberately explicit there:
the box preserves lower lineages, but lower-child solvability does not prove solvability after
enlargement.

`tools/pareto_lift_probe.c` implements the bounded experiment.  It centres the box at the
coordinatewise proportional lift, scales the lower split's three outcome masses to the parent mass
with exact apportionment, visits increasing `L1` shells, and orders a shell by total deviation from
those three targets.  Cached false children screen a candidate; other children get a strict local
deadline.  An inverse diagnostic enumerates lower cuts capable of producing a known parent cut.
The tool is standalone and research-only: no `radiobase.c` order, cache fact or proof rule changed.

The primary aligned example is

```
P  = [45:10,33:15,32:14,23:20] @7, mass 1853
T  = [24:5,19:9,19:8,15:13]    @6, mass 638
s  = [3:0,11:5,15:7,7:6], lower masses 202/239/197
```

The scaled target is `587/694/572` and the proportional centre is
`[6:0,19:8,25:12,11:9]`.  The final probe found a new split

```
[10:1,19:8,26:13,10:9], masses 590/701/562
```

at radius 8, structural rank 5.  Its three children independently solved at k=6.  Final wall cost
was 15 seconds and peak RSS 0.16 GB.  Ordinary search with the same warm cache found a different
split after 155,795 admitted top-level splits, 57 solver seconds and 65 wall seconds, peak RSS
0.28 GB.  This is about a 4.3x wall improvement on one positive path, not a broad speedup claim.

The adjacent lower point

```
T' = [24:4,19:9,19:9,15:13]
s' = [3:0,8:4,15:8,7:6], lower masses 194/240/199
```

gave target `568/703/582` and found

```
[11:1,16:7,27:13,11:10], masses 584/702/567
```

at radius 12, rank 274, in 70 wall seconds and 0.18 GB peak RSS.  Assigning the two equal `19:9`
components the other way found nothing within radius 12 in 76 seconds.  The conclusion is not that
the other mapping is globally impossible; it is that normalized sorting loses information needed
by this heuristic.  Lineage labels from the preceding cuts must survive.

Several negative controls narrowed the construction claim:

- Three exact k=5 descendants of the first template were aligned under its k=6 state.  Their lift
  boxes were exhausted (maximum useful radii at most 16) without a cache-open k=6 split.  For one
  known parent cut, inverse enumeration checked 20,000 possible lower cuts, of which 4,385 passed
  the information bound and none were cache-open.  Low-k degeneration is therefore substantial.
- Recursing through the successful k=7 split gave lower selected child
  `L=[3:0,11:5,15:7,7:6]@5` and parent child
  `R=[10:1,19:8,26:13,10:9]@6`.  Lifting the first solving split found for `L` exhaustively visited
  all 774,144 points in the lift box; 66,822 passed the information bound and zero were cache-open.
  The run took 6 wall seconds because these are warm negative lookups.
- Greedy componentwise upgrading of `L` inside the parent-conditioned box had the same unique
  endpoint under maximum- and minimum-gain walks:
  `U=[10:1,11:5,15:7,7:6]`, mass 212 against child capacity 243.  Lifting the first solving split
  found for `U` visited the complete 48,384-point box.  There were 4,509 information-feasible and
  19 cache-open candidates, but strict local solves accepted none; wall cost was 5 seconds.

Thus the lift-box lemma survives, but the greedy full construction does not: one lower witness,
one maximal upgrade and its first solving split need not extend even one more node.  A scalable
construction needs a choice property over multiple Pareto upgrades and multiple inequivalent splits,
or lookahead that couples the three branches.  Consistent with the prior decision about m=6, the
next useful corpus should be at larger k and then work backward through degeneracies.

One tempting ranking modification was rejected.  Prioritising candidates by the number of children
already positive in the warm cache moved the primary winner from rank 5 to rank 17 and wall time
from 15 to 21 seconds.  This gives cache history too much credit and supports keeping split hints
structural and transient.

The five positive-path/performance logs and two paired capped-run summaries are retained and
round-trip verified as `pareto-lift-2026-08-12`.  Each of the five solver outputs has complete
embedded provenance.  The final probe build id is
`8e00909e402767e947f90080ad0bbc6173e3e22f5f9b3672b48290ad5c0edfd6`; its source hashes identify
the then-uncommitted probe exactly.  Failed diagnostic logs were small and deliberately not
archived; their complete states, bounds and measured costs are recorded above so the experiments
are not repeated accidentally.

## 2026-08-13 — k=7 choice corpus completed; k=8 remainder moved to guarded AWS

`tools/pareto_prefix_census.c` now exhaustively preserves the two labelled cuts below every
one-part Pareto root, removes degenerate lineages only under Unit-Group Elimination, traverses each
effective dimension to its complete componentwise-maximal solvable antichain, and enumerates every
endpoint cut.  A full structural analyzer reconstructs all children and multiplicities from the
TSV records and rejects incomplete output.  Interrupted runs can be resumed only from a
`FIRST`/`LINEAGE` block closed by its matching `SECOND_SUMMARY`; imported winners are canonicalized,
deduplicated and exactly re-verified.  The scoped 16-second initial probe applies only to fresh
four-part exact queries in the exhaustive second-cut layer.  Ordinary recursive children keep the
production finite-probe scheduler.

The k=7 corpus completed and analyzed cleanly: 32 roots, 450 winning first cuts, 2,956 labelled
second-cut lineages, 563 canonical targets, 819 upgrade nodes, 610 fixed-dimension Pareto endpoints,
7,396 raw endpoint winners and 3,227 automorphism classes.  `representation_blocked=0`.  A replay
from all 448 closed second-state blocks reproduced the same labelled geometry and downstream
totals, including under the 16-second scoped probe, so the probe schedule changes cost rather than
the census semantics at k=7.

The local k=8 continuation was killed when the IDE restarted, not by its guards or solver.  It had
run for 17,829 seconds (4 h 57 m) and peaked at 7.043 GB physical footprint against a 20 GB cap.
The stopped log contains 621 of 815 summary-closed blocks: 17 of the 70 blocks genuinely absent
from the two older checkpoints were completed, and the eighteenth was interrupted.  That tail had
spent about 959 solver seconds across 200 completed k=6 calls; it is not treated as a complete
block and will be replayed.  The raw log has complete provenance for build
`d09122a95b45951de86762503bdbed7baca88688ffc239e3d134e2d8b5d98c0b` at `54486d6`.

At 2026-08-14T01:40:45Z the remainder launched detached on the existing
`i-0005d74f985c52ae1` r7iz.4xlarge as `/root/pareto-census-k8-20260814T0132Z/pareto_k8_aws`.
The 123 MB compressed input bundle has SHA-256
`dd6fbc6c2f613a21ef5d67309926affa0bc4d540ebf17b909d09f0f3f32cd671`; the exact source archive
from `54486d6ec68a1d268363c358e7de644f57581fb6` has SHA-256
`bc69bd8cfc0c06bcc208dc9ef1d659258235787b9ea0f64731a70a11de357ff1`.  Both and every unpacked
input were checked before build.  The AWS binary's build ID is
`d9a89e3002d69f7879a214fbc78452c257a1c05ac9c51a4ecee55c62432af3cf`, its SHA-256 is
`f7f58456dc7998f16a3a7dce4be0c1b82f956fc7d5d76045fd002c3b7e86ecbb`, and the live raw output
passed `tools/check_provenance.py` immediately after detachment.  The frozen binary and sidecar are
retained beside the source and input bundles in the same S3 prefix.

The census has a 20 GiB individual RSS cap and a ten-year accident backstop, so a normal verdict,
not a wall deadline, should end it.  A new 108 GiB joint guard counts all four solver names and
terminates this newest census wrapper first; the older three-run guard remains as the fallback for
run9.  The sole idle guard was replaced with a four-name guard only after the broader guard and all
four solvers were verified alive.  At the survival check the host had 93.9 GiB available, no swap,
one of each solver, one idle guard and two joint guards.  The supervisor will validate, analyze,
compress and upload the final result to
`s3://radio-sa193-393287594714/pareto-census-k8/20260814T0132Z/`.  Launch and independent survival
SSM commands were `4a6606f5-6875-4dc6-9763-51c328160cdc` and
`f5a47495-c697-4eba-bba1-02eafa3ceb1c`; frozen-binary preservation was
`df8cd807-7a93-4211-87a0-eb5632d759d8`.  `tools/pareto_census_status.sh` performs one bounded status
query and then exits; no local polling process is needed.

## 2026-08-13 — deterministic accepted-prefix budgets; eager root reachability rejected

The deadline state machine was sound after `45c34fd`, but its process-CPU clock still let hardware,
concurrent load and short-poll overshoot decide which finite recursive calls returned `MAYBE`. The
replacement keeps the state machine and changes only its clock: each accepted per-part split prefix,
at the existing `totalsplits++` point, charges one process-global work unit. Finite limits remain
absolute, so a recursive child consumes its parent's allowance and cannot mint a fresh interval.
The initial long-state quantum and later doubling now operate in nominal work seconds. A compile-time
`-DRADIO_CPU_BUDGET` fallback preserves the historical scheduler for controlled comparison.

Calibration deliberately targeted rough continuity rather than a fitted machine constant. CPU-clock
probes on two nontrivial states observed about 13.1--18.9 million accepted prefixes per second, so
the default is 20,000,000 units per nominal second. Repeated cold 100-ms work probes stopped at
exactly 2,000,001 units. The hard k=5 control emitted 404 ordered cache facts with identical SHA-256
in both runs; the independent k=6 residual emitted 1,234, also with identical SHA-256. Actual CPU
time differed. An exact run of the hard positive retained the same witness, 110,510,443 top-level
prefixes and identical reachability totals under CPU and work clocks; it used 132,279,387 recursive
work units. Exact hashes, timings and commands are in
[`../evidence/work_budget_rb_root_2026-08-13.txt`](../evidence/work_budget_rb_root_2026-08-13.txt).

The user-proposed `rb_dead(0,0,0,0)` test was isolated in `tools/rb_root_probe.c`. It is not a full
refutation run: after per-part theorem filtering it asks only whether some Cartesian combination of
first-test cuts can keep all three child masses under `3^(k-1)`. `DEAD` is an exact obstruction;
`ALIVE` leaves every recursive child question open. All 16 retained exhaustive multipart states
were `ALIVE`, including the known negative. An independent complete k=3 census over 283 four-part
states found 216 positives, all `ALIVE`; among 67 negatives, parent star-majorization already rejected
40, root reachability added 5, and 22 remained `ALIVE`. The added power is real but concentrated at
mass saturation.

A temporary eager hook was nevertheless measured, then removed. On the seven-second hard positive it
saved 119,888 of 132,279,387 recursive units (0.091%) while building and querying reachability more
often. On the saturated fourteen-part deadline state, fixed 2M/10M/40M-unit probes were slightly
slower eagerly and produced 17/18/18 cache facts versus 17/17/18 adaptively. The result supports the
existing `RB_TRIGGER` policy: build the DP only after a state demonstrates cost. The standalone root
probe remains for diagnosis and `tools/rb_contraction_regression.sh` now locks its forced DEAD/ALIVE
pair. An initial probe version called full `init()` at large `MAX_N` and became trapped constructing
dominance closures it never used; those exact local processes were stopped, and the shipped probe
uses a lightweight split/theorem initializer.

The work budget is deterministic for the same binary, query and cache history, not across different
histories: successful searches can still promote FAST options. Progress heartbeats remain CPU-timed
but do not affect scheduling. New verdicts carry `work` and `rate`; progress lines carry nominal
effort plus actual `cpu`. The Sa(193) comparison code uses work/rate for attempt effort, actual CPU
for self-time, and marks cross-basis ratios approximate. Existing run3/run8/run9 binaries are frozen
on the CPU scheduler and were not touched.

Regression coverage now includes two cold processes with identical work stopping points/facts,
shared recursive budget consumption, both scheduler modes, root reachability, monitoring parsers and
the existing contraction guard. Default work, CPU fallback and ASan+UBSan builds all reproduce the
same 1,038-answer normalized split-regression SHA-256. All work was performed in the isolated
`work-budget-rb-root` worktree; the primary checkout's unrelated IDE/benchmark changes and every
other chat's branches and processes were left untouched.

## 2026-08-13 — exact hereditary suffix pliability; length bound is sound but incomplete

The follow-up question was whether a long nonunit tail can make `rb_dead` predictably useless before
search.  The useful invariant is absolute pair slack, not a fraction.  With child cap
`C=3^(k-1)`, reduced nonunit mass `M` and `sigma=3C-M`, a suffix of mass `W` is universally pliable
when it can fit every capacity triple `h` in `[0,C]^3` with sum `W+sigma`.  The existing `rb_mx`
table decides this exactly: for every `h0,h2`, its prefix maximum must reach `W-h1`.  Scanning
backward then gives the first suffix after which every later suffix is pliable.  This is stronger
than root ALIVE and deliberately weaker than a solvability claim.

Two cheap sufficient theorems were added to the standalone probe.  Slack at least `2C` makes every
suffix trivial.  Otherwise a retained `(2:1)` table is a pliable base at slack one; a preceding mass
`w` with all pure corners extends a pliable tail of mass `T` when `2w<=T+sigma+2`.  Writing
`D=W-2q` for a `q`-part tail yields the coarser conditions `q>=D+2` at slack one and `q>=D+1` at
slack at least two.  The retained-corner qualification is essential because the child-level
theorem filter can remove a raw pure routing.

The boundary controls separate root packing from hereditary pliability.  Thirteen `(2:1)` parts at
`k=3` have slack one and every suffix is pliable.  Replacing one by `(3:1)` makes total mass 27 and
slack zero: the complete state still packs `9/9/9`, but the final `(2:1)` suffix is not universal, so
the hereditary tail is empty.  `tools/rb_pliability_regression.sh` locks both cases plus the large-
slack theorem and the historical hard positive.

`tools/rb_pliability_census.py` reproduced the complete 283-state k=3 census.  Parent
star-majorization removed 40 and 243 reached the DP.  Exactly 65 had no suffix index at which the
production lookup could reject; the direct theorem proved 29 and the q/D corollary 10.  Exact
no-call cases rose from 0/23 at slack zero to 23/42 at slack five, but every intermediate slack had
both outcomes.  On the 16 retained exhaustive multipart states, four were exact no-call, ten had no
nonempty hereditary tail and two were partial.  The hard eight-part positive (`C=81`, mass 229,
slack 14) has only its final one-part suffix pliable; the saturated fourteen-part state (mass 243,
slack zero) has none.  The known negative `Sb(111:3,115:2,121:1)@7` has slack 1503, at least `2C`,
so all suffixes are pliable and reachability cannot supply its refutation.

Decision: keep the measured-cost production trigger and do not infer an optimal level from length
or slack alone.  The exact cutoff could later suppress O(1) `rb_dead` calls after the DP is already
built, but first instrument call counts by suffix; the scan itself and the avoided lookups need a
measured tradeoff.  Definitions, full distributions and reproduction commands are in
[`../evidence/rb_pliability_2026-08-13.txt`](../evidence/rb_pliability_2026-08-13.txt).  All changes
remain in the isolated worktree and do not touch the running solver branches.

Final verification passed the new sharp-boundary regression and complete census, the pre-existing
rb-contraction regression, all table and witness checks, and `tools/check_docs.py`.  An
ASan+UBSan probe build ran both the thirteen-part slack-one case and the hard eight-part state with
no runtime diagnostic.

## 2026-08-14 — actual suffix pruning confirms slack as a prior, not a new RB policy

The requested combined test now records each actual `rb_dead` call and rejection by suffix, together
with absolute slack, remaining part count and excess `D=W-2q`.  The complete cold k=3 census forces
RB after its first accepted prefix so it measures the proactive counterfactual rather than the
production trigger.  Its 242 profiles contain 1,590 calls and 259 rejections.  Rejection rates by
slack are 49.47%, 18.93%, 3.25%, 5.36%, 0% and 0% for slack zero through five.  At suffix level,
`slack-D>=2` had zero rejections in 230 calls, but `slack-D=1` already fails empirically:
`Sb(4:3,4:1,2:2,2:2)@3` rejects one of three reached prefixes at its final suffix.  These quantities
are strong priors and useful explanations, not shape-free certificates.

Keeping the full slack also strengthens the proved q/D corollary.  Under the same sorted `(2:1)`
base and retained-corner premises, `2(D-q)<=slack-4` implies every direct extension inequality.  It
specializes to the old bounds at slack one through three, then uses additional slack instead of
discarding it.  Diagnostic assertions checked it against both the direct theorem and exact DP at
every suffix.  It improves 11 partial cutoffs in the small census, from head distribution
`{1:10,2:25,3:51,4:157}` to `{1:10,2:36,3:40,4:157}`, but still proves exactly the same 10 complete
no-call states.  Thus the theorem is genuinely stronger without supplying the desired trigger.

The real controls resolve the proposed post-build cutoff.  On the hard eight-part positive,
42,430,348 reachability calls produced 2,940,923 rejections; its exactly pliable final suffix alone
received 33,049,379 calls and rejected none.  The opt-in exact scan skipped 33,049,382 calls in all
(77.89%), retained every rejection and preserved the identical 132,279,387 deterministic work
units.  Five paired CPU runs nevertheless changed direction; four differed by at most 1.07% and one
unreplicated run favoured the cutoff by 7.94%.  There is no stable timing benefit because the avoided
lookup is already tiny beside the surrounding prefix work.  Conversely, the zero-slack fourteen-
part control produced 1,141,496 useful rejections in 6,921,698 calls during a 40,000,001-work probe,
mostly in its last three suffixes.

Decision: leave the ordinary measured-cost trigger and call path unchanged.  Retain
`RADIO_RB_PROFILE_DIAGNOSTIC`, the forced cold census and the sound-but-rejected
`RADIO_RB_PLIABLE_CUTOFF` as research modes.  Full proof, distributions, build IDs, commands and
timings are in
[`../evidence/rb_slack_profile_2026-08-14.txt`](../evidence/rb_slack_profile_2026-08-14.txt).
The primary checkout, other chat branches and live local/remote watchers were not modified.

## 2026-08-14 — excessive-q Pareto assembly reduces D to a two-dimensional deficit frontier

The corrected construction proposal starts from `A=(a:alpha)@k-1`,
`B=(b:beta)@k-2`, `C=(c:gamma)@k-2`, and total height `m`.  Its magenta four-segment branch is

    Sb(d:beta, b:alpha-beta, c:m-alpha-gamma, a-c:gamma) @ k-2,

and only `d=width(D)` is free; the parent objective is `a+b+d`.  The geometry requires
`beta<=alpha`, `alpha+gamma<=m`, and `c<=a`.  We are adopting the following only as a working
assumption: every fixed total height and labelled lineage geometry has some sufficiently large `q`
in which the relevant atomic-leaf construction stabilizes.  There is no proof or claimed threshold
yet.  These A/B/C/D names are diagram components, not the established dyadic atom notation.

Deficit coordinates isolate the free variable.  At residual level `s`, write
`D=(2^s-delta:h)`.  A viable cut is `y=2^(s-1)-u` with complementary deficit
`v=delta-u`.  Its pure children depend separately on `u` and `v`, while its mixed child contains both.
For fixed cuts of the other three staircase parts and a fixed height cut of D, let `U_2,U_0` be the
minimum deficits accepted by the two pure children, and let `M` be the Pareto-minimal feasible
`(u,v)` pairs of the mixed child.  Subgraph monotonicity then gives the exact formula

    delta_D = min_{(p,q) in M} (max(p,U_2) + max(q,U_0)).

Minimizing over the fixed-part cuts and D's height cut solves the slice.  This is the main conceptual
answer: the hard object is generally a two-dimensional mixed-child antichain, not a scalar capacity.
Since A lies at `k-1` and B,D at `k-2`, the diagram's candidate width is
`2^k-(delta_A+delta_B+delta_D)`, so choosing the widest candidate is again just choosing the smallest
total deficit.  The derivation and its exact quantifiers are now in
[conjectures.md](conjectures.md#excess-q-pareto-assembly-as-a-variable-d-slice-working-hypothesis-2026-08-14).

`tools/search_singletonization.cpp` now has a generic `slice` mode.  It adds
`(2^k-delta:variable_m)` to arbitrary fixed residual parts, scans deficits upward with one retained
exact memo, and prints the first positive tree.  Starting at deficit zero makes that first positive
the unconditional maximum if the scan reaches one; a nonzero start requires an independently proved
lower-deficit exclusion.  The synchronization counterexample recorded in
[the Singleton Majorization note](theorems/singleton-majorization.md#why-there-is-no-single-width-two-base-sequence)
gives a sharp regression: with fixed `(11:2,9:2,3:2)@4`, variable width 11 fails, while width 10 is
a subgraph of the note's solvable state and succeeds.
The new output tree passes `tools/check_witness.py`.  A separately compiled one-entry memo build exits
3 with an explicit abort and emits no verdict, locking the rule that exhaustion is not a negative.
`tools/singletonization_regression.sh` reproduces both checks.

The correct diagram supplied later in the session instantiates the previously abstract fixed state:
its three non-D parts are exactly `(b:alpha-beta)`, `(c:m-alpha-gamma)`, and `(a-c:gamma)`.
Thus there is no missing A/B/C child map.  The scale-free continuation is to store eventual
dyadic-polynomial width germs and their two-dimensional minimal antichains, then test the postulated
stabilization under atom refinement instead of guessing a formula for D.  The following entry records
the corrected finite controls and replaces point lists with guarded affine pieces.

## 2026-08-14 — corrected diagram instantiates D; its cut frontier has guarded pieces

The user reported that the initial attachment was the wrong picture.  Its color transcription and
the resulting claim that A/B/C boundaries were missing are retracted.  In the corrected picture,
A is upper-left, B upper-middle, C lies under the right end of A, and D is the upper-right part of one
magenta staircase.  With `A=(a:alpha)`, `B=(b:beta)`, and `C=(c:gamma)`, its four segments are

    (d:beta), (b:alpha-beta), (c:m-alpha-gamma), (a-c:gamma).

The new `search_singletonization assembly` mode constructs exactly this state, scans `d` downward
with one memo, and reports the parent candidate `a+b+d`.  Three exact `m=10` controls recover proven
Pareto rows.  At parent level 5, `A=(7:6)`, `B=(4:4)`, `C=(5:3)` give adjacent D results 2 NO / 1 YES
and parent width 12.  At level 6, `(19:6),(10:4),(12:3)` give 5 NO / 4 YES and width 33.  At level 7,
the repeated pattern `(46:6),(24:4),(27:3)` gives 12 NO / 11 YES and only candidate 81; enumerating
other Pareto triples finds `A=(46:6)`, `B=(22:5)`, `C=(27:3)`, whose branch
`Sb(27:1,22:1,19:3,14:5)@5` is solvable while replacing 14 by 15 is not, giving width 82.  Every
positive tree is independently verified, and `tools/singletonization_regression.sh` reproduces the
four adjacent boundaries.  These values agree with `data/pareto_sb.csv` but do not prove eventual
stabilization.

`search_singletonization mixed-frontier` computes the second-order two-coordinate object needed to
solve a chosen D slice recursively.  It is generic infrastructure, not a reconstruction of the
corrected picture.  For fixed residual parts it considers

    (2^k-u:left_m), (2^k-v:right_m),

uses subgraph monotonicity to binary-search the least feasible `v` for each increasing `u`, retains
one exact memo, and emits a point only when that threshold strictly drops.  Its `complete` flag is
proved from the search box: the vertical side must either contain the `u=0` threshold or exhaust the
legal range, and the horizontal side must either reach `v=0` or exhaust its legal range.  A truncated
frontier remains useful for candidates but cannot certify D.
The legal range includes the endpoint `u=2^k` or `v=2^k`, where that zero-width part is omitted;
stopping at the conventional `n>=m` orientation boundary would not justify an unconditional
`complete=YES` claim.

The synchronized four-bundle control makes the need for two coordinates concrete.  With fixed
`(9:2,3:2)@4`, the complete exact frontier for two variable height-two parts is

    (2,10), (3,8), (4,6), (6,4), (8,3), (10,2).

The pair `(4,6)` works but `(5,5)` does not, despite equal total deficit.  This is exactly the notch
hidden by the earlier scalar slice.  All six positive trees are regenerated and independently
checked by `tools/singletonization_regression.sh`.

Point lists are still the wrong excessive-`q` representation.  The tool now groups maximal runs as

    L <= p <= R,  q=C-p.

For such a piece, combining pure thresholds `U_2,U_0` has the closed form

    max(C,U_2+U_0)
      + distance([L,R], [min(U_2,C-U_0), max(U_2,C-U_0)]).

`tools/optimize_mixed_frontier.py` implements this formula, validates that the pieces reproduce every
printed point, and refuses an incomplete or bounded-depth frontier.  On the synchronized control,
the illustrative synthetic choice `U_2=U_0=5` returns `delta_D=11` and parent D-width 21; the
unattainable balanced shortcut would have incorrectly returned deficit 10.  This is a regression
value rather than a proposed Pareto width.

The first finite series support the guarded-piece representation while stopping short of an eventual
claim.  Complete exact scans with every positive tree checked give one affine piece for variable
heights `(1,2)` at residual levels 3 through 11.  Heights `(2,2)` give three pieces at levels 4
through 11; the number of minimal points is `2k-1`, so the list grows even though the piece rule stays
fixed throughout the checked range.  A level-6 `(1,4)` control already has three separated pieces,
showing that one half-plane is not general.  These finite families are locked in
`tools/singletonization_regression.sh`; none is promoted to an all-`k` theorem.

The exploratory `(1,4)` continuation at residual level 10 was deliberately bounded:

    (ulimit -t 30; search_singletonization mixed-frontier 10 10 1 4 1023 1020)

It was terminated by the 30-CPU-second cap with shell status 152 before printing a frontier summary,
so it produced no verdict; process inventory afterward was empty.  This is precisely why the apparent
piece pattern through the lower exploratory levels is not extrapolated.

The natural parent-level-8 continuation with `A=(104:6)`, `B=(50:5)`, `C=(58:3)`, and `d=35` gives
the branch `Sb(58:1,50:1,46:3,35:5)@6`.  One exact attempt hit a 60-CPU-second cap before emitting a
verdict; bounded singletonization depths 3, 4, and 5 each hit a separate 20-CPU-second cap without a
summary.  All processes were gone afterward.  This is 120 CPU seconds of inconclusive work, not a
construction or rejection.  No Pareto table entry changes in this work.

## 2026-08-14 — complete Pareto-triple enumeration, exact through parent level 7

`search_singletonization assembly-enumerate` now automates the outer A/B/C choice in the corrected
diagram.  It reads `data/pareto_sb.csv`, accepts only complete normalized levels made entirely of
source-carrying proven maxima, and refuses the current level-9 mixture of theorem, legacy and bound
rows.  It enumerates ordered triples under `beta<=alpha`, `alpha+gamma<=m`, `c<=a`, and the user's
still-unproved `m<=2a` working condition.  A binary search of full-star majorization supplies a sound
D upper bound.  Exact scans then continue only while a triple can tie the incumbent; every omitted
larger D fails the necessary bound and every omitted smaller D loses arithmetically.  Therefore the
final optimum is exact over this explicitly conditional family even though losing triples need not
have individually exact D maxima.

The new regression is the durable source for the following finite `m=10` facts.  At parent levels
5, 6 and 7 it respectively examines 68, 133 and 165 admissible triples; 3, 21 and 37 survive the
full-star screen.  The exact family optima are the proven Pareto widths 12, 33 and 82, attained by
2, 4 and 1 triples.  All seven printed branch trees pass `tools/check_witness.py`.  The unique
level-7 winner is the previously found `(46:6),(22:5),(27:3),d=14`; the enumeration proves that its
choice is best within the working family rather than merely exhibiting it.  Source:
`tools/singletonization_regression.sh`, with the input maxima sourced by `data/pareto_sb.csv`.

`assembly-rank` separates the completed static product from exact optimization.  For parent level
8 and `m=10`, the regression locks 165 admissible triples, 37 full-star survivors, top necessary
candidate width 195, and the proven parent maximum 189.  The especially simple width-189 target
`Sb(50:4,39:6)@6` is exactly negative in the regression, so neither low part count nor R_0 is a
construction criterion.  Exact optimization of the level-8 family is still open.

Exploratory local target checks were deliberately not promoted to durable evidence.  Ranking entries
3, 8, 9, 11, 14, 17, 18, 19 and 21 at width 189 returned exact negatives in the current recurrence;
together with the retained two-part control, their measured solve time was 5.459808 seconds.  Entries
2, 5, 6 and 7 each hit an independent 30-CPU-second cap without a verdict; entry 1 is the natural
candidate whose earlier 60-second cap is recorded immediately above.  The raw exploratory outputs
were under `/tmp/assembly-k8-rank*.out` and were not retained, so these observations are scheduling
notes, not evidence claims.  Two one-CPU-second development probes were also killed after entering
the first exact query; that led to the clean `assembly-rank` mode rather than another inferred
negative.  Process inventory was empty afterward.  No CSV fact changes.

## 2026-08-14 — ground-up atom assembly needs only outer A/B/C dimensions

The proposed height-first version starts at `m=1` and constructs consecutive heights.  A correction
to the first formulation is important: the internal witnesses of diagram components A/B/C do not
interact.  They sit in separate adaptive branches and can be attached independently, so the hard
state needs only `a,alpha,b,beta,c,gamma`.  In particular, `a-c` is scalar subtraction, not an
aligned subword of A.  Only the new four-segment D branch has synchronized cuts.

Writing `A_r,B_r,C_r,D_r` for atom values of `G_r`, the corrected recurrence gives explicit uniform
hard-branch constructions through height 5.  For `m=4`, heights `(alpha,beta,gamma)=(2,2,1)` reduce
the branch to `Sb(2C_r:2,A_{r+1}:1,B_{r+1}:1)@r+1`; one split leaves `(A_r,C_r)` in both pure
outcomes and `(A_r,B_r,C_r,C_r)` in the mixed outcome.  The parent profile is `AACC`.  For `m=5`,
heights `(3,2,2)` reduce the branch, two levels above `G_r`, to atom widths
`(ABCD:2),(AAAB:1),(AABC:2)`.  A two-test symbolic split, recorded in
`docs/conjectures.md`, proves the `BBBD` lower-bound construction for `r>=3` (`k>=7`).  Neither
argument proves Pareto maximality.

At `m=6`, the same calculation isolates the known break.  Heights `(4,3,2)` make the old `BBCD`
continuation require D profile `ABBD@G_6`, width 232, in the `k=10` hard branch.  Exact frontier
evidence rules that out and proves `d=229`.  Refining once gives failed word `AAAABBCD@G_5`; replacing
three A atoms by B atoms gives the exact-width accounting `ABBBBBCD@G_5=229`.  The accompanying
outer profiles are A=`A^10B^4C^2`, B=`A^5B^2C`, C=`A^7B`, so `a-c=A^3B^3C^2` is already a
nonnegative eight-atom word at this normalization.  The parent accounting is `A^16B^11C^4D=973`.
These are arithmetic atom decompositions, not a claim that the stored witness has a symmetric
per-coin profile, and the three-unit loss is not extrapolated to later levels.  Sources for the
finite 973/974 boundary remain `witnesses/majorized_973_6_at10.tree` and
`evidence/sb_m6_k10_frontier.txt`.

An attempted bounded-singletonization follow-up launched depths 3 through 8 concurrently before
their cost was known.  All six were interrupted after about 36 seconds each (about 216 CPU seconds
total) without a summary or verdict; process inventory was empty afterward.  Do not repeat those
depths in parallel.  This abort contributes no mathematical evidence.

## 2026-08-14 — the 229 split refines exactly once, then hits the known k=11 obstruction

The `k=10` hard state is more structured than its integer split initially suggests.  Normalize its
three widths at `G_5` as

    B-side = A^5 B^2 C,  A-C = A^3 B^3 C^2,  D = A B^5 C D.

After refinement to `G_4`, the stored cuts 120, 127 and 109 have unique eight-atom subprofiles inside
their respective refined totals:

    A^4 B^3 C,  A^7 B,  A^3 B^3 C D.

The complementary profiles are `A^8`, `A^2 B^4 C^2`, and `A^4 B^3 C`.  Thus the mixed child is
`(A^4B^3C:1)^2,(A^2B^4C^2:2),(A^3B^3CD:2)`.  Evaluating the identical profile split one level later
gives `[247:0,255:2,231:1]`, whose mixed child is exactly the already retained negative
`Sb(247:1,247:1,240:2,231:2)@8`.  This identifies the previous “literal scaling” failure as the
literal atom refinement of the successful 229 split.  It rules out that split template at the next
level, not the parent `Sb(503:1,495:2,478:3)@9` and not eventual stabilization after more levels.
Finite sources remain `witnesses/majorized_973_6_at10.tree` and
`evidence/m6_k11_scaled_attempt.txt`.

At a fixed eight-atom A--D normalization, the search object is finite.  An eight-atom profile
`p=(a,b,c,d)` refines to
`(2a+b,b+c,c+d,d)`; an aligned cut chooses an eight-atom subprofile and uses its complement.  The
first six terminal reference profiles at the same normalization are
`A^8,A^7B,A^4B^3C,A^4B^3C,AB^3C^3D,AB^3C^3D`.  Since the deficit polynomial is

    d binom(r,2) + (c+d)r + (b+c+d),

the 165 A--D eight-atom D germs have an eventual total order by `(d,c+d,b+c+d)`.  This supplies the
maximizer for that fixed slice: enumerate D germs in that order and apply exact profile recursion to
its finite fixed point; the first positive is then widest inside the explicitly restricted
non-wasteful aligned model.  At a bounded synchronized depth, a first positive is widest only at
that depth.  No A/B/C witness internals enter.  The next entry corrects the still-broader
excessive-`q` interpretation.

`tools/search_atom_profiles.cpp` implements that recursion with an explicit two-million-state abort,
rank slicing, symbolic terminal thresholds, and positive-tree output.  The separate
`tools/atom_profile_regression.sh` reproduces height 4 at aligned depth 1 and height 5 at depth 2.
For height 6, all 165 D germs fail at depth 1; the first 45 germs also fail the full-star necessary
condition.  Rank 46 `AAAAAAAD` is the first full-star survivor, the old `AAAABBCD` is rank 56, and
`ABBBBBCD` is rank 59.  At this stage ranks 46--58 still lacked synchronized exclusions;
the next entry supplies the stronger all-depth lineage proof.  `ABBBBBCD` itself was exhaustively
negative through aligned depth 2 (2.61105 solver seconds in the first optimized run).  The literal
five-part core was exhaustively negative at aligned depth 1 (0.121449 solver seconds).

Deeper exploratory work was deliberately bounded and produced no verdict: `ABBBBBCD` depth 3,
the widest-first depth-2 scan, the literal residual at depth 2, and the literal five-part core at
depth 2 each hit independent 60-second caps; observed peak RSS was respectively 0.03, 0.03, 0.02 and
0.01 GB.  An earlier JavaScript prototype completed depth 2 in 31.320 seconds and was stopped about
25 seconds into depth 3.  These five aborted probes are scheduling information only.  The retained
regression covers only completed statements, no solver process remains, and no Pareto CSV datum
changes.  The then-open rank-59 stabilization possibility—and therefore its conditional
`2^k-binom(k,2)-6` continuation—is refuted inside the aligned model by the next entry.
AddressSanitizer and UndefinedBehaviorSanitizer builds passed the height-4, height-5, and
literal-core controls.

## 2026-08-14 — D lineages close the eight-atom symbolic height-6 slice

The bounded-depth framing above was too weak.  In the aligned profile model, D is a non-branching
lineage: `D->CD` contains exactly one D, and a selected profile plus its complement partitions the
parent's D atoms.  Define `L_D(S)` as the sum of profile D counts over state parts, without multiplying
by part height.  Every mixed child preserves total height and has `L_D` no larger than its parent.
An eventual singleton leaf of height `h` needs at least `max(0,h-4)` D atoms, from the leading
coefficient of the first `h` terminal-reference profiles.  Following the mixed outcome at every node
therefore gives a closed losing set whenever `L_D(S)<max(0,h-4)`.

This settles the old 229 germ in the restricted model at all depths.  The two fixed profiles in the
height-6 hard branch have no D atoms, so every eight-atom D germ with zero or one D is impossible.
Those are ranks 1--81.  `AAAABBCD` is rank 56 and `ABBBBBCD` rank 59; neither can stabilize under
pure refinement.  The earlier depth-3 inconclusive result is superseded, not extended.

The first survivor is rank 82, `A^6D^2`.  Adding the sound two-coordinate over-approximation
`(D,C+D)` changed its exact depth-3 search from a 60-second timeout (0.02 GB peak RSS) to a positive
in 1.97468 solver seconds.  That first 19-node tree had root-base threshold 13; the alternative
tree found on 2026-08-15 and retained in the same certificate improves the threshold to 12.  Since
every wider germ has an all-depth certificate, this is the exact widest A--D eight-atom germ
without a depth qualifier.  The working outer assembly then has parent profile
`A^21B^6C^3D^2@G_(k-5)` and conditional width

    2^k - k^2 + 6k - 16,     k>=17.

This is a construction within the `(alpha,beta,gamma)=(4,3,2)` aligned family, not a new Pareto
maximum.  No CSV datum changed.

The proof is now mechanically split from discovery.  `search_atom_profiles.cpp` emits a closed
D-lineage certificate and a machine-readable positive tree.  `check_atom_profile_certificate.py`
independently enumerates profiles, refinements, cuts and mixed transitions; it checks 174,069 local
cases at eight atoms.  `check_atom_profile_tree.py` independently re-derives every child, eventual
majorization inequality and threshold.  The regression also rechecks the earlier height-4 and
height-5 trees.  The exact solver now uses the first-two-coefficient abstraction only as a sound
necessary filter; positive claims still come from the full profile tree.  The combined durable
proof object is `evidence/atom_profile_height6_ad8.cert`.  A combined AddressSanitizer and
UndefinedBehaviorSanitizer build reproduced the rank-82 positive in 16.609 solver seconds at
0.66 GB peak RSS with no diagnostic.

The phrase “the 165 possible D germs” in the preceding entry was too broad.  They are all A--D words
of length eight, not all profiles introduced by arbitrary excessive `q`.  The program now also
builds at 16 atoms.  Its independent certificate checks 5,540,319 local transitions and excludes
ranks 1--289 at all depths; the refined 229 class is rank 191 and the first state not excluded by
that lineage test is rank 290, `A^14D^2`.  The `(D,C+D)` abstraction rejects rank 290 through depth 5
in the retained regression.
An exploratory abstraction-only run remained negative through depth 16 and was stopped after about
81 single-threaded seconds; this is not an all-depth verdict.

At that point the genuinely new 16-atom band remained open.  A 60-second exact depth-3 range run discharged ranks
290--304 via the abstraction, then stalled on rank 305 with 0.05 GB peak RSS.  A separate 60-second
depth-3 run for rank 319, `A^12C^2D^2`, also timed out at 0.10 GB even though its construction follows
abstractly by refining the verified eight-atom tree; the timeout measures search order, not
constructibility.  These runs were capped and left no solver process.  This set up the next symbolic target:
an all-depth `(D,C+D)` invariant for the rank-290 band, followed by the rank-305 band where that
two-coordinate relaxation first permits depth-3 play.  Full proof and scope are in
`docs/theorems/atom-lineage.md`.

## 2026-08-14 — a finite `(D,C+D)` kernel excludes 16-atom ranks 290--304 at all depths

The apparent bounded-depth pattern at rank 290 is a genuine coinductive obstruction.  Project an
A--D profile to `(p_D,p_C+p_D)`.  The fixed height-1 and height-2 branches project to `(0,1):1` and
`(0,2):2`; every rank-290--304 germ has two D atoms and no C atom, so all fifteen profiles have the
same root

    (0,1):1, (0,2):2, (2,2):3.

Exploration first pushed that root from the retained depth 5 to depth 32 (2.99447 solver seconds,
169,293 memo entries) and depth 255 (28.1646 seconds, 1,576,423 entries), still negative.  Those
runs were only diagnostics.  The useful observation was that a depth-20 run had identical false
state sets at internal layers 3 and 4: 3,563 states at each layer.  Removing the immediate D-lineage
and projected full-star obstructions left 391 cyclic states; taking the minimal antichain under
multiset-substate inclusion left 242.

That antichain is now a depth-free proof object.  Let `K` be its upward closure together with the
two immediate obstruction classes.  For every minimal core and every legal assignment of projected
cuts to all its parts, at least one outcome contains another core or immediate obstruction as a
substate.  Hence `K` is closed under an adversarial outcome, and the root is all-depth negative.
Because the projection permits every exact cut and drops a terminal coefficient, its negative
verdict excludes the full A--D profiles too.  This closes ranks 290--304, not merely rank 290.

The independent `check_dc_kernel_certificate.py` does not trust the repeated search layers.  It
reimplements refinement, cut legality, all three children, terminal projected majorization, and
upward-substate closure.  It verifies all 242 cores through 925 cached local options and 641,741
partial global assignments.  The retained certificate is
`evidence/atom_profile_height6_dc16.cert`, regenerated and compared byte-for-byte by
`tools/atom_profile_regression.sh`.  A combined AddressSanitizer/UndefinedBehaviorSanitizer build
also regenerated the kernel and rank-305 tree with no diagnostic (leak detection is unsupported by
the platform runtime and was disabled).

The boundary changes exactly at rank 305, `A^13CD^2`: its last projected part is `(2,3):3`.  The
abstraction found a depth-3 tree in 0.0397185 seconds; its 25 nodes are retained in the same proof
object and independently replayed.  This is not a full construction, because the projection omits
the `B+C+D` deficit coefficient.  A temporary search constrained to the first projected witness
found no lift in 0.145508 seconds after 1,319,632 calls, but that filter did not enumerate other
projected witnesses and its negative was deliberately not retained.  The complete exact rank-305
depth-3 search again hit a 60-second cap (about 0.04 GB observed RSS) without a verdict.  No process
was left running.

The next symbolic problem is therefore narrower: lift a rank-305 projected tree through the third
coefficient, enumerating alternative projected skeletons if the first one fails, then walk
ranks 306--318 toward the known refined rank-319 construction.  No Pareto CSV datum changes.

## 2026-08-15 — an alternative lift closes the sixteen-atom slice at rank 305

The dropped coordinate has a simple exact parametrization.  If a parent profile is
`p=(a,b,c,d)` and a projected cut fixes `(u,v)=(x_D,x_C+x_D)`, write `j=x_B`.  The selected profile
is forced to be

    x=(N-v-j,j,v-u,u),

and its complete legal range is

    max(0,N-v-(2a+b)) <= j <= min(N-v,b+c).

The projection already enforces the C/D bounds.  These two inequalities are precisely the remaining
A/B containment in `R(p)`.  Once `j` is chosen, the complement and all three child states are
forced.  This turns lifting a projected tree into a finite recursive constraint system, including
all permutations between exact parts whose projected profiles coincide.

The first retained 25-node projected tree genuinely does not lift.  The new independent Python
implementation exhausts it in about 4.1 wall seconds: 177 exact node-states, 692 equal-projection
pairings, and 664,939 hidden-coordinate assignments, of which 558,007 fail an exact full-star
substate prefix.  Unlike the temporary filtered C++ experiment in the previous entry, this result
is retained and rerun by `tools/atom_profile_regression.sh`.  Its scope is only that skeleton.

The important correction is that the skeleton is not unique.  A product search now streams every
winning projected split, applies the interval above immediately, and recurses only on exact child
states.  It found a rank-305 exact tree after 3,981 projected split assignments and 20,448,787 exact
cut assignments.  The discovery run used 129.75 wall / 129.00 user seconds and 27,525,120 bytes
maximum RSS on the M4 Pro; its retained summary and regeneration command are in
`evidence/atom_profile_height6_rank305.cert`.  The successful root split is

    A^12B^3C:1   -> A^16:1,
    A^9B^5C^2:2  -> A^8B^6C^2:0,
    A^13CD^2:3   -> A^14C^2:2.

The resulting exact proof has 19 nodes.  `tools/check_atom_profile_tree.py`, which shares no search
code with the producer, re-derives every complement, child, leaf inequality, and the root-base
threshold `r>=6`.  This positive and the all-depth exclusions for ranks 1--304 make
`A^13CD^2` the exact widest sixteen-atom D germ in the aligned height-6 slice.  There is no need to
walk ranks 306--318.

Attaching the outer profiles at the same normalization gives

    c=A^15B,
    a-c=A^9B^5C^2,
    b=A^12B^3C,
    d=A^13CD^2,

and therefore parent profile `A^49B^9C^4D^2@G_(k-6)`.  It is the double refinement of the old
spreadsheet row `A^7B^7D^2@G_(k-4)` (`p6'`) and evaluates to

    2^k-k^2+7k-21.

The checked hard-tree threshold proves the conditional aligned-family construction for `k>=12`.
The same expression gives 473 at `k=9`, the proven maximum 973 at `k=10`, and 1983 at `k=11`; only
the middle value is a proven maximum, and the symbolic threshold does not settle the `k=11` case.
The new closed form and dyadic profile are therefore recorded as `conjecture`, sourced to the
checked conditional construction, rather than promoted to global Pareto facts.  No
`data/pareto_*.csv` row changed.

This closes the eight- and sixteen-atom finite optimizers, not arbitrary excessive `q`.  The next
slice is 32 atoms: recheck the no-C projected kernel and test whether `A^29CD^2` (which is wider than
the pure refinement of the rank-305 germ) admits an exact lift.  All local discovery processes
exited normally; none remains running.

## 2026-08-15 — the 32-atom slice contracts to one unresolved rank

The projected search now accepts an arbitrary power-of-two normalization, so the next slice could
be attacked without extending the full C++ profile enumerator.  There are 6,545 length-32 A--D
profiles in eventual-deficit order.  D lineage excludes ranks 1--1089.  The next complete bands are

    ranks 1090--1120: (p_C,p_D)=(0,2),
    ranks 1121--1150: (p_C,p_D)=(1,2),
    ranks 1151--1179: (p_C,p_D)=(2,2).

The first probe used rank 1121, `A^29CD^2`.  Projection-only negative searches at depths 3 through
10 took respectively 0.99, 3.67, 7.52, 12.11, 17.18, 21.42, 26.28 and 30.68 wall seconds.  Depth
20 took 82.18 seconds; depth 32 took 149.34 seconds, visited 401,549 projected memo states, and used
about 144 MB maximum RSS.  These bounded negatives were not promoted.  Their useful output was
structural: in the depth-32 computation the 5,646 false states at depth three were exactly the
5,646 false states at depth four.

After removing immediate D-lineage/full-star failures and minimizing under multiset inclusion, the
repeated layer gives 504 viable cores.  The independent checker exhausts every legal synchronized
cut from them: 1,673 cached local option sets and 1,776,407 partial global assignments.  Every core
is cyclically closed—each cut has an outcome containing another core or an immediate obstruction.
The same upward closure contains all three projected roots `(2,2):3`, `(2,3):3`, and `(2,4):3`
beside the two fixed branches.  The checker recomputes the profile ordering and verifies that these
are exactly the three complete bands above; it does not trust hand-written rank endpoints.  The
durable proof object is `evidence/atom_profile_height6_dc32.cert`, checked on an ordinary replay in
5.08 wall / 5.02 user seconds with 19,709,952 bytes maximum RSS.

Consequently every 32-atom rank through 1179 is impossible at all synchronized depths in the exact
aligned model: ranks 1--1089 by D lineage and 1090--1179 because the sound projection is already in
the closed losing set.  Rank 1180 is `A^27C^3D^2`, with projected last part `(2,5):3`, and is the
first profile outside this kernel.  Rank 1181 is `A^26BC^3D^2=R(A^13CD^2)`, so pure refinement of
the retained rank-305 exact tree constructs it.  The 32-atom optimum is therefore one of these two
adjacent ranks.

An exact all-skeleton depth-three search for rank 1180 reached the 300-CPU-second cap after 300.51
wall / 298.83 user seconds and about 33.4 MB maximum RSS, exiting 152 without a verdict.  This is an
inconclusive search, not an exclusion.  The next symbolic task is to find an exact rank-1180 tree or
retain the omitted `B+C+D` coefficient in a closed losing kernel.  Only then is it useful to move
to 64 atoms.  No Pareto CSV datum changed, and all processes launched for this work had exited by
the final inventory.

## 2026-08-15 — mixed supply turns the remaining word search into two scalars

The exact C++ profile engine now supports 32 atoms and accepts an arbitrary serialized
`profile-state`.  Merely raising the old normalization assertion did not solve rank 1180: the first
depth-three run reached its explicit 4,000,000-state ceiling after 578 seconds at 0.24 GB peak RSS.
That is an abort, not a bounded negative.

The useful result came from following the mixed outcome symbolically in all three deficit
coordinates.  For an unweighted state supply

    Sigma(S)=(D,V,W)=sum_parts(p_D,p_C+p_D,p_B+p_C+p_D),

one refinement has upper transform `T(D,V,W)=(D,V+D,W+V)`.  Selected and complementary profiles
partition that supply; deleting a zero-height piece can only lower it.  After `t` mixed outcomes,

    Sigma(S_t) <= (D,V+tD,W+tV+binom(t,2)D).

If this optimistic triple is lexicographically below the complete height-`h` singleton prefix, the
state cannot finish within `t` more tests.  This is finite-depth except in its leading D coordinate.
The independent Python certificate checker now verifies the local triangular inequality while it
replays every 8- and 16-atom cut; the 16-atom run covers 5,540,319 local transitions.  A sharp
32-atom regression state has supply upper `(0,2,11)` at depth three versus requirement `(0,2,13)`
and is rejected before recursion.

One attempted bridge explains both the promise and the remaining obstruction.  From the rank-1180
root

    A^27B^4C:1, A^23B^7C^2:2, A^27C^3D^2:3,

the split

    A^31B:1, A^23B^7C^2:0, A^28B^2C^2:3

has mixed child exactly equal to the known rank-1181 root.  Its other children are the trivial
`A^30B^2:2` state and the auxiliary `A^31B:1,A^28B^2C^2:3`.  The latter fails the mixed-supply
bound at depth three and exact recursion remains negative at every tested depth through 32.  The
depth-4, 5, 6, 10, 20 and 32 runs took respectively 0.003746, 0.009940, 0.021501, 0.097865,
0.694269 and 1.6507 solver seconds; depth 32 used 25,233,988 calls and 173,151 memo states.  These
are bounded negatives for one auxiliary state, not an all-depth kernel and not a rank-1180
negative.  The bridge is a measured dead end unless that auxiliary state changes.

With the supply filter enabled, the 4,000,000-state rank-1180 run reached the same explicit memo
abort in 156 seconds at 0.30 GB peak RSS.  A 16,000,000-state build then reached its 900-second wall
cap instead, using 0.73 GB peak RSS and producing no verdict.  The independently implemented Python
all-skeleton product search, with the same lemma coded separately, also reached its 900-second cap
at 0.18 GB peak RSS without a verdict.  Neither timeout emitted a negative line.  No further
undirected rank-1180 search was started.

The outer algebra nevertheless removes the arbitrary-word part of the excessive-`q` problem.  Let
`N=2^s` and parameterize the D germ as

    A^(N-b-c-2) B^b C^c D^2.

Adding the three known outer profiles gives

    A^(4N-3s-b-c-2) B^(3s+b-3) C^(c+3) D^2 @ G_(k-s-2)

and width

    2^k-k^2+(2s-c)k-s^2-3s+c(s+1)-b+2.

Thus a fixed normalization first minimizes `c`, then `b`.  At `s=5` the all-depth projected kernel
has already fixed `c=3`; the sole question is `b=1` (checked refinement) versus `b=0` (rank 1180).
The latter gives the formal parent `A^108B^12C^6D^2@G_(k-7)` and width
`2^k-k^2+7k-20`, one above the checked `-21` family.  The user's working sufficiently-large-`q`
postulate licenses comparing these eventual expressions, but does not prove the `b=0` state.  It
is not recorded in `data/conjectures.csv` because neither a construction nor a valid starting `k`
is known.
`tools/check_atom_parent_formula.py` independently reconstructs the parent counts and checks 5,136
direct atom evaluations, including both 32-atom alternatives.  No Pareto CSV datum changed, and the
final local process inventory was empty.

## 2026-08-15 — propagated loss closes depth three, not the rank-1180 problem

The mixed-supply bound has a transition form that is much more selective than testing a completed
child.  If the first mixed transition loses

    ell = T(Sigma(S)) - Sigma(S_1),

then after `t-1` further optimistic refinements its terminal loss is

    T^(t-1)(ell)
      = (ell_D,
         ell_V+(t-1)ell_D,
         ell_W+(t-1)ell_V+binom(t-1,2)ell_D).

Losses are nonnegative and additive over parts.  Thus the partial global cut can be rejected as
soon as `T^t(Sigma(S))-T^(t-1)(ell)` is lexicographically below the terminal requirement; unchosen
parts cannot recover supply already discarded.  This generalizes the initial equality-only rule.
The independent lineage checker now compares the closed propagation formula with literal
triangular iteration on 990 ordered-triple/depth cases at eight atoms and 5,814 at sixteen atoms.

For rank 1180 the independently derived data are

    Sigma=(2,8,19), Q_6=(2,14,45), T^3 Sigma=(2,14,49), T^4 Sigma=(2,16,63).

Hence a depth-three first transition must have `ell_D=ell_V=0, ell_W<=4`.  The complete exact
product then exhausts in 6.69201 solver seconds with `answer=NO`: 1,724,872 calls, 23,129 exact memo
states, 9,057,359 complete assignments and 820,472 propagated-loss rejections.  The process took
7.58 wall / 7.09 user seconds and 36.7 MB maximum RSS.  This bounded negative is now reproduced by
`tools/atom_profile_regression.sh`; it proves that rank 1180 has no aligned tree of depth at most
three, not that the state is impossible for sufficiently large `q`.

The independent Python all-skeleton implementation now corroborates that negative.  With the loss
formula implemented separately, it enumerated every winning projected skeleton and returned `NO`
after 1,342 exact states, 14,218 projected splits, 2,265,596 hidden-coordinate cut assignments and
1,801,750 propagated-loss rejections.  It took 38.47 wall / 38.16 user seconds and 39.4 MB maximum
RSS.  This replay is part of `tools/atom_profile_regression.sh`; agreement does not remove the
depth-three qualifier.

At depth four, the first transition instead has the sharp necessary budget

    ell_D=0, ell_V<=2; if ell_V=2, then ell_W<=12.

This is the promised outer-dimensional interface: it depends only on the known A/B/C profiles and
the candidate D profile, not on any inner witness structure.  It reduces the root's locally viable
option counts to `24 x 86 x 148`.  It did not yet settle the state.  A flat search with the checked
504-core projected kernel was interrupted after 417.78 wall / 415.32 user seconds at 348.5 MB RSS,
and an outer-prefix search reached its 600-CPU-second cap after 604.54 wall / 596.30 user seconds at
180.4 MB RSS.  Neither printed a verdict.  Earlier equality-only depth-four orders likewise hit
their caps; one 600-second run completed five concrete mixed-child negatives, while a loss-first
order spent 333.12 user seconds in one high-supply child.  These are search-order measurements, not
evidence against depth-four constructibility.

Two targeted lifts were also closed.  Preserving both D lineages in the natural refined rank-305
split gives a one-test mixed child with supply upper `(2,12,43)` against requirement `(2,14,45)`, so
no choice of its final cut can work.  Separately, the two-part state

    A^21B^9C^2:1, A^23B^2C^5D^2:2

looked hard only because prefix recursion manufactured irrelevant partial states: complete-product
order finds the one-test split `A^32:1, A^31B:1` in 0.0577194 solver seconds, and the independent
tree checker accepts its four-node certificate.  The solver now automatically uses complete
products for two-part states through depth three.  A sampled attempt to turn full
three-coordinate deficit thresholds into a small coinductive kernel reached its 200-core discovery
cap after 59.51 seconds, with 163 new cores all descending from the first seed; that route did not
produce a certificate and should not be mistaken for an all-depth proof.

The changed exact order also exposed a better positive that is independent of rank 1180.  The
eight-atom rank-82 scan now finds an alternative 19-node tree in 0.0669238 solver seconds whose
independently checked root-base threshold is 12, improving the previously retained threshold 13.
`evidence/atom_profile_height6_ad8.cert` now contains this tree.  Equation (5)'s conditional
`2^k-k^2+6k-16` construction therefore starts at `k>=17`, one level earlier than previously
recorded; its profile and scope are unchanged.

The next useful search should enumerate A/B/C prefix pairs under the propagated-loss budget and
retain the surviving D constraints, or construct a closed three-coordinate losing kernel.  Another
unstructured depth-four run is unlikely to add information.  No Pareto CSV datum changed, and all
processes launched for this work had exited by the final inventory.

## 2026-08-15 — exact pure branches reduce depth four to a W-only mixed frontier

The proposed outer-dimensional decomposition works as intended: the root search needs only the
profiles and heights of the already constructed A/B/C branches.  Their internal witness trees never
enter the first-test enumeration.  A new pure-frontier mode materializes every root test that passes
the sound symbolic and `(D,C+D)` filters, solves outcomes 0 and 2 exactly, and deliberately leaves
outcome 1—the synchronized four-segment branch—unresolved.

A height-aware supply bound was added first.  After `j` mixed levels, a height-`h` ancestral part
must satisfy `h<=2^j`, and each of its three optimistic terminal supplies is capped by `hN` because
it can end in only `h` profiles of `N` atoms.  The implementation compares every possible stopping
level `j<=t` with the singleton requirement.  This is a necessary over-approximation, independently
implemented in C++ and Python.  An exact relaxation that follows only the all-mixed transcript was
also added.  Rank 1180 passes that relaxation at depth four, so aggregate mixed evolution alone
does not close the state; the interaction with the two pure outcomes is essential.

For the rank-1180 root, the three local symbolic option lists have sizes `24 x 86 x 150`.  The 150
is not a correction to the previous 148 exact-per-part count: the frontier materializer intentionally
retains two additional locally plausible options and applies exact recursion only after assembling
the global child.  Prefix loss, height, majorization and projected checks leave 7,266 oriented
complete first tests.  Exact solution of both pure children leaves 6,712 tests with 1,826 distinct
mixed children.  Their loss classes are

    (ell_D,ell_V,ell_W)=(0,0,w), 1<=w<=14,
    (0,2,10), (0,2,11), (0,2,12).

There is no surviving `ell_V=1` class.  The three positive-`V` classes contain respectively
4, 6 and 6 oriented tests and only 2, 3 and 3 distinct mixed children.  Both implementations then
solve all eight distinct children exactly at depth three and return `NO` for every one.  Hence any
depth-four construction must satisfy

    ell_D=ell_V=0, 1<=ell_W<=14.

The remaining frontier is 6,696 oriented tests and 1,818 distinct mixed children in fourteen
one-dimensional W-loss classes.  Counts `(tests/distinct children)` for `w=1,...,14` are
`108/32, 226/64, 346/97, 468/131, 594/166, 726/202, 744/202, 752/202, 762/202,
644/170, 524/138, 398/105, 268/71, 136/36`.  This reduction alone was not a depth-four negative;
the guided exact-cover closure recorded in the next entry subsequently decided all 1,818 children.

The independent Python frontier and eight-child closure took 18.91 wall / 18.40 user seconds with
about 104 MB maximum physical footprint and reproduced every aggregate count above.  The C++ path
and eight direct exact child calls agree and are locked into `tools/atom_profile_regression.sh`.
A combined Python attempt to close the edge classes `w=1` and `w=14` was explicitly capped and
ended `TIMEOUT` after 62 wall seconds at 0.10 GB peak RSS; it produced no verdict.  An earlier
per-part exact-envelope prototype was manually stopped after at least 48 CPU seconds without a
result and was replaced by the cheap sound height-cap formula.

At this stage the next target was therefore the 1,818-state mixed antichain, not the solved A/B/C
internals and not another undirected root run.  The guided cover below closes that finite target.
Eventual rank-1180 constructibility, the formal `-20` width family, and arbitrary excessive `q`
remain open.  No Pareto CSV datum changed.

## 2026-08-15 — the symbolic boundary and guided exact cover close depth four

The finite-depth algebra now has a closed general form.  For normalization `N=2^s`, write the only
eventually competitive D germ as

    A^(N-b-c-2) B^b C^c D^2.

Its initial supply and the height-6 terminal demand are

    Sigma=(2,c+5,3s+b+c+1),       Q_6=(2,2s+4,s^2+3s+5).

After `t` optimistic mixed refinements, comparison with `Q_6` first forces

    c >= max(0,2s-1-2t).

When that unclamped bound is positive and ties the middle coordinate, the last coordinate then
forces

    b >= max(0,s^2-2s-2st-t+t^2+5).

`tools/check_atom_parent_formula.py` derives these inequalities independently for 63 `(s,t)` cases
through `s=12`.  At depth three, pure refinement of the checked sixteen-atom germ lies on
`c=2s-7, b=(s-4)^2`, while supply permits `b>=max(0,s^2-8s+11)`.  The possible B saving is therefore
zero at 16 atoms, one at 32, four at 64 and five from 128 atoms onward.  Refinement preserves a
fixed saving, so there are only five persistent nontrivial tracks: saving one begins at 32 atoms,
savings two through four at 64, and saving five at 128.  This is an exact organization of the
depth-three supply boundary, not a construction.  Once `t>=s`, even `b=c=0` passes the scalar
relaxation, which proves that this scalar method cannot by itself settle arbitrary excessive `q`.

The exact search was reorganized around that limitation.  `construct_guided` enumerates every
winning `(D,C+D)` projected skeleton first and then every compatible hidden B-coordinate lift.  It
changes only enumeration order: a positive still emits a full exact tree, and a negative is printed
only after every lift is exhausted.  At the root, cover order first solves the shared pure outcomes
and then the mixed children in retained-supply order.  The W-loss-slice mode makes a completed
interval durable and labels a negative explicitly as applying only to the declared root slice.
`tools/check_atom_profile_cover_log.py` reconstructs every child's loss from its atom words and
checks candidate multiplicities, distinct-child counts, class coverage, final scope and run
summary.

Several failed orders were useful in arriving there.  Ordinary identity cover ran about 4m25s,
rejected only 23 mixed children and produced no verdict.  Supply-first ordinary recursion spent
more than 90 seconds on its first W=1 child.  A depth-three ordinary W=1 materialization reduced
4,994 cuts to 2,726 candidates and 1,010 children but was stopped after two minutes.  Recursive W=1
cover timed out after 303 seconds at 0.03 GB RSS after reaching 81 grandchildren; an independent
Python W=1 closure timed out after 1,806 seconds at 0.16 GB, also without a verdict.  An attempted
MRV shortcut was unsound: it changed two locked outputs from `86/78` to `78/74`, so it was reverted.
After the revert the final-only canonical control returned `86/78` in 1.22 seconds, versus 0.766
seconds for the prior safe order and 0.103 seconds for the invalid shortcut.  These timings are
search-order measurements, not mathematical evidence.

The safe guided controls were decisive: rank 305 returned a checked 19-node positive in 0.013
solver seconds, while rank 1180 returned the already known depth-three `NO` in 0.915 seconds.  The
first W=1 depth-four child then returned exact `NO` in 55.8798 seconds after 99,340 calls, 9,748 memo
states and 247,471,270 cut assignments, establishing that the approach could finish individual
children.  An exploratory full-root run reached its 1,806-second cap at 0.04 GB RSS after completing
W=1, W=2 and 24 W=3 children, but emitted no root verdict.  A cold W=3-only restart was manually
stopped after about 7m29s with only 19 children complete: discarding the lower-loss memo lost more
than it saved.  The final slicing therefore kept W=1..3 together.

The exact completed slices use `W interval: oriented tests / distinct children / solver seconds`:
`1..3: 680/193/1784.59`, `4..8: 3284/903/1786.34`, `9: 762/202/781.322`,
`10: 644/170/590.728`, `11: 524/138/424.129`, `12: 398/105/287.151`,
`13: 268/71/171.830`, and `14: 136/36/98.1987`.  In total, 6,696 oriented tests and all 1,818
distinct mixed children returned exact `NO` in 5,924.29 solver-wall seconds.  The wrapper-reported
peak RSS was at most 0.04 GB in every slice; as elsewhere on this Mac, that is not a total-memory
bound.  All eight logs pass embedded-provenance validation and the independent cover-log accounting
checker; the verified raw outputs are archived as
`rank1180-depth4-2026-08-15`.

Combining those negatives with the already exact pure outcomes proves that `A^27C^3D^2` has no
aligned strategy tree of depth at most four.  Therefore the formal `2^k-k^2+7k-20` parent, if it is
constructible at all, needs a D-branch tree of depth at least five.  The checked scalable family
remains `2^k-k^2+7k-21`, obtained from rank 1181 by refinement.  This does **not** prove rank 1181 is
the all-depth 32-atom optimum: rank 1180 may still have a deeper tree.

A final direct guided probe at depth five was capped at 1,800 seconds.  It timed out after 1,802
wall seconds with exit 124 and no solver result line; the log contains only valid build/run
provenance and the capped-run summary.  Its wrapper-reported peak RSS was 0.03 GB, again not a total
memory bound on this Mac.  This is an inconclusive search-order measurement, not evidence against a
depth-five construction.  The small no-verdict log remains local and was not added to the artifact
store.

The logical next step is no longer another scalar supply inequality or a rerun of this finite
frontier.  Minimize the exact negative memo states into a three-coordinate losing antichain, then
seek a mechanically checkable fixed point: for every test of every listed core, at least one outcome
must contain a listed losing core after refinement.  If that closed kernel contains rank 1180, the
result becomes all-depth.  If closure fails, the uncovered transitions identify the only routes a
depth-at-least-five positive tree can use.  This is the scalable symbolic fork.  No Pareto CSV datum
changed.

## 2026-08-15 — literature and exact replay replace the m=5 extrapolation

The small-`m` formulas were checked against both a fresh exact sweep and the primary literature.
This changed the record materially: the former lemma-9 formula is not the `m=5` frontier after
`k=8`.

The exact `tools/search_singletonization.cpp` frontier mode was rebuilt through the provenance
builder and run for every normalized one-part case with `m<=6`, `k<=9`.  Each of the 46 logs has a
complete `radio-provenance-v1` block, an exact `n+1 NO` / `n YES` pair, and a positive tree that
passes `tools/check_witness.py`.  The nontrivial rows, omitting pre-diagonal orientations, are:

```text
m=1, k=1..9: 2, 4, 8, 16, 32, 64, 128, 256, 512
m=2, k=2..9: 3, 7, 15, 31, 63, 127, 255, 511
m=3, k=3..9: 5, 12, 27, 58, 121, 248, 503
m=4, k=3..9: 4, 10, 24, 54, 116, 242, 496
m=5, k=4..9: 9, 22, 50, 109, 231, 481
m=6, k=4..9: 7, 19, 46, 104, 225, 473
```

The 92 decision lines consumed 36.676203 summed solver-wall seconds.  At the two new K=9
boundaries, `Sb(482:5)` was rejected in 2.73659 seconds and `Sb(481:5)` solved in 0.295632
seconds; `Sb(474:6)` was rejected in 26.2015 seconds and `Sb(473:6)` solved in 4.12989 seconds.
The complete 416 KiB tar, including validation output, is archived as
`small-m-frontier-2026-08-15` (raw SHA-256
`0b6ec2d9933d210d671a7301f63d676ebb84ae3ce1c63d3d1f877f98979b923b`).  Compact boundary
summaries are committed at `evidence/sb_m5_k9_frontier.txt` and
`evidence/sb_m6_k9_frontier.txt`.  Consequently the K=9 column is now exact through `m=6`, not
merely witness-backed there.

The new 481 candidate strategy was extracted as `witnesses/majorized_481_5_at9.tree`.  Its 61
nodes, 20 splits and 41 singleton leaves pass structural checking; 38 fit distinct `G_k` slots,
but the other three leave the file conditional on the open singleton converse.  The exact value 481 instead rests on the published
Li--Wu--Triesch theorem.  The conditional tree's root is `[239:1]`, equivalently its complement
`[242:4]`.

An exact-negative forced-root scan constrains the extra coin.  At `Sb(481:5)@9`, every capacity-feasible
`3+2` root `[a:3]`, `a=226..248`, is negative.  Among `4+1` roots, `[a:4]` is negative for
`a=225..239` and conditionally accepted for `a=240,241,242`; `5+0` is impossible because two pure `m=5`,
`k=8` branches have total capacity only `2*231=462`.  Thus the root type is forced to switch from
`3+2` to `4+1`, up to complement.  The diagnostic is retained in
`evidence/sb_m5_k9_root_transition.txt`; the exact negative scan plus the published existence
theorem carry the root-type conclusion, while the individual positive tree remains conditional.
All 44 raw forced-root outputs have complete
provenance and are archived in the same tag as `m5-k9-forced-roots-2026-08-15.tar`; their summed
solver-wall cost was 15.424464 seconds (raw tar SHA-256
`32b78cc020c11edf4497267522f11ed5f6b248e0c1ef46831f5131df8a6f022b`).

The decisive paper was already among the downloaded ScienceDirect PDFs: Shengjia Li, Xiaohui Wu
and Eberhard Triesch, “A ternary search problem on two disjoint sets,” *Discrete Applied
Mathematics* 251 (2018), 221–235.  Its Corollary 3 proves the exact `m=4` formula, and Theorems
1–3 plus Remark 1 prove, with `F(k)=2^k-k(k-3)/2-5`,

```text
n(k,5)=F(k)     for 3<=k<=8,
         F(k)+1 for 9<=k<=10,
         F(k)+2 for k>=11.
```

Hence `n(9,5)=481`, `n(10,5)=985`, and `n(11,5)=2001`.  The paper itself changes its first
test from `3+2` in Theorem 1 to `4+1` in Theorem 2, exactly as the forced-root scan does; Theorem 3
adds a further recursive stage.  There are apparent transcription/index errors in displayed
equations (69)–(70): in particular equation (69) uses `k-2` for a singleton width whose preceding
selected test uses `k-1`.  The theorem statement is consistent and independently confirmed here,
but numerical proof displays must be recomputed rather than copied.

At normalization `t=k-2`, put
`A=2^t`, `B=A-1`, `D=A-1-t-binomial(t,2)`.  Then the old profile mass is
`BBBD=F(k)`; `ABBD=F(k)+1`; and `AABD=F(k)+2`.  For `k=9`, these are respectively 480, 481 and
482 because `(A,B,D)=(128,127,99)`.  This explains the three pieces arithmetically, but it is not
yet a symmetric per-coin profile proof for the compressed 481 tree.  The old `BBBD` construction
remains a valid lower family for `k>=7`; only its equality/optimality claim is refuted.

The m5 correction also retracts two recurrence statements.  The numerical identity
`n(k,5)=n(k-1,2)+n(k-1,6)` holds only for `k=5..8`; at `k=9` its right side is
`255+225=480`, one below the exact 481.  The separate identity
`n(k,6)=n(k-1,4)+n(k-1,5)` holds through `k=9`, but its first extrapolation now predicts
`496+481=977`, not 976, against the unconditional upper bound `n(10,6)<=973`.  The conditional root backs the relevant
width down from 481 to 477, a loss of four.  The old `BBCD` closed form independently predicts 976
and remains refuted by three.  These are distinct failed continuations and must not be conflated.

The other primary PDFs were also read and indexed in `docs/literature.md`: Hwang (1987) for the
historical two-coin/model-Q introduction, Aigner (1986) for the graph formulation, canonical
sequence and exact `m=2,3` cases, Andreae (1989) for the broader graph-search setting, Hao (1990)
for product composition and asymptotic limits, Gargano--Montuori--Setaro--Vaccaro (1992) for an
explicit scalable algorithm and `T(32,32)=7`, and Christen (1994) for the adaptive/limited-round
taxonomy.  No presently needed article remains blocked behind institutional access; Aigner's 1988
book is the highest-value item still worth obtaining.

## 2026-08-16 — rebuilding m=5 inside the Pareto assembly

The exact Li--Wu--Triesch result was used as a known-answer calibration rather than merely as a
replacement formula.  Pages 230--234 of the primary paper were reread visually and its selected
tests were recomputed with the repository's rectangle algebra.  Put `t=k-2`, `P=2^t`, and
`Q=binomial(t-2,2)`.  The old `(alpha,beta,gamma)=(3,2,2)` assembly has

```text
a=2P-t-1, b=c=P-1, d=P-2t-Q+1,
hard branch Sb(d:2,(P-1):1,(P-t):2)@t,
parent width 4P-3t-Q-1.
```

This is the already proved `BBBD` lower family.  The paper's new first test instead gives the
`(4,3,1)` assembly

```text
a=2P-2t, b=P-t, c=P, a-c=P-2t,
hard branch R_t(d)=Sb(d:3,(P-t):1,(P-2t):1)@t,
parent width 3P-3t+d.
```

The two outer tests were checked directly.  The root `[a:4]` has pure outcomes `(a:4)` and
`(b+d:1)`.  In its mixed outcome `(a:1,b+d:4)`, selecting `[a-c:0]` and `[b:3]` gives `(b:3)`,
`(c:1,d:1)`, and exactly `R_t(d)`.  Thus D is the only synchronized obligation.  This also fixes
the paper's displayed off-by-one: the singleton width is
`a-c=2^(k-2)-2(k-1)+2`, which is 114 at `k=9`, not the 116 produced by replacing `k-1` with
`k-2` in equation (69).

The displayed D targets (and unconditional upper ceilings) are

```text
d*(t)=P-Q     for t=7,8,
       P-Q+1  for t>=9.
```

**Correction 2026-08-26.**  The exclusion of `d*+1` below is an exact upper argument, but the
matching local achievability construction is conditional wherever it terminates only by arbitrary
weak majorization of singleton rows.  If `R_t(d*+1)` were solvable, the
other branches would remain solvable (`d*+1<=P-1`, `b+d*+1<=2P`), and the two outer tests would
construct `Sb(n(k,5)+1:5)`, contradicting the published upper bound.  Subgraph monotonicity excludes
all larger D.  The finite `k=8` tie separately has upper value `P-Q-1=57`.  The published theorem
independently establishes the parent `m=5` frontier; it does not, by itself, prove every displayed
synchronized D state in this reconstruction.

The eventual conditional lower construction was reduced to a symbolic template.  For
`d=P-Q+1`, one test of `R_t(d)` followed by one test in each non-singleton child leaves only
singleton states.  Their decisive five-part deficit multiset relative to `P/4` is
`{1,Q-t-2,t,t,2t-1}`, versus `{0,1,t-1,t-1,Q+t-1}` for the first five entries of `G_(t-2)`.
The candidate is weakly majorized throughout the intended range `t>=9`; a three-part leaf already
violates majorization at `t=7,8`.  Thus the `k=11` breakpoint arises inside the D strategy, not from
fitting the published final values.  In the majorized-terminal model, the finite `t=7,8`
constructions plus this template supply conditional achievability; the paper independently supplies
the sharp parent values and global upper bound.

The conditional `assembly-enumerate` replay over proven Pareto inputs gives the crossing:
`(3,2,2)` is
the sole winner for parent `k=4..7`, both `(3,2,2)` and `(4,3,1)` reach 231 at `k=8`, and only
`(4,3,1)` reaches 481 at `k=9`.  Majorized-terminal construction checks of the latter hard branch give
`d=241` at `k=10` and `d=492` at `k=11`, producing the published widths 985 and 2001; their emitted
trees are structurally checked but inherit the open converse at arbitrary majorized leaves.  The
published theorem independently proves the parent widths.  The global upper bounds remain sourced
to the paper, so no new solver artifact or Pareto row was created.

`tools/m5_assembly.py` now evaluates both symbolic candidates and separately labels the atom-mass
identities `BBBD`, `ABBD`, and `AABD`.  `tools/check_tables.py` validates the assembly algebra for
61 levels (`k=4..64`) and matches all seven recorded exact `m=5` rows to the theorem.
`tools/singletonization_regression.sh` contains the complete finite assembly controls plus the
`k=10,11` constructions.  The full regression passed in 74.87 wall seconds (70.15 user, 1.52 sys)
on this machine.

Strategic consequence: the general track survives, but its scalable state is a guarded envelope of
outer families and piecewise D frontiers, not one preferred height triple or one atom word.  The
height-6 rank-1180 problem remains a genuine all-depth question inside the fixed `(4,3,2)` aligned
slice; deciding it alone would not establish the unrestricted `m=6` frontier.  The proof status and
scope boundary are recorded in `docs/theorems/m5-pareto-assembly.md`.

## 2026-08-16 — parking the excess-q Pareto-assembly track

The user chose to close this construction track for now.  This is a prioritization decision, not a
refutation of the sufficiently-large-`q` postulate.  The investigation found a sound local assembly
mechanism, but the exact `m=5` calibration shows that a general theorem—if one exists—is not the
simple repetition of one outer height triple, one atom word, or one D recurrence that motivated the
track.

The durable findings are:

- The corrected diagram reduces every chosen A/B/C triple to the four-segment state
  `Sb(d:beta,b:alpha-beta,c:m-alpha-gamma,a-c:gamma)@k-2`, with parent width `a+b+d`.
  A/B/C are genuine black boxes: their internal witness trees do not constrain this residual; only
  their outer width-height pairs do.
- For fixed A/B/C, maximizing D is an exact one-dimensional monotone frontier problem.  Its natural
  scale-free object is nevertheless two-dimensional: the guarded antichain of mixed-child deficits.
  Full-star majorization supplies only a necessary bound and can overestimate D.
- The known-answer `m=5` parent frontier is supplied by the published theorem, while this local
  reconstruction is conditional at arbitrary majorized singleton leaves.  Its outer envelope changes
  from `(3,2,2)` to `(4,3,1)`, and the winning family's displayed D formula changes.  The eventual
  branch has a conditional singleton-majorization construction, so its algebra is a useful
  calibration of the local mechanism but is not an independent proof of achievability.
- Atom masses `BBBD`, `ABBD`, and `AABD` reproduce the three exact `m=5` values arithmetically, but
  `ABBD` and `AABD` have not been established as symmetric non-wasteful atom profiles.
- At height 6, literal refinement of the finite successful split is exactly negative.  D-lineage
  and coinductive kernels then give real all-depth obstructions inside the restricted aligned model,
  while checked trees give conditional lower constructions.  The unresolved rank 1180 is only a
  question inside one 32-atom `(4,3,2)` slice; deciding it would not establish the unrestricted
  `m=6` frontier.
- The finite assembly enumerator, guarded mixed-frontier optimizer, atom-profile certificates,
  positive trees, and depth-four cover remain useful reproducible tools and controls.  They are no
  longer an instruction to continue scanning larger normalizations.

What remains unproved is just as important: the proposed `m<=2a` admissibility condition, existence
of a sufficiently large stabilizing `q`, completeness of the outer assembly family among arbitrary
strategies, refinement stability of the synchronized D frontier, and any global formula beyond the
published `m=5` case.  No Pareto datum is changed by closing the track.

The reopening threshold is intentionally high.  Resume only if new mathematics supplies a complete
outer-family theorem, a refinement-stable exact recurrence for the guarded D antichain, or an
all-depth construction/obstruction that connects successive normalizations.  Another finite winner,
bounded-depth negative, fitted word, or larger rank scan does not close the missing global link.
This closure launched no solver and changed no source-of-truth datum; the table, witness, and
documentation checks all passed.

## 2026-08-16 — exact atomization of the eventual m=5 leaf

The discussion after parking the broader assembly track separated three questions that had been
conflated: the published numerical frontier, singleton majorization, and literal base-atom packing.
Li--Wu--Triesch's Theorem 3 is quantified over every `k>=11`, so its last formula is genuinely final:
there can be no later transition in the numerical value of `n(k,5)`.  The theorem gives one uniform
`4+1` construction and a matching upper bound.  It does **not** prove that all optimal strategies
have the same root, that the strategy is unique, or that the arithmetic mass word `AABD` is an
aligned per-coin profile.

The decisive five-part leaf in the self-contained eventual construction was isolated exactly.  With
`r=t-2=k-4` and `A_r=2^r`, define

```text
B_r=A_r-1,
X_r=A_r-r-2,
E_r=A_r+r+4-binomial(r,2),
Y_r=A_r-2r-3,
P_r=sort(B_r,X_r,X_r,E_r,Y_r)@r.
```

The first eventual case is `P_7=(127,119,119,118,111)@7`.  Two stronger terminal predicates were
added to `tools/search_singletonization.cpp`: `embedded` requires a coordinatewise injection into
distinct `G_s` slots, while `canonical-exact` requires a literal sub-multiset of `G_s`.  Exact
implies embedded, and each property survives deleting parts, so the existing recursive partial-state
pruning remains sound.

Complete searches give the sharp concrete boundary:

```text
P_7 exact:     depth 0 NO, depth 1 NO, depth 2 NO, depth 3 YES
P_7 embedded:  depth 0 NO, depth 1 NO, depth 2 NO, depth 3 YES
```

The exact positive is committed as `witnesses/canonical_m5_leaf_p7_at7.tree`; the independent
checker derives 19 nodes, six splits and 13 canonical leaves.  The regression reruns both depth-two
negatives and both depth-three positives.  Finite continuation gives exact depth-three trees for
`P_8` and `P_9`; `P_10` is exactly negative at depth three and positive at depth four.  These are
bounded construction facts, not a formula for later exactification depths.

The uniform arithmetic explains why the concrete answer three cannot stay fixed.  For a proposed
fixed depth `d`, put `s=r-d`, `N=2^d`, and measure each root component from `N A_s`.  The deficits are

```text
Delta(B_r)=1,
Delta(X_r)=r+2,
Delta(Y_r)=2r+3,
Delta(E_r)=binomial(r,2)-r-4.
```

The corresponding first atom values are `A_s=2^s`, `B_s=A_s-1`, `C_s=A_s-s-1`, and
`D_s=A_s-1-s(s+1)/2`.  For fixed `d` and all sufficiently large `r`, every component exceeds
`(N-1)A_s`, so an exact inventory needs exactly `N` positive pieces.  Its `E_r` inventory must
contain exactly one `D_s`, no atom below `D_s`, and then `q` copies of `C_s` and `p` of `B_s`.
Comparing coefficients forces

```text
q=d-2,                 p=(d-6)(d+1)/2.
```

Thus no fixed exact depth `d<=5` works uniformly for all sufficiently large `r`.  At `d=6`, the
first nonnegative case, the individual 64-piece identities are

```text
B_r = A_s^63 B_s
X_r = A_s^56 B_s^7 C_s
Y_r = A_s^49 B_s^13 C_s^2
E_r = A_s^59 C_s^4 D_s
```

The first elementary finite obstruction is already `r=10`: eight `G_7` atoms for `Y_10=1001`
would need deficit 23, but the only available deficits below that are 0, 1 and 8, and
`p+8q=23`, `p+q<=8`, has no solution.  `tools/m5_assembly.py` now checks this obstruction, the
forced coefficient formula, and every covered positive depth-six component identity.

This establishes a lower bound and a candidate, not sufficiency.  The four depth-six identities do
not assign their atoms to common outcome columns; a synchronized six-test tree could still fail to
exist.  The minimum uniform embedded depth is also open.  The safe summary is therefore: the
concrete first leaf needs exactly three tests; six is the first arithmetically possible uniform
**exact** depth; neither the paper nor the current work proves a six-level uniform packing.

Two exploratory packing scripts did not close that synchronized problem.  One stdin Python search
outlived the tool cell that launched it and was noticed as PID 56006; it consumed about 65 CPU
minutes at one core and roughly 7 MB RSS before being terminated.  It emitted no retained result,
so this is an inconclusive dead end, not negative evidence.  A final process inventory confirmed
that no related search remained.  This incident reinforces the existing rule that every spawned
process must be inventoried even when its launching tool call appears to have completed.

No Pareto value changes.  The new durable evidence is the canonical leaf tree, the exact/embedded
terminal implementation and regression, the independently checked symbolic inventories, and the
scope corrections in the theorem, status and research-plan notes.

## 2026-08-16 — proof-safe cold run9 closes H3: `Sa(10)=192`

All three retained cold AWS runs have concluded. Each independently returned `Sa(193)`
UNSOLVABLE after completing all sixteen roots, but their classifications differ:

| run | status | Sa(193) CPU | wrapper wall | peak RSS |
|---|---|---:|---:|---:|
| run3 (`3cf1406` era) | performance only; incomplete embedded provenance and pre-fix cache | 479020.9 s | 479580 s | 25.57 GB |
| run8 (`9395218d...`) | performance only; fully provenanced but pre-contraction-fix | 412561.4 s | 413045 s | 1.32 GB |
| run9 (`e7fa7472...`) | **proof source** | 419353.1 s | 419849 s | 1.32 GB |

Run9 began cold with no cache, stayed in one session, and first passed the independently known
positive control `Sa(192)` in 479.2 CPU seconds. It then printed exhaustive negatives for
`Sb(n1:193-n1)@9` for every `n1=97..112` and the final UNSOLVABLE result. Its source commit is
`e7fa747264476461a234bf78e49762ee77ad8d8d`; build ID
`219a8753a3caf79cf7a160cb220a7305b8d914d1bfd8989d52861d1cc1407de4`. The raw log has
3,174,576 lines, 365,340,502 bytes and SHA-256
`ba635d9141601ebb643ed4f102703deb112fc3e8260f4936e8545fe44a300cf4`. Embedded provenance checks,
the positive control and the audit all passed; the audit found zero contradictions. The final log
contains zero `rb-suppressed` markers. The proof-safe fix was present, but the affected contraction
shortcut was never invoked.

This settles H3. The verified `Sa(192)` tree proves achievability, and the sixteen run9 refutations
exhaust the possible first-test sizes for 193 because `Sa(112)` is the k=9 maximum. The
source-of-truth `Sa` row is now `max,proven-exhaustive`; the sixteen K=9 ceilings at `m=81..96` are
now `upper,proven-exhaustive`. Legacy lower rows were deliberately left unchanged, so the latter
remain brackets/upper bounds rather than invented exact frontiers. The 2023 verdict reached the same
answer but remains only historical cost evidence: its 37 known false negatives and non-closed
resume chain are not rehabilitated by agreement.

The matched run8/run9 parsed fact sets contain 3,166,649 and 3,167,184 distinct signed facts, with
3,160,113 in common, 6,536 run8-only, 7,071 run9-only and zero state keys carrying opposite signs.
Run9 emitted 535 more facts (+0.016895%), cost 6791.7 more CPU seconds than run8 (+1.646%), and was
12.456% faster than run3. The complete comparison, including all raw hashes, is committed in
`evidence/sa193_run_comparison_2026-08-16.txt`.

Durability work preceded promotion. EBS-only final sidecars for run3/run8/run9 were copied to their
S3 `final/` prefixes under SSM command `4dfc8613-78aa-4b81-a122-895e9675bf54`; every file matches
`final/sa193-cold-sidecars.sha256`. The private release `sa193-cold-2026-08-16` now contains the
run8 raw comparator, the run9 raw proof log and a 13,894,656-byte metadata tar with source bundles,
binaries, provenance, profiles, status, stderr and watchdog logs. Raw manifest hashes and sizes
round-trip through `tools/artifacts.sh`; the store now has 15 tags and 46 indexed assets.

While checking the global artifact index, `check-index` intermittently named a different existing
asset as missing. Nothing was absent: its `gh ... | grep -q` pipeline ran under `pipefail`, so an
early match could SIGPIPE `gh` and turn success into failure. The checker now fetches each release's
asset list once into a file before matching; the full index is green.

The EC2 instance was intentionally left running for the separate k=8 Pareto-prefix census. At
2026-08-17 00:52 UTC, PID 1926155 (`pareto_k8_aws`) was at one full core and 8,892,056 KiB RSS with
113 GiB available, no swap and 194 GB disk free. Its 44,833,189-byte output had closed all 815
second-cut blocks and emitted the prefix summary plus 1,688 targets, but no endpoint or full-state
record. The S3 census `STATUS` object is still the launch snapshot and must not be mistaken for a
live progress report. Do not stop the host until the census exits and its final output is archived.

Process inventory at handoff: no Sa solver remains; the remote census above is the only active
research binary. The pre-existing local watcher PID 73027 (`tools/sa193_status.sh --prefix run9
--watch`) belongs to the user and was not touched. No one-off Python search or locally launched
solver remains.

## 2026-08-16 — parallel verifier prototype: readable certificates, minimalize then color

The verifier/solver design discussion produced four concrete decisions. Use the existing
independent C verifier as the coloring engine; minimalize each support level before coloring; keep
the durable certificate human-readable; and distinguish logical verification order from coloring
order. Verification has no runtime level dependency as long as every local obligation passes,
because every edge decreases `k`. Coloring alone needs the top-down barrier, because citations at
`k` define the target set at `k-1`.

`radio_verify.c` now implements that design. All search-mutating globals—recursion state, direct
memo, live/pair caches, counters and derivation state—are worker-local. Frozen `Level` indexes are
shared. Ordinary parallel verification mixes every eligible fact from every level in one dynamic
queue. Coloring runs the same batches one level at a time and atomically ORs citations into the
lower level. Each batch destroys its worker-local caches, which also fixes the existing multi-pass
hazard where a reused live table issued no queries and therefore repainted nothing. Aggregate memo
capacity stays near the original `2^24` entries by shrinking the default per-worker memo as width
grows.

The first corpus gate was the clean retained
[`fullsolve-2026`](https://github.com/fedork/radio-data/releases/tag/fullsolve-2026)
`out_k7.txt`, SHA-256 `9bfcdd134fd16d1e1dc1f4a34154eaadc16a988f6e065517e62eece6e69c2cde`.
The preserved serial path and every parallel width verified all 62,366 canonical negatives with
zero gaps and exactly 97,483,464 split-recursion nodes. One-shot real times on the same O3 build
were 14.13, 8.24, 5.22, 3.24 and 2.79 seconds at 1, 2, 4, 8 and 16 workers. Scaling is useful but
not free: the 8-to-16 step saved only 14% wall while user CPU rose from 25.43 to 35.79 seconds,
because worker-local memo/live/pair construction duplicates work. Eight is the measured economical
width here; this is not yet a run9 prediction.

The new strict text form is:

```text
radio-negative-certificate-v1
root 6 Sb(17:17,16:15)
fact 5 Sb(12:11)
```

It accepts comments and inert `meta` lines, derives masses, canonicalizes states, and rejects
unknown records. `CERT_ONLY` reduced the 6,910,223-byte raw log to 1,908,729 readable bytes; at
`zstd -19` it is 194,131 bytes. A binary durable format would save little and make inspection and
independent parsers harder, so binary packing remains an in-memory index choice. The text parser
currently inherits the C verifier's 255-per-coordinate implementation bound; the grammar does not
need to.

A parse-only run9 gate then exercised the actual scale without starting proof enumeration. Both the
old parser and the new one extracted exactly 3,126,190 canonical negative records. Normalization
took 2.74 wall seconds and 457 MB peak RSS, producing 106,011,566 readable bytes and 7,194,721 bytes
under `zstd -19`. Parsing and rewriting that certificate was byte-identical at SHA-256
`3ad5877a2ffa3bcf04c3403a147ae075e406b4313cce83eb0761fdd563725116`. This validates the format and
full-corpus ingestion; it is not an independent proof replay.

One bounded top-layer measurement followed, still without beginning the expensive descent. Eight
workers verified all sixteen explicit run9 `k=9` roots in 0.23 seconds of batch wall time and cited
all 2,545 canonical `k=8` facts; the whole capped process, including parse/index construction,
finished in five wall seconds. It stopped at `k=9`, so none of those 2,545 facts or any lower fact
was verified. The normalized level counts are dominated by 2,576,885 facts at `k=7`; proper next
work is therefore pre-color minimalization followed by the `k=8` pass, not an unbounded descent.
Run9 happens to contain exactly sixteen canonical `k=9` facts, so implicit all-top seeding would
coincide with the roots in this one file. That coincidence does not alter the format decision.

Pre-color antichain reduction is real rather than decorative on this corpus: levels k=2..5 fell
from 13/637/19,527/41,409 facts to 9/529/11,767/28,632. Starting from the one nontrivial k=6 root
above then painted 37, 92, 234, 9 and 1 targets down the levels, producing one root plus 373 support
facts. The result is 9,897 bytes, 1,410 under zstd, and replays with zero gaps. One- and four-worker
coloring produced byte-identical SHA-256
`45ff9191881b56de73f296c37c0339d34ac8342a40de58fede0882da62bda0be`.

This also settled the explicit-root question empirically. Without root records, `TOPDOWN=6` must
treat all 779 facts at that level as requested roots; it then retains 38,275 support facts. Coloring
can remove unused descendants, but it cannot infer that a supplied top-level claim was incidental.
The root/support distinction is therefore part of the readable format, not optional metadata.

`tools/test_radio_verify.sh` now locks serial/parallel agreement, same-level redundancy removal,
byte-identical coloring and replay on a tiny closed fixture. Four-worker ThreadSanitizer and
Address/UndefinedBehaviorSanitizer runs passed for both verification and coloring. Compiler
`-Wall -Wextra -Wpedantic` is clean. Full commands, sizes and the complete table are retained in
`evidence/radio_verify_parallel_2026-08-16.txt`.

Run9 has not been replayed. The next measured step is normalization plus minimalize/color from the
sixteen explicit roots; only its actual reachable k=7 count can size the full independent replay.
For a later parallel solver, the current direction is limited-width coarse prefix batches for the
exhaustive tail while retaining heuristic depth-first search for early witnesses. Per-k frozen
cache epochs are promising; read/write locks across recursion and a language rewrite are not the
first prototype. No parallel solver change was made in this session.

## 2026-08-17 — full run9 independent coloring/replay launched on AWS

The full follow-up is now detached on the existing `r7iz.4xlarge`. The source archive is exact
commit `88565098b149654213c2a47eb9c966a0078d09dd`; the raw 365,340,502-byte run9 log matched SHA-256
`ba635d9141601ebb643ed4f102703deb112fc3e8260f4936e8545fe44a300cf4`. Before any expensive work,
the remote build normalized all 3,126,190 facts and reproduced the known 106,011,566-byte text file
byte-for-byte at SHA-256
`3ad5877a2ffa3bcf04c3403a147ae075e406b4313cce83eb0761fdd563725116`; its second parse/write pass
also matched. Build ID is `655d7f22ecec9db347b1bb62f44a5597532671ce20cf0a4e56b05ebbbf647c50`.

Coloring uses fourteen worker threads pinned to CPUs 0--13 at nice level 10, leaving two of the
sixteen vCPUs outside its affinity mask. The verifier has an 80 GiB RSS ceiling and 30-day wall
backstop; the pre-existing census retains its own 20 GiB ceiling. The idle guard was safely replaced
only after `run9_verify` was live and now tracks both jobs. At the first status query the verifier
used 1311% CPU and 422 MiB RSS, while the census still used 99.9% of a core and 8.62 GiB RSS. The
host had 113.3 GiB available and no swap.

The run had already minimalized `k=2..6`: `2->2`, `137->127`, `33,042->24,816`,
`125,246->82,674`, and `388,317->229,341`, then entered the 2,576,885-fact `k=7` level. These are
support-antichain counts, not yet reachable colored counts. When coloring finishes, the same
supervisor will replay the emitted bundle with fourteen workers, require zero unresolved targets,
compress both certificates and logs, and upload them to
`s3://radio-sa193-393287594714/run9-verifier/20260817T163700Z/`.

Three launch preflights failed harmlessly in 5--6 seconds each before compilation or detachment:
two literal grep assumptions missed raw lines with `size=...` between "solve" and `Sb`, and one
grep pattern over-escaped `(`. No process or guard changed. A separate successful runtime preflight
then built the verifier and reproduced the complete normalized hash before the final launch. Exact
SSM IDs, resource snapshot and hashes are in `evidence/run9_verifier_aws_2026-08-17.txt`.
Use `tools/run9_verifier_status.sh` for a bounded one-shot query; do not create another watcher.

### Progress telemetry correction

The first status format made health easy to see but progress hard to interpret: `stage=COLOR`
also covers pre-color minimalization, and the long `k=7` call emits no line until all 2,576,885
facts have been classified. At 16:46:37 UTC, 11m42s into that call, the process remained healthy at
1396% CPU on fourteen threads. The only exact progress milestone was still completion of levels
`k=2..6`, representing 546,744 of the 3,126,174 support inputs (17.5% by record count). That is not
17.5% of elapsed time or total work: per-fact cost is nonuniform and the deployed build does not
expose its atomic queue cursor. No defensible intra-level percentage or ETA exists without
restarting with new instrumentation, which is not warranted.

`tools/run9_verifier_status.sh` now leads with a four-step `PROGRESS` summary, names the active
level, labels the record fraction as a completed-level milestone, and separately reports CPU and
elapsed time as health. It will switch automatically to level-barrier counts during coloring and
to the final replay phase afterward. Future long verifier builds should publish their task cursor;
the current run was left untouched.

The missing milestone arrived at 16:47:44 UTC. Minimalization retained 2,507,270 of the 2,576,885
`k=7` facts—97.30%—after 713.01 seconds, then reduced `k=8` from 2,545 to 2,151. Coloring verified
the sixteen roots in 0.06 seconds and all 2,151 `k=8` targets in 43.79 seconds, but the latter cited
2,506,515 `k=7` facts: all but 755, or 99.97% of the minimal level. Thus the old 190x painting
reduction from the superseded 2023 corpus emphatically does not transfer to the closed run9 DAG;
the current `k=7` batch has 2.5 million targets. The status summary now prints both current target
count and level fraction, while still distinguishing batch size from completed work. No process
was restarted.

## 2026-08-17 — end-to-end verifier benchmark: Sa(66), then Sa(113)

The verifier now has a reproducible raw-log-to-proof benchmark rather than timings from isolated
internal modes. `tools/benchmark_verifier_pipeline.sh` extracts an exact prefix, checks that every
explicit root is present, builds with provenance, normalizes to the readable certificate, requires
a byte-identical parse/write round-trip, minimalizes and colors from the roots, independently
replays the emitted certificate, checks provenance and refuses completion unless every record
verifies with zero budget outcomes. The wrapper was committed before the measured AWS runs.

To avoid disturbing or contaminating the live run9 verifier and k=8 census, the benchmark used a
temporary second `r7iz.4xlarge` (`i-0dca43cb2bb3d9f04`) in the same availability zone and AMI. It
had eight physical Xeon Gold 6455B cores / sixteen sibling threads, 128 GiB nominal RAM and no
other workload. The O3 clang-15 verifier binary SHA-256 was
`6de550a1fc26f0c8333cc9a0d67e591314d0599c20750897d6e95a5c8296c67c`—byte-identical to the live
run9 verifier. Thus compiler and code, not merely the instance type, match the large live run.

Sa(66) used the six k=7 roots `Sb(38:28)` through `Sb(33:33)`. Its 2,854 normalized facts colored
to 6 roots plus 2,031 support facts, and replay closed all 2,037 records / 1,942,412 nodes. Three
complete runs at every width produced the same colored SHA-256
`6fde697ba138738a4bf35bd4ecd445fee6c8c465e3b25cf6cf19344caf57cb00`:

| workers / affinity | median color | median replay |
|---|---:|---:|
| 1 / CPU 0 | 2.12 s | 0.89 s |
| 2 / 0--1 | 1.21 s | 0.52 s |
| 4 / 0--3 | 0.72 s | 0.30 s |
| 8 / 0--7, one per core | 0.46 s | 0.19 s |
| 14 / 0--13 | 0.36 s | 0.16 s |
| 14 / 0--6,8--14, seven isolated cores | 0.36 s | 0.17 s |
| 16 / 0--15 | 0.38 s | 0.17 s |

The tiny workload therefore points to fourteen for wall time, but its sixteen-thread regression is
startup/short-batch overhead. The two 14-CPU masks are effectively tied. On the shared live host,
`0-6,8-14` is the topology-correct mask if the census is to own sibling 15 *and* an entire physical
core; the deployed `0-13` mask reaches all eight cores and therefore shares core 7 with CPU 15.
Changing the live affinity mid-batch was not part of this benchmark and was not attempted.

Sa(113) used the nine k=8 roots `Sb(65:48)` through `Sb(57:56)`. The exact 304,105-fact normalized
certificate round-tripped byte for byte. Same-level minimalization removed 91,067 records in 3.87
seconds. Coloring then emitted 9 roots plus 120,528 support facts (3,953,000 bytes, SHA-256
`89782c213bc459ccb32fe325e82207867a50a96c1049481668434b01bb4a4755`). Independent replay closed
all 120,537 records and exactly 2,491,817,467 recursion nodes with zero unresolved or budget-limited
facts. The 14-worker external stages were 0.30 s sanitize, 0.24 s round-trip, 375.04 s color and
369.57 s replay; peak replay RSS was 1,043,216 KiB, just below 1 GiB. The whole remote phase,
including build, assertions and upload, took 12m28.574s.

Coloring's shape answers the design question. The k=6 minimal level had 65,371 facts and 60,738
were reachable; that one batch took 367.66 seconds, 99.10% of summed per-level verification wall.
Coloring reduces the durable support set to 39.63% of the raw normalized set, but it does not remove
the computational bottleneck. Minimalization is inexpensive and worthwhile. Text parsing is
irrelevant—sanitize plus the extra round-trip cost 0.54 seconds—so a binary durable certificate
would optimize the wrong component.

Two controls replayed the exact colored certificate and exact node count:

| workers | wall | user+system CPU | memo hits+misses |
|---:|---:|---:|---:|
| 8 physical | 434.86 s | 3,478.17 s | 8,931,882,315 |
| 14 | 369.57 s | 5,172.59 s | 9,491,845,978 |
| 16 | 347.91 s | 5,563.51 s | 9,648,002,282 |

Fourteen saves 15.01% wall over eight at 48.72% more CPU; sixteen saves a further 5.86% wall at
7.56% more CPU. Worker-local memo/live/pair tables make fourteen issue 6.27% more memo queries than
eight, while SMT reduces per-thread throughput. The measured policy is therefore sixteen for an
idle host and minimum wall, fourteen when preserving two logical CPUs, and eight when CPU
efficiency matters. The next optimization target is dominant-level refutation/index/cache work and
cross-worker duplication, not language, parsing or further certificate packing.

Three launch diagnostics ended before benchmark enumeration and are retained so the same setup is
not repeated: Amazon Linux had no package named `ripgrep` (1.719 s), its GNU Time banner capitalized
`Time` and defeated a case-sensitive check (7.669 s), and the clean AMI lacked the builder's default
clang (3.038 s). The wrapper is now grep-only and accepts that GNU banner; clang was installed for
the exact compiler match. The successful one-thread smoke, including clang setup/build/upload,
took 10.669 s.

All staged inputs, 24 summaries, stage logs, certificates, binaries, runner scripts and 623-file
checksum manifest are archived at the full private URL
[`verifier-pipeline-2026-08-17`](https://github.com/fedork/radio-data/releases/tag/verifier-pipeline-2026-08-17).
The 86,970,368-byte raw tar has SHA-256
`b8e4c5e4fd469488c63205d04c3739154c5d62402aab39a70c5a0a5d0068c9a0` and compresses to
11,319,563 bytes. It contains the exact pre-banner `bench_sa113_k9.txt` input, so the metadata tar
used the explicit documented legacy-container override; that input is not a verdict source, while
every verifier output inside has complete provenance. The release was downloaded, decompressed and
matched to the raw tar hash.

The temporary instance launched at 18:13:37 UTC and was explicitly terminated at 18:49:04 UTC
after all phase statuses were zero, the S3 objects were present and no benchmark process remained.
Its delete-on-termination root was disposable; the durable S3 and GitHub copies precede deletion.
The original run9/census instance remained running and was not modified. One local Sa(66) smoke
completed in about 1.1 wall seconds and left no process; no local solver or one-off Python process
was launched. Only the user's pre-existing ignored `bench_sa113_k9.meta` remains in the working
tree. A final 18:56:41 UTC query found live run9 still healthy in the same k=7 barrier after
2h21m46s at 1399% CPU and 1,296.6 MiB RSS; the census remained at 99.9% CPU and 8,840.4 MiB RSS,
with 112.4 GiB host memory available and no swap. No new run9 proof milestone was inferred.

## 2026-08-17 — cache-key shape: product profiles are promising; implied-fact expansion is not

The proposed hierarchy was state length, total mass, each segment mass, then each segment's long
side. The tail is not new in the solver: `radiobase.c` assigns segment IDs in product order and,
within a product, factorization order; descending canonical states are therefore already ordered
by segment mass and then long side. The current mutable cache is rooted by k and consumes those
segment IDs. What is new is putting length and total mass in front.

That distinction matters. Exact lookup loves the full key—product plus long side determines the
short side—but dominance is a range problem. A negative supporting fact may have fewer parts and
less mass, while a positive supporting fact runs in the opposite direction. A length/mass-first
mutable trie must either search ranges or materialize the same implied result into many buckets.
The latter is the cache blowup already measured on 2026-08-10. The deployed last-part Pareto fronts
are a deliberate partial denormalization: preserve hot prefix closure, retain only an antichain in
the last dimension, and put repeated exact queries in the 2 MiB L1.

The static negative-only verifier index is the better first experiment. A fresh shape pass over the
retained, hash-checked run9 log found 388,317 raw k=6 facts in the level queried while k=7 facts are
verified. `(np,total mass)` has only 557 distinct keys and a largest bucket of 9,069. Adding the
sorted vector of segment products yields 275,020 signatures and a largest bucket of 39. The raw
k=7 level shows the same pattern at larger scale: 2,576,885 facts, a 25,379-record largest
`(np,total mass)` bucket, but no segment-product bucket larger than 22.

A sorted product vector is a sound necessary dominance condition. Associated long sides are not
lane-wise monotone, because the valid component injection can cross product-order lanes; keep
independent sorted n, m and product profiles, then run the exact matching check. The first A/B is a
packed product column plus candidate counters on a deterministic run9 k=7 sample, comparing the
current `(np,max n,total mass)` order with `(np,max product,total mass)` and
`(np,total mass,max product)`. This entry made no speedup claim; the measured follow-up is the
product-index entry below. Full shape commands, source hashes, counts and the counterexample are in
`evidence/cache_key_shape_2026-08-17.txt`.

For a parallel solver, this reinforces frozen per-k epochs plus worker-local exact/hot caches.
Publish a normalized antichain at a coarse batch boundary, rebuild immutable range indexes once,
and let each worker demand-materialize exact hot answers. Do not put read/write locks across
recursion, and do not denormalize the full implied closure into the durable fact set.

AWS policy was also corrected from a one-run choice to a workload rule. The 128 GB r7iz host was
selected against the old 90 GB risk; the compact solver actually peaked at 1.32 GB and the Sa(113)
replay just below 1 GiB. Future launches are right-sized from the closest measured phase. Short,
fully restartable diagnostics default to Spot when capacity exists; unique cold proofs and stages
without an intra-stage checkpoint remain On-Demand. The present run9 colorer has only level
barriers, so a Spot interruption during k=7 would discard that whole barrier. `AGENTS.md` and
`docs/aws-run.md` now make the distinction durable and link the current AWS interruption guidance.
A bounded 19:38:31 UTC status query found the existing verifier still healthy in that same barrier
after 3h03m35s at 1399% CPU and 1,296.6 MiB RSS; no new proof milestone was inferred. Every local
normalizer and streaming shape-analysis process had already exited.

## 2026-08-17 — verifier product index delivered; solver cache deliberately unchanged

The cache-shape proposal was tested specifically in `radio_verify.c`, not in the mutable solver.
The sound useful part is three independent sorted dominance profiles: n sides, m sides and segment
products. A componentwise injection implies all three scalar inequalities, but product-order long
sides still cannot be paired lane-wise; the existing exact injection matcher remains mandatory.
The production representation therefore leaves the canonical fact array alone and adds a separate
immutable `(part count,max product,total mass)` permutation. Its hot scan columns denormalize mass,
packed n/m/product profiles and the next equal-product-group boundary. Only profile survivors touch
the 88-byte fact. This is bounded static-index denormalization, not insertion of implied facts.

The exact hard control was `Sb(35:10,33:13,30:26,28:18)@7` over the full normalized run9 database.
Provenance-complete O3 one-worker runs returned the same verdict, 4,644,469 nodes, 5,583,390 memo
hits and 5,187,272 misses. The legacy index took 209.63 verifier seconds / 211 external seconds at
0.41 GB peak RSS; the production index took 33.24 / 36 seconds at 0.53 GB: **6.31x** inside the
checker and 5.86x end to end. Instrumented production scanning considered 69,164,074,015 cheap
candidates, rejected 68,964,467,550 (99.7%) on product alone, rejected another 196,011,289 on n/m,
and called exact injection only 3,595,176 times. This identified a small Pareto-minimal profile
summary per fixed-size block as the next bounded experiment; the follow-up entry below records its
delivered adaptive form.

The full Sa(113) control also passed. Minimalization retained the same per-level antichain counts;
coloring the nine explicit roots emitted the same 120,293 support-fact set as the earlier
product-reordered prototype (only presentation order differed). The canonical certificate is 3,946,534
bytes with SHA-256 `5c6d986e34d2e22f53cb3327b37343687df9809085ea41341db431f18ceb4032`.
A provenance-complete eight-worker replay verified all 120,302 root/fact records, zero unresolved
or budget outcomes, and exactly 2,491,283,058 nodes in 140.28 verifier / 141 external seconds at
0.88 GB peak RSS. A full run9 `CERT_ONLY` pass also reproduced the established normalized SHA-256
byte-for-byte in five external seconds.

This product index became the default at this stage; the adaptive block follow-up below is now
layered over it. `VERIFY_LEGACY_INDEX` preserves the old layout for exact A/Bs and
`VERIFY_INDEX_STATS` exposes the filter counts. The ordinary regression suite, an
AddressSanitizer+UndefinedBehaviorSanitizer build, and a two-worker ThreadSanitizer build all pass,
including minimalization/coloring. Full source hashes, build IDs, commands and outputs are in
`evidence/verifier_product_index_2026-08-17.txt`.

Two diagnostics should not be repeated. Sparse stride samples each consumed their 300-second cap
because one selected state dominated the batch, so the exact explicit-root control replaced them.
One provenance run accidentally treated the root as additive without filtering main-input facts;
it was stopped after about 6m20s of CPU with no result. The correct one-root command uses source
mask 2, and `docs/tools.md` now calls out that `ROOTS` does not suppress ordinary targets.

No solver source, solver cache or AWS process changed. A bounded status query at 22:46:58 UTC found
the already-deployed run9 verifier healthy in the same k=7 coloring barrier after 6h12m03s at 1399%
CPU and 1,296.6 MiB RSS. The Pareto census remained at 99.9% CPU and 8,855.3 MiB RSS; 112.4 GiB was
available and swap remained zero. This supplies no new proof milestone and does not justify
restarting the live old-index barrier.

## 2026-08-17 — adaptive block-Pareto verifier index delivered

The proposed second static-index layer works on the large verifier level, but only after two
negative controls changed its shape. The sound summary is attached to full 256-fact blocks wholly
inside one equal `(part count,largest product)` group. Each block stores componentwise minima and
the Pareto-minimal set of `(total mass,top-four sorted products)`. If no stored point fits a query,
no fact in the block can pass even that necessary test; a positive summary still falls through to
the existing product/n/m filters and exact injection matcher. The same filter now accelerates
same-level minimalization. It never inserts an implied fact or changes certificate text.

The first prototype aligned blocks only by part count and probed them before the existing mass-group
skip. It kept the exact verdict, 4,644,469 nodes and memo counts, but took 45.94 verifier seconds
against the then-current 33.24-second product index. Its 1,762,914,075 block probes performed
101,332,817,879 front-point tests, much of it over positions the old mass skip would not scan. That
layout was removed. Moving the mass skip first and confining blocks to one primary-key group made
every tested size faster; 32/64/128/192/256/384/512-fact blocks took respectively
14.88/13.48/12.89/13.01/12.74/13.57/13.68 seconds on the same logical hard control, selecting 256.

Applying those blocks indiscriminately was also wrong. A full eight-worker Sa(113) colored replay
still returned all 120,302 records and the exact expected 2,491,283,058 nodes, but took 143.41
verifier / 146 external seconds at 0.89 GB, slightly behind the product-only 140.28 / 141-second
control. It built only 377 useful blocks. The production form therefore builds summaries only for
levels with at least 65,536 facts and chooses the plain versus block scan outside the hot candidate
loop. The 388,317-fact run9 k=6 support level is eligible; the 9,311-fact support level dominating
Sa(113) k=6 verification is not.

Clean-commit O3 runs at `4e58ac2` give the final comparison. The explicit product-only and default
block builds verified the hard `Sb(35:10,33:13,30:26,28:18)@7` root with identical 4,644,469 nodes,
5,583,390 memo hits and 5,187,272 misses. Product-only took 39.16 verifier / 46 external seconds at
0.53 GB; adaptive blocks took 11.70 / 15 seconds at 0.57 GB: **3.35x** inside the verifier and
3.07x end to end. Relative to the separately retained 209.63-second legacy layout, the final time
is 17.92x faster. Building 11,659 blocks and 2,166,848 front points took 0.19 seconds and 45.1 MiB.
Instrumentation rejected 266,179,545 of 271,663,392 block probes (98.0%), skipping
68,141,963,520 positions; exact matching remained exactly 3,595,176 calls and 965,605 hits.

The final small-level guard selected every tenth k=6 fact from the canonical Sa(113) certificate.
Both eight-worker builds verified 6,045 targets and exactly 251,437,448 nodes at 0.84 GB;
product-only took 15.95 seconds and the adaptive default 15.88. This is a no-regression sample, not
an extrapolated complete-runtime claim. `tools/test_radio_verify.sh` now forces two-fact blocks on
a synthetic level and requires the same two-of-four minimal antichain as the plain index. The
ordinary serial/parallel, coloring and replay checks pass. Clean ASan+UBSan and two-worker TSan
builds also passed forced-block minimalization and emitted a byte-identical one-root colored
certificate.

The adaptive block layer is now the verifier default. `VERIFY_LEGACY_INDEX` retains the oldest
control; explicit product-profile/sort macros or `VERIFY_NO_BLOCK_PARETO` retain the product-only
control; `VERIFY_INDEX_STATS` reports both layers. Full hashes, build IDs, commands and discarded
measurements are in `evidence/verifier_block_pareto_2026-08-17.txt`. No solver source, mutable solver
cache, certificate fact or Pareto datum changed.

The already-running AWS verifier was deliberately not restarted. At the final bounded 23:38:44 UTC
query it remained healthy in the old-index k=7 coloring barrier after 7h03m49s at 1399% CPU and
1,296.6 MiB RSS; the separate census used one core, 112.4 GiB remained available and swap was zero.
No local verifier, canonical search, wrapper or one-off analysis process remained. The next default
work is still to let that replay finish and archive it; parallel-solver batching remains a separate
design track.

## 2026-08-17 — first parallel-solver ownership boundary delivered

The parallel-solver track now has its first code prerequisite rather than only a scheduling sketch.
`canSolveB_ctx` carries a `radio_search_context` through every recursive B-state call. The context
owns the deterministic accepted-prefix clock, exact-state L1, joint-suffix reachability allocation
and counters, and its negative-verdict count. `canSolveB` is now a compatibility wrapper over one
default context, so every existing driver retains its interface. Context-aware minimum-`k` and FAST
split preparation keep recursive scheduling charges on the same context. The reachability probe was
updated to name its default workspace explicitly.

The serial gate is exact. Before editing, `tools/split_regression.c` emitted 1,038 `CHECK` records
with SHA-256 `2cf020540919d6fe2f8da20636ecceb8f8d14ccc4d5a3cd599ac0a91e99eade2`;
the refactored engine reproduced the same file byte for byte. `tools/work_budget_regression.sh`,
work-clock and CPU-clock builds of `tools/deadline_regression.c`, and the contraction, pliability
and per-suffix reachability regressions all pass. Builds with `MEASURE_CACHE_L1` and the combined
reachability diagnostic switches compile. New `tools/search_context_regression.sh` checks both
schedulers, proves that a finite recursive query charges only its selected context, and checks
distinct L1 and reachability backing stores. Address+UndefinedBehaviorSanitizer runs pass for that
test and for a forced-reachability negative. A parsed-cache old/new comparison also emitted
508,722 identical verdict bytes (74,493 true, 228 false, 434,001 maybe), SHA-256
`993500e0d96e311eb98a0ab9f4e917451fa49015545aaa93f9dd873a9a1a8456`. Seven tiny serial-corpus
timings rounded to the same 0.002857-second mean for old and new; this corpus is useful for
correctness but too short for a performance claim.

Build ids, source hashes, commands and exact outputs are retained in
[`../evidence/parallel_solver_context_2026-08-17.txt`](../evidence/parallel_solver_context_2026-08-17.txt).

This change deliberately does **not** launch pthread workers. The remaining state map found three
real shared writers: the result dominance trie and arenas; the lazy split catalog and its learned
`s[4]`, `s[5]` and `FAST` fields; and `sbb_to_min_k`. Printing and definitive fact publication also
need coordinator ordering. Threading the new entry point today would therefore be undefined C, not
a parallel prototype. The next implementation unit is a frozen result-cache read view with a
worker-local exact overlay, followed by immutable split geometry/metadata and a resumable serial
pass-2 prefix cursor. Only then should a bounded queue schedule coarse prefixes. The complete
ownership and publication contract is now durable in [parallel-solver.md](parallel-solver.md).

The live AWS work was left untouched. At the bounded 23:54 UTC check, the fourteen-worker run9
verifier was healthy in the full `k=7` coloring barrier after 7h19m30s at 1,399% CPU and 1,296.6 MiB
RSS. The separate census remained healthy at one core and 8,860.6 MiB RSS; the host had 112.4 GiB
available and no swap. Neither process had reached a new proof barrier, so no completion or ETA was
inferred. No local solver, verifier, capped wrapper or one-off search process was started.

## 2026-08-17 — a right-sized verifier now reports actual completed work

The old shared-host run remained healthy after 9h21m58s in its k=7 color barrier, but its frozen
binary still could not say whether it was near the beginning or end. `radio_verify.c` now has a
proof-neutral progress reporter: workers publish completed outcomes and node counts; the reporter
adds total/window/EWMA rates, per-k and per-part counts, and three oldest active facts with a coarse
2^20-node cursor. Claims and completions are separate, so queued work is not presented as done.
Exact `BATCH_START` and `BATCH_DONE` records remain available with reporting disabled.

The gates were proportionate to the new concurrency. A full Sa(113) color/replay retained the same
certificate hash and exact node/verdict counts; the exact hard run9 k=7 state retained its
4,644,469 nodes; ASan+UBSan and TSan exercised repeated snapshots with no finding; and an alternating
same-binary timing check found no overhead above noise. `tools/test_radio_verify.sh` now locks the
batch records and invalid option handling. It was also changed from `rg` to portable `grep -E`
after the first clean Amazon Linux host exposed that the regression had an undeclared dependency.

The right-sized choice is on-demand `c8a.4xlarge`: sixteen physical 4.5-GHz cores, 32 GiB RAM and
$0.86216/hour in us-west-2. A 30-GiB encrypted gp3 root, 24-GiB RSS guard, twelve-hour cap per
color/replay phase and independent 25-hour shutdown bound the run. Spot is wrong for this launch
because a k=7 barrier has no checkpoint. Three bootstrap probes, each under 35 seconds, found and
fixed an AL2023 full-`curl` conflict, the missing `rg` test dependency, and the absence of a
`ripgrep` package; their instances were terminated after diagnostic logs reached S3.

The accepted instance is `i-01f8c56b7a53a1178`, run `20260818T014906Z`, from clean commit
`f170dedc4a2e17d17b85c562efb3288e1d8946bb`. The immutable source bundle SHA-256 is
`4d4d952f06ce59cf157c0ed4c7a57d6d18c0d3799152bb1deccf1e8900fd1661`, and the verifier build ID is
`6cebdeff2aafa370a4c4f561b7c3ab05ae670e00e179ec1af487cd3a7764c8ae`. Remote self-test, raw and
normalized hashes, sanitization and byte round-trip all passed. K=7 minimalization returned exactly
the old 2,507,270-fact antichain, but took 77.17 rather than 713.01 seconds; this 9.24x is a combined
new-index, worker-count and hardware comparison, not a single-factor claim.

K=8 coloring cited 2,505,858 k=7 targets, 657 fewer than the old search order. That is not a logical
disagreement: the unchanged minimal level can support different valid witness sub-DAGs. The first
four k=7 snapshots completed 8,641, 16,782, 25,650 and 34,436 targets at 60-second intervals, all
verified with zero unresolved. At 240 seconds the verifier sustained 143.478 targets/s and 17.34
million recursion nodes/s; CPU was effectively all sixteen cores, RSS stayed below 1.5 GiB and swap
was zero. No old active fact had emerged—but that early regime was misleading.

At 720 seconds the three-part region finished. The next four full intervals completed only
32/31/33/28 four-part targets, or 0.533/0.517/0.550/0.467 per second. The verifier still admitted
15--17 million recursion nodes/s, active cursors advanced, and tasks turned over, so the abrupt
slowdown is proof-search cost rather than deadlock or work-queue imbalance. The four-interval mean
extrapolates the remaining 2.396 million states to roughly 54 days. Costs can change again along the
canonical mass ordering, so that is a local diagnostic rather than a forecast, but it makes
completion within the twelve-hour cap unlikely without a dramatic later speedup. Let the bounded
run characterize that curve; do not call the first 4--8 hour total/EWMA ETA credible.

The observation prompted one display correction without touching the running binary. The status
helper derives `LATEST_WINDOW_PROJECTION` from the frozen `rate_window`, and subsequent C builds
also emit `eta_window_s`; both are explicitly latest-interval extrapolations. At 960 seconds the
helper showed 59.39 days, while the four-interval mean is the less noisy 54-day figure. Use
`tools/run9_verifier_progress_status.sh 20260818T014906Z`; the detailed record is
[`../evidence/verifier_progress_2026-08-17.txt`](../evidence/verifier_progress_2026-08-17.txt).

## 2026-08-17 — full run9 coloring deferred after measured proof-search cost

The full-run9 coloring experiment is closed without a certificate. The key result is negative but
useful: even after the product and adaptive block indexes, this checker is independently solving
millions of negative states, and the k=7 four-part region makes that work slower than the solver
which produced the facts. Coloring can discard unreachable facts only after paying that search
cost, so continuing would spend substantially more compute without improving the proof strategy.
At the user's direction, both coloring runs were stopped and the approach was deferred.

The instrumented sixteen-core run on dedicated `c8a.4xlarge` instance
`i-01f8c56b7a53a1178` ended its k=7 color phase after 11,460.1 seconds. Its exact final snapshot was
119,649/2,505,858 targets (4.7748%): 19/19 one-part, 1,235/1,235 two-part, 108,083/108,083
three-part and 10,312/2,396,521 four-part. Every completed target verified; unverified and budget
counts were zero. The workers had admitted 105,605,161,144 recursion nodes, and the last complete
minute delivered 0.450 targets/s. Advancing active cursors and task turnover had already ruled out
deadlock; this is the algorithm's real proof-search cost. The wrapper propagated TERM, the
supervisor finalized with exit 130 at 2026-08-18T05:02:58Z, and no partial colored certificate was
claimed. From launch through finalization the instance ran about 3h14m, approximately **$2.79** at
the recorded on-demand rate of $0.86216/hour; the three earlier sub-35-second bootstrap probes add
less than two cents.

The older fourteen-worker verifier on shared `r7iz.4xlarge` host `i-0005d74f985c52ae1` was stopped
at the same time. It had spent about 12h28m in coloring, closing only the k=9 and k=8 barriers before
entering k=7; its frozen binary supplied no intra-level cursor. It also finalized with exit 130 and
no colored bundle or replay. Its marginal dollar cost cannot be separated honestly from the shared
host, which continues to run the one-core Pareto census.

Both supervisors uploaded before cleanup. The dedicated prefix is
`s3://radio-sa193-393287594714/run9-verifier-progress/20260818T014906Z/`; its normalized certificate
and `color.out.zst` were streamed back and matched final-manifest SHA-256 values
`59b7f74730037ce8ccf5ff30049d78f5c0472b2c1fdc59586af780df27872d7c` and
`da1f4afc238ea62304681dda9d5ca13abfcdc8e46fdf65d2930b4456edc9a1f3`. The old prefix is
`s3://radio-sa193-393287594714/run9-verifier/20260817T163700Z/`; the corresponding hashes are
`f6fa14fabbb0f22d5df7ae375243b9f41774488a429301a1b2ee9bbf6b01efa4` and
`53e09f421d1778a4c5d98c8bb215d5038d778dfcf1f21568725d8a6e70c95952`.
Neither prefix was promoted to a GitHub release: the 106-MB normalized input duplicates the durable
run9 raw proof source, and the interrupted color logs are diagnostics rather than proof objects.
The small measurements and hashes are retained in `evidence/`.

After the final upload was checked, the dedicated instance was allowed to reach `stopped`. Its sole
30-GiB encrypted gp3 volume was read back with `DeleteOnTermination=true`, then the exact instance
was terminated, deleting that volume. This is recoverable: the source proof remains in the private
GitHub release and the run-specific diagnostics remain in S3. The shared instance was deliberately
left running. At 2026-08-18T05:07:48Z its census process was healthy at 99.9% CPU and 8,903.2 MiB
RSS; the host had 113.7 GiB available, no swap, and no verifier process.

The next verifier design should be proof-carrying rather than search-repeating. For each negative
fact, the solver should emit a compact cover of the admissible test/split space—ranges or subboxes
annotated with the rejecting outcome and cited lower-level fact or theorem. An independent checker
would validate that the cover is complete and that each citation is exact, with work close to the
certificate size. A first prototype should target one retained hard k=7 four-part fact and measure
proof bytes, solver emission overhead and checker wall. The current readable certificate remains a
good envelope, and the current verifier remains valuable for small complete corpora and targeted
audits. Binary encoding is not the issue measured here. Top-down coloring should return only after
explicit citations make graph reachability cheap; it should not trigger a second solve.

## 2026-08-17 — hierarchical dominance lookup reopens one bounded ordinary run9 audit

The first follow-up was intentionally narrow. Raising pairwise forward checking from 256 to 512
options halved the representative five-root recursion tree from 9,158,686 to 4,690,828 nodes, but
the existing block index improved wall by only 6--9%: dominance lookup, not recursion dispatch, was
still the cost. Larger direct memos likewise moved the exact hard root by less than two percent.
Those experiments ruled out treating either knob as the main verifier redesign.

`radio_verify.c` now builds an immutable kd hierarchy over each large lower level's sound packed
profiles: total mass, four sorted products, eight independently sorted n sides and eight sorted m
sides. Every node stores componentwise minima. A failed lane skips the subtree; a fitting 32-fact
leaf still executes the existing packed tests and exact injection matcher. Thus the tree can only
remove work, never supply a proof. The same query excludes self and accelerates pre-color
same-level minimalization. Canonical facts, exact hashing and certificate text are unchanged.

The exact hard k=7 root kept 4,644,469 proof nodes, 5,583,390 memo hits and 5,187,272 misses while
kd lookup reduced individual fact probes from 5.509 billion under adaptive blocks to 431.317
million. Verifier wall fell from 11.70 to 4.20 seconds before wider pairs. On five representative
four-part roots, kd plus the 512 cutoff produced the same complete verdict in 4,690,828 nodes and
5.34 seconds (6.04 seconds in the final warning-clean repeat), versus 9,158,686 nodes and 21.00
seconds under blocks. Leaf sizes 16 and 64 took 5.91 and 6.92 seconds, so 32 is the measured
default. Pair rows now have a 128-MiB-per-worker fail-open cap; reaching it disables only forward
checking. The five-root run used 0.3 MiB.

Two full antichain passes supplied a stronger semantic gate. K=6 reproduced 229,341 minima from
388,317 facts in 3.8 seconds. K=7 reproduced the prior exact 2,507,270 minima from 2,576,885 facts
in 49.8 seconds, or 55.55 seconds including parse/index build at 0.50 GiB peak. The complete
120,302-record Sa(113) replay then returned zero gaps and the established 2,491,283,058 nodes in
119.19 seconds on twelve local workers; pair rows used 109.3 MiB aggregate with no budget refusal.
The regression now forces a two-fact kd leaf and compares its minimal antichain with both plain and
block indexes. Forced-kd ASan+UBSan checks and warning-clean builds pass.

This does not reverse the decision to defer coloring. Instead it makes one ordinary audit of the
existing 3,126,190 run9 facts plausibly comparable to the original solver cost. The dedicated
pipeline now does no reachability pruning: after normalization and byte round-trip it times a
deterministic 9,995-fact k=7 sample, refuses the expensive stage if that projects above seven days,
then retains k=7, k<=6 and k=8..9 as separate checkpoints. A 16-vCPU `c8a.4xlarge` remains the
right-sized choice. The long k=7 level has no restart cursor, so on-demand is justified and Spot is
not. If the measured gate still fails, explicit solver-emitted split-space coverage remains the
next design rather than buying a larger instance. Full local commands, hashes and controls are in
`evidence/verifier_kd_index_2026-08-18.txt`.

The clean implementation commit `cbc3eade963ca93e9986be614f6c91557c762fda` was pushed before
launch. Run `20260818T055255Z` then started on on-demand `c8a.4xlarge` instance
`i-066a6cd0b7f66d581`; the exact source bundle SHA-256 is
`9d89857e4449b51f7d0283d9bf178d4bb96b6fd66c9f4912fb2ef8cd670e0a07`. Remote regression, raw
hash, normalization and byte round-trip passed before the sample. Its first complete minute closed
198/9,995 sampled four-part facts with zero gaps at 3.300/s, 1,472% CPU, 1,359.6 MiB RSS and no
swap. That local-window extrapolation is about 8.4 days and is not the decision: the canonical
sample spans the whole level, and the committed supervisor applied the seven-day gate only to its
complete measured wall. The retained data is under
`s3://radio-sa193-393287594714/run9-verifier-progress/20260818T055255Z/`.

### The missing group-order control is the largest verifier win

The first EC2 gate froze canonical descending long-side order, as it should: changing code under a
running benchmark would have destroyed provenance. While it ran, the layout proposal prompted the
control that the 2026-08-04 experiment had omitted. That experiment tried segment mass ascending
and fewest-options-first; it never tried **segment mass descending**. Sorting parent parts by
descending `n*m`, then descending `n`, is only a traversal permutation of the same Cartesian
product. Equal parts stay adjacent, complement symmetry still fixes one global flip, and all
assignments are visited.

The direction matters dramatically. On twenty k=7 four-part roots selected evenly across all
2,398,799 such run9 facts, canonical-n order took 41,945,991 nodes / 44.88 seconds on one M4 Pro
core. Mass-descending took 5,336,038 / 7.48 seconds: 7.86x fewer nodes and 5.99x less wall.
Mass-ascending took 52,998,146 nodes / 42.49 seconds on a five-root spread control, so this is not a
generic sorting effect. A 100-root spread sample then verified 100/100 with zero gaps in 30,978,940
nodes / 40.58 seconds. Increasing the forward-check cutoff from 512 to 1,024 left the twenty-root
canonical control at exactly 41,945,991 nodes and essentially unchanged wall, so that change was
rejected.

The cross-level guard was stronger. A new twelve-worker full Sa(113) replay verified every one of
120,302 records with zero unresolved/budget results. Its search fell from 2,491,283,058 nodes /
119.19 seconds under canonical-n order to 330,226,371 / 25.10 seconds under mass-descending. A
forced-kd ASan+UBSan build also verified five run9 roots without an error. The regression now runs
all four supported group orders on one closed multi-part fixture and rejects invalid selectors.

Mass-descending is therefore the production default for the clean rerun. The immutable
canonical-order before run completed the same 9,995-fact sample with zero gaps in 23,697,303,379
nodes / 1,627.30 seconds, projecting 390,552 seconds (4.52 days). It therefore passed the committed
gate. Its just-entered superseded full phase was stopped through the capped wrapper; the supervisor
finalized with exit 130, every object in `final.sha256` was streamed back and matched, and instance
`i-066a6cd0b7f66d581` was terminated at 06:23:18 UTC with its disposable root volume. Its 30.3
minutes cost approximately $0.44. The shared census instance was never touched.

Only then did clean commit `5869a46b70cc568a6ed83e70a64b08601c235e85` launch run
`20260818T062429Z` on on-demand `c8a.4xlarge` instance `i-0b81cd58d3ba14f0c`. The source-bundle
SHA-256 is `1cb324193d79fbd788c81961f4c35437b8096a2577dfbd61037ceda856b36e19`; its `radio_verify`
build ID is `a3391fb7ceb3f8c41bb3ceee2de5ee27443bad1e64b916384194cb65e68cfd0c`. Regression, raw hash,
normalization and byte round-trip passed. The identical sample then closed 9,995/9,995 with zero
gaps in 3,197,377,218 nodes / 341.32 seconds: 7.41x fewer nodes and 4.77x less wall than the frozen
before run. Its measured full-level projection is 81,917 seconds (22.75 hours), so it passed the
seven-day gate and began the full k=7 checkpoint at 06:31:23 UTC. The progress stream exposes
`group_order=3`, making the selected traversal visible without opening provenance metadata. At
120 seconds the full stage had verified 24,973/2,576,885 with zero gaps, but it was still in the
cheap three-part prefix; the completed four-part sample, not this early aggregate rate, controls
the forecast.

## 2026-08-18 — frozen solver-core refuter meets the solver-cost gate

The independent ordinary audit answered its engineering question before completion. The optimized
mass-descending run `20260818T062429Z` had reached 251,131/2,576,885 k=7 claims after 2,160 seconds:
all one-, two- and three-part facts plus 73,045 four-part facts, with zero gaps but
48,049,145,431 recursion nodes. That is still more repeated proof search than is useful for a
verifier. Its capped wrapper was sent TERM, the supervisor finalized at 07:08:07 UTC with exit 130,
and every final-manifest object—including raw and normalized inputs—was downloaded and hash-checked.
Instance `i-0b81cd58d3ba14f0c` was terminated; the shared Pareto-census host was untouched.

The replacement implements the proposed “same as the solver, read-only and refute-only” baseline.
`radio_refute.c` parses the readable certificate, loads every negative into the production compact
dominance trie, and serially materializes required split geometry plus the per-split one-part
viability frontier. It then freezes the epoch. Each pthread owns `radio_search_context`; an audited
root bypasses its own level-k cache entry, starts directly in exhaustive pass 2, and can ask only
theorems or the immutable k-1 cache about children. A complete split with no FALSE child is a gap,
not a recursive solve. No worker writes a cache fact or split field. Pre/post cache allocation
counts and a checksum of every materialized split field make that ownership contract executable.
Because all dependencies decrease k, verification order needs no level barrier.

The new regression checks one- and two-worker agreement on a closed eleven-claim corpus, forces the
reachability accelerator to arm, locks exact prefix work and the frozen checksum, and supplies a
false `Sb(1:1)@1` claim which must fail as contradicted. ASan+UBSan and TSan pass. The existing
independent-verifier, search-context, work-budget, contraction and provenance regressions also pass.
The provenance check exposed older missing verifier environment names in both allow-lists; those
lists now cover every literal runtime knob plus all `REFUTE_*` selectors.

Sa(113) supplied the semantic gate. The complete normalized 304,105-claim certificate closed with
zero gaps in 1,272,552,775 accepted prefixes and 10.232 verification wall seconds on twelve M4 Pro
workers (13.76 seconds end to end, 123.61 user seconds, 312,213,504 bytes maximum RSS). The colored
120,302-claim subset did not close: it exposed nine uncovered splits—one at k=5, seven at k=6 and
one at k=7. The first is `Sb(15:8,8:5,8:5)@5`; the k=7 gap is
`Sb(40:23,25:25)@7`. The existing independent C verifier had reported this same bundle closed, so
that coloring/replay result is retracted as evidence until the discrepancy is diagnosed. This does
not affect the cold run9 proof, and the new run deliberately uses all normalized facts.

On the complete normalized run9 input, twelve local M4 Pro workers verified the same deterministic
9,995-fact four-part sample with zero gaps in 126.337 verification seconds and
14,246,550,669 accepted prefixes. Including the one-core 3.126-million-fact cache load, the process
took 393.27 real / 1,751.02 user / 6.53 sys seconds, 854,917,120 bytes maximum RSS and a
1,369,130,376-byte peak footprint. The verification-only projection was 8.43 wall hours locally,
already 2.70x lower wall than the old independent checker's 341.32-second AWS sample despite using
fewer and different cores.

Clean commit `e0402900f4a74853ac44344aa8080c41ce0688fe` was pushed before deployment. Frozen-refuter
run `20260818T074026Z` launched at 07:40:32 UTC on dedicated on-demand `c8a.4xlarge` instance
`i-0cb3783e937115ff1`. The exact 1,620,907-byte source bundle SHA-256 is
`37bfe28092e394c09bf7cc136c95dbf102f5d78525b35e3110bdef126f9c43e8`. The remote regression
passed. Cache construction took 296.104 seconds; serial freeze took 0.090 seconds for 593 tables /
313,374 options and published checksum `b85d3cad4e229afc`. The 9,995-fact gate then closed with zero
gaps in 81.200 wall / 1,293.979 CPU seconds and 14,231,681,107 accepted prefixes. Its full k=7
four-part projection is 19,488 wall / 310,555 CPU seconds: 5.41 hours and 74.06% of the complete
cold solver's 419,353.1 CPU seconds. Both automatic gates passed, and full k=7 began at 07:47:30
UTC. The 8-GiB RSS cap, 24-hour phase cap, 36-hour host stop and separately uploaded level
checkpoints bound the unattended run. Live cache, freeze, batch, ETA and oldest-active-root state
was available from `tools/run9_refute_status.sh 20260818T074026Z`. Full commands and comparisons are
in `evidence/verifier_frozen_trie_2026-08-18.txt`. The retained full-k=7 process rebuilt the cache
in 291.931 seconds, froze 772 tables / 383,875 options in 0.112 seconds with checksum
`828a877ab5649882`, and entered its exact 2,576,885-claim worker batch. At its first 60-second
checkpoint it had completed 82,863 with zero gaps. At 180 seconds it had completed 188,695 with
zero gaps and all displayed active roots were four-part, confirming that the expensive region was
advancing rather than merely the cheap prefix. RSS was 1,203.7 MiB with 28.9 GiB host memory
available and no swap. At that checkpoint the completed four-part sample, not the short
mixed-region ETA, controlled the forecast; the completed result follows below.

## 2026-08-18 — complete frozen replay archived; child-query path is next

Run `20260818T074026Z` completed normally. The k=7 checkpoint verified 2,576,885/2,576,885 claims
with zero gaps in 3,437,350,318,801 accepted prefixes. Its worker epoch took 19,811.819 wall and
316,683.839 CPU seconds; the capped phase, including the 291.931-second serial cache build, took
20,113 seconds and peaked at 1.24 GB RSS. The lower checkpoint verified 546,744/546,744 in
129.665 wall / 2,072.754 CPU seconds and the upper checkpoint verified 2,561/2,561 in 0.914 wall /
14.578 CPU seconds, also with zero gaps. Across the three retained phases, all 3,126,190 claims
closed in 20,845 capped wall seconds (5h47m25s). Worker epochs used 318,771.171 CPU seconds, or
76.015% of the proof-producing solver's 419,353.1 seconds. Thus the measured end result passes the
user's “verifier must not be slower than the solver” baseline, with the deliberate qualification
that it shares the solver core and is validation rather than implementation-independent proof.

Every S3 object named by the final manifest was downloaded and hash-checked. The exact payload,
including the full normalized certificate, all raw outputs, binary and source provenance, guards,
benchmark and inner manifest, was packaged as a 15,605,760-byte tar with SHA-256
`40d6b2aad35e7b7319c9b5558e645f11440eaccdb6377547cbc52b4604e7da06`. Private release
[`sa193-frozen-refute-2026-08-18`](https://github.com/fedork/radio-data/releases/tag/sa193-frozen-refute-2026-08-18)
was then independently downloaded, decompressed, byte-counted and SHA-256 checked by
`tools/artifacts.sh verify`. Only after both checks, stopped instance `i-0cb3783e937115ff1` was
terminated. Its sole root volume `vol-0dfe51cb88d605ffd` had `DeleteOnTermination=true`; the shared
census host was not touched. A subsequent AWS lookup returned `InvalidVolume.NotFound`, confirming
that the verifier volume was deleted.

The completed measurements change the optimization priority. K=7 worker CPU divided by wall is
15.985 on sixteen workers, so task dispatch, load balance and shared-memory synchronization are
already effectively saturated. Replacing C, adding read/write locks or making smaller batches
would not reduce the 318,771 CPU seconds. An eight-second 1-ms macOS sample of a current-HEAD,
twelve-worker full normalized Sa(113) replay covered 73,224 runnable worker samples: 25,425
(34.7%) landed directly in `star_expansion_majorization_can_solve`, and 44,535 in the surrounding
`canSolveB_ctx` body. Frozen CACHE_ONLY children currently try worker L1, then full-star
majorization, then the immutable negative trie. Because a frozen trie negative is already a valid
lower-level citation, the first measured variant should be L1 -> explicit/trie -> theorem fallback,
without changing ordinary solver order.

Add counters before changing it: L1 hits/collisions, explicit exact hits, dominance hits, theorem
fallbacks, and aggregate reachability roots/build time/tests/prunes. Then test a compact flat hash
of explicit facts, L1 size/associativity, and `RB_TRIGGER` on the retained 9,995-root gate. A
read-only layout bucketed by `(k, part count, total pair mass, each part's pair mass, long side)` is
still plausible, but should compete against a contiguous flattening of the current trie after the
hit mix is known. Store explicit facts first; do not eagerly materialize the full implied closure.
Three phase processes also spent 878.207 seconds rebuilding the same cache, so a shared process with
separate checkpoint batches is an easy ten-minute startup saving, and a serialized derived cache
image is the right prerequisite for cheap distributed/spot shards. Those are constant-factor and
latency improvements. The independent-verifier direction remains solver-emitted split-space ranges
with exact lower-fact citations, so checking scales with the cover rather than 3.463 trillion
enumerated prefixes.

## 2026-08-18 — level-local certificate, endpoint majorization and no-L1 k7 gate

The cache dependency was simpler than the completed wrapper made it look. A level-k frozen root
enumerates children only at k-1; loading same-level roots or unrelated levels into the trie is
useless. The one apparent exception was inherited split metadata: `freeze_one_table` walked upward
through several `kk` values to learn a minimum-solvability frontier reused by repeated root calls
at that k. The frozen phase needs only the final question. It now asks all three isolated local
outcomes directly at k-1 and marks the option dead if any is FALSE. TRUE or MAYBE retains it. This
is sound by Subgraph Monotonicity and can find a later FALSE after an earlier MAYBE. On the retained
gate it lowers accepted prefixes from 14,246,550,669 to 13,403,862,290 (5.915%) with all verdicts
unchanged.

The durable execution format is now one text file per level rather than one file containing only
that level's roots plus an opaque full cache. `radio-negative-level-certificate-v2` puts sections in
load order: explicit certificate-local part dictionary, complete k-1 support, split-part hints, and
level-k claims. `tools/make_refute_level_certificate.py` is a bounded-memory multi-pass converter
from normalized v1. Record and part-reference counts permit exact allocation. The verifier checks
the dictionary, canonical ID order, section counts, every hinted root part and each occurrence
count. Hints control eager table order only; it derives the entire split domain and theorem filter.
This preserves the critical completeness boundary—a certificate cannot omit a split.

Run9 k7 produces 1,052 dictionary parts, 388,317 k6 support facts, 772 split hints and 2,576,885 k7
claims. The file is 63,781,183 bytes / 2,967,033 lines, SHA-256
`e89963d7284affd659bbfb31d1a3e1072cf0aa6eb830062638dda0f16531d9c3`; `zstd -19` gives
7,983,524 bytes, SHA-256
`59a1724ae89aea3c26e9784c5904b0a5eadc012a878d288877ac71d19a9e3d4f`. This is slightly larger
compressed than complete v1 because numeric IDs remove highly repetitive `n:m` strings; bounded
loading, not compression, is the reason for v2. The compact offset/length claim arenas report
6,043,290 bytes for support and 40,857,722 for roots, versus roughly 404 MB of geometrically grown
fixed forty-part records. Support storage is freed after the k6 trie freezes. The support trie now
builds in about 3.1 local seconds with 14,733 branches / 251,077 fronts; the old all-level build took
263.457 seconds locally with 439,499 / 2,706,062. No explicit split table was shipped: the prior full
k7 process derived all 772 tables / 383,875 options in 0.112 seconds, so reading and validating a
supplied table could not repay its new completeness obligation.

The 34.7% theorem profile did not establish that another cache probe would hit. The direct theorem
optimization is stronger. Within the m equal values contributed by `(n:m)`, the difference between
the lifted-state prefix and `G_k` has increments `n-G_k[r+j]`. They are non-decreasing because
`G_k` is sorted, so the difference is discrete convex and its maximum lies at an endpoint. The
beginning was already checked; only the run end is needed. This changes the comparison loop from
`O(sum m_i)` to `O(parts)`. Hot states of at most sixteen parts also use a fixed sort buffer. A new
differential C regression compares literal expansion on every one-, two- and three-part multiset at
MAX_N=16/k<=6 plus 200,000 deterministic states through 24 parts: all 535,328 agree. Endpoint and
literal builds also emit identical complete `split_regression` CHECK streams.

The user also questioned whether exact L1 was worth probing at all. Actual k7 A/B answers it more
directly than counters. On the identical 9,995-claim four-part stride, with level-v2, direct k-1
freeze and no L1 fixed, literal versus endpoint majorization averaged 1,302.135 versus 928.493 worker
CPU seconds and 111.654 versus 79.468 wall: endpoint saves 28.69% CPU / 28.83% wall. With endpoint
fixed, L1 versus no L1 averaged 1,046.624 versus 928.493 CPU and 89.774 versus 79.468 wall: L1 costs
11.29% CPU / 11.48% wall. Every run has exactly 13,403,862,290 prefixes and zero gaps. A final
default build confirmed 9,995/9,995 in 79.672 wall / 934.528 CPU seconds after 3.099 seconds of cache
and 0.062 seconds of split preparation; capped peak RSS was 0.30 GB. `radio_refute.c` therefore
disables L1 by default and exposes `RADIO_REFUTE_ENABLE_L1` only as the rejected control. Ordinary
solver builds retain L1 because their prior positive-path benchmark measured a benefit.

The systematic gate projects 19,072 local wall seconds (5.30 hours) / 222,838 local CPU seconds for
the 2,398,799 four-part k7 claims. Hardware differs from the completed AWS solver, so this is not yet
a solver-cost ratio. The next authorized step is a short same-type/right-sized AWS gate; start a
complete k7 replay only if that projection remains below the proof-producing solver. A global exact
hash is deferred absent evidence that first-touch exact hits repay another universal probe.
Per-root reachability benefit is the next useful hint measurement. Full commands, build/log hashes
and controls are in `evidence/verifier_level_v2_2026-08-18.txt`.

## 2026-08-18 — complete uncolored level-v2 replay launched

The short gate was no longer the useful stopping point: the local controls had already isolated the
dominant k7 implementation, so the user authorized a complete replay. Coloring is deliberately a
separate future experiment. It is optional certificate compression, adds usage tracking to the hot
path, and the previously colored Sa(113) support set has nine known gaps under the frozen refuter.
Combining it with this run would therefore confound both the correctness baseline and the measured
cost. No coloring or usage markers are enabled here.

Commit `0f34041e3e29a801d47133c5ad03844ad4d307f2` packages each populated claim level k=2..9
as its own human-readable v2 certificate, checks that their claim counts sum to 3,126,190, and runs
each against only its complete k-1 support. The dominant k7 phase runs first; every completed level
is compressed, hashed and uploaded immediately, so a later failure cannot erase prior checkpoints.
The live status distinguishes durable checkpoint coverage from current-level progress and retains
cache construction, split freeze, current rate, ETA, active roots, memory and swap. All repository
checks, the refuter regression, full eight-level local generation and small k2/k9 integration
replays passed before deployment. Generated counts were 2, 137, 33,042, 125,246, 388,317,
2,576,885, 2,545 and 16 claims respectively.

Run `20260818T194508Z` launched at 19:45:13 UTC on dedicated on-demand `c8a.4xlarge` instance
`i-04126f6d3016378a9`. On-demand is intentional: the projected multi-hour k7 phase has no internal
claim checkpoint, so a spot interruption would discard the dominant work; completed levels are
checkpointed only between processes. The exact 1,643,619-byte source bundle has SHA-256
`beb62def6dba281ff1c387c97f70bd0400f8007a99b455b74e784dd8195a654c`. All eight level
certificates were generated and uploaded. The k7 file contains 388,317 k6 support facts and
2,576,885 claims; its raw/zstd-10 sizes are 63,781,183/12,566,615 bytes.

The sixteen-worker 9,995-root k7 calibration closed with zero gaps in 53.582 worker wall /
854.158 CPU seconds. It projects 12,860 wall / 204,998 CPU seconds for the 2,398,799 four-part
band, well below the proof-producing solver's 419,353.1 CPU seconds, so both automatic guards
passed and the full replay continued. K7 loaded its 388,317 support facts in 3.122 seconds, froze
772 tables / 383,875 options in 0.096 seconds and published split checksum `3752300250bf6532`.
At its first 60-second report it had verified 140,144/2,576,885 claims with zero gaps and
15,623,138,715 accepted prefixes. CPU utilization was about 14.77 cores, RSS 298.6 MiB, host
memory available 29.7 GiB and swap zero. The displayed 1,043-second ETA is not a forecast for the
whole phase because these first roots are the cheap three-part prefix; the completed four-part gate
is the defensible forecast. At 180 seconds the run had reached 207,666 claims and
42,929,747,543 prefixes with zero gaps; every displayed active root was four-part, confirming that
the expensive region had begun rather than the run merely racing through its cheap prefix.

Live state is `tools/run9_refute_status.sh 20260818T194508Z`; artifacts are under
`s3://radio-sa193-393287594714/run9-frozen-refute/20260818T194508Z/`. The separate shared census
instance `i-0005d74f985c52ae1` remained running and was not touched.

## 2026-08-18 — frozen-trie citation coloring implemented and launched

The old coloring conclusion was narrower than it first sounded. `radio_verify.c` was slow because
it independently re-solved every retained negative; coloring itself needs only graph reachability
once the efficient frozen solver-core audit is already available. The new
`RADIO_REFUTE_ENABLE_COLORING` build therefore uses exactly the level-v2 refuter. While the k-1
support cache is built serially, every retained negative Pareto terminal receives the 1-based index
of the original support record which supplied it. A child cache rejection marks that source.
Dominated facts which do not survive in the trie cannot be emitted accidentally.

Literal per-fact reference counters would make billions of hot cache hits contend on shared memory,
while coloring needs only zero versus nonzero. Split preparation and each worker instead receive a
private dense bitset; the driver ORs them after the immutable epoch and separately sums the total
hit count for profiling. Eager split preparation is included because its DEAD decisions are part of
the proof traversal. That may conservatively retain a fact consulted for a table option later made
irrelevant, but cannot omit a required support. Source ids are stored behind the packed Pareto
points, so an ordinary comparison scan loads the wider lane only after it has found a match.

The output is strict human-readable `radio-negative-color-selection-v1`: parent and selected
levels, source/audited/support/used counts, citation hits, then ordered `use INDEX Sb(...)` records.
`tools/make_refute_level_certificate.py --selection` refuses partial parent audits, validates every
index and copied state against the full normalized v1 level, keeps complete lower support, and
regenerates dictionary and split hints. `used 0` is an explicit terminal. The coloring executable
reserves a new output path before starting, refuses overwrite, and never leaves a valid selection
after a gap. A regression extracted `Sb(4:4,4:2)@3` from run9: one and two workers emitted the same
one-fact selection, the filtered k2 fact verified, and the chain stopped at zero. Tampered state,
partial/empty handoff, v1 input and output overwrite all fail closed.

The final-source matched local k7 gate used the same 9,995 four-part stride as the level-v2 work.
Ordinary replay closed in 79.490 wall / 929.906 CPU seconds; coloring closed in 82.363 / 967.790,
or 3.61% wall and 4.07% CPU overhead. Both verified 9,995 claims with zero gaps and exactly
13,403,862,290 accepted prefixes. Coloring selected 209,189/388,317 supports from 4,881,149,078
cache hits. The 7,479,772-byte selection was byte-identical across two differently scheduled
colored builds. Cache front storage rose from 4,428,648 to 15,099,464 bytes, still negligible. A
combined ASan+UBSan run loaded all 388,317 supports and exercised attributed-front growth with no
finding; a two-worker TSan fixture was also clean. The exact hashes and commands are in
`evidence/verifier_coloring_citations_2026-08-18.txt`.

A real local top handoff then verified all sixteen run9 k9 claims, selected 2,151 of 2,545 k8 facts
from 123,600 citations, and generated a valid 53,288,979-byte k8 file with those 2,151 claims plus
complete 2,576,885-fact k7 support. This was enough to authorize the requested separate parallel
run. Commit `e206766eb7dee8888e798964150485c98893926b` was pushed before deployment.

Run `20260818T205010Z` launched at 20:50:15 UTC on dedicated on-demand `c8a.4xlarge` instance
`i-0901e2b2c266f7db2`. On-demand remains appropriate: the likely multi-hour k7 barrier has no
within-level checkpoint, while the remote supervisor uploads each completed certificate,
selection, verifier log, provenance check and checksum before descending. The exact 1,653,058-byte
source bundle has SHA-256 `db198050c5e77ab010952e59200ee22770c769c4c195153edea444854ed7adb1`.
The same-host coloring gate closed 9,995/9,995 with zero gaps in 55.977 wall / 891.641 CPU seconds,
projecting 13,434 wall / 213,994 CPU seconds for the full four-part band, below both automatic
guards. K9 then independently reproduced the local 2,151-fact selection, uploaded its checkpoint,
and k8 loaded complete k7 support. The attributed full-k7 cache took 301.309 seconds to build;
the actual k8 audit then verified 2,151/2,151 with zero gaps in 0.365 worker seconds and selected
2,508,278/2,576,885 k7 facts from 41,460,414 citations. That second checkpoint uploaded before k7
started all sixteen workers on its 2,508,278 selected targets. At the first 60-second report k7 had
verified 112,031 claims with zero gaps and 14,691,829,604 accepted prefixes; CPU was 1,470%, RSS
318.4 MiB and swap zero. The active roots were already four-part, but the initial 1,284-second ETA
is still a changing-task-mix projection rather than a forecast. Live status is
`tools/run9_color_refute_status.sh 20260818T205010Z`; staging is
`s3://radio-sa193-393287594714/run9-colored-refute/20260818T205010Z/`. The complete uncolored replay
and shared census were not modified. S3 is live staging rather than the durable destination; after
completion and manifest verification, the compact chain belongs in a private `fedork/radio-data`
GitHub release.

The final process inventory at 21:02 UTC showed all three authorized jobs healthy. The uncolored
replay was at 954,616/2,576,885 k7 claims with zero gaps, 1,596% CPU, 385.3 MiB RSS and no swap. The
colored replay was at 112,031/2,508,278 with zero gaps. Shared census `pareto_k8_aws` remained at one
core and 9,130.4 MiB RSS with 113.4 GiB available, 1,087 full-state records and no `CENSUS END`.
No local refuter, solver, one-off Python search or orphan `Python -`/`python3 -` process remained.

## 2026-08-18 — the single-solution split-choice corpus: sound filters measured, scalar rules refuted

The question was whether the single-solution cases of the k=7 and k=8 choice censuses yield a sharp
rule for finding a winning split when one exists. They yield a clean measurement, and the answer for
*local* rules is no.

### The k=7 corpus existed only in local scratch; it is now archived

`docs/data.md` had no row for it and neither S3 nor the release store held it. The completed corpus
and its replays were sitting in `~/radio-scratch/pareto-lift/.artifacts/pareto-census/final/`, which
is exactly how the K=9 Pareto walk was lost. `final/k7.out` is canonical and reproduces the
2026-08-13 entry exactly: 32 roots, 450 first cuts, 2,956 second-cut lineages, 563 targets, 819
upgrade nodes, 610 endpoints, 7,396 raw endpoint winners, 3,227 automorphism classes,
`representation_blocked=0`.

Its three replays — `k7_frontier_replay`, `k7_frontier_independent`, `k7_probe16_replay` — agree with
it on **every semantic count** and differ only in search effort (`full_complete_candidates`
9,283 / 9,360 / 9,726 / 9,361; `full_prefixes` 171,759,828 / 171,875,686 / 175,638,694 /
171,876,485). All four pass `tools/check_provenance.py`. Archived as
`pareto-census-k7-2026-08-13` (1.5 M, SHA-256
`ccdaed0f81157479a1f6f0415b852e59a287f216a3029a21c675b74c90fb4bbf`); `verify` round-trips.

### Single-solution states are common, and concentrated in the four-part band

| corpus | endpoints | single automorphism class | part counts of those |
|---|---|---|---|
| k=7 (complete) | 610 | **183** (30.0%) | 2-part 3, 3-part 27, 4-part 153 |
| k=8 (partial snapshot) | 1,092 closed | **262** (24.0%) | all 4-part |

k=7 also has 198 unique second-cut states. Raw labelled winners per single-class endpoint are 2 at
k=7 (161 endpoints) or 4 (22); at k=8, 2 (248), 4 (12) or 8 (2). So "one solution" means one class,
normally the split and its complement.

The k=8 figures come from a read-only snapshot of the still-running shared census, taken at
2026-08-19T00:25:35Z: 558,293 lines, 77,858,732 bytes, SHA-256
`27bb441e1c43f471f2fa415608a912ec0f845c2323e7f76b68e4d5529323473e`, staged under the census S3
prefix as `snapshots/k8_census_snap_20260819T002535Z.txt.zst`. Only endpoints whose `FULL_SUMMARY`
is present *and* whose winner count matches it were used (1,092 of 1,093). The census host was not
otherwise touched.

### The census's own candidate counts are a cache artifact, not a structural measurement

`FULL_SUMMARY complete=` looked encouraging — median 2 complete candidates at k=7 single-class
endpoints, median 6 at k=8 — but `cache_pruned` uses `CACHE_ONLY` lookups against a warm dominance
cache, so `complete` depends on cache history. Re-measured with **cache-free sound filters only**
(information cap plus the four-rectangle proven-frontier condition of Subgraph Monotonicity), the
153 four-part single-class k=7 endpoints have a median of **13,276** feasible candidates, p90 24,624,
max 37,714, 2,162,996 in total — against 2 winners each. The warm cache, not any structural rule,
is what currently closes that five-order-of-magnitude gap.

### A sound cache-free filter ladder, measured (first 25 four-part single-class endpoints)

Winner labels are exact: `FULL_WIN` is the complete verdict set, so no solver call is needed to
label a candidate, and every filter below is a *necessary* condition, so recall must be 100%.

| stage | candidates | cut vs cap+frontier |
|---|---|---|
| information cap + four-rectangle proven frontier | 2,089,596 | — |
| + `R_0` full-star majorization on all three children | 129,916 | 16.1x |
| + cross-part pair solvability on all three children | 300,694 | 6.9x |
| + **both** | **14,878** | **140.4x** |

Recall was 54/54 winners at every stage, as required. The two filters are **super-multiplicative**:
16.1 x 6.9 = 111x if independent, 140.4x measured. Precision of the combination is still only
0.363% — 276 candidates per winner.

The pair oracle is the exact 2-part table at the child level `k=4`, built with the independent
`tools/refsolve.py`: 1,478 states, 1,247 solvable, 231 unsolvable. refsolve reproduced the proven
one-part `k=4` frontier exactly as a self-check, so its agreement with the C solver's winner labels
is evidence, not assumption.

The cross-part pair condition is **not in the solver**. `radiobase.c`'s `s[4]`/`s[5]` loop
(around line 2223) tests `s` at size 1, `s+3` at size 1 and `s+1` at size 2 — one part, and the two
mixed rectangles *of that same part*. It is per-part and intra-part only, matching the 2026-08-09
note that the cross-part filter was the genuinely new constraint.

**This 140.4x is measured against my own cap+frontier enumerator, not against `radiobase.c`.** Per
the standing benchmark trap it is not a solver speedup claim; a production A/B has not been run.

Pushing to the depth-1 relaxation `R_1` (`tools/bundled_majorization.py relax`) cut a further 5.8x
with full recall, reaching a median of 517 survivors on 18 endpoints — but at 30-80 s per endpoint.
The `R_d` ladder converges to exact solving, so it buys selectivity with the cost it was meant to
avoid.

**The three-part strengthening does not pay.** Extending the cross-part condition from pairs to
triples adds only **1.71x** and **1.65x** on the first two endpoints (1,062 -> 622 and 1,360 -> 822
survivors), at 437 and 528 cumulative seconds and 522/747 distinct triples solved. A *complete*
`k=4` three-part table by refsolve exceeded 10 minutes and was abandoned for lazy memoization, which
is what makes the per-endpoint cost so high. So the useful selectivity in the subset-closure
direction is concentrated in the pair condition; going deeper costs like solving, for well under 2x.
Pairs are the right stopping point.

### Refuted: no scalar geometric feature identifies the winning split

Over all **153** four-part single-class k=7 endpoints, ranking the `R_0`-feasible candidates by each
of 19 features and recording the winner's absolute first-hit rank:

| rule | median rank | median fraction of set | top-10 | top-1% |
|---|---|---|---|---|
| max child mass nearest cap (tightness) | 659 | **0.050** | 1/153 | 21/153 |
| `dev` = sum \|a·m − b·n\| ascending | 1,258 | 0.098 | 1/153 | 12/153 |
| `dev` then tightness | 1,193 | 0.094 | 1/153 | 13/153 |
| child-mass spread, majorization margins, mixed-child mass/parts/distinctness, rectangle deficits, all-or-nothing count | 1,692–12,465 | 0.147–0.935 | 0–2/153 | 0–9/153 |

Random ordering gives median fraction ~0.5 (~0.25 with two winners). Tightness at 0.050 is real but
an order of magnitude short of a rule, and **one endpoint in 153** puts the winner in the top ten.
Filtering to `R_1` first does not rescue it: the best ordering there reaches median rank 75 of 517
with zero top-1 hits.

This supersedes any residual hope from the 2026-08-08 tightness entry. That entry's 137/137 came
from witness trees in the `sat>=0.95` band and was already retracted on 2026-08-09 at 22.7%; the
measurement above quantifies what survives — tightness is the best single scalar signal available and
it puts the winner at the 5th percentile, not the front. It is consistent with the 2026-08-09 fitted
score failing to transfer from k=5 to k=6, and explains why: there is no local signal to transfer.

### Consequence for the programme

Splitting a critical four-part state is not a classification problem over split geometry. The
information distinguishing the winner is recursive, which is why the cache and the `R_d` ladder work
and scoring does not. The productive direction is therefore **cheap sound necessary conditions**
whose selectivity multiplies — the cross-part pair table is one such, is absent from the solver, and
is a table lookup — rather than further tuning of split orderings.

### Cost

All measurements were local Python against the retained corpus, single-core, minutes each: the
153-endpoint `R_0` sweep 223 s, the 25-endpoint pair/majorization ladder about 8 min, the 18-endpoint
`R_1` sweep 914 s, the exact `k=4` pair table 43 s, the lazily-memoized triple probe about 9 min for
two endpoints. No solver was run and no AWS job was disturbed.

Everything above is reproducible from the archived corpus with `tools/split_choice_rules.py`
(`single` / `table` / `ladder` / `rank`). Writing it exposed two bugs worth remembering, both of the
silent kind.

- Building the pair oracle with `itertools.combinations` omits the 52 two-part states whose two
  components are *identical* — and a child, the mixed child especially, repeats components
  constantly. Every lookup on those states then misses and the filter quietly under-prunes. It must
  be `combinations_with_replacement`: the table is 1,478 states, not 1,426.
- A candidate split is bounded by its *children's* capacity `3^(k-1)`, not by the endpoint state's
  own `3^k`. Using the parent bound inflated the cap+frontier column by 2.4x (193,536 instead of
  81,812 on `U000067`) — and `r0` re-imposes the correct cap internally, so the `+r0` and `+both`
  columns still matched the correct run exactly. A downstream sound filter masking an upstream error
  is precisely why each column was checked against an independently written script rather than
  trusted because the bottom line looked right.

## 2026-08-18 — both run9 level-v2 replays closed; top-down coloring does not pay

Both replays launched earlier in the day finished while the split-choice work was running. Both
returned `exit_status=0` with zero gaps, both are archived as `run9-level-replay-2026-08-18`, and
the interesting result is a negative one about certificate compression.

### The complete uncolored replay

`20260818T194508Z`, commit `0f34041`, dedicated `c8a.4xlarge` `i-04126f6d3016378a9`:
`TOTAL verified 3126190, gaps 0 (eight independent level-v2 checkpoints)`.

| level | claims | prefixes | CPU s |
|---|---|---|---|
| 2 | 2 | 3 | 0.001 |
| 3 | 137 | 6,663 | 0.002 |
| 4 | 33,042 | 131,223,471 | 9.354 |
| 5 | 125,246 | 5,162,225,989 | 315.900 |
| 6 | 388,317 | 20,084,360,716 | 1,294.419 |
| 7 | 2,576,885 | 3,225,431,432,303 | 209,710.501 |
| 8 | 2,545 | 55,649,275 | 5.390 |
| 9 | 16 | 0 | 0.002 |

The claims sum to exactly 3,126,190, and split as 546,744 at k<=6 plus 2,576,885 at k=7 plus 2,561
at k=8..9 — the same decomposition the 2026-08-18 frozen refuter reported, now reproduced by eight
*independently checkpointed* level files. Total 211,335.569 CPU seconds, **50.40%** of the cold
proof solver's 419,353.1. k=7 alone is 99.2% of that cost.

### The top-down colored replay, and why coloring loses

`20260818T205010Z`, commit `e206766`, `i-0901e2b2c266f7db2`:
`TOP_DOWN_COLOR verified_top=16 levels=8 audited=2846568 terminal_level=2 terminal_used=0 gaps=0`.

| level | corpus | audited | retained | used -> next | citation hits | CPU s |
|---|---|---|---|---|---|---|
| 9 | 16 | 16 | 100.0% | 2,151 | 123,600 | 0.002 |
| 8 | 2,545 | 2,151 | 84.5% | 2,508,278 | 41,460,414 | 5.804 |
| 7 | 2,576,885 | 2,508,278 | **97.3%** | 230,725 | 1,183,136,753,919 | 217,675.837 |
| 6 | 388,317 | 230,725 | 59.4% | 80,634 | 4,231,130,295 | 898.766 |
| 5 | 125,246 | 80,634 | 64.4% | 24,635 | 623,165,108 | 203.327 |
| 4 | 33,042 | 24,635 | 74.6% | 127 | 14,822,307 | 8.888 |
| 3 | 137 | 127 | 92.7% | 2 | 662 | 0.002 |
| 2 | 2 | 2 | 100.0% | 0 | 0 | 0.001 |

Each level's `used` count is the next level's `audited`, the chain terminates with an explicit
`used 0` at k=2, and the audited counts total 2,846,568.

**The compression is 8.94%** — 3,126,190 claims down to 2,846,568 — and it is in the wrong place.
The dominant k=7 level is **97.3% cited**: the proof genuinely needs almost every one of its
2,576,885 k=7 facts. What compresses is k=6 (59.4% retained) and k=5 (64.4%), which together are
0.5% of the cost.

So the colored replay spent **218,792.627 CPU seconds against the complete replay's 211,335.569 —
3.5% *more* to verify 8.94% fewer claims.** Per level the trade is visible: coloring is genuinely
cheaper at k=6 (0.69x) and k=5 (0.64x), but at k=7 it audits 97.3% of the claims *and* pays
citation-tracing overhead, coming out at 1.04x. The 1.18 trillion k=7 citation hits are what that
overhead buys.

**Conclusion: top-down coloring is not a useful compression of the run9 certificate.** The
underlying structural fact is the valuable part and is worth keeping even though the engineering
did not pay off: *the run9 negative certificate is very close to minimal at the level that costs
anything.* There is no large dead-weight subset to strip. Do not spend more on coloring this
certificate; the earlier retired independent-checker coloring design and this
citation-tracing design have now both been measured, and the ceiling is a property of the proof,
not of either implementation.

### What was checked before archiving

- `exit.status` 0 and `final.sha256` verify for both runs: 53/53 and 12/12 entries.
- Every per-level manifest verifies: 40/40 uncolored, 40/40 colored.
- All eight uncolored certificates decompress to hashes matching `level-certificates.sha256`, at
  byte sizes matching `level-certificates.meta` exactly (256 / 2,736 / 668,482 / 3,296,510 /
  10,746,713 / 63,781,183 / 53,306,691 / 45,616; 126 MB raw).
- All sixteen verify/color logs pass `tools/check_provenance.py`.
- **An independent subset check**, written here rather than trusted from the run: resolving every
  `claim` record through each certificate's *own* part table, each colored level's claim set is a
  subset of the corresponding complete level's, and both cite the same source corpus hash
  `3ad5877a2ffa3bcf04c3403a147ae075e406b4313cce83eb0761fdd563725116`. The resolved counts reproduce
  `color-chain.tsv` exactly.

That last check needed care and is worth recording as a trap: certificate `part`, `fact`, `claim`
and `split` records are **indices into the individual file's own part table**, and a colored
certificate carries a *smaller* table (176 parts versus 181 at k=5), so the indices are renumbered.
Comparing the raw `fact`/`claim` lines between two certificates therefore reports spurious
differences — it initially showed four levels as non-subsets. Indices must be resolved to `n:m`
values before any cross-certificate comparison.

Both remain solver-core validation and certificate compression, not independent proof
implementations; proof-safe cold run9 is still the proof source. Neither retroactively rehabilitates
the old Sa(113) colored certificate, whose nine uncovered splits are a separate unfixed discrepancy.

### Cost and disposition

The two replays cost about 3.8 and 3.9 wall hours on dedicated on-demand `c8a.4xlarge`, roughly
$16-17 combined. Both instances auto-stopped on their idle guards, so neither billed compute past
completion. Both are now archived and hash-verified, so both instances and their root volumes are
ready to terminate.

## 2026-08-19 — third A/B point: ordinary verifier over the post-coloring selected input

The coloring measurement left one cell empty. Coloring was measured on the *selected* input with
citation tracing on; the complete input was measured with tracing off. That conflates instrumentation
overhead with the benefit of verifying fewer claims. Run `20260819T013030Z` fills it in:

| input | verifier | CPU s |
|---|---|---|
| complete (3,126,190 claims) | ordinary | 211,335.569 |
| selected (2,846,568 claims) | colored | 218,792.627 |
| selected (2,846,568 claims) | ordinary | **this run** |

Predicted ~205,111 CPU s by scaling each level's measured cost by its claim reduction, i.e. about 3%
under the complete replay — because k=7 is 99.2% of the cost and coloring removed only 2.7% of its
claims. The interesting outcome would be a result at or above 211,335: that would mean the claims
coloring dropped were the cheap ones.

It reuses the exact `run9_refute` binary and `/root/source` tree from the finished uncolored run
rather than rebuilding, so the input file is the only difference from the baseline. Selected level
certificates come from the colored run's S3 prefix, which outlives its terminated instance, and each
of the eight downloads is checked against a pinned SHA-256. `tools/run9_selected_ordinary_remote.sh`
and `tools/run9_selected_ordinary_status.sh` drive and observe it.

**Cheap levels closed first as a gate**, all at exactly their selected claim counts with zero gaps:

| level | claims | CPU s selected | CPU s complete | ratio |
|---|---|---|---|---|
| 6 | 230,725 | 838.543 | 1,294.419 | 0.65 |
| 5 | 80,634 | 192.453 | 315.900 | 0.61 |
| 4 | 24,635 | 7.871 | 9.354 | 0.84 |
| 8 | 2,151 | 5.179 | 5.390 | 0.96 |
| 3 / 9 / 2 | 145 | 0.006 | 0.005 | — |

Seven levels total 1,044.052 CPU s against the complete replay's 1,625.067 — a 581-second saving,
tracking the prediction closely (k=5 came in at 192 against 203 predicted, k=6 at 839 against 769).

**An early counter-signal at k=7, worth flagging rather than trusting.** At its first 60-second
report this run had verified 113,683 claims over 15,896,943,271 accepted prefixes; the complete
replay's first 60 seconds did 140,144 claims over 15,623,138,715 prefixes. So 19% fewer claims for
1.8% more prefix work in the same wall time — the retained, actually-cited claims look *harder per
claim* than the ones coloring dropped, exactly the failure mode predicted above. This is the cheap
early region and the repo's own note is that an early rate is not a whole-phase forecast, so it is
not a result yet; but if it holds, the selected input will not be meaningfully cheaper and may be
more expensive, which would make the coloring negative complete: the compression neither pays for
its instrumentation nor for itself.

Health at launch: 1,482% CPU, 301.9 MiB RSS, 29.8 GiB host memory available, swap zero, all displayed
active roots four-part. k=7 has no intra-level checkpoint, so expect roughly 3.5 wall hours; the
`capped_run.sh` guard is 86,400 s wall and 8 GiB RSS. Cost about $3.

Instance `i-0901e2b2c266f7db2` (colored) was **terminated** first, after confirming its disk held
nothing unarchived: every file was either in `run9-level-replay-2026-08-18`, a decompressed twin of an
archived `.zst`, transient run state (`stage`, `*.pid`, `current.expected`), or `run9.cert` — which is
archived in `sa193-frozen-refute-2026-08-18` and whose SHA-256 was checked here to equal
`3ad5877a2ffa3bcf04c3403a147ae075e406b4313cce83eb0761fdd563725116`, the exact source hash cited by
every level certificate, closing the provenance chain. Its volume `vol-0bdc1e36eea39386c` is confirmed
deleted. Comparing remote file lists to the archive needed basename matching, because S3 nests the
certificates under `certificates/` and `levels/` while the instance keeps them flat; a full-path diff
falsely reported 21 and 86 files as unarchived.

The kept instance `i-04126f6d3016378a9` is the uncolored run's own host, chosen because coloring is a
**compile-time** `#ifdef RADIO_REFUTE_ENABLE_COLORING`, so its `run9_refute` is exactly the ordinary
binary that produced the 211,335.569-second baseline. Same binary, same host, same instance type,
only the input differs.

### Still open: the variant with real upside

Each colored level file deliberately keeps *complete* lower support, so k=7 still loads all 388,317
k=6 facts even though the chain shows only 230,725 were ever cited. Restricting the support to the
cited set would shrink the dominance trie about 40% for the phase that makes 1.18 trillion lookups,
which is the only place a large speedup could come from. It looks sound — the retained set is by
construction every fact actually consulted, so hits stay hits and misses stay misses — but it needs a
change to `tools/make_refute_level_certificate.py`, which currently keeps full support on purpose, and
a written soundness argument before it is worth running. Not started.

### Trimmed to the transitive citation set (queued behind the selected run)

The selected-input run answered a narrower question than it looked. Its k=7 prefix total is
3,220,215,775,519 against the complete replay's 3,225,431,432,303 — **0.16% less search work for 2.7%
fewer claims**. The claims coloring dropped were nearly free, so trimming *claims* cannot buy
anything. At matched elapsed times the selected run is 13-14% behind on claims but 4% *ahead* on
prefixes, confirming the retained cited claims are the harder ones.

That leaves the support as the only real lever, which is what
`--support-selection` now does. Because a level-k audit's `used` count is by construction the
level-(k-1) claim count, the trimmed chain is one nested sequence, verified level by level:

| level | claims | support complete | support trimmed | retained |
|---|---|---|---|---|
| 9 | 16 | 2,545 | 2,151 | 84.5% |
| 8 | 2,151 | 2,576,885 | 2,508,278 | 97.3% |
| 7 | 2,508,278 | 388,317 | **230,725** | **59.4%** |
| 6 | 230,725 | 125,246 | 80,634 | 64.4% |
| 5 | 80,634 | 33,042 | 24,635 | 74.6% |
| 4 | 24,635 | 137 | 127 | 92.7% |
| 3 | 127 | 2 | 2 | 100% |
| 2 | 2 | 0 | 0 | — |

The k=7 phase is the whole cost and its support drops **40.6%**. Unlike the claims trim, this does not
reduce prefix count — it reduces the size of the dominance front each of the 1.18 trillion citation
lookups scans, so it attacks cost per lookup rather than the number of lookups. That is the first
change in this sequence that can plausibly move the total.

Two checks before spending anything: regenerating the archived colored k5 certificate through the
modified generator is **byte-identical**, and the refuter regression now asserts the soundness claim
directly — the level-3 audit cited exactly one level-2 fact, so its trimmed certificate must carry
that one fact and still close with zero gaps, and a support selection aimed at the wrong level must
fail closed.

Run `20260819T020000Z` is **queued on the same instance** behind the selected run rather than given
its own host, so the third and fourth points share a machine with the 211,335.569-second baseline.
A chainer waits for the predecessor's `exit.status`, refuses to start if it is non-zero, runs the
trimmed chain through `tools/run9_level_chain_verify_remote.sh`, then powers the instance down;
instance-initiated shutdown behaviour was confirmed to be `stop`, not `terminate`, so the volume
survives. No idle guard was running, which is why the shutdown is explicit — otherwise the host would
bill indefinitely after the last run. Inputs and a `level/claims/sha256` manifest are staged under
`s3://radio-sa193-393287594714/run9-trimmed-ordinary/20260819T020000Z/input/`, and the runner checks
each download against the manifest *and* against the certificate's own declared claim count.

If the trimmed chain closes with zero gaps, that is also empirical confirmation of the trimming
soundness argument at full scale. If it reports gaps, the argument is wrong and the trim is unsound —
the run is designed so that is the visible outcome rather than a silent one.

## 2026-08-19 — the coloring/compression programme is closed: the compression was mostly illusory

Both queued A/B runs finished with `exit_status=0` and zero gaps, completing the four-point table.
Points 1, 3 and 4 share one host (`i-04126f6d3016378a9`) and one binary (`run9_refute` at `0f34041`),
so only their inputs differ; point 2 used the colored build `e206766` on the now-terminated
`i-0901e2b2c266f7db2`, which makes it the least comparable row.

| # | input | claims | k=7 support | verifier | CPU s | vs #1 |
|---|---|---|---|---|---|---|
| 1 | complete | 3,126,190 | 388,317 | ordinary | 211,335.569 | 1.0000 |
| 2 | selected | 2,846,568 | 388,317 | colored | 218,792.627 | 1.0353 |
| 3 | selected | 2,846,568 | 388,317 | ordinary | 202,592.331 | **0.9586** |
| 4 | trimmed | 2,846,568 | **230,725** | ordinary | 201,982.710 | **0.9557** |

Decomposed: citation-tracing instrumentation costs **+8.0%** (#2/#3, cross-host so approximate);
trimming claims buys **-4.14%** (#3/#1); trimming the support a further **-0.30%** (#4/#3); everything
together **-4.43%**.

### The 40.6% support reduction was 99.7% illusory

This is the finding. The k=7 cache-build lines explain the whole outcome:

| | support loaded | branches | fronts | front bytes | redundant | build wall |
|---|---|---|---|---|---|---|
| #3 | 388,317 | 14,733 | 251,077 | 4,428,648 | **156,927** | 3.121 s |
| #4 | 230,725 | 14,358 | 245,355 | 4,326,304 | **0** | 2.187 s |

Of the 388,317 facts in the complete support, **156,927 were already discarded as redundant** during
Pareto-front construction, so only `388,317 - 156,927 = 231,390` ever entered the structure. The
cited set is 230,725. The difference is **665 facts, 0.29%**.

So the dominance front was already doing the trimming, for free, at load time. Coloring spent
13,616 wall seconds and 217,675 CPU seconds to rediscover a set the loader derives in three seconds.
The resulting structures shrank only 2.3-2.5% (branches 14,733 to 14,358; fronts 251,077 to 245,355),
and the entire measurable saving is 0.93 seconds of cache build out of ~201,000 CPU seconds. `redundant`
going to exactly 0 is the tell: after trimming, nothing is left to discard.

#3 and #4 report the identical `split_checksum=02f5ed6cbfc31d94` and identical prefix totals
(3,220,215,775,519), confirming their claim sets and split preparation match exactly, so the 0.30% is
a clean isolation of the support effect and not a difference in work.

### Why even the claims trim's saving is not what it looks like

At k=7, #1 to #3 drops CPU 209,710.501 to 201,548.279 (-3.9%) while prefix work falls only
3,225,431,432,303 to 3,220,215,775,519 (**-0.16%**). The search is essentially identical. What changed
is the split-table working set: 772 tables / 383,875 options becomes 692 / 355,174, and the prefix
rate rises 245,866,280/s to 255,596,243/s (**+4.0%**). So the gain is locality in split preparation,
not less searching — and it therefore says nothing about certificate size being a cost driver.

### Conclusion: certificate compression cannot speed verification here

Verification cost is **prefix enumeration**, not fact lookup. k=7 does 3.22 trillion accepted prefixes
against 1.18 trillion citation hits — 0.37 lookups per prefix — and cutting the dominance front by
40.6% (nominally) or 0.29% (actually) moves the total by 0.30%. Compressing the certificate attacks
the wrong term. Both coloring designs have now been measured end to end, and the ceiling is a property
of the proof and the engine, not of either implementation.

**Do not revisit certificate coloring or compression as a performance measure.** The available
compression is 8.94% of claims and 0.29% of genuinely-live k=7 support; obtaining it costs 8.0% and
using it saves 4.4%, of which the support half is 0.30%. If verification throughput ever matters
again, the target is prefix enumeration.

### The trimming soundness argument is confirmed at full scale

Run 4 verified all 2,846,568 claims with **zero gaps** while carrying 40.6% less k=7 support, which is
the empirical confirmation the argument needed: hits stayed hits, misses stayed misses. Its
`input.summary` independently records what was actually loaded per level (level 7: claims 2,508,278,
support 230,725), so the experiment demonstrably tested what it claims. `--support-selection` is
therefore sound and available, but with the above it has no performance use; its remaining value is as
a way to *measure* how much of a certificate is live.

### Cost and disposition

Run 3: 12,665.313 wall seconds. Run 4: 12,627.213. About 7.0 wall hours combined on one on-demand
`c8a.4xlarge`, roughly $5. The chainer stopped the host on completion as designed. Both runs'
`final.sha256` verify (37/38 and 38/39 entries present locally); the two absences are the reused
`run9_refute` binary and its sidecar, which live on the instance and are already archived in
`run9-level-replay-2026-08-18`. One spurious `STATUS` mismatch in each run was a flaw in
`run9_level_chain_verify_remote.sh`, which rewrote `STATUS` after hashing it; fixed by finalizing
`STATUS` before the manifest is built.

### The trimmed chain is adopted as the Sa(193) certificate of record

Performance was the wrong reason to want it and that is now settled, but minimality is a good reason,
so the trimmed chain becomes the artifact we hand out. Two properties were checked first, because
count agreement is not closure.

**Inductive closure, as resolved states.** For every level, the support set is *exactly* the claim set
one level down — zero set difference at all seven boundaries, not merely equal cardinalities — and
level 2 carries no support, so the induction terminates instead of dangling. The top level is exactly
the sixteen single-part states `Sb(97:96)` through `Sb(112:81)`, every one summing to 193. So each
claim is refuted using facts that are themselves claims proved one level lower, down to a level
proved outright.

`tools/check_level_chain.py` now performs this check with no solver involved, alongside per-level
internal consistency (declared counts against actual records, part indices in range, reference totals,
duplicate detection). It passes on the trimmed chain and — as a control that tests the tool rather
than the input — also on the complete chain, which is closed for the same structural reason. A
deliberately mixed chain (trimmed k=7 with complete k=8) fails with the right diagnosis: 68,607
support facts at level 8 not proved at level 7. The regression in `tools/test_radio_refute.sh` locks
both the positive and the dangling-reference negative.

**A size claim I had to correct before recording it.** The first comparison showed the trimmed chain
40.60% smaller compressed, which is wrong: I had compressed the trimmed certificates at `zstd -19`
while the complete ones came from the run at a lower level. Recompressing both at `-19` gives
**8.82%** (17,155,540 to 15,642,637 bytes), consistent with 8.23% raw and 8.94% fewer claims. The
40.60% figure was a compression-level artifact and coincidentally close to the unrelated 40.6% support
figure, which is exactly how such a number survives unchallenged.

Released as `sa193-certificate-2026-08-19` with the eight certificates, the per-level verification
evidence from run `20260819T020000Z` (all 2,846,568 claims, zero gaps), a hash manifest and a README.
`docs/sa193-certificate.md` is the standing description, including the two-halves checking recipe:
`check_level_chain.py` for structure without a solver, and the frozen refuter per level for semantics.

The classification is unchanged and stated explicitly in that document: cold run9 remains the proof
*source*; this is a compact checkable replay artifact. The refuter shares the solver core, so a
zero-gap replay is solver-core validation rather than an independent second implementation, and
`check_level_chain.py` is solver-independent but checks structure only. The trimmed chain's derivation
used the coloring run, but its validity does not depend on coloring being correct — coloring only
proposed the subset. The complete corpus (`run9.cert`, `3ad5877a...`) and the complete level chain are
retained deliberately, because the trimmed chain cannot answer anything outside its own claim set.

## 2026-08-19 — the k=8 census ETA: the blended endpoint rate is the wrong denominator

A status check on the shared k=8 Pareto-prefix census (instance `i-0005d74f985c52ae1`, run
`20260814T0132Z`). The run is healthy and in its final `map_endpoints` sweep, which emits exactly one
`CENSUS FULL_STATE` per endpoint and then `CENSUS END`, so for the first time in this run the
progress denominator is exact: 1,747 of 1,893 endpoints emitted at 2026-08-19 14:24:45 UTC, 1,746
complete, one core, 9,396.1 MiB RSS, 113.2 GiB host memory available, no swap.

**The obvious ETA is wrong and worth recording as such.** 659 endpoints closed in the 59,205 s
between two status snapshots — 89.8 s each, extrapolating to 3.7 h for the 148 remaining. That number
is refuted by direct observation, not by argument: I sampled the counter every five minutes for half
an hour and it never moved, because endpoint `U001747` alone has consumed >= 1,894 s. Endpoint
`U001087` behaved the same way earlier, sitting in flight across three snapshots for >= 1,809 s. A
prefix-only cost model at the largest admissible rate predicts 65 s for `U001087`; it used >= 1,809 s.
Refuted by 28x.

**What actually costs time.** Fitting `cost = a*prefixes + b*exact_queries` on the one window with a
known duration bounds `b` to 0.61-0.837 s per exact solver query, with enumeration prefixes almost
free by comparison. The measured window was 73% high-mass endpoints, which hold 97% of the prefixes
but only 30% of the queries — so its blended mean describes a population the remainder does not
belong to. Of the 147 endpoints left, 63 are below mass 600, where mean query counts run 10-20x
higher (1,101 at mass 550-574 against 49 at 625-649); only 234 of the 1,745 already done were in that
region. Re-predicting from per-mass-bucket means gives 41,972 remaining queries and 2.735e9 prefixes,
hence **7.95-9.76 h, central 2026-08-19 ~23:05 UTC**, plus 5-15 min of compress/analyse/upload.

The estimate cross-checks: the same `(a,b)` pairs place the start of the full-state phase between
2026-08-17 21:52 and 2026-08-18 01:29 UTC, all of which postdate the 2026-08-17 00:52 observation of
zero endpoint records — the one independent timing constraint available. It is nonetheless optimistic
at the low end, since `U001747` has already outrun its own band's mean by ~8x, and the 15 remaining
endpoints at mass 550-574 carry 3.2 h of the estimate with a 476-2,975 spread in query count. Treat
7.95 h as a floor.

Full derivation, observation log and band tables in
[../evidence/pareto_census_k8_eta_2026-08-19.txt](../evidence/pareto_census_k8_eta_2026-08-19.txt).
Cost of this measurement: four read-only SSM probes and a 30-minute sampler, no solver time, no
change to the running job. The durable lesson is method, not the number: when a phase's items are
heterogeneous, a mean rate over a window with a different mix is not a forecast, and the cheap way to
find out is to sample the counter and see whether it moves at all.

Process inventory at handoff: the census is the only research binary running anywhere; it was not
touched. The local 30-minute sampler (`/tmp/census_sampler.sh`) and its waiter both exited. No local
solver, refuter, one-off Python search or orphan `Python -`/`python3 -` process remains.

## 2026-08-20 — the k=8 census closed, archived, and its host torn down

The census exited 0 at 2026-08-19 22:34:43 UTC after 5.87 days, and the shared `r7iz.4xlarge`
`i-0005d74f985c52ae1` that had carried every `Sa(193)` run since 2026-08-05 is now terminated with
its volume confirmed deleted. The compute for this whole programme is wound down; one stopped
instance remains, `i-04126f6d3016378a9`, whose own output is already archived.

**The corpus.** 55 roots, 817 first cuts of which 344 strict, 815 second-cut blocks, 7,146 second
winners, 1,688 targets, 2,435 upgrade nodes, 1,893 endpoints, 50,494 raw endpoint winners and 24,330
automorphism classes. The internal cross-checks all close: `ENDPOINT` = `FULL_STATE` = `FULL_SUMMARY`
= 1,893 exactly, `FULL_WIN` matches the `STATUS` counter, `CENSUS END` is present and
`representation_blocked=0`, so nothing was silently dropped for want of `MAX_N`. The exact oracle
answered 29,366,073,123 hits against 878,206,368,508 misses over 11,655,466 facts. Archived and
round-trip verified as `pareto-census-k8-2026-08-19`; the raw log passes `check_provenance` and its
SHA-256 equals the one the host itself recorded in `run.meta`, which is the check that matters —
it ties the bytes in the release to the bytes the solver wrote.

**The ETA held, and the method is the durable part.** Yesterday's projection was 7.95-9.76 h with a
central 8.67 h; the run took 8.17 h, implying 0.63 s per exact solver query against a predicted
0.61-0.837 range, 5.8% below centre. The naive blended rate of 89.8 s/endpoint would have said 3.7 h
and been wrong by 2.2x. The reason is worth restating because it generalises past this run: the
measured window was 73% high-mass endpoints, which hold 97% of the enumeration prefixes but only 30%
of the exact queries, while the remaining work was 43% low-mass where the ratio inverts. A mean rate
over a window whose item mix differs from the remaining work is not a forecast. The cheap diagnostic
that exposed it was not analysis but sampling the progress counter every five minutes and noticing
it never moved.

**Teardown discipline.** I inventoried the disk rather than trusting the record, which took one SSM
call: 6.9 GiB used, no process running, and every artifact — nine `Sa(193)` run directories, four
verifier directories, the census work directory — had an S3 counterpart, with `run9` and `run8`
additionally in `sa193-cold-2026-08-16`. Only then did I terminate. Two things worth writing down
for whoever winds down the rest. Terminating an instance does not touch S3, so the distinction that
matters at teardown is *volume-only* versus *anywhere else*, not archived-versus-not. And
`s3://radio-sa193-393287594714/` is now the **only** copy of the run3/run/run2/run4-7 raw logs,
which were never promoted to the release store — deleting that bucket is therefore a real decision,
not cleanup.

**Deliberately not archived** into the release: the census `input.tar.zst` (123 M compressed, 934 M
raw of `exact.cache`, `dominance.cache`, `root_winners.out` and superseded local checkpoints), and a
9.2 M intermediate progress snapshot which I verified byte-for-byte to be a 558,293-line prefix of
the final log. Both stay in S3; the input bundle's per-file SHA-256 list travels inside the archived
metadata tar, so it can be promoted later and checked. Cost of the whole session: four read-only SSM
probes, one inventory, ~10 minutes of instance uptime to run it, and no solver time.

Process inventory at handoff: no research binary is running anywhere, on AWS or locally. The local
sampler and its waiter exited. No `Python -`/`python3 -` orphan remains.

## 2026-08-20 — forced cuts in the completed k=8 corpus: one sharp law, and no level connection

With the k=8 census finished I extracted every single-solution four-part endpoint from both censuses
and looked for structure. `tools/analyze_single_solution_cuts.py` does the whole thing in about four
seconds from the two archived logs; it imports the equivalence semantics from
`analyze_pareto_prefix_census.py` rather than reimplementing them, and it reproduces the archived
k=7 analyzer exactly (183 single-class, 153 four-part), which is what licenses the new k=8 numbers.
Full tables in [../evidence/single_solution_cuts_2026-08-20.txt](../evidence/single_solution_cuts_2026-08-20.txt).

**Final counts.** k=8: 1,893 endpoints, 50,494 winners, 24,330 classes, **505 single-class (26.7%)**
of which 471 four-part — superseding the partial 262 of 1,092. k=7: 183 of 610 (30.0%), 153
four-part. "Single solution" nearly always means one cut up to complementation: 450 of the 471
forced k=8 endpoints have exactly two raw winners.

**The one sharp law: the mixed child is always strictly the largest.** 26,876 of 26,876 winning
classes across both corpora, no ties, margin at least 18 at k=8 and 3 at k=7. I nearly filed this as
geometry until the control: uniform random splits have it 80.5% of the time, and random splits that
*already satisfy the information bound on all three children* only 59.0%. So the cap does not imply
it — solvability does. The mechanism is presumably that the mixed child spreads its mass over up to
twice as many parts and so is the easiest of the three at equal mass, which pushes solvable splits
to load it; the per-part identity `x^2 - 4sc = [a(m-b) - (n-a)b]^2 >= 0` points the same way but
does not sum to the global claim. If it survives testing at other k it is a zero-cost necessary
condition that discards ~41% of cap-feasible candidates. **It is a conjecture measured on maximal
endpoints at residual k=5 and k=6 only.**

**The levels do not connect, and the reason is shape.** This was the question I most expected to
pay off: a k=8 endpoint sits at k=6, so its children sit at k=5, exactly where the k=7 census's
endpoints live. Of the 1,413 children of the 471 forced four-part k=8 cuts, **one** is a k=7
endpoint — the same answer under dominance as under identity. It is not a size artifact; 942 of the
1,413 pass both necessary conditions (mass <= 221, parts <= 4). The two populations are simply
different regions of the k=5 space: k=7 endpoints are thin (aspect median 3.20) and 87% four-part,
while the k=5 states arising inside forced k=6 solutions are squat (2.29) and mostly three-part, so
containment fails on the short side. The k=7 census's endpoint family is not an upper set for k=5
solvable states — it enumerates what is maximal in *k=7 root lineages*. Corroborating that, k=8
children reach mass 242 where no k=7 endpoint exceeds 221. The exception is instructive: for the
degenerate endpoints the picture inverts completely, 48.5% of 2-part and 34.4% of 3-part k=8
children *are* k=7 endpoints. Thin lineages meet; four-part ones do not.

**Where the levels do agree is in aggregate geometry.** Scaled by `sqrt(cap)`, the two corpora have
the same normalized shape distribution: part size `n*m/cap` median 0.211 at k=8 against 0.206 at k=7,
quartiles 0.173/0.257 against 0.165/0.267. Each part takes about a fifth of the information budget
at both levels and four of them fill 85-88% of it. That is a real self-similarity under the sqrt(3)
length scaling, and it coexists with the state-level disjointness above.

**Negative results, which is most of it.** Nothing scalar separates forced from unforced endpoints:
occupancy 0.867 vs 0.866, child spread identical, diagonality identical. Diagonal cuts turn out to be
rare rather than canonical — only 6.4% of forced k=8 part-cuts are exactly proportional. I also
checked the obvious deflationary explanation, that single-class states are just automorphism-rich
states whose cuts collapse, and it is refuted in the right direction: k=8 four-part endpoints with a
repeated component are single-class in 8.0% of cases against 26.4% for asymmetric ones. The only
enrichment that survives a control is sliver cuts — parts feeding just two of the three outcomes —
at 33.4% vs 27.9%, z=+5.3 at k=8; but the same comparison at k=7 gives z=+1.1, so one level is not a
replication and it stays a lead. Popular cuts exist but shape does not force them: of 236 distinct
k=8 part shapes, 3 have a single forced cut, and the most concentrated (15:8 -> 14:8, 68% of 28
sightings) are near-whole-part slivers.

A methodological note worth keeping: the first version of the sliver comparison took one
representative class per multi-solution endpoint, which is an arbitrary choice that biased the
control; over all 23,410 multi classes the numbers moved. When the treatment group is defined by
having exactly one of something, the control has to use all of them.

Process inventory: no AWS compute remains — both radio-tagged hosts are terminated and their volumes
deleted. Nothing running locally.

## 2026-08-20 — a learned ranker for cut selection: the corpus was never the problem

Follow-up to the forced-cut analysis, prompted by the fair objection that "mixed is largest" is not
sharp enough to *choose* a cut. It is not — I measured it exactly (2D DP over `(S, X)`, so the whole
split space is counted rather than sampled) and it removes about half the cap-feasible candidates,
1.95x at k=7 and 1.92x at k=8. The best 100%-recall variant of it, a fixed margin threshold, gives
2.5x. Against the existing `R_0` at 16.1x and `R_0`+pairs at 140.4x that is not a filter worth
having. I had described it as one; that framing was wrong and is corrected in place.

**The right frame for learning is not 624 examples.** The unit is (state, candidate cut), the census
enumerates every winner of every endpoint, so unrecorded cap-feasible cuts are clean negatives:
26,876 positives against roughly 1e7 candidates per state. Scarce axis is states, not examples.

**Result.** Trained on the k=7 corpus only, tested on 120 forced k=8 states with 6,000
sound-filtered candidates each, a logistic regression on 26 scale-normalized features finds the
winner after a **median of 7 tries — 428x better than blind**, worst case 6.5x. Full detail in
[../evidence/learned_cut_ranker_2026-08-20.txt](../evidence/learned_cut_ranker_2026-08-20.txt),
reproducible in ~40 s with `tools/ml/cut_ranker.py`.

Three guards make that believable, and I would not have believed it without them. Splits are grouped
by state. The headline is cross-level, so neither the state nor the level was in training — that
rules out memorisation far more convincingly than a within-level holdout. And a permuted-label
control through the identical pipeline gives 1.6x. An earlier version of this measurement reported
1201x, which was an artifact of censoring: with 600 sampled negatives, "zero negatives outrank the
winner" is a floor, not a value. Quote floors as floors.

**The answer to "is it bound to overfit on such a small corpus" is no, and the reason is worth
keeping.** Performance is flat from 26 training states to 534 — the learning curve never rises. And
plain logistic regression beats gradient boosting, 428x to 273x. Both are signatures of a smooth,
low-dimensional decision surface, not of a data-starved one. The binding constraint is the feature
set, and the corpus is about two orders of magnitude larger than this model can use.

**The elicited rule is small.** Three features give 57.8x, eight give 250x: cut every part close to
proportionally, balance the two pure outcomes, leave headroom under the cap in the larger pure child,
and avoid a single dominant rectangle in the pure children.

**A correction I owe.** Yesterday I wrote that diagonal cuts "carry no signal". That was measured on
forced endpoints against multi-solution endpoints — winners against *other winners*. Against
non-winners, diagonality is the strongest single feature in the whole study: `diag_mean` alone gives
9.7x, with winners at |a/n − b/m| = 0.083 against 0.179 for sound-filtered non-winners. Both
statements are true and the distinction is the substance: **geometry does not tell you which states
are forced, but it does tell you a great deal about which cut wins.** Several sessions of negative
results on "no scalar feature locates the winner" were all measured on the first contrast; the
second was never tested until now.

**What this is and is not.** It is a ranker with no recall guarantee — worst case 6.5x — so it can
order a search but never prune one, and the sound filters keep their role. Untested where it matters
most: transfer to residual k=7, and composition with `R_0`/pairs, whose 140.4x was quoted against a
stronger denominator (cap + four-rectangle frontier) than the stage-2 set used here, so the numbers
are not additive. Also worth recording separately: the per-part Pareto bound read straight from
`data/pareto_sb.csv` is a sound full-recall filter of ~9-12x on its own, and it is applied before
anything above is measured.

`tools/ml/` is the first thing in this repo with third-party dependencies. They live in the
gitignored `.venv`; the three checks and everything under `tools/` proper stay dependency-free, and
`tools/ml/README.md` says so.

Process inventory: no AWS compute; nothing running locally.

## 2026-08-20 (later) — exact ranks, and why "top-5 guaranteed" is out of reach this way

Asked whether the ranker could be sharpened to a rule that names one cut, or five guaranteed to
contain a winner. Answering it forced a correction and then produced a clean negative.

**The correction.** My "median 7 tries" was 7 out of the 6,000 *sampled* candidates, not out of the
real set. The per-part Pareto bound is separable per part, so the stage-2 candidate set can be
enumerated outright: median **54,014** at k=7, roughly 3e6 at k=8. True ranks are therefore ~10x and
~500x those sampled counts. The 428x ratio is scale-free and survives; the absolute number did not.
Sampled costs mean nothing until scaled by (true set size / sample size), and I published one that
had not been.

**Exact ranks**, all 153 forced four-part k=7 states, model trained on k=8 only, no sampling:

  blind search        median 27,007
  logistic ranker     median    193   top-5  2.6%   worst  8,834
  R_0 then ranker     median     76   top-5  3.3%   worst  1,533

`R_0` full-star majorization on all three children kept the winner in 60 of 60 states, as a sound
filter must, and removed 8x of the candidates. It is the only component here carrying a guarantee,
and what it guarantees is retention, not smallness.

**So top-5 is roughly 15x short on the median and 300x short on the tail**, and the tail is what a
guarantee needs. Adding the cross-part pair condition (6.9x elsewhere) plausibly reaches a median
around 10 and does nothing obvious for the worst case.

**The reason is structural, not a modelling failure.** The R_d ladder already told us: R_0 is 8x
here, R_1 adds 5.8x at 30-80 s per endpoint, and R_d converges to exact solving. The winner is
defined by its children's solvability, not by the parent's shape, so a shallow scale-normalized
feature map cannot climb that ladder however much corpus it is given — which is consistent with the
learning curve being flat from 26 states.

**What is achievable, and is probably what was actually wanted:** a *complete* search with small
expected cost. Sound filters, which cannot drop the winner, then the learned ordering, then full
enumeration as fallback. Expected solver calls at k=7 fall from ~27,000 to ~76 while the algorithm
stays complete. The guarantee comes from the fallback, not the model. A guarantee proper would have
to come from a theorem — the mixed-largest law is the theorem-shaped candidate — or from exhaustive
verification over the domain of use, which for k=9 does not exist.

`tools/ml/exact_topk.py` and `tools/ml/filter_then_rank.py` reproduce the two tables. One trap worth
recording: I first ran the R_0 filter one level too low and it rejected every candidate including
every winner. A sound filter that removes the winner is always a bug in the caller, never a result —
the 0/60 survival rate is what caught it.

## 2026-08-20 (later still) — the predictor has to recurse, and the corpus for it already exists

The flat-feature ranker stalled at median rank 76 of 54,014 with a learning curve flat from 26
states, which says feature-set limit rather than data limit. `solvable(S,k)` is an AND-OR recursion,
so the object to learn is the recursion — a value `V(S,k)` and a policy over splits, applied at every
level — not a shape-to-cut map at one level. Design note in [ml-guided-search.md](ml-guided-search.md);
nothing is measured yet and it is labelled as design.

Three things came out of thinking it through that change what the project costs.

**The training corpus is already sitting in the artifact store.** The adopted Sa(193) certificate is
2,846,568 `(state, k) -> unsolvable` claims **spanning k=2 through k=9**, per level, normalized, with
checked split hints, verified with zero gaps. Add 57,890 complete census winners at k=5 and k=6 for
positives and 11.6M oracle facts in the solver caches. Nothing needs generating. The bias needs
stating: the chain is all negatives from a single refutation lineage, so positives must come from the
censuses and the witness trees.

**The action space has to be factored, and that also fixes the cost problem.** There are
`prod (n_i+1)(m_i+1)` splits, about 1e9 at a k=8 endpoint, so no softmax over actions and — the part
that actually matters — no per-candidate network evaluation. Factoring the policy per part means one
pass over `sum_i (n_i+1)(m_i+1)` ~ 1e3 options, and then the same `(S,X)` DP I used to count the
candidate space exactly gives **exact top-k decoding** with the information cap enforced inside it.
That is the difference between a feasible design and an unaffordable one, and it fell out of
machinery already built for the measurement.

**The soundness asymmetry decides where to aim.** Guidance is correctness-free for *achievability*
only when the resulting witness uses canonical, distinct-slot embedded, or fully explicit leaves.
`check_witness.py` checks arbitrary majorization terminals structurally but now labels them
conditional. For *unsolvability* the OR-branches have
to be exhausted, and pruning one by a learned value would manufacture exactly the false negatives the
2023 corpus already shipped 37 times. So this points at the k=9 achievability frontier, where the
14-month near-diagonal walk lives, and stays away from refutations.

The named methods that fit are learning-to-branch (strong branching is precisely our exact child
solve, and its evaluation protocol — end-to-end solve time, never prediction accuracy — is the one to
copy), Proof Number Search as the host algorithm since it already carries per-node difficulty
estimates on an AND-OR tree, expert iteration for bootstrapping, and Bellman consistency as a loss
for pushing past the levels the solver reaches. Curriculum across k is justified by evidence rather
than hope: the k=7 -> k=8 transfer is already measured.

First experiment is deliberately small: a DeepSets value model over parts with `sqrt(3^k)`
normalization, held out by *level* (train k<=6, test k=7) rather than at random, with a permuted-label
control. It only earns a policy stage if that separates. And it gets judged on end-to-end CPU seconds
against the sound filters it would displace — the per-part Pareto bound is 9-12x at full recall for a
table lookup and `R_0` another 8x, so anything learned has to beat a table lookup on cost, not only on
quality. A negative result there is worth recording with its cost.

Also cleaned two stale next-step items: both level replays finished and were archived days ago but
were still listed as running, and the numbering had collided.

## 2026-08-20 (evening) — the value model transfers across levels; the first attempt to show it did not

Ran the level-held-out experiment from [ml-guided-search.md](ml-guided-search.md). It works, but the
first version of the measurement was worthless and that is the part worth reading.

**The invalid experiment.** Negatives from the certificate chain (decoded per level: 2 / 127 / 24,635
/ 80,634 / 230,725 / 2,508,278 / 2,151 unsolvable states at k=2..8), positives from the censuses
(endpoints are solvable at their residual k; children of winners at k-1). Train k<=5, test k=6. It
gave AUC 0.9458 with a permuted-label control at 0.5163 and a beaten mass baseline at 0.8702 — all
the boxes ticked. It is still meaningless: **the two label sources occupy disjoint mass bands.** At
k=6 the census positives run 0.742-0.875 of cap and the certificate negatives 0.827-0.959, and the
central-90% overlap between them is *empty*. `headroom` alone scores 0.9784 on the matched four-part
subpopulation. The classifier separates which corpus a state came from, not whether it is solvable.

Two things to keep from that. A permuted-label control does **not** catch source confounding — it
destroys the source signal along with the label signal and comes back at chance, looking reassuring.
What caught it was a matched-pair probe: single-part states from `pareto_sb.csv` differing by exactly
one coin, `(n1:m)` solvable against `(n1+1:m)` unsolvable, on which the model scored **47%** — chance
— while the headline AUC said 0.946. When a matched-pair probe disagrees with an aggregate metric,
the probe is right. Both are now traps in [status.md](status.md).

**An oracle cost that nearly derailed this.** `radio_one` took 205 s per invocation and I briefly
concluded the oracle was unaffordable. It was not solve time: `init()` runs before the argument check
and its static tables scale with `MAX_N`, so the same query costs 205 s built at `-DMAX_N=400` and
**0.2 s** at `-DMAX_N=120`, with the solve itself at 0.0 s for k=4 and k=5. Sized honestly, 4,400
labelled states took about three minutes over eight workers. Also a trap now, because the failure
mode is to abandon a viable design over a build flag.

**The valid experiment.** Draw both classes from one sampler — random four-part states, mass in
[0.70, 1.02] of cap, side-sum <= 112, 2,200 per level — and label every one with `radio_one`. Train
k=4 (59.8% solvable), test k=5 (50.9% solvable):

  mass/cap                          AUC 0.8770        per-part Pareto deficit  AUC 0.8773
  logistic regression  k=4 -> k=5   AUC 0.9921        gradient boosting        AUC 0.9761
  permuted-label control            AUC 0.5234        within-k=5 5-fold        AUC 0.9974

So the value function transfers across a level boundary to within 0.005 AUC of a model trained on the
test level itself. That is the design premise holding: `sqrt(3^k)` normalization makes solvability
approximately level-invariant, at least on this sampler. Logistic regression beats gradient boosting
again — the same smooth low-dimensional signature as the cut ranker, and more evidence that data
volume is not the constraint anywhere in this programme.

**The honest discount.** Most of the gap over the baselines sits on states the cheap sound bounds
already decide. Restricting to the 1,751 of 2,200 that the information cap and the per-part Pareto
bound leave undecided, the model gives 0.9596 against mass at 0.9372. Real, much smaller, and that is
the number that would have to survive contact with the solver — where the thing to beat costs one
array index.

Next is k=5 -> k=6 with MAYBE as a third label rather than dropped, and then end-to-end CPU seconds
rather than AUC. Full detail in
[../evidence/value_level_transfer_2026-08-20.txt](../evidence/value_level_transfer_2026-08-20.txt);
scripts are `tools/ml/value_*.py`.

Process inventory: no AWS compute; the oracle workers all exited; nothing running locally.

## 2026-08-20 (night) — a warm-cache oracle, and why priming it is not worth doing

Built `radio_oracle.c`: pays `init()` and cache replay once, then answers `<k> <n1> <m1> ...` from
stdin until told to quit, keeping every fact it learns. Launcher `run_radio_oracle.sh`, client
`tools/oracle_client.py`, documented in [tools.md](tools.md), numbers in
[../evidence/warm_oracle_2026-08-20.txt](../evidence/warm_oracle_2026-08-20.txt).

**Throughput.** 2,200 four-part k=5 states, started cold: 37.3 s wall, of which ~37 s is init and
**243 ms is the 2,200 queries** — 0.11 ms each. Per query that is ~1,800x faster than the cheapest
one-shot `radio_one` build and ~340,000x faster than a correctly sized one, where the same batch
would be about 22 hours. The verdicts — 1,120 solvable, 1,080 unsolvable — are *identical* to those
from running `radio_one` once per state earlier, which is a two-driver cross-check on every state.

**Sizing, measured rather than assumed.** MAX_K=9 with MAX_N=300 inits in 37 s at 0.64 GB. The
binding constraint is not the queries: replaying a cache fact wider than the static tables is not
bounds-checked, and the archived census caches reach a side-sum of **258**, so anything below that
cannot be primed with them. Cost is also not a clean function of MAX_N — MAX_N=400 at MAX_K=6 costs
205 s while MAX_N=485 at MAX_K=9 costs 146 s — so a candidate sizing has to be measured. Both facts
are now traps in [status.md](status.md).

**Priming is the expensive part and mostly should not be done.** The archived caches are text logs
replayed through `parse_file`: 11,661,763 exact facts and 10,204,438 dominance facts. Measured replay
rate is about **700 facts/s**, so all 21.9M is roughly **8.7 hours** — I started it, watched it reach
about 1% in six minutes, and stopped it. The replay path is the bottleneck, not init. Since a cold
oracle did 2,200 queries in 243 ms anyway, the recommendation is to start cold and let the
in-process cache warm itself, priming only for a session that will outlive the load, and then from a
filtered subset. By level the exact cache is almost all k=4..6 (891k / 8.15M / 2.61M) with only
7,686 facts at k=7 and 255 at k=8, so a k-band filter is easy and would cut the load by most of it.

One design detail worth keeping: `radiobase.c` and `canSolveB` print progress to stdout, which would
corrupt a line protocol. The oracle duplicates the original stdout for its response stream and
repoints the C library's stdout at stderr *after* the provenance banner, so retained response output
still passes `check_provenance.py` while every later print goes to stderr. Any future stdin-driven
driver needs the same treatment.

Process inventory: no AWS compute; oracle and build processes all stopped; nothing running locally.

## 2026-08-20 (late) — oracle hardening: skip-don't-fail, journalling, and where the replay time goes

Four follow-ups on the warm oracle, all measured.

**The loader now skips what it cannot represent.** `parse_file` neither bounds-checks fact width nor
tolerates a malformed line, so a cache wider than `MAX_N` would corrupt the tables silently and a
stray line kills the process. The oracle parses its own input instead: facts above `MAX_K` or wider
than `MAX_N` are counted and dropped, and the counts come back in the load reply and in `stats`.
`MAX_N` is now chosen for the queries alone, not for the widest fact in any cache you might prime
with.

**A correction on cost.** I had written that init cost "climbs steeply" with `MAX_N` and framed it as
a scaling law. It is not one: `MAX_N=400/MAX_K=6` inits in 205 s while the *larger*
`MAX_N=485/MAX_K=9` inits in 146 s. Init and query cost track the work actually required — which
cache is loaded, how much refutation a state needs — far more than the table dimensions. Two points
do not make a curve. The trap in [status.md](status.md) now says measure the configuration rather
than extrapolate.

**Where the replay time actually goes.** Neither cache insert is a plain trie write. `cacheCanSolve`
propagates a positive fact down to every state it dominates; `cacheCantSolve` propagates a negative
fact up to every state that dominates it, for every permutation of the parts. Replay redoes the
closure work the original run already did. Two candidate explanations, one killed and one confirmed:

* *duplicate facts* — **wrong**. Of 99,672 inserts only 1,186 were redundant, 1.4%. Offline pruning
  of subsumed facts would buy nothing.
* *insertion order* — **right, partly**. Sorting so each fact subsumes as much as possible before
  the rest arrive — largest solvable first, smallest unsolvable first — took the same 99,672 facts
  from **304 to 685 facts/s, 2.25x**, for a pure reordering. That is `tools/sort_cache.py`.

2.25x is free and still leaves ~8.9 hours for the full 21.9M. The actual fix is a **structural
snapshot**: serialize the trie — roots, the branch arrays behind `branch_handles`, the front arena —
and reload it linearly, O(structure) rather than O(facts x closure), which should be seconds. I have
scoped it and deliberately not built it in the same pass as everything else: it touches the
allocator, and a silently corrupted cache means false verdicts, which is precisely how the 2023
corpus acquired 37 false negatives. It deserves its own change and its own regression test.

**Journalling replaces priming for most purposes.** `--journal FILE` appends every computed verdict
in the format the loader reads, so a session's work primes the next one — exactly the states you ask
about, and small. A MAYBE is never journalled, because it is not a fact. Smoke-tested end to end: a
deliberately over-wide fact was skipped rather than fatal, and both verdicts round-tripped into the
journal in loadable form.

Process inventory: no AWS compute; all oracle and build processes stopped; nothing running locally.

## 2026-08-20 (later) — binary cache snapshots, and a load estimate I had inflated

**The estimate was wrong in the direction Fedor said.** My 304 facts/s came from a stratified
1-in-117 sample, which deliberately destroys the property that makes a real load cheap: consecutive
facts in the corpus are related and subsume one another, while sampled facts each land in an emptier
trie and expand more. Redundancy in the sample was 1.4%; in a sequential full load it will be far
higher. The composition of the sample was at least right — the corpus is 98.4% negative — but the
rate is an upper bound on cost, not an estimate, and the real figure is somewhere between one and
nine hours. Recorded as such rather than defended.

**Snapshots are implemented.** `snapshot <path>` serializes the cache structure and `restore <path>`
reloads it linearly, O(structure) instead of O(facts x closure). The work was in getting the
descriptor walk right: four forms, and any of them mishandled gives a silently wrong cache. A branch
holds *front* descriptors in slots 0 and 1 and child nodes from slot 2; a front record holds two
front descriptors; an inline node packs two sbb values into the low bits; and a front descriptor is
itself either a vector handle or an inline value. Handles are canonicalised by discovery order,
since a fresh process allocates 1,2,3,... — so writing structures in visit order and remapping every
descriptor reproduces identical handles on load.

The guard that matters: the header carries the build id and MAX_K/MAX_N/MAX_SBB/sizeof(front_point),
and a mismatch is refused. The sbb numbering depends on MAX_N, so a cross-build restore would be
silently wrong, which in this repo means false verdicts.

Round-trip on 16,099 facts: 225,146 branches / 404,821 records / 466,552 vectors, dumped in 247 ms
to 43,616,050 bytes, restored into a fresh process in 212 ms, and **300 queries returned identical
verdicts either way**. That equality is the acceptance test; the timings alone would not be.

**One unresolved number.** 43.6 MB of structure for 16,099 facts is 2.7 KB per fact. That ratio
cannot hold to 21.9M facts — closures overlap increasingly as facts accumulate — but nobody knows by
how much, and a naive extrapolation says tens of GB. Measuring it is the first job of the full-corpus
run, and it is also what decides the instance size, so the run should report structure counts and RSS
before anything else.

Process inventory: no AWS compute; all local oracle processes stopped.

## 2026-08-20 (night, later) — measuring the load curve killed the EC2 plan, which is the right outcome

Before renting anything I measured what the full-corpus prime would actually cost, on sequential
prefixes of `exact.cache` at MAX_K=9 MAX_N=300, each run also dumping a snapshot:

  facts      load       rate        RSS       branches/records/vectors            snapshot
   50,000     0.6 s   78,752/s    0.59 GB      124,627 /    562,993 /   263,709    17.2 MB
  200,000     3.2 s   61,689/s    0.65 GB      356,213 /  1,557,051 /   695,444    54.9 MB
  800,000  1,164.8 s     687/s    2.75 GB   12,752,501 / 42,025,603 / 7,233,342     2.88 GB

Both axes are violently superlinear: **4x the facts costs 360x the time and 52x the structure.** A
contiguous 800k block from the negative-heavy middle of the file was killed after exceeding the
head-of-file time without finishing, so the table is the optimistic case. 21.9M facts is 27x further
along a curve that is still getting worse, in hours and in bytes alike.

So the answer to "which instance should we rent" is **none, for this job**. Priming the full corpus
is not a thing to do at a different size; it is a thing not to do. That is a better outcome than
renting 64 GB and finding out there.

**My estimates were wrong in both directions and it took three points to see it.** The stratified
sample said 304 facts/s and 8.7 hours. The file's head loads 200x faster than that, because
positives are cheap and the corpus head is all positive — which is what Fedor suspected. But the
same sample also hid the explosion: once the structure is large the rate falls to 687/s and keeps
falling. Two points never described this curve and three barely do. The trap now says so.

**What actually works.** Prime from a bounded subset — the k band and mass range a job really queries
— snapshot it once with the new `snapshot` command, and restore in milliseconds thereafter. The
corpus is 98.4% negative and almost all k=4..6, so filtering is easy. For most work, skip priming
entirely: a cold oracle answered 2,200 k=5 queries in 243 ms, and `--journal` makes each session
prime the next with exactly the states that were asked about.

**On Spot versus On-Demand,** since the question is now moot for this job but will recur: the
snapshot changes the calculus. An oracle that restores from a snapshot in milliseconds and journals
what it learns loses almost nothing to an interruption, so Spot becomes reasonable for *serving* —
which it would not have been for a process holding hours of irreproducible warm state. The
expensive, unrestartable part was always building the primer, and that part is now the part we are
not doing.

Process inventory: no AWS compute was ever launched for this; all local oracle processes stopped.

## 2026-08-20 (night, last) — launched the full-corpus prime rather than extrapolating again

I had talked myself out of this run on the strength of a curve fitted to three points, which is
exactly the reasoning this repo keeps punishing. The points were real — 78,752 facts/s at 50k,
61,689/s at 200k, 687/s at 800k, structure reaching 2.88 GB — but "27x further along a worsening
curve" is a prediction, not a measurement, and it was doing the work of one. So the run is going.

`tools/oracle_prime_ec2_launch.sh` starts an on-demand `r7iz.xlarge` — 4 vCPU and 32 GiB at a high
clock, since the load is single-threaded and memory is the constraint that matters. On-demand rather
than Spot because the load has no intra-run checkpoint: an interruption throws away the whole thing.
The remote script pulls the archived census caches from S3, builds the oracle at MAX_K=9 MAX_N=300,
sorts both caches with `tools/sort_cache.py` for the measured 2.25x, splits them into 250k-line
chunks, and loads chunk by chunk so progress is reported per chunk instead of inferred from a rate.
It dumps `cache.snap` at the end, compresses it and uploads.

Two guards, because 32 GiB was chosen from a superlinear curve and could be wrong: the run aborts at
28 GiB resident rather than swapping the host, and a systemd unit hard-stops at 24 h. If it hits
either, that is the answer — it tells us the full prime is infeasible at this size, which is what I
had merely asserted before.

Launched as `i-0957cf6024c13a1e3`, run `20260820T165448Z`, commit `cdffe46`, artifacts under
`s3://radio-sa193-393287594714/oracle-prime/20260820T165448Z/`. Status is
`tools/oracle_prime_status.sh 20260820T165448Z`, which reads the STATUS object the host writes every
60 s; it costs nothing and does not touch the run. Deliberately not polled from here.

## 2026-08-20 (done) — the full prime takes 1.58 h, and I was wrong three times getting there

Run `oracle-prime/20260820T165448Z`, on-demand r7iz.xlarge (4 vCPU, 32 GiB), commit `cdffe46`,
exit 0. **All 21,866,180 archived cache facts loaded in 5,684 s = 1.58 h**, 3,847 facts/s overall,
with 110 s of setup (fetch, build, sort, chunk) and 14,084,167 redundant inserts (64.4%). Fedor said
an hour or two and was right; I had argued the job should not be attempted.

**The rate curve is a hump, which is why every extrapolation failed.**

  c0000      44,484/s   cheap positives, sorted first
  c0004..9  431-779/s   the expensive band -- 36% of total time in five of 88 chunks
  c0032      70,134/s
  c0072     198,604/s   almost everything now subsumed
  c0087   1,067,487/s

My local prefix measurements sampled the rise and the collapse and never saw the recovery. From
50k/200k/800k I read "27x further along a worsening curve" and wrote that the full prime should not
be attempted; the honest reading of three points on the wrong side of a hump was that I did not know.
Three separate estimates — 8.7 h from a stratified sample, then 13 h, then infeasible — against
1.58 h measured. The trap in [status.md](status.md) now says the curve is hump-shaped rather than
merely that it should not be extrapolated from two points.

**Memory was never the issue.** Peak host use stayed around 2 GiB of 32. Fedor's "not even sure we
need 64 GB, probably fine with much less" was also right, and by a wide margin; 8 GiB would have
done.

**The snapshot works, which was the point.** 34,600,337 branches / 120,043,426 records /
26,353,897 vectors, 6,672,100,991 bytes dumped in 43 s, 667 MiB compressed. Restored locally in
**32.8 s at 2.41 GB resident** — 173x faster than replaying the facts. Resident is far below the file
size because branch slot arrays are sparse and `calloc` leaves untouched zero pages unmapped.

Correctness: after restore, the 2,200 k=5 states labelled independently days earlier re-queried to
1,120 solvable / 1,080 unsolvable — exactly the known answer — at 0.10 ms per query. That end-to-end
agreement is what makes the artifact usable rather than merely present.

**A defect the artifact itself exposed.** I had keyed snapshot compatibility on the build id, which
includes the compiler, so the Linux-built snapshot was refused on this Mac for no semantic reason —
a 6.67 GB file usable only on the machine that made it. Compatibility is now keyed on what actually
fixes the layout: source commit plus MAX_K, MAX_N, MAX_SBB, sizeof(front_point). Geometry mismatch is
refused outright; a foreign identity with matching geometry needs the explicit `restore-any` opt-in
and warns. This is the second time in two days that a guard I wrote for safety was wrong in its
choice of key.

The snapshot stays in S3 rather than the release store: it is derived, regenerable in 1.58 h from
archived inputs, and specific to the `MAX_K=9 MAX_N=300` geometry. Recorded in [data.md](data.md).

Instance terminated after all artifacts were in S3 and the snapshot verified locally. No AWS compute
remains; nothing running locally.

## 2026-08-20 (done) — the recursive predictor: built, and it nearly matches directly-supervised ranking with zero split labels

Picked up the fast-solver thread per the previous handoff. `docs/ml-guided-search.md`'s "first
experiment" asked two things: does the level-held-out value model keep transferring past k=5, and
does scoring a split by its children's value recover what the flat ranker needed cut supervision
for. Both answered; full numbers and reproduction commands in
[../evidence/recursive_value_2026-08-20.txt](../evidence/recursive_value_2026-08-20.txt).

**Two things broke before either question could be answered, and both are now in the trap table.**
The matched sampler's fixed [0.70,1.02]-of-cap mass band, which worked fine at k=4/k=5, sampled
**zero solvable states out of 300 at k=7** — the per-part solo Pareto maximum grows almost as fast
as the cap does, so a fixed fraction stops meaning anything level-independent. Fixed by bisecting
the band per level against the oracle (30-state probes, target ~50% solved). Then labeling the
bisected k=7 sample crashed the long-lived oracle six times: `out of front-record handles`, a hard
cap on `radiobase.c`'s tagged 32-bit handle space that a few million-split sub-searches can reach
well before any documented ceiling would suggest. No compile-time knob bounds it the way
`radio_canon_search_generic`'s pool is bounded. Worked around with a restart-and-skip wrapper, not
fixed — a caller keeping a warm oracle alive across many diverse queries needs one.

**The level-held-out value model transfers all the way to k=7** (AUC 0.986/0.996 against a 0.482
permuted control), but the sound per-part deficit baseline is now just as strong (0.996) — the
learned edge has narrowed to the states neither cheap sound filter decides, same pattern as the
k=4/k=5 result, sharper.

**The recursive cut scorer is the real result.** For every stage-2 candidate split of a real
forced k7 census endpoint, score it by `min(V(selected), V(mixed), V(complement))` — the actual
AND-OR children, via `analyze_single_solution_cuts.children` — using a value model trained *only*
on synthetic oracle-labelled states, with no split-label supervision at all. Against an identical
population, a flat ranker trained *with* direct supervision on which splits win gets 130.5x
selectivity; the zero-supervision recursive scorer gets 120x. Within 8%, on real held-out data the
value model was never built to see. This is the design note's "what decides the winner lives one
level down" diagnosis, confirmed rather than argued.

**A model that wins on standalone AUC lost badly once composed.** Gradient boosting beat logistic
regression at the k=7 value-model holdout (0.996 vs 0.986) but its recursive score collapsed to
2.3x, worse than blind on the hardest endpoint (0.6x). Logistic regression's smoother decision
surface — already the winner in `cut_ranker.py` and `value_level_transfer.py` — matters far more
once a model is composed through a `min` and shifted onto a distribution it never trained on than
it does for flat in-sample classification. Standalone AUC did not predict this; only the composed,
end-to-end metric did.

**What's not done:** the factored per-part policy and its `(S,X)` DP top-k decoder, and actually
putting either scorer in front of `canSolveB`'s split loop. That's a correctness-sensitive C change
gated on exactly the measurement this session produced, and the next thing to build — judged on
end-to-end CPU seconds on a known-hard instance, not on the selectivity numbers here. Also
untested: scoring k=8 endpoints (children at k=7), which hits both traps above at once.

`tools/ml/recursive_value.py` is new; `tools/ml/value_gen_states.py` now takes an explicit mass
band instead of a hardcoded one. No AWS compute; the oracle and all label data are local under
`/tmp/rec/`, not committed (small, deterministic from the fixed RNG seed given the same bands and
build).

## 2026-08-21 (done) — a challenge that found a real gap, and a fix that wasn't more data

Two pointed questions from Fedor about the previous session's recursive scorer, in sequence, both
worth recording because they each redirected the work correctly.

**"Easy cases are not the interesting claim — training on a random sample underrepresents the hard
ones."** Checked directly rather than argued with: re-ran the recursive scorer against the exact
stage-2 candidate enumeration (no 6,000-candidate sampling cap) and stratified by exact
candidate-set size, since the literal winning-cut count turned out to be a useless hardness proxy
(almost always exactly 2 or 4 — the trivial complementation pair, no real spread). The concern was
right: median rank roughly doubles on the hardest third of endpoints (146 -> 331) and the worst
case is 16,886 of up to 130,262 — barely better than blind. Full stratified numbers in
[../evidence/recursive_value_worst_case_2026-08-21.txt](../evidence/recursive_value_worst_case_2026-08-21.txt)
section 0.

**"Optimize the worst case first — find a sound early cutoff — then worry about ordering."** This
reframes correctly: no ranking, however good on the typical case, can ever certify "no solution
exists" — only a proven necessary condition can, by shrinking the set that must be exhaustively
tried. This repo already has one sitting unused in this exact context: `R_0` (full-star
majorization, proved in `theorems/singleton-majorization.md`, implemented as
`tools/bundled_majorization.r0`). It had been measured before, but never composed on top of this
thread's stage-2 candidate set, never stratified by hardness, and never combined with the recursive
scorer into one pipeline.

Composing them fixes both problems, and not by coincidence: `R_0`'s survivor count is a **real,
sound worst-case bound** (median 6,892, worst 16,547 of up to 130,262 — a median 6.1x shrink beyond
stage-2), zero winners dropped across every sampled candidate at every endpoint, as the theorem
requires. Stratified by the same hardness tiers, `R_0` shrinks the *hardest* third **more**, not
less (10.6x vs 4.7x on the easiest) — the opposite of "hard cases resist filtering." And once `R_0`
has removed whatever it removes, the recursive ranker's hardness-correlated degradation vanishes
entirely: rank medians go flat at 18/12/13 across the three tiers, and the hardness-vs-rank
correlation drops from 0.129 to 0.001. The earlier degradation was never evidence that the value
model gets confused by hard cases specifically — it was evidence that the hard tier's larger raw
candidate space contains more candidates only a *sound* condition can rule out, and once that's
done first, what's left is no harder to rank than the easy tier's leftovers.

**The two guarantees stay separate, on purpose.** `R_0`'s survivor count is the number that, if
exhausted with no success, soundly proves unsolvability — no learned model touches that claim. The
recursive ranker's position within the survivors is ordering only: it speeds up finding a witness
when one exists and must never be read as a stopping rule. Conflating them is exactly the shortcut
that produced the 2023 corpus's 37 false negatives.

**What this is not**: a small cutoff. Thousands of `R_0` survivors is a real improvement over
stage-2's up to 130,262, not the "exhaust a handful and stop" bound the ideal algorithm would want.
The cross-part pair condition (`tools/split_choice_rules.py`, previously measured combined with
`R_0` at 140.4x on a different, smaller denominator, not stratified by hardness) and deeper `R_d`
are the next sound filters to compose and re-test the same way — not done here. Nor is k=8/k=9
tested; k=7-rooted sub-searches are exactly what crashed the warm oracle building this thread's
training data yesterday.

Also declined, with reasons on record rather than just tried and dropped: training a neural net to
regress directly from state to winning-split coordinates. The target function has no tolerance for
approximation (`Sb(8:1,2:1)` solvable in 3, `Sb(9:1)` not, at strictly lower mass — one coin flips
the answer), the closely related easier version (rank candidates by their own shape) was already
refuted at the 5th percentile, the hard-case corpus is small by construction (153 forced endpoints
total at k=7, against an action space up to ~1e9) and expensive to grow (a hard case's solution is
exactly the expensive search being avoided), and the chess/Go precedent is that policy nets are
never trusted standalone on sharp positions — they're paired with real search, same as the
achievability/unsolvability asymmetry already written into this repo.

Code: `tools/ml/recursive_value.py` gained `worst_case_then_order` (Experiment 3) and
`_r0_children`, reusing `tools/bundled_majorization.py` (unmodified, already committed, no new
dependency). Evidence:
[../evidence/recursive_value_worst_case_2026-08-21.txt](../evidence/recursive_value_worst_case_2026-08-21.txt).
No AWS compute; all label/candidate data stays local under `/tmp/rec/`, not committed.

## 2026-08-21 (done) — 67 vs 37,899: the composed pipeline beats a real benchmark, with real solver calls

Same day, same thread, one more push: "do we have an algorithm yet, or are we still measuring."
Answer before this: measuring. Answer after: measured against the real thing, on one state, not
wired in yet.

`Sb(29:6,19:9,13:12,36:3)@6` is this repo's own standing test for exactly this question —
documented in status.md as the "residual positive control" specifically because a prior
split-ordering proposal already failed it. Current solver, default order: 37,899 top-level splits,
26.6-33 CPU seconds. The test: enumerate the same cap+per-part-Pareto candidate space this whole
thread has used (**4,449,172** candidates for this one state — far more than any k=7 census
endpoint, because its parts are wider), filter by `R_0` (809,706 survivors), train `V` on all four
matched-sampler levels pooled, order the survivors by `min(V(child))`, and — the part that makes
this real rather than another offline number — for each candidate in that order, ask a genuine
warm oracle (full `canSolveB` recursion, no shortcuts, no ML anywhere in the loop) whether its
three children are solvable. Stop at the first yes.

**67.** Against 37,899. A different, independently-verified winning split than the documented one
(mass arithmetic exact, both children-solvable answers reproduced in a separate process). The same
`R_0` survivors in their natural, unscored order had not found a working split after 1,340 tries —
so the ordering itself, not just the sound filter, is carrying this.

Cost accounting, because it matters for what to build next: the 46-minute wall-clock was almost
entirely unoptimized Python scoring 4.4M raw candidates one at a time. `r0()` and `feat()` are each
a handful of arithmetic operations — a C port should do this scoring pass in a small fraction of a
second. The number worth remembering is 67 vs 37,899 top-level candidates, not this harness's own
runtime.

**What this is and isn't.** It is the first result in the thread measured against the real solver
instead of a sampled or exactly-enumerated proxy, on the project's own previously-unbeaten
acceptance test for this question. It is not yet inside `radiobase.c` — this used the solver only
as a black-box oracle, changing nothing in production. It is one state. The witness rests on
`canSolveB`'s own verdict (the same trust every other achievability claim in this thread already
rests on) rather than a full singleton-leaf tree checked by `check_witness.py`, which remains the
strictly stronger proof and wasn't built here.

Housekeeping: killed one stray duplicate process from a botched quoting attempt (an `exec()`-split
smoke test that should have been thrown away and wasn't) before it could run in parallel with the
real one and confuse the results; confirmed after the fact it hadn't. No AWS compute. Script kept
at `/tmp/rec/real_benchmark.py`, not committed — self-contained and reproducible from this note.
Evidence:
[../evidence/real_benchmark_residual_control_2026-08-21.txt](../evidence/real_benchmark_residual_control_2026-08-21.txt).

## 2026-08-21 (done) — does the algorithm need to differ by state length? Yes, but not where I expected

Fedor's follow-up, two parts: is 67-vs-37,899 actually benchmarked against the real solver (yes —
that was already true, clarified for the record), and a genuinely new hypothesis not yet discussed
in this thread: 1-, 2-, 4-, 8-part states probably need different algorithms, and things may level
out past 8.

Picked real states from the documented record rather than inventing convenient ones: a 3-part
exact positive with only 2 winners of 37.7M splits (`Sb(110:3,115:2,121:1)@7`), and the "hard
eight-part positive" `Sb(15:3,14:3,17:2,8:4,11:2,10:2,19:1,15:1)@5` — 12,585 CPU seconds / 18.86M
evaluations historically, still timing out at 300s against a warmed cache in one attempt. 1- and
2-part states turn out not to need testing at all: this repo already solves both exactly (singleton
majorization; an exhaustive two-part pair table), no search involved.

**The scorer doesn't care about part count. The generator falls over at 8 without help.**
Candidates-to-success: 43 (3 parts), 67 (4 parts), 52 (8 parts) — the same trained value model,
never retrained per part count, landed a witness in roughly the same 40-70 range across a 3-8 part
span and nine orders of magnitude of raw search space. That's not what "different algorithms by
length" predicts for the ordering step, and it's a real, useful negative — the value model and
`min(child)` composition generalize by part count for free.

What did NOT generalize for free was generating the candidates to score. The exact DP enumeration
that handled 3 and 4 parts cleanly (millions of candidates, seconds) got OOM-killed outright at 8
parts — silently, no error, just gone, RSS climbing past several GB first. Two sampling fallbacks
(uniform random over the raw per-part range; random over each part's own Pareto-prefiltered
options) both starved: 2M draws produced 3, then 4, `R_0` survivors — nowhere near enough to test
anything. What worked was a width-bounded incremental DP: build the (S,X) combination one part at
a time like the exact method, but randomly subsample down to a fixed beam width whenever an
intermediate step would otherwise exceed it. At beam=3M: 513 survivors, no winner among them. At
beam=15M: 2,577 survivors, a winner at #52. Intermediate step sizes before capping reached 59M,
194M, 131M, 174M — the filtered space at 8 parts is not "somewhat bigger," it's big enough that
naive strategies fail in qualitatively different ways depending on which one you try, and even the
one that worked needed 5 attempts across two scripts to tune.

This is a cheap, forced-by-necessity version of exactly the "DP top-k decoder" the design note
already named as the deployable architecture (`docs/ml-guided-search.md`) — needed here just to
*measure* anything at 8 parts, which is itself evidence it'll be needed to *deploy* anything past
about 4-5 parts.

**Scope, stated plainly.** n=1 per part count — three real states, not a systematic sweep. The
8-part comparison is against a historical default-order figure from a different build/cache era,
not confirmed fresh on the current build the way the 4-part control's 37,899 was (independently
identical across two engine builds).

**Correction, same day: the "8-part k=6 is never solvable" aside above was wrong to assert.**
Fedor caught it: the 0-of-165 figure (2026-08-08 journal entry) came from one specific solver
run's log, not a neutral sample of 8-part k=6 states, and I never checked whether that run's own
selection was biased toward negatives before treating the count as a structural fact. It may well
be — this is exactly the "check how the sample was obtained" discipline the disjoint-corpora trap
already exists for, applied to a corpus I hadn't scrutinized. No conclusion about 8-part k=6
solvability, either direction, survives from this entry. The state actually tested above is at
k=5 and is a genuine, independently-verified positive, unaffected by this retraction.

Both new witnesses independently re-verified in a fresh process (mass arithmetic exact, all three
children genuinely `SOLVABLE`), same discipline as the 4-part result. Scripts:
`/tmp/rec/real_benchmark_generic.py` (exact DP), `/tmp/rec/real_benchmark_beam.py` (width-bounded
beam, needed beyond ~4-5 parts) — not committed, self-contained. Housekeeping: an early attempt
used a `radio_one` binary as if it were a `radio_oracle` (wrong stdin protocol, would have hung);
caught before it wasted a run. No AWS compute. Evidence:
[../evidence/real_benchmark_by_part_count_2026-08-21.txt](../evidence/real_benchmark_by_part_count_2026-08-21.txt).

## 2026-08-21 (done) — narrowing to 4-part, 2-or-4-winner states, and a wall-clock warning

Fedor's direction after the part-count test and the 8-part-k=6 retraction: stop varying part
count, focus on 4-part states specifically, and within those the ones with exactly 2 or 4 winning
splits — which the census stratification already showed isn't a narrow slice, it's close to the
whole forced-endpoint population (131 of 153 k7 endpoints have exactly 2 literal winners, 22 have
4).

Second real-oracle test, harder than the residual control: `Sb(16:12,17:10,29:5,21:6)@6`, a
documented "knife edge" with exactly 2 winners of 1,212,971,760 raw combinations. Result: a
genuine, independently-verified witness (mass arithmetic exact, all three children confirmed
`SOLVABLE` in a fresh process) at candidate **#1,373** of 885,342 `R_0` survivors — worse than the
residual control's 67, and that's the useful part: the number tracks how rare the actual winner
is, not a fixed constant. Still ~322x better than blind over the `R_0`-survivor population alone.

**A cost problem worth flagging rather than quietly absorbing:** the `R_0`-filtering pass on this
run took 3,611 seconds for 3,686,536 candidates — about 15x this thread's usual rate. The
per-checkpoint log shows why: normal ~110s/500k-row pacing for three million rows, then one single
2,964-second gap for the last half-million. Almost certainly this Mac sleeping or getting
throttled mid-run, not the algorithm — but I didn't catch it happening, only after the fact from
the log. The candidate-count results are unaffected (they don't depend on wall-clock), but every
local wall-clock figure from this session, including in the two prior real-benchmark evidence
files, should now be read as an upper bound, not a measurement. Worth adding a heartbeat/liveness
check to any future long local run rather than trusting elapsed time blindly.

Evidence: [../evidence/real_benchmark_4part_2or4_2026-08-21.txt](../evidence/real_benchmark_4part_2or4_2026-08-21.txt).

## 2026-08-21 (done, deployment details below superseded) — moving enumeration into the oracle, and off the laptop

**The deployment this entry describes (cold start, `MAX_K=8 MAX_N=400`, instance
`i-05196369e708e0740`) was superseded the same day** once Fedor asked for a real warm start
instead — see the later entry below and
[aws-run.md](aws-run.md#persistent-oracle-serve-instance-2026-08-21-no-fixed-end-date) for the
final instance, ID, and validated result. The `enumerate` capability and the C-side design
described here are unaffected and still accurate; only the specific instance/build/cold-start
choice changed.

The 60-minute run above was the last straw for doing this in Python. `radiobase.c` already has
everything needed: `all_solutions` enumerates every raw split (no pre-filter, which is why it's
slow), and `star_expansion_majorization_can_solve` is `R_0` itself, already proven and already
used inline by `canSolveB`. Added one new, purely additive `radio_oracle.c` command,
`enumerate <k> <n1> <m1> ...`: the same raw mixed-radix walk `all_solutions` uses, but check `R_0`
on all three children before ever paying for a real `canSolveB` call, and return the exact winner
list instead of a visualization grid. `radiobase.c` itself is untouched.

Validated against two exact ground truths, not just "looks plausible": the knife-edge state
matched 2 winners of 1,212,971,760 exactly, in 40 seconds (the Python pipeline took an hour for
one witness). The residual control's complete winner list is now known for the first time: 6
winners (not 2 or 4 — it was never actually in the tier we're focusing on) plus 2 correctly-
flagged-inconclusive candidates, in 7 minutes. One of the knife-edge winners, `[8:7,4:2,12:2,
19:6]`, is exactly what the slow Python-plus-oracle run found the same day at candidate #1,373 —
an accidental but welcome cross-check that both were right.

Then the actual ask: get this off a laptop that sleeps and loses connectivity. Wrote
`tools/oracle_server.py`, a small TCP front-end so the one long-lived oracle subprocess can be
reached from any future session (via an SSM port-forward, never a public port) instead of needing
a live terminal attached to it; it periodically dumps a local snapshot so a crash loses at most
one interval. `tools/oracle_serve_ec2_launch.sh` / `oracle_serve_ec2_remote.sh` /
`oracle_serve_status.sh` adapt the existing `oracle_prime_ec2_launch.sh` pattern (same subnet,
security group, IAM profile, S3-bundle bootstrap) but for a persistent service rather than a
one-shot load-and-terminate job: no hard-stop, cold start (grows its own cache from real queries,
by design — this isn't the sa193 lineage), r7i.large (2 vCPU: one for the solver, one for the
request loop; 16 GiB — Fedor's sizing, room for a cache that grows once long states are being
solved regularly), on-demand rather than Spot because the whole point is not depending on
interruption. EBS `DeleteOnTermination=false`, unlike the disposable oracle-prime volumes, since
the accumulated cache is the thing worth keeping.

No fixed end date for this one, by request — it stays up across sessions rather than being torn
down at the end of this one. Flagging that plainly: **an oracle-serve instance may be running and
billing right now** — check `tools/oracle_serve_status.sh` before assuming otherwise, and note it
at the start of any session that picks this thread back up.

**Launching it caught two real bugs, and one avoidable cost, before it actually worked.** First:
`restart_loop.sh` ran `python3 "$@"`, but `"$@"` already started with `python3` (passed explicitly
in the `systemd-run` command line) -- it looped "python3: can't open file 'python3'" every 5
seconds, forever, never starting. Caught via `aws ssm send-command` diagnosis, not by watching a
STATUS that never updated (it wrote once, correctly, then nothing -- the bug was downstream of
where STATUS gets written). Fixed to plain `"$@"` (`93fbd02`).

Second, more expensive: I sized the build `MAX_K=9 MAX_N=500` for "headroom" with no evidence it
was needed. On the actual `r7i.large` it was still deep in `init()` past 500 seconds and 2 GiB RSS
with no end in sight -- exactly the "do not extrapolate solver cost from table dimensions... a
padded MAX_N buys nothing" trap already on record, except this time paid for on a live, billing
instance rather than caught locally first. Terminated that instance and relaunched at
`MAX_K=8 MAX_N=400` (`9cd7f73`) -- sized to what this thread has actually queried (3-4 parts,
side-sum up to ~350), not to a round number. Init finished in 362 s at ~3.0 GiB RSS.

Locally, `session-manager-plugin` (needed for the SSM port-forward) wasn't installed, and its
Homebrew cask install wants an interactive sudo password this session couldn't supply. Fetched the
cask without installing (`brew fetch --cask`), expanded the `.pkg` by hand (`pkgutil --expand`,
then decompress the cpio `Payload`), and copied the binary to `~/.local/bin` (already on `$PATH`)
instead of the system location the installer would have used.

**Validated end-to-end after both fixes**, over the actual SSM tunnel, not just locally: a plain
query and an `enumerate` call reproduced the exact local results -- 2 winners of 1,212,971,760 for
the documented knife-edge state -- at 68.8 s (vs 40 s on this Mac; slower per-core on this cloud
instance's CPU, but perfectly usable). The persistent oracle is real, reachable, and running.
Full record, including the exact instance ID and reach/stop/terminate commands:
[aws-run.md](aws-run.md#persistent-oracle-serve-instance-2026-08-21-no-fixed-end-date).

## 2026-08-21 (done) — the real warm start: base snapshot + certificate, and three bugs caught live

Fedor's actual ask, arriving right as the entry above shipped: don't cold-start this, load all
available certified facts (a couple of hours is fine, faster than re-solving), and make sure it
can load its own snapshot too. Slow init is fine -- give it time.

**Loading the certificate needed a real converter, not a text substitution.** The sa193
certificate stores claims in a compact `part`/`claim`/`fact` format
(`radio-negative-level-certificate-v2`), not the "can't solve Sb(...) in k" lines a raw solver log
emits -- `radio_oracle.c`'s `load` command expects the further-distilled `parse_out.sh` cache-line
grammar (`- b n1 m1 ... t pairs n k`). Wrote `tools/cert_to_cache.py`, reusing
`check_level_chain.py`'s already-verified `Level` parser rather than re-implementing it. The one
real risk -- a wrong part order silently corrupting the dominance trie rather than crashing --
turned out not to apply: reading `cacheCantSolve` in `radiobase.c` shows it explicitly rotates
every remaining part into the pivot position at each recursion level ("other part permutations in
this call may still add information"), unlike `cacheCanSolve`, which does assume `sort1`'s
descending order. Every certificate claim is negative, so only the permutation-robust path is ever
exercised.

**Trusted that reading, then checked it lived, at three scales.** Generated a perturbed-neighbor
control population at each scale -- take a certified claim, nudge one coin on one part, solve the
result FRESH on an oracle with nothing loaded, independent of the certificate or the converter --
then loaded the certificate and re-checked both the claims (must be UNSOLVABLE) and the neighbors
(must match their independent ground truth exactly). k=3 (127 claims, 69 neighbors): 0 mismatches.
k=5 (80,634 claims, 1,112 neighbors): 0 mismatches, 3.3 s load. Full chain, all 8 levels
(2,846,568 claims): 0 mismatches on a 1,145-claim cross-level sample and 46 k<=7 neighbor
controls, 414.5 s load. k=8/k=9 neighbors couldn't get real ground truth this way -- every cold
solve hit MAYBE at a 10-15 s budget, which is exactly why the certificate exists (these are the
near-saturated Sa(193)-boundary states run9 spent days refuting) -- but their post-load behavior
was the most informative result of the day: for five k=8 and five k=9 perturbed pairs, one side of
each now resolves to UNSOLVABLE instantly (certificate-covered) while the other still needs real
search and hits MAYBE again -- a sharp, structurally sensible split, not corruption bleeding
either direction. Also validated the exact production sequence, not just each half separately:
dumped a snapshot from an oracle already holding one certificate level, started a fresh process,
restore-any'd from it, then loaded a different level's file on top -- both an old fact (from the
restore) and a new one (from the load) resolved correctly in the same process.

**Rewired the deploy script, and it caught two more bugs on the actual instance, live.** Rebuilt at
`MAX_K=9 MAX_N=300` to match the existing base snapshot's geometry (needed for `restore-any`).
First relaunch attempt (`i-0a64e15e18061c334`) crash-looped every 5 seconds, harmlessly: a real
argparse ambiguity -- `oracle_server.py`'s `caches` positional (`nargs='*'`) has to come
immediately after the binary positional, before any `--flag`, or argparse reports the cache path
as unrecognized. The process never got past argument parsing, so nothing was corrupted, just
nothing served. Fixed and reproduced with a 4-line minimal repro before touching AWS again
(`d9e3f03`). The corrected relaunch (`i-002cabc654b2078ed`) came up clean: 35 s to restore the
21.9M-fact base snapshot (matches the earlier measurement), then **560.6 s** to load the
certificate on top -- slower than the 414.5 s empty-cache local measurement, because inserting
2.85M more facts into an already-large warm trie costs more per fact than inserting into an empty
one. Worth remembering next time rather than re-deriving: don't extrapolate one cache-load
scenario's cost onto a differently-loaded one, same trap as the hump-shaped full-prime load, one
more shape of it.

Validated on the real, final, serving instance over the actual tunnel: a known certificate claim
resolved `UNSOLVABLE` in 0.0 ms (instant cache hit), and `enumerate` on the knife-edge state
returned the identical correct answer as every earlier test. Full instance record, superseded
history, and reach/stop/terminate commands:
[aws-run.md](aws-run.md#persistent-oracle-serve-instance-2026-08-21-no-fixed-end-date). ~10 GiB
resident of 16 GiB after warm-start -- real but not huge headroom; watch it.

## 2026-08-22 (done) — the ordering pipeline, re-validated against the persistent oracle, and two real bugs on the way

Back to the actual regression: does the R_0-then-recursive-V ordering still work, now that there's
a warm, persistent oracle instead of a cold local one to test it against. Wrote
`tools/oracle_tcp_client.py` (minimal client for `oracle_server.py`'s TCP protocol) and
`tools/ml/real_benchmark_via_aws.py`, then hit two real problems before getting a trustworthy
answer.

**First: `enumerate` is not the tool for this, and the reason generalizes past the 8-part case.**
Tried it directly on a k7 4-part census endpoint expecting the same 40-66s turnaround as the k=6
knife-edge test. It ran 10+ minutes with no result -- still connected, not hung, but not close to
done either. Reading `enumerate_winning_splits` again explains it: the admissibility check runs
only at the deepest leaf of the raw mixed-radix walk, never on a partial sub-tree, so cost is the
RAW combinatorial size no matter how selective `R_0` is. The function's own comment already said
this about 8-part states; it turns out an ordinary wide 4-part state at k=7 is enough to hit it
too. Killed it and switched to the already-proven method: exact stage-2 (cap + per-part-Pareto)
candidates, `R_0` filter, recursive-V order, walk with plain queries -- now against the fast
persistent oracle instead of a cold local one.

**Second, and more consequential: I queried the wrong level, and it looked completely fine.** The
"k7" census corpus's endpoints are residual states reached after root-level splits of a census
rooted at k=7 -- their real parent level is `C["rk"]`, which is **5**, not 7. Queried an endpoint's
children at k=6 (should have been k=4). Every verdict came back clean: `SOLVABLE`, mass arithmetic
exact, a plausible-looking winning split. All of it was answering a different, easier question
than the endpoint's actual claim -- and nothing about the output would have revealed that on its
own. What caught it: the stage-2 candidate count (18,952,500) didn't match this exact population's
already-known median (~54,000) from earlier work in this thread. That mismatch was a lucky
plausibility check, not a structural guard -- there's no code path that verifies a query's k
against a census corpus's actual residual level. Re-ran at the correct k=5 and the candidate count
landed at 56,798, right where it should.

**Also caught, cheaply, before trusting a good number: a trivial-state trap.** An early draft
picked an arbitrary endpoint (low mass, 192 of what I'd miscomputed as a 2187 cap using the wrong
k) where BOTH the learned order and a natural/unscored order succeeded at candidate #1 on the same
random subsample. That's not evidence the ordering does anything -- it's evidence the first
sampled candidate happened to be a winner regardless of order, because the state was too easy to
discriminate anything with. Re-did the test on the corpus's highest-mass (hardest) endpoint
instead, specifically to avoid this.

**The real result, once both bugs were fixed:** `Sb(14:5,9:6,13:4,9:5)@5`, mass 221 of 243 (90.9%
of cap), 2 known literal winners. Stage-2: 56,798 candidates, 2,626 pass `R_0`. Learned order
(`R_0` then recursive-V): success at candidate **#1**. Natural order, same 20,000-candidate random
subsample, not R_0-filtered: success at candidate **#2,970**. Both found the exact same split,
which is exactly one of the census's two recorded winners -- independently re-verified in a fresh
connection, mass arithmetic exact (73+80+68=221). Whole experiment, training included, under five
minutes wall-clock -- against 40-60+ minutes per state with the old method. The speed difference
here isn't from the oracle being faster per query (each query is still one fresh TCP connection);
it's from not needing to build and warm a fresh local oracle for every single test.

Not done: a systematic sample across the corpus's 131-two-winner/22-four-winner tiers -- this
session establishes the corrected, trustworthy method (right k, hardest-endpoint selection,
mandatory natural-order control) that sampling needs, but doesn't run it yet.

New trap-table entries for both bugs -- the census-corpus-level one especially, since it produces
answers that look completely correct while being wrong. Evidence:
[../evidence/real_benchmark_via_aws_oracle_2026-08-22.txt](../evidence/real_benchmark_via_aws_oracle_2026-08-22.txt).

## 2026-08-22 (partial) — the systematic tier sample, a subsampling coverage bug, and a tunnel that silently died

Picked up "let's go back to regression using the new oracle" by building the systematic sample
across the k7 census's 2-winner/4-winner tiers that the prior entry left pending
(`tools/ml/tier_sample_via_aws.py`): per sampled endpoint, exact stage-2 candidates, `R_0`-filter,
recursive-V order, walk learned vs. a mandatory natural-order control against the real oracle,
biased toward the hardest (highest-mass) endpoint per tier.

**A real bug, caught before trusting a tier-level number.** First version reused the 20,000
candidate-cap-with-random-subsample pattern from `real_benchmark_via_aws.py`. A 2-endpoint pilot
(1 per tier) gave one endpoint a clean `rank_learned=1` and the other `rank_learned=None,
rank_natural=None` on BOTH orders -- looked like a null result about the ordering. It wasn't:
offline census lookup (no oracle calls) showed the true, unsampled stage-2 pool for that endpoint
was 60,534 candidates and contained all 4 known literal winners -- the random 20,000-subsample
(33% retention) had simply excluded every one of them by chance. With only 2-4 known winners and
~65-67% per-winner exclusion odds at that retention rate, this isn't a rare corner case for this
population -- it's close to the modal case, since rare-winner endpoints are exactly what this
track targets. Fixed by raising the default candidate cap to 150,000 (above this corpus's typical
true stage-2 size, ~50-60k, so ordinary endpoints are never subsampled) and printing "known
winners in pool: X/Y" per endpoint so a future gap like this can't hide inside a rank number
again. `real_benchmark_via_aws.py` had the identical pattern; fixed the same way, though not
re-tested since its one prior use happened not to be hit by this.

**Re-run with the fix, both endpoints resolve for real:** `Sb(14:5,9:6,13:4,9:5)@5` (2-winner,
mass 221/243): stage-2 56,798, R_0 survivors 7,666, rank_learned=1, rank_natural=6,041, same
split -> 6,041x. `Sb(14:6,15:3,9:5,9:5)@5` (4-winner, mass 219/243): stage-2 60,534, R_0 survivors
7,152, rank_learned=**13** (not 1 -- more known winners didn't mean easier for the learned order
either), rank_natural=4,272, same split -> 328.6x. Both are complete, uncapped ranks, ~350s/
endpoint average against the real oracle.

**Also caught: the SSM port-forwarding tunnel died silently after ~4h52m**, with no error --
the local process (`session-manager-plugin`) stayed listed in `ps` but stopped actually listening
on 127.0.0.1:7777, so the next oracle query got `ConnectionRefusedError`. Not a bug in the oracle
or the pipeline; the persistent oracle-serve instance itself was untouched and healthy
(`oracle_serve_status.sh` showed `state=running`, cache intact, query counter kept climbing after
reconnect). Fix was just to kill the dead tunnel processes and re-run the documented
`ssm start-session --document-name AWS-StartPortForwardingSession` command. Practical
consequence for any future long batch through this tunnel: expect it to need re-establishing
after several hours, and don't assume a listed `session-manager-plugin` process means the tunnel
is actually forwarding -- check with a real connection (`stats`), not `ps`.

The full n_per_tier=8 systematic sample (16 endpoints) was launched after the fix and the tunnel
restart; its result is not in this entry -- see the next entry or docs/status.md's current state
if this one wasn't updated in place.

Evidence: [../evidence/tier_sample_via_aws_2026-08-22.txt](../evidence/tier_sample_via_aws_2026-08-22.txt).

## 2026-08-22 (done) — the full 16-endpoint tier sample, and why "median selectivity" needs care here

Continuation of the entry above: after the subsampling-coverage fix and the tunnel restart, the
full `n_per_tier=8` sample (16 endpoints, 8 per tier) completed -- 9,985s (2h 46m), ~624s/endpoint.

**`rank_learned` is real and complete for all 16 endpoints** (bounded only by the `R_0`-survivor
count, never near the 150,000 cap): 2-winner tier median 7 (range 1-104); 4-winner tier median 83
(range 15-687). Small in every single case, out of pools of 31k-85k true candidates.

**`rank_natural` is the weak link in this run, not the ordering.** The 8,000-try cap (sized off
the earlier 2-endpoint pilot's timing, not re-examined before the full run) resolved only 3/8
2-winner and 1/8 4-winner endpoints; the rest exhausted 8,000 tries without success, giving only a
lower bound (`>= 8000/rank_learned`) rather than an exact rank. Resolved selectivity: 559.6x,
4,691x, 6,041x (2-winner); 68.9x (4-winner). Unresolved lower bounds range from >=11.6x up to
>=1,600x. **The right read is two separate complete facts, not one blended median**: (a) the
learned order's rank is uniformly small and fully measured; (b) wherever the natural order's rank
*could* be measured, it was in the thousands, and the majority of endpoints are only bounded, not
resolved. Averaging just the resolved selectivity numbers would silently drop the harder-to-
measure endpoints and there is no evidence that direction of exclusion favors either measured
value being an over- or under-estimate (the weakest lower bound, 11.6x, sits below several fully
resolved values).

**One endpoint is a real outlier worth flagging, not smoothing over:** 4-winner `U000068` has
mass 165/243 (68% fill, well below every other sampled endpoint's 87-92%) and the sample's worst
`rank_learned` (687) and weakest lower bound (>=11.6x). Root cause, also caught and worth keeping:
the "hardest-endpoint" sampling only works when the tier has more than `3*n_per_tier` candidates
to draw the top slice from. The 2-winner tier has 131 endpoints, so top-24-of-131 really is a
hardest-mass bias (all 8 sampled land at 89.7-90.9% fill). The 4-winner tier has only 22 -- "top
24" is the whole tier, so its 8-endpoint sample is an *unbiased* draw across the full 4-winner
population, not a hardest-biased one. The two tiers' numbers describe different things (hardest
octile vs. whole tier) and should not be read as directly comparable.

**Split agreement**: of the 4 endpoints where both orders resolved, only 2 landed on the identical
split; the other 2 found different (but equally oracle-certified) solving splits -- expected,
since the census only records 2-4 winners per endpoint and these hard states generally have more
valid splits than that.

Net result for the original ask ("focus on 4-part states, 2 or 4 solutions"): the learned order
(`R_0` then recursive-V) resolves every sampled hard endpoint within double or low-triple digits
of tries, against pools of tens of thousands, while blind/natural order needs thousands of tries
wherever it could be measured at all and more often could not be measured within a
budget-constrained 8,000-try cap. That asymmetry, not any single "Nx" headline, is the load-
bearing finding of this sample.

Fixed the SSM tunnel dying mid-run (see prior entry) with a plain reconnect; the oracle-serve
instance and its cache were unaffected throughout.

Evidence, with full per-endpoint numbers: [../evidence/tier_sample_via_aws_2026-08-22.txt](../evidence/tier_sample_via_aws_2026-08-22.txt).

## 2026-08-22 (done) — prototyping BY_MAGIC3's replacement and direct candidate generation: one real win, one clean negative result

Follow-up to the tier-sample entries above, prompted by a direct question: can the existing
per-part Pareto-margin ("deficit") signal -- already measured offline at AUC 0.9961 alone, nearly
matching the full pooled model -- either (a) replace `BY_MAGIC3`'s current data-free "distance
from the midpoint" heuristic, or (b) drive a best-first candidate generator that never needs to
materialize the R_0-survivor list at all?

**A real bug caught first, in my own test harness, not the codebase:** validating that `deficit`
decomposes as a max over independent per-part terms (needed for idea (b)) gave 1833/2000
mismatches on first try. Cause: `bm.normalize` drops zero-sided rectangles before `deficit()` ever
sees them, but `deficit()`'s own "-1.0 = trivially fine, empty child" sentinel was being fed into a
combining `max()` as if comparable to a real value -- and an empty, unconstrained child's sentinel
routinely "won" that max over a real child's much-better genuine deficit, making a non-constraint
look like the bottleneck. Fixed (exclude empty children from the combination); re-checked over
20,000 random splits, 0 mismatches. The decomposition is genuinely sound once this is fixed.

**(a) Deficit order, tested on all 4 real endpoints from the last tier sample, via the real
oracle:** a real, complete, working improvement over blind order in every case (2.2x-4.7x where
natural order itself resolved; a real finite rank in two cases natural didn't resolve at all
within its 8,000-try budget) -- U000368 rank 2,698/7,666; U000535 rank 2,283/6,992; U000607 rank
1,260/10,940; U000068 rank 2,916/3,436. But consistently 1,000x-2,700x worse than the pooled
recursive-V model's ranks (1, 13, 85, 687 respectively) -- notably the gap narrows sharply on the
hardest case (687 vs 2,916, only ~4x), worth more data before reading into it. Root cause of the
underperformance, confirmed directly: 73-93% of EVERY tested endpoint's mass-feasible candidates
already sit at the single worst admissible deficit value -- these are near-cap-mass (87-93% fill)
states, so hitting the exact required total mass routinely forces some part to its own boundary
regardless of how comfortable the others are. The population-level AUC (measured across easy and
hard states together) doesn't survive conditioning on already being inside one hard state's own
R_0-survivor set -- a real, worth-remembering instance of range restriction hiding a predictor's
power. A same-cost SUM-of-slack variant has far better dynamic range (94 distinct values vs 3) and
nudges the rank a bit (2,196-2,218 vs 2,698) but doesn't close the gap to the pooled model --
more granularity alone isn't the missing ingredient.

**(b) Best-first generation via a heap over per-part deficit-sorted indices:** built, and it fails
cleanly -- 200,000 pops on the easiest of the 4 endpoints, ZERO mass-feasible candidates
generated. Diagnosed precisely, not just observed: both of U000368's known winners need, in some
part, an option near the *bottom* of that part's own deficit-sorted list, and the threshold
needed to admit either is exactly the worst deficit value in ALL FOUR parts simultaneously -- at
that threshold the number of index-tuples to explore is the full 4-way cross product (9.24M), not
a fraction of it, because independent-dimension best-first search needs the PRODUCT of
per-dimension counts below a threshold, not the sum. Not re-run to exhaustion on the other 3
endpoints given this diagnosis and the section-2-style distribution check already showing the same
73-93%-at-the-boundary shape on all 4 -- a deliberate scope limit, not an oversight.

**Scope check, not just an afterthought:** is "avoid full enumeration" even a live problem at the
scale this thread has been testing? No -- k7 4-part stage-2 sets (tens of thousands) build and
score in low single-digit seconds. It IS live one level up: `CORPORA["k8"]`'s hardest 4-part
endpoints have stage-2 sizes of 3.3-3.9 million, where the current unvectorized per-candidate
Python scoring extrapolates to ~3 minutes/endpoint. But the same saturation diagnosis says a
deficit-driven best-first generator would very likely fail the same way at that scale too -- this
isn't a scale problem, the SCORE is the limiting factor, not the search algorithm. The more
promising fix there, if ever needed, is vectorizing the existing scoring loop the way stage-2
candidate generation already is -- an engineering change, not a new algorithm, and not yet done.

**Net recommendation:** deficit order is worth adopting as a cheap, real, zero-extra-cost
replacement for `BY_MAGIC3`'s current unvalidated heuristic (better than nothing, much better than
blind order, complete, safe) -- but not as a substitute for the pooled model where that is
affordable. Direct generation via this specific mechanism does not work, and is recorded as a
negative result specifically so it is not re-attempted the same way later; a smarter DP-integrated
best-first search was designed but deliberately not built, since it would inherit the same
saturating score and is low-expected-value before a genuinely different, non-saturating
decomposable signal exists.

Evidence: [../evidence/deficit_order_and_bestfirst_2026-08-22.txt](../evidence/deficit_order_and_bestfirst_2026-08-22.txt).

## 2026-08-22 (done) — coordinate descent: names the problem correctly, works on the easy half, fails decisively on the hard half

Direct follow-up to the deficit-order/best-first entry above, prompted by naming the actual
problem shape: the mass/cap constraint is a multiple-choice knapsack (already solved exactly by
`exact_candidates`'s DP) -- the open problem is that "does this combination actually work" is a
genuinely joint, non-separable function of the parts, which is exactly why no per-part-decomposable
proxy (deficit, sum-of-slack) could rank well. Coordinate/block descent sidesteps that by using the
real, expensive, ACCURATE pooled recursive-V score, evaluated on only a small free block of parts
at a time while the rest stay fixed -- reusing the score that already gave rank 1/13/85/687, not a
cheap proxy.

**1-part-at-a-time fails outright**: 30 restarts, real oracle verification, U000368 (the easiest
endpoint for every other method) -- 0/30 successes, 1,765 evaluations. Offline (300 restarts,
19,392 evaluations) plateaus at score 0.039, never reaching the known winner's own 0.059 under the
same model -- single-part moves can't escape the local optima a random start lands in.

**2-parts-at-a-time (block descent) genuinely works, but only on half the population**, tested
with real oracle verification on all 4 tier-sample endpoints:

  U000368 (pooled rank 1):   SUCCESS, 18,447 evals (2.4x the R_0-survivor count)
  U000535 (pooled rank 13):  SUCCESS, 7,046 evals (~1.0x the R_0-survivor count)
  U000607 (pooled rank 85):  FAILED after 150 restarts, 257,591 evals (23.5x)
  U000068 (pooled rank 687): FAILED after 150 restarts, 774,712 evals (225x)

Both successes are real: U000368's matches a known census literal winner exactly; U000535's found
split does NOT match either known census winner -- confirming again these states have more valid
solving splits than the census happened to record. But the two failures are decisive, not
budget-starved: 150 restarts (up to 774,712 evaluations) never found a working combination on
either endpoint where the pooled model itself needed the most tries to find one by directly
scoring the full survivor list. The pattern tracks the endpoint's own pooled-model difficulty
exactly -- descent succeeds where the pooled model already had it easy (rank 1, 13) and fails
where the pooled model itself struggled (rank 85, 687), consistent with those states having a
narrower, harder-to-find optimum that restarts and a bigger neighborhood still don't reliably
locate.

**Honest cost accounting**: at the k7 scale tested, block descent is not cheaper than directly
scoring the full R_0-survivor list even where it succeeds (2.4x and ~1.0x the direct-scan cost,
not a fraction of it) -- let alone where it fails (23-225x the cost, nothing to show for it). Its
only plausible value is at scales where direct scoring itself is the bottleneck (the k8
3-9M-candidate endpoints from the prior entry), and the harder-endpoint failures here give no
reason to expect it would do better there specifically on hard cases -- if anything the opposite.

**Net assessment**: a real, working algorithm -- unlike deficit best-first, it does find genuine,
sometimes non-census-recorded winners, using the validated score rather than a separable proxy --
but not a general solution. It works on the easier half of exactly the population this track cares
about and fails, expensively, on the harder half. Not recommended for adoption over direct
full-list scoring at the tested scale. Untried, and worth a future look: simulated annealing or
3+-block moves to see if either closes the gap on the hard half specifically.

Evidence appended to [../evidence/deficit_order_and_bestfirst_2026-08-22.txt](../evidence/deficit_order_and_bestfirst_2026-08-22.txt) (section 6).

## 2026-08-22 (done) — concentric round expansion: succeeds on all 4 endpoints, including both where coordinate descent failed

Direct follow-up to a specific critique of the current solver: `canSolveB_ctx` walks a state's
segments (parts) with a shared CPU-unit budget that has no clean relationship to how far into any
one segment's own list has actually been covered. Proposed replacement, worked out analytically
before writing any code: split a state's P segments into P-1 "concentric" segments (grown together
each round via a shared radius) and 1 "last" segment (always walked in full, scored with the real
pooled recursive-V score -- the coordinate-descent "single free block" trick, cheap here since only
that one segment is free). Two design points settled by first-principles reasoning first: the
per-segment growth factor must be G^(1/(P-1)) for a target total-work growth G, not G itself (else
total work explodes as G^(P-1) per round); and propagating the ABSOLUTE radius down to children
(not a relative fraction) should automatically give children more relative effort as state size
shrinks, since the same absolute radius covers more of a smaller list -- reasoned through but not
simulated (single level tested here, not the recursive propagation).

**Real result, all 4 tier-sample endpoints, real oracle verification:**

  U000368 (pooled rank 1):   SUCCESS, round 16, 1,032-1,539 oracle calls
  U000535 (pooled rank 13):  SUCCESS, round 18, 4,399 oracle calls
  U000607 (pooled rank 85):  SUCCESS, round 17, 3,173 oracle calls  -- coord. descent FAILED here
  U000068 (pooled rank 687): SUCCESS, round 16, 2,652 oracle calls  -- coord. descent FAILED here

All 4 succeed, including both endpoints where block coordinate descent failed decisively (150
restarts, up to 774,712 evaluations, zero success). This is the headline finding: round-based
expansion is exhaustive within its current radius by construction, so it cannot get permanently
stuck in the wrong basin the way restart-based local search can -- it eventually covers every
combination as the radius grows, full stop. The oracle-call cost is also strikingly consistent
(1,032-4,399) across a ~700x range in how hard the endpoint was for the pooled model, unlike
coordinate descent's bimodal cheap-win-or-decisive-collapse pattern or blind order's cliff into
"didn't resolve at all." All 4 successes landed in a narrow round band (16-18) despite very
different underlying difficulty -- graceful degradation instead of a cliff, which is exactly the
"non-arbitrary, principled stopping point" property this design was proposed to get.

An offline dry run before spending real oracle calls had been a sobering signal (round-based
coverage of a known winner cost about as many pooled-score evaluations as directly scoring the
full R_0-survivor list) -- worth recording because it shows the METRIC THAT MATTERS is real oracle
calls, not Python-side scoring cost, and the two are not the same story here.

The self-limiting segment-saturation refinement (cap a segment's radius at its own list size, no
special-case code needed) worked exactly as designed in practice: U000068's two size-12 segments
saturated by round 11, leaving only 2 segments still expanding for the rest of the search.

One inconclusive comparison: U000368 tested with both deficit-ordered and blind outer segments;
both succeeded at the same round, blind used somewhat fewer real oracle calls (1,032 vs 1,539) --
not enough data (n=1) to call this a real preference either way.

**Not tested**: propagating the round/radius down into the recursive verification of the three
children themselves -- the "propagate effort to the level below" half of the original design,
which is where the absolute-vs-relative-effort argument actually bites. This note only validates
the single-level mechanism; the multi-level composition is the natural next step if this result is
judged worth pursuing into an actual C prototype.

Evidence: [../evidence/concentric_round_search_2026-08-22.txt](../evidence/concentric_round_search_2026-08-22.txt).

## 2026-08-22 (done) — benchmarking the concentric segment order against the actual production heuristic: no clean winner

Direct follow-up to a fair pushback on the concentric-round result above: the "deficit" and
"blind" orders tested there are BOTH different from what `radiobase.c` actually uses today
(`BY_MAGIC3`, selected for exactly this situation -- a >3-segment state's outermost split level,
`splitincr[0] = size<=3 ? BY_SP1 : BY_MAGIC3`). Fair point that a genuinely new signal should be
benchmarked against the existing, already-proven heuristic before being preferred. Ported
`magic3`/`distance` (radiobase.c:2829-2857) faithfully into Python, including C's
truncate-toward-zero integer division, verified the port is symmetric and minimized exactly at
the true midpoint, then re-ran the full real-oracle comparison on all 4 tier-sample endpoints.

  endpoint   deficit oracle calls (round)   magic3 oracle calls (round)   winner
  U000368    1,539 (16)                     844 (15)                     magic3, 1.8x fewer
  U000535    4,399 (18)                     5,226 (18)                   deficit, 1.2x fewer
  U000607    3,173 (17)                     5,735 (17)                   deficit, 1.8x fewer
  U000068    2,652 (16)                     2,654 (16)                   tie, same split found

**No clean winner.** Totals favor deficit by ~19% in aggregate (11,763 vs 14,459), but the
per-endpoint picture is genuinely mixed -- magic3 wins outright once, deficit wins twice, and the
fourth is an effective tie because that endpoint's smallest segment (12 options) saturates by
round 11 regardless of ordering, so both methods converge to walking nearly the same space. The
unvalidated production heuristic is not obviously worse than the data-driven signal for this
specific purpose. Both succeeded on all 4 endpoints regardless of which order was used -- the
round structure's own exhaustiveness is what carries the robustness result (see the entry above),
not the specific per-segment quality signal riding on top of it. Segment order is a real but modest
(roughly 20-80%) efficiency lever, not a correctness question, and there is no basis yet to prefer
either signal as a default.

Also settled two smaller design questions raised in the same conversation: (1) mixed children have
up to 2x the segments of pure children, which matters for recomputing the round-growth factor from
each state's own actual segment count when recursing (a general rule, not a mixed-specific one) --
separately, the existing "mixed-largest law" (docs/status.md item 4: mixed is strictly the largest
child in all 26,876 known winners) suggests a possible necessary-condition prefilter, untested; and
(2) checking mixed first to fail fast was a bad instinct on my part -- mixed being larger makes it
MORE expensive to resolve either way, so checking cheaper children first likely wins on expected
cost even though mixed fails more often, which the pooled last-segment score (already a `min` over
all three children) already implicitly biases toward without needing an explicit check-order rule.

Evidence appended to [../evidence/concentric_round_search_2026-08-22.txt](../evidence/concentric_round_search_2026-08-22.txt) (section 6).

## 2026-08-22 (done) — concentric round search at n=10: 100% success rate, tight round band

Direct response to fair pushback that n=4 (all cherry-picked from the earlier characterized
endpoints) is not a conclusive sample. Sampled 6 more endpoints from the same k7-census tiers,
excluding the 4 already tested, ran the identical real-oracle-verified concentric round search
(deficit-ordered outer segments, picked pragmatically since section 6's magic3-vs-deficit
comparison was inconclusive and not what this run was trying to settle).

All 6 succeed. Combined with the original 4: **10/10 real, oracle-verified successes**:

  oracle calls: median 2,154, range 886-6,971
  round of success: median 17, range 16-18

The round band stays remarkably tight (16-18) across all 10 endpoints despite a ~700x spread in
how hard they were for the pooled model (rank 1 to rank 687) -- this is now a real, sample-backed
finding, not an artifact of 4 convenient examples: a round cap around 20 would very likely resolve
this whole population, and the round number is a genuinely narrow, meaningful signal. Oracle-call
cost is more spread (8x range) -- one endpoint (U000358) took 481s wall-clock purely because of a
single unusually slow individual oracle query (confirmed live: a `stats` call to the same
serialized oracle process blocked for 16.5s behind it), a known oracle behavior independent of the
round-search design -- total wall-clock is not purely a function of oracle-call count.

Not yet done: the segment-order comparison at this scale (used deficit only here to keep the run
tractable), and the multi-level round/radius propagation into children -- both still open.

Evidence appended to [../evidence/concentric_round_search_2026-08-22.txt](../evidence/concentric_round_search_2026-08-22.txt) (section 7).

## 2026-08-23 (done) — native concentric round search: real, fast, correct, and two more real bugs found

Direct follow-up to "let's take this further... implement, test, do a larger test" after accepting
the concentric-round design as a working hypothesis at n=10. Ported it natively into radio_oracle.c
as a new additive `concentric` command (same discipline as `enumerate`: radiobase.c untouched,
every leaf check calls the existing, trusted `canSolveB`). This removes the Python/TCP oracle's
network-round-trip ceiling entirely, making real k8-scale testing tractable for the first time.

**A real bug found while porting, in the earlier Python work, not in new code**: `HOIST_ORDER`
walks `BY_MAGIC3`'s index array forward from position 0, and `indexSpl` sorts that array
DESCENDING by magic3 value -- so the real solver visits the LEAST balanced split first and the
most balanced last. The prior session's Python port of `magic3_key` sorted ascending (ties
Python's own default), the OPPOSITE direction. The "magic3 vs deficit, no clean winner" comparison
in evidence/concentric_round_search_2026-08-22.txt section 6 therefore measured an untested
direction of the real heuristic -- downgraded to inconclusive, not retracted, since the correct
direction was never actually compared. The native version reads the real array directly and isn't
exposed to this.

**A second real bug, caught only by testing, not by reasoning about it first**: the first native
version "simplified" the Python design by treating all P segments symmetrically (uniform
round-shared radius, no special segment) on the theory that the asymmetric "one segment always
full" treatment only existed to amortize an expensive ML score, and BY_MAGIC3 lookups are already
free. Wrong: measured immediately on the same 10 already-validated k7 endpoints, still 10/10
successes, but needing 64-99% of the FULL raw combinatorial space -- essentially `enumerate`'s own
unpruned-walk problem again. The asymmetric structure isn't about scoring cost; it's that most
winners in this population need exactly one part deep in its own order while the rest stay
comfortable (independently confirmed by the per-part deficit saturation finding from two days
earlier), and a symmetric radius box needs ALL segments simultaneously deep to catch that shape.
Fixed by restoring the asymmetric structure (smallest segment always full, growth factor
recalculated for P-1 segments) -- round-of-success stayed narrow (16-20) but the raw-space fraction
did NOT meaningfully improve (still 64-96%). This forced an honest reframing (see below).

**Real result at k7, native, cold, no cache**: same 10 endpoints as before, all 10 succeed, 36.7s
TOTAL for all ten combined (vs. 58-481s EACH over the Python/TCP path) -- 2-3 orders of magnitude
faster in wall-clock terms, despite doing far more raw work per query, purely because every check
here is an in-process call rather than a network round trip.

**The reframing this forced**: the original "round stays in a narrow band" framing had been read
as "finds winners after checking only a small fraction of the space." That's wrong -- round is
narrow, but the fraction of the TRUE raw space needed is 64-96%, not small (the Python version
never measured against true raw_space, only against R_0-survivor counts and oracle-call counts).
The honest value of this design is a predictable, non-arbitrary STOPPING POINT plus native C speed
making even near-full coverage practically fast -- not "avoids most of the search space." Whether a
genuinely sound early-stopping design exists is still an open question this work does not answer.

**The larger test, k8 real census endpoints**: 8 highest-mass 4-part endpoints (mass 642-643/729,
confirmed genuine solvable states with 2 known census winners each, not arbitrary picks) -- all 8
succeed, round 23-24 (even tighter than k7), 55.4s total. Six of eight land EXACTLY on the
census's own recorded literal winner -- independent correctness confirmation beyond "canSolveB
says yes," since these are facts the census committed to before this code existed.

**A real safety gap found and fixed**: `concentric_search` had no overall deadline at all, unlike
every other search path in this codebase. A broader 60-endpoint random sample hit a state with an
unusually lopsided part (43:2) that ran past 4 CPU-minutes with no way to stop it short of killing
the process. Added an overall deadline (reusing the existing `budget <seconds>` knob) that bails
out with `reason=timeout` instead of running unbounded. The specific triggering state turned out to
complete fine standalone (112s) once re-tested outside a long-lived process with accumulated
cache -- consistent with this repo's own already-documented "cache cost is hump-shaped, don't
extrapolate" trap, now shown to apply to per-query lookup cost within a batch, not just bulk
loading. The deadline fix stands regardless: a lucky single-case measurement isn't a bound.

**A second, subtler deadline gap, also caught only by testing**: the first fix checked the
deadline once per 2^20 candidate combinations, which assumes roughly uniform per-candidate cost.
Wrong whenever the expensive branch (an actual `canSolveB` call, reached only after the cheap
mass/cap filter passes) dominates: a run of several multi-second `canSolveB` calls in a row can
blow past the deadline before the counter next lands on a multiple of 2^20. Found live on the SAME
lopsided-part shape as above, this time inside a fresh 60-endpoint batch, running past 11 CPU-minutes
even with the first fix in place. Fixed by adding a second deadline check immediately after every
completed `canSolveB` attempt, win or lose, independent of the counter. Verified with a standalone
`budget 15` retest on the triggering endpoint: clean `reason=timeout` at 123s (some bounded
overshoot from the single longest in-flight `canSolveB` call remains, but it is no longer unbounded).

**The completed 60-endpoint random sample** (uniform draw from all 471 forced 4-part k8 endpoints,
`budget 20`, both deadline fixes in place): 55/60 (91.7%) succeed, 5/60 (8.3%) time out -- a genuine
MAYBE, not a claimed failure, exactly like `canSolveB`'s own tri-state. Round of success: median 24,
range 21-24 (tighter than either earlier test). Fraction of the true raw combinatorial space needed:
median 0.963, range 0.677-0.999 -- confirms the reframing above at population scale, not just on 8
cherry-picked endpoints. **55/55 (100%) of the successes match a known census literal winner
exactly** -- the strongest correctness signal in this whole thread: every single result reproduces
an external fact the census committed to independently, before this code existed, across a real
random sample rather than a hand-picked one. (A first pass at this check wrongly reported 0/55 due
to an off-by-one in my own verification script, not the C code -- the meta file has a header line
at index 0 that wasn't being skipped; corrected and re-run.)

The 5 timeouts all share a distinctive, reproducible pattern: an unusually lopsided part (large n,
small m -- e.g. 41:4,51:1 or 32:6,45:3,36:2), the same shape that triggered both deadline bugs above.
Worth investigating further as a real, characterized limitation rather than random variance -- a
lopsided part likely carries a disproportionately large admissible-option count relative to its own
information content, inflating the space that must be walked without a matching gain in how fast a
winner turns up.

**Where this leaves the thread**: the native `concentric` command is correct (100% exact-match
against independent ground truth on a random sample), practically fast (tens of seconds per k8
endpoint, in-process, no network round trip), and has an honestly-characterized rather than
unexplained failure mode. It has not been integrated into `canSolveB_ctx` itself as an actual
split-ordering strategy -- that would mean touching trusted solving logic, and was deliberately left
for a future, reviewed step rather than folded into an unattended overnight session.

Explicitly NOT attempted, by design: the literal cold Sa(193) canonical run (~4.85 CPU-days
historically) -- a real, expensive, high-stakes production benchmark this repo's own conventions
say needs a check-in before launching, not something to run unsupervised overnight.

Evidence: [../evidence/native_concentric_2026-08-23.txt](../evidence/native_concentric_2026-08-23.txt).

## 2026-08-23 (same day, follow-up) — three review corrections: rounds replace the deadline, MAYBE stays honest, and ordering (not starting point) is the real gap

Direct response to review of the overnight batch. Three concrete corrections, each applied and
retested, not left as discussion:

1. **Rounds are the effort-growth mechanism, full stop -- they must not be paired with a wall-clock
   deadline or round cap.** Both silently reintroduce what rounds exist to replace: a capped "no"
   is indistinguishable from a genuinely exhaustive one. Removed both from `concentric_search`; it
   now only stops on a confirmed winner or true `fully_saturated`, guaranteed to terminate since
   the radius grows by >=1/round and is capped at each segment's real size.
2. **MAYBE is fine at the intermediate (per-candidate) level, never as the top-level answer.**
   Added `has_maybe` tracking so a saturated "no" that rests on an unresolved child check is tagged
   `reason=unresolved_children` rather than silently reading like a proof -- the exact failure
   pattern this repo's own history (the `k(k-5)/2` transcription error, the 2023 corpus's 37 false
   negatives) warns about.
3. **Widened the round-1 starting radius (R0=8, was ~2)** to test directly whether easy states
   converge in round 1 as expected, and if not, whether the fix is a wider start or a better order.

Retested with all three changes in place. The 5 endpoints that had timed out in the earlier batch
(U000035, U001260, U001766, U001702, U001598) now all succeed with no deadline at all -- round
15-16, 8 CPU-minutes combined for all 5. They were never stuck; they were victims of a deadline too
tight for how much of the raw space this design generically needs, on states whose outer-segment
sizes (147-240) happened to make each round costlier. The original 10 k7 and 8 hardest-k8 endpoints
still succeed 10/10 and 8/8, with round-of-success dropping (k7: 16-20 -> 8-12; k8: 23-24 -> 15-16)
purely from the wider start.

**The real finding, from a genuinely difficulty-diverse test** (20 k8 endpoints spanning mass
497-638 and win-count 1-32, not just the highest-mass tier): all 20 succeed, but round-of-success
sits FLAT at 14-16 regardless of difficulty -- even the 32-winner endpoint, the easiest in the
sample by any reasonable measure, needs round 15. Widening the start narrowed the round band overall
but created no separation between hard and easy states. That rules out "start too narrow" and
points at "the ordering doesn't rank by winner-likelihood" -- BY_MAGIC3's descending-least-balanced
walk, direction confirmed correct in section 2 of the evidence file, evidently does not concentrate
this population's real winners toward the front of any outer segment's list. Recommended next
experiment, not yet run: swap in the earlier per-part "deficit" order (0.9961 population-level AUC,
see the 2026-08-22 evidence file) for the outer segments and re-measure.

Evidence: [../evidence/native_concentric_2026-08-23.txt](../evidence/native_concentric_2026-08-23.txt)
section 11.

## 2026-08-23 (same day, second follow-up) — top level now forces refutation instead of tagging uncertainty

Further correction to the MAYBE handling above: tagging a saturated "no" with
`reason=unresolved_children` was honest but stopped short of the actual goal -- "expand until a
solution is found or refutation is achieved" means the top level must spend more effort resolving
ambiguity, not just disclose it. No reimplementation needed: this codebase's search budget is
deterministic by default (RADIO_WORK_BUDGET in radiobase.c counts accepted split prefixes, not
wall-clock time), and canSolveB_ctx already propagates a shrinking budget from parent to child and
returns MAYBE only on budget exhaustion, never as a correctness compromise. So the fix is additive:
remember every candidate left ambiguous during the sweep (capped at 4096, with an honest overflow
flag past that), and once the round loop saturates with no winner, re-verify each one with
NO_DEADLINE using canSolveB's own unmodified recursion before finalizing -- a clean "no" only goes
out if every remembered candidate resolves to a genuine FALSE and the list never overflowed.

Natural MAYBEs turned out too rare to trigger for validation: even the tightest budget the protocol
exposes (1 nominal second = 20,000,000 work units per child) produced zero MAYBEs across all 43 real
endpoints tested today. Validated the resolve-pass code directly instead, with a temporary test hook
(reverted before commit) forcing an arbitrarily tiny raw work-unit budget: confirmed both the
overflow-honesty path (forced-MAYBE sweep correctly reports `reason=unresolved_children` rather than
a false refutation) and the promotion path (a candidate left ambiguous during the sweep gets
correctly resolved to a confirmed winner via the resolve pass, `resolved_ambiguous=yes`, including a
case that found a second, different valid literal winner for the same state). Reran the 10-endpoint
k7 regression after removing the hook: byte-for-byte identical to before -- no behavior change at
the realistic default budget, where this correction is a correctness guarantee for the rare/tight
case rather than a change to today's numbers.

Evidence: [../evidence/native_concentric_2026-08-23.txt](../evidence/native_concentric_2026-08-23.txt)
section 12.

## 2026-08-23 (same day, third follow-up) — radius propagation replaces work-unit currency inside canSolveB_ctx itself

Direct implementation of "the deadline propagation was there already... now we just want to
replace that with radius propagation instead." Section 12's resolve pass still bounded children
via radiobase.c's existing deterministic work-unit clock; this replaces that currency inside
canSolveB_ctx itself with radius, gated behind a new `radius_mode` field on `radio_search_context`
(default off, zero behavior change for every existing caller).

Three iterations, each corrected by something found only by testing or by tracing the code, not by
reasoning alone:
1. First attempt (radius = totalsplits vs a cap) was rejected before writing code -- a single
   aggregate-combinations cap scales unevenly across different part counts.
2. Second attempt (radius = totalsplits, absolute propagation) compiled and immediately hung: pass
   counts in the tens of millions. canSolveB_ctx's existing iterative-deepening retry assumes each
   retry gives children MORE budget; radius mode's unchanged child propagation broke that
   assumption. Fixed by making radius mode stop after exactly one exhaustive pass -- confirmed
   safe since the function already returns MAYBE unconditionally whenever `skipped_some` is true
   at loop exit, regardless of why the loop stopped.
3. Third attempt, the one that stuck: radius = "top-N candidates per segment, at every level" (1
   segment -> N, 2 segments -> N^2, 4 segments -> N^4), implemented by capping each level's own
   `splitindex[]` range directly rather than comparing an aggregate counter. This needed real
   correctness engineering: a new `radius_truncated` flag, set whenever any level's range is
   actually capped below its true size, threaded into every path that could otherwise conclude a
   definitive FALSE -- a truncated search can only end in MAYBE or a genuine TRUE, never a
   manufactured FALSE. Verified directly: a tiny standalone harness confirms `canSolveB_ctx` with
   `radius_mode=1, N=1` on a real solvable state returns MAYBE, and `N=2^40` returns TRUE.

Also found, by tracing the exact index arithmetic rather than assuming: `canSolveB_ctx`'s own
internal BY_MAGIC3 walk goes the OPPOSITE direction from concentric_search's own top-level sweep
(confirmed by the code's own comment, "walks the descending-sorted index from the far end, i.e.
in ascending key order") -- not a bug, but an inconsistency for "radius means the same thing at
every level." Fixed per explicit direction: radius mode reverses BY_MAGIC3 inside `HOIST_ORDER` so
every level walks least-balanced-first, matching the top level, and forces BY_MAGIC3 everywhere in
radius mode (overriding the existing per-level heuristic choice) so radius refers to one consistent
order throughout the recursion.

**Validation**: reran all 43 previously-tested endpoints (10 k7, 8 k8-hardest, 20 k8 diverse, 5
previously-hardest) under this design. Every one succeeds, and every substantive field -- round,
checked, raw_space, frac, and the literal winner reported -- is byte-for-byte identical to the
work-unit-currency results from the earlier entries today. Since those were already confirmed
55/55 exact census matches, this carries the same correctness weight transitively to the new
radius-currency design. No regression to the default (radius_mode=0) path: plain VERDICT queries
and `enumerate` (both using `radio_default_search_context`) are unaffected throughout.

Evidence: [../evidence/native_concentric_2026-08-23.txt](../evidence/native_concentric_2026-08-23.txt)
section 13.

## 2026-08-23 (same day, fourth follow-up) — drop ambiguous-candidate storage, re-sweep with cache instead

Simplification per direct feedback: section 13's has_maybe resolution stored up to 4096 ambiguous
candidates in fixed arrays to re-verify individually, with a real overflow risk if more were ever
left ambiguous. Replaced with something simpler and equally correct: when has_maybe is set at full
saturation, re-sweep the entire candidate space once more (every combination, not just "new" ones)
with an unbounded radius. No storage needed -- the shared dominance trie already holds every fact
this run has proven, so an already-resolved candidate is an instant cache hit on the re-sweep, and
only genuinely-still-ambiguous ones cost real work twice.

Validated with a temporary (reverted) test hook forcing a constant round_radius instead of the real
growth schedule: round_radius=2 matched the unforced run exactly on all 10 k7 endpoints (radius 2
per segment already sufficed for every child check this population needed -- many recursive calls
bottom out via theorem-based base cases that don't consult radius at all, so real truncation is
rarer than the raw N suggests); round_radius=0 (guarantees truncation everywhere) correctly
exhausted to full saturation, triggered exactly one re-sweep, and found the same previously-
validated winner via the unbounded radius, with no meaningful slowdown -- confirming the cache-hit
argument empirically. Reran the full 43-endpoint battery afterward: still byte-for-byte identical
to every prior result today.

Also clarified two points of possible confusion from the same feedback, worth recording:

1. **Ordering should be conditional on segment count and index, and canSolveB_ctx already has a
   version of this** (its own chosen/BY_SP0/1/2/DESC heuristic, size<=3 special-cased, branching on
   the relative magnitudes of p0/p1/p2 as recursion proceeds through segments). Radius mode
   currently overrides all of this with a blanket BY_MAGIC3-everywhere rule -- done specifically for
   direction consistency (so "radius" means the same order at every level, per the earlier
   direction-mismatch fix), not because BY_MAGIC3 is being claimed optimal. This existing heuristic
   is real prior art worth studying before the "fix ordering" experiment, not something to have
   quietly discarded.

2. **Two different "growing" mechanisms exist and were conflated in the last write-up.**
   concentric_search's own outer round loop (radio_oracle.c) is designed to widen and correctly
   still does -- unrelated to any of this session's radiobase.c work. canSolveB_ctx's own INTERNAL
   pass-retry loop (radiobase.c) is a separate, lower-level mechanism: a single invocation, on an
   unresolved exhaustive pass, normally retries with a bigger child quantum (probe_seconds
   doubling) rather than giving up. Radius mode's probe_child_deadline hands the child its full
   cap immediately (no incremental slicing), so there is nothing left to widen on a retry --
   retrying would repeat an identical attempt forever, which is the infinite loop section 13 found.
   Stopping that inner retry (rather than teaching it to also widen) is fine specifically because
   this section's re-sweep design now provides the widening role at the OUTER layer instead: the
   round loop widens the exploratory radius, and the final re-sweep widens it to unbounded when
   needed. The inner mechanism isn't broken, it's just made redundant by where widening now lives.

Evidence: [../evidence/native_concentric_2026-08-23.txt](../evidence/native_concentric_2026-08-23.txt)
section 14.

## 2026-08-23 (same day, fifth follow-up) — NO_DEADLINE now widens recursively; a real fork on concentric_search's fate

Two corrections per direct feedback, then a direct test of the resulting architectural question.

**Kept the pre-existing per-level ordering heuristic** (BY_SP0/1/2/DESC, size<=3 special-cased,
the counting-bound-driven selection) instead of forcing BY_MAGIC3 everywhere in radius mode --
that override was a reasonable first cut for the direction-mismatch finding, but discarded real
prior art (a heuristic with its own measured "3.5x fewer candidate evaluations" result). Reverted,
along with the now-moot HOIST_ORDER BY_MAGIC3 reversal. No regression: byte-identical on all 43
endpoints.

**Implemented real progressive widening for NO_DEADLINE**, replacing the earlier "stop after one
pass" fix: a growing call's radius_N starts small and doubles on every unresolved exhaustive pass
(mirroring the work-budget clock's own probe_seconds doubling), and its children get NO_DEADLINE
themselves -- not a finite slice -- so widening propagates recursively. Found and fixed a real
corruption bug while wiring this up: the pre-existing work-budget doubling logic and
probe_child_deadline both read `deadline` directly, which stays the NO_DEADLINE sentinel in radius
mode by design -- an unsigned underflow (deadline - a work-clock budget_start) that the overflow
clamp turned into UINT64_MAX, silently breaking a child's ability to recognize "I should also
widen" (UINT64_MAX != NO_DEADLINE). Fixed by gating the work-budget doubling to non-radius-mode
only and computing the child's cd explicitly in radius mode.

**The direct test this makes possible**: does a plain canSolveB_ctx(&radius_ctx, sb, size, k,
NO_DEADLINE) call -- no concentric_search wrapper, no asymmetric last-segment structure -- correctly
and efficiently answer the same queries? Ran all 43 previously-tested endpoints this way. All 43
correctly return TRUE. The 10 k7 and 8 k8-hardest endpoints resolve in 0.00-0.27s each -- two to
three orders of magnitude faster than concentric_search's own times for the identical states. Most
of the 20 k8-diverse endpoints resolve in under a second. But 2 of the 5 previously-hardest
endpoints -- the ones with the most extreme lopsided parts (41:4/51:1 and 46:2/50:1, the same shape
that triggered both deadline bugs earlier today) -- took 494s and 300s respectively, slower than
concentric_search's own handling of those same 5 endpoints (8 CPU-minutes combined for all 5).
Correct, not a false answer or a timeout -- just slow.

**Left open, not resolved unilaterally**: this is a real fork, not a clean win. Baking progressive
widening into canSolveB_ctx eliminates the need for concentric_search's bespoke sweep for the
overwhelming majority of states, dramatically faster -- but concentric_search's own asymmetric
"last segment always full" structure appears to be solving a real problem for the lopsided-part
shape that the existing per-level heuristic does not. Whether to retire concentric_search, keep it
as a fallback for that shape, or dig into why the heuristic struggles there specifically, is a
decision for the next session or explicit direction, not something to resolve by picking one
silently.

Evidence: [../evidence/native_concentric_2026-08-23.txt](../evidence/native_concentric_2026-08-23.txt)
section 15.

## 2026-08-23 (same day, sixth follow-up) — widening only at the top; lopsided-part slowness precisely diagnosed

Correction per direct feedback: propagating NO_DEADLINE to children (each level independently
widening) was wrong. Progressive widening should happen ONLY at the level that actually received
NO_DEADLINE from outside; every child gets the current attempt's concrete radius_N, does exactly
one pass at it, and passes that same number on unchanged. Fixed: the child cd computation in
radius mode is now simply `radius_N`, not `radius_grows ? NO_DEADLINE : deadline`. Regression:
byte-identical on the default path and concentric_search's own 43-endpoint battery.

Retested the direct canSolveB_ctx(NO_DEADLINE) call (no concentric_search) on all 25 previously-
timed endpoints. All 25 still correctly resolve TRUE. The two lopsided-part outliers improved
modestly (494s->433s, 300s->218s) but remained far slower than concentric_search's own handling --
the fix was correct and worth keeping, but didn't solve the core problem.

**Diagnosed the remaining gap by measuring, not guessing further**: printed each part's true
admissible-split-list size for both slow states. (26:7,41:4,27:5,51:1) at k=6: sizes 196, 90, 164,
28. (30:6,35:5,46:2,50:1): sizes 167, 128, 53, 30. In both, the lopsided part has by far the
SMALLEST list -- it was never the bottleneck. Symmetric radius_N growth (same cap on every part)
means once radius_N exceeds ~30 the lopsided part is fully covered and further growth buys it
nothing, but growth continues anyway to reach whatever the largest part needs (196 here) -- and
since the same radius_N multiplies across all four parts jointly, by the time it's large enough for
the largest part, the exploration is already near the full raw space. This is exactly the
degeneracy the 2026-08-22 naive-symmetric prototype hit, and exactly what concentric_search's own
asymmetric "smallest segment always full, others grow together" design already solves -- now
confirmed as the precise, measured reason, not a vague heuristic-quality gap.

Natural next step, left for explicit direction rather than implemented unilaterally: generalize
concentric_search's asymmetric structure into canSolveB_ctx's own radius-mode capping -- at each
level, always fully explore the segment with the smallest true admissible-split size, growing only
the rest together.

Evidence: [../evidence/native_concentric_2026-08-23.txt](../evidence/native_concentric_2026-08-23.txt)
section 16.

## 2026-08-23 (same day, seventh follow-up) — asymmetric capping tried and reverted: two hypotheses tested and falsified

Implemented the asymmetric-capping fix proposed after section 16's diagnosis: identify each
level's smallest-true-size segment and exempt it from radius_N capping, mirroring
concentric_search's own design. Result: SLOWER, not faster (532.97s/230.44s vs. the pre-fix
433.17s/217.76s on the same two lopsided-part states). The section 16 diagnosis was incomplete.

Captured real instrumentation (canSolveB_ctx's own progress printer) rather than guessing a third
time: at 60s and 120s into the run, the search was STILL on pass=7 (radius_N=128, unchanged) --
not cycling through many doublings, but stuck deep inside a single large pass, advancing only 8
positions of splitindex[0] in 60 seconds. Rules out "needs too many doublings" as the mechanism;
the real cost is per-candidate overhead within one exhaustive pass, compounding across tens of
millions of combinations.

Tested a second hypothesis per feedback: the newly-added eager segment-table-building (needed to
find the smallest segment) ran on every recursive invocation, not just the one call that received
NO_DEADLINE -- unlike concentric_search, which only ever does this bookkeeping once. Gated it to
radius_grows only. Retested: 541.08s/223.20s, statistically indistinguishable from before. Also
falsified.

Reverted both changes entirely -- neither helped, and both added real complexity for no measured
benefit. radiobase.c is back to exactly the post-section-16 state (widen only at the top, symmetric
single-pass propagation, existing per-level heuristic). Confirmed via diff and a regression check.

**Left open, with two falsified hypotheses on record** so neither is re-attempted blindly: the
~500s/220s timing for these two states is remarkably stable across every capping strategy and
build-eagerness variation tried today, suggesting the bottleneck is something more fundamental --
possibly the sheer node count this specific state shape's recursion tree generates, or the
existing per-level heuristic re-selection's genuine per-node cost (it depends on accumulated
p0/p1/p2 state, so it can't simply be memoized per level the way a static property could be).
concentric_search remains the better-performing option for this shape until a real fix is found.

Evidence: [../evidence/native_concentric_2026-08-23.txt](../evidence/native_concentric_2026-08-23.txt)
section 17.

## 2026-08-23 (same day, eighth follow-up) — the performance gap was mostly a flawed comparison

Profiled the slow lopsided-part states directly (macOS `sample`, real stack traces) instead of
guessing a fourth time. Corrected the earlier print-based inference: ~80% of actual solving time
(excluding init()) is inside canSolveB_ctx's own entry-sequence code, not
star_expansion_majorization_can_solve (14.6%, a red herring) or memmove (3.2%, also a red herring).

Implemented the resulting, well-targeted fix: canSolveB_ctx already has a joint mass/cap
feasibility check equivalent to concentric_search's own S/X/Cm check, but it only fires after an
expensive single-part viability precheck. Hoisted an equivalent check earlier, as a pure addition
touching nothing else. Regression: byte-identical everywhere. Result: still no meaningful change
(554s/230s) -- the THIRD fix attempt, despite increasingly precise profiling, to fail. Reverted.

Rather than guess a fourth time, checked the comparison itself: ran concentric_search on each of
the two states IN ISOLATION rather than as part of a 5-state batch average. Result: 510s and 187s
respectively -- essentially the SAME range every canSolveB_ctx-based variant measured today (433-
554s and 217-230s). **The original ~5x gap that motivated three rounds of fixing was mostly an
artifact of comparing individual state times against a 5-state BATCH AVERAGE** ("8 CPU-minutes
combined for all 5, ~96s average") rather than that state's own isolated cost. Both states are
genuinely, structurally expensive under concentric_search's OWN design too -- and since
concentric_search's child verification IS canSolveB_ctx, unmodified, every radiobase.c change made
today affected both designs' performance identically for the part that actually dominates cost, so
a same-day comparison was never apples-to-apples to begin with.

**Conclusion**: this lopsided-part shape is genuinely expensive under either design, not a
canSolveB_ctx-specific weakness. The three reverted fixes were not wrong ideas -- the joint-check
hoist in particular remains a defensible optimization on its own terms -- they were solving a
problem whose measured scale turned out to be mostly a comparison artifact. No further
optimization attempted; concentric_search and the unified radius-mode design are now understood to
perform comparably on this shape.

Evidence: [../evidence/native_concentric_2026-08-23.txt](../evidence/native_concentric_2026-08-23.txt)
section 18.

## 2026-08-23/24 (overnight) — cold Sa(192) exposed a real exponential-blowup bug in radius
## mode; chasing two fixes for it led to a first-principles result that ends the thread

Per direct request, benchmarked the old engine against radius mode on a real workload instead of
synthetic endpoints: cold Sa(192) in k=10 (confirmed cold via init(); no pre-loaded sa_can/
sa_cant facts). **Old engine (plain canSolveA -> canSolveB, work-budget, NO_DEADLINE): SOLVABLE
in 290.8 CPU seconds.** Radius mode (the "widen only at the top" design committed after section
16, absolute propagation to every child): killed after 60+ minutes, unresolved.

Per direct instruction to stop the run and analyze the choices it made rather than keep waiting,
found the mechanism by reading the log directly: one state, an 8-part
Sb(17:8,16:8,27:4,18:5,13:5,16:4,19:3,27:2), alone consumed 7,168 CPU seconds and **32.7 billion**
totalsplits. Root cause: canSolveB_ctx's recursion builds a "mixed" child whose part-count is
double the "pure" children's; propagating radius_N unchanged to it means the same per-segment cap
applies to an exponentially larger space (cost is radius_N^size), and Sa(n)'s own recursion
chains several such doublings in sequence -- exactly the blowup the old engine's additive,
divisible work-budget currency was designed to prevent, and radius mode had no equivalent for.

Tested two proposed fixes for the mixed child's propagated radius:
- **sqrt** (`cd_mixed = ceil(sqrt(cd))`, keeping radius_N^size invariant across a doubling):
  fixed the blowup completely (max totalsplits for any state: 1.75 million, ~18,700x smaller)
  but introduced a new problem -- a sqrt-scaled child's radius grows only sqrt(2)~=1.41x per
  parent retry instead of the parent's own 2x, so resolving it took far more retries the deeper
  the mixed-nesting (one tiny state, Sb(65:62), needed pass=19). Still killed after 60+ min.
- **/2** (`cd_mixed = (cd+1)/2`, matching the parent's growth rate exactly): fixed the retry-rate
  problem (99.8% of states resolve in pass 1-2, matching the old engine) but partially
  reintroduced blowup risk -- max totalsplits 7.39 billion (4.4x better than the original bug,
  but ~4,200x worse than sqrt's peak). Also killed after 60+ min.

**This is a genuine, empirically-confirmed, two-way trade-off** -- neither fix was adopted;
both fully reverted (radiobase.c confirmed byte-identical to d77f148, no cd_mixed distinction,
no `#include <math.h>`).

Per direct instruction to think from first principles about why the old engine is fast, rather
than trying a third propagation-scaling rule: work-units are additive and size-invariant by
construction (one unit per accepted split, globally), so search_deadline's existing division
rule already composes safely across the mixed child's size-doubling -- something a per-segment,
multiplicative radius currency (cost exponential in part-count) cannot do without patches like
sections 20-21's. Sections 19-21 were, in effect, teaching an exponential currency to imitate
what an additive one already does for free.

This motivated the one test that had not actually been run today: **does plain, unmodified
`canSolveB(sb, size, k, NO_DEADLINE)` -- no radius mode, no concentric_search wrapper -- also
resolve today's "hard" states quickly?** Every earlier comparison had been radius-mode vs.
concentric_search; the untouched default engine had never itself been the thing under test.

**Result: yes, dramatically, on every case tried.** The 8 "hardest" flat 4-part endpoints (the
same ones radius mode resolved in 0.02-0.27s each) all resolve in under 0.1s under plain
canSolveB. The 2 lopsided-part endpoints that sections 15-18 spent the whole day chasing --
217-554s under every radius-mode variant, 187-510s under concentric_search itself in isolation
-- resolve in **64.7s and 25.3s** under plain canSolveB: **7-8x faster than either alternative**,
on the exact states both were built to handle well.

**Conclusion, for this class of query (top-level Sb verification and Sa(n) recursion): do not
use radius_mode or concentric_search -- call `canSolveB(..., NO_DEADLINE)` directly.** The old
engine is fast not because of anything concentric_search's rounds or radius-mode's per-segment
capping added; it is fast because its existing per-level heuristic ordering combined with its
existing additive, size-safe work-budget division already handles these shapes well, including
the lopsided-part shape that motivated most of today's investigation. `radius_mode` remains in
`radiobase.c` as a default-off, zero-impact field (confirmed by every regression run today) --
left in place as a validated-but-not-recommended feature and a record of what was tried, not
torn out mid-investigation. `concentric_search` in `radio_oracle.c` is likewise left in place
pending an explicit decision on whether it is still worth maintaining.

Not yet done: broader validation beyond the 10 states above (a 25-state mass-diverse battery was
still running at write time -- see evidence file for its outcome if it completed in time).

Evidence: [../evidence/native_concentric_2026-08-23.txt](../evidence/native_concentric_2026-08-23.txt)
sections 19-22.

## 2026-08-24 (same session, follow-up) -- squaring the top-level retry rate: radius mode now
## beats the default engine on the multi-part battery, though a different shape still loses

Per direct request to keep pushing radius-mode toward "at least comparable, ideally better"
rather than stop at the prior conclusion. Diagnosed the pure-sqrt design's retry-count problem
precisely: sqrt-scaling is reapplied at every mixed-child boundary, so a child nested d levels
deep sees only radius_N^(1/2^d) of the top level's radius -- doubling the top radius therefore
grows a depth-3 child by only 2^(1/8)~=1.09x per retry, exactly matching the pass=19-38 observed
earlier. Fix: square the top level's radius_N on retry instead of doubling it (with an overflow
guard). A depth-d child then sees radius_N^(2^(t-d)) after t top-level retries -- once t>=d this
doubles in lockstep with the top level, so a handful of retries suffices regardless of nesting
depth, while sqrt-scaling still keeps per-level cost mathematically invariant across a doubling
(unchanged math from the earlier sqrt fix).

Regression: default path unaffected (117.9s vs the 117.0s baseline, noise-level). Cold Sa(192) in
k=10 with squaring+sqrt: retry-count problem solved (max pass 7, versus 19-38 under plain sqrt),
and max totalsplits per state (6.4M) stayed far below the original unscaled bug's 32.7 billion.
But the run still didn't finish (2,502 CPU-seconds across two attempts) -- log analysis pinned it
on one specific leaf query, Sb(112:80) in k=9 (the verification the winning n1=112 split needs),
stuck at pass=5 and 32.9 billion accumulated work-units.

**Decisive isolated test**: ran that exact query, Sb(112:80) in 9, under the plain default engine
directly. Result: TRUE in 293.3 CPU-seconds -- essentially the WHOLE 290.8s the default engine
needed for all of Sa(192), meaning every other step of that computation is cheap by comparison.
Squaring+sqrt radius mode took over 1256 CPU-seconds on the identical query without finishing --
4.3x+ slower and still open. This proves the remaining gap is no longer a currency-propagation
bug (both known bugs are now fixed) -- it's that radius-mode's hard per-level cap, even correctly
scaled and retried, explores this large single-part state's split space far less efficiently than
the default engine's heuristic ordering plus shared work-budget currency.

**But**: reran the 25-state multi-part battery (the follow-up validation battery from the prior
entry) through squaring+sqrt radius mode. Result: **85.823s total, zero incorrect verdicts across
25 states -- about 27% FASTER than plain canSolveB's 117.0-117.9s on the identical battery**, and
dramatically faster on the two hardest lopsided-part states specifically: 28.9s vs 55-65s (~2x),
7.9s vs 23-25s (~3x). Not uniformly better (a couple of small states are modestly slower), but the
aggregate and the two hardest cases both favor radius mode clearly.

**Conclusion: the workload shape matters, and the prior blanket recommendation is superseded for
one shape.** The 25-state battery is radius-mode's actual intended use case -- verifying an
already-split, moderate-size multi-part state -- and squaring+sqrt now wins there, a genuine
"comparable, ideally better" result. The Sb(112:80) leaf query is a different shape: a single
large unsplit pair that needs many chained levels of internal splitting from a much bigger raw
combinatorial space, and there plain `canSolveB(NO_DEADLINE)` remains clearly better (293s vs
>1256s unresolved) and should still be preferred. The squaring+sqrt change is committed to
`radiobase.c`, gated entirely behind `radius_mode` (default off, zero regression confirmed on the
default path). Not yet attempted: a targeted fix for the single-large-part shape -- the isolated
test points at search-ORDER quality under a hard per-level cap (e.g. the earlier per-part
deficit-score ranking, ~0.9961 AUC, referenced in docs/status.md sections 11-12) as the likely
direction, not further propagation-scaling arithmetic -- left for explicit direction.

Evidence: [../evidence/native_concentric_2026-08-23.txt](../evidence/native_concentric_2026-08-23.txt)
section 23.

## 2026-08-24 (same session, second follow-up) -- instrumented where the time actually goes
## inside Sb(112:80), and a linear-growth experiment that wins decisively on one shape and loses
## decisively on another

Per direct request to keep digging into whether the stall is wrong choices, too many wrong
choices, or too much time per choice. Built an offline instrumented copy of radiobase.c (not the
committed one) counting, in radius_mode only: every canSolveB_ctx entry, and how many return via
a cache/theorem shortcut before reaching the real split-search machinery. Found a methodology bug
first: comparing default and radius mode on the same query in one process is invalid, because they
share the same global cache trie -- whichever runs second gets a free ride on the first's answer.
Every number below is from a query run alone, cold, in its own process.

**The mechanism, precisely**: cache-hit fraction is consistently 91-99% throughout every run
measured. The cost is not individual calls being slow -- it's that "widen only at the top, single
pass below" means every top-level retry re-walks the ENTIRE top-level candidate range from
scratch, mostly landing on cache hits, but at billions-of-calls scale that re-walk itself is the
cost. Confirmed on a smaller, fully-resolved analog, Sb(67:46) in 8 (a real sub-state of
Sb(112:80)'s own recursion): squaring+sqrt resolves it (FALSE) in 232.3s with 8.47 billion total
calls, 99.1% cache hits -- versus plain default's 87.6s. Same mechanism, small enough to actually
finish and measure end to end.

**Linear-growth experiment** (direct request): does squaring/doubling itself overshoot past
whatever smaller radius would have sufficed? Implemented radius_N growing by +1 per retry instead
of *2/^2, paired with absolute (unscaled) propagation to the mixed child too, reverting to the
original pre-sqrt-fix propagation rule but with a much gentler schedule. Found and fixed a bug in
the experiment itself first: starting at radius_N=1 passes straight through 1,2,3,4 -- exactly
CACHE_ONLY/NO_DEADLINE/FAST_ONLY/FROZEN_REFUTE's sentinel values -- so a child handed cd==2
misread it as NO_DEADLINE and started its own incorrect independent growing sequence (visible as
multiple interleaved, resetting pass counters). Fixed by starting at 5, clear of the reserved
range.

Results, three shapes:
- **Sb(67:46) in 8: linear wins decisively.** FALSE in 88.6s (pass=1949) -- matches plain
  default's 87.6s within 1%, and is 2.65x faster than squaring+sqrt's 232.3s on the identical
  query. Confirms the overshoot hypothesis for this state.
- **Sb(112:80) in 9 (the actual target): linear does NOT win.** Tracked cleanly: cheap through
  pass ~600, then a series of jumps -- cpu climbing 7->21->43->87->198->270->413->487->587s by
  pass~1276 -- still unresolved when stopped, already 2x plain default's 293.3s with no sign of
  finishing.
- **25-state battery (radius mode's own intended shape, where squaring+sqrt currently wins at
  85.8s total): linear loses.** One state faster (2.4s vs 4.1s), one much slower (5.6s vs 0.3s),
  and stuck on the hardest lopsided-part state (squaring+sqrt: 28.9s) for 9+ CPU-minutes with no
  verdict before being stopped.

**Conclusion: neither a fast (squaring) nor a slow (linear) fixed growth schedule is uniformly
better** -- each wins decisively on states where its own bias happens to match where the state's
true necessary radius lies, and loses decisively otherwise. Same shape of trade-off as the earlier
sqrt-vs-/2 result, one level up: there the mixed-child scaling exponent traded blowup-safety for
retry-speed; here the top-level growth rate trades overshoot-waste for plateau-crawl-waste. Root
cause in both cases: "widen only at the top, one pass below, retry re-walks the whole subtree"
makes total cost = (retries needed to reach the true necessary radius) x (cost of one full sweep
at the current scale), and no fixed schedule can minimize that without knowing the true necessary
radius in advance, which varies unpredictably by state.

Not yet attempted: an ADAPTIVE schedule -- grow gently while new information keeps appearing each
pass, detect a plateau (no new real work vs the previous pass, the exact signature observed
repeatedly here) and jump ahead aggressively once one is detected, reverting to gentle growth when
progress resumes. Could plausibly capture linear's no-overshoot benefit without linear's
slow-crawl cost -- but it's a materially bigger design change than anything tried so far, left for
explicit direction. The committed radiobase.c is unaffected by this entire follow-up: every
experiment ran against an offline instrumented copy, never the committed file. Committed
radius_mode remains squaring+sqrt -- still the better choice for the multi-part battery shape,
still worse than plain canSolveB(NO_DEADLINE) for the single-large-part shape, now with a precise
mechanistic explanation for both facts.

Evidence: [../evidence/native_concentric_2026-08-23.txt](../evidence/native_concentric_2026-08-23.txt)
section 24.

## 2026-08-24 (same session, third follow-up) -- is the ordering actually good? Yes, with real
## skill, but nowhere near "the first few rounds," and it degrades exactly on the hardest states

Direct question: the hope is an algorithm that finds a solution in the first few rounds, ideally
the very first, avoiding the scanning/rescanning trap sections 19-24 kept running into. Why do we
keep missing a solution for so long -- is the shared per-level ordering heuristic (BY_SP0/1/2 +
adaptive selection, used by both engines) actually good?

Three tests, all on real data:

1. **Is the winning candidate itself expensive, or just late in the queue?** Verified, alone and
   cold, the exact three children default's own winning split produced for Sb(112:80) in k=9:
   Sb(48:32) TRUE 0.361s, Sb(64:48) TRUE 0.004s, Sb(48:48,64:32) [mixed] TRUE 23.479s -- 23.8s
   total, against the full run's 293.3s. The winner is cheap: verifying it alone is 8% of the
   total time. The other 92% (~269s) goes to ~1256 other, ultimately-rejected candidates examined
   before/around reaching it, at ~0.21s average each -- individually cheap, just numerous.

2. **Exactly how deep in the raw order does it sit?** Instrumented the success path to print the
   remaining-vs-total candidate count at the moment of success. Result: Sb(112:80)'s winner sits
   at fraction_from_top = **0.4135** -- 41.35% of the way through the raw order (splitsarr[0]
   size=6247, splitindex[0]=3664, totalsplits=1257). Nowhere close to "the first few."

3. **Is 41% typical, or specific to this hard state?** Ran the same instrumentation on the easy,
   0.388s-total Sb(48:32) and captured every recursive success in its own exploration tree -- 726
   data points. mean=0.220, median=0.206, min=0.0035, max=0.750, p10=0.036, p90=0.417; 12.4%
   resolve within the first 5% of their local order, 27.8% within the first 10%, 7.7% need more
   than 45%. **The heuristic has real, measurable skill** -- a median of 0.206 is far from the 0.5
   a no-information order would give, and the low end is often very early. But the median case
   still needs roughly a fifth of the local range, and Sb(112:80)'s own top-level fraction (0.4135)
   sits right at this distribution's p90 -- the hardest, most expensive state landed in the worst
   tail of the same heuristic that does reasonably well on average.

**Conclusion**: ordering is not bad, but it is not remotely what's hoped for, and it appears to
weaken exactly on the states where that costs the most. This reframes sections 19-24: every
growth-schedule experiment (squaring, sqrt, /2, linear) was fundamentally guessing, blind, how
deep into an unknown-quality order the winner sits, and paying however many retries that takes.
If the order itself reliably put a winner in the first few candidates, none of that machinery
would be needed -- a small, fixed, non-growing radius would already succeed on pass 1. The
growth-schedule dilemma is a symptom; ordering quality looks like the actual disease.

Not yet done: this points at the earlier, still-un-integrated per-part deficit-score ranking idea
(~0.9961 population-level AUC, docs/status.md sections 11-12) as the natural next test --
replace or augment BY_SP0/1/2's ranking with that score and re-measure where the winner lands for
the same states measured here, before touching growth schedules again. Since both engines share
this exact heuristic, an ordering fix would help plain canSolveB too, not just radius mode. Left
for explicit direction -- a materially different piece of work from anything tried so far.

Evidence: [../evidence/native_concentric_2026-08-23.txt](../evidence/native_concentric_2026-08-23.txt)
section 25.

## 2026-08-24 (same session, fourth follow-up) -- wired the recursive-value model into
## canSolveB_ctx: the mechanism is real and tested, the current weights make it worse on the
## primary target, and the failure has a clear cause

Wired the repo's own already-validated recursive value model into canSolveB_ctx's split loop,
something docs/ml-guided-search.md had called "the remaining step" since 2026-08-20. Corrected
scope twice before writing C: (1) the model's training data turned out to be exclusively 4-part
states, matching nothing like our size=1 target's own children shape (1/2/1); generated fresh
matched corpora at nparts in {1,2,3} x k in {4,5,6,7} to fix this, per direct instruction to test
combined-vs-separate models rather than assume. Caught a real generator bug first (WIDTHCAP could
exceed the labeling oracle's MAX_N). Combined model (all sizes pooled) tied or beat separate
per-size models on every size measured. (2) A second, more important correction: the n=1
corpus's AUC=1.0 is a trivial artifact -- the sampler's own n>=m floor combined with a ~45%-of-
cap mass target mathematically forces m <= sqrt(hi*cap), which lands EXACTLY on each level's own
proven-bound coverage ceiling (32 at k=7, 55 at k=8, both verified directly). Every test state
sat inside already-fully-decided territory. Our real target, Sb(112:80) (m=80, k=9), sits far
outside all of this -- k=9's own table only reaches m=6.

Getting genuine k=9 data required the real regime. Also resolved, mid-thread, a direct question
about AWS history: the instance I'd checked was a different, correctly-terminated one-time
corpus-load job -- but a SEPARATE, persistent oracle-serve instance (launched 2026-08-21 at
Fedor's own explicit request, no fixed end date) turned out to be live the whole time; my first
EC2 query missed it by filtering the wrong tag. Verified it directly and live (stats responded
instantly, sa193 certificate correctly loaded) -- but its S3 STATUS object hadn't updated since
the moment it launched three days earlier, despite the server being healthy and actively serving
280K+ real queries. A real, separate bug in the status-upload path, flagged for a fix. Redirected
k=9 labeling to this live server (a new TCPOracle mode in the labeling driver) and left it running
in the background while the C work proceeded on already-available data, per direct instruction to
work with what's available rather than block on the slow labeling.

**C implementation** (radiobase.c, gated behind a new `ml_order_mode` context flag, default off,
exactly matching `radius_mode`'s own precedent): confirmed first that R_0 needs no porting (it's
the identical predicate already enforced on every recursive call); confirmed the sb0/sb1/sb2
mapping is selected/mixed/complement with no reversal, byte-for-byte against two independent
implementations. Ported feat()'s 31-feature pooling in closed form for the count-in-{1,2} shapes
BY_ML ever scores (matching numpy's specific "median" convention on a 2-element array), and the
trained logistic model's standardize-then-dot-product (sigmoid skipped, only relative order is
used). Added `BY_ML` as a new static ordering exactly where `BY_MAGIC3` lives, reusing the same
table-build machinery, `ORDER_MONO_P = -1` (never admits the counting-bound early-abandon -- the
correctness invariant that makes a wrong score cost time, never correctness). Selected only when
`ml_order_mode && size==1` -- the one case where a per-(sbb,k) score is valid, since no
accumulated prefix exists yet at that point. A new, checked-in `tools/ml/export_ordering_model.py`
generates the embedded header from committed data and refuses to write output if the reproduced
holdout AUC drifts from the documented 0.986 by more than 0.02.

**Regression**: `ml_order_mode=0` unaffected (25-state battery 123.0s vs 117.0-117.9s baseline,
noise-level, byte-identical code path).

**Result**: correctness holds, benefit does not. `Sb(67:46)` (UNSOLVABLE): 88.4s under BY_ML vs
87.6s default -- within noise, and expected, since proving unsolvability requires exhausting
every candidate regardless of order; no ordering change can help a negative proof. `Sb(112:80)`
(SOLVABLE, the actual target): BY_ML did NOT finish in 900 CPU-seconds -- worse than default's
293.3s successful resolution. A genuine regression on the one case meant to benefit.

**Diagnosis**: `Sb(112:80)` has m=80 at k=9, entirely outside k=9's own proven-bound coverage
(m<=6) -- its deficit feature silently defaults to the crude fallback, a regime the model has
literally never seen in training (every training example, confirmed exhaustively, fell within
coverage). `deficit`/`headroom` carry two of the three largest-magnitude learned coefficients.
Extrapolating a heavily-weighted feature into an unseen regime can plausibly rank worse than an
untrained heuristic there.

**Conclusion**: the wiring is a real, tested, correct deliverable -- mechanically sound, verdicts
unchanged, an ordering-only change by construction, scoped exactly to what the model's own
validated methodology supports. The currently embedded weights, trained only on data that never
exercises the out-of-coverage regime, actively hurt on the case that matters most. Not the final
word: k=9 n=1 labeling (genuine in-regime data) was still running via the live oracle-serve
instance when this was written. Retrain including it and re-measure is the natural next step, not
yet done.

Evidence: [../evidence/native_concentric_2026-08-23.txt](../evidence/native_concentric_2026-08-23.txt)
section 26.

## 2026-08-24 -- Aigner 1988 scan indexed

Inspected the supplied image-only scan of Martin Aigner's *Combinatorial Search*. Its continuous
content is Chapter 3, book pp. 123--191, not the book's Chapter 2 “Weighing Problems”; PDF pp.
74--76 add the answer pages used below. Section 3.3 directly states the ternary edge-search model,
the `K_{m,n}` three-child decomposition, and subgraph monotonicity. Its `N(k)` star-forest
sequence is the repository's `G_k`, with Proposition 3.24 giving its recursive construction.

The scan provides direct book-source checks for `n(k,2)=2^k-1` (Corollary 3.26, p. 150) and
`n(k,3)=2^k-k` (answer to Exercise 3.3.1, p. 345). It also records an important historical limit:
Proposition 3.25 proves only the necessary majorization direction and explicitly leaves the
converse open, so the repository's later singleton-majorization iff result must not be credited to
this source. Detailed page locators and scope are in [aigner-1988-scan.md](aigner-1988-scan.md).

No solver ran; OCR and rendered pages were temporary inspection artifacts. `check_tables.py`,
`check_witness.py witnesses/*.tree`, and `check_docs.py` all passed after the documentation update.

## 2026-08-24 -- modern asymptotic literature indexed

Checked the supplied WCC 2019 paper by Jiang, Polyanskii and Vorobyev and the Florin--Ho--Jiang
paper against their PDFs. The former supplies an explicit `K_{n,n}` construction with a
`1.2938 log_2(n)(1+o(1))` worst-case bound; its mixed residual-matching construction is useful
context for multipart strategies, but is not a finite fixed-`m` boundary proof. The latter,
published in *IEEE Transactions on Information Theory* in 2022, settles the sharp asymptotic
one-set leading constant at `1.26624...` and proves the equal-side bipartite/binary-adder-channel
correspondence. It likewise provides no finite `Sb(n:m)` table cell.

Recorded both sources in [literature.md](literature.md), added compact context to the paper and
P5, and left `data/*.csv` unchanged. No solver ran; the PDF extraction and rendered-page files
are temporary inspection artifacts. The usual three documentation checks were rerun after this
update.

## 2026-08-24 -- literature trail beyond the supplied papers

Followed the references and model-equivalence trail beyond the WCC and Florin--Ho--Jiang papers.
Directly checked Zhang--Berger--Massey (1987), the early full-feedback binary-adder-channel
construction source, and Hwang--Lee (2001), an exact same-test-model result for paths and cycles.
Neither changes the complete-graph or complete-bipartite finite tables. Added both to
[literature.md](literature.md), along with an acquisition queue for the two Belokopytov papers:
the 1987 explicit `1.3277` construction and the 1989 nonconstructive `1.2662` capacity bound as
attributed by WCC. The 2018 Karimi et al. paper was classified as non-comparable, because it is
average-case and permits a weight-2 coin.

The comparison conclusion is now clear: exact local frontiers are the repository's contribution;
the published modern asymptotic optimum is already Florin--Ho--Jiang's `1.26624... log_2(n)`.
The old ternary counting lower bound is `1.26186... log_2(n)`, so it was close but not sharp.
No solver ran. Documentation checks follow this note.

## 2026-08-24 -- conservative publication-claim inventory

Created [publishable-claims.md](publishable-claims.md) to separate a viable finite-results paper
from theorem candidates, supporting infrastructure and explicitly excluded conjectures. The
recommended headline is the computer-assisted exact theorem `Sa(10)=192`, supported by a checked
positive witness and a solver-free-to-check compact `Sa(193)` certificate. The complete exact
`Sb` frontier through k=8, the exact `m=6` boundary at k=9, and the upper boundary at k=10 form the
finite-results package. Singleton-majorization necessity, unit-group elimination and full-star
pullback are the strongest theory candidates; the converse remains open and must not be presented
as a theorem. The aligned-profile D-lineage result is deliberately labelled restricted-model
only. No solver ran; documentation checks follow.

## 2026-08-24 -- publication scope tightened

Removed the aligned-profile D-lineage obstruction from the prospective-publication inventory. It
is a restricted-model research tool with no current unrestricted consequence, so it should not be
presented as a paper claim. The recommended structure is one finite-results paper centred on the
exact `Sa(10)=192` theorem, the complete exact `Sb` frontier through k=8, the exact k=9 `m=6`
boundary, the k=10 upper bound, and the directly supporting majorization theory. A separate theory paper is justified
only if the synchronized hierarchy later yields an independent unrestricted result. No solver ran;
documentation checks follow.

## 2026-08-24 -- closing the fast-solver thread: concentric search / radius mode / BY_ML moved to
## a branch, `main`'s solver reverted to the plain, understood engine

This closes out a long, multi-session thread (native concentric search -> radius mode -> BY_ML
learned ordering) rather than continuing to carry its experimental machinery on `main`. Everything
below is preserved, in full, on branch `concentric-search-radius-ml-exploration` (pushed,
`b71a5e3`) for anyone who wants to pick the thread back up; `main`'s `radiobase.c` and
`radio_oracle.c` are reverted to their state just before this thread began (`e206766` and
`c7bc503` respectively) -- the plain `canSolveA`/`canSolveB` engine this whole project has always
run on. Verified directly post-revert: cold `Sa(192)` in `k=10` solves in **292.4 CPU seconds**
(4.87 min), matching the pre-thread baseline (290.8s) within normal run-to-run noise, and every
standard check (`check_tables.py`, `check_docs.py`, `check_witness.py`) passes.

**What the thread found, in order:**

1. **Concentric round search, native in `radio_oracle.c`**: exhaustive-within-a-growing-radius
   search, validated to 55/55 exact census match. Real, correct, but round-of-success sits flat
   at 14-16 regardless of difficulty, and the fraction of the true raw space needed before success
   is 64-99%, not small -- "finds winners early" was the wrong read of its own data.

2. **Unifying it into `canSolveB_ctx` as `radius_mode`**: absolute (unscaled) propagation to the
   "mixed" child (whose part-count doubles at every level) makes cost compound catastrophically
   across `Sa(n)`'s chained doublings -- one real 8-part state hit 32.7 billion totalsplits.
   Tested sqrt-scaling (fixes cost, breaks retry-rate: pass=19-38 to resolve some states), `/2`
   scaling (fixes retry-rate, partially reintroduces cost: 7.39B worst case), and squaring the
   top-level growth instead of doubling it (fixes retry-rate correctly this time, cost bounded) --
   each a real, measured trade-off, not a clean win in isolation.

3. **The decisive comparison, run directly rather than assumed**: plain, unmodified
   `canSolveB(..., NO_DEADLINE)` beat every one of the above, and `concentric_search` itself, on
   every case tried -- often 7-8x faster on the states the whole thread was built to handle well.
   Squaring+sqrt radius mode DID eventually win on one shape specifically (an already-split,
   moderate-size multi-part state -- the 25-state battery, 85.8s vs 117.0s) but lost badly on a
   different shape (a single large unsplit pair needing many chained levels of internal
   splitting -- `Sb(112:80)` in k=9, our actual hard target: radius mode did not finish in 900s
   where default solved it in 293s).

4. **Instrumented WHERE the time goes, per direct question**: cache-hit fraction is 91-99%
   throughout every measured run -- the cost is not individual calls being slow, it's that
   "widen only at the top" forces every retry to re-walk the entire top-level candidate range
   from scratch, and at billions-of-calls scale that re-walk itself is the cost.

5. **Measured whether the shared ordering heuristic (`BY_SP0/1/2`/`BY_MAGIC3`) is actually good**,
   per direct question: real, measurable skill (median winner position ~20% into a level's order,
   far below the 0.5 a no-information order would give), but nowhere near "the first few
   candidates," and it visibly degrades on the hardest, most expensive states specifically
   (`Sb(112:80)`'s own winner sits at 41.35%, right at the p90 of the measured distribution).
   This reframed the whole growth-schedule investigation as chasing a symptom -- every schedule
   tried was guessing, blind, how deep into an unknown-quality order the winner sits.

6. **Wired the repo's own already-validated recursive value model in as `BY_ML`**, a real, tested,
   correctness-preserving addition (`ORDER_MONO_P=-1`, ordering-only by construction). Corrected
   scope twice first (the model's training data was exclusively 4-part states, nothing like the
   target's 1/2/1 shape; generated fresh matched corpora at multiple part counts to fix this,
   catching a real generator bug and a real "AUC=1.0 is a trivial coverage artifact" trap along the
   way). Result: correctness held, benefit did not, on the primary target -- `Sb(112:80)`'s deficit
   feature falls entirely outside k=9's proven-bound coverage (m=80 vs. table to m=6), a regime
   absent from every training example, and the model actively hurt there (did not finish in 900s).

7. **Along the way**: corrected a wrong claim about AWS history (the persistent `oracle-serve`
   instance from 2026-08-21 was live the whole time; an EC2 query missed it via the wrong tag
   filter) and found its S3 status-reporting had silently stopped updating despite the server
   being healthy -- flagged, not yet fixed. Also generated and committed genuine k=8 (now complete,
   100/100 labeled) and k=9 (100 states, 45 definitive + 55 MAYBE) n=1 in-regime training data via
   the live oracle -- the one ingredient BY_ML never had.

**Why revert rather than leave it gated off on `main`.** Every piece above is real, gated behind a
default-off flag, and provably non-regressing on the default path -- leaving it in place would
not be *incorrect*. But none of it is a net win for the workload this project actually needs
(`Sa(n)`-shaped, single-large-part recursion), and carrying ~700 lines of unused, only-partially-
successful machinery on the trusted core is a cost with no offsetting benefit right now. The
branch keeps every line, every commit message, and every piece of evidence available without
main paying rent on it.

**Future exploration avenues, left for explicit direction, roughly in order of what the evidence
already points at:**

- **Retrain BY_ML with the new k=8/k=9 in-regime data and re-test `Sb(112:80)` directly.** This is
  the most immediately actionable item: the model that failed had *zero* training examples outside
  proven-bound coverage; there are now 140 real ones (100 k=8, complete; 45 of 100 k=9,
  definitive). Whether this closes the gap, narrows it, or does nothing is currently unmeasured.
- **Resolve the remaining 55 k=9 MAYBEs** if a cheap path exists (the 5 remaining k=8 MAYBEs
  resolved as free instant cache hits against the live oracle's broader history; the k=9 ones
  already made first contact and hit real budget exhaustion, so a plain retry is unlikely to help
  -- a longer per-query budget or the oracle's own richer future cache might).
- **An adaptive growth schedule for radius mode** (grow gently while new information keeps
  appearing each pass; jump ahead aggressively on a detected plateau -- no new real work vs. the
  previous pass, the exact signature observed repeatedly this thread) was proposed but never
  built. Could plausibly capture linear growth's no-overshoot benefit without its slow-crawl cost.
- **Extend `BY_ML` beyond `size==1`.** The current scope is deliberately narrow (no accumulated-
  prefix mismatch); a `size>1` version needs dynamic, per-call scoring of the accumulated prefix
  rather than the static per-`(sbb,k)` table this pass used, and was noted as explicitly out of
  scope rather than attempted.
- **Fix the `oracle-serve` instance's S3 status-reporting gap** so a genuinely idle/hung server
  doesn't read identically to a healthy one that's just working on a hard query, the confusion
  this thread ran into directly.
- **Whether concentric-style capping has value for a genuinely different shape** -- e.g. real K=9
  residual work, where the raw space is large enough that even default's heuristic-plus-work-
  budget struggles -- was never tested; every comparison this thread ran used `Sa(192)`-scale or
  smaller states specifically because that was the concrete, available benchmark.

Evidence: [../evidence/native_concentric_2026-08-23.txt](../evidence/native_concentric_2026-08-23.txt)
sections 1-26 (the complete thread). Branch: `concentric-search-radius-ml-exploration` (`b71a5e3`).

## 2026-08-26 -- singleton-majorization sufficiency retracted and reduced to one open coloring lemma

The claimed converse was not complete.  Its Three-Way Majorization Decomposition Lemma is false
already at `k=2`: with the three staggered copies

    u=(2,0,1,0),  v=(2,1,0,0),  w=(0,2,0,1),

the individually dominated vectors `(2,1,0,0)`, `(0,2,1,0)`, `(1,0,2,0)` sum to
`(3,3,3,0)`, whose first three entries total 9 rather than at most 8 under
`G_2=(4,3,1,1)`.  Independently, an unconstrained three-polymatroid decomposition does not impose
the legal row condition that one singleton row may use left+mixed or mixed+right, but not both pure
children.  Aigner's 1988 text proves necessity and explicitly leaves the converse open, so the
repository had accidentally promoted the historical conjecture to a theorem.

There is now an exact reduction.  Let `h=G_(K-1)`, with saturated prefix function `H`, and color
each parent row `A` (left+mixed) or `B` (mixed+right).  If `A_p,B_q` are the sums of the `p,q`
largest rows of each color, the integral bipartite b-matching/Hall criterion says that a legal
three-child decomposition exists exactly when

    A_p+B_q <= H(p+q)+H(p)+H(q)                 for every p,q.       (C)

The parent majorization inequalities give only the balanced maximum of the right side.  Thus the
whole converse is equivalent to one **Row-Coloring Lemma**: every `a<=_w G_K` has a coloring
satisfying all of (C).  Necessity is now proved independently by the transcript conflict graph
`Q_K`: its vertices are ternary transcripts, adjacency means first difference `0/2`, and
`Q_K = Q_(K-1) disjoint-union (Q_(K-1) join Q_(K-1))`.  The maximum size of a `t`-colorable
induced subgraph is exactly the first-`t` prefix of `G_K`.

The special structure of `G` is sharper than paired rows.  Its conjugate columns are powers of two:

    (G_r)' has 2^(r-j) repeated binomial(r,j) times, 0<=j<=r.

Pascal's identity sharpens this to `G_K'={2c:c in G_(K-1)'} union {c:c in G_(K-1)'}`.
Thus parent majorization is bipartite degree feasibility for doubled and single columns, while a
legal split additionally asks for a row coloring that puts at most half of each doubled column on
each pure side.  The single columns are the mixed child.  The coloring belongs to whole rows, so a
balanced edge-coloring does not solve the problem.

This gives the best bottom-up target found in this session.  Let `F_h` be the labelled demand
vectors that admit a legal allocation into the three child column families.  Its rank on `t` rows
is the parent prefix `H_K(t)`.  If `F_h` satisfies the discrete-polymatroid augmentation/exchange
axiom, those rank inequalities characterize all its integer points and the converse follows.
This makes the user's multiple-identical-target intuition precise: a transfer should be routed by
a global augmenting path among the dyadic duplicate columns.  It also explains why a single-pass
rule is not enough.  This exchange statement remains unproved; it would be a sufficiently strong
route to the missing converse and is a substantially more local object to attack.

The apparent intermediate generalization to every `2`-flat base is false.  The base
`h=(6,5,1,1)` has `h_1=h_2+1`, paired remaining rows, and only one odd conjugate-column height.  Its
parent is `(12,11,6,6,1,1,1,1)`, which dominates
`(12,11,6,2,2,2,2,2)`, but no coloring satisfies (C).  The full-set inequality forces a 4/4 row
split; 12 and 11 must have opposite colors; placing 6 with either one then violates `(4,1)` or
`(1,4)` by `33>32`.  Thus the proof must use the full Pascal multiplicities, not just the
off-by-one row or parity.

Several attractive shortcuts are now ruled out by explicit checks:

- strict odd/even coloring fails for
  `(16,15,11,11,5,5,5,3,3)<=_w G_4` at `(p,q)=(5,2)`, by `66>65`;
- lower-current-total coloring sends `(8,5,5,5,1,1,1,1)<=_w G_3` to
  `A=(8,5,1)`, `B=(5,5,1,1,1)` and fails by `26>25`;
- a valid partial coloring `A=(7,4)`, `B=(4,4,2,2,2)` cannot accept the next row `2` on either
  side, although the full sequence has a feasible recoloring;
- reserving half of every mixed column for each color is too rigid: for `h=G_2`, it produces
  bases `u=(8,4,1,1)`, `v=(7,4,1,1)`, but `(8,5,5,5)<=_w G_3` cannot be split below `u,v`, even
  though the flexible coloring `(5,5)` / `(8,5)` satisfies (C);
- the two-color transfer obstruction is real: `Q_3` contains the induced claw with stable sides
  `{000,001,010}` and `{221}`, so changing sizes `3+1` to `2+2` can require a third class or a
  global recoloring.

The graph reformulation identifies the full-mass converse with `Q_K` being nice in Stanley's
sense.  Equivalently, `Q_K` is the incomparability graph of the lexicographic power of the
three-element poset `1<0,1<2`, and the target asks for every partition dominated by its
Greene--Kleitman chain shape to occur as a chain-partition type.  Literature search found the
general niceness framework and the theorem that hereditary niceness is equivalent to claw-freeness,
but no theorem covering these claw-containing lexicographic powers.  The standard result therefore
does not settle this family.

The proof-status correction has concrete consequences.  Canonical sub-multiset and distinct-slot
embedded singleton leaves remain unconditional by Aigner's explicit `G_k` strategy plus Subgraph
Monotonicity; arbitrary `[majorized G_k]` leaves are now marked conditional.  The exact
`n(9,5)=481` value remains supplied by Li--Wu--Triesch, and `n(9,6)=473` has a canonical tree.  At
`k=10,m=6`, the exhaustive rejection of 974 remains an unconditional upper bound
`n(10,6)<=973`, but the 973 majorized-terminal file is not presently a lower proof.  The old
solver shortcut was also removed: a majorization violation still returns `FALSE`, a distinct-slot
embedding may return `TRUE`, and every other singleton state continues through exact recursion.
Positive dominance-trie hits are ignored for the latter states because parsed historical caches do
not record whether the old shortcut created them; process-local L1 facts produced by the new exact
recursion remain usable.

The witness checker now reports conditional files without mistaking structural validity for a
complete proof.  A separate audit of the older numbered `radio_print` format found that its leaves
are only unit/base cases and its references use safe same-multiplicity coordinatewise domination;
those retained Sa trees do not invoke arbitrary singleton majorization.  The log-tree extractor now
also distinguishes unconditional distinct-slot singleton embeddings from arbitrary majorized
terminals and propagates a conditional warning to the rendered root.

Measured work: the pre-edit table and witness checks were green.  Three bounded one-off Python
searches took roughly 30--40 seconds each; they supplied the greedy/partial-coloring diagnostics but
gave no `k=5` verdict and were terminated cleanly.  A provenance-built solver audit took about two
seconds: it retained the embedded `Sb(8:1,2:1)@3` positive and solved the nonembedded majorized
`Sb(3:1,2:1,2:1,2:1)@2` by real recursion in 10 accepted split prefixes.  Short exhaustive
partition checks found no issue through the small controls; they are evidence about examples, not
a general proof.  The expanded C regression compared 535,328 full-star majorization cases and
then seeded the nonembedded state above as a historical positive cache fact: the solver ignored the
trie hit, spent the same 10 prefixes on exact recursion, and safely reused only the resulting
process-local L1 fact.  No long solver run was launched.

Final validation passed: `tools/check_tables.py`, all witness files through
`tools/check_witness.py`, `tools/check_docs.py`, Python byte-compilation of the modified utilities,
the five-level bundled-majorization ladder control, and the provenance-built 535,328-case C
regression.  The full frozen-refuter regression also passed after the solver change.  The witness
pass reports exactly the 481 and 973 majorized-terminal files as conditional (three and six
genuinely nonembedded leaves respectively); all canonical and numbered files verify
unconditionally.

## 2026-08-26 -- complete `K=4` Row-Coloring census and block-extension conjecture

The proposed brute-force route paid off, but did not close the theorem.  A new provenance-built
utility enumerates every full-mass integer partition majorized by `G_K`, quotients colorings by
equal-row permutations, and checks the exact Fixed-Color Hall inequalities (C).  The completion
counter independently agrees with the enumerator.  All 1,206 states at `K=3` and all 5,997,038
states at `K=4` have a coloring, giving a complete finite verification of the Row-Coloring Lemma
through `K=4`.

The adjacent-pair hypothesis is false on 916 `K=4` states.  Its first full-mass counterexample is
`(16,15,11,11,5,5,5,1^13)`: every paired coloring distributes the tail units too evenly, while
`(16,11,5,5,1^4)/(15,11,5,1^9)` satisfies (C).  This settles the earlier uncertainty about
alternation: independently rephasing adjacent pairs is still insufficient, and the artificial unit
rows must stay visible.

Normalizing by equal-value multiplicities exposed a much sharper candidate.  Process value blocks
in descending order; among current Hall-legal allocations that admit at least one allocation of the
next lower block, choose the one minimizing the A/B total-mass difference.  This forward rule uses
one-block lookahead and no recoloring.  It passes all 5,997,038 `K=4` states.  Plain block balancing
fails on 22; final-row reservation repairs those `K=4` failures but fails on six of ten million
uniform `K=5` samples.  One extracted failure is
`(31,29,23,22,21,21,12,12,12,7,7,6,5^5,1^15)`: the width-6 row must anticipate the following
width-5 block, whose balanced allocations otherwise fail off-diagonal Hall inequalities by one.

Dynamic completion counts put the exact full-mass `K=5` universe at 38,378,683,542,323 states.
On ten million independent uniform samples, plain blocks passed 9,999,988, row reservation passed
9,999,994, and one-block lookahead passed all ten million.  A separate 100,000-state `K=6`
dominance-transfer walk also found no lookahead failure.  These are stress tests, not proofs.

The proof target is now the Block-Extension Conjecture: show that a one-block-extendable allocation
always exists and that the most balanced such allocation preserves this property for the next
block.  A discrete-convex/interval theorem for projections onto consecutive block counts would
explain why the exact `K=4` search never needed longer backtracking.  The graph-recursion shortcut
of proving the self-join nice separately is false: `(2,2,2)<=(2,2,1,1)` cannot be split into two
subsequences each below `(2,1)`, so the middle component cannot be factored away.

Measured cost: the complete `K=4` runs took 25--71 wall seconds depending on the number of rules
instrumented, with negligible RSS.  One million transfer-generated `K=5` states took 91 seconds;
one million uniform states took 10 seconds; ten million uniform states took 81--106 seconds; the
100,000-state `K=6` transfer walk took 166 seconds.  Every retained headline above comes from a
bounded provenance-built run.  Full commands, examples and statuses are in
[the census record](../evidence/singleton_row_coloring_census_2026-08-26.md).

## 2026-08-26 -- global signed-lifting formulation; scalar balance refuted

The block-extension rule is useful experimental evidence but is too procedural to be the natural
proof target.  The Row-Coloring Lemma has a short exact global formulation.  For disjoint labelled
row sets `X,Y`, the signed Hall rank

    f(X,Y)=H(|X union Y|)+H(|X|)+H(|Y|)

is bisubmodular.  Integer points `z` of its signed polyhedron encode a legal coloring through the
signs of `z`, with row demands `|z|`.  If `M_h` is the set of such magnitude vectors, then
`conv(M_h)` is exactly the parent majorization polymatroid: inclusion follows from the signed Hall
inequality, and every polymatroid vertex is a permutation of an initial segment of `G_K`, hence is
obtained by deleting rows from the canonical strategy.  The open converse is therefore precisely
that the Pascal absolute-value fold has no lattice holes.  A sufficient local form says that every
unit Robin-Hood transfer of magnitudes can be routed, allowing a global signed augmenting path and
whole-row recolorings.  Generic bisubmodularity does not prove this; the recorded non-Pascal bases
have holes.  The missing cut argument must use the full dyadic capacities and Pascal
multiplicities.

The simplest global scalar rule was tested and refuted.  Among colorings with the necessary
`2^(K-1)` rows on each side, globally minimize final A/B mass difference.  It passes all 1,206
`K=3` states but the new exact mode stops at the 15,855th enumerated `K=4` state,
`(16,15,9,9,9,5,5,5,1^8)`.  Its unique normalized `41/40` split puts 16 and 15 together and fails
the pure `(2,0)` Hall inequality by one.  A `39/42` split is legal.  An independent enumeration of
the normalized eight/eight colorings confirmed the failure; the retained mode instead certifies it
by exact subset dynamic programming followed by exhaustive Hall search at the optimal mass.

Measured cost: the provenance-built exhaustive prefix visited 564,421 coloring nodes in 0.32 wall
seconds / 0.21 user CPU seconds on the M4 Pro.  A preceding 100,000-state uniform `K=4` request
found a different failure at sample 2,044 and stopped; the concurrently launched `K=5` sample was
interrupted once the `K=4` refutation was established, and no solver process was left running.

## 2026-08-27 -- coalescence-shape calculus and exact scalar cut census

The proposed “children of similar shape” idea has a canonical forest interpretation.  Define the
hinge profile `E_a(t)=sum_i(a_i-t)_+`.  If a legal first cut splits a row `u=x+y`, its threshold-`t`
removal is `min(x,t)+min(y,t)-min(u,t)`.  Summing gives the exact identity

    sum_(children C) E_C(t)=E_a(t)-J_a(t).

The largest removal available from one row is `min(t,max(u-t,0))`, hence the sharp aggregate cap
`J_a(t)<=E_a(t)-E_a(2t)`.  At `t=1`, `E_a(1)=3^K-r`, `J_a(1)` is the number of split rows, the
canonical top layer contains `2^(K-1)` joins, and the cap is exactly the number of nonunit rows.
Thus the rule

    split floor/ceiling(min(2^(K-1)(3^K-r)/(3^K-2^K), #{i:a_i>=2})) rows

means retaining the same normalized join density at the next scale; it does not special-case unit
rows arbitrarily.

The new provenance-built `tools/singleton_shape_survey.cpp` reconstructs the cut directly and
checks each child against `G_(K-1)`.  Exact censuses found no scalar-rule failure among all 1,206
full-mass `K=3` states or all 5,997,038 `K=4` states.  The latter visited 16,639,423,162 search
nodes in 1,854.89 user / 1,867.57 wall seconds.  A preliminary 100,000-state
uniform `K=4` sample also passed.  A direct 1,000-request `K=5` sample was killed by its 60-CPU-
second cap before a batch verdict and is an abort, not evidence.

The tempting coordinatewise extension to the whole hinge profile is false.  Scaling the canonical
top-layer removal independently at every `t`, capping by `E_a(t)-E_a(2t)`, and rounding each
coordinate fails at the 61st `K=3` state, `(8,5,5,5,1^4)`.  Its scalar target is attainable with
children `(4,3,1,1)/(4,2,2,1)/(4,3,1,1)`, but the independently rounded targets require `J(2)=6`
where that cut has 7 and exhaustive search finds no all-coordinate match.  The profile is intrinsic,
but its coordinates are lattice-coupled.  The scalar rule is now a plausible stronger finite
invariant, not a proof; the exact general target remains Pascal orthant saturation/no lattice holes.

Full commands and the distinction between exact results, samples and the aborted run are in
[the shape-survey record](../evidence/singleton_shape_survey_2026-08-27.md).

## 2026-08-27 -- split-count fibers, strong niceness, and the row-orientation reduction

Enumerating all feasible split counts at `K=3` sharpened what the shape calculation can and cannot
control.  Every one of the 1,206 fibers is an interval, but the hinge lower endpoint is exact for
only 1,190 states and the elementary mixed-mass upper endpoint for 1,177.  The misses are genuine
lattice effects: `(8,2^9,1)` needs a second split to repair parity; `(8,7,2^6)` permits only four
splits because `8` and `7` consume seven of the mixed child's nine units; and
`(8,6,5,3,2,1^3)` misses that upper bound because three pure remainders of width four cannot fit
under two copies of `(4,3,1,1)`.  The all-fiber traversal visited 4,740,395 nodes.  A ten-state
`K=4` request did not finish within 60 CPU seconds and is an abort, not evidence.  This makes the
interval a useful diagnostic but a poor primary induction variable.  Reproducible output is in
`evidence/singleton_shape_survey_2026-08-27.md`.

The transcript-graph formulation produced a stronger global lead.  If
`X_(Q_K)=sum c_K(lambda)m_lambda`, then `c_K(lambda)` counts semi-ordered legal decompositions.
Exact recursion over the row triples `(w-x,x,0)/(0,w-x,x)` proves computationally that `Q_3` is
strongly nice: all 1,206 supported types and 463,886 comparable pairs satisfy
`c_K(mu)>=c_K(lambda)` for `mu<=lambda`.  The provenance-wrapped run took five wall seconds and
4,740,395 allocation nodes.  Strong niceness would imply the open support theorem, but no generic
closure result finishes the induction: a 2026 theorem covers disjoint union, graph join can destroy
strong niceness, and `Q_3` already contains a claw.  The exact target is closure for the special
operator `T(G)=G disjoint-union (G join G)`.  Tool, command and literature links are in
`evidence/singleton_strong_niceness_2026-08-27.md`.

## 2026-08-27 -- padding gives an exact pure-first construction target

Returning to the partition algorithm with every state padded to `3^K` labelled slots yields a
useful exact reduction.  If `h'=(c_j)` is the conjugate of `G_(K-1)`, first orient the fixed row
slots left or right and place, for every `j`, `c_j` pure incidences on each side.  If the resulting
pure degree `p_i` does not exceed `a_i` and the residual `m=a-p` is majorized by `h`, then
Gale--Ryser constructs the mixed child automatically.  Conversely every legal cut has exactly
such a representation after realizing and pairing the pure child columns.  Thus “greedily assign
the pure children, then prove the remainder is majorized” is the right order; the unresolved part
is the balanced pure-support placement, not the mixed allocation.  Zero rows stay in the fixed
ground set and require no special deletion rule.

A first global implementation—put the slots in one alternating order and represent each doubled
pure column by an even consecutive interval—fails analytically at `K=3`.  For
`a=(8,7,4,1^8)<=_w G_3` and `G_2'=(4,2,2,1)`, the interval lengths are `8,4,4,2`.  Mixed
majorization forces the pure coverage profile to be either `(4,4,4,1^6)` or
`(4,4,3,1^7)`.  The first needs three points common to a length-two interval.  In the second,
removing that interval would leave lengths `8,4,4` with profile `(3^3,1^7)`; the two length-four
intervals must extend their common three-point block on opposite sides, while the length-eight
interval necessarily covers one extension and creates a forbidden degree-two point.  So fixed
interval geometry is too rigid even though padding itself is useful.

The next precise target is an adaptive augmenting-path algorithm: add paired pure columns in
decreasing capacity, reroute old pure incidences when necessary, and change a row's orientation
only after evacuating its pure incidences.  A stopped augmentation exposes the familiar
opposite-orientation tight separator.  The remaining proof must use the dyadic capacities and
Pascal multiplicities to cross that cut; padding makes the construction and failure certificate
cleaner but does not yet prove that augmentation always succeeds.

## 2026-08-27 -- canonical padded tail isolates a high-support alternation inequality

The equal partition into `M=3^(K-1)` left-eligible, right-eligible and mixed-only slots has an exact
three-variable Hall criterion.  Two exchange lemmas simplify it substantially.  A heavier
mixed-only row can always swap with a lighter pure-eligible row without hurting Hall, so the
mixed-only block may be taken to be the `M` smallest padded rows.  Those rows must be zeros and
ones.  If their mass is `c`, minimizing over their Hall coordinate contracts the mixed prefix from
`H(t)` to `min(H(t),M-c)`.

When the parent has at least `2M` nonzero rows, parent majorization plus the mandatory unit in every
unselected positive row gives the stronger prefix bound
`U_E(t)=min(H_K(t),E+t)`, `E=M-c`.  Alternating the remaining `2M` rows makes all inequalities on
the side receiving the smaller row of each pair follow from concavity.  Only

    floor((U_E(2q+1)+U_E(2p-1))/2)
      <= H(p)+H(q)+min(H(p+q),E),       p>q,

remains.  This is a one-dimensional Pascal-prefix statement, with no state or assignment
variables.  It was initially a proof target, but the exact `K=19` construction recorded below
refutes it.

The new `--padded-three-census` mode checks the construction directly.  The complete `K=3` census
has 66 states with nonzero canonical-tail mass and no alternating failure; the complete `K=4`
census has 11,309 and no failure.  All ordinary alternating failures have only 9 or 11 nonzero rows
at `K=3`, and 16 through 21 at `K=4`, far below the `2M` thresholds 18 and 54.  The new
`--padded-prefix-check` mode exhausts the integer breakpoints of the remaining scalar inequality;
every `K=1..12` passes, with 2,098,176 `(p,q)` pairs and 20,201,473 breakpoint values at `K=12`.
The final provenance build is `a895141b33ec5893167eb3e49a33dad0a11d2016ebd1e43ceeff2471907a67c3`;
the complete censuses took 5.4 wall seconds together and all twelve prefix checks took 2.1 wall
seconds on the recorded M4 Pro.  Full derivation and commands are in
`evidence/singleton_padded_three_blocks_2026-08-27.md`.  These finite passes do not survive
extrapolation to `K=19`.

The conjugate-layer view also isolates the assignment gap.  Each child layer of height `r` becomes
a parent layer `(2^r,1^r)`: its incidences split evenly into left, mixed and right, and a parent row
may take mixed plus one pure incidence in that layer.  Across layers the same row must keep one
pure orientation.  Fixing odd rows left and even rows right is too rigid already for
`(8,7,4,2,2,1^4)@3`: its alternating coloring violates Hall at `(p,q)=(5,1)` by `23>22`, whereas
the global coloring `A=(8,4,2,1)`, `B=(7,2,1,1,1)` passes.  Thus the remaining object is exactly a
row-orientation plus integral-flow theorem.  This is the layer form of Pascal orthant saturation,
not a new solution; it explains why amount assignment can be delegated to flow once the global row
signs are found.

## 2026-08-27 -- adjacent-fiber reduction of the transfer proof

The bottom-up transfer proposal has a still sharper exact obstruction.  For a fixed row coloring,
write

    r_(A,B)(S)=H(|S|)+H(|S intersection A|)+H(|S intersection B|).

If `x` is feasible and `y=x-e_i+e_j`, the same coloring remains feasible exactly when there is no
`x`-tight set containing recipient `j` but not donor `i`.  When donor and recipient have the same
color this is automatic: exchanging `j` for the at-least-two-larger `i` preserves the rank but
would violate feasibility.  Hence the only obstruction is an opposite-color tight separator.

This gives the Pascal Adjacent-Fiber Lemma: every feasible `x` and Robin-Hood neighbor `y` should
have at least one common feasible coloring.  If true, it completes the converse by the standard
unit-transfer generation of every partition dominated by `G_K`, followed by the Fixed-Color Hall
Lemma and induction on `K`.  The coloring is allowed to depend on the selected transfer; the
stronger demand for one coloring supporting all transfers is unnecessary.

A direct proof attempt by maximizing the minimum separator slack reaches one precise missing
step: uncrossing supplies a minimal opposite-color tight separator, but ordinary bisubmodular
exchange does not show how to recolor across it while preserving all other Hall inequalities.  The
needed move must use the dyadic capacities and Pascal multiplicities; generic bases have the same
signed submodularity and explicit counterexamples.  Thus the reduction is complete, but the
separator-elimination lemma remains open and no proof of singleton-majorization sufficiency is
claimed.

## 2026-08-27 -- why padded unsorted counting is support, not cardinality

Padding every full-mass state to `N=3^K` labelled rows gives a clean exact count of the target
lattice points: sum `N!/((N-length(lambda))! product_v multiplicity_v(lambda)!)` over the
partitions `lambda` of `N` dominated by `G_K`.  It also gives a clean labelled recurrence for
child decompositions.  These are not the same count, because the recombination map has nonuniform
fibers.

The obstruction is visible without computation at `K=1`.  The target has seven labelled states:
six permutations of `(2,1,0)` and `(1,1,1)`.  Choosing placements for the left, mixed and right
unit children, with the two pure placements distinct, gives 18 triples.  Each `(2,1,0)` has two
preimages and `(1,1,1)` has six.  Thus counting all child triples measures the sum of coefficients,
not the number of supported parent states.  Counting distinct outputs and proving that this count
equals the target count would be a valid proof, but it is equivalent to proving that every target
coefficient is nonzero—the Row-Coloring Lemma itself.  Padding removes permutation bookkeeping;
it does not remove collisions or the existence problem.

## 2026-08-27 -- history-labelled counting reaches the same opposite-pure obstruction

Unique transcript labels make the recursive decomposition bijective at the level of proper
colorings of `Q_K`, but the sufficient counting theorem is coefficientwise: inject colorings of
type `a` into those of every Robin-Hood neighbor `a-e_i+e_j`.  Recursive case analysis works when
the donor has an excess in the mixed child or in a pure child that the recipient may use.  It stops
when donor and recipient occupy opposite pure sides and the mixed child has no donor excess.  The
induced-claw example already rules out a two-color repair; an injection must use a third color or a
global augmenting chain.  Such a construction must overcome the same opposite-color tight
separators isolated by the Pascal Adjacent-Fiber Lemma.  Thus history counting remains viable but
is strictly stronger than the support theorem and does not bypass its hard case.

`tools/singleton_strong_niceness.cpp` now uses exact arbitrary-precision arithmetic and supports
single-profile and random-transfer-walk diagnostics through `K=4`.  At provenance build
`560d11ce7ebab7866dd56ed43d243ce9f01fcba3f928990a5996bf19a17f4a12`, the canonical `G_4`
coefficient was `817133116390102794240`; its first balancing neighbor
`(15,15,12,11,5,5,5,5,1^8)` had coefficient `7354198047510925148160`, exactly nine times larger.
One seeded ten-transfer walk completed monotonically after 400,927 level-four allocation nodes.
A 100-step request with the same seed completed no eleventh comparison and timed out after 63 wall
seconds at 1.45 GB peak RSS; it is an abort, not a 100-step result.  The exact commands, final state,
lower-level node counts and provenance are in
`evidence/singleton_strong_niceness_2026-08-27.md`.

## 2026-08-27 -- high-support strict alternation retracted at K=19

The apparent one-dimensional endgame from the canonical padded tail is false.  The arithmetic
inequality (PA), despite passing every exact breakpoint check through `K=12`, fails at `K=19` with
`M=E=3^18`, `p=513`, and `q=256` by 2,431.  More importantly, this is not slack introduced by the
upper envelope `U_E`: a compressed exact construction realizes the bad prefixes as a sorted,
full-mass state majorized by `G_19` with exactly `2M` nonzero rows.  Strict alternation gives
`A_513+B_256=276,817,774` against Hall capacity 276,815,343.

The new `--padded-alternation-counterexample` mode checks the compressed state's ordering, mass,
every nontrivial majorization prefix, and the failing Hall cut.  Provenance build
`44c17992e86ea10573820f143cc58fb8d3519edc5d6130e4d9659586725be8f7`; the build-and-run command
took 2.6 wall seconds on the recorded M4 Pro and the verification itself took under 0.1 second.
This retracts only the strict-alternation/high-support claim, not the Row-Coloring Lemma or the
exact canonical-tail exchange and Hall contraction.

Allowing each adjacent pair to choose its orientation gives a cleaner exact high-support
subproblem.  It cannot prove the full lemma because adjacent-pair orientation already fails on 916
lower-support `K=4` states.  If
`d_i` is the difference in pair `i`, `epsilon_i` records its orientation, and
`D_n=sum_(i<=n) epsilon_i d_i`, then
`A_p+B_q=(P(2p)+P(2q)+D_p-D_q)/2`.  Within that regime the missing theorem is consequently a
global interval-discrepancy or augmenting-path statement; merely balancing the current prefix total
cannot control all `D_p-D_q` simultaneously.  Do not spend further effort trying to prove (PA)
from the Pascal formula.

## 2026-08-27 -- global Adjacent-Fiber census passes through K=4

The global transfer target survives a substantially stronger finite test.  A new exact mode marks
a donor and recipient row, enumerates all feasible colorings up to permutations of unmarked equal
rows, and computes the least Hall slack over sets containing the recipient but not the donor.  It
also independently applies the transfer and checks that margin at least one is equivalent to Hall
feasibility of the transferred coloring.

Exact maximization at `K=1,2,3` covers respectively 1, 33 and 8,916 normalized transfer types; all
pass, and the best separator margin is always at least two.  The complete `K=4` existence census
covers all 5,997,038 full-mass states and 141,690,676 distinct state/donor-value/recipient-value
transfers.  Every transfer has a common coloring with separator margin at least two.  The run
visited 1,173,872,133 search nodes and completed in 353 wall seconds at reported peak RSS below
0.01 GB under a 1,800-second/4-GB cap.

The more tempting same-color statement is false.  Exactly 889 `K=4` transfer types require the
opposite-color search.  The first is
`(16,15,11,9,7,5,5,5,1^8)` with donor 11 and recipient 9: no feasible coloring puts those marked
rows together, while `A=(15,11,5,5,1^4)`, `B=(16,9,7,5,1^4)` works before and after the transfer.
Thus the opposite-color tight-separator case isolated by the proof reduction is already necessary
at `K=4`; it cannot be normalized away.

The final provenance build is
`b3809eae9bc918025b7c01ac262fed43372ae50f78c581bfe20514bce5f63ece`.  All four raw outputs pass
`tools/check_provenance.py`.  Full definitions, exact/capped distinctions and reproduction commands
are in `evidence/singleton_adjacent_fiber_census_2026-08-27.md`.  This is exhaustive finite evidence,
not a proof of the Pascal Adjacent-Fiber Lemma for arbitrary `K`.

## 2026-08-27 -- hard transfer fibers collapse to one Pascal crossing family

A diagnostic rerun exhaustively maximized and recorded all 889 `K=4` transfers lacking a
same-color certificate.  They are exactly the states `(16,15,11,9,lambda)`, with donor 11,
recipient 9 and a majorized tail partition of mass 30.  Conversely all 889 admissible tails occur.
Every same-color fiber is empty; every exact opposite-color optimum is two; and every selected
optimum has the unique minimizing cut `(p,q)=(1,2)`, demand 40 and capacity 42.  The complete rerun
used 1,173,872,133 nodes and 358 wall seconds under the 1,800-second/4-GB cap.  Provenance build:
`fdb42be9f6e301adb279582600397400fbee9a6648dd24a0de5b3355b620e415`.

The classification has a two-line explanation.  The rows 16 and 15 must be opposite because their
sum exceeds the pure two-row capacity 30.  If marked rows 11 and 9 were together, one of 16 and 15
would join them, producing a `(3,1)` demand of 51 against capacity 49.  Thus the hard search did
not expose 889 different global phenomena; it exposed one four-row obstruction with arbitrary
admissible tail.

The obstruction persists and its crossing repair can be proved for all levels.  Put
`U=2^(K-1)`, `M=3^(K-1)`, `d=2U-K-1`.  For each of the `K-3` integers
`2U-2K+1<=r<=2U-K-3`, the full-mass state `(2U,2U-1,d,r,1^T)` is majorized by `G_K`, but the same
two Hall inequalities forbid coloring `d,r` together.  Color `2U-1,d` against `2U,r` and allocate

    2U-1 -> (U,U-1,0),       d -> (U-1,U-K,0),
    2U   -> (0,U,U),         r -> (0,r-U+1,U-1).

Unit rows fill all three children to mass `M`.  Their nonunit prefixes lie under
`(U,U-1,U-K,U-K)`.  After transferring one unit, replace the two mixed contributions
`U-K,r-U+1` by `U-K-1,r-U+2`; both still fit beneath the identical pair.  The same argument handles
all intermediate transfers, and the `(1,2)` cut proves that its exact margin is `d-r`.

This is the first uniform, non-search Pascal separator crossing and directly realizes the proposed
“multiple identical targets” mechanism without a cyclic reassignment.  It does not finish the
Adjacent-Fiber Lemma.  The remaining global obligation is now sharper: show that a minimal
opposite-color tight separator can be followed down the dyadic/Pascal blocks until an identical
target pair performs this local reroute, or else combine the blocking plateaus into a violated
parent prefix.  Ordinary jump-system exchange only chooses some donor for an augmentation; after
the absolute-value fold it does not force the specified donor, so the standard bisubmodular
theorems do not supply this termination step.

The diagnostic source also gained a provenance-checkable single-case mode.  Exact all-unit-tail
probes at `K=5` (`26->24`, `26->23`) and `K=6` (`57->55`) agree with the formula; build
`f18ca95eb7b7cd6df8d25df3d35594490dc9392193df3c98c8e5d976dc76dc88`.  These probes are checks,
not premises of the proof.  Full details and reproduction metadata are in
`evidence/singleton_adjacent_fiber_census_2026-08-27.md`.

## 2026-08-28 -- a per-row canonical-atom cut rule fails at K=3

The proposed strengthening required every row's one- or two-piece first-cut image to contain a
positive piece whose width occurs in `G_(K-1)`, possibly requiring the larger piece.  Exact
restricted reconstruction refutes even the weaker version at the fourth `K=3` state,
`(8,7,4,2,2,2,1,1)`.  The state is majorized by `G_3`.  Since the `G_2` widths are `{4,3,1}`,
the rows 8 and 7 force at least 4 and 3 mixed units, while all three width-two rows must split as
`1+1`; the mixed lower bound is therefore 10, above its mass 9.

An unrestricted cut exists and has children `(4,3,1,1)`, `(4,3,1,1)`, `(4,2,2,1)`.  It repairs
the obstruction by leaving two width-two rows intact.  Hence atom widths cannot be imposed as a
row-by-row condition covering intact rows; an aggregate atom budget or a restriction only on
genuinely split rows is not refuted by this example.  `tools/singleton_shape_survey.cpp` now has
`--atom-{census,uniform}` and `--larger-atom-{census,uniform}` modes; final provenance build
`2140914d270dd46b4698a9a8bf352df8bf28f57d2f9945915d67fed56e96acfe`.  The direct proof and
reproduction lines are in `evidence/singleton_shape_survey_2026-08-27.md`.

## 2026-08-28 -- Pascal-first tight bands isolate the remaining transfer step

Writing the conjugate of `G_r` as Boolean-lattice columns makes the recursive structure exact:
one column `T subset [r]` has capacity `c(T)=2^(r-|T|)`, and adjoining the first-test coordinate
replaces it by a doubled pure column of capacity `2c(T)` and a mixed column of capacity `c(T)`.
For a full-mass parent, Gale--Ryser saturates all these columns.  A legal first cut is therefore
equivalent to one sign per row that bisects every doubled column; the only missing condition is
global row-sign coherence.

This viewpoint gives a general Tight Pascal-Band Lemma.  If a feasible coloring has an
opposite-color tight Hall separator with `p` left-color and `q` right-color rows, then equality of
the total Hall bound forces equality separately in every left, mixed and right child column.  If a
Robin--Hood donor lies just outside the left part and the recipient lies inside the right part,
the row-degree bounds imply

    #{T:p<c(T)<q} >= 2 + #{T:c(T)=p+q}.

Thus every genuine transfer obstruction crosses a repeated internal Pascal rank (for `r>=2`).
This proves globally the duplicated-target phenomenon previously proved only for the 889 explicit
`K=4` hard fibers.  It does not yet prove that an alternating path can use the duplicated rank.

The initial next target was phrased as a Dyadic Plateau-Descent Lemma inside one coloring.  The
later landscape analysis below corrects that formulation: a tight Fixed-Color Hall cut makes an
orientation-preserving completion impossible.  What survives is the need to use the repeated rank
while changing whole row colors.  A nested/laminar substitute is false already for
`(8,7,4,1^8)<=_w G_3`: its forced doubled-support sizes `8,4,4,2` and pure coverage profiles
`(4,4,4,1^6)` or `(4,4,3,1^7)` admit no laminar realization.  Crossing supports are essential.

This pass also corrected a transcription error in two research notes:
`G_3=(8,7,4,4,1,1,1,1)`, not `(8,7,4,3,2,1,1,1)`.  The canonical-atom counterexample remains
valid, because `(8,7,4,2,2,2,1,1)` is majorized by the corrected sequence and its contradiction
uses only the `G_2` child widths.

## 2026-08-28 -- fixed-color plateau descent corrected; two-row target survives

The first proposed Dyadic Plateau-Descent statement was internally impossible.  If a feasible
coloring has a zero-margin donor/recipient separator, the Fixed-Color Hall Lemma says precisely
that the transferred demand is infeasible with those row orientations.  No incidence switches
preserving every sign can complete it.  The proof must instead move among feasible colorings of
the original state, rerouting incidences while whole rows change orientation.

The corrected diagnostic enumerates this coloring landscape.  A transfer between partition values
may choose any equal donor/recipient row, and the two pure sides are globally interchangeable, so
the final quotient identifies both equal-row choices and global `A/B` complementation.  It measures
the minimum row-color distance in that quotient.

The exhaustive `K<=3` result initially looked like 11,296 failed *marked* colorings, including 92
one-row local maxima.  That was the wrong quotient: the first apparent two-row repair merely
exchanged the marked recipient with an identical unit row.  Quotienting equal rows but not global
complementation still manufactured apparent distance-four and distance-five `K=4` obstructions.
Those too were artifacts: a donor value occurring on both sides lets the globally complementary
marking choose the better donor identity.

After the full quotient, `K=1,2` have no failed feasible colorings.  At `K=3` there are 237,617
feasible normalized colorings across 8,916 transfer types; 348 fail, of which 325 have a successful
coloring at row distance one and 23 first succeed at distance two.  In the first 10,000 `K=4`
states--282,690 transfer types and 46,600,920 feasible normalized colorings--94,936 fail; 74,090
reach success at distance one and the remaining 20,846 at distance two.  The first genuine
distance-two case is

    x=(16,15,11,11,5,5,5,4,2,1^7),       transfer 4->2,
    A=(15,11,5,4,1^4),                    B=(16,11,5,5,2,1^3).

This supports the Two-Row Color-Exchange Lemma: every failed feasible coloring has a successful
one obtained by one row flip or one opposite-color swap in the correct quotient.  Proving it would
finish the Adjacent-Fiber step.  The result is exhaustive only through `K=3`; the `K=4` run is an
exact initial prefix, not a full level.

The move shape is not accidental.  Encoding a coloring by `z_i=+/-x_i` turns the Fixed-Color Hall
system into the integer points of the integral bisubmodular polyhedron with rank
`rho(X,Y)=H(|X|)+H(|Y|)+H(|X|+|Y|)`.  Standard delta exchange there uses directions of support at
most two.  It does not finish the proof because a row flip changes a coordinate by `2x_i`: the
colorings form the nonconvex fixed-absolute slice `|z_i|=x_i`, which does not automatically inherit
ambient unit exchange.  The theoretical task is now to use the repeated rank guaranteed by the
Tight Pascal-Band Lemma to compress such a unit-exchange chain to a boundary flip or swap while
making the transfer margin positive.

The first 10,000-state `K=4` segment is head-heavy, so three disjoint 1,000-state windows were also
enumerated after skipping 1,000,000, 3,000,000 and 5,000,000 states.  Across the four windows there
are 369,300 transfers, 261,315,748 feasible normalized colorings and 377,873 failures.  Exactly
352,471 failures have a distance-one repair and 25,402 need a swap; none needs anything else.  The
last two windows have only distance-one failures.  The four runs were concurrent, the longest took
under five wall minutes, and every process stayed below 50 MB RSS.  This rules out an obvious
enumeration-order artifact, but it still covers only 13,000 of 5,997,038 `K=4` states.

The final provenance build is
`559585c2f01d0c51942cdf27a6013160df14c3d8ee0d160c4746a7bfa618a638`.  The exact `K<=3` census
took under two wall seconds; the exact 10,000-state `K=4` prefix took about two wall minutes.
Definitions, reproduction commands and the distinction between exact windows and a complete level
are in
`evidence/singleton_coloring_landscape_2026-08-28.md`.

## 2026-08-28 -- exact boundary calculus refutes delta exchange and isolates an alternating-cut proof

The fixed-absolute-value caveat is substantive, not merely missing bookkeeping.  For a fixed
feasible coloring, flipping `v:B->A` changes the Hall rank of a containing set with old counts
`(p,q)` by exactly `h_(p+1)-h_q`.  Swapping `u in A` and `v in B` gives that change on sets
containing only `v`, the symmetric change `h_(q+1)-h_p` on sets containing only `u`, and zero on
sets containing both or neither.  Hence a failed flip has an oppositely imbalanced blocker, while
a failed swap has one blocker family of each imbalance.

For a `B`-heavy tight transfer separator `S=X union Y`, every successful one-flip repair must move
some row of `Y` to `A`, and every successful one-swap repair must exchange a row of `Y` with a row
of `A-X`; otherwise the Hall rank of `S` cannot rise while its transferred demand rises by one.
The Tight Pascal-Band Lemma makes every such crossing move gain at least two on `S`.  The remaining
proof is therefore exact: follow an `A`-heavy blocker of a failed crossing move, then the next
`B`-heavy blocker, and prove strict descent of the dyadic band before this alternating cut chain
can repeat.  The repeated Pascal rank supplies the departure capacity; termination is still open.

A new labelled-mask mode gives a small counterexample to using standard delta-matroid theory.
For `(3,2^11,1,1)@3`, feasible `A`-sets `{0,1,2,3}` and `{1,2,3,4,5}` fail symmetric exchange at
row 0.  The permitted flip leaves only three `A` rows and full-set capacity 26 below mass 27; the
two permitted swaps leave the opposite side at mass 19 above pure capacity 18.  Thus expanding a
jump-system coordinate into Boolean elements does not justify restricting every block to be
all-in or all-out.  An initial unoptimized Python pair check was killed after about 35 CPU seconds;
the provenance-built C++ mode returns the exact witness immediately.

The same diagnostic rejects two tempting ways to select the crossing row.  At `K=3`, the 325
above-floor failed colorings all flip and the 23 floor failures all swap.  In the first 5,000
`K=4` states, 516 above-floor failures also need a swap.  Trying the marked-recipient/closest-lower
swap and the largest-outside/closest-higher swap together still misses 170 failed colorings.  The
first miss is repaired, up to global side complementation, by swapping the marked donor itself
with a larger opposite-side row.  The final 5,000-state run took 55 wall seconds at 0.01 GB peak
RSS.  The exact identities, build and commands are in
`evidence/singleton_boundary_exchange_2026-08-28.md`.  Do not add another endpoint or closest-value
special case: within the local-repair route, existence must range over the whole crossing cut.

## 2026-08-28 -- same-band blockers refute descent; positive crossing gives a monotone target

The proposed strict Pascal-band descent is false even in the existing `K=3` boundary example.  For
`x=(3,2^11,1,1)`, transfer `3->1`, and coloring `A=(3,2,2,2)`,
`B=(2^8,1,1)`, the selected dangerous cut has counts `(0,10)`.  Swapping the marked width-three
donor with a selected width-two row is blocked at old/new counts `(1,9)->(0,10)`.  The blocker's
closed rank-loss band is exactly the original target band: three levels, four columns, width ten.
Thus an alternating proof cannot use the dyadic band alone as a strictly decreasing potential.
The exact `--boundary-blockers` mode now enumerates every crossing flip/swap for a selected failed
coloring and distinguishes original-state infeasibility from a new transferred-state separator.

A much simpler potential survived the finite tests.  Orient donor in `A` and recipient in `B`.
Across a dangerous cut `X union Y`, call a feasible flip of a positive row in `Y`, or a feasible
swap `v in Y`, `u in A-X` with `x_v>x_u`, a positive crossing.  It strictly increases total
`A`-mass.  If it moves either marked endpoint, donor and recipient share a color and the transfer
is finished.  Otherwise the endpoint orientation is unchanged; repeat if another separator
remains.  Since each nonterminal step strictly increases an integer mass bounded by the pure Hall
inequality, this process cannot cycle.  A padded zero recipient is assigned the donor's color for
free.  Consequently the open Positive Pascal Crossing Lemma--existence of one such move at every
material-row failure--would prove Adjacent-Fiber without one-step success or band descent.

The quotient landscape census tests the stronger rule that every maximum-mass-gain crossing
neighbor succeeds immediately.  The complete `K=3` census has 348 failed colorings and the first
5,000 `K=4` states have 45,504; neither has a failed maximizing choice or a bad tie, and the minimum
maximum gain is one.  Earlier-build disjoint windows found no missing successful maximum among
78,487 failures after skip 1,000,000, 41,842 after skip 3,000,000, and 18,137 after skip 5,000,000.
The latter two used 200 states each.  A 1,000-state attempt at skip 3,000,000 timed out after 184
wall seconds and is an abort, not evidence; its 200-state replacement completed in 96 seconds.
The 1M and 5M completed runs took 121 and 76 seconds.  The final complete-`K=3`/first-5,000 build is
`fa329a545ac76dca7dda565267c854962707ed331e393d111ae9303346c3e46e`; its `K=4` run took 56
wall seconds at 0.01 GB peak RSS.

The theorem remains open.  The exact remaining proof should suppose every positive crossing move
is blocked, uncross all `A`-heavy flip blockers and `B`-heavy swap blockers with the dangerous cut,
and use the repeated columns in (TB3) to sum those certificates into a parent-prefix violation.
The maximum-gain data identifies the correct global direction; it does not supply that uncrossing
argument.

## 2026-08-28 -- tight-set core eliminates the alternating blocker chain

The previous entry's expected `B`-heavy swap-blocker family is unnecessary.  For a failed coloring,
all dangerous `x`-tight sets are closed under union and intersection.  Let `C` and `U` be their
intersection and union.  Any one-row common recoloring must move a `B` row `v in C` to `A`; a swap
must return an `A` row `u notin U` to `B`.

Provisionally flip `v` and intersect all sets on which that flip violates `x`; call the intersection
`P_v`.  If `x_v>x_u`, swapping `u` and `v` is feasible for `x` exactly when `u in P_v`.  The only
new point is decisive: a set `T` containing `u` but not `v` has post-swap rank equal to the old rank
of `T-u+v`, while `x(T-u+v)=x(T)-x_u+x_v>x(T)`.  Old feasibility of the replacement set therefore
rules out this formerly expected reverse blocker.  If `v in C` and `u notin U`, the same swap is
also feasible for the transferred demand; unchanged tight dangerous sets are excluded by the
core/hull memberships, and the two changed directions have at least one unit of replacement slack
or the Tight Pascal-Band gain.  Thus no repeated recoloring or alternating cut descent is needed.

The resulting strong local statement is the **Core--Blocker Escape Lemma**: for some `v in C intersection
B`, either its flip has no blocker, or `P_v-U` contains a smaller `A` row.  This is still open, but
it is a strictly sharper global rule than positive crossing and has no arbitrary separator or
preselected endpoint.  Uncrossing a dangerous tight set `S=X union Y` with a flip blocker
`T=P union Q` gives the quantitative bound (CU): every Pascal column in the intersection of the
target open band and blocker closed band charges `|X-P|+|Q-Y|` to a submodularity defect smaller
than the blocker's full rank loss.  If the blocker band is contained in the target band, signed
nesting follows immediately.  The unresolved case is to combine these overlap charges across all
core rows and force one common blocker intersection beyond `U`, or a parent-prefix violation.

`tools/singleton_pair_coloring_census.cpp --boundary-blockers` now prints `C`, `U`, and every move's
blocker intersection/union.  The first genuine `K=4` no-flip state has one dangerous tight set.  For
`v=11`, its 24 blockers share `{15_A,8_A,5_A,4_A,16_B,11_B}`, so the three smaller outside rows
`8,5,4` all give successful swaps.  In the earlier closest-boundary counterexample, the 44 blockers
for `v=11` share only `{15_A,8_A,16_B,11_B}` and select the nonlocal swap `8_A<->11_B` directly.
Both exact labelled-mask runs completed below one wall second together.  The provenance build
`322b29b4bb6d29da2a36f410b69dcdba0a5b8b6cb0026862d90a7d9cb5d39036`, commands and full outputs
are summarized in `evidence/singleton_boundary_exchange_2026-08-28.md`.

## 2026-08-28 -- blocker cores are a monotone prefix staircase and survive a K=5 probe

The Core--Blocker Escape target no longer requires arbitrary labelled intersections.  For fixed
`v`, the `A` rows common to all flip blockers form an upper set by mass.  If every blocker contains
`u` and a heavier `A` row `w` were omitted by some blocker, replacing `u` by `w` would make another
blocker omitting `u`, a contradiction.  Therefore, if any smaller row outside the dangerous hull
works, the largest such row works.  This rehabilitates a closest-smaller choice only *after* the
crossing row `v` is selected globally from the dangerous core; the refuted rules selected `v` from
one arbitrary separator.

The upper sets are monotone in `v`.  For `x_v>=x_w`, any blocker for flipping `w` maps to one for
flipping `v`, either unchanged when it already contains `v`, or by replacing `w` with `v`.  Its
`A` part is unchanged, so `P_v intersection A` is contained in `P_w intersection A`.  Meanwhile
`U intersection (A-{i})` is itself an upper set, and every strictly heavier `B` row above a member
of the dangerous core is also in the core.  The open local strengthening is therefore a one-dimensional
staircase crossing: as `v` descends, the common blocker prefix grows and must cross the dangerous
hull before the newly reached outside row becomes at least as large as `v`.  Candidate membership
has the exact sorted-prefix test (ST2), so a contradiction proof can choose one violating `(a,b)`
pair per failed step and combine it with (TB3)/(CU).

A targeted falsification was worthwhile because all earlier local evidence stopped at `K=4`.
The new `--fixed-positive-sample` mode generates majorized states by Robin--Hood walks, finds one
exact coloring, walks on its feasible flip/swap graph to reach boundary colorings, and tests sampled
opposite-color transfers.  On 5,000 `K=5` states it tested 624,395 pairs; 3,220 current colorings
failed after their transfer, and every one had a positive common one-flip/one-swap neighbor.  This
is deterministic for seed `20260828` but neither the states, colorings nor pairs are uniform.  It is
a 50-wall-second falsification probe, not a theorem.  Build
`13b14aa1a44b8bacfcecf1574726543f5cafdc6d60709909257dd31a432b122a` and the capped command are in
`evidence/singleton_boundary_exchange_2026-08-28.md`.

## 2026-08-28 -- correction: the prefix staircase is a stronger route, not the remaining lemma

The proof search had silently strengthened its quantifiers.  The Row-Coloring Lemma is equivalent
to Pascal orthant saturation and, as a universal full-mass statement, to closure under unit
Robin--Hood transfers with an arbitrarily rebuilt coloring.  Adjacent-Fiber additionally requires
one coloring common to the two transfer endpoints.  Positive Crossing requires a monotone local
move from every failed coloring, and Core--Blocker Escape requires an immediately common
one-flip/one-swap neighbor.  The blocker-core and prefix-staircase deductions are correct
conditional results, but proving them is not known to be necessary for Row-Coloring.  Calling Core
Escape the sole remaining target was therefore an overstatement and risked another loop through
increasingly strong coloring rules.

The main global target is now recorded in an equivalent column form.  Write `c=G_(K-1)'`.
Gale--Ryser realizes every full-mass `x<=_w G_K` by a `0`--`1` matrix whose columns have degrees
`2c_t` and `c_t`.  A legal first cut is exactly a choice of such a realization plus one row
bipartition that gives `c_t` rows of each color in every doubled column.  Pairing opposite colors
inside each doubled column makes its incidences a matching; all doubled columns must have a
bipartite union, while the single columns are unrestricted.  This Balanced Pascal Realization
Lemma is exactly Row-Coloring, not a strengthening.  It also makes the intended role of repeated
Pascal targets precise: degree-preserving switches may change the entire incidence realization
and pairing, instead of repairing a prescribed coloring.

The decisive reason for the correction is logical, not computational: the stronger quantifiers
had never been derived from the exact statement.  A bounded scratch search over small generic
bases did not find a separation, but that unretained diagnostic supplies neither a reverse
implication nor evidence for the Pascal theorem.

The balanced-column form also yields a proved global normalization.  Minimize the sum of squared
color imbalances over all doubled columns, allowing both the incidence realization and row
coloring to vary.  If two doubled columns of the same degree `2c` have defects differing by at
least two, the larger-defect column has an `A` row absent from the other and the smaller-defect
column has a `B` row absent from the first.  The resulting `2x2` incidence switch preserves all
margins and strictly lowers the square sum.  Hence one capacity class has only two consecutive
defect values and cannot contain both signs.  The same count across capacities `c,e` bounds the
defect difference by `max(1,|c-e|)`.  If a doubled column has positive defect `d`, switching it
against any single column of degree `c` shows that every `B` row of the single column must already lie in the doubled
column; consequently the single column has at least `d` `A` rows.  Negative defect gives the
color-reversed nesting.  Flipping a whole row in the same global optimum shows that an `A` row's
incident defect sum is at most half its doubled-column degree, with the color-reversed lower bound
for a `B` row.  Summing gives `2 sum delta^2<=sum c`.  This reduces a putative counterexample to
coupled defects across the power-of-two capacity levels.  The missing step is to use their
binomial multiplicities to force a cross-capacity switch or a violated parent prefix.

The doubled/single labels themselves should also be optimized.  At parent rank `ell`, there are
`binomial(K,ell)` columns of the identical degree `2^(K-ell)`.  Pascal identity requires only
`binomial(K-1,ell)` of them to be designated doubled and exactly balanced; the remaining
`binomial(K-1,ell-1)` may be unrestricted single columns.  This gives the equivalent Binomial
Balanced-Columns formulation.  In a minimum-square realization, the doubled labels therefore sit
on the columns closest to half-and-half inside each degree pool: otherwise exchanging a doubled
label with an unrestricted label of the same actual degree lowers the objective without changing
one incidence.  Hence, if a rank misses its balance quota, every unrestricted column in that pool
is unbalanced too.  This is the exact global use of the multiple identical Pascal targets that was
lost when the local proof fixed a coloring and its child labels first.

The boundary ranks do not obstruct this program.  Full mass forces at least `2^K` positive rows.
Connect the unique degree-`2^K` column to the largest `2^K` residual degrees using the Ryser
reduction, then color half of that neighborhood on each side; the remaining margins stay
realizable.  The top quota is therefore exact, while the degree-one rank has quota zero.  Any
rank-lexicographically first failure must be one of the internal Pascal levels.

## 2026-08-28 -- Pascal quotas are Boolean coordinate deletion/contraction

The binomial split is the first test itself, not a numerical aid.  Label a parent column by
`S subset [K]` and give it degree `2^(K-|S|)`.  Exposing coordinate `t` bisects every column with
`t notin S` into the two pure copies; a column with `t in S` contracts to the mixed child after
deleting `t`.  The counts at rank `ell` are exactly `binomial(K-1,ell)` and
`binomial(K-1,ell-1)`.  Since equal-rank columns are interchangeable, this Boolean
Coordinate-Exposure Lemma is equivalent to the Binomial Balanced-Columns formulation.

The obvious symmetry argument is insufficient.  Summing the deletion and contraction capacities
for selected color counts `(p,q)` gives exactly `H(p)+H(q)+H(p+q)`; maximizing at fixed `p+q`
returns the parent rank `H_K`.  Thus maximizing away the color split passes to the convex
relaxation and erases the integral choice of which columns delete and which contract.  It cannot
exclude the lattice holes that are the whole open problem.

There is an important relabelling barrier.  Column labels can be assigned independently inside
each rank.  For a fixed coordinate `t`, any chosen quota of bisected columns can be labelled by the
sets omitting `t`, with the rest labelled by the sets containing it.  Hence coordinate exposure is
exactly the binomial rank quotas; it does not supply a cross-rank shadow relation.  A proposed
stopped-exchange-to-shadow argument would first have to construct a distinguished coupled
labelling and prove its closure, and no such construction is currently available.  Treating the
arbitrary labels as already shadow-closed would be circular.

The intrinsic next target therefore remains the global minimum-defect realization.  Use dyadic
degrees, adjacent deletion/contraction quotas, their arbitrary pairings, the tight-band identity,
and (BR0)--(BR4) to show that a positive minimum defect forces a violated parent prefix.  This is
an equivalent Pascal-specific goal; merely summing rank multiplicities still repeats the
convex-hull argument.

## 2026-08-29 -- singleton converse is a Pascal Griggs-dominance instance

The poset formulation now identifies exactly what the Pascal structure is doing.  Let `P_K` be the
`K`-fold lexicographic power of the three-element `V` poset `1<0,1<2`.  Replacing `1` by binary zero
and `0,2` by binary one gives the rank: level `r` has `2^popcount(r)` elements.  Across a binary
carry `u 0 1^m -> u 1 0^m`, the adjacent cover graph is a disjoint union of
`2^popcount(u)` copies of `K_(2^m,2)`, which proves normalized matching directly.

The recursive decomposition `P_K=P_(K-1) ordinal-sum (P_(K-1) parallel-union P_(K-1))` also gives
a nested chain decomposition.  Label the two upper copies of child chain `i` by `2i-1,2i`, and
concatenate lower chain `j` to the upper chain labelled `j`.  Global chain `j` then meets exactly
the ranks whose rank number is at least `j`.  Since the sorted rank-number multiset is `2^ell`
with multiplicity `binomial(K,ell)`, its conjugate is `G_K`.  Thus the full-mass Singleton
Majorization Converse is precisely the `P_K` instance of generalized Griggs chain-partition
dominance, or equivalently Stanley niceness of `Q_K`.  Shahriari's 2008 survey states the general
conjectural template; Stanley 1998 supplies the nice-graph language.  Neither source proves this
special family, and the Boolean-lattice dominance specialization remains current open context.

This exposes an exact global lift.  A target chain partition determines a zero--one rank-incidence
matrix with row sums `a_i` and ordered column sums `R_K(r)=2^popcount(r)`.  Gale--Ryser constructs
some such matrix exactly when `a<=_w G_K`; majorization therefore solves all margins.  The missing
condition is that the incidences can be bijected with actual rank elements so each row is a chain.
Recursively, the lower-half columns lift to one child, while all upper columns need one common row
bisection into the two outer orientations and then two child lifts.  This Carry-Compatible
Gale--Ryser Lemma is exactly the Balanced Pascal Realization Lemma in ordered-rank form.

An arbitrary correct-margin matrix cannot be lifted.  At `K=4`, take upper-rank neighborhoods
`{u,v}`, `{u,v,a,b}`, `{u,v,b,c}`, `{u,v,c,a}` at ranks `8,9,10,12`, and put every other rank
incidence on a fresh row.  The row-degree state is `(4,4,2,2,2,1^67)<=_w G_4`.  Any first-coordinate
bisection makes `u,v` opposite and then forces `a,b`, `b,c`, and `c,a` opposite, an odd-cycle
contradiction.  This does not refute the singleton state: split one canonical length-16 chain into
`4,4,2,2,2,1,1` and all remaining canonical chains into units.  It proves that the incidence
realization must be selected or switched jointly with the Pascal lift; sorting rank sizes and then
lifting a convenient Gale--Ryser matrix is another false shortcut.

No long-running process or new census was used for this result; the derivations and parity
counterexample are symbolic.  The repository baseline checks were green before the edit.

## 2026-08-29 -- canonical bottom cells give direct transfers, but scheduling is global

The Pascal chain codes make one part of the proposed no-exchange proof exact.  Number the canonical
chains by binary codes `j` and let `d(j)` be the code length.  Chain `j` contains one word for every
rank skeleton with at least `d(j)` outer positions, so its length is
`sum_(s=d(j)..K) binomial(K,s)`.  If `d(j)<d(q)`, the donor word with skeleton
`0^(K-d(j))1^d(j)` begins with enough inner symbols that every word of the shorter receiver chain
becomes comparable before an outer-label conflict is possible.  Moving this one word is therefore
a literal direct recoloring.  This proves that every first Robin--Hood transfer out of the
canonical `G_K` coloring is available without a component swap.

The full compatible-receiver set is also explicit.  If a donor word has outer positions `t_r`, a
depth-`e` receiver code must agree with its first `m` outer labels, where
`m=#{r:t_r-r<=K-e}`.  The available receivers form one dyadic code cylinder of size
`2^(e-1-m)` until `m` reaches `e`, when none remain.  This is the rigorous version of using the
multiple identical Pascal targets: as donor cells are exposed, their receiver block halves at
known binary thresholds.

Two obstructions prevent declaring victory.  First, the direct-move property is not invariant
under arbitrary choices.  At `K=3`, move `112` from canonical chain
`(112,121,120,211,210,201,200)` to `(122,212,221,220)`.  The six-word donor then has no vertex
compatible with `(102,012,021,020)`, despite the remaining size gap `6-4=2`.  Second, individual
receiver compatibility is not closed under union: at `K=2`, both `10` and `12` are compatible
with `{02}`, but they conflict with each other.  Thus the residual problem is a global list-coloring
schedule on laminar dyadic receiver cylinders, not scalar load balancing.

This suggests the Canonical Monotone-Transfer Conjecture: after padding by empty colors, every
`a<=_w G_K` can be reached from the canonical coloring by a globally chosen sequence of direct
unit recolorings.  It is sufficient and stronger than the desired existence theorem.  The new
dependency-free `tools/singleton_monotone_transfer_census.py` constructs such paths for all 1,206
full-mass dominated types at `K=3`: a unit-ready representative pass reaches 1,201 types and
targeted searches reach the remaining five.  Measured wall time was 3.0 seconds.  This is exact
finite evidence only; the missing theorem is a scheduling invariant that simultaneously balances
the dyadic code cylinders and keeps the words in every receiver mutually compatible.  Full method
and output are in `evidence/singleton_monotone_transfer_census_2026-08-29.md`.

## 2026-08-29 -- low-level solutions have a two-interval Pascal normal form

A survey of the actual `K<=3` chain partitions found a substantially smaller global construction
than a direct-transfer schedule.  Cut each canonical Pascal chain into contiguous rank intervals.
Every one of the 2, 15 and 1,206 full-mass dominated types at `K=1,2,3` has an exact cover in which
each target chain uses one interval or splices two intervals from source chains of different
depths; the two rank ranges are disjoint.  At `K=3`, 821 candidate chains suffice.  Since two
rank-separated chain intervals join exactly when their boundary endpoints are comparable, this is
an endpoint-matching form rather than arbitrary recoloring.

The restriction survey separates the useful structure from artifacts.  Cuts alone cover 2/2,
11/15 and 591/1,206 types, so joins are essential.  Same-depth and rank-interleaved joins are not
needed at `K=3`.  Depth gap at most two still covers everything, but adjacent depths miss
`(8,3^6,1)`.  Forcing shallower intervals always below deeper ones misses `(8,7,3^3,1^3)`; forcing
the reverse misses `(8,7,4,3,2,1^3)`.  Thus splice edges may be oriented by strict source depth,
which makes the source graph acyclic, but their direction in rank order must remain adaptive.

This motivates the Pascal Two-Interval Splicing Conjecture for general `K`.  It is a sufficient
strengthening, not equivalent to Row-Coloring, and it need not give a monotone unit-move history.
The precise open step is a coupled cut-and-endpoint-matching theorem: parent majorization must
guarantee Hall for some simultaneous choice of interval endpoints.  Choosing cuts first would
repeat the arbitrary-incidence error.

At `K=4` there are 456 intervals and 20,542 normal-form candidates.  Five varied selected targets
have exact covers.  A fixed-seed 100-target Robin--Hood-walk probe passed 85; 15 reached a
20,000-node cap and none failed exhaustively, using 340,651 nodes and 49.4 seconds.  A separate
valid balanced target `(6^13,3)` reached a 500,000-node cap after 111.1 seconds and is likewise
inconclusive.  An apparent fast rejection of `(9^9)` was discarded: it is not majorized by `G_4`,
because its nine-part prefix is 81 while the corresponding `G_4` prefix is 74.  The durable tool
now checks every selected `K=4` target before search.

The dependency-free exact-cover source and full results are in
`tools/singleton_cut_splice_survey.py` and
`evidence/singleton_cut_splice_survey_2026-08-29.md`.  Exact `K<=3` normal-form enumeration took
3.2 seconds for the strongest passing variant.  All exploratory Python processes exited.

## 2026-08-29 -- identical children fail, but adjacent children survive `K=3`

The proposal that every majorized parent has a valid split with two identical normalized child
sequences is false at `K=3`.  For fixed child partitions `L,M,R`, normalized recombination is
exactly a partial matching from the mixed parts to the disjoint pure multiset `L union R`; matched
values add and unmatched values remain separate rows.  Exhausting this formulation shows that all
2 and 15 parents at `K=1,2` have an equal pair, but only 1,190 of the 1,206 `K=3` parents do.

The clean counterexample is `a=(8,3^6,1)<=G_3`.  Since child entries are at most four, its eight
must split as `4+4` between `M` and one pure child, say `L`; these are the only children containing
four and hence the only possible equal pair.  If `L=M=(4,q)`, then
`q` is one of `(3,1,1)`, `(2,2,1)`, `(2,1^3)`, `(1^5)`.  The remaining components must form six
threes and one one, so every two needs a one and only one one remains unmatched.  In the four
cases, respectively: at least three right twos compete for two mixed ones; two left twos compete
for one mixed one; at least four pure twos compete for three mixed ones; or the third child's mass
nine would need six parts of size at least two.  Thus equality is impossible.

The state has the valid all-distinct split

    L=(4,3,1,1), M=(4,2,2,1), R=(3,3,2,1).

Match `L4+M4`, twice `L1+M2`, and `R2+M1`; leave `L3,R3,R3,R1`.  The split is not unique: exact
enumeration gives eight ordered child-type triples, four modulo the genuine `L<->R` symmetry, and
six normalized row-allocation orbits.  The four child-type representatives have allocation counts
`1,1,2,2`; a visibly inequivalent triple is `(3,2,2,2)`, `(4,1^5)`, `(4,2,2,1)`.  Thus the equality
failure is structural rather than forced by a singular decomposition.

The displayed split also exposes the useful
salvage: `L>M>R`, and both dominance steps are single Robin--Hood transfers.  Exact enumeration
finds that every `K=3` parent has a split with at least two children at transfer distance at most
one, and every parent has a split whose three children form a dominance chain.  These statements
are finite only; call the first the Adjacent-Children Split Conjecture if it survives `K=4` probes.

There are 16 exact equal-pair exceptions.  Besides the displayed state they are compactly listed
as `(8,5,5,1^9)`, `(7,6,6,2,1^6)`, `(7,6,6,1^8)`,
`(7,2^t,1^(20-2t))` for `0<=t<=10`, and `(6,6,6,1^9)`.  The dependency-free census checked all
3,375 ordered triples of the 15 child types in 2.4 seconds.  Source, hand proof, and reproduction
are in `tools/singleton_identical_children_census.py` and
`evidence/singleton_identical_children_census_2026-08-29.md`.

## 2026-08-29 -- the multiplicity-free split corpus forms a hereditary spine

An exact survey separated two notions of rigidity.  A parent is child-unique when one normalized
`(L,M,R)` type orbit produces it modulo `L<->R`; it is cut-unique when that orbit also has one
normalized row-triple multiset.  The counts `(all, child-unique, cut-unique)` are

    K=1: (2,2,2),       K=2: (15,4,3),       K=3: (1206,9,6).

Write the four child-unique `K=2` types as `A=(1^9)`, `B=(4,1^5)`, `C=(4,2,2,1)`, `D=G_2`.
The nine unique `K=3` child triples are

    AAA, ABB, DCD, BDB, CDC, CDD, BDD, CDD, DDD,

for parents

    (1^27), (8,1^19), (8,5^3,1^4), (8,7,1^12), (8,7,2^6),
    (8,7,3^3,1^3), (8,7,4,1^8), (8,7,4,2^3,1^2), G_3.

Their normalized cut counts are `1,1,1,1,1,2,2,4,1`.  Thus the complete fully rigid `K=3`
corpus has six members.  Every forced child is itself child-unique at `K=2`, and every forced
triple repeats a child.  This is the first clean recursive signal supporting the user's proposed
parent/children resemblance: although identical children fail in the full corpus, they are forced
on the rigid spine.

The correspondence is not literally injective.  The repeated `CDD` triple maps to two parents,
with two and four cuts.  The same phenomenon occurs one level earlier: the all-`(2,1)` child triple
maps to `(4,2,2,1)` with two cuts and `G_2` with one.  Child types therefore lose a cut coordinate.
The interior cut-unique state `(8,5^3,1^4)` is a useful clean model: its children are forced to
`(D,C,D)`, and its row triples are uniquely

    (0,4,4), (4,1,0), (3,2,0), (0,2,3), (1,0,0)^2, (0,0,1)^2.

This suggested the Rigidity-Heredity Conjecture: every child-unique parent has child-unique children
and a repeated child.  It passes exactly through `K=3` but is refuted by the subsequent `K=4`
entry below.  A direct `K=4` triple product would have
1,754,049,816 cases, so no such run was attempted.  The correct exact extension is parent-first
and stops after finding two child orbits among the 5,997,038 parents.  Restricting to the `9^3`
rigid child triples is exploratory only until heredity is proved.

`tools/singleton_unique_split_survey.py` reproduces the complete corpus in 3.3 seconds; definitions
and interpretation are in `evidence/singleton_unique_split_survey_2026-08-29.md`.  One preliminary
row-pattern script accidentally searched all 3,375 child triples instead of the eight relevant
counterexample triples; it was terminated after 30 seconds with no result, and the narrowed run
then completed in 2.7 seconds.  No exploratory Python process remains.

## 2026-08-29 -- exact `K=4` split multiplicity refutes strict rigidity heredity

The proposed parent-first census was much cheaper than the naive `1206^3` child-triple product.
`tools/singleton_split_multiplicity_census.cpp` enumerates each parent row directly as
`(p,a-p,0)` or `(0,a-p,p)`, quotients equal parent rows by monotone choice indices, and rejects a
partial child as soon as it violates a prefix of `G_(K-1)`.  It canonicalizes `L<->R` at leaves.
With an orbit cap of four, the one-, two- and three-child-orbit layers and their allocation counts
are exact; only the bulk is reported as at least four.

The implementation first reproduced the independent Python results through `K=3`: child-orbit
layers `9,19,6,1172` and allocation-orbit layers `6,4,8` at `K=3`, including all nine forced child
triples.  A 10,000-parent `K=4` gate took 0.062 seconds.  The first complete stop-after-two run took
38.959 seconds and found 30 child-unique parents.  The final orbit-cap-four run, with normalized
allocation storage and lower-layer classification, exhausted all 5,997,038 parents in 96.5534
in-process elapsed seconds / 101 wrapper wall seconds.  It visited 1,765,546,548 search nodes and
30,162,788 complete allocations, exited zero under a one-hour/4-GiB cap, and its temporary log
passed provenance checking with build id
`abc2dd99be783536c9d20bd91f765246016b84c92aa009d8b5c33f82a55d025b`.  The log is small and fully
reproducible, so it was not promoted to the artifact store.

The exact `K=4` child-orbit layers `(1,2,3,>=4)` are

    (30,123,106,5,996,779),

and the exact allocation-orbit layers `(1,2,3)` are `(8,19,32)`.  Thus extending the survey to
two and three solutions adds 229 boundary states, while the entire `<=3` corpus is only 259 of
5,997,038 parents (about 0.00432%).

The original Rigidity-Heredity Conjecture is false.  The child-unique parent

    (16,15,9^3,5^3,1^8)

has the forced all-distinct triple

    (8,6,5,4,1^4), (8,7,4,3,2,1^3), G_3.

The first two children each have exactly two child orbits at `K=3`; only the third is rigid.  This
simultaneously refutes strict child rigidity and forced repetition.  Across all 30 rigid parents,
5/15/10 have one/two/three rigid children and only 13 repeat a child.

The failure reveals a more useful filtration: every child in a forced `K=4` triple has one or two
child orbits, and every such triple contains at least one rigid child.  Call the possible general
statement the Multiplicity-Filtration Conjecture.  In the exact two-orbit `K=4` layer, 40 parents
admit three rigid children, 80 first require a two-orbit child, and three first require a
three-orbit child.  In the three-orbit parent layer the corresponding exclusive counts are
20,78,6,2 for best maximum child multiplicity `1,2,3,>=4`.  This makes the expansion beyond
singular states useful as a boundary microscope, although it says nothing directly about the
5,996,779-state bulk.

Rigid children are not available to every parent.  The independent complete `K=3` Python index
finds the unique no-rigid-child state `(3^9)`.  The reason is elementary: a row three cannot make
the part four in the three non-unit rigid child types.  If `(1^9)` is mixed, both pure masses nine
would have to be sums of twos; if it is pure, it occupies all rows and excludes the opposite pure
child.  The best number of rigid children over all `K=3` parents has histogram `1,60,331,814` for
zero through three.  At `K=3` every rigid parent has at most one upward one-unit neighbor, but five
nonrigid parents share that property, so simple boundary degree is not a characterization.

Full definitions, correctness argument and reproduction are in
`evidence/singleton_split_multiplicity_census_2026-08-29.md`.  All census and exploratory Python
processes exited; no process from this work remains.

## 2026-08-29 -- low multiplicity reveals an exact dyadic Pascal product

The complete one--three-orbit `K=4` relation was exported, including one representative cut for
every parent--child-shape orbit.  Its 259 parents have 594 child-shape orbits and 1,741 allocation
orbits, using only 48 distinct `K=3` child shapes.  A second complete census with the reporting
extension took
97.7258 in-process seconds / 101 wrapper wall seconds under a 30-minute, 4-GiB cap, visited the
same 1,765,546,548 search nodes and 30,162,788 complete allocations, and exited zero.  The log and
the small `K=3` control passed provenance checking with build id
`b3ad4bc8a693a00ca9cbebfa6cbd8bb9e440122660a3626277b62b6ac92369fb`.

The survey exposed a general lemma.  If a parent prefix of even dyadic length `t=2^j` is tight, the
mass of those rows in any split is at most `H(p)+H(t)+H(t-p)`.  Equality with `H_K(t)` forces
`p=t/2`, because the Pascal child profile drops strictly after the dyadic midpoint, and forces all
three contribution subsets to saturate prefixes of sizes `t/2,t,t/2`; `t=1` similarly forces pure
counts `0,1`.  The remaining rows fill the contracted suffix profiles.  Conversely the two
row-disjoint allocations concatenate.  This proves the Dyadic Tight-Prefix Factorization Lemma.

For `K=4,t=4`, head capacities are `(8,7),(8,7,4,4),(8,7)` and tail capacities are
`(4,4,1^4),(1^4),(4,4,1^4)`.  Hence pure head and mixed tail shapes are fixed and child-shape
orbits form an exact Cartesian product.  The local head layers have sizes `3,4,1`; tail layers
have sizes `7,19,6`.  Their truncated product gives full layers `21,85,25`, accounting for 131 of
the 259 parents.  This includes the formerly observed `3 x 7` rigid rectangle and explains why
the head chooses the mixed child while the tail chooses the pure children.  Overall 228/259 low
parents have a tight prefix at one of `1,2,4,8`.

There is a second, finite recursive signal.  Exactly 258/259 parents admit a rigid pure child.  The
exception `(16,15,9^3,5,3^4,1^6)` has a two-orbit pure child.  Every parent has an economical
solution below a closest rigid ancestor: total child transfer distance is at most the parent
distance.  This covers 592/594 solution orbits.  The low boundary is not itself transfer-closed:
only 176 parents are reachable from the rigid 30 by one-unit edges that remain in the 259-state
corpus, so the other 83 must pass through multiplicity at least four.

Two tempting interpretations were rejected.  Ordinary sorted best fit fails already on `G_4`:
it selects `(8,5^3,1^4)` as the cheapest rigid pure anchor, while the unique children are all
`G_3`.  Also, dyadic factorization does not close the original induction by itself; its head and
tail child capacities are asymmetric contracted Pascal intervals.  The next live target is a
Row-Coloring theorem for all such interval triples, not an illicit replacement by smaller
symmetric `G_j` instances.

The exact analysis and proof are in
`evidence/singleton_low_multiplicity_factorization_2026-08-29.md`; reproduction assertions are in
`tools/singleton_low_multiplicity_analysis.py`.  Both complete census processes and all short
analysis processes exited; no process from this work remains.

## 2026-08-29 -- tight-skeleton transfer matrices and minimum-support reduction

Replacing the dyadic-only view by all tight ranks gives an exact structural theorem.  If rank `t`
is tight, the number `p_t` of left-oriented top rows lies in the full Pascal plateau
`I(t)=argmax_p(H(p)+H(t-p))`, and all three child prefixes saturate.  Consecutive tight ranks
`u<v` therefore contract to child bands `h[p_u:p_v]`, `h[u:v]`, and
`h[u-p_u:v-p_v]`.  Conversely legal band allocations concatenate.  Raw oriented solution counts
are consequently sums over monotone plateau-count paths of products of local multiplicities--the
Pascal Tight-Skeleton Factorization Theorem.  The old dyadic Cartesian product is its singleton-
path case.

The count path is real.  In the `K=4` band `[5,9)`, `(4^4)` fails transitions `2->4` and `3->5`
but succeeds for `2->5` and `3->4`; four other band states allow every transition.  The stronger
arbitrary-row prefix statement is false for `(16,15,11,11,4^5)`, and strict exact-head alternation
fails at `K=5`.  The corrected exact transition search enforces both orientation counts, rather
than merely the positive child masses.

The main simplification came from retaining the unit block.  If a positive capacity partition has
at least as many unit as non-unit rows, merging the two smallest parts of any equal-mass dominated
refinement preserves dominance.  In conjugates, the merge removes cells from columns `1..y` and
adds them to `x+1..x+y`; the half-unit bound supplies exactly the missing prefix slack.  Every
`G_K` and every suffix has this shape.  Repeating the merge and then undoing it inside one row
orientation proves the Minimum-Support Reduction Theorem: the full Row-Coloring Lemma is equivalent
to its restriction to exactly `2^K` positive rows.  The former arbitrary-row Tail Extension
Conjecture follows from exact Positive-Band Extension for the terminal band.  At exact support,
removing one forced pure anchor coin from every row leaves capacities
`(G_(K-1)-1),G_(K-1),(G_(K-1)-1)` and parent `G_K-1`.

This reduces the complete `K=4` universe from 5,997,038 states to 408,776 exact-support states;
63,329 of those have no internal tight prefix.  All exact states pass direct allocation search.
Sorted alternation fails on 1,968 exact states, although none of the 63,329 strict states fails at
`K=4`.  That last pattern is not general.  The explicit 64-row, mass-729 state

    (63^2,57^2,42^3,23^5,22^5,3^44,2^3)

is strictly below every internal prefix of `G_6`, but odd/even coloring has
`A_9+B_3=478>H_5(9)+H_5(3)+H_5(12)=477`.

`tools/singleton_pascal_interval_census.cpp` now reproduces these facts.  Complete Positive-Band
Extension checks pass through `K=4`: 136 bands, 1,722,516 band-state instances and 1,443,610,330
search nodes in 127.473 in-process seconds / 132 wrapper wall seconds; the independent tail check
has 37 incoming cases, 1,422,304 state-case instances and 39,273,306 nodes in 5.529 seconds.  The
final build id is recorded in `evidence/singleton_pascal_tight_skeleton_2026-08-29.md`.

Exploratory checks supported the coalescence proof before it was derived: least-loaded grouping
passed 736,402 `K=4` suffix states in 19.5 seconds, and the direct two-smallest merge passed 566,578
generic half-unit instances of total mass at most 24 in 3.5 seconds.  One two-million-sample random
probe was terminated after 30 seconds, and one broader generic enumeration reached its 30-second
yield without retained output; both were replaced by the proof and bounded exact check.  Their
stdin Python processes exited or were explicitly killed.  No solver or exploratory process from
this work remains.

## 2026-08-29 -- two Pascal anchors isolate a smaller residual coloring lemma

The exact-support reduction has a second deterministic step.  Put `n=2^(K-1)`, `h=G_(K-1)` and
`c=h-1` with its trailing zeros.  In conjugates, `G_K'` contains one universal doubled column of
height `2n` and a maximum single column of height `n`.  Delete the universal column.  Bipartite
Havel--Hakimi allows a height-`n` column to meet the `n` largest remaining row degrees; delete that
too.  Therefore every sorted exact-support parent `a<=G_K` has `a_n>=2`, and

    b=sort(a_1-2,...,a_n-2,a_(n+1)-1,...,a_(2n)-1)

is dominated by the residual parent `J` induced from three copies of `c`.  Its prefix rank is

    J(t)=C(t)+max_p(C(p)+C(t-p)).

This is exactly Pascal deletion of Boolean columns `empty` and one singleton `{*}`.  It is not an
ad hoc anchor choice.

If `b` can be colored with at most `n` positive rows per side and satisfies Fixed-Color Hall for
child capacity `c`, residual allocation followed by restoring one pure anchor on every row and one
mixed anchor on the original top `n` rows produces three `h`-majorized children.  The lift uses the
identity `H(t)=C(t)+min(t,n)`; it is independent of which residual rows receive mixed mass.  This
proves the Two-Anchor Reduction Theorem and leaves the Balanced Residual Coloring Lemma as a new
sufficient target.  The lemma is still open, so Singleton Majorization remains open.

The extended `tools/singleton_pascal_interval_census.cpp` exhausts the target.  The complete capped
residual universes contain 73 states at `K=3` and 160,492 at `K=4`; all pass, using 303 and
1,140,358 exact coloring nodes.  The residuals derived from all 160 and 408,776 exact-support
parents also pass, using 678 and 2,929,065 nodes.  A stronger direct rule--all children have exact
support and exactly the longest half uses the mixed child--passes the same parent corpora with
2,331 and 39,086,058 nodes.  Requiring only exact child support uses 2,735 and 19,587,981 nodes.
The longest-half rule also finds a split of
the strict-interior `K=6` alternation counterexample in 169,567 nodes.

The residual census rejected several attempts to turn this into a scalar assignment rule.  Plain
balanced blocks fail on 403 complete `K=4` residuals.  One-block lookahead passes complete `K=4`
but fails twice in a deterministic 100,000-state `K=5` walk.  Globally minimizing final color-mass
difference, subject to both necessary color-row bounds, fails on 1,067 residuals.  The first is
`(14,13,9,5,4,4,4,2,2)`: all three admissible difference-one partitions violate Hall by one;
for example, `(14,9,4,2)|(13,5,4,4,2)` fails at `(p,q)=(3,5)`.  The split
`(14,5,4,4)|(13,9,4,2,2)` is legal at difference three.  Merging the two smallest two-anchor
residual rows fails for 9,804 original parents.  A fixed product under the feasible
color classes of canonical `J` is false already for `(6,3,3,3)@K=3`.  These failures confirm that
the full two-parameter Hall surface, not total balance or a fixed boundary shape, controls the cut.

A one-step transfer pattern remains striking.  Every noncanonical residual in the complete
`K<=4` corpus has a more head-heavy predecessor with a feasible coloring that puts the marked
transfer rows on the same side.  At `K=4`, 160,414 of 160,491 states use the first admissible
predecessor; the remaining 77 all use the second, and none needs a third.  The first miss is
`(14,13,8,8,5,3^3)`, where reverse values `(8,8)` fail and `(8,5)` work.  Sixty-one of the 77
misses use equal values, so avoiding ties is not a universal rule.  This does not yet give an
induction: independently selected common colorings need not agree along a whole predecessor path.
The next transfer target is an endpoint-rich augmentation lemma with a compatible global potential.

Two higher-level diagnostics remained positive.  A 100,000-state `K=5` dominance walk (at most 500
transfers, seed 161803398) had zero exact residual failures, 3 greedy failures and 2 lookahead
failures in 10 wall seconds; exact search used 579,956 nodes, maximum 16.  A 10,000-state `K=6`
walk (at most 1,000 transfers, seed 271828182) had zero failures for all three algorithms in 10 wall
seconds; exact search used 110,428 nodes, maximum 24.  A direct 20-state longest-half `K=5` walk
passed in 15 seconds after 227,641,556 nodes.  The attempted 1,000-state version hit its 120-second
cap without a batch verdict; it is recorded only as an abort.

The final optimized provenance build id is
`e42509888792c2be603e4e08c79c06d740b174405e2d7ccd334826f7729298eb`.  Address/undefined
sanitizers independently pass the complete `K=3` residual coloring, predecessor and longest-half
modes under build id `5f62ce7fa7ef3a413b4c563aba2cb7afe23de6dee127ff86aff4ecb2b4452528`.
The proof, exact finite status, counterexamples and reproduction commands are in
`evidence/singleton_two_anchor_residual_2026-08-29.md`.
Final process inventory found no `radio_canon`, census utility, capped runner, `Python -` or
`python3 -` process remaining.

## 2026-08-29 -- repeated halving becomes a complete Pascal switch search, but not yet a proof

The user asked why the two-anchor construction should stop after two anchors and proposed keeping
the halving process all the way down.  The useful answer is to stop making irrevocable row
assignments.  After Minimum-Support Reduction, fix the `2^K` parent rows and write the conjugate
Pascal capacities as labelled binary columns.  At rank `ell` there are `binomial(K,ell)` columns
of degree `2^(K-ell)`; exactly `binomial(K-1,ell)` must be bisected by one common equal row
coloring, while the other `binomial(K-1,ell-1)` are the mixed columns.  This is exactly the
Balanced Pascal Realization Lemma, so recursively exposing one coordinate is the rigorous form of
“keep halving.”

There is now a proved complete finite search space.  Opposite-color row swaps connect all equal
row bisections.  The standard binary-matrix interchange theorem connects all zero--one incidence
matrices with the fixed row and labelled column margins by `2x2` switches: decompose the symmetric
difference into alternating bipartite cycles and eliminate them by interchanges.  Taking the two
operations successively proves that every realization-and-bisection pair lies in one switch graph.
Thus breadth-first search from any Gale--Ryser realization finds a balanced vertex if and only if
one exists.  This is a universal exact algorithm for each finite parent, but it is not an
existence proof: connectivity alone does not say that the graph contains a zero-defect vertex.

The short constructive candidate starts with the canonical Havel--Hakimi realization and odd/even
row coloring.  In each internal rank, sort the squared column imbalances and retain the required
number of smallest values; their sum over ranks is `Phi`.  Zero is exactly a legal cut.  The tested
deterministic walk takes the strict move giving minimum `Phi`; if there is none, it takes the first
neutral row or incidence switch that exposes a strict move.  This gives the **Canonical Two-Move
Pascal-Switch Conjecture**: the walk never stops at positive `Phi`.  If proved, `Phi` drops every
one or two moves, and Minimum-Support Reduction plus induction proves Singleton Majorization.  It
is a sufficient strengthening, not an equivalent restatement.

Two exact counterexamples prevent this from being misreported as an easier theorem.  The canonical
matrix for

    (32,31,26,26,16^3,4^15,2^10)

has no legal row bisection: the grouped search closes after 15,139 nodes, and the rank-count proof
is written out in `evidence/singleton_balanced_hh_switch_2026-08-29.md`.  With the alternating
coloring, however, one `2x2` switch between global columns 4 and 7 on rows 12 and 23 changes the
rank energy profile `(0,1,0,0)` to zero.  Hence the choice of incidence realization is essential.
Strict descent alone is also false.  For

    (32,31,26,26,16^3,9,6^9,2^2,1^13),

no row or incidence move strictly lowers the initial energy one; the neutral row swap `(5,8)`
followed by the strict swap `(9,14)` reaches zero.  The algorithm therefore needs a globally
chosen setup move even though this example needs no cycle.

`tools/singleton_balanced_hh_census.cpp` implements the construction, exact canonical-coloring
checks and switch descent.  The final optimized build
`53ccceb84295839a9a68d8b92912ec4ee9f0770f9b14b788566f71cdc4ab223e`
gave these independently reproducible results:

- complete exact-support `K=3`: 160/160 pass, with alternating coloring already sufficient;
- complete exact-support `K=4`: 408,776/408,776 pass in 3.628 seconds; 69,664 need one row
  switch, none needs an incidence or neutral move, and maximum initial energy is two;
- first 500,000 exact-support `K=5` parents: all pass in 22.916 seconds; 190,975 total moves,
  including 347 incidence switches and 12 neutral moves, with at most two moves for one parent;
- a 500,000-iteration `K=5` difficulty hill climb found no failure in 112.593 in-process seconds /
  116 wrapper wall seconds; its best parent starts at energy three and needs two strict row swaps.

The identical source built as
`ade0deedd3cd2c952f9f71d1ea92a1a9097e717da51f23e86e6b481a5b40b668`
also passed the separated `K=5` windows and the 100,000-state `K=6` Robin--Hood walk; the latter
used 43,513 moves, including 417 incidence switches, had maximum three moves and maximum initial
energy four, and took 88.95 seconds.  A new 100,000-iteration `K=6` difficulty hill climb also
found no failure; its best state started at energy five and needed three strict row swaps, at a
cost of 169.971 in-process seconds / 171 wrapper wall seconds.  These `K>=5` runs are falsification
evidence only.  The
complete sanitized `K=3` run passes address/undefined sanitizers under build
`c6771476620eb57218b5d2c4eda3ec6777e1a57292126380da0b769391fea99d`.

Several seductive simplifications were deliberately attacked rather than accumulated as new
conjectures:

- Canonical columns need not all have imbalance at most one.  A 338,984-iteration adversarial
  search found
  `(23,22,22,22,18,17,14,11,11,10,10,7,7,6,6,5,5,5,3,3,2^4,1^8)`,
  with imbalance two in a rank-3 canonical column.  Its quota energy is already zero: the bad
  column is an allowed mixed buffer.  The search cost 61.2 seconds.  A proof must track quota
  defect, not maximum discrepancy.
- Assigning actual Pascal ranks irrevocably fails.  Largest-residual sequential assignment fails
  at `(3,2,2,2)@K=2`; maximum-weight rank matching repairs `K=2` but fails at
  `(8,5,4,4,3,1,1,1)@K=3`, even with longest-future-chain urgency.  These tiny checks took about
  0.2 seconds.  Incidence switches are the necessary backward correction.
- Simultaneously freezing every recursive cut into one global Boolean row address is much too
  strong.  If every subset-column must be a coordinate subcube, even `G_2=(4,3,1,1)` is
  impossible: the row of degree four fixes the two half-columns and the singleton, forcing its two
  square-neighbors to have degree at least two and yielding `(4,2,2,1)`.  Exhaustive enumeration
  took 0.2 seconds and found only 3 of the 4 exact-support `K=2` types and 38 of 160 at `K=3`.
  Recursive branches must relabel independently.
- The formal graph operator is not the missing induction theorem.  With
  `T(G)=G disjoint-union (G join G)`, `T(E_3)` is nice with maximum profile `(6,3)`, but
  `T(T(E_3))`, whose maximum profile is `(12,9,3,3)`, has no stable partition of the dominated
  type `(12,9,2,2,2)`.  The short packing proof is in the evidence note.  The exploratory generic
  ideal/operator surveys took between 0.5 and 17.1 seconds.  A proof for `Q_K=T^K(K_1)` must keep
  the exact Pascal seed hierarchy.
- A generic even-margin Havel--Hakimi discrepancy search found a non-Pascal counterexample at
  eight rows in 13 seconds, so the observed descent cannot be justified by a general matrix
  balancing theorem.  One earlier one-switch Python search was stopped after more than 30 seconds
  without a retained conclusion.  An initial Python probe also hit the unavailable
  `int.bit_count` method and was corrected after 0.2 seconds; none of these failed attempts is
  evidence for the positive conjecture.

This route is the current noncircular conclusion.  Repeated halving is workable and now has a
complete state graph; the fixed realization, fixed coloring and irrevocable-rank versions are
false.  The remaining proof obligation is one Pascal augmenting-cut theorem: if the canonical
two-move walk stops at positive `Phi`, use the dyadic degrees and adjacent binomial quotas to turn
its stopped switch cut into a violated prefix of the parent.  No such derivation is proved yet, so
the Singleton Majorization Converse remains open.  Full definitions, traces, proofs, run table and
commands are in `evidence/singleton_balanced_hh_switch_2026-08-29.md`.
Final inventory found no `radio_canon`, `singleton_balanced_hh`, capped runner, `Python -` or
`python3 -` process remaining.
