#!/usr/bin/env bash
# Progress reporting for the Sa(193) cold re-derivation, designed so the run can be followed
# WITHOUT logging into AWS: an SNS email on every milestone plus a heartbeat, and a STATUS object
# in S3 for detail.
#
#   tools/sa193_watchdog.sh --log FILE --pid PID [--bucket B] [--topic ARN]
#                           [--interval 600] [--heartbeat 21600]
#
# The useful progress metric is not verdicts or elapsed time, it is **how many of the sixteen**
# top-level states are done. Sa(193) in 10 is unsolvable iff all sixteen Sb(n1:193-n1) are
# unsolvable in 9, for n1 = 97..112, so each of those is 1/16 of the job and each prints a line.
# Everything else here is context for that fraction.
set -uo pipefail

LOG= PID= BUCKET= TOPIC= INTERVAL=600 HEARTBEAT=21600
while (( $# )); do
    case "$1" in
        --log) LOG="$2"; shift 2 ;;
        --pid) PID="$2"; shift 2 ;;
        --bucket) BUCKET="$2"; shift 2 ;;
        --topic) TOPIC="$2"; shift 2 ;;
        --interval) INTERVAL="$2"; shift 2 ;;
        --heartbeat) HEARTBEAT="$2"; shift 2 ;;
        *) echo "unknown arg $1" >&2; exit 2 ;;
    esac
done
[[ -n "$LOG" && -n "$PID" ]] || { echo "need --log and --pid" >&2; exit 2; }

START=$(date +%s)
LAST_BEAT=0
LAST_DONE=-1
LAST_UPLOAD=0
# A run may span more than one process lifetime. Fixed keys would let segment two overwrite segment
# one, and segment two CITES facts proved in segment one - losing it recreates the exact defect that
# makes the 2023 corpus unusable: verdicts whose proofs are gone. So every artifact key carries the
# segment stamp, and nothing is ever overwritten.
SEG=${SEG:-$(date -u +%Y%m%dT%H%M%SZ)}
echo "segment $SEG"
notify() {   # never let a failed AWS call kill the watchdog - the run matters more than the report
    local subject="$1" body="$2"
    [[ -n "$TOPIC" ]] && aws sns publish --topic-arn "$TOPIC" \
        --subject "$(echo "$subject" | cut -c1-99)" --message "$body" >/dev/null 2>&1 || true
}

status() {
    local now elapsed rss done16 v last
    now=$(date +%s); elapsed=$(( now - START ))
    rss=$(ps -o rss= -p "$PID" 2>/dev/null | awk '{printf "%.2f", $1/1048576}')
    [[ -z "$rss" ]] && rss="-"
    # the sixteen: distinct n1 with a printed k=9 verdict on a 193-coin state
    done16=$(grep -o "Sb([0-9]*:[0-9]*)\[[0-9]*,193\] in 9" "$LOG" 2>/dev/null \
             | sed 's/Sb(\([0-9]*\):.*/\1/' | sort -u | wc -l | tr -d ' ')
    v=$(grep -c "solve" "$LOG" 2>/dev/null || echo 0)
    last=$(grep "solve" "$LOG" 2>/dev/null | tail -1 | cut -c1-120)
    printf 'Sa(193) cold re-derivation\n'
    printf '  top-level states done   %s of 16\n' "$done16"
    printf '  elapsed                 %dd %02dh %02dm\n' $((elapsed/86400)) $((elapsed%86400/3600)) $((elapsed%3600/60))
    printf '  verdicts                %s\n' "$v"
    printf '  resident memory         %s GB\n' "$rss"
    printf '  log size                %s\n' "$(du -h "$LOG" 2>/dev/null | cut -f1)"
    printf '  solver process          %s\n' "$(kill -0 "$PID" 2>/dev/null && echo alive || echo GONE)"
    printf '  per-k verdicts          %s\n' \
        "$(grep -o '\] in [0-9]*' "$LOG" 2>/dev/null | sort | uniq -c | sort -k3 -n \
           | awk '{printf "k%s:%s ", $4, $1}')"
    printf '  control                 %s\n' \
        "$(grep -m1 'result CONTROL' "$LOG" 2>/dev/null || echo 'not yet reported')"
    printf '  last verdict            %s\n' "$last"
    grep -m1 '^result Sa(193)' "$LOG" 2>/dev/null && printf '  *** FINAL ANSWER ABOVE ***\n'
}

