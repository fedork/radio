#!/usr/bin/env bash
# Launch the Sa(193) cold re-derivation on one EC2 instance. Serialized deliberately: a single
# process shares one cache across all sixteen top-level states, and that reuse is the reason the
# 2023 run's later states were affordable at all. Sixteen parallel cold jobs would each pay the
# shared low-k cost from scratch.
#
#   tools/sa193_launch.sh [--days 7] [--type r7iz.4xlarge] [--dry-run]
#
# Cost is bounded by --days: the instance terminates itself when the cap trips, and the run resumes
# from its own checkpoint in S3. Seven-day increments keep the decision to continue with the human.
set -euo pipefail
cd "$(dirname "$0")/.."

DAYS=7 TYPE=r7iz.4xlarge DRY=0
while (( $# )); do
    case "$1" in
        --days) DAYS="$2"; shift 2 ;;
        --type) TYPE="$2"; shift 2 ;;
        --dry-run) DRY=1; shift ;;
        *) echo "unknown arg $1" >&2; exit 2 ;;
    esac
done

BUCKET=radio-sa193-393287594714
TOPIC=arn:aws:sns:us-west-2:393287594714:radio-sa193-progress
SECS=$(( DAYS * 86400 ))
SHA=$(git rev-parse --short HEAD)

tar czf /tmp/sa193_src.tgz radiobase.c radio_sa193.c parse_out.sh \
        tools/capped_run.sh tools/sa193_watchdog.sh
echo "source bundle: $(du -h /tmp/sa193_src.tgz | cut -f1), build $SHA"

cat > /tmp/sa193_userdata.sh <<EOF
#!/bin/bash
exec > >(tee -a /var/log/sa193.log) 2>&1
set -x
dnf install -y clang zstd tar gzip
cd /root
aws s3 cp s3://$BUCKET/src/sa193_src_$SHA.tgz src.tgz
mkdir -p run && tar xzf src.tgz -C run && cd run
chmod +x tools/*.sh parse_out.sh

# MAX_N is the coin count of the largest state the run will touch: 193. MAX_K is 10.
clang -O3 -DMAX_K=10 -DMAX_N=193 radio_sa193.c -o radio_sa193

# The control runs first inside radio_sa193 and aborts the whole run if Sa(192) does not come back
# solvable. Resume: if a checkpoint exists, pass it - warm-starting a negative from this run's OWN
# output is sound, unlike cache-2025:parsed_260.txt.
CACHE=""
if aws s3 cp s3://$BUCKET/run/sa193.checkpoint /root/run/resume.txt 2>/dev/null; then
    CACHE=/root/run/resume.txt
    echo "resuming from checkpoint (\$(wc -l < \$CACHE) facts)"
fi

( tools/capped_run.sh --seconds $SECS --rss-gb 110 --label sa193 -- \\
      ./radio_sa193 \$CACHE > /root/run/out_sa193.txt 2>/root/run/sa193.err ) &
SOLVER_WRAPPER=\$!
sleep 20
SOLVER=\$(pgrep -x radio_sa193 | head -1)

tools/sa193_watchdog.sh --log /root/run/out_sa193.txt --pid "\$SOLVER" \\
    --bucket $BUCKET --topic $TOPIC --interval 600 --heartbeat 21600

wait \$SOLVER_WRAPPER || true
aws s3 cp /root/run/sa193.err s3://$BUCKET/run/sa193.err || true
aws s3 cp /var/log/sa193.log s3://$BUCKET/run/instance.log || true
shutdown -h now
EOF

if (( DRY )); then echo "--- user-data ---"; cat /tmp/sa193_userdata.sh; exit 0; fi

aws s3 cp /tmp/sa193_src.tgz "s3://$BUCKET/src/sa193_src_$SHA.tgz"
AMI=$(aws ssm get-parameter \
    --name /aws/service/ami-amazon-linux-latest/al2023-ami-kernel-default-x86_64 \
    --query Parameter.Value --output text)

aws ec2 run-instances \
  --image-id "$AMI" --instance-type "$TYPE" \
  --iam-instance-profile Name=radio-sa193-ec2 \
  --instance-initiated-shutdown-behavior terminate \
  --block-device-mappings 'DeviceName=/dev/xvda,Ebs={VolumeSize=200,VolumeType=gp3,DeleteOnTermination=true}' \
  --user-data "file:///tmp/sa193_userdata.sh" \
  --tag-specifications "ResourceType=instance,Tags=[{Key=Project,Value=radio-sa193},{Key=Name,Value=sa193-cold-$SHA}]" \
  --query 'Instances[0].[InstanceId,InstanceType,Placement.AvailabilityZone]' --output text
