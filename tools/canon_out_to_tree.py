#!/usr/bin/env python3
"""Turn raw radio_canon_search_generic stdout into a witnesses/*.tree file.

The raw output begins with radiobase.c's provenance/environment banner (one bare value per
line) and ends with the canonical tree plus a `REACH:` summary on stderr.  This script keeps
the tree, records where it came from in a comment header, and writes the result in the exact
form `tools/check_witness.py` auto-detects as the `canonical` format.

    tools/canon_out_to_tree.py <raw.out> -o witnesses/<name>.tree \
        --command './canon8n 3 8 231 5' [--note '...']

It refuses to write when the raw output contains `out of nodes` (a node-pool ABORT, not a
proof of absence) or `NO_CANONICAL_TREE` (genuine exhaustion at that target_k, still not an
unsolvability claim), so neither can be mistaken for a witness.
"""
from __future__ import annotations

import argparse
import re
import sys

# A tree line: optional indent, then `n:m[,n:m...] @k` and either a split arrow or a leaf tag.
TREE = re.compile(r"^(?: *)\d+:\d+(?:,\d+:\d+)* @\d+(?: --\[.*\]-->| \[[^\]]+\])\s*$")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("raw")
    ap.add_argument("-o", "--out", required=True)
    ap.add_argument("--command", required=True, help="the search command, for the header")
    ap.add_argument("--note", action="append", default=[])
    args = ap.parse_args()

    text = open(args.raw, errors="replace").read()
    if "out of nodes" in text:
        print(f"{args.raw}: node-pool abort; refusing to write a tree", file=sys.stderr)
        return 2
    if "NO_CANONICAL_TREE" in text:
        print(f"{args.raw}: NO_CANONICAL_TREE; refusing to write a tree", file=sys.stderr)
        return 2

    lines = text.splitlines()
    body = [l for l in lines if TREE.match(l)]
    if not body:
        print(f"{args.raw}: no canonical tree lines found", file=sys.stderr)
        return 2

    reach = next((l for l in lines if l.startswith("REACH:")), None)
    build = next((l for l in lines if re.fullmatch(r"[0-9a-f]{64}", l.strip())), None)

    with open(args.out, "w") as out:
        out.write("# Canonical witness tree, produced by radio_canon_search_generic.\n")
        out.write(f"# command : {args.command}\n")
        out.write(f"# source  : {args.raw}\n")
        if build:
            out.write(f"# build_id: {build.strip()}\n")
        if reach:
            out.write(f"# search  : {reach}\n")
        for n in args.note:
            out.write(f"# note    : {n}\n")
        out.write("# format  : indented '<state> @k --[split]-->' / '<state> @k [canonical U_k]'\n")
        out.write("# checked : tools/check_witness.py\n")
        out.write("#\n")
        for l in body:
            out.write(l.rstrip() + "\n")

    print(f"{args.out}: {len(body)} tree lines")
    return 0


if __name__ == "__main__":
    sys.exit(main())
