/*
 * Frozen negative-certificate refuter.
 *
 * This deliberately reuses the production solver's theorem checks, compact dominance trie,
 * split geometry, ordering and reachability prune, but not its recursive solving policy.  The
 * input is loaded once as negative claims, the cache and split tables are frozen, and workers then
 * audit roots independently.  A level-k claim bypasses its own cache entry and exhausts every
 * legal test; each child may be discharged only by a theorem or the immutable k-1 negative trie.
 * A miss is an audit gap, never an invitation to solve, learn, or write a cache fact.
 *
 * Build:
 *   tools/build_radio.py -O3 -pthread -DMAX_K=10 -DMAX_N=194 radio_refute.c -o radio_refute
 * Run:
 *   REFUTE_THREADS=16 REFUTE_PROGRESS_SECONDS=60 \
 *     tools/run_with_provenance.py ./radio_refute certificate.cert
 *
 * Optional environment filters (all are recorded by radiobase provenance):
 *   REFUTE_MIN_K, REFUTE_MAX_K, REFUTE_MIN_PARTS, REFUTE_MAX_PARTS,
 *   REFUTE_STRIDE, REFUTE_OFFSET, REFUTE_THREADS, REFUTE_PROGRESS_SECONDS.
 */

#include <pthread.h>
#include <stdatomic.h>
#include <errno.h>
#include <math.h>

#include "radiobase.c"

#define CERT_HEADER "radio-negative-certificate-v1"
#define MAX_CERT_PARTS 40
#define LINE_BYTES (1u << 16)

typedef struct {
    uint8_t np;
    uint16_t part[MAX_CERT_PARTS];
} Claim;

typedef struct {
    Claim *v;
    size_t n;
    size_t cap;
} ClaimLevel;

typedef struct {
    uint32_t index;
    uint8_t k;
} Task;

static ClaimLevel claims[MAX_K + 1];
static pthread_mutex_t output_mu = PTHREAD_MUTEX_INITIALIZER;

static double monotonic_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static uint64_t monotonic_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * UINT64_C(1000000000) + (uint64_t)ts.tv_nsec;
}

static long env_long(const char *name, long def, long lo, long hi) {
    const char *s = getenv(name);
    char *end;
    long v;
    if (s == NULL || *s == 0) return def;
    errno = 0;
    v = strtol(s, &end, 10);
    if (errno || end == s || *end || v < lo || v > hi) {
        fprintf(stderr, "%s must be in %ld..%ld\n", name, lo, hi);
        exit(2);
    }
    return v;
}

static double env_double(const char *name, double def, double lo, double hi) {
    const char *s = getenv(name);
    char *end;
    double v;
    if (s == NULL || *s == 0) return def;
    errno = 0;
    v = strtod(s, &end);
    if (errno || end == s || *end || !isfinite(v) || v < lo || v > hi) {
        fprintf(stderr, "%s must be in %.0f..%.0f\n", name, lo, hi);
        exit(2);
    }
    return v;
}

static int cert_header(const char *p) {
    size_t n = strlen(CERT_HEADER);
    return strncmp(p, CERT_HEADER, n) == 0
        && (p[n] == 0 || p[n] == '\n' || p[n] == '\r' || p[n] == ' ' || p[n] == '\t');
}

static void append_claim(int k, const int *parts, int np) {
    ClaimLevel *L = &claims[k];
    Claim *c;
    if (L->n == L->cap) {
        size_t next = L->cap ? L->cap * 2 : 1024;
        Claim *grown = (Claim *)realloc(L->v, next * sizeof(*grown));
        if (grown == NULL) {
            fprintf(stderr, "out of memory growing level %d claims\n", k);
            exit(2);
        }
        L->v = grown;
        L->cap = next;
    }
    c = &L->v[L->n++];
    c->np = (uint8_t)np;
    for (int i = 0; i < np; i++) c->part[i] = (uint16_t)parts[i];
}

