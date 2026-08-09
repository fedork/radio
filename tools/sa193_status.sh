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
# `--prefix runN` reads one; `--all` reads every prefix in RUNS below.
PREFIX=run
BOTH=0
WATCH=0
while (( $# )); do
    case "$1" in
        --prefix) PREFIX="$2"; shift 2 ;;
        --both|--all) BOTH=1; shift ;;
        --watch)  WATCH=1; shift ;;
        *) echo "usage: $0 [--prefix runN] [--all] [--watch]" >&2; exit 2 ;;
    esac
done

show() {
    local PREFIX="$1"
    local mod age now
    mod=$(aws-vault exec --server default -- aws s3api head-object --bucket "$BUCKET" --key "$PREFIX/STATUS" \
            --query LastModified --output text 2>/dev/null)
    aws-vault exec --server default -- aws s3 cp "s3://$BUCKET/$PREFIX/STATUS" - 2>/dev/null
    if [[ -n "$mod" ]]; then
        now=$(date -u +%s)
        age=$(( now - $(date -u -j -f "%Y-%m-%dT%H:%M:%S" "${mod%%+*}" +%s 2>/dev/null || echo "$now") ))
        printf '\n  status written     %s  (%d min ago%s)\n' "$mod" $(( age / 60 )) \
            "$( (( age > 1800 )) && printf ', STALE - watchdog may be dead' )"
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
    if (( BOTH )); then
        banner run  "original build"
        banner run2 "A+B optimisations"
        banner run3 "A+B + star-expansion majorization"
    else
        show "$PREFIX"
    fi
}
if (( WATCH )); then
    while :; do clear; render; sleep 60; done
else
    render
fi
