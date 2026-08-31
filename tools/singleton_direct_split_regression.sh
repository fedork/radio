#!/usr/bin/env bash
set -euo pipefail

repo_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
direct_split_tmp=$(mktemp -d "${TMPDIR:-/tmp}/radio-singleton-direct.XXXXXX")

cleanup_direct_split_tmp() {
    if [[ -n "${direct_split_tmp:-}" && -d "$direct_split_tmp" ]]; then
        rm -rf -- "$direct_split_tmp"
    fi
}
trap cleanup_direct_split_tmp EXIT

cd "$repo_dir"
CC="${CXX:-clang++}" tools/build_radio.py \
    -std=c++20 -O3 -Wall -Wextra -pedantic \
    tools/singleton_direct_split_cleanroom.cpp \
    -o "$direct_split_tmp/singleton-direct"
tools/run_with_provenance.py "$direct_split_tmp/singleton-direct" regression
if [[ "${1:-}" == "--with-k7" ]]; then
    tools/run_with_provenance.py \
        "$direct_split_tmp/singleton-direct" k7-dyadic-family-control
elif [[ $# -ne 0 ]]; then
    echo "usage: $0 [--with-k7]" >&2
    exit 64
fi
