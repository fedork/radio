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
# `age` matters as much as the contents. The watchdog writes every 10 minutes, so a snapshot up to
# ten minutes old is normal and says nothing is wrong; one that is hours old means the watchdog died
# even though the solver may still be running fine.
set -uo pipefail
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
# `--prefix runN` reads one; `--all` reads every prefix listed in render() below. Run3 is the only
# live solver as of 2026-08-11; run7 remains in the comparison view as the final snapshot of the
# diagnosed scheduler failure.
PREFIX=run
BOTH=0
COMPARE=0
WATCH=0
while (( $# )); do
    case "$1" in
        --prefix) PREFIX="$2"; shift 2 ;;
        --both|--all) BOTH=1; shift ;;
        --compare) COMPARE=1; shift ;;
        --watch)  WATCH=1; shift ;;
        *) echo "usage: $0 [--prefix runN] [--all|--compare] [--watch]" >&2; exit 2 ;;
    esac
done

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
        banner run3 "full-star incumbent"
        banner run7 "depth-first compact sidecar (stopped)"
    elif (( BOTH )); then
        banner run  "original build"
        banner run2 "A+B optimisations"
        banner run3 "A+B + star-expansion majorization"
        banner run4 "compact cache (stopped prematurely)"
        banner run5 "exact L1 (stopped prematurely)"
        banner run6 "broken deadline experiment (aborted)"
        banner run7 "compact cache + exact L1 (stopped: obsolete deadlines)"
    else
        show "$PREFIX"
    fi
}
if (( WATCH )); then
    while :; do clear; render; sleep 60; done
else
    render
fi
