#include "global.h"

//todo - global variables are suck
namespace Render
{
int clickX, clickY;
int sy[256];	//todo rename - used as buffer for Y part of vertices?
}

using namespace Render;

SetBrush *REN_bestBrush;
float REN_bestDistance;
SetBrush *REN_testBrush;
face_t *REN_bestFace;
face_t *REN_testFace;
SetBrush *REN_curBrush;

bool r_facetrans = false;

//vec3_t      r_ctr;
vec3_t			r_origin, r_matrix[3];
int			t_coloroffset;
int				t_width, t_height;
unsigned char	*t_data;
float			t_widthadd, t_heightadd;
int			r_width, r_height;
float			*r_zbuffer   = 0;
unsigned		char *r_picbuffer = NULL;
vec5_t			rightside, leftside, rightstep, leftstep;
face_t			*r_face;
bool			r_drawflat;
bool			r_drawent = false;
bool			r_noz = false;
bool			r_drawwire = false;
pixel32_t		r_flatcolor;
pixel32_t		r_entitycolor;

/*
====================
REN_ClearBuffers
====================
*/
void REN_ClearBuffers ()
{
    if (!r_zbuffer || !r_picbuffer)
        return;

    memset(r_zbuffer,0,r_width*r_height*sizeof(float));

    int size = r_height * r_width;

    if (set.sinBsp)
    {
        unsigned char r=0, g=0, b=0;
        if (!r_drawwire)
        {
            r = GetRValue(set.color_3dbackground);
            g = GetGValue(set.color_3dbackground);
            b = GetBValue(set.color_3dbackground);
        }
        while (size--)
        {
            *r_picbuffer++ = b;
            *r_picbuffer++ = g;
            *r_picbuffer++ = r;
        }
    }
    else
    {
        unsigned char idx = 0;
        if (!r_drawwire)
        {
            if (texWindow && set.pal)
                idx = (unsigned char)GetPaletteIndex(256, set.pal, set.color_3dbackground);
        }
        memset(r_picbuffer,idx,size);
    }
}

void REN_CopyBuffers (float *inz, float *outz, unsigned char *inp, unsigned char *outp)
{
    int size = r_height; // *4;

    float *z = inz;
    unsigned char *p = inp;

    float *oz = outz;
    unsigned char *op = outp;

    if (!z || !p || !oz || !op)
        return;

    while (size--)
    {
        memcpy(oz,z,r_width*sizeof(float));
        z += r_width;
        oz += r_width;
    }

    size = r_height * r_width;

    if (set.sinBsp)
    {
        while (size--)
        {
            *op++ = *p++;
            *op++ = *p++;
            *op++ = *p++;
        }
    }
    else
    {
        while (size--) *op++ = *p++;
    }
}
/*
====================
REN_SetTexture
====================
*/

void REN_SetTexture (face_t *face)
{
    if (!face->qtexture)
        face->qtexture = texWindow->GetQTexture(face->texture.texture);	// try to load
    qtexture_t	*q = face->qtexture;

    t_width = q->width;
    t_height = q->height;
    t_data = (unsigned char*)q->data;

    if (r_drawent)
    {
        r_flatcolor = r_entitycolor;
        t_coloroffset = set.flatlevel*256L; // calculate it on the fly...
    }
    else
    {
        r_flatcolor = q->flatcolor;
    }

    t_widthadd    = t_width*1024.0f;//was q->widthadd;
    t_heightadd   = t_height*1024.0f;//was q->heightadd;
    r_flatcolor.chan[0] *= r_face->light;
    r_flatcolor.chan[1] *= r_face->light;
    r_flatcolor.chan[2] *= r_face->light;
}

void REN_SetupColorsSin(unsigned char *whichColorR, unsigned char *whichColorG, unsigned char *whichColorB, int selected)
{

    switch (selected)
    {
    case 0:
        if (set.color_wire || r_drawent)
        {
            *whichColorB = r_flatcolor.chan[0];
            *whichColorG = r_flatcolor.chan[1];
            *whichColorR = r_flatcolor.chan[2];
        }
        else
        {
            *whichColorB = GetRValue(set.color_brushoutline);
            *whichColorG = GetGValue(set.color_brushoutline);
            *whichColorR = GetBValue(set.color_brushoutline);
        }
        break;
    case 2:
        *whichColorB = GetRValue(set.color_faceoutline);
        *whichColorG = GetGValue(set.color_faceoutline);
        *whichColorR = GetBValue(set.color_faceoutline);
        break;
    case 3:
        *whichColorB = GetRValue(set.color_lock);
        *whichColorG = GetGValue(set.color_lock);
        *whichColorR = GetBValue(set.color_lock);
        break;
    default:
        *whichColorB = GetRValue(set.color_selectoutline);
        *whichColorG = GetGValue(set.color_selectoutline);
        *whichColorR = GetBValue(set.color_selectoutline);
    }
}

void REN_SetupColors(unsigned char *whichColor, int selected)
{
    switch(selected)
    {
    case 0:
        if (set.color_wire || r_drawent)
            *whichColor = (unsigned char)texColorMap->lookUp[t_coloroffset+r_flatcolor.p];
        else
            *whichColor = (unsigned char)set.outlineIndex;
        break;
    case 2:
        *whichColor = (unsigned char)set.faceIndex;
        break;
    case 3:
        *whichColor = (unsigned char)set.lockIndex;
        break;
    default:
        *whichColor = (unsigned char)set.selectedIndex;
    }
}

/*
==================
REN_DrawSpan  Regular BSP
==================
*/
void REN_DrawSpan (int y)
{
    int			x, count;
    int			ofs;
    int			tx, ty;
    int			x1, x2;
    float		ufrac, vfrac, zfrac;
    float    ustep, vstep, zstep;
    float *rz;
    unsigned char *pic;
    unsigned char *lu;
    float		scale;

    if(r_facetrans && set.render_trans==1 && (y & 1))
        return;

    if (y<0 || y >= r_height)
        return;

    x1 = (leftside[0]);
    x2 = (rightside[0]);

    count = x2 - x1;
    if (count <= 0)
        return;

    zfrac = leftside[2];
    ufrac = leftside[3];
    vfrac = leftside[4];

    scale = 1.0/count;

    zstep = (rightside[2] - zfrac)*scale;
    ustep = (rightside[3] - ufrac)*scale;
    vstep = (rightside[4] - vfrac)*scale;

    if (x1 < 0)
    {
        ufrac -= x1*ustep;
        vfrac -= x1*vstep;
        zfrac -= x1*zstep;
        x1 = 0;
    }

    if (x2 > r_width)
        x2 = r_width;

    ofs = y*r_width+x1;
    rz = &r_zbuffer[ofs];
    pic = &r_picbuffer[ofs];
    lu = &texColorMap->lookUp[t_coloroffset];

    for (x=x1 ; x < x2 ; x++)
    {

        //translucency
        if(!r_facetrans || !(set.render_trans==2 && ((x+y)&1)))
        {
            if (*rz <= zfrac)
            {
                scale = 1/zfrac;
                *rz = zfrac;

                tx = (int)((ufrac*scale)+t_widthadd) % t_width; // was % t_width
                ty = (int)((vfrac*scale)+t_heightadd) % t_height;

                ofs = ty*t_width + tx;
                *pic = lu[t_data[ofs]];
            }
        }
        pic++;
        rz++;
        ufrac += ustep;
        vfrac += vstep;
        zfrac += zstep;
    }
}

/*
==================
REN_DrawSpanNoZ Regular BSP
==================
*/
void REN_DrawSpanNoZ (int y)
{
    int			x, count;
    int			ofs;
    int			tx, ty;
    int			x1, x2;
    float		ufrac, vfrac, zfrac;
    float    ustep, vstep, zstep;
    unsigned char *in, *out;
    float		scale;

    if (y<0 || y >= r_height)
        return;

    x1 = (leftside[0]);
    x2 = (rightside[0]);

    count = x2 - x1;
    if (count < 0)
        return;

    zfrac = leftside[2];
    ufrac = leftside[3];
    vfrac = leftside[4];

    if (!count)
        scale = 1;
    else
        scale = 1.0/count;

    zstep = (rightside[2] - zfrac)*scale;
    ustep = (rightside[3] - ufrac)*scale;
    vstep = (rightside[4] - vfrac)*scale;

    if (x1 < 0)
    {
        ufrac -= x1*ustep;
        vfrac -= x1*vstep;
        zfrac -= x1*zstep;
        x1 = 0;
    }

    if (x2 > (int)r_width)
        x2 = (int)(r_width);

    ofs = y*r_width+x1;

    for (x=x1 ; x < x2 ; x++)
    {
        scale = 1/zfrac;
        tx = (int)((ufrac*scale)+t_widthadd) % t_width; // was % t_width
        ty = (int)((vfrac*scale)+t_heightadd) % t_height;
        in = &t_data [ty*t_width+tx];
        out = &r_picbuffer[ofs];
        *out = texColorMap->lookUp[t_coloroffset+*in];
        ufrac += ustep;
        vfrac += vstep;
        zfrac += zstep;
        ofs ++;
    }
}

/*
==================
REN_DrawFlatSpan Regular BSP
==================
*/
void REN_DrawFlatSpan (int y)
{
    int			x, count;
    int			ofs;
    int			x1, x2;
    float		zfrac, zstep;
    unsigned char *out;

    if(r_facetrans && set.render_trans==1 && (y & 1))
        return;

    if (y<0 || y >= r_height)
        return;

    x1 = (leftside[0]);
    x2 = (rightside[0]);

    count = x2 - x1;
    if (count <= 0)
        return;

    zfrac = leftside[2];

    zstep = (rightside[2] - zfrac)/count;

    if (x1 < 0)
    {
        zfrac -= x1*zstep;
        x1 = 0;
    }

    if (x2 > (int)r_width)
        x2 = (int)r_width;

    ofs = y*r_width+x1;

    for (x=x1 ; x < x2 ; x++)
    {
        //translucency
        if(!r_facetrans || !(set.render_trans==2 && ((x+y) & 1)))
        {
            if (r_zbuffer[ofs] <= zfrac)
            {
                r_zbuffer[ofs] = zfrac;
                out = &r_picbuffer[ofs]; // combine?
                *out = texColorMap->lookUp[t_coloroffset+r_flatcolor.p];
            }
        }
        zfrac += zstep;
        ofs ++;
    }
}

////////
/*
==================
REN_DrawSpanSin
==================
*/
void REN_DrawSpanSin (int y)
{
    int			    x, count;
    int			    ofs;
    int			    tx, ty;
    int			    x1, x2;
    float		    ufrac, vfrac, zfrac;
    float           ustep, vstep, zstep;
    float           *rz;
    unsigned char   *pic;
    float		    scale;

    unsigned char *in;
    if (y<0 || y >= r_height)
        return;

    x1 = (leftside[0]);
    x2 = (rightside[0]);

    count = x2 - x1;
    if (count <= 0)
        return;

    zfrac = leftside[2];
    ufrac = leftside[3];
    vfrac = leftside[4];
    // lightfrac = r_face->light;

    scale = 1.0/count;

    zstep = (rightside[2] - zfrac)*scale;
    ustep = (rightside[3] - ufrac)*scale;
    vstep = (rightside[4] - vfrac)*scale;

    if (x1 < 0)
    {
        ufrac -= x1*ustep;
        vfrac -= x1*vstep;
        zfrac -= x1*zstep;
        x1 = 0;
    }

    if (x2 > (int)r_width)
        x2 = (int)(r_width);

    ofs = y*r_width+x1;
    rz = &r_zbuffer[ofs];
    pic = &r_picbuffer[(ofs << 1) + ofs];

    for (x=x1 ; x < x2 ; x++)
    {
        if (*rz <= zfrac)
        {
            scale = 1/zfrac;
            *rz = zfrac;

            tx = (int)((ufrac*scale)+t_widthadd) % t_width; // was % t_width
            ty = (int)((vfrac*scale)+t_heightadd) % t_height;

            ofs = ty*t_width + tx;
            in = &t_data[(ofs << 1) + ofs];
            *pic++ = *in++;
            *pic++ = *in++;
            *pic++ = *in;
        }
        else
        {
            pic+=3;
        }
        rz++;
        ufrac += ustep;
        vfrac += vstep;
        zfrac += zstep;
    }
}

/*
==================
REN_DrawSpanNoZSin
==================
*/
void REN_DrawSpanNoZSin(int y)
{
    int			x, count;
    int			ofs;
    int			tx, ty;
    int			x1, x2;
    float		ufrac, vfrac, zfrac;
    float    ustep, vstep, zstep;
    unsigned char *in, *out;
    float		scale;

    if (y<0 || y >= r_height)
        return;

    x1 = (leftside[0]);
    x2 = (rightside[0]);

    count = x2 - x1;
    if (count < 0)
        return;

    zfrac = leftside[2];
    ufrac = leftside[3];
    vfrac = leftside[4];

    if (!count)
        scale = 1;
    else
        scale = 1.0/count;

    zstep = (rightside[2] - zfrac)*scale;
    ustep = (rightside[3] - ufrac)*scale;
    vstep = (rightside[4] - vfrac)*scale;

    if (x1 < 0)
    {
        ufrac -= x1*ustep;
        vfrac -= x1*vstep;
        zfrac -= x1*zstep;
        x1 = 0;
    }

    if (x2 > (int)r_width)
        x2 = (int)(r_width);

    ofs = y*r_width+x1;

    for (x=x1 ; x < x2 ; x++)
    {
        scale = 1/zfrac;
        tx = (int)((ufrac*scale)+t_widthadd) % t_width;
        ty = (int)((vfrac*scale)+t_heightadd) % t_height;
        in = &t_data [3L*(ty*t_width+tx)];
        out = &r_picbuffer[(ofs << 1) + ofs];
        *out++ = *in++;
        *out++ = *in++;
        *out = *in;
        ufrac += ustep;
        vfrac += vstep;
        zfrac += zstep;
        ofs ++;
    }
}

/*
==================
REN_DrawFlatSpanSin
==================
*/
void REN_DrawFlatSpanSin(int y)
{
    int			x, count;
    int			ofs;
    int			x1, x2;
    float		zfrac, zstep;
    unsigned char *out;

    if (y<0 || y >= r_height)
        return;

    x1 = (leftside[0]);
    x2 = (rightside[0]);

    count = x2 - x1;
    if (count <= 0)
        return;

    zfrac = leftside[2];

    zstep = (rightside[2] - zfrac)/count;

    if (x1 < 0)
    {
        zfrac -= x1*zstep;
        x1 = 0;
    }

    if (x2 > (int)r_width)
        x2 = (int)r_width;

    ofs = y*r_width+x1;

    for (x=x1 ; x < x2 ; x++)
    {
        if (r_zbuffer[ofs] <= zfrac)
        {
            r_zbuffer[ofs] = zfrac;
            out = &r_picbuffer[(ofs << 1) + ofs];
            *out++ = r_flatcolor.chan[0];
            *out++ = r_flatcolor.chan[1];
            *out = r_flatcolor.chan[2];
        }
        zfrac += zstep;
        ofs ++;
    }
}
/*
=====================
REN_RasterizeFace

=====================
*/
void REN_RasterizeFace (winding_t *w)
{
    int			y;
    int			i;
    int			top, bot;
    int			leftv, rightv;
    int			count;
    int 		numvertex;

//
// find top vertex
//
    numvertex = w->numpoints;

    top = INT_MAX;
    bot = INT_MIN;
    leftv = 0;

    for (i=0 ; i<numvertex ; i++)
    {
        w->points[i][3] *= w->points[i][2];
        w->points[i][4] *= w->points[i][2];

        sy[i] = (int)w->points[i][1];

        if (sy[i] < top)
        {
            top = sy[i];
            leftv = i;
        }
        if (sy[i] > bot)
            bot = sy[i];
    }
    rightv = leftv;

    if (top < 0 || bot > r_height || top > bot)
        return;		// shouldn't have to have this...

//
// render a trapezoid
//
    y = top;

    while (y < bot)
    {
        if (y >= sy[leftv])
        {
            do
            {
                for (i=0 ; i<5 ; i++)
                    leftside[i] = w->points[leftv][i];
                leftv--;
                if (leftv == -1)
                    leftv = numvertex-1;
            }
            while (sy[leftv] <= y);
            count = sy[leftv]-y;
            for (i=0 ; i<5 ; i++)
                leftstep[i] = (w->points[leftv][i] - leftside[i])/count;
        }
        if (y >= sy[rightv])
        {
            do
            {
                for (i=0 ; i<5 ; i++)
                    rightside[i] = w->points[rightv][i];
                rightv++;
                if (rightv == numvertex)
                    rightv = 0;
            }
            while (sy[rightv] <= y);
            count = sy[rightv]-y;
            for (i=0 ; i<5 ; i++)
                rightstep[i] = (w->points[rightv][i] - rightside[i])/count;
        }

        if (set.sinBsp)
        {
            if (r_drawflat)
                REN_DrawFlatSpanSin(y);
            else if (r_noz)
                REN_DrawSpanNoZSin(y);
            else
                REN_DrawSpanSin(y);
        }
        else
        {
            if (r_drawflat)
                REN_DrawFlatSpan (y);
            else if (r_noz)
                REN_DrawSpanNoZ (y);
            else
                REN_DrawSpan (y);
        }

        for (i=0 ; i<5 ; i++)
        {
            leftside[i] += leftstep[i];
            rightside[i] += rightstep[i];
        }

        y++;
    }
}

void REN_RasterizeFaceOutline (winding_t *w, int selected)
{
    int	y;
    int	i;
    int	top, bot;
    int	leftv, rightv;
    int	count;
    int numvertex;

//
// find top vertex
//
    numvertex = w->numpoints;
    top = INT_MAX;
    bot = INT_MIN;
    leftv = 0;

    for (i=0 ; i<numvertex ; i++)
    {
        w->points[i][3] *= w->points[i][2];
        w->points[i][4] *= w->points[i][2];

        sy[i] = (int)w->points[i][1];

        if (sy[i] < top)
        {
            top = sy[i];
            leftv = i;
        }
        if (sy[i] > bot)
            bot = sy[i];
    }
    rightv = leftv;

    if (top < 0 || bot > r_height || top > bot)
        return;		// shouldn't have to have this...

//
// render a trapezoid
//

    for (y = top; y < bot; y++)
    {
        if (y >= sy[leftv])
        {
            do
            {
                for (i=0 ; i<5 ; i++)
                    leftside[i] = w->points[leftv][i];
                leftv--;
                if (leftv == -1)
                    leftv = numvertex-1;
            }
            while (sy[leftv] <= y);
            count = sy[leftv]-y;
            for (i=0 ; i<5 ; i++)
                leftstep[i] = (w->points[leftv][i] - leftside[i])/count;
        }
        if (y >= sy[rightv])
        {
            do
            {
                for (i=0 ; i<5 ; i++)
                    rightside[i] = w->points[rightv][i];
                rightv++;
                if (rightv == numvertex)
                    rightv = 0;
            }
            while (sy[rightv] <= y);
            count = sy[rightv]-y;
            for (i=0 ; i<5 ; i++)
                rightstep[i] = (w->points[rightv][i] - rightside[i])/count;
        }

        if (set.sinBsp)
        {
            if (r_drawflat)
                REN_DrawFlatSpanOutlineSin(y,selected);
            else if (r_noz)
                REN_DrawSpanOutlineNoZSin(y, selected);
            else
                REN_DrawSpanOutlineSin(y, selected);
        }
        else
        {
            if (r_drawflat)
                REN_DrawFlatSpanOutline (y,selected);
            else if (r_noz)
                REN_DrawSpanOutlineNoZ (y, selected);
            else
                REN_DrawSpanOutline(y, selected);
        }

        for (i=0 ; i<5 ; i++)
        {
            leftside[i] += leftstep[i];
            rightside[i] += rightstep[i];
        }
    }
}

void REN_RasterizeFaceWire (winding_t *w, int selected)
{
    int numvertex = w->numpoints;
    float *p1, *p2;

    for (int i=0 ; i<numvertex ; i++)
    {
        p1 = w->points[i];
        p2 = w->points[(i+1) % numvertex];

        if (set.sinBsp)
        {
            REN_LineDrawZSin(p1, p2, selected);
        }
        else
        {
            REN_LineDrawZ(p1, p2, selected);
        }
    }
}

void REN_RasterizeLineWire (winding_t *w, int selected)
{
    int numvertex = w->numpoints;
    float *p1, *p2;

    for (int i=0 ; i< (numvertex - 1) ; i++)
    {
        p1 = w->points[i];
        p2 = w->points[(i+1) % numvertex];

        if (set.sinBsp)
        {
            REN_LineDrawZSin(p1, p2, selected);
        }
        else
        {
            REN_LineDrawZ(p1, p2, selected);
        }
    }
}
//=============================================================================

