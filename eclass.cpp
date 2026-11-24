#include "global.h"

void EntityError(char *title, char *fmt, ...);

int sort_entities = 1;
int anyErrors = 0;

EntityClassList *entity_classes_i;


/*
the classname, color triple, and bounding box are parsed out of comments
A ? size means take the exact brush size.

/*QUAKED <classname> (0 0 0) ?
/*QUAKED <classname> (0 0 0) (-8 -8 -8) (8 8 8)

Flag names can follow the size description:

/*QUAKED func_door (0 .5 .8) ? START_OPEN STONE_SOUND DOOR_DONT_LINK GOLD_KEY SILVER_KEY
*/

char	*debugname;

char *remove_path(char *src)
{
    char *ret = src;

    while (*src)
    {
        if ((*src=='/') || (*src=='\\'))
            ret=src+1;
        src++;
    }
    return ret;
}

void dos_path(char *src)
{
    while (*src)
    {
        if (*src=='/') *src = '\\';
        src++;
    }
}

char *extract_path(char *src)
{
    char *ret = remove_path(src);
    *ret = 0;

    return src;
}

/*
Used to remove any directories in front of the filename
/ and \ are treated as the same thing
Ex: /progs\blah\misc/a.html would return a.html
*/
void removedirs(char *basestr,char *newstr)
{
    int i;
    int modify;

    modify=-1;
    for (i=strlen(basestr)-1; i>=0; i--)
    {
        if ((basestr[i]=='/')||(basestr[i]=='\\'))
        {
            modify=i+1;
            break;
        }
    }
    if (modify==-1)
    {
        strcpy(newstr,basestr);
        return;
    }
    for (i=modify; i<(int)strlen(basestr); i++)
    {
        newstr[i-modify]=basestr[i];
    }
    newstr[i-modify]=0;
}

unsigned char BestColor(unsigned char *pal, int r, int g, int b, int start, int stop)
{
    int i;
    int diff, bestdiff;
    unsigned char bestindex;
    int dr, dg, db;
    int stepper = 3;

    bestdiff = 2*(r*r + g*g + b*b);
    bestindex = 0;

    for (i = start ; i <= stop; i++)
    {
        dr = pal[stepper*i+0] - r;
        dg = pal[stepper*i+1] - g;
        db = pal[stepper*i+2] - b;

        diff = ((dr*dr)+(dg*dg)+(db*db));

        if (diff <= bestdiff)
        {
            if (!abs(diff))
                return (unsigned char)i;

            bestdiff = diff;
            bestindex = (unsigned char)i;
        }
    }
    return bestindex;
}

EntityClass::EntityClass()
{
    name = NULL;
    esize = esize_fixed;
    memset(&mins,0,sizeof(vec3_t));
    memset(&maxs,0,sizeof(vec3_t));
    color = RGB(255,255,255);
    comments = NULL;
    for (int i = 0; i< MAX_FLAGS; i++)
        memset(flagnames[i],0,32);

    if (set.game_mode == 2)
        mdlType = 1;
    else
        mdlType = 0;
    mdl = NULL;
    mdlQ2 = NULL;
    curframe = 0;
    maxframe = 0;
    curskin = 0;
    maxskin = 0;
    showMdl = true;
    // Q2 stuff
    read_from_pak   = false;
    skinTextureData = NULL;
}

EntityClass::~EntityClass()
{
    delete [] name;
    name = 0;
    delete [] comments;
    comments = 0;

    if (mdl)
    {
        FreeModel(mdl);
        delete mdl;
    }
    if (mdlQ2)
    {
        FreeModelQ2(mdlQ2);
        delete mdlQ2;
    }
    if (skinTextureData)
    {
        if (skinTextureData->data)
            delete [] skinTextureData->data;
        skinTextureData->data = 0;
        delete skinTextureData;
    }
}

