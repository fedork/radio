#!/usr/bin/env python3
"""Check accounting and scope markers in an exact atom-profile cover-slice log."""

from __future__ import annotations

import argparse
import re
from collections import Counter
from pathlib import Path

Deficit = tuple[int, int, int]


def parse_fields(line: str) -> dict[str, str]:
    return dict(re.findall(r"([A-Za-z_]+)=([^ ]+)", line))


def state_supply(encoded: str) -> Deficit:
    total = [0, 0, 0]
    for part in encoded.split(","):
        word, separator, height = part.rpartition(":")
        if not separator or not word or not height.isdigit():
            raise ValueError(f"malformed profile state part: {part}")
        d = word.count("D")
        c = word.count("C")
        b = word.count("B")
        total[0] += d
        total[1] += c + d
        total[2] += b + c + d
    return tuple(total)  # type: ignore[return-value]


def refine_supply(value: Deficit) -> Deficit:
    d, v, w = value
    return d, v + d, w + v


def parse_loss(encoded: str) -> Deficit:
    values = tuple(int(item) for item in encoded.split(","))
    if len(values) != 3:
        raise ValueError(f"malformed loss: {encoded}")
    return values  # type: ignore[return-value]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("log", type=Path)
    parser.add_argument("--expect-loss", metavar="MIN:MAX")
    parser.add_argument("--expect-candidates", type=int)
    parser.add_argument("--expect-unique", type=int)
    args = parser.parse_args()

    lines = args.log.read_text().splitlines()
    materialized = [line for line in lines if line.startswith("cover_root_materialized ")]
    frontiers = [line for line in lines if line.startswith("cover_root_pure_frontier ")]
    summaries = [line for line in lines if line.startswith("atom_profile_cover_slice ")]
    final_results = [line for line in lines if line.startswith("cover_root_result ")]
    if len(materialized) != 1 or len(frontiers) != 1 or len(summaries) != 1:
        raise ValueError("expected one materialized, frontier, and scoped-summary line")
    if not final_results or "answer=NO" not in final_results[-1] or "active=0" not in final_results[-1]:
        raise ValueError("cover search did not finish with an exhaustive NO")

    materialized_fields = parse_fields(materialized[0])
    frontier_fields = parse_fields(frontiers[0])
    summary_fields = parse_fields(summaries[0])
    if summary_fields.get("answer") != "NO":
        raise ValueError("slice summary is not negative")
    if summary_fields.get("scope") != "declared_root_loss_slice_only":
        raise ValueError("slice summary is missing its restricted scope")

    slice_match = re.fullmatch(r"0,0,(\d+)\.\.(\d+)", materialized_fields.get("slice_loss", ""))
    summary_match = re.fullmatch(r"(\d+)\.\.(\d+)", summary_fields.get("loss_W", ""))
    if slice_match is None or summary_match is None:
        raise ValueError("malformed W-loss range")
    loss_range = tuple(map(int, slice_match.groups()))
    if loss_range != tuple(map(int, summary_match.groups())):
        raise ValueError("materialized and summary W-loss ranges differ")
    if summary_fields.get("loss_D") != "0" or summary_fields.get("loss_V") != "0":
        raise ValueError("slice summary is not a pure W-loss slice")
    if args.expect_loss is not None:
        expected = tuple(map(int, args.expect_loss.split(":")))
        if len(expected) != 2 or loss_range != expected:
            raise ValueError(f"loss range {loss_range} != expected {expected}")

    class_candidates: dict[Deficit, int] = {}
    class_unique: dict[Deficit, int] = {}
    for line in lines:
        if not line.startswith("cover_root_loss "):
            continue
        fields = parse_fields(line)
        loss = parse_loss(fields["loss"])
        class_candidates[loss] = int(fields["candidates"])
        class_unique[loss] = int(fields["unique_mixed_children"])

    candidates = int(frontier_fields["candidates"])
    unique = int(frontier_fields["unique_mixed_children"])
    if candidates > int(materialized_fields["candidates"]):
        raise ValueError("pure-frontier candidate count exceeds the materialized count")
    if candidates != sum(class_candidates.values()) or unique != sum(class_unique.values()):
        raise ValueError("loss-class totals do not match the frontier summary")
    if args.expect_candidates is not None and candidates != args.expect_candidates:
        raise ValueError(f"candidate count {candidates} != expected {args.expect_candidates}")
    if args.expect_unique is not None and unique != args.expect_unique:
        raise ValueError(f"unique count {unique} != expected {args.expect_unique}")

    root_state = summary_fields["state"]
    root_supply = refine_supply(state_supply(root_state))
    starts: list[str] = []
    observed_losses: Counter[Deficit] = Counter()
    for line in lines:
        if not line.startswith("cover_root_mixed_start "):
            continue
        fields = parse_fields(line)
        state = fields["state"]
        starts.append(state)
        child_supply = state_supply(state)
        loss = tuple(root_supply[index] - child_supply[index] for index in range(3))
        observed_losses[loss] += 1
    if len(starts) != len(set(starts)):
        raise ValueError("a mixed child was started more than once")
    if len(starts) != unique:
        raise ValueError(f"started {len(starts)} mixed children, expected {unique}")
    if observed_losses != Counter(class_unique):
        raise ValueError(
            f"started mixed-child loss counts {observed_losses} != frontier {class_unique}"
        )
    if any(loss[0] or loss[1] or not (loss_range[0] <= loss[2] <= loss_range[1])
           for loss in observed_losses):
        raise ValueError("a started child lies outside the declared loss slice")

    print(
        f"atom profile cover log verified: loss_W={loss_range[0]}..{loss_range[1]}, "
        f"{candidates} oriented tests, {unique} exact-negative mixed children"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
