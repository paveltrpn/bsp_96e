#include "global.h"
#include "propctl.h"

Window::Window(HWND parent, const char *caption)
{
    created = false;

    hwnd = 0;

    SetCaption(caption);

    wc.cbClsExtra = 0;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND+1;
    wc.hCursor = LoadCursor(0,IDC_ARROW);
    wc.hIconSm =
        wc.hIcon = LoadIcon(0,IDI_APPLICATION);
    wc.hInstance = hInstance;
    wc.lpfnWndProc = MsgRouter;
    wc.lpszClassName = "BSP_WIN";
    wc.lpszMenuName = 0;
    wc.style = 0;//CS_VREDRAW | CS_HREDRAW;

    attr.parent = parent;
    attr.dwExStyle = 0;
    attr.dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
    attr.width = CW_USEDEFAULT;
    attr.height = CW_USEDEFAULT;
    attr.x = CW_USEDEFAULT;
    attr.y = CW_USEDEFAULT;
    attr.menu = 0;
    attr.param = this;
}
Window::~Window()
{
    if(hwnd)
    {
#ifdef _WIN64
        ::SetWindowLong(hwnd, GWLP_USERDATA, 0);		//stop using class pointer
#else
        ::SetWindowLong(hwnd, GWL_USERDATA, 0);		    //stop using class pointer
#endif
        DestroyWindow(hwnd);
    }
}

LRESULT Window::SendMessage(UINT msg,WPARAM wParam,LPARAM lParam)
{
    return ::SendMessage(hwnd,msg,wParam,lParam);
}
int Window::ShowWindow(int nCmdShow)
{
    return ::ShowWindow(hwnd,nCmdShow);
}
bool Window::IsValid()
{
    return (created && hwnd && ::IsWindow(hwnd));
}
bool Window::IsWindowVisible()
{
    return (hwnd && ::IsWindowVisible(hwnd));
}
bool Window::IsVisible()
{
    return !IsIconic() && IsWindowVisible();
}
HWND Window::SetFocus()
{
    return ::SetFocus(hwnd);
}
bool Window::SetWindowPos(HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
    return !!::SetWindowPos(hwnd,hWndInsertAfter,X,Y,cx,cy,uFlags);
}
bool Window::SetWindowPos(HWND hWndInsertAfter, RECT *rc, UINT uFlags)
{
    return !!::SetWindowPos(hwnd,hWndInsertAfter,rc->left,rc->top,rc->right-rc->left,rc->bottom-rc->top,uFlags);
}
bool Window::CanClose()
{
    return true;
}
LRESULT CALLBACK Window::MsgRouter(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    Window *wnd;
    if(msg == WM_CREATE)
    {

        LPCREATESTRUCT lp = (LPCREATESTRUCT)lParam;

        //get creation param. mdi children use different structure
        if(lp->dwExStyle & WS_EX_MDICHILD)
        {
            wnd = (Window*)((LPMDICREATESTRUCT)lp->lpCreateParams)->lParam;
        }
        else
        {
            wnd = (Window*)lp->lpCreateParams;
        }
        if(!wnd)
        {
            syserror(const_cast<char *> ("Window failed to initialize"));
            return 0;
        }
        else
        {
#ifdef _WIN64
            SetWindowLong(hwnd, GWLP_USERDATA, (LONG_PTR)wnd);
#else
            SetWindowLong(hwnd, GWL_USERDATA, (LONG)wnd);
#endif
            wnd->hwnd = hwnd;
        }
    }
    else
    {
#ifdef _WIN64
        wnd = (Window*)GetWindowLong(hwnd, GWLP_USERDATA);
#else
        wnd = (Window*)GetWindowLong(hwnd, GWL_USERDATA);
#endif
    }

    //this bypasses the first few messages until WM_CREATE is hit
    if(!wnd)
        return DefWindowProc(hwnd,msg,wParam,lParam);

    return wnd->WndProc(msg,wParam,lParam);
}


bool Window::Create(bool show)
{
    if(created)
        return false;
    created = true;

    WNDCLASS wndclass;
    if(!::GetClassInfo(wc.hInstance,wc.lpszClassName,&wndclass))  	//dont re-register the same class
    {
        if(!::RegisterClassEx(&wc))
        {
            return false;
        }
    }
    hwnd = ::CreateWindowEx(attr.dwExStyle,wc.lpszClassName,caption,attr.dwStyle,
                            attr.x,attr.y,attr.width,attr.height,attr.parent,attr.menu,wc.hInstance,attr.param);

    if(!hwnd)
        return false;

    int swp_flags = SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER;
    if(show)
        swp_flags |= SWP_SHOWWINDOW;
    ::SetWindowPos(hwnd,0,0,0,0,0,swp_flags);
    return true;
}

void Window::SetCaption(const char *caption)
{
    *this->caption = 0;
    if(caption)
        strncpy(this->caption,caption,sizeof(this->caption));
    if(hwnd)
        ::SetWindowText(hwnd,caption);
}
bool Window::IsIconic()
{
    return hwnd ? !!::IsIconic(hwnd) : false;
}
bool Window::IsZoomed()
{
    return hwnd ? !!::IsZoomed(hwnd) : false;
}
void Window::SetIcon(int resid)
{
    if(hwnd)
    {
        HICON ico = LoadIcon(wc.hInstance,MAKEINTRESOURCE(resid));
        SendMessage(WM_SETICON,ICON_BIG,(LPARAM)ico);
    }
}