void EntityClass::InitModelTexture(EntityClass *e, dmdl_t *mdl, char *pakfile, int skin_num, char *skin_fname)
{
    unsigned char temppal[768];
    unsigned char *texdata;
    unsigned char *oldmap;
    unsigned char cur_byte;

    int   i,j,run_len;
    FILE  *f;
    int   tw,th;
    short w,h;
    char  *fname;
    int    offset,size;

    if (!skin_fname && (skin_num >= mdl->num_skins) || (skin_num < 0))
        return;

    if (skin_fname)
        fname = skin_fname;
    else
        fname = mdl->skinnames[skin_num];

    if (!read_from_pak)
    {
        //first try regular fname
        f = fopen(fname,"rb");
        if (!f)
        {
            //search the current directory
            char temp[MAX_PATH];
            strcpy(temp, pakfile);
            char c = pakfile[strlen(pakfile)-1];
            if(c!='\\' && c!='/')
                strcat(temp, "\\");
            strcat(temp, fname);
            for(char*p=temp; *p; p++)
                if(*p=='/') *p='\\';

            //fname = remove_path(fname);
            f = fopen(temp,"rb");
            if (!f)
            {
                //TODO!!! is this a bug? why is this "skin_fname" and not "!skin_fname"
                if (skin_fname || !skin_num)
                    EntityError(const_cast<char *> ("Load Q2 Model Texture"),const_cast<char *> ("Unable to read model skin, pak %s, skin %s"),pakfile,skin_fname);

                return;
            }
        }

        offset = 0;
        fseek(f,0,SEEK_END);
        size = ftell(f);
        fseek(f,0,SEEK_SET);
    }
    else
    {
        f = FindInPak(pakfile,fname,&size,&offset);
    }

    if (!f)
    {
        EntityError(const_cast<char *> ("Load Q2 Model Texture"),const_cast<char *> ("Unable to open/read model skin, pak %s, skin %s"),pakfile,fname);
        return;
    };

    //PCX reading stuff
    fseek(f,offset+8,SEEK_SET);
    fread(&w,sizeof(short),1,f);
    fread(&h,sizeof(short),1,f);

    w++;
    h++;

    tw = w;
    th = h;

    //allocate space for the pcx to be read into
    oldmap = new unsigned char[tw*th];
    fseek(f,offset+128,SEEK_SET);
    j = 0;

    //.pcx decoding loop type thing
    while(j < tw*th)
    {
        cur_byte = (unsigned char)fgetc(f);
        if ((cur_byte & 0xC0) == 0xC0)
        {
            run_len = cur_byte & 0x3F;
            cur_byte = (unsigned char)fgetc(f);
            for(; (run_len>0) && (j<tw*th); run_len--,j++)
                oldmap[j] = cur_byte;
        }
        else
        {
            oldmap[j] = cur_byte;
            j++;
        }
    } // while

    //read the pcx palette
    fseek(f,offset+size-768,SEEK_SET);
    fread(temppal,768,1,f);
    fclose(f);

    //Pick the closest compatible texture size

    bool bad = false;

    if (tw>128)     tw=256;
    else if (tw>64)  tw=128;
    else if (tw>32)  tw=64;
    else if (tw>16)  tw=32;
    else if (tw>8)   tw=16;
    else if (tw>4)   tw=8;
    else if (tw>2)   tw=4;
    else if (tw>1)   tw=2;
    else bad = true;

    if (th>128)    th=256;
    else if (th>64) th=128;
    else if (th>32) th=64;
    else if (th>16) th=32;
    else if (th>8)  th=16;
    else if (th>4)  th=8;
    else if (th>2)  th=4;
    else if (th>1)  th=2;
    else bad = true;

    if (bad)
    {
        delete [] oldmap;
        EntityError(const_cast<char *> ("Load Q2 Model Texture"),const_cast<char *> ("Bad dimension, not power of 2 [%ix%i]"),tw,th);
        return;
    }

    //allocate memory for the scaled texture
    texdata = new unsigned char[tw*th];

    // map temppal to set.pal;
    unsigned char xlat[256];
    for (i = 0; i < 256; i++)
    {
        xlat[i] = BestColor(set.pal, temppal[3*i+0], temppal[3*i+1], temppal[3*i+2], 0, 254);
    }

    //scale the old texture.
    //scaling formula. pretty simple. also pretty slow. =)
    for (i=0; i<th; i++)
    {
        for(j=0; j<tw; j++)
        {
            texdata[i*tw + j] = xlat
                                [
                                    oldmap[ (int)((float)(j*w)/(float)tw) + (int)((float)(i*h)/(float)th)*w ]
                                ];
        }
    };

    //store the scaled width and height into the model structure
    if (tw>th)
    {
        mdl->s_scale = 256.0;
        mdl->t_scale = 256.0f * (float)th/(float)tw;
    }
    else
    {
        mdl->t_scale = 256.0;
        mdl->s_scale = 256.0f * (float)tw/(float)th;
    }

    e->skinTextureData = new dskindata_t;
    memset(e->skinTextureData,0,sizeof(dskindata_t));

    e->skinTextureData->data       = texdata;
    e->skinTextureData->skinwidth  = tw;
    e->skinTextureData->skinheight = th;

    // FIXME: set.sinBsp reallocation...
    delete [] oldmap;
}

dmdl_t *EntityClass::LoadMDLFromQ2MDL(char *fname)
{
    char *title = const_cast<char *> ("Load Q2 Model from file...");

    if (!*fname)
    {
        EntityError(title,const_cast<char *> ("Invalid model filename [%s]"),fname);
        return NULL;
    }

    FILE *f = fopen(fname,"rb");
    if (f == NULL)
    {
        EntityError(title,const_cast<char *> ("Unable to open model file [%s]"),fname);
        return NULL;
    }

    dmdl_t * mdl = new dmdl_t;
    memset(mdl,0,sizeof(dmdl_t));

    mdl_loader(f,mdl,0);
    fclose(f);

    read_from_pak = 0;
    return(mdl);
}

