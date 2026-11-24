// BSP Copyright 1996 Yahn W. Bernier

#include "global.h"
#include "buildver.h"
#include <commctrl.h>

#define TIMER_IDLE	1				// timer replacing borlands "IdleAction" feature

//bool BSP_AutoLoad = false;
bool BSP_Active = true;				// disable certain tasks if BSP is not in foreground
bool KillAccels = false;			// kill accelerators for edit controls/etc
HINSTANCE hInstance;				// hinstance
HMENU hMenu = 0;					// main menu handle

int PALSTEP;						// Bytes to step to get to next palette triplet.

int LUMP_NAME_LENGTH = 16;          // Texture and .lmp filename length.  32 for Q2

int timeRender = 0;                 // True if we are profiling rendering.
DWORD startTime;                    // For profiling.
DWORD endTime;

int cColorBits    = 24;             // glBSP defaults.  Color Bits.
int cDepthBits    = 32;
int cTextureAlpha = 0;              // Include alpha byte when constructing gl textures?
int singleBuffer  = 0;              // Double buffer gl screens by default.
int nosplash      = 0;              // show the splash screen at startup.

TBSPSplashWindow *splash = 0;
WRebar *rebar = 0;
WToolbar **cbars = 0;
WStatus *status = 0;
FindReplaceDlg *FindReplace = 0;
KBSettings *kbset = 0;
WImageList imagelist;

HACCEL acceltable;

/*
==================
TBSPApp

This is the main application class for BSP

TBSPDecoratedMDIFrame

Constructor
===============
*/
TBSPApp::TBSPApp(const char *title)
    : Window((HWND)0, title)
{
    attr.dwStyle |= WS_CLIPCHILDREN | WS_MAXIMIZE;    // make sure tab control stays on top, default maximized.

    attr.width = GetSystemMetrics(SM_CXSCREEN) - 48; // Set initial width and height...
    attr.height = GetSystemMetrics(SM_CYSCREEN) - 72;
    attr.x = 16;
    attr.y = 16;
    wc.lpszClassName = "BspFrame";

    makeBox = NULL;
    makeBtn = NULL;
    TextureList = NULL;
    rebar = 0;
    data = 0;
}

LRESULT TBSPApp::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_CREATE:
        SetupWindow();
        Ccmd_UpdateEnablers();
        break;
    case WM_TIMER:
        client->IdleAction(1);
        break;
    case WM_SIZE:
    {
        SIZE sz = {LOWORD(lParam), HIWORD(lParam)};
        EvSize(wParam, sz);
        return 0;
    }
    case WM_INITMENU:
    {
        Ccmd_UpdateEnablers();

        return 0;
    }
    case WM_DROPFILES:
    {
        int filecount = DragQueryFile((HDROP)wParam, -1, 0, 0);
        for(int i=0; i<filecount; i++)
        {
            char buf[MAX_PATH];
            DragQueryFile((HDROP)wParam, i, buf, MAX_PATH);
            Caption(buf);
            Path filename(const_cast<char *>("%s"), buf);
            if(!stricmp("map",filename.ext()))
            {
                //open map file
                OpenMapFile(filename);
            }
            else
            {
                //open other types .... images
                int width=0, height=0;
                unsigned char*img=0;
                if(LoadImageFile(filename,&img,&width,&height,true))
                {
                    // in soviet russia, ImageView deletes itself
                    TransformImageBytes(img, width, height, TMODE_ARGB_ABGR, true);
                    ImageView *detach = new ImageView(client->hwnd, filename, img, width, height);
                    delete[]img;
                    detach->Create();
                    detach->ResetSize();
                }
            }
        }
        break;
    }
    case WM_COMMAND:
    {
        Ccmd_Execute(LOWORD(wParam));

        if(!c_noupdate)				//commands like navigation can skip updates
            Ccmd_UpdateEnablers();
        c_noupdate = false;

        break;
    }
    case WM_MENUSELECT:
    {
        char *str = const_cast<char *>("");
        if(!(HIWORD(wParam) & MF_POPUP))
        {
            ccmd_t *c = Ccmd_FindCmd(LOWORD(wParam));
            if(c && c->info_text)
                str = c->info_text;
        }
        status->SetText(str);
        status->DrawStatusBar();
        return 0;
    }
    case WM_NOTIFY:
    {
        NMHDR *nm = (NMHDR*)lParam;

        // resize rebar
        if(rebar && nm->hwndFrom==rebar->hwnd)
        {
            if(nm->code==RBN_HEIGHTCHANGE)
                ResizeFrame();
            break;
        }
        //display full info tip on statusbar
        if(nm->code==TBN_HOTITEMCHANGE)
        {
            NMTBHOTITEM *hi = (NMTBHOTITEM*)lParam;
            char buf[128] = "";
            ccmd_t *c = Ccmd_FindCmd(hi->idNew);
            if(c && c->info_text)
                strncpy(buf,c->info_text,sizeof(buf));
            status->SetText(buf);
            status->DrawStatusBar();
            break;
        }
        // set tooltip text
        if(nm->code==TTN_GETDISPINFO && nm->idFrom > 0)
        {
            NMTTDISPINFO *tt = (NMTTDISPINFO*)lParam;
            //use menu text as tooltip
            ccmd_t *c = Ccmd_FindCmd(nm->idFrom);
            if(c && c->info_text)
                strncpy(tt->szText,c->info_text,sizeof(tt->szText));
        }
        break;
    }
    case WM_PAINT:
        status->DrawStatusBar();
        break;
    case WM_CLOSE:
        EvClose();
        if(KillAllWindows)
        {
            DestroyWindow(hwnd);
        }
        return 0;
    case WM_ACTIVATEAPP:
        EvActivateApp(0 != wParam,(HANDLE)lParam);

        //run idle actions while active
        if(BSP_Active)
        {
            //rebuild menu when focus is gained...
            CmMenuRefresh();

            SetTimer(hwnd,TIMER_IDLE, 17, 0);	//about 60fps resolution
        }
        else
            KillTimer(hwnd,TIMER_IDLE);

        break;
    case WM_LBUTTONDOWN:
    {
        POINT pt = {LOWORD(lParam), HIWORD(lParam)};
        EvLButtonDown(wParam,pt);
        break;
    }
    }
    return DefFrameProc(hwnd, (client ? client->hwnd : 0),msg,wParam,lParam);
}

