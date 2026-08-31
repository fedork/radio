# Exact transfer-shell census through the `K=6` distance-13 ball

Date: 2026-08-31.

## Result

For a sorted full-mass exact-support parent `a <=_w G_K`, define its transfer distance by

    d(a,G_K) = (1/2) sum_i |a_i-(G_K)_i|.

The exact Fixed-Color Hall census gives:

| level and range | exact-support parents | majorized first cut | no first cut |
|---|---:|---:|---:|
| `K=5`, `d<=14` | 311,082,023 | 311,082,023 | 0 |
| `K=6`, `d<=13` | 5,189,450,419 | 5,189,450,419 | 0 |

The proved parent

    (64,63,57^2,42^4,22^7,8^15,7^2,1^32)

has distance 14 and no legal first cut.  Therefore:

> Transfer distance 14 is the global minimum among full-mass exact-support `K=6` parents that
> have no first split into three `G_5`-majorized children.

This replaces the earlier certificate-only minimum: an uncertified no-first-cut hole at distance
at most 13 is now excluded by direct exhaustive search.  It does **not** say that the displayed
parent is the unique hole at distance 14, because the other 9,960,648,264 parents in that shell
were not classified.  It also does not prove minimum distance among recursively unsolvable
`K=6` parents: a closer parent could have a majorized first cut whose child is an as-yet-unknown
`K=5` counterexample.

At `K=5`, every parent in the displayed ball is fully solvable, not merely first-cut feasible.
Full mass forces all three children to have full `G_4` mass, and the Singleton Majorization
Converse is exhaustively true at `K=4`.  This bounded result does not settle `K=5`: the full
exact-support space contains 1,431,800,647,444 parents.

## Exact search

The new transfer-shell modes in `tools/singleton_pair_coloring_census.cpp` enumerate precisely the
positive, nonincreasing, full-mass sequences majorized by `G_K` at a fixed half-`l1` distance.
The recursion maintains position, remaining mass, next-row maximum and used `l1` distance.  A
memoized suffix count independently rederives the requested shell size before the search and
prunes zero-completion subtrees.

For each parent, the search first tries the deterministic one-block-lookahead coloring.  A
successful coloring has already checked every Fixed-Color Hall inequality and is a sound positive
verdict.  Only a lookahead failure invokes `GeneralSearch`, which exhausts the number of rows of
each equal-value block sent left; complementation normalizes the first block.  The Fixed-Color Hall
criterion is exact for existence of a legal row allocation into three `G_(K-1)`-majorized
children, so an exhausted coloring search is a sound no-first-cut verdict.

Parallel workers receive contiguous intervals in the deterministic DFS rank order.  Suffix counts
skip earlier intervals without visiting their leaves.  The intervals are disjoint by construction,
and the run is accepted only if the sum of tested states equals the independently known shell
count.  Search statistics are deterministic and agree between sequential and parallel controls.

The complete `K=6` run reported

    TRANSFER_BALL_PARALLEL_COLORING_CENSUS K=6 maximum_distance=13 workers=12
      complete=YES verified=YES states=5189450419
      lookahead_ok=5184706512 exact_fallbacks=4743907
      exact_nodes=81692145 max_exact_nodes=22 hole=()

Thus one-block lookahead handled 99.908585561% of the ball.  The 4,743,907 remaining parents were
all closed positively by exact search.  The three largest shells were:

| distance | states | exact fallbacks | exact nodes | wall inside shell |
|---:|---:|---:|---:|---:|
| 11 | 371,716,237 | 312,831 | 5,200,533 | 208.903 s |
| 12 | 1,169,629,534 | 1,049,886 | 17,866,518 | 556.354 s |
| 13 | 3,494,418,291 | 3,269,295 | 56,828,225 | 1,742.9 s |

The clean `K=5` repetition reported

    TRANSFER_BALL_PARALLEL_COLORING_CENSUS K=5 maximum_distance=14 workers=12
      complete=YES verified=YES states=311082023
      lookahead_ok=311082023 exact_fallbacks=0 exact_nodes=0 hole=()

## Controls

`tools/singleton_transfer_shell_regression.sh` checks all of the following:

- the independent shell counts sum to 1,431,800,647,444 over the 116 nonempty `K=5` shells and
  15,150,098,684 through `K=6` distance 14;
- every one of the 160 exact-support `K=3` parents is feasible;
- sequential and four-worker enumeration of the complete 1,980,479-state `K=6`, distance-7 shell
  agree exactly: 836 fallbacks, 12,239 exact nodes, maximum 18;
- a 1,000-state window beginning at rank 3,000,000,000 of the distance-13 shell is reached by
  suffix-count skipping and verifies its exact endpoints;
- the final `K=5` shell at distance 115 contains 82 states, all feasible;
- canonical `G_6` and transfer step 13 are feasible, while the distance-14 counterexample is
  infeasible in seven normalized exact-search nodes.

An AddressSanitizer plus UndefinedBehaviorSanitizer build repeated the shell-count, parallel
`K=3`, complete distance-7 `K=6`, and counterexample controls without a finding.  LeakSanitizer
was disabled because it is unsupported by this macOS toolchain.

## Provenance and cost

The complete `K=6` invocation was

```sh
tools/capped_run.sh --seconds 3600 --rss-gb 2 \
  --label k6-transfer-ball13-parallel -- \
  tools/run_with_provenance.py /tmp/singleton_pair_shell_parallel3 \
  --transfer-ball-parallel 6 13 12
```

It completed with exit 0 in 2,602 wall seconds (43m22s), 2,597.9 seconds inside the census, at
1,997,560 parents/s and 0.02 GiB peak RSS.  `tools/check_provenance.py` accepts the raw log.  The
build was made from a dirty exploratory checkout based at `b3fc63e`, but this does not leave the
source unrecoverable: its recorded source SHA-256
`d3fd47be267dc5343f8e85e5898f68b7e86f3ed6bc4cca81fb69c06ef82b88aa` exactly matches
`tools/singleton_pair_coloring_census.cpp` in commit `41f3016`.  The build ID is
`cc03f6ca05f9f451ccea2b74e7d76a45d187268b7939aac3ef9b087c2a3c3480`.

The clean `K=5` invocation used commit `ce9aaf3`, source SHA-256
`febd11a65f5cc718c3d58c0f6226f231415fb67a7fd9535df86ff0ef833cdaac`, and build ID
`e16526cb27fb44729a94bdd2ac8dd893913b8afeea6b1d4368c4592f8d374e38`.  It completed with exit 0
in 65 wall seconds (61.5867 seconds inside the census), at 5,051,130 parents/s and 0.01 GiB peak
RSS.  Its raw log also passes `tools/check_provenance.py` with both dirty flags `no`.

The clean regression build ID is
`c45465971e9861d15ab1a1295740ded4e6cb70841d3f61064b4557843923f7d0`.  The sanitizer build ID is
`cbc1ed8b2e44c15e02bf34e847a3263468b210cb942e7ba6a8bcb50718254579`; it used the same committed
source hash.  Before every long launch, the process inventory was empty; every launched process
has exited.

## Aborted diagnostic

The first distant-`K=5` probes omitted a zero-completion-subtree prune in the streaming iterator.
Two bounded probes at distances 80 and 115 each consumed about 63 CPU seconds in enumeration and
were terminated without a verdict after a stack sample identified the cause.  With the prune,
the same distance-80 100,000-state window took 0.198145 seconds and the complete 82-state terminal
shell took 0.0233735 seconds.  This was generation overhead, not a hard Hall instance.
