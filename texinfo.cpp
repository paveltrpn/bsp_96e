#include "global.h"

texInfo_t::texInfo_t()
{
    index = 0;
    memset(bmp,0,LUMP_NAME_LENGTH);
    bits = NULL;
    w = 0;
    h = 0;
    memset(&flat,0,sizeof(pixel32_t));
    dibInfo = NULL;
    truedibInfo = NULL;
    truebits = NULL;
    palette = NULL;
    palSize = 0;
    previous = NULL;
    next = NULL;
    count = 0;

    aTextures = NULL;
    animated = 0;
    aCount = 0;
    aCurrent = 0;

    sky = NULL;
    bindIndex = 0;
    GLBits = NULL;
    glBound = false;
    useW = 0;
    useH = 0;
    uMult = 1.0;
    vMult = 1.0;

    selected = false;

    // Quake II support
    def_flags    = 0;
    def_contents = 0;
    def_value    = 0;

    texType = TEX_TYPE_Q1; // 1 is QII texture
    memset(basepath,0,32);
}

texInfo_t::~texInfo_t()
{
    delete[] bits;
    bits = NULL;

    delete[] dibInfo;
    dibInfo = NULL;

    if (truebits != NULL)
    {
        delete truebits;
        truebits = NULL;
    }

    if (truedibInfo != NULL)
    {
        delete truedibInfo;
        truedibInfo = NULL;
    }

    if (palette != NULL)
    {
        delete palette;
        palette = NULL;
    }

    if (aTextures != NULL)
    {
        delete aTextures;
        aTextures = NULL;
    }
    aCount = 0;
    aCurrent = 0;

    if (sky)
    {
        delete[] sky->bits;
    }

    if (sky != NULL)
    {
        delete sky;
        sky = NULL;
    }
    delete[] GLBits;
    GLBits = NULL;
}
// isWarp
bool texInfo_t::isWarp()
{
    return (texType==TEX_TYPE_Q2 && (def_flags & SURF_WARP)) || *bmp=='*';
}
// isSkyQ1 - Q1 style sky
bool texInfo_t::isSkyQ1()
{
    return sky && texType==TEX_TYPE_Q1 && !strnicmp(bmp,"sky",3);
}