//
// WFloatingList
//
WFloatingList::WFloatingList(HWND parent)
    : Window(parent, 0)
{
    attr.dwExStyle =  WS_EX_TOOLWINDOW;
    attr.dwStyle = WS_BORDER|WS_CLIPCHILDREN|WS_POPUP;
    wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    wc.lpszClassName = "BSP_WFloatingList";
    attr.width = 0;
    attr.height = 0;
    attr.x = 0;
    attr.y = 0;
    part = 0;
    mdown = false;
}
WFloatingList::~WFloatingList()
{
    delete list;
}
#define CORNER_CX 22
LRESULT WFloatingList::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_CREATE:
        list = new WList(hwnd,100, 0,0,0,0);
        list->attr.dwExStyle = 0;
        list->attr.dwStyle = LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|WS_CHILDWINDOW|WS_GROUP|WS_TABSTOP
                             |WS_VISIBLE|WS_VSCROLL |WS_HSCROLL; //TODO: hscroll never shows?
        list->Create();
        break;
    case WM_SIZE:
        if(list)
        {
            RECT rc;
            GetClientRect(hwnd, &rc);
            MoveWindow(list->hwnd,0,0,rc.right,rc.bottom - 5, 1);
            Redraw();
        }
        break;
    case WM_KILLFOCUS:
    {
        HWND focus = GetFocus();
        if(focus != list->hwnd)
            ShowWindow(SW_HIDE);
    }
    break;
    case WM_PAINT:
        //now decorate handle
        Redraw();
        ValidateRect(hwnd, 0);
        return 0;
    case WM_LBUTTONDOWN:
        mdown = true;
        POINT pt;
        GetCursorPos(&pt);
        startx = pt.x;
        starty = pt.y;
        SetCapture(hwnd);
        {
            //	RECT rc;
            GetWindowRect(hwnd,&startrc);
            if(startx < startrc.left+CORNER_CX)
                part = 0;
            else if(startx < startrc.right-CORNER_CX)
                part = 1;
            else
                part = 2;
        }
        break;
    case WM_LBUTTONUP:
        ReleaseCapture();
        mdown = false;
        break;
    case WM_MOUSEMOVE:
    {
        RECT rc;
        GetClientRect(hwnd,&rc);
        int mx = LOWORD(lParam);
        if(mx < rc.left+CORNER_CX)
            SetCursor(LoadCursor(0,IDC_SIZENESW));
        else if(mx < rc.right-CORNER_CX)
            SetCursor(LoadCursor(0,IDC_SIZEALL));
        else
            SetCursor(LoadCursor(0,IDC_SIZENWSE));
    }
    if(mdown)
    {
        POINT pt;
        GetCursorPos(&pt);
        int dx = pt.x - startx, dy = pt.y - starty;
        int left = startrc.left;
        int top = startrc.top;
        int width = startrc.right-startrc.left;
        int height = startrc.bottom-startrc.top;
        if(part == 0)  			//size bottom left
        {
            left = min(left + width - 72, left + dx);
            width = max(72, width - dx);
            height = max(32, height + dy);
        }
        else if(part == 1)  	//move window
        {
            left += dx;
            top += dy;
        }
        else if(part == 2)  	//size bottom right
        {
            width = max(72, width + dx);
            height = max(32, height + dy);
        }
        MoveWindow(hwnd, left, top, width, height, 1);
    }
    break;
    case WM_COMMAND:
        switch(HIWORD(wParam))
        {
        case LBN_DBLCLK:
            //		sysprintf("LBN_DBLCLK\r\n");
            int id = list->GetSelIndex();
            ::SendMessage(attr.parent, PM_LIST_CHANGE, id, 0);
            break;
            //	case LBN_SELCANCEL:
            //		sysprintf("LBN_SELCANCEL\r\n");
            //		break;
            //	case LBN_SELCHANGE:
            //		sysprintf("LBN_SELCHANGE\r\n");
            //		break;
            //	case LBN_ERRSPACE:
            //	case LBN_KILLFOCUS:
            //	case LBN_SETFOCUS:
            //		break;
            //forward message to parent
            //		::SendMessage(attr.parent, WM_COMMAND, wParam, lParam);
        }
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
void WFloatingList::Redraw()
{
    RECT rc,fillrc;
    GetClientRect(hwnd, &rc);
    CopyRect(&fillrc,&rc);
    fillrc.top = fillrc.bottom - 5;

    HDC hdc = GetDC(hwnd);

    SetBkColor(hdc,GetSysColor(COLOR_BTNFACE));
    ExtTextOut(hdc,0,0,ETO_OPAQUE,&fillrc,"",0,0);

    HPEN hi = CreatePen(PS_SOLID,1,GetSysColor(COLOR_3DHILIGHT));
    HPEN lo = CreatePen(PS_SOLID,1,GetSysColor(COLOR_3DSHADOW));

    //draw shadows
    HGDIOBJ oldpen = SelectObject(hdc,lo);
    MoveToEx(hdc, CORNER_CX, rc.bottom-5, 0);
    LineTo(hdc, CORNER_CX,rc.bottom);
    MoveToEx(hdc, rc.right-CORNER_CX-1, rc.bottom-5, 0);
    LineTo(hdc, rc.right-CORNER_CX-1,rc.bottom);
    MoveToEx(hdc, 0, rc.bottom-1, 0);
    LineTo(hdc, rc.right-1, rc.bottom-1);
    MoveToEx(hdc, rc.right-1, rc.bottom-5, 0);
    LineTo(hdc, rc.right-1, rc.bottom);

    //draw highlights
    SelectObject(hdc,hi);
    MoveToEx(hdc, CORNER_CX+1, rc.bottom-5, 0);
    LineTo(hdc, CORNER_CX+1,rc.bottom);
    MoveToEx(hdc, rc.right-CORNER_CX, rc.bottom-5, 0);
    LineTo(hdc, rc.right-CORNER_CX,rc.bottom);
    MoveToEx(hdc, 0, rc.bottom-5, 0);
    LineTo(hdc, 0, rc.bottom);
    MoveToEx(hdc, 0, rc.bottom-5, 0);
    LineTo(hdc, rc.right, rc.bottom-5);

    SelectObject(hdc,oldpen);
    DeleteObject(hi);
    DeleteObject(lo);
    ReleaseDC(hwnd, hdc);
}

