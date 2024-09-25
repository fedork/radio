#include "radiobase.c"
#include <math.h>

int main(int argc, char **argv){

    init();
    
    if (argc<2) {
        printf("insufficient args\n");
        exit(1);
    }
    
    int offset = 0;
    if (argc % 2 == 1) {
        offset = 1;
    }

//    int k = atoi(argv[offset+1]);
//    int size = (argc - offset - 2)/2;
    int k = 9;
    int size = 1;
    int sb[size];
    sb[0] = getSbb(112,80);
    printf("k=%d ", k);
    printSb(sb, size);
    printf("\n");
    if (offset>0) parse_file(argv[1]);
    all_solutions(sb, size, k);
}
