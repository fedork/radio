import random, sys
k=int(sys.argv[1]); n=int(sys.argv[2]); rng=random.Random(1000+k)
cap=3**k; out=[]
seen=set()
while len(out)<n:
    parts=[]
    for _ in range(4):
        m=rng.randint(2,7); nn=rng.randint(m, max(m, int(cap/(4*m)*rng.uniform(0.6,1.9))))
        parts.append((nn,m))
    mass=sum(a*b for a,b in parts)
    if not (0.70*cap <= mass <= 1.02*cap): continue
    if sum(a+b for a,b in parts)>112: continue
    key=tuple(sorted(parts))
    if key in seen: continue
    seen.add(key); out.append(parts)
for p in out:
    print(" ".join(f"{a} {b}" for a,b in p))
