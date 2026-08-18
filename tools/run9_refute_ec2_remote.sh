#!/usr/bin/env bash
# Frozen-trie run9 audit on a dedicated EC2 host. The exact source bundle is already /root/source.
set -euo pipefail

BUCKET=radio-sa193-393287594714
WORK=
PREFIX=
RUN_ID=
SOURCE_COMMIT=
SOURCE_SHA256=
CERT_KEY=run9-verifier-progress/20260818T062429Z/run9-sanitized.cert.zst
CERT_ZST_SHA256=59b7f74730037ce8ccf5ff30049d78f5c0472b2c1fdc59586af780df27872d7c
CERT_SHA256=3ad5877a2ffa3bcf04c3403a147ae075e406b4313cce83eb0761fdd563725116
THREADS=16
CPUSET=0-15
RSS_GIB=8
PHASE_SECONDS=86400
PROGRESS_SECONDS=60
BINARY=run9_refute
FINALIZED=0
SOURCE_DIR=/root/source
K7_FACTS=2576885
K7_FOUR_PART_FACTS=2398799
LOWER_FACTS=546744
UPPER_FACTS=2561
TOTAL_FACTS=3126190
BENCH_STRIDE=240
BENCH_FACTS=9995
MAX_PROJECTED_CPU_SECONDS=419353
MAX_PROJECTED_WALL_SECONDS=86400

usage() {
    echo "usage: $0 --run-id ID --work /root/DIR --prefix S3_PREFIX --source-commit SHA --source-sha256 SHA" >&2
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
        --threads) THREADS=$2; shift 2 ;;
        --cpuset) CPUSET=$2; shift 2 ;;
        --rss-gib) RSS_GIB=$2; shift 2 ;;
        --phase-seconds) PHASE_SECONDS=$2; shift 2 ;;
        --progress-seconds) PROGRESS_SECONDS=$2; shift 2 ;;
        *) usage ;;
    esac
done

[[ "$RUN_ID" =~ ^[0-9]{8}T[0-9]{6}Z$ ]] || usage
[[ "$WORK" == "/root/run9-frozen-refute-$RUN_ID" ]] || usage
[[ "$PREFIX" == "run9-frozen-refute/$RUN_ID" ]] || usage
[[ "$SOURCE_COMMIT" =~ ^[0-9a-f]{40}$ ]] || usage
valid_sha "$SOURCE_SHA256" || usage
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
        printf 'run9 frozen negative-trie refuter\n'
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
            grep '^CACHE_PROGRESS ' "$out" | tail -n 1 || true
            grep '^CACHE_DONE ' "$out" | tail -n 1 || true
            grep '^FREEZE_PROGRESS ' "$out" | tail -n 1 || true
            grep '^FREEZE_DONE ' "$out" | tail -n 1 || true
            grep '^FROZEN_EPOCH ' "$out" | tail -n 1 || true
            grep '^BATCH_START ' "$out" | tail -n 1 || true
            grep '^PROGRESS phase=' "$out" | tail -n 1 || true
            grep '^PROGRESS_LEVELS ' "$out" | tail -n 1 || true
            grep '^PROGRESS_ACTIVE ' "$out" | tail -n 3 || true
            grep '^BATCH_DONE ' "$out" | tail -n 1 || true
            grep '^TOTAL verified ' "$out" | tail -n 1 || true
        }
        [[ -s benchmark.projection ]] && cat benchmark.projection
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
        [[ -s "$out" ]] && grep -E '^(CACHE_PROGRESS|CACHE_DONE|FREEZE_PROGRESS|FREEZE_DONE|FROZEN_EPOCH|BATCH_START|BATCH_DONE|PROGRESS|TOTAL)' "$out" | tail -n 160 || true
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
    local label=$1 out=$2 err=$3
    shift 3
    "$SOURCE_DIR/tools/capped_run.sh" --seconds "$PHASE_SECONDS" --rss-gb "$RSS_GIB" --label "$label" -- \
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
    for file in test.out test.err bench.out bench.err verify-k7.out verify-k7.err \
            verify-lower.out verify-lower.err verify-upper.out verify-upper.err bootstrap.out; do
        [[ -f "$file" ]] || continue
        zstd -T1 -5 -f "$file" -o "$file.zst" || true
    done
    sha256sum "$BINARY" "$BINARY.provenance" benchmark.projection \
        verify-k7.total verify-lower.total verify-upper.total verify.total *.zst \
        > final.sha256 2>/dev/null || true
    upload_live finished
    for file in STATUS PROGRESS run.meta exit.status verify.total verify-k7.total \
            verify-lower.total verify-upper.total final.sha256 benchmark.projection \
            "$BINARY" "$BINARY.provenance" test.out.zst test.err.zst bench.out.zst bench.err.zst \
            verify-k7.out.zst verify-k7.err.zst verify-lower.out.zst verify-lower.err.zst \
            verify-upper.out.zst verify-upper.err.zst bootstrap.out.zst \
            bench-provenance.txt verify-k7-provenance.txt \
            verify-lower-provenance.txt verify-upper-provenance.txt; do
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
    printf 'source_commit=%s\nsource_sha256=%s\n' "$SOURCE_COMMIT" "$SOURCE_SHA256"
    printf 'source_key=s3://%s/%s/source-radio-%s.tar.zst\n' \
        "$BUCKET" "$PREFIX" "${SOURCE_COMMIT:0:7}"
    printf 'certificate_key=s3://%s/%s\ncertificate_zst_sha256=%s\ncertificate_sha256=%s\n' \
        "$BUCKET" "$CERT_KEY" "$CERT_ZST_SHA256" "$CERT_SHA256"
    printf 'threads=%s\ncpuset=%s\nrss_cap_gib=%s\nphase_seconds=%s\nprogress_seconds=%s\n' \
        "$THREADS" "$CPUSET" "$RSS_GIB" "$PHASE_SECONDS" "$PROGRESS_SECONDS"
    printf 'on_demand_usd_per_hour=0.86216\n'
    sha256sum /root/source.tar.zst /root/source/radio_refute.c /root/source/radiobase.c "$0"
} > run.meta

