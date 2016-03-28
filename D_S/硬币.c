#include <stdio.h>

int main(){
    int V, N;
    int coin[100] = {0};
    int seq[10000] = {0};
    int i, j;
    scanf("%d %d", &V, &N);
    for(i = 0; i < V; i++){
        scanf("%d",&coin[i]);
    }
    seq[0]= 1;
    for(i = 0; i < V; i++){
        for(j = 0; j < 10000; j++){
            if(j + coin[i] < 10000)
                seq[j + coin[i]] += seq[j]; 
        }
    }
    printf("%d\n", seq[N]);
    return 0;
}