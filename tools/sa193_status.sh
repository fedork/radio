#!/usr/bin/env bash
# Show the Sa(193) run's status, and when it was last written.
#
#   tools/sa193_status.sh          # the current snapshot
#   tools/sa193_status.sh --watch  # refresh every 60 s
#
# `aws s3 cp <src> -` is the incantation: cp needs a destination, and `-` means stdout. Forgetting
# the `-` gives "the following arguments are required: paths", which is what this script exists to
# save you from.
#
# `age` matters as much as the contents. Live watchdogs write every 5-10 minutes, so that much age is
# normal and says nothing is wrong; hours means the watchdog died even if the solver is still alive.
set -uo pipefail
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
BUCKET=radio-sa193-393287594714
INSTANCE=i-0005d74f985c52ae1
# Concurrent runs share the instance from 2026-08-09, each under its own S3 prefix:
#   run   original build (2026-08-05)
#   run2  + A+B k=6 optimisations (efadab0)
#   run3  + full star-expansion majorization (3cf1406)
#   run4  + level-lazy tables and compact last-segment Pareto cache (6af384e bundle)
#   run5  + bounded exact-state L1 before majorization/cache lookup (290a892)
#   run6  + broken zero-progress deadline/prefix polling experiment (c13b5d3)
#   run7  + restored depth-first progress and pass-2 NO_DEADLINE handoff (e648e83)
#   run8  + compact cache and bounded long-state probes (current main at launch)
# `--prefix runN` reads one; `--all` reads every prefix listed in render() below. The default live
# comparison is run3 against run8; override the candidate with `--candidate runN` for archaeology.
PREFIX=run
BOTH=0
COMPARE=0
WATCH=0
CANDIDATE=run8
while (( $# )); do
    case "$1" in
        --prefix) PREFIX="$2"; shift 2 ;;
        --candidate) CANDIDATE="$2"; shift 2 ;;
        --both|--all) BOTH=1; shift ;;
        --compare) COMPARE=1; shift ;;
        --watch)  WATCH=1; shift ;;
        *) echo "usage: $0 [--prefix runN] [--all|--compare] [--candidate runN] [--watch]" >&2; exit 2 ;;
    esac
done
[[ "$CANDIDATE" =~ ^[a-zA-Z0-9_-]+$ ]] || { echo "invalid candidate prefix" >&2; exit 2; }

object_exists() {
    aws-vault exec --server default -- aws s3 ls "s3://$BUCKET/$1" >/dev/null 2>&1
}

object_age_minutes() {
    local mod="$1" now
    now=$(date -u +%s)
    printf '%s' $(( (now - $(date -u -j -f "%Y-%m-%dT%H:%M:%S" \
        "${mod%%+*}" +%s 2>/dev/null || echo "$now")) / 60 ))
}

compact_count() {
    awk -v n="$1" 'BEGIN {
        if (n >= 1000000) printf "%.2fM", n/1000000
        else if (n >= 1000) printf "%.1fk", n/1000
        else printf "%d", n
    }'
}

compact_duration() {
    local seconds="${1:-0}"
    [[ "$seconds" =~ ^[0-9]+$ ]] || { printf '%s' '-'; return; }
    if (( seconds >= 86400 )); then
        printf '%dd%02dh%02dm' $((seconds / 86400)) $((seconds % 86400 / 3600)) \
            $((seconds % 3600 / 60))
    elif (( seconds >= 3600 )); then
        printf '%dh%02dm' $((seconds / 3600)) $((seconds % 3600 / 60))
    elif (( seconds >= 60 )); then
        printf '%dm%02ds' $((seconds / 60)) $((seconds % 60))
    else
        printf '%ss' "$seconds"
    fi
}

compact_level_times() {
    local snapshot="$1"
    python3 "$SCRIPT_DIR/sa193_level_times.py" <<<"$snapshot"
}

compact_row() {
    local p="$1" snapshot mod status wall cpu roots control verdicts rss log age updated
    if ! object_exists "$p/STATUS"; then
        printf '%-6s %-5s %-9s %-9s %-5s %-10s %-9s %-7s %-6s %s\n' \
            "$p" "-" "-" "-" "-" "pending" "-" "-" "-" "-"
        return
    fi
    mod=$(aws-vault exec --server default -- aws s3api head-object --bucket "$BUCKET" \
        --key "$p/STATUS" --query LastModified --output text 2>/dev/null)
    snapshot=$(aws-vault exec --server default -- aws s3 cp "s3://$BUCKET/$p/STATUS" - 2>/dev/null)
    status=$(sed -n 's/^  solver process  *//p' <<<"$snapshot")
    [[ "$status" == alive ]] && status=live || status=done
    wall=$(sed -n 's/^  solver running for  *//p' <<<"$snapshot" | tr -d ' ')
    cpu=$(awk '$1 == "cpu" {sub(/s$/, "", $2); print $2; exit}' <<<"$snapshot")
    cpu=$(compact_duration "$cpu")
    roots=$(awk '/top-level states done/{print $4 "/16"}' <<<"$snapshot")
    verdicts=$(awk '$1 == "verdicts" && $2 ~ /^[0-9]+$/ {print $2; exit}' <<<"$snapshot")
    verdicts=$(compact_count "${verdicts:-0}")
    rss=$(sed -n 's/^  resident memory  *//p' <<<"$snapshot" | sed 's/ GB$/G/')
    log=$(sed -n 's/^  log size  *//p' <<<"$snapshot" | tr -d ' ')
    control=$(sed -n 's/^  control  *//p' <<<"$snapshot")
    if [[ "$control" == result\ CONTROL* ]]; then
        control=$(awk 'match($0, /\([0-9.]+ s\)/) {
            value=substr($0,RSTART+1,RLENGTH-2); gsub(/ /,"",value); print "ok:" value
        }' <<<"$control")
    else
        control=pending
    fi
    age=$(object_age_minutes "$mod")
    updated="${age}m"
    if (( age > 30 )); then
        [[ "$status" == done ]] && updated="${age}m-final" || updated="${age}m-STALE"
    fi
    printf '%-6s %-5s %-9s %-9s %-5s %-10s %-9s %-7s %-6s %s\n' \
        "$p" "$status" "${wall:--}" "$cpu" "${roots:--}" "$control" "$verdicts" \
        "${rss:--}" "${log:--}" "$updated"
    # Keep the useful old view: the current recursive path from the root down to the active level.
    # STATUS already caps each activity line at 200 characters; omit its prose legend here.
    awk '
        /^  latest activity per level/ { in_stack=1; next }
        /^  time by level/ { in_stack=0 }
        in_stack && /^    k=/ { sub(/^    /, "       "); print }
    ' <<<"$snapshot"
    compact_level_times "$snapshot"
}

