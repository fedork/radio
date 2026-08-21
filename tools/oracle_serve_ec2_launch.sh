#!/usr/bin/env bash
# Launch a persistent radio_oracle serving instance: cold start, grows its own cache from real
# queries, no hard-stop. Different in kind from oracle_prime_ec2_launch.sh (that one loads a
# fixed corpus, snapshots, and terminates) -- this one is meant to sit there across sessions.
#
# On-demand, not Spot: the whole point per Fedor (2026-08-21) is not depending on interruption --
# a laptop sleeping or losing connectivity. An accumulated-but-unsaved cache interval would be
# lost on a Spot reclaim just as it would on a crash, and reliability was the explicit ask.
#
# 2 vCPU / 16 GiB (r7i.large): the solver is single-threaded (1 vCPU), oracle_server.py's request
# loop is the other, and 16 GiB is headroom for a cache that may grow substantially once long
# states are being solved regularly -- watch STATUS's rss_gib and re-provision (stop, resize,
# start; EBS-backed so the disk and its snapshot survive) if it isn't enough.
set -euo pipefail
cd "$(dirname "$0")/.."

REGION=us-west-2
BUCKET=radio-sa193-393287594714
TYPE=r7i.large
SUBNET=subnet-087431aaade3d3e12
SECURITY_GROUP=sg-085e171bf6e1381b6
PROFILE=radio-sa193-ec2
DISK_GIB=50
PORT=7777
SNAPSHOT_EVERY=1800
RUN_ID=$(date -u +%Y%m%dT%H%M%SZ)
DRY=0

while (($#)); do
    case "$1" in
        --dry-run) DRY=1; shift ;;
        --type) TYPE=$2; shift 2 ;;
        --disk) DISK_GIB=$2; shift 2 ;;
        --port) PORT=$2; shift 2 ;;
        --snapshot-every) SNAPSHOT_EVERY=$2; shift 2 ;;
        *) echo "usage: $0 [--dry-run] [--type T] [--disk GIB] [--port N] [--snapshot-every S]" >&2
           exit 64 ;;
    esac
done

aws_cmd=(aws-vault exec --server default -- aws)
commit=$(git rev-parse HEAD)
short=$(git rev-parse --short HEAD)
sha256_file() {
    if command -v sha256sum >/dev/null 2>&1; then sha256sum "$1" | awk '{print $1}'
    else shasum -a 256 "$1" | awk '{print $1}'; fi
}

required=(radio_oracle.c radiobase.c tools/build_radio.py tools/oracle_server.py \
          tools/oracle_client.py tools/oracle_serve_ec2_remote.sh)
for f in "${required[@]}"; do git ls-files --error-unmatch -- "$f" >/dev/null; done
if ! git diff --quiet HEAD -- "${required[@]}"; then
    echo 'refusing to launch with uncommitted pipeline inputs' >&2; exit 65
fi

active=$("${aws_cmd[@]}" ec2 describe-instances --region "$REGION" \
    --filters Name=tag:Purpose,Values=oracle-serve \
              Name=instance-state-name,Values=pending,running \
    --query 'length(Reservations[].Instances[])' --output text)
[[ "$active" == 0 ]] || { echo 'an oracle-serve instance is already active' >&2; exit 69; }

bundle=$(mktemp "/tmp/radio-oracle-serve-${short}.XXXXXX")
userdata=$(mktemp "/tmp/radio-oracle-serve-userdata.XXXXXX")
trap 'rm -f -- "$bundle" "$userdata"' EXIT
git archive --format=tar "$commit" | zstd -T0 -10 -f -o "$bundle"
bundle_sha=$(sha256_file "$bundle")
prefix="oracle-serve/$RUN_ID"
source_key="$prefix/source-radio-$short.tar.zst"

cat > "$userdata" <<EOF
#!/bin/bash
exec > >(tee -a /var/log/oracle-serve-bootstrap.log) 2>&1
set -euo pipefail
trap 'rc=\$?; aws s3 cp /var/log/oracle-serve-bootstrap.log s3://$BUCKET/$prefix/bootstrap-failure.log --no-progress || true; exit \$rc' EXIT
dnf install -y clang python3 zstd tar gzip time procps-ng coreutils
cd /root
aws s3 cp s3://$BUCKET/$source_key source.tar.zst --no-progress
printf '%s  source.tar.zst\n' '$bundle_sha' | sha256sum -c -
mkdir -p source
zstd -dc source.tar.zst | tar -xf - -C source
chmod +x source/tools/*.sh
trap - EXIT
export PORT='$PORT' SNAPSHOT_EVERY='$SNAPSHOT_EVERY'
exec source/tools/oracle_serve_ec2_remote.sh \
  --run-id '$RUN_ID' --work '/root/oracle-serve-$RUN_ID' \
  --prefix '$prefix' --source-commit '$commit' --source-sha256 '$bundle_sha'
EOF

if ((DRY)); then
    printf 'run_id=%s\ncommit=%s\nsource_sha256=%s\ntype=%s\ndisk=%sGiB\nport=%s\nprefix=%s\n' \
        "$RUN_ID" "$commit" "$bundle_sha" "$TYPE" "$DISK_GIB" "$PORT" "$prefix"
    bash -n "$userdata" && bash -n tools/oracle_serve_ec2_remote.sh && echo 'scripts parse'
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
    --block-device-mappings "DeviceName=/dev/xvda,Ebs={VolumeSize=$DISK_GIB,VolumeType=gp3,DeleteOnTermination=false,Encrypted=true}" \
    --user-data "file://$userdata" \
    --tag-specifications \
      "ResourceType=instance,Tags=[{Key=Project,Value=radio-sa193},{Key=Purpose,Value=oracle-serve},{Key=RunId,Value=$RUN_ID},{Key=Name,Value=oracle-serve-$RUN_ID}]" \
      "ResourceType=volume,Tags=[{Key=Project,Value=radio-sa193},{Key=Purpose,Value=oracle-serve},{Key=RunId,Value=$RUN_ID}]" \
    --query 'Instances[0].InstanceId' --output text)

printf 'launched instance=%s type=%s run_id=%s commit=%s\n' "$instance" "$TYPE" "$RUN_ID" "$commit"
printf 's3=s3://%s/%s/\nstatus: tools/oracle_serve_status.sh %s\n' "$BUCKET" "$prefix" "$RUN_ID"
printf 'to reach the server:  aws-vault exec --server default -- aws ssm start-session \\\n'
printf '  --target %s --document-name AWS-StartPortForwardingSession \\\n' "$instance"
printf '  --parameters "{\\"portNumber\\":[\\"%s\\"],\\"localPortNumber\\":[\\"%s\\"]}"\n' "$PORT" "$PORT"
printf 'instance_id=%s (save this)\n' "$instance"
