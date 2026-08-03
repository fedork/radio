#!/usr/bin/env python3
"""Structural checks on the documentation, so a cold-start read cannot silently rot.

The next session sees only AGENTS.md and whatever it links. This verifies that the entry
path actually works and that the pieces have not drifted apart. It cannot check whether a
sentence is *true* - that is what `check_tables.py` and `check_witness.py` are for, and what
the session-end protocol asks a human-or-agent to do by hand.

Checks:

  entry path    AGENTS.md exists, CLAUDE.md points at it (symlink or one-line reference)
                rather than duplicating it, and AGENTS.md names status.md as the first read.
  links         every relative markdown link resolves.
  goal coverage every H-goal in research-plan.md appears in status.md, so a goal cannot be
                added or retired in one place only.
  freshness     status.md carries a "Last refreshed" date, and it is not older than the last
                journal entry - if the journal moved on and status.md did not, the cold-start
                snapshot is stale.
  traps         status.md has a non-empty trap table; these are the items that silently
                invalidate work and the thing most worth not losing.
  tools         every tools/*.py or .sh mentioned in the docs exists.

Usage:  tools/check_docs.py
"""
from __future__ import annotations

import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), ".."))


def read(p: str) -> str:
    with open(os.path.join(ROOT, p)) as fh:
        return fh.read()


def prose(text: str) -> str:
    """Strip fenced code blocks and inline code.

    Witness-tree samples contain things like `Sa(112)[6216,112](line 2)`, which a naive
    link regex reads as a markdown link to a file named "line 2".
    """
    text = re.sub(r"```.*?```", "", text, flags=re.S)
    return re.sub(r"`[^`\n]*`", "", text)


