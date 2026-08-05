#!/usr/bin/env bash
# Enforce a memory cap on an ALREADY-RUNNING process, and nothing else.
#
#   tools/rss_guard.sh <pid> <cap_gb> [poll_seconds]
#
# capped_run.sh bakes its wall-clock cap into a shell variable, so a long run cannot be extended
# without restarting the process it supervises. This is the replacement for the case where the wall
# cap should be dropped but the memory cap must stay: the memory cap is the guard that matters, since
# the 2023 Sa(193) run reached ~90 GB and the OOM killer takes the checkpoint down with the process.
# Stopping it ourselves at a threshold leaves the checkpoint intact and the run resumable.
set -uo pipefail
PID=${1:?need pid}; CAP_GB=${2:?need cap in GB}; POLL=${3:-20}
CAP_KB=$(( CAP_GB * 1024 * 1024 ))
PEAK=0
echo "rss_guard: watching pid $PID, cap ${CAP_GB} GB, poll ${POLL}s, no wall-clock limit"
while kill -0 "$PID" 2>/dev/null; do
    R=$(ps -o rss= -p "$PID" 2>/dev/null | tr -d ' ')
    if [[ -n "${R:-}" ]]; then
        (( R > PEAK )) && PEAK=$R
        if (( R >= CAP_KB )); then
            echo "rss_guard: MEMORY KILL at $(( R / 1048576 )) GB (cap ${CAP_GB} GB) $(date -u +%FT%TZ)"
            kill -TERM "$PID" 2>/dev/null; sleep 30; kill -KILL "$PID" 2>/dev/null
            break
        fi
    fi
    sleep "$POLL"
done
echo "rss_guard: process $PID gone, peak $(( PEAK / 1048576 )) GB, $(date -u +%FT%TZ)"
