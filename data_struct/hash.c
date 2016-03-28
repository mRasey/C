#include <stdio.h>
//#include <Windows.h>

int sum;
int num[10000] = {0};
int hash[10000] = {0};

int main(){
    int i;
    char c = ' ';
    /*for(i = 0; i < 10000; i++){
        hash[i].num = i;
    }*/
    for(sum = 0; c != '#'; sum++){
        scanf("%d%c", &num[sum], &c);
    }
    /*for(i = 0; i < sum; i++){
        printf("%d\n", num[i]);
    }*/
    for(i = 0; i < sum; i++){
        hash[num[i]]++;
    }
    for(i = 0; i < 10000; i++){
        if(hash[i] != 0){
            printf("%d=%d\n", i, hash[i]);
        }
    }


    //system("pause");
    return 0;
}