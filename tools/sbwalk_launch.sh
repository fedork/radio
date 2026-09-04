#!/usr/bin/env bash
# Launch the k=10 Sb vertical scan on one EC2 instance, as N concurrent walkers.
#
#   tools/sbwalk_launch.sh --cache-key sbwalk/<run>/input/combined.cache.zst \
#       [--k 10] [--n1 192] [--m-list 136,130,120,110,100] [--m-end 160] \
#       [--days 2] [--type r7iz.2xlarge] [--rss-gb 10] [--dry-run]
#
# **Why several walkers instead of one ascending walk.** The walk-up design banks on memo reuse:
# on 2026-08-03 Sa(112)@9 cost an eighth of Sa(111)@9 purely because the k<=8 memo was already
# populated. That effect is real but it needs a *completed* cell to seed it, and on 2026-09-04 no
# cell at n1=192 completed locally at all - m=20 ran 25 minutes, m=100 15% of its splits in 20
# minutes, m=137 42% in 45 minutes, none reaching a verdict. Until one cell lands there is nothing
# to walk up from, so this launches independent shots at several m and lets whichever lands first
# seed the next run's cache. Each walker still ascends on success, so a lucky low shot turns into
# the walk that was wanted.
#
# Every query runs with NO_DEADLINE: TRUE and FALSE are both real, never budget artifacts. The
# corresponding risk is that the first unsolvable cell runs until the backstop, which is why
# --days is short by default and the answer of record is the last printed WALK line.
#
# **The final upload does not depend on a watchdog.** On 2026-09-04 run10 printed
# `Sa(193) = UNSOLVABLE` and its host was powered off 63 seconds later by a leftover 60-second
# shutdown poller, 32 seconds before the 600-second watchdog cycle that would have archived the
# log; the verdict survived only because the volume did. Here the shell that waits on the walkers
# uploads the logs itself, before anything can shut the host down, and no separate poller is
# installed. See evidence/run10_completion_2026-09-04.md.
set -euo pipefail
cd "$(dirname "$0")/.."

K=10 N1=192 M_LIST=136,130,120,110,100 M_END=160
DAYS=2 TYPE=r7iz.2xlarge RSS_GB=10 CACHE_KEY= DRY=0
while (( $# )); do
    case "$1" in
        --k) K="$2"; shift 2 ;;
        --n1) N1="$2"; shift 2 ;;
        --m-list) M_LIST="$2"; shift 2 ;;
        --m-end) M_END="$2"; shift 2 ;;
        --days) DAYS="$2"; shift 2 ;;
        --type) TYPE="$2"; shift 2 ;;
        --rss-gb) RSS_GB="$2"; shift 2 ;;
        --cache-key) CACHE_KEY="$2"; shift 2 ;;
        --dry-run) DRY=1; shift ;;
        *) echo "unknown arg $1" >&2; exit 2 ;;
    esac
done
[[ "$K" =~ ^[0-9]+$ && "$N1" =~ ^[0-9]+$ && "$M_END" =~ ^[0-9]+$ ]] || { echo "k/n1/m-end must be integers" >&2; exit 64; }
[[ "$M_LIST" =~ ^[0-9]+(,[0-9]+)*$ ]] || { echo "--m-list must be comma-separated integers" >&2; exit 64; }
[[ "$DAYS" =~ ^[1-9][0-9]*$ && "$RSS_GB" =~ ^[1-9][0-9]*$ ]] || { echo "--days/--rss-gb must be positive" >&2; exit 64; }
[[ -n "$CACHE_KEY" ]] || { echo "--cache-key is required (s3 key of the zstd cache)" >&2; exit 64; }

BUCKET=radio-sa193-393287594714
REGION=us-west-2
RUN_ID=$(date -u +%Y%m%dT%H%M%SZ)
PREFIX="sbwalk/$RUN_ID"
SECS=$(( DAYS * 86400 ))
MAX_N=$(( N1 + M_END ))
FULL_SHA=$(git rev-parse HEAD)
SHA=$(git rev-parse --short HEAD)

BUNDLE_FILES=(radiobase.c radio_sb_walk.c parse_out.sh tools/build_radio.py \
              tools/check_provenance.py tools/capped_run.sh)
for file in "${BUNDLE_FILES[@]}"; do
    git ls-files --error-unmatch -- "$file" >/dev/null 2>&1 || {
        echo "refusing to label an untracked source as commit $FULL_SHA: $file" >&2; exit 65; }
done
if ! git diff --quiet HEAD -- "${BUNDLE_FILES[@]}" || \
        ! git diff --cached --quiet HEAD -- "${BUNDLE_FILES[@]}"; then
    echo "refusing a commit-labelled source bundle with dirty build inputs" >&2; exit 65
fi
tar czf /tmp/sbwalk_src.tgz "${BUNDLE_FILES[@]}"

