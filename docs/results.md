# Established results

Everything on this page is backed by an entry in `data/pareto_sb.csv`, `data/pareto_sa.csv`
or a tree in `witnesses/`, each carrying its own status and source. The tables below are
**generated** from those files by `tools/check_tables.py --render`; do not edit them by
hand, and run `tools/check_tables.py` to confirm they are current.

See [problem.md](problem.md) for notation and [conjectures.md](conjectures.md) for what is
predicted but not established.

## Sa: both defectives unknown among n coins

Largest `n` such that `Sa(n)` is solvable in `k` tests.

<!-- generated:pareto_sa -->
| k | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| max n | 2 | 2 | 3 | 5 | 8 | 13 | 22 | 38 | 65 | 112 | (192) |

Parenthesised means lower bound only. Evidence per row in `data/pareto_sa.csv`.
<!-- /generated -->

`k = 1..9` are proven maximal: `out_radio_1.txt` contains both a `can solve Sa(n)` line and
a `can't solve Sa(n+1)` line for each. This was re-confirmed mechanically, not taken from
the older write-up.

`k = 10` is a **lower bound**. `Sa(192)` in 10 has two independent verified witness trees
(`witnesses/sa192_k10_a.tree`, `witnesses/sa192_k10_b.tree`). Whether `Sa(193)` is solvable
in 10 is open: the run that would have decided it was killed before reaching a verdict, and
`out_radio_1.txt` contains no `Sa(193)` line at all.

### Why Sa(193) is a small question

`Sa(n)` in `k` splits into a taken group of `n1` and the rest, requiring `Sa(n1)` solvable
in `k-1` and `Sb(n1 : n-n1)` solvable in `k-1` (see `canSolveA`, `radiobase.c:1041`). Since
`Sa(n1)` in 9 forces `n1 <= 112`, deciding `Sa(193)` in 10 reduces to **16 specific states**:

```
Sb(n1 : 193 - n1)  in 9,   for n1 = 97 .. 112
```

All sixteen sit near the diagonal at roughly 47% of the `3^9` capacity - the expensive
region, but a far smaller target than the full K=9 frontier.

## Sb: the Pareto frontier

Largest `n1` such that `Sb(n1 : m)` is solvable in `k` tests.

<!-- generated:pareto_sb -->
| m\k | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
|---|---|---|---|---|---|---|---|---|---|
| **1** | 2 | 4 | 8 | 16 | 32 | 64 | 128 | 256 | 512 |
| **2** |  | 3 | 7 | 15 | 31 | 63 | 127 | 255 | (511) |
| **3** |  |  | 5 | 12 | 27 | 58 | 121 | 248 | (503) |
| **4** |  |  | 4 | 10 | 24 | 54 | 116 | 242 | (496) |
| **5** |  |  |  | 9 | 22 | 50 | 109 | 231 | (480) |
| **6** |  |  |  | 7 | 19 | 46 | 104 | 225 | (473) |
| **7** |  |  |  |  | 17 | 42 | 97 | 214 |  |
| **8** |  |  |  |  | 15 | 38 | 91 | 206 |  |
| **9** |  |  |  |  | 14 | 36 | 87 | 198 |  |
| **10** |  |  |  |  | 12 | 33 | 82 | 189 |  |
| **11** |  |  |  |  | 11 | 31 | 77 | 182 |  |
| **12** |  |  |  |  |  | 29 | 73 | 174 |  |
| **13** |  |  |  |  |  | 27 | 69 | 168 |  |
| **14** |  |  |  |  |  | 25 | 66 | 161 |  |
| **15** |  |  |  |  |  | 24 | 63 | 155 |  |
| **16** |  |  |  |  |  | 22 | 60 | 150 |  |
| **17** |  |  |  |  |  | 21 | 58 | 144 |  |
| **18** |  |  |  |  |  | 20 | 55 | 139 |  |
| **19** |  |  |  |  |  | 19 | 53 | 135 |  |
| **20** |  |  |  |  |  |  | 51 | 130 |  |
| **21** |  |  |  |  |  |  | 49 | 126 |  |
| **22** |  |  |  |  |  |  | 47 | 122 |  |
| **23** |  |  |  |  |  |  | 45 | 118 |  |
| **24** |  |  |  |  |  |  | 43 | 115 |  |
| **25** |  |  |  |  |  |  | 41 | 111 |  |
| **26** |  |  |  |  |  |  | 40 | 108 |  |
| **27** |  |  |  |  |  |  | 38 | 105 |  |
| **28** |  |  |  |  |  |  | 37 | 102 |  |
| **29** |  |  |  |  |  |  | 36 | 100 |  |
| **30** |  |  |  |  |  |  | 35 | 97 |  |
| **31** |  |  |  |  |  |  | 34 | 94 |  |
| **32** |  |  |  |  |  |  | 33 | 92 |  |
| **33** |  |  |  |  |  |  |  | 89 |  |
| **34** |  |  |  |  |  |  |  | 87 |  |
| **35** |  |  |  |  |  |  |  | 85 |  |
| **36** |  |  |  |  |  |  |  | 83 |  |
| **37** |  |  |  |  |  |  |  | 81 |  |
| **38** |  |  |  |  |  |  |  | 79 |  |
| **39** |  |  |  |  |  |  |  | 77 |  |
| **40** |  |  |  |  |  |  |  | 76 |  |
| **41** |  |  |  |  |  |  |  | 74 |  |
| **42** |  |  |  |  |  |  |  | 72 |  |
| **43** |  |  |  |  |  |  |  | 71 |  |
| **44** |  |  |  |  |  |  |  | 69 |  |
| **45** |  |  |  |  |  |  |  | 68 |  |
| **46** |  |  |  |  |  |  |  | 66 |  |
| **47** |  |  |  |  |  |  |  | 65 |  |
| **48** |  |  |  |  |  |  |  | 64 |  |
| **49** |  |  |  |  |  |  |  | 62 |  |
| **50** |  |  |  |  |  |  |  | 61 |  |
| **51** |  |  |  |  |  |  |  | 60 |  |
| **52** |  |  |  |  |  |  |  | 59 |  |
| **53** |  |  |  |  |  |  |  | 58 |  |
| **54** |  |  |  |  |  |  |  | 57 |  |
| **55** |  |  |  |  |  |  |  | 56 |  |

