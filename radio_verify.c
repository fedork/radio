// An independent checker for negative certificates.
//
// Reads either a solver log or the human-readable `radio-negative-certificate-v1` format and
// verifies every negative fact from first principles. It does NOT include radiobase.c and shares
// no code with the solver - that independence is the whole point. It knows exactly four things:
//
//   * the split semantics of docs/problem.md: a test on (n:m) taking (a,b) yields children
//     (a:b), (n-a:m-b) and the mixed pair {(a:m-b), (n-a:b)}
//   * Singleton Majorization: an all-singleton state is unsolvable iff its n-sides are not
//     weakly majorized by G_k -- applied to the full star expansion of every state through
//     the Vertex-Splitting Pullback Lemma
//   * Unit-Group Elimination: (1:1) parts never affect solvability
//   * Subgraph Monotonicity: a <= b as sub-multisets with componentwise-smaller parts, and a
//     unsolvable, implies b unsolvable
//
// It knows nothing about split orderings, FAST, deadlines, the pass structure or the cache.
//
// Design notes in docs/certificate.md. Architecture, per the level-specialisation idea:
//
//   * a level-k check consults only the frozen k-1 fact set. Verification order does not matter:
//     the multicore path mixes every level in one dynamic queue, and soundness is well-founded
//     induction on k over the conjunction of all checks. Top-down coloring alone has a per-level
//     barrier because citations from k define the target set at k-1.
//   * each level's fact set is frozen: sorted, bucketed by part count, with an open-addressed
//     hash for exact membership. Read-only structures are both faster and far easier to reason
//     about than the mutable trie the solver needs.
//   * the group-local live-split table depends only on (part, k), never on the containing
//     state, so it is computed once and reused. Measured on the k=9 ladder: 729 distinct
//     (part,k) pairs serve 657,945 part-slots, a reuse factor near 900x.
//
//   * children are built INCREMENTALLY down the recursion - one insertion into a sorted array
//     per part, with a rolling mass and rolling hash - instead of re-canonicalising the whole
//     child at every node. Same mistake the solver's sort1 had. Measured 1.20x.
//   * pairwise narrowing (below) is look-ahead: it is the only reduction here that acts on
//     groups not yet fixed. Prefix narrowing already subsumes every SUBSET of the prefix,
//     because a refuted subset-child is a sub-multiset of the prefix-child.
//
// Reductions applied, all sound, all measured (see docs/journal.md 2026-08-04):
//   counting bound on prefixes | group-local split rejection | prefix subset narrowing
//   | pairwise narrowing with forward checking | complement symmetry
//   | identical-part permutation skip
//
//   tools/build_radio.py -O3 -pthread radio_verify.c -o radio_verify
//   tools/run_with_provenance.py ./radio_verify <log> [maxk [group_order [pairwise [pairwise_min_parts]]]]
//   VERIFY_THREADS=8 tools/run_with_provenance.py ./radio_verify <log> <maxk>
//   TOPDOWN=6 ROOTS=roots.cert MINIMIZE_BEFORE_COLOR=1 CERT_OUT=proof.cert \
//       tools/run_with_provenance.py ./radio_verify <log> 6
//
// Group order 0 (canonical descending) is the best of the three tried: ascending is 3.8x more
// nodes, fewest-options-first 1.09x. Option order WITHIN a group is provably irrelevant for a
// fact that verifies - the whole tree is enumerated, and the node set depends on the prefix as
// a set of assignments, not on the order they are tried in.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>

#define TLS _Thread_local

#define MAXK 12
#define MAXP 40                 /* parts per state */
#define NPART 65536             /* (n<<8)|m, so n,m <= 255 */

#define MAXC (2 * MAXP)          /* a mixed child has two parts per group */

/* The product index plus adaptive block summaries is the production default.
   VERIFY_LEGACY_INDEX retains the former (np,largest-n,mass) layout for exact A/B reproduction;
   defining either product implementation macro explicitly suppresses all defaults, which permits
   product-only and profile-only diagnostics. VERIFY_NO_BLOCK_PARETO is the convenient default-
   product control when no other implementation macro is supplied. */
#if !defined(VERIFY_LEGACY_INDEX) && !defined(VERIFY_PRODUCT_PROFILE) && \
    !defined(VERIFY_PRODUCT_SORT)
#define VERIFY_PRODUCT_PROFILE
#define VERIFY_PRODUCT_SORT
#if !defined(VERIFY_NO_BLOCK_PARETO) && !defined(VERIFY_BLOCK_PARETO)
#define VERIFY_BLOCK_PARETO
#endif
#endif

#if defined(VERIFY_PRODUCT_SORT) && !defined(VERIFY_PRODUCT_PROFILE)
#error "VERIFY_PRODUCT_SORT requires VERIFY_PRODUCT_PROFILE"
#endif
#if defined(VERIFY_BLOCK_PARETO) && !defined(VERIFY_PRODUCT_SORT)
#error "VERIFY_BLOCK_PARETO requires VERIFY_PRODUCT_SORT"
#endif

#ifndef VERIFY_BLOCK_SIZE
#define VERIFY_BLOCK_SIZE 256
#endif
#if VERIFY_BLOCK_SIZE < 2
#error "VERIFY_BLOCK_SIZE must be at least 2"
#endif
#ifndef VERIFY_BLOCK_MIN_LEVEL_FACTS
#define VERIFY_BLOCK_MIN_LEVEL_FACTS 65536
#endif
#if VERIFY_BLOCK_MIN_LEVEL_FACTS < VERIFY_BLOCK_SIZE
#error "VERIFY_BLOCK_MIN_LEVEL_FACTS must be at least VERIFY_BLOCK_SIZE"
#endif

typedef struct { unsigned char n, m; } Part;
typedef struct { unsigned char np, src; Part p[MAXP]; int mass; } Fact;
#ifdef VERIFY_BLOCK_PARETO
typedef struct { uint64_t pp; int mass; } BlockProfile;
#endif

/* Long batches are deliberately observable without putting an atomic increment in the hot
   dominance scans.  Workers publish one completion per fact and, while a fact is active, a
   coarse node cursor every 2^20 recursion nodes.  VERIFY_PROGRESS_SECONDS enables the reporter;
   the default emits exact batch boundaries but does not start the periodic reporter. */
static double g_progress_seconds;
static TLS atomic_ullong *g_progress_node_slot;

static double monotonic_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static uint64_t monotonic_nanoseconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static struct timespec realtime_after(double seconds) {
    struct timespec ts;
    long add_ns = (long)((seconds - (long)seconds) * 1e9);
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += (time_t)seconds;
    ts.tv_nsec += add_ns;
    if (ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }
    return ts;
}

static void write_fact_inline(FILE *fp, const Fact *f) {
    int i;
    fputs("Sb(", fp);
    for (i = 0; i < f->np; i++)
        fprintf(fp, "%s%d:%d", i ? "," : "", f->p[i].n, f->p[i].m);
    fputc(')', fp);
}

/* A partial child under construction. Built by copy-and-insert from the parent depth rather
   than re-canonicalised: measured 2026-08-04, canon() from scratch was 987M calls over 5.6
   BILLION parts and was the entire cost of verification, because it re-sorted at every node
   what the recursion appends to incrementally. Insert is O(np) memmove against O(np^2).
   `h` is a multiset hash: SUM (not xor - xor cancels on repeated parts) of per-part random
   values, so appending is O(1) where fhash was O(np) on 978M calls. */
typedef struct { unsigned char np; int mass; uint64_t h; Part p[MAXC]; } Chi;

static uint64_t ZOB[NPART];

static void build_zob(void) {
    uint64_t x = 0x243F6A8885A308D3ULL;
    int i;
    for (i = 0; i < NPART; i++) {
        x ^= x << 13; x ^= x >> 7; x ^= x << 17;
        ZOB[i] = x | 1;
    }
}

/* d = s with part (n,m) added, keeping p[] sorted descending. Applies Unit-Group Elimination
   and drops nil parts, exactly as canon() does. */
static inline void chi_add(Chi *d, const Chi *s, int n, int m) {
    int i;
    if (n < m) { int t = n; n = m; m = t; }
    if (m == 0 || n == 0 || (n == 1 && m == 1)) {
        d->np = s->np; d->mass = s->mass; d->h = s->h;
        memcpy(d->p, s->p, s->np * sizeof(Part));
        return;
    }
    for (i = 0; i < s->np; i++)
        if (s->p[i].n < n || (s->p[i].n == n && s->p[i].m < m)) break;
    memcpy(d->p, s->p, i * sizeof(Part));
    d->p[i].n = (unsigned char)n; d->p[i].m = (unsigned char)m;
    memcpy(d->p + i + 1, s->p + i, (s->np - i) * sizeof(Part));
    d->np = (unsigned char)(s->np + 1);
    d->mass = s->mass + n * m;
    d->h = s->h + ZOB[(n << 8) | m];
}

static int pow3[MAXK + 1];
static int gpref[MAXK + 1][1 << MAXK];   /* prefix sums of G_k */
static int glen[MAXK + 1];

/* ---------------------------------------------------------------- G_k */

static void build_G(void) {
    static int cur[1 << MAXK], nxt[1 << MAXK];
    int k, i, len = 1;
    cur[0] = 1;
    glen[0] = 1; gpref[0][0] = 1;
    for (k = 1; k <= MAXK && (1 << k) < (1 << MAXK); k++) {
        int plen = len; len = plen * 2;
        memset(nxt, 0, len * sizeof(int));
        for (i = 0; i < plen; i++) {
            int h = cur[i];
            nxt[i] += h; nxt[i * 2] += h; nxt[i * 2 + 1] += h;
        }
        /* sort descending (insertion: len is small and this runs once per k) */
        for (i = 1; i < len; i++) {
            int v = nxt[i], j = i - 1;
            while (j >= 0 && nxt[j] < v) { nxt[j + 1] = nxt[j]; j--; }
            nxt[j + 1] = v;
        }
        glen[k] = len;
        for (i = 0; i < len; i++) {
            cur[i] = nxt[i];
            gpref[k][i] = nxt[i] + (i ? gpref[k][i - 1] : 0);
        }
    }
}

/* ---------------------------------------------------------------- facts */

static int part_cmp_desc(Part a, Part b) {          /* descending, same key as the solver's sort */
    if (a.n != b.n) return b.n - a.n;
    return b.m - a.m;
}

/* Unit-Group Elimination + orient n>=m + sort descending. Returns 0 if the state is empty. */
static int canon(Part *in, int cnt, Fact *out) {
    int i, j, np = 0;
    for (i = 0; i < cnt; i++) {
        int n = in[i].n, m = in[i].m;
        if (n < m) { int t = n; n = m; m = t; }
        if (n == 0 || m == 0) continue;
        if (n == 1 && m == 1) continue;
        out->p[np].n = (unsigned char)n; out->p[np].m = (unsigned char)m; np++;
    }
    for (i = 1; i < np; i++) {
        Part v = out->p[i];
        for (j = i - 1; j >= 0 && part_cmp_desc(out->p[j], v) > 0; j--) out->p[j + 1] = out->p[j];
        out->p[j + 1] = v;
    }
    out->np = (unsigned char)np;
    out->mass = 0;
    for (i = 0; i < np; i++) out->mass += out->p[i].n * out->p[i].m;
    return np;
}

static uint64_t fhash(const Fact *f) {
    uint64_t h = 0x9E3779B97F4A7C15ULL;
    int i;
    for (i = 0; i < f->np; i++) {
        h ^= (uint64_t)((f->p[i].n << 8) | f->p[i].m) + 0x9E3779B9U + (h << 6) + (h >> 2);
        h *= 0xBF58476D1CE4E5B9ULL;
    }
    return h ? h : 1;
}

static int feq(const Fact *a, const Fact *b) {
    if (a->np != b->np) return 0;
    return memcmp(a->p, b->p, a->np * sizeof(Part)) == 0;
}

/* A frozen, read-only fact set for ONE level. */
typedef struct Level_ {
    int n;
    Fact *f;                 /* sorted by (np, mass) */
    int bstart[MAXP + 2];    /* first index with np >= i */
    int hmask;
    int32_t *hslot;          /* open addressing -> index into f, -1 empty */
    uint64_t *hkey;
    /* Single-part dominance, answered in O(1) instead of by scanning the np=1 bucket.
       sdom[(n<<8)|m] is 1 iff some ONE-part fact (a:b) with a<=n and b<=m is in this level; by
       Subgraph Monotonicity that refutes any state having a part (n:m). Built by a 2-D prefix OR
       over the 1-part facts, so it is exactly equivalent to the scan it replaces.

       This is where the time goes at high k, and it is the opposite of what k=4 suggested. At k=4
       the direct-mapped memo intercepts 99.996% of queries and the index never runs. At k=7 the
       memo hits only 74%: a fact there has ~4 parts of mass ~520, so live_get enumerates ~558
       options per part and issues ~6,700 refutation queries, ~1,700 of which fall through to a
       bucket scan over a 2.5 M-fact level. The enumeration is 25,700 nodes; the scans were 5 s. */
    unsigned char *sdom;
    int32_t *sdom_i;         /* index of a 1-part fact witnessing sdom[(n<<8)|m] */
    unsigned char *cited;    /* painted when this fact is used to discharge something */
    /* A split can usually be discharged several ways. Painting whichever witness the scan happens
       to reach first grows the certificate for no reason: an already-painted fact is free, since
       it is in the artifact and verified regardless. So a later pass searches the previously
       painted facts FIRST, and only falls back to the full level if none of them answers.

       This is greedy set cover, and it is also cheaper rather than more expensive: the painted set
       is ~0.5% of the level, so the preferred lookup is the small one. `sub` is that set as a
       Level in its own right - same index, same code path - and `orig` maps its facts back so a
       hit paints the parent. */
    struct Level_ *sub;
    int32_t *orig;           /* only in a sub: index of each fact in the parent level */

    /* Columnar dominance index. A dominance query is an exact orthogonal range query, not a
       similarity search, so the useful structure is a monotone signature plus a sort order that
       makes the answer a contiguous range.

       The signature: if f <= s by an injection, then taking f's j largest parts by n, they map to
       j distinct parts of s each with a larger n, so s has j parts with n >= N_j(f). Hence
       N_j(f) <= N_j(s) for every j, where N_j is the j-th largest n. The same holds independently
       for the m sides. Both are NECESSARY, cheap, and false for almost every candidate.

       Layout: the canonical fact array remains sorted by (np,largest n,mass), preserving stable
       certificate output and exact hashing. A separate read-only permutation is sorted by
       (np,largest segment product,mass). Its hot scan columns denormalize mass plus three packed
       necessary profiles: eight 8-bit n lanes, eight 8-bit m lanes, and four 16-bit product
       lanes. Almost every candidate dies in those contiguous columns; only survivors touch the
       88-byte Fact and run the exact injection matcher. */
    uint64_t *pn, *pm;       /* per index position: N_1..N_8 and M_1..M_8, descending, 0-padded */
#ifdef VERIFY_PRODUCT_PROFILE
    uint64_t *pp;            /* per fact: four largest n*m products, uint16 lanes, descending */
#endif
#ifdef VERIFY_PRODUCT_SORT
    int32_t *order;           /* product-index position -> canonical f[] index */
    int *imass;               /* mass column in product-index order */
    int32_t *group_end;       /* first index after this fact's equal-max-product group */
#ifdef VERIFY_BLOCK_PARETO
    int nblocks, block_points;
    int32_t *block_at;        /* block id at a summarized block start, -1 elsewhere */
    int32_t *block_off;       /* Pareto points for block b are [off[b],off[b+1]) */
    BlockProfile *block_min;  /* componentwise minima: cheap sound rejection first */
    BlockProfile *block_prof; /* minimal (mass, sorted-product-profile) points */
#endif
#else
    int *b2;                 /* b2[np * 257 + x] = first index with np and largest-n >= x */
#endif
} Level;

typedef unsigned char u8x8 __attribute__((vector_size(8)));
#ifdef VERIFY_PRODUCT_PROFILE
typedef uint16_t u16x4 __attribute__((vector_size(8)));
#endif

/* every lane of a <= corresponding lane of b */
static inline int prof_le(uint64_t a, uint64_t b) {
    u8x8 va, vb, gt;
    uint64_t r;
    memcpy(&va, &a, 8); memcpy(&vb, &b, 8);
    gt = (u8x8)(va > vb);
    memcpy(&r, &gt, 8);
    return r == 0;
}

#ifdef VERIFY_PRODUCT_PROFILE
/* Four sorted segment products. A componentwise injection maps every segment to one with a
   no-smaller product, so the sorted product vector is a necessary dominance condition. Four lanes
   cover the entire run9 k=6 index serving the expensive k=7 verification batch; longer states get
   a still-sound, weaker top-four filter. */
