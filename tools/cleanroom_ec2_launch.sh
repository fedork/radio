#!/usr/bin/env bash
# Launch one dedicated, on-demand c8a host to verify the sa193 certificate of record with the
# cleanroom Rust checker (tools/cleanroom). Modeled on tools/run9_refute_ec2_launch.sh.
#
# Default is the COMPLETE chain, all eight levels: structural closure via
# tools/check_level_chain.py plus a semantic audit of every level. k=7 is ~99% of the cost, so
# the other seven levels are nearly free and a partial attestation is not worth the ambiguity.
# --levels restricts the set for a cheap probe (e.g. --levels "2 3 4 5 6 8 9").
set -euo pipefail
cd "$(dirname "$0")/.."

REGION=us-west-2
BUCKET=radio-sa193-393287594714
TYPE=c8a.8xlarge
SUBNET=subnet-087431aaade3d3e12
SECURITY_GROUP=sg-085e171bf6e1381b6
PROFILE=radio-sa193-ec2
RUN_ID=$(date -u +%Y%m%dT%H%M%SZ)
DRY=0
CERT_DIR=.artifacts/cert
LEVELS="2 3 4 5 6 7 8 9"
# --flat-cert switches to a single self-contained radio-negative-certificate-v1.zst, which
# carries every level in one file. Used for the K=8 frontier certificate; the sa193 level-chain
# path is the default and is unchanged.
FLAT_CERT=
FLAT_EXPECT_CLAIMS=
HARD_STOP=12h

while (( $# )); do
    case "$1" in
        --dry-run) DRY=1; shift ;;
        --type) TYPE=$2; shift 2 ;;
        --levels) LEVELS=$2; shift 2 ;;
        --certs) CERT_DIR=$2; shift 2 ;;
        --flat-cert) FLAT_CERT=$2; shift 2 ;;
        --expect-claims) FLAT_EXPECT_CLAIMS=$2; shift 2 ;;
        --hard-stop) HARD_STOP=$2; shift 2 ;;
        *) echo "usage: $0 [--dry-run] [--type TYPE] [--levels \"2 3 ...\"] [--certs DIR]" \
                "[--flat-cert FILE.cert.zst] [--expect-claims N] [--hard-stop 12h]" >&2; exit 64 ;;
    esac
done

if [[ -n "$FLAT_CERT" ]]; then
    [[ -f "$FLAT_CERT" ]] || { echo "missing $FLAT_CERT" >&2; exit 66; }
    [[ "$(basename "$FLAT_CERT")" =~ ^[A-Za-z0-9._-]+\.cert\.zst$ ]] \
        || { echo 'flat certificate must be named *.cert.zst' >&2; exit 64; }
    [[ -z "$FLAT_EXPECT_CLAIMS" || "$FLAT_EXPECT_CLAIMS" =~ ^[0-9]+$ ]] \
        || { echo 'bad --expect-claims' >&2; exit 64; }
fi
[[ "$HARD_STOP" =~ ^[0-9]+[mh]$ ]] || { echo 'bad --hard-stop' >&2; exit 64; }

aws_cmd=(aws-vault exec --server default -- aws)
commit=$(git rev-parse HEAD)
short=$(git rev-parse --short HEAD)
sha256_file() {
    if command -v sha256sum >/dev/null 2>&1; then sha256sum "$1" | awk '{print $1}'
    else shasum -a 256 "$1" | awk '{print $1}'
    fi
}

# The trusted checker and this pipeline must be exactly what is committed.
required=(tools/cleanroom_ec2_remote.sh tools/testdata/radio_verify_v1.cert \
          tools/testdata/radio_refute_gap_v1.cert)
