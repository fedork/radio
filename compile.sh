#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
exec python3 tools/build_radio.py radioSbPareto.c -O3 -mcmodel=large -o radioSbPareto "$@"
