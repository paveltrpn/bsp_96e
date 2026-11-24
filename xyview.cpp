#include "global.h"

mouseobject_t flyover;
int knob_mode = KNOB_NONE;

bool alreadySelecting = false;
mouseEventType *mouse_events[MAX_MOUSE_EVENT_TYPES];
char EventTypes[MAX_MOUSE_EVENT_TYPES] [32];
int numEventTypes = 0;
int curEventType = 0;
bool MouseParsed = false;
vec3_t xy_eye;

bool glBspRotate = false;

POINT* movingCamStart;
float xydrag_offset_U, xydrag_offset_V;
vec3_t xydrag_eye;

map *rotateMap = NULL;
vec3_t rpy;

int eyetomove;

static vec3_t oldrelative;
float		xy_viewdist = -1024;		// clip behind this plane
vec3_t		direction;
SetBrush	*newbrush = NULL;
SetBrush	*selBrush;
vec3_t		neworg, newdrag;



int GetScaleText(char *in)
{
    if ((set.scale < 0.25) && (fabs((1.0/set.scale) - (int)(1.0/set.scale)) < 0.01))
        return sprintf(in,"1/%i",(int)(1.0/set.scale));
    else
        return sprintf(in,"%5.1f%",(set.scale*100));
}



LRESULT TXYViewWindow::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_CREATE:
    {
        ConstructMemDC();

        SetTimer(hwnd,TIMER_FLY,200,0);
    }
    break;

    case WM_COMMAND:

        switch(wParam)
        {
        case TB_SB:
            CmS();
            return 0;
        case TB_GB:
            CmG();
            return 0;
        case TB_CV:
            CmChangeViewNormal();
            return 0;
        case MOUSEHELPXY:
            MouseHelp();
            return 0;
        case XY_1:
            C1();
            return 0;
        case XY_2:
            C2();
            return 0;
        case XY_3:
            C3();
            return 0;
        case XY_4:
            C4();
            return 0;
        case XY_5:
            C5();
            return 0;
        case XY_6:
            ToggleCameras();
            return 0;
        case XY_LOCK:
            ToggleCameraLock();
            return 0;
        case ALIGNTOP:
            AlignTop();
            return 0;
        case ALIGNVCENTER:
            AlignVCenter();
            return 0;
        case ALIGNBOTTOM:
            AlignBottom();
            return 0;
        case ALIGNLEFT:
            AlignLeft();
            return 0;
        case ALIGNHCENTER:
            AlignHCenter();
            return 0;
        case ALIGNRIGHT:
            AlignRight();
            return 0;
        case XYMOUSEBASE+0:
            M1();
            return 0;
        case XYMOUSEBASE+1:
            M2();
            return 0;
        case XYMOUSEBASE+2:
            M3();
            return 0;
        case XYMOUSEBASE+3:
            M4();
            return 0;
        case XYMOUSEBASE+4:
            M5();
            return 0;
        case XYMOUSEBASE+5:
            M6();
            return 0;
        case XYMOUSEBASE+6:
            M7();
            return 0;
        case XYMOUSEBASE+7:
            M8();
            return 0;
        case XYMOUSEBASE+8:
            M9();
            return 0;
        case XYMOUSEBASE+9:
            M10();
            return 0;

        case IDTB_TU:
            BnTopUp();
            return 0;
        case IDTB_TD:
            BnTopDown();
            return 0;
        case IDTB_BU:
            BnBottomUp();
            return 0;
        case IDTB_BD:
            BnBottomDown();
            return 0;

        case XYPRINT:
            PrintMap();
            return 0;
        }
        break;
    case WM_ERASEBKGND:
        return 0;

    case WM_PAINT:
    {
        if (IsIconic() || !set.Map_Read)
            break;

        RedrawContents();
    }
    break;

    case WM_TIMER:
        EvTimer(wParam);
        return 0;

    case WM_SIZE:
    {
        int x = (short)LOWORD(lParam);
        int y = (short)HIWORD(lParam);
        RemoveMemDC();
        XYSize.cx = x;
        XYSize.cy = y;
        XYClip.left = 0;
        XYClip.top = 0;
        XYClip.right = x;
        XYClip.bottom = y;
        ConstructMemDC(); // new size

        RedrawContents(); // force a redraw
    }
    break;
    case WM_SETFOCUS:

        set.curxy = index;
        break;

    case WM_MOUSEMOVE:
    {
        if(set.use_crosshair)
            SetCursor(LoadCursor(0,IDC_CROSS));

        MAKEPOINT(pt, lParam);
        EvMouseMove(wParam,&pt);
    }
    break;
    case WM_MOUSEWHEEL:

        if(SetFocusUnderMouse(hwnd))
            return 0;

        if(wParam & MK_CONTROL)
        {
            if((short)HIWORD(wParam) >= 0)
            {
                GridUp();
            }
            else
            {
                GridDown();
            }
        }
        else
        {
            // this code zooms in or out centered on the mouse position.
            //calc is: get vector under mouse, zoom in, get new vector under mouse.
            // subtract new vector from eye. add old vector back to eye.
            POINT point;
            GetCursorPos(&point);
            ScreenToClient(hwnd, &point);
            vec3_t startpt;
            ConvertPoint(&point, startpt);

            if((short)HIWORD(wParam) >= 0)
            {
                ScaleUp();
            }
            else
            {
                ScaleDown();
            }

            vec3_t endpt;
            ConvertPoint(&point, endpt);

            xy_eye[UAxis] = xy_eye[UAxis] - endpt[0] + startpt[0];
            xy_eye[VAxis] = xy_eye[VAxis] - endpt[1] + startpt[1];
            MoveEyePosition(xy_eye,set.lock_cameras);
            set.redrawxy = 1;
            Show_Frame("Scale changed...",1);
        }
        return 0;
        ////////////////
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_LBUTTONUP:
    case WM_XBUTTONUP:
    {
        int btn;
        if(msg == WM_LBUTTONUP)
            btn = 0;
        else if(msg == WM_MBUTTONUP)
            btn = 1;
        else if(msg == WM_RBUTTONUP)
            btn = 2;
        else if(msg == WM_XBUTTONUP)
        {
            if(wParam & MK_XBUTTON1)
                btn = 3;
            else
                btn = 4;
        }
        MAKEPOINT(pt, lParam);
        StopMouse(&pt, btn);
        Ccmd_UpdateEnablers();	//BspCmd->UpdateEnablers();
    }
    break;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDOWN:
    {
        //focus clicked window... useful for clippers which depend on current window being active
        if (GetFocus() != hwnd)
        {
            SetWindowPos(0,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE); // Make me the focus window...
            SetFocus();
        }

        int btn;
        if(msg == WM_LBUTTONDOWN)
            btn = 0;
        else if(msg == WM_MBUTTONDOWN)
            btn = 1;
        else if(msg == WM_RBUTTONDOWN)
            btn = 2;
        else if(msg == WM_XBUTTONDOWN)
        {
            if(wParam & MK_XBUTTON1)
                btn = 3;
            else
                btn = 4;
        }

        MAKEPOINT(pt, lParam);
        StartMouse(wParam, &pt, btn);
    }
    break;
    /////////////////////

    case WM_LBUTTONDBLCLK:
    {
        MAKEPOINT(pt, lParam);
        StartMouse(wParam, &pt, 0 | DOUBLECLICK);
    }
    break;
    }
    return TCWindow::WndProc(msg,wParam,lParam);
}

TXYViewWindow::TXYViewWindow(HWND parent, char *title, int type)
    : TCWindow(parent,title)
{
    SetRect(&XYClip, ORIG_XMIN , ORIG_YMIN, ORIG_XMAX, ORIG_YMAX);
    XYSize.cx = ORIG_HORIZ;
    XYSize.cy = ORIG_VERT;

    attr.width = XYSize.cx;    // Set initial width and height...
    attr.height = XYSize.cy;
    attr.dwStyle |= CS_OWNDC;

    memset(&flyover,0,sizeof(mouseobject_t));

    // parse in mouse event handlers...
    ParseMouse();

    //m_hPal = NULL;
    // Attr.AccelTable = NO_ACCEL;
    hmemdc = 0;
    hmembitmap = NULL;
    disposableDC = NULL;
    hDisposeBM = NULL;

    ViewType = type;

    GetAxes(ViewType, &UAxis, &VAxis, &NormalAxis);

    xy_viewnormal[UAxis] = 0.0;
    xy_viewnormal[VAxis] = 0.0;
    xy_viewnormal[NormalAxis] = -1.0;

    xy_eye[0] = xy_eye[1] = xy_eye[2] = 0.0;
    MoveEyePosition(xy_eye,set.lock_cameras);

    Window::SetCaption(title);

    gridfont = GetFont("small fonts",14);

    mousedown = NO_MOUSE;
}

TXYViewWindow::~TXYViewWindow()
{
    KillTimer(hwnd,TIMER_FLY);

    RemoveMemDC();
    DeleteObject(gridfont);
    mouseEventType *cur, *nxt;
    for (int i = 0; i < numEventTypes; i++)
    {
        for (cur = mouse_events[i]; cur != NULL; cur = nxt)
        {
            nxt = cur->next;
            delete cur;
        }
        mouse_events[i] = NULL;
    }
}

//WindowPlacement
HWND TXYViewWindow::WP_GetHwnd()
{
    return hwnd;
}
const char *TXYViewWindow::WP_WindowName()
{
    if(ViewType == Type_XY)
        return "XY_Top";
    else if(ViewType == Type_XZ)
        return "XY_Front";
    else if(ViewType == Type_YZ)
        return "XY_Right";
    else
        return "XY";
}

//get the rest of PrintMap() code from old source archives!
void TXYViewWindow::PrintMap()
{
    ::MessageBox(hwnd,"Print function not yet enabled, sorry...","BSP Print",MB_OK);
    return;
}



void TXYViewWindow::DrawCameras(HDC hdc)
{
    map *m = map_i[set.curmap];

    float xofs = float(XYSize.cx/2) + 0.5f;
    float yofs = float(XYSize.cy/2) + 0.5f;

    float u = xy_eye[UAxis];
    float v = xy_eye[VAxis];

    POINT pt;

    HGDIOBJ oldfont = SelectObject(hdc,set.font12);

    HPEN pencur = CreatePen(PS_SOLID,1,set.color_activecamera);
    HPEN pen_other = CreatePen(PS_SOLID,1,set.color_othercamera);
    HGDIOBJ oldpen = SelectObject(hdc,pencur);	//just to save the old pen

    HBRUSH brush = CreateSolidBrush(set.color_background);
    HGDIOBJ oldbrush = SelectObject(hdc,brush);

    SetBkMode(hdc,TRANSPARENT);
    SetTextAlign(hdc,TA_LEFT & TA_TOP);
    SetTextColor(hdc,set.color_gridlabel);

    char str[80];
    for (int i = MAX_CAMERAS-1; i >= 0; i--)
    {
        pt.x = int( (m->eye[i][UAxis]-u)*set.scale+xofs);
        pt.y = XYSize.cy - int((m->eye[i][VAxis]-v)*set.scale+yofs);

        float use_scale = set.scale;
        if (use_scale > 1.0) use_scale = 1.0;
        if (use_scale < 0.5) use_scale = 0.5;

        RECT box;
        box.left   = (int)(pt.x - use_scale*(float)set.camera_size/2.0);
        box.right  = (int)(box.left + use_scale*(float)set.camera_size);
        box.top    = (int)(pt.y - use_scale*(float)set.camera_size/2.0);
        box.bottom = (int)(box.top + use_scale*(float)set.camera_size);

        if (i == m->cureye)
            SelectObject(hdc,pencur);
        else
            SelectObject(hdc,pen_other);

        Rectangle(hdc,box.left,box.top,box.right,box.bottom);

        // only for top down views... draw angle arrow and dot lines
        if (ViewType == Type_XY)
        {
            float ang = -m->angles[i].yaw+M_PI/2.0f;
            float size = use_scale*set.camera_size;
            vec3_t mid, end, end2, s1, s2;
            mid[0] = (float) pt.x;
            mid[1] = (float) pt.y;

            mid[0] = mid[0] - (0.3f*size)*cos(ang);
            mid[1] = mid[1] - (0.3f*size)*sin(ang);

            end[0] = mid[0] + (1.0f*size)*cos(ang);
            end[1] = mid[1] + (1.0f*size)*sin(ang);

            end2[0] = mid[0] + (0.8f*size)*cos(ang);
            end2[1] = mid[1] + (0.8f*size)*sin(ang);

            s1[0] = mid[0] + (0.6f*size)*cos(ang+0.2f);
            s1[1] = mid[1] + (0.6f*size)*sin(ang+0.2f);

            s2[0] = mid[0] + (0.6f*size)*cos(ang-0.2f);
            s2[1] = mid[1] + (0.6f*size)*sin(ang-0.2f);

            MoveToEx(hdc, (int) mid[0], (int) mid[1], 0);
            LineTo(hdc, (int) end2[0], (int) end2[1]);

            LineTo(hdc, (int) s1[0], (int) s1[1]);
            LineTo(hdc, (int) end[0], (int) end[1]);
            LineTo(hdc, (int) s2[0], (int) s2[1]);
            LineTo(hdc, (int) end2[0], (int) end2[1]);

            float endstep = 0.4f * size;

            if (set.show_camera_angle)
            {
                for (int k = 0; k < set.camera_angle_length; k++)
                {
                    end[0] += endstep * cos(ang);
                    end[1] += endstep * sin(ang);

                    if (i == m->cureye)
                        SetPixelV(hdc,(int) end[0],(int) end[1],set.color_othercamera);
                    else
                        SetPixelV(hdc,(int) end[0],(int) end[1],set.color_activecamera);
                }
            }
        }
        else if (m->cureye == i && ViewType == Type_XZ)
        {
            float angx = m->angles[i].yaw;
            float angy = -m->angles[i].pitch;
            float size = use_scale*set.camera_size;
            vec3_t mid, end, end2, s1, s2;
            mid[0] = (float) pt.x;
            mid[1] = (float) pt.y;

            mid[0] = mid[0] - (0.3f*size)*sin(angx);
            mid[1] = mid[1] - (0.3f*size)*sin(angy);

            end[0] = mid[0] + (1.0f*size)*sin(angx);
            end[1] = mid[1] + (1.0f*size)*sin(angy);

            end2[0] = mid[0] + (0.8f*size)*sin(angx);
            end2[1] = mid[1] + (0.8f*size)*sin(angy);

            s1[0] = mid[0] + (0.6f*size)*sin(angx+0.2f);
            s1[1] = mid[1] + (0.6f*size)*sin(angy+0.2f);

            s2[0] = mid[0] + (0.6f*size)*sin(angx-0.2f);
            s2[1] = mid[1] + (0.6f*size)*sin(angy-0.2f);

            MoveToEx(hdc, (int) mid[0], (int) mid[1], 0);
            LineTo(hdc, (int) end2[0], (int) end2[1]);

            LineTo(hdc, (int) s1[0], (int) s1[1]);
            LineTo(hdc, (int) end[0], (int) end[1]);
            LineTo(hdc, (int) s2[0], (int) s2[1]);
            LineTo(hdc, (int) end2[0], (int) end2[1]);

            float endstep = 0.4f * size;

            if (set.show_camera_angle)
            {
                for (int k = 0; k < set.camera_angle_length; k++)
                {
                    end[0] += endstep * sin(angx);
                    end[1] += endstep * sin(angy);
                    SetPixelV(hdc,(int) end[0],(int) end[1],set.color_othercamera);
                }
            }
        }
        else if (m->cureye == i && ViewType == Type_YZ)
        {
            float angx = m->angles[i].yaw;
            float angy = -m->angles[i].pitch;
            float size = use_scale*set.camera_size;
            vec3_t mid, end, end2, s1, s2;
            mid[0] = (float) pt.x;
            mid[1] = (float) pt.y;

            mid[0] = mid[0] - (0.3f*size)*-cos(angx);
            mid[1] = mid[1] - (0.3f*size)*sin(angy);

            end[0] = mid[0] + (1.0f*size)*-cos(angx);
            end[1] = mid[1] + (1.0f*size)*sin(angy);

            end2[0] = mid[0] + (0.8f*size)*-cos(angx);
            end2[1] = mid[1] + (0.8f*size)*sin(angy);

            s1[0] = mid[0] + (0.6f*size)*-cos(angx+0.2f);
            s1[1] = mid[1] + (0.6f*size)*sin(angy+0.2f);

            s2[0] = mid[0] + (0.6f*size)*-cos(angx-0.2f);
            s2[1] = mid[1] + (0.6f*size)*sin(angy-0.2f);

            MoveToEx(hdc, (int) mid[0], (int) mid[1], 0);
            LineTo(hdc, (int) end2[0], (int) end2[1]);

            LineTo(hdc, (int) s1[0], (int) s1[1]);
            LineTo(hdc, (int) end[0], (int) end[1]);
            LineTo(hdc, (int) s2[0], (int) s2[1]);
            LineTo(hdc, (int) end2[0], (int) end2[1]);

            float endstep = 0.4f * size;

            if (set.show_camera_angle)
            {
                for (int k = 0; k < set.camera_angle_length; k++)
                {
                    end[0] += endstep * -cos(angx);
                    end[1] += endstep * sin(angy);
                    SetPixelV(hdc,(int) end[0],(int) end[1],set.color_othercamera);
                }
            }
        }
        //END TEST

        if (set.scale > 0.49)
        {
            int len = sprintf(str,"%i",i+1);
            TextOut(hdc,pt.x-3,pt.y-(set.camera_font_size),str,len);
        }
    }	//end: for each camera

    SelectObject(hdc,oldbrush);
    DeleteObject(brush);
    SelectObject(hdc,oldpen);
    DeleteObject(pencur);
    DeleteObject(pen_other);
    SelectObject(hdc,oldfont);
}

