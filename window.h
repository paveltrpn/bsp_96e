#pragma once

struct WNDATTR
{
    HWND parent;
    int width;
    int height;
    int x;
    int y;
    DWORD dwExStyle;
    DWORD dwStyle;
    HMENU menu;
    LPVOID param;
};

//
// Window
//
class Window
{
public:
    HWND hwnd;
    WNDCLASSEX wc;
    WNDATTR attr;
    char caption[256];

protected:
    bool created;

//methods
public:
    //constructors
    Window(HWND parent, const char *caption);
    virtual ~Window();

    bool Create(bool show = true);
    bool IsValid();

    static LRESULT CALLBACK MsgRouter(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
    virtual LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam) = 0;

    LRESULT SendMessage(UINT msg,WPARAM wParam,LPARAM lParam);
    int ShowWindow(int nCmdShow = SW_RESTORE);
    virtual void SetCaption(const char *caption);
    bool IsIconic();
    bool IsZoomed();
    bool IsVisible();
    bool IsWindowVisible();
    bool SetWindowPos(HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
    bool SetWindowPos(HWND hWndInsertAfter, RECT *rc, UINT uFlags);
    HWND SetFocus();
    void SetIcon(int resid);
    virtual bool CanClose();
};

//
// WFloatingList
//
class WList;
class WFloatingList : public Window
{
public:
    WFloatingList(HWND parent);
    ~WFloatingList();
    virtual LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);
    void Redraw();
    WList *list;
    bool mdown;
    int part, startx, starty;
    RECT startrc;
};

//
// MDIClient
//
class MDIClient : public Window
{
public:
    MDIClient(HWND parent);
    virtual LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);
};

//
// MDIChild
//
class MDIChild : public Window
{
public:
    MDIChild(HWND parent, const char *caption);
    static LRESULT CALLBACK MsgRouter(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
};


#define BORDER_X 1
#define BORDER_Y 1
#define FRAME_X 4
#define FRAME_Y 4
//
// TCWindow - Tiny Caption Window
//
class DockWindow;
class TCWindow : public MDIChild
{
public:
    TCWindow(HWND parent, const char *caption);

    virtual LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);

    bool ToggleAlwaysOnTop();
    virtual int DoNCHitTest(LPPOINT screenPt);
    void DoNCPaint();
    int DoNCCalcSize(LPNCCALCSIZE_PARAMS calcSize);
    virtual bool DoNCRButtonDown(int hitTest, LPPOINTS screenPt);
    virtual bool DoNCLButtonDown(int hitTest, LPPOINTS screenPt);
    virtual bool DoMouseMove();
    bool DoLButtonUp();
    bool DoNCActivate(bool active);
    bool DoCommand(UINT id, int notifyCode, int& result);
    bool DoSysCommand(int cmdType);
    void PaintButton(HDC hdc, LPRECT rc, bool pressed);
    void PaintSysBox(HDC hdc);
    void PaintMinBox(HDC hdc, bool pressed);
    void PaintMaxBox(HDC hdc, bool pressed);
    virtual void PaintCaption(bool active);
    void GetCaptionRect(LPRECT rc);
    void GetSysBoxRect(LPRECT rc);
    void GetMinBoxRect(LPRECT rc);
    void GetMaxBoxRect(LPRECT rc);
    void DoSysMenu();

    bool WaitingForSysCmd;
    int DownHit;
    bool IsPressed;
    DockWindow *docked;
    static HWND topmost;			//window set as topmost, or null
};

//dock window
class DockWindow : public TCWindow
{
public:
    enum CPOS {TL=1, BL=2, BR=4, TR=8};
    struct child
    {
        bool inuse;
        TCWindow *win;
        int pos;
        int oldstyle;
    };
    child children[4];
    POINT center;
    RECT wmsizerc;

    DockWindow(HWND parent, const char*caption);
    virtual LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);
    bool CanClose();
    void PositionWindows();
    bool Dock(TCWindow *win);
    bool UnDock(TCWindow *win);
};



//PaletteWindow
class PaletteWindow : public TCWindow
{
public:
    PaletteWindow(HWND parent, const char*caption);
    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);
    void Redraw();
};

//ImageView
class ImageView : public TCWindow
{
public:
    int width;
    int height;
    HBITMAP bmp;
    ImageView(HWND parent, const char*caption, unsigned char*img,int width, int height);
    ~ImageView();
    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);
    void Redraw();
    void ResetSize();
};
