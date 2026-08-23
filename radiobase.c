#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/utsname.h>

#ifdef __APPLE__
#include <crt_externs.h>
#include <sys/sysctl.h>
#endif

/*
 * Build provenance is normally injected by tools/build_radio.py through a generated forced-include
 * header.  Keep an explicit fallback for direct compiler invocations: old habits must produce an
 * obviously incomplete record, not a log which can later be mistaken for an identified build.
 */
#ifndef RADIO_BUILD_PROVENANCE_AVAILABLE
#define RADIO_BUILD_PROVENANCE_AVAILABLE 0
#define RADIO_BUILD_PROVENANCE_COMPLETE 0
#define RADIO_BUILD_ID "unknown-direct-build"
#define RADIO_GIT_COMMIT "unknown"
#define RADIO_GIT_IDENTITY_SOURCE "unknown"
#define RADIO_GIT_SOURCE_DIRTY "unknown"
#define RADIO_GIT_WORKTREE_DIRTY "unknown"
#define RADIO_BUILD_UTC "unknown"
#define RADIO_BUILD_HOST "unknown"
#define RADIO_BUILD_UNAME "unknown"
#define RADIO_BUILD_CWD "unknown"
#define RADIO_COMPILER_VERSION "unknown"
#define RADIO_COMPILER_SHA256 "unknown"
#define RADIO_BUILD_TOOL_SHA256 "unknown"
#define RADIO_PROVENANCE_INJECTION "unknown"
#define RADIO_BUILD_ARGC 0
#define RADIO_BUILD_ENV_COUNT 0
#define RADIO_SOURCE_COUNT 0
static const char *const radio_provenance_build_argv[1] = {NULL};
static const char *const radio_provenance_build_env_names[1] = {NULL};
static const char *const radio_provenance_build_env_values[1] = {NULL};
static const char *const radio_provenance_source_paths[1] = {NULL};
static const char *const radio_provenance_source_sha256[1] = {NULL};
#endif

#ifdef DEBUG
#define debug_printf(...) do{ printf( __VA_ARGS__ ); fflush(stdout);} while( 0 )
#define DEBUG1 1
#else
#define debug_printf(...) /* Nothing */
#ifndef DEBUG1
#define OPT 1
#endif
#endif

#undef VERIFY_FAST

#ifndef MAX_K
#define MAX_K 10
#endif

#ifndef MAX_N
#define MAX_N 194
#endif

#define MAX_SBB MAX_N*MAX_N/4
#define MAX_SPLITS (MAX_N/2) * (MAX_N/2 + 2)
#define MAX_PROD (MAX_N/2)*(MAX_N - MAX_N/2)
#define BY_MAGIC 0
#define BY_MAX 1
#define BY_SP0 2
#define BY_SP1 3
#define BY_SP2 4
#define BY_MAGIC3 5
#define BY_MAGIC2 6
#define BY_SP0_DESC 7
#define BY_SP1_DESC 8
#define BY_SP2_DESC 9

#define INDEX_COUNT 10

#define PROGRESS_INTERVAL CLOCKS_PER_SEC*60

#define FALSE 0
#define TRUE 1
#define MAYBE 2

#define FAST 8
#define SPLIT_FIELD_COUNT 9

#define DEADLINE_RATIO 10
#define MIN_DEADLINE 3
#define DEADLINE_POLL_MASK 0xffffu
#ifndef PROBE_SECONDS
#define PROBE_SECONDS 2
#endif
#ifndef RADIO_INITIAL_PROBE_SECONDS
/* Research drivers may choose a larger first quantum for a narrowly scoped top-level exact
   query.  Recursive children still receive their ordinary bounded allowances unless the driver
   explicitly says otherwise.  The value is in nominal seconds: accepted-prefix work by default,
   process CPU under RADIO_CPU_BUDGET. */
#define RADIO_INITIAL_PROBE_SECONDS(k, size, parent_deadline) PROBE_SECONDS
#endif

#define CACHE_ONLY 1
#define NO_DEADLINE 2
#define FAST_ONLY 3
/* Exhaust one state against an already-frozen negative cache.  Unlike CACHE_ONLY, this bypasses
   the queried state's own level-k cache entry and enumerates its complete split space.  Every
   child is answered only by theorem checks or the immutable k-1 cache; a cache miss makes the
   claimed refutation unverified instead of recursively solving or learning anything. */
#define FROZEN_REFUTE 4

/* Per-search state is deliberately separate from the process-wide mathematical universe, result
   trie and split catalog.  The latter two are still mutable and therefore still serialize solver
   calls; this context is the first prerequisite for a later frozen-cache epoch.  In particular,
   it makes the deterministic work clock, exact L1 and joint-reachability scratch independently
   ownable by a worker without changing the legacy single-threaded API. */
typedef struct cache_l1_entry cache_l1_entry;
typedef struct radio_reachability_state radio_reachability_state;
typedef struct radio_search_context {
    uint64_t work_clock;
    cache_l1_entry *cache_l1;
    radio_reachability_state *reachability;
#ifdef RADIO_CACHE_CITATIONS
    uint64_t *cache_citation_bits;
    size_t cache_citation_count;
    unsigned long long cache_citation_hits;
#endif
#ifdef MEASURE_CACHE_L1
    unsigned long long cache_l1_queries;
    unsigned long long cache_l1_eligible;
    unsigned long long cache_l1_hits;
    unsigned long long cache_l1_stores;
    unsigned long long cache_l1_replacements;
#endif
    long long cant_solve_count;
    /* When set, every canSolveB_ctx call sharing this context measures and bounds its own effort
       against ITS OWN LOCAL totalsplits count (candidate splits tried at that level) rather than
       the shared, cross-call-tree work clock, and a child's cap propagates from its parent
       unchanged (absolute, not divided) instead of being sliced by search_deadline/
       probe_child_deadline's work-time-tuned policy. Off (0, the default for every existing
       context) leaves every current caller's behavior exactly as it was -- see radio_effort_now_ctx
       and the radius branches in search_deadline/probe_child_deadline. A caller opting in must
       never pass the NO_DEADLINE sentinel in this mode (it would be misread as a radius cap of 2);
       pass a large finite value instead for "run until genuinely resolved." */
    int radius_mode;
} radio_search_context;

#define RADIO_WORK_CLOCK_ORIGIN 1024ULL
static radio_search_context radio_default_search_context = {
    .work_clock = RADIO_WORK_CLOCK_ORIGIN,
};

void radio_search_context_init(radio_search_context *ctx) {
    memset(ctx, 0, sizeof(*ctx));
    ctx->work_clock = RADIO_WORK_CLOCK_ORIGIN;
}

void radio_search_context_destroy(radio_search_context *ctx);

#ifdef RADIO_CACHE_CITATIONS
/* Frozen-certificate coloring needs only reachability, not a globally contended reference count.
   Every worker writes its own dense bitset and the driver ORs them after the immutable epoch. */
static void radio_cache_citations_attach(radio_search_context *ctx, uint64_t *bits,
                                         size_t fact_count) {
    ctx->cache_citation_bits = bits;
    ctx->cache_citation_count = fact_count;
    ctx->cache_citation_hits = 0;
}

static inline __attribute__((always_inline)) void radio_cache_citation_record(
    radio_search_context *ctx, uint32_t source) {
    if (ctx->cache_citation_bits == NULL) return;
    if (source == 0 || (size_t)source > ctx->cache_citation_count) {
        printf("\ninvalid frozen-cache citation source %u/%zu\n",
               source, ctx->cache_citation_count);
        exit(19);
    }
    size_t bit = (size_t)source - 1;
    ctx->cache_citation_bits[bit >> 6] |= UINT64_C(1) << (bit & 63);
    ctx->cache_citation_hits++;
}
#endif

/* Deterministic accepted-prefix budgeting is the repository default.  RADIO_CPU_BUDGET retains a
   matched fallback for controlled comparisons and archaeology. */
#if defined(RADIO_WORK_BUDGET) && defined(RADIO_CPU_BUDGET)
#error "choose at most one of RADIO_WORK_BUDGET and RADIO_CPU_BUDGET"
#endif
#if !defined(RADIO_WORK_BUDGET) && !defined(RADIO_CPU_BUDGET)
#define RADIO_WORK_BUDGET 1
#endif

/* Search budgeting has two interchangeable clocks.  The default is a deterministic count of
   accepted split prefixes across the complete recursive call tree; RADIO_CPU_BUDGET restores the
   historical process-CPU clock for controlled comparisons.  The conversion constant is deliberately
   only a scheduling calibration: correctness never depends on a finite call finishing, because
   budget exhaustion returns MAYBE rather than FALSE.

   Keep the work counter monotone for the lifetime of the process.  Limits are absolute, exactly as
   the old clock_t deadlines were, so a child consumes its parent's allowance and an exhausted parent
   cannot manufacture a fresh budget. */
#if defined(RADIO_WORK_BUDGET) || defined(RADIO_MEASURE_WORK)
/* Stay clear of the historical small integer sentinels CACHE_ONLY/NO_DEADLINE/FAST_ONLY when this
   counter itself is the scheduling clock.  Subtracting the origin yields the diagnostic work. */
static inline void radio_budget_charge_split_ctx(radio_search_context *ctx) {
    if (ctx->work_clock != UINT64_MAX) ctx->work_clock++;
}
static inline uint64_t radio_work_units_used_ctx(const radio_search_context *ctx) {
    return ctx->work_clock - RADIO_WORK_CLOCK_ORIGIN;
}
#else
static inline void radio_budget_charge_split_ctx(radio_search_context *ctx) { (void)ctx; }
static inline uint64_t radio_work_units_used_ctx(const radio_search_context *ctx) {
    (void)ctx;
    return 0;
}
#endif

static inline void radio_budget_charge_split(void) {
    radio_budget_charge_split_ctx(&radio_default_search_context);
}
static inline uint64_t radio_work_units_used(void) {
    return radio_work_units_used_ctx(&radio_default_search_context);
}

#ifdef RADIO_WORK_BUDGET
#ifndef RADIO_WORK_UNITS_PER_SECOND
#define RADIO_WORK_UNITS_PER_SECOND 20000000ULL
#endif
#define RADIO_BUDGET_UNITS_PER_SECOND ((uint64_t)RADIO_WORK_UNITS_PER_SECOND)
static inline uint64_t radio_budget_now_ctx(const radio_search_context *ctx) {
    return ctx->work_clock;
}
#else
#define RADIO_BUDGET_UNITS_PER_SECOND ((uint64_t)CLOCKS_PER_SEC)
static inline uint64_t radio_budget_now_ctx(const radio_search_context *ctx) {
    (void)ctx;
    return (uint64_t)clock();
}
#endif

static inline uint64_t radio_budget_now(void) {
    return radio_budget_now_ctx(&radio_default_search_context);
}

static uint64_t radio_budget_add(uint64_t base, uint64_t amount) {
    return amount > UINT64_MAX - base ? UINT64_MAX : base + amount;
}

static uint64_t radio_budget_seconds(uint64_t seconds) {
    return seconds > UINT64_MAX / RADIO_BUDGET_UNITS_PER_SECOND
        ? UINT64_MAX : seconds * RADIO_BUDGET_UNITS_PER_SECOND;
}

static uint64_t radio_budget_after_seconds_ctx(const radio_search_context *ctx,
                                               uint64_t seconds) {
    return radio_budget_add(radio_budget_now_ctx(ctx), radio_budget_seconds(seconds));
}

static uint64_t radio_budget_after_seconds(uint64_t seconds) {
    return radio_budget_after_seconds_ctx(&radio_default_search_context, seconds);
}

static uint64_t radio_budget_after_milliseconds_ctx(const radio_search_context *ctx,
                                                    uint64_t milliseconds) {
    uint64_t seconds_part = milliseconds / 1000;
    uint64_t millis_part = milliseconds % 1000;
    uint64_t whole = radio_budget_seconds(seconds_part);
    /* Divide before multiplying so a deliberately large calibration cannot overflow here. */
    uint64_t fraction = (RADIO_BUDGET_UNITS_PER_SECOND / 1000) * millis_part
        + (RADIO_BUDGET_UNITS_PER_SECOND % 1000) * millis_part / 1000;
    uint64_t amount = radio_budget_add(whole, fraction);
    if (amount == 0) amount = 1;
    return radio_budget_add(radio_budget_now_ctx(ctx), amount);
}

static uint64_t radio_budget_after_milliseconds(uint64_t milliseconds) {
    return radio_budget_after_milliseconds_ctx(&radio_default_search_context, milliseconds);
}

/* A finite child gets a fraction of the allowance still owned by its parent.  In particular, an
   exhausted parent never manufactures a fresh MIN_DEADLINE interval for each child it tries; that
   was the source of zero-work retry loops. */
static uint64_t search_deadline(const radio_search_context *ctx, uint64_t parent_deadline,
                                uint64_t start, int size) {
    /* Radius mode propagates a child's cap unchanged: absolute, not relative. The design relies on
       a child's own per-part candidate lists naturally being no larger than its parent's, so the
       same numeric radius self-scales instead of needing to be divided the way a shared work-time
       budget does. NO_DEADLINE is a deliberate, meaningful value here (unlike work-budget mode's
       search_deadline below): it means "progressively widen your own radius until resolved, and
       tell your children to do the same" -- see radius_N/radius_grows in the caller. */
    if (ctx->radius_mode) return parent_deadline;
    if (parent_deadline == NO_DEADLINE)
        return radio_budget_add(start, radio_budget_seconds(1000));
    if (parent_deadline <= start)
        return parent_deadline;
    /* One- and two-segment states are the constructive spine: their diagonal/opposed-branch
       ordering is reliable enough to spend the caller's shared budget directly.  Dividing at
       these two layers starved the first genuinely long descendant before it reached its witness. */
    if (size <= 2)
        return parent_deadline;
    uint64_t remaining = parent_deadline - start;
    if (remaining > radio_budget_seconds(MIN_DEADLINE * DEADLINE_RATIO))
        return start + remaining / DEADLINE_RATIO;
    return parent_deadline;
}

static int deadline_expired(uint64_t deadline, uint64_t now) {
    return now > deadline;
}

/* Radius mode's actual bound is structural (each level's own splitindex range is capped at
   radius_N directly, see canSolveB_ctx and radius_truncated) rather than a running-total compared
   against a deadline -- "radius_N per segment" scales as N^size, not as a single shared count, so
   a totalsplits-vs-N comparison here would fire almost immediately for any size > 1 and defeat the
   cap's own purpose. Returning 0 makes every deadline_expired(...) call site below permanently
   false in radius mode, i.e. inert; totalsplits is accepted but unused, kept only so callers don't
   need their own radius_mode branch. Off (radius_mode == 0), this is exactly radio_budget_now_ctx:
   no behavior change for any existing caller. */
static inline uint64_t radio_effort_now_ctx(const radio_search_context *ctx, long long totalsplits) {
    (void)totalsplits;
    return ctx->radius_mode ? 0 : radio_budget_now_ctx(ctx);
}

/* Long states give each speculative child the current local quantum as well as sharing their
   absolute cap.  Short states keep the cap itself: those are the reliable constructive spine, and
   slicing it at every level prevents the first long child from ever receiving enough total time.
   The caller doubles the long-state quantum after every unresolved exhaustive pass, so a saturated
   cache cannot repeat the same bounded work forever and no split history is needed. */
static uint64_t probe_child_deadline(const radio_search_context *ctx, uint64_t child_cap,
                                     uint64_t now, unsigned int probe_seconds, int size) {
    /* Only ever called in work-budget mode -- canSolveB_ctx computes cd directly for radius_mode
       (see the cd computation there), since radius mode's child cap depends on radius_grows, which
       this function has no way to see. `ctx` is still accepted, unused, to keep call sites uniform. */
    (void)ctx;
    if (size <= 2) return child_cap;
    uint64_t quantum = radio_budget_seconds(probe_seconds);
    uint64_t local = radio_budget_add(now, quantum);
    return local < child_cap ? local : child_cap;
}

typedef struct {
    int size;
    int (*splitsl)[SPLIT_FIELD_COUNT];
    int *ind[INDEX_COUNT];
    /* cle[j][x] = how many of this part's options have key j <= x, where the keys are the three
       children's pair counts: j=0 is k0 (the key BY_SP2 is sorted by), j=1 is k1 (BY_SP1), j=2 is
       k2 (BY_SP0). Static per level and part, so built once per (k,sbb) and shared by every
       state using that key.
       It answers "how far will the level run before the counting bound retires it?" in one load,
       which is what lets the ordering be chosen by measurement instead of by a gap heuristic. */
    int *cle[3];
    int clen;
} splits;

/* Split admissibility depends on the number of tests left, so tables are keyed by both
   parent level and sbb.  The top-level array contains only one pointer per possible sbb;
   the MAX_K+1 pointer fanout is allocated only after that sbb is actually encountered. */
typedef struct {
    splits *at[MAX_K + 1];
} split_levels;

// The three _DESC orderings were materialised by indexDesc as exact reversals of their
// bases: ind[DESC][e] == ind[BASE][size-1-e]. Storing them cost 12 bytes per split for no
// information. Reverse the subscript instead; only four arrays are now allocated.
static const unsigned char ORDER_BASE[INDEX_COUNT] = {
    [BY_MAGIC] = BY_MAGIC, [BY_MAX] = BY_MAX, [BY_MAGIC2] = BY_MAGIC2,
    [BY_MAGIC3] = BY_MAGIC3,
    [BY_SP0] = BY_SP0, [BY_SP1] = BY_SP1, [BY_SP2] = BY_SP2,
    [BY_SP0_DESC] = BY_SP0, [BY_SP1_DESC] = BY_SP1, [BY_SP2_DESC] = BY_SP2,
};
static const unsigned char ORDER_REVERSED[INDEX_COUNT] = {
    [BY_SP0_DESC] = 1, [BY_SP1_DESC] = 1, [BY_SP2_DESC] = 1,
};

/* Which cumulative pair count is monotone non-decreasing along an ordering, or -1.
 *
 * BY_SP2's sort key is pairs2raw*(1+P) + |pairs1raw - pairs0raw| with P = sb_pairs[sbb], and
 * the tiebreak is < 1+P, so the key determines pairs2raw exactly. The loop walks the
 * descending-sorted index from the far end, i.e. in ascending key order, so sb_pairs[s[0]]
 * - and hence the running p0 - only ever grows. Once it passes max_pairs_1 every remaining
 * split at this level would fail the same test, so the level can be abandoned outright
 * instead of rejecting them one at a time. Same for BY_SP1 -> p1 and BY_SP0 -> p2.
 * The _DESC variants walk the other way (non-increasing), and BY_MAGIC3 is a distance, so
 * neither admits the cut. */
static const signed char ORDER_MONO_P[INDEX_COUNT] = {
    [BY_MAGIC] = -1, [BY_MAX] = -1, [BY_MAGIC2] = -1, [BY_MAGIC3] = -1,
    [BY_SP2] = 0, [BY_SP1] = 1, [BY_SP0] = 2,
    [BY_SP0_DESC] = -1, [BY_SP1_DESC] = -1, [BY_SP2_DESC] = -1,
};

/* The ordering is chosen once per level, so resolve base/direction there rather than on
   every one of the ~108M split-loop iterations. `ordp` points at the first element in
   iteration order and `ords` is the step, so the lookup is a single indexed load.

   NOTE (2026-08-23): this solver's own BY_MAGIC3 walk (via the countdown-index pattern in
   canSolveB_ctx's split loop) goes most-balanced-first, the OPPOSITE of concentric_search's own
   top-level sweep (which reads ind[BY_MAGIC3] forward from index 0, least-balanced-first). Not
   reconciled here -- radius mode no longer forces BY_MAGIC3 everywhere (kept as prior art, see
   docs/journal.md 2026-08-23), so this discrepancy only matters if/when a future change makes the
   two share an order again. */
#define HOIST_ORDER(lvl) do {                                                        \
    int _o = splitincr[lvl];                                                         \
    int _rev = ORDER_REVERSED[_o];                                                   \
    ordp[lvl] = splitsarr[lvl]->ind[ORDER_BASE[_o]] +                                \
                (_rev && splitsarr[lvl]->size > 0 ? splitsarr[lvl]->size - 1 : 0);    \
    ords[lvl] = _rev ? -1 : 1;                                                       \
    ordmono[lvl] = ORDER_MONO_P[_o];                                                 \
} while (0)

int power3[MAX_K+1];
int n_to_sbb[MAX_N+1][MAX_N/2 + 1];
int sbb_to_n1[MAX_SBB+1];
int sbb_to_n2[MAX_SBB+1];
#if MAX_N < 32768
uint32_t sbb_dominance_key[MAX_SBB+1];
#endif
char sbb_to_str[MAX_SBB+1][8];
int sb_pairs[MAX_SBB+1];
int sa_can[MAX_N+1];
int sa_cant[MAX_N+1];
int **sbb_lesser;
int **sbb_greater;
int max_sbb_for_pairs[MAX_PROD+1];
int singleton_base_len[MAX_K+1];
int singleton_base_prefix[MAX_K+1][1 << MAX_K];

