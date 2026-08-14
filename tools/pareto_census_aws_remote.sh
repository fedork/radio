#!/usr/bin/env bash
# Stage and detach one resumable Pareto-prefix census on the shared Sa(193) host.
#
# This helper runs on the EC2 instance as root.  The source and input archives must already be in
# the private radio bucket.  It refuses to reuse a work directory or process name, verifies every
# transferred byte, builds with embedded provenance, and replaces the host's idle guard only after
# a broader guard is alive.  The existing Sa(193) solvers are never stopped by this launcher.
set -euo pipefail

BUCKET=radio-sa193-393287594714
WORK_DIR=
S3_PREFIX=
SOURCE_KEY=
INPUT_KEY=
SOURCE_COMMIT=
SOURCE_SHA256=
INPUT_SHA256=
BINARY=pareto_k8_aws
RSS_GIB=20
JOINT_RSS_GIB=108
SECONDS_CAP=315360000
IDLE_SOLVERS=radio_sa193_v3,radio_sa193_v8,radio_sa193_v9,pareto_k8_aws

usage() {
    cat >&2 <<'EOF'
usage: pareto_census_aws_remote.sh \
  --work-dir /root/pareto-census-k8-TIMESTAMP \
  --s3-prefix pareto-census-k8/TIMESTAMP \
  --source-key KEY --source-commit COMMIT --source-sha256 SHA256 \
  --input-key KEY --input-sha256 SHA256
EOF
    exit 64
}

valid_name() { [[ "$1" =~ ^[a-zA-Z0-9_-]+$ ]]; }
valid_sha() { [[ "$1" =~ ^[0-9a-f]{64}$ ]]; }

write_status() {
    local state=$1 output=$2
    {
        printf 'pareto-prefix k=8 census\n'
        printf 'updated_utc=%s\n' "$(date -u +%FT%TZ)"
        printf 'state=%s\n' "$state"
        if [[ -s "$WORK_DIR/out.txt" ]]; then
            awk -F '\t' '
                /^CENSUS\tSECOND_SUMMARY\t/ {
                    second++
                    if ($0 ~ /memo_imported=0/) fresh++
                }
                /^CENSUS\tPREFIX_SUMMARY\t/ {prefix=1}
                /^CENSUS\tTARGET\t/ {target++}
                /^CENSUS\tENDPOINT\t/ {endpoint++}
                /^CENSUS\tFULL_STATE\t/ {full_state++}
                /^CENSUS\tFULL_WIN\t/ {full_win++}
                /^CENSUS\tEND\t/ {end++}
                END {
                    printf "second_blocks=%d/815\n", second
                    printf "fresh_second_blocks=%d/70\n", fresh
                    printf "prefix_summary=%d\n", prefix
                    printf "targets=%d\n", target
                    printf "endpoints=%d\n", endpoint
                    printf "full_states=%d\n", full_state
                    printf "full_winners=%d\n", full_win
                    printf "end_records=%d\n", end
                }
            ' "$WORK_DIR/out.txt"
            wc -l -c "$WORK_DIR/out.txt" | awk '{printf "output_lines=%s\noutput_bytes=%s\n", $1, $2}'
            grep '^CENSUS[[:space:]]FIRST[[:space:]]' "$WORK_DIR/out.txt" | tail -n 1 || true
            grep '^CENSUS[[:space:]]SECOND_SUMMARY[[:space:]]' "$WORK_DIR/out.txt" | tail -n 1 || true
            grep '^still solving in ' "$WORK_DIR/out.txt" | tail -n 1 || true
        fi
        [[ -f "$WORK_DIR/exit.status" ]] && printf 'exit_status=%s\n' "$(<"$WORK_DIR/exit.status")"
        [[ -f "$WORK_DIR/solver.pid" ]] && printf 'solver_pid=%s\n' "$(<"$WORK_DIR/solver.pid")"
        printf 's3=s3://%s/%s/\n' "$BUCKET" "$S3_PREFIX"
    } > "$output"
}

