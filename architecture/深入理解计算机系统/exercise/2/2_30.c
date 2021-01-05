#include <stdio.h>

int tadd_ok(int x, int y)
{
    unsigned int mask = 1 << (sizeof(unsigned int)*8-1);
    return ( !((x&mask)^(y&mask)) && (x&mask)^((x+y)&mask) );
}

int main(void)
{
    int a, b;

    a = 0x70000000;
    b = 0x20000000;
    printf("%d + %d = %d, %d\n", a, b, a+b, tadd_ok(a, b));

    a = 0xf0000000;
    b = 0x20000000;
    printf("%d + %d = %d, %d\n", a, b, a+b, tadd_ok(a, b));

    a = 0xc0000000;
    b = 0x20000000;
    printf("%d + %d = %d, %d\n", a, b, a+b, tadd_ok(a, b));

    a = 0x80000000;
    b = 0x90000000;
    printf("%d + %d = %d, %d\n", a, b, a+b, tadd_ok(a, b));

    return 0;
}