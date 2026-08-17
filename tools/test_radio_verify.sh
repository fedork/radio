#!/bin/sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
work_dir=$(mktemp -d /tmp/radio-verify-test.XXXXXX)
trap 'rm -rf "$work_dir"' EXIT HUP INT TERM

cd "$repo_dir"
tools/build_radio.py -O2 -pthread radio_verify.c -o "$work_dir/radio_verify"

fixture=tools/testdata/radio_verify_v1.cert
VERIFY_THREADS=1 VERIFY_MEMO_BITS=18 \
    "$work_dir/radio_verify" "$fixture" 2 > "$work_dir/serial.out"
VERIFY_THREADS=2 VERIFY_MEMO_BITS=18 \
    "$work_dir/radio_verify" "$fixture" 2 > "$work_dir/parallel.out"

rg -q 'TOTAL verified 11, unverified 0, budget 0, nodes 6' "$work_dir/serial.out"
rg -q 'TOTAL verified 11, unverified 0, budget 0, nodes 6' "$work_dir/parallel.out"

for workers in 1 2; do
    TOPDOWN=2 MINIMIZE_BEFORE_COLOR=1 VERIFY_THREADS=$workers VERIFY_MEMO_BITS=18 \
        CERT_OUT="$work_dir/colored-$workers.cert" \
        "$work_dir/radio_verify" "$fixture" 2 > "$work_dir/color-$workers.out"
done
cmp "$work_dir/colored-1.cert" "$work_dir/colored-2.cert"

[ "$(rg -c '^root ' "$work_dir/colored-1.cert")" -eq 9 ]
[ "$(rg -c '^fact ' "$work_dir/colored-1.cert")" -eq 1 ]
rg -q '^fact 1 Sb\(3:1\)$' "$work_dir/colored-1.cert"
if rg -q 'Sb\(4:1\)' "$work_dir/colored-1.cert"; then
    echo 'redundant fact survived minimalization' >&2
    exit 1
fi

VERIFY_THREADS=2 VERIFY_MEMO_BITS=18 \
    "$work_dir/radio_verify" "$work_dir/colored-1.cert" 2 > "$work_dir/replay.out"
rg -q 'TOTAL verified 10, unverified 0, budget 0, nodes 6' "$work_dir/replay.out"

if CERT_OUT="$work_dir/partial.cert" VERIFY_THREADS=2 VERIFY_MEMO_BITS=18 \
    "$work_dir/radio_verify" "$fixture" 2 0 1 3 0 999 1 0 2 \
    > "$work_dir/partial.out" 2>&1; then
    echo 'partial verification unexpectedly emitted a durable certificate' >&2
    exit 1
fi
[ ! -e "$work_dir/partial.cert" ]

echo 'radio_verify regression: serial, parallel, coloring, minimalization and text replay OK'
