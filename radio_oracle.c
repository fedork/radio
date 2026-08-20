// A persistent, warm-cache solvability oracle.
//
// Every other driver pays init() and cache replay per process. init() alone costs 205 s at
// -DMAX_N=400 (its static tables scale with MAX_N, and it runs before any argument check), and
// replaying the accumulated caches costs minutes more. That makes one-shot querying useless for
// anything that wants thousands of verdicts -- dataset labelling, split ranking, interactive
// exploration. This driver pays all of it once and then answers queries from stdin until told to
// stop, keeping every fact it learns in the shared cache so later queries get cheaper.
//
//   tools/build_radio.py -O3 -DMAX_K=<k> -DMAX_N=<sum of all sides> radio_oracle.c -o radio_oracle
//   ./radio_oracle [cache.txt ...] < queries > answers
//
// PROTOCOL. One request per line on stdin, one response per line on the response stream. Blank
// lines and lines starting with '#' are ignored.
//
//   <k> <n1> <m1> [<n2> <m2> ...]   ->  VERDICT <SOLVABLE|UNSOLVABLE|MAYBE> k=<k> ms=<n> <state>
//   budget <seconds>                ->  OK budget=<seconds>   (0 means no deadline)
//   load <path>                     ->  OK loaded <path>
//   stats                           ->  OK queries=<n> solvable=<n> unsolvable=<n> maybe=<n> ...
//   quit                            ->  OK bye
//   anything else                   ->  ERR <reason>
//
// STREAM SEPARATION. radiobase.c and canSolveB print progress to stdout, which would corrupt a
// line protocol. After the provenance banner is emitted, this driver keeps a duplicate of the
// original stdout for responses and points the C library's stdout at stderr, so all solver
// chatter lands on stderr and the response stream stays clean. Capture stderr to keep the banner's
// companion log; the banner itself stays on the response stream so retained output still passes
// tools/check_provenance.py.
//
// MAYBE IS NOT A REFUTATION. With a finite budget canSolveB returns MAYBE rather than FALSE. The
// default here is a finite per-query budget, because an unbounded query would hang the daemon; a
// caller that needs a real verdict must set `budget 0` and accept that some states will not return.
//
// MEMORY. The result cache grows without bound and is never freed -- that is the point of a warm
// oracle, but a long-lived process will grow. Run it under tools/capped_run.sh when unattended.

#include <stdarg.h>
#include <unistd.h>

#include "radiobase.c"

static FILE *resp;
static FILE *journal;
static uint64_t query_budget_seconds = 60;
static long n_query, n_true, n_false, n_maybe;
static double total_ms;
static long long n_loaded, n_skipped_wide, n_skipped_k, n_malformed;

