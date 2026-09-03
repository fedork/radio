#!/usr/bin/env bash
# Regression for the two log-to-evidence extractors added 2026-09-03:
#   tools/log_to_numbered_tree.py  (raw log -> numbered witness tree, via domination)
#   tools/log_to_v1_cert.sh        (raw log -> radio-negative-certificate-v1)
#
# Seconds, no corpus, no network. The fixtures are inline because they must stay in step with
# the assertions. Three things are actually load-bearing here:
#
#   * the DOMINATION path resolves a child that is not logged in its own right, which is the
#     whole reason the fifty K=8 frontier trees exist;
#   * a line whose printed children disagree with the re-derived split is REJECTED, which is
#     what keeps a misparsed take from silently entering a tree;
#   * unit `(1:1)` parts survive certificate extraction verbatim, because dropping them
#     asserts something strictly stronger than the log established (see the trap in
#     docs/status.md).
set -euo pipefail
cd "$(dirname "$0")/.."

work=$(mktemp -d /tmp/radio-log-to-tree-test.XXXXXX)
trap 'rm -rf -- "$work"' EXIT
pass=0

# Sb(4:1)@2 splits to Sb(2:1)@1 twice plus an empty child. Sb(2:1)@1 is deliberately NOT logged;
# only Sb(2:1,1:1)@1 is, and it dominates Sb(2:1) after unit deletion. A correct build therefore
# has two lines, with line 2 proving the SUPERSTATE and referenced twice.
cat > "$work/log.txt" <<'EOF'
can solve Sb(4:1)[4,5] in 2 with [2:1] Sb(2:1)[2,3]Sb(2:1)[2,3]Sb(0:0)[0,0] took 0.000 totalsplits=1 pass=1 fast_solve=0
can solve Sb(2:1,1:1)[3,5] in 1 with [1:1,0:0] Sb(1:1)[1,2]Sb(1:1)[1,2]Sb(1:1)[1,2] took 0.000 totalsplits=1 pass=1 fast_solve=0
EOF

tools/log_to_numbered_tree.py "$work/log.txt" --target 'Sb(4:1)' --k 2 -o "$work/tree.tree" \
    > "$work/build.out" 2>&1 || { echo 'FAIL: build'; cat "$work/build.out"; exit 1; }
tools/check_witness.py "$work/tree.tree" > "$work/check.out" 2>&1 \
    || { echo 'FAIL: checker rejected the tree'; cat "$work/check.out"; exit 1; }
grep -q 'verified unconditionally' "$work/check.out" \
    || { echo 'FAIL: not unconditional'; cat "$work/check.out"; exit 1; }
pass=$((pass + 1))

# The domination path must be the one taken: line 2 proves Sb(2:1,1:1), not Sb(2:1).
grep -Eq '^2\. \(in 1\) \(used 2\) Sb\(2:1,1:1\)' "$work/tree.tree" \
    || { echo 'FAIL: expected line 2 to prove the dominating superstate, used twice';
         cat "$work/tree.tree"; exit 1; }
pass=$((pass + 1))

# Drop the superstate line and nothing covers Sb(2:1)@1 any more: exit 3, with the gap named.
head -1 "$work/log.txt" > "$work/log-nodom.txt"
rc=0
tools/log_to_numbered_tree.py "$work/log-nodom.txt" --target 'Sb(4:1)' --k 2 \
    -o "$work/nodom.tree" > "$work/nodom.out" 2>&1 || rc=$?
[[ "$rc" == 3 ]] || { echo "FAIL: expected exit 3 without the dominating line, got $rc"; exit 1; }
grep -q 'UNRESOLVED' "$work/nodom.out" || { echo 'FAIL: no UNRESOLVED report'; exit 1; }
pass=$((pass + 1))

# A line whose printed children disagree with the re-derived split must be rejected, not used.
# Here the `both` child is falsified from Sb(2:1) to Sb(2:2).
sed 's/with \[2:1\] Sb(2:1)\[2,3\]/with [2:1] Sb(2:2)[4,4]/' "$work/log.txt" \
    > "$work/log-bad.txt"
rc=0
tools/log_to_numbered_tree.py "$work/log-bad.txt" --target 'Sb(4:1)' --k 2 \
    -o "$work/bad.tree" > "$work/bad.out" 2>&1 || rc=$?
[[ "$rc" == 3 ]] || { echo "FAIL: expected exit 3 on a child mismatch, got $rc"; exit 1; }
grep -q '1 child-mismatch lines rejected' "$work/bad.out" \
    || { echo 'FAIL: mismatch not counted'; cat "$work/bad.out"; exit 1; }
pass=$((pass + 1))

# --- certificate extraction ------------------------------------------------------------------
# Unit parts must survive verbatim, and `size=` lines must parse like plain ones.
cat > "$work/neg.txt" <<'EOF'
can't solve Sb(3:1)[3,4] in 1 took 0.000 totalsplits=1 pass=1 fast_solve=0
can't solve size=4/3 Sb(5:2,1:1)[11,9] in 3 took 0.000 totalsplits=1 pass=1 fast_solve=0
EOF
tools/log_to_v1_cert.sh "$work/neg.txt" "$work/neg.cert" 3 "$work" > "$work/cert.out" 2>&1 \
    || { echo 'FAIL: cert extraction'; cat "$work/cert.out"; exit 1; }
head -1 "$work/neg.cert" | grep -q '^radio-negative-certificate-v1$' \
    || { echo 'FAIL: missing v1 header'; exit 1; }
grep -q '^fact 1 Sb(3:1)$' "$work/neg.cert" \
    || { echo 'FAIL: k=1 fact missing'; cat "$work/neg.cert"; exit 1; }
# The unit part is kept, and the k=3 record is tagged `root` because 3 was passed as the root level.
grep -Eq '^root 3 Sb\(5:2,1:1\)$' "$work/neg.cert" \
    || { echo 'FAIL: unit part dropped or root level mistagged'; cat "$work/neg.cert"; exit 1; }
pass=$((pass + 1))

# The checker must accept the extracted certificate's shape (Sb(3:1) really is unsolvable in 1).
export PATH="$HOME/.cargo/bin:/opt/homebrew/opt/rustup/bin:$PATH"
if [[ ! -x tools/cleanroom/target/release/radio_cleanroom ]] && command -v cargo >/dev/null 2>&1; then
    ( cd tools/cleanroom && cargo build --release >/dev/null 2>&1 ) || true
fi
if [[ -x tools/cleanroom/target/release/radio_cleanroom ]]; then
    awk 'NR==1 || /^fact 1 /' "$work/neg.cert" > "$work/k1.cert"
    tools/cleanroom/target/release/radio_cleanroom audit "$work/k1.cert" > "$work/audit.out" 2>&1 \
        || { echo 'FAIL: cleanroom rejected Sb(3:1)@1'; cat "$work/audit.out"; exit 1; }
    grep -q 'TOTAL verified 1, gaps 0,' "$work/audit.out" \
        || { echo 'FAIL: unexpected audit total'; cat "$work/audit.out"; exit 1; }
    pass=$((pass + 1))
else
    echo 'note: cleanroom binary absent, skipping the audit cross-check'
fi

echo "log-to-tree regression: $pass checks OK (domination reuse, missing-domination gap,"
echo "  child-mismatch rejection, unit-part preservation, size= parsing, v1 shape)"
