
#include "global.h"


typedef struct patchDef2point_s
{
    vec3_t xyz;
    float tx;
    float ty;
} patchDef2point_t;

typedef struct patchDef2_s
{
    int cols;	//ctrl points of bezier
    int rows;	//width of spline :>
    patchDef2point_t *points;
} patchDef2_t;

patchDef2_t *createPatchDef2(int cols, int rows)
{
    patchDef2_t *retval = 0;
    if(rows >= 3 && cols >= 3 && cols & 1)
    {
        retval = new patchDef2_t;
        retval->cols = cols;
        retval->rows = rows;
        retval->points = new patchDef2point_t[cols*rows];
        memset(retval->points, 0, sizeof(patchDef2point_t)*cols*rows);
    }
    return retval;
}

void destroyPatchDef2(patchDef2_t *pd2)
{
    if(pd2 && pd2->points) delete[] pd2->points;
    delete pd2;
}

void getBezierCurve(vec3_t bp[3], int numpoints, vec3_t*dest)
{
    float dt = 1.0f / (float)(numpoints-1);
    for(int i=0; i < numpoints; i++)
    {
        float t = i * dt;
        float a = 1 - t;
        float asquared = a * a;
        float tsquared = t * t;
        float twota = 2 * t * a;
        dest[i][0] = asquared*bp[0][0] + twota*bp[1][0] + tsquared*bp[2][0];
        dest[i][1] = asquared*bp[0][1] + twota*bp[1][1] + tsquared*bp[2][1];
        dest[i][2] = asquared*bp[0][2] + twota*bp[1][2] + tsquared*bp[2][2];
    }
}

