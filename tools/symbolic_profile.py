#!/usr/bin/env python3
"""Solve the symbolic profile packing problem for n(k,m).

The programme is stated in
docs/conjectures.md#the-symbolic-profile-programme-2026-08-03. This is its solver.

Everything here is symbolic: no `n`, no `k`, no `t`. Atoms are dyadic-block *letters* of
`G_t` -- `A` = block 0, `B` = block 1, `C` = block 2, ... -- and the only facts used about
`G_t` are the block multiplicities `1, 1, 2, 4, 8, ...`, which are the same for every `t`.
So one solve per `(m, q)` yields a letter string, and `n(k,m)` is that string evaluated at
`t = k - q`, for every `k` at once.

## The model

A state at a node is a multiset of parts; a part is `(c, v)` where `c` is how many m-side
coins it holds and `v` is its n-side, carried as a multiset of letters. Splitting a part
chooses how many coins go in (`b`) and which letters go in (`a`, a sub-multiset of `v`),
and produces

    outcome 2   (b, a)
    outcome 0   (c-b, v-a)
    outcome 1   (c-b, a)  and  (b, v-a)

which is the split table of docs/problem.md read with the n-side as a letter multiset.
Coin identities never matter -- coins inside a part are interchangeable, and every coin in a
part has the same n-side, so a count suffices.

Two consequences worth stating, because they are what make this finite:

* **Every coin's profile is the root multiset `P`.** A part's letter multiset is exactly the
  multiset of level-`q` chunks below it, for each of its coins. So symmetry is not an extra
  assumption here -- it is forced as soon as you want one formula valid for all `t`, because
  distinct letter-count vectors are linearly independent as functions of `t`.
* **A split partitions a multiset.** So the whole search is: repeatedly cut `P` in two down a
  ternary tree, `q` times, and land with one letter per coin per node.

Feasibility is then: after `q` levels every part is a single coin holding a single letter,
and at each level-`q` node the letters received form a sub-multiset of `G_t`, i.e. respect
the multiplicities `1, 1, 2, 4, ...`.

Usage:
    tools/symbolic_profile.py solve <m> [qmax]   best profile for m, searching q upward
    tools/symbolic_profile.py check              reproduce the profiles in conjectures.csv
    tools/symbolic_profile.py show <profile> <q> evaluate e.g. AABC at q=2
    tools/symbolic_profile.py witness <m> <q> <k> [profile]   emit a checkable tree
"""
from __future__ import annotations

import sys
from functools import lru_cache
from itertools import product
from math import comb

LETTERS = "ABCDEFGH"


def multiplicity(i: int) -> int:
    """How many copies of dyadic block `i` the sequence `G_t` holds: 1, 1, 2, 4, 8, ..."""
    return 1 if i <= 1 else 1 << (i - 1)


def letter_value(i: int, t: int) -> int:
    """Block `i` of `G_t` = sum_{j=0}^{t-i} C(t,j) = 2^t - sum_{j<i} C(t,j)."""
    return (1 << t) - sum(comb(t, j) for j in range(i))


def value(counts, t: int) -> int:
    return sum(n * letter_value(i, t) for i, n in enumerate(counts) if n)


def as_string(counts) -> str:
    return "".join(LETTERS[i] * n for i, n in enumerate(counts))


def order_key(counts):
    """Descending symbolic value, for `t` large.

    value = N*2^t - sum_j C(t,j) * S_j   with   S_j = #letters of index > j.
    The 2^t term dominates everything, then C(t,j) grows with j, so compare N first and
    then the S_j from the deepest block down.
    """
    n = sum(counts)
    s = [sum(counts[j + 1:]) for j in range(len(counts))]
    return (-n,) + tuple(reversed(s))


# ------------------------------------------------------------------- the search