split_levels **sbb_splits;

#ifdef MEASURE_FAST_REPLAY
int fast_replay_capture, fast_replay_pass, fast_replay_fast;
unsigned long long fast_replay_splits;
unsigned long long fast_replay_first_splits, fast_replay_first_ok[32];
int fast_replay_first_depth;
#endif

splits *ensure_splits(int sbb, int k);
splits *prepare_splits(int sbb, int k, int need_fast);
splits *prepare_splits_ctx(radio_search_context *ctx, int sbb, int k, int need_fast);
int minK(int);
int minK_ctx(radio_search_context *ctx, int sbb);
int canSolveB_ctx(radio_search_context *ctx, int *sb, int size, int k,
                  uint64_t parent_deadline);
void init_singleton_majorization(void);
int singleton_majorization_can_solve(int *sb, int size, int k);
int star_expansion_majorization_can_solve(int *sb, int size, int k);

int min(int a,int b){
    return a<b?a:b;
}

int desc (const void * a, const void * b) {
    return ( *(int*)b - *(int*)a );
}

static inline void sort1(int *x, int len) {
    // Called ~8.9M times per k=8 ladder, mean length 2.84, 81% of calls len<=3 (measured
    // 2026-08-03, tools/instrument.py counters). libc qsort - an opaque call with an
    // indirect comparator per comparison and memcpy-based swaps - costs far more than the
    // sort itself at those lengths. Descending, matching the `desc` comparator exactly.
    for (int i = 1; i < len; i++) {
        int v = x[i], j = i - 1;
        while (j >= 0 && x[j] < v) { x[j + 1] = x[j]; j--; }
        x[j + 1] = v;
    }
}

int max(int a,int b){
    return a>b?a:b;
}

int getSbb(int n1, int n2){
    if (n1<n2) return getSbb(n2,n1);
    if (n2==0) return 0;
    return n_to_sbb[n1][n2];
}

int saPairs(int n) {
    return n * (n-1) / 2;
}

void printSa(int n){
    printf("Sa(%d)[%d,%d]", n, saPairs(n), n);
}

void printSb(int *sb, int size){
    printf("Sb(");
    int pairs=0;
    int n=0;
    int i;
    for (i=0; i<size; i++) {
        if (i>0) printf(",");
        printf("%s",sbb_to_str[sb[i]]);
        pairs+=sb_pairs[sb[i]];
        n+=sbb_to_n1[sb[i]];
        n+=sbb_to_n2[sb[i]];
    }
    printf(")[%d,%d]",pairs,n);
}


#ifdef DEBUG_CACHE
#ifndef DEBUG
#define DEBUG 1
#undef debug_printf
#define debug_printf(...) do{ printf( __VA_ARGS__ ); fflush(stdout);} while( 0 )
#undef OPT
#define DEBUG_CACHE_ONLY 1
#endif
#endif

/* Each trie edge is a 32-bit tagged descriptor.  Most terminal nodes contain one positive and/or
   negative point and pack both directly into that descriptor.  Nodes with descendants point to a
   dense uint32 branch array; its otherwise-unused slots 0 and 1 hold the two front descriptors. */
typedef uint32_t node_descriptor;

#define NODE_TAG_MASK       0xc0000000u
#define NODE_RECORD_TAG     0x40000000u
#define NODE_BRANCH_TAG     0x80000000u
#define NODE_HANDLE_MASK    0x3fffffffu
#define NODE_INLINE_BITS    14u
#define NODE_INLINE_MASK    ((1u << NODE_INLINE_BITS) - 1u)

#define FRONT_VECTOR_TAG    0x80000000u
#define FRONT_HANDLE_MASK   0x7fffffffu

#if MAX_SBB <= UINT16_MAX
typedef uint16_t front_point;
#else
typedef uint32_t front_point;
#endif

typedef struct {
    uint32_t len;
    uint32_t cap;
    front_point sbb[];
} front_vector;

#ifdef RADIO_CACHE_CITATIONS
/* A coloring build keeps source ids behind the packed sbb array.  Lookups therefore retain the
   ordinary cache-friendly part scan and touch the wider source array only after a match. */
static uint32_t cache_citation_insert_source;

static size_t front_vector_source_offset(uint32_t cap) {
    size_t end = sizeof(front_vector) + (size_t)cap * sizeof(front_point);
    return (end + _Alignof(uint32_t) - 1) & ~((size_t)_Alignof(uint32_t) - 1);
}

static size_t front_vector_bytes(uint32_t cap) {
    return front_vector_source_offset(cap) + (size_t)cap * sizeof(uint32_t);
}

static uint32_t *front_vector_sources(front_vector *v) {
    return (uint32_t *)(void *)((char *)v + front_vector_source_offset(v->cap));
}

static void radio_cache_citation_set_insert_source(uint32_t source) {
    cache_citation_insert_source = source;
}
#else
static size_t front_vector_bytes(uint32_t cap) {
    return sizeof(front_vector) + (size_t)cap * sizeof(front_point);
}
#endif

typedef struct {
    uint32_t positive;
    uint32_t negative;
} front_record;

typedef struct {
    uint32_t width;
    uint32_t slot[];
} branch_array;

node_descriptor sb_cache_root[MAX_K + 1];

long long alloc_count = 0;
long long alloc_size = 0;
long long branch_alloc_size = 0;
long long front_alloc_count = 0;
long long front_alloc_size = 0;
long long redundant_cache_replays = 0;

static branch_array **branch_handles;
static uint32_t branch_handles_len = 1;
static uint32_t branch_handles_cap;
static uint32_t branch_free_handle;

static front_vector **front_handles;
static uint32_t front_handles_len = 1;
static uint32_t front_handles_cap;
static uint32_t front_free_handle;

static front_record *front_records;
static uint32_t front_records_len = 1;
static uint32_t front_records_cap;
static uint32_t front_record_free;
static uint32_t front_record_live;

/* Demand-materialise only hot exact answers.  Definitive verdicts are mathematical facts and
   therefore never need invalidation; MAYBE is deliberately not retained.  Full state equality is
   checked after the hash, so collisions affect hit rate only, never correctness.  Longer states
   bypass this small front cache. */
#ifndef CACHE_L1_BITS
#define CACHE_L1_BITS 16u
#endif
#define CACHE_L1_SIZE (1u << CACHE_L1_BITS)
#define CACHE_L1_MAX_PARTS 12u
struct cache_l1_entry {
    uint32_t hash;
    front_point part[CACHE_L1_MAX_PARTS];
    uint8_t size;
    uint8_t k;
    uint8_t verdict_plus_one;
    uint8_t padding;
};
static cache_l1_entry radio_default_cache_l1[CACHE_L1_SIZE];

#ifndef RADIO_DISABLE_CACHE_L1
static cache_l1_entry *radio_search_context_cache_l1(radio_search_context *ctx) {
    if (ctx->cache_l1 == NULL) {
        if (ctx == &radio_default_search_context) {
            ctx->cache_l1 = radio_default_cache_l1;
        } else {
            ctx->cache_l1 = (cache_l1_entry *)calloc(CACHE_L1_SIZE, sizeof(*ctx->cache_l1));
            if (ctx->cache_l1 == NULL) {
                printf("\nout of memory allocating worker exact cache\n");
                exit(1);
            }
        }
    }
    return ctx->cache_l1;
}
#endif

#ifdef MEASURE_CACHE_L1
static void print_cache_l1_stats(void) {
    radio_search_context *ctx = &radio_default_search_context;
    fprintf(stderr,
            "CACHE_L1 queries=%llu eligible=%llu hits=%llu stores=%llu replacements=%llu\n",
            ctx->cache_l1_queries, ctx->cache_l1_eligible, ctx->cache_l1_hits,
            ctx->cache_l1_stores, ctx->cache_l1_replacements);
}
#endif

static int cache_replay_depth;

static void *grow_handle_table(void *old, uint32_t oldcap, uint32_t newcap, size_t item_size,
                               const char *what) {
    void *p = realloc(old, (size_t)newcap * item_size);
    if (p == NULL) {
        printf("\nout of memory growing %s handle table\n", what);
        exit(1);
    }
    memset((char *)p + (size_t)oldcap * item_size, 0,
           (size_t)(newcap - oldcap) * item_size);
    return p;
}

static inline branch_array *branch_for(uint32_t descriptor) {
    uint32_t handle = descriptor & NODE_HANDLE_MASK;
#ifndef OPT
    if ((descriptor & NODE_TAG_MASK) != NODE_BRANCH_TAG || handle == 0 ||
        handle >= branch_handles_len || branch_handles[handle] == NULL) {
        printf("\ninvalid branch descriptor %u\n", descriptor);
        exit(19);
    }
#endif
    return branch_handles[handle];
}

static uint32_t alloc_branch(uint32_t width) {
    uint32_t handle;
    if (branch_free_handle) {
        handle = branch_free_handle;
        branch_free_handle = (uint32_t)(uintptr_t)branch_handles[handle];
    } else {
        if (branch_handles_len >= branch_handles_cap) {
            uint32_t newcap = branch_handles_cap ? branch_handles_cap * 2 : 1024;
            branch_handles = (branch_array **)grow_handle_table(
                branch_handles, branch_handles_cap, newcap, sizeof(*branch_handles), "branch");
            branch_handles_cap = newcap;
        }
        handle = branch_handles_len++;
        if (handle > NODE_HANDLE_MASK) {
            printf("\nout of branch handles\n");
            exit(1);
        }
    }
    size_t bytes = sizeof(branch_array) + (size_t)width * sizeof(uint32_t);
    branch_array *b = (branch_array *)calloc(1, bytes);
    if (b == NULL) {
        printf("\nout of memory allocating cache branch\n");
        exit(1);
    }
    b->width = width;
    branch_handles[handle] = b;
    alloc_count++;
    alloc_size += width;
    branch_alloc_size += (long long)bytes;
    return NODE_BRANCH_TAG | handle;
}

static void release_branch(uint32_t descriptor) {
    uint32_t handle = descriptor & NODE_HANDLE_MASK;
    branch_array *b = branch_for(descriptor);
    size_t bytes = sizeof(branch_array) + (size_t)b->width * sizeof(uint32_t);
    alloc_count--;
    alloc_size -= b->width;
    branch_alloc_size -= (long long)bytes;
    free(b);
    branch_handles[handle] = (branch_array *)(uintptr_t)branch_free_handle;
    branch_free_handle = handle;
}

static inline front_vector *front_vector_for(uint32_t descriptor) {
    uint32_t handle = descriptor & FRONT_HANDLE_MASK;
#ifndef OPT
    if (!(descriptor & FRONT_VECTOR_TAG) || handle == 0 || handle >= front_handles_len ||
        front_handles[handle] == NULL) {
        printf("\ninvalid front descriptor %u\n", descriptor);
        exit(19);
    }
#endif
    return front_handles[handle];
}

static uint32_t alloc_front_vector(uint32_t a, uint32_t b) {
    uint32_t handle;
    if (front_free_handle) {
        handle = front_free_handle;
        front_free_handle = (uint32_t)(uintptr_t)front_handles[handle];
    } else {
        if (front_handles_len >= front_handles_cap) {
            uint32_t newcap = front_handles_cap ? front_handles_cap * 2 : 1024;
            front_handles = (front_vector **)grow_handle_table(
                front_handles, front_handles_cap, newcap, sizeof(*front_handles), "front");
            front_handles_cap = newcap;
        }
        handle = front_handles_len++;
        if (handle > FRONT_HANDLE_MASK) {
            printf("\nout of front handles\n");
            exit(1);
        }
    }
    uint32_t cap = 4;
    size_t bytes = front_vector_bytes(cap);
    front_vector *v = (front_vector *)malloc(bytes);
    if (v == NULL) {
        printf("\nout of memory allocating Pareto front\n");
        exit(1);
    }
    v->len = 2;
    v->cap = cap;
    v->sbb[0] = (front_point)a;
    v->sbb[1] = (front_point)b;
#ifdef RADIO_CACHE_CITATIONS
    front_vector_sources(v)[0] = 0;
    front_vector_sources(v)[1] = 0;
#endif
    front_handles[handle] = v;
    front_alloc_count++;
    front_alloc_size += (long long)bytes;
    return FRONT_VECTOR_TAG | handle;
}

#ifdef RADIO_CACHE_CITATIONS
static uint32_t alloc_negative_front(uint32_t sbb, uint32_t source) {
    uint32_t descriptor = alloc_front_vector(sbb, sbb);
    front_vector *v = front_vector_for(descriptor);
    v->len = 1;
    v->cap = 4;
    front_vector_sources(v)[0] = source;
    return descriptor;
}
#endif

static void release_front_vector(uint32_t descriptor) {
    if (!(descriptor & FRONT_VECTOR_TAG)) return;
    uint32_t handle = descriptor & FRONT_HANDLE_MASK;
    front_vector *v = front_vector_for(descriptor);
    size_t bytes = front_vector_bytes(v->cap);
    free(v);
    front_alloc_count--;
    front_alloc_size -= (long long)bytes;
    front_handles[handle] = (front_vector *)(uintptr_t)front_free_handle;
    front_free_handle = handle;
}

static front_vector *grow_front_vector(uint32_t descriptor) {
    uint32_t handle = descriptor & FRONT_HANDLE_MASK;
    front_vector *old = front_vector_for(descriptor);
    uint32_t oldcap = old->cap;
    uint32_t newcap = oldcap * 2;
    size_t oldbytes = front_vector_bytes(oldcap);
    size_t newbytes = front_vector_bytes(newcap);
    front_vector *v = (front_vector *)realloc(old, newbytes);
    if (v == NULL) {
        printf("\nout of memory growing Pareto front\n");
        exit(1);
    }
#ifdef RADIO_CACHE_CITATIONS
    /* The source array begins after capacity, so growing the packed point area moves it. */
    uint32_t len = v->len;
    size_t old_sources = front_vector_source_offset(oldcap);
    size_t new_sources = front_vector_source_offset(newcap);
    memmove((char *)v + new_sources, (char *)v + old_sources,
            (size_t)len * sizeof(uint32_t));
#endif
    v->cap = newcap;
    front_handles[handle] = v;
    front_alloc_size += (long long)(newbytes - oldbytes);
    return v;
}

static inline front_record *front_record_for(uint32_t descriptor) {
    uint32_t handle = descriptor & NODE_HANDLE_MASK;
#ifndef OPT
    if ((descriptor & NODE_TAG_MASK) != NODE_RECORD_TAG || handle == 0 ||
        handle >= front_records_len) {
        printf("\ninvalid front-record descriptor %u\n", descriptor);
        exit(19);
    }
#endif
    return &front_records[handle];
}

static uint32_t alloc_front_record(uint32_t positive, uint32_t negative) {
    uint32_t handle;
    if (front_record_free) {
        handle = front_record_free;
        front_record_free = front_records[handle].positive;
    } else {
        if (front_records_len >= front_records_cap) {
            uint32_t newcap = front_records_cap ? front_records_cap * 2 : 1024;
            front_record *records = (front_record *)realloc(
                front_records, (size_t)newcap * sizeof(*front_records));
            if (records == NULL) {
                printf("\nout of memory growing front-record arena\n");
                exit(1);
            }
            memset(records + front_records_cap, 0,
                   (size_t)(newcap - front_records_cap) * sizeof(*front_records));
            front_records = records;
            front_records_cap = newcap;
        }
        handle = front_records_len++;
        if (handle > NODE_HANDLE_MASK) {
            printf("\nout of front-record handles\n");
            exit(1);
        }
    }
    front_records[handle].positive = positive;
    front_records[handle].negative = negative;
    front_record_live++;
    return NODE_RECORD_TAG | handle;
}

static void release_front_record(uint32_t descriptor) {
    uint32_t handle = descriptor & NODE_HANDLE_MASK;
    (void)front_record_for(descriptor);
    front_records[handle].positive = front_record_free;
    front_records[handle].negative = 0;
    front_record_free = handle;
    front_record_live--;
}

static inline int part_greater_equal(int a, int b) {
#if MAX_N < 32768
    /* Put the two coordinates in independent 16-bit lanes.  Setting each lane's guard bit before
       subtraction prevents a borrow from crossing lanes; both guard bits survive exactly when
       both coordinates of a are at least those of b.  This replaces four scattered int loads and
       two comparisons on every Pareto-front probe with two packed loads and one branchless test. */
    uint32_t d = (sbb_dominance_key[a] | UINT32_C(0x80008000)) - sbb_dominance_key[b];
    return (d & UINT32_C(0x80008000)) == UINT32_C(0x80008000);
#else
    return sbb_to_n1[a] >= sbb_to_n1[b] && sbb_to_n2[a] >= sbb_to_n2[b];
#endif
}

static int front_has_greater_equal(uint32_t descriptor, int sbb) {
    if (descriptor == 0) return 0;
    if (!(descriptor & FRONT_VECTOR_TAG)) return part_greater_equal((int)descriptor, sbb);
    front_vector *v = front_vector_for(descriptor);
    for (uint32_t i = 0; i < v->len; i++)
        if (part_greater_equal((int)v->sbb[i], sbb)) return 1;
    return 0;
}

static int front_find_lesser_equal(uint32_t descriptor, int sbb, uint32_t *source_out) {
    if (source_out != NULL) *source_out = 0;
    if (descriptor == 0) return 0;
    if (!(descriptor & FRONT_VECTOR_TAG)) return part_greater_equal(sbb, (int)descriptor);
    front_vector *v = front_vector_for(descriptor);
    for (uint32_t i = 0; i < v->len; i++) {
        if (part_greater_equal(sbb, (int)v->sbb[i])) {
#ifdef RADIO_CACHE_CITATIONS
            if (source_out != NULL) *source_out = front_vector_sources(v)[i];
#endif
            return 1;
        }
    }
    return 0;
}

static int front_has_lesser_equal(uint32_t descriptor, int sbb) {
    return front_find_lesser_equal(descriptor, sbb, NULL);
}

static int add_front_descriptor(uint32_t *field, int positive, int sbb) {
    uint32_t descriptor = *field;
#ifdef RADIO_CACHE_CITATIONS
    if (!positive && cache_citation_insert_source == 0) {
        printf("\nnegative cache insertion has no citation source\n");
        exit(19);
    }
#endif
    if (descriptor == 0) {
#ifdef RADIO_CACHE_CITATIONS
        if (!positive) {
            *field = alloc_negative_front((uint32_t)sbb, cache_citation_insert_source);
            return 1;
        }
#endif
        *field = (uint32_t)sbb;
        return 1;
    }
    if (!(descriptor & FRONT_VECTOR_TAG)) {
#ifdef RADIO_CACHE_CITATIONS
        if (!positive) {
            printf("\ncolored negative cache contains an unattributed direct front\n");
            exit(19);
        }
#endif
        int old = (int)descriptor;
        int old_covers_new = positive ? part_greater_equal(old, sbb)
                                      : part_greater_equal(sbb, old);
        if (old_covers_new) return 0;
        int new_covers_old = positive ? part_greater_equal(sbb, old)
                                      : part_greater_equal(old, sbb);
        if (new_covers_old) {
            *field = (uint32_t)sbb;
            return 1;
        }
        *field = alloc_front_vector((uint32_t)old, (uint32_t)sbb);
        return 1;
    }

    front_vector *v = front_vector_for(descriptor);
#ifdef RADIO_CACHE_CITATIONS
    uint32_t *sources = front_vector_sources(v);
#endif
    for (uint32_t i = 0; i < v->len; i++) {
        int old = (int)v->sbb[i];
        int old_covers_new = positive ? part_greater_equal(old, sbb)
                                      : part_greater_equal(sbb, old);
        if (old_covers_new) return 0;
    }
    uint32_t w = 0;
    for (uint32_t i = 0; i < v->len; i++) {
        int old = (int)v->sbb[i];
        int new_covers_old = positive ? part_greater_equal(sbb, old)
                                      : part_greater_equal(old, sbb);
        if (!new_covers_old) {
            v->sbb[w] = v->sbb[i];
#ifdef RADIO_CACHE_CITATIONS
            sources[w] = sources[i];
#endif
            w++;
        }
    }
    if (w == 0) {
#ifdef RADIO_CACHE_CITATIONS
        if (!positive) {
            v->sbb[0] = (front_point)sbb;
            sources[0] = cache_citation_insert_source;
            v->len = 1;
            return 1;
        }
#endif
        release_front_vector(descriptor);
        *field = (uint32_t)sbb;
        return 1;
    }
    if (w == v->cap) v = grow_front_vector(descriptor);
    v->sbb[w++] = (front_point)sbb;
#ifdef RADIO_CACHE_CITATIONS
    front_vector_sources(v)[w - 1] = positive ? 0 : cache_citation_insert_source;
#endif
    v->len = w;
    return 1;
}

