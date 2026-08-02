#!/usr/bin/env bash
#
# Push and pull bulk solver output to a GitHub release store.
#
# Raw solver logs are far too large for this repo (the corpus is ~2.1 GB) but they are the
# evidence behind every "proven-exhaustive" row in data/pareto_sb.csv, so losing them means
# losing the provenance. They are compressed with zstd -19 (about 9% of raw, losslessly)
# and attached to tagged releases on a separate private repo. The *index* of what exists
# stays in docs/data.md, in this public repo, so the knowledge survives even if the bytes
# are not to hand.
#
# Usage:
#   tools/artifacts.sh list                        list tags in the store
#   tools/artifacts.sh show <tag>                  print a tag's manifest
#   tools/artifacts.sh push <tag> <file>...        compress, upload, record
#   tools/artifacts.sh pull <tag> [dest]           download, verify sha256, decompress
#   tools/artifacts.sh verify <tag>                download and check without keeping
#
# The store repo defaults to fedork/radio-data and can be overridden with RADIO_DATA_REPO.
# Requires: gh (authenticated), zstd, shasum.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# This repo belongs to the GitHub account `fedork`, which is not the machine's default gh
# login. Rather than switch the global active account, keep an isolated gh config inside the
# repo. `git` is already handled the same way, by the repo-local core.sshCommand.
#
#   GH_CONFIG_DIR="$PWD/.gh" gh auth login      # once, interactive
#
# .gh/ is gitignored. Anything here picks it up automatically.
if [ -d "$REPO_ROOT/.gh" ] && [ -z "${GH_CONFIG_DIR:-}" ]; then
    export GH_CONFIG_DIR="$REPO_ROOT/.gh"
fi

REPO="${RADIO_DATA_REPO:-fedork/radio-data}"
LEVEL="${RADIO_ZSTD_LEVEL:-19}"

die() { echo "artifacts: $*" >&2; exit 1; }

need() { command -v "$1" >/dev/null 2>&1 || die "$1 not found on PATH"; }
need gh; need zstd; need shasum

gh auth status >/dev/null 2>&1 || die "gh is not authenticated for this repo.
  Run once:  GH_CONFIG_DIR=\"$REPO_ROOT/.gh\" gh auth login
  That keeps the credential local to this repo; the global gh login is untouched."

ensure_release() {
    local tag="$1"
    if ! gh release view "$tag" -R "$REPO" >/dev/null 2>&1; then
        echo "creating release $tag in $REPO"
        gh release create "$tag" -R "$REPO" \
            --title "$tag" \
            --notes "Solver output archive. Index and provenance: docs/data.md in fedork/radio." \
            >/dev/null
    fi
}

cmd_list() {
    gh release list -R "$REPO" --limit 100
}

cmd_show() {
    local tag="${1:?usage: artifacts.sh show <tag>}"
    local tmp; tmp="$(mktemp -d)"; trap 'rm -rf "$tmp"' RETURN
    gh release download "$tag" -R "$REPO" -p MANIFEST.tsv -D "$tmp" --clobber 2>/dev/null \
        || die "no manifest for $tag"
    column -t -s $'\t' "$tmp/MANIFEST.tsv"
}

cmd_push() {
    local tag="${1:?usage: artifacts.sh push <tag> <file>...}"; shift
    [ $# -gt 0 ] || die "no files given"
    ensure_release "$tag"

    local tmp; tmp="$(mktemp -d)"; trap 'rm -rf "$tmp"' RETURN
    # start from the existing manifest so a tag can be added to incrementally
    if gh release download "$tag" -R "$REPO" -p MANIFEST.tsv -D "$tmp" --clobber 2>/dev/null; then
        :
    else
        printf 'asset\tsha256_raw\tbytes_raw\tbytes_zst\tsource_path\thost\tarchived\n' \
            > "$tmp/MANIFEST.tsv"
    fi

    local f base sha raw zst
    for f in "$@"; do
        [ -f "$f" ] || die "no such file: $f"
        base="$(basename "$f")"
        echo "==> $base"
        sha="$(shasum -a 256 "$f" | cut -d' ' -f1)"
        raw="$(stat -f%z "$f")"
        echo "    sha256 $sha  ($raw bytes)"
        echo "    compressing at zstd -$LEVEL ..."
        zstd -q -f -"$LEVEL" -T0 "$f" -o "$tmp/$base.zst"
        zst="$(stat -f%z "$tmp/$base.zst")"
        printf '    %s bytes (%.1f%% of raw)\n' "$zst" \
            "$(echo "scale=4; $zst*100/$raw" | bc)"
        # drop any previous row for this asset, then append the new one
        grep -v "^$base.zst	" "$tmp/MANIFEST.tsv" > "$tmp/m2" || true
        mv "$tmp/m2" "$tmp/MANIFEST.tsv"
        printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
            "$base.zst" "$sha" "$raw" "$zst" "$f" "$(hostname -s)" "$(date -u +%Y-%m-%d)" \
            >> "$tmp/MANIFEST.tsv"
        echo "    uploading ..."
        gh release upload "$tag" -R "$REPO" "$tmp/$base.zst" --clobber
    done

    gh release upload "$tag" -R "$REPO" "$tmp/MANIFEST.tsv" --clobber
    echo "done: $tag"
}

cmd_pull() {
    local tag="${1:?usage: artifacts.sh pull <tag> [dest]}"
    local dest="${2:-.}"
    mkdir -p "$dest"
    gh release download "$tag" -R "$REPO" -D "$dest" --clobber
    [ -f "$dest/MANIFEST.tsv" ] || die "no manifest in $tag"

    local rc=0 asset sha raw rest
    while IFS=$'\t' read -r asset sha raw rest; do
        [ "$asset" = "asset" ] && continue
        [ -f "$dest/$asset" ] || { echo "MISSING  $asset"; rc=1; continue; }
        zstd -q -d -f "$dest/$asset" -o "$dest/${asset%.zst}"
        local got size
        got="$(shasum -a 256 "$dest/${asset%.zst}" | cut -d' ' -f1)"
        size="$(stat -f%z "$dest/${asset%.zst}")"
        if [ "$got" = "$sha" ] && [ "$size" = "$raw" ]; then
            echo "OK       ${asset%.zst}  ($size bytes)"
            rm -f "$dest/$asset"
        else
            echo "CORRUPT  ${asset%.zst}  sha $got != $sha  size $size != $raw"
            rc=1
        fi
    done < "$dest/MANIFEST.tsv"
    return $rc
}

cmd_verify() {
    local tag="${1:?usage: artifacts.sh verify <tag>}"
    local tmp; tmp="$(mktemp -d)"; trap 'rm -rf "$tmp"' RETURN
    cmd_pull "$tag" "$tmp"
}

case "${1:-}" in
    list)   shift; cmd_list "$@" ;;
    show)   shift; cmd_show "$@" ;;
    push)   shift; cmd_push "$@" ;;
    pull)   shift; cmd_pull "$@" ;;
    verify) shift; cmd_verify "$@" ;;
    *) sed -n '3,26p' "$0" | sed 's/^# \{0,1\}//' ; exit 2 ;;
esac
