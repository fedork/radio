#!/usr/bin/env python3
"""Independently verify radioactive-coins witness trees.

A witness tree claims "state S is solvable in k tests". This checker re-derives every
step from first principles and never consults the solver, so a tree that passes here is
a proof you can trust without trusting radiobase.c.

Two input formats are auto-detected:

  canonical  - output of radio_canon_search_generic or search_singletonization
               `<state> @k --[split]-->` with three indented children, terminating in
               `<state> @k [canonical U_k]` leaves (atom sub-multisets) or
               `<state> @k [majorized G_k]` leaves (arbitrary singleton sequences).

  numbered   - output of radio_print.c
               `N. (in k) (used r) <state> take[...]:` followed by three
               `<digit>=><child>(line M|trivial)` lines. A `(line M)` reference means the
               child is solved by the strategy proved at line M.

What is checked, in both formats:

  * split derivation - the three children are exactly what the recorded test produces.
    Testing `a:b` against a part `n1:n2` yields
        outcome 2 (both defectives taken)  -> a:b
        outcome 0 (neither taken)          -> (n1-a):(n2-b)
        outcome 1 (exactly one taken)      -> a:(n2-b) and (n1-a):b     [doubles the parts]
  * depth - every child sits at exactly k-1.
  * information bound - mass(S) <= 3^k at every node, since k ternary tests have 3^k leaves.

Format-specific:

  canonical  * every leaf is a singleton state certified either as a sub-multiset of
               G_k or by direct weak majorization against G_k.  In both cases the
               Singleton Majorization Theorem makes the tree a self-contained proof.

  numbered   * a `(line M)` reference is legal iff the state at line M dominates the child
               after deleting unit groups (1:1), by the Unit-Group Elimination Theorem
               (docs/theorems/unit-group-elimination.md). Dominance = an injection from
               child parts to reference parts that is componentwise >=.
             * `trivial` leaves really are trivial, `(used r)` counts match, and every
               numbered line is reachable from the root.

Usage:  tools/check_witness.py witnesses/*.tree
Exit status is nonzero if any tree fails.
"""
from __future__ import annotations

import re
import sys
from collections import Counter
from typing import Dict, List, Optional, Sequence, Tuple

Part = Tuple[int, int]          # (n1, n2), always stored with n1 >= n2
State = Tuple[Part, ...]


# --------------------------------------------------------------------------- helpers

def singleton_base(k: int) -> List[int]:
    """G_k, the maximal singleton state solvable in k tests.

    G_0 = (1); G_k = sort(L + M + R) where L, M, R are the three zero-padded copies of
    G_{k-1} described in the Singleton Majorization Theorem. Equivalently the atom in
    dyadic block r is sum_{i=0}^{k-r} C(k,i); the recurrence is used here so the checker
    does not depend on that identity.
    """
    cur = [1]
    for _ in range(k):
        nxt = [0] * (2 * len(cur))
        for i, h in enumerate(cur):
            nxt[i] += h
            nxt[2 * i] += h
            nxt[2 * i + 1] += h
        cur = sorted(nxt, reverse=True)
    return cur


def normalize(parts: Sequence[Part]) -> State:
    """Canonical form for *comparing* states: drop empty parts, sort descending.

    Only ever apply this when comparing two states. A parent's parts must stay in printed
    order everywhere else, because the split vector is positional against them.
    """
    out = [(max(a, b), min(a, b)) for a, b in parts if a and b]
    return tuple(sorted(out, reverse=True))


def mass(parts: Sequence[Part]) -> int:
    """Number of candidate defective pairs the state still admits."""
    return sum(a * b for a, b in parts)


def children_of(parts: Sequence[Part], split: Sequence[Part]) -> Tuple[State, State, State]:
    """The three substates produced by one test. Returns (both, mixed, neither)."""
    both: List[Part] = []
    mixed: List[Part] = []
    neither: List[Part] = []
    for (n1, n2), (a, b) in zip(parts, split):
        if a > n1 or b > n2:            # the split may be written in the other orientation
            a, b = b, a
        if not (0 <= a <= n1 and 0 <= b <= n2):
            raise ValueError(f"split {a}:{b} out of range for part {n1}:{n2}")
        both.append((a, b))
        neither.append((n1 - a, n2 - b))
        mixed += [(a, n2 - b), (n1 - a, b)]
    return normalize(both), normalize(mixed), normalize(neither)


def strip_units(state: State) -> State:
    """Delete unit groups (1:1). Unit-Group Elimination Theorem: they cost one leaf of
    capacity each but impose no structural constraint, so they never affect dominance."""
    return tuple(p for p in state if p != (1, 1))


