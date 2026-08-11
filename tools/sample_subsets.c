/* Sample the pair and triple subset filters on parent states too large to enumerate.
 *
 * Take-vectors are drawn uniformly from each part's existing standalone-valid
 * option list.  The report conditions successively on the counting bound, the
 * exact pair filter, and the exact triple filter.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "read_int_table.h"

#define AXIS 64
#define MAX_PARTS 16
#define MAX_OPTIONS 4096

static const int power3[]={1,3,9,27,81,243,729,2187,6561};
static const int p5[]={0,32,31,27,24,22,19,17,15,14,12,11,10,9,9,8,7,7,6,6,5,5,5,4,4,3,3,3,2,2,2,2,1};
static unsigned char pair_ok[AXIS*AXIS][AXIS*AXIS/8];
static int single_id[AXIS][AXIS],single_count;
static unsigned char *triple_ok;
static long long triple_lookups;
static int N[MAX_PARTS],M[MAX_PARTS],A[MAX_PARTS],B[MAX_PARTS],parts,cap;
static int OX[MAX_PARTS][MAX_OPTIONS],OY[MAX_PARTS][MAX_OPTIONS],option_count[MAX_PARTS];

static int solvable5(int n,int m){
    if(n==0||m==0)return 1;
    if(n>32||m>32)return 0;
    return n<=p5[m];
}
static int grid_id(int n,int m){return n*AXIS+m;}
static void set_pair(int n1,int m1,int n2,int m2){
    int a=grid_id(n1,m1),b=grid_id(n2,m2);
    pair_ok[a][b>>3]|=(unsigned char)(1u<<(b&7));
    pair_ok[b][a>>3]|=(unsigned char)(1u<<(a&7));
}
static int get_pair(int n1,int m1,int n2,int m2){
    if(n1==0||m1==0||n2==0||m2==0)return 1;
    int a=grid_id(n1,m1),b=grid_id(n2,m2);
    return (pair_ok[a][b>>3]>>(b&7))&1;
}
static size_t triple_index(int a,int b,int c){
    if(a>b){int t=a;a=b;b=t;}if(b>c){int t=b;b=c;c=t;}if(a>b){int t=a;a=b;b=t;}
    return ((size_t)a*single_count+b)*single_count+c;
}
static void set_triple(int n1,int m1,int n2,int m2,int n3,int m3){
    int a=single_id[n1][m1],b=single_id[n2][m2],c=single_id[n3][m3];
    if(a>=0&&b>=0&&c>=0)triple_ok[triple_index(a,b,c)]=1;
}
static int get_triple(int n1,int m1,int n2,int m2,int n3,int m3){
    triple_lookups++;
    if(n1==0||m1==0||n2==0||m2==0||n3==0||m3==0)return 1;
    int a=single_id[n1][m1],b=single_id[n2][m2],c=single_id[n3][m3];
    return a>=0&&b>=0&&c>=0&&triple_ok[triple_index(a,b,c)];
}
static int pair_survives(void){
    int n0[MAX_PARTS],m0[MAX_PARTS],n2[MAX_PARTS],m2[MAX_PARTS];
    int n1[2*MAX_PARTS],m1[2*MAX_PARTS];
    for(int i=0;i<parts;i++){
        n2[i]=A[i];m2[i]=B[i];n0[i]=N[i]-A[i];m0[i]=M[i]-B[i];
        n1[2*i]=A[i];m1[2*i]=M[i]-B[i];
        n1[2*i+1]=N[i]-A[i];m1[2*i+1]=B[i];
    }
    int count[3]={parts,parts,2*parts};int *nn[3]={n0,n2,n1},*mm[3]={m0,m2,m1};
    for(int c=0;c<3;c++)for(int i=0;i<count[c];i++)for(int j=i+1;j<count[c];j++)
        if(!get_pair(nn[c][i],mm[c][i],nn[c][j],mm[c][j]))return 0;
    return 1;
}
static int triple_survives(void){
    int n0[MAX_PARTS],m0[MAX_PARTS],n2[MAX_PARTS],m2[MAX_PARTS];
    int n1[2*MAX_PARTS],m1[2*MAX_PARTS];
    for(int i=0;i<parts;i++){
        n2[i]=A[i];m2[i]=B[i];n0[i]=N[i]-A[i];m0[i]=M[i]-B[i];
        n1[2*i]=A[i];m1[2*i]=M[i]-B[i];
        n1[2*i+1]=N[i]-A[i];m1[2*i+1]=B[i];
    }
    int count[3]={parts,parts,2*parts};int *nn[3]={n0,n2,n1},*mm[3]={m0,m2,m1};
    for(int c=0;c<3;c++)for(int i=0;i<count[c];i++)for(int j=i+1;j<count[c];j++)
        for(int q=j+1;q<count[c];q++)
            if(!get_triple(nn[c][i],mm[c][i],nn[c][j],mm[c][j],nn[c][q],mm[c][q]))return 0;
    return 1;
}

int main(int argc,char **argv){
    if(argc!=6){
        fprintf(stderr,"usage: %s <parent-k> <pair-table> <triple-table> <states> <samples>\n",argv[0]);
        return 2;
    }
    cap=power3[atoi(argv[1])-1];
    memset(single_id,0xff,sizeof(single_id));
    for(int n=1;n<=32;n++)for(int m=1;m<=32;m++)if(solvable5(n,m))single_id[n][m]=single_count++;
    triple_ok=calloc((size_t)single_count*single_count*single_count,1);
    if(!triple_ok)return 2;
    FILE *f=fopen(argv[2],"r");int row[6];
    if(!f)return 2;while(radio_read_int_row(f,row,4))set_pair(row[0],row[1],row[2],row[3]);fclose(f);
    f=fopen(argv[3],"r");if(!f)return 2;
    while(radio_read_int_row(f,row,6))set_triple(row[0],row[1],row[2],row[3],row[4],row[5]);fclose(f);
    long long samples=atoll(argv[5]);
    f=fopen(argv[4],"r");if(!f)return 2;char line[1024];
    while(fgets(line,sizeof(line),f)){
        char state[768];if(sscanf(line,"%767s",state)!=1)continue;
        parts=0;char *p=state;int n,m;
        while(sscanf(p,"%d:%d",&n,&m)==2){N[parts]=n;M[parts]=m;parts++;p=strchr(p,',');if(!p)break;p++;}
        double product=1;
        for(int i=0;i<parts;i++){
            option_count[i]=0;
            for(int x=0;x<=N[i];x++)for(int y=0;y<=M[i];y++)
                if(solvable5(x,y)&&solvable5(N[i]-x,M[i]-y)
                   &&solvable5(x,M[i]-y)&&solvable5(N[i]-x,y)){
                    int q=option_count[i]++;OX[i][q]=x;OY[i][q]=y;
                }
            product*=option_count[i];
        }
        srandom(12345);long long cap_count=0,pair_count=0,triple_count=0;
        long long lookup_sum=0,reject_lookup_sum=0,reject_count=0;
        long long rejected_by[7]={0};
        static const int checkpoints[7]={1,5,10,25,50,100,250};
        for(long long s=0;s<samples;s++){
            int c0=0,c1=0,c2=0;
            for(int i=0;i<parts;i++){
                int q=(int)(random()%option_count[i]);A[i]=OX[i][q];B[i]=OY[i][q];
                c0+=(N[i]-A[i])*(M[i]-B[i]);
                c1+=A[i]*(M[i]-B[i])+(N[i]-A[i])*B[i];c2+=A[i]*B[i];
            }
            if(c0>cap||c1>cap||c2>cap)continue;cap_count++;
            if(!pair_survives())continue;pair_count++;
            triple_lookups=0;
            if(triple_survives())triple_count++;
            else {
                reject_count++;reject_lookup_sum+=triple_lookups;
                for(int q=0;q<7;q++)if(triple_lookups<=checkpoints[q])rejected_by[q]++;
            }
            lookup_sum+=triple_lookups;
        }
        printf("state=%s\n  option product %.3e; samples %lld; cap %lld; pair %lld; triple %lld\n",
               state,product,samples,cap_count,pair_count,triple_count);
        if(pair_count)printf("  pair gain %.2fx; triple marginal %.2fx; cumulative %.2fx\n",
            (double)cap_count/pair_count,triple_count?(double)pair_count/triple_count:0.0,
            triple_count?(double)cap_count/triple_count:0.0);
        if(pair_count)printf("  triple lookups: mean %.1f per pair survivor, %.1f per rejection",
            (double)lookup_sum/pair_count,reject_count?(double)reject_lookup_sum/reject_count:0.0);
        for(int q=0;q<7;q++)printf("; reject<=%d %.1f%%",checkpoints[q],
            reject_count?100.0*rejected_by[q]/reject_count:0.0);
        printf("\n");
    }
    fclose(f);free(triple_ok);return 0;
}
