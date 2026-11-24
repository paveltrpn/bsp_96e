#include "global.h"

//
// WControl
//

WControl::WControl(HWND parent,int id,char * text,int cx,int cy, int width,int height)
{
    created = false;
    SetCaption(text);

    attr.parent = parent;
    attr.dwExStyle = 0;
    attr.dwStyle = WS_CHILDWINDOW|WS_GROUP|WS_TABSTOP|WS_VISIBLE;
    attr.x = cx;
    attr.y = cy;
    attr.width = width;
    attr.height = height;
    attr.menu = (HMENU) id;
    attr.param = this;

    memset(&wc,0,sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND+1;
    wc.lpfnWndProc = 0;		//must be zero for builtin controls
    wc.hCursor = LoadCursor(0,IDC_ARROW);
    wc.hIconSm =
        wc.hIcon = LoadIcon(0,IDI_APPLICATION);
}
//init control from dialog.
// must prevent calling Create() and optionally subclass at this point.
WControl::WControl(HWND parentdlg, int id, bool subclass)
{
    created = true;

    hwnd = GetDlgItem(parentdlg, id);

    if(hwnd && subclass)
    {
#ifdef _WIN64
        SetWindowLong(hwnd, GWLP_USERDATA, reinterpret_cast<long long>(this));
#else
        SetWindowLong(hwnd, GWL_USERDATA, reinterpret_cast<long>(this));
#endif
        //subclasses should override WndProc and def with:  return WControl::CallWindowProc(msg,wParam,lParam)
#ifdef _WIN64
        oldwndproc = (WNDPROC)SetWindowLong(hwnd, GWLP_WNDPROC, (LONG_PTR)MsgRouter);
#else
        oldwndproc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (LONG)MsgRouter);
#endif
    }
}
WControl::~WControl()
{
}
//static wndproc
LRESULT CALLBACK WControl::MsgRouter(HWND hwnd, UINT msg,WPARAM wParam,LPARAM lParam)
{
#ifdef _WIN64
    WControl *wnd = (WControl*)GetWindowLong(hwnd, GWLP_USERDATA);
#else
    WControl *wnd = (WControl*)GetWindowLong(hwnd, GWL_USERDATA);
#endif
    //special care is taken here to handle custom controls (plain window)
    //and subclassed win32 controls.
    if(!wnd)
    {
        if(msg == WM_CREATE)
        {
            LPCREATESTRUCT lp = (LPCREATESTRUCT)lParam;
            wnd = (WControl*)lp->lpCreateParams;

            if(!wnd)
            {
                syserror(const_cast<char *> ("WControl failed to initialize"));
                return -1;
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
        //normal window that hasnt sent WM_CREATE yet
        if(!wnd)
        {
            return DefWindowProc(hwnd,msg,wParam,lParam);
        }
    }

    if(!wnd)
        sysfatal(const_cast<char *> ("WControl failed to initialize"));

    //call virtual wndproc
    return wnd->WndProc(msg,wParam,lParam);
}
LRESULT WControl::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    return CallWindowProc(oldwndproc,hwnd,msg,wParam,lParam);
}
bool WControl::Create()
{
    if(created)
        return false;
    created = true;

    WNDCLASS wndclass;
    if(!::GetClassInfo(wc.hInstance,wc.lpszClassName,&wndclass))  	//dont re-register the same class
    {
        SetLastError(0);
        if(!::RegisterClassEx(&wc))
        {
            DWORD d = GetLastError();
            syserror(const_cast<char *> ("WControl: RegisterClassEx failed: GLE: %d"), d);
            return false;
        }
    }
    hwnd = ::CreateWindowEx(attr.dwExStyle,wc.lpszClassName,caption,
                            attr.dwStyle,attr.x,attr.y,attr.width,attr.height,attr.parent,attr.menu,wc.hInstance,attr.param);

    if(!hwnd)
    {
        return false;
    }
    //set class pointer for subclass
#ifdef _WIN64
    SetWindowLong(hwnd, GWLP_USERDATA, reinterpret_cast<long long>(this));
#else
    SetWindowLong(hwnd, GWL_USERDATA, reinterpret_cast<long>(this));
#endif
    //subclasses should override WndProc and def with:  return WControl::CallWindowProc(msg,wParam,lParam)
#ifdef _WIN64
    oldwndproc = (WNDPROC)SetWindowLong(hwnd, GWLP_WNDPROC, (LONG_PTR)MsgRouter);
#else
    oldwndproc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (LONG)MsgRouter);
#endif
    return true;
}

LRESULT WControl::SendMessage(UINT msg,WPARAM wParam,LPARAM lParam)
{
    return ::SendMessage(hwnd,msg,wParam,lParam);
}

void WControl::SetCaption(char *caption)
{
    *this->caption = 0;
    if(caption)
        strncpy(this->caption,caption,sizeof(this->caption));
    if(hwnd)
        ::SetWindowText(hwnd,caption);
}

void WControl::SetFont(HFONT font, int redraw)
{
    SendMessage(WM_SETFONT,(WPARAM)font,redraw);
}

