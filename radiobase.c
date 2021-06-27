#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_K 10

#ifndef MAX_N
#define MAX_N 200
#endif

#define MAX_SBB MAX_N*MAX_N/4
#define MAX_SPLITS (MAX_N/2) * (MAX_N/2 + 2)
#define BY_MAGIC 0
#define BY_MAX 1
#define BY_SP0 2
#define BY_SP1 3
#define BY_SP2 4
#define BY_MAGIC3 5
#define BY_MAGIC2 6
//#define BY_SP1_0 7
//#define BY_SP1_2 8

#define INDEX_COUNT 7

#define DESC 16
#define DESC_MASK 15
#define PROGRESS_INTERVAL CLOCKS_PER_SEC*60

#define FALSE 0
#define TRUE 1
#define MAYBE 2
//#define MAYBE_SLOW 3

#define FAST 8
#define SPLIT_FIELD_COUNT 9

#define DEADLINE_RATIO 10

#define CACHE_ONLY 1
#define NO_DEADLINE 2
#define FAST_ONLY 3

typedef struct {
    int size;
    int splitsl[MAX_SPLITS][SPLIT_FIELD_COUNT];
    int ind[INDEX_COUNT][MAX_SPLITS];
} splits;

unsigned long power3[MAX_K+1];
int n_to_sbb[MAX_N+1][MAX_N/2 + 1];
int sbb_to_n1[MAX_SBB+1];
int sbb_to_n2[MAX_SBB+1];
char sbb_to_str[MAX_SBB+1][8];
int sb_pairs[MAX_SBB+1];
int sa_can[MAX_N+1];
int sa_cant[MAX_N+1];
int sbb_lesser[MAX_SBB+1][MAX_SBB+1];
int sbb_greater[MAX_SBB+1][MAX_SBB+1];
clock_t wasted = 0;
clock_t not_wasted = 0;

splits sbb_splits[MAX_SBB+1];

int min(int a,int b){
    return a<b?a:b;
}

int desc (const void * a, const void * b) {
    return ( *(int*)b - *(int*)a );
}

void sort1(int *x, int len) {
    qsort(x, len, sizeof(int), desc);
}

int max(int a,int b){
    return a>b?a:b;
}

int getSbb(int n1, int n2){
    if (n1<n2) return getSbb(n2,n1);
    if (n2==0) return 0;
    return n_to_sbb[n1][n2];
}

void printSa(int n){
    printf("Sa(%d)", n);
}

void printSb(int *sb, int size){
    printf("Sb(");
    int pairs=0;
    int n=0;
    int i;
    for (i=0; i<size; i++) {
        if (i>0) printf(",");
        printf("%s",sbb_to_str[sb[i]]);
        pairs+=sb_pairs[sb[i]];
        n+=sbb_to_n1[sb[i]];
        n+=sbb_to_n2[sb[i]];
    }
    printf(")[%d,%d]",pairs,n);
}

typedef struct node {
    struct node *next;
    int can; int cant;
//    int not_fast;
} node_struct;

struct node sb_cache_root;

struct node *alloc_next(int arrsize){
    struct node * next;
    next = (struct node *)malloc((arrsize)*sizeof(struct node));
    if (next == NULL){
        printf("out of memory\n");
        fflush(stdout);
        exit(1);
    }
    int j;
    for (j=0; j<arrsize; j++) {
        next[j].can = MAX_K+1;
        next[j].cant = 0;
//        next[j].not_fast = 0;
        next[j].next = NULL;
    }
    return next;
}

int cacheCanSolve(struct node *n, int* sb, int size, int k, int arrsize){
    //	printf("size = %d\n",size);
    int updated =0;
    if(n->can > k){
        //		printf("cache n->can was %d, now %d\n",n->can, k);
        n->can = k;
        updated = 1;
    }
    if (size>0) {
        if (n->next == NULL) {
            //			printf("cache allocating %d\n", arrsize);
            n->next = alloc_next(arrsize+1);
        }
        int sbb=*sb;
        int *lesser;
        int minSbb = size>1?sb[1]:2; // see Unit Group Triviality Lemma
        //		printf("minSbb = %d\n",minSbb);
        int sbb2;
        lesser = sbb_lesser[sbb];
        while(1) {
            sbb2 = *lesser;
            if (sbb2<minSbb) break;
            //			printf("sbb2 = %d\n",sbb2);
            if (cacheCanSolve(&(n->next)[sbb2], sb+1, size-1,k, sbb2)) {
                updated=1;
            }
// ????????
//            else {
//                break;
//            }
            lesser++;
        }
    }
    return updated;
    
    //  ------- more comprehensive, but seems counterproductive:
    
    //    int cacheCanSolve(struct node *n, int* sbOrig, int size, int k, int arrsize){
    //    int updated =0;
    //    if(n->can > k){
    //        //        printf("cache n->can was %d, now %d\n",n->can, k);
    //        n->can = k;
    //        updated = 1;
    //    }
    //    if (size>0) {
    //        if (n->next == NULL) {
    //            n->next = alloc_next(arrsize+1);
    //        }
    //        int sb[size];
    //        memcpy(sb, sbOrig, size*sizeof(int));
    //        int i = 0;
    //        int sbb=*sb;
    //        int *lesser;
    //        int minSbb = size>1?sb[1]:1;
    //        //        printf("minSbb = %d\n",minSbb);
    //        int sbb2;
    //        lesser = sbb_lesser[sbb];
    //        while(1) {
    //            int nxt = *lesser;
    //            while (nxt<minSbb) {
    //                if (++i>=size) break;
    //                sb[i-1] = sb[i];
    //                minSbb=size>i+1?sb[i+1]:2; // see Unit Group Trivilaity Lemma
    //            }
    //            sb[i] = nxt;
    //            sbb2 = sb[0];
    //            //            printf("sbb2 = %d\n",sbb2);
    //            if (cacheCanSolve(&(n->next)[sbb2], sb+1, size-1,k, sbb2)) {
    //                updated=1;
    //            }
    //            else {
    //                break;
    //            }
    //            lesser++;
    //        }
    //    }
    //    return updated;
}

