/* Exhaustive two-cut Pareto-prefix census and fixed-dimension upgrade corpus.
 *
 * For every proven one-part Pareto root at ROOT_K this tool:
 *
 *   1. enumerates every winning first cut;
 *   2. enumerates every winning cut of its labelled mixed (two-lineage) child;
 *   3. retains the labelled mixed (four-lineage) grandchild;
 *   4. deduplicates the corresponding canonical solver states;
 *   5. explores the complete upward-closed solvable region above every effective descendant in
 *      its own dimension and retains every componentwise-maximal endpoint; and
 *   6. enumerates every winning top-level cut of every endpoint.
 *
 * The cartesian enumerator is exhaustive, but uses only sound pruning already available in
 * radiobase.c: theorem-admissible local split tables, the counting bound, and exact negative
 * cache dominance on partial children.  A cache miss at a complete cut is resolved with
 * NO_DEADLINE.  Square components are expanded back across their shore-swap automorphism so the
 * lineage output contains every labelled cut, not merely the solver's normalized representative.
 *
 * The upgrade is global at the residual level, rather than parent-conditioned: both coordinates
 * of every normalized component may grow.  Multi-source traversal visits every solvable canonical
 * superstate above the census seeds without reviving empty lineages or padding shorter states.
 * Removed 1:1 lineages remain attached as a scalar information-capacity reserve.  A retained
 * endpoint has had every distinct unit successor rejected exactly or by the proved one-component
 * frontier, so the resulting antichain is complete in that fixed effective dimension and reserve.
 *
 * Usage:
 *
 *   tools/build_radio.py -O3 -DMAX_K=8 -DMAX_N=768 \
 *       tools/pareto_prefix_census.c -o pareto_prefix_census
 *   ./pareto_prefix_census CACHE data/pareto_sb.csv ROOT_K \
 *       [MAX_UPGRADE_STATES [ROOT_LOG [EXACT_CACHE]]]
 *
 * ROOT_LOG may contain the raw one-part `result in K can solve ... with [...]` records of a prior
 * exhaustive full-solution run.  Those cuts are re-verified exactly before use.  Supplying them
 * avoids replaying a huge negative oracle merely to rediscover the already archived first level;
 * any frontier root absent from the log is still enumerated normally.  Use `-` as ROOT_LOG when an
 * EXACT_CACHE is supplied but the first level should be independently re-enumerated.  EXACT_CACHE
 * is an optional full cache file indexed by exact canonical state equality only; unlike CACHE it does not
 * materialize dominance closure.  This is useful when a large audited historical oracle is too
 * costly to replay into the trie.  Misses still fall through to the current exact solver.
 *
 * Output records beginning CENSUS are stable, tab-separated machine records.  Ordinary solver
 * verdicts may appear between them when a warm cache lacks a fact; those are useful provenance,
 * but parsers should select only /^CENSUS\t/.
 */

static int census_exact_lookup(const int *sb, int size, int k);
#define RADIO_EXTERNAL_EXACT_LOOKUP(sb, size, k) census_exact_lookup((sb), (size), (k))
#include "../radiobase.c"

#include <stdint.h>

#define CENSUS_MAX_PARTS 4
#define CENSUS_MAX_CHILD_PARTS 8
#define CENSUS_MAX_CANON_PARTS 8
#define TARGET_HASH_BITS 20
#define TARGET_HASH_SIZE (1u << TARGET_HASH_BITS)
#define EXACT_MAX_PARTS 16

typedef struct {
    int u;
    int v;
} LabelPart;

typedef struct {
    int size;
    int sb[CENSUS_MAX_CANON_PARTS];
} CanonState;

typedef struct {
    CanonState state;
    int units;
    unsigned long long occurrences;
    unsigned long long opposed_occurrences;
    unsigned long long strict_first_occurrences;
    unsigned long long root_mask;
    int upgrade_node;
} TargetState;

typedef struct {
    CanonState state;
    int units;
    unsigned char is_seed;
    unsigned char is_endpoint;
} UpgradeNode;

typedef struct EnumContext EnumContext;
typedef void (*WinnerCallback)(EnumContext *ctx);

struct EnumContext {
    int k;
    int size;
    LabelPart part[CENSUS_MAX_PARTS];
    int take_u[CENSUS_MAX_PARTS];
    int take_v[CENSUS_MAX_PARTS];
    int selected[CENSUS_MAX_PARTS];
    int complement[CENSUS_MAX_PARTS];
    int mixed[CENSUS_MAX_CHILD_PARTS];
    splits *options[CENSUS_MAX_PARTS];
    long long selected_mass[CENSUS_MAX_PARTS + 1];
    long long mixed_mass[CENSUS_MAX_PARTS + 1];
    long long complement_mass[CENSUS_MAX_PARTS + 1];
    unsigned long long prefixes;
    unsigned long long local_options;
    unsigned long long cap_pruned;
    unsigned long long cache_pruned;
    unsigned long long complete;
    unsigned long long exact_queries;
    unsigned long long winners;
    WinnerCallback winner;
    void *opaque;
};

typedef struct {
    int root_index;
    int root_n;
    int root_m;
    int first_u;
    int first_v;
    int strict_first;
} SecondContext;

typedef struct {
    int take_u[2];
    int take_v[2];
} SecondCut;

typedef struct {
    /* Keep both components, including unit components: they still have labelled cuts even though
       canSolveB later erases them from its canonical state. */
    CanonState state;
    SecondCut *cuts;
    size_t cuts_len;
    size_t cuts_cap;
    unsigned long long prefixes;
    unsigned long long complete;
    unsigned long long cap_pruned;
    unsigned long long cache_pruned;
    unsigned long long exact_queries;
} SecondMemo;

typedef struct {
    uint64_t hash;
    uint32_t offset;
    uint16_t size;
    uint8_t k;
    uint8_t verdict;
} ExactSlot;

static int root_k;
static int residual_k;
static int frontier[MAX_K + 1][MAX_N / 2 + 1];

static TargetState *targets;
static size_t targets_len;
static size_t targets_cap;
static uint32_t target_hash[TARGET_HASH_SIZE];

static UpgradeNode *upgrade_nodes;
static size_t upgrade_len;
static size_t upgrade_cap;
static uint32_t *upgrade_hash;
static size_t upgrade_hash_cap;
static size_t max_upgrade_states = 2000000;
static unsigned long long upgrade_representation_blocked;
static unsigned long long upgrade_component_frontier_rejected;

static unsigned long long roots_seen;
static unsigned long long first_winners;
static unsigned long long first_strict_winners;
static unsigned long long roots_with_strict_first;
static unsigned long long second_invocations;
static unsigned long long second_unique_states;
static unsigned long long second_memo_hits;
static unsigned long long second_winners;
static unsigned long long second_opposed_winners;
static unsigned long long first_prefixes;
static unsigned long long second_prefixes;
static unsigned long long first_complete;
static unsigned long long second_complete;
static unsigned long long supplied_root_winners[512];
static unsigned long long supplied_root_strict[512];

static SecondMemo *second_memos;
static size_t second_memos_len;
static size_t second_memos_cap;

static ExactSlot *exact_slots;
static size_t exact_slots_cap;
static size_t exact_slots_len;
static uint16_t *exact_parts;
static size_t exact_parts_len;
static size_t exact_parts_cap;
static unsigned long long exact_hits;
static unsigned long long exact_misses;

