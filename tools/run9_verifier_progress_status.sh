#!/usr/bin/env bash
# One-shot (or watched) status for the latest dedicated verifier with completed-target telemetry.
set -euo pipefail

REGION=us-west-2
BUCKET=radio-sa193-393287594714
WATCH=0
RUN_ID=
while (( $# )); do
    case "$1" in
        --watch) WATCH=1; shift ;;
        -*) echo "usage: $0 [--watch] [RUN_ID]" >&2; exit 64 ;;
        *) [[ -z "$RUN_ID" ]] || { echo "usage: $0 [--watch] [RUN_ID]" >&2; exit 64; }
           RUN_ID=$1; shift ;;
    esac
done
[[ -z "$RUN_ID" || "$RUN_ID" =~ ^[0-9]{8}T[0-9]{6}Z$ ]] || { echo 'invalid RUN_ID' >&2; exit 64; }

aws_cmd=(aws-vault exec --server default -- aws)

show_status() {
    local query row instance state itype launched discovered prefix status_text
    if [[ -n "$RUN_ID" ]]; then
        query=(Name=tag:Purpose,Values=run9-verifier-progress Name=tag:RunId,Values="$RUN_ID")
    else
        query=(Name=tag:Purpose,Values=run9-verifier-progress)
    fi
    row=$("${aws_cmd[@]}" ec2 describe-instances --region "$REGION" --filters "${query[@]}" \
        Name=instance-state-name,Values=pending,running,stopping,stopped \
        --query 'sort_by(Reservations[].Instances[], &LaunchTime)[-1].[InstanceId,State.Name,InstanceType,LaunchTime,Tags[?Key==`RunId`]|[0].Value]' \
        --output text 2>/dev/null || true)
    if [[ -z "$row" || "$row" == None* ]]; then
        echo 'no matching dedicated verifier instance found'
        return 1
    fi
    read -r instance state itype launched discovered <<<"$row"
    [[ -n "$RUN_ID" ]] || RUN_ID=$discovered
    prefix="run9-verifier-progress/$RUN_ID"
    printf 'run9 verifier progress  run_id=%s instance=%s state=%s type=%s launched=%s\n' \
        "$RUN_ID" "$instance" "$state" "$itype" "$launched"
    if "${aws_cmd[@]}" s3 ls "s3://$BUCKET/$prefix/STATUS" >/dev/null 2>&1; then
        status_text=$("${aws_cmd[@]}" s3 cp "s3://$BUCKET/$prefix/STATUS" - --no-progress)
        printf '%s\n' "$status_text"
        printf '%s\n' "$status_text" | awk '
            /^PROGRESS phase=/ {
                done = total = rate = 0
                for (i = 1; i <= NF; i++) {
                    split($i, kv, "=")
                    if (kv[1] == "completed") {
                        split(kv[2], counts, "/")
                        done = counts[1] + 0
                        total = counts[2] + 0
                    } else if (kv[1] == "rate_window") {
                        sub(/\/s$/, "", kv[2])
                        rate = kv[2] + 0
                    }
                }
            }
            END {
                if (rate > 0 && total > done) {
                    remaining = total - done
                    eta = remaining / rate
                    printf "LATEST_WINDOW_PROJECTION remaining=%d rate_window=%.3f/s eta_window_s=%.0f eta_window_h=%.2f eta_window_d=%.2f basis=latest_interval_only\n", remaining, rate, eta, eta / 3600, eta / 86400
                }
            }'
        if "${aws_cmd[@]}" s3 ls "s3://$BUCKET/$prefix/PROGRESS" >/dev/null 2>&1; then
            echo
            echo 'RECENT_PROGRESS'
            "${aws_cmd[@]}" s3 cp "s3://$BUCKET/$prefix/PROGRESS" - --no-progress | tail -n 36
        fi
    else
        echo "state=bootstrapping; no STATUS object yet"
    fi
    printf 's3=s3://%s/%s/\n' "$BUCKET" "$prefix"
}

if (( WATCH )); then
    while :; do
        clear || true
        show_status || true
        sleep 60
    done
else
    show_status
fi
