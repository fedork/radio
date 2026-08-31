# Publication-claim inventory

This is a deliberately conservative inventory for a prospective paper. A candidate claim is
listed here because it has a proof object or a retained computational record; that is not by
itself a claim of priority. Before submission, each item needs a focused prior-art check and the
stated reproducibility package.

## Recommended headline: exact finite frontiers

### C1 — `Sa(10)=192` exactly

This is the strongest result. A checked witness proves `Sa(192)` is solvable in ten tests; the
proof-safe cold run and its compact level-chain certificate refute `Sa(193)` in ten tests. Thus
the result is a **computer-assisted exact theorem**, not merely a search result. See the source
record in [pareto_sa.csv](../data/pareto_sa.csv), the explanation in
[results.md](results.md#sa10--192-proven-maximal), and
[sa193-certificate.md](sa193-certificate.md).

Publication value: it is a concrete finite impossibility result substantially sharper than the
ternary counting bound. It should lead a computational-results paper.

Before submission: release the raw retained run, the compact certificate, the checker, build
provenance and a clean independent reproduction path. An independently written full-DAG replay
would further strengthen the trust base, but the present certificate is already solver-free to
check.

### C2 — complete exact `Sb` Pareto frontier through `k=8`

Every reported `Sb` cell through eight tests is a maximum: the source-of-truth records are in
[pareto_sb.csv](../data/pareto_sb.csv), and the coverage/provenance statement is in
[results.md](results.md#provenance). This is a finite classification, not an extrapolated formula.

Publication value: it maps the complete bipartite finite landscape much more finely than the
published asymptotic constructions. It is best presented as the companion dataset and theorem
to C1 rather than as an isolated large table.

Before submission: establish which cells, if any, already appeared in an accessible exact table;
include both boundary verdicts and their artifact hashes in supplementary material.

### C3 — an exact `m=6` boundary, a new upper bound, and failure of the former continuation

The value `n(9,6)=473` has a canonical achievability witness and retained rejection at 474.
At the next level, exhaustive search proves `n(10,6)<=973`; the present 973 construction stops at
arbitrary majorized singleton leaves, which are not certificates because the converse is false.  See
[pareto_sb.csv](../data/pareto_sb.csv) and
[special-cases.md](theorems/special-cases.md#10-sb2k---kk-12---3-6---refuted). In particular,
the former `m=6` continuation fits through nine tests but predicts 976 at ten, and is false.

Publication value: this is a compact, interpretable new finite theorem and a useful warning
against inferring global formulae from low-depth data. It belongs beside C1/C2.

Before submission: package the `m=6` rejection logs and the canonical 473 witness; do not present
973 as achievable unless an unconditional 973 tree is supplied.

## Theory suitable for a theorem section or a companion paper

### T1 — exact level boundary for the Singleton Majorization Converse

Every singleton state solvable in `K` tests is weakly majorized by the explicit base sequence
`G_K`.  Conversely, every `G_K`-majorized singleton state is solvable for `K<=5`.  For every
`K>=6` the converse is false: an explicit infinite family gives a full-mass exact-support parent
with no legal first cut.  Its first member,
`(64,63,57^2,42^4,22^7,8^15,7^2,1^32)`, is majorized by `G_6` but has no legal first split and is
unsolvable in six tests.  More sharply, its 30-row prefix
`(64,63,57^2,42^4,22^7,8^15)` of mass 683 already has no legal first split; deleting one 8 restores
first-cut feasibility.  Thus `K=6` is the exact first failure level.  The transcript-conflict
graph and Fixed-Color Hall criterion give the short tight-rank proof.  The `K=5` direction is a
computer-assisted theorem: exact prefix cylinders cover 1,431,650,734,151 of the
1,431,800,647,444 normalized parents, and uncapped Hall search closes the rest.  A separate complete
5,189,450,419-parent Hall census proves that transfer distance 14 is globally minimal among
full-mass exact-support recursively unsolvable `K=6` parents.  See
[singleton-majorization.md](theorems/singleton-majorization.md) and the
[complete `K=5` record](../evidence/singleton_k5_prefix_cylinder_2026-08-31.md).  Canonical and
distinct-slot witness leaves remain independently checkable from the explicit strategy for `G_K`;
the first-failure and distance claims also have independent records linked from the theorem note.

Publication value: this completely classifies the levels at which Aigner's majorization condition
is sufficient, resolves his stated converse question negatively, and gives both a short first
counterexample and an infinite family.  The exact distance census supplies a sharp finite
minimality statement.

Before submission: independently audit the short and infinite-family proofs, package the `K=5`
reproduction and perform a focused literature/novelty search before claiming priority.  Cite
Aigner for the historical necessity result and question.

### T2 — Exact Prefix-Cylinder Extension Lemma

For a fixed coloring of an exposed parent prefix and a fixed rank-coloring word on its unknown
suffix, the maximum left side of every Fixed-Color Hall inequality over **all** dominated suffix
completions is an integer support function.  Bounding these exact support values by the Hall
capacities is necessary and sufficient for that one uniform scheme to give every completion a
majorized first cut.  The support values obey a short exact finite recurrence.  A closed-form
three-bound corollary replaces each value by the minimum of the available parent-prefix mass,
remaining-mass and row-maximum bounds.  See
[singleton-majorization.md](theorems/singleton-majorization.md#exact-prefix-cylinder-extension-lemma).

Publication value: this is the reusable sufficient lemma behind the `K=5` computation.  It cleanly
separates an analytic universal certificate from finite search, and compresses enormous families
of parent partitions without assuming the false global converse.

Before submission: give pseudocode for the recurrence and a small independently checkable
certificate transcript; be explicit that exactness is relative to a fixed uniform coloring scheme,
not a closed-form characterization of the union over all colorings.

### T3 — Unit-Group Elimination Theorem

Appending `u` resolved `1:1` components affects solvability only through the total leaf capacity:
the non-unit core must be solvable and the total mass must not exceed `3^K`; see
[unit-group-elimination.md](theorems/unit-group-elimination.md).

Publication value: short but useful normalization theorem. It belongs naturally as a corollary
or implementation consequence of T1, rather than as a standalone headline.

### T4 — Vertex-splitting pullback and full-star majorization

Every general bipartite part can be split into singleton stars, yielding a sound necessary
majorization condition for arbitrary multipart states; see
[singleton-majorization.md](theorems/singleton-majorization.md#vertex-splitting-pullback-lemma-2026-08-09).

Publication value: a general, cheap obstruction that explains both the canonical certificate
format and a major solver reduction. It is a strong theory/algorithm bridge.

Before submission: distinguish clearly between this necessary condition and exact solvability;
the width-two counterexample in the same note shows why no naive one-sequence extension is exact.

### T5 — synchronized-majorization predicates

Every predicate `R_d` is necessary for solvability, and `R_K` is exact.  The previously claimed
nesting `R_(d+1)=>R_d` is unproved because its base step used the false singleton converse.  The
width-two counterexample to any one fixed base sequence remains valid. See
[singleton-majorization.md](theorems/singleton-majorization.md#the-synchronized-majorization-predicates-corrected-2026-08-26).

Publication value: promising, but presently a secondary theorem. Its importance depends on
whether the hierarchy yields a usable characterization, a nontrivial complexity result, or new
finite boundaries beyond the current implementation use.

## Important supporting material, not headline novelty

- The independent replay of Li--Wu--Triesch's exact `m=5` transition to 481 validates the
  pipeline and explains a previously misleading local formula, but the exact theorem is
  published already; see [literature.md](literature.md#li-wu-triesch-2018).
- Subgraph Monotonicity and the recursive lift-box lemma are clean, useful infrastructure
  results, but individually elementary; use them in proofs and certificates rather than selling
  them as main theorems. See [subgraph-monotonicity.md](theorems/subgraph-monotonicity.md) and
  [recursive-pareto-lift.md](theorems/recursive-pareto-lift.md).
- The asymptotic constant is not a project claim: Florin--Ho--Jiang already settle it. The
  paper should position the finite exact results as complementary to that asymptotic theorem;
  see [literature.md](literature.md#florin--ho--jiang-2022).
- The aligned-profile D-lineage obstruction is restricted-model research infrastructure, not a
  candidate publication claim. It may guide future work, but has no place in the proposed paper
  without an unrestricted consequence.

## Claims explicitly excluded from a submission

- Any global fixed-`m` formula for `m>=6`.
- Any assertion that the complete `k=9` `Sb` frontier is known; `m=7..64` is open.
- The antidiagonal conjecture `(u1)` and profile fits for `m>=7`.
- Legacy positive records without retained source output, except when explicitly labelled
  historical lower bounds.

## Sensible paper package

1. **Main paper:** C1, C2, the unconditional part of C3, and T1--T3, with the finite tables and certificates as the central
   contribution.
2. **Supplement:** raw/provenanced solver artifacts, machine-readable frontiers, witness trees,
   independent checkers, and a command-by-command reproduction guide.
3. **Possible later theory paper:** the synchronized hierarchy, only after it yields a useful
   characterization, nontrivial complexity result, or new unrestricted frontier consequence.
