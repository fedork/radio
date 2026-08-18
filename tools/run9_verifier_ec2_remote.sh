#!/usr/bin/env bash
# Full raw-log -> normalized certificate -> independent level-by-level audit on dedicated EC2.
# The caller has already unpacked the exact source bundle at /root/source.
set -euo pipefail

BUCKET=radio-sa193-393287594714
WORK=
PREFIX=
RUN_ID=
SOURCE_COMMIT=
SOURCE_SHA256=
RAW_KEY=
RAW_SHA256=ba635d9141601ebb643ed4f102703deb112fc3e8260f4936e8545fe44a300cf4
SANITIZED_SHA256=3ad5877a2ffa3bcf04c3403a147ae075e406b4313cce83eb0761fdd563725116
THREADS=16
CPUSET=0-15
RSS_GIB=24
PHASE_SECONDS=604800
PROGRESS_SECONDS=60
BINARY=run9_verify_p
FINALIZED=0
SOURCE_DIR=/root/source
K7_FACTS=2576885
K7_FOUR_PART_FACTS=2398799
LOWER_FACTS=546744
UPPER_FACTS=2561
BENCH_STRIDE=240
MAX_PROJECTED_SECONDS=604800

usage() {
    echo "usage: $0 --run-id ID --work /root/DIR --prefix S3_PREFIX --source-commit SHA --source-sha256 SHA --raw-key KEY" >&2
    exit 64
}

valid_sha() { [[ "$1" =~ ^[0-9a-f]{64}$ ]]; }

while (( $# )); do
    case "$1" in
        --run-id) RUN_ID=$2; shift 2 ;;
        --work) WORK=$2; shift 2 ;;
        --prefix) PREFIX=$2; shift 2 ;;
        --source-commit) SOURCE_COMMIT=$2; shift 2 ;;
        --source-sha256) SOURCE_SHA256=$2; shift 2 ;;
        --raw-key) RAW_KEY=$2; shift 2 ;;
        --threads) THREADS=$2; shift 2 ;;
        --cpuset) CPUSET=$2; shift 2 ;;
        --rss-gib) RSS_GIB=$2; shift 2 ;;
        --phase-seconds) PHASE_SECONDS=$2; shift 2 ;;
        --progress-seconds) PROGRESS_SECONDS=$2; shift 2 ;;
        *) usage ;;
    esac
done

[[ "$RUN_ID" =~ ^[0-9]{8}T[0-9]{6}Z$ ]] || usage
[[ "$WORK" == "/root/run9-verifier-progress-$RUN_ID" ]] || usage
[[ "$PREFIX" == "run9-verifier-progress/$RUN_ID" ]] || usage
[[ "$SOURCE_COMMIT" =~ ^[0-9a-f]{40}$ ]] || usage
valid_sha "$SOURCE_SHA256" || usage
[[ "$RAW_KEY" =~ ^[0-9A-Za-z_./-]+$ ]] || usage
[[ "$THREADS" =~ ^[1-9][0-9]*$ ]] || usage
[[ "$CPUSET" =~ ^[0-9,-]+$ ]] || usage
[[ "$RSS_GIB" =~ ^[1-9][0-9]*$ ]] || usage
[[ "$PHASE_SECONDS" =~ ^[1-9][0-9]*$ ]] || usage
[[ "$PROGRESS_SECONDS" =~ ^[1-9][0-9]*$ ]] || usage

mkdir -p "$WORK"
cd "$WORK"
printf 'SETUP\n' > stage

instance_id=$(TOKEN=$(curl -fsS -X PUT -H 'X-aws-ec2-metadata-token-ttl-seconds: 21600' \
    http://169.254.169.254/latest/api/token) && \
    curl -fsS -H "X-aws-ec2-metadata-token: $TOKEN" \
    http://169.254.169.254/latest/meta-data/instance-id)
instance_type=$(TOKEN=$(curl -fsS -X PUT -H 'X-aws-ec2-metadata-token-ttl-seconds: 21600' \
    http://169.254.169.254/latest/api/token) && \
    curl -fsS -H "X-aws-ec2-metadata-token: $TOKEN" \
    http://169.254.169.254/latest/meta-data/instance-type)

current_output() {
    case "$(cat stage 2>/dev/null || true)" in
        TEST) echo test.out ;;
        SANITIZE) echo sanitize.out ;;
        ROUNDTRIP) echo roundtrip.out ;;
        BENCH) echo bench.out ;;
        VERIFY_K7) echo verify-k7.out ;;
        VERIFY_LOWER) echo verify-lower.out ;;
        VERIFY_UPPER|COMPLETE) echo verify-upper.out ;;
        *) echo bootstrap.out ;;
    esac
}