static inline int prod_prof_le(uint64_t a, uint64_t b) {
    u16x4 va, vb, gt;
    uint64_t r;
    memcpy(&va, &a, 8); memcpy(&vb, &b, 8);
    gt = (u16x4)(va > vb);
    memcpy(&r, &gt, 8);
    return r == 0;
}

static uint64_t prod_prof_of(const Part *p, int np) {
    uint16_t best[4] = {0, 0, 0, 0};
    uint64_t out;
    int i, j;
    for (i = 0; i < np; i++) {
        uint16_t v = (uint16_t)(p[i].n * p[i].m);
        for (j = 0; j < 4; j++) if (v > best[j]) {
            int z;
            for (z = 3; z > j; z--) best[z] = best[z - 1];
            best[j] = v;
            break;
        }
    }
    memcpy(&out, best, 8);
    return out;
}
#endif

/* N_1..N_8 and M_1..M_8, each descending and 0-padded. p[] is sorted by (n desc, m desc), so the
   n side is already ordered; the m side is not and must be sorted separately. */
static void prof_of(const Part *p, int np, uint64_t *pn, uint64_t *pm) {
    unsigned char bn[8] = {0}, bm[8] = {0}, ms[MAXP];
    int i, j, lim = np < 8 ? np : 8;
    for (i = 0; i < lim; i++) bn[i] = p[i].n;
    for (i = 0; i < np; i++) ms[i] = p[i].m;
    for (i = 1; i < np; i++) {                       /* insertion sort, descending */
        unsigned char v = ms[i];
        for (j = i - 1; j >= 0 && ms[j] < v; j--) ms[j + 1] = ms[j];
        ms[j + 1] = v;
    }
    for (i = 0; i < lim; i++) bm[i] = ms[i];
    memcpy(pn, bn, 8); memcpy(pm, bm, 8);
}

#ifdef VERIFY_PRODUCT_SORT
static int fact_max_product(const Fact *f) {
    int i, out = 0;
    for (i = 0; i < f->np; i++) {
        int v = f->p[i].n * f->p[i].m;
        if (v > out) out = v;
    }
    return out;
}
#endif

static int fact_cmp(const void *A, const void *B) {
    const Fact *a = A, *b = B;
    if (a->np != b->np) return a->np - b->np;
    { int an = a->np ? a->p[0].n : 0, bn = b->np ? b->p[0].n : 0;   /* largest n */
      if (an != bn) return an - bn; }
    if (a->mass != b->mass) return a->mass - b->mass;
    return memcmp(a->p, b->p, (a->np < b->np ? a->np : b->np) * sizeof(Part));
}

#ifdef VERIFY_PRODUCT_SORT
static const Fact *g_index_sort_facts;

static int fact_index_cmp(const void *A, const void *B) {
    int32_t ai, bi;
    const Fact *a, *b;
    memcpy(&ai, A, sizeof ai); memcpy(&bi, B, sizeof bi);
    a = &g_index_sort_facts[ai]; b = &g_index_sort_facts[bi];
    if (a->np != b->np) return a->np - b->np;
    { int ap = fact_max_product(a), bp = fact_max_product(b);
      if (ap != bp) return ap - bp; }
    if (a->mass != b->mass) return a->mass - b->mass;
    return fact_cmp(a, b);
}
#endif

#ifdef VERIFY_BLOCK_PARETO
static long long g_block_build_blocks, g_block_build_points, g_block_build_facts;
static int g_block_build_max_front;
static double g_block_build_seconds;

static inline int block_profile_le(const BlockProfile *a, const BlockProfile *b) {
    return a->mass <= b->mass && prod_prof_le(a->pp, b->pp);
}

static inline void block_profile_take_min(BlockProfile *a, const BlockProfile *b) {
    uint16_t aa[4], bb[4];
    int z;
    if (b->mass < a->mass) a->mass = b->mass;
    memcpy(aa, &a->pp, 8); memcpy(bb, &b->pp, 8);
    for (z = 0; z < 4; z++) if (bb[z] < aa[z]) aa[z] = bb[z];
    memcpy(&a->pp, aa, 8);
}

/* A block can contain a useful fact only if one of its Pareto-minimal profile points fits the
   query. If an arbitrary point x fits, repeatedly replacing a non-minimal x by a smaller point
   terminates at a stored minimal point that also fits. The summary is therefore a sound rejection
   filter; a positive answer merely falls back to the unchanged per-fact and exact checks. */
static void level_build_blocks(Level *L) {
    int np, b = 0, used = 0, maxfront = 0;
    clock_t t0 = clock();
    L->nblocks = L->block_points = 0;
    L->block_at = NULL; L->block_off = NULL;
    L->block_min = NULL; L->block_prof = NULL;
    /* On smaller levels the block probe costs more than the candidates it avoids.  The
       production-sized run9 k=7 level is comfortably above this cutoff, while the expensive
       Sa(113) k=6 replay level is below it and retains the unmodified product-index hot path. */
    if (L->n < VERIFY_BLOCK_MIN_LEVEL_FACTS) return;
    L->block_at = malloc((size_t)(L->n ? L->n : 1) * sizeof(int32_t));
    L->block_prof = malloc((size_t)(L->n ? L->n : 1) * sizeof(BlockProfile));
    for (np = 0; np < L->n; np++) L->block_at[np] = -1;
    for (np = 0; np <= MAXP; np++) {
        int hi = L->bstart[np + 1], group = L->bstart[np];
        while (group < hi) {
            int gend = L->group_end[group], first;
            for (first = group; first + VERIFY_BLOCK_SIZE <= gend;
                 first += VERIFY_BLOCK_SIZE) b++;
            group = gend;
        }
    }
    L->nblocks = b;
    L->block_off = malloc((size_t)(b + 1) * sizeof(int32_t));
    L->block_min = malloc((size_t)(b ? b : 1) * sizeof(BlockProfile));
    b = 0;
    for (np = 0; np <= MAXP; np++) {
        int hi = L->bstart[np + 1], group = L->bstart[np];
        while (group < hi) {
            int gend = L->group_end[group], first;
            for (first = group; first + VERIFY_BLOCK_SIZE <= gend;
                 first += VERIFY_BLOCK_SIZE) {
                int end = first + VERIFY_BLOCK_SIZE, pos, nf = 0;
                BlockProfile *front = L->block_prof + used;
                L->block_at[first] = b;
                L->block_off[b] = used;
                for (pos = first; pos < end; pos++) {
                    BlockProfile x;
                    int z, redundant = 0;
                    x.pp = L->pp[pos]; x.mass = L->imass[pos];
                    if (pos == first) L->block_min[b] = x;
                    else block_profile_take_min(&L->block_min[b], &x);
                    for (z = 0; z < nf; z++) if (block_profile_le(&front[z], &x)) {
                        redundant = 1; break;
                    }
                    if (redundant) continue;
                    for (z = 0; z < nf; ) {
                        if (block_profile_le(&x, &front[z])) front[z] = front[--nf];
                        else z++;
                    }
                    front[nf++] = x;
                }
                used += nf;
                if (nf > maxfront) maxfront = nf;
                b++;
            }
            group = gend;
        }
    }
    L->block_off[b] = used;
    L->block_points = used;
    if (used < L->n) {
        BlockProfile *p = realloc(L->block_prof,
                                  (size_t)(used ? used : 1) * sizeof(BlockProfile));
        if (p) L->block_prof = p;
    }
    g_block_build_blocks += L->nblocks;
    g_block_build_points += used;
    g_block_build_facts += L->n;
    if (maxfront > g_block_build_max_front) g_block_build_max_front = maxfront;
    g_block_build_seconds += (double)(clock() - t0) / CLOCKS_PER_SEC;
}
#endif

static void level_freeze(Level *L) {
    int i, cap = 16;
    qsort(L->f, L->n, sizeof(Fact), fact_cmp);
    for (i = 0; i <= MAXP + 1; i++) L->bstart[i] = L->n;
    for (i = L->n - 1; i >= 0; i--) L->bstart[L->f[i].np] = i;
    for (i = MAXP; i >= 0; i--) if (L->bstart[i] > L->bstart[i + 1]) L->bstart[i] = L->bstart[i + 1];
    while (cap < L->n * 2) cap <<= 1;
    L->hmask = cap - 1;
    L->hslot = malloc(cap * sizeof(int32_t));
    L->hkey = malloc(cap * sizeof(uint64_t));
    for (i = 0; i < cap; i++) L->hslot[i] = -1;
    for (i = 0; i < L->n; i++) {
        uint64_t h = fhash(&L->f[i]);
        int s = (int)(h & L->hmask);
        while (L->hslot[s] >= 0) s = (s + 1) & L->hmask;
        L->hslot[s] = i; L->hkey[s] = h;
    }
    L->pn = malloc((L->n ? L->n : 1) * sizeof(uint64_t));
    L->pm = malloc((L->n ? L->n : 1) * sizeof(uint64_t));
#ifdef VERIFY_PRODUCT_PROFILE
    L->pp = malloc((L->n ? L->n : 1) * sizeof(uint64_t));
#endif
#ifdef VERIFY_PRODUCT_SORT
    L->order = malloc((L->n ? L->n : 1) * sizeof(int32_t));
    L->imass = malloc((L->n ? L->n : 1) * sizeof(int));
    for (i = 0; i < L->n; i++) L->order[i] = i;
    g_index_sort_facts = L->f;
    qsort(L->order, L->n, sizeof(int32_t), fact_index_cmp);
    g_index_sort_facts = NULL;
#endif
    for (i = 0; i < L->n; i++) {
        int fi = i;
#ifdef VERIFY_PRODUCT_SORT
        fi = L->order[i]; L->imass[i] = L->f[fi].mass;
#endif
        prof_of(L->f[fi].p, L->f[fi].np, &L->pn[i], &L->pm[i]);
#ifdef VERIFY_PRODUCT_PROFILE
        L->pp[i] = prod_prof_of(L->f[fi].p, L->f[fi].np);
#endif
    }
#ifdef VERIFY_PRODUCT_SORT
    L->group_end = malloc((L->n ? L->n : 1) * sizeof(int32_t));
    { int np;
      for (np = 0; np <= MAXP; np++) {
          int lo = L->bstart[np], hi = L->bstart[np + 1], a = lo;
          while (a < hi) {
              int b = a + 1, z, prod = fact_max_product(&L->f[L->order[a]]);
              while (b < hi && fact_max_product(&L->f[L->order[b]]) == prod) b++;
              for (z = a; z < b; z++) L->group_end[z] = b;
              a = b;
          }
      }
    }
#ifdef VERIFY_BLOCK_PARETO
    level_build_blocks(L);
#endif
#else
    L->b2 = malloc((MAXP + 2) * 257 * sizeof(int));
    { int np, x;
      for (np = 0; np <= MAXP + 1; np++)
          for (x = 0; x <= 256; x++) L->b2[np * 257 + x] = L->bstart[np < MAXP + 1 ? np + 1 : np];
      for (np = 0; np <= MAXP; np++) {
          int lo = L->bstart[np], hi = L->bstart[np + 1];
          for (x = 256; x >= 0; x--) L->b2[np * 257 + x] = hi;
          for (i = hi - 1; i >= lo; i--) {
              int an = L->f[i].np ? L->f[i].p[0].n : 0;
              L->b2[np * 257 + an] = i;
          }
          for (x = 255; x >= 0; x--)                       /* first index with largest-n >= x */
              if (L->b2[np * 257 + x] > L->b2[np * 257 + x + 1])
                  L->b2[np * 257 + x] = L->b2[np * 257 + x + 1];
      }
    }
#endif
    L->sdom = calloc(NPART, 1);
    L->sdom_i = malloc(NPART * sizeof(int32_t));
    L->cited = calloc(L->n ? L->n : 1, 1);
    for (i = 0; i < NPART; i++) L->sdom_i[i] = -1;
    for (i = L->bstart[1]; i < L->bstart[2]; i++) {         /* the 1-part facts */
        int ix = (L->f[i].p[0].n << 8) | L->f[i].p[0].m;
        L->sdom[ix] = 1; L->sdom_i[ix] = i;
    }
    { int n, m;                                             /* 2-D prefix OR, carrying a witness */
      for (n = 0; n < 256; n++)
          for (m = 0; m < 256; m++) {
              int ix = (n << 8) | m;
              if (!L->sdom[ix] && n && L->sdom[((n - 1) << 8) | m]) {
                  L->sdom[ix] = 1; L->sdom_i[ix] = L->sdom_i[((n - 1) << 8) | m]; }
              if (!L->sdom[ix] && m && L->sdom[(n << 8) | (m - 1)]) {
                  L->sdom[ix] = 1; L->sdom_i[ix] = L->sdom_i[(n << 8) | (m - 1)]; }
          }
    }
}

/* Candidate range and equal-primary-key skip for the production and legacy static orders. */
static int level_candidate_end(const Level *L, int np, const Fact *q) {
#ifdef VERIFY_PRODUCT_SORT
    int lo = L->bstart[np], hi = L->bstart[np + 1];
    int maxprod = fact_max_product(q);
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (fact_max_product(&L->f[L->order[mid]]) <= maxprod) lo = mid + 1;
        else hi = mid;
    }
    return lo;
#else
    int maxn = q->np ? q->p[0].n : 0;
    return L->b2[np * 257 + (maxn < 256 ? maxn + 1 : 256)];
#endif
}

static inline int level_fact_index(const Level *L, int pos) {
#ifdef VERIFY_PRODUCT_SORT
    return L->order[pos];
#else
    (void)L;
    return pos;
#endif
}

static inline int level_index_mass(const Level *L, int pos) {
#ifdef VERIFY_PRODUCT_SORT
    return L->imass[pos];
#else
    return L->f[pos].mass;
#endif
}

static int level_next_primary_group(const Level *L, int np, int i) {
#ifdef VERIFY_PRODUCT_SORT
    (void)np;
    return L->group_end[i];
#else
    int an = L->f[i].p[0].n;
    return L->b2[np * 257 + (an < 256 ? an + 1 : 256)];
#endif
}

#ifdef VERIFY_BLOCK_PARETO
static int level_block_possible(const Level *L, int pos, const BlockProfile *q,
                                int *tested, int *min_reject) {
    int b = L->block_at[pos];
    int z;
    *tested = 0;
    *min_reject = 0;
    if (!block_profile_le(&L->block_min[b], q)) {
        *min_reject = 1;
        return 0;
    }
    for (z = L->block_off[b]; z < L->block_off[b + 1]; z++) {
        (*tested)++;
        if (block_profile_le(&L->block_prof[z], q)) return 1;
    }
    return 0;
}
#endif

static void level_freeze(Level *L);

/* the painted facts of L, as a Level of their own */
static Level *level_sub_cited(Level *L) {
    int i, c = 0;
    for (i = 0; i < L->n; i++) c += L->cited[i] ? 1 : 0;
    if (!c) return NULL;
    Level *S = calloc(1, sizeof(Level));
    S->f = malloc((size_t)c * sizeof(Fact));
    S->orig = malloc((size_t)c * sizeof(int32_t));
    int w = 0;
    for (i = 0; i < L->n; i++)
        if (L->cited[i]) { S->f[w] = L->f[i]; S->orig[w] = i; w++; }
    S->n = w;
    /* level_freeze sorts, so the orig mapping must be permuted with it: tag each fact by writing
       its parent index into a side array keyed on identity after the sort. Simplest correct way is
       to sort a paired array ourselves, so freeze sees an already-sorted input and leaves it. */
    { int a, b;
      for (a = 1; a < S->n; a++) {                        /* insertion sort keeps pairs together */
          Fact fv = S->f[a]; int32_t ov = S->orig[a];
          for (b = a - 1; b >= 0 && fact_cmp(&S->f[b], &fv) > 0; b--) {
              S->f[b + 1] = S->f[b]; S->orig[b + 1] = S->orig[b];
          }
          S->f[b + 1] = fv; S->orig[b + 1] = ov;
      }
    }
    level_freeze(S);
    return S;
}

static int level_find(const Level *L, const Fact *q) {
    uint64_t h = fhash(q);
    int s = (int)(h & L->hmask);
    while (L->hslot[s] >= 0) {
        if (L->hkey[s] == h && feq(&L->f[L->hslot[s]], q)) return L->hslot[s];
        s = (s + 1) & L->hmask;
    }
    return -1;
}

/* Subgraph Monotonicity: is there an injection a -> b with each part componentwise <= ? */
static int dom_rec(const Fact *a, const Fact *b, int i, unsigned used) {
    if (i == a->np) return 1;
    int j;
    for (j = 0; j < b->np; j++) {
        if (used >> j & 1) continue;
        if (a->p[i].n <= b->p[j].n && a->p[i].m <= b->p[j].m)
            if (dom_rec(a, b, i + 1, used | 1u << j)) return 1;
    }
    return 0;
}

