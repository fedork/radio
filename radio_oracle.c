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
