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
static uint64_t query_budget_seconds = 60;
static long n_query, n_true, n_false, n_maybe;
static double total_ms;

static double now_ms(void) {
    return (double)clock() * 1000.0 / CLOCKS_PER_SEC;
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
        parse_file(argv[i]);
        respond("OK loaded %s", argv[i]);
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
            respond("OK queries=%ld solvable=%ld unsolvable=%ld maybe=%ld total_ms=%.0f",
                    n_query, n_true, n_false, n_maybe, total_ms);
            continue;
        }
        if (!strncmp(line, "budget ", 7)) {
            query_budget_seconds = strtoull(line + 7, NULL, 10);
            respond("OK budget=%llu", (unsigned long long)query_budget_seconds);
            continue;
        }
        if (!strncmp(line, "load ", 5)) {
            parse_file(line + 5);
            respond("OK loaded %s", line + 5);
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
        respond_verdict(r, k, ms, sb, size);
    }
    return 0;
}
