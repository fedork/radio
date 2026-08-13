// Probe a recursive lift of a lower-level solving split.
//
// For every parent component P=(N,M), lower component T=(n,m), and lower cut s=(a,b),
// a lineage-preserving lift X is constrained by
//
//                 s <= X <= s + (P-T).
//
// Thus all three children of X contain the corresponding child of s.  This does not prove
// that the enlarged children are solvable one level higher, but it gives a small, structured
// upgrade problem.  The probe starts at the coordinatewise proportional lift of s and visits
// increasing L1 shells.  Candidates are ranked by how closely their three outcome masses
// preserve the lower split's proportions.  Every non-cached child search has a strict deadline.
//
// Build and run:
//
//   tools/build_radio.py -O3 -DMAX_K=7 -DMAX_N=192 \
//       tools/pareto_lift_probe.c -o pareto_lift_probe
//   ./pareto_lift_probe CACHE k recursive R budget probe_ms \
//       N1 M1 n1 m1 a1 b1 [N2 M2 n2 m2 a2 b2 ...]
//
// Here the parent is to be solved in k, the lower template is known in k-1, and (a,b) is a
// solving cut of that lower template.  Exit 0 means a solving lifted split was found, 1 means
// the bounded shells were exhausted, and 3 means invalid or unverified input.
//
// A diagnostic inverse form asks whether a known parent cut has any lower solving split whose
// lift box contains it:
//
//   ./pareto_lift_probe CACHE k inverse budget probe_ms \
//       N1 M1 n1 m1 X1n X1m [...]

#include "../radiobase.c"

#include <stdint.h>

#define LIFT_MAX_PARTS 16

typedef struct {
    uint16_t x[LIFT_MAX_PARTS * 2];
    int p[3];
    int scaled_gap;
} LiftCandidate;

static int lift_parts;
static LiftCandidate *candidates;
static size_t candidates_len, candidates_cap;
static unsigned long long shell_total, shell_information, shell_cache_open;
static int outcome_target[3];

static void print_cut(const LiftCandidate *candidate) {
    printf("[");
    for (int i = 0; i < lift_parts; i++) {
        if (i) printf(",");
        printf("%u:%u", candidate->x[2 * i], candidate->x[2 * i + 1]);
    }
    printf("]");
}

static void make_children(const int *pn, const int *pm, const LiftCandidate *candidate,
                          int *sb0, int *sb1, int *sb2) {
    for (int i = 0; i < lift_parts; i++) {
        int a = candidate->x[2 * i], b = candidate->x[2 * i + 1];
        sb0[i] = getSbb(a, b);
        sb1[2 * i] = getSbb(a, pm[i] - b);
        sb1[2 * i + 1] = getSbb(pn[i] - a, b);
        sb2[i] = getSbb(pn[i] - a, pm[i] - b);
    }
}

static void measure(const int *pn, const int *pm, LiftCandidate *candidate) {
    int p0 = 0, p1 = 0, p2 = 0;
    for (int i = 0; i < lift_parts; i++) {
        int a = candidate->x[2 * i], b = candidate->x[2 * i + 1];
        p0 += a * b;
        p1 += a * (pm[i] - b) + (pn[i] - a) * b;
        p2 += (pn[i] - a) * (pm[i] - b);
    }
    candidate->p[0] = p0;
    candidate->p[1] = p1;
    candidate->p[2] = p2;
    candidate->scaled_gap = abs(p0 - outcome_target[0]) +
                            abs(p1 - outcome_target[1]) +
                            abs(p2 - outcome_target[2]);
}

static int cmp_candidate(const void *va, const void *vb) {
    const LiftCandidate *a = va, *b = vb;
    if (a->scaled_gap != b->scaled_gap) return a->scaled_gap < b->scaled_gap ? -1 : 1;
    int apure = abs(a->p[0] - a->p[2]);
    int bpure = abs(b->p[0] - b->p[2]);
    if (apure != bpure) return apure < bpure ? -1 : 1;
    for (int i = 0; i < lift_parts * 2; i++) {
        if (a->x[i] != b->x[i]) return a->x[i] < b->x[i] ? -1 : 1;
    }
    return 0;
}

