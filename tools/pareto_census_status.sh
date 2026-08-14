#!/usr/bin/env bash
# One-shot status query for the resumed k=8 Pareto-prefix census on the shared AWS host.
set -euo pipefail

INSTANCE=i-0005d74f985c52ae1
BUCKET=radio-sa193-393287594714
PREFIX=pareto-census-k8/20260814T0132Z
WORK=/root/pareto-census-k8-20260814T0132Z

if (( $# )); then
    echo "usage: $0" >&2
    exit 64
fi

aws_cmd=(aws-vault exec --server default -- aws)
instance_state=$("${aws_cmd[@]}" ec2 describe-instances --instance-ids "$INSTANCE" \
    --query 'Reservations[0].Instances[0].State.Name' --output text 2>/dev/null || true)
printf 'k=8 Pareto-prefix census  instance=%s state=%s\n' "$INSTANCE" "${instance_state:-unknown}"

if [[ "$instance_state" != running ]]; then
    if "${aws_cmd[@]}" s3 ls "s3://$BUCKET/$PREFIX/STATUS" >/dev/null 2>&1; then
        "${aws_cmd[@]}" s3 cp "s3://$BUCKET/$PREFIX/STATUS" - --no-progress
    else
        echo "no saved STATUS object at s3://$BUCKET/$PREFIX/STATUS"
    fi
    exit 0
fi

read -r -d '' remote <<'EOF' || true
set -u
work=/root/pareto-census-k8-20260814T0132Z
date -u +updated_utc=%FT%TZ
if [[ ! -d "$work" ]]; then
    echo state=not-staged
    exit 0
fi
state=stopped
if [[ -s "$work/solver.pid" ]] && kill -0 "$(cat "$work/solver.pid")" 2>/dev/null; then
    state=running
elif [[ -s "$work/exit.status" ]]; then
    state=finished
fi
echo state=$state
for label in supervisor wrapper solver joint_guard idle_guard; do
    file="$work/${label}.pid"
    [[ -s "$file" ]] || continue
    pid=$(cat "$file")
    if kill -0 "$pid" 2>/dev/null; then
        ps -p "$pid" -o pid=,ppid=,etimes=,rss=,vsz=,stat=,comm= | \
            awk -v label="$label" '{printf "%s_pid=%s ppid=%s elapsed_s=%s rss_mib=%.1f vsz_mib=%.1f stat=%s comm=%s\n", label,$1,$2,$3,$4/1024,$5/1024,$6,$7}'
    else
        echo "${label}_pid=$pid gone"
    fi
done
awk '/^MemTotal:/ {total=$2} /^MemAvailable:/ {avail=$2} END {printf "host_mem_used_gib=%.1f host_mem_available_gib=%.1f\n", (total-avail)/1048576, avail/1048576}' /proc/meminfo
df -Pk "$work" | awk 'NR==2 {printf "disk_available_gib=%.1f\n", $4/1048576}'
if [[ -s "$work/out.txt" ]]; then
    awk -F '\t' '
        /^CENSUS\tSECOND_SUMMARY\t/ {second++; if ($0 ~ /memo_imported=0/) fresh++}
        /^CENSUS\tPREFIX_SUMMARY\t/ {prefix=1}
        /^CENSUS\tTARGET\t/ {target++}
        /^CENSUS\tENDPOINT\t/ {endpoint++}
        /^CENSUS\tFULL_STATE\t/ {full_state++}
        /^CENSUS\tFULL_WIN\t/ {full_win++}
        /^CENSUS\tEND\t/ {end++}
        END {
            printf "second_blocks=%d/815 fresh_second_blocks=%d/70 prefix_summary=%d targets=%d endpoints=%d full_states=%d full_winners=%d end_records=%d\n", second,fresh,prefix,target,endpoint,full_state,full_win,end
        }
    ' "$work/out.txt"
    wc -l -c "$work/out.txt" | awk '{printf "output_lines=%s output_mib=%.2f\n", $1, $2/1048576}'
    echo 'last_first:'
    grep '^CENSUS[[:space:]]FIRST[[:space:]]' "$work/out.txt" | tail -n 1 || true
    echo 'last_closed_block:'
    grep '^CENSUS[[:space:]]SECOND_SUMMARY[[:space:]]' "$work/out.txt" | tail -n 1 || true
    echo 'last_solver_progress:'
    grep '^still solving in ' "$work/out.txt" | tail -n 1 || true
fi
[[ -f "$work/exit.status" ]] && echo "exit_status=$(cat "$work/exit.status")"
[[ -s "$work/solver.err" ]] && { echo 'solver_err_tail:'; tail -n 5 "$work/solver.err"; }
echo s3=s3://radio-sa193-393287594714/pareto-census-k8/20260814T0132Z/
EOF

parameters=$(python3 -c 'import json,sys; print(json.dumps({"commands":[sys.stdin.read()]}))' <<<"$remote")
command_id=$("${aws_cmd[@]}" ssm send-command --instance-ids "$INSTANCE" \
    --document-name AWS-RunShellScript --comment 'one-shot k8 Pareto census status' \
    --parameters "$parameters" --query 'Command.CommandId' --output text)

status=Pending
for _ in $(seq 1 15); do
    status=$("${aws_cmd[@]}" ssm get-command-invocation --command-id "$command_id" \
        --instance-id "$INSTANCE" --query Status --output text 2>/dev/null || true)
    case "$status" in Success|Failed|TimedOut|Cancelled) break ;; esac
    sleep 2
done
if [[ "$status" != Success ]]; then
    echo "status query $command_id ended as $status" >&2
    "${aws_cmd[@]}" ssm get-command-invocation --command-id "$command_id" \
        --instance-id "$INSTANCE" --query StandardErrorContent --output text >&2 || true
    exit 1
fi
"${aws_cmd[@]}" ssm get-command-invocation --command-id "$command_id" \
    --instance-id "$INSTANCE" --query StandardOutputContent --output text
