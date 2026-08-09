/* Filter an existing split-feature dataset through exact 3- and 4-part tables.
 *
 * The feature TSV has the form emitted by the heuristic lab's dumpz tool:
 * parent-state, numeric features, take-vector.  The labels file has the same
 * rows in the evaluator's numeric format.  Surviving label rows are written to
 * stdout, so the existing rank evaluator can measure the marginal effect of
 * the triple filter and the optional quad filter without regenerating features.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AXIS 64
#define MAX_PARTS 16

static const int p4[] = {0,16,15,12,10,9,7,6,5,5,4,3,3,2,2,2,1};
static int single_id[AXIS][AXIS], single_count;
static unsigned char *triple_ok, *quad_ok;

static size_t triple_index(int a, int b, int c) {
    if (a > b) { int t = a; a = b; b = t; }
    if (b > c) { int t = b; b = c; c = t; }
    if (a > b) { int t = a; a = b; b = t; }
    return ((size_t)a * single_count + b) * single_count + c;
}

static void set_triple(int n1, int m1, int n2, int m2, int n3, int m3) {
    int a = single_id[n1][m1], b = single_id[n2][m2], c = single_id[n3][m3];
    if (a >= 0 && b >= 0 && c >= 0) triple_ok[triple_index(a,b,c)] = 1;
}

static int get_triple(int n1, int m1, int n2, int m2, int n3, int m3) {
    if (n1 == 0 || m1 == 0 || n2 == 0 || m2 == 0 || n3 == 0 || m3 == 0)
        return 1;
    if (n1 >= AXIS || m1 >= AXIS || n2 >= AXIS || m2 >= AXIS
        || n3 >= AXIS || m3 >= AXIS) return 0;
    int a = single_id[n1][m1], b = single_id[n2][m2], c = single_id[n3][m3];
    return a >= 0 && b >= 0 && c >= 0 && triple_ok[triple_index(a,b,c)];
}

static size_t quad_index(int a, int b, int c, int d) {
    int v[4]={a,b,c,d};
    for(int i=1;i<4;i++){int x=v[i],j=i-1;while(j>=0&&v[j]>x){v[j+1]=v[j];j--;}v[j+1]=x;}
    return (((size_t)v[0]*single_count+v[1])*single_count+v[2])*single_count+v[3];
}

static void set_quad(int n1,int m1,int n2,int m2,int n3,int m3,int n4,int m4) {
    int a=single_id[n1][m1],b=single_id[n2][m2];
    int c=single_id[n3][m3],d=single_id[n4][m4];
    if(a<0||b<0||c<0||d<0)return;
    size_t bit=quad_index(a,b,c,d);
    quad_ok[bit>>3]|=(unsigned char)(1u<<(bit&7));
}

static int get_quad(int n1,int m1,int n2,int m2,int n3,int m3,int n4,int m4) {
    if(n1==0||m1==0||n2==0||m2==0||n3==0||m3==0||n4==0||m4==0)return 1;
    if(n1>=AXIS||m1>=AXIS||n2>=AXIS||m2>=AXIS||n3>=AXIS||m3>=AXIS
       ||n4>=AXIS||m4>=AXIS)return 0;
    int a=single_id[n1][m1],b=single_id[n2][m2];
    int c=single_id[n3][m3],d=single_id[n4][m4];
    if(a<0||b<0||c<0||d<0)return 0;
    size_t bit=quad_index(a,b,c,d);
    return (quad_ok[bit>>3]>>(bit&7))&1;
}

static int parse_parts(char *text, int *n, int *m) {
    int count = 0;
    char *p = text;
    while (sscanf(p, "%d:%d", &n[count], &m[count]) == 2) {
        if (++count >= MAX_PARTS) break;
        p = strchr(p, ',');
        if (!p) break;
        p++;
    }
    return count;
}

static int child_survives(int *n, int *m, int count) {
    for (int i = 0; i < count; i++) for (int j = i + 1; j < count; j++)
        for (int q = j + 1; q < count; q++)
            if (!get_triple(n[i],m[i],n[j],m[j],n[q],m[q])) return 0;
    if(quad_ok) for(int i=0;i<count;i++) for(int j=i+1;j<count;j++)
        for(int q=j+1;q<count;q++) for(int r=q+1;r<count;r++)
            if(!get_quad(n[i],m[i],n[j],m[j],n[q],m[q],n[r],m[r])) return 0;
    return 1;
}

static int split_survives(char *state, char *take) {
    int N[MAX_PARTS], M[MAX_PARTS], A[MAX_PARTS], B[MAX_PARTS];
    int c0n[MAX_PARTS], c0m[MAX_PARTS], c2n[MAX_PARTS], c2m[MAX_PARTS];
    int c1n[2*MAX_PARTS], c1m[2*MAX_PARTS];
    int parts = parse_parts(state,N,M);
    if (parse_parts(take,A,B) != parts) return 0;
    for (int i = 0; i < parts; i++) {
        c2n[i]=A[i]; c2m[i]=B[i];
        c0n[i]=N[i]-A[i]; c0m[i]=M[i]-B[i];
        c1n[2*i]=A[i]; c1m[2*i]=M[i]-B[i];
        c1n[2*i+1]=N[i]-A[i]; c1m[2*i+1]=B[i];
    }
    return child_survives(c0n,c0m,parts)
        && child_survives(c2n,c2m,parts)
        && child_survives(c1n,c1m,2*parts);
}

static int final_integer(const char *line) {
    const char *p = line + strlen(line);
    while (p > line && (p[-1] == '\n' || p[-1] == '\r' || p[-1] == ' ' || p[-1] == '\t')) p--;
    const char *q = p;
    while (q > line && q[-1] >= '0' && q[-1] <= '9') q--;
    return atoi(q);
}

int main(int argc, char **argv) {
    if (argc != 4 && argc != 5) {
        fprintf(stderr, "usage: %s <triple-table> [quad-table] <feature-tsv> <labels>\n", argv[0]);
        return 2;
    }
    memset(single_id, 0xff, sizeof(single_id));
    for (int n=1; n<(int)(sizeof(p4)/sizeof(p4[0])); n++)
        for (int m=1; m<(int)(sizeof(p4)/sizeof(p4[0])); m++)
            if (n <= p4[m]) single_id[n][m] = single_count++;
    triple_ok = calloc((size_t)single_count * single_count * single_count, 1);
    if (!triple_ok) return 2;

    FILE *f = fopen(argv[1], "r");
    if (!f) { fprintf(stderr, "cannot open %s\n", argv[1]); return 2; }
    int a,b,c,d,e,g;
    while (fscanf(f,"%d %d %d %d %d %d",&a,&b,&c,&d,&e,&g)==6)
        set_triple(a,b,c,d,e,g);
    fclose(f);

    int input=2;
    if(argc==5){
        size_t entries=(size_t)single_count*single_count*single_count*single_count;
        quad_ok=calloc((entries+7)/8,1);
        if(!quad_ok){fprintf(stderr,"cannot allocate quad table\n");return 2;}
        f=fopen(argv[input++],"r");
        if(!f){fprintf(stderr,"cannot open %s\n",argv[input-1]);return 2;}
        int h,j;
        while(fscanf(f,"%d %d %d %d %d %d %d %d",&a,&b,&c,&d,&e,&g,&h,&j)==8)
            set_quad(a,b,c,d,e,g,h,j);
        fclose(f);
    }

    FILE *features = fopen(argv[input], "r"), *labels = fopen(argv[input+1], "r");
    if (!features || !labels) {
        fprintf(stderr, "cannot open feature or label input\n");
        return 2;
    }
    char feature_line[2048], label_line[2048];
    long long rows=0, kept=0, winners=0, winners_kept=0;
    while (fgets(feature_line,sizeof(feature_line),features)) {
        if (!fgets(label_line,sizeof(label_line),labels)) {
            fprintf(stderr, "labels ended before features at row %lld\n", rows+1);
            return 2;
        }
        rows++;
        char *tab = strchr(feature_line,'\t');
        char *last = strrchr(feature_line,'\t');
        if (!tab || !last || tab == last) {
            fprintf(stderr, "bad feature row %lld\n", rows);
            return 2;
        }
        *tab = '\0';
        char *take = last + 1;
        int win = final_integer(label_line);
        winners += win != 0;
        if (split_survives(feature_line,take)) {
            fputs(label_line,stdout);
            kept++;
            winners_kept += win != 0;
        }
    }
    if (fgets(label_line,sizeof(label_line),labels)) {
        fprintf(stderr, "labels has rows after features ended\n");
        return 2;
    }
    fprintf(stderr,
            "rows=%lld kept=%lld (%.1f%%) winners=%lld kept=%lld%s\n",
            rows,kept,rows?100.0*kept/rows:0.0,winners,winners_kept,
            winners==winners_kept?"":"  ERROR: WINNER REMOVED");
    free(triple_ok); free(quad_ok);
    fclose(features); fclose(labels);
    return winners == winners_kept ? 0 : 1;
}
