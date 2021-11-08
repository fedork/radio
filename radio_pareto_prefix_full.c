#define MAX_N 260

#include "radiobase.c"

int main(int argc, char **argv){
    init();
    
    if (argc<2) {
        printf("insufficient args\n");
        exit(1);
    }

    int k = atoi(argv[1]);
    int size = (argc - 2)/2 + 1;
    int sb[size];
    int i;
    for (i=0; i<(size-1); i++) {
        sb[i] = getSbb(atoi(argv[2+i*2]), atoi(argv[3+i*2]));
    }
    printf("k=%d prefix: ", k);
    printSb(sb, size-1);
    printf("\n");
    
    int lastindex = size-1;
    int n1, n2;
    n1=1 << k;
    n2=1;
    while (n2<=n1) {
        sb[lastindex]=getSbb(n1,n2);
        if (canSolveB(sb,size,k,NO_DEADLINE)) {
            printf("result   can solve ");
            printSb(sb, size);
            printf(" in %d\n", k);
            all_solutions(sb, size, k);
            n2++;
        } else {
            printf("result can't solve ");
            n1--;
        }
    }
    return 0;
}