static uint64_t exact_hash(const int *sb, int size, int k) {
    uint64_t h = UINT64_C(1469598103934665603) ^ (uint64_t)(unsigned)k;
    h = (h ^ (uint64_t)(unsigned)size) * UINT64_C(1099511628211);
    for (int i = 0; i < size; i++)
        h = (h ^ (uint64_t)(unsigned)sb[i]) * UINT64_C(1099511628211);
    h ^= h >> 32;
    return h ? h : 1;
}

static int census_exact_lookup(const int *sb, int size, int k) {
    if (!exact_slots || size > EXACT_MAX_PARTS) return MAYBE;
    uint64_t hash = exact_hash(sb, size, k);
    size_t slot = (size_t)hash & (exact_slots_cap - 1);
    while (exact_slots[slot].hash) {
        ExactSlot *entry = &exact_slots[slot];
        if (entry->hash == hash && entry->size == size && entry->k == k) {
            const uint16_t *parts = exact_parts + entry->offset;
            int equal = 1;
            for (int i = 0; i < size; i++) {
                if (parts[i] != (uint16_t)sb[i]) { equal = 0; break; }
            }
            if (equal) {
                exact_hits++;
                return entry->verdict == 1 ? TRUE : FALSE;
            }
        }
        slot = (slot + 1) & (exact_slots_cap - 1);
    }
    exact_misses++;
    return MAYBE;
}

static void exact_parts_reserve(size_t need) {
    if (need <= exact_parts_cap) return;
    size_t next = exact_parts_cap ? exact_parts_cap : (1u << 20);
    while (next < need) next *= 2;
    uint16_t *grown = realloc(exact_parts, next * sizeof(*grown));
    if (!grown) { fprintf(stderr, "out of memory growing exact-fact parts\n"); exit(3); }
    exact_parts = grown;
    exact_parts_cap = next;
}

static void exact_insert(int *sb, int size, int k, int verdict) {
    if (size < 1 || size > EXACT_MAX_PARTS || k < 0 || k > MAX_K) return;
#if MAX_SBB > UINT16_MAX
#error "pareto_prefix_census exact oracle requires 16-bit Sbb identifiers"
#endif
    sort1(sb, size);
    uint64_t hash = exact_hash(sb, size, k);
    size_t slot = (size_t)hash & (exact_slots_cap - 1);
    while (exact_slots[slot].hash) {
        ExactSlot *entry = &exact_slots[slot];
        if (entry->hash == hash && entry->size == size && entry->k == k) {
            const uint16_t *parts = exact_parts + entry->offset;
            int equal = 1;
            for (int i = 0; i < size; i++) {
                if (parts[i] != (uint16_t)sb[i]) { equal = 0; break; }
            }
            if (equal) {
                int old = entry->verdict == 1 ? TRUE : FALSE;
                if (old != verdict) {
                    fprintf(stderr, "conflicting exact cache facts at k=%d size=%d\n", k, size);
                    exit(5);
                }
                return;
            }
        }
        slot = (slot + 1) & (exact_slots_cap - 1);
    }
    exact_parts_reserve(exact_parts_len + (size_t)size);
    uint32_t offset = (uint32_t)exact_parts_len;
    if ((size_t)offset != exact_parts_len) {
        fprintf(stderr, "exact-fact parts exceed 32-bit offset space\n");
        exit(3);
    }
    for (int i = 0; i < size; i++) exact_parts[exact_parts_len++] = (uint16_t)sb[i];
    exact_slots[slot] = (ExactSlot){hash, offset, (uint16_t)size, (uint8_t)k,
                                   (uint8_t)(verdict == TRUE ? 1 : 2)};
    exact_slots_len++;
}

static size_t next_power_of_two(size_t value) {
    size_t result = 1;
    while (result < value) result *= 2;
    return result;
}

static void load_exact_oracle(const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) { fprintf(stderr, "cannot open exact cache %s\n", path); exit(2); }
    char line[4096];
    size_t candidates = 0;
    while (fgets(line, sizeof(line), file))
        if ((line[0] == '+' || line[0] == '-') && line[1] == ' ' && line[2] == 'b')
            candidates++;
    exact_slots_cap = next_power_of_two(candidates * 10 / 7 + 2048);
    exact_slots = calloc(exact_slots_cap, sizeof(*exact_slots));
    if (!exact_slots) { fprintf(stderr, "out of memory allocating exact-fact index\n"); exit(3); }
    rewind(file);

    size_t lines = 0;
    while (fgets(line, sizeof(line), file)) {
        lines++;
        if (!((line[0] == '+' || line[0] == '-') && line[1] == ' ' && line[2] == 'b')) continue;
        int verdict = line[0] == '+' ? TRUE : FALSE;
        char *save = NULL;
        char *token = strtok_r(line, " \t\r\n", &save); /* sign */
        token = strtok_r(NULL, " \t\r\n", &save);      /* b */
        int sb[EXACT_MAX_PARTS];
        int size = 0;
        int overflow = 0;
        while ((token = strtok_r(NULL, " \t\r\n", &save)) != NULL && strcmp(token, "t") != 0) {
            int n = atoi(token);
            token = strtok_r(NULL, " \t\r\n", &save);
            if (!token) { fprintf(stderr, "malformed exact cache line %zu\n", lines); exit(2); }
            int m = atoi(token);
            if (size < EXACT_MAX_PARTS) sb[size++] = getSbb(n, m);
            else overflow = 1;
        }
        if (!token) { fprintf(stderr, "malformed exact cache line %zu\n", lines); exit(2); }
        token = strtok_r(NULL, " \t\r\n", &save); /* pairs */
        token = strtok_r(NULL, " \t\r\n", &save); /* total vertices */
        token = strtok_r(NULL, " \t\r\n", &save); /* k */
        if (!token) { fprintf(stderr, "malformed exact cache tail at line %zu\n", lines); exit(2); }
        int k = atoi(token);
        if (!overflow) exact_insert(sb, size, k, verdict);
        if (!(lines % 1000000))
            fprintf(stderr, "exact oracle progress %zu lines, %zu facts\n", lines, exact_slots_len);
    }
    fclose(file);
    fprintf(stderr,
            "exact oracle loaded %zu unique facts (%zu candidates, %zu slots, %zu parts) from %s\n",
            exact_slots_len, candidates, exact_slots_cap, exact_parts_len, path);
}

static uint32_t hash_words(int k, int size, const int *sb) {
    uint32_t h = UINT32_C(2166136261) ^ (uint32_t)(k * 131 + size);
    for (int i = 0; i < size; i++) {
        h ^= (uint32_t)sb[i];
        h *= UINT32_C(16777619);
    }
    h ^= h >> 16;
    h *= UINT32_C(0x7feb352d);
    h ^= h >> 15;
    return h;
}

static uint32_t hash_state_units(int k, const CanonState *state, int units) {
    uint32_t h = hash_words(k, state->size, state->sb);
    h ^= (uint32_t)units + UINT32_C(0x9e3779b9) + (h << 6) + (h >> 2);
    return h;
}

