#include "global.h"
#include "propctl.h"

TKPTransferBuffer kpxfer;

char tempKey[MAX_KEY];
char tempValue[MAX_VALUE];
char lastKey[MAX_KEY];
char lastValue[MAX_VALUE];


LRESULT TBSPListBox::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_RBUTTONDOWN:
    {
        MAKEPOINT(pt, lParam);
        EvRButtonDown(wParam,&pt);
    }
    break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case LB1:
            Cut();
            return 0;
        case LB2:
            Copy();
            return 0;
        case LB3:
            Paste();
            return 0;
        case LB4:
            Clear();
            return 0;
        case LB5:
            Last();
            return 0;
        case LB6:
            Edit();
            return 0;
        case LB7:
            Add();
            return 0;
        }
        break;
    }
    return WControl::WndProc(msg,wParam,lParam);
}
TBSPListBox::TBSPListBox(HWND parent, int Id, int x, int y, int w, int h)
    : WList(parent, Id, x, y, w, h)
{
    memset(saveKey,0,sizeof(saveKey));
    memset(saveValue,0,sizeof(saveValue));

    memset(tempKey,0,sizeof(tempKey));
    memset(tempValue,0,sizeof(tempValue));

    memset(lastKey,0,sizeof(lastKey));
    memset(lastValue,0,sizeof(lastValue));
}

bool TBSPListBox::SetTempValues(LPPOINT pt)
{
    // Figure where we are,
    // Set tempKey and tempValue
    //
    int top = GetTopIndex();
    int iHeight = GetItemHeight(0);

    int idx = (pt->y/iHeight) + top;
    if (idx < 0 || idx > GetCount())
        return false;

    epair_t	*pair;

    SetSelIndex(idx);

    pair = map_i[set.curmap]->currentEntity()->epairs;
    for (int i = 0; i < idx && pair; i++)
        pair = pair->next;

    // now we're at the selected pair...
    if (!pair)
        return false;

    STRNCPY(tempKey,pair->key);
    STRNCPY(tempValue,pair->value);

    return true;
}

void TBSPListBox::EvRButtonDown(UINT /*modKeys*/, LPPOINT point)
{
    if (!set.Map_Read)
        return;

    bool onPair = SetTempValues(point);
    const int SL = 128;
    char tempStr[SL];

    Popup PopupMenu;
    PopupMenu.Add("Add Keypair...", LB7);

    // EDIT Keypair
    if (onPair)
    {
        int r = snprintf(tempStr,sizeof(tempStr),"Edit\t\"%s\" - \"%s\"",tempKey,tempValue);
        if(r < 0) tempStr[SL-2]=tempStr[SL-3]=tempStr[SL-4] = '.';	//ellipses
        PopupMenu.Add(tempStr, LB6);
        PopupMenu.Separator();
    }

    // CUT
    if (onPair)
    {
        int r = snprintf(tempStr,sizeof(tempStr),"Cut\t\"%s\" - \"%s\"",tempKey,tempValue);
        if(r < 0) tempStr[SL-2]=tempStr[SL-3]=tempStr[SL-4] = '.';	//ellipses
        PopupMenu.Add(tempStr, LB1);
    }
    else
    {
        PopupMenu.Gray();
        PopupMenu.Add("Cut", LB1);
    }

    // COPY
    if (onPair)
    {
        int r = snprintf(tempStr,sizeof(tempStr),"Copy\t\"%s\" - \"%s\"",tempKey,tempValue);
        if(r < 0) tempStr[SL-2]=tempStr[SL-3]=tempStr[SL-4] = '.';	//ellipses
        PopupMenu.Add(tempStr, LB2);
    }
    else
    {
        PopupMenu.Gray();
        PopupMenu.Add("Copy", LB2);
    }

    // PASTE
    if (!*saveKey)
    {
        PopupMenu.Gray();
        PopupMenu.Add("Paste", LB3);
    }
    else
    {
        int r = snprintf(tempStr,sizeof(tempStr),"Paste\t\"%s\" - \"%s\"",saveKey,saveValue);
        if(r < 0) tempStr[SL-2]=tempStr[SL-3]=tempStr[SL-4] = '.';	//ellipses
        PopupMenu.Add(tempStr, LB3);
    }

    // CLEAR
    if (!*saveKey)
    {
        PopupMenu.Gray();
        PopupMenu.Add("Clear", LB4);
    }
    else
    {
        int r = snprintf(tempStr,sizeof(tempStr),"Clear\t\"%s\" - \"%s\"",saveKey,saveValue);
        if(r < 0) tempStr[SL-2]=tempStr[SL-3]=tempStr[SL-4] = '.';	//ellipses
        PopupMenu.Add(tempStr, LB4);
    }

    // LAST INPUT KEY
    PopupMenu.Separator();
    if (!*lastKey)
    {
        PopupMenu.Gray();
        PopupMenu.Add("Last", LB5);
    }
    else
    {
        int r = snprintf(tempStr,sizeof(tempStr),"Paste\t\"%s\" - \"%s\"",lastKey,lastValue);
        if(r < 0) tempStr[SL-2]=tempStr[SL-3]=tempStr[SL-4] = '.';	//ellipses
        PopupMenu.Add(tempStr, LB5);
    }

    PopupMenu.Show(hwnd);
}