//
// MDIClient
//
MDIClient::MDIClient(HWND parent)
    : Window(parent, 0)
{
    wc.lpszClassName = "MDICLIENT";
    wc.hbrBackground = (HBRUSH)COLOR_APPWORKSPACE+1;
    attr.dwExStyle = WS_EX_CLIENTEDGE;
    attr.dwStyle = MDIS_ALLCHILDSTYLES|WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|
                   WS_CLIPSIBLINGS|WS_VSCROLL|WS_HSCROLL;


    CLIENTCREATESTRUCT ccs;
    ccs.hWindowMenu = 0;
    ccs.idFirstChild = 20000;
    attr.param = (LPVOID)&ccs;
}
LRESULT MDIClient::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    sysfatal(const_cast<char *> ("MDIClient::WndProc stub called"));
    return 0;
}



//
// MDIChild
//
MDIChild::MDIChild(HWND parent, const char *caption)
    : Window(parent,caption)
{
    if (set.draw_old_style)
        Window::attr.dwExStyle |= WS_EX_MDICHILD | WS_EX_WINDOWEDGE;
    else
        Window::attr.dwExStyle |= WS_EX_MDICHILD;
}


LRESULT CALLBACK MDIChild::MsgRouter(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
#ifdef _WIN64
    Window *wnd = reinterpret_cast<Window *>(::GetWindowLong(hwnd, GWLP_USERDATA));
#else
    Window *wnd = reinterpret_cast<Window *>(::GetWindowLong(hwnd, GWL_USERDATA));
#endif
    if(!wnd)
        return DefMDIChildProc(hwnd,msg,wParam,lParam);		//mdi child must call this handler

    // call the windows message handler
    return wnd->WndProc(msg, wParam, lParam);
}

//
// TCWindow
//
HWND TCWindow::topmost = 0;
TCWindow::TCWindow(HWND parent, const char *caption)
    : MDIChild(parent,caption)
{
    if (set.draw_old_style)
    {
        attr.dwStyle &= ~WS_CAPTION;
        attr.dwStyle |= WS_BORDER;
    }
    wc.style = CS_DBLCLKS;

    //nc tracking stuff
    WaitingForSysCmd = false;
    DownHit = 0;
    IsPressed = false;
    docked = 0;
}
///////////////////////////////////////////////

LRESULT TCWindow::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    if (set.draw_old_style)
    {
        if(hwnd)
            switch(msg)
            {
            case WM_NCPAINT:
                DefMDIChildProc(hwnd,msg,wParam,lParam); //default painting first
                DoNCPaint();
                return 0;
            case WM_CLOSE:
                if(CanClose())
                    DestroyWindow(hwnd);
                return 0;
            case WM_WINDOWPOSCHANGED:
                if (TCWindow::topmost)
                {
                    WINDOWPOS* pWP = (WINDOWPOS*)lParam;
                    if (pWP->hwnd != TCWindow::topmost)
                        ::SetWindowPos (TCWindow::topmost, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
                }
                break;
            case WM_COMMAND:
            {
                int result;
                if (DoCommand(LOWORD(wParam), HIWORD(wParam), result))
                    return result;
            }
            break;
            case WM_NCCALCSIZE:
                DoNCCalcSize((LPNCCALCSIZE_PARAMS)lParam);
                return 0;

            case WM_NCACTIVATE:
                //	SetWindowLong(hwnd,GWL_STYLE,GetWindowLong(hwnd,GWL_STYLE)&~WS_CAPTION);
                //	SetWindowPos(0,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
                DoNCActivate(!!wParam);
                break;
            case WM_NCLBUTTONDOWN:


                if(DoNCLButtonDown((int) wParam, &MAKEPOINTS(lParam)))
                    return 0;
                break;
            case WM_NCRBUTTONDOWN:
                if(DoNCRButtonDown((int) wParam, &MAKEPOINTS(lParam)))
                    return 0;
                break;
            case WM_LBUTTONUP:
                if(DoLButtonUp())
                    return 0;
                break;
            case WM_MOUSEMOVE:
                if(DoMouseMove())
                    return 0;
                break;
            case WM_SYSCOMMAND:
                if(DoSysCommand((int) wParam))
                    return 0;
                break;
            case WM_NCHITTEST:
            {
                POINT screenPt;
                screenPt.x = LOWORD(lParam);
                screenPt.y = HIWORD(lParam);
                int ret = DoNCHitTest(&screenPt);
                if(ret > 0)
                    return ret;
            }
            break;
            case WM_SETTEXT:
                PaintCaption(GetFocus()==hwnd);
                break;
            case WM_MDIACTIVATE:
                PaintCaption((HWND)lParam == hwnd);
                break;
            }
    }
    else
    {
        if(hwnd)
            switch(msg)
            {
            case WM_CLOSE:
                if(CanClose())
                    break;
                else
                    return 0;
            }
    }

    return DefMDIChildProc(hwnd, msg,wParam,lParam);
}

