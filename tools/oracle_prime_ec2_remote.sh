#!/usr/bin/env bash
# Runs ON the EC2 host. Loads the full archived cache corpus into radio_oracle and dumps a snapshot.
#
# The point is to settle empirically what a full prime costs, after three local extrapolations
# disagreed with each other. Local measurement on sequential prefixes gave 78,752 facts/s at 50k,
# 61,689/s at 200k and 687/s at 800k with the structure reaching 2.88 GB -- violently superlinear,
# and no basis for predicting 21.9M. So this run reports progress per chunk rather than assuming a
# rate, and stops cleanly on a memory cap instead of swapping.
set -euo pipefail

RUN_ID=""; WORK=""; PREFIX=""; COMMIT=""; SRC_SHA=""
RSS_CAP_GIB=${RSS_CAP_GIB:-28}
CHUNK=${CHUNK:-250000}
BUCKET=radio-sa193-393287594714
INPUT_KEY=pareto-census-k8/20260814T0132Z/input.tar.zst
while (($#)); do
    case "$1" in
        --run-id) RUN_ID=$2; shift 2 ;;
        --work) WORK=$2; shift 2 ;;
        --prefix) PREFIX=$2; shift 2 ;;
        --source-commit) COMMIT=$2; shift 2 ;;
        --source-sha256) SRC_SHA=$2; shift 2 ;;
        *) echo "unknown argument $1" >&2; exit 64 ;;
    esac
done
SRC=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
mkdir -p "$WORK"; cd "$WORK"
exec > >(tee -a supervisor.log) 2>&1

put() { aws s3 cp "$1" "s3://$BUCKET/$PREFIX/$1" --no-progress >/dev/null 2>&1 || true; }

# NOTE: no `set -e` semantics inside here, and no trailing conditional. The first version ended
# with `[[ -f exit.status ]] && echo ...`, which returns 1 before the file exists; under `set -e`
# that failed the function, killed the status subshell on its first iteration, and the run went
# 20 minutes with no STATUS uploaded. A status writer must never be able to fail.
write_status() {
    set +e
    {
        date -u +updated_utc=%FT%TZ
        printf 'run_id=%s\nstate=%s\ncommit=%s\n' "$RUN_ID" "$1" "$COMMIT"
        printf 'chunks_total=%s chunks_loaded=%s\n' "${TOTAL_CHUNKS:-?}" \
               "$(grep -ac '^OK loaded' out.txt 2>/dev/null || echo 0)"
        if [[ -f oracle.pid ]] && kill -0 "$(cat oracle.pid)" 2>/dev/null; then
            ps -o rss=,etimes= -p "$(cat oracle.pid)" 2>/dev/null | \
                awk '{printf "rss_gib=%.2f elapsed_s=%s\n", $1/1048576, $2}'
        fi
        awk '/^MemAvailable:/ {printf "host_mem_available_gib=%.1f\n", $2/1048576}' /proc/meminfo
        df -Pk . | awk 'NR==2 {printf "disk_available_gib=%.1f\n", $4/1048576}'
        echo 'last_chunks:'; grep -a '^OK loaded' out.txt 2>/dev/null | tail -3
        echo 'last_stats:';  grep -a '^OK queries=' out.txt 2>/dev/null | tail -1
        grep -a '^OK snapshot' out.txt 2>/dev/null | tail -1
        if [[ -f exit.status ]]; then echo "exit_status=$(cat exit.status)"; fi
        if [[ -f abort.reason ]]; then echo "abort_reason=$(cat abort.reason)"; fi
        return 0
    } > STATUS 2>/dev/null
    put STATUS
    put out.txt
    set -e
    return 0
}

echo "== fetching inputs =="
aws s3 cp "s3://$BUCKET/$INPUT_KEY" input.tar.zst --no-progress
zstd -dc input.tar.zst | tar -xf - input/exact.cache input/dominance.cache
ls -l input/

