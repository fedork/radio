#!/usr/bin/env bash
# Follow a live Linux solver log and record resource usage whenever one exact state reports.
# This is intentionally a separate observer: it neither signals nor restarts the solver.
#
# Example:
#   tools/sa193_track_state.sh --log /root/run4/out_sa193.txt --pid 123 \
#       --state 'Sb(48:48,64:33)' --out /root/run4/match-sb48.tsv \
#       --s3 s3://radio-sa193-393287594714/run4/matches/sb48_48_64_33.tsv

set -uo pipefail

LOG=
PID=
STATE=
OUT=
S3_URI=

while (( $# )); do
    case "$1" in
        --log) LOG="$2"; shift 2 ;;
        --pid) PID="$2"; shift 2 ;;
        --state) STATE="$2"; shift 2 ;;
        --out) OUT="$2"; shift 2 ;;
        --s3) S3_URI="$2"; shift 2 ;;
        *) echo "unknown argument: $1" >&2; exit 64 ;;
    esac
done

[[ -n "$LOG" && -n "$PID" && -n "$STATE" && -n "$OUT" ]] || {
    echo "need --log, --pid, --state and --out" >&2
    exit 64
}
[[ "$PID" =~ ^[0-9]+$ ]] || { echo "invalid PID: $PID" >&2; exit 64; }
[[ -f "$LOG" ]] || { echo "missing log: $LOG" >&2; exit 66; }
[[ -r "/proc/$PID/status" ]] || { echo "solver PID $PID is not live" >&2; exit 69; }
[[ ! -e "$OUT" ]] || { echo "refusing to overwrite $OUT" >&2; exit 73; }
tail --help 2>&1 | grep -q -- '--pid' || {
    echo "GNU tail with --pid is required" >&2
    exit 69
}

upload() {
    [[ -n "$S3_URI" ]] && aws s3 cp "$OUT" "$S3_URI" >/dev/null 2>&1 || true
}

{
    printf '# sa193 exact-state resource profile v1\n'
    printf '# state=%s\n' "$STATE"
    printf '# log=%s pid=%s started=%s\n' "$LOG" "$PID" "$(date -u +%FT%TZ)"
    printf 'iso\tsolver_secs\trss_kb\tvmdata_kb\tvmpeak_kb\tkind\tlog_line\tmessage\n'
} > "$OUT"
upload

# Starting at line one is deliberate: if the observer starts just after the state does, its earlier
# progress lines still enter the record.  Resource fields are meaningful only for lines observed
# after tracker launch; the intended use is to start this before the target first appears.
tail --pid="$PID" -n +1 -F "$LOG" 2>/dev/null \
    | grep --line-buffered -nF -- "$STATE" \
    | while IFS= read -r MATCH; do
        LINE_NO=${MATCH%%:*}
        MESSAGE=${MATCH#*:}
        SOLVER_SECS=$(ps -o etimes= -p "$PID" 2>/dev/null | tr -d ' ')
        [[ "$SOLVER_SECS" =~ ^[0-9]+$ ]] || SOLVER_SECS=0
        read -r _ RSS_KB _ < <(grep -m1 '^VmRSS:' "/proc/$PID/status" 2>/dev/null || echo 'x 0 kB')
        read -r _ DATA_KB _ < <(grep -m1 '^VmData:' "/proc/$PID/status" 2>/dev/null || echo 'x 0 kB')
        read -r _ PEAK_KB _ < <(grep -m1 '^VmPeak:' "/proc/$PID/status" 2>/dev/null || echo 'x 0 kB')
        KIND=progress
        if [[ "$MESSAGE" == can\ solve* || "$MESSAGE" == "can't solve"* ]]; then
            KIND=verdict
        fi
        printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
            "$(date -u +%FT%TZ)" "$SOLVER_SECS" "$RSS_KB" "$DATA_KB" "$PEAK_KB" \
            "$KIND" "$LINE_NO" "$MESSAGE" >> "$OUT"
        upload
        [[ "$KIND" == verdict ]] && exit 0
    done

upload
