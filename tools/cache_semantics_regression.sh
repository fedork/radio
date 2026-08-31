#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

cache_semantics_tmp=$(mktemp -d "${TMPDIR:-/tmp}/radio-cache-semantics.XXXXXX")
trap 'rm -rf "$cache_semantics_tmp"' EXIT

tools/build_radio.py -O2 -DMAX_K=2 -DMAX_N=13 -DMAX_PART_N=5 \
    tools/cache_semantics_regression.c -o "$cache_semantics_tmp/cache-semantics"

"$cache_semantics_tmp/cache-semantics" \
    tools/testdata/cache_semantics_untrusted.cache \
    tools/testdata/cache_semantics_trusted.cache
