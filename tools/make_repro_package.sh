#!/usr/bin/env bash
# Assemble the self-contained reproduction package for Sa(10) = 192.
#
#   tools/make_repro_package.sh [--out DIR]
#
# Produces `sa193-repro-<date>.tar.gz` plus its SHA-256. The package is what a third party
# needs to re-check the result without any of this project's infrastructure, private
# archives or trust: the certificate, two independent checkers, the positive witnesses, and
# exact expected outputs to diff against.
#
# It is deliberately small (~16 MB, almost all certificate) so it can be deposited publicly
# and cited by DOI. Everything in it is already public in this repo except the certificate,
# which currently lives in the private artifact store - depositing this package is what makes
# the result independently checkable.
set -euo pipefail
cd "$(dirname "$0")/.."

OUT=.
while (( $# )); do
    case "$1" in
        --out) OUT=$2; shift 2 ;;
        *) echo "usage: $0 [--out DIR]" >&2; exit 64 ;;
    esac
done

CERTS=.artifacts/cert
[[ -f "$CERTS/MANIFEST.sha256" ]] || {
    echo "missing $CERTS - run: tools/artifacts.sh pull sa193-certificate-2026-08-19 $CERTS" >&2
    exit 66
}

sha256_file() {
    if command -v sha256sum >/dev/null 2>&1; then sha256sum "$1" | awk '{print $1}'
    else shasum -a 256 "$1" | awk '{print $1}'
    fi
}

stamp=$(date -u +%Y%m%d)
work=$(mktemp -d /tmp/sa193-repro.XXXXXX)
root="$work/sa193-repro"
trap 'rm -rf -- "$work"' EXIT
mkdir -p "$root"/{certificate,witnesses,checkers,evidence}

# 1. The certificate, hash-checked against the release manifest as it is copied.
for k in 2 3 4 5 6 7 8 9; do
    f="sa193-k$k.cert.zst"
    want=$(awk -v n="$f" '$2 == n {print $1}' "$CERTS/MANIFEST.sha256")
    [[ "$(sha256_file "$CERTS/$f")" == "$want" ]] || { echo "$f hash mismatch" >&2; exit 66; }
    cp "$CERTS/$f" "$root/certificate/$f"
done
cp "$CERTS/MANIFEST.sha256" "$root/certificate/"

# 2. The positive half.
cp witnesses/sa192_k10_a.tree witnesses/sa192_k10_b.tree "$root/witnesses/"

# 3. Both independent checkers. The Rust crate is source-only; it has no dependencies, so it
#    builds offline from a pinned toolchain.
mkdir -p "$root/checkers/cleanroom"
cp tools/cleanroom/Cargo.toml tools/cleanroom/Cargo.lock \
   tools/cleanroom/rust-toolchain.toml "$root/checkers/cleanroom/"
cp -R tools/cleanroom/src "$root/checkers/cleanroom/src"
cp tools/check_level_chain.py tools/check_witness.py "$root/checkers/"

# 4. What the result rests on, in the project's own words.
cp evidence/cleanroom_verifier_2026-09-01.txt \
   evidence/sa193_unsolvable_in_10.txt "$root/evidence/"

cat > "$root/README.md" <<'EOF'
# Reproduction package: Sa(10) = 192

**Claim.** With a test that reports how many of two defectives lie in the queried subset,
192 coins can be resolved in 10 adaptive tests and 193 cannot.

Everything needed to check that is here. No network access, no dependencies beyond a Rust
toolchain and Python 3, and no need to trust the search program that found the proof.

## The two halves

**192 is achievable.** `witnesses/sa192_k10_a.tree` is an explicit strategy. Check it with:

    python3 checkers/check_witness.py witnesses/sa192_k10_a.tree

This re-derives every test's three children from first principles, checks each child against
the reference that discharges it, and checks the information bound `mass <= 3^k` at every
node. It never consults a solver. `sa192_k10_b.tree` is a second, smaller witness.

**193 is not.** `Sa(n)` in `k` tests reduces to a taken group of `n1` and the rest, needing
`Sa(n1)` and `Sb(n1 : n-n1)` in `k-1`. Since the maximum at `k=9` is `Sa(112)`, `Sa(193)` in
10 is unsolvable exactly when all sixteen `Sb(n1 : 193-n1)` are unsolvable in 9, for
`n1 = 97..112`. Those sixteen are the top level of the certificate in `certificate/`: eight
levels, k=2 through k=9, 2,846,568 claims, each claim a state asserted unsolvable at its
level, proved from claims one level below.

Checking it has two independent halves; both are needed.

*Structure*, no solver involved:

    zstd -d certificate/sa193-k*.cert.zst
    python3 checkers/check_level_chain.py --expect-top-sum 193 certificate/sa193-k*.cert

Confirms each level's declared counts, that level k's support set is exactly level (k-1)'s
claim set as resolved states, that level 2's support is empty so the induction terminates,
and that the top level is precisely the sixteen states summing to 193. Seconds.

*Semantics*:

    cd checkers/cleanroom && cargo build --release && cd ../..
    checkers/cleanroom/target/release/radio_cleanroom selftest
    checkers/cleanroom/target/release/radio_cleanroom audit \
        --threads $(nproc) --progress 300 certificate/sa193-k*.cert

The second command must print `TOTAL verified 2846568, gaps 0`. It re-derives every claim:
for each, it enumerates the whole legal split space and requires every split to have a child
discharged by the information bound, by star-expansion majorization, or by citing a claim one
level lower. A claim whose split space is not covered is reported as a gap and the run fails.

Expect about 1h50m on 32 cores and 3.1 GB of memory; k=7 alone is 99.5% of it.

`selftest` is the check that the checker itself is honest: for every state with at most three
parts, `n <= 6` and `k <= 4`, it compares against a deliberately naive solver that applies no
symmetry reductions at all, and requires that it verify exactly the unsolvable states and
refuse the solvable ones even when given complete support.

## What this package does and does not establish

The certificate is a *replay* artifact: the proof source is an exhaustive search whose raw
log is archived separately. What the certificate plus these checkers establish is that a
complete, gap-free refutation exists and has been checked by an implementation sharing no
code with the search program. `checkers/cleanroom` was written from the problem statement and
the theorem statements alone; it reads no cache and recomputes every rule, including the base
sequence `G_k`, from its defining recurrence.

`evidence/cleanroom_verifier_2026-09-01.txt` records two full verification runs at different
commits, which reached an identical candidate-cell count of 3,252,096,103,282, and agree with
the original search program on the number of citations made (1.18 trillion at k=7).

`evidence/sa193_unsolvable_in_10.txt` holds the sixteen root verdict lines from the original
run with its hashes and build provenance.
EOF

( cd "$root" && find . -type f ! -name MANIFEST.sha256 | sort | while read -r f; do
    printf '%s  %s\n' "$(cd "$OLDPWD" && sha256_file "$root/${f#./}")" "${f#./}"
  done > MANIFEST.sha256 )

mkdir -p "$OUT"
tarball="$OUT/sa193-repro-$stamp.tar.gz"
tar -C "$work" -czf "$tarball" sa193-repro
printf 'package %s\nbytes   %s\nsha256  %s\nfiles   %s\n' \
    "$tarball" "$(wc -c < "$tarball" | tr -d ' ')" "$(sha256_file "$tarball")" \
    "$(tar -tzf "$tarball" | grep -c .)"
