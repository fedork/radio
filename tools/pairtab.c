/* Build the complete current-solver table for pairs of individually solvable parts.

   Usage:
     tools/build_radio.py -O3 -DMAX_K=4 -DMAX_N=64 tools/pairtab.c -o /tmp/pairtab4
     /tmp/pairtab4 4 16 > /tmp/pairs_k4.txt

   Exact four-integer stdout lines are the solvable pairs `n1 m1 n2 m2`.  radiobase also prints
   newly proved intermediate facts to stdout; consumers deliberately ignore every other line.
   Missing pairs are solver-negative only, not independent certificates. */

#include "../radiobase.c"

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s k max_side\n", argv[0]);
        return 2;
    }
    int k = atoi(argv[1]);
    int max_side = atoi(argv[2]);
    if (k < 0 || k > MAX_K || max_side < 1) return 2;

    init();
    int capacity = max_side * max_side;
    int *pn = malloc((size_t)capacity * sizeof(*pn));
    int *pm = malloc((size_t)capacity * sizeof(*pm));
    if (!pn || !pm) {
        fprintf(stderr, "out of memory\n");
        return 2;
    }

    int count = 0;
    for (int m = 1; m <= max_side; m++) {
        for (int n = 1; n <= max_side; n++) {
            int part = getSbb(n, m);
            if (canSolveB(&part, 1, k, NO_DEADLINE) == TRUE) {
                pn[count] = n;
                pm[count] = m;
                count++;
            }
        }
    }
    fprintf(stderr, "single parts solvable at k=%d: %d\n", k, count);

    long long solved = 0;
    long long total = 0;
    for (int i = 0; i < count; i++) {
        for (int j = i; j < count; j++) {
            int state[2] = {getSbb(pn[i], pm[i]), getSbb(pn[j], pm[j])};
            if (state[0] < state[1]) {
                int swap = state[0];
                state[0] = state[1];
                state[1] = swap;
            }
            total++;
            if (canSolveB(state, 2, k, NO_DEADLINE) == TRUE) {
                solved++;
                printf("%d %d %d %d\n", pn[i], pm[i], pn[j], pm[j]);
            }
        }
    }
    fprintf(stderr, "pairs solvable %lld of %lld (%.1f%%)\n",
            solved, total, total ? 100.0 * solved / total : 0.0);
    free(pm);
    free(pn);
    return 0;
}
