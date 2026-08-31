# An exact-support `K=6` counterexample to singleton-majorization sufficiency

Status: **proved analytically, checked by the original Hall/direct formulations, and independently
rechecked by a clean-room direct-row implementation** (updated 2026-08-31).

## Statement

Write repeated entries with exponents.  The canonical profiles are

    G_5=(32,31,26^2,16^4,6^8,1^16)
    G_6=(64,63,57^2,42^4,22^8,7^16,1^32).

The full-mass exact-support singleton state

    a=(64,63,57^2,42^4,22^7,8^15,7^2,1^32)                 (K6-1)

has 64 positive rows, mass 729, and satisfies `a<=_w G_6`, but it has no legal
first split into three children weakly majorized by `G_5`.  Consequently it is
not solvable in six tests.  This refutes all of the following universal
statements:

- the Singleton Majorization Converse;
- the Row-Coloring conjecture;
- Pascal orthant/lattice saturation and global Robin--Hood closure;
- niceness, and hence strong niceness, of every transcript graph `Q_K`;
- the Positive-Band Extension Conjecture;
- the Balanced Residual Coloring Lemma.

The necessity direction remains true: every singleton state solvable in `K`
tests is weakly majorized by `G_K`.

The especially short two-case proof uses only the first 32 rows.  Consequently the unit-free core

    a_core=(64,63,57^2,42^4,22^7,8^15,7^2)                    (K6-1c)

of mass 697 is already majorized by `G_6` and unsolvable in six tests.  The
displayed full-mass state (K6-1) is its canonical padding by 32 unit rows; it is
retained because it answers the stronger full-mass exact-support question with
all unit rows left in place.

A further truncation, noticed when the corrected production solver emitted its sound negative
prefix, is already enough:

    a_30=(64,63,57^2,42^4,22^7,8^15).                       (K6-1m)

This state has mass 683 and support 30.  It is weakly majorized by `G_6` and has no legal first
split.  Deleting one of its fifteen 8s does restore first-cut feasibility, with an explicit cut
given below.  Thus (K6-1m) is prefix-minimal along this truncation, although no global minimum-mass
or minimum-support claim is made.

## Majorization

The first 15 entries of (K6-1) agree with `G_6`.  Between ranks 16 and 31 it
replaces

    (22,7^15)  by  (8^15,7).

Both blocks have mass 127.  For the first `1<=r<=15` rows of the new block,

    8r <= 22+7(r-1) = 7r+15,

and equality holds at `r=15`; at `r=16` both complete blocks have mass 127.
Every prefix before and after the block therefore has the required inequality,
and all later prefixes have equality.  This proves
`a<=_w G_6`; no enumeration is involved.

For (K6-1m), canonical ranks 16--30 are `(22,7^14)`, of mass 120, while the new
band is `(8^15)`, also of mass 120.  The same inequality holds for `1<=r<=14`,
and equality holds after all 15 band rows.  Hence (K6-1m) is majorized as well.

## No legal first split

Let `H(t)` be the saturated prefix sum of `G_5`.  The values needed below are

    H(7)=163, H(8)=179,
    H(15)=221, H(16)=227,
    H(31)=242, H(32)=243.

Suppose for contradiction that (K6-1) has a legal first split.  Color a row `A`
if it feeds the left pure child and the mixed child, and `B` if it feeds the
mixed and right pure children.  If `p` of the globally largest `t` rows have
color `A`, their total mass is at most

    H(p)+H(t)+H(t-p).                                       (K6-2)

The parent is tight at ranks 15 and 32:

    sum_(i<=15) a_i = 563 = H(7)+H(15)+H(8),
    sum_(i<=32) a_i = 697 = H(16)+H(32)+H(16).

Concavity of `H` makes the maximizing color counts respectively

    p in {7,8},       q=16.                                  (K6-3)

Moreover, equality in (K6-2) forces all three child bounds to be saturated at
both ranks.  The 17 rows between those ranks, `(8^15,7^2)`, therefore send exactly

    H(32)-H(15)=22

