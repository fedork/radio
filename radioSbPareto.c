
#define MAX_N 260

#include "radiobase.c"

int main(int argc, char **argv){
    init();
    int sb[1];
    int k, n1, n2;
    for (k = 1; k<= MAX_K; k++){
        n1=min(1+(1<<k), MAX_N-1);
        n2=1;
        while (n1>=n2) {
            sb[0]=getSbb(n1,n2);
            if (canSolveB(sb,1,k,0)) {
                printf("result   can solve ");
                n2++;
            } else {
                printf("result can't solve ");
                n1--;
            }
            printSb(sb, 1);
            printf(" in %d\n", k);
        }
    }
    return 0;
}
