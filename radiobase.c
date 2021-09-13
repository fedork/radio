#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef DEBUG
#define debug_printf(...) do{ printf( __VA_ARGS__ ); fflush(stdout);} while( 0 )
#define DEBUG1 1
#else
#define debug_printf(...) /* Nothing */
#ifndef DEBUG1
#define OPT 1
#endif
#endif

#define MAX_K 10

#ifndef MAX_N
#define MAX_N 196
#endif

#define MAX_SBB MAX_N*MAX_N/4
#define MAX_SPLITS (MAX_N/2) * (MAX_N/2 + 2)
#define MAX_PROD (MAX_N/2)*(MAX_N - MAX_N/2)
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

#define DEADLINE_RATIO 6
#define MIN_DEADLINE 3

#define CACHE_ONLY 1
#define NO_DEADLINE 2
#define FAST_ONLY 3

typedef struct {
    int size;
    int splitsl[MAX_SPLITS][SPLIT_FIELD_COUNT];
    int ind[INDEX_COUNT][MAX_SPLITS];
} splits;

int power3[MAX_K+1];
int n_to_sbb[MAX_N+1][MAX_N/2 + 1];
int sbb_to_n1[MAX_SBB+1];
int sbb_to_n2[MAX_SBB+1];
char sbb_to_str[MAX_SBB+1][8];
int sb_pairs[MAX_SBB+1];
int sa_can[MAX_N+1];
int sa_cant[MAX_N+1];
int sbb_lesser[MAX_SBB+1][MAX_SBB+1];
int sbb_greater[MAX_SBB+1][MAX_SBB+1];
int max_sbb_for_pairs[MAX_PROD+1];

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
} node_struct;

struct node can_solve_marker[1];
struct node cant_solve_marker[1];

struct node sb_cache_root[MAX_K+1];

long long alloc_count = 0;
long long alloc_size = 0;

struct node *alloc_next(int arrsize){
    struct node * next;
    int size = (arrsize)*sizeof(struct node);
    next = (struct node *)malloc(size);
    alloc_count++;
    alloc_size+=arrsize;
    if (next == NULL){
        printf("\nout of memory\n");
        exit(1);
    }
    memset(next, 0, size);
    debug_printf("in alloc n=%p arrsize=%d\n", next, arrsize);
    return next;
}

void free_recursive(struct node *n, int arrsize, int pairs_remaining) {
    debug_printf("can't marker %p\n", cant_solve_marker);
    debug_printf("can marker %p\n", can_solve_marker);
    debug_printf("in free n=%p arrsize=%d pairs_remaining=%d\n", n, arrsize, pairs_remaining);
    int sbb;
    for (sbb=2; sbb<arrsize; sbb++) {
        struct node *next = n[sbb].next;
        if (next == NULL || next == can_solve_marker || next == cant_solve_marker) continue;
        int new_pairs_remaining = pairs_remaining - sb_pairs[sbb];
        int sbb2 = min(sbb, max_sbb_for_pairs[min(MAX_PROD, new_pairs_remaining)]);
        free_recursive(next, sbb2+1, new_pairs_remaining);
    }
    alloc_count--;
    alloc_size-=arrsize;
    free(n);
}

int cacheCanSolve(struct node *n, int* sb, int size, int k, int max_sbb, int pairs_remaining){
    //	printf("size = %d\n",size);
#ifdef DEBUG
    printf("in cacheCanSolve n=%p n->next=%p max_sbb=%d pairs_remaining=%d ", n, n->next, max_sbb, pairs_remaining);
    printSb(sb, size);
    printf("\n");
#endif
#ifndef OPT
    if (n->next == cant_solve_marker) {
        printf("\nencountered cant_solve_marker when caching can solve\n");
        fflush(stdout);
        exit(2);
    }
#endif
    if (size<1) {
        if (n->next == can_solve_marker) {
            debug_printf("already cached, returning 0\n");
            return 0;
        }
        if (n->next == NULL) {
            debug_printf("was NULL, setting \n");
            n->next = can_solve_marker;
        } else {
            if (n->next[0].next == can_solve_marker){
                debug_printf("already cached, returning 0\n");
                return 0;
            }
            debug_printf("n->next[0].next = can_solve_marker\n");
            n->next[0].next = can_solve_marker;
        }
        return 1;
    }
    int updated =0;
    max_sbb=min(max_sbb, max_sbb_for_pairs[min(MAX_PROD, pairs_remaining)]);
#ifdef DEBUG
    printf("in cacheCanSolve size=%d max_sbb=%d after ", size, max_sbb);
    printSb(&max_sbb, 1);
    printf("\n");
#endif
    if (n->next == NULL || n->next == can_solve_marker) {
        n->next = alloc_next(max_sbb+1);
    }
    if(n->next[0].next != can_solve_marker) {
        n->next[0].next = can_solve_marker;
        updated++;
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
        debug_printf("sbb2 = %d(%s)\n", sbb2, sbb_to_str[sbb2]);
        updated+=cacheCanSolve(&(n->next)[sbb2], sb+1, size-1,k, sbb2, pairs_remaining - sb_pairs[sbb2]);
        lesser++;
    }
    debug_printf("updated=%d\n", updated);
    return updated;
}

