#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

usage() {
    echo "Usage: $0 target_k k n1 n2 [n1 n2 ...]" >&2
    exit 1
}

if (( $# < 4 )); then
    usage
fi

target_k="$1"
k="$2"

if [[ ! "$target_k" =~ ^[0-9]+$ ]]; then
    echo "target_k must be a non-negative integer, got: $target_k" >&2
    exit 1
fi

if [[ ! "$k" =~ ^[0-9]+$ ]]; then
    echo "k must be a non-negative integer, got: $k" >&2
    exit 1
fi

pair_tokens=$(( $# - 2 ))
if (( pair_tokens < 2 || pair_tokens % 2 != 0 )); then
    echo "Expected at least one n1 n2 pair after target_k and k." >&2
    usage
fi

max_k=$k
if (( target_k > max_k )); then
    max_k=$target_k
fi

max_n=0
args=("$@")
for (( i=2; i<${#args[@]}; i+=2 )); do
    n1="${args[$i]}"
    n2="${args[$i+1]}"

    if [[ ! "$n1" =~ ^[0-9]+$ || ! "$n2" =~ ^[0-9]+$ ]]; then
        echo "n values must be non-negative integers, got: $n1 $n2" >&2
        exit 1
    fi

    pair_sum=$(( n1 + n2 ))
    if (( pair_sum > max_n )); then
        max_n=$pair_sum
    fi
done

if (( max_n < 2 )); then
    max_n=2
fi

echo "Compiling radio_canon_search_generic with MAX_K=$max_k MAX_N=$max_n"
: "${CC:=clang}"
"$CC" -O3 -DMAX_K="$max_k" -DMAX_N="$max_n" radio_canon_search_generic.c -o radio_canon_search_generic

exec ./radio_canon_search_generic "$@"
