#include <stdio.h>
#include <mem.h>
#include <ctype.h>
#include <malloc.h>

#define true 1
#define false 0
#define bool int

char* sub_string(char* s, int start, int end) {
    char* temp = (char*) malloc(sizeof(char) * (end - start + 1));
    int i = 0;
    for(i = start; i < end; i++)
        temp[i-start] = s[i];
    temp[end-start] = '\0';
    return temp;
}

int number_of(char* s, char c) {
    int count = 0;
    int i = 0;
    for(i = 0; i < strlen(s); i++) {
        if(s[i] == c)
            count++;
    }
    return count;
}

bool judge(char* s) {
    int i = 0;
    if(number_of(s, '.') > 1 || number_of(s, 'E') > 1 || s[0] == '0' || !isdigit(s[strlen(s) - 1])) {
        return false;
    }
    for(i = 0; i < strlen(s); i++) {
        if(!isdigit(s[i])) {
            if(s[i] == '.' && i == 0) {
                return false;
            }
            else if((s[i] == '+' || s[i] == '-') && s[i-1] != 'E') {
                return false;
            }
        }
    }
    return true;
}

int main() {
    char input[20];
    scanf("%s", input);
    char now = input[0];
    if(now == '+' || now == '-') {
        if(!judge(sub_string(input, 1, strlen(input)))) {
            printf("Wrong!");
            return 0;
        }
        if(number_of(input, 'E'))
            printf("Format2");
        else
            printf("Format1");
        return 0;
    }
    else if(isdigit(now)) {
        if(!judge(input)) {
            printf("Wrong!");
            return 0;
        }
        if(number_of(input, 'E'))
            printf("Format2");
        else
            printf("Format1");
        return 0;
    }
    else {
        printf("Wrong!");
        return 0;
    }
}