#include <string.h>
#include <math.h>
#include "mybmp.h"

#define X 400
#define Y 400

mybmp_24bits_pixel pic[Y][X];

void setpixel(mybmp_24bits_pixel pic[Y][X], int x, int y)
{
    pic[y][x].r = 255;
    pic[y][x].g = 255;
    pic[y][x].b = 255;
}

void bresenham(mybmp_24bits_pixel pic[Y][X], int x0, int y0, int x1, int y1)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;

    while (setpixel(pic, x0, y0), x0 != x1 || y0 != y1) {
        int e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 <  dy) { err += dx; y0 += sy; }
    }
}

void draw_circle_line(mybmp_24bits_pixel pic[Y][X], int n, int x, int y)
{
    int midx = x/2;
    int midy = y/2;
    double angle = 360.0 / n;
    double threshold_angle = atan(y/(double)x);

    
}

int main()
{
    memset(pic,0,sizeof(pic));
    bresenham(pic,20,300,100,300);
    mybmp_create_24bits_bmp("test.bmp",pic,X*Y*sizeof(mybmp_24bits_pixel),X);
    return 0;
}