#!/usr/bin/env python3
"""An independent reference solver, and the conjecture experiments built on it.

This is a second implementation of the search, written from `docs/problem.md` alone and
sharing no code with `radiobase.c`. It is far slower and only usable for `k <= 6`, but it
is short enough to audit by eye, which makes it the right instrument for settling
structural questions: when it and the C solver agree, the answer does not depend on either
one being correct.

It reproduces the proven Pareto columns for `k = 1..6` exactly.

State representation follows `docs/problem.md`: a state is a multiset of parts `(n,m)` with
`n >= m >= 1`, meaning one defective among `n` coins and the other among a disjoint `m`.
A test is a split vector `[(a_i,b_i)]` and the three children are

    outcome 2  {(a_i, b_i)}
    outcome 0  {(n_i-a_i, m_i-b_i)}
    outcome 1  {(a_i, m_i-b_i), (n_i-a_i, b_i)}

Usage:
    tools/refsolve.py frontier [K]          n(k,m) for k = 1..K            (default 5)
    tools/refsolve.py solve K n1 m [n1 m..] decide one state
    tools/refsolve.py check-c [K]           conjecture C, exhaustive
    tools/refsolve.py check-c-multipart [K] the multi-part generalisation (has a counterexample)
    tools/refsolve.py check-extremal [K]    the Extremal Split Lemma
    tools/refsolve.py two-part K s t        the staircase g(p) = max q, Sb(p:s, q:t) in K
    tools/refsolve.py check-2part [K]       the (2,1) and (2,2) closed forms, exhaustive
    tools/refsolve.py check-reduction [K]   the Two-Part Reduction Identity

See docs/conjectures.md#the-antidiagonal-conjecture-c and
docs/theorems/two-part-reduction.md for what these mean.
"""
from __future__ import annotations

import sys
from itertools import combinations_with_replacement, product

POW3 = [3 ** i for i in range(24)]


# --------------------------------------------------------------------------- core

def norm(parts) -> tuple:
    """Canonical form: drop empty parts, orient each `n >= m`, sort descending."""
    out = [(max(n, m), min(n, m)) for n, m in parts if n > 0 and m > 0]
    out.sort(reverse=True)
    return tuple(out)


def mass(state) -> int:
    return sum(n * m for n, m in state)


_memo: dict = {}


def solvable(state, k: int) -> bool:
    state = norm(state)
    total = mass(state)
    if total <= 1:
        return True
    if k <= 0 or total > POW3[k]:
        return False
    key = (state, k)
    hit = _memo.get(key)
    if hit is not None:
        return hit
    lim = POW3[k - 1]
    per_part = [[(a, b) for a in range(n + 1) for b in range(m + 1)] for n, m in state]
    result = False
    seen = set()
    for sv in product(*per_part):
        c2 = norm(sv)
        c0 = norm([(n - a, m - b) for (n, m), (a, b) in zip(state, sv)])
        c1 = norm([p for (n, m), (a, b) in zip(state, sv)
                   for p in ((a, m - b), (n - a, b))])
        if mass(c0) > lim or mass(c2) > lim or mass(c1) > lim:
            continue
        # a split and its complement have the same children with outcomes 0 and 2 swapped
        sig = (min(c0, c2), max(c0, c2), c1)
        if sig in seen:
            continue
        seen.add(sig)
        if solvable(c0, k - 1) and solvable(c2, k - 1) and solvable(c1, k - 1):
            result = True
            break
    _memo[key] = result
    return result


def winners(n: int, m: int, k: int) -> set:
    """Every split of the one-part state `(n:m)` whose three children survive `k-1`."""
    out = set()
    for a in range(n + 1):
        for b in range(m + 1):
            if (solvable([(a, b)], k - 1)
                    and solvable([(n - a, m - b)], k - 1)
                    and solvable([(a, m - b), (n - a, b)], k - 1)):
                out.add((a, b))
    return out


def frontier_row(k: int, mmax: int = 64) -> dict:
    """n(k,m): the largest n1 with Sb(n1:m) solvable in k."""
    row = {}
    for m in range(1, mmax + 1):
        n = m
        while n * m <= POW3[k] and solvable([(n, m)], k):
            n += 1
        if n == m:
            break
        row[m] = n - 1
    return row


# ---------------------------------------------------------------------- experiments

