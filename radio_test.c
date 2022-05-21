#define DEBUG_CACHE

#include "radiobase.c"

int main(int argc, char **argv){
    init();
    int sb[8];
    sb[0]=getSbb(17,8);
    sb[1]=getSbb(16,8);
    sb[2]=getSbb(20,6);
    sb[3]=getSbb(25,4);
    sb[4]=getSbb(12,8);
    sb[5]=getSbb(24,3);
    sb[6]=getSbb(21,2);
    sb[7]=getSbb(17,2);
    int pairs=0;
    int i;
    for(i=0; i<8; i++) pairs+=sb_pairs[sb[i]];
    int k;
    k=6;
    printf("BEFORE ==========\n");
    int ck0 = checkCache(sb, 8, k);
    printf("\n========== CHECK CACHE 1: %d\n", ck0);
    cache(sb, 8, FALSE, k, pairs);
    printf("\nAFTER ==========\n");
    ck0 = checkCache(sb, 8, k);
    printf("\n========== CHECK CACHE 2: %d\n", ck0);
    return 0;
}
