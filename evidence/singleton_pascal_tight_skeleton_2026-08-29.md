# Pascal tight-skeleton factorization and interval census (2026-08-29)

**Superseded status (2026-08-30).**  The factorization and minimum-support reduction below remain
proved, but Positive-Band Extension is false.  The `K=6` band `[15,32)` with target `(8^15,7^2)`
extends neither of its two forced endpoint transitions; see
[the counterexample](singleton_k6_counterexample_2026-08-30.md).

## Outcome

The low-multiplicity shape is governed by all tight Pascal prefixes, not only the dyadic ones.
There is an exact factorization theorem: the tight prefixes of a parent define a one-dimensional
path of pure-row counts, and consecutive parent bands contribute independent local allocation
matrices.  The number of global oriented allocations is a transfer-matrix product along that path.
The earlier dyadic Cartesian product is the special case in which every intermediate matrix has
one row and one column.

This is a proved structural theorem, not the Singleton Majorization Converse.  A second proved
reduction now removes the apparent arbitrary-support difficulty: because half the rows of `G_K`
are units, repeatedly merging the two smallest parent rows preserves majorization until exactly
`2^K` rows remain, and any split of the merged state lifts back.  Thus any full counterexample
reduces to minimum support; the `K=6` hole already has that support.  The formerly proposed
Pascal-band existence statement is false.

## Tight-skeleton factorization theorem

Put `h=G_(K-1)`, let `H` be its saturated prefix function, and write

    F(t)=H(t)+max_(0<=p<=t) (H(p)+H(t-p)) = H_K(t).

Define the Pascal plateau at rank `t` by

    I(t)=argmax_(0<=p<=t) (H(p)+H(t-p)).                       (TS1)

Let `a` be a sorted parent, and call `t` tight when

    sum_(i<=t) a_i=F(t).                                      (TS2)

In a legal first split, orient a row left if it may feed the left pure and mixed children, and
right if it may feed the mixed and right pure children.  If `p_t` of the first `t` rows are
left-oriented, their three contribution sets have total mass at most

    H(p_t)+H(t)+H(t-p_t) <= F(t).

At a tight rank both inequalities must be equalities.  Therefore

    p_t in I(t),                                               (TS3)

and the left, mixed and right contributions of the first `t` rows saturate child prefixes of
sizes `p_t,t,t-p_t`.

Now take consecutive tight ranks `u<v`.  Subtracting their three saturated masses shows that the
rows `a_(u+1),...,a_v` fill the contracted child profiles

    left:  h[p_u:p_v],
    mixed: h[u:v],
    right: h[u-p_u:v-p_v].                                   (TS4)

The count path is monotone in both colors:

    p_u<=p_v,             u-p_u<=v-p_v.                       (TS5)

The domination in (TS4) is also forced, not just the total mass.  For example, the earlier left
prefix is a tight set of the cardinality polymatroid with rank `H`; adjoining any `s` later left
rows gives at most `H(p_u+s)`, so those later rows have mass at most
`H(p_u+s)-H(p_u)`.  This is exactly contraction by the interval `h[p_u:p_v]`.  The same argument
applies to the other children.

Conversely, choose a monotone path `p_t in I(t)` and a legal contracted allocation (TS4) for every
band.  Concatenating the parent-row allocations is legal.  In each child, direct sums preserve
majorization: the union of vectors dominated by consecutive intervals of `h` is dominated by the
union of those intervals, namely `h`.  Thus the local allocations recombine to a global one.

This proves the **Pascal Tight-Skeleton Factorization Theorem**.  If `N_j(p,q)` counts oriented
local allocations of band `j` from count `p` to count `q`, then at the unquotiented level

    N(a)=sum_(admissible paths p_0,...,p_s) product_j N_j(p_(j-1),p_j).    (TS6)

Existence is the Boolean version of the same matrix product.  Quotienting by global exchange of
the pure sides can identify two terms, but does not change the factorization itself.

At an even dyadic tight rank the Pascal profile drops strictly at the midpoint, so `I(t)={t/2}`.
Formula (TS6) then reduces to the previously proved literal head--tail product.  At plateau or odd
ranks, retaining `I(t)` is essential.

## Half-unit coalescence and minimum-support reduction

The unit block supplies a general lemma which is useful beyond the tight faces.

> **Half-Unit Coalescence Lemma.**  Let `c=(c_1,...,c_m)` be a positive integer partition of a
> fixed mass.  Suppose at least as many entries of `c` equal one as exceed one.  If a positive
> integer partition `a<=_w c` of the same mass has more than `m` parts, replace its two smallest
> parts `x>=y` by `x+y` and sort.  The result is still dominated by `c`.

