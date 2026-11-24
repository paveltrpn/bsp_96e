#include "global.h"

char Path::_curdir[300] = "";

//
// GetFont
//
HFONT GetFont(const char *fontname,int fontsize,int width,int weight)
{
    if(fontname && (
                !_stricmp(fontname,"terminal") ||
                !_stricmp(fontname,"fixedsys") ||
                !_stricmp(fontname,"courier new"))
      )
        return CreateFont(fontsize,0,0,0,0,0,0,0,OEM_CHARSET ,0,0,0,FIXED_PITCH,fontname);
    else
        return CreateFont(fontsize,width,0,0,weight,0,0,0,0,OUT_CHARACTER_PRECIS,0,PROOF_QUALITY,0,fontname);
}
Path::Path(char *text, ...) : valid(false)
{
    va_list vargs;
    va_start(vargs, text);
    _setpath(text, vargs);
    va_end(vargs);
}
void Path::SetPath(char *text, ...)
{
    va_list vargs;
    va_start(vargs, text);
    _setpath(text, vargs);
    va_end(vargs);
}
void Path::ChangeDir(Path p)
{
    strncpy(Path::_curdir,p,sizeof(Path::_curdir));
}
void Path::_setpath(char*text,va_list args)
{
    *_path = 0;
    valid = false;
    int len = _vscprintf(text, args);
    if(len > 0)
    {
        char out[300];
        _vsnprintf (out, len, text, args);
        out[len] = 0;
        SetCurrentDirectory(Path::_curdir);
        valid = 0 != GetFullPathName(out,sizeof(_path),_path,0);
    }
}
bool Path::Exists()
{
    return valid && -1 != _access(_path, 0);
}
//returns zero-len str on failure so the pointer is always valid.
char *Path::ext()
{
    char *retval = const_cast<char *> ("");
    char *c = _path;
    while(*c)
    {
        if(*c=='/' ||*c=='\\') retval = const_cast<char *> ("");
        else if(*c=='.' && *(c+1)) retval = c+1;
        c++;
    }
    return retval;
}
Path::operator char*()
{
    return (char*)_path;
}
char *Path::operator =(char*rhs)
{
    _setpath(const_cast<char *> ("%s"),rhs);
    return (char*)_path;
}

//
// ColorDialog
//

ColorDialog::ColorDialog(HWND owner,COLORREF defcolor,char *title)
{
    memset(&cc,0,sizeof(CHOOSECOLOR));
    cc.lStructSize = sizeof(CHOOSECOLOR);
    cc.Flags = CC_RGBINIT|CC_FULLOPEN;
    cc.hwndOwner = owner;
    cc.lpCustColors = custColors;
    cc.rgbResult = result = defcolor;
    cc.lCustData = (LPARAM)this;
    if(title)  	//set hook only if we need to set the caption
    {
        STRNCPY(caption, title);
        cc.Flags |= CC_ENABLEHOOK;
        cc.lpfnHook = (LPCCHOOKPROC)ColorDialog::CCHookProc;
    }
}
COLORREF ColorDialog::custColors[16] =
{
    RGB(0,0,0),		  RGB(255,255,255), RGB(128, 128, 128), RGB(255,0,0),
    RGB(0,255,0),	  RGB(0, 0, 255),   RGB(255,128,0),		RGB(128,255,0),
    RGB(128,0,255),	  RGB(255,0,128),   RGB(0,255,128),		RGB(0, 128, 255),
    RGB(255,128,128), RGB(128,255,128), RGB(128,128,255),	RGB(255,255,255)
};
UINT_PTR * ColorDialog::CCHookProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if(msg == WM_INITDIALOG)
    {
        ColorDialog *cd = (ColorDialog *)(((LPCHOOSECOLOR)lParam)->lCustData);
        SetWindowText(hwnd,(char *)&cd->caption);
    }
    return 0;
}
bool ColorDialog::Show()
{
    bool ret = !!ChooseColor(&cc);
    if(ret)
        result = cc.rgbResult;
    return ret;
}

//
// WProfile
//
WProfile::WProfile(const char *section,const char *inifile)
{
    Path file((char*)inifile);
    strncpy(this->section,section,sizeof(this->section));
    this->section[sizeof(this->section) - 1] = 0;
    strncpy(this->inifile,file,sizeof(this->inifile));
    this->inifile[sizeof(this->inifile) - 1] = 0;
}
WProfile::~WProfile()
{
}
int WProfile::GetInt(const char *key,int defvalue)
{
    return GetPrivateProfileInt(section,key,defvalue,inifile);
}
bool WProfile::GetString(const char *key,char *buffer,int size,const char *defvalue)
{
    return !!GetPrivateProfileString(section,key,defvalue,buffer,size,inifile);
}
COLORREF WProfile::GetColor(const char *key, COLORREF defvalue)
{
    char temp[40];
    GetString(key, temp, sizeof(temp));

    COLORREF retval = Parse_Color(temp);
    if(-1 != retval)
        return retval;
    return defvalue;
}
bool WProfile::WriteInt(const char *key,int value)
{
    char buf[20];
    sprintf(buf,"%d",value);
    return !!WritePrivateProfileString(section,key,buf,inifile);
}
bool WProfile::WriteString(const char *key,const char *value)
{
    return !!WritePrivateProfileString(section,key,value,inifile);
}
bool WProfile::WriteColor(const char *key, const COLORREF value)
{
    char temp[40];
    snprintf(temp,sizeof(temp),"%d %d %d", GetRValue(value), GetGValue(value), GetBValue(value));
    return WriteString(key, temp);
}

