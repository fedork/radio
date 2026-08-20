"""EXACT top-k: enumerate the full stage-2 candidate set per k=7 state (no sampling) and find the
true rank of the best winner under a model trained on k=8 only."""
import sys, time
import numpy as np
sys.path.insert(0,"/Users/fedor/radio/tools"); sys.path.insert(0,"/Users/fedor/radio/tools/ml")
from cut_ranker_data import CORPORA, bound_table, features
from cut_ranker import NAMES, sample_state, four_part, training_set
from sklearn.linear_model import LogisticRegression
from sklearn.preprocessing import StandardScaler
from sklearn.pipeline import make_pipeline
from sklearn.ensemble import HistGradientBoostingClassifier

def part_options(n,m,tab,mmax):
    out=[]
    for a in range(n+1):
        for b in range(m+1):
            ok=True
            for pn,pm in ((a,b),(n-a,m-b),(a,m-b),(n-a,b)):
                hi_,lo_=max(pn,pm),min(pn,pm)
                if hi_*lo_>1 and (lo_>mmax+1 or hi_>tab[lo_]): ok=False;break
            if ok: out.append((a,b))
    return out

def exact_candidates(C,e,tab,mmax):
    """All stage-2 cuts of endpoint e, as per-part (a,b) arrays."""
    parts=C["states"][e]; M=C["meta"][e]["mass"]; capc=C["capc"]
    opts=[np.array(part_options(n,m,tab,mmax),dtype=np.int64) for n,m in parts]
    S=np.zeros(1,np.int64); X=np.zeros(1,np.int64); cols=[]
    for (n,m),o in zip(parts,opts):
        a,b=o[:,0],o[:,1]
        s=a*b; x=a*(m-b)+(n-a)*b
        S2=(S[:,None]+s[None,:]).ravel(); X2=(X[:,None]+x[None,:]).ravel()
        keep=(S2<=capc)&(X2<=capc)
        idx=np.flatnonzero(keep)
        old=idx//len(o); new=idx%len(o)
        cols=[c[old] for c in cols]+[o[new]]
        S,X=S2[idx],X2[idx]
    Cm=M-S-X
    keep=(Cm>=0)&(Cm<=capc)
    return [(c[keep,0],c[keep,1]) for c in cols], int(keep.sum())

print("training on k=8 only ...")
t0=time.time()
Xtr,ytr=training_set("k8",per_state=600)
lr=make_pipeline(StandardScaler(),LogisticRegression(max_iter=2000,class_weight="balanced")).fit(Xtr,ytr)
gb=HistGradientBoostingClassifier(max_iter=200,learning_rate=0.1,max_depth=6,
                                  l2_regularization=1.0,random_state=0).fit(Xtr,ytr)
print(f"  {Xtr.shape[0]:,} rows [{time.time()-t0:.0f}s]")

C=CORPORA["k7"]; capc=C["capc"]; kc=C["rk"]-1
mmax=max(max(max(p) for p in s) for s in C["states"].values()); tab=bound_table(kc,mmax)
forced=[e for e in four_part("k7",forced_only=True)]
print(f"\nEXACT ranks over the full stage-2 set, all {len(forced)} forced four-part k=7 states")
print("model trained on k=8 only -- different state and different level\n")
res={"logistic":[],"boosted":[]}; sizes=[]
for e in forced:
    ab,ncand=exact_candidates(C,e,tab,mmax)
    if ncand==0: continue
    _,F=features(C["states"][e],ab,capc,C["meta"][e]["mass"],mmax)
    winset={tuple(t) for t in C["wins"][e]}
    cuts=np.stack([np.stack([a,b],1) for a,b in ab],1)     # (ncand, parts, 2)
    iswin=np.array([tuple(map(tuple,cuts[i])) in winset for i in range(ncand)])
    if not iswin.any(): continue
    sizes.append(ncand)
    for tag,m in (("logistic",lr),("boosted",gb)):
        s=m.predict_proba(F)[:,1]
        res[tag].append(int((s>=s[iswin].max()).sum()))
sizes=np.array(sizes)
print(f"  candidate set size: median {int(np.median(sizes)):,}  min {sizes.min():,}  max {sizes.max():,}")
print(f"\n  {'ranker':12s} {'median rank':>12s} {'top-1':>7s} {'top-5':>7s} {'top-20':>7s} {'top-100':>8s} {'worst':>8s}")
for tag,v in res.items():
    r=np.array(v)
    print(f"  {tag:12s} {int(np.median(r)):12,} {100*(r<=1).mean():6.1f}% {100*(r<=5).mean():6.1f}%"
          f" {100*(r<=20).mean():6.1f}% {100*(r<=100).mean():7.1f}% {r.max():8,}")
print(f"\n  blind-search expectation for comparison: median {int(np.median(sizes)/2):,}")
print(f"[{time.time()-t0:.0f}s]")
