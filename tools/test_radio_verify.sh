#!/bin/sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
work_dir=$(mktemp -d /tmp/radio-verify-test.XXXXXX)
trap 'rm -rf "$work_dir"' EXIT HUP INT TERM

cd "$repo_dir"
tools/build_radio.py -O2 -pthread radio_verify.c -o "$work_dir/radio_verify"
tools/build_radio.py -O2 -pthread -DVERIFY_BLOCK_PARETO -DVERIFY_BLOCK_SIZE=2 \
    -DVERIFY_BLOCK_MIN_LEVEL_FACTS=2 radio_verify.c -o "$work_dir/radio_verify_forced_blocks"

fixture=tools/testdata/radio_verify_v1.cert
VERIFY_THREADS=1 VERIFY_MEMO_BITS=18 \
    "$work_dir/radio_verify" "$fixture" 2 > "$work_dir/serial.out"
VERIFY_THREADS=2 VERIFY_MEMO_BITS=18 \
    "$work_dir/radio_verify" "$fixture" 2 > "$work_dir/parallel.out"

grep -Eq 'TOTAL verified 11, unverified 0, budget 0, nodes 6' "$work_dir/serial.out"
grep -Eq 'TOTAL verified 11, unverified 0, budget 0, nodes 6' "$work_dir/parallel.out"
grep -Eq '^BATCH_START phase=verify k=all targets=11 threads=2 progress_seconds=0\.0$' \
    "$work_dir/parallel.out"
grep -Eq '^BATCH_DONE phase=verify k=all completed=11/11 verified=11 unverified=0 budget=0 nodes=6 ' \
    "$work_dir/parallel.out"

for workers in 1 2; do
    TOPDOWN=2 MINIMIZE_BEFORE_COLOR=1 VERIFY_THREADS=$workers VERIFY_MEMO_BITS=18 \
        CERT_OUT="$work_dir/colored-$workers.cert" \
        "$work_dir/radio_verify" "$fixture" 2 > "$work_dir/color-$workers.out"
done
cmp "$work_dir/colored-1.cert" "$work_dir/colored-2.cert"

[ "$(grep -Ec '^root ' "$work_dir/colored-1.cert")" -eq 9 ]
[ "$(grep -Ec '^fact ' "$work_dir/colored-1.cert")" -eq 1 ]
grep -Eq '^fact 1 Sb\(3:1\)$' "$work_dir/colored-1.cert"
if grep -Eq 'Sb\(4:1\)' "$work_dir/colored-1.cert"; then
    echo 'redundant fact survived minimalization' >&2
    exit 1
fi

VERIFY_THREADS=2 VERIFY_MEMO_BITS=18 \
    "$work_dir/radio_verify" "$work_dir/colored-1.cert" 2 > "$work_dir/replay.out"
grep -Eq 'TOTAL verified 10, unverified 0, budget 0, nodes 6' "$work_dir/replay.out"

# The tiny closed fixture is below the production block cutoff. Force two-fact blocks on a
# synthetic same-primary-key level and require the minimal antichain to match the plain index.
blocks_fixture=tools/testdata/radio_verify_blocks_v1.cert
MINIMAL_K=1 "$work_dir/radio_verify" "$blocks_fixture" 1 > "$work_dir/minimal-plain.out"
MINIMAL_K=1 "$work_dir/radio_verify_forced_blocks" "$blocks_fixture" 1 \
    > "$work_dir/minimal-block.out"
grep -Eq 'block Pareto index: size=2, min level=2 facts, 2 blocks' "$work_dir/minimal-block.out"
grep -Eq 'TOTAL         4 facts          2 minimal  \(50\.0% redundant\)' "$work_dir/minimal-block.out"
grep -E 'np=|  TOTAL' "$work_dir/minimal-plain.out" > "$work_dir/minimal-plain.summary"
grep -E 'np=|  TOTAL' "$work_dir/minimal-block.out" > "$work_dir/minimal-block.summary"
cmp "$work_dir/minimal-plain.summary" "$work_dir/minimal-block.summary"

if CERT_OUT="$work_dir/partial.cert" VERIFY_THREADS=2 VERIFY_MEMO_BITS=18 \
    "$work_dir/radio_verify" "$fixture" 2 0 1 3 0 999 1 0 2 \
    > "$work_dir/partial.out" 2>&1; then
    echo 'partial verification unexpectedly emitted a durable certificate' >&2
    exit 1
fi
[ ! -e "$work_dir/partial.cert" ]

if VERIFY_PROGRESS_SECONDS=invalid "$work_dir/radio_verify" "$fixture" 2 \
    > "$work_dir/invalid-progress.out" 2>&1; then
    echo 'invalid progress interval was accepted' >&2
    exit 1
fi
grep -Eq 'VERIFY_PROGRESS_SECONDS must be in 0\.\.86400' "$work_dir/invalid-progress.out"

echo 'radio_verify regression: serial, parallel, coloring, minimalization, progress and text replay OK'