void TXYViewWindow::DrawZeroCrosshair(HDC hdc)
{
    float u = xy_eye[UAxis];	//distance from origin to eye
    float v = xy_eye[VAxis];
    float xofs = XYSize.cx/2 + 0.5f;
    float yofs = XYSize.cy/2 + 0.5f;

    POINT pt;
    pt.x = int( (-u)*set.scale + xofs);
    pt.y = XYSize.cy - int((-v)*set.scale + yofs);

    if(set.zero_crosshair == 2)
    {
        //draw xhair across entire xy
        HPEN pen = CreatePen(PS_SOLID,1,set.color_eyemarker);
        HGDIOBJ old = SelectObject(hdc,pen);
        int sz = (int) (set.world_minmax * set.scale);
        MoveToEx(hdc, pt.x-sz, pt.y, 0);
        LineTo(hdc,   pt.x+sz, pt.y);
        MoveToEx(hdc, pt.x,    pt.y-sz, 0);
        LineTo(hdc,   pt.x,    pt.y+sz);
        SelectObject(hdc,old);
        DeleteObject(pen);
    }
    else
    {
        //draw old style xhair
        if (PtInRect(&XYClip,pt))
        {
            HPEN pen = CreatePen(PS_SOLID,1,set.color_eyemarker);
            HGDIOBJ old = SelectObject(hdc,pen);
            MoveToEx(hdc, pt.x-5,pt.y, 0);
            LineTo(hdc,   pt.x+5,pt.y);
            MoveToEx(hdc, pt.x,  pt.y-5, 0);
            LineTo(hdc,   pt.x,  pt.y+5);
            SelectObject(hdc,old);
            DeleteObject(pen);
        }
    }
}
void TXYViewWindow::DrawEye(HDC hdc,int sx, int sy)
{
    RECT origin;
    origin.left = int(sx/2+0.5) - 2;
    origin.right = origin.left + 4;
    origin.top = int(sy/2+0.5) - 2;
    origin.bottom = origin.top + 4;
    HBRUSH eyeColor = CreateSolidBrush(set.color_eyemarker);
    FrameRect(hdc,&origin,eyeColor);
    DeleteObject(eyeColor);
}

//
// UpdateSelected
//
// redraw with "ghosts". updates onto hmemdc. set copy_to_screen to copy to screen
//
void TXYViewWindow::UpdateSelected(bool copy_to_screen)
{
    GdiFlush();

    if (IsIconic() || !set.Map_Read)
        return;
    if (!hmemdc || !hmembitmap || !disposableDC)
        return;

    RECT r;
    GetClientRect(hwnd,&r);
    int height = r.bottom-r.top;
    int width  = r.right-r.left;

    // copy from disposable
    BitBltRect(hmemdc,XYClip,disposableDC,0,0,SRCCOPY);

    SetBkColor(hmemdc,set.color_background);
    SetTextColor(hmemdc,set.color_foreground);

    XYTraverseSelected(hmemdc);
    map_i[set.curmap]->clipper_i->XYDrawSelf(this, hmemdc, true);

    // draw xy eye (at center of view)
    if (set.show_eye)
        DrawEye(hmemdc,width,height);

    // draw small crosshair on top of brushes
    if (set.zero_crosshair == 1)
        DrawZeroCrosshair(hmemdc);

    ShowLeaks();
    ShowRegions();

    // draw cameras
    if (set.show_cameras)
        DrawCameras(hmemdc);

    if (groupWindow)
    {
        int idx = groupWindow->GroupList->GetSelIndex();
        if (idx >= 0 && idx < groupWindow->GroupList->GetCount())
        {
            char str[MAX_PATH];
            sprintf(str,map_i[set.curmap]->groups->GetName(idx));
            status->SetText(STATUS_GROUP, str, true);
        }
    }

    // render to screen
    if(copy_to_screen)
    {
        HDC hdc = GetDC(hwnd);
        BitBltRect(hdc,XYClip,hmemdc,0,0,SRCCOPY);
        ReleaseDC(hwnd,hdc);
    }
}

void TXYViewWindow::RemoveMemDC()
{
    if (hmemdc)
        DeleteDC(hmemdc);
    if (hmembitmap)
        DeleteObject(hmembitmap);
    hmemdc = 0;
    hmembitmap = 0;

    if (hbasedc)
        DeleteDC(hbasedc);
    if (hbasebitmap)
        DeleteObject(hbasebitmap);
    hbasedc = 0;
    hbasebitmap = 0;

    if (disposableDC)
        DeleteObject(disposableDC);
    if (hDisposeBM)
        DeleteObject(hDisposeBM);
    disposableDC = 0;
    hDisposeBM = 0;
}

void TXYViewWindow::ConstructMemDC()
{
    HDC hdc = GetDC(hwnd);     // get a model DC
    // allocate the memory dc and the bitmap
    hmemdc = CreateCompatibleDC(hdc);
    hmembitmap = CreateCompatibleBitmap(hdc, XYSize.cx, XYSize.cy);
    SelectObject(hmemdc,hmembitmap);

    hbasedc = CreateCompatibleDC(hdc);
    hbasebitmap = CreateCompatibleBitmap(hdc, XYSize.cx, XYSize.cy);
    SelectObject(hbasedc,hbasebitmap);

    disposableDC = CreateCompatibleDC(hmemdc); // create compatible one...
    hDisposeBM = CreateCompatibleBitmap(hdc, XYSize.cx, XYSize.cy);

    SelectObject(disposableDC,hDisposeBM);

    ReleaseDC(hwnd,hdc);
}

void TXYViewWindow::MoveEyePosition(float *newpos,int lockValue)
{
    if (!lockValue)
        return;
    if (!set.Map_Read || !map_i || ! map_i[set.curmap])
        return;

    map *m = map_i[set.curmap];
    //set.redrawedit = 1;

    VectorCopy(newpos,m->eye[m->cureye]);
    VectorAdd(m->eye[m->cureye], set.lock_camera_offset, m->eye[m->cureye]);

    m->lookAtSelection();
}

void TXYViewWindow::NewMouse(int mm)
{
    if ((mm < 0) || (mm >= numEventTypes))
        return;

    curEventType = mm;
}

void TXYViewWindow::M1()
{
    NewMouse(0);
}
void TXYViewWindow::M2()
{
    NewMouse(1);
}
void TXYViewWindow::M3()
{
    NewMouse(2);
}
void TXYViewWindow::M4()
{
    NewMouse(3);
}
void TXYViewWindow::M5()
{
    NewMouse(4);
}
void TXYViewWindow::M6()
{
    NewMouse(5);
}
void TXYViewWindow::M7()
{
    NewMouse(6);
}
void TXYViewWindow::M8()
{
    NewMouse(7);
}
void TXYViewWindow::M9()
{
    NewMouse(8);
}
void TXYViewWindow::M10()
{
    NewMouse(9);
}

// show mouse config
void TXYViewWindow::MouseHelp()
{
    BSPHelp(const_cast<char *> ("XY Mouse Settings..."), const_cast<char *> ("Current XY Window Mouse Settings"));
    BSPHelpAddV(const_cast<char *> ("\r\n%s\r\n--------------------------------------------------------------\r\n"),
                EventTypes[curEventType]);

    mouseEventType *p = mouse_events[curEventType];

    int counter = 1;
    int kk;
    while (p)
    {
        static char* buttons[] = { const_cast<char *> ("Left"),
                                   const_cast<char *> ("Middle"),
                                   const_cast<char *> ("Right"),
                                   const_cast<char *> ("X1"),
                                   const_cast<char *> ("X2")
                                 };
        char *which = buttons[p->whichMouse];

        char keys[40];
        int fl = 1;
        if ((p->moveFlags & CTRL_FLAG) && (p->moveFlags & SHIFT_FLAG) && (p->moveFlags & ALT_FLAG))
        {
            sprintf(keys,"Ctrl+Alt+Shift");
        }
        else if ((p->moveFlags & SHIFT_FLAG) && (p->moveFlags & CTRL_FLAG))
        {
            sprintf(keys,"Ctrl+Shift");
        }
        else if ((p->moveFlags & CTRL_FLAG) && (p->moveFlags & ALT_FLAG))
        {
            sprintf(keys,"Ctrl+Alt");
        }
        else if ((p->moveFlags & SHIFT_FLAG) && (p->moveFlags & ALT_FLAG))
        {
            sprintf(keys,"Shift+Alt");
        }
        else if (p->moveFlags & SHIFT_FLAG)
        {
            sprintf(keys,"Shift");
        }
        else if (p->moveFlags & CTRL_FLAG)
        {
            sprintf(keys,"Ctrl");
        }
        else if (p->moveFlags & ALT_FLAG)
        {
            sprintf(keys,"Alt");
        }
        else
        {
            fl = 0;
        }

        if (fl)
        {
            BSPHelpAddV(const_cast<char *> ("%i\t%s %s Click:\r\n"),counter,keys,which);
        }
        else
        {
            BSPHelpAddV(const_cast<char *> ("%i\t%s Click:\r\n"),counter,which);
        }

        for (kk = 0; kk < p->count; kk++)
        {
            BSPHelpAddV(const_cast<char *> ("\t%s\r\n"),p->moves[kk]);
        }
        BSPHelpAdd(const_cast<char *> ("\r\n"));

        p = p->next;
        counter++;
    }
}

void TXYViewWindow::AlignTop()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->saveForUndo(const_cast<char *> ("Align Tops"),UNDO_BRUSHES);

    CmAlign(ALIGNTOP);

    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Tops Aligned",true);
}

void TXYViewWindow::AlignVCenter()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->saveForUndo(const_cast<char *> ("Align Vertical Centers"),UNDO_BRUSHES);

    CmAlign(ALIGNVCENTER);

    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Vertical Centers Aligned",true);
}

void TXYViewWindow::AlignBottom()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->saveForUndo(const_cast<char *> ("Align Bottoms"),UNDO_BRUSHES);

    CmAlign(ALIGNBOTTOM);

    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Bottoms Aligned",true);
}

void TXYViewWindow::AlignLeft()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->saveForUndo(const_cast<char *> ("Align Left Sides"),UNDO_BRUSHES);

    CmAlign(ALIGNLEFT);

    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Left Sides Aligned",true);
}

void TXYViewWindow::AlignHCenter()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->saveForUndo(const_cast<char *> ("Align Horizontal Centers"),UNDO_BRUSHES);

    CmAlign(ALIGNHCENTER);

    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Horizontal Centers Aligned",true);
}

void TXYViewWindow::AlignRight()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->saveForUndo(const_cast<char *> ("Align Right Sides"),UNDO_BRUSHES);

    CmAlign(ALIGNRIGHT);

    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Right Sides Aligned",true);
}

void TXYViewWindow::CmAlign(int alignType)
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];

    sb_mins[0] = sb_mins[1] = sb_mins[2] = MAX_NUM;
    sb_maxs[0] = sb_maxs[1] = sb_maxs[2] = -MAX_NUM;

    m->makeSelectedPerform(SEL_ADDTOBBOX);

    // no snap to grid...
    vec3_t sel_org;
    sel_org[0] = (sb_mins[0] + sb_maxs[0])/2;
    sel_org[1] = (sb_mins[1] + sb_maxs[1])/2;
    sel_org[2] = (sb_mins[2] + sb_maxs[2])/2;


    // calc bbox
    // set up sb_
    // for each selected,
    //  get the center
    //  figure out the movement
    // lock textures?
    // move it

    m->dirty = 1;

    for (Entity *ent = m->objects.p_next; ent != &m->objects; ent = ent->p_next)
    {
        for (SetBrush *brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            if (!brush->IsDrawable())
                continue;

            if (!brush->IsSelected())
                continue;

            sb_translate[0] = 0.0f;
            sb_translate[1] = 0.0f;
            sb_translate[2] = 0.0f;

            switch (alignType)
            {
                // v axis
            case ALIGNTOP:
                sb_translate[VAxis] = sb_maxs[VAxis] - brush->bmaxs[VAxis];
                break;
            case ALIGNVCENTER:
                sb_translate[VAxis] = sel_org[VAxis] - brush->bctr[VAxis];
                break;
            case ALIGNBOTTOM:
                sb_translate[VAxis] = sb_mins[VAxis] - brush->bmins[VAxis];
                break;
                // u axis
            case ALIGNLEFT:
                sb_translate[UAxis] = sb_mins[UAxis] - brush->bmins[UAxis];
                break;
            case ALIGNHCENTER:
                sb_translate[UAxis] = sel_org[UAxis] - brush->bctr[UAxis];
                break;
            case ALIGNRIGHT:
                sb_translate[UAxis] = sb_maxs[UAxis] - brush->bmaxs[UAxis];
                break;
            }
            brush->translate();
            // process it
        }
    }
}

void TXYViewWindow::CmSwitchCamera()
{
    if (!set.Map_Read || !map_i || !map_i[set.curmap])
        return;

    Popup PopupMenu;

    PopupMenu.Add("Camera 1", XY_1);
    PopupMenu.Add("Camera 2", XY_2);
    PopupMenu.Add("Camera 3", XY_3);
    PopupMenu.Add("Camera 4", XY_4);
    PopupMenu.Add("Camera 5", XY_5);
    PopupMenu.Separator();
    if (set.show_cameras)
    {
        PopupMenu.Add("Hide Cameras...", XY_6);
    }
    else
    {
        PopupMenu.Add("Show Cameras...", XY_6);
    }
    PopupMenu.Separator();
    if (set.lock_cameras)
    {
        PopupMenu.Add("Unlock Cameras...", XY_LOCK);
    }
    else
    {
        PopupMenu.Add("Lock Cameras...", XY_LOCK);
    }

    RECT rect;

    GetClientRect(hwnd,&rect);
    PopupMenu.pt.x = rect.left+110;
    PopupMenu.pt.y = rect.top+13;

    ClientToScreen(hwnd,&PopupMenu.pt);
    PopupMenu.Show(hwnd);
    DoNCPaint();
}

void TXYViewWindow::CameraSelect(int num)
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    m->cureye = num - 1;

    VectorCopy(m->eye[m->cureye],xy_eye);
    MoveEyePosition(xy_eye,set.lock_cameras);

    set.redrawxy = 1;
    set.redrawedit = 1;
    char outstr[80];
    sprintf(outstr,"Camera %d Active",num);
    Show_Frame(outstr,true);
}

void TXYViewWindow::C1()
{
    CameraSelect(1);
}
void TXYViewWindow::C2()
{
    CameraSelect(2);
}
void TXYViewWindow::C3()
{
    CameraSelect(3);
}
void TXYViewWindow::C4()
{
    CameraSelect(4);
}
void TXYViewWindow::C5()
{
    CameraSelect(5);
}
void TXYViewWindow::ToggleCameras()
{
    if (!set.Map_Read)
        return;

    set.show_cameras ^= 1;
    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Toggled Camera Display and Tracking",true);
}

void TXYViewWindow::ToggleCameraLock()
{
    if (!set.Map_Read)
        return;

    // probably should synchronize the eyes right away... but no...
    set.lock_cameras ^= 1;
    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Toggled Camera Lock for 3D and XY Views",true);
};

void TXYViewWindow::ShowLeaks()
{
    if (!set.Map_Read || !set.track_leaks)
        return;
    if (numLeakPoints <= 0 || !aLeakPoints)
        return;

    POINT pt;
    HPEN pen = CreatePen(PS_SOLID,1,set.color_gridlabel);
    HGDIOBJ old = SelectObject(hmemdc,pen);

    ConvertPoint(aLeakPoints[0], &pt);
    MoveToEx(hmemdc,pt.x,pt.y,0);
    for (int counter = 1; counter < numLeakPoints; counter++)
    {
        ConvertPoint(aLeakPoints[counter], &pt);
        LineTo(hmemdc,pt.x,pt.y);
    }

    SelectObject(hmemdc,old);
    DeleteObject(pen);
}

