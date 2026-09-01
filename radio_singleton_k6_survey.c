/* Integrated ranked K=6 singleton-shell survey.

   This is intentionally a thin C driver around the production solver: generate one exact ranked
   transfer-shell window in memory, turn each singleton row n into getSbb(n,1), and call canSolveB
   directly.  There is no line protocol, pipe, parser, Hall search, or second process.

   Build for the live distance-14 census with:

     tools/build_radio.py -O3 -DMAX_K=6 -DMAX_N=793 -DMAX_PART_N=65 \
       -DRADIO_CACHE_DISABLED_LEVEL=6 radio_singleton_k6_survey.c -o survey

   Usage: survey K DISTANCE SKIP LIMIT PROGRESS_FILE
*/

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <unistd.h>

#include "radiobase.c"

#define SURVEY_MEMO_CAPACITY (1u << 18)

typedef struct {
    uint64_t key_plus_one;
    uint64_t value;
} survey_memo_entry;

typedef struct {
    int k, distance, rows, mass, target_l1;
    int parent[1 << MAX_K];
    int prefix[(1 << MAX_K) + 1];
    int suffix[(1 << MAX_K) + 1];
    int state[1 << MAX_K];
    uint64_t skip, remaining_skip, limit, counted;
    uint64_t tested, solvable, unsolvable, maybe;
    double total_ms;
    const char *progress_path;
    FILE *report;
    survey_memo_entry *memo;
    uint64_t memo_entries;
    struct timespec started;
} survey_state;

static int survey_parse_u64(const char *text, uint64_t *value) {
    char *end = NULL;
    errno = 0;
    unsigned long long parsed = strtoull(text, &end, 10);
    if (errno || text[0] == '\0' || end == NULL || *end != '\0') return FALSE;
    *value = (uint64_t)parsed;
    return TRUE;
}

static double survey_elapsed(const survey_state *survey) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (double)(now.tv_sec - survey->started.tv_sec)
        + (double)(now.tv_nsec - survey->started.tv_nsec) / 1e9;
}

static void survey_build_parent(survey_state *survey) {
    int current[1 << MAX_K] = {1};
    int current_size = 1;
    for (int level = 0; level < survey->k; level++) {
        int next[1 << MAX_K] = {0};
        for (int i = 0; i < current_size; i++) {
            next[i] += current[i];
            next[2 * i] += current[i];
            next[2 * i + 1] += current[i];
        }
        current_size *= 2;
        sort1(next, current_size);
        memcpy(current, next, (size_t)current_size * sizeof(int));
    }
    survey->rows = current_size;
    survey->prefix[0] = 0;
    for (int i = 0; i < current_size; i++) {
        survey->parent[i] = current[i];
        survey->mass += current[i];
        survey->prefix[i + 1] = survey->mass;
    }
    survey->suffix[current_size] = 0;
    for (int i = current_size - 1; i >= 0; i--)
        survey->suffix[i] = survey->suffix[i + 1] + current[i];
}

static uint64_t survey_completion_key(
    int position, int remaining, int maximum, int used_l1) {
    return (uint64_t)position
        | ((uint64_t)remaining << 7)
        | ((uint64_t)maximum << 17)
        | ((uint64_t)used_l1 << 24);
}

static int survey_memo_get(survey_state *survey, uint64_t key, uint64_t *value) {
    uint32_t slot = (uint32_t)((key * UINT64_C(11400714819323198485))
                               >> (64 - 18));
    for (uint32_t probe = 0; probe < SURVEY_MEMO_CAPACITY; probe++) {
        survey_memo_entry *entry = &survey->memo[(slot + probe) & (SURVEY_MEMO_CAPACITY - 1)];
        if (entry->key_plus_one == 0) return FALSE;
        if (entry->key_plus_one == key + 1) {
            *value = entry->value;
            return TRUE;
        }
    }
    fprintf(stderr, "INTEGRATED_ERROR completion memo lookup exhausted\n");
    exit(2);
}

static void survey_memo_put(survey_state *survey, uint64_t key, uint64_t value) {
    uint32_t slot = (uint32_t)((key * UINT64_C(11400714819323198485))
                               >> (64 - 18));
    for (uint32_t probe = 0; probe < SURVEY_MEMO_CAPACITY; probe++) {
        survey_memo_entry *entry = &survey->memo[(slot + probe) & (SURVEY_MEMO_CAPACITY - 1)];
        if (entry->key_plus_one == 0 || entry->key_plus_one == key + 1) {
            if (entry->key_plus_one == 0) survey->memo_entries++;
            entry->key_plus_one = key + 1;
            entry->value = value;
            return;
        }
    }
    fprintf(stderr, "INTEGRATED_ERROR completion memo insertion exhausted\n");
    exit(2);
}