while :; do
    S=$(status)
    printf '=== %s ===\n%s\n\n' "$(date -u +%FT%TZ)" "$S"      # also to stdout, for the instance log
    if [[ -n "$BUCKET" ]]; then
        printf '%s\n' "$S" | aws s3 cp - "s3://$BUCKET/run/STATUS" --content-type text/plain >/dev/null 2>&1 || true
        { date -u +%FT%TZ; printf '%s\n\n' "$S"; } | aws s3 cp - "s3://$BUCKET/run/STATUS-$(date -u +%F).log" \
            --content-type text/plain >/dev/null 2>&1 || true
    fi

    # $4 is the count: the line is "  top-level states done   N of 16", so $5 is the word "of".
    # Getting this wrong makes DONE a constant, which silently disables every milestone email and
    # leaves only the heartbeat - the failure is invisible because the heartbeat still arrives.
    DONE=$(printf '%s\n' "$S" | awk '/top-level states done/ {print $4}')
    NOW=$(date +%s)

    # Milestones worth an email: another of the sixteen is done, the control reported, the final
    # answer landed, or the solver died. Plus a heartbeat so silence is never ambiguous.
    if [[ "$DONE" != "$LAST_DONE" && "$LAST_DONE" != "-1" ]]; then
        notify "Sa(193): $DONE of 16 done" "$S"
    elif grep -q '^result Sa(193)' "$LOG" 2>/dev/null; then
        notify "Sa(193): FINISHED" "$S"
    elif ! kill -0 "$PID" 2>/dev/null; then
        notify "Sa(193): solver process is GONE" "$S"
    elif (( NOW - LAST_BEAT >= HEARTBEAT )); then
        notify "Sa(193): $DONE of 16, still running" "$S"; LAST_BEAT=$NOW
    fi
    [[ "$LAST_DONE" == "-1" ]] && { LAST_BEAT=$NOW; notify "Sa(193): run started" "$S"; }
    LAST_DONE="$DONE"

    if ! kill -0 "$PID" 2>/dev/null; then
        # Final upload: the raw log is the archival artifact, and its parsed form is the restart
        # checkpoint. Warm-starting a negative from a run's OWN output is sound.
        if [[ -n "$BUCKET" ]]; then
            head -c "$(stat -c%s "$LOG")" "$LOG" | zstd -q -19 -c | aws s3 cp - "s3://$BUCKET/run/seg-$SEG/out_sa193.txt.zst" >/dev/null 2>&1 || true
            { echo "# radio-cert v1 sa193 cold segment $SEG, final, generated $(date -u +%FT%TZ)";
              head -c "$(stat -c%s "$LOG")" "$LOG" | ./parse_out.sh; } | tee >(aws s3 cp - "s3://$BUCKET/run/seg-$SEG/sa193.checkpoint" >/dev/null 2>&1) \
              | aws s3 cp - "s3://$BUCKET/run/sa193.checkpoint" >/dev/null 2>&1 || true
        fi
        notify "Sa(193): run ended" "$(status)"
        exit 0
    fi

    # Hourly: stream the log out and refresh the restart checkpoint, so a lost instance costs at
    # most an hour rather than the whole run.
    # Explicit cadence, not (NOW-START) % 3600: iteration times drift by the work done each cycle,
    # so a modulo window silently skips hours.
    if (( NOW - LAST_UPLOAD >= 3600 )) && [[ -n "$BUCKET" ]]; then
        LAST_UPLOAD=$NOW
        head -c "$(stat -c%s "$LOG")" "$LOG" | zstd -q -3 -c | aws s3 cp - "s3://$BUCKET/run/seg-$SEG/out_sa193.txt.zst" >/dev/null 2>&1 || true
        { echo "# radio-cert v1 sa193 cold segment $SEG, generated $(date -u +%FT%TZ)";
          head -c "$(stat -c%s "$LOG")" "$LOG" | ./parse_out.sh; } | aws s3 cp - "s3://$BUCKET/run/seg-$SEG/sa193.checkpoint" >/dev/null 2>&1 || true
        aws s3 cp "s3://$BUCKET/run/seg-$SEG/sa193.checkpoint" "s3://$BUCKET/run/sa193.checkpoint" >/dev/null 2>&1 || true
    fi
    sleep "$INTERVAL"
done
