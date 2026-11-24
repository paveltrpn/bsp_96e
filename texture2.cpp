#include "global.h"

texture_hash::texture_hash()
{
    memset(name,0,32);
    memset(basepath,0,32);
    memset(&pakentry,0,sizeof(pakentry_t));
    next = NULL;
    prev = NULL;
}

//************************
TColorMap::TColorMap(unsigned char *pal)
{
    float range = 2;
    int levels = 64;
    int brights = 1;	// ignore 255 (transparent)

    lookUp = new unsigned char[levels*256 + 256];
    unsigned char *lump_p = lookUp;

    char str[128];
    if (set.fake_colormap)
    {
        //flat shading
        for (int i = 0; i < 256; i++)
            lookUp[i] = i;

        for (int i = 1; i < levels; i++)
        {
            if (splash)
            {
                sprintf(str,"Generating flat colormap [%i%%]",100*i/levels);
                splash->SetText(str);
            }
            memcpy(lump_p,lookUp,256);
            lump_p += 256;
        }
    }
    else
    {
        // shaded levels
        for (int i = 0; i < levels; i++)
        {
            if (splash)
            {
                sprintf(str,"Generating shaded colormap [%i%%]",100*i/levels);
                splash->SetText(str);
            }
            float frac = range - range*(float)i/(levels-1);
            int c = 0;
            for ( ; c < 256-brights ; c++)
            {
                int red   = (int) pal[c*PALSTEP+0]*frac;
                int green = (int) pal[c*PALSTEP+1]*frac;
                int blue  = (int) pal[c*PALSTEP+2]*frac;

                *lump_p++ = texWindow->BestColor(pal, red, green, blue, 1, 254);
            }
            // transparent always same
            for ( ; c<256 ; c++)
                *lump_p++ = (unsigned char)c;
        }
    }

    if (splash)
    {
        splash->SetText(const_cast<char *> ("Done building colormap..."));
    }
}

TColorMap::~TColorMap()
{
    delete[] lookUp;
}




//TTextureWindow hash functions

int TTextureWindow::GetHashValue(char *name, char *basepath)
{
    int hashval = 0;
    int i, l;

    l = strlen(name);
    for (i = 0; i < l; i++)
        hashval += (i+1)*(int)name[i];

    l = strlen(basepath);
    for (i = 0; i < l; i++)
        hashval += (i+1)*(int)basepath[i];

    return abs(hashval) % MAX_HASH;
}

pakentry_t *TTextureWindow::SearchHash(char *name, char *basepath, texture_hash **hashTable)
{
    // v is the slot to put it in.
    int v = GetHashValue(name,basepath);

    if (!hashTable[v])
        return NULL;

    texture_hash *n = hashTable[v]; // full match needed
    while (n)
    {
        if (!strcmpi(n->name,name) && !strcmpi(n->basepath,basepath)) // found it, exit now...
            return &n->pakentry;
        n = n->next;
    }

    return NULL;
}

void TTextureWindow::AddToHash(char *name, char *basepath, texture_hash **hashTable)
{
    // v is the slot to put it in.
    int v = GetHashValue(name,basepath);

    if (hashTable[v] == NULL)
    {
        texture_hash *n;
        n = new texture_hash;
        strncpy(n->name, name, sizeof(n->name));
        strncpy(n->basepath, basepath, sizeof(n->basepath));

        hashTable[v] = n;
    }
    else
    {
        texture_hash *n,*old = 0;

        n = hashTable[v];
        while (n)
        {
            old = n;
            if (!strcmpi(n->name,name)) // found it, exit now...
                return;
            n = n->next;
        }

        if(old)
        {
            n = new texture_hash;
            strncpy(n->name, name, sizeof(n->name));
            strncpy(n->basepath, basepath, sizeof(n->basepath));

            old->next = n;
        }
    }
}

void TTextureWindow::AddToHashPak(char *name, char *basepath, pakentry_t *p, texture_hash **hashTable)
{
    // v is the slot to put it in.
    int v = GetHashValue(name,basepath);

    if (hashTable[v] == NULL)
    {
        texture_hash *n;
        n = new texture_hash;
        strncpy(n->name, name, sizeof(n->name));
        strncpy(n->basepath, basepath, sizeof(n->basepath));
        n->pakentry = *p;
        hashTable[v] = n;
    }
    else
    {
        texture_hash *n,*old = 0;

        n = hashTable[v];
        while (n)
        {
            old = n;
            if (!strcmpi(n->name,name)) // found it, exit now...
                return;
            n = n->next;
        }

        if(old)
        {
            n = new texture_hash;
            strncpy(n->name, name, sizeof(n->name));
            strncpy(n->basepath, basepath, sizeof(n->basepath));
            n->pakentry = *p;
            old->next = n;
        }
    }
}
