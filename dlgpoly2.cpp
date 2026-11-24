#include "global.h"

#define MIP_W 320
#define MIP_H 200
#define MIP_L 30
#define MIP_T 50

// Load texture .wal ...
TWalDlg::TWalDlg(HWND parent, TWalTransferBuffer *xfer)
    : WDialog(parent, IDD_LOADTEXTURE)
{
    newTex = NULL;
    this->xfer = xfer;
}

TWalDlg::~TWalDlg()
{
    delete dir;
    delete wal;

    delete contents;
    delete flags;
    delete values;

    delete newTex;
}

INT_PTR TWalDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        SetupWindow();
        return TRUE;
    case WM_PAINT:
        EvPaint();
        return FALSE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_COMBOBOX6:
            if(HIWORD(wParam) == CBN_SELCHANGE)
                EvCBNSelChange();
            return TRUE;
        case IDC_LISTBOX3:
            if(HIWORD(wParam) == LBN_SELCHANGE)
                EvLBNSelChange();
            return TRUE;
        case IDOK:
            dir->GetText(xfer->sel_dir);
            wal->GetString(xfer->sel_wal, wal->GetSelIndex());
            EndDialog(hwnd, IDOK);
            return TRUE;
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
    }
    return FALSE;
}

void TWalDlg::SetupWindow()
{
    CenterWindow(hwnd);

    dir = new WCombo(hwnd, IDC_COMBOBOX6, 256);
    wal = new WList(hwnd, IDC_LISTBOX3);

    contents = new WStatic(hwnd, IDC_STATICTEXT11);
    flags = new WStatic(hwnd, IDC_STATICTEXT4);
    values = new WStatic(hwnd, IDC_STATICTEXT3);

    // need to fill in the combo and list boxes...
    dir->ClearList();
    wal->ClearList();

    struct _finddata_t ffb;
    memset(&ffb,0,sizeof(ffb));
    //ffb.ff_attrib = FA_DIREC;
    char *path = set.texture_path;
    char dirname[256];
    sprintf(dirname,"%s\\*.*",path);
    int ff = _findfirst(dirname,&ffb);
    int nomorefiles = ff < 0 ? 1 : 0;

    while (!nomorefiles)
    {
        if (!(ffb.attrib & _A_SUBDIR) || (ffb.name[0] == '.'))
        {
            nomorefiles = _findnext(ff, &ffb);
            continue;
        }
        dir->AddString(ffb.name);
        nomorefiles = _findnext(ff, &ffb);
    }
    _findclose(ff);

    if (dir->GetCount() <= 0)
        return;

    dir->SetSelIndex(0);
    dir->GetText(dirname,256);

    contents->SetText(const_cast<char *> ("Contents"));
    flags->SetText(const_cast<char *> ("Flags"));
    values->SetText(const_cast<char *> ("Value"));

    LoadListBox(dirname);
    // now populate the list box...
    InvalidateRect(hwnd,0,TRUE);
}

void TWalDlg::LoadListBox(char *dirname)
{
    if (!dirname || !*dirname)
        return;

    wal->ClearList();

    char walName[128];
    struct _finddata_t ffb;
    memset(&ffb,0,sizeof(ffb));
    char path[256];
    sprintf(path,"%s\\%s\\*.wal",set.texture_path, dirname);
    int ff = _findfirst(path,&ffb);
    int nomorefiles = ff < 0 ? 1 : 0;
    while (!nomorefiles)
    {
        STRNCPY(walName,ffb.name);
        // Load it in...
        StripExtension(walName);
        wal->AddString(walName);
        nomorefiles = _findnext(ff, &ffb);
    }
    _findclose(ff);
}

// Combo Box change populates the list box...
void TWalDlg::EvCBNSelChange()
{
    if (dir->GetCount() <= 0)
        return;

    char dirname[256];
    dir->GetText(dirname);

    LoadListBox(dirname);
    wal->SetSelIndex(0);
    EvLBNSelChange();
}