static int dominates(const Fact *a, const Fact *b) {
    if (a->np > b->np || a->mass > b->mass) return 0;
    if (b->np > 31) return 0;                 /* bitmask limit; falls back to no-claim */
    return dom_rec(a, b, 0, 0);
}

/* Is fact q redundant because another logged fact at the same k is a componentwise substate? */
static int level_redundant(const Level *L, int q) {
    const Fact *f = &L->f[q];
    int lim = f->np < MAXP ? f->np : MAXP, np, i;
    uint64_t qn, qm;
#ifdef VERIFY_PRODUCT_PROFILE
    uint64_t qp = prod_prof_of(f->p, f->np);
#endif
#ifdef VERIFY_BLOCK_PARETO
    BlockProfile bq;
    bq.pp = qp; bq.mass = f->mass;
#endif
    prof_of(f->p, f->np, &qn, &qm);
    for (np = 1; np <= lim; np++) {
        int end = level_candidate_end(L, np, f);
        i = L->bstart[np];
        while (i < end) {
            int fi = level_fact_index(L, i);
            if (level_index_mass(L, i) > f->mass) {
                int nxt = level_next_primary_group(L, np, i);
                if (nxt <= i) break;
                i = nxt; continue;
            }
#ifdef VERIFY_BLOCK_PARETO
            if (L->block_at && L->block_at[i] >= 0 && i + VERIFY_BLOCK_SIZE <= end) {
                int tested, min_reject;
                if (!level_block_possible(L, i, &bq, &tested, &min_reject)) {
                    i += VERIFY_BLOCK_SIZE; continue;
                }
            }
#endif
            if (fi == q) { i++; continue; }
#ifdef VERIFY_PRODUCT_PROFILE
            if (!prod_prof_le(L->pp[i], qp)) { i++; continue; }
#endif
            if (!prof_le(L->pn[i], qn) || !prof_le(L->pm[i], qm)) { i++; continue; }
            if (dominates(&L->f[fi], f)) return 1;
            i++;
        }
    }
    return 0;
}

typedef struct {
    const Level *level;
    unsigned char *keep;
    atomic_int next;
    atomic_int done;
    int k, threads;
    double started;
    struct MinWorkerProgress_ *progress;
    pthread_mutex_t progress_mu;
    pthread_cond_t progress_cv;
    int progress_stop;
} MinBatch;

typedef struct MinWorkerProgress_ {
    atomic_int task;                  /* -1 while idle */
    atomic_ullong started_ns;
} MinWorkerProgress;

typedef struct {
    MinBatch *batch;
    int slot;
} MinWorkerArg;

static void *minimize_worker(void *vp) {
    MinWorkerArg *a = vp;
    MinBatch *b = a->batch;
    MinWorkerProgress *p = &b->progress[a->slot];
    int q;
    while ((q = atomic_fetch_add_explicit(&b->next, 1, memory_order_relaxed)) < b->level->n) {
        atomic_store_explicit(&p->started_ns, monotonic_nanoseconds(), memory_order_relaxed);
        atomic_store_explicit(&p->task, q, memory_order_release);
        b->keep[q] = (unsigned char)!level_redundant(b->level, q);
        atomic_store_explicit(&p->task, -1, memory_order_release);
        atomic_fetch_add_explicit(&b->done, 1, memory_order_release);
    }
    return NULL;
}

static void print_minimize_progress(MinBatch *b, int *last_done, double *last_time) {
    double now = monotonic_seconds(), elapsed = now - b->started;
    int total = b->level->n;
    int done = atomic_load_explicit(&b->done, memory_order_acquire);
    int claimed = atomic_load_explicit(&b->next, memory_order_relaxed);
    int active = 0, oldest_slot = -1, oldest_task = -1, i;
    uint64_t now_ns = monotonic_nanoseconds(), oldest_age_ns = 0;
    if (claimed > total) claimed = total;
    for (i = 0; i < b->threads; i++) {
        int q = atomic_load_explicit(&b->progress[i].task, memory_order_acquire);
        if (q < 0 || q >= total) continue;
        uint64_t started = atomic_load_explicit(&b->progress[i].started_ns,
                                                memory_order_relaxed);
        uint64_t age = now_ns > started ? now_ns - started : 0;
        if (atomic_load_explicit(&b->progress[i].task, memory_order_acquire) != q) continue;
        active++;
        if (age > oldest_age_ns) {
            oldest_age_ns = age; oldest_slot = i; oldest_task = q;
        }
    }
    double total_rate = done / (elapsed > 0 ? elapsed : 1);
    double window_elapsed = now - *last_time;
    double window_rate = (done - *last_done) / (window_elapsed > 0 ? window_elapsed : 1);
    printf("PROGRESS phase=minimize k=%d elapsed_s=%.1f completed=%d/%d percent=%.4f "
           "claimed=%d active=%d queued=%d rate_total=%.3f/s rate_window=%.3f/s ",
           b->k, elapsed, done, total, 100.0 * done / (total ? total : 1),
           claimed, active, total - claimed, total_rate, window_rate);
    if (total_rate > 0) printf("eta_total_s=%.0f ", (total - done) / total_rate);
    else printf("eta_total_s=unknown ");
    if (window_rate > 0) printf("eta_recent_s=%.0f\n", (total - done) / window_rate);
    else printf("eta_recent_s=unknown\n");
    if (oldest_slot >= 0) {
        int q = atomic_load_explicit(&b->progress[oldest_slot].task, memory_order_acquire);
        if (q == oldest_task && q >= 0 && q < total) {
            const Fact *f = &b->level->f[q];
            printf("PROGRESS_ACTIVE phase=minimize k=%d worker=%d task=%d age_s=%.1f "
                   "np=%d mass=%d state=", b->k, oldest_slot, q,
                   oldest_age_ns * 1e-9, f->np, f->mass);
            write_fact_inline(stdout, f);
            fputc('\n', stdout);
        }
    }
    fflush(stdout);
    *last_done = done;
    *last_time = now;
}

static void *minimize_progress_reporter(void *vp) {
    MinBatch *b = vp;
    int last_done = 0;
    double last_time = b->started;
    pthread_mutex_lock(&b->progress_mu);
    while (!b->progress_stop) {
        struct timespec until = realtime_after(g_progress_seconds);
        int rc = 0;
        while (!b->progress_stop && rc != ETIMEDOUT)
            rc = pthread_cond_timedwait(&b->progress_cv, &b->progress_mu, &until);
        if (b->progress_stop) break;
        pthread_mutex_unlock(&b->progress_mu);
        print_minimize_progress(b, &last_done, &last_time);
        pthread_mutex_lock(&b->progress_mu);
    }
    pthread_mutex_unlock(&b->progress_mu);
    return NULL;
}

static void level_drop_indexes(Level *L) {
    free(L->hslot); free(L->hkey); free(L->sdom); free(L->sdom_i);
    free(L->cited); free(L->pn); free(L->pm);
#ifdef VERIFY_PRODUCT_PROFILE
    free(L->pp);
#endif
#ifdef VERIFY_PRODUCT_SORT
    free(L->order); free(L->imass);
    free(L->group_end);
#ifdef VERIFY_BLOCK_PARETO
    free(L->block_at); free(L->block_off); free(L->block_min); free(L->block_prof);
#endif
#else
    free(L->b2);
#endif
    L->hslot = NULL; L->hkey = NULL; L->sdom = NULL; L->sdom_i = NULL;
    L->cited = NULL; L->pn = NULL; L->pm = NULL;
#ifdef VERIFY_PRODUCT_PROFILE
    L->pp = NULL;
#endif
#ifdef VERIFY_PRODUCT_SORT
    L->order = NULL; L->imass = NULL; L->group_end = NULL;
#ifdef VERIFY_BLOCK_PARETO
    L->block_at = NULL; L->block_off = NULL;
    L->block_min = NULL; L->block_prof = NULL;
    L->nblocks = L->block_points = 0;
#endif
#else
    L->b2 = NULL;
#endif
}

static int level_minimize(Level *L, int threads, int k) {
    int i, w = 0;
    if (L->n < 2) return 0;
    unsigned char *keep = malloc((size_t)L->n);
    if (!keep) { fprintf(stderr, "no memory for minimalization marks\n"); exit(1); }
    if (threads < 1) threads = 1;
    if (threads > L->n) threads = L->n;
    MinBatch b = { .level = L, .keep = keep, .k = k, .threads = threads,
                   .started = monotonic_seconds() };
    atomic_init(&b.next, 0);
    atomic_init(&b.done, 0);
    b.progress = calloc((size_t)threads, sizeof *b.progress);
    MinWorkerArg *args = calloc((size_t)threads, sizeof *args);
    pthread_t *ids = threads > 1 ? malloc((size_t)(threads - 1) * sizeof *ids) : NULL;
    pthread_t reporter;
    int have_reporter = 0;
    if (!b.progress || !args || (threads > 1 && !ids)) {
        fprintf(stderr, "no memory for minimalization workers\n"); exit(1);
    }
    for (i = 0; i < threads; i++) {
        atomic_init(&b.progress[i].task, -1);
        atomic_init(&b.progress[i].started_ns, 0);
        args[i].batch = &b; args[i].slot = i;
    }
    printf("BATCH_START phase=minimize k=%d targets=%d threads=%d progress_seconds=%.1f\n",
           k, L->n, threads, g_progress_seconds);
    fflush(stdout);
    if (g_progress_seconds > 0) {
        pthread_mutex_init(&b.progress_mu, NULL);
        pthread_cond_init(&b.progress_cv, NULL);
        if (pthread_create(&reporter, NULL, minimize_progress_reporter, &b)) {
            fprintf(stderr, "cannot create minimalization progress reporter\n"); exit(1);
        }
        have_reporter = 1;
    }
    for (i = 0; i < threads - 1; i++)
        if (pthread_create(&ids[i], NULL, minimize_worker, &args[i])) {
            fprintf(stderr, "cannot create minimalization worker %d\n", i); exit(1);
        }
    minimize_worker(&args[threads - 1]);
    for (i = 0; i < threads - 1; i++) pthread_join(ids[i], NULL);
    if (have_reporter) {
        pthread_mutex_lock(&b.progress_mu);
        b.progress_stop = 1;
        pthread_cond_signal(&b.progress_cv);
        pthread_mutex_unlock(&b.progress_mu);
        pthread_join(reporter, NULL);
        pthread_cond_destroy(&b.progress_cv);
        pthread_mutex_destroy(&b.progress_mu);
    }
    printf("BATCH_DONE phase=minimize k=%d completed=%d/%d wall_s=%.2f\n",
           k, atomic_load(&b.done), L->n, monotonic_seconds() - b.started);
    fflush(stdout);
    free(ids);
    free(args);
    free(b.progress);
    for (i = 0; i < L->n; i++) if (keep[i]) L->f[w++] = L->f[i];
    free(keep);
    i = L->n - w;
    level_drop_indexes(L);
    L->n = w;
    level_freeze(L);
    return i;
}

/* Singleton Majorization applied to the full star expansion.  Replace each oriented part
   (n:m), n >= m, by m disjoint copies of (n:1).  This lift has one edge for every original edge:
   map every cloned n-side vertex back to its source and pull each original test back to all clones.
   Corresponding edges then have identical transcripts, so solvability of the original state implies
   solvability of the lift.  A majorization violation in the lift therefore refutes the original.

   This strictly dominates the 2026-08-06 one-copy downgrade, which kept only one (n:1) per part.
   Parts are sorted by n already, so repeat n exactly m times while scanning the prefixes; no expanded
   state needs to be materialised.

   The right-hand side must be CLAMPED past len(G_k), not treated as a violation. The theorem pads
   with trailing zeros, so for t > len(G_k) the bound is the constant sum G_k = 3^k, and the
   expanded n-sum is at most the mass, which COUNT has already bounded by 3^k - hence no violation
   can occur there. An earlier version returned "refuted" once i reached len(G_k); that was latent
   only because no logged state had more than 2^k singleton parts; star expansion makes the tail
   routine, so retaining that bug would produce wrong refutations. */
static int maj_refutes(const Fact *s, int k) {
    int i, rank = 0, run = 0, cap = gpref[k][glen[k] - 1];
    if (!s->np) return 0;
    /* parts are sorted descending by (n,m), so repeated n values are already nonincreasing */
    for (i = 0; i < s->np; i++) {
        int copies = s->p[i].m;
        while (copies-- > 0) {
            run += s->p[i].n;
            if (run > (rank < glen[k] ? gpref[k][rank] : cap)) return 1;
            rank++;
        }
    }
    return 0;
}

/* Direct-mapped memo for refuted(). Children recur across enormous numbers of splits, so this
   carries most of the load; Python's equivalent was Index.memo. It is SOUND because a hit is
   confirmed by comparing the full state - a hash collision costs a miss, never a wrong answer.
   States with more than MEMOP parts are simply not memoised. */
#ifndef MEMOBITS
#define MEMOBITS 24
#endif
#define MEMOP 16
/* `wit` is the index of the fact that answered this query, or -1 for a rule (COUNT, MAJ) or a
   derivation. Carrying it through the memo is what makes top-down painting possible: 99.996% of
   queries at k=4 are memo hits, so without it almost every citation would go unrecorded. */
typedef struct { uint64_t h; int32_t wit; unsigned char np, k; signed char res; Part p[MEMOP]; } MemoEnt;
static TLS int g_wit = -1;
static int g_paint = 0;
static TLS long long paint_hits = 0, pref_hits = 0, pref_miss = 0;
static TLS MemoEnt *memo;
static TLS size_t memo_mask;
#define C_REF
#define MEMO_HIT memo_hit++
#define MEMO_MISS memo_miss++
static TLS long long memo_hit = 0, memo_miss = 0;
#ifdef VERIFY_INDEX_STATS
static TLS long long idx_candidates = 0, idx_product_rejects = 0, idx_nm_rejects = 0;
static TLS long long idx_match_calls = 0, idx_match_hits = 0;
#ifdef VERIFY_BLOCK_PARETO
static TLS long long idx_block_tests = 0, idx_block_rejects = 0, idx_block_min_rejects = 0;
static TLS long long idx_block_skipped = 0, idx_block_front_tests = 0;
#endif
#define IDX_INC(x) ((x)++)
#define IDX_ADD(x, n) ((x) += (n))
#else
#define IDX_INC(x) ((void)0)
#define IDX_ADD(x, n) ((void)0)
#endif

static int refuted_raw(const Level *L, const Fact *s, int k);
static int derive(const Fact *s, int k);      /* prove, rather than cite */
static const void *g_levels;                  /* the Level[] array, for derive() */
static int g_derive = 0;          /* derive missing facts at k <= this */
static TLS long long derived_ok = 0, derived_no = 0;

static int refuted(const Level *L, const Fact *s, int k) {
    if (s->np > MEMOP) return refuted_raw(L, s, k);
    uint64_t h = fhash(s) + 0x51ULL * (uint64_t)k;
    MemoEnt *e = &memo[h & memo_mask];
    if (e->res >= 0 && e->h == h && e->k == (unsigned char)k && e->np == s->np
            && memcmp(e->p, s->p, s->np * sizeof(Part)) == 0) {
        memo_hit++;
        if (g_paint && e->res && e->wit >= 0) {
            __atomic_store_n(&L->cited[e->wit], 1, __ATOMIC_RELAXED); paint_hits++;
        }
        return e->res;
    }
    memo_miss++;
    int r = refuted_raw(L, s, k);
    e->h = h; e->k = (unsigned char)k; e->np = s->np; e->res = (signed char)r;
    e->wit = (int32_t)(r ? g_wit : -1);
    memcpy(e->p, s->p, s->np * sizeof(Part));
    if (g_paint && r && g_wit >= 0) {
        __atomic_store_n(&L->cited[g_wit], 1, __ATOMIC_RELAXED); paint_hits++;
    }
    return r;
}

/* Finish the unchanged per-fact part of a dominance lookup.  Keeping this in one force-inlined
   helper lets refuted_raw select the plain or block scan once per query, outside the candidate
   loop, without maintaining two copies of the exact filters. */
static inline __attribute__((always_inline)) int dominance_candidate(
        const Level *L, const Fact *s, int i, uint64_t qn, uint64_t qm, uint64_t qp) {
    int fi;
    IDX_INC(idx_candidates);
#ifdef VERIFY_PRODUCT_PROFILE
    if (!prod_prof_le(L->pp[i], qp)) {
        IDX_INC(idx_product_rejects);
        return -1;
    }
#else
    (void)qp;
#endif
    if (!prof_le(L->pn[i], qn) || !prof_le(L->pm[i], qm)) {
        IDX_INC(idx_nm_rejects);
        return -1;
    }
    fi = level_fact_index(L, i);
    IDX_INC(idx_match_calls);
    if (dominates(&L->f[fi], s)) {
        IDX_INC(idx_match_hits);
        return fi;
    }
    return -1;
}