//
// WImageList
//
WImageList::WImageList() : imbig(0), imsmall(0), head(0)
{
    imbig = ImageList_Create(20,20,ILC_MASK |ILC_COLOR24,200,10);					//20x20 toolbar
    imsmall = ImageList_Create(16,15, ILC_MASK |ILC_COLOR24,200,10);	//16x15 mdi child toolbars
    memset(hash,0,sizeof(hash));
}
WImageList::~WImageList()
{
    if(imbig) ImageList_Destroy(imbig);
    if(imsmall) ImageList_Destroy(imsmall);
    imbig = imsmall = 0;
    //clear hashlist
    wil_data_t *tmp;
    for(wil_data_t *w = head; w; w = tmp)
    {
        tmp = w->next;
        delete w;
    }
}
int WImageList::hash_key(int cmd_id)
{
    return cmd_id % WIL_HASH_SIZE;
}
wil_data_t *WImageList::find_img(int cmd_id)
{
    int key = hash_key(cmd_id);
    wil_data_t *w = hash[key];
    for( ; w; w = w->hash_next)
        if( w->cmd == cmd_id)
            return w;
    return 0;
}
//return image id, or -1
int WImageList::add_img(HIMAGELIST im,HBITMAP bmp,int cmd_id)
{
    if(find_img(cmd_id))
        return -1;	//error, already exists
    int id = ImageList_AddMasked(im,bmp,GetSysColor(COLOR_3DFACE));
    if(id == -1)
    {
        return -1;	//error, add failed
    }

    wil_data_t *c = new wil_data_t;
    c->cmd = cmd_id;
    c->him = im;
    c->index = id;
    c->next = head;
    head = c;
    int key = hash_key(cmd_id);
    c->hash_next = hash[key];
    hash[key] = c;
    return id;
}
//returns new image id, or -1
int WImageList::Add(char *bmp,int cmd_id)
{
    //load bmp
    HBITMAP hbmp = load_bitmap(bmp,LB_3DCOLORS);
    if(!hbmp)
        return -1;
    //get size
    BITMAP bm;
    int ret = -1;
    if(!find_img(cmd_id))
    {
        GetObject(hbmp, sizeof (BITMAP), &bm);
        if(bm.bmWidth==WIL_SMALL_CX && bm.bmHeight == WIL_SMALL_CY)
        {
            //small image
            ret = add_img(imsmall,hbmp,cmd_id);
        }
        else if(bm.bmWidth==WIL_BIG_CX && bm.bmHeight == WIL_BIG_CY)
        {
            //big image
            ret = add_img(imbig,hbmp,cmd_id);
        }
        else
            syserror(const_cast<char *> ("image dimensions are invalid: %s"),bmp);
    }
    DeleteObject(hbmp);
    return ret;
}
int WImageList::GetImage(int cmd_id)
{
    wil_data_t *w = find_img(cmd_id);
    if(w)
        // just return index... assume whoever is using this
        // knows which image list it is for.
        return w->index;
    return -1;
}

//
// Popup
//
Popup::Popup()
{
    hwnd = 0;
    flags = 0;
    GetCursorPos(&pt);
    menu = CreatePopupMenu();
}
Popup::~Popup()
{
    DestroyMenu(menu);
}
void Popup::Add(const char *text,int id,bool check)
{
    if(check)
        Check();
    AppendMenu(menu,MF_STRING | flags,id,text);
    lastid = id;
    flags = 0;
}
void Popup::Add(const char *text,Popup submenu)
{
    AppendMenu(menu, MF_STRING|MF_POPUP|flags, (UINT_PTR)submenu.menu, text);
    lastid = 0;
    flags = 0;
}
void Popup::Separator()
{
    AppendMenu(menu,MF_SEPARATOR,0,0);
}
void Popup::Show(HWND hwnd)
{
    TrackPopupMenu(menu,0,pt.x,pt.y,0,hwnd,0);
}
void Popup::Check()
{
    flags |= MF_CHECKED;
}
void Popup::Gray()
{
    flags |= MF_GRAYED;
}

//
// WScroll
//
WScroll::WScroll(HWND owner)
{
    hwnd = owner;
    memset(&X,0,sizeof(SCROLLINFO));
    memset(&Y,0,sizeof(SCROLLINFO));
    X.cbSize = Y.cbSize = sizeof(SCROLLINFO);
}
void WScroll::Update()
{
    X.fMask = Y.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS | SIF_DISABLENOSCROLL;
    SetScrollInfo(hwnd, SB_HORZ, &X, TRUE);
    SetScrollInfo(hwnd, SB_VERT, &Y, TRUE);
}
void WScroll::Sync()
{
    X.fMask = Y.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
    GetScrollInfo(hwnd, SB_HORZ, &X);
    GetScrollInfo(hwnd, SB_VERT, &Y);
}
//returns true when needs update
bool WScroll::VScroll(WPARAM wParam, int pagesize)
{
    switch(LOWORD(wParam))
    {
    case SB_PAGEDOWN:
        Y.nPos = min(Y.nMax, Y.nPos + pagesize);
        break;
    case SB_LINEDOWN:
        Y.nPos = min(Y.nMax, Y.nPos + 1);
        break;
    case SB_PAGEUP:
        Y.nPos = max(0, Y.nPos - pagesize);
        break;
    case SB_LINEUP:
        Y.nPos = max(0, Y.nPos - 1);
        break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        Y.nPos = (short) HIWORD(wParam);
        break;
    default:		//unhandled ones exit
        return false;
    }
    //update for handled cases
    if(Y.nPos > Y.nMax - (int)Y.nPage)
        Y.nPos = max(0, Y.nMax - (int)Y.nPage + 1);
    return true;
}

//
// linked list class to pass data to dialog listboxes.
//
ListData::ListData()
{
    reset_ptrs();
}
ListData::~ListData()
{
    clear_items();
}
void ListData::Add(const char *str, bool sel)
{
    ListDataItem *item = new ListDataItem;	// create new item
    item->str = new char[strlen(str) + 1];	// alloc strlen+null
    strcpy(item->str, str);			// copy string
    item->sel = sel;				// set selected
    item->next = 0;					// be sure next is NULL
    *tail = item;					// add item to list
    tail = &item->next;				// update tail
    count++;
}
void ListData::Clear()
{
    clear_items();
    reset_ptrs();
}
void ListData::clear_items()
{
    //delete all items in list
    ListDataItem *item = items, *tmp;
    count = 0;
    while(item)
    {
        tmp = item->next;			// save ptr to next item
        delete[] item->str;
        delete item;
        item = tmp;					// get next item
    }
}
void ListData::reset_ptrs()
{
    items = 0;
    count = 0;
    tail = &items;
    cur = &items;
}
bool ListData::hasNext()
{
    return !!*cur;					// just return if cur is NULL or not
}
const char *ListData::Next()
{
    ListDataItem *data = *cur;
    if(!data)
        return "ERROR";
    *cur = data->next;				// update cur to next item
    return data->str;				// return string data
}
//fills a listbox. independent of iterator.
void ListData::FillList(HWND listbox)
{
    SendMessage(listbox,LB_RESETCONTENT,0,0);
    ListDataItem *d = items;
    while(d)
    {
        const int idx = (int) SendMessage(listbox, LB_ADDSTRING, 0, (LPARAM) d->str);
        if(d->sel)
            SendMessage(listbox,LB_SETCURSEL,idx,0);
        d = d->next;
    }
}
//fills a combo box. independent of iterator.
void ListData::FillCombo(HWND combobox)
{
    SendMessage(combobox,CB_RESETCONTENT,0,0);
    ListDataItem *d = items;

    while(d)
    {
        const int idx = (int) SendMessage(combobox, CB_ADDSTRING, 0, (LPARAM) d->str);
        if(d->sel || !idx)
            SendMessage(combobox,CB_SETCURSEL,idx,0);
        d = d->next;
    }
}