static int timed_true(int *sb, int size, int k, int probe_ms) {
    int result = canSolveB(sb, size, k, CACHE_ONLY);
    if (result != MAYBE) return result == TRUE;
    return canSolveB(sb, size, k, radio_budget_after_milliseconds((uint64_t)probe_ms)) == TRUE;
}

static void append_candidate(const LiftCandidate *candidate) {
    if (candidates_len == candidates_cap) {
        size_t next_cap = candidates_cap == 0 ? 4096 : candidates_cap * 2;
        LiftCandidate *next = realloc(candidates, next_cap * sizeof(*next));
        if (next == NULL) {
            printf("out of memory growing lift candidates\n");
            exit(3);
        }
        candidates = next;
        candidates_cap = next_cap;
    }
    candidates[candidates_len++] = *candidate;
}

static void visit_leaf(const int *pn, const int *pm, int child_k, int cap,
                       LiftCandidate *candidate) {
    shell_total++;
    measure(pn, pm, candidate);
    if (candidate->p[0] > cap || candidate->p[1] > cap || candidate->p[2] > cap) return;
    shell_information++;

    int sb0[LIFT_MAX_PARTS], sb1[LIFT_MAX_PARTS * 2], sb2[LIFT_MAX_PARTS];
    make_children(pn, pm, candidate, sb0, sb1, sb2);
    int r0 = canSolveB(sb0, lift_parts, child_k, CACHE_ONLY);
    int r1 = canSolveB(sb1, lift_parts * 2, child_k, CACHE_ONLY);
    int r2 = canSolveB(sb2, lift_parts, child_k, CACHE_ONLY);
    if (r0 == FALSE || r1 == FALSE || r2 == FALSE) return;
    shell_cache_open++;
    append_candidate(candidate);
}

static void enumerate_shell(const int *pn, const int *pm, const int *lo, const int *hi,
                            const int *center, int child_k, int cap, int d, int remaining,
                            LiftCandidate *candidate) {
    if (d == lift_parts * 2) {
        if (remaining == 0) visit_leaf(pn, pm, child_k, cap, candidate);
        return;
    }
    int low = max(lo[d], center[d] - remaining);
    int high = min(hi[d], center[d] + remaining);
    for (int x = low; x <= high; x++) {
        int cost = abs(x - center[d]);
        if (cost > remaining) continue;
        candidate->x[d] = (uint16_t)x;
        enumerate_shell(pn, pm, lo, hi, center, child_k, cap, d + 1,
                        remaining - cost, candidate);
    }
}

