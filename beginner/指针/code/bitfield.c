#include <stdio.h>

typedef struct _test
{
    unsigned char byte0_lo_3: 3;
    unsigned char byte0_mid_2: 2;
    unsigned char byte0_hi_3: 3;
    unsigned char byte1_lo_4: 4;
    unsigned char byte1_hi_4: 4;
} test;

typedef union _data {
    unsigned int num;
    test all_bytes;
} data;

int main()
{
    data a;

    a.num = 0xdeadbeef;

    printf("sizeof test:             %X\n", sizeof(test));
    printf("a.all_bytes.byte0_lo_3:  %X\n", a.all_bytes.byte0_lo_3);
    printf("a.all_bytes.byte0_mid_2: %X\n", a.all_bytes.byte0_mid_2);
    printf("a.all_bytes.byte0_hi_3:  %X\n", a.all_bytes.byte0_hi_3);
    printf("a.all_bytes.byte1_lo_4:  %X\n", a.all_bytes.byte1_lo_4);
    printf("a.all_bytes.byte1_hi_4:  %X\n", a.all_bytes.byte1_hi_4);
}