static void respond(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

static double now_ms(void) {
    return (double)clock() * 1000.0 / CLOCKS_PER_SEC;
}

/* Replay a cache file, SKIPPING facts this build cannot represent instead of dying on them.
   parse_file() exits on a malformed line and does not bounds-check width at all, so a cache wider
   than MAX_N would corrupt the tables silently.  Skipping keeps a narrow build usable with a wide
   cache, which is the common case: most facts are small. */
static void load_cache(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) { respond("ERR cannot open %s", path); return; }
    long long loaded = 0, wide = 0, badk = 0, bad = 0;
    double t0 = now_ms();
    char buf[1 << 16];
    int continuation = 0;
    cache_replay_depth++;
    while (fgets(buf, sizeof buf, fp)) {
        if (continuation || buf[0] == '#') {
            continuation = strchr(buf, '\n') == NULL;
            continue;
        }
        if (buf[0] != '+' && buf[0] != '-') { if (buf[0] != '\n') bad++; continue; }
        int can = buf[0] == '+';
        char *tok = strtok(buf, " \t\n");
        tok = strtok(NULL, " \t\n");
        if (!tok) { bad++; continue; }
        if (*tok == 'a') {
            char *sn = strtok(NULL, " \t\n"), *sk = strtok(NULL, " \t\n");
            if (!sn || !sk) { bad++; continue; }
            int n = atoi(sn), k = atoi(sk);
            if (k > MAX_K) { badk++; continue; }
            if (n > MAX_N) { wide++; continue; }
            cache_a(can, n, k);
            loaded++;
            continue;
        }
        int sb[64], size = 0; long sides = 0; int ok = 1;
        while (1) {
            char *a = strtok(NULL, " \t\n");
            if (!a) { ok = 0; break; }
            if (*a == 't') break;
            char *b = strtok(NULL, " \t\n");
            if (!b || size >= 64) { ok = 0; break; }
            int n1 = atoi(a), n2 = atoi(b);
            sides += n1 + n2;
            if (sides <= MAX_N) sb[size] = getSbb(n1, n2);
            size++;
        }
        char *sp = ok ? strtok(NULL, " \t\n") : NULL;
        char *sn = sp ? strtok(NULL, " \t\n") : NULL;
        char *sk = sn ? strtok(NULL, " \t\n") : NULL;
        if (!sk || !size) { bad++; continue; }
        if (atoi(sk) > MAX_K) { badk++; continue; }
        if (sides > MAX_N) { wide++; continue; }
        cache(sb, size, can, atoi(sk), atoi(sp));
        loaded++;
    }
    cache_replay_depth--;
    fclose(fp);
    n_loaded += loaded; n_skipped_wide += wide; n_skipped_k += badk; n_malformed += bad;
    double ms = now_ms() - t0;
    respond("OK loaded %s facts=%lld skipped_wide=%lld skipped_k=%lld malformed=%lld ms=%.0f rate=%.0f/s",
            path, loaded, wide, badk, bad, ms, ms > 0 ? loaded * 1000.0 / ms : 0.0);
}

/* Append a computed verdict in the same format load_cache reads, so a session's work becomes the
   next session's primer.  Only real verdicts are journalled; a MAYBE is not a fact. */
static void journal_fact(int r, int k, int *sb, int size) {
    if (!journal || (r != TRUE && r != FALSE)) return;
    int pairs = 0, n = 0;
    fprintf(journal, "%c b", r == TRUE ? '+' : '-');
    for (int i = 0; i < size; i++) {
        int a = sbb_to_n1[sb[i]], b = sbb_to_n2[sb[i]];
        fprintf(journal, " %d %d", a, b);
        pairs += a * b; n += a + b;
    }
    fprintf(journal, " t %d %d %d\n", pairs, n, k);
    fflush(journal);
}

static void respond(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(resp, fmt, ap);
    va_end(ap);
    fputc('\n', resp);
    fflush(resp);
}

// Render the state the same way printSb does, but into the response stream.
static void respond_verdict(int r, int k, double ms, int *sb, int size) {
    fprintf(resp, "VERDICT %s k=%d ms=%.1f Sb(",
            r == TRUE ? "SOLVABLE" : r == FALSE ? "UNSOLVABLE" : "MAYBE", k, ms);
    for (int i = 0; i < size; i++)
        fprintf(resp, "%s%d:%d", i ? "," : "", sbb_to_n1[sb[i]], sbb_to_n2[sb[i]]);
    fprintf(resp, ")\n");
    fflush(resp);
}


/* ---- binary snapshot -------------------------------------------------------------------------
   Replaying facts re-derives the dominance closure the original run already computed, which is why
   it is slow.  A snapshot instead serializes the cache structure itself and reloads it linearly.

   The descriptor space has four forms and all of them have to be walked correctly or the reload is
   silently wrong: a branch (slots 0 and 1 are front descriptors, 2.. are child nodes), a front
   record (two front descriptors), an inline node (two packed sbb values), and, inside a front
   descriptor, either a front vector handle or an inline value.

   Handles are made canonical by discovery order: a fresh process allocates 1,2,3,... so writing the
   structures in visit order and remapping every descriptor reproduces identical handles on load.

   Compatibility is keyed on what actually determines the layout: the source commit plus MAX_K,
   MAX_N, MAX_SBB and sizeof(front_point).  The sbb numbering is a function of the source and MAX_N,
   not of the compiler, so keying on the *build id* was wrong -- it made a snapshot unusable on any
   host with a different compiler, which is exactly what happened to the first 6.67 GB artifact
   (built by Linux clang, refused by Apple clang for no semantic reason).  A semantic mismatch is
   still refused outright; an identity mismatch needs the explicit `restore-any` opt-in. */

