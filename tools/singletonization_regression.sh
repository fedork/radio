#!/usr/bin/env bash
# Exercise the deficit-slice frontier and ensure resource exhaustion remains an abort.
set -euo pipefail

cd "$(dirname "$0")/.."
tmp=$(mktemp -d "${TMPDIR:-/tmp}/radio-singletonization.XXXXXX")
trap 'rm -rf "$tmp"' EXIT

CC="${CXX:-clang++}" tools/build_radio.py -O3 -std=c++20 -Wall -Wextra -pedantic \
    tools/search_singletonization.cpp -o "$tmp/search_singletonization" >/dev/null

# Exhaust the closed-form piece optimizer against direct point enumeration on small integer boxes.
python3 - <<'PY'
import importlib.util
from pathlib import Path

path = Path("tools/optimize_mixed_frontier.py")
spec = importlib.util.spec_from_file_location("optimize_mixed_frontier", path)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
for total in range(1, 11):
    for lo in range(total + 1):
        for hi in range(lo, total + 1):
            for pure_u in range(11):
                for pure_v in range(11):
                    delta, p, q, cut_u, cut_v = module.optimize_piece(
                        lo, hi, total, pure_u, pure_v
                    )
                    brute = min(
                        (max(candidate, pure_u) + max(total - candidate, pure_v), candidate)
                        for candidate in range(lo, hi + 1)
                    )[0]
                    if delta != brute or cut_u + cut_v != brute or q != total - p:
                        raise SystemExit(
                            f"piece mismatch total={total} range={lo}:{hi} "
                            f"pure={pure_u},{pure_v}: formula={delta}, brute={brute}"
                        )
PY

# docs/theorems/singleton-majorization.md records the width-11 negative and a width-10 positive
# superstate; subgraph monotonicity gives this adjacent variable-part pair.  The scan must stop at 6.
"$tmp/search_singletonization" slice 4 4 2 5 6 11 2 9 2 3 2 >"$tmp/slice.out"
grep -F 'slice delta=5 variable_width=11 NO k=4 depth=4 state=11:2,11:2,9:2,3:2 ' \
    "$tmp/slice.out" >/dev/null
grep -F 'slice delta=6 variable_width=10 YES k=4 depth=4 state=11:2,10:2,9:2,3:2 ' \
    "$tmp/slice.out" >/dev/null
tools/check_witness.py "$tmp/slice.out" >/dev/null

# m=5 is the exact calibration for the corrected Pareto assembly.  Complete enumeration over
# proven A/B/C frontiers finds the old (3,2,2) family through k=7, a tie at k=8, and the forced
# (4,3,1) family at k=9.  This is the finite counterpart of the Li--Wu--Triesch regime switch.
tools/m5_assembly.py --check-through 64 >/dev/null
for k in {4..9}; do
    "$tmp/search_singletonization" assembly-enumerate "$k" 5 data/pareto_sb.csv \
        >"$tmp/assembly-m5-k$k.out"
    tools/check_witness.py "$tmp/assembly-m5-k$k.out" >/dev/null
    python3 - "$k" "$tmp/assembly-m5-k$k.out" <<'PY'
import re
import sys

k = int(sys.argv[1])
text = open(sys.argv[2]).read()
expected = {
    4: (9, [(1, 5, 3, 3, 2, 3, 2)]),
    5: (22, [(3, 12, 3, 7, 2, 7, 2)]),
    6: (50, [(8, 27, 3, 15, 2, 15, 2)]),
    7: (109, [(20, 58, 3, 31, 2, 31, 2)]),
    8: (231, [(47, 121, 3, 63, 2, 63, 2),
              (57, 116, 4, 58, 3, 64, 1)]),
    9: (481, [(118, 242, 4, 121, 3, 128, 1)]),
}
width, winners = expected[k]
summary = re.search(
    r"^assembly_enumeration_result best_width=(\d+) winners=(\d+) "
    r"optimization_complete=YES best_exact=YES .* known_parent_relation=(\w+)",
    text,
    re.MULTILINE,
)
if not summary:
    raise SystemExit(f"k={k}: missing complete exact m=5 assembly summary")