def dominates(big: State, small: State) -> bool:
    """True if every part of `small` can be injectively matched to a componentwise-larger
    part of `big`. Then any strategy for `big` also solves `small`."""
    if len(small) > len(big):
        return False
    adj = [[j for j, b in enumerate(big) if b[0] >= s[0] and b[1] >= s[1]] for s in small]
    match: List[int] = [-1] * len(big)

    def augment(i: int, seen: set) -> bool:
        # `seen` is shared across the whole augmenting search, not per call: that is what
        # bounds the recursion to one visit per right-hand vertex.
        for j in adj[i]:
            if j in seen:
                continue
            seen.add(j)
            if match[j] == -1 or augment(match[j], seen):
                match[j] = i
                return True
        return False

    return all(augment(i, set()) for i in range(len(small)))


def parse_state(text: str) -> State:
    """`Sb(3:2,2:1)`, `Sa(8)` or bare `3:2,2:1` -> parts in *printed order*.

    Each part is oriented n1 >= n2 (a no-op for solver output, which always is), but the
    sequence is left exactly as printed and empty parts are kept, because take[]/split
    vectors are positional against it. Use normalize() to compare two states.
    """
    text = text.strip()
    if text.startswith(("Sa(", "Sb(")):
        text = text[3:text.rindex(")")]
    out: List[Part] = []
    for tok in text.split(","):
        tok = tok.strip()
        if not tok:
            continue
        a, b = (int(x) for x in tok.split(":"))
        out.append((max(a, b), min(a, b)))
    return tuple(out)


def parse_split(text: str) -> List[Part]:
    """Split vectors are positional, so `x:0` entries must be kept."""
    return [tuple(int(x) for x in tok.split(":")) for tok in text.split(",") if tok.strip()]


# ------------------------------------------------------------------- canonical format

CANON = re.compile(
    r"^\s*(.*?)\s+@(\d+)\s+(?:\[(canonical U|majorized G)_(\d+)\]|--\[(.*?)\]-->)\s*$"
)


class CanonNode:
    __slots__ = ("state", "k", "terminal", "stopk", "split", "text", "kids")

    def __init__(self, state, k, terminal, stopk, split, text):
        self.state, self.k, self.terminal, self.stopk = state, k, terminal, stopk
        self.split, self.text = split, text
        self.kids: List["CanonNode"] = []


def check_canonical(lines: List[str], errs: List[str]) -> str:
    seq = []
    for line in lines:
        m = CANON.match(line)
        if m:
            seq.append((parse_state(m.group(1)), int(m.group(2)),
                        m.group(3), int(m.group(4)) if m.group(4) else None,
                        parse_split(m.group(5)) if m.group(5) else None,
                        m.group(1).strip()))
    if not seq:
        errs.append("no parseable nodes")
        return "0 nodes"

    # Rebuild from preorder + arity rather than indentation: a split node is followed by
    # exactly its three subtrees, a canonical node by nothing. This survives the
    # indentation loss that happens when a tree is pasted into a spreadsheet.
    pos = 0

    def build() -> Optional[CanonNode]:
        nonlocal pos
        if pos >= len(seq):
            errs.append("tree truncated: ran out of nodes")
            return None
        state, k, terminal, stopk, split, text = seq[pos]
        pos += 1
        node = CanonNode(state, k, terminal, stopk, split, text)
        if terminal is None:
            for _ in range(3):
                kid = build()
                if kid is None:
                    return node
                node.kids.append(kid)
        return node

    roots: List[CanonNode] = []
    while pos < len(seq):
        before = pos
        r = build()
        if r is None or pos == before:
            break
        roots.append(r)

    leaves: Counter = Counter()
    splits = 0

    def visit(n: CanonNode) -> None:
        nonlocal splits
        if mass(n.state) > 3 ** n.k:
            errs.append(f"{n.text} @{n.k}: mass {mass(n.state)} exceeds 3^{n.k}")
        if n.terminal is not None:
            if n.stopk != n.k:
                errs.append(f"{n.text}: labelled {n.terminal}_{n.stopk} but sits at depth {n.k}")
            norm = normalize(n.state)
            if any(b != 1 for _, b in norm):
                errs.append(f"{n.text} @{n.k}: certified leaf has a non-singleton part")
            else:
                base_values = singleton_base(n.k)
                widths = sorted((a for a, _ in norm), reverse=True)
                if n.terminal == "canonical U":
                    base = Counter(base_values)
                    need = Counter(widths)
                    if not all(base[v] >= c for v, c in need.items()):
                        errs.append(f"{n.text} @{n.k}: not a sub-multiset of G_{n.k}, "
                                    f"so its canonical label is false")
                else:
                    left = right = 0
                    for i, width in enumerate(widths[:len(base_values)]):
                        left += width
                        right += base_values[i]
                        if left > right:
                            errs.append(f"{n.text} @{n.k}: singleton prefix {i + 1} "
                                        f"has sum {left} > {right} in G_{n.k}")
                            break
            leaves[n.k] += 1
            return
        splits += 1
        if len(n.kids) != 3:
            errs.append(f"{n.text} @{n.k}: {len(n.kids)} children, expected 3")
            return
        if len(n.split) != len(n.state):
            errs.append(f"{n.text} @{n.k}: split has {len(n.split)} entries "
                        f"for {len(n.state)} parts")
            return
        try:
            derived = children_of(n.state, n.split)
        except ValueError as exc:
            errs.append(f"{n.text} @{n.k}: {exc}")
            return
        for want, kid, label in zip(derived, n.kids, ("both", "mixed", "neither")):
            if want != normalize(kid.state):
                errs.append(f"{n.text} @{n.k} child[{label}]: derived {list(want)} "
                            f"but tree says {list(normalize(kid.state))}")
            if kid.k != n.k - 1:
                errs.append(f"{n.text} @{n.k}: child {kid.text} is at depth {kid.k}")
        for kid in n.kids:
            visit(kid)

    for r in roots:
        visit(r)
    if pos != len(seq):
        errs.append(f"{len(seq) - pos} trailing nodes not attached to any tree")
    names = ", ".join(f"{r.text}@{r.k}" for r in roots[:3]) + ("..." if len(roots) > 3 else "")
    return (f"{len(seq)} nodes, {len(roots)} tree(s) [{names}], "
            f"{splits} splits, {sum(leaves.values())} certified leaves")