//TODO - this function looks incredibly wasteful
void TWalDlg::EvLBNSelChange()
{
    int idx;
    BITMAPINFOHEADER m_pBmInfoHeader;
    char filename[512];
    char walName[128],dirname[MAX_PATH];

    if (wal->GetCount() <= 0)
        return;
    if((idx = wal->GetSelIndex()) < 0)
        return;
    if ((idx = wal->GetString(walName,idx)) < 0)
        return;
    if (!walName || !*walName)
        return;

    dir->GetText(dirname,256);
    if (!dirname || !*dirname)
        return;

    // Load it in...
    StripExtension(walName);

    // load the file
    snprintf(filename, sizeof(filename), "%s/%s/%s.wal", set.texture_path, dirname, walName);

    unsigned char *lump;
    if(!(lump = file_get_contents(filename)))
    {
        syserror(const_cast<char *> ("Error: loading .wal \"%s\" failed!\n"), filename);
        return;
    }

    miptex2_t *qtex = (miptex2_t *)lump;
    int width = qtex->width;
    int height = qtex->height;

    delete newTex;
    newTex = new texInfo_t;
    newTex->def_flags    = qtex->flags;
    newTex->def_value    = qtex->value;
    newTex->def_contents = qtex->contents;
    newTex->texType = TEX_TYPE_Q2;
    strncpy(newTex->basepath,dirname,sizeof(newTex->basepath));
    newTex->basepath[sizeof(newTex->basepath)-1] = 0;

    strncpy (qtex->name, walName, sizeof(qtex->name));
    qtex->name[sizeof(qtex->name) - 1] = 0;
    StripExtension (qtex->name);

    char tempNm[LUMP_NAME_LENGTH_MAX];
    strncpy(tempNm,qtex->name,sizeof(tempNm));
    tempNm[LUMP_NAME_LENGTH - 1] = 0;
    strncpy(newTex->bmp,tempNm,sizeof(newTex->bmp));
    newTex->bmp[sizeof(newTex->bmp)-1] = 0;
    strupr(newTex->bmp);

    newTex->index = 0;

    uintptr_t size = width * height * 85/64;

    newTex->h = height;
    newTex->w = width;

    int count = size;
    unsigned char *dest = new unsigned char[size+width+1];
    unsigned char *source = (unsigned char *)qtex;
    source += qtex->offsets[0];

    memcpy(dest, source, count);

    newTex->bits = dest;
    newTex->flat = texWindow->TranslateColors(256,set.pal,dest,size);

    delete[] lump;		// done with data from disk

    if (set.glBsp)
        texWindow->CalcGLBits(newTex,set.pal);

    BITMAPINFO *m_pBmInfo;

    memset(&m_pBmInfoHeader,0,sizeof(BITMAPINFOHEADER));
    m_pBmInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
    m_pBmInfoHeader.biWidth = width;
    m_pBmInfoHeader.biHeight = -height;
    m_pBmInfoHeader.biPlanes = 1;
    m_pBmInfoHeader.biBitCount = 8;
    m_pBmInfoHeader.biCompression = BI_RGB;
    m_pBmInfoHeader.biSizeImage = width * height;

    m_pBmInfoHeader.biClrUsed = 256;
    m_pBmInfo = (BITMAPINFO*) new char[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];

    memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
    memcpy(m_pBmInfo->bmiColors,set.cTable,256*sizeof(RGBQUAD));

    newTex->dibInfo = m_pBmInfo;
    newTex->truedibInfo = NULL; // tm_pBmInfo;

    //dimensions
    RECT r, rc;
    GetClientRect(hwnd,&r);

    int l = 10;
    int t = 25;
    int tw = 128;
    int th = 128;

    //gdi
    HDC hdc = GetDC(hwnd);

    HBRUSH fColor = CreateSolidBrush(set.color_foreground);
    HBRUSH bColor = CreateSolidBrush(set.color_background);

    // TODO: not sure if this needs to be drawn off screen. the owl code was pretty gkgpgnarly
//	HDC cdc = CreateCompatibleDC(hdc);
//	HBITMAP hbmp = CreateCompatibleBitmap(hdc,r.right,r.bottom);
//	HGDIOBJ oldcdcbmp = SelectObject(cdc, hbmp);

//	SetStretchBltMode(hdc, COLORONCOLOR);

//	StretchDIBits(hdc, 0,0,newTex->w,newTex->h, 0,0,newTex->w,newTex->h,
//    	newTex->bits, newTex->dibInfo, DIB_RGB_COLORS, SRCCOPY);

    SetRect(&rc, r.left + l,r.top+t,r.left+l+tw,r.top+t+th);
    FillRect(hdc, &rc, bColor);

    if (newTex->dibInfo && newTex->bits)
    {
        RECT dst, src;
        dst.left = r.left+l;
        dst.top = r.top+t;
        src.left = 0;
        src.top = 0;
        int hs = set.stretch_textures;
        int hn = set.nosmallstretch;

        set.stretch_textures = 1;
        set.nosmallstretch = 1;

        texWindow->GetTextureDisplayRects(dst.left, dst.top, &src, &dst, newTex, tw, th);
        set.stretch_textures = hs;
        set.nosmallstretch = hn;

        SetStretchBltMode(hdc, COLORONCOLOR);
        StretchDIBits(hdc,
                      dst.left,dst.top,dst.right-dst.left,dst.bottom-dst.top,
                      src.left,src.top,src.right-src.left,src.bottom-src.top,
                      newTex->bits,
                      newTex->dibInfo,
                      DIB_RGB_COLORS, SRCCOPY);

        char outstr[128];
        sprintf(outstr,"Contents:  %i",newTex->def_contents);
        contents->SetText(outstr);
        sprintf(outstr,"Flags:  %i",newTex->def_flags);
        flags->SetText(outstr);
        sprintf(outstr,"Value:  %i",newTex->def_value);
        values->SetText(outstr);
    }

    SetRect(&rc, r.left + l - 1,r.top+t - 1, r.left+l+tw + 1,r.top+t+th + 1);
    FrameRect(hdc, &rc, fColor);

    ReleaseDC(hwnd, hdc);
    DeleteObject(bColor);
    DeleteObject(fColor);
}

