# Tight-Band Capacity Obstruction: fixed face, `K=5` boundary, and certificate distance

Status: **analytic obstruction proved; fixed-face classification, complete `K=5` certificate
census, and global minimum-distance optimization within this obstruction class are
provenance-built and sanitizer-checked** (2026-08-31).

The theorem and proof are in
[`docs/theorems/tight-band-capacity.md`](../docs/theorems/tight-band-capacity.md).  This record
separates the inequality-only certificate computation from the independent direct-row
classification of the surrounding finite face.

## Independent implementations

[`tools/singleton_tight_band_certificate.cpp`](../tools/singleton_tight_band_certificate.cpp)
recomputes `G_K`, its prefix functions, every tight rank and every monotone endpoint transition.
It applies only the proved mixed-floor and pure-capacity inequalities.  It contains no row-split
search, Hall test, result cache or code from `singleton_pascal_interval_census.cpp`.  Its extended
modes count dominated exact-length bands, enumerate every band on which the mixed floor is
positive, and minimize transfer distance over the finite disjunction of capacity inequalities.

[`tools/singleton_direct_split_cleanroom.cpp`](../tools/singleton_direct_split_cleanroom.cpp)
independently enumerates the legal integer row triples and verifies the three sorted child
majorizations.  Its trust boundary and tiny naive-oracle comparisons are recorded in
[`singleton_direct_split_cleanroom_2026-08-31.md`](singleton_direct_split_cleanroom_2026-08-31.md).
The face survey replays and rechecks every positive allocation before counting it.

Run both implementations with

```sh
tools/singleton_tight_band_regression.sh
```

## Primary certificate

For the full state and the mass-697 core, the extractor finds three valid anchor pairs:

    (15,30), (15,31), (15,32).

They leave six, four and two monotone endpoint transitions respectively.  The deterministic
primary certificate minimizes that transition count and is therefore `(15,32)`:

    CERTIFICATE anchors=(15,32) mixed_floor=1
      lower_counts=(7,8) upper_counts=(16) band=(8^15,7^2) transitions=2
      TRANSITION from=7 to=16 mixed_required=22
        left_rows=9 left_required=64 left_capacity=63
        right_rows=8 right_required=48 right_capacity=56 blocked=LEFT
      TRANSITION from=8 to=16 mixed_required=22
        left_rows=8 left_required=48 left_capacity=56
        right_rows=9 right_required=64 right_capacity=63 blocked=RIGHT

The program asserts every displayed integer, including the complete three-pair anchor list.  The
analytic proof does not depend on trusting this output: it is the direct evaluation of the theorem
using the six `H` values already displayed in the counterexample record.

## Complete fixed-face survey

Fix the first 15 rows `(64,63,57^2,42^4,22^7)` and let the next 17 rows range over every partition
`b` satisfying

    length(b)=17,
    mass(b)=134,
    b <=_w (22,7^16).

Appending `1^32` gives the corresponding full-mass exact-support parent; omitting it gives the
unit-free prefix.  Prefix-tightness makes these precisely the dominated integer points on the
fixed `[15,32)` band face.  Both implementations enumerate the face independently.

The capacity extractor reports

    K6_TIGHT_BAND_CERTIFICATE_SURVEY complete=YES band=(15,32)
      states=176 certified_states=1 certificate_pairs=3
      unique_certified=(8^15,7^2) minimum_transfer_distance=14

The direct-row solver reports

    K6_TIGHT_BAND_FACE_SURVEY complete=YES band=(15,32)
      states=176 feasible=175 holes=1 unique_hole=(8^15,7^2)
      minimum_transfer_distance=14 total_nodes=141216
      max_nodes=9345 worst_band=(8^15,7^2)

Thus the independently generated state lists and unique negative agree.  This proves the following
finite first-cut classification:

> The counterexample band `(8^15,7^2)` is the unique first-cut hole among all 176 exact 17-row
> refinements of `(22,7^16)` on the fixed rank-15/32 face.

Here distance means half the `l1` distance between the sorted equal-mass band vectors.  The unique
hole is therefore at distance 14, so the previously observed 14-step boundary is globally minimal
**within this face**, not merely along its one transfer path.  No claim is made about other `K=6`
faces or support sizes by this face calculation alone; the later transfer-shell census supplies
the global exact-support distance statement.

The direct face census used 141,216 DFS nodes in total.  Its largest search was the unique hole at
9,345 nodes.  The 175 positive results are first-cut feasibility results only; their majorized
children are not thereby proved recursively solvable.

## Exact `K=5` boundary size

Minimum-Support Reduction leaves the full-mass `K=5` problem on exactly 32 positive rows.  A
memoized exact-partition count, independently checked against the known `K=3,4` counts, gives

    exact-support parents        1,431,800,647,444
    strict internal-prefix states  147,422,086,892
    states with a tight skeleton 1,284,378,560,552.

