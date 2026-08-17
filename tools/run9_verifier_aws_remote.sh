#!/usr/bin/env bash
# Stage and detach the independent run9 certificate coloring/replay on the shared AWS host.
# Run this script as root on the instance after its exact source archive is in the private bucket.
set -euo pipefail

BUCKET=radio-sa193-393287594714
WORK_DIR=
S3_PREFIX=
SOURCE_KEY=
SOURCE_COMMIT=
SOURCE_SHA256=
RUN9_LOG=/root/run9/out_sa193.txt
RUN9_SHA256=ba635d9141601ebb643ed4f102703deb112fc3e8260f4936e8545fe44a300cf4
SANITIZED_SHA256=3ad5877a2ffa3bcf04c3403a147ae075e406b4313cce83eb0761fdd563725116
BINARY=run9_verify
THREADS=14
CPUSET=0-13
RSS_GIB=80
SECONDS_CAP=2592000
IDLE_NAMES=(radio_sa193_v3 radio_sa193_v8 radio_sa193_v9 pareto_k8_aws run9_verify)

usage() {
    cat >&2 <<'EOF'
usage: run9_verifier_aws_remote.sh \
  --work-dir /root/run9-verifier-TIMESTAMP \
  --s3-prefix run9-verifier/TIMESTAMP \
  --source-key KEY --source-commit COMMIT --source-sha256 SHA256
EOF
    exit 64
}

valid_sha() { [[ "$1" =~ ^[0-9a-f]{64}$ ]]; }

write_status() {
    local state=$1 output=$2
    {
        printf 'run9 independent verifier\n'
        printf 'updated_utc=%s\n' "$(date -u +%FT%TZ)"
        printf 'state=%s\n' "$state"
        [[ -s "$WORK_DIR/stage" ]] && printf 'stage=%s\n' "$(<"$WORK_DIR/stage")"
        for label in supervisor wrapper verifier idle_guard; do
            [[ -s "$WORK_DIR/$label.pid" ]] && printf '%s_pid=%s\n' "$label" "$(<"$WORK_DIR/$label.pid")"
        done
        for file in run9-sanitized.cert run9-colored.cert sanitize.out color.out verify.out; do
            [[ -f "$WORK_DIR/$file" ]] || continue
            stat -c "file=$file bytes=%s mtime=%y" "$WORK_DIR/$file"
        done
        [[ -s "$WORK_DIR/sanitize.fact_count" ]] && printf 'sanitized_facts=%s\n' "$(<"$WORK_DIR/sanitize.fact_count")"
        [[ -s "$WORK_DIR/color.root_count" ]] && printf 'colored_roots=%s\n' "$(<"$WORK_DIR/color.root_count")"
        [[ -s "$WORK_DIR/color.fact_count" ]] && printf 'colored_facts=%s\n' "$(<"$WORK_DIR/color.fact_count")"
        [[ -s "$WORK_DIR/verify.total" ]] && printf 'verify_total=%s\n' "$(<"$WORK_DIR/verify.total")"
        [[ -s "$WORK_DIR/exit.status" ]] && printf 'exit_status=%s\n' "$(<"$WORK_DIR/exit.status")"
        printf 'threads=%s\ncpuset=%s\nrss_cap_gib=%s\n' "$THREADS" "$CPUSET" "$RSS_GIB"
        printf 's3=s3://%s/%s/\n' "$BUCKET" "$S3_PREFIX"
    } > "$output"
}

upload_status() {
    write_status "${1:-running}" "$WORK_DIR/STATUS"
    aws s3 cp "$WORK_DIR/STATUS" "s3://$BUCKET/$S3_PREFIX/STATUS" --no-progress || true
}

set_stage() {
    printf '%s\n' "$1" > "$WORK_DIR/stage"
    printf '%s stage=%s\n' "$(date -u +%FT%TZ)" "$1" >> "$WORK_DIR/run.meta"
    upload_status running
}

run_phase() {
    local label=$1 rss=$2 out=$3 err=$4
    shift 4
    source/tools/capped_run.sh --seconds "$SECONDS_CAP" --rss-gb "$rss" --label "$label" -- \
        taskset -c "$CPUSET" nice -n 10 "$@" > "$out" 2> "$err" &
    local wrapper_pid=$!
    printf '%s\n' "$wrapper_pid" > wrapper.pid
    : > verifier.pid
    local verifier_pid=
    for _ in $(seq 1 120); do
        verifier_pid=$(pgrep -P "$wrapper_pid" -x "$BINARY" | head -n 1 || true)
        [[ -n "$verifier_pid" ]] && break
        kill -0 "$wrapper_pid" 2>/dev/null || break
        sleep 1
    done
    [[ -n "$verifier_pid" ]] && printf '%s\n' "$verifier_pid" > verifier.pid
    upload_status running
    set +e
    wait "$wrapper_pid"
    local status=$?
    set -e
    return "$status"
}