static int run_shells(int k, int max_radius, int explicit_budget, int probe_ms,
                      const int *pn, const int *pm, const int *lo, const int *hi,
                      const int *center, clock_t query_start) {
    int child_k = k - 1, cap = power3[child_k];
    unsigned long long total_points = 0;
    int total_attempted = 0;
    printf("LIFT recursive k=%d child_k=%d parts=%d cap=%d max_radius=%d budget=%d "
           "probe_ms=%d target=%d/%d/%d center=[",
           k, child_k, lift_parts, cap, max_radius, explicit_budget, probe_ms,
           outcome_target[0], outcome_target[1], outcome_target[2]);
    for (int i = 0; i < lift_parts; i++) {
        if (i) printf(",");
        printf("%d:%d", center[2 * i], center[2 * i + 1]);
    }
    printf("]\n");

    for (int radius = 0; radius <= max_radius; radius++) {
        candidates_len = 0;
        shell_total = shell_information = shell_cache_open = 0;
        LiftCandidate candidate;
        memset(&candidate, 0, sizeof(candidate));
        enumerate_shell(pn, pm, lo, hi, center, child_k, cap, 0, radius, &candidate);
        total_points += shell_total;
        if (candidates_len > 1)
            qsort(candidates, candidates_len, sizeof(*candidates), cmp_candidate);

        int attempted = 0;
        for (size_t ci = 0; ci < candidates_len && attempted < explicit_budget; ci++) {
            attempted++;
            total_attempted++;
            int sb0[LIFT_MAX_PARTS], sb1[LIFT_MAX_PARTS * 2], sb2[LIFT_MAX_PARTS];
            LiftCandidate *current = &candidates[ci];
            make_children(pn, pm, current, sb0, sb1, sb2);
            if (timed_true(sb0, lift_parts, child_k, probe_ms) &&
                timed_true(sb2, lift_parts, child_k, probe_ms) &&
                timed_true(sb1, lift_parts * 2, child_k, probe_ms)) {
                double seconds = (double)(clock() - query_start) / CLOCKS_PER_SEC;
                printf("LIFT solution radius=%d rank=%zu attempted=%d total_attempted=%d "
                       "lift_points=%llu cpu=%.3f masses=%d/%d/%d cut=",
                       radius, ci + 1, attempted, total_attempted, total_points, seconds,
                       current->p[0], current->p[1], current->p[2]);
                print_cut(current);
                printf("\n");

                int parent[LIFT_MAX_PARTS];
                for (int i = 0; i < lift_parts; i++) parent[i] = getSbb(pn[i], pm[i]);
                printf("can solve ");
                printSb(parent, lift_parts);
                printf(" in %d with ", k);
                print_cut(current);
                printf(" ");
                printSb(sb0, lift_parts);
                printSb(sb1, lift_parts * 2);
                printSb(sb2, lift_parts);
                printf(" took %.3f totalsplits=%llu pass=0 fast_solve=0 "
                       "probe=recursive-pareto-lift\n", seconds, total_points);
                free(candidates);
                return 0;
            }
        }
        printf("LIFT shell radius=%d total=%llu information=%llu cache_open=%llu "
               "attempted=%d\n", radius, shell_total, shell_information,
               shell_cache_open, attempted);
        fflush(stdout);
    }
    printf("LIFT no-solution\n");
    free(candidates);
    return 1;
}

static int rounded_ratio(int value, int numerator, int denominator) {
    // A zero-sized lower coordinate is a degeneration placeholder.  Its lift box is the
    // whole parent coordinate, so use the midpoint rather than pretending that a ratio exists.
    if (denominator == 0) return value / 2;
    return (int)(((long long)value * numerator + denominator / 2) / denominator);
}

static void apportion_targets(long long target_mass, long long source_mass,
                              const long long source_outcomes[3]) {
    long long remainder[3];
    int assigned = 0;
    for (int j = 0; j < 3; j++) {
        long long scaled = source_outcomes[j] * target_mass;
        outcome_target[j] = (int)(scaled / source_mass);
        remainder[j] = scaled % source_mass;
        assigned += outcome_target[j];
    }
    while (assigned < target_mass) {
        int best = 0;
        for (int j = 1; j < 3; j++) if (remainder[j] > remainder[best]) best = j;
        outcome_target[best]++;
        remainder[best] = -1;
        assigned++;
    }
}

