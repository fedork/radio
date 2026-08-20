"""Decode the certificate chain into (state,k)->unsolvable, gather positives, and CHECK
COMPARABILITY before modelling anything."""
import subprocess, sys, pickle, csv
from collections import defaultdict, Counter
sys.path.insert(0,"/Users/fedor/radio/tools")
from analyze_single_solution_cuts import classify, children as split_children
from analyze_pareto_prefix_census import semantic_state, mass

CERT="/Users/fedor/radio/.artifacts/cert"

def load_level(k):
    txt=subprocess.run(["zstd","-dc",f"{CERT}/sa193-k{k}.cert.zst"],capture_output=True,text=True).stdout
    parts={}; claims=[]
    for line in txt.splitlines():
        w=line.split()
        if not w: continue
        if w[0]=="part":
            n,m=w[2].split(":"); parts[int(w[1])]=(int(n),int(m))
        elif w[0]=="claim":
            claims.append(tuple(int(x) for x in w[1:]))
    return [semantic_state([parts[i] for i in c]) for c in claims]

NEG={}
for k in range(2,9):
    NEG[k]=load_level(k)
    print(f"  certificate level {k}: {len(NEG[k]):,} unsolvable states")

# ---- positives from the censuses
hi,lo=pickle.load(open("/tmp/census_cache.pkl","rb"))
POS=defaultdict(set)
for (rk,states,meta,wins) in (hi,lo):
    cls=classify(states,wins)
    for e,s in states.items():
        POS[rk].add(semantic_state(s))                       # endpoint solvable at rk
        for rep in cls[e].values():
            for kid in split_children(s,rep):
                if kid: POS[rk-1].add(kid)                   # children solvable at rk-1
for k in sorted(POS): print(f"  census positives at k={k}: {len(POS[k]):,}")

print("\nCOMPARABILITY CHECK — mass as a fraction of cap, by level and class")
print(f"  {'k':>2} {'cap':>6} {'class':>10} {'n':>9} {'p10':>6} {'med':>6} {'p90':>6}  {'over cap':>9}")
for k in sorted(set(NEG)|set(POS)):
    cap=3**k
    for tag,pop in (("unsolvable",NEG.get(k,[])),("solvable",sorted(POS.get(k,[])))):
        if not pop: continue
        ms=sorted(mass(s)/cap for s in pop)
        over=sum(1 for s in pop if mass(s)>cap)
        print(f"  {k:2d} {cap:6d} {tag:>10} {len(ms):9,} {ms[len(ms)//10]:6.3f} {ms[len(ms)//2]:6.3f} "
              f"{ms[9*len(ms)//10]:6.3f}  {100*over/len(ms):8.1f}%")
pickle.dump((NEG,dict(POS)),open("/tmp/rec/data.pkl","wb"))
