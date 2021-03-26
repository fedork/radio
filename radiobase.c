#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_K 10
#define MAX_N 300
#define MAX_SBB MAX_N*MAX_N/4
#define MAX_SPLITS (MAX_N/2 + 1) * (MAX_N/2 +2)

typedef struct { int size; int splitsl[MAX_SPLITS][6]; } splits;

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

//sorting lifted from JDK

void swap(int* x, int a, int b) {
	int t = x[a];
	x[a] = x[b];
	x[b] = t;
    }

void vecswap(int *x, int a, int b, int n) {
	int i;
	for (i=0; i<n; i++, a++, b++)
	    swap(x, a, b);
    }

    /**
     * Returns the index of the median of the three indexed integers.
     */
int med3(int *x, int a, int b, int c) {
	return (x[a] < x[b] ?
		(x[b] < x[c] ? b : x[a] < x[c] ? c : a) :
		(x[b] > x[c] ? b : x[a] > x[c] ? c : a));
    }
    
int min(int a,int b){
	return a<b?a:b;
}
    /**
     * Sorts the specified sub-array of integers into desc order.
     */
void sort1(int *x, int off, int len) {
	// Insertion sort on smallest arrays
	int i,j;
	if (len < 7) {
	    for (i=off; i<len+off; i++)
		for (j=i; j>off && x[j-1]<x[j]; j--)
		    swap(x, j, j-1);
	    return;
	}

	// Choose a partition element, v
	int m = off + (len >> 1);       // Small arrays, middle element
	if (len > 7) {
	    int l = off;
	    int n = off + len - 1;
	    if (len > 40) {        // Big arrays, pseudomedian of 9
		int s = len/8;
		l = med3(x, l,     l+s, l+2*s);
		m = med3(x, m-s,   m,   m+s);
		n = med3(x, n-2*s, n-s, n);
	    }
	    m = med3(x, l, m, n); // Mid-size, med of 3
	}
	int v = x[m];

	// Establish Invariant: v* (<v)* (>v)* v*
	int a = off, b = a, c = off + len - 1, d = c;
	while(1) {
	    while (b <= c && x[b] >= v) {
		if (x[b] == v)
		    swap(x, a++, b);
		b++;
	    }
	    while (c >= b && x[c] <= v) {
		if (x[c] == v)
		    swap(x, c, d--);
		c--;
	    }
	    if (b > c)
		break;
	    swap(x, b++, c--);
	}

	// Swap partition elements back to middle
	int s, n = off + len;
	s = min(a-off, b-a  );  vecswap(x, off, b-s, s);
	s = min(d-c,   n-d-1);  vecswap(x, b,   n-s, s);

	// Recursively sort non-partition-elements
	if ((s = b-a) > 1)
	    sort1(x, off, s);
	if ((s = d-c) > 1)
	    sort1(x, n-s, s);
    }
//end sorting lifted from JDK

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
//	printf("pairs=%d\n", pairs);
	
	// check pairs
	if (pairs<=1) return 1;
	if (pairs>power3[k]) return 0;
	
	size = newsize;

	sort1(tmp, 0, size);
	
	//check cache
	int ck = checkCache(tmp, size, k);
	// 2 == unknown;
