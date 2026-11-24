#include "global.h"
#include "propctl.h"

bool savetoini = false;


// ***********************************************************************************
//
// Arch Dialog Box
//
// ***********************************************************************************
TArchDlg::TArchDlg(HWND parent, TArchTransferBuffer *xfer)
    : WDialog(parent, ARCH_DIALOG)
{
    xferBuffer = xfer;

    Stones = 0;
    Width = 0;
    XAxis = 0;
    RectTop = 0;

    badbrush = 0;
    hmemdc = 0;
    hmembitmap = 0;
    mode = 0;

    PreviewRect.left   = ARCH_PREV_LEFT;
    PreviewRect.right  = PreviewRect.left + ARCH_W;
    PreviewRect.top    = ARCH_PREV_TOP;
    PreviewRect.bottom = PreviewRect.top + ARCH_H;

    PreviewSize.cx = PreviewRect.right - PreviewRect.left;
    PreviewSize.cy = PreviewRect.bottom - PreviewRect.top;

    c_yaw   = M_PI;
    v_roll  = 1.5f*M_PI;
    v_pitch = 0.0;
    dist    = 1.0;
    width   = 1.0;
    height  = 1.0;
    p       = 1.0;
    s		= 0.0;

    fakeEnt = new Entity;

    fakeBrush = map_i[set.curmap]->selectedBrush()->copy(); // make a copy
    fakeBrush->setParent(fakeEnt); // point to our fake entity...
    fakeBrush->calcWindings(); // so it is visible...
    fakeEnt->addObject(fakeBrush);

    SetBrush *b = fakeBrush;

    height = (b->bmaxs[2] - b->bmins[2]) / 2.0f;
    width  = max(b->bmaxs[1] - b->bmins[1], b->bmaxs[0] - b->bmins[0]) / 2.0f;

    p = set.ppd*set.mag; // from main program...

    // figure the larger way...
    float dist1 = (height*p/(PreviewSize.cy/2.0f));
    float dist2 = (width *p/(PreviewSize.cx/2.0f));
    dist = max(dist1,dist2);

    ctr[0] = (b->bmins[0] + b->bmaxs[0]) / 2.0f;
    ctr[1] = (b->bmins[1] + b->bmaxs[1]) / 2.0f;
    ctr[2] = (b->bmins[2] + b->bmaxs[2]) / 2.0f;

    Reconstruct();
}

INT_PTR TArchDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        SetupWindow();
        return TRUE;
    case WM_PAINT:
        EvPaint();
        return FALSE;
    case WM_MOUSEMOVE:
    {
        MAKEPOINT(pt, lParam);
        EvMouseMove(wParam, pt);
    }
    return TRUE;
    case WM_LBUTTONDOWN:
    {
        MAKEPOINT(pt, lParam);
        EvLButtonDown(wParam, pt);
    }
    return TRUE;
    case WM_LBUTTONUP:
    {
        MAKEPOINT(pt, lParam);
        EvLButtonUp(wParam, pt);
    }
    return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            set_Transfer();
            EndDialog(hwnd, IDOK);
            return TRUE;
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        case IDC_PUSHBUTTON20:
            ResetAngles();
            return TRUE;
        case IDC_PUSHBUTTON25:
            LeftButton();
            return TRUE;
        case IDC_PUSHBUTTON26:
            RightButton();
            return TRUE;
        case IDC_PUSHBUTTON28:
            InButton();
            return TRUE;
        case IDC_PUSHBUTTON27:
            OutButton();
            return TRUE;
        case IDC_PUSHBUTTON21:
            PUp();
            return TRUE;
        case IDC_PUSHBUTTON22:
            PDown();
            return TRUE;
        case IDC_PUSHBUTTON23:
            RUp();
            return TRUE;
        case IDC_PUSHBUTTON24:
            RDown();
            return TRUE;
        case IDMA_CHECKBOX1:
        case IDC_CHECKBOX26:
            Preview();
            return TRUE;
        case IDMA_EDIT1:
        case IDMA_EDIT2:
            if(HIWORD(wParam)==EN_CHANGE)
            {
                Preview();
                return TRUE;
            }
            break;
        }
    }
    return FALSE;
}
//xfer -> controls
void TArchDlg::get_Transfer()
{
    if(!Stones || !Width || !XAxis || !RectTop)
        return;
    Stones->SetText(xferBuffer->Stones);
    Width->SetText(xferBuffer->width);
    XAxis->SetCheck(xferBuffer->XAxis);
    RectTop->SetCheck(xferBuffer->RectTop);
}
//controls -> xfer
void TArchDlg::set_Transfer()
{
    if(!Stones || !Width || !XAxis || !RectTop)
        return;
    Stones->GetText(xferBuffer->Stones);
    Width->GetText(xferBuffer->width);
    xferBuffer->XAxis = XAxis->GetCheck();
    xferBuffer->RectTop = RectTop->GetCheck();
}

void TArchDlg::SetupWindow()
{
    CenterWindow(hwnd);
    ConstructMemDC();

    Stones = new WEdit(hwnd, IDMA_EDIT1, xferBuffer->Stones, MAX_EDITLEN);
    Width = new WEdit(hwnd, IDMA_EDIT2, xferBuffer->width, MAX_EDITLEN);
    XAxis = new WCheck(hwnd, IDMA_CHECKBOX1, xferBuffer->XAxis);
    RectTop = new WCheck(hwnd, IDC_CHECKBOX26, xferBuffer->RectTop);

    get_Transfer();
}

void TArchDlg::EvLButtonDown(UINT modKeys, POINT& point)
{
    if (mode)							// already processing...
        return;
    if (!PtInRect(&PreviewRect,point))	// not close enough.
        return;

    SetCapture(hwnd);

    startPt = point;
    lastPt = point;
    if (modKeys & MK_SHIFT)
    {
        mode = DRAG_DISTANCE;
    }
    else
    {
        mode = DRAG_ROTATE;
    }
}

void TArchDlg::EvLButtonUp(UINT /*modKeys*/, POINT& /*point*/)
{
    if (!mode) // not processing...
        return;

    ReleaseCapture();
    mode = 0; // turn it off...
}

void TArchDlg::EvMouseMove(UINT /*modKeys*/, POINT& point)
{
    if (!mode) // just ignore...
        return;
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
};

TArchDlg::~TArchDlg()
{
    delete Stones;
    delete Width;
    delete XAxis;
    delete RectTop;

    if (fakeEnt)
    {
        fakeEnt->freeObjects(true);
        delete fakeEnt;
        fakeEnt = NULL;
    }
    RemoveMemDC();
}

void TArchDlg::Reconstruct()
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

    fakeBrush = m->selectedBrush()->copy();	// make a copy
    fakeBrush->setParent(fakeEnt);		// point to our fake entity...
    fakeBrush->calcWindings();			// so it is visible...
    fakeEnt->addObject(fakeBrush);		// put it in...

    int xory = xferBuffer->XAxis;
    int rect = xferBuffer->RectTop;
    int stones = max(3, atoi(xferBuffer->Stones));
    int hgap = max(1, atoi(xferBuffer->width));

    // make them...don't delete the "seed" brush...
    badbrush = m->makeArch(fakeEnt,fakeBrush,false,stones,xory,hgap,rect);
}