Here “strict” means strictly below the `G_5` prefix at every rank 1 through 31.  Thus even the
proved support reduction leaves 1.43 trillion parents, and literal tight-skeleton factorization
still leaves 147 billion strict states.  A flat `K=5` first-cut census is not a plausible next
step.

Across all 528 canonical rank bands, the corresponding exact dominated-band count is
8,973,226,867,713 state--band instances.  The Tight-Band Capacity Obstruction can fire only when
`delta_v=H(v)-H(v-1)>0`.  Since `G_4` has support 16, this restricts `K=5` to the 136 anchor pairs

    0 <= u < v <= 16.

Those faces contain 613,689,090 dominated exact-length bands in total; the largest is `[0,16)`
with 228,246,747 bands.

## Complete `K=5` capacity-certificate census

The inequality-only enumerator checked all 613,689,090 relevant `K=5` band instances:

    TIGHT_BAND_CAPACITY_SURVEY K=5 complete=YES verified=YES
      bands=136 states=613689090 certified_bands=0 certified_states=0

This is a complete negative result for this **certificate class**, not a positive first-cut
theorem.  It says that no pair of tight `K=5` anchors can obstruct every endpoint transition by
the theorem's mixed-floor/pure-capacity inequalities.  A `K=5` hole could still exist without such
a certificate, including in the 147-billion-state strict interior where there are no internal
tight anchors at all.  The later
[prefix-cylinder census](singleton_k5_prefix_cylinder_2026-08-31.md) closes the full `K=5` space
positively; that theorem does not follow from this absent-certificate result.

The exhaustive enumeration also cross-checks the separate optimization below.  At `K=3` and
`K=4`, the analogous positive-floor corpora have 22 and 3,863 band instances and likewise contain
no certificate, as soundness requires from the known positive first-cut censuses.

## Global minimum distance within the obstruction class

For a fixed anchor pair, every endpoint transition supplies the integer disjunction

    B(s_L) <= L + s_L delta - 1
      or
    B(s_R) <= R + s_R delta - 1,

where `B(s)` is the sum of the `s` largest band rows.  The optimizer processes these clauses,
retains the componentwise maximal prefix-cap vectors, and uses an exact dynamic program over
`(rank, remaining mass, previous row)` to minimize

    sum_i |b_i-c_i|

under each vector.  A row below the mixed floor is optimized as a separate disjunct.  Discarding a
stronger cap vector is sound because every partition satisfying it also satisfies a weaker vector;
every capacity certificate selects at least one blocked side of every transition and is therefore
represented.  The reconstructed minimizer is replayed through the ordinary analyzer.

The complete results are

| level | eligible anchor pairs | maximal blocking-cap vectors | faces admitting a certificate | minimum distance |
|---:|---:|---:|---:|---:|
| 3 | 10 | 17 | 0 | none |
| 4 | 36 | 74 | 0 | none |
| 5 | 136 | 528 | 0 | none |
| 6 | 528 | 38,131 | 3 | 14 |

At `K=6`, the only three anchor faces admitting any certificate are the already known
`(15,30)`, `(15,31)` and `(15,32)` faces.  The global minimizer is displayed at the shortest
endpoint as

    anchors=(15,30), canonical band=(22,7^14), minimizing band=(8^15).

Putting the canonical head and tail back recovers exactly the known parent with intervening rows
`(8^15,7^2)`.  Its transfer distance is 14.  Therefore:

> No exact-support `K=6` parent at transfer distance at most 13 from `G_6` has a Tight-Band
> Capacity certificate, and distance 14 is globally minimal among all parents certified by this
> two-anchor theorem.

This certificate calculation alone does **not** give global hole minimality: an uncertified
first-cut hole could have lain closer.  A subsequent direct Fixed-Color Hall census nevertheless
made the 5,189,450,419-parent distance-13 ball practical and found a majorized first cut for every
state.  Thus distance 14 is now globally minimal for an exact-support no-first-cut hole.  The
distance-14 shell contains 9,960,648,265 parents and was not otherwise classified, so global
uniqueness at the minimum is not claimed.  See the
[transfer-shell record](singleton_transfer_shell_census_2026-08-31.md).

The standard regression wrapper reproduces the boundary counts, optimizer and complete `K=5`
certificate enumeration.  To reproduce the transfer-shell sizing separately:

```sh
tmp_bin=/tmp/singleton-tight-band-certificate
CC=clang++ tools/build_radio.py -std=c++20 -O3 -Wall -Wextra -pedantic \
  tools/singleton_tight_band_certificate.cpp -o "$tmp_bin"
tools/run_with_provenance.py "$tmp_bin" count-transfer-shells 4 14
tools/run_with_provenance.py "$tmp_bin" count-transfer-shells 6 14
```

