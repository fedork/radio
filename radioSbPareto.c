#define MAX_N 260

#include "radiobase.c"

int main(int argc, char **argv){
    init();
    int sb[1];
    int k, n1, n2;
    k=8;
    n1=140;
    n2=18;
    while (n2>9) {
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
