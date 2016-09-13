#include <stdio.h>
#include <afxres.h>

#define true 1
#define false 0
#define bool int
#define print(x) printf("%ld\n", x)

int record[30000];
bool composit_number[30001];

bool is_composite_number(int input) {
    int i;
    if(input == 1)
        return false;
    if(input == 2)
        return true;
    for(i = 2; i * i <= input; i++) {
        if(input % i == 0)
            return false;
    }
    return true;
}

void decompose(int input) {
    int i;
    for(i = 2; i <= input; i++) {
        if(composit_number[i] && (input % i == 0)) {
            record[i] += 1;
            input = input / i;
            i--;
        }
    }
}

void init_number() {
    int i;
    for(i = 1; i <= 30000; i++) {
        if(is_composite_number(i)) {
            composit_number[i] = true;
        }
    }
}

int main() {
    int n;
    int i;
    bool first_print = true;
    init_number();
    scanf("%d", &n);
    for(i = 1; i <= n; i++) {
        decompose(i);
    }
    for(i = 2; i <= n; i++) {
        if(composit_number[i] && record[i] >= 1) {
            if(record[i] == 1) {
                if(first_print) {
                    printf("%d", i);
                    first_print = false;
                }
                else
                    printf("*%d", i);
            }
            else {
                if(first_print) {
                    printf("%d^%d", i, record[i]);
                    first_print = false;
                }
                else
                    printf("*%d^%d", i, record[i]);
            }
        }
    }
    return 0;
}

