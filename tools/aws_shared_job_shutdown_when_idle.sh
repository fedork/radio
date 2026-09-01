#!/usr/bin/env bash
# Stop the shared AWS host only after both intended long-running jobs have ended.
set -euo pipefail

while pgrep -x radio_sa193 >/dev/null || systemctl is-active --quiet radio-k6-survey.service; do
    sleep 60
done
logger -t radio-shared-shutdown "Sa(193) and K=6 survey ended; stopping host"
exec /usr/sbin/shutdown -h now
