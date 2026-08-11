/* Focused regression for canSolveB's bounded-probe state machine.

   A finite child may return MAYBE without a cache mutation, but it must never receive fresh time
   beyond an exhausted parent.  Every speculative child remains finite; unresolved exhaustive
   passes make monotone progress by doubling one local probe quantum instead of saving a split.

     tools/build_radio.py -O2 -DMAX_K=2 -DMAX_N=8 tools/deadline_regression.c -o /tmp/deadline_regression
     /tmp/deadline_regression
*/
#include <assert.h>

#ifndef RADIOBASE_PATH
#define RADIOBASE_PATH "../radiobase.c"
#endif
#include RADIOBASE_PATH

int main(void) {
    /* An exhausted parent stays exhausted; no per-child three-second refill. */
    assert(search_deadline(99, 100, 4) == 99);
    assert(search_deadline(120, 100, 4) == 120);

    /* A long child receives one tenth only when the parent has enough time to divide. */
    assert(search_deadline(100 + CLOCKS_PER_SEC * 100, 100, 4)
           == 100 + CLOCKS_PER_SEC * 10);
    /* The reliable one/two-segment construction keeps the shared parent cap. */
    assert(search_deadline(100 + CLOCKS_PER_SEC * 100, 100, 2)
           == 100 + CLOCKS_PER_SEC * 100);
    assert(deadline_expired(200, 201));
    assert(!deadline_expired(200, 200));

    /* A long speculative child gets its local quantum, without weakening a nearer finite cap. */
    assert(probe_child_deadline(100 + CLOCKS_PER_SEC * 5, 100, PROBE_SECONDS, 4)
           == 100 + CLOCKS_PER_SEC * PROBE_SECONDS);
    assert(probe_child_deadline(120, 100, PROBE_SECONDS, 4) == 120);
    /* Short states preserve the constructive spine's full shared cap. */
    assert(probe_child_deadline(100 + CLOCKS_PER_SEC * 5, 100, PROBE_SECONDS, 2)
           == 100 + CLOCKS_PER_SEC * 5);

    /* Even a cache miss with an already-expired parent returns MAYBE immediately. */
    init();
    int state = getSbb(3, 2);
    clock_t expired = clock() ? clock() - 1 : 0;
    assert(canSolveB(&state, 1, 2, expired) == MAYBE);

    puts("deadline regression passed");
    return 0;
}
