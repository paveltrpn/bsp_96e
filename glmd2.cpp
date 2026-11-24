#include "global.h"

static vec5_t vert_list[64];

// RENDER THE Q2 MODEL
void glRenderModelQ2(dmdl_t *mdl, dskindata_t *skinTex, int selected, float Xrot, float Yrot, float Zrot, int framenum, int Xoffs, int Yoffs, float Zoffs, int skinNumber)
{
    float       ratio = 0.5;
    float       skin_interp = 0.0f;

    int         i, j, nextframe;
    float       z,s,t;
    float       Xcos,Ycos,Zcos, Xsin,Ysin,Zsin;
    float       dest_x, dest_y;
    vec3_t      p, p1, p2, rp;

    float       *v;
    vec5_t      *vtx;
    daliasframe_t *frameinfo1,*frameinfo2;
    int         *command;
    int         num_verts,vert_index,verts_processed;
    int         command_type;

    // set texture stuff...
    t_width  = skinTex->skinwidth;
    t_height = skinTex->skinheight;

    t_data = (unsigned char*)skinTex->data;

    if(mdl->num_skins)
    {
        if(!mdl->gltex)
        {
            glGenTextures(1,(GLuint*) &mdl->gltex);
            glBindTexture(GL_TEXTURE_2D, mdl->gltex);

            BYTE *glbits = new BYTE[t_width * t_height * 3];
            BYTE *gp = glbits;

            for(i = 0; i < t_width * t_height; i++)
            {
                int pal = t_data[i];
                *gp++ = set.pal[PALSTEP*pal + 0];
                *gp++ = set.pal[PALSTEP*pal + 1];
                *gp++ = set.pal[PALSTEP*pal + 2];
            }
            glAddTexture(mdl->gltex, t_width, t_height, (GLuint*) glbits);
            delete [] glbits;
        }

        glBindTexture(GL_TEXTURE_2D, mdl->gltex);
    }

    //only need to calculate these rotation sins and cosines once per frame
    Xcos = (float)cos(DEG2RAD(Xrot));
    Xsin = (float)sin(DEG2RAD(Xrot));

    Ycos = (float)cos(DEG2RAD(Yrot));
    Ysin = (float)sin(DEG2RAD(Yrot));

    Zcos = (float)cos(DEG2RAD(Zrot));
    Zsin = (float)sin(DEG2RAD(Zrot));

    //keep the frame value valid
    framenum     = framenum % mdl->numframes;
    nextframe = (framenum+1) % mdl->numframes;

    frameinfo1 = (daliasframe_t *)((char *)mdl->frames + mdl->framesize*framenum);
    frameinfo2 = (daliasframe_t *)((char *)mdl->frames + mdl->framesize*nextframe);

    verts_processed = 0;
    command = mdl->glcmds;

    while (*command)
    {

        if (*command > 0)
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

        for (i=0, vtx=vert_list; i<num_verts; i++, vtx++, verts_processed++)
        {
            v = (float *)vtx;
            //grab the floating point s and t
            s = *((float *)command);
            command++;
            t = *((float *)command);
            command++;

            v[3] = s * mdl->s_scale / t_width;
            v[4] = t * mdl->t_scale / t_height;

            //grab the vertex index
            vert_index = *command;
            command++;

            //grab the 2 vertices to interpolate between
            for (j = 0; j < 3; j++)
            {
                p1[j] = ((frameinfo1->verts[vert_index].v[j] * frameinfo1->scale[j]) + frameinfo1->origin[j]);
                p2[j] = ((frameinfo2->verts[vert_index].v[j] * frameinfo2->scale[j]) + frameinfo2->origin[j]);

                p[j] = p1[j] + (p2[j] - p1[j])*ratio;
            }

            //transform the point
            XRotatePoint(p,rp,Xcos,Xsin);
            YRotatePoint(rp,p,Ycos,Ysin);
            ZRotatePoint(p,rp,Zcos,Zsin);

            //setup the point
            z = rp[2] + Zoffs;

            v[0] = rp[0] + Xoffs;
            v[1] = rp[1] + Yoffs;
            v[2] = z;

            if (skin_interp != 0.f)
            {
                dest_x = (float)mdl->skinwidth  * s + mdl->skin_screen_offset_x;
                dest_y = (float)mdl->skinheight * t + mdl->skin_screen_offset_y;

                v[0] += (dest_x - v[0]) * skin_interp;
                v[1] += (dest_y - v[1]) * skin_interp;
                v[2] += (1.0f - z)*skin_interp;
            }
        }
        vec3_t normal;

        switch (command_type)
        {
        case 0:
            //tristrip
            VectorCalcNormal(vert_list[0],vert_list[1],vert_list[2], normal);
            glBegin(GL_TRIANGLE_STRIP);
            glNormal3fv(normal);
            glTexCoord2fv(&vert_list[0][3]);
            glVertex3fv(vert_list[0]);
            glTexCoord2fv(&vert_list[1][3]);
            glVertex3fv(vert_list[1]);
            glTexCoord2fv(&vert_list[2][3]);
            glVertex3fv(vert_list[2]);

            for (i=3; i<num_verts; i++)
            {
                if(i & 1)
                {
                    //calc normal normal
                    VectorCalcNormal(vert_list[i-0],vert_list[i-1],vert_list[i-2], normal);
                }
                else
                {
                    //calc normal reverse
                    VectorCalcNormal(vert_list[i-2],vert_list[i-1],vert_list[i-0], normal);
                }
                glNormal3fv(normal);
                glTexCoord2fv(&vert_list[i][3]);
                glVertex3fv(vert_list[i]);
            }
            glEnd();
            break;
        case 1:
            //trifan
            VectorCalcNormal(vert_list[0],vert_list[1],vert_list[2], normal);

            glBegin(GL_TRIANGLE_FAN);
            glNormal3fv(normal);
            glTexCoord2fv(&vert_list[0][3]);
            glVertex3fv(vert_list[0]);
            glTexCoord2fv(&vert_list[1][3]);
            glVertex3fv(vert_list[1]);
            glTexCoord2fv(&vert_list[2][3]);
            glVertex3fv(vert_list[2]);

            for (i=3; i<num_verts; i++)
            {
                VectorCalcNormal(vert_list[0],vert_list[i-1],vert_list[i], normal);
                glNormal3fv(normal);
                glTexCoord2fv(&vert_list[i][3]);
                glVertex3fv(vert_list[i]);
            }
            glEnd();
            break;
        }
    }
}