static int canon_equal(const CanonState *a, const CanonState *b) {
    if (a->size != b->size) return 0;
    for (int i = 0; i < a->size; i++) if (a->sb[i] != b->sb[i]) return 0;
    return 1;
}

static CanonState canon_from_sbb(const int *sb, int size) {
    CanonState out;
    out.size = 0;
    for (int i = 0; i < size; i++) {
        /* Match canSolveB: empty and unit groups do not affect solvability. */
        if (sb[i] > 1) out.sb[out.size++] = sb[i];
    }
    if (out.size > 1) sort1(out.sb, out.size);
    return out;
}

static CanonState canon_from_label(const LabelPart *part, int size) {
    int sb[CENSUS_MAX_PARTS];
    for (int i = 0; i < size; i++) sb[i] = getSbb(part[i].u, part[i].v);
    return canon_from_sbb(sb, size);
}

static CanonState full_canon_from_label(const LabelPart *part, int size) {
    CanonState out;
    out.size = size;
    for (int i = 0; i < size; i++) out.sb[i] = getSbb(part[i].u, part[i].v);
    if (out.size > 1) sort1(out.sb, out.size);
    return out;
}

static long long state_mass(const CanonState *state) {
    long long mass = 0;
    for (int i = 0; i < state->size; i++) mass += sb_pairs[state->sb[i]];
    return mass;
}

static void print_canon(const CanonState *state) {
    if (state->size == 0) {
        printf("-");
        return;
    }
    for (int i = 0; i < state->size; i++) {
        if (i) printf(",");
        printf("%d:%d", sbb_to_n1[state->sb[i]], sbb_to_n2[state->sb[i]]);
    }
}

static void print_label(const LabelPart *part, int size) {
    for (int i = 0; i < size; i++) {
        if (i) printf(",");
        printf("%d:%d", part[i].u, part[i].v);
    }
}

static void print_take(const EnumContext *ctx) {
    for (int i = 0; i < ctx->size; i++) {
        if (i) printf(",");
        printf("%d:%d", ctx->take_u[i], ctx->take_v[i]);
    }
}

static int exact_result(int *sb, int size, int k, unsigned long long *queries) {
    int result = canSolveB(sb, size, k, CACHE_ONLY);
    if (result == MAYBE) {
        (*queries)++;
        result = canSolveB(sb, size, k, NO_DEADLINE);
    }
    return result;
}

static void enumerate_rec(EnumContext *ctx, int i) {
    int cap = power3[ctx->k - 1];
    if (i == ctx->size) {
        ctx->complete++;
        int r0 = exact_result(ctx->selected, ctx->size, ctx->k - 1, &ctx->exact_queries);
        if (r0 != TRUE) return;
        int r2 = exact_result(ctx->complement, ctx->size, ctx->k - 1, &ctx->exact_queries);
        if (r2 != TRUE) return;
        int r1 = exact_result(ctx->mixed, ctx->size * 2, ctx->k - 1, &ctx->exact_queries);
        if (r1 != TRUE) return;
        ctx->winners++;
        ctx->winner(ctx);
        return;
    }

    splits *sp = ctx->options[i];
    int nu = ctx->part[i].u;
    int nv = ctx->part[i].v;
    int normalized_swapped = nu < nv;
    int square = nu == nv;

    for (int oi = 0; oi < sp->size; oi++) {
        int a0 = sp->splitsl[oi][6];
        int b0 = sp->splitsl[oi][7];
        int variants = square && a0 != b0 ? 2 : 1;
        for (int variant = 0; variant < variants; variant++) {
            int a = a0, b = b0;
            if (variant) { int t = a; a = b; b = t; }
            if (normalized_swapped) { int t = a; a = b; b = t; }
            ctx->local_options++;
            ctx->prefixes++;

            int selected = getSbb(a, b);
            int complement = getSbb(nu - a, nv - b);
            int mixed0 = getSbb(a, nv - b);
            int mixed1 = getSbb(nu - a, b);
            long long p0 = ctx->selected_mass[i] + sb_pairs[selected];
            long long p1 = ctx->mixed_mass[i] + sb_pairs[mixed0] + sb_pairs[mixed1];
            long long p2 = ctx->complement_mass[i] + sb_pairs[complement];
            if (p0 > cap || p1 > cap || p2 > cap) {
                ctx->cap_pruned++;
                continue;
            }

            ctx->take_u[i] = a;
            ctx->take_v[i] = b;
            ctx->selected[i] = selected;
            ctx->complement[i] = complement;
            ctx->mixed[2 * i] = mixed0;
            ctx->mixed[2 * i + 1] = mixed1;
            ctx->selected_mass[i + 1] = p0;
            ctx->mixed_mass[i + 1] = p1;
            ctx->complement_mass[i + 1] = p2;

            int r0 = canSolveB(ctx->selected, i + 1, ctx->k - 1, CACHE_ONLY);
            int r2 = canSolveB(ctx->complement, i + 1, ctx->k - 1, CACHE_ONLY);
            int r1 = canSolveB(ctx->mixed, 2 * (i + 1), ctx->k - 1, CACHE_ONLY);
            if (r0 == FALSE || r1 == FALSE || r2 == FALSE) {
                ctx->cache_pruned++;
                continue;
            }
            enumerate_rec(ctx, i + 1);
        }
    }
}

static void enumerate_winners(const LabelPart *part, int size, int k,
                              WinnerCallback winner, void *opaque, EnumContext *ctx) {
    memset(ctx, 0, sizeof(*ctx));
    ctx->k = k;
    ctx->size = size;
    ctx->winner = winner;
    ctx->opaque = opaque;
    for (int i = 0; i < size; i++) {
        ctx->part[i] = part[i];
        int sbb = getSbb(part[i].u, part[i].v);
        ctx->options[i] = prepare_splits(sbb, k, size > 1);
    }
    enumerate_rec(ctx, 0);
}

static int exact_front_pair(int k, int u, int v) {
    if (u < v) { int t = u; u = v; v = t; }
    return v > 0 && v <= MAX_N / 2 && frontier[k][v] == u;
}

static int first_is_strict(const EnumContext *ctx) {
    int n = ctx->part[0].u, m = ctx->part[0].v;
    int a = ctx->take_u[0], b = ctx->take_v[0];
    return exact_front_pair(root_k - 1, a, b)
        || exact_front_pair(root_k - 1, n - a, m - b);
}

static int second_is_opposed(const EnumContext *ctx) {
    if (ctx->size != 2) return 0;
    int n0 = ctx->part[0].u, m0 = ctx->part[0].v;
    int n1 = ctx->part[1].u, m1 = ctx->part[1].v;
    int a0 = ctx->take_u[0], b0 = ctx->take_v[0];
    int a1 = ctx->take_u[1], b1 = ctx->take_v[1];
    return (exact_front_pair(root_k - 2, a0, b0)
            && exact_front_pair(root_k - 2, n1 - a1, m1 - b1))
        || (exact_front_pair(root_k - 2, n0 - a0, m0 - b0)
            && exact_front_pair(root_k - 2, a1, b1));
}