void TArchDlg::Redraw(HDC hdc)
{
    char outstr[80];
    int len;
    float NewQ[4][4];
    float T[4][4];
    float V[4][4];
    vec3_t vp;
    RECT TempRect;

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

    COLORREF oldColor = RGB(0,0,255);		// blue
    COLORREF newColor = RGB(255,0,0);		// red

    TempRect.left = 0;
    TempRect.top = 0;
    TempRect.right = PreviewSize.cx;
    TempRect.bottom = PreviewSize.cy;

    HPEN pen = CreatePen(PS_SOLID,3,RGB(192,192,192));
    HGDIOBJ oldpen = SelectObject(hdc,pen);

    HBRUSH whitebr = CreateSolidBrush(RGB(255,255,255));
    HGDIOBJ oldbr = SelectObject(hdc, whitebr);
    Rectangle(hdc,0,0,PreviewSize.cx,PreviewSize.cy);

    for (SetBrush *b = fakeEnt->objects.p_next; b != &fakeEnt->objects; b = b->p_next)
    {
        if (b == fakeBrush)
            continue;
        b->CameraDrawSelfSpecial(hdc,p,newColor,U,set.near_clip_distance,set.far_clip_distance, &TempRect, &PreviewSize);
    }

    // now draw the seed brush...
    fakeBrush->CameraDrawSelfSpecial(hdc,p,oldColor,U,set.near_clip_distance,set.far_clip_distance, &TempRect, &PreviewSize);

    HGDIOBJ oldfont = SelectObject(hdc,set.font9);

    SetTextColor(hdc, RGB(0,0,0));
    SetBkColor(hdc, RGB(255,255,255));

    len = sprintf(outstr,"R(%i) P(%i) Y(%i) ", RAD2DEG(v_roll), RAD2DEG(v_pitch), RAD2DEG(c_yaw));
    TextOut(hdc, TempRect.left+4,TempRect.top+2,outstr,len);

    if (badbrush)
    {
        char prompt[128];
        len = sprintf(prompt,"Brush is too short to create requested arch,");
        TextOut(hdc, TempRect.left+4, PreviewSize.cy/2, prompt, len);
        len = sprintf(prompt,"try resizing seed brush or, possibly, changing the axis...");
        TextOut(hdc, TempRect.left+4, PreviewSize.cy/2+15, prompt, len);
    }

    SelectObject(hdc,oldfont);
    SelectObject(hdc,oldbr);
    SelectObject(hdc,oldpen);
    DeleteObject(whitebr);
    DeleteObject(pen);
}

void TArchDlg::MemDraw()
{
    if (!hmemdc || !hmembitmap)
        return;

    Redraw(hmemdc);

    HDC hdc = GetDC(hwnd);
    BitBlt(hdc, PreviewRect.left,PreviewRect.top,PreviewRect.right-PreviewRect.left,PreviewRect.bottom-PreviewRect.top,
           hmemdc, 0,0, SRCCOPY);
    ReleaseDC(hwnd,hdc);
}

void TArchDlg::RemoveMemDC()
{
    if(hmemdc)
        DeleteDC(hmemdc);
    if(hmembitmap)
        DeleteObject(hmembitmap);
    hmemdc = 0;
    hmembitmap = 0;
}

void TArchDlg::ConstructMemDC()
{
    HDC hdc = GetDC(hwnd);

    hmemdc = CreateCompatibleDC(hdc);
    hmembitmap = CreateCompatibleBitmap(hdc, PreviewRect.right-PreviewRect.left, PreviewRect.bottom-PreviewRect.top);

    SelectObject(hmemdc, hmembitmap);

    ReleaseDC(hwnd, hdc);
}

void TArchDlg::EvPaint()
{
    MemDraw();
}

void TArchDlg::ResetAngles()
{
    c_yaw   = M_PI;
    v_roll  = 1.5f*M_PI;
    v_pitch = 0.0;
    s       = 0.0;

    MemDraw();
}

void TArchDlg::Preview()
{
    if (!hmemdc || !hmembitmap)
        return;

    set_Transfer();

    Reconstruct();
    MemDraw();
}

void TArchDlg::PUp()
{
    v_pitch += 2.0f*M_PI/STEPS;
    if (v_pitch>= 2*M_PI)
        v_pitch -= 2*M_PI;

    MemDraw();
}

void TArchDlg::PDown()
{
    v_pitch -= 2.0f*M_PI/STEPS;
    if (v_pitch <= 0.0)
        v_pitch += 2*M_PI;

    MemDraw();
}

void TArchDlg::RUp()
{
    v_roll += 2.0f*M_PI/STEPS;
    if (v_roll>= 2*M_PI)
        v_roll -= 2*M_PI;

    MemDraw();
}
void TArchDlg::RDown()
{
    v_roll -= 2.0f*M_PI/STEPS;
    if (v_roll <= 0.0)
        v_roll += 2*M_PI;

    MemDraw();
}
void TArchDlg::LeftButton()
{
    c_yaw -= 2.0f*M_PI/STEPS;
    if (c_yaw <= 0.0)
        c_yaw += 2*M_PI;

    MemDraw();
}
void TArchDlg::RightButton()
{
    c_yaw += 2.0f*M_PI/STEPS;
    if (c_yaw>= 2*M_PI)
        c_yaw -= 2*M_PI;

    MemDraw();
}
void TArchDlg::InButton()
{
    s += dist/10.0f;
    MemDraw();
}
void TArchDlg::OutButton()
{
    s -= dist/10.0f;
    MemDraw();
}


// ***********************************************************************************
// Light Dialog Box
TLightDlg::TLightDlg(HWND parent, TLightTransferBuffer *xfer)
    : WDialog(parent, LIGHT_DIALOG)
{
    this->xfer = xfer;
}
TLightDlg::~TLightDlg()
{
    delete NumX;
    delete NumY;
    delete NumZ;
    delete LightVal;
    delete LightEnts;
}
INT_PTR TLightDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        CenterWindow(hwnd);

        NumX = new WEdit(hwnd, IDLA_EDIT1, xfer->NumX, MAX_EDITLEN);
        NumY = new WEdit(hwnd, IDLA_EDIT2, xfer->NumY, MAX_EDITLEN);
        NumZ = new WEdit(hwnd, IDLA_EDIT3, xfer->NumZ, MAX_EDITLEN);
        LightVal = new WEdit(hwnd, IDLA_EDIT4, xfer->LightVal, MAX_EDITLEN);
        LightEnts = new WCombo(hwnd, IDC_COMBOBOX2, MAX_KEY);
        xfer->boxdata.FillCombo(LightEnts->hwnd);

        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            NumX->GetText(xfer->NumX);
            NumY->GetText(xfer->NumY);
            NumZ->GetText(xfer->NumZ);
            LightVal->GetText(xfer->LightVal);
            LightEnts->GetText(xfer->selected);
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
TRotateDlg::TRotateDlg(HWND parent, TRotateTransferBuffer *xfer)
    : WDialog(parent, ROTATE_DIALOG)
{
    this->xfer = xfer;
}
TRotateDlg::~TRotateDlg()
{
    delete Roll;
    delete Pitch;
    delete Yaw;
}
INT_PTR TRotateDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        CenterWindow(hwnd);

        Roll = new WEdit(hwnd, IDR_EDIT1, xfer->Roll, MAX_EDITLEN);
        Pitch = new WEdit(hwnd, IDR_EDIT2, xfer->Pitch, MAX_EDITLEN);
        Yaw = new WEdit(hwnd, IDR_EDIT3, xfer->Yaw, MAX_EDITLEN);

        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            Roll->GetText(xfer->Roll);
            Pitch->GetText(xfer->Pitch);
            Yaw->GetText(xfer->Yaw);

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
TCopyDlg::TCopyDlg(HWND parent, TCopyTransferBuffer *xfer)
    : WDialog(parent, COPY_DIALOG)
{
    this->xfer = xfer;
}
TCopyDlg::~TCopyDlg()
{
    delete NumX;
    delete NumY;
    delete NumZ;
    delete NumCopies;
    delete MoveOnly;
}

INT_PTR TCopyDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        CenterWindow(hwnd);

        NumX = new WEdit(hwnd, IDCM_EDIT1, xfer->NumX, MAX_EDITLEN);
        NumY = new WEdit(hwnd, IDCM_EDIT2, xfer->NumY, MAX_EDITLEN);
        NumZ = new WEdit(hwnd, IDCM_EDIT3, xfer->NumZ, MAX_EDITLEN);
        NumCopies = new WEdit(hwnd, IDC_EDIT28, xfer->NumCopies, MAX_EDITLEN);
        MoveOnly = new WCheck(hwnd, IDCM_CHECK1, xfer->MoveOnly);

        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            NumX->GetText(xfer->NumX);
            NumY->GetText(xfer->NumY);
            NumZ->GetText(xfer->NumZ);
            NumCopies->GetText(xfer->NumCopies);
            xfer->MoveOnly = MoveOnly->GetCheck();

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
// Sphere Dialog Box
TSphereDlg::TSphereDlg(HWND parent, TSphereTransferBuffer *xfer)
    : WDialog(parent, SPHERE_DIALOG)
{
    this->xfer = xfer;
}
TSphereDlg::~TSphereDlg()
{
    delete HStrips;
    delete VStrips;
    delete Inner;
    delete Outer;
    delete InnerPercent;
    delete TakeSeed;
}

