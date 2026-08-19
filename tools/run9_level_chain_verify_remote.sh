#!/usr/bin/env bash
# Verify an arbitrary eight-level v2 certificate chain with the ORDINARY (uncolored) refuter.
#
# Generalizes tools/run9_selected_ordinary_remote.sh, which pinned one input set inline. Use this
# for new chains; that script is retained only because run 20260819T013030Z was launched from it.
#
# The chain and its expectations are described entirely by an uploaded manifest, so this script
# carries no per-experiment constants:
#
#   s3://BUCKET/INPUT_PREFIX/manifest.tsv      level <TAB> claims <TAB> sha256-of-.cert.zst
#   s3://BUCKET/INPUT_PREFIX/run9-k<L>.cert.zst
#
# Every download is checked against its manifest SHA-256, and every level must report
# `TOTAL verified <claims>, gaps 0` for exactly the manifest's claim count, so a wrong or truncated
# input fails closed instead of producing a fast, meaningless number.
#
# It reuses an existing `run9_refute` binary and /root/source tree rather than building, because
# these runs exist to be compared against a measurement made with that exact binary. Rebuilding
# would confound the comparison.
#
# This is performance measurement. A zero-gap replay of a trimmed chain re-verifies the same claims
# with less support; it is not an independent re-proof.
set -euo pipefail

BUCKET=radio-sa193-393287594714
BASELINE_WORK=/root/run9-frozen-refute-20260818T194508Z
SOURCE_DIR=/root/source
BINARY=run9_refute
RUN_ID=
LABEL=
INPUT_PREFIX=
THREADS=16
CPUSET=0-15
RSS_GIB=8
PHASE_SECONDS=86400
PROGRESS_SECONDS=60
# Cheap levels first as a gate, then the dominant k=7 phase.
REPLAY_ORDER=(2 3 4 9 8 5 6 7)

usage() {
    echo "usage: $0 --run-id ID --label NAME --input-prefix S3_PREFIX [--threads N]" >&2
    exit 64
}
while (( $# )); do
    case "$1" in
        --run-id) RUN_ID=$2; shift 2 ;;
        --label) LABEL=$2; shift 2 ;;
        --input-prefix) INPUT_PREFIX=$2; shift 2 ;;
        --threads) THREADS=$2; shift 2 ;;
        --cpuset) CPUSET=$2; shift 2 ;;
        --rss-gib) RSS_GIB=$2; shift 2 ;;
        --phase-seconds) PHASE_SECONDS=$2; shift 2 ;;
        --progress-seconds) PROGRESS_SECONDS=$2; shift 2 ;;
        *) usage ;;
    esac
done
[[ "$RUN_ID" =~ ^[0-9]{8}T[0-9]{6}Z$ ]] || usage
[[ "$LABEL" =~ ^[a-z0-9-]+$ ]] || usage
[[ -n "$INPUT_PREFIX" ]] || usage

WORK=/root/run9-$LABEL-$RUN_ID
PREFIX=run9-$LABEL/$RUN_ID
[[ -x "$BASELINE_WORK/$BINARY" ]] || { echo "missing $BASELINE_WORK/$BINARY" >&2; exit 70; }
[[ -d "$SOURCE_DIR/tools" ]] || { echo "missing $SOURCE_DIR/tools" >&2; exit 70; }

mkdir -p "$WORK"; cd "$WORK"
cp -n "$BASELINE_WORK/$BINARY" "$BASELINE_WORK/$BINARY.provenance" .
chmod +x "./$BINARY"
printf 'SETUP\n' > stage

