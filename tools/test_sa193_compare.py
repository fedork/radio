#!/usr/bin/env python3
import tempfile
import unittest
from pathlib import Path

import sa193_compare


class CompareTest(unittest.TestCase):
    def test_parses_positive_witness_tail(self) -> None:
        line = (
            "can solve Sb(7:3)[21,10] in 4 with [3:1] Sb(3:1)[3,4] "
            "took 0.125 totalsplits=17 pass=1 fast_solve=0\n"
        )
        verdict = sa193_compare.parse_verdict(line, 9)
        self.assertIsNotNone(verdict)
        assert verdict is not None
        self.assertEqual(verdict.key, ("Sb(7:3)[21,10]", 4))
        self.assertEqual(verdict.outcome, "yes")
        self.assertEqual(verdict.seconds, 0.125)
        self.assertEqual(verdict.splits, 17)

    def test_selects_lagging_run_and_compares_exact_calls(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            lag = root / "lag.log"
            lead = root / "lead.log"
            lag.write_text(
                "can't solve Sb(5:2)[10,7] in 3 took 2 totalsplits=20 pass=2 fast_solve=0\n"
                "can't solve Sb(7:3)[21,10] in 4 took 10 totalsplits=30 pass=2 fast_solve=0\n"
                "can't solve Sb(10:5)[50,15] in 5 took 30 totalsplits=40 pass=2 fast_solve=0\n",
                encoding="utf-8",
            )
            lead.write_text(
                "can't solve Sb(5:2)[10,7] in 3 took 1 totalsplits=10 pass=2 fast_solve=0\n"
                "can't solve Sb(7:3)[21,10] in 4 took 5 totalsplits=15 pass=2 fast_solve=0\n"
                "can't solve Sb(10:5)[50,15] in 5 took 20 totalsplits=25 pass=2 fast_solve=0\n"
                "can't solve Sb(97:96)[9312,193] in 9 took 50 totalsplits=5 pass=2 fast_solve=0\n",
                encoding="utf-8",
            )
            lag_summary = sa193_compare.scan(lag, "run8", 2)
            lead_summary = sa193_compare.scan(lead, "run3", 2)
            output = sa193_compare.format_summary(lag_summary, lead_summary)
            self.assertIn("behind=run8 (0/16 roots, 3 verdicts)", output)
            self.assertIn("30", output)
            self.assertIn("20", output)
            self.assertIn("1.50x", output)
            self.assertIn("slow calls (CPU incl/~self) from run8", output)
            self.assertNotIn("inclusive / self", output)

    def test_estimates_self_between_same_level_verdicts(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            log = Path(directory) / "calls.log"
            log.write_text(
                "can't solve Sb(5:2)[10,7] in 3 took 2 totalsplits=1 pass=2 fast_solve=0\n"
                "can't solve Sb(7:3)[21,10] in 4 took 10 totalsplits=2 pass=2 fast_solve=0\n"
                "can't solve Sb(6:2)[12,8] in 3 took 3 totalsplits=3 pass=2 fast_solve=0\n"
                "can't solve Sb(8:3)[24,11] in 4 took 9 totalsplits=4 pass=2 fast_solve=0\n",
                encoding="utf-8",
            )
            summary = sa193_compare.scan(log, "run", 10)
            verdicts = {verdict.key: verdict for verdict in summary.top_slow}
            first = verdicts[("Sb(7:3)[21,10]", 4)]
            second = verdicts[("Sb(8:3)[24,11]", 4)]
            self.assertIsNone(first.estimated_self)
            self.assertEqual(second.estimated_self, 6)


if __name__ == "__main__":
    unittest.main()
