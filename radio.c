#include "radiobase.c"

int main(int argc, char **argv){
    init();
    if (argc==2) parse_file(argv[1]);
    int k,n;
    int prev_max_n = 1;
    for (k=1, n=2; k<= MAX_K; k++){
        while (canSolveA(n,k)) {
            printf("result can solve Sa(%d) in %d\n",n,k);
#ifdef DEBUG1
    fflush(stdout);
#endif
            n++;
            if (n>MAX_N) return 0;
        }
        printf("result can't solve Sa(%d) in %d\n",n,k);
        // start with fibonacci as a good lower bound
        int max_n = n-1;
        n=prev_max_n+max_n;
        prev_max_n=max_n;
#ifdef DEBUG1
    fflush(stdout);
#endif
    }
    return 0;
}
