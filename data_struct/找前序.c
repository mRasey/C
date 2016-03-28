#include <stdio.h>
#include <stdlib.h>

char mid[10000] = {'\0'};
char las[10000] = {'\0'};
int lasToMid[10000] = {0};
int midToLas[10000] = {0};
char out[1000] = {'\0'};
int p = 0;

int main()
{
    int length = 0;
    int i = 0, j = 0;
    char c;
    for(i = 0; (c = getchar()) && (c != ' '); i++){
        mid[i] = c;
    }
    for(i = 0; (c = getchar()) && (c != '\n'); i++){
        las[i] = c;
    }
    length = i;
    front(length - 1, length - 1, length);
    for( i = p - 1; i >= 0; i--){
        if('a' <= out[i] && out[i] <= 'z'){
            printf("%c", out[i]);
        }
    }
    return 0;
}

void front(int mid_end, int las_end, int length){
    int i;
    if(length == 1){
        out[p++] = las[las_end];
    }
    else{
        for(i = length; i >= 0 && mid[mid_end - i] != las[las_end]; i--);
        if(i < length){
            front(mid_end, las_end - 1, i);
        }
        if(i + 1 > 0){
            front(mid_end - i - 1,las_end - i - 1, length - i - 1);
            out[p++] = las[las_end];
        }
    }
}











