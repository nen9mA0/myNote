#include <stdio.h>

char str[] = "Hello World\n";

#ifdef __GNUC__
int __attribute__((cdecl)) print_str_1(const char *string, int a, int b, int c, int d)
{
    printf("%s", string);
    return a+b+c+d;
}

int __attribute__((stdcall)) print_str_2(const char *string, int a, int b, int c, int d)
{
    printf("%s", string);
    return a+b+c+d;
}

int __attribute__((fastcall)) print_str_3(const char *string, int a, int b, int c, int d)
{
    printf("%s", string);
    return a+b+c+d;
}
#else
int __cdecl print_str_1(const char *string, int a, int b, int c, int d)
{
    printf("%s", string);
    return a+b+c+d;
}

int __stdcall print_str_2(const char *string, int a, int b, int c, int d)
{
    printf("%s", string);
    return a+b+c+d;
}

int __fastcall print_str_3(const char *string, int a, int b, int c, int d)
{
    printf("%s", string);
    return a+b+c+d;
}
#endif


// int print_str_1(const char *string, int a, int b, int c, int d)
// {
//     printf("%s", string);
//     return a+b+c+d;
// }

// int print_str_2(const char *string, int a, int b, int c, int d)
// {
//     printf("%s", string);
//     return a+b+c+d;
// }

// int print_str_3(const char *string, int a, int b, int c, int d)
// {
//     printf("%s", string);
//     return a+b+c+d;
// }


int main(void)
{
    print_str_1(str, 1, 2, 3, 4);
    print_str_2(str, 1, 2, 3, 4);
    print_str_3(str, 1, 2, 3, 4);
    return 0;
}