class Solver:
    def __init__(self, nlet: int):
        self.nlet = nlet
        self.zero = (0,) * nlet

    @lru_cache(maxsize=None)
    def submultisets(self, v):
        return [tuple(c) for c in product(*(range(x + 1) for x in v))]

    @staticmethod
    def sub(v, a):
        return tuple(x - y for x, y in zip(v, a))

    @staticmethod
    def norm(parts):
        """Drop nil parts -- no coins, or no n-side coins, means no edges."""
        return tuple(sorted(p for p in parts if p[0] > 0 and sum(p[1]) > 0))

    def leaf_ok(self, parts) -> bool:
        """A level-q node: every part one coin holding one atom, and the atoms fit G_t."""
        total = [0] * self.nlet
        for c, v in parts:
            if c != 1 or sum(v) != 1:
                return False
            for i, x in enumerate(v):
                total[i] += x
        return all(x <= multiplicity(i) for i, x in enumerate(total))

    @lru_cache(maxsize=None)
    def feasible(self, parts, d: int) -> bool:
        if not parts:
            return True
        if d == 0:
            return self.leaf_ok(parts)
        cap = 1 << d
        for c, v in parts:
            # d levels can atomize at most 2^d coins and cut v into at most 2^d pieces
            if c > cap or sum(v) > cap:
                return False
        # Capacity count. A part (c,v) puts one letter per coin per element of v somewhere
        # below, so it needs c * v[i] slots for letter i. There are 3^d level-q nodes below
        # and each holds at most multiplicity(i) copies. This is what kills AACC at q=2.
        room = 3 ** d
        for i in range(self.nlet):
            if sum(c * v[i] for c, v in parts) > multiplicity(i) * room:
                return False
        return self._choose(parts, 0, (), (), (), d)

    def options(self, parts, i, dem2, dem0, dem1, d):
        """Splits of part `i` that keep all three children within the shape and capacity
        bounds, given what the earlier parts already demand of each child.

        Pruning here rather than at the recursive call is what makes q >= 4 tractable: the
        product over parts is the blow-up, and most factors die on capacity immediately."""
        c, v = parts[i]
        cap, room = 1 << (d - 1), 3 ** (d - 1)
        lim = [multiplicity(j) * room for j in range(self.nlet)]
        out = []
        for a in self.submultisets(v):
            rest = self.sub(v, a)
            sa, sr = sum(a), sum(rest)
            for b in range(c + 1):
                # a part is nil unless it has both coins and n-side coins
                if b and sa and (b > cap or sa > cap):
                    continue
                if (c - b) and sr and (c - b > cap or sr > cap):
                    continue
                n2 = self._add(dem2, b, a, lim)
                if n2 is None:
                    continue
                n0 = self._add(dem0, c - b, rest, lim)
                if n0 is None:
                    continue
                mid = self._add(dem1, c - b, a, lim)     # the mixed child gets two parts
                if mid is None:
                    continue
                n1 = self._add(mid, b, rest, lim)
                if n1 is None:
                    continue
                out.append((b, a, rest, n2, n0, n1))
        return out

    def _add(self, dem, c, v, lim):
        """dem + c*v, or None if it busts a letter's capacity."""
        if not c or not sum(v):
            return dem
        new = list(dem)
        for j, x in enumerate(v):
            if x:
                new[j] += c * x
                if new[j] > lim[j]:
                    return None
        return tuple(new)

    def _choose(self, parts, i, k2, k0, k1, d,
                dem2=None, dem0=None, dem1=None) -> bool:
        """Pick a split for each part in turn, accumulating the three children."""
        if dem2 is None:
            dem2 = dem0 = dem1 = self.zero
        if i == len(parts):
            return (self.feasible(self.norm(k2), d - 1)
                    and self.feasible(self.norm(k0), d - 1)
                    and self.feasible(self.norm(k1), d - 1))
        c, _ = parts[i]
        for b, a, rest, n2, n0, n1 in self.options(parts, i, dem2, dem0, dem1, d):
            if self._choose(parts, i + 1,
                            k2 + ((b, a),),
                            k0 + ((c - b, rest),),
                            k1 + ((c - b, a), (b, rest)), d, n2, n0, n1):
                return True
        return False

    def build(self, parts, d):
        """The tree behind a `feasible` verdict, as nested choices. Cheap: `feasible` is
        memoized, so this just replays the search taking the first branch that works."""
        parts = self.norm(parts)
        if not parts:
            return ("nil",)
        if d == 0:
            return ("leaf", parts)
        choice = self._pick(parts, 0, (), (), (), d, ())
        if choice is None:
            return None
        k2, k0, k1, choices = choice
        return ("split", parts, choices,
                [self.build(k2, d - 1), self.build(k1, d - 1), self.build(k0, d - 1)])

    def _pick(self, parts, i, k2, k0, k1, d, choices,
              dem2=None, dem0=None, dem1=None):
        if dem2 is None:
            dem2 = dem0 = dem1 = self.zero
        if i == len(parts):
            if (self.feasible(self.norm(k2), d - 1)
                    and self.feasible(self.norm(k0), d - 1)
                    and self.feasible(self.norm(k1), d - 1)):
                return k2, k0, k1, choices
            return None
        c, _ = parts[i]
        for b, a, rest, n2, n0, n1 in self.options(parts, i, dem2, dem0, dem1, d):
            got = self._pick(parts, i + 1, k2 + ((b, a),), k0 + ((c - b, rest),),
                             k1 + ((c - b, a), (b, rest)), d, choices + ((b, a),),
                             n2, n0, n1)
            if got is not None:
                return got
        return None

    def best(self, m: int, q: int):
        """Largest-value profile of at most 2^q letters that m coins can pack in q levels."""
        cands = []
        for counts in product(*(range(min((1 << q), multiplicity(i) * (1 << q)) + 1)
                                for i in range(self.nlet))):
            if 1 <= sum(counts) <= (1 << q):
                cands.append(counts)
        cands.sort(key=order_key)
        for counts in cands:
            if self.feasible(((m, counts),), q):
                return counts
        return None