static int refuted_raw(const Level *L, const Fact *s, int k) {
    g_wit = -1;
    if (s->mass > pow3[k]) return 1;                 /* COUNT - a rule, no fact to paint */
    if (s->np == 0) return 0;                        /* solved */
    if (L->sub) {                                    /* prefer a witness already in the artifact */
        if (refuted_raw(L->sub, s, k)) {
            if (g_wit >= 0) g_wit = L->sub->orig[g_wit];   /* back to the parent's numbering */
            pref_hits++;
            return 1;
        }
        g_wit = -1;
        pref_miss++;
    }
    { int ix = level_find(L, s);
      if (ix >= 0) { g_wit = ix; return 1; } }
    if (maj_refutes(s, k)) return 1;                 /* MAJ (full star expansion) */
    int lim = s->np < MAXP ? s->np : MAXP, np, i;    /* DOM */
    if (L->sdom) {                                   /* one-part dominance, O(np) */
        for (i = 0; i < s->np; i++) {
            int ix = (s->p[i].n << 8) | s->p[i].m;
            if (L->sdom[ix]) { g_wit = L->sdom_i[ix]; return 1; }
        }
    }
    { uint64_t qn, qm, qp = 0;
#ifdef VERIFY_PRODUCT_PROFILE
      qp = prod_prof_of(s->p, s->np);
#endif
#ifdef VERIFY_BLOCK_PARETO
      BlockProfile bq;
      bq.pp = qp; bq.mass = s->mass;
#endif
      prof_of(s->p, s->np, &qn, &qm);
#ifdef VERIFY_BLOCK_PARETO
      if (L->block_at) {
        for (np = 2; np <= lim; np++) {
          int start = L->bstart[np], end = level_candidate_end(L, np, s);
          i = start;
          while (i < end) {
              int hit;
              if (level_index_mass(L, i) > s->mass) {
                  int nxt = level_next_primary_group(L, np, i);
                  if (nxt <= i) break;
                  i = nxt;
                  continue;
              }
              if (L->block_at[i] >= 0 && i + VERIFY_BLOCK_SIZE <= end) {
                  int tested, min_reject;
                  IDX_INC(idx_block_tests);
                  if (!level_block_possible(L, i, &bq, &tested, &min_reject)) {
                      IDX_INC(idx_block_rejects);
                      IDX_ADD(idx_block_min_rejects, min_reject);
                      IDX_ADD(idx_block_skipped, VERIFY_BLOCK_SIZE);
                      IDX_ADD(idx_block_front_tests, tested);
                      i += VERIFY_BLOCK_SIZE;
                      continue;
                  }
                  IDX_ADD(idx_block_front_tests, tested);
              }
              hit = dominance_candidate(L, s, i, qn, qm, qp);
              if (hit >= 0) { g_wit = hit; return 1; }
              i++;
          }
        }
      } else
#endif
      {
      for (np = 2; np <= lim; np++) {
          /* The primary monotone key (largest n in the baseline, largest product in the
             experiment) bounds a contiguous range. Mass is ordered inside each primary-key
             group, so an over-mass candidate skips the rest of that group. */
          int start = L->bstart[np], end = level_candidate_end(L, np, s);
          i = start;
          while (i < end) {
              int hit;
              if (level_index_mass(L, i) > s->mass) { /* skip to the next primary-key group */
                  int nxt = level_next_primary_group(L, np, i);
                  if (nxt <= i) break;
                  i = nxt;
                  continue;
              }
              hit = dominance_candidate(L, s, i, qn, qm, qp);
              if (hit >= 0) { g_wit = hit; return 1; }
              i++;
          }
      }
      }
    }
    /* DERIVE. Nothing in the fact set refutes s, so try to PROVE it instead of citing it: run
       the same SPLITS check on s itself, one level down. This closes the bottom of the DAG
       without anyone having to ship it. It matters because a resumed solver run carries facts in
       its warm cache whose own logs are gone, so its output is NOT closed - measured on the 2023
       Sa(193) corpus, where ~5% of k=5 facts cite a k=4 child that was never logged. At low k
       the derivation costs milliseconds, so the certificate need only carry the facts that are
       expensive to re-derive.

       Sound by construction: deriving is proving, and a derived fact is checked by exactly the
       rules a shipped one would be. It is not a weakening of the trust base, it is a shrinking
       of the artifact. */
    if (k >= 2 && k <= g_derive) {
        int r = derive(s, k);
        if (r) derived_ok++; else derived_no++;
        return r;
    }
    return 0;
}

/* Memo lookup for a partial child, keyed on its rolling hash. Sound: a hit is confirmed by
   comparing the full sorted part array, so a collision costs a miss and never a wrong answer. */
static int chi_refuted(const Level *L, const Chi *c, int k) {
    Fact t;
    C_REF;
    if (c->mass > pow3[k]) return 1;                    /* COUNT, no lookup needed */
    if (c->np == 0) return 0;
    if (c->np <= MEMOP) {
        uint64_t h = c->h + 0x51ULL * (uint64_t)k;
        MemoEnt *e = &memo[h & memo_mask];
        if (e->res >= 0 && e->h == h && e->k == (unsigned char)k && e->np == c->np
                && memcmp(e->p, c->p, c->np * sizeof(Part)) == 0) {
            MEMO_HIT;
            if (g_paint && e->res && e->wit >= 0) {
                __atomic_store_n(&L->cited[e->wit], 1, __ATOMIC_RELAXED); paint_hits++;
            }
            return e->res;
        }
        MEMO_MISS;
        if (c->np > MAXP) return 0;
        t.np = c->np; t.mass = c->mass;
        memcpy(t.p, c->p, c->np * sizeof(Part));
        int r = refuted_raw(L, &t, k);
        e->h = h; e->k = (unsigned char)k; e->np = c->np; e->res = (signed char)r;
        e->wit = (int32_t)(r ? g_wit : -1);
        if (g_paint && r && g_wit >= 0) {
            __atomic_store_n(&L->cited[g_wit], 1, __ATOMIC_RELAXED); paint_hits++;
        }
        memcpy(e->p, c->p, c->np * sizeof(Part));
        return r;
    }
    if (c->np > MAXP) return 0;
    t.np = c->np; t.mass = c->mass;
    memcpy(t.p, c->p, c->np * sizeof(Part));
    return refuted_raw(L, &t, k);
}

/* ---------------------------------------------------------------- live splits, per (part,k) */

typedef struct { unsigned char a, b; int k2, k0, k1; } Split;
typedef struct { int n; Split *s; } LiveTab;
static TLS LiveTab *live_tab[MAXK + 1][2];
static TLS long long live_built = 0, live_reused = 0;

static const Split *live_get(const Level *below, int k, Part p, int restrict_, int *cnt) {
    int idx = (p.n << 8) | p.m;
    if (!live_tab[k][restrict_]) {
        int z;
        live_tab[k][restrict_] = malloc(NPART * sizeof(LiveTab));
        if (!live_tab[k][restrict_]) { fprintf(stderr, "no memory for live tables\n"); exit(1); }
        for (z = 0; z < NPART; z++) {
            live_tab[k][restrict_][z].n = -1;
            live_tab[k][restrict_][z].s = NULL;
        }
    }
    LiveTab *T = live_tab[k][restrict_];
    if (T[idx].n >= 0) { live_reused++; *cnt = T[idx].n; return T[idx].s; }
    live_built++;
    int n = p.n, m = p.m, a, b, cap = (n + 1) * (m + 1), c = 0;
    Split *out = malloc(cap * sizeof(Split));
    Fact t;
    Part tmp[2];
    for (a = 0; a <= n; a++)
        for (b = 0; b <= m; b++) {
            /* complement symmetry: one representative per global-flip orbit, for one group */
            if (restrict_ && (a > n - a || (a == n - a && b > m - b))) continue;
            tmp[0].n = a; tmp[0].m = b;
            if (canon(tmp, 1, &t) && refuted(below, &t, k - 1)) continue;
            tmp[0].n = n - a; tmp[0].m = m - b;
            if (canon(tmp, 1, &t) && refuted(below, &t, k - 1)) continue;
            tmp[0].n = a; tmp[0].m = m - b; tmp[1].n = n - a; tmp[1].m = b;
            if (canon(tmp, 2, &t) && refuted(below, &t, k - 1)) continue;
            out[c].a = a; out[c].b = b;
            out[c].k2 = a * b; out[c].k0 = (n - a) * (m - b);
            out[c].k1 = n * m - out[c].k2 - out[c].k0;
            c++;
        }
    T[idx].n = c; T[idx].s = out;
    *cnt = c;
    return out;
}


/* ------------------------------------------- pairwise narrowing (2-group subgroup narrowing)
   live_get above is subgroup narrowing for a subgroup of size ONE: an option is dropped when
   that group's own contribution to some child is already refuted at k-1. This is the size-TWO
   case. For options oi of part P and oj of part Q, if the two-part partial child formed by P's
   and Q's contributions alone is refuted at k-1, then EVERY full split containing that pair is
   discharged: the full child contains the pair as a sub-multiset, and Subgraph Monotonicity
   carries unsolvability upward.

   Stored as a bitmask row per oi -- bit oj set means the pair is still live. During the search,
   the live domain of a not-yet-fixed group is the AND of the rows picked out by the fixed
   groups. An empty domain means every completion is discharged, so the subtree is verified
   without being enumerated: the conflict is acted on at the depth it becomes *knowable*, not
   the depth it would be reached. That is CSP forward checking.

   Keyed on (k, part_i, restrict_i, part_j, restrict_j) only -- never on the containing state --
   so the tables are shared across every fact, like the live lists. Built lazily and only for
   option lists up to FCMAXO, which is where the high-part-count facts live anyway. */

#define FCMAXO 256                     /* skip pair tables above this many options */
#define FCW    (FCMAXO / 64)
#define PAIRSZ (1 << 17)

typedef struct { uint64_t key; uint64_t *rows; int li, wj; } PairEnt;
static TLS PairEnt *pairtab;
static TLS long long pair_built = 0, pair_reused = 0, fc_prunes = 0, fc_dom_skips = 0;
static TLS long long pair_bits_set = 0, pair_bits_tot = 0;

static const uint64_t *pair_get(const Level *below, int k, Part pi, int ri, Part pj, int rj,
                                int *wjp) {
    if (!pairtab) {
        pairtab = calloc(PAIRSZ, sizeof(PairEnt));
        if (!pairtab) { fprintf(stderr, "no memory for pair tables\n"); exit(1); }
    }
    int ii = (pi.n << 8) | pi.m, ij = (pj.n << 8) | pj.m;
    uint64_t key = ((uint64_t)k << 34) | ((uint64_t)ri << 33) | ((uint64_t)rj << 32)
                 | ((uint64_t)ii << 16) | (uint64_t)ij;
    uint64_t h = key * 0x9E3779B97F4A7C15ULL;
    int slot = (int)(h >> 47) & (PAIRSZ - 1);
    while (pairtab[slot].rows && pairtab[slot].key != key) slot = (slot + 1) & (PAIRSZ - 1);
    if (pairtab[slot].rows) { pair_reused++; *wjp = pairtab[slot].wj; return pairtab[slot].rows; }
    /* open addressing with no eviction: refuse to fill past half, or probing degenerates */
    if (pair_built >= PAIRSZ / 2) { *wjp = 0; return NULL; }

    int li, lj;
    const Split *si = live_get(below, k, pi, ri, &li);
    const Split *sj = live_get(below, k, pj, rj, &lj);
    if (li > FCMAXO || lj > FCMAXO) { *wjp = 0; return NULL; }
    pair_built++;
    int wj = (lj + 63) / 64;
    uint64_t *rows = calloc((size_t)li * wj ? (size_t)li * wj : 1, sizeof(uint64_t));
    int oi, oj;
    Part t[4];
    Fact f;
    for (oi = 0; oi < li; oi++) {
        int a = si[oi].a, b = si[oi].b, na = pi.n - a, nb = pi.m - b;
        for (oj = 0; oj < lj; oj++) {
            int c = sj[oj].a, d = sj[oj].b, nc = pj.n - c, nd = pj.m - d;
            t[0].n = a;  t[0].m = b;  t[1].n = c;  t[1].m = d;
            if (canon(t, 2, &f) && refuted(below, &f, k - 1)) continue;
            t[0].n = na; t[0].m = nb; t[1].n = nc; t[1].m = nd;
            if (canon(t, 2, &f) && refuted(below, &f, k - 1)) continue;
            t[0].n = a;  t[0].m = nb; t[1].n = na; t[1].m = b;
            t[2].n = c;  t[2].m = nd; t[3].n = nc; t[3].m = d;
            if (canon(t, 4, &f) && refuted(below, &f, k - 1)) continue;
            rows[(size_t)oi * wj + (oj >> 6)] |= 1ULL << (oj & 63);
        }
    }
    { size_t w2; int bits = 0;
      for (w2 = 0; w2 < (size_t)li * wj; w2++) bits += __builtin_popcountll(rows[w2]);
      pair_bits_set += bits; pair_bits_tot += (long long)li * lj; }
    pairtab[slot].key = key; pairtab[slot].rows = rows;
    pairtab[slot].li = li; pairtab[slot].wj = wj;
    *wjp = wj;
    return rows;
}


/* ---------------------------------------------------- subtree memo (the prefix-state DP)
   splits_rec's result depends on exactly: the depth i, the option floor lo forced by the
   identical-part symmetry, the three accumulated child masses, and the three partial children.
   Nothing else -- the remaining parts are g_s->p[i..], fixed per fact. So it is a pure function
   and can be memoised, collapsing distinct option-prefixes that reach the same state.

   Why this is the right reduction for the regime that decides Sa(193), argued from the shape of
   the problem rather than from a small-k measurement:

     * a state at k has mass <= 3^k, and the three children's masses sum to it EXACTLY (mass
       conservation), each capped at 3^(k-1). Measured on out_k8.txt at k=4: as part count rises
       the mass saturates -- 89% of the cap at P=4, 97% at P=7, and 100.0% of facts are saturated
       at P>=9. High part count IS the saturated regime.
     * the parts consumed by depth i are fixed, so the accumulated masses satisfy
       s1 = M_i - s2 - s0. The numeric state is (s2, s0) alone: at most (3^(k-1)+1)^2 values per
       depth -- 784 at k=4, 6,724 at k=5.
     * saturation forces many small parts (mean largest part mass falls from 29 at P=4 to 12 at
       P=13). Small parts have children that are nil or (1:1), which Unit-Group Elimination drops,
       so the structural half of the state largely vanishes and the numeric half dominates.

   So the state count grows POLYNOMIALLY where the tree grows exponentially, and the gain should
   rise with part count -- the opposite of pairwise narrowing, whose gain decays with it.

   Exact confirmation, not hashing alone: the 64-bit rolling hashes only pick the slot. A
   probabilistic match is not acceptable in a checker whose only product is trust. */

static int g_dpen = 0;
static long long g_nodecap = 0;
static double g_timecap = 0;
static TLS clock_t g_fact_t0;
static TLS int g_budget_hit = 0;      /* measured a net 2.4x loss - see the journal; off by default */
static int g_diag = 0, g_srcmask = 0;
static TLS int g_diag_left;

#define DPBITS 20
#define DPSZ   (1 << DPBITS)
#define DPC    32                       /* children above this are not memoised */

typedef struct {
    uint32_t gen;
    uint16_t s2, s0;
    unsigned char i, lo, res, n2, n0, n1;
    Part c2[DPC], c0[DPC], c1[2 * DPC];
} DpEnt;

static TLS DpEnt *dp;
static TLS uint32_t dp_gen = 0;
static TLS long long dp_hit = 0, dp_miss = 0, dp_skip = 0;

static inline int dp_probe(int i, int lo, int s2, int s0,
                           const Chi *c2, const Chi *c0, const Chi *c1, DpEnt **slot) {
    if (c2->np > DPC || c0->np > DPC || c1->np > 2 * DPC) { dp_skip++; *slot = NULL; return -1; }
    uint64_t h = c2->h * 0x9E3779B97F4A7C15ULL + c0->h * 0xC2B2AE3D27D4EB4FULL
               + c1->h * 0x165667B19E3779F9ULL
               + ((uint64_t)i << 40) + ((uint64_t)lo << 48) + ((uint64_t)s2 << 20) + s0;
    h ^= h >> 29; h *= 0xBF58476D1CE4E5B9ULL; h ^= h >> 32;
    DpEnt *e = &dp[h & (DPSZ - 1)];
    *slot = e;
    if (e->gen == dp_gen && e->i == i && e->lo == lo && e->s2 == s2 && e->s0 == s0
            && e->n2 == c2->np && e->n0 == c0->np && e->n1 == c1->np
            && memcmp(e->c2, c2->p, c2->np * sizeof(Part)) == 0
            && memcmp(e->c0, c0->p, c0->np * sizeof(Part)) == 0
            && memcmp(e->c1, c1->p, c1->np * sizeof(Part)) == 0) { dp_hit++; return e->res; }
    dp_miss++;
    return -1;
}

