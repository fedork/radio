#!/usr/bin/env bash
set -euo pipefail

repo_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
tmp_dir=$(mktemp -d "${TMPDIR:-/tmp}/radio-integrated-k6.XXXXXX")
trap 'rm -rf -- "$tmp_dir"' EXIT
cd "$repo_dir"

tools/build_radio.py -O2 -DMAX_K=3 -DMAX_N=27 -DMAX_PART_N=9 \
    -DRADIO_CACHE_DISABLED_LEVEL=3 -DRADIO_SURVEY_TRACE_STATES \
    radio_singleton_k6_survey.c -o "$tmp_dir/survey-k3"
CC="${CXX:-clang++}" tools/build_radio.py -std=c++20 -O2 \
    tools/singleton_pair_coloring_census.cpp -o "$tmp_dir/ranker"

"$tmp_dir/survey-k3" 3 4 10 7 "$tmp_dir/progress-k3" 3 \
    > "$tmp_dir/integrated-k3.log" 2> "$tmp_dir/integrated-k3.err"
"$tmp_dir/ranker" --transfer-shell-oracle-input 3 4 10 7 0 \
    | awk '$1 == 3 { printf "SURVEY_STATE"; for (i=2; i<=NF; i+=2) printf " %s", $i; print "" }' \
    > "$tmp_dir/ranked-k3.states"
rg '^SURVEY_STATE' "$tmp_dir/integrated-k3.log" > "$tmp_dir/integrated-k3.states"
diff -u "$tmp_dir/ranked-k3.states" "$tmp_dir/integrated-k3.states"
rg -q '^INTEGRATED_SUMMARY queries=7 solvable=7 unsolvable=0 maybe=0 ' \
    "$tmp_dir/integrated-k3.log"
[[ "$(sed -n 's/^queries=//p' "$tmp_dir/progress-k3")" == 7 ]]
[[ "$(sed -n 's/^start_rank=//p' "$tmp_dir/progress-k3")" == 10 ]]
[[ "$(sed -n 's/^absolute_rank=//p' "$tmp_dir/progress-k3")" == 17 ]]
rg -q '^INTEGRATED_CHECKPOINT start=10 end=13 queries=3 solvable=3 unsolvable=0 maybe=0 cumulative_queries=3 cumulative_solvable=3 cumulative_unsolvable=0 cumulative_maybe=0 wall_seconds=' \
    "$tmp_dir/integrated-k3.log"
rg -q '^INTEGRATED_CHECKPOINT start=13 end=16 queries=3 solvable=3 unsolvable=0 maybe=0 cumulative_queries=6 cumulative_solvable=6 cumulative_unsolvable=0 cumulative_maybe=0 wall_seconds=' \
    "$tmp_dir/integrated-k3.log"
rg -q '^INTEGRATED_CHECKPOINT start=16 end=17 queries=1 solvable=1 unsolvable=0 maybe=0 cumulative_queries=7 cumulative_solvable=7 cumulative_unsolvable=0 cumulative_maybe=0 wall_seconds=' \
    "$tmp_dir/integrated-k3.log"
[[ "$(rg -c '^INTEGRATED_CHECKPOINT ' "$tmp_dir/integrated-k3.log")" == 3 ]]
tools/check_provenance.py "$tmp_dir/integrated-k3.log"

tools/build_radio.py -O3 -DMAX_K=6 -DMAX_N=793 -DMAX_PART_N=65 \
    -DRADIO_CACHE_DISABLED_LEVEL=6 \
    radio_singleton_k6_survey.c -o "$tmp_dir/survey-k6"
"$tmp_dir/survey-k6" 6 14 55096 1 "$tmp_dir/progress-k6" \
    > "$tmp_dir/hole-k6.log" 2> "$tmp_dir/hole-k6.err"
rg -q '^VERDICT UNSOLVABLE k=6 .*Sb\(64:1,63:1,57:1,57:1,42:1' \
    "$tmp_dir/hole-k6.log"
rg -q '^INTEGRATED_SUMMARY queries=1 solvable=0 unsolvable=1 maybe=0 ' \
    "$tmp_dir/hole-k6.log"
tools/check_provenance.py "$tmp_dir/hole-k6.log"

rg '^INTEGRATED_SUMMARY' "$tmp_dir/integrated-k3.log" "$tmp_dir/hole-k6.log"
