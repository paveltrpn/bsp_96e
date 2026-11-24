#ifndef _DIALOGS_H_
#define _DIALOGS_H_

#include "propctl.h"
#include "commands.h"

class SetBrush;
class Entity;
class CPoint;

#define MAX_EDITLEN 40

// Transfer structs

struct TGLBuffer
{
    int Mipmap;
    ListData Mode;
    char selected[256];
};

struct TGameBuffer
{
    int opengl;
    ListData dir;
    char selected[256];
};

struct TScanTransferBuffer
{
    ListData Directory;
    char selected[MAX_PATH];
    char caption[100];
};

struct TWalTransferBuffer
{
    char sel_dir[256];
    char sel_wal[256];
};

struct TMipMapTransferBuffer
{
    char Name[32+1];
    int Width;
    int Height;
    int Top;
    int Left;
};
struct TExportTransferBuffer
{
    int inBox;
    ListData sky;
    ListData walls;
    ListData floor;
    ListData exporter;
    char sel_sky[128];
    char sel_walls[128];
    char sel_floor[128];
    char sel_exporter[128];
};

struct TStairTransferBuffer
{
    //edit
    int NumSteps;
    int HGap;
    int VGap;
    float NumTurns;
    int Dist;
    //radio
    int RegularButton;
    int SpiralButton;
    int BlockButton;
    //check
    int XAxis;
    int GoesDown;
};

struct TWedgeTransferBuffer
{
    char Axis[MAX_EDITLEN];
    int flip;
};

struct TArchTransferBuffer
{
    char Stones[MAX_EDITLEN];
    char width[MAX_EDITLEN];
    int XAxis;
    int RectTop;
};

struct TLightTransferBuffer
{
    char NumX[MAX_EDITLEN];
    char NumY[MAX_EDITLEN];
    char NumZ[MAX_EDITLEN];
    char LightVal[MAX_EDITLEN];
    ListData boxdata;
    char selected[MAX_KEY];
};

struct TRotateTransferBuffer
{
    char Roll[MAX_EDITLEN];
    char Pitch[MAX_EDITLEN];
    char Yaw[MAX_EDITLEN];
};

struct TCopyTransferBuffer
{
    int MoveOnly;
    char NumX[MAX_EDITLEN];
    char NumY[MAX_EDITLEN];
    char NumZ[MAX_EDITLEN];
    char NumCopies[MAX_EDITLEN];
};

struct TSphereTransferBuffer
{
    char HStrips[MAX_EDITLEN];
    char VStrips[MAX_EDITLEN];
    char Inner[MAX_EDITLEN];
    char Outer[MAX_EDITLEN];
    char InnerPercent[MAX_EDITLEN];
    int TakeSeed;
};

struct TRevolveTransferBuffer
{
    char HStrips[MAX_EDITLEN];
    char Inner[MAX_EDITLEN];
    char Outer[MAX_EDITLEN];
};

struct TKPTransferBuffer
{
    char Key[MAX_KEY];
    char Value[MAX_VALUE];
    int Global;
};

struct TMergeTransferBuffer
{
    int SaveFirst;
    int SelectedOnly;
    int TextureCompare;
    int MatchingFlags;

    char Face[20];
    char Normal[20];
    char Point[20];
    char Convexity[20];
};

struct TSelectTransferBuffer
{
    ListData Entity;
    char Key[MAX_KEY];
    char Value[MAX_VALUE];

    int SelectEntity;
    int SelectKey;
    int SelectValue;
    char selected[MAX_KEY];
};

struct TCylTransferBuffer
{
    char Sides[MAX_EDITLEN];
    char Strips[MAX_EDITLEN];
    char Inner[MAX_EDITLEN];
    char Outer[MAX_EDITLEN];
    int TakeSeed;
    char InnerPercent[MAX_EDITLEN];
    int XAlign;
};

struct TNTransferBuffer
{
    char Sides[MAX_EDITLEN];
    char Radius[MAX_EDITLEN];
    int XAlign;
    int TakeSeed;
};

struct TScaleTransferBuffer
{
    char ScaleX[MAX_EDITLEN];
    char ScaleY[MAX_EDITLEN];
    char ScaleZ[MAX_EDITLEN];
};

struct TPyramidTransferBuffer
{
    char Sides[MAX_EDITLEN];
    char Radius[MAX_EDITLEN];
    char Height[MAX_EDITLEN];
    int XAlign;
    int TakeSeed;
};

