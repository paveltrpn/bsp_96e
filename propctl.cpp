#include "global.h"
#include "propctl.h"

//
// WPropEdit
//
WPropEdit::WPropEdit(HWND parent,int id,char * text,int cx,int cy, int width,int height,WPropertyGrid *wprop)
    : WEdit(parent,id,text,cx,cy,width,height), wprop(wprop)
{}
LRESULT WPropEdit::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_GETDLGCODE:
        return DLGC_WANTALLKEYS;
    case WM_KEYDOWN:
        switch(wParam)
        {
        case VK_UP:
            wprop->MoveSelUp();
            return 0;
        case VK_DOWN:
            wprop->MoveSelDown();
            return 0;
            //submit and go to next/prev item
        case VK_TAB:
            wprop->HideEdit(true);
            ::SendMessage(wprop->attr.parent,PM_TABBED, (0x8000 & GetKeyState(VK_SHIFT)?1:0), 0);
            return 0;
            //submit change and save
        case VK_RETURN:
            wprop->HideEdit(true);
            wprop->Redraw();
            return 0;
            //cancel change
        case VK_ESCAPE:
            wprop->HideEdit(false);
            wprop->Redraw();
            return 0;
        }
    }
    return WControl::WndProc(msg,wParam,lParam);
}
//
// WPropItem
//
WPropItem::WPropItem(char *name, char *value, DWORD data)
{
    this->name = 0;
    this->value = 0;
    set_name(name);
    set_value(value);

    header = false;
    disabled = false;
    this->data = data;
    backcolor = -1;
    forecolor = -1;
    modified = false;
    list_name = 0;
    list_value = 0;
    button_type = DRAWB_HIDE;
    max_edit_width = 0;
}
WPropItem::~WPropItem()
{
    delete [] name;
    delete [] value;
}
//name
void WPropItem::set_name(char *name)
{
    delete [] this->name;
    this->namelen = 0;
    this->name = 0;
    if(name)
    {
        this->namelen = strlen(name);
        this->name = new char[namelen + 1];
        strcpy(this->name, name);
    }
}
//value
void WPropItem::set_value(char *value)
{
    delete [] this->value;
    this->valuelen = 0;
    this->value = 0;
    if(value)
    {
        this->valuelen = strlen(value);
        this->value = new char[valuelen + 1];
        strcpy(this->value, value);
    }
}


