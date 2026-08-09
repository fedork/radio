// An independent checker for negative certificates.
//
// Reads a solver log, extracts the `can't solve Sb(...)` facts, and verifies every one of them
// from first principles. It does NOT include radiobase.c and shares no code with the solver -
// that independence is the whole point. It knows exactly four things:
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
//   * facts are grouped by k and checked level by level. A level-k check consults only the
//     k-1 facts, so resident memory is ONE level, not the certificate. Verification order does
//     not matter - soundness is well-founded induction on k over the conjunction of all checks
//     - so levels could equally be farmed out to separate machines.
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
//   clang -O3 radio_verify.c -o radio_verify
//   ./radio_verify <log> [maxk [group_order [pairwise [pairwise_min_parts]]]]
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

#define MAXK 12
#define MAXP 40                 /* parts per state */
#define NPART 65536             /* (n<<8)|m, so n,m <= 255 */

#define MAXC (2 * MAXP)          /* a mixed child has two parts per group */

typedef struct { unsigned char n, m; } Part;
typedef struct { unsigned char np, src; Part p[MAXP]; int mass; } Fact;

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

       Layout: two uint64 columns, eight 8-bit lanes each (n and m both fit a byte - the parser
       rejects anything above 255), so one candidate is a 16-byte read and the test is two
       vector byte-compares instead of a backtracking match. Facts are sorted by
       (np, largest n, mass), which turns "np <= np(s) and maxn <= maxn(s)" into a range and keeps
       the mass break inside each group. */
    uint64_t *pn, *pm;       /* per fact: N_1..N_8 and M_1..M_8, descending, 0-padded */
    int *b2;                 /* b2[np * 257 + x] = first index with np and largest-n >= x */
} Level;

typedef unsigned char u8x8 __attribute__((vector_size(8)));

/* every lane of a <= corresponding lane of b */
static inline int prof_le(uint64_t a, uint64_t b) {
    u8x8 va, vb, gt;
    uint64_t r;
    memcpy(&va, &a, 8); memcpy(&vb, &b, 8);
    gt = (u8x8)(va > vb);
    memcpy(&r, &gt, 8);
    return r == 0;
}

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

static int fact_cmp(const void *A, const void *B) {
    const Fact *a = A, *b = B;
    if (a->np != b->np) return a->np - b->np;
    { int an = a->np ? a->p[0].n : 0, bn = b->np ? b->p[0].n : 0;   /* largest n */
      if (an != bn) return an - bn; }
    if (a->mass != b->mass) return a->mass - b->mass;
    return memcmp(a->p, b->p, (a->np < b->np ? a->np : b->np) * sizeof(Part));
}

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
    for (i = 0; i < L->n; i++) prof_of(L->f[i].p, L->f[i].np, &L->pn[i], &L->pm[i]);
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
#define MEMOBITS 24
#define MEMOSZ (1 << MEMOBITS)
#define MEMOP 16
/* `wit` is the index of the fact that answered this query, or -1 for a rule (COUNT, MAJ) or a
   derivation. Carrying it through the memo is what makes top-down painting possible: 99.996% of
   queries at k=4 are memo hits, so without it almost every citation would go unrecorded. */
typedef struct { uint64_t h; int32_t wit; unsigned char np, k; signed char res; Part p[MEMOP]; } MemoEnt;
static int g_wit = -1;
static int g_paint = 0;
static long long paint_hits = 0, pref_hits = 0, pref_miss = 0;
static MemoEnt *memo;
#define C_REF
#define MEMO_HIT memo_hit++
#define MEMO_MISS memo_miss++
static long long memo_hit = 0, memo_miss = 0;

static int refuted_raw(const Level *L, const Fact *s, int k);
static int derive(const Fact *s, int k);      /* prove, rather than cite */
static const void *g_levels;                  /* the Level[] array, for derive() */
static int g_derive = 0;          /* derive missing facts at k <= this */
static long long derived_ok = 0, derived_no = 0;

