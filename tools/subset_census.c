/* Measure exact pair and triple subset filters on a list of parent states.
 *
 * Input states are one per line as n:m,n:m,... .  Candidate options first pass
 * the existing per-part test.  The program then reports how many cap-feasible
 * whole splits survive all exact two-part child subsets, and how many also
 * survive all exact three-part child subsets.
 *
 *   tools/build_radio.py -O3 tools/subset_census.c -o subset_census
 *   tools/run_with_provenance.py ./subset_census 5 pairs_k4.txt triples_k4.txt states.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "read_int_table.h"

#define MAX_PARTS 16
#define MAX_OPTIONS 4096
#define AXIS 64
#define MAX_SINGLE_PARTS 512

static const int power3[] = {1,3,9,27,81,243,729,2187,6561};
static const int p4[] = {0,16,15,12,10,9,7,6,5,5,4,3,3,2,2,2,1};

static unsigned char pair_ok[AXIS * AXIS][AXIS * AXIS / 8];
static int single_id[AXIS][AXIS], single_count;
static unsigned char *triple_ok;

static int N[MAX_PARTS], M[MAX_PARTS], parts, cap;
static int OX[MAX_PARTS][MAX_OPTIONS], OY[MAX_PARTS][MAX_OPTIONS];
static int option_count[MAX_PARTS], A[MAX_PARTS], B[MAX_PARTS];
static long long count_per_part, count_pair, count_triple;

static int solvable4(int n, int m) {
    if (n == 0 || m == 0) return 1;
    if (n >= (int)(sizeof(p4) / sizeof(p4[0]))
        || m >= (int)(sizeof(p4) / sizeof(p4[0]))) return 0;
    return n <= p4[m];
}

static int grid_id(int n, int m) {
    return n * AXIS + m;
}

static void set_pair(int n1, int m1, int n2, int m2) {
    int a = grid_id(n1, m1), b = grid_id(n2, m2);
    pair_ok[a][b >> 3] |= (unsigned char)(1u << (b & 7));
    pair_ok[b][a >> 3] |= (unsigned char)(1u << (a & 7));
}

static int get_pair(int n1, int m1, int n2, int m2) {
    if (n1 == 0 || m1 == 0 || n2 == 0 || m2 == 0) return 1;
    if (n1 >= AXIS || m1 >= AXIS || n2 >= AXIS || m2 >= AXIS) return 0;
    int a = grid_id(n1, m1), b = grid_id(n2, m2);
    return (pair_ok[a][b >> 3] >> (b & 7)) & 1;
}

static size_t triple_index(int a, int b, int c) {
    if (a > b) { int t = a; a = b; b = t; }
    if (b > c) { int t = b; b = c; c = t; }
    if (a > b) { int t = a; a = b; b = t; }
    return ((size_t)a * single_count + b) * single_count + c;
}

static void set_triple(int n1, int m1, int n2, int m2, int n3, int m3) {
    int a = single_id[n1][m1], b = single_id[n2][m2], c = single_id[n3][m3];
    if (a < 0 || b < 0 || c < 0) return;
    triple_ok[triple_index(a, b, c)] = 1;
}

static int get_triple(int n1, int m1, int n2, int m2, int n3, int m3) {
    if (n1 == 0 || m1 == 0 || n2 == 0 || m2 == 0 || n3 == 0 || m3 == 0)
        return 1;
    if (n1 >= AXIS || m1 >= AXIS || n2 >= AXIS || m2 >= AXIS
        || n3 >= AXIS || m3 >= AXIS) return 0;
    int a = single_id[n1][m1], b = single_id[n2][m2], c = single_id[n3][m3];
    if (a < 0 || b < 0 || c < 0) return 0;
    return triple_ok[triple_index(a, b, c)];
}

static void mixed_part(int index, int *n, int *m) {
    int parent = index / 2;
    if ((index & 1) == 0) {
        *n = A[parent];
        *m = M[parent] - B[parent];
    } else {
        *n = N[parent] - A[parent];
        *m = B[parent];
    }
}

static int pairs_survive(int i) {
    int a2 = A[i], b2 = B[i];
    int a0 = N[i] - A[i], b0 = M[i] - B[i];
    int u = A[i], v = M[i] - B[i];
    int x = N[i] - A[i], y = B[i];
    if (!get_pair(u, v, x, y)) return 0;
    for (int j = 0; j < i; j++) {
        if (!get_pair(a2, b2, A[j], B[j])) return 0;
        if (!get_pair(a0, b0, N[j] - A[j], M[j] - B[j])) return 0;
        if (!get_pair(u, v, A[j], M[j] - B[j])) return 0;
        if (!get_pair(u, v, N[j] - A[j], B[j])) return 0;
        if (!get_pair(x, y, A[j], M[j] - B[j])) return 0;
        if (!get_pair(x, y, N[j] - A[j], B[j])) return 0;
    }
    return 1;
}

static int triples_survive(int i) {
    int a2 = A[i], b2 = B[i];
    int a0 = N[i] - A[i], b0 = M[i] - B[i];
    for (int j = 0; j < i; j++) for (int q = j + 1; q < i; q++) {
        if (!get_triple(a2, b2, A[j], B[j], A[q], B[q])) return 0;
        if (!get_triple(a0, b0,
                        N[j] - A[j], M[j] - B[j],
                        N[q] - A[q], M[q] - B[q])) return 0;
    }

    int u = A[i], v = M[i] - B[i];
    int x = N[i] - A[i], y = B[i];
    int old = 2 * i;
    for (int j = 0; j < old; j++) {
        int an, am;
        mixed_part(j, &an, &am);
        if (!get_triple(u, v, x, y, an, am)) return 0;
    }
    for (int j = 0; j < old; j++) for (int q = j + 1; q < old; q++) {
        int an, am, bn, bm;
        mixed_part(j, &an, &am);
        mixed_part(q, &bn, &bm);
        if (!get_triple(u, v, an, am, bn, bm)) return 0;
        if (!get_triple(x, y, an, am, bn, bm)) return 0;
    }
    return 1;
}

static void enumerate(int i, int c0, int c1, int c2, int mode) {
    if (c0 > cap || c1 > cap || c2 > cap) return;
    if (i == parts) {
        if (mode == 0) count_per_part++;
        else if (mode == 1) count_pair++;
        else count_triple++;
        return;
    }
    for (int q = 0; q < option_count[i]; q++) {
        A[i] = OX[i][q];
        B[i] = OY[i][q];
        if (mode >= 1 && !pairs_survive(i)) continue;
        if (mode >= 2 && !triples_survive(i)) continue;
        enumerate(i + 1,
                  c0 + (N[i] - A[i]) * (M[i] - B[i]),
                  c1 + A[i] * (M[i] - B[i]) + (N[i] - A[i]) * B[i],
                  c2 + A[i] * B[i], mode);
    }
}

int main(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr, "usage: %s <parent-k> <pair-table> <triple-table> <states>\n", argv[0]);
        return 2;
    }
    int parent_level = atoi(argv[1]);
    if (parent_level != 5) {
        fprintf(stderr, "this prototype currently has the k=4 single-part frontier built in\n");
        return 2;
    }
    cap = power3[parent_level - 1];

    memset(single_id, 0xff, sizeof(single_id));
    for (int n = 1; n < (int)(sizeof(p4) / sizeof(p4[0])); n++)
        for (int m = 1; m < (int)(sizeof(p4) / sizeof(p4[0])); m++)
            if (solvable4(n, m)) single_id[n][m] = single_count++;
    if (single_count > MAX_SINGLE_PARTS) {
        fprintf(stderr, "too many single parts: %d\n", single_count);
        return 2;
    }
    triple_ok = calloc((size_t)single_count * single_count * single_count, 1);
    if (!triple_ok) {
        fprintf(stderr, "cannot allocate triple table\n");
        return 2;
    }

    FILE *f = fopen(argv[2], "r");
    if (!f) { fprintf(stderr, "cannot open %s\n", argv[2]); return 2; }
    int row[6];
    while (radio_read_int_row(f, row, 4)) set_pair(row[0],row[1],row[2],row[3]);
    fclose(f);
    f = fopen(argv[3], "r");
    if (!f) { fprintf(stderr, "cannot open %s\n", argv[3]); return 2; }
    long long triple_rows = 0;
    while (radio_read_int_row(f, row, 6)) {
        set_triple(row[0],row[1],row[2],row[3],row[4],row[5]);
        triple_rows++;
    }
    fclose(f);
    fprintf(stderr, "loaded %d single parts and %lld solvable triples\n",
            single_count, triple_rows);

    f = fopen(argv[4], "r");
    if (!f) { fprintf(stderr, "cannot open %s\n", argv[4]); return 2; }
    char line[1024];
    long long total_per_part = 0, total_pair = 0, total_triple = 0;
    int states = 0;
    while (fgets(line, sizeof(line), f)) {
        char state[768];
        if (sscanf(line, "%767s", state) != 1) continue;
        parts = 0;
        char *p = state;
        int n,m;
        while (sscanf(p, "%d:%d", &n, &m) == 2) {
            if (parts >= MAX_PARTS) { fprintf(stderr, "too many parts in %s\n", state); return 2; }
            N[parts] = n; M[parts] = m; parts++;
            p = strchr(p, ',');
            if (!p) break;
            p++;
        }
        if (!parts) continue;
        for (int i = 0; i < parts; i++) {
            option_count[i] = 0;
            for (int x = 0; x <= N[i]; x++) for (int y = 0; y <= M[i]; y++)
                if (solvable4(x,y)
                    && solvable4(N[i]-x,M[i]-y)
                    && solvable4(x,M[i]-y)
                    && solvable4(N[i]-x,y)) {
                    int q = option_count[i]++;
                    if (q >= MAX_OPTIONS) {
                        fprintf(stderr, "too many options for part %d in %s\n", i, state);
                        return 2;
                    }
                    OX[i][q] = x; OY[i][q] = y;
                }
        }
        count_per_part = count_pair = count_triple = 0;
        enumerate(0,0,0,0,0);
        enumerate(0,0,0,0,1);
        enumerate(0,0,0,0,2);
        total_per_part += count_per_part;
        total_pair += count_pair;
        total_triple += count_triple;
        states++;
        printf("C perpart=%lld pair=%lld triple=%lld state=%s\n",
               count_per_part, count_pair, count_triple, state);
    }
    fclose(f);
    fprintf(stderr,
            "TOTAL states=%d per-part=%lld pair=%lld triple=%lld "
            "pair-gain=%.2fx triple-gain=%.2fx cumulative=%.2fx\n",
            states, total_per_part, total_pair, total_triple,
            total_pair ? (double)total_per_part / total_pair : 0.0,
            total_triple ? (double)total_pair / total_triple : 0.0,
            total_triple ? (double)total_per_part / total_triple : 0.0);
    free(triple_ok);
    return 0;
}