void TBSPListBox::Cut()
{
    if (!*tempKey || !*tempValue)
        return;

    strncpy(saveKey,tempKey,MAX_KEY);
    strncpy(saveValue,tempValue,MAX_VALUE);

    map_i[set.curmap]->currentEntity()->removeKeyPair(saveKey);
    kpWindow->calcViewSize();
}

void TBSPListBox::Copy()
{
    if (!*tempKey || !*tempValue)
        return;

    strncpy(saveKey,tempKey,MAX_KEY);
    strncpy(saveValue,tempValue,MAX_VALUE);
    kpWindow->calcViewSize();
}

void TBSPListBox::Paste()
{
    if (!saveKey || !saveValue || !saveKey[0] || !saveValue[0])
        return;

    map_i[set.curmap]->currentEntity()->setKey(saveKey,saveValue);
    kpWindow->calcViewSize();
}

void TBSPListBox::Clear()
{
    *saveKey = 0;
    *saveValue = 0;
}

void TBSPListBox::Last()
{
    if (!lastKey || !lastValue || !*lastKey || !*lastValue)
        return;

    map_i[set.curmap]->currentEntity()->setKey(lastKey,lastValue);
    kpWindow->calcViewSize();
}

void TBSPListBox::Add()
{
    kpWindow->addPair();
}

void TBSPListBox::Edit()
{
    if (!tempKey || !*tempKey)
        return;
    map *m = map_i[set.curmap];

    STRNCPY(kpxfer.Key,tempKey);
    STRNCPY(kpxfer.Value,tempValue);
    kpxfer.Global = BST_UNCHECKED;

    TKPDlg dialog(0,&kpxfer,1);
    if (dialog.Execute() == IDOK)
    {
        char key[MAX_KEY];
        char value[MAX_VALUE];
        kpWindow->FixString(kpxfer.Key,key);
        kpWindow->FixString(kpxfer.Value,value);

        if (key && value && key[0] && value[0])
        {
            STRNCPY(lastKey,key);
            STRNCPY(lastValue,value);
            if (kpxfer.Global == (BST_CHECKED))
            {
                m->GlobalSetKey(key,value);
            }
            m->currentEntity()->setKey(key,value);
        }
        kpWindow->calcViewSize();
    }
}

