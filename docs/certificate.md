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
| x6.6 for dominance breadcrumbs (measured, see below) | order 13-26 GB raw |
| sorted small integers, `zstd` at ~15-25% | **order 2-6 GB shipped** |

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

Verification is *cheaper* than the proof: no search, no `MAYBE`, no re-passes, no heuristics, no
cache tuning. One pass with every answer already known.

## The one real problem: the log is not closed

The solver's cache materialises the **upward closure** — `cacheCantSolve` inserts every
`sbb_greater` substitution — so a child can be refuted purely by domination and **never appear
in the log**. The set of logged facts is therefore not closed under `SPLITS`, and a naive
verifier would reject perfectly good refutations because a referenced child is absent.

### The fix: inference breadcrumbs

Emit, for each distinct state that was refuted *by dominance*, a record naming the witness:

```
- b 9 5 7 3 t 66 24 5   dom 41827        <- this state is refuted; witness is fact 41827
```

A breadcrumb is a **hint, not an assertion**. The verifier confirms `s' <= s` componentwise
(O(parts)) and confirms `s'` is itself in the set. A wrong breadcrumb fails the check. So this
adds **nothing** to the trust base — it is exactly the LRAT-versus-DRAT distinction, where hints
make checking cheap without making it more credulous. An earlier draft of this page claimed
witnesses meant "trusting the emitter"; that was wrong.

**It is free to produce.** `cant_solve_marker` is a shared sentinel occupying the node's `next`
pointer slot. Replace it with a *tagged pointer* carrying the originating fact's id — low bits
tag it as a marker, the rest is the id. No extra memory in the solver, and the breadcrumb falls
out of the closure machinery that already exists.

### What it costs, measured

Instrumented `checkCache` over the k=8 ladder:

```
2,256,002 FALSE probes
   -> 18,770 distinct (state,k) pairs
        16,326 reached by a STRICT PREFIX   (refuted by dominance)
         2,534 reached at full depth
   vs  2,854 explicitly logged negatives
```

So the certificate carries **~6.6x the facts the log contains** — the dominance-derived states
have to be named, because the verifier will encounter them when it re-enumerates splits. That is
what the x6.6 row in the size table above accounts for: **order 2-6 GB shipped** for all sixteen.

That ratio is measured at k=8 ladder scale and may differ at `MAX_N=193`; it is the first thing
to re-measure on a real state.

### Why pay the 6.6x

The alternative is a minimal certificate plus a **dominance index** in the verifier, which
searches the fact set for some `s' <= s`. That is smaller on disk and strictly more complex
where it matters least: the verifier is the artefact we audit, and every line in it is a line
someone has to believe. Breadcrumbs reduce each dominance step to a componentwise comparison a
reader can check by eye.

Low single-digit GB is not the binding constraint on this project. Verifier simplicity is.

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

1. **Re-measure the breadcrumb ratio on a real state.** 6.6x comes from the k=8 ladder. If it
   is much worse at `MAX_N=193` — the dominance closure is wider there — reconsider a minimal
   certificate with a verifier-side index.
2. **`FAST` on or off for the sixteen.** They are refutations, where the pass-1 filter is pure
   cost (measured 4x on the k=9 ladder) — but refuting a root requires *proving children
   solvable* to rule them out, and that is where `FAST` pays. Decide by running one of the
   sixteen both ways under a fixed budget and comparing verdicts produced. Cheap next to the run.
3. **Whether to certify all sixteen or start with one.** `Sb(112:81)` is the 2023 outlier at
   1,725,456 s; the other fifteen totalled 2,353,729 s with a warm cache.
