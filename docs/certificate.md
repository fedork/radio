# A negative certificate for `Sa(193)`

Design, 2026-08-04. Nothing here is built yet. The point of writing it before the run is that
the format determines what the run must emit, and reconstructing a certificate afterwards from
900 MB of logs is the failure this project already lived through once.

## What is being certified

`Sa(n)` in `k` splits into a taken group of `n1` and the rest, needing `Sa(n1)` in `k-1` and
`Sb(n1 : n-n1)` in `k-1` (`canSolveA`). `Sa(112)` is the k=9 maximum, so `n1 <= 112`, and

> `Sa(193)` is unsolvable in 10 **iff** all sixteen `Sb(n1 : 193-n1)` are unsolvable in 9,
> for `n1 = 97..112`.

So the object is sixteen refutations at k=9. Everything below is about making those checkable
without trusting the solver that produced them.

## Why this is not a refutation tree

The naive certificate — for every split, a pointer to the failing child, recursively — is
astronomical. `Sb(112:80)` alone put 130 billion prefix tuples into a *single* k=5 node.

What makes it tractable is that the refutation is a **DAG, not a tree**: the same states recur,
which is exactly what the solver's memo exploits. So the certificate is the *set of distinct
refuted facts*, each with a reason, and the verifier checks that the set is closed.

The DAG is **stratified by `k`** — a fact at `k` only refers to children at `k-1` — so there is
no circularity, no fixpoint, and no topological sort. Verify level by level, ascending.

## Format: reuse what already exists

A fact is one line in the format `parse_out.sh` already produces and `parse_file` already reads:

```
# radio-cert v1 build=<sha> state=112_81 generated=<iso8601>
- b 95 69 t 6555 164 8
```

`-` negative, `b` = Sb, then the parts, `t`, mass, coins, `k`. **Only negatives are needed** — a
negative certificate never consults a positive fact. That is most of the data anyway: 304,105 of
315,184 verdicts in the k=9 ladder are negative.

Reusing the format is deliberate. It is already emitted, already parsed, already exercised by
every cache warm-start, and it is human-inspectable. A new packed binary format would be smaller
and would need its own writer, reader, and bugs. `parse_file` skips `#` lines as of `69ae856`,
so the provenance header costs nothing.

### Size, from measured quantities

| | |
|---|---|
| 2023 `Sb(112:81)` reached ~90 GB of trie | measured |
| trie cost at `MAX_N=193` | **2.26 KB per logical insert** (measured 2026-08-03) |
| ⇒ facts for that one state | **~43 M** |
| at ~25 bytes/line | ~1.1 GB raw |
| union of all sixteen (they share heavily — same 193 coins, which is why the 2023 tail collapsed 1300x on a warm cache) | order 2-4 GB raw |
| no breadcrumb multiplier — the log is closed as it stands (see below) | x1 |
| sorted small integers, `zstd` at ~15-25% | **order 0.3-1 GB shipped** |

For comparison, the 2023 run's raw log for a *single* state was 905 MB, and ~18 GB of that era's
`out*` was deliberately not archived (see [data.md](data.md)). So this is the same order as
artefacts the project already handles, but unlike them it is checkable.

## The reasons, and what checking each costs

| reason | rule | cost |
|---|---|---|
| `COUNT` | mass > `3^k` | O(parts) |
| `MAJ` | all parts singletons and majorization against `G_k` fails | O(parts) |
| `DOM s'` | `s' <= s` and `s'` refuted at the same `k` | O(parts), given a witness |
| `SPLITS` | every split of `s` has a child refuted at `k-1` | the expensive one |

The reason need not be stored — the verifier derives it. `COUNT` and `MAJ` are tested first
because they are free; `SPLITS` is the fallback.

### `SPLITS` is already 90% written

`radio_allsol.c` enumerates every split of a state with prefix pruning on the counting bound,
which mass conservation licenses. It took a 7-part state from 401 billion tuples to 231 million
in 8 minutes. Replace "are all three children solvable" with "is some child in the refuted set"
and it is a `SPLITS` checker. Per-fact independence makes the whole verification embarrassingly
parallel, unlike the search that produced it.

**Verification is not cheaper than the proof.** An earlier draft claimed it was. Dropping search,
`MAYBE` and re-passes removes constant factors, not the enumeration: checking one fact still
enumerates its split space. Measured on the `Sa(113)` k=9 ladder, the 216,580 facts at k=4 take
91 s single-threaded against 1,521 s for the whole solve, and the k=4 level alone is 1.4 billion
recursion nodes.

What verification buys is not speed. It is a trust base of three theorems and ~700 lines
(`radio_verify.c`) instead of ~1,600 lines of orderings, deadlines, passes and cache; per-fact
independence, so it parallelises arbitrarily where the search does not; per-level independence, so
resident memory is one level rather than the certificate; and spot-checkability, so any single
disputed fact can be re-derived alone.

## Closure depends on how the run was conducted

An earlier draft called non-closure "the one real problem" and designed inference breadcrumbs
around it. A later draft said the problem did not exist. Both were too general. The measured
position:

- **A cold single-session run produces a closed log.** The `Sa(113)` k=9 ladder verifies with 0
  unverified at every level checked. Most of what looked like a gap was the verifier missing
  Singleton Majorization on the singleton **sub-multiset** — `canSolveB` refutes that way and
  returns FALSE without printing, so those states are legitimately absent from every log.
