// Decide ONE Sb state, and nothing else.
//
// radio_full enumerates every top-level split, which is far more work than a verdict needs and is
// how several runs were killed without one. This driver asks canSolveB the single question and
// prints the answer, so a specific missing certificate fact can be proved on its own.
//
// Written 2026-08-04 for the sixteen k=8 facts the Sa(193) certificate is missing: each of the
// sixteen k=9 roots fails on exactly one split, whose only possible refutation is a two-part k=8
// child (for Sb(112:81) it is Sb(74:40, 41:38)). See docs/certificate.md.
//
//   clang -O3 -DMAX_K=<k> -DMAX_N=<sum of all sides> radio_one.c -o radio_one
//   ./radio_one [cache] <k> <n1> <m1> [<n2> <m2> ...]
//
// Exit 0 solvable, 1 unsolvable, 2 MAYBE (deadline), 3 usage. MAYBE is not a refutation.

#include "radiobase.c"

int main(int argc, char **argv) {
    init();
    if (argc < 4) { printf("usage: %s [cache] k n1 m1 [n2 m2 ...]\n", argv[0]); return 3; }

    int offset = (argc % 2 == 1) ? 1 : 0;
    int k = atoi(argv[offset + 1]);
    int size = (argc - offset - 2) / 2;
    int sb[size], i;
    for (i = 0; i < size; i++)
        sb[i] = getSbb(atoi(argv[offset + 2 + i * 2]), atoi(argv[offset + 3 + i * 2]));
    if (offset > 0) parse_file(argv[1]);

    printf("query k=%d ", k);
    printSb(sb, size);
    printf("\n");
    fflush(stdout);

    clock_t t0 = clock();
    int r = canSolveB(sb, size, k, NO_DEADLINE);
    double sec = (double)(clock() - t0) / CLOCKS_PER_SEC;

    printf("VERDICT %s  k=%d  %.1f s  ",
           r == TRUE ? "SOLVABLE" : r == FALSE ? "UNSOLVABLE" : "MAYBE", k, sec);
    printSb(sb, size);
    printf("\n");
    return r == TRUE ? 0 : r == FALSE ? 1 : 2;
}
