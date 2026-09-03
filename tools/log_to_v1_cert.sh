#!/bin/sh
# Reduce a raw solver log to a radio-negative-certificate-v1, streaming.
#
#   tools/log_to_v1_cert.sh <log> <out.cert> <root_level> [tmpdir]
#
# Every `can't solve [size=A/B ]Sb(...)[mass,n] in k` line becomes one record: `root` at
# <root_level>, `fact` elsewhere.  In a flat v1 audit the cleanroom checker treats both as
# claims and uses level k-1's claims as level k's support, so the tag is documentation.
#
# Parts are emitted VERBATIM in the order the solver printed them.  The checker applies
# CANON (orient n>=m, drop empty parts, sort descending) and UNIT (strip unit parts while
# keeping their mass in mass_full) itself.  In particular unit `(1:1)` parts are deliberately
# NOT stripped here: Unit-Group Elimination makes "R unsolvable" strictly stronger than
# "R + units unsolvable", so rewriting Sb(R+units) as Sb(R) would assert more than the log
# established.  Masses in the log annotation are ignored; the checker recomputes them.
set -eu
log=$1
out=$2
root_level=$3
tmp=${4:-${TMPDIR:-/tmp}}

pairs=$(mktemp "$tmp/v1pairs.XXXXXX")
trap 'rm -f -- "$pairs"' EXIT

# `can.t` avoids quoting an apostrophe inside the sed script; "can solve" cannot match it.
grep "^can't solve" "$log" \
  | sed -E "s/^can.t solve (size=[^ ]+ )?(Sb\([^)]*\))\[[0-9]+,[0-9]+\] in ([0-9]+).*/\3 \2/" \
  | grep -E '^[0-9]+ Sb\(' \
  | LC_ALL=C sort -u -T "$tmp" -S 25% > "$pairs"

{
    echo "radio-negative-certificate-v1"
    echo "# extracted from $log by tools/log_to_v1_cert.sh"
    echo "# parts verbatim; CANON/UNIT applied by the checker, masses recomputed"
    LC_ALL=C awk -v r="$root_level" '{ tag = ($1 == r ? "root" : "fact"); print tag, $1, $2 }' "$pairs"
} > "$out"

echo "wrote $out"
LC_ALL=C awk '{ print $2 }' "$pairs" 2>/dev/null | sort -n | uniq -c \
  | awk '{ printf "  k=%s: %s\n", $2, $1 }' || true
LC_ALL=C awk '{ c[$1]++ } END { for (k in c) printf "  k=%s: %s claims\n", k, c[k] }' "$pairs" \
  | sort -t= -k2 -n
