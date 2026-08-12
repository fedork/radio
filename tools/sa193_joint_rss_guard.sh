#!/usr/bin/env bash
# Bound the combined RSS of concurrent Sa(193) solvers.  If the host-wide ceiling is reached, stop
# only the newest sidecar wrapper; its capped_run trap then terminates that solver cleanly while the
# older, more valuable cold sessions remain untouched.
set -uo pipefail

if (( $# < 3 )); then
    echo "usage: $0 CAP_GIB VICTIM_WRAPPER_PID SOLVER_NAME..." >&2
    exit 64
fi
CAP_GIB=$1
VICTIM_WRAPPER=$2
shift 2
SOLVER_NAMES=("$@")
POLL=${SA193_JOINT_RSS_POLL:-5}

[[ "$CAP_GIB" =~ ^[0-9]+$ && "$CAP_GIB" -gt 0 ]] || { echo "invalid cap" >&2; exit 64; }
[[ "$VICTIM_WRAPPER" =~ ^[0-9]+$ && "$VICTIM_WRAPPER" -gt 1 ]] || {
    echo "invalid victim wrapper pid" >&2
    exit 64
}
[[ "$POLL" =~ ^[0-9]+$ && "$POLL" -gt 0 ]] || { echo "invalid poll interval" >&2; exit 64; }
for name in "${SOLVER_NAMES[@]}"; do
    [[ "$name" =~ ^[a-zA-Z0-9_-]+$ ]] || { echo "invalid solver name: $name" >&2; exit 64; }
done

CAP_KIB=$((CAP_GIB * 1024 * 1024))
PEAK_KIB=0
echo "joint_rss_guard: cap ${CAP_GIB} GiB; victim wrapper $VICTIM_WRAPPER; solvers ${SOLVER_NAMES[*]}"

while kill -0 "$VICTIM_WRAPPER" 2>/dev/null; do
    total=0
    for name in "${SOLVER_NAMES[@]}"; do
        while read -r pid; do
            [[ -n "$pid" ]] || continue
            rss=$(awk '/^VmRSS:/ {print $2; exit}' "/proc/$pid/status" 2>/dev/null || true)
            [[ "$rss" =~ ^[0-9]+$ ]] && total=$((total + rss))
        done < <(pgrep -x "$name" 2>/dev/null || true)
    done
    (( total > PEAK_KIB )) && PEAK_KIB=$total
    if (( total >= CAP_KIB )); then
        echo "joint_rss_guard: stopping newest sidecar at $((total / 1048576)) GiB combined RSS $(date -u +%FT%TZ)"
        kill -TERM "$VICTIM_WRAPPER" 2>/dev/null || true
        exit 137
    fi
    sleep "$POLL"
done

echo "joint_rss_guard: victim wrapper gone; peak combined RSS $((PEAK_KIB / 1048576)) GiB $(date -u +%FT%TZ)"
