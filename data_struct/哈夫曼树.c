#include <stdio.h>
#include <stdlib.h>

int num[10] = {0};
int target[100] = {0};
int tg = 0;
int sum = 0;

int huf()
{
    char c;
    int i;
    int length;
    //int sum;
    scanf("%d", &length);
    for(i = 0; i < length; i++){
        scanf("%d", &num[i]);
        sum += num[i];
    }
    hufman(length);
    for(i = 0; i < tg; i++){
        sum += target[i];
    }
    printf("%d", sum);
    return 0;
}

int hufman(int length){
    if(length == 1){
        //target[tg] = num[0];
        return num[0];
    }
    else if(length == 2){
        //target[tg] = num[0] + num[1];
        //target[tg++] = num[0];
        //target[tg] = num[1];
        return num[0] + num[1];
    }
    else{
        sort(length);
        num[0] = num[0] + num[1];
        target[tg++] = num[0];
        num[1] = 1000000;
        sort(length);
        hufman(length - 1);
    }
}

void sort(int length){
    int i, j;
    for(i = 0; i < length; i++){
        for(j = i; j < length; j++){
            if(num[i] > num[j]){
                int temp;
                temp = num[i];
                num[i] = num[j];
                num[j] = temp;
            }
        }
    }
}







