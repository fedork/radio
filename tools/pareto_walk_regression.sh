#!/usr/bin/env bash
# Exercise both radio_pareto traversal modes against the exact K=3 frontier.
set -euo pipefail
cd "$(dirname "$0")/.."

work=$(mktemp -d "${TMPDIR:-/tmp}/radio-pareto-regression.XXXXXX")
trap 'rm -rf -- "$work"' EXIT

tools/build_radio.py -O2 -DMAX_K=3 -DMAX_N=10 -DMAX_PART_N=10 \
    radio_pareto.c -o "$work/radio_pareto" >/dev/null

"$work/radio_pareto" --bootstrap-diagonal 3 1 1 >"$work/diagonal.out"
awk '/^PARETO CELL / {
    for (i=1; i<=NF; i++) {
        if ($i ~ /^m=/) { split($i,a,"="); m=a[2] }
        if ($i ~ /^max_n1=/) { split($i,a,"="); n=a[2] }
    }
    print m, n
}' "$work/diagonal.out" | sort -n >"$work/diagonal.cells"
printf '1 8\n2 7\n3 5\n4 4\n' >"$work/expected.cells"
cmp "$work/expected.cells" "$work/diagonal.cells"
grep -q '^PARETO BOOTSTRAP_END ' "$work/diagonal.out"
grep -q '^PARETO DONE k=3 cells=4 ' "$work/diagonal.out"

"$work/radio_pareto" 3 5 3 >"$work/staircase.out"
awk '/^PARETO CELL / {
    for (i=1; i<=NF; i++) {
        if ($i ~ /^m=/) { split($i,a,"="); m=a[2] }
        if ($i ~ /^max_n1=/) { split($i,a,"="); n=a[2] }
    }
    print m, n
}' "$work/staircase.out" | sort -n >"$work/staircase.cells"
printf '1 8\n2 7\n3 5\n' >"$work/expected-staircase.cells"
cmp "$work/expected-staircase.cells" "$work/staircase.cells"

# A zero wall cap is genuinely unbounded, while the memory monitor remains active.
tools/capped_run.sh --seconds 0 --rss-gb 1 --poll 1 --label no-wall-cap -- \
    sh -c 'exit 0' >/dev/null 2>"$work/capped.err"
grep -q 'no-wall-cap: completed | exit 0' "$work/capped.err"

echo 'pareto walk regression: diagonal bootstrap, staircase, and no-wall-cap runner OK'
