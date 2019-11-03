#include <stdio.h>

int main()
{
    char a;
    char *pa = &a;

    int b;
    int *pb = &b;

    long long c;
    long long *pc = &c;

    struct test
    {
        int mem1;
        int mem2;
    } d;

    struct test *pd = &d;


    printf("pa:%p pa+1:%p\n", pa, pa+1);
    printf("pb:%p pb+1:%p\n", pb, pb+1);
    printf("pc:%p pc+1:%p\n", pc, pc+1);
    printf("pd:%p pd+1:%p\n", pd, pd+1);

    return 0;
}