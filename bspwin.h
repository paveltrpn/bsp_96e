#ifndef _BSPWIN_H_
#define _BSPWIN_H_


class TTextureComboBox : public WCombo
{
public:
    TTextureComboBox(HWND parent, int id, int x, int y, int w, int h, UINT textLimit);
    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);

    void SetupWindow();
    LRESULT EvCompareItem(UINT, COMPAREITEMSTRUCT*);
    void EvDrawEdit(UINT, DRAWITEMSTRUCT*);
    void EvDrawItem(UINT, DRAWITEMSTRUCT*);
    void EvMeasureItem(UINT, MEASUREITEMSTRUCT*);
    void EvCBNSelChange();
};

#define IDC_MAPGADGET 737
#define IDC_MAPGADGETBASE 738

class ListBoxData;
class TBSPApp : public Window
{
public:
    TBSPApp(const char *title);  // constructor
    ~TBSPApp();

    LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

    void EvActivateApp(bool active, HANDLE threadId);  // Called when BSP gains, loses active status.

    void SetupWindow();
    void EvCBNSelChange();
    void EvClose();
    void SetCaption(const char * title);
    void ResizeFrame();

    TTextureComboBox *TextureList;
    WCombo *makeBox;
    WButton *makeBtn;
    ListBoxData *data;

    void ConstructControlBars();
    void ParseControl(int bar, char *control);
    int numbars;

    void EvSize(UINT sizeType, SIZE& size);
    void EvLButtonDown(UINT modKeys, POINT& point);
};

//
// Splash Window Shown at BSP Startup
//
class TBSPSplashWindow : public Window
{
public:
    TBSPSplashWindow(int resdib);
    ~TBSPSplashWindow();
    LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

    void SetupWindow();
    void SetText(char *str);

    WStatic *caption;
    int resdib;
    int bmpw, bmph;
    HBITMAP bitmap;
};

//
// TBSPWindow - MDIClient
//

class TBSPWindow : public MDIClient
{
//	friend TEditDlg;
    friend class TTextureWindow;
    friend class TGroupWindow;
    friend class map;


public:
    TBSPWindow(HWND parent);  // constructor
    ~TBSPWindow();

//	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

    bool itemsAdded;
    HMENU holdMenu;
    int  numExporters;
    char *ExporterNames[MAX_EXPORTERS];

    void PopulateMake();
    void AutoLoad(char *fname);
    bool IdleAction(long idleCount);
    void SetupWindow();
    void SetWindowPositions();
    void ShowAllWindows();
    void HideAllWindows();

public:
    TCopyTransferBuffer copyxfer;
    TRotateTransferBuffer rotatexfer;
    TStairTransferBuffer stairxfer;
    TArchTransferBuffer archxfer;
    TWedgeTransferBuffer wedgexfer;
    TLightTransferBuffer lightxfer;
    TSphereTransferBuffer spherexfer;
    TCylTransferBuffer cylxfer;
    TNTransferBuffer nxfer;
    TScaleTransferBuffer scalexfer;
    TPyramidTransferBuffer pyramidxfer;
    TCreateTransferBuffer createxfer;
    TRevolveTransferBuffer revolvexfer;
    TExportTransferBuffer exportxfer;
    TMergeTransferBuffer mergexfer;
    TSelectTransferBuffer selectxfer;
    TSetHeightsTransferBuffer setxfer;
};

#endif //_BSPWIN_H_