Bare numbers are proven maxima. Parenthesised numbers are lower bounds: a solution exists, maximality is open. Per-cell status and evidence are in `data/pareto_sb.csv`.
<!-- /generated -->

### Provenance

`k = 1..8` is proven maximal and **artifact-backed**: for 129 of the 130 cells, both the
`can solve Sb(n:m) in k` line and the `can't solve Sb(n+1:m) in k` line were located in the
retained outputs (`out_k7.txt`, `out_radio_1.txt`, `out_k8.txt` and the smaller full-solve
files). No contradictions were found. The exception is `k=8, m=1`, where the upper bound is
the trivial dichotomy argument rather than a logged line.

The `k=8` column for `m = 10..17` was corrected in the 2026-05-12 recomputation
(`out_k8.txt`) to `189, 182, 174, 168, 161, 155, 150, 144`. Older copies of this table
circulate with the superseded values `182, 176, 170, 165, 159, 153, 148, 142`; they are
wrong. `data/pareto_sb.csv` is the only copy that should be consulted.

`k = 9` has six established entries, all **lower bounds** except `m=1`:

| m | value | how |
|---|---|---|
| 1 | 512 | `2^k`, dichotomy - proven maximal |
| 2 | 511 | lemma 3, proved |
| 3 | 503 | lemma 6, proved |
| 4 | 496 | verified tree `witnesses/canon_496_4_at9.tree` |
| 5 | 480 | verified tree `witnesses/canon_480_5_at9.tree` |
| 6 | 473 | verified tree `witnesses/canon_473_6_at9.tree` |

Those three trees are self-contained proofs. Every leaf is a singleton state whose parts
form a sub-multiset of `G_k`, so the Singleton Majorization Theorem certifies it directly -
no solver soundness is assumed anywhere. `tools/check_witness.py` re-derives all of it.

Maximality at `k=9` is open for every `m`, including these six.

## Theorems

Two results are proved in full:

- [Singleton Majorization Theorem](theorems/singleton-majorization.md) - decides singleton
  states exactly, by weak majorization against `G_k`. This is what turns a canonical tree
  into a proof.
- [Unit-Group Elimination Theorem](theorems/unit-group-elimination.md) - `1:1` parts can be
  deleted from any state without affecting solvability, subject only to the mass bound.

The special-case constructions (lemmas 1-11) are in
[theorems/special-cases.md](theorems/special-cases.md), each marked proved or not.

## Verified witnesses

| tree | claim | shape |
|---|---|---|
| `canon_248_3_at8.tree` | `Sb(248:3)` in 8 | 2 trees, 7 splits, 16 canonical leaves |
| `canon_496_4_at9.tree` | `Sb(496:4)` in 9 | 2 trees, 20 splits, 42 leaves |
| `canon_480_5_at9.tree` | `Sb(480:5)` in 9 | 9 trees, 182 splits, 373 leaves |
| `canon_473_6_at9.tree` | `Sb(473:6)` in 9 | 154 nodes, 51 splits, 103 leaves |
| `sa192_k10_a.tree` | `Sa(192)` in 10 | 154 numbered nodes |
| `sa192_k10_b.tree` | `Sa(192)` in 10 | 149 numbered nodes (tighter) |

All six pass `tools/check_witness.py`.

Note on `473:6 @9`: this settles the tree/state level only. Whether it lifts to a valid
scalable compact atomic decomposition matrix - the question the journal flags as open -
is untouched by this verification.
