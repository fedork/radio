#!/usr/bin/env bash
# Launch another cold Sa(193) solver in an already-populated EC2 run directory.
# This script runs ON the instance, after the current source bundle has been unpacked.
# It never touches the incumbent run directory or process.
#
# Example (from /root/run4):
#   tools/sa193_sidecar_remote.sh --prefix run4 --binary radio_sa193_v4 \
#       --build 0123456789abcdef0123456789abcdef01234567 \
#       --peer radio_sa193_v3 --peer-prefix run3 --rss-gb 60 \
#       --idle-solvers radio_sa193_v3,radio_sa193_v4

set -euo pipefail

PREFIX=run4
BINARY=radio_sa193_v4
BUILD=unknown
PEER=radio_sa193_v3
PEER_PREFIX=run3
IDLE_SOLVERS=
RSS_GB=60
JOINT_RSS_GB=0
WATCH_INTERVAL=600
# Ten years is an accident backstop, not an intended deadline.  The run should end by verdict.
SECONDS_CAP=315360000
BUCKET=radio-sa193-393287594714
TOPIC=arn:aws:sns:us-west-2:393287594714:radio-sa193-progress

while (( $# )); do
    case "$1" in
        --prefix) PREFIX="$2"; shift 2 ;;
        --binary) BINARY="$2"; shift 2 ;;
        --build) BUILD="$2"; shift 2 ;;
        --peer) PEER="$2"; shift 2 ;;
        --peer-prefix) PEER_PREFIX="$2"; shift 2 ;;
        --idle-solvers) IDLE_SOLVERS="$2"; shift 2 ;;
        --rss-gb) RSS_GB="$2"; shift 2 ;;
        --joint-rss-gb) JOINT_RSS_GB="$2"; shift 2 ;;
        --watch-interval) WATCH_INTERVAL="$2"; shift 2 ;;
        --seconds) SECONDS_CAP="$2"; shift 2 ;;
        *) echo "unknown argument: $1" >&2; exit 64 ;;
    esac
done

