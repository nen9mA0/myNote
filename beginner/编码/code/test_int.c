#include <stdio.h>
#include <stdint.h>

void printbin(uint64_t num, size_t size);

int main()
{
    uint8_t a1 = 128;
    uint16_t b1 = 128;
    uint32_t c1 = 128;
    uint64_t d1 = 128;

    printf(" uint8_t: ");
    printbin(a1, sizeof(a1));

    printf("uint16_t: ");
    printbin(b1, sizeof(b1));

    printf("uint32_t: ");
    printbin(c1, sizeof(c1));

    printf("uint64_t: ");
    printbin(d1, sizeof(d1));


    int8_t a2 = -128;
    int16_t b2 = -128;
    int32_t c2 = -128;
    int64_t d2 = -128;

    printf("  int8_t: ");
    printbin(a2, sizeof(a2));

    printf(" int16_t: ");
    printbin(b2, sizeof(b2));

    printf(" int32_t: ");
    printbin(c2, sizeof(c2));

    printf(" int64_t: ");
    printbin(d2, sizeof(d2));

    return 0;
}

void printbin(uint64_t num, size_t size)
{
    uint64_t mask = 1ULL << (size*8-1);
    for(int i=0; i<size*8; i++)
    {
        printf("%d", (num & mask) >> (size*8-i-1));
        mask >>= 1;
    }
    printf("\n");
}