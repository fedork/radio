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