void TXYViewWindow::ShowRegions()
{
    if (!map_i || !set.Map_Read)
        return;
    if (!set.region_mode)
        return;
    map *m = map_i[set.curmap];
    if (!m || !m->regionSet || !m->regionBrush)
        return;

    vec3_t region_min, region_max;
    m->regionBrush->getMins(region_min, region_max);

    POINT pt;
    float x, y;
    int xofs = XYSize.cx / 2;
    int yofs = XYSize.cy / 2;

    RECT XYClipR = {-5, -5, XYSize.cx + 5, XYSize.cy + 5};	// +5 padding

    float u = xy_eye[UAxis];
    float v = xy_eye[VAxis];

    HPEN pen = CreatePen(PS_SOLID,1,RGB(255,0,255));
    HGDIOBJ old = SelectObject(hmemdc,pen);

    // bot left
    x = region_min[UAxis];
    y = region_min[VAxis];
    pt.x = int( (x-u)*set.scale) + xofs;
    pt.y = int(-(y-v)*set.scale) + yofs;
    if (PtInRect(&XYClipR,pt))
    {
        MoveToEx(hmemdc, pt.x, pt.y - 5, 0);
        LineTo(hmemdc,pt.x,pt.y);
        LineTo(hmemdc,pt.x + 5, pt.y);
    }

    // bot right
    x = region_max[UAxis];
    y = region_min[VAxis];
    pt.x = int( (x-u)*set.scale) + xofs;
    pt.y = int(-(y-v)*set.scale) + yofs;
    if (PtInRect(&XYClipR,pt))
    {
        MoveToEx(hmemdc, pt.x, pt.y - 5, 0);
        LineTo(hmemdc,pt.x,pt.y);
        LineTo(hmemdc,pt.x - 5, pt.y);
    }

    // top left
    x = region_min[UAxis];
    y = region_max[VAxis];
    pt.x = int( (x-u)*set.scale) + xofs;
    pt.y = int(-(y-v)*set.scale) + yofs;
    if (PtInRect(&XYClipR,pt))
    {
        MoveToEx(hmemdc, pt.x + 5, pt.y, 0);
        LineTo(hmemdc,pt.x,pt.y);
        LineTo(hmemdc,pt.x, pt.y + 5);
    }

    // top right
    x = region_max[UAxis];
    y = region_max[VAxis];
    pt.x = int( (x-u)*set.scale)+xofs;
    pt.y = int(-(y-v)*set.scale)+yofs;
    if (PtInRect(&XYClipR,pt))
    {
        MoveToEx(hmemdc, pt.x - 5, pt.y, 0);
        LineTo(hmemdc,pt.x,pt.y);
        LineTo(hmemdc,pt.x, pt.y + 5);
    }

    SelectObject(hmemdc,old);
    DeleteObject(pen);
}

void TXYViewWindow::ShowWorldMins()
{
    if (!map_i || !set.Map_Read)
        return;
    map *m = map_i[set.curmap];
    if (!m)
        return;

    const float xofs = XYSize.cx/2 + 0.5f;
    const float yofs = XYSize.cy/2 + 0.5f;
    const float u = xy_eye[UAxis];
    const float v = xy_eye[VAxis];

    RECT XYClipR = {0, 0, XYSize.cx, XYSize.cy};
    HPEN pen = CreatePen(PS_SOLID,1,RGB(255,0,255));
    HGDIOBJ old = SelectObject(hmemdc,pen);

    // top left
    int x1 =  int( (-set.world_minmax - u) * set.scale + xofs);
    int y1 = XYSize.cy - int((set.world_minmax - v) * set.scale + yofs);
    // bottom right
    int x2 = int((set.world_minmax - u) * set.scale + xofs);
    int y2 = int((set.world_minmax + v) * set.scale + yofs);

    MoveToEx(hmemdc, x1, y1, 0);
    LineTo(hmemdc, x2, y1);
    LineTo(hmemdc, x2, y2);
    LineTo(hmemdc, x1, y2);
    LineTo(hmemdc, x1, y1);

    SelectObject(hmemdc,old);
    DeleteObject(pen);
}


void TXYViewWindow::snapPoint(LPPOINT in)
{
    in->x = snapToGrid((float)in->x);
    in->y = snapToGrid((float)in->y);
}

void TXYViewWindow::snapPoint(vec3_t in)
{
    in[0] = (float)snapToGrid(in[0]);
    in[1] = (float)snapToGrid(in[1]);
}
//converts client coordinate to world coordinate
void TXYViewWindow::ConvertPoint(LPPOINT in, vec3_t out)
{
    out[0] = ((float)in->x - (float)XYSize.cx/2.0f) / set.scale + xy_eye[UAxis];
    out[1] = ((float)XYSize.cy/2.0f - (float)in->y) / set.scale + xy_eye[VAxis];
    out[2] = 0;
}
//converts world coordinate to client coordinate
void TXYViewWindow::ConvertPoint(vec3_t in, LPPOINT out)
{
    out->x = set.scale * (in[0] - xy_eye[UAxis]) + (float)XYSize.cx/2.0f;
    out->y = -(set.scale * (in[1] - xy_eye[VAxis]) - (float)XYSize.cy/2.0f);
}

void TXYViewWindow::CmChangeViewNormal()
{
    if (!set.Map_Read)
        return;

    // Just swap and change direction?
    if (xy_viewnormal[NormalAxis] == -1.0 )
        xy_viewnormal[NormalAxis] = 1.0;
    else
        xy_viewnormal[NormalAxis] = -1.0;

    RedrawContents();
}

//
// "change scale" popup
//
void TXYViewWindow::CmS()
{
    Popup PopupMenu;

    PopupMenu.Add("1/64", Ccmd_GetId(const_cast<char *> ("CM_SCALE1_64")));
    PopupMenu.Add("1/32", Ccmd_GetId(const_cast<char *> ("CM_SCALE1_32")));
    PopupMenu.Add("1/16", Ccmd_GetId(const_cast<char *> ("CM_SCALE1_16")));
    PopupMenu.Add("1/8", Ccmd_GetId(const_cast<char *> ("CM_SCALE1_8")));
    PopupMenu.Add("25%", Ccmd_GetId(const_cast<char *> ("CM_SCALE1_4")));
    PopupMenu.Add("50%", Ccmd_GetId(const_cast<char *> ("CM_SCALE1_2")));
    PopupMenu.Add("75% (default)", Ccmd_GetId(const_cast<char *> ("CM_SCALE3_4")));
    PopupMenu.Add("100%", Ccmd_GetId(const_cast<char *> ("CM_SCALE1")));
    PopupMenu.Add("200%", Ccmd_GetId(const_cast<char *> ("CM_SCALE2")));
    PopupMenu.Add("400%", Ccmd_GetId(const_cast<char *> ("CM_SCALE4")));
    PopupMenu.Add("800%", Ccmd_GetId(const_cast<char *> ("CM_SCALE8")));
    PopupMenu.Add("1600%", Ccmd_GetId(const_cast<char *> ("CM_SCALE16")));
    PopupMenu.Separator();
    PopupMenu.Add("Other Scale...", Ccmd_GetId(const_cast<char *> ("CM_SET_SCALE")));

    RECT r;
    GetScaleBoxRect(&r);
    POINT ul;
    ul.x = r.left;
    ul.y = r.top;
    ClientToScreen(hwnd,&ul);
    PopupMenu.pt.x = ul.x - FRAME_X;
    PopupMenu.pt.y = ul.y - FRAME_Y;

    PopupMenu.Show(frame->hwnd);
}

//
// "change grid" popup
//
void TXYViewWindow::CmG()
{
    Popup PopupMenu;

    PopupMenu.Add("1", Ccmd_GetId(const_cast<char *> ("CM_GRID1")));
    PopupMenu.Add("2", Ccmd_GetId(const_cast<char *> ("CM_GRID2")));
    PopupMenu.Add("4", Ccmd_GetId(const_cast<char *> ("CM_GRID4")));
    PopupMenu.Add("8", Ccmd_GetId(const_cast<char *> ("CM_GRID8")));
    PopupMenu.Add("16", Ccmd_GetId(const_cast<char *> ("CM_GRID16")));
    PopupMenu.Add("32", Ccmd_GetId(const_cast<char *> ("CM_GRID32")));
    PopupMenu.Add("64", Ccmd_GetId(const_cast<char *> ("CM_GRID64")));
    PopupMenu.Add("128", Ccmd_GetId(const_cast<char *> ("CM_GRID128")));
    PopupMenu.Add("256", Ccmd_GetId(const_cast<char *> ("CM_GRID256")));
    PopupMenu.Separator();
    int id = Ccmd_GetId(const_cast<char *> ("CM_SET_GRID"));
    if (set.grid_style == GRID_LINES)
    {
        PopupMenu.Add("Use Grid Dots...", id);
    }
    else
    {
        PopupMenu.Add("Use Grid Lines...", id);
    }

    RECT r;
    GetGridBoxRect(&r);
    POINT ul;
    ul.x = r.left;
    ul.y = r.top;
    ClientToScreen(hwnd,&ul);
    PopupMenu.pt.x = ul.x - FRAME_X;
    PopupMenu.pt.y = ul.y - FRAME_Y;

    PopupMenu.Show(frame->hwnd);

    DoNCPaint();
}

//
// CanClose
//
bool TXYViewWindow::CanClose()
{
    return KillAllWindows;
}

//
// DrawGrid
//
void TXYViewWindow::DrawGrid()
{
    if (!set.show_grid && !set.show_coordinates)
        return;

    RECT wind;
    GetClientRect(hwnd,&wind);

    float u = xy_eye[UAxis];	//distance from origin to eye
    float v = xy_eye[VAxis];
    float xofs = XYSize.cx/2 + 0.5f;
    float yofs = XYSize.cy/2 + 0.5f;

    RECT XYGridClipR = { 0, 0, XYSize.cx, XYSize.cy };

    map *m = map_i[set.curmap];

    bool grid_regions = (m && m->regionSet && m->regionBrush && set.region_clip_grid && set.region_mode);

    // adjust bounds if region is set...
    if (grid_regions)
    {

        vec3_t region_min, region_max;
        m->regionBrush->getMins(region_min, region_max);

        // bot left
        float x = region_min[UAxis];
        float y = region_min[VAxis];

        POINT pt;
        pt.x =  (LONG) (x-u)*set.scale+XYSize.cx/2;
        pt.y = (LONG) -(y-v)*set.scale+XYSize.cy/2;

        XYGridClipR.left = (LONG) x;
        XYGridClipR.bottom = (LONG) y;

        wind.left = pt.x;
        wind.bottom = pt.y;

        x = region_max[UAxis];
        y = region_max[VAxis];
        pt.x = (LONG) (x-u)*set.scale+XYSize.cx/2;
        pt.y = (LONG) -(y-v)*set.scale+XYSize.cy/2;

        XYGridClipR.right = (LONG) x;
        XYGridClipR.top = (LONG) y;

        wind.right = pt.x;
        wind.top = pt.y;
    }

    float adjustx = (u - (XYSize.cx/2)/set.scale); 	// total width is xxx pixels, but if
    float adjusty = (v - (XYSize.cy/2)/set.scale); 	// set.scale is .5, then it's twice as many "units"
    // so adjustment is eye.x - midpoint in real units...

    float top    = (adjusty - (float)set.gridsize);			// top is the "first" grid line...
    float bottom = (adjusty + ((float)XYSize.cy/set.scale));	// in real coordinates...
    float left   = (adjustx - set.gridsize);
    float right  = (adjustx + ((float)XYSize.cx/set.scale));


    int x, y, stopx, stopy;

    int m_s = (int) max(64, set.gridsize / set.scale);	//major scale

    //fixes major grid line from getting out of sync
    if(m_s > 512)
        m_s = 1024;
    else if(m_s > 256)
        m_s = 512;
    else if(m_s > 128)
        m_s = 256;
    else if(m_s > 64)
        m_s = 128;

    //MINOR
    if(set.show_grid)
    {
        HPEN graypen = CreatePen(PS_SOLID,1,set.color_gridminor);
        HGDIOBJ oldpen = SelectObject(hmemdc,graypen);
        if (set.gridsize >= 4/set.scale)
        {
            y =		(int) floor(top/set.gridsize) * set.gridsize;
            x =		(int) floor(left/set.gridsize) * set.gridsize;
            stopy = (int) floor(bottom/set.gridsize) * set.gridsize;
            stopx = (int) floor(right/set.gridsize) * set.gridsize;

            //clip grid to draw within [-TMAX,TMAX]
            x = max(x, -TMAX);			//start only needs to be checked at min. bound
            y = max(y, -TMAX);
            stopx = min(stopx, TMAX);	//end only needs to be checked at max bound
            stopy = min(stopy, TMAX);

            if (set.grid_style == GRID_LINES)
            {

                //regions
                if (grid_regions)
                {
                    for (; y <= stopy; y += set.gridsize)
                    {
                        if (y & (m_s-1))
                        {
                            int ypos = XYSize.cy - (int)((y - v)*set.scale + yofs);
                            if (y >= XYGridClipR.bottom && y <= XYGridClipR.top)
                            {
                                MoveToEx(hmemdc,wind.left ,ypos, 0);
                                LineTo(hmemdc,wind.right,ypos);
                            }
                        }
                    }

                    for (; x <= stopx; x+= set.gridsize)
                    {
                        if (x & (m_s-1))
                        {
                            int xpos = (int)((x - u)*set.scale + xofs);
                            if (x >= XYGridClipR.left && x <= XYGridClipR.right)
                            {
                                MoveToEx(hmemdc,xpos,wind.top,0);
                                LineTo(hmemdc,xpos,wind.bottom);
                            }
                        }
                    }
                    //no regions
                }
                else
                {
                    for (; y <= stopy; y += set.gridsize)
                    {
                        if (y & (m_s-1))
                        {
                            int ypos = XYSize.cy - (int)((y - v)*set.scale + yofs);
                            MoveToEx(hmemdc,0 ,ypos, 0);
                            LineTo(hmemdc,XYSize.cx,ypos);
                        }
                    }

                    for (; x <= stopx; x+= set.gridsize)
                    {
                        if (x & (m_s-1))
                        {
                            int xpos = (int)((x - u)*set.scale + xofs);
                            MoveToEx(hmemdc,xpos,0,0);
                            LineTo(hmemdc,xpos,XYSize.cy);
                        }
                    }
                }
            }
            else  						//dot grid
            {
                int xPlot,yPlot;
                int savey = y;

                if (grid_regions)
                {
                    for (; x <= stopx; x+= set.gridsize)
                    {
                        xPlot = (int)((x - u)*set.scale + xofs);
                        for (y = savey; y <= stopy ; y += set.gridsize)
                        {
                            yPlot = XYSize.cy - (int)((y - v)*set.scale + yofs);
                            POINT pt = { xPlot, yPlot };
                            if (PtInRect(&wind,pt))
                                SetPixelV(hmemdc,xPlot,yPlot,set.color_gridminor);
                        }
                    }
                }
                else
                {
                    for (; x <= stopx; x+= set.gridsize)
                    {
                        xPlot = (int)((x - u)*set.scale + xofs);
                        for (y = savey; y <= stopy ; y += set.gridsize)
                        {
                            yPlot = XYSize.cy - (int)((y - v)*set.scale + yofs);
                            SetPixelV(hmemdc,xPlot,yPlot,set.color_gridminor);
                        }
                    }
                }
            }
        }
        SelectObject(hmemdc,oldpen);
        DeleteObject(graypen);
    }

    y =	 (int) floor(top/m_s) * m_s;
    stopy = (int) floor(bottom/m_s) * m_s;
    x =	 (int) floor(left/m_s) * m_s;
    stopx = (int) floor(right/m_s) * m_s;

    //clip grid to be within [-TMAX,TMAX]
    if(x <= -TMAX) x = -TMAX;		//start only needs to be checked at min. bound
    if(stopx >= TMAX) stopx = TMAX;	//end only needs to be checked at max bound
    if(y <= -TMAX) y = -TMAX;
    if(stopy >= TMAX) stopy = TMAX;

    int savex = x;
    int savey = y;

    //MAJOR

    if(set.show_grid)
    {
        HPEN bluepen = CreatePen(PS_SOLID,1,set.color_gridmajor);
        HGDIOBJ oldpen = SelectObject(hmemdc,bluepen);

        if (set.scale > 4.0/(float)(m_s))
        {
            int xPlot, yPlot;

            if (set.grid_style == GRID_LINES)
            {
                //regions
                if (grid_regions)
                {
                    // vertical lines
                    for (; y<=stopy; y+= m_s)
                    {
                        int ypos = XYSize.cy - (int)((y - v)*set.scale + yofs); //XYSize.cy - ((float)y-adjusty)*set.scale;
                        if (y >= XYGridClipR.bottom && y <= XYGridClipR.top)
                        {
                            MoveToEx(hmemdc,wind.left, ypos, 0);
                            LineTo(hmemdc,wind.right, ypos);
                        }
                    }
                    // horizontal lines
                    for (; x<=stopx; x+= m_s)
                    {
                        int xpos = (int)((x - u)*set.scale + xofs);//(((float)x-adjustx)*set.scale);
                        if (x >= XYGridClipR.left && x <= XYGridClipR.right)
                        {
                            MoveToEx(hmemdc,xpos,wind.top, 0);
                            LineTo(hmemdc,xpos,wind.bottom);
                        }
                    }
                    //no regions
                }
                else
                {
                    // vertical lines
                    for (; y<=stopy; y+= m_s)
                    {
                        int ypos = XYSize.cy - (int)((y - v)*set.scale + yofs); //XYSize.cy - ((float)y-adjusty)*set.scale;
                        MoveToEx(hmemdc,0, ypos, 0);
                        LineTo(hmemdc,XYSize.cx, ypos);
                    }
                    // horizontal lines
                    for (; x<=stopx; x+= m_s)
                    {
                        int xpos = (int)((x - u)*set.scale + xofs);//(((float)x-adjustx)*set.scale);
                        MoveToEx(hmemdc, xpos, 0, 0);
                        LineTo(hmemdc, xpos, XYSize.cy);
                    }
                }

            }
            else  					//dots
            {

                //regions
                if (grid_regions)
                {
                    for (; x <= stopx; x+= m_s)
                    {
                        xPlot = (int)((x - u)*set.scale + xofs);//((x-adjustx)*set.scale);
                        for (y = savey; y <= stopy ; y += m_s)
                        {
                            yPlot = XYSize.cy - (int)((y - v)*set.scale + yofs); //(XYSize.cy-(y-adjusty)*set.scale);

                            POINT pt = { xPlot, yPlot };
                            if (PtInRect(&wind,pt))
                            {
                                if(set.grid_smallmajor)
                                {
                                    SetPixelV(hmemdc,xPlot,yPlot,set.color_gridmajor);
                                }
                                else
                                {
                                    MoveToEx(hmemdc,xPlot-1,yPlot,0);
                                    LineTo(hmemdc,xPlot+2,yPlot);
                                    SetPixelV(hmemdc,xPlot,yPlot-1,set.color_gridmajor);
                                    SetPixelV(hmemdc,xPlot,yPlot+1,set.color_gridmajor);
                                }
                            }
                        }
                    }
                    //no regions
                }
                else
                {
                    for (; x <= stopx; x+= m_s)
                    {
                        xPlot = (int)((x - u)*set.scale + xofs);//((x-adjustx)*set.scale);
                        for (y = savey; y <= stopy ; y += m_s)
                        {
                            yPlot = XYSize.cy - (int)((y - v)*set.scale + yofs); //(XYSize.cy-(y-adjusty)*set.scale);

                            if(set.grid_smallmajor)
                            {
                                SetPixelV(hmemdc,xPlot,yPlot,set.color_gridmajor);
                            }
                            else
                            {
                                MoveToEx(hmemdc,xPlot-1,yPlot,0);
                                LineTo(hmemdc,xPlot+2,yPlot);
                                SetPixelV(hmemdc,xPlot,yPlot-1,set.color_gridmajor);
                                SetPixelV(hmemdc,xPlot,yPlot+1,set.color_gridmajor);
                            }
                        }
                    }
                }


            }

        }
        SelectObject(hmemdc,oldpen);
        DeleteObject(bluepen);
    }

    // COORDS

    if (set.show_coordinates)
    {

        //for spacing, a modulus is used to preserve which pos has a coordinate displayed. simply skipping
        //steps caused the displayed coordinates to jump around because the starting pos would be unknown

        HGDIOBJ oldfont = SelectObject(hmemdc,gridfont);
        SetTextColor(hmemdc,set.color_gridlabel);
        SetBkColor(hmemdc,set.color_background);

        char text[32];

        int spacing = (int)(50.f / (m_s*set.scale));	//coords must be ~50px apart. 5 digits req ~50px

        for (y = savey; y<=stopy; y+= m_s)
        {
            if(spacing && (y/m_s) % spacing)
                continue;
            int len = sprintf(text, "%li",y);
            TextOut(hmemdc,2,XYSize.cy-int((y-adjusty)*set.scale-0.5),text,len);
        }

        for (x = savex; x<=stopx; x+= m_s)
        {
            if(spacing && (x/m_s) % spacing)
                continue;
            int len = sprintf(text, "%li",x);
            TextOut(hmemdc,(int)((x-adjustx)*set.scale+2.5f),XYSize.cy-14,text,len);
        }
        SelectObject(hmemdc,oldfont);
    }

}