supervise() {
    WORK_DIR=$2
    S3_PREFIX=$3
    BINARY=$4
    RSS_GIB=$5
    SECONDS_CAP=$6
    BUCKET=$7
    cd "$WORK_DIR"

    set +e
    RADIO_RUN_CONTEXT="pareto-prefix-k8 AWS resume; summary-closed checkpoints; unfinished local tail replayed" \
        source/tools/capped_run.sh --seconds "$SECONDS_CAP" --rss-gb "$RSS_GIB" \
        --label "$BINARY" -- "./$BINARY" \
        input/dominance.cache source/data/pareto_sb.csv 8 2000000 \
        input/root_winners.out input/exact.cache \
        input/checkpoint_complete2.out input/checkpoint_recovery.out \
        input/checkpoint_local_interrupted.out > out.txt 2> solver.err &
    wrapper_pid=$!
    printf '%s\n' "$wrapper_pid" > wrapper.pid
    solver_pid=
    for _ in $(seq 1 120); do
        solver_pid=$(pgrep -P "$wrapper_pid" -x "$BINARY" | head -n 1 || true)
        [[ -n "$solver_pid" ]] && break
        kill -0 "$wrapper_pid" 2>/dev/null || break
        sleep 1
    done
    if [[ -n "$solver_pid" ]]; then
        printf '%s\n' "$solver_pid" > solver.pid
    fi
    wait "$wrapper_pid"
    status=$?
    printf '%s\n' "$status" > exit.status
    set -e

    provenance_status=not-checked
    if source/tools/check_provenance.py out.txt > provenance-check.txt 2>&1; then
        provenance_status=valid
    else
        provenance_status=invalid
    fi
    analysis_status=not-run
    if [[ "$status" == 0 ]]; then
        if source/tools/analyze_pareto_prefix_census.py out.txt \
                --pareto-csv source/data/pareto_sb.csv --json > analysis.json 2> analysis.err; then
            analysis_status=valid
        else
            analysis_status=failed
        fi
    fi
    {
        printf 'finished_utc=%s\n' "$(date -u +%FT%TZ)"
        printf 'exit_status=%s\n' "$status"
        printf 'provenance_status=%s\n' "$provenance_status"
        printf 'analysis_status=%s\n' "$analysis_status"
        [[ -f out.txt ]] && sha256sum out.txt
        [[ -f analysis.json ]] && sha256sum analysis.json
    } >> run.meta
    write_status "finished" STATUS

    zstd -T2 -10 -f out.txt -o out.txt.zst
    for file in STATUS run.meta "$BINARY.provenance" provenance-check.txt solver.err \
            supervisor.log exit.status out.txt.zst analysis.json analysis.err \
            source-bundle.sha256 input-bundle.sha256 input.sha256; do
        [[ -f "$file" ]] || continue
        aws s3 cp "$file" "s3://$BUCKET/$S3_PREFIX/$file" --no-progress || true
    done
    exit "$status"
}

if [[ "${1:-}" == --supervise ]]; then
    (( $# == 7 )) || usage
    supervise "$@"
fi

while (( $# )); do
    case "$1" in
        --work-dir) WORK_DIR=$2; shift 2 ;;
        --s3-prefix) S3_PREFIX=$2; shift 2 ;;
        --source-key) SOURCE_KEY=$2; shift 2 ;;
        --input-key) INPUT_KEY=$2; shift 2 ;;
        --source-commit) SOURCE_COMMIT=$2; shift 2 ;;
        --source-sha256) SOURCE_SHA256=$2; shift 2 ;;
        --input-sha256) INPUT_SHA256=$2; shift 2 ;;
        --binary) BINARY=$2; shift 2 ;;
        --rss-gib) RSS_GIB=$2; shift 2 ;;
        --joint-rss-gib) JOINT_RSS_GIB=$2; shift 2 ;;
        --seconds) SECONDS_CAP=$2; shift 2 ;;
        --idle-solvers) IDLE_SOLVERS=$2; shift 2 ;;
        *) usage ;;
    esac
done