struct TCreateTransferBuffer
{
    char X[MAX_EDITLEN];
    char Y[MAX_EDITLEN];
    char Z[MAX_EDITLEN];
    char DX[MAX_EDITLEN];
    char DY[MAX_EDITLEN];
    char DZ[MAX_EDITLEN];
};

struct TSetHeightsTransferBuffer
{
    char Base[MAX_EDITLEN];
    char Height[MAX_EDITLEN];
};

//DIALOGS

class TRotateDlg : public WDialog
{
public:
    TRotateDlg(HWND parent, TRotateTransferBuffer *xfer);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~TRotateDlg();
    TRotateTransferBuffer *xfer;
    WEdit *Roll, *Pitch, *Yaw;
};

class TCopyDlg : public WDialog
{
public:
    TCopyDlg(HWND parent, TCopyTransferBuffer *xfer);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~TCopyDlg();
    TCopyTransferBuffer *xfer;
    WEdit *NumX, *NumY, *NumZ, *NumCopies;
    WCheck *MoveOnly;
};

class TCreateDlg : public WDialog
{
public:
    TCreateDlg(HWND parent, TCreateTransferBuffer *xfer);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~TCreateDlg();
    TCreateTransferBuffer *xfer;
    WEdit *X, *Y, *Z, *DX, *DY, *DZ;
};

class TSelectDlg : public WDialog
{
public:
    TSelectDlg(HWND parent, TSelectTransferBuffer *xfer);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~TSelectDlg();
    TSelectTransferBuffer *xfer;
    WCombo *Entity;
    WEdit *Key, *Value;
    WCheck *SelectEntity, *SelectKey, *SelectValue;
};

class TMergeDlg : public WDialog
{
public:
    TMergeDlg(HWND parent, TMergeTransferBuffer *xfer);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~TMergeDlg();
    TMergeTransferBuffer *xfer;
    WCheck *SaveFirst, *SelectedOnly, *TextureCompare, *MatchingFlags;
    WEdit *Face, *Normal, *Point, *Convexity;
};

class TKPDlg : public WDialog
{
public:
    TKPDlg(HWND parent, TKPTransferBuffer *xfer, int focusFld);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~TKPDlg();
    int focus;					// 0 = focus on editkey, 1 = focus on editvalue
    TKPTransferBuffer *xfer;	//
    WEdit *editkey, *editvalue;	//
    WCheck *global;				// apply to all selected ents
};

class TScaleDlg : public WDialog
{
public:
    TScaleDlg(HWND parent, TScaleTransferBuffer *xfer);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~TScaleDlg();
    TScaleTransferBuffer *xfer;
    WEdit *ScaleX, *ScaleY, *ScaleZ;
};

class TSphereDlg : public WDialog
{
public:
    TSphereDlg(HWND parent, TSphereTransferBuffer *xfer);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~TSphereDlg();
    TSphereTransferBuffer *xfer;
    WEdit *HStrips, *VStrips, *Inner, *Outer, *InnerPercent;
    WCheck *TakeSeed;
};

class TStairDlg : public WDialog
{
public:
    TStairDlg(HWND parent, TStairTransferBuffer *xfer);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~TStairDlg();
    TStairTransferBuffer *xferBuffer; // save pointer to it

    WEdit *NumSteps, *HGap, *VGap, *NumTurns, *Dist;
    WRadio *RegularButton, *SpiralButton, *BlockButton;
    WCheck *XAxis, *GoesDown;

    void CmOk();

    HDC         hmemdc;        // handle to device context for drawing
    HBITMAP		hmembitmap;  // memory bitmap of drawing

    void MemDraw();
    void Redraw(HDC hdc);
    void ConstructMemDC();
    void RemoveMemDC();

    void EvLButtonDown(UINT, POINT&);
    void EvLButtonUp(UINT, POINT&);
    void EvMouseMove(UINT, POINT&);

    void SetupWindow();
    void EvPaint();
    void Preview();
    void ResetAngles();
    void LeftButton();
    void RightButton();
    void InButton();
    void OutButton();

    void PUp();
    void PDown();
    void RUp();
    void RDown();

    void Reconstruct();

    SetBrush *fakeBrush;
    Entity *fakeEnt;

    float c_yaw;
    float v_pitch;
    float v_roll;
    float s;
    float p;
    float dist;
    float width;
    float height;

    float stepHeight;

    vec3_t ctr;
    float U[4][4];
    RECT PreviewRect;
    SIZE PreviewSize;

