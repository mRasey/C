#include <stdio.h>
#include <malloc.h>
#include <string.h>

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

int main() {
    char s[50], t[50];
    char* sub_str;
    int i = 0;
    FILE *in, *out;
    in = fopen("C:\\Users\\Billy\\Desktop\\123\\invertsub.in", "r");
    out = fopen("C:\\Users\\Billy\\Desktop\\123\\invertsub.out", "w");
    fgets(s, 50, in);
    fgets(t, 50, in);
    for(i = 0; i < strlen(s); i++) {
        if(s[i] == t[0]) {
            sub_str = sub_string(s, i, strlen(t) + i);
            if(strcmp(sub_str, t) == 0) {
                fputs(strcat(strcat(sub_string(s, 0, i), reverse(sub_str)),
                                    sub_string(s, strlen(t) + 2, strlen(s))), out);
                fclose(out);
                fclose(in);
                return 0;
            }
        }
    }
    fclose(out);
    fclose(in);
    return 0;
}