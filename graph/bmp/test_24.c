#include "mybmp.h"

#define X 200
#define Y 200

mybmp_24bits_pixel pic[Y][X];

int main()
{
    for(int i=0; i<Y; i++)
        for(int j=0; j<X; j++)
        {
            pic[i][j].r = 256/Y * i;
            pic[i][j].g = 256/X * j;
        }
    mybmp_create_24bits_bmp("test.bmp",pic,X*Y*sizeof(mybmp_24bits_pixel),X);
    return 0;
}