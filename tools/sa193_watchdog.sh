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
    local now elapsed rss
    now=$(date +%s)
    # The SOLVER's age, not the watchdog's. The watchdog can be restarted (to patch it, or after a
    # crash) and its own uptime then reports minutes for a run that is days old - which is exactly
    # backwards for the one field people read to judge whether anything is wrong.
    # Must be validated as numeric, not merely non-empty: BSD ps has no `etimes` and prints its
    # usage to STDOUT, so an emptiness check passes and the arithmetic below then fails on a page of
    # option names. Linux ps supports it, which is what the instance runs.
    elapsed=$(ps -o etimes= -p "$PID" 2>/dev/null | tr -d ' ')
    [[ "$elapsed" =~ ^[0-9]+$ ]] || elapsed=$(( now - START ))
    rss=$(ps -o rss= -p "$PID" 2>/dev/null | awk '{printf "%.2f", $1/1048576}')
    [[ -z "$rss" ]] && rss="-"

    # ONE pass over the log for everything. The log reaches gigabytes, so the previous four
    # independent greps meant four full scans every cycle; this is strictly cheaper and it is what
    # makes a per-level "most recent verdict" affordable at all.
    local scan
    scan=$(awk '
        /solve/ { v++ }
        match($0, /\] in [0-9]+ /) {
            k = substr($0, RSTART + 5, RLENGTH - 6) + 0
            cnt[k]++
            # A positive verdict carries its whole witness tree inline ("with [a:b] Sb(...)Sb(...)"),
            # which is hundreds of characters and was being cut mid-token by a flat truncation. The
            # witness is not progress context - the state, the level and the cost are - so elide it
            # and keep the line intact. Negatives have no witness and keep their took/pass tail,
            # which is the useful part: it shows what each level is costing.
            line = $0
            if (match(line, / with \[/)) line = substr(line, 1, RSTART - 1) "  [+witness]"
            last[k] = line
        }
        # the sixteen: distinct n1 with a printed k=9 verdict on a 193-coin state
        /\[[0-9]+,193\] in 9/ {
            if (match($0, /Sb\([0-9]+:/)) seen[substr($0, RSTART + 3, RLENGTH - 4)] = 1
        }
        END {
            n = 0; for (x in seen) n++
            printf "SIXTEEN %d\n", n
            printf "VERDICTS %d\n", v + 0
            for (k = 12; k >= 1; k--) if (k in cnt) printf "CNT %d %d\n", k, cnt[k]
            for (k = 12; k >= 1; k--) if (k in last) printf "LAST %d %s\n", k, substr(last[k], 1, 118)
        }' "$LOG" 2>/dev/null)

    printf 'Sa(193) cold re-derivation\n'
    printf '  top-level states done   %s of 16\n' "$(awk '/^SIXTEEN/{print $2}' <<<"$scan")"
    printf '  solver running for      %dd %02dh %02dm\n' $((elapsed/86400)) $((elapsed%86400/3600)) $((elapsed%3600/60))
    printf '  verdicts                %s\n' "$(awk '/^VERDICTS/{print $2}' <<<"$scan")"
    printf '  resident memory         %s GB\n' "$rss"
    printf '  log size                %s\n' "$(du -h "$LOG" 2>/dev/null | cut -f1)"
    printf '  solver process          %s\n' "$(kill -0 "$PID" 2>/dev/null && echo alive || echo GONE)"
    # by k descending, so the levels nearest the root - where progress actually means something -
    # come first. Sorting these by COUNT, as an earlier version did, buried k=9 in the middle.
    printf '  verdicts by level       %s\n' \
        "$(awk '/^CNT/{printf "k%s:%s  ", $2, $3}' <<<"$scan")"
    printf '  control                 %s\n' \
        "$(grep -m1 'result CONTROL' "$LOG" 2>/dev/null || echo 'not yet reported')"
    printf '\n  most recent verdict at each level (k=9 is a top-level state, 1 of the 16):\n'
    awk '/^LAST/ { k=$2; $1=""; $2=""; sub(/^  /,""); printf "    k=%-2s %s\n", k, $0 }' <<<"$scan"
    grep -m1 '^result Sa(193)' "$LOG" 2>/dev/null && printf '\n  *** FINAL ANSWER ABOVE ***\n'
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
