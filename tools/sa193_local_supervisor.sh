#!/bin/bash
# Supervise one cold local radio_sa193 run without imposing a time limit.
#
# Usage:
#   tools/sa193_local_supervisor.sh RUN_DIR [FOOTPRINT_GIB] [SAME_RUN_CHECKPOINT]
#
# RUN_DIR must already contain a freshly compiled `radio_sa193`.  The solver is
# invoked cold unless SAME_RUN_CHECKPOINT is supplied.  A resumed checkpoint is folded into every
# new checkpoint so the segment chain remains closed.  This script is intended to be started under
# nohup; it owns the solver, a caffeinate helper, checkpoints, and the physical-footprint guard that
# capped_run cannot provide on macOS.

set -u

if [ "$#" -lt 1 ] || [ "$#" -gt 3 ]; then
    echo "usage: $0 RUN_DIR [FOOTPRINT_GIB] [SAME_RUN_CHECKPOINT]" >&2
    exit 64
fi

RUN_DIR=$(cd "$1" 2>/dev/null && pwd) || {
    echo "run directory does not exist: $1" >&2
    exit 66
}
FOOTPRINT_GIB=${2:-20}
CACHE=${3:-}
REPO=/Users/fedor/radio
BIN="$RUN_DIR/radio_sa193"
OUT="$RUN_DIR/out_sa193.txt"
ERR="$RUN_DIR/sa193.err"
MONITOR_LOG="$RUN_DIR/monitor.log"
STATUS="$RUN_DIR/status.txt"
CHECKPOINT="$RUN_DIR/sa193.checkpoint"
META="$RUN_DIR/run.meta"
STOP_REASON="$RUN_DIR/stop.reason"
INTERVAL=120
CHECKPOINT_INTERVAL=3600
VMAP_TIMEOUT=20
MIN_DISK_KIB=$((10 * 1024 * 1024))
HARD_BYTES=$(awk -v gib="$FOOTPRINT_GIB" 'BEGIN { printf "%.0f", gib * 1024 * 1024 * 1024 }')

if [ ! -x "$BIN" ]; then
    echo "missing executable: $BIN" >&2
    exit 66
fi
if [ -n "$CACHE" ]; then
    CACHE=$(cd "$(dirname "$CACHE")" 2>/dev/null && printf '%s/%s' "$PWD" "$(basename "$CACHE")") || {
        echo "checkpoint directory does not exist: $CACHE" >&2
        exit 66
    }
    if [ ! -f "$CACHE" ]; then
        echo "checkpoint does not exist: $CACHE" >&2
        exit 66
    fi
fi
if [ -e "$OUT" ] || [ -e "$ERR" ] || [ -e "$META" ]; then
    echo "refusing to overwrite an existing run in $RUN_DIR" >&2
    exit 73
fi

human_bytes() {
    awk -v value="$1" 'BEGIN {
        gsub(/[+,]/, "", value)
        unit = substr(value, length(value), 1)
        if (unit ~ /[KMGT]/) {
            number = substr(value, 1, length(value) - 1) + 0
        } else {
            number = value + 0
            unit = "B"
        }
        multiplier = 1
        if (unit == "K") multiplier = 1024
        if (unit == "M") multiplier = 1024 * 1024
        if (unit == "G") multiplier = 1024 * 1024 * 1024
        if (unit == "T") multiplier = 1024 * 1024 * 1024 * 1024
        printf "%.0f", number * multiplier
    }'
}

write_checkpoint() {
    local tmp="$CHECKPOINT.tmp"
    {
        printf '# Same-run checkpoint generated from %s\n' "$OUT"
        if [ -n "$CACHE" ]; then
            printf '# Resume source: %s\n' "$CACHE"
            # Preserve the inherited facts as well as this segment's new facts.  Omitting the
            # source here would make a second restart forget everything learned before this one.
            command cat "$CACHE"
        else
            printf '# Cold solver command: %s (no cache argument)\n' "$BIN"
        fi
        "$REPO/parse_out.sh" < "$OUT"
    } > "$tmp" && mv "$tmp" "$CHECKPOINT"
}

bounded_vmmap() {
    # `vmmap` can itself hang while sampling a busy solver.  That previously froze both the memory
    # guard and hourly checkpoints indefinitely.  Kill only the diagnostic child after a short
    # timeout; five consecutive failed samples still stop the solver through the existing guard.
    vmmap -summary "$SOLVER_PID" 2>/dev/null &
    local probe_pid=$!
    (
        sleep "$VMAP_TIMEOUT"
        kill -TERM "$probe_pid" 2>/dev/null || exit 0
        sleep 1
        kill -KILL "$probe_pid" 2>/dev/null || true
    ) &
    local timer_pid=$!
    wait "$probe_pid"
    local probe_status=$?
    kill -TERM "$timer_pid" 2>/dev/null || true
    wait "$timer_pid" 2>/dev/null || true
    return "$probe_status"
}

