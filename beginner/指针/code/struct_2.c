#include <stdio.h>

#pragma pack(push, 1)

typedef struct _test
{
    int a;
    char b;
    long long c;
    char d;
} test;

#pragma pack(pop)

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
}