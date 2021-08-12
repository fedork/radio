#include "radiobase.c"

int main(int argc, char **argv){
    init();
    int k,n;
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
#ifdef DEBUG1
    fflush(stdout);
#endif
    }
    return 0;
}
