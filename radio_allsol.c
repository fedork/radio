// Enumerate EVERY working top-level split of a state, with prefix pruning.
//
// `all_solutions` in radiobase.c is a flat odometer: it visits all prod (n_i+1)(m_i+1) tuples
// and applies the counting bound only at the leaf. That is fine for small states and hopeless
// for saturated ones - Sb(8:6,7:7,8:5,11:4,7:4,15:1,5:3) in 5 has 4.01e11 tuples, ~6 days,
// of which 2.3e8 survive the counting bound. Factor 1734 thrown away.
//
// This applies the bound at every prefix instead. Mass is exactly conserved across the three
// children of a split (ab + (n-a)(m-b) + a(m-b) + (n-a)b = nm), so the running per-child
// sums only grow, and a prefix whose partial sum already exceeds 3^(k-1) is dead along with
// everything under it.
//
// Note the *lower* bound is deliberately absent. Requiring each child to end at
// >= mass - 2*3^(k-1) looks like an extra prune but is implied: if p_c + M_rem < mass-2cap
// then p_a + p_b > 2cap, so one of them already exceeds cap. Measured on the state above: it
// cuts 0.0% of 427,673,655 prefix nodes. Do not re-add it.
//
//   tools/build_radio.py -O3 -DMAX_K=5 -DMAX_N=91 radio_allsol.c -o radio_allsol
//   tools/capped_run.sh --seconds 3600 --rss-gb 12 -- ./radio_allsol 5 8 6 7 7 8 5 11 4 7 4 15 1 5 3
//
// Usage: radio_allsol <k> <n1> <m1> [<n2> <m2> ...]
//
// Prints one `solution [a:b,...]` line per working split, then a per-part summary of which
// (a,b) participate in at least one solution and how often, then a totals line.

#include "radiobase.c"

#define MAXP 64

static int P;                    /* part count */
static int pn[MAXP], pm[MAXP];   /* part dimensions, sorted as canSolveB sorts them */
static int psb[MAXP];            /* part sbb ids */
static int ta[MAXP], tb[MAXP];   /* current split choice per part */
static int suffix_mass[MAXP + 1];
static long long nodes = 0, leaves = 0, sols = 0;
static long long part_count[MAXP][MAX_N + 1][MAX_N + 1];
static int KK, CAP;
static int c0[MAXP], c1[2 * MAXP], c2[MAXP];

static void report_solution(void) {
    int i;
    sols++;
    printf("solution [");
    for (i = 0; i < P; i++) printf("%s%d:%d", i ? "," : "", ta[i], tb[i]);
    printf("] => ");
    printSb(c2, P); printSb(c1, 2 * P); printSb(c0, P);
    printf("\n");
    for (i = 0; i < P; i++) part_count[i][ta[i]][tb[i]]++;
    if ((sols & 1023) == 0) fflush(stdout);
}

/* depth = parts already assigned; s2/s0/s1 = their accumulated child masses */
static void rec(int depth, int s2, int s0, int s1) {
    if (depth == P) {
        leaves++;
        /* cheap cache-only screen first, then the real decision */
        if (canSolveB(c2, P, KK - 1, CACHE_ONLY) != FALSE &&
            canSolveB(c0, P, KK - 1, CACHE_ONLY) != FALSE &&
            canSolveB(c1, 2 * P, KK - 1, CACHE_ONLY) != FALSE &&
            canSolveB(c2, P, KK - 1, NO_DEADLINE) == TRUE &&
            canSolveB(c0, P, KK - 1, NO_DEADLINE) == TRUE &&
            canSolveB(c1, 2 * P, KK - 1, NO_DEADLINE) == TRUE)
            report_solution();
        return;
    }
    int n = pn[depth], m = pm[depth], a, b;
    for (a = 0; a <= n; a++) {
        for (b = 0; b <= m; b++) {
            int k2 = a * b, k0 = (n - a) * (m - b);
            int k1 = n * m - k2 - k0;
            int t2 = s2 + k2, t0 = s0 + k0, t1 = s1 + k1;
            if (t2 > CAP || t0 > CAP || t1 > CAP) continue;   /* the whole prune */
            nodes++;
            ta[depth] = a; tb[depth] = b;
            c2[depth] = getSbb(a, b);
            c0[depth] = getSbb(n - a, m - b);
            c1[depth * 2] = getSbb(a, m - b);
            c1[depth * 2 + 1] = getSbb(n - a, b);
            rec(depth + 1, t2, t0, t1);
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 4 || (argc - 2) % 2) {
        printf("usage: %s <k> <n1> <m1> [<n2> <m2> ...]\n", argv[0]);
        return 2;
    }
    KK = atoi(argv[1]);
    P = (argc - 2) / 2;
    if (P > MAXP) { printf("too many parts\n"); return 2; }
    int i, total_n = 0, mass = 0;
    for (i = 0; i < P; i++) {
        int n = atoi(argv[2 + i * 2]), m = atoi(argv[3 + i * 2]);
        if (n < m) { int t = n; n = m; m = t; }
        pn[i] = n; pm[i] = m; total_n += n + m; mass += n * m;
    }
    if (total_n > MAX_N) {
        printf("MAX_N too small: this state has %d coins, rebuild with -DMAX_N=%d\n",
               total_n, total_n);
        return 2;
    }
    if (KK > MAX_K) { printf("k=%d exceeds MAX_K=%d\n", KK, MAX_K); return 2; }

    init();
    CAP = power3[KK - 1];
    for (i = 0; i < P; i++) psb[i] = getSbb(pn[i], pm[i]);
    suffix_mass[P] = 0;
    for (i = P - 1; i >= 0; i--) suffix_mass[i] = suffix_mass[i + 1] + pn[i] * pm[i];

    printf("state ");
    printSb(psb, P);
    printf(" in %d   mass %d, child cap 3^%d = %d, saturation %.4f\n",
           KK, mass, KK - 1, CAP, (double)mass / power3[KK]);
    long long raw = 1;
    for (i = 0; i < P; i++) raw *= (long long)(pn[i] + 1) * (pm[i] + 1);
    printf("raw tuple space %lld\n", raw);
    fflush(stdout);

    rec(0, 0, 0, 0);

    printf("\n");
    for (i = 0; i < P; i++) {
        int a, b;
        printf("winners %d:%d =>", pn[i], pm[i]);
        for (a = 0; a <= pn[i]; a++)
            for (b = 0; b <= pm[i]; b++)
                if (part_count[i][a][b])
                    printf(" [%d:%d]x%lld", a, b, part_count[i][a][b]);
        printf("\n");
    }
    printf("\ntotals solutions=%lld leaves=%lld prefix_nodes=%lld raw=%lld"
           "  pruned_to=%.3g of raw\n",
           sols, leaves, nodes, raw, (double)leaves / (double)raw);
    return 0;
}
