#!/usr/bin/env bash
# Full sa193 certificate verification with the cleanroom Rust checker on a dedicated EC2 host.
# Started by tools/cleanroom_ec2_launch.sh via user data; the exact source is /root/source.
#
# Both halves of acceptance run here, so one retained artifact answers both questions:
#   structure - tools/check_level_chain.py: counts, inductive closure (level k's support is
#               exactly level k-1's claims as resolved states), empty base, expected roots;
#   semantics - the cleanroom checker audits every claim of every level.
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
LEVELS=
# Flat mode: audit one self-contained `radio-negative-certificate-v1` instead of an sa193 level
# chain. The flat file carries every level, so the checker's own k-stratified pass is the whole
# audit; the chain-structure and claim-total gates below are sa193 chain properties and do not
# apply. Used for the K=8 frontier certificate (evidence/pareto8_certificate_2026-09-03.md).
FLAT_CERT=
FLAT_SHA256=
FLAT_EXPECT_CLAIMS=
# Progress cadence. The S3 heartbeat matches it: uploading half as often as the checker
# reports left the watcher one or two lines behind and looking stalled.
PROGRESS_SECONDS=300
EXPECT_TOP_SUM=193
EXPECT_TOTAL_CLAIMS=2846568
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
        --levels) LEVELS=$2; shift 2 ;;
        --flat-cert) FLAT_CERT=$2; shift 2 ;;
        --flat-sha256) FLAT_SHA256=$2; shift 2 ;;
        --flat-expect-claims) FLAT_EXPECT_CLAIMS=$2; shift 2 ;;
        *) usage ;;
    esac
done

[[ "$RUN_ID" =~ ^[0-9]{8}T[0-9]{6}Z$ ]] || usage
[[ "$WORK" == "/root/cleanroom-verify-$RUN_ID" ]] || usage
[[ "$PREFIX" == "cleanroom-verify/$RUN_ID" ]] || usage
[[ "$SOURCE_COMMIT" =~ ^[0-9a-f]{40}$ ]] || usage
valid_sha "$SOURCE_SHA256" || usage
if [[ -n "$FLAT_CERT" ]]; then
    [[ "$FLAT_CERT" =~ ^[A-Za-z0-9._-]+\.cert\.zst$ ]] || usage
    valid_sha "$FLAT_SHA256" || usage
    [[ -z "$LEVELS" ]] || usage
    [[ -z "$FLAT_EXPECT_CLAIMS" || "$FLAT_EXPECT_CLAIMS" =~ ^[0-9]+$ ]] || usage
else
    [[ "$LEVELS" =~ ^[2-9]( [2-9])*$ ]] || usage
fi

mkdir -p "$WORK"
cd "$WORK"

