/* Regression for the positive-cache semantic epoch. */

#include "../radiobase.c"

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: cache_semantics_regression UNTRUSTED TRUSTED\n");
        return 2;
    }
    init();
    int positive[4] = {
        getSbb(3, 1), getSbb(2, 1), getSbb(2, 1), getSbb(2, 1),
    };
    int negative[1] = {getSbb(4, 1)};
    sort1(positive, 4);

    parse_file(argv[1]);
    if (checkCacheTrie(positive, 4, 2) != MAYBE || sa_can[3] <= 2
        || checkCacheTrie(negative, 1, 1) != FALSE
        || ignored_positive_cache_replays != 2) {
        fprintf(stderr, "untrusted cache semantics regression failed\n");
        return 1;
    }

    parse_file(argv[2]);
    if (checkCacheTrie(positive, 4, 2) != TRUE || sa_can[3] > 2
        || ignored_positive_cache_replays != 2) {
        fprintf(stderr, "trusted cache semantics regression failed\n");
        return 1;
    }

    printf("CACHE_SEMANTICS_REGRESSION trusted_positive=YES "
           "ignored_untrusted_positive=%lld negative_replay=YES\n",
           ignored_positive_cache_replays);
    return 0;
}