int cacheCantSolve(struct node *n, int* sb, int size, int k, int arrsize, int pairs){
    int updated =0;
    int tmp;
    //    printf("size=%d\n",size);
    if (size<1) {
        if (n->cant < k){
            //            printf("cache n->cant was %d, now %d\n",n->cant, k);
            n->cant = k;
            updated = 1;
        }
    } else {
        if (n->next == NULL) {
            //            printf("arrsize=%d\n",arrsize);
            n->next = alloc_next(arrsize+1);
        }
        int i;
        int sb2[size];
        memcpy(sb2, sb, size*sizeof(int));
        sb=sb2;
        for (i=0; i < size; i++) {
            if (i>0) {
                tmp = sb[i];
                sb[i]=sb[0];
                sb[0]=tmp;
            }
            int sbb = *sb;
            //        printf("sbb=%d\n",sbb);
            int *greater;
            greater = sbb_greater[sbb];
            int pairs_without_this = pairs - sb_pairs[sbb];
            //        printf("pairs_without_this=%d\n",pairs_without_this);
            int max_pairs = power3[k] - pairs_without_this;
            //        printf("max_pairs=%d\n",max_pairs);
            while(1) {
                int sbb2 = *greater;
                if (size==1 || sbb2>=sb[1]) {
                    //            printf("sbb2=%d %s\n",sbb2,sbb_to_str[sbb2]);
                    if (sbb2>arrsize) break;
                    int pairs_new = sb_pairs[sbb2];
                    //            printf("pairs_new=%d\n",pairs_new);
                    if (pairs_new>max_pairs) break;
                    
                    if (cacheCantSolve(&(n->next)[sbb2],sb+1, size-1,k, sbb2, pairs_without_this+pairs_new)) {
                        updated = 1;
                    }
//                    else {
//                        break;
//                    }
                }
                greater++;
            }
        }
    }
    return updated;
}

//void cacheNotFast(struct node *n, int* sb, int size, int k, int arrsize){
//
//    if (size == 0) {
//        n->not_fast = 1;
//    } else {
//        if (n->next == NULL) {
////            printf("cache allocating %d\n", arrsize);
////            fflush(stdout);
//            n->next = alloc_next(arrsize+1);
//        }
//
//        cacheNotFast(&(n->next)[sb[0]], sb+1, size-1,k, sb[0]);
//    }
//}

void cache(int *sb, int size, int canSolve, int k, int pairs) {
    struct node *n=&sb_cache_root;
    if (canSolve==TRUE) {
        cacheCanSolve(n,sb,size,k, MAX_SBB);
//    } else if (canSolve == MAYBE_SLOW) {
//        cacheNotFast(n,sb,size,k, MAX_SBB);
    } else {
        cacheCantSolve(n,sb,size,k, MAX_SBB, pairs);
    }
}

int checkCache(int *sb, int size, int k) {
    //	printf("in checkcache\n");
    int i;
    struct node *n=&sb_cache_root;
    for (i=0; i <= size; i++) {
        //		printf("checkcache i=%d\n",i);
        if(n->cant >= k){
            //			printf("checkcache can't solve: n->cant=%d k=%d\n", n->cant, k);
            return 0;
        } else {
            if (i==size) {
                if (n->can <= k){
                //				printf("checkcache can solve: n->can=%d k=%d\n", n->can, k);
                    return TRUE;
//                } else {
//                    if (n->not_fast) return MAYBE_SLOW;
                } else {
                    return MAYBE;
                }
            }
            if (n->next == NULL) {
                //				printf("checkcache n->next is null, return 2");
                return MAYBE;
            }
            n = &(n->next)[sb[i]];
        }
    }
    //	printf("checkcache n->next is null, return 2");
    return MAYBE;
}

