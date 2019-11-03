#include "mybmp.h"

#define X 200
#define Y 200

my_bmp_16bits_565_pixel pic[Y][X];

int main()
{
    for(int i=0; i<Y; i++)
        for(int j=0; j<X; j++)
        {
            pic[i][j].pixel.data.r = 2^5 / Y * i;
            pic[i][j].pixel.data.g = 2^6 / X * j;
        }
    mybmp_create_16bits_565_bmp("test_16.bmp",pic,X*Y*sizeof(my_bmp_16bits_565_pixel),X);
    return 0;
}