def formula(counts, q: int) -> str:
    """n(k,m) = value of the profile at t = k-q, as an explicit expression in k."""
    n = sum(counts)
    terms = [f"{n} * 2^(k-{q})" if n != 1 else f"2^(k-{q})"]
    for j in range(max(len(counts), 1)):
        s = sum(counts[j + 1:])
        if s:
            binom = "1" if j == 0 else ("(k-%d)" % q if j == 1 else f"C(k-{q},{j})")
            terms.append(f"{s}*{binom}" if s != 1 else binom)
    return terms[0] + "".join(" - " + x for x in terms[1:])


def evaluate(counts, q: int, k: int) -> int:
    return value(counts, k - q)


# ------------------------------------------------------------------- commands

KNOWN = {1: ("A", 0), 2: ("B", 0), 3: ("AC", 1), 4: ("AACC", 2),
         5: ("BBBD", 2), 6: ("BBCD", 2)}


def parse_profile(s: str):
    counts = [0] * len(LETTERS)
    for ch in s.upper():
        counts[LETTERS.index(ch)] += 1
    while counts and counts[-1] == 0:
        counts.pop()
    return tuple(counts)


def cmd_solve(argv) -> int:
    m = int(argv[0])
    qmax = int(argv[1]) if len(argv) > 1 else 5
    nlet = 4 if m <= 6 else 5
    solver = Solver(nlet)                 # one memo shared across all q
    for q in range(0, qmax + 1):
        if (1 << q) < m:
            continue                      # cannot atomize m coins in q levels
        got = solver.best(m, q)
        if got is None:
            print(f"m={m} q={q}: infeasible")
            continue
        print(f"m={m} q={q}: {as_string(got)}   n(k,{m}) = {formula(got, q)}")
        print(f"          values k=q..q+8: "
              + " ".join(str(evaluate(got, q, k)) for k in range(q, q + 9)))
        sys.stdout.flush()
    return 0


def cmd_show(argv) -> int:
    counts, q = parse_profile(argv[0]), int(argv[1])
    print(f"{as_string(counts)} at q={q}: n(k,m) = {formula(counts, q)}")
    print("  " + " ".join(f"k={k}:{evaluate(counts, q, k)}" for k in range(q + 1, q + 9)))
    return 0


def cmd_check(argv) -> int:
    """Every known profile must be feasible, and must be the best at its own q."""
    rc = 0
    for m, (prof, q) in sorted(KNOWN.items()):
        counts = parse_profile(prof)
        nlet = max(4, len(counts))
        solver = Solver(nlet)
        feas = solver.feasible(((m, counts + (0,) * (nlet - len(counts))),), q)
        got = solver.best(m, q)
        ok = feas and got is not None and as_string(got) == prof
        rc |= not ok
        print(f"m={m} q={q} {prof}: feasible={feas} best={as_string(got) if got else None}"
              f"  {'OK' if ok else 'MISMATCH'}")
        sys.stdout.flush()
    return rc


def render(node, t: int, depth: int, out: list, indent: int) -> None:
    """Instantiate a symbolic tree at a concrete `t`, in witnesses/*.tree format."""
    pad = "  " * indent
    if node[0] in ("nil", "leaf"):
        parts = node[1] if node[0] == "leaf" else ()
        s = ",".join(f"{value(v, t)}:{c}" for c, v in parts) or "0:0"
        out.append(f"{pad}{s} @{depth} [canonical U_{depth}]")
        return
    _, parts, choices, kids = node
    s = ",".join(f"{value(v, t)}:{c}" for c, v in parts)
    sp = ",".join(f"{value(a, t)}:{b}" for b, a in choices)
    out.append(f"{pad}{s} @{depth} --[{sp}]-->")
    for kid in kids:
        render(kid, t, depth - 1, out, indent + 1)


