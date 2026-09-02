#!/usr/bin/env bash
# Verify sa193 level certificates with the cleanroom Rust checker, locally, with a
# self-identifying log. Levels default to everything except k=7 (k=7 is hours of CPU; run
# it via tools/cleanroom_k7_ec2_launch.sh or pass --with-k7 explicitly).
#
#   tools/cleanroom_verify_chain.sh [--threads N] [--with-k7] [--certs DIR] > chain.out
#
# The checker is not a tools/build_radio.py product, so tools/run_with_provenance.py does
# not apply; the header below records the same facts (commit, dirtiness, binary hash,
# toolchain, host, input hashes, command).
set -euo pipefail
cd "$(dirname "$0")/.."

THREADS=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 8)
WITH_K7=0
CERTS=.artifacts/cert
while (( $# )); do
    case "$1" in
        --threads) THREADS=$2; shift 2 ;;
        --with-k7) WITH_K7=1; shift ;;
        --certs) CERTS=$2; shift 2 ;;
        *) echo "usage: $0 [--threads N] [--with-k7] [--certs DIR]" >&2; exit 64 ;;
    esac
done

sha256_file() {
    if command -v sha256sum >/dev/null 2>&1; then sha256sum "$1" | awk '{print $1}'
    else shasum -a 256 "$1" | awk '{print $1}'
    fi
}

export PATH="$HOME/.cargo/bin:/opt/homebrew/opt/rustup/bin:$PATH"
( cd tools/cleanroom && cargo build --release >&2 )
binary=tools/cleanroom/target/release/radio_cleanroom

levels=(2 3 4 5 6 8 9)
(( WITH_K7 )) && levels=(2 3 4 5 6 7 8 9)

work=$(mktemp -d /tmp/radio-cleanroom-chain.XXXXXX)
trap 'rm -rf -- "$work"' EXIT
inputs=()
for k in "${levels[@]}"; do
    zst="$CERTS/sa193-k$k.cert.zst"
    [[ -f "$zst" ]] || { echo "missing $zst" >&2; exit 66; }
    zstd -d -q -o "$work/sa193-k$k.cert" "$zst"
    inputs+=("$work/sa193-k$k.cert")
done

echo "# radio-cleanroom-provenance-v1 begin"
echo "# artifact=cleanroom-verifier-output"
echo "# git_commit=$(git rev-parse HEAD)"
git diff --quiet HEAD -- tools/cleanroom \
    && echo "# checker_source_dirty=no" || echo "# checker_source_dirty=yes"
echo "# binary_sha256=$(sha256_file "$binary")"
echo "# toolchain=$(rustc --version) / $(cargo --version)"
echo "# host=$(hostname) uname=$(uname -sm) threads=$THREADS"
for k in "${levels[@]}"; do
    echo "# input=sa193-k$k.cert.zst sha256=$(sha256_file "$CERTS/sa193-k$k.cert.zst")"
done
echo "# command=radio_cleanroom audit --threads $THREADS sa193-k{${levels[*]// /,}}.cert"
echo "# started_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
echo "# radio-cleanroom-provenance-v1 end"

"$binary" audit --threads "$THREADS" "${inputs[@]}"
rc=$?
echo "# finished_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
exit "$rc"
