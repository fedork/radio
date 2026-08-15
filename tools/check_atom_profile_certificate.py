#!/usr/bin/env python3
"""Independently verify the symbolic D-lineage certificate.

The producer is tools/search_atom_profiles.cpp.  This checker deliberately reimplements the
four-letter refinement, profile order, aligned cuts, and terminal reference in Python.
"""

from __future__ import annotations

import argparse
import shlex
from pathlib import Path


LETTERS = "ABCD"


def parse_profile(word: str, atom_count: int) -> tuple[int, int, int, int]:
    counts = tuple(word.count(letter) for letter in LETTERS)
    if len(word) != atom_count or sum(counts) != atom_count:
        raise ValueError(f"invalid {atom_count}-atom profile {word!r}")
    if "".join(letter * count for letter, count in zip(LETTERS, counts)) != word:
        raise ValueError(f"profile is not in canonical letter order: {word!r}")
    return counts


def profile_text(profile: tuple[int, int, int, int]) -> str:
    return "".join(letter * count for letter, count in zip(LETTERS, profile))


def refine(profile: tuple[int, int, int, int]) -> tuple[int, int, int, int]:
    a, b, c, d = profile
    return 2 * a + b, b + c, c + d, d


def lift(profile: tuple[int, int, int, int], atom_count: int) -> tuple[int, int, int, int]:
    value = profile
    atoms = sum(value)
    while atoms < atom_count:
        value = refine(value)
        atoms *= 2
    if atoms != atom_count:
        raise ValueError("profile does not lift to configured normalization")
    return value


def deficit(profile: tuple[int, int, int, int]) -> tuple[int, int, int]:
    _, b, c, d = profile
    return d, c + d, b + c + d


def profiles(atom_count: int) -> list[tuple[int, int, int, int]]:
    out = []
    for a in range(atom_count + 1):
        for b in range(atom_count - a + 1):
            for c in range(atom_count - a - b + 1):
                d = atom_count - a - b - c
                out.append((a, b, c, d))
    return out


def selected_profiles(parent: tuple[int, int, int, int], atom_count: int):
    total = refine(parent)
    for selected in profiles(atom_count):
        if all(x <= bound for x, bound in zip(selected, total)):
            yield selected, tuple(bound - x for x, bound in zip(selected, total))


def parse_certificate(path: Path) -> dict[str, str]:
    lines = [line.strip() for line in path.read_text().splitlines() if line.strip()]
    matches = [line for line in lines if line.startswith("atom_lineage_certificate ")]
    if len(matches) != 1:
        raise ValueError(f"expected exactly one atom_lineage_certificate line, found {len(matches)}")
    fields: dict[str, str] = {}
    for token in shlex.split(matches[0]):
        if "=" not in token:
            continue
        key, value = token.split("=", 1)
        if key in fields:
            raise ValueError(f"duplicate certificate field {key}")
        fields[key] = value
    return fields


