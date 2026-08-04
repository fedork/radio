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
| sorted small integers, `zstd` | **order 300-600 MB shipped** |

Smaller than the raw log of a *single* 2023 state (905 MB).

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

Verification is *cheaper* than the proof: no search, no `MAYBE`, no re-passes, no heuristics, no
cache tuning. One pass with every answer already known.

## The one real problem: the log is not closed

The solver's cache materialises the **upward closure** — `cacheCantSolve` inserts every
`sbb_greater` substitution — so a child can be refuted purely by domination and **never appear
in the log**. The set of logged facts is therefore not closed under `SPLITS`, and a naive
verifier would reject perfectly good refutations because a referenced child is absent.

Three ways out, in increasing order of rigour:

1. **Emit the closure.** ~19 members per logical insert (measured at `MAX_N=193`), so ~800 M
   facts, ~8x the data. Verification becomes a hash lookup with no dominance logic at all.
   Cheapest to build; but the verifier is then trusting the *emitter's* closure computation,
   which is the opposite of the point.
2. **Emit a `DOM` witness.** When `checkCache` returns `FALSE` by hitting a `cant_solve_marker`
   at a prefix, log the fact that justified it. Certificate stays minimal; each `DOM` step is
   an O(parts) check of `s' <= s` plus a lookup. Needs the marker to remember its originating
   fact, which costs memory in the solver.
3. **Let the verifier find the witness.** Store minimal facts only; when a child is not present
   literally, search the refuted set for some `s' <= s`. Needs a dominance index (by part count
   and mass, then candidate check). Most rigorous — the verifier implements Subgraph
   Monotonicity itself rather than trusting anyone — and the most work.

**Recommendation: (3), with (1) as the fallback if the index turns out slow.** The whole value
of a certificate is that the verifier trusts as little as possible, and (1) moves the trust into
the emitter. But this is the decision that should be made with a measurement — build the index,
time a dominance query against a few million facts, and if it is hopeless take (1) and say so.

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

1. **Dominance: option 1, 2 or 3.** Decide by timing an index, not by preference.
2. **`FAST` on or off for the sixteen.** They are refutations, where the pass-1 filter is pure
   cost (measured 4x on the k=9 ladder) — but refuting a root requires *proving children
   solvable* to rule them out, and that is where `FAST` pays. Decide by running one of the
   sixteen both ways under a fixed budget and comparing verdicts produced. Cheap next to the run.
3. **Whether to certify all sixteen or start with one.** `Sb(112:81)` is the 2023 outlier at
   1,725,456 s; the other fifteen totalled 2,353,729 s with a warm cache.
