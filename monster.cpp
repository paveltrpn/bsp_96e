
#include "global.h"

#define M_STEPS 12
#define FRAME_UP_MODE 99
#define FRAME_DOWN_MODE 100
#define MLEFT 15
#define MTOP 30


int skin = 0;
//////////////////////////////////////////////
TMonsterWindow::TMonsterWindow(HWND parent, char *title)
    : TCWindow(parent,title)
{
    attr.dwStyle |= CS_VREDRAW | CS_HREDRAW;

    hmemdc = 0;
    hmembitmap = NULL;

    lastSz.cx = lastSz.cy = -1;

    attr.width = 250;
    attr.height = 320;

    MonsterRect.left = MLEFT;
    MonsterRect.right = 240;
    MonsterRect.top = MTOP;
    MonsterRect.bottom = 300;

    zoffset = 0;
    SetCaption(title);

    mfSize = 10;
    mode = 0;

    MonsterFont = GetFont("arial",-mfSize);
    MonsterList = NULL;
    MonsterUp = NULL;
    MonsterDown = NULL;
    MonsterLeft = NULL;
    MonsterRight = NULL;
    bReset = NULL;

    memset(&m_pBmInfoHeader,0,sizeof(BITMAPINFOHEADER));
    m_pBmInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
    m_pBmInfoHeader.biWidth = 1;
    m_pBmInfoHeader.biHeight = 1;
    m_pBmInfoHeader.biPlanes = 1;
    m_pBmInfoHeader.biCompression = BI_RGB;

    if (set.sinBsp)
    {
        m_pBmInfoHeader.biBitCount =24;
        m_pBmInfoHeader.biClrUsed = 0;
        m_pBmInfoHeader.biSizeImage = 0;

        //this uses palette instead of 24bit. is this correct?
        m_pBmInfo = (BITMAPINFO*)new unsigned char[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];
        memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
        memset(m_pBmInfo->bmiColors,0,256*sizeof(RGBQUAD));
    }
    else
    {
        m_pBmInfoHeader.biBitCount = 8;
        m_pBmInfoHeader.biClrUsed = 256;
        m_pBmInfoHeader.biSizeImage = 1;

        m_pBmInfo = (BITMAPINFO*)new unsigned char[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];
        memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
        memset(m_pBmInfo->bmiColors,0,256*sizeof(RGBQUAD));
    }
}

LRESULT TMonsterWindow::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_CREATE:
        SetupWindow();
        break;
    case WM_TIMER:
        EvTimer(wParam);
        return 0;
    case WM_PAINT:
        RedrawContents();
        break;
    case WM_COMMAND:
        //buttons etc
        switch(LOWORD(wParam))
        {
        case IDM_MONSTERLEFT:
            LeftButton();
            return 0;
        case IDM_RESETANGLES:
            ResetAngles();
            return 0;
        case IDM_MONSTERRIGHT:
            RightButton();
            return 0;
        case IDM_MONSTERUP:
            ClickMonsterUp();
            return 0;
        case IDM_MONSTERDOWN:
            ClickMonsterDown();
            return 0;
        case IDM_MONSTERLIST:
            if(HIWORD(wParam) == CBN_SELCHANGE)
            {
                EvCBNSelChange();
                return 0;
            }
        }
        break;
    case WM_SIZE:
    {
        SIZE sz;
        sz.cx = (short)LOWORD(lParam);
        sz.cy = (short)HIWORD(lParam);
        EvSize(wParam,&sz);
    }
    break;
    case WM_RBUTTONDOWN:
    {
        MAKEPOINT(pt, lParam);
        EvRButtonDown(wParam,&pt);
    }
    break;
    case WM_RBUTTONUP:
    {
        MAKEPOINT(pt, lParam);
        EvRButtonUp(wParam,&pt);
    }
    break;
    case WM_LBUTTONDOWN:
    {
        MAKEPOINT(pt, lParam);
        EvLButtonDown(wParam,&pt);
    }
    break;
    case WM_LBUTTONUP:
    {
        MAKEPOINT(pt, lParam);
        EvLButtonUp(wParam,&pt);
    }
    break;
    case WM_MOUSEMOVE:
    {
        MAKEPOINT(pt, lParam);
        EvMouseMove(wParam,&pt);
    }
    break;
    }
    return TCWindow::WndProc(msg,wParam,lParam);
}