//toggle a windows topmost status
bool TCWindow::ToggleAlwaysOnTop()
{
    if (TCWindow::topmost != hwnd)
    {
        TCWindow::topmost = hwnd;
        return true;
    }
    else
    {
        TCWindow::topmost = 0;
        return false;
    }
}

bool TCWindow::DoCommand(UINT id, int notifyCode, int& result)
{
    if (id >= 0xF000)  		//all SC_ commands
    {
        WaitingForSysCmd = false;  // Let LButtonDown handler know that a command was sent
        result = (int) SendMessage(WM_SYSCOMMAND, id, notifyCode);
        return true;
    }
    return false;
}

bool TCWindow::DoNCRButtonDown(int hitTest, LPPOINTS screenPt)
{
    return false;	//no impl
}
bool TCWindow::DoNCLButtonDown(int hitTest, LPPOINTS screenPt)
{
    if (IsIconic() || !hwnd)
        return false;

    HDC hdc;

    switch (hitTest)
    {
    case HTSYSMENU:
    {
        DownHit = HTSYSMENU;
        RECT sysBoxRect;
        GetSysBoxRect(&sysBoxRect);
        InflateRect(&sysBoxRect,-1,-1);
        sysBoxRect.right-=3;
        sysBoxRect.bottom-=4;

        hdc = GetWindowDC(hwnd);

        PatBlt(hdc, sysBoxRect.left,sysBoxRect.top,sysBoxRect.right,sysBoxRect.bottom, PATINVERT);

        // Display sys menu on button down
        // Need to lock sys menu until user clicks outside

        // Set flag to indicate we're expecting a sys command, & then send
        // message to popup sys menu
        //
        WaitingForSysCmd = true;
        SendMessage(WM_SYSCOMMAND, SC_MOUSEMENU|HTSYSMENU, (screenPt->x|(screenPt->y<<16)) );

        // If we didn't execute a command, user released btn outside of menu
        // If it was released in sys menu box, then redisplay menu as if it
        // were brought up with a keystroke
        //
        if (WaitingForSysCmd)
        {
            POINT pt;
            GetCursorPos(&pt);
            int test = DoNCHitTest(&pt);
            if (test == HTSYSMENU)
                SendMessage(WM_SYSCOMMAND, SC_KEYMENU|HTSYSMENU,0);

        }
        PatBlt(hdc, sysBoxRect.left,sysBoxRect.top,sysBoxRect.right,sysBoxRect.bottom, PATINVERT);
        ReleaseDC(hwnd,hdc);
        return true;
    }
    case HTMINBUTTON:
        DownHit = HTMINBUTTON;
        IsPressed = true;
        SetCapture(hwnd);

        hdc = GetWindowDC(hwnd);
        PaintMinBox(hdc, IsPressed);
        ReleaseDC(hwnd,hdc);
        return true;

    case HTMAXBUTTON:
        DownHit = HTMAXBUTTON;
        IsPressed = true;
        SetCapture(hwnd);

        hdc = GetWindowDC(hwnd);
        PaintMaxBox(hdc, IsPressed);
        ReleaseDC(hwnd,hdc);
        return true;
    }
    DownHit = HTNOWHERE;
    return false;
}
bool TCWindow::DoLButtonUp()
{
    if (DownHit > 0)
    {
        ReleaseCapture();
        DoMouseMove();

        POINT screenPt;
        GetCursorPos(&screenPt);
        int hitTest = DoNCHitTest(&screenPt);

        if (hitTest == DownHit)
        {
            DownHit = HTNOWHERE;
            switch (hitTest)
            {
                // We have to handle these buttons also to prevent defproc from
                // painting the standard big min/max buttons when left mouse button is
                // pressed
                //
            case HTMINBUTTON:
                SendMessage(WM_SYSCOMMAND, SC_MINIMIZE,0);
                return true;

            case HTMAXBUTTON:
                //undock docked window
                if(docked)
                {
                    docked->UnDock(this);
                    return false;
                }
                SendMessage(WM_SYSCOMMAND, IsZoomed() ? SC_RESTORE : SC_MAXIMIZE,0);
                //fall

            case HTSYSMENU:
                return true;
            }
        }
        DownHit = HTNOWHERE;
    }
    return false;
}

bool TCWindow::DoMouseMove()
{
    if (DownHit > 0)
    {

        POINT screenPt;
        GetCursorPos(&screenPt);
        int hitTest = DoNCHitTest(&screenPt);
        bool isNowPressed = hitTest == DownHit;

        if (isNowPressed != IsPressed)
        {
            IsPressed = isNowPressed;
            HDC hdc;
            switch (DownHit)
            {
            case HTMINBUTTON:
                hdc = GetWindowDC(hwnd);
                PaintMinBox(hdc, IsPressed);
                ReleaseDC(hwnd,hdc);
                return true;
            case HTMAXBUTTON:
                hdc = GetWindowDC(hwnd);
                PaintMaxBox(hdc, IsPressed);
                ReleaseDC(hwnd,hdc);
                //fall...
            case HTSYSMENU:
                return true;
            }
        }
    }
    return false;
}

