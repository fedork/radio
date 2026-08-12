#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

tmp=$(mktemp -d "${TMPDIR:-/tmp}/radio-joint-rss.XXXXXX")
victim=
guard=
cleanup() {
    [[ -z "$guard" ]] || kill -TERM "$guard" 2>/dev/null || true
    [[ -z "$victim" ]] || kill -TERM "$victim" 2>/dev/null || true
    rm -rf "$tmp"
}
trap cleanup EXIT

bash -c 'exec -a radio_joint_tst sleep 30' &
victim=$!
SA193_JOINT_RSS_POLL=1 tools/sa193_joint_rss_guard.sh 1 "$victim" radio_joint_tst \
    >"$tmp/out" 2>&1 &
guard=$!

for _ in 1 2 3 4 5; do
    kill -0 "$guard"
    kill -0 "$victim"
    sleep 0.1
done
kill -TERM "$victim"
wait "$victim" 2>/dev/null || true
victim=
wait "$guard"
guard=
grep -F 'victim wrapper gone' "$tmp/out" >/dev/null
echo "joint RSS guard regression passed"