INT_PTR TSphereDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        CenterWindow(hwnd);

        HStrips = new WEdit(hwnd, IDS_EDIT1, xfer->HStrips, MAX_EDITLEN);
        VStrips = new WEdit(hwnd, IDS_EDIT2, xfer->VStrips, MAX_EDITLEN);
        Inner = new WEdit(hwnd, IDS_EDIT3, xfer->Inner, MAX_EDITLEN);
        Outer = new WEdit(hwnd, IDS_EDIT4, xfer->Outer, MAX_EDITLEN);
        InnerPercent = new WEdit(hwnd, IDC_EDIT8, xfer->InnerPercent, MAX_EDITLEN);
        TakeSeed = new WCheck(hwnd, IDC_CHECKBOX26, xfer->TakeSeed);

        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            HStrips->GetText(xfer->HStrips);
            VStrips->GetText(xfer->VStrips);
            Inner->GetText(xfer->Inner);
            Outer->GetText(xfer->Outer);
            InnerPercent->GetText(xfer->InnerPercent);
            xfer->TakeSeed = TakeSeed->GetCheck();

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
// Keypair Dialog Box
TKPDlg::TKPDlg(HWND parent, TKPTransferBuffer *xfer, int focusFld)
    : WDialog(parent, IDD_KPENTRY)
{
    this->xfer = xfer;
    focus = focusFld;
}
TKPDlg::~TKPDlg()
{
    delete editkey;
    delete editvalue;
    delete global;
}
INT_PTR TKPDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        CenterWindow(hwnd);

        editkey = new WEdit(hwnd, IDC_EDIT47, xfer->Key, MAX_KEY);
        editvalue = new WEdit(hwnd, IDC_EDIT48, xfer->Value, MAX_VALUE);
        global = new WCheck(hwnd, IDC_CHECKBOX26, xfer->Global);

        SetFocus(focus ? editvalue->hwnd : editkey->hwnd);

        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            editkey->GetText(xfer->Key);
            editvalue->GetText(xfer->Value);
            xfer->Global = global->GetCheck();

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
// Sphere Dialog Box
TCylDlg::TCylDlg(HWND parent, TCylTransferBuffer *xfer)
    : WDialog(parent, CYL_DIALOG)
{
    this->xfer = xfer;
}
TCylDlg::~TCylDlg()
{
    delete Sides;
    delete Strips;
    delete Inner;
    delete Outer;
    delete InnerPercent;
    delete TakeSeed;
    delete XAlign;
}
INT_PTR TCylDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        CenterWindow(hwnd);

        Sides = new WEdit(hwnd, IDC_EDIT11, xfer->Sides, MAX_EDITLEN);
        Strips = new WEdit(hwnd, IDC_EDIT12, xfer->Strips, MAX_EDITLEN);
        Inner = new WEdit(hwnd, IDC_EDIT13, xfer->Inner, MAX_EDITLEN);
        Outer = new WEdit(hwnd, IDC_EDIT14, xfer->Outer, MAX_EDITLEN);
        InnerPercent = new WEdit(hwnd, IDC_EDIT28, xfer->InnerPercent, MAX_EDITLEN);
        TakeSeed = new WCheck(hwnd, IDC_CHECKBOX25, xfer->TakeSeed);
        XAlign = new WCheck(hwnd, IDC_CHECKBOX30, xfer->XAlign);

        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            Sides->GetText(xfer->Sides);
            Strips->GetText(xfer->Strips);
            Inner->GetText(xfer->Inner);
            Outer->GetText(xfer->Outer);
            InnerPercent->GetText(xfer->InnerPercent);
            xfer->TakeSeed = TakeSeed->GetCheck();
            xfer->XAlign = XAlign->GetCheck();

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
TNDlg::TNDlg(HWND parent, TNTransferBuffer *xfer)
    : WDialog(parent, N_DIALOG)
{
    this->xfer = xfer;
}
TNDlg::~TNDlg()
{
    delete Sides;
    delete Radius;
    delete XAlign;
    delete TakeSeed;
}
INT_PTR TNDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        CenterWindow(hwnd);
        Sides = new WEdit(hwnd, IDC_EDIT15, xfer->Sides, MAX_EDITLEN);
        Radius = new WEdit(hwnd, IDC_EDIT16, xfer->Radius, MAX_EDITLEN);
        XAlign = new WCheck(hwnd, IDC_CHECKBOX19, xfer->XAlign);
        TakeSeed = new WCheck(hwnd, IDC_CHECKBOX20, xfer->TakeSeed);
        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            Sides->GetText(xfer->Sides);
            Radius->GetText(xfer->Radius);
            xfer->XAlign = XAlign->GetCheck();
            xfer->TakeSeed = TakeSeed->GetCheck();
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
TScaleDlg::TScaleDlg(HWND parent, TScaleTransferBuffer *xfer)
    : WDialog(parent, SCALE_DIALOG)
{
    this->xfer = xfer;
}
TScaleDlg::~TScaleDlg()
{
    delete ScaleX;
    delete ScaleY;
    delete ScaleZ;
}
INT_PTR TScaleDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        CenterWindow(hwnd);
        ScaleX = new WEdit(hwnd, IDC_EDIT20, xfer->ScaleX, MAX_EDITLEN);
        ScaleY = new WEdit(hwnd, IDC_EDIT21, xfer->ScaleY, MAX_EDITLEN);
        ScaleZ = new WEdit(hwnd, IDC_EDIT22, xfer->ScaleZ, MAX_EDITLEN);
        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            ScaleX->GetText(xfer->ScaleX);
            ScaleY->GetText(xfer->ScaleY);
            ScaleZ->GetText(xfer->ScaleZ);
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
TMergeDlg::TMergeDlg(HWND parent, TMergeTransferBuffer *xfer)
    : WDialog(parent, IDD_MERGE)
{
    this->xfer = xfer;
}
TMergeDlg::~TMergeDlg()
{
    delete SaveFirst;
    delete SelectedOnly;
    delete TextureCompare;
    delete MatchingFlags;
    delete Face;
    delete Normal;
    delete Point;
    delete Convexity;
}
INT_PTR TMergeDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        CenterWindow(hwnd);

        SaveFirst = new WCheck(hwnd, IDC_CHECKBOX26, xfer->SaveFirst);
        SelectedOnly = new WCheck(hwnd, IDC_CHECKBOX29, xfer->SelectedOnly);
        TextureCompare = new WCheck(hwnd, IDC_CHECKBOX19, xfer->TextureCompare);
        MatchingFlags = new WCheck(hwnd, IDC_CHECK1, xfer->MatchingFlags);

        Face = new WEdit(hwnd, IDC_EDIT27, xfer->Face, 20);
        Normal = new WEdit(hwnd, IDC_EDIT28, xfer->Normal, 20);
        Point = new WEdit(hwnd, IDC_EDIT50, xfer->Point, 20);
        Convexity = new WEdit(hwnd, IDC_EDIT9, xfer->Convexity, 20);

        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            xfer->SaveFirst = SaveFirst->GetCheck();
            xfer->SelectedOnly = SelectedOnly->GetCheck();
            xfer->TextureCompare = TextureCompare->GetCheck();
            xfer->MatchingFlags = MatchingFlags->GetCheck();

            Face->GetText(xfer->Face);
            Normal->GetText(xfer->Normal);
            Face->GetText(xfer->Face);
            Convexity->GetText(xfer->Convexity);

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
TSelectDlg::TSelectDlg(HWND parent, TSelectTransferBuffer *xfer)
    : WDialog(parent, IDD_SELECT)
{
    this->xfer = xfer;
}
TSelectDlg::~TSelectDlg()
{
    delete Entity;
    delete Key;
    delete Value;
    delete SelectEntity;
    delete SelectKey;
    delete SelectValue;
}
INT_PTR TSelectDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        CenterWindow(hwnd);
        Entity = new WCombo(hwnd, IDC_COMBOBOX3, MAX_KEY);
        xfer->Entity.FillCombo(Entity->hwnd);

        Key = new WEdit(hwnd, IDC_EDIT8, xfer->Key, MAX_KEY);
        Value = new WEdit(hwnd, IDC_EDIT13, xfer->Value, MAX_VALUE);
        SelectEntity = new WCheck(hwnd, IDC_CHECKBOX20, xfer->SelectEntity);
        SelectKey = new WCheck(hwnd, IDC_CHECKBOX25, xfer->SelectKey);
        SelectValue = new WCheck(hwnd, IDC_CHECKBOX3, xfer->SelectValue);

        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            Entity->GetText(xfer->selected);
            Key->GetText(xfer->Key);
            Value->GetText(xfer->Value);
            xfer->SelectEntity = SelectEntity->GetCheck();
            xfer->SelectKey = SelectKey->GetCheck();
            xfer->SelectValue = SelectValue->GetCheck();
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
TPyramidDlg::TPyramidDlg(HWND parent, TPyramidTransferBuffer *xfer)
    : WDialog(parent, PYRAMID_DIALOG)
{
    this->xfer = xfer;
}
TPyramidDlg::~TPyramidDlg()
{
    delete Sides;
    delete Radius;
    delete Height;
    delete XAlign;
    delete TakeSeed;
}
INT_PTR TPyramidDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        CenterWindow(hwnd);

        Sides = new WEdit(hwnd, IDC_EDIT17, xfer->Sides, MAX_EDITLEN);
        Radius = new WEdit(hwnd, IDC_EDIT18, xfer->Radius, MAX_EDITLEN);
        Height = new WEdit(hwnd, IDC_EDIT19, xfer->Height, MAX_EDITLEN);
        XAlign = new WCheck(hwnd, IDC_CHECKBOX20, xfer->XAlign);
        TakeSeed = new WCheck(hwnd, IDC_CHECKBOX25, xfer->TakeSeed);

        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            Sides->GetText(xfer->Sides);
            Radius->GetText(xfer->Radius);
            Height->GetText(xfer->Height);
            xfer->XAlign = XAlign->GetCheck();
            xfer->TakeSeed = TakeSeed->GetCheck();

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
TCreateDlg::TCreateDlg(HWND parent, TCreateTransferBuffer *xfer)
    : WDialog(parent, CREATE_BRUSH)
{
    this->xfer = xfer;
}
TCreateDlg::~TCreateDlg()
{
    delete X;
    delete Y;
    delete Z;
    delete DX;
    delete DY;
    delete DZ;
}
INT_PTR TCreateDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        CenterWindow(hwnd);

        X = new WEdit(hwnd, IDC_EDIT23, xfer->X, MAX_EDITLEN);
        Y = new WEdit(hwnd, IDC_EDIT24, xfer->Y, MAX_EDITLEN);
        Z = new WEdit(hwnd, IDC_EDIT25, xfer->Z, MAX_EDITLEN);
        DX = new WEdit(hwnd, IDC_EDIT26, xfer->DX, MAX_EDITLEN);
        DY = new WEdit(hwnd, IDC_EDIT27, xfer->DY, MAX_EDITLEN);
        DZ = new WEdit(hwnd, IDC_EDIT28, xfer->DZ, MAX_EDITLEN);

        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            X->GetText(xfer->X);
            Y->GetText(xfer->Y);
            Z->GetText(xfer->Z);
            DX->GetText(xfer->DX);
            DY->GetText(xfer->DY);
            DZ->GetText(xfer->DZ);
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
TSetHeightsDlg::TSetHeightsDlg(HWND parent, TSetHeightsTransferBuffer *xfer)
    : WDialog(parent, IDD_SETBASE)
{
    this->xfer = xfer;
}
TSetHeightsDlg::~TSetHeightsDlg()
{
    delete Base;
    delete Height;
}
INT_PTR TSetHeightsDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        CenterWindow(hwnd);
        Base = new WEdit(hwnd, IDC_EDIT11, xfer->Base, MAX_EDITLEN);
        Height = new WEdit(hwnd, IDC_EDIT8, xfer->Base, MAX_EDITLEN);
        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            Base->GetText(xfer->Base);
            Height->GetText(xfer->Height);
            EndDialog(hwnd, IDOK);
            return TRUE;
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
    }
    return FALSE;
}

