#include "global.h"
#include "lib/jpeg8/jpeglib.h"
#include "lib/png/png.h"


void glSetFogRange(float fognear, float fogfar)
{
    glFogf(GL_FOG_START, fognear);
    glFogf(GL_FOG_END, fogfar);
}

void glSetBackgroundColor(COLORREF color)
{
    glClearColor(GetRValue(color)/255.0f,GetGValue(color)/255.0f,GetBValue(color)/255.0f, 0.0f);
}
void glSetFogColor(COLORREF color)
{
    GLfloat FogCol[] = { GetRValue(color)/255.0f,GetGValue(color)/255.0f,GetBValue(color)/255.0f };
    glFogfv(GL_FOG_COLOR, FogCol);
}
void glFogOn()
{
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glHint (GL_FOG_HINT, GL_DONT_CARE);
}
void glFogOff()
{
    glDisable(GL_FOG);
}
void glLightAmbient(COLORREF color)
{
    GLfloat ambient[] = {GetRValue(color)/255.0f,GetGValue(color)/255.0f,GetBValue(color)/255.0f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
}
void glLightColor(COLORREF color)
{
    GLfloat diffuse[] = {GetRValue(color)/255.0f,GetGValue(color)/255.0f,GetBValue(color)/255.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
}

void glShadingOn()
{
    if(set.gl_shading)
        glEnable(GL_LIGHTING);
}

void glShadingOff()
{
    if(set.gl_shading)
        glDisable(GL_LIGHTING);
}

void glAddTexture(int name, int width, int height, GLuint* pic, int bits, int wrap)
{
    glBindTexture(GL_TEXTURE_2D, name);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLfloat) wrap);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLfloat) wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, set.gl_texturemode_value);

    switch (set.gl_texturemode_value)
    {
    case GL_NEAREST:
    case GL_NEAREST_MIPMAP_NEAREST:
    case GL_NEAREST_MIPMAP_LINEAR:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        break;
    case GL_LINEAR:
    case GL_LINEAR_MIPMAP_NEAREST:
    case GL_LINEAR_MIPMAP_LINEAR:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        break;
    }

    if(bits > 0)
    {
        gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, bits, GL_UNSIGNED_BYTE, pic);
    }
    else
    {
        if (cTextureAlpha)
        {
            gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pic);
        }
        else
        {
            gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB,  GL_UNSIGNED_BYTE, pic);
        }
    }
}


void glReloadBSPSettings()
{
    if(!set.glBsp) return;

    //fog
    if(set.gl_fog)
    {
        glFogOn();
        if(set.gl_fog_match_bg)
            glSetFogColor(set.color_3dbackground);
        else
            glSetFogColor(set.gl_fog_color);
        glSetFogRange((float)set.gl_fog_near, (float)set.gl_fog_far);
    }
    else
        glFogOff();

    //light
    glEnable(GL_LIGHT0);
    glLightAmbient(set.gl_ambient_color);
    glLightColor(set.gl_light_color);

    if(editWindow) editWindow->RedrawContents();
}

//===========================================================================
// skybox
//===========================================================================
char *skysuffix[] = {const_cast<char *> ("up"),
                     const_cast<char *> ("dn"),
                     const_cast<char *> ("rt"),
                     const_cast<char *> ("lf"),
                     const_cast<char *> ("ft"),
                     const_cast<char *> ("bk")
                    };
int skynames[6];