dmdl_t *EntityClass::LoadMDLFromQ2Pak(char *mdlname, char *pakname)
{
    FILE     *f;
    dmdl_t   *mdl;
    int      size, offset;

    f = FindInPak(pakname,mdlname,&size,&offset);
    if (!f)
        return LoadMDLFromQ2MDL(mdlname);

    mdl = new dmdl_t;
    memset(mdl,0,sizeof(dmdl_t));
    mdl_loader(f,mdl,offset);
    fclose(f);

    read_from_pak = 1;

    return(mdl);
}

void EntityError(char *title, char *fmt, ...)
{
    if (anyErrors) return;
    anyErrors = 1;

    va_list argptr;
    va_start(argptr,fmt);
    int len = _vscprintf(fmt, argptr) + 1;
    if(len <= 1) return;
    char *str = new char[len];
    _vsnprintf (str,len,fmt,argptr);
    va_end (argptr);

    splash->ShowWindow(SW_HIDE);
    MessageBox (0, str, title, MB_OK | MB_ICONEXCLAMATION);
    splash->ShowWindow(SW_RESTORE);
    delete [] str;
}

mdl_t *EntityClass::LoadMDLFromPak(char *fname,char *pakname)
{
    FILE        *f;
    mdlheader_t mdlheader;
    pakheader_t pakheader;
    pakentry_t  pakentry;
    int        numentries;
    char        cmpname[64];
    int         i,j;
    int k;
    bool        FOUND;
    mdl_t       *mdl;


    //This is the code that reads a model file out of a pakfile
    //understanding it is really not important if you're just
    //concerned with 3dfx programming
    int isPak = 1;

    if (!pakname[0])
    {
        EntityError(const_cast<char *> ("Load Models"),const_cast<char *> ("Invalid pak name, check/edit %s"),set.model_file);
        return NULL;
    }

    f = fopen(pakname,"rb");
    if (!f)
    {
        EntityError(const_cast<char *> ("Load Models"),const_cast<char *> ("Unable to open pak or file\n%s, check %s"),pakname,set.model_file);
        return NULL;
    }

    int pl = strlen(pakname);
    if (pl > 4)
    {
        if (toupper((unsigned char)pakname[pl-3]) == 'M' &&
                toupper((unsigned char)pakname[pl-2]) == 'D' &&
                toupper((unsigned char)pakname[pl-1]) == 'L')
        {
            isPak = 0;
        }
    }

    if (isPak)
    {
        fread(&pakheader,sizeof(pakheader_t),1,f);
        numentries = pakheader.dirsize / (sizeof(pakentry_t));

        fseek(f,pakheader.diroffset,SEEK_SET);
        FOUND=false;
        for (i=0; i<numentries; i++)
        {
            fread(&pakentry,sizeof(pakentry_t),1,f);
            if (stristr((char *)pakentry.filename,const_cast<char *> (".mdl")))
            {
                removedirs((char *)pakentry.filename,cmpname);
                if (!stricmp(fname,cmpname))
                {
                    FOUND=true;
                    break;
                }
            }
        }

        if (!FOUND)
        {
            fclose(f);
            EntityError(const_cast<char *> ("Load Model"),const_cast<char *> ("Couldn't find model [%s] in [%s], check %s"),fname,pakname,set.model_file);
            return NULL;
        }

        fseek(f,pakentry.offset,SEEK_SET);
    }

    mdl = new mdl_t;
    memset(mdl,0,sizeof(mdl_t));

    fread(&mdlheader,sizeof(mdlheader_t),1,f);

    VectorCopy(mdlheader.origin,mdl->origin);
    VectorCopy(mdlheader.scale,mdl->scale);
    mdl->numverts   = mdlheader.numverts;
    mdl->numtris    = mdlheader.numtris;
    mdl->numskins   = mdlheader.numskins;
    mdl->numframes  = mdlheader.numframes;
    mdl->skinwidth  = mdlheader.skinwidth;
    mdl->skinheight = mdlheader.skinheight;

    /* Skin Loading */
    mdl->skins = (skin_t *) new char[mdlheader.numskins*sizeof(skin_t)];
    memset(mdl->skins,0,mdlheader.numskins*sizeof(skin_t));
    for (i=0; i<mdlheader.numskins; i++)
    {

        /* Get skin type (group or single) */
        fread(&(mdl->skins[i].type),sizeof(int),1,f);
        if (mdl->skins[i].type==0)
        {

            /* Single skin */
            mdl->skins[i].numgroupskins = 1;

            /* Init the skin memory */
            mdl->skins[i].skin    = new u_char*;
            mdl->skins[i].skin[0] = new u_char[mdlheader.skinwidth*mdlheader.skinheight];
            /* read skin data */
            fread(mdl->skins[i].skin[0],mdlheader.skinwidth*mdlheader.skinheight,1,f);

            if (set.sinBsp)
            {
                int sz;
                u_char *hold, *tbits, *tstart, *sbits;
                u_char b;
                hold = mdl->skins[i].skin[0];

                tbits = new u_char[3*mdlheader.skinwidth*mdlheader.skinheight + 3];
                tstart = tbits;
                sbits = (u_char *)mdl->skins[i].skin[0];
                sz = (mdlheader.skinwidth*mdlheader.skinheight);
                for (k = 0; k < sz; k++)
                {
                    b = *sbits++;
                    *tbits++ = set.pal[3*b+2];
                    *tbits++ = set.pal[3*b+1];
                    *tbits++ = set.pal[3*b+0];
                };
                delete [] hold;
                mdl->skins[i].skin[0] = tstart;
            }

            /* init interval crap. dunno what its for */
            mdl->skins[i].interval = new float;
            *(mdl->skins[i].interval) = 0.f;

            // CalcGLBits(mdl, mdl->skins[i].skin[0], mdlheader.skinwidth, mdlheader.skinheight);
        }
        else
        {

            /* Group skin */
            fread(&(mdl->skins[i].numgroupskins),sizeof(int),1,f);

            /* load group skin interval data */
            mdl->skins[i].interval = new float[mdl->skins[i].numgroupskins];
            for (j=0; j<mdl->skins[i].numgroupskins; j++)
            {
                fread(&(mdl->skins[i].interval[j]),sizeof(float),1,f);
            }

            /* load group skin data */
            mdl->skins[i].skin = new u_char *[mdl->skins[i].numgroupskins];

            for (j=0; j<mdl->skins[i].numgroupskins; j++)
            {
                mdl->skins[i].skin[j] = new u_char[mdlheader.skinwidth*mdlheader.skinheight];
                fread(mdl->skins[i].skin[j],mdlheader.skinwidth*mdlheader.skinheight,1,f);
                if (set.sinBsp)
                {
                    int sz;
                    u_char *hold, *tbits, *tstart, *sbits;
                    u_char b;
                    hold = mdl->skins[i].skin[j];

                    tbits = new u_char[3*mdlheader.skinwidth*mdlheader.skinheight + 3];
                    tstart = tbits;
                    sbits = (u_char *)mdl->skins[i].skin[j];
                    sz = (mdlheader.skinwidth*mdlheader.skinheight);
                    for (k = 0; k < sz; k++)
                    {
                        b = *sbits++;
                        *tbits++ = set.pal[3*b+2];
                        *tbits++ = set.pal[3*b+1];
                        *tbits++ = set.pal[3*b+0];
                    };
                    delete [] hold;
                    mdl->skins[i].skin[j] = tstart;
                };
            };
        }
    }

    /*
       Vertex loading (texture vertices, not spatial vertices)
    */
    mdl->verts = new stvert_t[mdlheader.numverts];
    fread(mdl->verts,mdlheader.numverts*sizeof(stvert_t),1,f);

    /*
       Triangle loading
    */
    mdl->triangles = new itriangle_t[mdlheader.numtris];
    fread(mdl->triangles,mdlheader.numtris*sizeof(itriangle_t),1,f);

    /*
       Frame loading (spatial vertex information)
    */
    mdl->frames = new frame_t[mdlheader.numframes];
    for (i=0; i<mdlheader.numframes; i++)
    {

        /* Read the frame type (group or single) */
        fread(&(mdl->frames[i].type),sizeof(int),1,f);

        if (mdl->frames[i].type==0)
        {

            /* Single frame */
            mdl->frames[i].numgroupframes = 1;

            /* Get frame info */
            mdl->frames[i].info = new frameinfo_t;
            fread(mdl->frames[i].info,sizeof(frameinfo_t),1,f);

            /* Get frame data */
            mdl->frames[i].data = new trivertx_t*;
            mdl->frames[i].data[0] = new trivertx_t[mdlheader.numverts];
            fread(mdl->frames[i].data[0],mdlheader.numverts*sizeof(trivertx_t),1,f);

            /* make fake "interval" data */
            mdl->frames[i].interval = new float;
            *(mdl->frames[i].interval) = 0.f;

        }
        else
        {

            /* Group frame */
            /* get group information */
            fread(&(mdl->frames[i].numgroupframes),sizeof(int),1,f);
            fread(&(mdl->frames[i].groupmin),sizeof(trivertx_t),1,f);
            fread(&(mdl->frames[i].groupmax),sizeof(trivertx_t),1,f);

            /* init necessary data memory */
            mdl->frames[i].data = (trivertx_t **)new u_char*[mdl->frames[i].numgroupframes];

            /* init necessary info memory */
            mdl->frames[i].info = new frameinfo_t[mdl->frames[i].numgroupframes];

            /* load group frame interval data */
            mdl->frames[i].interval = new float[mdl->frames[i].numgroupframes];

            for (j=0; j<mdl->frames[i].numgroupframes; j++)
            {
                fread(&(mdl->frames[i].interval[j]),sizeof(float),1,f);
            }

            for (j=0; j<mdl->frames[i].numgroupframes; j++)
            {
                /* read in the frame info */
                fread(&(mdl->frames[i].info[j]),sizeof(frameinfo_t),1,f);

                /* allocate and read in the frame data */
                mdl->frames[i].data[j] = new trivertx_t[mdlheader.numverts];
                fread(mdl->frames[i].data[j],mdlheader.numverts*sizeof(trivertx_t),1,f);
            }
        }
    }

    /* Check if everything was read ok. We should be at the end of the
       pak entry right now */

    if (isPak)
    {
        if ((ftell(f)-pakentry.offset) != pakentry.size)
        {
            fclose(f);
            EntityError(const_cast<char *> ("Load Model"),const_cast<char *> ("Error reading model [%s] from pakfile %s"),fname,pakname);
            return NULL;
        }
    }
    fclose(f);
    return(mdl);
}