void TWalDlg::EvPaint()
{
    HDC hdc = GetDC(hwnd);
    HBRUSH fColor = CreateSolidBrush(set.color_foreground);
    HBRUSH bColor = CreateSolidBrush(set.color_background);

    RECT r, rc;
    GetClientRect(hwnd, &r);

    int l = 10;
    int t = 25;
    int tw = 128;
    int th = 128;

    SetRect(&rc, r.left + l,r.top+t,r.left+l+tw,r.top+t+th);
    FillRect(hdc, &rc, bColor);

    if (newTex)
    {
        if (newTex->dibInfo && newTex->bits)
        {
            RECT dst, src;
            dst.left = r.left+l;
            dst.top = r.top+t;

            texWindow->GetTextureDisplayRects(dst.left, dst.top, &src, &dst, newTex, tw, th);

            SetStretchBltMode(hdc, COLORONCOLOR);

            StretchDIBits(hdc,
                          dst.left,dst.top,dst.right-dst.left,dst.bottom-dst.top,
                          src.left,src.top,src.right-src.left,src.bottom-src.top,
                          newTex->bits,
                          newTex->dibInfo, DIB_RGB_COLORS, SRCCOPY);
        }
        char outstr[128];
        sprintf(outstr,"Contents:  %i",newTex->def_contents);
        contents->SetText(outstr);
        sprintf(outstr,"Flags:  %i",newTex->def_flags);
        flags->SetText(outstr);
        sprintf(outstr,"Value:  %i",newTex->def_value);
        values->SetText(outstr);

    }
    else
    {
        HGDIOBJ oldfont = SelectObject(hdc, set.font12);
        SetTextColor(hdc, set.color_foreground);
        SetBkMode(hdc,TRANSPARENT);
        char outstr[128];
        int len;
        len = sprintf(outstr,"Select a .wal file");
        TextOut(hdc, r.left + l + 2, r.top + t + 2, outstr, len);
        len = sprintf(outstr,"from the list...");
        TextOut(hdc, r.left + l + 2, r.top + t + 2 + 14, outstr, len);

        SelectObject(hdc, oldfont);

        contents->SetText(const_cast<char *> ("Contents"));
        flags->SetText(const_cast<char *> ("Flags"));
        values->SetText(const_cast<char *> ("Value"));
    }

    SetRect(&rc, r.left + l - 1,r.top+t - 1, r.left+l+tw + 1,r.top+t+th + 1);
    FrameRect(hdc, &rc, fColor);

    ReleaseDC(hwnd, hdc);
    DeleteObject(bColor);
    DeleteObject(fColor);
}

// ***********************************************************************************
// Stair Dialog Box

INT_PTR TStairDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        SetupWindow();
        return TRUE;
    case WM_PAINT:
        EvPaint();
        return FALSE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            CmOk();
            return TRUE;
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }

        if(HIWORD(wParam)==BN_CLICKED)
        {
            switch(LOWORD(wParam))
            {
            case IDC_PUSHBUTTON11:
                ResetAngles();
                return TRUE;
            case IDC_PUSHBUTTON12:
                LeftButton();
                return TRUE;
            case IDC_PUSHBUTTON13:
                RightButton();
                return TRUE;
            case IDC_PUSHBUTTON15:
                InButton();
                return TRUE;
            case IDC_PUSHBUTTON14:
                OutButton();
                return TRUE;
            case IDC_PUSHBUTTON16:
                PUp();
                return TRUE;
            case IDC_PUSHBUTTON17:
                PDown();
                return TRUE;
            case IDC_PUSHBUTTON18:
                RUp();
                return TRUE;
            case IDC_PUSHBUTTON19:
                RDown();
                return TRUE;

            case IDC_RADIOBUTTON1:
            case IDC_RADIOBUTTON2:
            case IDC_RADIOBUTTON3:
            case IDC_CHECKBOX1:
            case IDC_CHECKBOX2:
                Preview();
                return TRUE;
            }
        }

        if(HIWORD(wParam)==EN_CHANGE)
        {
            switch(LOWORD(wParam))
            {
            case IDC_EDIT1:
            case IDC_EDIT2:
            case IDC_EDIT3:
            case IDC_EDIT4:
            case IDC_EDIT5:
                Preview();
                return TRUE;
            }
        }

        break;
    case WM_MOUSEMOVE:
    {
        MAKEPOINT(pt, lParam);
        EvMouseMove(wParam,pt);
        return TRUE;
    }
    case WM_LBUTTONDOWN:
    {
        MAKEPOINT(pt, lParam);
        EvLButtonDown(wParam,pt);
        return TRUE;
    }
    case WM_LBUTTONUP:
    {
        MAKEPOINT(pt, lParam);
        EvLButtonUp(wParam,pt);
        return TRUE;
    }
    }
    return FALSE;
}

