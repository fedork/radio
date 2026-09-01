#!/usr/bin/env bash
# Show the durable AWS status for the full K=6 distance-14 production-solver census.
#
#   tools/singleton_k6_survey_status.sh
#   tools/singleton_k6_survey_status.sh --watch
set -euo pipefail

bucket=${RADIO_K6_SURVEY_BUCKET:-radio-sa193-393287594714}
prefix=${RADIO_K6_SURVEY_PREFIX:-run10/k6-main-survey}
instance=${RADIO_K6_SURVEY_INSTANCE:-}
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
    local modified state
    aws-vault exec --server default -- aws s3 cp "s3://$bucket/$prefix/STATUS" -
    modified=$(aws-vault exec --server default -- aws s3api head-object \
        --bucket "$bucket" --key "$prefix/STATUS" --query LastModified --output text)
    if [[ -z "$instance" ]]; then
        instance=$(aws-vault exec --server default -- aws ec2 describe-instances \
            --region us-west-2 \
            --filters Name=tag:Purpose,Values=k6-main-survey \
                      Name=instance-state-name,Values=pending,running,stopping,stopped \
            --query 'sort_by(Reservations[].Instances[], &LaunchTime)[-1].InstanceId' \
            --output text)
    fi
    if [[ -z "$instance" || "$instance" == None ]]; then
        instance=none
        state=none
    else
        state=$(aws-vault exec --server default -- aws ec2 describe-instances \
            --region us-west-2 --instance-ids "$instance" \
            --query 'Reservations[0].Instances[0].State.Name' --output text)
    fi
    printf '\n  status object      %s\n' "$modified"
    printf '  instance           %s  %s\n' "$instance" "$state"
    printf '  durable prefix     s3://%s/%s/\n' "$bucket" "$prefix"
}

while :; do
    render
    (( watch )) || break
    sleep 60
    printf '\n'
done