/*
==================
REN_DrawSpanLinear Regular BSP
==================
*/
void REN_DrawSpanLinear (int y)
{
    int			x, count;
    int			ofs;
    int			tx, ty;
    int			x1, x2;
    float		ufrac, vfrac, zfrac, ustep, vstep, zstep;
    unsigned char *in, *out;
    float		scale;

    if (y<0 || y >= r_height)
        return;

    x1 = (leftside[0]);
    x2 = (rightside[0]);

    count = x2 - x1;
    if (count < 0)
        return;

    zfrac = leftside[2];
    ufrac = leftside[3];
    vfrac = leftside[4];

    if (!count)
        scale = 1;
    else
        scale = 1.0/count;

    zstep = (rightside[2] - zfrac)*scale;
    ustep = (rightside[3] - ufrac)*scale;
    vstep = (rightside[4] - vfrac)*scale;

    if (x1 < 0)
    {
        ufrac -= x1*ustep;
        vfrac -= x1*vstep;
        zfrac -= x1*zstep;
        x1 = 0;
    }

    if (x2 > (int)r_width)
        x2 = (int)r_width;

    ofs = y*r_width+x1;

    for (x=x1 ; x < x2 ; x++)
    {
        if (r_zbuffer[ofs] <= zfrac)
        {
            r_zbuffer[ofs] = zfrac;

            tx = (int)(ufrac+t_widthadd) % t_width;
            ty = (int)(vfrac+t_heightadd) % t_height;
            in = &t_data [ty*t_width+tx];

            out = &r_picbuffer[ofs];
            *out = texColorMap->lookUp[t_coloroffset+*in];
        }

        ufrac += ustep;
        vfrac += vstep;
        zfrac += zstep;
        ofs ++;
    }
}

/*
==================
REN_DrawSpanLinearSin
==================
*/
void REN_DrawSpanLinearSin (int y)
{
    int			x, count;
    int			ofs;
    int			tx, ty;
    int			x1, x2;
    float		ufrac, vfrac, zfrac, ustep, vstep, zstep;
    unsigned char *in, *out;
    float		scale;

    if (y < 0 || y >= r_height)
        return;

    x1 = (leftside[0]);
    x2 = (rightside[0]);

    count = x2 - x1;
    if (count < 0)
        return;

    zfrac = leftside[2];
    ufrac = leftside[3];
    vfrac = leftside[4];

    if (!count)
        scale = 1;
    else
        scale = 1.0/count;

    zstep = (rightside[2] - zfrac)*scale;
    ustep = (rightside[3] - ufrac)*scale;
    vstep = (rightside[4] - vfrac)*scale;

    if (x1 < 0)
    {
        ufrac -= x1*ustep;
        vfrac -= x1*vstep;
        zfrac -= x1*zstep;
        x1 = 0;
    }

    if (x2 > (int)r_width)
        x2 = (int)r_width;

    ofs = y*r_width+x1;

    for (x=x1 ; x < x2 ; x++)
    {
        if (r_zbuffer[ofs] <= zfrac)
        {
            r_zbuffer[ofs] = zfrac;

            tx = (int)(ufrac+t_widthadd) % t_width;
            ty = (int)(vfrac+t_heightadd) % t_height;
            in = &t_data [3L*(ty*t_width+tx)];

            out = &r_picbuffer[(ofs << 1) + ofs];
            *out++ = *in++;
            *out++ = *in++;
            *out = *in;
        }
        ufrac += ustep;
        vfrac += vstep;
        zfrac += zstep;
        ofs ++;
    }
}
/*
=====================
REN_RasterizeFaceLinear

=====================
*/
void REN_RasterizeFaceLinear (winding_t *w)
{
    int			y;
    int			i;
    int			top, bot;
    int			leftv, rightv;
    int			count;
    int			numvertex;

//
// find top vertex
//
    numvertex = w->numpoints;
    top = INT_MAX;
    bot = INT_MIN;

    leftv = 0;
    for (i=0 ; i<numvertex ; i++)
    {
        sy[i] = (int)w->points[i][1];

        if (sy[i] < top)
        {
            top = sy[i];
            leftv = i;
        }
        if (sy[i] > bot)
            bot = sy[i];
    }
    rightv = leftv;

    if (top < 0 || bot > r_height || top > bot)
        return;		// shouldn't have to have this...

//
// render a trapezoid
//
    y = top;

    while (y < bot)
    {
        if (y >= sy[leftv])
        {
            do
            {
                for (i=0 ; i<5 ; i++)
                    leftside[i] = w->points[leftv][i];
                leftv--;
                if (leftv == -1)
                    leftv = numvertex-1;
            }
            while (sy[leftv] <= y);
            count = sy[leftv]-y;
            for (i=0 ; i<5 ; i++)
                leftstep[i] = (w->points[leftv][i] - leftside[i])/count;
        }
        if (y >= sy[rightv])
        {
            do
            {
                for (i=0 ; i<5 ; i++)
                    rightside[i] = w->points[rightv][i];
                rightv++;
                if (rightv == numvertex)
                    rightv = 0;
            }
            while (sy[rightv] <= y);
            count = sy[rightv]-y;
            for (i=0 ; i<5 ; i++)
                rightstep[i] = (w->points[rightv][i] - rightside[i])/count;
        }

        if (set.sinBsp)
            REN_DrawSpanLinearSin(y);
        else
            REN_DrawSpanLinear(y);

        for (i=0 ; i<5 ; i++)
        {
            leftside[i] += leftstep[i];
            rightside[i] += rightstep[i];
        }

        y++;
    }
}

//============================================================================

/*
==================
REN_BeginCamera
===================
*/
float	r_width_2, r_height_3;
plane_t	frustum[5];

void REN_BeginCamera()
{
    if (!r_width || !r_height || fabs(set.ppd*set.mag) < 0.001)
        return;

    r_width_2 = (float)r_width / 2;
    r_height_3 = (float)r_height / 3;


// clip to right side
    frustum[0].normal[0] = -1;
    frustum[0].normal[1] = 0;
    frustum[0].normal[2] = 1;
    frustum[0].dist = 0;

// clip to left side
    frustum[1].normal[0] = 1;
    frustum[1].normal[1] = 0;
    frustum[1].normal[2] = 1;
    frustum[1].dist = 0;

// clip to top side
    frustum[2].normal[0] = 0;
    frustum[2].normal[1] = -1;
    frustum[2].normal[2] = r_height_3 / r_width_2;
    frustum[2].dist = 0;

// clip to bottom side
    frustum[3].normal[0] = 0;
    frustum[3].normal[1] = 1;
    frustum[3].normal[2] = 2*r_height_3 / r_width_2;
    frustum[3].dist = 0;

// near Z
    frustum[4].normal[0] = 0;
    frustum[4].normal[1] = 0;
    frustum[4].normal[2] = 1;
    frustum[4].dist = 1;
}

void REN_BeginXY()
{
    frustum[0].normal[0] = 1;
    frustum[0].normal[1] = 0;
    frustum[0].normal[2] = 0;
    frustum[0].dist = 0;

    frustum[1].normal[0] = -1;
    frustum[1].normal[1] = 0;
    frustum[1].normal[2] = 0;
    frustum[1].dist = -r_width;

    frustum[2].normal[0] = 0;
    frustum[2].normal[1] = 1;
    frustum[2].normal[2] = 0;
    frustum[2].dist = 0;

    frustum[3].normal[0] = 0;
    frustum[3].normal[1] = -1;
    frustum[3].normal[2] = 0;
    frustum[3].dist = -r_height;
}

/*
=====================
REN_DrawCameraFace
=====================
*/
void REN_DrawCameraFace(face_t *idpol,int selected)
{
    if (!EditSize.cx || !EditSize.cy)
        return;

    int		i;
    float	scale;
    int		numvertex;
    winding_t *w, *in;
    vec3_t temp;
    float dot;
    float mag;

    if (!idpol->w)
        return;	// overconstrained plane

    r_face = idpol;

    dot = DotProduct (r_origin, idpol->plane.normal);
    //
    // back face cull
    //
    if ((dot <= idpol->plane.dist) && (!r_drawwire || set.cull_wire))
        return;

    //
    // transform in 3D (should be optimized by clipping first, then transforming)
    //
    in = idpol->w;
    numvertex = in->numpoints;

    w = NewWinding(numvertex);
    if(!w) return;

    for (i=0 ; i<numvertex ; i++)
    {
        VectorSubtract(in->points[i], r_origin, temp);

        w->points[i][0] = DotProduct(temp,r_matrix[0]);
        w->points[i][1] = DotProduct(temp,r_matrix[1]);
        w->points[i][2] = DotProduct(temp,r_matrix[2]);

        w->points[i][3] = in->points[i][3];
        w->points[i][4] = in->points[i][4];
    }

    //
    // 3D clip
    //
    for (i=0 ; i<4 ; i++)
    {
        w = ClipWinding(w, &frustum[i]);
        if (!w)
            return;
    }

    vec3_t dirV1, dirV2, dirV;

    // would be better to use an "eye" vector from cur pos in eye direction...
    map *m = map_i[set.curmap];

    VectorCopy(m->eye[m->cureye],dirV1);

    float yaw = m->angles[m->cureye].yaw;
    dirV2[0] = dirV1[0] + set.stepsize*cos(yaw-M_PI/2.0);
    dirV2[1] = dirV1[1] + set.stepsize*sin(yaw-M_PI/2.0);
    dirV2[2] = dirV1[2];

    VectorSubtract(dirV2,dirV1,dirV);

    VectorNormalize (dirV);

    mag = -DotProduct(dirV,idpol->plane.normal);

    mag = (63.0-set.lightintensity) + mag*set.lightintensity;

    if (mag < set.ambientintensity) mag = set.ambientintensity;
    if (mag > 63.0) mag = 63.0;

    if ((r_drawflat && set.shadeflats) || (!r_drawflat && set.shadetextures))
        t_coloroffset = (256*(63-(int)mag));
    else
        t_coloroffset = 256*set.flatlevel;

    if (!set.facelighting)
        t_coloroffset = 31*256;

    //
    // project to 2D
    //
    for (i=0 ; i<w->numpoints ; i++)
    {
        if (fabs(w->points[i][2]) < FP_EPSILON)
            scale = 1.0;
        else
            scale = r_width_2/w->points[i][2];

        w->points[i][0] = r_width_2 + scale*w->points[i][0];
        w->points[i][1] = r_height_3 - scale*w->points[i][1];
        w->points[i][2] = scale;
    }

    REN_SetTexture (idpol);
    if (r_drawwire)
    {
        REN_RasterizeFaceWire (w,selected);
    }
    else
    {

        r_facetrans = (bool) r_face->transparent;

        if (set.outline || selected)
            REN_RasterizeFaceOutline (w,selected);
        else
            REN_RasterizeFace (w);
    }
    delete [] w;
}

// For Model
void REN_DrawWinding(winding_t *inw,int selected)
{
    if (!EditSize.cx || !EditSize.cy)
        return;

    int		i;
    float		scale;
    int			numvertex;
    winding_t	*w, *in;
    vec3_t		temp;

    if (!inw)
        return;	// overconstrained plane

    //
    // transform in 3D (should be optimized by clipping first, then transforming)
    //
    in = inw;
    numvertex = in->numpoints;

    w = NewWinding (numvertex);
    if(!w) return;

    //VectorCopy(r_origin,r_ctr);

    for (i=0 ; i<numvertex ; i++)
    {
        VectorSubtract (in->points[i], r_origin, temp);

        w->points[i][0] = DotProduct(temp,r_matrix[0]);
        w->points[i][1] = DotProduct(temp,r_matrix[1]);
        w->points[i][2] = DotProduct(temp,r_matrix[2]);

        //for (j = 0; j < 3; j++) {
        //	r_ctr[j] += w->points[i][j];
        //};

        w->points[i][3] = in->points[i][3];
        w->points[i][4] = in->points[i][4];
    }

    //
    // 3D clip
    //
    for (i=0 ; i<4 ; i++)
    {
        w = ClipWinding (w, &frustum[i]);
        if (!w)
            return;
    }

    t_coloroffset = 31*256;
    r_flatcolor = r_entitycolor;
    //
    // project to 2D
    //
    for (i=0 ; i<w->numpoints ; i++)
    {
        if (fabs(w->points[i][2]) >= FP_EPSILON)
        {
            scale = r_width_2/w->points[i][2];

            w->points[i][0] = r_width_2 + scale*w->points[i][0];
            w->points[i][1] = r_height_3 - scale*w->points[i][1];
            w->points[i][2] = scale;
        }
        else
        {
            w->points[i][0] = r_width_2 + w->points[i][0];
            w->points[i][1] = r_height_3 - w->points[i][1];
            w->points[i][2] = 1.0;
        }
    }
    REN_RasterizeFaceWire (w,selected);
    delete [] w;
}

// For Model / leaks
void REN_DrawLineWinding(winding_t *inw, int selected)
{
    if (!EditSize.cx || !EditSize.cy)
        return;

    if (!inw)
        return;	// overconstrained plane

    //
    // transform in 3D (should be optimized by clipping first, then transforming)
    //
    winding_t *in = inw;
    int numvertex = in->numpoints;

    winding_t *w = NewWinding (numvertex);
    if(!w) return;

    vec3_t temp;

    for (int i=0 ; i<numvertex ; i++)
    {
        VectorSubtract (in->points[i], r_origin, temp);

        w->points[i][0] = DotProduct(temp,r_matrix[0]);
        w->points[i][1] = DotProduct(temp,r_matrix[1]);
        w->points[i][2] = DotProduct(temp,r_matrix[2]);
        w->points[i][3] = in->points[i][3];
        w->points[i][4] = in->points[i][4];
    }

    //
    // 3D clip
    //
    for (int i=0 ; i<4 ; i++)
    {
        w = ClipWinding (w, &frustum[i]);
        if (!w)
            return;
    }

    //
    // project to 2D
    //
    for (int i=0 ; i<w->numpoints ; i++)
    {
        if (fabs(w->points[i][2]) < FP_EPSILON)
        {
            w->points[i][0] = r_width_2 + w->points[i][0];
            w->points[i][1] = r_height_3 - w->points[i][1];
            w->points[i][2] = 1.0f;
        }
        else
        {
            float scale = r_width_2/w->points[i][2];
            w->points[i][0] = r_width_2 + scale*w->points[i][0];
            w->points[i][1] = r_height_3 - scale*w->points[i][1];
            w->points[i][2] = scale;
        }
    }
    REN_RasterizeLineWire (w,selected);
    delete [] w;
}

// For Model
void REN_RenderWinding(winding_t *inw,int selected)
{
    if (!EditSize.cx || !EditSize.cy)
        return;

    int		i;
    float		scale;
    int			numvertex;
    winding_t	*w, *in;
    vec3_t		temp;

    if (!inw)
        return;	// overconstrained plane

    //
    // transform in 3D (should be optimized by clipping first, then transforming)
    //
    in = inw;
    numvertex = in->numpoints;

    w = NewWinding (numvertex);
    if(!w) return;
    w->numpoints = numvertex;

    for (i=0 ; i<numvertex ; i++)
    {
        VectorSubtract (in->points[i], r_origin, temp);

        w->points[i][0] = DotProduct(temp,r_matrix[0]);
        w->points[i][1] = DotProduct(temp,r_matrix[1]);
        w->points[i][2] = DotProduct(temp,r_matrix[2]);

        w->points[i][3] = in->points[i][3];
        w->points[i][4] = in->points[i][4];
    }

    // 3D clip
    for (i=0 ; i<4 ; i++)
    {
        w = ClipWinding (w, &frustum[i]);
        if (!w)
            return;
    }

    if (!selected)
    {
        t_coloroffset = 31L*256L;
    }
    else
    {
        t_coloroffset = 20L*256L;
    }
    r_flatcolor = r_entitycolor;

    //
    // project to 2D
    //
    for (i=0 ; i<w->numpoints ; i++)
    {
        if (fabs(w->points[i][2]) < FP_EPSILON)
        {
            scale = 1.0;
        }
        else
        {
            scale = r_width_2/w->points[i][2];
        }
        w->points[i][0] = r_width_2 + scale*w->points[i][0];
        w->points[i][1] = r_height_3 - scale*w->points[i][1];
        w->points[i][2] = scale;
    }
    REN_RasterizeFace(w);
    delete [] w;
}

/*
=====================
REN_DrawCameraPoint Regular BSP
=====================
*/
void REN_DrawCameraPoint(float *pt,int selected)
{
    if (!EditSize.cx || !EditSize.cy)
        return;

    plane_t *plane;
    vec3_t temp, plot;
    float dot;
    int ofs;
    unsigned char *out;
    unsigned char whichColor;

    if (!pt)
        return;	// overconstrained plane

    vec3_t *xp = (vec3_t *)pt;
    // move to camera origin
    VectorSubtract (*xp, r_origin, temp);

    // rotate view view matrix...
    plot[0] = DotProduct(temp,r_matrix[0]);
    plot[1] = DotProduct(temp,r_matrix[1]);
    plot[2] = DotProduct(temp,r_matrix[2]);

    //
    // 3D clip
    //
    for (int i=0 ; i<4 ; i++)
    {
        plane = &frustum[i];
        if (!plane)
            continue;

        dot = DotProduct(plot, plane->normal);
        // clipped away, so exit...
        if (dot <= plane->dist)
            return;
    }

    //
    // project to 2D
    //
    float scale = 1.0;
    if (fabs(plot[2]) >= FP_EPSILON)
    {
        scale = r_width_2/plot[2];
    }

    plot[0] = (int)(r_width_2 + scale*plot[0]);
    plot[1] = (int)(r_height_3 - scale*plot[1]);
    plot[2] = scale;

    if (plot[1] < 0 || plot[1] >= r_height)
        return;

    if (plot[0] < 0 || plot[0] >= r_width)
        return;

    REN_SetupColors(&whichColor,selected);

    ofs = plot[1]*r_width + plot[0];

    if (r_zbuffer[ofs] <= plot[2])
    {
        r_zbuffer[ofs] = plot[2];
        out = &r_picbuffer[ofs];
        *out = whichColor;
    }
}
/*
=====================
REN_PointToScreen Regular BSP  - return true if visible, outpt will be -1,-1 if not inside screen
=====================
*/
bool REN_PointToScreen(float *pt, POINT *outpt)
{
    outpt->x = outpt->y = -1;

    if (!EditSize.cx || !EditSize.cy)
        return false;

    float	scale;
    vec3_t	temp,plot;
    int ofs;

    if (!pt)
        return false;	// overconstrained plane

    vec3_t *xp;
    xp = (vec3_t *)pt;
    // move to camera origin
    VectorSubtract (*xp, r_origin, temp);


    // rotate view view matrix...
    plot[0] = DotProduct(temp,r_matrix[0]);
    plot[1] = DotProduct(temp,r_matrix[1]);
    plot[2] = DotProduct(temp,r_matrix[2]);

    //
    // project to 2D
    //
    if (fabs(plot[2]) < FP_EPSILON)
        return false;							//remove points that are too close to the camera

    scale = r_width_2/plot[2];


    plot[0] = (int)(r_width_2 + scale*plot[0]);
    plot[1] = (int)(r_height_3 - scale*plot[1]);
    plot[2] = scale;

    if (plot[1] < 0 || plot[1] >= r_height)
        return false;

    if (plot[0] < 0 || plot[0] >= r_width)
        return false;

    outpt->x = (int)plot[0];
    outpt->y = (int)plot[1];

    ofs = plot[1]*r_width + plot[0];

    if (r_zbuffer[ofs] <= plot[2])
    {
        return true;
    }
    return false;
}


/*
=====================
REN_DrawVec Regular BSP
=====================
*/
void REN_DrawVec(float *pt, COLORREF color)
{
    if (!EditSize.cx || !EditSize.cy)
        return;
    if (!pt)
        return;	// overconstrained plane

    vec3_t temp, plot;
    int ofs;
    unsigned char *out;
    unsigned char whichColor;

    vec3_t *xp = (vec3_t *)pt;
    // move to camera origin
    VectorSubtract (*xp, r_origin, temp);


    // rotate view view matrix...
    plot[0] = DotProduct(temp,r_matrix[0]);
    plot[1] = DotProduct(temp,r_matrix[1]);
    plot[2] = DotProduct(temp,r_matrix[2]);


    //
    // project to 2D
    //
    if (fabs(plot[2]) < FP_EPSILON)
        return;							//remove points that are too close to the camera

    float scale = r_width_2/plot[2];


    plot[0] = (int)(r_width_2 + scale*plot[0]);
    plot[1] = (int)(r_height_3 - scale*plot[1]);
    plot[2] = scale;

    if (plot[1] < 0 || plot[1] >= r_height)
        return;

    if (plot[0] < 0 || plot[0] >= r_width)
        return;

    whichColor = (unsigned char)GetPaletteIndex(256, set.pal, color);

    ofs = plot[1]*r_width + plot[0];

    if (r_zbuffer[ofs] <= plot[2])
    {
        r_zbuffer[ofs] = plot[2];
        ofs = (plot[1] )*r_width + (plot[0] );
        out = &r_picbuffer[ofs];
        *out = whichColor;
    }
}

