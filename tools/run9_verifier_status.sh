#!/usr/bin/env bash
# One-shot status query for the independent run9 coloring and replay on the shared AWS host.
set -euo pipefail

INSTANCE=i-0005d74f985c52ae1
BUCKET=radio-sa193-393287594714
PREFIX=run9-verifier/20260817T163700Z
WORK=/root/run9-verifier-20260817T163700Z

if (( $# )); then
    echo "usage: $0" >&2
    exit 64
fi

aws_cmd=(aws-vault exec --server default -- aws)
instance_state=$("${aws_cmd[@]}" ec2 describe-instances --instance-ids "$INSTANCE" \
    --query 'Reservations[0].Instances[0].State.Name' --output text 2>/dev/null || true)
printf 'run9 independent verifier  instance=%s state=%s\n' "$INSTANCE" "${instance_state:-unknown}"

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
work=/root/run9-verifier-20260817T163700Z
date -u +updated_utc=%FT%TZ
if [[ ! -d "$work" ]]; then
    echo state=not-staged
    exit 0
fi
stage=$(cat "$work/stage" 2>/dev/null || echo UNKNOWN)
echo stage=$stage
state=running
[[ -s "$work/exit.status" ]] && state=finished
echo state=$state
for label in supervisor wrapper verifier idle_guard; do
    file="$work/${label}.pid"
    [[ -s "$file" ]] || continue
    pid=$(cat "$file")
    if kill -0 "$pid" 2>/dev/null; then
        threads=$(awk '/^Threads:/ {print $2}' "/proc/$pid/status" 2>/dev/null || echo '?')
        ps -p "$pid" -o pid=,ppid=,ni=,psr=,%cpu=,rss=,vsz=,etimes=,stat=,comm= | \
            awk -v label="$label" -v threads="$threads" '{printf "%s_pid=%s ppid=%s nice=%s cpu_now=%s cpu_pct=%s rss_mib=%.1f vsz_mib=%.1f elapsed_s=%s stat=%s comm=%s threads=%s\n", label,$1,$2,$3,$4,$5,$6/1024,$7/1024,$8,$9,$10,threads}'
        [[ "$label" == verifier ]] && taskset -pc "$pid" 2>/dev/null | sed 's/^/verifier_/'
    else
        echo "${label}_pid=$pid gone"
    fi
done
grep -E '^(MemTotal|MemAvailable|SwapTotal|SwapFree):' /proc/meminfo | \
    awk '{v[$1]=$2} END {printf "host_mem_used_gib=%.1f host_mem_available_gib=%.1f swap_used_gib=%.1f\n", (v["MemTotal:"]-v["MemAvailable:"])/1048576, v["MemAvailable:"]/1048576, (v["SwapTotal:"]-v["SwapFree:"])/1048576}'
uptime | sed 's/^/host_uptime=/'
df -Pk "$work" | awk 'NR==2 {printf "disk_available_gib=%.1f\n", $4/1048576}'

census=/root/pareto-census-k8-20260814T0132Z/solver.pid
if [[ -s "$census" ]]; then
    pid=$(cat "$census")
    if kill -0 "$pid" 2>/dev/null; then
        ps -p "$pid" -o pid=,%cpu=,rss=,etimes=,stat=,comm= | \
            awk '{printf "existing_census_pid=%s cpu_pct=%s rss_mib=%.1f elapsed_s=%s stat=%s comm=%s\n",$1,$2,$3/1024,$4,$5,$6}'
    else
        echo existing_census=gone
    fi
fi

for file in run9-sanitized.cert run9-colored.cert sanitize.out color.out verify.out; do
    [[ -f "$work/$file" ]] || continue
    stat -c "file=$file bytes=%s mtime=%y" "$work/$file"
done
[[ -s "$work/sanitize.fact_count" ]] && echo sanitized_facts=$(cat "$work/sanitize.fact_count")
[[ -s "$work/color.root_count" ]] && echo colored_roots=$(cat "$work/color.root_count")
[[ -s "$work/color.fact_count" ]] && echo colored_facts=$(cat "$work/color.fact_count")
[[ -s "$work/verify.total" ]] && echo verify_total=$(cat "$work/verify.total")
[[ -s "$work/exit.status" ]] && echo exit_status=$(cat "$work/exit.status")

case "$stage" in
    SANITIZE) out=sanitize.out; err=sanitize.err ;;
    COLOR) out=color.out; err=color.err ;;
    VERIFY|COMPLETE) out=verify.out; err=verify.err ;;
    *) out=supervisor.log; err=supervisor.log ;;
esac
if [[ -s "$work/$out" ]]; then
    echo "${out}_tail:"
    tail -n 24 "$work/$out"
fi
if [[ -s "$work/$err" ]]; then
    echo "${err}_tail:"
    tail -n 8 "$work/$err"
fi
echo s3=s3://radio-sa193-393287594714/run9-verifier/20260817T163700Z/
EOF

params=$(python3 -c 'import json,sys; print(json.dumps({"commands":[sys.stdin.read()]}))' <<<"$remote")
command_id=$("${aws_cmd[@]}" ssm send-command --instance-ids "$INSTANCE" \
    --document-name AWS-RunShellScript --comment 'one-shot run9 verifier status' \
    --parameters "$params" --query 'Command.CommandId' --output text)

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
