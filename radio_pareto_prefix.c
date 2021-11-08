#define MAX_N 260

#include "radiobase.c"

int main(int argc, char **argv){
    init();
    
    if (argc<2) {
        printf("insufficient args\n");
        exit(1);
    }

    int k = atoi(argv[1]);
    int size = atoi(argv[2]);
//    int size = (argc - 2)/2 + 1;
    int sb[size];
    int i;
    for (i=0; 4+i*2<argc; i++) {
        sb[i] = getSbb(atoi(argv[3+i*2]), atoi(argv[4+i*2]));
    }
    int j=i;
    printf("k=%d prefix: ", k);
    printSb(sb, size-1);
    printf("\n");
    
    int n1[size], n2[size];
    n1[i]=1 << k;
    n2[i]=1;
    while (i >= j) {
        if (n2[i]<=n1[i]) {
            sb[i]=getSbb(n1[i],n2[i]);
            if (canSolveB(sb,i+1,k,NO_DEADLINE)) {
                printf("result   can solve ");
                printSb(sb, i+1);
                printf(" in %d\n", k);
                n2[i]++;
                if (i < size-1) {
                    i++;
                    n1[i]=1 << k;
                    n2[i]=1;
                }
            } else {
//                printf("result can't solve ");
                n1[i]--;
            }
        } else {
            i--;
        }
    }
    return 0;
}