# Absolute paths throughout: the build and gate stages cd into the crate directory, and a
# relative stages.log there would create a second file and clobber the uploaded one (run
# 20260902T004213Z lost its GATES line that way - cosmetic, but confusing to watch).
upload() { aws s3 cp "$WORK/$1" "s3://$BUCKET/$PREFIX/$2" --no-progress || true; }
stage() {
    printf '%s %s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "$1" | tee -a "$WORK/stages.log"
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
        printf "%s ABORTED rc=%d\n" "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "$rc" >> "$WORK/stages.log"
        aws s3 cp "$WORK/stages.log" "s3://$BUCKET/$PREFIX/stages.log" --no-progress || true
        aws s3 cp "$WORK/run.log" "s3://$BUCKET/$PREFIX/run.log" --no-progress || true
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

certs=()
if [[ -n "$FLAT_CERT" ]]; then
    aws s3 cp "s3://$BUCKET/$PREFIX/$FLAT_CERT" "$FLAT_CERT" --no-progress
    printf '%s  %s\n' "$FLAT_SHA256" "$FLAT_CERT" | sha256sum -c -
    flat_plain=${FLAT_CERT%.zst}
    zstd -d -q -o "$flat_plain" "$FLAT_CERT"
    certs+=("$flat_plain")
else
    aws s3 cp "s3://$BUCKET/$PREFIX/MANIFEST.sha256" MANIFEST.sha256 --no-progress
    for k in $LEVELS; do
        aws s3 cp "s3://$BUCKET/$PREFIX/sa193-k$k.cert.zst" "sa193-k$k.cert.zst" --no-progress
        grep " sa193-k$k.cert.zst\$" MANIFEST.sha256 | sha256sum -c -
        zstd -d -q -o "sa193-k$k.cert" "sa193-k$k.cert.zst"
        certs+=("sa193-k$k.cert")
    done
fi

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
# Smoke the most EXPENSIVE level, not the highest-numbered one: k=9 has 16 claims and would
# gate nothing, while k=7 is ~99% of the work. Certificate size is the available proxy for
# cost here, and it picks k=7 for the full chain.
if [[ -n "$FLAT_CERT" ]]; then
    # A flat certificate carries every level in one file, so one strided pass samples all of
    # them - including the expensive one - and pays each level's support build exactly once.
    echo "smoke: flat ${certs[0]} stride 5000" >> "$WORK/run.log"
    "$binary" audit --threads "$threads" --stride 5000 "${certs[0]}" \
        > "$WORK/smoke.out" 2>&1 || fail "smoke"
else
    smoke_level=$(for k in $LEVELS; do
        printf '%s %s\n' "$(wc -c < "sa193-k$k.cert")" "$k"
    done | sort -rn | head -1 | awk '{print $2}')
    echo "smoke level: k=$smoke_level" >> "$WORK/run.log"
    "$binary" audit --threads "$threads" --stride 5000 "sa193-k$smoke_level.cert" \
        > "$WORK/smoke.out" 2>&1 || fail "smoke"
fi
grep -q ', gaps 0,' "$WORK/smoke.out" || fail "smoke gaps"
upload smoke.out smoke.out
upload selftest.out selftest.out

# Inductive closure and the expected roots are properties of the WHOLE chain; on a --levels
# subset the support of the lowest level is legitimately absent, so the check is skipped
# rather than run in a form that cannot pass.
if [[ -n "$FLAT_CERT" ]]; then
    # check_level_chain.py checks a v2 level chain; a flat v1 has no per-level sections for it
    # to relate, and the checker's own ascending k-stratified pass supplies the induction --
    # level k is audited against level k-1's claims, which are themselves audited claims.
    stage "STRUCTURE skipped (flat certificate)"
    echo 'skipped: flat certificate, no level chain to relate' > "$WORK/chain-structure.out"
elif [[ "$LEVELS" == "2 3 4 5 6 7 8 9" ]]; then
    stage STRUCTURE
    python3 "$SOURCE_DIR/tools/check_level_chain.py" --expect-top-sum "$EXPECT_TOP_SUM" \
        "${certs[@]}" > "$WORK/chain-structure.out" 2>&1 || fail "level chain structure"
    grep -q 'chain is internally consistent, inductively closed and terminating' \
        "$WORK/chain-structure.out" || fail "level chain structure verdict"
    upload chain-structure.out chain-structure.out
else
    stage "STRUCTURE skipped (partial --levels)"
    echo 'skipped: partial level set' > "$WORK/chain-structure.out"
fi

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
    if [[ -n "$FLAT_CERT" ]]; then
        echo "# mode=flat"
        echo "# input=$FLAT_SHA256  $FLAT_CERT"
        echo "# structure=skipped (flat certificate)"
    else
        echo "# levels=$LEVELS"
        sed 's/^/# input=/' MANIFEST.sha256
        if [[ "$LEVELS" == "2 3 4 5 6 7 8 9" ]]; then
            echo "# structure=tools/check_level_chain.py --expect-top-sum $EXPECT_TOP_SUM (passed)"
        else
            echo "# structure=skipped (partial level set)"
        fi
    fi
    echo "# command=radio_cleanroom audit --threads $threads --progress $PROGRESS_SECONDS ${certs[*]}"
    echo "# started_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
    echo "# radio-cleanroom-provenance-v1 end"
} > verify.out
upload verify.out verify.out

( while sleep "$PROGRESS_SECONDS"; do upload verify.out verify.out; done ) &
heartbeat=$!

/usr/bin/time -v "$binary" audit --threads "$threads" --progress "$PROGRESS_SECONDS" "${certs[@]}" \
    >> verify.out 2>&1 \
    || { kill "$heartbeat" || true; upload verify.out verify.out; fail "verify"; }
kill "$heartbeat" 2>/dev/null || true
echo "# finished_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)" >> verify.out

# A complete run must account for every claim in the certificate of record; a --levels subset
# only has to be gap-free.
if [[ -n "$FLAT_CERT" && -n "$FLAT_EXPECT_CLAIMS" ]]; then
    grep -q "TOTAL verified $FLAT_EXPECT_CLAIMS, gaps 0," verify.out \
        || { upload verify.out verify.out; fail "flat totals"; }
elif [[ "$LEVELS" == "2 3 4 5 6 7 8 9" ]]; then
    grep -q "TOTAL verified $EXPECT_TOTAL_CLAIMS, gaps 0," verify.out \
        || { upload verify.out verify.out; fail "chain totals"; }
else
    grep -q ', gaps 0,' verify.out || { upload verify.out verify.out; fail "verify gaps"; }
fi
# Upload the plain log too, not just the compressed one: the status script reads the plain
# file, and leaving it at the last heartbeat made a finished run look stuck mid-audit.
upload verify.out verify.out
zstd -q -19 -f verify.out
upload verify.out.zst verify.out.zst
sha256sum verify.out verify.out.zst chain-structure.out > final.sha256
upload final.sha256 final.sha256
upload run.log run.log
stage DONE
FINISHED=1
shutdown -h +2
