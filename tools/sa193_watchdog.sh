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

# Seconds since the solver started, or 0. BSD ps has no `etimes` and prints its usage to STDOUT, so
# every caller must validate numerically - not merely check for empty. This existed inline in status()
# and was then written again, unguarded, into the profile row; hence a function.
solver_age() {
    local a
    a=$(ps -o etimes= -p "$PID" 2>/dev/null | tr -d ' ')
    [[ "$a" =~ ^[0-9]+$ ]] && printf '%s' "$a" || printf '0'
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
    elapsed=$(solver_age)
    (( elapsed > 0 )) || elapsed=$(( now - START ))
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
            # Elide only the witness, and keep the cost tail that FOLLOWS it. The layout is
            #   can solve <state> in <k> with <witness...> took <s> totalsplits=<n> pass=<p> ...
            # so cutting from " with " to end-of-line also discards took/totalsplits/pass - which is
            # the most useful part: the k=9 control line reads "took 1745", i.e. 29 minutes for that
            # one state. Sb verdicts say "with [a:b] ..." and Sa verdicts "with following:...", so
            # match " with " for both.
            if (match(line, / with /)) {
                head = substr(line, 1, RSTART - 1)
                rest = substr(line, RSTART)
                tail = ""
                if (match(rest, / took .*$/)) tail = substr(rest, RSTART)
                line = head "  [+witness]" tail
            }
            last[k] = line; lastnr[k] = NR
        }
        # "still solving" lines are the real progress signal: they carry left=<remaining>/<total>
        # splits at that level and elapsed=<used>/<budget> against the deadline, so they say how far
        # through a level the search is. A completed verdict only says what finished, which at high k
        # can be hours stale. Record the log position too, so staleness is visible - the k=9 line can
        # still be the one from the control long after the control finished.
        /still solving in/ {
            match($0, /in [0-9]+/); sk = substr($0, RSTART + 3, RLENGTH - 3) + 0
            sline = $0
            sub(/^still solving in [0-9]+ /, "", sline)
            sub(/ trying .*elapsed /, "  elapsed ", sline)
            # An "elapsed X/Y" with Y barely above X is NOT a budget about to expire. On pass 2 every
            # descendant is given NO_DEADLINE (radiobase.c: child_deadline = pass<2 ? deadline :
            # NO_DEADLINE), and such a node does not return MAYBE when it passes its deadline - it
            # sets deadline = now + 10s and continues, forever. So Y tracks X at a fixed ~10s offset
            # and the node runs until it concludes. Saying "99.7% of budget used" about that is
            # exactly backwards: nothing is going to stop it.
            if (match(sline, /elapsed [0-9]+\/[0-9]+/)) {
                ef = substr(sline, RSTART + 8, RLENGTH - 8)
                split(ef, ep, "/")
                if (ep[2] - ep[1] <= 12) sub(/elapsed [0-9]+\/[0-9]+/, "elapsed " ef " (no deadline: auto-extends)", sline)
            }
            still[sk] = sline; stillnr[sk] = NR
        }
        { nr = NR }
        # the sixteen: distinct n1 with a printed k=9 verdict on a 193-coin state
        /\[[0-9]+,193\] in 9/ {
            if (match($0, /Sb\([0-9]+:/)) seen[substr($0, RSTART + 3, RLENGTH - 4)] = 1
        }
        END {
            n = 0; for (x in seen) n++
            printf "SIXTEEN %d\n", n
            printf "VERDICTS %d\n", v + 0
            for (k = 12; k >= 1; k--) if (k in cnt) printf "CNT %d %d\n", k, cnt[k]
            # Only from the current level UP. Levels below were last touched arbitrarily long
            # ago, so their "most recent" verdict is stale and placing it would mean scrolling
            # back through the whole log. Higher levels are the enclosing context: the path
            # from where the search is now toward the root.
            # Per level, take whichever is NEWER: the in-progress line or the completed verdict.
            # Either can be the more informative one - a level mid-search wants left=/elapsed=, a
            # level that just finished wants took=/totalsplits= - and which is current changes as the
            # search moves. Comparing log positions is the only way to know which.
            best = 0
            for (k = 12; k >= 1; k--) {
                sn = (k in stillnr) ? stillnr[k] : 0
                ln = (k in lastnr)  ? lastnr[k]  : 0
                if (sn == 0 && ln == 0) continue
                if (sn >= ln) { pick[k] = still[k]; picknr[k] = sn; kind[k] = "solving" }
                else          { pick[k] = last[k];  picknr[k] = ln; kind[k] = "done" }
                if (picknr[k] > best) { best = picknr[k]; curk = k }
            }
            printf "CURK %d\n", curk + 0
            for (k = 12; k >= curk; k--) if (k in pick)
                printf "ACT %d %s %d %s\n", k, kind[k], nr - picknr[k], substr(pick[k], 1, 200)
        }' "$LOG" 2>/dev/null)

    printf 'Sa(193) cold re-derivation\n'
    printf '  top-level states done   %s of 16\n' "$(awk '/^SIXTEEN/{print $2}' <<<"$scan")"
    printf '  solver running for      %dd %02dh %02dm\n' $((elapsed/86400)) $((elapsed%86400/3600)) $((elapsed%3600/60))
    printf '  verdicts                %s\n' "$(awk '/^VERDICTS/{print $2}' <<<"$scan")"
    printf '  resident memory         %s GB\n' "$rss"
    # Apparent size, not `du`. The log is on XFS, which speculatively preallocates blocks for a
    # growing file and trims them later, so `du` fluctuates and the log APPEARS TO SHRINK - 249M then
    # 122M for a file that only ever grows. That reads as truncation, which for the run's one
    # irreplaceable artifact is the most alarming thing a status line could say falsely.
    printf '  log size                %s\n' \
        "$(awk -v b="$(stat -c%s "$LOG" 2>/dev/null || stat -f%z "$LOG" 2>/dev/null)" \
             'BEGIN{ if (b >= 1073741824) printf "%.2f GB", b/1073741824; else printf "%.0f MB", b/1048576 }')"
    printf '  solver process          %s\n' "$(kill -0 "$PID" 2>/dev/null && echo alive || echo GONE)"
    # by k descending, so the levels nearest the root - where progress actually means something -
    # come first. Sorting these by COUNT, as an earlier version did, buried k=9 in the middle.
    printf '  verdicts by level       %s\n' \
        "$(awk '/^CNT/{printf "k%s:%s  ", $2, $3}' <<<"$scan")"
    printf '  control                 %s\n' \
        "$(grep -m1 'result CONTROL' "$LOG" 2>/dev/null || echo 'not yet reported')"
    printf '\n  latest activity per level, from the level the search is on (k=%s) up to the root\n' \
        "$(awk '/^CURK/{print $2}' <<<"$scan")"
    printf '  (solving: left=<splits remaining>/<total> is the progress; a deadline marked\n'
    printf '   auto-extends is NOT expiring - pass-2 descendants get NO_DEADLINE and bump it 10s)\n'
    awk '/^ACT/ { k=$2; kind=$3; age=$4; $1=""; $2=""; $3=""; $4=""; sub(/^    /,"")
                  printf "    k=%-2s %-7s %s%s\n", k, kind, $0,
                         (age > 200000 ? "   (stale)" : "") }' <<<"$scan"
    grep -m1 '^result Sa(193)' "$LOG" 2>/dev/null && printf '\n  *** FINAL ANSWER ABOVE ***\n'
}