static inline void dp_store(DpEnt *e, int i, int lo, int s2, int s0,
                            const Chi *c2, const Chi *c0, const Chi *c1, int res) {
    if (!e) return;
    e->gen = dp_gen; e->i = (unsigned char)i; e->lo = (unsigned char)lo;
    e->s2 = (uint16_t)s2; e->s0 = (uint16_t)s0; e->res = (unsigned char)res;
    e->n2 = c2->np; e->n0 = c0->np; e->n1 = c1->np;
    memcpy(e->c2, c2->p, c2->np * sizeof(Part));
    memcpy(e->c0, c0->p, c0->np * sizeof(Part));
    memcpy(e->c1, c1->p, c1->np * sizeof(Part));
}

/* ---------------------------------------------------------------- SPLITS check */

static TLS const Level *g_below;
static TLS int g_k, g_cap;
static TLS const Fact *g_s;
static TLS const Split *g_live[MAXP];
static TLS int g_ln[MAXP], g_last[MAXP + 1];
static TLS long long g_nodes, g_fact_nodes, g_max_fact_nodes;
/* one slot per depth: the child states for the prefix of that length */
static TLS Chi st2[MAXP + 1], st0[MAXP + 1], st1[MAXP + 1];
static TLS int g_fc;                                   /* forward checking enabled for this fact */
static TLS const uint64_t *g_row[MAXP][MAXP];          /* g_row[i][j], i<j: rows of pair (i,j) */
static TLS int g_wj[MAXP];                             /* words in group j's domain */
static TLS uint64_t g_dom[MAXP + 1][MAXP][FCW];        /* live domain of each group, per depth */

static int splits_rec(int i, int s2, int s0, int s1) {
    g_nodes++; g_fact_nodes++;
    if (g_progress_node_slot && !(g_fact_nodes & ((1LL << 20) - 1)))
        atomic_store_explicit(g_progress_node_slot, (unsigned long long)g_fact_nodes,
                              memory_order_relaxed);
    /* Per-fact node budget. Aborting a fact makes its verdict UNKNOWN, never "verified", so this
       is only ever used to measure the cost distribution - which is the quantity that decides
       whether a level is feasible, and which a mean cannot express when per-fact costs span four
       orders of magnitude. */
    if (g_nodecap && g_fact_nodes > g_nodecap) { g_budget_hit = 1; return 1; }
    /* Time budget. At k=7 a node costs ~134 us against ~0.07 us at k=4, so a node cap cannot
       bound the work - 30 M nodes was never approached in 40 minutes. Cost is per-query. */
    if (g_timecap && !(g_fact_nodes & 0x3FF)
            && (double)(clock() - g_fact_t0) / CLOCKS_PER_SEC > g_timecap) {
        g_budget_hit = 1; return 1;
    }
    DpEnt *dpe = NULL;
    int lo0 = (i && g_s->p[i].n == g_s->p[i - 1].n && g_s->p[i].m == g_s->p[i - 1].m)
              ? g_last[i - 1] + 1 : 0;
    if (g_dpen && i) {
        int r = dp_probe(i, lo0, s2, s0, &st2[i], &st0[i], &st1[i], &dpe);
        if (r >= 0) return r;
    }
    if (i) {
        /* prefix subset narrowing: a refuted PARTIAL child discharges every completion, since
           completions only add parts and Subgraph Monotonicity preserves unsolvability upward.
           Cheapest children first - c2 and c0 have i parts, the mixed child has 2i. */
        if (chi_refuted(g_below, &st2[i], g_k - 1)
                || chi_refuted(g_below, &st0[i], g_k - 1)
                || chi_refuted(g_below, &st1[i], g_k - 1)) {
            dp_store(dpe, i, lo0, s2, s0, &st2[i], &st0[i], &st1[i], 1);
            return 1;
        }
    }
    if (i == g_s->np) {
        /* A split with no refuted child. Printing it turns "unverified" into a concrete list of
           states the certificate is missing, which is the actionable form of a closure gap. */
        if (g_diag && g_diag_left > 0) {
            int j;
            g_diag_left--;
            printf("      GAP split");
            for (j = 0; j < g_s->np; j++)
                printf(" %d:%d->(%d,%d)", g_s->p[j].n, g_s->p[j].m,
                       g_live[j][g_last[j]].a, g_live[j][g_last[j]].b);
            printf("\n        c2:"); for (j = 0; j < st2[i].np; j++)
                printf(" %d:%d", st2[i].p[j].n, st2[i].p[j].m);
            printf(" [%d]\n        c0:", st2[i].mass); for (j = 0; j < st0[i].np; j++)
                printf(" %d:%d", st0[i].p[j].n, st0[i].p[j].m);
            printf(" [%d]\n        c1:", st0[i].mass); for (j = 0; j < st1[i].np; j++)
                printf(" %d:%d", st1[i].p[j].n, st1[i].p[j].m);
            printf(" [%d]\n", st1[i].mass);
        }
        return 0;
    }
    int n = g_s->p[i].n, m = g_s->p[i].m, oi, j, w;
    /* identical-part symmetry: equal parts share an option list, and swapping their splits
       permutes children which are multisets, so require a non-decreasing option index */
    int lo = lo0;
    for (oi = lo; oi < g_ln[i]; oi++) {
        const Split *sp = &g_live[i][oi];
        /* cap first: three comparisons, and it kills most of what the domain test would */
        if (s2 + sp->k2 > g_cap || s0 + sp->k0 > g_cap || s1 + sp->k1 > g_cap) continue;
        if (g_fc && !(g_dom[i][i][oi >> 6] >> (oi & 63) & 1)) { fc_dom_skips++; continue; }
        if (g_fc) {
            /* forward check: intersect every later group's domain with this option's row */
            int dead = 0;
            for (j = i + 1; j < g_s->np && !dead; j++) {
                const uint64_t *row = g_row[i][j];
                uint64_t any = 0;
                if (!row) {                     /* no table for this pair: carry the domain */
                    for (w = 0; w < g_wj[j]; w++)
                        any |= (g_dom[i + 1][j][w] = g_dom[i][j][w]);
                } else {
                    const uint64_t *r = row + (size_t)oi * g_wj[j];
                    for (w = 0; w < g_wj[j]; w++)
                        any |= (g_dom[i + 1][j][w] = g_dom[i][j][w] & r[w]);
                }
                if (!any) dead = 1;             /* group j has no live option left */
            }
            if (dead) { fc_prunes++; continue; }   /* every completion discharged */
        }
        chi_add(&st2[i + 1], &st2[i], sp->a, sp->b);
        chi_add(&st0[i + 1], &st0[i], n - sp->a, m - sp->b);
        { Chi tmp;
          chi_add(&tmp,        &st1[i], sp->a, m - sp->b);
          chi_add(&st1[i + 1], &tmp,    n - sp->a, sp->b); }
        g_last[i] = oi;
        if (!splits_rec(i + 1, s2 + sp->k2, s0 + sp->k0, s1 + sp->k1)) return 0;
    }
    if (i) dp_store(dpe, i, lo0, s2, s0, &st2[i], &st0[i], &st1[i], 1);
    return 1;
}

static int g_fcen = 1, g_fcmin = 3;
static int g_minp = 0, g_maxp = 999, g_stride = 1, g_mink = 1;
static long long sel = 0;                 /* serial compatibility path; parallel selects centrally */
static int g_bench = 0;
static int g_order = 0;                  /* 0 desc (canonical) 1 asc 2 fewest-options-first */
static TLS long long np_nodes[MAXP + 1], np_facts[MAXP + 1];
static TLS long long cost_hist[24], cost_sum, budget_out;
static TLS Fact g_perm;

static int verify(const Level *below, const Fact *s, int k) {
    if (s->mass > pow3[k]) return 1;
    if (maj_refutes(s, k)) return 1;
    int i, j;
    g_below = below; g_k = k; g_cap = pow3[k - 1];
    if (g_order == 0) {
        g_s = s;
        for (i = 0; i < s->np; i++) g_live[i] = live_get(below, k, s->p[i], i == 0, &g_ln[i]);
    } else {
        /* Reorder the parts. Sound: parts are a multiset, so enumeration order is free; the
           identical-part symmetry only needs equal parts adjacent, and every key below keeps
           them so. Complement symmetry is a single global flip, so it attaches to whichever
           part is enumerated first. */
        int ln[MAXP], ord[MAXP];
        const Split *lv[MAXP];
        for (i = 0; i < s->np; i++) { lv[i] = live_get(below, k, s->p[i], 0, &ln[i]); ord[i] = i; }
        for (i = 1; i < s->np; i++) {           /* insertion sort on the chosen key */
            int v = ord[i];
            for (j = i - 1; j >= 0; j--) {
                int a = ord[j], better;
                if (g_order == 1) better = (s->p[a].n * s->p[a].m > s->p[v].n * s->p[v].m);
                else better = (ln[a] > ln[v]) || (ln[a] == ln[v] &&
                               s->p[a].n * s->p[a].m > s->p[v].n * s->p[v].m);
                if (!better) break;
                ord[j + 1] = a;
            }
            ord[j + 1] = v;
        }
        g_perm.np = s->np; g_perm.mass = s->mass;
        for (i = 0; i < s->np; i++) g_perm.p[i] = s->p[ord[i]];
        g_s = &g_perm;
        g_live[0] = live_get(below, k, g_perm.p[0], 1, &g_ln[0]);
        for (i = 1; i < s->np; i++) { g_live[i] = lv[ord[i]]; g_ln[i] = ln[ord[i]]; }
    }
    for (i = 0; i <= s->np; i++) g_last[i] = -1;
    g_fc = g_fcen && g_s->np >= g_fcmin;
    for (i = 0; i < g_s->np && g_fc; i++) if (g_ln[i] > FCMAXO) g_fc = 0;
    if (g_fc) {
        for (i = 0; i < g_s->np; i++) {
            g_wj[i] = (g_ln[i] + 63) / 64;
            int w; for (w = 0; w < g_wj[i]; w++) g_dom[0][i][w] = ~0ULL;
            if (g_ln[i] & 63) g_dom[0][i][g_wj[i] - 1] = (1ULL << (g_ln[i] & 63)) - 1;
        }
        for (i = 0; i < g_s->np; i++)
            for (j = i + 1; j < g_s->np; j++) {
                int wj;
                g_row[i][j] = pair_get(below, k, g_s->p[i], i == 0, g_s->p[j], 0, &wj);
                if (g_row[i][j] && wj != g_wj[j]) g_row[i][j] = NULL;   /* paranoia */
            }
    }
    st2[0].np = st0[0].np = st1[0].np = 0;
    st2[0].mass = st0[0].mass = st1[0].mass = 0;
    st2[0].h = st0[0].h = st1[0].h = 0;
    g_fact_nodes = 0;
    g_budget_hit = 0;
    g_fact_t0 = clock();
    g_diag_left = g_diag;
    dp_gen++;
    int r = splits_rec(0, 0, 0, 0);
    if (g_fact_nodes > g_max_fact_nodes) g_max_fact_nodes = g_fact_nodes;
    np_nodes[s->np] += g_fact_nodes; np_facts[s->np]++;
    return r;
}

/* Reentrancy for derive(): verify() drives splits_rec through file-scope state, so a nested
   call must save and restore it. Depth is bounded by k, so a stack frame per level is fine. */
typedef struct {
    const Level *below; int k, cap; const Fact *s; Fact perm;
    const Split *live[MAXP]; int ln[MAXP], last[MAXP + 1];
    int fc; const uint64_t *row[MAXP][MAXP]; int wj[MAXP];
    Chi s2[MAXP + 1], s0[MAXP + 1], s1[MAXP + 1];
    long long fact_nodes;
    clock_t fact_t0;              /* a nested derive must not reset the enclosing fact's budget */
} SaveCtx;

static TLS int g_dderive = 0;                 /* recursion depth, for a sanity bound */

static int derive(const Fact *s, int k) {
    if (g_dderive >= MAXK || !g_levels) return 0;
    const Level *below = &((const Level *)g_levels)[k - 1];   /* children live one level down */
    if (g_dderive >= MAXK) return 0;
    SaveCtx *c = malloc(sizeof(SaveCtx));
    if (!c) return 0;
    c->below = g_below; c->k = g_k; c->cap = g_cap; c->s = g_s; c->perm = g_perm;
    c->fc = g_fc; c->fact_nodes = g_fact_nodes; c->fact_t0 = g_fact_t0;
    memcpy(c->live, g_live, sizeof g_live); memcpy(c->ln, g_ln, sizeof g_ln);
    memcpy(c->last, g_last, sizeof g_last);
    memcpy(c->row, g_row, sizeof g_row);  memcpy(c->wj, g_wj, sizeof g_wj);
    memcpy(c->s2, st2, sizeof st2); memcpy(c->s0, st0, sizeof st0);
    memcpy(c->s1, st1, sizeof st1);

    g_dderive++;
    Fact local = *s;
    int r = verify(below, &local, k);
    g_dderive--;

    g_below = c->below; g_k = c->k; g_cap = c->cap; g_s = c->s; g_perm = c->perm;
    g_fc = c->fc; g_fact_nodes = c->fact_nodes; g_fact_t0 = c->fact_t0;
    memcpy(g_live, c->live, sizeof g_live); memcpy(g_ln, c->ln, sizeof g_ln);
    memcpy(g_last, c->last, sizeof g_last);
    memcpy(g_row, c->row, sizeof g_row);  memcpy(g_wj, c->wj, sizeof g_wj);
    memcpy(st2, c->s2, sizeof st2); memcpy(st0, c->s0, sizeof st0);
    memcpy(st1, c->s1, sizeof st1);
    free(c);
    return r;
}

/* -------------------------------------------------------- parallel fact batches

   A fact at k reads only the frozen level k-1.  All mutable search machinery above is therefore
   worker-local, while Level is shared.  Ordinary verification may mix every k in one work queue;
   top-down painting invokes one batch per level because the citations produced at k define the
   targets at k-1. */

typedef struct {
    long long nodes, memo_hit, memo_miss;
    long long live_built, live_reused;
    long long pair_built, pair_reused, fc_prunes, fc_dom_skips;
    long long pair_bits_set, pair_bits_tot;
    long long paint_hits, pref_hits, pref_miss;
    long long derived_ok, derived_no;
    long long dp_hit, dp_miss, dp_skip;
    long long max_fact_nodes;
#ifdef VERIFY_INDEX_STATS
    long long idx_candidates, idx_product_rejects, idx_nm_rejects;
    long long idx_match_calls, idx_match_hits;
#ifdef VERIFY_BLOCK_PARETO
    long long idx_block_tests, idx_block_rejects, idx_block_min_rejects;
    long long idx_block_skipped, idx_block_front_tests;
#endif
#endif
} BatchStats;

typedef struct {
    int k;
    const Fact *fact;
    unsigned char status;             /* 0 unverified, 1 verified, 2 budget exhausted */
    long long nodes;
} VerifyTask;

typedef struct {
    atomic_size_t task;               /* SIZE_MAX while idle */
    atomic_ullong started_ns;
    atomic_ullong nodes;              /* coarse cursor for the active fact */
} VerifyWorkerProgress;

typedef struct {
    VerifyTask *tasks;
    size_t ntasks;
    atomic_size_t next;
    atomic_size_t done, verified, unverified, budget;
    atomic_ullong completed_nodes;
    atomic_size_t done_by_k[MAXK + 1], done_by_np[MAXP + 1];
    size_t total_by_k[MAXK + 1], total_by_np[MAXP + 1];
    int memo_bits;
    int threads, progress_k;
    const char *phase;
    double started;
    VerifyWorkerProgress *progress;
    pthread_mutex_t progress_mu;
    pthread_cond_t progress_cv;
    int progress_stop;
} Batch;

typedef struct {
    Batch *batch;
    int slot;
    BatchStats stats;
} WorkerArg;