static void node_fronts(uint32_t node, uint32_t *positive, uint32_t *negative) {
    uint32_t tag = node & NODE_TAG_MASK;
    if (tag == NODE_BRANCH_TAG) {
        branch_array *b = branch_for(node);
        *positive = b->slot[0];
        *negative = b->slot[1];
    } else if (tag == NODE_RECORD_TAG) {
        front_record *r = front_record_for(node);
        *positive = r->positive;
        *negative = r->negative;
    } else {
        *positive = node & NODE_INLINE_MASK;
        *negative = (node >> NODE_INLINE_BITS) & NODE_INLINE_MASK;
    }
}

static int can_inline_fronts(uint32_t positive, uint32_t negative) {
#if MAX_SBB <= NODE_INLINE_MASK
    return !(positive & FRONT_VECTOR_TAG) && !(negative & FRONT_VECTOR_TAG) &&
           positive <= NODE_INLINE_MASK && negative <= NODE_INLINE_MASK;
#else
    return positive == 0 && negative == 0;
#endif
}

static uint32_t inline_fronts(uint32_t positive, uint32_t negative) {
    return positive | (negative << NODE_INLINE_BITS);
}

static void set_front_only(uint32_t *node, uint32_t positive, uint32_t negative) {
    uint32_t old = *node;
    if ((old & NODE_TAG_MASK) == NODE_BRANCH_TAG) {
        printf("\ninternal error replacing live cache branch with fronts\n");
        exit(19);
    }
    if (can_inline_fronts(positive, negative)) {
        if ((old & NODE_TAG_MASK) == NODE_RECORD_TAG) release_front_record(old);
        *node = inline_fronts(positive, negative);
    } else if ((old & NODE_TAG_MASK) == NODE_RECORD_TAG) {
        front_record *r = front_record_for(old);
        r->positive = positive;
        r->negative = negative;
    } else {
        *node = alloc_front_record(positive, negative);
    }
}

static int add_node_front(uint32_t *node, int positive, int sbb) {
    uint32_t p, q;
    node_fronts(*node, &p, &q);
    uint32_t opposite = positive ? q : p;
    if ((positive && front_has_lesser_equal(opposite, sbb)) ||
        (!positive && front_has_greater_equal(opposite, sbb))) {
        printf("\ncontradictory cache fronts while adding %s %s\n",
               positive ? "positive" : "negative", sbb_to_str[sbb]);
        exit(2);
    }
    uint32_t *field = positive ? &p : &q;
    int updated = add_front_descriptor(field, positive, sbb);
    if (!updated) return 0;
    if ((*node & NODE_TAG_MASK) == NODE_BRANCH_TAG) {
        branch_array *b = branch_for(*node);
        b->slot[0] = p;
        b->slot[1] = q;
    } else {
        set_front_only(node, p, q);
    }
    return 1;
}

int clamp_sbb(int sbb, int pairs_remaining, int n_remaining) {
    int sbb2 = min(sbb, max_sbb_for_pairs[min(MAX_PROD, pairs_remaining)]);
    while (sbb2 > 0 && sbb_to_n1[sbb2] + sbb_to_n2[sbb2] > n_remaining) sbb2--;
    return sbb2;
}

static branch_array *ensure_branch(uint32_t *node, uint32_t width) {
    if ((*node & NODE_TAG_MASK) == NODE_BRANCH_TAG) {
        branch_array *b = branch_for(*node);
        if (b->width != width) {
            printf("\ncache branch width mismatch: have %u, need %u\n", b->width, width);
            exit(11);
        }
        return b;
    }
    uint32_t positive, negative;
    node_fronts(*node, &positive, &negative);
    if ((*node & NODE_TAG_MASK) == NODE_RECORD_TAG) release_front_record(*node);
    *node = alloc_branch(width);
    branch_array *b = branch_for(*node);
    b->slot[0] = positive;
    b->slot[1] = negative;
    return b;
}

static void clear_node(uint32_t *node) {
    uint32_t descriptor = *node;
    uint32_t tag = descriptor & NODE_TAG_MASK;
    if (tag == NODE_BRANCH_TAG) {
        branch_array *b = branch_for(descriptor);
        for (uint32_t sbb = 2; sbb < b->width; sbb++) {
            if (b->slot[sbb] == 0) continue;
            clear_node(&b->slot[sbb]);
        }
        release_front_vector(b->slot[0]);
        release_front_vector(b->slot[1]);
        release_branch(descriptor);
    } else if (tag == NODE_RECORD_TAG) {
        front_record *r = front_record_for(descriptor);
        release_front_vector(r->positive);
        release_front_vector(r->negative);
        release_front_record(descriptor);
    }
    *node = 0;
}

static void fold_empty_branch(uint32_t *node) {
    if ((*node & NODE_TAG_MASK) != NODE_BRANCH_TAG) return;
    uint32_t descriptor = *node;
    branch_array *b = branch_for(descriptor);
    for (uint32_t sbb = 2; sbb < b->width; sbb++)
        if (b->slot[sbb] != 0) return;
    uint32_t positive = b->slot[0];
    uint32_t negative = b->slot[1];
    release_branch(descriptor);
    *node = 0;
    set_front_only(node, positive, negative);
}

int cacheCanSolve(uint32_t *node, int *sb, int size, int k, int max_sbb,
                  int pairs_remaining, int n_remaining) {
    if (size < 1) {
        printf("\nempty positive cache insertion\n");
        exit(3);
    }
    int updated = add_node_front(node, 1, *sb);
    if (size == 1) return updated;
    max_sbb = clamp_sbb(max_sbb, pairs_remaining, n_remaining);
    branch_array *b = ensure_branch(node, (uint32_t)max_sbb + 1);
    int minSbb = sb[1]; /* Unit Group Triviality Lemma. */
    int *lesser = sbb_lesser[*sb];
    while (*lesser >= minSbb) {
        int sbb2 = *lesser++;
        updated += cacheCanSolve(&b->slot[sbb2], sb + 1, size - 1, k, sbb2,
                                 pairs_remaining - sb_pairs[sbb2],
                                 n_remaining - sbb_to_n1[sbb2] - sbb_to_n2[sbb2]);
    }
    fold_empty_branch(node);
    return updated;
}

int cacheCantSolve(uint32_t *node, int *sb_orig, int size, int k, int max_sbb, int pairs,
                   int pairs_remaining, int n_remaining) {
    if (size < 1) {
        printf("\nempty negative cache insertion\n");
        exit(3);
    }
    max_sbb = clamp_sbb(max_sbb, pairs_remaining, n_remaining);
    if (size == 1) {
        int updated = add_node_front(node, 0, *sb_orig);
        if (updated && (*node & NODE_TAG_MASK) == NODE_BRANCH_TAG) {
            branch_array *b = branch_for(*node);
            int *greater = sbb_greater[*sb_orig];
            while (*greater <= max_sbb) {
                int sbb2 = *greater++;
                if (b->slot[sbb2] == 0) continue;
                clear_node(&b->slot[sbb2]);
            }
            fold_empty_branch(node);
        }
        return updated;
    }

    uint32_t positive_front, negative_front;
    node_fronts(*node, &positive_front, &negative_front);
    branch_array *b = NULL;
    int updated = 0;
    int sb[size];
    memcpy(sb, sb_orig, (size_t)size * sizeof(int));
    for (int i = 0; i < size; i++) {
        if (i > 0) {
            int tmp = sb[i];
            sb[i] = sb[0];
            sb[0] = tmp;
        }
        int sbb = *sb;
        int *greater = sbb_greater[sbb];
        int pairs_without_this = pairs - sb_pairs[sbb];
        int max_pairs = power3[k] - pairs_without_this;
        while (1) {
            int sbb2 = *greater++;
            if (sbb2 >= sb[1]) {
                if (sbb2 > max_sbb) break;
                int pairs_new = sb_pairs[sbb2];
                if (pairs_new > max_pairs) break;
                /* The front is checked before this edge on lookup.  Descendants below a part it
                   already refutes are unreachable, but other part permutations in this call may
                   still add information, so skip this edge rather than returning from the call. */
                if (front_has_lesser_equal(negative_front, sbb2)) continue;
                if (b == NULL) b = ensure_branch(node, (uint32_t)max_sbb + 1);
                int next_pairs = pairs_remaining - pairs_new;
                int next_n = n_remaining - sbb_to_n1[sbb2] - sbb_to_n2[sbb2];
                updated += cacheCantSolve(&b->slot[sbb2], sb + 1, size - 1, k, sbb2,
                                          pairs_without_this + pairs_new, next_pairs, next_n);
            }
        }
    }
    if (b != NULL) fold_empty_branch(node);
    return updated;
}

void cache(int *sb, int size, int canSolve, int k, int pairs) {
    long long alloc_count_before = alloc_count;
    long long alloc_size_before = alloc_size;
    int updated = canSolve
        ? cacheCanSolve(&sb_cache_root[k], sb, size, k, MAX_SBB, power3[k], MAX_N)
        : cacheCantSolve(&sb_cache_root[k], sb, size, k, MAX_SBB, pairs, power3[k], MAX_N);
    debug_printf(" cache=%lld/%lld(%+lld/%+lld) fronts=%lld/%lld",
                 alloc_count, alloc_size, alloc_count - alloc_count_before,
                 alloc_size - alloc_size_before, front_alloc_count, front_alloc_size);
#ifndef OPT_2
    if (updated == 0 && cache_replay_depth == 0) {
        printf("\nupdated == 0 when caching result sign=%d k=%d ", canSolve, k);
        printSb(sb, size);
        printf("\n");
        fflush(stdout);
        exit(4);
    }
#endif
    if (updated == 0 && cache_replay_depth != 0) redundant_cache_replays++;
}

static inline __attribute__((always_inline)) int checkCacheTrie_ctx(
    radio_search_context *ctx, int *sb, int size, int k) {
    uint32_t node = sb_cache_root[k];
    for (int i = 0; i < size; i++) {
        uint32_t tag = node & NODE_TAG_MASK;
        uint32_t positive = 0, negative;
        branch_array *b = NULL;
        if (tag == NODE_BRANCH_TAG) {
            b = branch_for(node);
            negative = b->slot[1];
            if (i == size - 1) positive = b->slot[0];
        } else if (tag == NODE_RECORD_TAG) {
            front_record *r = front_record_for(node);
            negative = r->negative;
            if (i == size - 1) positive = r->positive;
        } else {
            negative = (node >> NODE_INLINE_BITS) & NODE_INLINE_MASK;
            if (i == size - 1) positive = node & NODE_INLINE_MASK;
        }
        int sbb = sb[i];
        uint32_t source = 0;
        if (front_find_lesser_equal(negative, sbb, &source)) {
#ifdef RADIO_CACHE_CITATIONS
            radio_cache_citation_record(ctx, source);
#else
            (void)ctx;
#endif
            return FALSE;
        }
        if (i == size - 1 && front_has_greater_equal(positive, sbb)) return TRUE;
        if (b == NULL) return MAYBE;
        if ((uint32_t)sbb >= b->width) return MAYBE;
        node = b->slot[sbb];
    }
    return MAYBE;
}

static inline __attribute__((always_inline)) int checkCacheTrie(int *sb, int size, int k) {
    return checkCacheTrie_ctx(&radio_default_search_context, sb, size, k);
}

static inline __attribute__((always_inline)) uint32_t cache_l1_hash(const int *sb, int size,
                                                                    int k) {
    uint32_t h = UINT32_C(2166136261) ^ ((uint32_t)(unsigned)k << 24)
                 ^ (uint32_t)(unsigned)size;
    for (int i = 0; i < size; i++) {
        h ^= (uint32_t)sb[i];
        h *= UINT32_C(16777619);
    }
    return h;
}

static inline __attribute__((always_inline)) int cache_l1_probe(
    radio_search_context *ctx, const int *sb, int size, int k,
    cache_l1_entry **entry_out, uint32_t *hash_out) {
#ifdef RADIO_DISABLE_CACHE_L1
    (void)ctx;
    (void)sb;
    (void)size;
    (void)k;
    *entry_out = NULL;
    *hash_out = 0;
    return MAYBE;
#else
#ifdef MEASURE_CACHE_L1
    ctx->cache_l1_queries++;
#endif
    *entry_out = NULL;
    if ((unsigned)size <= CACHE_L1_MAX_PARTS) {
#ifdef MEASURE_CACHE_L1
        ctx->cache_l1_eligible++;
#endif
        uint32_t hash = cache_l1_hash(sb, size, k);
        cache_l1_entry *entries = radio_search_context_cache_l1(ctx);
        cache_l1_entry *entry = &entries[hash & (CACHE_L1_SIZE - 1u)];
        *entry_out = entry;
        *hash_out = hash;
        if (entry->verdict_plus_one != 0 && entry->hash == hash && entry->size == (uint8_t)size &&
            entry->k == (uint8_t)k) {
            int i = 0;
            while (i < size && entry->part[i] == (front_point)sb[i]) i++;
            if (i == size) {
#ifdef MEASURE_CACHE_L1
                ctx->cache_l1_hits++;
#endif
                return (int)entry->verdict_plus_one - 1;
            }
        }
    }
    return MAYBE;
#endif
}

static inline __attribute__((always_inline)) void cache_l1_store(
    radio_search_context *ctx, cache_l1_entry *entry, uint32_t hash,
    const int *sb, int size, int k, int verdict) {
#ifdef RADIO_DISABLE_CACHE_L1
    (void)ctx;
    (void)entry;
    (void)hash;
    (void)sb;
    (void)size;
    (void)k;
    (void)verdict;
#else
    (void)ctx;
    if (entry != NULL && verdict != MAYBE) {
#ifdef MEASURE_CACHE_L1
        ctx->cache_l1_stores++;
        ctx->cache_l1_replacements += entry->verdict_plus_one != 0;
#endif
        entry->hash = hash;
        for (int i = 0; i < size; i++) entry->part[i] = (front_point)sb[i];
        entry->size = (uint8_t)size;
        entry->k = (uint8_t)k;
        entry->verdict_plus_one = (uint8_t)(verdict + 1);
    }
#endif
}

/* Kept as the cache-query API for regression tools and small diagnostic drivers.  The main solver
   probes L1 separately so an exact hit can avoid its bundled-majorization checks as well. */
int checkCache(int *sb, int size, int k) {
    cache_l1_entry *entry;
    uint32_t hash = 0;
    radio_search_context *ctx = &radio_default_search_context;
    int verdict = cache_l1_probe(ctx, sb, size, k, &entry, &hash);
    if (verdict == MAYBE) verdict = checkCacheTrie(sb, size, k);
    cache_l1_store(ctx, entry, hash, sb, size, k, verdict);
    return verdict;
}

#ifdef DEBUG_CACHE_ONLY
#undef DEBUG
#undef debug_printf
#define debug_printf(...) /* Nothing */
#endif

// returns >0 if sbb1 is harder to solve than sbb2, <0 if sbb2 is harder, 0 if equal
int compare_solvability_ctx(radio_search_context *ctx, int sbb1, int sbb2) {
    if (sbb1==sbb2) return 0;
    int n11 = sbb_to_n1[sbb1];
    int n12 = sbb_to_n2[sbb1];
    int sum1 = n11 + n12;
    int n21 = sbb_to_n1[sbb2];
    int n22 = sbb_to_n2[sbb2];
    int sum2 = n21 + n22;
    if (sum1 >= sum2 && n12 >= n22) {
        return 1;
    } else if (sum1 <= sum2 && n12 <= n22) {
        return -1;
    } else {
        int mink1 = minK_ctx(ctx, sbb1);
        int mink2 = minK_ctx(ctx, sbb2);
        if (mink1 > mink2) {
            return 1;
        } else if (mink1 < mink2) {
            return -1;
        } else {
            // just use natural order for now
            return sbb1-sbb2;
            // use pair diff
//            return sb_pairs[sbb1] - sb_pairs[sbb2];
        }
    }
}

int compare_solvability(int sbb1, int sbb2) {
    return compare_solvability_ctx(&radio_default_search_context, sbb1, sbb2);
}

void init_singleton_majorization(void) {
    int current[1 << MAX_K];
    int next[1 << MAX_K];
    int i;
    int k;
    memset(current, 0, sizeof(current));
    current[0] = 1;
    singleton_base_len[0] = 1;
    singleton_base_prefix[0][0] = 1;
    for (k = 1; k <= MAX_K; k++) {
        int prev_len = singleton_base_len[k - 1];
        int len = prev_len * 2;
        memset(next, 0, len * sizeof(int));
        for (i = 0; i < prev_len; i++) {
            int h = current[i];
            // G_k = sort(L_k + M_k + R_k), using theorem recurrence.
            next[i] += h;
            next[i * 2] += h;
            next[i * 2 + 1] += h;
        }
        sort1(next, len);
        singleton_base_len[k] = len;
        for (i = 0; i < len; i++) {
            current[i] = next[i];
            singleton_base_prefix[k][i] = next[i] + (i > 0 ? singleton_base_prefix[k][i - 1] : 0);
        }
    }
}

int singleton_majorization_can_solve(int *sb, int size, int k) {
    long long left_prefix = 0;
    int right_len = singleton_base_len[k];
    int i;
    int lim = size < right_len ? size : right_len;
    for (i = 0; i < lim; i++) {
        int right_prefix = singleton_base_prefix[k][i];
        left_prefix += sbb_to_n1[sb[i]];
        if (left_prefix > right_prefix) {
            return FALSE;
        }
    }
    return TRUE;
}

/* Check the star lift after `by_n` has been sorted by descending long side.  Within one part,
   the left profile adds `copies` equal values n.  If g[] is the non-increasing singleton base,

       D(j)-D(j-1) = n-g[rank+j].

   Those increments are non-decreasing, so D is discrete convex and its maximum over the equal-n
   run is at an endpoint.  The beginning was checked by the preceding run (or is D(0)=0), hence
   only the final prefix is needed.  This is exactly the old expanded-prefix test, reduced from
   O(sum m_i) comparisons to O(number of parts). */
static int star_expansion_majorization_sorted(int *by_n, int size, int k) {
    int i;
    long long left_prefix = 0;
    int rank = 0;
    int right_len = singleton_base_len[k];
    int right_total = singleton_base_prefix[k][right_len - 1];
    for (i = 0; i < size; i++) {
        int copies = sbb_to_n2[by_n[i]];
        int n = sbb_to_n1[by_n[i]];
#ifdef RADIO_STAR_MAJOR_EXPANDED_PREFIX
        while (copies-- > 0) {
            left_prefix += n;
            {
                int right_prefix = rank < right_len
                    ? singleton_base_prefix[k][rank] : right_total;
                if (left_prefix > right_prefix) return FALSE;
            }
            rank++;
        }
#else
        left_prefix += (long long)copies * n;
        rank += copies;
        int right_prefix = rank <= right_len ? singleton_base_prefix[k][rank - 1] : right_total;
        if (left_prefix > right_prefix) return FALSE;
#endif
    }
    return TRUE;
}

static int star_expansion_majorization_with_buffer(int *sb, int size, int k, int *by_n) {
    int i;
    for (i = 0; i < size; i++) by_n[i] = sb[i];
    for (i = 1; i < size; i++) {
        int v = by_n[i], j = i - 1;
        while (j >= 0 && sbb_to_n1[by_n[j]] < sbb_to_n1[v]) {
            by_n[j + 1] = by_n[j];
            j--;
        }
        by_n[j + 1] = v;
    }
    return star_expansion_majorization_sorted(by_n, size, k);
}

static int star_expansion_majorization_large(int *sb, int size, int k) {
    int by_n[size];
    return star_expansion_majorization_with_buffer(sb, size, k, by_n);
}

/* Necessary condition for an arbitrary Sb state.  Replace each oriented part (n:m), n >= m,
   by m disjoint singleton stars (n:1).  This is a vertex-splitting lift of the original graph:
   pulling every test back to all clones preserves every edge transcript, so a strategy for the
   original would solve the lift.  The Singleton Majorization Theorem then decides the lift.

   Hot verifier children contain at most eight parts.  Keep their sort buffer fixed-size so the
   compiler does not emit variable-stack probing on every call; retain the general VLA path for
   the solver's uncommon longer states. */
int star_expansion_majorization_can_solve(int *sb, int size, int k) {
    enum { STAR_MAJOR_LOCAL_PARTS = 16 };
    if (size <= STAR_MAJOR_LOCAL_PARTS) {
        int by_n[STAR_MAJOR_LOCAL_PARTS];
        return star_expansion_majorization_with_buffer(sb, size, k, by_n);
    }
    return star_expansion_majorization_large(sb, size, k);
}

int get_max_sbb_ctx(radio_search_context *ctx, int n1, int n2, int n3, int n4) {
    int sbb1 = getSbb(n1, n2);
    int sbb2 = getSbb(n3, n4);
    return (compare_solvability_ctx(ctx, sbb1, sbb2) > 0) ? sbb1 : sbb2;
}

int get_max_sbb(int n1, int n2, int n3, int n4) {
    return get_max_sbb_ctx(&radio_default_search_context, n1, n2, n3, n4);
}

