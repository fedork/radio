#!/usr/bin/env bash
# Run the complete K=6 transfer shell as one cache-warm process with rolling durable rank stages.
# There is no time or search-budget cutoff: each oracle query uses budget 0. A virtual-memory
# ceiling turns resource exhaustion into an abort, never a verdict. Completed stage markers and
# their log snapshots advance NEXT_RANK without restarting the solver process.
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
disable_vm_limit=${RADIO_K6_DISABLE_VM_LIMIT:-0}
status_interval=${RADIO_K6_STATUS_INTERVAL:-60}
poll_interval=${RADIO_K6_POLL_INTERVAL:-10}
provenance_check=${RADIO_K6_PROVENANCE_CHECK:-tools/check_provenance.py}
progress_interval=100000
[[ "$status_interval" =~ ^[1-9][0-9]*$ ]] || {
    echo "RADIO_K6_STATUS_INTERVAL must be a positive integer" >&2
    exit 2
}
[[ "$poll_interval" =~ ^[1-9][0-9]*$ ]] || {
    echo "RADIO_K6_POLL_INTERVAL must be a positive integer" >&2
    exit 2
}
[[ "$disable_vm_limit" == 0 || "$disable_vm_limit" == 1 ]] || {
    echo "RADIO_K6_DISABLE_VM_LIMIT must be 0 or 1" >&2
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
    local state=$1 now rss_kib elapsed sa_rss progress_file absolute_rank
    local cumulative_unsolvable cumulative_maybe stage_progress stage_count
    local stage_unsolvable stage_maybe stage_percent overall_percent rate stage_eta full_eta
    now=$(date -u +%FT%TZ)
    elapsed=$(( $(date +%s) - stage_started ))
    rss_kib=$({ ps -C k6-survey -o rss= 2>/dev/null || true; } \
        | awk '{sum += $1} END {print sum + 0}')
    sa_rss=$({ ps -C radio_sa193 -o rss= 2>/dev/null || true; } \
        | awk '{sum += $1} END {print sum + 0}')
    progress_file="$work_dir/ACTIVE_PROGRESS"
    absolute_rank=$next_rank
    cumulative_unsolvable=$stage_base_unsolvable
    cumulative_maybe=$stage_base_maybe
    if [[ -s "$progress_file" ]]; then
        absolute_rank=$(sed -n 's/^absolute_rank=\([0-9][0-9]*\)$/\1/p' "$progress_file" | tail -1)
        cumulative_unsolvable=$(sed -n 's/^unsolvable=\([0-9][0-9]*\)$/\1/p' "$progress_file" | tail -1)
        cumulative_maybe=$(sed -n 's/^maybe=\([0-9][0-9]*\)$/\1/p' "$progress_file" | tail -1)
    fi
    [[ "$absolute_rank" =~ ^[0-9]+$ ]] || absolute_rank=$next_rank
    [[ "$cumulative_unsolvable" =~ ^[0-9]+$ ]] || cumulative_unsolvable=$stage_base_unsolvable
    [[ "$cumulative_maybe" =~ ^[0-9]+$ ]] || cumulative_maybe=$stage_base_maybe
    (( absolute_rank < next_rank )) && absolute_rank=$next_rank
    (( absolute_rank > end_rank )) && absolute_rank=$end_rank
    stage_count=$((stage_end - next_rank))
    stage_progress=$((absolute_rank - next_rank))
    (( stage_progress > stage_count )) && stage_progress=$stage_count
    stage_unsolvable=$((cumulative_unsolvable - stage_base_unsolvable))
    stage_maybe=$((cumulative_maybe - stage_base_maybe))
    if (( stage_count == 0 )); then
        stage_percent=100.00
    else
        stage_percent=$(awk -v done="$stage_progress" -v total="$stage_count" \
            'BEGIN { printf "%.2f", 100 * done / total }')
    fi
    overall_percent=$(awk -v done="$absolute_rank" -v total="$end_rank" \
        'BEGIN { printf "%.3f", 100 * done / total }')
    rate=0
    stage_eta=0
    full_eta=0
    if (( stage_progress > 0 && elapsed > 0 )); then
        rate=$((stage_progress / elapsed))
        if (( rate > 0 )); then
            stage_eta=$(((stage_count - stage_progress) / rate))
            full_eta=$(((end_rank - absolute_rank) / rate))
        fi
    fi
    {
        printf 'K=6 main-solver distance-14 census\n'
        printf '  state              %s\n' "$state"
        printf '  overall progress   %s / %s  (%s%%)\n' \
            "$absolute_rank" "$end_rank" "$overall_percent"
        printf '  durable checkpoint %s\n' "$next_rank"
        printf '  active stage       %s..%s\n' "$next_rank" "$stage_end"
        printf '  stage progress     %s / %s  (%s%%)\n' \
            "$stage_progress" "$stage_count" "$stage_percent"
        printf '  stage exceptions   %s unsolvable, %s MAYBE\n' \
            "$stage_unsolvable" "$stage_maybe"
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
        printf '  cache epoch start  %s  (rolling checkpoints; same PID)\n' "$process_start"
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
    aws s3 cp "$work_dir/STATUS" "s3://$bucket/$prefix/STATUS" --only-show-errors || {
        [[ "$state" == COMPLETE ]] && return 75
        echo "warning: could not upload STATUS heartbeat" >&2
        return 0
    }
}

marker_field() {
    local marker=$1 name=$2
    sed -n "s/.* $name=\\([^ ]*\\).*/\\1/p" <<<"$marker"
}

commit_available_checkpoints() {
    local marker expected_end marker_end marker_queries marker_solvable marker_unsolvable
    local marker_maybe cumulative_solvable cumulative_unsolvable cumulative_maybe
    local stage_name snapshot
    while :; do
        marker=$(grep -m1 "^INTEGRATED_CHECKPOINT start=$next_rank " "$continuous_out" || true)
        [[ -n "$marker" ]] || return 0
        expected_end=$((next_rank + stage_size))
        (( expected_end > end_rank )) && expected_end=$end_rank
        marker_end=$(marker_field "$marker" end)
        marker_queries=$(marker_field "$marker" queries)
        marker_solvable=$(marker_field "$marker" solvable)
        marker_unsolvable=$(marker_field "$marker" unsolvable)
        marker_maybe=$(marker_field "$marker" maybe)
        cumulative_solvable=$(marker_field "$marker" cumulative_solvable)
        cumulative_unsolvable=$(marker_field "$marker" cumulative_unsolvable)
        cumulative_maybe=$(marker_field "$marker" cumulative_maybe)
        [[ "$marker_end" == "$expected_end" \
            && "$marker_queries" == $((expected_end - next_rank)) \
            && "$marker_maybe" == 0 \
            && "$marker_solvable" =~ ^[0-9]+$ \
            && "$marker_unsolvable" =~ ^[0-9]+$ \
            && "$cumulative_solvable" =~ ^[0-9]+$ \
            && "$cumulative_unsolvable" =~ ^[0-9]+$ \
            && "$cumulative_maybe" =~ ^[0-9]+$ \
            && $((marker_solvable + marker_unsolvable)) == "$marker_queries" ]] || {
                echo "invalid rolling checkpoint marker: $marker" >&2
                return 65
            }

        stage_name=$(printf 'stage_%010d_%010d' "$next_rank" "$expected_end")
        snapshot="$work_dir/stages/$stage_name.log"
        awk -v marker="$marker" '
            { print }
            $0 == marker { found=1; exit }
            END { if (!found) exit 1 }
        ' "$continuous_out" > "$snapshot.tmp" || return 75
        "$provenance_check" "$snapshot.tmp" || return 65
        mv "$snapshot.tmp" "$snapshot" || return 75
        aws s3 cp "$snapshot" "s3://$bucket/$prefix/stages/$stage_name.log" \
            --only-show-errors || return 75

        # Artifact first, checkpoint second: a visible NEXT_RANK always has its validating log.
        next_rank=$expected_end
        printf '%s\n' "$next_rank" > "$next_file" || return 75
        aws s3 cp "$next_file" "s3://$bucket/$prefix/NEXT_RANK" \
            --only-show-errors || return 75
        stage_base_solvable=$cumulative_solvable
        stage_base_unsolvable=$cumulative_unsolvable
        stage_base_maybe=$cumulative_maybe
        stage_end=$((next_rank + stage_size))
        (( stage_end > end_rank )) && stage_end=$end_rank
        stage_started=$(date +%s)
        write_status running
    done
}

if (( next_rank == end_rank )); then
    process_start=$next_rank
    stage_end=$next_rank
    stage_started=$(date +%s)
    stage_base_solvable=0
    stage_base_unsolvable=0
    stage_base_maybe=0
    write_status COMPLETE
    exit 0
fi

process_start=$next_rank
process_limit=$((end_rank - process_start))
stage_end=$((next_rank + stage_size))
(( stage_end > end_rank )) && stage_end=$end_rank
stage_started=$(date +%s)
stage_base_solvable=0
stage_base_unsolvable=0
stage_base_maybe=0
progress_file="$work_dir/ACTIVE_PROGRESS"
continuous_out="$work_dir/stages/continuous_$(printf '%010d' "$process_start").log"
continuous_err="$work_dir/stages/continuous_$(printf '%010d' "$process_start").err.tail"
rm -f "$progress_file" "$progress_file.tmp" "$continuous_out" "$continuous_err"

(
    if (( disable_vm_limit == 0 )); then
        ulimit -v "$vm_kib"
    fi
    exec taskset -c "$cpu" ./k6-survey \
        6 14 "$process_start" "$process_limit" "$progress_file" "$stage_size"
) > "$continuous_out" 2> >(tail -n 200 > "$continuous_err") &
stage_pid=$!

last_status=0
while kill -0 "$stage_pid" 2>/dev/null; do
    commit_rc=0
    commit_available_checkpoints || commit_rc=$?
    if (( commit_rc != 0 )); then
        if (( commit_rc == 65 )); then
            write_status INVALID
        else
            write_status FAILED
        fi
        aws s3 cp "$continuous_out" "s3://$bucket/$prefix/failed/continuous.log" --only-show-errors || true
        kill "$stage_pid" 2>/dev/null || true
        wait "$stage_pid" || true
        exit "$commit_rc"
    fi
    now_epoch=$(date +%s)
    if (( now_epoch - last_status >= status_interval )); then
        write_status running
        last_status=$now_epoch
    fi
    sleep "$poll_interval"
done

set +e
wait "$stage_pid"
stage_rc=$?
set -e
commit_rc=0
commit_available_checkpoints || commit_rc=$?
if (( commit_rc != 0 )); then
    if (( commit_rc == 65 )); then
        write_status INVALID
    else
        write_status FAILED
    fi
    aws s3 cp "$continuous_out" "s3://$bucket/$prefix/failed/continuous.log" --only-show-errors || true
    exit "$commit_rc"
fi
if (( stage_rc != 0 )); then
    write_status FAILED
    aws s3 cp "$continuous_out" "s3://$bucket/$prefix/failed/continuous.log" --only-show-errors || true
    aws s3 cp "$continuous_err" "s3://$bucket/$prefix/failed/continuous.err.tail" --only-show-errors || true
    exit 75
fi
if (( next_rank != end_rank )); then
    write_status INVALID
    aws s3 cp "$continuous_out" "s3://$bucket/$prefix/failed/continuous.log" --only-show-errors || true
    exit 65
fi

summary=$(grep '^INTEGRATED_SUMMARY ' "$continuous_out" | tail -1)
actual=$(sed -n 's/^INTEGRATED_SUMMARY queries=\([0-9][0-9]*\).*/\1/p' <<<"$summary")
maybe=$(sed -n 's/.* maybe=\([0-9][0-9]*\).*/\1/p' <<<"$summary")
[[ "$actual" == "$process_limit" && "$maybe" == 0 ]] || {
    write_status INVALID
    exit 65
}
"$provenance_check" "$continuous_out" || {
    write_status INVALID
    exit 65
}
aws s3 cp "$continuous_out" "s3://$bucket/$prefix/completed/continuous.log" \
    --only-show-errors || exit 75
write_status COMPLETE
