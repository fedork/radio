#!/usr/bin/env python3
"""
Extract recursively checkable witness trees from radio solver output.

The script indexes `can solve Sb(...) in K with [...]` lines from either a raw
out.txt file or a zip containing one text file, then recursively proves a target
state using:

  1. exact logged solve lines;
  2. unit-group triviality, by deleting 1:1 parts when enabled;
  3. singleton terminals whose rows fit coordinatewise in distinct `G_k` rows;
  4. logged superstates with extra parts deleted, when enabled.

It deliberately does NOT rewrite `Sb(a:2)` as two copies of `Sb(a:1)`.
An arbitrary weak-majorization terminal is not a proof: the converse is false.  Such a state is
therefore expanded from logged evidence instead of being accepted as a terminal.

WHAT THIS IS NOT (measured 2026-09-03).  Its rendered output is a *third* format --
`- #N <state> in k: <status>` with `source:` lines -- and `tools/check_witness.py` auto-detects
only the canonical (`<state> @k --[split]-->`) and numbered (`N. (in k) (used r)`) forms.  So
nothing in the repo can verify what this script prints.  Use it to *navigate* a log and see why
the solver believed a state was solvable, never as evidence.

For a checkable log-derived proof use **tools/log_to_numbered_tree.py**, which emits the numbered
format that `check_witness.py` verifies: it discharges each child with `(line M)` whenever line
M's state dominates it after unit-group deletion, which is how a log actually carries the
evidence -- most children are never logged in their own right because something already logged
embeds them.  That tool built unconditional trees for all fifty `K=8` frontier cells `m=6..55`
straight out of `out_k8.txt`.  For a self-contained `G_k`-terminal proof instead, run
`radio_canon_search_generic` and convert with `tools/canon_out_to_tree.py`.
"""
from __future__ import annotations

import argparse
import io
import math
import os
import re
import sys
import zipfile
from collections import Counter, defaultdict
from dataclasses import dataclass, field
from functools import lru_cache
from typing import Dict, Iterable, Iterator, List, Optional, Sequence, Tuple

Part = Tuple[int, int]          # (n, m)
State = Tuple[Part, ...]        # canonical sorted normalized tuple

CAN_SOLVE_RE = re.compile(
    r"^(?:result in\s+\d+\s+)?can solve\s+(Sb\([^)]*\))\[(\d+),(\d+)\]\s+in\s+(\d+)\s+with\s+\[([^\]]*)\]\s+(.*?)\s+took\s+"
)
RESULT_RE = re.compile(
    r"^result in\s+(\d+)\s+can solve\s+(Sb\([^)]*\))\[(\d+),(\d+)\]\s+with\s+\[([^\]]*)\]\s+=>\s+(.*?)\s*$"
)
CHILD_RE = re.compile(r"Sb\([^)]*\)\[\d+,\d+\]")
PART_RE = re.compile(r"(\d+)\s*:\s*(\d+)")
SPLIT_RE = re.compile(r"^\s*(\d+)\s*:\s*(\d+)")


def parse_parts_from_state_text(text: str) -> Tuple[Part, ...]:
    """Parse `Sb(3:2,2:1)` or just `3:2,2:1` into ordered parts."""
    text = text.strip()
    if text.startswith("Sb("):
        assert text.endswith(")"), f"bad state text: {text!r}"
        text = text[3:-1]
    if not text:
        return tuple()
    parts: List[Part] = []
    for token in text.split(','):
        token = token.strip()
        if not token:
            continue
        m = PART_RE.fullmatch(token)
        if not m:
            raise ValueError(f"bad part token {token!r} in {text!r}")
        parts.append((int(m.group(1)), int(m.group(2))))
    return tuple(parts)


def normalize(parts: Iterable[Part], *, strip_units: bool = False) -> State:
    """Canonicalize a state as an unordered multiset of nonzero parts."""
    out: List[Part] = []
    for n, m in parts:
        if n == 0 or m == 0:
            continue
        # Solver output canonicalizes states of width/size 1 as (m:1).
        # This is NOT the invalid rewrite (a:2)->(a:1,a:1); it only handles
        # the special unit-size case 1:m -> m:1, observed directly in logs
        # such as 4:3 split by 1:1, where the mixed 1:2 appears as 2:1.
        if n == 1:
            n, m = m, 1
        if strip_units and n == 1 and m == 1:
            continue
        out.append((n, m))
    # Solver output is not always in a stable semantic order.  For lookup we use
    # multiset semantics: larger n first, then larger m.
    return tuple(sorted(out, key=lambda p: (-p[0], -p[1])))