#define SNAP_MAGIC "RADIO-CACHE-SNAPSHOT-v"

static uint32_t *snap_bmap, *snap_rmap, *snap_vmap;
static uint32_t *snap_blist, *snap_rlist, *snap_vlist;
static uint32_t snap_bn, snap_rn, snap_vn;

static void snap_visit_front(uint32_t fd) {
    if (!(fd & FRONT_VECTOR_TAG)) return;
    uint32_t h = fd & FRONT_HANDLE_MASK;
    if (!h || h >= front_handles_len || snap_vmap[h]) return;
    snap_vmap[h] = ++snap_vn;
    snap_vlist[snap_vn] = h;
}

static void snap_visit_node(uint32_t nd) {
    uint32_t tag = nd & NODE_TAG_MASK;
    if (tag == NODE_BRANCH_TAG) {
        uint32_t h = nd & NODE_HANDLE_MASK;
        if (!h || h >= branch_handles_len || snap_bmap[h]) return;
        snap_bmap[h] = ++snap_bn;
        snap_blist[snap_bn] = h;
        branch_array *b = branch_handles[h];
        snap_visit_front(b->slot[0]);
        snap_visit_front(b->slot[1]);
        for (uint32_t i = 2; i < b->width; i++)
            if (b->slot[i]) snap_visit_node(b->slot[i]);
    } else if (tag == NODE_RECORD_TAG) {
        uint32_t h = nd & NODE_HANDLE_MASK;
        if (!h || h >= front_records_len || snap_rmap[h]) return;
        snap_rmap[h] = ++snap_rn;
        snap_rlist[snap_rn] = h;
        snap_visit_front(front_records[h].positive);
        snap_visit_front(front_records[h].negative);
    }
}

static uint32_t snap_remap_front(uint32_t fd) {
    if (!(fd & FRONT_VECTOR_TAG)) return fd;
    return FRONT_VECTOR_TAG | snap_vmap[fd & FRONT_HANDLE_MASK];
}

static uint32_t snap_remap_node(uint32_t nd) {
    uint32_t tag = nd & NODE_TAG_MASK;
    if (tag == NODE_BRANCH_TAG) return NODE_BRANCH_TAG | snap_bmap[nd & NODE_HANDLE_MASK];
    if (tag == NODE_RECORD_TAG) return NODE_RECORD_TAG | snap_rmap[nd & NODE_HANDLE_MASK];
    return nd;
}

static void snap_put(FILE *f, uint32_t v) { fwrite(&v, sizeof v, 1, f); }
static uint32_t snap_get(FILE *f) { uint32_t v = 0; if (fread(&v, sizeof v, 1, f) != 1) v = 0; return v; }