static size_t target_add(const CanonState *state, int units, unsigned long long root_bit,
                         int strict_first, int opposed) {
    uint32_t h = hash_state_units(residual_k, state, units) & (TARGET_HASH_SIZE - 1);
    while (target_hash[h]) {
        size_t index = target_hash[h] - 1;
        if (targets[index].units == units && canon_equal(&targets[index].state, state)) {
            TargetState *target = &targets[index];
            target->occurrences++;
            target->opposed_occurrences += opposed;
            target->strict_first_occurrences += strict_first;
            target->root_mask |= root_bit;
            return index;
        }
        h = (h + 1) & (TARGET_HASH_SIZE - 1);
    }
    if (targets_len == targets_cap) {
        size_t next = targets_cap ? targets_cap * 2 : 1024;
        TargetState *grown = realloc(targets, next * sizeof(*grown));
        if (!grown) { fprintf(stderr, "out of memory growing target states\n"); exit(3); }
        targets = grown;
        targets_cap = next;
    }
    size_t index = targets_len++;
    memset(&targets[index], 0, sizeof(targets[index]));
    targets[index].state = *state;
    targets[index].units = units;
    targets[index].occurrences = 1;
    targets[index].opposed_occurrences = opposed;
    targets[index].strict_first_occurrences = strict_first;
    targets[index].root_mask = root_bit;
    targets[index].upgrade_node = -1;
    target_hash[h] = (uint32_t)(index + 1);
    return index;
}

static void emit_second_winner(EnumContext *ctx) {
    SecondContext *second = ctx->opaque;
    int opposed = second_is_opposed(ctx);
    second_winners++;
    second_opposed_winners += opposed;

    LabelPart four[4];
    int raw_sbb[4];
    int units = 0;
    for (int i = 0; i < 2; i++) {
        int n = ctx->part[i].u, m = ctx->part[i].v;
        int a = ctx->take_u[i], b = ctx->take_v[i];
        four[2 * i] = (LabelPart){a, m - b};
        four[2 * i + 1] = (LabelPart){n - a, b};
        raw_sbb[2 * i] = getSbb(four[2 * i].u, four[2 * i].v);
        raw_sbb[2 * i + 1] = getSbb(four[2 * i + 1].u, four[2 * i + 1].v);
        units += raw_sbb[2 * i] == 1;
        units += raw_sbb[2 * i + 1] == 1;
    }
    CanonState state = canon_from_sbb(raw_sbb, 4);
    unsigned long long root_bit = second->root_index < 64
        ? (UINT64_C(1) << second->root_index) : 0;
    size_t target = target_add(&state, units, root_bit, second->strict_first, opposed);

    printf("CENSUS\tLINEAGE\troot=%d:%d\tk=%d\tfirst=%d:%d\tstrict_first=%d\ttwo=",
           second->root_n, second->root_m, residual_k,
           second->first_u, second->first_v, second->strict_first);
    print_label(ctx->part, 2);
    printf("\tsecond=");
    print_take(ctx);
    printf("\topposed=%d\tfour=", opposed);
    print_label(four, 4);
    printf("\ttarget=T%06zu\tcanonical=", target + 1);
    print_canon(&state);
    printf("\n");
}

static void remember_second_winner(EnumContext *ctx) {
    SecondMemo *memo = ctx->opaque;
    if (memo->cuts_len == memo->cuts_cap) {
        size_t next_cap = memo->cuts_cap ? memo->cuts_cap * 2 : 16;
        SecondCut *next = realloc(memo->cuts, next_cap * sizeof(*next));
        if (!next) { fprintf(stderr, "out of memory growing second-cut memo\n"); exit(3); }
        memo->cuts = next;
        memo->cuts_cap = next_cap;
    }
    SecondCut *cut = &memo->cuts[memo->cuts_len++];
    for (int i = 0; i < 2; i++) {
        cut->take_u[i] = ctx->take_u[i];
        cut->take_v[i] = ctx->take_v[i];
    }
}

static SecondMemo *second_memo_for(const LabelPart two[2], int *memo_hit) {
    CanonState state = full_canon_from_label(two, 2);
    for (size_t i = 0; i < second_memos_len; i++) {
        if (canon_equal(&second_memos[i].state, &state)) {
            *memo_hit = 1;
            second_memo_hits++;
            return &second_memos[i];
        }
    }

    if (second_memos_len == second_memos_cap) {
        size_t next_cap = second_memos_cap ? second_memos_cap * 2 : 256;
        SecondMemo *next = realloc(second_memos, next_cap * sizeof(*next));
        if (!next) { fprintf(stderr, "out of memory growing second-state memo\n"); exit(3); }
        second_memos = next;
        second_memos_cap = next_cap;
    }
    SecondMemo *memo = &second_memos[second_memos_len++];
    memset(memo, 0, sizeof(*memo));
    memo->state = state;
    *memo_hit = 0;
    second_unique_states++;

    LabelPart canonical[2];
    for (int i = 0; i < 2; i++) {
        canonical[i].u = sbb_to_n1[state.sb[i]];
        canonical[i].v = sbb_to_n2[state.sb[i]];
    }
    EnumContext enumeration;
    enumerate_winners(canonical, 2, root_k - 1, remember_second_winner, memo, &enumeration);
    memo->prefixes = enumeration.prefixes;
    memo->complete = enumeration.complete;
    memo->cap_pruned = enumeration.cap_pruned;
    memo->cache_pruned = enumeration.cache_pruned;
    memo->exact_queries = enumeration.exact_queries;
    if (memo->cuts_len != enumeration.winners) {
        fprintf(stderr, "second-cut memo count mismatch\n");
        exit(6);
    }
    second_prefixes += enumeration.prefixes;
    second_complete += enumeration.complete;
    return memo;
}

static void replay_second_memo(const LabelPart two[2], const SecondMemo *memo,
                               SecondContext *second) {
    int canonical_for_original[2] = {-1, -1};
    unsigned used = 0;
    for (int oi = 0; oi < 2; oi++) {
        int sbb = getSbb(two[oi].u, two[oi].v);
        for (int ci = 0; ci < 2; ci++) {
            if (!(used & (1u << ci)) && memo->state.sb[ci] == sbb) {
                canonical_for_original[oi] = ci;
                used |= 1u << ci;
                break;
            }
        }
        if (canonical_for_original[oi] < 0) {
            fprintf(stderr, "cannot transport canonical second-cut memo\n");
            exit(6);
        }
    }

    for (size_t wi = 0; wi < memo->cuts_len; wi++) {
        EnumContext replay;
        memset(&replay, 0, sizeof(replay));
        replay.k = root_k - 1;
        replay.size = 2;
        replay.opaque = second;
        for (int oi = 0; oi < 2; oi++) {
            int ci = canonical_for_original[oi];
            int a = memo->cuts[wi].take_u[ci];
            int b = memo->cuts[wi].take_v[ci];
            replay.part[oi] = two[oi];
            if (two[oi].u < two[oi].v) { int t = a; a = b; b = t; }
            replay.take_u[oi] = a;
            replay.take_v[oi] = b;
        }
        emit_second_winner(&replay);
    }
}

