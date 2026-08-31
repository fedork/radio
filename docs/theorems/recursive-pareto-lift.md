# Recursive Pareto lifting

Status: the lift-box lemma below is proved.  The recursive construction and the choice of a
Pareto upgrade are open.  The experiments are search-heuristic evidence, not an optimality claim.

## The proposed recursion

The first two levels of a one-part frontier strategy are unusually rigid.  A one-part state can
start from a nearby one-part frontier point one level down; after that split, the two nontrivial
lineages can be sent in opposite directions, producing a four-part descendant.  The proposed next
step is:

1. construct the corresponding four-part state one level down using the same two cuts;
2. if necessary, enlarge it to a componentwise-maximal solvable state inside the region allowed by
   the parent (a **Pareto upgrade**);
3. take a solving split of that lower state and lift it into the parent;
4. repeat on the three children, terminating at singleton states embedded in distinct rows of the
   explicit `G_k` construction.  Arbitrary weak-majorization terminals are not valid certificates:
   the [Singleton Majorization Converse](singleton-majorization.md) is false at `K=6`.

The elementary part of this programme is step 3.  It turns an otherwise unconstrained long-state
split into a box around a known lower split.

For this note, a **parent-conditioned Pareto upgrade** of `T@k-1` is a solvable aligned state `U`
with `T <= U <= P` componentwise for which no distinct solvable `V` satisfies
`U < V <= P`.  This is a local antichain, not the one-part `(n,m)` frontier and not the solver
cache's last-segment front.  Several incomparable upgrades can exist.
Because the interval is finite and contains the solvable state `T`, at least one such maximal
upgrade always exists; the hard question is whether any upgrade has a split compatible with the
next parent lift.

## Component-frontier box

There is a stronger finite box for a global upgrade than the information bound alone suggests.

> **Component-frontier lemma.** If `Sb(n1:m1, ..., nr:mr)` is solvable in `k`, then every
> nontrivial component `Sb(ni:mi)` is solvable in `k`.

Delete all other components.  The remaining graph is a subgraph of the original state, so the
claim follows immediately from [Subgraph Monotonicity](subgraph-monotonicity.md).  Consequently,
after normalizing each component, every solvable multi-part state lies in the Cartesian product of
the one-part downset recorded by the same-`k` frontier in
[`data/pareto_sb.csv`](../../data/pareto_sb.csv).

This does not decide a multi-part state: several individually admissible components can still be
jointly impossible.  It does make the global Pareto-upgrade search finite component by component,
and rejects any successor that crosses the one-part frontier without a recursive solve.  Empty and
`1:1` lineages do not require artificial components in this product.  Empty lineages disappear;
`1:1` lineages disappear structurally but their count remains as reserved information capacity,
exactly as required by the
[Unit-Group Elimination Theorem](unit-group-elimination.md).  Forgetting that scalar reserve would
permit a nominal upgrade whose non-unit core leaves no decision-tree leaves for the already-known
unit cases.

## Lift-box lemma

Consider one aligned component.  Let the parent component be

\[
P=(N:M),
\]

let a lower template embedded in it be

\[
T=(n:m),\qquad n\le N,\quad m\le M,
\]

and let the lower test take \(s=(a:b)\).  A parent test \(X=(x:y)\) is a
**lineage-preserving lift** of \(s\) whenever

\[
a\le x\le a+N-n,
\qquad
b\le y\le b+M-m. \tag{1}
\]

Equivalently,

\[
s\le X\le s+(P-T)
\]

coordinatewise.  For a multi-part state, impose (1) independently on every aligned component.
These are labelled lineage shores inherited from earlier cuts; canonical sorting or swapping a
component's two shores is not allowed while forming the correspondence.

> **Lift-box lemma.** Every child of the lower cut is a componentwise substate of the
> corresponding child of a lineage-preserving parent cut.

### Proof

The selected, complementary and two mixed rectangles produced by the lower cut are

\[
(a:b),\quad (n-a:m-b),\quad (a:m-b),\quad (n-a:b).
\]

The corresponding parent rectangles are

\[
(x:y),\quad (N-x:M-y),\quad (x:M-y),\quad (N-x:y).
\]

The lower bounds in (1) give \(x\ge a\) and \(y\ge b\).  Its upper bounds are equivalent to
\(N-x\ge n-a\) and \(M-y\ge m-b\).  Combining one inequality from each pair proves containment
for all four rectangles.  Applying this component by component proves the multi-part statement.
∎

The direction matters.  By
[Subgraph Monotonicity](subgraph-monotonicity.md), solvability of a *parent* child implies
solvability of its lower child.  Solvability of the lower child does **not** prove that its enlarged
parent child is solvable.  Thus the lemma supplies a structured search region, not a construction
by itself.

## A canonical centre and outcome target

For a positive lower coordinate, the proportional lift

