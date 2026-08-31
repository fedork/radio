# Tight-Band Capacity Obstruction

Status: **proved** (2026-08-31).  The theorem is a sufficient certificate of first-cut
infeasibility, not a necessary-and-sufficient characterization.

## Setting

Fix `K>=1`.  Let `h=G_(K-1)`, let `H(t)` be its saturated prefix function, and let `P(t)` be the
saturated prefix function of `G_K`.  Pascal recursion gives

\[
P(t)=H(t)+\max_{0\le p\le t}\bigl(H(p)+H(t-p)\bigr).
\]

Let `a=(a_1>=...>=a_n>0)` be weakly majorized by `G_K`, and write

\[
A(t)=\sum_{i=1}^t a_i,
\qquad
I(t)=\{p:P(t)=H(t)+H(p)+H(t-p)\}.
\]

A rank `t` is **tight** when `A(t)=P(t)`.  A legal first cut assigns each parent row one of

\[
(l_i,m_i,0),\quad l_i+m_i=a_i,
\qquad\hbox{or}\qquad
(0,m_i,r_i),\quad m_i+r_i=a_i,
\]

and requires each of the three sorted child multisets to be weakly majorized by `h`.

For two tight ranks `u<v`, an **endpoint transition** is a pair `(p,q)` with

\[
p\in I(u),\qquad q\in I(v),\qquad
p\le q,\qquad u-p\le v-q.                                      \tag{TB1}
\]

The last two inequalities say that the numbers of left and non-left rows cannot decrease across
the band.  Put

\[
\begin{aligned}
s_L&=q-p, & L_{p,q}&=H(q)-H(p),\\
s_R&=(v-q)-(u-p), & R_{p,q}&=H(v-q)-H(u-p),\\
&&M_{u,v}&=H(v)-H(u).
\end{aligned}                                                    \tag{TB2}
\]

Let `b=(a_(u+1),...,a_v)` be the band and

\[
\delta_v=H(v)-H(v-1).
\]

When `delta_v>0`, define `C_delta(b,s)` as the sum of the `s` largest numbers among
`b_i-delta_v`, with `C_delta(b,0)=0`.

## The obstruction

> **Tight-Band Capacity Obstruction.**  Suppose `u<v` are tight and `delta_v>0`.  Every legal
> first cut induces an endpoint transition `(p,q)` satisfying (TB1), contributes exactly the three
> band masses in (TB2), and sends at least `delta_v` coins from every band row to the mixed child.
> Consequently no legal first cut exists if either some `b_i<delta_v`, there is no endpoint
> transition, or every endpoint transition satisfies
>
> \[
> L_{p,q}>C_\delta(b,s_L)
> \quad\hbox{or}\quad
> R_{p,q}>C_\delta(b,s_R).                                      \tag{TB3}
> \]

*Proof.*  At a tight rank `t`, let `p_t` be the number of the first `t` rows whose left piece is
positive.  At most `t-p_t` of those rows have a positive right piece, because no row can feed both
pure children.  The contributions from these `t` parent rows therefore obey

\[
P(t)=A(t)
 \le H(p_t)+H(t)+H(t-p_t)
 \le P(t).
\]

Both inequalities are equalities.  Hence `p_t` lies in `I(t)`, and the left, mixed and right
contributions from the first `t` rows have masses exactly `H(p_t)`, `H(t)` and `H(t-p_t)`.
Taking `p=p_u` and `q=p_v` gives (TB1); subtracting the two saturated endpoint equalities gives
(TB2).

Now remove the mixed piece of any one band row from the `v` mixed contributions.  The remaining
`v-1` pieces are a child submultiset and have mass at most `H(v-1)`.  Since all `v` pieces have
mass `H(v)`, the removed piece has size at least

\[
H(v)-H(v-1)=\delta_v.
\]

Thus every band row retains at most `b_i-delta_v` coins for its one possible pure child.  The left
band mass uses exactly `s_L` positive left rows, while the right band mass uses at most `s_R`
positive right rows.  Their respective capacities are therefore at most
`C_delta(b,s_L)` and `C_delta(b,s_R)`.  If every possible transition violates one of these bounds,
none can come from a legal cut.  This proves (TB3).  The same argument immediately rules out a row
with `b_i<delta_v`.  QED.

