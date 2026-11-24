#ifndef _TEXTURE_H_
#define _TEXTURE_H_

// TexureWindow Boxes //////////////////////////////

#define TEXWIDTH 48
#define TEXHEIGHT 48
#define TEXHSPACE 10
#define TEXVSPACE 14
#define TEX_FONTSIZE 12
#define TEX_FONTNAME "arial"
#define TEXFONTBUFFER 1
#define NAMEOFFSET 35

#define MAX_FAVORITES 10
#define MAX_FREQUENT 10
#define LABEL_WIDTH 120		//label for texture name

#define SAVE_WAD		1999
#define SELECT_ALL		1998
#define DESELECT_ALL	1997
#define SELECT_USED		1996
#define SAVE_SELECTED	1995
#define CLIP_COPY		1994
#define CLIP_PASTE		1993
#define CLIP_COPY_IMAGE 1992
#define SCAN_NEW_WAD	1991
#define UNLOAD_WAD		1990
#define SCAN_NEW_WAL	1989
#define SCAN_IN_USE		1988
#define SCAN_NEW_WALDIR 1987
#define WAD_TO_WAL		1986
#define SCAN_WAL_PAKQ2	1985
#define SCAN_WAL_PAK	1984
#define GL_SETTINGS		1983

extern void UnloadTextures();

class TColorMap
{
public:
    TColorMap(unsigned char *pal);
    ~TColorMap();
    unsigned char *lookUp;
};

class texInfo_t;

typedef struct sky_s
{
    int skyRow;     // current starting row  for top part
    int skyCol;     // current starting column for bottom part

    unsigned char *bits;
    unsigned char *truebits;
} sky_t;

// create an array of these, sort by bmp and then ...and then WHAT??? what are you getting at???
// it's simple...
class texInfo_t
{
public:
    texInfo_t();
    ~texInfo_t();

    bool isWarp();
    bool isSkyQ1();

    int index;
    char bmp[256];

    unsigned char *bits;
    unsigned char *truebits;
    int w, h;
    pixel32_t flat;

    BITMAPINFO *dibInfo;
    BITMAPINFO *truedibInfo;
    unsigned char *palette;
    WORD palSize;
    int count;
    texInfo_t **aTextures;
    int animated;
    int aCount;
    int aCurrent;

    sky_t *sky;

    // GLBSP Stuff
    int bindIndex;
    bool glBound;
    int useW, useH;
    float uMult, vMult;
    unsigned char *GLBits;
    // end GLBSP

    bool selected;

    int def_flags;
    int def_contents;
    int def_value;

    int texType;
    char basepath[32];

    texInfo_t *previous;
    texInfo_t *next;
};

#define MAX_HASH 331
class texture_hash
{
public:
    texture_hash();

    char name[32];
    char basepath[32];
    pakentry_t pakentry;
    texture_hash *next;
    texture_hash *prev;
};

#define QUAKE_WAD 0x00
#define QUAKEII_WAD 0x01

class texWad
{
public:
    int WadType;   // 0:Quake 1:Quake2 - dir of .wal files
    char currentWad[128];
    int bmpsCount;
    texInfo_t *textures;
    texWad *nextWad;
};

typedef struct TLOGPAL_s
{
    WORD Version;
    WORD NumberOfEntries;
    PALETTEENTRY aEntries[256];
} TLOGPAL;

class ListBoxData
{
public:
    char description[32];
    texInfo_t *tx;
};

class textureEntry
{
public:
    textureEntry();
    textureEntry(char *name);

    char dirName[64];
    textureEntry *next;
};

class TTextureWindow : public TCWindow, public WindowPlacement
{
public:
    TTextureWindow(HWND parent, char *title);
    ~TTextureWindow();

    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);

    HWND WP_GetHwnd();
    const char *WP_WindowName();

    WCombo *texList;
    WScroll Scroller;

    int keepOnTop;
    void ToggleKeepOnTop();

    // Rendering stuff...
    HDC         hmemdc;			// handle to device context for drawing
    HBITMAP     hmembitmap;		// memory bitmap of drawing
    HDC         hmemdcFF;
    HBITMAP     hmembitmapFF;
    RECT		MainRect;
    RECT		FFRect;
    HFONT   namesfont;

    void ConstructMemDC();
    void RemoveMemDC();
    void ClearContents();
    SIZE lastSz;

    // GLBSP Stuff
    void CalcGLBits(texInfo_t *newTex, unsigned char *thePal);
    // End

    qtexture_t *badQ;

    texInfo_t *rightClickedTexture;

    int lmousedown;
    POINT startpt;
    POINT newpt;
    POINT delta, relative;
    int clickIndex;
    int clickArea;

    void ClickglBspSettings();

    float oldmag;
    void ZoomTexture(POINT *point);

    int curFav;
    int curFreq;

    RECT ClipFavorites;
    RECT ClipFrequent;

    void ToggleNames();

    void ToggleFav();
    void ToggleFrq();

    void CalcFrequents();
    void ClickFrequents();
    void CalcUsage();

    int nameoffset;

    char texStatic[128];

    texWad *firstWad;
    texWad *currentWadPtr;

    texturedef_t *currentTexture;

    int trueW, trueH;
    int fulltw, fullth;
    int useSmalls;
    int saveNames;
    void ToggleSmalls();
    void SetWH();

    void CalcWH();
    void CalcMetrics();
    void CalcFF();
    void UpdateScrollbar(bool setpos = true, bool redraw = true);

    void FakeSave();

    bool WasTextureClicked(LPPOINT, int *, map *);
    void AddToWad(texWad*, char *name, unsigned char *image, int rw, int rh, int xoffset, int yoffset, int ix, int iy);


    void GetTextureDisplayRects(int x, int y, LPRECT src, LPRECT dst,texInfo_t *tx, int tw, int th);

    int noffset;