void TXYViewWindow::SetCaption()
{
    char caption[80];
    char scalestr[32];
    GetScaleText(scalestr);

    static const char * xytype[] =
    {
        "Top   - XY View - [Grid %i / Scale %s]",
        "Right - YZ View - [Grid %i / Scale %s]",
        "Front - XZ View - [Grid %i / Scale %s]",
        "Bottom - XY View - [Grid %i / Scale %s]",
        "Left   - YZ View - [Grid %i / Scale %s]",
        "Back   - XZ View - [Grid %i / Scale %s]"
    };

    int vt = ViewType;
    if (xy_viewnormal[NormalAxis] != -1.0)
        vt += 3;

    sprintf(caption,xytype[vt],set.gridsize,scalestr);

    Window::SetCaption(caption);
}
//
void TXYViewWindow::ClearContents()
{
    if (IsIconic())
        return;

    HDC hdc = GetDC(hwnd);
    RECT r;
    GetClientRect(hwnd,&r);
    HBRUSH brush = CreateSolidBrush(set.color_background);
    FillRect(hdc,&r,brush);
    DeleteObject(brush);
    ReleaseDC(hwnd,hdc);
}

// draw view without cameras
void TXYViewWindow::RedrawFromBase()
{
    if (!set.Map_Read)
        return;
    if (IsIconic())
        return;
    if (!hmemdc || !hmembitmap)
        return;
    if (!disposableDC)
        return;

    HDC hdc = GetDC(hwnd);
    BitBltRect(hdc,XYClip,hbasedc,0,0,SRCCOPY);

    if (set.show_cameras)
        DrawCameras(hdc);

    ReleaseDC(hwnd,hdc);
}

void TXYViewWindow::RedrawContents()
{
    if (!set.Map_Read)
        return;


    GdiFlush();

    // fill in the caption, even if iconic...
    SetCaption();
    DoNCPaint();

    if (!IsVisible())
        return;
    if (!hmemdc || !hmembitmap)
        return;
    if (!disposableDC)
        return;

    map *m = map_i[set.curmap];

    RECT r;
    GetClientRect(hwnd,&r); // qqq window?
    int height = r.bottom-r.top;
    int width  = r.right-r.left;

    HBRUSH brush = CreateSolidBrush(set.color_background);
    FillRect(hmemdc,&r,brush);
    DeleteObject(brush);
    HGDIOBJ oldfont = SelectObject(hmemdc, set.font12);
    SetBkColor(hmemdc, set.color_background);
    SetTextColor(hmemdc, set.color_foreground);

    // grid
    DrawGrid();
    ShowWorldMins();

    // draw full origin crosshair before drawing brushes
    if (set.zero_crosshair == 2)
        DrawZeroCrosshair(hmemdc);

    // draw unselected brushes
    XYTraverseUnselected(hmemdc);

    // copy it
    BitBltRect(disposableDC,XYClip,hmemdc,0,0,SRCCOPY);

    // draw selected to current
    XYTraverseSelected(hmemdc);

    // shadow the selected onto disposable..
    if (set.ghosts_xy)
    {
        clipDraw = true;
        XYTraverseSelected(disposableDC);
        clipDraw = false;
    }

    // draw clippers
    m->clipper_i->XYDrawSelf(this, hmemdc, true);

    // draw xy eye (at center of view)
    if (set.show_eye)
        DrawEye(hmemdc,width,height);

    // draw small crosshair on top of brushes
    if (set.zero_crosshair == 1)
        DrawZeroCrosshair(hmemdc);


    ShowLeaks();
    ShowRegions();

    //copy rendered view without cameras
    BitBltRect(hbasedc,XYClip,hmemdc,0,0,SRCCOPY);

    // draw cameras
    if (set.show_cameras)
        DrawCameras(hmemdc);

    if (groupWindow)
    {
        int idx = groupWindow->GroupList->GetSelIndex();
        if (idx >= 0 && idx < groupWindow->GroupList->GetCount())
        {
            status->SetText(STATUS_GROUP, m->groups->GetName(idx), true);
        }
    }

    // render to screen
    HDC hdc = GetDC(hwnd);
    BitBltRect(hdc,XYClip,hmemdc,0,0,SRCCOPY);
    ReleaseDC(hwnd,hdc);

    SelectObject(hmemdc,oldfont);
}

// draw unselected brushes/ents
void TXYViewWindow::XYTraverseUnselected(HDC hdc)
{
    map *m = map_i[set.curmap];

    Entity *cur;
    SetBrush *curB;
    char *txt;
    vec3_t mins, maxs;
    vec3_t xymins, xymaxs;

    int ua, va, na;
    GetAxes(ViewType, &ua, &va, &na);

    float u = xy_eye[ua];
    float v = xy_eye[va];

    // Clipping box...
    xymins[ua] = u - (float)(XYSize.cx/2)/set.scale;
    xymins[va] = v - (float)(XYSize.cy/2)/set.scale;
    xymaxs[ua] = u + (float)(XYSize.cx/2)/set.scale;
    xymaxs[va] = v + (float)(XYSize.cy/2)/set.scale;
    xymins[na] = -8192.0;	//FIXME - these numbers might break something... ????? should use world_minmax?
    xymaxs[na] =  8192.0;

    int fsize = (int) 12*set.scale;
    if (fsize < 7) fsize = 7;
    if (fsize > 18) fsize = 18;

    for (cur = m->objects.p_next; cur && (cur != &m->objects); cur = cur->p_next)
    {
        LoopProblem("DMXYUE");
        for (curB = cur->objects.p_next; curB != &cur->objects; curB = curB->p_next)
        {
            LoopProblem("DMXYUB");
            if (!curB->IsDrawable())
                continue;
            if (curB->IsSelected())
                continue;

            curB->getMins(mins,maxs);

            // check bboxes
            if (mins[0] >= xymaxs[0] || maxs[0] <= xymins[0])
                continue;
            if (mins[1] >= xymaxs[1] || maxs[1] <= xymins[1])
                continue;
            if (mins[2] >= xymaxs[2] || maxs[2] <= xymins[2])
                continue;

            curB->XYDrawSelf(hdc,xy_eye,index);
        }
    }

    if (set.show_names && (m->objects.p_next != &m->objects))
    {

        HFONT font = GetFont(set.bsp_font,-fsize);
        HGDIOBJ oldfont = SelectObject(hdc,font);
        SetTextColor(hdc,set.color_foreground);
        SetBkColor(hdc,set.color_background);
        for (cur = m->objects.p_next; cur && (cur != &m->objects); cur = cur->p_next)
        {
            LoopProblem("DNAMESMXYUB");
            if (cur == m->world)
                continue;

            curB = cur->objects.p_next;
            if (!curB || !curB->IsDrawable())
                continue;

            curB->getMins(mins,maxs);

            // check bboxes
            for (int k=0 ; k<3 ; k++)
                if (mins[k] >= xymaxs[k] || maxs[k] <= xymins[k])
                    continue;

            int x = (int)(mins[ua]-u)*set.scale + XYSize.cx/2;
            int y = (int)XYSize.cy/2 - (mins[va]-v)*set.scale;
            txt = cur->classname;

            if (!txt || !*txt)
                continue;

            TextOut(hdc,x,y+3,txt,strlen(txt));
        }
        SelectObject(hdc,oldfont);
        DeleteObject(font);
    }

}

// draw selected brushes/ents
void TXYViewWindow::XYTraverseSelected(HDC hdc)
{
    map *m = map_i[set.curmap];

    Entity *cur;
    SetBrush *curB;
    SetBrush *firstSel;
    vec3_t mins, maxs;
    vec3_t xymins, xymaxs;

    int ua, va, na;
    GetAxes(ViewType, &ua, &va, &na);

    float u = xy_eye[ua];
    float v = xy_eye[va];

    // Clipping box...
    xymins[ua] = u - (float)(XYSize.cx/2)/set.scale;
    xymins[va] = v - (float)(XYSize.cy/2)/set.scale;
    xymaxs[ua] = u + (float)(XYSize.cx/2)/set.scale;
    xymaxs[va] = v + (float)(XYSize.cy/2)/set.scale;
    xymins[na] = -8192.0;
    xymaxs[na] =  8192.0;

    firstSel = m->selectedBrush();

    for (cur = m->objects.p_next; cur && (cur != &m->objects); cur = cur->p_next)
    {
        LoopProblem("DMXYSE");
        for (curB = cur->objects.p_next; curB != &cur->objects; curB = curB->p_next)
        {
            LoopProblem("DMXYSB");

            if (!curB->IsDrawable())
                continue;

            if (!curB->IsSelected())
                continue;

            curB->getMins(mins,maxs);

            // check bboxes
            for (int k=0 ; k<3 ; k++)
                if (mins[k] >= xymaxs[k] || maxs[k] <= xymins[k])
                    continue;

            if (curB != firstSel)
            {
                curB->XYDrawSelf(hdc,xy_eye,index);
            }
        }
    }

    // draw current last so currentface is always visible
    if(firstSel)
        firstSel->XYDrawSelfCur(hdc,xy_eye,index);
}

void TXYViewWindow::EvTimer(UINT timerId)
{
    if (!set.Map_Read)
        return;

    int shiftKey, ctrlKey;

    vec3_t p1, p_2;
    vec3_t pt;
    POINT location;

    switch (timerId)
    {

    case TIMER_SEL:
        if (!mousedown)
        {
            KillTimer(hwnd,TIMER_SEL);
            return;
        }
        NextHitBrush();
        RedrawContents();
        break;

    case TIMER_ADDCLIP:
    {

        KillTimer(hwnd,TIMER_ADDCLIP);	// timer needed only once

        if(!mousedown)
            return;

        map *m = map_i[set.curmap];

        GetCursorPos(&location); // get mouse location...
        if (WindowFromPoint(location) != hwnd)
            break;


        //if not 3rd point, add a new point. else just move the 3rd point.
        if(m->clipper_i->num < 3)
        {

            ScreenToClient(hwnd,&location);
            // convert from screen to world units
            ConvertPoint(&location,pt);

            // fake startmouse
            snapPoint(pt);
            startpt[0] = pt[0];
            startpt[1] = pt[1];
            startAngle = 0.0;

            delta[0]       = delta[1]       = 0;
            relative[0]    = relative[1]    = 0;
            oldrelative[0] = oldrelative[1] = 0;

            // add new point
            m->clipper_i->XYClick(pt,UAxis,VAxis,NormalAxis,index);

            if(!m->clipper_i->num)
                return;
            //flip clippers is done for pt2 in XZ...
            //	if(ViewType == Type_XZ && m->clipper_i->num == 2) {
            //		m->curclippoint = 0;
            //	}
        }
        else
        {
            snapPoint(startpt);
        }

        ug = true;
        mousedown = CLIPPT_CALLBACK;

        break;
    }
    case TIMER_FLY:
        if (mousedown)  // don't do anything...
            break;

        GetCursorPos(&location); // get mouse location...
        if (WindowFromPoint(location) != hwnd)
            break;

        shiftKey = GetAsyncKeyState(VK_SHIFT)   ? MK_SHIFT : 0;
        ctrlKey  = GetAsyncKeyState(VK_CONTROL) ? MK_CONTROL : 0;

        ScreenToClient(hwnd,&location);
        // convert from screen to world units
        ConvertPoint(&location,pt);

        p1[UAxis] = p_2[UAxis] = pt[0];
        p1[VAxis] = p_2[VAxis] = pt[1];
        p1[NormalAxis] = xy_viewnormal[NormalAxis] * -TMAX;//4096;
        p_2[NormalAxis] = xy_viewnormal[NormalAxis] * TMAX;//4096;

        map_i[set.curmap]->getMouseObject(&flyover, p1, p_2, pt, ViewType, UAxis, VAxis);
        // put up the appropriate messages...
        TranslateFlyOver(flyover, shiftKey | ctrlKey);

        break;

    case TIMER_CLIP:
    {
        if (!set.animate_clip_points || !map_i || !map_i[set.curmap])
        {
            KillTimer(hwnd,TIMER_CLIP);
            break;
        }

        map *m = map_i[set.curmap];
        // redraw clipper points?...
        if (m->clipper_i->num <= 0)
        {
            KillTimer(hwnd,TIMER_CLIP);
            break;
        }
        HDC hdc = GetDC(hwnd);
        m->clipper_i->XYDrawSelf(this, hdc, true);
        m->clipper_i->state ^= 1;
        ReleaseDC(hwnd, hdc);

        break;
    }
    }
}