## Controls

The inequality-only extractor reports no certificate on transfer steps `j=0,...,13` and all three
certificates at `j=14`.  Absence of a certificate is not a positive verdict; the existing direct
search supplies those path verdicts.

As a false-obstruction control, it scans every full-mass majorized partition at `K<=3`:

| `K` | states | tight-band certificates |
|---:|---:|---:|
| 1 | 2 | 0 |
| 2 | 15 | 0 |
| 3 | 1,206 | 0 |

The total is 1,223 states.  The separate direct solver already finds a first cut for every one.
The band enumerator additionally scans all 22, 3,863 and 613,689,090 positive-floor band instances
at `K=3,4,5`, respectively, and finds no certificate.  The first two results agree with the known
complete positive first-cut censuses.  At `K=5`, absence remains `UNKNOWN`, not `FEASIBLE`.

## Clean provenance and cost

The recorded ordinary run used commit
`9bf0f4a8aa3a7b05c0d2ce76d700fdd6ee6b49ea`; both sources and the worktree were clean.  The whole
wrapper command, including both builds and both regressions, took 4.60 wall seconds.

| implementation | source SHA-256 | build ID |
|---|---|---|
| capacity extractor | `72c7c04075d2fb324e89f2f23ad1b52310b3dcd98355538ff283f917bd847583` | `96976b088b511cab88b909de24e93c5068f6ac33c9beb81554be88ef7d4bb7a5` |
| direct-row solver | `885a0c4223ffe89515e407182dfbaf6c04fe4d4a9726d9f23c50648c2f77a00f` | `f63b7f1f2ff31886939274b4b818f0bb75f51e1973ab65dec22c8ea510a73d0d` |

Separate AddressSanitizer plus UndefinedBehaviorSanitizer builds repeated the complete regressions
with no finding:

| implementation | build ID | build plus run wall time |
|---|---|---:|
| capacity extractor | `6fb4eef4b660be79ea75735627f13772d8f774eba1ab1042b5d5993b8ce79f44` | 2.18 s |
| direct-row solver | `f34c58eb39e1a8575dee0604f249808544aa7a15e9c6a116b805f5b43ee7c1be` | 13.53 s |

Both sanitizer runs used the same clean commit and source hashes as the ordinary run.  They used
`ASAN_OPTIONS=detect_leaks=0` because LeakSanitizer is unsupported by this macOS toolchain; the
recorded check covers AddressSanitizer and UndefinedBehaviorSanitizer.

An initial exploratory provenance build accidentally omitted `CC=clang++`; the C driver reached
the linker without the C++ runtime and failed after 1.85 seconds.  It executed no survey and
produced no research verdict.  The checked regression wrapper sets `CC=clang++` explicitly.

The extended count/enumeration/optimization modes were frozen at clean commit
`9cc58b93e4403a7412e609bf30d5806e5b67a65a`.  The optimized capacity build has source SHA-256
`af07e6ce1313a441bb766ac6196a92839ef60daa9901ba97beced9ebcaf83c21` and build ID
`5bd50b81bb5bbe20eff11b23b595f68e8c1b13a87e03122e37ebacbc5f2c0c55`.  The standard wrapper now
runs the complete 613,689,090-instance `K=5` capacity census as well as the optimizer and the
independent direct-row controls; the clean capped invocation completed in 30 wall seconds.  Both
source and worktree dirty flags were `no`.

A clean AddressSanitizer plus UndefinedBehaviorSanitizer build repeated the count and optimization
regression with no finding.  It used the same source hash and commit, build ID
`98258f92cc9c1bfbc98bd0f166dff455e562de7dd8b637fa79ee4b0a82adcdf2`, and
`ASAN_OPTIONS=detect_leaks=0` for the macOS LeakSanitizer limitation.  The build plus run took
28.8 wall seconds.  The 613-million-instance enumeration itself was exercised by the optimized
clean wrapper, not repeated under sanitizers.

## Scope and next boundary

The Tight-Band Capacity Obstruction is a sound one-sided certificate.  The complete fixed-face
survey shows that it catches the only hole on that face, while the complete `K=5` certificate
census shows that this obstruction class is empty there.  Neither result proves that every hole
has a two-anchor certificate.  The later transfer-shell census rules out all closer uncertified
exact-support first-cut holes at `K=6`.  A subsequent
[dyadic balanced-band construction](singleton_dyadic_counterexample_family_2026-08-31.md) proves
that this same two-anchor obstruction supplies a no-first-cut parent at every `K>=6`; higher-level
existence is therefore settled.  The remaining questions include the actual `K=5` problem,
minimality and classification at higher levels, uniqueness within the `K=6` distance-14 shell,
recursive-unsolvability minimality, and whether uncertified holes require laminar multi-anchor or
general Hall-dual certificates.
