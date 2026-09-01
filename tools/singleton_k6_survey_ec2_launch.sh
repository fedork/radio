#!/usr/bin/env bash
# Launch or resume the rolling K=6 survey in a one-instance, Spot-only Auto Scaling group.
# EC2 replacement handles Spot loss; systemd restarts transient solver failures from NEXT_RANK.
set -euo pipefail
cd "$(dirname "$0")/.."

REGION=us-west-2
ACCOUNT=393287594714
BUCKET=radio-sa193-393287594714
PREFIX=run10/k6-main-survey
ASG_NAME=radio-k6-main-survey
LT_NAME=radio-k6-main-survey
ROLE=radio-sa193-ec2
SECURITY_GROUP=sg-085e171bf6e1381b6
# Public subnets in all four Oregon availability zones. Capacity-optimized Spot chooses among these
# and the compatible 32-GiB instance pools below. Capacity rebalance stays off: two simultaneous
# workers must never race to advance one NEXT_RANK object.
SUBNETS=subnet-08d0f78dd11d2c5ec,subnet-087431aaade3d3e12,subnet-0f7c6e7503e8987d7,subnet-0e20e06928c093b05
INSTANCE_TYPES=(r7a.xlarge r7i.xlarge r7iz.xlarge r6a.xlarge r6i.xlarge)
END_RANK=9960648265
STAGE_SIZE=3000000
DRY=0

