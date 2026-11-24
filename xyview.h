#ifndef _XYVIEW_H_
#define _XYVIEW_H_

#define TIMER_SEL     1				// Timer 1 - selection cycling
#define TIMER_CLIP    2				// Timer 2 - Clip points
#define TIMER_FLY     3				// Timer 3 - flyover hints
#define TIMER_ADDCLIP 4				// Timer 4 - addclip drag delay

#define DOUBLECLICK 0x10000			//some mask number bigger than the max number of mice buttons...

#define SHIFT_FLAG   1
#define CTRL_FLAG    2
#define ALT_FLAG     4

#define KNOB_NONE 0
#define KNOB_DRAG 1
#define KNOB_SHEAR 2
#define KNOB_ROTATE 3
#define KNOB_EDGE 4

#define MOUSEHELPXY  980
//#define ALIGNBUTTON  981
#define ALIGNTOP     982
#define ALIGNVCENTER 983
#define ALIGNBOTTOM  984
#define ALIGNLEFT    985
#define ALIGNHCENTER 986
#define ALIGNRIGHT   987
#define XYMODEBUTTON 989
#define XYPRINT      990

#define HTGRIDBUTTON 300
#define HTSCALEBUTTON 310

#define GRID_LINES 0
#define GRID_DOTS 1

// 90-100
#define XYMOUSEBASE 950

struct mouseobject_t;

class mouseEventType
{
public:
    mouseEventType();
    ~mouseEventType();

    int count;
    char **moves;
    int whichMouse;
    unsigned char moveFlags;
    mouseEventType *next;
};

class TXYViewWindow : public TCWindow, public WindowPlacement
{
public:
    TXYViewWindow(HWND parent, char *title, int type);
    ~TXYViewWindow();

    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);

    HWND WP_GetHwnd();
    const char *WP_WindowName();

    void PrintMap();

    // From CameraWindow
    void FaceTransFunc(char *actionmsg,char *savemsg,int gridsign,int zsign);
    void BnTopUp();
    void BnTopDown();
    void BnBottomUp();
    void BnBottomDown();

    void UpdateWindows();
    // Rendering stuff...
    HDC        hmemdc;        // handle to device context for drawing
    HBITMAP    hmembitmap;  // memory bitmap of drawing
    HDC        hbasedc;        // base xy representation without clip points, cameras
    HBITMAP    hbasebitmap;	// base bitmap
    HDC        disposableDC;
    HBITMAP    hDisposeBM;
    HFONT camfont, gridfont;

    void UpdateSelected(bool copy_to_screen);

    void ConstructMemDC();
    void RemoveMemDC();

    int mousedown;
    bool ug;
    vec3_t startpt;
    float startAngle;
    vec3_t newpt;
    vec3_t delta, relative;
    RECT oldclip;

    int ViewType;
    int UAxis, VAxis, NormalAxis;

    vec3_t xy_viewnormal;
    int index;
    SIZE XYSize;
    RECT XYClip;

    void MouseHelp();
    void CmSwitchCamera();
    void CameraSelect(int num);
    void C1();
    void C2();
    void C3();
    void C4();
    void C5();
    void ToggleCameras();
    void ToggleCameraLock();
    void RedrawContents();
    void RedrawFromBase();
    void ClearContents();
    void XYTraverseUnselected(HDC);
    void XYTraverseSelected(HDC);

    void CmS();
    void CmG();
    void CmChangeViewNormal();

    void ConvertPoint(LPPOINT in, vec3_t out);
    void ConvertPoint(vec3_t in, LPPOINT out);
    void MoveEyePosition(vec3_t newpos,int lockValue);

    //  Added for Grid and Scale
    void        PaintGridBox(HDC);
    void        PaintScaleBox(HDC);
    void        GetGridBoxRect(LPRECT rc);
    void		GetScaleBoxRect(LPRECT rc);
    void        PaintCaption(bool active);

    int  DoNCHitTest(LPPOINT screenPt);
    bool DoNCRButtonDown(int hitTest, LPPOINTS screenPt);
    bool DoNCLButtonDown(int hitTest, LPPOINTS screenPt);

    void ParseMouse();
    void StartMouse(UINT modKeys, LPPOINT point, int whichMouse);
    void StopMouse(LPPOINT point, int whichMouse);
    int TranslateCommand(char *);

    void ShowLeaks();
    void ShowRegions();
    void ShowWorldMins();
    void snapPoint(LPPOINT in);
    void snapPoint(vec3_t in);
    void EvTimer(UINT timerId);
    void EvMouseMove(UINT modkeys, LPPOINT location);
    void DrawGrid();
    bool CanClose();
    void DrawCameras(HDC hdc);
    void DrawZeroCrosshair(HDC hdc);
    void DrawEye(HDC hdc,int sx, int sy);
    void SetCaption();

    void dragFrom(vec3_t location,bool ug,int callback);

    void RotateCallback(float a1, float a2);
    void DragCallback(float dx, float dy);
    //void CameraDragCallback(float dx, float dy);
    void DirectionCallback(float dx, float dy);
    void NewCallback (float dx, float dy);
    void ControlCallback (float dx, float dy);
    void MultiControlCallback (float dx, float dy);
    void SelCallback (float dx, float dy, int /*type*/);

    void KnobCallback(float dx, float dy);
    bool knobDrag(vec3_t tp);
    bool objectClick(vec3_t tp);        // Always true?

    bool newBrushDragFrom(vec3_t tp);   // always TRUE
    bool selBrushDragFrom(vec3_t tp);   // always TRUE
    bool planeDragFrom(vec3_t tp);
    bool planeDragFromMulti(vec3_t tp, int);
    bool shearDragFrom(vec3_t tp);
    bool currentFaceDragFrom();   // no point needed...
    bool vertexDragFrom(vec3_t tp);
    bool vertexRotate(vec3_t tp);
    bool cameraDragFrom(vec3_t tp);  // always TRUE?
    bool selectionDragFrom(vec3_t pt); // always TRUE
    bool directionDragFrom(vec3_t tp); // always TRUE

    bool edgeDragFrom(vec3_t tp);

    //    TButton *align;
    //void AlignButton();
    void AlignTop();
    void AlignVCenter();
    void AlignBottom();
    void AlignLeft();
    void AlignHCenter();
    void AlignRight();
    void CmAlign(int);

    void NewMouse(int);
    // The User Mouse Settings...
    void M1();
    void M2();
    void M3();
    void M4();
    void M5();
    void M6();
    void M7();
    void M8();
    void M9();
    void M10();

    void TranslateFlyOver(mouseobject_t& flyover, UINT modkeys);
};

#endif	//_XYVIEW_H_
