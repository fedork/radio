// Walk m upward at a fixed n1, one k, printing one verdict per cell.
//
// Why this and not radio_pareto: the staircase driver steps *right* while a cell is solvable and
// down when it is not, tracing the frontier from the diagonal out to m=1. That is the wrong axis
// for the Sa ladder question. Sa(n) in k+1 is solvable iff some first test of size t leaves
// Sa(t)@k, Sa(n-t)@k and Sb(t : n-t)@k all solvable, so with Sa(10)=192 the k=11 value is
//
//     Sa(11) = max_m [ min(192, n(10,m)) + m ],
//
// and the number that decides it is max{m : n(10,m) >= 192} - a *vertical* scan at n1=192. The
// staircase would instead prove n(10,136) exactly, which is more work than the question needs.
// Cross-check of the identity one level down: Sa(193)'s sixteen roots are Sb(112:81)..Sb(97:96),
// exactly t in [81,112] up to symmetry, and run10 refuting Sb(112:81)@9 is the statement
// max{m : n(9,m) >= 112} = 80, giving Sa(10) = 112 + 80 = 192.
//
//   tools/build_radio.py -O3 -DMAX_K=10 -DMAX_N=330 -DMAX_PART_N=330 radio_sb_walk.c -o radio_sb_walk
//   ./radio_sb_walk [cache] <k> <n1> <m_start> <m_end>
//
// Ascending m is deliberate: each cell's facts warm the next, which is the effect measured on
// 2026-08-03 when Sa(112)@9 cost an eighth of Sa(111)@9 purely because the k<=8 memo was already
// populated. Start *below* the expected crossing so the walk banks cheap positives first.
//
// Every query runs with NO_DEADLINE, so a verdict is never a budget artifact: TRUE and FALSE are
// both real. The cost is that the first unsolvable cell can run indefinitely - refuting it is the
// prohibitive direction (measured: ~300-600x per level, and Sb(112:81)@9 alone cost 100,723 CPU s
// in run10). So bound the *run*, not the query: wrap in tools/capped_run.sh and keep the last
// printed WALK line. A run killed mid-cell proves nothing about that cell.
//
// Exit 0 if the walk ended on a printed FALSE (the crossing was found), 1 if it exhausted m_end
// with every cell solvable, 3 on usage error.
#include "radiobase.c"

static double secs(clock_t t0) { return (double)(clock() - t0) / CLOCKS_PER_SEC; }

int main(int argc, char **argv) {
    init();

    // The optional leading cache path makes the count even; radio_one.c uses the same trick.
    int offset = (argc % 2 == 0) ? 1 : 0;
    if (argc - offset != 5) {
        printf("usage: %s [cache] k n1 m_start m_end\n", argv[0]);
        return 3;
    }
    int k = atoi(argv[offset + 1]);
    int n1 = atoi(argv[offset + 2]);
    int m_start = atoi(argv[offset + 3]);
    int m_end = atoi(argv[offset + 4]);
    if (k < 1 || n1 < 1 || m_start < 1 || m_end < m_start) {
        printf("usage: %s [cache] k n1 m_start m_end  (need k,n1,m_start >= 1 and m_end >= m_start)\n",
               argv[0]);
        return 3;
    }
    if (offset > 0) parse_file(argv[1]);

    printf("=== Sb walk: k=%d n1=%d m=%d..%d  MAX_K=%d MAX_N=%d MAX_PART_N=%d cache=%s\n",
           k, n1, m_start, m_end, MAX_K, MAX_N, MAX_PART_N, offset > 0 ? argv[1] : "(none)");
    fflush(stdout);

    int m;
    for (m = m_start; m <= m_end; m++) {
        int sb[1];
        sb[0] = getSbb(n1, m);
        clock_t t0 = clock();
        int r = canSolveB(sb, 1, k, NO_DEADLINE);
        double sec = secs(t0);
        printf("WALK Sb(%d:%d) in %d = %s  (%.1f s)  ", n1, m, k,
               r == TRUE ? "SOLVABLE" : r == FALSE ? "UNSOLVABLE" : "MAYBE", sec);
        printSb(sb, 1);
        printf("\n");
        fflush(stdout);
        if (r != TRUE) {
            printf("=== crossing: largest solvable m at n1=%d is %d, so n(%d,%d) >= %d and "
                   "n(%d,%d) < %d\n", n1, m - 1, k, m - 1, n1, k, m, n1);
            printf("=== Sa(%d) >= %d follows if Sa(%d)@%d and Sa(%d)@%d also hold\n",
                   k + 1, n1 + m - 1, n1, k, m - 1, k);
            fflush(stdout);
            return 0;
        }
    }
    printf("=== every cell m=%d..%d is solvable at n1=%d; the crossing is above m_end\n",
           m_start, m_end, n1);
    return 1;
}