void glCreateQ2ModelList(dmdl_t *mdl, dskindata_t *skinTex)
{
    float ratio = 0.5;
    float skin_interp = 0.0f;

    float       z,s,t;
    float       dest_x, dest_y;
    vec3_t      p1;
    vec3_t      p2;
    vec3_t      rp;

    float       *v;
    vec5_t      *vtx;
    daliasframe_t *frameinfo1,*frameinfo2;
    int         *command;
    int         i, num_verts,vert_index,verts_processed;
    int         command_type;

    // set texture stuff...
    t_width  = skinTex->skinwidth;
    t_height = skinTex->skinheight;

    t_data = (unsigned char*)skinTex->data;

    //set display list num
    if(mdl->gl_list)
        glDeleteLists(mdl->gl_list, 1);
    mdl->gl_list = glGenLists(1);

    glNewList(mdl->gl_list, GL_COMPILE);

    if(mdl->num_skins)
    {
        if(!mdl->gltex)
        {
            glGenTextures(1,(GLuint*) &mdl->gltex);
            glBindTexture(GL_TEXTURE_2D, mdl->gltex);

            BYTE *glbits = new BYTE[t_width * t_height * 3];
            BYTE *gp = glbits;
            for(int i = 0; i < t_width * t_height; i++)
            {
                int pal = t_data[i];
                *gp++ = set.pal[PALSTEP*pal + 0];
                *gp++ = set.pal[PALSTEP*pal + 1];
                *gp++ = set.pal[PALSTEP*pal + 2];
            }
            glAddTexture(mdl->gltex, t_width, t_height, (GLuint*) glbits);
            delete [] glbits;
        }
        glBindTexture(GL_TEXTURE_2D, mdl->gltex);
    }

    //keep the frame value valid
    frameinfo1 = (daliasframe_t *)((char *)mdl->frames);
    frameinfo2 = (daliasframe_t *)((char *)mdl->frames + mdl->framesize);

    verts_processed = 0;
    command = mdl->glcmds;

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

        for (i=0, vtx=vert_list; i<num_verts; i++, vtx++, verts_processed++)
        {
            v = (float *)vtx;
            //grab the floating point s and t
            s = *((float *)command);
            command++;
            t = *((float *)command);
            command++;

            v[3] = s * mdl->s_scale / t_width;
            v[4] = t * mdl->t_scale / t_height;

            //grab the vertex index
            vert_index = *command;
            command++;

            //grab the 2 vertices to interpolate between
            for (int j = 0; j < 3; j++)
            {
                p1[j] = ((frameinfo1->verts[vert_index].v[j] * frameinfo1->scale[j]) + frameinfo1->origin[j]);
                p2[j] = ((frameinfo2->verts[vert_index].v[j] * frameinfo2->scale[j]) + frameinfo2->origin[j]);

                rp[j] = p1[j] + (p2[j] - p1[j])*ratio;
            }

            //setup the point
            z = rp[2];

            v[0] = rp[0];
            v[1] = rp[1];
            v[2] = z;

            if (skin_interp != 0.f)
            {
                dest_x = (float)mdl->skinwidth  * s + mdl->skin_screen_offset_x;
                dest_y = (float)mdl->skinheight * t + mdl->skin_screen_offset_y;

                v[0] += (dest_x - v[0]) * skin_interp;
                v[1] += (dest_y - v[1]) * skin_interp;
                v[2] += (1.0f - z)*skin_interp;
            }
        }
        vec3_t normal;

        switch (command_type)
        {
        case 0:
            //tristrip
            VectorCalcNormal(vert_list[0],vert_list[1],vert_list[2], normal);
            glBegin(GL_TRIANGLE_STRIP);
            glNormal3fv(normal);
            glTexCoord2fv(&vert_list[0][3]);
            glVertex3fv(vert_list[0]);
            glTexCoord2fv(&vert_list[1][3]);
            glVertex3fv(vert_list[1]);
            glTexCoord2fv(&vert_list[2][3]);
            glVertex3fv(vert_list[2]);

            for (int i=3; i<num_verts; i++)
            {
                if(i & 1)
                {
                    //calc normal normal
                    VectorCalcNormal(vert_list[i-0],vert_list[i-1],vert_list[i-2], normal);
                }
                else
                {
                    //calc normal reverse
                    VectorCalcNormal(vert_list[i-2],vert_list[i-1],vert_list[i-0], normal);
                }

                glNormal3fv(normal);
                glTexCoord2fv(&vert_list[i][3]);
                glVertex3fv(vert_list[i]);
            }
            glEnd();
            break;
        case 1:
            //trifan
            VectorCalcNormal(vert_list[0],vert_list[1],vert_list[2], normal);

            glBegin(GL_TRIANGLE_FAN);
            glNormal3fv(normal);
            glTexCoord2fv(&vert_list[0][3]);
            glVertex3fv(vert_list[0]);
            glTexCoord2fv(&vert_list[1][3]);
            glVertex3fv(vert_list[1]);
            glTexCoord2fv(&vert_list[2][3]);
            glVertex3fv(vert_list[2]);

            for (int i=3; i<num_verts; i++)
            {
                VectorCalcNormal(vert_list[0],vert_list[i-1],vert_list[i], normal);
                glNormal3fv(normal);
                glTexCoord2fv(&vert_list[i][3]);
                glVertex3fv(vert_list[i]);
            }
            glEnd();
            break;
        }
    }
    glEndList();
}