void EntityClass::mdl_loader(FILE *f, dmdl_t *mdl, int offset)
{
    //assumes f is pointing right at the mdl header
    int i;
    dmdlheader_t mdlheader;

    //read the header
    fread(&mdlheader,sizeof(dmdlheader_t),1,f);

    //dont need all the info from the header, just some of it
    mdl->numframes   = mdlheader.num_frames;
    mdl->framesize   = mdlheader.framesize;
    mdl->num_glcmds  = mdlheader.num_glcmds;
    mdl->skinwidth   = mdlheader.skinwidth;
    mdl->skinheight  = mdlheader.skinheight;


    //only need to allocate room for the glcmds and the framedata
    fseek(f,offset+mdlheader.ofs_glcmds,SEEK_SET);
    mdl->glcmds = new int[mdl->num_glcmds];
    fread(mdl->glcmds,mdl->num_glcmds*sizeof(int),1,f);

    // read framedata
    fseek(f,offset+mdlheader.ofs_frames,SEEK_SET);
    mdl->frames = (daliasframe_t *) new char[mdl->framesize*mdl->numframes];
    fread(mdl->frames,mdl->framesize*mdl->numframes,1,f);

    //the 1st skin name will be used as the texture
    fseek(f,offset+mdlheader.ofs_skins,SEEK_SET);
    mdl->num_skins = mdlheader.num_skins;

    if (mdl->num_skins)
    {
        mdl->skinnames = new char *[mdl->num_skins];
        for (i=0; i<mdl->num_skins; i++)
        {
            mdl->skinnames[i] = new char[64];
            fread(mdl->skinnames[i],64,1,f);
        }
    }
    else
    {
        mdl->skinnames = 0;
    }

    mdl->skin_screen_offset_x = (float) 320 - (mdl->skinwidth/2);
    mdl->skin_screen_offset_y = (float) 240 - (mdl->skinheight/2);
}

