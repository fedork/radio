/* Replay logged long k=5 states against a k<=4 cache without letting one target answer the next.
 * This is a lab driver for comparing FAST pass policies; it is not a solver front end. */
#ifndef MEASURE_FAST_REPLAY
#error "compile fast_replay.c with -DMEASURE_FAST_REPLAY"
#endif
#include "../radiobase.c"
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 8) {
        fprintf(stderr, "usage: %s <cache> <facts> <k> <parts> <+|-> <stride> <limit>\n", argv[0]);
        return 2;
    }
    int want_k=atoi(argv[3]), want_parts=atoi(argv[4]);
    char want_sign=argv[5][0];
    int stride=atoi(argv[6]), limit=atoi(argv[7]);
    if (stride < 1 || limit < 1) return 2;
    init();
    parse_file(argv[1]);
    FILE *f=fopen(argv[2],"r");
    if (!f) return 2;
    char line[4096]; int seen=0,done=0,mismatch=0;
    while (fgets(line,sizeof(line),f)) {
        char copy[4096],*tok[128],*save=NULL;
        memcpy(copy,line,strlen(line)+1);
        int nt=0;
        for (char *p=strtok_r(copy," \t\r\n",&save); p && nt<128;
             p=strtok_r(NULL," \t\r\n",&save)) tok[nt++]=p;
        if (nt<7 || tok[0][0]!=want_sign || strcmp(tok[1],"b")) continue;
        int ti=2; while (ti<nt && strcmp(tok[ti],"t")) ti++;
        if (ti>=nt || (ti-2)%2) continue;
        int parts=(ti-2)/2, k=atoi(tok[nt-1]);
        if (parts!=want_parts || k!=want_k) continue;
        if ((seen++ % stride) != 0) continue;
        int sb[parts];
        for (int i=0;i<parts;i++) sb[i]=getSbb(atoi(tok[2+2*i]),atoi(tok[3+2*i]));
        /* Preserve one identical lower-level cache image for every case. The child may extend any
           cache level and self-tune split metadata; copy-on-write discards all of it on exit. */
        fflush(NULL);
        pid_t pid=fork();
        if (pid<0) return 2;
        if (pid>0) {
            int status=0;
            if (waitpid(pid,&status,0)<0 || !WIFEXITED(status) || WEXITSTATUS(status)!=0) mismatch++;
            if (++done>=limit) break;
            continue;
        }
#ifdef MEASURE_FAST_REPLAY
        fast_replay_capture=1; fast_replay_pass=fast_replay_fast=0; fast_replay_splits=0;
        fast_replay_first_splits=0; fast_replay_first_depth=0;
        memset(fast_replay_first_ok,0,sizeof(fast_replay_first_ok));
#endif
        clock_t t0=clock();
        int r=canSolveB(sb,parts,k,NO_DEADLINE);
        double sec=(double)(clock()-t0)/CLOCKS_PER_SEC;
        int expected=want_sign=='+' ? TRUE : FALSE;
        fprintf(stderr,"FAST_REPLAY case=%d sign=%c result=%d pass=%d fast=%d splits=%llu sec=%.6f "
                       "first=%llu depth=%d ok=",
                done,want_sign,r,fast_replay_pass,fast_replay_fast,fast_replay_splits,sec,
                fast_replay_first_splits,fast_replay_first_depth);
        for (int i=0;i<parts;i++) fprintf(stderr,"%s%llu",i?",":"",fast_replay_first_ok[i]);
        fprintf(stderr,"\n");
        fflush(stderr);
        fflush(NULL);
        _exit(r==expected ? 0 : 1);
    }
    fclose(f);
    fprintf(stderr,"FAST_REPLAY_DONE sign=%c k=%d parts=%d stride=%d cases=%d mismatches=%d\n",
            want_sign,want_k,want_parts,stride,done,mismatch);
    return mismatch ? 1 : 0;
}