static void first_winner(EnumContext *ctx) {
    int *root_index = ctx->opaque;
    int strict = first_is_strict(ctx);
    first_winners++;
    first_strict_winners += strict;
    supplied_root_winners[*root_index]++;
    supplied_root_strict[*root_index] += strict;
    int n = ctx->part[0].u, m = ctx->part[0].v;
    int a = ctx->take_u[0], b = ctx->take_v[0];
    LabelPart two[2] = {{a, m - b}, {n - a, b}};
    CanonState two_canon = canon_from_label(two, 2);

    printf("CENSUS\tFIRST\troot=%d:%d\tk=%d\ttake=%d:%d\tstrict=%d\tmixed=",
           n, m, root_k, a, b, strict);
    print_label(two, 2);
    printf("\tcanonical=");
    print_canon(&two_canon);
    printf("\n");

    /* A zero mixed rectangle is a genuine low-level degeneration.  There is then no two-lineage
       state to which the opposed-branch premise applies; record the first cut but do not invent a
       second component. */
    if (getSbb(two[0].u, two[0].v) == 0 || getSbb(two[1].u, two[1].v) == 0) return;

    SecondContext second = {
        .root_index = *root_index,
        .root_n = n,
        .root_m = m,
        .first_u = a,
        .first_v = b,
        .strict_first = strict,
    };
    second_invocations++;
    int memo_hit;
    SecondMemo *memo = second_memo_for(two, &memo_hit);
    replay_second_memo(two, memo, &second);
    printf("CENSUS\tSECOND_SUMMARY\troot=%d:%d\tfirst=%d:%d\twinners=%llu"
           "\tprefixes=%llu\tcomplete=%llu\tcap_pruned=%llu\tcache_pruned=%llu"
           "\texact_queries=%llu\tmemo_hit=%d\n",
           n, m, a, b, (unsigned long long)memo->cuts_len, memo->prefixes, memo->complete,
           memo->cap_pruned, memo->cache_pruned, memo->exact_queries, memo_hit);
}

static void load_frontier(const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) { fprintf(stderr, "cannot open Pareto table %s\n", path); exit(2); }
    char line[2048];
    if (!fgets(line, sizeof(line), file)) { fprintf(stderr, "empty Pareto table\n"); exit(2); }
    while (fgets(line, sizeof(line), file)) {
        int k, m, n;
        char bound[32], status[64];
        if (sscanf(line, "%d,%d,%d,%31[^,],%63[^,]", &k, &m, &n, bound, status) != 5)
            continue;
        if (k >= 0 && k <= MAX_K && m >= 0 && m <= MAX_N / 2
            && strcmp(bound, "max") == 0 && strncmp(status, "proven", 6) == 0) {
            frontier[k][m] = n;
        }
    }
    fclose(file);
}

static void upgrade_hash_rebuild(size_t next_cap) {
    uint32_t *next = calloc(next_cap, sizeof(*next));
    if (!next) { fprintf(stderr, "out of memory growing upgrade hash\n"); exit(3); }
    for (size_t i = 0; i < upgrade_len; i++) {
        uint32_t h = hash_state_units(residual_k, &upgrade_nodes[i].state,
                                      upgrade_nodes[i].units)
            & (uint32_t)(next_cap - 1);
        while (next[h]) h = (h + 1) & (uint32_t)(next_cap - 1);
        next[h] = (uint32_t)(i + 1);
    }
    free(upgrade_hash);
    upgrade_hash = next;
    upgrade_hash_cap = next_cap;
}

static size_t upgrade_add(const CanonState *state, int units, int is_seed, int *inserted) {
    if (upgrade_hash_cap == 0) upgrade_hash_rebuild(2048);
    if ((upgrade_len + 1) * 10 >= upgrade_hash_cap * 7)
        upgrade_hash_rebuild(upgrade_hash_cap * 2);
    uint32_t h = hash_state_units(residual_k, state, units)
        & (uint32_t)(upgrade_hash_cap - 1);
    while (upgrade_hash[h]) {
        size_t index = upgrade_hash[h] - 1;
        if (upgrade_nodes[index].units == units
            && canon_equal(&upgrade_nodes[index].state, state)) {
            if (is_seed) upgrade_nodes[index].is_seed = 1;
            *inserted = 0;
            return index;
        }
        h = (h + 1) & (uint32_t)(upgrade_hash_cap - 1);
    }
    if (upgrade_len >= max_upgrade_states) {
        fprintf(stderr, "upgrade state limit reached at %zu; census is incomplete\n", upgrade_len);
        exit(4);
    }
    if (upgrade_len == upgrade_cap) {
        size_t next_cap = upgrade_cap ? upgrade_cap * 2 : 2048;
        UpgradeNode *next = realloc(upgrade_nodes, next_cap * sizeof(*next));
        if (!next) { fprintf(stderr, "out of memory growing upgrade nodes\n"); exit(3); }
        upgrade_nodes = next;
        upgrade_cap = next_cap;
    }
    size_t index = upgrade_len++;
    memset(&upgrade_nodes[index], 0, sizeof(upgrade_nodes[index]));
    upgrade_nodes[index].state = *state;
    upgrade_nodes[index].units = units;
    upgrade_nodes[index].is_seed = is_seed;
    upgrade_hash[h] = (uint32_t)(index + 1);
    *inserted = 1;
    return index;
}

static int solve_canon(const CanonState *state, int units) {
    if (state_mass(state) + units > power3[residual_k]) return FALSE;
    if (state->size == 0) return TRUE;
    int sb[CENSUS_MAX_PARTS];
    for (int i = 0; i < state->size; i++) sb[i] = state->sb[i];
    return exact_result(sb, state->size, residual_k, &(unsigned long long){0});
}

static int inside_component_frontier(const CanonState *state) {
    for (int i = 0; i < state->size; i++) {
        int n = sbb_to_n1[state->sb[i]];
        int m = sbb_to_n2[state->sb[i]];
        if (m <= 0 || m > MAX_N / 2 || frontier[residual_k][m] == 0
            || n > frontier[residual_k][m]) return 0;
    }
    return 1;
}

static CanonState bumped_state(const CanonState *state, int part, int which) {
    CanonState out = *state;
    int n = sbb_to_n1[state->sb[part]];
    int m = sbb_to_n2[state->sb[part]];
    if (which == 0) n++;
    else m++;
    out.sb[part] = getSbb(n, m);
    sort1(out.sb, out.size);
    return out;
}

static int endpoint_dominates_seed_rec(const CanonState *endpoint, const CanonState *seed,
                                       int i, unsigned used) {
    if (i == seed->size) return 1;
    int sn = sbb_to_n1[seed->sb[i]], sm = sbb_to_n2[seed->sb[i]];
    for (int j = 0; j < endpoint->size; j++) {
        if (used & (1u << j)) continue;
        int en = sbb_to_n1[endpoint->sb[j]], em = sbb_to_n2[endpoint->sb[j]];
        if (en >= sn && em >= sm
            && endpoint_dominates_seed_rec(endpoint, seed, i + 1, used | (1u << j)))
            return 1;
    }
    return 0;
}

static int endpoint_dominates_seed(const CanonState *endpoint, const CanonState *seed) {
    return endpoint->size == seed->size
        && endpoint_dominates_seed_rec(endpoint, seed, 0, 0);
}

static unsigned long long component_bijections_rec(const CanonState *endpoint,
                                                    const CanonState *seed,
                                                    int i, unsigned used) {
    if (i == seed->size) return 1;
    int sn = sbb_to_n1[seed->sb[i]], sm = sbb_to_n2[seed->sb[i]];
    unsigned long long total = 0;
    for (int j = 0; j < endpoint->size; j++) {
        if (used & (1u << j)) continue;
        int en = sbb_to_n1[endpoint->sb[j]], em = sbb_to_n2[endpoint->sb[j]];
        if (en >= sn && em >= sm)
            total += component_bijections_rec(endpoint, seed, i + 1, used | (1u << j));
    }
    return total;
}