/*
=====================
TBSPApp

Destructor for Application
=====================
*/
TBSPApp::~TBSPApp()
{
    delete makeBox;
    delete makeBtn;

    // Kill control bars.
    for(int i=0; i<this->numbars; i++)
        delete cbars[i];
    delete[] cbars;
    delete rebar;

    delete[] data;
    DestroyWindow(client->hwnd);
    delete client;
}

// generates WM_SIZE message
void TBSPApp::ResizeFrame()
{
    if(!frame || !client || !rebar || !status)
        return;
    RECT r;
    GetClientRect(frame->hwnd,&r);
    SIZE sz = {r.right, r.bottom};
    frame->EvSize(0,sz);
}

/*
================
EvActivateApp

Handle WM_ActivateApp message.
If we are not active, we don't do idle processing, such as animated textures.
================
*/
void TBSPApp::EvActivateApp(bool active, HANDLE threadId)
{
    BSP_Active = active;
}
/*
================
SetCaption

Set's the caption of the main BSP frame.
================
*/
void TBSPApp::SetCaption(const char* title)
{
    //game editing mode
    char *title1 = const_cast<char *>("cBSP ");
    char *userdef = const_cast<char *>("");
    if (*set.apptitle)
        userdef = set.apptitle;		//user defined
    if (set.valveBsp)
        title1 = const_cast<char *>("cBSP HL ");
    else if (set.sinBsp)
        title1 = const_cast<char *>("cBSP SiN");
    else if (set.game_mode == 1)
        title1 = const_cast<char *>("cBSP Hexen");
    else if (set.game_mode == 2)
        title1 = const_cast<char *>("cBSP/II ");

    //insert user data into title
    char *title2 = const_cast<char *>("");
    char *title3 = const_cast<char *>("");
    char *title4 = const_cast<char *>("");
    if(title && *title)
    {
        title2 = const_cast<char *> ("- [");
        title3 = const_cast<char *> (title);
        title4 = const_cast<char *> ("]");
    }

    // show -gl if needed
    char *opengl = set.glBsp ? const_cast<char *> ("- OpenGL") : const_cast<char *> ("");

    //build
    char outstr[256];
    snprintf(outstr,sizeof(outstr),"%s%s%s%s%s%s",title1,userdef,title2,title3,title4,opengl);
    Window::SetCaption(outstr);
}
/*
==============
EvClose

Close button clicked on main frame
==============
*/
void TBSPApp::EvClose()
{
    CmExitFinal();
}
/*
===============
SetupWindow

Prepares decorations on the frame
===============
*/
void TBSPApp::SetupWindow()
{
    // Create splash window.
    splash = new TBSPSplashWindow(SPLASH);
    if (!nosplash)
        splash->Create();

    // Set main windows icon
    frame->SetIcon(IDI_QUAKESMALL);

    SetCaption("");

    // Create the MDI Client.
    client = new TBSPWindow(frame->hwnd);
    client->Create();

    // load toolbar
    frame->ConstructControlBars();

    //create status bar
    status = new WStatus(frame->hwnd, 8);
    status->SetFont(GetFont("ms sans serif",-9));
    status->SetPart(STATUS_INFO,0);					//main status
    status->SetPart(STATUS_MODE,20,false,true);		//mode (bitmap)
    status->SetPart(STATUS_HEIGHT,9,true);			//height
    status->SetPart(STATUS_GROUP,8,true);			//group
    status->SetPart(STATUS_POS,7,true);				//pos
    status->SetPart(STATUS_DELTA,7,true);			//delta
    status->SetPart(STATUS_BOUND,10,true);			//bound
    status->SetPart(STATUS_MAP,3,true);				//map
    status->SetText(STATUS_MAP, "Map");				//default text

    HBITMAP bmpstrip = load_bitmap(const_cast<char *> ("settings/gfx/misc/mapgadgets.bmp"), LB_3DCOLORS);
    status->SetBmp(bmpstrip, 16);
    status->SetPartBmp(STATUS_MODE, 3);				//question mark ico

    //resize frame, else statusbar will initially be hidden
    ResizeFrame();

    //intialize all the important stuff :)
    client->SetupWindow();
}
/*
===============
EvSize

Resizing
===============
*/
void TBSPApp::EvSize(UINT sizeType, SIZE& size)
{
    // Let frame resize the client

    RECT r = {0,0,0,0},s;
    if(rebar)
        GetClientRect(rebar->hwnd,&r);
    status->GetRect(&s);
    //move mdi client
    MoveWindow(client->hwnd,
               0,
               r.bottom,
               size.cx, size.cy - (r.bottom-r.top) - (s.bottom-s.top),true);
    //resize rebar
    if(rebar)
        MoveWindow(rebar->hwnd, r.left, r.top, size.cx, r.bottom-r.top, true);

    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Resized...",true);
}

/*
================
Message Handlers
================
*/

void TBSPApp::EvLButtonDown(UINT modKeys, POINT& point)
{
    RECT r;
    status->GetPartRect(STATUS_MAP, &r);
    if(::PtInRect(&r,point))
        status_ShowOpenMapsMenu();
}
/*
================
SetupWindow

Set the window font
================
*/
TBSPSplashWindow::TBSPSplashWindow(int resdib)
    : Window(0,0)
{
    this->resdib = resdib;
    attr.dwExStyle = WS_EX_TOOLWINDOW;	//hide from taskbar
    attr.dwStyle = WS_POPUP | WS_BORDER;
    wc.lpszClassName="BspSplashWin";
    caption=0;
    bitmap = LoadBitmap(hInstance,MAKEINTRESOURCE(resdib));
    BITMAP bm;
    GetObject(bitmap,sizeof(BITMAP),&bm);
    bmpw = bm.bmWidth;
    bmph = bm.bmHeight;
    attr.width = bmpw + 2;
    attr.height = bmph + 32;
}

