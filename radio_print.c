#define MAX_K 7
#define MAX_N 40
#define START_N 38

#include "radiobase.c"
#include <math.h>

#define TRIVIAL -1
#define NEW_LINE -2

#define MAX_SOLUTIONS 10
#define MAX_LEN 40

typedef struct {
    int k;
    int size;
    int refs;
    int init[MAX_LEN];
    int op[MAX_LEN*2];
    int b[3][MAX_LEN*2];
    int l[3];
} sol;

sol solutions[500];
int next_sol = 1;

int normalize(int sb[], int size, int dest[]) {
    int i;
    int newsize=0;
    int sbb;
    for(i=0;i<size;i++) {
        sbb=sb[i];
        if (sbb > 1) {
            dest[newsize++]=sbb;
        }
    }
    sort1(dest, newsize);
    return newsize;
}

int is_inferior(int sb1[], int size1, int sb2[], int size2) {
    if (size1>size2) return FALSE;
    int i;
    for (i=0; i<size1; i++) {
        if (sbb_to_n1[sb1[i]] > sbb_to_n1[sb2[i]] || sbb_to_n2[sb1[i]] > sbb_to_n2[sb2[i]]) return FALSE;
    }
    return TRUE;
}

int main(int argc, char **argv){
    
    init();
    int check_cache_only = FALSE;
    if (argc>1) {
        parse_file(argv[1]);
        check_cache_only = TRUE;
    }
    //    int k = atoi(argv[offset+1]);
    //    int size = (argc - offset - 2)/2;
    //    int k = 9;
    //    int size = 1;
    
    sol *s = &solutions[next_sol++];
    s->k = MAX_K;
    s->refs=0;
    s->size = -1;
    s->init[0] = START_N;
    
    int next_level_start;
    
    int line = 1;
    while (line < next_sol) {
        printf("line=%d\n", line);
        fflush(stdout);
        s = &solutions[line];
        int k = s->k;
        int k1 = k-1;
        if (s->size < 0) {
            // Sa
            int count = s->init[0];
            
                        printf("DEBUG solving ");
                        printSa(count);
                        printf("\n");
                        fflush(stdout);
            
            
            int c1 = count;
            while(!canSolveA(c1, k1)) c1--;
            s->op[0] = c1;
            s->b[2][0] = c1;
            s->b[0][0] = count - c1;
            s->b[1][0] = getSbb(c1,count - c1);
            
            s->l[2] = s->l[0] = TRIVIAL;
            if (c1>2) {
                sol *s2 = &solutions[s->l[2] = next_sol++];
                s2->k=k1;
                s2->size = -1;
                s2->refs = 1;
                s2->init[0] = c1;
                if (count - c1 > 2) {
                    s->l[0] = s->l[2];
                    s2->refs = 2;
                }
            }
            
            sol *s1 = &solutions[next_level_start = s->l[1] = next_sol++];
            s1->k=k1;
            s1->size = 1;
            s1->refs = 1;
            s1->init[0] = s->b[1][0];
            
            //            1. (in 10) Sa(192)[18336,192] take[112] :
            //                2=>Sa(112)[6216,112](line 2)
            //                1=>Sb(112:80)[8960,192](line 3)
            //                0=>Sa(80)[3160,80](line 2)
            
            
            printf("resultprint======================\n");
            printf("resultprint %d. (in %d) (used %d) ", line, s->k, s->refs);
            printSa(count);
            printf(" take[%d]:\n", c1);
            printf("resultprint  2=>");
            printSa(c1);
            if (s->l[2] > 0){
                printf("(line %d)\n", s->l[2]);
            } else {
                printf("(trivial)\n");
            }
            printf("resultprint  1=>");
            printSb(s->b[1], 1);
            printf("(line %d)\n", s->l[1]);
            printf("resultprint  0=>");
            printSa(count-c1);
            if (s->l[0] > 0){
                printf("(line %d)\n", s->l[0]);
            } else {
                printf("(trivial)\n");
            }
            
            //            printf("DEBUG done solving ");
            //            printSa(count);
            //            printf("\n");
            fflush(stdout);
            
        } else {
            // Sb
            int size = s->size;
            int *tmp = s->init;
            
                        printf("DEBUG solving ");
                        printSb(tmp, size);
                        printf("\n");
                        fflush(stdout);

            int sb[3][size*2];
            int sbn[3][size*2];
            int sbnsize[3];
            int sbnline[3];
            
            int bestop[size*2];
            int bestsb[3][size*2];
            int bestsbn[3][size*2];
            int bestsbnsize[3];
            int bestsbnline[3];
            int best_add_lines = 5;
            
            
            int i;
            
            int size_1 = size-1;
            int size2 = size*2;
            int max_pairs_1 = power3[k1];
            int skiptop;
            int splitindex[size];
            splits *splitsarr[size];
            int spi;
            
            int sb0p[size],sb2p[size],sb1p[size];
            
            for(i=0;i<size;i++){
                splitsarr[i] = &sbb_splits[tmp[i]];
                if (size>1) {
                    for (spi = 0; spi<splitsarr[i]->size; spi++) {
                        int *s = splitsarr[i]->splitsl[spi];
                        while (s[4]<k) {
                            debug_printf("checking split solvability for %s -> [%d, %d], before: s[4]=%d s[5]=%d\n", sbb_to_str[tmp[i]], s[6], s[7], s[4], s[5]);
                            int kk = s[4];
                            int dd = NO_DEADLINE;
                            int ttt = canSolveB(s, 1, kk, dd);
                            if (ttt==TRUE) {
                                ttt = canSolveB(s+3, 1, kk, dd);
                                if (ttt==TRUE) {
                                    ttt = canSolveB(s+1, 2, kk, dd);
                                }
                            }
                            if (ttt == TRUE)
                                s[4] = MAX_K;
                            else if (ttt == FALSE)
                                s[5] = ++s[4];
                            else break;
                        }
                    }
                }
            }
            
            memset(splitindex, 0, size * sizeof(int));
            
            splitindex[0] = splitsarr[0]->size;
            int n1 = sbb_to_n1[tmp[0]];
            int n2 = sbb_to_n2[tmp[0]];
            if (n1 != n2) { // leave it alone for square
                int m1 = n1/2;
                int m2 = (n1 % 2 == 0)? (n2+1)/2 : 0;
                
//                printf("skiptop for ");
//                printSb(tmp, size);
//                printf(" skipping to %d:%d\n", m1, m2);
                while (splitsarr[0]->splitsl[splitindex[0]-1][6] != m1 || splitsarr[0]->splitsl[splitindex[0]-1][7] != m2) {
                    splitindex[0]--;
//                    printf("skipped %d:%d\n", splitsarr[0]->splitsl[splitindex[0]][6], splitsarr[0]->splitsl[splitindex[0]][7]);
                }
            }
            memset(sb, 0, sizeof(sb));
            sb[1][0] = -1; // to prevent skipping first due to skiptop
            
            int ck0,ck1,ck2;
            
            int j = 0;
            int cont = TRUE;
            clock_t start = clock();
            clock_t progress = start + PROGRESS_INTERVAL;
            long scount=0;
            long long totalsplits=0;
            long long progress_splits = 100000;
            while(cont) {
                while(splitindex[j] == 0) {
                    if (j==0) {
                        cont= FALSE;
                        break;
                    }
                    j--;
                }
                if (!cont) break;
                int spi = --splitindex[j];
                
                // for identical groups avoid trying redundant permutations
                // do not do this for i==1 because it conflicts with skiptop
                if (j>1 && tmp[j] == tmp[j-1] && spi > splitindex[j-1]) {
                    continue;
                }
                
                int *s = splitsarr[j]->splitsl[spi];
                
                //skip for split solvability
                if (s[5]>=k) continue;
                
                int p0 = sb0p[j] = sb_pairs[sb[0][j] = s[3]] + (j>0?sb0p[j-1]:0);
                int p1 = sb1p[j] = sb_pairs[sb[1][j*2] = s[1]] + sb_pairs[sb[1][j*2+1] = s[2]] + (j>0?sb1p[j-1]:0);
                int p2 = sb2p[j] = sb_pairs[sb[2][j] = s[0]] + (j>0?sb2p[j-1]:0);
                
                if (p0 > max_pairs_1 || p2 > max_pairs_1 || p1 > max_pairs_1) continue;
                
                if (j == size_1) {
                    //                    printf("DEBUG start take[");
                    //                    for(i=0;i<size*2;i++) {
                    //                        printf("%d,",m[i]);
                    //                    }
                    //                    printf("]\n");
                    //                    fflush(stdout);
                    totalsplits++;
                    if (totalsplits >= progress_splits) {
                        progress_splits = totalsplits + 1000000;
                        if (clock()>progress) {
                            if (scount>0 && clock()>start + 600 * CLOCKS_PER_SEC) {
                                printf("bailing out after 10 min\n");
                                break;
                            }
                            printf("still searching solutions for ");
                            printSb(tmp, size);
                            printf(" in %d totalsplits=%lld found=%ld elapsed=%lu trying [", k, totalsplits, scount, (clock() - start)/CLOCKS_PER_SEC);
                            for(i=0; i<size; i++) {
                                if (i>0) printf(",");
                                int *s = splitsarr[i]->splitsl[splitindex[i]];
                                printf("%d:%d", s[6], s[7]);
                            }
                            printf("]\n");
                            fflush(stdout);
                            progress = clock() + PROGRESS_INTERVAL;
                        }
                    }
                    if (check_cache_only ?
                        (canSolveB(sb[0], size, k1, CACHE_ONLY) == TRUE &&
                         canSolveB(sb[2], size, k1, CACHE_ONLY) == TRUE &&
                         canSolveB(sb[1], size*2, k1, CACHE_ONLY) == TRUE):
                        (canSolveB(sb[0], size, k1, CACHE_ONLY) != FALSE &&
                         canSolveB(sb[2], size, k1, CACHE_ONLY) != FALSE &&
                         canSolveB(sb[1], size*2, k1, CACHE_ONLY) != FALSE &&
                         canSolveB(sb[0], size, k1, NO_DEADLINE) == TRUE &&
                         canSolveB(sb[2], size, k1, NO_DEADLINE) == TRUE &&
                         canSolveB(sb[1], size*2, k1, NO_DEADLINE) == TRUE)) {
                        
                        printf("DEBUG WORKS! take[");
                        for(i=0; i<size; i++) {
                            if (i>0) printf(",");
                            int *s = splitsarr[i]->splitsl[splitindex[i]];
                            printf("%d:%d", s[6], s[7]);
                        }
                        printf("]\n");
                        fflush(stdout);
 
                        scount++;
                        int add_lines = 0;
                        int i4;
                        for (i4=0; i4<3; i4++) {
                            i = (i4+1) % 3;
//                            printf("i4=%d i=%d\n", i4, i);
//                            fflush(stdout);
                            sbnsize[i]  = normalize(sb[i], size*2, sbn[i]);
                            sbnline[i] = NEW_LINE-i;
                            if (sbnsize[i] == 0) {
                                sbnline[i] = TRIVIAL;
                            } else {
                                // find existing
                                int ii;
                                for (ii = next_level_start; ii < next_sol; ii++) {
                                    if (is_inferior(solutions[ii].init, solutions[ii].size, sbn[i], sbnsize[i]) ||
                                        is_inferior(sbn[i], sbnsize[i], solutions[ii].init, solutions[ii].size)) {
                                        // check if already using and if compatible
                                        int compat = TRUE;
                                        int i5;
                                        for (i5 = 0; i5 < i4; i5++) {
                                            int i3 = (i5+1) % 3;
                                            if (sbnline[i3] == ii &&
                                                !is_inferior(sbn[i], sbnsize[i],sbn[i3], sbnsize[i3]) &&
                                                !is_inferior(sbn[i3], sbnsize[i3],sbn[i], sbnsize[i])) {
                                                compat = FALSE;
                                            }
                                        }
                                        if (compat) {
                                            sbnline[i] = ii;
                                            break;
                                        }
                                    }
                                }
                            }
                            if (sbnline[i] <= NEW_LINE) {
                                int i5;
                                for (i5 = 0; i5 < i4; i5++) {
                                    int i3 = (i5 + 1) % 3;
                                    if (sbnline[i3] <=NEW_LINE && (
                                                                   is_inferior(sbn[i], sbnsize[i],sbn[i3], sbnsize[i3]) ||
                                                                   is_inferior(sbn[i3], sbnsize[i3],sbn[i], sbnsize[i]))) {
                                                                       sbnline[i] = sbnline[i3];
                                                                       break;
                                                                   }
                                }
                            }
                            if (sbnline[i] == NEW_LINE-i) {
                                add_lines++;
                            }
                        }
                        if (add_lines < best_add_lines) {
                            best_add_lines = add_lines;
                            
                            for(i=0; i<size; i++) {
                                int *s = splitsarr[i]->splitsl[splitindex[i]];
                                bestop[i*2] = s[6];
                                bestop[i*2+1] = s[7];
                            }
                            memcpy(bestsb, sb, sizeof(sb));
                            memcpy(bestsbn, sbn, sizeof(sbn));
                            memcpy(bestsbnsize, sbnsize, sizeof(sbnsize));
                            memcpy(bestsbnline, sbnline, sizeof(sbnline));
                            
                            if (add_lines == 0) break; // does not get any better;
                        }
                        //                        printf("DEBUG done take[");
                        //                        for(i=0;i<size*2;i++) {
                        //                            printf("%d,",m[i]);
                        //                        }
                        //                        printf("]\n");
                        
                    }
                } else {
                    if (/*check_cache_only?
                        (canSolveB(sb[0], (j+1), k1, CACHE_ONLY) == TRUE &&
                         canSolveB(sb[2], (j+1), k1, CACHE_ONLY) == TRUE &&
                         canSolveB(sb[1], (j+1)*2, k1, CACHE_ONLY) == TRUE):*/
                        (canSolveB(sb[0], (j+1), k1, CACHE_ONLY) != FALSE &&
                         canSolveB(sb[2], (j+1), k1, CACHE_ONLY) != FALSE &&
                         canSolveB(sb[1], (j+1)*2, k1, CACHE_ONLY) != FALSE)) {
                        j++;
                        splitindex[j] = splitsarr[j]->size;
                    }
                }
            }
            
            if(scount==0) {
                printf("found no solutions for ");
                printSb(tmp, size);
                printf("\n");
                exit(27);
            } else {
                printf("found %ld solutions for ", scount);
                printSb(tmp, size);
                printf("\n");
                fflush(stdout);
            }
            
            
            //            printf("DEBUG done solving ");
            //            printSb(tmp, size);
            //            printf("\n");
            //            fflush(stdout);
            //            int op[80];
            //            int b[3][80];
            //            int l[3];
            memcpy(s->op, bestop, sizeof(bestop));
            //            printf("DEBUG 123\n");            fflush(stdout);
            
            int i5;
            for (i5=0; i5<3; i5++) {
                i = (i5+1) % 3;
                //                printf("DEBUG foo i=%d\n", i);            fflush(stdout);
                memcpy(s->b[i], bestsb[i], sizeof(bestsb[i]));
                //                printf("DEBUG foo i=%d\n", i);            fflush(stdout);
                
                int l = bestsbnline[i];
                
                //                printf("DEBUG l=%d\n", l);            fflush(stdout);
                if (l != TRIVIAL) {
                    if (l == NEW_LINE - i) {
                        sol *snew = &solutions[l = next_sol++];
                        snew->k = k1;
                        snew->size = bestsbnsize[i];
                        snew->refs=0;
                        //                        printf("DEBUG bestsbnsize[i]=%d\n", bestsbnsize[i]);            fflush(stdout);
                        memcpy(snew->init, bestsbn[i], bestsbnsize[i] * sizeof(int));
                        bestsbnline[i] = l;
                        //                        printf("DEBUG l=%d\n", l);            fflush(stdout);
                    } else {
                        if (l <= NEW_LINE) {
                            
                            l = bestsbnline[NEW_LINE - l]; // reuse
                            //                            printf("DEBUG reuse l=%d\n", l);            fflush(stdout);
                        }
                        sol *sused = &solutions[l];
                        if (is_inferior(sused->init, sused->size, bestsbn[i], bestsbnsize[i])) {
                            //replace
                            //                            printf("DEBUG before replace \n");            fflush(stdout);
                            sused->size = bestsbnsize[i];
                            memcpy(sused->init, bestsbn[i], bestsbnsize[i] * sizeof(int));
                            //                            printf("DEBUG after replace \n");            fflush(stdout);
                            
                        } else {
                            // assert that we are inferior to the line we are using
                            if (!is_inferior(bestsbn[i], bestsbnsize[i], sused->init, sused->size)) {
                                printf("expected inferior: ");
                                printSb(bestsbn[i], bestsbnsize[i]);
                                printf(" to ");
                                printSb(sused->init, sused->size);
                                exit(26);
                            }
                        }
                    }
                }
                s->l[i] = l;
                solutions[l].refs++;
            }
            
            printf("resultprint %d. (in %d) (used %d) ", line, s->k, s->refs);
            printSb(s->init, s->size);
            printf(" take[");
            for(i=0;i<size*2;i++) {
                if (i>0) printf(",");
                printf("%d", s->op[i]);
            }
            printf("] :\n");
            for(i=2;i>=0;i--) {
                printf("resultprint  %d=>", i);
                printSb(s->b[i], i==1?size*2:size);
                if (s->l[i]>0) {
                    printf("(line %d)\n", s->l[i]);
                } else {
                    printf("(trivial)\n");
                }
            }
            
            printf("resultgraph\n");
            int height =0;
            for(i=0; i<size; i++){
                if (height<sbb_to_n1[tmp[i]]) height=sbb_to_n1[tmp[i]];
            }
            int line_num;
            for(line_num=0; line_num<height; line_num++) {
                printf("resultgraph ");
                for(i=0; i<size; i++){
                    int col;
                    for(col=0; col<sbb_to_n2[tmp[i]]; col++){
                        if (line_num>=sbb_to_n1[tmp[i]]) {
                            printf(" ");
                        } else {
                            if (line_num<s->op[i*2]) {
                                if (col<s->op[i*2+1]) {
                                    printf("*");
                                } else {
                                    printf("-");
                                }
                            } else {
                                if (col<s->op[i*2+1]) {
                                    printf("-");
                                } else {
                                    printf(".");
                                }
                            }
                        }
                    }
                    printf(" ");
                }
                printf("\n");
            }
        }
        printf("resultprint\n");
        fflush(stdout);
        line++;
    }
}
