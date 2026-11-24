#ifndef _MAP_H_
#define _MAP_H_

#define MODE_REGIONED 0
#define MODE_SELECTED 1

class Entity;
class Group;
class Clipper;
class SetBrush;
class texInfo_t;
class texWad;
class map;

struct mouseobject_t
{
    class SetBrush *Brush;
    class Entity   *Entity;
    bool CenterKnob;
    bool Vertex;
    //bool VertexSel;
    bool Selected;
    bool Nothing;
    bool Camera;
    bool Clipper;
};

// for SaveForUndo
#define UNDO_BRUSHES 0
#define UNDO_RENBRUSH 1
#define UNDO_GROUPS 2
#define UNDO_NOUNDO 3
#define UNDO_FORCEWHOLE 4

// For Undo structure...
#define UNDO_WHOLEMAP 0
#define UNDO_SINGLEBRUSH 1
#define UNDO_GROUP 2
#define UNDO_MULTIBRUSH 3

//snapping modes
#define SNAP_SELF   0	//snap axial planes only
#define SNAP_PLANES 1	//snap each plane point to grid

typedef struct undo_s
{
    char description[64];
    int undoType;
    map *undoMap;

    Entity *singleBrush;  // copy the entity stuff, and then copy only the sole brush
    // on undo:
    // check to see if the entity is still there, if so
    // remove the "original" brush and replace the ptr with
    // this brush...
    // delete the original brush
    // if the entity was deleted, too, then add the entire
    // singlebrush into the map

    Entity **multiBrush;  // similar to single, but many copies
    int numMultiBrush;
    Group *undoGroup;

} undo_t;

typedef struct angles_s
{
    float roll;
    float pitch;
    float yaw;
} t_Angles;

class Entity;

class map
{
public:
    int count;
    int dirty;
    char filename[256];
    Entity *world; // save a pointer to the world entity...
    Entity *current;
    Entity *oldselection;
    vec3_t minz, maxz;
    Entity objects;
    Group *groups;
    vec3_t eye[MAX_CAMERAS];  // current position...
    int cureye;
    t_Angles angles[MAX_CAMERAS];

    bool regionSet;
    SetBrush *regionBrush;

    // texture stuff
    texInfo_t **bests;     // an array of the bests
    texInfo_t **favorites; // an array of favorites...

    char **favs;
    int numf;   // number in array

    texInfo_t **mapTextures;
    int numMapTextures;
    texWad **Wads;
    int numWads;
    char texName[64];
    char texPath[64];

    //
    Clipper *clipper_i;
    int curclippoint;

    unsigned char *dat;

    int mapType;

    undo_t *undo;

    map();
    ~map();
    void cleanUpInvalidObjects();
    void saveForUndo(char * /*saveStr*/, int);
    void Undo(map *old, map **newm);
    void undoGetRest(map *cm);
    map *undoCopy();
    void RemoveUndo();

    bool lookForObject(Entity *e);
    void MoveEyePosition(vec3_t newpos, int lockValue);
    void copySelection(map **, bool rebuild);
    void pasteSelection();

    void GatherTextureDirectories();

    void GlobalSetKey(char *, char *);
    SetBrush *selectedBrush();
    int numSelected();
    int numSelectedTotal();
    int numBrushes();
    void newMap();
    void snapSelected(int mode);
    void getCenter(vec3_t ctr);
    void getSelectedCenter();
    void makeSelectedPerform(int sel);
    void makeSelectedXYDraw(HDC hdc,int index,int stopAt, Group *g);
    void makeUnselectedPerform(int sel);
    void makeGlobalPerform(int sel);
    void makeAllPerform(int sel);
    void removeObject(Entity *);
    Entity *currentEntity();
    void setCurrentEntity(Entity *);
    void setCurrentMinZ(vec3_t);
    void setCurrentMaxZ(vec3_t);
    vec_t currentMinZ(int);
    vec_t currentMaxZ(int);
    bool readMapFile(char *fname);
    void mergeMapFile(char *fname);
    void writeMapFile(char *fname, int writeWorldKeys,int selOnly);
    void writeBox(FILE *f, char *walls, char *sky, char *floor);
    void writeSelected(FILE *f, Entity *e);
    Entity *findPlayerStart();
    Entity *findWorldSpawn();
    void DoExport(char *goodname, char *runname, char *title);