static void snapshot_dump(const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) { respond("ERR cannot write %s", path); return; }
    double t0 = now_ms();
    snap_bmap = (uint32_t *)calloc(branch_handles_len + 1, sizeof(uint32_t));
    snap_rmap = (uint32_t *)calloc(front_records_len + 1, sizeof(uint32_t));
    snap_vmap = (uint32_t *)calloc(front_handles_len + 1, sizeof(uint32_t));
    snap_blist = (uint32_t *)calloc(branch_handles_len + 1, sizeof(uint32_t));
    snap_rlist = (uint32_t *)calloc(front_records_len + 1, sizeof(uint32_t));
    snap_vlist = (uint32_t *)calloc(front_handles_len + 1, sizeof(uint32_t));
    if (!snap_bmap || !snap_rmap || !snap_vmap || !snap_blist || !snap_rlist || !snap_vlist) {
        respond("ERR out of memory building snapshot maps"); fclose(f); return;
    }
    snap_bn = snap_rn = snap_vn = 0;
    for (int k = 0; k <= MAX_K; k++) snap_visit_node(sb_cache_root[k]);

    fprintf(f, "%s2\n%s\n%d %d %d %zu\n", SNAP_MAGIC, RADIO_GIT_COMMIT,
            MAX_K, MAX_N, MAX_SBB, sizeof(front_point));
    snap_put(f, snap_bn); snap_put(f, snap_rn); snap_put(f, snap_vn);
    for (uint32_t i = 1; i <= snap_bn; i++) snap_put(f, branch_handles[snap_blist[i]]->width);
    for (uint32_t i = 1; i <= snap_vn; i++) snap_put(f, front_handles[snap_vlist[i]]->len);
    for (uint32_t i = 1; i <= snap_bn; i++) {
        branch_array *b = branch_handles[snap_blist[i]];
        snap_put(f, snap_remap_front(b->slot[0]));
        snap_put(f, snap_remap_front(b->slot[1]));
        for (uint32_t j = 2; j < b->width; j++) snap_put(f, snap_remap_node(b->slot[j]));
    }
    for (uint32_t i = 1; i <= snap_rn; i++) {
        snap_put(f, snap_remap_front(front_records[snap_rlist[i]].positive));
        snap_put(f, snap_remap_front(front_records[snap_rlist[i]].negative));
    }
    for (uint32_t i = 1; i <= snap_vn; i++) {
        front_vector *v = front_handles[snap_vlist[i]];
        fwrite(v->sbb, sizeof(front_point), v->len, f);
    }
    for (int k = 0; k <= MAX_K; k++) snap_put(f, snap_remap_node(sb_cache_root[k]));
    fwrite(sa_can, sizeof(sa_can[0]), MAX_N + 1, f);
    fwrite(sa_cant, sizeof(sa_cant[0]), MAX_N + 1, f);
    long bytes = ftell(f);
    fclose(f);
    free(snap_bmap); free(snap_rmap); free(snap_vmap);
    free(snap_blist); free(snap_rlist); free(snap_vlist);
    snap_bmap = snap_rmap = snap_vmap = snap_blist = snap_rlist = snap_vlist = NULL;
    respond("OK snapshot %s branches=%u records=%u vectors=%u bytes=%ld ms=%.0f",
            path, snap_bn, snap_rn, snap_vn, bytes, now_ms() - t0);
}

/* Allocate a front vector of an exact capacity, mirroring alloc_front_vector's handle discipline
   so that a fresh process hands out 1,2,3,... in call order. */
static uint32_t snap_alloc_vector(uint32_t cap) {
    if (front_handles_len >= front_handles_cap) {
        uint32_t newcap = front_handles_cap ? front_handles_cap * 2 : 1024;
        front_handles = (front_vector **)grow_handle_table(
            front_handles, front_handles_cap, newcap, sizeof(*front_handles), "front");
        front_handles_cap = newcap;
    }
    uint32_t handle = front_handles_len++;
    front_vector *v = (front_vector *)malloc(front_vector_bytes(cap));
    if (!v) { printf("\nout of memory loading snapshot front\n"); exit(1); }
    v->len = 0; v->cap = cap;
    front_handles[handle] = v;
    front_alloc_count++;
    front_alloc_size += cap;
    return FRONT_VECTOR_TAG | handle;
}

