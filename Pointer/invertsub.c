#include <stdio.h>
#include <mem.h>
#include <malloc.h>

char* sub_string(char* s, int start, int end) {
    char* temp = (char*) malloc(sizeof(char) * (end - start + 1));
    int i = 0;
    for(i = start; i < end; i++)
        temp[i-start] = s[i];
    temp[end-start] = '\0';
    return temp;
}

char* reverse(char* s) {
    int length = strlen(s);
    char* temp = (char*) malloc(sizeof(char) * (length + 1));
    int i = 0;
    for(i = 0; i < length; i++)
        temp[i] = s[length - i - 1];
    temp[length] = '\0';
    return temp;
}

int invertsub() {
    char s[50], t[50];
    char* sub_str;
    int i = 0;
    FILE *in, *out;
    in = fopen("invertsub.in", "r");
    out = fopen("invertsub.out", "w");
    fgets(s, 1024, in);
    fgets(t, 1024, in);
    for(i = 0; i < strlen(s); i++) {
        if(s[i] == t[0]) {
            sub_str = sub_string(s, i, strlen(t) + i);
            if(strcmp(sub_str, t) == 0) {
                fputs(strcat(strcat(sub_string(s, 0, i), reverse(sub_str)),
                                    sub_string(s, strlen(t) + 2, strlen(s))), out);
                fclose(in);
                fclose(out);
                return 0;
            }
        }
    }
    fclose(in);
    fclose(out);
    return 0;
}