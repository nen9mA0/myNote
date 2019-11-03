#include <stdio.h>

int main()
{
    int array[3][3] = { {11,12,13},{21,22,23},{31,32,33} };
    int (*p)[3] = array;

    printf( "%d\n", *( *(p+1) + 2 ) );


    printf("array:%p\n", array);
    for(int i=0; i<3; i++)
        printf("array[%d]:%p\n", i, array[i]);

    for(int i=0; i<3; i++)
        for(int j=0; j<3; j++)
            printf("&array[%d][%d]:%p\n", i, j, &array[i][j]);

    int *p2 = array;
    for(int i=0; i<9; i++)
        printf("array[%d][%d]:%d\n", i/3, i%3, p2[i]);

    int **pp = array;
    printf("pp:%p  pp+1:%p\n", p2, p2+1);
    printf("p:%p  p+1:%p\n", p, p+1);

    return 0;
}