#include "radiobase.c"
#include <math.h>

double all_solutions(int sb[], int size, int k) {
    int tmp[size];
    int i;
    int pairs=0;
    int newsize=0;
    int sbb;
    for(i=0;i<size;i++) {
        sbb=sb[i];
        if (sbb > 0) {
            pairs+=sb_pairs[sbb];
            tmp[newsize++]=sbb;
        }
    }
    if (pairs == 0) return 0.0;
    size = newsize;
    
    sort1(tmp, size);
   
    int n[size*2], m[size*2];
    int sb0[size*2], sb1[size*2], sb2[size*2];
    
    int counts[size][MAX_N+1][MAX_N+1];
    memset(counts, 0, sizeof(counts));

    
    for(i=0;i<size;i++) {
        n[i*2] = sbb_to_n1[tmp[i]];
        n[i*2+1] = sbb_to_n2[tmp[i]];
        m[i*2] = 0;
        m[i*2+1] = 0;
    }
    
    m[0] = 1 + n[0];
    int j = 0;
    unsigned long long solved = 0;
    unsigned long long total = 0;
    while(1) {
        while(m[j] == 0) {
            if (j==0) {

                int m1, m2;
                for (i=0; i<size; i++) {
                    printf("i=%d\n", i);
                    for(m1 = 0; m1 <= n[i*2]; m1++) {
                        for(m2 = 0; m2 <= n[i*2+1]; m2++) {
                            int count = counts[i][m1][m2];
                            if (count > 0) {
                                printf("[%d:%d]:%d\n", m1, m2, count);
                            }
                        }
                    }
                    
                    for(m1 = 0; m1 <= n[i*2]; m1++) {
                        for(m2 = 0; m2 <= n[i*2+1]; m2++) {
                            int count = counts[i][m1][m2];
                            if (count > 0) {
                                if (count<10)
                                    printf("%d", count);
                                else
                                    printf("*");
                            } else printf(".");
                        }
                        printf("\n");
                    }
                }
                double s = exp(log((long double)solved / total)/2/size);
                printf("result in %d ratio = %llu/%llu solvability %f ", k, solved, total, s);
                printSb(tmp, size);
                printf("\n");
                return s;
            }
            j--;
        }
        m[j]--;
//        printf("l1 j = %d mj = %d\n", j, m[j]);
        for(i=0;i<size;i++) {
            sb0[i] = getSbb(m[i*2], m[i*2+1]);
            sb2[i] = getSbb(n[i*2] - m[i*2], n[i*2+1] - m[i*2+1]);
            sb1[i*2] = getSbb(m[i*2], n[i*2+1] - m[i*2+1]);
            sb1[i*2 + 1] = getSbb(n[i*2] - m[i*2], m[i*2+1]);
        }
        
        if (j == size*2 - 1) {
            total++;
            if (canSolveB(sb0, size, k-1, 2) == 1 && canSolveB(sb2, size, k-1, 2) == 1 && canSolveB(sb1, size*2, k-1, 2) == 1) {
                solved++;
                printf("result in %d can solve ", k);
                printSb(tmp, size);
                printf(" with [");
                for (i = 0; i<size; i++) {
                    if (i>0) printf(",");
                    printf("%d:%d", m[i*2], m[i*2+1]);
                    counts[i][m[i*2]][m[i*2+1]]++;
                }
                printf("] => ");
                printSb(sb0, size);
                printSb(sb1, size*2);
                printSb(sb2, size);
                printf("\n");
            }
        } else {
            j++;
            m[j] = n[j] + 1;
        }
    }
}

int main(int argc, char **argv){

    init();
    
    if (argc<4) {
        printf("insufficient args\n");
        exit(1);
    }

    int k = atoi(argv[1]);
    int size = (argc - 2)/2;
    int sb[size];
    int i;
    for (i=0; i<size; i++) {
        sb[i] = getSbb(atoi(argv[2+i*2]), atoi(argv[3+i*2]));
    }
    printf("k=%d ", k);
    printSb(sb, size);
    printf("\n");
    all_solutions(sb, size, k);
}
