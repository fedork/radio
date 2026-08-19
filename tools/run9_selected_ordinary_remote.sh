#!/usr/bin/env bash
# Run the ORDINARY (uncolored) level-v2 refuter over the post-coloring selected input.
#
# SUPERSEDED for new work by tools/run9_level_chain_verify_remote.sh, which takes the chain and its
# expected counts from an uploaded manifest instead of pinning one input set inline. This file is
# kept only because run 20260819T013030Z was launched from it and its exact content is that run's
# provenance.
#
# This is an A/B measurement, not a new proof. Three points, all on c8a.4xlarge with 16 threads:
#
#   complete input + ordinary verifier   211,335.569 CPU s   (run 20260818T194508Z)
#   selected input + colored verifier    218,792.627 CPU s   (run 20260818T205010Z)
#   selected input + ordinary verifier   <-- this run
#
# The third point separates citation-tracing overhead from the benefit of verifying fewer claims.
# The prediction, from scaling each level's measured cost by its claim reduction, is about
# 205,111 CPU s -- roughly 3% below the complete replay, because k=7 is 99.2% of the cost and
# coloring only removed 2.7% of its claims. A result at or above the complete replay's cost is a
# meaningful negative: it would mean the claims coloring dropped were the cheap ones.
#
# It deliberately reuses the exact `run9_refute` binary and /root/source tree from the completed
# uncolored run, so the only thing that differs from the baseline is the input file. Do not
# rebuild: a fresh build would confound the comparison.
#
# The selected level certificates are pulled from the colored run's S3 prefix, which outlives its
# terminated instance, and every download is checked against a pinned SHA-256.
set -euo pipefail

BUCKET=radio-sa193-393287594714
SELECTED_PREFIX=run9-colored-refute/20260818T205010Z/levels
BASELINE_WORK=/root/run9-frozen-refute-20260818T194508Z
SOURCE_DIR=/root/source
BINARY=run9_refute
WORK=
PREFIX=
RUN_ID=
THREADS=16
CPUSET=0-15
RSS_GIB=8
PHASE_SECONDS=86400
PROGRESS_SECONDS=60

# Cheap levels first as a gate, then the dominant k=7 phase.
REPLAY_ORDER=(2 3 4 9 8 5 6 7)
LEVELS=(2 3 4 5 6 7 8 9)
TOTAL_CLAIMS=2846568

# Selected claim counts, from the colored run's color-chain.tsv `audited` column.
expected_claims() {
    case $1 in
        2) echo 2 ;;      3) echo 127 ;;     4) echo 24635 ;;   5) echo 80634 ;;
        6) echo 230725 ;; 7) echo 2508278 ;; 8) echo 2151 ;;     9) echo 16 ;;
        *) return 1 ;;
    esac
}

# Pinned SHA-256 of each compressed selected level certificate.
expected_zst_sha() {
    case $1 in
        2) echo f0d768d4a88a306067ac07d1fceebc9efa82e158b33083a9b36d47d5184a527c ;;
        3) echo 04a8ee0fbff9388389d84d88ec512f651aa3cc609ee72482870047fc592541e0 ;;
        4) echo 1f4e30ebf61d7a538cf19555c393c4410f64dd9a4ec9f4c5a63890283bf5672c ;;
        5) echo 8744a7002d1b140372232932de23fce74b5a8b9c4501bf6a57ccfc8b62ecfca4 ;;
        6) echo a94a1f114c6995108a860b2eacbd830af58d6139c7f28d3197c53c382e4c9284 ;;
        7) echo 475c11a321f56b83620ef9376c0f10cb83d178c7c1a3d8da0f91c36a2d8dd049 ;;
        8) echo e34246f592f3a85c11e1f6d2358b89b0127ca094d8a976e8c89f89709362fddd ;;
        9) echo fffc0a6568f199e8c1635a1d698b2ebb1b28408d474632cbb65e386f0f7e86d4 ;;
        *) return 1 ;;
    esac
}

usage() { echo "usage: $0 --run-id ID [--threads N] [--cpuset SET] [--rss-gib N]" >&2; exit 64; }

while (( $# )); do
    case "$1" in
        --run-id) RUN_ID=$2; shift 2 ;;
        --threads) THREADS=$2; shift 2 ;;
        --cpuset) CPUSET=$2; shift 2 ;;
        --rss-gib) RSS_GIB=$2; shift 2 ;;
        --phase-seconds) PHASE_SECONDS=$2; shift 2 ;;
        --progress-seconds) PROGRESS_SECONDS=$2; shift 2 ;;
        *) usage ;;
    esac
