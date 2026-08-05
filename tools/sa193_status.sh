#!/usr/bin/env bash
# Show the Sa(193) run's status, and when it was last written.
#
#   tools/sa193_status.sh          # the current snapshot
#   tools/sa193_status.sh --watch  # refresh every 60 s
#
# `aws s3 cp <src> -` is the incantation: cp needs a destination, and `-` means stdout. Forgetting
# the `-` gives "the following arguments are required: paths", which is what this script exists to
# save you from.
#
# `age` matters as much as the contents. The watchdog writes every 10 minutes, so a snapshot up to
# ten minutes old is normal and says nothing is wrong; one that is hours old means the watchdog died
# even though the solver may still be running fine.
set -uo pipefail
BUCKET=radio-sa193-393287594714
INSTANCE=i-0005d74f985c52ae1

show() {
    local mod age now
    mod=$(aws-vault exec default -- aws s3api head-object --bucket "$BUCKET" --key run/STATUS \
            --query LastModified --output text 2>/dev/null)
    aws-vault exec default -- aws s3 cp "s3://$BUCKET/run/STATUS" - 2>/dev/null
    if [[ -n "$mod" ]]; then
        now=$(date -u +%s)
        age=$(( now - $(date -u -j -f "%Y-%m-%dT%H:%M:%S" "${mod%%+*}" +%s 2>/dev/null || echo "$now") ))
        printf '\n  status written     %s  (%d min ago%s)\n' "$mod" $(( age / 60 )) \
            "$( (( age > 1800 )) && printf ', STALE - watchdog may be dead' )"
    fi
    printf '  instance           %s\n' \
        "$(aws-vault exec default -- aws ec2 describe-instances --instance-ids "$INSTANCE" \
             --query 'Reservations[].Instances[].State.Name' --output text 2>/dev/null)"
}

if [[ "${1:-}" == "--watch" ]]; then
    while :; do clear; show; sleep 60; done
else
    show
fi
