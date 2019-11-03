#include <stdio.h>

int main()
{
    unsigned int array[2] = {0x12345678, 0xabcdef66};
    unsigned int *pa = array;
    unsigned char *pb = array;

    for(int i=0; i<2; i++)
        printf("%x\n", pa[i]);
    for(int i=0; i<8; i++)
        printf("%x\n", pb[i]);
    return 0;
}