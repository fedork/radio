#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_K 9

#ifndef MAX_N
#define MAX_N 200
#endif

#define MAX_SBB MAX_N*MAX_N/4
#define MAX_SPLITS (MAX_N/2) * (MAX_N/2 + 2)

int power3[MAX_K+1];
int k_1 = 0;

typedef struct {int sbb2; int sbb1_1; int sbb1_2; int sbb0;} sp_struct;

void *cache[MAX_K+1][MAX_SBB+1];
void *dummy[1];
sp_struct splits[MAX_K+1][MAX_SBB+1][MAX_SPLITS+1];
int sbb_to_n1[MAX_K+1][MAX_SBB+1];
int sbb_to_n2[MAX_K+1][MAX_SBB+1];
int n_to_sbb[MAX_K+1][MAX_N][MAX_N];
int sbb_pairs[MAX_K+1][MAX_SBB+1];
int splits_count[MAX_K+1][MAX_SBB+1];
char sbb_str[MAX_K+1][MAX_SBB+1][8];
int max_n1[MAX_K+1][MAX_N+1];

int desc (const void * a, const void * b) {
    return ( *(int*)b - *(int*)a );
}

void printSb(int k, int* sb, int size) {
    printf("Sb(");
    int pairs=0;
    int n=0;
    int i;
    for (i=0; i<size; i++) {
        if (i>0) printf(",");
        int sbb = sb[i];
        printf("%s",sbb_str[k][sbb]);
        pairs+=sbb_pairs[k][sbb];
        n+=sbb_to_n1[k][sbb];
        n+=sbb_to_n2[k][sbb];
    }
    printf(")[%d,%d]",pairs,n);
}

int canSolveB(int* sb, int size, int k) {
    
//    printf("in canSolveB k=%d ", k);
//    printSb(k, sb, size);
//    printf("\n");
//    fflush(stdout);
    int tmp[size];
    int pairs=0;
    int new_size = 0;
    int i;
    for(i = 0; i < size; i++) {
        int sbb = sb[i];
        pairs+=sbb_pairs[k][sbb];
//        printf("pairs=%d\n", pairs);
        if (pairs > power3[k]) {
//            printf("can't solve 1\n");
//            fflush(stdout);
            return 0;
        }
        if (sbb>1) { // drop zero group and unit group (0 and 1)
            tmp[new_size++] = sbb;
        }
    }
    if (pairs == 0) {
//        printf("can solve 1\n");
//        fflush(stdout);
        return 1;
    }
    if (new_size > 1) {
        qsort(tmp, new_size, sizeof(int), desc);
    }
//    printf("sorted: ");
//    printSb(k, tmp, new_size);
//    printf("\n");
//    fflush(stdout);
    void** node = cache[k];
    for (i = 0; i < new_size; i++) {
//        printf("i=%d dummy=%p node=%p\n", i, dummy, node);
//        fflush(stdout);
        if (node == dummy) {
            printf("dummy unexpected\n");
            fflush(stdout);
            exit(1);
        }
        node = (node[tmp[i]]);
        if (node == NULL) {
//            printf("can't solve 2\n");
//            fflush(stdout);
            return 0;
        }
    }
//    printf("can solve 2\n");
//    fflush(stdout);
    return 1;
}

int solveAll(void** results, int k, int* sb, int size_1, int max_size, int max_sbb) {
    int can_solve_any = 0;
    int size = size_1+1;
    int canSolve = (size_1==0); // for size_1==0 we know they are all solvable
    int pairs = 0;
    int i;
    for (i = 0; i < size_1; i++) {
        pairs+=sbb_pairs[k][sb[i]];
    }
    for (int sbb = max_sbb; sbb > 1; sbb--) {
        sb[size_1] = sbb;
        clock_t start = clock();
        clock_t progress = start + 60 * CLOCKS_PER_SEC;
        long long total_splits = 0;
        int sb2[size];
        int sb1[size*2];
        int sb0[size];
        if (pairs + sbb_pairs[k][sbb] > power3[k]) {
            canSolve = 0;
        } else if (!canSolve) {
            // find a solution
            sp_struct *sp[size];
            int sp_count[size];
            int sp_index[size];
            
            i = 0;
            for (i = 0; i < size; i++) {
                sp[i] = splits[k][sb[i]];
                sp_count[i] = splits_count[k][sb[i]];
                //TODO: do not iterate redundant splits at the top
                sp_index[i] = i==0 ? sp_count[i] : 0;
            }
            
            i = 0;
            int cont = 1;
            while(cont) {
                total_splits++;
                while(sp_index[i] == 0) {
                    if (i == 0) {
                        cont = 0;
                        break;
                    }
                    i--;
                }
                if (!cont) break;
                int spi = --sp_index[i];
                sp_struct* sppp = sp[i];
                sp_struct* s = &sppp[spi]; //??
                sb2[i] = s->sbb2;
                sb0[i] = s->sbb0;
                sb1[i*2] = s->sbb1_1;
                sb1[i*2+1] = s->sbb1_2;
                if (clock()>=progress) {
                    printf("still solving elapsed %lu splits %llu in %d ", (clock() - start), total_splits, k);
                    printSb(k, sb, size);
                    printf(" with ");
                    printSb(k_1, sb2, i+1);
                    printSb(k_1, sb1, i*2+2);
                    printSb(k_1, sb0, i+1);
                    printf("\n");
                    progress = clock() + 60 * CLOCKS_PER_SEC;
                }
                if (i==0 || // for i==0 we know they are all solvable
                    (canSolveB(sb2, i+1, k_1) && canSolveB(sb0, i+1, k_1) && canSolveB(sb1, i*2+2, k_1))) {
                    if (i == size_1) {
                        canSolve = 1;
                        cont = 0;

                        break;
                    } else {
                        i++;
                        sp_index[i] = sp_count[i];
                    }
                }
            }
            if (!canSolve) {
                printf("can't solve in %d ", k);
                printSb(k, sb, size);
                printf("\n");
    //            fflush(stdout);
            }
        }
        if (canSolve) {
            can_solve_any = 1;
            if (size < max_size) {
                
                // create next cache entry
                void** next = (void**)malloc((sbb+1)*sizeof(void*));
                if (next == NULL) {
                    printf("out of memory\n");
                    fflush(stdout);
                    exit(1);
                }
                results[sbb] = next;
                // descend
                if (!solveAll(next, k, sb, size, max_size, sbb)) {
                    // only pring if longer ones are not solvable
                    printf("can solve in %d ", k);
                    printSb(k, sb, size);
                    printf(" with ");
                    printSb(k_1, sb2, size);
                    printSb(k_1, sb1, size*2);
                    printSb(k_1, sb0, size);
                    printf("\n");
//                    fflush(stdout);
                }
            } else {
                results[sbb] = dummy; // means this is solvable, but we do not need to check longer ones
            }
            // if we reach diagonal, we start next row and need to start checking solvability again
            if (sbb_to_n2[sbb] == sbb_to_n1[sbb]) {
                canSolve = 0;
            }
        } else { // can't solve
            results[sbb] = NULL;
        }
    }
    return can_solve_any;
}