/////////////////////  WEDGE
// ***********************************************************************************
//
// Wedge Dialog Box
//
// ***********************************************************************************

INT_PTR TWedgeDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        SetupWindow();
        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            Axis->GetText(xferBuffer->Axis);
            xferBuffer->flip = flip->GetCheck();
            EndDialog(hwnd, IDOK);
            return TRUE;
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        case IDC_PUSHBUTTON20:
            ResetAngles();
            return TRUE;
        case IDC_PUSHBUTTON25:
            LeftButton();
            return TRUE;
        case IDC_PUSHBUTTON26:
            RightButton();
            return TRUE;
        case IDC_PUSHBUTTON28:
            InButton();
            return TRUE;
        case IDC_PUSHBUTTON27:
            OutButton();
            return TRUE;
        case IDC_PUSHBUTTON21:
            PUp();
            return TRUE;
        case IDC_PUSHBUTTON22:
            PDown();
            return TRUE;
        case IDC_PUSHBUTTON23:
            RUp();
            return TRUE;
        case IDC_PUSHBUTTON24:
            RDown();
            return TRUE;
        case IDMW_EDIT1:
        case IDMW_CHECKBOX1:
            Preview();
            return TRUE;
        }
        return FALSE;
    case WM_MOUSEMOVE:
    {
        MAKEPOINT(pt, lParam);
        EvMouseMove(wParam,&pt);
    }
    return TRUE;
    case WM_LBUTTONUP:
        EvLButtonUp(0,0);
        return TRUE;
    case WM_LBUTTONDOWN:
    {
        MAKEPOINT(pt, lParam);
        EvLButtonDown(wParam,&pt);
    }
    return TRUE;
    case WM_PAINT:
        EvPaint();
        return FALSE;
    case WM_KEYDOWN:
        if(wParam == VK_ESCAPE)
            EndDialog(hwnd, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}

TWedgeDlg::TWedgeDlg(HWND parent, TWedgeTransferBuffer *xfer)
    : WDialog(parent, WEDGE_DIALOG)
{
    xferBuffer = xfer;
    badbrush = 0;

    hmemdc = 0;
    hmembitmap = 0;

    mode = 0;

    PreviewRect.left   = WEDGE_PREV_LEFT;
    PreviewRect.right  = PreviewRect.left + WEDGE_W;
    PreviewRect.top    = WEDGE_PREV_TOP;
    PreviewRect.bottom = PreviewRect.top + WEDGE_H;

    PreviewSize.cx = PreviewRect.right-PreviewRect.left;
    PreviewSize.cy = PreviewRect.bottom-PreviewRect.top;

    c_yaw   = M_PI;
    v_roll  = 1.5f*M_PI;
    v_pitch = 0.0f;
    dist    = 1.0f;
    width   = 1.0f;
    height  = 1.0f;
    p       = 1.0f;
    s		= 0.0f;

    fakeEnt = new Entity;

    fakeBrush = map_i[set.curmap]->selectedBrush()->copy(); // make a copy
    fakeBrush->setParent(fakeEnt);	// point to our fake entity...
    fakeBrush->calcWindings();		// so it is visible...
    fakeEnt->addObject(fakeBrush);

    SetBrush *b = fakeBrush;

    height = (b->bmaxs[2]-b->bmins[2]) / 2.0f;
    width  = max(b->bmaxs[1]-b->bmins[1],b->bmaxs[0]-b->bmins[0]) / 2.0f;

    p = set.ppd*set.mag; // from main program...

    // figure the larger way...
    float dist1 = (height*p / (PreviewSize.cy/2.0f));
    float dist2 = (width *p / (PreviewSize.cx/2.0f));
    dist = max(dist1,dist2);

    ctr[0] = (b->bmins[0] + b->bmaxs[0]) / 2.0f;
    ctr[1] = (b->bmins[1] + b->bmaxs[1]) / 2.0f;
    ctr[2] = (b->bmins[2] + b->bmaxs[2]) / 2.0f;
}

