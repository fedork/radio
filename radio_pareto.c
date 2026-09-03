// Walk the Sb Pareto frontier for a given k, as a staircase.
//
// Starting from a cell on or below the frontier, step right while the state is solvable and
// down when it is not, which traces the whole frontier from the diagonal out to m = 1. Each
// step prints one line, so the output is directly comparable against data/pareto_sb.csv.
//
// This is the generic replacement for radioSbPareto.c, which hardcodes k = 9 and MAX_N = 204.
// It is a heavy, well-defined workload with a *known* answer for k <= 8, which makes it the
// right benchmark for anything that only shows up on long searches - the finite-budget machinery
// in particular never fires on the Sa ladder, because no single search there gets near the
// 1000 / 100 / 10 nominal-second / MIN_DEADLINE budget cascade. The default nominal clock counts
// accepted split prefixes; -DRADIO_CPU_BUDGET restores the historical process-CPU interpretation.
//
//   tools/build_radio.py -O3 -DMAX_K=8 -DMAX_N=258 radio_pareto.c -o radio_pareto
//   tools/capped_run.sh --seconds 3600 --rss-gb 16 -- ./radio_pareto 8 56 55 > out.txt
//
// Usage:
//   radio_pareto <k> <n1> <n2> [cache_file]
//   radio_pareto --bootstrap-diagonal <k> <n1> <n2> [cache_file]
//
// The first form starts at a known point on or below the frontier and walks toward m=1.  The
// bootstrap form first checks the supplied diagonal seed, advances alternately on the two shores
// until it finds the first unsolvable state, and then walks the complete staircase to m=1.  This
// is the form used for the K=9 production walk from Sb(55:55).

#include <errno.h>

#include "radiobase.c"

static int parse_int(const char *text, const char *name) {
    char *end = NULL;
    long value;
    errno = 0;
    value = strtol(text, &end, 10);
    if (errno || end == text || *end || value < 1 || value > INT_MAX) {
        fprintf(stderr, "invalid %s: %s\n", name, text);
        exit(2);
    }
    return (int)value;
}

static int solve_and_report(int k, int n1, int n2, const char *phase, long long step) {
    int sb[1];
    int verdict;
    sb[0] = getSbb(n1, n2);
    verdict = canSolveB(sb, 1, k, NO_DEADLINE);

    printf("result %s ", verdict == TRUE ? "  can solve" :
                             verdict == FALSE ? "can't solve" : "       MAYBE");
    printSb(sb, 1);
    printf(" in %d\n", k);
    printf("PARETO STEP phase=%s step=%lld verdict=%s n1=%d m=%d\n",
           phase, step, verdict == TRUE ? "SOLVABLE" :
                        verdict == FALSE ? "UNSOLVABLE" : "MAYBE", n1, n2);
    fflush(stdout);
    return verdict;
}

int main(int argc, char **argv) {
    int arg = 1;
    int bootstrap_diagonal = 0;
    if (arg < argc && !strcmp(argv[arg], "--bootstrap-diagonal")) {
        bootstrap_diagonal = 1;
        arg++;
    }
    if (argc - arg != 3 && argc - arg != 4) {
        printf("usage: %s [--bootstrap-diagonal] <k> <n1> <n2> [cache_file]\n",
               argv[0]);
        return 2;
    }
    int k = parse_int(argv[arg++], "k");
    int n1 = parse_int(argv[arg++], "n1");
    int n2 = parse_int(argv[arg++], "n2");
    char *cache_file = arg < argc ? argv[arg] : NULL;

    if (n1 < n2) {
        int tmp = n1;
        n1 = n2;
        n2 = tmp;
    }
    if (bootstrap_diagonal && n1 != n2) {
        fprintf(stderr, "--bootstrap-diagonal requires n1=n2\n");
        return 2;
    }

    // MAX_N undersizing is silent - the result cache prunes on it, so you lose caching and
    // may or may not get an abort. The largest queried state is the first negative at m=2,
    // Sb(2^k:2), because the exact m=2 frontier is 2^k-1. The final m=1 upper neighbor is handled
    // by dichotomy below and is not queried.
    if (k >= 30) {
        fprintf(stderr, "k=%d is too large for the integer state representation\n", k);
        return 2;
    }
    int max_n1 = 1 << k;
    int needed = max_n1 + 2;
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
    if (cache_file) parse_file(cache_file);

    printf("PARETO START k=%d start_n1=%d start_m=%d mode=%s cache=%s\n",
           k, n1, n2, bootstrap_diagonal ? "bootstrap-diagonal" : "staircase",
           cache_file ? cache_file : "none");
    fflush(stdout);

    long long steps = 0;
    int cells = 0;
    int verdict = MAYBE;
    int have_verdict = 0;
    int last_true_n1 = -1;
    int last_true_m = -1;

    if (bootstrap_diagonal) {
        verdict = solve_and_report(k, n1, n2, "seed", ++steps);
        if (verdict != TRUE) {
            printf("PARETO ABORT reason=diagonal-seed-not-solvable verdict=%s n1=%d m=%d\n",
                   verdict == FALSE ? "UNSOLVABLE" : "MAYBE", n1, n2);
            fflush(stdout);
            return 3;
        }
        last_true_n1 = n1;
        last_true_m = n2;

        for (;;) {
            if (n1 > n2) n2++;
            else n1++;
            verdict = solve_and_report(k, n1, n2, "bootstrap", ++steps);
            if (verdict == MAYBE) {
                printf("PARETO ABORT reason=unexpected-maybe phase=bootstrap n1=%d m=%d\n",
                       n1, n2);
                fflush(stdout);
                return 3;
            }
            if (verdict == FALSE) {
                have_verdict = 1;
                printf("PARETO BOOTSTRAP_END first_unsolvable_n1=%d first_unsolvable_m=%d "
                       "last_solvable_n1=%d last_solvable_m=%d\n",
                       n1, n2, last_true_n1, last_true_m);
                fflush(stdout);
                break;
            }
            last_true_n1 = n1;
            last_true_m = n2;
        }
    }

    while (n2 > 0 && n1 <= max_n1) {
        if (!have_verdict)
            verdict = solve_and_report(k, n1, n2, "frontier", ++steps);
        have_verdict = 0;
        if (verdict == MAYBE) {
            printf("PARETO ABORT reason=unexpected-maybe phase=frontier n1=%d m=%d\n",
                   n1, n2);
            fflush(stdout);
            return 3;
        }
        if (verdict == TRUE) {
            last_true_n1 = n1;
            last_true_m = n2;
            n1++;
        } else {
            if (last_true_m == n2) {
                printf("PARETO CELL k=%d m=%d max_n1=%d first_unsolvable_n1=%d\n",
                       k, n2, last_true_n1, n1);
                cells++;
            }
            n2--;
        }
        fflush(stdout);
    }

    // Sb(2^k:1) is the final dichotomy cell. Avoid constructing the impossible next state, which
    // would require MAX_N one larger solely to print its already-theoretical upper neighbor.
    if (n2 == 1 && n1 == max_n1 + 1 && last_true_m == 1) {
        printf("PARETO CELL k=%d m=1 max_n1=%d first_unsolvable_n1=%d "
               "upper_reason=dichotomy\n", k, max_n1, max_n1 + 1);
        cells++;
        n2 = 0;
    }
    printf("PARETO DONE k=%d cells=%d steps=%lld final_n1=%d final_m=%d\n",
           k, cells, steps, n1, n2);
    fflush(stdout);
    return 0;
}
