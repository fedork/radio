# Independent direct-row verification of the `K=6` singleton hole

Status: **complete, provenance-built, and sanitizer-checked** (2026-08-31).

This record adds an implementation-independent check of the counterexample in
[`singleton_k6_counterexample_2026-08-30.md`](singleton_k6_counterexample_2026-08-30.md).
The analytic rank-15/32 argument remains the proof.  The new program is a small exhaustive
regression whose main value is that it shares no Hall/coloring code, result cache, or search state
with `singleton_pascal_interval_census.cpp`.

## Clean-room boundary

[`tools/singleton_direct_split_cleanroom.cpp`](../tools/singleton_direct_split_cleanroom.cpp)
uses only the C++ standard library and recomputes `G_k` from its defining recurrence.  For each
parent row `a_i` it enumerates exactly

    (l_i,m_i,0),  l_i+m_i=a_i,

or

    (0,m_i,r_i),  m_i+r_i=a_i.

The three child multisets are stored in sorted order and tested directly against the prefix sums
of `G_(K-1)`.  It does not include `radiobase.c`, call the Hall criterion, read a cache, or import
any existing singleton search code.

The optimized DFS has four pruning families, all necessary:

1. every already-created child submultiset must itself be majorized by `G_(K-1)`;
2. remaining mass and remaining positive-row slots must be sufficient for the minimum final child
   mass forced by the other two child caps;
3. a remaining row can add at most one pure piece, so the two pure children share the remaining
   pure mass/support capacity;
4. at every future parent prefix `t`, some left-orientation count `p` must still satisfy
   `A(t)<=H(p)+H(t)+H(t-p)`, and the already assigned pure masses must fit `H(p)` and `H(t-p)`.

No preferred child profile is used as a prune.  For the two positive controls it only orders
branches whose current pieces remain submultisets of the requested children; every other branch
remains in the search.

Equal parent rows are quotiented by requiring their static row-triple option indices to be
nondecreasing.  This is sound because permuting equal rows changes no child multiset.  There is no
additional left/right quotient.  That distinction matters: the first draft combined the equal-row
order with an independently chosen global side normalization and falsely rejected all-unit
controls.  The unquotiented oracle caught the error in a 1.1-second run; removing the second
normalization fixed it.

## `K=6` controls

The original clean optimized build at commit `bc11b23c6407c27940ab906b0f545ed9041b8531` had source SHA-256
`277b6ce9469980fe38ebbcb7956643666b6f8f2b7e6cb8d273ac53d7cb0fa691` and build ID
`e5bd63dc7cc1fb7b9d036f73106b0bd482fdc8d594f24c03a82bb59c952d0427`.  Both
`git_source_dirty` and `git_worktree_dirty` were `no`.  Its deterministic results were:

| case | verdict | DFS nodes | row options | maximum depth | child-prefix prunes | future-prefix prunes |
|---|---:|---:|---:|---:|---:|---:|
| canonical `G_6` | yes | 375 | 540 | 64 | 166 | 0 |
| padded transfer state `j=13` | yes | 345 | 488 | 64 | 144 | 0 |
| padded transfer state `j=14` | no | 9,345 | 34,958 | 29 | 25,586 | 28 |
| unit-free mass-697 core | no | 9,345 | 34,958 | 29 | 25,586 | 28 |

The canonical control returns three copies of `G_5`.  The `j=13` control reproduces the requested
children exactly:

    L=(32,31,26^2,16^4,6^8,1^16),
    M=(32,31,26^2,16^4,6^7,2^5,1^12),
    R=(32,31,26^2,16^3,8,7^8,1^16).

Its replayable grouped row allocation is

    64 -> (0,32,32)
    63 -> (32,31,0)
    57 -> (0,26,31), (31,26,0)
    42 -> (0,16,26)^2, (26,16,0)^2
    22 -> (0,6,16)^3, (16,6,0)^4
     9 -> (0,1,8)
     8 -> (0,1,7)^8, (6,2,0)^5
     7 -> (6,1,0)^3
     1 -> (0,0,1)^16, (1,0,0)^16.

The program replays every positive allocation from its row triples and rechecks all three sorted
children before reporting success.

The core run is genuinely slack-aware.  Its parent mass is 697, so the generic residual bound is
only `697-2*243=211` per child; it never assumes three child masses of 243.  Its equality with the
padded run's branch counts arises because the rank-32 obstruction closes the search before any
trailing unit row is reached.

## Tiny-level controls

The source contains a second, deliberately naive oracle.  It performs no prefix pruning, no
equal-row quotient, no orientation bound, and no residual bound; it enumerates row triples and
sorts/checks children only at complete leaves.

The optimized and naive verdicts agree on 201 partitions: every partition of every mass through
`3^K` for `K=1,2`, and every partition of mass at most nine for `K=3`.  The naive oracle made
17,066 complete-leaf checks.

Separately, the optimized solver enumerates every full-mass partition majorized by `G_K` through
`K=3` and finds a first cut for all of them:

