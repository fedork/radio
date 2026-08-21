#!/usr/bin/env bash
# Runs ON the EC2 host. Builds radio_oracle (with the `enumerate` command) and keeps it serving
# indefinitely behind tools/oracle_server.py, unlike oracle_prime_ec2_remote.sh which loads a
# corpus, snapshots, and shuts down. This one is meant to persist across sessions: no hard-stop,
# a periodic local snapshot so a crash loses at most one interval, and a STATUS object in S3 so
# any future session can check on it without a live connection.
#
# WARM START, per Fedor 2026-08-21: never cold-start this. On a fresh disk (no local oracle.snap
# yet -- this boot has never snapshotted), restore the already-validated full-corpus snapshot
# (21.9M facts, 32.8s restore -- see docs/aws-run.md) and then load the sa193 certificate of
# record (2,846,568 more proof-safe negative facts, converted by tools/cert_to_cache.py, ~7 min
# locally -- see evidence/cert_to_cache_validation_2026-08-21.txt for why this is safe: the
# converter and load path were empirically validated at two scales, 127 and 80,634 claims, zero
# mismatches against independently-computed ground truth). If a local oracle.snap already exists
# (this boot's own oracle_server.py restarted after a crash), skip both external fetches entirely
# and restore that instead -- it already has everything the fresh-disk path would give it, plus
# whatever real queries added since.
set -euo pipefail

RUN_ID=""; WORK=""; PREFIX=""; COMMIT=""; SRC_SHA=""
BUCKET=radio-sa193-393287594714
PORT=${PORT:-7777}
SNAPSHOT_EVERY=${SNAPSHOT_EVERY:-1800}
BASE_SNAPSHOT_KEY=oracle-prime/20260820T165448Z/cache.snap.zst
CERT_CACHE_KEY=cert-cache/sa193-certificate-2026-08-19.cache.zst
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

# A status writer must never be able to fail (see oracle_prime_ec2_remote.sh's own note on this,
# 2026-08-19: an unguarded `[[ ... ]] &&` under `set -e` silently killed status reporting for
# 20 minutes once).
write_status() {
    set +e
    {
        date -u +updated_utc=%FT%TZ
        printf 'run_id=%s\nstate=%s\ncommit=%s\nport=%s\n' "$RUN_ID" "$1" "$COMMIT" "$PORT"
        if [[ -f oracle.pid ]] && kill -0 "$(cat oracle.pid)" 2>/dev/null; then
            ps -o rss=,etimes= -p "$(cat oracle.pid)" 2>/dev/null | \
                awk '{printf "rss_gib=%.2f elapsed_s=%s\n", $1/1048576, $2}'
        fi
        awk '/^MemAvailable:/ {printf "host_mem_available_gib=%.1f\n", $2/1048576}' /proc/meminfo
        awk '/^MemTotal:/ {printf "host_mem_total_gib=%.1f\n", $2/1048576}' /proc/meminfo
        df -Pk . | awk 'NR==2 {printf "disk_available_gib=%.1f\n", $4/1048576}'
        grep -a '^\[oracle_server\]' server.out 2>/dev/null | tail -5
        [[ -f oracle.snap ]] && ls -l oracle.snap | awk '{printf "snapshot_bytes=%s\n", $5}'
        return 0
    } > STATUS 2>/dev/null
    put STATUS
    set -e
    return 0
}

echo "== building oracle (MAX_K=9 MAX_N=300, matches the base snapshot's geometry) =="
# 300 was already measured safe at this MAX_K on this driver (oracle-prime used the same
# MAX_K=9/MAX_N=300 build). Do not raise it without evidence: MAX_N=500 sat in init() past 500s
# and 2 GiB RSS on this exact instance type with no forcing query, and cost real time before being
# caught (docs/aws-run.md's sizing-history note).
python3 "$SRC/tools/build_radio.py" -O3 -DMAX_K=9 -DMAX_N=300 \
    "$SRC/radio_oracle.c" -o oracle
cp "$SRC/radio_oracle.c" "$SRC/radiobase.c" . 2>/dev/null || true
put oracle.provenance

