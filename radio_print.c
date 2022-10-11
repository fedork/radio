#define MAX_K 10
#define MAX_N 194

#include "radiobase.c"
#include <math.h>

#define TRIVIAL -1
#define NEW_LINE -2


typedef struct {
    int k;
    int size;
    int init[40];
    int op[80];
    int b[3][80];
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
    s->size = -1;
    s->init[0] = 192;
    
    int next_level_start;
    
    int line = 1;
    while (line < next_sol) {
        //        printf("line=%d\n", line);
        s = &solutions[line];
        int k = s->k;
        int k1 = k-1;
        if (s->size < 0) {
            // Sa
            int count = s->init[0];
            
            //            printf("DEBUG solving ");
            //            printSa(count);
            //            printf("\n");
            //            fflush(stdout);
            
            
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
                s2->init[0] = c1;
                if (count - c1 > 2) {
                    s->l[0] = s->l[2];
                }
            }
            
            sol *s1 = &solutions[next_level_start = s->l[1] = next_sol++];
            s1->k=k1;
            s1->size = 1;
            s1->init[0] = s->b[1][0];
            
            //            1. (in 10) Sa(192)[18336,192] take[112] :
            //                2=>Sa(112)[6216,112](line 2)
            //                1=>Sb(112:80)[8960,192](line 3)
            //                0=>Sa(80)[3160,80](line 2)
            
            
            
            printf("resultprint %d. (in %d) ", line, s->k);
            printSa(count);
            printf(" take[%d] :\n", c1);
            printf("resultprint  2=>");
            printSa(c1);
            printf("(line %d)\n", s->l[2]);
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
            //
            //            printf("DEBUG solving ");
            //            printSb(tmp, size);
            //            printf("\n");
            //            fflush(stdout);
            
            
            int n[size*2], m[size*2];
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
            for(i=0;i<size;i++) {
                n[i*2] = sbb_to_n1[tmp[i]];
                n[i*2+1] = sbb_to_n2[tmp[i]];
                m[i*2] = 0;
                m[i*2+1] = 0;
                sb[0][i*2] = sb[0][i*2+1] = sb[1][i*2] = sb[1][i*2+1] = sb[2][i*2] = sb[2][i*2+1] = 0;
            }
            
            m[0] = (n[0] + 1)/2 + 1; // start from the middle
            int j = 0;
            int cont = TRUE;
            while(cont) {
                while(m[j] == 0) {
                    if (j==0) {
                        cont= FALSE;
                        break;
                    }
                    j--;
                }
                if (!cont) break;
                m[j]--;
                //        printf("l1 j = %d mj = %d\n", j, m[j]);
                if ( (j % 2) == 1) {
                    i=j/2;
                    sb[2][i] = getSbb(m[i*2], m[i*2+1]);
                    sb[0][i] = getSbb(n[i*2] - m[i*2], n[i*2+1] - m[i*2+1]);
                    sb[1][i*2] = getSbb(m[i*2], n[i*2+1] - m[i*2+1]);
                    sb[1][i*2 + 1] = getSbb(n[i*2] - m[i*2], m[i*2+1]);
                }
                if (j == size*2 - 1) {
                    //                    printf("DEBUG start take[");
                    //                    for(i=0;i<size*2;i++) {
                    //                        printf("%d,",m[i]);
                    //                    }
                    //                    printf("]\n");
                    //                    fflush(stdout);
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
                        
                        //                        printf("DEBUG WORKS! take[");
                        //                        for(i=0;i<size*2;i++) {
                        //                            printf("%d,",m[i]);
                        //                        }
                        //                        printf("]\n");
                        
                        int add_lines = 0;
                        for (i=0; i<3; i++) {
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
                                        int i3;
                                        for (i3 = 0; i3 < i; i3++) {
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
                                int i3;
                                for (i3 = 0; i3 < i; i3++) {
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
                            
                            memcpy(bestop, m, sizeof(m));
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
                    if (((j%2)==0) || check_cache_only ?
                        (canSolveB(sb[0], (i+1), k1, CACHE_ONLY) == TRUE &&
                         canSolveB(sb[2], (i+1), k1, CACHE_ONLY) == TRUE &&
                         canSolveB(sb[1], (i+1)*2, k1, CACHE_ONLY) == TRUE):
                        (canSolveB(sb[0], (i+1), k1, CACHE_ONLY) != FALSE &&
                         canSolveB(sb[2], (i+1), k1, CACHE_ONLY) != FALSE &&
                         canSolveB(sb[1], (i+1)*2, k1, CACHE_ONLY) != FALSE &&
                         canSolveB(sb[0], (i+1), k1, NO_DEADLINE) == TRUE &&
                         canSolveB(sb[2], (i+1), k1, NO_DEADLINE) == TRUE &&
                         canSolveB(sb[1], (i+1)*2, k1, NO_DEADLINE) == TRUE)) {
                        j++;
                        m[j] = n[j] + 1;
                    }
                }
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
            
            for (i=0; i<3; i++) {
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
            }
            
            printf("resultprint %d. (in %d) ", line, s->k);
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
            
            fflush(stdout);
            
        }
        line++;
    }
}