//WPropertyGrid *wprop = 0;
LRESULT EntityView::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_CREATE:
        SetupWindow();
        break;

    case WM_MOUSEWHEEL:

        SetFocusUnderMouse(hwnd);
        break;

    case WM_COMMAND:
        //combos and lists
        switch(HIWORD(wParam))
        {
        case CBN_SELCHANGE:
            if(LOWORD(wParam)==IDTE_BROWSER)
            {
                EvCBNSelChange();
                return 0;
            } /*else if(LOWORD(wParam)==IDTE_KPVIEW) {
				EvKPListChange();
				return 0;
			}*/
            break;
        case LBN_DBLCLK:
            if(LOWORD(wParam)==IDTE_KPVIEW)
            {
                EvKPListDoubleClick();
                return 0;
            }
        }

        //buttons etc
        switch(LOWORD(wParam))
        {
        case GAME_0:
            Mode0();
            return 0;
        case GAME_1:
            Mode1();
            return 0;
        case GAME_2:
            Mode2();
            return 0;
        case DM_BUTTON:
            ToggleModels();
            return 0;
        case TM_BUTTON:
            ToggleTextures();
            return 0;
        case TA_BUTTON:
            ToggleAnimate();
            return 0;
        case TW_BUTTON:
            ToggleWireEnts();
            return 0;
        case IDHELPBUTTON:
            clickHelp();
            return 0;
        case IDTB_ADD:
            addPair();
            return 0;
        case IDTB_DEL:
            delPair();
            return 0;
        case IDTB_CREATEENTITY:
            doubleClickEntity();
            return 0;
        case IDTCB_FLAGSBASE+0:
            setFlags();
            return 0;
        case IDTCB_FLAGSBASE+ 1:
            setFlags();
            return 0;
        case IDTCB_FLAGSBASE+ 2:
            setFlags();
            return 0;
        case IDTCB_FLAGSBASE+ 3:
            setFlags();
            return 0;
        case IDTCB_FLAGSBASE+ 4:
            setFlags();
            return 0;
        case IDTCB_FLAGSBASE+ 5:
            setFlags();
            return 0;
        case IDTCB_FLAGSBASE+ 6:
            setFlags();
            return 0;
        case IDTCB_FLAGSBASE+ 7:
            setFlags();
            return 0;
        case IDTCB_FLAGSBASE+ 8:
            setFlags();
            return 0;
        case IDTCB_FLAGSBASE+ 9:
            setFlags();
            return 0;
        case IDTCB_FLAGSBASE+10:
            setFlags();
            return 0;
        case IDTCB_FLAGSBASE+11:
            setFlags();
            return 0;
        case IDTCB_FLAGSBASE+12:
            setFlags();
            return 0;
        case IDTCB_FLAGSBASE+13:
            setFlags();
            return 0;
        case IDTCB_FLAGSBASE+14:
            setFlags();
            return 0;
        case IDTCB_FLAGSBASE+15:
            setFlags();
            return 0;
        case IDTCB_FLAGSBASE+16:
            setFlags();
            return 0;
        case IDTCB_FLAGSBASE+17:
            setFlags();
            return 0;
        case IDCOLOR:
            ClickColor();
            return 0;
        case ID0:
            add0();
            return 0;
        case ID45:
            add45();
            return 0;
        case ID90:
            add90();
            return 0;
        case ID135:
            add135();
            return 0;
        case ID180:
            add180();
            return 0;
        case ID225:
            add225();
            return 0;
        case ID270:
            add270();
            return 0;
        case ID315:
            add315();
            return 0;
        case IDUP:
            addUp();
            return 0;
        case IDDOWN:
            addDown();
            return 0;
        case IDCB_SKINUP:
            skinUp();
            return 0;
        case IDCB_SKINDOWN:
            skinDown();
            return 0;
        }
        break;
    case WM_SIZE:
        bounds.cx = (short)LOWORD(lParam);
        bounds.cy = (short)HIWORD(lParam);
        break;
    }
    return TCWindow::WndProc(msg,wParam,lParam);
}

void EntityView::FixString(char *in, char *out)
{
    while (*in)
    {
        if (*in != '\"')
        {
            *out = *in;
        }
        else
        {
            *out = '\'';
        }
        in++;
        out++;
    }
    *out = 0;
}

void EntityView::skinUp()
{
    int i = entity_browser_i->GetSelIndex();
    EntityClass *obj = entity_classes_i->objectAt(i);
    if (obj)
    {
        if (obj->mdl)
        {
            if (obj->mdl->numskins > 1)
            {
                obj->curskin++;
                if (obj->curskin >= obj->maxskin)
                    obj->curskin = 0;

                set.redrawedit = 1;
                set.redrawxy = 1;

                Show_Frame("Next skin chosen...",true);
            }
        }
        if (obj->mdlQ2)
        {
            if (obj->mdlQ2->num_skins > 1)
            {
                obj->curskin++;
                if (obj->curskin >= obj->maxskin)
                    obj->curskin = 0;

                set.redrawedit = 1;
                set.redrawxy = 1;

                Show_Frame("Next skin chosen...",true);
            }
        }
    }
}

void EntityView::skinDown()
{
    int i = entity_browser_i->GetSelIndex();
    EntityClass *obj = entity_classes_i->objectAt(i);
    if (obj)
    {
        if (obj->mdl)
        {
            if (obj->mdl->numskins > 1)
            {
                obj->curskin--;
                if (obj->curskin < 0)
                    obj->curskin = obj->maxskin - 1;

                set.redrawedit = 1;
                set.redrawxy = 1;

                Show_Frame("Previous skin chosen...",true);
            }
        }
        if (obj->mdlQ2)
        {
            if (obj->mdlQ2->num_skins > 1)
            {
                obj->curskin--;
                if (obj->curskin < 0)
                    obj->curskin = obj->maxskin - 1;

                set.redrawedit = 1;
                set.redrawxy = 1;

                Show_Frame("Previous skin chosen...",true);
            }
        }
    }
}

bool EntityView::CanClose()
{
    if (!KillAllWindows)
        ShowWindow(SW_MINIMIZE);
    return (0 != KillAllWindows);
}

