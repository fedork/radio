#define MAX_N 446
#define MAX_K 9

#include "radiobase.c"

int main(int argc, char **argv){
    init();
    if (argc>1) {
        parse_file(argv[1]);
    }
    int sb[1];
    int k, n1, n2;
    n2 = 17;
    n1 = 339;
    k=9;

    while (n1+n2<=MAX_N) {
        sb[0]=getSbb(n1,n2);
        if (canSolveB(sb,1,k,NO_DEADLINE)) {
            printf("result   can solve ");
            n1++;
        } else {
            printf("result can't solve ");
            n2--;
        }
        printSb(sb, 1);
        printf(" in %d\n", k);
    }
    return 0;
}