write_status() {
    local state=$1 tmp out pid
    tmp=$(mktemp "$WORK/STATUS.tmp.XXXXXX")
    out=$(current_output)
    pid=$(cat verifier.pid 2>/dev/null || true)
    {
        printf 'run9 independent verifier with live progress\n'
        printf 'updated_utc=%s\nstate=%s\nstage=%s\n' "$(date -u +%FT%TZ)" "$state" \
            "$(cat stage 2>/dev/null || echo UNKNOWN)"
        printf 'run_id=%s\ninstance_id=%s\ninstance_type=%s\n' "$RUN_ID" "$instance_id" "$instance_type"
        printf 'threads=%s\ncpuset=%s\nrss_cap_gib=%s\nphase_wall_cap_seconds=%s\n' \
            "$THREADS" "$CPUSET" "$RSS_GIB" "$PHASE_SECONDS"
        if [[ -n "$pid" ]] && kill -0 "$pid" 2>/dev/null; then
            ps -p "$pid" -o pid=,%cpu=,rss=,vsz=,etimes=,stat=,comm= | awk \
                '{printf "verifier_pid=%s cpu_pct=%s rss_mib=%.1f vsz_mib=%.1f elapsed_s=%s stat=%s comm=%s\n",$1,$2,$3/1024,$4/1024,$5,$6,$7}'
        else
            printf 'verifier_pid=%s verifier_alive=no\n' "${pid:-unknown}"
        fi
        awk '/^(MemTotal|MemAvailable|SwapTotal|SwapFree):/ {v[$1]=$2}
             END {printf "host_mem_available_gib=%.1f swap_used_gib=%.1f\n",
                         v["MemAvailable:"]/1048576,
                         (v["SwapTotal:"]-v["SwapFree:"])/1048576}' /proc/meminfo
        df -Pk "$WORK" | awk 'NR==2 {printf "disk_available_gib=%.1f\n",$4/1048576}'
        [[ -s "$out" ]] && {
            grep '^BATCH_START ' "$out" | tail -n 1 || true
            grep '^PROGRESS phase=' "$out" | tail -n 1 || true
            grep '^PROGRESS_LEVELS ' "$out" | tail -n 1 || true
            grep '^PROGRESS_PARTS ' "$out" | tail -n 1 || true
            grep '^PROGRESS_ACTIVE ' "$out" | tail -n 3 || true
            grep '^BATCH_DONE ' "$out" | tail -n 1 || true
        }
        [[ -s exit.status ]] && printf 'exit_status=%s\n' "$(<exit.status)"
        [[ -s verify.total ]] && printf 'result=%s\n' "$(<verify.total)"
        printf 's3=s3://%s/%s/\n' "$BUCKET" "$PREFIX"
    } > "$tmp"
    mv "$tmp" STATUS
}

upload_live() {
    local state=${1:-running} out tmp
    write_status "$state"
    out=$(current_output)
    tmp=$(mktemp "$WORK/PROGRESS.tmp.XXXXXX")
    {
        printf 'updated_utc=%s\n' "$(date -u +%FT%TZ)"
        [[ -s "$out" ]] && grep -E '^(BATCH_START|BATCH_DONE|PROGRESS)' "$out" | tail -n 120 || true
    } > "$tmp"
    mv "$tmp" PROGRESS
    aws s3 cp STATUS "s3://$BUCKET/$PREFIX/STATUS" --no-progress || true
    aws s3 cp PROGRESS "s3://$BUCKET/$PREFIX/PROGRESS" --no-progress || true
}