monitor_loop $$ &
monitor_pid=$!

aws s3 cp "s3://$BUCKET/$CERT_KEY" run9.cert.zst --no-progress
printf '%s  run9.cert.zst\n' "$CERT_ZST_SHA256" | sha256sum -c -
zstd -dc run9.cert.zst > run9.cert
printf '%s  run9.cert\n' "$CERT_SHA256" | sha256sum -c -

export RADIO_SOURCE_COMMIT="$SOURCE_COMMIT"
set_stage TEST
"$SOURCE_DIR/tools/test_radio_refute.sh" > test.out 2> test.err

python3 "$SOURCE_DIR/tools/build_radio.py" -O3 -pthread -Wall -Wextra -Wpedantic \
    -DMAX_K=10 -DMAX_N=194 \
    "$SOURCE_DIR/radio_refute.c" -o "$WORK/$BINARY"
python3 "$SOURCE_DIR/tools/check_provenance.py" "$BINARY.provenance"

set_stage BENCH
run_phase run9-frozen-refute-bench bench.out bench.err \
    env RADIO_RUN_CONTEXT="run_id=$RUN_ID; stage=bench" REFUTE_THREADS="$THREADS" \
        REFUTE_PROGRESS_SECONDS="$PROGRESS_SECONDS" REFUTE_MIN_K=7 REFUTE_MAX_K=7 \
        REFUTE_MIN_PARTS=4 REFUTE_MAX_PARTS=4 REFUTE_STRIDE="$BENCH_STRIDE" REFUTE_OFFSET=0 \
        stdbuf -oL -eL "$SOURCE_DIR/tools/run_with_provenance.py" "./$BINARY" "$WORK/run9.cert"
"$SOURCE_DIR/tools/check_provenance.py" bench.out > bench-provenance.txt
grep -Eq "^TOTAL verified $BENCH_FACTS, gaps 0," bench.out
bench_done=$(awk '/^BATCH_DONE / {for(i=1;i<=NF;i++) if($i ~ /^completed=/) {split($i,a,"[=/]"); print a[2]}}' bench.out | tail -n 1)
bench_wall=$(awk '/^BATCH_DONE / {for(i=1;i<=NF;i++) if($i ~ /^wall_s=/) {split($i,a,"="); print a[2]}}' bench.out | tail -n 1)
bench_cpu=$(awk '/^BATCH_DONE / {for(i=1;i<=NF;i++) if($i ~ /^cpu_s=/) {split($i,a,"="); print a[2]}}' bench.out | tail -n 1)
[[ "$bench_done" == "$BENCH_FACTS" && "$bench_wall" =~ ^[0-9]+([.][0-9]+)?$ \
   && "$bench_cpu" =~ ^[0-9]+([.][0-9]+)?$ ]]
