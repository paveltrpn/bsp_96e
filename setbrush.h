#ifndef _SETBRUSH_H_
#define _SETBRUSH_H_

class Entity;
class map;

extern bool fakebrush;

#define	FP_EPSILON		0.01
#define	VECTOR_EPSILON	0.0001

#define	SIDE_FRONT		0
#define	SIDE_BACK		1
#define	SIDE_ON			2

winding_t *ClipWinding (winding_t *in, plane_t *split);
winding_t *CopyWinding (winding_t *w);
winding_t *NewWinding (int points);

// flags "states" for brush
#define B_SELECTED 0x01
#define B_REGIONED 0x02		// not active
#define B_INVALID  0x04		// not a proper polyhedron
#define B_FILTERED 0x08
#define B_LOCKED   0x10
#define B_SAVESEL  0x20
#define B_DRAWABLE 0x40

class SetBrush
{
public:
    SetBrush    *p_next, *p_prev;

    int         flags;

    Entity   	*parent;			// the entity this brush is in
    vec3_t		bmins, bmaxs;
    vec3_t      bctr;
    COLORREF	entitycolor;
    int			numfaces;
    face_t     *currentFace;  // 0 up to numfaces-1
    face_t		faces;       // Double Linked List...

    int         group;
    int			brushId; // for undo operations...
    int         originalIndex;
    SetBrush 	*undoCameFrom;

    SetBrush    *next;   // for memory management...

    // METHODS
    SetBrush();
    ~SetBrush();
    SetBrush *copy();

    // Face handline stuff
    void addObject(face_t *);
    void removeObject(face_t *);
    int  CountFaces();
    void removeAllFaces();

    // Relies on parent...
    bool checkModifiable();
    bool fakeBrush(HDC hdc, int mode, vec3_t, int, int, bool);
    bool getClippedBrush(SetBrush**major, SetBrush**minor);

    void setSaveSelected(bool s);
    bool getSaveSelected();

    bool IsOriginBrush();
    bool IsDetailBrush();

    bool IsFiltered();

    bool CalcFiltered();
    void setFiltered();

    void setLocked(bool s);
    bool IsLocked();

    void setSelected(bool s);
    bool IsSelected();

    void setRegioned(bool s);
    bool IsRegioned();

    void setInvalid(bool s);
    bool IsInvalid();

    void setDrawable();
    bool IsDrawable();

    void setRegioned();
    void setRegionedPartial();

    void initOwner(Entity *own,float *mins,float *maxs,texturedef_t *tex);
    void initFromTokens(map *parent, Entity *own, Tokenizer &script);
    void setMins(float *mins,float *maxs);
    void drawConnections(HDC hdc,int);
    void renderConnections();
    void setParent(Entity *p);

    void setEntityColor(COLORREF color);
    void calcWindings();

    // For Q2 Support texture coordinate calculations...
    //
//   void BeginTexturingFace(face_t *, vec3_t);
    void EmitTextureCoordinates(float *, face_t *);

    void writeToFILE(FILE *f,int grp);

    void getMins(vec3_t mins, vec3_t maxs);

    void snapPlanes();
    void snapSelf();

    bool containsPoint(vec3_t pt);
    void freeWindings(bool free_qtexture = true);
    void removeUnusedFaces();
    void newRegion();

    texturedef_t *texturedef();
    texturedef_t *texturedefForFace(face_t *f);
    void setTexturedef(texturedef_t *tex);
    void setTexturedef(texturedef_t *tex, face_t *f);

    void DrawCenterKnob(HDC hdc, int whichXY, LPSIZE XYSize, float u, float v) ;
    void DrawCenterAngle(HDC hdc, float ang, LPSIZE XYSize, float u, float v);
    void DrawOrigin(HDC hdc, int whichXY, LPSIZE XYSize, float u, float v);
    void DrawDetail(HDC hdc, int whichXY, LPSIZE XYSize, float u, float v);
    HPEN GetBrushPen(Entity *worldent, Entity *currentent);
    void glGetBrushColor(Entity *worldent,Entity *currentent, unsigned char *sR, unsigned char *sG, unsigned char *sB);
    void GetUV(int ViewType, vec3_t vp, float *u, float *v);

    void XYDrawSelf(HDC hdc, vec3_t, int);
    void XYDrawSelfCur(HDC hdc, vec3_t, int);

    void ClosestRenderSelf();

    void CameraRenderSelf();
    void CameraDrawSelfSpecial(HDC hdc,
                               float p, COLORREF linecolor, float Q[][4], float EZMin, float EZMax, LPRECT EClip, LPSIZE ESize);
    void hitByRay(vec3_t p1, vec3_t p2, float *time, face_t **face);
    void centerDistToRay(vec3_t p1, vec3_t p2, float *ctrdist);
    void clipRay(vec3_t p1, vec3_t p2,vec3_t frontpoint, face_t **f_face,
                 vec3_t backpoint, face_t **b_face, int *fflag, int *bflag);

    //
    // single brush actions
    //
    void getZdragface(vec3_t dragpoint, int);
    void getXYdragface(vec3_t dragpoint, int);
    float checkXYmultidragfaces(vec3_t dragpoint, int, face_t **);
    void getXYmultidragfaces(face_t *, int);

    void addMultiPoint(float *);

    bool checkXYEdgeDragFaces(vec3_t ,int);
    void getXYEdgeDragFaces(vec3_t dragpoint, int);

    void getXYShearPoints(vec3_t dragpoint, int);
    void getShearPoints(face_t *face);
    void getXYdragvertex(vec3_t dragpoint, int);
    bool vertexClicked(vec3_t dragpoint, int ViewType);

    void addFace(SetBrush *in,face_t *f);
    //
    // multiple brush actions
    //
    void carveByClipper();
    void splitByClipper();

    void translate();

    void select();
    void deselect();
    void remove();
    void flushTextures();
    void addToBBoxCtr();
    void addToBBox();
    void transform();

    void flipNormals();
    void setCarveVars();

    void moveToEntity();
    void takeCurrentTexture();

    void selectPartial();
    void selectComplete();
    void regionPartial();
    void regionComplete();

    void feetToFloor();

    bool pointAlreadyProcessed(vec3_t pt, face_t *max_face);
    void CameraRenderEdges();
    void glCameraRenderEdges();
    bool checkEdgeDragFaces(vec3_t p1, vec3_t p2);
    void getEdgeDragFaces(vec3_t v1, vec3_t v2);

    // GLBSP stuff
    void glCameraRenderSkybox();
    void glCameraRenderSelfTexture();
    void glCameraRenderSelfTextureTrans(bool solids);
    void glCameraRenderSelfFixed();
    void glCameraRenderSelfOutline();
    void glCameraRenderSelfSelected();
    void glCameraRenderSelfNoTexture();
    void glCameraRenderSelfWireframe();
    void glCameraRenderSelfFlat();
    void glCameraRenderSelfFlatTrans(bool solids);

};

#endif	//_SETBRUSH_H_