//
// WEdit
//

WEdit::WEdit(HWND parent,int id,char * text,int cx,int cy, int width,int height,int textLimit, bool multiline)
    : WControl(parent,id,text,cx,cy,width,height)
{
    attr.dwExStyle |= WS_EX_CLIENTEDGE;
    attr.dwStyle |= ES_AUTOHSCROLL | ES_LEFT;

    if (multiline)
        attr.dwStyle |= ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | WS_HSCROLL;

    wc.lpszClassName = "EDIT";
    MaxLen = textLimit;
}
WEdit::WEdit(HWND parentdlg, int id, const char * text, int textLimit,bool subclass)
    : WControl(parentdlg,id,subclass)
{
    SetMaxText(textLimit);
    if(text)
        SetText(text);
}
int WEdit::GetText(char *lpString, int textLimit)
{
    return GetWindowText(hwnd, lpString, (textLimit < 0 ? MaxLen : textLimit));
}
void WEdit::SetText(const char *str)
{
    SetWindowText(hwnd,str);
}
void WEdit::SetMaxText(int textLimit)
{
    MaxLen = textLimit;
    SendMessage(EM_LIMITTEXT, (textLimit<=0?0:textLimit-1), 0);
}
void WEdit::SetInt(int val)
{
    static char buf[40];
    SetText(itoa(val, buf, 10));
}
int WEdit::GetInt()
{
    static char buf[40];
    GetText(buf, sizeof(buf));
    return atoi(buf);
}
void WEdit::SetFloat(float val)
{
    static char buf[40];
    sprintf(buf,"%f",val);
    FixFloatString(buf);
    SetText(buf);
}
float WEdit::GetFloat()
{
    static char buf[40];
    GetText(buf, sizeof(buf));
    return (float) atof(buf);
}

//
// WStatic
//
WStatic::WStatic(HWND parent,int id,char * text,int cx,int cy, int width,int height)
    : WControl(parent,id,text,cx,cy,width,height)
{
    wc.lpszClassName = "STATIC";
}
WStatic::WStatic(HWND parentdlg, int id,bool subclass)
    : WControl(parentdlg,id,subclass)
{}
void WStatic::SetText(char *str)
{
    SetWindowText(hwnd, str);
}

//
// WButton
//
WButton::WButton(HWND parent,int id,char * text,int cx,int cy, int width,int height)
    : WControl(parent,id,text,cx,cy,width,height)
{
    wc.lpszClassName = "BUTTON";
}
WButton::WButton(HWND parentdlg, int id,bool subclass)
    : WControl(parentdlg,id,subclass)
{}

//
// WCheck
//
WCheck::WCheck(HWND parent,int id,char * text,int cx,int cy, int width,int height)
    : WButton(parent,id,text,cx,cy,width,height)
{
    attr.dwStyle |= BS_AUTOCHECKBOX;
}
WCheck::WCheck(HWND parentdlg, int id,int defValue, bool subclass)
    : WButton(parentdlg,id,subclass)
{
    SetCheck(defValue);
}
int WCheck::GetCheck()
{
    return SendMessage(BM_GETCHECK, 0,0);
}
void WCheck::SetCheck(int checkstate)
{
    SendMessage(BM_SETCHECK,checkstate,0);
}

//
// WRadio
//
WRadio::WRadio(HWND parent,int id,char * text,int cx,int cy, int width,int height)
    : WCheck(parent,id,text,cx,cy,width,height)
{
    attr.dwStyle |= BS_AUTORADIOBUTTON;
}
WRadio::WRadio(HWND parentdlg, int id,int defValue, bool subclass)
    : WCheck(parentdlg,id,subclass)
{
    SetCheck(defValue);
}

//
// WList
//
WList::WList(HWND parent,int id,int cx,int cy, int width,int height)
    : WControl(parent,id,0,cx,cy,width,height)
{
    attr.dwExStyle |= WS_EX_CLIENTEDGE;
    attr.dwStyle |= WS_VSCROLL | LBS_NOTIFY;
    wc.lpszClassName = "LISTBOX";
}
WList::WList(HWND parentdlg, int id, bool subclass)
    : WControl(parentdlg,id,subclass)
{}
int WList::AddString(char *str)
{
    return SendMessage(LB_ADDSTRING,0,(LPARAM)str);
}
int WList::GetString(char *str, int index)
{
    return SendMessage(LB_GETTEXT,(WPARAM)index,(LPARAM)str);
}
void WList::ClearList()
{
    SendMessage(LB_RESETCONTENT,0,0);
}
void WList::InsertString(char *str, int index)
{
    SendMessage(LB_INSERTSTRING,(WPARAM)index,(LPARAM)str);
}
int WList::GetSelIndex()
{
    return SendMessage(LB_GETCURSEL,0,0);
}
void WList::SetSelIndex(int index)
{
    SendMessage(LB_SETCURSEL,index,0);
}
void WList::DeleteString(int index)
{
    SendMessage(LB_DELETESTRING,(WPARAM)index,0);
}
int WList::GetCount()
{
    return SendMessage(LB_GETCOUNT,0,0);
}
int WList::GetItemHeight(int index)
{
    return SendMessage(LB_GETITEMHEIGHT,index,0);
}
int WList::GetTopIndex()
{
    return SendMessage(LB_GETTOPINDEX,0,0);
}
DWORD WList::GetItemData(int index)
{
    return SendMessage(LB_GETITEMDATA,index,0);
}
bool WList::SetItemData(int index, DWORD data)
{
    return LB_ERR != SendMessage(LB_SETITEMDATA,index,data);
}