//
// WReBar
//
WRebar::WRebar(HWND parent)
{

    RECT rc;
    GetWindowRect(parent, &rc);
    const int width = rc.right-rc.left;

    hwnd = CreateWindow( REBARCLASSNAME, 0, /*WS_BORDER|*/WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|
                         WS_CLIPSIBLINGS| RBS_VARHEIGHT|CCS_NODIVIDER|CCS_NOPARENTALIGN,
                         0,0,width,400, parent, (HMENU)ID_REBAR, hInstance,0);

    if(!hwnd)
    {
        syserror(const_cast<char *> ("Couldn't create rebar! GLE: %d"), GetLastError());
        return;
    }

    REBARINFO rbi;
    rbi.cbSize = sizeof(rbi);
    rbi.fMask  = 0;
    rbi.himl   = 0;
    SendMessage(hwnd, RB_SETBARINFO, 0, (LPARAM)&rbi);
}
WRebar::~WRebar()
{
    DestroyWindow(hwnd);
}
void WRebar::AddBand(HWND child)
{
    REBARBANDINFO rbBand;
    memset(&rbBand,0,sizeof(rbBand));
    rbBand.cbSize = sizeof(rbBand);
    rbBand.fMask = RBBIM_CHILD | RBBIM_STYLE | RBBIM_CHILDSIZE | RBBIM_SIZE | RBBIM_IDEALSIZE;

    rbBand.fStyle = RBBS_BREAK/*|RBBS_GRIPPERALWAYS*/|RBBS_CHILDEDGE;
    rbBand.hwndChild = child;
    RECT r;
    GetWindowRect(child, &r);

    rbBand.cxIdeal =
        rbBand.cx = 500;

    rbBand.cxMinChild = TB_BMP_WIDTH;
    rbBand.cyMinChild =
        rbBand.cyChild = TB_BMP_HEIGHT+6;

    SendMessage(hwnd, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);	//add band
}

void WRebar::Show(int index)
{
    SendMessage(hwnd,RB_SHOWBAND,index,TRUE);
}
void WRebar::Hide(int index)
{
    SendMessage(hwnd,RB_SHOWBAND,index,FALSE);
}


//
// WToolBar
//

WToolbar::WToolbar(HWND parent, int bmpcx, int bmpcy , int btncx, int btncy)
{
    hwnd = CreateWindow(TOOLBARCLASSNAME, 0, TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | WS_CHILD |
                        CCS_NORESIZE |WS_GROUP | CCS_NODIVIDER | WS_VISIBLE,
                        0, 0, 0, 0, parent, (HMENU)0, hInstance, 0);
    SendMessage(hwnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hwnd, TB_SETBITMAPSIZE, 0, MAKELONG(bmpcx, bmpcy));
    SendMessage(hwnd, TB_SETBUTTONSIZE, 0, MAKELONG(btncx, btncy));
    if(!hwnd)
    {
        syserror(const_cast<char *> ("Couldn't create toolbar band! GLE: %d"), GetLastError());
    }

    visible = true;	// visible when created
    index = -1;		// index not initialized
}

WToolbar::~WToolbar()
{
    DestroyWindow(hwnd);
}

void WToolbar::AddSeparator()
{
    TBBUTTON tbb;
    memset(&tbb,0,sizeof(tbb));
    tbb.fsStyle = TBSTYLE_SEP;
    SendMessage(hwnd, TB_ADDBUTTONS, 1, (LPARAM)&tbb);
}

void WToolbar::AddButton(int imgId,int cmdId)
{
    TBBUTTON tbb;
    tbb.iBitmap = imgId; //tbimg;
    tbb.idCommand = cmdId;
    tbb.fsState = TBSTATE_ENABLED;
    tbb.fsStyle = TBSTYLE_BUTTON;
    tbb.dwData = 0;
    tbb.iString = 0;
    SendMessage(hwnd, TB_ADDBUTTONS, 1, (LPARAM)&tbb);
    SendMessage(hwnd, TB_AUTOSIZE, 0, 0);
}

void WToolbar::AddControl(HWND hwndchild)
{

    const int dlgItemId = GetDlgCtrlID(hwndchild);

    RECT r;
    GetWindowRect(hwndchild, &r);
    const int width = r.right - r.left;

    //add filler item
    TBBUTTON tbb;
    memset(&tbb,0,sizeof(tbb));
    tbb.iBitmap = -1;
    tbb.idCommand = dlgItemId;
    SendMessage(hwnd, TB_ADDBUTTONS, 1, (LPARAM)&tbb);

    // set button width
    TBBUTTONINFO tbi;
    tbi.cbSize = sizeof(tbi);
    tbi.dwMask = TBIF_SIZE;
    tbi.cx = width + TB_CTRL_PADDING*2;
    SendMessage(hwnd, TB_SETBUTTONINFO, dlgItemId, (LPARAM)&tbi);

    //get button bounds
    ::SendMessage(hwnd, TB_GETRECT, dlgItemId, (LPARAM) &r);

    // move child window over button
    SetWindowPos(hwndchild,0,r.left+TB_CTRL_PADDING,r.top,0,0,SWP_NOZORDER|SWP_NOSIZE);
}



//
// WStatus
//