static void snapshot_load(const char *path, int allow_foreign) {
    if (branch_handles_len > 1 || front_records_len > 1 || front_handles_len > 1) {
        respond("ERR snapshot must be loaded into a fresh oracle, before any facts or queries");
        return;
    }
    FILE *f = fopen(path, "rb");
    if (!f) { respond("ERR cannot open %s", path); return; }
    double t0 = now_ms();
    char magic[64] = {0}, build[128] = {0};
    int mk = 0, mn = 0, ms_ = 0; size_t fp = 0;
    if (!fgets(magic, sizeof magic, f) || strncmp(magic, SNAP_MAGIC, strlen(SNAP_MAGIC))) {
        respond("ERR %s is not a cache snapshot", path); fclose(f); return;
    }
    if (!fgets(build, sizeof build, f)) { respond("ERR truncated snapshot"); fclose(f); return; }
    build[strcspn(build, "\n")] = 0;
    if (fscanf(f, "%d %d %d %zu\n", &mk, &mn, &ms_, &fp) != 4) {
        respond("ERR truncated snapshot header"); fclose(f); return;
    }
    /* Semantics are non-negotiable: a mismatch here means a different sbb numbering. */
    if (mk != MAX_K || mn != MAX_N || ms_ != MAX_SBB || fp != sizeof(front_point)) {
        respond("ERR snapshot geometry k=%d n=%d sbb=%d front=%zu does not match this build "
                "(k=%d n=%d sbb=%d front=%zu); refusing", mk, mn, ms_, fp,
                MAX_K, MAX_N, MAX_SBB, sizeof(front_point));
        fclose(f); return;
    }
    int same = !strcmp(build, RADIO_GIT_COMMIT) || !strcmp(build, RADIO_BUILD_ID);
    if (!same && !allow_foreign) {
        respond("ERR snapshot identity %s matches neither this commit (%s) nor this build id; "
                "geometry does match, so use `restore-any` if you accept it", build,
                RADIO_GIT_COMMIT);
        fclose(f); return;
    }
    if (!same) respond("WARN restoring foreign snapshot identity %s with matching geometry", build);
    uint32_t bn = snap_get(f), rn = snap_get(f), vn = snap_get(f);
    uint32_t *widths = (uint32_t *)calloc(bn + 1, sizeof(uint32_t));
    uint32_t *vlens = (uint32_t *)calloc(vn + 1, sizeof(uint32_t));
    if ((bn && !widths) || (vn && !vlens)) { respond("ERR out of memory"); fclose(f); return; }
    for (uint32_t i = 1; i <= bn; i++) widths[i] = snap_get(f);
    for (uint32_t i = 1; i <= vn; i++) vlens[i] = snap_get(f);
    for (uint32_t i = 1; i <= bn; i++) alloc_branch(widths[i]);
    for (uint32_t i = 1; i <= rn; i++) alloc_front_record(0, 0);
    for (uint32_t i = 1; i <= vn; i++) snap_alloc_vector(vlens[i] ? vlens[i] : 1);
    for (uint32_t i = 1; i <= bn; i++) {
        branch_array *b = branch_handles[i];
        for (uint32_t j = 0; j < b->width; j++) b->slot[j] = snap_get(f);
    }
    for (uint32_t i = 1; i <= rn; i++) {
        front_records[i].positive = snap_get(f);
        front_records[i].negative = snap_get(f);
    }
    for (uint32_t i = 1; i <= vn; i++) {
        front_vector *v = front_handles[i];
        v->len = vlens[i];
        if (v->len && fread(v->sbb, sizeof(front_point), v->len, f) != v->len) {
            respond("ERR truncated snapshot fronts"); fclose(f); return;
        }
    }
    for (int k = 0; k <= MAX_K; k++) sb_cache_root[k] = snap_get(f);
    if (fread(sa_can, sizeof(sa_can[0]), MAX_N + 1, f) != (size_t)(MAX_N + 1) ||
        fread(sa_cant, sizeof(sa_cant[0]), MAX_N + 1, f) != (size_t)(MAX_N + 1)) {
        respond("ERR truncated snapshot Sa tables"); fclose(f); return;
    }
    fclose(f); free(widths); free(vlens);
    respond("OK restored %s branches=%u records=%u vectors=%u ms=%.0f",
            path, bn, rn, vn, now_ms() - t0);
}