static int run_recursive(int argc, char **argv, int k) {
    if (argc < 13 || (argc - 7) % 6 != 0) {
        printf("usage: %s CACHE k recursive R budget probe_ms "
               "N1 M1 n1 m1 a1 b1 [...]\n", argv[0]);
        return 3;
    }
    int max_radius = atoi(argv[4]);
    int explicit_budget = atoi(argv[5]);
    int probe_ms = atoi(argv[6]);
    lift_parts = (argc - 7) / 6;
    if (k < 2 || k > MAX_K || max_radius < 0 || explicit_budget < 0 || probe_ms < 1 ||
        lift_parts < 1 || lift_parts > LIFT_MAX_PARTS) {
        printf("invalid k, radius, budget, probe time, or part count\n");
        return 3;
    }
    clock_t query_start = clock();

    int pn[LIFT_MAX_PARTS], pm[LIFT_MAX_PARTS];
    int ln[LIFT_MAX_PARTS], lm[LIFT_MAX_PARTS];
    int lo[LIFT_MAX_PARTS * 2], hi[LIFT_MAX_PARTS * 2];
    int center[LIFT_MAX_PARTS * 2];
    LiftCandidate lower_cut;
    memset(&lower_cut, 0, sizeof(lower_cut));
    long long parent_mass = 0, lower_mass = 0;
    long long parent_coins = 0, lower_coins = 0;
    for (int i = 0; i < lift_parts; i++) {
        int offset = 7 + 6 * i;
        pn[i] = atoi(argv[offset]);
        pm[i] = atoi(argv[offset + 1]);
        ln[i] = atoi(argv[offset + 2]);
        lm[i] = atoi(argv[offset + 3]);
        int a = atoi(argv[offset + 4]);
        int b = atoi(argv[offset + 5]);
        if (ln[i] < 0 || lm[i] < 0 || pn[i] < ln[i] || pm[i] < lm[i] ||
            (long long)pn[i] + pm[i] > MAX_N ||
            a < 0 || a > ln[i] || b < 0 || b > lm[i]) {
            printf("invalid parent, lower template, or lower cut at part %d\n", i);
            return 3;
        }
        lower_cut.x[2 * i] = (uint16_t)a;
        lower_cut.x[2 * i + 1] = (uint16_t)b;
        lo[2 * i] = a;
        lo[2 * i + 1] = b;
        hi[2 * i] = a + pn[i] - ln[i];
        hi[2 * i + 1] = b + pm[i] - lm[i];
        center[2 * i] = rounded_ratio(pn[i], a, ln[i]);
        center[2 * i + 1] = rounded_ratio(pm[i], b, lm[i]);
        parent_mass += (long long)pn[i] * pm[i];
        lower_mass += (long long)ln[i] * lm[i];
        parent_coins += (long long)pn[i] + pm[i];
        lower_coins += (long long)ln[i] + lm[i];
    }
    if (lower_mass <= 0 || parent_mass > INT_MAX ||
        parent_coins > MAX_N || lower_coins > MAX_N) {
        printf("invalid lower or parent mass/coin count for this build\n");
        return 3;
    }

    long long lower_outcomes[3] = {0, 0, 0};
    for (int i = 0; i < lift_parts; i++) {
        int a = lower_cut.x[2 * i], b = lower_cut.x[2 * i + 1];
        lower_outcomes[0] += (long long)a * b;
        lower_outcomes[1] += (long long)a * (lm[i] - b) +
                             (long long)(ln[i] - a) * b;
        lower_outcomes[2] += (long long)(ln[i] - a) * (lm[i] - b);
    }
    apportion_targets(parent_mass, lower_mass, lower_outcomes);

    int lower0[LIFT_MAX_PARTS], lower1[LIFT_MAX_PARTS * 2], lower2[LIFT_MAX_PARTS];
    make_children(ln, lm, &lower_cut, lower0, lower1, lower2);
    int lower_child_k = k - 2;
    if (!timed_true(lower0, lift_parts, lower_child_k, probe_ms) ||
        !timed_true(lower2, lift_parts, lower_child_k, probe_ms) ||
        !timed_true(lower1, lift_parts * 2, lower_child_k, probe_ms)) {
        printf("LIFT lower split is not proved within %d ms per child at k=%d\n",
               probe_ms, lower_child_k);
        return 3;
    }

    printf("LIFT lower_mass=%lld parent_mass=%lld lower_outcomes=%lld/%lld/%lld "
           "lift_box=[", lower_mass, parent_mass, lower_outcomes[0],
           lower_outcomes[1], lower_outcomes[2]);
    for (int i = 0; i < lift_parts; i++) {
        if (i) printf(",");
        printf("%d:%d..%d:%d", lo[2 * i], lo[2 * i + 1],
               hi[2 * i], hi[2 * i + 1]);
    }
    printf("]\n");
    return run_shells(k, max_radius, explicit_budget, probe_ms,
                      pn, pm, lo, hi, center, query_start);
}

