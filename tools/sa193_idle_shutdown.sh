#!/usr/bin/env bash
# Stop the dedicated EC2 host after every named Sa(193) solver has gone.  The grace period lets
# 10-minute watchdogs notice the exits and finish their final S3 uploads first.  EC2 is configured
# with instance-initiated shutdown behaviour `stop`, so its EBS volume remains recoverable.

set -uo pipefail

if (( $# == 0 )); then
    echo "usage: $0 SOLVER_NAME..." >&2
    exit 64
fi
for name in "$@"; do
    [[ "$name" =~ ^[a-zA-Z0-9_-]+$ ]] || { echo "invalid process name: $name" >&2; exit 64; }
done

INTERVAL=${SA193_IDLE_INTERVAL:-600}
GRACE=${SA193_IDLE_GRACE:-1200}

any_solver_alive() {
    local name
    for name in "$@"; do
        pgrep -x "$name" >/dev/null 2>&1 && return 0
    done
    return 1
}

while :; do
    while any_solver_alive "$@"; do
        sleep "$INTERVAL"
    done

    printf '%s no tracked solver remains; beginning %ss final-upload grace period\n' \
        "$(date -u +%FT%TZ)" "$GRACE"
    sleep "$GRACE"
    if any_solver_alive "$@"; then
        printf '%s a tracked solver returned; cancelling shutdown\n' "$(date -u +%FT%TZ)"
        continue
    fi

    printf '%s stopping the completed dedicated Sa(193) instance\n' "$(date -u +%FT%TZ)"
    shutdown -h now
    exit 0
done
