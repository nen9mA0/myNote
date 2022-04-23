#include <stdio.h>

#define NUM 20

int table[NUM];

int main(void)
{
    int c;
    for(int i=0; i<NUM; i++)
        table[i] = NUM - i;

    for(int i=0; i<NUM-1; i++)
        for(int j=NUM-1; j>=i; j--)
        {
            if(table[j] > table[j+1])
            {
                c = table[j];
                table[j] = table[j+1];
                table[j+1] = c;
            }
        }

    for(int i=0; i<NUM; i++)
        printf("%d ", table[i]);

    return 0;
}