int main(int argc, char **argv) {
    init();

    // Keep the real stdout for responses; send every later solver print to stderr.
    fflush(stdout);
    int saved = dup(1);
    if (saved < 0) { perror("dup"); return 20; }
    resp = fdopen(saved, "w");
    if (!resp) { perror("fdopen"); return 20; }
    if (dup2(2, 1) < 0) { perror("dup2"); return 20; }

    for (int i = 1; i < argc; i++) {
        if (!strncmp(argv[i], "--restore=", 10)) { snapshot_load(argv[i] + 10, 0); continue; }
        if (!strncmp(argv[i], "--restore-any=", 14)) { snapshot_load(argv[i] + 14, 1); continue; }
        if (!strncmp(argv[i], "--journal=", 10)) {
            journal = fopen(argv[i] + 10, "a");
            if (!journal) { respond("ERR cannot open journal %s", argv[i] + 10); return 21; }
            respond("OK journal %s", argv[i] + 10);
            continue;
        }
        load_cache(argv[i]);
    }
    respond("ORACLE READY max_k=%d max_n=%d budget=%llu",
            MAX_K, MAX_N, (unsigned long long)query_budget_seconds);

    char line[8192];
    while (fgets(line, sizeof line, stdin)) {
        char *nl = strchr(line, '\n');
        if (nl) *nl = 0;
        if (line[0] == 0 || line[0] == '#') continue;

        if (!strncmp(line, "quit", 4)) { respond("OK bye"); break; }

        if (!strncmp(line, "stats", 5)) {
            respond("OK queries=%ld solvable=%ld unsolvable=%ld maybe=%ld total_ms=%.0f "
                    "loaded=%lld skipped_wide=%lld skipped_k=%lld malformed=%lld redundant=%lld",
                    n_query, n_true, n_false, n_maybe, total_ms,
                    n_loaded, n_skipped_wide, n_skipped_k, n_malformed,
                    redundant_cache_replays);
            continue;
        }
        if (!strncmp(line, "budget ", 7)) {
            query_budget_seconds = strtoull(line + 7, NULL, 10);
            respond("OK budget=%llu", (unsigned long long)query_budget_seconds);
            continue;
        }
        if (!strncmp(line, "load ", 5)) { load_cache(line + 5); continue; }
        if (!strncmp(line, "snapshot ", 9)) { snapshot_dump(line + 9); continue; }
        if (!strncmp(line, "restore ", 8)) { snapshot_load(line + 8, 0); continue; }
        if (!strncmp(line, "restore-any ", 12)) { snapshot_load(line + 12, 1); continue; }
        if (!strncmp(line, "journal ", 8)) {
            if (journal) fclose(journal);
            journal = fopen(line + 8, "a");
            respond(journal ? "OK journal %s" : "ERR cannot open journal %s", line + 8);
            continue;
        }

        // <k> <n1> <m1> [<n2> <m2> ...]
        int vals[512], nv = 0;
        char *tok = strtok(line, " \t");
        int bad = 0;
        while (tok && nv < 512) {
            char *end;
            long v = strtol(tok, &end, 10);
            if (*end || v < 0) { bad = 1; break; }
            vals[nv++] = (int)v;
            tok = strtok(NULL, " \t");
        }
        if (bad || nv < 3 || (nv - 1) % 2 != 0) { respond("ERR bad request"); continue; }

        int k = vals[0], size = (nv - 1) / 2;
        if (k < 1 || k > MAX_K) { respond("ERR k out of range 1..%d", MAX_K); continue; }
        long sides = 0;
        for (int i = 0; i < size; i++) sides += vals[1 + 2 * i] + vals[2 + 2 * i];
        if (sides > MAX_N) { respond("ERR side sum %ld exceeds MAX_N=%d", sides, MAX_N); continue; }

        int sb[512];
        for (int i = 0; i < size; i++)
            sb[i] = getSbb(vals[1 + 2 * i], vals[2 + 2 * i]);

        double t0 = now_ms();
        uint64_t deadline = query_budget_seconds
                                ? radio_budget_after_seconds(query_budget_seconds)
                                : NO_DEADLINE;
        int r = canSolveB(sb, size, k, deadline);
        double ms = now_ms() - t0;

        n_query++;
        total_ms += ms;
        if (r == TRUE) n_true++; else if (r == FALSE) n_false++; else n_maybe++;
        journal_fact(r, k, sb, size);
        respond_verdict(r, k, ms, sb, size);
    }
    return 0;
}
