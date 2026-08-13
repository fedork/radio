/* Focused regression for canSolveB's bounded-probe state machine.

   A finite child may return MAYBE without a cache mutation, but it must never receive fresh work
   beyond an exhausted parent.  Every speculative child remains finite; unresolved exhaustive
   passes make monotone progress by doubling one local probe quantum instead of saving a split.

     tools/build_radio.py -O2 -DMAX_K=3 -DMAX_N=20 tools/deadline_regression.c -o /tmp/deadline_regression
     /tmp/deadline_regression

   The same assertions exercise the historical CPU-clock fallback with:

     tools/build_radio.py -O2 -DMAX_K=3 -DMAX_N=20 -DRADIO_CPU_BUDGET \
         tools/deadline_regression.c -o /tmp/cpu_budget_regression
     /tmp/cpu_budget_regression
*/
#include <assert.h>

#ifndef RADIOBASE_PATH
#define RADIOBASE_PATH "../radiobase.c"
#endif
#include RADIOBASE_PATH

int main(void) {
    /* An exhausted parent stays exhausted; no per-child three-nominal-second refill. */
    assert(search_deadline(99, 100, 4) == 99);
    assert(search_deadline(120, 100, 4) == 120);

    /* A long child receives one tenth only when the parent has enough time to divide. */
    assert(search_deadline(100 + RADIO_BUDGET_UNITS_PER_SECOND * 100, 100, 4)
           == 100 + RADIO_BUDGET_UNITS_PER_SECOND * 10);
    /* The reliable one/two-segment construction keeps the shared parent cap. */
    assert(search_deadline(100 + RADIO_BUDGET_UNITS_PER_SECOND * 100, 100, 2)
           == 100 + RADIO_BUDGET_UNITS_PER_SECOND * 100);
    assert(deadline_expired(200, 201));
    assert(!deadline_expired(200, 200));

    /* A long speculative child gets its local quantum, without weakening a nearer finite cap. */
    assert(probe_child_deadline(100 + RADIO_BUDGET_UNITS_PER_SECOND * 5,
                                100, PROBE_SECONDS, 4)
           == 100 + RADIO_BUDGET_UNITS_PER_SECOND * PROBE_SECONDS);
    assert(probe_child_deadline(120, 100, PROBE_SECONDS, 4) == 120);
    /* Short states preserve the constructive spine's full shared cap. */
    assert(probe_child_deadline(100 + RADIO_BUDGET_UNITS_PER_SECOND * 5,
                                100, PROBE_SECONDS, 2)
           == 100 + RADIO_BUDGET_UNITS_PER_SECOND * 5);

    /* Even a cache miss with an already-expired parent returns MAYBE immediately. */
    init();
    int state = getSbb(3, 2);
    radio_budget_charge_split();
    uint64_t expired = radio_budget_now() - 1;
    assert(canSolveB(&state, 1, 2, expired) == MAYBE);

#ifdef RADIO_WORK_BUDGET
    /* The shared work clock advances inside recursive children, not merely in the printed root's
       local totalsplits.  A deliberately tiny finite allowance therefore cuts off this otherwise
       exact four-part query with MAYBE. */
    int multipart[] = { getSbb(5, 3), getSbb(2, 2), getSbb(2, 2), getSbb(2, 2) };
    uint64_t before = radio_budget_now();
    assert(canSolveB(multipart, 4, 3, radio_budget_add(before, 1)) == MAYBE);
    assert(radio_budget_now() > before + 1);
#endif

    puts("deadline regression passed");
    return 0;
}
