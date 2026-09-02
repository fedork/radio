#!/usr/bin/env bash
# Regression for the cleanroom verifier (tools/cleanroom).
#
# Runs in a few minutes with no network and no corpus: the crate's unit tests, the built-in
# exhaustive selftest against the unquotiented naive oracle, the repo's closed and gap
# fixtures, and a battery of mutations that must each be caught. The mutations are the point:
# a checker that verifies everything is worthless, so every one of these must fail closed.
set -euo pipefail
cd "$(dirname "$0")/.."

export PATH="$HOME/.cargo/bin:/opt/homebrew/opt/rustup/bin:$PATH"
work=$(mktemp -d /tmp/radio-cleanroom-test.XXXXXX)
trap 'rm -rf -- "$work"' EXIT

( cd tools/cleanroom && cargo build --release >/dev/null 2>&1 )
bin=$PWD/tools/cleanroom/target/release/radio_cleanroom

pass=0
# expect EXIT_CODE LABEL -- command...
expect() {
    local want=$1 label=$2; shift 3
    local got=0
    "$@" > "$work/out" 2>&1 || got=$?
    if [[ "$got" != "$want" ]]; then
        echo "FAIL [$label]: expected exit $want, got $got" >&2
        sed 's/^/    /' "$work/out" >&2
        exit 1
    fi
    pass=$((pass + 1))
}
# expect_gap LABEL -- command...   (exit 1 AND a GAP line, not merely a nonzero exit)
expect_gap() {
    local label=$1; shift 2
    local got=0
    "$@" > "$work/out" 2>&1 || got=$?
    if [[ "$got" != 1 ]] || ! grep -q '^GAP ' "$work/out"; then
        echo "FAIL [$label]: expected a reported GAP and exit 1, got exit $got" >&2
        sed 's/^/    /' "$work/out" >&2
        exit 1
    fi
    pass=$((pass + 1))
}

( cd tools/cleanroom && cargo test --release >/dev/null 2>&1 ) \
    || { echo 'FAIL: cargo test' >&2; exit 1; }
pass=$((pass + 1))

"$bin" selftest > "$work/selftest.out" 2>&1 || { cat "$work/selftest.out" >&2; exit 1; }
grep -q 'SELFTEST PASSED' "$work/selftest.out" || { echo 'FAIL: selftest' >&2; exit 1; }
pass=$((pass + 1))

# --- the repo's shared fixtures ------------------------------------------------------------
expect 0 'closed v1 fixture' -- "$bin" audit tools/testdata/radio_verify_v1.cert
expect_gap 'gap v1 fixture (Sb(1:1) is solvable by UNIT)' -- \
    "$bin" audit tools/testdata/radio_refute_gap_v1.cert

# --- v1 mutations ---------------------------------------------------------------------------
# A solvable state asserted as a negative must not verify. Sb(2:1) is solvable in 2.
sed 's/^root 2 Sb(3:3)$/root 2 Sb(3:3)\nroot 2 Sb(2:1)/' \
    tools/testdata/radio_verify_v1.cert > "$work/solvable-claim.cert"
expect_gap 'injected solvable claim' -- "$bin" audit "$work/solvable-claim.cert"

# --- level-v2 semantic mutations, on a real load-bearing level ------------------------------
# tools/testdata/cleanroom_level_v2.cert is level 3 of the certificate of record: 127 claims
# over 2 support facts, and the support genuinely carries 88 of them.
v2=tools/testdata/cleanroom_level_v2.cert
expect 0 'real level-v2 certificate' -- "$bin" audit "$v2"

# Dropping the support must expose the claims that depended on it.
python3 - "$v2" "$work/v2-nosupport.cert" <<'EOF'
import sys
src, dst = sys.argv[1], sys.argv[2]
out = []
for line in open(src).read().splitlines():
    if line.startswith('support '):
        out.append('support 2 0 0')
    elif not line.startswith('fact '):
        out.append(line)