\[
x_0=\operatorname{round}(Na/n),\qquad
y_0=\operatorname{round}(Mb/m)
\]

lies in the lift box.  Before rounding, for example,

\[
a\le Na/n\le a+N-n
\]

follows from \(N\ge n\), \(0\le a\le n\); the endpoints are integers, so nearest-integer rounding
preserves the bounds.  A zero-sized lower coordinate is a degeneration: its entire parent interval
is allowed, and the midpoint is a neutral centre.

Let the lower split's three outcome masses be \(q_0,q_1,q_2\), with
\(q_0+q_1+q_2=|T|\).  For parent mass \(|P|\), scale these rather than forcing three equal children:

\[
Q_j\approx q_j\,|P|/|T|,
\qquad Q_0+Q_1+Q_2=|P|.
\]

Largest-remainder rounding makes the sum exact.  The current probe enumerates increasing
coordinatewise \(L_1\) shells about the proportional centre and, within a shell, orders candidates
by

\[
|p_0-Q_0|+|p_1-Q_1|+|p_2-Q_2|,
\]

where \(p_j\) are the candidate's actual child masses.  Cached false children reject candidates;
cache positives do not affect their rank.  Any remaining child is solved only under a strict local
deadline, and failure falls back to the ordinary solver.

## Worked four-part lift

The k=7 Pareto-root strategy used in the experiment reaches the parent

```
P = Sb(45:10, 33:15, 32:14, 23:20),   mass 1853.
```

A lineage-aligned k=6 template is

```
T = Sb(24:5, 19:9, 19:8, 15:13),      mass 638,
s = [3:0, 11:5, 15:7, 7:6].
```

The lower split has outcome masses `202/239/197`.  Scaling them to mass 1853 gives targets
`587/694/572`, and the proportional centre is

```
[6:0, 19:8, 25:12, 11:9].
```

The bounded probe finds, at radius 8 and structural rank 5,

```
X = [10:1, 19:8, 26:13, 10:9],
```

with parent outcome masses `590/701/562`.  All three children solve in k=6.  With the same warm
cache, ordinary search found a different first split after 155,795 admitted top-level splits and
57 solver seconds / 65 wall seconds; the lift probe took 15 wall seconds.  These raw, fully
provenanced outputs are retained under `pareto-lift-2026-08-12` in
[the artifact index](../data.md).

A neighbouring lower point also works, but exposes a necessary piece of state:

```
T' = Sb(24:4, 19:9, 19:9, 15:13),
s' = [3:0, 8:4, 15:8, 7:6].
```

Its targets are `568/703/582`; the probe finds

```
X' = [11:1, 16:7, 27:13, 11:10]
```

with masses `584/702/567`, at radius 12 and rank 274 in 70 wall seconds.  Swapping the two equal
`19:9` template components did not find a split within the same radius.  Equal normalized parts
therefore cannot simply be sorted away: the lineage assignment inherited from the previous cuts is
part of the heuristic state.

## Where the full construction fails today

The successful first lift was followed one level deeper.  Its selected lower child is

```
L = Sb(3:0, 11:5, 15:7, 7:6) @ k=5,
```

and the corresponding parent child is

```
R = Sb(10:1, 19:8, 26:13, 10:9) @ k=6.
```

Lifting the first solving split found for `L` enumerated the complete 774,144-point lift box and
found no parent split whose three children were cache-open.  Upgrading `L` componentwise inside the
parent-conditioned box had a unique greedy endpoint,

```
U = Sb(10:1, 11:5, 15:7, 7:6),
```

but lifting the first solving split found for `U` also exhausted its box without a solution.
The candidate enumeration was complete, but each non-cached child had a 200 ms deadline; this is
not an exhaustive refutation of those 19 cache-open candidates.

This is a bounded-search observation, not a proof that no recursively lifted strategy exists: a
different Pareto upgrade, a different solving split of the same upgrade, or a different earlier
lift can change the descendants.  It does establish that the following greedy implication is
false as an algorithmic rule at low k:

```
one lower witness -> one maximal upgrade -> its first split -> a complete lifted tree.
```

The missing theorem would need a **choice property**: among the Pareto upgrades and solving splits
available at every node, at least one choice must admit compatible lifts in all three branches.
Without that property, recursive Pareto lifting remains a strong happy-path ordering scheme rather
than a direct construction.

## Implementation boundary

[`../../tools/pareto_lift_probe.c`](../../tools/pareto_lift_probe.c) implements the explicit
one-level experiment and an inverse diagnostic.  It deliberately does not modify `radiobase.c`.
Production integration should wait for automatic lower-template discovery and a larger-k corpus.
The smallest safe design is a transient split-hint table carrying lineage metadata: try one or two
lower-front templates in a bounded pass, verify every child with the ordinary tri-state solver, and
fall back unchanged.  No derived split is a cache fact or a proof.