{
    printf 'started_utc=%s\n' "$(date -u +%FT%TZ)"
    printf 'run_id=%s\nsource_commit=%s\nsource_sha256=%s\n' "$RUN_ID" "$COMMIT" "$SRC_SHA"
    printf 'max_k=9\nmax_n=300\nport=%s\nsnapshot_every_s=%s\n' "$PORT" "$SNAPSHOT_EVERY"
    printf 'instance_type=%s\n' \
           "$(curl -s -m 2 -H "X-aws-ec2-metadata-token: $(curl -s -m 2 -X PUT \
              -H 'X-aws-ec2-metadata-token-ttl-seconds: 60' \
              http://169.254.169.254/latest/api/token)" \
              http://169.254.169.254/latest/meta-data/instance-type || echo unknown)"
} > run.meta
put run.meta

# RESTORE_TARGET is the one snapshot oracle_server.py's --restore flag points at -- it already
# turns "restore this file, if it exists" into the correct --restore-any= form internally (see
# oracle_server.py), so passing a leading-dash token through this script's own argv is never
# needed. WARMSTART_CACHE is a plain file path (no leading dash), safe as an ordinary positional.
RESTORE_TARGET="$WORK/oracle.snap"
WARMSTART_CACHE=()
if [[ -f oracle.snap ]]; then
    echo "== local snapshot already exists (restart after a crash) -- skipping external warm-start =="
else
    echo "== fresh disk: warm-starting from the archived corpus, not cold-starting =="
    echo "-- restoring the validated full-corpus snapshot (21.9M facts, ~33s) --"
    aws s3 cp "s3://$BUCKET/$BASE_SNAPSHOT_KEY" base.snap.zst --no-progress
    zstd -d base.snap.zst -o base.snap
    rm -f base.snap.zst
    echo "-- loading the sa193 certificate of record on top (2,846,568 more facts, ~7 min) --"
    aws s3 cp "s3://$BUCKET/$CERT_CACHE_KEY" cert.cache.zst --no-progress
    zstd -d cert.cache.zst -o cert.cache
    rm -f cert.cache.zst
    # order matters: a snapshot restore must be the FIRST thing an oracle does, before any load or
    # query (radiobase.c refuses otherwise). --restore-any= is always injected ahead of any plain
    # cache path in oracle_server.py's own argv construction, so listing base.snap via --restore
    # and cert.cache as a positional preserves that order regardless of flag order on this line.
    RESTORE_TARGET="$WORK/base.snap"
    WARMSTART_CACHE=("$WORK/cert.cache")
fi

echo "== starting oracle_server.py =="
# Restart loop, not a one-shot: a crash should come back from whatever the last snapshot had, not
# end the service. --restore is a no-op until oracle.snap first exists (which the warm-start above
# does not create -- only oracle_server.py's own periodic snapshot does). systemd-run gives it a
# unit name to inspect/stop without an attached shell.
cat > restart_loop.sh <<'EOF'
#!/bin/bash
cd "$1"; shift
while true; do
    "$@" >> server.out 2>&1
    echo "$(date -u +%FT%TZ) oracle_server exited, restarting in 5s" >> server.out
    sleep 5
done
EOF
chmod +x restart_loop.sh
# The "caches" positional (nargs='*') MUST come immediately after the binary positional, before
# any --flag. argparse's handling of a nargs='*' positional interspersed with value-taking
# optionals is a known ambiguity: `binary --restore X CACHEFILE` reports CACHEFILE as an
# unrecognized argument, while `binary CACHEFILE --restore X` parses correctly. Caught live on
# the first real deploy attempt (2026-08-21) -- the restart loop crash-looped harmlessly (no
# facts, no queries, no cache mutation) rather than corrupting anything, but fix the order.
systemd-run --unit=radio-oracle-server --collect \
    /bin/bash "$WORK/restart_loop.sh" "$WORK" \
    python3 "$SRC/tools/oracle_server.py" ./oracle "${WARMSTART_CACHE[@]}" \
    --port "$PORT" --snapshot-every "$SNAPSHOT_EVERY" --snapshot-dir "$WORK" \
    --restore "$RESTORE_TARGET"
sleep 3
pgrep -f './oracle' > oracle.pid || true

write_status running
( while true; do write_status running; sleep 300; done ) &
disown
echo "serving on 127.0.0.1:$PORT under systemd unit radio-oracle-server; STATUS in s3://$BUCKET/$PREFIX/STATUS"
