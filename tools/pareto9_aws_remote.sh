#!/usr/bin/env bash
# Run the unbounded K=9 Pareto walk on EC2 with durable checkpoints and SNS progress mail.
set -euo pipefail

bucket=radio-sa193-393287594714
topic=arn:aws:sns:us-west-2:393287594714:radio-sa193-progress
run_id=
prefix=
source_commit=
source_sha256=
cache_key=run10/sa193.checkpoint
start_n=55
start_m=55
rss_gb=24
interval=600
heartbeat=21600

while (( $# )); do
    case "$1" in
        --run-id) run_id=$2; shift 2 ;;
        --prefix) prefix=$2; shift 2 ;;
        --source-commit) source_commit=$2; shift 2 ;;
        --source-sha256) source_sha256=$2; shift 2 ;;
        --cache-key) cache_key=$2; shift 2 ;;
        --start-n) start_n=$2; shift 2 ;;
        --start-m) start_m=$2; shift 2 ;;
        --rss-gb) rss_gb=$2; shift 2 ;;
        --interval) interval=$2; shift 2 ;;
        --heartbeat) heartbeat=$2; shift 2 ;;
        *) echo "unknown argument: $1" >&2; exit 64 ;;
    esac
done

[[ "$run_id" =~ ^[0-9]{8}T[0-9]{6}Z$ ]] || { echo 'invalid --run-id' >&2; exit 64; }
[[ "$prefix" == "pareto9-frontier/$run_id" ]] || { echo 'invalid --prefix' >&2; exit 64; }
[[ "$source_commit" =~ ^[0-9a-f]{40}$ ]] || { echo 'invalid --source-commit' >&2; exit 64; }
[[ "$source_sha256" =~ ^[0-9a-f]{64}$ ]] || { echo 'invalid --source-sha256' >&2; exit 64; }
[[ "$cache_key" =~ ^[A-Za-z0-9][A-Za-z0-9._/-]*$ ]] || { echo 'invalid --cache-key' >&2; exit 64; }
[[ "$start_n" =~ ^[1-9][0-9]*$ && "$start_m" == "$start_n" ]] || {
    echo 'the bootstrap seed must be a positive diagonal state' >&2; exit 64
}
[[ "$rss_gb" =~ ^[1-9][0-9]*$ ]] || { echo 'invalid --rss-gb' >&2; exit 64; }
[[ "$interval" =~ ^[1-9][0-9]*$ && "$interval" -ge 60 ]] || {
    echo 'invalid --interval' >&2; exit 64
}
[[ "$heartbeat" =~ ^[1-9][0-9]*$ && "$heartbeat" -ge "$interval" ]] || {
    echo 'invalid --heartbeat' >&2; exit 64
}

source_dir=$(cd "$(dirname "$0")/.." && pwd)
work_dir="/root/pareto9-$run_id"
mkdir -p "$work_dir"
cd "$work_dir"
exec > >(tee -a supervisor.log) 2>&1

notify() {
    local subject=$1 body=$2
    aws sns publish --region us-west-2 --topic-arn "$topic" \
        --subject "$(printf '[pareto9] %s' "$subject" | cut -c1-99)" \
        --message "$body" >/dev/null 2>&1 || true
}

put() {
    local attempt
    for attempt in 1 2 3; do
        if aws s3 cp --region us-west-2 "$1" "s3://$bucket/$prefix/$2" \
                --no-progress >/dev/null; then
            return 0
        fi
        echo "upload attempt $attempt failed: $1 -> s3://$bucket/$prefix/$2" >&2
        sleep 5
    done
    return 1
}

write_stage_status() {
    local stage=$1 detail=$2
    {
        printf 'K=9 Pareto frontier walk\n'
        printf '  run id              %s\n' "$run_id"
        printf '  stage               %s\n' "$stage"
        printf '  detail              %s\n' "$detail"
        printf '  start               Sb(%s:%s), diagonal bootstrap\n' "$start_n" "$start_m"
        printf '  wall-time bound     none\n'
        printf '  resident-memory cap %s GiB\n' "$rss_gb"
        printf '  source cache        s3://%s/%s\n' "$bucket" "$cache_key"
        printf '  updated UTC         %s\n' "$(date -u +%FT%TZ)"
    } > STATUS.tmp
    mv STATUS.tmp STATUS
    put STATUS STATUS || true
}

