#!/usr/bin/env bash
# Secondary, non-owning guard for an already-running local Sa(193) solver.  It was added after the
# active supervisor script was accidentally edited in place: Bash may have buffered the old tail
# and read the new tail later, so its finalizer cannot be assumed reliable even while its loop keeps
# working.  This process normally does nothing.  If the primary supervisor disappears first, it
# takes over the physical-footprint cap; after the solver exits it always writes an independent
# closed checkpoint without overwriting the primary's files.
set -u

if [ "$#" -ne 5 ]; then
    echo "usage: $0 RUN_DIR SOLVER_PID SUPERVISOR_PID SOURCE_CHECKPOINT FOOTPRINT_GIB" >&2
    exit 64
fi

RUN_DIR=$(cd "$1" 2>/dev/null && pwd) || exit 66
SOLVER_PID=$2
SUPERVISOR_PID=$3
SOURCE_CHECKPOINT=$4
FOOTPRINT_GIB=$5
REPO=${RADIO_REPO:-$(cd "$(dirname "$0")/.." && pwd)}
PARSE_OUT=${RADIO_PARSE_OUT:-$REPO/parse_out.sh}
OUT="$RUN_DIR/out_sa193.txt"
LOG="$RUN_DIR/recovery_guard.log"
RECOVERED="$RUN_DIR/sa193.recovery.checkpoint"
HARD_BYTES=$(awk -v gib="$FOOTPRINT_GIB" 'BEGIN { printf "%.0f", gib * 1024 * 1024 * 1024 }')

[[ "$SOLVER_PID" =~ ^[0-9]+$ && "$SUPERVISOR_PID" =~ ^[0-9]+$ ]] || exit 64
[[ "$FOOTPRINT_GIB" =~ ^[0-9]+$ && "$FOOTPRINT_GIB" -gt 0 ]] || exit 64
[ -f "$SOURCE_CHECKPOINT" ] || { echo "missing source checkpoint" >&2; exit 66; }
[ -f "$OUT" ] || { echo "missing solver output" >&2; exit 66; }

human_bytes() {
    awk -v value="$1" 'BEGIN {
        gsub(/[+,]/, "", value)
        unit = substr(value, length(value), 1)
        if (unit ~ /[KMGT]/) number = substr(value, 1, length(value) - 1) + 0
        else { number = value + 0; unit = "B" }
        multiplier = 1
        if (unit == "K") multiplier = 1024
        if (unit == "M") multiplier = 1024 * 1024
        if (unit == "G") multiplier = 1024 * 1024 * 1024
        if (unit == "T") multiplier = 1024 * 1024 * 1024 * 1024
        printf "%.0f", number * multiplier
    }'
}

probe_mem() {
    local probe="$RUN_DIR/recovery_guard.top" probe_pid timer_pid status
    /usr/bin/top -l 1 -pid "$SOLVER_PID" -stats pid,mem > "$probe" 2>/dev/null &
    probe_pid=$!
    ( sleep 20; kill -TERM "$probe_pid" 2>/dev/null || exit 0; sleep 1;
      kill -KILL "$probe_pid" 2>/dev/null || true ) &
    timer_pid=$!
    wait "$probe_pid"; status=$?
    kill -TERM "$timer_pid" 2>/dev/null || true
    wait "$timer_pid" 2>/dev/null || true
    [ "$status" -eq 0 ] || return 1
    awk -v pid="$SOLVER_PID" '$1 == pid {print $2; exit}' "$probe"
}

printf '%s guard started solver=%s supervisor=%s\n' "$(date -u +%FT%TZ)" \
    "$SOLVER_PID" "$SUPERVISOR_PID" >> "$LOG"
while kill -0 "$SOLVER_PID" 2>/dev/null; do
    if ! kill -0 "$SUPERVISOR_PID" 2>/dev/null; then
        footprint=$(probe_mem || true)
        if [ -n "$footprint" ]; then
            bytes=$(human_bytes "$footprint")
            printf '%s primary gone; takeover footprint=%s\n' "$(date -u +%FT%TZ)" \
                "$footprint" >> "$LOG"
            if [ "$bytes" -ge "$HARD_BYTES" ]; then
                printf '%s killing solver at recovery cap %s GiB\n' "$(date -u +%FT%TZ)" \
                    "$FOOTPRINT_GIB" >> "$LOG"
                kill -TERM "$SOLVER_PID" 2>/dev/null || true
            fi
        else
            printf '%s primary gone; footprint probe failed\n' "$(date -u +%FT%TZ)" >> "$LOG"
        fi
    fi
    sleep 120 & wait $! || true
done

# Give the primary finalizer a chance to finish, but never depend on it.
for _ in $(seq 1 30); do
    kill -0 "$SUPERVISOR_PID" 2>/dev/null || break
    sleep 2
done

tmp="$RECOVERED.tmp"
{
    printf '# Recovery checkpoint generated independently at %s\n' "$(date -u +%FT%TZ)"
    printf '# Resume source: %s\n' "$SOURCE_CHECKPOINT"
    cat "$SOURCE_CHECKPOINT"
    "$PARSE_OUT" < "$OUT"
} > "$tmp" && mv "$tmp" "$RECOVERED"

{
    printf 'ended_utc=%s\n' "$(date -u +%FT%TZ)"
    printf 'solver_pid=%s\n' "$SOLVER_PID"
    printf 'primary_supervisor_pid=%s\n' "$SUPERVISOR_PID"
    printf 'recovery_checkpoint=%s\n' "$RECOVERED"
    printf 'recovery_checkpoint_sha256=%s\n' "$(shasum -a 256 "$RECOVERED" | awk '{print $1}')"
    printf 'primary_completion_present=%s\n' "$([ -f "$RUN_DIR/completion.txt" ] && echo yes || echo no)"
} > "$RUN_DIR/recovery_guard.completion"
printf '%s recovery checkpoint complete\n' "$(date -u +%FT%TZ)" >> "$LOG"