void TXYViewWindow::TranslateFlyOver(mouseobject_t& flyover, UINT modkeys)
{
    char outstr[80];
    // search for the correct setting...
    int whichMouse = 0;  // left click assumed...
    unsigned char flags;
    mouseEventType *p;

    flags = 0x00;

    if (modkeys & (MK_SHIFT))
        flags |= SHIFT_FLAG;

    if (modkeys & (MK_CONTROL))
        flags |= CTRL_FLAG;

    if(0x8000 & GetKeyState(VK_MENU))
        flags |= ALT_FLAG;

    p = mouse_events[curEventType];

    while (p)
    {
        if ((p->whichMouse == whichMouse) && (p->moveFlags == flags))
            break;
        p = p->next;
    }

    if (!p)   // not found...
        return;

    // parse out the string...
    int action;
    bool handled = false;
    char *name;

    for (int i = 0; i < p->count; i++)
    {
        action = TranslateCommand(p->moves[i]);

        if (!action)
            break;

        switch (action)
        {
        case CAMERA_CALLBACK:
            if (flyover.Camera)
            {
                handled = true;
                status->SetPartBmp(STATUS_MODE,7);
            }
            break;
        case CLIPPT_CALLBACK:
            if (flyover.Clipper)
            {
                handled = true;
                status->SetPartBmp(STATUS_MODE,4);
            }
            break;
        case VERTEX_CALLBACK:
        case EDGEDRAG_CALLBACK:
            if (set.vertex_select_mode && (flyover.Vertex) /*&& flyover.Selected*/)
            {
                status->SetPartBmp(STATUS_MODE,1);
                handled = true;
            }
            break;
        case BRUSHDRAG_CALLBACK:
        case FACEDRAG_CALLBACK:
        case MULTIFACEDRAG_CALLBACK:
        case MULTIFACEDRAG_CALLBACKSTRICT:
        case NEW_CALLBACK:
            break;
        case SELECT_CALLBACK:
        case SELECTTOTAL_CALLBACK:
            if (flyover.CenterKnob)
            {
                status->SetPartBmp(STATUS_MODE,2);
                handled = true;
            }
            break;
        case ROTATE_CALLBACK:
            if (set.vertex_select_mode && (flyover.Vertex) /*&& flyover.Selected*/)
            {

                status->SetPartBmp(STATUS_MODE,0);
                handled = true;
            }
            break;
        case ENTITY_CONNECT:
        case CURFACEDRAG_CALLBACK:
        case SHEAR_CALLBACK:
        case CLIPWHOLE_CALLBACK:
        case MOVECAMERA_CALLBACK:
        case ADDCLIPPOINT_CALLBACK:
        case DIRECTION_CALLBACK:
        case TEXTUREBRUSH_CALLBACK:
        case TEXTUREFACE_CALLBACK:
            break;
        default:
            handled = false;
            break;
        }

        if (handled)
            break;
    }

    if (handled)
    {
        status->Redraw();
        return;
    }

    if (flyover.Nothing)
    {
        status->SetText("");
        status->SetText(STATUS_BOUND,"");
        status->SetPartBmp(STATUS_MODE,3);
        status->Redraw();
    }
    else
    {
        if (flyover.Brush->parent == map_i[set.curmap]->world)
        {
            sprintf(outstr,"World Brush");
            status->SetPartBmp(STATUS_MODE, (flyover.CenterKnob ? 2 : 5) );
        }
        else
        {
            name = flyover.Entity->classname;
            if (name && *name)
                sprintf(outstr,"Ent.:  %s",name);
            else
                sprintf(outstr,"?");

            status->SetPartBmp(STATUS_MODE, (flyover.CenterKnob ? 2 : 6) );
        }

        if (flyover.Brush)
        {
            vec3_t mins, maxs;
            char outstr2[256];
            flyover.Brush->getMins(mins,maxs);

            sprintf(outstr2,"Size (%i %i %i)",
                    (int)(maxs[0] - mins[0]),
                    (int)(maxs[1] - mins[1]),
                    (int)(maxs[2] - mins[2]));
            status->SetText(STATUS_BOUND, outstr2);
        }
        else
        {
            status->SetText(STATUS_BOUND, "");
        }

        status->SetText(outstr);
    }
    status->Redraw();
}


void TXYViewWindow::EvMouseMove(UINT modkeys, LPPOINT location)
{
    if (!set.Map_Read)
        return;
    if (!map_i || !map_i[set.curmap])
        return;
    if(mousedown == ADDCLIPPOINT_CALLBACK)	// dummy event
        return;

    vec3_t	p1, p_2;
    vec3_t pt;

    // convert from screen to world units
    ConvertPoint(location,pt);
    p1[UAxis] = p_2[UAxis] = pt[0];
    p1[VAxis] = p_2[VAxis] = pt[1];
    p1[NormalAxis] = xy_viewnormal[NormalAxis] * -TMAX;//4096;
    p_2[NormalAxis] = xy_viewnormal[NormalAxis] * TMAX;//4096;

    SetBrush *b;
    Entity *saveCurrent;
    bool olddraw;
    bool oldwire;

    char str[80];
    int i;
    map *m = map_i[set.curmap];
    int c_eye;
    RECT box;
    Group *saveGroupPtr;

    char outstr[80];

    GdiFlush();

    if (!mousedown)
    {
        m->getMouseObject(&flyover, p1, p_2, pt, ViewType, UAxis, VAxis);
        TranslateFlyOver(flyover,modkeys);
    }

    // these are the world coordinates...
    ///////////// PUT ME IN SUBROUTINE...
    vec3_t snappedPoint;
    if (set.show_grid)
    {
        snappedPoint[0] = (float)snapToGrid(pt[0]);
        snappedPoint[1] = (float)snapToGrid(pt[1]);
        snappedPoint[2] = (float)snapToGrid(pt[2]);
    }
    else
    {
        snappedPoint[0] = pt[0];
        snappedPoint[1] = pt[1];
        snappedPoint[2] = pt[2];
    }

    sprintf(outstr,"@%5i,%5i",(int)snappedPoint[0],(int)snappedPoint[1]);
    status->SetText(STATUS_POS, outstr);

    if (!mousedown)
        return;

    if (!disposableDC)
        return;  // something's wrong...

    saveCurrent = m->current;

    ConvertPoint(location,newpt);

    if (ug)
        snapPoint(newpt);

    relative[0] = newpt[0] - startpt[0];
    relative[1] = newpt[1] - startpt[1];

    if (relative[0]==oldrelative[0] && relative[1]==oldrelative[1])
        return;

    // for selection box, make them pull at least 4 pixels to start out...
    if ((mousedown == SELECT_CALLBACK) || (mousedown == SELECTTOTAL_CALLBACK))
    {
        if (!alreadySelecting && (abs(relative[0]) < 4) && (abs(relative[1]) < 4))
        {
            return;
        }
        else
        {
            alreadySelecting = true;
            HitRestore(); // restore original selection states for hit brushes...
        }
    }

    delta[0] = relative[0] - oldrelative[0];
    delta[1] = relative[1] - oldrelative[1];

    VectorCopy(relative,oldrelative);


    // first throw down the background...
    BitBltRect(hmemdc,XYClip,disposableDC,0,0,SRCCOPY);

    // Throw the cur position stuff here, too...
    status->SetText(STATUS_POS, outstr);

    // now paint this crap on there...
    if (set.show_drag)
    {
        if (mousedown == ROTATE_CALLBACK)
        {
            int ang = (int)(180.0*(startAngle-angle(newpt[0] - sb_ctr[UAxis],newpt[1] - sb_ctr[VAxis]))/M_PI);
            if(set.rotate_step > 1 && set.rotate_step < 360)
                ang -= ang % min(360,set.rotate_step);
            int len = sprintf(outstr,"%i deg.",ang);
            TextOut(hmemdc,location->x-50,location->y,outstr,len);
            char str2[64];
            sprintf(str2,"D(%s)",outstr);
            status->SetText(STATUS_DELTA, str2);
        }
        else
        {
            sprintf(outstr,"D(%i,%i)",(int)relative[0],(int)relative[1]);
            status->SetText(STATUS_DELTA, outstr);
        }
    }
    status->Redraw();
    bool looked_at = false;		//software 3d needs to be redrawn if lookatsel happened

    switch (mousedown)
    {
    case CLIPPT_CALLBACK:
        m->clipper_i->DragCallBack(delta[0],delta[1],map_i[set.curmap]->curclippoint, UAxis, VAxis);
        break;
    case CLIPWHOLE_CALLBACK:
        m->clipper_i->DragCallBack(delta[0],delta[1],-1, UAxis, VAxis);
        break;
    case BRUSHDRAG_CALLBACK:
        DragCallback(delta[0],delta[1]);
        looked_at = m->lookAtSelection();
        break;
    case CAMERA_CALLBACK:

        vec3_t v;

        ConvertPoint(location,v);

        int ua, va, na;
        GetAxes(ViewType, &ua, &va, &na);
        m->eye[eyetomove][ua] = v[0];
        m->eye[eyetomove][va] = v[1];

        m->cureye = eyetomove;

        MoveEyePosition(v,set.lock_cameras);

        findQ(set.Q,set.R,m->eye[m->cureye],
              m->angles[m->cureye].roll,
              m->angles[m->cureye].pitch,
              m->angles[m->cureye].yaw);

        m->lookAtSelection();

        editWindow->RedrawContents();
        break;

    case NEW_CALLBACK:
        NewCallback(delta[0],delta[1]);

        looked_at = m->lookAtSelection();
        break;
    case SELECT_CALLBACK:
    case SELECTTOTAL_CALLBACK:
        SelCallback(delta[0],delta[1],mousedown);
        looked_at = m->lookAtSelection();
        break;
    case SHEAR_CALLBACK:
    case VERTEX_CALLBACK:
    case FACEDRAG_CALLBACK:
    case CURFACEDRAG_CALLBACK:
        ControlCallback(delta[0],delta[1]);

        looked_at = m->lookAtSelection();
        break;
    case MULTIFACEDRAG_CALLBACK:
    case MULTIFACEDRAG_CALLBACKSTRICT:
    case EDGEDRAG_CALLBACK:
        MultiControlCallback(delta[0],delta[1]);

        looked_at = m->lookAtSelection();
        break;
    case ROTATE_CALLBACK:
    {
        float ang = startAngle - angle(newpt[0] - sb_ctr[UAxis],newpt[1] - sb_ctr[VAxis]);
        RotateCallback(ang, 0);
        break;
    }
    case DIRECTION_CALLBACK:
        //	DirectionCallback(/*delta[0]*/startAngle - angle(newpt[0] - sel_org[UAxis],newpt[1] - sel_org[VAxis]),delta[1]);
        break;

    case MOVECAMERA_CALLBACK:
    {
        //dragging callback
        if(set.xy_drag_move)
        {

            xy_eye[UAxis] = xydrag_eye[UAxis];
            xy_eye[VAxis] = xydrag_eye[VAxis];

            vec3_t tmp;
            ConvertPoint(location, tmp);

            xy_eye[UAxis] = -(tmp[0] - xydrag_offset_U);
            xy_eye[VAxis] = -(tmp[1] - xydrag_offset_V);

            MoveEyePosition(xy_eye,set.lock_cameras);	//moves CAMERA to follow xy views (default: off)
            //	UpdateWindows();

            for (i = 0; i < set.xyViews; i++)
                xyWindow[i]->RedrawContents();

            editWindow->RedrawContents();

            return;
        }
    }

    }//end switch

    b = m->selectedBrush();

    if ((mousedown == SHEAR_CALLBACK) ||
            (mousedown == VERTEX_CALLBACK) ||
            (mousedown == FACEDRAG_CALLBACK) ||
            (mousedown == CURFACEDRAG_CALLBACK))
    {
        if (b)
        {
            vec3_t mins, maxs;
            b->getMins(mins,maxs);

            sprintf(outstr,"Size (%i %i %i)",(int)(maxs[0] - mins[0]),
                    (int)(maxs[1] - mins[1]),(int)(maxs[2] - mins[2]));
            status->SetText(STATUS_BOUND, outstr);
            status->Redraw();
        }
    }

    if ((mousedown == MULTIFACEDRAG_CALLBACK) ||
            (mousedown == EDGEDRAG_CALLBACK) ||
            (mousedown == MULTIFACEDRAG_CALLBACKSTRICT) ||
            (!set.animate_clip_points && (mousedown == CLIPPT_CALLBACK || mousedown == CLIPWHOLE_CALLBACK)))
    {
        UpdateSelected(true);
        // m->makeSelectedXYDraw(disposableDC,index,-1,m->groups);
        if (set.trackxyin3d || (!set.glBsp && looked_at))
            if(looked_at)
                editWindow->RedrawContents();
            else
                editWindow->UpdateSelected();
        return;
    }
    else if (
        (mousedown == CLIPPT_CALLBACK) ||
        (mousedown == CLIPWHOLE_CALLBACK))
    {
        //special redraw for animated clip points - timer will do the painting so just reblit screen
        if (set.trackxyin3d)
            editWindow->UpdateSelected();
        m->current = saveCurrent;

        if(set.track_clippers == 2)
        {
            //redraw clipped brushes in all xy views
            for (i = 0; i < set.xyViews; i++)
                xyWindow[i]->RedrawContents();
            return;
        }
        else if(set.track_clippers == 1)
        {
            //redraw clipped brush in current xy
            for (i = 0 ; i < set.xyViews; i++)
            {
                if(xyWindow[i] == this)
                    UpdateSelected(false);
                else
                    xyWindow[i]->RedrawFromBase();
            }
        }
        else
        {
            //update clip point positions only
            for (i = 0 ; i < set.xyViews; i++)
                xyWindow[i]->RedrawFromBase();
            return;
        }
    }
    else if (mousedown == CAMERA_CALLBACK)
    {
        HGDIOBJ oldfont = SelectObject(hmemdc,set.font12);

        i = eyetomove;

        float use_scale = min(set.scale,1.0f);
        float size = use_scale*(float)set.camera_size;

        box.left   = (int)(location->x - size/2.0);
        box.right  = (int)(box.left + size);
        box.top    = (int)(location->y - size/2.0);
        box.bottom = (int)(box.top + size);

        HBRUSH brush = CreateSolidBrush(set.color_background);
        HPEN pen = CreatePen(PS_SOLID,1,set.color_activecamera);
        HGDIOBJ oldbrush = SelectObject(hmemdc,brush);
        HGDIOBJ oldpen = SelectObject(hmemdc,pen);
        Rectangle(hmemdc,box.left,box.top,box.right,box.bottom);

        if (set.scale > 0.49)
        {
            int len = sprintf(str,"%i",i+1);
            SetBkMode(hmemdc,TRANSPARENT);
            SetTextAlign(hmemdc,TA_LEFT & TA_TOP);
            TextOut(hmemdc,location->x-3, location->y - (set.camera_font_size),str,len);
        }

        SelectObject(hmemdc,oldpen);
        SelectObject(hmemdc,oldbrush);
        SelectObject(hmemdc,oldfont);
        DeleteObject(pen);
        DeleteObject(brush);
    }
    else if ( selBrush &&
              (mousedown == SELECT_CALLBACK) ||
              (mousedown == SELECTTOTAL_CALLBACK))
    {
        KillTimer(hwnd,TIMER_SEL);  // stop that!
        UpdateSelected(false);

        if (selBrush->IsDrawable())
            selBrush->XYDrawSelfCur(hmemdc,xy_eye,index);

        if (set.trackxyin3d || (!set.glBsp && looked_at))
        {
            if(looked_at)
                editWindow->RedrawContents();
            else
                editWindow->UpdateSelected();

            if (!set.glBsp)
            {
                olddraw = r_drawflat;
                oldwire = r_drawwire;
                r_drawflat = false;
                r_drawwire = true;
                clipDraw = true;
                editWindow->RedrawBrush(selBrush);
                clipDraw = false;
                r_drawflat = olddraw;
                r_drawwire = oldwire;
            }
        }
    }
    else if (b && (mousedown != NEW_CALLBACK))
    {
        switch (mousedown)
        {
        case BRUSHDRAG_CALLBACK:
            UpdateSelected(false);
            //	m->makeSelectedXYDraw(disposableDC,index,set.drag_brushes,m->groups);
            if (b->IsDrawable())
                b->XYDrawSelfCur(hmemdc,xy_eye,index);
            if (set.trackxyin3d || (!set.glBsp && looked_at))
                if(looked_at)
                    editWindow->RedrawContents();
                else
                    editWindow->UpdateSelected();
            break;
        case ROTATE_CALLBACK:
            if (rotateMap)
            {

                c_eye = m->cureye;
                m->current = rotateMap->current;

                rotateMap->makeSelectedXYDraw(hmemdc,index,set.drag_brushes,m->groups);

                if (set.trackxyin3d)
                {
                    saveGroupPtr = rotateMap->groups;

                    rotateMap->groups = m->groups;
                    strncpy(rotateMap->filename,m->filename,sizeof(rotateMap->filename));
                    VectorCopy(m->eye[c_eye],rotateMap->eye[c_eye]);
                    rotateMap->cureye = c_eye;
                    rotateMap->angles[c_eye].roll  = m->angles[c_eye].roll;
                    rotateMap->angles[c_eye].pitch = m->angles[c_eye].pitch;
                    rotateMap->angles[c_eye].yaw   = m->angles[c_eye].yaw;

                    if (set.glBsp)
                        glBspRotate = true;
                    editWindow->UpdateSelectedMap(rotateMap);
                    glBspRotate = false;

                    rotateMap->groups = saveGroupPtr;
                }
            }
            break;
        default:
            if (b->IsDrawable())
                b->XYDrawSelfCur(hmemdc,xy_eye,index);

            if (set.trackxyin3d || (!set.glBsp && looked_at))
                if(looked_at)
                    editWindow->RedrawContents();
                else
                    editWindow->UpdateSelected();
        }
    }
    else
    {
        if ((mousedown == NEW_CALLBACK) && newbrush)
        {

            int tempG = newbrush->group;

            newbrush->group = -1;

            clipDraw = true;
            if (newbrush->IsDrawable())
                newbrush->XYDrawSelfCur(hmemdc,xy_eye,index);

            clipDraw = false;

            newbrush->group = tempG;
            vec3_t mins, maxs;
            newbrush->getMins(mins,maxs);

            if (set.trackxyin3d)
            {
                //leave this gl code here
                if (!set.glBsp)
                {
                    if(looked_at)
                    {
                        editWindow->RedrawContents();
                    }
                    else
                    {
                        olddraw = r_drawflat;
                        oldwire = r_drawwire;
                        r_drawflat = false;
                        r_drawwire = true;
                        clipDraw = true;

                        editWindow->UpdateSelected();
                        // new brush is in the map already...
                        //	      editWindow->RedrawBrush(newbrush);
                        clipDraw = false;
                        r_drawflat = olddraw;
                        r_drawwire = oldwire;
                    }
                }
                else
                {
                    editWindow->UpdateSelected();
                }
            }

            sprintf(outstr,"Size (%i %i %i)",(int)(maxs[0] - mins[0]),
                    (int)(maxs[1] - mins[1]),(int)(maxs[2] - mins[2]));
            status->SetText(STATUS_BOUND, outstr);
            status->Redraw();
        }
    }

    if (set.show_eye)
        DrawEye(hmemdc,XYSize.cx,XYSize.cy);

    if (set.zero_crosshair == 1)				//draw crosshair at 0,0,0
        DrawZeroCrosshair(hmemdc);

    if (set.show_cameras)
    {
        DrawCameras(hmemdc);
        //redraw other 2 views, but not the current one.
        for(i = 0; i < 3; i++)
            if(xyWindow[i] != this)
                xyWindow[i]->RedrawFromBase();
    }


    HDC hdc = GetDC(hwnd);
    BitBlt(hdc,XYClip.left,XYClip.top,XYClip.right-XYClip.left,XYClip.bottom-XYClip.top,hmemdc,0,0,SRCCOPY);
    ReleaseDC(hwnd,hdc);

    m->current = saveCurrent;
}



