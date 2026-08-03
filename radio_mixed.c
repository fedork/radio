/* Decide the mixed children the scalable recursions bottom out in, and print the
 * split each one uses, so the descent can be followed by hand.
 *
 * docs/conjectures.md#scalable-constructions-for-m5-and-m6-2026-08-03 reduces
 *
 *     n(k,6) = n(k-1,4) + n(k-1,5)
 *
 * to three obligations, of which only the mixed child is open:
 *
 *     Sb( n(k-1,4):2 , n(k-1,5):4 )   solvable in k-1.
 *
 * Usage:
 *   ./radio_mixed chain <k> <n1> <m1> <n2> <m2>
 *       decide the state, then follow its own recursion: take the split the
 *       solver reports, print the three children, and recurse into whichever
 *       are non-trivial.  One level per line, prefixed DESCEND.
 *
 *   ./radio_mixed one <k> <n1> <m1> [<n2> <m2> ...]
 *       decide a single state (verdict line comes from the engine itself).
 */
#include "radiobase.c"

static int decide(int *sb, int size, int k) {
    return canSolveB(sb, size, k, NO_DEADLINE);
}

/* Re-derive the winning split by brute force over top-level splits, so the
 * reported split is checked rather than scraped from a log line. */
static int find_split(int *sb, int size, int k, int *out_a, int *out_b) {
    int sb0[8], sb1[16], sb2[8];
    if (size > 4) return 0;
    int a[4], b[4];
    int i;
    /* odometer over (a_i, b_i) for each part */
    int n[4], m[4];
    for (i = 0; i < size; i++) { n[i] = sbb_to_n1[sb[i]]; m[i] = sbb_to_n2[sb[i]]; a[i] = 0; b[i] = 0; }
    while (1) {
        for (i = 0; i < size; i++) {
            sb2[i] = getSbb(a[i], b[i]);
            sb0[i] = getSbb(n[i] - a[i], m[i] - b[i]);
            sb1[2 * i] = getSbb(a[i], m[i] - b[i]);
            sb1[2 * i + 1] = getSbb(n[i] - a[i], b[i]);
        }
        if (decide(sb2, size, k - 1) == TRUE
            && decide(sb0, size, k - 1) == TRUE
            && decide(sb1, size * 2, k - 1) == TRUE) {
            for (i = 0; i < size; i++) { out_a[i] = a[i]; out_b[i] = b[i]; }
            return 1;
        }
        /* increment odometer */
        for (i = 0; i < size; i++) {
            if (++b[i] <= m[i]) break;
            b[i] = 0;
            if (++a[i] <= n[i]) break;
            a[i] = 0;
            if (i == size - 1) return 0;
        }
        if (i == size) return 0;
    }
}

static void show(const char *tag, int depth, int *sb, int size, int k) {
    int i;
    printf("DESCEND %d %s k=%d ", depth, tag, k);
    for (i = 0; i < size; i++) if (sb[i] > 0) printf("%s ", sbb_to_str[sb[i]]);
    printf("\n");
    fflush(stdout);
}

static void chain(int *sb, int size, int k, int depth) {
    int compact[16], csize = 0, i;
    for (i = 0; i < size; i++) if (sb[i] > 1) compact[csize++] = sb[i];
    if (csize == 0 || k <= 0) return;
    if (csize > 4) { printf("DESCEND %d stop (too many parts)\n", depth); return; }
    int a[4], b[4];
    if (!find_split(compact, csize, k, a, b)) {
        printf("DESCEND %d NO-SPLIT k=%d\n", depth, k);
        return;
    }
    printf("SPLIT %d k=%d [", depth, k);
    for (i = 0; i < csize; i++) printf("%s%d:%d", i ? "," : "", a[i], b[i]);
    printf("] of ");
    for (i = 0; i < csize; i++) printf("%s ", sbb_to_str[compact[i]]);
    printf("\n");
    int sb0[8], sb1[16], sb2[8];
    for (i = 0; i < csize; i++) {
        int n = sbb_to_n1[compact[i]], m = sbb_to_n2[compact[i]];
        sb2[i] = getSbb(a[i], b[i]);
        sb0[i] = getSbb(n - a[i], m - b[i]);
        sb1[2 * i] = getSbb(a[i], m - b[i]);
        sb1[2 * i + 1] = getSbb(n - a[i], b[i]);
    }
    show("out2", depth, sb2, csize, k - 1);
    show("out0", depth, sb0, csize, k - 1);
    show("out1", depth, sb1, csize * 2, k - 1);
    chain(sb1, csize * 2, k - 1, depth + 1);
}

/* Every working top-level split of a two-part state, printed as
 * SPLITOK a1:b1,a2:b2 | out2 | out0 | out1
 * so the window can be read off the way the one-part windows in
 * docs/conjectures.md#scalable-constructions-for-m5-and-m6-2026-08-03 were. */
static void all_splits(int *sb, int size, int k) {
    int n[4], m[4], a[4], b[4], i;
    int sb0[8], sb1[16], sb2[8];
    long count = 0;
    for (i = 0; i < size; i++) { n[i] = sbb_to_n1[sb[i]]; m[i] = sbb_to_n2[sb[i]]; }
    for (a[0] = 0; a[0] <= n[0]; a[0]++)
      for (b[0] = 0; b[0] <= m[0]; b[0]++)
        for (a[1] = 0; a[1] <= (size > 1 ? n[1] : 0); a[1]++)
          for (b[1] = 0; b[1] <= (size > 1 ? m[1] : 0); b[1]++) {
            for (i = 0; i < size; i++) {
                sb2[i] = getSbb(a[i], b[i]);
                sb0[i] = getSbb(n[i] - a[i], m[i] - b[i]);
                sb1[2 * i] = getSbb(a[i], m[i] - b[i]);
                sb1[2 * i + 1] = getSbb(n[i] - a[i], b[i]);
            }
            if (decide(sb2, size, k - 1) != TRUE) continue;
            if (decide(sb0, size, k - 1) != TRUE) continue;
            if (decide(sb1, size * 2, k - 1) != TRUE) continue;
            count++;
            printf("SPLITOK ");
            for (i = 0; i < size; i++) printf("%s%d:%d", i ? "," : "", a[i], b[i]);
            printf(" | "); printSb(sb2, size);
            printf(" | "); printSb(sb0, size);
            printf(" | "); printSb(sb1, size * 2);
            printf("\n"); fflush(stdout);
          }
    printf("SPLITCOUNT %ld\n", count);
}

int main(int argc, char **argv) {
    init();
    if (argc < 4) { printf("usage: radio_mixed chain|one k n1 m1 [n2 m2 ...]\n"); exit(1); }
    int k = atoi(argv[2]);
    int size = (argc - 3) / 2, i;
    int sb[8];
    for (i = 0; i < size; i++) sb[i] = getSbb(atoi(argv[3 + 2 * i]), atoi(argv[4 + 2 * i]));
    printSb(sb, size); printf(" in %d\n", k);
    int r = decide(sb, size, k);
    printf("VERDICT %s\n", r == TRUE ? "solvable" : (r == FALSE ? "unsolvable" : "MAYBE"));
    fflush(stdout);
    if (r == TRUE && strcmp(argv[1], "chain") == 0) chain(sb, size, k, 0);
    if (r == TRUE && strcmp(argv[1], "allsplits") == 0) all_splits(sb, size, k);
    return 0;
}