TBSPSplashWindow::~TBSPSplashWindow()
{
    DeleteObject(bitmap);
}

LRESULT TBSPSplashWindow::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_CREATE:
        SetupWindow();
        ShowWindow(SW_SHOW);
        //fall to paint
    case WM_ERASEBKGND:
        //fall to paint
    case WM_PAINT:
    {
        HDC hdc = GetDC(hwnd);
        HDC cdc = CreateCompatibleDC(hdc);
        HGDIOBJ oldbmp = SelectObject(cdc,bitmap);

        BitBlt(hdc,0,0,bmpw,bmph,cdc,0,0,SRCCOPY);		//paint bitmap

        RECT rc;
        SetRect(&rc, 0, bmph, attr.width, attr.height);
        FillRect(hdc,&rc,(HBRUSH)GetStockObject(WHITE_BRUSH));		//fill white in rest of window

        //display version
        char ver[64];
        int len = sprintf(ver,"Version %i.%i" BSP_REV, BSP_VER_MAJOR, BSP_VER_MINOR);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(0,255,0));
        SetTextAlign(hdc, TA_RIGHT);
        HFONT font = GetFont("small fonts",12);
        HGDIOBJ oldfont = SelectObject(hdc, font);
        TextOut(hdc, this->attr.width-5, 5, ver, len);
        SelectObject(hdc, oldfont);
        DeleteObject(font);

        SelectObject(cdc,oldbmp);
        DeleteDC(cdc);
        ReleaseDC(hwnd, hdc);
    }
    return 0;
    case WM_KILLFOCUS:
        //always on top
        SetWindowPos(HWND_TOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE );
        break;
    case WM_CTLCOLORSTATIC:
        return (LRESULT) GetStockObject(WHITE_BRUSH);
    }
    return DefWindowProc(hwnd,msg,wParam,lParam);
}
void TBSPSplashWindow::SetupWindow()
{
    CenterWindow(hwnd);
    caption = new WStatic(hwnd,100,0,2,attr.height-24,attr.width-4,16);
    caption->Create();
    caption->SetFont(set.font_ui);
}

void TBSPSplashWindow::SetText(char *str)
{
    Sleep(5);							/// keep splash on screen a little bit longer
    if(caption) caption->SetText(str);
}

/*
=================
LButtonDown

Handle mouse click on gadget
=================
*/
void status_ShowOpenMapsMenu()
{
    //determine and cache cmd ids
    static int selmap_id[MAX_RECENT];
    static bool selmap_init = false;
    if(!selmap_init)
    {
        selmap_init = true;
        for(int i = 0; i < 10; i++)
        {
            char buf[128];
            snprintf(buf, sizeof(buf), "SelectMap%d", i+1);
            ccmd_t *cm = Ccmd_FindCmd(buf);
            if(cm) selmap_id[i] = cm->id;
        }
    }

    char fname[128];
    Popup MapMenu;

    // Loop through maps and grab root of file name for display in menu.
    for (int i = 0; i < set.nummaps; i++)
    {
        ExtractFileBase(map_i[i]->filename,fname,true);		// Get root.
        strlwr(fname);										// Convert to lower case.
        if (fname) toupper((unsigned char)*fname);		// Capitalize first letter
        if (i == set.curmap) MapMenu.Check();				// Checkmark the current map.
        MapMenu.Add(fname, selmap_id[i]);
    }

    RECT r;
    status->GetPartRect(STATUS_MAP, &r);
    MapMenu.pt.x = r.left;
    MapMenu.pt.y = r.top;
    ClientToScreen(frame->hwnd, &MapMenu.pt);
    MapMenu.Show(frame->hwnd);
}

/*
===============
EvCBNSelChange

Combo box selection change has occurred
===============
*/
void TTextureComboBox::EvCBNSelChange()
{
    // Validate.
    if (!set.Map_Read)
        return;
    if (!texWindow)
        return;
    if (!map_i || !map_i[set.curmap])
        return;
    if (!frame->TextureList)
        return;

    // Grap the texture list pointer.
    TTextureComboBox *box = frame->TextureList;

    // Get the current selection.
    int idx = box->GetSelIndex();

    // Get the private item data pointer
    ListBoxData* data = (ListBoxData *)box->GetItemData(idx);
    if (!data)
        return;

    map *m = map_i[set.curmap];
    // Set the .map's current texture to the name.
    STRNCPY(m->texName,data->description);

    // For Q2, assume we can't resolve the base path.
    if (set.game_mode == 2)
        strcpy(m->texPath,"UNKNOWN");

    // Force texture browser to redraw with new texture selection.
    texWindow->prevIndex = idx;
    if (idx < 0 || idx >= m->numMapTextures )
        return;

    // Get a pointer to that texture.
    texInfo_t *tx = m->mapTextures[texWindow->prevIndex];
    if (tx)
    {
        texWindow->NewCurrentTexture(tx);        // Set up new texture.
        texWindow->texList->SetText(tx->bmp);    // Set text in texture browser dropdown.
        if (set.game_mode == 2)
            sprintf(m->texPath,tx->basepath);  // Fill in correct q2 base
    }

    // Clean up in texture window.
    // FIXME, make into separate function.
    texWindow->curstartrow = (int)(texWindow->prevIndex/texWindow->colsperrow);
    texWindow->Scroller.Y.nMax = texWindow->totrows; //texWindow->Scroller->SetRange(0,texWindow->totrows);
    texWindow->Scroller.Y.nPos = texWindow->curstartrow;
    texWindow->Scroller.Update();

    texWindow->Select(texWindow->prevIndex,true);
}