instance_id=$(TOKEN=$(curl -fsS -X PUT -H 'X-aws-ec2-metadata-token-ttl-seconds: 21600' \
    http://169.254.169.254/latest/api/token) && \
    curl -fsS -H "X-aws-ec2-metadata-token: $TOKEN" \
    http://169.254.169.254/latest/meta-data/instance-id)

aws s3 cp "s3://$BUCKET/$INPUT_PREFIX/manifest.tsv" manifest.tsv --no-progress
grep -Eq '^[0-9]+	[0-9]+	[0-9a-f]{64}$' manifest.tsv || { echo "malformed manifest" >&2; exit 71; }
LEVELS=($(awk -F '\t' '{print $1}' manifest.tsv | sort -n))
TOTAL_CLAIMS=$(awk -F '\t' '{s+=$2} END {print s}' manifest.tsv)
claims_for() { awk -F '\t' -v l="$1" '$1==l {print $2}' manifest.tsv; }
sha_for()    { awk -F '\t' -v l="$1" '$1==l {print $3}' manifest.tsv; }

{
    printf 'started_utc=%s\nrun_id=%s\nlabel=%s\ninstance_id=%s\nwork=%s\n' \
        "$(date -u +%FT%TZ)" "$RUN_ID" "$LABEL" "$instance_id" "$WORK"
    printf 's3_prefix=s3://%s/%s/\ninput_prefix=s3://%s/%s/\n' \
        "$BUCKET" "$PREFIX" "$BUCKET" "$INPUT_PREFIX"
    printf 'binary_reused_from=%s\nbinary_sha256=%s\n' \
        "$BASELINE_WORK" "$(sha256sum "./$BINARY" | cut -d' ' -f1)"
    printf 'threads=%s\ncpuset=%s\nrss_cap_gib=%s\nphase_wall_cap_seconds=%s\n' \
        "$THREADS" "$CPUSET" "$RSS_GIB" "$PHASE_SECONDS"
    printf 'total_claims=%s\n' "$TOTAL_CLAIMS"
    printf 'baseline_complete_ordinary_cpu_seconds=211335.569\n'
    printf 'baseline_selected_colored_cpu_seconds=218792.627\n'
    printf 'purpose=performance A/B with the ordinary refuter; not an independent re-proof\n'
} > run.meta

upload_state() {
    for f in run.meta STATUS timings.tsv; do
        [[ -f $f ]] && aws s3 cp "$f" "s3://$BUCKET/$PREFIX/$f" --no-progress >/dev/null 2>&1
    done
    return 0
}
write_status() {
    {
        printf 'run9 ordinary verifier over the %s level chain\n' "$LABEL"
        printf 'updated_utc=%s\nrun_id=%s\ninstance_id=%s\nstage=%s\n' \
            "$(date -u +%FT%TZ)" "$RUN_ID" "$instance_id" "$(cat stage)"
        for lv in "${LEVELS[@]}"; do
            [[ -f "verify-k$lv.total" ]] && printf 'completed_k%s=%s\n' "$lv" "$(cat "verify-k$lv.total")"
        done
        [[ -f timings.tsv ]] && sed 's/^/timing=/' timings.tsv
        awk '/^MemAvailable:/ {printf "host_mem_available_gib=%.1f\n", $2/1048576}' /proc/meminfo
        df -Pk "$WORK" | awk 'NR==2 {printf "disk_available_gib=%.1f\n", $4/1048576}'
    } > STATUS
    upload_state
}

printf 'FETCH\n' > stage; write_status
for level in "${LEVELS[@]}"; do
    want=$(sha_for "$level")
    aws s3 cp "s3://$BUCKET/$INPUT_PREFIX/run9-k$level.cert.zst" "run9-k$level.cert.zst" --no-progress
    got=$(sha256sum "run9-k$level.cert.zst" | cut -d' ' -f1)
    [[ "$got" == "$want" ]] || { echo "level $level sha mismatch want $want got $got" >&2; exit 72; }
    zstd -dq -f "run9-k$level.cert.zst" -o "run9-k$level.cert"
    # Cross-check the manifest against the certificate's own declared claim count.
    declared=$(awk -v l="$level" '$1=="claims" && $2==l {print $3; exit}' "run9-k$level.cert")
    [[ "$declared" == "$(claims_for "$level")" ]] || {
        echo "level $level: manifest claims $(claims_for "$level") != certificate $declared" >&2
        exit 73
    }
    support=$(awk '$1=="support" {print $2" "$3; exit}' "run9-k$level.cert")
    printf 'level %s: claims %s support %s\n' "$level" "$declared" "$support" >> input.summary
done
aws s3 cp input.summary "s3://$BUCKET/$PREFIX/input.summary" --no-progress
printf 'level\tclaims\twall_seconds\tcpu_seconds\n' > timings.tsv

run_phase() {
    local label=$1 out=$2 err=$3; shift 3
    "$SOURCE_DIR/tools/capped_run.sh" --seconds "$PHASE_SECONDS" --rss-gb "$RSS_GIB" --label "$label" -- \
        taskset -c "$CPUSET" "$@" > "$out" 2> "$err" &
    local wrapper=$!
    printf '%s\n' "$wrapper" > wrapper.pid
    write_status
    set +e; wait "$wrapper"; local status=$?; set -e
    return "$status"
}

for level in "${REPLAY_ORDER[@]}"; do
    expected=$(claims_for "$level")
    [[ "$expected" =~ ^[0-9]+$ ]] || continue
    printf 'VERIFY_K%s\n' "$level" > stage; write_status
    run_phase "run9-$LABEL-k$level" "verify-k$level.out" "verify-k$level.err" \
        env RADIO_RUN_CONTEXT="run_id=$RUN_ID; label=$LABEL; stage=verify-k$level" \
            REFUTE_THREADS="$THREADS" REFUTE_PROGRESS_SECONDS="$PROGRESS_SECONDS" \
            REFUTE_MIN_K="$level" REFUTE_MAX_K="$level" \
            stdbuf -oL -eL "$SOURCE_DIR/tools/run_with_provenance.py" \
                "./$BINARY" "$WORK/run9-k$level.cert"
    "$SOURCE_DIR/tools/check_provenance.py" "verify-k$level.out" > "verify-k$level-provenance.txt"
    grep '^TOTAL verified ' "verify-k$level.out" | tail -n 1 > "verify-k$level.total"
    grep -Eq "^TOTAL verified $expected, gaps 0," "verify-k$level.total"
    wall=$(grep -oE 'wall_s=[0-9.]+' "verify-k$level.out" | tail -n 1 | cut -d= -f2)
    cpu=$(grep -oE 'cpu_s=[0-9.]+' "verify-k$level.out" | tail -n 1 | cut -d= -f2)
    printf '%s\t%s\t%s\t%s\n' "$level" "$expected" "${wall:-NA}" "${cpu:-NA}" >> timings.tsv
    for f in "verify-k$level.out" "verify-k$level.err"; do
        zstd -T1 -5 -f "$f" -o "$f.zst"
        aws s3 cp "$f.zst" "s3://$BUCKET/$PREFIX/$f.zst" --no-progress
    done
    for f in "verify-k$level.total" "verify-k$level-provenance.txt"; do
        aws s3 cp "$f" "s3://$BUCKET/$PREFIX/$f" --no-progress
    done
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
    printf 'TOTAL verified %d, gaps 0 (%s chain, ordinary refuter)\n' "$sum" "$LABEL"
    printf 'total_cpu_seconds=%s\ntotal_wall_seconds=%s\n' "$total_cpu" "$total_wall"
    awk -v c="$total_cpu" 'BEGIN {
        printf "ratio_vs_complete_ordinary=%.4f\nratio_vs_selected_colored=%.4f\n",
            c/211335.569, c/218792.627 }'
} > verify.total
printf 'COMPLETE\n' > stage
printf '0\n' > exit.status
# Finalize STATUS *before* hashing: an earlier version rewrote it afterwards, so its manifest entry
# could never match and every verification reported one spurious STATUS mismatch.
write_status
: > final.sha256
for f in "$BINARY" "$BINARY.provenance" run.meta STATUS manifest.tsv input.summary timings.tsv \
        verify.total exit.status verify-k*.total verify-k*-provenance.txt \
        verify-k*.out.zst verify-k*.err.zst; do
    [[ -f "$f" ]] && sha256sum "$f" >> final.sha256
done
for f in final.sha256 verify.total timings.tsv exit.status manifest.tsv input.summary STATUS; do
    aws s3 cp "$f" "s3://$BUCKET/$PREFIX/$f" --no-progress
done
cat verify.total
