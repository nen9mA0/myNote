#include <stdio.h>
#include <Windows.h>

int filter()
{
    printf("filter\n");
    return 1;
}
int main()
{
    __try {
        __try {
            int x = 0;
            int y = x/x;
        }
        __finally {
            printf("111\n");
        }
    }
    __except(filter()) {
        printf("222\n");
    }
}