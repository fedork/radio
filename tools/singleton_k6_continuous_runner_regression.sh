#!/usr/bin/env bash
# Exercise rolling S3 commits across two evidence stages while one fake solver PID stays alive.
set -euo pipefail

repo_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
tmp_dir=$(mktemp -d "${TMPDIR:-/tmp}/radio-k6-continuous.XXXXXX")
if [[ -z "${RADIO_KEEP_TEST_TMP:-}" ]]; then
    trap 'rm -rf -- "$tmp_dir"' EXIT
else
    printf 'test tmp: %s\n' "$tmp_dir"
fi
mkdir -p "$tmp_dir/bin" "$tmp_dir/s3/prefix" "$tmp_dir/work/tools"
printf '0\n' > "$tmp_dir/s3/prefix/NEXT_RANK"

cat > "$tmp_dir/bin/aws" <<'SH'
#!/usr/bin/env bash
set -euo pipefail
[[ "$1" == s3 && "$2" == cp ]]
src=$3
dst=$4
root=$FAKE_S3_ROOT
if [[ "$src" == s3://bucket/prefix/* ]]; then
    path=$root/prefix/${src#s3://bucket/prefix/}
    [[ -f "$path" ]] || exit 1
    cp "$path" "$dst"
elif [[ "$dst" == s3://bucket/prefix/* ]]; then
    path=$root/prefix/${dst#s3://bucket/prefix/}
    mkdir -p "$(dirname "$path")"
    cp "$src" "$path"
else
    exit 2
fi
SH
cat > "$tmp_dir/bin/taskset" <<'SH'
#!/usr/bin/env bash
set -euo pipefail
[[ "$1" == -c ]]
shift 2
exec "$@"
SH
cat > "$tmp_dir/work/check-ok" <<'SH'
#!/usr/bin/env bash
exit 0
SH
cat > "$tmp_dir/work/k6-survey" <<'SH'
#!/usr/bin/env bash
set -euo pipefail
start=$3
limit=$4
progress=$5
interval=$6
printf '%s %s %s\n' "$start" "$limit" "$$" >> invocation-records
printf '# radio-provenance-v1 begin\n# radio-provenance-v1 end\n'
printf 'INTEGRATED_BEGIN skip=%s limit=%s\n' "$start" "$limit"
queries=0
write_progress() {
    {
        printf 'start_rank=%s\n' "$start"
        printf 'absolute_rank=%s\n' $((start + queries))
        printf 'queries=%s\nsolvable=%s\nunsolvable=0\nmaybe=0\n' "$queries" "$queries"
        printf 'elapsed_seconds=%s.0\n' "$queries"
    } > "$progress.tmp"
    mv "$progress.tmp" "$progress"
}
while (( queries < limit )); do
    sleep 1
    count=$interval
    (( count > limit - queries )) && count=$((limit - queries))
    checkpoint_start=$((start + queries))
    queries=$((queries + count))
    checkpoint_end=$((start + queries))
    marker_maybe=0
    [[ -n "${FAKE_CORRUPT_CHECKPOINT:-}" ]] && marker_maybe=1
    printf 'INTEGRATED_CHECKPOINT start=%s end=%s queries=%s solvable=%s unsolvable=0 maybe=%s cumulative_queries=%s cumulative_solvable=%s cumulative_unsolvable=0 cumulative_maybe=%s wall_seconds=%s.0 pid=%s\n' \
        "$checkpoint_start" "$checkpoint_end" "$count" "$count" "$marker_maybe" \
        "$queries" "$queries" "$marker_maybe" "$queries" "$$"
    write_progress
    if [[ "${FAKE_FAIL_AT_QUERY:-}" == "$queries" ]]; then
        exit 42
    fi
done
printf 'INTEGRATED_SUMMARY queries=%s solvable=%s unsolvable=0 maybe=0\n' "$queries" "$queries"
SH
chmod +x "$tmp_dir/bin/aws" "$tmp_dir/bin/taskset" "$tmp_dir/work/check-ok" \
    "$tmp_dir/work/k6-survey"
cp "$repo_dir/tools/singleton_k6_survey_remote.sh" \
    "$tmp_dir/work/tools/singleton_k6_survey_remote.sh"

PATH="$tmp_dir/bin:$PATH" FAKE_S3_ROOT="$tmp_dir/s3" \
RADIO_K6_SURVEY_DIR="$tmp_dir/work" RADIO_K6_SURVEY_CPU=0 \
RADIO_K6_SURVEY_VM_KIB=1048576 RADIO_K6_STATUS_INTERVAL=1 RADIO_K6_POLL_INTERVAL=1 \
RADIO_K6_DISABLE_VM_LIMIT=1 \
RADIO_K6_PROVENANCE_CHECK="$tmp_dir/work/check-ok" \
    "$tmp_dir/work/tools/singleton_k6_survey_remote.sh" bucket prefix 0 6 3

[[ "$(tr -d '[:space:]' < "$tmp_dir/s3/prefix/NEXT_RANK")" == 6 ]]
[[ "$(wc -l < "$tmp_dir/work/invocation-records" | tr -d ' ')" == 1 ]]
read -r invoked_start invoked_limit pid < "$tmp_dir/work/invocation-records"
[[ "$invoked_start $invoked_limit" == "0 6" ]]
for stage in stage_0000000000_0000000003 stage_0000000003_0000000006; do
    log="$tmp_dir/s3/prefix/stages/$stage.log"
    [[ -s "$log" ]]
    rg -q "^INTEGRATED_CHECKPOINT .* pid=$pid$" "$log"
done
rg -q '^  state              COMPLETE$' "$tmp_dir/s3/prefix/STATUS"
rg -q '^  cache epoch start  0  \(rolling checkpoints; same PID\)$' \
    "$tmp_dir/s3/prefix/STATUS"

printf 'continuous runner: one PID %s committed ranks 0..3..6\n' "$pid"

# A solver crash after its first marker must retain rank 3; the next invocation resumes there.
rm -rf -- "$tmp_dir/work/stages" "$tmp_dir/s3/prefix/stages" "$tmp_dir/s3/prefix/failed"
mkdir -p "$tmp_dir/work/stages"
rm -f -- "$tmp_dir/work/invocation-records" "$tmp_dir/work/ACTIVE_PROGRESS" \
    "$tmp_dir/work/NEXT_RANK" "$tmp_dir/work/NEXT_RANK.remote"
printf '0\n' > "$tmp_dir/s3/prefix/NEXT_RANK"
set +e
PATH="$tmp_dir/bin:$PATH" FAKE_S3_ROOT="$tmp_dir/s3" FAKE_FAIL_AT_QUERY=3 \
RADIO_K6_SURVEY_DIR="$tmp_dir/work" RADIO_K6_SURVEY_CPU=0 \
RADIO_K6_SURVEY_VM_KIB=1048576 RADIO_K6_STATUS_INTERVAL=1 RADIO_K6_POLL_INTERVAL=1 \
RADIO_K6_DISABLE_VM_LIMIT=1 \
RADIO_K6_PROVENANCE_CHECK="$tmp_dir/work/check-ok" \
    "$tmp_dir/work/tools/singleton_k6_survey_remote.sh" bucket prefix 0 6 3
crash_rc=$?
set -e
[[ "$crash_rc" == 75 ]]
[[ "$(tr -d '[:space:]' < "$tmp_dir/s3/prefix/NEXT_RANK")" == 3 ]]

PATH="$tmp_dir/bin:$PATH" FAKE_S3_ROOT="$tmp_dir/s3" \
RADIO_K6_SURVEY_DIR="$tmp_dir/work" RADIO_K6_SURVEY_CPU=0 \
RADIO_K6_SURVEY_VM_KIB=1048576 RADIO_K6_STATUS_INTERVAL=1 RADIO_K6_POLL_INTERVAL=1 \
RADIO_K6_DISABLE_VM_LIMIT=1 \
RADIO_K6_PROVENANCE_CHECK="$tmp_dir/work/check-ok" \
    "$tmp_dir/work/tools/singleton_k6_survey_remote.sh" bucket prefix 0 6 3
[[ "$(tr -d '[:space:]' < "$tmp_dir/s3/prefix/NEXT_RANK")" == 6 ]]
[[ "$(awk '{print $1}' "$tmp_dir/work/invocation-records" | paste -sd, -)" == 0,3 ]]
printf 'continuous runner: crash exit 75 retained rank 3; replacement resumed 3..6\n'

# A semantically invalid marker is permanent and must not advance the durable checkpoint.
rm -rf -- "$tmp_dir/work/stages" "$tmp_dir/s3/prefix/stages" "$tmp_dir/s3/prefix/failed"
mkdir -p "$tmp_dir/work/stages"
rm -f -- "$tmp_dir/work/invocation-records" "$tmp_dir/work/ACTIVE_PROGRESS" \
    "$tmp_dir/work/NEXT_RANK" "$tmp_dir/work/NEXT_RANK.remote"
printf '0\n' > "$tmp_dir/s3/prefix/NEXT_RANK"
set +e
PATH="$tmp_dir/bin:$PATH" FAKE_S3_ROOT="$tmp_dir/s3" FAKE_CORRUPT_CHECKPOINT=1 \
RADIO_K6_SURVEY_DIR="$tmp_dir/work" RADIO_K6_SURVEY_CPU=0 \
RADIO_K6_SURVEY_VM_KIB=1048576 RADIO_K6_STATUS_INTERVAL=1 RADIO_K6_POLL_INTERVAL=1 \
RADIO_K6_DISABLE_VM_LIMIT=1 \
RADIO_K6_PROVENANCE_CHECK="$tmp_dir/work/check-ok" \
    "$tmp_dir/work/tools/singleton_k6_survey_remote.sh" bucket prefix 0 6 3
invalid_rc=$?
set -e
[[ "$invalid_rc" == 65 ]]
[[ "$(tr -d '[:space:]' < "$tmp_dir/s3/prefix/NEXT_RANK")" == 0 ]]
printf 'continuous runner: invalid marker exited 65 and retained rank 0\n'
