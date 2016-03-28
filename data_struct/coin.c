#include <stdio.h>
//非递归
int V, N;
int coin[1000] = {0};
int M[20000] = {0};

int main(){ 
	int i, j;
	M[0] = 1;
	scanf("%d %d", &V, &N);
	for(i = 0; i < V; i++){
		scanf("%d", &coin[i]);
	}
	for(j = 0; j < V; j++){
		for(i = 0; i < 20000; i++){
			if(i + coin[j] < 20000)
				M[i+coin[j]] += M[i];
		}
	}
	printf("%d\n", M[N]);
	return 0;
}