void EntityView::EvKPListDoubleClick()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    int cur = keypairview_i->GetSelIndex();

    if (cur >= 0)
    {
        epair_t	*pair = m->currentEntity()->epairs;

        for (int i = 0; i < cur && pair; i++)
            pair = pair->next;

        // now we're at the selected pair...
        if (pair)
        {
            char key[MAX_KEY], value[MAX_VALUE];

            STRNCPY(kpxfer.Key, pair->key);
            STRNCPY(kpxfer.Value, pair->value);
            kpxfer.Global = BST_UNCHECKED;

            TKPDlg dialog(frame->hwnd,&kpxfer,1);

            if (dialog.Execute() == IDOK)
            {
                FixString(kpxfer.Key,key);
                FixString(kpxfer.Value,value);

                if (key && value && key[0] && value[0])
                {
                    STRNCPY(lastKey, key);
                    STRNCPY(lastValue, value);
                    if (kpxfer.Global == BST_CHECKED)
                    {
                        m->GlobalSetKey(key,value);
                    }
                    m->currentEntity()->setKey(key,value);
                }
                calcViewSize();
                clearInputs();
            }
        }
    }
}

void EntityView::EvCBNSelChange()
{
    EntityClass *obj;
    lastSelected = entity_browser_i->GetSelIndex();
    obj = entity_classes_i->objectAt(lastSelected);
    if (!obj)
    {
        lastSelected = 0;
        return;
    }
    loadEntityComment(obj);
}

void EntityView::clickHelp()
{
    lastSelected = entity_browser_i->GetSelIndex();
    EntityClass *obj = entity_classes_i->objectAt(lastSelected);
    if (!obj)
    {
        lastSelected = 0;
        return;
    }
    char outstr[80];
    sprintf(outstr,"Help:  Entity [%s]",obj->name);
    BSPHelp(outstr,obj->comments);
}

void EntityView::RemoveFlags()
{
    int count = 12;
    if (BrowserGame == 1)
        count = 18;
    if (flags_i)
    {
        for (int i =0; i<count; i++)
        {
            if(flags_i[i]->hwnd)
                DestroyWindow(flags_i[i]->hwnd);
            delete flags_i[i];
        }
        delete[] flags_i;
        flags_i = NULL;
    }
}

void EntityView::SwitchMode(int newMode)
{
    RemoveFlags();

    BrowserGame = newMode;

    int numflags = 12;
    int fr = 4;
    int fc = 3;
    int w = set.entity_window_width;
    int r, c;

    if (BrowserGame == 1)
    {
        numflags = 18;
        fr = 6;
        SetCaption("Entities - Hexen II");
    }
    else if (BrowserGame == 2)
    {
        SetCaption("Entities - Quake II");
    }
    else
    {
        SetCaption("Entities - Quake");
    }

    int x = FlagsX;
    int y = FlagsY;

    flags_i = new WCheck* [numflags];
    char f[80];
    // four rows + 3 columns;
    for (r=0 ; r<fr ; r++)
    {
        for (c=0 ; c<fc ; c++)
        {
            if (BrowserGame == 1)
            {
                switch (c*fr+r+1)
                {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                    sprintf(f," - %d -",c*4+r+1);
                    break;
                case 9:
                    strcpy(f,"!paladin");
                    break;
                case 10:
                    strcpy(f,"!crusader");
                    break;
                case 11:
                    strcpy(f,"!necromancer");
                    break;
                case 12:
                    strcpy(f,"!assassin");
                    break;
                case 13:
                    strcpy(f,"!easy");
                    break;
                case 14:
                    strcpy(f,"!medium");
                    break;
                case 15:
                    strcpy(f,"!hard");
                    break;
                case 16:
                    strcpy(f,"!dm");
                    break;
                case 17:
                    strcpy(f,"!co-op");
                    break;
                case 18:
                    strcpy(f,"!single");
                    break;
                }
            }
            else
            {
                switch (c*fr+r+1)
                {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                    sprintf(f," - %d -",c*4+r+1);
                    break;
                case 9:
                    strcpy(f,"!easy");
                    break;
                case 10:
                    strcpy(f,"!norm");
                    break;
                case 11:
                    strcpy(f,"!hard");
                    break;
                case 12:
                    strcpy(f,"!dm");
                    break;
                }
            }
            /*
            256     =       Not paladin
            512     =       Not crusader
            1024    =       Not necromancer
            2048    =       Not assassin
            4096    =       Not easy
            8192    =       Not medium
            16384   =       Not hard
            32768   =       not deathmatch
            65536   =       not co-op
            131072  =       not single
            */
            flags_i[c*fr+r] = new WCheck(hwnd, IDTCB_FLAGSBASE+c*fr+r, f,
                                         (x+(w/fc)*c), (y+((set.entity_button_size-2)+2)*r), (w/fc), (set.entity_button_size-2));
            flags_i[c*fr+r]->Create();
            flags_i[c*fr+r]->SetFont(kpFlagFont);
        }
    }

    fillMatrix();
    newCurrentEntity(true);
    InvalidateRect(hwnd,0,TRUE);
    PaintCaption(true);
}