    POINT startPt;
    POINT lastPt;
    int mode;
};

class TArchDlg : public WDialog
{
public:
    TArchDlg(HWND parent, TArchTransferBuffer *xfer);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);

    ~TArchDlg();

    WEdit *Stones, *Width;
    WCheck *XAxis, *RectTop;

    int badbrush;

    HDC		hmemdc;        // handle to device context for drawing
    HBITMAP	hmembitmap;  // memory bitmap of drawing

    void MemDraw();
    void Redraw(HDC hdc);
    void ConstructMemDC();
    void RemoveMemDC();

    void EvLButtonDown(UINT, POINT&);
    void EvLButtonUp(UINT, POINT&);
    void EvMouseMove(UINT, POINT&);

    void SetupWindow();
    void EvPaint();
    void Preview();
    void ResetAngles();
    void LeftButton();
    void RightButton();
    void InButton();
    void OutButton();

    void PUp();
    void PDown();
    void RUp();
    void RDown();

    void Reconstruct();

    SetBrush *fakeBrush;
    Entity *fakeEnt;

    float c_yaw;
    float v_pitch;
    float v_roll;
    float s;
    float p;
    float dist;
    float width;
    float height;

    vec3_t ctr;
    float U[4][4];
    RECT PreviewRect;
    SIZE PreviewSize;

    POINT startPt;
    POINT lastPt;
    int mode;

    void get_Transfer();
    void set_Transfer();
    TArchTransferBuffer *xferBuffer; // save pointer to it
};

class TWedgeDlg : public WDialog
{
public:
    TWedgeDlg(HWND parent, TWedgeTransferBuffer *xfer);
    ~TWedgeDlg();
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    TWedgeTransferBuffer *xferBuffer; // saved pointer
    WEdit *Axis;
    WCheck *flip;

    int badbrush;

    HDC     hmemdc;
    HBITMAP hmembitmap;

    void MemDraw();
    void Redraw(HDC dc);
    void ConstructMemDC();
    void RemoveMemDC();

    void EvLButtonDown(UINT, LPPOINT);
    void EvLButtonUp(UINT, LPPOINT);
    void EvMouseMove(UINT, LPPOINT);

    void SetupWindow();
    void EvPaint();
    void Preview();
    void ResetAngles();
    void LeftButton();
    void RightButton();
    void InButton();
    void OutButton();

    void PUp();
    void PDown();
    void RUp();
    void RDown();

    void Reconstruct();

    SetBrush *fakeBrush;
    Entity *fakeEnt;

    float c_yaw;
    float v_pitch;
    float v_roll;
    float s;
    float p;
    float dist;
    float width;
    float height;

    vec3_t ctr;
    float U[4][4];
    RECT PreviewRect;
    SIZE PreviewSize;

    POINT startPt;
    POINT lastPt;
    int mode;
};

class TLightDlg : public WDialog
{
public:
    TLightDlg(HWND parent, TLightTransferBuffer *xfer);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~TLightDlg();
    TLightTransferBuffer *xfer;
    WEdit *NumX, *NumY, *NumZ, *LightVal;
    WCombo *LightEnts;
};

class TScanDlg : public WDialog
{
public:
    TScanDlg(HWND parent, TScanTransferBuffer *xfer);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~TScanDlg();
    TScanTransferBuffer *xfer;
    WCombo *Directory;
};

class TGameDlg : public WDialog
{
public:
    TGameDlg(HWND parent, TGameBuffer *xfer);
    ~TGameDlg();
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    TGameBuffer *xfer;
    WList *games;
    WCheck *opengl;
};

class TGLDlg : public WDialog
{
public:
    TGLDlg(HWND parent, TGLBuffer *xfer);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~TGLDlg();
    TGLBuffer *xfer;
//	WCheck *Mipmap;
    WCombo *Mode;
};

class texInfo_t;

class TWalDlg : public WDialog
{
public:
    TWalDlg(HWND parent, TWalTransferBuffer *xfer);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~TWalDlg();
    TWalTransferBuffer *xfer;

    void SetupWindow();
    void EvPaint();

    void LoadListBox(char *);

    void EvCBNSelChange();
    void EvLBNSelChange();

    WCombo *dir;
    WList *wal;

    WStatic *contents, *flags, *values;

    texInfo_t *newTex;
};