# -------------------------------------------------------------------- numbered format

# `(used N)` is absent in output from older builds of radio_print.c; when it is missing the
# reference-count cross-check is simply skipped.
HEAD = re.compile(r"^(\d+)\.\s+\(in (\d+)\)\s*(?:\(used (\d+)\)\s*)?(Sa|Sb)\(([^)]*)\)"
                  r"\[(\d+),(\d+)\]\s+take\[([^\]]*)\]\s*:?")
# Older builds wrote `(line -1)` where newer ones write `(trivial)`. A negative line number
# is treated as trivial; the claim is then checked on its own merits, not taken on trust.
CHILD = re.compile(r"^([012])=>(Sa|Sb)\(([^)]*)\)\[(\d+),(\d+)\]\((?:line (-?\d+)|trivial)\)")


def check_numbered(lines: List[str], errs: List[str]) -> str:
    nodes: Dict[int, dict] = {}
    cur = None
    for raw in lines:
        s = raw.strip()
        m = HEAD.match(s)
        if m:
            kind = m.group(4)
            body = m.group(5)
            cur = int(m.group(1))
            nodes[cur] = dict(
                k=int(m.group(2)),
                used=int(m.group(3)) if m.group(3) else None,
                kind=kind,
                n=int(body) if kind == "Sa" else None,
                state=() if kind == "Sa" else parse_state(body),
                take=[int(x) for x in m.group(8).split(",") if x.strip()],
                pairs=int(m.group(6)), total=int(m.group(7)), ch={})
            continue
        m = CHILD.match(s)
        if m and cur is not None:
            kind = m.group(2)
            body = m.group(3)
            nodes[cur]["ch"][int(m.group(1))] = dict(
                kind=kind,
                n=int(body) if kind == "Sa" else None,
                state=() if kind == "Sa" else parse_state(body),
                ref=int(m.group(6)) if m.group(6) and int(m.group(6)) >= 0 else None)
    if not nodes:
        errs.append("no parseable nodes")
        return "0 nodes"

    def expected(nd: dict):
        """Children of one line, as (kind, n_or_state) triples keyed by outcome digit."""
        if nd["kind"] == "Sa":
            n, c = nd["n"], nd["take"][0]
            # Testing c of n coins: both taken -> Sa(c); neither -> Sa(n-c);
            # exactly one -> Sb(c : n-c).
            return {2: ("Sa", c), 1: ("Sb", normalize([(c, n - c)])), 0: ("Sa", n - c)}
        b, m_, nn = children_of(nd["state"], list(zip(nd["take"][0::2], nd["take"][1::2])))
        return {2: ("Sb", b), 1: ("Sb", m_), 0: ("Sb", nn)}

    for ln, nd in sorted(nodes.items()):
        if nd["kind"] == "Sa":
            n = nd["n"]
            if nd["pairs"] != n * (n - 1) // 2 or nd["total"] != n:
                errs.append(f"line {ln}: Sa header pairs/n disagree with Sa({n})")
        else:
            if nd["pairs"] != mass(nd["state"]):
                errs.append(f"line {ln}: header pairs {nd['pairs']} != {mass(nd['state'])}")
            if nd["total"] != sum(a + b for a, b in nd["state"]):
                errs.append(f"line {ln}: header coin count disagrees with the state")
            if len(nd["take"]) != 2 * len(nd["state"]):
                errs.append(f"line {ln}: take[] has {len(nd['take'])} entries "
                            f"for {len(nd['state'])} parts")
                continue
        try:
            exp = expected(nd)
        except ValueError as exc:
            errs.append(f"line {ln}: {exc}")
            continue

        for d in (0, 1, 2):
            ch = nd["ch"].get(d)
            if ch is None:
                errs.append(f"line {ln}: missing child {d}")
                continue
            kind, want = exp[d]
            got = ch["n"] if ch["kind"] == "Sa" else normalize(ch["state"])
            if ch["kind"] != kind or got != want:
                errs.append(f"line {ln} child {d}: declared {ch['kind']}{got} "
                            f"!= derived {kind}{want}")
            child_mass = (ch["n"] * (ch["n"] - 1) // 2) if ch["kind"] == "Sa" else mass(ch["state"])
            if child_mass > 3 ** (nd["k"] - 1):
                errs.append(f"line {ln} child {d}: mass {child_mass} exceeds 3^{nd['k'] - 1}")

            if ch["ref"] is None:                       # claimed trivial
                if ch["kind"] == "Sa":
                    if ch["n"] > 2:
                        errs.append(f"line {ln} child {d}: Sa({ch['n']}) marked trivial")
                elif strip_units(normalize(ch["state"])):
                    errs.append(f"line {ln} child {d}: marked trivial but has a non-unit "
                                f"part {list(strip_units(normalize(ch['state'])))}")
                continue

            tgt = nodes.get(ch["ref"])
            if tgt is None:
                errs.append(f"line {ln} child {d}: reference to missing line {ch['ref']}")
                continue
            if tgt["k"] != nd["k"] - 1:
                errs.append(f"line {ln} child {d}: line {ch['ref']} proves depth "
                            f"{tgt['k']}, need {nd['k'] - 1}")
            if tgt["kind"] == "Sa" and ch["kind"] == "Sa":
                ok = tgt["n"] >= ch["n"]
            elif tgt["kind"] == "Sb" and ch["kind"] == "Sb":
                ok = dominates(strip_units(normalize(tgt["state"])),
                               strip_units(normalize(ch["state"])))
            else:
                ok = False
            if not ok:
                errs.append(f"line {ln} child {d}: line {ch['ref']} does not dominate it")

    used = Counter()
    for nd in nodes.values():
        for ch in nd["ch"].values():
            if ch["ref"] is not None:
                used[ch["ref"]] += 1
    for ln, nd in sorted(nodes.items()):
        if nd["used"] is not None and nd["used"] != used.get(ln, 0):
            errs.append(f"line {ln}: header says (used {nd['used']}) "
                        f"but {used.get(ln, 0)} references point here")

    root = min(nodes)
    seen, stack = {root}, [root]
    while stack:
        for ch in nodes[stack.pop()]["ch"].values():
            if ch["ref"] is not None and ch["ref"] not in seen:
                seen.add(ch["ref"])
                stack.append(ch["ref"])
    if len(seen) != len(nodes):
        errs.append(f"unreachable lines: {sorted(set(nodes) - seen)}")

    top = nodes[root]
    label = f"Sa({top['n']})" if top["kind"] == "Sa" else f"Sb{list(top['state'])}"
    return f"{len(nodes)} nodes, root {label} in {top['k']}, all reachable"


# -------------------------------------------------------------------------------- cli

RESULTPRINT = re.compile(r"^\s*resultprint\s?")


def check_file(path: str) -> bool:
    # radio_print.c tags its tree lines with a `resultprint` prefix so they can be grepped
    # out of a noisy log. Accept the raw form as well as the filtered one.
    lines = [RESULTPRINT.sub("", l.rstrip("\n"))
             for l in open(path) if not l.lstrip().startswith("#")]
    errs: List[str] = []
    numbered = any(HEAD.match(l.strip()) for l in lines)
    summary = check_numbered(lines, errs) if numbered else check_canonical(lines, errs)
    kind = "numbered" if numbered else "canonical"
    status = "OK" if not errs else f"{len(errs)} ERROR(S)"
    print(f"{path}: [{kind}] {summary} -> {status}")
    for e in errs[:20]:
        print(f"    {e}")
    if len(errs) > 20:
        print(f"    ... and {len(errs) - 20} more")
    return not errs


def main(argv: Sequence[str]) -> int:
    if not argv:
        print(__doc__)
        return 2
    ok = all([check_file(p) for p in argv])
    print(f"\n{'all trees verified' if ok else 'VERIFICATION FAILED'}")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