cat > /tmp/sbwalk_userdata.sh <<EOF
#!/bin/bash
exec > >(tee -a /var/log/sbwalk.log) 2>&1
set -x
export AWS_DEFAULT_REGION=$REGION
dnf install -y clang python3 zstd tar gzip
cd /root
aws s3 cp s3://$BUCKET/src/sbwalk_src_$SHA.tgz src.tgz
mkdir -p run && tar xzf src.tgz -C run && cd run
chmod +x tools/*.sh parse_out.sh

aws s3 cp s3://$BUCKET/$CACHE_KEY input.cache.zst
zstd -q -d input.cache.zst -o input.cache
wc -l input.cache

# MAX_N is n1 + m_end: every state this run touches has that mass or less. One first-test
# component can span the whole width, so MAX_PART_N cannot be reduced independently.
RADIO_SOURCE_COMMIT=$FULL_SHA python3 tools/build_radio.py \\
    -O3 -DMAX_K=$K -DMAX_N=$MAX_N -DMAX_PART_N=$MAX_N radio_sb_walk.c -o radio_sb_walk
python3 tools/check_provenance.py radio_sb_walk.provenance

upload_all() {
    for f in /root/run/walk_m*.txt; do
        [ -s "\$f" ] || continue
        b=\$(basename "\$f" .txt)
        zstd -q -3 -c "\$f" | aws s3 cp - "s3://$BUCKET/$PREFIX/logs/\$b.txt.zst" >/dev/null 2>&1 || true
    done
    { echo "run_id  $RUN_ID"
      echo "written \$(date -u +%FT%TZ)"
      echo "walkers $M_LIST  (k=$K n1=$N1 m_end=$M_END)"
      echo "alive   \$(pgrep -c -x radio_sb_walk || echo 0)"
      echo "uptime  \$(uptime)"
      echo "rss_kb  \$(ps -o rss= -C radio_sb_walk 2>/dev/null | tr '\\n' ' ')"
      echo
      echo "=== every WALK verdict so far ==="
      grep -h '^WALK' /root/run/walk_m*.txt 2>/dev/null || echo "(none yet)"
      echo
      echo "=== newest root progress per walker ==="
      for f in /root/run/walk_m*.txt; do
          echo "--- \$(basename \$f)"
          grep '^still solving in $K' "\$f" 2>/dev/null | tail -1 | cut -c1-240
      done
    } | aws s3 cp - "s3://$BUCKET/$PREFIX/STATUS" --content-type text/plain >/dev/null 2>&1 || true
}

WALKERS=()
for m in \$(echo "$M_LIST" | tr ',' ' '); do
    ( RADIO_RUN_CONTEXT="sbwalk m_start=\$m" \\
      tools/capped_run.sh --seconds $SECS --rss-gb $RSS_GB --label "walk_m\$m" -- \\
          ./radio_sb_walk input.cache $K $N1 "\$m" $M_END \\
          > "/root/run/walk_m\$m.txt" 2>"/root/run/walk_m\$m.err" ) &
    WALKERS+=(\$!)
done

# Ten-minute durable snapshot. This is a convenience, NOT the archival path.
# Unconditional on purpose: guarding the loop with \`while pgrep -x radio_sb_walk\` raced the
# walkers' own startup on 2026-09-04 - capped_run had not yet exec'd a solver when the first pgrep
# ran, so the loop exited immediately and the run went 20 minutes with no durable snapshot. The
# explicit kill after the waits below is what stops it.
( while true; do sleep 600; upload_all; done ) &
HEARTBEAT=\$!

# Wait on the walkers by explicit pid. Deriving the list from \`jobs -p\` would also match the
# heartbeat and is order-dependent; the archival upload below must not be reached early.
for pid in "\${WALKERS[@]}"; do wait "\$pid" || true; done
kill \$HEARTBEAT 2>/dev/null || true

# The archival path: this shell owns it, and it runs before anything can power the host off.
upload_all
for f in /root/run/walk_m*.err; do
    aws s3 cp "\$f" "s3://$BUCKET/$PREFIX/logs/\$(basename \$f)" || true
done
{ echo "# sbwalk facts, run $RUN_ID, generated \$(date -u +%FT%TZ)"
  cat /root/run/walk_m*.txt | ./parse_out.sh; } \\
  | aws s3 cp - "s3://$BUCKET/$PREFIX/walk.checkpoint" || true
aws s3 cp /var/log/sbwalk.log "s3://$BUCKET/$PREFIX/instance.log" || true
echo "FINAL UPLOAD COMPLETE"
shutdown -h now
EOF

if (( DRY )); then echo "--- user-data ---"; cat /tmp/sbwalk_userdata.sh; exit 0; fi

aws s3 cp --region "$REGION" /tmp/sbwalk_src.tgz "s3://$BUCKET/src/sbwalk_src_$SHA.tgz"
AMI=$(aws ssm get-parameter --region "$REGION" \
    --name /aws/service/ami-amazon-linux-latest/al2023-ami-kernel-default-x86_64 \
    --query Parameter.Value --output text)

echo "run id   $RUN_ID"
echo "prefix   s3://$BUCKET/$PREFIX/"
aws ec2 run-instances \
  --region "$REGION" \
  --image-id "$AMI" --instance-type "$TYPE" \
  --iam-instance-profile Name=radio-sa193-ec2 \
  --instance-initiated-shutdown-behavior stop \
  --block-device-mappings 'DeviceName=/dev/xvda,Ebs={VolumeSize=50,VolumeType=gp3,DeleteOnTermination=true}' \
  --user-data "file:///tmp/sbwalk_userdata.sh" \
  --tag-specifications "ResourceType=instance,Tags=[{Key=Project,Value=radio-sa193},{Key=Name,Value=sbwalk-$SHA},{Key=RunPrefix,Value=$PREFIX}]" \
  --query 'Instances[0].[InstanceId,InstanceType,Placement.AvailabilityZone]' --output text
