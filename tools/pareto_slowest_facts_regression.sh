#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

actual=$(tools/pareto_slowest_facts.py --limit 4 <<'LOG'
can't solve Sb(3:2)[6,5] in 2 took 12 totalsplits=10 pass=2 fast_solve=0
still solving in 2 pass=2 fast_solve=0 Sb(3:2)[6,5] trying Sb(1:1)[1,2] elapsed 20/30 left=1/2
can solve Sa(5)[10,5] in 3 with following: Sa(3), Sa(2), Sb(3:2) took 25 totalsplits=1
still solving in 3 pass=1 fast_solve=0 Sb(4:3)[12,7] trying Sb(2:1)[2,3] elapsed 30/40 left=2/4
can't solve Sb(2:1)[2,3] in 2 took 40 totalsplits=20 pass=2 fast_solve=0
LOG
)

grep -Fq '1.      40s completed UNSOLVABLE       Sb(2:1)@2 (took=40, elapsed=-)' <<<"$actual"
grep -Fq '2.      30s elapsed   NO_FINAL_VERDICT Sb(4:3)@3 (took=-, elapsed=30)' <<<"$actual"
grep -Fq '3.      25s completed SOLVABLE         Sa(5)@3 (took=25, elapsed=-)' <<<"$actual"
grep -Fq '4.      20s elapsed   UNSOLVABLE       Sb(3:2)@2 (took=12, elapsed=20)' <<<"$actual"
grep -Fq 'slowest completed   rank=final took only; inclusive process CPU seconds' <<<"$actual"
grep -Fq '1.      40s UNSOLVABLE       Sb(2:1)@2 (highest_elapsed=-)' <<<"$actual"
grep -Fq '2.      25s SOLVABLE         Sa(5)@3 (highest_elapsed=-)' <<<"$actual"
grep -Fq '3.      12s UNSOLVABLE       Sb(3:2)@2 (highest_elapsed=20)' <<<"$actual"
grep -Fq 'current stack       latest activity per level, current k=2 up to root k=3' <<<"$actual"
grep -Fq 'k=3 [solving] pass=1 fast_solve=0 Sb(4:3)[12,7]  elapsed 30/40 left=2/4' <<<"$actual"
grep -Fq "k=2 [done] can't solve Sb(2:1)[2,3] in 2 took 40" <<<"$actual"

echo 'pareto slowest-facts regression passed'