bool TCWindow::DoSysCommand(int cmdType)
{
    cmdType &= 0xFFF0;
    if ((cmdType == SC_KEYMENU || cmdType == SC_MOUSEMENU) && !IsIconic())
    {
        DoSysMenu();
        return true;
    }
    return false;
}

int TCWindow::DoNCHitTest(LPPOINT screenPt)
{
    if (IsIconic())
        return HTNOWHERE;

    // Check style bits to see what to paint
    //
    int style = GetWindowLong(hwnd,GWL_STYLE);
    bool hasSysMenu =  !docked && !!(style & WS_SYSMENU);
    bool hasMaximize =  !!(style & WS_MAXIMIZEBOX);
    bool hasMinimize = !docked && !!(style & WS_MINIMIZEBOX);

    RECT rc;
    GetWindowRect(hwnd,&rc);
    // Convert to window coordinates
    //
    POINT winPt;
    winPt.x = screenPt->x - rc.left;
    winPt.y = screenPt->y - rc.top;

    if (hasSysMenu)
    {
        GetSysBoxRect(&rc);
        if(PtInRect(&rc,winPt))
            return HTSYSMENU;
    }
    if (hasMinimize)
    {
        GetMinBoxRect(&rc);
        if(PtInRect(&rc,winPt))
            return HTMINBUTTON;
    }
    if (hasMaximize)
    {
        GetMaxBoxRect(&rc);
        if(PtInRect(&rc,winPt))
            return HTMAXBUTTON;
    }

    // CaptionRect includes buttons so make sure it's last checked
    GetCaptionRect(&rc);
    PtInRect(&rc,winPt);
    if (PtInRect(&rc,winPt))
        if(!docked)
            return HTCAPTION;
        else
        {
            SetFocus();
            //TODO: set a sentinel value saying caption was hit for dock tracking
        }

    return HTNOWHERE;
}
void TCWindow::DoNCPaint()
{
    if(IsIconic() || !hwnd || !set.draw_old_style)
        return;
    HWND focus = GetFocus();
    PaintCaption(GetActiveWindow() == hwnd || focus == hwnd || IsChild(hwnd,focus));
    return;
}
int TCWindow::DoNCCalcSize(LPNCCALCSIZE_PARAMS calcSize)
{
    RECT rc;
    GetCaptionRect(&rc);
    calcSize->rgrc[0].top += rc.bottom - rc.top;		//room for caption

    if(!docked)
        InflateRect(&calcSize->rgrc[0],-FRAME_X,-FRAME_Y);	//room for size borders

    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    if(style & WS_HSCROLL)
    {
        calcSize->rgrc[0].bottom -= GetSystemMetrics(SM_CYHSCROLL);
        calcSize->rgrc[0].bottom = max(5,calcSize->rgrc[0].bottom);
    }
    if(style & WS_VSCROLL)
    {
        calcSize->rgrc[0].right -= GetSystemMetrics(SM_CXVSCROLL);
        calcSize->rgrc[0].right = max(5,calcSize->rgrc[0].right);
    }
    return 1;
}

bool TCWindow::DoNCActivate(bool active)
{
    if(active && hwnd && !IsIconic())
    {
        PaintCaption(active);
    }
    return false;
}

void TCWindow::PaintButton(HDC hdc, LPRECT rc, bool pressed)
{
    FrameRect(hdc,rc,GetSysColorBrush(COLOR_WINDOWFRAME));

    RECT r;
    SetRect(&r,rc->left+1,rc->top+1,rc->right-1,rc->bottom-1);

    SetBkColor(hdc,GetSysColor(COLOR_3DFACE));
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &r, 0, 0, 0);

    if ((r.right-r.left) > 4 && (r.bottom-r.top) > 4)
    {
        RECT s;
        if (pressed)
        {

            SetRect(&s,r.left,r.top,r.right,r.top+1);
            SetBkColor(hdc,GetSysColor(COLOR_3DSHADOW));
            ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &s, 0, 0, 0);

            SetRect(&s,r.left,r.top+1,r.left+1,r.bottom);
            ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &s, 0, 0, 0);

        }
        else
        {

            SetRect(&s,r.left,r.top,r.right,r.top+1);
            SetBkColor(hdc,GetSysColor(COLOR_3DHIGHLIGHT));
            ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &s, 0, 0, 0);

            SetRect(&s,r.left,r.top+1,r.left+1,r.bottom-1);
            ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &s, 0, 0, 0);

            SetRect(&s,r.right-1,r.top+1,r.right,r.bottom);
            SetBkColor(hdc,GetSysColor(COLOR_3DSHADOW));
            ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &s, 0, 0, 0);

            SetRect(&s,r.left+1,r.bottom-1,r.right-1,r.bottom);
            ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &s, 0, 0, 0);
        }
    }
}
void TCWindow::PaintSysBox(HDC hdc)
{
    RECT boxRect;
    GetSysBoxRect(&boxRect);

    // Dont paint over the left & top borders
    //
    boxRect.left++;
    boxRect.top++;

    // Fill the box with 3d face
    //
    SetBkColor(hdc,GetSysColor(COLOR_3DFACE));
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &boxRect, 0, 0, 0);

    int bh = boxRect.bottom - boxRect.top;
    int bw = boxRect.right - boxRect.left;

    // Draw the ventilator (sysmenu) box, with shadow
    //
    POINT begPt;
    begPt.x = boxRect.left + 2;
    begPt.y = boxRect.top + (bh-3)/2;


    RECT ventRect;
    SetRect(&ventRect,begPt.x,begPt.y,begPt.x+bw-5,begPt.y+3);

