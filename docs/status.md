# Status

**Read this first.** Where everything stands, and what will silently ruin your work if you
don't know it. Last refreshed **2026-09-02**.

The central singleton question is now resolved negatively.  Necessity remains proved, but weak
majorization by `G_K` is not sufficient.  The exact-support, full-mass state

    (64,63,57^2,42^4,22^7,8^15,7^2,1^32) <=_w G_6

has no legal first split into three `G_5`-majorized children and is therefore unsolvable in six
tests.  The short proof and independent direct-split/Hall regressions are in the
[`K=6` counterexample record](../evidence/singleton_k6_counterexample_2026-08-30.md).

In fact the still smaller prefix

    (64,63,57^2,42^4,22^7,8^15)

of mass 683 and support 30 is already majorized and unsolvable.  It is tight at ranks 15 and 30;
all six transitions from counts `{7,8}` to `{14,15,16}` violate a pure-child capacity.  Deleting
one of the fifteen 8s makes this prefix first-cut feasible.  The original full-mass form remains
useful because ranks 15 and 32 contract its proof to two symmetric cases.

The full-mass obstruction is exactly Pascal-shaped.  The parent is tight at ranks 15 and 32, forcing the two
color-count transitions `7/8 -> 16`.  The 17 intervening rows `(8^15,7^2)` must send 22 coins
to the mixed child.  Mixed-child saturation at ranks 15 and 32, together with
`H(31)=242<H(32)=243`, forces every band mixed piece to be a positive integer.  In either case one
pure child needs 64 coins from nine rows that can retain at most 63.  Thus neither transition
exists.  This is a human proof; the executable checks are regressions, not the trust base.

This is now the first member of a proved infinite family.  For `K=2m` or `2m+1`, balance the
`n+1` canonical rows between tight anchors `(n-1,2n)`, where `n=2^(m+1)`.  The two endpoint
transitions again force one pure child to exceed the capacity of `n/2+1` band rows.  This gives a
full-mass exact-support majorized parent with no legal first cut for **every `K>=6`**.  The first
new state is

    (128,127,120^2,99^4,64^7,32,31^16,8^32,1^64) <=_w G_7.

