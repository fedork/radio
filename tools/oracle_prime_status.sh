#!/usr/bin/env bash
# One-shot progress check for an oracle-prime run. Reads the STATUS object the host uploads every
# 60 s, so it works whether the instance is running, stopped or already gone.
#
#   tools/oracle_prime_status.sh [RUN_ID]      # newest run if RUN_ID is omitted
#
# It does not poll and it does not touch the run. Re-run it whenever you want a reading.
set -euo pipefail
cd "$(dirname "$0")/.."

REGION=us-west-2
BUCKET=radio-sa193-393287594714
aws_cmd=(aws-vault exec --server default -- aws)

RUN_ID=${1:-}
if [[ -z "$RUN_ID" ]]; then
    RUN_ID=$("${aws_cmd[@]}" s3 ls "s3://$BUCKET/oracle-prime/" | awk '{print $2}' | tr -d / | sort | tail -1)
    [[ -n "$RUN_ID" ]] || { echo 'no oracle-prime runs found' >&2; exit 1; }
fi
PREFIX="oracle-prime/$RUN_ID"

state=$("${aws_cmd[@]}" ec2 describe-instances --region "$REGION" \
    --filters Name=tag:RunId,Values="$RUN_ID" Name=tag:Purpose,Values=oracle-prime \
    --query 'Reservations[].Instances[].[InstanceId,InstanceType,State.Name]' --output text)
printf 'run_id=%s\ninstance: %s\n\n' "$RUN_ID" "${state:-<terminated or not found>}"

if "${aws_cmd[@]}" s3 ls "s3://$BUCKET/$PREFIX/STATUS" >/dev/null 2>&1; then
    "${aws_cmd[@]}" s3 cp "s3://$BUCKET/$PREFIX/STATUS" - --no-progress
else
    echo 'no STATUS yet — the host is still bootstrapping (build + sort take a few minutes).'
    "${aws_cmd[@]}" s3 ls "s3://$BUCKET/$PREFIX/bootstrap-failure.log" >/dev/null 2>&1 && \
        echo 'a bootstrap-failure.log exists; fetch it for the reason.'
fi

echo
echo "artifacts so far:"
"${aws_cmd[@]}" s3 ls "s3://$BUCKET/$PREFIX/" --human-readable | sed 's/^/  /'
echo
echo "when it finishes, the snapshot is  s3://$BUCKET/$PREFIX/cache.snap.zst"
echo "to stop it early:  aws-vault exec --server default -- aws ec2 terminate-instances \\"
echo "                     --region $REGION --instance-ids <id from above>"
