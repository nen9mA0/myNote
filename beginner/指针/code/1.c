#include <stdio.h>

int main()
{
    int a = 3;
    int *p = &a;
    int **pp = &p;

    (*p)++;
    printf("%d ", a);

    (**pp)++;
    printf("%d ", a);

    return 0;
}