//Caption("vr 32 %d %d %d %d",ventRect.left,ventRect.top,ventRect.right,ventRect.bottom);
    RECT rc;

    // Draw shadow down and right 1
    //
    CopyRect(&rc,&ventRect);
    OffsetRect(&rc,1,1);
    SetBkColor(hdc,GetSysColor(COLOR_3DSHADOW));
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, 0, 0, 0);

    // Draw ventilator rectangle
    //
    FrameRect(hdc,&ventRect,GetSysColorBrush(COLOR_BTNTEXT));

    // Draw white interior of ventilator
    //
    rc.left = ventRect.left+1;
    rc.top = ventRect.top+1;
    rc.right = ventRect.right-1;
    rc.bottom = ventRect.top+2;
    SetBkColor(hdc,GetSysColor(COLOR_3DHIGHLIGHT));
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, 0, 0, 0);

    rc.left = boxRect.right;
    rc.top = boxRect.top;
    rc.right = boxRect.right+1;
    rc.bottom = boxRect.bottom;
    SetBkColor(hdc,GetSysColor(COLOR_BTNTEXT));
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, 0, 0, 0);
}
void TCWindow::PaintMinBox(HDC hdc, bool pressed)
{
    RECT boxRect;
    GetMinBoxRect(&boxRect);

    PaintButton(hdc, &boxRect, pressed);

    if (pressed)
    {
        boxRect.top++;
        boxRect.left++;
        boxRect.right++;
        boxRect.bottom++;
    }

    int bh = boxRect.bottom - boxRect.top;
    int bw = boxRect.right - boxRect.left;

    POINT begPt, endPt;

    begPt.x = boxRect.left + (bw+1)/4;
    begPt.y = boxRect.top + (bh+2)/3;

    endPt.x = begPt.x + (bw+1)/2 - 1;
    endPt.y = begPt.y;

    while (begPt.x < endPt.x)
    {
        MoveToEx(hdc,begPt.x,begPt.y,0);
        LineTo(hdc,endPt.x,endPt.y);
        begPt.x++;
        begPt.y++;
        endPt.x--;
        endPt.y++;
    }
}
void TCWindow::PaintMaxBox(HDC hdc, bool pressed)
{
    RECT boxRect;
    GetMaxBoxRect(&boxRect);

    PaintButton(hdc, &boxRect, pressed);

    if (pressed)
    {
        boxRect.top++;
        boxRect.left++;
        boxRect.right++;
        boxRect.bottom++;
    }

    // Down triangle
    //
    int bh = boxRect.bottom - boxRect.top;
    int bw = boxRect.right - boxRect.left;
    bool zoomed = IsZoomed();

    POINT begPt, endPt;

    if (zoomed)
    {

        begPt.x = boxRect.left + (bw+1)/4;
        begPt.y = boxRect.bottom - bh*3/8;

        endPt.x = begPt.x + (bw+1)/2 - 1;
        endPt.y = begPt.y;

        while (begPt.x < endPt.x)
        {
            MoveToEx(hdc,begPt.x,begPt.y,0);
            LineTo(hdc,endPt.x,endPt.y);
            begPt.x++;
            begPt.y++;
            endPt.x--;
            endPt.y++;
        }
    }

    // Up triangle
    //

    begPt.x = boxRect.left + (bw+1)/4;
    begPt.y = boxRect.top + (zoomed ? bh*3/8 : bh*2/3);

    endPt.x = begPt.x + (bw+1)/2 - 1;
    endPt.y = begPt.y;

    while (begPt.x < endPt.x)
    {
        MoveToEx(hdc,begPt.x,begPt.y,0);
        LineTo(hdc,endPt.x,endPt.y);
        begPt.x++;
        begPt.y--;
        endPt.x--;
        endPt.y--;
    }
}
void TCWindow::PaintCaption(bool active)
{
    if(set.draw_old_style)
    {
        if(!hwnd || IsIconic())
            return;

        HDC hdc = GetWindowDC(hwnd);
        RECT captRect;
        GetCaptionRect(&captRect);

        SetTextColor(hdc, GetSysColor(active ? COLOR_CAPTIONTEXT : COLOR_INACTIVECAPTIONTEXT));

        SetBkColor(hdc, GetSysColor(active ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION));
        ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &captRect, 0, 0, 0);

        SelectObject(hdc,g_hCaptionFont);

        SetBkMode(hdc,TRANSPARENT);

        // Calc x coord for text, so that text is centered between caption buttons
        //
        int style = GetWindowLong(hwnd,GWL_STYLE);
        int xOrg = captRect.left;

        if (!docked && (style & WS_SYSMENU))
        {
            RECT rc;
            GetSysBoxRect(&rc);
            xOrg += rc.right - rc.left;		//add sysbox width
        }

        ::ExtTextOut(hdc, xOrg, captRect.top-BORDER_Y, ETO_CLIPPED, &captRect,
                     Window::caption, (int) strlen(Window::caption), 0);

        // Paint widgets: sysmenu or close button, minimize button, maximize button
        // They currently all use a black pen
        //
        HGDIOBJ hOld = SelectObject(hdc,GetStockObject(BLACK_PEN));


        //dont draw sysmenu or minimize when docked.
        if(!docked)
        {
            // Paint system menu or close button
            if (style & WS_SYSMENU)
                PaintSysBox(hdc);

            // Paint minimize button
            if (style & WS_MINIMIZEBOX)
                PaintMinBox(hdc, false);
        }
        // Paint maximize button.  show when docked, but map functionality to "undock"
        if (style & WS_MAXIMIZEBOX)
            PaintMaxBox(hdc, false);

        // Draw window-frame color line under caption
        //
        captRect.top = captRect.bottom-1;
        FrameRect(hdc,&captRect, GetSysColorBrush(COLOR_WINDOWFRAME));
        SelectObject(hdc,hOld);
        ReleaseDC(hwnd,hdc);
    }
}
void TCWindow::GetCaptionRect(LPRECT rc)
{
    GetWindowRect(hwnd,rc);

    rc->right -= rc->left;
    if(!docked)
    {
        rc->right -= FRAME_X;
        rc->left =   FRAME_X;
        rc->top =    FRAME_Y;
    }
    else
    {
        rc->left =   0;
        rc->top =    0;
    }
    rc->bottom = rc->top + CaptionHeight;
}
void TCWindow::GetSysBoxRect(LPRECT rc)
{
    GetCaptionRect(rc);

    rc->right = rc->left + CaptionHeight;
    rc->left--;
    rc->top--;
}
void TCWindow::GetMinBoxRect(LPRECT rc)
{
    GetMaxBoxRect(rc);

    if (!docked && GetWindowLong(hwnd,GWL_STYLE) & WS_MAXIMIZEBOX)
    {
        rc->left -= CaptionHeight - 1;
        rc->right -= CaptionHeight - 1;
    }
}
void TCWindow::GetMaxBoxRect(LPRECT rc)
{
    GetCaptionRect(rc);

    rc->left = rc->right - CaptionHeight - 1;
    rc->top--;
//	rc->right++;
}
void TCWindow::DoSysMenu()
{
    if (set.draw_old_style)
    {
        HMENU hSysMenu = GetSystemMenu(hwnd,FALSE);
        if (!hSysMenu)
            return;

        int style = GetWindowLong(hwnd,GWL_STYLE);
        EnableMenuItem(hSysMenu, SC_RESTORE, (IsIconic() || IsZoomed()) ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(hSysMenu, SC_MOVE, (1/*style & WS_CAPTION*/) ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(hSysMenu, SC_SIZE, (style & WS_THICKFRAME) ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(hSysMenu, SC_MINIMIZE, ((style&WS_MINIMIZEBOX) && !IsIconic()) ? MF_ENABLED : MF_GRAYED);
        EnableMenuItem(hSysMenu, SC_MAXIMIZE, ((style&WS_MAXIMIZEBOX) && !IsZoomed()) ? MF_ENABLED : MF_GRAYED);

        RECT rc;
        GetSysBoxRect(&rc);

        POINT pt;
        RECT r;
        pt.x = rc.left;
        pt.y = rc.top;
        ClientToScreen(hwnd,&pt);     // Cvt pt to screen coord
        r.left = pt.x;
        r.top = pt.y;

        pt.x = rc.bottom;
        pt.y = rc.right;
        ClientToScreen(hwnd,&pt);
        r.bottom = pt.x;
        r.right = pt.y;

        TrackPopupMenu(hSysMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, r.left-FRAME_X, r.top-FRAME_Y, 0, hwnd, &r);
    }
}

//dock window
DockWindow::DockWindow(HWND parent, const char*caption)
    : TCWindow(parent,caption)
{
    memset(children,0,sizeof(children));
    center.x = center.y = 0;
}
LRESULT DockWindow::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_CREATE:
    {
        GetClientRect(hwnd, &wmsizerc);
        center.x = wmsizerc.right / 2;
        center.y = wmsizerc.bottom / 2;
    }
    break;
    case WM_SIZE:
    {
        int nWidth = (short)LOWORD(lParam);
        int nHeight = (short)HIWORD(lParam);
        center.x = (center.x * nWidth) / (wmsizerc.right?wmsizerc.right:1);
        center.y = (center.y * nHeight) / (wmsizerc.bottom?wmsizerc.bottom:1);
        wmsizerc.right = nWidth;
        wmsizerc.bottom = nHeight;
        PositionWindows();
    }
    break;
    }
    return TCWindow::WndProc(msg,wParam,lParam);
}
bool DockWindow::CanClose()
{
    return (0 != KillAllWindows);
}
void DockWindow::PositionWindows()
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    if(children[0].inuse && children[0].win)
        ::SetWindowPos(children[0].win->hwnd, 0, 0,0, rc.right/2 -1, rc.bottom/2 -1, SWP_FRAMECHANGED|SWP_NOZORDER);
    if(children[1].inuse && children[1].win)
        ::SetWindowPos(children[1].win->hwnd, 0, 0,rc.bottom/2 +1, rc.right/2 -1, rc.bottom/2 -1, SWP_FRAMECHANGED|SWP_NOZORDER);
    if(children[2].inuse && children[2].win)
        ::SetWindowPos(children[2].win->hwnd, 0, rc.right/2 +1,rc.bottom/2 +1, rc.right/2 -1, rc.bottom/2 -1, SWP_FRAMECHANGED|SWP_NOZORDER);
    if(children[3].inuse && children[3].win)
        ::SetWindowPos(children[3].win->hwnd, 0, rc.right/2 +1,0, rc.right/2 -1, rc.bottom/2 -1, SWP_FRAMECHANGED|SWP_NOZORDER);
}
bool DockWindow::Dock(TCWindow *win)
{
    int open = -1;
    for(int i=0; i<4; i++)
    {
        if(children[i].inuse && children[i].win == win)
            return false;
        if(open == -1 && !children[i].inuse)
            open = i;
    }
    if(open >= 0 && open < 4)
    {
        children[open].inuse = true;
        children[open].pos = 0;
        children[open].win = win;
        SetParent(win->hwnd, this->hwnd);
        int style = GetWindowLong(win->hwnd,GWL_STYLE);
        children[open].oldstyle = style;
        style &= ~(WS_SYSMENU|WS_MINIMIZEBOX|WS_THICKFRAME|WS_BORDER);
        SetWindowLong(win->hwnd, GWL_STYLE, style);
        win->docked = this;
        return true;
    }
    return false;
}
bool DockWindow::UnDock(TCWindow *win)
{
    for(int i =0; i<4; i++)
        if(children[i].inuse && children[i].win == win)
        {
            children[i].inuse = false;
            children[i].win = 0;
            children[i].pos = 0;
            SetParent(win->hwnd, client->hwnd);
            SetWindowLong(win->hwnd, GWL_STYLE, children[i].oldstyle);
            win->docked = 0;
            ::SetWindowPos(win->hwnd, 0, 0,0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);
            return true;
        }
    return false;
}

