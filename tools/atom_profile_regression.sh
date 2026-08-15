#!/usr/bin/env bash
set -euo pipefail

repo_dir=$(cd "$(dirname "$0")/.." && pwd)
work_dir=$(mktemp -d "${TMPDIR:-/tmp}/radio-atom-profile.XXXXXX")
trap 'rm -rf "$work_dir"' EXIT

cd "$repo_dir"
tools/check_atom_parent_formula.py
tools/check_atom_profile_certificate.py evidence/atom_profile_height6_ad8.cert
tools/check_atom_profile_tree.py evidence/atom_profile_height6_ad8.cert
tools/check_dc_kernel_certificate.py evidence/atom_profile_height6_dc16.cert
tools/check_dc_kernel_certificate.py evidence/atom_profile_height6_dc32.cert
tools/check_dc_tree_lift.py evidence/atom_profile_height6_dc16.cert \
    --rank 305 --expect NO
tools/check_atom_profile_tree.py evidence/atom_profile_height6_rank305.cert
tools/check_dc_tree_lift.py --atoms 32 --rank 1180 --all-skeletons \
    --depth 3 --expect NO > "$work_dir/rank1180-depth3-independent.out"
rg -q 'rank=1180 .*answer=NO scope=all_skeletons depth=3 .*supply_loss_rejects=[1-9][0-9]*' \
    "$work_dir/rank1180-depth3-independent.out"
tools/check_dc_tree_lift.py --atoms 32 --rank 1180 --depth 4 --pure-frontier \
    --close-positive-v-loss > "$work_dir/rank1180-depth4-frontier-independent.out"
rg -q 'pure_frontier rank=1180 depth=4 local_options=24,86,150 products=7266 .*survivors=6712 unique_mixed=1826' \
    "$work_dir/rank1180-depth4-frontier-independent.out"
rg -q 'pure_frontier_loss loss=0,0,1 candidates=108 unique_mixed=32' \
    "$work_dir/rank1180-depth4-frontier-independent.out"
rg -q 'pure_frontier_loss loss=0,0,14 candidates=136 unique_mixed=36' \
    "$work_dir/rank1180-depth4-frontier-independent.out"
rg -q 'pure_frontier_loss loss=0,2,10 candidates=4 unique_mixed=2' \
    "$work_dir/rank1180-depth4-frontier-independent.out"
test "$(rg -c '^pure_frontier_loss ' \
    "$work_dir/rank1180-depth4-frontier-independent.out")" -eq 17
if rg -q '^pure_frontier_loss loss=0,1,' \
    "$work_dir/rank1180-depth4-frontier-independent.out"; then
    echo 'rank-1180 frontier unexpectedly retained a V-loss-one class' >&2
    exit 1
fi
rg -q 'pure_frontier_closed_summary selected_states=8 yes=0 no=8' \
    "$work_dir/rank1180-depth4-frontier-independent.out"

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
rg -q 'height6_max_proof root_base_threshold=12 maximal_at_requested_depth=YES maximal_all_depth=YES' \
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

CC=clang++ tools/build_radio.py -O3 -std=c++20 -Wall -Wextra -pedantic \
    -DATOM_PROFILE_ATOMS=32 tools/search_atom_profiles.cpp \
    -o "$work_dir/search_atom_profiles_32"

run_profile_32() {
    tools/run_with_provenance.py "$work_dir/search_atom_profiles_32" "$@"
}

run_profile_32 profile-state 0 AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA:1 \
    > "$work_dir/singleton32.out"
tools/check_atom_profile_tree.py "$work_dir/singleton32.out"

if run_profile_32 profile-state 3 \
    AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB:1 \
    AAAAAAAAAAAAAAAAAAAAAAAAAAAABBCC:3 \
    > "$work_dir/mixed-supply32.out"; then
    echo '32-atom mixed-supply control unexpectedly solved within depth 3' >&2
    exit 1
else
    status=$?
    if [[ $status -ne 1 ]]; then
        echo "32-atom mixed-supply control aborted with status $status" >&2
        exit "$status"
    fi
fi
rg -q 'answer=NO .*profile_atoms=32 .*supply_rejects=1' \
    "$work_dir/mixed-supply32.out"
rg -q 'atom_profile_depth_obstruction=mixed_supply depth=3 supply_upper=0,2,11 required=0,2,13' \
    "$work_dir/mixed-supply32.out"

# The unique open 32-atom boundary profile is not blocked by the scalar supply bound, but an
# exhaustive complete-product search rules out every aligned tree of depth at most three.  This is
# deliberately a bounded negative: deeper synchronized trees remain open.
if run_profile_32 profile-state-flat 3 \
    AAAAAAAAAAAAAAAAAAAAAAAAAAABBBBC:1 \
    AAAAAAAAAAAAAAAAAAAAAAABBBBBBBCC:2 \
    AAAAAAAAAAAAAAAAAAAAAAAAAAACCCDD:3 \
    > "$work_dir/rank1180-depth3.out"; then
    echo '32-atom rank 1180 unexpectedly solved within depth 3' >&2
    exit 1
