#include <stdio.h>

#define true 1
#define false 0

typedef struct {
    int if_start;
    int if_end;
    int has_point;
    int start_pos;
    int end_pos;
} Point;

Point points[1000000];
int prz() {
    int length = 0;
    int start, end;
    int i = 0;
    FILE *in, *out;
    in = fopen("prz.in", "r");
    out = fopen("prz.out", "w");
    while(fscanf(in, "%d %d", &start, &end) == 2) {
        if(length < end)
            length = end;
        points[start].if_start = true;
        points[start].has_point = true;
        points[start].end_pos = end;
        points[end].if_end = true;
        points[end].has_point = true;
        points[end].start_pos = start;
    }
    int startPos;
    int endPos;
    for(i = 1; i <= length; i++) {
        B:startPos = i;
        if(points[i].has_point && points[i].if_start) { //发现一个单独Q
            endPos = points[i].end_pos;
            for(i = i + 1 ; i <= length; i++) {
                if(i == length) {
                    fprintf(out, "%d %d\n", startPos, length);
                    return 0;
                }
                else if(points[i].has_point && points[i].if_end && points[i].if_start != 1 && i >= endPos) { //发现一个单独Z
                    A:endPos = i;
                    if(i == length) {
                        fprintf(out, "%d %d\n", startPos, length);
                        return 0;
                    }
                    for(i = i + 1; i <= length; i++) {
                        if(points[i].has_point && points[i].if_start && points[i].if_end != 1) { //发现一个单独Q
                            fprintf(out, "%d %d\n", startPos, endPos);
                            goto B;
                        }
                        else if(points[i].has_point && points[i].if_end && points[i].if_start) { //发现一个QZ
                            goto A;
                        }
                        if(i == length) {
                            fprintf(out, "%d %d\n", startPos, length);
                            return 0;
                        }
                    }
                }
                else if(points[i].has_point && points[i].if_start) { //又发现一个Q
                    if(points[i].end_pos > endPos)
                        endPos = points[i].end_pos;
                }
            }
        }
    }
    fclose(in);
    fclose(out);
    return 0;
}