FILE *EntityClass::FindInPath(char *path, char *filename, int *size, int *offset)
{
    char fname[256];
    FILE *f;
    strcpy(fname,path);
    dos_path(filename);
    strcat(fname,filename);

    f = fopen(fname,"rb");
    if (!f)
        return NULL;

    fseek(f,0,SEEK_END);
    *size   = ftell(f);
    fseek(f,0,SEEK_SET);
    *offset = 0;
    return f;
}

FILE *EntityClass::FindInPak(char *pakname, char *filename, int *size, int *offset)
{
    //the size / offset thing is used so that the same code
    //can be reading either from a regular file, or a file
    //contained in in a .pak file

    int          num_pak_entries;
    pakentry_t   *pak_entries;
    pakheader_t  pakheader;
    int          i,found;
    FILE         *f;

    if (pakname[0] == 0)
    {
        EntityError(const_cast<char *> ("Find file in Pak"),const_cast<char *> ("Invalid pak filename [%s]"),pakname);
        return NULL;
    }

    f = fopen(pakname,"rb");
    if (!f)
        return FindInPath(extract_path(pakname),filename,size,offset);

    fread(&pakheader,sizeof(pakheader_t),1,f);
    fseek(f,pakheader.diroffset,SEEK_SET);

    num_pak_entries = pakheader.dirsize / (sizeof(pakentry_t));
    pak_entries = new pakentry_t[num_pak_entries];
    fread(pak_entries,pakheader.dirsize,1,f);

    found = 0;

    //find filename in directory
    for (i=0; i<num_pak_entries; i++)
    {
        if (!stricmp((char *)pak_entries[i].filename, filename))
        {
            found = 1;
            break;
        }
    }

    if (!found)
    {
        //couldnt find it. return 0
        delete [] pak_entries;
        return FindInPath(extract_path(pakname), filename, size, offset);
    }
    fseek(f, pak_entries[i].offset, SEEK_SET);

    *size   = pak_entries[i].size;
    *offset = pak_entries[i].offset;

    delete [] pak_entries;
    return f;
}

