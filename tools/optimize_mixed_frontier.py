#!/usr/bin/env python3
"""Combine a complete mixed-deficit frontier with the two pure-child thresholds."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path


SUMMARY = re.compile(
    r"^mixed_frontier points=(\d+) complete=(YES|NO) exact=(YES|NO) "
    r"u_box=0:(\d+) v_box=0:(\d+) k=(\d+) depth=(\d+)$",
    re.MULTILINE,
)
PIECE = re.compile(
    r"^mixed_piece u=(\d+):(\d+) sum=(\d+) formula=v=(\d+)-u$",
    re.MULTILINE,
)
POINT = re.compile(r"^mixed_point u=(\d+) v=(\d+) ", re.MULTILINE)


def interval_distance(left_lo: int, left_hi: int, right_lo: int, right_hi: int) -> int:
    if left_hi < right_lo:
        return right_lo - left_hi
    if right_hi < left_lo:
        return left_lo - right_hi
    return 0


def optimize_piece(
    lo: int, hi: int, total: int, pure_u: int, pure_v: int
) -> tuple[int, int, int, int, int]:
    """Return (delta, p, q, cut_u, cut_v) for one guarded slope-one piece."""
    optimum_lo = min(pure_u, total - pure_v)
    optimum_hi = max(pure_u, total - pure_v)
    distance = interval_distance(lo, hi, optimum_lo, optimum_hi)
    delta = max(total, pure_u + pure_v) + distance
    if hi < optimum_lo:
        p = hi
    elif lo > optimum_hi:
        p = lo
    else:
        p = max(lo, optimum_lo)
    q = total - p
    cut_u = max(p, pure_u)
    cut_v = max(q, pure_v)
    if cut_u + cut_v != delta:
        raise AssertionError("piece optimizer disagrees with its direct objective")
    return delta, p, q, cut_u, cut_v


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("pure_u", type=int, help="minimum u imposed by the outcome-2 child")
    parser.add_argument("pure_v", type=int, help="minimum v imposed by the outcome-0 child")
    parser.add_argument("frontier", type=Path, help="output of search_singletonization mixed-frontier")
    args = parser.parse_args()
    if args.pure_u < 0 or args.pure_v < 0:
        parser.error("pure thresholds must be nonnegative deficits")

    text = args.frontier.read_text()
    summaries = SUMMARY.findall(text)
    if len(summaries) != 1:
        raise SystemExit(f"expected one mixed_frontier summary, found {len(summaries)}")
    point_count_text, complete, exact, u_box_text, v_box_text, k_text, depth_text = summaries[0]
    point_count = int(point_count_text)
    u_box, v_box = int(u_box_text), int(v_box_text)
    k, depth = int(k_text), int(depth_text)
    if complete != "YES":
        raise SystemExit("mixed frontier is truncated; refusing to claim a global D optimum")
    if exact != "YES":
        raise SystemExit(
            "mixed frontier is bounded-depth only; refusing to claim an exact D optimum"
        )
    if depth != k:
        raise SystemExit("mixed frontier summary has inconsistent exact/depth fields")
    full_width = 1 << k
    if args.pure_u > full_width or args.pure_v > full_width:
        raise SystemExit("pure threshold lies outside the legal deficit box")

    pieces: list[tuple[int, int, int]] = []
    for lo_text, hi_text, total_text, formula_total_text in PIECE.findall(text):
        lo, hi = int(lo_text), int(hi_text)
        total, formula_total = int(total_text), int(formula_total_text)
        if (
            lo > hi
            or total != formula_total
            or hi > full_width
            or total - hi < 0
            or total - lo > full_width
        ):
            raise SystemExit("malformed mixed_piece line")
        pieces.append((lo, hi, total))
    if not pieces:
        raise SystemExit("complete mixed frontier is empty; D has no feasible width")

    parsed_points = [(int(u), int(v)) for u, v in POINT.findall(text)]
    actual_points = set(parsed_points)
    implied_points = {(u, total - u) for lo, hi, total in pieces for u in range(lo, hi + 1)}
    implied_count = sum(hi - lo + 1 for lo, hi, _ in pieces)
    if (
        any(u > u_box or v > v_box for u, v in actual_points)
        or len(parsed_points) != len(actual_points)
        or implied_count != len(implied_points)
        or actual_points != implied_points
        or len(actual_points) != point_count
    ):
        raise SystemExit("mixed_piece lines do not reproduce the printed frontier points")

    candidates = []
    for lo, hi, total in pieces:
        delta, p, q, cut_u, cut_v = optimize_piece(
            lo, hi, total, args.pure_u, args.pure_v
        )
        candidates.append((delta, p, q, cut_u, cut_v, lo, hi, total))

    best = min(candidate[0] for candidate in candidates)
    choices = [candidate for candidate in candidates if candidate[0] == best]
    parent_width = (1 << (k + 1)) - best
    print(
        f"mixed_optimum delta={best} parent_D_width={parent_width} complete={complete} "
        f"exact={exact} k={k} depth={depth} pure_u={args.pure_u} pure_v={args.pure_v} "
        f"choices={len(choices)}"
    )
    for _, p, q, cut_u, cut_v, lo, hi, total in choices:
        print(
            f"mixed_choice p={p} q={q} cut_u={cut_u} cut_v={cut_v} "
            f"piece_u={lo}:{hi} piece_sum={total}"
        )
    return 0


if __name__ == "__main__":
    sys.exit(main())