if (( $# > 1 )); then
    echo "usage: $0 [--dry-run]" >&2
    exit 2
fi
if (( $# == 1 )); then
    [[ "$1" == --dry-run ]] || { echo "usage: $0 [--dry-run]" >&2; exit 2; }
    DRY=1
fi

source_commit=$(git rev-parse HEAD)
if [[ -n "$(git status --porcelain --untracked-files=all)" && "$DRY" == 0 ]]; then
    echo "refusing to package a dirty worktree" >&2
    exit 65
fi
source_key="$PREFIX/source_${source_commit}.tgz"
checkpoint=$(aws-vault exec --server default -- aws s3 cp \
    "s3://$BUCKET/$PREFIX/NEXT_RANK" - --no-progress | tr -d '[:space:]')
[[ "$checkpoint" =~ ^[0-9]+$ ]] || { echo "invalid durable checkpoint: $checkpoint" >&2; exit 65; }
(( checkpoint >= 0 && checkpoint < END_RANK )) || {
    echo "checkpoint $checkpoint is outside 0..$END_RANK" >&2
    exit 65
}

active=$(aws-vault exec --server default -- aws ec2 describe-instances --region "$REGION" \
    --filters Name=tag:Purpose,Values=k6-main-survey \
              Name=instance-state-name,Values=pending,running \
    --query 'length(Reservations[].Instances[])' --output text)
asg_exists=$(aws-vault exec --server default -- aws autoscaling describe-auto-scaling-groups \
    --region "$REGION" --auto-scaling-group-names "$ASG_NAME" \
    --query 'length(AutoScalingGroups)' --output text)
asg_instances=0
if [[ "$asg_exists" == 1 ]]; then
    asg_instances=$(aws-vault exec --server default -- aws autoscaling describe-auto-scaling-groups \
        --region "$REGION" --auto-scaling-group-names "$ASG_NAME" \
        --query 'length(AutoScalingGroups[0].Instances)' --output text)
fi
if [[ "$active" != "$asg_instances" && "$DRY" == 0 ]]; then
    echo "refusing to launch beside $active active K6 instance(s), only $asg_instances owned by $ASG_NAME" >&2
    exit 69
fi

tmp_dir=$(mktemp -d /tmp/radio-k6-asg.XXXXXX)
trap 'rm -rf -- "$tmp_dir"' EXIT
user_data="$tmp_dir/user-data.sh"
cat > "$user_data" <<EOF
#!/bin/bash
exec > >(tee -a /var/log/radio-k6-survey-bootstrap.log) 2>&1
set -euxo pipefail
export AWS_DEFAULT_REGION=$REGION
BUCKET=$BUCKET
PREFIX=$PREFIX
ASG_NAME=$ASG_NAME
trap 'rc=\$?; aws s3 cp /var/log/radio-k6-survey-bootstrap.log s3://\$BUCKET/\$PREFIX/bootstrap/\$(hostname).log --no-progress || true; exit \$rc' EXIT

dnf install -y clang python3 tar gzip
mkdir -p /root/k6-survey
cd /root/k6-survey
aws s3 cp s3://\$BUCKET/$source_key /root/k6-source.tgz --no-progress
tar xzf /root/k6-source.tgz
chmod +x tools/*.sh

RADIO_SOURCE_COMMIT=$source_commit python3 tools/build_radio.py \
    -O3 -DMAX_K=6 -DMAX_N=793 -DMAX_PART_N=65 \
    -DRADIO_CACHE_DISABLED_LEVEL=6 radio_singleton_k6_survey.c -o k6-survey
python3 tools/check_provenance.py k6-survey.provenance

./k6-survey 6 14 55096 1 /root/k6-smoke.progress > /root/k6-smoke.log
grep -F 'INTEGRATED_SUMMARY queries=1 solvable=0 unsolvable=1 maybe=0' /root/k6-smoke.log
python3 tools/check_provenance.py /root/k6-smoke.log
aws s3 cp /root/k6-smoke.log s3://\$BUCKET/\$PREFIX/bootstrap/\$(hostname)-smoke.log --no-progress

cat > /usr/local/sbin/radio-k6-complete <<'SCRIPT'
#!/bin/bash
set -u
journalctl -u radio-k6-survey.service --no-pager > /root/k6-service-final.log
aws s3 cp /root/k6-service-final.log "s3://$BUCKET/$PREFIX/completed/service-final.log" --no-progress || true
aws s3 cp "s3://$BUCKET/$PREFIX/STATUS" /root/k6-complete-status --no-progress || exit 1
grep -q '^  state              COMPLETE$' /root/k6-complete-status || exit 1
for attempt in \$(seq 1 20); do
    if aws autoscaling update-auto-scaling-group --region "$REGION" \
        --auto-scaling-group-name "$ASG_NAME" --min-size 0 --max-size 1 \
        --desired-capacity 0; then
        exit 0
    fi
    sleep 30
done
exit 1
SCRIPT
cat > /usr/local/sbin/radio-k6-record-failure <<'SCRIPT'
#!/bin/bash
set -u
journalctl -u radio-k6-survey.service --no-pager > /root/k6-service-failed.log
aws s3 cp /root/k6-service-failed.log \
    "s3://$BUCKET/$PREFIX/failed/service-\$(hostname)-\$(date -u +%Y%m%dT%H%M%SZ).log" \
    --no-progress || true
SCRIPT
chmod +x /usr/local/sbin/radio-k6-complete /usr/local/sbin/radio-k6-record-failure

cat > /etc/systemd/system/radio-k6-survey.service <<UNIT
[Unit]
Description=rolling Spot K6 main-solver survey
After=network-online.target
Wants=network-online.target
OnSuccess=radio-k6-survey-complete.service
OnFailure=radio-k6-survey-failure.service
StartLimitIntervalSec=0

[Service]
Type=simple
WorkingDirectory=/root/k6-survey
Environment=AWS_DEFAULT_REGION=$REGION
Environment=RADIO_K6_SURVEY_CPU=0
Environment=RADIO_K6_SURVEY_VM_KIB=27262976
Environment=RADIO_K6_STATUS_INTERVAL=60
Environment=RADIO_K6_POLL_INTERVAL=10
MemoryMax=28G
ExecStart=/root/k6-survey/tools/singleton_k6_survey_remote.sh \$BUCKET \$PREFIX 0 $END_RANK $STAGE_SIZE
Restart=on-failure
RestartPreventExitStatus=2 65
RestartSec=30

[Install]
WantedBy=multi-user.target
UNIT
cat > /etc/systemd/system/radio-k6-survey-complete.service <<UNIT
[Unit]
Description=scale the completed K6 survey worker group to zero

[Service]
Type=oneshot
Environment=AWS_DEFAULT_REGION=$REGION
Environment=BUCKET=\$BUCKET
Environment=PREFIX=\$PREFIX
Environment=ASG_NAME=\$ASG_NAME
ExecStart=/usr/local/sbin/radio-k6-complete
UNIT
cat > /etc/systemd/system/radio-k6-survey-failure.service <<UNIT
[Unit]
Description=retain a permanent K6 survey failure for inspection

[Service]
Type=oneshot
Environment=AWS_DEFAULT_REGION=$REGION
Environment=BUCKET=\$BUCKET
Environment=PREFIX=\$PREFIX
ExecStart=/usr/local/sbin/radio-k6-record-failure
UNIT
systemctl daemon-reload
systemctl enable --now radio-k6-survey.service
EOF

printf 'durable checkpoint: %s\n' "$checkpoint"
printf 'source commit: %s\n' "$source_commit"
printf 'Spot pools:'
printf ' %s' "${INSTANCE_TYPES[@]}"
printf '\nsubnets: %s\n' "$SUBNETS"
if (( DRY )); then
    sed -n '1,300p' "$user_data"
    exit 0
fi

source_archive="$tmp_dir/source.tgz"
git archive --format=tar.gz -o "$source_archive" HEAD
source_sha=$(shasum -a 256 "$source_archive" | awk '{print $1}')
aws-vault exec --server default -- aws s3 cp "$source_archive" \
    "s3://$BUCKET/$source_key" --no-progress

policy_file="$tmp_dir/self-stop-policy.json"
cat > "$policy_file" <<EOF
{
  "Version": "2012-10-17",
  "Statement": [{
    "Effect": "Allow",
    "Action": "autoscaling:UpdateAutoScalingGroup",
    "Resource": "arn:aws:autoscaling:$REGION:$ACCOUNT:autoScalingGroup:*:autoScalingGroupName/$ASG_NAME"
  }]
}
EOF
aws-vault exec --server default -- aws iam put-role-policy \
    --role-name "$ROLE" --policy-name radio-k6-main-survey-self-stop \
    --policy-document "file://$policy_file"

ami=$(aws-vault exec --server default -- aws ssm get-parameter --region "$REGION" \
    --name /aws/service/ami-amazon-linux-latest/al2023-ami-kernel-default-x86_64 \
    --query Parameter.Value --output text)
encoded_user_data=$(base64 < "$user_data" | tr -d '\n')
launch_data="$tmp_dir/launch-data.json"
cat > "$launch_data" <<EOF
{
  "ImageId": "$ami",
  "IamInstanceProfile": {"Name": "$ROLE"},
  "SecurityGroupIds": ["$SECURITY_GROUP"],
  "UserData": "$encoded_user_data",
  "InstanceInitiatedShutdownBehavior": "terminate",
  "MetadataOptions": {"HttpTokens": "required", "HttpEndpoint": "enabled"},
  "BlockDeviceMappings": [{
    "DeviceName": "/dev/xvda",
    "Ebs": {"VolumeSize": 30, "VolumeType": "gp3", "DeleteOnTermination": true}
  }],
  "TagSpecifications": [
    {"ResourceType": "instance", "Tags": [
      {"Key": "Project", "Value": "radio-sa193"},
      {"Key": "Purpose", "Value": "k6-main-survey"},
      {"Key": "Name", "Value": "k6-main-survey-spot"},
      {"Key": "RunPrefix", "Value": "run10-k6-main-survey"}
    ]},
    {"ResourceType": "volume", "Tags": [
      {"Key": "Project", "Value": "radio-sa193"},
      {"Key": "Purpose", "Value": "k6-main-survey"},
      {"Key": "Name", "Value": "k6-main-survey-spot"}
    ]}
  ]
}
EOF

lt_id=$(aws-vault exec --server default -- aws ec2 describe-launch-templates --region "$REGION" \
    --launch-template-names "$LT_NAME" --query 'LaunchTemplates[0].LaunchTemplateId' \
    --output text 2>/dev/null || true)
if [[ -z "$lt_id" || "$lt_id" == None ]]; then
    read -r lt_id lt_version < <(aws-vault exec --server default -- aws ec2 create-launch-template \
        --region "$REGION" --launch-template-name "$LT_NAME" \
        --version-description "$source_commit" --launch-template-data "file://$launch_data" \
        --query 'LaunchTemplate.[LaunchTemplateId,LatestVersionNumber]' --output text)
else
    lt_version=$(aws-vault exec --server default -- aws ec2 create-launch-template-version \
        --region "$REGION" --launch-template-id "$lt_id" --source-version '$Latest' \
        --version-description "$source_commit" --launch-template-data "file://$launch_data" \
        --query 'LaunchTemplateVersion.VersionNumber' --output text)
    aws-vault exec --server default -- aws ec2 modify-launch-template --region "$REGION" \
        --launch-template-id "$lt_id" --default-version "$lt_version" >/dev/null
fi

overrides=
for instance_type in "${INSTANCE_TYPES[@]}"; do
    [[ -z "$overrides" ]] || overrides+=,
    overrides+="{\"InstanceType\":\"$instance_type\"}"
done
mixed_policy="$tmp_dir/mixed-policy.json"
cat > "$mixed_policy" <<EOF
{
  "LaunchTemplate": {
    "LaunchTemplateSpecification": {"LaunchTemplateId": "$lt_id", "Version": "$lt_version"},
    "Overrides": [$overrides]
  },
  "InstancesDistribution": {
    "OnDemandBaseCapacity": 0,
    "OnDemandPercentageAboveBaseCapacity": 0,
    "SpotAllocationStrategy": "capacity-optimized"
  }
}
EOF

if [[ "$asg_exists" == 0 ]]; then
    aws-vault exec --server default -- aws autoscaling create-auto-scaling-group \
        --region "$REGION" --auto-scaling-group-name "$ASG_NAME" \
        --mixed-instances-policy "file://$mixed_policy" \
        --min-size 0 --max-size 1 --desired-capacity 1 \
        --vpc-zone-identifier "$SUBNETS" --health-check-type EC2 \
        --health-check-grace-period 600 \
        --tags Key=Project,Value=radio-sa193,PropagateAtLaunch=true \
               Key=Purpose,Value=k6-main-survey,PropagateAtLaunch=true \
               Key=Name,Value=k6-main-survey-spot,PropagateAtLaunch=true
else
    aws-vault exec --server default -- aws autoscaling update-auto-scaling-group \
        --region "$REGION" --auto-scaling-group-name "$ASG_NAME" \
        --mixed-instances-policy "file://$mixed_policy" \
        --min-size 0 --max-size 1 --desired-capacity 1 \
        --vpc-zone-identifier "$SUBNETS" --health-check-type EC2 \
        --health-check-grace-period 600
fi

printf 'source: s3://%s/%s sha256=%s\n' "$BUCKET" "$source_key" "$source_sha"
printf 'launch template: %s version %s\n' "$lt_id" "$lt_version"
printf 'auto scaling group: %s desired=1 (Spot only, capacity-optimized)\n' "$ASG_NAME"
