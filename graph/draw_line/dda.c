#include <string.h>
#include <math.h>
#include "mybmp.h"

#define X 400
#define Y 400

mybmp_24bits_pixel pic[Y][X];

int dda_draw_line(mybmp_24bits_pixel pic[Y][X], int x0, int x1, int y0, int y1)
{
    double k = y0-y1;
    k /= x0-x1;
    int s0 = x0>x1 ? x1 : x0;
    int s1 = y0>y1 ? y1 : y0;

    for(int i=0; i<abs(x0-x1); i++)
    {
        pic[ (int)(s1+k*i+0.5) ][i].r = 255;
        pic[ (int)(s1+k*i+0.5) ][i].g = 255;
        pic[ (int)(s1+k*i+0.5) ][i].b = 255;
    }
    return 0;
}

int main()
{
    memset(pic,0,sizeof(pic));
    dda_draw_line(pic,20,300,100,200);
    mybmp_create_24bits_bmp("test.bmp",pic,X*Y*sizeof(mybmp_24bits_pixel),X);
    return 0;
}