static int parse_claim_line(char *line, int header_seen, int lineno) {
    char *p = line;
    char *end;
    int root;
    long k;
    int parts[MAX_CERT_PARTS];
    int np = 0;

    while (*p == ' ' || *p == '\t') p++;
    if (*p == 0 || *p == '\n' || *p == '\r' || *p == '#') return 0;
    if (cert_header(p) || strncmp(p, "meta ", 5) == 0) return 0;
    if (!header_seen) {
        fprintf(stderr, "line %d: record before %s header\n", lineno, CERT_HEADER);
        return -1;
    }
    if (strncmp(p, "fact ", 5) == 0) root = 0;
    else if (strncmp(p, "root ", 5) == 0) root = 1;
    else {
        fprintf(stderr, "line %d: unknown certificate record\n", lineno);
        return -1;
    }
    (void)root; /* Roots and support facts are both negative claims in a full audit. */
    p += 5;
    while (*p == ' ' || *p == '\t') p++;
    errno = 0;
    k = strtol(p, &end, 10);
    if (errno || end == p || k < 1 || k > MAX_K) {
        fprintf(stderr, "line %d: invalid k\n", lineno);
        return -1;
    }
    p = end;
    while (*p == ' ' || *p == '\t') p++;
    if (strncmp(p, "Sb(", 3) != 0) {
        fprintf(stderr, "line %d: expected Sb state\n", lineno);
        return -1;
    }
    p += 3;
    while (*p && *p != ')') {
        long n, m;
        int sbb;
        if (np >= MAX_CERT_PARTS) {
            fprintf(stderr, "line %d: more than %d parts\n", lineno, MAX_CERT_PARTS);
            return -1;
        }
        errno = 0;
        n = strtol(p, &end, 10);
        if (errno || end == p || *end != ':') {
            fprintf(stderr, "line %d: malformed part\n", lineno);
            return -1;
        }
        p = end + 1;
        errno = 0;
        m = strtol(p, &end, 10);
        if (errno || end == p || n < 1 || m < 1 || n > MAX_N || m > MAX_N
                || n + m > MAX_N) {
            fprintf(stderr, "line %d: part outside compiled MAX_N=%d\n", lineno, MAX_N);
            return -1;
        }
        sbb = getSbb((int)n, (int)m);
        if (sbb <= 0 || sbb > MAX_SBB) {
            fprintf(stderr, "line %d: part has no compiled encoding\n", lineno);
            return -1;
        }
        parts[np++] = sbb;
        p = end;
        if (*p == ',') p++;
        else if (*p != ')') {
            fprintf(stderr, "line %d: malformed part separator\n", lineno);
            return -1;
        }
    }
    if (*p != ')' || np == 0) {
        fprintf(stderr, "line %d: unterminated or empty Sb state\n", lineno);
        return -1;
    }
    p++;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (*p != 0 && *p != '#') {
        fprintf(stderr, "line %d: trailing certificate data\n", lineno);
        return -1;
    }
    sort1(parts, np);
    append_claim((int)k, parts, np);
    return 1;
}

static size_t read_certificate(const char *path) {
    FILE *fp = fopen(path, "r");
    char line[LINE_BYTES];
    size_t total = 0;
    int lineno = 0;
    int header_seen = 0;
    if (fp == NULL) {
        fprintf(stderr, "cannot open certificate %s\n", path);
        exit(2);
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        char *p = line;
        int parsed;
        lineno++;
        if (strchr(line, '\n') == NULL && !feof(fp)) {
            fprintf(stderr, "%s:%d: line exceeds %u bytes\n", path, lineno, LINE_BYTES - 1);
            exit(2);
        }
        while (*p == ' ' || *p == '\t') p++;
        if (cert_header(p)) header_seen = 1;
        parsed = parse_claim_line(line, header_seen, lineno);
        if (parsed < 0) exit(2);
        total += (size_t)parsed;
    }
    if (ferror(fp)) {
        fprintf(stderr, "error reading certificate %s\n", path);
        exit(2);
    }
    fclose(fp);
    if (!header_seen) {
        fprintf(stderr, "%s: missing %s header\n", path, CERT_HEADER);
        exit(2);
    }
    return total;
}

