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

verifier_alive=0
verifier_pid=
if [[ -s "$work/verifier.pid" ]]; then
    verifier_pid=$(cat "$work/verifier.pid")
    kill -0 "$verifier_pid" 2>/dev/null && verifier_alive=1
fi
verifier_elapsed_s=0
verifier_cpu='?'
if (( verifier_alive )); then
    verifier_elapsed_s=$(ps -p "$verifier_pid" -o etimes= | tr -d ' ')
    verifier_cpu=$(ps -p "$verifier_pid" -o %cpu= | tr -d ' ')
fi
duration() {
    local seconds=$1
    printf '%dd%02dh%02dm%02ds' "$((seconds / 86400))" "$((seconds / 3600 % 24))" \
        "$((seconds / 60 % 60))" "$((seconds % 60))"
}

echo
echo PROGRESS
case "$stage" in
    SANITIZE)
        echo '  step=1/4 normalize and byte-round-trip the raw log'
        echo '  now=normalization is still running'
        echo '  next=pre-color minimalization, top-down coloring, full replay'
        ;;
    COLOR)
        if grep -q '^--- pass ' "$work/color.out" 2>/dev/null; then
            read -r color_levels last_color_k color_targets color_verified <<<"$(
                awk '
                    /^--- pass / { levels=targets=verified=0; last=""; inpass=1; next }
                    inpass && $1 ~ /^[0-9]+$/ && $2 ~ /^[0-9]+$/ && $3 ~ /^[0-9]+$/ {
                        levels++; last=$1; targets+=$3; verified+=$4
                    }
                    END { printf "%d %s %d %d\n", levels, (last=="" ? 0 : last), targets, verified }
                ' "$work/color.out"
            )"
            echo '  step=3/4 top-down coloring from the 16 explicit roots'
            echo '  done=normalization, round-trip, and all pre-color minimalization'
            printf '  coloring_milestone=%s/9 level barriers complete; %s targets verified so far\n' \
                "$color_levels" "$color_verified"
            if (( color_levels < 9 )); then
                if (( color_levels == 0 )); then current_color_k=9; else current_color_k=$((last_color_k - 1)); fi
                printf '  now=coloring k=%s; this level reports only when its whole batch finishes\n' \
                    "$current_color_k"
            else
                echo '  now=coloring complete; preparing the colored certificate'
            fi
            echo '  next=full independent replay of the colored bundle'
        else
            read -r minimized_levels last_minimized_k completed_inputs <<<"$(
                awk '
                    $1 ~ /^k=[0-9]+:$/ && $2 ~ /^[0-9]+$/ {
                        k=$1; sub(/^k=/, "", k); sub(/:$/, "", k)
                        levels++; last=k; inputs+=$2
                    }
                    END { printf "%d %s %d\n", levels, (last=="" ? 1 : last), inputs }
                ' "$work/color.out" 2>/dev/null
            )"
            support_inputs=3126174
            input_pct=$(awk -v done="$completed_inputs" -v total="$support_inputs" \
                'BEGIN { printf "%.1f", 100*done/total }')
            current_minimized_k=$((last_minimized_k + 1))
            case "$current_minimized_k" in
                2) current_inputs=2 ;;
                3) current_inputs=137 ;;
                4) current_inputs=33042 ;;
                5) current_inputs=125246 ;;
                6) current_inputs=388317 ;;
                7) current_inputs=2576885 ;;
                8) current_inputs=2545 ;;
                *) current_inputs='?' ;;
            esac
            echo '  step=2/4 remove same-level redundant support facts before coloring'
            echo '  done=normalization and byte-identical round-trip'
            if (( minimized_levels )); then
                printf '  completed_levels=k=2..%s (%s/%s input facts in completed levels = %s%%)\n' \
                    "$last_minimized_k" "$completed_inputs" "$support_inputs" "$input_pct"
            else
                printf '  completed_levels=none (0/%s input facts)\n' "$support_inputs"
            fi
            if (( last_minimized_k >= 8 )); then
                echo '  now=all support levels minimalized; transitioning to top-down coloring'
            else
                printf '  now=minimalizing k=%s (%s input facts)\n' \
                    "$current_minimized_k" "$current_inputs"
            fi
            echo '  progress_limit=the running binary has no intra-level counter; the percentage above is'
            echo '                 only a completed-level record fraction, not elapsed-time or total-job progress'
            echo '  eta=unavailable until the current level finishes and prints its milestone'
            echo '  next=finish remaining support levels, top-down coloring, full replay'
        fi
        ;;
    VERIFY)
        echo '  step=4/4 independently replay the complete colored certificate'
        echo '  done=normalization, minimalization, and top-down coloring'
        if [[ -s "$work/color.fact_count" && -s "$work/color.root_count" ]]; then
            printf '  bundle=%s roots plus %s support facts\n' \
                "$(cat "$work/color.root_count")" "$(cat "$work/color.fact_count")"
        fi
        echo '  now=parallel replay in one shared task queue'
        echo '  progress_limit=the running binary reports the replay total only when the batch finishes'
        echo '  eta=unavailable from this build'
        ;;
    COMPLETE)
        echo '  step=4/4 complete'
        [[ -s "$work/verify.total" ]] && printf '  result=%s\n' "$(cat "$work/verify.total")"
        [[ -s "$work/exit.status" ]] && printf '  exit_status=%s\n' "$(cat "$work/exit.status")"
        ;;
    *)
        echo '  step=unknown; inspect the detailed process and log output below'
        ;;
esac
if [[ "$state" == finished || "$stage" == COMPLETE ]]; then
    echo '  health=FINISHED'
elif (( verifier_alive )); then
    printf '  health=WORKING: verifier PID %s, CPU=%s%%, elapsed=' "$verifier_pid" "$verifier_cpu"
    duration "$verifier_elapsed_s"
    echo
else
    echo '  health=no verifier process visible (this can be a short phase transition; inspect supervisor)'
fi

echo
echo DETAILS
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
