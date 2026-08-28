# Research plan

Living document. Update it when a goal is met or reprioritised; do not accumulate stale
entries. Last revised 2026-08-26.

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
conditional excess-`q` Pareto-assembly programme is **parked as of 2026-08-16**: retain its proved
local reductions and conditional constructions, but do not treat further normalization/rank scans
as a proximate goal.  The literature now settles `m=5`: the old formula/`BBBD` equality is refuted
at `k=9`, and the exact
piecewise theorem changes the root from `3+2` to `4+1`.  The corrected Pareto assembly finds the
crossing, but its arbitrary singleton-majorization terminals are conditional on the open converse;
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
sixteen-atom aligned tree now gives the different conditional continuation
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

### P6 - Full star expansion and synchronized majorization for long states

**Foundational singleton subproblem (corrected 2026-08-26).**  Weak majorization by `G_K` is
proved necessary, but its converse is open.  The exact target is the Row-Coloring Lemma (C) in
[the theorem note](theorems/singleton-majorization.md#the-exact-remaining-row-coloring-lemma).
The cleanest bottom-up target is the Pascal Orthant-Saturation Lemma: the magnitudes of the integer
points in the signed Hall bisubmodular polyhedron should contain every lattice point of their convex
hull, which is exactly the parent majorization polymatroid.  Equivalently, prove that every unit
Robin-Hood transfer can be routed by a global signed augmenting path through the doubled
Pascal/dyadic columns.  This formulation and the convex-hull equality are proved in the
[theorem note](theorems/singleton-majorization.md#global-signed-lifting-and-the-no-holes-target-2026-08-26);
only the no-lattice-holes/transfer step remains open.

The exact Adjacent-Fiber census now passes every one of 141,690,676 normalized transfers from all
5,997,038 full-mass `K=4` states.  For `K<=3`, the globally optimal separator margin is always at
least two, and every `K=4` transfer has a certificate of margin at least two.  Use this as evidence
for the transfer target, not as a theorem.  The same-color
strengthening is already false for 889 `K=4` transfer types, so the proof must handle a genuinely
opposite-color tight separator.  The next useful theoretical step is to characterize the minimal
separator exposed after maximizing its slack and show that Pascal-identical columns cross it.  The
889 cases are now exactly one four-row family, and its `K-3`-member continuation at every `K>=4`
has an explicit common crossing allocation: two mixed contributions move toward one another under
the identical pair `U-K,U-K`, with exact separator margin equal to their original gap.  Generalize
this proved local reroute into a block-descending augmenting path.  The new Tight Pascal-Band Lemma
shows that every actual dangerous separator `(p,q)` saturates every dyadic child column and obeys

`#{T:p<c(T)<q} >= 2 + #{T:c(T)=p+q}`.

Thus every obstruction crosses a repeated internal Boolean rank; this is now a theorem, not only
the pattern seen in the 889 cases.  A fixed-color tight cut cannot be crossed while preserving row
orientations--that would contradict the Fixed-Color Hall Lemma.  The corrected target optimizes
over feasible colorings of the original state and uses a global color-exchange augmenting chain to
raise the separating margin.  After quotienting equal-row identities and global side
complementation, every failed coloring in the exact `K<=3` landscape and the first 10,000 `K=4`
states has a successful coloring within two row changes.  Three disjoint 1,000-state windows after
1M, 3M and 5M enumerated states give the same result.  This motivates the Two-Row
Color-Exchange Lemma; more precisely, every observed repair is one row flip or one opposite-color
swap.  Encode a coloring as the fixed-absolute signed vector `z_i=+/-a_i`.  The ambient Hall
polyhedron is integral bisubmodular and therefore has support-one-or-two delta exchanges, but its
fixed-absolute-value slice does not automatically inherit them.  Prove that the repeated internal
rank from the Tight Pascal-Band Lemma compresses an ambient unit-exchange chain to a boundary flip
or swap that makes the transfer margin positive.  The Boolean labels alone do not supply that
exchange, and nested/laminar supports are false already at `K=3`.  The fixed-absolute coloring
family itself is not a delta-matroid: `(3,2^11,1,1)@3` has an explicit symmetric-exchange failure.
Use the exact boundary calculus instead.  A failed `B->A` flip is certified by an `A`-heavy cut;
a failed crossing swap has one blocker family of each imbalance; and every successful local repair
must cross the original `B`-heavy tight separator.  The remaining proof is therefore termination
of this alternating cut chain by strict descent of its dyadic band.  Row-count slack and two
closest-boundary prescriptions both have `K=4` counterexamples, so do not preselect the exchanged
identities.  Exact formulas and counterexamples are in the
[boundary-exchange record](../evidence/singleton_boundary_exchange_2026-08-28.md).
Do not spend another census on a rule that fixes donor and recipient to one side.  Exact method,
classification and the uniform construction are in the
[Adjacent-Fiber census](../evidence/singleton_adjacent_fiber_census_2026-08-27.md); local landscape
definitions and counterexamples are in the
[coloring-landscape record](../evidence/singleton_coloring_landscape_2026-08-28.md).

For a constructive attack, use the equivalent
[padded pure-first formulation](theorems/singleton-majorization.md#padded-pure-first-allocation-2026-08-27):
on `3^K` fixed labelled slots, place paired balanced supports for the two pure children so that the
residual stays under `G_(K-1)`, then let Gale--Ryser construct the mixed child.  Fixed alternating
intervals are too rigid already at `K=3` for `(8,7,4,1^8)`.  The live algorithmic target is an
adaptive decreasing-capacity insertion with alternating-path reroutes; prove that every stopped
insertion's opposite-orientation tight cut can be crossed using the Pascal multiplicities.

The equal-three-block view supplies a separate concrete subgoal.  Exchange always moves the
mixed-only block to the `3^(K-1)` smallest padded rows, which are zeros and ones.  For parents with
at least `2*3^(K-1)` nonzero rows, strict alternation of the remainder reduces every Hall inequality
to the explicit Pascal prefix family (P4) in the
[theorem note](theorems/singleton-majorization.md#equal-blocks-and-the-canonical-mixed-only-tail).
The exact breakpoint checker passes all `K<=12`, but (P4) is false at `K=19`; a full-mass
majorized state realizes the failure, so this is not merely looseness in the prefix bound.  Do not
try to prove (P4) or extrapolate its finite range.  Retain the exact canonical-tail contraction,
then seek an adaptive orientation.  For adjacent pairs, orienting pair differences gives a signed
prefix walk `D_n`; Hall becomes a family of interval bounds on `D_p-D_q`.  Prove those bounds by an
augmenting-path/separator argument, or attack the equivalent bottom-up Pascal Adjacent-Fiber Lemma,
where an opposite-color tight separator is the only obstruction to preserving a coloring under one
unit Robin-Hood transfer.  Do not substitute another fixed block heuristic for either exact target.

The 2026-08-27 forest-shape survey supplies a useful coarse invariant but not a replacement proof.
For `E_a(t)=sum_i(a_i-t)_+`, every cut has an exact removal profile
`sum_C E_C(t)=E_a(t)-J_a(t)` and the sharp capacity
`J_a(t)<=E_a(t)-E_a(2t)`.  Its first coordinate says to split approximately the same fraction of
the `2^(K-1)` top-layer joins as the fraction of all canonical joins retained by the parent, capped
by the number of nonunit rows.  Direct reconstruction verifies this scalar rule on all 5,997,038
`K=4` states.  Do not try to impose the analogous normalization independently at every threshold:
`(8,5,5,5,1^4)@3` refutes that stronger rule.  Use the hinge profile to describe augmenting-path
cuts and their obstructions, while keeping Pascal orthant saturation as the exact target; see the
[shape survey](../evidence/singleton_shape_survey_2026-08-27.md).
Likewise, do not require every row image to contain a piece whose width occurs in `G_(K-1)`:
`(8,7,4,2,2,2,1,1)@3` forces mixed mass at least ten under that rule, although the branch mass is
nine.  If atom widths remain useful, they need a weaker role, such as an aggregate resource or a
condition only on genuinely split rows, not a row-by-row requirement covering intact rows.

A second global formulation is now worth pursuing in parallel with the signed Hall fold.  For the
transcript graph `Q_K`, the monomial coefficient `c_K(lambda)` of its chromatic symmetric function
counts semi-ordered legal decompositions of type `lambda`.  Exact recursive counting shows that
`Q_3` is **strongly nice**: `c_K(mu)>=c_K(lambda)` whenever `mu<=lambda`, for all 1,206 supported
types and 463,886 comparable pairs.  Strong niceness would imply the converse immediately.  The
graph recursion is `Q_K=Q_(K-1) disjoint-union (Q_(K-1) join Q_(K-1))`; disjoint union preserves
strong niceness but graph join does not in general, so the proof obligation is closure under this
special combined operator, not an existing generic theorem.  See the
[theorem formulation](theorems/singleton-majorization.md#a-stronger-chromatic-symmetric-function-target-2026-08-27)
and [exact census](../evidence/singleton_strong_niceness_2026-08-27.md).

The complete feasible split-count fiber is less promising as an induction variable.  At `K=3` it
is always an integer interval, but continuous hinge and mixed-mass endpoint bounds miss parity and
pure-side packing obstructions.  Computing all endpoints did not finish even a ten-state `K=4`
sample in 60 CPU seconds.  Retain the endpoint examples as diagnostics; do not replace the no-holes
target by an unproved interval assertion.

The complete full-mass census gives supporting geometry: all 5,997,038 states at `K=4` satisfy the
Row-Coloring Lemma, and a balanced value-block rule with one-block lookahead succeeds on every one,
plus ten million uniform `K=5` samples.  Treat that rule as diagnostic for the exchange geometry,
not as the primary theorem: even the simpler global rule that fixes the necessary row counts and
then minimizes total-mass difference fails after 15,855 `K=4` states.  Exact data and counterexamples are in
[the census record](../evidence/singleton_row_coloring_census_2026-08-26.md).  Strict alternation,
lower-load assignment, local safe-next assignment, fixed half-mixed reservation, two-class
exchange, adjacent-pair orientation, global scalar balance, and even the broader class of all
`2`-flat bases have explicit counterexamples.  A proof must exploit the full Pascal multiplicities;
finite or sampled success must not be promoted to the open converse.

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
failed.  The k=7 exhaustive choice corpus is complete; the k=8 remainder is running on the shared
AWS host from 621 of 815 summary-closed prefix blocks.  Let that finish before adding another fitted
heuristic, then analyze the retained antichain of fixed-dimension Pareto upgrades and inequivalent
solving splits for a lineage-preserving choice property.  Only after that corpus is understood
should the minimal production form be tried: a transient lineage-aware hint, one or two templates,
a bounded pass, and unchanged fallback.  Split hints must influence order only; they are not cache
facts.  Use `tools/pareto_census_status.sh` for a one-shot progress query.

**Separate theoretical m=6 thread (corrected 2026-08-26).** The first large-`k` upper bound is
`n(10,6)<=973`, from an exact rejection at 974.  The singleton-majorized tree at 973 is
conditional on the open converse.  The upper bound already refutes both the closed form and `BBCD` profile, which
predict 976.  The separate finite recurrence, when fed the corrected `n(9,5)=481`, predicts
977 and misses by four.  The working root is `[477:2]`, with mixed child
`Sb(496:2,477:4)@9`; it avoids the `Z_7` kernel rather than repairing it.  The separate aligned
atom track now supplies a structural
continuation `2^k-k^2+7k-21` for `k>=12`, conditional on the outer assembly; it is not a fit to this
stored subtree.  Next:

1. find an unconditional 973 construction (or prove the singleton Row-Coloring Lemma), then
   classify working root splits and express the `m=4 + m=2` mixed-child boundary in deficit
   coordinates;
2. decide the proposed one-level lift, whose only new hard child is
   `Sb(503:1,495:2,478:3)@9` (the first five-minute run was inconclusive, and the literal scaled
   continuation is refuted by the exact negative `Sb(247:1,247:1,240:2,231:2)@8`);
3. compare the unrestricted two-bundle frontier with the new aligned construction, then recover the
   low-`k` degeneracies backwards or find an upper obstruction.

The new formula predicts 1983 at `k=11`, but its checked hard-tree threshold starts at `k=12`, so it
does not decide item 2.  Do not infer either a constant `-3` correction to the old closed form or
a constant `-4` correction to the finite recurrence from one value.  Reproduction,
correctness argument and costs are in the latest journal entry and `docs/tools.md`.

**Conditional excess-q construction track (parked 2026-08-16).** This is retained as a completed
investigation, not an active work queue.  Under the explicitly unproved working assumption that
every fixed labelled A/B/C/D pattern eventually reaches a stable atomic-leaf regime,
the corrected diagram gives a concrete branch rather than a missing map.  For
`A=(a:alpha)@k-1` and `B=(b:beta),C=(c:gamma)@k-2`, maximize `d` in
`Sb(d:beta,b:alpha-beta,c:m-alpha-gamma,a-c:gamma)@k-2`, then score the candidate `a+b+d`.
The known-answer `m=5` algebraic calibration is complete, but its arbitrary-majorized terminal
construction is conditional.  Enumeration within that terminal model chooses `(3,2,2)` through
parent `k=7`, ties it with `(4,3,1)` at `k=8`, and switches to `(4,3,1)` at `k=9`.  For the latter
family, the local program proposes the D frontier
`Sb(d:3,(2^t-t):1,(2^t-2t):1)@t`: its candidate value and unconditional upper ceiling are
`2^t-binomial(t-2,2)` at `t=7,8` and one larger from `t=9`.  The latter has a direct two-test
reduction to singleton-majorized leaves, whose decisive prefix inequality first holds at `t=9`.
Conditional on the singleton converse, this shows that a scalable state must
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
The calculation closes conditionally through the `AACC` and `BBBD` profiles and first fails at the
known height-6 `k=10` transition, where refinement changes the candidate D accounting from
`AAAABBCD` (232) to `ABBBBBCD` (229).  The literal refinement of the successful split is now
refuted at `k=11`; do not retry it.  The stronger symbolic result now rules out the entire one-D
refinement class: following mixed outcomes preserves height and cannot branch a D lineage, so all
eight-atom ranks 1--81 are all-depth negative.  Rank 82, `A^6D^2`, has a checked three-level tree
and is the exact A--D eight-atom optimum, yielding the conditional construction
`2^k-k^2+6k-16` from `k>=17`.  Deeper search for the finite 229 accounting was thereby superseded.
At 16 atoms the
D-lineage certificate excludes ranks 1--289, and the new 242-core `(D,C+D)` coinductive kernel
excludes ranks 290--304 at every depth.  The first projected rank-305 tree has no exact lift, but the
product search over alternative projected splits finds a checked 19-node exact tree for
`A^13CD^2`.  Thus rank 305 is the exact sixteen-atom optimum and yields the conditional parent
profile `A^49B^9C^4D^2@G[k-6]`, equivalently `A^7B^7D^2@G[k-4]`, with width
`2^k-k^2+7k-21` from `k>=12`.  At 32 atoms, lineage and a new independently checked 504-core
`(D,C+D)` kernel exclude ranks 1--1179 at every depth.  Rank 1181, `A^26BC^3D^2`, is constructible
by refining the rank-305 tree, so the only unresolved profile in this slice is the wider rank 1180,
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
8. **Delivered; uncolored and top-down colored replays are active separately.** Readable text remains the durable envelope,
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
   checkpointed them before the sixteen-worker k7 phase began. Let both active runs finish and
   verify their manifests before archival. A compact global exact hash remains deferred: neither the 34.7%
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