void EntityView::Mode0()
{
    MessageBox(hwnd,"Warning:  Changing the Entity Browser mode can lead to incorrect entity spawn flags.","BSP Entity Mode", MB_OK);
    SwitchMode(0);
}
void EntityView::Mode1()
{
    MessageBox(hwnd,"Warning:  Changing the Entity Browser mode can lead to incorrect entity spawn flags.","BSP Entity Mode", MB_OK);
    SwitchMode(1);
}
void EntityView::Mode2()
{
    MessageBox(hwnd,"Warning:  Changing the Entity Browser mode can lead to incorrect entity spawn flags.","BSP Entity Mode", MB_OK);
    SwitchMode(2);
}

EntityView::EntityView(HWND parent, char *title)
    : TCWindow(parent, 0)
{
    BrowserGame = set.game_mode;
    lastSelected = 0;

    attr.width = 250;
    attr.height = 200;

    SetCaption(title);

    set.entity_button_size = max(9,set.entity_button_size);

    kpFont = GetFont("arial",-set.entity_font_size);
    kpFlagFont = GetFont("arial",-(set.entity_font_size - 2)); // two smaller

    flags_i = NULL;
}

//WindowPlacement
HWND EntityView::WP_GetHwnd()
{
    return hwnd;
}
const char *EntityView::WP_WindowName()
{
    return "Entity";
}

void EntityView::SetupWindow()
{
    memset(&kpxfer,0,sizeof(kpxfer));

    int y = 2;
    int x = 2;
    int w = set.entity_window_width;
    RECT sz;
    GetClientRect(hwnd,&sz);

    //TODO TEST
//	wprop = new WPropertyGrid(hwnd, 100, w, 20, 200, 200);
//	wprop->Create();


    bounds.cx = sz.right-sz.left;
    bounds.cy = sz.bottom-sz.top;

    int useW = w - 14;

    entity_browser_i = new WCombo(hwnd, IDTE_BROWSER, x, y, useW, max(bounds.cy-10, 300));
    entity_browser_i->attr.dwStyle |= CBS_DROPDOWNLIST;
    entity_browser_i->Create();
    entity_browser_i->SendMessage(WM_SETFONT,(WPARAM)kpFont,0);
    ::SetFocus(entity_browser_i->hwnd);

    helpB = new WButton(hwnd, IDHELPBUTTON, const_cast<char *> ("?"), x + useW + 2, y, 16, 22);
    helpB->Create();
    helpB->SendMessage(WM_SETFONT,(WPARAM)kpFont,0);

    y += set.entity_button_size + 11;

    entity_comment_i = new WEdit(hwnd, IDTE_COMMENT, const_cast<char *> (""), x, y, w, set.entity_comment_size,0,1);
    entity_comment_i->attr.dwStyle |= WS_TABSTOP | ES_READONLY | WS_VSCROLL;
    if (set.entity_inline_help)
        entity_comment_i->Create();
    entity_comment_i->SendMessage(WM_SETFONT,(WPARAM)kpFont,0);

    if (set.entity_inline_help)
        y += set.entity_comment_size + 5;

    keypairview_i = new TBSPListBox(
        hwnd,
        IDTE_KPVIEW,
        x,
        y,
        useW,
        set.entity_keypair_size);

    keypairview_i->Create();
    keypairview_i->SendMessage(WM_SETFONT,(WPARAM)kpFont,0);

    addp = new WButton(hwnd,IDTB_ADD, const_cast<char *> ("+"), x + useW + 2 ,y,14,14);
    addp->Create();
    addp->SendMessage(WM_SETFONT,(WPARAM)kpFont,0);
    delp = new WButton(hwnd,IDTB_DEL, const_cast<char *> ("-"), x + useW + 2, y+16,14,14);
    delp->Create();
    delp->SendMessage(WM_SETFONT,(WPARAM)kpFont,0);

    y += (set.entity_keypair_size-8);
    int wgap = ((set.entity_button_size*3)/2)+2;
    int hgap = set.entity_button_size;

    b0    = new WButton(hwnd,ID0   ,const_cast<char *> ("0"), x+wgap*2, y+hgap*1, wgap, hgap);
    b45   = new WButton(hwnd,ID45  ,const_cast<char *> ("45"), x+wgap*2, y+hgap*0, wgap, hgap);
    b90   = new WButton(hwnd,ID90  ,const_cast<char *> ("90"), x+wgap*1, y+hgap*0, wgap, hgap);
    b135  = new WButton(hwnd,ID135 ,const_cast<char *> ("135"), x+wgap*0, y+hgap*0, wgap, hgap);
    b180  = new WButton(hwnd,ID180 ,const_cast<char *> ("180"), x+wgap*0, y+hgap*1, wgap, hgap);
    b225  = new WButton(hwnd,ID225 ,const_cast<char *> ("225"), x+wgap*0, y+hgap*2, wgap, hgap);
    b270  = new WButton(hwnd,ID270 ,const_cast<char *> ("270"), x+wgap*1, y+hgap*2, wgap, hgap);
    b315  = new WButton(hwnd,ID315 ,const_cast<char *> ("315"), x+wgap*2, y+hgap*2, wgap, hgap);
    bUp   = new WButton(hwnd,IDUP  ,const_cast<char *> ("Up"), x+wgap*3, y+hgap*0, wgap, hgap);
    bDown = new WButton(hwnd,IDDOWN,const_cast<char *> ("Dn"), x+wgap*3, y+hgap*2, wgap, hgap);

    b0   ->Create();
    b45  ->Create();
    b90  ->Create();
    b135 ->Create();
    b180 ->Create();
    b225 ->Create();
    b270 ->Create();
    b315 ->Create();
    bUp  ->Create();
    bDown->Create();

    b0->SendMessage(WM_SETFONT,(WPARAM)kpFlagFont,0);
    b45->SendMessage(WM_SETFONT,(WPARAM)kpFlagFont,0);
    b90->SendMessage(WM_SETFONT,(WPARAM)kpFlagFont,0);
    b135->SendMessage(WM_SETFONT,(WPARAM)kpFlagFont,0);
    b180->SendMessage(WM_SETFONT,(WPARAM)kpFlagFont,0);
    b225->SendMessage(WM_SETFONT,(WPARAM)kpFlagFont,0);
    b270->SendMessage(WM_SETFONT,(WPARAM)kpFlagFont,0);
    b315->SendMessage(WM_SETFONT,(WPARAM)kpFlagFont,0);
    bUp->SendMessage(WM_SETFONT,(WPARAM)kpFlagFont,0);
    bDown->SendMessage(WM_SETFONT,(WPARAM)kpFlagFont,0);

    color = new WButton(hwnd, IDCOLOR, const_cast<char *> ("Clr"), x+wgap*1,y+hgap*1,wgap,hgap);
    color->Create();
    color->SendMessage(WM_SETFONT,(WPARAM)kpFont,0);

    CreateEntity = new WButton(hwnd, IDTB_CREATEENTITY, const_cast<char *> ("Make Entity"), (x+4*wgap+5), y, (w-(4*wgap+5)), set.entity_button_size+1);
    CreateEntity->Create();
    CreateEntity->SendMessage(WM_SETFONT,(WPARAM)kpFont,0);

    ts3 = new WStatic(hwnd, TC_TEXT1, const_cast<char *> ("Current:"), x+4*wgap+5,y+set.entity_button_size+1,w-(4*wgap+5),2*set.entity_button_size-1);
    ts3->Create();
    ts3->SendMessage(WM_SETFONT,(WPARAM)kpFont,0);

    y += 3*set.entity_button_size+5;

    FlagsX = x;
    FlagsY = y;

    SwitchMode(set.game_mode);

    selectClass(const_cast<char *> ("worldspawn"));
}