TStairDlg::TStairDlg(HWND parent, TStairTransferBuffer *xfer)
    : WDialog(parent, STAIR_DIALOG)
{
    xferBuffer = xfer;

    hmemdc = 0;
    hmembitmap = 0;

    mode = 0;

    PreviewRect.left   = STAIR_PREV_LEFT;
    PreviewRect.right  = PreviewRect.left + STAIR_W;
    PreviewRect.top    = STAIR_PREV_TOP;
    PreviewRect.bottom = PreviewRect.top + STAIR_H;

    PreviewSize.cx = PreviewRect.right-PreviewRect.left;
    PreviewSize.cy = PreviewRect.bottom-PreviewRect.top;

    c_yaw      = M_PI;
    v_roll     = 1.5f*M_PI;
    v_pitch    = 0.0;
    stepHeight = 1.0;
    dist       = 1.0;
    width      = 1.0;
    height     = 1.0;
    p          = 1.0;

    fakeEnt = new Entity;

    fakeBrush = map_i[set.curmap]->selectedBrush()->copy(); // make a copy
    fakeBrush->setParent(fakeEnt); // point to our fake entity...
    fakeBrush->calcWindings(); // so it is visible...
    fakeEnt->addObject(fakeBrush);

    SetBrush *b = fakeBrush;

    height = (b->bmaxs[2]-b->bmins[2]) / 2.0f;
    width  = max(b->bmaxs[1]-b->bmins[1],b->bmaxs[0]-b->bmins[0]) / 2.0f;

    p = set.ppd*set.mag; // from main program...

    // figure the larger way...
    float dist1 = (height*p/(PreviewSize.cy/2.0f));
    float dist2 = (width *p/(PreviewSize.cx/2.0f));
    dist = max(dist1,dist2);
    s = 0.0;

    ctr[0] = (b->bmins[0]+b->bmaxs[0])/2.0f;
    ctr[1] = (b->bmins[1]+b->bmaxs[1])/2.0f;
    ctr[2] = (b->bmins[2]+b->bmaxs[2])/2.0f;

    Reconstruct();
}

void TStairDlg::CmOk()
{
    if (stepHeight >= 18.0)
    {
        char s[256];
        if (stepHeight < 40.0)
        {
            sprintf(s,"Step of %5.1f height can be jumped (< 40),\nbut not stepped (< 20), okay to save?",stepHeight);
        }
        else
        {
            sprintf(s,"Step of %5.1f height cannot be jumped (< 40)\nor stepped (< 20), okay to save anyway?",stepHeight);
        }
        int retval = MessageBox(hwnd, s, "BSP - Confirm Step Height", MB_YESNO | MB_ICONQUESTION);

        if (retval != IDYES)
        {
            return;
        }
    }
    EndDialog(hwnd, IDOK);
}

void TStairDlg::EvLButtonDown(UINT modKeys, POINT& point)
{
    if (mode)  // already processing...
        return;
    if (!PtInRect(&PreviewRect,point)) // not close enough.
        return;

    SetCapture(hwnd);

    startPt = point;
    lastPt = point;
    mode = modKeys & MK_SHIFT ? DRAG_DISTANCE : DRAG_ROTATE;
}

void TStairDlg::EvLButtonUp(UINT /*modKeys*/, POINT& /*point*/)
{
    ReleaseCapture();
    mode = 0;
}
void TStairDlg::EvMouseMove(UINT /*modKeys*/, POINT& point)
{
    if (!mode) // just ignore...
        return;

    // process...
    if (point.x == lastPt.x && point.y == lastPt.y)
        return;  // no move...

    int dH1 =   DRAG_DELTA/3;
    int dH2 = 2*DRAG_DELTA/3;

    int dx = point.x - lastPt.x;
    int dy = point.y - lastPt.y;

    if ((abs(dx) < DRAG_DELTA) && (abs(dy) < DRAG_DELTA))
        return;

    lastPt = point;

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
}

void TStairDlg::SetupWindow()
{
    CenterWindow(hwnd);

    NumSteps = new WEdit(hwnd, IDC_EDIT1, 0, MAX_EDITLEN);
    HGap = new WEdit(hwnd, IDC_EDIT2, 0, MAX_EDITLEN);
    VGap = new WEdit(hwnd, IDC_EDIT3, 0, MAX_EDITLEN);
    NumTurns = new WEdit(hwnd, IDC_EDIT4, 0, MAX_EDITLEN);
    Dist = new WEdit(hwnd, IDC_EDIT5, 0, MAX_EDITLEN);
    RegularButton = new WRadio(hwnd, IDC_RADIOBUTTON1, xferBuffer->RegularButton);
    SpiralButton = new WRadio(hwnd, IDC_RADIOBUTTON2, xferBuffer->SpiralButton);
    BlockButton = new WRadio(hwnd, IDC_RADIOBUTTON3, xferBuffer->BlockButton);
    XAxis = new WCheck(hwnd, IDC_CHECKBOX1, xferBuffer->XAxis);
    GoesDown = new WCheck(hwnd, IDC_CHECKBOX2, xferBuffer->GoesDown);

    NumSteps->SetInt(xferBuffer->Dist);
    HGap->SetInt(xferBuffer->HGap);
    VGap->SetInt(xferBuffer->VGap);
    Dist->SetInt(xferBuffer->Dist);
    NumTurns->SetFloat(xferBuffer->NumTurns);

    ConstructMemDC();
}

TStairDlg::~TStairDlg()
{
    if (fakeEnt)
    {
        fakeEnt->freeObjects(true);
        delete fakeEnt;
        fakeEnt = NULL;
    }
    delete NumSteps;
    delete HGap;
    delete VGap;
    delete NumTurns;
    delete Dist;
    delete RegularButton;
    delete SpiralButton;
    delete BlockButton;
    delete XAxis;
    delete GoesDown;

    RemoveMemDC ();
}