if (int(summary.group(1)), int(summary.group(2)), summary.group(3)) != (width, len(winners), "EQUAL"):
    raise SystemExit(f"k={k}: unexpected m=5 assembly summary {summary.groups()}")
actual = [tuple(map(int, match)) for match in re.findall(
    r"^assembly_winner index=\d+ candidate_width=\d+ d=(\d+) "
    r"A=(\d+):(\d+) B=(\d+):(\d+) C=(\d+):(\d+) ",
    text,
    re.MULTILINE,
)]
if actual != winners:
    raise SystemExit(f"k={k}: expected m=5 winners {winners}, got {actual}")
PY
done

# The k=9 Pareto level is incomplete as a whole, so the generic enumerator deliberately cannot
# consume it at parent k=10 or 11.  Check the published 4+1 construction's exact hard branches
# directly.  These are positive construction checks; the published theorem supplies the global
# upper bound at parent widths 985 and 2001.
"$tmp/search_singletonization" assembly 10 8 5 496 4 248 3 256 1 241 241 \
    >"$tmp/assembly-m5-k10.out"
grep -F 'assembly d=241 YES parent_width=985 parent_k=10 residual_k=8 depth=8 total_m=5 A=496:4 B=248:3 C=256:1 branch=248:1,241:3,240:1 ' \
    "$tmp/assembly-m5-k10.out" >/dev/null
grep -F 'assembly_result d=241 parent_width=985 global_maximum=NO exact=YES' \
    "$tmp/assembly-m5-k10.out" >/dev/null
tools/check_witness.py "$tmp/assembly-m5-k10.out" >/dev/null

"$tmp/search_singletonization" assembly 11 9 5 1006 4 503 3 512 1 492 492 \
    >"$tmp/assembly-m5-k11.out"
grep -F 'assembly d=492 YES parent_width=2001 parent_k=11 residual_k=9 depth=9 total_m=5 A=1006:4 B=503:3 C=512:1 branch=503:1,494:1,492:3 ' \
    "$tmp/assembly-m5-k11.out" >/dev/null
grep -F 'assembly_result d=492 parent_width=2001 global_maximum=NO exact=YES' \
    "$tmp/assembly-m5-k11.out" >/dev/null
tools/check_witness.py "$tmp/assembly-m5-k11.out" >/dev/null

# The corrected q=5 assembly diagram has four-segment branch
#   (d:beta, b:alpha-beta, c:m-alpha-gamma, a-c:gamma) @ parent_k-2.
# Three proven m=10 frontier rows are recovered by maximizing d for the displayed Pareto triples.
"$tmp/search_singletonization" assembly 5 3 10 7 6 4 4 5 3 2 1 \
    >"$tmp/assembly-k5.out"
grep -F 'assembly d=2 NO parent_width=13 parent_k=5 residual_k=3 depth=3 total_m=10 A=7:6 B=4:4 C=5:3' \
    "$tmp/assembly-k5.out" >/dev/null
grep -F 'assembly d=1 YES parent_width=12 parent_k=5 residual_k=3 depth=3 total_m=10 A=7:6 B=4:4 C=5:3' \
    "$tmp/assembly-k5.out" >/dev/null
grep -F 'assembly_result d=1 parent_width=12 global_maximum=YES exact=YES' \
    "$tmp/assembly-k5.out" >/dev/null
tools/check_witness.py "$tmp/assembly-k5.out" >/dev/null

"$tmp/search_singletonization" assembly 6 4 10 19 6 10 4 12 3 5 4 \
    >"$tmp/assembly-k6.out"
