# Monotone direct-transfer census (`K=3`, 2026-08-29)

## Question

Start with the canonical chain coloring of the transcript graph `Q_3`, of type

    G_3=(8,7,4,4,1,1,1,1),

and pad it by empty color classes.  Is every full-mass integer partition dominated by `G_3`
reachable by unit Robin--Hood transfers in which the moved vertex is already nonadjacent to the
whole recipient class?  Such a step is a direct proper recoloring; it performs no two-color
component swap or cyclic reassignment.

## Method

`tools/singleton_monotone_transfer_census.py` constructs the canonical chains as actual ternary
words and checks conflicts by their first differing symbol.  It independently enumerates every
integer partition of 27 dominated by `G_3`.

The main forward pass retains one reached coloring per type subject to a useful stronger invariant:
whenever two current classes have sizes differing by at least two, some vertex of the larger class
can be moved directly to the smaller.  This constructs 1,201 target types.  Because keeping only
one representative per type is a normalization rather than a completeness theorem, the program
then performs target-directed searches from the canonical coloring for the five types omitted by
that pass.  Every accepted edge is checked to

1. move one actual word and no others;
2. have donor size at least recipient size plus two;
3. introduce no conflict in the recipient; and
4. leave a size partition still dominating the requested target.

Thus the result is a collection of explicit paths, not a count of abstract size transitions.

## Reproduction

From the repository root:

```text
$ tools/singleton_monotone_transfer_census.py
K=3
canonical profile=(8, 7, 4, 4, 1, 1, 1, 1)
dominated full-mass types=1206
unit-ready representative pass=1201
targeted direct paths=5
  (8, 7, 3, 3, 3, 1, 1, 1): 2 moves
  (8, 4, 4, 4, 4, 1, 1, 1): 3 moves
  (8, 3, 3, 3, 3, 3, 3, 1): 6 moves
  (4, 3, 3, 3, 3, 3, 3, 3, 1, 1): 10 moves
  (3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1): 11 moves
ALL DOMINATED TYPES REACHED BY MONOTONE DIRECT RECOLORINGS
```

Measured local runtime was 3.0 wall-clock seconds with the system Python 3 interpreter.  The
script is dependency-free.

## Conclusion and limit

The Canonical Monotone-Transfer Conjecture holds at `K=3`.  This gives exact support to a global
schedule made entirely from simple one-vertex moves.  It does **not** prove that an arbitrary safe
move can be continued, nor that the statement holds for general `K`; both would be extrapolations.
The theorem note gives a symbolic `K=3` example in which one legal first move destroys direct
transferability for another pair, so the scheduling choice is essential.
