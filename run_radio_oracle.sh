#!/usr/bin/env bash
#
# Build and start the warm-cache oracle.
#
#   ./run_radio_oracle.sh [--max-k K] [--max-n N] [--no-prime] [--journal FILE] [-- <cache files>]
#
# Sizing is a decision you make once, because the process is meant to stay up:
#
#   MAX_N must be at least the largest side-sum you will ever ask about. It no longer has to cover
#   the caches: the loader SKIPS facts it cannot represent and reports how many, so a narrow build
#   stays usable with a wide cache. 300 covers the archived caches (widest fact 258) and Sa(193)
#   states (193). Measured at MAX_K=9: MAX_N=300 inits in 37 s at 0.64 GB, MAX_N=485 in 146 s.
#   Init cost is not a clean function of MAX_N, so measure a candidate rather than reasoning about
#   it -- and query cost varies enormously with what refutation work a state actually needs.
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
#
# --journal FILE appends every verdict the oracle computes, in the same format the loader reads, so
# a session's work primes the next one. That is usually a better primer than the archived caches:
# it is exactly the states you ask about, and it is small.

set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")"

MAX_K=9
MAX_N=300
PRIME=1
JOURNAL=""
EXTRA=()
while (($#)); do
    case "$1" in
        --max-k) MAX_K="$2"; shift 2 ;;
        --max-n) MAX_N="$2"; shift 2 ;;
        --no-prime) PRIME=0; shift ;;
        --journal) JOURNAL="$2"; shift 2 ;;
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

ARGS=()
[[ -n "$JOURNAL" ]] && ARGS+=("--journal=$JOURNAL")
exec "$BIN" "${ARGS[@]}" "${CACHES[@]}" "${EXTRA[@]}"
