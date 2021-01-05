#include <stdio.h>

int func1(unsigned word)
{
    return (int) ((word << 24) >> 24);
}

int func2(unsigned word)
{
    return ((int) word << 24) >> 24;
}

int showbyte(void *p, int len)
{
    unsigned char *m = (unsigned char*)p;
    for(int i=len-1; i>=0; i--)
    {
        printf("%02X ", *(m+i));
    }
    printf("\n");
}

int main(void)
{
    unsigned int w = 0x00000076;
    int tmp = func1(w);
    showbyte(&tmp, 4);
    tmp = func2(w);
    showbyte(&tmp, 4);

    w = 0x87654321;
    tmp = func1(w);
    showbyte(&tmp, 4);
    tmp = func2(w);
    showbyte(&tmp, 4);

    w = 0x000000C9;
    tmp = func1(w);
    showbyte(&tmp, 4);
    tmp = func2(w);
    showbyte(&tmp, 4);

    w = 0xEDCBA987;
    tmp = func1(w);
    showbyte(&tmp, 4);
    tmp = func2(w);
    showbyte(&tmp, 4);
    return 0;
}