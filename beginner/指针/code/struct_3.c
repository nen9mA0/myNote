#include <stdio.h>

#pragma pack(push, 1)

typedef struct _test
{
    unsigned char lo;
    unsigned char mid_1;
    unsigned char mid_2;
    unsigned char hi;
} test;

#pragma pack(pop)

int main()
{
    unsigned int a = 0xdeadbeef;
    test* p = &a;

    printf("a:        %X\n", a);
    printf("p->lo:    %X\n", p->lo);
    printf("p->mid_1: %X\n", p->mid_1);
    printf("p->mid_2: %X\n", p->mid_2);
    printf("p->hi:    %X\n", p->hi);
}