void REN_Draw3DGrid()
{
    r_drawent = true;
    if(set.grid_style != GRID_LINES)  			//dots
    {
        vec3_t pt = {0,0,0};
        //	for(int j=-64*32;j<=64*32;j+=64) {
        //		for(int i=-64*32;i<=64*32;i+=64) {
        for(int j=64*32; j>=0; j-=64)
        {
            for(int i=64*32; i>=0; i-=64)
            {
                if(set.show_world_axes && (!i || !j))
                    continue;
                pt[0] = i;
                pt[1] = j;
                REN_DrawVec(pt,set.color_3dgrid);
                pt[0] = -i;
                pt[1] = j;
                REN_DrawVec(pt,set.color_3dgrid);
                pt[0] = i;
                pt[1] = -j;
                REN_DrawVec(pt,set.color_3dgrid);
                pt[0] = -i;
                pt[1] = -j;
                REN_DrawVec(pt,set.color_3dgrid);
            }
        }
    }
    else  					//lines
    {


        winding_t *w = NewWinding(2);
        if(!w) return;

        pixel32_t color;

        pixel32_t oldfc = r_flatcolor;
        pixel32_t oldec = r_entitycolor;

        t_coloroffset = 31*256;

        vec3_t startx = {-2048,-2048,0};
        vec3_t endx =   { 2048,-2048,0};
        vec3_t starty = {-2048,-2048,0};
        vec3_t endy =   {-2048, 2048,0};

        //x
        color.rgb = set.color_3dgrid;
        color.p = GetPaletteIndex(256, set.pal, color.rgb);

        r_flatcolor = color;
        r_entitycolor = color;

        for(int i=0; i<=64; i++)
        {

            //don't draw lines over axes
            if(!(i==32 && set.show_world_axes))
            {
                //x
                VectorCopy(startx,w->points[0]);
                VectorCopy(endx,w->points[1]);
                REN_DrawLineWinding(w,0);

                //y
                VectorCopy(starty,w->points[0]);
                VectorCopy(endy,w->points[1]);
                REN_DrawLineWinding(w,0);
            }

            startx[1] = startx[1] + 64;
            endx[1] = endx[1] + 64;

            starty[0] = starty[0] + 64;
            endy[0] = endy[0] + 64;
        }


        delete [] w;
        r_flatcolor = oldfc;
        r_entitycolor = oldec;
    }
    r_drawent = false;
}
/*
=====================
REN_DrawCameraHandle Regular BSP
=====================
*/
void REN_DrawCameraHandle(float *pt, COLORREF color)
{
    if (!EditSize.cx || !EditSize.cy)
        return;

    int		i;
    float	scale;
    plane_t *plane;
    vec3_t	temp,plot;
    float dot;
    int ofs;
    unsigned char *out;
    unsigned char whichColor;

    if (!pt)
        return;	// overconstrained plane

    vec3_t *xp = (vec3_t *)pt;
    // move to camera origin
    VectorSubtract (*xp, r_origin, temp);

    // rotate view view matrix...
    plot[0] = DotProduct(temp,r_matrix[0]);
    plot[1] = DotProduct(temp,r_matrix[1]);
    plot[2] = DotProduct(temp,r_matrix[2]);

    //
    // 3D clip
    //
    for (i=0 ; i<4 ; i++)
    {
        plane = &frustum[i];
        if (!plane)
            continue;

        dot = DotProduct(plot,plane->normal);
        // clipped away, so exit...
        if (dot <= plane->dist)
            return;
    }

    //
    // project to 2D
    //
    if (fabs(plot[2]) < FP_EPSILON)
    {
        scale = 1.0;
    }
    else
    {
        scale = r_width_2/plot[2];
    }

    plot[0] = (int)(r_width_2 + scale*plot[0]);
    if (plot[1] < 0 || plot[1] >= r_height)
        return;

    plot[1] = (int)(r_height_3 - scale*plot[1]);
    if (plot[0] < 0 || plot[0] >= r_width)
        return;

    plot[2] = scale;

    whichColor = (unsigned char)GetPaletteIndex(256,set.pal, color);

    ofs = plot[1]*r_width + plot[0];

    if (r_zbuffer[ofs] <= plot[2])
    {
        r_zbuffer[ofs] = plot[2];
        for (i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                if (((plot[1] + j) < 0) || ((plot[1] + j) >= r_height))
                    continue;

                if (((plot[0] + i) < 0) || ((plot[0] + i) >= r_width))
                    continue;

                ofs = (plot[1] + j)*r_width + (plot[0] + i);
                out = &r_picbuffer[ofs];
                *out = whichColor;
            }
        }
    }
}

/*
=====================
REN_DrawCameraHandle Sin
=====================
*/
void REN_DrawCameraHandleSin(float *pt, unsigned char r, unsigned char g, unsigned char b)
{
    if (!EditSize.cx || !EditSize.cy)
        return;

    int		i;
    float		scale;
    plane_t *plane;
    vec3_t		temp,plot;
    float dot;
    int ofs;
    unsigned char *out;

    if (!pt)
        return;	// overconstrained plane

    vec3_t *xp;
    xp = (vec3_t *)pt;
    // move to camera origin
    VectorSubtract (*xp, r_origin, temp);

    // rotate view view matrix...
    plot[0] = DotProduct(temp,r_matrix[0]);
    plot[1] = DotProduct(temp,r_matrix[1]);
    plot[2] = DotProduct(temp,r_matrix[2]);

    //
    // 3D clip
    //
    for (i=0 ; i<4 ; i++)
    {
        plane = &frustum[i];
        if (!plane)
            continue;

        dot = DotProduct(plot,plane->normal);
        // clipped away, so exit...
        if (dot <= plane->dist)
            return;
    }

    //
    // project to 2D
    //
    if (fabs(plot[2]) < FP_EPSILON)
    {
        scale = 1.0;
    }
    else
    {
        scale = r_width_2/plot[2];
    };

    plot[0] = (int)(r_width_2 + scale*plot[0]);
    plot[1] = (int)(r_height_3 - scale*plot[1]);
    plot[2] = scale;

    if (plot[1] < 0 || plot[1] >= r_height)
        return;

    if (plot[0] < 0 || plot[0] >= r_width)
        return;

    ofs = plot[1]*r_width + plot[0];

    if (r_zbuffer[ofs] <= plot[2])
    {
        r_zbuffer[ofs] = plot[2];
        for (i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                if (((plot[1] + j) < 0) || ((plot[1] + j) >= r_height))
                    continue;

                if (((plot[0] + i) < 0) || ((plot[0] + i) >= r_width))
                    continue;

                ofs = (plot[1] + j)*r_width + (plot[0] + i);
                out = &r_picbuffer[(ofs << 1) + ofs];
                *out++ = r;
                *out++ = g;
                *out =   b;
            };
        };
    };
}
/*
=====================
REN_FindCameraPoint Regular and Sin BSP
Finds the x, y values of the point, if visible...
=====================
*/
bool REN_FindCameraPoint(float *pt, float /*fudge*/, int *plot_x, int *plot_y)
{
    *plot_x = *plot_y = 0;

    if (!EditSize.cx || !EditSize.cy)
        return false;
    if (!pt)
        return false;	// overconstrained plane

    int i;
    float scale;
    plane_t *plane;
    vec3_t temp, plot;
    float dot;

    vec3_t *xp = (vec3_t *)pt;
    // move to camera origin
    VectorSubtract (*xp, r_origin, temp);

    // rotate view view matrix...
    plot[0] = DotProduct(temp,r_matrix[0]);
    plot[1] = DotProduct(temp,r_matrix[1]);
    plot[2] = DotProduct(temp,r_matrix[2]);

    //
    // 3D clip
    //
    for (i=0 ; i<4 ; i++)
    {
        plane = &frustum[i];
        if (!plane)
            continue;

        dot = DotProduct(plot,plane->normal);
        // clipped away, so exit...
        if (dot <= plane->dist)
            return false;
    }

    //
    // project to 2D
    //
    if (fabs(plot[2]) < FP_EPSILON)
    {
        scale = 1.0;
    }
    else
    {
        scale = r_width_2/plot[2];
    };

    plot[0] = (r_width_2 + scale*plot[0]);
    plot[1] = (r_height_3 - scale*plot[1]);
    plot[2] = scale;

    if (plot[1] < 0 || plot[1] >= r_height)
        return false;

    if (plot[0] < 0 || plot[0] >= r_width)
        return false;

    *plot_x = (int) plot[0];
    *plot_y = (int) plot[1];
    return true;
}

/*
=====================
REN_DrawCameraPointSin
=====================
*/
void REN_DrawCameraPointSin(float *pt,int selected)
{
    if (!EditSize.cx || !EditSize.cy)
        return;

    int		i;
    float		scale;
    plane_t *plane;
    vec3_t		temp,plot;
    float dot;
    int ofs;
    unsigned char *out;
    unsigned char whichColor[3];

    if (!pt)
        return;	// overconstrained plane

    vec3_t *xp;
    xp = (vec3_t *)pt;
    // move to camera origin
    VectorSubtract (*xp, r_origin, temp);

    // rotate view view matrix...
    plot[0] = DotProduct(temp,r_matrix[0]);
    plot[1] = DotProduct(temp,r_matrix[1]);
    plot[2] = DotProduct(temp,r_matrix[2]);

    //
    // 3D clip
    //
    for (i=0 ; i<4 ; i++)
    {
        plane = &frustum[i];
        if (!plane)
            continue;

        dot = DotProduct(plot,plane->normal);
        // clipped away, so exit...
        if (dot <= plane->dist)
            return;
    }

    //
    // project to 2D
    //
    if (fabs(plot[2]) < FP_EPSILON)
    {
        scale = 1.0;
    }
    else
    {
        scale = r_width_2/plot[2];
    };

    plot[0] = (r_width_2 + scale*plot[0]);
    plot[1] = (r_height_3 - scale*plot[1]);
    plot[2] = scale;

    if (plot[1] < 0 || plot[1] >= r_height)
        return;

    if (plot[0] < 0 || plot[0] >= r_width)
        return;

    REN_SetupColorsSin(&whichColor[0],&whichColor[1],&whichColor[2],selected);

    ofs = (int) plot[1]*r_width + plot[0];

    if (r_zbuffer[ofs] <= plot[2])
    {
        r_zbuffer[ofs] = plot[2];
        out = &r_picbuffer[(ofs << 1) + ofs];
        *out++ = whichColor[0];     // FIXME
        *out++ = whichColor[1];
        *out =   whichColor[2];
    };
}

/*
=====================
REN_DrawXYFace
=====================
*/
void REN_DrawXYFace (face_t *idpol)
{
    int			i, j, numvertex;
    winding_t	*w, *in;
    float		*dest, *source;
    float		temp;

    if (!idpol->w)
        return;	// overconstrained plane

    w = idpol->w;

    r_face = idpol;

//
// back (and side) face cull
//
    if (DotProduct (idpol->plane.normal, xyWindow[set.curxy]->xy_viewnormal) > -VECTOR_EPSILON)
        return;

//
// transform
//
    in = w;
    numvertex = in->numpoints;
    w = NewWinding (numvertex);
    if(!w) return;

    for (i=0 ; i<numvertex ; i++)
    {
        // using Z as a scale for the 2D projection
        w->points[i][0] = (in->points[i][0] - r_origin[0])*r_origin[2];
        w->points[i][1] = r_height - (in->points[i][1] - r_origin[1])*r_origin[2];
        w->points[i][2] = in->points[i][2] + 3000;
        w->points[i][3] = in->points[i][3];
        w->points[i][4] = in->points[i][4];
    }

//
// clip
//
    for (i=0 ; i<4 ; i++)
    {
        w = ClipWinding (w, &frustum[i]);
        if (!w)
            return;
    }

//
// project to 2D
//
    for (i=0 ; i<w->numpoints ; i++)
    {
        dest = w->points[i];
        if (dest[0] < 0)
            dest[0] = 0;
        if (dest[0] > r_width)
            dest[0] = (float) r_width;
        if (dest[1] < 0)
            dest[1] = 0;
        if (dest[1] > r_height)
            dest[1] = (float) r_height;
        if (xyWindow[set.curxy]->xy_viewnormal[2] > 0)
            dest[2] = 4096-dest[2];
    }

    if (xyWindow[set.curxy]->xy_viewnormal[2] > 0)
    {
        // flip order when upside down
        for (i=0 ; i<w->numpoints/2 ; i++)
        {
            dest = w->points[i];
            source = w->points[w->numpoints-1-i];
            for (j=0 ; j<5 ; j++)
            {
                temp = dest[j];
                dest[j] = source[j];
                source[j] = temp;
            }
        }
    }
    REN_SetTexture (idpol);

//
// draw it
//
    REN_RasterizeFaceLinear (w);

    delete [] w;
}

/*
==================
REN_DrawSpanOutline Regular BSP
==================
*/
void REN_DrawSpanOutline (int y, int selected)
{
    int		x, count;
    int	ofs;
    int		tx, ty;
    int		x1, x2;
    float	ufrac, vfrac, zfrac;
    float   ustep, vstep, zstep;
    float	*rz;
    unsigned char *pic;
    unsigned char *lu;

    float		scale;

    unsigned char whichColor;

    if (y<0 || y >= r_height)
        return;

    //scanline translucency
    if(r_facetrans && set.render_trans==1 && (y & 1))
        return;

    x1 = (int) (leftside[0]);
    x2 = (int) (rightside[0]);

    count = x2 - x1;
    if (count <= 0)
        return;

    zfrac = leftside[2];
    ufrac = leftside[3];
    vfrac = leftside[4];

    scale = 1.0f/count;

    zstep = (rightside[2] - zfrac)*scale;
    ustep = (rightside[3] - ufrac)*scale;
    vstep = (rightside[4] - vfrac)*scale;

    if (x1 < 0)
    {
        ufrac -= x1*ustep;
        vfrac -= x1*vstep;
        zfrac -= x1*zstep;
        x1 = 0;
    }

    if (x2 > r_width)
        x2 = r_width;

    ofs = y*r_width+x1;

    pic = &r_picbuffer[ofs];
    REN_SetupColors(&whichColor,selected);

    rz = &r_zbuffer[ofs];
    lu = &texColorMap->lookUp[t_coloroffset];

    if (*rz <= zfrac)
    {
        *rz = zfrac;
        *pic   = whichColor;
    }
    pic++;
    rz++;
    ufrac += ustep;
    vfrac += vstep;
    zfrac += zstep;

    for (x=x1+1 ; x < (x2-1) ; x++)
    {

        //translucency
        if(!r_facetrans || !(set.render_trans==2 && ((x+y)&1)))
        {
            if (*rz <= zfrac)
            {
                scale = 1/zfrac;
                *rz = zfrac;

                tx = (int)((ufrac*scale)+t_widthadd) % t_width;
                ty = (int)((vfrac*scale)+t_heightadd) % t_height;
                ofs = (ty*t_width+tx);
                *pic = lu[t_data[ofs]];
            }
        }
        pic++;
        rz++;
        ufrac += ustep;
        vfrac += vstep;
        zfrac += zstep;
    }

    if ((x < x2) && (*rz <= zfrac))
    {
        *rz = zfrac;
        *pic = whichColor;
    }
}

/*
==================
REN_DrawSpanWire Regular BSP
==================
*/
void REN_DrawSpanWire(int y, int selected)
{
    int			count;
    int			ofs;
    int			x1, x2;
    unsigned char *out;
    unsigned char whichColor;

    if (y < 0 || y >= r_height)
        return;

    x1 = (int) (leftside[0]);
    x2 = (int) (rightside[0]);

    count = x2 - x1;
    if (count < 0)
        return;

    float zstep;
    if (!count)
    {
        zstep = 0.0;
    }
    else
    {
        zstep = (rightside[2] - leftside[2])/count;
    };

    REN_SetupColors(&whichColor,selected);

    float leftz, rightz;

    int ofs2;
    ofs2 = y*r_width;

    int lstop, rstop;
    // handle left size;
    if (leftstep[0] >= 0.0)   // positive...
    {
        x1    = (int) leftside[0];
        lstop = (int) leftside[0] + leftstep[0];
        leftz = leftside[2];
    }
    else
    {
        x1    = (int) leftside[0] + leftstep[0];
        lstop = (int) leftside[0];
        leftz = leftside[2] - zstep*leftstep[0];  // start to left
    };

    for (x1; x1 <= lstop; x1++)
    {
        if (x1 >= 0 && x1 < (int)r_width)
        {
            ofs = ofs2+x1;
            if (r_zbuffer[ofs] <= leftz)
            {
                r_zbuffer[ofs] = leftz;
                out = &r_picbuffer[ofs];
                *out = whichColor;
            };
        };
        leftz += zstep;
    }

    if (rightstep[0] >= 0.0)   // positive...
    {
        x2     = (int) rightside[0];
        rstop  = (int) rightside[0] + rightstep[0];
        rightz = rightside[2];
    }
    else
    {
        x2     = (int) rightside[0] + rightstep[0];
        rstop  = (int) rightside[0];
        rightz = rightside[2] - rightstep[0]*zstep;
    };

    for (x2; x2 <= rstop; x2++)
    {
        if (x2 >= 0 && x2 < (int)r_width)
        {
            ofs = ofs2+x2;
            if (r_zbuffer[ofs] <= rightz)
            {
                r_zbuffer[ofs] = rightz;
                out = &r_picbuffer[ofs];
                *out = whichColor;
            };
        };
        rightz += zstep;
    }
}

/*
==================
REN_DrawSpanWire
==================
*/
void REN_DrawSpanWireHoriz(int y, int selected)
{
    int			count;
    int			ofs;
    int			x1, x2;
    unsigned char *out;
    unsigned char whichColor;

    if (y < 0 || y >= r_height)
        return;

    x1 = (int) (leftside[0]);
    x2 = (int) (rightside[0]);

    count = x2 - x1;
    if (count <= 0)
        return;

    float zstep;
    zstep = (rightside[2] - leftside[2])/count;

    REN_SetupColors(&whichColor,selected);

    float leftz;

    leftz = leftside[2];

    int ofs2;
    ofs2 = y*r_width;

    for (x1; x1 <= x2; x1++)
    {
        if (x1 >= 0 && x1 < (int)r_width)
        {
            ofs = ofs2+x1;
            if (r_zbuffer[ofs] <= leftz)
            {
                r_zbuffer[ofs] = leftz;
                out = &r_picbuffer[ofs];
                *out = whichColor;
            };
        };
        leftz += zstep;
    }
}
/*
==================
REN_DrawSpanOutlineNoZ
==================
*/
void REN_DrawSpanOutlineNoZ (int y, int selected)
{
    int			x, count;
    int			ofs;
    int			tx, ty;
    int			x1, x2;
    float		ufrac, vfrac, zfrac;
    float    ustep, vstep, zstep;
    unsigned char *in, *out;
    float		scale;

    unsigned char whichColor;

    if (y<0 || y >= r_height)
        return;

    x1 = (int) (leftside[0]);
    x2 = (int) (rightside[0]);

    count = x2 - x1;
    if (count < 0)
        return;

    zfrac = leftside[2];
    ufrac = leftside[3];
    vfrac = leftside[4];

    if (!count)
        scale = 1;
    else
        scale = 1.0f/count;

    zstep = (rightside[2] - zfrac)*scale;
    ustep = (rightside[3] - ufrac)*scale;
    vstep = (rightside[4] - vfrac)*scale;

    if (x1 < 0)
    {
        ufrac -= x1*ustep;
        vfrac -= x1*vstep;
        zfrac -= x1*zstep;
        x1 = 0;
    }

    if (x2 > (int)r_width)
        x2 = (int)r_width;

    ofs = y*r_width+x1;

    REN_SetupColors(&whichColor,selected);

    out = &r_picbuffer[ofs];
    *out = whichColor;
    ufrac += ustep;
    vfrac += vstep;
    zfrac += zstep;
    ofs ++;
    for (x=x1+1 ; x < (x2-1) ; x++)
    {
        scale = 1/zfrac;
        tx = (int)((ufrac*scale)+t_widthadd) % t_width; // was % t_width
        ty = (int)((vfrac*scale)+t_heightadd) % t_height;
        in = &t_data [ty*t_width+tx];
        out = &r_picbuffer[ofs];
        *out = texColorMap->lookUp[t_coloroffset+*in];
        ufrac += ustep;
        vfrac += vstep;
        zfrac += zstep;
        ofs ++;
    }
    if (x < x2)
    {
        out = &r_picbuffer[ofs];
        *out = whichColor;
    }
}