static void worker_state_init(int memo_bits) {
    size_t slots;
    int k, r;
    if (memo || pairtab || dp) { fprintf(stderr, "worker state initialized twice\n"); exit(2); }
    if (memo_bits < 12 || memo_bits > MEMOBITS) {
        fprintf(stderr, "VERIFY_MEMO_BITS must be in 12..%d\n", MEMOBITS); exit(2);
    }
    slots = (size_t)1 << memo_bits;
    memo = malloc(slots * sizeof(MemoEnt));
    if (!memo) { fprintf(stderr, "no memory for verifier memo (%zu slots)\n", slots); exit(1); }
    memset(memo, 0xff, slots * sizeof(MemoEnt));
    memo_mask = slots - 1;
    if (g_dpen) {
        dp = calloc(DPSZ, sizeof(DpEnt));
        if (!dp) { fprintf(stderr, "no memory for subtree memo\n"); exit(1); }
    }
    for (k = 0; k <= MAXK; k++) for (r = 0; r < 2; r++) live_tab[k][r] = NULL;
    g_wit = -1; g_nodes = g_fact_nodes = g_max_fact_nodes = 0;
    memo_hit = memo_miss = paint_hits = pref_hits = pref_miss = 0;
    derived_ok = derived_no = live_built = live_reused = 0;
    pair_built = pair_reused = fc_prunes = fc_dom_skips = 0;
    pair_bits_set = pair_bits_tot = 0;
    dp_gen = 0; dp_hit = dp_miss = dp_skip = 0;
#ifdef VERIFY_INDEX_STATS
    idx_candidates = idx_product_rejects = idx_nm_rejects = 0;
    idx_match_calls = idx_match_hits = 0;
#ifdef VERIFY_BLOCK_PARETO
    idx_block_tests = idx_block_rejects = idx_block_min_rejects = 0;
    idx_block_skipped = idx_block_front_tests = 0;
#endif
#endif
    cost_sum = budget_out = 0;
    memset(np_nodes, 0, sizeof np_nodes); memset(np_facts, 0, sizeof np_facts);
    memset(cost_hist, 0, sizeof cost_hist);
}

static void worker_state_finish(BatchStats *s) {
    int k, r, i;
    memset(s, 0, sizeof *s);
    s->nodes = g_nodes; s->memo_hit = memo_hit; s->memo_miss = memo_miss;
    s->live_built = live_built; s->live_reused = live_reused;
    s->pair_built = pair_built; s->pair_reused = pair_reused;
    s->fc_prunes = fc_prunes; s->fc_dom_skips = fc_dom_skips;
    s->pair_bits_set = pair_bits_set; s->pair_bits_tot = pair_bits_tot;
    s->paint_hits = paint_hits; s->pref_hits = pref_hits; s->pref_miss = pref_miss;
    s->derived_ok = derived_ok; s->derived_no = derived_no;
    s->dp_hit = dp_hit; s->dp_miss = dp_miss; s->dp_skip = dp_skip;
    s->max_fact_nodes = g_max_fact_nodes;
#ifdef VERIFY_INDEX_STATS
    s->idx_candidates = idx_candidates; s->idx_product_rejects = idx_product_rejects;
    s->idx_nm_rejects = idx_nm_rejects; s->idx_match_calls = idx_match_calls;
    s->idx_match_hits = idx_match_hits;
#ifdef VERIFY_BLOCK_PARETO
    s->idx_block_tests = idx_block_tests; s->idx_block_rejects = idx_block_rejects;
    s->idx_block_min_rejects = idx_block_min_rejects;
    s->idx_block_skipped = idx_block_skipped;
    s->idx_block_front_tests = idx_block_front_tests;
#endif
#endif
    for (k = 0; k <= MAXK; k++) for (r = 0; r < 2; r++) if (live_tab[k][r]) {
        for (i = 0; i < NPART; i++) if (live_tab[k][r][i].n >= 0)
            free(live_tab[k][r][i].s);
        free(live_tab[k][r]); live_tab[k][r] = NULL;
    }
    if (pairtab) {
        for (i = 0; i < PAIRSZ; i++) free(pairtab[i].rows);
        free(pairtab); pairtab = NULL;
    }
    free(memo); memo = NULL; memo_mask = 0;
    free(dp); dp = NULL;
}

static void add_batch_stats(BatchStats *a, const BatchStats *b) {
#define ADD_STAT(name) a->name += b->name
    ADD_STAT(nodes); ADD_STAT(memo_hit); ADD_STAT(memo_miss);
    ADD_STAT(live_built); ADD_STAT(live_reused);
    ADD_STAT(pair_built); ADD_STAT(pair_reused); ADD_STAT(fc_prunes); ADD_STAT(fc_dom_skips);
    ADD_STAT(pair_bits_set); ADD_STAT(pair_bits_tot);
    ADD_STAT(paint_hits); ADD_STAT(pref_hits); ADD_STAT(pref_miss);
    ADD_STAT(derived_ok); ADD_STAT(derived_no);
    ADD_STAT(dp_hit); ADD_STAT(dp_miss); ADD_STAT(dp_skip);
#ifdef VERIFY_INDEX_STATS
    ADD_STAT(idx_candidates); ADD_STAT(idx_product_rejects); ADD_STAT(idx_nm_rejects);
    ADD_STAT(idx_match_calls); ADD_STAT(idx_match_hits);
#ifdef VERIFY_BLOCK_PARETO
    ADD_STAT(idx_block_tests); ADD_STAT(idx_block_rejects); ADD_STAT(idx_block_min_rejects);
    ADD_STAT(idx_block_skipped);
    ADD_STAT(idx_block_front_tests);
#endif
#endif
    if (b->max_fact_nodes > a->max_fact_nodes) a->max_fact_nodes = b->max_fact_nodes;
#undef ADD_STAT
}

static void *batch_worker(void *vp) {
    WorkerArg *a = vp;
    Batch *b = a->batch;
    VerifyWorkerProgress *p = &b->progress[a->slot];
    size_t q;
    worker_state_init(b->memo_bits);
    while ((q = atomic_fetch_add_explicit(&b->next, 1, memory_order_relaxed)) < b->ntasks) {
        VerifyTask *t = &b->tasks[q];
        /* verify() returns before its ordinary per-fact setup for direct COUNT/MAJ proofs. */
        g_fact_nodes = 0; g_budget_hit = 0;
        atomic_store_explicit(&p->nodes, 0, memory_order_relaxed);
        atomic_store_explicit(&p->started_ns, monotonic_nanoseconds(), memory_order_relaxed);
        atomic_store_explicit(&p->task, q, memory_order_release);
        g_progress_node_slot = g_progress_seconds > 0 ? &p->nodes : NULL;
        int ok = verify(&((const Level *)g_levels)[t->k - 1], t->fact, t->k);
        g_progress_node_slot = NULL;
        t->nodes = g_fact_nodes;
        t->status = g_budget_hit ? 2 : (unsigned char)(ok ? 1 : 0);
        if (g_budget_hit) budget_out++;
        atomic_store_explicit(&p->nodes, (unsigned long long)g_fact_nodes,
                              memory_order_relaxed);
        atomic_fetch_add_explicit(&b->completed_nodes, (unsigned long long)g_fact_nodes,
                                  memory_order_relaxed);
        atomic_fetch_add_explicit(&b->done_by_k[t->k], 1, memory_order_relaxed);
        atomic_fetch_add_explicit(&b->done_by_np[t->fact->np], 1, memory_order_relaxed);
        if (t->status == 1) atomic_fetch_add_explicit(&b->verified, 1, memory_order_relaxed);
        else if (t->status == 2) atomic_fetch_add_explicit(&b->budget, 1, memory_order_relaxed);
        else atomic_fetch_add_explicit(&b->unverified, 1, memory_order_relaxed);
        atomic_store_explicit(&p->task, SIZE_MAX, memory_order_release);
        atomic_fetch_add_explicit(&b->done, 1, memory_order_release);
    }
    worker_state_finish(&a->stats);
    return NULL;
}

static int default_memo_bits(int threads) {
    int bits = MEMOBITS;
    while (threads > 1 && bits > 18) { threads = (threads + 1) / 2; bits--; }
    return bits;
}

static void print_batch_progress(Batch *b, size_t *last_done, double *last_time,
                                 double *ewma_rate) {
    double now = monotonic_seconds(), elapsed = now - b->started;
    size_t done = atomic_load_explicit(&b->done, memory_order_acquire);
    size_t claimed = atomic_load_explicit(&b->next, memory_order_relaxed);
    size_t verified = atomic_load_explicit(&b->verified, memory_order_relaxed);
    size_t unverified = atomic_load_explicit(&b->unverified, memory_order_relaxed);
    size_t budget = atomic_load_explicit(&b->budget, memory_order_relaxed);
    unsigned long long nodes = atomic_load_explicit(&b->completed_nodes, memory_order_relaxed);
    size_t oldest_task[3] = {SIZE_MAX, SIZE_MAX, SIZE_MAX};
    int oldest_worker[3] = {-1, -1, -1};
    uint64_t oldest_age[3] = {0, 0, 0};
    uint64_t now_ns = monotonic_nanoseconds();
    int active = 0, i, z;
    char kval[16];
    if (claimed > b->ntasks) claimed = b->ntasks;
    if (b->progress_k >= 0) snprintf(kval, sizeof kval, "%d", b->progress_k);
    else strcpy(kval, "all");
    for (i = 0; i < b->threads; i++) {
        size_t q = atomic_load_explicit(&b->progress[i].task, memory_order_acquire);
        if (q >= b->ntasks) continue;
        uint64_t started = atomic_load_explicit(&b->progress[i].started_ns,
                                                memory_order_relaxed);
        uint64_t age = now_ns > started ? now_ns - started : 0;
        if (atomic_load_explicit(&b->progress[i].task, memory_order_acquire) != q) continue;
        active++;
        for (z = 0; z < 3; z++) if (age > oldest_age[z]) {
            int y;
            for (y = 2; y > z; y--) {
                oldest_age[y] = oldest_age[y - 1];
                oldest_task[y] = oldest_task[y - 1];
                oldest_worker[y] = oldest_worker[y - 1];
            }
            oldest_age[z] = age; oldest_task[z] = q; oldest_worker[z] = i;
            break;
        }
    }
    double total_rate = done / (elapsed > 0 ? elapsed : 1);
    double window_elapsed = now - *last_time;
    double window_rate = (done - *last_done) / (window_elapsed > 0 ? window_elapsed : 1);
    if (window_rate > 0) *ewma_rate = *ewma_rate > 0
        ? 0.75 * *ewma_rate + 0.25 * window_rate : window_rate;
    printf("PROGRESS phase=%s k=%s elapsed_s=%.1f completed=%zu/%zu percent=%.4f "
           "claimed=%zu active=%d queued=%zu verified=%zu unverified=%zu budget=%zu "
           "rate_total=%.3f/s rate_window=%.3f/s rate_ewma=%.3f/s "
           "nodes_done=%llu nodes_rate=%.3f/s ",
           b->phase, kval, elapsed, done, b->ntasks,
           100.0 * done / (b->ntasks ? b->ntasks : 1), claimed, active,
           b->ntasks - claimed, verified, unverified, budget, total_rate, window_rate,
           *ewma_rate, nodes, nodes / (elapsed > 0 ? elapsed : 1));
    if (total_rate > 0) printf("eta_total_s=%.0f ", (b->ntasks - done) / total_rate);
    else printf("eta_total_s=unknown ");
    if (*ewma_rate > 0) printf("eta_ewma_s=%.0f\n", (b->ntasks - done) / *ewma_rate);
    else printf("eta_ewma_s=unknown\n");

    printf("PROGRESS_LEVELS phase=%s", b->phase);
    for (i = 0; i <= MAXK; i++) if (b->total_by_k[i])
        printf(" k%d=%zu/%zu", i,
               atomic_load_explicit(&b->done_by_k[i], memory_order_relaxed), b->total_by_k[i]);
    fputc('\n', stdout);
    printf("PROGRESS_PARTS phase=%s", b->phase);
    for (i = 0; i <= MAXP; i++) if (b->total_by_np[i])
        printf(" np%d=%zu/%zu", i,
               atomic_load_explicit(&b->done_by_np[i], memory_order_relaxed), b->total_by_np[i]);
    fputc('\n', stdout);

    for (z = 0; z < 3 && oldest_task[z] < b->ntasks; z++) {
        size_t current = atomic_load_explicit(&b->progress[oldest_worker[z]].task,
                                              memory_order_acquire);
        if (current != oldest_task[z]) continue;
        const VerifyTask *t = &b->tasks[oldest_task[z]];
        unsigned long long cursor = atomic_load_explicit(&b->progress[oldest_worker[z]].nodes,
                                                         memory_order_relaxed);
        printf("PROGRESS_ACTIVE phase=%s k=%d worker=%d task=%zu age_s=%.1f nodes_cursor=%llu "
               "np=%d mass=%d state=", b->phase, t->k, oldest_worker[z], oldest_task[z],
               oldest_age[z] * 1e-9, cursor, t->fact->np, t->fact->mass);
        write_fact_inline(stdout, t->fact);
        fputc('\n', stdout);
    }
    fflush(stdout);
    *last_done = done;
    *last_time = now;
}

static void *batch_progress_reporter(void *vp) {
    Batch *b = vp;
    size_t last_done = 0;
    double last_time = b->started;
    double ewma_rate = 0;
    pthread_mutex_lock(&b->progress_mu);
    while (!b->progress_stop) {
        struct timespec until = realtime_after(g_progress_seconds);
        int rc = 0;
        while (!b->progress_stop && rc != ETIMEDOUT)
            rc = pthread_cond_timedwait(&b->progress_cv, &b->progress_mu, &until);
        if (b->progress_stop) break;
        pthread_mutex_unlock(&b->progress_mu);
        print_batch_progress(b, &last_done, &last_time, &ewma_rate);
        pthread_mutex_lock(&b->progress_mu);
    }
    pthread_mutex_unlock(&b->progress_mu);
    return NULL;
}

static double run_batch(VerifyTask *tasks, size_t ntasks, int threads, int memo_bits,
                        const char *phase, int progress_k, BatchStats *total) {
    double t0 = monotonic_seconds();
    int i;
    memset(total, 0, sizeof *total);
    if (!ntasks) return 0;
    if (threads < 1) threads = 1;
    if ((size_t)threads > ntasks) threads = (int)ntasks;
    Batch b = { .tasks = tasks, .ntasks = ntasks, .memo_bits = memo_bits,
                .threads = threads, .progress_k = progress_k, .phase = phase,
                .started = t0 };
    atomic_init(&b.next, 0);
    atomic_init(&b.done, 0); atomic_init(&b.verified, 0);
    atomic_init(&b.unverified, 0); atomic_init(&b.budget, 0);
    atomic_init(&b.completed_nodes, 0);
    for (i = 0; i <= MAXK; i++) atomic_init(&b.done_by_k[i], 0);
    for (i = 0; i <= MAXP; i++) atomic_init(&b.done_by_np[i], 0);
    for (size_t q = 0; q < ntasks; q++) {
        b.total_by_k[tasks[q].k]++;
        b.total_by_np[tasks[q].fact->np]++;
    }
    b.progress = calloc((size_t)threads, sizeof *b.progress);
    WorkerArg *args = calloc((size_t)threads, sizeof *args);
    pthread_t *ids = threads > 1 ? malloc((size_t)(threads - 1) * sizeof *ids) : NULL;
    pthread_t reporter;
    int have_reporter = 0;
    if (!b.progress || !args || (threads > 1 && !ids)) {
        fprintf(stderr, "no memory for worker pool\n"); exit(1);
    }
    for (i = 0; i < threads; i++) {
        atomic_init(&b.progress[i].task, SIZE_MAX);
        atomic_init(&b.progress[i].started_ns, 0);
        atomic_init(&b.progress[i].nodes, 0);
        args[i].batch = &b; args[i].slot = i;
    }
    if (progress_k >= 0)
        printf("BATCH_START phase=%s k=%d targets=%zu threads=%d progress_seconds=%.1f\n",
               phase, progress_k, ntasks, threads, g_progress_seconds);
    else
        printf("BATCH_START phase=%s k=all targets=%zu threads=%d progress_seconds=%.1f\n",
               phase, ntasks, threads, g_progress_seconds);
    fflush(stdout);
    if (g_progress_seconds > 0) {
        pthread_mutex_init(&b.progress_mu, NULL);
        pthread_cond_init(&b.progress_cv, NULL);
        if (pthread_create(&reporter, NULL, batch_progress_reporter, &b)) {
            fprintf(stderr, "cannot create verifier progress reporter\n"); exit(1);
        }
        have_reporter = 1;
    }
    for (i = 0; i < threads - 1; i++)
        if (pthread_create(&ids[i], NULL, batch_worker, &args[i])) {
            fprintf(stderr, "cannot create verifier worker %d\n", i); exit(1);
        }
    batch_worker(&args[threads - 1]);
    for (i = 0; i < threads - 1; i++) pthread_join(ids[i], NULL);
    if (have_reporter) {
        pthread_mutex_lock(&b.progress_mu);
        b.progress_stop = 1;
        pthread_cond_signal(&b.progress_cv);
        pthread_mutex_unlock(&b.progress_mu);
        pthread_join(reporter, NULL);
        pthread_cond_destroy(&b.progress_cv);
        pthread_mutex_destroy(&b.progress_mu);
    }
    for (i = 0; i < threads; i++) add_batch_stats(total, &args[i].stats);
    double wall = monotonic_seconds() - t0;
    if (progress_k >= 0)
        printf("BATCH_DONE phase=%s k=%d completed=%zu/%zu verified=%zu unverified=%zu "
               "budget=%zu nodes=%llu wall_s=%.2f\n",
               phase, progress_k, atomic_load(&b.done), ntasks, atomic_load(&b.verified),
               atomic_load(&b.unverified), atomic_load(&b.budget),
               atomic_load(&b.completed_nodes), wall);
    else
        printf("BATCH_DONE phase=%s k=all completed=%zu/%zu verified=%zu unverified=%zu "
               "budget=%zu nodes=%llu wall_s=%.2f\n",
               phase, atomic_load(&b.done), ntasks, atomic_load(&b.verified),
               atomic_load(&b.unverified), atomic_load(&b.budget),
               atomic_load(&b.completed_nodes), wall);
    fflush(stdout);
    free(ids); free(args); free(b.progress);
    return wall;
}