TWedgeDlg::~TWedgeDlg()
{
    if (fakeEnt)
    {
        fakeEnt->freeObjects(true);
        delete fakeEnt;
        fakeEnt = NULL;
    }
    delete Axis;
    delete flip;
    RemoveMemDC();
}

void TWedgeDlg::SetupWindow()
{
    CenterWindow(hwnd);

    Axis = new WEdit(hwnd, IDMW_EDIT1, xferBuffer->Axis, MAX_EDITLEN);
    flip = new WCheck(hwnd, IDMW_CHECKBOX1, xferBuffer->flip);

    ConstructMemDC();

    Reconstruct();
}

void TWedgeDlg::EvLButtonDown(UINT modKeys, LPPOINT point)
{
    if (mode)  // already processing...
        return;
    if (!PtInRect(&PreviewRect,*point)) // not close enough.
        return;

    SetCapture(hwnd);

    startPt = *point;
    lastPt = *point;

    mode = (modKeys & MK_SHIFT) ? DRAG_DISTANCE : DRAG_ROTATE;
}

void TWedgeDlg::EvLButtonUp(UINT /*modKeys*/, LPPOINT /*point*/)
{
    if (!mode) // not processing...
        return;

    ReleaseCapture();
    mode = 0; // turn it off...
}

void TWedgeDlg::EvMouseMove(UINT /*modKeys*/, LPPOINT point)
{
    if (!mode) // just ignore...
        return;
    // process...
    if (point->x == lastPt.x && point->y == lastPt.y)
        return;  // no move...

    int dH1 =   DRAG_DELTA/3;
    int dH2 = 2*DRAG_DELTA/3;

    int dx = point->x - lastPt.x;
    int dy = point->y - lastPt.y;

    if ((abs(dx) < DRAG_DELTA) && (abs(dy) < DRAG_DELTA))
        return;

    lastPt = *point;

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

void TWedgeDlg::Reconstruct()
{
    map *m = map_i[set.curmap];

    Axis->GetText(xferBuffer->Axis);
    xferBuffer->flip = flip->GetCheck();

    if (fakeBrush && fakeEnt)
        fakeEnt->removeObject(fakeBrush);

    if (fakeEnt)   // if stuff is left over, kill it...
    {
        fakeEnt->freeObjects(true);
        delete fakeEnt;
    }
    fakeEnt = new Entity;

    delete fakeBrush;
    fakeBrush = m->selectedBrush()->copy(); // make a copy
    fakeBrush->setParent(fakeEnt); // point to our fake entity...
    fakeBrush->calcWindings(); // so it is visible...
    fakeEnt->addObject(fakeBrush); // put it in...

    int axis = atoi(xferBuffer->Axis);
    axis = min(axis,2);
    axis = max(axis,0);

    // make them...don't delete the "seed" brush...
    badbrush = m->makeWedge(fakeEnt,fakeBrush,false,axis,xferBuffer->flip);
}

void TWedgeDlg::Redraw(HDC hdc)
{
    char outstr[80];
    float NewQ[4][4];
    float T[4][4];
    float V[4][4];
    vec3_t vp;
    SetBrush *b;
    RECT TempRect;

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

    COLORREF oldColor = RGB(0,0,255);
    COLORREF newColor = RGB(255,0,0);

    TempRect.left = 0;
    TempRect.right = PreviewSize.cx;
    TempRect.top = 0;
    TempRect.bottom = PreviewSize.cy;

    HPEN pen = CreatePen(PS_SOLID,3,RGB(179,179,179));
    HGDIOBJ oldpen = SelectObject(hdc, pen);
    HBRUSH brush = CreateSolidBrush(RGB(255,255,255));
    HGDIOBJ oldbrush = SelectObject(hdc, brush);
    Rectangle(hdc,TempRect.left,TempRect.top,TempRect.right,TempRect.bottom);

    //  draw the seed brush first...
    fakeBrush->CameraDrawSelfSpecial(hdc,p,oldColor,U,set.near_clip_distance,set.far_clip_distance, &TempRect, &PreviewSize);

    for (b = fakeEnt->objects.p_next; b != &fakeEnt->objects; b = b->p_next)
    {
        if (b == fakeBrush)
            continue;

        b->CameraDrawSelfSpecial(hdc,p,newColor,U,set.near_clip_distance,set.far_clip_distance, &TempRect, &PreviewSize);
    }

    HGDIOBJ oldfont = SelectObject(hdc, set.font9);	//font was +14
    SetTextColor(hdc, RGB(0,0,0));
    SetBkColor(hdc, RGB(255,255,255));
    int len = sprintf(outstr,"R(%i) P(%i) Y(%i) ", RAD2DEG(v_roll), RAD2DEG(v_pitch), RAD2DEG(c_yaw));
    TextOut(hdc,TempRect.left+4,TempRect.top+2,outstr,len);

    if (badbrush)
    {
        char *prompt = const_cast<char *> ("Couldn't create wedge... try changing settings...");
        TextOut(hdc, TempRect.left+4,PreviewSize.cy/2,prompt,strlen(prompt));
    }
    SelectObject(hdc, oldpen);
    SelectObject(hdc, oldbrush);
    SelectObject(hdc, oldfont);
    DeleteObject(pen);
    DeleteObject(brush);
}

void TWedgeDlg::MemDraw()
{
    if (!hmemdc || !hmembitmap)
        return;

    HDC hdc = GetDC(hwnd);
    Redraw(hmemdc);
    BitBlt(hdc, PreviewRect.left,PreviewRect.top,PreviewRect.right-PreviewRect.left,PreviewRect.bottom-PreviewRect.top, hmemdc, 0,0, SRCCOPY);
    ReleaseDC(hwnd, hdc);
}

void TWedgeDlg::RemoveMemDC()
{
    if (hmemdc)
        DeleteDC(hmemdc);
    if (hmembitmap)
        DeleteObject(hmembitmap);
    hmemdc = 0;
    hmembitmap = 0;
}
void TWedgeDlg::ConstructMemDC()
{
    HDC hdc = GetDC(hwnd);
    hmemdc     = CreateCompatibleDC(hdc);
    hmembitmap = CreateCompatibleBitmap(hdc, PreviewRect.right-PreviewRect.left, PreviewRect.bottom-PreviewRect.top);
    SelectObject(hmemdc,hmembitmap); // install bitmap into memDC
    ReleaseDC(hwnd,hdc);
}
void TWedgeDlg::EvPaint()
{
    MemDraw();
}
void TWedgeDlg::ResetAngles()
{
    c_yaw   = M_PI;
    v_roll  = 1.5f*M_PI;
    v_pitch = 0.0f;
    s       = 0.0f;

    MemDraw();
}

void TWedgeDlg::Preview()
{
    if (!hmemdc || !hmembitmap)
        return;
    Reconstruct();
    MemDraw();
}
void TWedgeDlg::PUp()
{
    v_pitch += 2.0f*M_PI/STEPS;
    if (v_pitch>= 2*M_PI)
        v_pitch -= 2*M_PI;
    MemDraw();
}
void TWedgeDlg::PDown()
{
    v_pitch -= 2.0f*M_PI/STEPS;
    if (v_pitch <= 0.0)
        v_pitch += 2*M_PI;
    MemDraw();
}
void TWedgeDlg::RUp()
{
    v_roll += 2.0f*M_PI/STEPS;
    if (v_roll>= 2*M_PI)
        v_roll -= 2*M_PI;
    MemDraw();
}
void TWedgeDlg::RDown()
{
    v_roll -= 2.0f*M_PI/STEPS;
    if (v_roll <= 0.0)
        v_roll += 2*M_PI;
    MemDraw();
}
void TWedgeDlg::LeftButton()
{
    c_yaw -= 2.0f*M_PI/STEPS;
    if (c_yaw <= 0.0)
        c_yaw += 2*M_PI;
    MemDraw();
}
void TWedgeDlg::RightButton()
{
    c_yaw += 2.0f*M_PI/STEPS;
    if (c_yaw>= 2*M_PI)
        c_yaw -= 2*M_PI;
    MemDraw();
}
void TWedgeDlg::InButton()
{
    s += dist/10.0f;
    MemDraw();
}
void TWedgeDlg::OutButton()
{
    s -= dist/10.0f;
    MemDraw();
}


// ***********************************************************************************
//
// Texture Find/Replace Dialog
//
FindReplaceDlg::FindReplaceDlg(HWND parent, char *findwhat, char *replacewith, FR_CALLBACK findrep)
    : WDialog(parent, IDD_FINDREPLACE)
{
    hwnd = CreateModeless();
    ShowWindow(hwnd, SW_SHOW);
    this->findwhat = new WEdit(hwnd, IDC_EDIT1, findwhat, LUMP_NAME_LENGTH_MAX);
    this->replacewith = new WEdit(hwnd, IDC_EDIT53, replacewith, LUMP_NAME_LENGTH_MAX);
    this->Callback = findrep;
}
FindReplaceDlg::~FindReplaceDlg()
{
    delete findwhat;
    delete replacewith;
}
INT_PTR FindReplaceDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
        {
            char fw[LUMP_NAME_LENGTH_MAX], rw[LUMP_NAME_LENGTH_MAX];
            findwhat->GetText(fw);
            replacewith->GetText(rw);
            if(!Callback(true, fw, rw)) break;
            EndDialog(hwnd, IDOK);
            return TRUE;
        }
        case IDCANCEL:
            if(!Callback(false, const_cast<char *> (""), const_cast<char *> (""))) break;
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
    }
    return FALSE;
}