static void expand_claim(const Claim *c, int *parts) {
    for (int i = 0; i < c->np; i++) parts[i] = (int)c->part[i];
}

static void print_claim(FILE *fp, int k, const Claim *c) {
    fprintf(fp, "k=%d Sb(", k);
    for (int i = 0; i < c->np; i++) {
        if (i) fputc(',', fp);
        fputs(sbb_to_str[c->part[i]], fp);
    }
    fputc(')', fp);
}

static void load_negative_cache(size_t total) {
    size_t done = 0;
    double start = monotonic_seconds();
    cache_replay_depth++;
    printf("CACHE_START claims=%zu\n", total);
    fflush(stdout);
    for (int k = 1; k <= MAX_K; k++) {
        ClaimLevel *L = &claims[k];
        for (size_t q = 0; q < L->n; q++) {
            int parts[MAX_CERT_PARTS];
            int pairs = 0;
            expand_claim(&L->v[q], parts);
            for (int i = 0; i < L->v[q].np; i++) pairs += sb_pairs[parts[i]];
            cache(parts, L->v[q].np, FALSE, k, pairs);
            done++;
            if (done % 100000 == 0 || done == total) {
                double elapsed = monotonic_seconds() - start;
                printf("CACHE_PROGRESS completed=%zu/%zu percent=%.4f rate=%.1f/s "
                       "branches=%lld fronts=%lld\n",
                       done, total, total ? 100.0 * (double)done / (double)total : 100.0,
                       elapsed > 0 ? (double)done / elapsed : 0.0,
                       alloc_count, front_alloc_count);
                fflush(stdout);
            }
        }
    }
    cache_replay_depth--;
    printf("CACHE_DONE claims=%zu wall_s=%.3f branches=%lld branch_bytes=%lld "
           "fronts=%lld front_bytes=%lld redundant=%lld\n",
           total, monotonic_seconds() - start, alloc_count, branch_alloc_size,
           front_alloc_count, front_alloc_size, redundant_cache_replays);
    fflush(stdout);
}

/* Reproduce the solver's per-part split viability frontier before publishing the read epoch.
   Every query is CACHE_ONLY and strictly below the audited root level. */
static size_t freeze_one_table(radio_search_context *ctx, int sbb, int k) {
    splits *sp = ensure_splits(sbb, k);
    for (int c = 0; c < sp->size; c++) {
        int *s = sp->splitsl[c];
        while (s[4] < k) {
            int kk = s[4];
            int verdict = canSolveB_ctx(ctx, s, 1, kk, CACHE_ONLY);
            if (verdict == TRUE)
                verdict = canSolveB_ctx(ctx, s + 3, 1, kk, CACHE_ONLY);
            if (verdict == TRUE)
                verdict = canSolveB_ctx(ctx, s + 1, 2, kk, CACHE_ONLY);
            if (verdict == TRUE) s[4] = MAX_K;
            else if (verdict == FALSE) s[5] = ++s[4];
            else break;
        }
    }
    return (size_t)sp->size;
}

typedef struct {
    Task *tasks;
    size_t ntasks;
    atomic_size_t next;
    atomic_size_t completed;
    atomic_size_t verified;
    atomic_size_t gaps;
    atomic_ullong prefixes;
    atomic_size_t completed_k[MAX_K + 1];
    atomic_size_t verified_k[MAX_K + 1];
    atomic_size_t gaps_k[MAX_K + 1];
    size_t total_k[MAX_K + 1];
    int threads;
    double progress_seconds;
    double started;
    struct WorkerProgress *progress;
    pthread_mutex_t progress_mu;
    pthread_cond_t progress_cv;
    int progress_stop;
} Batch;

