/* Measure persistent split-table bytes requested by one radio_one-style query.

   This deliberately excludes allocator bookkeeping and every non-split structure.  Build once
   against the parent engine and once against the current engine to isolate the table-layout
   change.  OLD_SPLITS selects the former layout's accounting formula.

     tools/build_radio.py -O3 -DMAX_K=6 -DMAX_N=193 -DOLD_SPLITS \
       -DRADIOBASE_PATH='"/path/to/old/radiobase.c"' tools/split_memory_probe.c -o /tmp/old
     tools/build_radio.py -O3 -DMAX_K=6 -DMAX_N=193 tools/split_memory_probe.c -o /tmp/new

   Usage is the same as radio_one: [cache] k n1 m1 [n2 m2 ...].
*/
#ifndef RADIOBASE_PATH
#define RADIOBASE_PATH "../radiobase.c"
#endif
#include RADIOBASE_PATH

int main(int argc, char **argv) {
    int offset;
    int k;
    int size;
    int i;
    int result;
    int sb[32];

    if (argc < 4) {
        printf("usage: %s [cache] k n1 m1 [n2 m2 ...]\n", argv[0]);
        return 3;
    }
    offset = (argc % 2 == 1) ? 1 : 0;
    k = atoi(argv[offset + 1]);
    size = (argc - offset - 2) / 2;
    if (size > (int)(sizeof(sb) / sizeof(sb[0]))) return 3;

    init();
    if (offset) parse_file(argv[1]);
    for (i = 0; i < size; i++) {
        sb[i] = getSbb(atoi(argv[offset + 2 + i * 2]),
                       atoi(argv[offset + 3 + i * 2]));
    }
    result = canSolveB(sb, size, k, NO_DEADLINE);
    printf("MEASURE verdict=%d\n", result);

#ifdef OLD_SPLITS
    {
        unsigned long long tables = 0;
        unsigned long long options = 0;
        unsigned long long bytes = (unsigned long long)(MAX_SBB + 1) * sizeof(splits);
        for (i = 1; i <= MAX_SBB; i++) {
            unsigned long long capacity;
            if (sbb_splits[i].size < 0) continue;
            capacity = (unsigned long long)(sbb_to_n1[i] + 1) * (sbb_to_n2[i] + 1);
            tables++;
            options += sbb_splits[i].size;
            bytes += capacity * (SPLIT_FIELD_COUNT + 4) * sizeof(int);
            bytes += (unsigned long long)3 * (sb_pairs[i] + 2) * sizeof(int);
        }
        printf("MEASURE tables=%llu options=%llu bytes=%llu\n", tables, options, bytes);
    }
#else
    {
        unsigned long long tables = 0;
        unsigned long long candidates = 0;
        unsigned long long options = 0;
        unsigned long long bytes = (unsigned long long)(MAX_SBB + 1) * sizeof(*sbb_splits)
                                 + split_level_fanout_bytes;
        for (i = 1; i <= MAX_K; i++) {
            tables += split_tables_built[i];
            candidates += split_table_candidates[i];
            options += split_table_options[i];
            bytes += split_table_bytes[i];
        }
        printf("MEASURE tables=%llu candidates=%llu options=%llu bytes=%llu\n",
               tables, candidates, options, bytes);
    }
#endif
    return result == TRUE ? 0 : result == FALSE ? 1 : 2;
}
