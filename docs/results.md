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
| k | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| max n | 2 | 2 | 3 | 5 | 8 | 13 | 22 | 38 | 65 | 112 | 192 | (197) |

Parenthesised means lower bound only. Evidence per row in `data/pareto_sa.csv`.
<!-- /generated -->

`k = 1..10` are proven maximal. For `k=1..9`, `out_radio_1.txt` contains both a
`can solve Sa(n)` line and a `can't solve Sa(n+1)` line for each, re-confirmed mechanically
rather than taken from the older write-up. The `k=10` boundary combines an independently
verified witness at 192 with the proof-safe cold rejection of 193 described below.

Achievability has verified witness trees at every k from 7 up: `witnesses/sa38_k7.tree`,
`sa65_k8_{a,b,c}`, `sa112_k9_{a,b,c}`, `sa192_k10_{a,b}`. These are proofs independent of the
solver, so `Sa(192)` in 10 is not in doubt.

### Sa(10) = 192, proven maximal

`Sa(n)` in `k` splits into a taken group of `n1` and the rest, requiring `Sa(n1)` solvable
in `k-1` and `Sb(n1 : n-n1)` solvable in `k-1` (see `canSolveA` in `radiobase.c`). Since
`Sa(n1)` in 9 forces `n1 <= 112`, `Sa(193)` in 10 comes down to **16 states**:

```
Sb(n1 : 193 - n1)  in 9,   for n1 = 97 .. 112
```

Cold AWS `run9` refuted all sixteen in one uninterrupted session from an empty cache and then
printed `result Sa(193) in 10 = UNSOLVABLE (419353.1 s)`. Its positive `Sa(192)` control
passed first in 479.2 CPU seconds. The run used the contraction-safe build at commit
`e7fa747264476461a234bf78e49762ee77ad8d8d`, carried complete embedded provenance, completed
in 419849 wall seconds with 1.32 GB peak RSS, and produced no contradictory audited verdict.

The raw proof log is archived as
`sa193-cold-2026-08-16:run9_out_sa193.txt`; the sixteen root lines and exact hashes are
committed in
[`evidence/sa193_unsolvable_in_10.txt`](../evidence/sa193_unsolvable_in_10.txt). Together
with the checked `Sa(192)` witness tree, this proves the boundary.

A second cold derivation reproduced the verdict on 2026-09-04: `run10`, commit `9e9e25a` with
necessity-only cache semantics, printed `result Sa(193) in 10 = UNSOLVABLE  (301127.6 s)` with all
sixteen roots refuted, its `Sa(192)` control passed, complete provenance and zero audit
contradictions, for 28.19% less CPU than run9. It is archived as
`sa193-cold-run10-2026-09-04:run10_out_sa193.txt`. This is a recomputation by a different build of
the same engine, not an independent implementation, so it does not extend the certificate's
independence; it does show current main reaches the same boundary from cold.

The 2023 run reached the same answer after roughly 47 days, but its build produced 37 known
false negatives. That historical evidence remains useful for cost comparison only and is
superseded as the source of the claim.

## Sb: the Pareto frontier

Largest `n1` such that `Sb(n1 : m)` is solvable in `k` tests.