typedef struct WorkerProgress {
    atomic_size_t task;
    atomic_ullong started_ns;
} WorkerProgress;

typedef struct {
    Batch *batch;
    int slot;
    unsigned long long prefixes;
    size_t verified;
    size_t gaps;
} WorkerArg;

static void report_gap(const Task *t, int verdict) {
    static int printed;
    const Claim *c = &claims[t->k].v[t->index];
    pthread_mutex_lock(&output_mu);
    if (printed < 100) {
        printf("GAP verdict=%s ", verdict == TRUE ? "contradicted" : "uncovered");
        print_claim(stdout, t->k, c);
        putchar('\n');
        fflush(stdout);
        printed++;
        if (printed == 100) {
            printf("GAP further_records_suppressed=yes\n");
            fflush(stdout);
        }
    }
    pthread_mutex_unlock(&output_mu);
}

static void *verify_worker(void *vp) {
    WorkerArg *a = (WorkerArg *)vp;
    Batch *b = a->batch;
    radio_search_context ctx;
    radio_search_context_init(&ctx);
    for (;;) {
        size_t q = atomic_fetch_add_explicit(&b->next, 1, memory_order_relaxed);
        Task *t;
        Claim *c;
        int parts[MAX_CERT_PARTS];
        uint64_t before;
        int verdict;
        if (q >= b->ntasks) break;
        t = &b->tasks[q];
        c = &claims[t->k].v[t->index];
        atomic_store_explicit(&b->progress[a->slot].started_ns, monotonic_ns(),
                              memory_order_relaxed);
        atomic_store_explicit(&b->progress[a->slot].task, q, memory_order_release);
        expand_claim(c, parts);
        before = radio_work_units_used_ctx(&ctx);
        verdict = canRefuteB_ctx(&ctx, parts, c->np, t->k);
        a->prefixes += (unsigned long long)(radio_work_units_used_ctx(&ctx) - before);
        if (verdict == FALSE) {
            a->verified++;
            atomic_fetch_add_explicit(&b->verified, 1, memory_order_relaxed);
            atomic_fetch_add_explicit(&b->verified_k[t->k], 1, memory_order_relaxed);
        } else {
            a->gaps++;
            atomic_fetch_add_explicit(&b->gaps, 1, memory_order_relaxed);
            atomic_fetch_add_explicit(&b->gaps_k[t->k], 1, memory_order_relaxed);
            report_gap(t, verdict);
        }
        atomic_fetch_add_explicit(&b->prefixes,
                                  (unsigned long long)(radio_work_units_used_ctx(&ctx) - before),
                                  memory_order_relaxed);
        atomic_fetch_add_explicit(&b->completed_k[t->k], 1, memory_order_relaxed);
        atomic_fetch_add_explicit(&b->completed, 1, memory_order_release);
        atomic_store_explicit(&b->progress[a->slot].task, SIZE_MAX, memory_order_release);
    }
    radio_search_context_destroy(&ctx);
    return NULL;
}