#define PD2SUBDIV 6
void glRenderPatchDef2(patchDef2_t *pd)
{
    if(!pd)
        return;
    int bpts = 3+2*PD2SUBDIV;

    glColor3ubv((GLubyte*) &set.color_brushoutline);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    vec3_t *pts = new vec3_t[pd->rows * bpts];
    int num_ren_pts = (pd->rows-1) * bpts * 2;
    vec3_t *vertex_pts = new vec3_t[num_ren_pts];	//quadstrip data
    vec3_t *normal_pts = new vec3_t[num_ren_pts];	//quadstrip data
    glDisable(GL_CULL_FACE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    for(int j=0; j < pd->cols-1; j+=2)
    {
        //which set to build
        int start = j * pd->rows;

        //get curves
        for(int i=0; i < pd->rows; i++)
        {
            vec3_t bp[3];
            VectorCopy(pd->points[start + i].xyz, bp[0]);
            VectorCopy(pd->points[start + i+pd->rows].xyz, bp[1]);
            VectorCopy(pd->points[start + i+2*pd->rows].xyz, bp[2]);
            getBezierCurve(bp, bpts, &pts[i*bpts]);
        }

        //construct linear data for quadstrip
        for(int i=0; i < pd->rows-1; i++)
        {
            int s = bpts * i;
            vec3_t *p = vertex_pts;
            vec3_t *n = normal_pts;
            for(int k=0; k<bpts; k++)
            {
                VectorCopy(pts[s + k], *p++);
                VectorCopy(pts[s + k + bpts], *p++);
            }
            for(int k=0; k<bpts-1; k++)
            {
                vec3_t normal;
                VectorCalcNormal(pts[s + k],pts[s + k+1],pts[s + k + bpts],normal);
                VectorCopy(normal, *n++);
                VectorCopy(normal, *n++);
                VectorCopy(normal, *n++);
                VectorCopy(normal, *n++);
            }
            //render boxes
            //lines test
            glShadingOff();
            glVertexPointer(3, GL_FLOAT, 0, vertex_pts);
            glNormalPointer(GL_FLOAT, 0, normal_pts);
            glDrawArrays(GL_QUAD_STRIP, 0, bpts * 2);
            glShadingOn();
            //solid test
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glColor3ub(255,128,0);
            glVertexPointer(3, GL_FLOAT, 0, vertex_pts);
            glNormalPointer(GL_FLOAT, 0, normal_pts);
            //	glTexCoordPointer(2, GL_FLOAT, 0,tex_pts);//TEST
            glDrawArrays(GL_QUAD_STRIP, 0, bpts * 2);
            glColor3ubv((GLubyte*) &set.color_brushoutline);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        }

    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    delete[] normal_pts;
    delete[] vertex_pts;
    delete[] pts;
}

void beztest()
{
    patchDef2_t *pd = createPatchDef2(5,3);

    if(!pd)
    {
        syserror(const_cast<char *> ("failed to make pd2!"));
        return;
    }

    float crap[] =
    {
        120,256.000031f, 60,  0,0,
        72, 256.000031f, 60,  0,0,
        24 ,256.000031f, 60,  0,0,

        120, 256.000031f, 172,  0,0,
        72 ,256.000031f, 172,  0,0,
        24, 256.000031f, 172,  0,0,

        120 ,95.999992f, 172,  0,0,
        72, 95.999992f, 172,  0,0,
        24, 95.999992f, 172,  0,0,

        120, -64.000015f, 172,  0,0,
        72, -64.000015f, 172,  0,0,
        24, -64.000015f, 172,  0,0,

        120, -64.000015f, 60,  0,0,
        72, -64.000015f ,60,  0,0,
        24, -64.000015f, 60,  0,0,
    };
    memcpy(pd->points, crap, sizeof(crap));
    glRenderPatchDef2(pd);

    float crap2[] =
    {
        120, -64.000015f ,60 ,  0,0,
        120, -64.000015f ,172 ,  0,0,
        120, -64.000015f ,172,  0,0,
        120, -64.000015f ,172 ,  0,0,
        120, -64.000015f ,172 ,  0,0,
        120, -64.000015f ,172,  0,0,
        120, 95.999992f, 172 ,  0,0,
        120, 95.999992f, 172 ,  0,0,
        120, 95.999992f, 172,  0,0,
        120, 256.000031f, 172,  0,0,
        120, 256.000031f, 172 ,  0,0,
        120, 256.000031f, 172,  0,0,
        120, 256.000031f, 60 ,  0,0,
        120, 256.000031f, 172 ,  0,0,
        120, 256.000031f, 172,  0,0,
    };
    memcpy(pd->points, crap2, sizeof(crap2));
    glRenderPatchDef2(pd);

    float crap3[] =
    {
        24, -64.000015f, 172,  0,0,
        24, -64.000015f, 172 ,  0,0,
        24, -64.000015f, 60,  0,0,
        24, -64.000015f, 172 ,  0,0,
        24, -64.000015f, 172,  0,0,
        24, -64.000015f, 172,  0,0,
        24, 95.999992f, 172,  0,0,
        24, 95.999992f, 172,  0,0,
        24, 95.999992f, 172,  0,0,
        24, 256.000031f, 172,  0,0,
        24, 256.000031f, 172,  0,0,
        24, 256.000031f, 172 ,  0,0,
        24, 256.000031f, 172,  0,0,
        24, 256.000031f, 172,  0,0,
        24, 256.000031f, 60 ,  0,0,
    };
    memcpy(pd->points, crap3, sizeof(crap3));
    glRenderPatchDef2(pd);

    float crap4[] =
    {
        0, 0, 0,  0,0,
        0, 64, 0,  0,0,
        64, 64, 0,  0,0,

        0, 0, 128,  0,0,
        0, 64, 128,  0,0,
        64, 64, 128,  0,0,

        0, 0, 200,  0,0,
        0, 0, 200,  0,0,
        0, 64, 200,  0,0,

    };
    pd->cols = 3;

    memcpy(pd->points, crap4, sizeof(crap4));

    glRenderPatchDef2(pd);
    destroyPatchDef2(pd);
}