else
    status=$?
    if [[ $status -ne 1 ]]; then
        echo "32-atom rank 1180 depth-3 search aborted with status $status" >&2
        exit "$status"
    fi
fi
rg -q 'answer=NO .*profile_atoms=32 .*supply_loss_rejects=[1-9][0-9]*' \
    "$work_dir/rank1180-depth3.out"
if rg -q 'atom_profile_depth_obstruction=' "$work_dir/rank1180-depth3.out"; then
    echo 'rank-1180 depth-3 result was only a root obstruction, not exhaustive' >&2
    exit 1
fi

# At depth four, exact solution of both pure children leaves a finite mixed-child frontier.  The
# independent Python implementation above reproduces these counts and exhausts the eight states
# that spend the available V slack.
run_profile_32 profile-state-pure-frontier-dc-kernel 4 \
    evidence/atom_profile_height6_dc32.cert \
    AAAAAAAAAAAAAAAAAAAAAAAAAAABBBBC:1 \
    AAAAAAAAAAAAAAAAAAAAAAABBBBBBBCC:2 \
    AAAAAAAAAAAAAAAAAAAAAAAAAAACCCDD:3 \
    > "$work_dir/rank1180-depth4-frontier.out" 2>&1
rg -q 'cover_root_pure_frontier candidates=6712 unique_mixed_children=1826 loss_classes=17' \
    "$work_dir/rank1180-depth4-frontier.out"
rg -q 'cover_root_loss loss=0,2,12 candidates=6 unique_mixed_children=3' \
    "$work_dir/rank1180-depth4-frontier.out"

: > "$work_dir/rank1180-depth4-vtight.out"
while IFS='|' read -r first second third; do
    if run_profile_32 profile-state-flat-dc-kernel 3 \
        evidence/atom_profile_height6_dc32.cert "$first" "$second" "$third" \
        >> "$work_dir/rank1180-depth4-vtight.out" 2>/dev/null; then
        echo 'rank-1180 V-tight mixed child unexpectedly solved within depth 3' >&2
        exit 1
    else
        status=$?
        if [[ $status -ne 1 ]]; then
            echo "rank-1180 V-tight mixed-child search aborted with status $status" >&2
            exit "$status"
        fi
    fi
done <<'EOF'
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAABC:1|AAAAAAAAAAAAAAAAAAAAAABBBBBBBBCC:2|AAAAAAAAAAAAAAAAAAAAAAAAAAACCCDD:3
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAC:1|AAAAAAAAAAAAAAAAAAAAAABBBBBBBBCC:2|AAAAAAAAAAAAAAAAAAAAAAAAAABCCCDD:3
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAABC:1|AAAAAAAAAAAAAAAAAAAAAAABBBBBBBCC:2|AAAAAAAAAAAAAAAAAAAAAAAAAAACCCDD:3
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAC:1|AAAAAAAAAAAAAAAAAAAAAABBBBBBBBCC:2|AAAAAAAAAAAAAAAAAAAAAAAAAAACCCDD:3
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAC:1|AAAAAAAAAAAAAAAAAAAAAAABBBBBBBCC:2|AAAAAAAAAAAAAAAAAAAAAAAAAABCCCDD:3
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAABC:1|AAAAAAAAAAAAAAAAAAAAAAAABBBBBBCC:2|AAAAAAAAAAAAAAAAAAAAAAAAAAACCCDD:3
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAC:1|AAAAAAAAAAAAAAAAAAAAAAABBBBBBBCC:2|AAAAAAAAAAAAAAAAAAAAAAAAAAACCCDD:3
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAC:1|AAAAAAAAAAAAAAAAAAAAAAAABBBBBBCC:2|AAAAAAAAAAAAAAAAAAAAAAAAAABCCCDD:3
EOF
test "$(rg -c '^atom_profile depth=3 answer=NO' \
    "$work_dir/rank1180-depth4-vtight.out")" -eq 8

# A two-part child that traps prefix recursion has a one-test exact construction.  Keep this as a
# positive control for the complete-product path used by the depth-three boundary search.
run_profile_32 profile-state-flat 3 \
    AAAAAAAAAAAAAAAAAAAAABBBBBBBBBCC:1 \
    AAAAAAAAAAAAAAAAAAAAAAABBCCCCCDD:2 \
    > "$work_dir/two-part-flat32.out"
rg -q 'answer=YES .*profile_atoms=32' "$work_dir/two-part-flat32.out"
tools/check_atom_profile_tree.py "$work_dir/two-part-flat32.out"

echo 'atom profile regression passed'