<!-- generated:pareto_sb -->
| m\k | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
|---|---|---|---|---|---|---|---|---|---|---|
| **1** | 2 | 4 | 8 | 16 | 32 | 64 | 128 | 256 | 512 |  |
| **2** |  | 3 | 7 | 15 | 31 | 63 | 127 | 255 | 511 |  |
| **3** |  |  | 5 | 12 | 27 | 58 | 121 | 248 | 503 |  |
| **4** |  |  | 4 | 10 | 24 | 54 | 116 | 242 | 496 |  |
| **5** |  |  |  | 9 | 22 | 50 | 109 | 231 | 481 | 985 |
| **6** |  |  |  | 7 | 19 | 46 | 104 | 225 | 473 | ≤973 |
| **7** |  |  |  |  | 17 | 42 | 97 | 214 |  |  |
| **8** |  |  |  |  | 15 | 38 | 91 | 206 |  |  |
| **9** |  |  |  |  | 14 | 36 | 87 | 198 |  |  |
| **10** |  |  |  |  | 12 | 33 | 82 | 189 |  |  |
| **11** |  |  |  |  | 11 | 31 | 77 | 182 |  |  |
| **12** |  |  |  |  |  | 29 | 73 | 174 |  |  |
| **13** |  |  |  |  |  | 27 | 69 | 168 |  |  |
| **14** |  |  |  |  |  | 25 | 66 | 161 |  |  |
| **15** |  |  |  |  |  | 24 | 63 | 155 |  |  |
| **16** |  |  |  |  |  | 22 | 60 | 150 |  |  |
| **17** |  |  |  |  |  | 21 | 58 | 144 |  |  |
| **18** |  |  |  |  |  | 20 | 55 | 139 |  |  |
| **19** |  |  |  |  |  | 19 | 53 | 135 |  |  |
| **20** |  |  |  |  |  |  | 51 | 130 |  |  |
| **21** |  |  |  |  |  |  | 49 | 126 |  |  |
| **22** |  |  |  |  |  |  | 47 | 122 |  |  |
| **23** |  |  |  |  |  |  | 45 | 118 |  |  |
| **24** |  |  |  |  |  |  | 43 | 115 |  |  |
| **25** |  |  |  |  |  |  | 41 | 111 |  |  |
| **26** |  |  |  |  |  |  | 40 | 108 |  |  |
| **27** |  |  |  |  |  |  | 38 | 105 |  |  |
| **28** |  |  |  |  |  |  | 37 | 102 |  |  |
| **29** |  |  |  |  |  |  | 36 | 100 |  |  |
| **30** |  |  |  |  |  |  | 35 | 97 |  |  |
| **31** |  |  |  |  |  |  | 34 | 94 |  |  |
| **32** |  |  |  |  |  |  | 33 | 92 |  |  |
| **33** |  |  |  |  |  |  |  | 89 |  |  |
| **34** |  |  |  |  |  |  |  | 87 |  |  |
| **35** |  |  |  |  |  |  |  | 85 |  |  |
| **36** |  |  |  |  |  |  |  | 83 |  |  |
| **37** |  |  |  |  |  |  |  | 81 |  |  |
| **38** |  |  |  |  |  |  |  | 79 |  |  |
| **39** |  |  |  |  |  |  |  | 77 |  |  |
| **40** |  |  |  |  |  |  |  | 76 |  |  |
| **41** |  |  |  |  |  |  |  | 74 |  |  |
| **42** |  |  |  |  |  |  |  | 72 |  |  |
| **43** |  |  |  |  |  |  |  | 71 |  |  |
| **44** |  |  |  |  |  |  |  | 69 |  |  |
| **45** |  |  |  |  |  |  |  | 68 |  |  |
| **46** |  |  |  |  |  |  |  | 66 |  |  |
| **47** |  |  |  |  |  |  |  | 65 |  |  |
| **48** |  |  |  |  |  |  |  | 64 |  |  |
| **49** |  |  |  |  |  |  |  | 62 |  |  |
| **50** |  |  |  |  |  |  |  | 61 |  |  |
| **51** |  |  |  |  |  |  |  | 60 |  |  |
| **52** |  |  |  |  |  |  |  | 59 |  |  |
| **53** |  |  |  |  |  |  |  | 58 |  |  |
| **54** |  |  |  |  |  |  |  | 57 |  |  |
| **55** |  |  |  |  |  |  |  | 56 |  |  |
| **65** |  |  |  |  |  |  |  |  | ≥112 |  |
| **66** |  |  |  |  |  |  |  |  | ≥112 |  |
| **67** |  |  |  |  |  |  |  |  | ≥112 |  |
| **68** |  |  |  |  |  |  |  |  | ≥112 |  |
| **69** |  |  |  |  |  |  |  |  | ≥112 |  |
| **70** |  |  |  |  |  |  |  |  | ≥112 |  |
| **71** |  |  |  |  |  |  |  |  | ≥112 |  |
| **72** |  |  |  |  |  |  |  |  | ≥112 |  |
| **73** |  |  |  |  |  |  |  |  | ≥112 |  |
| **74** |  |  |  |  |  |  |  |  | ≥112 |  |
| **75** |  |  |  |  |  |  |  |  | ≥112 |  |
| **76** |  |  |  |  |  |  |  |  | ≥112 |  |
| **77** |  |  |  |  |  |  |  |  | ≥112 |  |
| **78** |  |  |  |  |  |  |  |  | ≥112 |  |
| **79** |  |  |  |  |  |  |  |  | ≥112 |  |
| **80** |  |  |  |  |  |  |  |  | ≥112 |  |
| **81** |  |  |  |  |  |  |  |  | 82–111 |  |
| **82** |  |  |  |  |  |  |  |  | 83–110 |  |
| **83** |  |  |  |  |  |  |  |  | 84–109 |  |
| **84** |  |  |  |  |  |  |  |  | 85–108 |  |
| **85** |  |  |  |  |  |  |  |  | 86–107 |  |
| **86** |  |  |  |  |  |  |  |  | 87–106 |  |
| **87** |  |  |  |  |  |  |  |  | 88–105 |  |
| **88** |  |  |  |  |  |  |  |  | 89–104 |  |
| **89** |  |  |  |  |  |  |  |  | 90–103 |  |
| **90** |  |  |  |  |  |  |  |  | 91–102 |  |
| **91** |  |  |  |  |  |  |  |  | 92–101 |  |
| **92** |  |  |  |  |  |  |  |  | 93–100 |  |
| **93** |  |  |  |  |  |  |  |  | 94–99 |  |
| **94** |  |  |  |  |  |  |  |  | 95–98 |  |
| **95** |  |  |  |  |  |  |  |  | ≤97 |  |
| **96** |  |  |  |  |  |  |  |  | ≤96 |  |

