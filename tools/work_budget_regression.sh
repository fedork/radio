#!/usr/bin/env bash
# A fixed work allowance must stop at the same accepted-prefix count and leave the same ordered cache
# facts in two independent cold processes.  Wall time is deliberately not part of the comparison.
set -euo pipefail
cd "$(dirname "$0")/.."

tmp=$(mktemp -d "${TMPDIR:-/tmp}/radio-work-budget.XXXXXX")
trap 'rm -rf "$tmp"' EXIT

python3 tools/build_radio.py -O3 -DMAX_K=5 -DMAX_N=127 \
    tools/budget_probe.c -o "$tmp/budget_probe" >/dev/null

run_probe() {
    local label=$1 status
    set +e
    "$tmp/budget_probe" 50 5 15 3 14 3 17 2 8 4 11 2 10 2 19 1 15 1 \
        >"$tmp/$label.out" 2>"$tmp/$label.err"
    status=$?
    set -e
    [[ "$status" == 2 ]] || { echo "$label exited $status, expected MAYBE (2)" >&2; exit 1; }
    grep -E '^BUDGET_PROBE result=MAYBE requested_ms=50 .* work=1000001 state=' \
        "$tmp/$label.out" >/dev/null
    ./parse_out.sh <"$tmp/$label.out" | grep -E '^[+-] ' >"$tmp/$label.facts"
    [[ -s "$tmp/$label.facts" ]] || { echo "$label produced no reusable cache facts" >&2; exit 1; }
}

run_probe first
run_probe second
cmp "$tmp/first.facts" "$tmp/second.facts"

echo "work budget regression passed"
