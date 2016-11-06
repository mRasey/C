//
// Created by 王震 on 2016/11/6.
//
#include <stdio.h>

const int x = 0;
const char y = 'b';
int z = 0;
char m = 'm';

int foo(int a, int b)
{
    if(a > b)
        printf("big\n");
    if(a < b)
        printf("less\n");
    if(a >= b)
        printf("big or equal\n");
    if(a <= b)
        printf("less or equal\n");
    if(a != b)
        printf("not equal\n");
    if(a == b)
        printf("equal\n");
    return (233);
}

void foo2()
{
    int i = 0;

    for(i = 0; i < 10; i = i + 1)
    {
        printf("%d\n", i);
    }

    do
    {
        printf("%d\n", i);
        i = i - 1;
    } while(i > 0);


}

int foo3(int i)
{
    if(i == 10)
        return (i);
    return (i + foo3(i+1));
}

void foo4(int a, int b)
{
    printf("%d\n", a - b);
    printf("%d\n", a + b);
    printf("%d\n", a * b);
    printf("%d\n", a / b);
}

void main()
{
    int a = 0;
    char b = 'b';
    scanf("%c", &b);
    a = foo(2, 3);
    foo2();
    printf("digui: %d\n", foo3(1));
    foo4(1, 2);
    return;
}