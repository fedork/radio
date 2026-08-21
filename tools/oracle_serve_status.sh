#!/usr/bin/env bash
# One-shot progress/health check for an oracle-serve instance. Reads the STATUS object the host
# uploads every 5 minutes. Does not poll and does not touch the running server.
#
#   tools/oracle_serve_status.sh [RUN_ID]      # newest run if RUN_ID is omitted
set -euo pipefail
cd "$(dirname "$0")/.."

REGION=us-west-2
BUCKET=radio-sa193-393287594714
aws_cmd=(aws-vault exec --server default -- aws)

RUN_ID=${1:-}
if [[ -z "$RUN_ID" ]]; then
    RUN_ID=$("${aws_cmd[@]}" s3 ls "s3://$BUCKET/oracle-serve/" | awk '{print $2}' | tr -d / | sort | tail -1)
    [[ -n "$RUN_ID" ]] || { echo 'no oracle-serve runs found' >&2; exit 1; }
fi
PREFIX="oracle-serve/$RUN_ID"

state=$("${aws_cmd[@]}" ec2 describe-instances --region "$REGION" \
    --filters Name=tag:RunId,Values="$RUN_ID" Name=tag:Purpose,Values=oracle-serve \
    --query 'Reservations[].Instances[].[InstanceId,InstanceType,State.Name]' --output text)
printf 'run_id=%s\ninstance: %s\n\n' "$RUN_ID" "${state:-<terminated or not found>}"

if "${aws_cmd[@]}" s3 ls "s3://$BUCKET/$PREFIX/STATUS" >/dev/null 2>&1; then
    "${aws_cmd[@]}" s3 cp "s3://$BUCKET/$PREFIX/STATUS" - --no-progress
else
    echo 'no STATUS yet -- still bootstrapping (build takes a few minutes).'
    "${aws_cmd[@]}" s3 ls "s3://$BUCKET/$PREFIX/bootstrap-failure.log" >/dev/null 2>&1 && \
        echo 'a bootstrap-failure.log exists; fetch it for the reason.'
fi

echo
echo "to reach it:"
instance_id=$(echo "$state" | awk '{print $1}')
if [[ -n "$instance_id" ]]; then
    port=$(echo "$state" >/dev/null; echo 7777)  # default; check STATUS's port= line if changed
    printf '  aws-vault exec --server default -- aws ssm start-session --target %s \\\n' "$instance_id"
    printf '    --document-name AWS-StartPortForwardingSession \\\n'
    printf '    --parameters "{\\"portNumber\\":[\\"%s\\"],\\"localPortNumber\\":[\\"%s\\"]}"\n' "$port" "$port"
fi
echo
echo "to stop it (billing pauses, EBS persists):"
echo "  aws-vault exec --server default -- aws ec2 stop-instances --instance-ids $instance_id"
echo "to terminate it (EBS also persists -- DeleteOnTermination=false):"
echo "  aws-vault exec --server default -- aws ec2 terminate-instances --instance-ids $instance_id"