static uint64_t survey_count(
    survey_state *survey, int position, int remaining, int maximum, int used_l1) {
    if (position == survey->rows)
        return remaining == 0 && used_l1 == survey->target_l1 ? 1 : 0;
    int slots = survey->rows - position;
    if (remaining < slots || remaining > slots * maximum
        || used_l1 > survey->target_l1) return 0;
    int remaining_difference = remaining - survey->suffix[position];
    int available_l1 = survey->target_l1 - used_l1;
    if (remaining_difference < 0 || remaining_difference > available_l1
        || ((available_l1 - remaining_difference) & 1)) return 0;

    uint64_t key = survey_completion_key(position, remaining, maximum, used_l1);
    uint64_t memoized;
    if (survey_memo_get(survey, key, &memoized)) return memoized;

    int canonical = survey->parent[position];
    int smallest = max(1, canonical - available_l1);
    int greatest = min(maximum, remaining - (slots - 1));
    greatest = min(greatest, canonical + available_l1);
    int used_mass = survey->mass - remaining;
    uint64_t result = 0;
    for (int value = greatest; value >= smallest; value--) {
        if (used_mass + value > survey->prefix[position + 1]) continue;
        int next_l1 = used_l1 + abs(value - canonical);
        if (next_l1 > survey->target_l1) continue;
        uint64_t add = survey_count(
            survey, position + 1, remaining - value, value, next_l1);
        if (UINT64_MAX - result < add) {
            fprintf(stderr, "INTEGRATED_ERROR completion count overflow\n");
            exit(2);
        }
        result += add;
    }
    survey_memo_put(survey, key, result);
    return result;
}

static void survey_write_progress(const survey_state *survey) {
    if (!survey->progress_path[0] || !strcmp(survey->progress_path, "-")) return;
    char temporary[4096];
    if (snprintf(temporary, sizeof temporary, "%s.tmp", survey->progress_path)
        >= (int)sizeof temporary) {
        fprintf(stderr, "INTEGRATED_PROGRESS_WRITE_FAILED path-too-long\n");
        return;
    }
    FILE *out = fopen(temporary, "w");
    if (!out) {
        fprintf(stderr, "INTEGRATED_PROGRESS_WRITE_FAILED path=%s\n", temporary);
        return;
    }
    fprintf(out, "queries=%" PRIu64 "\n", survey->tested);
    fprintf(out, "solvable=%" PRIu64 "\n", survey->solvable);
    fprintf(out, "unsolvable=%" PRIu64 "\n", survey->unsolvable);
    fprintf(out, "maybe=%" PRIu64 "\n", survey->maybe);
    fprintf(out, "elapsed_seconds=%.6f\n", survey_elapsed(survey));
    if (fclose(out) || rename(temporary, survey->progress_path))
        fprintf(stderr, "INTEGRATED_PROGRESS_WRITE_FAILED path=%s\n",
                survey->progress_path);
}

static void survey_print_exception(
    survey_state *survey, int result, double ms) {
    fprintf(survey->report, "VERDICT %s k=%d ms=%.1f Sb(",
            result == FALSE ? "UNSOLVABLE" : "MAYBE", survey->k, ms);
    for (int i = 0; i < survey->rows; i++)
        fprintf(survey->report, "%s%d:1", i ? "," : "", survey->state[i]);
    fprintf(survey->report, ")\n");
    fflush(survey->report);
}

static void survey_inspect(survey_state *survey) {
#ifdef RADIO_SURVEY_TRACE_STATES
    fprintf(survey->report, "SURVEY_STATE");
    for (int i = 0; i < survey->rows; i++)
        fprintf(survey->report, " %d", survey->state[i]);
    fprintf(survey->report, "\n");
#endif
    int sb[1 << MAX_K];
    for (int i = 0; i < survey->rows; i++) sb[i] = getSbb(survey->state[i], 1);
    clock_t before = clock();
    int result = canSolveB(sb, survey->rows, survey->k, NO_DEADLINE);
    double ms = (double)(clock() - before) * 1000.0 / CLOCKS_PER_SEC;
    survey->total_ms += ms;
    survey->tested++;
    if (result == TRUE) {
        survey->solvable++;
    } else if (result == FALSE) {
        survey->unsolvable++;
        survey_print_exception(survey, result, ms);
    } else {
        survey->maybe++;
        survey_print_exception(survey, result, ms);
    }
    if (survey->tested % 100000 == 0 || survey->tested == survey->limit)
        survey_write_progress(survey);
}