preserve_final() {
    local status=$1
    printf '%s\n' "$status" > exit.status
    {
        printf 'finished_utc=%s\n' "$(date -u +%FT%TZ)"
        printf 'exit_status=%s\n' "$status"
        sha256sum run9-sanitized.cert run9-colored.cert sanitize.out color.out verify.out 2>/dev/null || true
    } >> run.meta
    upload_status finished

    for cert in run9-sanitized.cert run9-colored.cert; do
        [[ -f "$cert" ]] || continue
        taskset -c 0-1 nice -n 10 zstd -T2 -19 -f "$cert" -o "$cert.zst" || true
    done
    for log in sanitize.out sanitize.err sanitize-roundtrip.out sanitize-roundtrip.err \
            color.out color.err verify.out verify.err supervisor.log; do
        [[ -f "$log" ]] || continue
        zstd -T1 -10 -f "$log" -o "$log.zst" || true
    done
    sha256sum run9-sanitized.cert run9-colored.cert *.zst > final.sha256 2>/dev/null || true
    for file in STATUS run.meta exit.status "$BINARY.provenance" final.sha256 \
            run9-sanitized.cert.zst run9-colored.cert.zst sanitize.out.zst sanitize.err.zst \
            sanitize-roundtrip.out.zst sanitize-roundtrip.err.zst color.out.zst color.err.zst \
            verify.out.zst verify.err.zst supervisor.log.zst source-bundle.sha256; do
        [[ -f "$file" ]] || continue
        aws s3 cp "$file" "s3://$BUCKET/$S3_PREFIX/$file" --no-progress || true
    done
}

supervise() {
    WORK_DIR=$2
    S3_PREFIX=$3
    BINARY=$4
    THREADS=$5
    CPUSET=$6
    RSS_GIB=$7
    SECONDS_CAP=$8
    BUCKET=$9
    RUN9_LOG=${10}
    SANITIZED_SHA256=${11}
    cd "$WORK_DIR"
    trap 'status=$?; if (( status != 0 )) && [[ ! -f exit.status ]]; then printf "supervisor_abort_status=%s\n" "$status" >> run.meta; preserve_final "$status"; fi' EXIT

    set_stage SANITIZE
    if run_phase run9-sanitize 16 sanitize.out sanitize.err \
            env CERT_ONLY=1 CERT_OUT="$WORK_DIR/run9-sanitized.cert" \
            stdbuf -oL -eL source/tools/run_with_provenance.py \
            "./$BINARY" "$RUN9_LOG" 10; then
        :
    else
        status=$?; printf 'SANITIZE failed status=%s\n' "$status" >> run.meta
        preserve_final "$status"; exit "$status"
    fi
    source/tools/check_provenance.py sanitize.out > sanitize-provenance.txt
    fact_count=$(grep -c '^fact ' run9-sanitized.cert)
    printf '%s\n' "$fact_count" > sanitize.fact_count
    [[ "$fact_count" == 3126190 ]]
    printf '%s  run9-sanitized.cert\n' "$SANITIZED_SHA256" | sha256sum -c -

    if run_phase run9-sanitize-roundtrip 16 sanitize-roundtrip.out sanitize-roundtrip.err \
            env CERT_ONLY=1 CERT_OUT="$WORK_DIR/run9-sanitized.roundtrip.cert" \
            stdbuf -oL -eL source/tools/run_with_provenance.py \
            "./$BINARY" "$WORK_DIR/run9-sanitized.cert" 10; then
        :
    else
        status=$?; printf 'SANITIZE_ROUNDTRIP failed status=%s\n' "$status" >> run.meta
        preserve_final "$status"; exit "$status"
    fi
    source/tools/check_provenance.py sanitize-roundtrip.out > sanitize-roundtrip-provenance.txt
    cmp run9-sanitized.cert run9-sanitized.roundtrip.cert
    rm -f -- "$WORK_DIR/run9-sanitized.roundtrip.cert"

    set_stage COLOR
    if run_phase run9-color "$RSS_GIB" color.out color.err \
            env TOPDOWN=9 ROOTS="$WORK_DIR/source/evidence/sa193_unsolvable_in_10.txt" \
            MINIMIZE_BEFORE_COLOR=1 CERT_OUT="$WORK_DIR/run9-colored.cert" \
            VERIFY_THREADS="$THREADS" stdbuf -oL -eL \
            source/tools/run_with_provenance.py "./$BINARY" "$WORK_DIR/run9-sanitized.cert" 9; then
        :
    else
        status=$?; printf 'COLOR failed status=%s\n' "$status" >> run.meta
        preserve_final "$status"; exit "$status"
    fi
    source/tools/check_provenance.py color.out > color-provenance.txt
    root_count=$(grep -c '^root ' run9-colored.cert)
    fact_count=$(grep -c '^fact ' run9-colored.cert)
    printf '%s\n' "$root_count" > color.root_count
    printf '%s\n' "$fact_count" > color.fact_count
    [[ "$root_count" == 16 && "$fact_count" -gt 0 ]]

    set_stage VERIFY
    if run_phase run9-verify "$RSS_GIB" verify.out verify.err \
            env VERIFY_THREADS="$THREADS" stdbuf -oL -eL \
            source/tools/run_with_provenance.py "./$BINARY" "$WORK_DIR/run9-colored.cert" 9; then
        :
    else
        status=$?; printf 'VERIFY failed status=%s\n' "$status" >> run.meta
        preserve_final "$status"; exit "$status"
    fi
    source/tools/check_provenance.py verify.out > verify-provenance.txt
    total=$(grep '^TOTAL verified ' verify.out | tail -n 1)
    printf '%s\n' "$total" > verify.total
    grep -Eq '^TOTAL verified [0-9]+, unverified 0, budget 0,' verify.total
    verified=$(sed -E 's/^TOTAL verified ([0-9]+),.*/\1/' verify.total)
    expected=$(( $(<color.root_count) + $(<color.fact_count) ))
    [[ "$verified" == "$expected" ]]

    set_stage COMPLETE
    preserve_final 0
    trap - EXIT
}

