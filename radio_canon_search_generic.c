#define MAX_K 9
#define MAX_N 490
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
static int solver_printf(const char *fmt, ...) { return 0; }
#define printf solver_printf
#include "radiobase.c"
#undef printf

#define MAX_STATE_SIZE 5120
#define MAX_TREE_NODES 4000000
#define POOL_CHUNK_NODES 20480
#define MAX_POOL_CHUNKS ((MAX_TREE_NODES + POOL_CHUNK_NODES - 1) / POOL_CHUNK_NODES)
#define HASH_SIZE 262144
#define MAX_MEMO 8000000

typedef struct TreeNode { int k,size,canonical,stopk,child_count; int sb[MAX_STATE_SIZE]; int split_m[MAX_STATE_SIZE*2]; struct TreeNode *child[3]; } TreeNode;
static TreeNode *pool_chunks[MAX_POOL_CHUNKS];
static int pool_used=0; static int pool_chunk_count=0; static int TARGET_K=3;
static TreeNode* new_node(void){
    if(pool_used>=MAX_TREE_NODES){fprintf(stderr,"out of nodes\n"); exit(2);}
    int idx=pool_used++;
    int chunk=idx/POOL_CHUNK_NODES;
    int off=idx%POOL_CHUNK_NODES;
    if(chunk>=pool_chunk_count){
        pool_chunks[chunk]=(TreeNode*)calloc(POOL_CHUNK_NODES,sizeof(TreeNode));
        if(!pool_chunks[chunk]){ fprintf(stderr,"out of memory allocating node pool chunk\n"); exit(2); }
        pool_chunk_count=chunk+1;
    }
    TreeNode* n=&pool_chunks[chunk][off];
    memset(n,0,sizeof(*n));
    return n;
}

typedef struct Memo { uint64_t hash; int used; int result; int next; int k,size; int sb[40]; } Memo;
static Memo memo[MAX_MEMO]; static int memo_used=0; static int buckets[HASH_SIZE];
static uint64_t state_hash(int *sb,int size,int k){ uint64_t h=1469598103934665603ULL ^ (uint64_t)k; for(int i=0;i<size;i++){ h ^= (uint64_t)sb[i] + 0x9e3779b97f4a7c15ULL + (h<<6) + (h>>2); } return h; }
static int memo_get(int *sb,int size,int k,int *result){ if(size>40) return 0; uint64_t h=state_hash(sb,size,k); int b=h & (HASH_SIZE-1); for(int e=buckets[b]-1; e!=-1; e=memo[e].next){ if(memo[e].used && memo[e].hash==h && memo[e].k==k && memo[e].size==size && memcmp(memo[e].sb,sb,sizeof(int)*size)==0){ *result=memo[e].result; return 1; } } return 0; }
static void memo_put(int *sb,int size,int k,int result){ if(size>40 || memo_used>=MAX_MEMO) return; uint64_t h=state_hash(sb,size,k); int b=h & (HASH_SIZE-1); int e=memo_used++; memo[e].used=1; memo[e].hash=h; memo[e].k=k; memo[e].size=size; memo[e].result=result; memcpy(memo[e].sb,sb,sizeof(int)*size); memo[e].next=buckets[b]-1; buckets[b]=e+1; }

static int normalize_state(int *src,int size,int *dst){ int n=0; for(int i=0;i<size;i++) if(src[i]>0) dst[n++]=src[i]; sort1(dst,n); return n; }
static int binom_int(int n,int k){ if(k<0||k>n) return 0; if(k==0||k==n) return 1; if(k>n-k) k=n-k; long long r=1; for(int i=1;i<=k;i++) r=r*(n-k+i)/i; return (int)r; }
static void make_u_freq(int k,int *freq){ memset(freq,0,sizeof(int)*(MAX_N+1)); int len=1<<k; for(int j=1;j<=len;j++){ int r; if(j==1) r=0; else {r=0; int x=j-1; while(x>0){r++; x>>=1;}} int val=0; for(int i=0;i<=k-r;i++) val += binom_int(k,i); if(val<=MAX_N) freq[val]++; } }
static int is_canonical_state(int *sb,int size,int k){ int freq[MAX_N+1]; make_u_freq(k,freq); for(int i=0;i<size;i++){ int sbb=sb[i], n1=sbb_to_n1[sbb], n2=sbb_to_n2[sbb]; if(n2!=1) return FALSE; if(n1<0||n1>MAX_N) return FALSE; if(freq[n1]<=0) return FALSE; freq[n1]--; } return TRUE; }
static void copy_state(TreeNode *node,int *sb,int size,int k){ node->k=k; node->size=size; memcpy(node->sb,sb,sizeof(int)*size); }
static int solvable_exact(int *sb,int size,int k){ return canSolveB(sb,size,k,CACHE_ONLY)!=FALSE && canSolveB(sb,size,k,NO_DEADLINE)==TRUE; }