static unsigned long long component_bijections(const CanonState *endpoint,
                                                const CanonState *seed) {
    if (endpoint->size != seed->size) return 0;
    return component_bijections_rec(endpoint, seed, 0, 0);
}

static unsigned long long shore_embeddings_rec(const CanonState *endpoint,
                                                const CanonState *seed,
                                                int i, unsigned used) {
    if (i == seed->size) return 1;
    int sn = sbb_to_n1[seed->sb[i]], sm = sbb_to_n2[seed->sb[i]];
    unsigned long long total = 0;
    for (int j = 0; j < endpoint->size; j++) {
        if (used & (1u << j)) continue;
        int en = sbb_to_n1[endpoint->sb[j]], em = sbb_to_n2[endpoint->sb[j]];
        int orientations = (en >= sn && em >= sm) + (em >= sn && en >= sm);
        if (orientations)
            total += (unsigned long long)orientations
                * shore_embeddings_rec(endpoint, seed, i + 1, used | (1u << j));
    }
    return total;
}

static unsigned long long shore_embeddings(const CanonState *endpoint,
                                            const CanonState *seed) {
    if (endpoint->size != seed->size) return 0;
    return shore_embeddings_rec(endpoint, seed, 0, 0);
}

static size_t enumerate_upgrade_antichain(void) {
    size_t eligible = 0;
    for (size_t i = 0; i < targets_len; i++) {
        if (state_mass(&targets[i].state) + targets[i].units == 0) continue;
        if (!inside_component_frontier(&targets[i].state)) {
            fprintf(stderr, "solvable target T%06zu crosses the proven component frontier\n",
                    i + 1);
            exit(5);
        }
        if (state_mass(&targets[i].state) + targets[i].units > power3[residual_k]) {
            fprintf(stderr, "solvable target T%06zu exceeds its information capacity\n", i + 1);
            exit(5);
        }
        int inserted;
        size_t node = upgrade_add(&targets[i].state, targets[i].units, 1, &inserted);
        targets[i].upgrade_node = (int)node;
        eligible++;
    }

    unsigned long long successor_queries = 0;
    unsigned long long information_rejected = 0;
    size_t endpoint_count = 0;
    for (size_t cursor = 0; cursor < upgrade_len; cursor++) {
        /* upgrade_add may realloc upgrade_nodes, so never retain a pointer into the arena across
           a successor insertion. */
        CanonState current = upgrade_nodes[cursor].state;
        int units = upgrade_nodes[cursor].units;
        int has_successor = 0;
        CanonState seen_next[2 * CENSUS_MAX_PARTS];
        int seen_next_len = 0;
        for (int part = 0; part < current.size; part++) {
            int n = sbb_to_n1[current.sb[part]];
            int m = sbb_to_n2[current.sb[part]];
            int variants = n == m ? 1 : 2;
            for (int which = 0; which < variants; which++) {
                if (n + m + 1 > MAX_N) {
                    /* A boundary-limited node is not proved maximal.  Keep it out of the endpoint
                       corpus and fail the run after reporting the exact count. */
                    upgrade_representation_blocked++;
                    has_successor = 1;
                    continue;
                }
                CanonState next = bumped_state(&current, part, which);
                int duplicate = 0;
                for (int si = 0; si < seen_next_len; si++)
                    if (canon_equal(&seen_next[si], &next)) { duplicate = 1; break; }
                if (duplicate) continue;
                seen_next[seen_next_len++] = next;
                if (!inside_component_frontier(&next)) {
                    upgrade_component_frontier_rejected++;
                    continue;
                }
                if (state_mass(&next) + units > power3[residual_k]) {
                    information_rejected++;
                    continue;
                }
                successor_queries++;
                if (solve_canon(&next, units) != TRUE) continue;
                has_successor = 1;
                int inserted;
                (void)upgrade_add(&next, units, 0, &inserted);
            }
        }
        if (!has_successor) {
            upgrade_nodes[cursor].is_endpoint = 1;
            endpoint_count++;
        }
        if (!(cursor & 0x3fff)) {
            fprintf(stderr, "upgrade progress %zu/%zu nodes, endpoints %zu\n",
                    cursor + 1, upgrade_len, endpoint_count);
        }
    }

    printf("CENSUS\tUPGRADE_SUMMARY\tk=%d\teligible_seeds=%zu\tvisited=%zu"
           "\tendpoints=%zu\tsuccessor_queries=%llu\tinformation_rejected=%llu"
           "\tcomponent_frontier_rejected=%llu"
           "\trepresentation_blocked=%llu\n",
           residual_k, eligible, upgrade_len, endpoint_count, successor_queries,
           information_rejected, upgrade_component_frontier_rejected,
           upgrade_representation_blocked);

    size_t endpoint_id = 0;
    for (size_t i = 0; i < upgrade_len; i++) {
        if (!upgrade_nodes[i].is_endpoint) continue;
        endpoint_id++;
        long long core_mass = state_mass(&upgrade_nodes[i].state);
        printf("CENSUS\tENDPOINT\tid=U%06zu\tnode=%zu\tk=%d\tparts=%d\tunits=%d"
               "\tcore_mass=%lld\tmass=%lld\tstate=",
               endpoint_id, i + 1, residual_k, upgrade_nodes[i].state.size,
               upgrade_nodes[i].units, core_mass, core_mass + upgrade_nodes[i].units);
        print_canon(&upgrade_nodes[i].state);
        printf("\n");
    }

    for (size_t ti = 0; ti < targets_len; ti++) {
        TargetState *target = &targets[ti];
        if (state_mass(&target->state) + target->units == 0) continue;
        endpoint_id = 0;
        size_t matches = 0;
        for (size_t ui = 0; ui < upgrade_len; ui++) {
            if (!upgrade_nodes[ui].is_endpoint) continue;
            endpoint_id++;
            if (upgrade_nodes[ui].units != target->units) continue;
            if (!endpoint_dominates_seed(&upgrade_nodes[ui].state, &target->state)) continue;
            unsigned long long bijections = component_bijections(&upgrade_nodes[ui].state,
                                                                  &target->state);
            unsigned long long embeddings = shore_embeddings(&upgrade_nodes[ui].state,
                                                               &target->state);
            if (!bijections || !embeddings) {
                fprintf(stderr, "upgrade alignment count vanished for T%06zu -> U%06zu\n",
                        ti + 1, endpoint_id);
                exit(6);
            }
            matches++;
            printf("CENSUS\tUPGRADE\tseed=T%06zu\tendpoint=U%06zu\tdelta_mass=%lld"
                   "\tcomponent_bijections=%llu\tshore_embeddings=%llu\n",
                   ti + 1, endpoint_id,
                   state_mass(&upgrade_nodes[ui].state) - state_mass(&target->state),
                   bijections, embeddings);
        }
        printf("CENSUS\tUPGRADE_SEED_SUMMARY\tseed=T%06zu\tendpoints=%zu\n",
               ti + 1, matches);
    }
    return endpoint_count;
}