This proof deliberately defines `p_t` by **positive left pieces**.  Rows sent wholly to the mixed
child need no arbitrary left/right orientation, so no compatibility assumption about two
independent endpoint colorings is hidden in the transition.

## The `K=6` certificate

For the counterexample, take `u=15`, `v=32` and

\[
b=(8^{15},7^2).
\]

For `h=G_5`,

\[
I(15)=\{7,8\},\qquad I(32)=\{16\},\qquad
\delta_{32}=H(32)-H(31)=1.
\]

The mixed band mass is `H(32)-H(15)=22`.  The two endpoint transitions are:

| transition | left rows | left required / capacity | right rows | right required / capacity | obstruction |
|---|---:|---:|---:|---:|---|
| `7->16` | 9 | `64 / 63` | 8 | `48 / 56` | left |
| `8->16` | 8 | `48 / 56` | 9 | `64 / 63` | right |

Thus both transitions fail.  The same state also has valid but longer certificates at anchor pairs
`(15,30)` and `(15,31)`, with six and four transitions respectively.  Rank 32 is the preferred
human certificate because it leaves only the two cases above.  Exact machine output is in the
[capacity-certificate record](../../evidence/singleton_tight_band_capacity_2026-08-31.md).

## A counterexample at every `K>=6`

The same obstruction yields an explicit infinite family, although the anchors must widen as `K`
grows.  Write

\[
V_K(j)=\sum_{i=j}^K {K\choose i};
\]

then `G_K` contains `2^(j-1)` copies of `V_K(j)` for `j>=1`.  Let `K=2m` or `2m+1`, with
`m>=3`, and set

\[
d=m+1,\qquad n=2^d,qquad u=n-1,qquad v=2n.
\]

The canonical band on ranks `u+1,...,v` is

\[
b_0=(V_K(d),V_K(d+1)^n).
\]

Let `M` be its mass, write `M=(n+1)q+r` with `0<=r<n+1`, and replace it by the balanced
equal-mass band

\[
b=((q+1)^r,q^{n+1-r}).                                       \tag{TB5}
\]

Keeping every row outside the band canonical gives a full-mass, exact-support parent `a_K`.
Balancing only moves mass downward, so `a_K<=_w G_K`.

> **Dyadic Balanced-Band Family.**  For every `K>=6`, the parent `a_K` in (TB5) has no legal
> first cut into three `G_(K-1)`-majorized children.  Consequently singleton majorization is
> insufficient at every level `K>=6`.

Here is the capacity calculation.  Put

\[
A=V_{K-1}(d-1),\quad B=V_{K-1}(d),\quad C=V_{K-1}(d+1),
\quad L=n+1,\quad s=n/2+1.
\]

Concavity gives `I(n-1)={n/2-1,n/2}` and `I(2n)={n}`.  Thus there are two transitions, and in
either one a pure child needs `A+(n/2)B` coins from `s` band rows.  The mixed floor is `C`.
Writing `B_b(s)` for the balanced band's `s`-row prefix, both transitions are therefore blocked
if

\[
B_b(s)\le A+(n/2)B+sC-1.                                    \tag{TB6}
\]

Pascal recursion and one simplification give

\[
T-\frac{sM}{L}=\frac FL-1,qquad
F=\frac n2 {K-1\choose d-1}-s{K-1\choose d},                \tag{TB7}
\]

where `T` is the right side of (TB6).  Integer balancing gives

\[
B_b(s)-\frac{sM}{L}<\frac L4.                                \tag{TB8}
\]

For even and odd `K`, respectively,

\[
F_m^E={2m-1\choose m+1}\left(\frac n{m-1}-1\right),\qquad
F_m^O={2m\choose m+1}\left(\frac n{2m}-1\right).
\]

At `m=5`, both exceed `L^2/4+L`.  Adjacent binomial ratios show
`F_{m+1}^E>4F_m^E` and `F_{m+1}^O>4F_m^O`, while doubling `n` multiplies
`L^2/4+L` by less than four.  Hence (TB7)--(TB8) prove (TB6) for `m>=5`.
The four remaining cases `K=6,7,8,9` are the exact integer bases; their two sides in (TB6) are
`72=72`, `280=280`, `663=663`, and `2278<=2279`.  This proves the theorem.

The first new member is

\[
(128,127,120^2,99^4,64^7,32,31^{16},8^{32},1^{64})
\preceq_w G_7.
\]

