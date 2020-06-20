#include <stdio.h>
#include <stdint.h>

void printbin(void* f, size_t size);
uint32_t bin2float(int s, uint8_t e, uint32_t f);

int main()
{
    float f1 = 10.625;
    printbin(&f1, sizeof(f1));

    float f2 = 0.1;
    printbin(&f2, sizeof(f2));

    float f3 = 15000000000.000000001;
    printf("%f\n", f3);

    float f4 = 1500.1;
    float f5 = 15.1;
    printf("%f\n", f4);
    printf("%f\n", f5);

    uint32_t tmp;
    float tmpf;
    tmp = bin2float(0, 127, 0);
    tmpf = *(float*)&tmp;
    printf("%x\n", tmp);
    printf("%f\n", tmpf);

    return 0;
}

void printbin(void* f, size_t size)
{
    uint64_t mask = 1ULL << (size*8-1);
    uint64_t num = *(uint64_t*)f;
    for(int i=0; i<size*8; i++)
    {
        printf("%d", (num & mask) >> (size*8-i-1));
        mask >>= 1;
    }
    printf("\n");
}

uint32_t bin2float(int s, uint8_t e, uint32_t f)
{
    uint32_t tmp = 0;

    tmp |= (s&0x01) << 31;
    tmp |= e << 23;
    tmp |= f & 0x7fffff;

    return tmp;
}