[[ "$WORK_DIR" =~ ^/root/pareto-census-k8-[0-9A-Za-z_-]+$ ]] || usage
[[ "$S3_PREFIX" =~ ^[0-9A-Za-z_./-]+$ ]] || usage
[[ "$SOURCE_KEY" =~ ^[0-9A-Za-z_./-]+$ ]] || usage
[[ "$INPUT_KEY" =~ ^[0-9A-Za-z_./-]+$ ]] || usage
[[ "$SOURCE_COMMIT" =~ ^[0-9a-f]{40}$ ]] || usage
valid_sha "$SOURCE_SHA256" || usage
valid_sha "$INPUT_SHA256" || usage
valid_name "$BINARY" || usage
[[ "$RSS_GIB" =~ ^[0-9]+$ && "$RSS_GIB" -gt 0 ]] || usage
[[ "$JOINT_RSS_GIB" =~ ^[0-9]+$ && "$JOINT_RSS_GIB" -gt 0 ]] || usage
[[ "$SECONDS_CAP" =~ ^[0-9]+$ && "$SECONDS_CAP" -gt 0 ]] || usage
IFS=',' read -r -a IDLE_NAMES <<< "$IDLE_SOLVERS"
for name in "${IDLE_NAMES[@]}"; do valid_name "$name" || usage; done
[[ "${IDLE_NAMES[*]}" == *"$BINARY"* ]] || { echo "idle-solvers must include $BINARY" >&2; exit 64; }

[[ ! -e "$WORK_DIR" ]] || { echo "refusing to reuse $WORK_DIR" >&2; exit 73; }
! pgrep -x "$BINARY" >/dev/null 2>&1 || { echo "$BINARY is already running" >&2; exit 69; }
available_kib=$(awk '/^MemAvailable:/ {print $2}' /proc/meminfo)
(( available_kib >= (RSS_GIB + 16) * 1024 * 1024 )) || {
    echo "insufficient available memory" >&2
    exit 70
}
disk_kib=$(df -Pk /root | awk 'NR == 2 {print $4}')
(( disk_kib >= 20 * 1024 * 1024 )) || { echo "less than 20 GiB disk free" >&2; exit 70; }

mkdir -p "$WORK_DIR/source"
cp "$0" "$WORK_DIR/launch_remote.sh"
chmod +x "$WORK_DIR/launch_remote.sh"
cd "$WORK_DIR"
aws s3 cp "s3://$BUCKET/$SOURCE_KEY" source.tar.zst --no-progress
aws s3 cp "s3://$BUCKET/$INPUT_KEY" input.tar.zst --no-progress
printf '%s  %s\n' "$SOURCE_SHA256" source.tar.zst > source-bundle.sha256
printf '%s  %s\n' "$INPUT_SHA256" input.tar.zst > input-bundle.sha256
sha256sum -c source-bundle.sha256
sha256sum -c input-bundle.sha256
zstd -dc source.tar.zst | tar -xf - -C source
zstd -dc input.tar.zst | tar -xf -
(cd input && sha256sum -c ../input.sha256)

RADIO_SOURCE_COMMIT="$SOURCE_COMMIT" python3 source/tools/build_radio.py \
    -O3 -DMAX_K=8 -DMAX_N=336 source/tools/pareto_prefix_census.c -o "$WORK_DIR/$BINARY"
python3 source/tools/check_provenance.py "$BINARY.provenance"

start_utc=$(date -u +%FT%TZ)
{
    printf 'started_utc=%s\n' "$start_utc"
    printf 'source_commit=%s\n' "$SOURCE_COMMIT"
    printf 'work_dir=%s\n' "$WORK_DIR"
    printf 's3_prefix=s3://%s/%s/\n' "$BUCKET" "$S3_PREFIX"
    printf 'source_key=s3://%s/%s\n' "$BUCKET" "$SOURCE_KEY"
    printf 'source_sha256=%s\n' "$SOURCE_SHA256"
    printf 'input_key=s3://%s/%s\n' "$BUCKET" "$INPUT_KEY"
    printf 'input_sha256=%s\n' "$INPUT_SHA256"
    printf 'command=%s/%s input/dominance.cache source/data/pareto_sb.csv 8 2000000 input/root_winners.out input/exact.cache input/checkpoint_complete2.out input/checkpoint_recovery.out input/checkpoint_local_interrupted.out\n' "$WORK_DIR" "$BINARY"
    printf 'resume_policy=only summary-closed second-cut blocks; interrupted tail ignored and replayed\n'
    printf 'individual_rss_limit_gib=%s\n' "$RSS_GIB"
    printf 'joint_rss_limit_gib=%s\n' "$JOINT_RSS_GIB"
    printf 'wall_backstop_seconds=%s\n' "$SECONDS_CAP"
    printf 'idle_guard_solvers=%s\n' "$IDLE_SOLVERS"
    printf 'mem_available_gib=%s\n' "$((available_kib / 1048576))"
    printf 'disk_available_gib=%s\n' "$((disk_kib / 1048576))"
    sha256sum launch_remote.sh source.tar.zst input.tar.zst
    cat "$BINARY.provenance"
} > run.meta