echo "== building oracle (MAX_K=9 MAX_N=300) =="
python3 "$SRC/tools/build_radio.py" -O3 -DMAX_K=9 -DMAX_N=300 \
    "$SRC/radio_oracle.c" -o oracle
cp "$SRC/radio_oracle.c" . 2>/dev/null || true

echo "== sorting caches (measured 2.25x on replay) =="
for c in exact dominance; do
    python3 "$SRC/tools/sort_cache.py" "input/$c.cache" "$c.sorted" --max-k 9 --max-n 300
done

echo "== chunking =="
rm -rf chunks; mkdir chunks
cat exact.sorted dominance.sorted | grep -v '^#' > all.sorted
rm -f exact.sorted dominance.sorted
split -l "$CHUNK" -d -a 4 all.sorted chunks/c
TOTAL_CHUNKS=$(ls chunks | wc -l | tr -d ' ')
TOTAL_FACTS=$(wc -l < all.sorted | tr -d ' ')
echo "chunks=$TOTAL_CHUNKS facts=$TOTAL_FACTS"

{
    printf 'started_utc=%s\n' "$(date -u +%FT%TZ)"
    printf 'run_id=%s\nsource_commit=%s\nsource_sha256=%s\n' "$RUN_ID" "$COMMIT" "$SRC_SHA"
    printf 'max_k=9\nmax_n=300\nchunk_lines=%s\nchunks=%s\nfacts=%s\n' \
           "$CHUNK" "$TOTAL_CHUNKS" "$TOTAL_FACTS"
    printf 'rss_cap_gib=%s\ninstance_type=%s\n' "$RSS_CAP_GIB" \
           "$(curl -s -m 2 -H "X-aws-ec2-metadata-token: $(curl -s -m 2 -X PUT \
              -H 'X-aws-ec2-metadata-token-ttl-seconds: 60' \
              http://169.254.169.254/latest/api/token)" \
              http://169.254.169.254/latest/meta-data/instance-type || echo unknown)"
} > run.meta
put run.meta
put oracle.provenance

echo "== building the query script =="
{
    for f in chunks/*; do printf 'load %s\n' "$WORK/$f"; printf 'stats\n'; done
    printf 'snapshot %s/cache.snap\n' "$WORK"
    printf 'stats\nquit\n'
} > script.txt

echo "== loading =="
( ./oracle < script.txt > out.txt 2> err.txt; echo $? > exit.status ) &
RUNNER=$!
# the oracle is the only child that matters; find it once it exists
for _ in $(seq 1 60); do pgrep -P $RUNNER -f oracle > oracle.pid 2>/dev/null && \
    [[ -s oracle.pid ]] && break; sleep 1; done

# memory guard: stop cleanly rather than swap the host to death
( while kill -0 $RUNNER 2>/dev/null; do
      if [[ -s oracle.pid ]]; then
          rss=$(ps -o rss= -p "$(cat oracle.pid)" 2>/dev/null || echo 0)
          if (( rss > RSS_CAP_GIB * 1048576 )); then
              echo "RSS CAP $RSS_CAP_GIB GiB exceeded; killing" | tee -a guard.log
              kill -TERM "$(cat oracle.pid)" 2>/dev/null || true
              echo rss-cap > abort.reason
              break
          fi
      fi
      sleep 20
  done ) &
GUARD=$!

( while kill -0 $RUNNER 2>/dev/null; do write_status running; sleep 60; done ) &
STATUSER=$!

wait $RUNNER || true
kill $GUARD $STATUSER 2>/dev/null || true
write_status finished

echo "== finalizing =="
[[ -f cache.snap ]] && ls -l cache.snap && zstd -T0 -3 -f cache.snap -o cache.snap.zst && \
    sha256sum cache.snap cache.snap.zst > snapshot.sha256
for f in out.txt err.txt supervisor.log guard.log run.meta STATUS exit.status abort.reason \
         snapshot.sha256 oracle.provenance cache.snap.zst; do
    [[ -f "$f" ]] && put "$f"
done
write_status finished
echo "done; s3://$BUCKET/$PREFIX/"
shutdown -h +2 || true