def cmd_witness(argv) -> int:
    """witness <m> <q> <k> -- emit a checkable tree for the best profile at (m,q)."""
    m, q, k = (int(x) for x in argv[:3])
    solver = Solver(4 if m <= 6 else 5)
    # an explicit profile skips the search, which is the expensive part
    counts = parse_profile(argv[3]) if len(argv) > 3 else solver.best(m, q)
    if counts is not None:
        counts = tuple(counts) + (0,) * (solver.nlet - len(counts))
    if counts is None:
        print(f"m={m} q={q}: infeasible", file=sys.stderr)
        return 1
    tree = solver.build(((m, counts),), q)
    n = evaluate(counts, q, k)
    out = [f"# Symbolic profile solution for Sb({n}:{m}) in {k}, profile "
           f"{as_string(counts)} at q={q} (t={k - q}).",
           f"# Generated by tools/symbolic_profile.py witness {m} {q} {k}.",
           f"# n(k,{m}) = {formula(counts, q)}",
           "# format  : indented '<state> @k --[split]-->' / '<state> @k [canonical U_k]'",
           "# checked : tools/check_witness.py",
           "#"]
    render(tree, k - q, k, out, 0)
    print("\n".join(out))
    return 0


def cmd_test(argv) -> int:
    """test <m> <q> <profile> -- feasibility of one profile, skipping the search."""
    m, q, prof = int(argv[0]), int(argv[1]), argv[2]
    solver = Solver(max(4, len(parse_profile(prof))))
    counts = parse_profile(prof) + (0,) * (solver.nlet - len(parse_profile(prof)))
    ok = solver.feasible(((m, counts),), q)
    print(f"m={m} q={q} {as_string(counts)} ({sum(counts)} letters): "
          f"{'FEASIBLE' if ok else 'infeasible'}")
    if ok:
        print(f"   n(k,{m}) = {formula(counts, q)}")
        print("   " + " ".join(f"k={k}:{evaluate(counts, q, k)}" for k in range(q, q + 7)))
    return 0 if ok else 1


def refine(counts):
    """One step of the forced refinement A->aa, B->ab, C->bc, D->cd (Atom Descent)."""
    out = [0] * (len(counts) + 1)
    for i, n in enumerate(counts):
        if i == 0:
            out[0] += 2 * n
        else:
            out[i - 1] += n
            out[i] += n
    while out and out[-1] == 0:
        out.pop()
    return tuple(out)


def cmd_refine(argv) -> int:
    counts, times = parse_profile(argv[0]), int(argv[1]) if len(argv) > 1 else 1
    for _ in range(times):
        counts = refine(counts)
    print(as_string(counts))
    return 0


def cmd_testroot(argv) -> int:
    """testroot <m> <q> <profile> <b> <a> -- feasibility with the root split forced.

    Cuts the search enormously when the right root split is known from elsewhere. For m=6
    the recursion n(k,6) = n(k-1,4) + n(k-1,5) names it: b=2 and a worth n(k-1,5)."""
    m, q, prof, b, aprof = int(argv[0]), int(argv[1]), argv[2], int(argv[3]), argv[4]
    solver = Solver(4 if m <= 6 else 5)
    pad = lambda c: tuple(c) + (0,) * (solver.nlet - len(c))
    P, a = pad(parse_profile(prof)), pad(parse_profile(aprof))
    rest = solver.sub(P, a)
    if any(x < 0 for x in rest):
        print("a is not a sub-multiset of the profile", file=sys.stderr)
        return 2
    kids = {"out2": ((b, a),), "out0": ((m - b, rest),),
            "out1": ((m - b, a), (b, rest))}
    ok = True
    for name, kp in kids.items():
        good = solver.feasible(solver.norm(kp), q - 1)
        parts = ", ".join(f"{c}x[{as_string(v)}]" for c, v in solver.norm(kp))
        print(f"  {name}: {parts or 'nil'} -> {'ok' if good else 'INFEASIBLE'}")
        ok &= good
        sys.stdout.flush()
    print(f"m={m} q={q} {as_string(P)} with root split b={b}, a={as_string(a)}: "
          f"{'FEASIBLE' if ok else 'infeasible'}")
    return 0 if ok else 1


COMMANDS = {"solve": cmd_solve, "check": cmd_check, "show": cmd_show,
            "testroot": cmd_testroot,
            "witness": cmd_witness, "test": cmd_test, "refine": cmd_refine}


def main() -> int:
    sys.setrecursionlimit(100000)
    if len(sys.argv) < 2 or sys.argv[1] not in COMMANDS:
        print(__doc__)
        return 2
    return COMMANDS[sys.argv[1]](sys.argv[2:])


if __name__ == "__main__":
    sys.exit(main())
