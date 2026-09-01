/*
 * Frozen negative-certificate refuter.
 *
 * This deliberately reuses the production solver's theorem checks, compact dominance trie,
 * split geometry, ordering and reachability prune, but not its recursive solving policy.  A
 * level-v2 input contains one audit level in load order: a part dictionary, complete k-1
 * support, checked split-part hints, then level-k claims.  Only support enters the trie; claims are
 * compact targets.  Legacy v1 full certificates remain accepted.  The cache and split tables are
 * frozen before workers audit roots independently.  A level-k claim bypasses its own cache entry
 * and exhausts every legal test; each child may be discharged only by a theorem or the immutable
 * k-1 negative trie.  A miss is an audit gap, never an invitation to solve, learn, or write a fact.
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
 * Build with -DRADIO_REFUTE_ENABLE_L1 only for the rejected exact-L1 benchmark control.
 * Build with -DRADIO_REFUTE_ENABLE_COLORING to emit the actually cited k-1 support facts:
 *   ./radio_refute level-k.cert level-(k-1).selection
 */

#include <pthread.h>
#include <stdatomic.h>
#include <errno.h>
#include <math.h>

/* The mutable solver benefits from its worker-local exact front cache, but the k=7 frozen audit
   does not: two matched 9,995-root pairs measured an 11.3% CPU regression.  Keep the solver default
   unchanged and make the refuter's rejected control explicitly selectable at build time. */
#ifndef RADIO_REFUTE_ENABLE_L1
#define RADIO_DISABLE_CACHE_L1
#endif
#ifdef RADIO_REFUTE_ENABLE_COLORING
#define RADIO_CACHE_CITATIONS
#endif
#include "radiobase.c"

#define CERT_HEADER "radio-negative-certificate-v1"
#define LEVEL_CERT_HEADER "radio-negative-level-certificate-v2"
#define COLOR_SELECTION_HEADER "radio-negative-color-selection-v1"
#define MAX_CERT_PARTS 40
#define LINE_BYTES (1u << 16)

typedef struct {
    uint32_t offset;
    uint8_t np;
} Claim;

typedef struct {
    Claim *v;
    size_t n;
    size_t cap;
    uint16_t *parts;
    size_t parts_n;
    size_t parts_cap;
} ClaimLevel;

typedef struct {
    uint16_t part;
    uint32_t uses;
} SplitHint;

typedef struct {
    uint32_t index;
    uint8_t k;
} Task;

static ClaimLevel claims[MAX_K + 1];
static int level_certificate;
static int certificate_level;
static int certificate_support_level;
static SplitHint *certificate_split_hints;
static size_t certificate_split_hints_n;
static pthread_mutex_t output_mu = PTHREAD_MUTEX_INITIALIZER;

_Static_assert(sizeof(Claim) == 8, "compact Claim layout changed");
_Static_assert(MAX_SBB <= UINT16_MAX, "level certificates require 16-bit part encodings");

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

static void reserve_claim_level(int k, size_t records, size_t part_refs) {
    ClaimLevel *L = &claims[k];
    if (records > UINT32_MAX || part_refs > UINT32_MAX) {
        fprintf(stderr, "level-%d certificate section exceeds 32-bit compact offsets\n", k);
        exit(2);
    }
    if (records > L->cap) {
        Claim *grown = (Claim *)realloc(L->v, records * sizeof(*grown));
        if (grown == NULL) {
            fprintf(stderr, "out of memory reserving %zu level-%d claims\n", records, k);
            exit(2);
        }
        L->v = grown;
        L->cap = records;
    }
    if (part_refs > L->parts_cap) {
        uint16_t *grown = (uint16_t *)realloc(L->parts, part_refs * sizeof(*grown));
        if (grown == NULL) {
            fprintf(stderr, "out of memory reserving %zu level-%d part references\n",
                    part_refs, k);
            exit(2);
        }
        L->parts = grown;
        L->parts_cap = part_refs;
    }
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
    if (L->parts_n + (size_t)np > UINT32_MAX) {
        fprintf(stderr, "too many level-%d part references\n", k);
        exit(2);
    }
    if (L->parts_n + (size_t)np > L->parts_cap) {
        size_t need = L->parts_n + (size_t)np;
        size_t next = L->parts_cap ? L->parts_cap * 2 : 4096;
        while (next < need) {
            if (next > SIZE_MAX / 2) {
                fprintf(stderr, "level-%d part-reference capacity overflow\n", k);
                exit(2);
            }
            next *= 2;
        }
        uint16_t *grown = (uint16_t *)realloc(L->parts, next * sizeof(*grown));
        if (grown == NULL) {
            fprintf(stderr, "out of memory growing level %d part references\n", k);
            exit(2);
        }
        L->parts = grown;
        L->parts_cap = next;
    }
    c = &L->v[L->n++];
    c->offset = (uint32_t)L->parts_n;
    c->np = (uint8_t)np;
    for (int i = 0; i < np; i++) L->parts[L->parts_n++] = (uint16_t)parts[i];
}

