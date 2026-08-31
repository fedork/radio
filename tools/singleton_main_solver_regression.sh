#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

solver_regression_tmp=$(mktemp -d "${TMPDIR:-/tmp}/radio-singleton-main.XXXXXX")
trap 'rm -rf "$solver_regression_tmp"' EXIT

tools/build_radio.py -O3 -DMAX_K=6 -DMAX_N=793 -DMAX_PART_N=65 -DRADIO_INIT_PROFILE \
    tools/singleton_main_solver_regression.c \
    -o "$solver_regression_tmp/singleton-main-regression"

"$solver_regression_tmp/singleton-main-regression"