def main() -> int:
    errs: list[str] = []
    warns: list[str] = []

    # --- entry path ---------------------------------------------------------------
    if not os.path.exists(os.path.join(ROOT, "AGENTS.md")):
        errs.append("AGENTS.md is missing - it is the vendor-neutral canonical brief")
        return report(errs, warns)
    agents = read("AGENTS.md")

    claude = os.path.join(ROOT, "CLAUDE.md")
    if os.path.exists(claude):
        if os.path.islink(claude):
            if os.readlink(claude) != "AGENTS.md":
                errs.append(f"CLAUDE.md symlinks to {os.readlink(claude)!r}, expected AGENTS.md")
        else:
            body = read("CLAUDE.md")
            if "AGENTS.md" not in body:
                errs.append("CLAUDE.md neither links to nor references AGENTS.md")
            elif len(body.splitlines()) > 10:
                errs.append("CLAUDE.md looks like a *copy* of AGENTS.md rather than a pointer; "
                            "copies diverge - use a symlink or a one-line reference")

    for must in ("docs/status.md", "docs/research-plan.md"):
        if must not in agents:
            errs.append(f"AGENTS.md does not point at {must}")
    if "Start here" not in agents:
        warns.append("AGENTS.md has no 'Start here' section; a cold reader has no entry order")
    if "Before you finish" not in agents:
        errs.append("AGENTS.md has no session-end protocol; status.md and the journal will rot")

    # --- links -------------------------------------------------------------------
    nlinks = 0
    for dirpath, dirnames, filenames in os.walk(ROOT):
        dirnames[:] = [d for d in dirnames
                       if d not in {".git", ".venv", ".gh", "__pycache__", "data"}]
        for fn in filenames:
            if not fn.endswith(".md"):
                continue
            rel = os.path.relpath(os.path.join(dirpath, fn), ROOT)
            for m in re.finditer(r"\[[^\]]+\]\(([^)]+)\)", prose(read(rel))):
                t = m.group(1).split("#")[0].strip()
                if not t or t.startswith(("http://", "https://", "mailto:")):
                    continue
                nlinks += 1
                if not os.path.exists(os.path.normpath(
                        os.path.join(ROOT, os.path.dirname(rel), t))):
                    errs.append(f"{rel}: broken link -> {t}")

    # --- goal coverage -----------------------------------------------------------
    plan = read("docs/research-plan.md")
    status = read("docs/status.md")
    goals = set(re.findall(r"\*\*(H\d)\b", plan))
    if not goals:
        warns.append("no H-goals found in research-plan.md")
    for g in sorted(goals):
        if not re.search(rf"\b{g}\b", status):
            errs.append(f"goal {g} is in research-plan.md but not in status.md")

    # --- freshness ---------------------------------------------------------------
    m = re.search(r"Last refreshed\s+\*\*(\d{4}-\d{2}-\d{2})\*\*", status)
    if not m:
        errs.append("status.md has no 'Last refreshed **YYYY-MM-DD**' marker")
    else:
        stamp = m.group(1)
        jdates = re.findall(r"^## (\d{4}-\d{2}-\d{2})", read("docs/journal.md"), re.M)
        if jdates and max(jdates) > stamp:
            errs.append(f"status.md refreshed {stamp} but journal.md has a later entry "
                        f"{max(jdates)} - the cold-start snapshot is stale")

    # --- traps -------------------------------------------------------------------
    tm = re.search(r"## Active traps(.*?)^## ", status, re.S | re.M)
    if not tm:
        errs.append("status.md has no '## Active traps' section")
    elif tm.group(1).count("|") < 12:
        errs.append("status.md trap table looks empty")

    # --- status.md's counts must match the machine-readable sources ---------------
    # Freshness alone is too weak: bumping the date without updating the content would
    # pass. These are the numbers most likely to drift.
    import csv
    ntrees = len([f for f in os.listdir(os.path.join(ROOT, "witnesses"))
                  if f.endswith(".tree")])
    m = re.search(r"\*\*(\d+) verified witness trees\*\*", status)
    if not m:
        warns.append("status.md does not state a verified-witness-tree count")
    elif int(m.group(1)) != ntrees:
        errs.append(f"status.md says {m.group(1)} witness trees, witnesses/ has {ntrees}")

    apath = os.path.join(ROOT, "data", "artifacts.csv")
    if os.path.exists(apath):
        rows = list(csv.DictReader(open(apath)))
        ntags, nassets = len({r["tag"] for r in rows}), len(rows)
        m = re.search(r"(\d+) tags, (\d+) assets", status)
        if not m:
            warns.append("status.md does not state the store's tag/asset counts")
        elif (int(m.group(1)), int(m.group(2))) != (ntags, nassets):
            errs.append(f"status.md says {m.group(1)} tags / {m.group(2)} assets; "
                        f"data/artifacts.csv has {ntags} / {nassets}")

    npos = len([r for r in csv.DictReader(open(os.path.join(ROOT, "data", "exhaustive_multipart.csv")))])
    m = re.search(r"\*\*(\d+) exhaustive multi-part enumerations\*\*", status)
    if m and int(m.group(1)) != npos:
        errs.append(f"status.md says {m.group(1)} multi-part enumerations, "
                    f"data/exhaustive_multipart.csv has {npos}")

    # --- tools referenced actually exist -----------------------------------------
    for doc in ["AGENTS.md"] + [os.path.join("docs", f) for f in os.listdir(os.path.join(ROOT, "docs"))
                                if f.endswith(".md")]:
        for t in set(re.findall(r"tools/[A-Za-z0-9_.-]+\.(?:py|sh)", read(doc))):
            if not os.path.exists(os.path.join(ROOT, t)):
                errs.append(f"{doc}: references missing {t}")

    print(f"entry path     AGENTS.md + {'symlink' if os.path.islink(claude) else 'pointer'}")
    print(f"links checked  {nlinks}")
    print(f"goals covered  {len(goals)} ({', '.join(sorted(goals))})")
    return report(errs, warns)


def report(errs, warns) -> int:
    for w in warns:
        print(f"  WARN  {w}")
    for e in errs:
        print(f"  FAIL  {e}")
    print("\n" + ("docs consistent" if not errs else f"{len(errs)} FAILURE(S)"))
    return 0 if not errs else 1


if __name__ == "__main__":
    sys.exit(main())