monitor_loop() {
    local parent=$1
    while kill -0 "$parent" 2>/dev/null; do
        upload_live running
        sleep "$PROGRESS_SECONDS"
    done
}

set_stage() {
    printf '%s\n' "$1" > stage
    printf '%s stage=%s\n' "$(date -u +%FT%TZ)" "$1" >> run.meta
    upload_live running
}

run_phase() {
    local label=$1 rss=$2 out=$3 err=$4
    shift 4
    "$SOURCE_DIR/tools/capped_run.sh" --seconds "$PHASE_SECONDS" --rss-gb "$rss" --label "$label" -- \
        taskset -c "$CPUSET" "$@" > "$out" 2> "$err" &
    local wrapper=$!
    printf '%s\n' "$wrapper" > wrapper.pid
    : > verifier.pid
    local pid=
    for _ in $(seq 1 120); do
        pid=$(pgrep -x "$BINARY" | head -n 1 || true)
        [[ -n "$pid" ]] && break
        kill -0 "$wrapper" 2>/dev/null || break
        sleep 1
    done
    [[ -n "$pid" ]] && printf '%s\n' "$pid" > verifier.pid
    upload_live running
    set +e
    wait "$wrapper"
    local status=$?
    set -e
    return "$status"
}

preserve_final() {
    local status=$1
    (( FINALIZED == 0 )) || return
    FINALIZED=1
    printf '%s\n' "$status" > exit.status
    if [[ "$status" == 0 ]]; then printf 'COMPLETE\n' > stage; fi
    write_status finished
    for file in run9-sanitized.cert; do
        [[ -f "$file" ]] || continue
        zstd -T2 -10 -f "$file" -o "$file.zst" || true
    done
    for file in test.out test.err sanitize.out sanitize.err roundtrip.out roundtrip.err \
            bench.out bench.err verify-k7.out verify-k7.err verify-lower.out verify-lower.err \
            verify-upper.out verify-upper.err bootstrap.out; do
        [[ -f "$file" ]] || continue
        zstd -T1 -5 -f "$file" -o "$file.zst" || true
    done
    sha256sum run9-sanitized.cert *.zst > final.sha256 2>/dev/null || true
    upload_live finished
    for file in STATUS PROGRESS run.meta exit.status verify.total verify-k7.total \
            verify-lower.total verify-upper.total final.sha256 \
            "$BINARY.provenance" test.out.zst test.err.zst sanitize.out.zst sanitize.err.zst \
            roundtrip.out.zst roundtrip.err.zst bench.out.zst bench.err.zst \
            verify-k7.out.zst verify-k7.err.zst verify-lower.out.zst verify-lower.err.zst \
            verify-upper.out.zst verify-upper.err.zst bootstrap.out.zst \
            run9-sanitized.cert.zst sanitize-provenance.txt roundtrip-provenance.txt \
            bench-provenance.txt verify-k7-provenance.txt verify-lower-provenance.txt \
            verify-upper-provenance.txt benchmark.projection; do
        [[ -f "$file" ]] || continue
        aws s3 cp "$file" "s3://$BUCKET/$PREFIX/$file" --no-progress || true
    done
    printf '%s final_upload_status=%s\n' "$(date -u +%FT%TZ)" "$status" >> run.meta
    aws s3 cp run.meta "s3://$BUCKET/$PREFIX/run.meta" --no-progress || true
}

trap 'status=$?; kill "${monitor_pid:-}" 2>/dev/null || true; wait "${monitor_pid:-}" 2>/dev/null || true; preserve_final "$status"; shutdown -h +2 >/dev/null 2>&1 || true' EXIT