grep -F 'assembly d=5 NO parent_width=34 parent_k=6 residual_k=4 depth=4 total_m=10 A=19:6 B=10:4 C=12:3' \
    "$tmp/assembly-k6.out" >/dev/null
grep -F 'assembly d=4 YES parent_width=33 parent_k=6 residual_k=4 depth=4 total_m=10 A=19:6 B=10:4 C=12:3' \
    "$tmp/assembly-k6.out" >/dev/null
grep -F 'assembly_result d=4 parent_width=33 global_maximum=YES exact=YES' \
    "$tmp/assembly-k6.out" >/dev/null
tools/check_witness.py "$tmp/assembly-k6.out" >/dev/null

"$tmp/search_singletonization" assembly 7 5 10 46 6 22 5 27 3 15 14 \
    >"$tmp/assembly-k7.out"
grep -F 'assembly d=15 NO parent_width=83 parent_k=7 residual_k=5 depth=5 total_m=10 A=46:6 B=22:5 C=27:3' \
    "$tmp/assembly-k7.out" >/dev/null
grep -F 'assembly d=14 YES parent_width=82 parent_k=7 residual_k=5 depth=5 total_m=10 A=46:6 B=22:5 C=27:3' \
    "$tmp/assembly-k7.out" >/dev/null
grep -F 'assembly_result d=14 parent_width=82 global_maximum=YES exact=YES' \
    "$tmp/assembly-k7.out" >/dev/null
tools/check_witness.py "$tmp/assembly-k7.out" >/dev/null

# Repeating the lower-level (6,4,3) height pattern is not optimal at parent k=7: its D boundary
# is 12 NO / 11 YES, one parent-width unit below the alternative B-height-five triple above.
"$tmp/search_singletonization" assembly 7 5 10 46 6 24 4 27 3 12 11 \
    >"$tmp/assembly-k7-nonoptimal.out"
grep -F 'assembly d=12 NO parent_width=82 parent_k=7 residual_k=5 depth=5 total_m=10 A=46:6 B=24:4 C=27:3' \
    "$tmp/assembly-k7-nonoptimal.out" >/dev/null
grep -F 'assembly d=11 YES parent_width=81 parent_k=7 residual_k=5 depth=5 total_m=10 A=46:6 B=24:4 C=27:3' \
    "$tmp/assembly-k7-nonoptimal.out" >/dev/null
grep -F 'assembly_result d=11 parent_width=81 global_maximum=YES exact=YES' \
    "$tmp/assembly-k7-nonoptimal.out" >/dev/null
tools/check_witness.py "$tmp/assembly-k7-nonoptimal.out" >/dev/null

# Enumerate the complete ordered A/B/C triple family from source-carrying proven Pareto rows.
# R_0 is only a necessary bound; every triple capable of tying the incumbent is still decided by
# the exact residual recurrence.  The final summaries therefore certify the family optimum even
# though losing triples below the incumbent need not have their individual d maxima computed.
"$tmp/search_singletonization" assembly-enumerate 5 10 data/pareto_sb.csv \
    >"$tmp/assembly-enumerate-k5.out"
grep -F 'assembly_inventory cartesian=96 admissible=68 geometry_rejected=28 ' \
    "$tmp/assembly-enumerate-k5.out" >/dev/null
grep -F 'assembly_ranking_result ranked=3 complete=YES bound=R0_NECESSARY' \
    "$tmp/assembly-enumerate-k5.out" >/dev/null
grep -F 'assembly_enumeration_result best_width=12 winners=2 optimization_complete=YES best_exact=YES ' \
    "$tmp/assembly-enumerate-k5.out" >/dev/null
grep -F 'assembly_winner index=1 candidate_width=12 d=1 A=7:6 B=4:4 C=5:3 ' \
    "$tmp/assembly-enumerate-k5.out" >/dev/null
