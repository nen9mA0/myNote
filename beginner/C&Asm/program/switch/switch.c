#include <stdio.h>

int main(void)
{
    int num;
    int i = 0;

    scanf("%d", &num);

    while(i<10)
    {
        switch (num%8)
        {
        case 0:
            num++;
            break;
        case 1:
            num += 10;
            break;
        case 2:
            num--;
            break;
        case 3:
            num -= 10;
            break;
        case 4:
            num *= 4;
            break;
        case 5:
            num *= 5;
            break;
        case 6:
            num /= 2;
            break;
        case 7:
            num /= 3;
            break;
        default:
            break;
        }
    }

    return 0;
}