projected_wall=$(awk -v x="$bench_wall" -v sample="$BENCH_FACTS" -v total="$K7_FOUR_PART_FACTS" 'BEGIN {printf "%.0f", x*total/sample}')
projected_cpu=$(awk -v x="$bench_cpu" -v sample="$BENCH_FACTS" -v total="$K7_FOUR_PART_FACTS" 'BEGIN {printf "%.0f", x*total/sample}')
{
    printf 'sample_facts=%s\nsample_wall_seconds=%s\nsample_cpu_seconds=%s\n' \
        "$BENCH_FACTS" "$bench_wall" "$bench_cpu"
    printf 'projected_k7_four_part_wall_seconds=%s\nprojected_k7_four_part_cpu_seconds=%s\n' \
        "$projected_wall" "$projected_cpu"
    printf 'gate_max_wall_seconds=%s\ngate_max_cpu_seconds=%s\n' \
        "$MAX_PROJECTED_WALL_SECONDS" "$MAX_PROJECTED_CPU_SECONDS"
} > benchmark.projection
(( projected_wall <= MAX_PROJECTED_WALL_SECONDS )) || exit 75
(( projected_cpu <= MAX_PROJECTED_CPU_SECONDS )) || exit 76

set_stage VERIFY_K7
run_phase run9-frozen-refute-k7 verify-k7.out verify-k7.err \
    env RADIO_RUN_CONTEXT="run_id=$RUN_ID; stage=verify-k7" REFUTE_THREADS="$THREADS" \
        REFUTE_PROGRESS_SECONDS="$PROGRESS_SECONDS" REFUTE_MIN_K=7 REFUTE_MAX_K=7 \
        stdbuf -oL -eL "$SOURCE_DIR/tools/run_with_provenance.py" "./$BINARY" "$WORK/run9.cert"
"$SOURCE_DIR/tools/check_provenance.py" verify-k7.out > verify-k7-provenance.txt
grep '^TOTAL verified ' verify-k7.out | tail -n 1 > verify-k7.total
grep -Eq "^TOTAL verified $K7_FACTS, gaps 0," verify-k7.total

set_stage VERIFY_LOWER
run_phase run9-frozen-refute-lower verify-lower.out verify-lower.err \
    env RADIO_RUN_CONTEXT="run_id=$RUN_ID; stage=verify-lower" REFUTE_THREADS="$THREADS" \
        REFUTE_PROGRESS_SECONDS="$PROGRESS_SECONDS" REFUTE_MIN_K=1 REFUTE_MAX_K=6 \
        stdbuf -oL -eL "$SOURCE_DIR/tools/run_with_provenance.py" "./$BINARY" "$WORK/run9.cert"
"$SOURCE_DIR/tools/check_provenance.py" verify-lower.out > verify-lower-provenance.txt
grep '^TOTAL verified ' verify-lower.out | tail -n 1 > verify-lower.total
grep -Eq "^TOTAL verified $LOWER_FACTS, gaps 0," verify-lower.total

set_stage VERIFY_UPPER
run_phase run9-frozen-refute-upper verify-upper.out verify-upper.err \
    env RADIO_RUN_CONTEXT="run_id=$RUN_ID; stage=verify-upper" REFUTE_THREADS="$THREADS" \
        REFUTE_PROGRESS_SECONDS="$PROGRESS_SECONDS" REFUTE_MIN_K=8 REFUTE_MAX_K=9 \
        stdbuf -oL -eL "$SOURCE_DIR/tools/run_with_provenance.py" "./$BINARY" "$WORK/run9.cert"
"$SOURCE_DIR/tools/check_provenance.py" verify-upper.out > verify-upper-provenance.txt
grep '^TOTAL verified ' verify-upper.out | tail -n 1 > verify-upper.total
grep -Eq "^TOTAL verified $UPPER_FACTS, gaps 0," verify-upper.total
printf 'TOTAL verified %d, gaps 0 (three frozen-level checkpoints)\n' "$TOTAL_FACTS" > verify.total

kill "$monitor_pid" 2>/dev/null || true
wait "$monitor_pid" 2>/dev/null || true
preserve_final 0
trap - EXIT
shutdown -h +2 >/dev/null 2>&1 || true
