#!/usr/bin/env bash
# Force joint-suffix reachability to arm immediately and verify that its suffix-dependent rejection
# cannot be promoted to an implicit shorter negative.  The full state below is unsolvable in three,
# while the one-part contraction candidate Sb(5:3) is solvable.
set -euo pipefail
cd "$(dirname "$0")/.."

tmp=$(mktemp -d "${TMPDIR:-/tmp}/radio-rb-contraction.XXXXXX")
trap 'rm -rf "$tmp"' EXIT

python3 tools/build_radio.py -O3 -DMAX_K=3 -DMAX_N=20 -DRB_TRIGGER=1 \
    radio_one.c -o "$tmp/radio_one" >/dev/null

if "$tmp/radio_one" 3 5 3 2 2 2 2 2 2 >"$tmp/full.out" 2>"$tmp/full.err"; then
    echo "expected the four-part state to be unsolvable" >&2
    exit 1
else
    status=$?
    [[ "$status" == 1 ]] || { echo "four-part query exited $status, expected 1" >&2; exit 1; }
fi
grep -F 'VERDICT UNSOLVABLE' "$tmp/full.out" >/dev/null
grep -F 'contraction=rb-suppressed:1' "$tmp/full.out" >/dev/null
grep -F 'REACH:' "$tmp/full.err" >/dev/null
grep -F 'pruned (100.0%)' "$tmp/full.err" >/dev/null

./parse_out.sh <"$tmp/full.out" >"$tmp/full.cache"
grep -F -- '- b 5 3 2 2 2 2 2 2 t 27 20 3' "$tmp/full.cache" >/dev/null
if grep -F -- '- b 5 3 t 15 8 3' "$tmp/full.cache" >/dev/null; then
    echo "unsafe one-part negative escaped into the replay cache" >&2
    exit 1
fi

"$tmp/radio_one" 3 5 3 >"$tmp/prefix.out" 2>"$tmp/prefix.err"
grep -F 'VERDICT SOLVABLE' "$tmp/prefix.out" >/dev/null

tools/sa193_watchdog.sh --log "$tmp/full.out" --pid 999999 --interval 60 \
    >"$tmp/watchdog.out"
grep -F 'rb-tainted contractions 1 suppressed' "$tmp/watchdog.out" >/dev/null
grep -F 'latest suppression      Sb(5:3,2:2,2:2,2:2) in 3 target-size=1' \
    "$tmp/watchdog.out" >/dev/null

echo "rb contraction regression passed"
