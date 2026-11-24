#include "global.h"
#include "draw.h"

#define swap(a,b) do { (a)^=(b);(b)^=(a);(a)^=(b); } while(0)

//bresenham line
void line(int x0, int y0, int x1, int y1)
{
    const bool steep = abs(y1 - y0) > abs(x1 - x0);

    if(steep)
    {
        swap(x0, y0);
        swap(x1, y1);
    }

    if(x0 > x1)
    {
        swap(x0, x1);
        swap(y0, y1);
    }
    int deltax = x1 - x0;
    int deltay = abs(y1 - y0);
    int error = 0;
    int ystep = (y0 < y1 ? 1 : -1);
    int y = y0;

    if(steep)
    {
        for(int x=x0; x<x1; x++)
        {
            error += deltay;
            if((2*error) >= deltax)
            {
                y += ystep;
                error -= deltax;
            }
        }
    }
    else
    {
        for(int x=x0; x<x1; x++)
        {
            error += deltay;
            if((2*error) >= deltax)
            {
                y += ystep;
                error -= deltax;
            }
        }
    }
}

void rect(int x, int y, int right, int bottom)
{
    line(x,y,right,y);
    line(right,y,right,bottom);
    line(right,bottom,x,bottom);
    line(x,bottom,x,y);
}

void rect_filled(int x, int y, int right, int bottom)
{
    for(int i=y; i<=bottom; i++)
    {
        line(x,i,right,i);
    }
}

int line_x, line_y;
void moveto(int x, int y)
{
    line_x = x;
    line_y = y;
}

void lineto(int x, int y)
{
    line(line_x,line_y,x,y);
    line_x = x;
    line_y = y;
}