static void visit_inverse_leaf(const int *ln, const int *lm, int lower_child_k, int cap,
                               LiftCandidate *candidate) {
    shell_total++;
    measure(ln, lm, candidate);
    if (candidate->p[0] > cap || candidate->p[1] > cap || candidate->p[2] > cap) return;
    shell_information++;
    int sb0[LIFT_MAX_PARTS], sb1[LIFT_MAX_PARTS * 2], sb2[LIFT_MAX_PARTS];
    make_children(ln, lm, candidate, sb0, sb1, sb2);
    int r0 = canSolveB(sb0, lift_parts, lower_child_k, CACHE_ONLY);
    int r1 = canSolveB(sb1, lift_parts * 2, lower_child_k, CACHE_ONLY);
    int r2 = canSolveB(sb2, lift_parts, lower_child_k, CACHE_ONLY);
    if (r0 == FALSE || r1 == FALSE || r2 == FALSE) return;
    shell_cache_open++;
    append_candidate(candidate);
}

static void enumerate_inverse(const int *ln, const int *lm, const int *lo, const int *hi,
                              int lower_child_k, int cap, int d,
                              LiftCandidate *candidate) {
    if (d == lift_parts * 2) {
        visit_inverse_leaf(ln, lm, lower_child_k, cap, candidate);
        return;
    }
    for (int x = lo[d]; x <= hi[d]; x++) {
        candidate->x[d] = (uint16_t)x;
        enumerate_inverse(ln, lm, lo, hi, lower_child_k, cap, d + 1, candidate);
    }
}

