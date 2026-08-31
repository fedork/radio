# A dyadic balanced-band counterexample at every `K>=6`

Status: **analytic infinite family proved; machine survey through `K=15` and independent direct
`K=7` exhaustion passed** (2026-08-31).

This record extends the isolated `K=6` example using the proved
[Tight-Band Capacity Obstruction](../docs/theorems/tight-band-capacity.md).  The family gives a
full-mass, exact-support state weakly majorized by `G_K` with no legal first split for every
`K>=6`.  Hence the singleton-majorization converse fails at every such level, not only at `K=6`.

## Construction

Write

\[
V_K(j)=\sum_{i=j}^K {K\choose i}.
\]

The Pascal profile has one copy of `V_K(0)` and `2^(j-1)` copies of `V_K(j)` for `j>=1`.
Let `K=2m` or `K=2m+1`, where `m>=3`, and put

\[
d=m+1,\qquad n=2^d,\qquad L=n+1,\qquad s=n/2+1.
\]

At the tight anchors

\[
u=n-1,\qquad v=2n,
\]

the canonical parent band is

\[
b_0=(X,Y^n),\qquad X=V_K(d),\quad Y=V_K(d+1).
\]

Let `M=X+nY`, write `M=Lq+r` with `0<=r<L`, and replace `b_0` by its balanced
equal-mass partition

\[
b=((q+1)^r,q^{L-r}).                                           \tag{D1}
\]

Call the resulting parent `a_K`.  Balancing is a sequence of Robin--Hood transfers, so `b` is
weakly majorized by `b_0`.  It follows that `a_K <=_w G_K`.  It retains all `2^K` positive rows
and mass `3^K`.

## Why every constructed parent has no first cut

Let `H` be the prefix function of `G_(K-1)` and abbreviate

\[
A=V_{K-1}(d-1),\qquad B=V_{K-1}(d),\qquad C=V_{K-1}(d+1).
\]

Concavity gives exactly

\[
I(n-1)=\{n/2-1,n/2\},\qquad I(2n)=\{n\}.
\]

Thus there are two endpoint transitions.  In either one, one pure child receives `s=n/2+1`
band rows and must receive

\[
R=A+(n/2)B
\]

coins.  The mixed floor is `delta=C`, so the pure capacity of those rows is their prefix sum
minus `sC`.  If `B_b(s)` denotes the sum of the `s` largest balanced band rows, both transitions
are blocked once

\[
B_b(s)\le T:=A+(n/2)B+sC-1.                                  \tag{D2}
\]

Pascal recursion gives `X=A+B` and `Y=B+C`, hence

\[
M=A+(n+1)B+nC.
\]

Set `alpha=A-B=binom(K-1,d-1)` and `beta=B-C=binom(K-1,d)`.  Direct simplification gives

\[
T-\frac{sM}{L}=\frac{F}{L}-1,
\qquad
F=\frac n2\alpha-s\beta.                                     \tag{D3}
\]

For a balanced integer partition,

\[
B_b(s)=sq+\min(s,r),
\qquad
B_b(s)-\frac{sM}{L}<\frac L4.                                \tag{D4}
\]

The second inequality follows separately from `r<=s` and `r>=s`; in both cases the difference
is at most `s(L-s)/L=(L^2-1)/(4L)`.

For even and odd levels respectively, (D3) becomes

\[
\begin{aligned}
F_m^E&={2m-1\choose m+1}\left(\frac n{m-1}-1\right),\\
F_m^O&={2m\choose m+1}\left(\frac n{2m}-1\right),
\end{aligned}                                                  \tag{D5}
\]

where `n=2^(m+1)`.  At `m=5`, these are `1260` and `1134`, both larger than

\[
Q(n):=L^2/4+L=1121.25.
\]

Straight algebra using the adjacent binomial-coefficient ratios gives

\[
\frac{F_{m+1}^E-4F_m^E}{{2m-1\choose m+1}}
=\frac{4n(m-1)+2m-8}{(m+2)(m-1)}>0,
\]

and

\[
\frac{F_{m+1}^O-4F_m^O}{{2m\choose m+1}}
=\frac{2(m-1)(n+1)}{m(m+2)}>0.
\]

Meanwhile `Q(2n)<4Q(n)`.  Therefore `F>=Q(n)` for both parities and every `m>=5`.
Equations (D3)--(D4) then imply (D2).  The four smaller parity bases are exact:

| `K` | `n` | `M` | `(q,r)` | `B_b(s)` | `T` |
|---:|---:|---:|---:|---:|---:|
| 6 | 16 | 134 | `(7,15)` | 72 | 72 |
| 7 | 16 | 528 | `(31,1)` | 280 | 280 |
| 8 | 32 | 1277 | `(38,23)` | 663 | 663 |
| 9 | 32 | 4416 | `(133,27)` | 2278 | 2279 |

Thus (D2) holds for every `m>=3`.  The Tight-Band Capacity Obstruction rejects both transitions,
so `a_K` has no legal first cut for every `K>=6`.  In particular it is not solvable in `K` tests.

## First examples

The first three new full parents are

\[
\begin{aligned}
a_7={}&(128,127,120^2,99^4,64^7,32,31^{16},8^{32},1^{64}),\\
a_8={}&(256,255,247^2,219^4,163^8,93^{15},39^{23},38^{10},9^{64},1^{128}),\\
a_9={}&(512,511,502^2,466^4,382^8,256^{15},134^{27},133^6,
        46^{64},10^{128},1^{256}).
\end{aligned}
\]

Their selected dyadic certificates have blocking margins `1`, `1`, and `2`.  The balanced family
is chosen for its closed form, not for minimum transfer distance.  For example, exact face
optimization finds a different `K=9` certificate at anchors `(63,128)` and distance `61`, whereas
the displayed balanced `K=9` member uses anchors `(31,64)` and distance `122`.

## Machine survey and independent `K=7` exhaustion

The new mode

```sh
survey-dyadic-family 15
```

checks every dyadic face `(2^d-1,2^(d+1))` through `K=15`, reconstructs the balanced band, and
evaluates the transition capacities directly.  It reports

```text
DYADIC_TIGHT_BAND_FAMILY maximum_K=15 complete_within_family=YES
  verified=YES faces=91 counterexamples=20
```

There are no hits at `K=3,4,5`, and at least one at every `K=6,...,15`.  Some levels have several
dyadic certificates; all 20 reported parents are replayed from exact integer data.  This finite
survey is a regression for the formulas above, not the proof of the infinite quantifier.

The independent clean-room direct-row solver also exhausts `a_7` without using the capacity
theorem:

```text
RESULT K=7 parent=(128,127,120^2,99^4,64^7,32,31^16,8^32,1^64)
  feasible=NO nodes=21489353 options=238217814 max_depth=31
  prunes(child=216728462,mass=0,support=0,prefix=0)
```

Its initial implementation took about five minutes under a 300-CPU-second cap.  Hoisting child
construction out of the option-sort comparator preserves the exact 21,489,353-node tree and cuts
the clean rerun to 30.41 wall / 29.76 user seconds.  This optimization adds no prune, cache or
theorem-specific logic; the existing unquotiented tiny oracle and all regression counts still
pass.

## Provenance

Both retained local logs pass `tools/check_provenance.py`.

| run | source SHA-256 | build ID | binary SHA-256 |
|---|---|---|---|
| dyadic family through `K=15` | `f13eb75cea4199595eecf9f28e131968bad84edcceb3452f2d10c482cb544e40` | `fc207120e3f0fcc1bd5614abeb3c12fca2e0b701163111d3cf956d1543c59491` | `060e922d05ebd82616e8e158db6fa20a39e5418087775d73a7dd414a47ca22ff` |
| optimized direct `K=7` | `2a5a6535303569ded90eda9cc40265a5d705c99b574db2ddd8574fa2b8b9fce9` | `15a5b75c637c505ad16735c173b0eff70e972bfb306313d8762c66f65bc83d7d` | `942963f03a42eeeb3897f6daa1e502f62a2ac59be5e0c315fda4129929c7dcd5` |

The family tool contains no split search, Hall code or cache.  The direct solver contains no Hall
code, capacity-certificate shortcut or shared cache, and replays every positive allocation.

Final AddressSanitizer plus UndefinedBehaviorSanitizer builds repeated both standard regressions,
the complete dyadic survey and the full `K=7` direct exhaustion with no finding.  LeakSanitizer is
unsupported on this macOS toolchain, so both used `ASAN_OPTIONS=detect_leaks=0` and
`UBSAN_OPTIONS=halt_on_error=1`.  The final inequality source hash / sanitizer build ID are
`cb40d4fb5501051c380dbe5a5ee7afafab4889e1ba5234ca3f62aef80eebd4a5` /
`753f173439099ac725fcb6db3d863370f42df9181600db508465726cfebda79a`; the final direct source
hash / sanitizer build ID are
`2e66912b16059b543f789bd0612999967b0960e2dc7619efe132a9dcd825d7f1` /
`fa93a5579885857cda37736a56b005b80a6f44ec5a11fffe699a33073429f646`.