if [[ "${1:-}" == --supervise ]]; then
    (( $# == 11 )) || usage
    supervise "$@"
    exit
fi

while (( $# )); do
    case "$1" in
        --work-dir) WORK_DIR=$2; shift 2 ;;
        --s3-prefix) S3_PREFIX=$2; shift 2 ;;
        --source-key) SOURCE_KEY=$2; shift 2 ;;
        --source-commit) SOURCE_COMMIT=$2; shift 2 ;;
        --source-sha256) SOURCE_SHA256=$2; shift 2 ;;
        --threads) THREADS=$2; shift 2 ;;
        --cpuset) CPUSET=$2; shift 2 ;;
        --rss-gib) RSS_GIB=$2; shift 2 ;;
        --seconds) SECONDS_CAP=$2; shift 2 ;;
        *) usage ;;
    esac
done

[[ "$WORK_DIR" =~ ^/root/run9-verifier-[0-9A-Za-z_-]+$ ]] || usage
[[ "$S3_PREFIX" =~ ^[0-9A-Za-z_./-]+$ ]] || usage
[[ "$SOURCE_KEY" =~ ^[0-9A-Za-z_./-]+$ ]] || usage
[[ "$SOURCE_COMMIT" =~ ^[0-9a-f]{40}$ ]] || usage
valid_sha "$SOURCE_SHA256" || usage
[[ "$THREADS" =~ ^[0-9]+$ && "$THREADS" -gt 0 ]] || usage
[[ "$CPUSET" =~ ^[0-9,-]+$ ]] || usage
[[ "$RSS_GIB" =~ ^[0-9]+$ && "$RSS_GIB" -gt 0 ]] || usage
[[ "$SECONDS_CAP" =~ ^[0-9]+$ && "$SECONDS_CAP" -gt 0 ]] || usage
[[ ! -e "$WORK_DIR" ]] || { echo "refusing to reuse $WORK_DIR" >&2; exit 73; }
! pgrep -x "$BINARY" >/dev/null 2>&1 || { echo "$BINARY is already running" >&2; exit 69; }
command -v taskset >/dev/null
command -v stdbuf >/dev/null
command -v zstd >/dev/null

available_kib=$(awk '/^MemAvailable:/ {print $2}' /proc/meminfo)
(( available_kib >= (RSS_GIB + 16) * 1024 * 1024 )) || { echo "insufficient available memory" >&2; exit 70; }
disk_kib=$(df -Pk /root | awk 'NR == 2 {print $4}')
(( disk_kib >= 10 * 1024 * 1024 )) || { echo "less than 10 GiB disk free" >&2; exit 70; }

mkdir -p "$WORK_DIR/source"
cp "$0" "$WORK_DIR/launch_remote.sh"
chmod +x "$WORK_DIR/launch_remote.sh"
cd "$WORK_DIR"
aws s3 cp "s3://$BUCKET/$SOURCE_KEY" source.tar.zst --no-progress
printf '%s  source.tar.zst\n' "$SOURCE_SHA256" > source-bundle.sha256
sha256sum -c source-bundle.sha256
zstd -dc source.tar.zst | tar -xf - -C source
printf '%s  %s\n' "$RUN9_SHA256" "$RUN9_LOG" | sha256sum -c -
[[ "$(grep -c '^can.t solve.*Sb(' "$RUN9_LOG")" -gt 3000000 ]]
[[ "$(grep -c '^can.t solve Sb' source/evidence/sa193_unsolvable_in_10.txt)" == 16 ]]

