#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

cache_disabled_tmp=$(mktemp -d "${TMPDIR:-/tmp}/radio-cache-disabled.XXXXXX")
trap 'rm -rf "$cache_disabled_tmp"' EXIT

tools/build_radio.py -O3 -DMAX_K=3 -DMAX_N=24 -DMAX_PART_N=7 \
    -DRADIO_CACHE_DISABLED_LEVEL=3 tools/cache_disabled_level_regression.c \
    -o "$cache_disabled_tmp/cache-disabled-level-regression"

"$cache_disabled_tmp/cache-disabled-level-regression"