class TExportDlg : public WDialog
{
public:
    TExportDlg(HWND parent, TExportTransferBuffer *xfer);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~TExportDlg();
    TExportTransferBuffer *xfer;
    WCheck *inBox;
    WCombo *sky, *walls, *floor, *exporter;
};

class TCylDlg : public WDialog
{
public:
    TCylDlg(HWND parent, TCylTransferBuffer *xfer);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~TCylDlg();
    TCylTransferBuffer *xfer;
    WEdit *Sides, *Strips, *Inner, *Outer, *InnerPercent;
    WCheck *TakeSeed, *XAlign;
};

class TNDlg : public WDialog
{
public:
    TNDlg(HWND parent, TNTransferBuffer *xfer);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~TNDlg();
    TNTransferBuffer *xfer;
    WEdit *Sides, *Radius;
    WCheck *XAlign, *TakeSeed;
};

class TPyramidDlg : public WDialog
{
public:
    TPyramidDlg(HWND parent, TPyramidTransferBuffer *xfer);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~TPyramidDlg();
    TPyramidTransferBuffer *xfer;
    WEdit *Sides, *Radius, *Height;
    WCheck *XAlign, *TakeSeed;
};

// TMipMapDlg
class TMipMapDlg : public WDialog
{
public:
    TMipMapDlg(HWND parent, TMipMapTransferBuffer *xfer, unsigned char *image_i, int ix_i, int iy_i);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~TMipMapDlg();
    TMipMapTransferBuffer *xferBuffer;
    WEdit *te, *le, *we, *he, *name;

    void MemDraw();
    void Redraw(HDC hdc);
    void ConstructMemDC();
    void RemoveMemDC();

    void EvLButtonDown(UINT, POINT&);
    void EvLButtonUp(UINT, POINT&);
    void EvMouseMove(UINT, POINT&);

    void SetupWindow();
    void EvPaint();
    void Preview();

    void Interpolate(int, int, int*, int*);
    int top, left, w, h;

    unsigned char *image;
    int ix;
    int iy;
    HDC hmemdc;        // handle to device context for drawing
    HBITMAP hmembitmap;  // memory bitmap of drawing

    RECT PreviewRect;
    POINT startPt;
    POINT lastPt;
    int mode;
};

class TSetHeightsDlg : public WDialog
{
public:
    TSetHeightsDlg(HWND parent, TSetHeightsTransferBuffer *xfer);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~TSetHeightsDlg();
    TSetHeightsTransferBuffer *xfer;
    WEdit *Base, *Height;
};

//
// FindReplaceDlg
//
// callback function responsible for destroying window, deleting instance, and set instance ptr=0.
// findreplace dialog closes when callback returns true.
typedef bool (*FR_CALLBACK) (bool replace, char *findwhat, char *replacewith);
class FindReplaceDlg : public WDialog
{
public:
    FindReplaceDlg(HWND parent, char *findwhat, char *replacewith, FR_CALLBACK findrep);
    ~FindReplaceDlg();
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);

    HWND hwnd;
    FR_CALLBACK Callback;
    WEdit *findwhat, *replacewith;
};

//
// Keyboard Settings
//
class KeyboardSettingsDlg : public WDialog
{
public:
    KeyboardSettingsDlg(HWND parent);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~KeyboardSettingsDlg();
    void SetDescription(ccmd_t *c);

    KBSettings *kbset_temp;				// temporary keyboard settings
    WEdit *shortcut;					// shortcut edit box
    WButton *showkeys, *restorekeys, *save;
    WCombo *commands;					// bsp commands list
    WStatic *descript;					// command description box
};


//
// BSP Settings
//
class BspSettingsDlg : public WDialog
{
public:
    BspSettingsDlg(HWND parent, char *start_group);
    ~BspSettingsDlg();
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    bool FillPropertyGrid(char *groupname);
    void FillGroupList(char *select);
    void ChangePropertyView(char *newgroupname);
    int GetMinWidth();
    void Sizing(int side, RECT *rc);
    void Resize(int width, int height);
    bool SaveSettings();
    bool StartEdit();
    void BtnWriteBspSettings();
    void BtnWriteGameSettings();
    void BtnCancel();
    void BtnOk();
    void BtnClose();
    WPropertyGrid *wprop;
    WButton *wbtn_ok;
    WButton *wbtn_cancel;
    WButton *wbtn_write;
    WButton *wbtn_writegame;
    WStatic *iteminfo;
    WStatic *groupstatic;
    WCombo *grouplist;
    char *start_group;
};


#endif	//_DIALOGS_H_
