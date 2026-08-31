/* Production-engine regression for the K=6 singleton-majorization refutation.
 *
 * This deliberately uses radiobase.c rather than the independent Hall/direct utilities.  It
 * locks three safety boundaries:
 *   1. the 30-row mass-683 core is majorized but is not an embedded G_6 subprofile;
 *   2. an unmarked historical positive is rejected before entering the dominance trie;
 *   3. the ordinary recursive solver returns FALSE, using the proved K<=5 converse only on its
 *      children and never as a K=6 terminal.
 *
 * Build through tools/build_radio.py with MAX_K=6, MAX_N>=793 and MAX_PART_N>=65.
 */

#include "../radiobase.c"

static int append(int *state, int size, int value, int copies) {
    for (int i = 0; i < copies; i++) state[size++] = getSbb(value, 1);
    return size;
}

static int append_head(int *state, int size) {
    size = append(state, size, 64, 1);
    size = append(state, size, 63, 1);
    size = append(state, size, 57, 2);
    size = append(state, size, 42, 4);
    return append(state, size, 22, 7);
}

int main(void) {
    int core[30];
    int size = 0;
    init();

    size = append_head(core, size);
    size = append(core, size, 8, 15);
    if (size != 30) return 2;
    sort1(core, size);

    if (!singleton_majorization_holds(core, size, 6)) {
        fprintf(stderr, "K=6 control is not majorized\n");
        return 1;
    }
    if (singleton_embedded_can_solve(core, size, 6)) {
        fprintf(stderr, "K=6 control unexpectedly embeds in distinct G_6 slots\n");
        return 1;
    }

    /* Simulate a pre-refutation cache line.  It must not survive ingestion. */
    cache_replay_depth++;
    cache_replay_accept_positive = FALSE;
    cache(core, size, TRUE, 6, 683);
    cache_replay_depth--;
    if (checkCacheTrie(core, size, 6) != MAYBE || ignored_positive_cache_replays != 1) {
        fprintf(stderr, "untrusted K=6 positive reached the dominance trie\n");
        return 1;
    }

    /* A legacy positive can be tainted above the singleton leaf as well.  The larger state is
       unsolvable by Subgraph Monotonicity because deleting its (2:2) component leaves the core.
       It too must remain absent from the trie; this is the case the old singleton-only cache
       screening missed. */
    int parent[31];
    memcpy(parent, core, (size_t)size * sizeof(*parent));
    parent[size] = getSbb(2, 2);
    sort1(parent, size + 1);
    cache_replay_depth++;
    cache_replay_accept_positive = FALSE;
    cache(parent, size + 1, TRUE, 6, 687);
    cache_replay_depth--;
    if (checkCacheTrie(parent, size + 1, 6) != MAYBE
        || ignored_positive_cache_replays != 2) {
        fprintf(stderr, "ancestor cache-taint boundary failed\n");
        return 1;
    }

    /* Positive controls bracket the failed final transfer under the ordinary engine. */
    int canonical[32];
    int canonical_size = append_head(canonical, 0);
    canonical_size = append(canonical, canonical_size, 22, 1);
    canonical_size = append(canonical, canonical_size, 7, 16);
    sort1(canonical, canonical_size);
    if (canonical_size != 32
        || canSolveB(canonical, canonical_size, 6, NO_DEADLINE) != TRUE) {
        fprintf(stderr, "canonical G_6 control failed\n");
        return 1;
    }

    int j13[32];
    int j13_size = append_head(j13, 0);
    j13_size = append(j13, j13_size, 9, 1);
    j13_size = append(j13, j13_size, 8, 13);
    j13_size = append(j13, j13_size, 7, 3);
    sort1(j13, j13_size);
    if (j13_size != 32
        || canSolveB(j13, j13_size, 6, NO_DEADLINE) != TRUE
        || checkCacheTrie(j13, j13_size, 6) != TRUE) {
        fprintf(stderr, "j=13 positive control failed\n");
        return 1;
    }

    long long truncated_before = truncated_cache_insertions;
    uint64_t before = radio_budget_now_ctx(&radio_default_search_context);
    int verdict = canSolveB(core, size, 6, NO_DEADLINE);
    uint64_t work = radio_budget_now_ctx(&radio_default_search_context) - before;
    if (verdict != FALSE || work == 0) {
        fprintf(stderr, "K=6 counterexample verdict=%d, expected FALSE\n", verdict);
        return 1;
    }

    /* The general dominance trie explores component permutations.  Its deterministic insertion
       allowance must retain the exact negative while stopping the otherwise factorial closure.
       The ordinary canSolveB call above performs this insertion. */
    if (checkCacheTrie(core, size, 6) != FALSE
        || RADIO_CACHE_INSERT_NODE_LIMIT == 0
        || !cache_insert_truncated
        || cache_insert_nodes != RADIO_CACHE_INSERT_NODE_LIMIT
        || truncated_cache_insertions != truncated_before + 1) {
        fprintf(stderr, "bounded dominance insertion failed nodes=%llu truncated=%d\n",
                (unsigned long long)cache_insert_nodes, cache_insert_truncated);
        return 1;
    }

    /* Both previously recorded forms are upward consequences of the smaller core. */
    int core32[32];
    memcpy(core32, core, (size_t)size * sizeof(*core32));
    int core32_size = append(core32, size, 7, 2);
    sort1(core32, core32_size);
    if (core32_size != 32 || canSolveB(core32, core32_size, 6, NO_DEADLINE) != FALSE) {
        fprintf(stderr, "mass-697 core consequence failed\n");
        return 1;
    }

    int padded[64];
    memcpy(padded, core32, (size_t)core32_size * sizeof(*padded));
    int padded_size = append(padded, core32_size, 1, 32);
    sort1(padded, padded_size);
    if (padded_size != 64 || canSolveB(padded, padded_size, 6, NO_DEADLINE) != FALSE) {
        fprintf(stderr, "full-mass padded counterexample control failed\n");
        return 1;
    }

    printf("SINGLETON_MAIN_SOLVER_REGRESSION verdict=FALSE work=%llu cache_nodes=%llu "
           "ignored_untrusted_positive=%lld\n",
           (unsigned long long)work, (unsigned long long)cache_insert_nodes,
           ignored_positive_cache_replays);
    return 0;
}
