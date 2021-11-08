#include "radiobase.c"
#include <math.h>

int main(int argc, char **argv){

    init();
    
    if (argc<4) {
        printf("insufficient args\n");
        exit(1);
    }

    int k = atoi(argv[1]);
    int size = (argc - 2)/2;
    int sb[size];
    int i;
    for (i=0; i<size; i++) {
        sb[i] = getSbb(atoi(argv[2+i*2]), atoi(argv[3+i*2]));
    }
    printf("k=%d ", k);
    printSb(sb, size);
    printf("\n");
    all_solutions(sb, size, k);
}