cp /var/log/cloud-init-output.log bootstrap.out 2>/dev/null || : > bootstrap.out
{
    printf 'started_utc=%s\nrun_id=%s\ninstance_id=%s\ninstance_type=%s\n' \
        "$(date -u +%FT%TZ)" "$RUN_ID" "$instance_id" "$instance_type"
    printf 'source_commit=%s\nsource_sha256=%s\nraw_key=s3://%s/%s\nraw_sha256=%s\n' \
        "$SOURCE_COMMIT" "$SOURCE_SHA256" "$BUCKET" "$RAW_KEY" "$RAW_SHA256"
    printf 'source_key=s3://%s/%s/source-radio-%s.tar.zst\n' \
        "$BUCKET" "$PREFIX" "${SOURCE_COMMIT:0:7}"
    printf 'threads=%s\ncpuset=%s\nrss_cap_gib=%s\nphase_seconds=%s\nprogress_seconds=%s\n' \
        "$THREADS" "$CPUSET" "$RSS_GIB" "$PHASE_SECONDS" "$PROGRESS_SECONDS"
    printf 'on_demand_usd_per_hour=0.86216\n'
    sha256sum /root/source.tar.zst /root/source/radio_verify.c "$0"
} > run.meta

monitor_loop $$ &
monitor_pid=$!

aws s3 cp "s3://$BUCKET/$RAW_KEY" run9.log.zst --no-progress
zstd -t run9.log.zst
zstd -dc run9.log.zst > run9.log
printf '%s  run9.log\n' "$RAW_SHA256" | sha256sum -c -

export RADIO_SOURCE_COMMIT="$SOURCE_COMMIT"
set_stage TEST
"$SOURCE_DIR/tools/test_radio_verify.sh" > test.out 2> test.err

python3 "$SOURCE_DIR/tools/build_radio.py" -O3 -pthread -Wall -Wextra -Wpedantic \
    "$SOURCE_DIR/radio_verify.c" -o "$WORK/$BINARY"
python3 "$SOURCE_DIR/tools/check_provenance.py" "$BINARY.provenance"

set_stage SANITIZE
run_phase run9-progress-sanitize 4 sanitize.out sanitize.err \
    env RADIO_RUN_CONTEXT="run_id=$RUN_ID; stage=sanitize" CERT_ONLY=1 \
        CERT_OUT="$WORK/run9-sanitized.cert" stdbuf -oL -eL \
        "$SOURCE_DIR/tools/run_with_provenance.py" "./$BINARY" "$WORK/run9.log" 10
"$SOURCE_DIR/tools/check_provenance.py" sanitize.out > sanitize-provenance.txt
[[ "$(grep -c '^fact ' run9-sanitized.cert)" == 3126190 ]]
printf '%s  run9-sanitized.cert\n' "$SANITIZED_SHA256" | sha256sum -c -

set_stage ROUNDTRIP
run_phase run9-progress-roundtrip 4 roundtrip.out roundtrip.err \
    env RADIO_RUN_CONTEXT="run_id=$RUN_ID; stage=roundtrip" CERT_ONLY=1 \
        CERT_OUT="$WORK/run9-sanitized-roundtrip.cert" stdbuf -oL -eL \
        "$SOURCE_DIR/tools/run_with_provenance.py" "./$BINARY" "$WORK/run9-sanitized.cert" 10
"$SOURCE_DIR/tools/check_provenance.py" roundtrip.out > roundtrip-provenance.txt
cmp run9-sanitized.cert run9-sanitized-roundtrip.cert
rm -f -- run9-sanitized-roundtrip.cert

set_stage BENCH
run_phase run9-progress-bench "$RSS_GIB" bench.out bench.err \
    env RADIO_RUN_CONTEXT="run_id=$RUN_ID; stage=bench" VERIFY_THREADS="$THREADS" \
        VERIFY_PROGRESS_SECONDS="$PROGRESS_SECONDS" stdbuf -oL -eL \
        "$SOURCE_DIR/tools/run_with_provenance.py" "./$BINARY" "$WORK/run9-sanitized.cert" \
        7 3 1 3 4 4 "$BENCH_STRIDE" 0 7
