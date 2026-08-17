#!/usr/bin/env bash
#
# Push and pull bulk solver output to a GitHub release store.
#
# Raw solver logs are far too large for this repo (~3.2 GB archived, ~26 GB in total across
# machines) but they are the evidence behind the "proven-exhaustive" rows in
# data/pareto_sb.csv, so losing them means losing the provenance. The valuable subset is
# compressed with zstd -19 (about 10% of raw, losslessly) and attached to tagged releases on
# a separate private repo.
#
# Two indexes live in this public repo, so the knowledge survives even when the bytes are not
# to hand: docs/data.md for humans (including what was deliberately NOT archived and why),
# and data/artifacts.csv for machines, which is what makes a `tag:path` source in data/*.csv
# checkable offline. `check-index` confirms the two still agree with the store.
#
# Usage:
#   tools/artifacts.sh list                        list tags in the store
#   tools/artifacts.sh check-index                 confirm data/artifacts.csv matches the store
#   tools/artifacts.sh show <tag>                  print a tag's manifest
#   tools/artifacts.sh push <tag> <file>...        compress, upload, record
#   tools/artifacts.sh pull <tag> [dest]           download, verify sha256, decompress
#   tools/artifacts.sh verify <tag>                download and check without keeping
#
# The store repo defaults to fedork/radio-data and can be overridden with RADIO_DATA_REPO.
# Requires: gh (authenticated), zstd, shasum.
# New solver logs must pass tools/check_provenance.py.  Historical/pre-banner logs require the
# explicit RADIO_ALLOW_LEGACY_PROVENANCE=1 escape hatch; using it is a classification decision that
# belongs in docs/data.md, not a quiet convenience flag.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# This repo belongs to the GitHub account `fedork`, which is not the machine's default gh
# login. Rather than switch the global active account, keep an isolated gh config inside the
# repo. `git` is already handled the same way, by the repo-local core.sshCommand.
#
#   GH_CONFIG_DIR="$PWD/.gh" gh auth login      # once, interactive
#
# .gh/ is gitignored. Anything here picks it up automatically.
if [ -z "${GH_CONFIG_DIR:-}" ]; then
    if [ -d "$REPO_ROOT/.gh" ]; then
        export GH_CONFIG_DIR="$REPO_ROOT/.gh"
    elif [ "${RADIO_ALLOW_GLOBAL_GH:-}" != "1" ]; then
        # Refuse rather than fall through to the machine's default login. That account has
        # no write access here, and silently archiving under the wrong owner is worse than
        # failing: the artifacts would be somewhere nobody thinks to look.
        cat >&2 <<MSG
artifacts: no repo-local gh credential.
  Run once:  GH_CONFIG_DIR="$REPO_ROOT/.gh" gh auth login
  This repo is owned by 'fedork'; the machine's default gh login is a different
  account with no write access. The global login is left untouched.
  To deliberately use the global login anyway, set RADIO_ALLOW_GLOBAL_GH=1.
MSG
        exit 1
    fi
fi

REPO="${RADIO_DATA_REPO:-fedork/radio-data}"
LEVEL="${RADIO_ZSTD_LEVEL:-19}"

die() { echo "artifacts: $*" >&2; exit 1; }

need() { command -v "$1" >/dev/null 2>&1 || die "$1 not found on PATH"; }
need gh; need zstd; need shasum

gh auth status >/dev/null 2>&1 \
    || die "gh is not authenticated (GH_CONFIG_DIR=${GH_CONFIG_DIR:-default}). Run: gh auth login"

WHO="$(gh api user --jq .login 2>/dev/null || true)"
[ -n "$WHO" ] || die "could not determine the authenticated gh account"
echo "artifacts: acting as $WHO, store $REPO" >&2