void TStairDlg::Reconstruct()
{
    map *m = map_i[set.curmap];

    if (fakeBrush && fakeEnt)
    {
        fakeEnt->removeObject(fakeBrush);
    }

    if (fakeEnt)   // if stuff is left over, kill it...
    {
        fakeEnt->freeObjects(true);
        delete fakeEnt;
    }
    fakeEnt = new Entity;

    delete fakeBrush;
    fakeBrush = m->selectedBrush()->copy(); // make a copy
    fakeBrush->setParent(fakeEnt);		// point to our fake entity...
    fakeBrush->calcWindings();			// so it is visible...
    fakeEnt->addObject(fakeBrush);		// put it in...

    int style;
    if (xferBuffer->RegularButton)
    {
        style = STAIR_REGULAR;
    }
    else if (xferBuffer->SpiralButton)
    {
        style = STAIR_SPIRAL;
    }
    else
    {
        style = STAIR_BLOCK;
    }

    int down     = xferBuffer->GoesDown;
    int xory     = xferBuffer->XAxis;
    int steps    = xferBuffer->NumSteps;
    int hgap     = xferBuffer->HGap;
    int vgap     = xferBuffer->VGap;
    int distfc   = xferBuffer->Dist;
    float turns    = xferBuffer->NumTurns;

    // error checking...
    steps  = max(1,steps);
    hgap   = max(1,hgap);
    vgap   = max(0,vgap);
    turns  = max(0.1f,turns);
    distfc = max(1,distfc);

    // make them...don't delete the "seed" brush...
    stepHeight = m->makeStairs(fakeEnt,fakeBrush,false,style,steps,hgap,vgap,down,xory,turns,distfc);
}

void TStairDlg::Redraw(HDC hdc)
{
    float NewQ[4][4];
    float T[4][4];
    float V[4][4];
    vec3_t vp;

    vp[0] = ctr[0] + (s+dist+width) * cos(c_yaw); // move out to right...
    vp[1] = ctr[1] + (s+dist+width) * sin(c_yaw);
    vp[2] = ctr[2];

    findQ2(NewQ,vp,ctr);
    rot3(1,0.0,T);
    mult3(NewQ,T,V);
    rot3(3,v_roll,NewQ);
    mult3(NewQ,V,T);
    rot3(2,v_pitch,V);
    mult3(T,V,U);

    HPEN pen = CreatePen(PS_SOLID,1,RGB(192,192,192));
    HGDIOBJ oldpen = SelectObject(hdc, pen);
    HBRUSH brwhite = CreateSolidBrush(RGB(255,255,255));
    HGDIOBJ oldbrush = SelectObject(hdc, brwhite);

    RECT rc;
    SetRect(&rc, 0, 0, PreviewSize.cx, PreviewSize.cy);
    Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);

    for (SetBrush *b = fakeEnt->objects.p_next; b != &fakeEnt->objects; b = b->p_next)
    {
        if (b == fakeBrush)
            continue;

        b->CameraDrawSelfSpecial(hdc,p,RGB(255,0,0),U,set.near_clip_distance,set.far_clip_distance, &rc, &PreviewSize);
    }

    // now draw the seed brush...
    fakeBrush->CameraDrawSelfSpecial(hdc,p,RGB(0,0,255),U,set.near_clip_distance,set.far_clip_distance, &rc, &PreviewSize);

    SelectObject(hdc, set.font9);
    SetTextColor(hdc, RGB(0,0,0));
    SetBkColor(hdc, RGB(255,255,255));
    char outstr[128];
    int len = sprintf(outstr," Step Height: %3.2f R(%i) P(%i) Y(%i) ", stepHeight,
                      RAD2DEG(v_roll), RAD2DEG(v_pitch), RAD2DEG(c_yaw));
    TextOut(hdc,rc.left+4,rc.top+2,outstr,len);

    SelectObject(hdc, oldpen);
    SelectObject(hdc, oldbrush);
    DeleteObject(pen);
    DeleteObject(brwhite);
}

void TStairDlg::MemDraw()
{
    if (!hmemdc || !hmembitmap)
        return;

    HDC hdc = GetDC(hwnd);

    Redraw(hmemdc);
    BitBlt(hdc, PreviewRect.left,PreviewRect.top,PreviewRect.right-PreviewRect.left,PreviewRect.bottom-PreviewRect.top,
           hmemdc, 0,0, SRCCOPY);

    ReleaseDC(hwnd, hdc);
}

void TStairDlg::RemoveMemDC()
{
    if (hmemdc)
        DeleteDC(hmemdc);
    if (hmembitmap)
        DeleteObject(hmembitmap);
    hmemdc = 0;
    hmembitmap = 0;
}

void TStairDlg::ConstructMemDC()
{
    HDC hdc = GetDC(hwnd);

    hmemdc       = CreateCompatibleDC(hdc);
    hmembitmap = CreateCompatibleBitmap(hdc, PreviewRect.right-PreviewRect.left, PreviewRect.bottom-PreviewRect.top);

    SelectObject(hmemdc, hmembitmap); // install bitmap into memDC

    ReleaseDC(hwnd, hdc);
}

