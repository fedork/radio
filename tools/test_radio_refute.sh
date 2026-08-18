#!/bin/sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
work_dir=$(mktemp -d /tmp/radio-refute-test.XXXXXX)
trap 'rm -rf "$work_dir"' EXIT HUP INT TERM

cd "$repo_dir"
tools/build_radio.py -O2 -pthread -DMAX_K=3 -DMAX_N=20 -DRB_TRIGGER=1 \
    radio_refute.c -o "$work_dir/radio_refute"

closed=tools/testdata/radio_verify_v1.cert
for workers in 1 2; do
    REFUTE_THREADS=$workers REFUTE_PROGRESS_SECONDS=0 \
        tools/run_with_provenance.py "$work_dir/radio_refute" "$closed" \
        > "$work_dir/closed-$workers.out" \
        2> "$work_dir/closed-$workers.err"
    grep -Eq '^TOTAL verified 11, gaps 0, prefixes 3$' "$work_dir/closed-$workers.out"
    grep -Eq '^FROZEN_EPOCH cache_branches=[0-9]+ cache_fronts=[0-9]+ split_checksum=[0-9a-f]{16}$' \
        "$work_dir/closed-$workers.out"
    grep -Eq "^# runtime_env.REFUTE_THREADS=$workers$" "$work_dir/closed-$workers.out"
    if grep -q '^REACH:' "$work_dir/closed-$workers.err"; then
        echo 'frozen refuter emitted per-root reachability telemetry' >&2
        exit 1
    fi
done

if REFUTE_THREADS=2 "$work_dir/radio_refute" \
        tools/testdata/radio_refute_gap_v1.cert > "$work_dir/gap.out" 2>&1; then
    echo 'false negative certificate unexpectedly verified' >&2
    exit 1
fi
grep -Eq '^GAP verdict=contradicted k=1 Sb\(1:1\)$' "$work_dir/gap.out"
grep -Eq '^TOTAL verified 0, gaps 1, prefixes 0$' "$work_dir/gap.out"

echo 'radio_refute regression: frozen serial/parallel replay and fail-closed gap detection OK'
