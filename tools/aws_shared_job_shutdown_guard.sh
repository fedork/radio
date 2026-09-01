#!/usr/bin/env bash
# Installed as /usr/local/bin/shutdown on a shared search host. Defer cloud-init's final shutdown
# while the K=6 survey is active; /root/k6-survey/shutdown-when-idle invokes the real binary after
# both the survey and Sa(193) have ended.
set -euo pipefail

if systemctl is-active --quiet radio-k6-survey.service; then
    logger -t radio-shared-shutdown "deferred shutdown while radio-k6-survey is active"
    exit 0
fi
exec /usr/sbin/shutdown "$@"
