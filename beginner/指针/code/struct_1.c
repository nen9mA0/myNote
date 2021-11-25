#include <stdio.h>

typedef struct _test
{
    int a;
    char b;
    long long c;
    char d;
} test;

typedef struct _test2
{
    long long a;
    char b;
} test2;

typedef struct _test3
{
    int a;
    int b;
    char c;
} test3;


int main()
{
    test mytest;
    test* p = &mytest;

    printf("test      addr: %p\n", p);
    printf("test.a    addr: %p\n", &p->a);
    printf("test.b    addr: %p\n", &p->b);
    printf("test.c    addr: %p\n", &p->c);
    printf("test.d    addr: %p\n", &p->d);
    printf("next test addr: %p\n", &p[1]);
    printf("sizeof test   : %x\n", sizeof(test));
    printf("===========================\n");
    printf("sizeof test2  : %x\n", sizeof(test2));
    printf("sizeof test3  : %x\n", sizeof(test3));
}