/*
=================
EvDrawItem

Owner drawn texture in dropdown list
=================
*/
void TTextureComboBox::EvDrawEdit(UINT, DRAWITEMSTRUCT* dis)
{
    if (!set.Map_Read)
        return;

    if (!(dis->itemState & ODS_COMBOBOXEDIT))         // Redraw the edit control portion.
        return;

    HDC hdc = dis->hDC;
    HBRUSH background = (HBRUSH) GetStockObject(BLACK_BRUSH);
    SetTextColor(hdc, RGB(255, 255, 255));			// white text

    // Clear background.
    FillRect(hdc, &dis->rcItem, background);

    // Get data pointer.
    ListBoxData* data = (ListBoxData *) dis->itemData;
    if (!data || !dis->itemData || (int)dis->itemData == -1)
        return;

    // Set font and font mode.
    HGDIOBJ oldfont = SelectObject(hdc, set.font10);
    SetBkMode(hdc, TRANSPARENT);

    RECT dst, src;
    src.top = 0;
    src.left = 0;
    src.right = data->tx->w;
    src.bottom = data->tx->h;
    dst.left = dis->rcItem.left;
    dst.top = dis->rcItem.top;

    int ht = dis->rcItem.bottom - dis->rcItem.top;

    // Draw the text description.
    TextOut(hdc, dis->rcItem.left+ ht + 2, dis->rcItem.top + 3, data->description, strlen(data->description) );

    // Determine how to stretch texture.
    texWindow->GetTextureDisplayRects(dst.left, dst.top, &src, &dst, data->tx, ht, ht);

    // Set DIB stretch mode.
    SetStretchBltMode(hdc, COLORONCOLOR);

    // Draw texture into rectangle.
    if (data && data->tx && data->tx->bits && data->tx->dibInfo)
    {
        ::StretchDIBits( hdc,
                         dst.left,dst.top,dst.right-dst.left,dst.bottom-dst.top,
                         src.left,src.top,src.right-src.left,src.bottom-src.top,
                         data->tx->bits,
                         data->tx->dibInfo,
                         DIB_RGB_COLORS,
                         SRCCOPY);
    }

    // If we are focused, draw a focus rect, too.
    if (dis->itemState & ODS_FOCUS)
        DrawFocusRect(hdc, &dis->rcItem);

    SelectObject(hdc, oldfont);
}
/*
====================
TTextureComboBox

=====================
*/
TTextureComboBox::TTextureComboBox(HWND parent, int id, int x, int y, int w, int h, UINT textLimit)
    : WCombo(parent,id,x,y,w,h,textLimit)
{
    attr.dwStyle |= CBS_DROPDOWNLIST | CBS_OWNERDRAWVARIABLE;
}

LRESULT TTextureComboBox::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_CREATE:
        SetupWindow();
        break;
    case WM_COMPAREITEM:
    {
        COMPAREITEMSTRUCT *cis = (COMPAREITEMSTRUCT*)lParam;
        return EvCompareItem(wParam,cis);
    }
    case WM_DRAWITEM:
    {
        DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT*)lParam;
        if(dis->itemState & ODS_COMBOBOXEDIT)
            EvDrawEdit(wParam,dis);
        else
            EvDrawItem(wParam,dis);
        return TRUE;
    }
    case WM_MEASUREITEM:
    {
        MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT*)lParam;
        EvMeasureItem(wParam,mis);
        return TRUE;
    }
    case WM_COMMAND:
        if(HIWORD(wParam)==CBN_SELCHANGE)
        {
            EvCBNSelChange();
        }
    }
    return WControl::WndProc(msg,wParam,lParam);
}

// EvCompareItem -- Combo box sorting callback.
LRESULT TTextureComboBox::EvCompareItem(UINT, COMPAREITEMSTRUCT* cis)
{
    if (!set.Map_Read)
        return 0;

    // Fixme, strip * and @ characters from front of name as in textures.cpp.
    LPSTR text1 = ( (ListBoxData *) cis->itemData1)->description;
    LPSTR text2 = ( (ListBoxData *) cis->itemData2)->description;

    return lstrcmp(text1, text2);
}

// EvDrawItem -- Owner draw combo box
void TTextureComboBox::EvDrawItem(UINT, DRAWITEMSTRUCT* dis)
{
    if (!set.Map_Read)
        return;

    // compute the size of the focus rectangle
    RECT rect;
    SetRect(&rect, dis->rcItem.left,dis->rcItem.top, dis->rcItem.left+set.texlist_size, dis->rcItem.top+set.texlist_size);

    HDC hdc = dis->hDC; //TDC dc(dis.hDC);

    if (dis->itemAction & (ODA_DRAWENTIRE | ODA_SELECT) )
    {
        HPEN pen;
        HBRUSH background;
        if (dis->itemState & ODS_SELECTED)
        {
            background = (HBRUSH) GetStockObject(BLACK_BRUSH);
            SetTextColor(hdc, RGB(255, 255, 255));				// white text
            pen = CreatePen(PS_SOLID,1,RGB(200,192,186));		// light gray pen
        }
        else
        {
            background = (HBRUSH) GetStockObject(LTGRAY_BRUSH);
            SetTextColor(hdc, RGB(0, 0, 0));					// black text
            pen = CreatePen(PS_SOLID,1,RGB(0,0,0));				// black pen
        }
        HGDIOBJ oldpen = SelectObject(hdc, pen);

        dis->rcItem.bottom -= 2;
        FillRect(hdc, &dis->rcItem, background);
        dis->rcItem.bottom++;
        MoveToEx(hdc, dis->rcItem.left,dis->rcItem.bottom, 0);
        LineTo(hdc, dis->rcItem.right,dis->rcItem.bottom);
        dis->rcItem.bottom++;

        ListBoxData* data = (ListBoxData *) dis->itemData;

        // display the text
        SelectObject(hdc, set.font10);
        SetBkMode(hdc, TRANSPARENT);

        RECT dst, src;
        SetRect(&src, 0, 0, data->tx->w, data->tx->h);
        SetRect(&dst, dis->rcItem.left, dis->rcItem.top, 0, 0);
        texWindow->GetTextureDisplayRects(dst.left, dst.top, &src, &dst, data->tx, set.texlist_size, set.texlist_size);
        SetStretchBltMode(hdc, COLORONCOLOR);

        if (data && data->tx && data->tx->bits && data->tx->dibInfo)
        {
            ::StretchDIBits( hdc,
                             dst.left,dst.top,dst.right-dst.left,dst.bottom-dst.top,
                             src.left,src.top,src.right-src.left,src.bottom-src.top,
                             data->tx->bits,
                             data->tx->dibInfo,
                             DIB_RGB_COLORS, SRCCOPY);
        }

        TextOut(hdc, dis->rcItem.left + set.texlist_size + 2,
                dis->rcItem.top, data->description, strlen(data->description) );

        char bounds[80];
        int len = sprintf(bounds,"[%i x %i]",data->tx->w,data->tx->h);
        SelectObject(hdc, set.font9);
        TextOut(hdc, dis->rcItem.left + set.texlist_size + 2 + 2,
                dis->rcItem.bottom - 9 - 6, bounds,len);

        if (dis->itemState & ODS_FOCUS)
            DrawFocusRect(hdc,&rect);

        SelectObject(hdc, oldpen);
        DeleteObject(pen);
    }
    else if (dis->itemAction & ODA_FOCUS)
    {
        // Draw the focus rectangle
        DrawFocusRect(hdc, &rect);
    }
}

