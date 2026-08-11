// The Sa(193) cold re-derivation: one process, one closed log.
//
// Why a cold single-session run rather than checking the 2023 corpus (decided 2026-08-05): every
// gap found in that corpus traces to warm-cache resumption. A run that is warm-started from caches
// whose own logs were discarded cites facts whose proofs are gone - measured, ~5% of its k=5 facts
// and all sixteen of its k=9 facts. A cold single session cannot have that defect, so its log is
// closed under SPLITS and `radio_verify` can check it end to end. See docs/certificate.md.
//
//   tools/build_radio.py -O3 -DMAX_K=10 -DMAX_N=193 radio_sa193.c -o radio_sa193
//   ./radio_sa193 [cache] [--no-control]
//
// Order matters. The positive control runs FIRST: Sa(192) in 10 is solvable and has a verified
// witness tree, so if it does not come back TRUE the engine is broken and the negative below is
// worthless. That is not a formality - on 2026-08-04 an engine change made a run sink 43 minutes
// into one node, and the 2023 corpus contains 37 provably false negatives with no syntactic marker.
//
// Restart: pass the parsed form of this run's OWN log (parse_out.sh) as `cache`. Warm-starting a
// negative from a run's own output is sound; from cache-2025:parsed_260.txt it is forbidden, since
// that file holds the sixteen suspect verdicts and cannot be filtered by era. The header written by
// tools/... is what keeps the two impossible to confuse - parse_file skips '#' lines.

#include "radiobase.c"

static double secs(clock_t t0) { return (double)(clock() - t0) / CLOCKS_PER_SEC; }

int main(int argc, char **argv) {
    init();

    char *cache = NULL;
    int control = 1, i;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--no-control")) control = 0;
        else cache = argv[i];
    }
    if (cache) parse_file(cache);

    printf("=== Sa(193) cold re-derivation, MAX_K=%d MAX_N=%d, control=%s, cache=%s\n",
           MAX_K, MAX_N, control ? "yes" : "no", cache ? cache : "(none, cold)");
    fflush(stdout);

    if (control) {
        clock_t t0 = clock();
        int r = canSolveA(192, 10);
        printf("result CONTROL Sa(192) in 10 = %s  (%.1f s)\n",
               r == TRUE ? "SOLVABLE" : r == FALSE ? "UNSOLVABLE" : "MAYBE", secs(t0));
        fflush(stdout);
        if (r != TRUE) {
            printf("ABORT: the control did not reproduce. The engine is not trustworthy here;\n"
                   "       any negative it produces for 193 would be meaningless.\n");
            return 3;
        }
    }

    clock_t t0 = clock();
    int r = canSolveA(193, 10);
    printf("result Sa(193) in 10 = %s  (%.1f s)\n",
           r == TRUE ? "SOLVABLE" : r == FALSE ? "UNSOLVABLE" : "MAYBE", secs(t0));
    printf("=== done. MAYBE is not a refutation - it means a deadline fired, not that 193 is hard.\n");
    return r == TRUE ? 0 : r == FALSE ? 1 : 2;
}
