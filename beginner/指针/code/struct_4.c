#include <stdio.h>

#pragma pack(push, 1)

typedef struct _test
{
    unsigned char lo;
    unsigned char mid_1;
    unsigned char mid_2;
    unsigned char hi;
} test;

typedef union _data {
    unsigned int num;
    test all_bytes;
} data;

#pragma pack(pop)

int main()
{
    data a;

    a.num = 0xdeadbeef;

    printf("a.num:             %X\n", a.num);
    printf("a.all_bytes.lo:    %X\n", a.all_bytes.lo);
    printf("a.all_bytes.mid_1: %X\n", a.all_bytes.mid_1);
    printf("a.all_bytes.mid_2: %X\n", a.all_bytes.mid_2);
    printf("a.all_bytes.hi:    %X\n", a.all_bytes.hi);
}
