#include <stdio.h>

int main(){
    int fib[1000] = {0};
    int i;
    int input;
    scanf("%d", &input);
    fib[0] = 1;
    fib[1] = 1;
    for(i = 2; i < 1000; i++){
        fib[i] = fib[i-1] + fib[i-2];
    }
    printf("%d\n", fib[input]);
    return 0;
}