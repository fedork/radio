#!/usr/bin/env bash
# Exercise the deficit-slice frontier and ensure resource exhaustion remains an abort.
set -euo pipefail

cd "$(dirname "$0")/.."
tmp=$(mktemp -d "${TMPDIR:-/tmp}/radio-singletonization.XXXXXX")
trap 'rm -rf "$tmp"' EXIT

CC="${CXX:-clang++}" tools/build_radio.py -O3 -std=c++20 -Wall -Wextra -pedantic \
    tools/search_singletonization.cpp -o "$tmp/search_singletonization" >/dev/null

# docs/theorems/singleton-majorization.md records the width-11 negative and a width-10 positive
# superstate; subgraph monotonicity gives this adjacent variable-part pair.  The scan must stop at 6.
"$tmp/search_singletonization" slice 4 4 2 5 6 11 2 9 2 3 2 >"$tmp/slice.out"
grep -F 'slice delta=5 variable_width=11 NO k=4 depth=4 state=11:2,11:2,9:2,3:2 ' \
    "$tmp/slice.out" >/dev/null
grep -F 'slice delta=6 variable_width=10 YES k=4 depth=4 state=11:2,10:2,9:2,3:2 ' \
    "$tmp/slice.out" >/dev/null
tools/check_witness.py "$tmp/slice.out" >/dev/null

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