//
// WCombo - ComboBox
//
WCombo::WCombo(HWND parent,int id,int cx,int cy, int width,int height, int textLimit)
    : WControl(parent,id,0,cx,cy,width,height)
{
    attr.dwStyle |= WS_VSCROLL;
    wc.lpszClassName = "COMBOBOX";

    MaxLen = textLimit;
}
WCombo::WCombo(HWND parentdlg, int id, int textLimit, bool subclass)
    : WControl(parentdlg,id,subclass)
{
    LimitText(textLimit);
}
void WCombo::LimitText(int len)
{
    MaxLen = len;
    SendMessage(CB_LIMITTEXT, len, 0);
}
int WCombo::SetEditSel(int start, int end)
{
    return SendMessage(CB_SETEDITSEL, 0, MAKELPARAM(start, end));
}
int WCombo::SetSelString(const char *lpszSelect, int indexStart)
{
    return SendMessage(CB_SELECTSTRING, indexStart, (LPARAM)lpszSelect);
}
void WCombo::SetText(const char *str)
{
    if (SetSelString(str, -1) < 0)
    {
        SetWindowText(hwnd,str);
        SetEditSel(0, strlen(str));
    }
}
int WCombo::ClearList()
{
    return SendMessage(CB_RESETCONTENT, 0, 0);
}
int WCombo::InsertString(char *str, int index)
{
    return SendMessage(CB_INSERTSTRING,index,(LPARAM)str);
}
int WCombo::GetText(char *lpString, int textLimit)
{
    return GetWindowText(hwnd, lpString, (textLimit < 0 ? MaxLen : textLimit));
}
int WCombo::GetString(char *lpszBuffer, int index)
{
    return SendMessage(CB_GETLBTEXT, index, (LPARAM) lpszBuffer);
}
int WCombo::FindString(char *lpszFind, int index)
{
    return SendMessage(CB_FINDSTRING, index, (LPARAM)lpszFind);
}
int WCombo::FindStringExact(char *lpszFind, int index)
{
    return SendMessage(CB_FINDSTRINGEXACT, index, (LPARAM)lpszFind);
}
int WCombo::GetSelIndex()
{
    return SendMessage(CB_GETCURSEL,0,0);
}
int WCombo::SetSelIndex(int index)
{
    return SendMessage(CB_SETCURSEL,index,0);
}
int WCombo::DeleteString(int index)
{
    return SendMessage(CB_DELETESTRING,index,0);
}
int WCombo::GetCount()
{
    return SendMessage(CB_GETCOUNT,0,0);
}
int WCombo::GetItemHeight(int index)
{
    return SendMessage(CB_GETITEMHEIGHT,index,0);
}
int WCombo::GetTopIndex()
{
    return SendMessage(CB_GETTOPINDEX,0,0);
}
int WCombo::ShowList()
{
    return SendMessage(CB_SHOWDROPDOWN,TRUE,0);
}
int WCombo::HideList()
{
    return SendMessage(CB_SHOWDROPDOWN,FALSE,0);
}
int WCombo::AddString(char *str)
{
    return SendMessage(CB_ADDSTRING,0,(LPARAM)str);
}
DWORD_PTR WCombo::GetItemData(int index)
{
    return SendMessage(CB_GETITEMDATA, index, 0);
}
//
// WListView
//
WListView::WListView(HWND parent,int id,int cx,int cy, int width,int height)
    : WControl(parent,id,0,cx,cy,width,height)
{
    wc.lpszClassName = WC_LISTVIEW;
    attr.dwStyle |= LVS_REPORT | LVS_SINGLESEL | WS_HSCROLL|WS_VSCROLL | LVS_OWNERDRAWFIXED;
    //attr.dwExStyle = LVS_EX_GRIDLINES;
}
int WListView::AddColumn(const char *text, int width)
{
    LVCOLUMN lvc;
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = width;
    lvc.pszText = const_cast<LPTSTR>(text);
    return ListView_InsertColumn(hwnd, 200, &lvc);
}

int WListView::AddItem(const char *text)
{
    LVITEM lvi;
    lvi.mask = LVIF_TEXT;
    lvi.iItem = 32000;
    lvi.iSubItem = 0;
    lvi.pszText = const_cast<LPTSTR>(text);
    lvi.cchTextMax = MAX_KEY;	//64 chars ought to be enough...
    return SendMessage(LVM_INSERTITEM, 0, (LPARAM)&lvi);
}


