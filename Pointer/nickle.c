#include <stdio.h>

int main() {
    long n = 0;
    scanf("%ld", &n);
    printf("%ld", (long) ((10 + 100 * n) * 5 * n - (10 * n - 1)));
    return 0;
}

