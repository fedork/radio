#!/usr/bin/env bash
# Durable status writer for an sbwalk run: verified log archive, liveness, per-level stack, and
# SNS mail on milestones.
#
#   tools/sbwalk_heartbeat.sh --bucket B --prefix sbwalk/<run> --k 10 [--topic ARN]
#                             [--interval 600] [--digest 21600] [--once]
#
# Lives in the repo rather than inside the launcher's heredoc. Two escaping bugs came from
# embedding it there - an awk field reference eaten by the surrounding quotes, and a readiness
# guard that raced the walkers - and a file that can be syntax-checked and read is worth more than
# the convenience of a self-contained launcher.
#
# Three things it does that the naive version did not:
#
#   1. **Verifies the archive.** Streamed `zstd -c | aws s3 cp -` produced truncated objects on
#      2026-09-05, losing the `with [...]` strategies that witness reconstruction needs from a run
#      whose volume was already gone. Compress to a file, `zstd -t`, upload only on success.
#   2. **Leads with liveness.** The k=10 root progress line reprints only on a 20e9 work-unit
#      threshold, so it can sit byte-identical for half an hour while the solver does ten verdicts
#      a second. A monotone counter with a rate is the signal; the root line is not.
#   3. **Mails the milestones.** A WALK verdict is the answer of record and should not wait for
#      someone to poll S3.
set -uo pipefail

BUCKET= PREFIX= K=10 TOPIC= INTERVAL=600 DIGEST=21600 ONCE=0
while (( $# )); do
    case "$1" in
        --bucket) BUCKET="$2"; shift 2 ;;
        --prefix) PREFIX="$2"; shift 2 ;;
        --k) K="$2"; shift 2 ;;
        --topic) TOPIC="$2"; shift 2 ;;
        --interval) INTERVAL="$2"; shift 2 ;;
        --digest) DIGEST="$2"; shift 2 ;;
        --once) ONCE=1; shift ;;
        *) echo "unknown arg $1" >&2; exit 2 ;;
    esac
done
[[ -n "$BUCKET" && -n "$PREFIX" ]] || { echo "need --bucket and --prefix" >&2; exit 2; }

