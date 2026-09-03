#!/bin/sh
# Canonical-tree sweep over Pareto frontier maxima read from data/pareto_sb.csv.
#
#   tools/sweep_canon_frontier.sh <binary> <k> <cpu_seconds> <target_k> <outdir> <m>...
#
# For each m it looks up n1 in the k-row of data/pareto_sb.csv and searches for a canonical
# witness tree for Sb(n1:m) @k, strictly one search at a time.  Build the binary first with
# compile-time memory bounds, e.g.
#
#   tools/build_radio.py -O3 -DMAX_K=8 -DMAX_N=260 -DMAX_STATE_SIZE=1024 \
#       -DMAX_TREE_NODES=400000 -DMAX_MEMO=4000000 radio_canon_search_generic.c -o canon8n
#
# `ulimit -t` is the only portable runtime bound on macOS; memory must be bounded at compile
# time (docs/tools.md).  The four outcomes are kept distinct on purpose:
#
#   TREE / TERMINAL  a canonical tree or a single canonical terminal was printed
#   NO_TREE          genuine exhaustion at this target_k -- NOT an unsolvability claim
#   OUT_OF_NODES     node-pool abort -- NOT evidence of absence
#   CPU_CAP          hit ulimit -t -- NOT evidence of absence
#
# Convert a TREE outcome into a checkable witness with tools/canon_out_to_tree.py, then run
# tools/check_witness.py.  Only that last step decides whether it is a proof.
set -eu
bin=$1; shift
k=$1; shift
cpu=$1; shift
target_k=$1; shift
outdir=$1; shift
mkdir -p "$outdir"

for m in "$@"; do
    n1=$(awk -F, -v k="$k" -v m="$m" '$1==k && $2==m {print $3; exit}' data/pareto_sb.csv)
    if [ -z "${n1:-}" ]; then
        echo "m=$m: no k=$k row in data/pareto_sb.csv"
        continue
    fi
    out="$outdir/canon_${n1}_${m}_at${k}.out"
    rc=0
    ( ulimit -t "$cpu"; "$bin" "$target_k" "$k" "$n1" "$m" > "$out" 2>&1 ) || rc=$?
    if grep -q 'out of nodes' "$out"; then
        verdict=OUT_OF_NODES
    elif grep -q 'NO_CANONICAL_TREE' "$out"; then
        verdict=NO_TREE
    elif grep -q -- '--\[' "$out"; then
        verdict=TREE
    elif grep -q 'canonical U_' "$out"; then
        verdict=TERMINAL
    elif [ "$rc" -ge 128 ]; then
        verdict=CPU_CAP
    else
        verdict="UNKNOWN(rc=$rc)"
    fi
    reach=$(grep -o 'REACH: .*' "$out" | tail -1 || true)
    echo "m=$m Sb($n1:$m)@$k target_k=$target_k rc=$rc $verdict ${reach:-}"
done
