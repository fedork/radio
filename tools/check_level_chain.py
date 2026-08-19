#!/usr/bin/env python3
"""Check that a set of level-v2 certificates forms a closed, self-contained proof chain.

The frozen refuter checks the *semantics* of a certificate: that every claim really is refuted by
the support it ships with.  This tool checks the *structure*, without a solver, and the two together
are what make a chain usable as the certificate of record:

  1. Internal consistency of each level -- every declared count matches the records actually
     present, every part index is in range, and every reference total adds up.
  2. Inductive closure -- level k's support set is exactly level (k-1)'s claim set.  Equal *counts*
     are not enough; the sets are compared as resolved states.
  3. A base level whose support is empty, so the induction terminates rather than dangling.
  4. Optionally, that the top level's claims are exactly the expected target roots.

Point 2 is the one that matters and the one that is easy to get wrong: `part`, `fact`, `claim` and
`split` records are indices into *each file's own* part dictionary, and a trimmed certificate carries
a smaller dictionary than the complete one, so indices are renumbered between files.  Comparing raw
index lines across certificates reports differences that are not there.  Everything here resolves
indices to `n:m` values first.

A chain that passes has no dangling reference at any level: each claim is refuted using facts that
are themselves claims proved one level down, terminating at a level proved outright.

Usage:
    tools/check_level_chain.py CERT...
    tools/check_level_chain.py --expect-top-sum 193 run9-k*.cert
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

Part = tuple[int, int]
State = tuple[Part, ...]


class Level:
    def __init__(self, path: Path):
        self.path = path
        self.level: int | None = None
        self.support_level: int | None = None
        self.parts: dict[int, Part] = {}
        self.claims: set[State] = set()
        self.facts: set[State] = set()
        self.declared: dict[str, tuple[int, ...]] = {}
        self.claim_records = 0
        self.fact_records = 0
        self.split_records = 0
        self.claim_refs = 0
        self.fact_refs = 0
        self._parse()

    def _state(self, fields: list[str], what: str, lineno: int) -> State:
        out = []
        for token in fields:
            if not token.isdigit():
                raise ValueError(f"{self.path}:{lineno}: {what} index {token!r} is not a number")
            index = int(token)
            if index not in self.parts:
                raise ValueError(f"{self.path}:{lineno}: {what} index {index} not in part table")
            out.append(self.parts[index])
        if not out:
            raise ValueError(f"{self.path}:{lineno}: empty {what}")
        return tuple(sorted(out))

    def _parse(self) -> None:
        header_seen = False
        for lineno, raw in enumerate(self.path.read_text().splitlines(), 1):
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            fields = line.split()
            tag = fields[0]
            if not header_seen:
                if line != "radio-negative-level-certificate-v2":
                    raise ValueError(f"{self.path}:{lineno}: missing v2 header")
                header_seen = True
                continue
            if tag == "level":
                self.level = int(fields[1])
            elif tag == "parts":
                self.declared["parts"] = (int(fields[1]),)
            elif tag == "part":
                index = int(fields[1])
                n, m = (int(x) for x in fields[2].split(":"))
                if index in self.parts:
                    raise ValueError(f"{self.path}:{lineno}: duplicate part index {index}")
                self.parts[index] = (n, m) if n >= m else (m, n)
            elif tag == "support":
                self.support_level = int(fields[1])
                self.declared["support"] = (int(fields[2]), int(fields[3]))
            elif tag == "fact":
                state = self._state(fields[1:], "fact", lineno)
                self.facts.add(state)
                self.fact_records += 1
                self.fact_refs += len(state)
            elif tag == "split-hints":
                self.declared["split-hints"] = (int(fields[1]),)
            elif tag == "split":
                self.split_records += 1
            elif tag == "claims":
                if int(fields[1]) != self.level:
                    raise ValueError(f"{self.path}:{lineno}: claims level {fields[1]} != {self.level}")
                self.declared["claims"] = (int(fields[2]), int(fields[3]))
            elif tag == "claim":
                state = self._state(fields[1:], "claim", lineno)
                self.claims.add(state)
                self.claim_records += 1
                self.claim_refs += len(state)
            else:
                raise ValueError(f"{self.path}:{lineno}: unknown record {tag!r}")
        if self.level is None:
            raise ValueError(f"{self.path}: no level record")
        if self.support_level != self.level - 1:
            raise ValueError(
                f"{self.path}: support level {self.support_level} is not {self.level - 1}"
            )

    def check_internal(self) -> list[str]:
        problems = []
        if self.declared.get("parts", (None,))[0] != len(self.parts):
            problems.append(f"declared parts {self.declared.get('parts')} != {len(self.parts)}")
        if sorted(self.parts) != list(range(1, len(self.parts) + 1)):
            problems.append("part indices are not 1..N without gaps")
        count, refs = self.declared.get("support", (0, 0))
        if count != self.fact_records:
            problems.append(f"declared support {count} != {self.fact_records} fact records")
        if refs != self.fact_refs:
            problems.append(f"declared support refs {refs} != {self.fact_refs}")
        if len(self.facts) != self.fact_records:
            problems.append(f"{self.fact_records - len(self.facts)} duplicate support facts")
        count, refs = self.declared.get("claims", (0, 0))
        if count != self.claim_records:
            problems.append(f"declared claims {count} != {self.claim_records} claim records")
        if refs != self.claim_refs:
            problems.append(f"declared claim refs {refs} != {self.claim_refs}")
        if len(self.claims) != self.claim_records:
            problems.append(f"{self.claim_records - len(self.claims)} duplicate claims")
        hints = self.declared.get("split-hints", (0,))[0]
        if hints != self.split_records:
            problems.append(f"declared split-hints {hints} != {self.split_records}")
        return problems


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("certs", type=Path, nargs="+")
    parser.add_argument("--expect-top-sum", type=int,
                        help="require every top-level claim to be one part with n+m equal to this")
    args = parser.parse_args()

    levels: dict[int, Level] = {}
    for path in args.certs:
        try:
            level = Level(path)
        except ValueError as exc:
            print(f"FAIL {exc}", file=sys.stderr)
            return 1
        if level.level in levels:
            print(f"FAIL two certificates for level {level.level}", file=sys.stderr)
            return 1
        levels[level.level] = level

    ordered = sorted(levels)
    if ordered != list(range(min(ordered), max(ordered) + 1)):
        print(f"FAIL levels {ordered} are not contiguous", file=sys.stderr)
        return 1

    failures = 0
    print(f"{'level':>5} {'parts':>7} {'support':>9} {'claims':>9}  internal")
    for k in ordered:
        lv = levels[k]
        problems = lv.check_internal()
        failures += len(problems)
        print(f"{k:5d} {len(lv.parts):7d} {lv.fact_records:9d} {lv.claim_records:9d}  "
              f"{'OK' if not problems else '; '.join(problems)}")

    print()
    print("inductive closure: level k support == level k-1 claims, as resolved states")
    base = min(ordered)
    for k in ordered:
        lv = levels[k]
        if k == base:
            if lv.facts:
                print(f"  level {k}: FAIL base level carries {len(lv.facts)} support facts, "
                      f"so the induction dangles")
                failures += 1
            else:
                print(f"  level {k}: base case, no support -- induction terminates")
            continue
        lower = levels[k - 1].claims
        extra = lv.facts - lower
        missing = lower - lv.facts
        if extra or missing:
            print(f"  level {k}: FAIL support-only {len(extra)}, claims-only {len(missing)}")
            for state in list(extra)[:3]:
                print(f"      support fact not proved below: {state}")
            failures += 1
        else:
            print(f"  level {k}: OK {len(lv.facts)} facts all proved at level {k - 1}")

    top = levels[max(ordered)]
    if args.expect_top_sum is not None:
        bad = [s for s in top.claims
               if len(s) != 1 or s[0][0] + s[0][1] != args.expect_top_sum]
        if bad:
            print(f"\ntop level: FAIL {len(bad)} claims are not a single part summing to "
                  f"{args.expect_top_sum}")
            failures += 1
        else:
            print(f"\ntop level {max(ordered)}: OK all {len(top.claims)} claims are single parts "
                  f"summing to {args.expect_top_sum}")

    total = sum(levels[k].claim_records for k in ordered)
    print(f"\ntotal claims across the chain: {total}")
    if failures:
        print(f"\n{failures} FAILURE(S)", file=sys.stderr)
        return 1
    print("\nchain is internally consistent, inductively closed and terminating")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
