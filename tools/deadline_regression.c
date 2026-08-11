/* Focused regression for canSolveB's deadline state machine.

   A bounded child must be able to return MAYBE even if it has produced no new negative verdict.
   If it produces exactly one, it receives one grace period, not a perpetually sliding deadline.

     clang -O2 -DMAX_K=2 -DMAX_N=8 tools/deadline_regression.c -o /tmp/deadline_regression
     /tmp/deadline_regression
*/
#include <assert.h>

#ifndef RADIOBASE_PATH
#define RADIOBASE_PATH "../radiobase.c"
#endif
#include RADIOBASE_PATH

int main(void) {
    int grace_given = FALSE;
    clock_t deadline = 200;

    /* No progress: an expired deadline stays expired. */
    assert(deadline_expired(&deadline, 100, 201, 0, 1, &grace_given));
    assert(deadline == 200);
    assert(!grace_given);

    /* First progress: five times the elapsed work is granted once. */
    deadline = 200;
    assert(!deadline_expired(&deadline, 100, 150, 1, 1, &grace_given));
    assert(deadline == 400);
    assert(grace_given);

    /* The unchanged count cannot slide that deadline forward again. */
    assert(deadline_expired(&deadline, 100, 401, 1, 1, &grace_given));
    assert(deadline == 400);

    puts("deadline regression passed");
    return 0;
}