static int refuted(const Level *L, const Fact *s, int k) {
    if (s->np > MEMOP) return refuted_raw(L, s, k);
    uint64_t h = fhash(s) + 0x51ULL * (uint64_t)k;
    MemoEnt *e = &memo[h & (MEMOSZ - 1)];
    if (e->res >= 0 && e->h == h && e->k == (unsigned char)k && e->np == s->np
            && memcmp(e->p, s->p, s->np * sizeof(Part)) == 0) {
        memo_hit++;
        if (g_paint && e->res && e->wit >= 0) { L->cited[e->wit] = 1; paint_hits++; }
        return e->res;
    }
    memo_miss++;
    int r = refuted_raw(L, s, k);
    e->h = h; e->k = (unsigned char)k; e->np = s->np; e->res = (signed char)r;
    e->wit = (int32_t)(r ? g_wit : -1);
    memcpy(e->p, s->p, s->np * sizeof(Part));
    if (g_paint && r && g_wit >= 0) { L->cited[g_wit] = 1; paint_hits++; }
    return r;
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
    { uint64_t qn, qm;
      int maxn = s->np ? s->p[0].n : 0;
      prof_of(s->p, s->np, &qn, &qm);
      for (np = 2; np <= lim; np++) {
          /* only facts with largest n <= maxn can inject, so the candidates are the range below
             b2[np][maxn+1]; mass is ordered inside each largest-n group, so break per group */
          int end = L->b2[np * 257 + (maxn < 256 ? maxn + 1 : 256)];
          for (i = L->bstart[np]; i < end; i++) {
              if (L->f[i].mass > s->mass) {          /* skip to the next largest-n group */
                  int an = L->f[i].p[0].n;
                  int nxt = L->b2[np * 257 + (an < 256 ? an + 1 : 256)];
                  if (nxt <= i) break;
                  i = nxt - 1;
                  continue;
              }
              if (!prof_le(L->pn[i], qn) || !prof_le(L->pm[i], qm)) continue;
              if (dominates(&L->f[i], s)) { g_wit = i; return 1; }
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
        MemoEnt *e = &memo[h & (MEMOSZ - 1)];
        if (e->res >= 0 && e->h == h && e->k == (unsigned char)k && e->np == c->np
                && memcmp(e->p, c->p, c->np * sizeof(Part)) == 0) {
            MEMO_HIT;
            if (g_paint && e->res && e->wit >= 0) { L->cited[e->wit] = 1; paint_hits++; }
            return e->res;
        }
        MEMO_MISS;
        if (c->np > MAXP) return 0;
        t.np = c->np; t.mass = c->mass;
        memcpy(t.p, c->p, c->np * sizeof(Part));
        int r = refuted_raw(L, &t, k);
        e->h = h; e->k = (unsigned char)k; e->np = c->np; e->res = (signed char)r;
        e->wit = (int32_t)(r ? g_wit : -1);
        if (g_paint && r && g_wit >= 0) { L->cited[g_wit] = 1; paint_hits++; }
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
static LiveTab *live_tab[MAXK + 1][2];
static long long live_built = 0, live_reused = 0;

static const Split *live_get(const Level *below, int k, Part p, int restrict_, int *cnt) {
    int idx = (p.n << 8) | p.m;
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
static PairEnt pairtab[PAIRSZ];
static long long pair_built = 0, pair_reused = 0, fc_prunes = 0, fc_dom_skips = 0;
static long long pair_bits_set = 0, pair_bits_tot = 0;

static const uint64_t *pair_get(const Level *below, int k, Part pi, int ri, Part pj, int rj,
                                int *wjp) {
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
static clock_t g_fact_t0;
static int g_budget_hit = 0;      /* measured a net 2.4x loss - see the journal; off by default */
static int g_diag = 0, g_diag_left, g_srcmask = 0;

#define DPBITS 20
#define DPSZ   (1 << DPBITS)
#define DPC    32                       /* children above this are not memoised */

typedef struct {
    uint32_t gen;
    uint16_t s2, s0;
    unsigned char i, lo, res, n2, n0, n1;
    Part c2[DPC], c0[DPC], c1[2 * DPC];
} DpEnt;

static DpEnt *dp;
static uint32_t dp_gen = 0;
static long long dp_hit = 0, dp_miss = 0, dp_skip = 0;

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

static const Level *g_below;
static int g_k, g_cap;
static const Fact *g_s;
static const Split *g_live[MAXP];
static int g_ln[MAXP], g_last[MAXP + 1];
static long long g_nodes, g_fact_nodes, g_max_fact_nodes;
/* one slot per depth: the child states for the prefix of that length */
static Chi st2[MAXP + 1], st0[MAXP + 1], st1[MAXP + 1];
static int g_fc;                                   /* forward checking enabled for this fact */
static const uint64_t *g_row[MAXP][MAXP];          /* g_row[i][j], i<j: rows of pair (i,j) */
static int g_wj[MAXP];                             /* words in group j's domain */
static uint64_t g_dom[MAXP + 1][MAXP][FCW];        /* live domain of each group, per depth */

static int splits_rec(int i, int s2, int s0, int s1) {
    g_nodes++; g_fact_nodes++;
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
static long long sel = 0;
static int g_bench = 0;
static int g_order = 0;                  /* 0 desc (canonical) 1 asc 2 fewest-options-first */
static long long np_nodes[MAXP + 1], np_facts[MAXP + 1];
static long long cost_hist[24], cost_sum, budget_out;
static Fact g_perm;

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

static int g_dderive = 0;                 /* recursion depth, for a sanity bound */

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

/* ---------------------------------------------------------------- log parsing */

static Fact *lvl[MAXK + 1];
static int lvln[MAXK + 1], lvlcap[MAXK + 1];

static void add_fact(const Fact *f, int k) {
    if (k < 0 || k > MAXK) return;
    if (lvln[k] == lvlcap[k]) {
        lvlcap[k] = lvlcap[k] ? lvlcap[k] * 2 : 1024;
        lvl[k] = realloc(lvl[k], lvlcap[k] * sizeof(Fact));
    }
    lvl[k][lvln[k]++] = *f;
}

int main(int argc, char **argv) {
    if (argc < 2) { printf("usage: %s <log>[,<log>...] [maxk]\n", argv[0]); return 2; }
    int maxk = argc > 2 ? atoi(argv[2]) : MAXK;
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
    dp = calloc(DPSZ, sizeof(DpEnt));
    if (!dp) { fprintf(stderr, "no memory for subtree memo\n"); return 1; }
    int i, k;
    for (i = 0, pow3[0] = 1; i < MAXK; i++) pow3[i + 1] = pow3[i] * 3;
    build_G();
    build_zob();
    for (k = 0; k <= MAXK; k++)
        for (i = 0; i < 2; i++) {
            live_tab[k][i] = malloc(NPART * sizeof(LiveTab));
            int j; for (j = 0; j < NPART; j++) { live_tab[k][i][j].n = -1; live_tab[k][i][j].s = NULL; }
        }

    memo = calloc(MEMOSZ, sizeof(MemoEnt));
    { int j; for (j = 0; j < MEMOSZ; j++) memo[j].res = -1; }

    /* a comma-separated list of logs. The union of fact sets is itself a fact set: every fact
       is a claim about one state at one k, and each is checked on its own merits. Merging logs
       from different runs and different eras is therefore sound - an unsound fact cannot be
       laundered by the company it keeps, it just fails to verify. */
    static char line[1 << 16];
    long long nread = 0;
    char *logs = strdup(argv[1]), *fname, *save = NULL;
    int fidx = 0;
    for (fname = strtok_r(logs, ",", &save); fname; fname = strtok_r(NULL, ",", &save), fidx++) {
    FILE *fp = fopen(fname, "r");
    if (!fp) { printf("cannot open %s\n", fname); return 2; }
    while (fgets(line, sizeof line, fp)) {
        if (strncmp(line, "can't solve", 11)) continue;
        char *p = strstr(line, "Sb(");
        if (!p) continue;
        p += 3;
        Part in[MAXP]; int cnt = 0;
        while (*p && *p != ')' && cnt < MAXP) {
            int a = strtol(p, &p, 10);
            if (*p != ':') break;
            p++;
            int b = strtol(p, &p, 10);
            in[cnt].n = (unsigned char)(a > 255 ? 255 : a);
            in[cnt].m = (unsigned char)(b > 255 ? 255 : b);
            if (a > 255 || b > 255) { cnt = -1; break; }
            cnt++;
            if (*p == ',') p++;
        }
        if (cnt <= 0) continue;
        char *q = strstr(p, "] in ");
        if (!q) continue;
        int kk = atoi(q + 5);
        Fact f;
        if (!canon(in, cnt, &f)) continue;
        f.src = (unsigned char)(fidx < 8 ? 1u << fidx : 0x80);
        add_fact(&f, kk);
        nread++;
    }
    fclose(fp);
    }
    printf("logs %s: %lld negative facts parsed\n", argv[1], nread); fflush(stdout);

    /* dedup each level, then freeze */
    static Level L[MAXK + 1];
    g_levels = L;
    for (k = 0; k <= MAXK; k++) {
        if (!lvln[k]) { L[k].n = 0; L[k].f = NULL; level_freeze(&L[k]); continue; }
        qsort(lvl[k], lvln[k], sizeof(Fact), fact_cmp);
        int w = 0;
        for (i = 0; i < lvln[k]; i++)
            if (!i || !feq(&lvl[k][i], &lvl[k][w - 1])) lvl[k][w++] = lvl[k][i];
            else lvl[k][w - 1].src |= lvl[k][i].src;
        L[k].n = w; L[k].f = lvl[k];
        level_freeze(&L[k]);
    }

    if (getenv("MINIMAL_K")) {
        /* How much of a level is redundant? The refuted set is upward-closed, so only the minimal
           antichain carries information for dominance queries: if g <= f and both are present, f
           can never be the reason a query succeeds that g would not already have answered.
           Everything non-minimal is pure scan cost. */
        int mk = atoi(getenv("MINIMAL_K")), q, np, i;
        long long minimal[MAXP + 2] = {0}, total[MAXP + 2] = {0};
        clock_t t0 = clock();
        const Level *L2 = &L[mk];
        for (q = 0; q < L2->n; q++) {
            const Fact *f = &L2->f[q];
            int dominated = 0, lim = f->np < MAXP ? f->np : MAXP;
            uint64_t qn, qm;
            int maxn = f->np ? f->p[0].n : 0;
            prof_of(f->p, f->np, &qn, &qm);
            for (np = 1; np <= lim && !dominated; np++) {
                int end = L2->b2[np * 257 + (maxn < 256 ? maxn + 1 : 256)];
                for (i = L2->bstart[np]; i < end; i++) {
                    if (i == q) continue;                       /* not itself */
                    if (L2->f[i].mass > f->mass) {
                        int an = L2->f[i].p[0].n;
                        int nxt = L2->b2[np * 257 + (an < 256 ? an + 1 : 256)];
                        if (nxt <= i) break;
                        i = nxt - 1; continue;
                    }
                    if (!prof_le(L2->pn[i], qn) || !prof_le(L2->pm[i], qm)) continue;
                    if (dominates(&L2->f[i], f)) { dominated = 1; break; }
                }
            }
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
        int topk = atoi(getenv("TOPDOWN")), kk, pass;
        int npass = getenv("PASSES") ? atoi(getenv("PASSES")) : 1;
        g_paint = 1;
        for (pass = 1; pass <= npass; pass++) {
            printf("--- pass %d%s ---\n", pass,
                   pass > 1 ? " (preferring witnesses already in the artifact)" : "");
            printf("%3s %10s %10s %10s %8s %10s %12s\n",
                   "k", "in level", "targets", "verified", "unver", "sec", "cited k-1");
            for (kk = topk; kk >= (g_mink > 2 ? g_mink : 2); kk--) {
                if (!L[kk].n) continue;
                { int z; for (z = 0; z < MEMOSZ; z++) memo[z].res = -1; }
                clock_t t0 = clock();
                int tgt = 0, ver = 0, un = 0, budg = 0, q;
                /* target list: everything at the top, otherwise what the level above painted.
                   On a later pass that list is the previous pass's painting, held in `sub`. */
                int use_sub = (kk != topk && pass > 1 && L[kk].sub);
                int nt = use_sub ? L[kk].sub->n : L[kk].n;
                for (q = 0; q < nt; q++) {
                    int fi = use_sub ? L[kk].sub->orig[q] : q;
                    if (!use_sub && kk != topk && !L[kk].cited[fi]) continue;
                    tgt++;
                    if (verify(&L[kk - 1], &L[kk].f[fi], kk)) {
                        if (g_budget_hit) budg++; else ver++;
                    } else un++;
                }
                int cited = 0;
                for (q = 0; q < L[kk - 1].n; q++) cited += L[kk - 1].cited[q];
                printf("%3d %10d %10d %10d %8d %10.1f %12d  (budget %d, prefer hit %lld/%lld)\n",
                       kk, L[kk].n, tgt, ver, un, (double)(clock() - t0) / CLOCKS_PER_SEC, cited,
                       budg, pref_hits, pref_hits + pref_miss);
                fflush(stdout);
            }
            if (pass == npass) break;
            /* Drop the live-split tables. They are memoised on (part, k) and were built during
               the previous pass, so replaying a level would reuse them and issue no refutation
               queries at all - and painting only happens where a query happens. Rebuilding costs
               0.24 s for a whole level, against silently painting nothing. */
            { int a, b;
              for (a = 0; a <= MAXK; a++)
                  for (b = 0; b < 2; b++)
                      if (live_tab[a][b]) {
                          int z;
                          for (z = 0; z < NPART; z++)
                              if (live_tab[a][b][z].n >= 0) {
                                  free(live_tab[a][b][z].s);
                                  live_tab[a][b][z].n = -1; live_tab[a][b][z].s = NULL;
                              }
                      }
            }
            /* freeze this pass's painting as the next pass's preference set, then repaint */
            for (kk = topk; kk >= 1; kk--) {
                if (!L[kk].n) continue;
                Level *ns = level_sub_cited(&L[kk]);
                if (ns) L[kk].sub = ns;
                memset(L[kk].cited, 0, L[kk].n);
            }
            pref_hits = pref_miss = 0;
        }
        return 0;
    }
    printf("%3s %10s %10s %8s %9s %14s %8s\n",
           "k", "facts", "verified", "unver", "sec", "nodes", "live");
    long long tot_ver = 0, tot_un = 0, tot_nodes = 0;
    double tot_s = 0;
    for (k = g_mink; k <= maxk && k <= MAXK; k++) {
        if (!L[k].n) continue;
        { int j; for (j = 0; j < MEMOSZ; j++) memo[j].res = -1; }   /* level set changed */
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
    return tot_un ? 1 : 0;
}
