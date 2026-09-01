#!/usr/bin/env bash
# Show the durable AWS status for the full K=6 distance-14 production-solver census.
#
#   tools/singleton_k6_survey_status.sh
#   tools/singleton_k6_survey_status.sh --watch
set -euo pipefail

bucket=${RADIO_K6_SURVEY_BUCKET:-radio-sa193-393287594714}
prefix=${RADIO_K6_SURVEY_PREFIX:-run10/k6-main-survey}
instance=${RADIO_K6_SURVEY_INSTANCE:-}
asg=${RADIO_K6_SURVEY_ASG:-radio-k6-main-survey}
watch=0

if (( $# > 1 )); then
    echo "usage: $0 [--watch]" >&2
    exit 2
fi
if (( $# == 1 )); then
    [[ "$1" == --watch ]] || { echo "usage: $0 [--watch]" >&2; exit 2; }
    watch=1
fi

render() {
    local modified state resolved_instance asg_state
    aws-vault exec --server default -- aws s3 cp "s3://$bucket/$prefix/STATUS" -
    modified=$(aws-vault exec --server default -- aws s3api head-object \
        --bucket "$bucket" --key "$prefix/STATUS" --query LastModified --output text)
    resolved_instance=$instance
    if [[ -z "$resolved_instance" ]]; then
        resolved_instance=$(aws-vault exec --server default -- aws ec2 describe-instances \
            --region us-west-2 \
            --filters Name=tag:Purpose,Values=k6-main-survey \
                      Name=instance-state-name,Values=pending,running \
            --query 'sort_by(Reservations[].Instances[], &LaunchTime)[-1].InstanceId' \
            --output text)
    fi
    if [[ -z "$resolved_instance" || "$resolved_instance" == None ]]; then
        resolved_instance=none
        state=none
    else
        state=$(aws-vault exec --server default -- aws ec2 describe-instances \
            --region us-west-2 --instance-ids "$resolved_instance" \
            --query 'Reservations[0].Instances[0].State.Name' --output text)
    fi
    asg_state=$(aws-vault exec --server default -- aws autoscaling describe-auto-scaling-groups \
        --region us-west-2 --auto-scaling-group-names "$asg" \
        --query 'AutoScalingGroups[0].[DesiredCapacity,length(Instances[?LifecycleState==`InService`])]' \
        --output text 2>/dev/null || true)
    [[ -n "$asg_state" && "$asg_state" != None* ]] || asg_state='not configured'
    printf '\n  status object      %s\n' "$modified"
    printf '  instance           %s  %s\n' "$resolved_instance" "$state"
    printf '  auto replacement   %s  (desired, in-service)\n' "$asg_state"
    printf '  durable prefix     s3://%s/%s/\n' "$bucket" "$prefix"
}

while :; do
    render
    (( watch )) || break
    sleep 60
    printf '\n'
done