/* ---- Joint suffix reachability -------------------------------------------------------------
   A prefix can satisfy the counting bound on all three children so far and still be impossible to
   complete: the parts that remain simply cannot distribute their mass so that every child lands
   within 3^(k-1). Such a prefix currently proceeds to three cache probes that can only fail.

   R[i] = the set of (r0,r2) the remaining parts i..P-1 can contribute, as a bitmap; r1 follows from
   mass conservation. A prefix at (p0,p1,p2) is completable iff some reachable (r0,r2) satisfies
     r0 <= cap-p0,  r2 <= cap-p2,  r0+r2 >= p1 + Mrem - cap
   the last being the c1 bound. Answered in O(1) from a 2-D running max of r0+r2 over the box.

   Note the one-dimensional version of this is provably vacuous: taken alone each child can absorb
   the entire remaining mass, so per-child bounds never fire. It has to be joint.

   Built by bitmap convolution - R[i] = union over options of R[i+1] shifted by (k0,k2) - which is
   a few million word-ops for a whole state, ~1-2 ms. */
#define RB_MAXCAP 800
#ifndef RB_TRIGGER
#define RB_TRIGGER 10000000LL      /* arm the prune once a state has cost this many candidates */
#endif
#ifdef RADIO_RB_PROFILE_DIAGNOSTIC
#ifndef RADIO_RB_PLIABILITY_DIAGNOSTIC
#define RADIO_RB_PLIABILITY_DIAGNOSTIC
#endif
#endif
struct radio_reachability_state {
    int on;
    int cap;
    int words;
    int parts;
    unsigned long long *bits[17];
    short *mx[17];
    int remaining_mass[17];
    long long tested;
    long long pruned;
#ifdef RADIO_RB_PLIABLE_CUTOFF
/* Lab-only comparison: pay an exact post-build scan, then omit lookups that it proves vacuous.
   The default solver deliberately leaves this off; the measured lookup savings did not reduce
   CPU time on the hard positive control. */
    unsigned char pliable[17];
#ifdef RADIO_RB_PROFILE_DIAGNOSTIC
    long long pliable_skipped;
#endif
#endif
#ifdef RADIO_RB_PROFILE_DIAGNOSTIC
    unsigned long long suffix_tested[17];
    unsigned long long suffix_pruned[17];
    int profile_parts[17];
#endif
};

static radio_reachability_state *radio_search_context_reachability(
    radio_search_context *ctx) {
    if (ctx->reachability == NULL) {
        ctx->reachability = (radio_reachability_state *)calloc(1, sizeof(*ctx->reachability));
        if (ctx->reachability == NULL) {
            printf("\nout of memory allocating worker reachability state\n");
            exit(1);
        }
    }
    return ctx->reachability;
}

static void rb_free(radio_reachability_state *rb) {
    int i;
    for (i = 0; i <= rb->parts; i++) {
        free(rb->bits[i]);
        free(rb->mx[i]);
        rb->bits[i] = NULL;
        rb->mx[i] = NULL;
    }
}

void radio_search_context_destroy(radio_search_context *ctx) {
    if (ctx == &radio_default_search_context) return;
    if (ctx->reachability != NULL) {
        rb_free(ctx->reachability);
        free(ctx->reachability);
    }
    free(ctx->cache_l1);
    radio_search_context_init(ctx);
}

static void rb_build(radio_reachability_state *rb, splits **sa, int *tmpp, int P, int cap) {
    int i, c, r0, w, W = cap + 1;
    rb->cap = cap;
    rb->parts = P;
    rb->words = (W + 63) / 64;
#ifdef RADIO_RB_PROFILE_DIAGNOSTIC
    memset(rb->suffix_tested, 0, sizeof(rb->suffix_tested));
    memset(rb->suffix_pruned, 0, sizeof(rb->suffix_pruned));
    memcpy(rb->profile_parts, tmpp, (size_t)P * sizeof(*tmpp));
#endif
    rb->remaining_mass[P] = 0;
    for (i = P - 1; i >= 0; i--)
        rb->remaining_mass[i] = rb->remaining_mass[i + 1] + sb_pairs[tmpp[i]];
    for (i = 0; i <= P; i++) {
        rb->bits[i] = (unsigned long long *)calloc(
            (size_t)W * rb->words, sizeof(unsigned long long));
        rb->mx[i] = (short *)malloc((size_t)W * W * sizeof(short));
    }
    rb->bits[P][0] = 1ULL;                                 /* (0,0) reachable by the empty suffix */
    for (i = P - 1; i >= 0; i--) {
        for (c = 0; c < sa[i]->size; c++) {
            int *sp = sa[i]->splitsl[c];
            int k0 = sb_pairs[sp[0]], k2 = sb_pairs[sp[3]];
            if (k0 > cap || k2 > cap) continue;
            int ws = k2 >> 6, bs = k2 & 63;                /* shift along r2, within each row */
            for (r0 = 0; r0 + k0 <= cap; r0++) {
                unsigned long long *src = rb->bits[i + 1] + (size_t)r0 * rb->words;
                unsigned long long *dst = rb->bits[i] + (size_t)(r0 + k0) * rb->words;
                for (w = rb->words - 1; w >= 0; w--) {
                    int sw = w - ws;
                    if (sw < 0) break;
                    unsigned long long v = src[sw] << bs;
                    if (bs && sw > 0) v |= src[sw-1] >> (64 - bs);
                    dst[w] |= v;
                }
            }
        }
    }
    for (i = 0; i <= P; i++) {
        int r2;
        for (r0 = 0; r0 <= cap; r0++) for (r2 = 0; r2 <= cap; r2++) {
            unsigned long long bit = rb->bits[i][(size_t)r0 * rb->words + (r2 >> 6)]
                >> (r2 & 63) & 1ULL;
            short b = bit ? (short)(r0 + r2) : -1;
            if (r0 && rb->mx[i][(size_t)(r0 - 1) * W + r2] > b)
                b = rb->mx[i][(size_t)(r0 - 1) * W + r2];
            if (r2 && rb->mx[i][(size_t)r0 * W + r2 - 1] > b)
                b = rb->mx[i][(size_t)r0 * W + r2 - 1];
            rb->mx[i][(size_t)r0 * W + r2] = b;
        }
    }
}
static void rb_release_mode(radio_reachability_state *rb, int quiet) {
#ifdef RADIO_RB_PROFILE_DIAGNOSTIC
    int i;
    for (i = 1; i < rb->parts; i++) {
        int q = rb->parts - i;
        int mass = rb->remaining_mass[i];
        int excess = mass - 2 * q;
        fprintf(stderr,
                "RB_PROFILE_SUFFIX index=%d parts=%d mass=%d excess=%d calls=%llu pruned=%llu\n",
                i, q, mass, excess,
                rb->suffix_tested[i], rb->suffix_pruned[i]);
    }
    fprintf(stderr, "RB_PROFILE_END tested=%lld pruned=%lld", rb->tested, rb->pruned);
#if defined(RADIO_RB_PLIABLE_CUTOFF) && defined(RADIO_RB_PROFILE_DIAGNOSTIC)
    fprintf(stderr, " pliable_skipped=%lld", rb->pliable_skipped);
#endif
    fputc('\n', stderr);
#endif
    /* A frozen verifier may arm this per root across millions of independent claims.  Its
       batch-level progress already reports aggregate work, so the solver's per-state diagnostic
       would turn stderr into a multi-gigabyte log and materially perturb the benchmark. */
    if (!quiet) {
#if defined(RADIO_RB_PLIABLE_CUTOFF) && defined(RADIO_RB_PROFILE_DIAGNOSTIC)
        fprintf(stderr, "\nREACH: %lld tested, %lld pruned (%.1f%%), %lld pliable-skipped\n",
                rb->tested, rb->pruned, rb->tested ? 100.0 * rb->pruned / rb->tested : 0.0,
                rb->pliable_skipped);
#else
        fprintf(stderr, "\nREACH: %lld tested, %lld pruned (%.1f%%)\n",
                rb->tested, rb->pruned, rb->tested ? 100.0 * rb->pruned / rb->tested : 0.0);
#endif
    }
    rb_free(rb);
    rb->on = 0;
    rb->tested = rb->pruned = 0;
#if defined(RADIO_RB_PLIABLE_CUTOFF) && defined(RADIO_RB_PROFILE_DIAGNOSTIC)
    rb->pliable_skipped = 0;
#endif
}
#define rb_release(rb) rb_release_mode((rb), FALSE)
static inline int rb_dead(radio_reachability_state *rb, int nexti, int p0, int p1, int p2) {
    int W = rb->cap + 1;
    int a = rb->cap - p0, cc = rb->cap - p2;
    long long need = (long long)p1 + rb->remaining_mass[nexti] - rb->cap;
    rb->tested++;
#ifdef RADIO_RB_PROFILE_DIAGNOSTIC
    rb->suffix_tested[nexti]++;
#endif
    if (a < 0 || cc < 0) {
        rb->pruned++;
#ifdef RADIO_RB_PROFILE_DIAGNOSTIC
        rb->suffix_pruned[nexti]++;
#endif
        return 1;
    }
    if (rb->mx[nexti][(size_t)a * W + cc] >= need) return 0;
    rb->pruned++;
#ifdef RADIO_RB_PROFILE_DIAGNOSTIC
    rb->suffix_pruned[nexti]++;
#endif
    return 1;
}

#if defined(RADIO_RB_PLIABILITY_DIAGNOSTIC) || defined(RADIO_RB_PLIABLE_CUTOFF)
/* A suffix is sigma-pliable when it can fit every residual capacity triple h with

       0 <= hj <= cap,       h0 + h1 + h2 = suffix_mass + sigma.

   This is stronger than merely passing rb_dead at one reachable prefix.  If suffix i and every
   later suffix are pliable, rb_dead cannot reject after the search has assigned i parts.  The
   existing prefix-max table decides the definition exactly: among contributions with r0 <= h0 and
   r2 <= h2, some choice also has r1 <= h1 iff max(r0+r2) >= suffix_mass-h1. */
static int rb_suffix_pliable(radio_reachability_state *rb, int suffix, int slack) {
    int side = rb->cap + 1;
    int target = rb->remaining_mass[suffix] + slack;
    int h0;

    for (h0 = 0; h0 <= rb->cap; h0++) {
        int h2_lo = max(0, target - h0 - rb->cap);
        int h2_hi = min(rb->cap, target - h0);
        int h2;
        for (h2 = h2_lo; h2 <= h2_hi; h2++) {
            int h1 = target - h0 - h2;
            int best = rb->mx[suffix][(size_t)h0 * side + h2];
            int need = rb->remaining_mass[suffix] - h1;
            if (best < 0 || best < need) return FALSE;
        }
    }
    return TRUE;
}
#endif

#ifdef RADIO_RB_PLIABILITY_DIAGNOSTIC
static int rb_has_mass_vector(const splits *table, int want0, int want1, int want2) {
    int c;
    for (c = 0; c < table->size; c++) {
        const int *sp = table->splitsl[c];
        int k0 = sb_pairs[sp[0]];
        int k1 = sb_pairs[sp[1]] + sb_pairs[sp[2]];
        int k2 = sb_pairs[sp[3]];
        if (k0 == want0 && k1 == want1 && k2 == want2) return TRUE;
    }
    return FALSE;
}

static int rb_has_pure_corners(const splits *table, int mass) {
    return rb_has_mass_vector(table, mass, 0, 0)
        && rb_has_mass_vector(table, 0, mass, 0)
        && rb_has_mass_vector(table, 0, 0, mass);
}

/* The special base of the extension theorem.  A retained (2:1) table has these five mass vectors;
   at slack >= 1 they make one copy, and hence a tail of copies, universally pliable. */
static int rb_has_two_one_base(const splits *table, int mass, int slack) {
    return mass == 2 && slack >= 1
        && rb_has_mass_vector(table, 2, 0, 0)
        && rb_has_mass_vector(table, 0, 2, 0)
        && rb_has_mass_vector(table, 0, 0, 2)
        && rb_has_mass_vector(table, 1, 1, 0)
        && rb_has_mass_vector(table, 0, 1, 1);
}

/* Report only; this never changes rb_dead, split ordering, or the adaptive arming policy.

   `theorem` is the direct extension certificate.  If a tail of mass T is pliable and the next part
   of mass w retains all three pure corners, adding that part preserves pliability whenever

       2*w <= T + slack + 2.

   `coarse` is the simpler nonunit-length corollary.  For q parts of total mass W, let D=W-2q.
   Starting from a (2:1) base, q>=D+2 at slack 1 or q>=D+1 at slack >=2 certifies the whole hereditary
   tail (subject to the same retained-corner check). */
static void rb_report_pliability(radio_reachability_state *rb, splits **tables,
                                 const int *parts, FILE *out, int verbose) {
    unsigned char exact[17] = {0};
    unsigned char hereditary[17] = {0};
    unsigned char theorem[17] = {0};
    unsigned char coarse[17] = {0};
    unsigned char slack_excess[17] = {0};
    unsigned char corners[17] = {0};
    unsigned char all_corners[17] = {0};
    unsigned char all_nonempty[17] = {0};
    int slack = 3 * rb->cap - rb->remaining_mass[0];
    int sorted_by_mass = TRUE;
    int exact_head = rb->parts;
    int theorem_head = rb->parts;
    int coarse_head = rb->parts;
    int slack_excess_head = rb->parts;
    int potential_call_suffixes = 0;
    int i;

    for (i = 0; i < rb->parts - 1; i++) {
        if (sb_pairs[parts[i]] < sb_pairs[parts[i + 1]]) sorted_by_mass = FALSE;
    }
    for (i = 0; i <= rb->parts; i++) exact[i] = rb_suffix_pliable(rb, i, slack);
    hereditary[rb->parts] = exact[rb->parts];
    theorem[rb->parts] = TRUE;
    coarse[rb->parts] = TRUE;
    slack_excess[rb->parts] = TRUE;
    all_corners[rb->parts] = TRUE;
    all_nonempty[rb->parts] = TRUE;

    for (i = rb->parts - 1; i >= 0; i--) {
        int mass = sb_pairs[parts[i]];
        int tail_mass = rb->remaining_mass[i + 1];
        int extension_ok;
        int special_base;
        int q = rb->parts - i;
        int excess = rb->remaining_mass[i] - 2 * q;
        int coarse_bound = FALSE;
        int slack_excess_bound = FALSE;

        corners[i] = rb_has_pure_corners(tables[i], mass);
        all_corners[i] = corners[i] && all_corners[i + 1];
        all_nonempty[i] = tables[i]->size > 0 && all_nonempty[i + 1];
        hereditary[i] = exact[i] && hereditary[i + 1];
        extension_ok = theorem[i + 1] && corners[i]
            && 2LL * mass <= (long long)tail_mass + slack + 2;
        special_base = i == rb->parts - 1 && rb_has_two_one_base(tables[i], mass, slack);
        /* If slack >= 2*cap, every residual capacity is at least the entire suffix mass: the
           other two capacities can consume at most 2*cap.  Then any retained routing fits. */
        theorem[i] = (slack >= 2 * rb->cap && all_nonempty[i]) || extension_ok || special_base;

        if (sorted_by_mass && all_corners[i]
            && rb_has_two_one_base(tables[rb->parts - 1],
                                   sb_pairs[parts[rb->parts - 1]], slack)) {
            if (q == 1) coarse_bound = TRUE;
            else if (slack == 1 && q >= excess + 2) coarse_bound = TRUE;
            else if (slack >= 2 && q >= excess + 1) coarse_bound = TRUE;

            /* Preserve the full absolute slack instead of collapsing it to slack>=2.  Writing
               each mass as 2+d_j, with nondecreasing d_j from the (2:1) base outwards, the same
               extension proof works whenever

                   2 * (D-q) <= slack-4,

               where D=sum(d_j)=suffix_mass-2q.  This specializes to q>=D+2 at slack one and
               q>=D+1 at slack two or three, then admits progressively more excess as slack grows. */
            if (q == 1 || (slack >= 1
                           && 2LL * (excess - q) <= (long long)slack - 4))
                slack_excess_bound = TRUE;
        }
        coarse[i] = coarse_bound;
        slack_excess[i] = slack_excess_bound;

        if (theorem[i] && !hereditary[i]) {
            fprintf(stderr, "RB_PLIABILITY internal-error theorem-not-exact suffix=%d\n", i);
            exit(4);
        }
        if (coarse[i] && !theorem[i]) {
            fprintf(stderr, "RB_PLIABILITY internal-error coarse-not-direct suffix=%d\n", i);
            exit(4);
        }
        if (slack_excess[i] && !theorem[i]) {
            fprintf(stderr, "RB_PLIABILITY internal-error slack-excess-not-direct suffix=%d\n", i);
            exit(4);
        }
    }

    for (i = 0; i <= rb->parts; i++) {
        if (hereditary[i]) { exact_head = i; break; }
    }
    for (i = 0; i <= rb->parts; i++) {
        if (theorem[i]) { theorem_head = i; break; }
    }
    for (i = 0; i <= rb->parts; i++) {
        if (coarse[i]) { coarse_head = i; break; }
    }
    for (i = 0; i <= rb->parts; i++) {
        if (slack_excess[i]) { slack_excess_head = i; break; }
    }
    for (i = 1; i < rb->parts; i++) {
        if (!exact[i]) potential_call_suffixes++;
    }

    fprintf(out,
            "RB_PLIABILITY parts=%d mass=%d cap=%d slack=%d root_pliable=%d "
            "potential_call_suffixes=%d exact_head=%d exact_tail=%d "
            "theorem_head=%d theorem_tail=%d coarse_head=%d coarse_tail=%d "
            "slack_excess_head=%d slack_excess_tail=%d\n",
            rb->parts, rb->remaining_mass[0], rb->cap, slack, exact[0],
            potential_call_suffixes, exact_head, rb->parts - exact_head,
            theorem_head, rb->parts - theorem_head, coarse_head,
            rb->parts - coarse_head, slack_excess_head,
            rb->parts - slack_excess_head);

    if (verbose) {
        for (i = 0; i < rb->parts; i++) {
            int mass = sb_pairs[parts[i]];
            int margin = rb->remaining_mass[i + 1] + slack + 2 - 2 * mass;
            fprintf(out,
                    "RB_PLIABILITY_SUFFIX index=%d parts=%d mass=%d exact=%d hereditary=%d "
                    "theorem=%d coarse=%d slack_excess=%d corners=%d extension_margin=%d\n",
                    i, rb->parts - i, rb->remaining_mass[i], exact[i], hereditary[i],
                    theorem[i], coarse[i],
                    slack_excess[i], corners[i], margin);
        }
    }
}
#endif

#ifdef RADIO_RB_PROFILE_DIAGNOSTIC
static void rb_profile_begin(radio_reachability_state *rb, splits **tables,
                             const int *parts, int k) {
    int i;
    fprintf(stderr, "RB_PROFILE_BEGIN k=%d cap=%d parts=%d mass=%d slack=%d state=Sb(",
            k, rb->cap, rb->parts, rb->remaining_mass[0],
            3 * rb->cap - rb->remaining_mass[0]);
    for (i = 0; i < rb->parts; i++) {
        if (i) fputc(',', stderr);
        fputs(sbb_to_str[rb->profile_parts[i]], stderr);
    }
    fprintf(stderr, ")\n");
    rb_report_pliability(rb, tables, parts, stderr, TRUE);
}
#endif

