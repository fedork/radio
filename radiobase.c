#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_K 10
#define MAX_N 200
#define MAX_SBB MAX_N*MAX_N/4
#define MAX_SPLITS (MAX_N/2) * (MAX_N/2 +2)
#define BY_MAGIC 0
#define BY_MAX 1
#define BY_SP0 2
#define BY_SP1 3
#define BY_SP2 4
#define BY_MIN 5
#define BY_MAGIC2 6
#define DESC 16
#define DESC_MASK 15
#define PROGRESS_INTERVAL CLOCKS_PER_SEC*60

typedef struct { int size; int splitsl[MAX_SPLITS][8]; int ind[7][MAX_SPLITS]; } splits;

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

typedef struct node { struct node *next; int can; int cant;} node_struct;

struct node sb_cache_root;

struct node *alloc_next(int arrsize){
	struct node * next;
	next = (struct node *)malloc((arrsize)*sizeof(struct node));
    if (next == NULL){
		printf("out of memory\n");
		exit(1);
	}
	int j;
	for (j=0; j<arrsize; j++) { 
		next[j].can = MAX_K+1; 
		next[j].cant = 0;
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
		int minSbb = size>1?sb[1]:1;
//		printf("minSbb = %d\n",minSbb);
		int sbb2;
		lesser = sbb_lesser[sbb];
		while(1) {
			sbb2 = *lesser;
			if (sbb2<minSbb) break;
//			printf("sbb2 = %d\n",sbb2);
			if (cacheCanSolve(&(n->next)[sbb2], sb+1, size-1,k, sbb2)) {
				updated=1;
			} else {
				break;
			}
			lesser++;
		}
	}
	return updated;
}

int cacheCantSolve(struct node *n, int* sb, int size, int k, int arrsize, int pairs){
	int updated =0;
//	printf("size=%d\n",size);
	if (size<1) {
		if (n->cant < k){
//			printf("cache n->cant was %d, now %d\n",n->cant, k);
			n->cant = k;
			updated = 1;
		}
	} else {
		if (n->next == NULL) {
//			printf("arrsize=%d\n",arrsize);
			n->next = alloc_next(arrsize+1);
		}
		int sbb = *sb;
//		printf("sbb=%d\n",sbb);
		int *greater;
		greater = sbb_greater[sbb];
		int pairs_without_this = pairs - sb_pairs[sbb];
//		printf("pairs_without_this=%d\n",pairs_without_this);
		int max_pairs = power3[k] - pairs_without_this;
//		printf("max_pairs=%d\n",max_pairs);
		while(1) {
			int sbb2 = *greater;
//			printf("sbb2=%d %s\n",sbb2,sbb_to_str[sbb2]);
			if (sbb2>arrsize) break;
			int pairs_new = sb_pairs[sbb2];
//			printf("pairs_new=%d\n",pairs_new);
			if (pairs_new>max_pairs) break;
			
			if (cacheCantSolve(&(n->next)[sbb2],sb+1, size-1,k, sbb2, pairs_without_this+pairs_new)) {
				updated = 1;
			} else {
				break;
			}
			greater++;
		}
	}
	return updated;
}

void cacheCantSolveOld(struct node *n, int* sb, int size, int k){
	int i;
	for (i=0; i <= size; i++) {
//		printf("cache i=%d\n",i);
		if (i==size) {
			if (n->cant < k){
//				printf("cache n->cant was %d, now %d\n",n->cant, k);
				n->cant = k;
			}
		} else {
			if (n->next == NULL) {
				int arrsize = (i==0?MAX_SBB:sb[i-1])+1;
//				printf("cache allocating %d", arrsize);
				n->next = alloc_next(arrsize);
			}
			n = &(n->next)[sb[i]];
		}
	}
}

void cache(int *sb, int size, int canSolve, int k, int pairs) {
//	printf("in cache ");
//	printSb(sb,size);
	struct node *n=&sb_cache_root;
	if (canSolve) {
//		printf("caching cansolve ");
//		printSb(sb,size);
//		printf(" k=%d\n", k);
		cacheCanSolve(n,sb,size,k, MAX_SBB);
	} else {
		cacheCantSolve(n,sb,size,k, MAX_SBB, pairs);
//		cacheCantSolve(n,sb,size,k);
	}
}

