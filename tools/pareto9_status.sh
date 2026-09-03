#!/usr/bin/env bash
# Read the durable status of the newest (or named) K=9 Pareto AWS walk.
set -euo pipefail

region=us-west-2
bucket=radio-sa193-393287594714
run_id=
watch=0
while (( $# )); do
    case "$1" in
        --watch) watch=1; shift ;;
        *) [[ -z "$run_id" ]] || { echo "usage: $0 [RUN_ID] [--watch]" >&2; exit 2; }
           run_id=$1; shift ;;
    esac
done
[[ -z "$run_id" || "$run_id" =~ ^[0-9]{8}T[0-9]{6}Z$ ]] || { echo 'invalid run id' >&2; exit 2; }

aws_cmd=(aws-vault exec --server default -- aws)
script_dir=$(cd "$(dirname "$0")" && pwd)

render_slowest_facts() {
    local instance=$1 prefix=$2 resolved_run_id encoded remote ssm_parameters command_id status
    resolved_run_id=${prefix##*/}
    [[ "$resolved_run_id" =~ ^[0-9]{8}T[0-9]{6}Z$ ]] || {
        echo '  slowest facts       unavailable (invalid run id in instance tag)'
        return
    }
    [[ -f "$script_dir/pareto_slowest_facts.py" ]] || {
        echo '  slowest facts       unavailable (local analyzer missing)'
        return
    }

    encoded=$(base64 < "$script_dir/pareto_slowest_facts.py" | tr -d '\n')
    remote="python3 -c 'import base64; exec(compile(base64.b64decode(\"$encoded\"), \"pareto_slowest_facts.py\", \"exec\"))' --limit 10 /root/pareto9-$resolved_run_id/seg-*.out"
    ssm_parameters=$(python3 -c \
        'import json,sys; print(json.dumps({"commands":[sys.stdin.read()]}))' <<<"$remote")
    if ! command_id=$("${aws_cmd[@]}" ssm send-command --region "$region" \
            --instance-ids "$instance" --document-name AWS-RunShellScript \
            --comment 'rank live Pareto fact timings' --parameters "$ssm_parameters" \
            --query 'Command.CommandId' --output text 2>/dev/null); then
        echo '  slowest facts       unavailable (SSM command could not be sent)'
        return
    fi

    status=Pending
    for _ in $(seq 1 15); do
        status=$("${aws_cmd[@]}" ssm get-command-invocation --region "$region" \
            --command-id "$command_id" --instance-id "$instance" \
            --query Status --output text 2>/dev/null || true)
        case "$status" in Success|Failed|TimedOut|Cancelled) break ;; esac
        sleep 1
    done
    if [[ "$status" != Success ]]; then
        printf '  slowest facts       unavailable (SSM status %s)\n' "$status"
        return
    fi
    "${aws_cmd[@]}" ssm get-command-invocation --region "$region" \
        --command-id "$command_id" --instance-id "$instance" \
        --query StandardOutputContent --output text
}

render() {
    local query instance prefix state modified
    query=(Name=tag:Purpose,Values=pareto9-frontier)
    [[ -z "$run_id" ]] || query+=(Name=tag:RunId,Values="$run_id")
    instance=$("${aws_cmd[@]}" ec2 describe-instances --region "$region" \
        --filters "${query[@]}" Name=instance-state-name,Values=pending,running,stopping,stopped \
        --query 'sort_by(Reservations[].Instances[], &LaunchTime)[-1].InstanceId' --output text)
    [[ -n "$instance" && "$instance" != None ]] || { echo 'no matching Pareto instance'; return 1; }
    prefix=$("${aws_cmd[@]}" ec2 describe-instances --region "$region" --instance-ids "$instance" \
        --query 'Reservations[0].Instances[0].Tags[?Key==`RunPrefix`]|[0].Value' --output text)
    state=$("${aws_cmd[@]}" ec2 describe-instances --region "$region" --instance-ids "$instance" \
        --query 'Reservations[0].Instances[0].State.Name' --output text)
    if "${aws_cmd[@]}" s3api head-object --region "$region" --bucket "$bucket" \
        --key "$prefix/STATUS" >/dev/null 2>&1; then
        "${aws_cmd[@]}" s3 cp "s3://$bucket/$prefix/STATUS" - --no-progress
        modified=$("${aws_cmd[@]}" s3api head-object --region "$region" --bucket "$bucket" \
            --key "$prefix/STATUS" --query LastModified --output text)
        printf '\n  status object       %s\n' "$modified"
    else
        echo 'K=9 Pareto frontier walk'
        echo '  stage               EC2 bootstrap (STATUS not uploaded yet)'
    fi
    printf '  instance            %s  %s\n' "$instance" "$state"
    printf '  durable prefix      s3://%s/%s/\n' "$bucket" "$prefix"
    if [[ "$state" == running ]]; then
        printf '\n'
        render_slowest_facts "$instance" "$prefix"
    else
        echo '  slowest facts       unavailable (instance is not running)'
    fi
}

while :; do
    render
    (( watch )) || break
    sleep 60
    printf '\n'
done
