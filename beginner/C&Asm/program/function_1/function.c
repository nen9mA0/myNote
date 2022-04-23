#include <stdio.h>

char str[] = "Hello World\n";

int print_str(const char *string, int a, int b, int c, int d)
{
    printf("%s", string);
    return a+b+c+d;
}

int main(void)
{
    print_str(str, 1, 2, 3, 4);
    return 0;
}