def require(fields: dict[str, str], key: str, expected: str) -> None:
    actual = fields.get(key)
    if actual != expected:
        raise ValueError(f"{key}: expected {expected!r}, got {actual!r}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("certificate", type=Path)
    args = parser.parse_args()
    fields = parse_certificate(args.certificate)

    require(fields, "version", "1")
    require(fields, "model", "power_of_two_atom_aligned")
    atom_count = int(fields["profile_atoms"])
    if atom_count < 8 or atom_count > 16 or atom_count & (atom_count - 1):
        raise ValueError(f"unsupported profile atom count {atom_count}")
    levels = atom_count.bit_length() - 1
    require(fields, "normalization_levels", str(levels))
    require(fields, "candidate_rank_first", "1")
    require(fields, "candidate_d_max", "1")
    require(fields, "candidate_height", "3")
    require(fields, "closure_outcome", "mixed")
    require(fields, "verdict", "ALL_DEPTH_NO")

    # Derive the first six G_(r+s) profiles by refining A,B,C,C,D,D s times.
    unit = {
        letter: tuple(1 if index == position else 0 for index in range(4))
        for position, letter in enumerate(LETTERS)
    }
    terminal = []
    for letter in "ABCCDD":
        value = unit[letter]
        for _ in range(levels):
            value = refine(value)
        terminal.append(value)
    reference_d = tuple(profile[3] for profile in terminal)
    encoded_reference_d = tuple(int(value) for value in fields["terminal_reference_d"].split(","))
    if encoded_reference_d != reference_d:
        raise ValueError(
            f"terminal reference D coefficients: expected {reference_d}, got {encoded_reference_d}"
        )

    required = sum(reference_d)
    require(fields, "required_d_lineages", str(required))

    ordered = sorted(profiles(atom_count), key=deficit)
    expected_profiles = (atom_count + 1) * (atom_count + 2) * (atom_count + 3) // 6
    if len(ordered) != expected_profiles:
        raise AssertionError(
            f"independent profile enumeration returned {len(ordered)}, not {expected_profiles}"
        )
    excluded = [profile for profile in ordered if profile[3] <= 1]
    last_rank = len(excluded)
    require(fields, "candidate_rank_last", str(last_rank))
    require(fields, "next_rank", str(last_rank + 1))
    require(fields, "next", profile_text(ordered[last_rank]))

    target = parse_profile(fields["target"], atom_count)
    expected_target = lift((1, 5, 1, 1), atom_count)
    if target != expected_target:
        raise ValueError(
            f"certificate target is {profile_text(target)}, expected refined ABBBBBCD class "
            f"{profile_text(expected_target)}"
        )
    require(fields, "target_rank", str(ordered.index(target) + 1))
    if target[3] > 1 or ordered.index(target) >= last_rank:
        raise ValueError("target is not covered by the advertised one-D rank interval")

    fixed_height = 0
    fixed_d = 0
    fixed_parts = []
    for encoded_part in fields["fixed_parts"].split(","):
        word, height_text = encoded_part.split(":", 1)
        profile = parse_profile(word, atom_count)
        height = int(height_text)
        if height <= 0:
            raise ValueError("fixed-part height must be positive")
        fixed_height += height
        fixed_d += profile[3]
        fixed_parts.append((profile, height))
    expected_fixed = [(lift((5, 2, 1, 0), atom_count), 1), (lift((3, 3, 2, 0), atom_count), 2)]
    if fixed_parts != expected_fixed:
        raise ValueError("certificate fixed parts do not match the height-6 hard branch")
    root_height = fixed_height + int(fields["candidate_height"])
    require(fields, "root_height", str(root_height))
    root_d_max = fixed_d + int(fields["candidate_d_max"])
    require(fields, "root_d_lineages_max", str(root_d_max))
    if root_d_max >= required:
        raise ValueError("certificate does not exhibit a strict D-lineage deficit")

    # Exhaust the local algebra used by the coinductive closure.  Refinement preserves D;
    # selected and complementary profiles partition it; and every possible mixed child keeps
    # no more D-bearing lineages than its parent.  Additivity then proves the same for states.
    local_cases = 0
    for parent in profiles(atom_count):
        if refine(parent)[3] != parent[3]:
            raise ValueError(f"D count is not preserved by refinement for {profile_text(parent)}")
        for selected, complement in selected_profiles(parent, atom_count):
            if selected[3] + complement[3] != parent[3]:
                raise ValueError("selected/complementary D counts do not partition the parent")
            for height in range(1, 7):
                for selected_height in range(height + 1):
                    mixed_d = 0
                    if height - selected_height > 0:
                        mixed_d += selected[3]
                    if selected_height > 0:
                        mixed_d += complement[3]
                    if mixed_d > parent[3]:
                        raise ValueError("mixed outcome increases the number of D lineages")
                    local_cases += 1

    print(
        "atom lineage certificate verified: "
        f"ranks 1..{last_rank} all-depth excluded; "
        f"target rank {ordered.index(target) + 1}; next rank {last_rank + 1} "
        f"{profile_text(ordered[last_rank])}; {local_cases} local transitions checked"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