    int getVisibility(Entity *e, int inBox, int mode);
    void groupExportWrite(FILE *f, Entity *e, int func);
    void groupExport(int func);

    void groupSwap(int g1, int g2);
    void groupInsert(int pos);

    void selectedExport();
    void regionExport();
    void addObject(Entity *ent);
    void DrawOrientation(HDC hdc);
    bool entityConnect(vec3_t p1, vec3_t p2);
    void selectRay(vec3_t p1, vec3_t p2, bool ef);
    void selectKnob(vec3_t p1, vec3_t p2, int UAxis, int VAxis, bool ef);
    SetBrush *grabRay(vec3_t p1, vec3_t p2);
    SetBrush *getTextureRay(vec3_t p1, vec3_t p2);
    void setTextureRay(vec3_t p1, vec3_t p2, bool allsides);
    void sel_identity();
    void transformSelection();
    void swapvectors(vec3_t a, vec3_t b);
    void rotate_x();
    void rotate_y();
    void rotate_z();
    void flip_x();
    void flip_y();
    void flip_z();
    void cloneSelection();
    void selectCompleteEntity();
    void selectPartialEntity();
    void makeEntity();
    bool lookAtSelection(bool force = false);

    int countSelectedBrushesWithEdge(vec3_t p1, vec3_t p2);

    void selbox(int sel);
    void selectCompletelyInside();
    void selectPartiallyInside();
    void makeRegionBrush();
    void makeRegionBrushPartial();
    void tallBrush();
    void shortBrush();
    void sizeBrush(int);
    void makeRoom();
    void extrudeRoom();
    void extrudeFace();
    void rotateBrush(int r, int p, int y);
    void rotateBrushOrg(int r, int p, int y, vec3_t org);
    void makeLights(int x, int y, int z, int val, char *lightType);
    int makeArch(Entity *currentEntity, SetBrush *seedBrush, int removeSeed,
                 int numsteps, int xory, int width, int rect);
    int makeStairs(Entity *currentEntity, SetBrush *seedBrush, int removeSeed, int style,
                   int numsteps, int hgap, int vgap, int down, int xory, float turns, int distfc);
    void makeSphere(int type, int hstrips, int vstrips, int neardist, int fardist, int take, int percent);
    void makeCylinder(int sides, int strips, int neardist, int fardist, int take, int percent, int xalign);
    void makeNSided(int sides, int radius, int xalign, int take);
    void makePyramid(int sides, int radius, int height, int xalign, int take);
    int makeWedge(Entity *currentEntity, SetBrush *seedBrush, int removeSeed, int axis, bool flip);
    void scaleBrush(float sx, float sy, float sz);
    void subtractSelection();
    void replaceTexture(char *f, char *r);
    void selectByTexture(char *buffer);
    void selectByEntityKeyValue(bool useE, bool useK, bool useV,char *ent, char *key, char *val);
    void revolveObject(char *fname, int hstrips, int neardist, int fardist);

    void getMouseObject(mouseobject_t *obj, vec3_t p1, vec3_t p2, vec3_t dragpoint, int ViewType, int UAxis, int VAxis);

    void UnloadTextures();

    int MixFilter(texturedef_t *td);
    int MixedFaceSearch();
    int TextureSearch(char *name);

    void AddDetail();
    void RemoveDetail();
    void SetDetail(bool addDetail); // true for add, false for subtract

    void glDrawOrientation();
    void glBrightness();

    void UpdateGroupVisibility(int);
    void UpdateMapVisibility();
    void UpdateCurrent();
};


#endif	//_MAP_H_
