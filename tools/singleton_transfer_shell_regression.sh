#!/usr/bin/env bash
set -euo pipefail

repo_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
shell_tmp=$(mktemp -d "${TMPDIR:-/tmp}/radio-singleton-shell.XXXXXX")

cleanup_shell_tmp() {
    if [[ -n "${shell_tmp:-}" && -d "$shell_tmp" ]]; then
        rm -rf -- "$shell_tmp"
    fi
}
trap cleanup_shell_tmp EXIT

cd "$repo_dir"
CC="${CXX:-clang++}" tools/build_radio.py \
    -std=c++20 -O3 -Wall -Wextra -pedantic \
    tools/singleton_pair_coloring_census.cpp \
    -o "$shell_tmp/singleton-shell"

tools/run_with_provenance.py "$shell_tmp/singleton-shell" \
    --transfer-shell-counts 5 115 > "$shell_tmp/k5-counts.log"
rg -q \
    'TRANSFER_SHELL_COUNT_CHECK K=5 maximum_distance=115 shells=116 states=1431800647444 .* verified=YES' \
    "$shell_tmp/k5-counts.log"

tools/run_with_provenance.py "$shell_tmp/singleton-shell" \
    --transfer-shell-counts 6 14 > "$shell_tmp/k6-counts.log"
rg -q \
    'TRANSFER_SHELL_COUNT_CHECK K=6 maximum_distance=14 shells=15 states=15150098684 .* verified=YES' \
    "$shell_tmp/k6-counts.log"

tools/run_with_provenance.py "$shell_tmp/singleton-shell" \
    --transfer-ball-parallel 3 8 4 > "$shell_tmp/k3-ball.log"
rg -q \
    'TRANSFER_BALL_PARALLEL_COLORING_CENSUS K=3 maximum_distance=8 .* complete=YES verified=YES states=160 ' \
    "$shell_tmp/k3-ball.log"

tools/run_with_provenance.py "$shell_tmp/singleton-shell" \
    --prefix-cylinder-parallel 3 3 16 1 1 6 > "$shell_tmp/k3-prefix.log"
rg -q \
    'PREFIX_CYLINDER_PARALLEL_CENSUS K=3 workers=3 complete=YES verified=YES total_states=160 covered_states=160 tested_leaves=0 ' \
    "$shell_tmp/k3-prefix.log"

tools/run_with_provenance.py "$shell_tmp/singleton-shell" \
    --prefix-cylinder-parallel 4 12 16 2 4 12 > "$shell_tmp/k4-prefix.log"
rg -q \
    'PREFIX_CYLINDER_PARALLEL_CENSUS K=4 workers=12 complete=YES verified=YES total_states=408776 covered_states=408772 tested_leaves=4 ' \
    "$shell_tmp/k4-prefix.log"

tools/run_with_provenance.py "$shell_tmp/singleton-shell" \
    --transfer-shell-parallel 6 7 4 > "$shell_tmp/k6-shell7.log"
rg -q \
    'TRANSFER_SHELL_PARALLEL_COLORING_CENSUS K=6 distance=7 .* complete=YES verified=YES expected_states=1980479 tested=1980479 .* exact_fallbacks=836 exact_nodes=12239 ' \
    "$shell_tmp/k6-shell7.log"

tools/run_with_provenance.py "$shell_tmp/singleton-shell" \
    --transfer-shell 6 13 1000 3000000000 > "$shell_tmp/k6-window.log"
rg -q \
    'TRANSFER_SHELL_COLORING_CENSUS K=6 distance=13 .* verified=YES expected_states=3494418291 counted_states=3494418291 .* seen=3000001000 skipped=3000000000 tested=1000 ' \
    "$shell_tmp/k6-window.log"

tools/run_with_provenance.py "$shell_tmp/singleton-shell" \
    --transfer-shell 5 115 > "$shell_tmp/k5-last-shell.log"
rg -q \
    'TRANSFER_SHELL_COLORING_CENSUS K=5 distance=115 complete=YES verified=YES expected_states=82 counted_states=82 .* tested=82 lookahead_ok=82 exact_fallbacks=0 ' \
    "$shell_tmp/k5-last-shell.log"

canonical=(
    64 63 57 57 42 42 42 42
    22 22 22 22 22 22 22 22
    7 7 7 7 7 7 7 7 7 7 7 7 7 7 7 7
    1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
    1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
)
j13=(
    64 63 57 57 42 42 42 42
    22 22 22 22 22 22 22 9
    8 8 8 8 8 8 8 8 8 8 8 8 8
    7 7 7
    1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
    1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
)
j14=(
    64 63 57 57 42 42 42 42
    22 22 22 22 22 22 22
    8 8 8 8 8 8 8 8 8 8 8 8 8 8 8
    7 7
    1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
    1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
)

tools/run_with_provenance.py "$shell_tmp/singleton-shell" \
    --general-coloring-case 6 "${canonical[@]}" > "$shell_tmp/canonical.log"
rg -q 'GENERAL_COLORING_CASE K=6 .* lookahead=YES feasible=YES ' \
    "$shell_tmp/canonical.log"

tools/run_with_provenance.py "$shell_tmp/singleton-shell" \
    --general-coloring-case 6 "${j13[@]}" > "$shell_tmp/j13.log"
rg -q 'GENERAL_COLORING_CASE K=6 .* lookahead=YES feasible=YES ' \
    "$shell_tmp/j13.log"

tools/run_with_provenance.py "$shell_tmp/singleton-shell" \
    --general-coloring-case 6 "${j14[@]}" > "$shell_tmp/j14.log"
rg -q 'GENERAL_COLORING_CASE K=6 .* lookahead=NO feasible=NO exact_nodes=7 ' \
    "$shell_tmp/j14.log"

rg '^TRANSFER_BALL_PARALLEL_COLORING_CENSUS' "$shell_tmp/k3-ball.log"
rg '^PREFIX_CYLINDER_PARALLEL_CENSUS' \
    "$shell_tmp/k3-prefix.log" "$shell_tmp/k4-prefix.log"
rg '^TRANSFER_SHELL_COUNT_CHECK' \
    "$shell_tmp/k5-counts.log" "$shell_tmp/k6-counts.log"
rg '^TRANSFER_SHELL_PARALLEL_COLORING_CENSUS' "$shell_tmp/k6-shell7.log"
rg '^TRANSFER_SHELL_COLORING_CENSUS' \
    "$shell_tmp/k6-window.log" "$shell_tmp/k5-last-shell.log"
rg '^GENERAL_COLORING_CASE' \
    "$shell_tmp/canonical.log" "$shell_tmp/j13.log" "$shell_tmp/j14.log"
