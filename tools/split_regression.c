/* Small exhaustive regression corpus for changes to radiobase.c.

   It is intentionally a driver, not an independent solver: compile the same source against two
   engine checkouts and compare the CHECK lines.  The bounds cover every information-feasible
   one-part state of total width <=18 and two-part state whose individual totals are <=10, for
   k=1..5.  NO_DEADLINE makes every emitted answer definitive.

     clang -O3 -DMAX_K=5 -DMAX_N=24 tools/split_regression.c -o /tmp/split_regression
     /tmp/split_regression | grep '^CHECK' > /tmp/checks

   RADIOBASE_PATH can name an engine in another checkout for old/new comparisons:

     clang -O3 -DMAX_K=5 -DMAX_N=24 \
       -DRADIOBASE_PATH='"/path/to/old/radiobase.c"' tools/split_regression.c -o /tmp/old
*/
#ifndef RADIOBASE_PATH
#define RADIOBASE_PATH "../radiobase.c"
#endif
#include RADIOBASE_PATH

int main(void) {
    int k;
    int a;
    int b;
    int checks = 0;
    int last_sbb;

    init();
    last_sbb = MAX_SBB;
    while (last_sbb > 0 && sbb_to_n2[last_sbb] == 0) last_sbb--;
    for (k = 1; k <= MAX_K; k++) {
        for (a = 2; a <= last_sbb; a++) {
            int one[1] = { a };
            int result;
            if (sbb_to_n1[a] + sbb_to_n2[a] > 18) continue;
            if (sb_pairs[a] > power3[k]) continue;
            result = canSolveB(one, 1, k, NO_DEADLINE);
            printf("CHECK1 %d %d %d %d\n",
                   k, sbb_to_n1[a], sbb_to_n2[a], result);
            checks++;
        }
        for (a = 2; a <= last_sbb; a++) {
            if (sbb_to_n1[a] + sbb_to_n2[a] > 10) continue;
            for (b = 2; b <= a; b++) {
                int two[2] = { a, b };
                int result;
                if (sbb_to_n1[b] + sbb_to_n2[b] > 10) continue;
                if (sb_pairs[a] + sb_pairs[b] > power3[k]) continue;
                result = canSolveB(two, 2, k, NO_DEADLINE);
                printf("CHECK2 %d %d %d %d %d %d\n",
                       k, sbb_to_n1[a], sbb_to_n2[a],
                       sbb_to_n1[b], sbb_to_n2[b], result);
                checks++;
            }
        }
    }
    printf("CHECK_SUMMARY %d\n", checks);
    return 0;
}
