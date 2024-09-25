#include "radiobase.c"

int main(int argc, char **argv){

    init();
    
    int sbb = getSbb(25,1);
    int sb[] = {sbb, sbb, sbb, sbb, sbb, sbb, sbb, sbb, sbb, sbb, sbb, sbb, sbb};
    
    int  k = 5;
    int i = 1;
    while(i<13 && canSolveB(sb, i, k, 2)) {
        printf("result can solve ");
        printSb(sb, i);
        printf(" in %d\n", k);
        i++;
    }
    printf("result can't solve ");
    printSb(sb, i);
    printf(" in %d\n", k);

    
    
    
//    int k=5;
//    int nn1[] = {64, 63, 58, 54, 50, 46, 42, 38, 36, 33, 31, 29, 27, 25, 24, 22, 21, 20, 19};
//    int i;
//    for (i = 0; i < 19; i++) {
//        int n1 = nn1[i];
//        int n2 = i+1;
//        int m1,m2;
//        int sb0[1], sb1[2], sb2[1];
//        for (m2 = n2/2; m2>=0; m2--) {
//            for (m1 = 0; m1 <=n1; m1++) {
//                sb0[0] = getSbb(m1,m2);
//                sb1[0] = getSbb(n1-m1,m2);
//                sb1[1] = getSbb(m1,n2-m2);
//                sb2[0] = getSbb(n1-m1,n2-m2);
//                if (canSolveB(sb0,1,k) &&
//                    canSolveB(sb2,1,k) &&
//                    canSolveB(sb1,2,k)) {
//                    printf("result   can solve ");
//                } else {
//                    printf("result can't solve ");
//                }
//                printf("%d-%d with\t%d\t%d\t0.%d\t0.%d\t0.%d\n", n1, n2, m1, m2, m1*1000/n1, m2*1000/n2, (m1+m2)*1000/(n1+n2));
//            }
//        }
//    }
    return 0;
}