EntityView::~EntityView()
{
//	delete wprop;	//TODO TEST

    delete color;
    lastSelected = 0;
    delete entity_browser_i;
    delete entity_comment_i;
    RemoveFlags();
    delete keypairview_i;
    delete addp;
    delete delp;
    delete CreateEntity;
    delete b0;
    delete b45;
    delete b90;
    delete b135;
    delete b180;
    delete b225;
    delete b270;
    delete b315;
    delete bUp;
    delete bDown;
    delete ts3;

    delete entity_classes_i;

    DeleteObject(kpFont);
    DeleteObject(kpFlagFont);
}

void EntityView::ClickColor()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    char *v = m->currentEntity()->valueForQKey(const_cast<char *> ("_color"));
    int r = 255;
    int g = 255;
    int b = 255;
    if (v && *v)
    {
        float fr, fg, fb;
        if (sscanf(v,"%f %f %f",&fr, &fg, &fb) == 3)
        {
            r = (int)(255.0f*fr + 0.5);
            g = (int)(255.0f*fg + 0.5);
            b = (int)(255.0f*fb + 0.5);
        }
    }

    ColorDialog cd(hwnd, RGB(r,g,b));

    if (cd.Show())
    {
        float fr = (float)GetRValue(cd.result)/255.0f;
        float fg = (float)GetGValue(cd.result)/255.0f;
        float fb = (float)GetBValue(cd.result)/255.0f;

        m->currentEntity()->setKey(const_cast<char *> ("_color"), Str(const_cast<char *> ("%.3f %.3f %.3f"), fr, fg, fb));
        clearInputs();
        calcViewSize();
        Show_Frame("Color set...",true);
    }
}