RUN_ID=${PREFIX##*/}
STATE=/root/.sbwalk_hb_state
RUN_DIR=/root/run
export AWS_DEFAULT_REGION=${AWS_DEFAULT_REGION:-us-west-2}

notify() {  # never let a failed AWS call kill the heartbeat; the run matters more than the report
    local subject="$1" body="$2"
    [[ -n "$TOPIC" ]] || return 0
    aws sns publish --topic-arn "$TOPIC" --subject "$(printf '%.99s' "$subject")" \
        --message "$body" >/dev/null 2>&1 || true
}

# Newest activity at each level, high k first. A "solving" line means that level is still open; a
# "done" line means it closed and its parent is waiting on a sibling. Lines are aged in log lines,
# so a large age on an enclosing level is normal rather than a stall.
render_stack() {
    awk -v maxk=12 '
        /^still solving in [0-9]+/ { k=$4+0; last[k]=$0; nr[k]=NR; kind[k]="solving"; next }
        match($0, / in [0-9]+ (took|with)/) {
            s=substr($0, RSTART+4); split(s, a, " "); k=a[1]+0
            last[k]=$0; nr[k]=NR; kind[k]="done"; next
        }
        END {
            # NR is the total at END; a separate counter would be skipped by the `next` above and
            # yield negative ages.
            for (k=maxk; k>=1; k--)
                if (k in last)
                    printf "    k=%-2d [%-7s] %7d lines ago  %.170s\n", k, kind[k], NR-nr[k], last[k]
        }' "$@" 2>/dev/null
}

status_body() {
    local now verdicts prev_t prev_v rate walks
    now=$(date +%s)
    verdicts=$(grep -chE " in [0-9]+ (took|with)" "$RUN_DIR"/walk_m*.txt 2>/dev/null | paste -sd+ - | bc)
    verdicts=${verdicts:-0}
    rate="n/a (first snapshot)"
    if [ -r "$STATE" ]; then
        read -r prev_t prev_v _ < "$STATE"
        if [ "${prev_t:-0}" -gt 0 ] && [ "$now" -gt "$prev_t" ]; then
            rate="$(( (verdicts - prev_v) * 60 / (now - prev_t) )) verdicts/min over the last $(( (now - prev_t) / 60 )) min"
        fi
    fi
    walks=$(grep -h '^WALK' "$RUN_DIR"/walk_m*.txt 2>/dev/null)

    echo "run_id    $RUN_ID"
    echo "written   $(date -u +%FT%TZ)"
    echo "alive     $(pgrep -c -x radio_sb_walk || echo 0)"
    echo "cpu       $(ps -o time= -C radio_sb_walk 2>/dev/null | tr -d ' ' | tr '\n' ' ')"
    echo "rss_kb    $(ps -o rss= -C radio_sb_walk 2>/dev/null | tr '\n' ' ')"
    echo "uptime    $(uptime)"
    echo
    echo "LIVENESS  verdicts $verdicts   $rate"
    echo "          truncated inserts: $(grep -oh 'cache=partial:' "$RUN_DIR"/walk_m*.txt 2>/dev/null | wc -l)"
    echo "          by level: $(grep -ohE ' in [0-9]+ (took|with)' "$RUN_DIR"/walk_m*.txt 2>/dev/null \
                  | cut -d' ' -f3 | sort -n | uniq -c | tr '\n' ' ')"
    echo
    echo "=== every WALK verdict so far (the answer of record) ==="
    echo "${walks:-(none yet)}"
    echo
    echo "=== stack: newest activity per level, $K down ==="
    echo "(an enclosing level reprints rarely; a large 'lines ago' there is normal, not a stall)"
    render_stack "$RUN_DIR"/walk_m*.txt
    printf '%s %s %s\n' "$now" "$verdicts" "$(printf '%s' "$walks" | grep -c . )" > "$STATE"
}

upload_all() {
    local f b ok=no
    for f in "$RUN_DIR"/walk_m*.txt; do
        [ -s "$f" ] || continue
        b=$(basename "$f" .txt)
        cp "$f" /root/.up.txt || continue
        if zstd -q -3 -f /root/.up.txt -o /root/.up.zst && zstd -t /root/.up.zst 2>/dev/null; then
            aws s3 cp /root/.up.zst "s3://$BUCKET/$PREFIX/logs/$b.txt.zst" --no-progress \
                >/dev/null 2>&1 && ok=yes
        fi
        rm -f /root/.up.txt /root/.up.zst
    done

    local body prev_walks now_walks last_digest
    prev_walks=0; last_digest=0
    [ -r "$STATE" ] && read -r _ _ prev_walks < "$STATE" 2>/dev/null
    [ -r "$STATE.digest" ] && read -r last_digest < "$STATE.digest" 2>/dev/null
    prev_walks=${prev_walks:-0}; last_digest=${last_digest:-0}

    body="$(status_body)
log_upload_verified $ok"
    printf '%s\n' "$body" | aws s3 cp - "s3://$BUCKET/$PREFIX/STATUS" \
        --content-type text/plain >/dev/null 2>&1 || true

    now_walks=$(grep -h '^WALK' "$RUN_DIR"/walk_m*.txt 2>/dev/null | grep -c .)
    now_walks=${now_walks:-0}
    # A new WALK verdict is the whole point of the run: mail it immediately.
    if [ "$now_walks" -gt "${prev_walks:-0}" ]; then
        notify "sbwalk $RUN_ID: $(grep -h '^WALK' "$RUN_DIR"/walk_m*.txt | tail -1)" "$body"
    elif [ $(( $(date +%s) - last_digest )) -ge "$DIGEST" ]; then
        notify "sbwalk $RUN_ID: still running, $now_walks verdict(s)" "$body"
        date +%s > "$STATE.digest"
    fi
}

upload_all
(( ONCE )) && exit 0
while pgrep -x radio_sb_walk >/dev/null; do
    sleep "$INTERVAL"
    upload_all
done
upload_all
notify "sbwalk $RUN_ID: run ended" "$(status_body)"
