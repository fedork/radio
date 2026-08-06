#!/usr/bin/env bash
# Replace the watchdog on the running instance, correctly.
#
#   tools/sa193_restart_watchdog.sh
#
# Two failure modes this exists to prevent, both hit on 2026-08-05:
#
#   1. **Overwriting the script a running bash is executing.** bash re-reads a script from a byte
#      offset as it goes, so editing the file underneath a live process can make it execute garbage.
#      Every restart therefore stages a NEW filename, `tools/wd-<HHMMSS>.sh`.
#   2. **A restart that silently does not stick.** One `pkill; start` left the OLD watchdog running
#      and the new one dead - reported success, and the pre-fix code kept running for 90 minutes.
#      So: kill by PID until none remain, assert zero, start one, assert exactly one and that its
#      age is small.
#
# The solver is never touched. Killing the watchdog is safe: user-data has already fallen through to
# waiting on the solver, so nothing shuts down.
set -euo pipefail
cd "$(dirname "$0")/.."

BUCKET=radio-sa193-393287594714
TOPIC=arn:aws:sns:us-west-2:393287594714:radio-sa193-progress
INSTANCE=${INSTANCE:-i-0005d74f985c52ae1}

aws-vault exec --server default -- aws s3 cp tools/sa193_watchdog.sh "s3://$BUCKET/src/sa193_watchdog.sh" >/dev/null
echo "staged watchdog to s3"

cat > /tmp/sa193_restart.json <<JSON
{"commands":[
 "cd /root/run",
 "V=wd-\$(date -u +%H%M%S).sh",
 "aws s3 cp s3://$BUCKET/src/sa193_watchdog.sh \\"tools/\$V\\" && chmod +x \\"tools/\$V\\" && bash -n \\"tools/\$V\\" && echo \\"staged tools/\$V\\"",
 "for p in \$(ps -eo pid,args | grep -E 'sa193_watchdog|wd-[0-9]+[.]sh' | grep -v grep | awk '{print \$1}'); do kill -9 \\"\$p\\" 2>/dev/null && echo \\"killed \$p\\"; done",
 "sleep 3",
 "N=\$(ps -eo args | grep -E 'sa193_watchdog|wd-[0-9]+[.]sh' | grep -v grep | wc -l); test \\"\$N\\" -eq 0 || { echo \\"ABORT: \$N watchdogs still alive\\"; exit 1; }",
 "S=\$(pgrep -x radio_sa193 | head -1); test -n \\"\$S\\" || { echo 'ABORT: no solver'; exit 1; }",
 "setsid nohup env SEG=\${SEG:-seg1-detached} PROFILE=/root/run/memprofile.csv \\"tools/\$V\\" --log /root/run/out_sa193.txt --pid \\"\$S\\" --bucket $BUCKET --topic $TOPIC --interval 600 --heartbeat 21600 >> /var/log/sa193-watchdog.log 2>&1 < /dev/null &",
 "sleep 12",
 "N=\$(ps -eo args | grep -E 'wd-[0-9]+[.]sh' | grep -v grep | wc -l); test \\"\$N\\" -eq 1 || { echo \\"ABORT: \$N watchdogs after start\\"; exit 1; }",
 "ps -eo pid,ppid,etimes,args --sort=pid | grep -E 'wd-[0-9]+[.]sh' | grep -v grep | cut -c1-75",
 "echo \\"OK solver=\$(pgrep -x radio_sa193|wc -l) guard=\$(pgrep -f rss_guard.sh|wc -l)\\""
]}
JSON

CID=$(aws-vault exec --server default -- aws ssm send-command --instance-ids "$INSTANCE" \
        --document-name AWS-RunShellScript --parameters file:///tmp/sa193_restart.json \
        --timeout-seconds 120 --query 'Command.CommandId' --output text)
sleep 35
aws-vault exec --server default -- aws ssm get-command-invocation --command-id "$CID" \
    --instance-id "$INSTANCE" --query '[Status,StandardOutputContent]' --output text
