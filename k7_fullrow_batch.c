#define MAX_N 130
#define MAX_K 7
#include "radiobase.c"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
    init();
    if (argc > 1) parse_file(argv[1]);

    const int k = 7;
    const int pairs[][2] = {
        {128,1},{127,2},{121,3},{116,4},{109,5},{104,6},{97,7},{91,8},
        {87,9},{82,10},{77,11},{73,12},{69,13},{66,14},{63,15},{60,16},
        {58,17},{55,18},{53,19},{51,20},{49,21},{47,22},{45,23},{43,24},
        {41,25},{40,26},{38,27},{37,28},{36,29},{35,30},{34,31},{33,32}
    };
    const int count = sizeof(pairs)/sizeof(pairs[0]);

    for (int i=0; i<count; i++) {
        int n = pairs[i][0], m = pairs[i][1];
        int sb[1];
        sb[0] = getSbb(n,m);
        printf("==== CASE %d/%d: Sb(%d:%d) in %d ====\n", i+1, count, n, m, k);
        fflush(stdout);
        all_solutions(sb, 1, k);
        printf("==== END CASE Sb(%d:%d) ====\n\n", n, m);
        fflush(stdout);
    }
    return 0;
}