static void print_progress(Batch *b, size_t *last_done, double *last_time,
                           double *ewma_rate) {
    size_t done = atomic_load_explicit(&b->completed, memory_order_acquire);
    size_t verified = atomic_load_explicit(&b->verified, memory_order_relaxed);
    size_t gaps = atomic_load_explicit(&b->gaps, memory_order_relaxed);
    unsigned long long prefixes = atomic_load_explicit(&b->prefixes, memory_order_relaxed);
    double now = monotonic_seconds();
    double elapsed = now - b->started;
    double window = now - *last_time;
    double window_rate = window > 0 ? (double)(done - *last_done) / window : 0;
    double total_rate = elapsed > 0 ? (double)done / elapsed : 0;
    size_t active_task[3] = {SIZE_MAX, SIZE_MAX, SIZE_MAX};
    int active_worker[3] = {-1, -1, -1};
    double active_age[3] = {0, 0, 0};
    uint64_t now_ns = monotonic_ns();

    if (*ewma_rate == 0) *ewma_rate = window_rate;
    else if (window_rate > 0) *ewma_rate = 0.25 * window_rate + 0.75 * *ewma_rate;

    pthread_mutex_lock(&output_mu);
    printf("PROGRESS phase=frozen-refute elapsed_s=%.1f completed=%zu/%zu percent=%.4f "
           "verified=%zu gaps=%zu rate_total=%.3f/s rate_window=%.3f/s rate_ewma=%.3f/s "
           "prefixes_done=%llu prefix_rate=%.0f/s eta_s=%.0f\n",
           elapsed, done, b->ntasks, b->ntasks ? 100.0 * (double)done / (double)b->ntasks : 100.0,
           verified, gaps, total_rate, window_rate, *ewma_rate, prefixes,
           elapsed > 0 ? (double)prefixes / elapsed : 0.0,
           *ewma_rate > 0 ? (double)(b->ntasks - done) / *ewma_rate : -1.0);
    printf("PROGRESS_LEVELS phase=frozen-refute");
    for (int k = 1; k <= MAX_K; k++) {
        size_t kd = atomic_load_explicit(&b->completed_k[k], memory_order_relaxed);
        size_t kt = b->total_k[k];
        if (kt) printf(" k%d=%zu/%zu", k, kd, kt);
    }
    putchar('\n');

    for (int i = 0; i < b->threads; i++) {
        size_t q = atomic_load_explicit(&b->progress[i].task, memory_order_acquire);
        uint64_t started;
        double age;
        int pos;
        if (q == SIZE_MAX || q >= b->ntasks) continue;
        started = atomic_load_explicit(&b->progress[i].started_ns, memory_order_relaxed);
        if (atomic_load_explicit(&b->progress[i].task, memory_order_acquire) != q) continue;
        age = started && now_ns >= started ? (double)(now_ns - started) * 1e-9 : 0;
        for (pos = 0; pos < 3 && age <= active_age[pos]; pos++);
        if (pos < 3) {
            for (int z = 2; z > pos; z--) {
                active_age[z] = active_age[z - 1];
                active_task[z] = active_task[z - 1];
                active_worker[z] = active_worker[z - 1];
            }
            active_age[pos] = age;
            active_task[pos] = q;
            active_worker[pos] = i;
        }
    }
    for (int z = 0; z < 3 && active_worker[z] >= 0; z++) {
        Task *t = &b->tasks[active_task[z]];
        Claim *c = &claims[t->k].v[t->index];
        printf("PROGRESS_ACTIVE phase=frozen-refute worker=%d task=%zu age_s=%.1f np=%u ",
               active_worker[z], active_task[z], active_age[z], (unsigned)c->np);
        print_claim(stdout, t->k, c);
        putchar('\n');
    }
    fflush(stdout);
    pthread_mutex_unlock(&output_mu);
    *last_done = done;
    *last_time = now;
}

static struct timespec realtime_after(double seconds) {
    struct timespec ts;
    time_t whole = (time_t)seconds;
    long nanos = (long)((seconds - (double)whole) * 1e9);
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += whole;
    ts.tv_nsec += nanos;
    if (ts.tv_nsec >= 1000000000L) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000L;
    }
    return ts;
}

