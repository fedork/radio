# Research plan

Living document. Update it when a goal is met or reprioritised; do not accumulate stale
entries. Last revised 2026-08-31.

## High-level goals

**H1 - Publish.** Get the two-coin quantity group testing results out: the `Sa` sequence
proven optimal through `k=10`, the `Sb` Pareto frontier proven for `k<=8`, and the foundational
theorems. The draft in `paper/` can now state the exact `Sa(10)=192` boundary; its remaining
work is the P5 cleanup and theorem integration.

**H2 - The K=9 Sb column.** Published theorems now make `m=1..5` exact, including the
corrected `n(9,5)=481`; a retained exact replay makes `n(9,6)=473` exact as well. Run9 adds
proven upper bounds at `m=81..96`; legacy lower bounds remain in parts of `m=65..94`. The band
`m=7..64` is entirely blank.
This is the main open front.

**H3 - Is `Sa = 192` maximal at k=10? Done 2026-08-16.** Proof-safe cold run9 began from an
empty cache, passed the independently known `Sa(192)` control, and exhaustively refuted all sixteen
states `Sb(n1:193-n1)@9`, `n1=97..112`, in one uninterrupted session. It returned UNSOLVABLE in
419353.1 CPU seconds, used 419849 wall seconds and peaked at 1.32 GB RSS. A verified witness proves
192 achievable, so `Sa(10)=192` is exact.

The raw proof log and reproduction metadata are archived under `sa193-cold-2026-08-16`; the compact
root evidence is in
[`../evidence/sa193_unsolvable_in_10.txt`](../evidence/sa193_unsolvable_in_10.txt). Run3 and run8
also concluded UNSOLVABLE but remain performance-only because their builds predate the contraction
fix. An independent full-DAG `radio_verify` replay would strengthen the trust-base separation, but
it is optional follow-up rather than unfinished H3 work.

**H4 - Structural theory.** Prove or refute fixed-`m` families rather than fitting them.  The
excess-`q` Pareto-assembly programme is **parked as of 2026-08-16**: retain its proved local
reductions, exact trees and relaxed-terminal derivations, but do not treat further normalization/rank scans
as a proximate goal.  The literature now settles `m=5`: the old formula/`BBBD` equality is refuted
at `k=9`, and the exact
piecewise theorem changes the root from `3+2` to `4+1`.  The corrected Pareto assembly finds the
crossing, but its arbitrary singleton-majorization terminals relied on the now-refuted converse;
the published theorem, not that reconstruction, is the unconditional source.  It remains the
known-answer standard for every
proposed generalization: retain competing outer triples, solve each D problem, and take their
guarded envelope.  Its first eventual decisive singleton-majorized leaf has sharp exact and
distinct-slot-embedded depth three.  No fixed exact depth at most five can work uniformly at every
sufficiently large level; depth six is the first compatible per-component inventory, but its
synchronized packing and the uniform embedded minimum are open.  This is deliberately separate
from the published formula, whose quantified `k>=11` range excludes later numerical transitions
without asserting a unique or aligned `AABD` strategy.  The old `m=6` closed form and `BBCD`
profile are refuted by the unconditional upper bound `n(10,6)<=973`.  A checked
sixteen-atom relaxed aligned tree suggests the different unsupported continuation
`2^k-k^2+7k-21` for `k>=12`; global equality and the finite `k=11` case remain open.  Inside the
fixed `(4,3,2)` aligned height-6 slice, the unresolved excessive-`q` question narrowed sharply: a
32-atom all-depth kernel leaves only rank 1180 between the excluded prefix and the constructible
pure refinement at rank 1181.  That one-rank decision does not by itself establish the global
`m=6` frontier; an outer-family comparison analogous to the exact `m=5` envelope is separate.
More generally,
the outer algebra reduces each `N=2^s` slice to minimizing the D germ's `C` count and then its `B`
count; no arbitrary word search remains.  The remaining rank has now been exactly excluded through
depth four.  Exact pure-outcome solution reduces the last level to 1,818 distinct mixed children,
indexed by the single loss `1<=ell_W<=14`, and guided cover exhausts every one.  An all-depth
decision would require a synchronized closed kernel/recurrence containing rank 1180 or an exact
tree whose depth is necessarily at least five.  That question is preserved but not scheduled; do
not restart the closed depth-four frontier.
For a general normalization `N=2^s` and a fixed mixed-depth budget `t`, scalar supply gives the
sharp necessary boundary `c>=max(0,2s-1-2t)` and a quadratic B bound when equality holds.  Its
depth-three specialization has only five persistent B-saving tracks, but it becomes vacuous in C
once `t>=s`; the arbitrary-excess conclusion therefore needs a synchronized closed kernel or
recurrence rather than further propagation of this scalar inequality.

## Proximate goals

### P1 - Make the record trustworthy *(done 2026-08-03)*

The repo reorganisation: source-of-truth tables with per-cell provenance, verified witness
trees, mechanical invariant checks, agent instructions, artifact archival.

**Done when** all witness trees pass `tools/check_witness.py`, `tools/check_tables.py` is
green, and every artifact cited by `data/*.csv` is fetchable by tag.

Delivered: source-of-truth CSVs with per-cell `bound`/`status`/`source`; 13 verified witness
trees; four checking tools; the artifact store `fedork/radio-data` (10 tags, round-trip
verified) with the archiving decision recorded in [data.md](data.md) including what was
deliberately not kept; the vendor-neutral `AGENTS.md` brief with a session-end protocol;
[status.md](status.md) as the cold-start snapshot. Merged to `main` and pushed 2026-08-03.

Credentials are per-repo and leave global config alone: `core.sshCommand` for git,
`GH_CONFIG_DIR=.gh` for `gh`, both as `fedork`.

Durability extension (2026-08-11): new solver output is self-identifying. The canonical builder
embeds commit/source hashes/compiler arguments and emits a binary sidecar; runtime stdout adds exact
arguments, host/OS/CPU/RAM and resource limits. Checkpoints preserve the block and archival rejects
missing/incomplete provenance unless a historical override is explicit.

### P2 - Extend the K=9 column constructively

Canonical search for `m = 7` (predicted 457) and `m = 8` (predicted 447), where both models
agree. Each success is a proof standing on Aigner's explicit `G_k` strategy and Subgraph
Monotonicity.

```
./run_radio_canon_search_generic.sh 4 9 457 7
./run_radio_canon_search_generic.sh 4 9 447 8
```

**Done when** a validated tree is committed for each, or the attempt is recorded in
[journal.md](journal.md) with its cost and where it stalled.

### P3 - Distinguish the remaining m=9,10 rows