int canSolveB_ctx(radio_search_context *ctx, int *sb, int size, int k,
                  uint64_t parent_deadline){
    int frozen_refute = parent_deadline == FROZEN_REFUTE;
#ifdef DEBUG1
    if(k>7) {
        printf("in canSolveB k=%d ", k);
        printSb(sb, size);
        printf("\n");
        fflush(stdout);
    }
#endif
    int canSolve=FALSE;
    int tmp[size];
    int singletons[size];
    //todo: replace with memcpy
    int i;
    int pairs=0;
    int pairs_full=0;
    int newsize=0;
    int singleton_size=0;
    int sbb;
    for(i=0;i<size;i++) {
        sbb=sb[i];
        pairs_full+=sb_pairs[sbb];
        // Unit Group Trivilaity Lemma: Unit Groups (1-1) do not affect solvability within information maximum and can be ignored
        if (sbb>1) {
            tmp[newsize++]=sbb;
            pairs+=sb_pairs[sbb];
            if (sbb_to_n2[sbb] == 1) {
                singletons[singleton_size++] = sbb;
            }
        }
    }
    
    //    printf("in canSolveB in %d ", k);
    //    printSb(tmp, newsize);
    //    printf("\n");
    //    printf("pairs=%d\n", pairs);
    

    // check pairs
    if (pairs_full>power3[k]) return FALSE;
    if (newsize == 0) return TRUE; // if we only had unit groups
    
    size = newsize;
    int shared_probe = size <= 2;
    if (size>1) sort1(tmp, size);
    int query_size = size;
    cache_l1_entry *l1_entry = NULL;
    uint32_t l1_hash = 0;
    int ck = MAYBE;
    /* The level-k claim being audited is present in the frozen trie.  Looking it up here would
       prove it by circular citation, so only ordinary solver calls probe the root cache. */
    if (!frozen_refute) {
        ck = cache_l1_probe(ctx, tmp, size, k, &l1_entry, &l1_hash);
        if (ck == TRUE || ck == FALSE) return ck;
    }
    if (singleton_size == size) {
        // Singleton states are decided exactly by majorization against G_k.
        ck = singleton_majorization_can_solve(tmp, size, k);
        cache_l1_store(ctx, l1_entry, l1_hash, tmp, size, k, ck);
        return ck;
    }
    // Apply Singleton Majorization to the full star expansion: (n:m) becomes m copies of (n:1).
    // This strictly dominates the old one-copy downgrade, because it contains that downgraded
    // singleton sequence and adds only nonnegative entries.  It is a necessary condition, not an
    // ordering heuristic; see docs/theorems/singleton-majorization.md.
    if (!star_expansion_majorization_can_solve(tmp, size, k)) {
        cache_l1_store(ctx, l1_entry, l1_hash, tmp, size, k, FALSE);
        return FALSE;
    }
#ifdef RADIO_EXTERNAL_EXACT_LOOKUP
    /* Research drivers may carry a compact exact-fact index beside the dominance trie.  This is
       deliberately a compile-time hook: the production engine has no extra lookup or trust
       boundary.  A hook must return MAYBE on absence and may only return definitive archived
       facts for full canonical state equality.  Keep it after independent exact/necessary
       theorem checks so an external research artifact can never override them. */
    if (!frozen_refute) {
        ck = RADIO_EXTERNAL_EXACT_LOOKUP(tmp, size, k);
        if (ck == TRUE || ck == FALSE) {
            cache_l1_store(ctx, l1_entry, l1_hash, tmp, size, k, ck);
            return ck;
        }
    }
#endif
    //check cache
    if (!frozen_refute) {
        ck = checkCacheTrie_ctx(ctx, tmp, size, k);
        cache_l1_store(ctx, l1_entry, l1_hash, tmp, size, k, ck);
    }
    //	printf("got from cache %d\n", ck);
    if (parent_deadline == CACHE_ONLY || ck == TRUE || ck == FALSE) {
//        debug_printf("returning ck=%d\n", ck);
        return ck;
    }
    /* radio_effort_now_ctx(ctx, 0): in radius mode this is always 0, so the check below never
       fires here -- an entry-time "has the shared budget already been spent by a sibling" bail is
       a work-currency-specific optimization (radius isn't shared across siblings the way work-time
       is), not a mathematical necessity this call must reproduce. */
    if (!frozen_refute && parent_deadline != NO_DEADLINE && parent_deadline != FAST_ONLY
        && deadline_expired(parent_deadline, radio_effort_now_ctx(ctx, 0)))
        return MAYBE;
    
    int size_1 = size-1;
    int size2 = size*2;
    int k_1 = k-1;
    int max_pairs_1 = power3[k_1];
    int sb0[size],sb2[size],sb1[size2];
    int cont;
    long long totalsplits;
    int skiptop;
    int splitincr[size];
    int *ordp[size];
    int ords[size];
    signed char ordmono[size];
    int splitindex[size];
    splits *splitsarr[size];
    int spi, spi2;
    
    int sb0p[size],sb2p[size],sb1p[size];
    
    memset(splitsarr, 0, sizeof(splitsarr));
    /* Search is depth first.  Building every suffix table here made a large-k state pay for
       many parts it never reached.  Materialise the first table now and each suffix only when
       the prefix survives far enough to enter it. */
    splitsarr[0] = frozen_refute
        ? ensure_splits(tmp[0], k)
        : prepare_splits_ctx(ctx, tmp[0], k, size > 1);
    //full search
    clock_t cpu_start = frozen_refute ? 0 : clock();
    clock_t progress = cpu_start + PROGRESS_INTERVAL;
    uint64_t budget_start = radio_budget_now_ctx(ctx);
    radio_reachability_state *rb = radio_search_context_reachability(ctx);
    
    /* Reachability is armed by observed cost, not by a shape signature. A state that has already
       burned RB_TRIGGER candidate evaluations has earned the ~1-2 ms the tables take to build, and
       cheap states - the overwhelming majority - never pay anything. That also avoids privileging
       the 8-part near-saturated shape: any state that turns out expensive gets the prune. */
    int rb_here = 0;
    /* rb_dead uses the unassigned suffix, so a rejection proves only that the FULL state cannot
       use this prefix.  The old implicit-contraction argument is prefix-local: if no assignment
       reaches part q, the first q parts alone are impossible.  Once rb_dead has rejected anything,
       that argument is no longer available for this invocation; retain the exact full negative but
       never cache the tempting shorter one.  Keep this local across iterative-deepening passes. */
    int rb_tainted_contraction = 0;
    uint64_t deadline = frozen_refute
        ? UINT64_MAX : search_deadline(ctx, parent_deadline, budget_start, size);
    
    //    printf("k=%d parent_budget=%llu start=%llu limit=%llu\n",
    //           k, parent_deadline, budget_start, deadline);
    
    int cont2=1;
    int skipped_some;
    /* Radius mode caps each level's own candidate range at radius_N (see below) instead of always
       exploring its full size -- set the moment any level actually gets truncated below its true
       size. Must gate every path that would otherwise conclude a clean FALSE: a truncated search
       proves nothing about the candidates it never looked at, so it can only ever be MAYBE or a
       genuine TRUE (a witness found within the cap is unconditionally valid regardless of what was
       skipped elsewhere). */
    int radius_truncated;
    /* radius_grows: this call was asked (via NO_DEADLINE) to progressively widen its own radius
       until it gets a definitive answer, and to tell its children to do the same -- radius mode's
       analogue of the work-budget clock's own NO_DEADLINE/probe_seconds-doubling iterative
       deepening. radius_N is the actual per-segment cap used below (see splitindex[] capping):
       starts small and doubles on every unresolved exhaustive pass when radius_grows, or is just
       the caller's fixed cap otherwise (bounded probe, one pass, MAYBE is an acceptable answer --
       the intermediate-level case). Kept separate from `deadline` itself, which stays NO_DEADLINE
       (2) for the whole call so no_deadline/search_deadline/probe_child_deadline's own sentinel
       checks keep working unmodified. */
    int radius_grows = ctx->radius_mode && parent_deadline == NO_DEADLINE;
    uint64_t radius_N = radius_grows ? 4 : deadline;
    /* Start the frozen audit at pass 2: there is no point running the solver's deliberately
       incomplete FAST witness pass before an exhaustive refutation-only traversal. */
    int pass = frozen_refute ? 1 : 0;
    unsigned int probe_seconds = RADIO_INITIAL_PROBE_SECONDS(k, size, parent_deadline);
    if (probe_seconds == 0) probe_seconds = PROBE_SECONDS;
    int max_solvable_maybe = 0;
    int fast_solve;

    while (cont2) {
        pass++;
        // Pass 1 restricts groups 0..size-2 to FAST splits, leaving the LAST group free.
        // The point is to descend across all groups quickly rather than exhaust one wrong
        // split at the first sbb; at the last group the counting bound is decisive anyway
        // and there is no descent below it to protect.
        //
        // FAST tests a local optimum in the m2 direction of the harder mixed child, i.e. the
        // line a*m = n*b where the two mixed children balance - and its `m2==0 ||` /
        // `m2==n2 ||` clauses auto-pass at the edges, so it admits the diagonal AND the
        // corners. Measured 2026-08-04 over 2,126 winning part-splits from all 15 witness
        // trees: every winner lies within 2.19 of that line, 96.5% within 1.0.
        //
        // Removed 2026-08-03 on a 3.84x ladder measurement and restored 2026-08-04: that
        // ladder is refutation-dominated, and without this the search sank into single
        // prefixes on solvable states (Sb(112:80) in 9 spent 43 minutes on one k=5 node,
        // clearing 1 of its 52 splits). Do not remove it on a refutation-only benchmark.
        // FAST_MIN_K skips pass 1 below a level. Default 0 = unconditional, and LEAVE IT THERE.
        //
        // 2026-08-05: I gated it at 7 after measuring the share of verdicts pass 1 resolved - 100%
        // at k=8, 26.3% at k=7, then 0.5%, 0.4%, 0.2% at k=6, 5, 4 - and reported 4.3x. That
        // reasoning was wrong. FAST's value is not the verdicts it closes. A split is three-way and
        // only one child needs refuting; the expensive mistake is grinding toward a refutation on a
        // child that is actually solvable, exhausting it, and finding a solution at the end anyway.
        // FAST exhibits one witness cheaply on those children so exhaustion is spent where it is the
        // only option. A pass-1 success PREVENTS work and prints no verdict, so counting verdicts is
        // blind to what it does - across 356,433 verdicts of that run, pass 1 printed no positive at
        // any level, which measures the instrument and not the mechanism.
        //
        // The gated run showed it: 1,258 k=8 verdicts at 10:21, still exactly 1,258 at 45:00, and
        // the solvable control Sb(112:80) in 9 never concluded. That is sinking into a solvable
        // branch - the same failure the 2026-08-04 restoration was for.
        //
        // Correctness is not at stake either way (pass 2 is exhaustive). Throughput is, in the
        // opposite direction to the one I measured. If pass 1 is to be improved, improve its hit
        // rate on solvable children; do not do less of it.
#ifndef FAST_MIN_K
#define FAST_MIN_K 0
#endif
        fast_solve = FALSE;
        if (pass==1) {
            fast_solve = !frozen_refute && size > 2 && k >= FAST_MIN_K;
        }
        
        // Every child call is bounded, including children of a NO_DEADLINE proof.  An unresolved
        // exhaustive pass doubles probe_seconds and starts over, so this is iterative deepening
        // rather than a same-budget retry.  Finite calls stop at their shared absolute limit.
        int no_deadline = parent_deadline == NO_DEADLINE || frozen_refute;
        
//        int no_deadline = (pass==1);
        
        skipped_some = 0;
        radius_truncated = 0;
        totalsplits=0;
        skiptop = 0;
        cont=1;
        /* Radius mode keeps this heuristic exactly as-is (no longer overridden to force BY_MAGIC3
           everywhere): it is already a decent, segment-count/index-conditioned baseline, and the
           2026-08-23 direction-consistency argument for forcing a single order was premature --
           iterate on ordering separately, against this as the baseline, rather than discarding it. */
        splitincr[0] = size<=3 ? BY_SP1 : BY_MAGIC3;
//        splitincr[0] = size == 1 ? BY_MAGIC : (size<=3 ? BY_SP1 : BY_MAGIC3);
//        splitincr[0] = size == 1 ? BY_MAGIC : (size<=3 ? BY_SP2_DESC : ((sb_pairs[tmp[0]] < pairs / 3) ? BY_MAGIC3 : BY_MAX));
//        splitincr[0] = size == 1 ? BY_MAGIC : (size<=3 ? BY_MAGIC2 : ((sb_pairs[tmp[0]] < pairs / 3) ? BY_MAGIC3 : BY_MAX));
//        splitincr[0] = size == 1 ? BY_SP2_DESC : (size<=3 ? BY_MAGIC2 : ((sb_pairs[tmp[0]] < pairs / 3) ? BY_MAGIC3 : BY_MAX));
//        splitincr[0] = size == 1 ? BY_SP1 : (size<=3 ? BY_MAGIC2 : ((sb_pairs[tmp[0]] < pairs / 3) ? BY_MAGIC3 : BY_MAX));
//        splitincr[0] = size == 1 ? (DESC + BY_SP0) : (size<=3 ? BY_MAGIC2 : ((sb_pairs[tmp[0]] < pairs / 3) ? BY_MAGIC3 : BY_MAX));
        //    splitincr[0] = size == 1 ? BY_MAGIC : BY_MAX;
        //    splitincr[0] = size == 1 ? BY_MAGIC : ( (size>2 && sb_pairs[0] < pairs / 2) ? DESC + BY_MIN : BY_MAX);
        
        
        memset(splitindex, 0, size * sizeof(int));
        if (ctx->radius_mode) {
            int full0 = splitsarr[0]->size;
            int capped0 = radius_N < (uint64_t)full0 ? (int)radius_N : full0;
            splitindex[0] = capped0;
            if (capped0 < full0) radius_truncated = 1;
        } else {
            splitindex[0] = splitsarr[0]->size;
        }
        HOIST_ORDER(0);
        
#ifdef DEBUG1
        clock_t t = clock();
        printf("solving in %d pass=%d ", k, pass);
#ifdef RADIO_WORK_BUDGET
        printf("budget=%llu ",
               (unsigned long long)(deadline > radio_budget_now_ctx(ctx)
                   ? deadline - radio_budget_now_ctx(ctx) : 0));
#else
        printf("deadline=%llu ",
               (unsigned long long)(deadline > (uint64_t)t
                   ? (deadline - (uint64_t)t + CLOCKS_PER_SEC) / CLOCKS_PER_SEC : 0));
#endif
        printSb(tmp, size);
        printf("\n");
#ifdef DEBUG
        int ii2;
        for (ii2=0; ii2<size; ii2++) {
            printf("splitindex[%d]=%d\n", ii2, splitindex[ii2]);
        }
#endif
        fflush(stdout);
#endif
        
        
        //    fflush(stdout);
        
        i = 0;
        sb1[0] = -1; // to prevent skipping first due to skiptop
        int ck0,ck1,ck2;
        
        while(cont) {
            while(splitindex[i] == 0) {
                //                if (splitindex[i] > 0) skipped_some = 1;
                if (i==0) {
                    // can't solve
#ifdef MEASURE_FAST_REPLAY
                    if (fast_replay_capture && k == 5 && parent_deadline == NO_DEADLINE && pass == 1) {
                        fast_replay_first_splits = totalsplits;
                        fast_replay_first_depth = max_solvable_maybe;
                    }
#endif
                    cont=0;
                    if (!skipped_some && !radius_truncated) {
                        cont2=0; // really can't solve
//                    } else if (parent_deadline==FAST_ONLY) {
//                        cache(tmp, size, MAYBE_SLOW, k, pairs);
//                        return MAYBE;
                    } else {
                        /* Radius mode's own NO_DEADLINE growth (radius_N doubling, below) is
                           separate from this work-budget doubling and must never touch `deadline`
                           itself: deadline stays the NO_DEADLINE sentinel (2) for the whole call in
                           that mode, so budget_start (a work-clock value, unrelated to radius
                           currency) would underflow the subtraction and, after radio_budget_add's
                           overflow clamp, corrupt deadline to UINT64_MAX -- silently breaking the
                           child's own no_deadline recognition (UINT64_MAX != NO_DEADLINE) at the cd
                           computation below. */
                        if (!ctx->radius_mode && parent_deadline == NO_DEADLINE) {
                            // double this root's absolute allowance
                            deadline = radio_budget_add(deadline, deadline - budget_start);
                        }
                        if (pass >= 2) {
                            if (ctx->radius_mode) {
                                /* Bounded (non-growing) radius calls never retry: they were asked
                                   for exactly one pass at a fixed cap, and MAYBE is an acceptable
                                   answer at that intermediate level. Growing calls retry with a
                                   doubled radius_N -- real widening, not a repeat of the same
                                   attempt -- until either resolved or radius_N exceeds every
                                   level's own true size (radius_truncated then reads false and the
                                   !skipped_some branch above takes over, since skipped_some is also
                                   false once no child can return MAYBE any more -- see radius_grows
                                   propagation at the cd computation below). Guard the multiply the
                                   same way probe_seconds guards its own doubling. */
                                if (radius_grows) {
                                    if (radius_N <= UINT64_MAX / 2) radius_N *= 2;
                                    else radius_N = UINT64_MAX;
                                } else {
                                    cont2 = 0;
                                }
                            } else if (!no_deadline
                                && deadline_expired(deadline, radio_effort_now_ctx(ctx, totalsplits))) {
                                cont2=0;
                            } else if (probe_seconds <= UINT_MAX / 2) {
                                /* Monotone work allowance replaces the old cache-progress gate:
                                   even with zero new verdicts, the next pass is not the same pass. */
                                probe_seconds *= 2;
                            }
                        }
                    }
                    break;
                }
                i--;
            }
            if (!cont) break;
            spi = --splitindex[i];
            spi2 = ordp[i][ords[i] * spi];
            
            debug_printf("i=%d, spi=%d, spi2=%d\n", i, spi, spi2);
            
            // for identical groups avoid trying redundant permutations
            if (i>1 && tmp[i] == tmp[i-1]) { // do not do this for i==1 because it conflicts with skiptop
                int spi_1 = splitindex[i-1];
                int spi2_1 = ordp[i-1][ords[i-1] * spi_1];
                if (spi2 > spi2_1) {
                    debug_printf("skip permutations\n");
                    continue;
                }
            }
            
            int *s = splitsarr[i]->splitsl[spi2];

            /* Counting-bound cut: the ordering makes one cumulative pair count monotone, so
               the first candidate to exceed max_pairs_1 retires the whole rest of the level.
               Measured 2026-08-03: ~89% of the 84M candidates that reach here fail this test
               one at a time. */
            if (ordmono[i] >= 0) {
                int pm;
                if (ordmono[i] == 0)      pm = sb_pairs[s[0]] + (i>0?sb0p[i-1]:0);
                else if (ordmono[i] == 1) pm = sb_pairs[s[1]] + sb_pairs[s[2]] + (i>0?sb1p[i-1]:0);
                else                      pm = sb_pairs[s[3]] + (i>0?sb2p[i-1]:0);
                if (pm > max_pairs_1) { splitindex[i] = 0; continue; }
            }

            /* Frozen refute tables have this one-part viability frontier prepared serially before
               workers start.  Never mutate shared split metadata inside the read epoch. */
            while (!frozen_refute && s[4]<k) {
                debug_printf("checking split solvability for %s -> [%d, %d], before: s[4]=%d s[5]=%d\n", sbb_to_str[tmp[i]], s[6], s[7], s[4], s[5]);
                int kk = s[4];
                uint64_t dd = size > 1 ? deadline : CACHE_ONLY;
                int ttt = canSolveB_ctx(ctx, s, 1, kk, dd);
                if (ttt==TRUE) {
                    ttt = canSolveB_ctx(ctx, s+3, 1, kk, dd);
                    if (ttt==TRUE) {
                        ttt = canSolveB_ctx(ctx, s+1, 2, kk, dd);
                    }
                }
                if (ttt == TRUE)
                    s[4] = MAX_K;
                else if (ttt == FALSE)
                    s[5] = ++s[4];
                else break;
            }
            if (s[5]>=k) {
                debug_printf("skipping for split solvablility %s -> [%d, %d], s[4]=%d s[5]=%d\n", sbb_to_str[tmp[i]], s[6], s[7], s[4], s[5]);
            } else {
                debug_printf("split solvablility ok %s -> [%d, %d], s[4]=%d s[5]=%d\n", sbb_to_str[tmp[i]], s[6], s[7], s[4], s[5]);
                if (i==0 &&
                    sb1[0] == s[1] &&
                    max(sb0[0],sb2[0]) == max(s[0], s[3]) &&
                    min(sb0[0],sb2[0]) == min(s[0], s[3])
                    )
                {
                    skiptop++;
                    debug_printf("skiptop\n");
                } else if (fast_solve
                           && (i<size_1)
                           && !s[FAST]
                           ) {
                    debug_printf("skipping not fast %d:%d for i = %d (%s)\n", s[6], s[7], i, sbb_to_str[tmp[i]]);
                    skipped_some = 1;
                } else {
                    totalsplits++;
                    radio_budget_charge_split_ctx(ctx);
                    /* Accepted prefixes can be extremely sparse in information-tight long states.
                       The deterministic clock is checked at every charged prefix.  The historical
                       CPU fallback retains its every-2^16 poll to keep clock() out of the hottest
                       loop.  A NO_DEADLINE proof is never stopped here, so exact work stays exact. */
#ifdef RADIO_WORK_BUDGET
                    if (!no_deadline
                        && deadline_expired(deadline, radio_effort_now_ctx(ctx, totalsplits))) {
#else
                    if (!no_deadline && !(totalsplits & DEADLINE_POLL_MASK)
                        && deadline_expired(deadline, radio_effort_now_ctx(ctx, totalsplits))) {
#endif
                        if (rb_here) rb_release_mode(rb, frozen_refute);
                        return MAYBE;
                    }
                    /* `>=`, not `==`. One reachability scratch set belongs to the complete recursive
                       search context, so only one state in that tree holds the tables at a time. With
                       equality a state whose trigger instant fell while another was armed lost its
                       only chance and ran the rest unpruned. Arming later is never worse than never
                       arming - the prune is a performance device, not a correctness one - so this
                       is safe. NOTE: this was originally changed while chasing a 27-minute cold
                       monster run that turned out to be a cold-vs-warm comparison error, not a
                       race. The fragility is real but has not been observed to fire. */
                    if (!rb_here && !rb->on && totalsplits >= RB_TRIGGER
                        && size >= 4 && power3[k_1] < RB_MAXCAP) {
                        int ri;
                        /* Reachability needs every suffix at once.  This is deliberately the
                           only bulk materialisation path; it runs only after the state has paid
                           enough search cost to arm the accelerator. */
                        for (ri = 0; ri < size; ri++) {
                            if (splitsarr[ri] == NULL)
                                splitsarr[ri] = ensure_splits(tmp[ri], k);
                        }
                        rb->on = rb_here = 1;
                        rb_build(rb, splitsarr, tmp, size, power3[k_1]);
#ifdef RADIO_RB_PLIABLE_CUTOFF
                        {
                            int slack = 3 * rb->cap - rb->remaining_mass[0];
                            for (ri = 0; ri <= rb->parts; ri++)
                                rb->pliable[ri] = rb_suffix_pliable(rb, ri, slack);
                        }
#endif
#ifdef RADIO_RB_PROFILE_DIAGNOSTIC
                        rb_profile_begin(rb, splitsarr, tmp, k);
#endif
                    }
                    int p0 = sb0p[i] = sb_pairs[sb0[i] = s[0]] + (i>0?sb0p[i-1]:0);
                    int p1 = sb1p[i] = sb_pairs[sb1[i*2] = s[1]] + sb_pairs[sb1[i*2+1] = s[2]] + (i>0?sb1p[i-1]:0);
                    int p2 = sb2p[i] = sb_pairs[sb2[i] = s[3]] + (i>0?sb2p[i-1]:0);
                    
                    int cs0, cs1, cs2;
                    int within_cap = p0 <= max_pairs_1 && p1 <= max_pairs_1 && p2 <= max_pairs_1;
                    int rb_rejected = 0;
                    
#ifdef DEBUG
                    printSb(sb0, i+1);
                    printSb(sb1, 2*i+2);
                    printSb(sb2, i+1);
                    
                    debug_printf(" i=%d p0=%d p1=%d p2=%d\n", i, p0,p1,p2);
#endif
                    if (within_cap && rb_here && i + 1 < size
#ifdef RADIO_RB_PLIABLE_CUTOFF
                        && !rb->pliable[i + 1]
#endif
                        ) {
                        rb_rejected = rb_dead(rb, i + 1, p0, p1, p2);
                        if (rb_rejected) rb_tainted_contraction = 1;
                    }
#if defined(RADIO_RB_PLIABLE_CUTOFF) && defined(RADIO_RB_PROFILE_DIAGNOSTIC)
                    else if (within_cap && rb_here && i + 1 < size && rb->pliable[i + 1]) {
                        rb->pliable_skipped++;
                    }
#endif
                    if (within_cap && !rb_rejected
                        && (cs0 = canSolveB_ctx(ctx, sb0, i+1, k_1, CACHE_ONLY))
                        && (cs2 = canSolveB_ctx(ctx, sb2, i+1, k_1, CACHE_ONLY))
                        && (cs1 = canSolveB_ctx(ctx, sb1, (i+1) * 2, k_1, CACHE_ONLY))
//                        && ((i != size/2) ||
//                            (((cs0 == TRUE) || (cs0 = canSolveB(sb0, i+1, k_1, SUBSPLIT_DEADLINE)))
//                             && ((cs2 == TRUE) || (cs2 = canSolveB(sb2, i+1, k_1, SUBSPLIT_DEADLINE)))
//                             && ((cs1 == TRUE) || (cs1 = canSolveB(sb1, (i+1) * 2, k_1, SUBSPLIT_DEADLINE)))
//                             ))
                        )
                    {
#ifdef MEASURE_FAST_REPLAY
                        if (fast_replay_capture && k == 5 && parent_deadline == NO_DEADLINE && pass == 1 && i < 32)
                            fast_replay_first_ok[i]++;
#endif
                        debug_printf("can solve\n");
                        if (i == size_1) {
                            /* One uncovered complete split is a complete audit failure.  A
                               refute-only verifier must not recurse in an attempt to solve any of
                               its children, and it need not enumerate later splits after this
                               concrete gap has been found. */
                            if (frozen_refute) {
#ifdef RADIO_FROZEN_REFUTE_TRACE
                                flockfile(stdout);
                                printf("FROZEN_REFUTE_GAP root=");
                                printSb(tmp, size);
                                printf(" k=%d split=", k);
                                printSb(sb0, size);
                                printSb(sb1, size2);
                                printSb(sb2, size);
                                printf(" child_verdicts=%d,%d,%d\n", cs0, cs1, cs2);
                                fflush(stdout);
                                funlockfile(stdout);
#endif
                                if (rb_here) rb_release_mode(rb, frozen_refute);
                                return MAYBE;
                            }
                            if (cs0 != TRUE || cs1 != TRUE || cs2 != TRUE) {
                                uint64_t budget_now = radio_effort_now_ctx(ctx, totalsplits);
                                if (deadline_expired(deadline, budget_now)) {
                                    if (no_deadline) {
                                        deadline = radio_budget_add(
                                            budget_now, radio_budget_seconds(10));
                                    } else {
                                        { if (rb_here) rb_release_mode(rb, frozen_refute); return MAYBE; }
                                    }
                                }
                                clock_t cpu_now = clock();
                                if (cpu_now >= progress) {
                                    printf("still solving in %d pass=%d fast_solve=%d ", k, pass, fast_solve);
                                    printSb(tmp, size);
                                    printf(" trying ");
                                    printSb(sb0, size);
                                    printSb(sb1, size2);
                                    printSb(sb2, size);
#ifdef RADIO_WORK_BUDGET
                                    uint64_t work_used = radio_budget_now_ctx(ctx) - budget_start;
                                    uint64_t work_limit = deadline > budget_start
                                        ? deadline - budget_start : 0;
                                    printf(" elapsed %llu/%llu work=%llu/%llu left=%d/%d totalsplits=%llu",
                                           (unsigned long long)(work_used
                                               / RADIO_BUDGET_UNITS_PER_SECOND),
                                           (unsigned long long)(work_limit
                                               / RADIO_BUDGET_UNITS_PER_SECOND),
                                           (unsigned long long)work_used,
                                           (unsigned long long)work_limit,
                                           splitindex[0], splitsarr[0]->size, totalsplits);
                                    printf(" cpu=%lu", (unsigned long)
                                           ((cpu_now - cpu_start) / CLOCKS_PER_SEC));
#else
                                    printf(" elapsed %lu/%lu left=%d/%d totalsplits=%llu",
                                           (unsigned long)((cpu_now - cpu_start) / CLOCKS_PER_SEC),
                                           (unsigned long)(deadline > budget_start
                                               ? (deadline - budget_start) / CLOCKS_PER_SEC : 0),
                                           splitindex[0], splitsarr[0]->size, totalsplits);
#endif
                                    if (shared_probe) printf(" probe=shared");
                                    else printf(" probe=%us", probe_seconds);
                                    printf(" budget=%s\n", no_deadline ? "unbounded" : "finite");
                                    fflush(stdout);
                                    progress = cpu_now + PROGRESS_INTERVAL;
                                }
                            }
                            /* radius_grows propagates recursively: a child of a progressively-
                               widening call gets NO_DEADLINE itself (not radius_N or deadline,
                               either of which could be a plain finite number by now) so it does
                               its own widen-until-resolved, all the way down. A bounded radius
                               call (radius_grows false) still propagates its fixed cap unchanged,
                               absolute, exactly as before. */
                            uint64_t cd = ctx->radius_mode
                                ? (radius_grows ? NO_DEADLINE : deadline)
                                : probe_child_deadline(ctx, deadline,
                                      radio_effort_now_ctx(ctx, totalsplits), probe_seconds, size);
                            /* MAYBE in one branch must not hide an easy refutation in another.
                               Probe all still-possible children, stopping only after a FALSE. */
                            if (cs0 == MAYBE)
                                cs0 = canSolveB_ctx(ctx, sb0, i+1, k_1, cd);
                            if (cs0 != FALSE && cs2 == MAYBE)
                                cs2 = canSolveB_ctx(ctx, sb2, i+1, k_1, cd);
                            if (cs0 != FALSE && cs2 != FALSE && cs1 == MAYBE)
                                cs1 = canSolveB_ctx(ctx, sb1, (i+1) * 2, k_1, cd);

                            if (cs0 == TRUE && cs2 == TRUE && cs1 == TRUE) {
                                //can solve
                                canSolve=TRUE;
                                cont=0;
                                cont2=0;
                                break;
                            }
                            if (cs0 != FALSE && cs2 != FALSE && cs1 != FALSE)
                                skipped_some = 1;

                            /* A child may have consumed the remainder of this probe's shared
                               allowance.  Unwind now instead of starting another candidate with
                               an already-expired parent budget. */
                            if (!no_deadline
                                && deadline_expired(deadline, radio_effort_now_ctx(ctx, totalsplits))) {
                                if (rb_here) rb_release_mode(rb, frozen_refute);
                                return MAYBE;
                            }
                        } else {
                            i++;
                            /* `rb_build` may already have materialised this table, but FAST is
                               intentionally prepared only when ordinary search reaches it. */
                            splitsarr[i] = frozen_refute
                                ? ensure_splits(tmp[i], k)
                                : prepare_splits_ctx(ctx, tmp[i], k, size > 1);
                            if (i>max_solvable_maybe) {
                                max_solvable_maybe = i;
                                debug_printf("max_solvable_maybe=%d\n", max_solvable_maybe);
                            }
                            if (ctx->radius_mode) {
                                int fulli = splitsarr[i]->size;
                                int cappedi = radius_N < (uint64_t)fulli ? (int)radius_N : fulli;
                                splitindex[i] = cappedi;
                                if (cappedi < fulli) radius_truncated = 1;
                            } else {
                                splitindex[i] = splitsarr[i]->size;
                            }
                            {
                                /* Pass 2 is exhaustive, so iteration order carries no
                                   solution-finding value and can be chosen purely to retire the
                                   level as early as possible. BY_SP2/BY_SP1/BY_SP0 are monotone in
                                   p0/p1/p2 respectively, and the counting-bound cut ends the level
                                   at the first option whose key exceeds the remaining budget - so
                                   the level runs for exactly cle[j][budget_j] options. Pick the
                                   smallest. The _DESC variants are never chosen here: they have
                                   ORDER_MONO_P = -1, so they get no early termination at all.
                                   Measured 2026-08-08 on the cheapest monster: 3.5x fewer candidate
                                   evaluations than the gap heuristic below. Radius mode keeps this
                                   heuristic unchanged -- see the level-0 comment above. */
                                int chosen = -1;
                                if (pass >= 2 && size > 3) {
                                    splits *sp_ = splitsarr[i];
                                    int M = sp_->clen, best = 1 << 30, j;
                                    int bnd[3];
                                    bnd[0] = max_pairs_1 - p0;   /* BY_SP2 is monotone in p0 */
                                    bnd[1] = max_pairs_1 - p1;   /* BY_SP1 in p1 */
                                    bnd[2] = max_pairs_1 - p2;   /* BY_SP0 in p2 */
                                    static const int ORD_FOR_KEY[3] = { BY_SP2, BY_SP1, BY_SP0 };
                                    for (j = 0; j < 3; j++) {
                                        int b = bnd[j]; int cc;
                                        if (b < 0) cc = 0; else cc = sp_->cle[j][b > M ? M : b];
                                        if (cc < best) { best = cc; chosen = ORD_FOR_KEY[j]; }
                                    }
                                }
                                if (chosen >= 0) {
                                    splitincr[i] = chosen;
                                } else
                                // confusingly enough p0 corresponds to BY_SP2 and p2 to BY_SP0
                                if (size<=3) {
                                    splitincr[i] = BY_SP1; // special case for size== 2 or 3
                                } else if (p0 > p1) {
                                    if (p1 > p2) { // p0 > p1 > p2
                                        splitincr[i] = ( p0 - p1 > p1 - p2) ? BY_SP2 : BY_SP0_DESC;
                                    } else if (p0 > p2) { // p0 > p2 >= p1
                                        splitincr[i] = ( p0 - p2 > p2 - p1) ? BY_SP2 : BY_SP1_DESC;
                                    } else { // p2 >= p0 > p1
                                        splitincr[i] = ( p2 - p0 > p0 - p1) ? BY_SP0 : BY_SP1_DESC;
                                    }
                                } else { // p1 >=p0
                                    if (p0 > p2) { // p1 >= p0 > p2
                                        splitincr[i] = (p1 - p0 > p0 - p2) ? BY_SP1 : BY_SP0_DESC;
                                    } else if (p1 > p2) { // p1 > p2 >= p0
                                        splitincr[i] = ( p1 - p2 > p2 - p0) ? BY_SP1 : BY_SP2_DESC;
                                    } else { // p2 >= p1 >= p0
                                        splitincr[i] = ( p2 - p1 > p1 - p0) ? BY_SP0 : BY_SP2_DESC;
                                    }
                                }
                            }
                            HOIST_ORDER(i);
                        }
                    }
                }
            }
        }
    }
    
    if (frozen_refute) {
        if (rb_here) rb_release_mode(rb, frozen_refute);
        if (!canSolve && !skipped_some && !radius_truncated) ctx->cant_solve_count++;
        return (skipped_some || radius_truncated) ? MAYBE : canSolve;
    }

    if (canSolve) {
        //        printf("cansolve=true\n");
        //        fflush(stdout);
        
        printf("can solve ");
        printSb(tmp, size);
        printf(" in %d with [",k);
        for (i = 0; i<size; i++) {
            spi = splitindex[i];
            spi2 = ordp[i][ords[i] * spi];
            int *s = splitsarr[i]->splitsl[spi2];
            if (i>0) printf(",");
            printf("%d:%d", s[6], s[7]);
            if (!s[FAST]) {
                printf(":NOTFAST");
                if (size>2 && i<size_1) {
#ifndef NO_FAST_LEARN
                    s[FAST]=1;
#endif
                    printf("-ADDED");
                }
            }
        }
        printf("] ");
        printSb(sb0,size);
        printSb(sb1,size*2);
        printSb(sb2,size);
        //        printf("cansolve=true\n");
        //        printf("totalsplits=%llu\n", totalsplits);
        //        fflush(stdout);
        
    } else if (skipped_some || radius_truncated) {
        if (rb_here) rb_release_mode(rb, frozen_refute);
        return MAYBE;
    } else {
        ctx->cant_solve_count++;
        printf("can't solve ");
        int contraction_candidate_size = max_solvable_maybe + 1;
        int contraction_suppressed = contraction_candidate_size < size && rb_tainted_contraction;
        if (contraction_candidate_size < size && !contraction_suppressed) {
            debug_printf("max_solvable_maybe=%d\n", max_solvable_maybe);
            printf("size=%d/", size);
            size = contraction_candidate_size;
            printf("%d ", size);
            // recompute pairs
            pairs = 0;
            for(i=0;i<size;i++) {
                pairs+=sb_pairs[sb[i]];
            }
        }
        printSb(tmp, size);
        printf(" in %d",k);
    }
    clock_t t = clock()-cpu_start;
    clock_t s = t/CLOCKS_PER_SEC;
    if (s>0)
        printf(" took %ld", s);
    else
        printf(" took 0.%03ld", t * 1000/CLOCKS_PER_SEC);
    if (rb_here) rb_release_mode(rb, frozen_refute);
    printf(" totalsplits=%llu pass=%d fast_solve=%d", totalsplits, pass, fast_solve);
    if (shared_probe) printf(" probe=shared");
    else printf(" probe=%us", probe_seconds);
    if (!canSolve && max_solvable_maybe + 1 < query_size && rb_tainted_contraction)
        printf(" contraction=rb-suppressed:%d", max_solvable_maybe + 1);
#ifdef RADIO_WORK_BUDGET
    printf(" work=%llu rate=%llu",
           (unsigned long long)(radio_budget_now_ctx(ctx) - budget_start),
           (unsigned long long)RADIO_BUDGET_UNITS_PER_SECOND);
#endif
#ifdef MEASURE_FAST_REPLAY
    if (fast_replay_capture && k == 5 && parent_deadline == NO_DEADLINE) {
        fast_replay_pass = pass;
        fast_replay_fast = fast_solve;
        fast_replay_splits = totalsplits;
        fast_replay_capture = 0;
    }
#endif
    
#ifdef DEBUG1
    fflush(stdout);
#endif
    cache_l1_store(ctx, l1_entry, l1_hash, tmp, query_size, k, canSolve);
    cache(tmp, size, canSolve, k, pairs);
    //    fflush(stdout);
    printf("\n");
#ifndef OPT_2
    fflush(stdout);
#endif
    return canSolve;
}

int canSolveB(int *sb, int size, int k, uint64_t parent_deadline) {
    return canSolveB_ctx(&radio_default_search_context, sb, size, k, parent_deadline);
}

/* Audit one claimed negative against a cache which the caller has completely built and frozen.
   FALSE means every legal test was covered by a theorem or a cached lower-level negative;
   MAYBE exposes a concrete uncovered test; TRUE means a base theorem contradicts the claim. */
int canRefuteB_ctx(radio_search_context *ctx, int *sb, int size, int k) {
    return canSolveB_ctx(ctx, sb, size, k, FROZEN_REFUTE);
}

int sbb_to_min_k[MAX_SBB+1];

int minK_ctx(radio_search_context *ctx, int sbb) {
    int kk = sbb_to_min_k[sbb];
    if (kk<0) {
        debug_printf("computing min_k for %s...\n", sbb_to_str[sbb]);
        kk=1;
        int rr;
        while ((rr = canSolveB_ctx(
                    ctx, &sbb, 1, kk, radio_budget_after_seconds_ctx(ctx, 1000))) == TRUE)
            kk++;
        debug_printf("min_k=%d for %s...\n", kk, sbb_to_str[sbb]);
        if (rr == FALSE) sbb_to_min_k[sbb]=kk; // if we got maybe, assume false, but do not memorize
        debug_printf("cached min_k=%d for %s...\n", kk, sbb_to_str[sbb]);
    }
    return kk;
}

int minK(int sbb) {
    return minK_ctx(&radio_default_search_context, sbb);
}

void cache_a(int canSolve, int n, int k) {
    if(canSolve){
        int i;
        for (i = n; i > 0; i--) {
            sa_can[i] = min(k, sa_can[i]);
        }
    } else {
        int i;
        for (i = n; i <=MAX_N; i++) {
            sa_cant[i]=max(k, sa_cant[i]);
        }
    }
}

int canSolveA(int n, int k) {
    
    int pairs = saPairs(n);
    //	printf("pairs=%d\n",pairs);
    if (pairs<=1) {
        //		printf("pairs is <=1, can solve\n");
        return 1;
    }
    if (sa_can[n]<=k) {
#ifdef DEBUG1
        printf("k=%d sa_can[%d]=%d, can solve\n", k, n, sa_can[n]);
#endif
        return 1;
    }
    if (sa_cant[n]>=k) {
#ifdef DEBUG1
        printf("k=%d sa_cant[%d]=%d, can't solve\n", k, n, sa_cant[n]);
#endif
        return 0;
    }
    int canSolve = 0;
    
    clock_t start = clock();
#ifdef DEBUG1
    printf("solving Sa(%d) in %d\n",n,k);
    fflush(stdout);
#endif
    if (pairs<=power3[k]){
        int n1 = n-1;
        int sb[1];
        while (n1>=(n+1)/2 && canSolve == 0) {
            //			printf("n1=%d\n", n1);
            if (canSolveA(n1,k-1)) {
                sb[0]=getSbb(n1,n-n1);
                //				printf("sb[0]=%d \n",sb[0]);
                //				printSb(sb,1);
                //				printf("\n");
                if (canSolveB(sb,1,k-1,NO_DEADLINE)) {
                    canSolve = 1;
                    printf("can solve Sa(%d) in %d with following:",n,k);
                    printSa(n1);
                    printf(",");
                    printSa(n - n1);
                    printf(",");
                    printSb(sb,1);
#ifdef DEBUG1
                    fflush(stdout);
#endif
                }
            }
            n1--;
        }
    } else {
        printf("power3[%d]=%d can't solve should not be here\n", k, power3[k]);
        fflush(stdout);
        exit(5);
    }
    cache_a(canSolve,n,k);
    if(!canSolve){
        printf("can't solve Sa(%d) in %d",n,k);
    }
    clock_t t = clock()-start;
    clock_t s = t/CLOCKS_PER_SEC;
    if (s>0)
        printf(" took %ld\n", s);
    else
        printf(" took 0.%03ld\n", t * 1000/CLOCKS_PER_SEC);
#ifdef DEBUG1
    fflush(stdout);
#endif
    return canSolve;
} 

typedef struct { int sort; int index; int *s;} srt;

// sort splits such that symmetrical ones are next to each other
int descSpl (const void * a, const void * b) {
    srt *a1 = (srt*)a;
    srt *b1 = (srt*)b;
    int result = b1->sort - a1->sort;
    if (result) return result;
    result = b1->s[1] - a1->s[1];
    if (result) return result;
    result = max(b1->s[0], b1->s[3]) - max(a1->s[0], a1->s[3]);
    if (result) return result;
    result = min(b1->s[0], b1->s[3]) - min(a1->s[0], a1->s[3]);
    return result;
}

void indexSpl(int sbb, splits* s, int indexindex, int (*f)(int, int[])) {
    srt *splitsort;
    int e;
    int c = s->size;
    if (c == 0) return;
    splitsort = (srt *)malloc(c * sizeof(srt));
    if (splitsort == NULL) {
        printf("\nout of memory - can't allocate split sort buffer for sbb=%d\n", sbb);
        exit(1);
    }
    for(e = 0; e<c; e++) {
        splitsort[e].sort = f(sbb, s->splitsl[e]);
        splitsort[e].index = e;
        splitsort[e].s = s->splitsl[e];
    }
    qsort(splitsort, c, sizeof(srt), descSpl); // sort by first element
    for(e = 0; e<c; e++) {
        s->ind[indexindex][e] = splitsort[e].index;
    }
    free(splitsort);
}

int maxpairsraw(int sbb, int spl[]) {
    return max(sb_pairs[spl[0]], max(sb_pairs[spl[3]], sb_pairs[spl[1]] + sb_pairs[spl[2]]));
}

int minpairsraw(int sbb, int spl[]) {
    //    return min(sb_pairs[spl[0]], min(sb_pairs[spl[3]], (sb_pairs[spl[1]] + sb_pairs[spl[2]]) * 411 / 1000 /* magic! */));
    return min(sb_pairs[spl[0]], min(sb_pairs[spl[3]], sb_pairs[spl[1]] + sb_pairs[spl[2]]));
}

int pairs2raw(int sbb, int spl[]) {
    return sb_pairs[spl[0]];
}

int pairs0raw(int sbb, int spl[]) {
    return sb_pairs[spl[3]];
}

int pairs1raw(int sbb, int spl[]) {
    return sb_pairs[spl[1]] + sb_pairs[spl[2]];
}

int pairs2(int sbb, int spl[]) {
    return pairs2raw(sbb, spl) * (1+sb_pairs[sbb]) + abs(pairs1raw(sbb, spl) - pairs0raw(sbb, spl));
}

int pairs0(int sbb, int spl[]) {
    return pairs0raw(sbb, spl) * (1+sb_pairs[sbb]) + abs(pairs2raw(sbb, spl) - pairs0raw(sbb, spl));
}

int pairs1(int sbb, int spl[]) {
    return pairs1raw(sbb, spl) * (1+sb_pairs[sbb]) + abs(pairs2raw(sbb, spl) - pairs0raw(sbb, spl));
}

int pairs1_0(int sbb, int spl[]) {
    int n1 = sbb_to_n1[sbb];
    int n2 = sbb_to_n2[sbb];
    int magicm1 = n1/4;
    int magicm2 = 0;
    int dx = abs(spl[6] - magicm1);
    int dy = abs(spl[7] - magicm2);
    return dy * (n1+1) + dx;
}

int pairs1_2(int sbb, int spl[]) {
    int n1 = sbb_to_n1[sbb];
    int n2 = sbb_to_n2[sbb];
    int magicm1 = n1 - n1/4;
    int magicm2 = n2;
    int dx = abs(spl[6] - magicm1);
    int dy = abs(spl[7] - magicm2);
    return dy * (n1+1) + dx;
}

int maxpairs(int sbb, int spl[]) {
    return (1+maxpairsraw(sbb, spl)) * (1+sb_pairs[sbb]) - minpairsraw(sbb, spl);
}

int distance(int spl[], int magicm1, int magicm2, int n1, int n2) {
    int dx = (spl[6] - magicm1) * 1000/n1;
    int dy = (spl[7] - magicm2)* 1000/n2;
    int dx2 = (spl[6] - (n1 - magicm1))* 1000/n1;
    int dy2 = (spl[7] - (n2 - magicm2))* 1000/n2;
    return min(dx*dx + dy*dy, dx2*dx2 + dy2*dy2);
}

int magic(int sbb, int spl[]) {
//    int n1 = sbb_to_n1[sbb];
//    int n2 = sbb_to_n2[sbb];
//    int msum = ((n1+n2)*577+999)/1000;   // magic ratio is 0.577 = (1/sqrt(3))
//    int magicm1 = min(n1, (n1*577+999)/1000);
//    int magicm2 = min(n2, msum-magicm1);
//    return distance(spl, magicm1, magicm2, n1, n2);
    return max(pairs1raw(sbb, spl) * 10000, 14141 * max(pairs0raw(sbb, spl), pairs2raw(sbb, spl))); // sqrt(2) ratio seems right
}


int magic2(int sbb, int spl[]) { // magic ration is .666 = 2/3
    int n1 = sbb_to_n1[sbb];
    int n2 = sbb_to_n2[sbb];
    int msum = ((n1+n2)*700+500)/1000;
    int magicm1 = min(n1, (n1*700+500)/1000);
    int magicm2 = min(n2, msum-magicm1);
    return distance(spl, magicm1, magicm2, n1, n2);
}

int magic3(int sbb, int spl[]) {
    //    return (1+minpairsraw(sbb, spl)) * (1+sb_pairs[sbb]) - maxpairsraw(sbb, spl);
    int n1 = sbb_to_n1[sbb];
    int n2 = sbb_to_n2[sbb];
    //    int msum = (n1+n2-3)/2;
    int magicm1 = n1/2;
    int magicm2 = n2/2;
    //int magicm2 = ((n1-n2)<2)?(n2/2-1):(n2/2);
    //    int magicm2 = min(n2/2, magicm1-1);
    return distance(spl, magicm1, magicm2, n1, n2);
}

/* Unconditional counters make memory experiments possible without changing table layout.
   They are not printed during normal runs; small probe drivers can inspect them directly. */
unsigned long long split_level_fanouts;
unsigned long long split_level_fanout_bytes;
unsigned long long split_tables_built[MAX_K + 1];
unsigned long long split_table_candidates[MAX_K + 1];
unsigned long long split_table_options[MAX_K + 1];
unsigned long long split_table_bytes[MAX_K + 1];

#ifdef SPLIT_STATS
static void report_split_stats(void) {
    unsigned long long table_total = 0;
    int k;
    fprintf(stderr, "SPLIT_STATS index_bytes=%zu fanouts=%llu fanout_bytes=%llu\n",
            (size_t)(MAX_SBB + 1) * sizeof(*sbb_splits),
            split_level_fanouts, split_level_fanout_bytes);
    for (k = 1; k <= MAX_K; k++) {
        if (split_tables_built[k] == 0) continue;
        fprintf(stderr,
                "SPLIT_STATS k=%d tables=%llu candidates=%llu options=%llu bytes=%llu\n",
                k, split_tables_built[k], split_table_candidates[k],
                split_table_options[k], split_table_bytes[k]);
        table_total += split_table_bytes[k];
    }
    fprintf(stderr, "SPLIT_STATS table_bytes=%llu total_bytes=%llu\n",
            table_total,
            table_total + split_level_fanout_bytes
                + (unsigned long long)(MAX_SBB + 1) * sizeof(*sbb_splits));
}
#endif

static void fill_split(int sbb, int m1, int m2, int out[SPLIT_FIELD_COUNT]) {
    int n1 = sbb_to_n1[sbb];
    int n2 = sbb_to_n2[sbb];
    int sbb1;
    int sbb2;
    int maxpairs;
    int kk = 0;

    out[0] = getSbb(m1, m2);
    sbb1 = getSbb(n1 - m1, m2);
    sbb2 = getSbb(m1, n2 - m2);
    out[1] = max(sbb1, sbb2);
    out[2] = min(sbb1, sbb2);
    out[3] = getSbb(n1 - m1, n2 - m2);
    maxpairs = max(sb_pairs[out[0]],
                   max(sb_pairs[out[3]], sb_pairs[out[1]] + sb_pairs[out[2]]));
    while (kk < MAX_K && power3[kk] <= maxpairs) kk++;
    out[4] = out[5] = kk - 1;
    out[6] = m1;
    out[7] = m2;
    out[FAST] = -1;
}

/* This mirrors canSolveB's two theorem-level prechecks, including Unit Group Elimination.
   No cache fact or heuristic estimate is allowed here: a false rejection would make the
   supposedly exhaustive pass incomplete. */
static int split_child_admissible(const int *sb, int size, int k) {
    int nonunit[2];
    int nonunit_size = 0;
    int pairs_full = 0;
    int i;

    for (i = 0; i < size; i++) {
        pairs_full += sb_pairs[sb[i]];
        if (sb[i] > 1) nonunit[nonunit_size++] = sb[i];
    }
    if (pairs_full > power3[k]) return FALSE;
    if (nonunit_size == 0) return TRUE;
    return star_expansion_majorization_can_solve(nonunit, nonunit_size, k);
}

static int split_admissible(const int sp[SPLIT_FIELD_COUNT], int child_k) {
    int middle[2] = { sp[1], sp[2] };
    return split_child_admissible(sp, 1, child_k)
        && split_child_admissible(middle, 2, child_k)
        && split_child_admissible(sp + 3, 1, child_k);
}

splits *ensure_splits(int sbb, int k) {
    static const int live_orderings[] = { BY_SP0, BY_SP1, BY_SP2, BY_MAGIC3 };
    const int live_count = (int)(sizeof(live_orderings) / sizeof(live_orderings[0]));
    split_levels *levels;
    splits *s;
    int n1;
    int n2;
    int candidate[SPLIT_FIELD_COUNT];
    int candidates = 0;
    int c = 0;
    int m1;
    int m2;
    int ii;
    int M;
    size_t table_ints;
    size_t index_ints;
    size_t cle_ints;
    size_t total_ints;
    size_t bytes;
    int *storage;

    if (sbb <= 0 || sbb > MAX_SBB || k <= 0 || k > MAX_K) {
        printf("\ninvalid split table key sbb=%d k=%d\n", sbb, k);
        exit(1);
    }
    levels = sbb_splits[sbb];
    if (levels == NULL) {
        levels = (split_levels *)calloc(1, sizeof(*levels));
        if (levels == NULL) {
            printf("\nout of memory - can't allocate split levels for sbb=%d\n", sbb);
            exit(1);
        }
        sbb_splits[sbb] = levels;
        split_level_fanouts++;
        split_level_fanout_bytes += sizeof(*levels);
    }
    if (levels->at[k] != NULL) return levels->at[k];

    n1 = sbb_to_n1[sbb];
    n2 = sbb_to_n2[sbb];
    for (m1 = 0; m1 <= n1; m1++) {
        for (m2 = (n2 == n1 ? m1 : n2); m2 >= 0; m2--) {
            fill_split(sbb, m1, m2, candidate);
            candidates++;
            if (split_admissible(candidate, k - 1)) c++;
        }
    }

    M = sb_pairs[sbb];
    table_ints = (size_t)c * SPLIT_FIELD_COUNT;
    index_ints = (size_t)c * live_count;
    cle_ints = (size_t)3 * (M + 2);
    total_ints = table_ints + index_ints + cle_ints;
    if (total_ints > ((size_t)-1 - sizeof(*s)) / sizeof(int)) {
        printf("\nsplit table size overflow for sbb=%d k=%d\n", sbb, k);
        exit(1);
    }
    bytes = sizeof(*s) + total_ints * sizeof(int);
    s = (splits *)calloc(1, bytes);
    if (s == NULL) {
        printf("\nout of memory - can't allocate split table for sbb=%d k=%d (%zu bytes)\n",
               sbb, k, bytes);
        exit(1);
    }

    s->size = c;
    s->clen = M;
    storage = (int *)(s + 1);
    s->splitsl = (int (*)[SPLIT_FIELD_COUNT])storage;
    storage += table_ints;
    for (ii = 0; ii < live_count; ii++) {
        s->ind[live_orderings[ii]] = storage;
        storage += c;
    }
    for (ii = 0; ii < 3; ii++) {
        s->cle[ii] = storage;
        storage += M + 2;
    }

    c = 0;
    for (m1 = 0; m1 <= n1; m1++) {
        for (m2 = (n2 == n1 ? m1 : n2); m2 >= 0; m2--) {
            fill_split(sbb, m1, m2, candidate);
            if (split_admissible(candidate, k - 1)) {
                memcpy(s->splitsl[c++], candidate, sizeof(candidate));
            }
        }
    }
    if (c != s->size || storage != (int *)(s + 1) + total_ints) {
        printf("\ninternal split table size mismatch for sbb=%d k=%d\n", sbb, k);
        exit(1);
    }

    {   /* cle: histogram of each key, then prefix-summed */
        int j;
        int x;
        for (x = 0; x < c; x++) {
            int *sp = s->splitsl[x];
            int key[3];
            key[0] = sb_pairs[sp[0]];
            key[1] = sb_pairs[sp[1]] + sb_pairs[sp[2]];
            key[2] = sb_pairs[sp[3]];
            for (j = 0; j < 3; j++) {
                if (key[j] >= 0 && key[j] <= M) s->cle[j][key[j]]++;
            }
        }
        for (j = 0; j < 3; j++) {
            for (x = 1; x <= M; x++) s->cle[j][x] += s->cle[j][x - 1];
        }
    }
    indexSpl(sbb, s, BY_SP0, pairs0);
    indexSpl(sbb, s, BY_SP1, pairs1);
    indexSpl(sbb, s, BY_SP2, pairs2);
    indexSpl(sbb, s, BY_MAGIC3, magic3);

    levels->at[k] = s;
    split_tables_built[k]++;
    split_table_candidates[k] += candidates;
    split_table_options[k] += s->size;
    split_table_bytes[k] += bytes;
    return s;
}

splits *prepare_splits_ctx(radio_search_context *ctx, int sbb, int k, int need_fast) {
    splits *sp = ensure_splits(sbb, k);
    if (need_fast && sp->size > 0 && sp->splitsl[0][FAST] < 0) {
        int n1 = sbb_to_n1[sbb];
        int n2 = sbb_to_n2[sbb];
        int c;
        debug_printf("initializing FAST for %s in k=%d\n", sbb_to_str[sbb], k);
        for (c = 0; c < sp->size; c++) {
            int m1 = sp->splitsl[c][6];
            int m2 = sp->splitsl[c][7];
            int fast = FALSE;
            if (n1 == n2) {
                /* Special case for square groups (n1==n2). */
                if (m2 == m1 - 1) fast = TRUE;
            } else {
                int sbb1 = get_max_sbb_ctx(ctx, m1, n2 - m2, n1 - m1, m2);
                if ((m2 == 0 || compare_solvability_ctx(
                         ctx, sbb1,
                         get_max_sbb_ctx(ctx, m1, n2 - m2 + 1, n1 - m1, m2 - 1)) <= 0)
                    && (m2 == n2 || compare_solvability_ctx(
                         ctx, sbb1,
                         get_max_sbb_ctx(ctx, m1, n2 - m2 - 1, n1 - m1, m2 + 1)) <= 0)) {
                    fast = TRUE;
                }
            }
            sp->splitsl[c][FAST] = fast;
#ifdef DEBUG1
            if (fast) printf("FAST for %s -> [%d:%d]\n", sbb_to_str[sbb], m1, m2);
#endif
        }
    }
    return sp;
}

splits *prepare_splits(int sbb, int k, int need_fast) {
    return prepare_splits_ctx(&radio_default_search_context, sbb, k, need_fast);
}

int canSolveAll4(int n1, int n2, int m1, int m2, int k) {
    int sbb = getSbb(m1,m2);
    if (!canSolveB(&sbb, 1, k, CACHE_ONLY)) return FALSE;
    sbb = getSbb(n1-m1,n2-m2);
    if (!canSolveB(&sbb, 1, k, CACHE_ONLY)) return FALSE;
    sbb = getSbb(m1,n2-m2);
    if (!canSolveB(&sbb, 1, k, CACHE_ONLY)) return FALSE;
    sbb = getSbb(n1-m1,m2);
    return canSolveB(&sbb, 1, k, CACHE_ONLY);
}

void all_solutions(int sb[], int size, int k) {
    int tmp[size];
    int i;
    int pairs=0;
    int newsize=0;
    int sbb;
    for(i=0;i<size;i++) {
        sbb=sb[i];
        if (sbb > 0) {
            pairs+=sb_pairs[sbb];
            tmp[newsize++]=sbb;
        }
    }
    size = newsize;
    
    sort1(tmp, size);
   
    int n[size*2], m[size*2];
    int sb0[size*2], sb1[size*2], sb2[size*2];
    
    int counts[size][MAX_N+1][MAX_N+1];
    memset(counts, 0, sizeof(counts));

    
    for(i=0;i<size;i++) {
        n[i*2] = sbb_to_n1[tmp[i]];
        n[i*2+1] = sbb_to_n2[tmp[i]];
        m[i*2] = 0;
        m[i*2+1] = 0;
    }
    
    m[0] = 1 + n[0];
    int j = 0;
    unsigned long long solved = 0;
    unsigned long long total = 0;
    while(1) {
        while(m[j] == 0) {
            if (j==0) {
                int m1, m2;
                //headers
                for (i=0; i<size; i++) {
                    printf("result ");
                    int i2;
                    for (i2 = 0; i2<i; i2++) {
                        int i3;
                        for(i3 = 0; i3<=n[i2*2+1]; i3++) printf(" ");
                        printf(" ");
                    }
                    
                    printf("%s => ", sbb_to_str[tmp[i]]);
                    for(m1 = 0; m1 <= n[i*2]; m1++) {
                        for(m2 = 0; m2 <= n[i*2+1]; m2++) {
                            int count = counts[i][m1][m2];
                            if (count > 0) {
                                printf("[%d:%d]; ", m1, m2);
                            }
                        }
                    }
                    printf("\n");
                }
                m1 = 0;
                while(1) {
                    int lastrow=1;
                    printf("result ");
                    for (i=0; i<size; i++) {
                        if (m1 < n[i*2]) lastrow = 0;
                        for(m2 = 0; m2 <= n[i*2+1]; m2++) {
                            if (m1 <= n[i*2]) {
                                int count = counts[i][m1][m2];
                                if (count > 0) {
                                    if (count<10)
                                        printf("%d", count);
                                    else
                                        printf("*");
                                } else if (canSolveAll4(n[i*2],n[i*2+1], m1, m2, k-1)) {
                                    printf(".");
                                } else {
                                    printf("-");
                                }
                            } else printf(" ");
                        }
                        printf(" ");
                    }
                    printf("\n");
                    if (lastrow) break;
                    m1++;
                }
                double s = (double)solved / total;
                printf("result in %d ratio = %llu/%llu solvability %f ", k, solved, total, s);
                printSb(tmp, size);
                printf("\n");
                return;
            }
            j--;
        }
        m[j]--;
//        printf("l1 j = %d mj = %d\n", j, m[j]);
        for(i=0;i<size;i++) {
            sb0[i] = getSbb(m[i*2], m[i*2+1]);
            sb2[i] = getSbb(n[i*2] - m[i*2], n[i*2+1] - m[i*2+1]);
            sb1[i*2] = getSbb(m[i*2], n[i*2+1] - m[i*2+1]);
            sb1[i*2 + 1] = getSbb(n[i*2] - m[i*2], m[i*2+1]);
        }
        
        if (j == size*2 - 1) {
            total++;
            if (canSolveB(sb0, size, k-1, CACHE_ONLY) != FALSE &&
                canSolveB(sb2, size, k-1, CACHE_ONLY) != FALSE &&
                canSolveB(sb1, size*2, k-1, CACHE_ONLY) != FALSE &&
                canSolveB(sb0, size, k-1, 2) == TRUE &&
                canSolveB(sb2, size, k-1, 2) == TRUE &&
                canSolveB(sb1, size*2, k-1, 2) == TRUE) {
                solved++;
                printf("result in %d can solve ", k);
                printSb(tmp, size);
                printf(" with [");
                for (i = 0; i<size; i++) {
                    if (i>0) printf(",");
                    printf("%d:%d", m[i*2], m[i*2+1]);
                    counts[i][m[i*2]][m[i*2+1]]++;
                }
                printf("] => ");
                printSb(sb0, size);
                printSb(sb1, size*2);
                printSb(sb2, size);
                printf("\n");
            }
        } else {
            j++;
            m[j] = n[j] + 1;
        }
    }
}

#define BUFSIZE 1000

void parse_file(char *file_name) {
    FILE *fp = fopen(file_name, "r");
    
    if (fp == NULL) {
        printf("Failed to open file '%s'\n", file_name);
        exit(14);
    }
    cache_replay_depth++;
    printf("\nreading file %s\n", file_name);
    char buff[BUFSIZE];
    int line_count=0;
    int comment_continuation=0;

    while(fgets(buff, BUFSIZE - 1, fp) != NULL)
    {
        line_count++;
        if (line_count % 10000 == 0) {
            printf(".");
            if (line_count % 1000000 == 0) {
                printf("\n");
            }
            fflush(stdout);
        }
        debug_printf("\nINPUT: %s\n", buff);
        if (comment_continuation || buff[0] == '#') {
            /* A provenance argument can legally be longer than BUFSIZE.  fgets then returns its
               continuation without a leading '#'; remember that state so an exact long argument
               cannot be mistaken for a cache fact on replay. */
            /* Echo cache metadata verbatim. parse_out.sh preserves comment lines, so a generic
               warm-started segment carries the provenance/certificate headers of its inputs even
               outside the specialised Sa(193) checkpoint wrappers. */
            fputs(buff, stdout);
            comment_continuation = strchr(buff, '\n') == NULL;
            continue;
        }
        char* token = strtok(buff, " ");
        if (token == NULL) {
            printf("Unexpected NULL\n");
            exit(15);
        }
        int can_solve = ((*token) == '+');
        token = strtok(NULL, " ");
        int is_a = ((*token) == 'a');
        if (is_a) { // Sa
            token = strtok(NULL, " ");
            int n = atoi(token);
            token = strtok(NULL, " ");
            int k = atoi(token);
            debug_printf("\nPARSED: Sa(%d) cs:%d in %d\n", n, can_solve, k);
            // cache
            cache_a(can_solve,n,k);
        } else { // Sb
            int sb[BUFSIZE];
            int size = 0;
            while(1) {
                token = strtok(NULL, " ");
                if (*token == 't') break;
                int n1 = atoi(token);
                token = strtok(NULL, " ");
                int n2 = atoi(token);
                sb[size++] = getSbb(n1,n2);
            }
            token = strtok(NULL, " ");
            int pairs = atoi(token);
            token = strtok(NULL, " ");
            int n = atoi(token);
            token = strtok(NULL, " ");
            int k = atoi(token);
#ifdef DEBUG
            debug_printf("PARSED: ");
            printSb(sb, size);
            printf(" pairs=%d n=%d k=%d cs=%d ", pairs, n, k, can_solve);
#endif
            cache(sb, size, can_solve, k, pairs);
        }
        if (strtok(NULL, " ") != NULL) {
            printf("\nexpected end of line\n");
            exit(18);
        }
    }
    fclose(fp);
    cache_replay_depth--;
    printf("done\n");
    fflush(stdout);
}

/* ------------------------------------------------------------------------- provenance
 *
 * Every durable solver output starts with this block.  Lines are comments so they are harmless in
 * cache files (parse_file deliberately skips '#') and witness extractors can ignore them.  Values
 * use a small byte-exact escape language: backslash, newline, carriage return and tab have their
 * usual C spellings; other non-printable/non-ASCII bytes are \xHH.  Argument boundaries are kept by
 * printing one indexed field per argument rather than a lossy shell command string.
 */

static int radio_provenance_printed;

static void radio_provenance_value(const char *key, const char *value) {
    const unsigned char *p = (const unsigned char *)(value ? value : "");
    printf("# %s=", key);
    for (; *p; p++) {
        switch (*p) {
            case '\\': printf("\\\\"); break;
            case '\n': printf("\\n"); break;
            case '\r': printf("\\r"); break;
            case '\t': printf("\\t"); break;
            default:
                if (*p >= 0x20 && *p <= 0x7e) putchar(*p);
                else printf("\\x%02x", *p);
        }
    }
    putchar('\n');
}

static void radio_provenance_number(const char *key, unsigned long long value) {
    printf("# %s=%llu\n", key, value);
}

static int radio_provenance_runtime_argv(void) {
    int count = 0;
    int complete = 1;
#ifdef __APPLE__
    int argc = *_NSGetArgc();
    char **argv = *_NSGetArgv();
    radio_provenance_value("runtime_argv_source", "libSystem _NSGetArgv");
    for (int i = 0; i < argc; i++) {
        char key[64];
        snprintf(key, sizeof(key), "run_arg[%d]", i);
        radio_provenance_value(key, argv[i]);
    }
    count = argc;
#elif defined(__linux__)
    FILE *fp = fopen("/proc/self/cmdline", "rb");
    if (fp != NULL) {
        size_t len = 0, cap = 128;
        char *arg = (char *)malloc(cap);
        radio_provenance_value("runtime_argv_source", "/proc/self/cmdline");
        if (arg != NULL) {
            int ch;
            while ((ch = fgetc(fp)) != EOF) {
                if (ch != 0) {
                    if (len + 1 >= cap) {
                        size_t new_cap = cap * 2;
                        char *grown = (char *)realloc(arg, new_cap);
                        if (grown == NULL) {
                            complete = 0;
                            break;
                        }
                        arg = grown;
                        cap = new_cap;
                    }
                    arg[len++] = (char)ch;
                } else {
                    char key[64];
                    arg[len] = 0;
                    snprintf(key, sizeof(key), "run_arg[%d]", count++);
                    radio_provenance_value(key, arg);
                    len = 0;
                }
            }
            if (ferror(fp)) complete = 0;
            if (len > 0) {
                char key[64];
                arg[len] = 0;
                snprintf(key, sizeof(key), "run_arg[%d]", count++);
                radio_provenance_value(key, arg);
            }
            free(arg);
        }
        fclose(fp);
    } else {
        complete = 0;
        radio_provenance_value("runtime_argv_source", "unavailable");
    }
#else
    complete = 0;
    radio_provenance_value("runtime_argv_source", "unavailable on this platform");
#endif
    radio_provenance_number("run_arg_count", (unsigned long long)count);
    radio_provenance_value("runtime_argv_complete", complete && count > 0 ? "yes" : "no");
    return complete && count > 0;
}

static void radio_provenance_cpu_model(void) {
#ifdef __APPLE__
    char model[512];
    size_t size = sizeof(model);
    if (sysctlbyname("machdep.cpu.brand_string", model, &size, NULL, 0) == 0 && size > 0) {
        model[sizeof(model) - 1] = 0;
        radio_provenance_value("runtime_cpu_model", model);
        return;
    }
#elif defined(__linux__)
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp != NULL) {
        char line[1024];
        while (fgets(line, sizeof(line), fp) != NULL) {
            if (strncmp(line, "model name", 10) == 0 || strncmp(line, "Hardware", 8) == 0) {
                char *value = strchr(line, ':');
                if (value != NULL) {
                    value++;
                    while (*value == ' ' || *value == '\t') value++;
                    value[strcspn(value, "\r\n")] = 0;
                    radio_provenance_value("runtime_cpu_model", value);
                    fclose(fp);
                    return;
                }
            }
        }
        fclose(fp);
    }
#endif
    radio_provenance_value("runtime_cpu_model", "unknown");
}

