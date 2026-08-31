#!/bin/sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$repo_dir"

certificate_tmp=$(mktemp -d "${TMPDIR:-/tmp}/radio-tight-band-certificate.XXXXXX")
trap 'rm -rf "$certificate_tmp"' EXIT HUP INT TERM

CC=clang++ tools/build_radio.py \
    -std=c++20 -O3 -Wall -Wextra -pedantic \
    tools/singleton_tight_band_certificate.cpp \
    -o "$certificate_tmp/tight-band-certificate"

tools/run_with_provenance.py \
    "$certificate_tmp/tight-band-certificate" regression

tools/run_with_provenance.py \
    "$certificate_tmp/tight-band-certificate" survey-capacity-bands 5

tools/run_with_provenance.py \
    "$certificate_tmp/tight-band-certificate" survey-dyadic-family 15

tools/singleton_direct_split_regression.sh --with-k7
