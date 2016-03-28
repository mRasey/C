#include<stdio.h>

int main()
{
    int i, j, k, l, m, n, s, count;
    int ri, repeat;
    scanf("%d", &repeat);
    for(ri = 1; ri <= repeat; ri++){
        scanf("%d%d", &m, &n);
        s = 0;
        for(m = m; m <= n; m++){
            j = m % 100; //百位
            k = (m - j * 100) % 10; //十位
            l = (m - j * 100 - k * 10); //个位
            if(j != k && k != l && j != l){
                s++;
            }
        }
        printf("count=%d\n", s);
    }
}

