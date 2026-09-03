#!/usr/bin/env bash
# Launch one isolated, on-demand, unbounded K=9 Pareto-frontier walk.
set -euo pipefail
cd "$(dirname "$0")/.."

region=us-west-2
bucket=radio-sa193-393287594714
topic=arn:aws:sns:us-west-2:393287594714:radio-sa193-progress
type=r7iz.xlarge
subnet=subnet-087431aaade3d3e12
security_group=sg-085e171bf6e1381b6
profile=radio-sa193-ec2
cache_key=run10/sa193.checkpoint
rss_gb=24
run_id=$(date -u +%Y%m%dT%H%M%SZ)
dry=0

while (( $# )); do
    case "$1" in
        --dry-run) dry=1; shift ;;
        --type) type=$2; shift 2 ;;
        --cache-key) cache_key=$2; shift 2 ;;
        --rss-gb) rss_gb=$2; shift 2 ;;
        *) echo "usage: $0 [--dry-run] [--type TYPE] [--cache-key KEY] [--rss-gb N]" >&2; exit 64 ;;
    esac
done
[[ "$cache_key" =~ ^[A-Za-z0-9][A-Za-z0-9._/-]*$ ]] || { echo 'invalid cache key' >&2; exit 64; }
[[ "$rss_gb" =~ ^[1-9][0-9]*$ ]] || { echo 'invalid RSS cap' >&2; exit 64; }

aws_cmd=(aws-vault exec --server default -- aws)
commit=$(git rev-parse HEAD)
short=$(git rev-parse --short HEAD)
required=(radio_pareto.c radiobase.c parse_out.sh tools/build_radio.py \
          tools/check_provenance.py tools/capped_run.sh tools/pareto_walk_regression.sh \
          tools/pareto9_aws_remote.sh)
for file in "${required[@]}"; do git ls-files --error-unmatch -- "$file" >/dev/null; done
if ! git diff --quiet HEAD -- "${required[@]}" || ! git diff --cached --quiet HEAD -- "${required[@]}"; then
    echo 'refusing to launch with dirty build or pipeline inputs' >&2
    exit 65
fi

active=$("${aws_cmd[@]}" ec2 describe-instances --region "$region" \
    --filters Name=tag:Purpose,Values=pareto9-frontier \
              Name=instance-state-name,Values=pending,running \
    --query 'length(Reservations[].Instances[])' --output text)
[[ "$active" == 0 ]] || { echo 'a pareto9-frontier instance is already active' >&2; exit 69; }

confirmed=$("${aws_cmd[@]}" sns list-subscriptions-by-topic --region "$region" \
    --topic-arn "$topic" --query 'length(Subscriptions[?Protocol==`email` && SubscriptionArn!=`PendingConfirmation`])' \
    --output text)
(( confirmed > 0 )) || { echo 'SNS topic has no confirmed email subscriber' >&2; exit 70; }

"${aws_cmd[@]}" s3api head-object --region "$region" --bucket "$bucket" --key "$cache_key" \
    --query '[ContentLength,LastModified,ETag]' --output text >/dev/null

offered=$("${aws_cmd[@]}" ec2 describe-instance-type-offerings --region "$region" \
    --location-type availability-zone --filters Name=location,Values=us-west-2b \
    Name=instance-type,Values="$type" --query 'length(InstanceTypeOfferings)' --output text)
[[ "$offered" == 1 ]] || { echo "$type is not offered in us-west-2b" >&2; exit 70; }

bundle=$(mktemp "/tmp/radio-pareto9-${short}.XXXXXX")
userdata=$(mktemp "/tmp/radio-pareto9-userdata.XXXXXX")
trap 'rm -f -- "$bundle" "$userdata"' EXIT
git archive --format=tar "$commit" | zstd -T0 -10 -f -o "$bundle"
bundle_sha=$(shasum -a 256 "$bundle" | awk '{print $1}')
prefix="pareto9-frontier/$run_id"
source_key="$prefix/source-radio-$short.tar.zst"

