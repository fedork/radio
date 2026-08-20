"""Sound filters FIRST (they cannot drop the winner), then rank what survives. Exact top-k."""
import sys, time, functools
import numpy as np
sys.path.insert(0,"/Users/fedor/radio/tools"); sys.path.insert(0,"/Users/fedor/radio/tools/ml")
from cut_ranker_data import CORPORA, bound_table, features
from cut_ranker import NAMES, four_part, training_set
from bundled_majorization import r0, normalize
from sklearn.linear_model import LogisticRegression
from sklearn.preprocessing import StandardScaler
from sklearn.pipeline import make_pipeline

R0CACHE={}
def r0ok(state,k):
    key=(state,k)
    v=R0CACHE.get(key)
    if v is None:
        v=r0(state,k); R0CACHE[key]=v
    return v

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
    parts=C["states"][e]; M=C["meta"][e]["mass"]; capc=C["capc"]
    opts=[np.array(part_options(n,m,tab,mmax),dtype=np.int64) for n,m in parts]
    S=np.zeros(1,np.int64); X=np.zeros(1,np.int64); cols=[]
    for (n,m),o in zip(parts,opts):
        a,b=o[:,0],o[:,1]; s=a*b; x=a*(m-b)+(n-a)*b
        S2=(S[:,None]+s[None,:]).ravel(); X2=(X[:,None]+x[None,:]).ravel()
        idx=np.flatnonzero((S2<=capc)&(X2<=capc))
        cols=[c[idx//len(o)] for c in cols]+[o[idx%len(o)]]
        S,X=S2[idx],X2[idx]
    Cm=M-S-X; keep=(Cm>=0)&(Cm<=capc)
    return [(c[keep,0],c[keep,1]) for c in cols]

print("training on k=8 ..."); t0=time.time()
Xtr,ytr=training_set("k8",per_state=600)
lr=make_pipeline(StandardScaler(),LogisticRegression(max_iter=2000,class_weight="balanced")).fit(Xtr,ytr)
print(f"  [{time.time()-t0:.0f}s]")

C=CORPORA["k7"]; capc=C["capc"]; kc=C["rk"]-1
mmax=max(max(max(p) for p in s) for s in C["states"].values()); tab=bound_table(kc,mmax)
forced=four_part("k7",forced_only=True)
sub=forced[::max(1,len(forced)//60)][:60]
before=[]; after=[]; rank_all=[]; rank_r0=[]; t0=time.time()
for e in sub:
    parts=C["states"][e]
    ab=exact_candidates(C,e,tab,mmax)
    n=len(ab[0][0])
    if not n: continue
    winset={tuple(t) for t in C["wins"][e]}
    cuts=np.stack([np.stack([a,b],1) for a,b in ab],1)
    iswin=np.array([tuple(map(tuple,cuts[i])) in winset for i in range(n)])
    if not iswin.any(): continue
    # sound R_0 on all three children
    ok=np.zeros(n,bool)
    for i in range(n):
        sel=[];comp=[];mix=[]
        for j,(pn,pm) in enumerate(parts):
            a,b=int(cuts[i,j,0]),int(cuts[i,j,1])
            sel.append((a,b)); comp.append((pn-a,pm-b)); mix += [(a,pm-b),(pn-a,b)]
        ok[i]=(r0ok(normalize(sel),kc) and r0ok(normalize(mix),kc) and r0ok(normalize(comp),kc))
    before.append(n); after.append(int(ok.sum()))
    _,F=features(parts,ab,capc,C["meta"][e]["mass"],mmax)
    s=lr.predict_proba(F)[:,1]
    rank_all.append(int((s>=s[iswin].max()).sum()))
    if ok[iswin].any():
        s2=np.where(ok,s,-np.inf)
        rank_r0.append(int((s2>=s2[iswin&ok].max()).sum()))
b,a=np.array(before),np.array(after); ra,rr=np.array(rank_all),np.array(rank_r0)
print(f"\n  {len(b)} forced k=7 states  [{time.time()-t0:.0f}s]")
print(f"  candidates before R_0 : median {int(np.median(b)):,}")
print(f"  candidates after  R_0 : median {int(np.median(a)):,}   (sound filter, {np.median(b)/max(np.median(a),1):.0f}x)")
print(f"  winner survived R_0   : {len(rr)}/{len(b)} states")
print(f"\n  {'pipeline':26s} {'median rank':>12s} {'top-5':>7s} {'top-20':>7s} {'top-100':>8s} {'worst':>8s}")
for tag,r in (("rank only",ra),("R_0 then rank",rr)):
    print(f"  {tag:26s} {int(np.median(r)):12,} {100*(r<=5).mean():6.1f}% {100*(r<=20).mean():6.1f}%"
          f" {100*(r<=100).mean():7.1f}% {r.max():8,}")