def state_to_text(state: Sequence[Part]) -> str:
    if not state:
        return "Sb(0:0)"
    return "Sb(" + ",".join(f"{n}:{m}" for n, m in state) + ")"


def parse_split_token(token: str) -> Part:
    """Parse split token like `7:2:NOTFAST` or `1:1:NOTFAST-ADDED`."""
    m = SPLIT_RE.match(token)
    if not m:
        raise ValueError(f"bad split token {token!r}")
    return int(m.group(1)), int(m.group(2))


def parse_split_vector(text: str) -> Tuple[Part, ...]:
    if not text.strip():
        return tuple()
    return tuple(parse_split_token(tok) for tok in text.split(','))


def split_children(parent_parts: Sequence[Part], splits: Sequence[Part], *, strip_units: bool) -> Tuple[State, State, State]:
    """Compute the three child states induced by a solver split vector.

    The mixed branch order matches the observed solver convention:
      part n:m split x:y gives mixed pieces (n-x):y, x:(m-y).
    """
    if len(parent_parts) != len(splits):
        raise ValueError(f"parent/split length mismatch: {len(parent_parts)} vs {len(splits)}")
    b0: List[Part] = []
    b1: List[Part] = []
    b2: List[Part] = []
    for (n, m), (x, y) in zip(parent_parts, splits):
        b0.append((x, y))
        b1.append((n - x, y))
        b1.append((x, m - y))
        b2.append((n - x, m - y))
    return (
        normalize(b0, strip_units=strip_units),
        normalize(b1, strip_units=strip_units),
        normalize(b2, strip_units=strip_units),
    )


def base_sequence(k: int) -> List[int]:
    """Return G_k/U_k sequence: right binomial tail sums with dyadic repeats.

    Examples:
      G_3 = [8,7,4,4,1,1,1,1]
      G_4 starts [16,15,11,11,5,5,5,5,...]
      G_5 starts [32,31,26,26,16,16,16,16,...]
    """
    if k < 0:
        return []
    vals: List[int] = []
    for i in range(k + 1):
        value = sum(math.comb(k, j) for j in range(i, k + 1))
        reps = 1 if i == 0 else 2 ** (i - 1)
        vals.extend([value] * reps)
    return vals


def singleton_majorized(state: State, k: int) -> Tuple[bool, str]:
    if any(m != 1 for _, m in state):
        return False, "not singleton-only"
    rows = sorted([n for n, _ in state], reverse=True)
    g = base_sequence(k)
    if len(rows) > len(g):
        return False, f"too many singleton rows: {len(rows)} > {len(g)}"
    sr = 0
    sg = 0
    for i, n in enumerate(rows):
        sr += n
        sg += g[i]
        if sr > sg:
            return False, f"prefix {i+1}: {sr} > {sg}"
    return True, f"rows={rows}, G_{k} prefix={g[:len(rows)]}"



def same_m_dominance_slack(big: State, small: State) -> Optional[int]:
    """Return total slack if big safely dominates small by same-m monotonicity."""
    used = [False] * len(big)
    total = 0
    # Harder/larger target parts first.
    for n, m in sorted(small, key=lambda p: (-p[1], -p[0])):
        best = None
        best_slack = None
        for i, (N, M) in enumerate(big):
            if used[i] or M != m or N < n:
                continue
            slack = N - n
            if best is None or slack < best_slack:
                best = i
                best_slack = slack
        if best is None:
            return None
        used[best] = True
        total += int(best_slack)
    return total


def same_m_dominates(big: State, small: State) -> bool:
    return same_m_dominance_slack(big, small) is not None

def counter_subset(small: Sequence[Part], big: Sequence[Part]) -> bool:
    return not (Counter(small) - Counter(big))


def choose_subsequence_indices(big_parts: Sequence[Part], target_state: State) -> Optional[List[int]]:
    """Find indices in ordered big_parts whose multiset equals target_state."""
    need = Counter(target_state)
    indices: List[int] = []
    for i, part in enumerate(big_parts):
        if need[part] > 0:
            indices.append(i)
            need[part] -= 1
    if all(v == 0 for v in need.values()):
        return indices
    return None


