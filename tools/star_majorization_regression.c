/* Differential regression for the endpoint-only full-star majorization check.
 *
 * Build through tools/build_radio.py, for example:
 *   tools/build_radio.py -O2 -DMAX_K=6 -DMAX_N=16 \
 *       tools/star_majorization_regression.c -o /tmp/star-majorization-regression
 */

#include "../radiobase.c"

enum { TEST_MAX_PARTS = 24 };

static int explicit_reference(const int *sb, int size, int k) {
    int expanded[TEST_MAX_PARTS * MAX_N];
    int count = 0;
    long long left = 0;
    int right_len = singleton_base_len[k];
    int right_total = singleton_base_prefix[k][right_len - 1];
    for (int i = 0; i < size; i++) {
        int n = sbb_to_n1[sb[i]];
        int copies = sbb_to_n2[sb[i]];
        for (int q = 0; q < copies; q++) expanded[count++] = n;
    }
    sort1(expanded, count);
    for (int i = 0; i < count; i++) {
        int right = i < right_len ? singleton_base_prefix[k][i] : right_total;
        left += expanded[i];
        if (left > right) return FALSE;
    }
    return TRUE;
}

static unsigned next_random(unsigned *state) {
    *state = *state * 1664525u + 1013904223u;
    return *state;
}

static void compare_one(int *sb, int size, int k, unsigned long long *checked) {
    int expected = explicit_reference(sb, size, k);
    int actual = star_expansion_majorization_can_solve(sb, size, k);
    (*checked)++;
    if (expected != actual) {
        fprintf(stderr, "star majorization mismatch k=%d expected=%d actual=%d state=",
                k, expected, actual);
        printSb(sb, size);
        fputc('\n', stderr);
        exit(1);
    }
}

int main(void) {
    int ids[MAX_SBB];
    int count = 0;
    unsigned random_state = 1;
    unsigned long long checked = 0;

    init();
    for (int sbb = 1; sbb <= MAX_SBB; sbb++)
        if (sbb_to_n1[sbb] > 0) ids[count++] = sbb;

    /* Exhaust every one-, two- and three-part multiset at every compiled level. */
    for (int k = 0; k <= MAX_K; k++) {
        for (int a = 0; a < count; a++) {
            int sb1[1] = {ids[a]};
            compare_one(sb1, 1, k, &checked);
            for (int b = 0; b <= a; b++) {
                int sb2[2] = {ids[b], ids[a]}; /* Deliberately not n-sorted. */
                compare_one(sb2, 2, k, &checked);
                for (int c = 0; c <= b; c++) {
                    int sb3[3] = {ids[b], ids[a], ids[c]};
                    compare_one(sb3, 3, k, &checked);
                }
            }
        }
    }

    /* Exercise both the fixed hot buffer and the general long-state path with noncanonical
       input permutations. */
    for (int trial = 0; trial < 200000; trial++) {
        int sb[TEST_MAX_PARTS];
        int size = 1 + (int)(next_random(&random_state) % TEST_MAX_PARTS);
        int k = (int)(next_random(&random_state) % (MAX_K + 1));
        for (int i = 0; i < size; i++)
            sb[i] = ids[next_random(&random_state) % (unsigned)count];
        compare_one(sb, size, k, &checked);
    }

    printf("star majorization endpoint regression: %llu states agree\n", checked);
    return 0;
}
