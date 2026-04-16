#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

usage() {
    echo "Usage: $0 [cache_file] k n1 n2 [n1 n2 ...]" >&2
    exit 1
}

if (( $# < 3 )); then
    usage
fi

args=("$@")
offset=0
cache_file=""
if (( $# % 2 == 0 )); then
    offset=1
    cache_file="${args[0]}"
    if [[ ! -f "$cache_file" ]]; then
        echo "Cache file not found: $cache_file" >&2
        exit 1
    fi
fi

k="${args[$offset]}"
if [[ ! "$k" =~ ^[0-9]+$ ]]; then
    echo "k must be a non-negative integer, got: $k" >&2
    exit 1
fi

pair_tokens=$(( $# - offset - 1 ))
if (( pair_tokens < 2 || pair_tokens % 2 != 0 )); then
    echo "Expected at least one n1 n2 pair after k." >&2
    usage
fi

max_k=$k
max_n=0
input_total_n=0

for (( i=offset+1; i<${#args[@]}; i++ )); do
    n="${args[$i]}"
    if [[ ! "$n" =~ ^[0-9]+$ ]]; then
        echo "n values must be non-negative integers, got: $n" >&2
        exit 1
    fi
    input_total_n=$(( input_total_n + n ))
done
max_n=$input_total_n

if [[ -n "$cache_file" ]]; then
    read -r file_max_n file_max_k < <(
        awk -v max_n="$max_n" -v max_k="$max_k" '
            {
                if (NF < 4) next;
                type = $2;
                if (type == "a") {
                    n = $3 + 0;
                    kk = $4 + 0;
                    if (n > max_n) max_n = n;
                    if (kk > max_k) max_k = kk;
                    next;
                }
                if (type == "b") {
                    line_n = 0;
                    for (i = 3; i <= NF; i++) {
                        if ($i == "t") {
                            if (i + 2 <= NF) {
                                n = $(i + 2) + 0;
                                if (n > line_n) line_n = n;
                            }
                            if (i + 3 <= NF) {
                                kk = $(i + 3) + 0;
                                if (kk > max_k) max_k = kk;
                            }
                            break;
                        }
                        line_n += $i + 0;
                    }
                    if (line_n > max_n) max_n = line_n;
                }
            }
            END {
                printf "%d %d\n", max_n, max_k;
            }
        ' "$cache_file"
    )
    max_n=$file_max_n
    max_k=$file_max_k
fi

if (( max_n < 2 )); then
    max_n=2
fi

echo "Compiling radio_full with MAX_K=$max_k MAX_N=$max_n"
: "${CC:=clang}"
"$CC" -O3 -DMAX_K="$max_k" -DMAX_N="$max_n" radio_full.c -o radio_full

exec ./radio_full "$@"