// dx is the angle...
void TXYViewWindow::RotateCallback(float dx, float /*dy*/)
{
    //TODO: deleting this map resets all qtextures, causing them to be reloaded on each redraw. this
    // happens on almost all types of edits. here is just an example. FIX IT!
    if (rotateMap)
    {
        delete rotateMap;
        rotateMap = NULL;
    }

    int ang = RAD2DEG(dx);
    if(set.rotate_step > 1 && set.rotate_step < 360)
        ang -= ang % min(360,set.rotate_step);

    rpy[UAxis] = 0.0;
    rpy[VAxis] = 0.0;
    //HACK: this corrects rotation in the FRONT view. probably needs to be fixed elsewhere, also check normalaxis=1.0
    if(ViewType==Type_XZ)
        rpy[NormalAxis] = (float)-ang;
    else
        rpy[NormalAxis] = (float)ang;


    map *m = map_i[set.curmap];

    // rotateMap = new map;
    m->copySelection(&rotateMap,true); // force calcwindings
    VectorCopy(m->eye[m->cureye], rotateMap->eye[m->cureye]);
    rotateMap->cureye = m->cureye;
    rotateMap->rotateBrushOrg((int)rpy[1],(int)rpy[0],(int)rpy[2],sb_ctr);
}

void TXYViewWindow::DragCallback(float dx, float dy)
{
    sb_translate[UAxis] = dx;
    sb_translate[VAxis] = dy;
    sb_translate[NormalAxis] = 0;

    map_i[set.curmap]->makeSelectedPerform(SEL_TRANSLATE);
}

bool TXYViewWindow::selectionDragFrom(vec3_t)
{
    map_i[set.curmap]->saveForUndo(const_cast<char *> ("Drag Selection"),UNDO_BRUSHES);
    return true;
}

bool TXYViewWindow::cameraDragFrom(vec3_t pt)
{
    if (!set.Map_Read)
        return false;
    if (!set.show_cameras)
        return false;

    map *m = map_i[set.curmap];
    float pixelWidth = 1.0f/set.scale;						// each pixel is this many world units...
    float delta = ((float)set.camera_size/2.0f) * pixelWidth;	// how many units to allow...

    for (int i = 0; i < MAX_CAMERAS; i++)
    {
        float *v = m->eye[i];
        float pt0 = pt[0] - v[UAxis];
        float pt1 = pt[1] - v[VAxis];
        float d = sqrt(pt0*pt0 + pt1*pt1);

        if (d > delta)
            continue;

        eyetomove = i;
        return true;
    }
    return false;
}

//============================================================================

void TXYViewWindow::DirectionCallback(float /*dx*/, float /*dy*/)
{
//	direction[UAxis] += dx;
//	direction[VAxis] += dy;

//	if (direction[UAxis] == xy_eye[UAxis] && direction[VAxis] == xy_eye[VAxis])
//		return;

    /*	if (ViewType != Type_XY)
    		return;

    	map_i[set.curmap]->angles[map_i[set.curmap]->cureye].pitch = 0;
     	map_i[set.curmap]->angles[map_i[set.curmap]->cureye].roll = 0;
    	map_i[set.curmap]->angles[map_i[set.curmap]->cureye].yaw = -dx;//angle(dx,dy);

    */
}

bool TXYViewWindow::directionDragFrom(vec3_t /*tp*/)
{
//	direction[UAxis] = tp[0];
//	direction[VAxis] = tp[1];
    return true;
}


//============================================================================

void TXYViewWindow::NewCallback (float dx, float dy)
{
    vec3_t	mn, mx;
    int		i;

    newdrag[UAxis] += dx;
    newdrag[VAxis] += dy;

    for (i=0 ; i<3 ; i++)
    {
        if (neworg[i] < newdrag[i])
        {
            mn[i] = neworg[i];
            mx[i] = newdrag[i];
        }
        else
        {
            mn[i] = newdrag[i];
            mx[i] = neworg[i];
        }
    }

    newbrush->setMins(mn,mx);
}

void TXYViewWindow::SelCallback (float dx, float dy, int /*type*/)
{
    if (!selBrush)
        return;

    map *m = map_i[set.curmap];

    newdrag[UAxis] += dx;
    newdrag[VAxis] += dy;

    vec3_t	mn, mx;
    for (int i=0 ; i<3 ; i++)
    {
        if (neworg[i] < newdrag[i])
        {
            mn[i] = neworg[i];
            mx[i] = newdrag[i];
        }
        else
        {
            mn[i] = newdrag[i];
            mx[i] = neworg[i];
        }
    }
    selBrush->setMins(mn,mx);

    if (!selBrush->IsInvalid())
    {
        selBrush->getMins(select_min,select_max);
        m->dirty = 1;
        select_deselect = true;
        if (mousedown == SELECT_CALLBACK)
        {
            m->makeAllPerform(SEL_SELECTPARTIAL);
        }
        else
        {
            m->makeAllPerform(SEL_SELECTCOMPLETE);
        }
        select_deselect = false;
    }
}

bool TXYViewWindow::newBrushDragFrom(vec3_t tp)
{
    Entity *owner;
    texturedef_t	td;

    vec3_t vecEye;

    map *m = map_i[set.curmap];

    VectorCopy(xy_eye,vecEye);

    //	qprintf ("sizing new brush");

    neworg[UAxis] = (float)snapToGrid(tp[0]);
    neworg[VAxis] = (float)snapToGrid(tp[1]);
    neworg[NormalAxis] = (float)snapToGrid(vecEye[NormalAxis]);  // use current eye position, snap first...


    float brushdepth;
    if(set.new_brush_depth > 0)
        brushdepth = (float)set.new_brush_depth;
    else
        brushdepth = m->currentMaxZ(NormalAxis) - m->currentMinZ(NormalAxis);

    newdrag[UAxis] = neworg[UAxis];
    newdrag[VAxis] = neworg[VAxis];
    newdrag[NormalAxis] = neworg[NormalAxis] + brushdepth; // current eye, plus the delta in min/max for the map
    //(map_i[set.curmap]->currentMaxZ(NormalAxis)-
    //map_i[set.curmap]->currentMinZ(NormalAxis))
    //);

    owner = m->current;

    texWindow->getTextureDef(&td);
    int newindex = 0;
    if (groupWindow)
    {
        newindex = groupWindow->GroupList->GetSelIndex();

        if (newindex < 0 || newindex >= groupWindow->GroupList->GetCount()) // name field not one of the groups...
            newindex = 0; // put in default if no group is current...
    };

    newbrush = new SetBrush();
    newbrush->initOwner(owner,neworg,newdrag,&td);
    owner->addObject(newbrush);

    newbrush->setSelected(true);
    newbrush->group = newindex;
    newbrush->flags &= ~B_FILTERED;  // assume no

    m->dirty = 1;
    m->saveForUndo(NULL,UNDO_NOUNDO);

    return true;
}

bool TXYViewWindow::selBrushDragFrom(vec3_t tp)
{
    neworg[UAxis] = (float)(int)tp[0];
    neworg[VAxis] = (float)(int)tp[1];
    neworg[NormalAxis] = -TMAX;//-2048.0;

    newdrag[UAxis] = neworg[UAxis];
    newdrag[VAxis] = neworg[VAxis];
    newdrag[NormalAxis] = TMAX;//2048.0;

    Entity *owner;
    owner = map_i[set.curmap]->world;   // assign to world, never added...

    texturedef_t td;
    texWindow->getTextureDef(&td);
    selBrush = new SetBrush();
    selBrush->initOwner(owner,neworg,newdrag,&td);

    selBrush->setSelected(true);
    selBrush->group = -1;
    selBrush->flags &= ~B_FILTERED;  // assume no

    map_i[set.curmap]->saveForUndo(NULL,UNDO_NOUNDO);
    return true;
}

//============================================================================

void TXYViewWindow::ControlCallback (float dx, float dy)
{
    for (int i=0 ; i<numcontrolpoints ; i++)
    {
        controlpoints[i][UAxis] += dx;
        controlpoints[i][VAxis] += dy;
        if(set.edge_use_grid && set.snap_back)
        {
            controlpoints[i][UAxis] = (float)snapToGrid(controlpoints[i][UAxis]);
            controlpoints[i][VAxis] = (float)snapToGrid(controlpoints[i][VAxis]);
        }
        else
        {
            //must at least be snapped to integers
            controlpoints[i][UAxis] = (float)rint(controlpoints[i][UAxis]);
            controlpoints[i][VAxis] = (float)rint(controlpoints[i][VAxis]);
        }
    }

    SetBrush *b = map_i[set.curmap]->selectedBrush();
    if (b)
        b->calcWindings();
}

void TXYViewWindow::MultiControlCallback (float dx, float dy)
{
    for (int i=0 ; i<multicontrolpoints ; i++)
    {
        mcontrolpoints[i][UAxis] += dx;
        mcontrolpoints[i][VAxis] += dy;
        if(set.edge_use_grid && set.snap_back)
        {
            mcontrolpoints[i][UAxis] = (float)snapToGrid(mcontrolpoints[i][UAxis]);
            mcontrolpoints[i][VAxis] = (float)snapToGrid(mcontrolpoints[i][VAxis]);
        }

    }


    // do them all
    map_i[set.curmap]->makeSelectedPerform(SEL_CALCWINDINGS);
}

bool TXYViewWindow::currentFaceDragFrom()
{
    SetBrush *b;

    map *m = map_i[set.curmap];

    if (m->numSelected() != 1)
        return false;

    b = m->selectedBrush();

    controlpoints[0] = b->currentFace->planepts[0];
    controlpoints[1] = b->currentFace->planepts[1];
    controlpoints[2] = b->currentFace->planepts[2];
    numcontrolpoints = 3;

    m->saveForUndo(const_cast<char *> ("Drag Current Face"),UNDO_BRUSHES);

    return true;
}

bool TXYViewWindow::planeDragFrom(vec3_t tp)
{
    vec3_t dragpoint;

    if (map_i[set.curmap]->numSelected() != 1)
        return false;

    SetBrush *b = map_i[set.curmap]->selectedBrush();

    dragpoint[UAxis] = tp[0];
    dragpoint[VAxis] = tp[1];
    dragpoint[NormalAxis] = b->bctr[NormalAxis];

    b->getXYdragface(dragpoint,ViewType);
    if (!numcontrolpoints)
        return false;

    //	qprintf ("dragging brush plane");

    map_i[set.curmap]->saveForUndo(const_cast<char *> ("Drag XY Face"),UNDO_BRUSHES);

    return true;
}

bool TXYViewWindow::planeDragFromMulti(vec3_t tp, int mousedown)
{
    SetBrush *b;
    Entity *e;

    map *m = map_i[set.curmap];
    int c = m->numSelected();

    if (!c)
        return false;

    vec3_t dragpoint;
    dragpoint[UAxis] = tp[0];
    dragpoint[VAxis] = tp[1];
    dragpoint[NormalAxis] = 2048;

    // find the best brush with the "lead"
    //c = m->count;
    float bestd = 99999.0;
    SetBrush *bestB = NULL;
    float testVal;
    face_t *bestF, *f;

    bestF = NULL;
    for (e = m->objects.p_next; e != &m->objects; e = e->p_next)
    {
        if (!e->modifiable)
            continue;

        for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
        {
            if (!b->IsDrawable())
                continue;

            if (!b->IsSelected())
                continue;

            // FIXME
            testVal = b->checkXYmultidragfaces(dragpoint,ViewType,&f);
            if (testVal < bestd)
            {
                bestd = testVal;
                bestB = b;
                bestF = f;
            }
        }
    }

    if (!bestB)    // this should never happen!
        return false;

    multicontrolpoints = 0;
    if (mcontrolpoints)
    {
        delete[] mcontrolpoints;
        mcontrolpoints = NULL;
    }
    mcontrolpoints = new float *[6*c];
    maxmulticontrolpoints = 6*c;

    for (e = m->objects.p_next; e != &m->objects; e = e->p_next)
    {
        if (!e->modifiable)
            continue;

        for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
        {
            if (!b->IsDrawable())
                continue;

            if (!b->IsSelected())
                continue;

            // FIXME
            // select any that are "parallel" to bestB faces...
            // just look at normal
            b->getXYmultidragfaces(bestF, (mousedown == MULTIFACEDRAG_CALLBACKSTRICT));
        }
    }
    if (!multicontrolpoints)
    {
        delete[] mcontrolpoints;
        mcontrolpoints = NULL;
        maxmulticontrolpoints = 0;
        return false;
    }

    //	qprintf ("dragging brush plane");

    m->saveForUndo(const_cast<char *> ("Multi Drag XY Faces"),UNDO_BRUSHES);

    return true;
}

bool TXYViewWindow::vertexRotate(vec3_t tp)
{
    vec3_t dragpoint;

    map *m = map_i[set.curmap];
    if (m->numSelected() == 0)
        return false;

    dragpoint[UAxis] = tp[0];
    dragpoint[VAxis] = tp[1];
    dragpoint[NormalAxis] = 0.0;

    // NOW, go through all of the potential brushes...
    //
    Entity *ent;
    SetBrush *brush;
    int clicked = 0;
    for (ent = m->objects.p_next; ent != &m->objects; ent = ent->p_next)
    {
        LoopProblem("VROTE");
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            LoopProblem("VROTB");
            if (!brush->IsDrawable())
                continue;

            if (brush->vertexClicked(dragpoint,ViewType))
                clicked = 1;
        }
    }

    if (!clicked)
        return false;

    m->getSelectedCenter();

    rpy[0] = rpy[1] = rpy[2] = 0.0;

    m->saveForUndo(const_cast<char *> ("Rotate Brushes"),UNDO_BRUSHES);

    return true;
}

void KnobCallback(float /*dx*/, float /*dy*/)
{
// Do something...
}

bool knobDrag(vec3_t /*tp*/)
{
    // if (!set.Show_Knobs)
    //    return NO;
    return false;
}

bool objectClick(vec3_t /*tp*/)
{
    return true;
}

bool TXYViewWindow::edgeDragFrom(vec3_t tp)
{

    map *m = map_i[set.curmap];
    if (m->numSelected() == 0)
        return false;

    vec3_t dragpoint;
    dragpoint[UAxis] = tp[0];
    dragpoint[VAxis] = tp[1];
    dragpoint[NormalAxis] = 0.0;

    Entity *ent;
    SetBrush *brush;

    int brushCount = 0;
    for (ent = m->objects.p_next; ent != &m->objects; ent = ent->p_next)
    {
        LoopProblem("EDRAGXYE");
        if (!ent->modifiable)
            continue;

        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            LoopProblem("EDRAGXYB");
            if (!brush->IsDrawable())
                continue;
            if (!brush->IsSelected())
                continue;

            if (brush->checkXYEdgeDragFaces(dragpoint,ViewType))
                brushCount++;
        }
    }

    if (!brushCount)
        return false;

    multicontrolpoints = 0;
    if (mcontrolpoints)
    {
        delete[] mcontrolpoints;
        maxmulticontrolpoints = 0;
        mcontrolpoints = NULL;
    }
    // 2 points per face, 2 faces per edge...
    mcontrolpoints = new float *[5*brushCount];
    maxmulticontrolpoints = 5*brushCount;

    for (ent = m->objects.p_next; ent != &m->objects; ent = ent->p_next)
    {
        LoopProblem("EDRAGE2");
        if (!ent->modifiable)
            continue;

        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            LoopProblem("EDRAGB2");
            if (!brush->IsDrawable())
                continue;
            if (!brush->IsSelected())
                continue;

            brush->getXYEdgeDragFaces(dragpoint,ViewType);
        }
    }

    if (!multicontrolpoints)
    {
        delete[] mcontrolpoints;
        maxmulticontrolpoints = 0;
        mcontrolpoints = NULL;
        return false;
    }

    m->saveForUndo(const_cast<char *> ("Manipulate Edge"),UNDO_BRUSHES);

    return true;
}

