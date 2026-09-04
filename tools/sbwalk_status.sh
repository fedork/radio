#!/usr/bin/env bash
# Read the durable status of an sbwalk run - the k=10 Sb vertical scan that decides Sa(11).
#
#   tools/sbwalk_status.sh                     # the newest run
#   tools/sbwalk_status.sh 20260904T205613Z    # a named run
#   tools/sbwalk_status.sh --watch             # refresh every 60 s
#   tools/sbwalk_status.sh --live              # also ask the instance for resident memory
#
# `age` matters as much as the contents: the heartbeat writes every ten minutes, so that much age
# is normal and says nothing is wrong. Hours means the uploader died even if the solver is alive.
#
# Read the **instance** line, never the `alive` count, to decide whether a run is still going. On
# 2026-09-04 run10's last STATUS said "solver process alive" and "13 of 16" while the run had in
# fact finished and the host had been powered off - a STATUS object is the last snapshot, not the
# outcome. See evidence/run10_completion_2026-09-04.md.
set -uo pipefail

REGION=us-west-2
BUCKET=radio-sa193-393287594714
RUN_ID=
WATCH=0
LIVE=0
while (( $# )); do
    case "$1" in
        --watch) WATCH=1; shift ;;
        --live) LIVE=1; shift ;;
        -h|--help) sed -n '2,16p' "$0"; exit 0 ;;
        *) [[ -z "$RUN_ID" ]] || { echo "usage: $0 [RUN_ID] [--watch] [--live]" >&2; exit 2; }
           RUN_ID="$1"; shift ;;
    esac
done
[[ -z "$RUN_ID" || "$RUN_ID" =~ ^[0-9]{8}T[0-9]{6}Z$ ]] || { echo "invalid run id: $RUN_ID" >&2; exit 2; }

AWS=(aws-vault exec --server default -- aws)

render() {
    local run_id="$1"
    if [[ -z "$run_id" ]]; then
        # Newest prefix that actually has a STATUS; an input-only prefix is a cache upload, not a run.
        local p
        for p in $("${AWS[@]}" s3 ls "s3://$BUCKET/sbwalk/" --region "$REGION" 2>/dev/null \
                   | awk '$1 == "PRE" {gsub("/","",$2); print $2}' | sort -r); do
            if "${AWS[@]}" s3 ls "s3://$BUCKET/sbwalk/$p/STATUS" --region "$REGION" >/dev/null 2>&1; then
                run_id="$p"; break
            fi
        done
        [[ -n "$run_id" ]] || { echo "no sbwalk run with a STATUS object found"; return 1; }
    fi

    echo "K=10 Sb vertical scan (decides Sa(11) = max_m [ min(192, n(10,m)) + m ])"
    echo "  run id              $run_id"
    echo "  durable prefix      s3://$BUCKET/sbwalk/$run_id/"

    local body
    body=$("${AWS[@]}" s3 cp "s3://$BUCKET/sbwalk/$run_id/STATUS" - --region "$REGION" 2>/dev/null)
    if [[ -z "$body" ]]; then
        echo "  STATUS              not written yet (the heartbeat's first upload is ~10 min in)"
    else
        local written age_s
        written=$("${AWS[@]}" s3 ls "s3://$BUCKET/sbwalk/$run_id/STATUS" --region "$REGION" 2>/dev/null \
                  | awk '{print $1" "$2}')
        if [[ -n "$written" ]]; then
            age_s=$(( $(date +%s) - $(date -j -f "%Y-%m-%d %H:%M:%S" "$written" +%s 2>/dev/null \
                    || date -d "$written" +%s 2>/dev/null || echo 0) ))
            if (( age_s > 0 && age_s < 86400 )); then
                printf '  status written      %s local (%d min ago%s)\n' "$written" $(( age_s / 60 )) \
                    "$( (( age_s > 2400 )) && echo ', STALE - uploader may be dead')"
            else
                echo "  status written      $written local"
            fi
        fi
        echo
        printf '%s\n' "$body" | sed 's/^/  /'
    fi

    echo
    local inst
    inst=$("${AWS[@]}" ec2 describe-instances --region "$REGION" \
        --filters "Name=tag:RunPrefix,Values=sbwalk/$run_id" \
        --query 'Reservations[].Instances[].[InstanceId,State.Name,InstanceType]' --output text 2>/dev/null)
    if [[ -n "$inst" ]]; then
        echo "  instance            $inst"
        # Resident memory is the standing risk on a multi-day walk and the heartbeat does not
        # report it: capped_run kills a walker at --rss-gb, and the trie only grows.
        if (( LIVE )) && [[ "$inst" == *running* ]]; then
            local iid cid out
            iid=${inst%%$'\t'*}
            cid=$("${AWS[@]}" ssm send-command --region "$REGION" --instance-ids "$iid" \
                --document-name AWS-RunShellScript --comment 'sbwalk live rss' \
                --parameters '{"commands":["ps -o rss=,etime= -C radio_sb_walk"]}' \
                --query 'Command.CommandId' --output text 2>/dev/null)
            if [[ -n "$cid" ]]; then
                sleep 6
                out=$("${AWS[@]}" ssm get-command-invocation --region "$REGION" \
                    --command-id "$cid" --instance-id "$iid" \
                    --query 'StandardOutputContent' --output text 2>/dev/null)
                [[ -n "$out" ]] && printf '  live rss/elapsed    %s\n' "$(echo "$out" | tr -s ' \n' ' ')"
            fi
        fi
    else
        echo "  instance            not found (terminated, or launched under a different tag)"
    fi
    # Name the log objects that actually exist; hardcoding a walker name goes stale the moment
    # the run starts at a different m.
    local logs
    logs=$("${AWS[@]}" s3 ls "s3://$BUCKET/sbwalk/$run_id/logs/" --region "$REGION" 2>/dev/null \
           | awk '{print $4}' | grep -E '\.txt\.zst$' | tr '\n' ' ')
    if [[ -n "${logs// /}" ]]; then
        local one=${logs%% *}
        echo "  logs                $logs"
        echo "  read one            aws s3 cp s3://$BUCKET/sbwalk/$run_id/logs/$one - | zstd -dc | tail"
    else
        echo "  logs                none uploaded yet"
    fi
    echo
    echo "  A WALK line is the answer of record. Its absence is UNKNOWN, never a refutation:"
    echo "  a cell is only decided when its own WALK line prints."
}

if (( WATCH )); then
    while true; do clear; render "$RUN_ID"; sleep 60; done
else
    render "$RUN_ID"
fi
