/* Sweep the two-part frontier.
 *
 * For a fixed k and a fixed pair of m-sides (s,t), print the staircase
 *
 *     g(p) = max { q : Sb(p:s, q:t) solvable in k }
 *
 * which is well defined and non-increasing in p by Subgraph Monotonicity
 * (docs/theorems/subgraph-monotonicity.md), so the sweep can walk q downward
 * from the previous row instead of restarting.
 *
 * This is the object docs/conjectures.md calls "the 2-part mixed frontier":
 * the mixed child of a one-part split [a:b] of n:m is Sb(a:(m-b), (n-a):b),
 * so s+t = m and p+q = n, and it is what both scalable recursions bottom out in.
 *
 *   clang -O3 -DMAX_K=6 -DMAX_N=80 radio_2part.c -o radio_2part
 *   ./radio_2part <k> <s> <t>          one pair
 *   ./radio_2part <k>                  every pair with s >= t >= 1, s+t <= 12
 *
 * Output lines are  FRONT k s t p q  (machine readable), plus the solver's own
 * "can solve ... with [...]" witness lines on stdout.
 */
#include "radiobase.c"

static int solvable2(int p, int s, int q, int t, int k) {
    int sb[2];
    int size = 0;
    if (p > 0 && s > 0) sb[size++] = getSbb(p, s);
    if (q > 0 && t > 0) sb[size++] = getSbb(q, t);
    if (size == 0) return 1;
    return canSolveB(sb, size, k, NO_DEADLINE) == TRUE;
}

/* n(k,m): largest n with Sb(n:m) solvable in k. 0 if none. */
static int one_part_frontier(int m, int k) {
    int n = m;
    while (n + 1 <= MAX_N - m && (long long)(n + 1) * m <= power3[k]
           && solvable2(n + 1, m, 0, 0, k)) n++;
    return solvable2(n, m, 0, 0, k) ? n : 0;
}

/* MAX_N bounds the total coin count n1+n2 of every state the search touches, and a
 * test preserves that total exactly (a_i + (m_i-b_i) + (n_i-a_i) + b_i = n_i + m_i),
 * so the root is the worst case.  Undersizing it does not abort: the result cache
 * prunes on `n_remaining` and silently loses entries.  Check it up front. */
static void require_capacity(int p, int s, int q, int t) {
    int need = p + s + q + t;
    if (need > MAX_N) {
        printf("FATAL root has %d coins but MAX_N=%d; rebuild with -DMAX_N=%d\n",
               need, MAX_N, need + 8);
        exit(3);
    }
}

static void sweep_range(int k, int s, int t, int pmin, int pmax, int qstart) {
    int q = qstart;
    int p;
    require_capacity(pmax, s, qstart, t);
    printf("PAIR k=%d s=%d t=%d n(k,s)=%d n(k,t)=%d\n",
           k, s, t, one_part_frontier(s, k), one_part_frontier(t, k));
    for (p = pmin; p <= pmax; p++) {
        while (q > 0 && !solvable2(p, s, q, t, k)) q--;
        printf("FRONT %d %d %d %d %d\n", k, s, t, p, q);
        fflush(stdout);
        if (q == 0) break;
    }
}

static void sweep(int k, int s, int t) {
    sweep_range(k, s, t, 1, one_part_frontier(s, k), one_part_frontier(t, k));
}

int main(int argc, char **argv) {
    init();
    if (argc < 2) { printf("usage: radio_2part k [s t]\n"); exit(1); }
    int k = atoi(argv[1]);
    if (argc >= 7) {
        /* k s t pmin pmax qstart -- walk only the interesting part of the staircase */
        sweep_range(k, atoi(argv[2]), atoi(argv[3]),
                    atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
    } else if (argc >= 4) {
        sweep(k, atoi(argv[2]), atoi(argv[3]));
    } else {
        int mmax = (argc >= 3) ? atoi(argv[2]) : 12;
        int s, t;
        for (s = 1; s <= mmax; s++)
            for (t = 1; t <= s && s + t <= mmax; t++)
                sweep(k, s, t);
    }
    return 0;
}
