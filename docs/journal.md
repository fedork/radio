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

Theorem-level result

- Singleton Majorization Theorem

- Key lemma: Three-Way Majorization Decomposition Lemma

- Working statement: Sb(a1:1, a2:1, ..., an:1) with nonincreasing a is solvable in K tests iff a is weakly majorized by G_K.

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

- Can the Three-Way Majorization Decomposition Lemma be proved internally without citing polymatroid machinery?

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

- Repair: Sb(32:1,31:1,17:2) in 5 can be proved directly by split [8:1,16:0,16:1]. The three K=4 branches are Sb(16:1,8:1), Sb(16:1,15:1,9:1,8:1), and Sb(16:1,9:1). All are singleton-only and weakly majorized by G_4, whose first relevant entries are [16,15,11,11]. This supplies an explicit replacement for the previously invalid domination/singleton rewrite.

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
`k=9` but predicts `496+481=977` versus exact 973 at `k=10`.

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
second is obtained from the first by a single coin move that *strictly decreases* mass. Both are
singleton states, so this follows from the Singleton Majorization Theorem rather than from a
solver: prefix sums `8, 10` pass against `G_3 = (8,7,4,4,1,1,1,1)`, `9 > 8` fails.

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

**Can the Singleton Majorization Theorem be generalised to a metric on non-singleton parts?
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

## 2026-08-10 — the deficit automaton gives the exact `k=10,m=6` break

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

### Exact bounded recurrence

`tools/search_singletonization.cpp` implements `C_d(S,k)`: require full-star majorization, stop
exactly on an arbitrary singleton-majorized state, otherwise choose one legal synchronized split
whose children satisfy `C_(d-1)`.  At `d=k` this is exact solvability.  The wide-cut interval above
is complete; while assembling several parent parts, rejection of a partial child is sound by
subgraph monotonicity.  Along the present searches the total narrow-side multiplicity is at most
six, so a normalized state has at most six nonempty parts.

Before trusting a new negative implementation, it was checked against independent controls.  On all
25 proven one-part frontiers with `k<=6,m<=6`, it rejected `n+1` and accepted `n`.  It also
reproduced the `Sb(16:1,12:2)@4` negative and `Sb(16:1,11:2)@4` positive from `refsolve.py`;
accepted the known `Z_6` and rejected the independently `R_4`-refuted `Z_7`; and reproduced the
width-two positive/negative pair in the synchronized-hierarchy theorem note.  Representative cases
also passed AddressSanitizer and UndefinedBehaviorSanitizer.  Positive output is separately checked
by `tools/check_witness.py`, which now accepts `[majorized G_k]` terminals and verifies every
weak-majorization prefix rather than trusting the search.

### Exact frontier and the corrected root

The final cold replay exhaustively rejected `Sb(974:6)@10` in 576.178 s, visiting 810,726 memo
states and 3,712,815,870 partial split assignments.  With that memo retained, `Sb(973:6)@10` was
accepted in 54.4011 s.  Subgraph monotonicity plus the checked positive tree proves

    n(10,6) = 973.

The retained line pair is `evidence/sb_m6_k10_frontier.txt`; the 115-node, 38-split, 77-terminal
proof is `witnesses/majorized_973_6_at10.tree`.  It uses root `[477:2]`, producing

    Sb(477:2),        Sb(496:2,477:4),        Sb(496:4).

The `m=4` pure child remains saturated, but the other width is 477 rather than the fitted 480.
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
componentwise Pareto upgrade before its split is useful; singleton leaves remain exact under the
Singleton Majorization Theorem.  This suggested a construction, not merely another scalar score.

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

The new 481 strategy was extracted as `witnesses/majorized_481_5_at9.tree`.  Its 61 nodes, 20
splits and 41 singleton-majorized leaves re-verify independently of the exact solver.  The root is
`[239:1]`, equivalently its complement `[242:4]`.