// put in a calcMetrics thingy...
    int totrows;
    int curstartrow;
    int rowsperpage;
    int colsperrow;

    int prevIndex;

    int waterFrame;
    float ha, va;
    float hp, vp;
    int tperiod;
    float *xo_lookup;
    float *yo_lookup;

    int stepSize;
    void ToggleSky();

    void EvTimer(UINT timerId);

    void CreateSkyBits(int w, int h, sky_t *sky, void *out);
    void CreateWaterBits(int w, int h, sky_t *sky, void *out);

    void UpdateColorValues();
    void SetUpMap(map *);
    texWad *QueryWadLoaded(char *); // returns NULL if not found...
    void ChangeToMap(map *);
    void AddWad(texWad *wad, char *wadName, char *loadName, int loadType);
    texInfo_t *GetTextureNo(texWad *wad,int index);
    texInfo_t *GetTextureByName(map *m,char *name);
    void FreeTextures(texWad *wad);
    void ClearCounts(map *m);
    int GetTextureIndex(map *m,texInfo_t *t);

    void SetEditBoxes();
    void GetEditBoxes();
    void ChangeTexture();

    void NewCurrentTexture(texInfo_t *tx);

    void ResetDefaults();
    void getTextureDef(texturedef_t *);
    void setTextureDef(texturedef_t *);
    void LoadTextures(char *fname, int showstat);
    void LoadTextureDirectory(char *basepath, char *dirname);
    void LoadTexturesInUse(char *basepath, map *m);

    int GetHashValue(char *name, char *basepath);
    void AddToHash(char *name, char *basepath, texture_hash **hashTable);
    void AddToHashPak(char *name, char *basepath, pakentry_t *p, texture_hash **hashTable);
    void GatherPakTextures(char *pakname, texture_hash **hashTable);
    pakentry_t *SearchHash(char *name, char *basepath, texture_hash **hashTable);

    bool CheckWad(char *fname);
    void CreateFakeWad(char *fname);

    int GetRandomFlat();

    unsigned char BestColor(unsigned char *pal, int r, int g, int b, int start, int stop);
    unsigned char AveragePixels(int);

    qtexture_t *GetQTexture(char *name);
    pixel32_t TranslateColors(int pSize,unsigned char *pal,unsigned char *t_bits,int size);

    void ResetButton();
    void StretchButton();
    void TDButton();
    void TUButton();
    void SetScaleStep();
    void SetShiftStep();
    void SetRotateStep();

    void SetCaption(const char* title);

    void RedrawFavorite();
    void RedrawFrequent();
    void RedrawContents();
    void RedrawAnimated();
    void RedrawClient(HDC hdc);
    void Select(int, bool);
    void SelectFF(HDC);
    void StartAnimation();
    void StopAnimation();

    void SelectAll();
    void DeselectAll();
    void SelectUsed();
    void SaveSelected();
    int NumSelected();

    void CopyToClipboard();
    void CopyImageClipboard();
    void PasteFromClipboard();

    void ScanNewWad();
    void ScanNewWalDirectory();
    void UnloadWad();
    void UnloadWad(char *dir, bool verbose);

    void ScanNewTexture();
    texInfo_t *TryLoadTextureWal(map *,char *);
    void ScanInUse();
    void ScanInUse(map *m);

    void ScanNewWalFromPak(bool);
    void ScanNewWalFromQ2Pak();
    void ScanNewWalFromSelectedPak();

    void LoadPakDirectory(char *dir, char *pakname);
    void WadToWal();

    void Invalidate();
private:
    bool EvEraseBkgnd(HDC);
    void SetupWindow();
    void EvSize(UINT, LPSIZE);
    void Paint();

    void EvLButtonDblClk(UINT modKeys, LPPOINT point);
    void EvLButtonUp(UINT modKeys, LPPOINT point);
    void EvLButtonDown(UINT modKeys, LPPOINT point);
    void EvRButtonDown(UINT modKeys, LPPOINT point);
    void EvMouseMove(UINT modKeys, LPPOINT point);
    void MouseWheel(WPARAM wParam, LPARAM lParam);

    void EvCBNSelChange();
    bool CanClose();
};

#endif	//_TEXTURE_H_
