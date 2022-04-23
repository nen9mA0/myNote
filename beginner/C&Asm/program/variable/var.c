#include <stdio.h>
#include <string.h>

int global_var = 100;
char string[] = "Hello World";
const char *const_str = "Hello World";

int table[100];

int main(void)
{
    int local_var = 101;
    static int static_var = 102;
    char* local_str = "Hello World";

    printf("global_var:         %d\n", global_var);
    printf("local_var:          %d\n", local_var);
    printf("static_var:         %d\n", static_var);

    printf("global char*:       %s\n", string);
    printf("global const char*: %s\n", const_str);
    printf("local str:          %s\n", local_str);

    memset(table, 0, sizeof(table));

    return 0;
}