/*
==================
REN_DrawFlatSpanOutline Regular BSP
==================
*/
void REN_DrawFlatSpanOutline (int y, int selected)
{
    int			x, count;
    int			ofs;
    int			x1, x2;
    float		zfrac, zstep;
    unsigned char *out;
    unsigned char whichColor;

    if (y<0 || y >= r_height)
        return;

    if(r_facetrans && set.render_trans==1 && (y & 1))
        return;

    x1 = (int) (leftside[0]);
    x2 = (int) (rightside[0]);

    count = x2 - x1;
    if (count <= 0)
        return;

    zfrac = leftside[2];

    zstep = (rightside[2] - zfrac)/count;

    if (x1 < 0)
    {
        zfrac -= x1*zstep;
        x1 = 0;
    }

    if (x2 > r_width)
        x2 = r_width;

    ofs = y*r_width+x1;

    REN_SetupColors(&whichColor,selected);

    if (r_zbuffer[ofs] <= zfrac)
    {
        r_zbuffer[ofs] = zfrac;
        out = &r_picbuffer[ofs];
        *out = whichColor;
    }
    zfrac += zstep;
    ofs ++;

    //translucency
    /*	if(r_facetrans && set.render_trans==1 && (y & 1)) {
    		ofs += x2-x1;
    		x = (x2-1);
    		zfrac += zstep * (float)(x1-x2);
    		if ((x < x2) && (r_zbuffer[ofs] <= zfrac)) {
    			r_zbuffer[ofs] = zfrac;
    			out = &r_picbuffer[ofs];
    			*out = whichColor;
    		}
    		return;
    	}*/

    for (x=x1+1 ; x < (x2-1) ; x++)
    {

        //translucency
        if(!r_facetrans || !(set.render_trans==2 && ((x+y)&1)))
        {
            if (r_zbuffer[ofs] <= zfrac)
            {
                r_zbuffer[ofs] = zfrac;
                out = &r_picbuffer[ofs]; // combine?
                *out = texColorMap->lookUp[t_coloroffset+r_flatcolor.p];
            }
        }
        zfrac += zstep;
        ofs ++;
    }
    if ((x < x2) && (r_zbuffer[ofs] <= zfrac))
    {
        r_zbuffer[ofs] = zfrac;
        out = &r_picbuffer[ofs];
        *out = whichColor;
    }

}
/////////////////SIN Stuff
/*
==================
REN_DrawSpanOutlineSin
==================
*/
void REN_DrawSpanOutlineSin(int y, int selected)
{
    int			x, count;
    int			ofs;
    int			tx, ty;
    int			x1, x2;
    float		ufrac, vfrac, zfrac;
    float    ustep, vstep, zstep;
    float *rz;
    unsigned char *pic;
//   unsigned char *lu;

    float		scale;

    unsigned char *in;
    unsigned char whichColor[3];

    if (y<0 || y >= r_height)
        return;

    x1 = (int) (leftside[0]);
    x2 = (int) (rightside[0]);

    count = x2 - x1;
    if (count <= 0)
        return;

    zfrac = leftside[2];
    ufrac = leftside[3];
    vfrac = leftside[4];

    scale = 1.0f/count;

    zstep = (rightside[2] - zfrac)*scale;
    ustep = (rightside[3] - ufrac)*scale;
    vstep = (rightside[4] - vfrac)*scale;

    if (x1 < 0)
    {
        ufrac -= x1*ustep;
        vfrac -= x1*vstep;
        zfrac -= x1*zstep;
        x1 = 0;
    }

    if (x2 > (int)r_width)
        x2 = (int)r_width;

    ofs = y*r_width+x1;

    REN_SetupColorsSin(&whichColor[0],&whichColor[1],&whichColor[2],selected);
    pic = &r_picbuffer[(ofs << 1) + ofs];
    rz = &r_zbuffer[ofs];
//   lu = &texColorMap->lookUp[t_coloroffset];

    if (*rz <= zfrac)
    {
        *rz = zfrac;
        *pic++ = whichColor[0];     // FIXME
        *pic++ = whichColor[1];
        *pic++ = whichColor[2];
    }
    else
    {
        pic+=3;
    };
    rz++;
    ufrac += ustep;
    vfrac += vstep;
    zfrac += zstep;

    for (x=x1+1 ; x < (x2-1) ; x++)
    {
        if (*rz <= zfrac)
        {
            scale = 1/zfrac;
            *rz = zfrac;

            tx = (int)((ufrac*scale)+t_widthadd) % t_width; // was % t_width
            ty = (int)((vfrac*scale)+t_heightadd) % t_height;
            ofs = (ty*t_width+tx);
            in = &t_data [(ofs << 1) + ofs];
            *pic++ = *in++;
            *pic++ = *in++;
            *pic++ = *in;
        }
        else
        {
            pic+= 3;
        };
        rz++;
        ufrac += ustep;
        vfrac += vstep;
        zfrac += zstep;
    }

    if ((x < x2) && (*rz <= zfrac))
    {
        *rz = zfrac;
        *pic++ = whichColor[0];     // FIXME
        *pic++ = whichColor[1];
        *pic = whichColor[2];
    }
}

/*
==================
REN_DrawSpanWireSin
==================
*/
void REN_DrawSpanWireSin(int y, int selected)
{
    int ofs;
    unsigned char *out;
    unsigned char whichColor[3];

    if (y < 0 || y >= r_height)
        return;

    int x1 = (int) (leftside[0]);
    int x2 = (int) (rightside[0]);

    int count = x2 - x1;
    if (count < 0)
        return;

    float zstep = 0.0;
    if (count)
    {
        zstep = (rightside[2] - leftside[2])/count;
    }

    REN_SetupColorsSin(&whichColor[0],&whichColor[1],&whichColor[2],selected);

    float leftz, rightz;

    int ofs2 = y*r_width;

    int lstop, rstop;
    // handle left size;
    if (leftstep[0] >= 0.0)   // positive...
    {
        x1    = (int) leftside[0];
        lstop = (int) leftside[0] + leftstep[0];
        leftz = leftside[2];
    }
    else
    {
        x1    = (int) leftside[0] + leftstep[0];
        lstop = (int) leftside[0];
        leftz = leftside[2] - zstep*leftstep[0];  // start to left
    }

    for (x1; x1 <= lstop; x1++)
    {
        if (x1 >= 0 && x1 < (int)r_width)
        {
            ofs = ofs2+x1;
            if (r_zbuffer[ofs] <= leftz)
            {
                r_zbuffer[ofs] = leftz;
                out = &r_picbuffer[(ofs << 1) + ofs];
                *out++ = whichColor[0];     // FIXME
                *out++ = whichColor[1];
                *out =   whichColor[2];
            };
        };
        leftz += zstep;
    }

    if (rightstep[0] >= 0.0)   // positive...
    {
        x2     = (int) rightside[0];
        rstop  = (int) rightside[0] + rightstep[0];
        rightz = rightside[2];
    }
    else
    {
        x2     = (int) rightside[0] + rightstep[0];
        rstop  = (int) rightside[0];
        rightz = rightside[2] - rightstep[0]*zstep;
    };

    for (x2; x2 <= rstop; x2++)
    {
        if (x2 >= 0 && x2 < (int)r_width)
        {
            ofs = ofs2+x2;
            if (r_zbuffer[ofs] <= rightz)
            {
                r_zbuffer[ofs] = rightz;
                out = &r_picbuffer[(ofs << 1) + ofs];
                *out++ = whichColor[0];     // FIXME
                *out++ = whichColor[1];
                *out =   whichColor[2];
            };
        };
        rightz += zstep;
    }
}

/*
==================
REN_DrawSpanWireHorizSin
==================
*/
void REN_DrawSpanWireHorizSin(int y, int selected)
{
    int			count;
    int			ofs;
    int			x1, x2;
    unsigned char *out;
    unsigned char whichColor[3];

    if (y < 0 || y >= r_height)
        return;

    x1 = (int) (leftside[0]);
    x2 = (int) (rightside[0]);

    count = x2 - x1;
    if (count <= 0)
        return;

    float zstep;
    zstep = (rightside[2] - leftside[2])/count;

    REN_SetupColorsSin(&whichColor[0],&whichColor[1],&whichColor[2],selected);

    float leftz;

    leftz = leftside[2];

    int ofs2;
    ofs2 = y*r_width;

    for (x1; x1 <= x2; x1++)
    {
        if (x1 >= 0 && x1 < (int)r_width)
        {
            ofs = ofs2+x1;
            if (r_zbuffer[ofs] <= leftz)
            {
                r_zbuffer[ofs] = leftz;
                out = &r_picbuffer[(ofs << 1) + ofs];
                *out++ = whichColor[0];     // FIXME
                *out++ = whichColor[1];
                *out =   whichColor[2];
            };
        };
        leftz += zstep;
    }
}

/*
==================
REN_DrawSpanOutlineNoZSin
==================
*/
void REN_DrawSpanOutlineNoZSin(int y, int selected)
{
    int			x, count;
    int			ofs;
    int			tx, ty;
    int			x1, x2;
    float		ufrac, vfrac, zfrac;
    float    ustep, vstep, zstep;
    unsigned char *in, *out;
    float		scale;

    unsigned char whichColor[3];

    if (y<0 || y >= r_height)
        return;

    x1 = (int) (leftside[0]);
    x2 = (int) (rightside[0]);

    count = x2 - x1;
    if (count < 0)
        return;

    zfrac = leftside[2];
    ufrac = leftside[3];
    vfrac = leftside[4];

    if (!count)
        scale = 1;
    else
        scale = 1.0f/count;

    zstep = (rightside[2] - zfrac)*scale;
    ustep = (rightside[3] - ufrac)*scale;
    vstep = (rightside[4] - vfrac)*scale;

    if (x1 < 0)
    {
        ufrac -= x1*ustep;
        vfrac -= x1*vstep;
        zfrac -= x1*zstep;
        x1 = 0;
    }

    if (x2 > (int)r_width)
        x2 = (int)r_width;

    ofs = y*r_width+x1;

    REN_SetupColorsSin(&whichColor[0],&whichColor[1],&whichColor[2],selected);

    out = &r_picbuffer[(ofs << 1) + ofs];
    *out++ = whichColor[0];     // FIXME
    *out++ = whichColor[1];
    *out = whichColor[2];
    ufrac += ustep;
    vfrac += vstep;
    zfrac += zstep;
    ofs ++;
    for (x=x1+1 ; x < (x2-1) ; x++)
    {
        scale = 1/zfrac;

        tx = (int)((ufrac*scale)+t_widthadd) % t_width; // was % t_width
        ty = (int)((vfrac*scale)+t_heightadd) % t_height;
        in = &t_data [3L*(ty*t_width+tx)];
        out = &r_picbuffer[(ofs << 1) + ofs];
        *out++ = *in++;
        *out++ = *in++;
        *out = *in;
        ufrac += ustep;
        vfrac += vstep;
        zfrac += zstep;
        ofs ++;
    }
    if (x < x2)
    {
        out = &r_picbuffer[(ofs << 1) + ofs];
        *out++ = whichColor[0];
        *out++ = whichColor[1];
        *out = whichColor[2];
    }
}

/*
==================
REN_DrawFlatSpanOutlineSin
==================
*/
void REN_DrawFlatSpanOutlineSin(int y, int selected)
{
    int			x, count;
    int			ofs;
    int			x1, x2;
    float		zfrac, zstep;
    unsigned char *out;
    unsigned char whichColor[3];

    if (y<0 || y >= r_height)
        return;

    x1 = (int) (leftside[0]);
    x2 = (int) (rightside[0]);

    count = x2 - x1;
    if (count <= 0)
        return;

    zfrac = leftside[2];

    zstep = (rightside[2] - zfrac)/count;

    if (x1 < 0)
    {
        zfrac -= x1*zstep;
        x1 = 0;
    }

    if (x2 > (int)r_width)
        x2 = (int)r_width;

    ofs = y*r_width+x1;

    REN_SetupColorsSin(&whichColor[0],&whichColor[1],&whichColor[2],selected);

    if (r_zbuffer[ofs] <= zfrac)
    {
        r_zbuffer[ofs] = zfrac;
        out = &r_picbuffer[(ofs << 1) + ofs];
        *out++ = whichColor[0];
        *out++ = whichColor[1];
        *out = whichColor[2];
    };
    zfrac += zstep;
    ofs ++;
    for (x=x1+1 ; x < (x2-1) ; x++)
    {
        if (r_zbuffer[ofs] <= zfrac)
        {
            r_zbuffer[ofs] = zfrac;
            out = &r_picbuffer[(ofs << 1) + ofs];
            *out++ = r_flatcolor.chan[0];;
            *out++ = r_flatcolor.chan[1];
            *out = r_flatcolor.chan[2];
        }
        zfrac += zstep;
        ofs ++;
    }
    if ((x < x2) && (r_zbuffer[ofs] <= zfrac))
    {
        r_zbuffer[ofs] = zfrac;
        out = &r_picbuffer[(ofs << 1) + ofs];
        *out++ = whichColor[0];
        *out++ = whichColor[1];
        *out = whichColor[2];
    };
}

///////////////////// Stuff for tracking click point...
// Point is given in x, y coordinates
// interpolated to render_h and _w size from click coordinates...
void REN_FindClosestFace(face_t *idpol)
{
    if (!EditSize.cx || !EditSize.cy )
        return;

    int			i;
    float		scale;
    int			numvertex;
    winding_t	*w, *in;
    vec3_t		temp;

    if (!idpol->w)
        return;	// overconstrained plane

    r_face = idpol;

    //
    // back face cull
    //
    if (DotProduct (r_origin, idpol->plane.normal) <= idpol->plane.dist)
        return;

    //
    // transform in 3D (should be optimized by clipping first, then transforming)
    //
    in = idpol->w;
    numvertex = in->numpoints;

    w = NewWinding (numvertex);
    if(!w) return;
    for (i=0 ; i<numvertex ; i++)
    {
        VectorSubtract (in->points[i], r_origin, temp);

        w->points[i][0] = DotProduct(temp,r_matrix[0]);
        w->points[i][1] = DotProduct(temp,r_matrix[1]);
        w->points[i][2] = DotProduct(temp,r_matrix[2]);
    }

    //
    // 3D clip
    //
    for (i=0 ; i<4 ; i++)
    {
        w = ClipWinding(w, &frustum[i]);
        if (!w)
            return;
    }

    //
    // project to 2D
    //
    for (i=0 ; i<w->numpoints ; i++)
    {
        scale = r_width_2 / w->points[i][2];
        w->points[i][0] = r_width_2 + scale*w->points[i][0];
        w->points[i][1] = r_height_3 - scale*w->points[i][1];
        w->points[i][2] = scale;
    }

//	REN_SetTexture (idpol);
    REN_CheckDistanceRasterizeFace (w);
    delete [] w;
}

/*
=====================
REN_RasterizeFace

=====================
*/
void REN_CheckDistanceRasterizeFace(winding_t *w)
{
    int			y;
    int			i;
    int			top, bot;
    int			leftv, rightv;
    int			count;
    int 		numvertex;

//
// find top vertex
//
    numvertex = w->numpoints;
    top = INT_MAX;
    bot = INT_MIN;
    leftv = 0;

    for (i=0 ; i<numvertex ; i++)
    {
        sy[i] = (int)w->points[i][1];

        if (sy[i] < top)
        {
            top = sy[i];
            leftv = i;
        }
        if (sy[i] > bot)
            bot = sy[i];
    }
    rightv = leftv;

    if (top < 0 || bot > r_height || top > bot)
        return;		// shouldn't have to have this...

    if (Render::clickY < top || Render::clickY > bot) // click is out of range...
        return;

//
// calculate a rendered trapezoid
//
    y = top;

    while (y < bot)
    {
        if (y >= sy[leftv])
        {
            do
            {
                for (i=0 ; i<5 ; i++)
                    leftside[i] = w->points[leftv][i];
                leftv--;
                if (leftv == -1)
                    leftv = numvertex-1;
            }
            while (sy[leftv] <= y);
            count = sy[leftv]-y;
            for (i=0 ; i<5 ; i++)
                leftstep[i] = (w->points[leftv][i] - leftside[i])/count;
        }
        if (y >= sy[rightv])
        {
            do
            {
                for (i=0 ; i<5 ; i++)
                    rightside[i] = w->points[rightv][i];
                rightv++;
                if (rightv == numvertex)
                    rightv = 0;
            }
            while (sy[rightv] <= y);
            count = sy[rightv]-y;
            for (i=0 ; i<5 ; i++)
                rightstep[i] = (w->points[rightv][i] - rightside[i])/count;
        }

        REN_CheckClosestSpan (y);

        for (i=0 ; i<5 ; i++)
        {
            leftside[i] += leftstep[i];
            rightside[i] += rightstep[i];
        }

        y++;
    }
}

/*
==================
REN_checkclosestspan
// relies on clickX
// and bestZ;
==================
*/
void REN_CheckClosestSpan (int y)
{
    int		count;
    int		x1, x2;
    float	zfrac;
    float	zstep;
    float	scale;

    if (y<0 || y >= r_height)
        return;

    if (y != Render::clickY)
        return;

    x1 = (int) (leftside[0]);
    x2 = (int) (rightside[0]);

    count = x2 - x1;

    if (count < 0)
        return;

    if (Render::clickX < x1 || Render::clickX > x2) // out of range...
        return;

    zfrac = leftside[2];

    if (!count)
        scale = 1;
    else
        scale = 1.0f/count;

    zstep = (rightside[2] - zfrac)*scale;

    if (x1 < 0)
    {
        zfrac -= x1*zstep;
        x1 = 0;
    }

    int jump;

    jump = Render::clickX - x1;

    zfrac += jump*zstep;

    if (zfrac >= -REN_bestDistance)
    {
        // click point is better
        REN_bestDistance = -zfrac;
        REN_bestBrush = REN_testBrush;
        REN_bestFace = REN_testFace;
        // log the current brush's pointer...
    };
}

///////////////////////////////
// For drawing only outlines...
////////////////////////////////
/* Draws a horizontal run of pixels, then advances the bitmap pointer to
   the first pixel of the next run. */
void DrawHorizontalRun(unsigned char **ScreenPtr, int XAdvance, int RunLength, unsigned char Color)
{
    unsigned char *WorkingScreenPtr = *ScreenPtr;

    for (int i=0; i<RunLength; i++)
    {
        if (((WorkingScreenPtr - r_picbuffer) >= 0) &&
                ((WorkingScreenPtr - r_picbuffer) < (r_width*r_height)))
        {
            *WorkingScreenPtr = Color;
        }
        WorkingScreenPtr += XAdvance;
    }
    /* Advance to the next scan line */
    WorkingScreenPtr += r_width;
    *ScreenPtr = WorkingScreenPtr;
}

/* SINDraws a horizontal run of pixels, then advances the bitmap pointer to
   the first pixel of the next run. */
void DrawHorizontalRunSin(unsigned char **ScreenPtr, int XAdvance,
                          int RunLength, unsigned char Color[3])
{
    int i;
    unsigned char *WorkingScreenPtr = *ScreenPtr;

    for (i=0; i<RunLength; i++)
    {
        *WorkingScreenPtr       = Color[0];
        *(WorkingScreenPtr + 1) = Color[1];
        *(WorkingScreenPtr + 2) = Color[2];
        WorkingScreenPtr += 3L*XAdvance;
    }
    /* Advance to the next scan line */
    WorkingScreenPtr += 3L*r_width;
    *ScreenPtr = WorkingScreenPtr;
}

/* Draws a vertical run of pixels, then advances the bitmap pointer to
   the first pixel of the next run. */
void DrawVerticalRun(unsigned char **ScreenPtr, int XAdvance,
                     int RunLength, unsigned char Color)
{
    int i;
    unsigned char *WorkingScreenPtr = *ScreenPtr;

    for (i=0; i<RunLength; i++)
    {
        if (((WorkingScreenPtr - r_picbuffer) >= 0) &&
                ((WorkingScreenPtr - r_picbuffer) < (r_width*r_height)))
        {
            *WorkingScreenPtr = Color;
        };
        WorkingScreenPtr += r_width;
    }
    /* Advance to the next scan line */
    WorkingScreenPtr += XAdvance;
    *ScreenPtr = WorkingScreenPtr;
}

/* SIN Draws a vertical run of pixels, then advances the bitmap pointer to
   the first pixel of the next run. */
void DrawVerticalRunSin(unsigned char **ScreenPtr, int XAdvance,
                        int RunLength, unsigned char Color[3])
{
    int i;
    unsigned char *WorkingScreenPtr = *ScreenPtr;

    for (i=0; i<RunLength; i++)
    {
        *WorkingScreenPtr       = Color[0];
        *(WorkingScreenPtr + 1) = Color[1];
        *(WorkingScreenPtr + 2) = Color[2];
        WorkingScreenPtr += 3L*r_width;
    }
    /* Advance to the next scan line */
    WorkingScreenPtr += 3L*XAdvance;
    *ScreenPtr = WorkingScreenPtr;
}

#define CURLOC (ScreenPtr - r_picbuffer)