open(dst, 'w').write('\n'.join(out) + '\n')
EOF
expect_gap 'level-v2 support dropped' -- "$bin" audit "$work/v2-nosupport.cert"

# Removing one support fact (counts kept consistent) must break what cited it.
python3 - "$v2" "$work/v2-onefact.cert" <<'EOF'
import sys
src, dst = sys.argv[1], sys.argv[2]
lines = open(src).read().splitlines()
facts = [i for i, l in enumerate(lines) if l.startswith('fact ')]
drop = facts[0]
refs = len(lines[drop].split()) - 1
out = []
for i, line in enumerate(lines):
    if i == drop:
        continue
    if line.startswith('support '):
        tag, lvl, n, r = line.split()
        line = f'{tag} {lvl} {int(n)-1} {int(r)-refs}'
    out.append(line)
open(dst, 'w').write('\n'.join(out) + '\n')
EOF
expect_gap 'level-v2 one support fact removed' -- "$bin" audit "$work/v2-onefact.cert"

# --- v2 grammar strictness ------------------------------------------------------------------
cat > "$work/v2-good.cert" <<'EOF'
radio-negative-level-certificate-v2
level 2
parts 2
part 1 3:1
part 2 5:1
support 1 1 1
fact 1
split-hints 1
split 2 uses 1
claims 2 1 1
claim 2
EOF
expect 0 'well-formed v2' -- "$bin" audit "$work/v2-good.cert"

mutate_v2() { sed "$1" "$work/v2-good.cert" > "$work/v2-bad.cert"; }

mutate_v2 's/^claims 2 1 1$/claims 2 2 2/'
expect 2 'v2 claim count overstated' -- "$bin" audit "$work/v2-bad.cert"

mutate_v2 's/^support 1 1 1$/support 1 1 2/'
expect 2 'v2 support ref count wrong' -- "$bin" audit "$work/v2-bad.cert"

mutate_v2 's/^support 1 1 1$/support 3 1 1/'
expect 2 'v2 support level must be k-1' -- "$bin" audit "$work/v2-bad.cert"

mutate_v2 's/^split 2 uses 1$/split 2 uses 2/'
expect 2 'v2 split hint miscounted' -- "$bin" audit "$work/v2-bad.cert"

mutate_v2 's/^split 2 uses 1$/split 1 uses 1/'
expect 2 'v2 split hint names the wrong part' -- "$bin" audit "$work/v2-bad.cert"

mutate_v2 's/^part 2 5:1$/part 2 2:1/'
expect 2 'v2 part definitions not ascending' -- "$bin" audit "$work/v2-bad.cert"

mutate_v2 's/^part 1 3:1$/part 1 1:3/'
expect 2 'v2 part not canonical (n >= m)' -- "$bin" audit "$work/v2-bad.cert"

printf 'claim 1 2\n' >> "$work/v2-good.cert"
expect 2 'v2 trailing data after claims' -- "$bin" audit "$work/v2-good.cert"

# --- determinism and sharding ----------------------------------------------------------------
# Thread count must not change the verdict, and the stride shards must partition the claims.
a=$("$bin" audit --threads 1 tools/testdata/radio_verify_v1.cert | grep '^TOTAL')
b=$("$bin" audit --threads 4 tools/testdata/radio_verify_v1.cert | grep '^TOTAL')
[[ "$a" == "$b" ]] || { echo "FAIL: thread count changed the result: '$a' vs '$b'" >&2; exit 1; }
pass=$((pass + 1))

total=0
for off in 0 1 2; do
    n=$("$bin" audit --stride 3 --offset "$off" tools/testdata/radio_verify_v1.cert \
        | sed -n 's/^TOTAL verified \([0-9]*\),.*/\1/p')
    total=$((total + n))
done
[[ "$total" == 11 ]] || { echo "FAIL: stride shards summed to $total, expected 11" >&2; exit 1; }
pass=$((pass + 1))

echo "cleanroom regression: $pass checks OK (unit tests, exhaustive selftest, fixtures,"
echo "  claim/support/level mutations, v2 grammar strictness, thread and shard invariance)"