// ***********************************************************************************
//
// Keyboard Settings Dialog
//
KeyboardSettingsDlg::KeyboardSettingsDlg(HWND parent)
    : WDialog(parent, IDD_KEYBOARD), shortcut(0)
{}

ACCEL key; //keycombo_t key;

void UpdateKeycombo(HWND edit)
{

    if(!key.key)
        return;
    key.fVirt &= ~(FCONTROL|FALT|FSHIFT);
    //control
    if(GetKeyState(VK_CONTROL) < 0)
        key.fVirt |= FCONTROL;
    //alt
    if(GetKeyState(VK_MENU) < 0)
        key.fVirt |= FALT;
    //shift
    if(GetKeyState(VK_SHIFT) < 0)
        key.fVirt |= FSHIFT;

    char buf[128];
    KBSettings::AccelToText(key, buf);
    SetWindowText(edit,buf);

    //handle inside scope of KBSettings dialog
    SendMessage(GetParent(edit),WM_SHORTCUT_CHANGED,0,0);
}

WNDPROC KBOldShortcutWndProc;
LRESULT KBShortcutWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_GETDLGCODE:
        return DLGC_WANTTAB | DLGC_WANTARROWS | DLGC_WANTMESSAGE | DLGC_WANTALLKEYS;
    case WM_CHAR:
        return 0;
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
        if(wParam!=VK_SHIFT && wParam!=VK_CONTROL && wParam!=VK_MENU)
        {
            UINT result = (int)(short)MapVirtualKey(wParam, 2);	//hackity hacks?
            switch(result)
            {
                //these characters need to be treated as ascii for backwards compat
            case '[':
            case ']':
            case '\\':
            case '`':
            case '/':
            case ';':
                key.key = result;
                key.fVirt = FNOINVERT;
                break;
                //treat rest as virtual
            default:
                key.key = wParam;
                key.fVirt = FNOINVERT|FVIRTKEY;
            }
        }
        UpdateKeycombo(hwnd);
        return 0;

    case WM_SYSCOMMAND:
        UpdateKeycombo(hwnd);
        return 0;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
        SetFocus(hwnd);
        return 0;
    }
    return CallWindowProc(KBOldShortcutWndProc, hwnd, msg, wParam, lParam);
}
void KeyboardSettingsDlg::SetDescription(ccmd_t *c)
{

    if(c <= 0 || c->id <= 0)
    {
        descript->SetText(const_cast<char *> (""));
        return;
    }

    char boundkeys[128], outstr[256];
    kbset_temp->ShowAccelsForCommand(c->id,boundkeys,sizeof(boundkeys));	// find other binds for cmd

    if(*boundkeys)
    {
        snprintf(outstr,sizeof(outstr),"%s\nKeys: %s",c->info_text,boundkeys);	// append bind info
        descript->SetText(outstr);
    }
    else
    {
        descript->SetText(c->info_text);							// just display cmd desc
    }
}
INT_PTR KeyboardSettingsDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch(msg)
    {
    case WM_INITDIALOG:
    {
        CenterWindow(hwnd);

        //work on a copy of kbset so it can be canceled
        kbset_temp = new KBSettings(*kbset);

        shortcut = new WEdit(hwnd,IDC_EDIT1);
        save = new WButton(hwnd,IDC_BUTTON2);
        showkeys = new WButton(hwnd,IDC_BUTTON4);
        restorekeys = new WButton(hwnd,IDC_BUTTON3);
        descript = new WStatic(hwnd,IDC_STATIC1);

        commands = new WCombo(hwnd,IDC_COMBO1);
        Ccmd_FillCombo(commands);

        commands->InsertString(const_cast<char *> ("None"),0);
        commands->SetSelIndex(0);

#ifdef _WIN64
        KBOldShortcutWndProc = (WNDPROC) SetWindowLong(shortcut->hwnd, GWLP_WNDPROC, (LONG_PTR)KBShortcutWndProc);
#else
        KBOldShortcutWndProc = (WNDPROC) SetWindowLong(shortcut->hwnd, GWL_WNDPROC, (LONG)KBShortcutWndProc);
#endif
        break;
    }
    //shortcut changed
    case WM_SHORTCUT_CHANGED:
    {
        ACCEL *ac = kbset_temp->FindAccel(key,true,false);
        if(ac)
        {
            ccmd_t *c = Ccmd_FindCmd(ac->cmd);
            SetDescription(c);
            if(c)
            {
                int idx = commands->FindStringExact(c->name);
                if(idx >= 0) commands->SetSelIndex(idx);
                //broken : "CM_F" would find "CM_FORWARD" first.. commands->SetSelString(c->name,0);
            }
        }
        else
        {
            descript->SetText(const_cast<char *> (""));		//
            commands->SetSelIndex(0);	//select "None"
        }
    }
    return TRUE;
    case WM_CLOSE:
        EndDialog(hwnd,IDCANCEL);
        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_COMBO1:
            if(HIWORD(wParam)==CBN_SELCHANGE)
            {

                char cmdstr[128];
                commands->GetString(cmdstr,commands->GetSelIndex());
                ccmd_t *c = Ccmd_FindCmd(cmdstr);
                SetDescription(c);
            }
            return TRUE;
        case IDC_BUTTON1:		//set key
            if(key.key)
            {
                int idx = commands->GetSelIndex();
                char buf[128];
                commands->GetString(buf,idx);		// get selected command from list
                ccmd_t *c = Ccmd_FindCmd(buf);
                int cmd = Ccmd_GetId(buf);
                if(c)
                    cmd = c->id;

                ACCEL k;
                k.key = key.key;
                k.fVirt = key.fVirt;
                k.cmd = cmd;
                if(!kbset_temp->AddReplaceAccel(&k))
                    Msgbox(const_cast<char *> ("Command '%s' not found!"),buf);	// uhh weird... !

                shortcut->SetText("");
                SetDescription(c);
            }
            return TRUE;
            //save to disk
        case IDC_BUTTON2:

            if(IDYES==MessageBox(client->hwnd,"Save keyboard settings to disk? Current changes "
                                 "to key settings will be applied first.","Confirm Save",MB_YESNO))
            {
                //commit changes
                kbset_temp->OverwriteAndActivate(kbset);

                kbset->SaveToDisk();
            }
            return TRUE;
            //load from disk
        case IDC_BUTTON3:
            kbset_temp->ClearKeys();
            kbset_temp->LoadFromDisk(const_cast<char *> ("settings\\keyboard.cfg"));
            key.key = 0;
            shortcut->SetText("");
            Msgbox(const_cast<char *> ("Keyboard configuration reloaded from disk"));
            return TRUE;
        case IDOK:
            if(GetFocus()==shortcut->hwnd)
                shortcut->SendMessage(WM_KEYDOWN,VK_RETURN,0);	//fake VK_RETURN
            return TRUE;
        case IDCANCEL:
            if(GetFocus()==shortcut->hwnd)
                shortcut->SendMessage(WM_KEYDOWN,VK_ESCAPE,0);	//fake VK_ESCAPE
            return TRUE;
        case ID_KBOK:

            //commit changes
            kbset_temp->OverwriteAndActivate(kbset);

            EndDialog(hwnd, IDOK);
            return TRUE;
        case ID_KBCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
    }
    return FALSE;
}
KeyboardSettingsDlg::~KeyboardSettingsDlg()
{
    delete kbset_temp;
    delete shortcut;
    delete save;
    delete showkeys;
    delete restorekeys;
    delete commands;
    delete descript;
}