A bare number is a proven maximum. `≥n` is a lower bound (a solution exists, maximality open), `≤n` an upper bound (exhaustively refuted above), `a–b` a two-sided bracket. Per-cell status and evidence are in `data/pareto_sb.csv`.
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

The K=9 column has three kinds of entry. For `m=1..5`, published theorems give exact
maxima; the new `m=5` boundary was also replayed independently with an unsupported
majorized-terminal file at 481 and an exact rejection at 482.  At `m=6`, a retained exact replay
rejects 474 and a canonical tree proves 473. Values at `m=65..80` remain legacy lower bounds recovered from the
old cache. At `m=81..94`, those legacy lower bounds are paired with proof-safe run9 upper
bounds; `m=95,96` have proof-safe upper bounds only. Thus all sixteen ceilings used in the
`Sa(193)` proof are established, but none of these rows is an exact K=9 maximum. The whole
band `m=7..64` is blank.

Publication priority is finer-grained than proof status. Aigner 1986 already gives the exact
`m=2,3,4` formulas and the isolated exact cell `n(4,6)=7`; Li--Wu--Triesch 2018 gives exact
`m=5`. Zhang--Berger--Massey 1987 and Gargano et al. 1992 also publish individual finite
constructions at several local frontier endpoints, including exact costs for `Sb(21:17)@6` and
`Sb(32:32)@7`, but not the neighboring negative verdicts needed for the corresponding fixed-`m`
maxima. See [the cell-honest prior-art audit](../evidence/publication_prior_art_2026-09-02.md).

At `k=10`, Li--Wu--Triesch's theorem gives the exact cell **`n(10,5)=985`**.  Independently,
the exhaustive rejection of `Sb(974:6)` proves **`n(10,6)<=973`**.  The checked 115-node file
`witnesses/majorized_973_6_at10.tree` reaches 77 singleton terminals; 71 already fit in distinct
`G_k` slots, but six require arbitrary weak majorization.  Because sufficiency of that condition is
false in general, it is not a 973 lower proof; those six leaves need independent strategies.  Full provenance for the unconditional upper bound is in
`evidence/sb_m6_k10_frontier.txt`.

The generated grid above shows six exact K=9 maxima at `m=1..6`.  Their per-cell theorem,
witness and exhaustive-replay sources live in `data/pareto_sb.csv`; no second hand-maintained
copy is kept here.

