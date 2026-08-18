#!/bin/sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
work_dir=$(mktemp -d /tmp/radio-refute-test.XXXXXX)
trap 'rm -rf "$work_dir"' EXIT HUP INT TERM

cd "$repo_dir"
tools/build_radio.py -O2 -pthread -DMAX_K=3 -DMAX_N=20 -DRB_TRIGGER=1 \
    radio_refute.c -o "$work_dir/radio_refute"
tools/build_radio.py -O2 -pthread -DMAX_K=3 -DMAX_N=20 -DRB_TRIGGER=1 \
    -DRADIO_REFUTE_ENABLE_L1 radio_refute.c -o "$work_dir/radio_refute_with_l1"
tools/build_radio.py -O2 -DMAX_K=6 -DMAX_N=16 \
    tools/star_majorization_regression.c -o "$work_dir/star_majorization_regression"
"$work_dir/star_majorization_regression" > "$work_dir/star-majorization.out"
grep -Eq '^star majorization endpoint regression: [0-9]+ states agree$' \
    "$work_dir/star-majorization.out"

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

tools/make_refute_level_certificate.py "$closed" --level 1 -o "$work_dir/closed-k1-v2.cert"
tools/make_refute_level_certificate.py "$closed" --level 2 -o "$work_dir/closed-k2-v2.cert"
for workers in 1 2; do
    REFUTE_THREADS=$workers REFUTE_PROGRESS_SECONDS=0 \
        tools/run_with_provenance.py "$work_dir/radio_refute" "$work_dir/closed-k2-v2.cert" \
        > "$work_dir/closed-k2-v2-$workers.out" \
        2> "$work_dir/closed-k2-v2-$workers.err"
    grep -Eq '^TOTAL verified 9, gaps 0, prefixes 3$' "$work_dir/closed-k2-v2-$workers.out"
    grep -Eq '^CACHE_DONE claims=2 support_level=1 ' "$work_dir/closed-k2-v2-$workers.out"
    grep -Eq '^INPUT .* format=level-v2 .* level=2 support_level=1 split_hints=8$' \
        "$work_dir/closed-k2-v2-$workers.out"
done
REFUTE_THREADS=2 REFUTE_PROGRESS_SECONDS=0 \
    tools/run_with_provenance.py "$work_dir/radio_refute" "$work_dir/closed-k1-v2.cert" \
    > "$work_dir/closed-k1-v2.out" 2> "$work_dir/closed-k1-v2.err"
grep -Eq '^TOTAL verified 2, gaps 0, prefixes 0$' "$work_dir/closed-k1-v2.out"
grep -Eq '^CACHE_DONE claims=0 support_level=0 ' "$work_dir/closed-k1-v2.out"

REFUTE_THREADS=2 REFUTE_PROGRESS_SECONDS=0 \
    tools/run_with_provenance.py "$work_dir/radio_refute_with_l1" "$work_dir/closed-k2-v2.cert" \
    > "$work_dir/closed-k2-v2-with-l1.out" 2> "$work_dir/closed-k2-v2-with-l1.err"
grep -Eq '^TOTAL verified 9, gaps 0, prefixes 3$' "$work_dir/closed-k2-v2-with-l1.out"

sed 's/uses 5/uses 6/' "$work_dir/closed-k2-v2.cert" > "$work_dir/bad-hint-v2.cert"
if REFUTE_THREADS=1 "$work_dir/radio_refute" "$work_dir/bad-hint-v2.cert" \
        > "$work_dir/bad-hint.out" 2>&1; then
    echo 'incorrect split-hint usage count unexpectedly accepted' >&2
    exit 1
fi
grep -Eq 'split hint for .* says 6 uses, observed 5' "$work_dir/bad-hint.out"

# IDs are certificate-local, but their dictionary is canonically ordered so descending ID records
# are also in the trie order.  Swapping two definitions must fail closed instead of feeding an
# unnormalised state to the dominance cache.
awk '
    /^part 1 / { first=$0; next }
    /^part 2 / {
        second=$0
        sub(/^part 2 /, "part 1 ", second)
        sub(/^part 1 /, "part 2 ", first)
        print second
        print first
        next
    }
    { print }
' "$work_dir/closed-k2-v2.cert" > "$work_dir/bad-dictionary-v2.cert"
if REFUTE_THREADS=1 "$work_dir/radio_refute" "$work_dir/bad-dictionary-v2.cert" \
        > "$work_dir/bad-dictionary.out" 2>&1; then
    echo 'noncanonical part dictionary unexpectedly accepted' >&2
    exit 1
fi
grep -Eq 'malformed or duplicate part definition' "$work_dir/bad-dictionary.out"

if REFUTE_THREADS=2 "$work_dir/radio_refute" \
        tools/testdata/radio_refute_gap_v1.cert > "$work_dir/gap.out" 2>&1; then
    echo 'false negative certificate unexpectedly verified' >&2
    exit 1
fi
grep -Eq '^GAP verdict=contradicted k=1 Sb\(1:1\)$' "$work_dir/gap.out"
grep -Eq '^TOTAL verified 0, gaps 1, prefixes 0$' "$work_dir/gap.out"

echo 'radio_refute regression: level-v2 loading, split hints, endpoint majorization, L1 switch, frozen serial/parallel replay and fail-closed gaps OK'
