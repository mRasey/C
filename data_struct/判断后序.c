#include <stdio.h>
#include <stdlib.h>

int num[100] = {0};
int target = 0;

int judge()
{
    char c;
    int i = 0;
    int length;
    while(1){
        scanf("%d", &num[i++]);
        scanf("%c", &c);
        if(c == '\n'){
            break;
        }
    }
    length = i;
    get(0, length-1, length-1);
    if(target == 0){
        printf("true");
    }
    return 0;
}

int get(int start, int end, int length){
    int i, j;
    int root = num[length - 1];
    if(length == 1){
        return 0;
    }
    for(i = 0; i < length && num[i] < root; i++);
    for(j = i; j < length; j++){
        if(num[j] < root){
            printf("false");
            target = 1;
            return 0;
        }
    }
    if(i > 0){
        get(start, i-1, i-1);
    }
    if(i < length - 1){
        get(i, end-1, end-1);
    }
}











