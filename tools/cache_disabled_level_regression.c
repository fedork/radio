/* Regression for the survey-only RADIO_CACHE_DISABLED_LEVEL option. */

#include "../radiobase.c"

int main(void) {
    init();

    int level_three[] = {getSbb(3, 1), getSbb(2, 1)};
    sort1(level_three, 2);
    long long allocations_before = alloc_count;
    cache(level_three, 2, TRUE, 3, 5);
    if (alloc_count != allocations_before
        || checkCacheTrie(level_three, 2, 3) != MAYBE) {
        fprintf(stderr, "disabled level reached the dominance trie\n");
        return 1;
    }

    cache_l1_entry *entry = NULL;
    uint32_t hash = 0;
    if (cache_l1_probe(&radio_default_search_context, level_three, 2, 3,
                       &entry, &hash) != MAYBE
        || entry != NULL) {
        fprintf(stderr, "disabled level reached the exact cache probe\n");
        return 1;
    }
    cache_l1_store(&radio_default_search_context, entry, hash,
                   level_three, 2, 3, TRUE);
    if (cache_l1_probe(&radio_default_search_context, level_three, 2, 3,
                       &entry, &hash) != MAYBE) {
        fprintf(stderr, "disabled level reached the exact cache store\n");
        return 1;
    }

    int level_two[] = {
        getSbb(3, 1), getSbb(2, 1), getSbb(2, 1), getSbb(2, 1),
    };
    sort1(level_two, 4);
    cache(level_two, 4, TRUE, 2, 9);
    if (checkCacheTrie(level_two, 4, 2) != TRUE) {
        fprintf(stderr, "enabled child level did not retain its fact\n");
        return 1;
    }

    printf("CACHE_DISABLED_LEVEL_REGRESSION disabled=3 parent=ABSENT child=RETAINED\n");
    return 0;
}