/* Regular BSP Draws a line between the specified endpoints in color Color. */
void REN_LineDraw(int XStart, int YStart, int XEnd, int YEnd, int selected)
{
    int Temp, AdjUp, AdjDown, ErrorTerm, XAdvance, XDelta, YDelta;
    int WholeStep, InitialPixelCount, FinalPixelCount, i, RunLength;
    int ofs;
    unsigned char *ScreenPtr;
    unsigned char whichColor;

    /* We'll always draw top to bottom, to reduce the number of cases we have to
    handle, and to make lines between the same endpoints draw the same pixels */
    if (YStart > YEnd)
    {
        Temp = YStart;
        YStart = YEnd;
        YEnd = Temp;
        Temp = XStart;
        XStart = XEnd;
        XEnd = Temp;
    }
    /* Point to the bitmap address first pixel to draw */
    // ScreenPtr = MK_FP(SCREEN_SEGMENT, YStart * r_width + XStart);
    ofs = (YStart * r_width + XStart);
    if (ofs < 0 || ofs >= r_width * r_height)
        return;

    ScreenPtr = &r_picbuffer[ofs];

    REN_SetupColors(&whichColor,selected);

    /* Figure out whether we're going left or right, and how huge we're
       going horizontally */
    if ((XDelta = (XEnd - XStart)) < 0)
    {
        XAdvance = -1;
        XDelta = -XDelta;
    }
    else
    {
        XAdvance = 1;
    }
    /* Figure out how huge we're going vertically */
    YDelta = YEnd - YStart;

    /* Special-case horizontal, vertical, and diagonal lines, for speed
       and to avoid nasty boundary conditions and division by 0 */
    if (!XDelta)
    {
        /* Vertical line */
        for (i=0; i<=YDelta; i++)
        {
            if (CURLOC >= 0 && CURLOC < r_width*r_height)
                *ScreenPtr = whichColor;
            else
                return;

            if ((CURLOC + r_width) < r_width*r_height)
                ScreenPtr += r_width;
            else
                return;
        }
        return;
    }
    if (!YDelta)
    {
        /* Horizontal line */
        for (i=0; i<=XDelta; i++)
        {
            if (((ScreenPtr - r_picbuffer) >= 0) &&
                    ((ScreenPtr - r_picbuffer) < (r_width*r_height)))
            {
                *ScreenPtr = whichColor;
            }
            else
            {
                return;
            };
            if (((CURLOC + XAdvance) < (r_width*r_height)) &&
                    ((CURLOC + XAdvance) >= 0))
            {
                ScreenPtr += XAdvance;
            }
            else
            {
                return;
            };
        }
        return;
    }
    if (XDelta == YDelta)
    {
        /* Diagonal line */
        for (i=0; i<=XDelta; i++)
        {
            if (((ScreenPtr - r_picbuffer) >= 0) &&
                    ((ScreenPtr - r_picbuffer) < (r_width*r_height)))
            {
                *ScreenPtr = whichColor;
            }
            else
            {
                return;
            };
            if (((CURLOC + (XAdvance + r_width)) < (r_width*r_height)) &&
                    ((CURLOC + (XAdvance + r_width)) >= 0))
            {
                ScreenPtr += (XAdvance + r_width);
            }
            else
            {
                return;
            };
        }
        return;
    }

    /* Determine whether the line is X or Y major, and handle accordingly */
    if (XDelta >= YDelta)
    {
        /* X major line */
        /* Minimum # of pixels in a run in this line */
        WholeStep = (int)(XDelta / YDelta);

        /* Error term adjust each time Y steps by 1; used to tell when one
           extra pixel should be drawn as part of a run, to account for
           fractional steps along the X axis per 1-pixel steps along Y */
        AdjUp = (int)((XDelta % YDelta) * 2);

        /* Error term adjust when the error term turns over, used to factor
           out the X step made at that time */
        AdjDown = (int)(YDelta * 2);

        /* Initial error term; reflects an initial step of 0.5 along the Y
           axis */
        ErrorTerm = (int)((XDelta % YDelta) - (YDelta * 2));

        /* The initial and last runs are partial, because Y advances only 0.5
           for these runs, rather than 1. Divide one full run, plus the
           initial pixel, between the initial and last runs */
        InitialPixelCount = (int)((WholeStep / 2) + 1);
        FinalPixelCount = InitialPixelCount;

        /* If the basic run length is even and there's no fractional
           advance, we have one pixel that could go to either the initial
           or last partial run, which we'll arbitrarily allocate to the
           last run */
        if (!AdjUp && !(WholeStep & 0x01))
            InitialPixelCount--;

        /* If there're an odd number of pixels per run, we have 1 pixel that can't
           be allocated to either the initial or last partial run, so we'll add 0.5
           to error term so this pixel will be handled by the normal full-run loop */
        if ((WholeStep & 0x01) != 0)
        {
            ErrorTerm = (int)(ErrorTerm+YDelta);
        }
        /* Draw the first, partial run of pixels */
        DrawHorizontalRun(&ScreenPtr, XAdvance, InitialPixelCount, whichColor);

        /* Draw all full runs */
        for (i=0; i<(YDelta-1); i++)
        {
            RunLength = WholeStep;  /* run is at least this long */
            /* Advance the error term and add an extra pixel if the error
               term so indicates */
            if ((ErrorTerm += AdjUp) > 0)
            {
                RunLength++;
                ErrorTerm = (int)(ErrorTerm - AdjDown);   /* reset the error term */
            }
            /* Draw this scan line's run */
            DrawHorizontalRun(&ScreenPtr, XAdvance, RunLength, whichColor);
        }
        /* Draw the final run of pixels */
        DrawHorizontalRun(&ScreenPtr, XAdvance, FinalPixelCount, whichColor);
        return;
    }
    else
    {
        /* Y major line */

        /* Minimum # of pixels in a run in this line */
        WholeStep = (int)(YDelta / XDelta);

        /* Error term adjust each time X steps by 1; used to tell when 1 extra
           pixel should be drawn as part of a run, to account for
           fractional steps along the Y axis per 1-pixel steps along X */
        AdjUp = (int)((YDelta % XDelta) * 2);

        /* Error term adjust when the error term turns over, used to factor
           out the Y step made at that time */
        AdjDown = (int)(XDelta * 2);

        /* Initial error term; reflects initial step of 0.5 along the X axis */
        ErrorTerm = (int)((YDelta % XDelta) - (XDelta * 2));

        /* The initial and last runs are partial, because X advances only 0.5
           for these runs, rather than 1. Divide one full run, plus the
           initial pixel, between the initial and last runs */
        InitialPixelCount = (int)((WholeStep / 2) + 1);
        FinalPixelCount = InitialPixelCount;

        /* If the basic run length is even and there's no fractional advance, we
           have 1 pixel that could go to either the initial or last partial run,
           which we'll arbitrarily allocate to the last run */
        if ((AdjUp == 0) && ((WholeStep & 0x01) == 0))
        {
            InitialPixelCount--;
        }
        /* If there are an odd number of pixels per run, we have one pixel
           that can't be allocated to either the initial or last partial
           run, so we'll add 0.5 to the error term so this pixel will be
           handled by the normal full-run loop */
        if ((WholeStep & 0x01) != 0)
        {
            ErrorTerm = (int)(ErrorTerm + XDelta);
        }
        /* Draw the first, partial run of pixels */
        DrawVerticalRun(&ScreenPtr, XAdvance, InitialPixelCount, whichColor);

        /* Draw all full runs */
        for (i=0; i<(XDelta-1); i++)
        {
            RunLength = WholeStep;  /* run is at least this long */
            /* Advance the error term and add an extra pixel if the error
               term so indicates */
            if ((ErrorTerm += AdjUp) > 0)
            {
                RunLength++;
                ErrorTerm = (int)(ErrorTerm - AdjDown);   /* reset the error term */
            }
            /* Draw this scan line's run */
            DrawVerticalRun(&ScreenPtr, XAdvance, RunLength, whichColor);
        }
        /* Draw the final run of pixels */
        DrawVerticalRun(&ScreenPtr, XAdvance, FinalPixelCount, whichColor);
        return;
    }
}

/* Draws a line between the specified endpoints in color Color. */
void REN_LineDrawSin(int XStart, int YStart, int XEnd, int YEnd, int selected)
{
    int Temp, AdjUp, AdjDown, ErrorTerm, XAdvance, XDelta, YDelta;
    int WholeStep, InitialPixelCount, FinalPixelCount, i, RunLength;
    int ofs;
    unsigned char *ScreenPtr;
    unsigned char whichColor[3];

    /* We'll always draw top to bottom, to reduce the number of cases we have to
    handle, and to make lines between the same endpoints draw the same pixels */
    if (YStart > YEnd)
    {
        Temp = YStart;
        YStart = YEnd;
        YEnd = Temp;
        Temp = XStart;
        XStart = XEnd;
        XEnd = Temp;
    }
    /* Point to the bitmap address first pixel to draw */
    // ScreenPtr = MK_FP(SCREEN_SEGMENT, YStart * r_width + XStart);
    ofs = (YStart * r_width + XStart);
    if ((ofs < 0L) || (ofs >= (r_width * r_height)))
        return;

    ScreenPtr = &r_picbuffer[(ofs << 1) + ofs];

    REN_SetupColorsSin(&whichColor[0],&whichColor[1],&whichColor[2],selected);

    /* Figure out whether we're going left or right, and how huge we're
       going horizontally */
    if ((XDelta = (XEnd - XStart)) < 0)
    {
        XAdvance = -1;
        XDelta = -XDelta;
    }
    else
    {
        XAdvance = 1;
    }
    /* Figure out how huge we're going vertically */
    YDelta = YEnd - YStart;

    /* Special-case horizontal, vertical, and diagonal lines, for speed
       and to avoid nasty boundary conditions and division by 0 */
    if (XDelta == 0)
    {
        /* Vertical line */
        for (i=0; i<=YDelta; i++)
        {
            *ScreenPtr     = whichColor[0];
            *(ScreenPtr+1) = whichColor[1];
            *(ScreenPtr+2) = whichColor[2];
            ScreenPtr += 3L*r_width;
        }
        return;
    }
    if (YDelta == 0)
    {
        /* Horizontal line */
        for (i=0; i<=XDelta; i++)
        {
            *ScreenPtr     = whichColor[0];
            *(ScreenPtr+1) = whichColor[1];
            *(ScreenPtr+2) = whichColor[2];
            ScreenPtr += 3L*XAdvance;
        }
        return;
    }
    if (XDelta == YDelta)
    {
        /* Diagonal line */
        for (i=0; i<=XDelta; i++)
        {
            *ScreenPtr     = whichColor[0];
            *(ScreenPtr+1) = whichColor[1];
            *(ScreenPtr+2) = whichColor[2];
            ScreenPtr += 3L*(XAdvance + r_width);
        }
        return;
    }

    /* Determine whether the line is X or Y major, and handle accordingly */
    if (XDelta >= YDelta)
    {
        /* X major line */
        /* Minimum # of pixels in a run in this line */
        WholeStep = (int)(XDelta / YDelta);

        /* Error term adjust each time Y steps by 1; used to tell when one
           extra pixel should be drawn as part of a run, to account for
           fractional steps along the X axis per 1-pixel steps along Y */
        AdjUp = (int)((XDelta % YDelta) * 2);

        /* Error term adjust when the error term turns over, used to factor
           out the X step made at that time */
        AdjDown = (int)(YDelta * 2);

        /* Initial error term; reflects an initial step of 0.5 along the Y
           axis */
        ErrorTerm = (int)((XDelta % YDelta) - (YDelta * 2));

        /* The initial and last runs are partial, because Y advances only 0.5
           for these runs, rather than 1. Divide one full run, plus the
           initial pixel, between the initial and last runs */
        InitialPixelCount = (int)((WholeStep / 2) + 1);
        FinalPixelCount = InitialPixelCount;

        /* If the basic run length is even and there's no fractional
           advance, we have one pixel that could go to either the initial
           or last partial run, which we'll arbitrarily allocate to the
           last run */
        if ((AdjUp == 0) && ((WholeStep & 0x01) == 0))
        {
            InitialPixelCount--;
        }
        /* If there're an odd number of pixels per run, we have 1 pixel that can't
           be allocated to either the initial or last partial run, so we'll add 0.5
           to error term so this pixel will be handled by the normal full-run loop */
        if ((WholeStep & 0x01) != 0)
        {
            ErrorTerm = (int)(ErrorTerm+YDelta);
        }
        /* Draw the first, partial run of pixels */
        DrawHorizontalRunSin(&ScreenPtr, XAdvance, InitialPixelCount, whichColor);

        /* Draw all full runs */
        for (i=0; i<(YDelta-1); i++)
        {
            RunLength = WholeStep;  /* run is at least this long */
            /* Advance the error term and add an extra pixel if the error
               term so indicates */
            if ((ErrorTerm += AdjUp) > 0)
            {
                RunLength++;
                ErrorTerm = (int)(ErrorTerm - AdjDown);   /* reset the error term */
            }
            /* Draw this scan line's run */
            DrawHorizontalRunSin(&ScreenPtr, XAdvance, RunLength, whichColor);
        }
        /* Draw the final run of pixels */
        DrawHorizontalRunSin(&ScreenPtr, XAdvance, FinalPixelCount, whichColor);
        return;
    }
    else
    {
        /* Y major line */

        /* Minimum # of pixels in a run in this line */
        WholeStep = (int)(YDelta / XDelta);

        /* Error term adjust each time X steps by 1; used to tell when 1 extra
           pixel should be drawn as part of a run, to account for
           fractional steps along the Y axis per 1-pixel steps along X */
        AdjUp = (int)((YDelta % XDelta) * 2);

        /* Error term adjust when the error term turns over, used to factor
           out the Y step made at that time */
        AdjDown = (int)(XDelta * 2);

        /* Initial error term; reflects initial step of 0.5 along the X axis */
        ErrorTerm = (int)((YDelta % XDelta) - (XDelta * 2));

        /* The initial and last runs are partial, because X advances only 0.5
           for these runs, rather than 1. Divide one full run, plus the
           initial pixel, between the initial and last runs */
        InitialPixelCount = (int)((WholeStep / 2) + 1);
        FinalPixelCount = InitialPixelCount;

        /* If the basic run length is even and there's no fractional advance, we
           have 1 pixel that could go to either the initial or last partial run,
           which we'll arbitrarily allocate to the last run */
        if ((AdjUp == 0) && ((WholeStep & 0x01) == 0))
        {
            InitialPixelCount--;
        }
        /* If there are an odd number of pixels per run, we have one pixel
           that can't be allocated to either the initial or last partial
           run, so we'll add 0.5 to the error term so this pixel will be
           handled by the normal full-run loop */
        if ((WholeStep & 0x01) != 0)
        {
            ErrorTerm = (int)(ErrorTerm + XDelta);
        }
        /* Draw the first, partial run of pixels */
        DrawVerticalRunSin(&ScreenPtr, XAdvance, InitialPixelCount, whichColor);

        /* Draw all full runs */
        for (i=0; i<(XDelta-1); i++)
        {
            RunLength = WholeStep;  /* run is at least this long */
            /* Advance the error term and add an extra pixel if the error
               term so indicates */
            if ((ErrorTerm += AdjUp) > 0)
            {
                RunLength++;
                ErrorTerm = (int)(ErrorTerm - AdjDown);   /* reset the error term */
            }
            /* Draw this scan line's run */
            DrawVerticalRunSin(&ScreenPtr, XAdvance, RunLength, whichColor);
        }
        /* Draw the final run of pixels */
        DrawVerticalRunSin(&ScreenPtr, XAdvance, FinalPixelCount, whichColor);
        return;
    }
}
///////////

///////////////////////////////
// For drawing only outlines with zbuffering......
////////////////////////////////
/* Regular BSP Draws a horizontal run of pixels, then advances the bitmap pointer to
   the first pixel of the next run. */
void DrawHorizontalRunZ(unsigned char **ScreenPtr, int *ofs, float *zfrac, float *zstep, int XAdvance,
                        int RunLength, unsigned char Color)
{

    int i;
    unsigned char *WorkingScreenPtr = *ScreenPtr;

    for (i=0; i<RunLength; i++)
    {
        if (r_zbuffer[*ofs] <= *zfrac)
        {
            r_zbuffer[*ofs] = *zfrac;
            *WorkingScreenPtr = Color;
        };
        WorkingScreenPtr += XAdvance;
        *ofs += XAdvance;
        *zfrac += *zstep;
    }
    /* Advance to the next scan line */
    WorkingScreenPtr += r_width;
    *ofs += r_width;
    *ScreenPtr = WorkingScreenPtr;
}

/* SIN Draws a horizontal run of pixels, then advances the bitmap pointer to
   the first pixel of the next run. */
void DrawHorizontalRunZSin(unsigned char **ScreenPtr, int *ofs, float *zfrac, float *zstep, int XAdvance,
                           int RunLength, unsigned char Color[3])
{
    int i;
    unsigned char *WorkingScreenPtr = *ScreenPtr;

    for (i=0; i<RunLength; i++)
    {
        if (r_zbuffer[*ofs] <= *zfrac)
        {
            r_zbuffer[*ofs] = *zfrac;

            *WorkingScreenPtr       = Color[2];
            *(WorkingScreenPtr + 1) = Color[1];
            *(WorkingScreenPtr + 2) = Color[0];
        };
        WorkingScreenPtr += 3L*XAdvance;
        *ofs += XAdvance;
        *zfrac += *zstep;
    }
    /* Advance to the next scan line */
    WorkingScreenPtr += 3L*r_width;
    *ofs += r_width;
    *ScreenPtr = WorkingScreenPtr;
}

#define X_START 1
/* Regular BSP Draws a vertical run of pixels, then advances the bitmap pointer to
   the first pixel of the next run. */
void DrawVerticalRunZ(unsigned char **ScreenPtr, int *ofs, float *zfrac, float *zstep, int XAdvance,
                      int RunLength, unsigned char Color)
{
    int i;
    unsigned char *WorkingScreenPtr = *ScreenPtr;

    for (i=0; i<RunLength; i++)
    {
        if (r_zbuffer[*ofs] <= *zfrac)
        {
            r_zbuffer[*ofs] = *zfrac;
            *WorkingScreenPtr = Color;
        };
        WorkingScreenPtr += r_width;
        *ofs += r_width;
        *zfrac += *zstep;
    }
    /* Advance to the next scan line */
    WorkingScreenPtr += XAdvance;
    *ofs += XAdvance;
    *ScreenPtr = WorkingScreenPtr;
}


/* SIN Draws a vertical run of pixels, then advances the bitmap pointer to
   the first pixel of the next run. */
void DrawVerticalRunZSin(unsigned char **ScreenPtr, int *ofs, float *zfrac, float *zstep, int XAdvance,
                         int RunLength, unsigned char Color[3])
{
    int i;
    unsigned char *WorkingScreenPtr = *ScreenPtr;

    for (i=0; i<RunLength; i++)
    {
        if (r_zbuffer[*ofs] <= *zfrac)
        {
            r_zbuffer[*ofs] = *zfrac;
            *WorkingScreenPtr       = Color[2];
            *(WorkingScreenPtr + 1) = Color[1];
            *(WorkingScreenPtr + 2) = Color[0];
        };
        WorkingScreenPtr += 3L*r_width;
        *ofs += r_width;
        *zfrac += *zstep;
    }
    /* Advance to the next scan line */
    WorkingScreenPtr += 3L*XAdvance;
    *ofs += XAdvance;
    *ScreenPtr = WorkingScreenPtr;
}

