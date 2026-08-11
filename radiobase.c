#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
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

#define CACHE_ONLY 1
#define NO_DEADLINE 2
#define FAST_ONLY 3
#define SUBSPLIT_DEADLINE (clock() + CLOCKS_PER_SEC * 1)

/* A bounded search must contribute new negative facts before it may bail.  This is the depth-first
   progress guarantee: without it, a whole pass can consume its budget in prefixes and return MAYBE
   without leaving anything reusable in the cache.  At exactly the minimum count, keep extending
   the grace window so the dive can produce another fact; once it moves beyond that count, expiry
   is enforceable.  This is the historical state machine, deliberately restored. */
static int deadline_expired(clock_t *deadline, clock_t start, clock_t now,
                            long long cant_solve_count, long long cant_solve_count_min) {
    if (cant_solve_count < cant_solve_count_min) return FALSE;
    if (cant_solve_count == cant_solve_count_min) {
        clock_t new_deadline = now + (now - start) * 5;
        if (new_deadline > *deadline) *deadline = new_deadline;
    }
    return now > *deadline;
}

static clock_t child_deadline_for_pass(int pass, clock_t deadline) {
    return pass < 2 ? deadline : NO_DEADLINE;
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
   iteration order and `ords` is the step, so the lookup is a single indexed load. */
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
int minK(int);
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
typedef struct {
    uint32_t hash;
    front_point part[CACHE_L1_MAX_PARTS];
    uint8_t size;
    uint8_t k;
    uint8_t verdict_plus_one;
    uint8_t padding;
} cache_l1_entry;
static cache_l1_entry cache_l1[CACHE_L1_SIZE];
#ifdef MEASURE_CACHE_L1
static unsigned long long cache_l1_queries;
static unsigned long long cache_l1_eligible;
static unsigned long long cache_l1_hits;
static unsigned long long cache_l1_stores;
static unsigned long long cache_l1_replacements;
static void print_cache_l1_stats(void) {
    fprintf(stderr,
            "CACHE_L1 queries=%llu eligible=%llu hits=%llu stores=%llu replacements=%llu\n",
            cache_l1_queries, cache_l1_eligible, cache_l1_hits, cache_l1_stores,
            cache_l1_replacements);
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
    size_t bytes = sizeof(front_vector) + (size_t)cap * sizeof(front_point);
    front_vector *v = (front_vector *)malloc(bytes);
    if (v == NULL) {
        printf("\nout of memory allocating Pareto front\n");
        exit(1);
    }
    v->len = 2;
    v->cap = cap;
    v->sbb[0] = (front_point)a;
    v->sbb[1] = (front_point)b;
    front_handles[handle] = v;
    front_alloc_count++;
    front_alloc_size += (long long)bytes;
    return FRONT_VECTOR_TAG | handle;
}

static void release_front_vector(uint32_t descriptor) {
    if (!(descriptor & FRONT_VECTOR_TAG)) return;
    uint32_t handle = descriptor & FRONT_HANDLE_MASK;
    front_vector *v = front_vector_for(descriptor);
    size_t bytes = sizeof(front_vector) + (size_t)v->cap * sizeof(front_point);
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
    size_t oldbytes = sizeof(front_vector) + (size_t)oldcap * sizeof(front_point);
    size_t newbytes = sizeof(front_vector) + (size_t)newcap * sizeof(front_point);
    front_vector *v = (front_vector *)realloc(old, newbytes);
    if (v == NULL) {
        printf("\nout of memory growing Pareto front\n");
        exit(1);
    }
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

static int front_has_lesser_equal(uint32_t descriptor, int sbb) {
    if (descriptor == 0) return 0;
    if (!(descriptor & FRONT_VECTOR_TAG)) return part_greater_equal(sbb, (int)descriptor);
    front_vector *v = front_vector_for(descriptor);
    for (uint32_t i = 0; i < v->len; i++)
        if (part_greater_equal(sbb, (int)v->sbb[i])) return 1;
    return 0;
}

static int add_front_descriptor(uint32_t *field, int positive, int sbb) {
    uint32_t descriptor = *field;
    if (descriptor == 0) {
        *field = (uint32_t)sbb;
        return 1;
    }
    if (!(descriptor & FRONT_VECTOR_TAG)) {
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
        if (!new_covers_old) v->sbb[w++] = v->sbb[i];
    }
    if (w == 0) {
        release_front_vector(descriptor);
        *field = (uint32_t)sbb;
        return 1;
    }
    if (w == v->cap) v = grow_front_vector(descriptor);
    v->sbb[w++] = (front_point)sbb;
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

static inline __attribute__((always_inline)) int checkCacheTrie(int *sb, int size, int k) {
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
        if (front_has_lesser_equal(negative, sbb)) return FALSE;
        if (i == size - 1 && front_has_greater_equal(positive, sbb)) return TRUE;
        if (b == NULL) return MAYBE;
        if ((uint32_t)sbb >= b->width) return MAYBE;
        node = b->slot[sbb];
    }
    return MAYBE;
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
    const int *sb, int size, int k, cache_l1_entry **entry_out, uint32_t *hash_out) {
#ifdef MEASURE_CACHE_L1
    cache_l1_queries++;
#endif
    *entry_out = NULL;
    if ((unsigned)size <= CACHE_L1_MAX_PARTS) {
#ifdef MEASURE_CACHE_L1
        cache_l1_eligible++;
#endif
        uint32_t hash = cache_l1_hash(sb, size, k);
        cache_l1_entry *entry = &cache_l1[hash & (CACHE_L1_SIZE - 1u)];
        *entry_out = entry;
        *hash_out = hash;
        if (entry->verdict_plus_one != 0 && entry->hash == hash && entry->size == (uint8_t)size &&
            entry->k == (uint8_t)k) {
            int i = 0;
            while (i < size && entry->part[i] == (front_point)sb[i]) i++;
            if (i == size) {
#ifdef MEASURE_CACHE_L1
                cache_l1_hits++;
#endif
                return (int)entry->verdict_plus_one - 1;
            }
        }
    }
    return MAYBE;
}

static inline __attribute__((always_inline)) void cache_l1_store(
    cache_l1_entry *entry, uint32_t hash, const int *sb, int size, int k, int verdict) {
    if (entry != NULL && verdict != MAYBE) {
#ifdef MEASURE_CACHE_L1
        cache_l1_stores++;
        cache_l1_replacements += entry->verdict_plus_one != 0;
#endif
        entry->hash = hash;
        for (int i = 0; i < size; i++) entry->part[i] = (front_point)sb[i];
        entry->size = (uint8_t)size;
        entry->k = (uint8_t)k;
        entry->verdict_plus_one = (uint8_t)(verdict + 1);
    }
}

/* Kept as the cache-query API for regression tools and small diagnostic drivers.  The main solver
   probes L1 separately so an exact hit can avoid its bundled-majorization checks as well. */
int checkCache(int *sb, int size, int k) {
    cache_l1_entry *entry;
    uint32_t hash = 0;
    int verdict = cache_l1_probe(sb, size, k, &entry, &hash);
    if (verdict == MAYBE) verdict = checkCacheTrie(sb, size, k);
    cache_l1_store(entry, hash, sb, size, k, verdict);
    return verdict;
}

#ifdef DEBUG_CACHE_ONLY
#undef DEBUG
#undef debug_printf
#define debug_printf(...) /* Nothing */
#endif

// returns >0 if sbb1 is harder to solve than sbb2, <0 if sbb2 is harder, 0 if equal
int compare_solvability(int sbb1, int sbb2) {
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
        int mink1 = minK(sbb1);
        int mink2 = minK(sbb2);
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

/* Necessary condition for an arbitrary Sb state.  Replace each oriented part (n:m), n >= m,
   by m disjoint singleton stars (n:1).  This is a vertex-splitting lift of the original graph:
   pulling every test back to all clones preserves every edge transcript, so a strategy for the
   original would solve the lift.  The Singleton Majorization Theorem then decides the lift.

   Sort only the distinct input parts by n, not the expanded sequence.  Long states have many
   repeated stars but few distinct parts, so this avoids making the stronger theorem expensive. */
int star_expansion_majorization_can_solve(int *sb, int size, int k) {
    int by_n[size];
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

    long long left_prefix = 0;
    int rank = 0;
    int right_len = singleton_base_len[k];
    int right_total = singleton_base_prefix[k][right_len - 1];
    for (i = 0; i < size; i++) {
        int copies = sbb_to_n2[by_n[i]];
        int n = sbb_to_n1[by_n[i]];
        while (copies-- > 0) {
            left_prefix += n;
            int right_prefix = rank < right_len ? singleton_base_prefix[k][rank] : right_total;
            if (left_prefix > right_prefix) return FALSE;
            rank++;
        }
    }
    return TRUE;
}

int get_max_sbb(int n1, int n2, int n3, int n4) {
    int sbb1 = getSbb(n1, n2);
    int sbb2 = getSbb(n3, n4);
    return (compare_solvability(sbb1, sbb2) > 0) ? sbb1 : sbb2;
}

long long cant_solve_count=0;


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
#define RB_TRIGGER 10000000LL      /* arm the prune once a state has cost this many candidates */
static int rb_on = 0, rb_cap = 0, rb_words = 0, rb_P = 0;
static unsigned long long *rb_bits[17];
static short *rb_mx[17];
static int rb_mrem[17];
static long long rb_tested = 0, rb_pruned = 0;

static void rb_free(void) {
    int i;
    for (i = 0; i <= rb_P; i++) { free(rb_bits[i]); free(rb_mx[i]); rb_bits[i] = NULL; rb_mx[i] = NULL; }
}
static void rb_build(splits **sa, int *tmpp, int P, int cap) {
    int i, c, r0, w, W = cap + 1;
    rb_cap = cap; rb_P = P; rb_words = (W + 63) / 64;
    rb_mrem[P] = 0;
    for (i = P - 1; i >= 0; i--) rb_mrem[i] = rb_mrem[i+1] + sb_pairs[tmpp[i]];
    for (i = 0; i <= P; i++) {
        rb_bits[i] = (unsigned long long *)calloc((size_t)W * rb_words, sizeof(unsigned long long));
        rb_mx[i] = (short *)malloc((size_t)W * W * sizeof(short));
    }
    rb_bits[P][0] = 1ULL;                                  /* (0,0) reachable by the empty suffix */
    for (i = P - 1; i >= 0; i--) {
        for (c = 0; c < sa[i]->size; c++) {
            int *sp = sa[i]->splitsl[c];
            int k0 = sb_pairs[sp[0]], k2 = sb_pairs[sp[3]];
            if (k0 > cap || k2 > cap) continue;
            int ws = k2 >> 6, bs = k2 & 63;                /* shift along r2, within each row */
            for (r0 = 0; r0 + k0 <= cap; r0++) {
                unsigned long long *src = rb_bits[i+1] + (size_t)r0 * rb_words;
                unsigned long long *dst = rb_bits[i]   + (size_t)(r0 + k0) * rb_words;
                for (w = rb_words - 1; w >= 0; w--) {
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
            unsigned long long bit = rb_bits[i][(size_t)r0*rb_words + (r2>>6)] >> (r2 & 63) & 1ULL;
            short b = bit ? (short)(r0 + r2) : -1;
            if (r0 && rb_mx[i][(size_t)(r0-1)*W + r2] > b) b = rb_mx[i][(size_t)(r0-1)*W + r2];
            if (r2 && rb_mx[i][(size_t)r0*W + r2-1] > b) b = rb_mx[i][(size_t)r0*W + r2-1];
            rb_mx[i][(size_t)r0*W + r2] = b;
        }
    }
}
static void rb_release(void) {
    fprintf(stderr, "\nREACH: %lld tested, %lld pruned (%.1f%%)\n",
            rb_tested, rb_pruned, rb_tested ? 100.0*rb_pruned/rb_tested : 0.0);
    rb_free(); rb_on = 0; rb_tested = rb_pruned = 0;
}
static inline int rb_dead(int nexti, int p0, int p1, int p2) {
    int W = rb_cap + 1;
    int a = rb_cap - p0, cc = rb_cap - p2;
    long long need = (long long)p1 + rb_mrem[nexti] - rb_cap;
    rb_tested++;
    if (a < 0 || cc < 0) { rb_pruned++; return 1; }
    if (rb_mx[nexti][(size_t)a*W + cc] >= need) return 0;
    rb_pruned++; return 1;
}

int canSolveB(int *sb, int size, int k, clock_t parent_deadline){
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
    if (size>1) sort1(tmp, size);
    int query_size = size;
    cache_l1_entry *l1_entry;
    uint32_t l1_hash = 0;
    int ck = cache_l1_probe(tmp, size, k, &l1_entry, &l1_hash);
    if (ck == TRUE || ck == FALSE) return ck;
    if (singleton_size == size) {
        // Singleton states are decided exactly by majorization against G_k.
        ck = singleton_majorization_can_solve(tmp, size, k);
        cache_l1_store(l1_entry, l1_hash, tmp, size, k, ck);
        return ck;
    }
    // Apply Singleton Majorization to the full star expansion: (n:m) becomes m copies of (n:1).
    // This strictly dominates the old one-copy downgrade, because it contains that downgraded
    // singleton sequence and adds only nonnegative entries.  It is a necessary condition, not an
    // ordering heuristic; see docs/theorems/singleton-majorization.md.
    if (!star_expansion_majorization_can_solve(tmp, size, k)) {
        cache_l1_store(l1_entry, l1_hash, tmp, size, k, FALSE);
        return FALSE;
    }
    //check cache
    ck = checkCacheTrie(tmp, size, k);
    cache_l1_store(l1_entry, l1_hash, tmp, size, k, ck);
    //	printf("got from cache %d\n", ck);
    if (parent_deadline == CACHE_ONLY || ck == TRUE || ck == FALSE) {
//        debug_printf("returning ck=%d\n", ck);
        return ck;
    }
    
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
    splitsarr[0] = prepare_splits(tmp[0], k, size > 1);
    //full search
    clock_t start = clock();
    clock_t progress = start + PROGRESS_INTERVAL;
    
    /* Reachability is armed by observed cost, not by a shape signature. A state that has already
       burned RB_TRIGGER candidate evaluations has earned the ~1-2 ms the tables take to build, and
       cheap states - the overwhelming majority - never pay anything. That also avoids privileging
       the 8-part near-saturated shape: any state that turns out expensive gets the prune. */
    int rb_here = 0;
    long long cant_solve_count_min = cant_solve_count + 1; // min progress before bailing out
    clock_t deadline = 0;
    if (parent_deadline == NO_DEADLINE) {
        deadline = start + CLOCKS_PER_SEC * (1000);
    } else {
//        if (start > ) return MAYBE;
//        int deadline_ratio = size;
        int deadline_ratio = DEADLINE_RATIO;
        if (parent_deadline > start && parent_deadline - start > CLOCKS_PER_SEC * MIN_DEADLINE * deadline_ratio) {
            deadline = start + ((parent_deadline - start) / deadline_ratio);
        } else {
            deadline = start + CLOCKS_PER_SEC * MIN_DEADLINE + 300;
        }
    }
    
    //    printf("k=%d parent_deadline=%llu start=%llu deadline=%llu\n", k, parent_deadline, start, deadline);
    
    int cont2=1;
    int skipped_some;
    int pass = 0;
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
            fast_solve = TRUE && size > 2 && k >= FAST_MIN_K;
        }
        
        // Deadlines restored 2026-08-04 after disabling them trapped a real run. They are
        // the only escape from an intractable subtree: Sb(112:80) in 9 - a state with a
        // KNOWN solution - sank 39 minutes into one 13-part k=5 node of mass 243 = 3^5
        // (exactly information-tight, so nothing prunes), evaluating 119 billion split
        // combinations to clear 1 of its 52 splits.
        //
        // Removing them bought nothing. A printed `can't solve` is emitted only when
        // !skipped_some, so negatives are exhaustive with deadlines in place - the
        // "guarantee" I removed them for was already there. And a NO_DEADLINE root
        // iteratively deepens (`deadline += deadline - start`) until it concludes, so top
        // level answers stay definitive. Measured cost on the k=9 ladder: 266s vs 267s with
        // identical verdicts.
        int no_deadline = /*size <=2 || */(pass==1 && size <= 4) || parent_deadline == NO_DEADLINE;
        
//        int no_deadline = (pass==1);
        
        skipped_some = 0;
        totalsplits=0;
        skiptop = 0;
        cont=1;
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
        splitindex[0] = splitsarr[0]->size;
        HOIST_ORDER(0);
        
#ifdef DEBUG1
        clock_t t = clock();
        printf("solving in %d pass=%d ", k, pass);
        printf("deadline=%lu ", deadline>t ? (deadline - t + CLOCKS_PER_SEC) / CLOCKS_PER_SEC : 0);
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
        
        
        clock_t child_deadline = child_deadline_for_pass(pass, deadline);
        clock_t middle_child_deadline = child_deadline;

        
//        clock_t child_deadline = (parent_deadline == NO_DEADLINE && size < 3)? NO_DEADLINE : deadline;
//        clock_t middle_child_deadline = (parent_deadline == NO_DEADLINE && size <=3)? NO_DEADLINE : deadline;

//
//        clock_t child_deadline = (parent_deadline == NO_DEADLINE && size < 3)? NO_DEADLINE : deadline;
//        clock_t middle_child_deadline = (parent_deadline == NO_DEADLINE && size == 1)? NO_DEADLINE : deadline;
//
//        clock_t child_deadline = (parent_deadline == NO_DEADLINE && size == 1)? NO_DEADLINE : deadline;
//        clock_t middle_child_deadline = deadline;

//        clock_t child_deadline = NO_DEADLINE;
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
                    if (!skipped_some) {
                        cont2=0; // really can't solve
//                    } else if (parent_deadline==FAST_ONLY) {
//                        cache(tmp, size, MAYBE_SLOW, k, pairs);
//                        return MAYBE;
                    } else {
                        if (parent_deadline == NO_DEADLINE) {
                            // double deadline
                            deadline += (deadline - start);
                        } else {
                            // do not bail out until you make at least some progress
//                            if (clock()>deadline && cant_solve_count>=cant_solve_count_min) return MAYBE;
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

            while (s[4]<k) {
                debug_printf("checking split solvability for %s -> [%d, %d], before: s[4]=%d s[5]=%d\n", sbb_to_str[tmp[i]], s[6], s[7], s[4], s[5]);
                int kk = s[4];
                int dd = size > 1 ? deadline : CACHE_ONLY;
                int ttt = canSolveB(s, 1, kk, dd);
                if (ttt==TRUE) {
                    ttt = canSolveB(s+3, 1, kk, dd);
                    if (ttt==TRUE) {
                        ttt = canSolveB(s+1, 2, kk, dd);
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
                    /* Do not poll the deadline while this is still only a partial split.  In
                       pass 2 the first complete candidate is the handoff to an unresolved child
                       with NO_DEADLINE.  Bailing here bypasses that depth-first dive and can make
                       an expensive pass return MAYBE without producing a single reusable fact. */
                    /* `>=`, not `==`. rb_on is global, so only one state holds the tables at a time; with
                       equality a state whose trigger instant fell while another was armed lost its
                       only chance and ran the rest unpruned. Arming later is never worse than never
                       arming - the prune is a performance device, not a correctness one - so this
                       is safe. NOTE: this was originally changed while chasing a 27-minute cold
                       monster run that turned out to be a cold-vs-warm comparison error, not a
                       race. The fragility is real but has not been observed to fire. */
                    if (!rb_here && !rb_on && totalsplits >= RB_TRIGGER
                        && size >= 4 && power3[k_1] < RB_MAXCAP) {
                        int ri;
                        /* Reachability needs every suffix at once.  This is deliberately the
                           only bulk materialisation path; it runs only after the state has paid
                           enough search cost to arm the accelerator. */
                        for (ri = 0; ri < size; ri++) {
                            if (splitsarr[ri] == NULL)
                                splitsarr[ri] = ensure_splits(tmp[ri], k);
                        }
                        rb_on = rb_here = 1;
                        rb_build(splitsarr, tmp, size, power3[k_1]);
                    }
                    int p0 = sb0p[i] = sb_pairs[sb0[i] = s[0]] + (i>0?sb0p[i-1]:0);
                    int p1 = sb1p[i] = sb_pairs[sb1[i*2] = s[1]] + sb_pairs[sb1[i*2+1] = s[2]] + (i>0?sb1p[i-1]:0);
                    int p2 = sb2p[i] = sb_pairs[sb2[i] = s[3]] + (i>0?sb2p[i-1]:0);
                    
                    int cs0, cs1, cs2;
                    
#ifdef DEBUG
                    printSb(sb0, i+1);
                    printSb(sb1, 2*i+2);
                    printSb(sb2, i+1);
                    
                    debug_printf(" i=%d p0=%d p1=%d p2=%d\n", i, p0,p1,p2);
#endif
                    if ((p0 <= max_pairs_1) && (p1 <= max_pairs_1) && (p2 <= max_pairs_1)
                        && !(rb_here && i + 1 < size && rb_dead(i + 1, p0, p1, p2))
                        && (cs0 = canSolveB(sb0, i+1, k_1, CACHE_ONLY))
                        && (cs2 = canSolveB(sb2, i+1, k_1, CACHE_ONLY))
                        && (cs1 = canSolveB(sb1, (i+1) * 2, k_1, CACHE_ONLY))
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
                            if (cant_solve_count >= cant_solve_count_min
                                && (cs0 != TRUE || cs1 != TRUE || cs2 != TRUE)) {
                                clock_t t = clock();
                                if (deadline_expired(&deadline, start, t, cant_solve_count,
                                                     cant_solve_count_min)) {
                                    if (no_deadline) {
                                        //                                    deadline=0;  // now do full solution
                                        // double deadline
//                                        deadline+= (deadline - start);
                                        // bump deadline
                                        deadline = t + 10 * CLOCKS_PER_SEC;
//                                        cont=0;
//                                        break;
                                    } else {
                                        { if (rb_here) rb_release(); return MAYBE; }
                                    }
                                }
                                if (t >= progress) {
                                    printf("still solving in %d pass=%d fast_solve=%d ", k, pass, fast_solve);
                                    printSb(tmp, size);
                                    printf(" trying ");
                                    printSb(sb0, size);
                                    printSb(sb1, size2);
                                    printSb(sb2, size);
                                    printf(" elapsed %lu/%lu left=%d/%d totalsplits=%llu\n", (t - start)/CLOCKS_PER_SEC, (deadline - start) / CLOCKS_PER_SEC, splitindex[0], splitsarr[0]->size, totalsplits);
                                    fflush(stdout);
                                    progress = t + PROGRESS_INTERVAL;
                                }
                            }
                            clock_t cd = (/*size<=2 && */ totalsplits<5) ? NO_DEADLINE : child_deadline;
                            if ((cs0 = (cs0==MAYBE?canSolveB(sb0, i+1, k_1, cd):cs0)) != TRUE) {
                                if (cs0 != FALSE)
                                    skipped_some = 1;
                            } else if ((cs2 = (cs2==MAYBE?canSolveB(sb2, i+1, k_1, cd):cs2)) != TRUE) {
                                if (cs2 != FALSE)
                                    skipped_some = 1;
                            } else if((cs1 = (cs1==MAYBE?canSolveB(sb1, (i+1) * 2, k_1, cd):cs1))  != TRUE) {
                                if (cs1!=FALSE)
                                    skipped_some = 1;
                            } else {
                                //can solve
                                canSolve=TRUE;
                                cont=0;
                                cont2=0;
                                break;
                            }
                        } else {
                            i++;
                            /* `rb_build` may already have materialised this table, but FAST is
                               intentionally prepared only when ordinary search reaches it. */
                            splitsarr[i] = prepare_splits(tmp[i], k, size > 1);
                            if (i>max_solvable_maybe) {
                                max_solvable_maybe = i;
                                debug_printf("max_solvable_maybe=%d\n", max_solvable_maybe);
                            }
                            splitindex[i] = splitsarr[i]->size;
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
                                   evaluations than the gap heuristic below. */
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
        
    } else if (skipped_some) {
        if (rb_here) rb_release();
        return MAYBE;
    } else {
        cant_solve_count++;
        printf("can't solve ");
        if (max_solvable_maybe + 1 < size) {
            debug_printf("max_solvable_maybe=%d\n", max_solvable_maybe);
            printf("size=%d/", size);
            size = max_solvable_maybe + 1;
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
    clock_t t = clock()-start;
    clock_t s = t/CLOCKS_PER_SEC;
    if (s>0)
        printf(" took %ld", s);
    else
        printf(" took 0.%03ld", t * 1000/CLOCKS_PER_SEC);
    if (rb_here) rb_release();
    printf(" totalsplits=%llu pass=%d fast_solve=%d", totalsplits, pass, fast_solve);
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
    cache_l1_store(l1_entry, l1_hash, tmp, query_size, k, canSolve);
    cache(tmp, size, canSolve, k, pairs);
    //    fflush(stdout);
    printf("\n");
#ifndef OPT_2
    fflush(stdout);
#endif
    return canSolve;
}

int sbb_to_min_k[MAX_SBB+1];

int minK(int sbb) {
    int kk = sbb_to_min_k[sbb];
    if (kk<0) {
        debug_printf("computing min_k for %s...\n", sbb_to_str[sbb]);
        kk=1;
        int rr;
        while ((rr = canSolveB(&sbb, 1, kk, clock() + CLOCKS_PER_SEC * 1000)) == TRUE) kk++;
        debug_printf("min_k=%d for %s...\n", kk, sbb_to_str[sbb]);
        if (rr == FALSE) sbb_to_min_k[sbb]=kk; // if we got maybe, assume false, but do not memorize
        debug_printf("cached min_k=%d for %s...\n", kk, sbb_to_str[sbb]);
    }
    return kk;
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

splits *prepare_splits(int sbb, int k, int need_fast) {
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
                int sbb1 = get_max_sbb(m1, n2 - m2, n1 - m1, m2);
                if ((m2 == 0 || compare_solvability(
                         sbb1, get_max_sbb(m1, n2 - m2 + 1, n1 - m1, m2 - 1)) <= 0)
                    && (m2 == n2 || compare_solvability(
                         sbb1, get_max_sbb(m1, n2 - m2 - 1, n1 - m1, m2 + 1)) <= 0)) {
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
        "RADIO_PROBE_INIT"
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