typedef struct {
    size_t endpoint_id;
    int units;
    unsigned long long unit_extended_winners;
} FullContext;

static unsigned long long choose_small(int n, int r) {
    unsigned long long result = 1;
    for (int i = 1; i <= r; i++) result = result * (unsigned)(n - r + i) / (unsigned)i;
    return result;
}

static unsigned long long count_unit_extensions(int units, const EnumContext *ctx) {
    long long cap = power3[ctx->k - 1];
    long long masses[3] = {
        ctx->selected_mass[ctx->size],
        ctx->mixed_mass[ctx->size],
        ctx->complement_mass[ctx->size],
    };
    unsigned long long total = 0;
    for (int outcome0 = 0; outcome0 <= units; outcome0++) {
        for (int outcome1 = 0; outcome1 <= units - outcome0; outcome1++) {
            int outcome2 = units - outcome0 - outcome1;
            if (masses[0] + outcome0 > cap || masses[1] + outcome1 > cap
                || masses[2] + outcome2 > cap) continue;
            unsigned long long assignments = choose_small(units, outcome0)
                * choose_small(units - outcome0, outcome1);
            total += assignments << outcome1;
        }
    }
    return total;
}

static void full_winner(EnumContext *ctx) {
    FullContext *full = ctx->opaque;
    CanonState selected = canon_from_sbb(ctx->selected, ctx->size);
    CanonState mixed = canon_from_sbb(ctx->mixed, ctx->size * 2);
    CanonState complement = canon_from_sbb(ctx->complement, ctx->size);
    unsigned long long unit_extensions = count_unit_extensions(full->units, ctx);
    if (!unit_extensions) {
        fprintf(stderr, "endpoint U%06zu has a core split that cannot place its unit reserve\n",
                full->endpoint_id);
        exit(6);
    }
    full->unit_extended_winners += unit_extensions;
    printf("CENSUS\tFULL_WIN\tid=U%06zu\tk=%d\ttake=", full->endpoint_id, ctx->k);
    print_take(ctx);
    printf("\tmasses=%lld/%lld/%lld\tselected=",
           ctx->selected_mass[ctx->size], ctx->mixed_mass[ctx->size],
           ctx->complement_mass[ctx->size]);
    print_canon(&selected);
    printf("\tmixed=");
    print_canon(&mixed);
    printf("\tcomplement=");
    print_canon(&complement);
    printf("\tunit_extensions=%llu\n", unit_extensions);
}

static void map_endpoints(void) {
    size_t endpoint_id = 0;
    for (size_t i = 0; i < upgrade_len; i++) {
        if (!upgrade_nodes[i].is_endpoint) continue;
        endpoint_id++;
        CanonState *state = &upgrade_nodes[i].state;
        LabelPart parts[4];
        for (int j = 0; j < state->size; j++) {
            parts[j].u = sbb_to_n1[state->sb[j]];
            parts[j].v = sbb_to_n2[state->sb[j]];
        }
        FullContext full = {endpoint_id, upgrade_nodes[i].units, 0};
        EnumContext enumeration;
        long long core_mass = state_mass(state);
        printf("CENSUS\tFULL_STATE\tid=U%06zu\tk=%d\tparts=%d\tunits=%d"
               "\tcore_mass=%lld\tmass=%lld\tstate=",
               endpoint_id, residual_k, state->size, full.units,
               core_mass, core_mass + full.units);
        print_canon(state);
        printf("\n");
        fflush(stdout);
        if (state->size > 0) {
            enumerate_winners(parts, state->size, residual_k, full_winner, &full, &enumeration);
        } else {
            memset(&enumeration, 0, sizeof(enumeration));
            enumeration.k = residual_k;
            enumeration.size = 0;
            enumeration.complete = 1;
            enumeration.winners = 1;
            enumeration.opaque = &full;
            full_winner(&enumeration);
        }
        printf("CENSUS\tFULL_SUMMARY\tid=U%06zu\twinners=%llu\tprefixes=%llu"
               "\tunit_extended_winners=%llu\tcomplete=%llu\tcap_pruned=%llu"
               "\tcache_pruned=%llu\texact_queries=%llu\n",
               endpoint_id, enumeration.winners, enumeration.prefixes,
               full.unit_extended_winners, enumeration.complete, enumeration.cap_pruned,
               enumeration.cache_pruned, enumeration.exact_queries);
        fflush(stdout);
    }
}

static int collect_roots(int k, int roots[][2], int cap) {
    int count = 0;
    for (int m = 1; m <= MAX_N / 2; m++) {
        if (!frontier[k][m]) continue;
        if (count >= cap) { fprintf(stderr, "too many roots\n"); exit(2); }
        roots[count][0] = frontier[k][m];
        roots[count][1] = m;
        count++;
    }
    return count;
}

typedef struct {
    int n, m, a, b;
} SuppliedCut;

static int supplied_cut_equal(const SuppliedCut *x, int n, int m, int a, int b) {
    return x->n == n && x->m == m && x->a == a && x->b == b;
}

static int root_index_for(const int roots[][2], int root_count, int n, int m) {
    for (int i = 0; i < root_count; i++)
        if (roots[i][0] == n && roots[i][1] == m) return i;
    return -1;
}

static void consume_supplied_cut(int root_index, int n, int m, int a, int b) {
    if (a < 0 || a > n || b < 0 || b > m) {
        fprintf(stderr, "invalid supplied cut %d:%d for root %d:%d\n", a, b, n, m);
        exit(2);
    }
    int selected[1] = {getSbb(a, b)};
    int complement[1] = {getSbb(n - a, m - b)};
    int mixed[2] = {getSbb(a, m - b), getSbb(n - a, b)};
    unsigned long long exact_queries = 0;
    if (exact_result(selected, 1, root_k - 1, &exact_queries) != TRUE
        || exact_result(complement, 1, root_k - 1, &exact_queries) != TRUE
        || exact_result(mixed, 2, root_k - 1, &exact_queries) != TRUE) {
        fprintf(stderr, "supplied cut failed current exact verification: root %d:%d take %d:%d\n",
                n, m, a, b);
        exit(5);
    }
    EnumContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.k = root_k;
    ctx.size = 1;
    ctx.part[0] = (LabelPart){n, m};
    ctx.take_u[0] = a;
    ctx.take_v[0] = b;
    ctx.selected[0] = selected[0];
    ctx.complement[0] = complement[0];
    ctx.mixed[0] = mixed[0];
    ctx.mixed[1] = mixed[1];
    ctx.opaque = &root_index;
    first_prefixes++;
    first_complete++;
    first_winner(&ctx);
}