void TMonsterWindow::RemoveMemDC()
{
    if (hmemdc)
        DeleteDC(hmemdc);
    if (hmembitmap)
        DeleteObject(hmembitmap);
    hmemdc = 0;
    hmembitmap = 0;
}

void TMonsterWindow::ConstructMemDC()
{
    HDC hdc = GetDC(hwnd);
    hmemdc = CreateCompatibleDC(hdc);

    int tempW = max(1, MonsterRect.right-MonsterRect.left);
    int tempH = max(1, MonsterRect.bottom-MonsterRect.top);
    hmembitmap = CreateCompatibleBitmap(hdc, tempW, tempH);

    SelectObject(hmemdc, hmembitmap);

    RECT rc;
    GetClientRect(hwnd, &rc);
    lastSz.cx = rc.right;
    lastSz.cy = rc.bottom;
}

void TMonsterWindow::ResetAngles()
{
    yawangle = rollangle = pitchangle = 0.0;
    RedrawContents();
}
void TMonsterWindow::ClickMonsterDown()
{
    zoffset -= 5.0;
    RedrawContents();
}
void TMonsterWindow::ClickMonsterUp()
{
    zoffset += 5.0;
    RedrawContents();
}
void TMonsterWindow::EvLButtonDown(UINT modKeys, LPPOINT point)
{
    if (mode)  // already processing...
        return;

    SetCapture(hwnd);

    startPt.x = lastPt.x = point->x;
    startPt.y = lastPt.y = point->y;

    mode = DRAG_ROTATE;
    if (modKeys & MK_SHIFT)
        mode = DRAG_DISTANCE;
}

void TMonsterWindow::EvLButtonUp(UINT modKeys, LPPOINT point)
{
    if (DoLButtonUp())
        return;

    if (!mode) // not processing...
        return;

    KillTimer(hwnd,3);

    ReleaseCapture();
    mode = 0; // turn it off...
}

void TMonsterWindow::EvRButtonUp(UINT /*modKeys*/, LPPOINT /*point*/)
{
    if (!mode) // not processing...
        return;

    KillTimer(hwnd,3);

    ReleaseCapture();
    mode = 0; // turn it off...
}

void TMonsterWindow::EvMouseMove(UINT modKeys, LPPOINT point)
{
    if (!mode) // just ignore...
        return;
    if (point->x == lastPt.x && point->y == lastPt.y)
        return;  // no move...

    int dH1 =   DRAG_DELTA/3;
    int dH2 = 2*DRAG_DELTA/3;

    int dx = point->x - lastPt.x;
    int dy = point->y - lastPt.y;

    if ((abs(dx) < DRAG_DELTA) && (abs(dy) < DRAG_DELTA))
        return;

    lastPt.x = point->x;
    lastPt.y = point->y;

    switch (mode)
    {
    case DRAG_ROTATE:
        if (abs(dx) >= dH2 && abs(dy) <= dH1)
        {
            if (dx > 0)
            {
                LeftButton();
            }
            else
            {
                RightButton();
            }
        }
        else if (abs(dy) >= dH2 && abs(dx) <= dH1)
        {
            if (dy > 0)
            {
                PUp();
            }
            else
            {
                PDown();
            }
        }
        else if (abs(dy) >= dH1 && abs(dx) >= dH1)
        {
            if (dy > 0)
            {
                RUp();
            }
            else
            {
                RDown();
            }
        }
        break;
    case DRAG_DISTANCE:
        if (dy > 0)
        {
            InButton();
        }
        else if (dy < 0)
        {
            OutButton();
        }
        break;
    }
    // flush queue of MM messages!
    /*	MSG msg;
    	int removedOne = 0;
    	while (PeekMessage(&msg, NULL, WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE)) {
    		removedOne = 1;
    		// do nothing...
    	}
    	if (removedOne) { // stick it back in!
    		PostMessage(hwnd,msg.message,msg.wParam,msg.lParam);
    	};*/
}

