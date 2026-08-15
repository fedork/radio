#!/usr/bin/env bash
set -euo pipefail

repo_dir=$(cd "$(dirname "$0")/.." && pwd)
work_dir=$(mktemp -d "${TMPDIR:-/tmp}/radio-atom-profile.XXXXXX")
trap 'rm -rf "$work_dir"' EXIT

cd "$repo_dir"
CC=clang++ tools/build_radio.py -O3 -std=c++20 -Wall -Wextra -pedantic \
    tools/search_atom_profiles.cpp -o "$work_dir/search_atom_profiles"

run_profile() {
    tools/run_with_provenance.py "$work_dir/search_atom_profiles" "$@"
}

run_profile height4-control 1 > "$work_dir/height4.out"
rg -q 'atom_profile depth=1 answer=YES' "$work_dir/height4.out"
rg -q 'atom_profile_proof root_base_threshold=' "$work_dir/height4.out"

run_profile height5-control 2 > "$work_dir/height5.out"
rg -q 'atom_profile depth=1 answer=NO' "$work_dir/height5.out"
rg -q 'atom_profile depth=2 answer=YES' "$work_dir/height5.out"

if run_profile height6 2 > "$work_dir/height6.out"; then
    echo 'height6 ABBBBBCD unexpectedly solved within depth 2' >&2
    exit 1
else
    status=$?
    if [[ $status -ne 1 ]]; then
        echo "height6 ABBBBBCD aborted with status $status" >&2
        exit "$status"
    fi
fi
rg -q 'atom_profile depth=2 answer=NO' "$work_dir/height6.out"

if run_profile height6-literal-core 1 > "$work_dir/literal-core.out"; then
    echo 'height6 literal core unexpectedly solved at depth 1' >&2
    exit 1
else
    status=$?
    if [[ $status -ne 1 ]]; then
        echo "height6 literal core aborted with status $status" >&2
        exit "$status"
    fi
fi
rg -q 'atom_profile depth=1 answer=NO' "$work_dir/literal-core.out"

if run_profile height6-max 1 > "$work_dir/height6-rank.out"; then
    echo 'some height6 D profile unexpectedly solved at depth 1' >&2
    exit 1
else
    status=$?
    if [[ $status -ne 1 ]]; then
        echo "height6 rank scan aborted with status $status" >&2
        exit "$status"
    fi
fi
rg -q 'rank=45 D=CCCCCCCC .*full_star=NO' "$work_dir/height6-rank.out"
rg -q 'rank=46 D=AAAAAAAD .*full_star=YES' "$work_dir/height6-rank.out"
rg -q 'rank=56 D=AAAABBCD ' "$work_dir/height6-rank.out"
rg -q 'rank=59 D=ABBBBBCD ' "$work_dir/height6-rank.out"
rg -q 'height6_max_result feasible=NONE depth=1 first_rank=1 last_rank=165 range_complete=YES' \
    "$work_dir/height6-rank.out"

echo 'atom profile regression passed'