grep -F 'assembly_winner index=2 candidate_width=12 d=1 A=7:6 B=4:4 C=4:4 ' \
    "$tmp/assembly-enumerate-k5.out" >/dev/null
tools/check_witness.py "$tmp/assembly-enumerate-k5.out" >/dev/null

"$tmp/search_singletonization" assembly-enumerate 6 10 data/pareto_sb.csv \
    >"$tmp/assembly-enumerate-k6.out"
grep -F 'assembly_inventory cartesian=396 admissible=133 geometry_rejected=263 ' \
    "$tmp/assembly-enumerate-k6.out" >/dev/null
grep -F 'assembly_ranking_result ranked=21 complete=YES bound=R0_NECESSARY' \
    "$tmp/assembly-enumerate-k6.out" >/dev/null
grep -F 'assembly_enumeration_result best_width=33 winners=4 optimization_complete=YES best_exact=YES ' \
    "$tmp/assembly-enumerate-k6.out" >/dev/null
grep -F 'assembly_winner index=4 candidate_width=33 d=4 A=19:6 B=10:4 C=12:3 ' \
    "$tmp/assembly-enumerate-k6.out" >/dev/null
tools/check_witness.py "$tmp/assembly-enumerate-k6.out" >/dev/null

"$tmp/search_singletonization" assembly-enumerate 7 10 data/pareto_sb.csv \
    >"$tmp/assembly-enumerate-k7.out"
grep -F 'assembly_inventory cartesian=2299 admissible=165 geometry_rejected=2134 ' \
    "$tmp/assembly-enumerate-k7.out" >/dev/null
grep -F 'assembly_ranking_result ranked=37 complete=YES bound=R0_NECESSARY' \
    "$tmp/assembly-enumerate-k7.out" >/dev/null
grep -F 'assembly_enumeration_result best_width=82 winners=1 optimization_complete=YES best_exact=YES ' \
    "$tmp/assembly-enumerate-k7.out" >/dev/null
grep -F 'assembly_winner index=1 candidate_width=82 d=14 A=46:6 B=22:5 C=27:3 ' \
    "$tmp/assembly-enumerate-k7.out" >/dev/null
tools/check_witness.py "$tmp/assembly-enumerate-k7.out" >/dev/null

# At the next level exact slices can be expensive, so ranking is a separate completed product.
# This emits all sound R_0 bounds and exits before entering the first exact query.
"$tmp/search_singletonization" assembly-rank 8 10 data/pareto_sb.csv \
    >"$tmp/assembly-rank-k8.out"
grep -F 'assembly_inventory cartesian=11552 admissible=165 geometry_rejected=11387 ' \
    "$tmp/assembly-rank-k8.out" >/dev/null
grep -F 'assembly_rank rank=1 width_upper=195 d_upper=41 A=104:6 B=50:5 C=58:3 ' \
    "$tmp/assembly-rank-k8.out" >/dev/null
grep -F 'assembly_rank rank=37 width_upper=170 d_upper=47 A=87:9 B=36:9 C=64:1 ' \
    "$tmp/assembly-rank-k8.out" >/dev/null
grep -F 'assembly_ranking_result ranked=37 complete=YES bound=R0_NECESSARY known_parent_max=189 ' \
    "$tmp/assembly-rank-k8.out" >/dev/null
if grep -F 'assembly_probe' "$tmp/assembly-rank-k8.out" >/dev/null; then
    echo "rank-only assembly mode entered exact search" >&2
    exit 1
fi

# One particularly simple width-189 target from that ranking reduces to a two-part branch and is
# exactly negative.  This locks that R_0 ranking remains only a necessary bound, even in the diagram.
set +e
"$tmp/search_singletonization" assembly 8 6 10 104 6 46 6 54 4 39 39 \
    >"$tmp/assembly-k8-two-part.out"
result=$?
set -e
if [[ "$result" != 1 ]]; then
    echo "k=8 two-part assembly control exited $result, expected exact negative status 1" >&2
    exit 1
