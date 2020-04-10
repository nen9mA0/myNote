#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 100000

int* sieve_1(int n, int *size)
{
    int *table = malloc(n * sizeof(int));
    int *prime = malloc(n * sizeof(int) / 2);
    int prime_cnt = 0;

    memset(table, 0, n*sizeof(int));

    for(int i=2; i*i<n; i++)
    {
        for(int j=i; i*j<n; j++)
            table[i*j] = 1;
    }

    for(int i=2; i<n; i++)
        if(table[i] == 0)
            prime[prime_cnt++] = i;

    free(table);
    *size = prime_cnt;
    return prime;
}

int* sieve_2(int n, int *size)
{
    int *table = malloc(n * sizeof(int));
    int *prime = malloc(n * sizeof(int) / 2);
    int prime_cnt = 0;

    memset(table, 0, n*sizeof(int));

    for(unsigned int i=2; i<n; i++)
    {
        if(table[i] == 0)
            prime[prime_cnt++] = i;
        for(unsigned int j=i*i; j<n; j+=i)
            table[j] = 1;
    }

    free(table);
    *size = prime_cnt;
    return prime;
}

int* sieve_3(int n, int *size)
{
    int *table = malloc(n * sizeof(int));
    int *prime = malloc(n * sizeof(int) / 2);
    int prime_cnt = 0;
    int tmp;

    memset(table, 0, n*sizeof(int));

    for(int i=2; i<n; i++)
    {
        if(table[i] == 0)
            prime[prime_cnt++] = i;
        for(int j=0; j<prime_cnt && i*prime[j]<n; j++)
        {
            tmp = prime[j];
            table[i*tmp] = 1;
            if(i % tmp == 0)
                break;
        }
    }

    free(table);
    *size = prime_cnt;
    return prime;
}

int check(void *buf1, void *buf2, int size)
{
    return memcmp(buf1, buf2, size);
}

int main()
{
    int size1, size2, size3;

    int *p1 = sieve_1(N, &size1);
    printf("size1: %d\n", size1);

    int *p2 = sieve_2(N, &size2);
    printf("size2: %d\n", size2);
    
    int *p3 = sieve_3(N, &size3);
    printf("size3: %d\n", size3);

    if(size1 == size2 && size2 == size3)
    {
        if(!check(p1, p2, size1) && !check(p2, p3, size1))
            printf("size: %d\n", size1);
    }
    else
    {
        printf("check failed\n");
    }
    

    free(p1);
    free(p2);
    free(p3);
}
