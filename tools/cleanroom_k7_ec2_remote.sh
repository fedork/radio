#!/usr/bin/env bash
# Full sa193 k=7 verification with the cleanroom Rust checker on a dedicated EC2 host.
# Started by tools/cleanroom_k7_ec2_launch.sh via user data; the exact source is /root/source.
#
# Gates before the long run, in order: cargo test, the built-in exhaustive selftest against
# the unquotiented naive oracle, the closed and gap fixtures (the latter must FAIL), and a
# 1/5000 stride smoke of the k7 level itself. Only then the full audit runs, tee'd to a log
# whose header carries an explicit provenance block (the Rust checker is not built by
# tools/build_radio.py, so tools/run_with_provenance.py does not apply; the block below
# records the same facts by hand: source commit and bundle hash, binary hash, toolchain,
# host, input hashes, exact command).
set -euo pipefail

# cloud-init runs user data with no HOME, and `set -u` makes an unguarded $HOME fatal - that
# killed run 20260902T002643Z right after the toolchain install, leaving the host idle and
# billing because a `set -u` shell error does not reliably run the ERR trap. Pin all three
# explicitly, and give the toolchain a fixed home rather than an inherited one.
export HOME=${HOME:-/root}
export RUSTUP_HOME=${RUSTUP_HOME:-/root/.rustup}
export CARGO_HOME=${CARGO_HOME:-/root/.cargo}

BUCKET=radio-sa193-393287594714
WORK=
PREFIX=
RUN_ID=
SOURCE_COMMIT=
SOURCE_SHA256=
CERT_ZST_SHA256=5171ae78d2dd8a9ddb060f792e378a153d10df38c0b2e2bf0d2a158321459d7e
CERT_SHA256=41a233f73eea36012aa325eb1d61d1e3ca5bc857d6a7b30418c112cec6e8a9a6
SOURCE_DIR=/root/source
TOOLCHAIN=1.98.0

usage() {
    echo "usage: $0 --run-id ID --work /root/DIR --prefix S3_PREFIX --source-commit SHA --source-sha256 SHA" >&2
    exit 64
}
valid_sha() { [[ "$1" =~ ^[0-9a-f]{64}$ ]]; }

while (( $# )); do
    case "$1" in
        --run-id) RUN_ID=$2; shift 2 ;;
        --work) WORK=$2; shift 2 ;;
        --prefix) PREFIX=$2; shift 2 ;;
        --source-commit) SOURCE_COMMIT=$2; shift 2 ;;
        --source-sha256) SOURCE_SHA256=$2; shift 2 ;;
        *) usage ;;
    esac
done

[[ "$RUN_ID" =~ ^[0-9]{8}T[0-9]{6}Z$ ]] || usage
[[ "$WORK" == "/root/cleanroom-verify-$RUN_ID" ]] || usage
[[ "$PREFIX" == "cleanroom-verify/$RUN_ID" ]] || usage
[[ "$SOURCE_COMMIT" =~ ^[0-9a-f]{40}$ ]] || usage
valid_sha "$SOURCE_SHA256" || usage

mkdir -p "$WORK"
cd "$WORK"

upload() { aws s3 cp "$1" "s3://$BUCKET/$PREFIX/$2" --no-progress || true; }
stage() {
    printf '%s %s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "$1" | tee -a stages.log
    upload stages.log stages.log
}
fail() {
    stage "FAILED: $1"
    upload run.log run.log
    shutdown -h +2 || true
    exit 1
}
trap 'fail "unexpected error at line $LINENO"' ERR
# Belt and braces: a `set -u` expansion error exits without running the ERR trap, so catch
# every nonzero exit path here too. Otherwise the host sits idle at full price until the
# 12-hour systemd hard stop. FINISHED is set before the trap that reads it, or `set -u`
# would fault inside the handler itself.
FINISHED=0
trap 'rc=$?; if (( rc )) && (( ! FINISHED )); then
        printf "%s ABORTED rc=%d\n" "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "$rc" >> stages.log
        aws s3 cp stages.log "s3://$BUCKET/$PREFIX/stages.log" --no-progress || true
        aws s3 cp run.log "s3://$BUCKET/$PREFIX/run.log" --no-progress || true
        shutdown -h +2 || true
      fi' EXIT

