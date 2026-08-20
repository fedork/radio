#!/usr/bin/env bash
#
# Build and start the warm-cache oracle.
#
#   ./run_radio_oracle.sh [--max-k K] [--max-n N] [--no-prime] [-- <extra cache files>]
#
# Sizing is a decision you make once, because the process is meant to stay up:
#
#   MAX_N must be at least the largest side-sum you will ever ask about, AND at least the largest
#   side-sum present in any cache you prime with -- replaying a fact wider than the tables is not
#   checked. The archived census caches reach 258, and Sa(193) states reach 193, so 300 is the
#   default: it covers both with headroom. Measured cost at MAX_K=9: MAX_N=300 is 37 s to init and
#   0.64 GB resident. Cost climbs steeply above that, so do not pad it "just in case".
#
#   MAX_K bounds the questions you can ask, not the memory much. 9 covers everything up to the
#   k=9 frontier.
#
# Priming replays the archived caches. They are not in git; fetch once with
#   tools/artifacts.sh pull pareto-census-k8-2026-08-19 .artifacts/pareto-census-k8
# and extract input/exact.cache and input/dominance.cache from its input.tar.zst, or point
# RADIO_ORACLE_CACHES at whatever you have.
#
# The oracle grows its result cache forever by design. For an unattended session wrap it:
#   tools/capped_run.sh --seconds 86400 --rss-gb 32 --label oracle -- ./run_radio_oracle.sh

set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")"

MAX_K=9
MAX_N=300
PRIME=1
EXTRA=()
while (($#)); do
    case "$1" in
        --max-k) MAX_K="$2"; shift 2 ;;
        --max-n) MAX_N="$2"; shift 2 ;;
        --no-prime) PRIME=0; shift ;;
        --) shift; EXTRA=("$@"); break ;;
        *) echo "unknown argument: $1" >&2; exit 64 ;;
    esac
done

BIN="./radio_oracle_k${MAX_K}_n${MAX_N}"
if [[ ! -x "$BIN" || radio_oracle.c -nt "$BIN" ]]; then
    echo "building $BIN (init is measured at ~37 s for k=9 n=300; larger MAX_N costs much more)" >&2
    tools/build_radio.py -O3 -DMAX_K="$MAX_K" -DMAX_N="$MAX_N" radio_oracle.c -o "$BIN" >&2
fi

CACHES=()
if ((PRIME)); then
    if [[ -n "${RADIO_ORACLE_CACHES:-}" ]]; then
        # shellcheck disable=SC2206
        CACHES=(${RADIO_ORACLE_CACHES})
    else
        for c in .artifacts/oracle-cache/exact.cache .artifacts/oracle-cache/dominance.cache; do
            [[ -f "$c" ]] && CACHES+=("$c")
        done
    fi
    if ((${#CACHES[@]} == 0)); then
        echo "no primer caches found; starting cold (see the header for how to fetch them)" >&2
    fi
fi

exec "$BIN" "${CACHES[@]}" "${EXTRA[@]}"