Here is a direct proof.  Let `s` be the number of non-unit parts of `c`, so `m-s>=s`, and write
`alpha=a'`, `gamma=c'` for conjugate partitions.  Dominance reverses under conjugation, so
`alpha` dominates `gamma`.  If `a` has `r>m` rows, then

    gamma_1=m,     gamma_j<=s for j>=2,
    alpha_j=r for j<=y,       alpha_j>=r-1 for y<j<=x.

Merging `x,y` moves one conjugate cell out of each column `1,...,y` and into columns
`x+1,...,x+y`.  The loss in the first `j` columns is therefore

    D_j = j                 for j<=y,
          y                 for y<=j<=x,
          x+y-j             for x<=j<=x+y,
          0                 for j>=x+y.                         (HC1)

Let `S_j=sum_(d<=j)(alpha_d-gamma_d)` be the old conjugate dominance slack.  For `j<=y`,

    S_j >= (r-m)+(j-1)(r-s) >= j.

Between `y` and `x` the slack cannot decrease, because `alpha_d>=r-1>=s>=gamma_d`.  Finally, for
`j=x+t` with `1<=t<=y-1`,

    S_j-D_j
      >= (y-1)(m-s)-t(s-1) >= 0,                               (HC2)

using `m-s>=s` and `t<=y-1`.  At and after `x+y` the moved cells have all returned, so ordinary
dominance of `alpha` applies.  Thus the new conjugate still dominates `gamma`, proving the lemma.

`G_K` has exactly `2^(K-1)` non-unit entries followed by exactly `2^(K-1)` units.  Every suffix of
`G_K` likewise has at least as many units as non-units.  The hypothesis is substantive:
`(2,2,2)<=_w(3,3)`, but merging its two smallest parts gives `(4,2)`, which is not dominated by
`(3,3)`.  Repeated application of the lemma gives:

> **Minimum-Support Reduction Theorem.**  Any failure of the Row-Coloring property at level `K`
> has a failure among full-mass parents having exactly `2^K` positive rows.

Indeed, a full-mass parent dominated by `G_K` cannot have fewer than `2^K` rows.  Merge its two
smallest rows until exactly that many remain.  Given a legal split of a merged row `x+y`, orient
both original rows the same way and divide its pure amount `d` as

    d_x in [max(0,d-y), min(d,x)],        d_y=d-d_x.

The interval is nonempty.  The mixed remainders complete the two original rows.  In each child this
only refines one part into two, which moves mass toward the tail and therefore preserves child
majorization.  Undoing all merges lifts the exact-support split to the original state.

The same proof applies to a suffix `G_K[u:]`.  Consequently the former Tail Extension Conjecture
is a corollary of Positive-Band Extension for the single terminal band `[u,2^K]`: coalesce the tail
to exactly `2^K-u` rows, apply that band statement, then undo the merges.

The minimum-support case has a useful anchored form.  Put `n=2^(K-1)` and let `a` have exactly
`2n` positive rows.  Equality at the full rank forces exactly `n` rows of each orientation and a
positive pure contribution from every row.  Give one pure coin to every row and write

    bar(a)=(a_i-1),             bar(h)=(h_i-1).

The residual problem is to color `n` rows each way and allocate `bar(a)` to

    bar(h), h, bar(h).                                           (AR1)

This contraction loses no information.  Its induced parent prefix rank is

    H(t)+max_(max(0,t-n)<=p<=min(t,n))
             ((H(p)-p)+(H(t-p)-(t-p)))
      =F(t)-t,                                                   (AR2)

which is exactly the prefix function of `(G_K[i]-1)`.  Thus `bar(a)<=_w(G_K-1)` is precisely the
residual parent condition.  In conjugate language this step removes the unique universal Pascal
column; the remaining doubled/single columns are the nonempty Boolean subsets.  This does not yet
choose the coloring, but it is the smallest exact formulation left by coalescence, and it uses the
unit structure rather than discarding it.

The coalescence argument cannot simply be iterated after removing the anchors.  At `K=3`, the
exact-support parent `(8,7,3,3,3,1,1,1)` has residual positive partition `(7,6,2,2,2)` dominated
by `(G_3-1)_+=(7,6,3,3)`.  Merging the two smallest residual parts gives `(7,6,4,2)`, whose first
three parts have mass 17 rather than the capacity 16.  The first unit column is special.