void EntityClass::FreeModel(mdl_t *mdl)
{
    int i,j;
    if (!mdl) return;
    if (mdl->skins!=NULL)
    {
        for (i=0; i<mdl->numskins; i++)
        {
            if (mdl->skins[i].skin!=NULL)
            {
                for (j=0; j<mdl->skins[i].numgroupskins; j++)
                {
                    if (mdl->skins[i].skin[j]!=NULL)
                        delete [] mdl->skins[i].skin[j];
                }
                delete [] mdl->skins[i].skin;
            }
            if (mdl->skins[i].interval!=NULL)
                delete [] mdl->skins[i].interval;
        }
        delete [] mdl->skins;
    }

    if (mdl->verts!=NULL) delete [] mdl->verts;

    if (mdl->triangles!=NULL) delete [] mdl->triangles;

    if (mdl->frames!=NULL)
    {
        for (i=0; i<mdl->numframes; i++)
        {
            if (mdl->frames[i].data!=NULL)
            {
                for (j=0; j<mdl->frames[i].numgroupframes; j++)
                {
                    if (mdl->frames[i].data[j]!=NULL)
                        delete [] mdl->frames[i].data[j];
                }
                delete mdl->frames[i].data;
            }
            if (mdl->frames[i].info!=NULL) delete [] mdl->frames[i].info;
            if (mdl->frames[i].interval!=NULL) delete [] mdl->frames[i].interval;
        }
        delete [] mdl->frames;
    }
}

void EntityClass::FreeModelQ2(dmdl_t *mdl)
{
    int i;

    if (!mdl) return;

    delete [] mdl->frames;
    mdl->frames = 0;

    delete [] mdl->glcmds;
    mdl->glcmds = 0;

    if (mdl->skinnames)
    {
        for (i=0; i<mdl->num_skins; i++)
        {
            if (mdl->skinnames[i])
            {
                delete [] mdl->skinnames[i];
                mdl->skinnames[i] = 0;
            }
        }
        delete [] mdl->skinnames;
        mdl->skinnames = 0;
    }
}

void EntityClass::initFromText(char *text)
{
    char	*t;
    int		len;
    int		r, i;
    char	parms[256], *p;

    text += sizeof("/*QUAKED ") - 1;

    // grab the name
    text = COM_Parse (text);
    name = new char[strlen(com_token)+2];
    strcpy (name, com_token);
    debugname = name;

    // grab the color
    vec3_t tmpcolor;
    r = sscanf (text," (%f %f %f)", &tmpcolor[0], &tmpcolor[1], &tmpcolor[2]);
    if (r != 3)
    {
        EntityError(const_cast<char *> ("Load Models"),const_cast<char *> ("Couldn't read colors for entity: %s"),name);
        return;
    }
    color = RGB(tmpcolor[0]*255, tmpcolor[1]*255, tmpcolor[2]*255);

    while (*text != ')')
    {
        if (!*text)
            return;
        text++;
    }
    text++;

    // get the size
    text = COM_Parse (text);
    if (com_token[0] == '(')
    {
        // parse the size as two vectors
        esize = esize_fixed;
        r = sscanf (text,"%f %f %f) (%f %f %f)", &mins[0], &mins[1], &mins[2], &maxs[0], &maxs[1], &maxs[2]);
        if (r != 6)
        {
            EntityError(const_cast<char *> ("Load Entity Information"),const_cast<char *> ("Couldn't read bounds for entity: %s"),name);
            return;
        };

        for (i=0 ; i<2 ; i++)
        {
            while (*text != ')')
            {
                if (!*text)
                    return;
                text++;
            }
            text++;
        }
    }
    else
    {
        // use the brushes
        esize = esize_model;
    }

    // get the flags
    // copy to the first line break;
    p = parms;
    while (*text && *text != '\n')
        *p++ = *text++;
    *p = 0;
    text++;

    // any remaining words are parm flags
    p = parms;
    for (i=0 ; i<8 ; i++)
    {
        p = COM_Parse (p);
        if (!p)
            break;
        strcpy (flagnames[i], com_token);
    }

    // find the length until close comment
    // and count # of unpaired /r/n 's
    int unpaired = 0;
    for (t=text ; (*t != '\0') && !(*(t) == '*' && *(t+1) == '/') ; t++)
    {
        if ((*t != '\r') && (*(t+1) == '\n'))
            unpaired++;
    };

    char *end;
    end = t;
    // copy the comment block out
    len = (int)(t-text);
    //len += unpaired + 1;
    comments = new char[len+unpaired+5];
    t = text;
    char *com;
    com = comments;
    while (t <= end)
    {
        if (*t == '\n')
        {
            if ((t - text) < 1)
            {
                *com++ = '\r';
            }
            else if (*(t-1) != '\r')
            {
                *com++ = '\r';
            };
        };
        *com++ = *t++;
    };
    if ((com - comments) >= 1)
    {
        *(com - 1) = 0;
    }
    else
    {
        *com = 0;
    };

    /*
    // copy the comment block out
    len = (int)(t-text);
    comments = new char[len+1];
    memcpy (comments, text, len);
    comments[len] = 0;
    */
}

