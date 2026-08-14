#!/usr/bin/env bash
# Lock diagnostic per-suffix call/prune accounting and the opt-in exact-pliability cutoff.
set -euo pipefail
cd "$(dirname "$0")/.."

tmp=$(mktemp -d "${TMPDIR:-/tmp}/radio-rb-suffix-profile.XXXXXX")
trap 'rm -rf "$tmp"' EXIT

python3 tools/build_radio.py -O3 -DMAX_K=3 -DMAX_N=40 -DRB_TRIGGER=1 \
    -DRADIO_RB_PROFILE_DIAGNOSTIC radio_one.c -o "$tmp/profile" >/dev/null
python3 tools/build_radio.py -O3 -DMAX_K=3 -DMAX_N=40 -DRB_TRIGGER=1 \
    -DRADIO_RB_PROFILE_DIAGNOSTIC -DRADIO_RB_PLIABLE_CUTOFF \
    radio_one.c -o "$tmp/cutoff" >/dev/null

# sigma-D=1 is not a sound cutoff by itself: the final suffix here rejects one reached prefix.
"$tmp/profile" 3 4 3 4 1 2 2 2 2 >"$tmp/rigid.out" 2>"$tmp/rigid.err"
grep -F 'RB_PROFILE_SUFFIX index=3 parts=1 mass=4 excess=2 calls=3 pruned=1' \
    "$tmp/rigid.err" >/dev/null
grep -F 'RB_PROFILE_END tested=6 pruned=1' "$tmp/rigid.err" >/dev/null

# The exact post-build scan may skip a universally pliable suffix without changing the verdict.
"$tmp/cutoff" 3 4 3 2 2 2 2 2 1 >"$tmp/pliable.out" 2>"$tmp/pliable.err"
grep -F 'RB_PROFILE_END tested=0 pruned=0 pliable_skipped=6' "$tmp/pliable.err" >/dev/null
grep -F 'VERDICT SOLVABLE' "$tmp/pliable.out" >/dev/null

echo "rb suffix profile regression passed"
