#!/usr/bin/env bash
# Launch one dedicated on-demand host to load the full archived cache corpus and dump a snapshot.
#
# On-demand, not Spot: the load has no intra-run checkpoint, so an interruption throws away the
# whole thing. (Once a snapshot exists the calculus flips -- a serving oracle restores in
# milliseconds and journals what it learns, so Spot is fine for serving.)
#
# 32 GiB was chosen deliberately. Local measurement reached 2.75 GB resident at 800k facts on a
# superlinear curve, so 32 GiB is real headroom rather than a guess, and the remote guard stops the
# run at 28 GiB instead of letting the host swap.
set -euo pipefail
cd "$(dirname "$0")/.."

REGION=us-west-2
BUCKET=radio-sa193-393287594714
TYPE=r7iz.xlarge          # 4 vCPU, 32 GiB, high clock -- the load is single-threaded
SUBNET=subnet-087431aaade3d3e12
SECURITY_GROUP=sg-085e171bf6e1381b6
PROFILE=radio-sa193-ec2
DISK_GIB=300              # snapshot size is unknown; 2.88 GB at 800k facts and superlinear
RUN_ID=$(date -u +%Y%m%dT%H%M%SZ)
DRY=0

while (($#)); do
    case "$1" in
        --dry-run) DRY=1; shift ;;
        --type) TYPE=$2; shift 2 ;;
        --disk) DISK_GIB=$2; shift 2 ;;
        *) echo "usage: $0 [--dry-run] [--type T] [--disk GIB]" >&2; exit 64 ;;
    esac
done

aws_cmd=(aws-vault exec --server default -- aws)
commit=$(git rev-parse HEAD)
short=$(git rev-parse --short HEAD)
sha256_file() {
    if command -v sha256sum >/dev/null 2>&1; then sha256sum "$1" | awk '{print $1}'
    else shasum -a 256 "$1" | awk '{print $1}'; fi
}

required=(radio_oracle.c radiobase.c tools/build_radio.py tools/sort_cache.py \
          tools/oracle_prime_ec2_remote.sh tools/oracle_client.py)
for f in "${required[@]}"; do git ls-files --error-unmatch -- "$f" >/dev/null; done
if ! git diff --quiet HEAD -- "${required[@]}"; then
    echo 'refusing to launch with uncommitted pipeline inputs' >&2; exit 65
fi

active=$("${aws_cmd[@]}" ec2 describe-instances --region "$REGION" \
    --filters Name=tag:Purpose,Values=oracle-prime \
              Name=instance-state-name,Values=pending,running \
    --query 'length(Reservations[].Instances[])' --output text)
[[ "$active" == 0 ]] || { echo 'an oracle-prime instance is already active' >&2; exit 69; }

bundle=$(mktemp "/tmp/radio-oracle-${short}.XXXXXX")
userdata=$(mktemp "/tmp/radio-oracle-userdata.XXXXXX")
trap 'rm -f -- "$bundle" "$userdata"' EXIT
git archive --format=tar "$commit" | zstd -T0 -10 -f -o "$bundle"
bundle_sha=$(sha256_file "$bundle")
prefix="oracle-prime/$RUN_ID"
source_key="$prefix/source-radio-$short.tar.zst"

cat > "$userdata" <<EOF
#!/bin/bash
exec > >(tee -a /var/log/oracle-prime-bootstrap.log) 2>&1
set -euo pipefail
trap 'rc=\$?; aws s3 cp /var/log/oracle-prime-bootstrap.log s3://$BUCKET/$prefix/bootstrap-failure.log --no-progress || true; shutdown -h +2 >/dev/null 2>&1 || true; exit \$rc' EXIT
systemd-run --unit=oracle-prime-hard-stop --on-active=24h /sbin/shutdown -h now
dnf install -y clang python3 zstd tar gzip time procps-ng coreutils
cd /root
aws s3 cp s3://$BUCKET/$source_key source.tar.zst --no-progress
printf '%s  source.tar.zst\n' '$bundle_sha' | sha256sum -c -
mkdir -p source
zstd -dc source.tar.zst | tar -xf - -C source
chmod +x source/tools/*.sh
trap - EXIT
exec source/tools/oracle_prime_ec2_remote.sh \
  --run-id '$RUN_ID' --work '/root/oracle-prime-$RUN_ID' \
  --prefix '$prefix' --source-commit '$commit' --source-sha256 '$bundle_sha'
EOF

if ((DRY)); then
    printf 'run_id=%s\ncommit=%s\nsource_sha256=%s\ntype=%s\ndisk=%sGiB\nprefix=%s\n' \
        "$RUN_ID" "$commit" "$bundle_sha" "$TYPE" "$DISK_GIB" "$prefix"
    bash -n "$userdata" && bash -n tools/oracle_prime_ec2_remote.sh && echo 'scripts parse'
    exit 0
fi

"${aws_cmd[@]}" s3 cp "$bundle" "s3://$BUCKET/$source_key" --no-progress
ami=$("${aws_cmd[@]}" ssm get-parameter --region "$REGION" \
    --name /aws/service/ami-amazon-linux-latest/al2023-ami-kernel-default-x86_64 \
    --query Parameter.Value --output text)

instance=$("${aws_cmd[@]}" ec2 run-instances --region "$REGION" \
    --image-id "$ami" --instance-type "$TYPE" --subnet-id "$SUBNET" \
    --security-group-ids "$SECURITY_GROUP" --associate-public-ip-address \
    --iam-instance-profile "Name=$PROFILE" --instance-initiated-shutdown-behavior stop \
    --metadata-options 'HttpTokens=required,HttpPutResponseHopLimit=1,HttpEndpoint=enabled' \
    --block-device-mappings "DeviceName=/dev/xvda,Ebs={VolumeSize=$DISK_GIB,VolumeType=gp3,DeleteOnTermination=true,Encrypted=true}" \
    --user-data "file://$userdata" \
    --tag-specifications \
      "ResourceType=instance,Tags=[{Key=Project,Value=radio-sa193},{Key=Purpose,Value=oracle-prime},{Key=RunId,Value=$RUN_ID},{Key=Name,Value=oracle-prime-$RUN_ID}]" \
      "ResourceType=volume,Tags=[{Key=Project,Value=radio-sa193},{Key=Purpose,Value=oracle-prime},{Key=RunId,Value=$RUN_ID}]" \
    --query 'Instances[0].InstanceId' --output text)

printf 'launched instance=%s type=%s run_id=%s commit=%s\n' "$instance" "$TYPE" "$RUN_ID" "$commit"
printf 's3=s3://%s/%s/\nstatus: tools/oracle_prime_status.sh %s\n' "$BUCKET" "$prefix" "$RUN_ID"
