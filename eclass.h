#ifndef _ECLASS_H_
#define _ECLASS_H_

typedef enum
{
    esize_model, esize_fixed
} esize_t;

typedef struct dskindata_s
{
    int skinwidth;
    int skinheight;
    unsigned char *data;
} dskindata_t;

#define	MAX_FLAGS	8
class EntityClass
{
public:

    EntityClass();
    ~EntityClass();

    char	 *name;
    esize_t	 esize;
    vec3_t	 mins, maxs;
    COLORREF color;
    char	 *comments;
    char	 flagnames[MAX_FLAGS][32];

    int      mdlType; // 0 = Quake, 1 = Quake2
    mdl_t    *mdl;
    dmdl_t   *mdlQ2;

    // Generalized MDL Stuff
    int      curframe;
    int      maxframe;
    int      curskin;
    int      maxskin;
    bool     showMdl;

    // Model loaders
    mdl_t    *LoadMDLFromPak(char *,char *);

    // Q2
    bool     read_from_pak;
    dskindata_t *skinTextureData;
    void     InitModelTexture(EntityClass *, dmdl_t *mdl,char *pakfile,int skin_num,char *skin_fname);
    void     mdl_loader(FILE *f, dmdl_t *mdl, int offset);
    dmdl_t   *LoadMDLFromQ2Pak(char *, char *);
    FILE     *FindInPath(char *path, char *filename, int *size, int *offset);
    FILE     *FindInPak(char *pakname, char *filename, int *size, int *offset);
    dmdl_t   *LoadMDLFromQ2MDL(char *fname);

    // Model Unloaders
    void     FreeModel(mdl_t *);
    void     FreeModelQ2(dmdl_t *);

    // Utility Functions...
    void     initFromText(char *text);
    char     *getclassname();
    esize_t  getesize();
    float    *getmins();		// only for esize_fixed
    float    *getmaxs();		// only for esize_fixed
    COLORREF drawColor();
    char     *getcomments();
    char     *getflagName(unsigned flagnum);
};

class EntityClassList
{
public:
    EntityClassList();
    ~EntityClassList();

    EntityClass   *nullclass;
    char	        *source_path;
    int           numElements;
    EntityClass   **objects;
    unsigned char *pal;

    int            indexOf(EntityClass *e);
    EntityClass    *objectAt(int i);
    void           initForSourceDirectory(char *path);
    EntityClass    *classForName(char *name);
    void           scanDirectory();
    void           scanFile(char *filename);
    void           insertEC(EntityClass *ec);
};


#endif	//_ECLASS_H_