static void survey_enumerate(
    survey_state *survey, int position, int remaining, int maximum, int used_l1) {
    if (survey->tested >= survey->limit) return;
    uint64_t subtree = survey_count(survey, position, remaining, maximum, used_l1);
    if (!subtree) return;
    if (survey->remaining_skip && subtree <= survey->remaining_skip) {
        survey->remaining_skip -= subtree;
        return;
    }
    if (position == survey->rows) {
        if (remaining == 0 && used_l1 == survey->target_l1) survey_inspect(survey);
        return;
    }

    int slots = survey->rows - position;
    int remaining_difference = remaining - survey->suffix[position];
    int available_l1 = survey->target_l1 - used_l1;
    if (remaining < slots || remaining > slots * maximum || available_l1 < 0
        || remaining_difference < 0 || remaining_difference > available_l1
        || ((available_l1 - remaining_difference) & 1)) return;
    int canonical = survey->parent[position];
    int smallest = max(1, canonical - available_l1);
    int greatest = min(maximum, remaining - (slots - 1));
    greatest = min(greatest, canonical + available_l1);
    int used_mass = survey->mass - remaining;
    for (int value = greatest; value >= smallest; value--) {
        if (used_mass + value > survey->prefix[position + 1]) continue;
        int next_l1 = used_l1 + abs(value - canonical);
        if (next_l1 > survey->target_l1) continue;
        survey->state[position] = value;
        survey_enumerate(
            survey, position + 1, remaining - value, value, next_l1);
        if (survey->tested >= survey->limit) return;
    }
}

int main(int argc, char **argv) {
    if (argc != 6) {
        fprintf(stderr, "usage: %s K DISTANCE SKIP LIMIT PROGRESS_FILE\n", argv[0]);
        return 2;
    }
    survey_state survey = {0};
    survey.k = atoi(argv[1]);
    survey.distance = atoi(argv[2]);
    survey.target_l1 = 2 * survey.distance;
    survey.progress_path = argv[5];
    if (survey.k < 2 || survey.k > MAX_K || survey.distance < 0
        || !survey_parse_u64(argv[3], &survey.skip)
        || !survey_parse_u64(argv[4], &survey.limit) || !survey.limit) {
        fprintf(stderr, "invalid integrated survey arguments\n");
        return 2;
    }
    survey.remaining_skip = survey.skip;
    survey.memo = calloc(SURVEY_MEMO_CAPACITY, sizeof(*survey.memo));
    if (!survey.memo) {
        fprintf(stderr, "INTEGRATED_ERROR cannot allocate completion memo\n");
        return 2;
    }
    survey_build_parent(&survey);

    init();
    fflush(stdout);
    int saved_stdout = dup(STDOUT_FILENO);
    if (saved_stdout < 0 || dup2(STDERR_FILENO, STDOUT_FILENO) < 0
        || !(survey.report = fdopen(saved_stdout, "w"))) {
        perror("integrated survey stream setup");
        return 20;
    }
    clock_gettime(CLOCK_MONOTONIC, &survey.started);
    survey.counted = survey_count(
        &survey, 0, survey.mass, survey.parent[0], 0);
    if (survey.skip >= survey.counted || survey.limit > survey.counted - survey.skip) {
        fprintf(survey.report,
                "INTEGRATED_ERROR invalid window skip=%" PRIu64
                " limit=%" PRIu64 " total=%" PRIu64 "\n",
                survey.skip, survey.limit, survey.counted);
        return 2;
    }
    fprintf(survey.report,
            "INTEGRATED_BEGIN k=%d distance=%d total=%" PRIu64
            " skip=%" PRIu64 " limit=%" PRIu64 " memo=%" PRIu64 "\n",
            survey.k, survey.distance, survey.counted, survey.skip,
            survey.limit, survey.memo_entries);
    fflush(survey.report);
    survey_enumerate(&survey, 0, survey.mass, survey.parent[0], 0);
    survey_write_progress(&survey);
    fprintf(survey.report,
            "INTEGRATED_SUMMARY queries=%" PRIu64 " solvable=%" PRIu64
            " unsolvable=%" PRIu64 " maybe=%" PRIu64
            " total_ms=%.0f wall_seconds=%.3f"
            " truncated_cache_insertions=%lld"
            " majorization_pruned_cache_branches=%lld\n",
            survey.tested, survey.solvable, survey.unsolvable, survey.maybe,
            survey.total_ms, survey_elapsed(&survey), truncated_cache_insertions,
            majorization_pruned_cache_branches);
    fflush(survey.report);
    return survey.tested == survey.limit && survey.maybe == 0 ? 0 : 1;
}