| `K` | all full-mass majorized types | exact-support types | optimized DFS nodes |
|---:|---:|---:|---:|
| 1 | 2 | 1 | 7 |
| 2 | 15 | 4 | 198 |
| 3 | 1,206 | 160 | 58,212 |

These counts independently reproduce the existing 160-state exact-support `K=3` control while
also checking the full 1,206-type universe.

## Fixed rank-15/32 face extension

At commit `944b16f28cf13e1457c9f176c25b62034152ee16`, the same direct-row implementation gained a
bounded mode which enumerates every 17-part band of mass 134 weakly majorized by `(22,7^16)` while
holding the other parent rows fixed.  It finds 176 bands, replays feasible allocations for 175,
and exhausts exactly one hole, `(8^15,7^2)`.  The total is 141,216 DFS nodes and the unique hole is
the largest search at 9,345 nodes.  A separate inequality-only program independently certifies
exactly that state.  Full provenance and the restricted transfer-distance consequence are in the
[tight-band capacity record](singleton_tight_band_capacity_2026-08-31.md).

## Reproduction and sanitizers

The ordinary regression is one command and took 2.6 wall seconds including its provenance build
in the recorded clean run:

```sh
tools/singleton_direct_split_regression.sh
```

The final line must be

    CLEANROOM_SINGLETON_DIRECT_SPLIT verified=YES implementation=direct-row-triples hall_code=NONE shared_cache=NONE

An independent ASan+UBSan build used

```sh
CC=clang++ tools/build_radio.py -std=c++20 -O1 -g -Wall -Wextra -pedantic \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  tools/singleton_direct_split_cleanroom.cpp \
  -o /tmp/singleton-direct-cleanroom-sanitize
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 \
  tools/run_with_provenance.py /tmp/singleton-direct-cleanroom-sanitize regression
```

It completed the identical regression with exit zero in 4.1 wall seconds including the clean
build.  Its build ID was
`ebe27e3473e450ad45619f4e630388c90c27b422b005df12f36f66bfd6992a29` and the source hash was the
same as above.  LeakSanitizer is unsupported on this macOS toolchain; an initial
`detect_leaks=1` invocation stopped before executing the tests, so the recorded sanitizer pass
uses `detect_leaks=0` and covers AddressSanitizer plus UndefinedBehaviorSanitizer.

## `K=7` family control and mechanical speedup

After the dyadic family was found, the solver independently exhausted its first new member

    (128,127,120^2,99^4,64^7,32,31^16,8^32,1^64).

The initial cleanroom code returned

```text
RESULT K=7 ... feasible=NO nodes=21489353 options=238217814 max_depth=31
  prunes(child=216728462,mass=0,support=0,prefix=0)
```

in about five minutes under a 300-CPU-second cap.  Profiling by inspection exposed a purely
mechanical cost: the option-sort comparator reconstructed and rechecked both candidate child
states every time it compared two options.  The solver now constructs each candidate once, stores
its exact score and next state, and sorts those records.  This changes no option order, prune,
cache, node or mathematical condition.  All pre-existing regression node counts remain unchanged.

The provenance-clean rerun traversed the identical 21,489,353 nodes and 238,217,814 row options in
30.41 wall / 29.76 user seconds.  Its source SHA-256 is
`2a5a6535303569ded90eda9cc40265a5d705c99b574db2ddd8574fa2b8b9fce9`, build ID is
`15a5b75c637c505ad16735c173b0eff70e972bfb306313d8762c66f65bc83d7d`, and binary SHA-256 is
`942963f03a42eeeb3897f6daa1e502f62a2ac59be5e0c315fda4129929c7dcd5`.  The log passes
`tools/check_provenance.py`.  This remains a direct row-triple exhaustion: it does not call or
reimplement the Tight-Band Capacity Obstruction.  See the
[dyadic-family record](singleton_dyadic_counterexample_family_2026-08-31.md) for the independent
analytic proof and the full construction.

The optional `k7-dyadic-family-control` mode now locks the exact negative verdict and all search
counts.  A final ASan+UBSan build repeated both the standard regression and that full control with
no finding.  Its final source SHA-256 is
`2e66912b16059b543f789bd0612999967b0960e2dc7619efe132a9dcd825d7f1`, build ID is
`fa93a5579885857cda37736a56b005b80a6f44ec5a11fffe699a33073429f646`, and the sanitizer binary
SHA-256 is `7c5e03cb67138475ae64f2ef0a28902acb727e89fd5f30028003ff803796c1f1`.

## Scope

This establishes an independent exhaustive implementation check for both the full-mass hole and
its underfull core.  It does not supersede the two-line analytic capacity contradiction, which
remains shorter and stronger as a proof.  The extension completely classifies the fixed
rank-15/32 face and independently confirms the first new `K=7` member.  It does not classify holes
outside the surveyed faces or prove that every minimal hole has a two-anchor certificate.  The
separate [prefix-cylinder census](singleton_k5_prefix_cylinder_2026-08-31.md) subsequently settles
`K=5` positively.