while :; do
    S=$(status)
    printf '=== %s ===\n%s\n\n' "$(date -u +%FT%TZ)" "$S"      # also to stdout, for the instance log

    # Memory profile, one row per cycle, so consumption can be attributed after the fact rather than
    # only watched live. VmData alongside VmRSS is what makes the attribution possible: measured
    # 2026-08-06, VmData is ~100% of VmRSS and Pss_File is 98 kB, so the whole footprint is the
    # solver's own heap - the result cache - and nothing else. Paired with verdict counts and the
    # current level, a step in RSS can be pinned to the subtree that caused it.
    #
    # Fields are parsed back out of the status TEXT rather than reusing status()'s internals: it runs
    # inside a command substitution, so nothing it sets survives, local or not.
    if [[ -n "${PROFILE:-}" ]]; then
        read -r _ vmrss _ < <(grep -m1 '^VmRSS:'  /proc/$PID/status 2>/dev/null || echo "x 0 kB")
        read -r _ vmdata _ < <(grep -m1 '^VmData:' /proc/$PID/status 2>/dev/null || echo "x 0 kB")
        read -r _ vmpeak _ < <(grep -m1 '^VmPeak:' /proc/$PID/status 2>/dev/null || echo "x 0 kB")
        [[ -s "$PROFILE" ]] || echo "iso,solver_secs,rss_kb,vmdata_kb,vmpeak_kb,verdicts,curk,by_level" > "$PROFILE"
        printf '%s,%s,%s,%s,%s,%s,%s,"%s"\n' \
            "$(date -u +%FT%TZ)" \
            "$(solver_age)" \
            "$vmrss" "$vmdata" "$vmpeak" \
            "$(sed -n 's/^  verdicts  *\([0-9][0-9]*\)$/\1/p' <<<"$S")" \
            "$(sed -n 's/.*search is on (k=\([0-9]*\)).*/\1/p' <<<"$S")" \
            "$(sed -n 's/^  verdicts by level  *//p' <<<"$S")" >> "$PROFILE"
    fi
    if [[ -n "$BUCKET" ]]; then
        printf '%s\n' "$S" | aws s3 cp - "s3://$BUCKET/run/STATUS" --content-type text/plain >/dev/null 2>&1 || true

    fi

    # $4 is the count: the line is "  top-level states done   N of 16", so $5 is the word "of".
    # Getting this wrong makes DONE a constant, which silently disables every milestone email and
    # leaves only the heartbeat - the failure is invisible because the heartbeat still arrives.
    DONE=$(printf '%s\n' "$S" | awk '/top-level states done/ {print $4}')
    NOW=$(date +%s)

    # Exactly one email per cycle at most. The previous version sent TWO on every watchdog start:
    # LAST_BEAT began at 0, so `NOW - LAST_BEAT >= HEARTBEAT` was immediately true and fired the
    # heartbeat, and then the unconditional LAST_DONE == -1 line fired "run started" as well. With
    # the watchdog restarted a dozen times to patch it, that was a dozen duplicate pairs.
    if [[ "$LAST_DONE" == "-1" ]]; then
        # First cycle of THIS process. Start the heartbeat clock here, and distinguish a genuine
        # start from a reattach - the solver outlives the watchdog, so most starts are reattaches.
        LAST_BEAT=$NOW
        AGE=$(solver_age)
        if (( AGE < 2 * INTERVAL )); then
            notify "Sa(193): run started" "$S"
        else
            notify "Sa(193): watchdog reattached, $DONE of 16" "$S"
        fi
    elif grep -q '^result Sa(193)' "$LOG" 2>/dev/null; then
        # Once only. Without the flag this repeats every cycle for as long as the process lives.
        if [[ -z "${SENT_FINAL:-}" ]]; then SENT_FINAL=1; notify "Sa(193): FINISHED" "$S"; fi
    elif [[ "$DONE" != "$LAST_DONE" ]]; then
        notify "Sa(193): $DONE of 16 done" "$S"
    elif (( NOW - LAST_BEAT >= HEARTBEAT )); then
        notify "Sa(193): $DONE of 16, still running" "$S"; LAST_BEAT=$NOW
    fi
    # No "solver is GONE" branch: the exit path below already mails "run ended", and having both
    # meant two emails for one event.
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
        [[ -n "${PROFILE:-}" && -s "$PROFILE" ]] && aws s3 cp "$PROFILE" "s3://$BUCKET/run/seg-$SEG/memprofile.csv" >/dev/null 2>&1 || true
        # Both keys from ONE parse, via tee - an S3-to-S3 `cp` needs s3:GetObjectTagging, which the
        # run role deliberately does not have, and the failure was invisible behind `|| true`.
        { echo "# radio-cert v1 sa193 cold segment $SEG, generated $(date -u +%FT%TZ)";
          head -c "$(stat -c%s "$LOG")" "$LOG" | ./parse_out.sh; } \
          | tee >(aws s3 cp - "s3://$BUCKET/run/seg-$SEG/sa193.checkpoint" >/dev/null 2>&1) \
          | aws s3 cp - "s3://$BUCKET/run/sa193.checkpoint" >/dev/null 2>&1 || true
    fi
    sleep "$INTERVAL"
done
