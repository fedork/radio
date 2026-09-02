#!/usr/bin/env bash
# Launch one dedicated, on-demand c8a host to verify the full sa193 k=7 level with the
# cleanroom Rust checker (tools/cleanroom). Modeled on tools/run9_refute_ec2_launch.sh.
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
CERT_LOCAL=.artifacts/cert/sa193-k7.cert.zst
CERT_ZST_SHA256=5171ae78d2dd8a9ddb060f792e378a153d10df38c0b2e2bf0d2a158321459d7e

while (( $# )); do
    case "$1" in
        --dry-run) DRY=1; shift ;;
        --type) TYPE=$2; shift 2 ;;
        *) echo "usage: $0 [--dry-run] [--type INSTANCE_TYPE]" >&2; exit 64 ;;
    esac
done

aws_cmd=(aws-vault exec --server default -- aws)
commit=$(git rev-parse HEAD)
short=$(git rev-parse --short HEAD)
sha256_file() {
    if command -v sha256sum >/dev/null 2>&1; then sha256sum "$1" | awk '{print $1}'
    else shasum -a 256 "$1" | awk '{print $1}'
    fi
}

# The trusted checker and this pipeline must be exactly what is committed.
required=(tools/cleanroom_k7_ec2_remote.sh tools/testdata/radio_verify_v1.cert \
          tools/testdata/radio_refute_gap_v1.cert)
while IFS= read -r f; do required+=("$f"); done < <(git ls-files tools/cleanroom)
(( ${#required[@]} > 10 )) || { echo 'cleanroom sources missing from git' >&2; exit 65; }
for file in "${required[@]}"; do git ls-files --error-unmatch -- "$file" >/dev/null; done
if ! git diff --quiet HEAD -- "${required[@]}" || ! git diff --cached --quiet HEAD -- "${required[@]}"; then
    echo 'refusing to launch with dirty checker or pipeline inputs' >&2
    exit 65
fi

[[ -f "$CERT_LOCAL" ]] || { echo "missing $CERT_LOCAL (pull sa193-certificate-2026-08-19)" >&2; exit 66; }
actual=$(sha256_file "$CERT_LOCAL")
[[ "$actual" == "$CERT_ZST_SHA256" ]] || { echo 'k7 certificate hash mismatch' >&2; exit 66; }

active=$("${aws_cmd[@]}" ec2 describe-instances --region "$REGION" \
    --filters Name=tag:Purpose,Values=cleanroom-verify \
              Name=instance-state-name,Values=pending,running \
    --query 'length(Reservations[].Instances[])' --output text)
[[ "$active" == 0 ]] || { echo 'a cleanroom-verify instance is already active' >&2; exit 69; }

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
cert_key="$prefix/sa193-k7.cert.zst"

cat > "$userdata" <<EOF
#!/bin/bash
exec > >(tee -a /var/log/cleanroom-verify-bootstrap.log) 2>&1
set -euo pipefail
trap 'rc=\$?; aws s3 cp /var/log/cleanroom-verify-bootstrap.log s3://$BUCKET/$prefix/bootstrap-failure.log --no-progress || true; shutdown -h +2 >/dev/null 2>&1 || true; exit \$rc' EXIT
systemd-run --unit=cleanroom-verify-hard-stop --on-active=12h /sbin/shutdown -h now
dnf install -y gcc python3 zstd tar gzip time procps-ng
cd /root
aws s3 cp s3://$BUCKET/$source_key source.tar.zst --no-progress
printf '%s  source.tar.zst\n' '$bundle_sha' | sha256sum -c -
mkdir -p source
zstd -dc source.tar.zst | tar -xf - -C source
chmod +x source/tools/*.sh
trap - EXIT
exec source/tools/cleanroom_k7_ec2_remote.sh \
  --run-id '$RUN_ID' --work '/root/cleanroom-verify-$RUN_ID' \
  --prefix '$prefix' --source-commit '$commit' --source-sha256 '$bundle_sha'
EOF

if (( DRY )); then
    printf 'run_id=%s\ncommit=%s\nsource_sha256=%s\nsource_bytes=%s\ninstance_type=%s\n' \
        "$RUN_ID" "$commit" "$bundle_sha" "$(wc -c < "$bundle" | tr -d ' ')" "$TYPE"
    bash -n "$userdata"
    exit 0
fi

"${aws_cmd[@]}" s3 cp "$bundle" "s3://$BUCKET/$source_key" --no-progress
"${aws_cmd[@]}" s3 cp "$CERT_LOCAL" "s3://$BUCKET/$cert_key" --no-progress
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

printf 'launched instance=%s type=%s run_id=%s commit=%s\n' "$instance" "$TYPE" "$RUN_ID" "$commit"
printf 's3=s3://%s/%s/\n' "$BUCKET" "$prefix"
