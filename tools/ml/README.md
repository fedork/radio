# Learned cut ranking — experiment scripts

**These are the only scripts in the repo with third-party dependencies.** Everything under
`tools/` proper stays dependency-free, and the checks (`check_tables.py`, `check_witness.py`,
`check_docs.py`) must never import from here. Set up once:

```
.venv/bin/pip install numpy scikit-learn        # .venv is gitignored
```

Then, with the two census logs pulled from the artifact store:

```
tools/artifacts.sh pull pareto-census-k8-2026-08-19 .artifacts/pareto-census-k8
tools/artifacts.sh pull pareto-census-k7-2026-08-13 .artifacts/pareto-census-k7
CENSUS_CACHE=/tmp/census_cache.pkl .venv/bin/python tools/ml/cut_ranker.py
```

`cut_ranker_data.py` builds the dataset (features, sampling, the sound per-part Pareto filter);
`cut_ranker.py` trains and evaluates. Findings and the numbers they produced are in
[`../../evidence/learned_cut_ranker_2026-08-20.txt`](../../evidence/learned_cut_ranker_2026-08-20.txt).

## The three things that make this measurement honest

1. **Group by state.** Cuts of one state never straddle train and test. The headline result is
   stronger still — trained on the k=7 corpus only and tested on k=8, so neither the state nor the
   level was seen.
2. **State the denominator.** Selectivity is meaningless without one. Candidates here are
   *stage 2*: information cap on all three children, plus every part of every child inside the
   proven per-part bound from `data/pareto_sb.csv`. That sound filter alone is ~8-11x, and it is
   already applied before anything is measured, so the learned gain is on top of it.
3. **Permuted-label control.** Shuffling labels and rerunning the identical pipeline must collapse
   to ~1x. It gives 1.6x against the real model's 428x. Without this check a result this large
   should not be believed.

A fourth guard worth keeping in mind: the ranking metric is censored by how many negatives you
sample. Quote a floor, not a number, whenever the model puts zero sampled negatives above the
winner.