void TMonsterWindow::EvRButtonDown(UINT, LPPOINT point)
{
    if (mode)  // already processing...
        return;

    SetCapture(hwnd);

    startPt.x = lastPt.x = point->x;
    startPt.y = lastPt.y = point->y;
    if (point->y >= (MonsterRect.top + (MonsterRect.bottom-MonsterRect.top)/2))
    {
        mode = FRAME_UP_MODE;
    }
    else
    {
        mode = FRAME_DOWN_MODE;
    }
    SetTimer(hwnd,3,100,0); // send WM_TIMER messages...
}

void TMonsterWindow::EvTimer(int timerId)
{
    if (!mode)
        return;
    if (timerId != 3)
        return;

    switch (mode)
    {
    case FRAME_UP_MODE:
        curframe++;
        RedrawContents();
        break;
    case FRAME_DOWN_MODE:
        curframe--;
        RedrawContents();
        break;
    }
}

void TMonsterWindow::SetupWindow()
{

    int x = 0;
    int y = 0;
    int w = 150;

    MonsterList = new WCombo( hwnd, IDM_MONSTERLIST, x, y, w, 450);
    MonsterList->attr.dwStyle |= CBS_DROPDOWNLIST;
    MonsterList->Create();
    MonsterList->SendMessage(WM_SETFONT,(WPARAM)MonsterFont,0);
    ::SetFocus(MonsterList->hwnd);
    y+=24;
    MonsterUp = new WButton(hwnd, IDM_MONSTERUP, const_cast<char *> ("+"), x, y, 14, 14);
    MonsterUp->Create();
    MonsterUp->SendMessage(WM_SETFONT,(WPARAM)MonsterFont,0);

    MonsterDown = new WButton( hwnd, IDM_MONSTERDOWN, const_cast<char *> ("-"), x, y+14, 14, 14);
    MonsterDown->Create();
    MonsterDown->SendMessage(WM_SETFONT,(WPARAM)MonsterFont,0);

    bReset = new WButton(hwnd,IDM_RESETANGLES,const_cast<char *> ("Reset"), x+w, 0, 40, 14);
    bReset->Create();
    bReset->SendMessage(WM_SETFONT,(WPARAM)MonsterFont,0);


    RECT r;
    GetClientRect(hwnd, &r);

    y = max(0,r.bottom - 15);
    x = max(0,(r.right - 80) / 2);


    MonsterLeft = new WButton(hwnd,IDM_MONSTERLEFT, const_cast<char *> ("Left"), x, y, 40, 14);
    MonsterLeft->Create();
    MonsterLeft->SendMessage(WM_SETFONT,(WPARAM)MonsterFont,0);
    MonsterRight = new WButton(hwnd,IDM_MONSTERRIGHT, const_cast<char *> ("Right"), x+40, y, 40, 14);
    MonsterRight->Create();
    MonsterRight->SendMessage(WM_SETFONT,(WPARAM)MonsterFont,0);

    SetIcon(IDI_MONSTER);

    if (!entity_classes_i)
        return;

    EntityClass *fc = 0;

    MonsterList->ClearList();
    for (int i = 0; i < entity_classes_i->numElements; i++)
    {

        EntityClass *ec = entity_classes_i->objects[i];
        if (!ec->mdl && !ec->mdlQ2)
            continue;

        if (!fc)
            fc = ec;

        MonsterList->AddString(ec->name);
    }
    MonsterList->SetSelIndex(0); // xxx use current ent name...

    distance = 50.0f;
    curframe = 0;
    if (fc)
    {
        curframe = 0;

        float h = fc->maxs[2]-fc->mins[2]; // model height...
        distance = h * 2.0f;
    }
    rw = 640;
    rh = 480;

    yawangle = 0.0;
    rollangle = 0.0;
    pitchangle = 0.0;

    s = 0.0;
    int size = rw*rh;

    mzbuffer = new float[size+16];

    if (set.sinBsp)
        mimagebuffer = new unsigned char[3L*(size+rw+1)];
    else
        mimagebuffer = new unsigned char[size+rw+1];

    ConstructMemDC();
}