static int run_inverse(int argc, char **argv, int k) {
    if (argc < 12 || (argc - 6) % 6 != 0) {
        printf("usage: %s CACHE k inverse budget probe_ms "
               "N1 M1 n1 m1 X1n X1m [...]\n", argv[0]);
        return 3;
    }
    int explicit_budget = atoi(argv[4]);
    int probe_ms = atoi(argv[5]);
    lift_parts = (argc - 6) / 6;
    if (k < 2 || k > MAX_K || explicit_budget < 0 || probe_ms < 1 ||
        lift_parts < 1 || lift_parts > LIFT_MAX_PARTS) {
        printf("invalid k, budget, probe time, or part count\n");
        return 3;
    }

    int pn[LIFT_MAX_PARTS], pm[LIFT_MAX_PARTS];
    int ln[LIFT_MAX_PARTS], lm[LIFT_MAX_PARTS];
    int lo[LIFT_MAX_PARTS * 2], hi[LIFT_MAX_PARTS * 2];
    LiftCandidate high_cut;
    memset(&high_cut, 0, sizeof(high_cut));
    long long parent_mass = 0, lower_mass = 0;
    long long parent_coins = 0, lower_coins = 0;
    for (int i = 0; i < lift_parts; i++) {
        int offset = 6 + 6 * i;
        pn[i] = atoi(argv[offset]);
        pm[i] = atoi(argv[offset + 1]);
        ln[i] = atoi(argv[offset + 2]);
        lm[i] = atoi(argv[offset + 3]);
        int x = atoi(argv[offset + 4]);
        int y = atoi(argv[offset + 5]);
        if (ln[i] < 0 || lm[i] < 0 || pn[i] < ln[i] || pm[i] < lm[i] ||
            (long long)pn[i] + pm[i] > MAX_N ||
            x < 0 || x > pn[i] || y < 0 || y > pm[i]) {
            printf("invalid parent, lower template, or parent cut at part %d\n", i);
            return 3;
        }
        high_cut.x[2 * i] = (uint16_t)x;
        high_cut.x[2 * i + 1] = (uint16_t)y;
        lo[2 * i] = max(0, x - (pn[i] - ln[i]));
        lo[2 * i + 1] = max(0, y - (pm[i] - lm[i]));
        hi[2 * i] = min(x, ln[i]);
        hi[2 * i + 1] = min(y, lm[i]);
        parent_mass += (long long)pn[i] * pm[i];
        lower_mass += (long long)ln[i] * lm[i];
        parent_coins += (long long)pn[i] + pm[i];
        lower_coins += (long long)ln[i] + lm[i];
    }
    if (lower_mass <= 0 || parent_mass > INT_MAX ||
        parent_coins > MAX_N || lower_coins > MAX_N) {
        printf("invalid lower or parent mass/coin count for this build\n");
        return 3;
    }

    // measure() also computes a ranking gap.  The inverse ranking target is assigned below;
    // only the three masses are used from this call.
    outcome_target[0] = outcome_target[1] = outcome_target[2] = 0;
    measure(pn, pm, &high_cut);
    int high0[LIFT_MAX_PARTS], high1[LIFT_MAX_PARTS * 2], high2[LIFT_MAX_PARTS];
    make_children(pn, pm, &high_cut, high0, high1, high2);
    if (!timed_true(high0, lift_parts, k - 1, probe_ms) ||
        !timed_true(high2, lift_parts, k - 1, probe_ms) ||
        !timed_true(high1, lift_parts * 2, k - 1, probe_ms)) {
        printf("LIFT parent cut is not proved within %d ms per child at k=%d\n",
               probe_ms, k - 1);
        return 3;
    }
    long long high_outcomes[3] = {high_cut.p[0], high_cut.p[1], high_cut.p[2]};
    apportion_targets(lower_mass, parent_mass, high_outcomes);

    candidates_len = 0;
    shell_total = shell_information = shell_cache_open = 0;
    LiftCandidate lower_cut;
    memset(&lower_cut, 0, sizeof(lower_cut));
    enumerate_inverse(ln, lm, lo, hi, k - 2, power3[k - 2], 0, &lower_cut);
    if (candidates_len > 1)
        qsort(candidates, candidates_len, sizeof(*candidates), cmp_candidate);
    printf("LIFT inverse k=%d parts=%d parent_mass=%lld lower_mass=%lld "
           "parent_outcomes=%d/%d/%d lower_target=%d/%d/%d total=%llu "
           "information=%llu cache_open=%llu\n",
           k, lift_parts, parent_mass, lower_mass, high_cut.p[0], high_cut.p[1],
           high_cut.p[2], outcome_target[0], outcome_target[1], outcome_target[2],
           shell_total, shell_information, shell_cache_open);

    int attempted = 0;
    for (size_t ci = 0; ci < candidates_len && attempted < explicit_budget;
         ci++, attempted++) {
        int sb0[LIFT_MAX_PARTS], sb1[LIFT_MAX_PARTS * 2], sb2[LIFT_MAX_PARTS];
        LiftCandidate *current = &candidates[ci];
        make_children(ln, lm, current, sb0, sb1, sb2);
        if (timed_true(sb0, lift_parts, k - 2, probe_ms) &&
            timed_true(sb2, lift_parts, k - 2, probe_ms) &&
            timed_true(sb1, lift_parts * 2, k - 2, probe_ms)) {
            printf("LIFT inverse-solution rank=%zu attempted=%d masses=%d/%d/%d cut=",
                   ci + 1, attempted + 1, current->p[0], current->p[1], current->p[2]);
            print_cut(current);
            printf("\n");
            free(candidates);
            return 0;
        }
    }
    printf("LIFT inverse-no-solution attempted=%d\n", attempted);
    free(candidates);
    return 1;
}

int main(int argc, char **argv) {
    if (argc < 4 || (strcmp(argv[3], "recursive") != 0 &&
                     strcmp(argv[3], "inverse") != 0)) {
        printf("usage: %s CACHE k {recursive|inverse} ...\n", argv[0]);
        return 3;
    }
    init();
    parse_file(argv[1]);
    int k = atoi(argv[2]);
    if (strcmp(argv[3], "inverse") == 0) return run_inverse(argc, argv, k);
    return run_recursive(argc, argv, k);
}
