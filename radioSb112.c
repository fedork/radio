#include "radiobase.c"

int main(int argc, char **argv){

    init();
    int k=8;
    int n1;
    for (n1 = 65; n1>=56; n1--) {
        int n2 = 112-n1;
        int sb[1]= {getSbb(n1,n2)};

        if (canSolveB(sb,1,k)) {
            printf("result   can solve ");
        } else {
            printf("result can't solve ");
        }
        printSb(sb, 1);
        printf("in %d\n", k);
    }
    return 0;
}