#define CURLOC (ScreenPtr - r_picbuffer)
/* Regular BSP Draws a line between the specified endpoints in color Color. */
void REN_LineDrawZ(float *p1, float *p2, int selected)
{
    int Temp, AdjUp, AdjDown, ErrorTerm, XAdvance, XDelta, YDelta;
    int WholeStep, InitialPixelCount, FinalPixelCount, i, RunLength;
    int ofs;
    unsigned char *ScreenPtr;
    unsigned char whichColor;

    int XStart, YStart, XEnd, YEnd;

    if (!p1[2] || !p2[2])
        return;

    XStart = (int)(p1[0]);
    YStart = (int)(p1[1]);
    XEnd   = (int)(p2[0]);
    YEnd   = (int)(p2[1]);

    float ZStart, ZEnd;
    float ZStep;

    ZStart = p1[2];
    ZEnd   = p2[2];

    ZStep = ZEnd - ZStart;
    // spare clipping stuff...

    if (YStart < YEnd)
    {
        if (YStart < 0)
        {
            if (YEnd < 0)
                return;

            float t = -(float)YStart/(float)(YEnd - YStart);
            XStart += (int)(t*(float)(XEnd - XStart));
            YStart = 0;
        };
        // spare clipping stuff...
        if (YEnd >= r_height)
        {
            if (YStart >= r_height)
                return;

            float t = (float)(r_height - YEnd)/(float)(YEnd - YStart);
            XEnd += (int)(t*(float)(XEnd - XStart));
            YEnd = r_height - 1;
        };
    }
    else
    {
        if (YEnd < 0)
        {
            if (YStart < 0)
                return;

            float t = -(float)YEnd/(float)(YStart - YEnd);
            XStart += (int)(t*(float)(XEnd - XStart));
            YEnd = 0;

        };
        // spare clipping stuff...
        if (YStart >= r_height)
        {
            if (YEnd >= r_height)
                return;

            float t = (float)(r_height - YStart)/(float)(YStart - YEnd);
            XEnd += (int)(t*(float)(XEnd - XStart));
            YStart = r_height - 1;
        };
    };

    if (XEnd >= XStart)
    {
        if (XStart < X_START)
        {
            if (XEnd < X_START)
                return;

            float t = -(float)XStart/(float)(XEnd - XStart);
            YStart += (int)(t*(float)(YEnd - YStart));
            XStart = X_START;
        };
        if (XEnd >= r_width)
        {
            if (XStart >= r_width)
                return;
            float t= (float)(r_width - XEnd)/(float)(XEnd - XStart);
            YEnd += (int)(t*(float)(YEnd - YStart));
            XEnd = r_width - 1;
        };
    }
    else
    {
        if (XEnd < X_START)
        {
            if (XStart < X_START)
                return;

            float t = -(float)XEnd/(float)(XStart - XEnd);
            YStart += (int)(t*(float)(YEnd - YStart));
            XEnd = X_START;
        };
        if (XStart >= r_width)
        {
            if (XEnd >= r_width)
                return;
            float t;

            t = (float)(r_width - XStart)/(float)(XStart - XEnd);

            YEnd += (int)(t*(float)(YEnd - YStart));
            XStart = r_width - 1;

        };
    };
    /* We'll always draw top to bottom, to reduce the number of cases we have to
    handle, and to make lines between the same endpoints draw the same pixels */
    if (YStart > YEnd)
    {
        Temp = YStart;
        YStart = YEnd;
        YEnd = Temp;
        Temp = XStart;
        XStart = XEnd;
        XEnd = Temp;
        ZStep = -ZStep;
        ZStart = ZEnd;
    };

    /* Point to the bitmap address first pixel to draw */
    // ScreenPtr = MK_FP(SCREEN_SEGMENT, YStart * r_width + XStart);
    ofs = (YStart * r_width + XStart);

    ScreenPtr = &r_picbuffer[ofs];

    REN_SetupColors(&whichColor,selected);

    /* Figure out whether we're going left or right, and how huge we're
       going horizontally */
    if ((XDelta = (XEnd - XStart)) < 0)
    {
        XAdvance = -1;
        XDelta = -XDelta;
    }
    else
    {
        XAdvance = 1;
    }
    /* Figure out how huge we're going vertically */
    YDelta = YEnd - YStart;

    float zfrac = ZStart;
    /* Special-case horizontal, vertical, and diagonal lines, for speed
       and to avoid nasty boundary conditions and division by 0 */
    if (XDelta == 0)
    {
        if (YDelta)
            ZStep /= YDelta;

        /* Vertical line */
        for (i=0; i<=YDelta; i++)
        {
            if (r_zbuffer[ofs] <= zfrac)
            {
                r_zbuffer[ofs] = zfrac;
                *ScreenPtr = whichColor;
            };
            ScreenPtr += r_width;
            ofs += r_width;
            zfrac += ZStep;
        }
        return;
    }
    if (YDelta == 0)
    {
        if (XDelta)
            ZStep /= XDelta;

        /* Horizontal line */
        for (i=0; i<=XDelta; i++)
        {
            if (r_zbuffer[ofs] <= zfrac)
            {
                r_zbuffer[ofs] = zfrac;
                *ScreenPtr = whichColor;
            };
            ScreenPtr += XAdvance;
            ofs += XAdvance;
            zfrac += ZStep;
        }
        return;
    }
    if (XDelta == YDelta)
    {
        if (XDelta)
            ZStep /= XDelta;
        /* Diagonal line */
        for (i=0; i<=XDelta; i++)
        {
            if (r_zbuffer[ofs] <= zfrac)
            {
                r_zbuffer[ofs] = zfrac;
                *ScreenPtr = whichColor;
            };
            ScreenPtr += (XAdvance + r_width);
            ofs += (XAdvance + r_width);
            zfrac += ZStep;
        }
        return;
    }

    /* Determine whether the line is X or Y major, and handle accordingly */
    if (XDelta >= YDelta)
    {
        if (XDelta)
            ZStep /= XDelta;

        /* X major line */
        /* Minimum # of pixels in a run in this line */
        WholeStep = (int)(XDelta / YDelta);

        /* Error term adjust each time Y steps by 1; used to tell when one
           extra pixel should be drawn as part of a run, to account for
           fractional steps along the X axis per 1-pixel steps along Y */
        AdjUp = (int)((XDelta % YDelta) * 2);

        /* Error term adjust when the error term turns over, used to factor
           out the X step made at that time */
        AdjDown = (int)(YDelta * 2);

        /* Initial error term; reflects an initial step of 0.5 along the Y
           axis */
        ErrorTerm = (int)((XDelta % YDelta) - (YDelta * 2));

        /* The initial and last runs are partial, because Y advances only 0.5
           for these runs, rather than 1. Divide one full run, plus the
           initial pixel, between the initial and last runs */
        InitialPixelCount = (int)((WholeStep / 2) + 1);
        FinalPixelCount = InitialPixelCount;

        /* If the basic run length is even and there's no fractional
           advance, we have one pixel that could go to either the initial
           or last partial run, which we'll arbitrarily allocate to the
           last run */
        if ((AdjUp == 0) && ((WholeStep & 0x01) == 0))
        {
            InitialPixelCount--;
        }
        /* If there're an odd number of pixels per run, we have 1 pixel that can't
           be allocated to either the initial or last partial run, so we'll add 0.5
           to error term so this pixel will be handled by the normal full-run loop */
        if ((WholeStep & 0x01) != 0)
        {
            ErrorTerm = (int)(ErrorTerm+YDelta);
        }
        /* Draw the first, partial run of pixels */
        DrawHorizontalRunZ(&ScreenPtr, &ofs, &zfrac, &ZStep, XAdvance, InitialPixelCount, whichColor);

        /* Draw all full runs */
        for (i=0; i<(YDelta-1); i++)
        {
            RunLength = WholeStep;  /* run is at least this long */
            /* Advance the error term and add an extra pixel if the error
               term so indicates */
            if ((ErrorTerm += AdjUp) > 0)
            {
                RunLength++;
                ErrorTerm = (int)(ErrorTerm - AdjDown);   /* reset the error term */
            }
            /* Draw this scan line's run */
            DrawHorizontalRunZ(&ScreenPtr, &ofs, &zfrac, &ZStep, XAdvance, RunLength, whichColor);
        }
        /* Draw the final run of pixels */
        DrawHorizontalRunZ(&ScreenPtr, &ofs, &zfrac, &ZStep, XAdvance, FinalPixelCount, whichColor);
        return;
    }
    else
    {
        /* Y major line */
        if (YDelta)
            ZStep /= YDelta;

        /* Minimum # of pixels in a run in this line */
        WholeStep = (int)(YDelta / XDelta);

        /* Error term adjust each time X steps by 1; used to tell when 1 extra
           pixel should be drawn as part of a run, to account for
           fractional steps along the Y axis per 1-pixel steps along X */
        AdjUp = (int)((YDelta % XDelta) * 2);

        /* Error term adjust when the error term turns over, used to factor
           out the Y step made at that time */
        AdjDown = (int)(XDelta * 2);

        /* Initial error term; reflects initial step of 0.5 along the X axis */
        ErrorTerm = (int)((YDelta % XDelta) - (XDelta * 2));

        /* The initial and last runs are partial, because X advances only 0.5
           for these runs, rather than 1. Divide one full run, plus the
           initial pixel, between the initial and last runs */
        InitialPixelCount = (int)((WholeStep / 2) + 1);
        FinalPixelCount = InitialPixelCount;

        /* If the basic run length is even and there's no fractional advance, we
           have 1 pixel that could go to either the initial or last partial run,
           which we'll arbitrarily allocate to the last run */
        if (!AdjUp && !(WholeStep & 0x01))
        {
            InitialPixelCount--;
        }
        /* If there are an odd number of pixels per run, we have one pixel
           that can't be allocated to either the initial or last partial
           run, so we'll add 0.5 to the error term so this pixel will be
           handled by the normal full-run loop */
        if ((WholeStep & 0x01))
        {
            ErrorTerm = (int)(ErrorTerm + XDelta);
        }
        /* Draw the first, partial run of pixels */
        DrawVerticalRunZ(&ScreenPtr, &ofs, &zfrac, &ZStep, XAdvance, InitialPixelCount, whichColor);

        /* Draw all full runs */
        for (i=0; i<(XDelta-1); i++)
        {
            RunLength = WholeStep;  /* run is at least this long */
            /* Advance the error term and add an extra pixel if the error
               term so indicates */
            if ((ErrorTerm += AdjUp) > 0)
            {
                RunLength++;
                ErrorTerm = (int)(ErrorTerm - AdjDown);   /* reset the error term */
            }
            /* Draw this scan line's run */
            DrawVerticalRunZ(&ScreenPtr, &ofs, &zfrac, &ZStep, XAdvance, RunLength, whichColor);
        }
        /* Draw the final run of pixels */
        DrawVerticalRunZ(&ScreenPtr, &ofs, &zfrac, &ZStep, XAdvance, FinalPixelCount, whichColor);
        return;
    }
}

/* SIN Draws a line between the specified endpoints in color Color. */
void REN_LineDrawZSin(float *p1, float *p2, int selected)
{
    int Temp, AdjUp, AdjDown, ErrorTerm, XAdvance, XDelta, YDelta;
    int WholeStep, InitialPixelCount, FinalPixelCount, i, RunLength;
    int ofs;
    unsigned char *ScreenPtr;
    unsigned char whichColor[3];

    int XStart, YStart, XEnd, YEnd;

    if (!p1[2] || !p2[2])
        return;

    XStart = (int)(p1[0]);
    YStart = (int)(p1[1]);
    XEnd   = (int)(p2[0]);
    YEnd   = (int)(p2[1]);

    float ZStart, ZEnd;
    float ZStep;

    ZStart = p1[2];
    ZEnd   = p2[2];


    ZStep = ZEnd - ZStart;
    // spare clipping stuff...

    if (YStart < YEnd)
    {
        if (YStart < 0)
        {
            if (YEnd >= 0)
            {
                float t;

                t = -(float)YStart/(float)(YEnd - YStart);

                XStart += (int)(t*(float)(XEnd - XStart));
                YStart = 0;
            }
            else
            {
                return;
            };
        };
        // spare clipping stuff...
        if (YEnd >= r_height)
        {
            if (YStart < r_height)
            {
                float t;

                t = (float)(r_height - YEnd)/(float)(YEnd - YStart);

                XEnd += (int)(t*(float)(XEnd - XStart));
                YEnd = r_height - 1;
            }
            else
            {
                return;
            };
        };
    }
    else
    {
        if (YEnd < 0)
        {
            if (YStart >= 0)
            {
                float t;

                t = -(float)YEnd/(float)(YStart - YEnd);

                XStart += (int)(t*(float)(XEnd - XStart));
                YEnd = 0;
            }
            else
            {
                return;
            };
        };
        // spare clipping stuff...
        if (YStart >= r_height)
        {
            if (YEnd < r_height)
            {
                float t;

                t = (float)(r_height - YStart)/(float)(YStart - YEnd);

                XEnd += (int)(t*(float)(XEnd - XStart));
                YStart = r_height - 1;
            }
            else
            {
                return;
            };
        };
    };

    if (XEnd >= XStart)
    {
        if (XStart < X_START)
        {
            if (XEnd >= X_START)
            {
                float t;

                t = -(float)XStart/(float)(XEnd - XStart);

                YStart += (int)(t*(float)(YEnd - YStart));
                XStart = X_START;
            }
            else
            {
                return;
            };
        };
        if (XEnd >= r_width)
        {
            if (XStart < r_width)
            {
                float t;

                t = (float)(r_width - XEnd)/(float)(XEnd - XStart);

                YEnd += (int)(t*(float)(YEnd - YStart));
                XEnd = r_width - 1;
            }
            else
            {
                return;
            };
        };
    }
    else
    {
        if (XEnd < X_START)
        {
            if (XStart >= X_START)
            {
                float t;

                t = -(float)XEnd/(float)(XStart - XEnd);

                YStart += (int)(t*(float)(YEnd - YStart));
                XEnd = X_START;
            }
            else
            {
                return;
            };
        };
        if (XStart >= r_width)
        {
            if (XEnd < r_width)
            {
                float t;

                t = (float)(r_width - XStart)/(float)(XStart - XEnd);

                YEnd += (int)(t*(float)(YEnd - YStart));
                XStart = r_width - 1;
            }
            else
            {
                return;
            };
        };
    };
    /* We'll always draw top to bottom, to reduce the number of cases we have to
    handle, and to make lines between the same endpoints draw the same pixels */
    if (YStart > YEnd)
    {
        Temp = YStart;
        YStart = YEnd;
        YEnd = Temp;
        Temp = XStart;
        XStart = XEnd;
        XEnd = Temp;
        ZStep = -ZStep;
        ZStart = ZEnd;
    };

    /* Point to the bitmap address first pixel to draw */
    // ScreenPtr = MK_FP(SCREEN_SEGMENT, YStart * r_width + XStart);
    ofs = (YStart * r_width + XStart);

    ScreenPtr = &r_picbuffer[(ofs << 1) + ofs];

    REN_SetupColorsSin(&whichColor[0],&whichColor[1],&whichColor[2],selected);

    /* Figure out whether we're going left or right, and how huge we're
       going horizontally */
    if ((XDelta = (XEnd - XStart)) < 0)
    {
        XAdvance = -1;
        XDelta = -XDelta;
    }
    else
    {
        XAdvance = 1;
    }
    /* Figure out how huge we're going vertically */
    YDelta = YEnd - YStart;

    float zfrac;
    zfrac = ZStart;
    /* Special-case horizontal, vertical, and diagonal lines, for speed
       and to avoid nasty boundary conditions and division by 0 */
    if (XDelta == 0)
    {
        if (YDelta)
            ZStep /= YDelta;

        /* Vertical line */
        for (i=0; i<=YDelta; i++)
        {
            if (r_zbuffer[ofs] <= zfrac)
            {
                r_zbuffer[ofs] = zfrac;
                *ScreenPtr     = whichColor[2];
                *(ScreenPtr+1) = whichColor[1];
                *(ScreenPtr+2) = whichColor[0];
            };
            ScreenPtr += 3L*r_width;
            ofs += r_width;
            zfrac += ZStep;
        }
        return;
    }
    if (YDelta == 0)
    {
        if (XDelta)
            ZStep /= XDelta;

        /* Horizontal line */
        for (i=0; i<=XDelta; i++)
        {
            if (r_zbuffer[ofs] <= zfrac)
            {
                r_zbuffer[ofs] = zfrac;
                *ScreenPtr     = whichColor[2];
                *(ScreenPtr+1) = whichColor[1];
                *(ScreenPtr+2) = whichColor[0];
            };
            ScreenPtr += 3L*XAdvance;
            ofs += XAdvance;
            zfrac += ZStep;
        }
        return;
    }
    if (XDelta == YDelta)
    {
        if (XDelta)
            ZStep /= XDelta;
        /* Diagonal line */
        for (i=0; i<=XDelta; i++)
        {
            if (r_zbuffer[ofs] <= zfrac)
            {
                r_zbuffer[ofs] = zfrac;
                *ScreenPtr     = whichColor[2];
                *(ScreenPtr+1) = whichColor[1];
                *(ScreenPtr+2) = whichColor[0];
            };
            ScreenPtr += 3L*(XAdvance + r_width);
            ofs += (XAdvance + r_width);
            zfrac += ZStep;
        }
        return;
    }

    /* Determine whether the line is X or Y major, and handle accordingly */
    if (XDelta >= YDelta)
    {
        if (XDelta)
            ZStep /= XDelta;

        /* X major line */
        /* Minimum # of pixels in a run in this line */
        WholeStep = (int)(XDelta / YDelta);

        /* Error term adjust each time Y steps by 1; used to tell when one
           extra pixel should be drawn as part of a run, to account for
           fractional steps along the X axis per 1-pixel steps along Y */
        AdjUp = (int)((XDelta % YDelta) * 2);

        /* Error term adjust when the error term turns over, used to factor
           out the X step made at that time */
        AdjDown = (int)(YDelta * 2);

        /* Initial error term; reflects an initial step of 0.5 along the Y
           axis */
        ErrorTerm = (int)((XDelta % YDelta) - (YDelta * 2));

        /* The initial and last runs are partial, because Y advances only 0.5
           for these runs, rather than 1. Divide one full run, plus the
           initial pixel, between the initial and last runs */
        InitialPixelCount = (int)((WholeStep / 2) + 1);
        FinalPixelCount = InitialPixelCount;

        /* If the basic run length is even and there's no fractional
           advance, we have one pixel that could go to either the initial
           or last partial run, which we'll arbitrarily allocate to the
           last run */
        if (!AdjUp && !(WholeStep & 1))
        {
            InitialPixelCount--;
        }
        /* If there're an odd number of pixels per run, we have 1 pixel that can't
           be allocated to either the initial or last partial run, so we'll add 0.5
           to error term so this pixel will be handled by the normal full-run loop */
        if (WholeStep & 1)
        {
            ErrorTerm = (int)(ErrorTerm+YDelta);
        }
        /* Draw the first, partial run of pixels */
        DrawHorizontalRunZSin(&ScreenPtr, &ofs, &zfrac, &ZStep, XAdvance, InitialPixelCount, whichColor);

        /* Draw all full runs */
        for (i=0; i<(YDelta-1); i++)
        {
            RunLength = WholeStep;  /* run is at least this long */
            /* Advance the error term and add an extra pixel if the error
               term so indicates */
            if ((ErrorTerm += AdjUp) > 0)
            {
                RunLength++;
                ErrorTerm = (int)(ErrorTerm - AdjDown);   /* reset the error term */
            }
            /* Draw this scan line's run */
            DrawHorizontalRunZSin(&ScreenPtr, &ofs, &zfrac, &ZStep, XAdvance, RunLength, whichColor);
        }
        /* Draw the final run of pixels */
        DrawHorizontalRunZSin(&ScreenPtr, &ofs, &zfrac, &ZStep, XAdvance, FinalPixelCount, whichColor);
        return;
    }
    else
    {
        /* Y major line */
        if (YDelta)
            ZStep /= YDelta;

        /* Minimum # of pixels in a run in this line */
        WholeStep = (int)(YDelta / XDelta);

        /* Error term adjust each time X steps by 1; used to tell when 1 extra
           pixel should be drawn as part of a run, to account for
           fractional steps along the Y axis per 1-pixel steps along X */
        AdjUp = (int)((YDelta % XDelta) * 2);

        /* Error term adjust when the error term turns over, used to factor
           out the Y step made at that time */
        AdjDown = (int)(XDelta * 2);

        /* Initial error term; reflects initial step of 0.5 along the X axis */
        ErrorTerm = (int)((YDelta % XDelta) - (XDelta * 2));

        /* The initial and last runs are partial, because X advances only 0.5
           for these runs, rather than 1. Divide one full run, plus the
           initial pixel, between the initial and last runs */
        InitialPixelCount = (int)((WholeStep / 2) + 1);
        FinalPixelCount = InitialPixelCount;

        /* If the basic run length is even and there's no fractional advance, we
           have 1 pixel that could go to either the initial or last partial run,
           which we'll arbitrarily allocate to the last run */
        if ((AdjUp == 0) && ((WholeStep & 0x01) == 0))
        {
            InitialPixelCount--;
        }
        /* If there are an odd number of pixels per run, we have one pixel
           that can't be allocated to either the initial or last partial
           run, so we'll add 0.5 to the error term so this pixel will be
           handled by the normal full-run loop */
        if ((WholeStep & 0x01) != 0)
        {
            ErrorTerm = (int)(ErrorTerm + XDelta);
        }
        /* Draw the first, partial run of pixels */
        DrawVerticalRunZSin(&ScreenPtr, &ofs, &zfrac, &ZStep, XAdvance, InitialPixelCount, whichColor);

        /* Draw all full runs */
        for (i=0; i<(XDelta-1); i++)
        {
            RunLength = WholeStep;  /* run is at least this long */
            /* Advance the error term and add an extra pixel if the error
               term so indicates */
            if ((ErrorTerm += AdjUp) > 0)
            {
                RunLength++;
                ErrorTerm = (int)(ErrorTerm - AdjDown);   /* reset the error term */
            }
            /* Draw this scan line's run */
            DrawVerticalRunZSin(&ScreenPtr, &ofs, &zfrac, &ZStep, XAdvance, RunLength, whichColor);
        }
        /* Draw the final run of pixels */
        DrawVerticalRunZSin(&ScreenPtr, &ofs, &zfrac, &ZStep, XAdvance, FinalPixelCount, whichColor);
        return;
    }
}
//these effect the view window
#define ZSCALE 512.f

