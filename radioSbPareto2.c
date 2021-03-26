#include "radiobase.c"

int main(int argc, char **argv){
  init();
  int k,n;
  int n1,m1,n2,m2;
  int sb[2];
  for (k = 1; k<= MAX_K; k++){
    n1=min(1+(1<<k), MAX_N);
    m1=1;
    while (n1>=m1) {
        sb[0]=getSbb(n1,m1);
        if (canSolveB(sb,1,k)) {
            printf("result can solve ");
            printSb(sb, 1);
            printf(" in %d\n", k);
            n2=min((1<<k), MAX_N);
            m2=1;
            while (n2>=m2) {
                sb[1]=getSbb(n2,m2);
                if (canSolveB(sb,2,k)) {
                      printf("result can solve ");
                      printSb(sb, 2);
                      printf(" in %d\n", k);
                      m2++;
                  } else {
        //              printf("result can't solve ");
                    n2--;
                  }

              }
  		    m1++;
  		} else {
//  		    printf("result can't solve ");
            n1--;
  		}
  	}
  }  
  return 0; 
}
