#!/usr/bin/env bash
# Small exhaustive/sanitized regression for lineage degeneration and unit-capacity accounting.

set -euo pipefail

ROOT=$(cd "$(dirname "$0")/.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

cd "$ROOT"
tools/build_radio.py -O1 -g -fsanitize=address,undefined \
    -DMAX_K=3 -DMAX_N=18 tools/pareto_prefix_census.c -o "$TMP/census" >/dev/null

ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 \
RADIO_RUN_CONTEXT='pareto-prefix unit-reserve regression' \
    "$TMP/census" /dev/null data/pareto_sb.csv 3 10000 \
    > "$TMP/census.out" 2> "$TMP/census.err"

tools/analyze_pareto_prefix_census.py --pareto-csv data/pareto_sb.csv --json \
    "$TMP/census.out" > "$TMP/analysis.json"

# Reusing only SECOND_SUMMARY-closed blocks must reproduce every labelled first/second geometry.
# The exact-cache slot is `-`: each imported canonical winner is therefore re-verified by the
# current solver rather than trusted as a cache fact.
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 \
RADIO_RUN_CONTEXT='pareto-prefix checkpoint-replay regression' \
    "$TMP/census" /dev/null data/pareto_sb.csv 3 10000 - - "$TMP/census.out" \
    > "$TMP/replay.out" 2> "$TMP/replay.err"

tools/analyze_pareto_prefix_census.py --pareto-csv data/pareto_sb.csv --json \
    "$TMP/replay.out" > "$TMP/replay.json"

python3 - "$TMP/analysis.json" <<'PY'
import json
import sys

result = json.load(open(sys.argv[1]))
expected = {
    "root_k": 3,
    "roots": 4,
    "first_winners": 12,
    "second_winners": 28,
    "targets": 2,
    "upgrade_eligible_targets": 2,
    "endpoints": 2,
    "raw_full_winners": 3,
    "unit_extended_full_winners": 14,
    "representation_blocked": 0,
}
for key, value in expected.items():
    if result[key] != value:
        raise SystemExit(f"{key}: expected {value!r}, got {result[key]!r}")
if result["target_unit_reserve_histogram"] != {"1": 1, "3": 1}:
    raise SystemExit(f"wrong unit reserve histogram: {result['target_unit_reserve_histogram']}")
if result["endpoint_parts_histogram"] != {"0": 1, "1": 1}:
    raise SystemExit(f"wrong endpoint dimensions: {result['endpoint_parts_histogram']}")
PY

python3 - "$TMP/census.out" "$TMP/replay.out" <<'PY'
import sys


def geometry(path):
    records = []
    with open(path) as handle:
        for line in handle:
            if line.startswith("CENSUS\tFIRST\t"):
                records.append(line.rstrip())
            elif line.startswith("CENSUS\tLINEAGE\t"):
                # Target IDs depend on seed discovery order, not on labelled cut geometry.
                records.append(line.split("\ttarget=", 1)[0])
    return sorted(records)


if geometry(sys.argv[1]) != geometry(sys.argv[2]):
    raise SystemExit("checkpoint replay changed the labelled first/second census")
PY

echo "pareto-prefix census regression: OK"