static unsigned long long radio_provenance_physical_memory(void) {
#ifdef __APPLE__
    uint64_t bytes = 0;
    size_t size = sizeof(bytes);
    if (sysctlbyname("hw.memsize", &bytes, &size, NULL, 0) == 0) return bytes;
#endif
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);
    if (pages > 0 && page_size > 0)
        return (unsigned long long)pages * (unsigned long long)page_size;
    return 0;
}

static void radio_provenance_rlimit(const char *name, int resource) {
    struct rlimit limit;
    char key[96];
    if (getrlimit(resource, &limit) != 0) return;
    snprintf(key, sizeof(key), "runtime_rlimit.%s.soft", name);
    if (limit.rlim_cur == RLIM_INFINITY) radio_provenance_value(key, "infinity");
    else radio_provenance_number(key, (unsigned long long)limit.rlim_cur);
    snprintf(key, sizeof(key), "runtime_rlimit.%s.hard", name);
    if (limit.rlim_max == RLIM_INFINITY) radio_provenance_value(key, "infinity");
    else radio_provenance_number(key, (unsigned long long)limit.rlim_max);
}

static void radio_print_provenance(void) {
    if (radio_provenance_printed) return;
    radio_provenance_printed = 1;
    /* tools/run_with_provenance.py is the generic path for standalone utilities.  If somebody uses
       it for a radiobase driver too, it has already verified the binary and emitted the same block. */
    if (getenv("RADIO_PROVENANCE_WRAPPER_EMITTED") != NULL) return;

    printf("# radio-provenance-v1 begin\n");
    radio_provenance_value("artifact", "solver-output");
    radio_provenance_value("provenance_complete",
                           RADIO_BUILD_PROVENANCE_COMPLETE ? "yes" : "no");
    radio_provenance_value("build_id", RADIO_BUILD_ID);
    radio_provenance_value("git_commit", RADIO_GIT_COMMIT);
    radio_provenance_value("git_identity_source", RADIO_GIT_IDENTITY_SOURCE);
    radio_provenance_value("git_source_dirty", RADIO_GIT_SOURCE_DIRTY);
    radio_provenance_value("git_worktree_dirty", RADIO_GIT_WORKTREE_DIRTY);
    radio_provenance_value("build_utc", RADIO_BUILD_UTC);
    radio_provenance_value("build_host", RADIO_BUILD_HOST);
    radio_provenance_value("build_uname", RADIO_BUILD_UNAME);
    radio_provenance_value("build_cwd", RADIO_BUILD_CWD);
    radio_provenance_value("compiler_version", RADIO_COMPILER_VERSION);
    radio_provenance_value("compiler_executable_sha256", RADIO_COMPILER_SHA256);
    radio_provenance_value("build_tool_sha256", RADIO_BUILD_TOOL_SHA256);
    radio_provenance_value("provenance_injection", RADIO_PROVENANCE_INJECTION);
    radio_provenance_number("compile_arg_count", RADIO_BUILD_ARGC);
    for (int i = 0; i < RADIO_BUILD_ARGC; i++) {
        char key[64];
        snprintf(key, sizeof(key), "compile_arg[%d]", i);
        radio_provenance_value(key, radio_provenance_build_argv[i]);
    }
    radio_provenance_number("build_env_count", RADIO_BUILD_ENV_COUNT);
    for (int i = 0; i < RADIO_BUILD_ENV_COUNT; i++) {
        char key[64];
        snprintf(key, sizeof(key), "build_env[%d].name", i);
        radio_provenance_value(key, radio_provenance_build_env_names[i]);
        snprintf(key, sizeof(key), "build_env[%d].value", i);
        radio_provenance_value(key, radio_provenance_build_env_values[i]);
    }
    radio_provenance_number("source_count", RADIO_SOURCE_COUNT);
    for (int i = 0; i < RADIO_SOURCE_COUNT; i++) {
        char key[64];
        snprintf(key, sizeof(key), "source[%d].path", i);
        radio_provenance_value(key, radio_provenance_source_paths[i]);
        snprintf(key, sizeof(key), "source[%d].sha256", i);
        radio_provenance_value(key, radio_provenance_source_sha256[i]);
    }
    radio_provenance_number("define.MAX_K", MAX_K);
    radio_provenance_number("define.MAX_N", MAX_N);
#ifdef RADIO_WORK_BUDGET
    radio_provenance_value("search_budget", "deterministic-accepted-prefixes");
    radio_provenance_number("search_work_units_per_nominal_second",
                            RADIO_BUDGET_UNITS_PER_SECOND);
#else
    radio_provenance_value("search_budget", "process-cpu-clock");
#endif

    char stamp[64] = "unknown";
    time_t now = time(NULL);
    struct tm utc;
    if (gmtime_r(&now, &utc) != NULL)
        strftime(stamp, sizeof(stamp), "%Y-%m-%dT%H:%M:%SZ", &utc);
    radio_provenance_value("run_utc", stamp);
    radio_provenance_runtime_argv();

    char cwd[4096];
    radio_provenance_value("runtime_cwd", getcwd(cwd, sizeof(cwd)) ? cwd : "unknown");
    char host[256] = "unknown";
    if (gethostname(host, sizeof(host)) != 0) strcpy(host, "unknown");
    host[sizeof(host) - 1] = 0;
    radio_provenance_value("runtime_host", host);
    struct utsname uts;
    if (uname(&uts) == 0) {
        radio_provenance_value("runtime_os", uts.sysname);
        radio_provenance_value("runtime_kernel_release", uts.release);
        radio_provenance_value("runtime_kernel_version", uts.version);
        radio_provenance_value("runtime_arch", uts.machine);
    } else {
        radio_provenance_value("runtime_os", "unknown");
        radio_provenance_value("runtime_kernel_release", "unknown");
        radio_provenance_value("runtime_kernel_version", "unknown");
        radio_provenance_value("runtime_arch", "unknown");
    }
    long cpus = sysconf(_SC_NPROCESSORS_ONLN);
    radio_provenance_number("runtime_logical_cpus", cpus > 0 ? (unsigned long long)cpus : 0);
    radio_provenance_number("runtime_physical_memory_bytes", radio_provenance_physical_memory());
    radio_provenance_number("runtime_pointer_bits", 8ULL * sizeof(void *));
    radio_provenance_cpu_model();
    radio_provenance_rlimit("CPU_seconds", RLIMIT_CPU);
    radio_provenance_rlimit("address_space_bytes", RLIMIT_AS);
    radio_provenance_rlimit("data_bytes", RLIMIT_DATA);
    radio_provenance_rlimit("stack_bytes", RLIMIT_STACK);
    radio_provenance_rlimit("open_files", RLIMIT_NOFILE);

    const char *safe_env[] = {
        "LANG", "LC_ALL", "TZ", "OMP_NUM_THREADS", "RADIO_RUNNER", "RADIO_RUN_LABEL",
        "RADIO_LIMIT_WALL_SECONDS", "RADIO_LIMIT_RSS_GIB",
        "RADIO_LIMIT_PHYSICAL_FOOTPRINT_GIB", "RADIO_RUN_CONTEXT", "TWO_SIDED_ONLY",
        "TRACE_INDEX", "BENCH_K", "NODECAP", "TIMECAP", "MINIMAL_K", "TOPDOWN", "PASSES",
        "VERIFY_THREADS", "VERIFY_MEMO_BITS", "VERIFY_PROGRESS_SECONDS", "ROOTS",
        "MINIMIZE_BEFORE_COLOR", "CERT_ONLY", "CERT_OUT", "RB_PLIABILITY_VERBOSE",
        "RADIO_PROBE_INIT", "REFUTE_THREADS", "REFUTE_PROGRESS_SECONDS", "REFUTE_MIN_K",
        "REFUTE_MAX_K", "REFUTE_MIN_PARTS", "REFUTE_MAX_PARTS", "REFUTE_STRIDE",
        "REFUTE_OFFSET"
    };
    for (size_t i = 0; i < sizeof(safe_env) / sizeof(safe_env[0]); i++) {
        const char *value = getenv(safe_env[i]);
        if (value != NULL) {
            char key[96];
            snprintf(key, sizeof(key), "runtime_env.%s", safe_env[i]);
            radio_provenance_value(key, value);
        }
    }
    printf("# radio-provenance-v1 end\n");
    fflush(stdout);
}

