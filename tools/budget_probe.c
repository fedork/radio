/* Run one Sb query under a finite scheduling allowance.

   Compile once with the CPU fallback plus RADIO_MEASURE_WORK and once with the default work budget.
   The same millisecond argument becomes process-CPU time in the former and the calibrated number
   of deterministic split-prefix units in the latter.

     tools/build_radio.py -O3 -DMAX_K=<k> -DMAX_N=<total-coins> \
         -DRADIO_CPU_BUDGET -DRADIO_MEASURE_WORK \
         tools/budget_probe.c -o budget_cpu
     tools/build_radio.py -O3 -DMAX_K=<k> -DMAX_N=<total-coins> \
         tools/budget_probe.c -o budget_work
     ./budget_cpu 100 k n1 m1 [n2 m2 ...]
     ./budget_work 100 k n1 m1 [n2 m2 ...]
*/

#ifndef RADIOBASE_PATH
#define RADIOBASE_PATH "../radiobase.c"
#endif
#include RADIOBASE_PATH

int main(int argc, char **argv) {
    if (argc < 5 || !(argc & 1)) {
        fprintf(stderr, "usage: %s milliseconds k n1 m1 [n2 m2 ...]\n", argv[0]);
        return 3;
    }

    int milliseconds = atoi(argv[1]);
    int k = atoi(argv[2]);
    int size = (argc - 3) / 2;
    if (milliseconds < 0 || k < 1 || k > MAX_K) return 3;

    init();
    int sb[size];
    int total_coins = 0;
    for (int i = 0; i < size; i++) {
        int n = atoi(argv[3 + 2 * i]);
        int m = atoi(argv[4 + 2 * i]);
        if (n < 0 || m < 0 || n > MAX_N || m > MAX_N
            || min(n, m) > MAX_N / 2 || n + m > MAX_N - total_coins) {
            fprintf(stderr, "part %d:%d exceeds compiled total bound MAX_N=%d\n", n, m, MAX_N);
            return 3;
        }
        total_coins += n + m;
        sb[i] = getSbb(n, m);
    }

    uint64_t work_before = radio_work_units_used();
    clock_t cpu_before = clock();
    uint64_t limit = radio_budget_after_milliseconds((uint64_t)milliseconds);
    int result = canSolveB(sb, size, k, limit);
    double cpu_seconds = (double)(clock() - cpu_before) / CLOCKS_PER_SEC;
    uint64_t work = radio_work_units_used() - work_before;

    printf("BUDGET_PROBE result=%s requested_ms=%d cpu_seconds=%.6f work=%llu state=",
           result == TRUE ? "TRUE" : result == FALSE ? "FALSE" : "MAYBE",
           milliseconds, cpu_seconds, (unsigned long long)work);
    printSb(sb, size);
    printf(" k=%d\n", k);
    return result == TRUE ? 0 : result == FALSE ? 1 : 2;
}