static void consume_root_log(const char *path, const int roots[][2], int root_count) {
    FILE *file = fopen(path, "r");
    if (!file) { fprintf(stderr, "cannot open root winner log %s\n", path); exit(2); }
    SuppliedCut *cuts = NULL;
    size_t cuts_len = 0, cuts_cap = 0;
    char line[8192];
    while (fgets(line, sizeof(line), file)) {
        int k, n, m, consumed = 0;
        if (sscanf(line, "result in %d can solve Sb(%d:%d)%n", &k, &n, &m, &consumed) != 3
            || k != root_k || consumed <= 0) continue;
        int root_index = root_index_for(roots, root_count, n, m);
        if (root_index < 0) continue;
        char *with = strstr(line + consumed, " with [");
        int a, b;
        if (!with || sscanf(with, " with [%d:%d]", &a, &b) != 2) {
            fprintf(stderr, "malformed root winner line: %s", line);
            exit(2);
        }
        int duplicate = 0;
        for (size_t i = 0; i < cuts_len; i++)
            if (supplied_cut_equal(&cuts[i], n, m, a, b)) { duplicate = 1; break; }
        if (duplicate) continue;
        if (cuts_len == cuts_cap) {
            size_t next_cap = cuts_cap ? cuts_cap * 2 : 1024;
            SuppliedCut *next = realloc(cuts, next_cap * sizeof(*next));
            if (!next) { fprintf(stderr, "out of memory growing supplied cuts\n"); exit(3); }
            cuts = next;
            cuts_cap = next_cap;
        }
        cuts[cuts_len++] = (SuppliedCut){n, m, a, b};
        consume_supplied_cut(root_index, n, m, a, b);
    }
    fclose(file);
    printf("CENSUS\tROOT_LOG\tpath=%s\tunique_cuts=%zu\n", path, cuts_len);
    free(cuts);
}

int main(int argc, char **argv) {
    if (argc < 4 || argc > 7) {
        fprintf(stderr,
                "usage: %s CACHE PARETO_CSV ROOT_K "
                "[MAX_UPGRADE_STATES [ROOT_LOG [EXACT_CACHE]]]\n",
                argv[0]);
        return 2;
    }
    root_k = atoi(argv[3]);
    residual_k = root_k - 2;
    if (root_k < 3 || root_k > MAX_K) {
        fprintf(stderr, "ROOT_K must be in 3..%d\n", MAX_K);
        return 2;
    }
    if (argc >= 5) {
        unsigned long long parsed = strtoull(argv[4], NULL, 10);
        if (parsed == 0 || parsed > UINT32_MAX) {
            fprintf(stderr, "invalid MAX_UPGRADE_STATES\n");
            return 2;
        }
        max_upgrade_states = (size_t)parsed;
    }

    init();
    load_frontier(argv[2]);
    if (argc == 7) load_exact_oracle(argv[6]);
    parse_file(argv[1]);

    int roots[512][2];
    int root_count = collect_roots(root_k, roots, 512);
    printf("CENSUS\tBEGIN\troot_k=%d\tresidual_k=%d\troots=%d\tupgrade_limit=%zu\n",
           root_k, residual_k, root_count, max_upgrade_states);

    if (argc >= 6 && strcmp(argv[5], "-") != 0)
        consume_root_log(argv[5], roots, root_count);

    for (int ri = 0; ri < root_count; ri++) {
        LabelPart root[1] = {{roots[ri][0], roots[ri][1]}};
        EnumContext enumeration;
        memset(&enumeration, 0, sizeof(enumeration));
        roots_seen++;
        /* The raw k=8 batch omitted the theorem-trivial m=1 root.  More generally, enumerate any
           frontier root for which the supplied log had no winning cut rather than silently leaving
           a hole. */
        if (supplied_root_winners[ri] == 0) {
            enumerate_winners(root, 1, root_k, first_winner, &ri, &enumeration);
            first_prefixes += enumeration.prefixes;
            first_complete += enumeration.complete;
        }
        roots_with_strict_first += supplied_root_strict[ri] > 0;
        printf("CENSUS\tROOT_SUMMARY\troot=%d:%d\twinners=%llu\tstrict_winners=%llu"
               "\tprefixes=%llu\tcomplete=%llu\tcap_pruned=%llu\tcache_pruned=%llu"
               "\texact_queries=%llu\n",
               roots[ri][0], roots[ri][1], supplied_root_winners[ri],
               supplied_root_strict[ri], enumeration.prefixes, enumeration.complete,
               enumeration.cap_pruned, enumeration.cache_pruned, enumeration.exact_queries);
        fflush(stdout);
    }

    printf("CENSUS\tPREFIX_SUMMARY\troot_k=%d\troots=%llu\tfirst_winners=%llu"
           "\tfirst_strict_winners=%llu\troots_with_strict_first=%llu"
           "\tsecond_invocations=%llu\tsecond_winners=%llu\tsecond_opposed_winners=%llu"
           "\tsecond_unique_states=%llu\tsecond_memo_hits=%llu"
           "\tfirst_prefixes=%llu\tfirst_complete=%llu\tsecond_prefixes=%llu"
           "\tsecond_complete=%llu\ttargets=%zu\n",
           root_k, roots_seen, first_winners, first_strict_winners, roots_with_strict_first,
           second_invocations, second_winners, second_opposed_winners,
           second_unique_states, second_memo_hits,
           first_prefixes, first_complete, second_prefixes, second_complete, targets_len);

    size_t degenerate = 0, four = 0, upgrade_seeds = 0, empty_core = 0;
    for (size_t i = 0; i < targets_len; i++) {
        TargetState *target = &targets[i];
        long long core_mass = state_mass(&target->state);
        long long total_mass = core_mass + target->units;
        int empty_lineages = 4 - target->state.size - target->units;
        if (empty_lineages < 0) {
            fprintf(stderr, "target T%06zu has impossible lineage accounting\n", i + 1);
            return 6;
        }
        int eligible = total_mass > 0;
        if (eligible) upgrade_seeds++;
        if (target->state.size == 4 && target->units == 0) four++;
        else degenerate++;
        if (target->state.size == 0) empty_core++;
        printf("CENSUS\tTARGET\tid=T%06zu\tk=%d\tparts=%d\tunits=%d"
               "\tempty_lineages=%d\teligible=%d\tcore_mass=%lld\tmass=%lld"
               "\toccurrences=%llu\tstrict_first_occurrences=%llu"
               "\topposed_occurrences=%llu\troot_mask=%016llx\tstate=",
               i + 1, residual_k, target->state.size, target->units, empty_lineages,
               eligible, core_mass, total_mass,
               target->occurrences, target->strict_first_occurrences,
               target->opposed_occurrences, target->root_mask);
        print_canon(&target->state);
        printf("\n");
    }
    printf("CENSUS\tTARGET_SUMMARY\tk=%d\ttargets=%zu\tupgrade_seeds=%zu"
           "\tcanonical_four=%zu\tdegenerate=%zu\tempty_core=%zu\n",
           residual_k, targets_len, upgrade_seeds, four, degenerate, empty_core);
    fflush(stdout);

    size_t endpoints = enumerate_upgrade_antichain();
    fflush(stdout);
    if (upgrade_representation_blocked) {
        fprintf(stderr,
                "MAX_N=%d blocked %llu upgrade successors; rerun with a larger MAX_N\n",
                MAX_N, upgrade_representation_blocked);
        return 4;
    }
    map_endpoints();
    printf("CENSUS\tEXACT_ORACLE_SUMMARY\tfacts=%zu\thits=%llu\tmisses=%llu\n",
           exact_slots_len, exact_hits, exact_misses);
    printf("CENSUS\tEND\troot_k=%d\tresidual_k=%d\ttargets=%zu\tupgrade_nodes=%zu"
           "\tendpoints=%zu\n", root_k, residual_k, targets_len, upgrade_len, endpoints);
    return 0;
}
