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

Full `radio_full` split enumerations of the frontier states, m=5 and m=6 at k=5,6,7. Two
recursions, exact on every available k:

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