esize_t EntityClass::getesize()
{
    return esize;
}

char *EntityClass::getclassname()
{
    return name;
}

float *EntityClass::getmins()
{
    return mins;
}

float *EntityClass::getmaxs()
{
    return maxs;
}

COLORREF EntityClass::drawColor()
{
    return color;
}

char *EntityClass::getcomments()
{
    return comments;
}


char *EntityClass::getflagName(unsigned flagnum)
{
    if (flagnum >= MAX_FLAGS)
    {
        syserror(const_cast<char *> ("EntityClass flagName: bad number"));
        return 0;
    }
    return flagnames[flagnum];
}

//===========================================================================

EntityClassList::EntityClassList()
{
    nullclass = NULL;
    source_path = NULL;
    numElements = 0;
    objects = new EntityClass *[MAX_CLASSES];
    memset(objects,0,MAX_CLASSES*sizeof(EntityClass *));
}

EntityClassList::~EntityClassList()
{
    source_path = NULL;

    for (int i=0 ; i < numElements; i++)
    {
        if (!objects[i])
            continue;

        delete objects[i];
    }
    delete[] objects;
    numElements = 0;
    delete nullclass;
}

EntityClass *EntityClassList::objectAt(int num)
{
    if (num < 0 || num >= numElements)
        return NULL;
    return objects[num];
}

/*
=================
insertEC:
=================
*/
void EntityClassList::insertEC(EntityClass *ec)
{
    if (numElements == MAX_CLASSES)
    {
        syserror(const_cast<char *> ("EntityClass->insert, too many classes"));
        delete ec;
        return;
    }

    // leave in file order...
    if (!sort_entities)
    {
        objects[numElements++] = ec;
        return;
    }

    int i,j, done = 0;
    // find a slot, move the rest down one, insert the entity...
    for (i = 0; (i < numElements) && !done; i++)
    {
        if (strcmpi(objects[i]->name,ec->name) <= 0)
            continue;

        done = 1;
        for (j = numElements; j > i; j--)
        {
            objects[j] = objects[j-1];
        };
        numElements++;
        objects[i] = ec;
        break;
    }
    if (!done)   // nothing yet...
    {
        objects[numElements++] = ec;
    }
}

/*
=================
scanFile
=================
*/
void EntityClassList::scanFile(char *filename)
{
    int size;
    unsigned char *data;
    EntityClass	*cl;

    _finddata_t ffb;
    memset(&ffb,0,sizeof(ffb));
    char dirname[256];
    strncpy(dirname,filename,sizeof(dirname));
    dirname[sizeof(dirname)-1] = 0;

    int ff = _findfirst(dirname,&ffb);
    int nomorefiles = ff < 0 ? 1 : 0;

    while (!nomorefiles)
    {
        if ((ffb.attrib & _A_SUBDIR) || (ffb.name[0] == '.'))
        {
            nomorefiles = _findnext(ff, &ffb);
            continue;
        }

        Path file(const_cast<char *> ("%s/%s"),source_path,ffb.name);
        if (!file.Exists())
        {
            // just skip
            nomorefiles = _findnext(ff, &ffb);
            continue;
        }

        data = file_get_contents(file, &size);
        if(!data)
        {
            // syserror ("Error scanning %s for eclasses...",filename);
            continue;
        }

        for (int i=0 ; i<size ; i++)
        {
            if (!strncmp((char *)(data+i), "/*QUAKED",8))
            {
                cl = new EntityClass;
                cl->initFromText((char *)(data+i));
                if (cl)
                {
                    insertEC(cl);
                }
                else
                {
                    syserror (const_cast<char *> ("Error parsing: %s in %s\n"),debugname, filename);
                    break;
                }
            }
        }
        delete[] data;
        nomorefiles = _findnext(ff, &ffb);
    }
    _findclose(ff);
}