RADIO_SOURCE_COMMIT="$SOURCE_COMMIT" python3 source/tools/build_radio.py \
    -O3 -pthread source/radio_verify.c -o "$WORK_DIR/$BINARY"
python3 source/tools/check_provenance.py "$BINARY.provenance"

{
    printf 'started_utc=%s\n' "$(date -u +%FT%TZ)"
    printf 'source_commit=%s\nsource_key=s3://%s/%s\nsource_sha256=%s\n' \
        "$SOURCE_COMMIT" "$BUCKET" "$SOURCE_KEY" "$SOURCE_SHA256"
    printf 'run9_log=%s\nrun9_sha256=%s\n' "$RUN9_LOG" "$RUN9_SHA256"
    printf 'sanitized_expected_sha256=%s\n' "$SANITIZED_SHA256"
    printf 'threads=%s\ncpuset=%s\nnice=10\n' "$THREADS" "$CPUSET"
    printf 'individual_rss_limit_gib=%s\nwall_backstop_seconds=%s\n' "$RSS_GIB" "$SECONDS_CAP"
    printf 'mem_available_gib=%s\ndisk_available_gib=%s\n' \
        "$((available_kib / 1048576))" "$((disk_kib / 1048576))"
    printf 's3=s3://%s/%s/\n' "$BUCKET" "$S3_PREFIX"
    sha256sum launch_remote.sh source.tar.zst "$RUN9_LOG" source/evidence/sa193_unsolvable_in_10.txt
    cat "$BINARY.provenance"
} > run.meta

setsid nohup "$WORK_DIR/launch_remote.sh" --supervise "$WORK_DIR" "$S3_PREFIX" \
    "$BINARY" "$THREADS" "$CPUSET" "$RSS_GIB" "$SECONDS_CAP" "$BUCKET" \
    "$RUN9_LOG" "$SANITIZED_SHA256" > supervisor.log 2>&1 < /dev/null &
supervisor_pid=$!
printf '%s\n' "$supervisor_pid" > supervisor.pid

for _ in $(seq 1 300); do
    stage=$(cat stage 2>/dev/null || true)
    verifier_pid=$(pgrep -x "$BINARY" | head -n 1 || true)
    [[ "$stage" == COLOR && -n "$verifier_pid" ]] && break
    kill -0 "$supervisor_pid" 2>/dev/null || break
    sleep 1
done
[[ "$stage" == COLOR && -n "$verifier_pid" ]] || {
    echo "verifier did not reach the coloring phase" >&2
    tail -n 80 supervisor.log sanitize.err sanitize-roundtrip.err color.err 2>/dev/null >&2 || true
    exit 71
}

old_idle_pids=$(pgrep -f '[s]a193_idle_shutdown[.]sh' || true)
setsid nohup source/tools/sa193_idle_shutdown.sh "${IDLE_NAMES[@]}" \
    >> /var/log/sa193-idle-shutdown.log 2>&1 < /dev/null &
idle_guard_pid=$!
printf '%s\n' "$idle_guard_pid" > idle_guard.pid
sleep 3
kill -0 "$idle_guard_pid"
for old_pid in $old_idle_pids; do
    [[ "$old_pid" == "$idle_guard_pid" ]] || kill -TERM "$old_pid" 2>/dev/null || true
done
printf '%s\n' "$idle_guard_pid" > /root/pareto-census-k8-20260814T0132Z/idle_guard.pid
{
    printf 'supervisor_pid=%s\nverifier_pid=%s\nidle_guard_pid=%s\n' \
        "$supervisor_pid" "$verifier_pid" "$idle_guard_pid"
    printf 'idle_guard_names=%s\n' "${IDLE_NAMES[*]}"
} >> run.meta
write_status running STATUS
for file in STATUS run.meta "$BINARY.provenance" launch_remote.sh source-bundle.sha256; do
    aws s3 cp "$file" "s3://$BUCKET/$S3_PREFIX/$file" --no-progress
done

echo "launched $BINARY verifier=$verifier_pid supervisor=$supervisor_pid idle_guard=$idle_guard_pid"
echo "stage=$stage threads=$THREADS cpuset=$CPUSET rss_cap=${RSS_GIB}GiB"
echo "work=$WORK_DIR s3=s3://$BUCKET/$S3_PREFIX/"
