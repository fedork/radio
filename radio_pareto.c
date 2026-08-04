// Walk the Sb Pareto frontier for a given k, as a staircase.
//
// Starting from a cell on or below the frontier, step right while the state is solvable and
// down when it is not, which traces the whole frontier from the diagonal out to m = 1. Each
// step prints one line, so the output is directly comparable against data/pareto_sb.csv.
//
// This is the generic replacement for radioSbPareto.c, which hardcodes k = 9 and MAX_N = 204.
// It is a heavy, well-defined workload with a *known* answer for k <= 8, which makes it the
// right benchmark for anything that only shows up on long searches - the deadline machinery
// in particular never fires on the Sa ladder, because no single search there gets near the
// 1000s / 100s / 10s / MIN_DEADLINE budget cascade.
//
//   clang -O3 -DMAX_K=8 -DMAX_N=258 radio_pareto.c -o radio_pareto
//   tools/capped_run.sh --seconds 3600 --rss-gb 16 -- ./radio_pareto 8 56 55 > out.txt
//
// Usage: radio_pareto <k> <n1> <n2> [cache_file]

#include "radiobase.c"

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("usage: %s <k> <n1> <n2> [cache_file]\n", argv[0]);
        return 2;
    }
    int k = atoi(argv[1]);
    int n1 = atoi(argv[2]);
    int n2 = atoi(argv[3]);

    // MAX_N undersizing is silent - the result cache prunes on it, so you lose caching and
    // may or may not get an abort. The walk reaches n1 = 2^k at m = 1, so that is the bound.
    int needed = (1 << k) + 1;
    if (needed > MAX_N) {
        printf("MAX_N too small: this walk reaches %d coins, rebuild with -DMAX_N=%d\n",
               needed, needed);
        return 2;
    }
    if (k > MAX_K) {
        printf("k=%d exceeds MAX_K=%d, rebuild\n", k, MAX_K);
        return 2;
    }

    init();
    if (argc > 4) parse_file(argv[4]);

    while (n2 > 0 && n1 <= (1 << k)) {
        int sb[1];
        sb[0] = getSbb(n1, n2);
        if (canSolveB(sb, 1, k, NO_DEADLINE)) {
            printf("result   can solve ");
            n1++;
        } else {
            printf("result can't solve ");
            n2--;
        }
        printSb(sb, 1);
        printf(" in %d\n", k);
        fflush(stdout);
    }
    return 0;
}
