/* Exhaustive tiny-universe check for negative dominance-cache insertion.
 *
 * For every three-part seed inside the K=3 information and star-majorization bounds, build its
 * upward cache closure.  Compare every in-bound query with an independent perfect-matching test
 * for coordinatewise part dominance.  Repeated parts are common in this corpus, so this locks the
 * equal-choice quotient used by cacheCantSolve as well as the majorization boundary prune.
 *
 * Build with MAX_K=3, MAX_N=24 and MAX_PART_N=7.
 */

#include "../radiobase.c"

static int state_pairs(const int *state, int size) {
    int result = 0;
    for (int i = 0; i < size; i++) result += sb_pairs[state[i]];
    return result;
}

static int state_vertices(const int *state, int size) {
    int result = 0;
    for (int i = 0; i < size; i++)
        result += sbb_to_n1[state[i]] + sbb_to_n2[state[i]];
    return result;
}

static int match_dominance(const int *seed, const int *query, int size, int at,
                           unsigned used) {
    if (at == size) return TRUE;
    for (int i = 0; i < size; i++) {
        if ((used & (1u << i)) != 0) continue;
        if (sbb_to_n1[query[i]] < sbb_to_n1[seed[at]]
            || sbb_to_n2[query[i]] < sbb_to_n2[seed[at]]) continue;
        if (match_dominance(seed, query, size, at + 1, used | (1u << i))) return TRUE;
    }
    return FALSE;
}

static int in_region(int *state, int size) {
    return state_pairs(state, size) <= power3[3]
        && state_vertices(state, size) <= MAX_N
        && star_expansion_majorization_can_solve(state, size, 3);
}

int main(void) {
    enum { SIZE = 3 };
    unsigned long long seeds = 0, queries = 0;
    init();
    /* Sbb index 1 is (1:1), removed by Unit Group Triviality before trie insertion. Slots 0 and 1
       of every branch are front metadata rather than child edges, so enumerate the normalized
       non-unit cache domain only. */
    for (int a = MAX_SBB; a >= 2; a--) {
        for (int b = a; b >= 2; b--) {
            for (int c = b; c >= 2; c--) {
                int seed[SIZE] = {a, b, c};
                if (!in_region(seed, SIZE)) continue;
                cache(seed, SIZE, FALSE, 3, state_pairs(seed, SIZE));
                seeds++;
                for (int x = MAX_SBB; x >= 2; x--) {
                    for (int y = x; y >= 2; y--) {
                        for (int z = y; z >= 2; z--) {
                            int query[SIZE] = {x, y, z};
                            if (!in_region(query, SIZE)) continue;
                            int expected = match_dominance(seed, query, SIZE, 0, 0);
                            int actual = checkCacheTrie(query, SIZE, 3) == FALSE;
                            queries++;
                            if (actual != expected) {
                                fprintf(stderr, "upward closure mismatch seed=");
                                printSb(seed, SIZE);
                                fprintf(stderr, " query=");
                                printSb(query, SIZE);
                                fprintf(stderr, " expected=%d actual=%d\n", expected, actual);
                                return 1;
                            }
                        }
                    }
                }
                clear_node(&sb_cache_root[3]);
            }
        }
    }
    printf("CACHE_UPWARD_CLOSURE_REGRESSION seeds=%llu queries=%llu result=PASS\n",
           seeds, queries);
    return 0;
}