terminate_solver() {
    local reason=$1
    printf '%s\n' "$reason" > "$STOP_REASON"
    printf '%s STOP: %s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "$reason" >> "$MONITOR_LOG"
    if kill -0 "$SOLVER_PID" 2>/dev/null; then
        kill -TERM "$SOLVER_PID" 2>/dev/null || true
    fi
}

cleanup_signal() {
    terminate_solver "supervisor received a termination signal"
}
trap cleanup_signal INT TERM HUP

START_UTC=$(date -u +%Y-%m-%dT%H:%M:%SZ)
START_EPOCH=$(date +%s)
COMMIT=$(git -C "$REPO" rev-parse HEAD 2>/dev/null || printf unknown)
SOURCE_SHA=$(shasum -a 256 "$REPO/radio_sa193.c" | awk '{print $1}')
BASE_SHA=$(shasum -a 256 "$REPO/radiobase.c" | awk '{print $1}')
BINARY_SHA=$(shasum -a 256 "$BIN" | awk '{print $1}')
SUPERVISOR_SHA=$(shasum -a 256 "$0" | awk '{print $1}')
BASE_SWAP=$(sysctl -n vm.swapusage 2>/dev/null || printf unavailable)

{
    printf 'started_utc=%s\n' "$START_UTC"
    printf 'commit=%s\n' "$COMMIT"
    printf 'command=%s\n' "$BIN"
    if [ -n "$CACHE" ]; then
        printf 'cache=%s (same-run resume)\n' "$CACHE"
        printf 'cache_sha256=%s\n' "$(shasum -a 256 "$CACHE" | awk '{print $1}')"
    else
        printf 'cache=none (cold)\n'
    fi
    printf 'control=Sa(192) in 10, enabled\n'
    printf 'time_limit=none\n'
    printf 'physical_footprint_limit_gib=%s\n' "$FOOTPRINT_GIB"
    printf 'minimum_free_disk_gib=10\n'
    printf 'monitor_interval_seconds=%s\n' "$INTERVAL"
    printf 'checkpoint_interval_seconds=%s\n' "$CHECKPOINT_INTERVAL"
    printf 'vmmap_timeout_seconds=%s\n' "$VMAP_TIMEOUT"
    printf 'baseline_swap=%s\n' "$BASE_SWAP"
    printf 'radio_sa193_c_sha256=%s\n' "$SOURCE_SHA"
    printf 'radiobase_c_sha256=%s\n' "$BASE_SHA"
    printf 'binary_sha256=%s\n' "$BINARY_SHA"
    printf 'supervisor_sha256=%s\n' "$SUPERVISOR_SHA"
} > "$META"

if [ -n "$CACHE" ]; then
    "$BIN" "$CACHE" > "$OUT" 2> "$ERR" &
else
    "$BIN" > "$OUT" 2> "$ERR" &
fi
SOLVER_PID=$!
printf '%s\n' "$SOLVER_PID" > "$RUN_DIR/solver.pid"

CAFFEINATE_PID=none
if command -v caffeinate >/dev/null 2>&1; then
    caffeinate -i -m -s -w "$SOLVER_PID" >/dev/null 2>&1 &
    CAFFEINATE_PID=$!
    printf '%s\n' "$CAFFEINATE_PID" > "$RUN_DIR/caffeinate.pid"
fi

printf '%s\n' "$$" > "$RUN_DIR/supervisor.pid"
printf '%s START supervisor=%s solver=%s caffeinate=%s hard_footprint=%sGiB\n' \
    "$START_UTC" "$$" "$SOLVER_PID" "$CAFFEINATE_PID" "$FOOTPRINT_GIB" >> "$MONITOR_LOG"

LAST_CHECKPOINT=$START_EPOCH
VMAP_FAILURES=0