done
[[ "$RUN_ID" =~ ^[0-9]{8}T[0-9]{6}Z$ ]] || usage
WORK=/root/run9-selected-ordinary-$RUN_ID
PREFIX=run9-selected-ordinary/$RUN_ID

# Refuse to run if the baseline artifacts are not exactly where the comparison requires.
[[ -x "$BASELINE_WORK/$BINARY" ]] || { echo "missing baseline binary $BASELINE_WORK/$BINARY" >&2; exit 70; }
[[ -f "$BASELINE_WORK/$BINARY.provenance" ]] || { echo "missing baseline provenance" >&2; exit 70; }
[[ -d "$SOURCE_DIR/tools" ]] || { echo "missing $SOURCE_DIR/tools" >&2; exit 70; }

mkdir -p "$WORK"
cd "$WORK"
cp -n "$BASELINE_WORK/$BINARY" "$BASELINE_WORK/$BINARY.provenance" .
chmod +x "./$BINARY"

instance_id=$(TOKEN=$(curl -fsS -X PUT -H 'X-aws-ec2-metadata-token-ttl-seconds: 21600' \
    http://169.254.169.254/latest/api/token) && \
    curl -fsS -H "X-aws-ec2-metadata-token: $TOKEN" \
    http://169.254.169.254/latest/meta-data/instance-id)

{
    printf 'started_utc=%s\n' "$(date -u +%FT%TZ)"
    printf 'run_id=%s\ninstance_id=%s\nwork=%s\ns3_prefix=s3://%s/%s/\n' \
        "$RUN_ID" "$instance_id" "$WORK" "$BUCKET" "$PREFIX"
    printf 'purpose=ordinary verifier over post-coloring selected input; A/B only, not a proof\n'
    printf 'binary_reused_from=%s\n' "$BASELINE_WORK"
    printf 'binary_sha256=%s\n' "$(sha256sum "./$BINARY" | cut -d' ' -f1)"
    printf 'selected_input_prefix=s3://%s/%s/\n' "$BUCKET" "$SELECTED_PREFIX"
    printf 'threads=%s\ncpuset=%s\nrss_cap_gib=%s\nphase_wall_cap_seconds=%s\n' \
        "$THREADS" "$CPUSET" "$RSS_GIB" "$PHASE_SECONDS"
    printf 'total_selected_claims=%s\n' "$TOTAL_CLAIMS"
    printf 'baseline_complete_ordinary_cpu_seconds=211335.569\n'
    printf 'baseline_selected_colored_cpu_seconds=218792.627\n'
    printf 'predicted_cpu_seconds=205111\n'
} > run.meta

printf 'SETUP\n' > stage
upload_state() {
    aws s3 cp run.meta "s3://$BUCKET/$PREFIX/run.meta" --no-progress >/dev/null 2>&1 || true
    [[ -f STATUS ]] && aws s3 cp STATUS "s3://$BUCKET/$PREFIX/STATUS" --no-progress >/dev/null 2>&1 || true
    [[ -f timings.tsv ]] && aws s3 cp timings.tsv "s3://$BUCKET/$PREFIX/timings.tsv" --no-progress >/dev/null 2>&1 || true
    return 0
}

write_status() {
    {
        printf 'run9 ordinary verifier over post-coloring selected input\n'
        printf 'updated_utc=%s\n' "$(date -u +%FT%TZ)"
        printf 'run_id=%s\ninstance_id=%s\n' "$RUN_ID" "$instance_id"
        printf 'stage=%s\n' "$(cat stage)"
        for lv in "${LEVELS[@]}"; do
            [[ -f "verify-k$lv.total" ]] && printf 'completed_k%s=%s\n' "$lv" "$(cat "verify-k$lv.total")"
        done
        [[ -f timings.tsv ]] && sed 's/^/timing=/' timings.tsv
        awk '/^MemAvailable:/ {printf "host_mem_available_gib=%.1f\n", $2/1048576}' /proc/meminfo
        df -Pk "$WORK" | awk 'NR==2 {printf "disk_available_gib=%.1f\n", $4/1048576}'
    } > STATUS
    upload_state
}

# Stage every selected level certificate, hash-checked, before any verification starts.
printf 'FETCH\n' > stage; write_status
for level in "${LEVELS[@]}"; do
    want=$(expected_zst_sha "$level")
    aws s3 cp "s3://$BUCKET/$SELECTED_PREFIX/run9-k$level.cert.zst" "run9-k$level.cert.zst" --no-progress
    got=$(sha256sum "run9-k$level.cert.zst" | cut -d' ' -f1)
    if [[ "$got" != "$want" ]]; then
        echo "level $level: selected certificate sha256 mismatch (want $want got $got)" >&2
        exit 71
    fi
    zstd -dq -f "run9-k$level.cert.zst" -o "run9-k$level.cert"
done
printf 'level\tclaims\twall_seconds\tcpu_seconds\n' > timings.tsv

run_phase() {
    local label=$1 out=$2 err=$3
    shift 3
    "$SOURCE_DIR/tools/capped_run.sh" --seconds "$PHASE_SECONDS" --rss-gb "$RSS_GIB" --label "$label" -- \
        taskset -c "$CPUSET" "$@" > "$out" 2> "$err" &
    local wrapper=$!
    printf '%s\n' "$wrapper" > wrapper.pid
    write_status
    set +e
    wait "$wrapper"
    local status=$?
    set -e
    return "$status"
}

for level in "${REPLAY_ORDER[@]}"; do
    expected=$(expected_claims "$level")
    printf 'VERIFY_K%s\n' "$level" > stage; write_status
    run_phase "run9-selected-ordinary-k$level" "verify-k$level.out" "verify-k$level.err" \
        env RADIO_RUN_CONTEXT="run_id=$RUN_ID; stage=verify-k$level; input=selected" \
            REFUTE_THREADS="$THREADS" REFUTE_PROGRESS_SECONDS="$PROGRESS_SECONDS" \
            REFUTE_MIN_K="$level" REFUTE_MAX_K="$level" \
            stdbuf -oL -eL "$SOURCE_DIR/tools/run_with_provenance.py" \
                "./$BINARY" "$WORK/run9-k$level.cert"
    "$SOURCE_DIR/tools/check_provenance.py" "verify-k$level.out" > "verify-k$level-provenance.txt"
    grep '^TOTAL verified ' "verify-k$level.out" | tail -n 1 > "verify-k$level.total"
    # The selected count is the contract: a different number means the input was not what we think.
    grep -Eq "^TOTAL verified $expected, gaps 0," "verify-k$level.total"
    wall=$(grep -oE 'wall_s=[0-9.]+' "verify-k$level.out" | tail -n 1 | cut -d= -f2)
    cpu=$(grep -oE 'cpu_s=[0-9.]+' "verify-k$level.out" | tail -n 1 | cut -d= -f2)
    printf '%s\t%s\t%s\t%s\n' "$level" "$expected" "${wall:-NA}" "${cpu:-NA}" >> timings.tsv
    for f in "verify-k$level.out" "verify-k$level.err"; do
        zstd -T1 -5 -f "$f" -o "$f.zst"
        aws s3 cp "$f.zst" "s3://$BUCKET/$PREFIX/$f.zst" --no-progress
    done
    aws s3 cp "verify-k$level.total" "s3://$BUCKET/$PREFIX/verify-k$level.total" --no-progress
    aws s3 cp "verify-k$level-provenance.txt" "s3://$BUCKET/$PREFIX/verify-k$level-provenance.txt" --no-progress
    write_status
done

sum=0
for level in "${LEVELS[@]}"; do
    v=$(awk '/^TOTAL verified / {gsub(/,/,"",$3); print $3}' "verify-k$level.total")
    sum=$((sum + v))
done
[[ "$sum" == "$TOTAL_CLAIMS" ]]
total_cpu=$(awk -F '\t' 'NR>1 && $4 != "NA" {s+=$4} END {printf "%.3f", s}' timings.tsv)
total_wall=$(awk -F '\t' 'NR>1 && $3 != "NA" {s+=$3} END {printf "%.3f", s}' timings.tsv)
{
    printf 'TOTAL verified %d, gaps 0 (eight ordinary level-v2 checkpoints over selected input)\n' "$sum"
    printf 'total_cpu_seconds=%s\ntotal_wall_seconds=%s\n' "$total_cpu" "$total_wall"
    printf 'baseline_complete_ordinary_cpu_seconds=211335.569\n'
    printf 'baseline_selected_colored_cpu_seconds=218792.627\n'
    awk -v c="$total_cpu" 'BEGIN {
        printf "ratio_vs_complete_ordinary=%.4f\nratio_vs_selected_colored=%.4f\n",
            c/211335.569, c/218792.627 }'
} > verify.total
printf 'COMPLETE\n' > stage
printf '0\n' > exit.status

: > final.sha256
for f in "$BINARY" "$BINARY.provenance" run.meta STATUS timings.tsv verify.total exit.status \
        verify-k*.total verify-k*-provenance.txt verify-k*.out.zst verify-k*.err.zst; do
    [[ -f "$f" ]] && sha256sum "$f" >> final.sha256
done
for f in final.sha256 verify.total timings.tsv exit.status "$BINARY.provenance"; do
    aws s3 cp "$f" "s3://$BUCKET/$PREFIX/$f" --no-progress
done
write_status
cat verify.total