ensure_repo_nonempty() {
    # A release needs a commit to tag, and `gh repo create` leaves the repo empty.
    gh api "repos/$REPO/commits?per_page=1" >/dev/null 2>&1 && return 0
    echo "initialising $REPO with a README (releases need a commit to tag)"
    local body
    body=$(printf '# radio-data\n\nBulk solver output for fedork/radio, attached to tagged releases.\n\nThe index, provenance and reliability notes live in `docs/data.md` of the main repo, not\nhere. Fetch with `tools/artifacts.sh pull <tag>`.\n' | base64)
    gh api -X PUT "repos/$REPO/contents/README.md" \
        -f message="Initialise artifact store" -f content="$body" >/dev/null
}

ensure_release() {
    local tag="$1"
    if ! gh release view "$tag" -R "$REPO" >/dev/null 2>&1; then
        ensure_repo_nonempty
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
        local looks_solver=0
        case "$base" in out*.txt|*.solver.log) looks_solver=1 ;; esac
        if grep -Eq '^# radio-provenance-v1 begin$|^(can|can.t) solve |^still solving in ' \
                < <(head -c 1048576 "$f"); then
            looks_solver=1
        fi
        if [ "$looks_solver" -eq 1 ] && \
                ! python3 "$REPO_ROOT/tools/check_provenance.py" "$f" >&2; then
            if [ "${RADIO_ALLOW_LEGACY_PROVENANCE:-0}" != 1 ]; then
                die "$f is solver output without complete provenance; archive only with an explicit documented RADIO_ALLOW_LEGACY_PROVENANCE=1 override"
            fi
            echo "    WARNING: archiving pre-banner/incomplete solver output under the legacy-provenance override" >&2
        fi
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

cmd_check_index() {
    # data/artifacts.csv is the offline index that makes `tag:path` sources checkable.
    # This confirms it still matches the store.
    local idx="$REPO_ROOT/data/artifacts.csv"
    [ -f "$idx" ] || die "no $idx"
    local tmp; tmp="$(mktemp -d)"; trap 'rm -rf "$tmp"' RETURN
    local have rc=0
    have="$(gh release list -R "$REPO" --limit 100 --json tagName --jq '.[].tagName' | sort -u)"
    local tag asset rest prev="" assets_file
    while IFS=, read -r tag asset rest; do
        [ "$tag" = "tag" ] && continue
        if ! grep -qx "$tag" <<<"$have"; then
            [ "$tag" = "$prev" ] || echo "MISSING TAG   $tag"
            prev="$tag"; rc=1; continue
        fi
        [[ "$tag" =~ ^[A-Za-z0-9._-]+$ ]] || {
            echo "INVALID TAG   $tag"; rc=1; continue
        }
        assets_file="$tmp/$tag.assets"
        if [ ! -f "$assets_file" ]; then
            # Do not pipe `gh` into `grep -q` under pipefail: once grep finds an early match it
            # closes the pipe, and gh's resulting SIGPIPE is misreported as a missing asset.
            if ! gh release view "$tag" -R "$REPO" --json assets \
                    --jq '.assets[].name' > "$assets_file" 2>/dev/null; then
                echo "UNREADABLE TAG $tag"
                rc=1
                : > "$assets_file"
            fi
        fi
        if ! grep -qx "$asset" "$assets_file"; then
            echo "MISSING ASSET $tag / $asset"; rc=1
        fi
    done < "$idx"
    local extra
    extra="$(comm -13 <(awk -F, 'NR>1{print $1}' "$idx" | sort -u) <(echo "$have"))"
    [ -n "$extra" ] && echo "IN STORE BUT NOT INDEXED: $extra" && rc=1
    [ $rc -eq 0 ] && echo "index matches the store"
    return $rc
}

case "${1:-}" in
    list)   shift; cmd_list "$@" ;;
    check-index) shift; cmd_check_index "$@" ;;
    show)   shift; cmd_show "$@" ;;
    push)   shift; cmd_push "$@" ;;
    pull)   shift; cmd_pull "$@" ;;
    verify) shift; cmd_verify "$@" ;;
    *) sed -n '3,26p' "$0" | sed 's/^# \{0,1\}//' ; exit 2 ;;
esac