//PaletteWindow
PaletteWindow::PaletteWindow(HWND parent, const char*caption)
    : TCWindow(parent, caption)
{
    attr.width = 275;
    attr.height = 275;
}
LRESULT PaletteWindow::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_SIZE:
        Redraw();
        break;
    case WM_PAINT:
        Redraw();
        return 0;
    case WM_MOUSEMOVE:
        if(status)
        {
            RECT rc;
            GetClientRect(hwnd, &rc);
            MAKEPOINT(pt, lParam);
            int x = pt.x*16/rc.right;
            int y = pt.y*16/rc.bottom;
            int pal = y*16+x;

            char buf[128];
            sprintf(buf,"Palette Index: %d (#%.2X%.2X%.2X)", pal,
                    set.pal[pal*PALSTEP + 0],set.pal[pal*PALSTEP + 1],set.pal[pal*PALSTEP + 2]);
            status->SetText(buf, true);
        }
        break;
    }
    return TCWindow::WndProc(msg,wParam,lParam);
}
void PaletteWindow::Redraw()
{
    HDC hdc = GetDC(hwnd);
    RECT rc;
    GetClientRect(hwnd, &rc);
    int pal = 0;
    for(int j=0; j<16; j++)
        for(int i=0; i<16; i++)
        {
            RECT r = {i*rc.right/16,j*rc.bottom/16,(i+1)*rc.right/16,(j+1)*rc.bottom/16};
            COLORREF color = RGB(set.pal[pal + 0],set.pal[pal + 1],set.pal[pal + 2]);
            HBRUSH br = CreateSolidBrush(color);
            FillRect(hdc,&r,br);
            DeleteObject(br);
            pal += PALSTEP;
        }
    ReleaseDC(hwnd, hdc);
    ValidateRect(hwnd,0);
}

