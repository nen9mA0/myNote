#include <stdio.h>
#include <stdint.h>
#include <wchar.h>

void printbin(uint64_t num, size_t size);


int main()
{
    char a1 = 'a';
    unsigned char b1 = 'a';
    int c1 = 'a';


    printf("  char: ");
    printbin(a1, sizeof(a1));

    printf(" uchar: ");
    printbin(b1, sizeof(b1));

    printf("   int: ");
    printbin(c1, sizeof(c1));

    char ansi_str[10] = {0x45, 0xc3, 0xc8};

    printf("%s\n", ansi_str);

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