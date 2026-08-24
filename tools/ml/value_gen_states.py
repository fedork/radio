import random, sys
k=int(sys.argv[1]); n=int(sys.argv[2])
# Optional explicit mass band as a fraction of cap=3^k; defaults to the original [0.70,1.02].
# Needed because a fixed band does NOT transfer across k -- see the 2026-08-20 recursive-value
# note: at k=7 that fixed band sampled 0/300 solvable (sum of per-part solo Pareto maxima grows
# almost as fast as the cap itself, so "mass near cap" increasingly means "mass past what any
# part-respecting split can carry," not a level-independent difficulty).
lo = float(sys.argv[3]) if len(sys.argv) > 3 else 0.70
hi = float(sys.argv[4]) if len(sys.argv) > 4 else 1.02
# 2026-08-24: generalized from a hardcoded 4 parts to an explicit part count, plus an explicit
# per-part m-range, needed to generate corpora matching OTHER shapes canSolveB_ctx actually
# produces (a size=1 leaf query like Sb(112:80) has one large, lopsided part -- m~80 -- nothing
# like the original 4-part battery's m in [2,7]). Defaults reproduce the original 4-part/[2,7]
# generator exactly when omitted, so every existing call site is unaffected.
nparts = int(sys.argv[5]) if len(sys.argv) > 5 else 4
m_lo = int(sys.argv[6]) if len(sys.argv) > 6 else 2
m_hi = int(sys.argv[7]) if len(sys.argv) > 7 else 7
# Hard ceiling on sum(n_i+m_i), e.g. to stay under a labeling oracle's compiled MAX_N. Default
# is effectively unbounded so every existing call site (which never passes this) is unaffected.
widthcap_max = int(sys.argv[8]) if len(sys.argv) > 8 else 10**9
rng=random.Random(1000+k)
cap=3**k; out=[]
seen=set()
# Width cap 112 was tuned at k=5, nparts=4 (cap=243). It does not scale with k or nparts, so a
# hard 112 makes k>=7 infeasible outright at the default band: the mass band needs
# sum(n_i*m_i) ~ 0.7-1.0*cap, and by AM-GM that forces sum(n_i+m_i) >~ sqrt(nparts*cap) (not
# linear in nparts). Scale by sqrt(3^k/3^5) * sqrt(nparts/4), floored at 112 so the original
# k<=5, nparts=4 sample is reproduced exactly.
WIDTHCAP = min(widthcap_max, max(112, round(112 * (cap/243.0)**0.5 * (nparts/4.0)**0.5)))
tries = 0
while len(out)<n:
    tries += 1
    if tries > 400_000:
        break                                    # infeasible band at this k -- give up, don't hang
    parts=[]
    for _ in range(nparts):
        m=rng.randint(m_lo,m_hi)
        nn=rng.randint(m, max(m, int(cap/(nparts*m)*rng.uniform(0.6,1.9))))
        parts.append((nn,m))
    mass=sum(a*b for a,b in parts)
    if not (lo*cap <= mass <= hi*cap): continue
    if sum(a+b for a,b in parts)>WIDTHCAP: continue
    key=tuple(sorted(parts))
    if key in seen: continue
    seen.add(key); out.append(parts)
for p in out:
    print(" ".join(f"{a} {b}" for a,b in p))
