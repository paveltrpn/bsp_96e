#ifndef _EDITWIN_H_
#define _EDITWIN_H_

#define SHIFT_TEXTURE 1
#define STRETCH_TEXTURE 2
#define ROTATE_TEXTURE 3
#define LIFT_TEXTURE 24
#define LIFT_APPLY 25
#define CLICKFACEDRAG 26
#define CLICKFACESHEAR 27
#define DRAGBRUSHES 28
#define DRAGBRUSHESSIDE 29
#define SETCLIPPER 30
#define SETCLIPPERINV 31
#define NAVIGATE 32
#define EDGEDRAG 33
#define MLOOK 40
#define NAVIGATE_STRAFE 34

#define EDITMENU_NO_ANGLE_CONTROL	511		// menu message id
#define EDITMENU_YAW_CONTROL		513		// menu message id
#define EDITMENU_PITCH_CONTROL		514		// menu message id
#define EDITMENU_FOV_CONTROL		515		// menu message id
#define ANGLE_CONTROL				512		// id for mousedown
#define NO_ANGLE_CONTROL			  0
#define YAW_CONTROL					  1
#define PITCH_CONTROL				  2
#define FOV_CONTROL					  3


#define TIMEREFRESH 2399
#define HELPBUTTON 2400
#define NUMPORTALS 2401
#define STOPPORTALS 2402
#define CENTERKNOBS 2403
#define MODEBUTTON 2404
#define CONNECTIONBUTTON 2405

// 105-115
#define MOUSEBASE 2406
#define MODEBASE 2426
class map;
class SetBrush;

class TEditWindow : public TCWindow, public WindowPlacement
{

public:
    TEditWindow(HWND parent, char *title);
    ~TEditWindow();

    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);

    HWND WP_GetHwnd();
    const char *WP_WindowName();

    // GLBSP Stuff
    int font_list;
    HDC   ghDC;
    HGLRC ghRC;
    void resize();
    void initializeGL();
    void drawScene();
    bool bSetupPixelFormat(HDC hdc);
    void getRenBrush(int clickX, int clickY);
    void mapRenderNumber(map *m);
    void setRenderBrush(map *m, int hitIndex);
    void glRenderScene(map *m);
    bool glClickOnEdgeKnob(int clickX, int clickY);
    bool glPointOnEdgeKnob(int clickX, int clickY);

    // Rendering stuff...
    HDC         hmemdc;        // handle to device context for drawing
    HBITMAP hmembitmap;			// memory bitmap of drawing
    void ConstructMemDC();
    void RemoveMemDC();
    void CalcBltRects(int scx, int scy, int rx, int ry, RECT*dst, RECT*src);

    float *zbuffer;
    unsigned char *imagebuffer;
    float *t_zbuffer;
    unsigned char *t_imagebuffer;

    void ConstructBuffers(int w, int h);
    void RemoveBuffers();
    void StoreBuffers();
    void RestoreBuffers();
    void UpdateSelected();
    void UpdateSelectedMap(map *m);
    void UpdateOtherViews();
    void SyncAutoRender();
    void SetOrientControl(int ctrl_mode);

    bool show_connections;

    void MouseHelp();

    void TimeRefresh();

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

    LRESULT MouseWheel(WPARAM wParam, LPARAM lParam);

    void RedrawBrush(SetBrush *b);
    void RedrawBrushZ();
    void RedrawContents();
    void RedrawContentsMap(map *m);
    void ClearContents();
    void SetCaption(const char* title);
    float s, t, r, sx, sy;

    POINT startpt;
    POINT newpt;
    POINT delta, relative, oldrelative;
    int mousedown;
    int modal;
    bool ug;

    // Modes...
    void MNormal();
    void MSelect();
    void MSelectOne();
    void MShift();
    void MStretch();
    void MRotate();
    void MApplyToBrush();
    void MApplyToFace();
    void MLift();
    void MLiftApply();
    void MDragFace();
    void MShearFace();
    void MDragBrushes();
    void MDragBrushesSide();
    void MSetClipper();
    void MSetClipperInv();
    void MNavigate();
    void MNavigateStrafe();
    void MEdgeDrag();
    void MLook();

    WButton *numPortals;
    int maxPortals;
    WButton *killPortals;

    void SetNumPortals();
    void KillPortals();

    BITMAPINFOHEADER m_pBmInfoHeader;
    BITMAPINFO *m_pBmInfo;

//
private:
    void ParseMouse();
    void StartMouse(UINT modKeys, POINT *point, int whichMouse);
    void StopMouse(POINT *point, int whichMouse);
    int TranslateCommand(char *);

    void ShowLeaks();
    void ShowPortals();
    void ToggleConnections();
    void ToggleCenterKnobs();
    void DrawAxes();
    void glDrawAxes();
    void glDraw3DGrid();

    void SetupWindow();
    void Paint(HDC hdc);
    bool CanClose();
    void EvMouseMove(UINT modKeys, POINT* point);

    bool currentFaceDragFrom();   // no point needed...
    bool shearCurrentFace();
    bool selectionDragFrom();
    bool selectionDragFromSide();
    bool currentEdgeDragFrom();

    void SetYaw(POINT *point);
    void SetPitch(POINT *point);
    void SetFov(POINT *point);

    bool mouseOnEdgeKnob(int m_x, int m_y);

    bool DoNCRButtonDown(UINT, POINT screenPt);
};

#endif	//_EDITWIN_H_
