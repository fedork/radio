#!/usr/bin/env bash
# Lock the exact hereditary-suffix scan and the two cheap pliability certificates used by the
# standalone rb-root diagnostic.  These probes do not recursively solve any child.
set -euo pipefail
cd "$(dirname "$0")/.."

tmp=$(mktemp -d "${TMPDIR:-/tmp}/radio-rb-pliability.XXXXXX")
trap 'rm -rf "$tmp"' EXIT

python3 tools/build_radio.py -O3 -DMAX_K=5 -DMAX_N=157 \
    tools/rb_root_probe.c -o "$tmp/rb_root_probe" >/dev/null

# Thirteen retained (2:1) parts have mass 26 against three child caps of 9: absolute slack is one.
# Every suffix is pliable, and both the direct extension theorem and its q/D corollary see all of it.
slack_one=$(
    "$tmp/rb_root_probe" 3 \
        2 1 2 1 2 1 2 1 2 1 2 1 2 1 2 1 2 1 2 1 2 1 2 1 2 1 \
        2>/dev/null | grep '^RB_PLIABILITY '
)
grep -F 'slack=1' <<<"$slack_one" >/dev/null
grep -F 'potential_call_suffixes=0 exact_head=0 exact_tail=13' <<<"$slack_one" >/dev/null
grep -F 'theorem_head=0 theorem_tail=13 coarse_head=0 coarse_tail=13' <<<"$slack_one" >/dev/null

# At zero slack the full mass still packs 9/9/9, but the final (2:1) suffix is not universal.  This
# is exactly why root ALIVE and hereditary pliability are different questions.
zero_slack=$(
    "$tmp/rb_root_probe" 3 \
        3 1 2 1 2 1 2 1 2 1 2 1 2 1 2 1 2 1 2 1 2 1 2 1 2 1 \
        2>/dev/null | grep '^RB_PLIABILITY '
)
grep -F 'slack=0 root_pliable=1' <<<"$zero_slack" >/dev/null
grep -F 'potential_call_suffixes=9 exact_head=13 exact_tail=0' <<<"$zero_slack" >/dev/null
grep -F 'theorem_head=13 theorem_tail=0 coarse_head=13 coarse_tail=0' <<<"$zero_slack" >/dev/null

# If slack is at least twice one child cap, every residual capacity is at least the suffix mass, so
# even a non-(2:1) base is certified without constructing a joint suffix theorem.
large_slack=$(
    "$tmp/rb_root_probe" 3 8 1 2>/dev/null | grep '^RB_PLIABILITY '
)
grep -F 'mass=8 cap=9 slack=19 root_pliable=1' <<<"$large_slack" >/dev/null
grep -F 'exact_head=0 exact_tail=1 theorem_head=0 theorem_tail=1' <<<"$large_slack" >/dev/null

# A real hard positive has ample-looking absolute slack but only its final suffix is exact-pliable;
# the corner-extension bound deliberately makes no stronger claim.
hard_state=$(
    "$tmp/rb_root_probe" 5 15 3 14 3 17 2 8 4 11 2 10 2 19 1 15 1 \
        2>/dev/null | grep '^RB_PLIABILITY '
)
grep -F 'parts=8 mass=229 cap=81 slack=14' <<<"$hard_state" >/dev/null
grep -F 'potential_call_suffixes=5 exact_head=7 exact_tail=1' <<<"$hard_state" >/dev/null
grep -F 'theorem_head=8 theorem_tail=0 coarse_head=8 coarse_tail=0' <<<"$hard_state" >/dev/null

# Retaining the full absolute slack strengthens the old q/D corollary.  Here sigma=5: the former
# slack>=2 form certifies only the final (2:1), while 2*(D-q)<=sigma-4 certifies two suffix parts.
full_slack=$(
    "$tmp/rb_root_probe" 3 4 3 2 2 2 2 2 1 \
        2>/dev/null | grep '^RB_PLIABILITY '
)
grep -F 'slack=5' <<<"$full_slack" >/dev/null
grep -F 'coarse_head=3 coarse_tail=1' <<<"$full_slack" >/dev/null
grep -F 'slack_excess_head=2 slack_excess_tail=2' <<<"$full_slack" >/dev/null

echo "rb pliability regression passed"