while kill -0 "$SOLVER_PID" 2>/dev/null; do
    NOW_UTC=$(date -u +%Y-%m-%dT%H:%M:%SZ)
    NOW_EPOCH=$(date +%s)
    ELAPSED=$((NOW_EPOCH - START_EPOCH))
    PS_FIELDS=$(ps -p "$SOLVER_PID" -o %cpu=,rss=,vsz=,state= 2>/dev/null | awk '{$1=$1; print}' || true)
    LINES=$(wc -l < "$OUT" | tr -d ' ')
    BYTES=$(wc -c < "$OUT" | tr -d ' ')
    DISK_KIB=$(df -k "$RUN_DIR" | awk 'NR == 2 {print $4}')
    SWAP=$(sysctl -n vm.swapusage 2>/dev/null || printf unavailable)
    FREE_PERCENT=$(memory_pressure 2>/dev/null | awk -F': ' '/System-wide memory free percentage/ {print $2; exit}')
    VMAP=$(bounded_vmmap)
    VMAP_STATUS=$?
    [ "$VMAP_STATUS" -eq 0 ] || VMAP=
    FOOTPRINT_RAW=$(printf '%s\n' "$VMAP" | awk '/^Physical footprint:/ {print $3; exit}')
    FOOTPRINT_BYTES=unknown

    if [ -n "$FOOTPRINT_RAW" ]; then
        FOOTPRINT_BYTES=$(human_bytes "$FOOTPRINT_RAW")
        VMAP_FAILURES=0
    else
        VMAP_FAILURES=$((VMAP_FAILURES + 1))
    fi

    {
        printf 'sample_utc=%s\n' "$NOW_UTC"
        printf 'started_utc=%s\n' "$START_UTC"
        printf 'elapsed_seconds=%s\n' "$ELAPSED"
        printf 'supervisor_pid=%s\n' "$$"
        printf 'solver_pid=%s\n' "$SOLVER_PID"
        printf 'caffeinate_pid=%s\n' "$CAFFEINATE_PID"
        printf 'ps_cpu_rss_kib_vsz_kib_state=%s\n' "$PS_FIELDS"
        printf 'physical_footprint=%s\n' "${FOOTPRINT_RAW:-unavailable}"
        printf 'physical_footprint_bytes=%s\n' "$FOOTPRINT_BYTES"
        printf 'hard_footprint_gib=%s\n' "$FOOTPRINT_GIB"
        printf 'output_lines=%s\n' "$LINES"
        printf 'output_bytes=%s\n' "$BYTES"
        printf 'free_disk_kib=%s\n' "$DISK_KIB"
        printf 'memory_free=%s\n' "${FREE_PERCENT:-unavailable}"
        printf 'swap=%s\n' "$SWAP"
        printf 'vmmap_consecutive_failures=%s\n' "$VMAP_FAILURES"
    } > "$STATUS.tmp" && mv "$STATUS.tmp" "$STATUS"

    printf '%s elapsed=%ss ps=[%s] footprint=%s lines=%s bytes=%s disk_kib=%s free=%s swap=[%s]\n' \
        "$NOW_UTC" "$ELAPSED" "$PS_FIELDS" "${FOOTPRINT_RAW:-unavailable}" "$LINES" "$BYTES" \
        "$DISK_KIB" "${FREE_PERCENT:-unavailable}" "$SWAP" >> "$MONITOR_LOG"

    if [ "$FOOTPRINT_BYTES" != unknown ] && [ "$FOOTPRINT_BYTES" -ge "$HARD_BYTES" ]; then
        terminate_solver "physical footprint ${FOOTPRINT_RAW} reached the ${FOOTPRINT_GIB} GiB ceiling"
        break
    fi
    if [ "$VMAP_FAILURES" -ge 5 ]; then
        terminate_solver "vmmap failed for five consecutive samples; the physical-memory guard is no longer trustworthy"
        break
    fi
    if [ -n "$DISK_KIB" ] && [ "$DISK_KIB" -lt "$MIN_DISK_KIB" ]; then
        terminate_solver "free disk fell below 10 GiB"
        break
    fi

    if [ $((NOW_EPOCH - LAST_CHECKPOINT)) -ge "$CHECKPOINT_INTERVAL" ]; then
        write_checkpoint
        LAST_CHECKPOINT=$NOW_EPOCH
    fi

    sleep "$INTERVAL" &
    wait $! || true
done

wait "$SOLVER_PID"
SOLVER_EXIT=$?
END_UTC=$(date -u +%Y-%m-%dT%H:%M:%SZ)
write_checkpoint
printf '%s END solver_exit=%s lines=%s bytes=%s\n' "$END_UTC" "$SOLVER_EXIT" \
    "$(wc -l < "$OUT" | tr -d ' ')" "$(wc -c < "$OUT" | tr -d ' ')" >> "$MONITOR_LOG"
{
    printf 'ended_utc=%s\n' "$END_UTC"
    printf 'solver_exit=%s\n' "$SOLVER_EXIT"
    if [ -f "$STOP_REASON" ]; then
        printf 'stop_reason=%s\n' "$(tr '\n' ' ' < "$STOP_REASON")"
    else
        printf 'stop_reason=solver exited on its own\n'
    fi
} > "$RUN_DIR/completion.txt"

exit "$SOLVER_EXIT"
