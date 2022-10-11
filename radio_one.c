#include "radiobase.c"

int main(int argc, char **argv){

    init();
    
    if (argc<4) {
        printf("insufficient args\n");
        exit(1);
    }
    
    int offset = 0;
    if (argc % 2 == 1) {
        offset = 1;
    }

    int k = atoi(argv[offset+1]);
    int size = (argc - offset - 2)/2;
    int sb[size];
    int i;
    for (i=0; i<size; i++) {
        sb[i] = getSbb(atoi(argv[offset + 2 + i*2]), atoi(argv[offset+ 3 + i*2]));
    }
    printf("k=%d ", k);
    printSb(sb, size);
    printf("\n");
    if (offset>0) parse_file(argv[1]);
    canSolveB(sb, size, k, NO_DEADLINE);
}
