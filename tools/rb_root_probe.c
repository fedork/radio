/* Decide the complete first-test mass relaxation for one Sb state.

   This does not recursively solve any child.  It builds the same theorem-filtered per-part split
   tables and joint suffix reachability DP used by rb_dead, then asks rb_dead(0,0,0,0): is there any
   legal first test whose three children all fit beneath 3^(k-1)?  DEAD is an exact refutation;
   ALIVE only means that this necessary condition passes.  It then reports the exact hereditary
   suffix at which rb_dead becomes universally vacuous, plus two cheap sufficient certificates;
   set RB_PLIABILITY_VERBOSE=1 for every suffix row.

     tools/build_radio.py -O3 -DMAX_K=7 -DMAX_N=<sum of all sides> \
         tools/rb_root_probe.c -o rb_root_probe
     ./rb_root_probe <k> <n1> <m1> [<n2> <m2> ...]
*/

#ifndef RADIOBASE_PATH
#define RADIOBASE_PATH "../radiobase.c"
#endif
#ifndef RADIO_RB_PLIABILITY_DIAGNOSTIC
#define RADIO_RB_PLIABILITY_DIAGNOSTIC
#endif
#include RADIOBASE_PATH

/* The full solver initializer materializes dominance closures used by its result cache.  At large
   MAX_N those deliberately dominate startup, but this probe never consults the cache.  Build only
   the exact (n:m) id map, theorem bases, and lazy split-table index that rb_build needs. */
static void init_rb_probe(void) {
    int value = 1;
    for (int k = 0; k <= MAX_K; k++) {
        power3[k] = value;
        value *= 3;
    }
    init_singleton_majorization();

    int sbb = 0;
    sb_pairs[0] = 0;
    sprintf(sbb_to_str[0], "0:0");
    for (int prod = 1; prod <= MAX_PROD; prod++) {
        for (int n2 = MAX_N - 1; n2 > 0; n2--) {
            int n1 = prod / n2;
            if (n1 < n2 || n1 + n2 > MAX_N || n1 * n2 != prod) continue;
            n_to_sbb[n1][n2] = ++sbb;
            if (sbb > MAX_SBB) {
                fprintf(stderr, "part index overflow sbb=%d MAX_SBB=%d\n", sbb, MAX_SBB);
                exit(3);
            }
            sbb_to_n1[sbb] = n1;
            sbb_to_n2[sbb] = n2;
            sb_pairs[sbb] = prod;
            sprintf(sbb_to_str[sbb], "%d:%d", n1, n2);
        }
    }
    sbb_splits = calloc((size_t)MAX_SBB + 1, sizeof(*sbb_splits));
    if (sbb_splits == NULL) {
        fprintf(stderr, "out of memory allocating split-table index\n");
        exit(3);
    }
}

int main(int argc, char **argv) {
    radio_print_provenance();
    if (argc < 4 || (argc & 1)) {
        fprintf(stderr, "usage: %s k n1 m1 [n2 m2 ...]\n", argv[0]);
        return 2;
    }

    int k = atoi(argv[1]);
    int input_size = (argc - 2) / 2;
    if (k < 1 || k > MAX_K || input_size > 16) {
        fprintf(stderr, "unsupported k=%d or parts=%d (MAX_K=%d, RB max parts=16)\n",
                k, input_size, MAX_K);
        return 2;
    }

    init_rb_probe();

    int state[input_size];
    int size = 0;
    int pairs_full = 0;
    int total_coins = 0;
    for (int i = 0; i < input_size; i++) {
        int n = atoi(argv[2 + 2 * i]);
        int m = atoi(argv[3 + 2 * i]);
        if (n < 0 || m < 0 || n > MAX_N || m > MAX_N
            || min(n, m) > MAX_N / 2 || n + m > MAX_N - total_coins) {
            fprintf(stderr, "part %d:%d exceeds compiled bounds MAX_N=%d\n", n, m, MAX_N);
            return 2;
        }
        total_coins += n + m;
        int sbb = getSbb(n, m);
        pairs_full += sb_pairs[sbb];
        if (sbb > 1) state[size++] = sbb;  /* Unit Group Elimination, as in canSolveB. */
    }
    if (size > 1) sort1(state, size);

    printf("RB_ROOT k=%d state=", k);
    printSb(state, size);
    printf(" mass=%d cap=%d parts=%d", pairs_full, power3[k - 1], size);

    if (pairs_full > power3[k]) {
        printf(" result=DEAD reason=parent-information-bound\n");
        return 0;
    }
    if (size == 0) {
        printf(" result=ALIVE reason=unit-state\n");
        return 0;
    }
    if (!star_expansion_majorization_can_solve(state, size, k)) {
        printf(" result=DEAD reason=parent-star-majorization\n");
        return 0;
    }
    if (power3[k - 1] >= RB_MAXCAP) {
        printf(" result=UNAVAILABLE reason=cap-limit\n");
        return 0;
    }

    splits *tables[size];
    clock_t table_start = clock();
    printf(" options=");
    for (int i = 0; i < size; i++) {
        tables[i] = ensure_splits(state[i], k);
        printf("%s%d", i ? ":" : "", tables[i]->size);
    }
    double table_ms = 1000.0 * (double)(clock() - table_start) / CLOCKS_PER_SEC;

    clock_t build_start = clock();
    radio_reachability_state *rb =
        radio_search_context_reachability(&radio_default_search_context);
    rb->on = 1;
    rb_build(rb, tables, state, size, power3[k - 1]);
    int dead = rb_dead(rb, 0, 0, 0, 0);
    double build_ms = 1000.0 * (double)(clock() - build_start) / CLOCKS_PER_SEC;

    printf(" result=%s table_build_ms=%.3f rb_build_ms=%.3f\n",
           dead ? "DEAD" : "ALIVE", table_ms, build_ms);
    rb_report_pliability(
        rb, tables, state, stdout, getenv("RB_PLIABILITY_VERBOSE") != NULL);
    rb_release(rb);
    return 0;
}