void glUnloadSkybox()
{
    if(set.gl_skynames_loaded)
    {
        for(int i=0; i<6; i++)
        {
            glDeleteTextures(1, (const GLuint*)&skynames[i]);
            skynames[i]=0;
        }
        set.gl_skynames_loaded = false;
    }
}
void glReloadSkybox()
{
    glUnloadSkybox();
    for(int i=0; i<6; i++)
    {
        skynames[i] = 0;
        if(set.gl_skybox_auto)
        {
            map* map;
            map = map_i[set.curmap];
            Entity* ent = map->findWorldSpawn();
            strcpy(set.gl_skybox_name, ent->valueForQKey(const_cast<char *> ("sky")));
            if (!strcmp(set.gl_skybox_name, ""))
                strcpy(set.gl_skybox_name, "unit1_");
        }

        Path filename(const_cast<char *> ("%s/%s*%s.*"), set.gl_skybox_path, set.gl_skybox_name, skysuffix[i]);
        WIN32_FIND_DATA ffd;
        HANDLE ff = FindFirstFile(filename, &ffd);
        if(ff != INVALID_HANDLE_VALUE)
        {
            FindClose(ff);
            Path texpath(const_cast<char *> ("%s/%s"), set.gl_skybox_path, ffd.cFileName);
            unsigned char *pic=0;
            int cx=0,cy=0;
            if(LoadImageFile(texpath, &pic, &cx,&cy,true))
            {
                glGenTextures(1,(GLuint*)&skynames[i]);
                glAddTexture(skynames[i],cx,cy,(GLuint*)pic,GL_RGBA,GL_CLAMP);
                delete[]pic;
            }
            else
                syserror(const_cast<char *> ("skybox: failed to load %s"), (char*)texpath);
        }
        else
            sysprintf(const_cast<char *> ("not found: %s\r\n"), (char*)filename);
    }
    set.gl_skynames_loaded = true;
}
void CmListSkybox()
{
    sysprintf(const_cast<char *> ("Skyboxes in directory: %s\n"), set.gl_skybox_path);
    Path filename(const_cast<char *> ("%s/*bk.*"), set.gl_skybox_path);
    WIN32_FIND_DATA ffd;
    HANDLE ff = FindFirstFile(filename, &ffd);
    if(ff != INVALID_HANDLE_VALUE)
    {
        do
        {
            char *c = stristr(ffd.cFileName, const_cast<char *> ("bk."));
            if(!c) continue;
            if(c == ffd.cFileName) continue;	// filename shouldnt be "bk.w/e", thats dumb
            *(*(c-1) == '_' ? c-1 : c) = 0;
            sysprintf(const_cast<char *> ("    %s\n"), ffd.cFileName);
        }
        while(FindNextFile(ff, &ffd));
        FindClose(ff);
    }
}

void renderskybox()
{
    static const vec2_t tex[] = { {0,0},{1,0},{1,1},{0,1} };

    static const float NB=-1024, PB=1024;
    static const float box_up[] = { NB,PB,PB, NB,NB,PB, PB,NB,PB, PB,PB,PB };
    static const float box_dn[] = { PB,PB,NB, PB,NB,NB, NB,NB,NB, NB,PB,NB };
    static const float box_rt[] = { PB,PB,PB, PB,NB,PB, PB,NB,NB, PB,PB,NB };
    static const float box_lf[] = { NB,NB,PB, NB,PB,PB, NB,PB,NB, NB,NB,NB };
    static const float box_ft[] = { PB,NB,PB, NB,NB,PB, NB,NB,NB, PB,NB,NB };
    static const float box_bk[] = {	NB,PB,PB, PB,PB,PB, PB,PB,NB, NB,PB,NB };


    glShadingOff();
    if(set.gl_fog)
        glFogOff();
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_DEPTH_TEST);

    glPushMatrix();
    map *m = map_i[set.curmap];
    glTranslatef(m->eye[m->cureye][0], m->eye[m->cureye][1], m->eye[m->cureye][2]);

    glColor3ub(255,225,255);
    glTexCoordPointer(2,GL_FLOAT,0,tex);
    glBindTexture(GL_TEXTURE_2D, skynames[0]);
    glVertexPointer(3, GL_FLOAT, 0, box_up);
    glDrawArrays(GL_QUADS, 0, 4);
    glBindTexture(GL_TEXTURE_2D, skynames[1]);
    glVertexPointer(3, GL_FLOAT, 0, box_dn);
    glDrawArrays(GL_QUADS, 0, 4);
    glBindTexture(GL_TEXTURE_2D, skynames[2]);
    glVertexPointer(3, GL_FLOAT, 0, box_rt);
    glDrawArrays(GL_QUADS, 0, 4);
    glBindTexture(GL_TEXTURE_2D, skynames[3]);
    glVertexPointer(3, GL_FLOAT, 0, box_lf);
    glDrawArrays(GL_QUADS, 0, 4);
    glBindTexture(GL_TEXTURE_2D, skynames[4]);
    glVertexPointer(3, GL_FLOAT, 0, box_ft);
    glDrawArrays(GL_QUADS, 0, 4);
    glBindTexture(GL_TEXTURE_2D, skynames[5]);
    glVertexPointer(3, GL_FLOAT, 0, box_bk);
    glDrawArrays(GL_QUADS, 0, 4);

    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_TEXTURE_2D);
    if(set.gl_fog)
        glFogOn();
    glShadingOn();
}

void glBeginAddToSkybox()
{
    glEnable(GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);

    glStencilFunc(GL_ALWAYS,1,1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glShadingOff();
    glColorMask(0,0,0,0);
}
void glEndAddToSkybox()
{
    glColorMask(1,1,1,1);
    glShadingOn();
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_STENCIL_BUFFER_BIT);
}
void glRenderSkybox()
{
    glEnable(GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 1,1);
    glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);

    renderskybox();

    glDisable(GL_STENCIL_TEST);
    glDisable(GL_STENCIL_BUFFER_BIT);
}
