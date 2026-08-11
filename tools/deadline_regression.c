/* Focused regression for canSolveB's deadline state machine.

   A bounded child must produce new negative verdicts before returning MAYBE.  At exactly the
   minimum progress count its grace window keeps moving so the depth-first dive can produce another
   reusable fact; only progress beyond that count makes expiry enforceable.  Pass 2 must hand its
   unresolved children NO_DEADLINE instead of repeatedly timing out at the same frontier.

     clang -O2 -DMAX_K=2 -DMAX_N=8 tools/deadline_regression.c -o /tmp/deadline_regression
     /tmp/deadline_regression
*/
#include <assert.h>

#ifndef RADIOBASE_PATH
#define RADIOBASE_PATH "../radiobase.c"
#endif
#include RADIOBASE_PATH

int main(void) {
    clock_t deadline = 200;

    /* No progress: expiry is deliberately suppressed until the subtree contributes a fact. */
    assert(!deadline_expired(&deadline, 100, 201, 0, 1));
    assert(deadline == 200);

    /* At the minimum count, five times the elapsed work is granted. */
    deadline = 200;
    assert(!deadline_expired(&deadline, 100, 201, 1, 1));
    assert(deadline == 706);

    /* The unchanged minimum count deliberately keeps the grace window moving. */
    assert(!deadline_expired(&deadline, 100, 707, 1, 1));
    assert(deadline == 3742);

    /* Once another fact arrives, an already-expired budget may bail immediately. */
    assert(deadline_expired(&deadline, 100, 3743, 2, 1));
    assert(deadline == 3742);

    /* Exhaustive pass 2 delegates unresolved children without a deadline. */
    assert(child_deadline_for_pass(1, 123) == 123);
    assert(child_deadline_for_pass(2, 123) == NO_DEADLINE);

    puts("deadline regression passed");
    return 0;
}