void XRotatePoint(float *in, float *out, float c, float s)
{
    out[0] = in[0];
    out[1] = (in[1]*c) - (in[2]*s);
    out[2] = (in[1]*s) + (in[2]*c);
}

void YRotatePoint(float *in,float *out,float c, float s)
{
    out[0] = (in[0]*c) + (in[2]*s);
    out[1] = in[1];
    out[2] = (in[2]*c) - (in[0]*s);
}

void ZRotatePoint(float *in, float *out, float c, float s)
{
    out[0] = (in[0]*c) + (in[1]*s);
    out[1] = (in[1]*c) - (in[0]*s);
    out[2] = in[2];
}

// LINE DRAW THE MODEL
void REN_DrawModel(mdl_t *mdl, int selected,
                   float Xrot, float Yrot, float Zrot, int framenum, int Xoffs, int Yoffs, float Zoffs)
{
    int         i;
    vec3_t      p1,p2,p3,rp1,rp2,rp3;
    // GrVertex    vtx1, vtx2, vtx3;
    float       z;
    //float       sadd,invz;
    float       Xcos,Ycos,Zcos,Xsin,Ysin,Zsin;
    //bool        facesfront;
    //bool        v1OnSeam,v2OnSeam,v3OnSeam;

    //only need to calculate these rotation sins and cosines once
    //per frame
    Xcos = (float)cos(DEG2RAD(Xrot));
    Xsin = (float)sin(DEG2RAD(Xrot));

    Ycos = (float)cos(DEG2RAD(Yrot));
    Ysin = (float)sin(DEG2RAD(Yrot));

    Zcos = (float)cos(DEG2RAD(Zrot));
    Zsin = (float)sin(DEG2RAD(Zrot));

    //keep the frame value valid
    framenum = framenum % mdl->numframes;

    //Loop through all the triangles and display them
    for (i=0; i<mdl->numtris; i++)
    {

        //boolean telling if this triangle is on the "front"
        //or back of the model
        //facesfront = mdl->triangles[i].facesfront;

        //these booleans tell if the vertex is on the "seam"
        //of the skin. more information on the "seam" and
        //its importance can be found in accompanying documentation
        //v1OnSeam = mdl->verts[mdl->triangles[i].vertices[0]].onseam;
        //v2OnSeam = mdl->verts[mdl->triangles[i].vertices[1]].onseam;
        //v3OnSeam = mdl->verts[mdl->triangles[i].vertices[2]].onseam;


        //Dont worry about trying to understand this mess below.
        //All it does is pull the appropriate 3 points for the
        //ith triangle out of the model data structure in such
        //a way that the model stands straight up and faces front
        //when rotated 0 degrees in all directions

        // was 2 0 1
        // last and first was -
        p1[0] = ((mdl->frames[framenum].data[0][mdl->triangles[i].vertices[0]].x * mdl->scale[0]) + mdl->origin[0]);
        p1[1] = (mdl->frames[framenum].data[0][mdl->triangles[i].vertices[0]].y * mdl->scale[1]) + mdl->origin[1];
        p1[2] = ((mdl->frames[framenum].data[0][mdl->triangles[i].vertices[0]].z * mdl->scale[2]) + mdl->origin[2]);

        p2[0] = ((mdl->frames[framenum].data[0][mdl->triangles[i].vertices[1]].x * mdl->scale[0]) + mdl->origin[0]);
        p2[1] = (mdl->frames[framenum].data[0][mdl->triangles[i].vertices[1]].y * mdl->scale[1]) + mdl->origin[1];
        p2[2] = ((mdl->frames[framenum].data[0][mdl->triangles[i].vertices[1]].z * mdl->scale[2]) + mdl->origin[2]);

        p3[0] = ((mdl->frames[framenum].data[0][mdl->triangles[i].vertices[2]].x * mdl->scale[0]) + mdl->origin[0]);
        p3[1] = (mdl->frames[framenum].data[0][mdl->triangles[i].vertices[2]].y * mdl->scale[1]) + mdl->origin[1];
        p3[2] = ((mdl->frames[framenum].data[0][mdl->triangles[i].vertices[2]].z * mdl->scale[2]) + mdl->origin[2]);

        /*
        VectorCopy(p1,rp1);
        VectorCopy(p2,rp2);
        VectorCopy(p3,rp3);
        */
        //rotate each point
        XRotatePoint(p1,rp1,Xcos,Xsin);
        YRotatePoint(rp1,p1,Ycos,Ysin);
        ZRotatePoint(p1,rp1,Zcos,Zsin);

        XRotatePoint(p2,rp2,Xcos,Xsin);
        YRotatePoint(rp2,p2,Ycos,Ysin);
        ZRotatePoint(p2,rp2,Zcos,Zsin);

        XRotatePoint(p3,rp3,Xcos,Xsin);
        YRotatePoint(rp3,p3,Ycos,Ysin);
        ZRotatePoint(p3,rp3,Zcos,Zsin);

        //simple perspective projection and screen centering

        winding_t *w = NewWinding (3);
        if(!w) return;

        //Point 1
        z = rp1[2] + Zoffs;
        //if (z==0.f) return; //just in case =)
        //invz = 1/z;
        w->points[0][0] = rp1[0] + Xoffs;
        w->points[0][1] = rp1[1] + Yoffs;
        w->points[0][2] = z;

        //sadd = (v1OnSeam && !facesfront)?((float)mdl->scaledwidth/2.f):(0);

        //vtx1.tmuvtx[0].sow = (float)(mdl->verts[mdl->triangles[i].vertices[0]].s + sadd)*invz;
        //vtx1.tmuvtx[0].tow = (float)(mdl->verts[mdl->triangles[i].vertices[0]].t)*invz;

        //Point 2
        z = rp2[2] + Zoffs;
        //if (z==0.f) return; //just in case =)
        //invz = 1/z;
        /*		vtx2[0] = (ZSCALE * rp2[0] * invz)+Xoffs;
        		vtx2[1] = (ZSCALE * rp2[1] * invz)+Yoffs;
        		vtx2.oow = invz;
        */
        w->points[1][0] = rp2[0] + Xoffs;
        w->points[1][1] = rp2[1] + Yoffs;
        w->points[1][2] = z;

//		sadd = (v2OnSeam && !facesfront)?((float)mdl->scaledwidth/2.f):(0.f);
//		vtx2.tmuvtx[0].sow = (float)(mdl->verts[mdl->triangles[i].vertices[1]].s + sadd)*invz;
//		vtx2.tmuvtx[0].tow = (float)(mdl->verts[mdl->triangles[i].vertices[1]].t)*invz;

        //Point 3
        z = rp3[2] + Zoffs;
        //	if (z==0.f) return; //just in case =)
        //	invz = 1/z;
        /*		vtx3[0] = (ZSCALE * rp3[0] * invz)+Xoffs;
        		vtx3[1] = (ZSCALE * rp3[1] * invz)+Yoffs;
        		vtx3.oow = invz;

        		sadd = (v3OnSeam && !facesfront)?((float)mdl->scaledwidth/2.f):(0.f);

        		vtx3.tmuvtx[0].sow = (float)(mdl->verts[mdl->triangles[i].vertices[2]].s + sadd)*invz;
        		vtx3.tmuvtx[0].tow = (float)(mdl->verts[mdl->triangles[i].vertices[2]].t)*invz;
        */
        w->points[2][0] = rp3[0] + Xoffs;
        w->points[2][1] = rp3[1] + Yoffs;
        w->points[2][2] = z;

        /*
        		//Colors are full bright so you can actually see the texture
        		//changing these to green would give it a green tint. kinda
        		//neat. I assume some sort of shading could also be done
        		vtx1.r = vtx2.r = vtx3.r = 255.f;
        		vtx1.g = vtx2.g = vtx3.g = 255.f;
        		vtx1.b = vtx2.b = vtx3.b = 255.f;

        		//snap vertices, suggested by glide
        		vtx1[0] += VTX_SNAP; vtx1[0] -= VTX_SNAP;
        		vtx1[1] += VTX_SNAP; vtx1[1] -= VTX_SNAP;
        		vtx1[2] += VTX_SNAP; vtx1[2] -= VTX_SNAP;

        		vtx2[0] += VTX_SNAP; vtx2[0] -= VTX_SNAP;
        		vtx2[1] += VTX_SNAP; vtx2[1] -= VTX_SNAP;
        		vtx2[2] += VTX_SNAP; vtx2[2] -= VTX_SNAP;

        		vtx3[0] += VTX_SNAP; vtx3[0] -= VTX_SNAP;
        		vtx3[1] += VTX_SNAP; vtx3[1] -= VTX_SNAP;
        		vtx3[2] += VTX_SNAP; vtx3[2] -= VTX_SNAP;
        */
        //Finally. Draw the damned triangle.
        // grDrawTriangle(&vtx1,&vtx2,&vtx3);
        REN_DrawWinding(w,selected);
        delete [] w;
    };
};

// LINE DRAW THE MODEL
void REN_RenderModel(mdl_t *mdl, int selected,
                     float Xrot, float Yrot, float Zrot, int framenum, int Xoffs, int Yoffs, float Zoffs, int skinNumber)
{
    int         i;
    vec3_t      p1,p2,p3,rp1,rp2,rp3;
    // GrVertex    vtx1, vtx2, vtx3;
    float       z;
    float       sadd; // ,invz;
    float       Xcos,Ycos,Zcos,Xsin,Ysin,Zsin;
    bool        facesfront;
    bool        v1OnSeam,v2OnSeam,v3OnSeam;
    bool        oldr_drawflat;

    r_facetrans = false;
    oldr_drawflat = r_drawflat;
    r_drawflat = false;
    // set texture stuff...
    t_width = mdl->skinwidth;
    t_height = mdl->skinheight;

    int r_skin;

    r_skin = skinNumber;
    r_skin = max(0,r_skin);
    r_skin = min(r_skin, (int)(mdl->numskins - 1));

    t_data = (unsigned char*)mdl->skins[r_skin].skin[0];

    t_widthadd   = t_width *1024.0f;
    t_heightadd  = t_height*1024.0f;

    //only need to calculate these rotation sins and cosines once
    //per frame
    Xcos = (float)cos(DEG2RAD(Xrot));
    Xsin = (float)sin(DEG2RAD(Xrot));

    Ycos = (float)cos(DEG2RAD(Yrot));
    Ysin = (float)sin(DEG2RAD(Yrot));

    Zcos = (float)cos(DEG2RAD(Zrot));
    Zsin = (float)sin(DEG2RAD(Zrot));

    //keep the frame value valid
    framenum = framenum % mdl->numframes;

    //Loop through all the triangles and display them
    for (i=0; i<mdl->numtris; i++)
    {

        //boolean telling if this triangle is on the "front"
        //or back of the model
        facesfront = mdl->triangles[i].facesfront;

        //these booleans tell if the vertex is on the "seam"
        //of the skin. more information on the "seam" and
        //its importance can be found in accompanying documentation
        v1OnSeam = mdl->verts[mdl->triangles[i].vertices[0]].onseam;
        v2OnSeam = mdl->verts[mdl->triangles[i].vertices[1]].onseam;
        v3OnSeam = mdl->verts[mdl->triangles[i].vertices[2]].onseam;


        //Dont worry about trying to understand this mess below.
        //All it does is pull the appropriate 3 points for the
        //ith triangle out of the model data structure in such
        //a way that the model stands straight up and faces front
        //when rotated 0 degrees in all directions

        // was 2 0 1
        // last and first was -
        p1[0] = ((mdl->frames[framenum].data[0][mdl->triangles[i].vertices[0]].x * mdl->scale[0]) + mdl->origin[0]);
        p1[1] = (mdl->frames[framenum].data[0][mdl->triangles[i].vertices[0]].y * mdl->scale[1]) + mdl->origin[1];
        p1[2] = ((mdl->frames[framenum].data[0][mdl->triangles[i].vertices[0]].z * mdl->scale[2]) + mdl->origin[2]);

        p2[0] = ((mdl->frames[framenum].data[0][mdl->triangles[i].vertices[1]].x * mdl->scale[0]) + mdl->origin[0]);
        p2[1] = (mdl->frames[framenum].data[0][mdl->triangles[i].vertices[1]].y * mdl->scale[1]) + mdl->origin[1];
        p2[2] = ((mdl->frames[framenum].data[0][mdl->triangles[i].vertices[1]].z * mdl->scale[2]) + mdl->origin[2]);

        p3[0] = ((mdl->frames[framenum].data[0][mdl->triangles[i].vertices[2]].x * mdl->scale[0]) + mdl->origin[0]);
        p3[1] = (mdl->frames[framenum].data[0][mdl->triangles[i].vertices[2]].y * mdl->scale[1]) + mdl->origin[1];
        p3[2] = ((mdl->frames[framenum].data[0][mdl->triangles[i].vertices[2]].z * mdl->scale[2]) + mdl->origin[2]);

        /*
        VectorCopy(p1,rp1);
        VectorCopy(p2,rp2);
        VectorCopy(p3,rp3);
        */
        //rotate each point
        XRotatePoint(p1,rp1,Xcos,Xsin);
        YRotatePoint(rp1,p1,Ycos,Ysin);
        ZRotatePoint(p1,rp1,Zcos,Zsin);

        XRotatePoint(p2,rp2,Xcos,Xsin);
        YRotatePoint(rp2,p2,Ycos,Ysin);
        ZRotatePoint(p2,rp2,Zcos,Zsin);

        XRotatePoint(p3,rp3,Xcos,Xsin);
        YRotatePoint(rp3,p3,Ycos,Ysin);
        ZRotatePoint(p3,rp3,Zcos,Zsin);

        //simple perspective projection and screen centering

        winding_t *w = NewWinding (3);
        if(!w) return;

        //Point 1
        z = rp1[2] + Zoffs;
        //if (z==0.f) return; //just in case =)
        //invz = 1/z;
        w->points[0][0] = rp1[0] + Xoffs;
        w->points[0][1] = rp1[1] + Yoffs;
        w->points[0][2] = z;

        sadd = (v1OnSeam && !facesfront) ? ((float)mdl->skinwidth/2.f):(0);
        w->points[0][3] = (float)(mdl->verts[mdl->triangles[i].vertices[0]].s + sadd);
        w->points[0][4] = (float)(mdl->verts[mdl->triangles[i].vertices[0]].t);

        //Point 2
        z = rp2[2] + Zoffs;
        //if (z==0.f) return; //just in case =)
        //invz = 1/z;
        /*		vtx2[0] = (ZSCALE * rp2[0] * invz)+Xoffs;
        		vtx2[1] = (ZSCALE * rp2[1] * invz)+Yoffs;
        		vtx2.oow = invz;
        */
        w->points[1][0] = rp2[0] + Xoffs;
        w->points[1][1] = rp2[1] + Yoffs;
        w->points[1][2] = z;

        sadd = (v2OnSeam && !facesfront) ? ((float)mdl->skinwidth/2.f):(0);
        w->points[1][3] = (float)(mdl->verts[mdl->triangles[i].vertices[1]].s + sadd);
        w->points[1][4] = (float)(mdl->verts[mdl->triangles[i].vertices[1]].t);

//		sadd = (v2OnSeam && !facesfront)?((float)mdl->scaledwidth/2.f):(0.f);
//		vtx2.tmuvtx[0].sow = (float)(mdl->verts[mdl->triangles[i].vertices[1]].s + sadd)*invz;
//		vtx2.tmuvtx[0].tow = (float)(mdl->verts[mdl->triangles[i].vertices[1]].t)*invz;

        //Point 3
        z = rp3[2] + Zoffs;
        //	if (z==0.f) return; //just in case =)
        //	invz = 1/z;
        /*		vtx3[0] = (ZSCALE * rp3[0] * invz)+Xoffs;
        		vtx3[1] = (ZSCALE * rp3[1] * invz)+Yoffs;
        		vtx3.oow = invz;

        		sadd = (v3OnSeam && !facesfront)?((float)mdl->scaledwidth/2.f):(0.f);

        		vtx3.tmuvtx[0].sow = (float)(mdl->verts[mdl->triangles[i].vertices[2]].s + sadd)*invz;
        		vtx3.tmuvtx[0].tow = (float)(mdl->verts[mdl->triangles[i].vertices[2]].t)*invz;
        */
        w->points[2][0] = rp3[0] + Xoffs;
        w->points[2][1] = rp3[1] + Yoffs;
        w->points[2][2] = z;
        sadd = (v3OnSeam && !facesfront) ? ((float)mdl->skinwidth/2.f):(0);
        w->points[2][3] = (float)(mdl->verts[mdl->triangles[i].vertices[2]].s + sadd);
        w->points[2][4] = (float)(mdl->verts[mdl->triangles[i].vertices[2]].t);

        REN_RenderWinding(w,selected);
        delete [] w;
    };
    r_drawflat = oldr_drawflat;
};



//BEGIN TEST
//
void glRenderModel(mdl_t *mdl, int selected,
                   float Xrot, float Yrot, float Zrot, int framenum, int Xoffs, int Yoffs, float Zoffs, int skinNumber)
{
    int         i;
    vec3_t      p1,p2,p3,rp1,rp2,rp3;
    float       z;
    float       sadd;
    float       Xcos,Ycos,Zcos,Xsin,Ysin,Zsin;
    bool        facesfront;
    bool        v1OnSeam,v2OnSeam,v3OnSeam;

    // set texture stuff...
    t_width = mdl->skinwidth;
    t_height = mdl->skinheight;

    int r_skin = skinNumber;
    r_skin = max(0,r_skin);
    r_skin = min(r_skin, (int)(mdl->numskins - 1));

    //only need to calculate these rotation sins and cosines once
    //per frame
    Xcos = (float)cos(DEG2RAD(Xrot));
    Xsin = (float)sin(DEG2RAD(Xrot));

    Ycos = (float)cos(DEG2RAD(Yrot));
    Ysin = (float)sin(DEG2RAD(Yrot));

    Zcos = (float)cos(DEG2RAD(Zrot));
    Zsin = (float)sin(DEG2RAD(Zrot));

    //keep the frame value valid
    framenum = framenum % mdl->numframes;

    if(mdl->numskins)
    {
        if(!mdl->gltex)
        {

            glGenTextures(1,(GLuint*) &mdl->gltex);
            glBindTexture(GL_TEXTURE_2D, mdl->gltex);

            BYTE *glbits = new BYTE[mdl->skinwidth * mdl->skinheight * 3];
            BYTE *gp = glbits;
            for(i = 0; i < mdl->skinwidth * mdl->skinheight; i++)
            {
                int pal = mdl->skins[0].skin[0][i];
                *gp++ = set.pal[PALSTEP*pal + 0];
                *gp++ = set.pal[PALSTEP*pal + 1];
                *gp++ = set.pal[PALSTEP*pal + 2];
            }
            glAddTexture(mdl->gltex, mdl->skinwidth, mdl->skinheight, (GLuint*) glbits);
            delete [] glbits;
        }

        glBindTexture(GL_TEXTURE_2D, mdl->gltex);
    }

    //Loop through all the triangles and display them
    for (i=0; i<mdl->numtris; i++)
    {

        //boolean telling if this triangle is on the "front"
        //or back of the model
        facesfront = mdl->triangles[i].facesfront;

        int *vert = mdl->triangles[i].vertices;

        //these booleans tell if the vertex is on the "seam"
        //of the skin. more information on the "seam" and
        //its importance can be found in accompanying documentation
        v1OnSeam = mdl->verts[vert[0]].onseam;
        v2OnSeam = mdl->verts[vert[1]].onseam;
        v3OnSeam = mdl->verts[vert[2]].onseam;


        //Dont worry about trying to understand this mess below.
        //All it does is pull the appropriate 3 points for the
        //ith triangle out of the model data structure in such
        //a way that the model stands straight up and faces front
        //when rotated 0 degrees in all directions

        // was 2 0 1
        // last and first was -
        rp1[0] = ((mdl->frames[framenum].data[0][vert[0]].x * mdl->scale[0]) + mdl->origin[0]);
        rp1[1] = (mdl->frames[framenum].data[0][vert[0]].y * mdl->scale[1]) + mdl->origin[1];
        rp1[2] = ((mdl->frames[framenum].data[0][vert[0]].z * mdl->scale[2]) + mdl->origin[2]);

        rp2[0] = ((mdl->frames[framenum].data[0][vert[1]].x * mdl->scale[0]) + mdl->origin[0]);
        rp2[1] = (mdl->frames[framenum].data[0][vert[1]].y * mdl->scale[1]) + mdl->origin[1];
        rp2[2] = ((mdl->frames[framenum].data[0][vert[1]].z * mdl->scale[2]) + mdl->origin[2]);

        rp3[0] = ((mdl->frames[framenum].data[0][vert[2]].x * mdl->scale[0]) + mdl->origin[0]);
        rp3[1] = (mdl->frames[framenum].data[0][vert[2]].y * mdl->scale[1]) + mdl->origin[1];
        rp3[2] = ((mdl->frames[framenum].data[0][vert[2]].z * mdl->scale[2]) + mdl->origin[2]);

        //rotate each point
        XRotatePoint(p1,rp1,Xcos,Xsin);
        YRotatePoint(rp1,p1,Ycos,Ysin);
        ZRotatePoint(p1,rp1,Zcos,Zsin);

        XRotatePoint(p2,rp2,Xcos,Xsin);
        YRotatePoint(rp2,p2,Ycos,Ysin);
        ZRotatePoint(p2,rp2,Zcos,Zsin);

        XRotatePoint(p3,rp3,Xcos,Xsin);
        YRotatePoint(rp3,p3,Ycos,Ysin);
        ZRotatePoint(p3,rp3,Zcos,Zsin);

        //simple perspective projection and screen centering

        vec5_t tri[3];

        //Point 1
        z = rp1[2] + Zoffs;

        tri[0][0] = rp1[0] + Xoffs;
        tri[0][1] = rp1[1] + Yoffs;
        tri[0][2] = z;

        sadd = (v1OnSeam && !facesfront) ? ((float)mdl->skinwidth/2.f):(0);
        tri[0][3] = (float)(mdl->verts[vert[0]].s + sadd)/ mdl->skinwidth;
        tri[0][4] = (float)(mdl->verts[vert[0]].t)/ mdl->skinheight;

        //Point 2
        z = rp2[2] + Zoffs;

        tri[1][0] = rp2[0] + Xoffs;
        tri[1][1] = rp2[1] + Yoffs;
        tri[1][2] = z;

        sadd = (v2OnSeam && !facesfront) ? ((float)mdl->skinwidth/2.f):(0);
        tri[1][3] = (float)(mdl->verts[vert[1]].s + sadd)/ mdl->skinwidth;
        tri[1][4] = (float)(mdl->verts[vert[1]].t)/ mdl->skinheight;

        //Point 3
        z = rp3[2] + Zoffs;

        tri[2][0] = rp3[0] + Xoffs;
        tri[2][1] = rp3[1] + Yoffs;
        tri[2][2] = z;
        sadd = (v3OnSeam && !facesfront) ? ((float)mdl->skinwidth/2.f):(0);
        tri[2][3] = (float)(mdl->verts[vert[2]].s + sadd)/ mdl->skinwidth;
        tri[2][4] = (float)(mdl->verts[vert[2]].t)/ mdl->skinheight;

        //calc normal
        vec3_t t1,t2,normal;
        for (int j=0 ; j<3 ; j++)
        {
            t1[j] = tri[0][j] - tri[1][j];
            t2[j] = tri[2][j] - tri[1][j];
        }
        CrossProduct(t1,t2, normal);
        VectorNormalize (normal);

        glBegin(GL_TRIANGLES);
        glNormal3fv(normal);
        glTexCoord2fv(&tri[0][3]);
        glVertex3fv(tri[0]);
        glTexCoord2fv(&tri[1][3]);
        glVertex3fv(tri[1]);
        glTexCoord2fv(&tri[2][3]);
        glVertex3fv(tri[2]);
        glEnd();
    }
}