fi
grep -F 'assembly d=39 NO parent_width=189 parent_k=8 residual_k=6 depth=6 total_m=10 A=104:6 B=46:6 C=54:4 branch=50:4,39:6 ' \
    "$tmp/assembly-k8-two-part.out" >/dev/null

# The k=9 input level currently has one theorem row and legacy bounds, not a complete proven
# frontier.  Refuse it rather than silently upgrading lower bounds into Pareto states.
set +e
"$tmp/search_singletonization" assembly-enumerate 10 10 data/pareto_sb.csv \
    >"$tmp/assembly-enumerate-incomplete.out" 2>"$tmp/assembly-enumerate-incomplete.err"
result=$?
set -e
if [[ "$result" != 3 ]]; then
    echo "incomplete Pareto input exited $result, expected abort status 3" >&2
    exit 1
fi
grep -F 'ABORT: Pareto level 9 contains non-max or non-proven rows' \
    "$tmp/assembly-enumerate-incomplete.err" >/dev/null
if grep -F 'assembly_enumeration_result' "$tmp/assembly-enumerate-incomplete.out" >/dev/null; then
    echo "incomplete Pareto input emitted an enumeration result" >&2
    exit 1
fi

# Two singleton rows at k=2 have the complete deficit antichain {(0,1),(1,0)}.  This checks
# coordinate ordering, sharp thresholds, completeness detection and multiple emitted trees.
"$tmp/search_singletonization" mixed-frontier 2 2 1 1 1 1 >"$tmp/mixed-small.out"
grep -F 'mixed_threshold u=0 minimum_v=1 POINT ' "$tmp/mixed-small.out" >/dev/null
grep -F 'mixed_threshold u=1 minimum_v=0 POINT ' "$tmp/mixed-small.out" >/dev/null
grep -F 'mixed_frontier points=2 complete=YES exact=YES u_box=0:1 v_box=0:1' \
    "$tmp/mixed-small.out" >/dev/null
tools/check_witness.py "$tmp/mixed-small.out" >/dev/null

# Keeping the two variable lineages separate exposes the synchronization notch behind the slice
# control: (4,6) is minimal but (5,5) is not feasible, despite the same total deficit.
"$tmp/search_singletonization" mixed-frontier 4 4 2 2 16 16 9 2 3 2 \
    >"$tmp/mixed-sync.out"
python3 - "$tmp/mixed-sync.out" <<'PY'
import re
import sys

text = open(sys.argv[1]).read()
actual = [(int(u), int(v))
          for u, v in re.findall(r"^mixed_point u=(\d+) v=(\d+) ", text, re.MULTILINE)]
expected = [(2, 10), (3, 8), (4, 6), (6, 4), (8, 3), (10, 2)]
if actual != expected:
    raise SystemExit(f"expected synchronized mixed frontier {expected}, got {actual}")
if "mixed_frontier points=6 complete=YES exact=YES" not in text:
    raise SystemExit("synchronized mixed frontier was not reported complete and exact")
PY
tools/check_witness.py "$tmp/mixed-sync.out" >/dev/null
tools/optimize_mixed_frontier.py 5 5 "$tmp/mixed-sync.out" >"$tmp/mixed-optimum.out"
grep -F 'mixed_optimum delta=11 parent_D_width=21 complete=YES exact=YES k=4 depth=4 pure_u=5 pure_v=5 choices=2' \
    "$tmp/mixed-optimum.out" >/dev/null
grep -F 'mixed_choice p=4 q=6 cut_u=5 cut_v=6 piece_u=4:4 piece_sum=10' \
    "$tmp/mixed-optimum.out" >/dev/null
grep -F 'mixed_choice p=6 q=4 cut_u=6 cut_v=5 piece_u=6:6 piece_sum=10' \
    "$tmp/mixed-optimum.out" >/dev/null

