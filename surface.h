#ifndef _SURFACE_H_
#define _SURFACE_H_

#define IDCB_SFLAGBASE 14000
#define IDCB_SCONTENTSBASE 14050
#define IDCB_APPLYFACE 14100
#define IDCB_APPLYBRUSH 14101
#define IDCB_RESET 14102
#define IDCB_APPLYWAL 14103
#define IDCE_SVALUE 14104
#define IDCB_ONTOP 14105

#define IDCB_ADDF 14110
#define IDCB_ADDC 14111
#define IDCB_DELF 14112
#define IDCB_DELC 14113
#define IDCB_SETC 14114
#define IDCB_SETF 14115

#define S_FLAGS 10

class WEditSurf : public WEdit
{
public:
    WEditSurf(HWND parent,int id,char *text,int cx,int cy,int width,int height,int textLimit=0);
    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);
};

class TDownButton : public WButton
{
public:
    TDownButton(HWND parent, int Id, char* text, int X, int Y, int W, int H, int downMsg = 0);
    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);

    int dm;
};

class TSurfaceWindow : public TCWindow, public WindowPlacement
{
public:
    TSurfaceWindow(HWND parent, char *title);
    ~TSurfaceWindow();
    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);

    int bw, fw, cw;
    HWND WP_GetHwnd();
    const char *WP_WindowName();

    WStatic *flagsStatic, *contentsStatic;
    WStatic *sl, *tl, *rl, *sxl, *syl;
    TDownButton *s[2], *t[2], *r[2], *sx[2], *sy[2];
    WEditSurf *sE, *tE, *rE, *sxE, *syE;

    WCheck* flags[S_FLAGS];
    WCheck **contents;
    WEdit *valEdit;
    WStatic *valStatic;
    WCheck *ontop;

    WButton *applyFace;
    void ApplyFace();
    WButton *applyBrush;
    void ApplyBrush();
    WButton *reset;
    void ApplyWal();
    WButton *applyWal;

    void AddFlags();
    void DelFlags();
    void AddContents();
    void DelContents();
    void SetContents();
    void SetFlags();

    WButton *af, *df, *ac, *dc, *sf, *sc;

    void Reset();

    int BrowserGame;
    void GetFlags();
    void GetContents();
    void SetFlags(int);
    void SetContents(int);
    void CreateFlags();
    void CreateContents();
    void CreateValue();

    void MouseDown(int operation);
    void MouseUp(int operation);
    void MouseFunc(int operation);
    /*	void UpX();
    	void DownX();
    	void UpY();
    	void DownY();
    	void UpRotate();
    	void DownRotate();
    	void UpScaleX();
    	void DownScaleX();
    	void UpScaleY();
    	void DownScaleY();
    */
    void SetCaption(const char*);
    void SetupWindow();

    bool CanClose();
};

#endif	//_SURFACE_H_
