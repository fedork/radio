# Tight-Band Capacity Obstruction and the complete `K=6` rank-15/32 face

Status: **analytic obstruction proved; certificate extractor and fixed-face classification
complete, provenance-built, and sanitizer-checked** (2026-08-31).

The theorem and proof are in
[`docs/theorems/tight-band-capacity.md`](../docs/theorems/tight-band-capacity.md).  This record
separates the inequality-only certificate computation from the independent direct-row
classification of the surrounding finite face.

## Independent implementations

[`tools/singleton_tight_band_certificate.cpp`](../tools/singleton_tight_band_certificate.cpp)
recomputes `G_K`, its prefix functions, every tight rank and every monotone endpoint transition.
It applies only the proved mixed-floor and pure-capacity inequalities.  It contains no row-split
search, Hall test, result cache or code from `singleton_pascal_interval_census.cpp`.

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
faces or support sizes.

The direct face census used 141,216 DFS nodes in total.  Its largest search was the unique hole at
9,345 nodes.  The 175 positive results are first-cut feasibility results only; their majorized
children are not thereby proved recursively solvable.

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
The previously completed `K=4` first-cut census has no holes, so certificate coverage there is
vacuous; it was not rerun inside this small extractor.

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

## Scope and next boundary

The Tight-Band Capacity Obstruction is a sound one-sided certificate.  The complete fixed-face
survey shows that it catches the only hole on this face, but that is not evidence that every hole
on every face has a two-anchor certificate.  The remaining next questions are `K=5`, broader
`K=6` support/face minimality, and whether newly found holes require laminar multi-anchor or general
Hall-dual certificates.