@dataclass(frozen=True)
class Evidence:
    line_no: int
    raw_line: str
    parent_parts: Tuple[Part, ...]       # original solver order, zero-free parent
    parent_key: State                    # normalized parent, no unit stripping applied here
    k: int
    splits: Tuple[Part, ...]
    children: Tuple[State, State, State] # normalized children under extractor's strip setting
    kind: str = "exact"
    restricted_from: Optional[int] = None


@dataclass
class Proof:
    state: State
    k: int
    status: str
    detail: str = ""
    evidence: Optional[Evidence] = None
    children: List['Proof'] = field(default_factory=list)
    ref_id: Optional[int] = None


class WitnessExtractor:
    def __init__(self, path: str, *, strip_units: bool = True, allow_superstate: bool = True, max_superstate_extra: int = 4):
        self.path = path
        self.strip_units = strip_units
        self.allow_superstate = allow_superstate
        self.max_superstate_extra = max_superstate_extra
        self.exact: Dict[Tuple[State, int], List[Evidence]] = defaultdict(list)
        self.by_k: Dict[int, List[Evidence]] = defaultdict(list)
        self.part_index: Dict[Tuple[int, Part], List[Evidence]] = defaultdict(list)
        self.m_index: Dict[Tuple[int, int], List[Evidence]] = defaultdict(list)
        self._memo: Dict[Tuple[State, int], Proof] = {}
        self._active: set[Tuple[State, int]] = set()

    def iter_lines(self) -> Iterator[Tuple[int, str]]:
        if zipfile.is_zipfile(self.path):
            with zipfile.ZipFile(self.path) as zf:
                names = [n for n in zf.namelist() if not n.endswith('/')]
                if not names:
                    return
                # Prefer .txt; otherwise use first file.
                name = next((n for n in names if n.lower().endswith('.txt')), names[0])
                with zf.open(name, 'r') as raw:
                    wrapper = io.TextIOWrapper(raw, encoding='utf-8', errors='replace', newline='')
                    for i, line in enumerate(wrapper, 1):
                        yield i, line.rstrip('\n')
        else:
            with open(self.path, 'r', encoding='utf-8', errors='replace') as f:
                for i, line in enumerate(f, 1):
                    yield i, line.rstrip('\n')

    def parse_evidence_line(self, line_no: int, line: str) -> Optional[Evidence]:
        m = CAN_SOLVE_RE.match(line)
        if m:
            parent_text = m.group(1)
            k = int(m.group(4))
            split_text = m.group(5)
        else:
            r = RESULT_RE.match(line)
            if not r:
                return None
            k = int(r.group(1))
            parent_text = r.group(2)
            split_text = r.group(5)
        parent_parts_raw = parse_parts_from_state_text(parent_text)
        parent_parts = tuple((n, mm) for n, mm in parent_parts_raw if n != 0 and mm != 0)
        splits = parse_split_vector(split_text)
        if len(parent_parts_raw) != len(splits):
            # Parent solve lines should not contain 0:0 parts; keep this explicit.
            return None
        parent_key = normalize(parent_parts, strip_units=False)
        children = split_children(parent_parts_raw, splits, strip_units=self.strip_units)
        return Evidence(
            line_no=line_no,
            raw_line=line,
            parent_parts=parent_parts,
            parent_key=parent_key,
            k=k,
            splits=splits,
            children=children,
        )

    def load_index(self) -> None:
        count = 0
        for line_no, line in self.iter_lines():
            ev = self.parse_evidence_line(line_no, line)
            if ev is None:
                continue
            self.exact[(ev.parent_key, ev.k)].append(ev)
            self.by_k[ev.k].append(ev)
            for part in set(ev.parent_key):
                self.part_index[(ev.k, part)].append(ev)
            for mm in {m for _, m in ev.parent_key}:
                self.m_index[(ev.k, mm)].append(ev)
            count += 1
        print(f"indexed {count} can-solve lines", file=sys.stderr)

    def restrict_evidence(self, ev: Evidence, target: State) -> Optional[Evidence]:
        """Restrict a logged superstate solve line by deleting extra parent parts.

        This is valid only when target is an exact submultiset of ev.parent_parts.
        It does not perform arbitrary numeric domination.
        """
        if not counter_subset(target, ev.parent_key):
            return None
        extra = len(ev.parent_key) - len(target)
        if extra <= 0 or extra > self.max_superstate_extra:
            return None
        indices = choose_subsequence_indices(ev.parent_parts, target)
        if indices is None:
            return None
        parent_parts = tuple(ev.parent_parts[i] for i in indices)
        splits = tuple(ev.splits[i] for i in indices)
        children = split_children(parent_parts, splits, strip_units=self.strip_units)
        return Evidence(
            line_no=ev.line_no,
            raw_line=ev.raw_line,
            parent_parts=parent_parts,
            parent_key=target,
            k=ev.k,
            splits=splits,
            children=children,
            kind="superstate-delete-extra-parts",
            restricted_from=ev.line_no,
        )

    def candidate_dominators(self, state: State, k: int) -> Iterator[Evidence]:
        """Yield logged states that safely dominate target by same-m monotonicity."""
        if not state:
            return
        # Pick a multiplicity class that must be present; this is much cheaper
        # than scanning every K-level solve line.
        m_lists = [(len(self.m_index.get((k, m), [])), m) for _, m in set(state)]
        if not m_lists:
            return
        _, pivot_m = min(m_lists)
        candidates = []
        for ev in self.m_index.get((k, pivot_m), []):
            if ev.parent_key == state:
                continue
            extra = len(ev.parent_key) - len(state)
            if extra < 0 or extra > self.max_superstate_extra:
                continue
            slack = same_m_dominance_slack(ev.parent_key, state)
            if slack is not None:
                candidates.append((extra, slack, ev.line_no, ev))
        candidates.sort(key=lambda t: (t[0], t[1], t[2]))
        for _, _, _, ev in candidates[:200]:
            yield ev

    def candidate_evidences(self, state: State, k: int) -> Iterator[Evidence]:
        # Exact lines first. Prefer full-solve `result in ...` witnesses,
        # then non-fast standalone lines. This avoids getting stuck first on
        # a logged but awkward fast_solve branch when a cleaner full-solve
        # witness is available for the same state.
        exacts = list(self.exact.get((state, k), []))
        exacts.sort(key=lambda ev: (
            0 if ev.raw_line.startswith("result in") else 1,
            0 if "fast_solve=0" in ev.raw_line else 1,
            ev.line_no,
        ))
        yield from exacts
        if not self.allow_superstate:
            return
        # Then logged superstates containing the target as a submultiset.
        # Use an inverted index over parent parts; scanning all K-level evidence
        # lines is too slow for large trees.
        if not state:
            return
        # Choose the rarest required part among this K's evidence lines.
        part_lists = [(len(self.part_index.get((k, part), [])), part) for part in set(state)]
        if not part_lists:
            return
        _, pivot = min(part_lists)
        for ev in self.part_index.get((k, pivot), []):
            restricted = self.restrict_evidence(ev, state)
            if restricted is not None:
                yield restricted

    def prove(self, parts_or_state: Sequence[Part], k: int) -> Proof:
        state = normalize(parts_or_state, strip_units=self.strip_units)
        key = (state, k)
        if key in self._memo:
            cached = self._memo[key]
            if cached.status in ("failed", "cycle"):
                return Proof(state=state, k=k, status=cached.status, detail="cached: " + cached.detail, ref_id=id(cached))
            return Proof(state=state, k=k, status="see-earlier", ref_id=id(cached))
        if key in self._active:
            return Proof(state=state, k=k, status="cycle", detail="recursive cycle avoided")
        self._active.add(key)
        proof = self._prove_uncached(state, k)
        self._active.remove(key)
        self._memo[key] = proof
        return proof

    def _prove_uncached(self, state: State, k: int) -> Proof:
        if not state:
            return Proof(state=state, k=k, status="terminal", detail="empty/zero state")
        if k < 0:
            return Proof(state=state, k=k, status="failed", detail="negative test budget")
        if self.strip_units:
            # normalize has already stripped units.  If state became empty, handled above.
            pass
        ok, why = singleton_majorized(state, k)
        unsupported_majorization = ""
        if ok:
            rows = sorted([n for n, _ in state], reverse=True)
            g = base_sequence(k)
            embedded = all(n <= slot for n, slot in zip(rows, g))
            if embedded:
                return Proof(state=state, k=k, status="terminal",
                             detail="distinct-slot embedding: " + why)
            unsupported_majorization = why
        if k == 0:
            # Only empty or unit-stripped states should survive at k=0.
            return Proof(state=state, k=k, status="failed", detail="nonterminal at k=0")

        failures: List[str] = []
        for ev in self.candidate_evidences(state, k):
            child_proofs = [self.prove(child, k - 1) for child in ev.children]
            if all(cp.status not in ("failed", "cycle") for cp in child_proofs):
                detail = f"{ev.kind} line {ev.line_no}, split [{','.join(f'{x}:{y}' for x,y in ev.splits)}]"
                if ev.kind != "exact":
                    detail += f"; restricted from logged superstate on line {ev.restricted_from}"
                return Proof(state=state, k=k, status="proved", detail=detail, evidence=ev,
                             children=child_proofs)
            failures.append(f"line {ev.line_no}: " + "; ".join(f"child {i} {cp.status}" for i, cp in enumerate(child_proofs)))

        # Last resort: safe monotonicity with identical multiplicities.
        # This allows, for example, Sb(58:3,35:1) <= Sb(58:3,52:1),
        # but does NOT allow Sb(17:2) <= Sb(19:1,18:1).
        for ev in self.candidate_dominators(state, k):
            # Treat safe same-m monotonicity as a terminal trivial implication.
            # We do not expand the dominating tree here; the logged line itself
            # certifies the larger state, and the same-m map cannot make the
            # target harder.
            detail = f"terminal safe same-m dominance by line {ev.line_no}: {state_to_text(state)} <= {state_to_text(ev.parent_key)}"
            return Proof(state=state, k=k, status="proved", detail=detail, evidence=ev, children=[])

        detail = "no exact/superstate evidence whose children all prove"
        if unsupported_majorization:
            detail += "; weak majorization holds but is not a certificate: " + unsupported_majorization
        if failures:
            detail += "; tried " + " | ".join(failures[:5])
            if len(failures) > 5:
                detail += f" | ... {len(failures)-5} more"
        return Proof(state=state, k=k, status="failed", detail=detail)


