# Training data for the `BY_ML` split-ordering model

`labeled_k{4,5,6,7}.txt` — 300 states each, one per line: `<code> <n1> <m1> <n2> <m2> ...`
(a variable number of `(n,m)` pairs), where `code` is `0`=SOLVABLE, `1`=UNSOLVABLE, `2`=MAYBE.
`2` (MAYBE) rows are dropped before training — never recorded as either class.

**Provenance.** States are a matched synthetic sample (mass band bisected per level), labeled by
querying this repo's own trusted solver via the warm oracle (`tools/oracle_client.py` against
`radio_oracle.c`, which wraps the unmodified `canSolveB`) — not a third-party or hand-derived
source. Generated 2026-08-20/21 in the session documented in
[`../../evidence/recursive_value_2026-08-20.txt`](../../evidence/recursive_value_2026-08-20.txt)
and [`../../docs/ml-guided-search.md`](../../docs/ml-guided-search.md); archived here 2026-08-24
because the model trained from it is now embedded in `radiobase.c` (`ml_order_model.h`, generated
by `tools/ml/export_ordering_model.py`) and needs a committed, reproducible source rather than a
`/tmp` artifact.

Regenerate the embedded model from this data with:
```
tools/ml/export_ordering_model.py
```