//
// WPropertyGrid - generic property control
//
WPropertyGrid::WPropertyGrid(HWND parent,int id,int cx,int cy, int width,int height)
    : WControl(parent,id,0,cx,cy,width,height), scroll(0)
{
    wc.lpszClassName = "BSP_Ctl_PropertyBase";
    wc.lpfnWndProc = WControl::MsgRouter;
    wc.style |= CS_DBLCLKS;
    attr.dwExStyle |= WS_EX_CLIENTEDGE;
    attr.dwStyle |= WS_VSCROLL | WS_CLIPCHILDREN | WS_BORDER; //|WS_CLIPSIBLINGS

    rect.left = 0;
    rect.top = 0;
    rect.right = cx;
    rect.bottom = cy;

    splitter_pos = 3*width/8;	//-- 37.5% --|---- 62.5% ----//
    selected_idx = -1;
    tmp_idx = -1;
    item_height = 16;
    mdown = false;
    splitdown = false;

    modified = false;
    wpedit = 0;
    edit_visible = false;
    edit_part = 0;
    edit_button = 0;
}
WPropertyGrid::~WPropertyGrid()
{
    delete wpedit;
    delete wplist;
    delete edit_button;
}
LRESULT WPropertyGrid::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_CREATE:
        scroll.hwnd = hwnd;

        wpedit = new WPropEdit(hwnd,100,const_cast<char *> (""),0,0,0,0,this);
        wpedit->attr.dwStyle &= ~WS_BORDER;
        wpedit->attr.dwExStyle &= ~WS_EX_CLIENTEDGE;
        wpedit->Create();
        wpedit->SetFont(set.font_ui);
        wplist = new WFloatingList(hwnd);
        wplist->Create(false);
        wplist->list->SetFont(set.font_ui);
        edit_button = new DrawButton(hwnd, 101, DRAWB_HIDE, 0);
        break;

    case WM_GETDLGCODE:
        return DLGC_WANTALLKEYS;

    case PM_LIST_CHANGE:
    {
        WPITEM *wpi = GetItem();
        char *valstr = (char *) wplist->list->GetItemData(wParam);
        WPropItem *item = wpi ? wpi->data : 0;
        ::SendMessage(attr.parent, PM_CHANGE,(WPARAM)item, (LPARAM)valstr);
#ifdef _WIN64
        bool accept_change = 0 != GetWindowLong(attr.parent, DWLP_MSGRESULT);
#else
        bool accept_change = 0 != GetWindowLong(attr.parent, DWL_MSGRESULT);
#endif
        if (accept_change)
        {
            //get display value
            char display_str[1024];
            wplist->list->GetString(display_str,wParam);
            //set display and real (hidden) value
            if(edit_part==partLeft)
            {
                if(strcmp(display_str, wpi->data->name))
                {
                    wpi->data->set_name(display_str);
                    wpi->data->list_name = (DWORD_PTR) valstr;
                    wpi->data->modified = true;
                    modified = true;
                }
            }
            else if(edit_part==partRight)
            {
                if(strcmp(display_str, wpi->data->value))
                {
                    wpi->data->set_value(display_str);
                    wpi->data->list_value = (DWORD_PTR) valstr;
                    wpi->data->modified = true;
                    modified = true;
                }
            }
            else
                syserror(const_cast<char *> ("HideEdit: (1) no edit_part."));
        }
        wplist->ShowWindow(SW_HIDE);
    }
    break;

    case WM_COMMAND:
        if(wParam == 101)
        {
            //edit button was pressed
            if(edit_button->type == DRAWB_DOWN)
            {
                //dropdown
                WPITEM *wpi = GetItem();
                if(wplist->IsWindowVisible())
                {
                    wplist->ShowWindow(SW_HIDE);
                }
                else
                {
                    //request to fill popup
                    WPropItem *item = wpi ? wpi->data : 0;
                    ::SendMessage(attr.parent,PM_POPLIST, (WPARAM)item, (LPARAM) wplist->list);
#ifdef _WIN64
                    bool success = 0 != GetWindowLong(attr.parent, DWLP_MSGRESULT);
#else
                    bool success = 0 != GetWindowLong(attr.parent, DWL_MSGRESULT);
#endif
                    if(success)
                    {
                        ResizeList();
                        wplist->ShowWindow(SW_SHOW);
                    }
                    else syserror(const_cast<char *> ("WPropertyGrid: failed to retrieve list data"));
                }
            }
            else if(edit_button->type == DRAWB_ELLIPSES)
            {
                // if custom, sendmessage parent PM_EDITBTN
                WPITEM *wpi = GetItem();
                WPropItem *item = wpi ? wpi->data : 0;
                ::SendMessage(attr.parent,PM_EDITBTN,(WPARAM)item,0);
            }
        }
        break;

    case WM_KEYDOWN:
        switch(wParam)
        {
        case VK_UP:
            if(selected_idx > 0 && HideEdit(true))
            {
                selected_idx--;
                ScrollIntoView();
                Redraw();
            }
            break;
        case VK_DOWN:
            if(selected_idx >= 0 && selected_idx < (items.count-1) && HideEdit(true))
            {
                selected_idx++;
                ScrollIntoView();
                Redraw();
            }
            break;
        case VK_RETURN:
            if(!edit_visible && selected_idx >= 0)
            {
                WPITEM *wpi = GetItem();
                WPropItem *item = wpi ? wpi->data : 0;
                ::SendMessage(attr.parent, PM_CLICK, (WPARAM)item, partAny); //part is ambiguous - let host figure it out
            }
            break;
        case VK_TAB:
            HideEdit(true);
            ::SendMessage(attr.parent,PM_TABBED, (0x8000 & GetKeyState(VK_SHIFT)?1:0), 0);
            break;
        case VK_ESCAPE:
            HideEdit(false);
        }

        break;
    case WM_PAINT:
        Redraw();
        return 0;
    case WM_SIZE:
        Resize((short)LOWORD(lParam), (short) HIWORD(lParam));
        break;
    case WM_LBUTTONDBLCLK:
    {
        ::SetFocus(this->hwnd);
        MAKEPOINT(pt, lParam);
        mousedown(pt, 1, true);
    }
    break;
    case WM_LBUTTONDOWN:
    {
        ::SetFocus(this->hwnd);
        MAKEPOINT(pt, lParam);
        mousedown(pt,1,false);
    }
    break;
    case WM_RBUTTONDOWN:
    {
        ::SetFocus(this->hwnd);
        MAKEPOINT(pt, lParam);
        mousedown(pt,2,false);
    }
    break;
    case WM_LBUTTONUP:
    {
        MAKEPOINT(pt, lParam);
        mouseup(pt, 1);
    }
    break;
    case WM_RBUTTONUP:
    {
        MAKEPOINT(pt, lParam);
        mouseup(pt, 2);
    }
    break;
    case WM_MOUSEMOVE:
    {
        MAKEPOINT(pt, lParam);
        mousemove(pt);
    }
    break;
    //scrollbar
    case WM_VSCROLL:
        if(scroll.VScroll(wParam, 3))
        {
            wplist->ShowWindow(SW_HIDE);
            ResizeEdit();
            Redraw();
        }
        return 0;
    }
    return DefWindowProc(hwnd,msg,wParam,lParam);
}
void WPropertyGrid::MoveSelUp()
{
    HideEdit(true);
    if(--selected_idx < 0)
        selected_idx = items.count - 1;
    ScrollIntoView();
    Redraw();
}
void WPropertyGrid::MoveSelDown()
{
    HideEdit(true);
    if(++selected_idx >= items.count)
        selected_idx = 0;
    ScrollIntoView();
    Redraw();
}
void WPropertyGrid::ScrollUp()
{
    SendMessage(WM_VSCROLL,SB_LINEUP,0);
}
void WPropertyGrid::ScrollDown()
{
    SendMessage(WM_VSCROLL,SB_LINEDOWN,0);
}
void WPropertyGrid::Resize(int cx, int cy)
{
    GetClientRect(hwnd, &rect);
    ResizeEdit();
    ResizeScrollbar();
}
void WPropertyGrid::ResizeScrollbar()
{
    int sy = ItemFromPoint(rect.bottom) - scroll.Y.nPos;
    scroll.Y.nPage = sy;
    scroll.Y.nMax = items.count;

    if(items.count < sy)
    {
        ShowScrollBar(hwnd,SB_VERT,0);
    }
    else
    {
        ShowScrollBar(hwnd,SB_VERT,1);
    }
}
void WPropertyGrid::ScrollIntoView()
{
    if(selected_idx >= 0 && selected_idx < items.count)
    {
        int sy = ItemFromPoint(rect.bottom) - scroll.Y.nPos;
        if(selected_idx < scroll.Y.nPos)
        {
            scroll.Y.nPos = selected_idx;
        }
        else if(selected_idx >= sy + scroll.Y.nPos)
        {
            scroll.Y.nPos = selected_idx - sy + 1;
        }
    }
}
//adds new item, returns ref to WPropItem
WPropItem *WPropertyGrid::Add(char *name, char *value, DWORD data)
{
    WPropItem *item = new WPropItem(name,value,data);
    items.add(item);
    ResizeScrollbar();
    return item;
}
void WPropertyGrid::Clear()
{
    HideEdit(false);
    items.reset<WPropItem*>();
    modified = false;
    scroll.Y.nMax = 0;
    scroll.Y.nPos = 0;
    scroll.Update();
}
// calculate item from point. calculates height + spacer and adds scroll.Y.nPos.
// Y/h = item without spacer
//
//     (Y - Y/h) / h    Y-point minus spacers, div by item height
//    Y/h - Y/h^2		maths...
//   (Y*h - Y) / h^2	...and safe integral div
int WPropertyGrid::ItemFromPoint(int Y)
{
    return (Y*item_height - Y) / (item_height*item_height) + scroll.Y.nPos;
}