int canSolveB(int *sb, int size, int k, clock_t parent_deadline){
    int canSolve=FALSE;
    int tmp[size];
    //todo: replace with memcpy
    int i;
    int pairs=0;
    int pairs_full=0;
    int newsize=0;
    int sbb;
    for(i=0;i<size;i++) {
        sbb=sb[i];
        pairs_full+=sb_pairs[sbb];
        // Unit Group Trivilaity Lemma: Unit Groups (1-1) do not affect solvability within information maximum and can be ignored
        if (sbb>1) {
            tmp[newsize++]=sbb;
            pairs+=sb_pairs[sbb];
        }
    }
    
    //    printf("in canSolveB in %d ", k);
    //    printSb(tmp, newsize);
    //    printf("\n");
    //    printf("pairs=%d\n", pairs);
    
    // check pairs
    if (pairs_full<=1) return TRUE;
    if (pairs_full>power3[k]) return FALSE;
    
    size = newsize;
    
    sort1(tmp, size);
    
    //    printf("sorted: ");
    //    printSb(tmp, size);
    //    printf("\n");
    
    //check cache
    int ck = checkCache(tmp, size, k);
    //	printf("got from cache %d\n", ck);
    if (parent_deadline == CACHE_ONLY || ck == TRUE || ck == FALSE) {
        return ck;
    }
//    if (parent_deadline == FAST_ONLY && ck == MAYBE_SLOW) {
//        return ck;
//    }

    int size_1 = size-1;
    int size2 = size*2;
    int k_1 = k-1;
    int max_pairs_1 = power3[k_1];
    int sb0[size],sb2[size],sb1[size2];
    int cont;
    long long totalsplits;
    int skiptop;
    
    unsigned long long maxsplits = 1;
    int splitincr[size];
    int splitindex[size];
    splits *splitsarr[size];
    int spi, spi2;
    
    int sb0p[size],sb2p[size],sb1p[size];
    
    for(i=0;i<size;i++){
        maxsplits *= (splitsarr[i] = &sbb_splits[tmp[i]])->size;
    }
    //full search
    clock_t start = clock();
    clock_t progress = start + PROGRESS_INTERVAL;
    
    clock_t deadline = 0;
    if (parent_deadline == NO_DEADLINE) {
        deadline = start + CLOCKS_PER_SEC * (1000);
//    } else if (parent_deadline == FAST_ONLY) {
//        deadline = start + CLOCKS_PER_SEC * (power3[k] * (1 << k)) / 100000;
    } else {
        if (start > parent_deadline) return MAYBE;
//        int deadline_ratio = size;
        int deadline_ratio = DEADLINE_RATIO;
        if (parent_deadline - start > CLOCKS_PER_SEC * deadline_ratio) {
            deadline = start + ((parent_deadline - start) / deadline_ratio);
        } else {
            deadline = start + CLOCKS_PER_SEC * 1 + 300;
        }
    }
    
//    printf("k=%d parent_deadline=%llu start=%llu deadline=%llu\n", k, parent_deadline, start, deadline);

    int cont2=1;
    int skipped_some;
    int pass = 0;

    clock_t child_deadline = (parent_deadline == NO_DEADLINE && size == 1)? NO_DEADLINE : deadline;
//    clock_t first_deadline;
    while (cont2) {
        pass++;
        clock_t max_time = (deadline - start) / CLOCKS_PER_SEC;
        skipped_some = 0;
        totalsplits=0;
        skiptop = 0;
        cont=1;
//        splitincr[0] = size == 1 ? BY_MAGIC : (size<=3 ? BY_MAGIC2 : ((sb_pairs[tmp[0]] < pairs / 3) ? BY_MAGIC3 : BY_MAX));
        splitincr[0] = size == 1 ? (DESC + BY_SP0) : (size<=3 ? BY_MAGIC2 : ((sb_pairs[tmp[0]] < pairs / 3) ? BY_MAGIC3 : BY_MAX));
        //    splitincr[0] = size == 1 ? BY_MAGIC : BY_MAX;
        //    splitincr[0] = size == 1 ? BY_MAGIC : ( (size>2 && sb_pairs[0] < pairs / 2) ? DESC + BY_MIN : BY_MAX);
        
        
        memset(splitindex, 0, size * sizeof(int));
        splitindex[0] = splitsarr[0]->size;
    
//        clock_t t = clock();
//        printf("solving in %d pass=%d ", k, pass);
//        printf("deadline=%lu ", deadline>t ? (deadline - t + CLOCKS_PER_SEC) / CLOCKS_PER_SEC : 0);
//        printSb(tmp, size);
//        printf("\n");

        //    fflush(stdout);
        
        i = 0;
        int ck0,ck1,ck2;
        
        while(cont) {
//            if (i > 0 && clock() > first_deadline) {
//                clock_t t = clock();
//                if (t < deadline) {
//                    first_deadline = t + (deadline - t) / 2 + CLOCKS_PER_SEC;
//                    i = 0;
//                    skipped_some = 1;
//                }
//            }
            while(splitindex[i] == 0
//                  || ( i > 0 && pass == 1 && i < size / 2 && splitindex[i] < splitsarr[i]->size / 2  && (clock()-start) * 2 > (deadline - start))
                  ) {
//                if (splitindex[i] > 0) skipped_some = 1;
                if (i==0) {
                    // can't solve
                    cont=0;
                    if (!skipped_some) {
                        cont2=0; // really can't solve
//                    } else if (parent_deadline==FAST_ONLY) {
//                        cache(tmp, size, MAYBE_SLOW, k, pairs);
//                        return MAYBE;
                    } else {
                        if (parent_deadline != NO_DEADLINE && pass>1) return MAYBE;
                        //                        deadline = 0; // do full solution now
                        // double deadline
                        //                        deadline = 2 * deadline - start;
                        // try again from the beginning with leftover deadline
                    }
                    break;
                }
                i--;
            }
            if (!cont) break;
            spi = --splitindex[i];
            spi2 = (splitincr[i] & DESC) ?
            splitsarr[i]->ind[splitincr[i] & DESC_MASK][splitsarr[i]->size - 1 - spi] :
            splitsarr[i]->ind[splitincr[i] & DESC_MASK][spi];
            int *s = splitsarr[i]->splitsl[spi2];
            
            //        while (size > 1 && s[4]<k) { // don't do this for size==1 messes up counts
            while (s[4]<k) {
                int kk = s[4];
                if (size > 1) {
                    if (canSolveB(s, 1, kk, NO_DEADLINE) != FALSE && canSolveB(s+3, 1, kk, NO_DEADLINE) != FALSE && canSolveB(s+1, 2, kk, NO_DEADLINE) != FALSE)
                        s[4] = MAX_K;
                    else
                        s[5] = ++s[4];
                } else {
                    int ttt = canSolveB(s, 1, kk, CACHE_ONLY);
                    if (ttt==TRUE) {
                        ttt = canSolveB(s+3, 1, kk, CACHE_ONLY);
                        if (ttt==TRUE) {
                            ttt = canSolveB(s+1, 2, kk, CACHE_ONLY);
                        }
                    }
                    if (ttt == TRUE)
                        s[4] = MAX_K;
                    else if (ttt == FALSE)
                        s[5] = ++s[4];
                    else break;
                }
            }
            if (s[5]<k) {
                if (i==0 &&
                    sb1[0] == s[1] &&
                    max(sb0[0],sb2[0]) == max(s[0], s[3]) &&
                    min(sb0[0],sb2[0]) == min(s[0], s[3])
                    )
                {
                    skiptop++;
                } else if (pass == 1
                           && !s[FAST]
                           && (i<size_1)
                           ) {
                    skipped_some = 1;
                } else {
                    totalsplits++;
                    int p0 = sb0p[i] = sb_pairs[sb0[i] = s[0]] + (i>0?sb0p[i-1]:0);
                    int p1 = sb1p[i] = sb_pairs[sb1[i*2] = s[1]] + sb_pairs[sb1[i*2+1] = s[2]] + (i>0?sb1p[i-1]:0);
                    int p2 = sb2p[i] = sb_pairs[sb2[i] = s[3]] + (i>0?sb2p[i-1]:0);
                    
                    int cs0, cs1, cs2;

                    if ((p0 <= max_pairs_1) && (p1 <= max_pairs_1) && (p2 <= max_pairs_1)
                        && (cs0 = canSolveB(sb0, i+1, k_1, CACHE_ONLY)) && (cs2 = canSolveB(sb2, i+1, k_1, CACHE_ONLY)) && (cs1 = canSolveB(sb1, (i+1) * 2, k_1, CACHE_ONLY))
                        )
                    {
                        if (i == size_1) {
                            clock_t t = clock();
                            if (t>deadline){
                                if (parent_deadline != NO_DEADLINE) {
                                    return MAYBE;
                                } else {
                                    cont=0;
                                    //                                    deadline=0;  // now do full solution
                                    // double deadline
                                    deadline+= (deadline - start);
                                    break;
                                }
                            }
                            if (t >= progress) {
                                printf("still solving in %d pass=%d ", k, pass);
//                                printf("wasted %lu \tnot_wasted %lu ", wasted / CLOCKS_PER_SEC, not_wasted / CLOCKS_PER_SEC);
                                printSb(tmp, size);
                                printf(" trying ");
                                printSb(sb0, size);
                                printSb(sb1, size2);
                                printSb(sb2, size);
                                printf(" elapsed %lu/%lu left=%d/%d totalsplits=%llu\n", (t - start)/CLOCKS_PER_SEC, max_time, splitindex[0], splitsarr[0]->size, totalsplits);
                                fflush(stdout);
                                progress = t + PROGRESS_INTERVAL;
                            }
                            if ((cs0 = canSolveB(sb0, i+1, k_1, child_deadline)) != TRUE) {
                                if (cs0 != FALSE)
                                    skipped_some = 1;
//                                else {
//                                    int s2 = i;
//                                    while (s2>0 && canSolveB(sb0, s2, k-1, deadline)==0) s2--;
//                                }
                            } else if ((cs2 = canSolveB(sb2, i+1, k_1, child_deadline)) != TRUE) {
                                if (cs2 != FALSE)
                                    skipped_some = 1;
//                                else {
//                                    int s2 = i;
//                                    while (s2>0 && canSolveB(sb2, s2, k-1, deadline)==0) s2--;
//                                }
                            } else if((cs1 = canSolveB(sb1, (i+1) * 2, k_1, deadline))  != TRUE) {
                                if (cs1!=FALSE)
                                    skipped_some = 1;
//                                else {
//                                    int s2 = i;
//                                    while (s2>0 && canSolveB(sb1, s2*2, k-1, deadline)==0) s2--;
//                                }
                            } else {
                                //can solve
                                canSolve=TRUE;
                                cont=0;
                                cont2=0;
                                break;
                            }
                        } else {
                            i++;
                            //                    splitindex[i] = tmp[i]==tmp[i-1] ? splitindex[i-1]+1 : splitsarr[i]->size;
                            splitindex[i] = splitsarr[i]->size;
                            

                            //                    int e=1;
                            //                        if ( p1<=max(p0,p1) && max(abs(p0-p1), max(abs(p0-p2), abs(p2-p1))) <= e) {
                            //                        if ( (p1<=p0 && p1<=p2) && max(abs(p0-p1), max(abs(p0-p2), abs(p2-p1))) <= e)
//                            if (size==2 && i==1)
//                            {
//                                splitincr[i] = BY_MAGIC2;
//                            }
//                            else
                            //                            if (size/(i+1)>=2)
                            //                            {
                            //                                splitincr[i] = /*(sb_pairs[tmp[i]] < pairs / 3) ? BY_MAGIC3 : */ BY_MAX;
                            //                            }
                            //                            else
//                            int e = sb_pairs[tmp[i]] / 3; // is this a good factor?
//                            if ( abs(p0-p1) <= e && abs(p0-p2) <= e && abs(p2-p1) <= e)
//                            {
//                                //                                splitincr[i] = BY_MAGIC3; // if routhly equal - split equally
//                                splitincr[i] = (size - i > 1)? BY_MAGIC : BY_MAGIC3; // if routhly equal - split equally
//                            }
//                            else
                            {
                                // confusingly enough p0 corresponds to BY_SP2 and p2 to BY_SP0
                                if (p0 > p1) {
                                    if (p1 > p2) { // p0 > p1 > p2
                                        splitincr[i] = ( p0 - p1 > p1 - p2) ? BY_SP2 : (DESC + BY_SP0);
                                    } else if (p0 > p2) { // p0 > p2 >= p1
                                        splitincr[i] = ( p0 - p2 > p2 - p1) ? BY_SP2 : (DESC + BY_SP1);
                                    } else { // p2 >= p0 > p1
                                        splitincr[i] = ( p2 - p0 > p0 - p1) ? BY_SP0 : (DESC + BY_SP1);
                                    }
                                } else { // p1 >=p0
                                    if (p0 > p2) { // p1 >= p0 > p2
                                        //                                    splitincr[i] = (p1 - p2 > e*2) ? (DESC + BY_SP0) : BY_MAGIC3;
                                        //                                    splitincr[i] = (p1 - p0 > p0 - p2) ? BY_SP1_2 : (DESC + BY_SP0);
                                        //                                    splitincr[i] = (DESC + BY_SP0);
                                        splitincr[i] = (p1 - p0 > p0 - p2) ? BY_SP1 : (DESC + BY_SP0);
                                    } else if (p1 > p2) { // p1 > p2 >= p0
                                        //                                    splitincr[i] = (p1 - p0 > e*2) ? (DESC + BY_SP2) : BY_MAGIC3;
                                        //                                    splitincr[i] = ( p1 - p2 > p2 - p0) ? BY_SP1_0 : (DESC + BY_SP2);
                                        //                                    splitincr[i] = (DESC + BY_SP2);
                                        splitincr[i] = ( p1 - p2 > p2 - p0) ? BY_SP1 : (DESC + BY_SP2);
                                    } else { // p2 >= p1 >= p0
                                        splitincr[i] = ( p2 - p1 > p1 - p0) ? BY_SP0 : (DESC + BY_SP2);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    if (canSolve) {
        //        printf("cansolve=true\n");
        //        fflush(stdout);
        
        printf("can solve ");
        printSb(tmp, size);
        printf(" in %d  with [",k);
        for (i = 0; i<size; i++) {
            spi = splitindex[i];
            spi2 = (splitincr[i] & DESC) ?
            splitsarr[i]->ind[splitincr[i] & DESC_MASK][splitsarr[i]->size - 1 - spi] :
            splitsarr[i]->ind[splitincr[i] & DESC_MASK][spi];
            int *s = splitsarr[i]->splitsl[spi2];
            if (i>0) printf(",");
            printf("%d:%d", s[6], s[7]);
            if (!s[FAST]) {
                printf(":NOTFAST");
                if (i<size-1) {
                    s[FAST]=1;
                    printf("-ADDED");
                }
            }
        }
        printf("] ");
        printSb(sb0,size);
        printSb(sb1,size*2);
        printSb(sb2,size);
        //        printf("cansolve=true\n");
        //        printf("totalsplits=%llu\n", totalsplits);
        //        fflush(stdout);
        
        
//        i=0;
//        while(i < size && splitindex[i] == splitsarr[i]->size-1) i++;
//        if (i<size) {
//            printf(" suboptimal for i=%d splitincr=%d tried: ", i, splitincr[i]);
//            int desc = splitincr[i] & DESC;
//            //            printf("desc=%d\n", desc);
//            int *ind = splitsarr[i]->ind[splitincr[i] & DESC_MASK];
//            for (spi = splitsarr[i]->size-1; spi> splitindex[i]; spi--) {
//                //                printf("spi=%d\n", spi);
//                spi2 = desc ? ind[splitsarr[i]->size - 1 - spi] : ind[spi];
//                //                printf("spi2=%d\n", spi2);
//                int *s = splitsarr[i]->splitsl[spi2];
//                printf(" [%d-%d] -> ", s[6], s[7]);
//                printSb(s,1);
//                printSb(s+1,2);
//                printSb(s+3,1);
//                printf(";");
//                //                fflush(stdout);
//            }
//        }
    } else if (skipped_some) {
        return MAYBE;
    } else {
        printf("can't solve ");
        printSb(tmp, size);
        printf(" in %d",k);
    }
    clock_t t = clock()-start;
    clock_t s = t/CLOCKS_PER_SEC;
    if (s>0)
        printf(" took %ld ", s);
    else
        printf(" took 0.%03ld ", t * 1000/CLOCKS_PER_SEC);
    printf("totalsplits=%llu of %llu skiptop=%d pass=%d\n", totalsplits, maxsplits, skiptop, pass);
    //    fflush(stdout);
    cache(tmp, size, canSolve, k, pairs);

    return canSolve;
}

int saPairs(int n) {
    return n * (n-1) / 2;
}

int canSolveA(int n, int k) {
    
    int pairs = saPairs(n);
    //	printf("pairs=%d\n",pairs);
    if (pairs<=1) {
        //		printf("pairs is <=1, can solve\n");
        return 1;
    }
    if (sa_can[n]<=k) {
        //		printf("sa_can[%d]=%d, can solve\n", n, sa_can[n]);
        return 1;
    }
    if (sa_cant[n]>=k) {
        //		printf("sa_cant[%d]=%d, can't solve\n", n, sa_cant[n]);
        return 0;
    }
    int canSolve = 0;
    
    clock_t start = clock();
    //  	printf("solving Sa(%d) in %d\n",n,k);
    if (pairs<=power3[k]){
        int n1 = n-1;
        int sb[1];
        while (n1>=(n+1)/2 && canSolve == 0) {
            //			printf("n1=%d\n", n1);
            if (canSolveA(n1,k-1)) {
                sb[0]=getSbb(n1,n-n1);
                //				printf("sb[0]=%d \n",sb[0]);
                //				printSb(sb,1);
                //				printf("\n");
                if (canSolveB(sb,1,k-1,NO_DEADLINE)>0) {
                    canSolve = 1;
                    printf("can solve Sa(%d) in %d with following:",n,k);
                    printSa(n1);
                    printf(",");
                    printSa(n - n1);
                    printf(",");
                    printSb(sb,1);
                }
            }
            n1--;
        }
    } else {
        printf("power3[%d]=%lu can't solve should not be here\n", k, power3[k]);
        fflush(stdout);
        exit(1);
    }
    if(canSolve){
        int i;
        for (i = n; i > 0; i--) {
            sa_can[i] = min(k, sa_can[i]);
        }
    } else {
        printf("can't solve Sa(%d) in %d",n,k);
        int i;
        for (i = n; i <=MAX_N; i++) {
            sa_cant[i]=max(k, sa_cant[i]);
        }
    }
    clock_t t = clock()-start;
    clock_t s = t/CLOCKS_PER_SEC;
    if (s>0)
        printf(" took %ld\n", s);
    else
        printf(" took 0.%03ld\n", t * 1000/CLOCKS_PER_SEC);
    return canSolve;
} 

typedef struct { int sort; int index; int *s;} srt;

// sort splits such that symmetrical ones are next to each other
int descSpl (const void * a, const void * b) {
    srt *a1 = (srt*)a;
    srt *b1 = (srt*)b;
    int result = b1->sort - a1->sort;
    if (result) return result;
    result = b1->s[1] - a1->s[1];
    if (result) return result;
    result = max(b1->s[0], b1->s[3]) - max(a1->s[0], a1->s[3]);
    if (result) return result;
    result = min(b1->s[0], b1->s[3]) - min(a1->s[0], a1->s[3]);
    return result;
}

void indexSpl(int sbb, splits* s, int indexindex, int (*f)(int, int[])) {
    srt splitsort[MAX_SPLITS];
    int e;
    int c = s->size;
    for(e = 0; e<c; e++) {
        splitsort[e].sort = f(sbb, s->splitsl[e]);
        splitsort[e].index = e;
        splitsort[e].s = s->splitsl[e];
    }
    qsort(splitsort, c, sizeof(srt), descSpl); // sort by first element
    for(e = 0; e<c; e++) {
        s->ind[indexindex][e] = splitsort[e].index;
    }
}

int maxpairsraw(int sbb, int spl[]) {
    return max(sb_pairs[spl[0]], max(sb_pairs[spl[3]], sb_pairs[spl[1]] + sb_pairs[spl[2]]));
}

int minpairsraw(int sbb, int spl[]) {
    //    return min(sb_pairs[spl[0]], min(sb_pairs[spl[3]], (sb_pairs[spl[1]] + sb_pairs[spl[2]]) * 411 / 1000 /* magic! */));
    return min(sb_pairs[spl[0]], min(sb_pairs[spl[3]], sb_pairs[spl[1]] + sb_pairs[spl[2]]));
}

int pairs2raw(int sbb, int spl[]) {
    return sb_pairs[spl[0]];
}

int pairs0raw(int sbb, int spl[]) {
    return sb_pairs[spl[3]];
}

int pairs1raw(int sbb, int spl[]) {
    return sb_pairs[spl[1]] + sb_pairs[spl[2]];
}

int pairs2(int sbb, int spl[]) {
    return pairs2raw(sbb, spl) * (1+sb_pairs[sbb]) + abs(pairs1raw(sbb, spl) - pairs0raw(sbb, spl));
}

int pairs0(int sbb, int spl[]) {
    return pairs0raw(sbb, spl) * (1+sb_pairs[sbb]) + abs(pairs2raw(sbb, spl) - pairs0raw(sbb, spl));
}

int pairs1(int sbb, int spl[]) {
    return pairs1raw(sbb, spl) * (1+sb_pairs[sbb]) + abs(pairs2raw(sbb, spl) - pairs0raw(sbb, spl));
}

int pairs1_0(int sbb, int spl[]) {
    int n1 = sbb_to_n1[sbb];
    int n2 = sbb_to_n2[sbb];
    int magicm1 = n1/4;
    int magicm2 = 0;
    int dx = abs(spl[6] - magicm1);
    int dy = abs(spl[7] - magicm2);
    return dy * (n1+1) + dx;
}

int pairs1_2(int sbb, int spl[]) {
    int n1 = sbb_to_n1[sbb];
    int n2 = sbb_to_n2[sbb];
    int magicm1 = n1 - n1/4;
    int magicm2 = n2;
    int dx = abs(spl[6] - magicm1);
    int dy = abs(spl[7] - magicm2);
    return dy * (n1+1) + dx;
}

int maxpairs(int sbb, int spl[]) {
    return (1+maxpairsraw(sbb, spl)) * (1+sb_pairs[sbb]) - minpairsraw(sbb, spl);
}

int distance(int spl[], int magicm1, int magicm2, int n1, int n2) {
    int dx = (spl[6] - magicm1) * 1000/n1;
    int dy = (spl[7] - magicm2)* 1000/n2;
    int dx2 = (spl[6] - (n1 - magicm1))* 1000/n1;
    int dy2 = (spl[7] - (n2 - magicm2))* 1000/n2;
    return min(dx*dx + dy*dy, dx2*dx2 + dy2*dy2);
}

int magic(int sbb, int spl[]) {
    int n1 = sbb_to_n1[sbb];
    int n2 = sbb_to_n2[sbb];
    int msum = ((n1+n2)*577+999)/1000;   // magic ratio is 0.577 = (1/sqrt(3))
    int magicm1 = min(n1, (n1*577+999)/1000);
    int magicm2 = min(n2, msum-magicm1);
    return distance(spl, magicm1, magicm2, n1, n2);
}


int magic2(int sbb, int spl[]) {
    int n1 = sbb_to_n1[sbb];
    int n2 = sbb_to_n2[sbb];
    int msum = ((n1+n2)*666+500)/1000;
    int magicm1 = min(n1, (n1*666+500)/1000);
    int magicm2 = min(n2, msum-magicm1);
    return distance(spl, magicm1, magicm2, n1, n2);
}

int magic3(int sbb, int spl[]) {
    //    return (1+minpairsraw(sbb, spl)) * (1+sb_pairs[sbb]) - maxpairsraw(sbb, spl);
    int n1 = sbb_to_n1[sbb];
    int n2 = sbb_to_n2[sbb];
    //    int msum = (n1+n2-3)/2;
    int magicm1 = n1/2;
    int magicm2 = n2/2;
    //int magicm2 = ((n1-n2)<2)?(n2/2-1):(n2/2);
    //    int magicm2 = min(n2/2, magicm1-1);
    return distance(spl, magicm1, magicm2, n1, n2);
}

void init(){
    int i,pow,k,n;
    for (i=0,pow=1; i<= MAX_K; i++){
        power3[i]=pow;
        pow*=3;
        //  	printf("%d\n", pow);
    }
    k=0;
    for(i=2;i<=MAX_N;i++) {
        sa_can[i] = MAX_K+1;
        while(saPairs(i)>=power3[k+1]) k++;
        sa_cant[i] = k;
        //  	printf("can't solve %d in %d pairs = %d power3 = %d\n", i,k,saPairs(i),power3[k+1]);
    }
    //  exit(0);
    
    sb_cache_root.next = NULL;
    
    int n1, n2, sbb;
    sbb=0;
    sb_pairs[0]=0;
    sprintf(sbb_to_str[0],"0:0");
    int prod;
    int maxprod = (MAX_N/2)*(MAX_N - MAX_N/2);
    //  printf("maxprod=%d\n", maxprod);
    for (prod =1; prod<=maxprod;prod++) {
        //  	printf("prod=%d\n", prod);
        for (n2 = MAX_N-1; n2 > prod/n2; n2--);
        for (; n2>0; n2--) {
//        for (n1=min(prod,MAX_N-1), n2=1; n1>=n2; n1--) {
//            if (n1>0) n2 = prod/n1;
            n1 = prod/n2;
            if (n1>=n2 && n1+n2<=MAX_N && n1*n2 == prod) {
                //   		printf("n1=%d\n", n1);
                //	  		printf("n2=%d\n", n2);
                //  for (n1 = 1; n1 < MAX_N; n1++) {
                //  	for (n2 = 1; n2 <= n1 && n1+n2 <= MAX_N; n2++) {
                n_to_sbb[n1][n2] = ++sbb;
                if (sbb>=MAX_SBB+1) {
                    printf ("sbb=%d, MAX_SBB=%d\n", sbb, MAX_SBB);
                    exit(1);
                }
                
                //		  		printf("sbb=%d\n", sbb);
                
                sbb_to_n1[sbb]=n1;
                sbb_to_n2[sbb]=n2;
                sb_pairs[sbb]=n1*n2;
                sprintf(sbb_to_str[sbb],"%d:%d",n1,n2);
                int c=0;
                int k1,k2;
                for (k1=n1; k1>0; k1--) {
                    for (k2=min(k1,n2); k2>0; k2--) {
                        sbb_lesser[sbb][c++]=getSbb(k1,k2);
                    }
                }
                //!!! must be sorted!!!!
                sort1(sbb_lesser[sbb], c);
                sbb_lesser[sbb][c++]=0; //terminator
            }
        }
    }
    
    if (sbb>MAX_SBB) {
        printf ("sbb=%d, MAX_SBB=%d\n", sbb, MAX_SBB);
        exit(1);
    }
    
    for (i=1; i<=MAX_SBB; i++) {
        int k = 0;
        int j;
        for (j=i; j<=MAX_SBB; j++) {
            if (sbb_to_n1[j]>=sbb_to_n1[i] && sbb_to_n2[j]>=sbb_to_n2[i])
                sbb_greater[i][k++]=j;
        }
        // terminator
        sbb_greater[i][k++]=MAX_SBB + 1;
    }
    printf("initializing splits");
    int maxsplits = 0;
    for(sbb=1; sbb<=MAX_SBB; sbb++) {
        splits *s = &sbb_splits[sbb];
        //        printf("initializing splits for ");
        //        printSb(&sbb, 1);
        //        printf("\n");
        if ((sbb & 0xff) == 0) printf(".");
        fflush(stdout);
        n1 = sbb_to_n1[sbb];
        n2 = sbb_to_n2[sbb];
        int c=0;
        int e=0;
        int m1,m2;
        int bestmaxpairs = sb_pairs[sbb];
        
        for (m1=0; m1<=n1; m1++) {
//            int b = (n1-n2)/2;
//            int fast_min = max(0, m1 - b - 1) ;
//            int fast_max = min(n2, fast_min + 2);
            int fast_min = max(0, m1 * n2 / n1 - 0) ;
            int fast_max = min(n2, fast_min + 1);
//            if (m1 <= (n1/2 + 1) && m1 >= (n1/2 - 1)) {
                // thicker middle
//                fast_min--;
//                fast_max++;
//            }
            for (m2=(n2==n1?m1:n2); m2>=0; m2--) {
                s->splitsl[c][0]=getSbb(m1, m2);
                int sbb1 = getSbb(n1-m1, m2);
                int sbb2 = getSbb(m1, n2-m2);
                s->splitsl[c][1]=max(sbb1, sbb2);
                s->splitsl[c][2]=min(sbb1, sbb2);
                s->splitsl[c][3]=getSbb(n1-m1, n2-m2);
                int maxpairs = max(sb_pairs[s->splitsl[c][0]],max(sb_pairs[s->splitsl[c][3]], sb_pairs[s->splitsl[c][1]] + sb_pairs[s->splitsl[c][2]]));
                
                int k=0;
                while (power3[k]<=maxpairs) k++;
                s->splitsl[c][4] = s->splitsl[c][5] = k-1;
                s->splitsl[c][6] = m1;
                s->splitsl[c][7] = m2;
                s->splitsl[c][FAST] = ((m2>=fast_min) && (m2<=fast_max));
                c++;
            }
        }
        if (c>MAX_SPLITS) {
            printf("c=%d MAX_SPLITS=%d\n",c,MAX_SPLITS);
            exit(1);
        }
        
        s->size = c;
        if (c>maxsplits) maxsplits = c;
        indexSpl(sbb, s, BY_MAX, maxpairs);
        indexSpl(sbb, s, BY_MAGIC, magic);
        indexSpl(sbb, s, BY_SP0, pairs0);
        indexSpl(sbb, s, BY_SP1, pairs1);
        indexSpl(sbb, s, BY_SP2, pairs2);
        indexSpl(sbb, s, BY_MAGIC3, magic3);
        indexSpl(sbb, s, BY_MAGIC2, magic2);
//        indexSpl(sbb, s, BY_SP1_0, pairs1_0);
//        indexSpl(sbb, s, BY_SP1_2, pairs1_2);
        //        int r;
        //        for(r=0; r<5; r++)
        //            for(e = 0; e<c; e++) {
        //                printf("sorted %d: ", r);
        //                printSb(s->splitsl[s->ind[r][e]], 1);
        //                printSb(s->splitsl[s->ind[r][e]]+1, 2);
        //                printSb(s->splitsl[s->ind[r][e]]+3, 1);
        //                printf("\n");
        //            }
    }
    if (maxsplits < MAX_SPLITS) {
        printf("expected MAX_SPLITS: %d - actual: %d\n", MAX_SPLITS, maxsplits);
        exit(1);
    }
    printf("\ninit done\n");
    fflush(stdout);
}