void EntityView::calcViewSize()
{
    if (set.Map_Read)
        drawSelf();
}
void EntityView::drawSelf()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    epair_t	*pair = m->currentEntity()->epairs;
    keypairview_i->ClearList();
    for (int i = 0 ; pair ; pair=pair->next, i++)
    {
        keypairview_i->InsertString(Str(const_cast<char *> ("\"%s\" - \"%s\""), pair->key, pair->value), i);
    }
    char *classname = m->currentEntity()->classname;
    if (classname && *classname)
    {
        ts3->SetText(Str(const_cast<char *> ("Current:  %s"), classname));
    }
}
void EntityView::ReloadPropertyView()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    epair_t	*pair = m->currentEntity()->epairs;
    char *classname = m->currentEntity()->classname;
    if (classname && *classname)
    {
        ts3->SetText(Str(const_cast<char *> ("Current:  %s"), classname));
    }
}
//	Load the TEXT object with the entity comment
void EntityView::loadEntityComment(EntityClass *obj)
{
    entity_browser_i->SetText(obj->name);
    if (set.entity_inline_help)
        entity_comment_i->SetCaption(obj->comments);
}

void EntityView::initEntities()
{
    char path[MAX_PATH];
    STRNCPY(path,set.entity_directory); //[project_i getProgDirectory];

    entity_classes_i = new EntityClassList;
    entity_classes_i->initForSourceDirectory(path);
    loadEntityComment(entity_classes_i->objectAt(lastSelected));
}

void EntityView::selectEntity()
{
    lastSelected = entity_browser_i->GetSelIndex();
    loadEntityComment(entity_classes_i->objectAt(lastSelected));
}

void EntityView::doubleClickEntity()
{
    if (!set.Map_Read) return;

    map_i[set.curmap]->makeEntity();
}

char *EntityView::spawnName()
{
    return entity_classes_i->objectAt(lastSelected)->getclassname();
}

//
//	Flush entity classes & reload them!
//
void EntityView::reloadEntityClasses()
{
    //	Free all entity info in memory...
    int c = entity_classes_i->numElements;
    for (int i = 0; i < c; i++)
    {
        delete entity_classes_i->objects[i];
        entity_classes_i->objects[i] = NULL;
    }
    entity_classes_i->numElements = 0;
    delete[] entity_classes_i->objects;
    entity_classes_i->objects = NULL;
    delete entity_classes_i;

    entity_classes_i = new EntityClassList;
    entity_classes_i->initForSourceDirectory(set.entity_directory);

    lastSelected = 0;

    EntityClass *ent = entity_classes_i->objectAt(lastSelected);

    fillMatrix(); // fill in the browser stuff...

    loadEntityComment(ent);
    newCurrentEntity(false);
}

void EntityView::selectClass(char *cl)
{
    EntityClass *classent = entity_classes_i->classForName(cl);
    if (!classent)
        return;
    lastSelected = entity_classes_i->indexOf(classent);

    if (lastSelected < 0)
    {
        lastSelected = 0;
        entity_browser_i->SetText(cl);
    }
    loadEntityComment(classent);
}

void EntityView::newCurrentEntity(bool allowWorld)
{
    if (!set.Map_Read) return;

    EntityClass *classent;
    WCheck *cell;
    char	*classname;
    int		r, c;
    char	*flagname;
    int flags = 0;

    Entity *ent = map_i[set.curmap]->currentEntity();

    if (!ent)
    {
        map_i[set.curmap]->current = map_i[set.curmap]->world;
        ent = map_i[set.curmap]->currentEntity();
        if (!ent)
            return;
    }

    classname = ent->classname;
    if (ent != map_i[set.curmap]->world || allowWorld)
        selectClass(classname);	// don't reset for world
    classent = entity_classes_i->classForName(classname);
    flagname = ent->valueForQKey(const_cast<char *> ("spawnflags"));

    if (flagname)
        flags = atoi(flagname);

    int fr = 4;
    int fc = 3;
    if (BrowserGame == 1)
    {
        fr = 6;
        fc = 3;
    }

    for (r=0 ; r<fr ; r++)
    {
        for (c=0 ; c<fc ; c++)
        {
            cell = flags_i[c*fr+r];
            if ((c*fr + r) < 8)
            {
                if(flagname = classent->getflagName(c*fr + r))
                {
                    strlwr(flagname);
                    cell->SetCaption(flagname);
                }
                else
                {
                    cell->SetCaption(const_cast<char *> (""));
                }
            }
            cell->SetCheck((flags & (1 << ((c*fr)+r))) > 0);
        }
    }
    calcViewSize();
    ReloadPropertyView();
}

//
//	Clicked in the Keypair view - set as selected
//
void EntityView::clearInputs()
{
    memset(&kpxfer,0,sizeof(kpxfer));
}

