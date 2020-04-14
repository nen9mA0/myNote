#include <stdio.h>

int main()
{
    int a = 1, b = 2, c;
    int arr[10] = {0,1,2,3,4,5};
    int *p;

    asm( "add %2, %0    \r\n"
        : "+r"(a)
        : "r"(b)
    );

    printf("a = %d \t c = %d\n", a, c);
    
    asm( "shl $8, %0    \r\n"
        : "=r"(a)
        : "0"(a)
    );

    p = arr;
    // asm( "movl %1, (%0)"
    //     : "=m"(&arr[1])
    //     : "r"(a)
    // );

    printf("a = %d \t c = %d\n", a, c);
}