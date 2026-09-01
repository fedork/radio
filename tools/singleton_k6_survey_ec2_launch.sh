#!/usr/bin/env bash
# Resume the staged K=6 production-solver census on a dedicated Spot instance.
# Completed rank stages live in S3; an interrupted instance restarts from NEXT_RANK and repeats at
# most the currently active three-million-rank stage.
set -euo pipefail
cd "$(dirname "$0")/.."

REGION=us-west-2
BUCKET=radio-sa193-393287594714
PREFIX=run10/k6-main-survey
TYPE=r7a.xlarge
# us-west-2d currently has the strongest placement score among the 32-GiB pools measured for this
# restartable job. The instance type remains overrideable, but a type override may also warrant a
# different subnet after checking current Spot placement scores.
SUBNET=subnet-0e20e06928c093b05
SECURITY_GROUP=sg-085e171bf6e1381b6
SOURCE_KEY=$PREFIX/source_d295b17.tgz
RUNNER_KEY=$PREFIX/runner_9684bdb.sh
SOURCE_COMMIT=d295b174984a0a91256622dc39c34c5ae17e1632
END_RANK=9960648265
STAGE_SIZE=3000000
DRY=0

while (( $# )); do
    case "$1" in
        --type) TYPE=$2; shift 2 ;;
        --dry-run) DRY=1; shift ;;
        *) echo "usage: $0 [--type INSTANCE_TYPE] [--dry-run]" >&2; exit 2 ;;
    esac
done

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
[[ "$active" == 0 ]] || { echo "refusing to launch beside $active active K6 survey instance(s)" >&2; exit 69; }

user_data=$(mktemp /tmp/radio-k6-survey-userdata.XXXXXX)
cat > "$user_data" <<EOF
#!/bin/bash
exec > >(tee -a /var/log/radio-k6-survey-bootstrap.log) 2>&1
set -euxo pipefail
export AWS_DEFAULT_REGION=$REGION
BUCKET=$BUCKET
PREFIX=$PREFIX
trap 'rc=\$?; aws s3 cp /var/log/radio-k6-survey-bootstrap.log s3://\$BUCKET/\$PREFIX/migration/bootstrap.log --no-progress || true; exit \$rc' EXIT

dnf install -y clang python3 tar gzip
mkdir -p /root/k6-survey
cd /root/k6-survey
aws s3 cp s3://\$BUCKET/$SOURCE_KEY /root/k6-source.tgz --no-progress
tar xzf /root/k6-source.tgz
aws s3 cp s3://\$BUCKET/$RUNNER_KEY tools/singleton_k6_survey_remote.sh --no-progress
chmod +x tools/*.sh

RADIO_SOURCE_COMMIT=$SOURCE_COMMIT python3 tools/build_radio.py \
    -O3 -DMAX_K=6 -DMAX_N=793 -DMAX_PART_N=65 \
    -DRADIO_CACHE_DISABLED_LEVEL=6 radio_singleton_k6_survey.c -o k6-survey
python3 tools/check_provenance.py k6-survey.provenance

./k6-survey 6 14 55096 1 /root/k6-smoke.progress > /root/k6-smoke.log
grep -F 'INTEGRATED_SUMMARY queries=1 solvable=0 unsolvable=1 maybe=0' /root/k6-smoke.log
python3 tools/check_provenance.py /root/k6-smoke.log
aws s3 cp /root/k6-smoke.log s3://\$BUCKET/\$PREFIX/migration/smoke.log --no-progress

cat > /etc/systemd/system/radio-k6-survey.service <<UNIT
[Unit]
Description=dedicated Spot K6 main-solver survey
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
WorkingDirectory=/root/k6-survey
Environment=AWS_DEFAULT_REGION=$REGION
Environment=RADIO_K6_SURVEY_CPU=0
Environment=RADIO_K6_SURVEY_VM_KIB=16777216
Environment=RADIO_K6_STATUS_INTERVAL=60
MemoryMax=20G
ExecStart=/root/k6-survey/tools/singleton_k6_survey_remote.sh \$BUCKET \$PREFIX 0 $END_RANK $STAGE_SIZE
UNIT
systemctl daemon-reload
systemctl start radio-k6-survey.service

while systemctl is-active --quiet radio-k6-survey.service; do sleep 60; done
systemctl --no-pager status radio-k6-survey.service || true
journalctl -u radio-k6-survey.service --no-pager > /root/k6-service.log
aws s3 cp /root/k6-service.log s3://\$BUCKET/\$PREFIX/migration/service-final.log --no-progress || true
shutdown -h now
EOF

if (( DRY )); then
    printf 'durable checkpoint: %s\ninstance type: %s (Spot)\n' "$checkpoint" "$TYPE"
    sed -n '1,260p' "$user_data"
    exit 0
fi

ami=$(aws-vault exec --server default -- aws ssm get-parameter --region "$REGION" \
    --name /aws/service/ami-amazon-linux-latest/al2023-ami-kernel-default-x86_64 \
    --query Parameter.Value --output text)

aws-vault exec --server default -- aws ec2 run-instances --region "$REGION" \
    --image-id "$ami" --instance-type "$TYPE" \
    --subnet-id "$SUBNET" --security-group-ids "$SECURITY_GROUP" \
    --iam-instance-profile Name=radio-sa193-ec2 \
    --instance-market-options 'MarketType=spot,SpotOptions={SpotInstanceType=one-time,InstanceInterruptionBehavior=terminate}' \
    --instance-initiated-shutdown-behavior terminate \
    --block-device-mappings 'DeviceName=/dev/xvda,Ebs={VolumeSize=30,VolumeType=gp3,DeleteOnTermination=true}' \
    --user-data "file://$user_data" \
    --tag-specifications \
      'ResourceType=instance,Tags=[{Key=Project,Value=radio-sa193},{Key=Purpose,Value=k6-main-survey},{Key=Name,Value=k6-main-survey-spot},{Key=RunPrefix,Value=run10-k6-main-survey}]' \
      'ResourceType=volume,Tags=[{Key=Project,Value=radio-sa193},{Key=Purpose,Value=k6-main-survey},{Key=Name,Value=k6-main-survey-spot}]' \
    --query 'Instances[0].[InstanceId,InstanceType,Placement.AvailabilityZone,InstanceLifecycle]' \
    --output text