stage SETUP
instance_id=$(TOKEN=$(curl -fsS -X PUT -H 'X-aws-ec2-metadata-token-ttl-seconds: 21600' \
    http://169.254.169.254/latest/api/token) && \
    curl -fsS -H "X-aws-ec2-metadata-token: $TOKEN" \
    http://169.254.169.254/latest/meta-data/instance-id)
instance_type=$(TOKEN=$(curl -fsS -X PUT -H 'X-aws-ec2-metadata-token-ttl-seconds: 21600' \
    http://169.254.169.254/latest/api/token) && \
    curl -fsS -H "X-aws-ec2-metadata-token: $TOKEN" \
    http://169.254.169.254/latest/meta-data/instance-type)
threads=$(nproc)

curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs \
    | sh -s -- -y --profile minimal --default-toolchain "$TOOLCHAIN" >> run.log 2>&1
export PATH="$HOME/.cargo/bin:$PATH"

aws s3 cp "s3://$BUCKET/$PREFIX/sa193-k7.cert.zst" sa193-k7.cert.zst --no-progress
printf '%s  sa193-k7.cert.zst\n' "$CERT_ZST_SHA256" | sha256sum -c -
zstd -d -q -o sa193-k7.cert sa193-k7.cert.zst
printf '%s  sa193-k7.cert\n' "$CERT_SHA256" | sha256sum -c -

stage BUILD
cd "$SOURCE_DIR/tools/cleanroom"
cargo build --release >> "$WORK/run.log" 2>&1
binary="$SOURCE_DIR/tools/cleanroom/target/release/radio_cleanroom"
binary_sha=$(sha256sum "$binary" | awk '{print $1}')

stage GATES
cargo test --release >> "$WORK/run.log" 2>&1 || fail "cargo test"
"$binary" selftest > "$WORK/selftest.out" 2>&1 || fail selftest
grep -q 'SELFTEST PASSED' "$WORK/selftest.out" || fail "selftest output"
"$binary" audit "$SOURCE_DIR/tools/testdata/radio_verify_v1.cert" \
    > "$WORK/fixture-closed.out" 2>&1 || fail "closed fixture"
if "$binary" audit "$SOURCE_DIR/tools/testdata/radio_refute_gap_v1.cert" \
    > "$WORK/fixture-gap.out" 2>&1; then fail "gap fixture did not fail closed"; fi
cd "$WORK"
"$binary" audit --threads "$threads" --stride 5000 sa193-k7.cert > smoke.out 2>&1 \
    || fail "k7 smoke"
grep -q ', gaps 0,' smoke.out || fail "k7 smoke gaps"
upload smoke.out smoke.out
upload selftest.out selftest.out

stage VERIFY
{
    echo "# radio-cleanroom-provenance-v1 begin"
    echo "# artifact=cleanroom-verifier-output"
    echo "# run_id=$RUN_ID"
    echo "# source_commit=$SOURCE_COMMIT"
    echo "# source_bundle_sha256=$SOURCE_SHA256"
    echo "# binary_sha256=$binary_sha"
    echo "# toolchain=$(rustc --version) / $(cargo --version)"
    echo "# host=$instance_id type=$instance_type threads=$threads"
    echo "# uname=$(uname -a)"
    echo "# input=sa193-k7.cert sha256=$CERT_SHA256 (zst $CERT_ZST_SHA256)"
    echo "# command=radio_cleanroom audit --threads $threads sa193-k7.cert"
    echo "# started_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
    echo "# radio-cleanroom-provenance-v1 end"
} > k7-verify.out
upload k7-verify.out k7-verify.out

( while sleep 600; do upload k7-verify.out k7-verify.out; done ) &
heartbeat=$!

/usr/bin/time -v "$binary" audit --threads "$threads" sa193-k7.cert >> k7-verify.out 2>&1 \
    || { kill "$heartbeat" || true; upload k7-verify.out k7-verify.out; fail "k7 verify"; }
kill "$heartbeat" 2>/dev/null || true
echo "# finished_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)" >> k7-verify.out

grep -q 'TOTAL verified 2508278, gaps 0,' k7-verify.out || { upload k7-verify.out k7-verify.out; fail "k7 totals"; }
zstd -q -19 -f k7-verify.out
upload k7-verify.out.zst k7-verify.out.zst
sha256sum k7-verify.out k7-verify.out.zst > final.sha256
upload final.sha256 final.sha256
upload run.log run.log
stage DONE
FINISHED=1
shutdown -h +2