//WStatus::WStatus
WStatus::WStatus(HWND parent, int numParts)
    : parent(parent), font(0), sbmp(0), sdc(0), bmp_strip(0), visible(true)
{
    this->numParts = max(1,numParts);
    parts = new partinfo_t[this->numParts];
//	memset(parts,0,sizeof(parts));	//doesnt do anything?
    for(int i=0; i<this->numParts; i++)
    {
        parts[i].bmpid = -1;
        *parts[i].text = 0;
    }
}
//WStatus::~WStatus
WStatus::~WStatus()
{
    delete [] parts;
    if(font) DeleteObject(font);
    if(bmp_strip) DeleteObject(bmp_strip);
    RemoveMemDC();
}
void WStatus::ConstructMemDC()
{
    RemoveMemDC();
    HDC hdc = GetDC(parent);
    sdc = CreateCompatibleDC(hdc);
    sbmp = CreateCompatibleBitmap(hdc,sbrc.right-sbrc.left, sbrc.bottom-sbrc.top);
    SelectObject(sdc,sbmp);
    ReleaseDC(parent, hdc);
}
void WStatus::RemoveMemDC()
{
    if(sdc) DeleteDC(sdc);
    if(sbmp) DeleteObject(sbmp);
}
//rebuild memdc if size has changed
void WStatus::CheckMemDC()
{
    BITMAP b;
    GetObject(sbmp,sizeof(b),&b);
    if(b.bmWidth != sbrc.right-sbrc.left || b.bmHeight != sbrc.bottom-sbrc.top)
        ConstructMemDC();
}
//WStatus::IsVisible
bool WStatus::IsVisible()
{
    return visible;
}
//WStatus::Show
void WStatus::Show(bool visible, bool redraw)
{
    this->visible = visible;
    if(redraw) Redraw();
}
//WStatus::GetRect
void WStatus::GetRect(RECT *rc)
{
    CopyRect(rc, &sbrc);
}
//WStatus::GetPartRect
void WStatus::GetPartRect(int index, RECT *rc)
{
    if(index<0 || index>=numParts)
        return;
    CopyRect(rc, &sbrc);
    rc->left = parts[index].left;
    rc->right = rc->left + parts[index].cx;
}
//WStatus::SetFont
void WStatus::SetFont(HFONT font)
{
    this->font = font;
}
//WStatus::SetPart
void WStatus::SetPart(int index, int width, bool charwidth, bool ownerdraw)
{
    if(index<0 || index>=numParts)
        return;
    parts[index].ownerdraw = ownerdraw;
    if(!charwidth)
    {
        parts[index].cx = width;			// set width in pixels
    }
    else
    {
        SIZE sz;
        HDC hdc = GetDC(parent);
        HGDIOBJ oldfont = SelectObject(hdc, font);
        GetTextExtentPoint32(hdc,"W",1,&sz);
        SelectObject(hdc, oldfont);
        ReleaseDC(parent,hdc);

        parts[index].cx = width * sz.cx;	// set width in characters
    }
}
//WStatus::SizePartToText
void WStatus::SizePartToText(int index)
{
    if(index<0 || index>=numParts)
        return;
    SIZE sz;
    HDC hdc = GetDC(parent);
    HGDIOBJ oldfont = SelectObject(hdc, font);
    GetTextExtentPoint32(hdc, parts[index].text, (int) strlen(parts[index].text), &sz);
    SelectObject(hdc, oldfont);
    ReleaseDC(parent,hdc);

    SetPart(index, sz.cx+8);
    Redraw();
}
//WStatus::SetPartBmp
void WStatus::SetPartBmp(int index, int bmpid)
{
    if(index<0 || index>=numParts)
        return;
    parts[index].bmpid = bmpid;
}
//WStatus::SetBmp
void WStatus::SetBmp(HBITMAP bmp, int piece_width)
{
    if(bmp_strip) DeleteObject(bmp_strip);
    bmp_strip = bmp;
    bmp_cx = piece_width;
}
//WStatus::SetText
void WStatus::SetText(int index, const char *text, bool redraw)
{
    if(index<0 || index>=numParts)
        return;
    STRNCPY(parts[index].text,text);
    if(redraw) Redraw();
}
void WStatus::SetText(const char *text, bool redraw)
{
    SetText(0, text, redraw);
}
//WStatus::Redraw
void WStatus::Redraw()
{
    //InvalidateRect(parent, &sbrc, true);
    DrawStatusBar();
}
//WStatus::DrawStatusBar
void WStatus::DrawStatusBar()
{

    GetClientRect(parent, &sbrc);
    sbrc.top = sbrc.bottom - SB_HEIGHT;

    CheckMemDC();	//recreates sdc/sbmp if sbrc size changed

    //set rect to minimum size if hidden
    if(!visible)
    {
        sbrc.top = sbrc.bottom;
        return;
    }
    RECT rc, part, dest;

    //dest rect needs to be relative to the bitmap, which has a top left pt at 0,0
    CopyRect(&dest, &sbrc);
    OffsetRect(&dest, -sbrc.left, -sbrc.top);

    FillRect(sdc, &dest, GetSysColorBrush(COLOR_BTNFACE));

    //set part rect, top and bottom
    rc.top = dest.top + 2;
    rc.bottom = dest.bottom;
    //set left and right for rightmost fixed part
    rc.left = dest.right - parts[numParts-1].cx;
    rc.right = dest.right;


    //draw fixed-size parts
    for(int i=numParts-1; i > 0; i--)
    {
        parts[i].left = rc.left;

        if(!parts[i].ownerdraw) FrameRect(sdc, &rc, GetSysColorBrush(COLOR_BTNSHADOW));
        rc.left -= parts[i-1].cx + SB_SPACE;
        rc.right -= parts[i].cx + SB_SPACE;
    }
    //draw leftmost autosize part
    rc.left = dest.left;
    parts[0].left = dest.left;
    FrameRect(sdc, &rc, GetSysColorBrush(COLOR_BTNSHADOW));

    //set
    if(numParts>1)
        part.right = parts[0].left + parts[1].left - SB_SPACE;	// edge of next part
    else
        part.right = dest.right;	// edge of screen


    HDC hdc = GetDC(parent);

    //test
    SetTextColor(sdc, GetSysColor(COLOR_BTNTEXT));
    SetBkMode(sdc, TRANSPARENT);
    HGDIOBJ oldfont = SelectObject(sdc, font);
    for(int i=0; i<numParts; i++)
    {

        part.left = parts[i].left;

        //dont set right for first part
        if(i) part.right = parts[i].left + parts[i].cx;

        part.top = dest.top+2;
        part.bottom = dest.bottom;

        if(parts[i].bmpid < 0)
        {
            part.left += 5;	//add padding for text

            DrawText(sdc, parts[i].text, (int) strlen(parts[i].text), &part, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
        }
        else
        {
            HDC cdc = CreateCompatibleDC(hdc);
            SelectObject(cdc,bmp_strip);
            BitBlt(sdc,
                   part.right - (part.right-part.left)/2 - bmp_cx/2,
                   part.top,
                   bmp_cx,
                   part.bottom - part.top,
                   cdc,parts[i].bmpid*bmp_cx, 0, SRCCOPY);
            DeleteDC(cdc);
        }
    }
    SelectObject(sdc, oldfont);

    //blit to parent
    BitBlt(hdc,sbrc.left,sbrc.top,sbrc.right-sbrc.left,sbrc.bottom-sbrc.top,sdc,0,0,SRCCOPY);
    ReleaseDC(parent, hdc);
}



//
// Keyboard Settings
//

KBSettings::KBSettings() : keys(0) {}

//copy constructor
KBSettings::KBSettings(const KBSettings& kbs)
{
    keys = 0;
    KBAccel_t *k = kbs.keys, **dest = &keys;
    while(k)
    {

        *dest = new KBAccel_t;
        memcpy(*dest,k,sizeof(ACCEL));
        (*dest)->next = 0;

        dest = &(*dest)->next;
        k=k->next;
    }
}

void KBSettings::CopyKeys(const KBSettings& source)
{

    ClearKeys();

    KBAccel_t *k = source.keys, **dest = &keys;
    while(k)
    {

        *dest = new KBAccel_t;
        memcpy(*dest,k,sizeof(ACCEL));
        (*dest)->next = 0;

        dest = &(*dest)->next;
        k=k->next;
    }
}

KBSettings::~KBSettings()
{
    ClearKeys();
}
void KBSettings::ClearKeys()
{

    KBAccel_t *k = keys;
    while(k)
    {
        KBAccel_t *tmp = k->next;
        delete k;
        k = tmp;
    }
    keys = 0;
}
//this is gone because there are no more accels in the resource!
// load *keys from an existing table
/*void KBSettings::LoadAccels(HACCEL accel) {
	int count = CopyAcceleratorTable(accel,0,0);
	ACCEL *tbl = new ACCEL[count];
	CopyAcceleratorTable(accel,tbl,count);
	for(int i=0;i < count;i++) {
		AddAccel(tbl[i]);
	}
	delete[] tbl;
}
*/
// create table from *keys
HACCEL KBSettings::CreateAccelTable()
{
    int count = 0;
    KBAccel_t *k = keys;
    while(k)
    {
        count++;
        k=k->next;
    }
    ACCEL *tbl = new ACCEL[count];
    k = keys;

    int i=0;
    for(k=keys; k && i < count; i++, k=k->next)
    {
        memcpy(&tbl[i], &k->key, sizeof(ACCEL));
    }

    HACCEL ret = CreateAcceleratorTable(tbl, i);
    delete[] tbl;
    return ret;
}
// find key in *keys
ACCEL *KBSettings::FindAccel(ACCEL a, bool matchkey, bool matchcmd)
{
    KBAccel_t *k = keys;
    //CTRL+X can be ascii or virtual... so we need to check both kinds of codes
    bool ascii_ctrl = false, virt_ctrl = false;
    if(matchkey)
    {
        if(a.fVirt)	 		//input is virtual
        {
            if(a.fVirt & FCONTROL && !(a.fVirt & (FSHIFT|FALT)) )
                ascii_ctrl = true;
        }
        else  				//input is ascii
        {
            if(a.key < 0x20)
                virt_ctrl = true;
        }
    }
    for( ; k; k=k->next)
    {
        if(matchkey && (k->key.fVirt!=a.fVirt || k->key.key!=a.key))
        {
            if(ascii_ctrl)  	//try finding matching ascii key?
            {
                if(k->key.fVirt & FVIRTKEY || toupper(a.key) != (toupper(k->key.key)+0x40))
                    continue;
            }
            else if(virt_ctrl)  	//try finding a virtual key version??
            {
                if(!(k->key.fVirt & FCONTROL) || k->key.fVirt & (FALT|FSHIFT) || a.key+0x40 != toupper(k->key.key) )
                    continue;
            }
            else
                continue;
        }
        if(matchcmd && k->key.cmd != a.cmd)
            continue;
        return &k->key;			// found
    }
    return (ACCEL*) 0;
}
// add key to *keys
void KBSettings::AddAccel(ACCEL k)
{
    KBAccel_t *key = new KBAccel_t;
    memcpy(&key->key,&k,sizeof(ACCEL));
    key->next = keys;
    keys = key;
}

bool KBSettings::AddReplaceAccel(ACCEL *k)
{
    ACCEL *existing = FindAccel(*k, true, false);

    if(existing)
    {
        if(WORD(k->cmd) <= 0)
        {
            RemoveAccel(*existing);			// unset key
        }
        else
        {
            memcpy(existing, k, sizeof(ACCEL));	// copy over existing key
            existing->cmd = k->cmd;					// set command
        }
        return true;
    }
    else  							// must not have "None" selected
    {
        if(WORD(k->cmd) > 0)
        {
            AddAccel(*k);				// add new key
            return true;
        }
    }
    return true;
}

// remove by shortcut
bool KBSettings::RemoveAccel(ACCEL rm)
{
    //case 0
    if(keys->key.fVirt==rm.fVirt && keys->key.key==rm.key)
    {
        KBAccel_t *tmp = keys->next;
        delete keys;
        keys = tmp;
        return true;
    }

    //case 1 ...n
    KBAccel_t *k = keys;
    while(k && k->next)
    {
        if(k->next->key.fVirt==rm.fVirt && k->next->key.key==rm.key)
        {
            KBAccel_t *tmp = k->next->next;
            delete k->next;
            k->next = tmp;
            return true;
        }
        k = k->next;
    }
    return false;
}
void KBSettings::OverwriteAndActivate(KBSettings *dest_kbset)
{

    KillAccels = true;								// just in case? if necessary, are there threading issues here?
    dest_kbset->ClearKeys();						// clear dest
    DestroyAcceleratorTable(acceltable);			// active accel table must be destroyed...
    dest_kbset->CopyKeys(*this);					//
    acceltable = dest_kbset->CreateAccelTable();	// replace accel table
    KillAccels = false;								//
}

// return string of all shortcuts used by cmd
void KBSettings::ShowAccelsForCommand(int cmd, char *out, int outlen)
{
    *out = 0;
    int remaining = outlen;
    KBAccel_t *k = keys;
    while(k)
    {
        if(k->key.cmd==cmd)
        {
            if(*out)
            {
                strncat(out, ", ", remaining);
                remaining -= 2;
            }
            char keystr[128];
            KBSettings::AccelToText(k->key, keystr);
            int keylen = (int) strlen(keystr);

            strncat(out, keystr, remaining);
            remaining -= keylen;

            if(remaining <= 1)
                break;
        }
        k=k->next;
    }
}
// NOTE: only virtual keys are handled fully. try to avoid using ascii codes for accelerators
// unless they are CTRL+[A-Z] codes. Virtual codes and ascii codes can overlap, and if they do
// overlap then commands wont work when it cant locate the correct key code.
// use ascii for CTRL+[A-Z] only, and virtual for everything else.
char* KBSettings::AccelToText(ACCEL k, char* buf)
{
    if(buf)
    {
        *buf = 0;
        if(k.fVirt & FCONTROL)	strcpy(buf,"Ctrl+");
        if(k.fVirt & FALT)		strcat(buf,"Alt+");
        if(k.fVirt & FSHIFT)	strcat(buf,"Shift+");

        if(k.fVirt & FVIRTKEY)
        {
            char *t=GetVKeyName(k.key);
            if(t)
            {
                strcat(buf,t);				// use virtkey name
            }
            else
            {
                char b[2] = { (char) k.key, 0 };	// use character
                strcat(buf,b);
            }
        }
        else
        {
            char b[32];
            if(k.key < 0x20)  				// ascii codes under 0x20 are actually ctrl+letter
            {
                sprintf(b,"Ctrl+%c",k.key + 0x40);			// a=0x01,b=0x02 etc
            }
            else
            {
                sprintf(b,"%c",k.key);						// regular ascii char
            }
            strcat(buf,b);
        }
    }
    return buf;
}

//convert accel to config file format
char* KBSettings::AccelToCfgText(ACCEL k, char* buf)
{
    if(buf)
    {
        *buf = 0;

        if(k.fVirt & FCONTROL)	strcpy(buf,"Ctrl+");
        if(k.fVirt & FALT)		strcat(buf,"Alt+");
        if(k.fVirt & FSHIFT)	strcat(buf,"Shift+");

        //no modifier keys
        if(!*buf)				strcpy(buf,"none ");

        // overwrite last character with space
        buf[strlen(buf)-1] = 0x20;

        if(k.fVirt & FVIRTKEY)
        {
            char *t=GetVKeyName(k.key);
            if(t)
            {
                strcat(buf,t);				// use virtkey name
            }
            else
            {
                char b[2] = { (char) k.key, 0 };	// use character
                strcat(buf,b);
            }
        }
        else
        {
            char b[32];
            if(k.key < 0x20)  				// ascii codes under 0x20 are actually ctrl+letter
            {
                sprintf(b,"^%c",k.key + 0x40);				// a=0x01,b=0x02 etc
            }
            else
            {
                sprintf(b,"%c",k.key);						// regular ascii char
            }
            strcat(buf,b);
        }

        strcat(buf," ");

        //char *strcmd = BspCmd->GetName(k.cmd);
        ccmd_t *c = Ccmd_FindCmd(k.cmd);
        if(!c)  	//!strcmd || !*strcmd) {
        {
            *buf = 0;
        }
        else
        {
            strcat(buf, c->name);
        }
    }
    return buf;
}

bool KBSettings::ParseCfgText(char *mod, char *key, char*cmd, ACCEL *ret)
{

    ret->fVirt = FNOINVERT;

    //parse shortcut
    if(!strcmpi(mod,"none") && key[0]=='^')
    {
        ret->key = key[1] - 0x40;					//^A-Z ctrl codes
    }
    else
    {

        //key

        //special cases for ascii chars
        if(*key=='[' || *key==']' || *key=='\\' || *key=='`' || *key=='/' || *key==';')
        {
            ret->key = key[0];
            //vkeys
        }
        else
        {
            *key = toupper(*key);
            int v = GetVKey(key);
            if(!v)
                ret->key = key[0];
            else
                ret->key = v;
            ret->fVirt |= FVIRTKEY;
        }

        // mod keys

        char temp[32];
        strncpy(temp, mod, sizeof(temp));
        temp[sizeof(temp)-1] = 0;

        if(stristr(temp,const_cast<char *> ("CTRL")))	ret->fVirt |= FCONTROL;
        if(stristr(temp,const_cast<char *> ("SHIFT")))	ret->fVirt |= FSHIFT;
        if(stristr(temp,const_cast<char *> ("ALT")))	ret->fVirt |= FALT;

    }

    // cmd

    //int c = BspCmd->GetID(cmd);
    ccmd_t *c = Ccmd_FindCmd(cmd);
    if(!c)
        return false;
    ret->cmd = c->id;

    return true;
}
//
// Show keyboard accelerators and their commands
//
void KBSettings::ShowKeys(HACCEL accel)
{
    char tmp[128];

    //copy accelerators
    int count = CopyAcceleratorTable(accel,0,0);

    ACCEL *acctbl = new ACCEL[count];

    CopyAcceleratorTable(accel,acctbl,count);

    BSPHelp(const_cast<char *> ("Key Mappings"),const_cast<char *> (""));
    //loop for each accel
    for(int i=0; i < count; i++)
    {

        ACCEL *a = &acctbl[i];

        KBSettings::AccelToText(*a, tmp);				// get shortcut text
        ccmd_t *c = Ccmd_FindCmd(a->cmd);
        char *cmd;
        if(c)
            cmd = c->name;
        else
            cmd = const_cast<char *> ("Unknown Command");

        if(c && c->info_text)
            BSPHelpAddV(const_cast<char *> ("%s\r\n\t%s - %s\r\n\r\n"), tmp, cmd, c->info_text);
        else
            BSPHelpAddV(const_cast<char *> ("%s\r\n\t%s\r\n\r\n"), tmp, cmd);
    }
    delete [] acctbl;
}


/*
	keyboard.cfg format:

		ctrl+shift+alt keyname command

	example lines:

		ctrl space CM_EDITCLONE
		none / CM_FLIPCLIPPERS

	- case insensitive
	- first token will be strstr'd for ctrl alt and shift.
*/
// save keyboard settings
void KBSettings::SaveToDisk()
{
    Path kb(const_cast<char *> ("settings/keyboard.cfg"));

    FILE* kbfile = fopen(kb,"w+t");
    if (!kbfile)
    {
        Msgbox(const_cast<char *> ("Error! Couldn't open '%s' for writing..."), (char*)kb);
        return;
    }

    //write keyboard.cfg
    fprintf(kbfile,"// BSP Keyboard Mappings\n");
    fprintf(kbfile,"// Line format: [none | ctrl+alt+shift] [keyname] [CM_command]\n");
    fprintf(kbfile,"// Refer to keyboard dialog for key names and command names\n//\n");

    //TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO  accelerators need to go away....!!!
    //copy accelerators
    int count = CopyAcceleratorTable(acceltable,0,0);

    ACCEL *acctbl = new ACCEL[count];

    CopyAcceleratorTable(acceltable,acctbl,count);

    char line[256];
    //loop for each accel
    for(int i=0; i < count; i++)
    {

        AccelToCfgText(acctbl[i], line);
        if(!*line)
            continue;
        fprintf(kbfile,"%s\n",line);
    }
    fclose(kbfile);
    delete [] acctbl;

}
// load keyboard settings
void KBSettings::LoadFromDisk(char *filename)
{

    char *dat = (char*) file_get_contents(filename);
    if (!dat)
        return;

    Tokenizer config(dat);

    char mod[32], key[32], cmd[32];
    while(true)
    {

        if(!config.next(true))
            break;
        strncpy(mod,config.token,sizeof(mod));

        if(!config.next(true))
            break;
        strncpy(key,config.token,sizeof(key));

        if(!config.next(true))
            break;
        strncpy(cmd,config.token,sizeof(cmd));

        ACCEL a;
        if(!ParseCfgText(mod, key, cmd, &a))
        {
            syserror(const_cast<char *> ("Keyboard settings: couldn't find command \"%s\" in file %s (Line %d)"),
                     cmd, filename, config.line);
        }
        else
        {
            AddReplaceAccel(&a);
        }
    }
    delete[] dat;
}

/*
//
// Registry
//
Registry::Registry(const char *path)
	: key(0)
{
	snprintf(this->path, sizeof(this->path), "%s\\%s", REG_BASEPATH, path);
	RegCreateKeyEx(HKEY_CURRENT_USER, this->path, 0, 0, 0, KEY_ALL_ACCESS, 0, &key, 0);
}
Registry::~Registry() {
	RegCloseKey(key);
}
void Registry::ChangePath(const char *path) {
	RegCloseKey(key);
	RegCreateKeyEx(HKEY_CURRENT_USER, path, 0, 0, 0, KEY_ALL_ACCESS, 0, &key, 0);
}
//get DWORD
bool Registry::GetValue(const char *name, DWORD *value) {
	DWORD len = sizeof(DWORD);
	DWORD type = REG_DWORD;
	return ERROR_SUCCESS == RegQueryValueEx(key, name, 0, &type, (BYTE*) value, &len);
}
//get string
bool Registry::GetValue(const char *name, char *value, int maxlen) {
	DWORD type = REG_SZ;
	return ERROR_SUCCESS == RegQueryValueEx(key, name, 0, &type, (BYTE*) value, (DWORD*)&maxlen);
}
//write DWORD
bool Registry::SetValue(const char *name, DWORD value) {
	return ERROR_SUCCESS == RegSetValueEx(key, name, 0, REG_DWORD, (BYTE*) &value, sizeof(DWORD));
}
//write string
bool Registry::SetValue(const char *name, const char *value) {
	return ERROR_SUCCESS == RegSetValueEx(key, name, 0, REG_SZ, (BYTE*) value, strlen(value));
}
*/

//
// WindowPlacement
//

//get window's current position
void WindowPlacement::WP_GetPos()
{
    HWND hwnd = WP_GetHwnd();
    WP_isvalid = 0 != GetWindowPlacement(hwnd,&WP_wp);

    WP_wp.showCmd = SW_SHOWNORMAL;
    if(!IsWindowVisible(hwnd))
        WP_wp.showCmd = SW_HIDE;

    WP_zorder = 0;

    HWND first = GetTopWindow(GetParent(hwnd));
    HWND win = first;

    while(win && win != hwnd)
    {
        WP_zorder++;
        win = GetNextWindow(win, GW_HWNDNEXT);

        if(win == first)
            return;
    }
}
//set window's position
void WindowPlacement::WP_SetPos()
{
    HWND hwnd = WP_GetHwnd();
    if(WP_isvalid)
        SetWindowPlacement(hwnd, &WP_wp);
    else
        ShowWindow(hwnd, SW_RESTORE);
}
void WindowPlacement::WP_SavePos(int slot)
{
    const char *name = WP_WindowName();
    char base[64];
    snprintf(base,sizeof(base),"slot%d_%s",slot,name);

    WProfile pf(const_cast<char *> ("Window Placement"), Path(const_cast<char *> ("settings/bsp.ini")));

    pf.WriteInt(base, 1);

    char key[64];
    snprintf(key,sizeof(key), "%s_showCmd",base);
    pf.WriteInt(key, WP_wp.showCmd);
    snprintf(key,sizeof(key), "%s_zorder",base);
    pf.WriteInt(key, WP_zorder);

    snprintf(key,sizeof(key), "%s_x",base);
    pf.WriteInt(key, WP_wp.rcNormalPosition.left);
    snprintf(key,sizeof(key), "%s_y",base);
    pf.WriteInt(key, WP_wp.rcNormalPosition.top);
    snprintf(key,sizeof(key), "%s_cx",base);
    pf.WriteInt(key, WP_wp.rcNormalPosition.right - WP_wp.rcNormalPosition.left);
    snprintf(key,sizeof(key), "%s_cy",base);
    pf.WriteInt(key, WP_wp.rcNormalPosition.bottom - WP_wp.rcNormalPosition.top);
}
void WindowPlacement::WP_LoadPos(int slot)
{
    const char *name = WP_WindowName();
    char base[64];
    snprintf(base,sizeof(base),"slot%d_%s",slot,name);

    WProfile pf(const_cast<char *> ("Window Placement"), Path(const_cast<char *> ("settings/bsp.ini")));

    WP_isvalid = 0 != pf.GetInt(base);
    if(!WP_isvalid)
        return;

    WP_wp.length = sizeof(WP_wp);
    WP_wp.flags = 0;

    char key[64];
    snprintf(key,sizeof(key), "%s_showCmd",base);
    WP_wp.showCmd = pf.GetInt(key);
    snprintf(key,sizeof(key), "%s_zorder",base);
    WP_zorder = pf.GetInt(key);

    snprintf(key,sizeof(key), "%s_x",base);
    WP_wp.rcNormalPosition.left = pf.GetInt(key);
    snprintf(key,sizeof(key), "%s_y",base);
    WP_wp.rcNormalPosition.top = pf.GetInt(key);
    snprintf(key,sizeof(key), "%s_cx",base);
    WP_wp.rcNormalPosition.right = pf.GetInt(key);
    WP_wp.rcNormalPosition.right += WP_wp.rcNormalPosition.left;
    snprintf(key,sizeof(key), "%s_cy",base);
    WP_wp.rcNormalPosition.bottom = pf.GetInt(key);
    WP_wp.rcNormalPosition.bottom += WP_wp.rcNormalPosition.top;
}

//static
void WindowPlacement::SavePositions(int slot)
{
    surfaceWindow->WP_GetPos();
    surfaceWindow->WP_SavePos(slot);
    groupWindow->WP_GetPos();
    groupWindow->WP_SavePos(slot);
    kpWindow->WP_GetPos();
    kpWindow->WP_SavePos(slot);
    texWindow->WP_GetPos();
    texWindow->WP_SavePos(slot);
    editWindow->WP_GetPos();
    editWindow->WP_SavePos(slot);
    wconsole->WP_GetPos();
    wconsole->WP_SavePos(slot);

    for (int c = 0; c < set.xyViews; c++)
    {
        xyWindow[c]->WP_GetPos();
        xyWindow[c]->WP_SavePos(slot);
    }
}


#define WP_NUM_WINS 8			//this is used for zorder counting

//static
void WindowPlacement::LoadPositions(int slot)
{
    surfaceWindow->WP_LoadPos(slot);
    surfaceWindow->WP_SetPos();
    groupWindow->WP_LoadPos(slot);
    groupWindow->WP_SetPos();
    kpWindow->WP_LoadPos(slot);
    kpWindow->WP_SetPos();
    texWindow->WP_LoadPos(slot);
    texWindow->WP_SetPos();
    editWindow->WP_LoadPos(slot);
    editWindow->WP_SetPos();
    wconsole->WP_LoadPos(slot);
    wconsole->WP_SetPos();

    for (int c = 0; c < set.xyViews; c++)
    {
        xyWindow[c]->WP_LoadPos(slot);
        xyWindow[c]->WP_SetPos();
    }

    //zorder, probably a better way of doing this. maybe create auto-LList for all WP instances
    for(int i=WP_NUM_WINS-1; i>=0; i--)
    {
        if(surfaceWindow->WP_zorder == i)
            BringWindowToTop(surfaceWindow->hwnd);
        if(groupWindow->WP_zorder == i)
            BringWindowToTop(groupWindow->hwnd);
        if(kpWindow->WP_zorder == i)
            BringWindowToTop(kpWindow->hwnd);
        if(texWindow->WP_zorder == i)
            BringWindowToTop(texWindow->hwnd);
        if(editWindow->WP_zorder == i)
            BringWindowToTop(editWindow->hwnd);
        if(wconsole->WP_zorder == i)
            BringWindowToTop(wconsole->hwnd);

        for (int c = 0; c < set.xyViews; c++)
        {
            if(xyWindow[c]->WP_zorder == i)
                BringWindowToTop(xyWindow[c]->hwnd);
        }
    }
}


//
// DrawButton
//
DrawButton::DrawButton(HWND parent, int id, int type, RECT *rect)
{
    this->parent = parent;
    this->type = type;
    this->id = id;
    if(rect)
        rc = *rect;
    else
        rc.left=rc.top=rc.right=rc.bottom = 0;
    mdown = 0;
    pushed = 0;
    enabled = 1;
    visible = 1;
}
bool DrawButton::mousedown(POINT pt)
{
    bool handled = false;
    if(enabled && visible && type!=DRAWB_HIDE)
    {
        if(PtInRect(&rc, pt))
        {
            SetCapture(parent);
            mdown = true;
            pushed = true;
            redraw();
            handled = true;
        }
    }
    return handled;
}
bool DrawButton::mouseup(POINT pt)
{
    bool handled = false;
    if(mdown)
    {
        ReleaseCapture();
        if(pushed)
        {
            SendMessage(parent, WM_COMMAND, id, 0);
        }
        mdown = false;
        pushed = false;
        redraw();
        handled = true;
    }
    return handled;
}
void DrawButton::mousemove(POINT pt)
{
    if(!enabled || !visible || type==DRAWB_HIDE)
        return;
    if(mdown)
    {
        pushed = 0 != PtInRect(&rc, pt);
    }
    redraw();
}
void DrawButton::redraw()
{
    if(!visible || type==DRAWB_HIDE)
        return;
    HDC hdc = GetDC(parent);
    redraw(hdc);
    ReleaseDC(parent, hdc);
}
void DrawButton::redraw(HDC hdc)
{
    if(!visible || type==DRAWB_HIDE)
        return;
    int state = pushed ? DFCS_PUSHED|DFCS_FLAT : 0;
    RECT r;
    CopyRect(&r, &rc);
    InflateRect(&r, -2, -2);

    if(type == DRAWB_ELLIPSES)
    {
        state |= DFCS_BUTTONPUSH;
        DrawFrameControl(hdc, &rc, DFC_BUTTON, state);
        SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
        SetBkMode(hdc, TRANSPARENT);
        SetTextAlign(hdc, TA_CENTER);
        int x = (rc.right - rc.left) / 2 + rc.left + 1;	//calc center
        int y = rc.top;
        if(pushed)
        {
            ++x;
            ++y;
        }
        HGDIOBJ oldfont = SelectObject(hdc, set.font_ui_bold);
        ExtTextOut(hdc, x, y, ETO_CLIPPED, &r, "...", 3, 0);
        SelectObject(hdc, oldfont);
    }
    else if(type == DRAWB_UP)
    {
        state |= DFCS_SCROLLUP;
        DrawFrameControl(hdc, &rc, DFC_SCROLL, state);
    }
    else if(type == DRAWB_DOWN)
    {
        state |= DFCS_SCROLLDOWN;
        DrawFrameControl(hdc, &rc, DFC_SCROLL, state);
    }
    else
    {
        DrawFrameControl(hdc, &rc, DFC_BUTTON, state);
    }
}