// ImageView
ImageView::ImageView(HWND parent, const char*caption, unsigned char*img,int width, int height)
    : TCWindow(parent, caption)
{
    attr.width = width;
    attr.height = height;
    this->width = width;
    this->height = height;
    bmp = CreateBitmap(width,height,1,32,img);
}
ImageView::~ImageView()
{
    if(bmp) DeleteObject(bmp);
}
LRESULT ImageView::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_SIZE:
        Redraw();
        break;
    case WM_PAINT:
        Redraw();
        return 0;
    case WM_NCDESTROY:
        delete this;
        return 0;
    case WM_LBUTTONDBLCLK:
        ResetSize();
    }
    return TCWindow::WndProc(msg,wParam,lParam);
}
void ImageView::Redraw()
{
    if(!bmp)
        return;
    RECT rc;
    GetClientRect(hwnd, &rc);
    HDC hdc = GetDC(hwnd);
    HDC cdc = CreateCompatibleDC(hdc);
    HGDIOBJ old = SelectObject(cdc, bmp);
    StretchBlt(hdc,0,0,rc.right,rc.bottom,cdc,0,0,width,height,SRCCOPY);
    SelectObject(cdc,old);
    DeleteDC(cdc);
    ReleaseDC(hwnd,hdc);
    ValidateRect(hwnd,0);
}
void ImageView::ResetSize()
{
    // calc difference between window and client area
    RECT src, crc;
    GetWindowRect(hwnd, &src);
    GetClientRect(hwnd, &crc);
    int dx = (src.right-src.left) - crc.right;
    int dy = (src.bottom-src.top) - crc.bottom;
    // resize window
    SetWindowPos(0,0,0,width+dx,height+dy, SWP_NOZORDER|SWP_NOMOVE);
}