void cacheOld(int *sb, int size, int canSolve, int k) {
	int i;
//	printf("in cache ");
//	printSb(sb,size);
	struct node *n=&sb_cache_root;
	for (i=0; i <= size; i++) {
//		printf("cache i=%d\n",i);
		if(canSolve && n->can > k){
//			printf("cache n->can was %d, now %d\n",n->can, k);
			n->can = k;
		}
		if (i==size) {
			if (!canSolve && n->cant < k){
//				printf("cache n->cant was %d, now %d\n",n->cant, k);
				n->cant = k;
			}
		} else {
			if (n->next == NULL) {
				int arrsize = (i==0?MAX_SBB:sb[i-1])+1;
//				printf("cache allocating %d", arrsize);
				n->next = (struct node *)malloc((arrsize)*sizeof(struct node));
			    if (n->next == NULL){
					printf("out of memory\n");
					exit(1);
				}
				int j;
				for (j=0; j<arrsize; j++) { 
					(n->next)[j].can = MAX_K+1; 
					(n->next)[j].cant = 0;
					(n->next)[j].next = NULL;
				}
			}
			n = &(n->next)[sb[i]];
		}
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
			if ((i==size) && (n->can <= k)){
//				printf("checkcache can solve: n->can=%d k=%d\n", n->can, k);
				return 1;
			}
			if (n->next == NULL) {
//				printf("checkcache n->next is null, return 2");
				return 2;
			}
			n = &(n->next)[sb[i]];
		}
	}
//	printf("checkcache n->next is null, return 2");
	return 2;
}