coins to the mixed child.  Every one of those 17 mixed pieces is positive.  If
one were zero, the other 16 band pieces together with the 15 head pieces would
be at most 31 entries of the mixed child but would have mass

    H(15)+22=243 > H(31)=242,

contradicting mixed-child majorization.  Integrality is the decisive step: each
band row consequently retains at most seven of its at-most-eight coins for its
pure child.

There are only two endpoint-count transitions.  They give the same impossible
requirement on opposite pure children:

| `p` | pure side | band rows on that side | required pure mass | maximum after one mixed coin per row |
|---|---:|---:|---:|---:|
| `7` | `A` | 9 | `H(16)-H(7)=64` | `9*7=63` |
| `8` | `B` | 9 | `H(16)-H(7)=64` | `9*7=63` |

Thus no transition is possible and no legal first split exists.  If the state
were solvable in six tests, each child of its first test would be solvable in
five and hence, by the proved necessity theorem, majorized by `G_5`.  The
absence of such a first split therefore proves six-test unsolvability.

### The smaller rank-15/30 obstruction

For (K6-1m), the tight ranks are 15 and 30.  Besides the values above, use

    H(14)=215, H(30)=241.

The endpoint color counts are `{7,8}` at rank 15 and `{14,15,16}` at rank 30.  The fifteen
intervening rows `(8^15)` send `H(30)-H(15)=20` coins to the mixed child.  Every mixed piece is
positive, since otherwise mass 241 would occupy at most 29 mixed-child entries, contradicting
`H(29)=240`.  Each row therefore retains at most 7 pure coins.  The six transitions are all
impossible:

| transition | blocked side | rows | required | capacity |
|---|---|---:|---:|---:|
| `7->14` | left | 7 | 52 | 49 |
| `7->15` | left | 8 | 58 | 56 |
| `7->16` | left | 9 | 64 | 63 |
| `8->14` | right | 9 | 64 | 63 |
| `8->15` | right | 8 | 58 | 56 |
| `8->16` | right | 7 | 52 | 49 |

This proves that (K6-1m) has no legal first cut.  Rank 32 remains the cleaner presentation for the
full-mass state because it contracts these six cases to the two symmetric `64>63` violations.

This proof is exactly the Pascal tight-skeleton mechanism.  The Pascal profile
does not merely help choose a cut: its two tight ranks force the child prefix
saturations, and the one-unit tail of the contracted mixed band creates the
integral obstruction.

## The one-transfer phase boundary

The changing 16-row core is reached from the canonical core by the monotone path

    b_j=(22-j,8^j,7^(15-j)),       0<=j<=14.                  (K6-4)

Each step moves one coin from the first row to a distinct row of size seven.
Together with the adjacent unchanged row of size seven, its last state is the
17-row obstruction band `(8^15,7^2)` used above.
The exact fixed-color Hall search finds a majorized first cut for every
`j<=13` and none for `j=14`.  Hence the very last Robin--Hood transfer is a
phase loss.  Reversing transfers makes the solution fiber locally reachable,
but it does not imply downward closure.

For `j=13`, a direct first split has normalized children

    L=G_5,
    M=(32,31,26^2,16^4,6^7,2^5,1^12),
    R=(32,31,26^2,16^3,8,7^8,1^16).

All three are weakly majorized by `G_5`.  This is a certificate for first-cut
feasibility at the last good point, not by itself a complete six-test strategy.

## Residual counterexample

Applying the proved Two-Anchor Reduction to (K6-1) gives

    b=(62,61,55^2,40^4,20^7,6^15,5^2),                      (K6-5)

which is dominated by

    J=(62,61,55^2,40^4,20^8,5^16).

Its residual child profile is

    c=G_5-1=(31,30,25^2,15^4,5^8).

If (K6-5) had the capped balanced residual coloring, the Two-Anchor Reduction
would lift it to a legal split of (K6-1).  Therefore (K6-5) is also an explicit
counterexample to the Balanced Residual Coloring Lemma.  The verifier checks
this residual Hall failure directly as well.

## Independent executable checks

`tools/singleton_pascal_interval_census.cpp` has a dedicated regression mode.
It performs four logically separate checks:

