import sys, csv
from collections import defaultdict
import numpy as np
sys.path.insert(0,"/Users/fedor/radio/tools")
from sklearn.linear_model import LogisticRegression
from sklearn.ensemble import HistGradientBoostingClassifier
from sklearn.preprocessing import StandardScaler
from sklearn.pipeline import make_pipeline
from sklearn.metrics import roc_auc_score

MAXN1=defaultdict(dict)
for row in csv.DictReader(open("/Users/fedor/radio/data/pareto_sb.csv")):
    try: k,m,n1=int(row["k"]),int(row["m"]),int(row["n1"])
    except (ValueError,KeyError): continue
    if row.get("bound")=="max": MAXN1[k][m]=max(MAXN1[k].get(m,0),n1)

def deficit(parts,k):
    return max(n-(MAXN1.get(k,{}).get(m,0)) for n,m in ((max(a,b),min(a,b)) for a,b in parts))

def feat(parts,k):
    C=3.0**k; R=C**0.5
    n=np.array([max(a,b) for a,b in parts],float); m=np.array([min(a,b) for a,b in parts],float)
    area=n*m/C; nn=n/R; mm=m/R; asp=n/np.maximum(m,1)
    def pool(v):
        s=np.sort(v); return [v.sum(),v.mean(),s[-1],s[0],v.std(),s[len(s)//2]]
    f=[float(len(parts)), area.sum()]
    for v in (area,nn,mm,asp): f+=pool(v)
    d=deficit(parts,k)
    f += [d/R, float(d>0), 1.0-area.sum(), float(np.sort(area)[-1]), float(np.sort(area)[-1]/max(area.sum(),1e-9))]
    return f
NAMES=["nparts","mass"]+[f"{v}_{p}" for v in ("area","n","m","asp") for p in ("sum","mean","max","min","std","med")]+["deficit","violated","headroom","maxarea","maxshare"]

def load(k):
    X,y=[],[]
    for line in open(f"/tmp/rec/labeled_k{k}.txt"):
        w=line.split(); rc=int(w[0])
        if rc not in (0,1): continue
        v=[int(x) for x in w[1:]]; parts=[(v[i],v[i+1]) for i in range(0,len(v),2)]
        X.append(feat(parts,k)); y.append(1 if rc==0 else 0)
    return np.array(X,np.float32), np.array(y)

Xtr,ytr=load(4); Xte,yte=load(5)
print(f"train k=4: {len(ytr)} states, {ytr.mean():.1%} solvable")
print(f"test  k=5: {len(yte)} states, {yte.mean():.1%} solvable")
print("SAME sampler for both classes at both levels -- no source artifact\n")

def show(t,s): print(f"  {t:36s} AUC {roc_auc_score(yte,s):.4f}")
print("BASELINES")
show("mass/cap (lower = solvable)", -Xte[:,NAMES.index("mass")])
show("per-part Pareto deficit (sound)", -Xte[:,NAMES.index("deficit")])
print("\nLEARNED, trained on k=4 only, tested on k=5")
lr=make_pipeline(StandardScaler(),LogisticRegression(max_iter=4000)).fit(Xtr,ytr)
show("logistic regression", lr.predict_proba(Xte)[:,1])
gb=HistGradientBoostingClassifier(max_iter=250,learning_rate=0.08,max_depth=5,
                                  l2_regularization=1.0,random_state=0).fit(Xtr,ytr)
show("gradient boosting", gb.predict_proba(Xte)[:,1])
rs=np.random.default_rng(9); yp=ytr.copy(); rs.shuffle(yp)
show("PERMUTED-LABEL CONTROL", HistGradientBoostingClassifier(max_iter=250,learning_rate=0.08,
     max_depth=5,l2_regularization=1.0,random_state=0).fit(Xtr,yp).predict_proba(Xte)[:,1])
print("\nsame-level reference (5-fold within k=5, not a transfer claim)")
from sklearn.model_selection import cross_val_predict
cv=cross_val_predict(HistGradientBoostingClassifier(max_iter=250,learning_rate=0.08,max_depth=5,
    l2_regularization=1.0,random_state=0),Xte,yte,cv=5,method="predict_proba")[:,1]
print(f"  {'gradient boosting, within k=5':36s} AUC {roc_auc_score(yte,cv):.4f}")
print("\nHARD SUBSET: drop states the sound bounds already decide")
und=(Xte[:,NAMES.index("mass")]<=1.0)&(Xte[:,NAMES.index("deficit")]<=0)
print(f"  {und.sum()} of {len(yte)} undecided by info cap + per-part bound, {yte[und].mean():.1%} solvable")
if und.sum()>50 and 0<yte[und].mean()<1:
    show2=lambda t,s: print(f"  {t:36s} AUC {roc_auc_score(yte[und],s[und]):.4f}")
    show2("mass/cap", -Xte[:,NAMES.index("mass")])
    show2("gradient boosting (k=4 -> k=5)", gb.predict_proba(Xte)[:,1])