cat > "$userdata" <<EOF
#!/bin/bash
exec > >(tee -a /var/log/pareto9-bootstrap.log) 2>&1
set -euo pipefail
export AWS_DEFAULT_REGION=$region
trap 'rc=\$?; aws s3 cp /var/log/pareto9-bootstrap.log s3://$bucket/$prefix/bootstrap-failure.log --no-progress || true; shutdown -h +2 >/dev/null 2>&1 || true; exit \$rc' EXIT
dnf install -y clang python3 zstd tar gzip time procps-ng
cd /root
aws s3 cp s3://$bucket/$source_key source.tar.zst --no-progress
printf '%s  source.tar.zst\n' '$bundle_sha' | sha256sum -c -
mkdir -p source
zstd -dc source.tar.zst | tar -xf - -C source
chmod +x source/parse_out.sh source/tools/*.sh source/tools/*.py
trap - EXIT
exec source/tools/pareto9_aws_remote.sh \
  --run-id '$run_id' --prefix '$prefix' --source-commit '$commit' \
  --source-sha256 '$bundle_sha' --cache-key '$cache_key' --start-n 55 --start-m 55 \
  --rss-gb '$rss_gb' --interval 600 --heartbeat 21600
EOF

if (( dry )); then
    printf 'run_id=%s\ncommit=%s\nsource_sha256=%s\nsource_bytes=%s\ninstance_type=%s\n' \
        "$run_id" "$commit" "$bundle_sha" "$(wc -c < "$bundle" | tr -d ' ')" "$type"
    printf 'cache=s3://%s/%s\nprefix=s3://%s/%s/\nwall_time_limit=none\nrss_limit_gib=%s\n' \
        "$bucket" "$cache_key" "$bucket" "$prefix" "$rss_gb"
    bash -n "$userdata"
    exit 0
fi

"${aws_cmd[@]}" s3 cp "$bundle" "s3://$bucket/$source_key" --no-progress
ami=$("${aws_cmd[@]}" ssm get-parameter --region "$region" \
    --name /aws/service/ami-amazon-linux-latest/al2023-ami-kernel-default-x86_64 \
    --query Parameter.Value --output text)
instance=$("${aws_cmd[@]}" ec2 run-instances --region "$region" \
    --image-id "$ami" --instance-type "$type" --subnet-id "$subnet" \
    --security-group-ids "$security_group" --associate-public-ip-address \
    --iam-instance-profile "Name=$profile" --instance-initiated-shutdown-behavior stop \
    --metadata-options 'HttpTokens=required,HttpPutResponseHopLimit=1,HttpEndpoint=enabled' \
    --block-device-mappings 'DeviceName=/dev/xvda,Ebs={VolumeSize=50,VolumeType=gp3,DeleteOnTermination=false,Encrypted=true}' \
    --user-data "file://$userdata" \
    --tag-specifications \
      "ResourceType=instance,Tags=[{Key=Project,Value=radio-sa193},{Key=Purpose,Value=pareto9-frontier},{Key=RunId,Value=$run_id},{Key=RunPrefix,Value=$prefix},{Key=Name,Value=pareto9-frontier-$run_id}]" \
      "ResourceType=volume,Tags=[{Key=Project,Value=radio-sa193},{Key=Purpose,Value=pareto9-frontier},{Key=RunId,Value=$run_id}]" \
    --query 'Instances[0].InstanceId' --output text)

printf 'launched instance=%s type=%s run_id=%s commit=%s\n' "$instance" "$type" "$run_id" "$commit"
printf 'cache=s3://%s/%s\ns3=s3://%s/%s/\nstatus=tools/pareto9_status.sh %s\n' \
    "$bucket" "$cache_key" "$bucket" "$prefix" "$run_id"
printf 'wall_time_limit=none rss_limit_gib=%s notification_topic=%s\n' "$rss_gb" "$topic"
