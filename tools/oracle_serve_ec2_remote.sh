#!/usr/bin/env bash
# Runs ON the EC2 host. Builds radio_oracle (with the `enumerate` command) and keeps it serving
# indefinitely behind tools/oracle_server.py, unlike oracle_prime_ec2_remote.sh which loads a
# corpus, snapshots, and shuts down. This one is meant to persist across sessions: no hard-stop,
# a periodic local snapshot so a crash loses at most one interval, and a STATUS object in S3 so
# any future session can check on it without a live connection.
set -euo pipefail

RUN_ID=""; WORK=""; PREFIX=""; COMMIT=""; SRC_SHA=""
BUCKET=radio-sa193-393287594714
PORT=${PORT:-7777}
SNAPSHOT_EVERY=${SNAPSHOT_EVERY:-1800}
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

echo "== building oracle (MAX_K=9 MAX_N=500, includes the new enumerate command) =="
python3 "$SRC/tools/build_radio.py" -O3 -DMAX_K=9 -DMAX_N=500 \
    "$SRC/radio_oracle.c" -o oracle
cp "$SRC/radio_oracle.c" "$SRC/radiobase.c" . 2>/dev/null || true
put oracle.provenance

{
    printf 'started_utc=%s\n' "$(date -u +%FT%TZ)"
    printf 'run_id=%s\nsource_commit=%s\nsource_sha256=%s\n' "$RUN_ID" "$COMMIT" "$SRC_SHA"
    printf 'max_k=9\nmax_n=500\nport=%s\nsnapshot_every_s=%s\n' "$PORT" "$SNAPSHOT_EVERY"
    printf 'instance_type=%s\n' \
           "$(curl -s -m 2 -H "X-aws-ec2-metadata-token: $(curl -s -m 2 -X PUT \
              -H 'X-aws-ec2-metadata-token-ttl-seconds: 60' \
              http://169.254.169.254/latest/api/token)" \
              http://169.254.169.254/latest/meta-data/instance-type || echo unknown)"
} > run.meta
put run.meta

echo "== starting oracle_server.py (cold start, no pre-loaded cache) =="
# Restart loop, not a one-shot: a crash should come back from whatever the last snapshot had, not
# end the service. --restore is a no-op until oracle.snap first exists. systemd-run gives it a
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
systemd-run --unit=radio-oracle-server --collect \
    /bin/bash "$WORK/restart_loop.sh" "$WORK" \
    python3 "$SRC/tools/oracle_server.py" ./oracle --port "$PORT" \
    --snapshot-every "$SNAPSHOT_EVERY" --snapshot-dir "$WORK" \
    --restore "$WORK/oracle.snap"
sleep 3
pgrep -f './oracle' > oracle.pid || true

write_status running
( while true; do write_status running; sleep 300; done ) &
disown
echo "serving on 127.0.0.1:$PORT under systemd unit radio-oracle-server; STATUS in s3://$BUCKET/$PREFIX/STATUS"