bool TXYViewWindow::vertexDragFrom(vec3_t tp)
{
    vec3_t dragpoint;

    map *m = map_i[set.curmap];
    if (m->numSelected() != 1)
        return false;

    SetBrush *b = m->selectedBrush();

    dragpoint[UAxis] = tp[0];
    dragpoint[VAxis] = tp[1];
    dragpoint[NormalAxis] = 0.0;

    if (!b || b->IsInvalid())
        return false;

    // make a copy and if when done it's bad, delete and replace with copy...
    b->getXYdragvertex(dragpoint,ViewType);
    // must have at least one "control point"...
    if (!numcontrolpoints)
        return false;

    m->saveForUndo(const_cast<char *> ("Manipulate Vertex"),UNDO_BRUSHES);

    return true;
}

bool TXYViewWindow::shearDragFrom(vec3_t tp)
{
    vec3_t	 dragpoint;
    vec3_t	 p1, p2;
    float	 time;
    SetBrush *br;
    face_t   *face;

    map *m = map_i[set.curmap];
    if (m->numSelected() != 1)
        return false;
    br = m->selectedBrush();

    // if the XY point is inside the brush, make the point on top
    p1[UAxis] = tp[0];
    p1[VAxis] = tp[1];
    VectorCopy (p1, p2);

    p1[NormalAxis] = -2048*xy_viewnormal[NormalAxis];
    p2[NormalAxis] = 2048*xy_viewnormal[NormalAxis];

    VectorCopy (p1, dragpoint);
    br->hitByRay(p1,p2,&time,&face);

    if (time > 0)
    {
        dragpoint[NormalAxis] = p1[NormalAxis] + (time-0.01f)*xy_viewnormal[NormalAxis];
    }
    else
    {
        br->getMins(p1,p2);
        dragpoint[NormalAxis] = (p1[NormalAxis] + p2[NormalAxis])/2;
    }

    br->getXYShearPoints(dragpoint,ViewType);
    if (!numcontrolpoints)
        return false;

    m->saveForUndo(const_cast<char *> ("Shear Face"),UNDO_BRUSHES);

    return true;
}



void TXYViewWindow::StartMouse(UINT modkeys, LPPOINT point, int whichMouse)
{
    vec3_t pt;
    SetBrush *ent;
    vec3_t	p1, p2;

    if (!set.Map_Read || mousedown)
        return;
    map *m = map_i[set.curmap];

    bool doubleclick = (0 != (whichMouse & DOUBLECLICK));
    whichMouse &= ~DOUBLECLICK;

    ug = false;      // assume no use of grid

    // convert from screen to world units
    ConvertPoint(point,pt);

    // set starting point
    startpt[0] = pt[0];
    startpt[1] = pt[1];
    startAngle = 0.0;
    delta[0]       = delta[1]       = 0;
    relative[0]    = relative[1]    = 0;
    oldrelative[0] = oldrelative[1] = 0;

    p1[UAxis] = p2[UAxis] = pt[0];
    p1[VAxis] = p2[VAxis] = pt[1];
    p1[NormalAxis] = xy_viewnormal[NormalAxis] * -TMAX;//-4096;
    p2[NormalAxis] = xy_viewnormal[NormalAxis] * TMAX;//4096;

    alreadySelecting = false;
    // search for the correct setting...
    unsigned char flags = 0x00;

    if (modkeys & (MK_SHIFT))
        flags |= SHIFT_FLAG;

    if (modkeys & (MK_CONTROL))
        flags |= CTRL_FLAG;

    if(0x8000 & GetKeyState(VK_MENU))
        flags |= ALT_FLAG;

    mouseEventType *p = mouse_events[curEventType];

    while (p)
    {
        if ((p->whichMouse == whichMouse) && (p->moveFlags == flags))
            break;
        p = p->next;
    }

    if (!p)   // not found...
        return;
    // parse out the string...
    int action;
    bool handled;
    for (int i = 0; i < p->count; i++)
    {
        action = TranslateCommand(p->moves[i]);
        handled = false;

        switch (action)
        {
        case CAMERA_CALLBACK:
            handled = cameraDragFrom(pt);
            ug = false;
            break;
        case CLIPPT_CALLBACK:
            handled = m->clipper_i->XYDrag(pt,UAxis,VAxis,index);
            m->saveForUndo(NULL,UNDO_NOUNDO);
            ug = true;
            break;
        case CLIPWHOLE_CALLBACK:
            handled = m->clipper_i->XYDragWhole(pt,UAxis,VAxis,index);
            m->saveForUndo(NULL,UNDO_NOUNDO);
            ug = true;
            break;
        case VERTEX_CALLBACK:
            if (set.vertex_select_mode)
            {
                if((handled = vertexDragFrom(pt)))
                {
                    if(!set.snap_back)	// just use "snap_back" if set instead of following..
                        ug = (0 != set.vertex_use_grid);
                    status->SetText("Drag Vertex");
                }
            }
            break;
        case EDGEDRAG_CALLBACK:
            if((handled = edgeDragFrom(pt)))
            {
                ug = (0 != set.edge_use_grid);
                status->SetText("Drag Edge");
            }
            break;
        case BRUSHDRAG_CALLBACK:
            ent = m->grabRay(p1,p2);
            if (ent && (handled = selectionDragFrom(pt)))
            {
                ug = true;
                status->SetText("Drag Brush");
            }
            break;
        case FACEDRAG_CALLBACK:
            if((handled = planeDragFrom(pt)))
            {
                ug = true;
                status->SetText("Drag Face");
            }
            break;
        case MULTIFACEDRAG_CALLBACK:
        case MULTIFACEDRAG_CALLBACKSTRICT:
            if((handled = planeDragFromMulti(pt, action)))
            {
                ug = true;
                status->SetText("Drag Multiple Face");
            }
            break;
        case NEW_CALLBACK:
            if (m->numSelected() == 0 && m->current->modifiable)
            {
                handled = newBrushDragFrom(pt);
            }
            ug = true;
            break;
        case SELECT_CALLBACK:
        case SELECTTOTAL_CALLBACK:
            if(doubleclick)
            {
                //deselect all first
                m->makeGlobalPerform(SEL_DESELECT);
                m->setCurrentEntity(m->world);
            }
            SetTimer(hwnd,TIMER_SEL,set.select_delay,0);
            if (set.center_knobs_xy)
                m->selectKnob(p1,p2,UAxis,VAxis,true);
            else
                m->selectRay(p1,p2,true);

            // save the current mouse pos and then only set timer if no movement...
            //	mousedown = action;
            handled = selBrushDragFrom(pt);
            if (handled)
                UpdateWindows();
            ug = false;
            break;
        case ADDCLIPPOINT_CALLBACK:
            if(set.animate_clip_points)
            {
                //animate clip points in each xy window
                for (int j = 0; j < set.xyViews; j++)
                {
                    KillTimer(xyWindow[j]->hwnd,TIMER_CLIP);
                    SetTimer(xyWindow[j]->hwnd,TIMER_CLIP,50,0);
                }
            }
            m->clipper_i->XYClick(pt,UAxis,VAxis,NormalAxis,index);

            UpdateWindows();

            // allow drag to create 2nd or 3rd point on mouseup
            SetTimer(hwnd,TIMER_ADDCLIP,300,0);
            handled = true;
            break;
        case ROTATE_CALLBACK:
            if((handled = vertexRotate(pt)))
            {

                m->getSelectedCenter();
                sb_ctr[0] = (float) snapToGrid(sb_ctr[0]);
                sb_ctr[1] = (float) snapToGrid(sb_ctr[1]);
                sb_ctr[2] = (float) snapToGrid(sb_ctr[2]);

                startAngle = angle(pt[0] - sb_ctr[UAxis],pt[1] - sb_ctr[VAxis]);
                ug = false;
                status->SetText("Rotate Brush");
            }
            break;
        case ENTITY_CONNECT:
            handled = m->entityConnect(p1,p2);
            if (handled)
            {
                m->saveForUndo(NULL,UNDO_NOUNDO);
                UpdateWindows();
                return;
            }
            break;
        case CURFACEDRAG_CALLBACK:
            if((handled = currentFaceDragFrom()))
            {
                ug = true;
                status->SetText("Drag Current Face");
            }
            break;
        case SHEAR_CALLBACK:
            if((handled = shearDragFrom(pt)))
            {
                ug = true;
                status->SetText("Shear Brush");
            }
            break;
        case MOVECAMERA_CALLBACK:
            if(!set.xy_drag_move)  				// old style
            {
                xy_eye[UAxis] = pt[0];
                xy_eye[VAxis] = pt[1];
                MoveEyePosition(xy_eye,set.lock_cameras);
                UpdateWindows();
                return;	//no further drawing
            }
            else  							// new style
            {
                handled = true;
                VectorCopy(xy_eye, xydrag_eye);

                xydrag_offset_U = xy_eye[UAxis] + pt[0];	// get U/V offset values
                xydrag_offset_V = xy_eye[VAxis] + pt[1];
            }
            ug = false;
            break;
        case DIRECTION_CALLBACK:
            handled = directionDragFrom(pt);
            ug = false;
            break;
        case TEXTUREBRUSH_CALLBACK:
            m->saveForUndo(NULL,UNDO_NOUNDO);
            m->setTextureRay(p1,p2,true);
            UpdateWindows();
            ug = false;
            return;
        case TEXTUREFACE_CALLBACK:
            m->saveForUndo(NULL,UNDO_NOUNDO);
            m->setTextureRay(p1,p2,false);
            UpdateWindows();
            ug = false;
            return;
        }

        if (handled)
            break;
    }

    if (handled)
    {
        if (ug)
        {
            snapPoint(startpt);
            snapPoint(pt);
        }
        mousedown = action;
        SetCapture(hwnd);
    }
}

void TXYViewWindow::StopMouse(LPPOINT point, int /*whichouse*/)
{
    int i,j;
    int retval;
    SetBrush *b;
    Entity *e;

    if (!set.Map_Read || !mousedown)
        return;
    map *m = map_i[set.curmap];

    ReleaseCapture();

    // finish up
    switch (mousedown)
    {
    case DIRECTION_CALLBACK:
        for (i = 0; i < set.xyViews; i++)
        {
            xyWindow[i]->RedrawContents();
        }
        editWindow->RedrawContents();
        break;
    case NEW_CALLBACK:
        newbrush->removeUnusedFaces();
        if (newbrush->IsInvalid())
        {
            m->current->removeObject(newbrush);
            delete newbrush;
            newbrush = NULL;
        }

        if (newbrush && set.group_mode && !m->groups->GetVisible(newbrush->group))
        {

            retval = MessageBox(hwnd,"New Brush added to invisible group, make invisible group visible?",
                                "BSP - New Brush", MB_YESNO | MB_ICONQUESTION);

            if (retval == IDYES)
            {
                m->groups->SetVisible(newbrush->group,true);
                m->UpdateMapVisibility();

                if (groupWindow)
                    groupWindow->RedrawReload();

            }
        }
        newbrush = NULL;
        break;
    case VERTEX_CALLBACK:
        // vertex... // need a sub-mode...
        b = m->selectedBrush();
        if (b)
            b->removeUnusedFaces();
        break;
    case SHEAR_CALLBACK:
        // shear...
        b = m->selectedBrush();
        if (b)
            b->removeUnusedFaces();
        break;
    case FACEDRAG_CALLBACK:
    case CURFACEDRAG_CALLBACK:
        b = m->selectedBrush();
        if (b)
            b->removeUnusedFaces();
        break;
    case MULTIFACEDRAG_CALLBACK:
    case MULTIFACEDRAG_CALLBACKSTRICT:
    case EDGEDRAG_CALLBACK:
        for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
        {
            LoopProblem("EDCALLBACK");
            if (!e->modifiable)
                continue;

            for (b = e->objects.p_next; !e->invalid && b && (b != &e->objects); b = b->p_next)
            {
                LoopProblem("EDCALLB");
                if (!b->IsDrawable())
                    continue;
                if (!b->IsSelected())
                    continue;
                b->removeUnusedFaces();
            }
        }


        multicontrolpoints = 0;
        if (mcontrolpoints)
        {
            delete[] mcontrolpoints;
            maxmulticontrolpoints = 0;
            mcontrolpoints = NULL;
        }
        break;
    case ROTATE_CALLBACK:
        if (rotateMap)
        {
            m->rotateBrushOrg((int)rpy[1],(int)rpy[0],(int)rpy[2],sb_ctr);
            delete rotateMap;
            rotateMap = NULL;
        }
        // do the rotation...
        break;
    case CAMERA_CALLBACK:
        vec3_t v;

        ConvertPoint(point,v);

        int ua, va, na;
        GetAxes(ViewType, &ua, &va, &na);
        m->eye[eyetomove][ua] = v[0];
        m->eye[eyetomove][va] = v[1];
        m->cureye = eyetomove;

        MoveEyePosition(v,set.lock_cameras);

        findQ(set.Q,set.R,m->eye[m->cureye],
              m->angles[m->cureye].roll,
              m->angles[m->cureye].pitch,
              m->angles[m->cureye].yaw);
        break;
    case SELECT_CALLBACK:
    case SELECTTOTAL_CALLBACK:
        // kill timer
        KillTimer(hwnd,TIMER_SEL);

        // check size of selBrush, if zero, leave, if not, deselect
        // starting brush
        if (selBrush && !selBrush->IsInvalid())
        {
            selBrush->getMins(select_min,select_max);
            m->dirty = 1;
            select_deselect = true;
            if (mousedown == SELECT_CALLBACK)
                m->makeAllPerform(SEL_SELECTPARTIAL);
            else
                m->makeAllPerform(SEL_SELECTCOMPLETE);

            select_deselect = false;
            m->UpdateCurrent();		// In case only selected brush is a
            // non world brush and world was current...
        }
        delete selBrush;
        selBrush = NULL;
        m->lookAtSelection();
        // ignoreBounds = false;
        break;
    case CLIPPT_CALLBACK:
        m->clipper_i->pos[m->curclippoint][UAxis] = (float) snapToGrid(m->clipper_i->pos[m->curclippoint][UAxis]);
        m->clipper_i->pos[m->curclippoint][VAxis] = (float) snapToGrid(m->clipper_i->pos[m->curclippoint][VAxis]);
        break;
    case CLIPWHOLE_CALLBACK:
        for (j=0; j < 3; j++)
        {
            m->clipper_i->pos[j][UAxis] = (float) snapToGrid(m->clipper_i->pos[j][UAxis]);
            m->clipper_i->pos[j][VAxis] = (float) snapToGrid(m->clipper_i->pos[j][VAxis]);
        }
        break;
        //  case MOVECAMERA_CALLBACK:
        //   	break;
    }

    m->cleanUpInvalidObjects();

    status->SetText(STATUS_DELTA, "");
    status->Redraw();
    mousedown = NO_MOUSE; // reset...

    UpdateWindows();
}
//TODO - redo ? #include "mouse.h"
void TXYViewWindow::ParseMouse()
{
    if (MouseParsed)
        return;

    MouseParsed = true;

    char *dat = (char*) file_get_contents(const_cast<char *> ("settings\\bspmouse.cfg"));
    if(!dat)
    {
        syserror(const_cast<char *> ("XY mouse: failed to open settings\\bspmouse.cfg"));
        return;
    }

    Tokenizer script(dat);
    if (!script.next(true))
    {
        syserror(const_cast<char *> ("XY mouse: Number of Setups not found"));
        delete [] dat;
        return;
    }

    numEventTypes = atoi(script.token);
    if (numEventTypes <= 0 || numEventTypes > MAX_MOUSE_EVENT_TYPES)
    {
        syserror(const_cast<char *> ("XY Mouse: Number of Setups [1 min/ 10 max] invalid"));
        delete [] dat;
        return;
    }
    memset(mouse_events,0,numEventTypes * sizeof(mouseEventType *));
    memset(EventTypes,0,numEventTypes * sizeof(char *));

    if (!script.next(true))
    {
        syserror(const_cast<char *> ("Parsing Mouse: Current Setup not found"));
        delete [] dat;
        return;
    }
    curEventType = atoi(script.token);
    if (curEventType <= 0 || curEventType > numEventTypes)
    {
        syserror(const_cast<char *> ("Parsing Mouse: Current Setup invalid"));
        delete [] dat;
        return;
    }

    curEventType--; // adjust down...

    for (int i = 0; i < numEventTypes; i++)
    {
        mouseEventType *hd = NULL;
        mouseEventType *cur = NULL, *temp;

        if (!script.next(true))
        {
            syserror(const_cast<char *> ("Parsing Mouse: '{' not found"));
            break;
        }
        if (!script.next(true))
        {
            syserror(const_cast<char *> ("Parsing Mouse: Current Setup Name not found"));
            break;
        }

        memset(EventTypes[i],0,32 * sizeof(char));
        strncpy(EventTypes[i],script.token,sizeof(EventTypes[i]));

        while (1)
        {
            if (!script.next(true))
                break;  // done...

            if (strcmpi (script.token, "{"))
                break;

            if (!script.next(false))
                break;  // done...

            temp = cur;
            cur = new mouseEventType;
            if (hd)		//TODO: i think this is hacking a "head" insert.... fix1!!@1321
                temp->next = cur;
            else
                hd = cur;

            cur->count = CountWads(script.token);
            if (cur->count <= 0)
            {
                syserror(const_cast<char *> ("Parse Mouse:  Not enough mouse options..."));
                break;
            }
            if (cur->count >= MAX_MOUSE_EVENT_TYPES)
            {
                syserror(const_cast<char *> ("Parse Mouse:  Too many mouse options [%i]..."), cur->count);
                break;
            }

            cur->moves = new char *[cur->count];
            if (!cur->moves)
            {
                syserror(const_cast<char *> ("Parse Mouse:  Couldn't allocate moves..."));
                delete [] dat;
                return;
            }
            memset(cur->moves,0,cur->count*sizeof(char *));

            int counter;
            char *start = script.token;
            char *end;
            for (counter = 0; counter < cur->count; counter++)
            {
                cur->moves[counter] = new char[32];
                if (!cur->moves[counter])
                {
                    syserror(const_cast<char *> ("Parse Mouse:  Couldn't allocate moves[%i]..."), counter);
                    delete [] dat;
                    return;
                }
                memset(cur->moves[counter],0,32*sizeof(char));

                for (end = start; end && *end && *end != ';' && *end != ' '; end++);

                strncpy(cur->moves[counter],start,(int)(end-start));
                cur->moves[counter][(int)(end-start)] = '\0';

                start = ++end;
            }

            if (!script.next(false))
                break;  // done...

            if (!strcmpi(script.token,"LEFT"))
                cur->whichMouse = 0;
            else if (!strcmpi(script.token,"MIDDLE"))
                cur->whichMouse = 1;
            else if (!strcmpi(script.token,"RIGHT"))
                cur->whichMouse = 2;
            else if (!strcmpi(script.token,"X1"))
                cur->whichMouse = 3;
            else if (!strcmpi(script.token,"X2"))
                cur->whichMouse = 4;
            else
            {
                syserror(const_cast<char *> ("XY Mouse Parse:  Bad mouse type \"%s\""), script.token);
                break;
            }

            while (1)
            {
                if (!script.next(false))
                    break;  // done...

                if (!strcmpi(script.token,"}"))
                    break;

                if (!strcmpi(script.token,"SHIFT"))
                    cur->moveFlags |= SHIFT_FLAG;
                else if (!strcmpi(script.token,"CTRL"))
                    cur->moveFlags |= CTRL_FLAG;
                else if (!strcmpi(script.token,"ALT"))
                    cur->moveFlags |= ALT_FLAG;
            }

        }
        mouse_events[i] = hd;
    } // end for...

    delete[] dat;
}