# A box-truncated antichain can supply candidates but cannot certify the global D optimum.
"$tmp/search_singletonization" mixed-frontier 3 3 2 2 4 4 >"$tmp/mixed-truncated.out"
set +e
tools/optimize_mixed_frontier.py 0 0 "$tmp/mixed-truncated.out" \
    >"$tmp/mixed-truncated-optimum.out" 2>"$tmp/mixed-truncated-optimum.err"
result=$?
set -e
if [[ "$result" == 0 ]]; then
    echo "optimizer accepted a truncated mixed frontier" >&2
    exit 1
fi
grep -F 'mixed frontier is truncated; refusing to claim a global D optimum' \
    "$tmp/mixed-truncated-optimum.err" >/dev/null

# A complete bounding-box scan at depth < k is still only a sufficient construction predicate.
# Do not let the combiner turn that one-sided approximation into an exact D claim.
sed 's/complete=YES exact=YES/complete=YES exact=NO/' \
    "$tmp/mixed-sync.out" >"$tmp/mixed-bounded.out"
set +e
tools/optimize_mixed_frontier.py 5 5 "$tmp/mixed-bounded.out" \
    >"$tmp/mixed-bounded-optimum.out" 2>"$tmp/mixed-bounded-optimum.err"
result=$?
set -e
if [[ "$result" == 0 ]]; then
    echo "optimizer accepted a bounded-depth mixed frontier as exact" >&2
    exit 1
fi
grep -F 'mixed frontier is bounded-depth only; refusing to claim an exact D optimum' \
    "$tmp/mixed-bounded-optimum.err" >/dev/null

set +e
tools/optimize_mixed_frontier.py 17 5 "$tmp/mixed-sync.out" \
    >"$tmp/mixed-outside-optimum.out" 2>"$tmp/mixed-outside-optimum.err"
result=$?
set -e
if [[ "$result" == 0 ]]; then
    echo "optimizer accepted a pure threshold outside the legal deficit box" >&2
    exit 1
fi
grep -F 'pure threshold lies outside the legal deficit box' \
    "$tmp/mixed-outside-optimum.err" >/dev/null

# The (height 1, height 2) frontier is one stable affine segment over every encoded exact level
# starting at k=3.  This also checks that unequal variable lineages keep their coordinate identity.
for k in {3..11}; do
    max_u=$(( 1 << k ))
    max_v=$(( 1 << k ))
    "$tmp/search_singletonization" mixed-frontier "$k" "$k" 1 2 "$max_u" "$max_v" \
        >"$tmp/mixed-12-$k.out"
    tools/check_witness.py "$tmp/mixed-12-$k.out" >/dev/null
    python3 - "$k" "$tmp/mixed-12-$k.out" <<'PY'
import re
import sys

k = int(sys.argv[1])
text = open(sys.argv[2]).read()
actual = [(int(u), int(v))
          for u, v in re.findall(r"^mixed_point u=(\d+) v=(\d+) ", text, re.MULTILINE)]
expected = [(u, k + 1 - u) for u in range(k + 1)]
if actual != expected:
    raise SystemExit(f"k={k}: expected (1,2) mixed frontier {expected}, got {actual}")
if f"mixed_frontier points={k + 1} complete=YES exact=YES" not in text:
    raise SystemExit(f"k={k}: missing complete exact (1,2) summary")
piece = f"mixed_piece u=0:{k} sum={k + 1} formula=v={k + 1}-u"
if piece not in text:
    raise SystemExit(f"k={k}: missing affine (1,2) piece")
PY
done

# The first genuinely recursive family has stabilized by k=4 in every exact level the key encoding
# can represent near 2^k.  These are finite regression facts, not a proof beyond k=11.
for k in {4..11}; do
    legal=$(( 1 << k ))
    "$tmp/search_singletonization" mixed-frontier "$k" "$k" 2 2 "$legal" "$legal" \
        >"$tmp/mixed-$k.out"
    tools/check_witness.py "$tmp/mixed-$k.out" >/dev/null
    python3 - "$k" "$tmp/mixed-$k.out" <<'PY'
