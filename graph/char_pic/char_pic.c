#include <stdio.h>
#include <stdlib.h>

#define X 40
#define Y 40

int cycle(int x, int y);
int triangle(int l, int h, int x, int y);
int triangle2(int l, int h, int x, int y);
int diamond(int l, int h, int x, int y);

int main()
{
    int i,j;

    for(i=0; i<Y; i++)
    {
        for(j=0; j<X; j++)
        {
            printf(diamond(20,20,j,i)?"* ":"  ");
        }
        printf("\n");
    }
    return 0;
}

int cycle(int x, int y)
{
    return ( x*x + y*y <= 25*25 );
}

int triangle(int l, int h, int x, int y)
{
    double k = h / l;
    return (y < h-k*x);
}

int triangle2(int l, int h, int x, int y)
{
    int status = triangle(l,h,x-l,y);
    status &= triangle(-l,h,x-l,y);
    return status;
}

int diamond(int l, int h, int x, int y)
{
    int status;
    status = triangle2(-l,h,x,y-h);
    //status = triangle2(l,h,x,y-h);
    return status;
}