//mousedown
void WPropertyGrid::mousedown(POINT pt, int button, bool doubleclick)
{
    SetCapture(hwnd);
    if(button==1 && edit_button->mousedown(pt))
        return;

    int msgclick = 0;
    if(button==1 && doubleclick)
        msgclick = PM_DBLCLICK;

    mdown = true;
    if(onsplit)
    {
        //splitter
        splitdown = true;
        SetCursor(LoadCursor(0,IDC_SIZEWE));
    }
    else if (pt.x < splitter_pos)
    {
        int newidx = ItemFromPoint(pt.y);
        bool ok = true;
        if(newidx != selected_idx) ok = HideEdit(true);
        if(ok)
        {
            //set selected idx
            selected_idx = newidx;
            ScrollIntoView();
            Redraw();
            if(msgclick)
            {
                WPITEM *wpi = GetItem();
                WPropItem *item = wpi ? wpi->data : 0;
                ::SendMessage(attr.parent, msgclick, (WPARAM)item, partLeft);
            }
        }
    }
    else if (pt.x > splitter_pos)
    {
        int newidx = ItemFromPoint(pt.y);
        bool ok = true;
        if(newidx != selected_idx) ok = HideEdit(true);
        if(ok)
        {
            //set selected idx and edit
            selected_idx = newidx;
            ScrollIntoView();
            Redraw();
            if(msgclick)
            {
                WPITEM *wpi = GetItem();
                WPropItem *item = wpi ? wpi->data : 0;
                ::SendMessage(attr.parent, msgclick, (WPARAM)item, partRight);
            }
        }
    }
}
//mouseup
void WPropertyGrid::mouseup(POINT pt, int button)
{
    ReleaseCapture();
    if(button==1 && edit_button->mouseup(pt))
        return;

    //send click messages
    int msgclick = 0;
    if(button==1)
        msgclick = PM_CLICK;
    else if(button==2)
        msgclick = PM_RCLICK;
    if (pt.x < splitter_pos)
    {
        if(msgclick)
        {
            WPITEM *wpi = GetItem();
            WPropItem *item = wpi ? wpi->data : 0;
            ::SendMessage(attr.parent, msgclick, (WPARAM)item, partLeft);
        }
    }
    else if (pt.x > splitter_pos)
    {
        if(msgclick)
        {
            WPITEM *wpi = GetItem();
            WPropItem *item = wpi ? wpi->data : 0;
            ::SendMessage(attr.parent, msgclick, (WPARAM)item, partRight);
        }
    }

    mdown = false;
    splitdown = false;
    SetCursor(LoadCursor(0,IDC_ARROW));
}
//mousemove
void WPropertyGrid::mousemove(POINT pt)
{
    edit_button->mousemove(pt);
    if(!mdown)
        onsplit = pt.x >= (splitter_pos - 3) && pt.x < (splitter_pos + 3);
    if(onsplit)
        SetCursor(LoadCursor(0,IDC_SIZEWE));

    RECT rc;
    GetClientRect(hwnd, &rc);

    //move splitter
    if(splitdown)
    {
        splitter_pos = max(0, min(rc.right - 25, max(25,pt.x)));	//hmm?
        ResizeEdit();
        Redraw();
    }
}
//click selected button
void WPropertyGrid::ClickEditButton()
{
    if(selected_idx >= 0 && edit_button->type > 0)
        SendMessage(WM_COMMAND, edit_button->id, 0);
}
//resize the floating list before displaying it. make as small as possible, restrict
//to a max height, and move above edit if not enough room to display below.
void WPropertyGrid::ResizeList()
{
    //determine height to use for list window
    int itemheight = wplist->list->GetItemHeight(0);
    int itemcount = wplist->list->GetCount();
    int height = max(32,min(250,itemheight * itemcount + 8));
    //determine placement above or below edit
    RECT rc, rcedit;
    GetWindowRect(wpedit->hwnd, &rcedit);
    if (edit_button->type > 0) rcedit.right += WP_BWIDTH;	//readjust for button width
    bool above = (height-16) > (GetSystemMetrics(SM_CYFULLSCREEN) - rcedit.bottom);
    if (above)
    {
        rc.bottom = rcedit.top;
        rc.top = rc.bottom - height;
    }
    else
    {
        rc.top = rcedit.bottom;
        rc.bottom = rc.top + height;
    }
    //determine width and left/right placement
    int editwidth = rcedit.right-rcedit.left;
    int hzext = wplist->list->SendMessage(LB_GETHORIZONTALEXTENT,0,0);
    int width = min(350,max(100, (hzext > editwidth) ? hzext : editwidth));
    bool fits = width <= editwidth;
    if(fits)
    {
        rc.left = rcedit.left;
        rc.right = rcedit.right;
    }
    else
    {
        //align to left or right side depending on room
        if(rcedit.left + width < GetSystemMetrics(SM_CYFULLSCREEN))
        {
            rc.left = rcedit.left;
            rc.right = rc.left + width;
        }
        else
        {
            rc.right = rcedit.right;
            rc.left = rc.right - width;
        }
    }
    MoveWindow(wplist->hwnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, 1);
}
void WPropertyGrid::ResizeEdit()
{
//	if(edit_visible) {
    // resize and reposition edit and button
    RECT rc;
    GetItemRect(selected_idx,&rc);
    if(edit_part==partLeft)
    {
        rc.right = splitter_pos - 3;	//leave extra space for splitter
    }
    else if(edit_part==partRight)
    {
        rc.left = splitter_pos;
    }
    if(edit_button->type > 0)
    {
        // set dimensions of button
        CopyRect(&edit_button->rc, &rc);
        edit_button->rc.left = edit_button->rc.right - WP_BWIDTH;
        // shrink edit for room for button
        rc.right -= WP_BWIDTH;
    }
    WPITEM *wpi = GetItem();
    if(wpi && wpi->data && wpi->data->max_edit_width > 0)
    {
        rc.right = min(rc.right, rc.left + wpi->data->max_edit_width);
    }
    MoveWindow(wpedit->hwnd,rc.left+3, rc.top, rc.right-rc.left-3, rc.bottom-rc.top,edit_visible);//1
    //}
}
void WPropertyGrid::ShowEdit(int part)
{
    if(selected_idx >= 0 && HideEdit(true) && part==partLeft || part==partRight)
    {
        WPITEM *wpi = GetItem();
        if(wpi && !wpi->data->disabled && !wpi->data->header)  			//dont edit disabled or headers
        {
            edit_button->type = wpi->data->button_type;
            edit_part = part;
            edit_visible = true;
            //if dropdown type, dont actually enable the textbox
            if(edit_button->type != DRAWB_DOWN)
            {
                //copy item text for part
                if(part == partLeft)
                    wpedit->SetText(wpi->data->name);
                else
                    wpedit->SetText(wpi->data->value);
                if(wpi->data->modified)
                    wpedit->SetFont(set.font_ui_bold);
                else
                    wpedit->SetFont(set.font_ui);

                //resize edit before clicking...
                ResizeEdit();

                ShowWindow(wpedit->hwnd,SW_SHOW);


                //set edit caret to click point
                RECT erc;//, wrc;
                GetWindowRect(wpedit->hwnd, &erc);
                POINT pt;
                GetCursorPos(&pt);
                if(PtInRect(&erc,pt))
                {
                    //	GetWindowRect(hwnd, &wrc);
                    //	erc.left -= wrc.left;
                    //	erc.top -= wrc.top;
                    //	pt.x -= wrc.left;
                    //	pt.y -= wrc.top;
                    SetFocus(wpedit->hwnd);
                    int c = wpedit->SendMessageA(EM_CHARFROMPOS,0,MAKELPARAM(pt.x-erc.left, pt.y-erc.top));
                    wpedit->SendMessageA(EM_SETSEL,c,c);
                }
            }
            else
                ResizeEdit();
        }
    }
}
// return true to signal edit as being hidden. set change=false to cancel changes
bool WPropertyGrid::HideEdit(bool change)
{
    bool retval = true;
    if(edit_visible)
    {
        if(change)
        {
            WPITEM *wpi = GetItem();
            if(!wpi)
            {
                syserror(const_cast<char *> ("HideEdit: no wpi."));
                return false;	//huh??
            }
            //dropdown gets its value differently
            if(edit_button->type != DRAWB_DOWN)
            {

                char str[1024];
                wpedit->GetText(str, sizeof(str));

                //determine if text changed
                bool skip = false;
                if(edit_part==partLeft)
                {
                    if(!_stricmp(wpi->data->name,str))
                        skip=true;
                }
                else if(edit_part==partRight)
                {
                    if(!_stricmp(wpi->data->value,str))
                        skip=true;
                }
                if(!skip)
                {
                    // retval = sendmessage(parent, value to change to)
                    WPropItem *item = wpi ? wpi->data : 0;
                    ::SendMessage(attr.parent,PM_CHANGE,(WPARAM)item,(LPARAM)str);
#ifdef _WIN64
                    retval = 0 != GetWindowLong( attr.parent, DWLP_MSGRESULT );
#else
                    retval = 0 != GetWindowLong( attr.parent, DWL_MSGRESULT );
#endif
                    if(retval)
                    {
                        //if retval then set new value
                        if(edit_part==partLeft)
                        {
                            if(strcmp(str,wpi->data->name))
                            {
                                wpi->data->set_name(str);
                                wpi->data->modified = true;
                                modified = true;
                            }
                        }
                        else if(edit_part==partRight)
                        {
                            if(strcmp(str,wpi->data->value))
                            {
                                wpi->data->set_value(str);
                                wpi->data->modified = true;
                                modified = true;
                            }
                        }
                        else
                            syserror(const_cast<char *> ("HideEdit: (2) no edit_part."));
                    }
                }
            }
        }
    }
    //force list to hide
    wplist->ShowWindow(SW_HIDE);
    //true signals ok to stop editing
    if(retval)
    {
        ShowWindow(wpedit->hwnd, SW_HIDE);
        edit_button->type = DRAWB_HIDE;
        edit_visible = false;
    }
    return retval;
}