An independent direct-row exhaustion rejects it in 21,489,353 nodes.  The full algebra, the first
examples, a 20-counterexample dyadic survey through `K=15`, and provenance are in the
[dyadic-family record](../../evidence/singleton_dyadic_counterexample_family_2026-08-31.md).

## Complete fixed-face classification

Keep the first 15 rows

\[
(64,63,57^2,42^4,22^7)
\]

and, optionally, the final `1^32` fixed.  Let the intervening band `b` range over every 17-part
partition of mass 134 weakly majorized by `(22,7^16)`.  This is the exact tight
`[15,32)` face considered here; it contains 176 bands.

Two independent computations give a complete **first-cut** classification of this face:

- the direct-row clean-room solver finds a legal majorized-child cut for 175 bands and exhausts
  the remaining one;
- the inequality-only extractor certifies exactly one band, using three possible anchor pairs.

The two unique states agree:

\[
b=(8^{15},7^2).
\]

Therefore the counterexample is the unique first-cut hole on this fixed face.  If transfer distance
on the face is defined by

\[
d(b,c)=\tfrac12\sum_i |b_i-c_i|,
\qquad c=(22,7^{16}),
\]

its distance is 14 and hence is minimal on this face.  This face calculation is not by itself a
claim of global `K=6` hole minimality; the exhaustive boundary of the **certificate class** is
sharpened below, and a later direct transfer-shell census closes the global exact-support
no-first-cut distance question.
The enumeration, replay policy, node counts and provenance are in the
[capacity-certificate record](../../evidence/singleton_tight_band_capacity_2026-08-31.md).

## Exhaustive boundary of this certificate class

The obstruction can be surveyed without searching row allocations.  For each anchor pair, every
transition gives the disjunction

\[
B(s_L)\le L_{p,q}+s_L\delta_v-1
\quad\hbox{or}\quad
B(s_R)\le R_{p,q}+s_R\delta_v-1,                              \tag{TB4}
\]

where `B(s)` is the band prefix sum.  Selecting one side of each disjunction gives finitely many
additional prefix caps.  Componentwise stronger cap vectors may be discarded, and a dynamic
program over sorted partitions minimizes half the `l1` distance to the canonical band under every
remaining vector.  Rows below `delta_v` form one additional disjunct.  This exhausts precisely the
certificates allowed by the theorem, not all possible first-cut obstructions.

At `K=5`, `delta_v>0` forces `v<=16`.  Complete enumeration of all 613,689,090 dominated bands on
the resulting 136 anchor faces finds no certificate.  Thus the two-anchor theorem cannot settle
`K=5`; absence is not a positive first-cut verdict.

Across all 528 eligible `K=6` anchor faces, the exact optimization finds minimum transfer distance
14.  The minimizer at anchors `(15,30)` replaces `(22,7^14)` by `(8^15)`; restoring the canonical
tail gives the same counterexample band `(8^15,7^2)`.  Therefore distance 14 is globally minimal
within the Tight-Band Capacity certificate class.  A separate exact Fixed-Color Hall census has
since checked all 5,189,450,419 exact-support parents through distance 13 and found no uncertified
no-first-cut hole.  Hence the same distance is globally minimal for that first-cut problem, while
`K=5` and recursive-unsolvability minimality remain undecided.  See the
[transfer-shell record](../../evidence/singleton_transfer_shell_census_2026-08-31.md); counts, the
optimization argument and clean provenance for the inequality class remain in the
[capacity-certificate record](../../evidence/singleton_tight_band_capacity_2026-08-31.md).

## Machine checker

[`tools/singleton_tight_band_certificate.cpp`](../../tools/singleton_tight_band_certificate.cpp)
recomputes the Pascal profiles and evaluates only the tight-prefix, transition and capacity
inequalities above.  Its `survey-dyadic-family 15` mode also constructs and checks all dyadic
family members through the supported level.  It contains no split search, Hall test, result cache
or dependency on the existing interval census.  The companion clean-room solver independently
classifies the same 176 `K=6` bands by direct legal row triples.  Reproduce the standard controls
with

```sh
tools/singleton_tight_band_regression.sh
```

The extractor is one-sided: absence of a certificate does not imply a first cut exists.  The 175
positive face verdicts come from the separate direct solver, which replays every returned row
allocation and rechecks all three child majorizations.
