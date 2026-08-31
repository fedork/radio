#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

cache_regression_tmp=$(mktemp -d "${TMPDIR:-/tmp}/radio-cache-upward.XXXXXX")
trap 'rm -rf "$cache_regression_tmp"' EXIT

tools/build_radio.py -O3 -DMAX_K=3 -DMAX_N=24 -DMAX_PART_N=7 \
    tools/cache_upward_closure_regression.c \
    -o "$cache_regression_tmp/cache-upward-regression"

"$cache_regression_tmp/cache-upward-regression"