//get_item_rect - return false if item is outside of client area (not visible)
bool WPropertyGrid::GetItemRect(int idx, RECT *rc)
{
    bool retval = true;
    rc->left = 0;
    rc->right = rect.right;
    rc->bottom = rect.bottom;
    rc->top = (idx - scroll.Y.nPos)*item_height + (idx - scroll.Y.nPos);
    if(rc->top >= rc->bottom || rc->top < 0)
        retval = false;
    rc->bottom = rc->top+item_height;
    return retval;
}
//get item
WPITEM *WPropertyGrid::GetItem(int idx)
{
    if(idx < 0) idx = selected_idx;
    WPITEM *wpi = 0;
    if(idx >= 0 && idx < items.count)
    {
        for(wpi = items.items ; wpi && idx > 0; wpi = wpi->next, idx--);
    }
    return wpi;
}
//redraw
void WPropertyGrid::Redraw()
{
    scroll.Update();

    //stuff to show a button if edit is not shown
    int button_hack = 0;
    if(selected_idx != tmp_idx)
    {
        tmp_idx = selected_idx;
        if(selected_idx >= 0)
        {
            ::SendMessage(attr.parent,PM_SELCHANGE,0,0);
#ifdef _WIN64
            button_hack = GetWindowLong( attr.parent, DWLP_MSGRESULT );
#else
            button_hack = GetWindowLong( attr.parent, DWL_MSGRESULT );
#endif
        }
    }

    HDC hdc = GetDC(hwnd);
    HDC cdc = CreateCompatibleDC(hdc);
    HBITMAP hbmp = CreateCompatibleBitmap(hdc,rect.right,rect.bottom);
    HGDIOBJ oldbmp = SelectObject(cdc, hbmp);

    //fill bg
    SetBkColor(cdc, GetSysColor(COLOR_WINDOW));
    ExtTextOut(cdc, 0, 0, ETO_CLIPPED | ETO_OPAQUE, &rect,"",0,0);

    //setup item draw
    SetTextColor(cdc, GetSysColor(COLOR_WINDOWTEXT));
    HGDIOBJ oldfont = SelectObject(cdc, set.font_ui);
    HPEN btnfacepen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
    HGDIOBJ oldpen = SelectObject(cdc, btnfacepen);

    //determine height of items based on font size
    SIZE sz;
    GetTextExtentPoint32(cdc, "W", 1, &sz);
    item_height = sz.cy + 1;

    //draw items
    WPITEM *wpi = items.items;

    for(int i = 0; wpi && i < items.count; wpi = wpi->next, i++)
    {
        RECT item;
        if(!GetItemRect(i, &item))
            continue;

        //header item
        if(wpi->data->header)
        {
            SetTextColor(cdc, GetSysColor(COLOR_WINDOWTEXT));
            SetBkColor(cdc, GetSysColor(COLOR_BTNFACE));
            HGDIOBJ oldf = SelectObject(cdc, set.font_ui_bold);
            ExtTextOut(cdc, item.left+4, item.top, ETO_CLIPPED|ETO_OPAQUE, &item,
                       wpi->data->name, wpi->data->namelen,0);
            SelectObject(cdc, oldf);
            //draw regular item
        }
        else
        {
            RECT rcleft, rcright;
            CopyRect(&rcleft, &item);
            CopyRect(&rcright, &item);
            rcleft.right = splitter_pos;
            rcright.left = splitter_pos;


            //bold for modified
            HGDIOBJ oldf;
            if(wpi->data->modified)
            {
                oldf = SelectObject(cdc, set.font_ui_bold);
            }


            //////////////////////////////////////
            // draw left side
            //////////////////////////////////////

            //background is windowtext or select highlight only
            if(selected_idx == i && edit_visible && edit_button->type > 0 && edit_part==partLeft)
            {
                if(edit_button->type == DRAWB_DOWN)
                {
                    //set to default colors because no edit control will be shown
                    SetBkColor(cdc, GetSysColor(COLOR_WINDOW));
                    SetTextColor(cdc, GetSysColor(COLOR_WINDOWTEXT));
                }
                else
                {
                    //blank rect for edit control
                    SetBkColor(cdc, GetSysColor(COLOR_WINDOW));
                    SetTextColor(cdc, GetSysColor(COLOR_WINDOW));
                }
            }
            else
            {
                if(selected_idx == i)
                    SetBkColor(cdc, GetSysColor(COLOR_HIGHLIGHT));
                else
                    SetBkColor(cdc, GetSysColor(COLOR_WINDOW));
                //text is windowtext, disabled, or highlight
                if(wpi->data->disabled)
                    SetTextColor(cdc, GetSysColor(COLOR_GRAYTEXT));
                else
                {
                    if(selected_idx == i)
                        SetTextColor(cdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
                    else
                        SetTextColor(cdc, GetSysColor(COLOR_WINDOWTEXT));
                }
            }
            //draw out
            ExtTextOut(cdc, rcleft.left+6, rcleft.top, ETO_CLIPPED|ETO_OPAQUE,
                       &rcleft, wpi->data->name,wpi->data->namelen,0);

            //////////////////////////////////////
            // draw right side
            //////////////////////////////////////

            if (selected_idx == i && edit_visible && edit_button->type > 0 && edit_part==partRight)
            {
                if(edit_button->type == DRAWB_DOWN)
                {
                    //set to default colors because no edit control will be shown
                    SetBkColor(cdc, GetSysColor(COLOR_WINDOW));
                    SetTextColor(cdc, GetSysColor(COLOR_WINDOWTEXT));
                }
                else
                {
                    //blank rect for edit control
                    SetBkColor(cdc, GetSysColor(COLOR_WINDOW));
                    SetTextColor(cdc, GetSysColor(COLOR_WINDOW));
                }
            }
            else
            {
                //disabled gray on white
                if(wpi->data->disabled)
                {
                    SetBkColor(cdc, GetSysColor(COLOR_WINDOW));
                    SetTextColor(cdc, GetSysColor(COLOR_GRAYTEXT));
                }
                else
                {
                    //backcolor
                    if(wpi->data->backcolor != -1)
                        SetBkColor(cdc, wpi->data->backcolor);
                    else
                        SetBkColor(cdc, GetSysColor(COLOR_WINDOW));
                    //forecolor
                    if(wpi->data->forecolor != -1)
                        SetTextColor(cdc, wpi->data->forecolor);
                    else
                    {
                        if(wpi->data->backcolor != -1)
                        {
                            //automatically determine forecolor
                            BYTE *color = (BYTE *) &wpi->data->backcolor;
                            if((DWORD)color[0]+(DWORD)color[1]+(DWORD)color[2] < 128*3)
                                SetTextColor(cdc, RGB(255,255,255));
                            else
                                SetTextColor(cdc, RGB(0,0,0));
                        }
                        else
                            SetTextColor(cdc, GetSysColor(COLOR_WINDOWTEXT));
                    }
                }
            }
            //draw out
            ExtTextOut(cdc, rcright.left+6, rcright.top, ETO_CLIPPED|ETO_OPAQUE,
                       &rcright, (char*)wpi->data->value, wpi->data->valuelen,0);

            //a hack - draw bgcolor to right of edit control if edit width is restricted
            if(wpi->data->max_edit_width > 0 && wpi->data->backcolor != -1)
            {
                SetBkColor(cdc, wpi->data->backcolor);
                rcright.left += wpi->data->max_edit_width;
                ExtTextOut(cdc, 0, 0, ETO_CLIPPED|ETO_OPAQUE, &rcright, "",0,0);
            }

            //draw vertical split
            MoveToEx(cdc, splitter_pos, item.top, 0);
            LineTo(cdc, splitter_pos, item.bottom);

            //////////////////////////////////////
            // end item draw
            //////////////////////////////////////

            //undo bold for modified
            if(wpi->data->modified)
            {
                SelectObject(cdc, oldf);
            }


            //button hack - show button for selected item when edit not visible
            if(selected_idx == i && button_hack > 0 && wpi->data->button_type > 0 && edit_button->type != wpi->data->button_type)
            {
                edit_button->type = wpi->data->button_type;
                ResizeEdit();
            }

        }

        //draw bottom divider
        MoveToEx(cdc, item.left, item.bottom, 0);
        LineTo(cdc, item.right, item.bottom);
    }

    //redraw button
    edit_button->redraw(cdc);

    //display
    BitBlt(hdc,0,0,rect.right,rect.bottom,cdc,0,0,SRCCOPY);

    //cleanup
    SelectObject(cdc, oldbmp);
    DeleteObject(hbmp);
    SelectObject(cdc, oldpen);
    DeleteObject(btnfacepen);
    SelectObject(cdc, oldfont);
    DeleteDC(cdc);
    ReleaseDC(hwnd, hdc);

    ValidateRect(hwnd, 0);
    if(edit_visible)
        InvalidateRect(wpedit->hwnd,0,1);
}