int cacheCantSolve(struct node *n, int* sb_orig, int size, int k, int max_sbb, int pairs, int pairs_remaining){
    
    debug_printf("in cache n=%p n->next=%p can't solve size=%d max_sbb=%d before pairs=%d, pairs_remaining=%d ", n, n->next, size, max_sbb, pairs, pairs_remaining);
    
#ifdef DEBUG
    printSb(sb_orig, size);
    printf("\n");
#endif
    
    //    printf("size=%d\n",size);
    if (n->next == cant_solve_marker) {
        debug_printf("already cached, returning 0\n");
        return 0;
    }
    max_sbb = min(max_sbb, max_sbb_for_pairs[min(MAX_PROD, pairs_remaining)]);
    debug_printf("in can't solve size=%d max_sbb=%d(%s) after\n", size, max_sbb, sbb_to_str[max_sbb]);
    if (size < 1) {
#ifndef OPT
        if (n->next == can_solve_marker) {
            printf("\nencountered can_solve_marker when caching can't solve (1)\n");
            exit(3);
        }
#endif
        if (n->next != NULL) {
            debug_printf("before free\n");
            free_recursive(n->next, max_sbb+1, pairs_remaining);
            debug_printf("after free\n");
        }
        n->next = cant_solve_marker;
        return 1;
    }
    if (n->next == NULL || n->next == can_solve_marker) {
        //            printf("arrsize=%d\n",arrsize);
        debug_printf("before alloc\n");
        n->next = alloc_next(max_sbb+1);
        debug_printf("after alloc\n");
    }
    int updated = 0;
    int tmp;
    int i;
    int sb[size];
    memcpy(sb, sb_orig, size*sizeof(int));
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
                if (sbb2>max_sbb) break;
                int pairs_new = sb_pairs[sbb2];
                //            printf("pairs_new=%d\n",pairs_new);
                if (pairs_new>max_pairs) break;
                int next_pairs_remaining = pairs_remaining - pairs_new;
                debug_printf("sbb2 = %d(%s)\n", sbb2, sbb_to_str[sbb2]);
                updated+=cacheCantSolve(&(n->next)[sbb2],sb+1, size-1,k, sbb2, pairs_without_this+pairs_new, next_pairs_remaining);
            }
            greater++;
        }
    }
    
    return updated;
}

void cache(int *sb, int size, int canSolve, int k, int pairs) {
    
#ifdef DEBUG
    printf("\nin cache for ");
    printSb(sb, size);
    printf(" cs=%d k=%d pairs=%d\n", canSolve, k, pairs);
    fflush(stdout);
#endif
    
    long long alloc_count_before = alloc_count;
    long long alloc_size_before = alloc_size;
    
    int updated;
    if (canSolve) {
        updated = cacheCanSolve(&sb_cache_root[k],sb,size,k, MAX_SBB, power3[k]);
    } else {
        updated = cacheCantSolve(&sb_cache_root[k],sb,size,k, MAX_SBB, pairs, power3[k]);
    }
    printf(" cache=%lld/%lld(%+lld/%+lld)", alloc_count, alloc_size, alloc_count-alloc_count_before, alloc_size-alloc_size_before);
    
#ifndef OPT
    if (updated == 0) {
        printf("\nupdated == 0 when caching result\n");
        fflush(stdout);
        exit(4);
    }
#endif
}

