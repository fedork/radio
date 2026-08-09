/* Attach winner labels to split-feature rows.
 *
 * Winner logs contain lines with `state=... take=...`; feature rows contain
 * the same state and take vector around tab-separated numeric features.  The
 * output is `state-id features... winner`, suitable for the rank evaluators.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE (1u<<18)

typedef struct Entry { char *key; struct Entry *next; } Entry;
static Entry *table[HASH_SIZE];

static unsigned hash_string(const char *s) {
    unsigned h=2166136261u;
    while(*s){h^=(unsigned char)*s++;h*=16777619u;}
    return h&(HASH_SIZE-1);
}

static void insert_key(const char *key) {
    unsigned h=hash_string(key);
    for(Entry *e=table[h];e;e=e->next)if(strcmp(e->key,key)==0)return;
    Entry *e=malloc(sizeof(*e));e->key=strdup(key);e->next=table[h];table[h]=e;
}

static int has_key(const char *key) {
    for(Entry *e=table[hash_string(key)];e;e=e->next)if(strcmp(e->key,key)==0)return 1;
    return 0;
}

static void make_key(char *dst,size_t size,const char *state,const char *take) {
    snprintf(dst,size,"%s|%s",state,take);
}

int main(int argc,char **argv) {
    if(argc!=3){fprintf(stderr,"usage: %s <winner-log> <feature-tsv>\n",argv[0]);return 2;}
    FILE *f=fopen(argv[1],"r");if(!f){fprintf(stderr,"cannot open %s\n",argv[1]);return 2;}
    char line[4096],key[2048];long long winner_rows=0;
    while(fgets(line,sizeof(line),f)){
        if(strncmp(line,"WIN ",4)!=0)continue;
        char *sp=strstr(line,"state="),*tp=strstr(line," take=");
        if(!sp||!tp)continue;sp+=6;*tp='\0';tp+=6;
        char *end=strpbrk(tp," \r\n");if(end)*end='\0';
        make_key(key,sizeof(key),sp,tp);insert_key(key);winner_rows++;
    }
    fclose(f);
    f=fopen(argv[2],"r");if(!f){fprintf(stderr,"cannot open %s\n",argv[2]);return 2;}
    char previous[1024]="";int state_id=-1;long long rows=0,winners=0;
    while(fgets(line,sizeof(line),f)){
        char *first=strchr(line,'\t'),*last=strrchr(line,'\t');
        if(!first||!last||first==last){fprintf(stderr,"bad feature row %lld\n",rows+1);return 2;}
        *first='\0';char *state=line,*features=first+1,*take=last+1;*last='\0';
        char *end=strpbrk(take," \r\n");if(end)*end='\0';
        if(strcmp(previous,state)!=0){snprintf(previous,sizeof(previous),"%s",state);state_id++;}
        make_key(key,sizeof(key),state,take);int win=has_key(key);winners+=win;rows++;
        printf("%d %s %d\n",state_id,features,win);
    }
    fclose(f);
    fprintf(stderr,"winner-log rows=%lld feature rows=%lld states=%d labelled winners=%lld\n",
            winner_rows,rows,state_id+1,winners);
    return 0;
}
