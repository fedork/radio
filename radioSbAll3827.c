#include "radiobase.c"

int main(int argc, char **argv){

    init();
    int k=6;
    int n1 = 45;
    int n2 = 23;
    int m1,m2;
    int sb0[1], sb1[2], sb2[1];
    for (m2 = n2/2; m2>=0; m2--) {
        for (m1 = 0; m1 <=n1; m1++) {
            sb0[0] = getSbb(m1,m2);
            sb1[0] = getSbb(n1-m1,m2);
            sb1[1] = getSbb(m1,n2-m2);
            sb2[0] = getSbb(n1-m1,n2-m2);
            if (canSolveB(sb0,1,k) &&
                canSolveB(sb2,1,k) &&
                canSolveB(sb1,2,k)) {
                printf("result   can solve ");
            } else {
                printf("result can't solve ");
            }
            printf("with\t%d\t%d\n", m1, m2);
        }
    }
    return 0;
}