void TStairDlg::EvPaint()
{
    // Invoke base class...
    //TDialog::EvPaint();
    MemDraw();
}
void TStairDlg::ResetAngles()
{
    c_yaw   = M_PI;
    v_roll  = 1.5f*M_PI;
    v_pitch = 0.0;
    s       = 0.0;

    MemDraw();
}
void TStairDlg::Preview()
{
    if (!hmemdc || !hmembitmap)
        return;

    if(GoesDown)  	// make sure last control was created
    {
        //edit
        xferBuffer->NumSteps = NumSteps->GetInt();
        xferBuffer->HGap = HGap->GetInt();
        xferBuffer->VGap = VGap->GetInt();
        xferBuffer->Dist = Dist->GetInt();
        xferBuffer->NumTurns = NumTurns->GetFloat();
        //radio
        xferBuffer->RegularButton = RegularButton->GetCheck();
        xferBuffer->SpiralButton = SpiralButton->GetCheck();
        xferBuffer->BlockButton = BlockButton->GetCheck();
        //check
        xferBuffer->XAxis = XAxis->GetCheck();
        xferBuffer->GoesDown = GoesDown->GetCheck();
    }

    Reconstruct();

    MemDraw();
}
void TStairDlg::PUp()
{
    v_pitch += 2.0f*M_PI/STEPS;
    if (v_pitch>= 2*M_PI)
        v_pitch -= 2*M_PI;

    MemDraw();
}
void TStairDlg::PDown()
{
    v_pitch -= 2.0f*M_PI/STEPS;
    if (v_pitch <= 0.0)
        v_pitch += 2*M_PI;

    MemDraw();
}

void TStairDlg::RUp()
{
    v_roll += 2.0f*M_PI/STEPS;
    if (v_roll>= 2*M_PI)
        v_roll -= 2*M_PI;

    MemDraw();
}

void TStairDlg::RDown()
{
    v_roll -= 2.0f*M_PI/STEPS;
    if (v_roll <= 0.0)
        v_roll += 2*M_PI;

    MemDraw();
}
void TStairDlg::LeftButton()
{
    c_yaw -= 2.0f*M_PI/STEPS;
    if (c_yaw <= 0.0)
        c_yaw += 2*M_PI;

    MemDraw();
}
void TStairDlg::RightButton()
{
    c_yaw += 2.0f*M_PI/STEPS;
    if (c_yaw>= 2*M_PI)
        c_yaw -= 2*M_PI;

    MemDraw();
}
void TStairDlg::InButton()
{
    s += dist/10.0f;
    MemDraw();
}
void TStairDlg::OutButton()
{
    s -= dist/10.0f;
    MemDraw();
}

// ***********************************************************************************
// Scan texture directorys...
TScanDlg::TScanDlg(HWND parent, TScanTransferBuffer *xfer)
    : WDialog(parent, IDD_LOADWAD)
{
    this->xfer = xfer;
}
TScanDlg::~TScanDlg()
{
    delete Directory;
}
INT_PTR TScanDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        CenterWindow(hwnd);
        Directory = new WCombo(hwnd, IDC_COMBOBOX5, MAX_PATH);
        xfer->Directory.FillCombo(Directory->hwnd);
        if(*xfer->caption)
        {
            SetWindowText(hwnd,xfer->caption);
        }
        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            Directory->GetText(xfer->selected);
            EndDialog(hwnd, IDOK);
            return TRUE;
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
    }
    return FALSE;
}

// ***********************************************************************************
// Scan game directories...
TGameDlg::TGameDlg(HWND parent, TGameBuffer *xfer)
    : WDialog(parent, IDD_GAME)
{
    this->xfer = xfer;
    games = 0;
}
TGameDlg::~TGameDlg()
{
    delete games;
    delete opengl;
}
INT_PTR TGameDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        CenterWindow(hwnd);
        opengl = new WCheck(hwnd, IDC_OPENGL, xfer->opengl);
        games = new WList(hwnd, IDC_LIST1);
        xfer->dir.FillList(games->hwnd);
        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            games->GetString(xfer->selected, games->GetSelIndex());
            xfer->opengl = opengl->GetCheck();
            EndDialog(hwnd, IDOK);
            return TRUE;
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
    }
    return FALSE;
}

// ***********************************************************************************
// Export settings...
TExportDlg::TExportDlg(HWND parent, TExportTransferBuffer *xfer)
    : WDialog(parent, IDD_EXPORT)
{
    this->xfer = xfer;
}
TExportDlg::~TExportDlg()
{
    delete inBox;
    delete sky;
    delete walls;
    delete floor;
    delete exporter;
}
INT_PTR TExportDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        CenterWindow(hwnd);
        inBox = new WCheck(hwnd, IDC_CHECKBOX26, xfer->inBox);
        sky = new WCombo(hwnd, IDC_COMBOBOX5, 128);
        walls = new WCombo(hwnd, IDC_COMBOBOX4, 128);
        floor = new WCombo(hwnd, IDC_COMBOBOX6, 128);
        exporter = new WCombo(hwnd, IDC_COMBOBOX7, 128);
        xfer->sky.FillCombo(sky->hwnd);
        xfer->walls.FillCombo(walls->hwnd);
        xfer->floor.FillCombo(floor->hwnd);
        xfer->exporter.FillCombo(exporter->hwnd);
        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            xfer->inBox = inBox->GetCheck();
            sky->GetText(xfer->sel_sky);
            walls->GetText(xfer->sel_walls);
            floor->GetText(xfer->sel_floor);
            exporter->GetText(xfer->sel_exporter);
            EndDialog(hwnd, IDOK);
            return TRUE;
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
    }
    return FALSE;
}

