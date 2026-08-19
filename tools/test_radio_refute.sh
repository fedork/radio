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
tools/build_radio.py -O2 -pthread -DMAX_K=3 -DMAX_N=20 -DRB_TRIGGER=1 \
    -DRADIO_REFUTE_ENABLE_COLORING radio_refute.c -o "$work_dir/radio_refute_color"
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

color_source=tools/testdata/radio_refute_color_v1.cert
tools/make_refute_level_certificate.py "$color_source" --level 3 \
    -o "$work_dir/color-k3-v2.cert"
for workers in 1 2; do
    REFUTE_THREADS=$workers REFUTE_PROGRESS_SECONDS=0 \
        tools/run_with_provenance.py "$work_dir/radio_refute_color" \
        "$work_dir/color-k3-v2.cert" "$work_dir/color-k2-$workers.selection" \
        > "$work_dir/color-k3-$workers.out" 2> "$work_dir/color-k3-$workers.err"
    grep -Eq '^TOTAL verified 1, gaps 0, prefixes 1$' "$work_dir/color-k3-$workers.out"
    grep -Eq '^COLOR_SELECTION parent_level=3 selected_level=2 support=2 used=1 citation_hits=1 ' \
        "$work_dir/color-k3-$workers.out"
done
cmp "$work_dir/color-k2-1.selection" "$work_dir/color-k2-2.selection"
grep -Fxq 'source-claims 1' "$work_dir/color-k2-1.selection"
grep -Fxq 'audited 1' "$work_dir/color-k2-1.selection"
grep -Fxq 'use 1 Sb(2:2,2:2)' "$work_dir/color-k2-1.selection"
if grep -q 'Sb(4:1,2:2)' "$work_dir/color-k2-1.selection"; then
    echo 'coloring retained an uncited support fact' >&2
    exit 1
fi
if REFUTE_THREADS=1 "$work_dir/radio_refute_color" "$work_dir/color-k3-v2.cert" \
        "$work_dir/color-k2-1.selection" > "$work_dir/color-overwrite.out" 2>&1; then
    echo 'coloring unexpectedly overwrote an existing selection' >&2
    exit 1
fi
grep -Eq 'cannot reserve new color selection .*File exists' "$work_dir/color-overwrite.out"

tools/make_refute_level_certificate.py "$color_source" --level 2 \
    --selection "$work_dir/color-k2-1.selection" -o "$work_dir/color-k2-filtered.cert"
grep -Fxq 'claims 2 1 2' "$work_dir/color-k2-filtered.cert"
REFUTE_THREADS=2 REFUTE_PROGRESS_SECONDS=0 \
    tools/run_with_provenance.py "$work_dir/radio_refute_color" \
    "$work_dir/color-k2-filtered.cert" "$work_dir/color-k1.selection" \
    > "$work_dir/color-k2-filtered.out" 2> "$work_dir/color-k2-filtered.err"
grep -Eq '^TOTAL verified 1, gaps 0, prefixes 2$' "$work_dir/color-k2-filtered.out"
grep -Fxq 'used 0' "$work_dir/color-k1.selection"
if tools/make_refute_level_certificate.py "$color_source" --level 1 \
        --selection "$work_dir/color-k1.selection" -o "$work_dir/empty-color.cert" \
        > "$work_dir/empty-color.out" 2>&1; then
    echo 'empty terminal color selection unexpectedly generated another level' >&2
    exit 1
fi
grep -Eq 'color selection is empty; the top-down chain is complete' "$work_dir/empty-color.out"

# --support-selection trims level-(k-1) support to the facts a level-k audit cited. The level-3
# audit cited exactly one level-2 fact, so the trimmed level-3 certificate must carry that one fact
# and must still close with zero gaps -- that is the whole soundness claim, so assert it directly
# rather than trusting the argument.
tools/make_refute_level_certificate.py "$color_source" --level 3 \
    --support-selection "$work_dir/color-k2-1.selection" -o "$work_dir/trim-support-k3.cert"
grep -Fxq 'support 2 1 2' "$work_dir/trim-support-k3.cert"
grep -Eq '^# support-selection color-k2-1\.selection sha256=[0-9a-f]{64} parent-level=3 used=1$' \
    "$work_dir/trim-support-k3.cert"
REFUTE_THREADS=2 REFUTE_PROGRESS_SECONDS=0 REFUTE_MIN_K=3 REFUTE_MAX_K=3 \
    tools/run_with_provenance.py "$work_dir/radio_refute" "$work_dir/trim-support-k3.cert" \
    > "$work_dir/trim-support-k3.out" 2> "$work_dir/trim-support-k3.err"
grep -Eq '^TOTAL verified 1, gaps 0,' "$work_dir/trim-support-k3.out"

# A support selection must target level k-1: pointing one at the wrong level has to fail closed.
if tools/make_refute_level_certificate.py "$color_source" --level 2 \
        --support-selection "$work_dir/color-k2-1.selection" -o "$work_dir/bad-support.cert" \
        > "$work_dir/bad-support.out" 2>&1; then
    echo 'support selection for the wrong level was unexpectedly accepted' >&2
    exit 1
fi
grep -Eq 'support selection targets level 2, not support level 1' "$work_dir/bad-support.out"

sed 's/use 1 Sb(2:2,2:2)/use 1 Sb(4:1,2:2)/' "$work_dir/color-k2-1.selection" \
    > "$work_dir/bad-color-state.selection"
if tools/make_refute_level_certificate.py "$color_source" --level 2 \
        --selection "$work_dir/bad-color-state.selection" -o "$work_dir/bad-color.cert" \
        > "$work_dir/bad-color.out" 2>&1; then
    echo 'color selection with mismatched source state unexpectedly accepted' >&2
    exit 1
fi
grep -Eq 'selection use 1 state does not match source' "$work_dir/bad-color.out"

if REFUTE_THREADS=1 "$work_dir/radio_refute_color" "$closed" "$work_dir/v1.selection" \
        > "$work_dir/color-v1.out" 2>&1; then
    echo 'coloring unexpectedly accepted a non-level certificate' >&2
    exit 1
fi
grep -Eq 'coloring requires a self-contained level-v2 certificate' "$work_dir/color-v1.out"

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

echo 'radio_refute regression: level-v2 loading, split hints, endpoint majorization, L1 switch, deterministic coloring/filtering, frozen serial/parallel replay and fail-closed gaps OK'