show() {
    local PREFIX="$1"
    local mod age now snapshot age_note
    mod=$(aws-vault exec --server default -- aws s3api head-object --bucket "$BUCKET" --key "$PREFIX/STATUS" \
            --query LastModified --output text 2>/dev/null)
    snapshot=$(aws-vault exec --server default -- aws s3 cp "s3://$BUCKET/$PREFIX/STATUS" - 2>/dev/null)
    printf '%s\n' "$snapshot"
    if [[ -n "$mod" ]]; then
        now=$(date -u +%s)
        age=$(( now - $(date -u -j -f "%Y-%m-%dT%H:%M:%S" "${mod%%+*}" +%s 2>/dev/null || echo "$now") ))
        age_note=
        if (( age > 1800 )); then
            if grep -q 'solver process  *GONE' <<<"$snapshot"; then
                age_note=', final snapshot'
            else
                age_note=', STALE - watchdog may be dead'
            fi
        fi
        printf '\n  status written     %s  (%d min ago%s)\n' "$mod" $(( age / 60 )) "$age_note"
    fi
    printf '  instance           %s\n' \
        "$(aws-vault exec --server default -- aws ec2 describe-instances --instance-ids "$INSTANCE" \
             --query 'Reservations[].Instances[].State.Name' --output text 2>/dev/null)"
}

# `run` is the original build; `run2` carries the A+B k=6 optimisations. --both prints them one
# after the other so the comparison the second run exists to make can be read at a glance.
banner() {
    local p="$1" label="$2"
    printf '\n===== %s  (s3://%s/%s/) =====\n' "$label" "$BUCKET" "$p"
    if ! aws-vault exec --server default -- aws s3 ls "s3://$BUCKET/$p/STATUS" >/dev/null 2>&1; then
        printf '  no STATUS object yet - run not started, or watchdog has not written its first cycle\n'
        return
    fi
    show "$p"
}
render() {
    if (( COMPARE )); then
        printf 'Sa(193) live comparison  %s\n' "$(date -u +%FT%TZ)"
        printf '%-6s %-5s %-9s %-9s %-5s %-10s %-9s %-7s %-6s %s\n' \
            RUN STATE WALL CPU ROOT CONTROL VERDICTS RSS LOG UPDATE
        compact_row run3
        compact_row "$CANDIDATE"
        printf '\n'
        if object_exists "$CANDIDATE/COMPARE"; then
            aws-vault exec --server default -- aws s3 cp \
                "s3://$BUCKET/$CANDIDATE/COMPARE" - 2>/dev/null | awk '
                    # Run8 was launched with the first comparator. Hide its now-retired by-level
                    # block client-side rather than altering the frozen remote monitor files.
                    /^call=inclusive CPU; self=/ { next }
                    /^level[[:space:]]/ { drop=1; next }
                    drop && /^slow calls selected/ { drop=0 }
                    !drop { print }
                '
        else
            printf 'comparison pending first %s watchdog cycle\n' "$CANDIDATE"
        fi
        printf '\ninstance %s  %s\n' "$INSTANCE" \
            "$(aws-vault exec --server default -- aws ec2 describe-instances --instance-ids "$INSTANCE" \
                 --query 'Reservations[].Instances[].State.Name' --output text 2>/dev/null)"
    elif (( BOTH )); then
        banner run  "original build"
        banner run2 "A+B optimisations"
        banner run3 "A+B + star-expansion majorization"
        banner run4 "compact cache (stopped prematurely)"
        banner run5 "exact L1 (stopped prematurely)"
        banner run6 "broken deadline experiment (aborted)"
        banner run7 "compact cache + exact L1 (stopped: obsolete deadlines)"
        banner run8 "compact cache + bounded probes"
    else
        show "$PREFIX"
    fi
}
if (( WATCH )); then
    while :; do clear; render; sleep 60; done
else
    render
fi