An exact forced-root scan explains the extra coin.  At `Sb(481:5)@9`, every capacity-feasible
`3+2` root `[a:3]`, `a=226..248`, is negative.  Among `4+1` roots, `[a:4]` is negative for
`a=225..239` and positive for `a=240,241,242`; `5+0` is impossible because two pure `m=5`,
`k=8` branches have total capacity only `2*231=462`.  Thus the root type is forced to switch from
`3+2` to `4+1`, up to complement.  The diagnostic is retained in
`evidence/sb_m5_k9_root_transition.txt`; it is structural solver evidence, while the verified tree
and published theorem carry the mathematical claim.  All 44 raw forced-root outputs have complete
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
`496+481=977`, not 976, against the exact `n(10,6)=973`.  The successful root backs the relevant
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

The exact D values are

```text
d*(t)=P-Q     for t=7,8,
       P-Q+1  for t>=9.
```

This is an exact D maximum, not just a lower construction.  If `R_t(d*+1)` were solvable, the
other branches would remain solvable (`d*+1<=P-1`, `b+d*+1<=2P`), and the two outer tests would
construct `Sb(n(k,5)+1:5)`, contradicting the published upper bound.  Subgraph monotonicity excludes
all larger D.  The finite `k=8` tie separately has `d*=P-Q-1=57`.

The eventual lower construction was also reduced to a self-contained symbolic template.  For
`d=P-Q+1`, one test of `R_t(d)` followed by one test in each non-singleton child leaves only
singleton states.  Their decisive five-part deficit multiset relative to `P/4` is
`{1,Q-t-2,t,t,2t-1}`, versus `{0,1,t-1,t-1,Q+t-1}` for the first five entries of `G_(t-2)`.
The candidate is weakly majorized throughout the intended range `t>=9`; a three-part leaf already
violates majorization at `t=7,8`.  Thus the `k=11` breakpoint arises inside the D strategy, not from
fitting the published final values.  The finite `t=7,8` constructions plus this template supply
achievability, while the paper supplies the sharp global upper bound.

Complete `assembly-enumerate` replay over proven Pareto inputs now locks the crossing: `(3,2,2)` is
the sole winner for parent `k=4..7`, both `(3,2,2)` and `(4,3,1)` reach 231 at `k=8`, and only
`(4,3,1)` reaches 481 at `k=9`.  Direct exact construction checks of the latter hard branch give
`d=241` at `k=10` and `d=492` at `k=11`, producing the published widths 985 and 2001; their emitted
trees verify independently.  The global upper bounds remain sourced to the paper, so no new solver
artifact or Pareto row was created.

`tools/m5_assembly.py` now evaluates both symbolic candidates and separately labels the atom-mass
identities `BBBD`, `ABBD`, and `AABD`.  `tools/check_tables.py` validates the assembly algebra for
61 levels (`k=4..64`) and matches all seven recorded exact `m=5` rows to the theorem.
`tools/singletonization_regression.sh` contains the complete finite assembly controls plus the
`k=10,11` constructions.  The full regression passed in 74.87 wall seconds (70.15 user, 1.52 sys)
on this machine.

Strategic consequence: the general track survives, but its scalable state is a guarded envelope of
outer families and piecewise D frontiers, not one preferred height triple or one atom word.  The
height-6 rank-1180 problem remains a genuine all-depth question inside the fixed `(4,3,2)` aligned
slice; deciding it alone would not establish the unrestricted `m=6` frontier.  The complete proof
and scope boundary are recorded in `docs/theorems/m5-pareto-assembly.md`.

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
- The known-answer `m=5` case is completely reconstructed.  Its outer envelope changes from the
  `(3,2,2)` family to `(4,3,1)`, and the winning family's D maximum itself changes formula.  The
  eventual branch has a direct singleton-majorization construction, so this is a positive
  calibration of the local mechanism and a negative calibration of the hoped-for simplification.
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