//EvMeasureItem
void TTextureComboBox::EvMeasureItem(UINT, MEASUREITEMSTRUCT* mis)
{
    int height = 16;

    if (set.Map_Read)
        height = set.texlist_size;

    mis->itemHeight = height + 2;	// 2 pix buffer.
}

//SetupWindow
void TTextureComboBox::SetupWindow()
{
    // make the combobox shorter
    SendMessage(CB_SETITEMHEIGHT, -1, 16 + 2);		// set height of selection field
}

/*
===================
ConstructControlBars

Parses in the button bars from bspbars.cfg
===================
*/
void TBSPApp::ConstructControlBars()
{
    // Now allocate space for it.  Add a bit of padding space.
    char *dat = (char*) file_get_contents(const_cast<char *> ("settings\\bspbars.cfg"));	// Load bspbars.cfg file
    if (!dat)
    {
        syserror (const_cast<char *> ("Parsing Button Bars: Error opening settings\\bspbars.cfg"));
        return;
    }

    Tokenizer config(dat);			// Tokenize it.

    // No tokens?
    if (!config.next(true))
    {
        syserror (const_cast<char *> ("Parsing Button Bars: 'bars' not found"));
        goto error_exit;
    }
    // Should start with "bars"
    if (strcmpi(config.token, "bars"))
    {
        syserror (const_cast<char *> ("Parsing Button Bars: 'bars' not found, %s instead"),config.token);
        goto error_exit;
    }
    // Then number, without going to next line.
    if (!config.next(false))
    {
        syserror (const_cast<char *> ("Parsing Button Bars: number of bars not found"));
        goto error_exit;
    }
    // Set # of bars.
    frame->numbars = atoi(config.token);
    if(frame->numbars < 1)
        frame->numbars = 1;		//must have atleast one bar. worst case is it will be empty

    // Allocate space for the bars and their positions.
    cbars = new WToolbar*[frame->numbars];
    memset(cbars,0,frame->numbars*sizeof(WToolbar*));

    //create toolbar host
    rebar = new WRebar(frame->hwnd);

    char outstr[40];
    // Create control bars.
    for (int i = 0; i < frame->numbars; i++)
    {
        cbars[i] = new WToolbar(frame->hwnd,20,20,20,20);
        ::SendMessage(cbars[i]->hwnd,TB_SETIMAGELIST,0,(LPARAM)imagelist.imbig);

        sprintf(outstr,"BSP Bar %i", i+1);
        ::SetWindowText(cbars[i]->hwnd, outstr);
    }

    // Now parse in the definitions.
    while(1)
    {
        if (!config.next(true))			// No more tokens.
            break;						// done...
        if (strcmpi (config.token, "bar"))		// Should start with "bar"
            break;
        if (!config.next(false))			// On same line should have bar number
            break;						// done/error...
        int barno = atoi(config.token);
        if (!config.next(false))			// Should still have function name on same line.
            break;						// done...
        ParseControl(barno, config.token);		// Assign the info
    }
    for(int i=0; i < frame->numbars; i++)
    {
        cbars[i]->index = i;
        rebar->AddBand(cbars[i]->hwnd);
    }
error_exit:
    delete[] dat;
}


/*
=================
ParseControl


Parses a button from the control stream into the specified bar.
=================
*/

//Toolbar hosting texture list must process WM_DRAWITEM message
WNDPROC old_tbWndProc;
LRESULT CALLBACK tbWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    if(msg==WM_DRAWITEM && wParam==IDCB_TEXBOX1)
    {
        return SendMessage(frame->TextureList->hwnd,msg,wParam,lParam);
    }
    return CallWindowProc(old_tbWndProc,hwnd,msg,wParam,lParam);
}