setsid nohup "$WORK_DIR/launch_remote.sh" --supervise "$WORK_DIR" "$S3_PREFIX" \
    "$BINARY" "$RSS_GIB" "$SECONDS_CAP" "$BUCKET" \
    > supervisor.log 2>&1 < /dev/null &
supervisor_pid=$!
printf '%s\n' "$supervisor_pid" > supervisor.pid

wrapper_pid=
solver_pid=
for _ in $(seq 1 120); do
    [[ -s wrapper.pid ]] && wrapper_pid=$(<wrapper.pid)
    [[ -s solver.pid ]] && solver_pid=$(<solver.pid)
    [[ -n "$solver_pid" ]] && kill -0 "$solver_pid" 2>/dev/null && break
    kill -0 "$supervisor_pid" 2>/dev/null || break
    sleep 1
done
[[ -n "$wrapper_pid" && -n "$solver_pid" ]] || {
    echo "census did not survive launch" >&2
    tail -n 80 supervisor.log solver.err 2>/dev/null >&2 || true
    exit 71
}
kill -0 "$wrapper_pid"
kill -0 "$solver_pid"
[[ "$(pgrep -xc "$BINARY" || true)" == 1 ]]

setsid nohup source/tools/sa193_joint_rss_guard.sh "$JOINT_RSS_GIB" "$wrapper_pid" \
    "${IDLE_NAMES[@]}" >> joint-rss-guard.log 2>&1 < /dev/null &
joint_guard_pid=$!
printf '%s\n' "$joint_guard_pid" > joint_guard.pid

old_idle_pids=$(pgrep -f '[s]a193_idle_shutdown[.]sh' || true)
setsid nohup source/tools/sa193_idle_shutdown.sh "${IDLE_NAMES[@]}" \
    >> /var/log/sa193-idle-shutdown.log 2>&1 < /dev/null &
idle_guard_pid=$!
printf '%s\n' "$idle_guard_pid" > idle_guard.pid
sleep 3
kill -0 "$joint_guard_pid"
kill -0 "$idle_guard_pid"
for name in "${IDLE_NAMES[@]}"; do
    [[ "$(pgrep -xc "$name" || true)" == 1 ]] || {
        echo "expected exactly one live $name after launch" >&2
        exit 69
    }
done
for old_pid in $old_idle_pids; do
    [[ "$old_pid" == "$idle_guard_pid" ]] || kill -TERM "$old_pid" 2>/dev/null || true
done
{
    printf 'supervisor_pid=%s\n' "$supervisor_pid"
    printf 'wrapper_pid=%s\n' "$wrapper_pid"
    printf 'solver_pid=%s\n' "$solver_pid"
    printf 'joint_guard_pid=%s\n' "$joint_guard_pid"
    printf 'idle_guard_pid=%s\n' "$idle_guard_pid"
} >> run.meta
write_status running STATUS
for file in STATUS run.meta "$BINARY.provenance" launch_remote.sh \
        source-bundle.sha256 input-bundle.sha256 input.sha256; do
    aws s3 cp "$file" "s3://$BUCKET/$S3_PREFIX/$file" --no-progress
done

echo "launched $BINARY solver=$solver_pid wrapper=$wrapper_pid supervisor=$supervisor_pid"
echo "guards: individual=${RSS_GIB}GiB joint=${JOINT_RSS_GIB}GiB/$joint_guard_pid idle=$idle_guard_pid"
echo "work=$WORK_DIR s3=s3://$BUCKET/$S3_PREFIX/"