int getSbb(int k, int n1, int n2) {
//    printf("getSbb: k=%d n1=%d n2=%d max_n1=%d n_to_sbb=%d\n", k, n1, n2, max_n1[k][n2], n_to_sbb[k][n1][n2]);
    fflush(stdout);
    if (n1 == 0 || n2 == 0) return 0;
    if (n2 > n1) return getSbb(k, n2, n1);
    if (n1 > max_n1[k][n2]) return -1;
    return n_to_sbb[k][n1][n2];
}

int main(int argc, char **argv){

    int k;

    memset(max_n1, 0, sizeof(max_n1));
    power3[0] = 1;
    sprintf(sbb_str[0][0], "%d:%d", 0, 0);
    sprintf(sbb_str[0][1], "%d:%d", 1, 1);
    sbb_pairs[0][0] = 0;
    sbb_pairs[0][1] = 1;
    max_n1[0][1] = 1;
    n_to_sbb[0][1][1] = 1;
    
    do { // k loop
        k = k_1 + 1;
        printf("starting for k=%d\n", k);
        fflush(stdout);
        power3[k] = power3[k_1] * 3;
        printf("power3=%d\n", power3[k]);
        fflush(stdout);

        // zero group init
        sbb_pairs[k][0] = 0;
        sprintf(sbb_str[k][0], "%d:%d", 0, 0);
        
        // enumerate all solvable sbbs
        int sbb = 1;
        int n2 = 1;
        
        while (1) {

            int n1 = n2;
            while (1) {
                printf("starting splits for %d:%d\n", n1, n2);
                fflush(stdout);
                // enumerate all solvable splits
                int m1, m2;
                int split_count = 0;
                for (m1 = 0 ; m1 <= n1; m1++) {

                    int m1_c = n1 - m1;
                    for (m2 = 0; m2 <= (n1==n2 ? m1 : n2) ; m2++) {
//                        printf("trying split with %d:%d\n", m1, m2);
//                        fflush(stdout);
                        int m2_c = n2 - m2;
                        
                        int sb2 = getSbb(k_1, m1, m2);
                        int sb0 = getSbb(k_1, m1_c, m2_c);
                        int sb1[] = {getSbb(k_1, m1_c, m2), getSbb(k_1, m1, m2_c)};
                        if (sb0 >= 0 && sb2 >= 0 && sb1[0] >= 0 && sb1[1]>= 0) {
                            if (canSolveB(sb1, 2, k_1)) {
                                printf("good split %d:%d with %d:%d => %s  /  %s,%s  /  %s\n", n1,n2,m1,m2, sbb_str[k_1][sb2], sbb_str[k_1][sb1[0]], sbb_str[k_1][sb1[1]], sbb_str[k_1][sb0]);
                                fflush(stdout);
                                // good split
                                splits[k][sbb][split_count].sbb2 = sb2;
                                splits[k][sbb][split_count].sbb0 = sb0;
                                // TODO: maybe sort sb1
                                splits[k][sbb][split_count].sbb1_1 = sb1[0];
                                splits[k][sbb][split_count].sbb1_2 = sb1[1];
                                split_count++;
                            }
                        }
                    }
                }
                if (split_count==0) break;
                splits_count[k][sbb] = split_count;
            
                // solvable sbb
                sbb_to_n1[k][sbb] = n1;
                sbb_to_n2[k][sbb] = n2;
                n_to_sbb[k][n1][n2] = sbb;
                n_to_sbb[k][n2][n1] = sbb;
                sbb_pairs[k][sbb] = n1*n2;
                sprintf(sbb_str[k][sbb], "%d:%d", n1, n2);
                max_n1[k][n2] = n1;
                
                sbb++;
                n1++;
            }
            n2++;
            if (n2>=n1) break;
        }
        
        int sbb_count = sbb;  // done listing sbb
        
        // find max solutions
        int max_size = 1 << (MAX_K - k); // longest states we need to consider
        
        int sb[max_size];
 
        printf("done with splits for k=%d\n", k);
        fflush(stdout);
        
        solveAll(cache[k], k, sb, 0, max_size, sbb_count-1);
        
        
        // done for k
        printf("done for k=%d\n", k);
        fflush(stdout);
        k_1 = k;
//        power3_1 = power3;
    } while ( k < MAX_K);
    printf("done\n");
    fflush(stdout);
    return 0;
}
