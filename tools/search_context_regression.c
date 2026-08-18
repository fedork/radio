/* Lock the first parallel-solver prerequisite: worker-local scheduling, exact-L1 and
   reachability state must be independently owned even while the result trie and split catalog
   remain intentionally process-global.

     tools/build_radio.py -O2 -DMAX_K=3 -DMAX_N=20 \
         tools/search_context_regression.c -o /tmp/search_context_regression
     /tmp/search_context_regression
*/
#include <assert.h>

#ifndef RADIOBASE_PATH
#define RADIOBASE_PATH "../radiobase.c"
#endif
#include RADIOBASE_PATH

int main(void) {
    radio_search_context first;
    radio_search_context second;
    radio_search_context_init(&first);
    radio_search_context_init(&second);
    init();

    assert(first.work_clock == RADIO_WORK_CLOCK_ORIGIN);
    assert(second.work_clock == RADIO_WORK_CLOCK_ORIGIN);
    uint64_t default_before = radio_budget_now();

#ifdef RADIO_WORK_BUDGET
    /* Recursive work charged to one context must not consume another worker's allowance or the
       legacy default context. */
    int multipart[] = {
        getSbb(5, 3), getSbb(2, 2), getSbb(2, 2), getSbb(2, 2),
    };
    uint64_t first_before = radio_budget_now_ctx(&first);
    uint64_t limit = radio_budget_add(first_before, 1);
    assert(canSolveB_ctx(&first, multipart, 4, 3, limit) == MAYBE);
    assert(radio_budget_now_ctx(&first) > first_before + 1);
    assert(radio_budget_now_ctx(&second) == RADIO_WORK_CLOCK_ORIGIN);
    assert(radio_budget_now() == default_before);
#else
    /* The historical CPU scheduler uses one process clock by definition.  Context ownership still
       applies to the exact-L1 and reachability scratch tested below. */
    (void)default_before;
#endif

    /* A theorem-level answer populates each context's exact L1 without entering recursive search.
       The backing arrays and reachability workspaces must be distinct. */
    int singleton = getSbb(3, 1);
    assert(canSolveB_ctx(&first, &singleton, 1, 2, CACHE_ONLY) == TRUE);
    assert(canSolveB_ctx(&second, &singleton, 1, 2, CACHE_ONLY) == TRUE);
    assert(first.cache_l1 != NULL);
    assert(second.cache_l1 != NULL);
    assert(first.cache_l1 != second.cache_l1);

    radio_reachability_state *first_rb = radio_search_context_reachability(&first);
    radio_reachability_state *second_rb = radio_search_context_reachability(&second);
    assert(first_rb != second_rb);
    first_rb->tested = 7;
    assert(second_rb->tested == 0);

    radio_search_context_destroy(&first);
    radio_search_context_destroy(&second);
    puts("search context regression passed");
    return 0;
}
