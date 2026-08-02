# Unit-Group Elimination Theorem

## Statement

Let

\[
S = R \sqcup U
\]

be an Sb state, where

\[
U = (1:1,\dots,1:1)
\]

consists of `u` unit groups, and let \(\mu(\cdot)\) denote total mass.

Then for every \(K\ge 0\),

\[
S \text{ is solvable in } K \text{ tests}
\iff
\bigl(\mu(S)\le 3^K\bigr) \text{ and } \bigl(R \text{ is solvable in } K \text{ tests}\bigr).
\]

Equivalently: unit groups may be removed from an Sb state without affecting solvability, except that they consume one unit of information capacity each.

---

## Interpretation

A unit group \((1:1)\) contains no internal ambiguity: once the process reaches that case, the defective pair is already identified. Therefore a unit group behaves as a pure leaf consumer in a ternary decision tree. The theorem says that this intuition is exact.

---

## Proof

We prove both directions.

### Necessity

Assume \(S=R\sqcup U\) is solvable in \(K\) tests.

1. **Information bound.** Any \(K\)-test quantitative decision tree has at most \(3^K\) leaves, so every solvable state must satisfy

   \[
   \mu(S)\le 3^K.
   \]

2. **Removing unit groups cannot destroy solvability.** A strategy solving \(S\) distinguishes all cases contained in \(S\), hence in particular it distinguishes the substate \(R\subseteq S\). Therefore \(R\) is also solvable in \(K\) tests.

This proves the necessity of both conditions.

### Sufficiency

Assume now that

\[
\mu(S)\le 3^K
\quad\text{and}\quad
R \text{ is solvable in } K \text{ tests}.
\]

Let \(u\) be the number of unit groups in \(U\). Then

\[
\mu(S)=\mu(R)+u,
\]

so the assumption \(\mu(S)\le 3^K\) gives

\[
\mu(R)+u\le 3^K.
\]

Take any \(K\)-test solution tree for \(R\). If some leaves occur at depth strictly smaller than \(K\), pad them with dummy tests so that all leaves are viewed at depth exactly \(K\). This does not change what the tree solves; it only embeds the same strategy into the full \(3\)-ary depth-\(K\) tree.

Such a full depth-\(K\) tree has exactly \(3^K\) leaves. Since \(R\) has mass \(\mu(R)\), its solution uses exactly \(\mu(R)\) leaves, leaving

\[
3^K-\mu(R)
\]

unused leaves. By the capacity inequality above,

\[
3^K-\mu(R)\ge u,
\]

so there are at least \(u\) unused leaves available.

Assign one unused leaf to each unit group \((1:1)\). Because a unit group consists of exactly one case, it requires exactly one leaf and imposes no further structural constraint.

Thus the original solution tree for \(R\) extends to a \(K\)-test solution tree for

\[
S=R\sqcup U.
\]

This proves sufficiency.

---

## Corollary

If two Sb states differ only by the presence of unit groups, then their solvability in \(K\) tests differs only through the information-capacity inequality.

In particular, once the non-unit core \(R\) is known to be solvable in \(K\), the maximum number of additional unit groups that may be appended is exactly

\[
3^K-\mu(R).
\]

---

## Remarks

1. This theorem is the unique fully distribution-agnostic absorption rule for singleton tails: unlike larger singleton groups \((n:1)\) with \(n\ge 2\), unit groups depend only on total residual capacity and not on how that capacity is fragmented.

2. In solver terms, unit groups may be stripped eagerly, with only the total mass retained for the final information-bound check.

3. This theorem is a special case of the broader "singleton absorption" viewpoint, but it is the only case that collapses to a pure scalar condition.