// ***********************************************************************************
//
// BSP Settings
//
BspSettingsDlg::BspSettingsDlg(HWND parent, char* start_group)
    : WDialog(parent, BSP_SETTINGS), start_group(start_group)
{
    wprop = 0;
}
BspSettingsDlg::~BspSettingsDlg()
{
    delete wprop;
}
INT_PTR BspSettingsDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
    {
        HICON ico = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_QUAKESMALL));
        ::SendMessage(hwnd,WM_SETICON,ICON_BIG,(LPARAM)ico);

        wprop = new WPropertyGrid(hwnd,100, 8,8, 350,180);
        wprop->Create();

        wbtn_ok = new WButton(hwnd,IDOK);
        wbtn_cancel = new WButton(hwnd,IDC_BUTTON3);
        wbtn_write = new WButton(hwnd,IDC_BUTTON1);
        wbtn_writegame = new WButton(hwnd,IDC_BUTTON2);
        iteminfo = new WStatic(hwnd,IDC_STATIC1);
        groupstatic = new WStatic(hwnd,IDC_STATIC2);
        grouplist = new WCombo(hwnd,IDC_COMBO1);
        FillPropertyGrid(start_group);
        FillGroupList(start_group);

        RECT rc;
        GetWindowRect(hwnd, &rc);
        Resize(rc.right-rc.left,rc.bottom-rc.top);
    }
    break;

    case WM_SIZING:
        Sizing(wParam, (RECT*)lParam);
        return TRUE;

    case WM_SIZE:
        Resize((DWORD)LOWORD(lParam),(DWORD)HIWORD(lParam));
        return TRUE;

    case WM_MOUSEWHEEL:
        if(((short) HIWORD(wParam)) > 0)
        {
            wprop->ScrollUp();
            wprop->ScrollUp();
            wprop->ScrollUp();
        }
        else
        {
            wprop->ScrollDown();
            wprop->ScrollDown();
            wprop->ScrollDown();
        }

        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            BtnOk();
            return TRUE;
        case IDCANCEL:	//"x" close button
            BtnClose();
            return TRUE;
        case IDC_BUTTON3:
            BtnCancel();
            return TRUE;
        case IDC_BUTTON1:
            BtnWriteBspSettings();
            return TRUE;
        case IDC_BUTTON2:
            BtnWriteGameSettings();
            return TRUE;
        case IDC_COMBO1:
            if(HIWORD(wParam)==CBN_SELENDOK)
            {
                int idx = grouplist->GetSelIndex();
                if(0 == idx)
                {
                    ChangePropertyView(0);
                }
                else
                {
                    char buf[200];
                    grouplist->GetString(buf, idx);
                    ChangePropertyView(buf);
                }
            }
            return TRUE;
        }
        break;

/////////////////////////////////////////////////////////////////////////////
// PROPERTY GRID MESSAGES
// - return value must be set with: SetWindowLong(hwnd, DWL_MSGRESULT, retval);
    case PM_DBLCLICK:
        if(lParam == WPropertyGrid::partRight)
        {
            if(wParam)
            {
                cset_t *c = (cset_t *)((WPropItem *)wParam)->data;
                if(c && c->setinfo && c->setinfo->values)
                {
                    if(!wprop->wplist->IsWindowVisible())
                    {
                        StartEdit();
                        wprop->ClickEditButton();
                    }
                }
            }
        }
        return TRUE;

    case PM_CLICK:
        //show edit box. if item is dropdown list type, dont do anything
        if(lParam == WPropertyGrid::partRight || lParam == WPropertyGrid::partAny)
        {
            if(wParam)
            {
                cset_t *c = (cset_t *)((WPropItem *)wParam)->data;
                if(c && ((!c->setinfo || !c->setinfo->values) || !wprop->edit_button->visible))
                {
                    StartEdit();
                    //select all if got here by enter press
                    if(wprop->edit_visible && lParam == WPropertyGrid::partAny)
                        wprop->wpedit->SendMessage(EM_SETSEL,0,-1);
                }
            }
        }
        return TRUE;

    case PM_CHANGE:
        //set item colors for color type
        if(wParam)
        {
            WPropItem *pi = (WPropItem*)wParam;
            cset_t *c = (cset_t *)((WPropItem *)wParam)->data;
            if(c && c->type == CSET_COLOR)
            {
                pi->backcolor = Parse_Color((char*)lParam);
            }
        }
        //allow all changes...
#ifdef _WIN64
        SetWindowLong(hwnd, DWLP_MSGRESULT, 1); //return value must be set this way
#else
        SetWindowLong(hwnd, DWL_MSGRESULT, 1); //return value must be set this way
#endif
        return TRUE;

    case PM_SELCHANGE:
        //set info text
    {
        char str[250] = "";
        WPITEM *wpi = wprop->GetItem();
        if(wpi && wpi->data && wpi->data->data)
        {
            cset_t *c = (cset_t *) wpi->data->data;
            if(c->setinfo && c->setinfo->info)
                strncpy(str, c->setinfo->info, sizeof(str));
        }
        iteminfo->SetText(str);
    }
    //tell property grid it is ok to show button on right part
#ifdef _WIN64
    SetWindowLong(hwnd, DWLP_MSGRESULT, WPropertyGrid::partRight);
#else
    SetWindowLong(hwnd, DWL_MSGRESULT, WPropertyGrid::partRight);
#endif
    return TRUE;

    case PM_EDITBTN:
        //show color dialog
        if(wParam)
        {
            WPropItem *pi = (WPropItem*)wParam;
            cset_t *c = (cset_t *)((WPropItem *)wParam)->data;
            if(c && c->type == CSET_COLOR)
            {
                wprop->HideEdit(true);
                ColorDialog cd(hwnd, pi->backcolor);	//use the last set color value
                if(cd.Show())
                {
                    pi->backcolor = cd.result;
                    pi->modified = true;
                    wprop->modified = true;
                    Str colorstr(const_cast<char *> ("#%.2X%.2X%.2X"),GetRValue(cd.result),GetGValue(cd.result),GetBValue(cd.result));
                    pi->set_value(colorstr);
                    wprop->Redraw();
                }
            }
        }
        break;

    case PM_TABBED:
        if(wprop->selected_idx < 0)
            wprop->selected_idx = 0;
        else if(wParam)
            wprop->MoveSelUp();
        else
            wprop->MoveSelDown();
        if(StartEdit() && IsWindowVisible(wprop->wpedit->hwnd))
        {
            ::SetFocus(wprop->wpedit->hwnd);
            wprop->wpedit->SendMessageA(EM_SETSEL,0,-1);//selectall
        }
        break;

    case PM_POPLIST:
    {
        int retval = 0;
        WList *list = (WList*) lParam;
        if(wParam && list)
        {
            cset_t *c = (cset_t *)((WPropItem *)wParam)->data;
            if(c && c->setinfo && c->setinfo->values)
            {
                list->ClearList();
                listitem<setinfo_value*> *li = c->setinfo->values->items;
                for( ; li; li = li->next)
                {
                    int id = list->AddString(li->data->info);		//this is displayed
                    list->SetItemData(id, (DWORD_PTR)li->data->value);	//this is the value to return
                }
                retval = 1;
            }
        }
#ifdef _WIN64
        SetWindowLong(hwnd, DWLP_MSGRESULT, retval);
#else
        SetWindowLong(hwnd, DWL_MSGRESULT, retval);
#endif
        return TRUE;
    }
