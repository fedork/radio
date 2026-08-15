#!/usr/bin/env bash
set -euo pipefail

repo_dir=$(cd "$(dirname "$0")/.." && pwd)
work_dir=$(mktemp -d "${TMPDIR:-/tmp}/radio-atom-profile.XXXXXX")
trap 'rm -rf "$work_dir"' EXIT

cd "$repo_dir"
tools/check_atom_profile_certificate.py evidence/atom_profile_height6_ad8.cert
tools/check_atom_profile_tree.py evidence/atom_profile_height6_ad8.cert
tools/check_dc_kernel_certificate.py evidence/atom_profile_height6_dc16.cert

CC=clang++ tools/build_radio.py -O3 -std=c++20 -Wall -Wextra -pedantic \
    tools/search_atom_profiles.cpp -o "$work_dir/search_atom_profiles"

run_profile() {
    tools/run_with_provenance.py "$work_dir/search_atom_profiles" "$@"
}

run_profile height6-lineage-certificate > "$work_dir/lineage.cert"
tools/check_atom_profile_certificate.py "$work_dir/lineage.cert"
rg -q 'candidate_rank_last=81 .*target_rank=59 .*next_rank=82 next=AAAAAADD' \
    "$work_dir/lineage.cert"

run_profile height4-control 1 > "$work_dir/height4.out"
rg -q 'atom_profile depth=1 answer=YES' "$work_dir/height4.out"
rg -q 'atom_profile_proof root_base_threshold=' "$work_dir/height4.out"
tools/check_atom_profile_tree.py "$work_dir/height4.out"

run_profile height5-control 2 > "$work_dir/height5.out"
rg -q 'atom_profile depth=1 answer=NO' "$work_dir/height5.out"
rg -q 'atom_profile depth=2 answer=YES' "$work_dir/height5.out"
tools/check_atom_profile_tree.py "$work_dir/height5.out"

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
rg -q 'atom_profile_all_depth_obstruction=d_lineage d_lineages=1 required=2' \
    "$work_dir/height6.out"

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
rg -q 'atom_profile_all_depth_obstruction=d_lineage' "$work_dir/literal-core.out"

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
rg -q 'rank=81 .*all_depth_obstruction=d_lineage' "$work_dir/height6-rank.out"
rg -q 'rank=82 D=AAAAAADD .*all_depth_obstruction=NO' "$work_dir/height6-rank.out"
rg -q 'height6_max_result feasible=NONE depth=1 first_rank=1 last_rank=165 range_complete=YES' \
    "$work_dir/height6-rank.out"

if run_profile height6-max 2 82 82 > "$work_dir/height6-rank82-depth2.out"; then
    echo 'height6 rank 82 unexpectedly solved within depth 2' >&2
    exit 1
else
    status=$?
    if [[ $status -ne 1 ]]; then
        echo "height6 rank 82 depth-2 search aborted with status $status" >&2
        exit "$status"
    fi
fi
rg -q 'rank=82 D=AAAAAADD .*answer=NO depth=2 .*all_depth_obstruction=NO' \
    "$work_dir/height6-rank82-depth2.out"

run_profile height6-max 3 1 82 > "$work_dir/height6-symbolic-max.out"
rg -q 'rank=82 D=AAAAAADD .*answer=YES depth=3 ' "$work_dir/height6-symbolic-max.out"
rg -q 'height6_max_proof root_base_threshold=13 maximal_at_requested_depth=YES maximal_all_depth=YES' \
    "$work_dir/height6-symbolic-max.out"
tools/check_atom_profile_tree.py "$work_dir/height6-symbolic-max.out"

CC=clang++ tools/build_radio.py -O3 -std=c++20 -Wall -Wextra -pedantic \
    -DATOM_PROFILE_ATOMS=16 -DATOM_PROFILE_MAX_MEMO=3000000 \
    tools/search_atom_profiles.cpp -o "$work_dir/search_atom_profiles_16"

run_profile_16() {
    tools/run_with_provenance.py "$work_dir/search_atom_profiles_16" "$@"
}

run_profile_16 height6-lineage-certificate > "$work_dir/lineage16.cert"
tools/check_atom_profile_certificate.py "$work_dir/lineage16.cert"
rg -q 'profile_atoms=16 .*candidate_rank_last=289 .*target_rank=191 .*next_rank=290' \
    "$work_dir/lineage16.cert"

run_profile_16 height6-dc-kernel-certificate > "$work_dir/dc-kernel16.cert"
tools/check_dc_kernel_certificate.py "$work_dir/dc-kernel16.cert"
diff \
    <(rg '^(dc_kernel_certificate|dc_kernel_state)' \
        evidence/atom_profile_height6_dc16.cert) \
    <(rg '^(dc_kernel_certificate|dc_kernel_state)' "$work_dir/dc-kernel16.cert")

if run_profile_16 height6-max 5 290 290 > "$work_dir/height6-rank290-depth5.out"; then
    echo 'sixteen-atom height6 rank 290 unexpectedly solved within depth 5' >&2
    exit 1
else
    status=$?
    if [[ $status -ne 1 ]]; then
        echo "sixteen-atom rank 290 depth-5 search aborted with status $status" >&2
        exit "$status"
    fi
fi
rg -q 'rank=290 D=AAAAAAAAAAAAAADD .*answer=NO depth=5 .*dc_rejects=1' \
    "$work_dir/height6-rank290-depth5.out"

run_profile_16 height6-dc 3 305 > "$work_dir/dc-rank305-depth3.out"
rg -Fq 'height6_dc rank=305 D=AAAAAAAAAAAAACDD depth=3 answer=YES state=(0,1):1,(0,2):2,(2,3):3' \
    "$work_dir/dc-rank305-depth3.out"
{
    rg '^(dc_kernel_certificate|dc_kernel_state)' evidence/atom_profile_height6_dc16.cert
    rg '^dc_tree' "$work_dir/dc-rank305-depth3.out"
} > "$work_dir/dc-combined16.cert"
tools/check_dc_kernel_certificate.py "$work_dir/dc-combined16.cert"
diff \
    <(rg '^dc_tree' evidence/atom_profile_height6_dc16.cert) \
    <(rg '^dc_tree' "$work_dir/dc-rank305-depth3.out")

echo 'atom profile regression passed'
