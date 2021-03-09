#include <stdio.h>

#define ALIGN_DOWN(base, size)	((base) & -((__typeof__ (base)) (size)))
#define ALIGN_UP(base, size)	ALIGN_DOWN ((base) + (size) - 1, (size))

int main()
{
    printf("%d\n", ALIGN_DOWN(8191, 4096));
    return 0;
}