while IFS= read -r f; do required+=("$f"); done < <(git ls-files tools/cleanroom)
(( ${#required[@]} > 10 )) || { echo 'cleanroom sources missing from git' >&2; exit 65; }
for file in "${required[@]}"; do git ls-files --error-unmatch -- "$file" >/dev/null; done
if ! git diff --quiet HEAD -- "${required[@]}" || ! git diff --cached --quiet HEAD -- "${required[@]}"; then
    echo 'refusing to launch with dirty checker or pipeline inputs' >&2
    exit 65
fi

# Every level must be present and match the released manifest before anything is launched.
if [[ -n "$FLAT_CERT" ]]; then
    flat_base=$(basename "$FLAT_CERT")
    flat_sha=$(sha256_file "$FLAT_CERT")
    manifest=
    LEVELS=
else
    manifest=$CERT_DIR/MANIFEST.sha256
    [[ -f "$manifest" ]] || { echo "missing $manifest (pull sa193-certificate-2026-08-19)" >&2; exit 66; }
    for k in $LEVELS; do
        f=$CERT_DIR/sa193-k$k.cert.zst
        [[ -f "$f" ]] || { echo "missing $f" >&2; exit 66; }
        want=$(awk -v n="sa193-k$k.cert.zst" '$2 == n {print $1}' "$manifest")
        [[ -n "$want" ]] || { echo "no manifest entry for sa193-k$k.cert.zst" >&2; exit 66; }
        [[ "$(sha256_file "$f")" == "$want" ]] || { echo "sa193-k$k.cert.zst hash mismatch" >&2; exit 66; }
    done
fi

# Not in dry-run mode: checking the plan while another run is in flight is a normal thing to do.
if (( ! DRY )); then
    active=$("${aws_cmd[@]}" ec2 describe-instances --region "$REGION" \
        --filters Name=tag:Purpose,Values=cleanroom-verify \
                  Name=instance-state-name,Values=pending,running \
        --query 'length(Reservations[].Instances[])' --output text)
    [[ "$active" == 0 ]] || { echo 'a cleanroom-verify instance is already active' >&2; exit 69; }
fi

offered=$("${aws_cmd[@]}" ec2 describe-instance-type-offerings --region "$REGION" \
    --location-type availability-zone --filters Name=location,Values=us-west-2b \
    Name=instance-type,Values="$TYPE" --query 'length(InstanceTypeOfferings)' --output text)
[[ "$offered" == 1 ]] || { echo "$TYPE is not offered in us-west-2b" >&2; exit 70; }

bundle=$(mktemp "/tmp/radio-cleanroom-${short}.XXXXXX")
userdata=$(mktemp "/tmp/radio-cleanroom-userdata.XXXXXX")
trap 'rm -f -- "$bundle" "$userdata"' EXIT
git archive --format=tar "$commit" | zstd -T0 -10 -f -o "$bundle"
bundle_sha=$(sha256_file "$bundle")
prefix="cleanroom-verify/$RUN_ID"
source_key="$prefix/source-radio-$short.tar.zst"

if [[ -n "$FLAT_CERT" ]]; then
    remote_mode_args="--flat-cert '$flat_base' --flat-sha256 '$flat_sha'"
    [[ -n "$FLAT_EXPECT_CLAIMS" ]] \
        && remote_mode_args="$remote_mode_args --flat-expect-claims '$FLAT_EXPECT_CLAIMS'"
else
    remote_mode_args="--levels '$LEVELS'"
fi

cat > "$userdata" <<EOF
#!/bin/bash
exec > >(tee -a /var/log/cleanroom-verify-bootstrap.log) 2>&1
set -euo pipefail
trap 'rc=\$?; aws s3 cp /var/log/cleanroom-verify-bootstrap.log s3://$BUCKET/$prefix/bootstrap-failure.log --no-progress || true; shutdown -h +2 >/dev/null 2>&1 || true; exit \$rc' EXIT
systemd-run --unit=cleanroom-verify-hard-stop --on-active=$HARD_STOP /sbin/shutdown -h now
dnf install -y gcc python3 zstd tar gzip time procps-ng
cd /root
aws s3 cp s3://$BUCKET/$source_key source.tar.zst --no-progress
printf '%s  source.tar.zst\n' '$bundle_sha' | sha256sum -c -
mkdir -p source
zstd -dc source.tar.zst | tar -xf - -C source
chmod +x source/tools/*.sh
trap - EXIT
exec source/tools/cleanroom_ec2_remote.sh \
  --run-id '$RUN_ID' --work '/root/cleanroom-verify-$RUN_ID' \
  --prefix '$prefix' --source-commit '$commit' --source-sha256 '$bundle_sha' \
  $remote_mode_args
EOF

if [[ -n "$FLAT_CERT" ]]; then
    what="flat=$flat_base sha256=$flat_sha bytes=$(wc -c < "$FLAT_CERT" | tr -d ' ')"
    what="$what expect_claims=${FLAT_EXPECT_CLAIMS:-unset}"
else
    what="levels=$LEVELS"
fi

if (( DRY )); then
    printf 'run_id=%s\ncommit=%s\nsource_sha256=%s\nsource_bytes=%s\ninstance_type=%s\nhard_stop=%s\n%s\n' \
        "$RUN_ID" "$commit" "$bundle_sha" "$(wc -c < "$bundle" | tr -d ' ')" "$TYPE" \
        "$HARD_STOP" "$what"
    bash -n "$userdata"
    exit 0
fi

"${aws_cmd[@]}" s3 cp "$bundle" "s3://$BUCKET/$source_key" --no-progress
if [[ -n "$FLAT_CERT" ]]; then
    "${aws_cmd[@]}" s3 cp "$FLAT_CERT" "s3://$BUCKET/$prefix/$flat_base" --no-progress
else
    for k in $LEVELS; do
        "${aws_cmd[@]}" s3 cp "$CERT_DIR/sa193-k$k.cert.zst" \
            "s3://$BUCKET/$prefix/sa193-k$k.cert.zst" --no-progress
    done
    "${aws_cmd[@]}" s3 cp "$manifest" "s3://$BUCKET/$prefix/MANIFEST.sha256" --no-progress
fi
ami=$("${aws_cmd[@]}" ssm get-parameter --region "$REGION" \
    --name /aws/service/ami-amazon-linux-latest/al2023-ami-kernel-default-x86_64 \
    --query Parameter.Value --output text)

instance=$("${aws_cmd[@]}" ec2 run-instances --region "$REGION" \
    --image-id "$ami" --instance-type "$TYPE" --subnet-id "$SUBNET" \
    --security-group-ids "$SECURITY_GROUP" --associate-public-ip-address \
    --iam-instance-profile "Name=$PROFILE" --instance-initiated-shutdown-behavior stop \
    --metadata-options 'HttpTokens=required,HttpPutResponseHopLimit=1,HttpEndpoint=enabled' \
    --block-device-mappings 'DeviceName=/dev/xvda,Ebs={VolumeSize=30,VolumeType=gp3,DeleteOnTermination=true,Encrypted=true}' \
    --user-data "file://$userdata" \
    --tag-specifications \
      "ResourceType=instance,Tags=[{Key=Project,Value=radio-sa193},{Key=Purpose,Value=cleanroom-verify},{Key=RunId,Value=$RUN_ID},{Key=Name,Value=cleanroom-verify-$RUN_ID}]" \
      "ResourceType=volume,Tags=[{Key=Project,Value=radio-sa193},{Key=Purpose,Value=cleanroom-verify},{Key=RunId,Value=$RUN_ID}]" \
    --query 'Instances[0].InstanceId' --output text)

printf 'launched instance=%s type=%s run_id=%s commit=%s hard_stop=%s %s\n' \
    "$instance" "$TYPE" "$RUN_ID" "$commit" "$HARD_STOP" "$what"
printf 's3=s3://%s/%s/\n' "$BUCKET" "$prefix"