## The precise existence target

The tight-skeleton theorem isolates the following stronger local statement.

**Positive-Band Extension Conjecture.**  Let `0<=u<v<=2^K`.  Suppose `b` has exactly `v-u`
positive rows, has mass `F(v)-F(u)`, and is weakly majorized by the contracted parent band
`G_K[u:v]`.  For every incoming `p in I(u)`, there is an outgoing `q in I(v)` satisfying (TS5)
and a legal allocation of `b` to

    h[p:q], h[u:v], h[u-p:v-q].                               (BE)

This is Pascal-specific and remains unproved for arbitrary `K`.  Its terminal case gives tail
extension by coalescence.  Its full-band case `u=0,v=2^K` is precisely the exact-support
Row-Coloring statement, and the Minimum-Support Reduction Theorem shows that this one special case
alone would prove the full Singleton Majorization Converse.  The all-band form is stronger than
necessary, but it is the form naturally propagated by tight-skeleton induction.

## Why the count path cannot be fixed in advance

At `K=4`, use the band `[5,9)` of

    G_4=(16,15,11,11,5,5,5,5,1^8).

Its parent capacity is `(5,5,5,1)`, and `b=(4,4,4,4)` is dominated by it.  Here

    I(5)={2,3},             I(9)={4,5}.

Keeping the same side ahead gives transitions `2->4` and `3->5`; neither can allocate `(4^4)`.
Switching the leading side works.  For `2->5`, the child bands are

    left=(4,4,1),          mixed=(1,1,1),          right=(4).

Put pure `4` in one right row and put `(pure,mixed)=(3,1)` in each of the other three rows.  The
global side exchange gives `3->4`.  The exact census finds five parent shapes in this band: four
allow all four transitions, while `(4^4)` allows exactly the two switching transitions.  Thus the
global state variable is the plateau count `p_t`; special-casing equal row values is neither the
definition nor the explanation.

Two stronger shortcuts are false.

1. Dropping the exact band row count fails already at `K=4`.  The nine-row partition

       (16,15,11,11,4^5) <=_w (16,15,11,11,5^4)

   cannot fill prefix child capacities `(8,7,4,4),G_3,(8,7,4,4)`.  Each pure side needs at least
   four rows, so the color counts are `4,5`.  Rows 16 and 15 must be separated because
   `16+15>2(8+7)`.  Put 16 in the four-row side.  If that side contains both 11s, `(p,q)=(4,1)`
   gives `57>23+24+8=55`.  If it contains one or neither, `(2,5)` gives `65>15+26+23=64`.

2. Strict sorted alternation is not the band rule.  At `K=5`, the exact 16-row head

       (32,31,26,26,16,16,16,11,11,6^7)

   is obtained from `G_5[:16]` by one Robin--Hood transfer, hence is dominated by it and has the
   same mass.  Alternation fails the child Hall inequality at `(p,q)=(5,2)`:

       158 = (32+26+16+16+11)+(31+26)
           > H_4(5)+H_4(2)+H_4(7) = 58+31+68 = 157.

   The endpoint transition must therefore be selected globally and adaptively even on an exact
   dyadic head.

3. Nor does alternation become a theorem after the minimum-support reduction, even if the parent
   is strictly inside every prefix face.  At `K=6`, put

       a=(63^2,57^2,42^3,23^5,22^5,3^44,2^3).

   This has 64 rows, mass 729, and is strictly majorized by

       G_6=(64,63,57^2,42^4,22^8,7^16,1^32).

   The prefix slack is one through rank seven; the smoothed ten-row band returns it to one at rank
   17; it then rises to 61 and falls to one before the final equality.  Thus there is no internal
   tight rank.  Nevertheless sorted odd/even alternation fails at `(p,q)=(9,3)`:

       A_9+B_3 = 478
         > H_5(9)+H_5(3)+H_5(12) = 185+89+203 = 477.

   Exhaustion happens to find no such strict-interior alternating failure through `K=4`; this
   compact `K=6` state shows that the finite pattern is not an induction invariant.

## Exact finite census

`tools/singleton_pascal_interval_census.cpp` enumerates parent bands independently of the child
search.  A parent row is searched in the forms `(pure,mixed,0)` and `(0,mixed,pure)`; child
majorization is checked after every row.  Transition mode additionally enforces exactly `q-p`
left-oriented rows and `(v-q)-(u-p)` right-oriented rows, so splitting one pure capacity over extra
rows cannot create a false endpoint transition.