The analytic construction and induction are in the
[Tight-Band theorem](theorems/tight-band-capacity.md#a-counterexample-at-every-k6), with exact
machine and independent `K=7` direct-search evidence in the
[dyadic-family record](../evidence/singleton_dyadic_counterexample_family_2026-08-31.md).

The counterexample has the minimum possible full-mass support `2^6=64`.  A complete
prefix-cylinder census now proves the Row-Coloring property at `K=5` for all
1,431,800,647,444 exact-support parents; Minimum-Support Reduction and the complete `K=4` converse
make every one recursively solvable.  Hence `K=6` is the **proved smallest failure level**.

The bottom-up transfer idea locates the boundary exactly.  Replace the canonical band
`(22,7^15)` successively by

    (22-j,8^j,7^(15-j)),  0<=j<=14.

Every state through `j=13` has a first cut whose children are majorized by `G_5`, while `j=14`
is the hole; this statement concerns first-cut feasibility, not complete recursive strategies.
The final one-coin Robin--Hood transfer destroys the whole cut fiber.  A complete exact-support
shell census strengthens the path observation: every one of the 5,189,450,419 parents through
distance 13 has a first cut, so 14 is the global exact-support no-first-cut minimum.  This remains
also the recursive-unsolvability minimum: the complete `K=5` theorem makes every majorized child
recursively solvable.
Allowing transfers in both directions still proves fixed-color fiber connectivity, but feasibility
itself is not downward closed, so bidirectionality cannot recover the converse.

All equivalent or sufficient universal proof targets must be read accordingly.  Row-Coloring,
Pascal orthant saturation, global Robin--Hood closure, Carry-Compatible Gale--Ryser, Balanced
Pascal Realization, canonical direct/allocation transport, Pascal-shuffle coverage and
two-interval splicing are false universally.  `Q_6` is not nice and therefore not strongly nice.
The Positive-Band Extension Conjecture fails on the band `[15,32)`, where neither endpoint
transition works.  The Two-Anchor Reduction remains proved, but its residual conjecture is false
on

    (62,61,55^2,40^4,20^7,6^15,5^2)
      <=_w (62,61,55^2,40^4,20^8,5^16).

The earlier `K<=4` censuses, switch connectivity theorem, fixed-color fiber connectivity,
tight-skeleton factorization, Half-Unit Coalescence and Two-Anchor Reduction remain valid within
their exact statements.  They now describe the boundary and anatomy of the hole rather than a route
to universal sufficiency.  Historical survey records are retained as finite theorems and failed
proof routes.

Operationally, arbitrary `[majorized G_k]` witness leaves are not certificates.  The former
universal justification is false; a particular such leaf needs an independent strategy.
`[canonical U_k]` and `[embedded G_k]` leaves remain unconditional because they delete edges
from Aigner's explicit `G_k` strategy.  The proof-safe `Sa(10)=192`, exact frontiers through
`k=8`, exact `k=9,m=1..6` cells, and published `m=5` theorem are unaffected.  The unsupported
973 tree remains only an upper-bound companion, not an achievability proof.

The earlier real-cover holes remain correct, but the primitive integer question no longer needs a
rounding proof: the state above is the missing primitive lattice hole.  The theorem note has been
superseded in place at
[singleton-majorization.md](theorems/singleton-majorization.md).

The verification-first and first structural items are delivered.  The standalone clean-room
solver enumerates legal integer row triples without Hall code or shared caches, reproduces the
canonical and `j=13` positive cuts, and independently exhausts the padded hole, the mass-697 core,
and the smaller mass-683 core in 9,345 DFS states / 34,958 row options each.  It finds an explicit
cut after deleting one 8 from the smaller core.  Its optimized and naive implementations agree on 201 small
partitions, and it closes all 1,223 full-mass majorized types at `K<=3`, including all 1,206 at
`K=3`.  Hoisting candidate construction out of the option-sort comparator leaves every search
node unchanged and makes an independent exhaustion of the new `K=7` parent practical: 21,489,353
nodes and 238,217,814 options in 30.41 wall seconds, down from about five minutes; see the
[clean-room verification record](../evidence/singleton_direct_split_cleanroom_2026-08-31.md) and
[dyadic-family record](../evidence/singleton_dyadic_counterexample_family_2026-08-31.md).
The proved [Tight-Band Capacity Obstruction](theorems/tight-band-capacity.md) now turns two tight
anchors into a deterministic no-first-cut certificate.  An inequality-only implementation finds
the rank-15/32 certificate independently.  A complete survey of the fixed `[15,32)` face contains
176 dominated 17-row bands: the direct solver finds 175 feasible first cuts and one hole, while the
capacity extractor certifies exactly that same unique band `(8^15,7^2)`.  Hence its half-`l1`
transfer distance 14 is minimal on this face.  The extended inequality census now exhausts the
entire two-anchor certificate class through `K=6`: none of the 613,689,090 eligible `K=5` band
instances has a certificate, while exact prefix-cap optimization proves that 14 is globally
minimal among `K=6` parents that do have one.  A subsequent exact Fixed-Color Hall census checks
all 5,189,450,419 exact-support parents through transfer distance 13 and finds a first cut for
every one.  Hence 14 is globally minimal for an exact-support no-first-cut hole, not merely within
the certificate class.  Counts and provenance are in the
[capacity record](../evidence/singleton_tight_band_capacity_2026-08-31.md) and
[transfer-shell record](../evidence/singleton_transfer_shell_census_2026-08-31.md).
The dyadic constructor separately checks 91 canonical faces through `K=15` and emits 20 certified
parents, while the analytic binomial-tail argument proves at least one for every higher level.

The `K=5` boundary is now closed.  Exact alternating-tail prefix cylinders certify
1,431,650,734,151 of its 1,431,800,647,444 exact-support parents; uncapped Fixed-Color Hall search
checks the remaining 149,913,293 and finds no hole.  The 14-way ranked run reconciles every shard
and completes in 74m49s at 1.31 GB peak RSS; see the
[prefix-cylinder record](../evidence/singleton_k5_prefix_cylinder_2026-08-31.md).  At `K=6`, the
remaining questions are uniqueness within the distance-14 shell and smaller non-unit cores under
other orders.  The computation's analytic core is now the
[Exact Prefix-Cylinder Extension Lemma](theorems/singleton-majorization.md#exact-prefix-cylinder-extension-lemma):
for a fixed prefix coloring and tail word, exact suffix support functions characterize uniform
coverage of the entire completion cylinder, with a closed-form Three-Bound Prefix Corollary.  The
remaining structural target is a compact laminar/Hall-dual law that eliminates the choice among
colorings and explains why the majorization outer bound is exact through `K=5` but first acquires a
thin balanced-band hole at `K=6`.  The ordered programme is recorded under P6 in
[research-plan.md](research-plan.md).

The ordinary recursive solver is usable for an independent `K=6` shell survey without adopting the
research Hall algorithm. Survey builds disable cache lookup/retention at level 6—distinct
full-mass, exact-support parents cannot dominate one another—but retain the valuable `K<=5` child
cache. The attempted full distance-14 census was stopped on 2026-09-01 at the validated durable
boundary 34,000,000 of 9,960,648,265 states (0.341%). Its nine provenance-checked stage logs contain
33,999,999 exact positives, exactly the known rank-55,096 hole, and zero `MAYBE`. This is a finite
diagnostic prefix, not evidence that the known hole is unique outside that prefix.

The broad run was not economical to continue. Across 5h17m40s elapsed it suffered three Spot
`instance-terminated-no-capacity` losses, then the final 26-GiB process limit was exhausted by the
growing child cache after checkpointing 34,000,000. Systemd restarted correctly from that boundary,
but the observed post-restart rate of 826 states/s projected about 139 more days. The uncommitted
400,000-state tail reported no exception but is excluded from the durable result. The Auto Scaling
group is retained at desired/minimum capacity zero and has no instance; all validated logs,
checkpoints and the final stopped status remain under `run10/k6-main-survey` in S3. See the
[production survey record](../evidence/singleton_k6_main_solver_survey_2026-08-31.md).

The post-refutation cold `Sa(193)` rerun is live on AWS under S3 prefix `run10`. Its current-main
binary passed the mandatory `Sa(192)` control as solvable in 389.9 CPU seconds before entering the
193 search. The on-demand `r7iz.xlarge` instance is `i-0318c3349a0df835b`; use
`tools/sa193_status.sh` (now defaulting to `run10`) rather than the terminated historical instance.
The stopped singleton survey remains under `run10/k6-main-survey`; use
`tools/singleton_k6_survey_status.sh` to read its final S3 status. The Auto Scaling group is at
desired/minimum capacity zero, has no worker, and cannot replace one until deliberately scaled up.
The old Sa host's scoped census service remains inactive, and Sa continues alone.

This page says where things *stand*. For what happened and why, see
[journal.md](journal.md); for what to do next, [research-plan.md](research-plan.md).

## Active traps

Each of these has already caused, or was one step from causing, a wrong result.

| trap | why it matters |
|---|---|
| **Production majorization is rejection-only, even at `K<=5`.** | Necessity holds at every level, and a separate exhaustive proof establishes sufficiency through `K=5`; that separate theorem is not a production positive certificate. `[canonical U_k]` and `[embedded G_k]` leaves remain unconditional because they delete edges from Aigner's explicit strategy. Every other majorized singleton follows ordinary exact recursion at every level. Positive cache replay requires the current necessity-only epoch marker because earlier marked ancestors may depend on the retired low-level shortcut. |
| **The primitive integer lattice has a hole; do not restart a real-to-integer rounding proof.** | Earlier padded and exact-support real holes remain valid, but (K6-1) is already a primitive full-mass integer hole.  The Integral Final-Band Extension Lemma remains correct only after a legal head-band allocation; this counterexample shows that the head allocation itself may not exist. |
| **Do not extrapolate a finite singleton census or sampled rule.** | Exact exhaustion proves Row-Coloring through `K=5`, not universally.  The `K=6` hole refutes Row-Coloring, orthant saturation, Carry-Compatible Gale--Ryser, niceness/strong niceness, one-block extension and every sufficient global construction.  Lower-level counts remain finite theorems, not evidence that can override the counterexample. |
| **The Two-Anchor Reduction survives, but its residual conjecture does not.** | The reduction itself is proved.  Its `K=6` image `(62,61,55^2,40^4,20^7,6^15,5^2)` is dominated by `J=(62,61,55^2,40^4,20^8,5^16)` and has no capped residual coloring.  The `K<=4` residual censuses remain correct. |
| **Switch connectivity is a decision method, not an existence theorem.** | Row and incidence switches connect the fixed-margin search graph, but the `K=6` hole has no balanced vertex anywhere in that graph.  Consequently the universal two-move escape/descent conjecture is false even though its complete `K=4` runs remain valid. |
| **Do not continue a universal transfer, repair, shuffle or splice proof.** | The final transfer in `(22-j,8^j,7^(15-j))` takes a feasible first-cut fiber at `j=13` to the empty fiber at `j=14`.  Global Robin--Hood closure and the stronger Adjacent-Fiber, Core--Blocker, canonical transport, Pascal-shuffle and two-interval-splicing claims are therefore false.  Their lower-level lemmas and censuses may still describe local structure, but they cannot prove universal sufficiency. |
| **Do not combine an equal-row quotient with a separate left/right normalization without proving the two canonical orders are compatible.** | The first clean-room direct-split draft sorted equal-row options and also forbade the first right-pure option.  Each rule alone was sound, but together they deleted the sole representative of all-unit solutions and produced false negatives already at `K=1`.  The unquotiented oracle caught it.  The final solver quotients only the row-triple multiset and applies no side normalization; see the [verification record](../evidence/singleton_direct_split_cleanroom_2026-08-31.md). |
| **A missing tight-band certificate is not a positive result.** | The two-anchor capacity test is a sound sufficient obstruction only.  `certificates=0` means unknown.  This remains true even though the complete `K=5` inequality census returns zero certificates on 613,689,090 eligible bands.  The 175 positive verdicts on the fixed rank-15/32 face come from the independent direct-row solver and its replay check, not from the extractor. |
| **Never warm-start a *negative* result from `cache-2025:parsed_260.txt`.** | It contains the 16 `Sa(193)` verdicts under suspicion. Loading it re-reads the old answers and "confirms" them. It cannot be filtered: the cache spans 2023–2025 and does not record which build wrote each line. Fine for *finding* solutions — a poisoned negative only slows a search, never corrupts it, because any solution found is re-verified as a tree. |
| **Never promote a 2023-era negative above `legacy`.** | That build emits false negatives — 37 known, ~0.27%, with **no syntactic marker**. `Sb(143:17)` in 8 was declared unsolvable after 10 passes and 4 days, and is wrong. See [`../evidence/refuted_2023_negatives.txt`](../evidence/refuted_2023_negatives.txt). |
| **A solver log without complete embedded provenance is not new durable evidence.** | Historical outputs cannot identify which bugs and optimizations their binaries contained. New builds go through `tools/build_radio.py`; every raw output must contain `radio-provenance-v1` and pass `tools/check_provenance.py`. Direct compiler builds explicitly say `provenance_complete=no`. Standalone utilities run through `tools/run_with_provenance.py`. The artifact uploader enforces this, with a conspicuous legacy-only override. |
| **Do not use a negative derived by `run3` or `run8` as proof.** | Joint suffix reachability (`rb_dead`) was sound for rejecting the full state but incompatible with the older implicit-prefix contraction: it could cache a shorter negative that was actually solvable, then contaminate later searches. Forced counterexample: `Sb(5:3,2:2,2:2,2:2)@3` is unsolvable while its inferred `Sb(5:3)@3` negative is false. Builds containing `efadab0` but predating fix `75814a7` have no marker distinguishing affected contractions; this includes frozen run3 and run8. Cold `run9` suppresses contraction after an actual reachability rejection. Older exact lines independently rechecked without contraction may still be valid, and positive witnesses remain independently checkable. |
| **`out26_1.txt` / `out26_2.txt` exist twice under the same names.** | ~130-byte stubs in `fullsolve-2026`; the 905 MB / 51 MB originals in `sa193-2023`. Only the latter are evidence. Pulling the wrong tag yields nothing, silently. |
| **A missing `can't solve` line does not mean unsolvable.** | `canSolveB` returns a tri-state and gives up with `MAYBE` on a finite budget, printing nothing. Absence of a verdict is not a verdict. (Briefly narrowed on 2026-08-04 when budgets were disabled; that change was reverted the same day — disabling them trapped a real run for six hours. In a proof-safe cache, a printed `can't solve` is exhaustive because it is emitted only when `!skipped_some`; the separate `rb_dead` trap explains why run3/run8 caches are not proof-safe.) |
| **Do not restore either old budget extreme.** | `c13b5d3` could return before trying a complete child; `e648e83` required a new negative cache fact and then handed pass 2 an unbounded child. Run7 demonstrated the latter failure for 20,460 CPU seconds in a finite-parent k=5 state after 280,116,882,707 prefixes. Current policy permits zero-progress `MAYBE`, never refills an exhausted parent, keeps the reliable one/two-segment spine on the shared allowance, and probes longer states with a geometrically increasing local slice. New builds count accepted split prefixes deterministically at 20,000,000 units per nominal second; `-DRADIO_CPU_BUDGET` is the historical fallback. See [`../evidence/deadline_stall_2026-08-10.txt`](../evidence/deadline_stall_2026-08-10.txt) and [`../evidence/work_budget_rb_root_2026-08-13.txt`](../evidence/work_budget_rb_root_2026-08-13.txt). |
| **`tools/capped_run.sh --rss-gb` cannot bound a long solver run on this machine.** | The result-cache trie grows unboundedly as it solves, and macOS swaps it out rather than keeping it resident, so RSS reads 0.2 GB while 27 GB sits in swap and the cap never fires. A k=8-rooted mapping run reached `VSZ 424 GB` and 6,395 swapins per 45 s, managing 2 of 35 roots in 9 h 20 m. Use the local supervisor, which guards macOS `top`'s documented physical-footprint field, plus `vm_stat` swapins. `vmmap -summary` is useful for one-off attribution but can itself hang indefinitely. The 2026-08-10 local `Sa(193)` trial independently reproduced the RSS gap: 2.77 GB peak RSS versus 7.1 GB footprint. |
| **Do not apply old oracle footprint estimates to the new cache.** | The pre-2026-08-10 pointer trie needed 4.04 GB at `MAX_N=132` and about 20 GB at `MAX_N=262`; those measurements remain explanations of old failed runs, not predictions for current `main`. The deployed last-segment cache is 11.2x smaller on the `MAX_N=193` checkpoint, but a full `MAX_N=262` oracle has not been measured. Cap and inventory any new mapping run rather than assuming either figure. |
| **Benchmark against `radiobase.c`, not against a tool you wrote.** | Three times on 2026-08-09 a headline number came from comparing against the wrong baseline: a 40-state sample, a holdout that shared its *construction* with the training set, and a cold validation measured against a warm benchmark. The last one led to patching a "race" that did not exist. A per-part filter measured at 15.3x turned out to be already implemented by the per-split `s[4]` / `s[5]` loop in `canSolveB`, and a "200x" combined figure collapsed to ~5-13x. Before quoting a speedup, find the existing check that already does it. |
| **`canSolveB_ctx` is not general permission to call the mutable solver concurrently.** | The context owns the accepted-prefix clock, exact L1 and reachability scratch, but ordinary solving still mutates the dominance trie/arenas, lazy split catalog, learned `s[4]`/`s[5]`/`FAST` metadata, and `sbb_to_min_k`. `radio_refute.c` is the narrow safe exception: it prepares cache and split metadata serially, checks their frozen checksum/allocation counts, permits only root enumeration plus k-1 CACHE_ONLY reads, and publishes no results. A thread pool around ordinary solving would still have C data races; see [parallel-solver.md](parallel-solver.md). |
| **Do not use the old Sa(113) colored certificate as proof, and do not reuse `radio_verify.c`'s split enumeration.** | The old independent checker reported its 120,302 records closed, but the frozen solver-core refuter exposes nine uncovered splits while closing all 304,105 normalized facts. The first gap is `Sb(15:8,8:5,8:5)@5`; the discrepancy in `radio_verify.c` coloring/replay is still undiagnosed. Its equal-part quotient is a plausible cause and worth testing: `radio_verify.c:1716` requires a *strictly* increasing option index across equal parts (`g_last[i-1] + 1`) where the sound rule is non-decreasing, and it applies that at `i == 1`, whose predecessor carries the complement restriction — `radiobase.c:2373` guards against exactly that with `i > 1`. Two identical parts could then never take the same option, which over-refutes. That first gap state has two equal `8:5` parts. Unverified inference from reading the source, but cheap to test, and the replacement checker (`tools/cleanroom`) deliberately uses non-decreasing indices with the composition argument written out. **New evidence 2026-09-02, and it points the other way:** the independent `tools/cleanroom` checker closes that same colored bundle with **zero gaps** (120,537 records; the documented first gap `Sb(15:8,8:5,8:5)@5` is present in the file and verifies), agreeing with `radio_verify.c` and not with the refuter. Two independent implementations against the solver-core one. That does **not** clear the bundle - it may equally mean both independent checkers share a misreading - but the suspicion should now include the refuter's own dominance-closure boundary, which prunes the upward closure at the `G_k` majorization edge and could lose a needed citation on a thinned support set where the full corpus had the fact exactly. **That experiment has now been run, and the gap is spurious:** the refuter's own trace names the uncovered split on `Sb(15:8,8:5,8:5)@5`, whose outcome-1 child is `Sb(8:3,7:5,5:2,3:3)`; the bundle contains `fact 4 Sb(8:3,7:5,3:3)` (line 70609), a literal sub-multiset of it, so Subgraph Monotonicity discharges that child and the split is covered. INFO and STAR do not fire on any of the three children, so a citation was required and one was available. The refuter's `child_verdicts=2,2,2` is wrong on its middle entry. So the prohibition's stated reason is wrong for this gap, the suspicion moves to the refuter's citation lookup rather than `radio_verify.c`, and the strict-quotient guess above is superseded. **The mechanism is now found.** `checkCacheTrie_ctx` (radiobase.c:1330) is positional: query part `i` serves both as the dominance test at trie level `i` and as the index selecting the next node, so there is no search over which query part a stored part maps to, and a fact is found only when its parts align with the query's positions. Minimal reproducer via the new `tools/cache_citation_probe.c`: at k=4, fact `Sb(8:3)` is NOT found in query `Sb(7:5,8:3,3:3)` although it is literally one of its parts, while `Sb(3:3)` is found because `3:3 <= 7:5` matches the first part. This is a **completeness** defect, not a soundness one - a missed citation costs search or yields MAYBE and cannot manufacture a negative, so the run9 proof and the certificate of record are unaffected - but it is what produces the spurious refuter gaps. How much the insert-side closure normally restores is not established, and the other eight gaps are unexamined. Details in [the record](../evidence/cleanroom_verifier_2026-09-01.txt). Until the mechanism is found, keep treating both the colored bundle and refuter-only closure claims as unsettled. The new run9 coloring path does not trust that subset: it starts from complete level support, records the exact production-trie terminal cited by every rejection, emits a selection only after zero gaps, and independently audits every selected next level. |
| **Fits with fewer than ~4 data points are meaningless.** | The Pareto data thins out fast: m ≥ 33 has a single k value. A profile or closed form fitted there is unconstrained. |
| **The old `m=5` formula and `BBBD` profile stop being optimal after `k=8`.** | Li--Wu--Triesch prove the piecewise correction: add 1 at `k=9,10` and add 2 from `k=11`; hence `n(9,5)=481` and `n(10,5)=985`.  The old word is still a valid lower construction, not an equality.  Their displayed intermediate equations (69)–(70) have apparent index/off-by-one inconsistencies, so cite the theorem and use the recomputed assembly in [the exact m=5 calibration](theorems/m5-pareto-assembly.md), not those displays. |
| **Do not report that the eventual `m=5` strategy “requires six atomization levels.”** | The first eventual hard leaf `P_7=(127,119,119,118,111)@7` has sharp exact and embedded depth **three**.  Six is instead the first nonnegative scalar inventory for one depth that could exactify `P_r` uniformly for all large `r`; the 64-piece component identities do not yet pack into one synchronized tree.  The paper needs only majorization and proves no aligned `AABD` profile.  Its `k>=11` theorem does rule out later changes in the numerical frontier. See [the exactification analysis](theorems/m5-pareto-assembly.md#exactifying-the-decisive-majorized-leaf). |
| **Never add "move a coin to the larger side" to `compare_solvability`.** | Conjecture (u1) is unproven, and its multi-part form is outright **false**: `Sb(15:2, 5:4)` is solvable in 4, `Sb(15:2, 6:3)` is not, despite lower mass. Wired into the cache as a dominance rule it would manufacture false negatives — the exact failure mode that makes the 2023 corpus unusable. Only *componentwise* part dominance is sound; see [theorems/subgraph-monotonicity.md](theorems/subgraph-monotonicity.md). |
| **Do not reconstruct the Pareto assembly from the first 2026-08-14 attachment.** | The user explicitly reported that it was the wrong picture. Its color/atom transcription is retracted. The corrected diagram gives the four-segment branch `Sb(d:beta, b:alpha-beta, c:m-alpha-gamma, a-c:gamma)@k-2`; see [conjectures.md](conjectures.md#excess-q-pareto-assembly-as-a-variable-d-slice-working-hypothesis-2026-08-14). |
| **Do not maximize a free D-width with full-star majorization—or an approximate mixed frontier—alone.** | Full-star majorization is only a static upper bound; synchronized choices in the mixed child can lower the exact maximum. The exact pair `Sb(11:2,11:2,9:2,3:2)@4` (unsolvable) / `Sb(11:2,10:2,9:2,3:2)@4` (solvable) exhibits the gap, as does the assembly target `Sb(50:4,39:6)@6`. `assembly-rank ... complete=YES` means the necessary-bound ranking is complete, not that its top candidate works. A `mixed-frontier` result with `complete=NO` omits part of the antichain, while `exact=NO` describes only the bounded singletonization predicate. Neither certifies a global exact optimum; the mixed-frontier optimizer deliberately refuses both incomplete and bounded-depth inputs. See [conjectures.md](conjectures.md#excess-q-pareto-assembly-as-a-variable-d-slice-working-hypothesis-2026-08-14). |
| **Do not extrapolate the one-D `ABBBBBCD` accounting—or identify a bounded/profile projection with the exact all-depth problem.** | One D lineage cannot serve a height-6 mixed path. Finite `(D,C+D)` kernels now exclude 16-atom ranks 290--304 and 32-atom ranks 1090--1179, but rank 1180 lies outside the latter kernel. Exact cover now excludes rank 1180 through depth four; this is still bounded, so depth five and all-depth constructibility remain open. The first projected rank-305 tree has no exact lift, while a *different* projected skeleton yields a checked 19-node exact tree. Projection YES is search permission, not a proof; failure of one skeleton, one finite depth, or a capped search is not global failure. See [the atom-lineage note](theorems/atom-lineage.md). |
| **The excess-`q` Pareto assembly is parked, not a pending global formula.** | Its corrected four-segment reduction and exact `m=5` calibration are durable, but the sufficiently-large-`q` postulate, completeness of the outer-family list, and stabilization of the synchronized D frontier are unproved. `m=5` already needs competing outer triples and a piecewise D solution; the height-6 rank-1180 question concerns only one restricted aligned slice. Do not restart finite normalization/rank searches unless a new theorem or construction addresses one of those global gaps. |
| **Never train solvable-vs-unsolvable on two different corpora.** | Certificate negatives and census positives occupy *disjoint* mass bands at k=6 (positives 0.742-0.875 of cap, negatives 0.827-0.959; the central-90% overlap is empty), so any classifier scores AUC 0.946 by learning which corpus a state came from. A permuted-label control does **not** catch this — it destroys the source signal too and returns a reassuring 0.516. What caught it was a matched-pair probe: single-part states differing by one coin scored 47%, chance. Draw both classes from one sampler and label with `radio_one`. Measured 2026-08-20; see [../evidence/value_level_transfer_2026-08-20.txt](../evidence/value_level_transfer_2026-08-20.txt). |
| **A fixed mass-fraction-of-cap sampler does not transfer across `k`.** | `value_gen_states.py`'s original band, [0.70,1.02] of cap, sampled **0 of 300 solvable at k=7**: the per-part solo Pareto maximum grows almost as fast as the cap does, so "mass near cap" stops meaning "near the achievability frontier" by k=7. The band must be bisected per level against a warm oracle (30-state probes, target ~50% solved-of-decided) before drawing a training sample. Measured 2026-08-20; see [../evidence/recursive_value_2026-08-20.txt](../evidence/recursive_value_2026-08-20.txt). |
| **A long-lived `radio_oracle` can crash outright on an ordinary query, silently.** | `alloc_front_record` in `radiobase.c` hard-caps total handles at `NODE_HANDLE_MASK` (~1.07e9, packed into a tagged 32-bit descriptor); a handful of multi-million-split sub-searches can burn through enough of that space to exit with `out of front-record handles`, closing the oracle's stdout pipe. Seen 6 times in 300 k=7 queries. No compile-time knob bounds this the way `MAX_TREE_NODES`/`MAX_MEMO` bound `radio_canon_search_generic`'s pool. A caller that keeps a warm oracle alive across many diverse queries must catch the closed pipe, log the offending state, and restart — losing that one query, not the accumulated warm state. Separately, the deterministic nominal-second budget does not tightly bound wall time: queries budgeted at 5 nominal seconds took up to 270 real seconds. Measured 2026-08-20; see [../evidence/recursive_value_2026-08-20.txt](../evidence/recursive_value_2026-08-20.txt). |
| **`oracle-serve`'s uploaded `state=running` was only a host heartbeat.** | On 2026-08-31 its `STATUS` had been refreshed one minute earlier, but direct SSM inspection found no oracle process and no `radio-oracle-server` unit; `server.out` ended with the last complete snapshot on 2026-08-24. A future standing service needs `systemctl`/`pgrep` in its health report. Do not infer service availability from the old `STATUS` file. The historical host is now terminated. |
| **A headline median/selectivity number can hide hard-case collapse — stratify by exact candidate-set size before trusting it.** | The recursive cut scorer's "120x median" looked uniform until stratified by exact stage-2 candidate-set size: the hardest third has median rank 331 (vs 146 for the middle third) and a worst case of 16,886 of up to 130,262 — barely better than blind. Literal winning-cut count is *not* a usable hardness proxy here — it is almost always exactly 2 or 4 (the trivial complementation pair), with no real spread. The fix was not more data: composing with the sound `R_0` filter first made the degradation disappear entirely (hardness/rank correlation 0.129 -> 0.001), because `R_0` shrinks the hardest tier *more*, not less (10.6x vs 4.7x). Measured 2026-08-21; see [../evidence/recursive_value_worst_case_2026-08-21.txt](../evidence/recursive_value_worst_case_2026-08-21.txt). |
| **A learned ranking can never certify "no solution exists" — only a sound filter can, and none has been composed into a small cutoff yet.** | `R_0` (`tools/bundled_majorization.r0`, proved in `theorems/singleton-majorization.md`) is the only sound worst-case bound measured against this thread's stage-2 candidate set so far: median 6,892 / worst 16,547 survivors, down from up to 130,262 — real, but still thousands, not the "exhaust a handful and stop" cutoff the ideal algorithm would have. Do not read the recursive scorer's rank-within-survivors (median 14) as any kind of stopping bound; it is ordering only. The cross-part pair condition and deeper `R_d` are the next sound filters to compose and have not been tested on this stratification. Measured 2026-08-21; see [../evidence/recursive_value_worst_case_2026-08-21.txt](../evidence/recursive_value_worst_case_2026-08-21.txt). |
| **A solvability-rate-by-(k,parts) count from one run's log is not a neutral sample — check how it was obtained before drawing a structural conclusion.** | The 2026-08-08 journal figure "8-part k=6 states: 0 of 165 solvable" was read on 2026-08-21 as "8-part k=6 is structurally unsolvable, a pure refutation regime" and written into new evidence/docs that way. Wrong: those 165 states came from one specific solver run's log, whose own selection may have been biased toward negatives, and that was never checked before the count was treated as a fact about the state space. Retracted the same day. This is the same corpus-provenance discipline as the disjoint-corpora trap above, applied to a corpus nobody had scrutinized yet — any rate/count pulled from "a run's log" needs its selection process checked before it supports a positive-or-negative structural claim, not just before training a classifier on it. |
| **A census corpus named `kN` is rooted at k=N, but its *endpoints* are residual states at a smaller level — check `C["rk"]`, never assume the label is the query level.** | `CORPORA["k7"]`'s forced endpoints are reached after some root-level splits of a census rooted at k=7; their real parent level is `C["rk"] = 5`. Querying at k=7 instead of 5 gives internally-consistent but WRONG answers — a different, easier question, with correct-looking mass arithmetic and genuine `SOLVABLE` verdicts that mean nothing about the endpoint's real claim. Caught 2026-08-22 only because the resulting stage-2 candidate count (18.9M) didn't match the already-known median for that population (~54,000) — a plausibility check that happened to exist, not a structural guard. |
| **Subsampling a stage-2 candidate pool before checking winner-membership can silently drop every known winner for a rare-winner (hard) endpoint — check membership, or don't subsample at all.** | `tools/ml/tier_sample_via_aws.py`'s first version capped the exact stage-2 pool at 20,000 (~33% retention of a 56-60k true pool) before ranking. For a 4-winner endpoint this gave `rank_learned=None, rank_natural=None` — read as a null result about the ordering, but actually the sampled pool excluded all 4 known winners by chance (confirmed: all winners are present in the *true*, unsampled pool). This is not a corner case for this track: rare-winner endpoints are exactly the hard population it targets. Fixed by raising the default cap to 150,000 (above this corpus's typical true stage-2 size, so ordinary endpoints are never subsampled) and printing `known winners in pool: X/Y` per endpoint. `real_benchmark_via_aws.py` had the same pattern; fixed the same way. Measured 2026-08-22; see [../evidence/tier_sample_via_aws_2026-08-22.txt](../evidence/tier_sample_via_aws_2026-08-22.txt). |
| **A per-part signal's strong population-level AUC does not survive conditioning on one hard state's own R_0-survivor set — check within-population discrimination before proposing it as a ranker.** | The per-part Pareto-margin ("deficit") signal scores AUC 0.9961 alone as a solvability classifier across many states of varying difficulty — nearly matching the full pooled model. But 73-93% of every tested near-cap-mass endpoint's own mass-feasible candidates already sit at the single worst admissible deficit value (hitting an exact mass total forces some part to its own boundary), so *within* one hard endpoint's R_0 survivors the signal has almost no dynamic range left: ordering by it landed 1,000x-2,700x behind the pooled model on the same 4 real endpoints that otherwise validate the pooled order. A best-first *generator* built on the same signal fails outright for the identical reason — the winners it needs sit at the worst threshold in every dimension at once, so the search must explore close to the full cross product before finding anything feasible (200,000 pops, zero feasible candidates, on the easiest of the 4 endpoints). Not a search-algorithm problem — the score itself doesn't discriminate near the boundary it saturates at. Measured 2026-08-22; see [../evidence/deficit_order_and_bestfirst_2026-08-22.txt](../evidence/deficit_order_and_bestfirst_2026-08-22.txt). |
| **Block coordinate descent (using the real pooled score, not a separable proxy) works on the easier half of a hard population and fails decisively, not just slowly, on the harder half.** | Fixing 2 of 4 parts and jointly re-optimizing the other 2 against the full recursive-V score (a real "packing problem" framing: the mass/cap constraint is a solved multiple-choice knapsack, but the quality signal is a non-separable joint function of the parts) succeeded with real oracle-verified splits on the two tier-sample endpoints where the pooled model itself ranked easily (rank 1, 13) — but failed after 150 restarts (up to 774,712 evaluations, 225x the endpoint's own R_0-survivor count) on the two endpoints where the pooled model itself struggled (rank 85, 687). 1-part-at-a-time descent fails outright everywhere (0/30 real successes even on the easiest endpoint). Not cheaper than direct full-list scoring at the k7 scale tested even where it succeeds (1.0x-2.4x the direct-scan cost). Measured 2026-08-22; see [../evidence/deficit_order_and_bestfirst_2026-08-22.txt](../evidence/deficit_order_and_bestfirst_2026-08-22.txt) section 6. |
| **`enumerate` has no incremental pruning — it is not usable for ordering, even at k=7 on an ordinary 4-part state.** | `enumerate_winning_splits` checks the cap/`R_0` bound only at the deepest leaf of its raw mixed-radix walk, never on a partial sub-tree, so its cost is the RAW combinatorial size regardless of how selective the bound is. Fine through k<=6 (validated); at k=7 with wider parts a single call ran 10+ minutes with no result (killed, not timed out — the connection stayed alive, streaming partial `WINNER` lines). Use the existing stage-2 (cap + per-part-Pareto)/`R_0`/recursive-V pipeline for ordering a k=7 search instead; `enumerate` remains valid and fast for its original purpose (complete exact winner sets, k<=6). |
| **Cache-load cost is hump-shaped; never extrapolate it.** | Loading the archived corpus, the rate goes 44,484/s at the start, down to 431/s through an expensive band, then up to 1,067,487/s once almost everything is subsumed. Local prefixes sampled the rise and the collapse and missed the recovery, so I predicted 13+ hours for a load that took **1.58 h** (21,866,180 facts, 3,847/s overall, 36% of the time in five of 88 chunks). Two points were wrong and three were wrong; only the run settled it. Measured 2026-08-20; see [../evidence/warm_oracle_2026-08-20.txt](../evidence/warm_oracle_2026-08-20.txt). |
| **Do not extrapolate solver cost from one compile-time dimension.** | Historical builds conflated total state size with component-catalog width: `MAX_N=400/MAX_K=6` initialized in 205 s while `MAX_N=485/MAX_K=9` initialized in 146 s. Current builds can set `MAX_PART_N` independently. Measure eager init, lazy splits, exact search and cache insertion separately; the oracle skips cache facts exceeding either geometry bound. Historical measurements: [warm-oracle record](../evidence/warm_oracle_2026-08-20.txt); corrected attribution: [main-solver record](../evidence/main_solver_singleton_refutation_2026-08-31.md). |
| **Size `MAX_N` and `MAX_PART_N` to different things.** | `MAX_N` covers the sum of all component side-sums; `MAX_PART_N` covers the largest one-component side-sum and sizes the expensive catalog. They remain equal by default for compatibility. The 30-row mass-683 core needs total bound 713 but component bound only 65; a `MAX_N=793,MAX_PART_N=65` regression (large enough for the 64-row padded control too) initialized in 0.016 s. |
| **Bound negative cache closure by the same necessary region as search, and quotient equal parts.** | The ordinary engine proved the 30-row K=6 core negative in 0.004 s, then the old permutation-expanded dominance insertion continued past 12.9 CPU minutes. Majorization pruning alone still exceeded a 60-CPU-second diagnostic: the first million nodes were admissible. The decisive second fix was to choose one representative of each equal part. Exact unbounded insertion now takes 30 recursive nodes with 203 majorization-boundary prunes; an independent K=3 regression matches all 65,025 normalized in-bound seed/query dominance relations. The arbitrary node limit is disabled by default. |
| **Do not read the choice census's `complete=` as a structural candidate count.** | `enumerate_rec` prunes with `CACHE_ONLY` lookups against a warm dominance cache, so `complete` — and therefore `full_complete_candidates` — depends on cache history, not on the state. It reads as a median of 2 candidates at k=7 single-class endpoints; the cache-free count under sound filters (information cap plus the four-rectangle proven frontier) is a median of **13,276**. Quote `complete` as search effort, never as the size of the choice problem. Measured 2026-08-18; see [journal.md](journal.md). |
| **Do not re-attempt a scalar score over split geometry.** | Refuted 2026-08-18 on all 153 four-part single-class k=7 endpoints. Ranking the majorization-feasible candidates by each of 19 geometric features puts the winner at best at the **5th percentile** (tightness, median rank 659; `dev` 0.098), with **one endpoint in 153** reaching the top ten. Filtering to `R_1` first does not rescue it. This is the same wall the 2026-08-08 tightness claim and the 2026-08-09 fitted score hit, now measured on the complete single-solution corpus: there is no local signal to fit or to transfer. Spend effort on cheap sound *necessary conditions* instead — their selectivity multiplies. |

## Goals

| | goal | state |
|---|---|---|
| **H1** | Publish | Draft in `paper/`. `Sa` is now proven through k=10; the `<TODO>` sections and remaining paper cleanup are tracked in P5. |
| **H2** | The K=9 Sb column | **Main open front.** Exact maxima are known for `m=1..6`; run9 supplies proven upper bounds at `m=81..96`, retained legacy lower bounds cover parts of `m=65..94`, and the band **m=7..64 is entirely blank**. |
| **H3** | Is `Sa = 192` maximal at k=10? | **Done 2026-08-16; independently re-verified 2026-09-02.** Proof-safe cold run9 rejected all sixteen `Sb(n1:193-n1)@9` roots in one session; a verified tree proves 192 achievable. The certificate of record is closed with zero gaps by `tools/cleanroom`, which shares no code with the solver: all 2,846,568 claims and the structural chain check in one 1h50m run, agreeing with the production engine on total work to 0.46%. See [the record](../evidence/cleanroom_verifier_2026-09-01.txt). The trust base for the negative half is therefore no longer a single implementation. |
| **H4** | Structural theory | The Singleton Majorization Converse is exhaustively true through `K=5` and false for every `K>=6`; hence `K=6` is the proved first failure level.  Prefix cylinders plus exact Hall fallback close all 1,431,800,647,444 exact-support `K=5` parents, and the Exact Prefix-Cylinder Extension Lemma now isolates their reusable sufficient criterion.  The original `K=6` counterexample is the unique first-cut hole on its fixed rank-15/32 face, and exhaustive search makes distance 14 globally minimal for exact-support recursive unsolvability.  Next classify the distance-14 holes/non-unit cores and seek the laminar Hall-dual law eliminating the coloring choice.  The excess-`q` Pareto-assembly avenue remains parked. |

## What is established

Facts live in `data/*.csv` with per-cell `bound`, `status` and `source`;
[results.md](results.md) narrates them. Summary:

- **Sb Pareto frontier, K = 1..8** — proven maximal, artifact-backed. 129 of 130 cells have
  both certifying log lines located; the exception is the trivial `k=8, m=1` dichotomy. The
  30 KB of certifying lines is committed at
  [`../evidence/pareto_certification_k1_8.txt`](../evidence/pareto_certification_k1_8.txt),
  so the provenance survives the 1.8 GB of logs.
- **Sa sequence, k = 1..10** — proven maximal. `Sa(192)` has independently verified witness
  trees; proof-safe cold run9 exhaustively rejects all sixteen first-test possibilities for
  `Sa(193)`. The **certificate of record** for the `Sa(193)` refutation is now the trimmed
  eight-level chain `sa193-certificate-2026-08-19`: 2,846,568 claims, 15.6 MB compressed, inductively
  closed, structurally checkable without a solver by
  `tools/check_level_chain.py --expect-top-sum 193`. See
  [sa193-certificate.md](sa193-certificate.md). Cold run9 remains the proof *source*; the certificate
  is the compact replay artifact, and it cannot answer anything outside its own claim set — the
  complete corpus is retained for that.
- **Sharp asymptotic `Sa` rate** — Florin--Ho--Jiang (2022) determine the exact leading
  constant, while Jiang--Polyanskii--Vorobyev (2019) give an explicit near-optimal construction.
  These are asymptotic statements, not finite-cell evidence; see [literature.md](literature.md).
- **Singleton necessity, a negative converse theorem, and two further theorems** — Singleton
  Majorization Necessity, Unit-Group Elimination and Subgraph Monotonicity are proved.  The
  full-mass exact-support `K=6` state in the opening section refutes the converse.  Row-Coloring is
  nevertheless exhaustively true through `K=5`, making `K=6` the proved first failure level.  The
  Exact Prefix-Cylinder Extension Lemma is a sharp uniform-completion criterion for a fixed
  coloring scheme, with an explicit weaker corollary.  The signed Hall rank,
  convex-hull identity, switch connectivity and tight-skeleton factorization remain useful exact
  formulations of the hole, not missing steps toward a universal proof.  Subgraph Monotonicity is elementary
  but was load-bearing and unwritten: it is
  what the result cache's downward/upward closure and the whole `sbb_greater` relation rest
  on, and what lets a negative certificate store antichains instead of closures.
- **Exact small-`m` frontiers** — Aigner gives `m=2,3`; Li--Wu--Triesch gives `m=4` and the
  piecewise `m=5` formula; the retained local replay gives the `m=6` upper boundary.  Thus the
  K=9 column is exact through `m=6`, with values 512, 511, 503, 496, 481, 473. The newly
  inspected Aigner 1988 Chapter 3 scan independently verifies the graph model and its `m=2,3`
  results; it does not include the book's Chapter 2 weighing material. See
  [aigner-1988-scan.md](aigner-1988-scan.md).
- **18 structurally checked witness trees** — `Sa(38)` through `Sa(192)`, plus recursive trees for
  `Sb(248:3)@8`, `Sb(496:4)@9`, the old 480 and new 481 `m=5` constructions,
  `Sb(473:6)@9`, their two-sided variants where available, and the unsupported
  singleton-majorized derivation for `Sb(973:6)@10`, plus the exact three-level atomization of
  the decisive `m=5` majorized leaf at `P_7`. All pass structural checking; the two
  `majorized_*` files are explicitly reported as unsupported terminals.
- **16 exhaustive multi-part enumerations** — `data/exhaustive_multipart.csv`, including one
  proven negative.

## What is refuted

Kept on record so it is not re-derived: the Singleton Majorization Converse and all equivalent
universal no-hole/transfer/niceness formulations (counterexample (K6-1)); the old `m=5` closed form and `BBBD` profile as
equalities (both predict 480 instead of exact 481 at `k=9`), the `m=6` closed form and `BBCD`
profile (both predict `n(10,6)=976`, while the unconditional upper bound is 973), the `m=11` closed form
(violates monotonicity in m), the hand-typed `409?` as a *derivation* (though see below — it is
*consistent* with the profile model), and 31 verdicts from the 2023 build.

## Where H4 stands — structural results and the parked profile track

For fixed m, `n(k,m)` appears to be a fixed multiset of atoms drawn from the base sequence
`G_{k−q}` (a "profile"). Established this session:

- Profiles form **refinement classes** under `A→aa, B→ab, C→bc, D→cd`. Pure refinement preserves
  the value function and doubles the length, so it adds no expressive power *within that class*.
  A larger `q` also admits genuinely new, non-refined profiles; the 16-atom height-6 band now makes
  this distinction operational.  The class invariance is verified against the spreadsheet columns.
- **`length = 2^q` is a refinement invariant** — a property of the whole class, not of the
  shortest representative. Holds for every m ≤ 13.
- Within the originally fitted refinement classes the profile is **unique** for m ≤ 9 and
  m = 11..13, but that no longer implies a frontier value: `BBBD` uniquely gives 480 for
  `m=5`, while the exact frontier is 481 in a new `4+1` regime.  The still-open row-wise
  predictions are 457, 447 and 432 for `m=7,8,9`, then `414..416` for `m=10` (two fits),
  followed by 410, 395 and 388.
- **The `m=5` transition is structural, not just +1.**  At `Sb(481:5)@9` all 23 feasible
  `3+2` roots fail, while the relaxed majorized-terminal search finds `4+1` roots of selected
  width 240–242.  Its tree starts with the complementary `[239:1]`; Li--Wu--Triesch independently
  make the same root-type
  switch.  Atom masses read `BBBD=480`, `ABBD=481`, `AABD=482`, but the latter words are
  presently arithmetic interpretations, not per-coin profiles extracted from the compressed
  witness.  See [conjectures.md](conjectures.md#exact-m5-transition-and-the-structural-break-updated-2026-08-16).
- **The published exact `m=5` result calibrates the A/B/C/D assembly diagnostically (corrected
  2026-08-26).**  Complete finite
  enumeration over proven lower frontiers chooses `(alpha,beta,gamma)=(3,2,2)` through parent
  `k=7`, ties it with `(4,3,1)` at `k=8`, and chooses `(4,3,1)` at `k=9`.  With `t=k-2`, the
  latter branch's only hard outcome is
  `Sb(d:3,(2^t-t):1,(2^t-2t):1)@t`.  Finite files and a uniform two-test reduction reach
  singleton-majorized leaves at `d*=2^t-binomial(t-2,2)` for `t=7,8`, then one larger for every
  `t>=9`; these local positive claims used the now-refuted singleton converse and are not
  unconditional constructions.  The uniform
  leaf inequality first succeeds at `t=9`, internally explaining the `k=11` breakpoint.
  The published theorem independently supplies the final 985 and 2001 values.  Thus this supports,
  but does not unconditionally validate,
  black-box A/B/C composition and D maximization for the known case, while refuting any restriction
  to one outer triple or one non-piecewise D formula.  Proof and reproduction are in
  [the m=5 calibration](theorems/m5-pareto-assembly.md).
- **The stronger exact-atom question is now separated from the paper's majorization proof.**  For
  the first eventual decisive leaf `P_7=(127,119,119,118,111)@7`, exhaustive distinct-slot and
  exact-submultiset recurrences both fail through two further tests and an independently checked
  exact tree succeeds in three.  Coefficient comparison excludes every fixed exact depth at most
  five as one choice valid for all sufficiently large `r`; depth six is the first scalar possibility
  and has explicit 64-piece inventories for each component.  Their synchronized column packing and
  the minimum uniform embedded depth remain open.  The published formula itself is final for every `k>=11`, so
  there is no later numerical regime transition.  See
  [the exactification analysis](theorems/m5-pareto-assembly.md#exactifying-the-decisive-majorized-leaf).
- The journal's "m=11 first large jump to length 64" **dissolves** if the single
  exactly-diagonal cell `Sb(11:11)` is treated as pre-stabilised; m=11 then fits at length 16
  like its neighbours. Not airtight — m=4 includes *its* diagonal cell and fits fine.
- **The profile now has a mechanism, derived from the trees rather than fitted
  (2026-08-03).** Fix an m-side coin; over the `k-t` tests above normalisation level `t` it is
  taken or not, giving exactly `2^(k-t)` paths, each ending in one n-side chunk that must be
  an atom of `G_t`. The profile is that multiset. So `length = 2^q` counts paths, refinement
  invariance is automatic, and the whole-tree census is `m` copies of it. Verified on 11 of
  14 committed solutions; `tools/profile_from_tree.py`. Full argument in
  [conjectures.md](conjectures.md#the-profile-has-a-mechanism-derived-2026-08-03).
- **Invariants (2026-08-03):** across the 9 solutions of `Sb(480:5)@9` only total mass and the
  atom count `m·2^(k-t)` are invariant; the atom census takes **3 distinct values**. So no
  invariant selects "the" solution — the space splits into genuinely inequivalent classes, which
  reframes the canonicalisation programme: the profile is an invariant of the
  *symmetric, non-wasteful* class, not of the state.
- **The canonical searches report one tree per *top split*, not all trees.** `search_state`
  returns on the first successful subtree and memoises it. `Sb(473:6)@9` has exactly one working
  top split (`[242:4]`), so each mode returns exactly one tree — yet restricted and unrestricted
  return *different* subtrees under it, proving several exist. Do not read "1 tree" as "one
  solution", and do not read it as evidence that no symmetric tree exists.
- **m=6's extra depth is forced.** `target_k=4` on `Sb(473:6)@9` returns `NO_CANONICAL_TREE`:
  no solution has all leaves canonical above depth 3, against depth 5 for m=5 and 6 for m=3,4.
  So `q` jumps 4 -> 6 (profile length 16 -> 64), skipping 32 — the same gap seen at m=11.
- **`Sb(473:6)@9` does not exhibit the profile** — asymmetric, and it wastes 7 paths. This
  answers the journal's headline open question negatively *for that witness*; whether a
  symmetric one exists is open.
- **Reframed obstruction:** `q(m)` is not arbitrary, since `q = k - t`. The real question is
  how deep a solution must go before every leaf is a singleton.
- **n-side splits — ANSWERED (2026-08-03): one-sided is not necessary.** Two-sided-only
  canonical trees exist for both `Sb(480:5)@9` (5 trees) and `Sb(473:6)@9` (1 tree), verified.
  The latter is decisive: the unrestricted witness uses 5 one-sided splits and all are
  avoidable, on two states at one k. Earlier notes below are superseded.
- **Caveat: orientation flips.** A part is stored `n >= m` and a child can invert that, at which
  point the fixed-m-side path model miscounts. The two-sided `473:6` tree has 1 flip, so its
  waste and symmetry figures are not meaningful. Trees with 0 flips — the unrestricted `473:6`
  and all nine `480:5` — are unaffected. `profile_from_tree.py` now detects and refuses.
- *(superseded)* **n-side splits — unresolved.** Zero working splits leave the n-side whole in
  full enumerations of six frontier states (m=5,6 at k=5,6,7) or in the mixed child one level
  down. The 34 instances in the unrestricted `Sa` trees are **genuine** but avoidable: every
  state tested exhaustively also admits a fully two-sided split, though those are rare (76 of
  5440 in one case). The 5 in the canonical trees are no-ops. **The open question is
  compositional** — local two-sidedness at every node does not imply a complete two-sided tree,
  since taking the two-sided split changes the children. `radio_canon_twosided.c` now exists
  for this: `TWO_SIDED_ONLY=1` rejects any split leaving a non-nil part's n-side whole.
  **First result: m=5 at k=9 survives the restriction** — 5 verified trees in
  `witnesses/canon_480_5_at9_twosided.tree`, 0 one-sided splits in 474 entries. The m=6 run
  was killed for memory before finishing, so m=6 is still open.
- **Caveat on `canon_*.tree` evidence.** `radio_canon_search_generic` does *not* prohibit
  one-sided splits (verified in source: n-side spans `[0,n]`, m-side enumeration is a bijection,
  the mirror filter is top-level dedup only). But it *does* require every leaf to be a
  `G_k`-atom singleton state, so conclusions about tree interiors from those trees are
  specialized to that terminal class; the resulting trees themselves are unconditional proofs.
- **The apparent m=6 recursion is refuted at its first extrapolation.**
  `n(k,6) = n(k-1,4) + n(k-1,5)` is exact for `k=5..9` but, using the corrected
  `n(9,5)=481`, predicts 977 at `k=10`; the unconditional upper bound is 973.  The unsupported
  relaxed file uses a `2+4` root and saturates the `m=4`
  pure child, but backs the other width down from 481 to 477.  The separate old `BBCD`
  closed form predicts 976.  This is one datum, not a new
  constant-correction formula.  See
  [conjectures.md](conjectures.md#finite-m5m6-recurrences-and-the-k10-break-2026-08-10).
- **Ruled out:** the non-adaptive reformulation. Each test returns
  `[x∈S] + [y∈S]`, so non-adaptive solving is a Sidon condition
  `(U−U) ∩ (V−V) = {0}` — exact for m ≤ 2, but strictly weaker from m = 3 (k=4, m=5: 6 vs 9).
  Adaptivity is essential; the Sidon picture is an intuition device, not a reduction.
- **Conjecture (u1) — mapped, not proved (2026-08-03).** `Sb(n1:n2)` solvable in k, n1 ≥ n2,
  implies `Sb((n1+1):(n2-1))`; equivalently the frontier decreases *strictly* in m. Now
  exhaustive over every one-part state at k ≤ 5, on top of the 130 proven cells. Two routes are
  **refuted**: the multi-part generalisation is false (`Sb(15:2,5:4)` solvable in 4,
  `Sb(15:2,6:3)` not), which kills every induction that rewrites a strategy part by part; and no
  mass-based coin-move lemma exists (`Sb(8:1,2:1)` solvable in 3, `Sb(9:1)` not, at strictly
  lower mass — by a direct embedding in `G_3`, not by solver). What remains is one lemma: *the winning
  split minimising `p−q` survives the coin move*, 187/187 at k ≤ 5 plus 28 at k = 6.
  [conjectures.md](conjectures.md#conjecture-u1---the-antidiagonal-conjecture).
- **Parked construction track (opened 2026-08-14; parked 2026-08-16):** the working programme
  assumed without proof that every
  labelled A/B/C/D component geometry (not the atom-letter notation) reaches an atomic-leaf regime
  for sufficiently large `q`.  For `A=(a:alpha)@k-1` and
  `B=(b:beta),C=(c:gamma)@k-2`, the corrected picture makes the hard branch exactly
  `Sb(d:beta,b:alpha-beta,c:m-alpha-gamma,a-c:gamma)@k-2`, with parent candidate `a+b+d`.
  `search_singletonization assembly-enumerate` now consumes only complete proven Pareto input
  levels, exhausts the ordered triple family, ranks it by sound full-star D bounds, and exact-solves
  every slice still able to tie the incumbent.  At `m=10` it certifies family optima 12, 33 and 82
  at parent levels 5, 6 and 7, with respectively 2, 4 and 1 winning triples; all trees verify
  (`tools/singletonization_regression.sh`).  `assembly-rank` completes the level-8 static stage—165
  admissible triples, 37 surviving bounds—but exact optimization remains open; in particular its
  simple target `Sb(50:4,39:6)@6` is negative.  The generic mixed-frontier/guarded-piece optimizer
  identifies the recursive object a scale-free D formula would have to control; no such formula was
  obtained.  A height-first atom calculation
  clarifies that A/B/C are black boxes: only their outer width-height pairs enter the hard branch;
  no witness-tree or atom-placement alignment is required.  It gives a uniform conditional
  construction for the `AACC` height-4 profile.  At height 5 the old `(3,2,2)` branch gives the
  uniform `BBBD` lower construction for `k>=7`, while the exact calibration retains the competing
  `(4,3,1)` branch and recovers the full published frontier through its sharp piecewise D slice.
  The `ABBD`/`AABD` parent words remain arithmetic masses rather than aligned-profile proofs.  At
  height 6 the original track lands on the known obstruction exactly: the old D word `ABBD@G_6` has width 232,
  whereas the exact `k=10` slice has `d=229`, expressible after refinement as `ABBBBBCD@G_5`.
  Each selected width in the stored split has a unique aligned decomposition inside its refined
  component one level lower; the resulting literal lift is exactly
  the already refuted `Sb(247:1,247:1,240:2,231:2)@8` residual, so that split is not an induction.
  The symbolic fixed-depth search is now supplemented by an all-depth D-lineage certificate.
  Refinement `D->CD` never branches a D lineage, and the mixed outcome preserves height, so a
  height-6 eventual leaf needs two unweighted D lineages.  This excludes eight-atom ranks 1--81 at
  every depth, including `ABBBBBCD` (rank 59): the finite 229 accounting cannot stabilize by pure
  refinement.  Rank 82, `A^6D^2`, has an independently checked 19-node, three-level tree with root
  base threshold 12.  It is therefore the exact widest A--D eight-atom germ in the relaxed model
  and gives only an unsupported parent derivation `2^k-k^2+6k-16` for `k>=17` inside this height
  triple.  At 16 atoms the
  D-lineage theorem excludes ranks 1--289 (the refined 229 class is rank 191), and a separately
  checked 242-core `(D,C+D)` coinductive kernel excludes ranks 290--304 at every depth.  The first
  25-node projected tree for rank 305, `A^13CD^2`, does not lift exactly, but an all-skeleton product
  search finds a different 19-node relaxed tree with root-base threshold 6.  Rank 305 is consequently
  the exact sixteen-atom optimum in that model.  Attaching the outer branches yields the unsupported profile
  `A^49B^9C^4D^2@G_(k-6)`, equivalently `A^7B^7D^2@G_(k-4)`, and width
  `2^k-k^2+7k-21` for `k>=12`.  Its arithmetic reproduces 473 at `k=9` and 973 at `k=10`, but
  predicts the still-open 1983 at `k=11`; the symbolic threshold does not settle that finite case.
  At 32 atoms, lineage excludes ranks 1--1089 and a separately checked 504-core projected kernel
  excludes ranks 1090--1179 at every depth.  Rank 1181, `A^26BC^3D^2`, is constructible by pure
  refinement of rank 305; only the wider rank 1180, `A^27C^3D^2`, remains unresolved in this
  normalization.  Propagated mixed-supply loss makes its exact depth-three product exhaustive and
  both the C++ complete-product engine and the independent Python all-skeleton implementation prove
  that no aligned tree of depth at most three exists.  At depth four, exact solution of both pure
  outcomes reduces the root from 7,266 filtered first tests to 6,712 tests and 1,826 distinct mixed
  children.  The only eight children that spend any `C+D` supply are all exact depth-three
  negatives in both implementations.  Thus every remaining first transition has
  `ell_D=ell_V=0` and `1<=ell_W<=14`: 6,696 tests and 1,818 distinct mixed children in fourteen
  scalar W-loss classes.  Guided exact cover exhausts every one of those children and returns
  `NO`; `tools/check_atom_profile_cover_log.py` independently replays all class counts and scoped
  verdicts from artifact `rank1180-depth4-2026-08-15`.  Consequently no rank-1180 aligned tree has
  depth at most four.  This remains a bounded result: depth five and all-depth constructibility are
  open.  The mixed-supply lemma gives the sound
  finite-depth bound
  `(D,V,W)->(D,V+tD,W+tV+binom(t,2)D)`, and the outer algebra reduces a general `N=2^s` D germ
  `A^(N-b-c-2)B^bC^cD^2` to the width
  `2^k-k^2+(2s-c)k-s^2-3s+c(s+1)-b+2`.  Hence each slice minimizes `c`, then `b`; at 32 atoms
  the still-unconstructed `b=0,c=3` postulate would give `2^k-k^2+7k-20`, while only `b=1` and
  the `-21` construction are checked.  The former now requires depth at least five if it exists.
  At depth `t`, scalar supply forces
  `c>=max(0,2s-1-2t)` and a tied-boundary quadratic lower bound on `b`.  At depth three this leaves
  only five persistent B-saving tracks; rank 1180 begins the first.  Once `t>=s`, however, even
  `b=c=0` passes scalar supply, so this calculation cannot itself become an all-depth theorem.
  The depth-four calculation uses only the outer profiles of A/B/C;
  their internal witness trees do not enter the D-branch constraint.  Arbitrary excessive `q`
  therefore remains open and the programme is now parked; no further normalization or rank search
  is scheduled.  The 32-atom slice is
  a one-rank problem rather than a profile scan.  All losing and positive
  certificates are independently checked by `tools/atom_profile_regression.sh`, and the depth-four
  cover logs by `tools/check_atom_profile_cover_log.py`; see
  [the atom-lineage note](theorems/atom-lineage.md).  No Pareto datum changes.
- **Split choice is recursive, not geometric (measured 2026-08-18; k=8 finalised 2026-08-20).** The
  k=7 choice census has **183 of 610** endpoints with a single winning automorphism class (30.0%;
  153 of them four-part) and the **completed** k=8 census **505 of 1,893** (26.7%; 3 two-part, 31
  three-part, 471 four-part), plus 198 unique k=7 second-cut
  states. On that corpus no scalar geometric feature locates the winner — see the trap above. What
  does work is stacking cheap *sound necessary* conditions, whose selectivity multiplies
  super-multiplicatively. On the first 25 four-part single-class k=7 endpoints, against 2,089,596
  candidates feasible under information cap plus the four-rectangle proven frontier:
  `R_0` full-star majorization on all three children alone gives **16.1x**, cross-part pair
  solvability on all three children alone **6.9x**, and both together **140.4x** (111x if they were
  independent), at 100% winner recall throughout — still 276 candidates per winner, so this
  sharpens the search without solving the choice problem. The cross-part pair condition is **absent
  from the solver**: `radiobase.c`'s `s[4]`/`s[5]` loop is per-part and intra-part only. Its oracle
  is the exact `k=4` two-part table from `refsolve.py` (1,478 states, 231 unsolvable). The
  **140.4x is against my own enumerator, not against `radiobase.c`** — not a solver speedup claim.
  The depth-1 relaxation `R_1` adds 5.8x with full recall but costs 30-80 s per endpoint, and the
  `R_d` ladder converges to exact solving. **Pairs are the right stopping point**: extending the
  cross-part condition to triples adds only 1.71x and 1.65x on the first two endpoints, for
  hundreds of seconds each. All four measurements are reproducible with
  `tools/split_choice_rules.py`.
- **A second solver exists.** `tools/refsolve.py`, written from [problem.md](problem.md) alone,
  no shared code with `radiobase.c`, reproduces the proven columns for k = 1..6 exactly. Slow —
  k ≤ 6 only — but auditable, which is what settles structural questions.

## Infrastructure

Working and worth trusting: `tools/check_tables.py`, `tools/check_witness.py`,
`tools/extract_evidence.py` (`certify` / `audit`), `tools/artifacts.sh`
(`push`/`pull`/`verify`/`check-index`), `tools/check_docs.py`, `tools/refsolve.py`, and the
independent `tools/singleton_direct_split_regression.sh` and
`tools/singleton_tight_band_regression.sh`, plus the
fixed-small-m exact recurrence and complete assembly rank/enumeration modes in
`tools/search_singletonization.cpp`,
including its exact-atom and distinct-slot embedded terminals, together with its guarded-piece
combiner `tools/optimize_mixed_frontier.py` and the exact
`m=5` symbolic calibration `tools/m5_assembly.py`.  The aligned symbolic
track is independently checked by `tools/check_atom_profile_certificate.py` and
`tools/check_atom_profile_tree.py`; `tools/check_dc_tree_lift.py` exhausts fixed projected lifts and
searches alternative skeletons.  `tools/atom_profile_regression.sh` ties the certificates together.

The parallel-solver prerequisite now has an explicit `radio_search_context`: recursive work
budgeting, exact L1 and joint reachability are worker-owned while `canSolveB` remains a compatible
default-context wrapper. The 1,038-answer serial gate, work/CPU scheduler regressions,
reachability regressions and ASan+UBSan checks pass. This deliberately stops before concurrency;
the remaining shared mutation boundary and limited-width epoch plan are in
[parallel-solver.md](parallel-solver.md).

Artifact store `fedork/radio-data` (private): 22 tags, 60 assets plus a manifest per tag,
about 605 MB stored. `sa193-cold-2026-08-16` contains the proof log, matched comparator and final
reproduction metadata; `sa193-frozen-refute-2026-08-18` contains the complete normalized
certificate and all three solver-core replay checkpoints; `run9-level-replay-2026-08-18` contains
both finished level-v2 replays, uncolored and colored, with every manifest and certificate
re-verified before archival; `pareto-census-k8-2026-08-19` and `pareto-census-k7-2026-08-13`
contain the k=8 and k=7 choice corpora, the latter with its three independent replays.
`check-index` is green.
Deliberately **not** archived: ~18 GB of unreliable 2023 `out*` — see the decision in
[data.md](data.md).

Git and `gh` are pointed at the `fedork` account **per repo**, leaving global config alone.
Do not run `gh auth switch`.

## Running now

**Nothing is running.** On 2026-08-31 the only live `Project=radio-sa193` EC2 instance,
persistent oracle `i-002cabc654b2078ed`, was terminated at 14:25:41 UTC at the user's request.
Its build and mixed cache predated the singleton-majorization refutation and must not be reused.
The pre-termination SSM inventory found that the oracle process and transient systemd unit were
already gone; the last complete mutable snapshot was from 2026-08-24. Because its root disk had
`DeleteOnTermination=false`, encrypted 50-GiB volume `vol-04260ae18b515e7f5` remains unattached and
`available`; EC2 compute billing has ended, while EBS storage billing continues. No other pending,
running, stopping or stopped radio-tagged instance remained after the termination query. Exact
commands and disk disposition are in [aws-run.md](aws-run.md).

Four more unattached 50-GiB `oracle-serve` volumes from the superseded 2026-08-21 launches were
also still present. All five volumes, 250 GiB total, were inventoried and left untouched because
the request authorized terminating the instance, not deleting persistent data.

The oracle-prime full-corpus load finished earlier: run `20260820T165448Z` on
`i-0957cf6024c13a1e3` loaded 21,866,180 facts in **1.58 h**, exit 0, and dumped a 6.67 GB snapshot
(667 MiB compressed). Restore verified locally at **32.8 s / 2.41 GB resident**, with 2,200
independently labelled states re-queried to exactly the expected verdicts. The snapshot is a derived
artifact reproducible in 1.58 h from archived inputs, so it stays in S3 at
`s3://radio-sa193-393287594714/oracle-prime/20260820T165448Z/cache.snap.zst` rather than the release
store — see [data.md](data.md). Those historical performance measurements remain valid, but the
mixed snapshot is not a safe warm start after the singleton-majorization refutation.

No `Sa(193)` solver remains. Run3, run8 and run9 all completed all sixteen roots and independently
reported UNSOLVABLE. **The k=8 Pareto-prefix census is finished, archived and its host is gone.** It
exited 0 at 2026-08-19 22:34:43 UTC after 5.87 days on shared `r7iz.4xlarge` `i-0005d74f985c52ae1`,
emitting a `CENSUS END` record and a self-consistent corpus: 55 roots, 817 first cuts (344 strict),
815 second-cut blocks, 7,146 second winners, 1,688 targets, 2,435 upgrade nodes, and 1,893
`ENDPOINT` = 1,893 `FULL_STATE` = 1,893 `FULL_SUMMARY` with 50,494 `FULL_WIN`, matching its `STATUS`
exactly. `representation_blocked=0`, so no result was lost to `MAX_N`. Archived and round-trip
verified as [`pareto-census-k8-2026-08-19`](https://github.com/fedork/radio-data/releases/tag/pareto-census-k8-2026-08-19);
the raw log passes `tools/check_provenance.py` and its SHA-256 matches the one `run.meta` recorded on
the host. Instance terminated 2026-08-20 00:22:20 UTC and volume `vol-045c828c1f0ab2c2e` confirmed
deleted, after an inventory established that every file on the disk had an S3 counterpart. Its
123 M `input.tar.zst` and a redundant progress snapshot stay in S3 only — see
[data.md](data.md) for why and how to promote them.

**The ETA method is worth keeping; the obvious one is not.** The blended 89.8 s/endpoint mean
projected 3.7 h and was wrong by 2.2x, because the measured window was 73% high-mass endpoints and
the remainder was not. Modelling cost as exact solver queries at 0.61-0.837 s each — enumeration
prefixes being nearly free — predicted 7.95-9.76 h; the run took 8.17 h, implying 0.63 s/query and
landing 5.8% below the central estimate. Derivation, band tables and the outcome are in
[../evidence/pareto_census_k8_eta_2026-08-19.txt](../evidence/pareto_census_k8_eta_2026-08-19.txt).
The general trap: **a mean rate over a window whose item mix differs from the remaining work is not a
forecast**, and the cheap test is to sample the progress counter and see whether it moves at all.

**The forced-cut analysis of the completed corpus (2026-08-20).** Full numbers, method and
controls in [../evidence/single_solution_cuts_2026-08-20.txt](../evidence/single_solution_cuts_2026-08-20.txt);
reproduce in ~4 s with `tools/analyze_single_solution_cuts.py`. The findings:

- **The mixed child is always strictly the largest** — 26,876 of 26,876 winning classes across both
  censuses, margin at least 18 (k=8) and 3 (k=7). Controls: 80.5%/77.0% for uniform random splits
  and only 59.0%/57.1% for random splits that already satisfy the information bound. So it is a
  property of solvability rather than of geometry or the cap, and is the one candidate here for a
  free necessary condition. **Conjecture, not theorem**, and measured only on maximal endpoints at
  residual k=5 and k=6.
- **Levels do not connect state-to-state.** A k=8 endpoint sits at k=6, so its children sit at k=5
  where the k=7 census's endpoints live — but of the 1,413 children of the 471 forced four-part
  k=8 cuts, exactly **1** is a k=7 endpoint (under dominance as well as identity). It is not a size
  artifact: 942 of the 1,413 pass both necessary conditions. The cause is shape — k=7 endpoints are
  thin (aspect median 3.20) and mostly four-part, the k=5 states inside forced k=6 solutions are
  squat (2.29) and mostly three-part. The k=7 endpoint family is not an upper set for k=5 solvable
  states. For the degenerate endpoints the picture inverts: 48.5% of 2-part and 34.4% of 3-part
  k=8 children *are* k=7 endpoints.
- **The populations are self-similar in aggregate.** Scaled by `sqrt(cap)`, part size `n*m/cap` has
  median 0.211 at k=8 against 0.206 at k=7 (p25 0.173/0.165, p75 0.257/0.267): each part takes about
  a fifth of the information budget at both levels and four of them fill 85-88%.
- **Nothing scalar separates forced from unforced.** Occupancy (0.867 vs 0.866), child spread and
  diagonality are flat. Symmetry is refuted as an explanation: k=8 four-part endpoints with a
  repeated component are single-class in 2 of 25 cases (8.0%) against 442 of 1,674 (26.4%) for the
  asymmetric ones. The only real enrichment is sliver cuts — parts feeding just two of the three
  outcomes — at 33.4% vs 27.9% (z=+5.3) at k=8, but only z=+1.1 at k=7, so treat it as a lead.
- **Diagonal cuts are rare, and carry no signal for the forced-vs-unforced contrast**: 6.4% of
  forced k=8 part-cuts are exactly proportional, 10.9% at k=7, single and multi indistinguishable.
  Scoped strictly — against *non-winners* diagonality is the strongest single feature found, see
  the learned ranker below.

**A learned ranker does work, as an ordering rather than a filter (2026-08-20).** Numbers and
controls in [../evidence/learned_cut_ranker_2026-08-20.txt](../evidence/learned_cut_ranker_2026-08-20.txt);
reproduce with `tools/ml/cut_ranker.py` (the only scripts here with third-party dependencies).
Counting this as a 624-example problem is the wrong frame: the unit is (state, candidate cut), the
census enumerates every winner so unrecorded cap-feasible cuts are clean negatives, and that gives
26,876 positives against ~1e7 candidates per state.

- Trained on the k=7 corpus **only** and tested on forced k=8 states, a logistic regression ranks
  the winner **428x better than blind**, worst case 6.5x. A permuted-label control on the identical
  pipeline gives 1.6x, which is what rules out leakage. It transfers across levels, so it is not
  memorising states. **Quote the ratio, not a candidate count**: an earlier "median 7 tries" was 7
  of 6,000 *sampled* candidates against a real set of ~3e6, and is superseded by the exact ranks
  below.
- **Exact ranks, by enumerating the candidate set rather than sampling it** (the per-part Pareto
  bound is separable, so this is cheap). All 153 forced four-part k=7 states, median 54,014
  candidates: blind 27,007, ranker **193**, sound `R_0` then ranker **76**, worst case 1,533.
  Top-5 recall is 3.3%. **A "top-5 always contains a winner" rule is ~15x short on the median and
  ~300x short on the tail**, and adding the pair condition addresses only the median.
- **The corpus is not the constraint.** Performance is flat from 26 training states to 534, and
  plain logistic regression beats gradient boosting — both signatures of a smooth low-dimensional
  surface rather than a data-starved one. The feature set binds, not the sample size.
- The elicited rule is small: three features give 57.8x — cut every part close to proportionally,
  balance the two pure outcomes, and leave headroom under the cap in the larger pure child.
- **It is a ranker, not a filter.** No recall guarantee, so it can order a search but never prune
  one; the sound filters keep their role. The per-part Pareto bound from `pareto_sb.csv` is itself
  a sound ~9-12x filter at full recall and is already applied before any of the above is measured.
- Untested where it would matter most: transfer to residual k=7, and composition with `R_0`/pairs.

The mass-descending independent audit is no longer active. Run `20260818T062429Z` reached
251,131/2,576,885 k=7 claims (73,045 four-part), with zero gaps, after 2,160 seconds and
48,049,145,431 nodes. That was enough to confirm that even the optimized independent checker was
repeating more search than the original solver. It was stopped deliberately with exit 130; its
final manifest was hash-checked and instance `i-0b81cd58d3ba14f0c` was terminated.

The replacement frozen solver-core refuter completed on dedicated on-demand `c8a.4xlarge` instance
`i-0cb3783e937115ff1`, run `20260818T074026Z`, from clean commit `e040290`. It loaded the complete
3,126,190-fact normalized certificate into the production dominance trie, prepared split metadata
serially, and published an immutable epoch to sixteen worker-local search contexts. Roots
bypass their own cache entry; children are theorem/CACHE_ONLY queries at k-1; a miss is an exposed
gap rather than recursive solving. All three checkpoints closed: 2,576,885 k=7 claims, 546,744
k<=6 claims and 2,561 k=8..9 claims, all with zero gaps. Their worker epochs consumed 318,771.171
CPU seconds, 76.015% of the complete cold solver's 419,353.1 seconds. The capped phases took
20,113 + 427 + 305 = 20,845 wall seconds (5h47m25s) and peaked at 1.24 GB RSS. The exact S3
manifest and the private GitHub release round trip both pass SHA-256 verification. The release is
[`sa193-frozen-refute-2026-08-18`](https://github.com/fedork/radio-data/releases/tag/sa193-frozen-refute-2026-08-18).
The dedicated instance was terminated after verification; its sole root volume had
`DeleteOnTermination=true` and is confirmed deleted. This is a solver-core validation replay, not an independent proof
implementation; the proof-safe cold run9 remains the proof source.

The complete uncolored level-local replay **finished** on dedicated on-demand `c8a.4xlarge` instance
`i-04126f6d3016378a9`, run `20260818T194508Z`, from clean commit `0f34041`. Its eight populated
level-v2 files cover every one of the 3,126,190 normalized claims at k=2..9; each carries only its
complete k-1 support, checked split hints and target claims. The exact 1,643,619-byte source bundle
has SHA-256 `beb62def6dba281ff1c387c97f70bd0400f8007a99b455b74e784dd8195a654c`.
The same-host 9,995-root k7 gate closed with zero gaps in 53.582 worker wall / 854.158 CPU seconds
and projected 12,860 wall / 204,998 CPU seconds for the four-part band, safely below the cold
solver's 419,353.1 CPU seconds. At 2026-08-18T23:36:50Z it reported `exit_status=0` and
`TOTAL verified 3126190, gaps 0` across all eight independent level-v2 checkpoints. Per-level claims
were 2 / 137 / 33,042 / 125,246 / 388,317 / 2,576,885 / 2,545 / 16 at k=2..9, summing exactly to the
corpus, for 211,335.569 CPU seconds — **50.40%** of the cold proof solver, with k=7 alone 99.2% of
that. It is **archived and verified** as `run9-level-replay-2026-08-18`: `final.sha256` 53/53,
per-level manifests 40/40, all eight certificates decompressing to their manifest hashes at the exact
`level-certificates.meta` byte sizes, and all eight logs passing `check_provenance`. Instance
`i-04126f6d3016378a9` auto-stopped on its idle guard and is ready to terminate.
This replay is validation, not a new proof; exact A/B controls are in
[`../evidence/verifier_level_v2_2026-08-18.txt`](../evidence/verifier_level_v2_2026-08-18.txt).

The requested top-down colored replay is active in parallel on separate on-demand
`c8a.4xlarge` instance `i-0901e2b2c266f7db2`, run `20260818T205010Z`, from clean commit
`e206766`. `RADIO_REFUTE_ENABLE_COLORING` attaches each retained negative Pareto terminal to its
original support-record index; split preparation and sixteen workers write private usage bitsets,
which are merged only after a zero-gap epoch. The strict text selection is checked against both
the original record index and copied `Sb(...)` state before it can generate the next level file.
The exact 1,653,058-byte source bundle has SHA-256
`db198050c5e77ab010952e59200ee22770c769c4c195153edea444854ed7adb1`.

Its same-host 9,995-root k7 coloring gate closed with zero gaps in 55.977 worker wall / 891.641
CPU seconds, projecting 13,434 wall / 213,994 CPU seconds for the complete four-part band; both
automatic guards passed. The first real checkpoint then verified all 16 k9 roots and selected
2,151/2,545 k8 support facts from 123,600 citations. K8 then verified all 2,151 selected claims
with zero gaps and selected 2,508,278/2,576,885 k7 facts from 41,460,414 citations; both checkpoints
were compressed, hashed and uploaded. The dominant k7 phase is now running 2,508,278 targets on
all sixteen workers after loading its 388,317-fact k6 support in 3.210 seconds and freezing 692
tables / 355,174 options in 0.090 seconds. Its k7 phase passed 2,443,493/2,508,278 claims
(97.4172%) with zero gaps at 13,500 batch seconds and 3,182,063,023,008 accepted prefixes; the early
1,284-second ETA was not a whole-phase forecast, the phase took roughly ten times that. The run then
**finished** with `exit_status=0` and
`TOP_DOWN_COLOR verified_top=16 levels=8 audited=2846568 terminal_level=2 terminal_used=0 gaps=0`.
So the top-down colored chain closes: eight levels, 2,846,568 audited claims against the complete
replay's 3,126,190, and an explicit `used 0` terminal at k=2. All eight level certificates
(`run9-k2..k9.cert.zst`), the per-level selections, the binary, its provenance and the exact source
bundle are staged under
`s3://radio-sa193-393287594714/run9-colored-refute/20260818T205010Z/`.

**Top-down coloring does not pay, and that is now measured rather than hoped.** The compression is
only **8.94%** (3,126,190 claims to 2,846,568) and it lands in the wrong place: the dominant k=7
level is **97.3% cited**, while what compresses is k=6 (59.4% retained) and k=5 (64.4%), together
0.5% of the cost. The colored replay therefore spent **218,792.627 CPU seconds against the complete
replay's 211,335.569 — 3.5% *more* to verify 8.94% fewer claims**, because at k=7 it audits nearly
every claim *and* pays citation tracing (1.18 trillion k=7 citation hits). Per level it is cheaper at
k=6 (0.69x) and k=5 (0.64x) but 1.04x at k=7. The durable finding is structural, not about either
implementation: **the run9 negative certificate is close to minimal at the only level that costs
anything**, so there is no large dead-weight subset to strip. Both coloring designs — the retired
independent checker and this citation tracer — have now been measured; do not spend more on coloring
this certificate.

**A third A/B point is running to separate instrumentation cost from claim reduction.** Run
`20260819T013030Z` on the kept instance `i-04126f6d3016378a9` puts the *ordinary* verifier over the
*selected* input, completing the table: complete+ordinary 211,335.569, selected+colored 218,792.627,
selected+ordinary in flight, predicted ~205,111. It reuses the baseline `run9_refute` binary and
`/root/source` unchanged — coloring is a compile-time `#ifdef`, so that binary is exactly the ordinary
one — meaning the input file is the only difference. The seven cheap levels closed as a gate at exactly
their selected claim counts with zero gaps, totalling 1,044.052 CPU s against the complete replay's
1,625.067. **An early k=7 counter-signal:** at 60 seconds this run had done 113,683 claims over
15,896,943,271 prefixes where the complete replay did 140,144 claims over 15,623,138,715 — 19% fewer
claims for 1.8% more prefix work, i.e. the retained cited claims look harder per claim than the ones
coloring dropped. An early rate is not a whole-phase forecast, so this is a signal, not a result; if it
holds, the selected input is not cheaper and the coloring negative is complete. Live state is
`tools/run9_selected_ordinary_status.sh 20260819T013030Z`; staging is
`s3://radio-sa193-393287594714/run9-selected-ordinary/20260819T013030Z/`.

**All four points are now measured and the programme is closed.** Points 1, 3 and 4 share one host and
one binary, so only their inputs differ; point 2 used the colored build on a different host and is the
least comparable row.

| # | input | claims | k=7 support | verifier | CPU s | vs #1 |
|---|---|---|---|---|---|---|
| 1 | complete | 3,126,190 | 388,317 | ordinary | 211,335.569 | 1.0000 |
| 2 | selected | 2,846,568 | 388,317 | colored | 218,792.627 | 1.0353 |
| 3 | selected | 2,846,568 | 388,317 | ordinary | 202,592.331 | 0.9586 |
| 4 | trimmed | 2,846,568 | 230,725 | ordinary | 201,982.710 | 0.9557 |

Citation tracing costs **+8.0%**; trimming claims buys **-4.14%**, trimming support a further
**-0.30%**, all of it **-4.43%**. Both new runs closed with zero gaps over all 2,846,568 claims.

**The 40.6% support reduction was 99.7% illusory** — this is the finding. Of the 388,317 complete k=7
support facts, **156,927 were already discarded as redundant** during Pareto-front construction, so
only 231,390 ever entered the structure against the cited 230,725: a true live reduction of **665
facts, 0.29%**. The dominance front was already doing the trimming for free at load time, and
`redundant` dropping to exactly 0 in the trimmed run is the tell. Structures shrank 2.3-2.5% and the
whole measurable saving is 0.93 s of cache build out of ~201,000 CPU s. #3 and #4 share
`split_checksum=02f5ed6cbfc31d94` and identical prefix totals, so that 0.30% cleanly isolates the
support effect. Even the claims trim's saving is misread as compression: at k=7 prefix work fell only
**0.16%** while the prefix *rate* rose 4.0% as the split tables shrank 772 to 692 — locality in split
preparation, not less search.

**Verification cost is prefix enumeration, not fact lookup** (3.22 trillion prefixes against 1.18
trillion citation hits, 0.37 lookups per prefix), so compressing the certificate attacks the wrong
term. **Do not revisit certificate coloring or compression as a performance measure**; both designs
are now measured end to end and the ceiling is a property of the proof and the engine. If verification
throughput matters again, the target is prefix enumeration. `--support-selection` is sound and
retained, but its use is *measuring* how much of a certificate is live, not speeding anything up.
Archived and verified as `run9-verifier-ab-2026-08-19`; the chainer stopped the host as designed.

Both replays are **archived and verified** as `run9-level-replay-2026-08-18`. For the colored chain,
each level's `used` count is the next level's `audited`, the chain terminates with an explicit
`used 0` at k=2, and an independent check written against the archived files — resolving every
`claim` through each certificate's own part table — confirms every colored level's claim set is a
subset of the corresponding complete level's, sharing source hash `3ad5877a...`. Both instances
(`i-04126f6d3016378a9`, `i-0901e2b2c266f7db2`) auto-stopped, never billed compute past completion,
and never billed compute past completion. `i-0901e2b2c266f7db2` (colored) is now **terminated** and
its volume `vol-0bdc1e36eea39386c` is confirmed deleted, after checking that its disk held nothing
unarchived — every file was in the release, a decompressed twin of an archived `.zst`, transient run
state, or `run9.cert`, which is archived in `sa193-frozen-refute-2026-08-18` and whose SHA-256 was
verified here to equal the `3ad5877a...` source hash every level certificate cites.
`i-04126f6d3016378a9` is now **stopped**, its A/B points complete and archived, and it is the last
radio-tagged instance in the account; terminating it needs the same disk check its sibling got. Both replays
remain solver-core validation and certificate compression, not
independent proof implementations, and neither retroactively rehabilitates the old Sa(113) colored
certificate — that trap stands.
Live state is `tools/run9_color_refute_status.sh 20260818T205010Z`; staging is
`s3://radio-sa193-393287594714/run9-colored-refute/20260818T205010Z/`. This is still solver-core
validation and certificate compression, not a new independent proof. The local matched gate and
sanitizer record is
[`../evidence/verifier_coloring_citations_2026-08-18.txt`](../evidence/verifier_coloring_citations_2026-08-18.txt).

The canonical-order before run `20260818T055255Z` passed the same sample with zero gaps in
23,697,303,379 nodes / 1,627.30 seconds. Its superseded full phase was immediately stopped through
the capped wrapper; exit 130 and every final-manifest hash were checked before instance
`i-066a6cd0b7f66d581` was terminated. It cost approximately $0.44 and did not touch the shared
census host.

Both optional run9 coloring attempts were intentionally interrupted on 2026-08-18 with exit 130,
after their supervisors uploaded final diagnostics. The old fourteen-worker shared-host build had
spent about 12h28m in coloring and had only closed the k=9/k=8 barriers; it had no intra-k=7 cursor.
The progress build ran its k=7 color phase for 11,460.1 seconds and completed 119,649 of 2,505,858
targets (4.7748%): every one-, two- and three-part target, but only 10,312 of 2,396,521 four-part
targets. All completed targets verified, with zero unresolved/budget outcomes, after
105,605,161,144 recursion nodes. No colored certificate or replay result exists.

The progress run's final S3 manifest and compressed logs were streamed back and hash-checked. Its
dedicated `c8a.4xlarge` instance `i-01f8c56b7a53a1178` and 30-GiB root volume were then terminated;
the shared census instance was untouched. That independent-checker coloring design remains
retired: it re-solved each negative state and made coloring slower than the solver. The active
replacement colors the already-gated frozen production-trie refuter by tracing exact cache
citations; it does not revive the retired checker or trust either interrupted partial result.
Exact operational paths and disposition are in [aws-run.md](aws-run.md).

| prefix / build | freshness | last reported state |
|---|---|---|
| `run/` — original | stale; solver gone | 2,568,394 verdicts, 0 of 16 |
| `run2/` — A+B | stale; solver gone | 1,897,635 verdicts, 5.72 GB, 0 of 16 |
| `run3/` — A+B + full-star majorization | completed; performance only | 3,319,030 raw lines; 16 of 16; 479020.9 CPU s; 25.57 GB peak RSS |
| `run4/` — compact cache at frozen commit `6af384e` | stopped, archived; old scheduler | 103,773 verdicts, **0.29 GB**, control never returned |
| `run5/` — compact cache + exact L1 at frozen commit `290a892` | stopped, archived; old scheduler | 103,769 verdicts, **0.29 GB**, control never returned |
| `run6/` — broken deadline experiment at `c13b5d3` | stopped, archived | control SOLVABLE in 922.0 s; 618,816 raw lines, 1.37 GB peak RSS |
| `run7/` — progress-gated pass-2 dive at `e648e83` | stopped and archived; obsolete scheduler | 104,931 verdicts; control never returned; **0.29 GB** peak RSS |
| `run8/` — compact cache + bounded probes at `9395218` | completed; performance only | 3,173,928 raw lines; 16 of 16; 412561.4 CPU s; 1.32 GB peak RSS |
| `run9/` — rb-safe contraction at `e7fa747` | **completed; proof source** | control SOLVABLE in 479.2 s; 16 of 16; 419353.1 CPU s; 1.32 GB peak RSS |

The finalized comparison and raw hashes are in
[`../evidence/sa193_run_comparison_2026-08-16.txt`](../evidence/sa193_run_comparison_2026-08-16.txt).
Run9 was 1.646% slower than the matched run8 comparator and 12.456% faster than run3, with no
run8/run9 sign disagreement across their 3,160,113 common parsed facts. `tools/sa193_status.sh`
now reads final snapshots; `--watch` is no longer needed for these solvers.

Every run in this table is a frozen process-CPU-budget binary. The 2026-08-13 deterministic-budget
default does not alter or restart any of them. New work-clock verdicts append `work=<units>` and
`rate=<units-per-nominal-second>`; the comparison tool uses that effort for attempt accounting, keeps
`took` for actual CPU/self time, and marks a ratio approximate when the two sides use different
budget bases.

Run8 started cold at 2026-08-11 22:46:06 UTC with the `Sa(192)` control enabled. Its full embedded
commit is `9395218dcbdd90d8f6a208b15da1878ff75f6ee1`. Its 60 GiB wrapper and run3's 40 GiB wrapper
formed the original two-run envelope; run9's combined guard capped all three solvers at 108 GiB.
The source archive, frozen binary, sidecar and `run.meta` are retained under `run8/`; hashes and SSM
command IDs are in [aws-run.md](aws-run.md).

Run9 started cold at 2026-08-12 03:21:12 UTC from `e7fa747264476461a234bf78e49762ee77ad8d8d`.
It changes only the unsound interaction: once `rb_dead` actually rejects a partial assignment, that
invocation retains an exact full negative but cannot materialize an implicit shorter negative. Each
such event prints `contraction=rb-suppressed:<size>`; its five-minute status reports the count and
latest state. The exact source archive, binary, build sidecar and launch metadata were hash-verified
through S3. During execution, run9 had a 60 GiB individual guard and a separate **108 GiB
combined-solver guard** would have stopped it first, preserving run3/run8 and about 15 GiB of host
headroom. Neither guard fired.

Run8's mandatory remote happy-path gate passed:
`result CONTROL Sa(192) in 10 = SOLVABLE (471.6 s)`.
This is 0.872x run3's 540.7-second control on the same host. Run8 subsequently completed all roots,
but its pre-fix negative cache leaves it classified as a performance comparator only.

Run9's independent cold control also passed:
`result CONTROL Sa(192) in 10 = SOLVABLE (479.2 s)`. It then completed every `Sa(193)` root and
returned UNSOLVABLE in 419353.1 CPU seconds. No tainted contraction was suppressed anywhere in the
final log. This is the proof-source execution.

During execution, the run8 watchdog scanned the run3 and run8 raw prefixes every five minutes with bounded state. It chose the
run that is behind by completed roots and verdict count, ranks its six slowest completed exact
states, and joins `(state,k)` keys in the peer log. A verdict's `took` covers only its final
activation, so the ranking and first timing column now add the last visible `elapsed` value from
each earlier progress episode. Historical `MAYBE` returns have no exact timestamp: `≥` marks this
attempt-sum floor, `(2a)` means two visible attempts, and a ratio involving a floor is prefixed `~`.
Per-call `~self-final` still subtracts all k-1 verdict time since the previous k verdict and applies
only to the final activation. `MAYBE` and cache effects make it approximate, and the first call at
each level has no left boundary. The compact view keeps the recursive stack and a per-run level
profile, but no side-by-side level comparison. The profile adds the visible elapsed time of current
`still solving` frames before taking level differences. In new work-budget logs, `elapsed` is nominal
work effort and the line separately appends `cpu=`; the level profile uses that actual CPU field.

Run3 ultimately reached 25.57 GB peak RSS and 3,319,030 raw lines. The stale `run2` snapshot is not a matched-time comparison,
so it does not by itself identify the cause; it does show that memory per verdict cannot be assumed
stable across the changed search shape.  `run3` still has the old unbounded dense result trie; the
current compact representation remains unbounded but is over an order of magnitude smaller on the
retained checkpoint.  The level-lazy split-table and cache changes are not deployed in `run3`.
Do not restart a cold run solely to pick them up.

Run7 resolves the ambiguity left by run4/run5. At retirement it was in exhaustive pass 2 of their
same information-tight 14-part k=5 state after 280,116,882,707 prefixes. The call had a finite
parent, but `e648e83`'s pass-2 rule made the child `NO_DEADLINE`; its required negative-cache count
could not advance when the cache was saturated. The 20,460-second activation was therefore not a
healthy mandatory dive. Run6 represents the opposite failure: its poll could return before a
complete child was tried. Printed definitive verdicts from either build remain sound, but neither
timing profile is a baseline.

Commit `45c34fd` replaces both extremes without remembering a split.  Finite children share an
absolute parent bound and may return `MAYBE` with no mutation.  One/two-segment states retain that
shared allowance; longer states start with two-second speculative children and double the quantum
after an unresolved exhaustive pass.  The exact stuck k=5 state now returns `MAYBE` in 1.000 CPU
second with zero new negatives.  A normal genuinely cold `Sa(192)` control completed in 376.293 CPU
seconds (382 s wall, 0.18 GiB peak RSS), following `[48:32]`, `[16:15,45:23]`, then
`[17:8,27:13,10:8,12:0]`.  The 1,038-answer regression and ASan+UBSan gates match prior main.

The 2026-08-13 scheduler change preserves that state machine but replaces process CPU as the default
allowance clock. One accepted per-part split prefix is one deterministic work unit across the whole
recursive call tree; 20,000,000 units are one nominal second, so the initial long-state quantum is
40,000,000 units. Two cold controls stopped at exactly 2,000,001 units in repeated runs and emitted
identical ordered cache facts despite differing CPU time. The old clock remains available as
`-DRADIO_CPU_BUDGET`. See
[`../evidence/work_budget_rb_root_2026-08-13.txt`](../evidence/work_budget_rb_root_2026-08-13.txt).

The proposed eager `rb_dead(0,0,0,0)` test was also measured and is **not enabled**. It is an exact
first-test mass relaxation after theorem-filtering each part, but `ALIVE` does not solve any child.
All 16 retained exhaustive multipart states passed it, including the sole known negative. In an
independent 283-state k=3 census it added 5 refutations after parent star-majorization, while an eager
hook saved only 0.091% recursive work on the hard k=5 positive and slightly slowed fixed-work probes
of the saturated fourteen-part state. `tools/rb_root_probe.c` retains the diagnostic; ordinary search
keeps the measured-cost trigger.

That probe now also computes the exact hereditary pliable tail from the already-built `rb_mx`
tables.  If `exact_head=i`, `rb_dead` cannot reject at suffix `i` or later.  A sound cheaper theorem
uses absolute slack, a `(2:1)` base, retained pure corners and the extension inequality
`2w<=T+slack+2`; a length-only corollary uses the tail excess above mass two.  In the complete small
census, 65 of 243 parent-theorem survivors had no useful production call site, but the direct cheap
theorem proved only 29 (the coarser length bound only 10).  Slack is the correct proof parameter but
does not determine the cutoff by itself.  The hard eight-part positive has only a one-part pliable
tail, while the saturated fourteen-part state at zero slack has none.  No production policy changed;
the exact scan and reproducible census remain diagnostics. See
[`../evidence/rb_pliability_2026-08-13.txt`](../evidence/rb_pliability_2026-08-13.txt).

The proposed per-depth follow-up is now measured.  Keeping the full absolute slack strengthens the
q/D corollary to `2(D-q)<=slack-4`; it improves 11 partial cutoffs in the small census but no
additional complete no-call state.  A forced cold census made 1,590 actual calls and 259 rejections:
the rejection rate fell from 49.47% at slack zero to none in 291 calls at slack four or five, while
`slack-D>=2` had no rejection in 230 calls.  These are useful priors, not theorems; `slack-D=1`
already has a reached counterexample.  On the real hard positive, the exact tail cutoff removed
33.05 M of 42.43 M lookups while retaining all 2.94 M rejections and identical deterministic work,
yet five paired timings showed no stable CPU benefit.  The saturated state instead made 1.14 M
useful rejections in a 40 M-work probe.  Therefore the measured-cost trigger remains and no cutoff
is enabled; the profiler and rejected exact switch are reproducible research modes. See
[`../evidence/rb_slack_profile_2026-08-14.txt`](../evidence/rb_slack_profile_2026-08-14.txt).

Raw validation and rejected-experiment logs are archived as `bounded-probe-2026-08-11` and
`bounded-probe-rejected-2026-08-11`.  Full control flow, build IDs and the two-stage correction are
in [`../evidence/deadline_stall_2026-08-10.txt`](../evidence/deadline_stall_2026-08-10.txt).
Run3 remained untouched through completion. Run7 and the local `e648e83` continuation predate embedded
`radio-provenance-v1`; both were stopped only after their source/launch metadata and raw segments
were preserved. Run7's finalized S3 raw stream matches the retained EBS file exactly: 104,936 lines,
11,065,274 bytes and SHA-256
`a79d31d9b11bf97679451087b90978f7fdc3b8874847bda2ebca305142ddb72c`. Its final checkpoint,
source archive, frozen binary, final profile, stderr, watchdog log and `run.meta` are under `run7/`.

The completed common prefix does give a clean throughput warning.  Matching negative `Sb` verdicts
by exact printed state and level found 98,253 common calls.  Across the k=8, k=7 and k=6 groups,
`run4`'s summed inclusive per-call `took` values were respectively 1.246x, 1.247x and 1.264x
`run3`'s.  The slowest six completed `run4` refutations were all one-part k=8 states and each was
slower by 19-33%, even though its local `totalsplits` count was 25-30% smaller.  At k=7 and k=6 the
aggregate split counts are essentially identical, so reduced split construction is not hiding the
cost.  Treat this as an approximately 25% cost for the *whole compact build on this natural cold
path*, not as an isolated Pareto-front lookup measurement: `took` is inclusive and the two builds
also differ in split storage/filtering and code layout.  Exact states and snapshot hashes are in the
2026-08-10 journal entry.

That measurement motivated an isolated profile and is no longer the lookup-cost expectation for
current `main`.  A bounded exact-state L1 now probes before repeated bundled majorization and
the compact dominance trie.  On the fixed warm four-part control it takes **26.6** solve seconds,
versus 42.6 for the first compact build and 33.0 before compaction, with the identical witness and
37,899 local splits.  The full `Sa(192)` gate is SOLVABLE in **711.7 CPU seconds**, versus 819.9 for
the first compact build and 734.5 before compaction, at 0.35 GB peak RSS.  All 3,379,067 verdict
bytes in the combined-checkpoint regression match compact baseline c146d9d exactly.  This fixes the
measured tax for current runs; frozen `run4` does not contain it, while stopped `run5`/`run6`/`run7`
do.

The proposed matched state `Sb(48:48,64:33)@8` now has definitive matching refutations. Run3 first
logged an abandoned episode through 2,602 seconds, then much later returned `FALSE` in a 14-second
warm retry; run8 used an approximately 999-second bounded probe and returned `FALSE` on a
1,181-second retry. Both definitive calls report the same 53,834 admitted split combinations. The
observable attempt floors are therefore `run8 ≥ 2,152 s` and `run3 ≥ 2,616 s`, not the misleading
final-activation ratio `1181/14 = 84.36`. More importantly, the enclosing `Sb(112:81)@9` reached the
run8 refutation and entered pass 2 at 4,881 CPU seconds; run3 reached the same refutation immediately
before its 155,329-second root verdict. That is a 31.8x earlier phase transition, not yet an
end-to-end root comparison: run8 remained in pass 2 at this snapshot. Exact raw line numbers and
the lower-bound reconstruction are in the 2026-08-12 journal entry.

A current-main local trial (`713b7d6`, M4 Pro / 24 GB) was stopped deliberately rather than left
to swap.  The cold `Sa(192)` control passed in **734.5 CPU seconds**.  In 1,152 wall seconds the
process emitted 232,725 verdicts, but after entering `Sa(193)` its `vmmap` footprint reached
**7.1 GB**, of which **5.9 GB was swapped**, while the wrapper reported only 2.77 GB peak RSS.
CPU utilisation had fallen to about 44% and no top-level k=9 state had completed.  This is an
abort, not a refutation.  It shows that the level-lazy split tables do not by themselves make a
full 24 GB local run practical.  This is now the pre-compaction baseline, not the operational state
of current `main`.
The raw log is archived as `sa193-local-2026-08-10:out_sa193.txt` and its same-run parsed checkpoint
remains locally at `/Users/fedor/radio-runs/sa193-local-713b7d6-trial1/sa193.checkpoint`.

Checkpoint replay now explains the footprint.  The k=5..7 cache roots reserve **5.20 GiB** for
90.7 million live transitions; the 232,725 exact facts occupy only 8.0 MiB in parsed form.  Two
different multipliers are involved: eager dominance-closure materialisation (especially positive
k=5/6 facts) and sparse dense arrays (especially negative k=7).  The exact shape counts and discarded
intermediate layout models remain in
[`../evidence/cache_shape_sa193_local.txt`](../evidence/cache_shape_sa193_local.txt).

The local design is now deployed: maximal-positive and minimal-negative Pareto fronts store the last
part at every exact prefix, with tagged uint32 child descriptors and inline singleton fronts.  The
same k=5..7 replay requests **499,877,916 bytes**, down **11.174x / 91.05%** from 5,585,395,760;
the process peaks at 0.60 GB RSS.  Across 4,164,958 exact, mutated and deterministic random queries,
every old verdict is preserved and 4,622 `MAYBE` answers become sound positive-front hits.  The full
`Sa(192)` control now remains SOLVABLE in 711.7 CPU seconds after adding the 2 MiB exact-state L1,
versus 819.9 for the first compact build and 734.5 before compaction; peak RSS is **0.35 GB**.  This
made a bounded local `Sa(193)` continuation credible without retaining the earlier CPU premium;
the later AWS run9 completion supplied the actual full refutation and measured peak.
Implementation measurements and commands are in
[`../evidence/cache_last_front_2026-08-10.txt`](../evidence/cache_last_front_2026-08-10.txt).

The genuinely cold local segment at commit `7ceb59d` ran from 2026-08-11 01:12:42 to 05:47:30 UTC
in `/Users/fedor/radio-runs/sa193-local-front-7ceb59d-cold2`.  Its control passed in 807.7 seconds;
it then emitted 485,337 log lines while holding about a 1.3 GiB physical footprint.  It was stopped
under the first deadline diagnosis.  Run7 later confirmed that the inherited progress-gated
pass-2 policy can indeed lose its finite escape, although the original run4/run5 snapshots alone
had not proved that.  The supervisor exited 143 and generated a
17 MiB final same-run checkpoint (SHA-256
`3b8622f4d1cc342f28c93626e6554d2c7ca8da8ff0582c993ceeca6e19c73ae2`).  This is an interrupted
segment, not an `Sa(193)` verdict.

A corrected same-chain continuation ran in
`/Users/fedor/radio-runs/sa193-local-depth-e648e83-resume3` from 2026-08-11 15:49:16 to
22:12:33 UTC on commit `e648e83`. It loaded the 931,075-line checkpoint folded from every earlier
segment, reproduced the positive control from cache, and added 188,172 raw lines before deliberate
retirement. Its scheduler is obsolete; do not use its slow-state timing as evidence about `45c34fd`.
The old supervisor's `vmmap` probe itself hung for 19 minutes, freezing both its memory guard and
checkpoint cadence; merely bounding it showed that it still timed out consistently.  The guard now
reads macOS `top`'s documented physical-footprint field, with its own 20-second timeout, and its
first sample succeeded.  Resumed checkpoints fold inherited facts into each new file, so another
restart does not silently forget an earlier segment.  A final proof must retain all raw logs; the
continuation alone is not a closed derivation.

Operational guard outcome: while adding future provenance, the primary supervisor script was edited
in place despite the documented Bash re-read trap. As expected, the primary exited without its
completion marker after solver 19088 was terminated. Independent recovery guard 28814 then folded
the exact inherited checkpoint plus the final raw segment into `sa193.recovery.checkpoint` and
exited. The recovered file has 1,118,898 lines, 42,433,056 bytes and SHA-256
`ba6ba91fdad83681b36a1b79c126060dc1607857cdf57ed34b18bb3ca65e2f7a`; its body matches a fresh
`source checkpoint + parse_out.sh(raw)` reconstruction byte for byte. PIDs 19088, 19059 and 28814
are all gone. Every raw segment and checkpoint remains on disk; nothing was deleted.
The four raw segments, closed checkpoint and launch/runtime sidecars are also archived under
`sa193-local-chain-2026-08-11`; the explicit legacy-provenance classification is recorded in
[data.md](data.md).

The sibling `cold1` directory is an empty launcher failure: a managed one-shot shell reaped both
descendants despite `nohup`; it contains no solver result and must never be used as a checkpoint.

Historical lesson, retained without treating the old processes as live: before full-star
majorization, almost all measured CPU was in near-saturated 8-part k=6 states.  Full-star
majorization removes that mode strongly enough that `run3` is instead dominated by k=7.  Optimising
only the old k=6 monsters is therefore no longer a complete plan for the current engine.

## The Sa(193) certificate

The primary computational certificate is complete. A verified witness proves `Sa(192)` solvable;
the cold run9 log exhaustively rejects every `Sb(n1:193-n1)@9`, `n1=97..112`, in the only possible
first-test range. The raw source is
`sa193-cold-2026-08-16:run9_out_sa193.txt`, and the compact root excerpt with hashes is
[`../evidence/sa193_unsolvable_in_10.txt`](../evidence/sa193_unsolvable_in_10.txt).

Run9 began from an empty cache, stayed in one session, passed its positive control, carried complete
embedded provenance and used the contraction-safe build. This avoids both defects that made the
2023 result unusable: inherited unarchived facts and false implicit shorter negatives.

`radio_verify.c` is **superseded** by `tools/cleanroom` (2026-09-01), which is the independent
implementation this section was hoping for: it closes the whole certificate of record with zero
gaps and agrees with the production engine on total work to 0.46%. `radio_verify.c` never finished
run9 k=7 — it was stopped at 251,131 of 2,576,885 claims — and its colored output remains an active
trap. What follows is retained as measurement and as the history of the attempt.

It was an experimental independent strengthening path, not a prerequisite for the
current `proven-exhaustive` classification. It shares no solver search code and reported the whole
normalized `Sa(113)` k=9 ladder—304,105 negative facts across k=2..8—with zero unverified; the new
solver-core refuter (separate from that checker, but deliberately sharing solver code) also closes
the full normalized corpus. However,
the same refuter exposes nine uncovered splits in the 120,302-record colored Sa(113) subset which
`radio_verify.c` reported closed. Until that discrepancy is diagnosed, do not promote its colored
replays as evidence. Its pthread scaling, parallel pre-color antichain reduction, explicit root
records and strict human-readable `radio-negative-certificate-v1` format remain useful engineering.
On the retained 62,366-fact `out_k7.txt` corpus, one through sixteen workers returned the identical
97,483,464-node result, with wall time falling from 14.13 to 2.79 seconds. Full measurements are in
[`../evidence/radio_verify_parallel_2026-08-16.txt`](../evidence/radio_verify_parallel_2026-08-16.txt).

The full explicit-root `Sa(113)` pipeline remains a completed performance benchmark. On an isolated
same-type AWS host using the exact live-run9 verifier binary, 304,105 normalized facts became a
3,953,000-byte readable certificate with 9 roots and 120,528 support facts. Independent replay
reported all 120,537 records and exactly 2,491,817,467 recursion nodes with zero gaps, but the
newly exposed colored-support discrepancy means this is not presently a proof result. The 14-worker
sanitize/round-trip/color/replay stages took 0.30/0.24/375.04/369.57 seconds and peaked at
1,043,216 KiB, just under 1 GiB.
Replay at 8/14/16 workers took 434.86/369.57/347.91 seconds: sixteen minimizes wall, eight uses CPU
most economically, and fourteen is the measured shared-host compromise. The dominant k=6 batch
retained 60,738 of 65,371 minimal facts and consumed 99.10% of coloring's per-level verification
wall, so text parsing and further artifact coloring are not optimization priorities. Full evidence
and the private release URL are in
[`../evidence/verifier_pipeline_benchmark_2026-08-17.txt`](../evidence/verifier_pipeline_benchmark_2026-08-17.txt).

Run9 itself has passed the parse/format gate: both verifier parsers extract 3,126,190 canonical
negative records, and the 106 MB readable certificate round-trips byte-identically (7.19 MB at
`zstd -19`). A bounded top-layer pass verified all sixteen `k=9` roots and cited all 2,545
canonical `k=8` facts; it stopped before checking any of those facts. This is not a proof replay.

Two end-to-end coloring attempts established why the current design should not be used for full
run9. Both reproduced the 3,126,190-record normalized input and the 2,507,270-fact minimal k=7
level. The old fourteen-worker build closed k=9 and k=8, which cited 2,506,515 k=7 targets—99.97%
of that minimal level—but exposed no finer progress. The later sixteen-core build made progress
observable and confirmed that the bottleneck was computation rather than a hang: active cursors
advanced and tasks turned over while throughput collapsed after the three-part region.

The progress run was stopped after 11,460.1 seconds of k=7 coloring at 119,649/2,505,858 targets.
It had verified all 108,083 three-part states but only 10,312 of 2,396,521 four-part states, spending
105,605,161,144 recursion nodes with zero unresolved or budget outcomes. Its final one-minute rate
was 0.450 target/s. The older run was stopped at the same time after about 12h28m in coloring. Both
exited 130 by request, their final diagnostic manifests were hash-checked, and neither emitted a
complete colored certificate or began proof replay. Exact records are in
[`../evidence/run9_verifier_aws_2026-08-17.txt`](../evidence/run9_verifier_aws_2026-08-17.txt) and
[`../evidence/verifier_progress_2026-08-17.txt`](../evidence/verifier_progress_2026-08-17.txt).

Full coloring is therefore deferred, not merely waiting for a faster instance. A useful compact
certificate should eventually carry enough branch-level derivation that an independent checker
validates recorded choices and coverage instead of solving millions of negative states again. Only
after that check is cheap should top-down reachability coloring return as an artifact-pruning pass.
This optional strengthening remains outside the established `Sa(10)=192` proof dependency.

The packed product-profile columns remain deployed, but the adaptive fixed blocks are now a
retained control rather than the production hierarchy. Canonical facts remain untouched for stable
hashes and text output. On levels with at least 65,536 facts, a balanced immutable kd tree
partitions total mass, four sorted products, eight sorted n sides and eight independently sorted m
sides. Each node stores componentwise minima. A failed minimum soundly rejects every descendant; a
fitting 32-fact leaf still reaches the original packed filters and exact injection matcher. The
same exact lookup, excluding self, accelerates same-level minimalization. No implied fact is
inserted.

On the exact hard run9 k=7 root, the final product-only and block builds returned identical
4,644,469 nodes, 5,583,390 memo hits and 5,187,272 misses. Verifier wall fell from a contemporaneous
39.16 to 11.70 seconds (**3.35x**), after the earlier product index had reduced the retained legacy
209.63-second control to 33.24 seconds. The summaries add 45.1 MiB and rejected 98.0% of 271,663,392
block probes, skipping 68,141,963,520 positions. The cutoff matters: an ungated block build slightly
regressed the full Sa(113) replay, while the final small-level control preserved exactly
251,437,448 nodes and took 15.88 versus 15.95 seconds. The then-current checker reported all
120,302 colored records closed in 2,491,283,058 nodes; the newly exposed nine-gap discrepancy
reclassifies this as a performance control rather than proof validation.

The kd hierarchy reduces the exact hard-root fact probes from 5.509 billion to 431.317 million and
verifier wall from 11.70 to 4.20 seconds with identical proof/memo counts. Bounded forward checking
through 512-option lists then reduces the five-root control from 9,158,686 to 4,690,828 nodes and
from 21.00 to 5.34 seconds. Pair rows have a 128-MiB-per-worker fail-open ceiling. Complete run9
k=6/k=7 antichain passes reproduce 229,341/2,507,270 minimal facts in 3.8/49.8 seconds. A twelve-
worker Sa(113) guard reported all 120,302 colored records closed and exactly 2,491,283,058 nodes;
that verdict is superseded by the solver-core refuter's nine explicit gaps.

The missing traversal control was segment mass in the opposite direction from the rejected
mass-ascending experiment: enumerate parent parts by descending mass, then descending long side.
This is a sound permutation of the same Cartesian product. On twenty roots spread across all
2,398,799 k=7 four-part facts it reduced canonical-n search from 41,945,991 nodes / 44.88 seconds
to 5,336,038 / 7.48 seconds. A 100-root spread sample reported 100/100 with zero gaps in 30,978,940
nodes / 40.58 seconds. A complete twelve-worker Sa(113) colored replay reported 120,302/120,302
while dropping from 2,491,283,058 nodes / 119.19 seconds to 330,226,371 / 25.10 seconds; use these
only as traversal-cost measurements. The new order is now the local production default; every supported
order agrees on the committed closed multi-part regression fixture, and a forced-kd ASan+UBSan run
closed five run9 roots without an error.

That optimization justified a bounded ordinary run9 audit, but its partial full phase confirmed
that the independent checker still repeats too much search. The active replacement reuses the
solver's exhaustive traversal against a frozen negative trie; its AWS gate compares projected CPU
directly with the complete cold solver cost. Proof-carrying split coverage remains the longer-term
independent-checker direction. Source hashes, failed layouts,
tuning and sanitizer controls are in
[`../evidence/verifier_product_index_2026-08-17.txt`](../evidence/verifier_product_index_2026-08-17.txt)
and
[`../evidence/verifier_block_pareto_2026-08-17.txt`](../evidence/verifier_block_pareto_2026-08-17.txt),
with the current hierarchy in
[`../evidence/verifier_kd_index_2026-08-18.txt`](../evidence/verifier_kd_index_2026-08-18.txt).
Solver cache structure remains a separate experiment.

The abandoned 2023-corpus painting and sixteen missing-k=8-fact programme is superseded: it was a
way to rehabilitate a resumed, non-closed log. The new cold log is closed by construction.

## Where P6 stands — full star expansion is the structural rule

The long-state work has a theorem-backed improvement, not another fitted heuristic. The
**Vertex-Splitting Pullback Lemma** says that `(n:m)`, `n>=m`, may be lifted to `m` disjoint copies of
`(n:1)` by cloning the wide shore. Pulling tests back to all clones preserves edge transcripts, so
every solvable state must have its mass-preserving profile

    sort(n_1 repeated m_1 times, n_2 repeated m_2 times, ...) <=_w G_k.

On an exact-L1 miss, `radiobase.c` applies this full star-expansion majorization before the
dominance trie; the independent C verifier and Python certificate prototype apply the same lemma
separately. This **corrects** the
earlier claim that the `majtight>1` cutoff was heuristic-only. The cutoff is sound; its continuous
value below 1 remains only an ordering score.

Measured effect: the hard 8-part positive moved from a matched 300-second timeout to **5.3 CPU
seconds**. The A+B monster is now refuted at the root by the prefix certificate `714>705`, instead of
237.4 CPU seconds / 8.1 billion candidate evaluations. The filter directly proves 822,537 of
2,024,705 recorded k=5 negatives and hits zero of 20,780 positives. All 62,366 negative facts in
`out_k7.txt` still verify independently with zero gaps. Full proof and benchmarks are in the latest
[journal entry](journal.md).

The condition is not sufficient: `Sb(16:1,12:2)` passes it but is unsolvable in 4. The residual is
now formalised by the
[synchronized-majorization predicates](theorems/singleton-majorization.md#the-synchronized-majorization-predicates-corrected-2026-08-26).
`R_0` is full star expansion; `R_d` requires one legal rectangle split whose children pass
`R_{d-1}`. Every predicate is necessary and `R_k` is exact solvability; adjacent intermediate
predicates are not known to be nested. `R_1` also has an
additive hinge-vector formulation, so it can be checked without sorting child profiles.

The predicates are structurally sharp on the complete current-solver k=4 pair universe. Among its 238
canonical negatives, `R_0,R_1,R_2,R_3` reject respectively 68, 150, 229 and all 238, with zero
rejections among 1,247 canonical positives. `Sb(16:1,12:2)` itself passes `R_0,R_1,R_2` and fails
`R_3`. `tools/bundled_majorization.py` and `tools/pairtab.c` reproduce the census.

Deeper synchronization does **not** yet improve the long-state solver. On an exact-L1 miss,
`radiobase.c` already invokes `R_0` on every partial child before its dominance-trie lookup, so a
separate `R_1` pass duplicates the existing prefix walk. On the residual four-part positive
`Sb(29:6,19:9,13:12,36:3)` in 6, the first
cheap `R_1` witness has two exactly unsolvable children and even passes `R_2`; `R_1` feasibility
therefore does not distinguish it from the real winning split. On a residual four-part negative,
isolated `R_2` took 6.3 Python CPU seconds and still passed, while the warmed exact solver refuted it
in 0.1 seconds; `R_3` hit a 30-second cap.

### Recursive Pareto lifting: a sharp first step, open recursion (2026-08-12)

The proposed long-state construction now has an exact local lemma.  For aligned lower template
`T <= P` and lower cut `s`, every lineage-preserving lift is in

```
s <= X <= s + (P - T).
```

Every lower selected/complement/mixed rectangle is then a componentwise substate of its parent
counterpart.  This proves the search box, not the enlarged children: they still require exact
verification.  Full proof and the distinction are in
[recursive-pareto-lift.md](theorems/recursive-pareto-lift.md).

The rigid prefix premise checks out empirically.  Across all 32 one-part k=7 Pareto roots, a winning
first cut can be chosen from the corresponding k=6 frontier; in all 31 cases with a nontrivial
two-part continuation, opposing lower-front lineages give a winning second cut.  The new probe then
lifts the resulting four-part template by preserving its three outcome proportions.  On
`Sb(45:10,33:15,32:14,23:20)@7`, it found a new solution at radius 8, structural rank 5, in 15 wall
seconds.  Ordinary search under the same warm cache took 65 wall / 57 solver seconds and admitted
155,795 top-level splits.  An adjacent lower-front point also found a different solution, but
swapping its two equal `19:9` lineages did not succeed within the same radius: inherited component
identity matters.

Greedy full recursion fails at the next low-k node.  The direct lower split's complete 774,144-point
lift box had no cache-open candidate.  A unique greedy Pareto upgrade produced 19 cache-open
candidates in its smaller box, but strict 200 ms-per-child probes accepted none.  Therefore one lower witness,
one maximal upgrade and its first split are not a construction.  The open object is a choice theorem
over an antichain of Pareto upgrades and inequivalent solving splits, preferably tested first at
larger k where degeneration is weaker.  The implementation remains the standalone
`tools/pareto_lift_probe.c`; no production search order or cache semantics changed.  Fully
provenanced positive-path logs are archived as `pareto-lift-2026-08-12`.

The exhaustive choice-corpus driver is now built and its k=7 run is complete: 32 roots, 450 winning
first cuts, 2,956 labelled second-cut lineages, 563 canonical targets, 819 upgrade nodes, 610
fixed-dimension Pareto endpoints, 7,396 raw endpoint winners and 3,227 exact automorphism classes.
No seed was blocked by the representation limit.  The larger k=8 run was interrupted by an IDE
restart after 4 h 57 m at 7.043 GB peak physical footprint.  Its durable input contains 621 of 815
summary-closed first-cut blocks, including 17 of the 70 blocks absent from the older checkpoints;
the unfinished next block is deliberately ignored and replayed.

That remainder is now detached on the shared AWS host as `pareto_k8_aws`, built from `54486d6` with
build ID `d9a89e3002d69f7879a214fbc78452c257a1c05ac9c51a4ecee55c62432af3cf`.  It has a 20 GiB
individual RSS cap and is the victim of a four-solver 108 GiB combined guard; the host idle guard
also tracks it.  Its supervisor analyzes and uploads the final corpus automatically.  Query it once,
without starting a local watch loop, with `tools/pareto_census_status.sh`.

One safe `R_0` deployment landed on 2026-08-10: split-table construction omits a local cut when one
of that part's child substates already fails counting or full-star majorization at `k-1`. Subgraph
monotonicity proves that later parts cannot rescue it. Tables are now exact-sized contiguous blocks
keyed by `(k,sbb)`, and suffix tables remain absent until depth-first search reaches them. Against
the same warm cache, the positive above retained the identical winning split and top-level
`totalsplits=37899` while moving from 43 to 32 solver seconds; its requested persistent split memory
moved from 1,407,276 to 261,560 bytes. The exact negative control stayed at 0.08-0.09 seconds. This is
allocation and a redundant local necessary check, not an unconditional hierarchy pre-pass. The
remaining theoretical target is a genuinely cheap approximation to deeper synchronization for
**ordering**.

### Theoretical m=6 track: exact k=10 upper break beyond the fitted continuation (corrected 2026-08-26)

The `473:6@9` witness must not be treated as a canonical scalable object. Exhaustive data support
only its early trunk: `Sb(110:3,115:2,121:1)@7` has two working splits, one outcome-complement pair.
Its mixed child `Sb(53:2,52:2,57:1,57:1)@6` has 12 working splits, or three genuine symmetry
classes. The stored tree chooses one; the ambiguity is real from that point onward.

Extending the forced arithmetic prefix gives the parametric four-part kernel

    Z_t = Sb((D_t+2t-1):2, (A_t-2t):2, C_t:1, C_t:1),

where `A_t,C_t,D_t` are the first, third and fourth dyadic atom values of `G_t`. Its six-row
full-star mass is exactly one below the top-six capacity. This forces any `R_1`-feasible next test
into row-count patterns `(2,6,4)`, `(3,6,3)` or `(4,6,2)`, with one unit of total child slack.

The independent predicate checker exhausts every such first test without using a witness
continuation. `Z_6` passes `R_4`, as it must. `Z_7` has 356 `R_1`-feasible raw splits / 84 distinct
normalized child triples and fails `R_4`; `Z_8` has 424 / 101 and also fails `R_4`. Since every
solvable state satisfies every `R_d`, both kernels are unsolvable. Reproduce with
`tools/bundled_majorization.py m6-kernel <t> 4` (21.2 and 57.5 CPU seconds in the recorded runs).

The exact small-m synchronized search exhausts every strategy for `Sb(974:6)@10`, proving
**`n(10,6)<=973`**.  The 115-node 973 file uses the false universal singleton stopping rule
and therefore does not prove 973 achievable; its six nonembedded leaves need separate
strategies.  The old
formula and `BBCD` profile both predict 976 and are therefore refuted.  In the diagnostic tree,
the root `[477:2]` has children `Sb(477:2)`,
`Sb(496:2,477:4)`, and `Sb(496:4)`: it keeps the saturated `m=4` side but avoids the dead
`Z_7` continuation by giving up three units on the other width.  The centered `3+3` alternatives
`Sb(488:3,488:3)@9` (total 976) and `Sb(487:3,486:3)@9` (total 973) are both exactly unsolvable;
this does not classify every working root at 973.

The unconditional upper source is `evidence/sb_m6_k10_frontier.txt`; the unsupported diagnostic file is
`witnesses/majorized_973_6_at10.tree`.  At full depth the default terminal mode of
`tools/search_singletonization.cpp` is a permissive relaxation: a negative is an exact refutation,
whereas a positive is unsupported unless embedded/canonical terminals are requested.
The finite `k=11` target remains open.  The candidate reduces to
`Sb(503:1,495:2,478:3)@9`; its first five-minute exact run timed out, and the literal scaled split
is dead because it produces the exactly unsolvable residual
`Sb(247:1,247:1,240:2,231:2)@8` (277.622 s).  Separately, the relaxed aligned atom model gives
the different structural candidate `2^k-k^2+7k-21` for `k>=12`.  It is not an achievability
construction; it predicts 1983 at `k=11` but its root threshold is one level too high even within
that model to settle that case.  This formula comes from a checked 19-node relaxed symbolic tree, not from promoting the one-point
`-3` correction.

## Immediate next steps

0. **Derive a fast solver from the learned predictor — the active thread.** The substrate, the
   level-held-out value model (now through k=7), and a recursive cut scorer are all built and
   measured, and are now validated end-to-end via real solver calls on a real benchmark; wiring
   a prototype into `radiobase.c` itself is not done. Read
   [ml-guided-search.md](ml-guided-search.md) first: its "Read this first" section lists what
   exists, what each piece measured, and the one command that starts a fully warm oracle. In short:
   a learned ranker orders candidates 428x better than blind and transfers across levels, but is an
   *ordering* not a filter; top-5-guaranteed is ~15x short on the median and ~300x on the tail;
   data is not the constraint anywhere (flat from 26 training states); and the things to beat are a
   9-12x table lookup and an 8x `R_0`. **New 2026-08-20:** scoring a split by `min(V(child))` one
   level down, with a value model that never saw a single split label, reaches 120x selectivity
   against a flat ranker directly supervised on winning splits (130.5x) — on the same real, held-out
   k7 census endpoints. Two traps broke the data pipeline getting there (a fixed mass band doesn't
   transfer past k=6; a long-lived oracle can crash on a hard query) — see the trap table and
   [../evidence/recursive_value_2026-08-20.txt](../evidence/recursive_value_2026-08-20.txt).
   **New 2026-08-21:** that 120x figure hid real degradation on the hardest endpoints (median rank
   doubling, worst case near-blind) — composing the recursive scorer with the sound `R_0` filter
   first fixes it completely (degradation-vs-hardness correlation 0.129 -> 0.001), and `R_0` itself
   gives the first real worst-case cutoff bound in this thread (median 6,892 / worst 16,547
   survivors). See
   [../evidence/recursive_value_worst_case_2026-08-21.txt](../evidence/recursive_value_worst_case_2026-08-21.txt).
   **New 2026-08-21, real solver calls, no offline proxy:** on this repo's own standing hard
   benchmark, `Sb(29:6,19:9,13:12,36:3)@6` (documented at 37,899 top-level splits, 26.6-33 CPU
   seconds under the current default order — see item 4 below), ordering `R_0` survivors by the
   recursive scorer and asking a real warm oracle (genuine `canSolveB`, no shortcuts) whether each
   candidate's three children are solvable finds a working split after **67** top-level tries — a
   **566x reduction**, independently re-verified. The `R_0`-survivor set in its natural order had
   not found one after 1,340 tries, so the ordering itself, not just the sound filter, is doing the
   work. Not yet wired into `radiobase.c`, and n=1. See
   [../evidence/real_benchmark_residual_control_2026-08-21.txt](../evidence/real_benchmark_residual_control_2026-08-21.txt).
   **Same day, does the algorithm differ by part count?** Tested at 3, 4 and 8 parts (real
   documented states, real oracle calls): candidates-to-success is **43 / 67 / 52** — strikingly
   stable across a 3-8 part range and 9 orders of magnitude of raw search space, same value
   model, unretrained. What actually breaks by part count is *generating* the candidates, not
   scoring them: exact enumeration (clean at 3-4 parts) OOM'd outright at 8 parts and needed a
   width-bounded beam DP, which itself needed two widths tried before one found anything. 1- and
   2-part states weren't tested — already solved exactly with no search in this codebase. See
   [../evidence/real_benchmark_by_part_count_2026-08-21.txt](../evidence/real_benchmark_by_part_count_2026-08-21.txt).
   **Narrowed back to 4 parts, 2-or-4-winner tier (the direction after this):** a second, harder
   4-part instance, `Sb(16:12,17:10,29:5,21:6)@6` (documented: exactly 2 winners of 1.2 billion),
   needed **1,373** real oracle-checked candidates against the residual control's 67 — still
   overwhelmingly better than blind (~322x over the `R_0`-survivor population alone), but not a
   constant number: it tracks how rare the winner actually is. Also surfaced a measurement
   problem: the `R_0`-filtering pass on this run took an anomalous 60 minutes (15x this thread's
   usual rate), almost certainly this machine sleeping/throttling mid-run rather than the
   algorithm — candidate counts are unaffected, but local wall-clock numbers from this session
   should be read as upper bounds only. See
   [../evidence/real_benchmark_4part_2or4_2026-08-21.txt](../evidence/real_benchmark_4part_2or4_2026-08-21.txt).
   Guidance is correctness-free for **achievability** only, since a witness is checked by
   `check_witness.py` — never prune an OR-branch with a learned value; only a proven filter like
   `R_0` may ever certify a negative.

1. **The historical warm oracle is retired; do not restore its mixed cache.** `radio_oracle.c`
   still amortizes initialization when many verdicts are needed, but start a current build cold or
   load only provenance-separated facts whose status survived the singleton-majorization
   refutation. In particular, the `Sa(193)` negative certificate remains proof-safe, while the
   opaque full-corpus and mutable service snapshots do not. Persistent instance
   `i-002cabc654b2078ed` was terminated on 2026-08-31; see [aws-run.md](aws-run.md).
   Historically, `radio_oracle.c`
   pays `init()` and cache replay once, then answers `<k> <n1> <m1> ...` from stdin. Measured at
   **MAX_K=9, MAX_N=300** (init 37 s, 0.64 GB): 2,200 k=5 states in 243 ms of query time, **0.11 ms
   each**, verdicts identical to per-process `radio_one` on all 2,200. Start with
   `./run_radio_oracle.sh`, drive with `tools/oracle_client.py`, details in
   [tools.md](tools.md) and [../evidence/warm_oracle_2026-08-20.txt](../evidence/warm_oracle_2026-08-20.txt).
   **Start it cold and journal what you compute.** The old 21,866,180-fact snapshot restored in
   32.8 s at 2.41 GB resident, but that is now only a historical performance datum, not permission
   to load it.

2. **Take the value model to k=7 — done; now cost it against the solver, not AUC.** With matched
   oracle-labelled sampling (mass band bisected per level, not fixed — see the trap table), a
   scale-normalized value model trained on `k<=6` predicts k=7 solvability at AUC 0.986 (logistic)
   / 0.996 (boosted), against a 0.482 permuted control; the sound per-part deficit alone is also at
   0.996 by k=7, so the learned edge is now concentrated on the 131 of 273 states neither sound
   filter decides (boosted 0.971 there vs mass 0.843). Details in
   [../evidence/recursive_value_2026-08-20.txt](../evidence/recursive_value_2026-08-20.txt) (k=4/k=5
   history in [../evidence/value_level_transfer_2026-08-20.txt](../evidence/value_level_transfer_2026-08-20.txt)).
   Next: judge it on end-to-end CPU against a table lookup, not on AUC — still not done.

3. **Make the predictor recursive — done for the value side; the policy/DP side is next.** The
   flat-feature ranker stalled at median rank 76 of 54,014, and the learning curve said that was a
   feature-set limit, not a data limit — what decides the winner lives one level down. Confirmed
   2026-08-20: scoring a candidate split by `min(V(child))`, `V` trained with **zero split-label
   supervision**, reaches 120x selectivity on real forced k7 endpoints — within 8% of a flat ranker
   directly supervised on winning splits (130.5x), on the identical population. A model with
   *higher* standalone AUC (gradient boosting) collapsed to 2.3x under the same composition —
   standalone AUC did not predict this; only the end-to-end composed metric did. Design note and
   full numbers: [ml-guided-search.md](ml-guided-search.md),
   [../evidence/recursive_value_2026-08-20.txt](../evidence/recursive_value_2026-08-20.txt).
   **2026-08-21, worst case first, then order:** stratified by exact candidate-set size, the
   recursive ranker alone degrades sharply on the hardest third (median rank 146->331, worst
   near-blind at 16,886 of up to 130,262) — a real gap, not a data-generation artifact. Composing
   with `R_0` (the proven sound filter, `tools/bundled_majorization.r0`) FIRST fixes it: `R_0`
   gives a real worst-case cutoff (median 6,892 / worst 16,547 survivors, vs stage-2's up to
   130,262), shrinks the hardest tier *more* than the easiest (10.6x vs 4.7x), and once applied,
   the recursive ranker's hardness-correlated degradation vanishes (correlation 0.129 -> 0.001).
   The two guarantees stay separate: `R_0`'s survivor count is a sound proof-of-unsolvability
   bound if exhausted; the rank within it is ordering only, never a stopping rule. See
   [../evidence/recursive_value_worst_case_2026-08-21.txt](../evidence/recursive_value_worst_case_2026-08-21.txt).
   **2026-08-21, end-to-end on a real benchmark:** the offline selectivity claim above is now
   confirmed with real solver calls, not a proxy — 67 vs 37,899 top-level splits on this repo's own
   `Sb(29:6,19:9,13:12,36:3)@6` control (item 4 below); see
   [../evidence/real_benchmark_residual_control_2026-08-21.txt](../evidence/real_benchmark_residual_control_2026-08-21.txt).
   **2026-08-22, re-validated against the persistent oracle:** on the corpus's own highest-mass
   (hardest) forced endpoint (90.9% of cap, 2 known winners), learned order succeeds at candidate
   #1 of 2,626 `R_0` survivors; natural order on the identical subsample needs #2,970 — same exact
   winning split either way, matching a known census winner. Two real bugs caught first: `enumerate`
   has no incremental pruning and is not usable for ordering past k<=6 even on an ordinary 4-part
   state (10+ minutes, no result, killed); and census "kN" corpora are rooted at k=N but their
   *endpoints*' real parent level is `C["rk"]` (5, not 7, for the "k7" corpus) — querying the wrong
   k gives internally-consistent, wrong answers with no code-level guard against it. See
   [../evidence/real_benchmark_via_aws_oracle_2026-08-22.txt](../evidence/real_benchmark_via_aws_oracle_2026-08-22.txt).
   **2026-08-22, the systematic 16-endpoint sample (8 per tier), done:** `rank_learned` is small
   and fully measured for every endpoint (2-winner tier: median 7, max 104, of 31k-75k true
   candidates; 4-winner tier: median 83, max 687). `rank_natural`'s 8,000-try cap resolved only
   4/16 endpoints (selectivity 68.9x-6,041x where measured); the other 12 give only a lower bound
   (as low as >=11.6x, as high as >=1,600x) — read this as two separate complete facts (learned
   rank always small; natural rank, wherever measurable, in the thousands) rather than one blended
   median, since averaging just the resolved cases would silently drop the harder-to-measure
   majority. One real bug caught first: subsampling the stage-2 pool to 20,000 before checking
   winner-membership can silently exclude every known winner for a rare-winner endpoint (not a
   corner case for this population — fixed by ranking over the full true pool, ~50-85k, instead).
   One sampling-bias caveat: the 4-winner tier has only 22 endpoints total, so its "hardest-octile"
   sampling is actually an unbiased draw over the whole tier, unlike the 131-endpoint 2-winner
   tier where it is a real bias — the two tiers' numbers are not directly comparable as
   hardest-vs-hardest. See
   [../evidence/tier_sample_via_aws_2026-08-22.txt](../evidence/tier_sample_via_aws_2026-08-22.txt).
   **2026-08-22, two more prototypes tested, one negative and one positive.** Block coordinate
   descent (fix 2 of 4 segments, jointly re-optimize the other 2 against the real pooled score —
   a genuine "packing problem" framing, since the mass/cap constraint is a solved multiple-choice
   knapsack and the open problem is the non-separable joint quality signal) succeeded on the two
   endpoints where the pooled model itself ranked easily, but failed decisively — not just slowly —
   after 150 restarts (up to 774,712 evaluations) on the two endpoints where the pooled model
   itself struggled. See
   [../evidence/deficit_order_and_bestfirst_2026-08-22.txt](../evidence/deficit_order_and_bestfirst_2026-08-22.txt)
   section 6.

   **What followed from here (concentric round expansion through radius mode through BY_ML) is
   closed, per direct instruction 2026-08-24 -- see below.**

   **2026-08-24: this whole native-concentric-search / radius-mode / BY_ML thread is now closed.**
   `main`'s `radiobase.c`/`radio_oracle.c` are reverted to their pre-thread state (`e206766`/
   `c7bc503`) -- the plain, understood `canSolveA`/`canSolveB` engine, verified post-revert to
   still solve cold `Sa(192)` in k=10 in 292.4 CPU seconds. Every experiment described above
   (concentric round search, radius mode in all its propagation variants, the BY_ML learned
   ordering) is real, was measured honestly, and is preserved in full -- code, data, and commit
   history -- on branch `concentric-search-radius-ml-exploration` (`b71a5e3`), not deleted. The
   short version: plain `canSolveB(NO_DEADLINE)` beat every alternative tried on the actual hard
   target (`Sb(112:80)`, a single large unsplit pair -- the `Sa(n)` leaf-verification shape), and
   while squaring+sqrt radius mode did win on a different shape (an already-split multi-part
   battery state), and BY_ML's wiring is mechanically sound, neither was a net win worth carrying
   as unused weight on the trusted core. Full findings, the mechanistic diagnoses (why ordering
   quality degrades on the hardest states; why radius currency can't compose safely across size-
   changing recursion; why BY_ML's deficit feature failed on an out-of-coverage regime it never
   trained on), and a concrete list of future exploration avenues (retraining BY_ML with newly-
   acquired k=8/k=9 in-regime data chief among them) are in the 2026-08-24 journal entry "closing
   the fast-solver thread." See
   [../evidence/native_concentric_2026-08-23.txt](../evidence/native_concentric_2026-08-23.txt)
   (the complete 26-section record) and
   [journal.md](journal.md#2026-08-24-closing-the-fast-solver-thread-concentric-search--radius-mode--by_ml-moved-to-a-branch-mains-solver-reverted-to-the-plain-understood-engine).

4. **Follow up the mixed-largest law.** The k=8 corpus is analysed (below); the one result worth
   acting on is that in all 26,876 winning classes across both censuses the *mixed* child is
   strictly the largest, against 59%/57% among random cap-feasible splits. If it holds beyond
   residual k=5/6 it is a free necessary condition worth putting in front of the solver's split
   loop. Test it on another corpus before using it. No AWS compute remains under
   `Project=radio-sa193`; the retired oracle's persistent volume remains available, while the old
   replay hosts are terminated. Do not restart either retired independent-checker
   coloring pipeline or the superseded independent ordinary audit.

5. **Finish P5 with the new exact Sa boundary.** The paper may now state `Sa(10)=192` as a proven
   maximum, citing the verified witness and proof-safe cold log. Its remaining TODO sections are
   editorial/theorem integration work, not an H3 compute dependency.

6. **Continue the parallel-solver prerequisite, not the thread pool yet.** Objectify the result
   cache as a frozen read view plus worker-local overlay, then separate immutable split geometry
   from learned cut metadata. Extract and regression-test a resumable serial pass-2 prefix cursor
   before scheduling limited-width batches. The ownership contract is in
   [parallel-solver.md](parallel-solver.md).

The pair/triple/quad deployment and limited-discrepancy FAST passes remain **rejected**. Their offline
facts are real, but the warm upward-closed prefix cache already contains the subset information; the
former added zero marginal rejections on the A+B monster, and the latter regressed negatives. Full
star expansion is different: it is an arbitrary-part-count global theorem and is now deployed.

4. For further P6 work, use `Sb(29:6,19:9,13:12,36:3)` in 6 as the residual positive control. A new
   bundled proposal must order the real winning split earlier under the same warm k<=5 cache; merely
   finding an `R_1` or `R_2` witness is already known not to do that. Current `main` takes 26.6
   solve seconds and 37,899 top-level splits after the exact-L1 change. Keep any deeper check
   bounded and fallback-safe. **2026-08-21: the recursive-V-ordered `R_0` survivors pass this test**
   — not yet inside `radiobase.c`, but real `canSolveB` calls via the oracle confirm a working split
   (different from the documented one, independently re-verified, exact mass arithmetic) after 67
   top-level candidates, against the documented 37,899 — while the SAME `R_0`-survivor set in its
   natural, unscored order had not found one after 1,340 tries. See
   [../evidence/real_benchmark_residual_control_2026-08-21.txt](../evidence/real_benchmark_residual_control_2026-08-21.txt).
   Still not C-integrated, and n=1. For the separate recursive Pareto-lift track, move upward in k before
   adding solver code: retain several parent-conditioned Pareto upgrades and inequivalent splits,
   preserve lineage labels through equal components, and measure whether one branch survives at the
   next recursive node.  One greedy low-k path is already known to fail.
5. `./run_radio_canon_search_generic.sh 4 9 457 7` and `... 447 8` — unique forced predictions
   of the profile model; minutes each, and a hit is a self-verifying proof.  These are direct H2
   construction attempts, not a resumption of the parked assembly programme.
6. `... 432 9` — discriminates the remaining `m=9` profile row (432) from its closed form (431).
7. The **Extremal Split Lemma** — the whole remaining gap in conjecture (u1), and the only item
   here needing no compute at all. An exchange argument is the natural shape; the surviving
   obligations are listed in
   [conjectures.md](conjectures.md#where-the-proof-gets-stuck).