write_stage_status bootstrap 'downloading the latest durable Sa(193) cache checkpoint'

# S3 replacement is atomic but this source key is refreshed by the live Sa run. Verify that the
# object identity did not change across our download; retry rather than silently recording the
# wrong ETag in the run metadata.
cache_head=
for attempt in 1 2 3; do
    before=$(aws s3api head-object --region us-west-2 --bucket "$bucket" --key "$cache_key" \
        --query '[ETag,ContentLength,LastModified]' --output text)
    aws s3 cp --region us-west-2 "s3://$bucket/$cache_key" input-sa193.cache --no-progress
    after=$(aws s3api head-object --region us-west-2 --bucket "$bucket" --key "$cache_key" \
        --query '[ETag,ContentLength,LastModified]' --output text)
    if [[ "$before" == "$after" ]]; then
        cache_head=$after
        break
    fi
    echo "Sa cache changed during download attempt $attempt; retrying"
done
[[ -n "$cache_head" ]] || { notify 'bootstrap failed' 'Sa cache kept changing during download'; exit 72; }
cache_sha256=$(sha256sum input-sa193.cache | awk '{print $1}')
cache_lines=$(wc -l < input-sa193.cache | tr -d ' ')
grep -qx '# radio-cache-semantics=singleton-majorization-necessity-only-v1' input-sa193.cache || {
    notify 'bootstrap failed' 'The selected Sa cache lacks the current necessity-only marker.'
    exit 72
}
python3 "$source_dir/tools/check_provenance.py" input-sa193.cache

# Preserve the exact input because run10/sa193.checkpoint continues to change in place.
zstd -T0 -10 -f input-sa193.cache -o input-sa193.cache.zst
put input-sa193.cache.zst input/input-sa193.cache.zst

write_stage_status bootstrap 'building and running local/remote correctness gates'
cd "$source_dir"
tools/pareto_walk_regression.sh
RADIO_SOURCE_COMMIT="$source_commit" python3 tools/build_radio.py \
    -O3 -DMAX_K=9 -DMAX_N=514 -DMAX_PART_N=514 radio_pareto.c \
    -o "$work_dir/radio_pareto_k9"
python3 tools/check_provenance.py "$work_dir/radio_pareto_k9.provenance"
cd "$work_dir"

start_utc=$(date -u +%FT%TZ)
read -r cache_etag cache_bytes cache_modified <<<"$cache_head"
{
    printf 'run_id=%s\n' "$run_id"
    printf 'started_utc=%s\n' "$start_utc"
    printf 'source_commit=%s\n' "$source_commit"
    printf 'source_bundle_sha256=%s\n' "$source_sha256"
    printf 'source_cache=s3://%s/%s\n' "$bucket" "$cache_key"
    printf 'source_cache_etag=%s\n' "$cache_etag"
    printf 'source_cache_bytes=%s\n' "$cache_bytes"
    printf 'source_cache_last_modified=%s\n' "$cache_modified"
    printf 'source_cache_sha256=%s\n' "$cache_sha256"
    printf 'source_cache_lines=%s\n' "$cache_lines"
    printf 'query=radio_pareto --bootstrap-diagonal 9 %s %s input-sa193.cache\n' \
        "$start_n" "$start_m"
    printf 'wall_time_limit=none\n'
    printf 'rss_limit_gib=%s\n' "$rss_gb"
    printf 'notification_topic=%s\n' "$topic"
    cat radio_pareto_k9.provenance
} > run.meta
put run.meta run.meta
put radio_pareto_k9 radio_pareto_k9
put radio_pareto_k9.provenance radio_pareto_k9.provenance

segment="seg-${start_utc//[:T-]/}"
out="$work_dir/$segment.out"
err="$work_dir/$segment.err"