//add a control to a toolbar
void TBSPApp::ParseControl(int bar, char *control)
{
    if ((bar > frame->numbars) || (bar < 1))   // Bogus #?
        return;

    WToolbar *dest = cbars[bar-1];

    // special case for TextureList
    if (!strcmpi(control,"TextureList"))
    {
        // Only put the list in one button bar.  Or else major problems.  :)
        static bool list_loaded = false;
        if (!list_loaded)
        {
            list_loaded = true;
            frame->TextureList = new TTextureComboBox(
                dest->hwnd,
                IDCB_TEXBOX1,
                0,0,set.texture_list_width,(int)(1.75*(float)set.scy),
                40);
            frame->TextureList->Create();
            frame->TextureList->SetFont(set.font12);

            //subclass toolbar that owns TextureList
#ifdef _WIN64
            old_tbWndProc = (WNDPROC)::SetWindowLong(dest->hwnd,GWLP_WNDPROC,(LONG_PTR) tbWndProc);
#else
            old_tbWndProc = (WNDPROC)::SetWindowLong(dest->hwnd,GWL_WNDPROC,(LONG) tbWndProc);
#endif
            frame->data = new ListBoxData[1];
            memset(frame->data,0,sizeof(ListBoxData));
            dest->AddControl(frame->TextureList->hwnd);
            frame->TextureList->SetFont(set.font12, 1);
        }
        else
        {
            syserror(const_cast<char *> ("Warning: Only one texture list is allowed in the toolbar"));
        }
    }
    // special case for CM_MAKE
    else if (!strcmpi(control,"CM_MAKE"))
    {
        if (!frame->makeBox)
        {
            int makeid = -1;
            int id = Ccmd_GetId(const_cast<char *> ("CM_MAKE"));
            if(id > 0)
                makeid = id;

            frame->makeBtn = new WButton(dest->hwnd,makeid,0,0,0,32+6,20);
            frame->makeBtn->attr.dwStyle |= BS_BITMAP;
            frame->makeBtn->Create();
            HANDLE hmake = load_bitmap(const_cast<char *> ("settings/gfx/misc/CM_MAKE.bmp"), LB_3DCOLORS);
            frame->makeBtn->SendMessage(BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)hmake);
            dest->AddControl(frame->makeBtn->hwnd);

            frame->makeBox = new WCombo(dest->hwnd,IDCB_MAKEBOX,0,0,set.texture_list_width,(int)(1.75*(float)set.scy),40);
            frame->makeBox->attr.dwStyle |= CBS_DROPDOWNLIST;
            frame->makeBox->Create();
            frame->makeBox->SetFont(set.font_ui,1);
            dest->AddControl(frame->makeBox->hwnd);
        }
        else
        {
            syserror(const_cast<char *> ("Warning: Only one 'Make' button is allowed in the toolbar"));
        }
    }
    // add separator
    else if (!strcmpi(control,"Separator"))
    {
        dest->AddSeparator();
    }
    // add regular toolbar button
    else
    {
        ccmd_t *c = Ccmd_FindCmd(control);
        if(c)
            dest->AddButton(c->image, c->id);
        else
            syserror(const_cast<char *> ("Warning: Couldn't find toolbar button for '%s'", control));
    }

}

/*
=====================
InitGameSettings

old: Loads settings from the selected .cfg settings file.
=====================
*/
void InitGameSettings()
{
    // any
    if (set.game_mode == 2)
    {
        if (strcmpi(set.pak_file, "none"))
        {
            set.pakOk = 1;

            while (!VerifyPak(set.pak_file))
            {
                char outstr[128];
                sprintf(outstr,"[%s] is not a valid .pak file, choose another file...",set.pak_file);
                ::MessageBox(0,outstr,"BSP Loader",MB_ICONEXCLAMATION | MB_OK);

                char filename[256];
                *filename = 0;

                OPENFILENAME ofn;
                memset(&ofn,0,sizeof(OPENFILENAME));
                ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
                ofn.hwndOwner = 0;//frame->hwnd;
                ofn.lpstrFilter = "Pak files (*.pak)\0*.pak\0";
                ofn.lpstrDefExt = "pak";
                ofn.lpstrFile = filename;
                ofn.nMaxFile = 256;
                ofn.lpstrTitle = "Find Pak File";
                ofn.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
                if(!GetOpenFileName(&ofn))
                {
                    //	sprintf(outstr,"Pak will be ignored when loading textures...",set.pak_file);
                    //	::MessageBox(0,outstr,"BSP Loader",MB_ICONEXCLAMATION | MB_OK);
                    set.pakOk = 0;
                    break;
                }
                strncpy(set.pak_file,filename,sizeof(set.pak_file));
            }
        }
        else
        {
            set.pakOk = 0;
        }
    }

    //set some default colors in case something blows up
    unsigned char defcolor[] = {0,0,0, 128,128,128, 255,255,255, 255,0,0, 0,255,0, 0,0,255, 255,255,0, 255,0,255, 0,255,255 };
    memcpy(set.pal, defcolor, sizeof(defcolor));

    // Now parse in the .pal file.
    Path fn(const_cast<char *> ("%s/%s"),set.game_directory,set.palette_file);
    unsigned char *raw = file_get_contents(fn);
    if(!raw)
    {
        char filename[256];
        strncpy(filename,set.palette_file,sizeof(filename));
        filename[sizeof(filename)-1] = 0;

        OPENFILENAME ofn;
        memset(&ofn,0,sizeof(OPENFILENAME));
        ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
        ofn.hwndOwner = 0;//frame->hwnd;
        ofn.lpstrFilter = "Palette File (*.pal)\0*.pal\0";
        ofn.lpstrDefExt = "pal";
        ofn.lpstrFile = filename;
        ofn.nMaxFile = 256;
        ofn.lpstrInitialDir = set.game_directory;
        ofn.lpstrTitle = "Select Palette File";
        ofn.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;

        if(!GetOpenFileName(&ofn))
            sysfatal(const_cast<char *> ("You must specify a valid palette file for BSP..."));

        fn.SetPath(filename);
        raw = file_get_contents(fn);
    }

    if(!raw)
        return;

    // See if it has a JASC palette header...

    // Load in the raw data and tokenize it.
    Tokenizer script((char*)raw);
    if (!script.next(true))
    {
        syserror(const_cast<char *> ("Palette file [%s] had no tokens"), fn);
        goto err_continue;
    }

    // See if the first token is the pal header
    if (!strcmpi(script.token,"JASC-PAL"))
    {
        // Parse the first 2 tokens away
        if (!script.next(true))
        {
            syserror(const_cast<char *> ("Palette file [%s] should have more lines after JASC-PAL"), fn);
            goto err_continue;
        }
        if (!script.next(true))
        {
            syserror(const_cast<char *> ("Palette file [%s] should have more lines..."), fn);
            goto err_continue;
        }
    }
    else
    {
        // otherwise go back to beginning...
        script.begin();
    }

    // Read in the entries from the current parse point.
    for (int i = 0; i < 256; i++)
    {
        if (!script.next (true))    // Can go to next line
        {
            syserror(const_cast<char *> ("Expecting red palette entry [%i] line %d"), i, script.line);
            goto err_continue;
        }
        set.pal[3*i+0] = (unsigned char)atoi(script.token);
        if (!script.next (false))   // Should be on same line.
        {
            syserror(const_cast<char *> ("Expecting green palette entry [%i] line %d"), i, script.line);
            goto err_continue;
        }
        set.pal[3*i+1] = (unsigned char)atoi(script.token);
        if (!script.next (false))   // On same line still
        {
            syserror(const_cast<char *> ("Expecting blue palette entry [%i] line %d"), i, script.line);
            goto err_continue;
        }
        set.pal[3*i+2] = (unsigned char)atoi(script.token);
    }
err_continue:
    delete[] raw;

    // GAMMA CORRECT THE PALLETTE
    // FIXME, this code is reproduced in several spots, make into one function.
    //
    unsigned char gammatable[256];
    float gamma = set.gamma;
    if (gamma == 1.0f)
    {
        for (int i=0 ; i<256 ; i++)
            gammatable[i] = (unsigned char)i;
    }
    else
    {
        int inf;
        for (int i=0 ; i<256 ; i++)
        {
            inf = 255 * (float) pow( (float) ((i+0.5f)/255.5f), (float) gamma ) + 0.5f;
            inf = min(255,max(0,inf));
            gammatable[i] = (unsigned char)inf;
        }
    }

    for (int i = 0; i < 768; i++)
        set.pal[i] = gammatable[set.pal[i]];

    for (int i = 0; i < 256; i++)
    {
        set.cTable[i].rgbRed   = set.pal[3*i+0];
        set.cTable[i].rgbGreen = set.pal[3*i+1];
        set.cTable[i].rgbBlue  = set.pal[3*i+2];
        set.cTable[i].rgbReserved = 0;
    }

    // We track a few special indices so we don't have to
    //  look them up every time we use them.
    set.outlineIndex  = GetPaletteIndex(256,set.pal, set.color_brushoutline);
    set.selectedIndex = GetPaletteIndex(256,set.pal, set.color_selectoutline);
    set.faceIndex     = GetPaletteIndex(256,set.pal, set.color_faceoutline);
    set.lockIndex     = GetPaletteIndex(256,set.pal, set.color_lock);
}