/*
=================
scanDirectory
=================
*/
void EntityClassList::scanDirectory()
{
    char fileList[256];
    char outstr[256];
    char path[256];

    strncpy(fileList,set.entity_file,sizeof(fileList));
    fileList[sizeof(fileList)-1] = 0;

    if (fileList && *fileList)
    {
        // char QCList[64][128];
        char curName[128];
        int counter;
        int nQC = CountWads(fileList);
        char *start = fileList;
        char *end;

        for (counter = 0; (counter < nQC); counter++)
        {
            // split up list of files...

            for (end = start; end && *end && *end != ';' ; end++);

            strncpy(curName,start,(int)(end-start));
            curName[(int)(end-start)] = '\0';

            start = ++end;
            ExtractFileBase(curName,outstr,false);

            sprintf(path,"%s\\%s",source_path,outstr);

            char dirname[256];
            strncpy(dirname,path,sizeof(dirname));
            dirname[sizeof(dirname)-1] = 0;

            struct _finddata_t ffb;
            memset(&ffb,0,sizeof(ffb));
            int ff = _findfirst(dirname,&ffb);
            _findclose(ff);
            if (ff < 0)
            {
                char filename[256] = "";
                char ti[256];
                sprintf(ti,"Where are .qc files:  %s?",path);

                OPENFILENAME ofn;
                memset(&ofn,0,sizeof(OPENFILENAME));
                ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
                ofn.hwndOwner = frame->hwnd;
                ofn.lpstrFile = filename;
                ofn.lpstrFilter = "Entity .qc Files (*.qc)\0*.qc\0";
                ofn.lpstrDefExt = "qc";
                ofn.nMaxFile = 256;
                ofn.lpstrInitialDir = set.map_directory;
                ofn.lpstrTitle = ti;
                ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

                if(splash) splash->ShowWindow(SW_HIDE);
                if (GetOpenFileName(&ofn))
                {
                    scanFile(filename);
                }
                if(splash) splash->ShowWindow(SW_RESTORE);
                // just skip if not Okay?
            }
            else
            {
                scanFile(path);
            }
        }
    }

    // Just skip
    if (set.no_models)
        return;

    EntityClass *o;
    int i;

    char filename[256];
    sprintf(filename,"%s\\%s", set.game_directory, set.model_file);

    unsigned char *data = file_get_contents(filename);
    if (!data)
    {
        syserror(const_cast<char *> ("Couldn't open %s, skipping"),filename);
        return;
    }
    Tokenizer script((char*)data);
    char entname[256];
    char mdlfile[256];
    char pakfile[256];
    bool visible;

    while (1)
    {
        //entity name
        if (!script.next(true))
            break;
        snprintf(entname,sizeof(entname),script.token);
        //model path
        if (!script.next(true))
            break;
        snprintf(mdlfile,sizeof(mdlfile),script.token);
        //"pak"
        if (!script.next(true))
            break;
        snprintf(pakfile,sizeof(pakfile),script.token);
        //show/hide
        if (!script.next(true))
            break;
        visible = true;
        if (!_stricmp(script.token,"hide"))
            visible = false;

        sprintf(outstr,"Loading Models:  [%s]",entname);
        splash->SetText(outstr);

        for (o=0, i=0 ; i<numElements ; i++)
        {
            if (!strcmp (entname,objects[i]->name) )
            {
                o = objects[i];
                break;
            }
        }
        if (!o)
            continue;

        o->maxframe = 0;
        o->maxskin = 0;
        if (set.game_mode == 2)
        {
            if (strcmpi(pakfile,"pak") || set.pakOk)
            {
                o->mdlType = 1;
                if (!strcmpi(pakfile,"pak"))
                    strncpy(pakfile,set.pak_file,sizeof(pakfile));

                o->mdlQ2 = o->LoadMDLFromQ2Pak(mdlfile,pakfile);
                if (o->mdlQ2)
                {
                    o->InitModelTexture(o,o->mdlQ2,pakfile,0,0);
                    if (!o->skinTextureData)
                    {
                        o->FreeModelQ2(o->mdlQ2);
                        o->mdlQ2 = NULL;
                    }
                    else
                    {
                        o->maxframe = (int)o->mdlQ2->numframes;
                        o->maxskin = (int)o->mdlQ2->num_skins;
                    }
                }
            }
        }
        else
        {
            o->mdlType = 0;
            o->mdl = o->LoadMDLFromPak(mdlfile,pakfile);
            if (o->mdl)
            {
                o->maxframe = (int)o->mdl->numframes;
                o->maxskin = (int)o->mdl->numskins;
            }
        }
        o->curframe = 0;
        o->curskin = 0;
        o->showMdl = visible;
    }

    delete[] data;
}

void EntityClassList::initForSourceDirectory(char *path)
{
    source_path = path;

    sort_entities = set.sort_entities;

    scanDirectory();
    nullclass = new EntityClass;
    char nullChars[128];
    strcpy(nullChars,"/*QUAKED UNKNOWN_CLASS (1.0 1.0 0) ?");
    nullclass->initFromText(nullChars);
}

EntityClass *EntityClassList::classForName(char *name)
{
    EntityClass *o;

    for (int i=0; i<numElements ; i++)
    {
        o = objects[i];

        if( !strcmp(name, o->getclassname()) )
            return o;
    }
    return nullclass;
}

int EntityClassList::indexOf(EntityClass *e)
{
    for (int i=0 ; i<numElements ; i++)
    {
        if (objects[i] == e)
        {
            return i;
        }
    }
    return -1;
}
