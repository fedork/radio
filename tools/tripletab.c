/* Emit the exact solvable 3-part table at one level.
 *
 * Subgraph Monotonicity makes this a sound filter: every three-part subset of
 * every child of a winning split must itself be solvable.  The optional exact
 * pair table avoids asking the solver about triples already refuted by one of
 * their pairs.
 *
 *   tools/build_radio.py -O3 -DMAX_K=4 -DMAX_N=96 tools/tripletab.c -o tripletab
 *   ./tripletab 4 16 data/pairs_k4.txt
 *
 * Stdout starts with provenance comments, then contains six-integer table rows; solver chatter is
 * silenced. Consumers use read_int_table.h and ignore non-row lines.
 */
#include "../radiobase.c"
#include "read_int_table.h"
#include <unistd.h>

#define AXIS 64

static unsigned char pair_ok[AXIS * AXIS][AXIS * AXIS / 8];

static int part_id(int n, int m) {
    return n * AXIS + m;
}

static void set_pair(int n1, int m1, int n2, int m2) {
    int a = part_id(n1, m1), b = part_id(n2, m2);
    pair_ok[a][b >> 3] |= (unsigned char)(1u << (b & 7));
    pair_ok[b][a >> 3] |= (unsigned char)(1u << (a & 7));
}

static int get_pair(int n1, int m1, int n2, int m2) {
    int a = part_id(n1, m1), b = part_id(n2, m2);
    return (pair_ok[a][b >> 3] >> (b & 7)) & 1;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "usage: %s <k> <axis-limit> <pair-table>\n", argv[0]);
        return 2;
    }
    int level = atoi(argv[1]), limit = atoi(argv[2]);
    if (limit >= AXIS) {
        fprintf(stderr, "axis limit %d exceeds compile-time AXIS=%d\n", limit, AXIS);
        return 2;
    }
    /* Keep the table self-identifying even on a compiler without constructor support, but leave
       engine initialization behind the stdout redirect so the table itself stays clean. */
    radio_print_provenance();

    FILE *pf = fopen(argv[3], "r");
    if (!pf) {
        fprintf(stderr, "cannot open pair table %s\n", argv[3]);
        return 2;
    }
    int row[6];
    while (radio_read_int_row(pf, row, 4)) set_pair(row[0], row[1], row[2], row[3]);
    fclose(pf);

    int table_fd = dup(STDOUT_FILENO);
    FILE *table = table_fd >= 0 ? fdopen(table_fd, "w") : NULL;
    if (!table || !freopen("/dev/null", "w", stdout)) {
        fprintf(stderr, "cannot redirect solver output\n");
        return 2;
    }
    init();
    int pn[512], pm[512], np = 0;
    for (int n = 1; n <= limit; n++) for (int m = 1; m <= limit; m++) {
        int sb = getSbb(n, m);
        if (canSolveB(&sb, 1, level, NO_DEADLINE) == TRUE) {
            pn[np] = n;
            pm[np] = m;
            np++;
        }
    }
    fprintf(stderr, "single parts solvable at k=%d: %d\n", level, np);

    long long pair_feasible = 0, solvable = 0, total = 0;
    for (int i = 0; i < np; i++) for (int j = i; j < np; j++)
        for (int q = j; q < np; q++) {
            total++;
            if (!get_pair(pn[i], pm[i], pn[j], pm[j])
                || !get_pair(pn[i], pm[i], pn[q], pm[q])
                || !get_pair(pn[j], pm[j], pn[q], pm[q]))
                continue;
            pair_feasible++;
            int sb[3] = {
                getSbb(pn[i], pm[i]),
                getSbb(pn[j], pm[j]),
                getSbb(pn[q], pm[q])
            };
            sort1(sb, 3);
            if (canSolveB(sb, 3, level, NO_DEADLINE) == TRUE) {
                solvable++;
                fprintf(table, "%d %d %d %d %d %d\n",
                        pn[i], pm[i], pn[j], pm[j], pn[q], pm[q]);
            }
        }
    fprintf(stderr,
            "triples solvable %lld of %lld pair-feasible, %lld total "
            "(pair gate %.1f%%, solvable %.1f%% of gated)\n",
            solvable, pair_feasible, total,
            total ? 100.0 * pair_feasible / total : 0.0,
            pair_feasible ? 100.0 * solvable / pair_feasible : 0.0);
    fclose(table);
    return 0;
}