void GetGameDialog()
{
    struct _finddata_t ffb;   // FindFirst block header.
    memset(&ffb,0,sizeof(ffb));
    //ffb.ff_attrib = FA_DIREC;   // Looking for directories (off of \bsp)

    char *path = set.main_dir;


    // Set up the transfer buffer listing game names.
    TGameBuffer x;
    x.opengl = set.glBsp;

    // Search for all directories.
    int ff = _findfirst(Path(const_cast<char *> ("%s/*.*"), path), &ffb);
    bool morefiles = ff != -1;
    while (morefiles)
    {
        // If the file is not a dir, or is the current dir then skip to the next file.
        if (!(ffb.attrib & _A_SUBDIR) || (ffb.name[0] == '.'))
        {
            morefiles=!_findnext(ff, &ffb);
            continue;
        }

        // See if game.cfg is in the directory, and if so, add to the transfer buffer list.
        if (Path(const_cast<char *> ("%s/%s/game.cfg"), set.main_dir,ffb.name).Exists())
        {
            // Select first item or Quake2 directory as default.
            bool sel = !x.dir.count || !strcmpi(ffb.name, "QUAKE2");
            x.dir.Add(ffb.name,sel);
        }
        morefiles=!_findnext(ff, &ffb);  // More files?
    }
    _findclose(ff);

    //no game configs found?
    if(x.dir.count <= 0)
        return;

    //show game dialog
    TGameDlg dialog(0,&x);
    if (dialog.Execute() != IDOK)
        return;

    set.glBsp = x.opengl;
    // Set the game_directory.
    sprintf(set.game_directory,"%s/%s", set.main_dir, x.selected);
}



