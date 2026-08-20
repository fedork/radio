import random, sys
k=int(sys.argv[1]); n=int(sys.argv[2])
# Optional explicit mass band as a fraction of cap=3^k; defaults to the original [0.70,1.02].
# Needed because a fixed band does NOT transfer across k -- see the 2026-08-20 recursive-value
# note: at k=7 that fixed band sampled 0/300 solvable (sum of per-part solo Pareto maxima grows
# almost as fast as the cap itself, so "mass near cap" increasingly means "mass past what any
# part-respecting split can carry," not a level-independent difficulty).
lo = float(sys.argv[3]) if len(sys.argv) > 3 else 0.70
hi = float(sys.argv[4]) if len(sys.argv) > 4 else 1.02
rng=random.Random(1000+k)
cap=3**k; out=[]
seen=set()
# Width cap 112 was tuned at k=5 (cap=243). It does not scale with k, so a hard 112 makes k>=7
# infeasible outright at the default band: the mass band needs sum(n_i*m_i) ~ 0.7-1.0*cap, and by
# AM-GM that forces sum(n_i+m_i) >~ 2*sqrt(cap) per part-count-normalized terms. Scale it by
# sqrt(3^k/3^5), floored at 112 so k<=5 is reproduced exactly (matches the committed k=4/k=5
# sample used elsewhere).
WIDTHCAP = max(112, round(112 * (cap/243.0)**0.5))
tries = 0
while len(out)<n:
    tries += 1
    if tries > 400_000:
        break                                    # infeasible band at this k -- give up, don't hang
    parts=[]
    for _ in range(4):
        m=rng.randint(2,7); nn=rng.randint(m, max(m, int(cap/(4*m)*rng.uniform(0.6,1.9))))
        parts.append((nn,m))
    mass=sum(a*b for a,b in parts)
    if not (lo*cap <= mass <= hi*cap): continue
    if sum(a+b for a,b in parts)>WIDTHCAP: continue
    key=tuple(sorted(parts))
    if key in seen: continue
    seen.add(key); out.append(parts)
for p in out:
    print(" ".join(f"{a} {b}" for a,b in p))
