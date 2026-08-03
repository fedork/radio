#!/usr/bin/env python3
"""Generate an instrumented copy of radiobase.c, for profiling the solver's hot path.

The engine is one deeply-recursive function with everything inlined at -O3, so sampling
profilers attribute ~100% of self time to `canSolveB` and tell you nothing. These counters
and the ablation technique below are what actually located the cost on 2026-08-03.

    tools/instrument.py counters  > /tmp/rb_counters.c    call/probe/closure/occupancy stats
    tools/instrument.py x2cache   > /tmp/rb_x2cache.c     trie walk executed twice
    tools/instrument.py x2pre     > /tmp/rb_x2pre.c       probe preamble executed twice
    tools/instrument.py fastsort  > /tmp/rb_fastsort.c    libc qsort -> inline insertion sort

**The ablation trick.** To cost a component without changing semantics, execute it *twice*
with a compiler barrier between, and diff the runtime against the unmodified build. The
delta is one extra execution of that component. Caveat: this measures the *marginal, warm*
cost - the second execution benefits from the first having pulled the lines into cache - so
it underestimates memory-bound components. Treat the numbers as lower bounds.

Build against a generated file with, e.g.

    clang -O3 -DMAX_K=9 -DMAX_N=193 -DRADIOBASE='"/tmp/rb_counters.c"' driver.c -o prog

where the driver does `#include RADIOBASE` instead of `#include "radiobase.c"`. For
`counters`, the driver must also call `instr_report()` (declared `void instr_report(void);`)
before exiting, or nothing is printed. For a run that will not terminate, install a
`SIGALRM` handler that calls it and `_exit(0)` - that is how the `MAX_N=193` figures were
taken, 180 s into a state that needs weeks.
"""
from __future__ import annotations

import os
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), ".."))


def read_base() -> str:
    with open(os.path.join(ROOT, "radiobase.c")) as fh:
        return fh.read()


def sub(s: str, old: str, new: str, what: str) -> str:
    if s.count(old) != 1:
        sys.exit(f"instrument.py: anchor for {what!r} matched {s.count(old)} times - "
                 f"radiobase.c has changed, update this script")
    return s.replace(old, new, 1)


FASTSORT = ('''void sort1(int *x, int len) {
    qsort(x, len, sizeof(int), desc);
}''', '''static inline void sort1(int *x, int len) {
    /* Measured 2026-08-03: mean length 2.84, 81% of calls len<=3, ~8.9M calls per k=8
       ladder. libc qsort - opaque call, indirect comparator per comparison, memcpy swaps -
       costs far more than the sort. Descending, matching the `desc` comparator exactly.
       Worth a measured 1.10x on the k=8 ladder with byte-identical verdicts. */
    for (int i = 1; i < len; i++) {
        int v = x[i], j = i - 1;
        while (j >= 0 && x[j] < v) { x[j + 1] = x[j]; j--; }
        x[j + 1] = v;
    }
}''')


