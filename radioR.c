#include "radiobase.c"

int main(int argc, char **argv){
    
    init();
    int k,n;
    
    for (k = MAX_K, n=MAX_N; k<= MAX_K; k++){
        while (!canSolveA(n,k)) {
            printf("result can't solve Sa(%d) in %d\n",n,k);
            n--;
            if (n<2) return 0;
        }
        printf("result can solve Sa(%d) in %d\n",n,k);
    }
//    for (k = 1, n=2; k<= MAX_K; k++){
//        while (canSolveA(n,k)) {
//            printf("result can solve Sa(%d) in %d\n",n,k);
//            n++;
//            if (n>MAX_N) return 0;
//        }
//        printf("result can't solve Sa(%d) in %d\n",n,k);
//    }
    return 0;
}