/////////////////////////////////////////////////////////////////////////////

    }
    return FALSE;
}
int BspSettingsDlg::GetMinWidth()
{
    RECT a,c,b,d;
    GetClientRect(wbtn_write->hwnd, &a);
    GetClientRect(wbtn_writegame->hwnd, &b);
    GetClientRect(wbtn_ok->hwnd, &c);
    GetClientRect(wbtn_cancel->hwnd, &d);
    return 8 + a.right + 4 + b.right + 4 + c.right + 4 + d.right + 8 + GetSystemMetrics(SM_CXSIZEFRAME)*2;
}
#define MINHEIGHT 200
void BspSettingsDlg::Sizing(int side, RECT *rc)
{
    int minwidth = GetMinWidth();
    //limit width
    if(rc->right-rc->left < minwidth)
    {
        if(side==WMSZ_BOTTOMLEFT || side==WMSZ_LEFT || side==WMSZ_TOPLEFT)
        {
            rc->left = rc->right - minwidth;
        }
        else if(side==WMSZ_BOTTOMRIGHT || side==WMSZ_RIGHT || side==WMSZ_TOPRIGHT)
        {
            rc->right = rc->left + minwidth;
        }
    }
    //limit height
    if(rc->bottom-rc->top < MINHEIGHT)
    {
        if(side==WMSZ_TOP || side==WMSZ_TOPLEFT || side==WMSZ_TOPRIGHT)
        {
            rc->top = rc->bottom - MINHEIGHT;
        }
        else if(side==WMSZ_BOTTOM || side==WMSZ_BOTTOMLEFT || side==WMSZ_BOTTOMRIGHT)
        {
            rc->bottom = rc->top + MINHEIGHT;
        }
    }
}
void BspSettingsDlg::Resize(int width, int height)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    InflateRect(&rc, -8, -8);	//pad edges

    RECT a,c,b,d, rcinfo, rcgs;
    GetClientRect(wbtn_write->hwnd, &a);
    GetClientRect(wbtn_writegame->hwnd, &b);
    GetClientRect(wbtn_ok->hwnd, &c);
    GetClientRect(wbtn_cancel->hwnd, &d);
    GetWindowRect(iteminfo->hwnd, &rcinfo);
    GetClientRect(groupstatic->hwnd, &rcgs);

    int ypos = rc.bottom-a.bottom;
    SetWindowPos(wbtn_write->hwnd,0,		8, ypos ,0,0,SWP_NOZORDER|SWP_NOSIZE);
    SetWindowPos(wbtn_writegame->hwnd,0,	8 + a.right + 4, ypos ,0,0,SWP_NOZORDER|SWP_NOSIZE);
    SetWindowPos(wbtn_ok->hwnd,0,			rc.right-d.right-4-c.right, ypos ,0,0,SWP_NOZORDER|SWP_NOSIZE);
    SetWindowPos(wbtn_cancel->hwnd,0,		rc.right-d.right, ypos ,0,0,SWP_NOZORDER|SWP_NOSIZE);

    int yinfo = rc.bottom-a.bottom-8-(rcinfo.bottom-rcinfo.top);
    SetWindowPos(iteminfo->hwnd,0,			8, yinfo, rc.right-rc.left, rcinfo.bottom-rcinfo.top, SWP_NOZORDER);

    SetWindowPos(wprop->hwnd,0,				8, 8+rcgs.bottom + 8, rc.right-rc.left, yinfo-4-8 -(rcgs.bottom + 8), SWP_NOZORDER);

    InvalidateRect(hwnd, 0, 1);
}
bool BspSettingsDlg::StartEdit()
{
    wprop->ShowEdit(WPropertyGrid::partRight);
    return true;
}

//save settings to current session
bool BspSettingsDlg::SaveSettings()
{
    WPITEM *wpi = wprop->items.items;
    for( ; wpi; wpi = wpi->next)
    {
        if(!wpi->data)
            continue;
        WPropItem *pi = (WPropItem*)wpi->data;
        if(pi->modified && pi->data)
        {
            cset_t *c = (cset_t *)pi->data;
            if(c->setinfo && c->setinfo->values)
            {
                if(pi->list_value)	//list types will set this to a pointer to string
                    Cset_SetValue(pi->name, (char *) pi->list_value);
                else
                    syserror(const_cast<char *> ("couldn't save value for %s, value is null"),pi->name);
            }
            else
            {
                Cset_SetValue(pi->name, pi->value);
            }
            pi->modified = false;
        }
    }
    wprop->modified = false;
    Ccmd_UpdateEnablers();	//update commands after settings change
    return true;
}
void BspSettingsDlg::BtnWriteBspSettings()
{
    wprop->HideEdit(true);
    if(IDYES == MessageBox(hwnd, "Apply changes and update bsp.cfg?", "Confirm", MB_YESNO|MB_ICONQUESTION))
    {
        SaveSettings();
        CmWriteConfig();
    }
}
void BspSettingsDlg::BtnWriteGameSettings()
{
    wprop->HideEdit(true);
    if(IDYES == MessageBox(hwnd, "Apply changes and update game.cfg?", "Confirm", MB_YESNO|MB_ICONQUESTION))
    {
        SaveSettings();
        CmWriteGameConfig();
    }
}
void BspSettingsDlg::BtnCancel()
{
    EndDialog(hwnd, IDCANCEL);
}
void BspSettingsDlg::BtnOk()
{
    wprop->HideEdit(true);
    if(wprop->modified)
    {
        SaveSettings();
    }
    EndDialog(hwnd, IDOK);
}
void BspSettingsDlg::BtnClose()
{
    wprop->HideEdit(true);
    if(wprop->modified)
    {
        if(IDNO == MessageBox(hwnd, "Changes have not been saved. Discard changes?", "Confirm Close", MB_YESNO|MB_ICONQUESTION))
            return;
    }
    EndDialog(hwnd, IDCANCEL);
}
void BspSettingsDlg::ChangePropertyView(char *newgroupname)
{
    wprop->HideEdit(true);
    if(wprop->modified)
    {
        int result = MessageBox(hwnd, "Do you want to save your changes before switching views?", "Confirm Close", MB_YESNOCANCEL|MB_ICONQUESTION);
        if(result == IDCANCEL)
            return;
        else if(result == IDYES)
            SaveSettings();
    }
    wprop->Clear();
    FillPropertyGrid(newgroupname);
    wprop->Redraw();
}

void BspSettingsDlg::FillGroupList(char *select)
{
    cset_group_t *g = cset_groups;
    grouplist->ClearList();
    grouplist->AddString(const_cast<char *> ("All"));
    int sel = 0;
    for(; g; g=g->next)
    {
        int idx=grouplist->AddString(g->name);
        if(select && !_stricmp(select,g->name))
            sel = idx;
    }
    grouplist->SetSelIndex(sel);
}

bool BspSettingsDlg::FillPropertyGrid(char *groupname)
{
    cset_group_t *g = cset_groups;
    //for each group
    for(; g; g=g->next)
    {

        //limit to one group if requested
        if(groupname && _stricmp(g->name, groupname))
            continue;

        WPropItem* w = wprop->Add(g->name);
        w->header = true;

        //for each item in group
        for(cset_t *c = g->head; c; c=c->next_in_group)
        {
            char val[MAX_VALUE] = "";

            WPropItem *wpi = wprop->Add(c->name, 0, (DWORD_PTR) c);

            Cset_GetValueString(c, val, sizeof(val));
            if(c->type == CSET_COLOR)
            {
                wpi->backcolor = *(COLORREF*)c->value;
                wpi->button_type = DRAWB_ELLIPSES;
                wpi->max_edit_width = 130;
            }
            //dropdown type - try to replace 'val' with the info string
            if(c->setinfo && c->setinfo->values)
            {
                wpi->button_type = DRAWB_DOWN;
                listitem<setinfo_value*> *li = c->setinfo->values->items;
                for( ; li; li = li->next)
                {
                    if(!_stricmp(val, li->data->value))	 	//match to item
                    {
                        //set info text as value
                        strncpy(val, li->data->info, sizeof(val));
                        val[sizeof(val)-1] = 0;
                    }
                }
            }
            //set display value
            wpi->set_value(val);
        }
        //if groupname is not null, we just finished processing its group and can exit
        if(groupname)
        {
            break;
        }
    }
    return true;
}
