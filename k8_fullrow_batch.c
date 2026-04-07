#define MAX_N 258
#define MAX_K 8
#include "radiobase.c"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
    init();
    if (argc > 1) parse_file(argv[1]);

    const int k = 8;
    int n1 = 56;
    int n2 = 55;
    const int max_n1 = 1 << k;
    int case_num = 0;
    while (n2 >= 1 && n1 <= max_n1) {
        int sb[1];
        sb[0] = getSbb(n1, n2);
        if (canSolveB(sb, 1, k, NO_DEADLINE) == TRUE) {
            n1++;
        } else {
            sb[0] = getSbb(n1-1, n2);
            case_num++;
            printf("==== CASE %d: Sb(%d:%d) in %d ====\n", case_num, n1-1, n2, k);
            fflush(stdout);
            all_solutions(sb, 1, k);
            printf("==== END CASE Sb(%d:%d) ====\n\n", n1-1, n2);
            fflush(stdout);
            n2--;
        }
    }
    return 0;
}
