/* Deterministic result-cache regression for a parsed checkpoint.
 *
 * It verifies every exact Sb fact, then writes one verdict byte for targeted dominance/prefix
 * mutations and 500,000 valid random states.  Compile the same source against old and new engines
 * with RADIOBASE_PATH and compare the output files byte-for-byte; any intentional strengthening is
 * then visible as a specific MAYBE -> TRUE/FALSE direction rather than hidden behind a hash.
 *
 *   tools/build_radio.py -O3 -DMAX_K=10 -DMAX_N=193 tools/cache_query_regression.c -o /tmp/cache-query
 *   /tmp/cache-query /tmp/verdicts.bin parsed.cache 5
 */
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef RADIOBASE_PATH
#define RADIOBASE_PATH "../radiobase.c"
#endif
#include RADIOBASE_PATH

static FILE *verdicts;
static unsigned long long query_count;
static unsigned long long true_count;
static unsigned long long false_count;
static unsigned long long maybe_count;
static unsigned long long trace_index = ULLONG_MAX;

static int valid_state(const int *sb, int size, int k) {
    if (size < 1 || k < 0 || k > MAX_K) return 0;
    long long pairs = 0;
    int coins = 0;
    for (int i = 0; i < size; i++) {
        if (sb[i] < 2 || sb[i] > MAX_SBB || (i && sb[i - 1] < sb[i])) return 0;
        pairs += sb_pairs[sb[i]];
        coins += sbb_to_n1[sb[i]] + sbb_to_n2[sb[i]];
    }
    return pairs <= power3[k] && coins <= MAX_N;
}

static void emit_state(int *sb, int size, int k, int expected) {
    sort1(sb, size);
    if (!valid_state(sb, size, k)) return;
    int verdict = checkCache(sb, size, k);
    if (query_count == trace_index) {
        fprintf(stderr, "TRACE %llu k=%d verdict=%d expected=%d ", query_count, k, verdict,
                expected);
        for (int i = 0; i < size; i++)
            fprintf(stderr, "%s%d:%d", i ? "," : "Sb(", sbb_to_n1[sb[i]], sbb_to_n2[sb[i]]);
        fprintf(stderr, ")\n");
    }
    if (expected >= 0 && verdict != expected) {
        fprintf(stderr, "exact checkpoint query mismatch at %llu: got %d expected %d k=%d ",
                query_count, verdict, expected, k);
        for (int i = 0; i < size; i++)
            fprintf(stderr, "%s%d:%d", i ? "," : "Sb(", sbb_to_n1[sb[i]], sbb_to_n2[sb[i]]);
        fprintf(stderr, ")\n");
        exit(20);
    }
    fputc(verdict, verdicts);
    query_count++;
    true_count += verdict == TRUE;
    false_count += verdict == FALSE;
    maybe_count += verdict == MAYBE;
}

static void emit_mutation(const int *original, int size, int k, int index, int d1, int d2) {
    if (index < 0 || index >= size || size > 510) return;
    int sb[512];
    memcpy(sb, original, (size_t)size * sizeof(int));
    int n1 = sbb_to_n1[sb[index]] + d1;
    int n2 = sbb_to_n2[sb[index]] + d2;
    if (n1 < n2) {
        int tmp = n1;
        n1 = n2;
        n2 = tmp;
    }
    if (n2 < 1 || n1 + n2 > MAX_N) return;
    int changed = getSbb(n1, n2);
    if (changed < 2) return;
    sb[index] = changed;
    emit_state(sb, size, k, -1);
}

static void emit_variants(const int *original, int size, int k) {
    int sb[512];
    memcpy(sb, original, (size_t)size * sizeof(int));
    emit_state(sb, size, k, -1);

    emit_mutation(original, size, k, 0, -1, 0);
    emit_mutation(original, size, k, 0, 0, -1);
    emit_mutation(original, size, k, 0, 1, 0);
    emit_mutation(original, size, k, 0, 0, 1);
    if (size > 1) {
        emit_mutation(original, size, k, size - 1, -1, 0);
        emit_mutation(original, size, k, size - 1, 0, -1);
        emit_mutation(original, size, k, size - 1, 1, 0);
        emit_mutation(original, size, k, size - 1, 0, 1);

        memcpy(sb, original, (size_t)(size - 1) * sizeof(int));
        emit_state(sb, size - 1, k, -1);
        memcpy(sb, original + 1, (size_t)(size - 1) * sizeof(int));
        emit_state(sb, size - 1, k, -1);
    }
    if (size < 511) {
        memcpy(sb, original, (size_t)size * sizeof(int));
        sb[size] = getSbb(2, 1);
        emit_state(sb, size + 1, k, -1);
    }
}

static void replay_queries(const char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) exit(14);
    char line[4096];
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (line[0] == '#') continue;
        char *token = strtok(line, " \n");
        if (token == NULL) continue;
        int expected = token[0] == '+' ? TRUE : FALSE;
        token = strtok(NULL, " \n");
        if (token == NULL || token[0] != 'b') continue;
        int sb[512];
        int size = 0;
        while ((token = strtok(NULL, " \n")) != NULL && token[0] != 't') {
            int n1 = atoi(token);
            token = strtok(NULL, " \n");
            if (token == NULL || size >= 512) exit(15);
            int n2 = atoi(token);
            sb[size++] = getSbb(n1, n2);
        }
        if (token == NULL) exit(15);
        (void)strtok(NULL, " \n"); /* pairs */
        (void)strtok(NULL, " \n"); /* coins */
        token = strtok(NULL, " \n");
        if (token == NULL) exit(15);
        int k = atoi(token);

        int exact[512];
        memcpy(exact, sb, (size_t)size * sizeof(int));
        emit_state(exact, size, k, expected);
        emit_variants(sb, size, k);
    }
    fclose(fp);
}

static uint64_t random64(void) {
    static uint64_t state = UINT64_C(0x9e3779b97f4a7c15);
    state ^= state >> 12;
    state ^= state << 25;
    state ^= state >> 27;
    return state * UINT64_C(2685821657736338717);
}

static void random_queries(int level, unsigned count) {
    for (unsigned emitted = 0; emitted < count;) {
        int size = 1 + (int)(random64() % 10);
        int sb[16];
        for (int i = 0; i < size; i++) {
            int n2 = 1 + (int)(random64() % 14);
            int n1 = n2 + (int)(random64() % 42);
            if (n1 + n2 > MAX_N) n1 = MAX_N - n2;
            sb[i] = getSbb(n1, n2);
        }
        sort1(sb, size);
        if (!valid_state(sb, size, level)) continue;
        emit_state(sb, size, level, -1);
        emitted++;
    }
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "usage: cache_query_regression OUTPUT CACHE LEVEL\n");
        return 2;
    }
    const char *trace = getenv("TRACE_INDEX");
    if (trace != NULL) trace_index = strtoull(trace, NULL, 10);
    int level = atoi(argv[3]);
    init();
    parse_file(argv[2]);
    verdicts = fopen(argv[1], "wb");
    if (verdicts == NULL) return 14;
    setvbuf(verdicts, NULL, _IOFBF, 1 << 20);
    clock_t started = clock();
    replay_queries(argv[2]);
    random_queries(level, 500000);
    fclose(verdicts);
    fprintf(stderr, "QUERIES total=%llu true=%llu false=%llu maybe=%llu cpu=%.3f\n",
            query_count, true_count, false_count, maybe_count,
            (double)(clock() - started) / CLOCKS_PER_SEC);
    return 0;
}