def cmd_frontier(argv) -> int:
    kmax = int(argv[0]) if argv else 5
    for k in range(1, kmax + 1):
        row = frontier_row(k)
        print(f"k={k}: " + " ".join(f"{m}:{v}" for m, v in sorted(row.items())))
        bad = [m for m in sorted(row) if m + 1 in row and row[m + 1] >= row[m]]
        print(f"   strictly decreasing in m: {'yes' if not bad else 'NO at m=' + str(bad)}")
        sys.stdout.flush()
    return 0


def cmd_solve(argv) -> int:
    k = int(argv[0])
    nums = [int(x) for x in argv[1:]]
    state = norm(list(zip(nums[0::2], nums[1::2])))
    verdict = "can solve" if solvable(state, k) else "can't solve"
    label = ",".join(f"{n}:{m}" for n, m in state)
    print(f"Sb({label})[{mass(state)}] in {k}: {verdict}")
    return 0


def cmd_check_c(argv) -> int:
    """C: Sb(a:b) solvable in k, a >= b  =>  Sb(a+1:b-1) solvable in k."""
    kmax = int(argv[0]) if argv else 5
    rc = 0
    for k in range(1, kmax + 1):
        bad, tested = [], 0
        for b in range(1, 40):
            for a in range(b, POW3[k] + 1):
                if a * b > POW3[k]:
                    break
                if not solvable([(a, b)], k):
                    continue
                tested += 1
                if not solvable([(a + 1, b - 1)], k):
                    bad.append((a, b))
        print(f"k={k}: {tested} solvable one-part states, "
              f"{len(bad)} counterexamples {bad if bad else ''}")
        rc |= bool(bad)
        sys.stdout.flush()
    return rc


def _enum_states(k: int, maxparts: int):
    lim = POW3[k]
    parts = [(n, m) for m in range(1, lim + 1) for n in range(m, lim + 1) if n * m <= lim]
    seen = set()
    for r in range(1, maxparts + 1):
        for combo in combinations_with_replacement(parts, r):
            s = norm(combo)
            if mass(s) <= lim and s not in seen:
                seen.add(s)
                yield s


def cmd_check_c_multipart(argv) -> int:
    """The multi-part generalisation of C. This is FALSE; the command exhibits it."""
    kmax = int(argv[0]) if argv else 4
    for k in range(2, kmax + 1):
        maxparts = 3 if k <= 3 else 2
        tested, bad = 0, []
        for s in _enum_states(k, maxparts):
            if not solvable(s, k):
                continue
            for i, (n, m) in enumerate(s):
                if m < 1:
                    continue
                s2 = norm(list(s[:i]) + [(n + 1, m - 1)] + list(s[i + 1:]))
                tested += 1
                if not solvable(s2, k):
                    bad.append((s, s2))
        print(f"k={k} (<= {maxparts} parts): {tested} intra-part moves, "
              f"{len(bad)} counterexamples")
        for s, s2 in bad[:5]:
            print(f"    {s} mass {mass(s)}  ->  {s2} mass {mass(s2)}   solvable -> UNSOLVABLE")
        sys.stdout.flush()
    return 0


def cmd_check_extremal(argv) -> int:
    """Extremal Split Lemma: the winning split minimising p-q survives edit A."""
    kmax = int(argv[0]) if argv else 5
    rc = 0
    for k in range(2, kmax + 1):
        tested = ok = 0
        bad = []
        for b in range(2, 40):
            for a in range(b, POW3[k] + 1):
                if a * b > POW3[k]:
                    break
                if not solvable([(a, b)], k):
                    continue
                W = winners(a, b, k)
                p, q = min(W, key=lambda s: (s[0] - s[1], s[0]))
                tested += 1
                if q >= 1 and (p + 1, q - 1) in winners(a + 1, b - 1, k):
                    ok += 1
                else:
                    bad.append(((a, b), (p, q)))
        print(f"k={k}: {ok}/{tested} states obey the Extremal Split Lemma"
              + (f"   failures: {bad}" if bad else ""))
        rc |= bool(bad)
        sys.stdout.flush()
    return rc


# ------------------------------------------------------- the two-part frontier