write_checkpoint() {
    local final=${1:-0} upload_failed=0
    cp input-sa193.cache checkpoint.cache.tmp
    "$source_dir/parse_out.sh" < "$out" >> checkpoint.cache.tmp
    mv checkpoint.cache.tmp checkpoint.cache
    sha256sum checkpoint.cache > checkpoint.cache.sha256
    zstd -T0 -3 -f -c "$out" > "$segment.out.zst.tmp"
    mv "$segment.out.zst.tmp" "$segment.out.zst"
    put checkpoint.cache checkpoint.cache || upload_failed=1
    put checkpoint.cache.sha256 checkpoint.cache.sha256 || upload_failed=1
    put "$segment.out.zst" "segments/$segment.out.zst" || upload_failed=1
    put "$err" "segments/$segment.err" || upload_failed=1
    if (( final != 0 )) && ! python3 "$source_dir/tools/check_provenance.py" "$out"; then
        echo 'final raw-log provenance check failed' >&2
        upload_failed=1
    fi
    return "$upload_failed"
}

render_status() {
    local state=$1 now elapsed rss scan current
    now=$(date +%s)
    elapsed=$((now - solver_started_epoch))
    rss=$(ps -o rss= -p "$solver_pid" 2>/dev/null | awk '{printf "%.2f", $1/1048576}')
    [[ -n "$rss" ]] || rss='-'
    scan=$(awk '
        /^PARETO STEP / { steps++; last_step=$0 }
        /^PARETO CELL / { cells++; last_cell=$0 }
        /^PARETO BOOTSTRAP_END / { bootstrap=$0 }
        /^PARETO DONE / { done=$0 }
        /^PARETO ABORT / { abort=$0 }
        /still solving in/ { progress=$0 }
        END {
            printf "STEPS %d\nCELLS %d\n", steps+0, cells+0
            if (last_step) printf "LAST_STEP %s\n", last_step
            if (last_cell) printf "LAST_CELL %s\n", last_cell
            if (bootstrap) printf "BOOTSTRAP %s\n", bootstrap
            if (progress) printf "PROGRESS %s\n", progress
            if (done) printf "DONE %s\n", done
            if (abort) printf "ABORT %s\n", abort
        }
    ' "$out" 2>/dev/null || true)
    current=$(kill -0 "$solver_pid" 2>/dev/null && echo alive || echo gone)
    {
        printf 'K=9 Pareto frontier walk\n'
        printf '  run id              %s\n' "$run_id"
        printf '  stage               solving\n'
        printf '  solver process      %s\n' "$current"
        printf '  running for         %dd %02dh %02dm\n' \
            $((elapsed/86400)) $((elapsed%86400/3600)) $((elapsed%3600/60))
        printf '  resident memory     %s GiB (cap %s GiB)\n' "$rss" "$rss_gb"
        printf '  wall-time bound     none\n'
        printf '  decisions           %s\n' "$(awk '$1=="STEPS"{print $2}' <<<"$scan")"
        printf '  frontier cells      %s\n' "$(awk '$1=="CELLS"{print $2}' <<<"$scan")"
        awk '/^BOOTSTRAP /{sub(/^BOOTSTRAP /,"  bootstrap           "); print}' <<<"$scan"
        awk '/^LAST_CELL /{sub(/^LAST_CELL /,"  latest cell         "); print}' <<<"$scan"
        awk '/^LAST_STEP /{sub(/^LAST_STEP /,"  latest decision     "); print}' <<<"$scan"
        awk '/^PROGRESS /{sub(/^PROGRESS /,"  solver progress     "); print}' <<<"$scan"
        awk '/^DONE /{sub(/^DONE /,"  completion          "); print}' <<<"$scan"
        awk '/^ABORT /{sub(/^ABORT /,"  abort               "); print}' <<<"$scan"
        printf '  cache input         %s facts, sha256=%s\n' "$cache_lines" "$cache_sha256"
        printf '  raw log             %s bytes\n' "$(wc -c < "$out" | tr -d ' ')"
        printf '  updated UTC         %s\n' "$(date -u +%FT%TZ)"
        printf '  durable prefix      s3://%s/%s/\n' "$bucket" "$prefix"
    } > STATUS.tmp
    mv STATUS.tmp STATUS
    put STATUS STATUS || true
    printf '%s' "$scan"
}

notify 'run started' "Run $run_id passed its gates and loaded $cache_lines facts from s3://$bucket/$cache_key. Cache sha256: $cache_sha256. Starting the unbounded K=9 diagonal bootstrap at Sb($start_n:$start_m); RSS cap ${rss_gb} GiB."

solver_started_epoch=$(date +%s)
RADIO_RUN_CONTEXT="pareto9-frontier; cache_sha256=$cache_sha256; no wall-time bound" \
    "$source_dir/tools/capped_run.sh" --seconds 0 --rss-gb "$rss_gb" --poll 10 \
        --label pareto9-frontier -- "$work_dir/radio_pareto_k9" \
        --bootstrap-diagonal 9 "$start_n" "$start_m" input-sa193.cache \
        > "$out" 2> "$err" &
wrapper_pid=$!
solver_pid=
for _ in $(seq 1 180); do
    solver_pid=$(pgrep -P "$wrapper_pid" -x radio_pareto_k9 | head -1 || true)
    [[ -n "$solver_pid" ]] && break
    kill -0 "$wrapper_pid" 2>/dev/null || break
    sleep 1
done
if [[ -z "$solver_pid" ]]; then
    notify 'launch failed' "Run $run_id did not produce a live radio_pareto_k9 process."
    write_stage_status failed 'solver did not survive launch'
    exit 71
fi
printf 'wrapper_pid=%s\nsolver_pid=%s\n' "$wrapper_pid" "$solver_pid" >> run.meta
put run.meta run.meta

last_checkpoint=0
last_heartbeat=$solver_started_epoch
last_cells=0
bootstrap_mailed=0
while kill -0 "$wrapper_pid" 2>/dev/null; do
    sleep "$interval"
    scan=$(render_status alive)
    now=$(date +%s)
    cells=$(awk '$1=="CELLS"{print $2}' <<<"$scan")
    if (( cells > last_cells )); then
        latest=$(awk '/^LAST_CELL /{sub(/^LAST_CELL /,""); print}' <<<"$scan")
        notify 'frontier progress' "Run $run_id resolved $((cells-last_cells)) new frontier cell(s); $cells total. Latest: $latest"
        last_cells=$cells
    fi
    if (( bootstrap_mailed == 0 )) && grep -q '^BOOTSTRAP ' <<<"$scan"; then
        notify 'diagonal bootstrap complete' "$(awk '/^BOOTSTRAP /{sub(/^BOOTSTRAP /,""); print}' <<<"$scan")"
        bootstrap_mailed=1
    fi
    if (( now - last_checkpoint >= 3600 )); then
        if ! write_checkpoint 0; then
            notify 'checkpoint upload warning' "Run $run_id is still solving, but at least one hourly S3 upload failed after three attempts. The complete files remain on persistent EBS and the next checkpoint will retry."
        fi
        last_checkpoint=$now
    fi
    if (( now - last_heartbeat >= heartbeat )); then
        body=$(sed -n '1,16p' STATUS)
        notify '6-hour heartbeat' "$body"
        last_heartbeat=$now
    fi
done

set +e
wait "$wrapper_pid"
solver_rc=$?
set -e
scan=$(render_status gone)
final_upload_ok=1
if ! write_checkpoint 1; then final_upload_ok=0; fi

final_state=failed
if (( solver_rc == 0 )) && grep -q '^DONE ' <<<"$scan"; then
    final_state=complete
fi
{
    printf 'final_state=%s\n' "$final_state"
    printf 'solver_exit=%s\n' "$solver_rc"
    printf 'finished_utc=%s\n' "$(date -u +%FT%TZ)"
    sha256sum run.meta STATUS checkpoint.cache checkpoint.cache.sha256 \
        "$segment.out.zst" "$err" radio_pareto_k9 radio_pareto_k9.provenance \
        input-sa193.cache.zst
} > final.sha256
put supervisor.log supervisor.log || final_upload_ok=0
put final.sha256 final.sha256 || final_upload_ok=0

if (( final_upload_ok == 0 )); then
    notify 'final upload failed' "Run $run_id stopped with exit $solver_rc, but its final S3 upload did not complete after retries. The instance and persistent EBS volume were deliberately left running for recovery."
    exit 75
fi

if [[ "$final_state" == complete ]]; then
    notify 'run complete' "Run $run_id completed the K=9 frontier walk. $(awk '/^DONE /{sub(/^DONE /,""); print}' <<<"$scan")"
else
    notify 'run stopped' "Run $run_id stopped with exit $solver_rc. The latest checkpoint, raw log, status, and persistent EBS volume were retained under s3://$bucket/$prefix/."
fi
shutdown -h +2 >/dev/null 2>&1 || true
exit "$solver_rc"