void EntityView::addPair()
{
    if (!set.Map_Read) return;

    char key[MAX_KEY], value[MAX_VALUE];

    *kpxfer.Key = 0;
    *kpxfer.Value = 0;
    kpxfer.Global = BST_UNCHECKED;

    TKPDlg dialog(hwnd,&kpxfer,0);

    if (dialog.Execute() == IDOK)
    {
        FixString(kpxfer.Key,key);
        FixString(kpxfer.Value,value);

        if (key && value && *key && *value)
        {
            STRNCPY(lastKey,key);
            STRNCPY(lastValue,value);
            if (kpxfer.Global == BST_CHECKED)
            {
                map_i[set.curmap]->GlobalSetKey(key,value);
            }
            map_i[set.curmap]->currentEntity()->setKey(key,value);
        }
        calcViewSize();
        clearInputs();
    }
    set.redrawxy = 1;
    Show_Frame("Add key...",true);
}

void EntityView::delPair()
{
    if (!set.Map_Read)
        return;
    // Figure where we are,
    // Set tempKey and tempValue
    int idx = keypairview_i->GetSelIndex();

    if (idx < 0 || idx > keypairview_i->GetCount())
        return;

    epair_t	*pair = map_i[set.curmap]->currentEntity()->epairs;
    for (int i = 0; i < idx && pair; i++)
        pair = pair->next;

    // now we're at the selected pair...
    if (pair)
    {
        strncpy(tempKey,pair->key,MAX_KEY);
        strncpy(tempValue,pair->value,MAX_VALUE);

        keypairview_i->Cut();
    }
    set.redrawxy = 1;
    Show_Frame("Delete key...",true);
}

void EntityView::add0()
{
    setAngle(0);
}
void EntityView::add45()
{
    setAngle(45);
}
void EntityView::add90()
{
    setAngle(90);
}
void EntityView::add135()
{
    setAngle(135);
}
void EntityView::add180()
{
    setAngle(180);
}
void EntityView::add225()
{
    setAngle(225);
}
void EntityView::add270()
{
    setAngle(270);
}
void EntityView::add315()
{
    setAngle(315);
}
void EntityView::addUp()
{
    setAngle(-1);
}
void EntityView::addDown()
{
    setAngle(-2);
}
void EntityView::ToggleModels()
{
    if (!set.Map_Read)
        return;

    set.draw_models ^= 1;
    set.redrawedit = 1;
    set.redrawxy = 1;

    Show_Frame("Model draw toggled...",true);
}

void EntityView::ToggleWireEnts()
{
    if (!set.Map_Read)
        return;

    set.wire_ents ^= 1;
    set.redrawedit = 1;
    set.redrawxy = 1;

    Show_Frame("Entity wire frames toggled...",true);
}

void EntityView::ToggleAnimate()
{
    if (!set.Map_Read)
        return;

    set.animate_models ^= 1;
    set.redrawedit = 1;
    set.redrawxy = 1;

    Show_Frame("Model animation toggled...",true);
}

void EntityView::ToggleTextures()
{
    if (!set.Map_Read)
        return;

    set.texture_models ^= 1;
    set.redrawedit = 1;
    set.redrawxy = 1;

    Show_Frame("Model texturing toggled...",true);
}
//
//	Set the key/value fields to "angle <button value>"
//
void EntityView::setAngle(int angle)
{
//	const char *title;
    char	value[20];

    sprintf(value,"%d",angle);

    map_i[set.curmap]->currentEntity()->setKey(const_cast<char *> ("angle"), value);
    clearInputs();
    calcViewSize();
    set.redrawxy = 1;
    set.redrawedit = 1;

    Show_Frame("Angle set...",true);
}

void EntityView::setFlags()
{
    if (!set.Map_Read)
        return;

    int flags = 0;

    clearInputs();

    int fr = 4;
    int fc = 3;
    if (BrowserGame == 1)
    {
        fr = 6;
    }

    for (int r=0 ; r<fr ; r++)
    {
        for (int c=0 ; c<fc ; c++)
        {
            WCheck *cell = flags_i[c*fr+r];
            flags |= (cell->GetCheck() << (c*fr + r));
        }
    }

    if (!flags)
    {
        map_i[set.curmap]->currentEntity()->removeKeyPair(const_cast<char *> ("spawnflags"));
    }
    else
    {
        char str[80];
        sprintf(str, "%i", flags);
        map_i[set.curmap]->currentEntity()->setKey(const_cast<char *> ("spawnflags"), str);
    }
    calcViewSize();

    set.redrawxy = 1;
    Show_Frame("Entity flags updated...",true);
}

//	Fill the Entity browser
int EntityView::fillMatrix()
{
    EntityClass *object;
    int mx = entity_classes_i->numElements;
    int i = 0;
    entity_browser_i->ClearList();
    while (mx--)
    {
        object = entity_classes_i->objectAt(i);
        entity_browser_i->InsertString(object->getclassname(),i);
        i++;
    }
    return i;
}