static uint64_t frozen_split_checksum(void) {
    uint64_t h = UINT64_C(1469598103934665603);
    for (int sbb = 1; sbb <= MAX_SBB; sbb++) {
        split_levels *levels = sbb_splits[sbb];
        if (levels == NULL) continue;
        for (int k = 1; k <= MAX_K; k++) {
            splits *sp = levels->at[k];
            if (sp == NULL) continue;
            h ^= (uint64_t)(unsigned)sbb | ((uint64_t)(unsigned)k << 32);
            h *= UINT64_C(1099511628211);
            for (int q = 0; q < sp->size; q++) {
                for (int z = 0; z < SPLIT_FIELD_COUNT; z++) {
                    h ^= (uint32_t)sp->splitsl[q][z];
                    h *= UINT64_C(1099511628211);
                }
            }
        }
    }
    return h;
}

static void *progress_worker(void *vp) {
    Batch *b = (Batch *)vp;
    size_t last_done = 0;
    double last_time = b->started;
    double ewma_rate = 0;
    pthread_mutex_lock(&b->progress_mu);
    while (!b->progress_stop) {
        struct timespec until = realtime_after(b->progress_seconds);
        int rc = 0;
        while (!b->progress_stop && rc != ETIMEDOUT)
            rc = pthread_cond_timedwait(&b->progress_cv, &b->progress_mu, &until);
        if (b->progress_stop) break;
        pthread_mutex_unlock(&b->progress_mu);
        print_progress(b, &last_done, &last_time, &ewma_rate);
        pthread_mutex_lock(&b->progress_mu);
    }
    pthread_mutex_unlock(&b->progress_mu);
    return NULL;
}

