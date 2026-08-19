#!/usr/bin/env bash
# One-shot status for the selected-input ordinary verifier A/B. Reads S3 only; starts no polling.
#
# The run measures the third point of the coloring cost comparison:
#   complete input + ordinary verifier   211,335.569 CPU s  (run 20260818T194508Z)
#   selected input + colored verifier    218,792.627 CPU s  (run 20260818T205010Z)
#   selected input + ordinary verifier   this run, predicted ~205,111
set -euo pipefail

BUCKET=radio-sa193-393287594714
RUN_ID=${1:-}
[[ "$RUN_ID" =~ ^[0-9]{8}T[0-9]{6}Z$ ]] || { echo "usage: $0 RUN_ID" >&2; exit 64; }
PREFIX="run9-selected-ordinary/$RUN_ID"
aws_cmd=(aws-vault exec --server default -- aws)

get() { "${aws_cmd[@]}" s3 cp "s3://$BUCKET/$PREFIX/$1" - --no-progress 2>/dev/null || true; }

printf 'run9 selected-input ordinary verifier  run=%s\n' "$RUN_ID"
status=$(get STATUS)
[[ -n "$status" ]] || { echo "no STATUS at s3://$BUCKET/$PREFIX/"; exit 0; }
printf '%s\n' "$status" | grep -E '^(updated_utc|stage|instance_id|host_mem_available_gib|disk_available_gib|completed_k)'

echo
echo "TIMINGS"
timings=$(get timings.tsv)
printf '%s\n' "$timings"

final=$(get verify.total)
if [[ -n "$final" ]]; then
    echo
    echo "FINAL"
    printf '%s\n' "$final"
    exit 0
fi

# Not finished: k=7 dominates, so surface its live progress line if the log is up.
echo
echo "k7 IN FLIGHT (no per-level log is uploaded until the level closes)"
awk -F '\t' 'NR>1 {s+=$4} END {if (s) printf "  closed levels so far: %.3f CPU s\n", s}' <<<"$timings"
cat <<'NOTE'
  k=7 is 99.2% of the total cost and has no intra-level checkpoint; expect ~3.5 h wall.
  Live per-level progress stays on the instance until the level closes. To watch it directly:
    aws ssm send-command --instance-ids <id> --document-name AWS-RunShellScript \
      --parameters commands='tail -3 /root/run9-selected-ordinary-RUNID/verify-k7.out'
NOTE