1. recompute `G_5,G_6`, full mass, support, every parent prefix, and the two
   tight ranks;
2. enumerate every normalized row coloring and apply Fixed-Color Hall;
3. enumerate the integer row splits directly, without the coloring reduction;
4. contract the tight band and exhaust both forced count transitions.

It also checks the 14-step transfer path and the two-anchor residual.

Reproduce with

```sh
CC=clang++ tools/build_radio.py -std=c++20 -O3 -Wall -Wextra -pedantic \
  tools/singleton_pascal_interval_census.cpp \
  -o /tmp/singleton_pascal_interval_census
tools/run_with_provenance.py /tmp/singleton_pascal_interval_census \
  singleton-k6-counterexample
```

The final summary reports

    SINGLETON_K6_COUNTEREXAMPLE verified=YES ...
      mass=729 support=64 tight_ranks=(15,32)
      full_coloring_found=NO full_nodes=7
      direct_split_found=NO direct_nodes=9373
      residual_coloring_found=NO residual_nodes=7
      first_failed_transfer=14 forced_transitions=2 band_nodes=4638

The separate clean-room implementation is
[`tools/singleton_direct_split_cleanroom.cpp`](../tools/singleton_direct_split_cleanroom.cpp).
It shares no Hall code, cache, or search implementation with the census, handles underfull child
slack, and directly enumerates sorted child row triples.  Reproduce all controls with

```sh
tools/singleton_direct_split_regression.sh
```

It finds the canonical `G_6` cut, reproduces the displayed `j=13` children, and independently
rejects the padded `j=14` state, its mass-697 core, and the mass-683 prefix.  Each negative
exhausts 9,345 DFS states and 34,958 row options.  Removing one 8 from the last state is feasible
in 1,245 nodes and has the explicit children

    L=(32,31,26^2,16^4,6^5),
    M=(32,31,26^2,16^4,6^7,2^5,1^8),
    R=(32,31,26^2,16^3,8,7^8).

Its naive unquotiented oracle agrees with the optimized search on
201 small partitions, and its full-mass census closes all 1,223 majorized types at `K<=3`,
including all 1,206 at `K=3`.
Exact provenance, branch/prune counts, the replayable `j=13` allocation and sanitizer commands are
in the [clean-room verification record](singleton_direct_split_cleanroom_2026-08-31.md).

The analytic argument has also been generalized to the proved
[Tight-Band Capacity Obstruction](../docs/theorems/tight-band-capacity.md).  Its separate
inequality-only extractor finds the preferred `(15,32)` certificate above without performing a
split search.  Together with a new clean-room face mode, it exhausts all 176 exact 17-row bands
dominated by `(22,7^16)`: 175 have a majorized first cut and `(8^15,7^2)` is the unique hole.
Consequently transfer distance 14 is minimal on this fixed face.  An extended prefix-cap
optimization now proves that 14 is also globally minimal among all `K=6` parents carrying a
Tight-Band Capacity certificate.  A later exact Fixed-Color Hall census checks all 5,189,450,419
exact-support parents through distance 13 and excludes a closer uncertified no-first-cut hole as
well.  Thus 14 is the global exact-support first-cut minimum.  The complete `K=5` certificate
census is empty, but a later prefix-cylinder census proves all 1,431,800,647,444 exact-support
`K=5` parents recursively solvable.  Thus 14 is also the minimum exact-support recursive failure
distance at `K=6`.  See the [capacity record](singleton_tight_band_capacity_2026-08-31.md),
[transfer-shell record](singleton_transfer_shell_census_2026-08-31.md), and
[complete `K=5` record](singleton_k5_prefix_cylinder_2026-08-31.md).

All exhaustive runs are regression checks.  The short saturation-and-integrality argument above
is the proof.

## Scope

The complete normalized census proves the converse through `K=5`: the new `K=5` prefix-cylinder
census extends the earlier `K<=4` exhaustion to the entire exact-support boundary.  This
construction first obstructs at `K=6`, so `K=6` is the proved minimum counterexample level.