int canSolveB(int *sb, int size, int k){
	int canSolve=0;
	int tmp[size];
	//todo: replace with memcpy
	int i;
	int pairs=0;
	int newsize=0;
	int sbb;
	for(i=0;i<size;i++) {
		sbb=sb[i];
		if (sbb) {
			tmp[newsize++]=sbb;
			pairs+=sb_pairs[sbb];
		}
	}

//    printf("in canSolveB in %d ", k);
//    printSb(tmp, newsize);
//    printf("\n");
//    printf("pairs=%d\n", pairs);
	
	// check pairs
	if (pairs<=1) return 1;
	if (pairs>power3[k]) return 0;
	
	size = newsize;

	sort1(tmp, size);
	
//    printf("sorted: ");
//    printSb(tmp, size);
//    printf("\n");
 
	//check cache
	int ck = checkCache(tmp, size, k);
	// 2 == unknown;
//	printf("got from cache %d\n", ck);
	if (ck != 2) {
		return ck;
	}
    
    // maybe remove - slows down?
//    if (size>1 && !canSolveB(tmp, size - 1, k)) {
//        return 0;
//    }
    
    //full search
	clock_t start = clock();
    clock_t progress = start + PROGRESS_INTERVAL;

	int sb0[size],sb2[size],sb1[size*2];
    int sb0p[size],sb2p[size],sb1p[size];
    
    long long maxsplits = 1;
    splits *splitsarr[size];
	for(i=0;i<size;i++){
		maxsplits *= (splitsarr[i] = &sbb_splits[tmp[i]])->size;
	}
	
    int splitincr[size];
    splitincr[0] = size == 1 ? BY_MAGIC : (size==2 ? BY_MAGIC2 : ((sb_pairs[tmp[0]] < pairs / 2) ? DESC + BY_MIN : BY_MAX));
//    splitincr[0] = size == 1 ? BY_MAGIC : BY_MAX;
//    splitincr[0] = size == 1 ? BY_MAGIC : ( (size>2 && sb_pairs[0] < pairs / 2) ? DESC + BY_MIN : BY_MAX);
    
	int splitindex[size];
	memset(splitindex, 0, size * sizeof(int));
	splitindex[0] = splitsarr[0]->size;
    
    printf("solving in %d maxsplits=%llu ", k, maxsplits);
    printSb(tmp, size);
    printf("\n");
    fflush(stdout);
    
	i = 0;
	int cont=1;
	int ck0,ck1,ck2;
    long long totalsplits=0;
    int spi, spi2;
	while(cont) {
//        printf("totalsplits=%llu\n", totalsplits);
        

		while(splitindex[i] == 0) {
			if (i==0) {
				// can't solve
				cont=0;
				break;
			}
			i--;
		}

//        printf("i=%d\n", i);
//        fflush(stdout);
		if (!cont) break;
		spi = --splitindex[i];
//        printf("spi=%d\n", spi);
//        printf("splitincr[i]=%d\n", splitincr[i]);
//        fflush(stdout);
        spi2 = (splitincr[i] & DESC) ?
                splitsarr[i]->ind[splitincr[i] & DESC_MASK][splitsarr[i]->size - 1 - spi] :
                splitsarr[i]->ind[splitincr[i] & DESC_MASK][spi];
//        printf("spi2=%d\n", spi2);
//        fflush(stdout);
        int *s = splitsarr[i]->splitsl[spi2];
        while (size > 1 && s[4]<k) { // don't do this for size==1 messes up counts
            int kk = s[4]++;
            if (
//                kk>=6 || // don't do this for large k
                canSolveB(s, 1, kk) && canSolveB(s+3, 1, kk) && canSolveB(s+1, 2, kk))
                s[4] = MAX_K;
            else
                s[5] = s[4];
        }
        if (s[5]<k)
        {
            totalsplits++;
            int p0 = sb0p[i] = sb_pairs[sb0[i] = s[0]] + (i>0?sb0p[i-1]:0);
            int p1 = sb1p[i] = sb_pairs[sb1[i*2] = s[1]] + sb_pairs[sb1[i*2+1] = s[2]] + (i>0?sb1p[i-1]:0);
            int p2 = sb2p[i] = sb_pairs[sb2[i] = s[3]] + (i>0?sb2p[i-1]:0);
//            printf("i=%d p0=%d p1=%d p2=%d\n", i, p0, p1, p2);
//            fflush(stdout);
//            int cs;
//            if (p0>p1) {
//                if (p0>p2) {
//                    cs = canSolveB(sb0, i+1, k-1);
//                    if (cs) {
//                        if (p1>p2) {
//                            cs = canSolveB(sb1, (i+1) * 2, k-1) && canSolveB(sb2, i+1, k-1);
//                        } else {
//                            cs = canSolveB(sb2, i+1, k-1) && canSolveB(sb1, (i+1) * 2, k-1);
//                        }
//                    }
//                } else {
//                    cs = canSolveB(sb2, i+1, k-1) && canSolveB(sb0, i+1, k-1) && canSolveB(sb1, (i+1) * 2, k-1);
//                }
//            } else { // p1 >= p0
//                if (p1 > p2) {
//                    cs = canSolveB(sb1, (i+1) * 2, k-1);
//                    if (cs) {
//                        if (p0>p2) {
//                            cs = canSolveB(sb0, i+1, k-1) && canSolveB(sb2, i+1, k-1);
//                        } else {
//                            cs = canSolveB(sb2, i+1, k-1) && canSolveB(sb0, i+1, k-1);
//                        }
//                    }
//                } else { // p2 >= p1 >= p0
//                    cs = canSolveB(sb2, i+1, k-1) && canSolveB(sb1, (i+1) * 2, k-1) && canSolveB(sb0, i+1, k-1);
//                }
//            }
//            printf("cs=%d\n", cs);
//            fflush(stdout);
//            if (cs)
            if ((p0 <= power3[k-1]) && (p1 <= power3[k-1]) && (p2 <= power3[k-1])) {
//                if (canSolveB(sb0, i+1, k-1) && canSolveB(sb2, i+1, k-1) && canSolveB(sb1, (i+1) * 2, k-1))
                {
                    if (i == size - 1) {
                        clock_t cur = clock();
                        if (cur >= progress) {
                            printf("still solving in %d ", k);
                            printSb(tmp, size);
                            printf(" trying ");
                            printSb(sb0, size);
                            printSb(sb1, size*2);
                            printSb(sb2, size);
                            printf(" elapsed %ld left=%d/%d totalsplits=%llu of %llu\n", (cur - start)/CLOCKS_PER_SEC, splitindex[0], splitsarr[0]->size, totalsplits, maxsplits);
                            fflush(stdout);
                            progress = cur + PROGRESS_INTERVAL;
                        }
                        
                        if (canSolveB(sb0, i+1, k-1) && canSolveB(sb2, i+1, k-1) && canSolveB(sb1, (i+1) * 2, k-1)) {
                            //can solve
                            canSolve=1;
                            cont=0;
                            break;
                        }
                    } else {
                        i++;
    //                    splitindex[i] = tmp[i]==tmp[i-1] ? splitindex[i-1]+1 : splitsarr[i]->size;
                        splitindex[i] = splitsarr[i]->size;

                        int e = sb_pairs[tmp[i]] / 3; // is 3 a good factor?
    //                    int e=1;
                        if (max(abs(p0-p1), max(abs(p0-p2), abs(p2-p1))) <= e) {
                            splitincr[i] = BY_MAX; // if routhly equal - split equally
                        } else {
                            if (p0 > p1) {
                                if (p1 > p2) {
                                    splitincr[i] = ( p0 - p1 > p1 - p2) ? BY_SP2 : (DESC + BY_SP0);
                                } else {
                                    if (p0 > p2) { // p0 > p2 >= p1
                                        splitincr[i] = ( p0 - p2 > p2 - p1) ? BY_SP2 : (DESC + BY_SP1);
                                    } else { // p2 >= p0 > p1
                                        splitincr[i] = ( p2 - p0 > p0 - p1) ? BY_SP0 : (DESC + BY_SP1);
                                    }
                                }
                            } else { // p1 >=p0
                                if (p0 > p2) {
                                    splitincr[i] = (p1 - p0 > p0 - p2) ? BY_SP1 : (DESC + BY_SP0);
                                } else {
                                    if (p1 > p2) { // p1 > p2 >= p0
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
		printf(" in %d  with following:",k);
		printSb(sb0,size);
		printSb(sb1,size*2);
		printSb(sb2,size);
//        printf("cansolve=true\n");
//        printf("totalsplits=%llu\n", totalsplits);
//        fflush(stdout);
        

        i=0;
        while(i < size && splitindex[i] == splitsarr[i]->size-1) i++;
//        printf("i=%d\n", i);
//        fflush(stdout);
        if (i<size) {
            printf(" suboptimal for i=%d splitincr=%d tried: ", i, splitincr[i]);
            int desc = splitincr[i] & DESC;
//            printf("desc=%d\n", desc);
            int *ind = splitsarr[i]->ind[splitincr[i] & DESC_MASK];
            for (spi = splitsarr[i]->size-1; spi> splitindex[i]; spi--) {
//                printf("spi=%d\n", spi);
                spi2 = desc ? ind[splitsarr[i]->size - 1 - spi] : ind[spi];
//                printf("spi2=%d\n", spi2);
                int *s = splitsarr[i]->splitsl[spi2];
                printf(" ");
                printSb(s,1);
                printSb(s+1,2);
                printSb(s+3,1);
                printf(";");
//                fflush(stdout);
            }
        }

	} else {
		printf("can't solve ");
		printSb(tmp, size);
		printf(" in %d",k);
	}
	printf(" took %ld totalsplits=%llu of %llu\n", (clock() - start)/CLOCKS_PER_SEC, totalsplits, maxsplits);
    fflush(stdout);
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
				if (canSolveB(sb,1,k-1)) {
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
		printf("power3[%d]=%d can't solve should not be here\n", k, power3[k]);
		exit(1);		
	}
	if(canSolve){
		sa_can[n] = k;
	} else {
		printf("can't solve Sa(%d) in %d",n,k);
		sa_cant[n]=k;
	}
		printf(" took %ld\n", (clock()-start)/CLOCKS_PER_SEC);					
	return canSolve;
} 

void indexSpl(int sbb, splits* s, int indexindex, int (*f)(int, int[])) {
    int splitsort[MAX_SPLITS][2];
    int e;
    int c = s->size;
    for(e = 0; e<c; e++) {
        splitsort[e][0] = f(sbb, s->splitsl[e]);
        splitsort[e][1] = e;
    }
    qsort(splitsort, c, sizeof(int)*2, desc); // sort by first element
    for(e = 0; e<c; e++) {
        s->ind[indexindex][e] = splitsort[e][1];
    }
}

int maxpairsraw(int sbb, int spl[]) {
    return max(sb_pairs[spl[0]], max(sb_pairs[spl[3]], sb_pairs[spl[1]] + sb_pairs[spl[2]]));
}

int minpairsraw(int sbb, int spl[]) {
    return min(sb_pairs[spl[0]], min(sb_pairs[spl[3]], sb_pairs[spl[1]] + sb_pairs[spl[2]]));
}

int maxpairs(int sbb, int spl[]) {
    return (1+maxpairsraw(sbb, spl)) * (1+sb_pairs[sbb]) - minpairsraw(sbb, spl);
}

int minpairs(int sbb, int spl[]) {
    return (1+minpairsraw(sbb, spl)) * (1+sb_pairs[sbb]) - maxpairsraw(sbb, spl);
}

int pairs2(int sbb, int spl[]) {
    return sb_pairs[spl[0]];
}

int pairs0(int sbb, int spl[]) {
    return sb_pairs[spl[3]];
}

int pairs1(int sbb, int spl[]) {
    return sb_pairs[spl[1]] + sb_pairs[spl[2]];
}

int magic(int sbb, int spl[]) {
    int n1 = sbb_to_n1[sbb];
    int n2 = sbb_to_n2[sbb];
    int msum = ((n1+n2)*577+999)/1000;
    int magicm1 = min(n1, (n1*577+999)/1000);
    int magicm2 = min(n2, msum-magicm1);
    int dx = spl[6] - magicm1;
    int dy = spl[7] - magicm2;
    return dx*dx + dy*dy;
}


int magic2(int sbb, int spl[]) {
    int n1 = sbb_to_n1[sbb];
    int n2 = sbb_to_n2[sbb];
    int msum = ((n1+n2)*666+500)/1000;
    int magicm1 = min(n1, (n1*666+500)/1000);
    int magicm2 = min(n2, msum-magicm1);
    int dx = spl[6] - magicm1;
    int dy = spl[7] - magicm2;
    return dx*dx + dy*dy;
}

void initSplits() {
    int sbb;
    int maxsplits = 0;
    for(sbb=1; sbb<=MAX_SBB; sbb++) {
        splits *s = &sbb_splits[sbb];
        printf("initializing splits for ");
        printSb(&sbb, 1);
        printf("\n");
        int n1 = sbb_to_n1[sbb];
        int n2 = sbb_to_n2[sbb];
        int c=0;
        int e=0;
        int m1,m2;
        int d1,d2;
        int bestmaxpairs = sb_pairs[sbb];
    
        // iterate outside-in on smaller side
//        m2=0;
//        d2=n2;
//        while(1) {

//            int min2 = min(m2, n2-m2); // smaller part of the  smaller side split
            // iterate inside-out on greater side
//            for(m1=(n1+1)/2, d1=(n1/2)*2+1; m1>=0 && m1 <= n1; m1=d1-m1, d1 = 2*n1+1-d1) {
//        for (m2=(n2+1)/2, d2=(n2 ^ 1)+1; m2>=0 && m2 <=n2; m2=d2-m2, d2=2*n2+1-d2) { // iterate middle-out on smaller side
            
            // iterate outside-in on greater side, unless n1==n2 in which case just 0 to m2
//            m1 = 0;
//            d1 = n1;
//            while (1) {
        for (m1=0; m1<=n1; m1++) {
            for (m2=(n2==n1?m1:n2); m2>=0; m2--) {
        
        //estimate optimal split - empirical. proof?
//        int midm2 = n2 * 577 / 1000;
//        int midm1 = n1 * 577 / 1000;
        
        // iterate by sum.

//        int summ;
//        for (summ = n1+n2; summ >= 0 ; summ--) {
//            printf("summ = %d\n", summ);
            
//            for (m2 = min(summ, n2); m2>=0 && (m1 = summ - m2)<=n1; m2--) {
//                printf("m2 = %d, m1 = %d\n", m2, m1);
                // EXPERIMENTAL - NEED RIGOROUS PROOF!!!!! MAY BE WRONG!!!
                // only consider splits on longer side where both halves are no less than the smaller part of the shorter side

//                if (m1 >= min2 && (n1-m1) >= min2)  //  !!!!!!  SEE ABOVE !!!!
                {
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
                    c++;
                }
                
//                if (n1==n2) {
//                    m1++;
//                    if (m1>m2) break;
//                } else {
//                    if (m1==(n1+1)/2) break;
//                    m1=d1-m1;
//                    d1=2*n1+1-d1;
//                }
            }
//            if (m2==(n2+1)/2) break;
//            m2=d2-m2;
//            d2=2*n2+1-d2;
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
        indexSpl(sbb, s, BY_MIN, minpairs);
        indexSpl(sbb, s, BY_MAGIC2, magic2);
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
  sprintf(sbb_to_str[0],"0-0");
  int prod;
  int maxprod = (MAX_N/2)*(MAX_N - MAX_N/2);
//  printf("maxprod=%d\n", maxprod);
  for (prod =1; prod<=maxprod;prod++) {
//  	printf("prod=%d\n", prod);
  	for (n1=min(prod,MAX_N-1), n2=1; n1>=n2; n1--) {
  		if (n1>0) n2 = prod/n1;
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
		  		sprintf(sbb_to_str[sbb],"%d-%d",n1,n2);
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
    
  initSplits();
}
