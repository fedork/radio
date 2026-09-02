/*
 * Ask the production dominance cache one question directly:
 * "given these negative facts at level k, is this state refuted at level k?"
 *
 * Written to isolate the 2026-09-02 finding that the frozen refuter reported an uncovered
 * split on the colored Sa(113) bundle whose outcome-1 child `Sb(8:3,7:5,5:2,3:3)` has a
 * literal sub-multiset `Sb(8:3,7:5,3:3)` present in that bundle as a k=4 fact. The refuter's
 * audit path cannot be asked that question in isolation - it bypasses the audited root's own
 * cache entry and returns at its FIRST uncovered split - so this driver loads facts exactly
 * as `load_negative_cache` does and then performs the single CACHE_ONLY query the audit would
 * have performed for that child.
 *
 * Build and run (standalone driver, emits its own provenance):
 *   tools/build_radio.py -O2 -DMAX_K=10 -DMAX_N=194 tools/cache_citation_probe.c -o probe
 *   ./probe 4 'Sb(8:3,7:5,3:3)' 'Sb(8:3,7:5,5:2,3:3)'
 *
 * Verdict 0 = FALSE = refuted (the citation was found), 1 = TRUE, 2 = MAYBE = not refuted.
 * Any number of fact arguments may be given before the final query argument.
 */

#include "../radiobase.c"

#define PROBE_MAX_PARTS 40

/* Parse `Sb(n:m,...)` or a bare `n:m,...` into sbb codes; returns the part count. */
static int parse_state(const char *text, int *parts) {
    const char *p = text;
    int np = 0;
    if (strncmp(p, "Sb(", 3) == 0) p += 3;
    while (*p && *p != ')') {
        long n, m;
        char *end;
        if (np >= PROBE_MAX_PARTS) {
            fprintf(stderr, "more than %d parts in %s\n", PROBE_MAX_PARTS, text);
            exit(2);
        }
        n = strtol(p, &end, 10);
        if (end == p || *end != ':') {
            fprintf(stderr, "malformed part in %s\n", text);
            exit(2);
        }
        p = end + 1;
        m = strtol(p, &end, 10);
        if (end == p || n < 1 || m < 1) {
            fprintf(stderr, "malformed part in %s\n", text);
            exit(2);
        }
        parts[np++] = getSbb((int)n, (int)m);
        p = end;
        if (*p == ',') p++;
    }
    if (np == 0) {
        fprintf(stderr, "empty state %s\n", text);
        exit(2);
    }
    sort1(parts, np);
    return np;
}

static void print_state(const int *parts, int np) {
    printf("Sb(");
    for (int i = 0; i < np; i++) {
        if (i) putchar(',');
        fputs(sbb_to_str[parts[i]], stdout);
    }
    putchar(')');
}

int main(int argc, char **argv) {
    int k;
    int query[PROBE_MAX_PARTS];
    int query_np;
    radio_search_context ctx;
    int verdict;

    if (argc < 4) {
        fprintf(stderr, "usage: %s K 'FACT' ['FACT' ...] 'QUERY'\n", argv[0]);
        return 2;
    }
    k = atoi(argv[1]);
    if (k < 1 || k > MAX_K) {
        fprintf(stderr, "K must be in 1..%d\n", MAX_K);
        return 2;
    }

    init();

    /* Load facts exactly as radio_refute's load_negative_cache does. */
    cache_replay_depth++;
    for (int a = 2; a < argc - 1; a++) {
        int parts[PROBE_MAX_PARTS];
        int np = parse_state(argv[a], parts);
        int pairs = 0;
        for (int i = 0; i < np; i++) pairs += sb_pairs[parts[i]];
        printf("FACT k=%d ", k);
        print_state(parts, np);
        printf(" mass=%d\n", pairs);
        cache(parts, np, FALSE, k, pairs);
    }
    cache_replay_depth--;
    printf("CACHE_LOADED branches=%lld fronts=%lld redundant=%lld\n",
           alloc_count, front_alloc_count, redundant_cache_replays);

    query_np = parse_state(argv[argc - 1], query);
    {
        int pairs = 0;
        for (int i = 0; i < query_np; i++) pairs += sb_pairs[query[i]];
        printf("QUERY k=%d ", k);
        print_state(query, query_np);
        printf(" mass=%d\n", pairs);
    }

    radio_search_context_init(&ctx);
    verdict = canSolveB_ctx(&ctx, query, query_np, k, CACHE_ONLY);
    radio_search_context_destroy(&ctx);

    printf("VERDICT %d (%s)\n", verdict,
           verdict == FALSE ? "FALSE: refuted, citation found"
           : verdict == TRUE ? "TRUE: solvable"
                             : "MAYBE: no citation and no theorem fired");
    return 0;
}