/* ---------------------------------------------------------------- log parsing */

static Fact *lvl[MAXK + 1];
static int lvln[MAXK + 1], lvlcap[MAXK + 1];
static Fact *rootlvl[MAXK + 1];
static int rootn[MAXK + 1], rootcap[MAXK + 1];

static void add_fact(const Fact *f, int k) {
    if (k < 0 || k > MAXK) return;
    if (lvln[k] == lvlcap[k]) {
        lvlcap[k] = lvlcap[k] ? lvlcap[k] * 2 : 1024;
        lvl[k] = realloc(lvl[k], lvlcap[k] * sizeof(Fact));
    }
    lvl[k][lvln[k]++] = *f;
}

static void add_root(const Fact *f, int k) {
    if (k < 0 || k > MAXK) return;
    if (rootn[k] == rootcap[k]) {
        rootcap[k] = rootcap[k] ? rootcap[k] * 2 : 16;
        rootlvl[k] = realloc(rootlvl[k], (size_t)rootcap[k] * sizeof(Fact));
    }
    rootlvl[k][rootn[k]++] = *f;
}

/* The durable text form is intentionally boring:

       radio-negative-certificate-v1
       # arbitrary comments
       meta source-sha256 <hex>
       root 9 Sb(112:81)
       fact 8 Sb(53:52,44:44)

   The same parser continues to accept raw `can't solve` lines.  Masses are derived rather than
   stored, so a stale hand-copied annotation cannot become part of the proof. */
#define CERT_HEADER "radio-negative-certificate-v1"

static int is_cert_header(char *p) {
    size_t n = strlen(CERT_HEADER);
    return !strncmp(p, CERT_HEADER, n)
        && (p[n] == 0 || p[n] == '\n' || p[n] == '\r' || p[n] == ' ' || p[n] == '\t');
}

static int parse_sb(char *p, Fact *f, char **endp) {
    Part in[MAXP];
    int cnt = 0;
    p = strstr(p, "Sb(");
    if (!p) return 0;
    p += 3;
    while (*p && *p != ')' && cnt < MAXP) {
        long a = strtol(p, &p, 10);
        if (*p != ':') return 0;
        p++;
        long b = strtol(p, &p, 10);
        if (a < 0 || b < 0 || a > 255 || b > 255) return 0;
        in[cnt].n = (unsigned char)a; in[cnt].m = (unsigned char)b; cnt++;
        if (*p == ',') p++;
        else if (*p != ')') return 0;
    }
    if (*p != ')' || cnt <= 0) return 0;
    if (endp) *endp = p + 1;
    return canon(in, cnt, f);
}

