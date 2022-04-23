#include <stdio.h>

int gcd(int a, int b);

int main(void)
{
    int a = gcd(100, 48);
    printf("%d\n", a);
    return 0;
}

int gcd(int a, int b)
{
    int c;

    if(a < b)
    {
        c = a;
        a = b;
        b = c;
    }

    if(a%b)
        return gcd(b, a%b);
    else
        return b;
}