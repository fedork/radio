#!/usr/bin/env bash
# Run a command under a wall-clock cap and a resident-memory cap, and report what happened.
#
# macOS has neither `timeout` nor a working `ulimit -v` (the latter fails silently, which is
# worse). AGENTS.md requires every long run to be capped and every process accounted for;
# this is the thing that makes that possible without remembering to improvise each time.
#
# Usage:
#   tools/capped_run.sh --seconds 3600 --rss-gb 16 [--label NAME] -- ./radio_k9
#
# Exits with the command's own status, or 124 on timeout / 137 on memory kill, matching
# GNU timeout's convention. Peak RSS and wall time go to stderr so they survive a redirect
# of the command's stdout.
set -uo pipefail

SECONDS_CAP=3600
RSS_GB_CAP=16
LABEL=""
POLL=5

while (( $# )); do
    case "$1" in
        --seconds) SECONDS_CAP="$2"; shift 2 ;;
        --rss-gb)  RSS_GB_CAP="$2";  shift 2 ;;
        --label)   LABEL="$2";       shift 2 ;;
        --poll)    POLL="$2";        shift 2 ;;
        --) shift; break ;;
        *) echo "capped_run.sh: unknown option $1" >&2; exit 2 ;;
    esac
done

if (( $# == 0 )); then
    echo "capped_run.sh: no command given (did you forget --?)" >&2
    exit 2
fi

RSS_KB_CAP=$(( RSS_GB_CAP * 1024 * 1024 ))
[[ -n "$LABEL" ]] || LABEL="$(basename "$1")"
RUN_CONTEXT="capped_run_poll_seconds=$POLL"
if [[ -n "${RADIO_RUN_CONTEXT:-}" ]]; then
    RUN_CONTEXT="$RADIO_RUN_CONTEXT; $RUN_CONTEXT"
fi

RADIO_RUNNER=capped_run \
RADIO_RUN_LABEL="$LABEL" \
RADIO_LIMIT_WALL_SECONDS="$SECONDS_CAP" \
RADIO_LIMIT_RSS_GIB="$RSS_GB_CAP" \
RADIO_RUN_CONTEXT="$RUN_CONTEXT" \
    "$@" &
PID=$!
START=$(date +%s)
PEAK_KB=0
REASON="completed"

cleanup() {
    if kill -0 "$PID" 2>/dev/null; then
        kill -TERM "$PID" 2>/dev/null
        sleep 2
        kill -KILL "$PID" 2>/dev/null
    fi
}
trap 'REASON="interrupted"; cleanup; exit 130' INT TERM

while kill -0 "$PID" 2>/dev/null; do
    RSS_KB=$(ps -o rss= -p "$PID" 2>/dev/null | tr -d ' ')
    [[ -n "${RSS_KB:-}" ]] && (( RSS_KB > PEAK_KB )) && PEAK_KB=$RSS_KB
    NOW=$(date +%s)
    ELAPSED=$(( NOW - START ))
    if (( ELAPSED >= SECONDS_CAP )); then
        REASON="TIMEOUT after ${SECONDS_CAP}s"; cleanup; STATUS=124; break
    fi
    if [[ -n "${RSS_KB:-}" ]] && (( RSS_KB >= RSS_KB_CAP )); then
        REASON="MEMORY KILL at $(( RSS_KB / 1048576 )) GB (cap ${RSS_GB_CAP} GB)"
        cleanup; STATUS=137; break
    fi
    sleep "$POLL"
done

if [[ -z "${STATUS:-}" ]]; then
    wait "$PID"; STATUS=$?
fi
ELAPSED=$(( $(date +%s) - START ))

printf '\n[capped_run] %s: %s | exit %d | wall %ds (%dm%02ds) | peak RSS %.2f GB\n' \
    "$LABEL" "$REASON" "$STATUS" "$ELAPSED" "$((ELAPSED/60))" "$((ELAPSED%60))" \
    "$(echo "$PEAK_KB" | awk '{printf "%.2f", $1/1048576}')" >&2
exit "$STATUS"