void TMonsterWindow::EvCBNSelChange()
{
    int idx = MonsterList->GetSelIndex();
    if (idx < 0)
        return;

    EntityClass *ec;
    char text[64];
    MonsterList->GetText(text,64);

    ec = entity_classes_i->classForName(text);
    if (!ec || (!ec->mdl && !ec->mdlQ2))
        return;

    // get the class and draw it...
    // set min/max frames...
    RedrawContents();
}

void TMonsterWindow::SetAngles()
{
    char buf[80];

    float r, p, y;

    sprintf(buf,"%i %i %i",(int)(rollangle*180.0/M_PI),
            (int)(pitchangle*180.0/M_PI),(int)(yawangle*180.0/M_PI));

    if (IDOK == (intptr_t)InputDialog(hwnd, const_cast<char *> ("SBV:  Set Angles"), const_cast<char *> ("Enter rpy values..."), buf, sizeof(buf)).Execute())
    {
        if (3 == sscanf(buf,"%f %f %f",&r,&p,&y))
        {
            rollangle =  DEG2RAD(r);
            pitchangle = DEG2RAD(p);
            yawangle =   DEG2RAD(y);
        }
    }
    RedrawContents();
}
void TMonsterWindow::LeftButton()
{
    yawangle -= M_PI/M_STEPS;
    while (yawangle < 0.0f)
        yawangle += 2*M_PI;

    RedrawContents();
}
void TMonsterWindow::RightButton()
{
    yawangle += M_PI/M_STEPS;
    while (yawangle > 2.0*M_PI)
        yawangle -= 2.0*M_PI;

    RedrawContents();
}
void TMonsterWindow::RedrawContents()
{
    if (!hmemdc || !hmembitmap)
        return;
    if (!texWindow/* || !set.m_hPal*/)
        return;

    int idx = MonsterList->GetSelIndex();
    if (idx < 0)
        return;

    EntityClass *ec;
    char text[64];
    MonsterList->GetText(text,64);

    ec = entity_classes_i->classForName(text);
    if (!ec || (!ec->mdl && !ec->mdlQ2))
        return;

    vec3_t ctr;

    if (ec->mdl)
    {
        ctr[0] = ec->mdl->origin[0];
        ctr[1] = ec->mdl->origin[1];
        ctr[2] = ec->mdl->origin[2];
    }
    else
    {
        ctr[0] = 0.0f;
        ctr[1] = 0.0f;
        ctr[2] = 0.0f;
    }

    // bounds...
    if (curframe < 0)
        curframe = ec->maxframe -1;		// wrap to end

    if (curframe >= ec->maxframe)
        curframe = 0;					// wrap to start

    VectorCopy (ctr, r_origin);

    vec3_t zero;
    VectorCopy(vec3_origin,zero);
    float Q[4][4];
    float Temp[4][4];

    findQ(Q,Temp,zero,
          0.0,        // PI 1.5 1.5 PI
          0.0,
          M_PI
         );

    // vpn
    r_matrix[0][0] = Q[0][0];
    r_matrix[0][1] = Q[0][1];
    r_matrix[0][2] = Q[0][2];

    r_matrix[1][0] = Q[1][0];
    r_matrix[1][1] = Q[1][1];
    r_matrix[1][2] = Q[1][2];

    r_matrix[2][0] = Q[2][0];
    r_matrix[2][1] = Q[2][1];
    r_matrix[2][2] = Q[2][2];

    r_width = rw; // EditSize.cx;
    r_height = rh; // EditSize.cy;

    r_picbuffer = mimagebuffer;
    r_zbuffer = mzbuffer;

    r_drawflat = true;//false;
    r_drawwire = false;

    REN_BeginCamera ();
    REN_ClearBuffers ();

    //
    // render the model setbrushes
    //
    if (ec->mdl)
    {
        if (ec->mdl->numskins > 1)
            skin++;
        if (skin >= ec->mdl->numskins)
            skin = 0;

        REN_RenderModel(ec->mdl,
                        2, (float) RAD2DEG(rollangle), (float) RAD2DEG(pitchangle), (float) RAD2DEG(yawangle),
                        curframe,
                        ctr[0], distance + s, ctr[2] + zoffset, skin);
    }
    else
    {
        if (ec->mdlQ2->num_skins > 1)
            skin++;
        if (skin >= ec->mdlQ2->num_skins)
            skin = 0;

        REN_RenderModelQ2(ec->mdlQ2,ec->skinTextureData,
                          2, (float) RAD2DEG(rollangle), (float) RAD2DEG(pitchangle), (float) RAD2DEG(yawangle),
                          curframe,
                          ctr[0], distance + s, ctr[2] + zoffset, skin);
    }


    //
    // display the output
    //

    m_pBmInfoHeader.biWidth = (int)rw;
    m_pBmInfoHeader.biHeight = (int)-rh;
    memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
    if (set.sinBsp)
    {
        m_pBmInfoHeader.biSizeImage = 0;
        memset(m_pBmInfo->bmiColors,0,256*sizeof(RGBQUAD));
    }
    else
    {
        m_pBmInfoHeader.biSizeImage = rw*rh;
        memcpy(m_pBmInfo->bmiColors,set.cTable,256*sizeof(RGBQUAD));
    }

    SetStretchBltMode(hmemdc,COLORONCOLOR);

    if (mimagebuffer && m_pBmInfo)
    {
        StretchDIBits(hmemdc, 0,0,
                      MonsterRect.right-MonsterRect.left,MonsterRect.bottom-MonsterRect.top,
                      0,0,rw,rh, mimagebuffer, m_pBmInfo, DIB_RGB_COLORS, SRCCOPY);
    }

    SelectObject(hmemdc,MonsterFont);
    SetTextColor(hmemdc,set.color_foreground);
    SetBkColor(hmemdc,set.color_background);
    SetBkMode(hmemdc,TRANSPARENT);

    char outstr[128];
    const int leftS = 1;	// left margin
    const int lineA = 2;	// line1 y
    const int lineB = 14;	// line2 y
    const int lineC = 26;	// line3 y
    int len = sprintf(outstr,"Frame [%i]",curframe);
    TextOut(hmemdc,leftS,lineA,outstr,len);

    if (ec->mdl)
    {
        len = sprintf(outstr,"Origin %i %i %i",
                      (int)ec->mdl->origin[0],
                      (int)ec->mdl->origin[1],
                      (int)ec->mdl->origin[2]);
        TextOut(hmemdc,leftS,lineB,outstr,len);
        len = sprintf(outstr,"Skin: [%i/%i]",skin+1,ec->mdl->numskins);
        TextOut(hmemdc,leftS,lineC,outstr,len);
    }
    else
    {
        len = sprintf(outstr,"No Origin for Q2");
        TextOut(hmemdc,leftS,lineB,outstr,len);
        len = sprintf(outstr,"Skin: [%i/%i]",skin+1,ec->mdlQ2->num_skins);
        TextOut(hmemdc,leftS,lineC,outstr,len);
    }

    HDC hdc = GetDC(hwnd);
    RECT rc, tr;
    GetClientRect(hwnd, &rc);
    //redraw right and bottom sides of dialog face
    SetRect(&tr, rc.left,MonsterRect.bottom,rc.right,rc.bottom);
    FillRect(hdc, &tr, GetSysColorBrush(COLOR_3DFACE));
    SetRect(&tr, MonsterRect.right,rc.top,rc.right,rc.bottom);
    FillRect(hdc, &tr, GetSysColorBrush(COLOR_3DFACE));
    //blit monster image
    BitBlt(hdc, MonsterRect.left,MonsterRect.top,MonsterRect.right-MonsterRect.left,MonsterRect.bottom-MonsterRect.top, hmemdc,0,0,SRCCOPY);
    ReleaseDC(hwnd,hdc);
}