def render_proof(proof: Proof, *, max_line_len: int = 220, _seen: Optional[Dict[int, int]] = None, _depth: int = 0) -> str:
    if _seen is None:
        _seen = {}
    indent = "  " * _depth
    lines: List[str] = []
    obj_id = id(proof)
    if proof.status == "see-earlier" and proof.ref_id is not None:
        lines.append(f"{indent}- {state_to_text(proof.state)} in {proof.k}: see earlier")
        return "\n".join(lines)
    if obj_id in _seen:
        lines.append(f"{indent}- {state_to_text(proof.state)} in {proof.k}: see node #{_seen[obj_id]}")
        return "\n".join(lines)
    node_no = len(_seen) + 1
    _seen[obj_id] = node_no

    head = f"{indent}- #{node_no} {state_to_text(proof.state)} in {proof.k}: {proof.status}"
    if proof.detail:
        head += f" ({proof.detail})"
    lines.append(head)
    if proof.evidence is not None:
        raw = proof.evidence.raw_line
        if len(raw) > max_line_len:
            raw = raw[: max_line_len - 3] + "..."
        lines.append(f"{indent}  source: {raw}")
    for child in proof.children:
        lines.append(render_proof(child, max_line_len=max_line_len, _seen=_seen, _depth=_depth + 1))
    return "\n".join(lines)


