#!/usr/bin/env bash
# Classify a deterministic rank window of the exact-support K=6 transfer shell with the ordinary
# recursive solver.  Parent-level caching is disabled because equal-mass, equal-support parents
# cannot dominate one another; the useful K<=5 child cache remains enabled and shared.
set -euo pipefail

cd "$(dirname "$0")/.."

if (( $# < 3 || $# > 4 )); then
    echo "usage: $0 DISTANCE SKIP LIMIT [PER_QUERY_BUDGET_SECONDS]" >&2
    exit 2
fi
distance=$1
skip=$2
limit=$3
budget=${4:-1}
wall_seconds=${RADIO_SURVEY_SECONDS:-3600}
rss_gb=${RADIO_SURVEY_RSS_GB:-8}
for value in "$distance" "$skip" "$limit" "$budget"; do
    [[ "$value" =~ ^[0-9]+$ ]] || {
        echo "distance, skip, limit and budget must be nonnegative integers" >&2
        exit 2
    }
done
(( limit > 0 )) || { echo "limit must be positive" >&2; exit 2; }

survey_tmp=$(mktemp -d "${TMPDIR:-/tmp}/radio-k6-main-survey.XXXXXX")
trap 'rm -rf "$survey_tmp"' EXIT

CC="${CXX:-clang++}" tools/build_radio.py \
    -std=c++20 -O3 -Wall -Wextra -pedantic \
    tools/singleton_pair_coloring_census.cpp \
    -o "$survey_tmp/shell-ranker" >&2
tools/build_radio.py -O3 -DMAX_K=6 -DMAX_N=793 -DMAX_PART_N=65 \
    -DRADIO_CACHE_DISABLED_LEVEL=6 radio_oracle.c \
    -o "$survey_tmp/oracle" >&2

"$survey_tmp/shell-ranker" \
    --transfer-shell-oracle-input 6 "$distance" "$skip" "$limit" "$budget" \
    | tools/capped_run.sh --seconds "$wall_seconds" --rss-gb "$rss_gb" \
        --label k6-main-solver-survey -- "$survey_tmp/oracle"
