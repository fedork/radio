#!/usr/bin/env bash
# One-shot status for the dedicated cleanroom certificate-verification host.
#
#   tools/cleanroom_ec2_status.sh [RUN_ID] [--follow [SECONDS]]
#
# With no RUN_ID it reports the most recently launched cleanroom-verify instance.
# --follow re-polls until the run reaches DONE or FAILED (default every 300 s).
set -euo pipefail

REGION=us-west-2
BUCKET=radio-sa193-393287594714
RUN_ID=
FOLLOW=0
INTERVAL=300

while (( $# )); do
    case "$1" in
        --follow) FOLLOW=1; shift; if [[ ${1:-} =~ ^[0-9]+$ ]]; then INTERVAL=$1; shift; fi ;;
        -h|--help) sed -n '2,8p' "$0"; exit 0 ;;
        *)
            [[ "$1" =~ ^[0-9]{8}T[0-9]{6}Z$ ]] || { echo "usage: $0 [RUN_ID] [--follow [SECONDS]]" >&2; exit 64; }
            RUN_ID=$1; shift ;;
    esac
done

aws_cmd=(aws-vault exec --server default -- aws)

# On-demand us-west-2 rates for the types this pipeline uses; only for a rough running cost.
hourly_rate() {
    case "$1" in
        c8a.4xlarge) echo 0.6912 ;;
        c8a.8xlarge) echo 1.3824 ;;
        c8a.16xlarge) echo 2.7648 ;;
        *) echo 0 ;;
    esac
}

report_once() {
    local filters=(Name=tag:Purpose,Values=cleanroom-verify)
    [[ -z "$RUN_ID" ]] || filters+=(Name=tag:RunId,Values="$RUN_ID")
    local row
    row=$("${aws_cmd[@]}" ec2 describe-instances --region "$REGION" --filters "${filters[@]}" \
        Name=instance-state-name,Values=pending,running,stopping,stopped,shutting-down,terminated \
        --query 'sort_by(Reservations[].Instances[], &LaunchTime)[-1].[InstanceId,State.Name,InstanceType,LaunchTime,Tags[?Key==`RunId`]|[0].Value]' \
        --output text 2>/dev/null || true)
    if [[ -z "$row" || "$row" == None* ]]; then
        echo 'no matching cleanroom-verify instance found'
        return 1
    fi
    local instance state itype launched discovered
    read -r instance state itype launched discovered <<<"$row"
    [[ -n "$RUN_ID" ]] || RUN_ID=$discovered
    local prefix="cleanroom-verify/$RUN_ID"

    local elapsed="" cost=""
    # BSD date -j -f ignores the trailing offset and assumes local time, so force TZ=UTC.
    if launched_epoch=$(TZ=UTC date -j -f '%Y-%m-%dT%H:%M:%S+00:00' "$launched" +%s 2>/dev/null \
            || date -d "$launched" +%s 2>/dev/null); then
        local secs=$(( $(date -u +%s) - launched_epoch ))
        elapsed=$(printf '%dh%02dm' $(( secs / 3600 )) $(( secs % 3600 / 60 )))
        local rate; rate=$(hourly_rate "$itype")
        [[ "$rate" == 0 ]] || cost=$(awk -v s="$secs" -v r="$rate" 'BEGIN{printf "$%.2f", s/3600*r}')
    fi

    printf 'cleanroom certificate verification  run_id=%s\n' "$RUN_ID"
    printf '  instance=%s state=%s type=%s launched=%s' "$instance" "$state" "$itype" "$launched"
    [[ -z "$elapsed" ]] || printf ' elapsed=%s' "$elapsed"
    [[ -z "$cost" ]] || printf ' on_demand_cost~%s' "$cost"
    echo

    local stages
    stages=$("${aws_cmd[@]}" s3 cp "s3://$BUCKET/$prefix/stages.log" - --no-progress 2>/dev/null || true)
    if [[ -z "$stages" ]]; then
        echo '  stage=bootstrapping; no stages.log yet (Rust toolchain install takes ~1 min)'
    else
        echo '  STAGES'
        sed 's/^/    /' <<<"$stages"
    fi

    # The verify log carries the provenance header, then BUILD/RESULT_LEVEL/TOTAL as they land.
    # It is re-uploaded every 600 s while the audit runs, so this tail is the progress view.
    local verify
    verify=$("${aws_cmd[@]}" s3 cp "s3://$BUCKET/$prefix/verify.out" - --no-progress 2>/dev/null || true)
    if [[ -n "$verify" ]]; then
        echo '  VERIFY (last upload, refreshed every 10 min)'
        grep -E '^(BUILD|PROGRESS|STATS_NP|RESULT_LEVEL|TOTAL|GAP|# started_utc|# finished_utc|# host|Elapsed|Maximum resident)' \
            <<<"$verify" | tail -n 20 | sed 's/^/    /' || true
        if ! grep -q '^BUILD' <<<"$verify"; then
            echo '    (index build in progress: minutes, single-threaded, before the parallel audit)'
        fi
    fi

    local failure
    failure=$("${aws_cmd[@]}" s3 cp "s3://$BUCKET/$prefix/bootstrap-failure.log" - --no-progress 2>/dev/null || true)
    if [[ -n "$failure" ]]; then
        echo '  BOOTSTRAP FAILURE (tail)'
        tail -n 20 <<<"$failure" | sed 's/^/    /'
    fi

    if "${aws_cmd[@]}" s3 ls "s3://$BUCKET/$prefix/final.sha256" >/dev/null 2>&1; then
        echo '  FINAL'
        "${aws_cmd[@]}" s3 cp "s3://$BUCKET/$prefix/final.sha256" - --no-progress 2>/dev/null \
            | sed 's/^/    /'
    fi

    printf '  s3=s3://%s/%s/\n' "$BUCKET" "$prefix"
    if [[ "$state" == stopped ]]; then
        printf '  NOTE: the host stopped itself; terminate it to release the EBS volume:\n'
        printf '    aws-vault exec --server default -- aws ec2 terminate-instances --region %s --instance-ids %s\n' \
            "$REGION" "$instance"
    fi

    grep -qE 'DONE|FAILED' <<<"$stages"
}

if (( FOLLOW )); then
    while true; do
        date -u +'=== %Y-%m-%dT%H:%M:%SZ'
        if report_once; then
            echo 'terminal stage reached'
            exit 0
        fi
        sleep "$INTERVAL"
    done
else
    report_once || true
fi