static int search_state(int *in_sb,int in_size,int k,TreeNode **out_node){
    int sb[MAX_STATE_SIZE]; int size=normalize_state(in_sb,in_size,sb); int memo_res;
    if(memo_get(sb,size,k,&memo_res) && memo_res==0) return FALSE;
    TreeNode* node=new_node(); copy_state(node,sb,size,k); *out_node=node;
    if(size==0){ node->canonical=TRUE; node->stopk=k; return TRUE; }
    if(k>=TARGET_K && is_canonical_state(sb,size,k)){ node->canonical=TRUE; node->stopk=k; return TRUE; }
    if(k<=TARGET_K){ memo_put(sb,size,k,0); return FALSE; }
    int n[MAX_STATE_SIZE*2], m[MAX_STATE_SIZE*2], sb0[MAX_STATE_SIZE], sb2[MAX_STATE_SIZE], sb1[MAX_STATE_SIZE*2];
    for(int i=0;i<size;i++){ n[i*2]=sbb_to_n1[sb[i]]; n[i*2+1]=sbb_to_n2[sb[i]]; m[i*2]=0; m[i*2+1]=0; }
    int j=0; m[0]=1+n[0];
    while(1){
        while(m[j]==0){ if(j==0){ memo_put(sb,size,k,0); return FALSE; } j--; }
        m[j]--;
        if(j==size*2-1){
            for(int i=0;i<size;i++){
                sb0[i]=getSbb(m[i*2],m[i*2+1]);
                sb2[i]=getSbb(n[i*2]-m[i*2], n[i*2+1]-m[i*2+1]);
                sb1[i*2]=getSbb(m[i*2], n[i*2+1]-m[i*2+1]);
                sb1[i*2+1]=getSbb(n[i*2]-m[i*2], m[i*2+1]);
            }
            if(solvable_exact(sb0,size,k-1) && solvable_exact(sb1,size*2,k-1) && solvable_exact(sb2,size,k-1)){
                int snap=pool_used; TreeNode *c0=NULL,*c1=NULL,*c2=NULL;
                if(search_state(sb0,size,k-1,&c0) && search_state(sb1,size*2,k-1,&c1) && search_state(sb2,size,k-1,&c2)){
                    memcpy(node->split_m,m,sizeof(int)*size*2); node->child[0]=c0; node->child[1]=c1; node->child[2]=c2; node->child_count=3; memo_put(sb,size,k,1); return TRUE; }
                pool_used=snap;
            }
        } else { j++; m[j]=n[j]+1; }
    }
}

static void print_state(FILE* f,int *sb,int size){ if(size==0){ fprintf(f,"0:0"); return;} for(int i=0;i<size;i++){ if(i) fprintf(f,","); fprintf(f,"%d:%d", sbb_to_n1[sb[i]], sbb_to_n2[sb[i]]);} }
static void print_tree(FILE* f,TreeNode *n,int indent){ for(int i=0;i<indent;i++) fprintf(f,"  "); print_state(f,n->sb,n->size); fprintf(f," @%d",n->k); if(n->canonical){ fprintf(f," [canonical U_%d]\n",n->stopk); return;} fprintf(f," --["); for(int i=0;i<n->size;i++){ if(i) fprintf(f,","); fprintf(f,"%d:%d", n->split_m[i*2], n->split_m[i*2+1]); } fprintf(f,"]-->\n"); for(int c=0;c<3;c++) print_tree(f,n->child[c],indent+1); }

int main(int argc,char**argv){
    init();
    if(argc<5){ fprintf(stderr,"usage: %s target_k k n1 m1 [n2 m2 ...]\n", argv[0]); return 1; }
    TARGET_K=atoi(argv[1]); int k=atoi(argv[2]); int size=(argc-3)/2; int sb[MAX_STATE_SIZE];
    for(int i=0;i<size;i++) sb[i]=getSbb(atoi(argv[3+i*2]), atoi(argv[4+i*2]));
    TreeNode* root=NULL; if(!search_state(sb,size,k,&root)){ fprintf(stderr,"NO_CANONICAL_TREE\n"); return 2; }
    print_tree(stdout,root,0); return 0; }
