#include <stdio.h>

int main()
{
    int array[10];
    int *p = array;

    for(int i=0; i<10; i++)
    {
        *(p+i) = i;
    }

    for(int i=0; i<10; i++)
        printf("%d ", array[i]);

    return 0;
}