int checkCache(int *sb, int size, int k) {
    int i;
    struct node *n=&sb_cache_root[k];
    for (i=0; i <= size; i++) {
        //		printf("checkcache i=%d\n",i);
        if(n->next == cant_solve_marker){
            //			printf("checkcache can't solve: n->cant=%d k=%d\n", n->cant, k);
            return FALSE;
        } else {
            if (i==size) {
                if (n->next == can_solve_marker){
                    //				printf("checkcache can solve: n->can=%d k=%d\n", n->can, k);
                    return TRUE;
                    //                } else {
                    //                    if (n->not_fast) return MAYBE_SLOW;
                } else if (n->next == NULL) {
                    return MAYBE;
                } else if (n->next[0].next == NULL) {
                    return MAYBE;
                } else {
                    return TRUE;
                }
            }
            if (n->next == NULL || n->next == can_solve_marker) {
                //				printf("checkcache n->next is null, return 2");
                return MAYBE;
            }
            n = &(n->next)[sb[i]];
        }
    }
    //	printf("checkcache n->next is null, return 2");
    return MAYBE;
}

long long cant_solve_count=0;

int canSolveB(int *sb, int size, int k, clock_t parent_deadline){
#ifdef DEBUG1
    if(k>7) {
        printf("in canSolveB k=%d ", k);
        printSb(sb, size);
        printf("\n");
        fflush(stdout);
    }
#endif
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
#ifdef DEBUG1
        if(k>7) {
            printf("sbb=%d\n", sbb);
            printSb(&sbb, 1);
            printf("\n");
            fflush(stdout);
        }
#endif
        pairs_full+=sb_pairs[sbb];
#ifdef DEBUG1
        if(k>7) {
            printf("b_pairs[sbb]=%d\n", sb_pairs[sbb]);
            fflush(stdout);
        }
#endif
#ifdef DEBUG1
        if(k>7) {
            printf("pairs_full=%d\n", pairs_full);
            fflush(stdout);
        }
#endif
        // Unit Group Trivilaity Lemma: Unit Groups (1-1) do not affect solvability within information maximum and can be ignored
        if (sbb>1) {
            tmp[newsize++]=sbb;
            pairs+=sb_pairs[sbb];
#ifdef DEBUG1
            if(k>7) {
                printf("pairs=%d\n", pairs);
                fflush(stdout);
            }
#endif
        }
    }
    
    //    printf("in canSolveB in %d ", k);
    //    printSb(tmp, newsize);
    //    printf("\n");
    //    printf("pairs=%d\n", pairs);
    

#ifdef DEBUG1
    if(k>7) {
        printf("newsize = %d pairs_full= %d\n", newsize, pairs_full);
        fflush(stdout);
    }
#endif
    // check pairs
    if (pairs_full>power3[k]) return FALSE;
    if (newsize == 0) return TRUE; // if we only had unit groups
    
    size = newsize;
#ifdef DEBUG1
    if(k>7) {
        printf("before sort: ");
        printSb(tmp, size);
        printf("\n");
        fflush(stdout);
    }
#endif
    if (size>1) sort1(tmp, size);
#ifdef DEBUG
    printf("sorted: ");
    printSb(tmp, size);
    printf("\n");
    fflush(stdout);
#elif defined( DEBUG1 )
    if(k>7) {
        printf("sorted: ");
        printSb(tmp, size);
        printf("\n");
        fflush(stdout);
    }
