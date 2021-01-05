#include <stdio.h>

int main(void)
{
    int x = 0x87654321;

    int x1 = (~x & ~0xff) | (x & 0xff);

    printf("x:\t%x\n", x);
    printf("x1:\t%x\n", x1);
}