void glCreateQ1ModelList(mdl_t *mdl)
{
    int         i;
    vec3_t      rp1,rp2,rp3;
    float       z;
    float       sadd;
    bool        facesfront;
    bool        v1OnSeam,v2OnSeam,v3OnSeam;

    // set texture stuff...
    t_width = mdl->skinwidth;
    t_height = mdl->skinheight;

    //keep the frame value valid
    int framenum = 0;

    //set display list num
    if(mdl->gl_list)
        glDeleteLists(mdl->gl_list, 1);
    mdl->gl_list = glGenLists(1);

    glNewList(mdl->gl_list, GL_COMPILE);

    if(mdl->numskins)
    {
        if(!mdl->gltex)
        {

            glGenTextures(1,(GLuint*) &mdl->gltex);
            glBindTexture(GL_TEXTURE_2D, mdl->gltex);

            BYTE *glbits = new BYTE[mdl->skinwidth * mdl->skinheight * 3];
            BYTE *gp = glbits;
            for(i = 0; i < mdl->skinwidth * mdl->skinheight; i++)
            {
                int pal = mdl->skins[0].skin[0][i];
                *gp++ = set.pal[PALSTEP*pal + 0];
                *gp++ = set.pal[PALSTEP*pal + 1];
                *gp++ = set.pal[PALSTEP*pal + 2];
            }
            glAddTexture(mdl->gltex, mdl->skinwidth, mdl->skinheight, (GLuint*) glbits);
            delete [] glbits;
        }

        glBindTexture(GL_TEXTURE_2D, mdl->gltex);
    }

    //Loop through all the triangles and display them
    for (i=0; i<mdl->numtris; i++)
    {

        //boolean telling if this triangle is on the "front"
        //or back of the model
        facesfront = mdl->triangles[i].facesfront;

        int *vert = mdl->triangles[i].vertices;

        //these booleans tell if the vertex is on the "seam"
        //of the skin. more information on the "seam" and
        //its importance can be found in accompanying documentation
        v1OnSeam = mdl->verts[vert[0]].onseam;
        v2OnSeam = mdl->verts[vert[1]].onseam;
        v3OnSeam = mdl->verts[vert[2]].onseam;


        //Dont worry about trying to understand this mess below.
        //All it does is pull the appropriate 3 points for the
        //ith triangle out of the model data structure in such
        //a way that the model stands straight up and faces front
        //when rotated 0 degrees in all directions

        // was 2 0 1
        // last and first was -
        rp1[0] = ((mdl->frames[framenum].data[0][vert[0]].x * mdl->scale[0]) + mdl->origin[0]);
        rp1[1] = (mdl->frames[framenum].data[0][vert[0]].y * mdl->scale[1]) + mdl->origin[1];
        rp1[2] = ((mdl->frames[framenum].data[0][vert[0]].z * mdl->scale[2]) + mdl->origin[2]);

        rp2[0] = ((mdl->frames[framenum].data[0][vert[1]].x * mdl->scale[0]) + mdl->origin[0]);
        rp2[1] = (mdl->frames[framenum].data[0][vert[1]].y * mdl->scale[1]) + mdl->origin[1];
        rp2[2] = ((mdl->frames[framenum].data[0][vert[1]].z * mdl->scale[2]) + mdl->origin[2]);

        rp3[0] = ((mdl->frames[framenum].data[0][vert[2]].x * mdl->scale[0]) + mdl->origin[0]);
        rp3[1] = (mdl->frames[framenum].data[0][vert[2]].y * mdl->scale[1]) + mdl->origin[1];
        rp3[2] = ((mdl->frames[framenum].data[0][vert[2]].z * mdl->scale[2]) + mdl->origin[2]);

        //simple perspective projection and screen centering

        vec5_t tri[3];

        //Point 1
        z = rp1[2];

        tri[0][0] = rp1[0];
        tri[0][1] = rp1[1];
        tri[0][2] = z;

        sadd = (v1OnSeam && !facesfront) ? ((float)mdl->skinwidth/2.f):(0);
        tri[0][3] = (float)(mdl->verts[vert[0]].s + sadd)/ mdl->skinwidth;
        tri[0][4] = (float)(mdl->verts[vert[0]].t)/ mdl->skinheight;

        //Point 2
        z = rp2[2];

        tri[1][0] = rp2[0];
        tri[1][1] = rp2[1];
        tri[1][2] = z;

        sadd = (v2OnSeam && !facesfront) ? ((float)mdl->skinwidth/2.f):(0);
        tri[1][3] = (float)(mdl->verts[vert[1]].s + sadd)/ mdl->skinwidth;
        tri[1][4] = (float)(mdl->verts[vert[1]].t)/ mdl->skinheight;

        //Point 3
        z = rp3[2];

        tri[2][0] = rp3[0];
        tri[2][1] = rp3[1];
        tri[2][2] = z;
        sadd = (v3OnSeam && !facesfront) ? ((float)mdl->skinwidth/2.f):(0);
        tri[2][3] = (float)(mdl->verts[vert[2]].s + sadd)/ mdl->skinwidth;
        tri[2][4] = (float)(mdl->verts[vert[2]].t)/ mdl->skinheight;

        //calc normal
        vec3_t t1,t2,normal;
        for (int j=0 ; j<3 ; j++)
        {
            t1[j] = tri[0][j] - tri[1][j];
            t2[j] = tri[2][j] - tri[1][j];
        }
        CrossProduct(t1,t2, normal);
        VectorNormalize (normal);

        glBegin(GL_TRIANGLES);
        glNormal3fv(normal);
        glTexCoord2fv(&tri[0][3]);
        glVertex3fv(tri[0]);
        glTexCoord2fv(&tri[1][3]);
        glVertex3fv(tri[1]);
        glTexCoord2fv(&tri[2][3]);
        glVertex3fv(tri[2]);
        glEnd();
    }
    glEndList();
}

///////////////////////////////////////////
// Q2 Model Drawing Functions...         //
///////////////////////////////////////////

static vec5_t vert_list[64];
// LINE DRAW THE MODEL
void REN_DrawModelQ2(dmdl_t *mdl, int selected,	float Xrot, float Yrot, float Zrot,
                     int framenum, int Xoffs, int Yoffs, float Zoffs)
{
    float ratio = 0.5;
    float skin_interp = 0.0f;

    int         i,nextframe;
    float       z,s,t;
    float       Xcos,Ycos,Zcos,Xsin,Ysin,Zsin;
    float       dest_x, dest_y;
    vec3_t      p1;
    vec3_t      p2;
    vec3_t      p,rp;
    int         j;

    float       *v;
    vec5_t      *vtx;
    daliasframe_t *frameinfo1,*frameinfo2;
    int         *command;
    int         num_verts,vert_index,verts_processed;
    int         command_type;

    //only need to calculate these rotation sins and cosines once
    //per frame
    Xcos = (float)cos(DEG2RAD(Xrot));
    Xsin = (float)sin(DEG2RAD(Xrot));

    Ycos = (float)cos(DEG2RAD(Yrot));
    Ysin = (float)sin(DEG2RAD(Yrot));

    Zcos = (float)cos(DEG2RAD(Zrot));
    Zsin = (float)sin(DEG2RAD(Zrot));

    //keep the frame value valid
    framenum  = framenum % mdl->numframes;
    nextframe = (framenum+1) % mdl->numframes;

    frameinfo1 = (daliasframe_t *)((char *)mdl->frames + mdl->framesize*framenum);
    frameinfo2 = (daliasframe_t *)((char *)mdl->frames + mdl->framesize*nextframe);

    verts_processed = 0;
    command = mdl->glcmds;

    winding_t *w = NewWinding (3);
    if(!w) return;

    //do the gl commands <?>
    while (*command)
    {
        if (*command>0)
        {
            //triangle strip
            num_verts = *command;
            command_type = 0;
        }
        else
        {
            //triangle fan
            num_verts = -(*command);
            command_type = 1;
        };

        command++;

        for (i=0,vtx=vert_list; i<num_verts; i++,vtx++,verts_processed++)
        {
            v = (float *)vtx;
            //grab the floating point s and t
            s = *((float *)command);
            command++;
            t = *((float *)command);
            command++;

            v[3] = s * mdl->s_scale;
            v[4] = t * mdl->t_scale;

            //grab the vertex index
            vert_index = *command;
            command++;

            //grab the 2 vertices to interpolate between
            for (j = 0; j < 3; j++)
            {
                p1[j] = ((frameinfo1->verts[vert_index].v[j] * frameinfo1->scale[j]) + frameinfo1->origin[j]);
                p2[j] = ((frameinfo2->verts[vert_index].v[j] * frameinfo2->scale[j]) + frameinfo2->origin[j]);

                p[j] = p1[j] + (p2[j] - p1[j])*ratio;
            };

            //transform the point
            XRotatePoint(p,rp,Xcos,Xsin);
            YRotatePoint(rp,p,Ycos,Ysin);
            ZRotatePoint(p,rp,Zcos,Zsin);

            //setup the point
            z = rp[2] + Zoffs;

            v[0] = rp[0] + Xoffs;
            v[1] = rp[1] + Yoffs;
            v[2] = z;

            /*invz   = 1.f / z;
            v->x   = (ZSCALE * rp.x * invz) + Xoffs;
              v->y   = (ZSCALE * rp.y * invz) + Yoffs;
              v->oow = invz;
            */
            if (skin_interp != 0.f)
            {
                dest_x = (float)mdl->skinwidth  * s + mdl->skin_screen_offset_x;
                dest_y = (float)mdl->skinheight * t + mdl->skin_screen_offset_y;

                v[0] += (dest_x - v[0]) * skin_interp;
                v[1] += (dest_y - v[1]) * skin_interp;
                v[2] += (1.0f - z)*skin_interp;
                /*
                v->x = v->x + (dest_x - v->x) * skin_interp;
                v->y = v->y + (dest_y - v->y) * skin_interp;
                z    = z    + (1.f - z)*skin_interp;

                invz   = 1.f / z;
                v->oow = invz;
                */
            }
        }

        switch (command_type)
        {
        case 0:
            //tristrip
            for (i=0; i<num_verts-2; i++)
            {
                //this is to ensure correct vertex order
                if (i & 1)
                {
                    for (j = 0; j < 5; j++)
                    {
                        w->points[0][j] = vert_list[i][j];
                        w->points[1][j] = vert_list[i+2][j];
                        w->points[2][j] = vert_list[i+1][j];
                    };
                    //guDrawTriangleWithClip(&vert_list[i],&vert_list[i+2],&vert_list[i+1]);
                }
                else
                {
                    for (j = 0; j < 5; j++)
                    {
                        w->points[0][j] = vert_list[i][j];
                        w->points[1][j] = vert_list[i+1][j];
                        w->points[2][j] = vert_list[i+2][j];
                    };
                    //   guDrawTriangleWithClip(&vert_list[i],&vert_list[i+1],&vert_list[i+2]);
                };
                REN_DrawWinding(w,selected);
            }
            break;
        case 1:
            //trifan
            for (i=0; i<num_verts-2; i++)
            {
                for (j = 0; j < 5; j++)
                {
                    w->points[0][j] = vert_list[0][j];
                    w->points[1][j] = vert_list[i+1][j];
                    w->points[2][j] = vert_list[i+2][j];
                };
                //guDrawTriangleWithClip(&vert_list[0],&vert_list[i+1],&vert_list[i+2]);
                REN_DrawWinding(w,selected);
            }
            break;
        }
    };

    delete [] w;
};

// RENDER THE Q2 MODEL
void REN_RenderModelQ2(dmdl_t *mdl, dskindata_t *skinTex, int selected,
                       float Xrot, float Yrot, float Zrot, int framenum, int Xoffs, int Yoffs, float Zoffs, int skinNumber)
{
    float ratio = 0.5;
    float skin_interp = 0.0f;

    int         i,nextframe;
    float       z,s,t;
    float       Xcos,Ycos,Zcos,Xsin,Ysin,Zsin;
    float       dest_x, dest_y;
    vec3_t      p1;
    vec3_t      p2;
    vec3_t      p,rp;
    int         j;

    float       *v;
    vec5_t      *vtx;
    daliasframe_t *frameinfo1,*frameinfo2;
    int         *command;
    int         num_verts,vert_index,verts_processed;
    int         command_type;

    //BLAH! set some global variables for rendering... ... .. . ..
    // set texture stuff...
    t_width  = skinTex->skinwidth;
    t_height = skinTex->skinheight;

    bool save_r_drawflat = r_drawflat;
    r_drawflat = false;
    r_facetrans = false;

    int r_skin = skinNumber;
    r_skin = max(0,r_skin);
    r_skin = min(r_skin, (int)(mdl->num_skins - 1));

    t_data = (unsigned char *)skinTex->data; // mdl->skins[r_skin].skin[0];

    //only need to calculate these rotation sins and cosines once
    //per frame
    Xcos = (float)cos(DEG2RAD(Xrot));
    Xsin = (float)sin(DEG2RAD(Xrot));

    Ycos = (float)cos(DEG2RAD(Yrot));
    Ysin = (float)sin(DEG2RAD(Yrot));

    Zcos = (float)cos(DEG2RAD(Zrot));
    Zsin = (float)sin(DEG2RAD(Zrot));

    //keep the frame value valid
    framenum  = framenum % mdl->numframes;
    nextframe = (framenum+1) % mdl->numframes;

    frameinfo1 = (daliasframe_t *)((char *)mdl->frames + mdl->framesize*framenum);
    frameinfo2 = (daliasframe_t *)((char *)mdl->frames + mdl->framesize*nextframe);

    verts_processed = 0;
    command = mdl->glcmds;

    winding_t *w = NewWinding (3);
    if(!w) return;

    //do the gl commands <?>
    while (*command)
    {
        if (*command>0)
        {
            //triangle strip
            num_verts = *command;
            command_type = 0;
        }
        else
        {
            //triangle fan
            num_verts = -(*command);
            command_type = 1;
        }

        command++;

        for (i=0,vtx=vert_list; i<num_verts; i++,vtx++,verts_processed++)
        {
            v = (float *)vtx;
            //grab the floating point s and t
            s = *((float *)command);
            command++;
            t = *((float *)command);
            command++;

            v[3] = s * mdl->s_scale;
            v[4] = t * mdl->t_scale;

            //grab the vertex index
            vert_index = *command;
            command++;

            //grab the 2 vertices to interpolate between
            for (j = 0; j < 3; j++)
            {
                p1[j] = ((frameinfo1->verts[vert_index].v[j] * frameinfo1->scale[j]) + frameinfo1->origin[j]);
                p2[j] = ((frameinfo2->verts[vert_index].v[j] * frameinfo2->scale[j]) + frameinfo2->origin[j]);

                p[j] = p1[j] + (p2[j] - p1[j])*ratio;
            };

            //transform the point
            XRotatePoint(p,rp,Xcos,Xsin);
            YRotatePoint(rp,p,Ycos,Ysin);
            ZRotatePoint(p,rp,Zcos,Zsin);

            //setup the point
            z = rp[2] + Zoffs;

            v[0] = rp[0] + Xoffs;
            v[1] = rp[1] + Yoffs;
            v[2] = z;

            /*invz   = 1.f / z;
            v->x   = (ZSCALE * rp.x * invz) + Xoffs;
            v->y   = (ZSCALE * rp.y * invz) + Yoffs;
            v->oow = invz;
            */
            if (skin_interp != 0.f)
            {
                dest_x = (float)mdl->skinwidth  * s + mdl->skin_screen_offset_x;
                dest_y = (float)mdl->skinheight * t + mdl->skin_screen_offset_y;

                v[0] += (dest_x - v[0]) * skin_interp;
                v[1] += (dest_y - v[1]) * skin_interp;
                v[2] += (1.0f - z)*skin_interp;
                /*
                v->x = v->x + (dest_x - v->x) * skin_interp;
                v->y = v->y + (dest_y - v->y) * skin_interp;
                z    = z    + (1.f - z)*skin_interp;

                invz   = 1.f / z;
                v->oow = invz;
                */
            }
        }

        switch (command_type)
        {
        case 0:
            //tristrip
            for (i=0; i<num_verts-2; i++)
            {
                //this is to ensure correct vertex order
                if (i & 1)
                {
                    for (j = 0; j < 5; j++)
                    {
                        w->points[0][j] = vert_list[i][j];
                        w->points[1][j] = vert_list[i+2][j];     // 2
                        w->points[2][j] = vert_list[i+1][j];     // 1
                    };
                    //guDrawTriangleWithClip(&vert_list[i],&vert_list[i+2],&vert_list[i+1]);
                }
                else
                {
                    for (j = 0; j < 5; j++)
                    {
                        w->points[0][j] = vert_list[i][j];
                        w->points[1][j] = vert_list[i+1][j];
                        w->points[2][j] = vert_list[i+2][j];
                    };
                    //   guDrawTriangleWithClip(&vert_list[i],&vert_list[i+1],&vert_list[i+2]);
                };
                REN_RenderWinding(w,selected);
            }
            break;
        case 1:
            //trifan
            for (i=0; i<num_verts-2; i++)
            {
                for (j = 0; j < 5; j++)
                {
                    w->points[0][j] = vert_list[0][j];
                    w->points[1][j] = vert_list[i+1][j];
                    w->points[2][j] = vert_list[i+2][j];
                };
                //guDrawTriangleWithClip(&vert_list[0],&vert_list[i+1],&vert_list[i+2]);
                REN_RenderWinding(w,selected);
            }
            break;
        }
    }

    r_drawflat = save_r_drawflat;

    delete [] w;
//	r_drawflat = oldr_drawflat;
}