def counters(s: str) -> str:
    s = sub(s, '#include <time.h>', '''#include <time.h>
long long I_calls=0,I_probe=0,I_full=0,I_sorts=0,I_sortelems=0;
long long I_cc_calls=0,I_cc_steps=0,I_closure=0,I_ins=0,I_splititer=0;
long long I_peak_alloc=0,O_nodes=0,O_slots=0,O_used=0;
long long I_size_hist[80];''', 'counter globals')
    s = sub(s, '''long long alloc_count = 0;''', '''static void occ_walk(struct node *n){
    if (n->next==NULL||n->next==can_solve_marker||n->next==cant_solve_marker) return;
    O_nodes++; O_slots += n->size;
    for (int j=0;j<n->size;j++){ struct node *c=&n->next[j];
        if (c->next!=NULL){ O_used++; occ_walk(c); } }
}
long long alloc_count = 0;''', 'occupancy walker')
    # the walker needs n->size unconditionally
    s = sub(s, '''typedef struct node {
    struct node *next;
#ifndef OPT
    int size;
#endif
} node_struct;''', '''typedef struct node {
    struct node *next;
    int size;
} node_struct;''', 'node struct')
    s = sub(s, '''    n->next = next;
#ifndef OPT
    n->size = arrsize;
#endif''', '''    n->next = next;
    n->size = arrsize;''', 'alloc_next size')
    s = sub(s, '''#ifndef OPT
    if (n->size != arrsize) {''', '''#if 0
    if (n->size != arrsize) {''', 'free_children size check')
    s = sub(s, '''int checkCache(int *sb, int size, int k) {
    int i;''', '''int checkCache(int *sb, int size, int k) {
    I_cc_calls++;
    int i;''', 'checkCache entry')
    s = sub(s, '''            n = &(n->next)[sb[i]];''', '''            I_cc_steps++;
            n = &(n->next)[sb[i]];''', 'checkCache step')
    s = sub(s, '''void sort1(int *x, int len) {
    qsort''', '''void sort1(int *x, int len) {
    I_sorts++; I_sortelems+=len;
    qsort''', 'sort1')
    s = sub(s, '''    int canSolve=FALSE;
    int tmp[size];''', '''    I_calls++; if (parent_deadline==CACHE_ONLY) I_probe++; else I_full++;
    I_size_hist[size<79?size:79]++;
    int canSolve=FALSE;
    int tmp[size];''', 'canSolveB entry')
    s = sub(s, '''            spi = --splitindex[i];''', '''            I_splititer++;
            spi = --splitindex[i];''', 'split loop')
    s = sub(s, '''    debug_printf(" cache=%lld/%lld''', '''    I_closure += updated; I_ins++;
    if (alloc_size > I_peak_alloc) I_peak_alloc = alloc_size;
    debug_printf(" cache=%lld/%lld''', 'closure accounting')
    return s + '''
void instr_report(void){
    for (int kk=0;kk<=MAX_K;kk++) occ_walk(&sb_cache_root[kk]);
    fprintf(stderr,"\\n=== INSTRUMENTATION ===\\n");
    fprintf(stderr,"canSolveB calls   %lld  (CACHE_ONLY probes %lld = %.1f%%, full %lld)\\n",
            I_calls,I_probe,100.0*I_probe/(I_calls?I_calls:1),I_full);
    fprintf(stderr,"split-loop iterations %lld\\n",I_splititer);
    fprintf(stderr,"checkCache probes %lld  trie steps %lld (%.2f/probe)\\n",
            I_cc_calls,I_cc_steps,(double)I_cc_steps/(I_cc_calls?I_cc_calls:1));
    fprintf(stderr,"sort1 calls %lld  mean len %.2f\\n",
            I_sorts,(double)I_sortelems/(I_sorts?I_sorts:1));
    fprintf(stderr,"TRIE: %lld nodes, %lld slots, %lld used => %.3f%% occupancy, fanout %.2f\\n",
            O_nodes,O_slots,O_used,100.0*O_used/(O_slots?O_slots:1),
            (double)O_used/(O_nodes?O_nodes:1));
    fprintf(stderr,"trie memory %.1f MB for %lld closure members = %.0f bytes/member"
            "  (%lld inserts, %.1f members each)\\n",
            I_peak_alloc*8.0/1048576.0,I_closure,I_closure?I_peak_alloc*8.0/I_closure:0.0,
            I_ins,(double)I_closure/(I_ins?I_ins:1));
    fprintf(stderr,"state-size histogram: ");
    for (int i=0;i<40;i++) if (I_size_hist[i]) fprintf(stderr,"%d:%lld ",i,I_size_hist[i]);
    fprintf(stderr,"\\n");
}
'''


def x2cache(s: str) -> str:
    s = sub(s, 'int checkCache(int *sb, int size, int k) {',
            'static int checkCache_inner(int *sb, int size, int k) {', 'checkCache rename')
    tail = '''    //	printf("checkcache n->next is null, return 2");
    return MAYBE;
}'''
    return sub(s, tail, tail + '''

volatile int g_cc_sink;
static int checkCache(int *sb, int size, int k) {
    g_cc_sink = checkCache_inner(sb, size, k);
    __asm__ __volatile__("" ::: "memory");
    return checkCache_inner(sb, size, k);
}''', 'checkCache wrapper')


def x2pre(s: str) -> str:
    s = sub(s, 'int min(int a,int b){', 'volatile int g_pre_sink;\nint min(int a,int b){',
            'preamble sink')
    return sub(s, '''    int canSolve=FALSE;
    int tmp[size];
    int singletons[size];''', '''    int canSolve=FALSE;
    int tmp[size];
    int singletons[size];
    {
        int tmp2[size], sing2[size];
        int ns2=0, ss2=0, pf2=0, p2=0;
        for (int q=0;q<size;q++){ int b=sb[q]; pf2+=sb_pairs[b];
            if (b>1){ tmp2[ns2++]=b; p2+=sb_pairs[b]; if (sbb_to_n2[b]==1) sing2[ss2++]=b; } }
        if (ns2>1) sort1(tmp2, ns2);
        if (ss2>1) sort1(sing2, ss2);
        g_pre_sink = tmp2[0]+pf2+p2+ns2+ss2;
        __asm__ __volatile__("" ::: "memory");
    }''', 'preamble duplicate')


MODES = {
    "counters": counters,
    "x2cache": x2cache,
    "x2pre": x2pre,
    "fastsort": lambda s: sub(s, FASTSORT[0], FASTSORT[1], "sort1"),
}


def main() -> int:
    if len(sys.argv) != 2 or sys.argv[1] not in MODES:
        print(__doc__)
        return 2
    sys.stdout.write(MODES[sys.argv[1]](read_base()))
    return 0


if __name__ == "__main__":
    sys.exit(main())