The complete positive-band extension census gives:

| level | rank bands | exact band-state instances | search nodes | result |
|---:|---:|---:|---:|---|
| 3 | 36 | 561 | 8,482 | every incoming count extends |
| 4 | 136 | 1,722,516 | 1,443,610,330 | every incoming count extends |

The final `K=4` run took 127.473 in-process seconds / 132 wrapper wall seconds.  Its hardest single
state used 2,299,669 nodes in band `[0,12)`.

As an independent check of the coalescence consequence, the direct tail-extension census gives:

| level | `(u,p)` cases | state--case instances | search nodes | result |
|---:|---:|---:|---:|---|
| 3 | 13 | 443 | 4,859 | pass |
| 4 | 37 | 1,422,304 | 39,273,306 | pass |

The final `K=4` tail run took 5.529 in-process seconds.

The full-band slice gives a much smaller exact-support corpus than the unrestricted parent census:

| level | exact-support parents | without internal tight prefix | alternating failures | strict-interior alternating failures |
|---:|---:|---:|---:|---:|
| 3 | 160 | 33 | 0 | 0 |
| 4 | 408,776 | 63,329 | 1,968 | 0 |

Every one of these parents has a legal allocation by the complete positive-band census.  The final
column is only a finite observation and is false in general by the displayed `K=6` counterexample.
All census statements in this section are finite theorems only.

Among all 5,997,038 full-mass `K=4` parents, exactly 1,000,432 have an internal tight prefix and
4,996,606 do not.  Thus literal factorization at a parent equality applies directly to about one
sixth of the unrestricted corpus.  Coalescence changes the relevant hard core: it reduces all
5,997,038 parents to exact-support states, of which 345,447 have an internal tight prefix and only
63,329 are strict-prefix interior.  Tight structure is even more concentrated on the low-fiber
boundary: 253 of the 259 parents with one, two or three child-shape orbits have an internal tight
prefix.  By parent multiplicity, the with/without counts are

| child-shape orbits | with tight skeleton | without |
|---:|---:|---:|
| 1 | 29 | 1 |
| 2 | 122 | 1 |
| 3 | 102 | 4 |

The six exceptions are

    (15^2,1^51), (15,1^66), (14,1^67), (13^4,1^29), (2,1^79), (1^81).

This explains why the low-multiplicity survey displayed such a strong parent--cut isomorphism.
The proved coalescence step removes row-count refinement as an obstacle, but it does not allocate
the remaining 63,329 strict exact-support states in general; the exact-support Row-Coloring case is
still the open core.

## Reproduction

```text
CC=clang++ tools/build_radio.py -std=c++20 -O3 \
  tools/singleton_pascal_interval_census.cpp \
  -o /tmp/singleton_pascal_interval_census

tools/capped_run.sh --seconds 1800 --rss-gb 4 --label pascal-k4-bands -- \
  tools/run_with_provenance.py /tmp/singleton_pascal_interval_census \
  4 all-left-extension-bands

tools/capped_run.sh --seconds 1800 --rss-gb 4 --label pascal-k4-tails -- \
  tools/run_with_provenance.py /tmp/singleton_pascal_interval_census \
  4 all-tail-extensions

tools/run_with_provenance.py /tmp/singleton_pascal_interval_census \
  4 transition-band 5 9
tools/run_with_provenance.py /tmp/singleton_pascal_interval_census \
  4 transition-band 0 16
tools/run_with_provenance.py /tmp/singleton_pascal_interval_census \
  strict-exact-alternation-counterexample
tools/run_with_provenance.py /tmp/singleton_pascal_interval_census \
  4 tight-state-count

tools/singleton_low_multiplicity_analysis.py \
  /tmp/singleton_k3_cuts.log /tmp/singleton_k4_cuts.log
```

The complete band run used build id
`845ccebcc115cd9bdd34616c953253a08bd3af81ecd3b5b2c93c196890c392b4`; the source build at the
conclusion of this particular study, which added regression/reporting checks and reproduced the
exact-support and counterexample checks, was
`8bf9d29627cb7c2bb1af37dc8e3c605ef4315e9425d0c1027209b0e6f35011dc`.  The tool was subsequently
extended with two-anchor residual modes while retaining those regressions; its current provenance
is recorded in the
[two-anchor residual record](singleton_two_anchor_residual_2026-08-29.md).
The complete band run exited zero under its 30-minute/4-GiB cap; all other displayed final runs
also exited zero.  No process remains.
