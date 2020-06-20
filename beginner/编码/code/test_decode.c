#include <stdio.h>
#include <stdint.h>

int main()
{
    uint8_t a = 0x41;

    printf("%d\n", a);
    printf("%c\n", a);

    printf("\n");
    int32_t b = -1;
    printf("%d\n", b);
    printf("%u\n", b);

    printf("\n");
    uint64_t c = 0x00656f6d456948;
    printf("%lld\n", c);
    printf("%s\n", &c);

    return 0;
}