int main(int argc, char **argv) {
    const char *path;
    int min_k, max_k, min_parts, max_parts, threads;
    long stride, offset;
    double progress_seconds;
    size_t total_claims;
    size_t selected = 0;
    Task *tasks;
    unsigned char *needed;
    size_t needed_stride = (size_t)MAX_SBB + 1;
    radio_search_context prep_ctx;
    size_t tables = 0, options = 0;
    double freeze_start;
    Batch b;
    WorkerArg *args;
    pthread_t *ids;
    pthread_t reporter;
    int have_reporter = 0;
    double verify_wall;
    double verify_cpu;
    clock_t verify_cpu_start;
    long long frozen_cache_branches, frozen_cache_fronts;
    uint64_t split_checksum_before, split_checksum_after;

    if (argc != 2) {
        fprintf(stderr, "usage: %s certificate.cert\n", argv[0]);
        return 2;
    }
    path = argv[1];
    min_k = (int)env_long("REFUTE_MIN_K", 1, 1, MAX_K);
    max_k = (int)env_long("REFUTE_MAX_K", MAX_K, 1, MAX_K);
    min_parts = (int)env_long("REFUTE_MIN_PARTS", 1, 1, MAX_CERT_PARTS);
    max_parts = (int)env_long("REFUTE_MAX_PARTS", MAX_CERT_PARTS, 1, MAX_CERT_PARTS);
    threads = (int)env_long("REFUTE_THREADS", 1, 1, 256);
    stride = env_long("REFUTE_STRIDE", 1, 1, LONG_MAX);
    offset = env_long("REFUTE_OFFSET", 0, 0, stride - 1);
    progress_seconds = env_double("REFUTE_PROGRESS_SECONDS", 0, 0, 86400);
    if (min_k > max_k || min_parts > max_parts) {
        fprintf(stderr, "invalid inverted refute range\n");
        return 2;
    }

    init();
    total_claims = read_certificate(path);
    printf("INPUT certificate=%s claims=%zu filters=k%d..%d,np%d..%d,stride=%ld,offset=%ld "
           "threads=%d progress_seconds=%.1f\n",
           path, total_claims, min_k, max_k, min_parts, max_parts,
           stride, offset, threads, progress_seconds);
    for (int k = 1; k <= MAX_K; k++)
        if (claims[k].n) printf("INPUT_LEVEL k=%d claims=%zu\n", k, claims[k].n);
    fflush(stdout);

    load_negative_cache(total_claims);
    frozen_cache_branches = alloc_count;
    frozen_cache_fronts = front_alloc_count;

    for (int k = min_k; k <= max_k; k++) {
        size_t eligible = 0;
        for (size_t q = 0; q < claims[k].n; q++) {
            int np = claims[k].v[q].np;
            if (np < min_parts || np > max_parts) continue;
            if ((long)(eligible % (size_t)stride) == offset) selected++;
            eligible++;
        }
    }
    if (selected == 0) {
        fprintf(stderr, "no claims selected\n");
        return 2;
    }
    tasks = (Task *)malloc(selected * sizeof(*tasks));
    needed = (unsigned char *)calloc(((size_t)MAX_K + 1) * needed_stride, 1);
    if (tasks == NULL || needed == NULL) {
        fprintf(stderr, "out of memory allocating tasks/freeze bitmap\n");
        return 2;
    }
    {
        size_t w = 0;
        for (int k = min_k; k <= max_k; k++) {
            size_t eligible = 0;
            for (size_t q = 0; q < claims[k].n; q++) {
                Claim *c = &claims[k].v[q];
                if (c->np < min_parts || c->np > max_parts) continue;
                if ((long)(eligible % (size_t)stride) == offset) {
                    tasks[w].k = (uint8_t)k;
                    tasks[w].index = (uint32_t)q;
                    w++;
                    for (int i = 0; i < c->np; i++)
                        needed[(size_t)k * needed_stride + c->part[i]] = 1;
                }
                eligible++;
            }
        }
        if (w != selected) {
            fprintf(stderr, "internal selected-task count mismatch\n");
            return 2;
        }
    }

    radio_search_context_init(&prep_ctx);
    freeze_start = monotonic_seconds();
    printf("FREEZE_START selected=%zu\n", selected);
    fflush(stdout);
    for (int k = min_k; k <= max_k; k++) {
        for (int sbb = 1; sbb <= MAX_SBB; sbb++) {
            if (!needed[(size_t)k * needed_stride + (size_t)sbb]) continue;
            options += freeze_one_table(&prep_ctx, sbb, k);
            tables++;
            if (tables % 100 == 0) {
                printf("FREEZE_PROGRESS tables=%zu options=%zu wall_s=%.3f\n",
                       tables, options, monotonic_seconds() - freeze_start);
                fflush(stdout);
            }
        }
    }
    radio_search_context_destroy(&prep_ctx);
    free(needed);
    printf("FREEZE_DONE tables=%zu options=%zu wall_s=%.3f\n",
           tables, options, monotonic_seconds() - freeze_start);
    split_checksum_before = frozen_split_checksum();
    printf("FROZEN_EPOCH cache_branches=%lld cache_fronts=%lld split_checksum=%016llx\n",
           frozen_cache_branches, frozen_cache_fronts,
           (unsigned long long)split_checksum_before);
    fflush(stdout);

    memset(&b, 0, sizeof(b));
    b.tasks = tasks;
    b.ntasks = selected;
    b.threads = threads;
    b.progress_seconds = progress_seconds;
    b.progress = (WorkerProgress *)calloc((size_t)threads, sizeof(*b.progress));
    args = (WorkerArg *)calloc((size_t)threads, sizeof(*args));
    ids = threads > 1 ? (pthread_t *)calloc((size_t)threads - 1, sizeof(*ids)) : NULL;
    if (b.progress == NULL || args == NULL || (threads > 1 && ids == NULL)) {
        fprintf(stderr, "out of memory allocating worker batch\n");
        return 2;
    }
    atomic_init(&b.next, 0);
    atomic_init(&b.completed, 0);
    atomic_init(&b.verified, 0);
    atomic_init(&b.gaps, 0);
    atomic_init(&b.prefixes, 0);
    for (int k = 0; k <= MAX_K; k++) {
        atomic_init(&b.completed_k[k], 0);
        atomic_init(&b.verified_k[k], 0);
        atomic_init(&b.gaps_k[k], 0);
    }
    for (int i = 0; i < threads; i++) {
        atomic_init(&b.progress[i].task, SIZE_MAX);
        atomic_init(&b.progress[i].started_ns, 0);
        args[i].batch = &b;
        args[i].slot = i;
    }
    for (size_t q = 0; q < selected; q++) b.total_k[tasks[q].k]++;
    pthread_mutex_init(&b.progress_mu, NULL);
    pthread_cond_init(&b.progress_cv, NULL);
    b.started = monotonic_seconds();
    verify_cpu_start = clock();
    printf("BATCH_START phase=frozen-refute targets=%zu threads=%d progress_seconds=%.1f\n",
           selected, threads, progress_seconds);
    fflush(stdout);
    if (progress_seconds > 0) {
        if (pthread_create(&reporter, NULL, progress_worker, &b) != 0) {
            fprintf(stderr, "cannot create progress reporter\n");
            return 2;
        }
        have_reporter = 1;
    }
    for (int i = 0; i < threads - 1; i++) {
        if (pthread_create(&ids[i], NULL, verify_worker, &args[i]) != 0) {
            fprintf(stderr, "cannot create verifier worker %d\n", i);
            return 2;
        }
    }
    verify_worker(&args[threads - 1]);
    for (int i = 0; i < threads - 1; i++) pthread_join(ids[i], NULL);
    if (have_reporter) {
        pthread_mutex_lock(&b.progress_mu);
        b.progress_stop = 1;
        pthread_cond_signal(&b.progress_cv);
        pthread_mutex_unlock(&b.progress_mu);
        pthread_join(reporter, NULL);
    }
    verify_wall = monotonic_seconds() - b.started;
    verify_cpu = (double)(clock() - verify_cpu_start) / CLOCKS_PER_SEC;
    split_checksum_after = frozen_split_checksum();
    if (alloc_count != frozen_cache_branches || front_alloc_count != frozen_cache_fronts
            || split_checksum_after != split_checksum_before) {
        fprintf(stderr, "frozen epoch mutated: cache=%lld/%lld fronts=%lld/%lld "
                        "splits=%016llx/%016llx\n",
                frozen_cache_branches, alloc_count, frozen_cache_fronts, front_alloc_count,
                (unsigned long long)split_checksum_before,
                (unsigned long long)split_checksum_after);
        return 2;
    }
    printf("BATCH_DONE phase=frozen-refute completed=%zu/%zu verified=%zu gaps=%zu "
           "prefixes=%llu wall_s=%.3f cpu_s=%.3f rate=%.3f/s prefix_rate=%.0f/s\n",
           atomic_load(&b.completed), selected, atomic_load(&b.verified), atomic_load(&b.gaps),
           (unsigned long long)atomic_load(&b.prefixes), verify_wall, verify_cpu,
           verify_wall > 0 ? (double)selected / verify_wall : 0.0,
           verify_wall > 0 ? (double)atomic_load(&b.prefixes) / verify_wall : 0.0);
    for (int k = min_k; k <= max_k; k++) {
        size_t completed = atomic_load(&b.completed_k[k]);
        if (completed)
            printf("RESULT_LEVEL k=%d completed=%zu verified=%zu gaps=%zu\n", k, completed,
                   atomic_load(&b.verified_k[k]), atomic_load(&b.gaps_k[k]));
    }
    printf("TOTAL verified %zu, gaps %zu, prefixes %llu\n",
           atomic_load(&b.verified), atomic_load(&b.gaps),
           (unsigned long long)atomic_load(&b.prefixes));
    fflush(stdout);

    pthread_cond_destroy(&b.progress_cv);
    pthread_mutex_destroy(&b.progress_mu);
    free(ids);
    free(args);
    free(b.progress);
    free(tasks);
    for (int k = 0; k <= MAX_K; k++) free(claims[k].v);
    return atomic_load(&b.gaps) == 0 ? 0 : 1;
}
