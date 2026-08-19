#!/usr/bin/env bash
# One-shot progress for the run9 verifier A/B sequence. Bounded: makes at most one SSM round trip
# and then exits, so it is safe to run repeatedly and never leaves a polling process behind.
#
# Two things are worth knowing before reading the output:
#
#   * A level's log is uploaded to S3 only when that level CLOSES. k=7 is ~99% of each run's cost
#     and has no intra-level checkpoint, so while it is in flight S3 shows nothing new. Live k=7
#     progress exists only on the instance, which is why this script also asks the host directly.
#   * If the instance is stopped, the chainer finished everything and powered it down on purpose.
#     Final numbers are then in S3 and no SSM call is attempted.
#
# Usage: tools/run9_progress.sh [--instance ID]
set -euo pipefail

BUCKET=radio-sa193-393287594714
INSTANCE=i-04126f6d3016378a9
# label:run-id pairs, in execution order.
RUNS=(
    "selected-ordinary:20260819T013030Z"
    "trimmed-ordinary:20260819T020000Z"
)

while (( $# )); do
    case "$1" in
        --instance) INSTANCE=$2; shift 2 ;;
        *) echo "usage: $0 [--instance ID]" >&2; exit 64 ;;
    esac
done

aws_cmd=(aws-vault exec --server default -- aws)
s3get() { "${aws_cmd[@]}" s3 cp "s3://$BUCKET/$1" - --no-progress 2>/dev/null || true; }

state=$("${aws_cmd[@]}" ec2 describe-instances --instance-ids "$INSTANCE" \
    --query 'Reservations[0].Instances[0].State.Name' --output text 2>/dev/null || echo unknown)
printf '=== instance %s: %s ===\n' "$INSTANCE" "$state"
if [[ "$state" == stopped ]]; then
    echo '    (the chainer stops the host after the last run; final numbers are in S3 below)'
elif [[ "$state" == terminated ]]; then
    echo '    WARNING: terminated, not stopped. Anything not already in S3 is gone.'
fi

for entry in "${RUNS[@]}"; do
    label=${entry%%:*}
    run=${entry##*:}
    prefix="run9-$label/$run"
    printf '\n=== %s  run=%s ===\n' "$label" "$run"
    status=$(s3get "$prefix/STATUS")
    final=$(s3get "$prefix/verify.total")
    if [[ -z "$status" && -z "$final" ]]; then
        echo '    not started (no STATUS in S3 yet)'
        continue
    fi
    [[ -n "$status" ]] && printf '%s\n' "$status" | \
        grep -E '^(updated_utc|stage|instance_id)' | sed 's/^/    /'
    timings=$(s3get "$prefix/timings.tsv")
    if [[ -n "$timings" ]]; then
        echo '    closed levels:'
        printf '%s\n' "$timings" | sed 's/^/      /'
        printf '%s\n' "$timings" | awk -F '\t' 'NR>1 && $4!="NA" {s+=$4}
            END {if (s) printf "      subtotal %.3f CPU s\n", s}'
    fi
    if [[ -n "$final" ]]; then
        echo '    FINAL:'
        printf '%s\n' "$final" | sed 's/^/      /'
    fi
done

if [[ "$state" != running ]]; then
    printf '\n(no SSM call: instance is %s)\n' "$state"
    exit 0
fi

# One bounded remote look: active stage, the live progress line of whichever level is running,
# the chainer's state, and a health check.
remote=$(cat <<'EOS'
set -u
for d in /root/run9-selected-ordinary-* /root/run9-trimmed-ordinary-*; do
    [ -d "$d" ] || continue
    echo "--- $(basename "$d") stage=$(cat "$d/stage" 2>/dev/null || echo ?) ---"
    lv=$(sed -n 's/^VERIFY_K//p' "$d/stage" 2>/dev/null)
    if [ -n "${lv:-}" ] && [ -f "$d/verify-k$lv.out" ]; then
        grep -E "^PROGRESS phase" "$d/verify-k$lv.out" | tail -1
    fi
done
echo "--- chainer ---"
tail -3 /root/chain-trimmed.log 2>/dev/null || echo "(no chainer log)"
echo "--- health ---"
ps -o etimes=,%cpu=,rss= -C run9_refute 2>/dev/null | \
    awk '{printf "run9_refute elapsed_s=%s cpu=%s%% rss_mib=%.1f\n",$1,$2,$3/1024}' || echo "(no verifier running)"
awk '/^MemAvailable:/ {printf "mem_avail_gib=%.1f\n",$2/1048576}' /proc/meminfo
free -g 2>/dev/null | awk '/^Swap:/ {print "swap_used_gib="$3}'
df -Pk / | awk 'NR==2 {printf "disk_avail_gib=%.1f\n",$4/1048576}'
EOS
)
b64=$(printf '%s' "$remote" | base64 | tr -d '\n')
tmp=$(mktemp); trap 'rm -f "$tmp"' EXIT
python3 - "$tmp" "$INSTANCE" "$b64" <<'PY'
import json, sys
path, instance, b64 = sys.argv[1], sys.argv[2], sys.argv[3]
json.dump({"InstanceIds": [instance], "DocumentName": "AWS-RunShellScript",
           "Parameters": {"commands": ["echo %s | base64 -d | bash" % b64]}}, open(path, "w"))
PY
cmd=$("${aws_cmd[@]}" ssm send-command --cli-input-json "file://$tmp" \
    --query Command.CommandId --output text 2>/dev/null || true)
if [[ -z "$cmd" ]]; then
    echo; echo '(SSM send-command failed; instance may still be booting its agent)'
    exit 0
fi
printf '\n=== live on host (SSM %s) ===\n' "$cmd"
for _ in $(seq 1 30); do
    s=$("${aws_cmd[@]}" ssm get-command-invocation --command-id "$cmd" \
        --instance-id "$INSTANCE" --query Status --output text 2>/dev/null || true)
    [[ "$s" == Success || "$s" == Failed ]] && break
    sleep 4
done
"${aws_cmd[@]}" ssm get-command-invocation --command-id "$cmd" --instance-id "$INSTANCE" \
    --query StandardOutputContent --output text 2>/dev/null | sed 's/^/    /'