// ***********************************************************************************
// Mip Map Dialog Box
#define MMD_MODE_SIZE 1
#define MMD_MODE_MOVE 2
INT_PTR TMipMapDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        SetupWindow();
        return TRUE;
    case WM_PAINT:
        EvPaint();
        return FALSE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
        {
            name->GetText(xferBuffer->Name);
            xferBuffer->Width = we->GetInt();
            xferBuffer->Height = he->GetInt();
            xferBuffer->Top = te->GetInt();
            xferBuffer->Left = le->GetInt();
        }
        EndDialog(hwnd, IDOK);
        return TRUE;
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        case IDC_EDIT50:
        case IDC_EDIT9:
        case IDC_EDIT51:
        case IDC_EDIT52:
        case IDC_EDIT39:
            if(!mode && name && HIWORD(wParam)==EN_CHANGE)
            {

                w = we->GetInt();
                h = he->GetInt();
                top = te->GetInt();
                left = le->GetInt();

                Preview();
                return TRUE;
            }
        }
        break;
    case WM_MOUSEMOVE:
    {
        MAKEPOINT(pt, lParam);
        EvMouseMove(wParam,pt);
        return TRUE;
    }
    case WM_LBUTTONDOWN:
    {
        MAKEPOINT(pt, lParam);
        EvLButtonDown(wParam,pt);
        return TRUE;
    }
    case WM_LBUTTONUP:
    {
        MAKEPOINT(pt, lParam);
        EvLButtonUp(wParam,pt);
        return TRUE;
    }
    }
    return FALSE;
}

TMipMapDlg::TMipMapDlg(HWND parent, TMipMapTransferBuffer *xfer, unsigned char *image_i, int ix_i, int iy_i)
    : WDialog(parent, IDD_MIPMAP)
{
    ix = ix_i;	// input width of image
    iy = iy_i;  // input height of image

    image = image_i;	// image bits

    name = 0;
    we = 0;
    he = 0;
    te = 0;
    le = 0;

    mode = 0;
    w = h = 64;
    top = left = 0;

    hmemdc = 0;
    hmembitmap = NULL;
    xferBuffer = xfer;

    PreviewRect.left   = MIP_L;
    PreviewRect.right  = PreviewRect.left + MIP_W;
    PreviewRect.top    = MIP_T;
    PreviewRect.bottom = PreviewRect.top + MIP_H;
}

void TMipMapDlg::EvLButtonDown(UINT /*modKeys*/, POINT& point)
{
    if (mode)  // already processing...
        return;
    if (!PtInRect(&PreviewRect,point)) // not clicking in image
        return;

    SetCapture(hwnd);

    startPt = point;
    lastPt = point;

    RECT rc;
    SetRect(&rc, left, top, left+w, top+h);

    int a, b;

    Interpolate(rc.left, rc.top, &a, &b);
    rc.left = a;
    rc.top = b;
    Interpolate(rc.right, rc.bottom, &a, &b);
    rc.right = a;
    rc.bottom = b;

    // move over since we are not in memdc
    OffsetRect(&rc, PreviewRect.left, PreviewRect.top);

    mode = PtInRect(&rc,point) ? MMD_MODE_MOVE : MMD_MODE_SIZE;
}

void TMipMapDlg::Interpolate(int x, int y, int *outx, int *outy)
{
    *outx = x * (PreviewRect.right-PreviewRect.left) / ix;
    *outy = y * (PreviewRect.bottom-PreviewRect.top) / iy;
}
void TMipMapDlg::EvLButtonUp(UINT /*modKeys*/, POINT& point)
{
    ReleaseCapture();
    mode = 0;
}
void TMipMapDlg::EvMouseMove(UINT /*modKeys*/, POINT& point)
{
    if (!mode) // just ignore...
        return;
    if (point.x == lastPt.x && point.y == lastPt.y)
        return;  // no move...

    int x, y;
    if (mode == MMD_MODE_SIZE)
    {

        Interpolate(abs(point.x-startPt.x), abs(point.y-startPt.y), &x, &y);

        w = 16 * (int)((x + 15)/16);
        h = 16 * (int)((y + 15)/16);

        we->SetInt(w);
        he->SetInt(h);
    }
    else  	// MMD_MODE_MOVE
    {
        int dx = point.x - lastPt.x;
        int dy = point.y - lastPt.y;

        Interpolate(dx, dy,&x,&y);

        top = max(0,top + y);		// upper left
        left = max(0,left + x);

        top = min(top, iy-h);		// lower right
        left = min(left, ix-w);
    }
    // fill in details
    te->SetInt(top);
    le->SetInt(left);

    lastPt = point;

    MemDraw();
}

