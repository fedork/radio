# The Sa(193) certificate of record

Adopted 2026-08-19. The artifact you hand someone who wants to re-check that `Sa(193)` is
unsolvable in **10** tests is the **trimmed eight-level chain**, released as
[`sa193-certificate-2026-08-19`](https://github.com/fedork/radio-data/releases/tag/sa193-certificate-2026-08-19).

## What it is

Eight `radio-negative-level-certificate-v2` files, `sa193-k2.cert.zst` through `sa193-k9.cert.zst`.
Each level carries a part dictionary, its support, checked split hints and the claims to audit.

| level | claims | support | claims are proved using |
|---|---|---|---|
| 9 | **16** — the `Sb(n1:193-n1)@9` roots, n1 = 97..112 | 2,151 | level 8 |
| 8 | 2,151 | 2,508,278 | level 7 |
| 7 | 2,508,278 | 230,725 | level 6 |
| 6 | 230,725 | 80,634 | level 5 |
| 5 | 80,634 | 24,635 | level 4 |
| 4 | 24,635 | 127 | level 3 |
| 3 | 127 | 2 | level 2 |
| 2 | 2 | 0 | refuted outright |

2,846,568 claims, 15,642,637 compressed bytes.

## Why the trimmed chain rather than the complete corpus

The complete chain ships every normalized run9 fact at each level — 3,126,190 claims. The trimmed
chain ships only the **transitive citation set**: at each level, exactly the facts some claim one
level up actually cited. Because a level-k audit's cited-support count is by construction the
level-(k-1) claim count, the result is one nested chain with nothing carried that nothing uses.

At matched compression the trimmed chain is **8.82% smaller** (17,155,540 to 15,642,637 bytes),
8.23% smaller raw, and carries 8.94% fewer claims. It is not faster to verify — see below — the
reason to prefer it is that it is minimal and has no unused content.

## How to check it

Two independent halves. Both are needed; neither alone is sufficient.

**Structure**, no solver involved:

```
tools/check_level_chain.py --expect-top-sum 193 sa193-k*.cert
```

This confirms each level's declared counts match its records, that level k's support set is exactly
level (k-1)'s claim set as resolved states, that level 2's support is empty so the induction
terminates, and that the top level is precisely the 16 single-part states summing to 193. A chain
that passes has no dangling reference: every fact used in a refutation is itself a claim proved one
level lower, down to a level proved outright.

**Semantics**, with the frozen refuter, one level at a time:

```
REFUTE_MIN_K=<k> REFUTE_MAX_K=<k> ./run9_refute sa193-k<k>.cert
```

Each level must print `TOTAL verified <claims>, gaps 0`. A missing refutation is reported as a gap,
so an inadequate certificate fails loudly rather than passing quietly.

**Semantics, independently** — this is the stronger check, since the refuter shares the solver
core and `tools/cleanroom` shares nothing:

```
cd tools/cleanroom && cargo build --release
./target/release/radio_cleanroom audit --threads N --progress 300 sa193-k*.cert
```

It must print `TOTAL verified 2846568, gaps 0`. `tools/cleanroom_ec2_launch.sh` runs both halves
on one dedicated host and does exactly this; the whole chain is about 1h50m on 32 vCPU.

The release contains the verification evidence for exactly these files: run `20260819T020000Z`
verified all 2,846,568 claims with zero gaps, and its per-level logs, totals and provenance checks
are included.

## What it does and does not establish

- The **proof source is still the proof-safe cold `run9`** execution, which exhaustively rejected all
  sixteen roots in one session. This certificate is a compact, checkable *replay* artifact derived
  from that run's output. Adopting it changes which file you hand out, not the provenance of the
  result.
- The refuter shares the solver core (split enumeration, dominance), so a zero-gap refuter replay
  is solver-core validation, **not** an independent second implementation.
- **The chain is now also verified independently** (2026-09-01). `tools/cleanroom` shares no code
  with the solver and recomputes every rule from `docs/problem.md` and `docs/theorems/`; one
  1h50m run on a `c8a.8xlarge` closes all eight levels with zero gaps — all 2,846,568 claims —
  having first run `tools/check_level_chain.py` over the same eight files on the same host.
  Record and hashes: [../evidence/cleanroom_verifier_2026-09-01.txt](../evidence/cleanroom_verifier_2026-09-01.txt).
  Verified twice, at two commits with materially different index construction, reaching an
  identical candidate-cell count to the digit (3,252,096,103,282) and an identical structural
  check hash. The two implementations also agree on citations specifically: 1,179,555,891,520
  against the engine's documented 1.18 trillion k=7 citation hits.
  It agrees with the production engine on total work to 0.46% over 3.2 trillion candidate cells, so
  it is exploring the same tree rather than a cheaper approximation of it.
- The trimmed chain's *derivation* used the coloring run, but its *validity* does not depend on the
  coloring being correct: coloring only proposed the subset, and the zero-gap replay plus the
  structural check are what establish it.
- **The trimmed chain cannot answer questions outside its own claim set.** It is not the corpus.
  For anything else — a different query, a new frontier question, re-deriving the level decomposition
  — use the complete normalized `run9.cert` (SHA-256
  `3ad5877a2ffa3bcf04c3403a147ae075e406b4313cce83eb0761fdd563725116`, archived in
  `sa193-frozen-refute-2026-08-18`) or the complete level chain in `run9-level-replay-2026-08-18`.
  Both are retained deliberately as the reference form.

## Trimming does not make verification faster

Recorded so it is not re-attempted. Measured on one host with one binary:

| input | claims | k=7 support | CPU s |
|---|---|---|---|
| complete | 3,126,190 | 388,317 | 211,335.569 |
| selected claims only | 2,846,568 | 388,317 | 202,592.331 |
| trimmed (this certificate) | 2,846,568 | 230,725 | 201,982.710 |

Trimming the support 40.6% bought **0.30%**, because 156,927 of the complete 388,317 k=7 facts were
already discarded as redundant during Pareto-front construction: only 231,390 ever entered the
structure, against the 230,725 cited here. The true live reduction is 665 facts. Verification cost is
prefix enumeration — 3.22 trillion accepted prefixes against 1.18 trillion cache citations — not fact
lookup, so certificate size is not the lever. The full measurement is in the 2026-08-19 journal entry.

The smaller certificate is worth having for being minimal. It is not worth having for speed.