def two_part_row(k: int, s: int, t: int) -> dict:
    """g(p) = max q with Sb(p:s, q:t) solvable in k, for every p >= 1 with g(p) > 0.

    Non-increasing in p by Subgraph Monotonicity, so q only ever walks downward."""
    row = {}
    q = frontier_row(k, t).get(t, 0) if t else 0
    p = 1
    while q > 0:
        if not solvable([(p, s)], k):
            break
        while q > 0 and not solvable([(p, s), (q, t)], k):
            q -= 1
        if q == 0:
            break
        row[p] = q
        p += 1
    return row


def claim_21(p: int, q: int, k: int) -> bool:
    """Sb(p:2, q:1) in k -- Two-Part Frontier Theorem, docs/theorems/two-part-reduction.md."""
    return p <= 2 ** k - 1 and q <= 2 ** k and p + q <= 2 ** (k + 1) - k - 1


def claim_22(p: int, q: int, k: int) -> bool:
    """Sb(p:2, q:2) in k -- conjectured, docs/conjectures.md."""
    if p > 2 ** k - 1 or q > 2 ** k - 1:
        return False
    cap = 2 ** (k + 1) - 2 * k - (1 if max(p, q) >= 2 ** k - 2 else 0)
    return p + q <= cap


def cmd_two_part(argv) -> int:
    k, s, t = (int(x) for x in argv[:3])
    row = two_part_row(k, s, t)
    print(f"k={k} s={s} t={t}: " + " ".join(f"{p}->{q}" for p, q in sorted(row.items())))
    if row:
        best = max(p + q for p, q in row.items())
        print(f"   max p+q = {best}")
    return 0


def cmd_check_2part(argv) -> int:
    kmax = int(argv[0]) if argv else 5
    rc = 0
    for k in range(1, kmax + 1):
        for t, claim, name in ((1, claim_21, "(2,1)"), (2, claim_22, "(2,2)")):
            bad, n = [], 0
            lim = 2 ** k + 2
            for p in range(0, lim + 1):
                for q in range(0, lim + 1):
                    if p and q and 2 * p + t * q > POW3[k]:
                        continue
                    n += 1
                    if solvable([(p, 2), (q, t)], k) != claim(p, q, k):
                        bad.append((p, q))
            print(f"k={k} {name}: {n} pairs, {len(bad)} disagreements "
                  f"{bad[:5] if bad else ''}")
            rc |= bool(bad)
            sys.stdout.flush()
    return rc


def cmd_check_reduction(argv) -> int:
    """n(k+1,m) = max over top splits of the capped two-part maximum.

    An identity, not a conjecture: it just says a strategy is a first test plus
    three sub-strategies, with the outcome-2 and outcome-0 children single parts.
    Checking it cross-validates the one-part and two-part tables against each other."""
    kmax = int(argv[0]) if argv else 4
    rc = 0
    for k in range(1, kmax + 1):
        nk = frontier_row(k)
        nk1 = frontier_row(k + 1)
        rows = {}
        for m in sorted(nk1):
            if m < 2:
                continue
            best = 2 * nk.get(m, 0)                      # b = 0 or m: m-side left whole
            for b in range(1, m):
                s, t = m - b, b
                if (s, t) not in rows:
                    rows[(s, t)] = two_part_row(k, s, t)
                row = rows[(s, t)]
                cap_p, cap_q = nk.get(b, 0), nk.get(m - b, 0)   # outcome 2 / outcome 0
                for p in range(0, min(cap_p, nk.get(s, 0)) + 1):
                    q = min(cap_q, row.get(p, nk.get(t, 0) if p == 0 else 0))
                    best = max(best, p + q)
            ok = best == nk1[m]
            rc |= not ok
            print(f"k={k}->{k+1} m={m}: predicted {best}, actual n({k+1},{m})={nk1[m]}"
                  f"  {'OK' if ok else 'MISMATCH'}")
            sys.stdout.flush()
    return rc


COMMANDS = {
    "frontier": cmd_frontier,
    "two-part": cmd_two_part,
    "check-2part": cmd_check_2part,
    "check-reduction": cmd_check_reduction,
    "solve": cmd_solve,
    "check-c": cmd_check_c,
    "check-c-multipart": cmd_check_c_multipart,
    "check-extremal": cmd_check_extremal,
}


def main() -> int:
    sys.setrecursionlimit(100000)
    if len(sys.argv) < 2 or sys.argv[1] not in COMMANDS:
        print(__doc__)
        return 2
    return COMMANDS[sys.argv[1]](sys.argv[2:])


if __name__ == "__main__":
    sys.exit(main())
