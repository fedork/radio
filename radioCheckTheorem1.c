#include "radiobase.c"

int main(int argc, char **argv){
    init();
    int sb[power3[MAX_K]];
    int k, size, n1, n2, i,pairs;
    size = 3;
    for (k = 5; k<= MAX_K; k++){
//        for (size = 2; size<=5; size++)
        {
            for (i=0; i<size; i++) {
                sb[i] = 0;
            }
            
            pairs = 0;
            i=0;
            while(1) {
                int overflow = 1;
                i = 0;
                while (overflow && i<size) {
                    int sbbPrev = sb[i];
                    int sbbNext = sbbPrev+1;
                    pairs -= sb_pairs[sbbPrev];
                    int pairsNext = sb_pairs[sbbNext];
                    if (pairs + pairsNext <= power3[k]){
                        overflow = 0;
                        pairs += pairsNext;
                        sb[i] = sbbNext;
                    } else { //overflow
                        sb[i] = 1;
                        pairs += sb_pairs[sb[i]];
                        i++;
                    }
                }
                if (i>=size) break;
                if (canSolveB(sb, size, k, 0)) {
//                    printf("checking theorem for ");
//                    printSb(sb,size);
//                    printf(" in %d\n",k);
//                    fflush(stdout);
                    int i1, i2;
                    for (i1 = 1; i1 < size; i1++) {
                        int sbb1 = sb[i1];
                        if (!sbb1) continue;
                        int n1 = sbb_to_n1[sbb1];
                        int n2 = sbb_to_n2[sbb1];
                        // check theorem 1
                        for (i2=0; i2 < i1; i2++) {
                            int sbb2 = sb[i2];
                            if (!sbb2) continue;
                            int n3 = sbb_to_n1[sbb2];
                            int n4 = sbb_to_n2[sbb2];
                            if (n1>n2 && n2>n3 && n3>n4) {
                                sb[i1] = getSbb(n1, n2-1);
                                sb[i2] = getSbb(n3, n4+1);
                                if(!canSolveB(sb, size, k, 0)) {
                                    printf("theorem 1 FAIL: can't solve ");
                                    printSb(sb,size);
                                    printf(" even though can solve ");
                                    sb[i1] = sbb1;
                                    sb[i2] = sbb2;
                                    printSb(sb,size);
                                    printf(" in %d\n",k);
                                    fflush(stdout);
                                    exit(1);
                                }
                            } else if (n3>n4 && n4>n1 && n1>n2) {
                                sb[i1] = getSbb(n1, n2+1);
                                sb[i2] = getSbb(n3, n4-1);
                                if(!canSolveB(sb, size, k, 0)) {
                                    printf("theorem 1 FAIL: can't solve ");
                                    printSb(sb,size);
                                    printf(" even though can solve ");
                                    sb[i1] = sbb1;
                                    sb[i2] = sbb2;
                                    printSb(sb,size);
                                    printf(" in %d\n",k);
                                    fflush(stdout);
                                    exit(1);
                                }
                            }
                            sb[i1] = sbb1;
                            sb[i2] = sbb2;
                        }
                        
                        // check theorem 2
//                        sb[i1] = getSbb(n1+1, n2-1);
//                        if(!canSolveB(sb, size, k, 0)) {
//                            printf("theorem 2 FAIL: can't solve ");
//                            printSb(sb,size);
//                            printf(" even though can solve ");
//                            sb[i1] = sbb1;
//                            printSb(sb,size);
//                            printf(" in %d\n",k);
//                            fflush(stdout);
//                            exit(1);
//                        }
//                        sb[i1] = sbb1;
                    }
//                    printf("checked theorem for ");
//                    printSb(sb,size);
//                    printf(" in %d\n", k);
//                    fflush(stdout);
                }
                
            }
            printf("theorem checked all for k=%d size=%d\n", k, size);
            fflush(stdout);
        }
        printf("theorem checked all for k=%d\n", k);
        fflush(stdout);
    }
    return 0;
}