import re
import sys

k = int(sys.argv[1])
text = open(sys.argv[2]).read()
actual = [(int(u), int(v))
          for u, v in re.findall(r"^mixed_point u=(\d+) v=(\d+) ", text, re.MULTILINE)]
expected = ([(1, 2 * k), (2, 2 * k - 1)]
            + [(u, 2 * k - u) for u in range(3, 2 * k - 2)]
            + [(2 * k - 1, 2), (2 * k, 1)])
if actual != expected:
    raise SystemExit(f"k={k}: expected mixed frontier {expected}, got {actual}")
summary = (f"mixed_frontier points={2 * k - 1} complete=YES exact=YES "
           f"u_box=0:{1 << k} v_box=0:{1 << k}")
if summary not in text:
    raise SystemExit(f"k={k}: missing complete exact summary")
pieces = [
    f"mixed_piece u=1:2 sum={2 * k + 1} formula=v={2 * k + 1}-u",
    f"mixed_piece u=3:{2 * k - 3} sum={2 * k} formula=v={2 * k}-u",
    f"mixed_piece u={2 * k - 1}:{2 * k} sum={2 * k + 1} formula=v={2 * k + 1}-u",
]
if any(piece not in text for piece in pieces):
    raise SystemExit(f"k={k}: missing guarded affine (2,2) pieces")
PY
done

# By height four the symbolic frontier already needs guarded pieces rather than one half-plane.
"$tmp/search_singletonization" mixed-frontier 6 6 1 4 64 64 >"$tmp/mixed-14.out"
python3 - "$tmp/mixed-14.out" <<'PY'
import re
import sys

text = open(sys.argv[1]).read()
actual = [(int(u), int(v))
          for u, v in re.findall(r"^mixed_point u=(\d+) v=(\d+) ", text, re.MULTILINE)]
expected = [(0, 17), (1, 16), (2, 15), (4, 14),
            (5, 13), (6, 12), (7, 11), (9, 10)]
if actual != expected:
    raise SystemExit(f"expected guarded (1,4) frontier {expected}, got {actual}")
if "mixed_frontier points=8 complete=YES exact=YES" not in text:
    raise SystemExit("(1,4) mixed frontier was not reported complete and exact")
pieces = [
    "mixed_piece u=0:2 sum=17 formula=v=17-u",
    "mixed_piece u=4:7 sum=18 formula=v=18-u",
    "mixed_piece u=9:9 sum=19 formula=v=19-u",
]
if any(piece not in text for piece in pieces):
    raise SystemExit("(1,4) frontier lost its guarded affine pieces")
PY
tools/check_witness.py "$tmp/mixed-14.out" >/dev/null

# An exhausted memo is deliberately distinguishable from a completed negative scan.
CC="${CXX:-clang++}" tools/build_radio.py -O1 -std=c++20 -Wall -Wextra -pedantic \
    -DSINGLETONIZATION_MAX_MEMO=1 tools/search_singletonization.cpp \
    -o "$tmp/search_singletonization_tiny_memo" >/dev/null
set +e
"$tmp/search_singletonization_tiny_memo" slice 3 3 2 2 2 \
    >"$tmp/abort.out" 2>"$tmp/abort.err"
result=$?
set -e
if [[ "$result" != 3 ]]; then
    echo "tiny-memo query exited $result, expected abort status 3" >&2
    exit 1
fi
grep -F 'ABORT: memo limit reached (abort, not NO) (not a negative verdict)' \
    "$tmp/abort.err" >/dev/null
if grep -Eq ' (YES|NO) ' "$tmp/abort.out"; then
    echo "aborted slice emitted a verdict" >&2
    exit 1
fi

echo "singletonization regression passed"