[[ "$PREFIX" =~ ^[a-zA-Z0-9_-]+$ ]] || { echo "invalid prefix: $PREFIX" >&2; exit 64; }
[[ "$BINARY" =~ ^[a-zA-Z0-9_-]+$ ]] || { echo "invalid binary name: $BINARY" >&2; exit 64; }
[[ "$BUILD" =~ ^([0-9a-fA-F]{40}|[0-9a-fA-F]{64})$ ]] || {
    echo "--build must be a full 40- or 64-digit source commit hash" >&2
    exit 64
}
[[ "$PEER" =~ ^[a-zA-Z0-9_-]+$ ]] || { echo "invalid peer name: $PEER" >&2; exit 64; }
[[ "$PEER_PREFIX" =~ ^[a-zA-Z0-9_-]+$ ]] || {
    echo "invalid peer prefix: $PEER_PREFIX" >&2
    exit 64
}
[[ "$RSS_GB" =~ ^[0-9]+$ && "$RSS_GB" -gt 0 ]] || { echo "invalid RSS cap" >&2; exit 64; }
[[ "$JOINT_RSS_GB" =~ ^[0-9]+$ ]] || { echo "invalid joint RSS cap" >&2; exit 64; }
[[ "$WATCH_INTERVAL" =~ ^[0-9]+$ && "$WATCH_INTERVAL" -ge 60 ]] || {
    echo "watch interval must be at least 60 seconds" >&2
    exit 64
}
[[ "$SECONDS_CAP" =~ ^[0-9]+$ && "$SECONDS_CAP" -gt 0 ]] || { echo "invalid time cap" >&2; exit 64; }
[[ -n "$IDLE_SOLVERS" ]] || {
    echo "need --idle-solvers with every solver that must keep the host alive" >&2
    exit 64
}
IFS=',' read -r -a IDLE_NAMES <<< "$IDLE_SOLVERS"
(( ${#IDLE_NAMES[@]} > 0 )) || { echo "need at least one idle-guard solver" >&2; exit 64; }
for name in "${IDLE_NAMES[@]}"; do
    [[ "$name" =~ ^[a-zA-Z0-9_-]+$ ]] || {
        echo "invalid idle-guard process name: $name" >&2
        exit 64
    }
done

RUN_DIR=$(pwd -P)
[[ "$RUN_DIR" == /root/run* ]] || {
    echo "refusing to launch outside a dedicated /root/run* directory: $RUN_DIR" >&2
    exit 73
}

for needed in radiobase.c radio_sa193.c parse_out.sh tools/capped_run.sh \
              tools/build_radio.py tools/check_provenance.py \
              tools/sa193_watchdog.sh tools/sa193_compare.py \
              tools/sa193_idle_shutdown.sh tools/sa193_joint_rss_guard.sh; do
    [[ -f "$needed" ]] || { echo "missing $RUN_DIR/$needed" >&2; exit 66; }
done
for existing in "$BINARY" "$BINARY.provenance" out_sa193.txt sa193.err run.meta wrapper.pid solver.pid watchdog.pid; do
    [[ ! -e "$existing" ]] || { echo "refusing to overwrite $RUN_DIR/$existing" >&2; exit 73; }
done

PEER_COUNT=$(pgrep -xc "$PEER" || true)
[[ "$PEER_COUNT" == 1 ]] || {
    echo "expected exactly one incumbent $PEER, found $PEER_COUNT" >&2
    exit 69
}
PEER_LOG="/root/$PEER_PREFIX/out_sa193.txt"
[[ -r "$PEER_LOG" ]] || {
    echo "peer log is not readable: $PEER_LOG" >&2
    exit 66
}
if pgrep -x "$BINARY" >/dev/null 2>&1; then
    echo "target process $BINARY is already running" >&2
    exit 69
fi

AVAILABLE_KB=$(awk '/^MemAvailable:/ {print $2}' /proc/meminfo)
MIN_AVAILABLE_KB=$(( (RSS_GB + 16) * 1024 * 1024 ))
[[ "$AVAILABLE_KB" -ge "$MIN_AVAILABLE_KB" ]] || {
    echo "only $((AVAILABLE_KB / 1048576)) GiB available; need at least $((RSS_GB + 16)) GiB" >&2
    exit 70
}
DISK_KB=$(df -Pk "$RUN_DIR" | awk 'NR == 2 {print $4}')
[[ "$DISK_KB" -ge $((50 * 1024 * 1024)) ]] || {
    echo "less than 50 GiB free in $RUN_DIR" >&2
    exit 70
}

chmod +x parse_out.sh tools/*.sh tools/*.py
RADIO_SOURCE_COMMIT="$BUILD" python3 tools/build_radio.py \
    -O3 -DMAX_K=10 -DMAX_N=193 radio_sa193.c -o "$BINARY"
python3 tools/check_provenance.py "$BINARY.provenance"

START_UTC=$(date -u +%FT%TZ)
SEG="seg-${START_UTC//[:T-]/}-${BUILD}"
{
    printf 'started_utc=%s\n' "$START_UTC"
    printf 'build=%s\n' "$BUILD"
    printf 'directory=%s\n' "$RUN_DIR"
    printf 'command=%s/%s\n' "$RUN_DIR" "$BINARY"
    printf 'cache=none (cold)\n'
    printf 'control=Sa(192) in 10, enabled\n'
    printf 'peer=%s pid=%s\n' "$PEER" "$(pgrep -x "$PEER")"
    printf 'peer_prefix=%s\n' "$PEER_PREFIX"
    printf 'peer_log=%s\n' "$PEER_LOG"
    printf 'rss_limit_gib=%s\n' "$RSS_GB"
    printf 'joint_rss_limit_gib=%s\n' "$JOINT_RSS_GB"
    printf 'wall_backstop_seconds=%s\n' "$SECONDS_CAP"
    printf 'watch_interval_seconds=%s\n' "$WATCH_INTERVAL"
    printf 's3_prefix=s3://%s/%s/\n' "$BUCKET" "$PREFIX"
    printf 'mem_available_gib=%s\n' "$((AVAILABLE_KB / 1048576))"
    printf 'disk_available_gib=%s\n' "$((DISK_KB / 1048576))"
    sha256sum tools/sa193_watchdog.sh tools/sa193_compare.py \
        tools/capped_run.sh tools/sa193_idle_shutdown.sh tools/sa193_joint_rss_guard.sh
    cat "$BINARY.provenance"
} > run.meta

RADIO_RUN_CONTEXT="cold; prefix=$PREFIX; peer=$PEER" \
setsid nohup tools/capped_run.sh --seconds "$SECONDS_CAP" --rss-gb "$RSS_GB" \
    --label "$BINARY" -- "./$BINARY" > out_sa193.txt 2> sa193.err < /dev/null &
WRAPPER_PID=$!
printf '%s\n' "$WRAPPER_PID" > wrapper.pid

SOLVER_PID=
for _ in $(seq 1 30); do
    SOLVER_PID=$(pgrep -P "$WRAPPER_PID" -x "$BINARY" | head -1 || true)
    [[ -n "$SOLVER_PID" ]] && break
    kill -0 "$WRAPPER_PID" 2>/dev/null || break
    sleep 1
done
if [[ -z "$SOLVER_PID" ]] || ! kill -0 "$SOLVER_PID" 2>/dev/null; then
    echo "solver did not survive launch" >&2
    tail -n 40 sa193.err >&2 || true
    kill -TERM "$WRAPPER_PID" 2>/dev/null || true
    exit 71
fi
printf '%s\n' "$SOLVER_PID" > solver.pid

for _ in $(seq 1 30); do
    grep -q 'cache=(none, cold)' out_sa193.txt 2>/dev/null && break
    kill -0 "$SOLVER_PID" 2>/dev/null || break
    sleep 1
done
if ! grep -q 'cache=(none, cold)' out_sa193.txt 2>/dev/null; then
    echo "cold-run banner did not appear" >&2
    kill -TERM "$WRAPPER_PID" 2>/dev/null || true
    exit 71
fi

setsid nohup env SEG="$SEG" PROFILE="$RUN_DIR/memprofile.csv" \
    tools/sa193_watchdog.sh --log "$RUN_DIR/out_sa193.txt" --pid "$SOLVER_PID" \
    --bucket "$BUCKET" --topic "$TOPIC" --prefix "$PREFIX" \
    --interval "$WATCH_INTERVAL" --heartbeat 21600 \
    --compare-log "$PEER_LOG" --compare-label "$PEER_PREFIX" --compare-top 6 \
    >> wd.log 2>&1 < /dev/null &
WATCHDOG_PID=$!
printf '%s\n' "$WATCHDOG_PID" > watchdog.pid

JOINT_GUARD_PID=
if (( JOINT_RSS_GB > 0 )); then
    setsid nohup tools/sa193_joint_rss_guard.sh "$JOINT_RSS_GB" "$WRAPPER_PID" \
        "${IDLE_NAMES[@]}" >> joint-rss-guard.log 2>&1 < /dev/null &
    JOINT_GUARD_PID=$!
    printf '%s\n' "$JOINT_GUARD_PID" > joint_guard.pid
fi

# Replace any older idle guard only after the broader one is alive.  Leaving a two-name guard in
# place when a third sidecar is added can stop the host while that third solver is still running.
OLD_IDLE_PIDS=$(pgrep -f '[s]a193_idle_shutdown[.]sh' || true)
setsid nohup tools/sa193_idle_shutdown.sh "${IDLE_NAMES[@]}" \
    >> /var/log/sa193-idle-shutdown.log 2>&1 < /dev/null &
IDLE_GUARD_PID=$!
printf '%s\n' "$IDLE_GUARD_PID" > idle_guard.pid

sleep 3
kill -0 "$SOLVER_PID"
kill -0 "$WATCHDOG_PID"
[[ -z "$JOINT_GUARD_PID" ]] || kill -0 "$JOINT_GUARD_PID"
kill -0 "$IDLE_GUARD_PID"
[[ "$(pgrep -xc "$PEER" || true)" == 1 ]]
for name in "${IDLE_NAMES[@]}"; do
    [[ "$(pgrep -xc "$name" || true)" == 1 ]] || {
        echo "expected exactly one live $name after launch" >&2
        exit 69
    }
done
for old_pid in $OLD_IDLE_PIDS; do
    [[ "$old_pid" == "$IDLE_GUARD_PID" ]] || kill -TERM "$old_pid" 2>/dev/null || true
done
{
    printf 'wrapper_pid=%s\n' "$WRAPPER_PID"
    printf 'solver_pid=%s\n' "$SOLVER_PID"
    printf 'watchdog_pid=%s\n' "$WATCHDOG_PID"
    printf 'joint_guard_pid=%s\n' "${JOINT_GUARD_PID:-none}"
    printf 'idle_guard_pid=%s\n' "$IDLE_GUARD_PID"
    printf 'idle_guard_solvers=%s\n' "$IDLE_SOLVERS"
} >> run.meta

echo "launched $BINARY pid=$SOLVER_PID, watchdog=$WATCHDOG_PID, prefix=$PREFIX"
echo "incumbent $PEER pid=$(pgrep -x "$PEER") remains alive"
echo "memory: $(free -h | awk '/^Mem:/ {print $3 " used, " $7 " available"}')"
