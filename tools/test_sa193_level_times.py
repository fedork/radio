#!/usr/bin/env python3
import unittest

import sa193_level_times


FIGURE_SPACE = "\u2007"


def time_line(level: int, verdicts: int, inclusive: int) -> str:
    count = str(verdicts).rjust(11, FIGURE_SPACE)
    timing = f"{inclusive}s".rjust(10, FIGURE_SPACE)
    return f"    k={level}{count}{timing}"


class LevelTimesTest(unittest.TestCase):
    def test_adds_visible_active_elapsed_before_computing_self(self) -> None:
        status = "\n".join(
            [
                "  latest activity per level, from the level the search is on (k=3) up to the root",
                "    k=4 [solving] pass=1 elapsed 20/30 left=1/2",
                "    k=3 [solving] pass=1 elapsed 5/10 left=1/2",
                "  time by level - k, verdicts, inclusive, self, %cpu, splits",
                time_line(4, 10, 100),
                time_line(3, 20, 70),
                time_line(2, 5, 20),
            ]
        )
        inclusive, active = sa193_level_times.parse_status(status)
        self.assertEqual(inclusive, {4: 100, 3: 70, 2: 20})
        self.assertEqual(active, {4: 20, 3: 5})
        output = sa193_level_times.format_level_times(status)
        self.assertIn("k4 120/45", output)
        self.assertIn("k3 75/55", output)
        self.assertIn("k2 20/20", output)

    def test_prefers_actual_cpu_from_work_budget_progress(self) -> None:
        status = "\n".join(
            [
                "  latest activity per level, from the level the search is on (k=4) up to the root",
                "    k=4 [solving] pass=1 elapsed 20/30 work=400000000/600000000 "
                "left=1/2 totalsplits=1 cpu=7",
                "  time by level - k, verdicts, inclusive, self, %cpu, splits",
                time_line(4, 10, 100),
                time_line(3, 20, 20),
            ]
        )
        inclusive, active = sa193_level_times.parse_status(status)
        self.assertEqual(inclusive, {4: 100, 3: 20})
        self.assertEqual(active, {4: 7})
        output = sa193_level_times.format_level_times(status)
        self.assertIn("k4 107/87", output)
        self.assertIn("k3 20/20", output)


if __name__ == "__main__":
    unittest.main()