/*void TMonsterWindow::ClearContents()
{
	if (IsIconic())
		return;
	if (!hmemdc || !hmembitmap)
		return;

    HDC hdc = GetDC(hwnd);
    RECT rc;
    GetClientRect(hwnd, &rc);
    FillRect(hdc, &rc, GetSysColorBrush(COLOR_3DFACE));
    ReleaseDC(hwnd,hdc);

	if (mimagebuffer && mzbuffer) {
		r_picbuffer = mimagebuffer;
		r_zbuffer = mzbuffer;
		REN_ClearBuffers();
	}
}*/

TMonsterWindow::~TMonsterWindow()
{
    KillTimer(hwnd,3);
    delete MonsterList;
    delete MonsterUp;
    delete MonsterDown;
    delete MonsterLeft;
    delete MonsterRight;
    delete bReset;

    DeleteObject(MonsterFont);

    delete m_pBmInfo;
    delete mzbuffer;
    delete mimagebuffer;

    RemoveMemDC();
}

void TMonsterWindow::EvSize(UINT sizeType, LPSIZE sz)
{
    MonsterRect.left = MLEFT;
    MonsterRect.right = max(MonsterRect.left,sz->cx-MonsterRect.left);
    MonsterRect.top = MTOP;
    MonsterRect.bottom = max(MonsterRect.top,sz->cy-MonsterRect.top);

    int y = max(0, int(sz->cy - 15));
    int x = max(0, int((sz->cx - 80) / 2));

    MoveWindow(MonsterLeft->hwnd,x,y,40,14,true);
    MoveWindow(MonsterRight->hwnd,x+40,y,40,14,true);
    InvalidateRect(MonsterLeft->hwnd,0,false);
    InvalidateRect(MonsterRight->hwnd,0,false);

    if (sz->cx != lastSz.cx || sz->cy != lastSz.cy)
    {
        RemoveMemDC();
        ConstructMemDC();
        lastSz.cx = sz->cx;
        lastSz.cy = sz->cy;
    }

    RedrawContents();
}

void TMonsterWindow::PUp()
{
    pitchangle += 2.0*M_PI/M_STEPS;
    if (pitchangle>= 2*M_PI)
        pitchangle -= 2*M_PI;

    RedrawContents();
}
void TMonsterWindow::PDown()
{
    pitchangle -= 2.0*M_PI/M_STEPS;
    if (pitchangle <= 0.0)
        pitchangle += 2*M_PI;

    RedrawContents();
}
void TMonsterWindow::RUp()
{
    rollangle += 2.0*M_PI/M_STEPS;
    if (rollangle>= 2*M_PI)
        rollangle -= 2*M_PI;

    RedrawContents();
}
void TMonsterWindow::RDown()
{
    rollangle -= 2.0*M_PI/M_STEPS;
    if (rollangle <= 0.0)
        rollangle += 2*M_PI;

    RedrawContents();
}
void TMonsterWindow::InButton()
{
    s += 5.0;
    RedrawContents();
}
void TMonsterWindow::OutButton()
{
    s -= 5.0;
    RedrawContents();
}