#endif
    //check cache
    int ck = checkCache(tmp, size, k);
    //	printf("got from cache %d\n", ck);
    if (parent_deadline == CACHE_ONLY || ck == TRUE || ck == FALSE) {
        debug_printf("returning ck=%d\n", ck);
        return ck;
    }
    
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
    
    long long cant_solve_count_min = cant_solve_count + 10; // do not bail out until you make at least some progress
    clock_t deadline = 0;
    if (parent_deadline == NO_DEADLINE) {
        deadline = start + CLOCKS_PER_SEC * (100);
    } else {
//        if (start > ) return MAYBE;
//        int deadline_ratio = size;
        int deadline_ratio = DEADLINE_RATIO;
        if (parent_deadline > start && parent_deadline - start > CLOCKS_PER_SEC * MIN_DEADLINE * deadline_ratio) {
            deadline = start + ((parent_deadline - start) / deadline_ratio);
        } else {
            deadline = start + CLOCKS_PER_SEC * MIN_DEADLINE + 300;
        }
    }
    
    //    printf("k=%d parent_deadline=%llu start=%llu deadline=%llu\n", k, parent_deadline, start, deadline);
    
    int cont2=1;
    int skipped_some;
    int pass = 0;
    int max_solvable_maybe = 0;

    while (cont2) {
        pass++;
        clock_t max_time = (deadline - start) / CLOCKS_PER_SEC;
        skipped_some = 0;
        totalsplits=0;
        skiptop = 0;
        cont=1;
        splitincr[0] = size == 1 ? BY_MAGIC : (size<=3 ? BY_MAGIC2 : ((sb_pairs[tmp[0]] < pairs / 3) ? BY_MAGIC3 : BY_MAX));
//        splitincr[0] = size == 1 ? (DESC + BY_SP0) : (size<=3 ? BY_MAGIC2 : ((sb_pairs[tmp[0]] < pairs / 3) ? BY_MAGIC3 : BY_MAX));
        //    splitincr[0] = size == 1 ? BY_MAGIC : BY_MAX;
        //    splitincr[0] = size == 1 ? BY_MAGIC : ( (size>2 && sb_pairs[0] < pairs / 2) ? DESC + BY_MIN : BY_MAX);
        
        
        memset(splitindex, 0, size * sizeof(int));
        splitindex[0] = splitsarr[0]->size;
        
#ifdef DEBUG1
        clock_t t = clock();
        printf("solving in %d pass=%d ", k, pass);
        printf("deadline=%lu ", deadline>t ? (deadline - t + CLOCKS_PER_SEC) / CLOCKS_PER_SEC : 0);
        printSb(tmp, size);
        printf("\n");
        fflush(stdout);
#endif
        
        clock_t child_deadline = (parent_deadline == NO_DEADLINE && size == 1)? NO_DEADLINE : deadline;
        //    fflush(stdout);
        
        i = 0;
        int ck0,ck1,ck2;
        
        while(cont) {
            while(splitindex[i] == 0) {
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
                        if (parent_deadline == NO_DEADLINE) {
                            // double deadline
                            deadline += (deadline - start);
                        } else {
                            // do not bail out until you make at least some progress
                            if (pass>1 && cant_solve_count>=cant_solve_count_min) return MAYBE;
                        }
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
            
            // for identical groups avoid trying redundant permutations
            if (i>1 && tmp[i] == tmp[i-1]) { // do not do this for i==1 because it conflicts with skiptop
                int spi_1 = splitindex[i-1];
                int spi2_1 = (splitincr[i-1] & DESC) ?
                splitsarr[i-1]->ind[splitincr[i-1] & DESC_MASK][splitsarr[i-1]->size - 1 - spi_1] :
                splitsarr[i-1]->ind[splitincr[i-1] & DESC_MASK][spi_1];
                if (spi2 > spi2_1) continue;
            }
            
            int *s = splitsarr[i]->splitsl[spi2];
            
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
                    
#ifdef DEBUG
                    printSb(sb0, i+1);
                    printSb(sb1, 2*i+2);
                    printSb(sb2, i+1);
                    
                    debug_printf(" i=%d p0=%d p1=%d p2=%d\n", i, p0,p1,p2);
#endif
                    if ((p0 <= max_pairs_1) && (p1 <= max_pairs_1) && (p2 <= max_pairs_1)
                        && (cs0 = canSolveB(sb0, i+1, k_1, CACHE_ONLY)) && (cs2 = canSolveB(sb2, i+1, k_1, CACHE_ONLY)) && (cs1 = canSolveB(sb1, (i+1) * 2, k_1, CACHE_ONLY))
                        )
                    {
                        debug_printf("can solve\n");
                        if (i == size_1) {
                            // do not bail out until you make at least some progress
                            if (cant_solve_count>=cant_solve_count_min && (cs0 != TRUE || cs1 != TRUE || cs2 != TRUE)) {
                                clock_t t = clock();
                                // if we just fulfilled min progress, allow at least as much time as it took to get here
                                if (cant_solve_count == cant_solve_count_min) {
                                    clock_t new_deadline = t+(t-start);
                                    if (new_deadline>deadline) deadline = new_deadline;
                                }
                                if (t>deadline){
                                    if (parent_deadline != NO_DEADLINE) {
                                        return MAYBE;
                                    } else {
                                        cont=0;
                                        //                                    deadline=0;  // now do full solution
                                        // factorial deadline increase
                                        deadline+= pass * (deadline - start);
                                        break;
                                    }
                                }
                                if (t >= progress) {
                                    printf("still solving in %d pass=%d ", k, pass);
                                    printSb(tmp, size);
                                    printf(" trying ");
                                    printSb(sb0, size);
                                    printSb(sb1, size2);
                                    printSb(sb2, size);
                                    printf(" elapsed %lu/%lu left=%d/%d totalsplits=%llu\n", (t - start)/CLOCKS_PER_SEC, max_time, splitindex[0], splitsarr[0]->size, totalsplits);
                                    fflush(stdout);
                                    progress = t + PROGRESS_INTERVAL;
                                }
                            }
                            if ((cs0 = (cs0==MAYBE?canSolveB(sb0, i+1, k_1, child_deadline):cs0)) != TRUE) {
                                if (cs0 != FALSE)
                                    skipped_some = 1;
                            } else if ((cs2 = (cs2==MAYBE?canSolveB(sb2, i+1, k_1, child_deadline):cs2)) != TRUE) {
                                if (cs2 != FALSE)
                                    skipped_some = 1;
                            } else if((cs1 = (cs1==MAYBE?canSolveB(sb1, (i+1) * 2, k_1, deadline):cs1))  != TRUE) {
                                if (cs1!=FALSE)
                                    skipped_some = 1;
                            } else {
                                //can solve
                                canSolve=TRUE;
                                cont=0;
                                cont2=0;
                                break;
                            }
                        } else {
                            i++;
                            if (i>max_solvable_maybe) {
                                debug_printf("max_solvable_maybe=%d\n", max_solvable_maybe);
                                max_solvable_maybe = i;
                            }
                            splitindex[i] = splitsarr[i]->size;
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
        
    } else if (skipped_some) {
        return MAYBE;
    } else {
        cant_solve_count++;
        printf("can't solve ");
        if (max_solvable_maybe + 1 < size) {
            printf("size=%d/", size);
            size = max_solvable_maybe + 1;
            printf("%d ", size);
            // recompute pairs
            pairs = 0;
            for(i=0;i<size;i++) {
                pairs+=sb_pairs[sb[i]];
            }
        }
        printSb(tmp, size);
        printf(" in %d",k);
    }
    clock_t t = clock()-start;
    clock_t s = t/CLOCKS_PER_SEC;
    if (s>0)
        printf(" took %ld", s);
    else
        printf(" took 0.%03ld", t * 1000/CLOCKS_PER_SEC);
    printf(" totalsplits=%llu of %llu skiptop=%d pass=%d", totalsplits, maxsplits, skiptop, pass);
    
#ifdef DEBUG1
    fflush(stdout);
#endif
    cache(tmp, size, canSolve, k, pairs);
    //    fflush(stdout);
    printf("\n");
#ifdef DEBUG1
    fflush(stdout);
#endif
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
#ifdef DEBUG1
        printf("k=%d sa_can[%d]=%d, can solve\n", k, n, sa_can[n]);
#endif
        return 1;
    }
    if (sa_cant[n]>=k) {
#ifdef DEBUG1
        printf("k=%d sa_cant[%d]=%d, can't solve\n", k, n, sa_cant[n]);
#endif
        return 0;
    }
    int canSolve = 0;
    
    clock_t start = clock();
#ifdef DEBUG1
    printf("solving Sa(%d) in %d\n",n,k);
    fflush(stdout);
#endif
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
                if (canSolveB(sb,1,k-1,NO_DEADLINE)) {
                    canSolve = 1;
                    printf("can solve Sa(%d) in %d with following:",n,k);
                    printSa(n1);
                    printf(",");
                    printSa(n - n1);
                    printf(",");
                    printSb(sb,1);
#ifdef DEBUG1
                    fflush(stdout);
#endif
                }
            }
            n1--;
        }
    } else {
        printf("power3[%d]=%d can't solve should not be here\n", k, power3[k]);
        fflush(stdout);
        exit(5);
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
#ifdef DEBUG1
    fflush(stdout);
#endif
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


int magic2(int sbb, int spl[]) { // magic ration is .666 = 2/3
    int n1 = sbb_to_n1[sbb];
    int n2 = sbb_to_n2[sbb];
    int msum = ((n1+n2)*700+500)/1000;
    int magicm1 = min(n1, (n1*700+500)/1000);
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
    for (i=0; i<=k; i++) {
        sb_cache_root[k].next = NULL;
    }
    
    int n1, n2, sbb;
    sbb=0;
    sb_pairs[0]=0;
    sprintf(sbb_to_str[0],"0:0");
    int prod;
    //  printf("maxprod=%d\n", maxprod);
    for (prod =1; prod<=MAX_PROD;prod++) {
        max_sbb_for_pairs[prod] = sbb;
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
                    exit(6);
                }
                
                //		  		printf("sbb=%d\n", sbb);
                
                sbb_to_n1[sbb]=n1;
                sbb_to_n2[sbb]=n2;
                sb_pairs[sbb]=n1*n2;
                max_sbb_for_pairs[prod] = sbb;
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
        exit(7);
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
            exit(8);
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
    }
    if (maxsplits != MAX_SPLITS) {
        printf("expected MAX_SPLITS: %d - actual: %d\n", MAX_SPLITS, maxsplits);
        exit(9);
    }
    printf("\ninit done\n");
    fflush(stdout);
}