static const uint16_t *claim_part_data(const ClaimLevel *L, const Claim *c) {
    return L->parts + c->offset;
}

static void free_claim_level(int k) {
    free(claims[k].v);
    free(claims[k].parts);
    memset(&claims[k], 0, sizeof(claims[k]));
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

static char *skip_space(char *p) {
    while (*p == ' ' || *p == '\t') p++;
    return p;
}

static int line_tail(const char *p) {
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    return *p == 0 || *p == '#';
}

static int line_equals(const char *p, const char *word) {
    size_t n = strlen(word);
    return strncmp(p, word, n) == 0 && line_tail(p + n);
}

static char *next_data_line(FILE *fp, const char *path, char *line, int *lineno) {
    while (fgets(line, LINE_BYTES, fp) != NULL) {
        char *p;
        (*lineno)++;
        if (strchr(line, '\n') == NULL && !feof(fp)) {
            fprintf(stderr, "%s:%d: line exceeds %u bytes\n", path, *lineno, LINE_BYTES - 1);
            exit(2);
        }
        p = skip_space(line);
        if (*p == 0 || *p == '\n' || *p == '\r' || *p == '#') continue;
        return p;
    }
    if (ferror(fp)) {
        fprintf(stderr, "error reading certificate %s\n", path);
        exit(2);
    }
    return NULL;
}

static long parse_one_long(const char *path, int lineno, char *p, const char *tag,
                           long lo, long hi) {
    char *end;
    long value;
    size_t n = strlen(tag);
    if (strncmp(p, tag, n) != 0 || (p[n] != ' ' && p[n] != '\t')) {
        fprintf(stderr, "%s:%d: expected %s record\n", path, lineno, tag);
        exit(2);
    }
    p = skip_space(p + n);
    errno = 0;
    value = strtol(p, &end, 10);
    if (errno || end == p || value < lo || value > hi || !line_tail(end)) {
        fprintf(stderr, "%s:%d: invalid %s value\n", path, lineno, tag);
        exit(2);
    }
    return value;
}

static void parse_count_header(const char *path, int lineno, char *p, const char *tag,
                               long level_lo, long level_hi, long *level_out,
                               size_t *records_out, size_t *refs_out) {
    char *end;
    unsigned long long records, refs;
    long level;
    size_t n = strlen(tag);
    if (strncmp(p, tag, n) != 0 || (p[n] != ' ' && p[n] != '\t')) {
        fprintf(stderr, "%s:%d: expected %s section\n", path, lineno, tag);
        exit(2);
    }
    p = skip_space(p + n);
    errno = 0;
    level = strtol(p, &end, 10);
    if (errno || end == p || level < level_lo || level > level_hi) goto bad;
    p = skip_space(end);
    errno = 0;
    records = strtoull(p, &end, 10);
    if (errno || end == p || records > SIZE_MAX) goto bad;
    p = skip_space(end);
    errno = 0;
    refs = strtoull(p, &end, 10);
    if (errno || end == p || refs > SIZE_MAX || !line_tail(end)) goto bad;
    *level_out = level;
    *records_out = (size_t)records;
    *refs_out = (size_t)refs;
    return;
bad:
    fprintf(stderr, "%s:%d: malformed %s section header\n", path, lineno, tag);
    exit(2);
}

static int parse_id_state(const char *path, int lineno, char *p, const char *tag,
                          const uint16_t *part_ids, size_t part_ids_n, int *parts,
                          unsigned char *part_used) {
    size_t n = strlen(tag);
    int np = 0;
    long previous = LONG_MAX;
    if (strncmp(p, tag, n) != 0 || (p[n] != ' ' && p[n] != '\t')) {
        fprintf(stderr, "%s:%d: expected %s record\n", path, lineno, tag);
        exit(2);
    }
    p = skip_space(p + n);
    while (!line_tail(p)) {
        char *end;
        long id;
        if (np >= MAX_CERT_PARTS) {
            fprintf(stderr, "%s:%d: more than %d parts\n", path, lineno, MAX_CERT_PARTS);
            exit(2);
        }
        errno = 0;
        id = strtol(p, &end, 10);
        if (errno || end == p || id < 1 || (size_t)id > part_ids_n) {
            fprintf(stderr, "%s:%d: invalid part id\n", path, lineno);
            exit(2);
        }
        if (id > previous) {
            fprintf(stderr, "%s:%d: part ids are not in canonical descending order\n",
                    path, lineno);
            exit(2);
        }
        previous = id;
        parts[np++] = part_ids[id];
        part_used[id] = 1;
        p = skip_space(end);
    }
    if (np == 0) {
        fprintf(stderr, "%s:%d: empty %s state\n", path, lineno, tag);
        exit(2);
    }
    return np;
}

static size_t read_level_certificate(FILE *fp, const char *path) {
    char line[LINE_BYTES];
    char *p;
    int lineno = 0;
    long level, section_level;
    long parts_count_long;
    size_t parts_count, support_count, support_refs, claim_count, claim_refs;
    size_t seen_refs;
    uint16_t *part_ids;
    unsigned char *part_used;
    unsigned char *sbb_defined;
    uint32_t *root_uses;
    unsigned char *hint_seen;
    int previous_sbb = 0;

    p = next_data_line(fp, path, line, &lineno);
    if (p == NULL || !line_equals(p, LEVEL_CERT_HEADER)) {
        fprintf(stderr, "%s:%d: missing %s header\n", path, lineno, LEVEL_CERT_HEADER);
        exit(2);
    }
    p = next_data_line(fp, path, line, &lineno);
    if (p == NULL) {
        fprintf(stderr, "%s: missing level record\n", path);
        exit(2);
    }
    level = parse_one_long(path, lineno, p, "level", 1, MAX_K);
    certificate_level = (int)level;
    certificate_support_level = (int)level - 1;

    p = next_data_line(fp, path, line, &lineno);
    if (p == NULL) {
        fprintf(stderr, "%s: missing parts section\n", path);
        exit(2);
    }
    parts_count_long = parse_one_long(path, lineno, p, "parts", 1, UINT16_MAX);
    parts_count = (size_t)parts_count_long;
    part_ids = (uint16_t *)calloc(parts_count + 1, sizeof(*part_ids));
    part_used = (unsigned char *)calloc(parts_count + 1, 1);
    sbb_defined = (unsigned char *)calloc((size_t)MAX_SBB + 1, 1);
    root_uses = (uint32_t *)calloc((size_t)MAX_SBB + 1, sizeof(*root_uses));
    hint_seen = (unsigned char *)calloc((size_t)MAX_SBB + 1, 1);
    if (part_ids == NULL || part_used == NULL || sbb_defined == NULL || root_uses == NULL
            || hint_seen == NULL) {
        fprintf(stderr, "out of memory reading level certificate dictionary\n");
        exit(2);
    }
    for (size_t expected = 1; expected <= parts_count; expected++) {
        char *end;
        long id, n1, n2;
        int sbb;
        p = next_data_line(fp, path, line, &lineno);
        if (p == NULL || strncmp(p, "part", 4) != 0 || (p[4] != ' ' && p[4] != '\t')) {
            fprintf(stderr, "%s:%d: expected part %zu\n", path, lineno, expected);
            exit(2);
        }
        p = skip_space(p + 4);
        errno = 0;
        id = strtol(p, &end, 10);
        if (errno || end == p || id != (long)expected) goto bad_part;
        p = skip_space(end);
        errno = 0;
        n1 = strtol(p, &end, 10);
        if (errno || end == p || *end != ':') goto bad_part;
        p = end + 1;
        errno = 0;
        n2 = strtol(p, &end, 10);
        if (errno || end == p || n1 < n2 || n2 < 1 || n1 > MAX_N || n1 + n2 > MAX_N
                || !line_tail(end)) goto bad_part;
        sbb = getSbb((int)n1, (int)n2);
        if (sbb <= previous_sbb || sbb > MAX_SBB || sbb_defined[sbb]) goto bad_part;
        part_ids[expected] = (uint16_t)sbb;
        sbb_defined[sbb] = 1;
        previous_sbb = sbb;
        continue;
bad_part:
        fprintf(stderr, "%s:%d: malformed or duplicate part definition\n", path, lineno);
        exit(2);
    }

    p = next_data_line(fp, path, line, &lineno);
    if (p == NULL) {
        fprintf(stderr, "%s: missing support section\n", path);
        exit(2);
    }
    parse_count_header(path, lineno, p, "support", 0, MAX_K, &section_level,
                       &support_count, &support_refs);
    if (section_level != certificate_support_level || (section_level == 0 && support_count != 0)) {
        fprintf(stderr, "%s:%d: support level must be %d\n",
                path, lineno, certificate_support_level);
        exit(2);
    }
    if (support_count > 0) reserve_claim_level((int)section_level, support_count, support_refs);
    seen_refs = 0;
    for (size_t q = 0; q < support_count; q++) {
        int parts[MAX_CERT_PARTS];
        int np;
        p = next_data_line(fp, path, line, &lineno);
        if (p == NULL) {
            fprintf(stderr, "%s: truncated support section\n", path);
            exit(2);
        }
        np = parse_id_state(path, lineno, p, "fact", part_ids, parts_count, parts, part_used);
        append_claim((int)section_level, parts, np);
        seen_refs += (size_t)np;
    }
    if (seen_refs != support_refs) {
        fprintf(stderr, "%s: support part-reference count %zu != declared %zu\n",
                path, seen_refs, support_refs);
        exit(2);
    }

    p = next_data_line(fp, path, line, &lineno);
    if (p == NULL) {
        fprintf(stderr, "%s: missing split-hints section\n", path);
        exit(2);
    }
    certificate_split_hints_n = (size_t)parse_one_long(
        path, lineno, p, "split-hints", 0, MAX_SBB);
    certificate_split_hints = (SplitHint *)calloc(
        certificate_split_hints_n ? certificate_split_hints_n : 1,
        sizeof(*certificate_split_hints));
    if (certificate_split_hints == NULL) {
        fprintf(stderr, "out of memory reading split hints\n");
        exit(2);
    }
    for (size_t q = 0; q < certificate_split_hints_n; q++) {
        char *end;
        long id;
        unsigned long uses;
        int sbb;
        p = next_data_line(fp, path, line, &lineno);
        if (p == NULL || strncmp(p, "split", 5) != 0 || (p[5] != ' ' && p[5] != '\t')) {
            fprintf(stderr, "%s:%d: expected split hint\n", path, lineno);
            exit(2);
        }
        p = skip_space(p + 5);
        errno = 0;
        id = strtol(p, &end, 10);
        if (errno || end == p || id < 1 || (size_t)id > parts_count) goto bad_hint;
        p = skip_space(end);
        if (strncmp(p, "uses", 4) != 0 || (p[4] != ' ' && p[4] != '\t')) goto bad_hint;
        p = skip_space(p + 4);
        errno = 0;
        uses = strtoul(p, &end, 10);
        if (errno || end == p || uses == 0 || uses > UINT32_MAX || !line_tail(end)) goto bad_hint;
        sbb = part_ids[id];
        if (sbb == 1 || hint_seen[sbb]) goto bad_hint;
        hint_seen[sbb] = 1;
        certificate_split_hints[q].part = (uint16_t)sbb;
        certificate_split_hints[q].uses = (uint32_t)uses;
        continue;
bad_hint:
        fprintf(stderr, "%s:%d: malformed or duplicate split hint\n", path, lineno);
        exit(2);
    }

    p = next_data_line(fp, path, line, &lineno);
    if (p == NULL) {
        fprintf(stderr, "%s: missing claims section\n", path);
        exit(2);
    }
    parse_count_header(path, lineno, p, "claims", 1, MAX_K, &section_level,
                       &claim_count, &claim_refs);
    if (section_level != certificate_level || claim_count == 0) {
        fprintf(stderr, "%s:%d: claims level must be %d and nonempty\n",
                path, lineno, certificate_level);
        exit(2);
    }
    reserve_claim_level(certificate_level, claim_count, claim_refs);
    seen_refs = 0;
    for (size_t q = 0; q < claim_count; q++) {
        int parts[MAX_CERT_PARTS];
        int np;
        p = next_data_line(fp, path, line, &lineno);
        if (p == NULL) {
            fprintf(stderr, "%s: truncated claims section\n", path);
            exit(2);
        }
        np = parse_id_state(path, lineno, p, "claim", part_ids, parts_count, parts, part_used);
        append_claim(certificate_level, parts, np);
        seen_refs += (size_t)np;
        for (int i = 0; i < np; i++) {
            int sbb = parts[i];
            if (sbb > 1) {
                if (root_uses[sbb] == UINT32_MAX) {
                    fprintf(stderr, "%s:%d: root part-use count overflow\n", path, lineno);
                    exit(2);
                }
                root_uses[sbb]++;
            }
        }
    }
    if (seen_refs != claim_refs) {
        fprintf(stderr, "%s: claim part-reference count %zu != declared %zu\n",
                path, seen_refs, claim_refs);
        exit(2);
    }
    p = next_data_line(fp, path, line, &lineno);
    if (p != NULL) {
        fprintf(stderr, "%s:%d: trailing certificate data\n", path, lineno);
        exit(2);
    }

    {
        size_t expected_hints = 0;
        for (int sbb = 2; sbb <= MAX_SBB; sbb++) {
            if (root_uses[sbb]) expected_hints++;
            if ((root_uses[sbb] != 0) != (hint_seen[sbb] != 0)) {
                fprintf(stderr, "%s: split-hint part set does not match level-%d claims\n",
                        path, certificate_level);
                exit(2);
            }
        }
        if (expected_hints != certificate_split_hints_n) {
            fprintf(stderr, "%s: split-hint count %zu != required %zu\n",
                    path, certificate_split_hints_n, expected_hints);
            exit(2);
        }
        for (size_t q = 0; q < certificate_split_hints_n; q++) {
            SplitHint *hint = &certificate_split_hints[q];
            if (hint->uses != root_uses[hint->part]) {
                fprintf(stderr, "%s: split hint for %s says %u uses, observed %u\n",
                        path, sbb_to_str[hint->part], hint->uses, root_uses[hint->part]);
                exit(2);
            }
        }
        for (size_t id = 1; id <= parts_count; id++) {
            if (!part_used[id]) {
                fprintf(stderr, "%s: unused part id %zu\n", path, id);
                exit(2);
            }
        }
    }

    free(hint_seen);
    free(root_uses);
    free(sbb_defined);
    free(part_used);
    free(part_ids);
    level_certificate = 1;
    return support_count + claim_count;
}

static size_t read_v1_certificate(FILE *fp, const char *path) {
    char line[LINE_BYTES];
    size_t total = 0;
    int lineno = 0;
    int header_seen = 0;
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
    if (!header_seen) {
        fprintf(stderr, "%s: missing %s header\n", path, CERT_HEADER);
        exit(2);
    }
    return total;
}

static size_t read_certificate(const char *path) {
    FILE *fp = fopen(path, "r");
    char line[LINE_BYTES];
    char *p;
    int lineno = 0;
    size_t total;
    if (fp == NULL) {
        fprintf(stderr, "cannot open certificate %s\n", path);
        exit(2);
    }
    p = next_data_line(fp, path, line, &lineno);
    if (p == NULL) {
        fprintf(stderr, "%s: empty certificate\n", path);
        exit(2);
    }
    rewind(fp);
    if (line_equals(p, LEVEL_CERT_HEADER)) total = read_level_certificate(fp, path);
    else total = read_v1_certificate(fp, path);
    fclose(fp);
    return total;
}

static void expand_claim(const ClaimLevel *L, const Claim *c, int *parts) {
    const uint16_t *stored = claim_part_data(L, c);
    for (int i = 0; i < c->np; i++) parts[i] = (int)stored[i];
}

static void print_state(FILE *fp, const ClaimLevel *L, const Claim *c) {
    const uint16_t *parts = claim_part_data(L, c);
    fputs("Sb(", fp);
    for (int i = 0; i < c->np; i++) {
        if (i) fputc(',', fp);
        fputs(sbb_to_str[parts[i]], fp);
    }
    fputc(')', fp);
}

static void print_claim(FILE *fp, int k, const Claim *c) {
    fprintf(fp, "k=%d ", k);
    print_state(fp, &claims[k], c);
}

#ifdef RADIO_REFUTE_ENABLE_COLORING
static size_t color_bit_words(size_t facts) {
    return (facts + 63) / 64;
}

static size_t color_count_used(const uint64_t *bits, size_t words) {
    size_t used = 0;
    for (size_t q = 0; q < words; q++) used += (size_t)__builtin_popcountll(bits[q]);
    return used;
}

static int write_color_selection(const char *path, const uint64_t *bits, size_t words,
                                 size_t audited, unsigned long long citation_hits) {
    int support_level = certificate_support_level;
    ClaimLevel *support = support_level > 0 ? &claims[support_level] : NULL;
    size_t support_count = support != NULL ? support->n : 0;
    size_t used = color_count_used(bits, words);
    FILE *fp = fopen(path, "w");
    if (fp == NULL) {
        fprintf(stderr, "cannot create color selection %s\n", path);
        return -1;
    }
    fprintf(fp, "%s\n", COLOR_SELECTION_HEADER);
    fprintf(fp, "parent-level %d\n", certificate_level);
    fprintf(fp, "selected-level %d\n", support_level);
    fprintf(fp, "source-claims %zu\n", claims[certificate_level].n);
    fprintf(fp, "audited %zu\n", audited);
    fprintf(fp, "support %zu\n", support_count);
    fprintf(fp, "used %zu\n", used);
    fprintf(fp, "citation-hits %llu\n", citation_hits);
    for (size_t q = 0; q < support_count; q++) {
        if (!(bits[q >> 6] & (UINT64_C(1) << (q & 63)))) continue;
        fprintf(fp, "use %zu ", q + 1);
        print_state(fp, support, &support->v[q]);
        fputc('\n', fp);
    }
    if (fclose(fp) != 0) {
        fprintf(stderr, "cannot finish color selection %s\n", path);
        return -1;
    }
    printf("COLOR_SELECTION parent_level=%d selected_level=%d support=%zu used=%zu "
           "citation_hits=%llu path=%s\n",
           certificate_level, support_level, support_count, used, citation_hits, path);
    fflush(stdout);
    return 0;
}
#endif

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
            expand_claim(L, &L->v[q], parts);
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

static void load_level_support_cache(void) {
    size_t total = certificate_support_level > 0
        ? claims[certificate_support_level].n : 0;
    size_t done = 0;
    double start = monotonic_seconds();
    cache_replay_depth++;
    printf("CACHE_START claims=%zu support_level=%d\n", total, certificate_support_level);
    fflush(stdout);
    if (certificate_support_level > 0) {
        ClaimLevel *L = &claims[certificate_support_level];
        for (size_t q = 0; q < L->n; q++) {
            int parts[MAX_CERT_PARTS];
            int pairs = 0;
            expand_claim(L, &L->v[q], parts);
            for (int i = 0; i < L->v[q].np; i++) pairs += sb_pairs[parts[i]];
#ifdef RADIO_REFUTE_ENABLE_COLORING
            radio_cache_citation_set_insert_source((uint32_t)q + 1);
#endif
            cache(parts, L->v[q].np, FALSE, certificate_support_level, pairs);
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
#ifdef RADIO_REFUTE_ENABLE_COLORING
    radio_cache_citation_set_insert_source(0);
#endif
    cache_replay_depth--;
    printf("CACHE_DONE claims=%zu support_level=%d wall_s=%.3f branches=%lld "
           "branch_bytes=%lld fronts=%lld front_bytes=%lld redundant=%lld\n",
           total, certificate_support_level, monotonic_seconds() - start, alloc_count,
           branch_alloc_size, front_alloc_count, front_alloc_size, redundant_cache_replays);
    fflush(stdout);
}

/* Prepare exactly the viability information the frozen level-k audit consumes.  Production split
   metadata walks upward through several kk values to learn a minimum-solvability frontier reused
   by repeated root calls at that k.  A self-contained level certificate needs only k-1: if any
   isolated outcome is refuted there, Subgraph Monotonicity kills every completion of this local
   split.  TRUE and MAYBE both leave it in the exhaustive traversal.  Checking all three outcomes
   also finds a negative after an earlier MAYBE instead of inheriting the solver's incremental
   short-circuit. */
#ifdef RADIO_COVER_PILOT
/* Pilot: number of citation-dead options marked at freeze time = the Tier-1 shared
   dead-option dictionary a coverage certificate would carry for this run. */
static size_t pilot_dead_options;
#endif

static size_t freeze_one_table(radio_search_context *ctx, int sbb, int k) {
    splits *sp = ensure_splits(sbb, k);
    for (int c = 0; c < sp->size; c++) {
        int *s = sp->splitsl[c];
        int child_k = k - 1;
        int v0 = canSolveB_ctx(ctx, s, 1, child_k, CACHE_ONLY);
        int v2 = canSolveB_ctx(ctx, s + 3, 1, child_k, CACHE_ONLY);
        int v1 = canSolveB_ctx(ctx, s + 1, 2, child_k, CACHE_ONLY);
        if (v0 == FALSE || v1 == FALSE || v2 == FALSE) {
            s[5] = k;
#ifdef RADIO_COVER_PILOT
            pilot_dead_options++;
#endif
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
#ifdef RADIO_REFUTE_ENABLE_COLORING
    uint64_t *citation_bits;
    size_t citation_facts;
    unsigned long long citation_hits;
#endif
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
#ifdef RADIO_REFUTE_ENABLE_COLORING
    radio_cache_citations_attach(&ctx, a->citation_bits, a->citation_facts);
#endif
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
        expand_claim(&claims[t->k], c, parts);
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
#ifdef RADIO_REFUTE_ENABLE_COLORING
    a->citation_hits = ctx.cache_citation_hits;
#endif
#ifdef RADIO_COVER_PILOT
    radio_cover_pilot_flush();
#endif
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
#ifdef RADIO_REFUTE_ENABLE_COLORING
    const char *color_output;
    size_t citation_facts, citation_words, citation_word_stride;
    uint64_t *prep_citation_bits = NULL;
    uint64_t *worker_citation_bits = NULL;
    uint64_t *merged_citation_bits = NULL;
    unsigned long long prep_citation_hits = 0;
#endif
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

#ifdef RADIO_REFUTE_ENABLE_COLORING
    if (argc != 3) {
        fprintf(stderr, "usage: %s level-certificate.cert color-selection.txt\n", argv[0]);
        return 2;
    }
    color_output = argv[2];
    {
        /* Reserve the destination before doing expensive work.  Refuse to overwrite a prior
           selection, and leave at worst an empty/partial file which the strict parser rejects. */
        FILE *reservation = fopen(color_output, "wx");
        if (reservation == NULL) {
            fprintf(stderr, "cannot reserve new color selection %s: %s\n",
                    color_output, strerror(errno));
            return 2;
        }
        if (fclose(reservation) != 0) {
            fprintf(stderr, "cannot reserve new color selection %s\n", color_output);
            return 2;
        }
    }
#else
    if (argc != 2) {
        fprintf(stderr, "usage: %s certificate.cert\n", argv[0]);
        return 2;
    }
#endif
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
#ifdef RADIO_REFUTE_ENABLE_COLORING
    if (!level_certificate) {
        fprintf(stderr, "coloring requires a self-contained level-v2 certificate\n");
        return 2;
    }
#endif
    printf("INPUT certificate=%s format=%s claims=%zu filters=k%d..%d,np%d..%d,"
           "stride=%ld,offset=%ld threads=%d progress_seconds=%.1f",
           path, level_certificate ? "level-v2" : "full-v1", total_claims,
           min_k, max_k, min_parts, max_parts, stride, offset, threads, progress_seconds);
    if (level_certificate)
        printf(" level=%d support_level=%d split_hints=%zu",
               certificate_level, certificate_support_level, certificate_split_hints_n);
    putchar('\n');
    for (int k = 1; k <= MAX_K; k++)
        if (claims[k].n)
            printf("INPUT_LEVEL k=%d claims=%zu part_refs=%zu storage_bytes=%zu\n",
                   k, claims[k].n, claims[k].parts_n,
                   claims[k].cap * sizeof(*claims[k].v)
                       + claims[k].parts_cap * sizeof(*claims[k].parts));
    fflush(stdout);

    if (level_certificate) load_level_support_cache();
    else load_negative_cache(total_claims);
    frozen_cache_branches = alloc_count;
    frozen_cache_fronts = front_alloc_count;
#ifndef RADIO_REFUTE_ENABLE_COLORING
    if (level_certificate && certificate_support_level > 0)
        free_claim_level(certificate_support_level);
#endif

    for (int k = min_k; k <= max_k; k++) {
        if (level_certificate && k != certificate_level) continue;
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
            if (level_certificate && k != certificate_level) continue;
            size_t eligible = 0;
            for (size_t q = 0; q < claims[k].n; q++) {
                Claim *c = &claims[k].v[q];
                if (c->np < min_parts || c->np > max_parts) continue;
                if ((long)(eligible % (size_t)stride) == offset) {
                    tasks[w].k = (uint8_t)k;
                    tasks[w].index = (uint32_t)q;
                    w++;
                    const uint16_t *stored = claim_part_data(&claims[k], c);
                    for (int i = 0; i < c->np; i++) {
                        if (stored[i] > 1)
                            needed[(size_t)k * needed_stride + stored[i]] = 1;
                    }
                }
                eligible++;
            }
        }
        if (w != selected) {
            fprintf(stderr, "internal selected-task count mismatch\n");
            return 2;
        }
    }

#ifdef RADIO_REFUTE_ENABLE_COLORING
    citation_facts = certificate_support_level > 0
        ? claims[certificate_support_level].n : 0;
    citation_words = color_bit_words(citation_facts);
    citation_word_stride = citation_words ? citation_words : 1;
    prep_citation_bits = (uint64_t *)calloc(citation_word_stride, sizeof(*prep_citation_bits));
    merged_citation_bits = (uint64_t *)calloc(
        citation_word_stride, sizeof(*merged_citation_bits));
    if ((size_t)threads > SIZE_MAX / citation_word_stride) {
        fprintf(stderr, "coloring worker bitset size overflow\n");
        return 2;
    }
    worker_citation_bits = (uint64_t *)calloc(
        (size_t)threads * citation_word_stride, sizeof(*worker_citation_bits));
    if (prep_citation_bits == NULL || merged_citation_bits == NULL
            || worker_citation_bits == NULL) {
        fprintf(stderr, "out of memory allocating color citation bitsets\n");
        return 2;
    }
#endif

    radio_search_context_init(&prep_ctx);
#ifdef RADIO_REFUTE_ENABLE_COLORING
    radio_cache_citations_attach(&prep_ctx, prep_citation_bits, citation_facts);
#endif
    freeze_start = monotonic_seconds();
    printf("FREEZE_START selected=%zu\n", selected);
    fflush(stdout);
    if (level_certificate) {
        for (size_t q = 0; q < certificate_split_hints_n; q++) {
            int sbb = certificate_split_hints[q].part;
            unsigned char *flag = &needed[(size_t)certificate_level * needed_stride + (size_t)sbb];
            if (!*flag) continue;
            options += freeze_one_table(&prep_ctx, sbb, certificate_level);
            tables++;
            *flag = 0;
            if (tables % 100 == 0) {
                printf("FREEZE_PROGRESS tables=%zu options=%zu wall_s=%.3f\n",
                       tables, options, monotonic_seconds() - freeze_start);
                fflush(stdout);
            }
        }
        for (int sbb = 2; sbb <= MAX_SBB; sbb++) {
            if (needed[(size_t)certificate_level * needed_stride + (size_t)sbb]) {
                fprintf(stderr, "selected root part %s has no split hint\n", sbb_to_str[sbb]);
                return 2;
            }
        }
    } else {
        for (int k = min_k; k <= max_k; k++) {
            for (int sbb = 2; sbb <= MAX_SBB; sbb++) {
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
    }
#ifdef RADIO_REFUTE_ENABLE_COLORING
    prep_citation_hits = prep_ctx.cache_citation_hits;
#endif
    radio_search_context_destroy(&prep_ctx);
    free(needed);
    printf("FREEZE_DONE tables=%zu options=%zu wall_s=%.3f\n",
           tables, options, monotonic_seconds() - freeze_start);
#ifdef RADIO_COVER_PILOT
    printf("PILOT_COVER_DEADOPTS tables=%zu options=%zu dead=%zu\n",
           tables, options, pilot_dead_options);
#endif
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
#ifdef RADIO_REFUTE_ENABLE_COLORING
        args[i].citation_bits = worker_citation_bits + (size_t)i * citation_word_stride;
        args[i].citation_facts = citation_facts;
#endif
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
#ifdef RADIO_COVER_PILOT
    radio_cover_pilot_report();
#endif
#ifdef RADIO_REFUTE_ENABLE_COLORING
    if (atomic_load(&b.gaps) == 0) {
        unsigned long long citation_hits = prep_citation_hits;
        memcpy(merged_citation_bits, prep_citation_bits,
               citation_words * sizeof(*merged_citation_bits));
        for (int i = 0; i < threads; i++) {
            uint64_t *worker = worker_citation_bits + (size_t)i * citation_word_stride;
            citation_hits += args[i].citation_hits;
            for (size_t q = 0; q < citation_words; q++)
                merged_citation_bits[q] |= worker[q];
        }
        if (write_color_selection(color_output, merged_citation_bits, citation_words,
                                  selected, citation_hits) != 0)
            return 2;
    } else {
        printf("COLOR_SELECTION skipped=yes reason=gaps path=%s\n", color_output);
        if (unlink(color_output) != 0)
            fprintf(stderr, "cannot remove invalid color selection %s: %s\n",
                    color_output, strerror(errno));
    }
#endif
    fflush(stdout);

    pthread_cond_destroy(&b.progress_cv);
    pthread_mutex_destroy(&b.progress_mu);
    free(ids);
    free(args);
    free(b.progress);
    free(tasks);
#ifdef RADIO_REFUTE_ENABLE_COLORING
    free(merged_citation_bits);
    free(worker_citation_bits);
    free(prep_citation_bits);
#endif
    for (int k = 0; k <= MAX_K; k++) free_claim_level(k);
    free(certificate_split_hints);
    return atomic_load(&b.gaps) == 0 ? 0 : 1;
}