void TMipMapDlg::SetupWindow()
{
    CenterWindow(hwnd);

    name = new WEdit(hwnd, IDC_EDIT50, xferBuffer->Name, 33);
    we = new WEdit(hwnd, IDC_EDIT9, 0, MAX_EDITLEN);
    he = new WEdit(hwnd, IDC_EDIT51, 0, MAX_EDITLEN);
    te = new WEdit(hwnd, IDC_EDIT52, 0, MAX_EDITLEN);
    le = new WEdit(hwnd, IDC_EDIT39, 0, MAX_EDITLEN);

    //copy xfer values to edit controls
    we->SetInt(xferBuffer->Width);
    he->SetInt(xferBuffer->Height);
    te->SetInt(xferBuffer->Top);
    le->SetInt(xferBuffer->Left);

    top  = xferBuffer->Top;
    left = xferBuffer->Left;
    w    = xferBuffer->Width;
    h    = xferBuffer->Height;

    ConstructMemDC();
}
TMipMapDlg::~TMipMapDlg()
{
    delete name;
    delete we;
    delete he;
    delete te;
    delete le;

    RemoveMemDC ();
}
void TMipMapDlg::Redraw(HDC hdc)
{
    RECT rc;
    int a, b;

    Interpolate(left, top, &a, &b);
    rc.left = a;
    rc.top = b;
    Interpolate(left+w, top+h, &a, &b);
    rc.right = a;
    rc.bottom = b;

    BITMAPINFO *m_pBmInfo;
    BITMAPINFOHEADER m_pBmInfoHeader;

    memset(&m_pBmInfoHeader,0,sizeof(BITMAPINFOHEADER));
    m_pBmInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
    m_pBmInfoHeader.biWidth = ix;
    m_pBmInfoHeader.biHeight = -iy;
    m_pBmInfoHeader.biPlanes = 1;
    m_pBmInfoHeader.biCompression = BI_RGB;

    if (set.sinBsp)
    {
        //24bit image
        m_pBmInfoHeader.biBitCount =24;
        m_pBmInfoHeader.biClrUsed = 0;
        m_pBmInfoHeader.biSizeImage = 0;

        m_pBmInfo = (BITMAPINFO*) new char[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];
        memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
        memset(m_pBmInfo->bmiColors,0,256*sizeof(RGBQUAD));
    }
    else
    {
        //8bit pal
        m_pBmInfoHeader.biBitCount = 8;
        m_pBmInfoHeader.biClrUsed = 256;
        m_pBmInfoHeader.biSizeImage = ix * iy;

        m_pBmInfo = (BITMAPINFO*) new char [sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];
        memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
        memcpy(m_pBmInfo->bmiColors,set.cTable,256*sizeof(RGBQUAD));
    }

    memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));

    SetStretchBltMode(hdc, COLORONCOLOR);

    //draw base image
    StretchDIBits(hdc,
                  0,0, PreviewRect.right-PreviewRect.left, PreviewRect.bottom-PreviewRect.top,
                  0,0, ix,iy,
                  image, m_pBmInfo, DIB_RGB_COLORS, SRCCOPY);

    //draw selection bounds
    DrawFocusRect(hdc, &rc);

    delete [] m_pBmInfo;
}

void TMipMapDlg::MemDraw()
{
    if (!hmemdc || !hmembitmap)
        return;

    HDC hdc = GetDC(hwnd);
    Redraw(hmemdc);
    BitBlt(hdc, PreviewRect.left,PreviewRect.top,PreviewRect.right-PreviewRect.left,PreviewRect.bottom-PreviewRect.top,
           hmemdc, 0,0, SRCCOPY);
    ReleaseDC(hwnd, hdc);
}

void TMipMapDlg::RemoveMemDC()
{
    if (hmemdc)
        DeleteDC (hmemdc);
    if (hmembitmap)
        DeleteObject(hmembitmap);
    hmemdc = 0;
    hmembitmap = 0;
}
void TMipMapDlg::ConstructMemDC()
{
    HDC hdc = GetDC(hwnd);

    // allocate the memory dc and the bitmap
    hmemdc     = CreateCompatibleDC(hdc);
    hmembitmap = CreateCompatibleBitmap(hdc, PreviewRect.right-PreviewRect.left, PreviewRect.bottom-PreviewRect.top);

    SelectObject(hmemdc, hmembitmap); // install bitmap into memDC

    ReleaseDC(hwnd, hdc);
}
void TMipMapDlg::EvPaint()
{
    MemDraw();
}
void TMipMapDlg::Preview()
{
    MemDraw();
}

// ***********************************************************************************
// Scan game directories...
TGLDlg::TGLDlg(HWND parent, TGLBuffer *xfer)
    : WDialog(parent, IDD_GL)
{
    this->xfer = xfer;
}
TGLDlg::~TGLDlg()
{
    delete Mode;
}
INT_PTR TGLDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        CenterWindow(hwnd);
        Mode = new WCombo(hwnd, IDC_COMBOBOX5, 256);
        xfer->Mode.FillCombo(Mode->hwnd);
        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            //	xfer->Mipmap = Mipmap->GetCheck();
            Mode->GetText(xfer->selected);
            EndDialog(hwnd, IDOK);
            return TRUE;
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
    }
    return FALSE;
}