The canonical and distinct-slot trees are self-contained proofs: their leaves embed into the
explicit solvable state `G_k`.  The 481 and 973 files instead use arbitrary weak-majorization
terminals, which are not certificates because the universal converse is false.  The exact 481 value remains established by
Li--Wu--Triesch's published theorem.  `tools/check_witness.py` now reports the distinction.

Maximality at `k=9` is open in the unfilled band, not for `m=1..6`.

## Theorems

Three foundational facts are central here:

- [Singleton Majorization Necessity and the `K=6` counterexample](theorems/singleton-majorization.md) -
  proves the obstruction, refutes sufficiency with an exact-support state, gives the exhaustive
  no-first-cut transfer-distance minimum, and explains why canonical distinct-slot terminals are
  unconditional while arbitrary majorized terminals are not certificates.
- [Unit-Group Elimination Theorem](theorems/unit-group-elimination.md) - `1:1` parts can be
  deleted from any state without affecting solvability, subject only to the mass bound.
- [Subgraph Monotonicity Theorem](theorems/subgraph-monotonicity.md) - deleting candidate
  edges cannot make a solvable graph harder; this supplies the upward closure of every exact
  frontier rejection.

The special-case constructions (lemmas 1-11) are in
[theorems/special-cases.md](theorems/special-cases.md), each marked proved or not.

## Verified witnesses

| tree | claim | shape |
|---|---|---|
| `canon_248_3_at8.tree` | `Sb(248:3)` in 8 | 2 trees, 7 splits, 16 canonical leaves |
| `canon_496_4_at9.tree` | `Sb(496:4)` in 9 | 2 trees, 20 splits, 42 leaves |
| `canon_480_5_at9.tree` | `Sb(480:5)` in 9 | 9 trees, 182 splits, 373 leaves |
| `canon_480_5_at9_twosided.tree` | `Sb(480:5)` in 9 with two-sided roots | 5 trees |
| `majorized_481_5_at9.tree` | unsupported relaxed `Sb(481:5)` diagnostic | 61 nodes, 20 splits, 41 singleton leaves (3 nonembedded) |
| `canon_473_6_at9.tree` | `Sb(473:6)` in 9 | 154 nodes, 51 splits, 103 leaves |
| `canon_473_6_at9_twosided.tree` | `Sb(473:6)` in 9 with two-sided roots | 1 tree |
| `majorized_973_6_at10.tree` | unsupported relaxed `Sb(973:6)` diagnostic | 115 nodes, 38 splits, 77 singleton leaves (6 nonembedded) |
| `sa38_k7.tree` | `Sa(38)` in 7 | 24 numbered nodes |
| `sa65_k8_{a,b,c}.tree` | `Sa(65)` in 8 | 46 / 40 / 35 nodes |
| `sa112_k9_{a,b,c}.tree` | `Sa(112)` in 9 | 78 / 72 / 74 nodes |
| `sa192_k10_a.tree` | `Sa(192)` in 10 | 154 numbered nodes |
| `sa192_k10_b.tree` | `Sa(192)` in 10 | 149 numbered nodes (tighter) |

All files pass structural checking.  The checker marks the two `majorized_*` files unsupported;
the other listed trees are unconditional. The seven `Sa(38)` / `Sa(65)` / `Sa(112)` trees were
recovered from `radio.zip` on 2026-08-02.

## Exhaustively enumerated multi-part states

`data/exhaustive_multipart.csv` records 16 complete `all_solutions` runs, where every
top-level split of a multi-part state was enumerated. These are not Pareto cells, but they
are hard facts and they are the input the decomposition-matrix work consumes.

The solvable ones sit on a knife edge - `Sb(16:12,17:10,29:5,21:6)` in 6 has exactly **2**
working splits out of 1,212,971,760, and `Sb(8:7,8:5,12:3,17:2,8:4,13:2,6:2)` in 5 has 40 out
of 433,315,733,760. One is a proven negative: `Sb(111:3,115:2,121:1)` is **not** solvable in
7, with all 38,040,576 splits refuted, against `Sb(110:3,115:2,121:1)` which is.

Note on `473:6 @9`: this settles the tree/state level only. Whether it lifts to a valid
scalable compact atomic decomposition matrix - the question the journal flags as open -
is untouched by this verification.
