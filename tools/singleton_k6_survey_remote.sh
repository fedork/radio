#!/usr/bin/env bash
# Run the complete K=6 transfer shell as restartable rank stages on an AWS host.  There is no time
# or search-budget cutoff: each oracle query uses budget 0.  A virtual-memory ceiling turns resource
# exhaustion into an abort, never a verdict.  Completed stages and NEXT_RANK are uploaded durably.
set -euo pipefail

if (( $# != 5 )); then
    echo "usage: $0 BUCKET PREFIX START_RANK END_RANK STAGE_SIZE" >&2
    exit 2
fi
bucket=$1
prefix=$2
start_rank=$3
end_rank=$4
stage_size=$5
for value in "$start_rank" "$end_rank" "$stage_size"; do
    [[ "$value" =~ ^[0-9]+$ ]] || { echo "ranks and stage size must be integers" >&2; exit 2; }
done
(( start_rank < end_rank && stage_size > 0 )) || { echo "invalid rank interval" >&2; exit 2; }

work_dir=${RADIO_K6_SURVEY_DIR:-/root/k6-survey}
cpu=${RADIO_K6_SURVEY_CPU:-0}
vm_kib=${RADIO_K6_SURVEY_VM_KIB:-16777216}
status_interval=${RADIO_K6_STATUS_INTERVAL:-60}
progress_interval=100000
[[ "$status_interval" =~ ^[1-9][0-9]*$ ]] || {
    echo "RADIO_K6_STATUS_INTERVAL must be a positive integer" >&2
    exit 2
}
mkdir -p "$work_dir/stages"
cd "$work_dir"

next_file="$work_dir/NEXT_RANK"
if aws s3 cp "s3://$bucket/$prefix/NEXT_RANK" "$next_file.remote" >/dev/null 2>&1; then
    next_rank=$(tr -d '[:space:]' < "$next_file.remote")
else
    next_rank=$start_rank
fi
[[ "$next_rank" =~ ^[0-9]+$ ]] || { echo "invalid NEXT_RANK checkpoint" >&2; exit 2; }
(( next_rank >= start_rank && next_rank <= end_rank )) || {
    echo "NEXT_RANK outside requested interval" >&2
    exit 2
}

format_duration() {
    local seconds=$1
    if (( seconds >= 86400 )); then
        printf '%dd %02dh %02dm' $((seconds / 86400)) $((seconds % 86400 / 3600)) \
            $((seconds % 3600 / 60))
    elif (( seconds >= 3600 )); then
        printf '%dh %02dm' $((seconds / 3600)) $((seconds % 3600 / 60))
    elif (( seconds >= 60 )); then
        printf '%dm %02ds' $((seconds / 60)) $((seconds % 60))
    else
        printf '%ds' "$seconds"
    fi
}

write_status() {
    local state=$1 stage_end=$2 stage_started=$3 now rss_kib elapsed sa_rss
    local progress_file progress_queries stage_count overall_completed stage_percent overall_percent
    local rate stage_eta full_eta
    now=$(date -u +%FT%TZ)
    elapsed=$(( $(date +%s) - stage_started ))
    rss_kib=$(ps -C k6-oracle -o rss= 2>/dev/null | awk '{sum += $1} END {print sum + 0}')
    sa_rss=$(ps -C radio_sa193 -o rss= 2>/dev/null | awk '{sum += $1} END {print sum + 0}')
    progress_file="$work_dir/ACTIVE_PROGRESS"
    progress_queries=0
    if [[ -s "$progress_file" ]]; then
        progress_queries=$(sed -n 's/^queries=\([0-9][0-9]*\)$/\1/p' "$progress_file" | tail -1)
    fi
    [[ "$progress_queries" =~ ^[0-9]+$ ]] || progress_queries=0
    stage_count=$((stage_end - next_rank))
    (( progress_queries > stage_count )) && progress_queries=$stage_count
    overall_completed=$((next_rank + progress_queries))
    stage_percent=$(awk -v done="$progress_queries" -v total="$stage_count" \
        'BEGIN { printf total == 0 ? "100.00" : "%.2f", 100 * done / total }')
    overall_percent=$(awk -v done="$overall_completed" -v total="$end_rank" \
        'BEGIN { printf "%.3f", 100 * done / total }')
    rate=0
    stage_eta=0
    full_eta=0
    if (( progress_queries > 0 && elapsed > 0 )); then
        rate=$((progress_queries / elapsed))
        if (( rate > 0 )); then
            stage_eta=$(((stage_count - progress_queries) / rate))
            full_eta=$(((end_rank - overall_completed) / rate))
        fi
    fi
    {
        printf 'K=6 main-solver distance-14 census\n'
        printf '  state              %s\n' "$state"
        printf '  overall progress   %s / %s  (%s%%)\n' \
            "$overall_completed" "$end_rank" "$overall_percent"
        printf '  durable checkpoint %s\n' "$next_rank"
        printf '  active stage       %s..%s\n' "$next_rank" "$stage_end"
        printf '  stage progress     %s / %s  (%s%%)\n' \
            "$progress_queries" "$stage_count" "$stage_percent"
        printf '  stage elapsed      %s\n' "$(format_duration "$elapsed")"
        if (( rate > 0 )); then
            printf '  current rate       %s states/s\n' "$rate"
            printf '  stage ETA          %s\n' "$(format_duration "$stage_eta")"
            printf '  full ETA           %s  (at current-stage rate)\n' \
                "$(format_duration "$full_eta")"
        else
            printf '  current rate       pending first %s states\n' "$progress_interval"
            printf '  stage ETA          pending\n'
            printf '  full ETA           pending\n'
        fi
        printf '  survey RSS         %.2f GiB  (%s KiB)\n' \
            "$(awk -v kib="$rss_kib" 'BEGIN { print kib / 1048576 }')" "$rss_kib"
        printf '  survey VM limit    %.2f GiB\n' \
            "$(awk -v kib="$vm_kib" 'BEGIN { print kib / 1048576 }')"
        printf '  Sa193 RSS          %.2f GiB  (%s KiB)\n' \
            "$(awk -v kib="$sa_rss" 'BEGIN { print kib / 1048576 }')" "$sa_rss"
        printf '  pinned CPU         %s\n' "$cpu"
        printf '  query budget       unlimited\n'
        printf '  updated             %s\n' "$now"
    } > "$work_dir/STATUS"
    aws s3 cp "$work_dir/STATUS" "s3://$bucket/$prefix/STATUS" --only-show-errors
}

while (( next_rank < end_rank )); do
    count=$stage_size
    (( count > end_rank - next_rank )) && count=$(( end_rank - next_rank ))
    stage_end=$(( next_rank + count ))
    stage_name=$(printf 'stage_%010d_%010d' "$next_rank" "$stage_end")
    stage_out="$work_dir/stages/$stage_name.log"
    stage_err="$work_dir/stages/$stage_name.err.tail"
    stage_started=$(date +%s)
    progress_file="$work_dir/ACTIVE_PROGRESS"
    rm -f "$progress_file" "$progress_file.tmp"

    (
        set -o pipefail
        {
            printf 'report exceptions\n'
            RADIO_TRANSFER_PROGRESS_FILE="$progress_file" taskset -c "$cpu" ./shell-ranker \
                --transfer-shell-oracle-input 6 14 "$next_rank" "$count" 0
        } | (
            ulimit -v "$vm_kib"
            exec taskset -c "$cpu" ./k6-oracle
        ) 2> >(tail -n 200 > "$stage_err") \
          | grep --line-buffered -E '^#|^OK report=|^VERDICT (UNSOLVABLE|MAYBE)|^OK queries=|^OK bye'
    ) > "$stage_out" &
    stage_pid=$!

    last_status=0
    while kill -0 "$stage_pid" 2>/dev/null; do
        now_epoch=$(date +%s)
        if (( now_epoch - last_status >= status_interval )); then
            write_status running "$stage_end" "$stage_started"
            last_status=$now_epoch
        fi
        sleep 10
    done
    if ! wait "$stage_pid"; then
        write_status FAILED "$stage_end" "$stage_started"
        aws s3 cp "$stage_out" "s3://$bucket/$prefix/failed/$stage_name.log" --only-show-errors || true
        aws s3 cp "$stage_err" "s3://$bucket/$prefix/failed/$stage_name.err.tail" --only-show-errors || true
        exit 1
    fi

    summary=$(grep '^OK queries=' "$stage_out" | tail -1)
    actual=$(sed -n 's/^OK queries=\([0-9][0-9]*\).*/\1/p' <<<"$summary")
    maybe=$(sed -n 's/.* maybe=\([0-9][0-9]*\).*/\1/p' <<<"$summary")
    [[ "$actual" == "$count" && "$maybe" == 0 ]] || {
        write_status INVALID "$stage_end" "$stage_started"
        aws s3 cp "$stage_out" "s3://$bucket/$prefix/failed/$stage_name.log" --only-show-errors || true
        exit 1
    }
    python3 tools/check_provenance.py "$stage_out"
    aws s3 cp "$stage_out" "s3://$bucket/$prefix/stages/$stage_name.log" --only-show-errors
    aws s3 cp "$stage_err" "s3://$bucket/$prefix/stages/$stage_name.err.tail" --only-show-errors
    next_rank=$stage_end
    printf '%s\n' "$next_rank" > "$next_file"
    aws s3 cp "$next_file" "s3://$bucket/$prefix/NEXT_RANK" --only-show-errors
    write_status running "$next_rank" "$stage_started"
done

write_status COMPLETE "$end_rank" "$(date +%s)"