/* Possible command strings are:
   NEWBRUSH
   FACEDRAG
   BRUSHDRAG
   SHEAR
   CURFACEDRAG
   VERTEXDRAG
   ROTATE
   MOVECAMERA
   CAMERADIRECTION
   CLIPPOINTDRAG
	CLIPPLANEDRAG
   CAMERADRAG
   ENTITYCONNECT

   SELECT
   ADDCLIPPOINT
   TEXTUREBRUSH
   TEXTUREFACE
*/

int TXYViewWindow::TranslateCommand(char *in)
{
    if (!strcmpi(in,"OBJECTTOOL"))
        return OBJECT_CALLBACK;
    if (!strcmpi(in,"KNOB"))
        return KNOB_CALLBACK;
    if (!strcmpi(in,"NEWBRUSH"))
        return NEW_CALLBACK;
    if (!strcmpi(in,"MULTIFACEDRAG"))
        return MULTIFACEDRAG_CALLBACK;
    if (!strcmpi(in,"MULTIFACEDRAGSTRICT"))
        return MULTIFACEDRAG_CALLBACKSTRICT;
    if (!strcmpi(in,"FACEDRAG"))
        return FACEDRAG_CALLBACK;
    if (!strcmpi(in,"BRUSHDRAG"))
        return BRUSHDRAG_CALLBACK;
    if (!strcmpi(in,"SHEAR"))
        return SHEAR_CALLBACK;
    if (!strcmpi(in,"CURFACEDRAG"))
        return CURFACEDRAG_CALLBACK;
    if (!strcmpi(in,"VERTEXDRAG"))
        return VERTEX_CALLBACK;
    if (!strcmpi(in,"EDGEDRAG"))
        return EDGEDRAG_CALLBACK;
    if (!strcmpi(in,"ROTATE"))
        return ROTATE_CALLBACK;
    if (!strcmpi(in,"MOVECAMERA"))
        return MOVECAMERA_CALLBACK;
    if (!strcmpi(in,"CAMERADIRECTION"))
        return DIRECTION_CALLBACK;
    if (!strcmpi(in,"SELECT"))
        return SELECT_CALLBACK;
    if (!strcmpi(in,"SELECTTOTAL"))
        return SELECTTOTAL_CALLBACK;
    if (!strcmpi(in,"ADDCLIPPOINT"))
        return ADDCLIPPOINT_CALLBACK;
    if (!strcmpi(in,"TEXTUREBRUSH"))
        return TEXTUREBRUSH_CALLBACK;
    if (!strcmpi(in,"TEXTUREFACE"))
        return TEXTUREFACE_CALLBACK;
    if (!strcmpi(in,"CLIPPOINTDRAG"))
        return CLIPPT_CALLBACK;
    if (!strcmpi(in,"CAMERADRAG"))
        return CAMERA_CALLBACK;
    if (!strcmpi(in,"ENTITYCONNECT"))
        return ENTITY_CONNECT;
    if (!strcmpi(in,"CLIPPLANEDRAG"))
        return CLIPWHOLE_CALLBACK;

    return NO_MOUSE;
}

// force redraw of all 4 edit views
void TXYViewWindow::UpdateWindows()
{
    if (!set.Map_Read)
        return;

    for (int i = 0; i < set.xyViews; i++)
        xyWindow[i]->RedrawContents();

    editWindow->RedrawContents();
}

mouseEventType::mouseEventType()
{
    count = 0;
    moves = NULL;
    whichMouse = 0; // left...
    moveFlags = 0;
    next = NULL;
}

mouseEventType::~mouseEventType()
{
    for (int i = 0; i < count; i++)
    {
        delete[] moves[i];
        moves[i] = NULL;
    }
    delete[] moves;
    moves = NULL;
    count = 0;
}

//////////////////
//New Caption stuff - overrides for TCWindow to handle grid + scale boxes
////////////////////
int TXYViewWindow::DoNCHitTest(LPPOINT screenPt)
{
    RECT rc;
    GetWindowRect(hwnd,&rc);
    POINT winPt;
    winPt.x = screenPt->x - rc.left;
    winPt.y = screenPt->y - rc.top;

    GetGridBoxRect(&rc);
    if(PtInRect(&rc,winPt))
        return HTGRIDBUTTON;

    GetScaleBoxRect(&rc);
    if(PtInRect(&rc,winPt))
        return HTSCALEBUTTON;

    return TCWindow::DoNCHitTest(screenPt);
}
//popup
bool TXYViewWindow::DoNCRButtonDown(int /*hitTest*/, LPPOINTS /*screenPt*/)
{
    if (IsIconic())
        return false;
    map *m = map_i[set.curmap];
    if(!m)
        return false;
    // create the submenus

    Popup AlignMenu;
    AlignMenu.Add("Tops", ALIGNTOP);
    AlignMenu.Add("Vertical Centers", ALIGNVCENTER);
    AlignMenu.Add("Bottoms", ALIGNBOTTOM);
    AlignMenu.Add("Left Sides", ALIGNLEFT);
    AlignMenu.Add("Horizontal Centers", ALIGNHCENTER);
    AlignMenu.Add("Right Sides", ALIGNRIGHT);

    Popup TopBottomMenu;
    TopBottomMenu.Add("Raise Top(s)", IDTB_TU);
    TopBottomMenu.Add("Raise Bottom(s)", IDTB_BU);
    TopBottomMenu.Add("Lower Top(s)", IDTB_TD);
    TopBottomMenu.Add("Lower Bottom(s)", IDTB_BD);

    Popup CameraMenu;
    if(m->cureye == 0)
        CameraMenu.Check();
    CameraMenu.Add("Camera 1",XY_1);
    if(m->cureye == 1)
        CameraMenu.Check();
    CameraMenu.Add("Camera 2",XY_2);
    if(m->cureye == 2)
        CameraMenu.Check();
    CameraMenu.Add("Camera 3",XY_3);
    if(m->cureye == 3)
        CameraMenu.Check();
    CameraMenu.Add("Camera 4",XY_4);
    if(m->cureye == 4)
        CameraMenu.Check();
    CameraMenu.Add("Camera 5",XY_5);
    CameraMenu.Separator();
    CameraMenu.Add( (set.show_cameras ? "Hide Cameras..." : "Show Cameras..."), XY_6);
    CameraMenu.Separator();
    CameraMenu.Add( (set.lock_cameras ? "Unlock Cameras..." : "Lock Cameras..."), XY_LOCK);

    Popup MouseMenu;
    for (int i = 0; i < numEventTypes; i++)
    {
        if(i == curEventType)
            MouseMenu.Check();
        MouseMenu.Add(EventTypes[i], XYMOUSEBASE+i);
    }

//todo: this doesnt work / not impld
    //flip view normal
//	static const char *dirnormal[] = {"Up", "Right", "Front",  "Down", "Left", "Back"};
//	int index = ViewType;					//urf - get viewtype
//	if(xy_viewnormal[NormalAxis] != 1.0)	//is normal reversed?
//		index += 3;							//dlr - next set

    // Main Menu
    Popup PopupMenu;
//	PopupMenu.Add(dirnormal[index],TB_CV);
    PopupMenu.Add("Camera",CameraMenu);
    PopupMenu.Separator();
    PopupMenu.Add("Align",AlignMenu);
    PopupMenu.Add("Top/Bottom",TopBottomMenu);
    PopupMenu.Separator();
    PopupMenu.Add("Display Settings",MOUSEHELPXY);
    //PopupMenu.Add("Print...",XYPRINT);
    PopupMenu.Add("Mouse",MouseMenu);

    PopupMenu.Show(hwnd);

    return true;
}

bool TXYViewWindow::DoNCLButtonDown(int hitTest, LPPOINTS screenPt)
{
    if (IsIconic() || !hwnd)
        return false;

    switch(hitTest)
    {
    case HTGRIDBUTTON:
        DownHit = HTGRIDBUTTON;
        CmG();
        return true;
    case HTSCALEBUTTON:
        DownHit = HTSCALEBUTTON;
        CmS();
        return true;
    }
    return TCWindow::DoNCLButtonDown(hitTest,screenPt);
}
void TXYViewWindow::PaintGridBox(HDC hdc)
{
    RECT boxRect;
    GetGridBoxRect(&boxRect);
    // Dont paint over the left & top borders
    //
    boxRect.left++;
    boxRect.top++;

    // Fill the box with 3d face
    //
    SetBkColor(hdc,GetSysColor(COLOR_3DFACE));
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &boxRect, 0, 0, 0);

    char gridstr[80];
    int l = sprintf(gridstr,"%i",set.gridsize);
    SetTextColor(hdc,GetSysColor(COLOR_BTNTEXT));
    SetBkColor(hdc,GetSysColor(COLOR_3DFACE));
//	HGDIOBJ old = SelectObject(hdc,hCaptionFont);

    boxRect.top--;
    DrawText(hdc,gridstr,l,&boxRect, DT_CENTER);
}

void TXYViewWindow::PaintScaleBox(HDC hdc)
{
    RECT boxRect;
    GetScaleBoxRect(&boxRect);

    // Dont paint over the left & top borders
    //
    boxRect.left++;
    boxRect.top++;

    // Fill the box with 3d face
    //
    SetBkColor(hdc,GetSysColor(COLOR_3DFACE));
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &boxRect, 0, 0, 0);

    char scalestr[80];
    sprintf(scalestr,"%i",set.gridsize);
    SetTextColor(hdc,GetSysColor(COLOR_BTNTEXT));
    SetBkColor(hdc,GetSysColor(COLOR_3DFACE));
//	HGDIOBJ old = SelectObject(hdc,hCaptionFont);

    int l = GetScaleText(scalestr);
    boxRect.top--;
    DrawText(hdc,scalestr,l,&boxRect, DT_CENTER);
}

void TXYViewWindow::PaintCaption(bool active)
{
    if(!hwnd || IsIconic())
        return;

    HDC hdc = GetWindowDC(hwnd);
    RECT captRect;
    GetCaptionRect(&captRect);

    SetTextColor(hdc, GetSysColor(active ? COLOR_CAPTIONTEXT : COLOR_INACTIVECAPTIONTEXT));

    SetBkColor(hdc, GetSysColor(active ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION));
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &captRect, 0, 0, 0);

    HGDIOBJ oldfont = SelectObject(hdc,g_hCaptionFont);

    SetBkMode(hdc,TRANSPARENT);

    // Calc x coord for text, so that text is centered between caption buttons
    //
    int style = GetWindowLong(hwnd,GWL_STYLE);
    int xOrg = captRect.left;
    RECT rc;

    GetSysBoxRect(&rc);
    xOrg += rc.right - rc.left;			//add sysbox width

    GetScaleBoxRect(&rc);
    xOrg +=  rc.right - rc.left;		//add scalebox width

    GetGridBoxRect(&rc);
    xOrg +=  rc.right - rc.left;		//add gridbox width

    ::ExtTextOut(hdc, xOrg, captRect.top-BORDER_Y, ETO_CLIPPED, &captRect,
                 Window::caption, strlen(Window::caption), 0);

    // Paint widgets: sysmenu or close button, minimize button, maximize button
    // They currently all use a black pen
    //
    HGDIOBJ oldpen = SelectObject(hdc,GetStockObject(BLACK_PEN));

    // Paint system menu or close button
    if (style & WS_SYSMENU)
        PaintSysBox(hdc);

    // Paint minimize button
    if (style & WS_MINIMIZEBOX)
        PaintMinBox(hdc, false);

    // Paint maximize button
    if (style & WS_MAXIMIZEBOX)
        PaintMaxBox(hdc, false);

    PaintGridBox(hdc);
    PaintScaleBox(hdc);

    // Draw window-frame color line under caption
    //
    captRect.top = captRect.bottom-1;
    FrameRect(hdc,&captRect, GetSysColorBrush(COLOR_WINDOWFRAME));
    SelectObject(hdc,oldpen);
    SelectObject(hdc,oldfont);
    ReleaseDC(hwnd,hdc);
}

void TXYViewWindow::GetGridBoxRect(LPRECT boxRect)
{
    GetCaptionRect(boxRect);
    const int multiplier = 3;
    // Move over away from sysbox...
    OffsetRect(boxRect,CaptionHeight,0);
    boxRect->right = boxRect->left + multiplier*CaptionHeight;
    boxRect->top -= 1;
    boxRect->bottom -= 1;
}
void TXYViewWindow::GetScaleBoxRect(LPRECT boxRect)
{
    GetGridBoxRect(boxRect);
    const int multiplier = 4;
    OffsetRect(boxRect,boxRect->right-boxRect->left,0);
    boxRect->right = boxRect->left + multiplier*CaptionHeight;
}


//////////////////////

// from camerawindow... add to menus
void TXYViewWindow::FaceTransFunc(char *actionmsg,char *savemsg,int gridsign,int zsign)
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];
    if (!m->numSelected())
        return;

    vec3_t mins, maxs;
    vec3_t dragpoint;

    Entity *e;
    SetBrush *b;

    m->saveForUndo(savemsg,UNDO_BRUSHES);
    for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
    {
        if (!e->modifiable)
            continue;

        for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
        {
            if (!b->IsDrawable() || !b->IsSelected())
                continue;

            b->getMins(mins,maxs);

            dragpoint[0] = (mins[0]+maxs[0])/2.0f;
            dragpoint[1] = (mins[1]+maxs[1])/2.0f;
            dragpoint[2] = maxs[2]+zsign*2;

            b->getZdragface(dragpoint,Type_XY);
            if (!numcontrolpoints)
                continue;

            for (int k=0 ; k<numcontrolpoints ; k++)
                controlpoints[k][2] += gridsign*set.gridsize;

            b->calcWindings();
            b->removeUnusedFaces();
        }
    }

    m->cleanUpInvalidObjects();

    editWindow->RedrawContents();
    for (int i = 0; i < set.xyViews; i++)
        xyWindow[i]->RedrawContents();

    status->SetText(actionmsg);
    status->Redraw();
}
void TXYViewWindow::BnTopUp()
{
    FaceTransFunc(const_cast<char *> ("Raise Top Face(s)"),const_cast<char *> ("Top face(s) raised..."),1,1);
}
void TXYViewWindow::BnTopDown()
{
    FaceTransFunc(const_cast<char *> ("Lower Top Face(s)"),const_cast<char *> ("Top face(s) lowered..."),-1,1);
}
void TXYViewWindow::BnBottomUp()
{
    FaceTransFunc(const_cast<char *> ("Raise Bottom Face(s)"),const_cast<char *> ("Bottom face(s) raised..."),1,-1);
}
void TXYViewWindow::BnBottomDown()
{
    FaceTransFunc(const_cast<char *> ("Lower Bottom Face(s)"),const_cast<char *> ("Bottom face(s) lowered..."),-1,-1);
}