//	printf("got from cache %d\n", ck); 
	if (ck != 2) {
		return ck;
	}
    
    // maybe remove - slows down?
    if (size>1 && !canSolveB(tmp, size - 1, k)) {
        return 0;
    }
    
    //full search
	clock_t start = clock();

	int sb0[size],sb2[size],sb1[size*2];
	splits *splitsarr[size];
	for(i=0;i<size;i++){
		splitsarr[i] = &sbb_splits[tmp[i]];
	}
	
	int splitindex[size];
	memset(splitindex, 0, size * sizeof(int));
	splitindex[0] = splitsarr[0]->size;
    
    printf("solving in %d ", k);
    printSb(tmp, size);
    printf("\n");
    
	i = 0;
	int cont=1;
	int ck0,ck1,ck2;
    long long totalsplits=0;
	while(cont) {
		while(splitindex[i] == 0) {
			if (i==0) {
				// can't solve
				cont=0;
				break;
			}
			i--;
		}
		if (!cont) break;
		int spi = --splitindex[i];
		int *s = splitsarr[i]->splitsl[spi];
        while (s[4]<k) {
            int kk = s[4]++;
            if (canSolveB(s, 1, kk) && canSolveB(s+3, 1, kk) && canSolveB(s+1, 2, kk))
                s[4] = MAX_K;
            else
                s[5] = s[4];
        }
        if (s[5]<k)
        {
            sb0[i] = s[0];
            sb1[i*2] = s[1];
            sb1[i*2+1] = s[2];
            sb2[i] = s[3];
            totalsplits++;
            if (canSolveB(sb1, (i+1) * 2, k-1))
                if (canSolveB(sb2, i+1, k-1))
                    if (canSolveB(sb0, i+1, k-1)) {
                        if (i == size - 1) {
                            //can solve
                            canSolve=1;
                            cont=0;
                            break;
                        } else {
                            i++;
                            splitindex[i] = tmp[i]==tmp[i-1] ? splitindex[i-1]+1 : splitsarr[i]->size;
                        }
                    }
        }
	}

	if (canSolve) {
		printf("can solve ");
		printSb(tmp, size);
		printf(" in %d  with following:",k);
		printSb(sb0,size);
		printSb(sb1,size*2);
		printSb(sb2,size);
	} else {
		printf("can't solve ");
		printSb(tmp, size);
		printf(" in %d",k);
	}
	printf(" took %ld totalsplits=%llu\n", (clock() - start)/CLOCKS_PER_SEC, totalsplits);
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

void initSplits() {
    int sbb;
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
        
        
        // SPLITS ARE ITERATED in reverse order, so generate them in the order OPPOSITE to intended iteration
        // iterate outside-in on smaller side
        m2=0;
        d2=n2;
        while(1) {

            // iterate inside-out on greater side, unless n1==n2 in which case just 0 to m2
            for(m1=(n1+1)/2, d1=(n1/2)*2+1; m1>=0 && m1 <= n1; m1=d1-m1, d1 = 2*n1+1-d1) {
//        for (m2=(n2+1)/2, d2=(n2 ^ 1)+1; m2>=0 && m2 <=n2; m2=d2-m2, d2=2*n2+1-d2) { // iterate middle-out on smaller side
            
            // iterate outside-in on greater side, unless n1==n2 in which case just 0 to m2
//            m1 = 0;
//            d1 = n1;
//            while (1) {
//        for (m1=0; m1<=n1; m1++) {
//            for (m2=(n2==n1?m1:n2); m2>=0; m2--) {
                s->splitsl[c][0]=getSbb(m1, m2);
                s->splitsl[c][1]=getSbb(n1-m1, m2);
                s->splitsl[c][2]=getSbb(m1, n2-m2);
                s->splitsl[c][3]=getSbb(n1-m1, n2-m2);
                int maxpairs = max(sb_pairs[s->splitsl[c][0]],max(sb_pairs[s->splitsl[c][3]], sb_pairs[s->splitsl[c][1]] + sb_pairs[s->splitsl[c][2]]));
                int k=0;
                while (power3[k]<=maxpairs) k++;
                s->splitsl[c][4] = s->splitsl[c][5] = k-1;
                c++;
                
//                if (n1==n2) {
//                    m1++;
//                    if (m1>m2) break;
//                } else {
//                    if (m1==(n1+1)/2) break;
//                    m1=d1-m1;
//                    d1=2*n1+1-d1;
//                }
            }
            if (m2==(n2+1)/2) break;
            m2=d2-m2;
            d2=2*n2+1-d2;
        }
        if (c>MAX_SPLITS) {
            printf("c=%d MAX_SPLITS=%d\n",c,MAX_SPLITS);
            exit(1);
        }

        s->size = c;
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
				sort1(sbb_lesser[sbb],0, c);
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