/* Clang/GCC run this after the C runtime is ready but before main.  Keeping the init() call too is
   intentional: the guard makes it a no-op, while nonstandard toolchains without constructor
   attributes still get provenance on the normal solver path.  The constructor covers usage errors
   and focused regressions which return without calling the expensive engine initializer. */
#if defined(__clang__) || defined(__GNUC__)
__attribute__((constructor))
static void radio_provenance_constructor(void) {
    radio_print_provenance();
}
#endif

void init(){
    radio_print_provenance();
    radio_default_search_context.work_clock = RADIO_WORK_CLOCK_ORIGIN;
    radio_default_search_context.cache_l1 = radio_default_cache_l1;
    memset(radio_default_cache_l1, 0, sizeof(radio_default_cache_l1));
#ifdef MEASURE_CACHE_L1
    atexit(print_cache_l1_stats);
#endif
    int i,pow,k,n;
    for (i=0,pow=1; i<= MAX_K; i++){
        power3[i]=pow;
        pow*=3;
    }
    init_singleton_majorization();
    
    for (i=0; i<=MAX_SBB; i++) {
        sbb_to_min_k[i] = i<=1?0:-1;   // -1 = not yet computed; minK() memoises into this
    }
    k=0;
    for(i=2;i<=MAX_N;i++) {
        sa_can[i] = MAX_K+1;
        while(k < MAX_K && saPairs(i)>=power3[k+1]) k++;
        sa_cant[i] = k;
        //      printf("can't solve %d in %d pairs = %d power3 = %d\n", i,k,saPairs(i),power3[k+1]);
    }
    memset(sb_cache_root, 0, sizeof(sb_cache_root));
    
    int n1, n2, sbb;
    sbb=0;
    sb_pairs[0]=0;
    sprintf(sbb_to_str[0],"0:0");
    sbb_lesser = (int **)malloc((MAX_SBB + 1) * sizeof(int *));
    sbb_greater = (int **)malloc((MAX_SBB + 1) * sizeof(int *));
    if (sbb_lesser == NULL || sbb_greater == NULL) {
        printf("\nout of memory - can't allocate sbb relation tables\n");
        exit(1);
    }
    sbb_lesser[0] = (int *)malloc(sizeof(int));
    sbb_greater[0] = (int *)malloc(sizeof(int));
    if (sbb_lesser[0] == NULL || sbb_greater[0] == NULL) {
        printf("\nout of memory - can't allocate sbb relation row 0\n");
        exit(1);
    }
    sbb_lesser[0][0] = 0;
    sbb_greater[0][0] = MAX_SBB + 1;
    int prod;
    //  printf("maxprod=%d\n", maxprod);
    for (prod =1; prod<=MAX_PROD;prod++) {
        max_sbb_for_pairs[prod] = sbb;
        //      printf("prod=%d\n", prod);
        for (n2 = MAX_N-1; n2 > prod/n2; n2--);
        for (; n2>0; n2--) {
//        for (n1=min(prod,MAX_N-1), n2=1; n1>=n2; n1--) {
//            if (n1>0) n2 = prod/n1;
            n1 = prod/n2;
            if (n1>=n2 && n1+n2<=MAX_N && n1*n2 == prod) {
                //           printf("n1=%d\n", n1);
                //              printf("n2=%d\n", n2);
                //  for (n1 = 1; n1 < MAX_N; n1++) {
                //      for (n2 = 1; n2 <= n1 && n1+n2 <= MAX_N; n2++) {
                n_to_sbb[n1][n2] = ++sbb;
                if (sbb>=MAX_SBB+1) {
                    printf ("sbb=%d, MAX_SBB=%d\n", sbb, MAX_SBB);
                    exit(6);
                }
                
                //                  printf("sbb=%d\n", sbb);
                
                sbb_to_n1[sbb]=n1;
                sbb_to_n2[sbb]=n2;
#if MAX_N < 32768
                sbb_dominance_key[sbb]=((uint32_t)n1 << 16) | (uint32_t)n2;
#endif
                sb_pairs[sbb]=n1*n2;
                max_sbb_for_pairs[prod] = sbb;
                sprintf(sbb_to_str[sbb],"%d:%d",n1,n2);
                
#ifdef DEBUG
                printf("sbb=%d (%s) pairs=%d\n", sbb, sbb_to_str[sbb], sb_pairs[sbb]);
                fflush(stdout);
#endif
                int c=0;
                int k1,k2;
                for (k1=n1; k1>0; k1--) {
                    for (k2=min(k1,n2); k2>0; k2--) {
                        c++;
                    }
                }
                sbb_lesser[sbb] = (int *)malloc((c + 1) * sizeof(int));
                if (sbb_lesser[sbb] == NULL) {
                    printf("\nout of memory - can't allocate sbb_lesser[%d]\n", sbb);
                    exit(1);
                }
                c=0;
                for (k1=n1; k1>0; k1--) {
                    for (k2=min(k1,n2); k2>0; k2--) {
                        sbb_lesser[sbb][c++]=getSbb(k1,k2);
                    }
                }
                //!!! must be sorted!!!!
                sort1(sbb_lesser[sbb], c);
                sbb_lesser[sbb][c++]=0; //terminator
            }
        }
    }
    
    if (sbb>MAX_SBB) {
        printf ("sbb=%d, MAX_SBB=%d\n", sbb, MAX_SBB);
        exit(7);
    }
    
    for (i=1; i<=MAX_SBB; i++) {
        int k = 0;
        int j;
        for (j=i; j<=MAX_SBB; j++) {
            if (sbb_to_n1[j]>=sbb_to_n1[i] && sbb_to_n2[j]>=sbb_to_n2[i]) {
                k++;
            }
        }
        sbb_greater[i] = (int *)malloc((k + 1) * sizeof(int));
        if (sbb_greater[i] == NULL) {
            printf("\nout of memory - can't allocate sbb_greater[%d]\n", i);
            exit(1);
        }
        k = 0;
        for (j=i; j<=MAX_SBB; j++) {
            if (sbb_to_n1[j]>=sbb_to_n1[i] && sbb_to_n2[j]>=sbb_to_n2[i]) {
                sbb_greater[i][k++]=j;
            }
        }
        // terminator
        sbb_greater[i][k++]=MAX_SBB + 1;
    }
    printf("initializing split table index\n");
    {
        size_t splits_count = MAX_SBB + 1;
        size_t splits_size = splits_count * sizeof(*sbb_splits);
        printf("split_index_size = %zu (level-lazy mode)\n", splits_size);
        sbb_splits = (split_levels **)calloc(splits_count, sizeof(*sbb_splits));
        if (sbb_splits == NULL){
            printf("\nout of memory - can't allocate split table index\n");
            exit(1);
        }
    }
#ifdef SPLIT_STATS
    atexit(report_split_stats);
#endif
    printf("\ninit done\n");
    fflush(stdout);
}