def main(argv: Optional[Sequence[str]] = None) -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("output", help="radio output .txt or .zip")
    p.add_argument("--target", required=True, help="target state, e.g. 'Sb(189:10)' or '17:2,32:1,31:1'")
    p.add_argument("--k", required=True, type=int, help="test budget for target")
    p.add_argument("--no-strip-units", action="store_true", help="do not delete 1:1 unit groups before lookup")
    p.add_argument("--no-superstate", action="store_true", help="do not use logged superstates with extra parts deleted")
    p.add_argument("--max-superstate-extra", type=int, default=4, help="max number of extra parts allowed in superstate restriction")
    p.add_argument("--out", help="write rendered tree to this file")
    p.add_argument("--max-line-len", type=int, default=240, help="truncate raw source lines in rendered output")
    args = p.parse_args(argv)

    target_parts = parse_parts_from_state_text(args.target)
    ex = WitnessExtractor(
        args.output,
        strip_units=not args.no_strip_units,
        allow_superstate=not args.no_superstate,
        max_superstate_extra=args.max_superstate_extra,
    )
    ex.load_index()
    proof = ex.prove(target_parts, args.k)
    rendered = render_proof(proof, max_line_len=args.max_line_len)
    if args.out:
        with open(args.out, 'w', encoding='utf-8') as f:
            f.write(rendered + "\n")
    else:
        print(rendered)
    return 0 if proof.status in ("proved", "terminal") else 2


if __name__ == "__main__":
    raise SystemExit(main())
