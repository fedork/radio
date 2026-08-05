#!/usr/bin/env bash
# Launch the Sa(193) cold re-derivation on one EC2 instance. Serialized deliberately: a single
# process shares one cache across all sixteen top-level states, and that reuse is the reason the
# 2023 run's later states were affordable at all. Sixteen parallel cold jobs would each pay the
# shared low-k cost from scratch.
#
#   tools/sa193_launch.sh [--days 60] [--type r7iz.4xlarge] [--resume] [--dry-run]
#
# **Cold by default.** A checkpoint in S3 is NOT picked up unless --resume is passed. That default is
# the whole point of this run: a cold single session produces a log that is closed under SPLITS, and
# resuming makes it multi-segment, verifiable only if every segment survives. Warm-starting also
# hides work rather than doing it, so a resumed run cannot answer "was 2023 right" on its own.
# Reserve --resume for catastrophic failure, where the alternative is starting over.
#
# --days is a BACKSTOP, not a schedule. It defaults to 60 because the point of this run is a single
# cold session: an interruption makes the log multi-segment, which is only closed if every segment
# survives, so stopping and resuming is a cost to avoid rather than a checkpoint strategy. Shutdown
# behaviour is `stop`, not `terminate`, so if the backstop or the memory cap does trip, the EBS volume
# and the full uncompressed log survive and the instance can simply be started again.
#
# The memory cap is the guard that matters. 2023 reached ~90 GB and this run was at 5.28 GB after one
# hour, so exhaustion is the expected first failure; --rss-gb 110 stops it before the OOM killer,
# which preserves the checkpoint instead of losing it.
set -euo pipefail
cd "$(dirname "$0")/.."

DAYS=60 TYPE=r7iz.4xlarge DRY=0 RESUME=0
while (( $# )); do
    case "$1" in
        --days) DAYS="$2"; shift 2 ;;
        --type) TYPE="$2"; shift 2 ;;
        --dry-run) DRY=1; shift ;;
        --resume) RESUME=1; shift ;;
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
if [ "$RESUME" = "1" ]; then
    if aws s3 cp s3://$BUCKET/run/sa193.checkpoint /root/run/resume.txt 2>/dev/null; then
        CACHE=/root/run/resume.txt
        echo "RESUMING from checkpoint (\$(wc -l < \$CACHE) facts) - this run is NOT cold"
    else
        echo "--resume was requested but no checkpoint exists; running cold"
    fi
else
    echo "COLD run: any checkpoint in S3 is deliberately ignored"
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
  --instance-initiated-shutdown-behavior stop \
  --block-device-mappings 'DeviceName=/dev/xvda,Ebs={VolumeSize=200,VolumeType=gp3,DeleteOnTermination=true}' \
  --user-data "file:///tmp/sa193_userdata.sh" \
  --tag-specifications "ResourceType=instance,Tags=[{Key=Project,Value=radio-sa193},{Key=Name,Value=sa193-cold-$SHA}]" \
  --query 'Instances[0].[InstanceId,InstanceType,Placement.AvailabilityZone]' --output text