/*
=====================
OwlMain -- oooh, its WinMain now :)

App. entry point.
=====================
*/

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
    //grab args from stdlib
    int argc = __argc;
    char** argv = __argv;

    hInstance = hInst;

    struct tm *newtime;
    time_t aclock;
    time(&aclock);
    newtime = localtime(&aclock);

    LogFileReset(const_cast<char *> (BSP_LOG));

    sysprintf(const_cast<char *> ("cBSP %d.%d"BSP_REV" (%d.%d)\r\n"), BSP_VER_MAJOR, BSP_VER_MINOR, _BUILDNUM, _BUILDREV);
    sysprintf(const_cast<char *> ("Started: %s\r\n"), asctime(newtime));	//asctime leaves in a \n. bleh.

    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_BAR_CLASSES | ICC_COOL_CLASSES;
    InitCommonControlsEx(&icc);

    //init settings system
    sysprintf(const_cast<char *> ("Registering settings\r\n"));
    Cset_Initialize();
    sysprintf(const_cast<char *> ("Loading setinfo.cfg\r\n"));
    SetInfo_Parse(const_cast<char *> ("settings/setinfo.cfg"));
    sysprintf(const_cast<char *> ("Loading bsp.cfg\r\n"));
    Cset_LoadConfig(const_cast<char *> ("settings/bsp.cfg"));

    if(*set.default_gamedir)
        sprintf(set.game_directory,"%s/%s",set.main_dir,set.default_gamedir);

    //fonts in different styles that will not change during the life of the process
    set.font_ui = GetFont(set.bsp_font,-set.bsp_font_size);
    set.font_ui_bold = GetFont(set.bsp_font,-set.bsp_font_size,0,800);
    set.font12 = GetFont(set.bsp_font,-set.bsp_font_size);
    set.font10 = GetFont(set.bsp_font,-10);
    set.font9 = GetFont(set.bsp_font,-9);

    // Set flags based on command line options.
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] != '-')
            break;
        else if (!strcmpi (argv[i],"-texalpha"))
            cTextureAlpha = 1;
        else if (!strcmpi (argv[i],"-single"))
            singleBuffer = 1;
        else if (!strcmpi (argv[i],"-s_1"))
            set.sinBsp = 1;
        else if (!strcmpi (argv[i],"-v_1"))
            set.sinBsp = set.valveBsp = 1;
        else if (!strcmpi (argv[i],"-gl"))
            set.glBsp = 1;
        else if (!strcmpi (argv[i],"-time"))
            timeRender = 1;
        else if (!strcmpi (argv[i],"-nosplash"))
            nosplash = 1;
        else if (!strcmp (argv[i],"-game"))
            ++i, sprintf(set.game_directory,"%s/%s",set.main_dir,argv[i]);
        else if (!strcmp (argv[i],"-colorbits"))
            ++i, cColorBits = atoi(argv[i]);
        else if (!strcmp (argv[i],"-depthbits"))
            ++i, cDepthBits = atoi(argv[i]);
        else
            sysfatal(const_cast<char *> ("BSP: Unknown option '%s'\n"
                                         "Usage:  %s [options] \n\t options are \n"
                                         "-game dirname : uses settings from game.cfg file in the dirname directory\n"
                                         "-gl\tRuns glBSP, the following options are for glBSP\n"
                                         "-texalpha\tToggles use of GL_RGBA texture style (vs. GL_RGB)\n"
                                         "-colorbits <num>\tSets number of color bits (try 16, 24, 32)\n"
                                         "-depthbits <num>\tSets number of z buffer bits (try 16 or 32)\n"
                                         "-single\tForces single buffering\n "
                                         "\n"
                                         "-nosplash\tHide the splash screen at startup..."), argv[i], argv[0]);
    }

    //setup palettes
    PALSTEP = 3;
    LUMP_NAME_LENGTH = 16;
    if (set.sinBsp)
    {
        if (!set.valveBsp)
        {
            PALSTEP = 4;
            LUMP_NAME_LENGTH = 32;
        }
    }


    //check bsp.cfg for gl mode
    if(set.gl_mode_default)
        set.glBsp = 1;

    // check that game_directory exists...
    char gamesettings[MAX_PATH];
    sprintf(gamesettings,"%s/game.cfg",set.game_directory);
    if (!file_exists(gamesettings))
    {
        // Put up dialog
        GetGameDialog();
        sprintf(gamesettings,"%s/game.cfg",set.game_directory);
        if (!file_exists(gamesettings))
        {
            sprintf(set.game_directory,"%s/quake2",set.main_dir);
            sprintf(gamesettings,"%s/game.cfg",set.game_directory);
            if (!file_exists(gamesettings))
            {
                syserror(const_cast<char *> ("Error: game.cfg not found in any subdirectory"));
                sysfatal(const_cast<char *> ("Couldn't find [%s], quitting..."), gamesettings);
            }
        }
    }
    //load game.cfg
    sysprintf(const_cast<char *> ("Loading game.cfg\r\n"));
    Cset_LoadConfig(gamesettings);
    set.game_mode = set.game;

    // Set clipping sizes.
    SetRect(&EditClip, ORIG_XMIN , ORIG_YMIN, ORIG_XMAX, ORIG_YMAX);
    SetRect(&RenderClip, ORIG_XMIN , ORIG_YMIN, ORIG_XMAX, ORIG_YMAX);
    EditSize.cx = ORIG_EDIT_H;
    EditSize.cy = ORIG_EDIT_V;
    RenderSize.cx = ORIG_HORIZ;
    RenderSize.cy = ORIG_VERT;

    //initialize built-in bsp commands
    sysprintf(const_cast<char *> ("Registering commands\r\n"));
    Ccmd_Initialize();	//new commands .. . .. :D
    sysprintf(const_cast<char *> ("Loading cmdstr.cfg\r\n"));
    Ccmd_LoadCommandDef(const_cast<char *> ("settings/cmdstr.cfg"));

    // load game settings
    InitGameSettings();

    // load accels
    sysprintf(const_cast<char *> ("Loading keyboard.cfg\r\n"));
    kbset = new KBSettings;
    kbset->LoadFromDisk(const_cast<char *> ("settings/keyboard.cfg"));	// next, override from saved settings
    acceltable = kbset->CreateAccelTable();		// build new accel table

    // create app window
    frame = new TBSPApp("cBSP");
    frame->Create();

    // load main menu
    sysprintf(const_cast<char *> ("Loading menu config\r\n"));
    Menu_Initialize();
    Menu_LoadConfig(const_cast<char *> ("settings/menu.cfg"));
    CmMenuRefresh();

    //enable dragdrop for files
    DragAcceptFiles(frame->hwnd, TRUE);

    //
    // message loop
    //
    MSG msg;
    BOOL gmret;

    // RUN IT.
    while(gmret=GetMessage(&msg,0,0,0))
    {
        if(gmret==-1)
        {
            syserror(const_cast<char *> ("GetMessage failed with GLE: %d"), GetLastError());
            break;
        }
        if(FindReplace && FindReplace->hwnd && IsDialogMessage(FindReplace->hwnd, &msg))	// for findreplace dlg
            continue;

        if(	KillAccels || 											        // disable accelerator keys?
                !client || (											    // need client
                    !TranslateMDISysAccel(client->hwnd, &msg) &&			// sysmenu shortcuts
                    !TranslateAccelerator(frame->hwnd,acceltable,&msg)))	// bsp commands
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    //disable dragdrop
    DragAcceptFiles(frame->hwnd, FALSE);

    // shutdown
    Cset_Unload();
    Ccmd_Unload();
    Menu_DestroyMenus();	//destroy menu list and all menus
    DestroyAcceleratorTable(acceltable);
    delete frame;
    return 0;
}
