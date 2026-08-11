/* Emit the exact solvable 4-part table at one level.
 *
 * The exact pair and triple tables are necessary gates and eliminate most
 * combinations before the solver is called.  Stdout contains only rows of
 * a provenance comment block followed by rows of eight non-negative integers.
 *
 *   tools/build_radio.py -O3 -DMAX_K=4 -DMAX_N=128 tools/quadtab.c -o quadtab
 *   ./quadtab 4 16 pairs_k4.txt triples_k4.txt
 */
#include "../radiobase.c"
#include "read_int_table.h"
#include <unistd.h>

#define AXIS 64
#define MAX_SINGLE_PARTS 512

static unsigned char pair_ok[AXIS * AXIS][AXIS * AXIS / 8];
static int single_id[AXIS][AXIS], single_count;
static unsigned char *triple_ok;

static int grid_id(int n, int m) { return n * AXIS + m; }

static void set_pair(int n1,int m1,int n2,int m2) {
    int a=grid_id(n1,m1), b=grid_id(n2,m2);
    pair_ok[a][b>>3] |= (unsigned char)(1u<<(b&7));
    pair_ok[b][a>>3] |= (unsigned char)(1u<<(a&7));
}

static int get_pair(int n1,int m1,int n2,int m2) {
    int a=grid_id(n1,m1), b=grid_id(n2,m2);
    return (pair_ok[a][b>>3]>>(b&7))&1;
}

static size_t triple_index(int a,int b,int c) {
    if(a>b){int t=a;a=b;b=t;} if(b>c){int t=b;b=c;c=t;} if(a>b){int t=a;a=b;b=t;}
    return ((size_t)a*single_count+b)*single_count+c;
}

static void set_triple(int n1,int m1,int n2,int m2,int n3,int m3) {
    int a=single_id[n1][m1], b=single_id[n2][m2], c=single_id[n3][m3];
    if(a>=0&&b>=0&&c>=0) triple_ok[triple_index(a,b,c)]=1;
}

static int get_triple_by_index(int a,int b,int c) {
    return triple_ok[triple_index(a,b,c)];
}

int main(int argc,char **argv) {
    if(argc!=5){
        fprintf(stderr,"usage: %s <k> <axis-limit> <pair-table> <triple-table>\n",argv[0]);
        return 2;
    }
    int level=atoi(argv[1]), limit=atoi(argv[2]);
    if(limit>=AXIS){fprintf(stderr,"axis limit too large\n");return 2;}
    /* See tripletab.c: provenance belongs in the table, engine chatter does not. */
    radio_print_provenance();

    memset(single_id,0xff,sizeof(single_id));
    FILE *pf=fopen(argv[3],"r");
    if(!pf){fprintf(stderr,"cannot open %s\n",argv[3]);return 2;}
    int row[8];
    while(radio_read_int_row(pf,row,4)) set_pair(row[0],row[1],row[2],row[3]);
    fclose(pf);

    int table_fd=dup(STDOUT_FILENO);
    FILE *table=table_fd>=0?fdopen(table_fd,"w"):NULL;
    if(!table||!freopen("/dev/null","w",stdout)){
        fprintf(stderr,"cannot redirect solver output\n");return 2;
    }
    init();
    int pn[MAX_SINGLE_PARTS],pm[MAX_SINGLE_PARTS],np=0;
    for(int n=1;n<=limit;n++) for(int m=1;m<=limit;m++){
        int sb=getSbb(n,m);
        if(canSolveB(&sb,1,level,NO_DEADLINE)==TRUE){
            single_id[n][m]=np;pn[np]=n;pm[np]=m;np++;
        }
    }
    fprintf(stderr,"single parts solvable at k=%d: %d\n",level,np);
    triple_ok=calloc((size_t)np*np*np,1);
    if(!triple_ok){fprintf(stderr,"cannot allocate triple table\n");return 2;}
    FILE *tf=fopen(argv[4],"r");
    if(!tf){fprintf(stderr,"cannot open %s\n",argv[4]);return 2;}
    while(radio_read_int_row(tf,row,6))
        set_triple(row[0],row[1],row[2],row[3],row[4],row[5]);
    fclose(tf);

    long long total=0,triple_feasible=0,solvable=0;
    for(int i=0;i<np;i++) for(int j=i;j<np;j++) for(int q=j;q<np;q++)
        for(int r=q;r<np;r++){
            total++;
            if(!get_pair(pn[i],pm[i],pn[j],pm[j])
               ||!get_pair(pn[i],pm[i],pn[q],pm[q])
               ||!get_pair(pn[i],pm[i],pn[r],pm[r])
               ||!get_pair(pn[j],pm[j],pn[q],pm[q])
               ||!get_pair(pn[j],pm[j],pn[r],pm[r])
               ||!get_pair(pn[q],pm[q],pn[r],pm[r])) continue;
            if(!get_triple_by_index(i,j,q)||!get_triple_by_index(i,j,r)
               ||!get_triple_by_index(i,q,r)||!get_triple_by_index(j,q,r)) continue;
            triple_feasible++;
            int sb[4]={getSbb(pn[i],pm[i]),getSbb(pn[j],pm[j]),
                       getSbb(pn[q],pm[q]),getSbb(pn[r],pm[r])};
            sort1(sb,4);
            if(canSolveB(sb,4,level,NO_DEADLINE)==TRUE){
                solvable++;
                fprintf(table,"%d %d %d %d %d %d %d %d\n",
                        pn[i],pm[i],pn[j],pm[j],pn[q],pm[q],pn[r],pm[r]);
            }
        }
    fprintf(stderr,
            "quads solvable %lld of %lld triple-feasible, %lld total "
            "(triple gate %.1f%%, solvable %.1f%% of gated)\n",
            solvable,triple_feasible,total,total?100.0*triple_feasible/total:0.0,
            triple_feasible?100.0*solvable/triple_feasible:0.0);
    fclose(table);free(triple_ok);
    return 0;
}
