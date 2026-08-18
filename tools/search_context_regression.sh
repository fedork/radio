#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

tmp=$(mktemp -d "${TMPDIR:-/tmp}/radio-search-context.XXXXXX")
trap 'rm -rf "$tmp"' EXIT

tools/build_radio.py -O2 -DMAX_K=3 -DMAX_N=20 \
    tools/search_context_regression.c -o "$tmp/work" >/dev/null
"$tmp/work" | grep -F 'search context regression passed' >/dev/null

tools/build_radio.py -O2 -DMAX_K=3 -DMAX_N=20 -DRADIO_CPU_BUDGET \
    tools/search_context_regression.c -o "$tmp/cpu" >/dev/null
"$tmp/cpu" | grep -F 'search context regression passed' >/dev/null

echo "search context regression passed (work and CPU schedulers)"