"$SOURCE_DIR/tools/check_provenance.py" bench.out > bench-provenance.txt
grep -Eq '^TOTAL verified [0-9]+, unverified 0, budget 0,' bench.out
bench_done=$(awk '/^BATCH_DONE / {for(i=1;i<=NF;i++) if($i ~ /^completed=/) {split($i,a,"[=/]"); print a[2]}}' bench.out | tail -n 1)
bench_wall=$(awk '/^BATCH_DONE / {for(i=1;i<=NF;i++) if($i ~ /^wall_s=/) {split($i,a,"="); print a[2]}}' bench.out | tail -n 1)
[[ "$bench_done" =~ ^[1-9][0-9]*$ && "$bench_wall" =~ ^[0-9]+([.][0-9]+)?$ ]]
projected=$(awk -v wall="$bench_wall" -v sample="$bench_done" -v total="$K7_FOUR_PART_FACTS" \
    'BEGIN {printf "%.0f", wall * total / sample}')
{
    printf 'sample_facts=%s\nsample_wall_seconds=%s\nprojected_k7_four_part_seconds=%s\n' \
        "$bench_done" "$bench_wall" "$projected"
    printf 'gate_max_projected_seconds=%s\n' "$MAX_PROJECTED_SECONDS"
} > benchmark.projection
(( projected <= MAX_PROJECTED_SECONDS )) || {
    printf 'benchmark projection %s exceeds gate %s\n' "$projected" "$MAX_PROJECTED_SECONDS" >&2
    exit 75
}

set_stage VERIFY_K7
run_phase run9-progress-verify-k7 "$RSS_GIB" verify-k7.out verify-k7.err \
    env RADIO_RUN_CONTEXT="run_id=$RUN_ID; stage=verify-k7" VERIFY_THREADS="$THREADS" \
        VERIFY_PROGRESS_SECONDS="$PROGRESS_SECONDS" stdbuf -oL -eL \
        "$SOURCE_DIR/tools/run_with_provenance.py" "./$BINARY" "$WORK/run9-sanitized.cert" \
        7 3 1 3 0 999 1 0 7
"$SOURCE_DIR/tools/check_provenance.py" verify-k7.out > verify-k7-provenance.txt
grep '^TOTAL verified ' verify-k7.out | tail -n 1 > verify-k7.total
grep -Eq "^TOTAL verified $K7_FACTS, unverified 0, budget 0," verify-k7.total

set_stage VERIFY_LOWER
run_phase run9-progress-verify-lower "$RSS_GIB" verify-lower.out verify-lower.err \
    env RADIO_RUN_CONTEXT="run_id=$RUN_ID; stage=verify-lower" VERIFY_THREADS="$THREADS" \
        VERIFY_PROGRESS_SECONDS="$PROGRESS_SECONDS" stdbuf -oL -eL \
        "$SOURCE_DIR/tools/run_with_provenance.py" "./$BINARY" "$WORK/run9-sanitized.cert" 6 3
"$SOURCE_DIR/tools/check_provenance.py" verify-lower.out > verify-lower-provenance.txt
grep '^TOTAL verified ' verify-lower.out | tail -n 1 > verify-lower.total
grep -Eq "^TOTAL verified $LOWER_FACTS, unverified 0, budget 0," verify-lower.total

set_stage VERIFY_UPPER
run_phase run9-progress-verify-upper "$RSS_GIB" verify-upper.out verify-upper.err \
    env RADIO_RUN_CONTEXT="run_id=$RUN_ID; stage=verify-upper" VERIFY_THREADS="$THREADS" \
        VERIFY_PROGRESS_SECONDS="$PROGRESS_SECONDS" stdbuf -oL -eL \
        "$SOURCE_DIR/tools/run_with_provenance.py" "./$BINARY" "$WORK/run9-sanitized.cert" \
        9 3 1 3 0 999 1 0 8
"$SOURCE_DIR/tools/check_provenance.py" verify-upper.out > verify-upper-provenance.txt
grep '^TOTAL verified ' verify-upper.out | tail -n 1 > verify-upper.total
grep -Eq "^TOTAL verified $UPPER_FACTS, unverified 0, budget 0," verify-upper.total
printf 'TOTAL verified %d, unverified 0, budget 0 (three level checkpoints)\n' \
    "$((K7_FACTS + LOWER_FACTS + UPPER_FACTS))" > verify.total

kill "$monitor_pid" 2>/dev/null || true
wait "$monitor_pid" 2>/dev/null || true
preserve_final 0
trap - EXIT
shutdown -h +2 >/dev/null 2>&1 || true