- **A resumed run does not.** The 2023 `Sa(193)` run ran for months, warm-started repeatedly, and
  ~18 GB of its earlier output was deliberately not archived. Its cache therefore knew facts whose
  proofs are gone. Measured: ~5% of its k=5 facts cite a k=4 child that was never logged, and each
  of its sixteen k=9 facts cites a k=8 child that was never logged.

**Constraint on the re-run: keep every session's raw output, or start cold and never resume.** This
is the one operational requirement the format cannot fix after the fact.

### The fix is derivation, not breadcrumbs

When nothing in the fact set refutes a state, the checker runs the same `SPLITS` check on that
state one level down, memoised. Deriving is proving: a derived fact is checked by exactly the rules
a shipped one would be, so this does not touch the trust base — it shrinks the artifact. At k<=4 it
costs milliseconds and it closed the 2023 k=5 gap completely (4,859 of 4,859 sampled facts verified
after deriving 2,836 missing children).

So the certificate is **the facts that are expensive to re-derive**, and the right extent is the
sub-DAG *reachable from the sixteen roots*, not a whole log. The x6.6 breadcrumb multiplier is
withdrawn: it measured cache queries, not facts.

## The sixteen are already 31/32 checked

`sa193-2023` contains all sixteen `can't solve Sb(n1:193-n1) in 9`. Verified against itself plus
the 2026 `out_k8.txt`, **each fails on exactly one split** — 32 recursion nodes across all sixteen.
Every other split of every root is discharged. The survivor is always the near-balanced split, and
its two single-part children are both solvable by the proven Pareto table, so the refutation must
be the two-part mixed child:

| root | surviving split | the one k=8 fact needed |
|---|---|---|
| `Sb(112:81)` | `(38,40)` | `Sb(74:40, 41:38)` |
| `Sb(111:82)` | `(39,42)` | `Sb(72:42, 40:39)` |
| ... | ... | (one per root, `n1 = 97..112`) |

Nothing among the 1,879 logged 2023 k=8 facts dominates `Sb(74:40, 41:38)`. So the remaining work
at the top of the DAG is **sixteen two-part k=8 refutations** — finite, independent, and
parallel — not a 47-day re-run. Whether the k=6 and k=7 levels are equally close is the open
measurement; k=7 carries 3.1 M facts at P=4 with ~558 options per part.

## Trust base

The verifier depends on exactly four things:

- **Singleton Majorization Theorem** — [theorems/singleton-majorization.md](theorems/singleton-majorization.md)
- **Unit-Group Elimination Theorem** — [theorems/unit-group-elimination.md](theorems/unit-group-elimination.md)
- **Subgraph Monotonicity Theorem** — [theorems/subgraph-monotonicity.md](theorems/subgraph-monotonicity.md)
- the split semantics of [problem.md](problem.md), i.e. that a test on `(n:m)` taking `(a,b)`
  yields children `(a:b)`, `(n-a:m-b)` and `{(a:m-b), (n-a:b)}`

All three theorems are proved. It depends on **nothing** about the solver — not the split
orderings, not `FAST`, not deadlines, not the cache, not the pass structure. That is what makes
a re-run worth doing instead of re-asserting the 2023 answer, and it is what the 2023 corpus can
never have: 37 of its negatives are provably false and there is no syntactic marker separating
them.

### Why the logged negatives are usable at all

Two properties, both established 2026-08-03/04:

- **A printed `can't solve` is exhaustive.** It is emitted only when `!skipped_some`, so a
  logged negative is a complete refutation even with deadlines enabled. `MAYBE` states print
  nothing. (This is why disabling deadlines bought nothing — see the journal.)
- **Log lines are distinct states.** Measured exactly on the k=9 ladder: 315,184 verdicts,
  315,184 distinct `(state,k)` pairs. The log *is* the fact set; there is nothing to reconstruct.

## What the run must emit

1. Raw stdout, streamed and compressed — the archival artifact, needed for witness-tree
   reconstruction later, which `parse_out.sh` output cannot support.
2. The parsed fact file, regenerated periodically, each with the `# radio-cert v1` header naming
   build and state. This doubles as the restart checkpoint, so it costs nothing extra.
3. Whatever option 1/2/3 above requires for dominance.

The header is not cosmetic. Warm-starting a *negative* from `cache-2025:parsed_260.txt` is
forbidden because it holds the sixteen suspect verdicts and cannot be filtered by era; from a
run's own output it is sound. The header is what makes those two impossible to confuse, and it
is the fix for the exact defect that makes this re-run necessary.

## Open decisions

1. **How `SPLITS` cost scales with part count.** Measured at k=4 on the k=9 ladder, nodes per
   fact go 11, 74, 427, 2231, 9027, 12079 for 2..7 parts — roughly x4.5 per added part, and 93%
   of all nodes sit in 6- and 7-part facts. Part count grows with depth, so this exponent, not
   the fact count, sets the cost at `MAX_N=193`. Measure it on one real k=9 state before sizing
   the machine.
2. **`FAST` on or off for the sixteen.** They are refutations, where the pass-1 filter is pure
   cost (measured 4x on the k=9 ladder) — but refuting a root requires *proving children
   solvable* to rule them out, and that is where `FAST` pays. Decide by running one of the
   sixteen both ways under a fixed budget and comparing verdicts produced. Cheap next to the run.
3. **Whether to certify all sixteen or start with one.** `Sb(112:81)` is the 2023 outlier at
   1,725,456 s; the other fifteen totalled 2,353,729 s with a warm cache.