/* 0 ignore, 1 fact, 2 root, -1 malformed certificate record. */
static int parse_input_line(char *line, int cert_mode, Fact *f, int *k) {
    char *p = line;
    while (*p == ' ' || *p == '\t') p++;
    if (!*p || *p == '\n' || *p == '#') return 0;
    if (is_cert_header(p) || !strncmp(p, "meta ", 5)) return 0;
    if (!strncmp(p, "fact ", 5) || !strncmp(p, "root ", 5)) {
        int root = *p == 'r';
        char *q = p + 5, *end;
        char *num;
        while (*q == ' ' || *q == '\t') q++;
        num = q;
        long kk = strtol(q, &q, 10);
        if (q == num || kk < 1 || kk > MAXK || !parse_sb(q, f, &end)) return -1;
        while (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n') end++;
        if (*end && *end != '#') return -1;
        *k = (int)kk;
        return root ? 2 : 1;
    }
    if (!strncmp(p, "can't solve", 11)) {
        char *q = strstr(p, "] in ");
        if (!q || !parse_sb(p, f, NULL)) return cert_mode ? -1 : 0;
        *k = atoi(q + 5);
        if (*k < 1 || *k > MAXK) return cert_mode ? -1 : 0;
        return 1;
    }
    return cert_mode ? -1 : 0;
}

static long long read_input_file(const char *fname, int fidx, int facts_as_roots) {
    static char line[1 << 16];
    FILE *fp = fopen(fname, "r");
    long long nread = 0;
    int lineno = 0, cert_mode = 0;
    if (!fp) { fprintf(stderr, "cannot open %s\n", fname); exit(2); }
    while (fgets(line, sizeof line, fp)) {
        Fact f;
        int k, kind;
        lineno++;
        { char *p = line; while (*p == ' ' || *p == '\t') p++;
          if (is_cert_header(p)) cert_mode = 1; }
        kind = parse_input_line(line, cert_mode, &f, &k);
        if (kind < 0) {
            fprintf(stderr, "%s:%d: malformed or unknown certificate record\n", fname, lineno);
            exit(2);
        }
        if (!kind) continue;
        f.src = (unsigned char)(fidx < 8 ? 1u << fidx : 0x80);
        if (facts_as_roots || kind == 2) add_root(&f, k); else add_fact(&f, k);
        nread++;
    }
    fclose(fp);
    return nread;
}

static void dedup_facts(Fact *f, int *np) {
    int i, w = 0;
    if (!*np) return;
    qsort(f, *np, sizeof(Fact), fact_cmp);
    for (i = 0; i < *np; i++)
        if (!i || !feq(&f[i], &f[w - 1])) f[w++] = f[i];
        else f[w - 1].src |= f[i].src;
    *np = w;
}

static void write_state(FILE *fp, const char *kind, int k, const Fact *f) {
    fprintf(fp, "%s %d ", kind, k);
    write_fact_inline(fp, f);
    fputc('\n', fp);
}

static void write_certificate(const char *path, const Level *L, int maxk, int colored, int topk) {
    FILE *fp = fopen(path, "w");
    int k, i, nr = 0;
    long long nf = 0;
    if (!fp) { fprintf(stderr, "cannot write %s\n", path); exit(2); }
    fprintf(fp, "radio-negative-certificate-v1\n");
    fprintf(fp, "# Canonical facts; masses are derived by the checker.\n");
    for (k = 0; k <= maxk && k <= MAXK; k++) nr += rootn[k];
    if (nr) {
        for (k = maxk; k >= 0; k--)
            for (i = 0; i < rootn[k]; i++) write_state(fp, "root", k, &rootlvl[k][i]);
    } else if (colored) {
        for (i = 0; i < L[topk].n; i++) write_state(fp, "root", topk, &L[topk].f[i]);
    }
    for (k = maxk; k >= 0; k--)
        for (i = 0; i < L[k].n; i++) {
            if (colored && (k >= topk || !L[k].cited[i])) continue;
            write_state(fp, "fact", k, &L[k].f[i]);
            nf++;
        }
    if (fclose(fp)) { fprintf(stderr, "cannot finish %s\n", path); exit(2); }
    printf("certificate %s: %d roots, %lld facts%s\n", path,
           nr ? nr : (colored ? L[topk].n : 0), nf, colored ? " (colored)" : "");
}

int main(int argc, char **argv) {
    if (argc < 2) { printf("usage: %s <log>[,<log>...] [maxk]\n", argv[0]); return 2; }
    int maxk = argc > 2 ? atoi(argv[2]) : MAXK;
    int threads = 1, memo_bits;
    { char *e = getenv("VERIFY_THREADS"); if (e) threads = atoi(e); }
    if (threads < 1) { fprintf(stderr, "VERIFY_THREADS must be positive\n"); return 2; }
    { char *e = getenv("VERIFY_PROGRESS_SECONDS");
      if (e) {
          char *end = NULL;
          errno = 0;
          g_progress_seconds = strtod(e, &end);
          if (errno || end == e || *end || g_progress_seconds < 0
                  || g_progress_seconds > 86400 || g_progress_seconds != g_progress_seconds) {
              fprintf(stderr, "VERIFY_PROGRESS_SECONDS must be in 0..86400\n"); return 2;
          }
      }
    }
    memo_bits = default_memo_bits(threads);
    { char *e = getenv("VERIFY_MEMO_BITS"); if (e) memo_bits = atoi(e); }
    if (argc > 3) g_order = atoi(argv[3]);
    { char *e = getenv("BENCH_K"); if (e) g_bench = atoi(e); }
    { char *e = getenv("NODECAP"); if (e) g_nodecap = atoll(e); }
    { char *e = getenv("TIMECAP"); if (e) g_timecap = atof(e); }
    if (argc > 4) g_fcen = atoi(argv[4]);
    if (argc > 5) g_fcmin = atoi(argv[5]);
    if (argc > 6) g_minp = atoi(argv[6]);
    if (argc > 7) g_maxp = atoi(argv[7]);
    if (argc > 8) g_stride = atoi(argv[8]);
    if (argc > 9) g_dpen = atoi(argv[9]);
    if (argc > 10) g_mink = atoi(argv[10]);
    if (argc > 11) g_diag = atoi(argv[11]);
    if (argc > 12) g_srcmask = atoi(argv[12]);
    if (argc > 13) g_derive = atoi(argv[13]);   /* bitmask of log indices to VERIFY;
                                                    all logs always serve as the database */
    if (threads > 1 && g_dpen) {
        fprintf(stderr, "parallel prototype does not duplicate the disabled 0.29 GB subtree memo\n");
        return 2;
    }
    if (threads > 1 && (g_timecap || g_diag)) {
        fprintf(stderr, "TIMECAP and diagnostic split printing are single-thread-only\n");
        return 2;
    }
    int i, k;
    for (i = 0, pow3[0] = 1; i < MAXK; i++) pow3[i + 1] = pow3[i] * 3;
    build_G();
    build_zob();

    /* a comma-separated list of logs. The union of fact sets is itself a fact set: every fact
       is a claim about one state at one k, and each is checked on its own merits. Merging logs
       from different runs and different eras is therefore sound - an unsound fact cannot be
       laundered by the company it keeps, it just fails to verify. */
    long long nread = 0;
    char *logs = strdup(argv[1]), *fname, *save = NULL;
    int fidx = 0;
    for (fname = strtok_r(logs, ",", &save); fname; fname = strtok_r(NULL, ",", &save), fidx++)
        nread += read_input_file(fname, fidx, 0);
    free(logs);
    if (getenv("ROOTS")) nread += read_input_file(getenv("ROOTS"), fidx, 1);
    { int nr = 0;
      for (k = 0; k <= MAXK; k++) nr += rootn[k];
      printf("inputs %s: %lld records parsed (%d explicit roots), threads=%d memo=2^%d/worker\n",
             argv[1], nread, nr, threads, memo_bits); }
    fflush(stdout);

    /* dedup each level, then freeze */
    static Level L[MAXK + 1];
    g_levels = L;
    for (k = 0; k <= MAXK; k++) {
        dedup_facts(lvl[k], &lvln[k]);
        dedup_facts(rootlvl[k], &rootn[k]);
        L[k].n = lvln[k]; L[k].f = lvl[k];
        level_freeze(&L[k]);
    }
#ifdef VERIFY_BLOCK_PARETO
    printf("block Pareto index: size=%d, min level=%d facts, %lld blocks, %lld points "
           "(%.2f/block, %.1f%% of indexed facts), "
           "max front %d, %.2f s build, %.1f MiB summaries\n",
           VERIFY_BLOCK_SIZE, VERIFY_BLOCK_MIN_LEVEL_FACTS,
           g_block_build_blocks, g_block_build_points,
           (double)g_block_build_points / (g_block_build_blocks ? g_block_build_blocks : 1),
           100.0 * g_block_build_points / (g_block_build_facts ? g_block_build_facts : 1),
           g_block_build_max_front, g_block_build_seconds,
           (double)(g_block_build_points * (long long)sizeof(BlockProfile)
                    + g_block_build_blocks * ((long long)sizeof(BlockProfile) + sizeof(int32_t))
                    + g_block_build_facts * (long long)sizeof(int32_t)
                    + (MAXK + 1) * (long long)sizeof(int32_t))
                    / (1024.0 * 1024.0));
    fflush(stdout);
#endif
    for (k = 1; k <= MAXK; k++) if (rootn[k] && (k < g_mink || k > maxk)) {
        fprintf(stderr, "explicit root at k=%d is outside the requested verification range %d..%d\n",
                k, g_mink, maxk);
        return 2;
    }

    if (getenv("CERT_ONLY")) {
        if (!getenv("CERT_OUT")) {
            fprintf(stderr, "CERT_ONLY requires CERT_OUT=<path>\n"); return 2;
        }
        write_certificate(getenv("CERT_OUT"), L, maxk < MAXK ? maxk : MAXK, 0, 0);
        return 0;
    }

    if (getenv("MINIMAL_K")) {
        /* How much of a level is redundant? The refuted set is upward-closed, so only the minimal
           antichain carries information for dominance queries: if g <= f and both are present, f
           can never be the reason a query succeeds that g would not already have answered.
           Everything non-minimal is pure scan cost. */
        int mk = atoi(getenv("MINIMAL_K")), q, np;
        long long minimal[MAXP + 2] = {0}, total[MAXP + 2] = {0};
        clock_t t0 = clock();
        const Level *L2 = &L[mk];
        for (q = 0; q < L2->n; q++) {
            const Fact *f = &L2->f[q];
            int dominated = level_redundant(L2, q);
            total[f->np]++;
            if (!dominated) minimal[f->np]++;
        }
        printf("MINIMALITY of level k=%d (%.1f s)\n", mk, (double)(clock()-t0)/CLOCKS_PER_SEC);
        { long long tt = 0, mm = 0;
          for (np = 0; np <= MAXP; np++) if (total[np]) {
              printf("  np=%-2d  %9lld facts  %9lld minimal  (%.1f%% redundant)\n",
                     np, total[np], minimal[np], 100.0*(total[np]-minimal[np])/total[np]);
              tt += total[np]; mm += minimal[np]; }
          printf("  TOTAL %9lld facts  %9lld minimal  (%.1f%% redundant)\n",
                 tt, mm, 100.0*(tt-mm)/(tt?tt:1)); }
        return 0;
    }
    if (g_bench) {
        /* Isolate the dominance index: issue one refuted() query per fact at level BENCH_K
           against the level below, in file order, and time it. Same queries for any build, so
           the difference is the lookup structure and nothing else. */
        /* The real workload: build the live-split table for every distinct part occurring at
           level bk. That issues three queries per candidate option against the level below, and
           those are genuine misses - the states are split children, not logged facts. This is
           exactly the work a per-(part,k) hint file would replace. */
        int bk = g_bench, q, j;
        BatchStats ignored;
        worker_state_init(memo_bits);
        static unsigned char seen[NPART];
        long long nparts = 0, nopts = 0;
        clock_t t0 = clock();
        for (q = 0; q < L[bk].n; q++)
            for (j = 0; j < L[bk].f[q].np; j++) {
                Part pp = L[bk].f[q].p[j];
                int idx = (pp.n << 8) | pp.m;
                if (seen[idx]) continue;
                seen[idx] = 1; nparts++;
                int cnt; live_get(&L[bk - 1], bk, pp, 0, &cnt);
                nopts += cnt;
            }
        printf("BENCH k=%d: %lld distinct parts, %lld live options, %.3f s (%.1f ms/part)\n",
               bk, nparts, nopts, (double)(clock() - t0) / CLOCKS_PER_SEC,
               1e3 * (clock() - t0) / CLOCKS_PER_SEC / (nparts ? nparts : 1));
        worker_state_finish(&ignored);
        return 0;
    }
    if (getenv("TOPDOWN")) {
        /* Top-down reachability. The certificate is not "every fact the solver logged" - it is the
           sub-DAG reachable from the roots. Verify level k, painting each fact at k-1 that was
           actually used to discharge something, then descend and verify only the painted ones.
           Unpainted facts are discarded: nothing cites them, so nothing depends on them.

           Still well-founded induction on k, so soundness is untouched - every cited fact is
           itself verified before the certificate closes. What changes is that both the artifact
           and the verification work shrink to what the proof needs. */
        int topk = atoi(getenv("TOPDOWN")), kk, pass, final_bad = 0, nr = 0;
        int npass = getenv("PASSES") ? atoi(getenv("PASSES")) : 1;
        if (topk < 2 || topk > MAXK) { fprintf(stderr, "TOPDOWN must be in 2..%d\n", MAXK); return 2; }
        for (kk = 0; kk <= MAXK; kk++) nr += rootn[kk];
        if (nr) for (kk = 0; kk <= MAXK; kk++) if (kk != topk && rootn[kk]) {
            fprintf(stderr, "top-down coloring requires every explicit root at k=%d\n", topk);
            return 2;
        }
        if (getenv("MINIMIZE_BEFORE_COLOR")) {
            printf("subsumption-minimalizing support levels before coloring\n");
            for (kk = 1; kk < topk; kk++) if (L[kk].n > 1) {
                int before = L[kk].n;
                double t0 = monotonic_seconds();
                int removed = level_minimize(&L[kk], threads, kk);
                printf("  k=%d: %d -> %d facts, removed %d (%.2f s)\n",
                       kk, before, L[kk].n, removed, monotonic_seconds() - t0);
            }
        }
        g_paint = 1;
        for (pass = 1; pass <= npass; pass++) {
            int pass_bad = 0;
            printf("--- pass %d%s ---\n", pass,
                   pass > 1 ? " (preferring witnesses already in the artifact)" : "");
            printf("%3s %10s %10s %10s %8s %8s %10s %12s\n",
                   "k", "in level", "targets", "verified", "unver", "budget", "wall", "cited k-1");
            for (kk = topk; kk >= (g_mink > 1 ? g_mink : 1); kk--) {
                int use_roots = kk == topk && nr;
                int use_sub = kk != topk && pass > 1 && L[kk].sub;
                int nt = 0, q, w = 0, ver = 0, un = 0, budg = 0, cited = 0;
                BatchStats bs;
                VerifyTask *tasks;
                if (use_roots) nt = rootn[kk];
                else if (kk == topk) nt = L[kk].n;
                else if (use_sub) nt = L[kk].sub->n;
                else for (q = 0; q < L[kk].n; q++) nt += L[kk].cited[q] ? 1 : 0;
                tasks = nt ? malloc((size_t)nt * sizeof *tasks) : NULL;
                if (nt && !tasks) { fprintf(stderr, "no memory for coloring tasks\n"); return 1; }
                for (q = 0; q < nt; q++) {
                    const Fact *f;
                    if (use_roots) f = &rootlvl[kk][q];
                    else if (kk == topk) f = &L[kk].f[q];
                    else if (use_sub) f = &L[kk].f[L[kk].sub->orig[q]];
                    else {
                        while (w < L[kk].n && !L[kk].cited[w]) w++;
                        f = &L[kk].f[w++];
                    }
                    tasks[q].k = kk; tasks[q].fact = f;
                    tasks[q].status = 0; tasks[q].nodes = 0;
                }
                double sec = run_batch(tasks, (size_t)nt, threads, memo_bits,
                                       "color", kk, &bs);
                for (q = 0; q < nt; q++) {
                    if (tasks[q].status == 1) ver++;
                    else if (tasks[q].status == 2) budg++;
                    else un++;
                }
                for (q = 0; q < L[kk - 1].n; q++) cited += L[kk - 1].cited[q] ? 1 : 0;
                printf("%3d %10d %10d %10d %8d %8d %10.2f %12d  (prefer %lld/%lld)\n",
                       kk, L[kk].n, nt, ver, un, budg, sec, cited,
                       bs.pref_hits, bs.pref_hits + bs.pref_miss);
                fflush(stdout);
                pass_bad += un + budg;
                free(tasks);
            }
            final_bad = pass_bad;
            if (pass == npass) break;
            /* Freeze this pass's painting as the next pass's preference set, then repaint.
               Worker-local live/pair tables have already been destroyed, so pass 2 cannot reuse
               a memo that would silently suppress its citations. */
            for (kk = topk; kk >= 1; kk--) {
                if (!L[kk].n) continue;
                Level *ns = level_sub_cited(&L[kk]);
                if (ns) L[kk].sub = ns;
                memset(L[kk].cited, 0, (size_t)L[kk].n);
            }
        }
        if (getenv("CERT_OUT") && g_mink > 1) {
            fprintf(stderr, "not writing a partial colored certificate stopped at k=%d\n", g_mink);
            return 2;
        } else if (getenv("CERT_OUT") && !final_bad)
            write_certificate(getenv("CERT_OUT"), L, topk, 1, topk);
        else if (getenv("CERT_OUT"))
            fprintf(stderr, "not writing an incomplete colored certificate (%d unresolved targets)\n",
                    final_bad);
        return final_bad ? 1 : 0;
    }
    if (getenv("CERT_OUT") &&
            (g_mink != 1 || g_minp > 0 || g_maxp < MAXP || g_stride != 1 || g_srcmask)) {
        fprintf(stderr, "CERT_OUT requires full unfiltered verification; use CERT_ONLY to normalize\n");
        return 2;
    }
    { int nr = 0;
      for (k = 0; k <= MAXK; k++) nr += rootn[k];
      if (threads > 1 || nr) {
        long long selected[MAXK + 1] = {0}, ver[MAXK + 1] = {0};
        long long un[MAXK + 1] = {0}, budg[MAXK + 1] = {0}, nodes[MAXK + 1] = {0};
        long long pick = 0, total_tasks = 0, q;
        BatchStats bs;
        for (k = g_mink; k <= maxk && k <= MAXK; k++) {
            for (i = 0; i < L[k].n; i++) {
                if (L[k].f[i].np < g_minp || L[k].f[i].np > g_maxp) continue;
                if (g_srcmask && !(L[k].f[i].src & g_srcmask)) continue;
                if (g_stride > 1 && (pick++ % g_stride)) continue;
                selected[k]++; total_tasks++;
            }
            for (i = 0; i < rootn[k]; i++) { selected[k]++; total_tasks++; }
        }
        VerifyTask *tasks = total_tasks ? malloc((size_t)total_tasks * sizeof *tasks) : NULL;
        if (total_tasks && !tasks) { fprintf(stderr, "no memory for verification tasks\n"); return 1; }
        pick = q = 0;
        for (k = g_mink; k <= maxk && k <= MAXK; k++) {
            for (i = 0; i < L[k].n; i++) {
                if (L[k].f[i].np < g_minp || L[k].f[i].np > g_maxp) continue;
                if (g_srcmask && !(L[k].f[i].src & g_srcmask)) continue;
                if (g_stride > 1 && (pick++ % g_stride)) continue;
                tasks[q].k = k; tasks[q].fact = &L[k].f[i]; tasks[q++].status = 0;
            }
            for (i = 0; i < rootn[k]; i++) {
                tasks[q].k = k; tasks[q].fact = &rootlvl[k][i]; tasks[q++].status = 0;
            }
        }
        double wall = run_batch(tasks, (size_t)total_tasks, threads, memo_bits,
                                "verify", -1, &bs);
        for (q = 0; q < total_tasks; q++) {
            k = tasks[q].k; nodes[k] += tasks[q].nodes;
            if (tasks[q].status == 1) ver[k]++;
            else if (tasks[q].status == 2) budg[k]++;
            else {
                un[k]++;
                if (un[k] <= 3) write_state(stdout, "UNVERIFIED", k, tasks[q].fact);
            }
        }
        printf("%3s %10s %10s %8s %8s %14s\n",
               "k", "targets", "verified", "unver", "budget", "nodes");
        for (k = g_mink; k <= maxk && k <= MAXK; k++) if (selected[k])
            printf("%3d %10lld %10lld %8lld %8lld %14lld\n",
                   k, selected[k], ver[k], un[k], budg[k], nodes[k]);
        { long long tv = 0, tu = 0, tb = 0;
          for (k = 0; k <= MAXK; k++) { tv += ver[k]; tu += un[k]; tb += budg[k]; }
          printf("\nTOTAL verified %lld, unverified %lld, budget %lld, nodes %lld, %.2f s wall on %d thread%s\n",
                 tv, tu, tb, bs.nodes, wall, threads, threads == 1 ? "" : "s");
          printf("memo: %lld hits, %lld misses (%.1f%%); live tables %lld built/%lld reused; "
                 "pair tables %lld built/%lld reused\n",
                 bs.memo_hit, bs.memo_miss,
                 100.0 * bs.memo_hit / (bs.memo_hit + bs.memo_miss ? bs.memo_hit + bs.memo_miss : 1),
                 bs.live_built, bs.live_reused, bs.pair_built, bs.pair_reused);
#ifdef VERIFY_INDEX_STATS
          printf("dominance index: %lld candidates; product rejected %lld (%.1f%%); "
                 "n/m rejected %lld; exact matching %lld calls/%lld hits\n",
                 bs.idx_candidates, bs.idx_product_rejects,
                 100.0 * bs.idx_product_rejects / (bs.idx_candidates ? bs.idx_candidates : 1),
                 bs.idx_nm_rejects, bs.idx_match_calls, bs.idx_match_hits);
#ifdef VERIFY_BLOCK_PARETO
          printf("block filter: %lld blocks tested/%lld rejected (%.1f%%; %lld by minima); "
                 "%lld positions skipped; %lld front points tested (%.2f/block)\n",
                 bs.idx_block_tests, bs.idx_block_rejects,
                 100.0 * bs.idx_block_rejects / (bs.idx_block_tests ? bs.idx_block_tests : 1),
                 bs.idx_block_min_rejects, bs.idx_block_skipped, bs.idx_block_front_tests,
                 (double)bs.idx_block_front_tests / (bs.idx_block_tests ? bs.idx_block_tests : 1));
#endif
#endif
          if (getenv("CERT_OUT") && !tu && !tb)
              write_certificate(getenv("CERT_OUT"), L, maxk < MAXK ? maxk : MAXK, 0, 0);
          else if (getenv("CERT_OUT"))
              fprintf(stderr, "not writing a certificate with unresolved facts\n");
          free(tasks);
          return tu || tb ? 1 : 0; }
      }
    }

    /* Preserve the detailed single-thread instrumentation as the reference execution path. */
    worker_state_init(memo_bits);
    printf("%3s %10s %10s %8s %9s %14s %8s\n",
           "k", "facts", "verified", "unver", "sec", "nodes", "live");
    long long tot_ver = 0, tot_un = 0, tot_nodes = 0;
    double tot_s = 0;
    for (k = g_mink; k <= maxk && k <= MAXK; k++) {
        if (!L[k].n) continue;
        memset(memo, 0xff, (memo_mask + 1) * sizeof(MemoEnt));   /* level set changed */
        clock_t t0 = clock();
        long long n0 = g_nodes;
        int ver = 0, un = 0;
        g_max_fact_nodes = 0;
        clock_t tick = clock();
        for (i = 0; i < L[k].n; i++) {
            if (clock() - tick > 10 * CLOCKS_PER_SEC) {
                tick = clock();
                { long long done = 0; int z;
                  for (z = 0; z < 24; z++) done += cost_hist[z];
                  fprintf(stderr, "    k=%d  %d/%d  nodes=%.3g  memo=%.1f%%  "
                          "| done %lld, over budget %lld, mean %.0f nodes\n",
                          k, i, L[k].n, (double)(g_nodes - n0),
                          100.0 * memo_hit / (memo_hit + memo_miss ? memo_hit + memo_miss : 1),
                          done, budget_out, (double)cost_sum / (done ? done : 1)); }
            }
            if (L[k].f[i].np < g_minp || L[k].f[i].np > g_maxp) continue;
            if (g_srcmask && !(L[k].f[i].src & g_srcmask)) continue;   /* not a target fact */
            if (g_stride > 1 && (sel++ % g_stride)) continue;
            if (verify(&L[k - 1], &L[k].f[i], k)) {
                if (g_budget_hit) { budget_out++; }
                else { ver++;
                    int b = 0; long long v = g_fact_nodes;
                    while (v > 1 && b < 23) { v >>= 1; b++; }
                    cost_hist[b]++; cost_sum += g_fact_nodes; }
            }
            else { un++; if (un <= 3) {
                printf("    UNVERIFIED k=%d Sb(", k);
                int j; for (j = 0; j < L[k].f[i].np; j++)
                    printf("%s%d:%d", j ? "," : "", L[k].f[i].p[j].n, L[k].f[i].p[j].m);
                printf(")[%d]\n", L[k].f[i].mass); } }
        }
        double sec = (double)(clock() - t0) / CLOCKS_PER_SEC;
        printf("%3d %10d %10d %8d %9.2f %14lld %8lld  max/fact %.3g memo %.3f%%\n",
               k, L[k].n, ver, un, sec, g_nodes - n0, live_built,
               (double)g_max_fact_nodes,
               100.0 * memo_hit / (memo_hit + memo_miss ? memo_hit + memo_miss : 1));
        fflush(stdout);
        tot_ver += ver; tot_un += un; tot_nodes += g_nodes - n0; tot_s += sec;
    }
    printf("memo: %lld hits, %lld misses (%.1f%% hit rate)\n",
           memo_hit, memo_miss, 100.0 * memo_hit / (memo_hit + memo_miss ? memo_hit + memo_miss : 1));
#ifdef VERIFY_INDEX_STATS
    printf("dominance index: %lld candidates; product rejected %lld (%.1f%%); "
           "n/m rejected %lld; exact matching %lld calls/%lld hits\n",
           idx_candidates, idx_product_rejects,
           100.0 * idx_product_rejects / (idx_candidates ? idx_candidates : 1),
           idx_nm_rejects, idx_match_calls, idx_match_hits);
#ifdef VERIFY_BLOCK_PARETO
    printf("block filter: %lld blocks tested/%lld rejected (%.1f%%; %lld by minima); "
           "%lld positions skipped; %lld front points tested (%.2f/block)\n",
           idx_block_tests, idx_block_rejects,
           100.0 * idx_block_rejects / (idx_block_tests ? idx_block_tests : 1),
           idx_block_min_rejects, idx_block_skipped, idx_block_front_tests,
           (double)idx_block_front_tests / (idx_block_tests ? idx_block_tests : 1));
#endif
#endif
    printf("\nTOTAL verified %lld, unverified %lld, nodes %lld, %.2f s single-threaded\n",
           tot_ver, tot_un, tot_nodes, tot_s);
    { int q; printf("nodes by part count:\n");
      for (q = 1; q <= MAXP; q++) if (np_facts[q])
          printf("  %2d parts %8lld facts %14lld nodes %10.0f/fact\n",
                 q, np_facts[q], np_nodes[q], (double)np_nodes[q] / np_facts[q]); }
    printf("pair tables: %lld built, %lld reused; fc prunes %lld, domain skips %lld\n",
           pair_built, pair_reused, fc_prunes, fc_dom_skips);
    if (pair_bits_tot) printf("pair row density q = %.4f (%lld of %lld option pairs live)\n",
           (double)pair_bits_set / pair_bits_tot, pair_bits_set, pair_bits_tot);
    if (derived_ok + derived_no)
        printf("derived on demand: %lld proved, %lld failed (%.1f%% proved)\n",
               derived_ok, derived_no, 100.0 * derived_ok / (derived_ok + derived_no));
    printf("subtree memo: %lld hits, %lld misses, %lld too-wide (%.1f%% hit), %.2f GB table\n",
           dp_hit, dp_miss, dp_skip,
           100.0 * dp_hit / (dp_hit + dp_miss ? dp_hit + dp_miss : 1),
           (double)DPSZ * sizeof(DpEnt) / 1e9);
    { int q; long long done = 0;
      for (q = 0; q < 24; q++) done += cost_hist[q];
      printf("COST DISTRIBUTION: %lld completed, %lld exceeded budget (%.1f%%), mean %.0f nodes\n",
             done, budget_out, 100.0 * budget_out / (done + budget_out ? done + budget_out : 1),
             (double)cost_sum / (done ? done : 1));
      for (q = 0; q < 24; q++) if (cost_hist[q])
          printf("   <=2^%-2d nodes: %lld\n", q, cost_hist[q]); }
    printf("live-split tables: %lld built, %lld reused (%.0fx)\n",
           live_built, live_reused, live_reused / (double)(live_built ? live_built : 1));
    if (getenv("CERT_OUT") && !tot_un && !budget_out)
        write_certificate(getenv("CERT_OUT"), L, maxk < MAXK ? maxk : MAXK, 0, 0);
    else if (getenv("CERT_OUT"))
        fprintf(stderr, "not writing a certificate with unresolved facts\n");
    { BatchStats ignored; worker_state_finish(&ignored); }
    return tot_un || budget_out ? 1 : 0;
}