The models are already dead as global laws at `m=5,6`, but their row-wise `m=9,10`
predictions remain open.  `Sb(432:9)` in 9 distinguishes 432 from the closed-form equality
431; `Sb(416:10)` independently distinguishes 416 from 414. Details and the asymmetry of
the test are in [conjectures.md](conjectures.md#the-remaining-m910-discriminating-experiment).

**Done when** one row is refuted, or both searches are recorded as inconclusive with
their cost. Also worth recovering: the length-64 `m = 11` profile string, which the journal
mentions but never wrote down, and which would let `n(9,11)` be predicted at all.

### P4 - Fill the K=9 band m = 7..64

Superseded in part: proof-safe run9 now decides the 16 `Sa(193)` upper bounds, and
`parsed_260.txt` has been recovered, so `run_pareto9.sh` can restart. What remains is the blank band.

Note what the recovered data shows about cost: the near-diagonal walk moved from `m = 96`
down to about `m = 81` over 14 months of wall clock across 81 chained runs, and single states
in that region took weeks each. A frontier walk downward from the diagonal will not reach
`m = 64` on any useful timescale. Approaching the band from small `m` upward with the
canonical search (P2/P3) is the only cheap direction, and P2 should be read as the first step
of P4 rather than as separate work.

**Done when** there is a measured cost model for the band and a decision about whether to
pursue it at all.

### P5 - Correct and finish the paper

- Replace the stale K=8 column (`m = 10..17`) with the values in `data/pareto_sb.csv`.
- Fix lemma (10): `k(k-1)/2`, not `k(k-5)/2`.
- Fill the `<TODO>` sections - Terminology, the Unit Group Triviality Lemma (now proved in
  [theorems/unit-group-elimination.md](theorems/unit-group-elimination.md)), Insights,
  Refuted lemmas.
- Add lemma (12) for `m = 8`, and the `G_k = sum of binomials` closed form.
- Present `Sa(10)=192` as an exact maximum: a verified witness proves achievability and the
  proof-safe cold run9 log rejects all sixteen first-test states for 193. Keep the suspect 2023
  result only as historical cost context.
- Fix the numbering collision: `(7)` is currently a duplicate of `(5)`, and the sentence
  "(7) holds true k up to 8" plainly refers to `(u1)`.
- Replace proposed lemma (8) by the published exact `m=4` theorem and proposed lemma (9) by
  Li--Wu--Triesch's exact piecewise `m=5` theorem.  Record the independently replayed
  `n(9,5)=481` / `n(9,5)<482` boundary and the `3+2` to `4+1` structural switch.
- Add the primary-source discussion from [literature.md](literature.md): Aigner for the graph
  model and exact `m=2,3`, Li--Wu--Triesch for `m=4,5`, and Hao/Gargano et al. for scalable
  product constructions; also distinguish the modern asymptotic results of
  Jiang--Polyanskii--Vorobyev and Florin--Ho--Jiang from finite fixed-`m` claims. Do not copy
  Li et al.'s apparently inconsistent intermediate equations (69)–(70) without recomputing
  their indices.
- State that every `Sa` table entry through `k=10` is proven maximal, and cite the distinct
  witness/exhaustive sources for the new last cell.

**Done when** the draft passes `tools/check_tables.py` with no stale generated blocks and
contains no number absent from `data/*.csv`.

### P6 - Full star expansion after the singleton converse counterexample

**Foundational singleton subproblem (resolved negatively 2026-08-30).**  Weak majorization by
`G_K` is necessary but not sufficient.  The full-mass exact-support state

    (64,63,57^2,42^4,22^7,8^15,7^2,1^32)

is majorized by `G_6` and has no legal first split.  Do not pursue a universal Row-Coloring,
no-holes, transfer, switch, shuffle or chain-splicing proof; all such sufficient targets below are
retained as historical structure and finite lower-level theorems.  The ordered singleton programme
is:

1. **Delivered 2026-08-31:** the dependency-free clean-room solver enumerates legal row triples
   `(l,m,r)` directly, shares no Hall/coloring code or caches with the current census, reproduces
   the canonical and `j=13` positive cuts, and independently rejects both the padded `K=6` state
   and its underfull mass-697 core.  An unquotiented leaf-only oracle agrees on 201 small
   partitions; the optimized solver closes all 1,223 full-mass majorized types at `K<=3`, including
   all 1,206 types at `K=3`.
   Reproduction and exact counts are in the
   [clean-room verification record](../evidence/singleton_direct_split_cleanroom_2026-08-31.md);
2. **Delivered 2026-08-31:** the proved **Tight-Band Capacity Obstruction** extracts every monotone
   transition between two tight anchors and rejects it by a mixed-floor/pure-capacity inequality.
   An inequality-only checker reproduces the two-case rank-15/32 proof.  On the complete fixed
   `[15,32)` face, independent enumeration finds 176 dominated 17-row bands: the direct solver
   classifies 175 as first-cut feasible, while both implementations identify `(8^15,7^2)` as the
   unique hole.  An extended complete census finds no capacity certificate among all 613,689,090
   eligible `K=5` bands, while exact prefix-cap optimization makes distance 14 globally minimal
   within this certificate class at `K=6`.  A later direct shell census, recorded in the next item,
   upgrades the distance statement beyond this certificate class.  See the
   [theorem](theorems/tight-band-capacity.md) and
   [verification record](../evidence/singleton_tight_band_capacity_2026-08-31.md);
3. **Exact-support first-cut distance delivered 2026-08-31; `K=5` remains open.**  A ranked,
   parallel Fixed-Color Hall census exhausts all 5,189,450,419 exact-support `K=6` parents through
   distance 13 and finds a majorized first cut for every one.  The known distance-14 parent is
   therefore globally minimum for exact-support no-first-cut holes.  The same tool proves all
   311,082,023 exact-support `K=5` parents through distance 14 recursively solvable.  See the
   [transfer-shell record](../evidence/singleton_transfer_shell_census_2026-08-31.md).

   Now settle `K=5`, and minimize the **non-unit core** beyond the fixed face under level, support,
   recursive solvability and a clearly specified dominance/normalization order.  Minimum-Support
   Reduction leaves 1,431,800,647,444 exact-support `K=5` parents, of which 147,422,086,892 have no
   internal tight prefix, so do not launch a flat parent census.  The complete zero-certificate
   result rules out the current two-anchor theorem as the deciding method; next seek a symbolic
   tight-skeleton automaton, a laminar/general Hall-dual search, or another exact quotient that
   covers the strict interior.  At `K=6`, do not confuse the new first-cut minimum with recursive
   unsolvability: a closer parent could cut only to an unknown `K=5` failure.  The distance-14 shell
   may also contain other holes.  Deleting the harmless `1^32` padding already gives the 32-row
   mass-697 core and does not count as a new smaller obstruction.  Feed every new hole through the
   extractor; an uncertified hole is the concrete input for the laminar/general-Hall programme;
4. replace scalar majorization by a useful necessary-and-sufficient description.  The existing
   stable-partition/chain-partition formulation is exact but tautological; the target is a compact
   recursive support or forbidden-band criterion that recognizes when the union of fixed-color
   Hall base sets contains the requested integer point.

Separately, replace each nonembedded majorized witness leaf by an individual exact/canonical
strategy rather than a blanket lemma.  See the
[`K=6` proof](../evidence/singleton_k6_counterexample_2026-08-30.md) and
[theorem note](theorems/singleton-majorization.md).

The cleanest former global target had an ordered-rank form.  The transcript poset `P_K` has rank
numbers `R_K(r)=2^popcount(r)`, is normalized matching, and has a nested chain decomposition of
shape `G_K`.  Gale--Ryser says `a<=_w G_K` is exactly the condition for a zero--one matrix with row
sums `a` and column sums `R_K(r)`.  A matrix is **carry-compatible** when its rank incidences lift
to actual `P_K` chains through the complete-bipartite binary-carry components.  The claim that one
can always choose such a matrix is false at `K=6`.  This Carry-Compatible Gale--Ryser statement is equivalent to the Balanced Pascal
Realization and Row-Coloring claims, and the counterexample refutes all three simultaneously.
Do not try to lift an arbitrary margin realization: (PR6) gives a solvable `K=4` row-degree state
and a correct-margin matrix whose four upper columns force an impossible odd cycle of row
bisections.  The exact proof must choose or switch the incidence matrix jointly with its lift; see
the [Pascal-rank-poset formulation](theorems/singleton-majorization.md#the-pascal-rank-poset-and-the-griggs-dominance-connection-2026-08-29).

A concrete stronger route realizes the no-exchange proposal locally.  The canonical chains
have binary codes; if chain `C_j` is longer than `C_q`, its rank-`2^d-1` bottom word is comparable
with every word of `C_q`, so the first unit transfer is a direct recoloring.  For an arbitrary
exposed donor word, the compatible receivers in one equal-length code block form a dyadic cylinder
whose prefix length is given by (PR8).  The **Canonical Monotone-Transfer Conjecture** asked whether every
target below `G_K` has some globally scheduled path of these direct moves.  It is sufficient but
stronger than the converse.  Exact construction reaches all 1,206 `K=3` targets; arbitrary move
order is false, and incoming words that share a receiver cylinder can conflict with one another.
The `K=6` hole proves that no universal schedule or laminar receiver-list invariant can cover every
target.  Retain the direct bottom-cell lemma and the finite `K=3` construction, but do not continue
this global route; see the
[direct-transfer section](theorems/singleton-majorization.md#a-canonical-direct-transfer-lemma-and-the-failed-global-schedule)
and [census](../evidence/singleton_monotone_transfer_census_2026-08-29.md).

The simpler final-state route exposed by the low-level survey is also refuted universally.  The
**Pascal Two-Interval Splicing Conjecture** asked to cut the canonical chains into rank intervals and pair at most
two intervals per target chain, with every pair rank-separated and drawn from distinct source
depths.  This is exact for every `K<=3` target.  It is not a disguised cuts-only assertion--cuts
alone miss 615 of 1,206 `K=3` types--and neither fixed rank direction nor adjacent-depth pairing is
valid.  Orienting splice edges by source depth gives a DAG and a coupled interval-cut/endpoint-
matching problem, but the `K=6` hole proves that the required endpoint matching need not exist.
Definitions, exact counterexamples to the stronger
forms, and `K=4` probes are in the
[cut-and-splice section](theorems/singleton-majorization.md#a-low-level-cut-and-splice-normal-form-2026-08-29)
and [survey](../evidence/singleton_cut_splice_survey_2026-08-29.md).

Do not require two normalized children to be identical.  The complete `K=3` recombination census
finds 16 counterexamples, led by `(8,3^6,1)`.  The exact nearby target is the **Adjacent-Children
Split Conjecture**: every parent has a valid split in which some two child types are equal or differ
by one Robin--Hood transfer.  This covers all 1,206 `K=3` targets but is false universally because
the `K=6` hole has no child triple at all; the same applies to the stronger-looking dominance-chain
condition.  The counterexample proof,
the 16-state exception family, and the exact finite salvage are in the
[identical-child census](../evidence/singleton_identical_children_census_2026-08-29.md).

The multiplicity probe is now complete through `K=4`.  A compiled parent-first counter avoids the
`1206^3` triple product and exhausts all 5,997,038 parents.  The child-orbit layers one, two, three
and at least four have exact sizes `30,123,106,5,996,779`; allocation-orbit layers one, two and
three have sizes `8,19,32`.  The `K<=3` **Rigidity-Heredity Conjecture is refuted**: a forced `K=4`
triple can be all-distinct and contain two children that each have two child orbits.  Do not try to
prove or use strict heredity.

The useful replacement is the **Multiplicity-Filtration Conjecture**: a child-unique parent has at
least one child-unique child and every child has at most two child orbits.  It is exact through
`K=4`: of the 30 child-unique parents, 10 have three rigid children and the other 20 use only the
one- and two-orbit layer.  Test a theoretical implication from singularity of the weighted
matching projection to this bounded child multiplicity; do not launch `K=5`, whose parent corpus is
far beyond exhaustive enumeration.  The two- and three-orbit `K=4` layers are only 229 additional
states and provide the right finite boundary corpus for finding the first bifurcation rule.  Keep
the limitation explicit: 5,996,779 parents remain in the bulk, and `(3^9)@3` already proves that
an arbitrary parent need not admit even one rigid child.  Exact method, low-layer child statistics
and reproduction are in the
[multiplicity record](../evidence/singleton_split_multiplicity_census_2026-08-29.md); the original
`K<=3` table is in the
[unique-split record](../evidence/singleton_unique_split_survey_2026-08-29.md).

The complete low-fiber parent--solution relation supplies a sharper route than multiplicity alone.
The proved **Pascal Tight-Skeleton Factorization Theorem** now supersedes the dyadic-only
formulation.  At every tight rank `t`, retain the full plateau
`I(t)=argmax_p(H(p)+H(t-p))`; consecutive tight ranks produce contracted child bands, and global
allocations are transfer-matrix products along monotone plateau-count paths.  The old dyadic
head--tail product is the singleton-path case and still explains 131/259 low parents.

The former primary full-band target, the **Balanced Residual Coloring Conjecture**, is false.
Minimum-Support
Reduction first leaves exactly `2^K` rows.  The proved Two-Anchor Reduction then deletes the
universal doubled column and one maximum single column: with `n=2^(K-1)`, `h=G_(K-1)` and
`c=h-1`, subtract two from the longest `n` rows and one from the other `n`.  Bipartite
Havel--Hakimi proves that the resulting state `b` is dominated by the induced residual profile

    Jprefix(t)=C(t)+max_p(C(p)+C(t-p)),

where `C` is the prefix function of `c`.  It is enough to color every full `b<=J` of support at
most `2n` with at most `n` positive rows per color and satisfy

    B^A_p+B^B_q <= C(p)+C(p+q)+C(q).

Residual Hall allocation followed by one pure anchor on every row and one mixed anchor on the
original longest half then gives the desired split.  In Boolean columns this removes exactly
`empty` and one singleton `{*}`, so Pascal structure defines the residual rather than merely
supplying repeated targets afterward.  The `K=6` counterexample maps to the explicit residual hole

    (62,61,55^2,40^4,20^7,6^15,5^2)
      <=_w (62,61,55^2,40^4,20^8,5^16).

This residual lemma passes all 73 states at `K=3` and all 160,492 at `K=4`; the two-anchor images
of all 160/408,776 exact-support parents also pass.  The stronger Longest-Half Mixed Conjecture,
which gives all three children exact support and forbids the bottom half from using the mixed
child, also passes the complete exact-support `K<=4` corpus and the strict-interior `K=6`
alternation counterexample.  Those finite results remain correct, but the new residual hole refutes
both universal conjectures.  A useful
transfer clue is that every one of the 160,491 noncanonical
`K=4` residuals has an endpoint-coherent upward predecessor; 160,414 use the first admissible
predecessor and the remaining 77 use the second.  Turning that one-step fact into an induction
cannot be promoted to a global induction.

Do not replace the full Hall surface by a scalar objective.  Plain balance, globally minimum final
mass difference, residual two-smallest coalescence and a fixed product below the canonical boundary
all have exact counterexamples.  In particular `(14,13,9,5,4,4,4,2,2)` needs color-mass difference three
although difference one is possible under both necessary row-count bounds.  The theorem, complete
counts, stronger finite
pattern and counterexamples are in the
[two-anchor residual record](../evidence/singleton_two_anchor_residual_2026-08-29.md).

The broader **Positive-Band Extension Conjecture** is false.  It
passes the exhaustive `K=4` census on 1,722,516 band-state instances, and the old direct suffix
census passes 1,422,304 instances.  At `K=6`, the dominated band `(8^15,7^2)` below `(22,7^16)`
extends neither of its two endpoint transitions.  Internal tight subprefixes still factor immediately, but `(16,15,11,11,4^5)` refutes unrestricted
prefix refinement, `(4^4)` in band `[5,9)` forces an orientation switch, and a 64-row strict
`G_6` interior state refutes sorted alternation.  See the
[tight-skeleton record](../evidence/singleton_pascal_tight_skeleton_2026-08-29.md) and original
[factorization record](../evidence/singleton_low_multiplicity_factorization_2026-08-29.md).

Equivalently, the former Pascal Orthant-Saturation Conjecture said that the magnitudes of the integer points in
the signed Hall bisubmodular polyhedron contain every lattice point of their convex hull, which is
exactly the parent majorization polymatroid.  One constructive form asks that every unit
Robin-Hood transfer be routed by a global signed augmenting path through the doubled Pascal/dyadic
columns.  This formulation and the convex-hull equality are proved in the
[theorem note](theorems/singleton-majorization.md#global-signed-lifting-and-the-lattice-hole-at-k6).
The `K=6` state is the primitive lattice hole and the final edge of its 14-transfer path refutes
global transfer closure.

The exact Adjacent-Fiber census now passes every one of 141,690,676 normalized transfers from all
5,997,038 full-mass `K=4` states.  For `K<=3`, the globally optimal separator margin is always at
least two, and every `K=4` transfer has a certificate of margin at least two.  These are exact
lower-level theorems; Adjacent-Fiber itself fails on the final `K=6` transfer.  The same-color
strengthening is already false for 889 `K=4` transfer types, so the proof must handle a genuinely
opposite-color tight separator.  The earlier proposed global step was to characterize the minimal
separator exposed after maximizing its slack and show that Pascal-identical columns cross it.  The
889 cases are now exactly one four-row family, and its `K-3`-member continuation at every `K>=4`
has an explicit common crossing allocation: two mixed contributions move toward one another under
the identical pair `U-K,U-K`, with exact separator margin equal to their original gap.  The new Tight Pascal-Band Lemma
shows that every actual dangerous separator `(p,q)` saturates every dyadic child column and obeys

`#{T:p<c(T)<q} >= 2 + #{T:c(T)=p+q}`.

Thus every obstruction crosses a repeated internal Boolean rank; this is now a theorem, not only
the pattern seen in the 889 cases.  A fixed-color tight cut cannot be crossed while preserving row
orientations--that would contradict the Fixed-Color Hall Lemma.  This stronger local route optimizes
over feasible colorings of the original state and uses a global color-exchange augmenting chain to
raise the separating margin.  After quotienting equal-row identities and global side
complementation, every failed coloring in the exact `K<=3` landscape and the first 10,000 `K=4`
states has a successful coloring within two row changes.  Three disjoint 1,000-state windows after
1M, 3M and 5M enumerated states give the same result.  This finite two-row repair pattern cannot be
universal because the `K=6` target has no coloring.  Encode a coloring as the fixed-absolute signed vector `z_i=+/-a_i`.  The ambient Hall
polyhedron is integral bisubmodular and therefore has support-one-or-two delta exchanges, but its
fixed-absolute-value slice does not automatically inherit them.  The former target asked whether the repeated internal
rank from the Tight Pascal-Band Lemma compressed an ambient unit-exchange chain to a boundary flip
or swap that makes the transfer margin positive; the final `K=6` transfer refutes this.  The Boolean labels alone do not supply that
exchange, and nested/laminar supports are false already at `K=3`.  The fixed-absolute coloring
family itself is not a delta-matroid: `(3,2^11,1,1)@3` has an explicit symmetric-exchange failure.
Use the exact boundary calculus only as an optional stronger route.  Strict descent of the dyadic band is false:
`(3,2^11,1,1)@3` has a crossing-swap blocker with exactly the target band.  The dangerous tight
sets form a lattice; write `C` for their intersection and `U` for their union.  For each opposite-
color row `v in C`, provisionally flip it and intersect all sets that then violate the original
demand.  The **Core--Blocker Escape Conjecture**, now false universally, said that some flip is feasible, or one such blocker
intersection contains a smaller returning row `u in A-U`.  The swap `u<->v` would then be feasible for
both the original and transferred demand.  Positivity supplies an important exact simplification:
a set containing `u` but not `v` cannot block, because its post-swap rank is the old rank of
`T-u+v`, whose old demand is larger.  Thus do not construct an alternating `A`-heavy/`B`-heavy cut
chain; only the common intersection of the `A`-heavy flip blockers remains.  The prefix staircase,
(ST2), (CU), and (TB3) remain exact local calculus, but the `K=6` hole proves they cannot always
force an escape or a violated parent prefix.  Complete `K=3`, the first 5,000 `K=4` states and a
targeted `K=5` boundary sample support the statement only in those finite ranges.  Row-count slack and
two rules that preselect `v` from one separator have `K=4` counterexamples; closest-smaller `u` is
canonical only after the global core scan.  Exact formulas and counterexamples are in the
[boundary-exchange record](../evidence/singleton_boundary_exchange_2026-08-28.md).

The staircase is retained as local finite structure, not a proof obligation.  The quantifiers strengthen
at each step: the exact Row-Coloring/no-holes statement permits an arbitrary construction for each
state; global transfer closure permits an arbitrary new coloring at the next state; Adjacent-Fiber
requires a common coloring; Core--Blocker Escape requires a nearby common coloring from every
failed one.  The `K=6` hole refutes every universal statement in this hierarchy.

A whole-fiber experiment tested the global-transfer intuition without
making incompatible local choices.  Over each parent keep every normalized legal first-cut
allocation; over each Robin--Hood edge retain the literal links that move only the marked coin in
one fixed child.  At `K=3` this gives 1,063,464 allocation orbits and 26,135,976 links.  Every one
of the 8,916 parent edges lifts, and the component of the unique canonical `G_3` cut projects onto
all 1,206 parents, although 916 edges kill some source cuts and 320 individual cut orbits are not
canonically reachable.  The resulting **Canonical Allocation-Transport Conjecture** is false
universally because the `K=6` parent has no fiber.  The complete `K=3` result remains useful as a
finite phase classification.

The survey also supplies a proved model of a phase boundary rather than another guessed rule.  For
`U=2^(K-1)` and `w=U-K`, transfer the second canonical parent row to the third and reassociate the
four head allocations as

    (U,U,0),(0,U-1,U),(U-1,w,0),(0,w,U-1)
      -> (0,U,U),(0,U-1,U-1),(U,w,0),(U-1,w,0).

All three children remain `G_(K-1)`, but this cut has no literal predecessor: the original top two
rows are forced to opposite sides by a Hall inequality, while the new top row and shrunken donor
share a side.  Lorenz slack shows that `G_K` is the phase parent's unique one-unit predecessor.
Thus the full solution DAG has a genuine new source at every `K>=3`; trying to generate *every*
solution from `G_K` is false.  At `K=3` it is the only extra source, and its descendants together
with the canonical descendants cover the entire cut corpus.

The first requested `K=4` boundary survey is now complete rather than pending.  Measure a parent
by the integral area `D` between its Lorenz curve and `G_K`; Robin--Hood moves strictly increase
`D`, so `D<=B` is downward closed and source classifications there have no missing predecessors.
Through `D=14`, all 2,852 `K=4` parents, 26,067 edges and 871,752 allocation orbits still have only
the canonical source and the displayed reassociation source.  Every edge lifts and the canonical
component projects onto every parent.  The all-level first shell is proved more sharply.  At the
dyadic boundary `2^e`, one off-balance color costs the exact child-profile jump
`binomial(K-1,e-1)`; one unit of Lorenz slack can therefore create a new coloring only at `e=1`.
Every deeper immediate cover has four colorings, all inherited, while the top cover has two
inherited colorings plus the reassociation.  Complete allocation fibers through `K=5` refine this
to six inherited cuts at every deeper boundary and three inherited plus one new at the top.

The several-defect picture is now sharper, and the former “same reassociation at every collision”
target is superseded.  Fixed-color Hall feasibility is an integral polymatroid base.  Headward
exchange to a local maximum proves that every feasible coloring descends from a self-sorted
Pascal greedy shuffle with row sums `h_i+h_(color occurrence)` and three canonical children.
Consequently every possible coloring source is one of these finite rigid anchors.  Accumulated
slack can create higher phases: boundary `e` costs exactly `binomial(K-1,e-1)` and yields two new
sources once `K>=2e+1`.  At `K=5`, sources occur at areas `0,1,4,4,5,19`; an independent exact
ideal through area five has 267 parents, 866 nonempty edges and 5,089 colorings, and the canonical
component still projects onto all 267 parents.  Its cover-edge sub-DAG reaches only 266, so long
one-coin moves cannot be discarded.

The former directed target was the **Pascal-Shuffle Coverage Conjecture**:
the parent projections of the color-preserving downward exchange cones of all self-sorted shuffles
cover the full dominance ideal.  If transfers may run in either direction, source management is
unnecessary.  A fixed row coloring gives an integral polymatroid base set, every such set contains
a colored permutation of `G_K`, and base exchange connects every existing coloring to it.  Thus
the normalized Hall-coloring graph is universally connected after edges are made undirected.
Separately, the labelled union over colorings contains every vertex and its convex hull is already
the complete `G_K` permutahedron.  The `K=6` state is the missing integer lattice hole, so shuffle
coverage, no-holes and M-convexity are false.  Do **not** replace the lower-level facts by convexity of
the real fixed-color union: that strengthening fails for primitive Pascal profiles.  The padded
`K=2` point `(4,3,2/3,2/3,2/3)` is outside every fixed-color base, and even exact support fails at
`K=3` for `(8,7,4,(8/5)^5)`.  Flattening the final `m+1` rows gives such a hole at every `K>=3`,
and its forced defect exceeds one from `K=5`, so a universal real subunit-excess theorem is false
too.  The proved Integral Final-Band Extension Lemma remains a correct conditional statement: on
the tight `m-1` face, any legal head-band allocation leaves enough slots for the tail.  The new
counterexample shows that a legal head allocation need not exist.  Connectivity plus convex-hull coverage is insufficient, as the generic
`h=(6,1)` hole shows.  For the stronger
allocation route, the proposal that every first parent with empty `R(x)` has an incoming literal
link is now refuted.  Do not return to a
selected transfer tree: already at area two a cut born relative to the `G_4` edge is inherited
along another predecessor, and the `K=5` cover-only miss shows that even the graded cover graph
loses useful links.  See the
[solution-fiber DAG record](../evidence/singleton_solution_fiber_dag_2026-08-30.md) and the
[real-cover counterexamples](../evidence/singleton_exact_support_real_cover_2026-08-30.md).

The equivalent **Balanced Pascal Realization Conjecture** is likewise false; its formulation remains in the
[theorem note](theorems/singleton-majorization.md#an-equivalent-balanced-column-realization).
For `c=G_(K-1)'`, Gale--Ryser supplies a `0`--`1` matrix with row degrees `x` and paired column
degrees `(2c_t,c_t)`.  Choose the realization so that all degree-`2c_t` neighborhoods admit one
common bisection into `c_t+c_t`.  Pairing opposite sides inside each doubled column turns them into
prescribed-size matchings; the exact task is to choose degree-preserving switches and pairings so
their union is bipartite.  This puts the Pascal multiplicities into the construction from the
start and allows a global change of realization.  The `K=6` hole has no balanced realization in
the complete connected switch graph.

Repeated halving now has a proved complete search space.  Starting from any one Gale--Ryser
matrix and equal row bisection, arbitrary opposite-color row swaps and `2x2` incidence switches
reach every other matrix-and-bisection pair with the same margins.  Exhaustive switch-graph search
is therefore a universal exact constructor/refuter for one parent; it is not itself an existence
proof.  Use this graph for new surveys instead of inventing another irrevocable assignment rule.
The canonical matrix alone is false at `K=5` for
`(32,31,26,26,16^3,4^15,2^10)`, although one rank-1/rank-2 incidence switch repairs it.  Strict
defect descent alone is also false: `(32,31,26,26,16^3,9,6^9,2^2,1^13)` needs a neutral row swap
before a strict one.

The **Canonical Two-Move Pascal-Switch Conjecture** is false.  Build the
canonical Havel--Hakimi matrix, alternate its rows, and let `Phi` be the sum, rank by rank, of the
required number of smallest squared column imbalances.  The conjecture asserted that the walk always
has either a strict decrease or one neutral switch exposing a strict decrease.  The `K=6` hole has
no zero-energy vertex and refutes it.  The rule passes every one of the 408,776 exact-support `K=4`
parents, the first 500,000 `K=5` parents, separated `K=5` windows, and the recorded `K=5,6`
adversarial/walk probes.  These remain finite observations only.  Do not assume that every canonical column has discrepancy at most one, since an exact `K=5` counterexample has
discrepancy two in an allowed buffer column.  Details and reproduction are in the
[Pascal-switch record](../evidence/singleton_balanced_hh_switch_2026-08-29.md).

The first switch normalization is now proved.  Minimize the sum of squared doubled-column
imbalances over all realizations, row colorings, and doubled/single labels.  At parent rank `ell`,
there are `binomial(K,ell)` identical columns and only `binomial(K-1,ell)` need balance; the other
`binomial(K-1,ell-1)` may be declared single.  Therefore the selected doubled columns are exactly
those closest to half-and-half in their degree pool (BR0).  Two doubled columns of the same capacity then
have imbalances differing by at most one; otherwise an opposite-color `2x2` incidence switch
strictly improves the objective.  Across capacities `c,e`, their defects differ by at most
`max(1,|c-e|)`, so every capacity class has one defect sign.  If a doubled column of degree `2c`
has positive defect `d`, every
single column of degree `c` has all of its `B` rows nested inside that doubled column and contains
at least `d` `A` rows; reverse the colors for negative defect.  Work next at the level of the `K`
power-of-two capacity classes, not individual failed colorings.  Whole-row optimality adds (BR3):
the signed sum of incident defects is at most half the doubled incidence degree on an `A` row and
at least its negative on a `B` row; summing gives `2 sum delta^2<=sum c`.  Combine these row bounds
and forced nestings with
multiplicities `binomial(K-1,j)` and try to derive either a cross-capacity improving switch or a
violated parent prefix.  This is the smallest current obstruction on an equivalent global route.
The unique top-degree column can always be balanced by the Ryser reduction on the `2^K` largest
rows, while the degree-one rank has quota zero.  A rank-lexicographic contradiction may therefore
start with the first deficient internal rank `1<=ell<=K-1` and assume every larger-degree quota is
already exact.

Treat these quotas as Boolean coordinate deletion/contraction, not as helpful repetitions.  Label
rank-`ell` columns by the `ell`-subsets of `[K]`.  Exposing coordinate `t` requires exactly the
columns omitting `t` to balance and leaves the columns containing `t` unrestricted.  Equation
(CE1) shows the precise relaxation trap: maximizing over the color counts gives only
`H(p)+H(q)+H(p+q)` and then the known parent rank.  It loses precisely the integer column selection
that must be proved.

Do not infer Boolean shadow closure from these labels.  The Independent-Relabelling Lemma shows
that labels can be permuted independently inside every rank; coordinate exposure is exactly the
rank quotas and contains no further cross-rank incidence relation.  A shadow proof would first
have to construct and justify a coupled labelling from an augmenting search.  Until such a rule is
found, Kruskal--Katona/LYM is not an available conclusion.  Likewise do not freeze all recursive
exposures into one global bit address on the rows: the resulting Boolean-subcube model already
misses `G_2=(4,3,1,1)`, forcing `(4,2,2,1)` instead.  Later branches must be free to relabel their
rows independently.

The intrinsic deletion/contraction data--dyadic degrees, adjacent Pascal quotas, tight-band
identity (TB3), and switch-minimal relations (BR0)--(BR4)--remain valid descriptions of the
`K=6` obstruction.  They cannot force a violated parent prefix, because the counterexample
satisfies every parent prefix.

Do not spend another census on a rule that fixes donor and recipient to one side.  Exact method,
classification and the uniform construction are in the
[Adjacent-Fiber census](../evidence/singleton_adjacent_fiber_census_2026-08-27.md); local landscape
definitions and counterexamples are in the
[coloring-landscape record](../evidence/singleton_coloring_landscape_2026-08-28.md).

The formerly equivalent constructive formulation was the
[padded pure-first formulation](theorems/singleton-majorization.md#padded-pure-first-allocation-2026-08-27):
on `3^K` fixed labelled slots, place paired balanced supports for the two pure children so that the
residual stays under `G_(K-1)`, then let Gale--Ryser construct the mixed child.  Fixed alternating
intervals are too rigid already at `K=3` for `(8,7,4,1^8)`.  The `K=6` hole shows that an adaptive
insertion may also stop at an uncrossable tight cut, so this is no longer an existence target.

The equal-three-block view supplies a separate concrete subgoal.  Exchange always moves the
mixed-only block to the `3^(K-1)` smallest padded rows, which are zeros and ones.  For parents with
at least `2*3^(K-1)` nonzero rows, strict alternation of the remainder reduces every Hall inequality
to the explicit Pascal prefix family (P4) in the
[theorem note](theorems/singleton-majorization.md#equal-blocks-and-the-canonical-mixed-only-tail).
The exact breakpoint checker passes all `K<=12`, but (P4) is false at `K=19`; a full-mass
majorized state realizes the failure, so this is not merely looseness in the prefix bound.  Do not
try to prove (P4) or extrapolate its finite range.  Retain the exact canonical-tail contraction as
a scoped lemma; neither adaptive orientation nor Adjacent-Fiber can work universally after the
`K=6` final-transfer obstruction.

The 2026-08-27 forest-shape survey supplies a useful coarse invariant but not a replacement proof.
For `E_a(t)=sum_i(a_i-t)_+`, every cut has an exact removal profile
`sum_C E_C(t)=E_a(t)-J_a(t)` and the sharp capacity
`J_a(t)<=E_a(t)-E_a(2t)`.  Its first coordinate says to split approximately the same fraction of
the `2^(K-1)` top-layer joins as the fraction of all canonical joins retained by the parent, capped
by the number of nonunit rows.  Direct reconstruction verifies this scalar rule on all 5,997,038
`K=4` states.  Do not try to impose the analogous normalization independently at every threshold:
`(8,5,5,5,1^4)@3` refutes that stronger rule.  Use the hinge profile to describe augmenting-path
cuts and their obstructions; Pascal orthant saturation itself is now refuted; see the
[shape survey](../evidence/singleton_shape_survey_2026-08-27.md).
Likewise, do not require every row image to contain a piece whose width occurs in `G_(K-1)`:
`(8,7,4,2,2,2,1,1)@3` forces mixed mass at least ten under that rule, although the branch mass is
nine.  If atom widths remain useful, they need a weaker role, such as an aggregate resource or a
condition only on genuinely split rows, not a row-by-row requirement covering intact rows.

A second former global formulation was strong niceness.  For the
transcript graph `Q_K`, the monomial coefficient `c_K(lambda)` of its chromatic symmetric function
counts semi-ordered legal decompositions of type `lambda`.  Exact recursive counting shows that
`Q_3` is **strongly nice**: `c_K(mu)>=c_K(lambda)` whenever `mu<=lambda`, for all 1,206 supported
types and 463,886 comparable pairs.  The `K=6` missing type proves that `Q_6` is not nice and hence
not strongly nice.  The
graph recursion is `Q_K=Q_(K-1) disjoint-union (Q_(K-1) join Q_(K-1))`; disjoint union preserves
strong niceness but graph join does not in general.  Even the special combined operator does not
preserve support niceness for a generic recursively generated seed: `T(E_3)` is nice, whereas
`T(T(E_3))` omits the dominated type `(12,9,2,2,2)`.  The recurrence and finite `Q_3` theorem
remain useful negative-result context.  See the
[theorem formulation](theorems/singleton-majorization.md#a-stronger-chromatic-symmetric-function-target-refuted-at-k6)
the [exact census](../evidence/singleton_strong_niceness_2026-08-27.md), and the
[operator counterexample](../evidence/singleton_balanced_hh_switch_2026-08-29.md#three-other-global-shortcuts-rejected).

The complete feasible split-count fiber is less promising as an induction variable.  At `K=3` it
is always an integer interval, but continuous hinge and mixed-mass endpoint bounds miss parity and
pure-side packing obstructions.  Computing all endpoints did not finish even a ten-state `K=4`
sample in 60 CPU seconds.  Retain the endpoint examples as diagnostics only.

The complete full-mass census gives supporting geometry: all 5,997,038 states at `K=4` satisfy the
Row-Coloring property, and a balanced value-block rule with one-block lookahead succeeds on every one,
plus ten million uniform `K=5` samples.  Treat that rule as diagnostic for the exchange geometry,
not as the primary theorem: even the simpler global rule that fixes the necessary row counts and
then minimizes total-mass difference fails after 15,855 `K=4` states.  Exact data and counterexamples are in
[the census record](../evidence/singleton_row_coloring_census_2026-08-26.md).  Strict alternation,
lower-load assignment, local safe-next assignment, fixed half-mixed reservation, two-class
exchange, adjacent-pair orientation, global scalar balance, and even the broader class of all
`2`-flat bases have explicit counterexamples.  Finite or sampled success must not be promoted over
the exact `K=6` refutation.

**Delivered 2026-08-09.** The useful child-profile feature was not merely an ordering score. The
[Vertex-Splitting Pullback Lemma](theorems/singleton-majorization.md#vertex-splitting-pullback-lemma-2026-08-09)
proves that every part `(n:m)`, `n>=m`, may be lifted to `m` disjoint singleton stars `(n:1)`.
Therefore the mass-preserving block profile of every solvable state must be weakly majorized by
`G_k`. On an exact-L1 miss, `radiobase.c` applies this sound filter before its dominance-trie
lookup; `radio_verify.c` and `tools/certify.py` implement it independently.

This supersedes the value-order experiment. Against the same warm lower cache, the hard positive
`Sb(15:3,14:3,17:2,8:4,11:2,10:2,19:1,15:1)` in 5 moved from a 300-second timeout to 5.3 CPU seconds.
The exact A+B negative monster is refuted at the root by `714 > 705` at prefix 40 of its expanded
profile, so its previous 237.4-second / 8.1-billion-candidate search is no longer a search problem.
Broad replay and independent-verifier results are in the latest [journal entry](journal.md).

Reproduce the two controls after compiling `radio_one.c` at the stated bounds:

```
tools/capped_run.sh --seconds 300 --rss-gb 8 --label star-hard -- \
  ./radio_one /tmp/k6lab/warm_k4.txt 5 15 3 14 3 17 2 8 4 11 2 10 2 19 1 15 1
tools/capped_run.sh --seconds 300 --rss-gb 8 --label star-monster -- \
  ./radio_one /tmp/k6lab/warm_k5.txt 6 18 8 22 6 15 8 13 9 23 4 23 2 21 2 17 2
```

The old pair/triple/quad deployment remains rejected: its information is already present in the warm
upward-closed prefix cache. The full star filter is different—it is a global arbitrary-part-count
theorem and directly proves 40.6% of the recorded k=5 negatives.

**The synchronized predicates are now characterised, but not deployed.** Define `R_0` as full-star
majorization and `R_d` by requiring one legal synchronized split whose three children pass
`R_{d-1}`. Each condition is necessary and `R_k` is exact solvability; the previously claimed
nesting between adjacent depths is unproved. The first level is
an additive hinge-capacity problem. The full theorem, a worked `Sb(16:1,12:2)` ladder, and a
width-two counterexample to any single-base majorization rule are in
[the theorem note](theorems/singleton-majorization.md#the-synchronized-majorization-predicates-corrected-2026-08-26).

Do not add `R_1` as a production pre-pass. The current recursive prefix checks already enforce its
child inequalities, and on the residual four-part positive the first `R_1` witness is dead and
still passes `R_2`; mere feasibility does not distinguish it from the true winning split. Direct
deeper checks also lose to the warmed exact cache on the tested residual negative.

One theorem-level use *is* now deployed in split-table construction (2026-08-10): each local cut
whose one-part child already fails the information bound or `R_0` is omitted from that level's
table. By subgraph monotonicity no completion with the other parts could rescue it. This is the
same cheap necessary test the recursive loop would eventually perform, not an `R_1` search.
Tables are coarse-grained lazy per `(k,sbb)`, exact-sized and contiguous; suffix parts allocate no
table until search reaches them. On the fixed warm positive control, the identical winning split
and top-level counter were found in 32 rather than 43 solver seconds (49.91 rather than 62.05 wall
seconds including cache load), while the warm negative stayed at 0.08-0.09 solver seconds. The
reproducible regression and allocation measurements are in the 2026-08-10 journal entry.

If P6 continues, the ordering benchmark remains
`Sb(29:6,19:9,13:12,36:3)` in 6 and the only promising use is a **bounded, fallback-safe ordering
signal** that approximates deeper synchronization without recursively solving it.  The current
exact-L1 baseline is 26.6 solve seconds with 37,899 top-level splits; compare both numbers, because
a cache-only speed change and an ordering improvement answer different questions.

**Recursive Pareto-lift track (2026-08-12).** A second bounded ordering signal now has a proved
geometric core.  If a lower aligned template `T` uses cut `s`, every lineage-preserving parent cut
lies in the box `s <= X <= s+(P-T)`; its four rectangles componentwise contain the corresponding
lower rectangles.  Searching `L1` shells around the proportional lift and preserving the lower
split's outcome proportions found a new solution of the four-part k=7 control at structural rank 5
in 15 wall seconds, versus 65 wall / 57 solver seconds for ordinary search with the same warm cache.
An adjacent lower point also succeeds, but only under the correct inherited assignment of two equal
components.  See [the theorem note](theorems/recursive-pareto-lift.md) and the fully provenanced
`pareto-lift-2026-08-12` artifacts.

This is not ready for `radiobase.c`.  A literal second recursive step exhausted the complete lower
split's lift box, and a greedy componentwise Pareto upgrade followed by its first solving split also
failed.  The k=7 and k=8 exhaustive choice corpora are complete and archived; no AWS remainder is
running. Analyze the retained antichain of fixed-dimension Pareto upgrades and inequivalent
solving splits for a lineage-preserving choice property.  Only after that corpus is understood
should the minimal production form be tried: a transient lineage-aware hint, one or two templates,
a bounded pass, and unchanged fallback.  Split hints must influence order only; they are not cache
facts.  Use `tools/pareto_census_status.sh` for a one-shot progress query.

**Separate theoretical m=6 thread (corrected 2026-08-26).** The first large-`k` upper bound is
`n(10,6)<=973`, from an exact rejection at 974.  The singleton-majorized tree at 973 used the now
refuted converse and does not prove achievability.  The upper bound already refutes both the closed form and `BBCD` profile, which
predict 976.  The separate finite recurrence, when fed the corrected `n(9,5)=481`, predicts
977 and misses by four.  The working root is `[477:2]`, with mixed child
`Sb(496:2,477:4)@9`; it avoids the `Z_7` kernel rather than repairing it.  The separate aligned
atom track now supplies only a relaxed structural candidate
`2^k-k^2+7k-21` for `k>=12`; it is not an achievability construction or a fit to this stored
subtree.  Next:

1. find an unconditional 973 construction by exact/canonical terminals, then
   classify working root splits and express the `m=4 + m=2` mixed-child boundary in deficit
   coordinates;
2. decide the proposed one-level lift, whose only new hard child is
   `Sb(503:1,495:2,478:3)@9` (the first five-minute run was inconclusive, and the literal scaled
   continuation is refuted by the exact negative `Sb(247:1,247:1,240:2,231:2)@8`);
3. compare the unrestricted two-bundle frontier with the relaxed aligned candidate, then recover the
   low-`k` degeneracies backwards or find an upper obstruction.

The new formula predicts 1983 at `k=11`, but its checked hard-tree threshold starts at `k=12`, so it
does not decide item 2.  Do not infer either a constant `-3` correction to the old closed form or
a constant `-4` correction to the finite recurrence from one value.  Reproduction,
correctness argument and costs are in the latest journal entry and `docs/tools.md`.

**Relaxed excess-q diagnostic track (parked 2026-08-16).** This is retained as a completed
investigation, not an active work queue.  Under the explicitly unproved working assumption that
every fixed labelled A/B/C/D pattern eventually reaches a stable atomic-leaf regime,
the corrected diagram gives a concrete branch rather than a missing map.  For
`A=(a:alpha)@k-1` and `B=(b:beta),C=(c:gamma)@k-2`, maximize `d` in
`Sb(d:beta,b:alpha-beta,c:m-alpha-gamma,a-c:gamma)@k-2`, then score the candidate `a+b+d`.
The known-answer `m=5` algebraic calibration is complete, but its arbitrary-majorized terminal
derivation is unsupported.  Enumeration within that terminal model chooses `(3,2,2)` through
parent `k=7`, ties it with `(4,3,1)` at `k=8`, and switches to `(4,3,1)` at `k=9`.  For the latter
family, the local program proposes the D frontier
`Sb(d:3,(2^t-t):1,(2^t-2t):1)@t`: its candidate value and unconditional upper ceiling are
`2^t-binomial(t-2,2)` at `t=7,8` and one larger from `t=9`.  The latter has a direct two-test
reduction to singleton-majorized leaves, whose decisive prefix inequality first holds at `t=9`.
Inside the now-invalid blanket-majorization terminal model, this suggested that a scalable state must
store a guarded family envelope, not one atom word or one preferred height triple.  See
[the exact calibration](theorems/m5-pareto-assembly.md).
For the stronger atom interpretation, the first eventual decisive leaf
`(127,119,119,118,111)@7` has a committed exact tree at depth three and exhaustive exact/embedded
failure through depth two.  A uniform exact depth needs at least six tests asymptotically, but the
depth-six identities are only scalar inventories, not a common synchronized packing.  Do not reopen
the parked track merely to search larger finite instances of that packing.
The direct finite mode recovers the proven `m=10` widths at parent levels 5 through 7 and shows why
all Pareto triples must be retained: the repeated `(6,4,3)` height choice falls one short at level 7,
while B-height 5 reaches the frontier.  That complete enumerator now exists: at each requested `m`
it validates the proven Pareto input levels, exhausts ordered triples, emits a complete full-star
ranking, and exact-solves all candidates still able to tie its incumbent.  It closes the `m=10`
working family through parent level 7; the level-8 ranking completes but its exact optimization does
not.  If the track is ever reopened, the missing finite capability is an exact resumable/per-slice
schedule so one hard level-8 query cannot block cheaper later triples; the missing symbolic object
is a proved refinement law for the guarded mixed-frontier pieces.  The completed atom-induction
exploration proceeded by height: A/B/C contribute only their already constructed outer profiles,
while the four-segment D state is the sole synchronized obligation.
The calculation closes only in the relaxed majorized-terminal model through the `AACC` and `BBBD`
profiles and first fails at the
known height-6 `k=10` transition, where refinement changes the candidate D accounting from
`AAAABBCD` (232) to `ABBBBBCD` (229).  The literal refinement of the successful split is now
refuted at `k=11`; do not retry it.  The stronger symbolic result now rules out the entire one-D
refinement class: following mixed outcomes preserves height and cannot branch a D lineage, so all
eight-atom ranks 1--81 are all-depth negative.  Rank 82, `A^6D^2`, has a checked three-level relaxed
tree and is the exact A--D eight-atom optimum only in that model, yielding an unsupported parent derivation
`2^k-k^2+6k-16` from `k>=17`.  Deeper search for the finite 229 accounting was thereby superseded.
At 16 atoms the
D-lineage certificate excludes ranks 1--289, and the new 242-core `(D,C+D)` coinductive kernel
excludes ranks 290--304 at every depth.  The first projected rank-305 tree has no exact lift, but the
product search over alternative projected splits finds a checked 19-node relaxed tree for
`A^13CD^2`.  Thus rank 305 is the exact sixteen-atom optimum in that model and yields the unsupported parent
profile `A^49B^9C^4D^2@G[k-6]`, equivalently `A^7B^7D^2@G[k-4]`, with width
`2^k-k^2+7k-21` from `k>=12`.  At 32 atoms, lineage and a new independently checked 504-core
`(D,C+D)` kernel exclude ranks 1--1179 at every depth.  Rank 1181, `A^26BC^3D^2`, is feasible in
the relaxed model by refining the rank-305 tree, so the only unresolved profile in this slice is the wider rank 1180,
`A^27C^3D^2`.  The propagated-loss bound makes the exact depth-three product exhaustive and
negative.  At depth four its first mixed transition must have
`ell_D=ell_V=0` and `1<=ell_W<=14`.  Exact pure-outcome solution leaves 6,696 oriented first tests
and 1,818 distinct mixed children in those fourteen classes; loss-sliced guided cover exhausts all
of them, proving that no aligned tree of depth at most four exists.  Inside this fixed height-6
slice, the remaining symbolic question is singular but genuinely infinite-depth: a
three-coordinate closed kernel or W-indexed recurrence would have to decide rank 1180, or an exact
tree would need depth at least five.  It is parked with the rest of the track; moving the slice
optimizer to 64 atoms before that decision would add data without resolving the structural gap.
Separately, before any slice formula is promoted to the unrestricted `m=6` frontier, enumerate and
compare the competing outer families as in the `m=5` calibration.  The general candidate
`A^(N-b-c-2)B^bC^cD^2`, `N=2^s`, has parent width
`2^k-k^2+(2s-c)k-s^2-3s+c(s+1)-b+2`; use this formula to compare slices, and keep the
postulated `s=5,b=0,c=3` value `2^k-k^2+7k-20` explicitly separate from the checked `b=1`
construction.  This carries no inner A/B/C
witness data, changes no Pareto datum, and remains conditional on the outer assembly; definitions,
proof and controls are in
[the atom-lineage note](theorems/atom-lineage.md) and
[conjectures.md](conjectures.md#excess-q-pareto-assembly-as-a-variable-d-slice-working-hypothesis-2026-08-14).

Reopen this track only if new mathematics addresses at least one of its missing global links: a
complete outer-family theorem, a refinement-stable exact description of the synchronized D
antichain, or an all-depth construction/obstruction that connects successive normalizations.  A
new finite winner, another bounded-depth exclusion, or a fitted atom word is not enough.

### P7 - Compile and independently replay the `Sa(193)` certificate

The small-corpus gate is delivered. `radio_verify.c` now accepts and emits a strict readable text
certificate, distinguishes roots from support facts, minimalizes support levels before coloring,
and verifies all levels concurrently with pthread workers. On `fullsolve-2026:out_k7.txt`, one
through sixteen workers produced the identical 62,366 verdicts and 97,483,464-node enumeration;
wall time fell from 14.13 to 2.79 seconds. A one-root colored certificate retained 373 support facts
and replayed cleanly. Details and hashes:
[`../evidence/radio_verify_parallel_2026-08-16.txt`](../evidence/radio_verify_parallel_2026-08-16.txt).

The intermediate full-pipeline performance benchmark is delivered. Explicit Sa(66) roots produced
a 2,037-record colored bundle at every tested width. Explicit Sa(113) roots reduced 304,105
normalized facts to 9 roots plus 120,528 support facts, and the independent checker reported all
120,537 records closed in 2,491,817,467 recursion nodes. A later solver-core replay exposed nine
uncovered splits in the colored subset while closing the full normalized corpus, so this is no
longer a proof result. On the same `r7iz.4xlarge` hardware and exact run9
verifier binary, 8/14/16 workers replayed in 434.86/369.57/347.91 seconds. This establishes the
small-corpus scaling result: sixteen is minimum wall on an idle host, eight is CPU-efficient, and
fourteen is a sound shared-host compromise. The dominant Sa(113) k=6 batch retained 92.91% of its
minimal level. Coloring can reduce a completed artifact, but this benchmark did not establish that
independent proof search remains cheaper than the solver at run9 scale. Full evidence:
[`../evidence/verifier_pipeline_benchmark_2026-08-17.txt`](../evidence/verifier_pipeline_benchmark_2026-08-17.txt).

Disposition and next design work:

1. **Delivered:** normalize run9 to `radio-negative-certificate-v1` and round-trip it byte-for-byte;
   a bounded top pass also verifies the sixteen explicit roots and shows that they cite all 2,545
   canonical `k=8` facts, but deliberately verifies nothing below `k=9`;
2. **Delivered on AWS:** minimalize every support level. The dominant `k=7` level retained
   2,507,270 of 2,576,885 facts (97.30%), so same-level reduction is modest on this corpus;
3. **Stopped and retired (independent checker):** both original full-run9 coloring attempts were interrupted by request on
   2026-08-18. The instrumented sixteen-core run spent 11,460.1 seconds in k=7 coloring and closed
   119,649/2,505,858 targets, including all 108,083 three-part targets but only 10,312/2,396,521
   four-part targets. It had consumed 105,605,161,144 recursion nodes with zero unresolved/budget
   outcomes; its final window rate was 0.450 target/s. The old fourteen-worker run spent about
   12h28m in coloring and had only whole-level progress. Neither produced a colored bundle or replay
   result. Final diagnostic uploads were hash-checked and the dedicated instance was terminated.
   This finding applies to `radio_verify.c` rediscovering proofs; item 8 records the later
   production-trie citation coloring which does not repeat that implementation;
4. **Delivered:** the packed product-profile index retains canonical fact storage but scans a
   separate `(np,max-product,total-mass)` permutation with denormalized mass and packed n/m/product
   columns. On an exact hard run9 k=7 root it preserves the 4,644,469-node proof and memo counts
   while reducing verifier wall from 209.63 to 33.24 seconds (6.31x). A complete 120,302-record
   Sa(113) colored replay also reported zero gaps; retain that only as a performance measurement.
   Measurements, build IDs and controls are in
   [`../evidence/verifier_product_index_2026-08-17.txt`](../evidence/verifier_product_index_2026-08-17.txt);
5. **Delivered:** adaptive fixed-size block summaries over the packed index. Full 256-fact blocks
   inside an equal primary-key group retain the Pareto-minimal `(mass,top-four products)` profiles;
   levels below 65,536 facts take the product-only loop. The exact hard root keeps all proof/memo
   counts while falling from a contemporaneous 39.16 to 11.70 seconds (3.35x), and the small-level
   Sa(113) guard is neutral. The failed ungated layout, size sweep and sanitizer controls are in
   [`../evidence/verifier_block_pareto_2026-08-17.txt`](../evidence/verifier_block_pareto_2026-08-17.txt);
6. **Delivered:** an immutable kd hierarchy over the packed necessary profiles, reused for ordinary
   dominance lookup and same-level minimalization. Every fitting 32-fact leaf still reaches exact
   injection matching. Together with bounded 512-option pairwise forward checking, the five-root
   k=7 control fell from 21.00 seconds / 9,158,686 nodes to 5.34 seconds / 4,690,828 nodes. Complete
   k=6 and k=7 antichain passes reproduce 229,341 and 2,507,270 minima in 3.8 and 49.8 seconds.
   A subsequent descending-segment-mass part order reduced a twenty-root level-spread sample from
   41,945,991 to 5,336,038 nodes and a complete Sa(113) replay from 2,491,283,058 to 330,226,371
   nodes, with all 120,302 colored records reported closed. The solver-core discrepancy supersedes
   that verdict without changing the traversal-cost comparison.
   Soundness, memory caps and the Sa(113) guard are in
   [`../evidence/verifier_kd_index_2026-08-18.txt`](../evidence/verifier_kd_index_2026-08-18.txt);
7. **The bounded independent run9 audit answered its engineering question and was stopped.** The
   mass-descending sample fell to 3,197,377,218 nodes / 341.32 seconds, but the full phase still
   repeated more work than the solver. It was stopped at 251,131/2,576,885 k=7 claims after 2,160
   seconds and 48,049,145,431 nodes, with zero gaps; final hashes were checked and its host was
   terminated. The replacement `radio_refute.c` freezes the production negative trie and split
   metadata, then runs the solver's exhaustive traversal in parallel with root-cache bypass,
   CACHE_ONLY k-1 children and no recursive solving or writes. Its AWS gate rejects the full run if
   either projected wall exceeds 24 hours or projected CPU exceeds the complete 419,353.1-second
   cold solver. The gate closed 9,995/9,995 with zero gaps in 81.200 wall / 1,293.979 CPU seconds,
   projecting 19,488 wall / 310,555 CPU seconds. The full replay then verified all 3,126,190 claims
   with zero gaps across retained k=7, k<=6 and k=8..9 checkpoints. Its capped phases took 20,845
   wall seconds; worker epochs used 318,771.171 CPU seconds, 76.015% of the complete solver, and
   peak RSS was 1.24 GB. The exact payload is release-verified under
   [`sa193-frozen-refute-2026-08-18`](https://github.com/fedork/radio-data/releases/tag/sa193-frozen-refute-2026-08-18).
   It uses the complete normalized certificate because the colored Sa(113) control is missing nine required supports.
   Architecture, controls and exact measurements are in
   [`../evidence/verifier_frozen_trie_2026-08-18.txt`](../evidence/verifier_frozen_trie_2026-08-18.txt);
8. **Delivered; both uncolored and top-down colored replays completed and were archived.** Readable text remains the durable envelope,
   now as one self-contained file per level. `radio-negative-level-certificate-v2` orders an
   explicit part dictionary, complete k-1 support, checked split-part hints and level-k claims;
   only support enters the trie. The run9 k7 file reduces cache construction locally from 263.457
   to about 3.1 seconds and compact claim/support storage from roughly 404 MB of fixed records to
   about 47 MB. Explicit split options remain derived: all 383,875 full-k7 local options previously
   took only 0.112 seconds, while trusting a supplied subset would introduce a completeness hole.
   Full-star majorization now checks only the mathematically sufficient endpoint of each equal-star
   run. Two matched controls save 28.69% worker CPU, and compiling exact L1 out of the frozen
   verifier saves another 11.29%; the mutable solver retains L1. The final local 9,995-root k7 gate
   closes with zero gaps in 79.672 wall / 934.528 CPU seconds and projects 5.30 local wall hours for
   the four-part band. Details:
   [`../evidence/verifier_level_v2_2026-08-18.txt`](../evidence/verifier_level_v2_2026-08-18.txt).

   The same-type gate passed and complete uncolored run `20260818T194508Z` is in its dominant k7
   phase. Coloring is a separate compile-time mode, not a change to that baseline. Every surviving
   negative Pareto terminal carries its original support-record index; eager split preparation and
   each audit worker record citations in private bitsets, merged only after zero gaps. The readable
   selection is checked against the complete source index and copied state before it can generate
   the next level's file, and the chain stops only at `used 0`. A final-source local A/B on the
   same 9,995 roots kept all 13,403,862,290 prefixes and zero gaps; coloring added 3.61% wall and
   4.07% CPU. ASan+UBSan exercised all 388,317 support insertions and front growth, and a two-worker
   TSan control was clean. Details:
   [`../evidence/verifier_coloring_citations_2026-08-18.txt`](../evidence/verifier_coloring_citations_2026-08-18.txt).

   Dedicated on-demand run `20260818T205010Z` passed its sixteen-worker AWS gate in 55.977 wall /
   891.641 CPU seconds, projecting 13,434 wall / 213,994 CPU seconds for the full four-part band.
   Its first durable top-down checkpoint verified all sixteen k9 roots and selected 2,151/2,545 k8
   facts. K8 verified those 2,151 claims with zero gaps, selected 2,508,278/2,576,885 k7 facts and
   checkpointed them before the sixteen-worker k7 phase began. Both full chains later closed with
   zero gaps, their manifests and release round trips were verified, and their hosts were
   terminated. A compact global exact hash remains deferred: neither the 34.7%
   theorem profile nor solver order demonstrates enough first-touch exact hits to justify another
   lookup. Separately make the solver emit split-space coverage if an independently cheap verifier
   is required: compact ranges/subboxes annotated with the outcome and exact lower-fact/theorem
   citation, so checking scales with the cover instead of rediscovering it.

The first parallel-solver prerequisite is delivered. `canSolveB_ctx` carries one explicit search
context through the complete recursive tree; its deterministic work clock, exact L1 and joint
reachability workspace are worker-owned. The legacy entry point wraps a default context, the
1,038-answer serial corpus is byte-identical, both budget schedulers pass, and sanitizer coverage is
clean. Full commands and hashes are in
[`../evidence/parallel_solver_context_2026-08-17.txt`](../evidence/parallel_solver_context_2026-08-17.txt).
`radio_refute.c` now exercises a deliberately narrower thread-safe frozen epoch: cache and split
metadata are completed serially, worker roots cannot learn, and allocation counts plus a split
checksum prove that the read epoch stayed immutable. This is still not a thread-safe mutable
solver: the dominance trie, lazy split catalog with learned cut metadata, and minimum-`k` memo
remain process-global outside that mode.

Next, introduce a frozen read-only result-cache view plus worker-local exact-result overlay, then
separate immutable split geometry from mutable cut metadata. Only after that gate should the
exhaustive tail become a limited-width queue of coarse split-prefix batches. Keep the cheap
heuristic pass depth first; use per-`k` immutable cache epochs plus short exact-result publication,
not read/write locks held across recursion. The mutable solver already canonicalizes each segment
by mass then long side. Putting length and total mass ahead of that is most credible in the frozen
epoch: otherwise positive and negative monotonicity require opposite range searches or eager
duplication across buckets. Keep the deployed last-segment Pareto fronts as the mutable baseline,
and denormalize hot exact answers or read-only indexes rather than a full implied-fact closure. The
ownership and publication contract is in [parallel-solver.md](parallel-solver.md); a language
rewrite is not the next experiment.

## Ordering

P1 first and quickly. The 2023 corpus spent months of compute and, until 2026-08-02, existed
in exactly one place: a zip on one disk. It is unreliable but not worthless—it records what
was attempted and provides historical cost context. New claims use the retained 2026 artifacts.

Then P5 and P2 in parallel - P5 is writing, P2 is compute, so they do not contend. P3 follows
P2, reusing the same tooling and the same feel for which `target_k` values work. P4 is now
more a costing exercise than a plan.

The proof-safe cold run9 and retained run3/run8 baselines have all completed. Run8 is the matched
performance comparator; only run9 is the negative proof source. Run9 cost 419353.1 CPU seconds and
1.32 GB peak RSS, 1.646% more CPU than run8 and 12.456% less than run3. The bounded ordering
approximation is therefore measured end to end; no further split-history experiment is needed for
H3.
The separate recursive Pareto-lift corpus may still be analyzed as a bounded solver-ordering
question after its k=8 remainder finishes.  The excess-`q` profile branch—including rank 1180,
larger normalizations, and attempts to extract a global formula from the A/B/C/D assembly—is parked.
Its positive trees, all-depth kernels, depth-four cover, and exact `m=5` calibration remain durable
evidence; they are not an instruction to continue the scan.  The reopening criteria are recorded
in the parked-track section above.

The result-cache prerequisite for H3 was delivered. Last-segment positive/negative Pareto fronts
reduce the measured k=5..7 checkpoint storage 11.2x; with the exact-state L1, the full `Sa(192)`
control passes at 0.35 GB peak RSS and 711.7 CPU seconds, with no remaining measured premium over the
734.5-second pre-compaction control. AWS `run7` and the same-chain local continuation used the
now-obsolete `e648e83` progress-gated pass-2 scheduler and were retired on 2026-08-11 after their raw
segments and closed checkpoints were preserved. Run3 finished as the untouched historical
incumbent, run8 as the cold `9395218` bounded-probe baseline, and run9 as the cold `e7fa747`
proof run. Neither `c13b5d3` nor `e648e83` is a valid performance baseline for the new scheduler.
The durable operational rule remains: resume only from a run's own output and retain every raw
segment; compact does not mean bounded.
