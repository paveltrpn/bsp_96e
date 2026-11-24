#include "global.h"

unsigned char pixdata[256];
int d_red, d_green, d_blue;

int needredraw = 1;
int inSelect = 0;

TColorMap *texColorMap;

bool suppress_gets = false;


void UpdateTextureTransFlags(face_t *f)
{
    //set sky and trans to true
    if(f->texture.flags & SURF_SKY || !strnicmp(f->texture.texture,"SKY",3))
    {
        f->transparent = true;
        f->sky = true;
    }
    //set trans only to true
    else if( (f->texture.contents & CONTENTS_TRANS_MASK) || (f->texture.flags & SURF_TRANS_MASK) ||
             *f->texture.texture == '*' ||//water
             !strnicmp(f->texture.texture,"CLIP",4) ||
             !strnicmp(f->texture.texture,"TRIGGER",7) ||
             !strnicmp(f->texture.texture,"HINT",4) ||
             !strnicmp(f->texture.texture,"SKIP",4))
    {
        f->transparent = true;
        f->sky = false;
    }
    else
    {
        f->transparent = false;
        f->sky = false;
    }
}

LRESULT TTextureWindow::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_CREATE:
        SetupWindow();
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {

        case ID_COMBOBOX1:
            switch(HIWORD(wParam))
            {
            case CBN_SELCHANGE:
                EvCBNSelChange();
                return 0;
            case CBN_KILLFOCUS:
                ChangeTexture();
                return 0;
            }
            break;

        case WAD_TO_WAL:
            WadToWal();
            return 0;
        case SELECT_ALL:
            SelectAll();
            return 0;
        case DESELECT_ALL:
            DeselectAll();
            return 0;
        case SELECT_USED:
            SelectUsed();
            return 0;
        case SAVE_SELECTED:
            SaveSelected();
            return 0;
        case SAVE_WAD:
            FakeSave();
            return 0;
        case CLIP_COPY:
            CopyToClipboard();
            return 0;
        case SCAN_NEW_WAD:
            ScanNewWad();
            return 0;
        case SCAN_NEW_WALDIR:
            ScanNewWalDirectory();
            return 0;
        case SCAN_WAL_PAKQ2:
            ScanNewWalFromQ2Pak();
            return 0;
        case SCAN_WAL_PAK:
            ScanNewWalFromSelectedPak();
            return 0;
        case SCAN_NEW_WAL:
            ScanNewTexture();
            return 0;
        case SCAN_IN_USE:
            ScanInUse();
            return 0;
        case UNLOAD_WAD:
            UnloadWad();
            return 0;
        case CLIP_COPY_IMAGE:
            CopyImageClipboard();
            return 0;
        case CLIP_PASTE:
            PasteFromClipboard();
            return 0;
        case ID_SBASE+10:
            ResetButton();
            return 0;
        case ID_SBASE+11:
            StretchButton();
            return 0;
        case ID_SBASE+12:
            TDButton();
            return 0;
        case ID_SBASE+13:
            TUButton();
            return 0;
        case IDB_CALC:
            ClickFrequents();
            return 0;
        case IDB_CALCUSAGE:
            CalcUsage();
            return 0;
        case IDB_FAV:
            ToggleFav();
            return 0;
        case IDB_FRQ:
            ToggleFrq();
            return 0;
        case IDB_NAME:
            ToggleNames();
            return 0;
        case 10400:
            StartAnimation();
            return 0;
        case 10401:
            StopAnimation();
            return 0;
        case 10402:
            ToggleSmalls();
            return 0;
        case 10403:
            ToggleSky();
            return 0;
        case 10404:
            SetWH();
            return 0;
        case 10405:
            SetScaleStep();
            return 0;
        case 10406:
            ToggleAlwaysOnTop();
            return 0;
        case 10407:
            SetShiftStep();
            return 0;
        case 10408:
            SetRotateStep();
            return 0;
        case GL_SETTINGS:
            ClickglBspSettings();
            return 0;
        }
        break;
    case WM_ERASEBKGND:
    {
        HDC hdc = GetDC(hwnd);
        EvEraseBkgnd(hdc);
        ReleaseDC(hwnd, hdc);
    }
    return 0;
    case WM_VSCROLL:
        if(Scroller.VScroll(wParam, 1))
        {
            Scroller.Update();
            RedrawContents();
        }
        return 0;
    case WM_PAINT:
    {
        TCWindow::WndProc(msg,wParam,lParam);
        Paint();
    }
    return 0;
    case WM_SIZE:
    {
        SIZE sz;
        sz.cx = (short)LOWORD(lParam);
        sz.cy = (short)HIWORD(lParam);
        EvSize(wParam,&sz);
    }
    case WM_LBUTTONDBLCLK:
    {
        MAKEPOINT(pt, lParam);
        EvLButtonDblClk(wParam, &pt);
    }
    return 0;
    case WM_LBUTTONDOWN:
    {
        SetFocus();		//in case focus is left on combo, main textwnd needs focus for mousewheel

        MAKEPOINT(pt, lParam);
        EvLButtonDown(wParam, &pt);
    }
    return 0;
    case WM_LBUTTONUP:
    {
        MAKEPOINT(pt, lParam);
        EvLButtonUp(wParam, &pt);
    }
    return 0;
    case WM_RBUTTONDOWN:
    {
        MAKEPOINT(pt, lParam);
        EvRButtonDown(wParam, &pt);
    }
    return 0;
    case WM_MOUSEMOVE:
    {
        MAKEPOINT(pt, lParam);
        EvMouseMove(wParam, &pt);
    }
    return 0;

    case WM_TIMER:
        EvTimer(wParam);
        return 0;
    case WM_MOUSEWHEEL:
        MouseWheel(wParam, lParam);
        return 0;
    }

    return TCWindow::WndProc(msg,wParam,lParam);
}

void TTextureWindow::EvLButtonDblClk(UINT /*modKeys*/, LPPOINT point)
{
    if (!set.Map_Read)
        return;

    RECT border;

    GetClientRect(hwnd, &border);
    border.top = 0;
    border.bottom = noffset;
    if (PtInRect(&border,*point))
        ResetButton();
}

//redraw client portion at top of texture window.
void TTextureWindow::RedrawClient(HDC hdc)
{

    RECT rc;
    GetClientRect(hwnd, &rc);

    rc.bottom = noffset;

    HBRUSH btnface = GetSysColorBrush(COLOR_3DFACE);
    FillRect(hdc, &rc, btnface);
    DeleteObject(btnface);

    HGDIOBJ oldfont = SelectObject(hdc,set.font9);
    SetBkMode(hdc, TRANSPARENT);
    TextOut(hdc,LABEL_WIDTH+4, 1, texStatic, strlen(texStatic));
    SelectObject(hdc, oldfont);
}

void TTextureWindow::EvTimer(UINT timerId)
{
    if (!set.Map_Read || !BSP_Active)
        return;
    if (!set.tex_animate)
    {
        KillTimer(hwnd,2);
        return;
    }
    if (timerId != 2)
        return;
    RedrawAnimated();
}

void TTextureWindow::MouseWheel(WPARAM wParam, LPARAM /*lParam*/)
{
    if(SetFocusUnderMouse(hwnd))
        return;

    if((short)HIWORD(wParam) >= 0)
    {
        if(wParam & MK_CONTROL)
        {
            TDButton();
        }
        else
        {
            SendMessage(WM_VSCROLL,SB_LINEUP,0);
            SendMessage(WM_VSCROLL,SB_LINEUP,0);
            SendMessage(WM_VSCROLL,SB_LINEUP,0);
            Scroller.Update();
        }
    }
    else
    {
        if(wParam & MK_CONTROL)
        {
            TUButton();
        }
        else
        {
            SendMessage(WM_VSCROLL,SB_LINEDOWN,0);
            SendMessage(WM_VSCROLL,SB_LINEDOWN,0);
            SendMessage(WM_VSCROLL,SB_LINEDOWN,0);
        }
    }
    Invalidate();
    return;
}

//
bool TTextureWindow::EvEraseBkgnd(HDC hdc)
{
    RECT r;
    GetClientRect(hwnd, &r);

    RedrawClient(hdc);
    HPEN pen = CreatePen(PS_SOLID,1,GetSysColor(COLOR_BTNTEXT));
    HGDIOBJ oldpen = SelectObject(hdc, pen);
    MoveToEx(hdc, r.left,noffset, 0);
    LineTo(hdc, r.right,noffset);
    SelectObject(hdc,oldpen);
    DeleteObject(pen);
    return true;
}


//...
void UnloadTextures()
{
    for (int i = 0; i < set.nummaps; i++)
    {
        if (!map_i[i])
            continue;
        map_i[i]->UnloadTextures();
    }
}
int GetPaletteIndex(int pSize, unsigned char *pal, COLORREF color)
{
    if (!pal)
        return 0;

    static int r, g, b;
    r = GetRValue(color);
    g = GetGValue(color);
    b = GetBValue(color);

    int bestdiff = 2*(r*r + g*g + b*b);
    unsigned char bestindex = 0;

    const int stop = pSize-1;
    if(4 == PALSTEP)
    {
        for (int i = 0 ; i <= stop; i++)
        {
            int dr = *pal++ - r;
            int dg = *pal++ - g;
            int db = *pal++ - b;
            pal++;

            int diff = dr*dr + dg*dg + db*db;

            if (diff <= bestdiff)
            {
                if (!diff)
                    return (unsigned char)i;

                bestdiff = diff;
                bestindex = (unsigned char)i;
            }
        }
    }
    else
    {
        for (int i = 0 ; i <= stop; i++)
        {
            int dr = *pal++ - r;
            int dg = *pal++ - g;
            int db = *pal++ - b;

            int diff = dr*dr + dg*dg + db*db;

            if (diff <= bestdiff)
            {
                if (!diff)
                    return (unsigned char)i;

                bestdiff = diff;
                bestindex = (unsigned char)i;
            }
        }
    }
    return bestindex;
}
// unused, blends two paletted colors together
int BlendPal(int index1, int index2)
{
    int a = max(0,min(255,index1));
    int b = max(0,min(255,index2));

    int dr = set.pal[PALSTEP*a+0] + set.pal[PALSTEP*b+0];
    int dg = set.pal[PALSTEP*a+1] + set.pal[PALSTEP*b+1];
    int db = set.pal[PALSTEP*a+2] + set.pal[PALSTEP*b+2];

    return GetPaletteIndex(256, set.pal, RGB(dr, dg, db));
}

int QCompare(const char *str1, const char *str2)
{
    char num1[64];
    char num2[64];

    char s1[LUMP_NAME_LENGTH_MAX];
    char s2[LUMP_NAME_LENGTH_MAX];

    strncpy(s1,str1,sizeof(s1));
    s1[sizeof(s1)-1] = 0;
    strncpy(s2,str2,sizeof(s2));
    s2[sizeof(s2)-1] = 0;

    char *st1 = s1, *st2 = s2;
    char *n1 = num1, *n2 = num2;
    int count = 0;
    if ((strlen(st1) > 2) && !strnicmp(st1,"+A",2))
    {
        st1++;
        st1++;
    }
    else
    {
        while ((count < LUMP_NAME_LENGTH) && st1[0] && ((st1[0] > 'Z') || (st1[0] < 'A')))
        {
            if (isdigit(*st1))
                *n1++ = *st1;

            st1++;
            count++;
        }
        *n1 = 0;
    }

    count = 0;
    if ((strlen(st2) > 2) && !strnicmp(st2,"+A",2))
    {
        st2 += 2;
    }
    else
    {
        while ((count < LUMP_NAME_LENGTH) && *st2 && ((*st2 > 'Z') || (*st2 < 'A')))
        {
            if (isdigit(*st2))
            {
                *n2++ = *st2;
            }
            st2++;
            count++;
        }
        *n2 = '\0';
    }

    int temp = strcmpi(st1,st2);

    if (temp)
        return temp;

    // if equal, do some more...
    st1 = s1;
    st2 = s2;

    if (((n1-num1) > 0) && ((n2 - num2) > 0))
    {
        if (atoi(num1) > atoi(num2))
        {
            temp = 1;
        }
        else if (atoi(num1) < atoi(num2))
        {
            temp = -1;
        }
        else
        {
            temp = 0;
        }
    }
    else
    {
        temp = strcmpi(st1,st2);
    }
    // do more checking...include the starting stuff...
    return temp;
}

textureEntry::textureEntry()
{
    *dirName = 0;
}

textureEntry::textureEntry(char *name)
{
    STRNCPY(dirName,name);
}

void TTextureWindow::ScanNewWalFromQ2Pak()
{
    ScanNewWalFromPak(false);
}

void TTextureWindow::ScanNewWalFromSelectedPak()
{
    ScanNewWalFromPak(true);
}

void TTextureWindow::ScanNewWalFromPak(bool prompt)
{
    const char err_title[] = "BSP Scan Pak";

    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    if (!m)
        return;

    char dirname[256] = "";

    if (prompt)
    {
        char filename[256];

        OPENFILENAME ofn;
        memset(&ofn,0,sizeof(OPENFILENAME));
        ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
        ofn.hwndOwner = frame->hwnd;
        ofn.lpstrFilter = "Pak files (*.pak)\0*.pak\0";
        ofn.lpstrDefExt = "pak";
        ofn.lpstrFile = filename;
        ofn.nMaxFile = 256;
        ofn.lpstrTitle = "Select Quake 2 Pak0.Pak File";
        ofn.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;

        if (!set.pakOk)
        {
            *filename = 0;
        }
        else
        {
            strncpy(filename,set.pak_file,sizeof(filename));
        }

        if(!GetOpenFileName(&ofn))
            return;

        STRNCPY(dirname,filename);
    }
    else
    {
        if (!set.pakOk)
            return;

        STRNCPY(dirname,set.pak_file);
    }

    // check that it's a .pak file...
    if (!file_exists(dirname))
    {
        sprintf(dirname, "Pak file does not exist [%s]...", dirname);
        MessageBox(hwnd,dirname, err_title, MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    FILE        *f;
    pakheader_t pakheader;
    pakentry_t  *pakentry;
    int			numentries;
    int         i;

    f = fopen(dirname,"rb");
    if (!f)
    {
        sprintf(dirname,"Unable to open pak file\n%s",dirname);
        MessageBox(hwnd,dirname,err_title,MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    fread(&pakheader,sizeof(pakheader_t),1,f);
    if (strnicmp((char *)pakheader.magic,"PACK",4))
    {
        fclose(f);
        sprintf(dirname,"[%s] is not a valid pak file...",dirname);
        MessageBox(hwnd,dirname,err_title,MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    numentries = pakheader.dirsize / (sizeof(pakentry_t));
    fseek(f,pakheader.diroffset,SEEK_SET);

    pakentry = new pakentry_t[numentries];
    for (i=0; i<numentries; i++)
        fread(&pakentry[i],sizeof(pakentry_t),1,f);

    fclose(f);
    pakentry_t *e;

    textureEntry firstEntry;

    memset(&firstEntry,0,sizeof(textureEntry));

    char **EntryList;
    int maxEntries = 64; // maximum texture dirs...
    int EntrySize = 64;
    int curEntries = 0;
    EntryList = new char *[maxEntries];
    memset(EntryList,0,maxEntries*sizeof(char *));
    for (i = 0; i < maxEntries; i++)
    {
        EntryList[i] = new char[EntrySize];
        memset(EntryList[i],0,EntrySize * sizeof(char));
    }

    int left, right, current;
    int done, index;
    int testVal;
    char dir[256];
    char *start;
    int c = numentries;
    int ct;
    for (i= 0 ; i < c ; i++)
    {
        e = &pakentry[i];
        start = (char *)e->filename;

        if (strnicmp(start,"textures",8))
            continue;

        start += strlen("textures");
//verify dir separator
        if(*start != '/' && *start != '\\')
            continue;

        start++;

        STRNCPY(dir,start);
        start = dir;
        ct = 0;
        while (*start && *start != '\\' && *start != '/' && ct++ <= 32)
            start++;

        if (ct >= 32)  // problem, so skip...
            continue;

//verify dir separator
        if(*start != '/' && *start != '\\')
            continue;

        *start = 0;

        left = 0;        // first
        right = curEntries;   //last+1 // never gets tested...

        done = 0;
        index = -1;

        current = (left+right)/2;

        while (!done)
        {
            char *tP = EntryList[current];

            testVal = strcmpi(dir,tP);
            // check for exact match...
            if (testVal == 0)
            {
                index = current;
                break;
            }
            else if (testVal > 0)
            {
                left = current;
                current = (left+right)/2;
            }
            else if (testVal < 0)
            {
                right = current;
                current = (left+right)/2;
            }
            if ((right - left) <= 1)
            {
                if (!strcmpi(dir,EntryList[left]))
                {
                    index = left;
                }
                done = 1;
            }
        }

        if (index >= 0) // if found it, then skip
            continue;

        if (index == -1)   // not found, just insert
        {
            if (curEntries == 0)   // empty list, put in first slot
            {
                strcpy(EntryList[curEntries++],dir);
            }
            else
            {
                int slot = -1;
                int w;
                int tVal;

                for (w = 0; w < curEntries; w++)
                {
                    tVal = strcmpi(dir,EntryList[w]);
                    if (tVal < 0)
                    {
                        slot = w;
                        break;
                    }
                }
                if (slot == -1)
                    slot = curEntries;  // put at end...

                for (w = curEntries; w > slot; w--)
                    strcpy(EntryList[w], EntryList[w-1]);

                strcpy(EntryList[slot],dir);
                curEntries++;
            }
        }
    }

    if (curEntries <= 0)
    {
        sprintf(dirname,"No subdirectories in pak [%s]...",dirname);
        MessageBox(hwnd,dirname, err_title, MB_OK | MB_ICONEXCLAMATION);
        delete[] pakentry;
        for (i = 0; i < maxEntries; i++)
            delete[] EntryList[i];
        delete[] EntryList;
        return;
    }

    TScanTransferBuffer x;
    x.Directory.Clear();
    *x.caption = 0;

    for (i = 0; i < curEntries; i++)
        x.Directory.Add(EntryList[i]);


    TScanDlg dialog(hwnd,&x);
    INT_PTR retval = dialog.Execute();
    if (retval == IDOK)
    {
        strncpy(dir,x.selected,sizeof(dir));

        texWad *Wad;
        Wad = QueryWadLoaded(dir);  // scan through list!
        // if found it, just change to it...
        if (!Wad)
        {
            Wad = new texWad;
            strncpy(Wad->currentWad,dir,sizeof(Wad->currentWad)); // may get overwritten anyway...
            Wad->WadType   = QUAKE_WAD;
            Wad->bmpsCount = 0;
            Wad->textures  = NULL;
            Wad->nextWad 	= firstWad;
            firstWad 		= Wad;
            currentWadPtr 	= Wad;
            LoadPakDirectory(dir,dirname); // load it in...
        }
        m->Wads[m->numWads++] = Wad;

        UnloadTextures();

        SetUpMap(m);
        ChangeToMap(m);

        sprintf(dirname,"Directory [%s] scanned...",dir);

        set.redrawxy = 1;
        set.redrawedit = 1;
        Show_Frame(dirname,true);
        prevIndex = 0;
        ResetDefaults();
        RedrawContents();
    }

    delete[] pakentry;
    for (i = 0; i < maxEntries; i++)
        delete[] EntryList[i];
    delete[] EntryList;
}

void TTextureWindow::LoadPakDirectory(char *dir, char *pakname)
{
    int			i;//, j;
    uintptr_t size;
    texInfo_t *newTex;
    texInfo_t *currentHead;
    BITMAPINFOHEADER m_pBmInfoHeader;
    char fname[256];
    unsigned char *lump;
    miptex2_t *qtex;
    char	filename[1024];
    unsigned char *source;
    unsigned	char *dest;
    int width, height, count;
    pakheader_t pakheader;
    pakentry_t  *pakentry;
    int        numentries;

    const char *err_title = "BSP Load Directory From Pak";

    if(!dir || !pakname)
        return;

    if (!file_exists(pakname))
    {
        char outstr[256];
        sprintf(outstr,"Pak file %s does not exist!",pakname);
        MessageBox(hwnd,outstr,err_title,MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    sprintf(fname,"%s\\",dir);
    FILE *f;

    // Cycle through the .wal files!
    f = fopen(pakname,"rb");
    if (!f)
    {
        char outstr[256];
        sprintf(outstr,"Unable to open pak file\n%s",pakname);
        MessageBox(hwnd,outstr,err_title,MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    fread(&pakheader,sizeof(pakheader_t),1,f);
    numentries = pakheader.dirsize / (sizeof(pakentry_t));
    fseek(f,pakheader.diroffset,SEEK_SET);

    pakentry = new pakentry_t[numentries];
    for (i=0; i<numentries; i++)
    {
        fread(&pakentry[i],sizeof(pakentry_t),1,f);
    }
    // Quake II .wal file loader...
    currentWadPtr->WadType = QUAKEII_WAD;
    STRNCPY(currentWadPtr->currentWad,fname);
    if (currentWadPtr->bmpsCount)
        FreeTextures(currentWadPtr);

    char statstr[80] = "";
    status->SetText("",true);

    int counter = 0;

    currentWadPtr->textures = new texInfo_t; // the head...

    currentHead = currentWadPtr->textures;

    newTex = new texInfo_t;

    newTex->next = NULL;
    newTex->previous = currentHead;
    currentHead->next = newTex;

    char tempNm[LUMP_NAME_LENGTH_MAX];

    char walName[128];
    char d[128];
    char dirpart[128];
    for (i = 0; i < numentries; i++)
    {
        pakentry_t *e = &pakentry[i];
        char *start = (char *)e->filename;
        if (strnicmp(start,"textures",8))
            continue;

        start += strlen("textures");
        start++;

        strncpy(d,start,sizeof(d));
        d[sizeof(d)-1] = 0;
        start = d;
        int ct = 0;

        //find last slash.
        char *ts = d;
        int last_slash = 0;
        for( ; *ts && ct <= 32; ts++, ct++)
        {
            if(*ts == '\\' || *ts == '/')
            {
                start = ts + 1;
                last_slash = ct;
            }
        }
        if (ct >= 32)  // problem, so skip...
            continue;

        strncpy(dirpart, d, last_slash);	//get directory part
        dirpart[last_slash] = 0;

        *ts = '\0';	//force null term at end of .wal

        if (strcmpi(dirpart,dir))
            continue;

        newTex = new texInfo_t;

        strncpy(walName,start,sizeof(walName));
        walName[sizeof(walName)-1] = 0;
        // Load it in...
        //todo... maybe this is bad. just leave the filename how it is???
        StripExtension(walName);

        sprintf(statstr,"Pak [%i] %s",counter+1,walName);
        status->SetText(statstr,true);

        // load the file
        sprintf (filename, "%s/%s.wal",dir,walName);

        lump = new unsigned char[e->size];

        // jump to .wal offset
        fseek(f,e->offset,SEEK_SET);

        fread(lump,e->size,1,f);  // read in the .wal

        qtex = (miptex2_t *)lump;

        width = qtex->width;
        height = qtex->height;

        newTex->def_flags    = qtex->flags;
        newTex->def_value    = qtex->value;
        newTex->def_contents = qtex->contents;

        newTex->texType = TEX_TYPE_Q2;
        strncpy(newTex->basepath,dir,sizeof(newTex->basepath));
        newTex->basepath[sizeof(newTex->basepath)-1] = 0;

        strncpy (qtex->name, walName, sizeof(qtex->name));
        qtex->name[sizeof(qtex->name)-1] = 0;
        StripExtension (qtex->name);

        strncpy(tempNm,qtex->name,sizeof(tempNm));
        tempNm[LUMP_NAME_LENGTH-1] = '\0';

        strncpy(newTex->bmp,tempNm,sizeof(newTex->bmp));
        newTex->bmp[sizeof(newTex->bmp)-1] = 0;
        strupr(newTex->bmp);

        newTex->index = counter;

        size = (uintptr_t)width * (uintptr_t)height;
        size = size * 85/64;

        newTex->h = height;
        newTex->w = width;

        dest = new unsigned char[size+width+1];

        count = size;
        source = (unsigned char *)qtex;
        source += qtex->offsets[0];

        memcpy(dest, source, count);
        delete[] lump;

        newTex->bits = dest;
        newTex->flat = TranslateColors(256,set.pal,dest,size);

        if (newTex->isSkyQ1())
        {
            // cut down the heights...
            newTex->w /= 2;
            width /= 2;

            newTex->sky = new sky_t;
            newTex->sky->skyRow = 0;
            newTex->sky->skyCol = 0;

            newTex->sky->bits = newTex->bits;
            size = (uintptr_t)width * (uintptr_t)height;
            size = size * 85/64;
            newTex->bits = (unsigned char*)new char[size+3];
            CreateSkyBits(newTex->w,newTex->h,newTex->sky,newTex->bits);
        }
        else if (newTex->isWarp())
        {
            newTex->sky = new sky_t;
            newTex->sky->skyRow = 0;
            newTex->sky->skyCol = 0;

            newTex->sky->bits = newTex->bits;
            size = (uintptr_t)width * (uintptr_t)height;
            size = size * 85/64;
            newTex->bits = (unsigned char*)new char[size+3];
            CreateWaterBits(newTex->w,newTex->h,newTex->sky,newTex->bits);
        }

        unsigned char *thePal = (unsigned char *)set.pal;

        if (set.glBsp)
            CalcGLBits(newTex,thePal);

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
        m_pBmInfo = (BITMAPINFO*)new char[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];

        memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
        memcpy(m_pBmInfo->bmiColors,set.cTable,256*sizeof(RGBQUAD));

        newTex->dibInfo = m_pBmInfo;
        newTex->truedibInfo = NULL; // tm_pBmInfo;

        HDC hdc = GetDC(hwnd);
        SetStretchBltMode(hdc, COLORONCOLOR);

        StretchDIBits( hdc,
                       0,0,newTex->w,newTex->h,
                       0,0,newTex->w,newTex->h,
                       newTex->bits,
                       newTex->dibInfo,
                       DIB_RGB_COLORS,
                       SRCCOPY);
        ReleaseDC(hwnd,hdc);

        currentHead = currentWadPtr->textures->next; // first one
        int done = 0;

        while (currentHead && !done)
        {
            if (currentHead->next)
            {
                if (QCompare(currentHead->bmp,newTex->bmp) > 0)
                {
                    newTex->next = currentHead;
                    newTex->previous = currentHead->previous;
                    newTex->previous->next = newTex;
                    newTex->next->previous = newTex;
                    done = 1;
                }
            }
            else
            {
                // insert before this one...
                newTex->next = currentHead;
                newTex->previous = currentHead->previous;
                newTex->previous->next = newTex;
                newTex->next->previous = newTex;
                done = 1;
            }
            currentHead = currentHead->next;
        }
        counter++;
    }

    fclose(f);
    currentWadPtr->bmpsCount = counter;

    delete[] pakentry;

    status->SetText("Done...",true);

    // Is this needed???xxx FIXME
    texList->ClearList();
    if (frame->TextureList)
    {
        frame->TextureList->ClearList();
        delete[] frame->data;
        frame->data = new ListBoxData[currentWadPtr->bmpsCount];
        memset(frame->data,0,currentWadPtr->bmpsCount*sizeof(ListBoxData));
    }

    for (i = 0; i< currentWadPtr->bmpsCount; i++)
    {
        newTex = GetTextureNo(currentWadPtr,i);
        texList->InsertString(newTex->bmp,i);
        if (frame->TextureList)
        {
            frame->data[i].tx = newTex;
            strncpy(frame->data[i].description,newTex->bmp,sizeof(frame->data[i].description));
            frame->TextureList->InsertString((LPSTR)&frame->data[i],i);
        }
    }

    newTex = GetTextureNo(currentWadPtr,0);
    if (newTex)
    {
        texList->SetText(newTex->bmp);
    }
    else
    {
        texList->SetText("");
        if (frame->TextureList)
            frame->TextureList->SetText("");
    }

    if (frame->TextureList)
        frame->TextureList->SetSelIndex(0);

    UpdateScrollbar();

    prevIndex = 0;
    ResetDefaults();
    RedrawContents();
}

void TTextureWindow::WadToWal()
{
    /*	Present list of .wad files with selection, make sure that texture_path +
    	wadname (no extension) exists. if not, query and create it. Load up
    	Textures (Load Textures) with a fake name?; Go through and Write them
    	as .wal files in the directory. Unload the Loadded Textures
    */
    struct _finddata_t ffb;
    memset(&ffb,0,sizeof(ffb));

    Path dirname(const_cast<char *> ("%s/*.wad"), set.wad_directory);
    int ff = _findfirst(dirname,&ffb);
    int nomorefiles = ff < 0 ? 1 : 0;

    TScanTransferBuffer x;
    x.Directory.Clear();
    *x.caption = 0;

    int count = 0;
    while (!nomorefiles)
    {
        if (ffb.attrib & _A_SUBDIR)
        {
            nomorefiles = _findnext(ff, &ffb);
            continue;
        }
        x.Directory.Add(ffb.name);
        nomorefiles = _findnext(ff, &ffb);
        count++;
    }
    _findclose(ff);

    if (count <= 0)
    {
        sprintf(dirname,"No .wad files in wad directory [%s]...",set.wad_directory);
        MessageBox(hwnd, dirname, "BSP Wad To Wal", MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    if (TScanDlg(frame->hwnd,&x).Execute() != IDOK)
        return;

    char dir[256];
    strncpy(dir,x.selected,sizeof(dir));

    texWad *Wad = QueryWadLoaded(dir);

    if (!Wad)
    {
        Wad = new texWad;
        int saveGame = set.game_mode;
        set.game_mode = 0; // Act like quake
        int inUse = set.loadInUse;
        set.loadInUse = 0;
        AddWad(Wad,dir,dir,set.game_mode);
        set.loadInUse = inUse;
        set.game_mode = saveGame;
    }
    // Wad is loaded...

    // Get rid of .wad extension
    char s[256];
    ExtractFileBase(dir,s,true);

    // See if directory exists?
    dirname.SetPath(const_cast<char *> ("%s\\%s"), set.texture_path,s);

    ff = _findfirst(dirname,&ffb);
    nomorefiles = ff < 0 ? 1 : 0;
    _findclose(ff);

    if (nomorefiles)
        CreateDirectory(dirname,0);		//mkdir(dirname);  // create it...

    miptex2_t *qtex;
    qtex = new miptex2_t;

    texInfo_t *head, *tx;

    head = Wad->textures->next;
    int offset = sizeof(miptex2_t);
    int counter = 1;
    while (head != NULL)
    {
        tx = head;

        if (!tx || !tx->bmp || !*tx->bmp || *tx->bmp == '.')
        {
            head = head->next;
            continue;
        }

        char filename[256];
        sprintf(filename,"Writing [%s.wal] %i/%i",
                tx->bmp,(int)counter++,(int)Wad->bmpsCount);
        status->SetText(filename,true);

        memset(qtex,0,sizeof(miptex2_t));

        qtex->width  = tx->w;
        qtex->height = tx->h;

        qtex->flags    = tx->def_flags;
        qtex->value    = tx->def_value;
        qtex->contents = tx->def_contents;

        STRNCPY(qtex->name,tx->bmp);
        *qtex->animname = 0;
        int size = tx->w * tx->h;

        // wrong?
        qtex->offsets[0] = offset;
        qtex->offsets[1] = qtex->offsets[0] + size/4;
        qtex->offsets[2] = qtex->offsets[1] + size/16;
        qtex->offsets[3] = qtex->offsets[2] + size/64;

        FILE *fp = fopen(Path(const_cast<char *> ("%s/%s.wal"), dirname._path,tx->bmp), "wb");
        if (!fp)
        {
            head = head->next;
            continue;
        }
        fwrite(qtex,sizeof(miptex2_t),1,fp); // should return 1
        fwrite(tx->bits,sizeof(unsigned char),size*85/64,fp);    // same...
        fclose(fp);
        head = head->next;
    }
    delete qtex;
}

void TTextureWindow::UnloadWad()
{
    if (!set.Map_Read)
        return;

    // Scan through maps and remove textures
    TScanTransferBuffer x;
    x.Directory.Clear();
    strcpy(x.caption, "Unload Textures...");

    texWad *head = firstWad;
    int count = 0;
    char shortstr[2048];
    while (head)
    {
        if (!strnicmp(head->currentWad,"Use(",4))
            strncpy(shortstr,head->currentWad,sizeof(shortstr));
        else
            ExtractFileBase(head->currentWad,shortstr,false);

        if (!strcmpi(head->currentWad,"NONE"))
        {
            head = head->nextWad;
            continue;
        }
        x.Directory.Add(shortstr);
        count++;
        head = head->nextWad;
    }

    if (count <= 0)
    {
        MessageBox(hwnd,"No textures loaded?...", "BSP Unload textures", MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    if (TScanDlg(frame->hwnd,&x).Execute() != IDOK)
        return;

    UnloadWad(x.selected,true);

    map *m = map_i[set.curmap];

    SetUpMap(m);
    ChangeToMap(m);

    char str[256];  // temporary stuff
    sprintf(str,"Textures [%s] unloaded...",x.selected);
    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame(str,true);
    prevIndex = 0;
    ResetDefaults();
    RedrawContents();
}

void TTextureWindow::ScanInUse()
{
    if (!set.Map_Read)
        return;

    ScanInUse(map_i[set.curmap]);
    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Used textures reloaded...",true);
    prevIndex = 0;
    ResetDefaults();
    RedrawContents();
}

void TTextureWindow::ScanInUse(map *m)
{
    char checkStr[256];
    ExtractFileBase(m->filename,checkStr,false);
    char fname[256];
    sprintf(fname,"Use(%s)",checkStr);
    // Clear it if it's loaded...
    texWindow->UnloadWad(fname,false);

    texWad *Wad = new texWad;

    int SaveRead = set.Map_Read;
    set.Map_Read = 0;

    char dir[256];
    AddWad(Wad,m->filename,dir,set.game_mode);
    m->Wads[m->numWads++] = Wad;
    set.Map_Read = SaveRead;

    UnloadTextures();
    SetUpMap(m);
    ChangeToMap(m);
}

void TTextureWindow::UnloadWad(char *dir, bool verbose)
{
    char outstr[256];
    texWad *Wad = QueryWadLoaded(dir);  // scan through list!
    // if found it, just change to it...
    if (!Wad)
    {
        if (verbose)
        {
            sprintf(outstr,"Couldn't find those textures [%s]?...",dir);
            MessageBox(hwnd,outstr, "BSP Unload textures", MB_OK | MB_ICONEXCLAMATION);
        }
        return;
    }

    int i,j,k;
    int oldNum;
    map *m;
    for (i = 0; i < set.nummaps; i++)
    {
        m = map_i[i];
        if (!m)
            continue;

        m->UnloadTextures();

        oldNum = m->numWads;
        for (j = 0; j < m->numWads; j++)
        {
            if (m->Wads[j] != Wad)
                continue;

            for (k = j+1; k < m->numWads; k++)
                m->Wads[k-1] = m->Wads[k];

            j--;
            m->numWads--;
        }

        for ( ; j < oldNum; j++)
        {
            if (m->Wads[j] != NULL)
                m->Wads[j] = NULL;
        }
    }

    texWad *head;

    head = firstWad;
    texWad *pWad = NULL;
    bool found = false;
    if (head == Wad)
    {
        found = true;
        pWad = NULL;
    }
    else
    {
        while (head)
        {
            pWad = head;
            if (head->nextWad == Wad)
            {
                found = true;
                break;
            }
            head = head->nextWad;
        }
    }

    if (!found)
    {
        if (verbose)
        {
            MessageBox(hwnd,"Couldn't find right textures?...", "BSP Unload textures",
                       MB_OK | MB_ICONEXCLAMATION);
        }
        return;
    }

    // Unlink Wad...
    if (pWad)
    {
        pWad->nextWad = Wad->nextWad;
        currentWadPtr = pWad;
    }
    else
    {
        firstWad = firstWad->nextWad;
        currentWadPtr = firstWad;
    }

    // Now Kill off Wad
    if (Wad->textures)
    {
        FreeTextures(Wad);
    }

    delete Wad->textures; // cause head is left over? what about tail?
    delete Wad;
}

void TTextureWindow::NewCurrentTexture(texInfo_t *tx)
{
    if (!set.Map_Read || !tx)
        return;

    strcpy(currentTexture->texture,tx->bmp);

    if (set.game_mode == 2)
    {
        sprintf(currentTexture->basepath,"%s%c",tx->basepath,set.rel_path_separator);
        currentTexture->flags = tx->def_flags;
        currentTexture->contents = tx->def_contents;
        currentTexture->value = tx->def_value;
    }
    SetEditBoxes();
}

bool TTextureWindow::CheckWad(char *fname)
{
    int			i;
    FILE 		*wadfile;
    int			numlumps;
    uintptr_t offset;
    wadinfo_t   *wadinfo;
    lumpinfo_t *lumpinfo;
    sin_lumpinfo_t *slumpinfo;
    lumpinfo_t *curlump;
    sin_lumpinfo_t *scurlump;
    char outstr[128];
    int counter = 0;

    // char tempNm[LUMP_NAME_LENGTH_MAX];

    if (!file_exists(fname))
    {
        char tempName[MAX_PATH];

        // try just adding the path + extension...
        snprintf(tempName,sizeof(tempName),"%s\\%s",set.wad_directory,fname);
        if (file_exists(tempName))
        {
            //xxx
            strcpy(fname,tempName);
        }
        else
        {
            char filename[256] = "";

            OPENFILENAME ofn;
            memset(&ofn,0,sizeof(OPENFILENAME));
            ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
            ofn.hwndOwner = frame->hwnd;
            ofn.lpstrFilter = "Wad Files (*.wad)\0*.wad\0";
            ofn.lpstrDefExt = "wad";
            ofn.lpstrFile = filename;
            ofn.nMaxFile = sizeof(filename);
            ofn.lpstrInitialDir = set.wad_directory;
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

            char tempStr[256];
            sprintf(tempStr,"Where is texture .wad file: %s?",fname);
            ofn.lpstrTitle = tempStr;

            if (GetOpenFileName(&ofn))
            {
                strcpy(fname,filename);
            }
            else
            {
                MessageBox(hwnd,"You must select a valid .wad file...", "BSP Load Wad Error",
                           MB_OK | MB_ICONEXCLAMATION);
                return false;
            }
        }
    }

    if ((wadfile = fopen(fname,"rb")) == NULL)
    {
        //fclose(wadfile);
        MessageBox(hwnd,"Couldn't open wad file...", "BSP Load Wad Error", MB_OK | MB_ICONEXCLAMATION);
        return false;
    }

    wadinfo = new wadinfo_t;
    memset(wadinfo,0,sizeof(wadinfo_t));

    fread(wadinfo,sizeof(wadinfo_t),1,wadfile);

    if (set.sinBsp)
    {
        if (set.valveBsp)
        {
            if (strncmp (wadinfo->identification, "WAD3", 4))
            {
                sprintf(outstr,"%s isn't a VALVE wad file", fname);
                MessageBox(hwnd,outstr, "BSP Load Wad Error", MB_OK | MB_ICONEXCLAMATION);
                fclose(wadfile);
                delete wadinfo;
                return false;
            }
        }
        else
        {
            if (strncmp (wadinfo->identification, "SIN1", 4) &&
                    strncmp (wadinfo->identification, "1SIN", 4))
            {
                sprintf(outstr,"%s isn't a SIN wad file", fname);
                MessageBox(hwnd,outstr, "BSP Load Wad Error", MB_OK | MB_ICONEXCLAMATION);
                fclose(wadfile);
                delete wadinfo;
                return false;
            }
        }
    }
    else if (strncmp (wadinfo->identification, "WAD", 3))
    {
        sprintf(outstr,"%s isn't a wad file", fname);
        MessageBox(hwnd,outstr, "BSP Load Wad Error", MB_OK | MB_ICONEXCLAMATION);
        fclose(wadfile);
        delete wadinfo;
        return false;
    }

    numlumps = (int)wadinfo->numlumps;
    offset = wadinfo->infotableofs;

    fseek(wadfile,offset,SEEK_SET);

    if (set.sinBsp && !set.valveBsp)
    {
        slumpinfo = (sin_lumpinfo_t *)new sin_lumpinfo_t[numlumps]; // get the headers...
        memset(slumpinfo,0,numlumps*sizeof(sin_lumpinfo_t));
        fread(slumpinfo,sizeof(sin_lumpinfo_t),(int)numlumps,wadfile);
    }
    else
    {
        lumpinfo = (lumpinfo_t *)new lumpinfo_t[numlumps]; // get the headers...
        memset(lumpinfo,0,numlumps*sizeof(lumpinfo_t));
        fread(lumpinfo,sizeof(lumpinfo_t),(int)numlumps,wadfile);
    }


    for (i=0 ; i < numlumps ; i++)
    {
        if (set.sinBsp && !set.valveBsp)
        {
            scurlump = &slumpinfo[i];
        }
        else
        {
            curlump = &lumpinfo[i];
        }

        sin_lumpinfo_t *sinlump = NULL;
        lumpinfo_t *lmp = NULL;

        if (set.sinBsp  && !set.valveBsp)
            sinlump = scurlump;
        else
            lmp = curlump;

        if (set.sinBsp)
        {
            if (!set.valveBsp)
            {
                if (sinlump->type != LUMPTYPE_MIPTEXNEW)
                    continue;
            }
            else
            {
                if (lmp->type != LUMPTYPE_MIPTEX)
                    continue;
            }
        }
        else if (set.game_mode == 1)
        {
            if ((lmp->type != LUMPTYPE_MIPTEX) && (lmp->type != TYP_MIPTEX))
                continue;
        }
        else
        {
            if ((lmp->type != LUMPTYPE_MIPTEX) && (lmp->type != TYP_MIPTEX))
                continue;
        }
        counter++;
    }

    fclose(wadfile);
    delete wadinfo;

    // currentHead = currentHead->next; // for next loop;
    if (set.sinBsp && !set.valveBsp)
        delete[] slumpinfo;
    else
        delete[] lumpinfo;

    if (counter <= 0)
    {
        sprintf(outstr,"%s did not have any textures!", fname);
        MessageBox(hwnd, outstr, "BSP Load Wad Error", MB_OK | MB_ICONEXCLAMATION);
        return false;
    }
    return true;
}

void TTextureWindow::CreateFakeWad(char *fname)
{
    int			i, j;
    uintptr_t size;
    texInfo_t *newTex;
    texInfo_t *currentHead;
    BITMAPINFOHEADER m_pBmInfoHeader;
    BITMAPINFO *m_pBmInfo;
    char tempNm[LUMP_NAME_LENGTH_MAX];

    STRNCPY(currentWadPtr->currentWad,fname);

    currentWadPtr->bmpsCount = 1;
    currentWadPtr->textures = new texInfo_t; // the head...
    currentHead = currentWadPtr->textures;

    newTex = new texInfo_t;

    newTex->next = NULL;
    newTex->previous = currentHead;
    currentHead->next = newTex;

    newTex = new texInfo_t;

    strcpy(tempNm,"BAD_WAD");

    STRNCPY(newTex->bmp,tempNm);
    strupr(newTex->bmp);
    newTex->index = 0;

    newTex->h = 64;
    newTex->w = 64;

    size = newTex->h * newTex->w * 85/64;

    newTex->bits = (unsigned char*)new char[size+1];
    if (set.sinBsp)
    {
        newTex->truebits = (unsigned char*)new char[3*(size+1)];
        memset(newTex->truebits,255,3*(size+1));
    }

    uintptr_t ofs;
    bool onoff;
    unsigned char onIdx;
    unsigned char offIdx;
    onIdx  = (unsigned char)GetPaletteIndex(256,set.pal,RGB(255,0,0));
    offIdx = (unsigned char)GetPaletteIndex(256,set.pal,RGB(0,0,0));
    unsigned char *dest;
    unsigned char *tdest;

    int miph, mipw;
    for (int mip = 0; mip < 3; mip++)
    {
        dest = (unsigned char *)newTex->bits;
        tdest = (unsigned char *)newTex->truebits;

        switch (mip)
        {
        case 0:
            miph = 64;
            mipw = 64;
            break;
        case 1:
            miph = 32;
            mipw = 32;
            dest = dest + 64*64;
            tdest = tdest + 64*64*3;
            break;
        case 2:
            miph = 16;
            mipw = 16;
            dest = dest + 64*64 + 32*32;
            tdest = tdest + (64*64 + 32*32)*3;
            break;
        case 3:
            miph = 8;
            mipw = 8;
            dest = dest + 64*64 + 32*32 + 16*16;
            tdest = tdest + (64*64 + 32*32 + 16*16)*3;
            break;
        }

        for (i = 0; i < miph; i++)
        {
            for (j = 0; j < mipw; j++)
            {
                ofs = miph*(i) + j;
                onoff = (!((i/4) % 2)) ^ (!((j/4) % 2));

                if (onoff)
                {
                    dest[ofs] = onIdx;
                    if (set.sinBsp)
                    {
                        tdest[(ofs << 1) + ofs + 0] = 255;
                        tdest[(ofs << 1) + ofs + 1] = 0;
                        tdest[(ofs << 1) + ofs + 2] = 0;
                    }
                }
                else
                {
                    dest[ofs] = offIdx;
                    if (set.sinBsp)
                    {
                        tdest[(ofs << 1) + ofs + 0] = 0;
                        tdest[(ofs << 1) + ofs + 1] = 0;
                        tdest[(ofs << 1) + ofs + 2] = 0;
                    }
                }
            }
        }
    }

    if (cTextureAlpha)
    {
        newTex->GLBits = (unsigned char*)new char[4*(64*64+1)];
        memset(newTex->GLBits,255,4*(64*64+1));
        newTex->useW = 64;
        newTex->useH = 64;
    }
    else
    {
        newTex->GLBits = (unsigned char*)new char[3*(64*64+1)];
        memset(newTex->GLBits,255,3*(64*64+1));
        newTex->useW = 64;
        newTex->useH = 64;
    }

    //create a new bitmap for fake img
    memset(&m_pBmInfoHeader,0,sizeof(BITMAPINFOHEADER));
    m_pBmInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
    m_pBmInfoHeader.biWidth = 64;
    m_pBmInfoHeader.biHeight = -64;
    m_pBmInfoHeader.biPlanes = 1;
    m_pBmInfoHeader.biBitCount = 8;
    m_pBmInfoHeader.biCompression = BI_RGB;
    m_pBmInfoHeader.biClrUsed = 256;
    m_pBmInfoHeader.biSizeImage = 64*64;

    //copy palette into struct
    m_pBmInfo = (BITMAPINFO*)new char[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];
    memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
    memcpy(m_pBmInfo->bmiColors,set.cTable,256*sizeof(RGBQUAD));

    if (set.sinBsp)
    {
        newTex->palette = new unsigned char[PALSTEP*256+3];
        memcpy(newTex->palette,set.pal,PALSTEP*256);
    }

    newTex->dibInfo = m_pBmInfo;
    newTex->truedibInfo = NULL;
    if (set.sinBsp && set.valveBsp)
    {
        newTex->palSize = 256;
    }

    newTex->flat.chan[0] = 255;
    newTex->flat.chan[1] = 0;
    newTex->flat.chan[2] = 0;
    newTex->flat.p = GetRandomFlat();

    HDC hdc = GetDC(hwnd);
    SetStretchBltMode(hdc, COLORONCOLOR);

    StretchDIBits(hdc,
                  0,0,newTex->w,newTex->h,
                  0,0,newTex->w,newTex->h,
                  newTex->bits, newTex->dibInfo,
                  DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(hwnd,hdc);

    currentHead = currentWadPtr->textures->next; // first one

    int done = 0;
    while (currentHead && !done)
    {
        if (currentHead->next)
        {
            if (QCompare(currentHead->bmp, newTex->bmp) > 0)
            {
                newTex->next = currentHead;
                newTex->previous = currentHead->previous;
                newTex->previous->next = newTex;
                newTex->next->previous = newTex;
                done = 1;
            }
        }
        else
        {
            // insert before this one...
            newTex->next = currentHead;
            newTex->previous = currentHead->previous;
            newTex->previous->next = newTex;
            newTex->next->previous = newTex;
            done = 1;
        }
        currentHead = currentHead->next;
    }

    currentWadPtr->bmpsCount = 1;

    texList->ClearList();
    if (frame->TextureList)
    {
        frame->TextureList->ClearList();
        delete[] frame->data;
        frame->data = new ListBoxData[currentWadPtr->bmpsCount];
        memset(frame->data,0,currentWadPtr->bmpsCount*sizeof(ListBoxData));
    }

    for (i = 0; i< currentWadPtr->bmpsCount; i++)
    {
        newTex = GetTextureNo(currentWadPtr,i);
        texList->InsertString(newTex->bmp,i);
        if (frame->TextureList)
        {
            frame->data[i].tx = newTex;
            STRNCPY(frame->data[i].description,newTex->bmp);
            frame->TextureList->InsertString((LPSTR)&frame->data[i],i);
        }
    }

    texList->SetText(GetTextureNo(currentWadPtr,0)->bmp);
    if (frame->TextureList)
        frame->TextureList->SetSelIndex(0);

    UpdateScrollbar();

    prevIndex = 0;
    ResetDefaults();
    RedrawContents();
}

int GetTextureModeValue(char *md)
{
    if (!strcmpi(md, "GL_NEAREST"))
        return GL_NEAREST;
    else if (!strcmpi(md, "GL_NEAREST_MIPMAP_NEAREST"))
        return GL_NEAREST_MIPMAP_NEAREST;
    else if (!strcmpi(md, "GL_NEAREST_MIPMAP_LINEAR"))
        return GL_NEAREST_MIPMAP_LINEAR;
    else if (!strcmpi(md, "GL_LINEAR"))
        return GL_LINEAR;
    else if (!strcmpi(md, "GL_LINEAR_MIPMAP_NEAREST"))
        return GL_LINEAR_MIPMAP_NEAREST;
    else if (!strcmpi(md, "GL_LINEAR_MIPMAP_LINEAR"))
        return GL_LINEAR_MIPMAP_LINEAR;
    return 0;
}

void TTextureWindow::ClickglBspSettings()
{
    if (!set.Map_Read || !set.glBsp)
        return;

    TGLBuffer x;
    x.Mode.Clear();
    x.Mode.Add("GL_NEAREST");
    x.Mode.Add("GL_NEAREST_MIPMAP_NEAREST", true);
    x.Mode.Add("GL_NEAREST_MIPMAP_LINEAR");
    x.Mode.Add("GL_LINEAR");
    x.Mode.Add("GL_LINEAR_MIPMAP_NEAREST");
    x.Mode.Add("GL_LINEAR_MIPMAP_LINEAR");

    if (TGLDlg(frame->hwnd,&x).Execute() == IDOK)
    {
        set.gl_texturemode_value = GetTextureModeValue(x.selected);
        UnloadTextures();
        SetUpMap(map_i[set.curmap]);
        set.redrawedit = 1;
        Show_Frame("Texture mode changed",true);
    }
}



void TTextureWindow::AddWad(texWad *wad, char *wadName, char *loadName, int loadType)
{
    strncpy(wad->currentWad,wadName,sizeof(wad->currentWad)); // may get overwritten anyway...

    wad->WadType   = QUAKE_WAD;
    wad->bmpsCount = 0;
    wad->textures  = NULL;
    wad->nextWad 	= firstWad;
    firstWad 		= wad;
    currentWadPtr 	= wad;
    if (loadType == 2)
    {
        if (set.loadInUse)
        {
            LoadTexturesInUse(set.texture_path,map_i[set.curmap]); // load it in...
        }
        else
        {
            LoadTextureDirectory(set.texture_path,loadName); // load it in...
        }
    }
    else
    {
        LoadTextures(loadName,1); // load it in...
    }
}

texWad *TTextureWindow::QueryWadLoaded(char *wadName)
{
    char outstr[256];
    texWad *cur = firstWad;
    while (cur)
    {
        ExtractFileBase(cur->currentWad,outstr,false);

        if (!strcmpi(wadName,outstr))
            return cur;
        cur = cur->nextWad;
    }
    return (texWad *)NULL;
}

void TTextureWindow::CalcWH()
{
    trueW = set.texture_width + set.show_tex_names*TEXHSPACE;
    trueH = set.texture_height + set.show_tex_names*TEXVSPACE;
}

void TTextureWindow::CalcFF()
{
    nameoffset = noffset + 3;

    int leftEdge = 50;

    ClipFavorites.left   = leftEdge;
    ClipFavorites.top    = nameoffset;
    ClipFavorites.right  = leftEdge+(set.height_fav+(set.show_tex_names*TEXHSPACE))*set.tex_numfav;
    ClipFavorites.bottom = nameoffset+set.height_fav;

    nameoffset += set.show_favorites*(set.height_fav+(set.show_tex_names*TEXVSPACE));

    ClipFrequent.left    = leftEdge;
    ClipFrequent.top     = nameoffset;
    ClipFrequent.right   = leftEdge+(set.height_frq+(set.show_tex_names*TEXHSPACE))*set.tex_numfrq;
    ClipFrequent.bottom  = nameoffset+set.height_frq;

    nameoffset += set.show_frequent*(set.height_frq+(set.show_tex_names*TEXVSPACE));
}


void TTextureWindow::UpdateScrollbar(bool setpos, bool redraw)
{
    Scroller.Y.nMax = totrows;
    Scroller.Y.nPage = rowsperpage;
    if(totrows <= rowsperpage)
    {
        Scroller.Y.nMax = 0;
    }
    if(setpos)
    {
        Scroller.Y.nPos = curstartrow;
    }
    if(redraw)
        Scroller.Update();
}

void TTextureWindow::CalcMetrics()
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    int height = (rc.bottom-rc.top) - nameoffset;
    int width =  rc.right-rc.left;

    colsperrow = (int)(width / trueW);
    colsperrow = max(1,colsperrow);
    rowsperpage = (int)(height / trueH);
    rowsperpage = max(1,rowsperpage);


    curstartrow = 0;
    if (prevIndex > 0)
        curstartrow = prevIndex / colsperrow;

    if (set.Map_Read && map_i && map_i[set.curmap] && (map_i[set.curmap]->numMapTextures > 0))
        totrows = map_i[set.curmap]->numMapTextures/colsperrow;
    else
        totrows = currentWadPtr->bmpsCount/colsperrow;


    MainRect.left = 0;
    MainRect.right = width;
    MainRect.top = 0;
    MainRect.bottom = (totrows + rowsperpage + 1)*trueH;
    FFRect.left = 0;
    FFRect.right = width;
    FFRect.top = 0;
    FFRect.bottom = nameoffset - noffset;

    UpdateScrollbar();
}

void TTextureWindow::ChangeToMap(map *m)
{
    int i;

    if (!m)
        return;

    if (!m->mapTextures || (m->numMapTextures <= 0))
    {
        SetCaption("NO TEXTURES");
        DoNCPaint();
        texList->ClearList();
        if (frame->TextureList)
        {
            frame->TextureList->ClearList();
            delete[] frame->data;
            frame->data = new ListBoxData[1];
            memset(frame->data,0,sizeof(ListBoxData));
        }
        texList->SetText("");
        if (frame->TextureList)
            frame->TextureList->SetText("");
        *m->texName = 0;
        *m->texPath = 0;
        totrows = 0;
        prevIndex = 0;
        curstartrow = 0;
        return;
    }

    // recalculate stuff that depends upon currentWadPtr fields...
    totrows = m->numMapTextures/colsperrow;

    // deal with currentTexture
    ResetDefaults();

    // use texture 0
    prevIndex = 0;
    curstartrow = (int)(prevIndex/colsperrow);

    UpdateScrollbar();

    texInfo_t *tx = m->mapTextures[0];
    if (tx)
        NewCurrentTexture(tx);

    texList->ClearList();
    if (frame->TextureList)
    {
        frame->TextureList->ClearList();
        delete[] frame->data;
        frame->data = new ListBoxData[m->numMapTextures];
        memset(frame->data,0,m->numMapTextures*sizeof(ListBoxData));
    }

    for (i = 0; i< m->numMapTextures; i++)
    {
        tx = m->mapTextures[i];
        if (tx)
        {
            texList->InsertString(tx->bmp,i);
            if (frame->TextureList)
            {
                frame->data[i].tx = tx;
                STRNCPY(frame->data[i].description,tx->bmp);
                frame->TextureList->InsertString((LPSTR)&frame->data[i],i);
            }
        }
    }

    if (m->mapTextures && m->numMapTextures > 0)
    {
        texList->SetText(m->mapTextures[0]->bmp);
    }
    else
    {
        texList->SetText("");
        if (frame->TextureList)
            frame->TextureList->SetText("");
    }

    if (frame->TextureList)
        frame->TextureList->SetSelIndex(0);

    if (m->numMapTextures > 0)
    {
        STRNCPY(m->texName,m->mapTextures[0]->bmp);
        if (set.game_mode == 2)
            STRNCPY(m->texPath,m->mapTextures[0]->basepath);
    }
    else
    {
        *m->texName = 0;
        if (set.game_mode == 2)
            *m->texPath = 0;
    }

    char outstr[2048];
    char shortstr[2048];
    char *start = outstr;
    *outstr = 0;
    for (i = 0; i < m->numWads; i++)
    {
        if (!m->Wads || !m->Wads[i])
            continue;

        if (!strnicmp(m->Wads[i]->currentWad,"Use(",4))
        {
            STRNCPY(shortstr,m->Wads[i]->currentWad);
        }
        else
        {
            ExtractFileBase(m->Wads[i]->currentWad,shortstr,false);
        }
        if (!i)
        {
            strcpy(start,shortstr);
            start += strlen(shortstr);
        }
        else
        {
            *start = ';';
            start++;
            strcpy(start,shortstr);
            start += strlen(shortstr);
        }
    }

    if (m->numWads <= 0)
        strcpy(outstr,"NO TEXTURES");

    currentWadPtr = m->Wads[0];

    SetCaption(outstr);
    DoNCPaint();

    RemoveMemDC();
    MainRect.bottom = (totrows + rowsperpage)*trueH;
    ConstructMemDC();
    RedrawContents();
}

void TTextureWindow::GetEditBoxes()
{
    if (suppress_gets)
        return;

    float temp1;
    int tempi;
    char str[21];

    suppress_gets = true;

    surfaceWindow->sE->GetText(str,20);
    if (str[0] != '\0')
    {
        temp1 = (float) atof(str);
        currentTexture->shift[0] = temp1;
    }
    else
    {
        currentTexture->shift[0] = 0.0;
        surfaceWindow->sE->SetText("0");
    }
    surfaceWindow->tE->GetText(str,20);
    if (str[0] != '\0')
    {
        temp1 = (float) atof(str);
        currentTexture->shift[1] = temp1;
    }
    else
    {
        currentTexture->shift[1] = 0.0;
        surfaceWindow->sE->SetText("0");
    }
    surfaceWindow->rE->GetText(str,20);
    if (str[0] != '\0')
    {
        temp1 = (float) atof(str);
        currentTexture->rotate = temp1;
    }
    else
    {
        currentTexture->rotate = 0.0;
        surfaceWindow->sE->SetText("0");
    }
    surfaceWindow->sxE->GetText(str,20);
    if (str[0] != '\0')
    {
        temp1 = (float) atof(str);
        if (fabs(temp1) < BAD_SCALE) temp1 = 1.0; // can't be zero
        currentTexture->scale[0] = temp1;
    }
    surfaceWindow->syE->GetText(str,20);
    if (str[0] != '\0')
    {
        temp1 = (float) atof(str);
        if (fabs(temp1) < BAD_SCALE) temp1 = 1.0; // can't be zero
        currentTexture->scale[1] = temp1;
    }

    if (set.game_mode == 2)
    {
        surfaceWindow->valEdit->GetText(str,20);
        if (str[0] != '\0')
        {
            tempi = atoi(str);
            currentTexture->value = tempi;
        }
    }
    suppress_gets = false;
}

void TTextureWindow::SetEditBoxes()
{
    surfaceWindow->sE->SetInt((int)currentTexture->shift[0]);
    surfaceWindow->tE->SetInt((int)currentTexture->shift[1]);
    surfaceWindow->rE->SetInt((int)currentTexture->rotate);
    surfaceWindow->sxE->SetFloat(currentTexture->scale[0]);
    surfaceWindow->syE->SetFloat(currentTexture->scale[1]);

    if (set.game_mode == 2)
    {
        surfaceWindow->valEdit->SetInt(currentTexture->value);
        surfaceWindow->SetFlags(currentTexture->flags);
        surfaceWindow->SetContents(currentTexture->contents);
    }
    surfaceWindow->SetCaption("Surface");
    surfaceWindow->DoNCPaint();
}

void TTextureWindow::SetCaption(const char* title)
{
    char outstr[1024];
    if (set.game_mode == 2)
    {
        sprintf(outstr,"[Texture/QII- %s]",title);
    }
    else
    {
        sprintf(outstr,"[Texture - %s]",title);
    }
    Window::SetCaption(outstr);
}



void TTextureWindow::StartAnimation()
{
    SetTimer(hwnd, 2,set.tex_timerdelay,0); // send WM_TIMER messages...
    set.tex_animate = 1;
}

void TTextureWindow::StopAnimation()
{
    KillTimer(hwnd, 2);
    set.tex_animate = 0;
}

void TTextureWindow::ResetButton()
{
    ResetDefaults();
    SetEditBoxes();
}

void TTextureWindow::ResetDefaults()
{
    currentTexture->rotate = 0.0;
    currentTexture->shift[0] = 0.0;
    currentTexture->shift[1] = 0.0;
    currentTexture->scale[0] = 1.0;
    currentTexture->scale[1] = 1.0;

    currentTexture->value = 0;
    currentTexture->flags = 0;
    currentTexture->contents = 0;
    sprintf(currentTexture->basepath,"UNKNOWN%c",set.rel_path_separator);

    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    if (!m)
        return;

    texInfo_t *tx = GetTextureByName(m,currentTexture->texture);

    // if "none" and there are some textures loaded, default to 0th texture
    if (!tx && !strcmpi(currentTexture->texture,"none"))
    {
        if (!m->mapTextures || m->numMapTextures <= 0)
            return;

        tx = m->mapTextures[0];
        if (!tx)
            return;

        STRNCPY(currentTexture->texture,tx->bmp);
        prevIndex = 0;
        Select(0,true);
    }

    if (!tx)
    {
        currentTexture->value    = 0;
        currentTexture->contents = 0;
        currentTexture->flags    = 0;

        sprintf(currentTexture->basepath,"UNKNOWN%c",set.rel_path_separator);
    }
    else
    {
        currentTexture->value    = tx->def_value;
        currentTexture->contents = tx->def_contents;
        currentTexture->flags    = tx->def_flags;

        sprintf(currentTexture->basepath,"%s%c",tx->basepath,set.rel_path_separator);
    }
}

int TTextureWindow::GetRandomFlat()
{
    int r = (rand() & 0x7F)+127;
    int g = (rand() & 0x7F)+127;
    int b = (rand() & 0x7F)+127;

    return GetPaletteIndex(256,set.pal,RGB(r,g,b));
}

unsigned char TTextureWindow::BestColor(unsigned char *pal, int r, int g, int b, int start, int stop)
{
    int i;
    int diff, bestdiff;
    unsigned char bestindex;
    int dr, dg, db;
    int stepper;

    stepper = PALSTEP;

    bestdiff = 2*(r*r + g*g + b*b);
    bestindex = 0;

    for (i = start ; i <= stop; i++)
    {
        dr = pal[stepper*i+0] - r;
        dg = pal[stepper*i+1] - g;
        db = pal[stepper*i+2] - b;

        diff = ((dr*dr)+(dg*dg)+(db*db));

        if (diff <= bestdiff)
        {
            if (!diff)
                return (unsigned char)i;

            bestdiff = diff;
            bestindex = (unsigned char)i;
        }
    }
    return bestindex;
}

// get best index...
pixel32_t TTextureWindow::TranslateColors(int pSize,unsigned char *pal,unsigned char *t_bits,int size)
{
    pixel32_t retval;
    unsigned char c;
    int rcount, gcount, bcount;
    rcount = gcount = bcount = 0;
    int stop = min(1024,size);

    if (set.sinBsp)
    {
        for (int j = 0; j < stop; j++)
        {
            c = *t_bits;
            rcount += pal[PALSTEP*c+0];
            gcount += pal[PALSTEP*c+1];
            bcount += pal[PALSTEP*c+2];
            t_bits++;
        }
    }
    else
    {
        for (int j = 0; j < stop; j++)
        {
            c = *t_bits;
            rcount +=  pal[PALSTEP*c+0];
            gcount +=  pal[PALSTEP*c+1];
            bcount +=  pal[PALSTEP*c+2];
            t_bits++;
        }
    }

    if(set.flats_like_texture)
    {
        retval.chan[0] = (unsigned char)((rcount/stop));
        retval.chan[1] = (unsigned char)((gcount/stop));
        retval.chan[2] = (unsigned char)((bcount/stop));
    }
    else
    {
        retval.chan[0] = (unsigned char)(127+(256*rcount/stop)/2);
        retval.chan[1] = (unsigned char)(127+(256*gcount/stop)/2);
        retval.chan[2] = (unsigned char)(127+(256*bcount/stop)/2);
    }
    retval.p = GetPaletteIndex(pSize, pal,retval.rgb);

    return retval;
}

void TTextureWindow::FreeTextures(texWad *wad)
{
    texInfo_t *next;
    texInfo_t *head = wad->textures->next;

    while (head)
    {
        next = head->next;
        if(set.glBsp)
        {
            glDeleteTextures(1, (GLuint*) &head->bindIndex);
        }
        delete head;
        head = next;
    }
}

texInfo_t *TTextureWindow::GetTextureNo(texWad *wad,int index)
{
    // this is number 0
    texInfo_t *cur = wad->textures->next;
    for (int i = 0; (i < index) && cur; i++)
        cur = cur->next;

    return cur; // NULL if too high...
}

int TTextureWindow::GetTextureIndex(map *m,texInfo_t *t)
{
    int i;
    if (!m || !m->mapTextures)
        return -1;

    for (i = 0; i < m->numMapTextures; i++)
    {
        if (m->mapTextures[i] == t)
            return i;
    }
    return -1;
}

void TTextureWindow::ClearCounts(map *m)
{
    if (!m || !m->mapTextures)
        return;

    for (int i = 0; i < m->numMapTextures; i++)
        m->mapTextures[i]->count = 0;
}

void TTextureWindow::UpdateColorValues()
{
    set.outlineIndex  = GetPaletteIndex(256, set.pal, set.color_brushoutline);
    set.selectedIndex = GetPaletteIndex(256, set.pal, set.color_selectoutline);
    set.faceIndex     = GetPaletteIndex(256, set.pal, set.color_faceoutline);
    set.lockIndex     = GetPaletteIndex(256, set.pal, set.color_lock);
}

void TTextureWindow::CalcGLBits(texInfo_t *newTex, unsigned char *thePal)
{
    uintptr_t i, j;
    uintptr_t size;
    unsigned char *gP;
    unsigned char *sP;
    unsigned char *in;
    unsigned char *out;

    uintptr_t stepSize = 3;
    if (cTextureAlpha)
        stepSize = 4;

    bool glAdjust = false;

    newTex->useW = newTex->w;
    newTex->useH = newTex->h;

    if(!set.gl_full_res_texture)
    {
        if (!isPowerof2(newTex->w) || (newTex->w > 128))
        {
            newTex->useW = nextPowerof2(newTex->w);
            if (newTex->useW > 128)
                newTex->useW = 128;
            glAdjust = true;
            newTex->uMult = (float) newTex->useW/(float)newTex->w;
        }
        if (!isPowerof2(newTex->h)|| (newTex->h > 128))
        {
            newTex->useH = nextPowerof2(newTex->h);
            if (newTex->useH > 128)
                newTex->useH = 128;
            glAdjust = true;
            newTex->vMult = (float) newTex->useH/(float)newTex->h;
        }
    }

    // get a big enough chunk
    size = (uintptr_t) newTex->useW * (uintptr_t) newTex->useH;
    newTex->GLBits = (unsigned char*)new char[stepSize*size+3];
    gP = (unsigned char *)newTex->GLBits;
    sP = (unsigned char *)newTex->bits;

    if (!glAdjust)
    {
        if (stepSize == 3L)
        {
            for (i = 0; i < size; i++)
            {
                *gP++ = thePal[PALSTEP*(*sP)+0];
                *gP++ = thePal[PALSTEP*(*sP)+1];
                *gP++ = thePal[PALSTEP*(*sP)+2];
                sP++;
            }
        }
        else
        {
            for (i = 0; i < size; i++)
            {
                *gP++ = thePal[PALSTEP*(*sP)+0];
                *gP++ = thePal[PALSTEP*(*sP)+1];
                *gP++ = thePal[PALSTEP*(*sP)+2];
                *gP++ = 255;
                sP++;
            }
        }
    }
    else
    {
        // interpolate...  SLOW
        int row, col;
        uintptr_t rowOfs;
        uintptr_t ofs;
        for (i = 0; i <  (uintptr_t)newTex->useH; i++)
        {
            row = (int)((float)i*(float)newTex->h/(float) newTex->useH);
            rowOfs = i*(uintptr_t)newTex->useW;
            if (cTextureAlpha)
            {
                for (j = 0; j <  (uintptr_t)newTex->useW; j++)
                {
                    col = (int)((float)j*(float)newTex->w/(float) newTex->useW);

                    ofs = (uintptr_t)row*(uintptr_t)newTex->w;

                    in = &sP[ofs + (uintptr_t)col];

                    out = &gP[stepSize*(rowOfs + j)];

                    *out++ = thePal[PALSTEP*(*in)+0];
                    *out++ = thePal[PALSTEP*(*in)+1];
                    *out++ = thePal[PALSTEP*(*in)+2];
                    *out   = 255;
                }
            }
            else
            {
                for (j = 0; j <  (uintptr_t)newTex->useW; j++)
                {
                    col = (int)((float)j*(float)newTex->w/(float) newTex->useW);

                    ofs = (uintptr_t)row*(uintptr_t)newTex->w;

                    in = &sP[ofs + (uintptr_t)col];

                    out = &gP[stepSize*(rowOfs + j)];

                    *out++ = thePal[PALSTEP*(*in)+0];
                    *out++ = thePal[PALSTEP*(*in)+1];
                    *out   = thePal[PALSTEP*(*in)+2];
                }
            }
        }
    }
}


void TTextureWindow::LoadTextures(char *fname, int showstat)
{
    int				i;
    FILE 			*wadfile;
    int				numlumps;
    uintptr_t	offset;
    wadinfo_t		*wadinfo;
    lumpinfo_t		*lumpinfo;
    sin_lumpinfo_t	*slumpinfo;
    lumpinfo_t		*curlump;
    sin_lumpinfo_t	*scurlump;

    unsigned char	*curtex;
    int				w, h;
    uintptr_t	size;
    uintptr_t	chunksize;
    unsigned char   *t_bits;
    texInfo_t		*newTex;
    texInfo_t		*currentHead;
    BITMAPINFOHEADER m_pBmInfoHeader;

    if (!CheckWad(fname))
    {
        MessageBox(hwnd,"Error:  Creating a Fake Texture Wad", "BSP Wad Loader", MB_OK);
        CreateFakeWad(fname);
        return;
    }

    int HasPalette = 0;
    STRNCPY(currentWadPtr->currentWad,fname);
    char fakePath[256];
    ExtractFileBase(fname,fakePath,true);
    strupr(fakePath);

    if (!file_exists(currentWadPtr->currentWad))
    {
        char tempName[MAX_PATH];

        sprintf(tempName,"%s\\%s",set.wad_directory,fname); // try just adding the path + extension...
        if (file_exists(tempName))
        {
            STRNCPY(currentWadPtr->currentWad,tempName);
        }
        else
        {

            char filename[256] = "";

            OPENFILENAME ofn;
            memset(&ofn,0,sizeof(OPENFILENAME));
            ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
            ofn.hwndOwner = frame->hwnd;
            ofn.lpstrFilter = "Wad Files (*.wad)\0*.wad\0";
            ofn.lpstrDefExt = "wad";
            ofn.lpstrFile = filename;
            ofn.nMaxFile = 256;
            ofn.lpstrInitialDir = set.wad_directory;
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

            char tempStr[256];
            sprintf(tempStr,"Where is texture .wad file: %s?",fname);
            ofn.lpstrTitle = tempStr;

            if (!GetOpenFileName(&ofn))
            {
                syserror(const_cast<char *> ("You must select a valid .wad file..."));
                Msgbox(const_cast<char *> ("You must select a valid .wad file..."));
                return;
            }

            STRNCPY(currentWadPtr->currentWad,filename);
        }
    }

    //  free any texture names
    if ((wadfile = fopen(currentWadPtr->currentWad,"rb")) == NULL)
    {
        syserror(const_cast<char *> ("Texture:  Couldn't open wad file..."));
        Msgbox(const_cast<char *> ("Texture:  Couldn't open wad file..."));
        return;
    }

    wadinfo = new wadinfo_t;
    memset(wadinfo,0,sizeof(wadinfo_t));
    fread(wadinfo,sizeof(wadinfo_t),1,wadfile);

    if (set.sinBsp)
    {
        if (set.valveBsp)
        {
            if (strncmp (wadinfo->identification, "WAD3", 4))
            {
                syserror(const_cast<char *> ("Error: %s isn't a SIN wadfile"), currentWadPtr->currentWad);
                delete wadinfo;
                return;
            }
        }
        else
        {
            if (strncmp (wadinfo->identification, "SIN1", 4) &&
                    strncmp (wadinfo->identification, "1SIN", 4))
            {
                syserror(const_cast<char *> ("Error: %s isn't a SIN wadfile"), currentWadPtr->currentWad);
                delete wadinfo;
                return;
            }
        }
    }
    else if (strncmp (wadinfo->identification, "WAD", 3))
    {
        syserror(const_cast<char *> ("Error: %s isn't a WAD type wadfile"), currentWadPtr->currentWad);
        delete wadinfo;
        return;
    }

    numlumps = (int)wadinfo->numlumps;
    offset = wadinfo->infotableofs;
    fseek(wadfile,offset,SEEK_SET);

    if (set.sinBsp && !set.valveBsp)
    {
        slumpinfo = (sin_lumpinfo_t *)new sin_lumpinfo_t[numlumps]; // get the headers...
        memset(slumpinfo,0,numlumps*sizeof(sin_lumpinfo_t));
        fread(slumpinfo,sizeof(sin_lumpinfo_t),(int)numlumps,wadfile);
    }
    else
    {
        lumpinfo = (lumpinfo_t *)new lumpinfo_t[numlumps]; // get the headers...
        memset(lumpinfo,0,numlumps*sizeof(lumpinfo_t));
        fread(lumpinfo,sizeof(lumpinfo_t),(int)numlumps,wadfile);
    }

    if (currentWadPtr->bmpsCount)
    {
        FreeTextures(currentWadPtr);
    }

    currentWadPtr->bmpsCount = (int)(numlumps - HasPalette);


    char statstr[80] = "";
    if (showstat)
    {
        status->SetText("",true);
    }

    int counter = 0;

    currentWadPtr->textures = new texInfo_t; // the head...

    currentHead = currentWadPtr->textures;

    newTex = new texInfo_t;

    newTex->next = NULL;
    newTex->previous = currentHead;
    currentHead->next = newTex;

    char tempNm[LUMP_NAME_LENGTH_MAX];
    for (i=0 ; i<numlumps ; i++)
    {
        if (showstat)
        {
            sprintf(statstr,"Loading %s [%i/%i] ",currentWadPtr->currentWad,i,numlumps);
            status->SetText(statstr,true);
        }

        if (set.sinBsp && !set.valveBsp)
        {
            scurlump = &slumpinfo[i];
        }
        else
        {
            curlump = &lumpinfo[i];
        }

        sin_lumpinfo_t *sinlump = NULL;
        lumpinfo_t *lmp = NULL;

        if (set.sinBsp  && !set.valveBsp)
        {
            sinlump = scurlump;
        }
        else
        {
            lmp = curlump;
        }

        if (set.sinBsp)
        {
            if (!set.valveBsp)
            {
                if (sinlump->type != LUMPTYPE_MIPTEXNEW)
                    continue;
            }
            else
            {
                if (lmp->type != LUMPTYPE_MIPTEX)
                    continue;
            }
        }
        else if (set.game_mode == 1)
        {
            if ((lmp->type != LUMPTYPE_MIPTEX) && (lmp->type != TYP_MIPTEX))
                continue;
        }
        else
        {
            if ((lmp->type != LUMPTYPE_MIPTEX) && (lmp->type != TYP_MIPTEX))
                continue;
        }
        newTex = new texInfo_t;

        if (set.sinBsp && !set.valveBsp)
            strncpy(tempNm,sinlump->name,LUMP_NAME_LENGTH);
        else
            strncpy(tempNm,lmp->name,LUMP_NAME_LENGTH);

        //HACK - conchars in gfx.wad breaks because it is a different tex format but has "D" type
        if(!strcmpi(tempNm,"CONCHARS"))
        {
            delete newTex;
            continue;
        }


        tempNm[LUMP_NAME_LENGTH-1] = '\0';
        STRNCPY(newTex->bmp,tempNm);
        strupr(newTex->bmp);
        newTex->index = i;
        newTex->texType = TEX_TYPE_Q1;
        STRNCPY(newTex->basepath,fakePath);

        int ds;
        if (set.sinBsp && !set.valveBsp)
        {
            offset = sinlump->filepos;
            ds = sinlump->disksize;
        }
        else
        {
            offset = lmp->filepos;
            ds = lmp->disksize;
        }

        if ((ds < 0L) || (ds > 256000L)) 			// bad...
        {
            newTex->h = 32;
            newTex->w = 32;

            size = newTex->h * newTex->w * 85/64;

            newTex->bits = (unsigned char*)new char[size+1];
            memset(newTex->bits,255,size+1);

            if (cTextureAlpha)
            {
                newTex->GLBits = (unsigned char*)new char[4*(32*32+1)];
                memset(newTex->GLBits,255,4*(32*32+1));
            }
            else
            {
                newTex->GLBits = (unsigned char*)new char[3*(32*32+1)];
                memset(newTex->GLBits,255,3*(32*32+1));
            }
            newTex->useW = 32;
            newTex->useH = 32;

            if (set.sinBsp)
            {
                newTex->truebits = (unsigned char*)new char[3*(size+1)];
                memset(newTex->truebits,255,3*(size+1));
            }
            BITMAPINFO *m_pBmInfo;

            memset(&m_pBmInfoHeader,0,sizeof(BITMAPINFOHEADER));
            m_pBmInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
            m_pBmInfoHeader.biWidth = 32;
            m_pBmInfoHeader.biHeight = -32;
            m_pBmInfoHeader.biPlanes = 1;
            m_pBmInfoHeader.biBitCount = 8;
            m_pBmInfoHeader.biCompression = BI_RGB;
            m_pBmInfoHeader.biClrUsed = 256;
            m_pBmInfoHeader.biSizeImage = 32*32;

            m_pBmInfo = (BITMAPINFO*)new char[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];
            memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
            memcpy(m_pBmInfo->bmiColors,set.cTable,256*sizeof(RGBQUAD));

            if (set.sinBsp)
            {
                newTex->palette = new unsigned char[PALSTEP*256+3];
                memset(newTex->palette,128,PALSTEP*256);
            }

            newTex->dibInfo = m_pBmInfo;
            newTex->truedibInfo = NULL;
            if (set.valveBsp)
                newTex->palSize = 256;

            newTex->flat.rgb = RGB(255,255,255);
            newTex->flat.p = GetRandomFlat();

        }
        else
        {
            fseek(wadfile,offset,SEEK_SET);

            //alloc curtex
            if (set.sinBsp)
            {
                if (set.valveBsp)
                {
                    int vs = sizeof(valve_miptexnew_t);
                    int rs = vs-sizeof(WORD)-sizeof(byte *);

                    curtex = reinterpret_cast<unsigned char*>((valve_miptexnew_t *) new valve_miptexnew_t);
                    memset(curtex,0,vs);
                    fread(curtex,rs,1,wadfile);

                    ((valve_miptexnew_t *)curtex)->palSize = 0;
                    ((valve_miptexnew_t *)curtex)->palette = NULL;
                }
                else
                {
                    curtex = reinterpret_cast<unsigned char*>((sin_miptexnew_t *)new sin_miptexnew_t);
                    memset(curtex,0,sizeof(sin_miptexnew_t));
                    fread(curtex,sizeof(sin_miptexnew_t),1,wadfile);
                    newTex->palette = new unsigned char[PALSTEP*256+3];
                    memcpy(newTex->palette,((sin_miptexnew_t *)curtex)->palette,PALSTEP*256);
                }
            }
            else
            {
                curtex = reinterpret_cast<unsigned char*>((miptex_t *) new miptex_t);
                memset(curtex,0,sizeof(miptex_t));
                fread(curtex,sizeof(miptex_t),1,wadfile);
            }

            //get dimensions
            if (set.sinBsp)
            {
                if (set.valveBsp)
                {
                    w = (int)((valve_miptexnew_t *)curtex)->width;
                    h = (int)((valve_miptexnew_t *)curtex)->height;
                }
                else
                {
                    w = (int)((sin_miptexnew_t *)curtex)->width;
                    h = (int)((sin_miptexnew_t *)curtex)->height;
                }
            }
            else
            {
                w = (int)((miptex_t *)curtex)->width;
                h = (int)((miptex_t *)curtex)->height;
            }

            //make sure dimensions are valid
            if(w<=0 || h<=0 || w>4096 || h>4096)
            {
                if (set.sinBsp && !set.valveBsp)
                {
                    delete newTex->palette;
                }
                delete curtex;
                delete newTex;
                continue;
            }
            size = w * h * 85/64;
            newTex->h = h;
            newTex->w = w;
            newTex->bits = (unsigned char*)new char[size+3];

            int saveoffset = offset;

            if (set.sinBsp)
            {
                if (set.valveBsp)
                {
                    offset += ((valve_miptexnew_t *)curtex)->offsets[0];
                }
                else
                {
                    offset += ((sin_miptexnew_t *)curtex)->offsets[0];
                }
            }
            else
            {
                offset += ((miptex_t *)curtex)->offsets[0];
            }
            fseek(wadfile,offset,SEEK_SET);

            t_bits = (unsigned char*)newTex->bits;

            chunksize = 0x8000;
            while (size > chunksize)
            {
                if (fread(t_bits,(int)chunksize,1,wadfile) != 1)
                {
                    syserror(const_cast<char *> ("Error:  couldn't read texture bits..."));  // read in the bits...
                    break;
                }
                size -= chunksize;
                t_bits += chunksize;
            }
            if (size)
            {
                if (fread(t_bits,(int)size,1,wadfile) != 1)
                    syserror(const_cast<char *> ("Error:  couldn't read texture bits..."));  // read in the bits...
            }

            // reset...
            size = (uintptr_t)w * (uintptr_t)h;

            // do in place now...
            t_bits = (unsigned char*)newTex->bits;

            if (set.valveBsp)
            {
                offset = saveoffset;
                if (set.sinBsp)
                {
                    if (set.valveBsp)
                    {
                        offset = offset + ((valve_miptexnew_t *)curtex)->offsets[0];
                    }
                    else
                    {
                        offset = offset + ((sin_miptexnew_t *)curtex)->offsets[0];
                    }
                }
                else
                {
                    offset = offset + ((miptex_t *)curtex)->offsets[0];
                }
                uintptr_t w1;

                w1 = size + size/4 + size/16 + size/64;

                offset += w1;

                fseek(wadfile,offset,SEEK_SET);

                fread(&((valve_miptexnew_t *)curtex)->palSize,sizeof(WORD),1,wadfile);

                // ignore?
                //	if (((valve_miptexnew_t *)curtex)->palSize > 1024) {
                //		sysfatal("Error: set.valveBsp couldn't read palette size...");  // read in the bits...
                //	}

                ((valve_miptexnew_t *)curtex)->palette =
                    new BYTE[PALSTEP*((valve_miptexnew_t *)curtex)->palSize];
                newTex->palette =
                    new unsigned char[PALSTEP*((valve_miptexnew_t *)curtex)->palSize+3];
                memcpy(newTex->palette,
                       ((valve_miptexnew_t *)curtex)->palette,
                       PALSTEP*((valve_miptexnew_t *)curtex)->palSize);
                newTex->palSize = ((valve_miptexnew_t *)curtex)->palSize;
            }

            if (set.sinBsp)
            {
                if (set.valveBsp)
                {
                    newTex->flat = TranslateColors(((valve_miptexnew_t *)curtex)->palSize,
                                                   ((valve_miptexnew_t *)curtex)->palette,t_bits,size);
                }
                else
                {
                    newTex->flat = TranslateColors(256,((sin_miptexnew_t *)curtex)->palette,t_bits,size);
                }
            }
            else
            {
                newTex->flat = TranslateColors(256,set.pal,t_bits,size);
            }

            if (newTex->isSkyQ1())
            {
                // cut down the heights...
                newTex->w /= 2;
                w /= 2;

                newTex->sky = new sky_t;
                newTex->sky->skyRow = 0;
                newTex->sky->skyCol = 0;

                newTex->sky->bits = newTex->bits;
                size = (uintptr_t)w * (uintptr_t)h;
                size = size * 85/64;
                newTex->bits = (unsigned char*)new char[size+3];
                CreateSkyBits(newTex->w,newTex->h,newTex->sky,newTex->bits);
            }
            else if (newTex->isWarp())
            {
                newTex->sky = new sky_t;
                newTex->sky->skyRow = 0;
                newTex->sky->skyCol = 0;

                newTex->sky->bits = newTex->bits;
                size = (uintptr_t)w * (uintptr_t)h;
                size = size * 85/64;
                newTex->bits = (unsigned char*)new char[size+3];
                CreateWaterBits(newTex->w,newTex->h,newTex->sky,newTex->bits);
            }

            unsigned char *thePal;
            if (set.sinBsp)
            {
                if (set.valveBsp)
                {
                    thePal = (unsigned char *)((valve_miptexnew_t *)curtex)->palette;
                }
                else
                {
                    thePal = (unsigned char *)((sin_miptexnew_t *)curtex)->palette;
                }
            }
            else
            {
                thePal = (unsigned char *)set.pal;
            }

            if (set.glBsp)
                CalcGLBits(newTex,thePal);

            if (set.sinBsp)
            {
                uintptr_t qq;

                size = (uintptr_t)w * (uintptr_t)h;

                size = size * 85/64;
                if (newTex->truebits)
                {
                    //delete [] newTex->truebits;
                    free (newTex->truebits);
                    newTex->truebits = NULL;
                }
                newTex->truebits = (unsigned char*)new char[3*size+3];

                unsigned char *tP;
                unsigned char *sP;

                tP = (unsigned char *)newTex->truebits;
                sP = (unsigned char *)newTex->bits;

                if (set.valveBsp)
                {
                    for (qq = 0; qq < size; qq++)
                    {
                        *tP++ = ((valve_miptexnew_t *)curtex)->palette[PALSTEP*(*sP)+2];
                        *tP++ = ((valve_miptexnew_t *)curtex)->palette[PALSTEP*(*sP)+1];
                        *tP++ = ((valve_miptexnew_t *)curtex)->palette[PALSTEP*(*sP)+0];
                        sP++;
                    }
                }
                else
                {
                    for (qq = 0; qq < size; qq++)
                    {
                        *tP++ = ((sin_miptexnew_t *)curtex)->palette[PALSTEP*(*sP)+2];
                        *tP++ = ((sin_miptexnew_t *)curtex)->palette[PALSTEP*(*sP)+1];
                        *tP++ = ((sin_miptexnew_t *)curtex)->palette[PALSTEP*(*sP)+0];
                        sP++;
                    }
                }
            }
            //
            BITMAPINFO *m_pBmInfo;

            memset(&m_pBmInfoHeader,0,sizeof(BITMAPINFOHEADER));
            m_pBmInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
            m_pBmInfoHeader.biWidth = w;
            m_pBmInfoHeader.biHeight = -h;
            m_pBmInfoHeader.biPlanes = 1;
            m_pBmInfoHeader.biBitCount = 8;
            m_pBmInfoHeader.biCompression = BI_RGB;
            m_pBmInfoHeader.biSizeImage = w * h;

            if (set.valveBsp)
            {
                m_pBmInfoHeader.biClrUsed = ((valve_miptexnew_t *)curtex)->palSize;
                m_pBmInfo = (BITMAPINFO*)new char[sizeof(BITMAPINFOHEADER)+((valve_miptexnew_t *)curtex)->palSize*sizeof(RGBQUAD)];
            }
            else
            {
                m_pBmInfoHeader.biClrUsed = 256;
                m_pBmInfo = (BITMAPINFO*)new char[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];
            }

            //err("Couldn't allocate bitmap info header in texture loader...");

            memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
            if (set.sinBsp)
            {
                if (set.valveBsp)
                {
                    for (int hhh = 0; hhh < ((valve_miptexnew_t *)curtex)->palSize; hhh++)
                    {
                        m_pBmInfo->bmiColors[hhh].rgbRed   = ((valve_miptexnew_t *)curtex)->palette[PALSTEP*hhh+0];
                        m_pBmInfo->bmiColors[hhh].rgbGreen = ((valve_miptexnew_t *)curtex)->palette[PALSTEP*hhh+1];
                        m_pBmInfo->bmiColors[hhh].rgbBlue  = ((valve_miptexnew_t *)curtex)->palette[PALSTEP*hhh+2];
                        m_pBmInfo->bmiColors[hhh].rgbReserved = 0;
                    }
                }
                else
                {
                    for (int hhh = 0; hhh < 256; hhh++)
                    {
                        m_pBmInfo->bmiColors[hhh].rgbRed   = ((sin_miptexnew_t *)curtex)->palette[PALSTEP*hhh+0];
                        m_pBmInfo->bmiColors[hhh].rgbGreen = ((sin_miptexnew_t *)curtex)->palette[PALSTEP*hhh+1];
                        m_pBmInfo->bmiColors[hhh].rgbBlue  = ((sin_miptexnew_t *)curtex)->palette[PALSTEP*hhh+2];
                        m_pBmInfo->bmiColors[hhh].rgbReserved = 0;
                    }
                }
            }
            else
            {
                memcpy(m_pBmInfo->bmiColors,set.cTable,256*sizeof(RGBQUAD));
            }

            newTex->dibInfo = m_pBmInfo;
            newTex->truedibInfo = NULL; // tm_pBmInfo;

            if (set.valveBsp)
            {
                delete[] ((valve_miptexnew_t *)curtex)->palette;
            }
            delete curtex;
        }

        HDC hdc = GetDC(hwnd);
        SetStretchBltMode(hdc, COLORONCOLOR);

        StretchDIBits(hdc,
                      0,0,newTex->w,newTex->h,
                      0,0,newTex->w,newTex->h,
                      newTex->bits,
                      newTex->dibInfo,
                      DIB_RGB_COLORS,
                      SRCCOPY);
        ReleaseDC(hwnd,hdc);

        currentHead = currentWadPtr->textures->next; // first one
        int done = 0;

        while (currentHead && !done)
        {
            if (currentHead->next)
            {
                if (QCompare(currentHead->bmp, newTex->bmp) > 0)
                {
                    newTex->next = currentHead;
                    newTex->previous = currentHead->previous;
                    newTex->previous->next = newTex;
                    newTex->next->previous = newTex;
                    done = 1;
                }
            }
            else
            {
                // insert before this one...
                newTex->next = currentHead;
                newTex->previous = currentHead->previous;
                newTex->previous->next = newTex;
                newTex->next->previous = newTex;
                done = 1;
            }
            currentHead = currentHead->next;
        }

        // currentHead = currentWadPtr->textures;

        counter++;
    }
    currentWadPtr->bmpsCount = counter;

    fclose(wadfile);

    // currentHead = currentHead->next; // for next loop;

    if (showstat)
    {
        status->SetText("Done...",true);
    }
    else
    {
        char ot[80];
        sprintf(ot,"Loading %s [%i/%i] done...",currentWadPtr->currentWad,numlumps,numlumps);
        splash->SetText(ot);
    }

    delete wadinfo;
    if (set.sinBsp && !set.valveBsp)
    {
        delete[] slumpinfo;
    }
    else
    {
        delete[] lumpinfo;
    }


    texList->ClearList();
    if (frame->TextureList)
    {
        frame->TextureList->ClearList();
        delete[] frame->data;
        frame->data = new ListBoxData[currentWadPtr->bmpsCount];
        memset(frame->data,0,currentWadPtr->bmpsCount*sizeof(ListBoxData));
    }

    for (i = 0; i< currentWadPtr->bmpsCount; i++)
    {
        newTex = GetTextureNo(currentWadPtr,i);
        texList->InsertString(newTex->bmp,i);
        if (frame->TextureList)
        {
            frame->data[i].tx = newTex;
            STRNCPY(frame->data[i].description,newTex->bmp);
            frame->TextureList->InsertString((LPSTR)&frame->data[i],i);
        }
    }

    newTex = GetTextureNo(currentWadPtr,0);
    if (newTex)
    {
        texList->SetText(newTex->bmp);
    }
    else
    {
        texList->SetText("");
        if (frame->TextureList)
            frame->TextureList->SetText("");
    }

    if (frame->TextureList)
        frame->TextureList->SetSelIndex(0);

    UpdateScrollbar();

    prevIndex = 0;
    ResetDefaults();
    RedrawContents();
}

void TTextureWindow::LoadTextureDirectory(char *basepath, char *dirname)
{
    int			i;
    uintptr_t size;
    texInfo_t *newTex;
    texInfo_t *currentHead;
    BITMAPINFOHEADER m_pBmInfoHeader;
    char fname[256];
    unsigned char *lump;
    miptex2_t *qtex;
    char	filename[1024];
    unsigned char *source;
    unsigned	char *dest;
    int width, height, count;

    sprintf(fname,"%s/%s/",basepath,dirname);

    // Quake II .wal file loader...
    currentWadPtr->WadType = QUAKEII_WAD;

    /*FIXME for QII
    if (!CheckWad(fname)) {
    MessageBox("Error:  Creating a Fake Texture Wad", "BSP Wad Loader", MB_OK);
    CreateFakeWad(fname);
    return;
    };
    */

    STRNCPY(currentWadPtr->currentWad,fname);

    // This would be weird...
    if (currentWadPtr->bmpsCount)
        FreeTextures(currentWadPtr);

    char statstr[80] = "";
    status->SetText("",true);

    int counter = 0;

    currentWadPtr->textures = new texInfo_t; // the head...

    currentHead = currentWadPtr->textures;

    newTex = new texInfo_t;

    newTex->next = NULL;
    newTex->previous = currentHead;
    currentHead->next = newTex;

    char tempNm[LUMP_NAME_LENGTH_MAX];

    // Cycle through the .wal files!
    char walName[128];
    struct _finddata_t ffb;
    memset(&ffb,0,sizeof(ffb));

    char path[256];
    sprintf(path,"%s*.wal",fname);
    int ff = _findfirst(path,&ffb);
    int nomorefiles = ff < 0 ? 1 : 0;
    while (!nomorefiles)
    {
        sprintf(statstr,"Loading %s [%i] ",currentWadPtr->currentWad,counter+1);
        status->SetText(statstr,true);

        newTex = new texInfo_t;

        strncpy(walName,ffb.name,sizeof(walName));

        // Load it in...
        StripExtension(walName);

        // load the file
        sprintf (filename, "%s\\%s\\%s.wal", set.texture_path,dirname, walName);

        lump = file_get_contents(filename);
        if (!lump)
        {
            syserror(const_cast<char *> ("load .wal \"%s\" failed!\n"), filename);
            delete newTex;
            nomorefiles = _findnext(ff, &ffb);
            continue;
        }

        qtex = (miptex2_t *)lump;

        width = qtex->width;
        height = qtex->height;

        newTex->def_flags    = qtex->flags;
        newTex->def_value    = qtex->value;
        newTex->def_contents = qtex->contents;

        newTex->texType = TEX_TYPE_Q2;
        STRNCPY(newTex->basepath,dirname);

        strncpy (qtex->name, walName, sizeof(qtex->name)-1);
        StripExtension (qtex->name);

        strncpy(tempNm,qtex->name,sizeof(tempNm));

        tempNm[LUMP_NAME_LENGTH-1] = '\0';
        strncpy(newTex->bmp,tempNm,sizeof(newTex->bmp));
        strupr(newTex->bmp);

        newTex->index = counter;

        size = (uintptr_t)width * (uintptr_t)height;
        size = size * 85/64;

        newTex->h = height;
        newTex->w = width;

        dest = new unsigned char[size+width+1];

        count = size;
        source = (unsigned char *)qtex;
        source += qtex->offsets[0];

        // copy the bits...
        for (i=0 ; i<count ; i++) dest[i] = source[i];

        delete[] lump;

        newTex->bits = dest;
        newTex->flat = TranslateColors(256,set.pal,dest,size);

        if (newTex->isSkyQ1())
        {
            // cut down the heights...
            newTex->w /= 2;
            width /= 2;

            newTex->sky = new sky_t;
            newTex->sky->skyRow = 0;
            newTex->sky->skyCol = 0;

            newTex->sky->bits = newTex->bits;
            size = (uintptr_t)width * (uintptr_t)height;
            size = size * 85/64;
            newTex->bits = (unsigned char*)new char[size+3];
            CreateSkyBits(newTex->w,newTex->h,newTex->sky,newTex->bits);
        }
        else if (newTex->isWarp())
        {
            newTex->sky = new sky_t;
            newTex->sky->skyRow = 0;
            newTex->sky->skyCol = 0;

            newTex->sky->bits = newTex->bits;
            size = (uintptr_t)width * (uintptr_t)height;
            size = size * 85/64;
            newTex->bits = (unsigned char*)new char[size+3];
            CreateWaterBits(newTex->w,newTex->h,newTex->sky,newTex->bits);
        }

        unsigned char *thePal = (unsigned char *)set.pal;

        if (set.glBsp)
            CalcGLBits(newTex,thePal);

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
        m_pBmInfo = (BITMAPINFO*)new char[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];

        memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
        memcpy(m_pBmInfo->bmiColors,set.cTable,256*sizeof(RGBQUAD));

        newTex->dibInfo = m_pBmInfo;
        newTex->truedibInfo = NULL; // tm_pBmInfo;

        HDC hdc = GetDC(hwnd);
        SetStretchBltMode(hdc, COLORONCOLOR);

        StretchDIBits(hdc,
                      0,0,newTex->w,newTex->h,
                      0,0,newTex->w,newTex->h,
                      newTex->bits,
                      newTex->dibInfo,
                      DIB_RGB_COLORS,
                      SRCCOPY);
        ReleaseDC(hwnd,hdc);

        currentHead = currentWadPtr->textures->next; // first one
        int done = 0;

        while (currentHead && !done)
        {
            if (currentHead->next)
            {
                if (QCompare(currentHead->bmp,newTex->bmp) > 0)
                {
                    newTex->next = currentHead;
                    newTex->previous = currentHead->previous;
                    newTex->previous->next = newTex;
                    newTex->next->previous = newTex;
                    done = 1;
                }
            }
            else
            {
                // insert before this one...
                newTex->next = currentHead;
                newTex->previous = currentHead->previous;
                newTex->previous->next = newTex;
                newTex->next->previous = newTex;
                done = 1;
            }
            currentHead = currentHead->next;
        }
        counter++;
        nomorefiles = _findnext(ff, &ffb);
    }
    _findclose(ff);

    currentWadPtr->bmpsCount = counter;

    status->SetText("Done...",true);

    // Is this needed???xxx FIXME
    texList->ClearList();
    if (frame->TextureList)
    {
        frame->TextureList->ClearList();
        delete[] frame->data;
        frame->data = new ListBoxData[currentWadPtr->bmpsCount];
        memset(frame->data,0,currentWadPtr->bmpsCount*sizeof(ListBoxData));
    }

    for (i = 0; i< currentWadPtr->bmpsCount; i++)
    {
        newTex = GetTextureNo(currentWadPtr,i);
        texList->InsertString(newTex->bmp,i);
        if (frame->TextureList)
        {
            frame->data[i].tx = newTex;
            STRNCPY(frame->data[i].description,newTex->bmp);
            frame->TextureList->InsertString((LPSTR)&frame->data[i],i);
        }
    }

    newTex = GetTextureNo(currentWadPtr,0);
    if (newTex)
    {
        texList->SetText(newTex->bmp);
    }
    else
    {
        texList->SetText("");
        if (frame->TextureList)
            frame->TextureList->SetText("");
    }

    if (frame->TextureList)
        frame->TextureList->SetSelIndex(0);

    UpdateScrollbar();

    prevIndex = 0;
    ResetDefaults();
    RedrawContents();
}

void TTextureWindow::SetUpMap(map *m)
{
    int i;
    int numTex;
    texInfo_t *newTex;

    if (!m)
        return;

    int oldAnimate = set.tex_animate;
    set.tex_animate = 0;

    if (m->favorites)  		//TODO
    {
        delete[] m->favorites;
        m->favorites = NULL;
    }
    m->favorites = new texInfo_t *[MAX_FAVORITES];	//todo make auto
    memset(m->favorites,0,set.tex_numfav*sizeof(texInfo_t *));

    if (m->bests)
    {
        delete[] m->bests;
        m->bests = NULL;
    }
    m->bests = new texInfo_t *[MAX_FREQUENT];	//todo
    memset(m->bests,0,set.tex_numfrq*sizeof(texInfo_t *));

    if (m->mapTextures)
    {
        for (i = 0; i < m->numMapTextures; i++)
        {
            if (m->mapTextures[i]->aTextures)
            {
                delete[] m->mapTextures[i]->aTextures;
                m->mapTextures[i]->aTextures = NULL;
            }
            m->mapTextures[i]->aCount = 0;
            m->mapTextures[i]->aCurrent = 0;
        }
        delete[] m->mapTextures;
        m->mapTextures = NULL;
    }

    for (numTex = 0, i = 0; i < m->numWads; i++)
    {
        numTex += m->Wads[i]->bmpsCount;
    }

    m->mapTextures = new texInfo_t *[numTex];
    memset(m->mapTextures,0,numTex*sizeof(texInfo_t *));

    m->numMapTextures = numTex;

    // do a merge sort...
    int done = 0;
    texInfo_t **currents;

    currents = new texInfo_t *[m->numWads];
    memset(currents,0,m->numWads*sizeof(texInfo_t *));

    for (i = 0; i < m->numWads; i++)
    {
        currents[i] = m->Wads[i]->textures->next; // first one...
        currents[i]->selected = false;
    }

    int anyChanged;
    int bestWad;
    char bestTex[LUMP_NAME_LENGTH_MAX];
    char test[LUMP_NAME_LENGTH_MAX];
    int index = 0;
    while (!done && (index < numTex))
    {
        bestWad = 0;
        anyChanged = 0;
        *bestTex = 0;

        for (i = 0; i < m->numWads; i++)
        {
            if (!currents[i]->next)
                continue;

            strcpy(test,currents[i]->bmp);

            if (!*bestTex)
            {
                strcpy(bestTex,test);
                bestWad = i;
                anyChanged = 1;
            }
            else
            {
                if (QCompare(bestTex,test) > 0)
                {
                    strcpy(bestTex,test);
                    bestWad = i;
                    anyChanged = 1;
                }
            }
        }
        if (!anyChanged)
        {
            done = 1;
        }
        else
        {
            // skip if duplicate!
            if (!(index && !QCompare(m->mapTextures[index-1]->bmp,currents[bestWad]->bmp)))
            {
                m->mapTextures[index++] = currents[bestWad];
            }
            currents[bestWad] = currents[bestWad]->next;
        }
    }
    m->numMapTextures = index;

    // now go thru and remove duplicates from animating frame, leave
    // the first in the list and tag rest aint with it
    // copy first over also...
    //////////////////////////////////////
    //////////////// Animating textures...
    //////////////////////////////////////
    texInfo_t *t;
    char base[40];
    char *bStart;
    int bc;
    for (i = 0; i < m->numMapTextures; i++)
    {
        t = m->mapTextures[i];

        strncpy(base,t->bmp,sizeof(base));
        bStart = base;

        if (t->isWarp() || (*bStart == '*'))     // set.sinBsp a second one?
        {
            t->animated = 2;
            continue;
        }

        if ((*bStart != '+') && (*bStart != '@'))
            continue;

        bStart++;
        bc = 0;
        while (isdigit((unsigned char)*bStart) && (bc++ <= 3))
            bStart++;

        if (bc > 3)
            continue;

        char realBase[40];
        texInfo_t *working;
        int testIdx = i+1;
        strncpy(realBase,bStart,sizeof(realBase));
        realBase[sizeof(realBase)-1] = 0;

        int repeatCount = 0;
        done = 0;
        while (!done)
        {
            if (testIdx >= m->numMapTextures)
                break;

            working = m->mapTextures[testIdx];

            char temp[40];
            strncpy(temp,working->bmp,40);
            bStart = temp;
            if ((*bStart != '+') && (*bStart != '@'))
                break;

            bStart++;
            bc = 0;
            while (isdigit((unsigned char)*bStart) && (bc++ <= 3))
                bStart++;

            if (bc > 3)
                break;

            if (strcmpi(realBase,bStart)) // not equal...
                break;

            repeatCount++;
            testIdx++;
        }

        if (!repeatCount)
            continue;

        t->aCount = repeatCount+1;
        t->aTextures = new texInfo_t *[t->aCount];
        memset(t->aTextures,0,t->aCount * sizeof(texInfo_t *));
        t->animated = 1;
        t->aCurrent = 0;

        t->aTextures[0] = t; // point to this one...
        for (int c = 1; c < t->aCount; c++)
            t->aTextures[c] = m->mapTextures[c+i];

        for (int c = i+1; c < (m->numMapTextures - (t->aCount-1)); c++)
            m->mapTextures[c] = m->mapTextures[c+(t->aCount-1)];

        for (int c = (m->numMapTextures - (t->aCount - 1)); c < m->numMapTextures; c++)
            m->mapTextures[c] = NULL;

        m->numMapTextures -= (t->aCount-1);
    }


    //////////////////////////////////////
    delete[] currents;
    // do merging here for multi wad maps...
    for (i = 0; i< numTex; i++)
    {
        if (i >= m->numMapTextures)
            continue;

        newTex = m->mapTextures[i];
        if (i < set.tex_numfav)
            m->favorites[i] = newTex;
        if (i < set.tex_numfrq)
            m->bests[i] = NULL;
    }

    if (m->numf && m->favs)
    {
        texInfo_t *tx;
        char lookfor[64];
        int idx = 0;
        for (i = 0; i < m->numf; i++)
        {
            if (i >= set.tex_numfav)
                continue;

            if (!m->favs[i])
                continue;

            if (set.game_mode == 2)
            {
                char ignore[64];
                SplitBaseQuiet(m->favs[i],ignore,lookfor);
            }
            else
            {
                strncpy(lookfor,m->favs[i],sizeof(lookfor));
            }

            tx = texWindow->GetTextureByName(m,lookfor);
            if (!tx)
            {
                tx = texWindow->TryLoadTextureWal(m,lookfor);
            }
            else
            {
                m->favorites[idx++] = tx;
            }
        }

        if (idx < set.tex_numfav)
        {
            for (i = idx; i < set.tex_numfav; i++)
            {
                m->favorites[i] = NULL;
            }
        }
    }

    if (set.glBsp)
    {
        if (!wglMakeCurrent(editWindow->ghDC, editWindow->ghRC))
            syserror(const_cast<char *> ("set.glBsp:  Couldn't make EditWindow's OpenGL context current"));

        if(set.gl_skybox)
            glReloadSkybox();

        char outstr[128];
        for (i = 0; i < m->numMapTextures; i++)
        {
            sprintf(outstr,"glBind: [%s] [%i/%i]...", newTex->bmp, i, m->numMapTextures);
            status->SetText(outstr,true);

            newTex = m->mapTextures[i];
            if(newTex->bindIndex > 0)	//check if texture is already bound
                continue;

            //HR TEST
            //C:/Quake/ezquake/textures
            //c:/bsp/settings/gfx/highres
            //c:/paintball2/pball/textures/pball/hr4

            bool found_high_res = false;

            char path[MAX_PATH];
            char *src = set.gl_highres_path, *dst;

            if(set.gl_highres && set.gl_highres_path && *set.gl_highres_path)
            {
                bool keep_looping = true;
                while(keep_looping)
                {

                    //split
                    while(*src && *src==';') *src++;
                    if(!*src) break;
                    int count = 0;
                    dst = path;
                    while(*src && *src!=';' && count++<MAX_PATH) *dst++ = *src++;
                    if(count >= MAX_PATH) break;
                    *dst = 0;

                    Path filename(const_cast<char *> ("%s/%s.*"), path, newTex->bmp);
                    WIN32_FIND_DATA ffd;
                    HANDLE ff = FindFirstFile(filename, &ffd);
                    if(ff != INVALID_HANDLE_VALUE)
                    {
                        while(1)
                        {
                            Path texpath(const_cast<char *> ("%s/%s"), path, ffd.cFileName);
                            unsigned char *pic=0;
                            int cx=0,cy=0;
                            if(LoadImageFile(texpath, &pic, &cx,&cy, true))
                            {
                                int tmpid = 0;
                                glGenTextures(1, (GLuint*) &tmpid);
                                glAddTexture(tmpid, /*newTex->useW=*/cx, /*newTex->useH=*/cy, (GLuint*) pic, GL_RGBA);
                                newTex->bindIndex = tmpid;
                                delete[]pic;
                                found_high_res = true;
                                keep_looping = false;
                                break;
                            }
                            if(!FindNextFile(ff, &ffd))
                            {
                                keep_looping = false;
                                break;
                            }
                        }
                        FindClose(ff);
                    }
                }
            }
            //load the .wal texture into ogl
            if(!found_high_res)
            {
                glGenTextures(1, (GLuint*) &newTex->bindIndex);
                glAddTexture(newTex->bindIndex, newTex->useW, newTex->useH, (GLuint*) newTex->GLBits);
            }
            //	delete[] newTex->GLBits;
            //	newTex->GLBits = 0;
        }

        // Reset...
        glBindTexture( GL_TEXTURE_2D, 0 );
    }

    if (set.autocalc_frequents)
    {
        CalcFrequents();
    }

    char capt[1024] = "";
    char shortName[256];

    if (m->numWads > 0)
    {
        for (i = 0; i < m->numWads - 1; i++)
        {
            if (!strnicmp(m->Wads[i]->currentWad,"Use(",4))
            {
                STRNCPY(shortName,m->Wads[i]->currentWad);
            }
            else
            {
                ExtractFileBase(m->Wads[i]->currentWad,shortName,false);
            }
            strcat(capt,shortName);
            strcat(capt,";");
        }
        if (!strnicmp(m->Wads[m->numWads - 1]->currentWad,"Use(",4))
        {
            STRNCPY(shortName,m->Wads[m->numWads - 1]->currentWad);
        }
        else
        {
            ExtractFileBase(m->Wads[m->numWads - 1]->currentWad,shortName,false);
        }
        strcat(capt,shortName);

        if (m->numWads <= 0)
            strcpy(capt,"NO TEXTURES");
        SetCaption(capt);
        DoNCPaint();
    }
    ResetDefaults();
    Invalidate();
    set.tex_animate = oldAnimate;
}

qtexture_t *TTextureWindow::GetQTexture(char *name)
{

    map *m;
    if (set.glBsp && glBspRotate)
        m = glMap;
    else
        m = map_i[set.curmap];

    qtexture_t *retval;
    if (!m || !m->mapTextures || (m->numMapTextures <= 0))
    {
        retval = new qtexture_t;
        memcpy(retval,badQ,sizeof(qtexture_t)); // copy the badQ into retval...
        return retval;
    }
    texInfo_t *tx = GetTextureByName(m, name);

    if(tx)
    {
        retval = new qtexture_t;
        memset(retval,0,sizeof(retval));
        strncpy(retval->name,tx->bmp,sizeof(retval->name));
        retval->width = tx->w;
        retval->height = tx->h;
        if (set.sinBsp)
        {
            retval->data = reinterpret_cast<unsigned char *> (tx->truebits);
        }
        else
        {
            retval->data = reinterpret_cast<unsigned char *> (tx->bits);
        }
        retval->flatcolor = tx->flat;
        retval->bindIndex = tx->bindIndex;
    }
    else
    {
        retval = new qtexture_t;
        memcpy(retval,badQ,sizeof(qtexture_t)); // copy the badQ into retval...
    }

    return retval;
}

//find texture using a binary search
texInfo_t *TTextureWindow::GetTextureByName(map *m, char *name)
{
    texInfo_t *retval = NULL;

    if (!name || !name[0])
        return retval;
    if (!m || !m->mapTextures || m->numMapTextures <= 0)
        return retval;

    char lookfor[LUMP_NAME_LENGTH_MAX];
    STRNCPY(lookfor,name);
    strupr(lookfor);

    int left = 0;                           // first
    int right = m->numMapTextures;   //last+1 // never gets tested...
    int current = (left+right)/2;

    int index = -1;

    while (true)
    {
        int testVal = QCompare(lookfor,m->mapTextures[current]->bmp);
        if (!testVal)
        {
            index = current;
            break;
        }
        else if (testVal > 0)
        {
            left = current;
            current = (left+right)/2;
        }
        else if (testVal < 0)
        {
            right = current;
            current = (left+right)/2;
        }
        if ((right - left) <= 1)
        {
            if(!QCompare(lookfor,m->mapTextures[left]->bmp))
            {
                index = left;
            }
            break;
        }
    }

    if ((index >= 0) && (index < m->numMapTextures))
        retval = m->mapTextures[index];

    return retval;
}

void TTextureWindow::getTextureDef(texturedef_t *td)
{
    GetEditBoxes();
    *td = *currentTexture;
}

void TTextureWindow::setTextureDef(texturedef_t *td)
{
    if (!strcmpi(td->texture,"ENTITY"))
        return;

    if (!map_i || !map_i[set.curmap])   // just set it
    {
        *currentTexture = *td;
        SetEditBoxes();
        return;
    }
    map *m = map_i[set.curmap];

    *currentTexture = *td;
    SetEditBoxes();
    texInfo_t *tx;
    for (int i = 0; i < m->numMapTextures; i++)
    {
        tx = m->mapTextures[i];

        if (!strcmp(td->texture,tx->bmp))
        {
            STRNCPY(m->texName,tx->bmp);
            if (set.game_mode == 2)
                STRNCPY(m->texPath,tx->basepath);
            texList->SetText(tx->bmp);
            if (frame->TextureList)
                frame->TextureList->SetSelIndex(i);

            Select(i,true); // select, then set...
            // reset the contents, flags stuff
            *currentTexture = *td;
            SetEditBoxes();
            break;
        }
    }
}

void TTextureWindow::EvCBNSelChange()
{
    const int len = 15;

    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    texList->GetText(m->texName,len);
    prevIndex = texList->GetSelIndex();

    if (set.game_mode == 2)
        strcpy(m->texPath,"UNKNOWN");

    if ((prevIndex < 0) || (prevIndex >= m->numMapTextures))
        return;

    texInfo_t *tx = m->mapTextures[prevIndex];
    if (tx)
    {
        NewCurrentTexture(tx);
        if (set.game_mode == 2)
            STRNCPY(m->texPath,tx->basepath);
        if (frame->TextureList)
            frame->TextureList->SetSelIndex(prevIndex);
    }
    curstartrow = (int)(prevIndex/colsperrow);

    UpdateScrollbar();

    Select(prevIndex,true);
}

void TTextureWindow::EvMouseMove(UINT modKeys, LPPOINT point)
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];
    if(!m)
        return;

    GdiFlush();

    texInfo_t *tx;
    char outstr[40];
    int idx;
    int selcol, selrow;

    // always display where mouse is over, in case text is "off"
    // if dragging, don't do it, actually...
    if (!lmousedown)
    {

        if (point->y >= nameoffset)
        {
            selcol = (int)(point->x / trueW);
            selrow = (int)((point->y-nameoffset) / trueH);

            idx = -1;
            if ((selrow <= rowsperpage) && (selcol < colsperrow))
            {
                selrow += curstartrow;
                idx = selrow * colsperrow + selcol;
            }

            if (idx != -1 && (idx < m->numMapTextures))
            {
                tx = m->mapTextures[idx];
                if (tx)
                {
                    if (set.game_mode == 2)
                    {
                        sprintf(outstr,"%s%c%s %ix%i",
                                tx->basepath,set.rel_path_separator,tx->bmp,tx->w,tx->h); //,tx->def_value, tx->def_contents, tx->def_flags);
                    }
                    else
                    {
                        sprintf(outstr,"%s %ix%i", tx->bmp,tx->w,tx->h);
                    }
                    strncpy(texStatic, outstr, sizeof(texStatic));
                }
            }
        }
        else if (set.show_favorites && PtInRect(&ClipFavorites,*point) && m->favorites)
        {
            idx = (point->x - ClipFavorites.left)/(set.height_fav+(set.show_tex_names*TEXHSPACE));

            if ((idx >=0) && (idx < set.tex_numfav))
            {
                tx = m->favorites[idx];
                if (tx)
                {
                    if (set.game_mode == 2)
                    {
                        sprintf(outstr,"%s%c%s %ix%i",
                                tx->basepath,set.rel_path_separator,tx->bmp,tx->w,tx->h); //,tx->def_value, tx->def_contents, tx->def_flags);
                    }
                    else
                    {
                        sprintf(outstr,"%s %ix%i", tx->bmp,tx->w,tx->h);
                    }
                    strncpy(texStatic, outstr, sizeof(texStatic));
                }
            }
        }
        else if (set.show_frequent && PtInRect(&ClipFrequent,*point) && m->bests)
        {
            idx = (point->x - ClipFrequent.left)/(set.height_frq+(set.show_tex_names*TEXHSPACE));

            if ((idx >=0) && (idx < set.tex_numfrq))
            {
                tx = m->bests[idx];
                if (tx)
                {
                    if (set.game_mode == 2)
                    {
                        if (tx->count)
                        {
                            sprintf(outstr,"%s%c%s %ix%i %i",
                                    tx->basepath,set.rel_path_separator,tx->bmp,tx->w,tx->h,tx->count); //,tx->def_value, tx->def_contents, tx->def_flags);
                        }
                        else
                        {
                            sprintf(outstr,"%s%c%s %ix%i",
                                    tx->basepath,set.rel_path_separator,tx->bmp,tx->w,tx->h); //,tx->def_value, tx->def_contents, tx->def_flags);
                        }
                    }
                    else
                    {
                        if (tx->count)
                        {
                            sprintf(outstr,"%s %ix%i %i", tx->bmp,tx->w,tx->h,tx->count);
                        }
                        else
                        {
                            sprintf(outstr,"%s %ix%i", tx->bmp,tx->w,tx->h);
                        }
                    }
                    strncpy(texStatic, outstr, sizeof(texStatic));
                }
            }
        }

        HDC hdc = GetDC(hwnd);
        RedrawClient(hdc);
        ReleaseDC(hwnd,hdc);
        return;
    }
    else
        // calc current texture, if not = to start, do the dragging. ..
        // show a drag outline...
        if (lmousedown == 1)
        {
            /*		TBrush *tbr = new TBrush(TColor(RGB(255,255,0)));
            		TClientDC *m_hDC = new TClientDC(*this);
            		m_hDC->SetROP2(R2_XORPEN);
            		if (!firstpt)
            			m_hDC->FrameRect(oldpt.x-5,oldpt.y-5,oldpt.x+5,oldpt.y+5,*tbr);
            		firstpt = 0;
            		oldpt = point;
            		m_hDC->FrameRect(oldpt.x-5,oldpt.y-5,oldpt.x+5,oldpt.y+5,*tbr);
            		delete m_hDC;
            		delete tbr;
            */
        }
        else if (lmousedown == 2)
        {
            ZoomTexture(point);
        }

}

void TTextureWindow::ZoomTexture(POINT *point)
{

    RECT src, dst;
    float mag;

    map *m = map_i[set.curmap];

    newpt.x = point->x;
    newpt.y = point->x;

    texInfo_t *tx;
    mag = (float) (newpt.x - startpt.x);
    mag = mag / 20.f;
    mag = max(1.0f, mag);
    mag = min(mag, 12.0f);

    if (mag == oldmag)
        return;

    if ((clickIndex < 0) || (clickIndex >= m->numMapTextures))
        return;

    tx = m->mapTextures[clickIndex];
    if (!tx)
        return;

    if (!tx->dibInfo)
    {
        MessageBox(hwnd,"Empty DibInfo?", "BSP", MB_OK);
        return;
    }

    sprintf(texStatic,"%s %ix%i %3.1fx",tx->bmp, tx->w,tx->h,mag);

    SetRect(&src,0,0,tx->w,tx->h);
    SetRect(&dst, 0, noffset, (int)(mag*src.right), noffset + src.top + (int)(mag*src.bottom));

    HDC hdc = GetDC(hwnd);

    RECT r, rc;
    GetClientRect(hwnd,&r);

    HBRUSH tbr = CreateSolidBrush(set.color_texturewindow);
    SetRect(&rc, r.left,noffset,r.right,r.bottom);
    FillRect(hdc, &rc, tbr);
    DeleteObject(tbr);


    RedrawClient(hdc);

    HPEN pen = CreatePen(PS_SOLID,1,GetSysColor(COLOR_BTNTEXT));
    HGDIOBJ oldpen = SelectObject(hdc, pen);
    MoveToEx(hdc, r.left,noffset, 0);
    LineTo(hdc, r.right,noffset);
    SelectObject(hdc, oldpen);
    DeleteObject(pen);

    if (tx->dibInfo && tx->bits)
    {
        SetStretchBltMode(hdc, COLORONCOLOR);

        StretchDIBits(hdc,
                      dst.left,dst.top,dst.right-dst.left,dst.bottom-dst.top,
                      src.left,src.top,src.right-src.left,src.bottom-src.top,
                      tx->bits, tx->dibInfo,
                      DIB_RGB_COLORS, SRCCOPY);
    }
    ReleaseDC(hwnd, hdc);

    oldmag = mag;
}

bool TTextureWindow::WasTextureClicked(LPPOINT point, int *idx, map *m)
{
    if (!m)
        return false;

    int selcol = (int)(point->x / trueW);
    int selrow = (int)((point->y - nameoffset) / trueH);

    // consider granularity...
    if ((selrow <= rowsperpage) && (selcol < colsperrow))
    {
        selrow += curstartrow;
        *idx = selrow * colsperrow + selcol;
        if ((*idx >= 0) && (*idx < m->numMapTextures))
            return true;
    }
    return false;
}

void TTextureWindow::EvLButtonUp(UINT modKeys, LPPOINT point)
{
    if (DoLButtonUp())
        return;
    if (!lmousedown)
        return;
    if (!map_i || !map_i[set.curmap])
        return;
    map *m = map_i[set.curmap];
    if(!m)
        return;

    int idx;

    texInfo_t *tx;

    // where did user drop it?
    // what was being done...
    switch (clickArea)
    {
        // dragged to favorites?  otherwise ignore...
    case AREA_FAV:
        if (set.show_favorites && PtInRect(&ClipFavorites,*point) && m->favorites)
        {
            idx = (point->x - ClipFavorites.left)/(set.height_fav+(set.show_tex_names*TEXHSPACE));

            if ((idx >=0) && (idx < set.tex_numfav))
            {
                tx = m->favorites[idx];
                if (tx)
                {
                    clickIndex = GetTextureIndex(m,tx);

                    if ((clickIndex != -1) && (clickIndex < m->numMapTextures))
                    {
                        STRNCPY(m->texName,tx->bmp);
                        if (set.game_mode == 2)
                            STRNCPY(m->texPath,tx->basepath);
                        texList->SetText(m->texName);
                        NewCurrentTexture(tx);
                        if (frame->TextureList)
                            frame->TextureList->SetSelIndex(clickIndex);

                        prevIndex = clickIndex;
                        curstartrow = (int)(prevIndex/colsperrow);

                        UpdateScrollbar();

                        RedrawFrequent();
                        Select(prevIndex,true);
                    }
                }
            }
        }
        break;
    case AREA_FRQ:
        if (set.show_frequent && PtInRect(&ClipFrequent,*point) && m->bests)
        {
            idx = (point->x - ClipFrequent.left)/(set.height_frq+(set.show_tex_names*TEXHSPACE));

            if ((idx >=0) && (idx < set.tex_numfrq))
            {
                tx = m->bests[idx];
                if (tx)
                {
                    clickIndex = GetTextureIndex(m,tx);

                    if ((clickIndex != -1) && (clickIndex < m->numMapTextures))
                    {
                        STRNCPY(m->texName,tx->bmp);
                        if (set.game_mode == 2)
                            STRNCPY(m->texPath,tx->basepath);
                        texList->SetText(m->texName);
                        NewCurrentTexture(tx);
                        if (frame->TextureList)
                            frame->TextureList->SetSelIndex(clickIndex);

                        prevIndex = clickIndex;
                        curstartrow = (int)(prevIndex/colsperrow);

                        UpdateScrollbar();

                        RedrawFrequent();
                        Select(prevIndex,true);
                    }
                }
            }
        }
        else if (set.show_favorites && PtInRect(&ClipFavorites,*point) && m->favorites)
        {
            idx = (point->x - ClipFavorites.left)/(set.height_fav+(set.show_tex_names*TEXHSPACE));
            tx = m->bests[clickIndex];

            if (tx)
                clickIndex = GetTextureIndex(m,tx);

            if (tx && idx >=0 && idx < set.tex_numfav && clickIndex < m->numMapTextures && clickIndex != -1)
            {
                m->favorites[idx] = tx;
                tx = m->favorites[idx];
                if (tx)
                {
                    clickIndex = GetTextureIndex(m,tx);

                    if ((clickIndex != -1) && (clickIndex < m->numMapTextures))
                    {
                        STRNCPY(m->texName,tx->bmp);
                        if (set.game_mode == 2)
                            STRNCPY(m->texPath,tx->basepath);
                        texList->SetText(m->texName);
                        NewCurrentTexture(tx);
                        if (frame->TextureList)
                            frame->TextureList->SetSelIndex(clickIndex);

                        prevIndex = clickIndex;
                        curstartrow = (int)(prevIndex/colsperrow);

                        UpdateScrollbar();

                        RedrawFavorite();
                        Select(prevIndex,true);
                    }
                }
            }
        }
        break;
    }
//	} else { // shift drag...
//	}

    Invalidate();
    lmousedown = 0;
    ReleaseCapture();
}

void TTextureWindow::EvLButtonDown(UINT modKeys, LPPOINT point)
{
    if (lmousedown || !set.Map_Read)
        return;
    map *m = map_i[set.curmap];
    if(!m)
        return;

    int idx;
    texInfo_t *tx;
    startpt.x = point->x;
    startpt.y = point->y;
    delta.x = delta.y = 0;
    relative.x = relative.y = 0;
    clickIndex = -1;

    if (point->y >= nameoffset)
    {
        if (WasTextureClicked(point,&clickIndex,m))
        {
            if ((modKeys & MK_SHIFT) == 0)
            {
                lmousedown = 1;
                clickArea = AREA_TEX;
                SetCapture(hwnd);
                //		case AREA_TEX:
                if (point->y >= nameoffset)
                {
                    if (WasTextureClicked(point,&idx,m))
                    {
                        tx = m->mapTextures[idx];
                        if (tx)
                        {
                            STRNCPY(m->texName,tx->bmp);
                            if (set.game_mode == 2)
                                STRNCPY(m->texPath,tx->basepath);
                            texList->SetText(m->texName);
                            NewCurrentTexture(tx);
                            if (frame->TextureList)
                                frame->TextureList->SetSelIndex(idx);
                        }
                        Select(idx,true);
                    }
                }
                else if (set.show_favorites && PtInRect(&ClipFavorites,*point) && m->favorites)
                {
                    idx = (point->x - ClipFavorites.left)/(set.height_fav+(set.show_tex_names*TEXHSPACE));

                    if ((idx >=0) && (idx < set.tex_numfav) && (idx < m->numMapTextures)
                            && (clickIndex < m->numMapTextures) && (clickIndex != -1))
                    {
                        m->favorites[idx] = m->mapTextures[clickIndex];
                        tx = m->favorites[idx];
                        if (tx)
                        {
                            clickIndex = GetTextureIndex(m,tx);

                            if ((clickIndex != -1) && (clickIndex < m->numMapTextures))
                            {
                                STRNCPY(m->texName,tx->bmp);
                                if (set.game_mode == 2)
                                    STRNCPY(m->texPath,tx->basepath);
                                texList->SetText(m->texName);
                                NewCurrentTexture(tx);
                                if (frame->TextureList)
                                    frame->TextureList->SetSelIndex(clickIndex);

                                prevIndex = clickIndex;
                                curstartrow = (int)(prevIndex/colsperrow);

                                UpdateScrollbar();

                                RedrawFavorite();
                                Select(prevIndex,true);
                            }
                        }
                    }
                }

                //end test
            }
        }
    }
    else if (set.show_favorites && PtInRect(&ClipFavorites,*point) && m->favorites)
    {
        idx = (point->x - ClipFavorites.left)/(set.height_fav+(set.show_tex_names*TEXHSPACE));

        if ((idx >=0) && (idx < set.tex_numfav))
        {
            tx = m->favorites[idx];
            if (tx)
                clickIndex = GetTextureIndex(m,tx);
        }
        if ((modKeys & MK_SHIFT) == 0)
        {
            lmousedown = 1;
            clickArea = AREA_FAV;
            SetCapture(hwnd);
        }
    }
    else if (set.show_frequent && PtInRect(&ClipFrequent,*point) && m->bests)
    {
        idx = (point->x - ClipFrequent.left)/(set.height_frq+(set.show_tex_names*TEXHSPACE));

        if ((idx >=0) && (idx < set.tex_numfrq))
        {
            tx = m->bests[idx];
            if (tx)
                clickIndex = GetTextureIndex(m,tx);
        }
        if ((modKeys & MK_SHIFT) == 0)
        {
            lmousedown = 1;
            clickArea = AREA_FRQ;
            SetCapture(hwnd);
        }
    }

    if (lmousedown)
        return;

    //shift+drag to zoom texture
    if ((modKeys & MK_SHIFT) && (clickIndex >= 0) && (clickIndex < m->numMapTextures))
    {
        SetCapture(hwnd);
        oldmag = 0.0f;			//oldmag must be outside of range(1-12) so initial redraw is triggered
        startpt.x -= 12*20/2;	//hack mouse pos to make zoom start at middle
        if(startpt.x < 0) startpt.x = 0;
        lmousedown = 2;
        ZoomTexture(point);
    }
}

void TTextureWindow::EvRButtonDown(UINT modKeys, LPPOINT point)
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];

    if (modKeys & MK_SHIFT)
    {
        int clickIndex = -1;
        if (point->y >= nameoffset)
        {
            if (WasTextureClicked(point,&clickIndex,m))
            {
                if ((clickIndex >= 0) && (clickIndex < m->numMapTextures))
                {
                    m->mapTextures[clickIndex]->selected ^= 1;
                    STRNCPY(m->texName,m->mapTextures[clickIndex]->bmp);
                    if (set.game_mode == 2)
                        STRNCPY(m->texPath,m->mapTextures[clickIndex]->basepath);
                    texList->SetText(m->texName);
                    NewCurrentTexture(m->mapTextures[clickIndex]);
                    if (frame->TextureList)
                        frame->TextureList->SetSelIndex(clickIndex);

                    Select(clickIndex,true);
                    RedrawContents();
                }
            }
        }
        return;
    }

    rightClickedTexture = NULL;
    int clickIndex = -1;
    if (WasTextureClicked(point,&clickIndex,m))
    {
        if ((clickIndex >= 0) && (clickIndex < m->numMapTextures))
        {
            rightClickedTexture = m->mapTextures[clickIndex];
        }
    }
    // show the menu...
    Popup menu;
    char tempStr[64];

    menu.Add("Reset", ID_SBASE+10);
    menu.Add("Set Shift Step...", 10407);
    menu.Add("Set Scale Step...", 10405);
    menu.Add("Set Rotate Step...", 10408);
    menu.Add("Recalculate Frequents", IDB_CALC);
    menu.Separator();
    menu.Add("Set Thumbnail Size", 10404);
    menu.Add("Smaller Thumbnails", ID_SBASE+12);
    menu.Add("Bigger Thumbnails", ID_SBASE+13);

    menu.Add("Small Thumbnails", 10402, (bool)useSmalls);
    menu.Add("Animate", 10400, (bool)set.tex_animate);
    menu.Add("Animate Sky", 10403, (bool)set.tex_animatesky);
    menu.Separator();

    if (set.glBsp)
    {
        menu.Add("OpenGL Settings...", GL_SETTINGS);
        menu.Separator();
    }
    menu.Add("Favorites", IDB_FAV, (bool)set.show_favorites);
    menu.Add("Frequents", IDB_FRQ, (bool)set.show_frequent);
    menu.Add("Stretch", ID_SBASE+11, (bool)set.stretch_textures);
    menu.Add("Names", IDB_NAME, (bool)set.show_tex_names);
    menu.Separator();
    menu.Add("List Used Textures", IDB_CALCUSAGE);
    if (!set.glBsp)
    {
        menu.Separator();
        menu.Add("Save 3d Window Area to MipMap...", SAVE_WAD);
    }
    menu.Separator();
    menu.Add("Select All", SELECT_ALL);
    menu.Add("Deselect All", DESELECT_ALL);
    menu.Add("Select Used", SELECT_USED);
    menu.Add("Save Selected to Wad...", SAVE_SELECTED);

    // Q2 only?
    menu.Separator();
    menu.Add("Unload Wad/Subdirectory/Used...\tBackSpace", UNLOAD_WAD);
    menu.Add("Load texture Wad file...\tW", SCAN_NEW_WAD);
    if (set.game_mode == 2)
    {
        menu.Add("Load texture directory...\tL", SCAN_NEW_WALDIR);
        menu.Add("Load textures from default (Q2) .pak...\tP", SCAN_WAL_PAKQ2);
        menu.Add("Load textures from selected .pak...", SCAN_WAL_PAK);
        menu.Add("Load textures used in current .map...\tU", SCAN_IN_USE);
        menu.Add("Load a texture from a .wal file...", SCAN_NEW_WAL);
        menu.Add("Convert a Wad file to .wal Textures...", WAD_TO_WAL);
    }

    // copy paste
    if (!set.glBsp)
    {
        menu.Separator();
        menu.Add("Copy 3d Window Area to Clipboard...", CLIP_COPY_IMAGE);
    }
    if (rightClickedTexture)
    {
        sprintf(tempStr,"Copy [%s] To Clipboard...",rightClickedTexture->bmp);
        menu.Add(tempStr, CLIP_COPY);
    }
    if (IsClipboardFormatAvailable(CF_DIB))
    {
        menu.Add("Paste from Clipboard...",CLIP_PASTE);
    }

    menu.Separator();
    strcpy(tempStr,"Keep On Top");
    if (TCWindow::topmost == hwnd)
        menu.Check();
    menu.Add(tempStr, 10406);

    menu.Show(hwnd);
}

bool TTextureWindow::CanClose()
{
    if (!KillAllWindows)
    {
        ShowWindow(SW_MINIMIZE);
    }
    return KillAllWindows;
}

TTextureWindow::TTextureWindow(HWND parent, char *title)
    : TCWindow(parent,0), Scroller(0)
{
    attr.dwStyle |=	WS_VSCROLL|CS_VREDRAW|CS_HREDRAW;
    attr.width = 300;
    attr.height = 500;
    *texStatic = 0;

    waterFrame = 0;
    ha = 4.0f;
    va = 4.0f;
    hp = 64.0f;
    vp = 64.0f;
    tperiod = 16;

    xo_lookup = new float[tperiod];
    yo_lookup = new float[tperiod];

    for (int i = 0; i < tperiod; i++)
    {
        xo_lookup[i] = ha * sin(2.0f * M_PI * ((float)i / (float)tperiod));
        yo_lookup[i] = va * cos(2.0f * M_PI * ((float)i / (float)tperiod));
    }

    lastSz.cx = lastSz.cy = -1;
    stepSize = 10;

    SetCaption(title);
    DoNCPaint();

    hmemdc = 0;
    hmembitmap = NULL;
    hmemdcFF = 0;
    hmembitmapFF = NULL;

    /////
    badQ = new qtexture_t;
    memset(badQ,0,sizeof(badQ));
    strcpy(badQ->name,"badtex");
    badQ->width = 32;
    badQ->height = 32;
    if (set.sinBsp)
    {
        badQ->data = new unsigned char[3*32*32+1];
        memset(badQ->data,0,3*32*32+1);
        uintptr_t ofs;
        unsigned char *pBits;

        pBits = (unsigned char *)badQ->data;

        for (int i = 0; i < 32; i+=4)
        {
            for (int j = 0; j < 32; j+=4)
            {
                ofs = 3*(uintptr_t)(i*32 + j);
                pBits[ofs++] = 0;
                pBits[ofs++] = 0;
                pBits[ofs++] = 255;
                pBits[ofs++] = 0;
                pBits[ofs++] = 0;
                pBits[ofs]   = 255;
                ofs = 3*(uintptr_t)((i+1)*32 + j);
                pBits[ofs++] = 0;
                pBits[ofs++] = 0;
                pBits[ofs++] = 255;
                pBits[ofs++] = 0;
                pBits[ofs++] = 0;
                pBits[ofs]   = 255;
            }
        }
    }
    else
    {
        int tempcol = 127+(rand() & 0x7F);
        badQ->data = new unsigned char[32*32+1];
        memset(badQ->data,tempcol,32*32+1);
    }

    int flatc = (rand() & 128) + 127;
    badQ->flatcolor.rgb = RGB(flatc,flatc,flatc);
    badQ->flatcolor.p = 255;  // white
    badQ->bindIndex = 1; // hack

    lmousedown = 0;

    texList   = NULL;
    prevIndex = 0;

    firstWad = new texWad;
    currentWadPtr = firstWad;
    strcpy(currentWadPtr->currentWad,"None");
    currentWadPtr->WadType = QUAKE_WAD;
    currentWadPtr->bmpsCount = 0;
    currentWadPtr->textures  = NULL;
    currentWadPtr->nextWad = NULL;

    fulltw = set.texture_width;
    fullth = set.texture_height;
    useSmalls = 0;

    saveNames = set.show_tex_names;


    noffset = 24;

    CalcFF();
    CalcWH();
}

//WindowPlacement
HWND TTextureWindow::WP_GetHwnd()
{
    return hwnd;
}
const char *TTextureWindow::WP_WindowName()
{
    return "Texture";
}


void TTextureWindow::ToggleSky()
{
    set.tex_animatesky ^= 1;
}

void TTextureWindow::ToggleSmalls()
{
    if (!set.Map_Read)
        return;

    useSmalls ^= 1;
    if (useSmalls)
    {
        set.texture_width = set.tex_small_width;
        set.texture_height = set.tex_small_height;
        saveNames = set.show_tex_names;
        set.show_tex_names = 0;
    }
    else
    {
        set.texture_width = fulltw;
        set.texture_height = fullth;
        set.show_tex_names = saveNames;
    }

    CalcWH();
    RemoveMemDC();
    CalcMetrics();
    ConstructMemDC();
    RedrawContents();
}

void TTextureWindow::SetWH()
{
    if (!set.Map_Read)
        return;

    char s[32];
    sprintf(s,"%i",max(set.texture_width,set.texture_height));
    if (IDOK != InputDialog(hwnd, const_cast<char *> ("Change Texture Size..."), const_cast<char *> ("New Texture Width/Height [4 - 256] ? "), s, sizeof(s)).Execute())
        return;

    int wh = atoi(s);

    if ((wh < 4) || (wh > 256))
        return;

    set.texture_width = set.texture_height = wh;

    CalcWH();
    RemoveMemDC();
    CalcMetrics();
    ConstructMemDC();
    RedrawContents();
}

void TTextureWindow::SetScaleStep()
{
    if (!set.Map_Read)
        return;

    char s[64];
    sprintf(s,"%g",set.tex_scalestep);

    if (IDOK != InputDialog(hwnd, const_cast<char *> ("Change Scale Step..."), const_cast<char *> ("New Scale Step [0.0001 - 1.0] ? "), s, sizeof(s)).Execute())
        return;

    float step = (float) atof(s);

    step = max(step,0.0001f);
    step = min(step,1.0f);
    set.tex_scalestep = step;

    sprintf(s,"Scale step set to %g...",step);
    status->SetText(s,true);
}

void TTextureWindow::SetShiftStep()
{
    if (!set.Map_Read)
        return;

    char s[64];
    sprintf(s,"%g",set.tex_shiftstep);

    if (IDOK != InputDialog(hwnd, const_cast<char *> ("Change Shift Step..."), const_cast<char *> ("New Shift Step [0.0001 - 128.0] ? "), s, sizeof(s)).Execute())
        return;

    float step = (float) atof(s);
    step = max(step,0.0001f);
    step = min(step,128.0f);
    set.tex_shiftstep = step;

    sprintf(s,"Shift step set to %g...",step);
    status->SetText(s,true);
}

void TTextureWindow::SetRotateStep()
{
    if (!set.Map_Read)
        return;

    char s[64];
    sprintf(s,"%g",set.tex_rotatestep);

    if (IDOK != InputDialog(hwnd, const_cast<char *> ("Change Rotation Step..."), const_cast<char *> ("New Rotation Step [0.0001 - 90.0] ? "), s, sizeof(s)).Execute())
        return;

    float step = (float) atof(s);
    step = max(step,0.0001f);
    step = min(step,90.0f);
    set.tex_rotatestep = step;

    sprintf(s,"Rotation step set to %g...",step);
    status->SetText(s,true);
}

void TTextureWindow::ToggleNames()
{
    set.show_tex_names ^= 1;

    CalcWH();
    CalcFF();

    RemoveMemDC();
    CalcMetrics();
    ConstructMemDC();
    RedrawContents();
}

void TTextureWindow::ToggleFav()
{
    set.show_favorites ^= 1;

    CalcFF();

    RemoveMemDC();
    CalcMetrics();
    ConstructMemDC();
    RedrawContents();
}

void TTextureWindow::ToggleFrq()
{
    set.show_frequent ^= 1;

    CalcFF();

    RemoveMemDC();
    CalcMetrics();
    ConstructMemDC();
    RedrawContents();
}

TTextureWindow::~TTextureWindow()
{
    if (set.tex_animate)
        StopAnimation();

    RemoveMemDC();

    delete[] xo_lookup;
    delete[] yo_lookup;

    // xxx cycle through them!
    texWad *cur = firstWad;
    while (cur)
    {
        texWad *temp = cur;
        cur = cur->nextWad;
        if (temp->textures)
            FreeTextures(temp);
        delete temp->textures; // cause head is left over? what about tail?
        delete temp;
    }
    delete [] texList;
    delete [] currentTexture;
    delete [] badQ->data;
    delete [] badQ;
    delete [] texColorMap;
}

void TTextureWindow::ChangeTexture()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];
    if(!m)
        return;


    char temp[64];
    int len = 15;
    texInfo_t *tx;

    texList->GetText(temp,len);
    strupr(temp);

    int retval = texList->FindString(temp,0);

    if (retval < 0)
    {
        char temp2[80];
        int c;
        c = texList->GetCount();
        retval = -1;
        for (int i = 0; i < c; i++)
        {
            texList->GetString(temp2,i);
            if (QCompare(temp,temp2) <= 0)
            {
                retval = i; // PREVIOUS!
                break;
            }
        }
    }
    if (retval < 0)
        retval = 0;

    prevIndex = retval;
    texList->GetString(m->texName,retval);
    texList->SetText(m->texName);
    if (set.game_mode == 2)
        strcpy(m->texPath,"UNKNOWN");

    if ((prevIndex < 0) || (prevIndex >= m->numMapTextures))
        return;

    tx = m->mapTextures[prevIndex];
    if (tx)
    {
        NewCurrentTexture(tx);
        if (set.game_mode == 2)
            STRNCPY(m->texPath,tx->basepath);
        if (frame->TextureList)
            frame->TextureList->SetSelIndex(prevIndex);
    }

    curstartrow = (int)(prevIndex/colsperrow);

    UpdateScrollbar();

    Select(prevIndex,true);
}


void TTextureWindow::SetupWindow()
{
    texList = new WCombo(hwnd, ID_COMBOBOX1, 0, 0, LABEL_WIDTH, 450);
    texList->attr.dwStyle |= CBS_DROPDOWN;
    texList->Create();
    texList->LimitText(40);
    texList->SetFont(set.font9);

    currentTexture = new texturedef_t;
    memset(currentTexture,0,sizeof(texturedef_t));
    currentTexture->scale[0] = 1.0f;
    currentTexture->scale[1] = 1.0f;
    if (set.game_mode == 2)
    {
        currentTexture->texType = TEX_TYPE_Q2;
        sprintf(currentTexture->basepath,"UNKNOWN%c",set.rel_path_separator);
    }

    strcpy(currentTexture->texture,"none");

    Scroller.hwnd = hwnd;
    Scroller.Y.nMax = 0;//currentWadPtr->bmpsCount;
    Scroller.Y.nPage = 0;//1;
    prevIndex = 0;

    texColorMap = new TColorMap(set.pal);
    // otherwise, just copy same thing into all entries...

    if (set.tex_animate)
        StartAnimation();

    Invalidate();
}

void TTextureWindow::StretchButton()
{
    set.stretch_textures ^= 1;
    RedrawContents();
}
void TTextureWindow::TDButton()
{
    if (set.texture_width >= 12 && set.texture_height >= 12)
    {
        set.texture_width -= 4;
        set.texture_height -= 4;
    }
    fulltw = set.texture_width;
    fullth = set.texture_height;
    CalcWH();

    RemoveMemDC();
    CalcMetrics();
    ConstructMemDC();
    RedrawContents();
}

void TTextureWindow::TUButton()
{
    if (set.texture_width <= 252 && set.texture_height <= 252)
    {
        set.texture_width += 4;
        set.texture_height += 4;
    }
    fulltw = set.texture_width;
    fullth = set.texture_height;
    CalcWH();

    RemoveMemDC();
    CalcMetrics();
    ConstructMemDC();
    RedrawContents();
}

void TTextureWindow::GetTextureDisplayRects(int x, int y,LPRECT src, LPRECT dst, texInfo_t *tx, int tw, int th)
{
    float heightScale, widthScale, useScale;

    dst->left = x;
    dst->top = y;

    if (!set.stretch_textures)
    {
        src->right  = min(tw, tx->w);
        src->bottom = min(th, tx->h);
        dst->right  = dst->left + min(src->right, tw);
        dst->bottom = dst->top  + min(src->bottom, th);
    }
    else
    {
        if (set.nosmallstretch && ((tx->w <= tw) || (tx->h <= th)))
        {
            if ((tx->w <= tw) && (tx->h <= th))
            {
                src->right  = tx->w;
                src->bottom = tx->h;

                dst->right  = dst->left + tx->w;
                dst->bottom = dst->top + tx->h;
            }
            else
            {
                if (tx->w <= tw)   // width is okay
                {
                    heightScale = (float)th/(float)tx->h;
                    src->right  = tx->w;
                    src->bottom = tx->h;
                    dst->right  = dst->left + (int)(heightScale * tx->w);
                    dst->bottom = dst->top + (int)(heightScale * tx->h);
                }
                else
                {
                    widthScale = (float)tw/(float)tx->w;
                    src->right  = tx->w;
                    src->bottom = tx->h;
                    dst->right  = dst->left + (int)(widthScale * tx->w);
                    dst->bottom = dst->top + (int)(widthScale * tx->h);
                }
            }
        }
        else
        {
            if (set.nosmallstretch)
            {
                heightScale = (float)th/(float)tx->h;
                widthScale  = (float)tw/(float)tx->w;

                src->right  = tx->w;
                src->bottom = tx->h;

                useScale = min(heightScale,widthScale);
                dst->right  = dst->left + (int)(useScale*tx->w);
                dst->bottom = dst->top  + (int)(useScale*tx->h);
            }
            else
            {
                src->right  = tx->w;
                src->bottom = tx->h;
                dst->right  = dst->left + tw;
                dst->bottom = dst->top  + th;
            }
        }
    }
}

void TTextureWindow::Paint()
{
    if(!IsVisible())
        return;

    RedrawContents();
    needredraw = 0;
    Select(prevIndex,false);
    needredraw = 1;
    ValidateRect(hwnd,0);
}

void TTextureWindow::RemoveMemDC()
{
    if(hmemdc)			DeleteDC (hmemdc);
    if(hmembitmap)		DeleteObject(hmembitmap);
    if(hmemdcFF)		DeleteDC (hmemdcFF);
    if(hmembitmapFF)	DeleteObject(hmembitmapFF);
    hmemdc = 0;
    hmembitmap = 0;
    hmemdcFF = 0;
    hmembitmapFF = 0;
}

void TTextureWindow::ConstructMemDC()
{
    RECT rc;
    GetClientRect(hwnd,&rc);

    HDC hdc = GetDC(hwnd);
    // allocate the memory dc and the bitmap
    hmemdc = CreateCompatibleDC(hdc);

    int ht = max(1, rc.bottom - rc.top - nameoffset);	// exact number of pixels, at least 1

    hmembitmap = CreateCompatibleBitmap(hdc, MainRect.right-MainRect.left, ht);

    SelectObject(hmemdc, hmembitmap);	 // install bitmap into memDC

    hmemdcFF = CreateCompatibleDC(hdc);
    int tempW = max(1, FFRect.right-FFRect.left);
    int tempH = max(1, FFRect.bottom-FFRect.top);
    hmembitmapFF = CreateCompatibleBitmap(hdc, tempW, tempH);

    SelectObject(hmemdcFF, hmembitmapFF);	 // install bitmap into memDC

    lastSz.cx = rc.right - rc.left;
    lastSz.cy = rc.bottom - rc.top;

    ReleaseDC(hwnd, hdc);
}

void TTextureWindow::EvSize(UINT sizeType, LPSIZE size)
{
    if (!IsIconic())
    {
        CalcMetrics();
        if (size->cx != lastSz.cx || size->cy != lastSz.cy)
        {
            RemoveMemDC();
            ConstructMemDC();
            lastSz.cx = size->cx;
            lastSz.cy = size->cy;
        }
        RedrawContents();
        Select(prevIndex,true);
    }
    ValidateRect(hwnd,0);
}

void TTextureWindow::ClearContents()
{
    if (IsIconic() || !IsWindowVisible())
        return;
    if (!hmemdc  || !hmembitmap)
        return;
    if (!hmemdcFF  || !hmembitmapFF)
        return;

    RECT rc;
    SetRect(&rc,0,0, MainRect.right-MainRect.left, 0);

    HBRUSH brush = CreateSolidBrush(set.color_texturewindow);

    GetClientRect(hwnd,&rc);
    rc.bottom = max(1, rc.bottom - rc.top - nameoffset);	// exact number of pixels, at least 1

    FillRect(hmemdc, &rc, brush);

    SetRect(&rc,0,0, FFRect.right-FFRect.left, FFRect.bottom-FFRect.top);
    FillRect(hmemdcFF, &rc, brush);
    DeleteObject(brush);
}

void TTextureWindow::RedrawContents()
{
    if (!IsVisible())
        return;
    if (!set.Map_Read)
        return;
    if (!hmemdc || !hmembitmap)
        return;
    if (!hmemdcFF || !hmembitmapFF)
        return;

    GdiFlush();

    map *m = map_i[set.curmap];
    if(!m)
        return;

    int curcol, currow;
    int c;
    RECT dst, src, r, t, rc;

    SetRect(&t, 0, nameoffset, MainRect.right-MainRect.left, 0);

    GetClientRect(hwnd, &r);
    HDC hdc = GetDC(hwnd);

    HDC cdc = CreateCompatibleDC(hdc);
    HBITMAP hbmp = CreateCompatibleBitmap(hdc,r.right,r.bottom);
    HGDIOBJ oldcdcbmp = SelectObject(cdc, hbmp);

    HPEN pen = CreatePen(PS_SOLID,1,COLOR_BTNTEXT);
    HBRUSH tbr = CreateSolidBrush(set.color_texturewindow);

    FillRect(cdc,&r,tbr);

    RedrawClient(cdc);

    HGDIOBJ oldcdcpen = SelectObject(cdc,pen);
    MoveToEx(cdc,r.left,noffset,0);
    LineTo(cdc,r.right,noffset);

    HGDIOBJ oldmemfont = SelectObject(hmemdc, set.font9);
    SetTextColor(hmemdc, set.color_foreground);
    SetBkColor(hmemdc, set.color_texturewindow);
    HGDIOBJ oldFFfont = SelectObject(hmemdcFF, set.font9);
    //SetTextColor(hmemdcFF, set.color_foreground);
    SetBkColor(hmemdcFF, set.color_texturewindow);

    curstartrow = Scroller.Y.nPos;
//   curstartrow = max(0,(int)curstartrow); // at least zero...

    ClearContents();

    SetStretchBltMode(hmemdc, COLORONCOLOR);
    SetStretchBltMode(hmemdcFF, COLORONCOLOR);

    src.left   = 0;
    src.top    = 0;
    c = 0;

    GetClientRect(hwnd,&rc);
    int ht = max(1,(rc.bottom-rc.top) - nameoffset);	// exact number of pixels, at least 1
    SetRect(&rc,0,0, MainRect.right-MainRect.left, ht);
    FillRect(hmemdc,&rc,tbr);

    for (currow = 0; currow <= totrows; currow++)
    {
        for (curcol = 0; curcol < colsperrow; curcol++)
        {
            if (c >= m->numMapTextures)
                break;

            if (currow < curstartrow)  		    // skip to first row on fast render...
            {
                c++;
                continue;
            }

            if (currow >= (curstartrow+rowsperpage)+1)
                break;										// past end, just exit loop...

            dst.left = trueW*curcol + (set.show_tex_names*TEXHSPACE)/2;
            dst.top = trueH*(currow - curstartrow);

            texInfo_t *tx = m->mapTextures[c++];
            if (!tx || !tx->dibInfo)
                continue;

            char *outStart = tx->bmp;
            int animated = tx->animated;
            if ((*outStart == '+') && (animated == 1))
            {
                outStart++;
                int bc = 0;
                while(isdigit((unsigned char)*outStart) && (bc++ <= 3))
                    outStart++;
            }
            else if(tx->texType==TEX_TYPE_Q1 && animated==2)
            {
                outStart++;
            }

            GetTextureDisplayRects(dst.left, dst.top, &src, &dst, tx, set.texture_width, set.texture_height);

            StretchDIBits(hmemdc,
                          dst.left,dst.top,dst.right-dst.left,dst.bottom-dst.top,
                          src.left,src.top,src.right-src.left,src.bottom-src.top,
                          tx->bits, tx->dibInfo,
                          DIB_RGB_COLORS, SRCCOPY);


            dst.right  = trueW*(curcol+1)-(set.show_tex_names*TEXHSPACE)/2;
            dst.bottom = trueH*((currow-curstartrow)+1)-(set.show_tex_names*TEXVSPACE);

            if (set.show_tex_names)
            {
                if (tx->animated)
                    SetTextColor(hmemdc,RGB(255,255,0));
                if (tx->selected)
                    SetBkColor(hmemdc,RGB(192,192,192));

                TextOut(hmemdc,dst.left,dst.bottom+TEXFONTBUFFER, outStart,strlen(outStart));

                if (tx->animated)
                    SetTextColor(hmemdc,set.color_foreground);
                if (tx->selected)
                    SetBkColor(hmemdc,set.color_texturewindow);

            }
            else
            {
                if (tx->selected)
                {
                    SetRect(&rc, dst.left + 1,dst.top + 1, dst.left + 4, dst.top + 4);

                    HBRUSH sColorBrush = CreateSolidBrush(set.color_selectedtexture);
                    FillRect(hmemdc, &rc, sColorBrush);
                    DeleteObject(sColorBrush);
                }
            }
        }
    }

    SelectObject(hmemdc,oldmemfont);
    SelectObject(hmemdcFF,oldFFfont);

    /////////////////////// Favorites and Frequents...
    if (set.show_favorites)
        RedrawFavorite();
    if (set.show_frequent)
        RedrawFrequent();

    int orig_y = 0;

    GetClientRect(hwnd,&rc);
    t.bottom = t.top + max(1,(rc.bottom-rc.top) - nameoffset);	// exact number of pixels, at least 1

    BitBlt(cdc,t.left,t.top,t.right-t.left,t.bottom-t.top,hmemdc, 0,orig_y,SRCCOPY);

    t.top = noffset;
    t.bottom = nameoffset;
    BitBlt(cdc,t.left,t.top,t.right-t.left,t.bottom-t.top,hmemdcFF, 0,0,SRCCOPY);

    if (!inSelect)
        Select(prevIndex,true);

    BitBlt(hdc,r.left,r.top,r.right-r.left,r.bottom-r.top,cdc,0,0,SRCCOPY);

    ReleaseDC(hwnd, hdc);
    SelectObject(cdc,oldcdcpen);
    SelectObject(cdc,oldcdcbmp);
    DeleteDC(cdc);
    DeleteObject(pen);
    DeleteObject(tbr);
    DeleteObject(hbmp);
}

void TTextureWindow::Select(int Idx, bool moveScroller)
{
    if (IsIconic())
        return;
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];
    if(!m)
        return;
    if (m->numMapTextures <= 0)
        return;
    if (!m->mapTextures)
        return;
    if (!hmemdc || !hmembitmap)
        return;
    if (!hmemdcFF || !hmembitmapFF)
        return;

    if (Idx >=0 && moveScroller)
    {
        curstartrow = Scroller.Y.nPos;

        if (moveScroller)
        {
            UpdateScrollbar(false,false);
        }
    }

    if (needredraw)
    {
        inSelect = 1;
        RedrawContents();
        inSelect = 0;
        ValidateRect(hwnd,0);
    }

    RECT dst, r, t;
    GetClientRect(hwnd, &r);

    HDC hdc = GetDC(hwnd);

    SetRect(&t,0, noffset, MainRect.right-MainRect.left, nameoffset);
    BitBlt(hdc, t.left,t.top,t.right-t.left,t.bottom-t.top, hmemdcFF, 0,0,SRCCOPY);

    SetTextColor(hdc, set.color_foreground);
    SetBkColor(hdc, set.color_texturewindow);

    if (Idx >= 0)
    {

        int currow = Idx / colsperrow;
        int curcol = Idx % colsperrow;

        currow -= curstartrow;

        if ((currow >= 0) && (currow <= rowsperpage))
        {
            dst.left   = trueW * curcol + (set.show_tex_names * TEXHSPACE) / 2;
            dst.top    = nameoffset + trueH*currow;
            dst.right  = trueW * (curcol + 1) - (set.show_tex_names * TEXHSPACE) / 2;
            dst.bottom = nameoffset + trueH*(currow+1)-(set.show_tex_names*TEXVSPACE);

            InflateRect(&dst,1,1);

            HBRUSH tbr = CreateSolidBrush(set.color_textureselection);
            FrameRect(hdc,&dst,tbr);
            DeleteObject(tbr);
        }
        prevIndex = Idx;

        SelectFF(hdc);
    }
    ReleaseDC(hwnd,hdc);
}

void TTextureWindow::RedrawFavorite()
{
    RECT t, src, dst;
    CopyRect(&t, &ClipFavorites);
    t.top    = ClipFavorites.top - noffset;
    t.bottom = t.top + (ClipFavorites.bottom-ClipFavorites.top);

    HBRUSH tbr = CreateSolidBrush(set.color_texturewindow);
    HGDIOBJ oldbr = SelectObject(hmemdcFF, tbr);
    FillRect(hmemdcFF, &t, tbr);

    SetTextColor(hmemdcFF,set.color_foreground);
    SetBkColor(hmemdcFF,set.color_texturewindow);
    HGDIOBJ oldfont = SelectObject(hmemdcFF,set.font9);
    TextOut(hmemdcFF,2,t.top,"Favorites",9);	// 9 == strlen "Favorites"

    SelectObject(hmemdcFF,oldbr);
    SelectObject(hmemdcFF,oldfont);
    DeleteObject(tbr);

    texInfo_t *tx;
    src.left = 0;
    src.top = 0;

    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    if (!m->favorites)
        return;

    for (int c = 0; c < set.tex_numfav; c++)
    {
        tx = m->favorites[c];
        if (!tx)
            continue;

        dst.left = ClipFavorites.left+(set.height_fav+(set.show_tex_names*TEXHSPACE))*c+(set.show_tex_names*TEXHSPACE)/2;
        dst.top  = t.top;

        if (!tx->dibInfo)
        {
            MessageBox(hwnd,"Empty DibInfo......?", "BSP", MB_OK);
            continue;
        }

        GetTextureDisplayRects(dst.left, dst.top, &src, &dst, tx, set.height_fav, set.height_fav);

        if (tx->dibInfo && tx->bits)
        {
            SetStretchBltMode(hmemdcFF, COLORONCOLOR);

            StretchDIBits(hmemdcFF,
                          dst.left,dst.top, dst.right-dst.left, dst.bottom-dst.top,
                          src.left,src.top, src.right-src.left, src.bottom-src.top,
                          tx->bits, tx->dibInfo,
                          DIB_RGB_COLORS, SRCCOPY);
        }
    }
}

void TTextureWindow::RedrawFrequent()
{
    RECT t, src, dst;

    CopyRect(&t, &ClipFrequent);
    t.top    = ClipFrequent.top - noffset;
    t.bottom = t.top + (ClipFrequent.bottom-ClipFrequent.top);

    HBRUSH tbr = CreateSolidBrush(set.color_texturewindow);
    HGDIOBJ oldbr = SelectObject(hmemdcFF, tbr);
    FillRect(hmemdcFF, &t, tbr);

    SetTextColor(hmemdcFF, set.color_foreground);
    SetBkColor(hmemdcFF, set.color_texturewindow);
    HGDIOBJ oldfont = SelectObject(hmemdcFF, set.font9);
    TextOut(hmemdcFF, 2, t.top, "Frequent",8);	// 8 == strlen "Frequent"

    SelectObject(hmemdcFF,oldbr);
    SelectObject(hmemdcFF,oldfont);
    DeleteObject(tbr);

    texInfo_t *tx;
    src.left = 0;
    src.top = 0;

    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];
    if (!m->bests)
        return;

    for (int c = 0; c < set.tex_numfrq; c++)
    {
        tx = m->bests[c];
        if (!tx)
            continue;

        dst.left = ClipFrequent.left+(set.height_frq+(set.show_tex_names*TEXHSPACE))*c+(set.show_tex_names*TEXHSPACE)/2;
        dst.top  = t.top;

        if (!tx->dibInfo)
        {
            MessageBox(hwnd,"Empty DibInfo...?", "BSP", MB_OK);
            continue;
        }

        GetTextureDisplayRects(dst.left, dst.top, &src, &dst, tx, set.height_frq, set.height_frq);

        if (tx->dibInfo && tx->bits)
        {
            SetStretchBltMode(hmemdcFF, COLORONCOLOR);

            StretchDIBits(hmemdcFF,
                          dst.left, dst.top, dst.right-dst.left, dst.bottom-dst.top,
                          src.left, src.top, src.right-src.left, src.bottom-src.top,
                          tx->bits, tx->dibInfo,
                          DIB_RGB_COLORS, SRCCOPY);
        }
    }
}

void TTextureWindow::SelectFF(HDC hdc)
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];
    if (!m->bests || !m->favorites)
        return;

    int c;

    texInfo_t *tx;
    RECT src, dst, t;
    CopyRect(&t, &ClipFrequent);

    SetTextColor(hdc, set.color_foreground);
    SetBkColor(hdc, set.color_texturewindow);

    HBRUSH tbr = CreateSolidBrush(set.color_textureselection);
    HGDIOBJ oldbr = SelectObject(hdc, tbr);

    src.left = 0;
    src.top = 0;

    if (set.show_frequent)
    {
        for (c = 0; c < set.tex_numfrq; c++)
        {
            tx = m->bests[c];

            if (!tx)
                continue;

            if (prevIndex == GetTextureIndex(m,tx))
            {
                dst.left   = t.left+(set.height_frq+(set.show_tex_names*TEXHSPACE))*c+(set.show_tex_names*TEXHSPACE)/2;
                dst.top    = t.top;
                dst.right  = dst.left + (int)set.height_frq;
                dst.bottom = dst.top  + (int)set.height_frq;

                FrameRect(hdc, &dst, tbr);
            }
        }
    }

    t = ClipFavorites;

    if (set.show_favorites)
    {
        for (c = 0; c < set.tex_numfav; c++)
        {
            tx = m->favorites[c];

            if (!tx)
                continue;

            if (prevIndex == GetTextureIndex(m,tx))
            {
                dst.left   = t.left+(set.height_fav+(set.show_tex_names*TEXHSPACE))*c+(set.show_tex_names*TEXHSPACE)/2;
                dst.top    = t.top;
                dst.right  = dst.left + (int)set.height_fav;
                dst.bottom = dst.top  + (int)set.height_fav;

                FrameRect(hdc, &dst, tbr);
            }
        }
    }

    SelectObject(hdc,oldbr);
    DeleteObject(tbr);
}

void TTextureWindow::CalcFrequents()
{
    if (!set.Map_Read)
        return;

    texInfo_t **counts;
    Entity *e;
    SetBrush *b;
    int c3;
    int i,j,w;
    face_t *f;

    map *m = map_i[set.curmap];
    if (!m)
        return;

    c3 = m->numMapTextures;

    if (c3 <=0)
        return;

    counts = new texInfo_t *[c3];
    memset(counts,0,c3*sizeof(texInfo_t *));

    ClearCounts(m); // clear them out...

    for (i = 0; i < c3; i++)
    {
        counts[i] = m->mapTextures[i];
    }

    for (e = m->objects.p_next; e != &m->objects; e = e->p_next)
    {
        //if (e == m->world)
        //  	continue;
        if (!e->modifiable)
            continue;

        for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
        {
            for (f = b->faces.p_next; f != &b->faces; f = f->p_next)
            {
                for (w = 0; w<c3; w++)
                {
                    if (!strnicmp(f->texture.texture, counts[w]->bmp,LUMP_NAME_LENGTH))
                    {
                        counts[w]->count++;
                        break;
                    }
                }
            }
        }
    }

    texInfo_t *temp;
    // all counts filled in, now set up the bests array...
    for (i = 0; i < c3; i++)
    {
        for (j = i+1; j < c3; j++)
        {
            if (counts[j]->count > counts[i]->count)
            {
                temp = counts[i];
                counts[i] = counts[j];
                counts[j] = temp;
            }
        }
    }
    for (i = 0; i < set.tex_numfrq; i++)
    {
        if ((i < c3) &&
                (counts[i]->count >= 1))
        {
            m->bests[i] = counts[i];
        }
        else
        {
            m->bests[i] = NULL;
        }
    }

    delete[] counts;
}

void TTextureWindow::ClickFrequents()
{
    CalcFrequents();
    RedrawContents();
}

void TTextureWindow::CalcUsage()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    if (!m)
        return;

    int c3 = m->numMapTextures;
    if (c3 <=0)
        return;

    texInfo_t **counts = new texInfo_t *[c3];
    memset(counts,0,c3*sizeof(texInfo_t *));

    ClearCounts(m); // clear them out...

    for(int i=0; i<c3; i++)
        counts[i] = m->mapTextures[i];

    int runcount = 0;
    int total = m->numBrushes();
    char outstr[80];

    for (Entity *e = m->objects.p_next; e != &m->objects; e = e->p_next)
    {
        if (!e->modifiable)
        {
            runcount++;
            continue;
        }
        for (SetBrush *b = e->objects.p_next; b != &e->objects; b = b->p_next)
        {
            runcount++;
            if (!(runcount % 5))
            {
                sprintf(outstr,"Usage [%i/%i] %i%%...",runcount,total,(int)(100.0f*(float)runcount/(float)total));
                status->SetText(outstr,true);
            }
            for (face_t *fa = b->faces.p_next; fa != &b->faces; fa = fa->p_next)
            {
                for (int w = 0; w<c3; w++)
                {
                    if (!strnicmp(fa->texture.texture, counts[w]->bmp, LUMP_NAME_LENGTH))
                    {
                        counts[w]->count++;
                        break;
                    }
                }
            }
        }
    }

    // all counts filled in, now set up the bests array...
    for (int i = 0; i < c3; i++)
    {
        for (int j = i+1; j < c3; j++)
        {
            if (counts[j]->count > counts[i]->count)
            {
                texInfo_t *temp = counts[i];
                counts[i] = counts[j];
                counts[j] = temp;
            }
        }
    }

    BSPHelp(const_cast<char *> ("Texture List"), const_cast<char *> ("Count\tTexture\r\n"));
    for (int i = 0; i < c3; i++)
    {
        if (!counts[i]->count)   // skip zeros...
            continue;
        BSPHelpAddV(const_cast<char *> ("%i\t%s\r\n"), counts[i]->count, counts[i]->bmp);
    }
    delete[] counts;
}

void TTextureWindow::RedrawAnimated()
{
    if (!IsVisible())
        return;
    if (!set.Map_Read || !map_i || !map_i[set.curmap])
        return;
    if (!hmemdc || !hmembitmap)
        return;
    if (!hmemdcFF || !hmembitmapFF)
        return;
    if (lmousedown)
        return;

    waterFrame++;
    while (waterFrame > 16384)
        waterFrame -= 2048;

    map *m = map_i[set.curmap];

    int curcol, currow;
    int c = 0;

    RECT dst, src, tmp, rc;

    HDC hdc = GetDC(hwnd);
    HBRUSH sColorBrush = CreateSolidBrush(set.color_selectedtexture);
    HBRUSH yellowBrush = CreateSolidBrush(RGB(255,255,0));
    HBRUSH redBrush = CreateSolidBrush(RGB(255,0,0));
    texInfo_t *tx, *txOrig;

    curstartrow = Scroller.Y.nPos;

    SetStretchBltMode(hmemdc,COLORONCOLOR);
    SetStretchBltMode(hmemdcFF,COLORONCOLOR);
    SetStretchBltMode(hdc,COLORONCOLOR);

    src.left = src.top = 0;

    for (currow = 0; currow <= totrows; currow++)
    {
        for (curcol = 0; curcol < colsperrow; curcol++)
        {
            if (c >= m->numMapTextures)
                break;

            tx = m->mapTextures[c++];
            if (!tx)
                continue;

            if (!(tx->animated || tx->isSkyQ1() || tx->isWarp()))
                continue;

            if (currow < curstartrow)
                continue;

            if (currow >= (curstartrow + rowsperpage) )
                break;

            dst.left = trueW*curcol+(set.show_tex_names*TEXHSPACE)/2;
            dst.top  = trueH*currow;

            txOrig = tx;
            if (!tx->isSkyQ1() && (txOrig->animated == 1) && tx->aTextures)
            {
                tx->aCurrent++;
                if (tx->aCurrent >= tx->aCount)
                    tx->aCurrent = 0;
                tx = tx->aTextures[tx->aCurrent];
            }

            if (!tx || !tx->dibInfo)
                continue;
            // move me

            GetTextureDisplayRects(dst.left, dst.top, &src, &dst, tx, set.texture_width, set.texture_height);

            if (set.tex_animatesky && tx->isSkyQ1())
            {
                CreateSkyBits(tx->w,tx->h,tx->sky,tx->bits);
            }

            if (txOrig->isWarp() || (txOrig->animated == 2))
            {
                CreateWaterBits(tx->w, tx->h, tx->sky, tx->bits);
            }

            if (tx->dibInfo && tx->bits)
            {
                StretchDIBits(hmemdc,
                              dst.left,dst.top,dst.right-dst.left,dst.bottom-dst.top,
                              src.left,src.top,src.right-src.left,src.bottom-src.top,
                              tx->bits, tx->dibInfo,
                              DIB_RGB_COLORS, SRCCOPY);
            }

            if (txOrig->selected && !set.show_tex_names)
            {
                tmp.left = dst.left+1;
                tmp.top = dst.top+1;
                tmp.right = dst.left+4;
                tmp.bottom = dst.top+4;
                FillRect(hmemdc, &tmp, sColorBrush);
            }
            rc.left = dst.left;
            rc.top = nameoffset + trueH*(currow - curstartrow);
            rc.right = rc.left + (dst.right-dst.left);
            rc.bottom = rc.top + (dst.bottom-dst.top);
            if (tx->dibInfo && tx->bits)
            {
                StretchDIBits(hdc,
                              rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,
                              src.left,src.top,src.right-src.left,src.bottom-src.top,
                              tx->bits, tx->dibInfo,
                              DIB_RGB_COLORS, SRCCOPY);
            }

            if (txOrig->selected && !set.show_tex_names)
            {
                tmp.left = rc.left+1;
                tmp.top = rc.top+1;
                tmp.right = rc.left+4;
                tmp.bottom = rc.top+4;
                FillRect(hdc, &tmp, sColorBrush);
            }
            int mult = 1;
            if (set.texture_width >= 32 && set.texture_height >= 32)
                mult = 2;

            rc.top = rc.top + set.texture_height - 2*mult;
            rc.bottom = rc.top + mult;
            int stL = rc.left + mult;
            for (int i = 0; i < txOrig->aCount; i++)
            {
                rc.left = stL + (mult+1)*i;
                rc.right = rc.left + mult;
                if (rc.right > ((stL - mult) + set.texture_width))
                    continue;

                if (i == txOrig->aCurrent)
                    FillRect(hdc, &rc, yellowBrush);
                else
                    FillRect(hdc, &rc, redBrush);

            }
        }
    }
    ReleaseDC(hwnd,hdc);
    DeleteObject(sColorBrush);
    DeleteObject(yellowBrush);
    DeleteObject(redBrush);
}

// w, h of actual texture stuff
// current sky structure
// out should already be allocated...
void TTextureWindow::CreateSkyBits(int w, int h, sky_t *sky, void *out)
{
    int c, r;

    int destRow;
    int offset;
    int rt;
    int wt = w*2;
    unsigned char temp;

    unsigned char *o = (unsigned char *)out;
    unsigned char *i = (unsigned char *)sky->bits;

    // lay down the bottom
    for (r = 0; r < h; r++)
    {
        destRow = (r + sky->skyRow) % h;
        offset = destRow * (int)w;
        rt = wt*r;
        for (c = 0; c < w; c++)
        {
            temp = i[rt+w+c];
            o[offset+((c + sky->skyCol) % w)] = temp; // pull bottom from "right"
        }
    }
    // now overlay it...
    for (r = 0; r < h; r++)
    {
        destRow = (r + (h-sky->skyRow-1)) % h;
        offset = destRow * (int)w;
        rt = wt*r;
        for (c = 0; c < w; c++)
        {
            temp = i[rt+c];
            if (set.sinBsp)
            {
                if (temp && temp <= 240)
                    o[offset+((c + sky->skyCol) %w)] = temp; // pull top from "left"
            }
            else
            {
                if (temp)
                    o[offset+((c + sky->skyCol) %w)] = temp; // pull top from "left"
            }
        }
    }

    sky->skyRow += set.tex_skyspeed;
    sky->skyCol += set.tex_skyspeed;

    while (sky->skyRow >= h)
        sky->skyRow -= h;

    while (sky->skyCol >= w)
        sky->skyCol -= w;
}

// w, h of actual texture stuff
// current sky structure
// out should already be allocated...
void TTextureWindow::CreateWaterBits(int w, int h, sky_t *sky, void *out)
{
    unsigned char *o = (unsigned char *)out;
    unsigned char *i = (unsigned char *)sky->bits;

    int t = (waterFrame % tperiod);

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int xo = rint(sin(2*M_PI*(y/vp)) * xo_lookup[t]);
            int yo = rint(cos(2*M_PI*(x/hp)) * yo_lookup[t]);
            *o = i[((y + yo + h) % h) * w + (x + xo + w) % w];
            o++;
        }
    }
}

void TTextureWindow::FakeSave()
{
    if (!set.Map_Read)
        return;

    TMipMapTransferBuffer xfer;
    memset(&xfer,0,sizeof(xfer));

    strcpy(xfer.Name,"NAME");
    xfer.Left = 0;
    xfer.Top = 0;
    xfer.Width = 64;
    xfer.Height = 64;

    if (TMipMapDlg(hwnd,&xfer,editWindow->imagebuffer,set.render_width,set.render_height).Execute() == IDOK)
    {
        if (strlen(xfer.Name) <= 0)
            return;

        int w = xfer.Width;
        int h = xfer.Height;

        int x = xfer.Left;
        int y = xfer.Top;

        if (w % 16)
            w = 16*(int)((w+15)/16);
        if (h % 16)
            h = 16*(int)((h+15)/16);

        if (!w || !h)
            return;

        if (x + w >= set.render_width)
            x = set.render_width - w;
        if (y + h >= set.render_height)
            y = set.render_height - h;

        if (x < 0 || y < 0)
            return;

        AddToWad(currentWadPtr,xfer.Name,editWindow->imagebuffer,set.render_width,set.render_height,x,y,w,h);
    }
}

void TTextureWindow::SelectAll()
{
    if (!set.Map_Read)
        return;
    if (!map_i)
        return;

    map *m = map_i[set.curmap];
    if (!m)
        return;

    if (!m->mapTextures)
        return;

    for (int i = 0; i < m->numMapTextures; i++)
    {
        if (!m->mapTextures[i])
            continue;
        m->mapTextures[i]->selected = true;
    }
    RedrawContents();
}

void TTextureWindow::DeselectAll()
{
    if (!set.Map_Read)
        return;
    if (!map_i)
        return;

    map *m = map_i[set.curmap];
    if (!m)
        return;

    if (!m->mapTextures)
        return;

    for (int i = 0; i < m->numMapTextures; i++)
    {
        if (!m->mapTextures[i])
            continue;
        m->mapTextures[i]->selected = false;
    }
    RedrawContents();
}

// todo: what is wrong with this??: just select all for now...
void TTextureWindow::SelectUsed()
{
    if (!set.Map_Read)
        return;
    if (!map_i)
        return;

    map *m = map_i[set.curmap];
    if (!m)
        return;

    if (!m->mapTextures)
        return;

    texInfo_t **counts;
    Entity *e;
    SetBrush *b;
    int i,j,w;

    int c3 = m->numMapTextures;

    if (c3 <=0)
        return;

    DeselectAll();

    counts = new texInfo_t *[c3];
    memset(counts,0,c3*sizeof(texInfo_t *));

    ClearCounts(m); // clear them out...

    for (i = 0; i < c3; i++)
    {
        counts[i] = m->mapTextures[i];
    }

    int runcount = 0;
    int total = m->numBrushes();
    char outstr[80];
    for (e = m->objects.p_next; e != &m->objects; e = e->p_next)
    {
        if (!e->modifiable)
        {
            runcount++;
            continue;
        }
        for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
        {
            runcount++;
            if (!(runcount % 5))
            {
                sprintf(outstr,"Usage [%i/%i] %i%%...",runcount,total,(int)(100.0f*(float)runcount/(float)total));
                status->SetText(outstr,true);
            }
            face_t *fa;
            for (fa = b->faces.p_next; fa != &b->faces; fa = fa->p_next)
            {
                for (w = 0; w<c3; w++)
                {
                    if (!strnicmp(fa->texture.texture, counts[w]->bmp,LUMP_NAME_LENGTH))
                    {
                        counts[w]->count++;
                        break;
                    }
                }
            }
        }
    }

    texInfo_t *temp;
    // all counts filled in, now set up the bests array...
    for (i = 0; i < c3; i++)
    {
        for (j = i+1; j < c3; j++)
        {
            if (counts[j]->count > counts[i]->count)
            {
                temp = counts[i];
                counts[i] = counts[j];
                counts[j] = temp;
            }
        }
    }

    for (i = 0; i < c3; i++)
        counts[i]->selected = counts[i]->count;
    delete[] counts;

    RedrawContents();
}

// save selected textures to a new wad file
void TTextureWindow::SaveSelected()
{
    if (!set.Map_Read)
        return;

    int			i,j,k;
    FILE 		*wadfile;
    int			numlumps;
    uintptr_t offset;
    wadinfo_t   *wadinfo;
    lumpinfo_t *lumpinfo = NULL;
    sin_lumpinfo_t *slumpinfo = NULL;
    lumpinfo_t *curlump;
    sin_lumpinfo_t *scurlump;

    int texStart;
    texInfo_t *tx,*ax;
    int w, h;
    unsigned char *bits;
    unsigned char *curtex;

    //	unsigned char *pal;
    numlumps = NumSelected();
    if (numlumps <= 0)    // nothing to do...
        return;

    map *m = map_i[set.curmap];
    if (!m)
        return;

    char filename[256] = "";

    OPENFILENAME ofn;
    memset(&ofn,0,sizeof(OPENFILENAME));
    ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
    ofn.hwndOwner = frame->hwnd;
    ofn.lpstrFilter = "Wad Files (*.wad)\0*.wad\0";
    ofn.lpstrDefExt = "wad";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = 256;
    ofn.lpstrTitle = "Output Texture Wad";
    ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;


    if (!GetSaveFileName(&ofn))
        return;

    int addPalette = 0;
    if (!set.sinBsp)
    {
        // Only for regular bsp
        int retval = MessageBox(hwnd,"Include Palette?","BSP - Paste Texture",MB_ICONEXCLAMATION | MB_YESNO);
        addPalette = (retval == IDYES) ? 1 : 0;
    }
    numlumps += addPalette;

    // otherwise make addPalette = 0
    int size;
    int sizeAdd = 0;
    wadinfo = new wadinfo_t;
    memset(wadinfo,0,sizeof(wadinfo_t));
    if (set.sinBsp)
    {
        if (set.valveBsp)
        {
            strncpy(wadinfo->identification,"WAD3",4);
        }
        else
        {
            strncpy(wadinfo->identification,"SIN1",4);
        }
    }
    else
    {
        strncpy(wadinfo->identification,"WAD2",4);
    }
    wadinfo->numlumps = numlumps;

    if (set.sinBsp && !set.valveBsp)
    {
        slumpinfo = new sin_lumpinfo_t[numlumps]; // get the headers...
        memset(slumpinfo,0,numlumps*sizeof(sin_lumpinfo_t));
    }
    else
    {
        lumpinfo = (lumpinfo_t *)new lumpinfo_t[numlumps]; // get the headers...
        memset(lumpinfo,0,numlumps*sizeof(lumpinfo_t));
    }

    wadfile = fopen(filename,"wb");
    if (!wadfile)
    {
        MessageBox(hwnd,"Couldn't create .wad file...","BSP - Save Wad",MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    // go back to here...
    offset = 0;
    fwrite(wadinfo,sizeof(wadinfo_t),1,wadfile);
    offset += sizeof(wadinfo_t);

    j = 0;

    if (addPalette)
    {
        if (set.sinBsp && !set.valveBsp)
        {
            scurlump = &slumpinfo[j];
            scurlump->type = 0;
            strcpy(scurlump->name,"PALETTE");
            scurlump->filepos = offset;
            scurlump->compression = (char)0;
            scurlump->pad1 = (char)0;
            scurlump->pad2 = (char)0;
            scurlump->size = (int)(256*3*sizeof(unsigned char));
            scurlump->disksize = (int)(256*3*sizeof(unsigned char));
        }
        else
        {
            curlump = &lumpinfo[j];
            curlump->type = 0;
            strcpy(curlump->name,"PALETTE");
            curlump->filepos = offset;
            curlump->compression = (char)0;
            curlump->pad1 = (char)0;
            curlump->pad2 = (char)0;
            curlump->size = (int)(256*3*sizeof(unsigned char));
            curlump->disksize = (int)(256*3*sizeof(unsigned char));
        }
        fwrite(set.pal,256*3*sizeof(unsigned char),1,wadfile);

        offset += (int)(256*3*sizeof(unsigned char));
        j++;
    }

    int tsize;
    if (set.sinBsp)
    {
        if (set.valveBsp)
        {
            tsize = sizeof(valve_miptexnew_t);
            curtex = reinterpret_cast<unsigned char*>((valve_miptexnew_t *) new valve_miptexnew_t);
        }
        else
        {
            tsize = sizeof(sin_miptexnew_t);
            curtex = reinterpret_cast<unsigned char*>((sin_miptexnew_t *) new sin_miptexnew_t);
        }
    }
    else
    {
        tsize = sizeof(miptex_t);
        curtex = reinterpret_cast<unsigned char*>((miptex_t *)new miptex_t);
    }
    memset(curtex,0,tsize);

    for (i = 0; i < m->numMapTextures; i++)
    {
        tx = m->mapTextures[i];
        if (!tx)
            continue;

        if (!tx->selected)
            continue;

        w = tx->w;
        h = tx->h;

        if (set.sinBsp)
        {
            if (set.valveBsp)
            {
                ((valve_miptexnew_t *)curtex)->palette = new BYTE[PALSTEP*tx->palSize];
                memcpy(((valve_miptexnew_t *)curtex)->palette,tx->palette,tx->palSize*PALSTEP);
                ((valve_miptexnew_t *)curtex)->palSize = tx->palSize;
                sizeAdd = sizeof(WORD) + tx->palSize*PALSTEP*sizeof(BYTE);
            }
            else
            {
                memcpy(((sin_miptexnew_t *)curtex)->palette,tx->palette,256*PALSTEP);
            }
        }
        bits = (unsigned char *)tx->bits;
        if (tx->sky)
        {
            bits = (unsigned char *)tx->sky->bits;
            if (tx->texType==TEX_TYPE_Q1 && !strnicmp(tx->bmp,"sky",3))
                w*=2;
        }

        size = w * h * 85/64;

        if (set.sinBsp && !set.valveBsp)
        {
            scurlump = &slumpinfo[j];
            if (set.valveBsp)
            {
                scurlump->type = LUMPTYPE_MIPTEX;
            }
            else
            {
                scurlump->type = LUMPTYPE_MIPTEXNEW;
            }
            STRNCPY(scurlump->name,tx->bmp);
            scurlump->filepos = offset;
            scurlump->compression = (char)0;
            scurlump->pad1 = (char)0;
            scurlump->pad2 = (char)0;
            scurlump->size = (int)(size + tsize + sizeAdd);
            scurlump->disksize = (int)(size + tsize + sizeAdd);
        }
        else
        {
            curlump = &lumpinfo[j];
            curlump->type = TYP_MIPTEX;
            STRNCPY(curlump->name,tx->bmp);
            curlump->filepos = offset;
            curlump->compression = (char)0;
            curlump->pad1 = (char)0;
            curlump->pad2 = (char)0;
            curlump->size = (int)(size + tsize + sizeAdd);
            curlump->disksize = (int)(size + tsize + sizeAdd);
        }

        texStart = tsize;
        if (set.sinBsp)
        {
            if (set.valveBsp)
            {
                STRNCPY(((valve_miptexnew_t *)curtex)->name,tx->bmp);
                ((valve_miptexnew_t *)curtex)->width  = w;
                ((valve_miptexnew_t *)curtex)->height = h;
                ((valve_miptexnew_t *)curtex)->offsets[0] = texStart;
                ((valve_miptexnew_t *)curtex)->offsets[1] = texStart + w * h;
                ((valve_miptexnew_t *)curtex)->offsets[2] = texStart + w * h + (w * h)/4;
                ((valve_miptexnew_t *)curtex)->offsets[3] = texStart + w * h + (w * h)/4 + (w * h)/16;
                fwrite(((valve_miptexnew_t *)curtex),tsize,1,wadfile);
            }
            else
            {
                STRNCPY(((sin_miptexnew_t *)curtex)->name,tx->bmp);
                ((sin_miptexnew_t *)curtex)->width  = w;
                ((sin_miptexnew_t *)curtex)->height = h;
                ((sin_miptexnew_t *)curtex)->offsets[0] = texStart;
                ((sin_miptexnew_t *)curtex)->offsets[1] = texStart + w * h;
                ((sin_miptexnew_t *)curtex)->offsets[2] = texStart + w * h + (w * h)/4;
                ((sin_miptexnew_t *)curtex)->offsets[3] = texStart + w * h + (w * h)/4 + (w * h)/16;
                fwrite(((sin_miptexnew_t *)curtex),tsize,1,wadfile);
            }
        }
        else
        {
            STRNCPY(((miptex_t *)curtex)->name,tx->bmp);
            ((miptex_t *)curtex)->width  = w;
            ((miptex_t *)curtex)->height = h;
            ((miptex_t *)curtex)->offsets[0] = texStart;
            ((miptex_t *)curtex)->offsets[1] = texStart + w * h;
            ((miptex_t *)curtex)->offsets[2] = texStart + w * h + (w * h)/4;
            ((miptex_t *)curtex)->offsets[3] = texStart + w * h + (w * h)/4 + (w * h)/16;
            fwrite(((miptex_t *)curtex),tsize,1,wadfile);
        }
        fwrite(bits,sizeof(unsigned char),size,wadfile);
        if (set.valveBsp)
        {
            fwrite(&((valve_miptexnew_t *)curtex)->palSize,sizeof(WORD),1,wadfile);
            fwrite(((valve_miptexnew_t *)curtex)->palette,PALSTEP*sizeof(BYTE),((valve_miptexnew_t *)curtex)->palSize,wadfile);
            delete[] ((valve_miptexnew_t *)curtex)->palette;
        }
        offset += sizeof(unsigned char)*size + tsize + sizeAdd;

        j++;

        if (tx->animated == 1)
        {
            for (k = 1; k < tx->aCount; k++)
            {
                ax = tx->aTextures[k];
                if (!ax)
                    continue;

                w = ax->w;
                h = ax->h;

                bits = (unsigned char *)ax->bits;
                size = w * h * 85/64;

                if (set.sinBsp && !set.valveBsp)
                {
                    scurlump = &slumpinfo[j];
                    if (set.valveBsp)
                    {
                        scurlump->type = LUMPTYPE_MIPTEX;
                    }
                    else
                    {
                        scurlump->type = LUMPTYPE_MIPTEXNEW;
                    }
                    STRNCPY(scurlump->name,tx->bmp);
                    scurlump->filepos = offset;
                    scurlump->compression = (char)0;
                    scurlump->pad1 = (char)0;
                    scurlump->pad2 = (char)0;
                    scurlump->size = (int)(size + tsize + sizeAdd);
                    scurlump->disksize = (int)(size + tsize + sizeAdd);
                }
                else
                {
                    curlump = &lumpinfo[j];
                    curlump->type = TYP_MIPTEX;
                    STRNCPY(curlump->name,tx->bmp);
                    curlump->filepos = offset;
                    curlump->compression = (char)0;
                    curlump->pad1 = (char)0;
                    curlump->pad2 = (char)0;
                    curlump->size = (int)(size + tsize + sizeAdd);
                    curlump->disksize = (int)(size + tsize + sizeAdd);
                }

                if (set.sinBsp)
                {
                    if (set.valveBsp)
                    {
                        ((valve_miptexnew_t *)curtex)->palette = new BYTE[PALSTEP*tx->palSize];
                        memcpy(((valve_miptexnew_t *)curtex)->palette,tx->palette,tx->palSize*PALSTEP);
                        ((valve_miptexnew_t *)curtex)->palSize = tx->palSize;
                        sizeAdd = sizeof(WORD) + tx->palSize*PALSTEP*sizeof(BYTE);
                    }
                    else
                    {
                        memcpy(((sin_miptexnew_t *)curtex)->palette,tx->palette,256*PALSTEP);
                    }
                }

                texStart = tsize;

                if (set.sinBsp)
                {
                    if (set.valveBsp)
                    {
                        STRNCPY(((valve_miptexnew_t *)curtex)->name,tx->bmp);
                        ((valve_miptexnew_t *)curtex)->width  = w;
                        ((valve_miptexnew_t *)curtex)->height = h;
                        ((valve_miptexnew_t *)curtex)->offsets[0] = texStart;
                        ((valve_miptexnew_t *)curtex)->offsets[1] = texStart + w * h;
                        ((valve_miptexnew_t *)curtex)->offsets[2] = texStart + w * h + (w * h)/4;
                        ((valve_miptexnew_t *)curtex)->offsets[3] = texStart + w * h + (w * h)/4 + (w * h)/16;
                        fwrite(((valve_miptexnew_t *)curtex),tsize,1,wadfile);
                    }
                    else
                    {
                        STRNCPY(((sin_miptexnew_t *)curtex)->name,tx->bmp);
                        ((sin_miptexnew_t *)curtex)->width  = w;
                        ((sin_miptexnew_t *)curtex)->height = h;
                        ((sin_miptexnew_t *)curtex)->offsets[0] = texStart;
                        ((sin_miptexnew_t *)curtex)->offsets[1] = texStart + w * h;
                        ((sin_miptexnew_t *)curtex)->offsets[2] = texStart + w * h + (w * h)/4;
                        ((sin_miptexnew_t *)curtex)->offsets[3] = texStart + w * h + (w * h)/4 + (w * h)/16;
                        fwrite(((sin_miptexnew_t *)curtex),tsize,1,wadfile);
                    }
                }
                else
                {
                    STRNCPY(((miptex_t *)curtex)->name,tx->bmp);
                    ((miptex_t *)curtex)->width  = w;
                    ((miptex_t *)curtex)->height = h;
                    ((miptex_t *)curtex)->offsets[0] = texStart;
                    ((miptex_t *)curtex)->offsets[1] = texStart + w * h;
                    ((miptex_t *)curtex)->offsets[2] = texStart + w * h + (w * h)/4;
                    ((miptex_t *)curtex)->offsets[3] = texStart + w * h + (w * h)/4 + (w * h)/16;
                    fwrite(((miptex_t *)curtex),tsize,1,wadfile);
                }
                fwrite(bits,sizeof(unsigned char),size,wadfile);
                if (set.valveBsp)
                {
                    fwrite(&((valve_miptexnew_t *)curtex)->palSize,sizeof(WORD),1,wadfile);
                    fwrite(((valve_miptexnew_t *)curtex)->palette,PALSTEP*sizeof(BYTE),((valve_miptexnew_t *)curtex)->palSize,wadfile);
                    delete[] ((valve_miptexnew_t *)curtex)->palette;
                }
                offset += sizeof(unsigned char)*size + tsize + sizeAdd;

                j++;
            }
        }
    }
    // info table is at this spot...
    if (set.sinBsp && !set.valveBsp)
    {
        fwrite(slumpinfo,sizeof(sin_lumpinfo_t),numlumps,wadfile);
    }
    else
    {
        fwrite(lumpinfo,sizeof(lumpinfo_t),numlumps,wadfile);
    }
    wadinfo->infotableofs = offset;

    // Jump back to start and put the real info in...
    fseek(wadfile,0L,SEEK_SET);
    fwrite(wadinfo,sizeof(wadinfo_t),1,wadfile);
    fclose(wadfile);

    delete curtex;
    delete wadinfo;
    if (set.sinBsp && !set.valveBsp)
        delete[] slumpinfo;
    else
        delete[] lumpinfo;
}

int TTextureWindow::NumSelected()
{
    if (!set.Map_Read)
        return 0;
    if (!map_i)
        return 0;

    map* m = map_i[set.curmap];

    if (!m || !m->mapTextures)
        return 0;

    int count = 0;

    for (int i = 0; i < m->numMapTextures; i++)
    {
        if (!m->mapTextures[i])
            continue;
        if (!m->mapTextures[i]->selected)
            continue;

        if (m->mapTextures[i]->animated == 1)
            count+= m->mapTextures[i]->aCount;
        else
            count++;
    }
    return count;
}

// create a miptex_t...
void TTextureWindow::AddToWad(texWad *toWad, char *name, unsigned char *image,
                              int rw, int /*rh*/, int xoffset, int yoffset, int ix, int iy)
{
    if(!set.Map_Read)
        return;

    int			    i,j;
    unsigned char   *curtex;
    char            outstr[128];
    texInfo_t       *newTex;
    texInfo_t       *currentHead;
    char            tempNm[LUMP_NAME_LENGTH_MAX];

    newTex = new texInfo_t;
    strncpy(tempNm,name,LUMP_NAME_LENGTH);
    strncpy(newTex->bmp,tempNm,sizeof(newTex->bmp));
    strupr(newTex->bmp); // upper case it!
    newTex->w = ix;
    newTex->h = iy;

    //	unsigned char *pal;
    int size;
    unsigned char *truebits;

    int tsize;
    if (set.sinBsp)
    {
        if (set.valveBsp)
        {
            tsize = sizeof(valve_miptexnew_t);
            curtex = reinterpret_cast<unsigned char*>(new valve_miptexnew_t);
            memset(curtex,0,tsize);
            ((valve_miptexnew_t *)curtex)->palSize = 256;
            ((valve_miptexnew_t *)curtex)->palette = new BYTE[256*PALSTEP];
            memcpy(((valve_miptexnew_t *)curtex)->palette,set.pal,256*PALSTEP);
        }
        else
        {
            tsize = sizeof(sin_miptexnew_t);
            curtex = reinterpret_cast<unsigned char*>(new sin_miptexnew_t);
            memset(curtex,0,tsize);
            memcpy(((sin_miptexnew_t *)curtex)->palette,set.pal,256*PALSTEP);
        }
    }
    else
    {
        tsize = sizeof(miptex_t);
        curtex = reinterpret_cast<unsigned char*>(new miptex_t);
        memset(curtex,0,tsize);
    }

    if (set.sinBsp)
    {
        if (set.valveBsp)
        {
            newTex->palSize = 256;
            STRNCPY(((valve_miptexnew_t *)curtex)->name,name);
            ((valve_miptexnew_t *)curtex)->width  = ix;
            ((valve_miptexnew_t *)curtex)->height = iy;
        }
        else
        {
            newTex->palette = new BYTE[256*PALSTEP+3];
            memcpy(newTex->palette,((sin_miptexnew_t *)curtex)->palette,256*PALSTEP);
            STRNCPY(((sin_miptexnew_t *)curtex)->name,name);
            ((sin_miptexnew_t *)curtex)->width  = ix;
            ((sin_miptexnew_t *)curtex)->height = iy;
        }
    }
    else
    {
        STRNCPY(((miptex_t *)curtex)->name,name);
        ((miptex_t *)curtex)->width  = ix;
        ((miptex_t *)curtex)->height = iy;
    }

    // four offsets...

    int ts = ix * iy;
    size = ts + ts/4 + ts/16 + ts/64;

    truebits = new unsigned char [size + ix + 1];

    /////////////////
    uintptr_t offsets[4];
    offsets[0] = 0;
    offsets[1] = ts;
    offsets[2] = offsets[1] + ts/4;
    offsets[3] = offsets[2] + ts/16;

    int delta;
    int miph, mipw;
    int x, y;

    unsigned char *holdStart = truebits;
    mipw = ix;
    miph = iy;
    if (set.sinBsp)
    {
        unsigned char pixR, pixG, pixB;
        for (i = 0; i < miph; i++)
        {
            for (j = 0; j < mipw; j++)
            {
                pixR = image[3L*((i+yoffset)*rw + j + xoffset)+2];
                pixG = image[3L*((i+yoffset)*rw + j + xoffset)+1];
                pixB = image[3L*((i+yoffset)*rw + j + xoffset)+0];
                truebits[i*mipw + j] = BestColor(set.pal,
                                                 pixR,pixG,pixB,0,240);
            }
        }
    }
    else
    {
        // quantize to good palette...
        for (i = 0; i < miph; i++)
        {
            for (j = 0; j < mipw; j++)
            {
                truebits[i*mipw + j] = image[((i+yoffset)*rw + j + xoffset)];
            }
        }
    }

    int count;
    d_red = d_green = d_blue = 0;
    for (int miplevel = 1; miplevel < 4; miplevel++)
    {
        delta = 1 << miplevel;
        truebits = (unsigned char *)(holdStart + offsets[miplevel]);
        for (i = 0; i < miph; i+=delta)
        {
            for (j = 0; j < mipw; j+=delta)
            {
                count = 0;
                for (y = 0; y < delta; y++)
                {
                    for (x = 0; x < delta; x++)
                    {
                        pixdata[count] = holdStart[(y + i) * mipw + x + j];
                        count++;
                    }
                }
                *truebits++ = AveragePixels(count);
            }
        }
    }

    int texStart = tsize;
    if (set.sinBsp)
    {
        if (set.valveBsp)
        {
            for (i = 0; i < 4; i++)
            {
                ((valve_miptexnew_t *)curtex)->offsets[i] = offsets[i] + texStart;
            }
        }
        else
        {
            for (i = 0; i < 4; i++)
            {
                ((sin_miptexnew_t *)curtex)->offsets[i] = offsets[i] + texStart;
            }
        }
    }
    else
    {
        for (i = 0; i < 4; i++)
        {
            ((miptex_t *)curtex)->offsets[i] = offsets[i] + texStart;
        }
    }

    newTex->bits = holdStart;
    unsigned char gammatable[256];
    float	gamma;

    gamma = set.gamma;
    if (gamma == 1.0f)
    {
        for (i=0 ; i<256 ; i++)
            gammatable[i] = (unsigned char)i;
    }
    else
    {
        int inf;
        for (i=0 ; i<256 ; i++)
        {
            inf = 255 * (float)pow ( (float) ((i+0.5f)/255.5f) , (float) gamma ) + 0.5f;
            inf = min(255,max(0,inf));
            gammatable[i] = (unsigned char)inf;
        }
    }

    if (set.sinBsp)
    {
        if (set.valveBsp)
        {
            for (int palc = 0; palc < ((valve_miptexnew_t *)curtex)->palSize*3; palc++)
                ((valve_miptexnew_t *)curtex)->palette[palc] = gammatable[((valve_miptexnew_t *)curtex)->palette[palc]];
            newTex->flat = TranslateColors(((valve_miptexnew_t *)curtex)->palSize,((valve_miptexnew_t *)curtex)->palette,truebits,size);
        }
        else
        {
            for (int palc = 0; palc < 256*3; palc++)
                ((sin_miptexnew_t *)curtex)->palette[palc] = gammatable[((sin_miptexnew_t *)curtex)->palette[palc]];
            newTex->flat = TranslateColors(256,((sin_miptexnew_t *)curtex)->palette,truebits,size);
        }
    }
    else
    {
        newTex->flat = TranslateColors(256,set.pal,truebits,size);
    }

    if (newTex->isSkyQ1())
    {
        // cut down the heights...
        newTex->w /= 2;
        ix /= 2;
        newTex->sky = new sky_t;
        newTex->sky->skyRow = 0;
        newTex->sky->skyCol = 0;
        newTex->sky->bits = newTex->bits;
        size = (uintptr_t)ix * (uintptr_t)iy;
        size = size * 85/64;
        newTex->bits = (unsigned char*)new char[size+3];
        CreateSkyBits(newTex->w,newTex->h,newTex->sky,newTex->bits);
    }
    else if (newTex->isWarp())
    {
        newTex->sky = new sky_t;
        newTex->sky->skyRow = 0;
        newTex->sky->skyCol = 0;
        newTex->sky->bits = newTex->bits;
        size = (uintptr_t)ix * (uintptr_t)iy;
        size = size * 85/64;
        newTex->bits = (unsigned char*)new char[size+3];
        CreateWaterBits(newTex->w,newTex->h,newTex->sky,newTex->bits);
    }

    unsigned char *thePal;
    if (set.sinBsp)
    {
        if (set.valveBsp)
        {
            thePal = (unsigned char *)((valve_miptexnew_t *)curtex)->palette;
        }
        else
        {
            thePal = (unsigned char *)((sin_miptexnew_t *)curtex)->palette;
        }
    }
    else
    {
        thePal = (unsigned char *)set.pal;
    }
    if (set.glBsp)
        CalcGLBits(newTex,thePal);

    BITMAPINFO *m_pBmInfo;
    BITMAPINFOHEADER m_pBmInfoHeader;

    memset(&m_pBmInfoHeader,0,sizeof(BITMAPINFOHEADER));
    m_pBmInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
    m_pBmInfoHeader.biWidth = ix;
    m_pBmInfoHeader.biHeight = -iy;
    m_pBmInfoHeader.biPlanes = 1;
    m_pBmInfoHeader.biBitCount = 8;
    m_pBmInfoHeader.biCompression = BI_RGB;
    m_pBmInfoHeader.biSizeImage = ix * iy;
    m_pBmInfoHeader.biClrUsed = 256;
    m_pBmInfo = (BITMAPINFO*)new char[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];

    memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));

    if (set.sinBsp)
    {
        if (set.valveBsp)
        {
            for (int hhh = 0; hhh < 256; hhh++)
            {
                m_pBmInfo->bmiColors[hhh].rgbRed   = ((valve_miptexnew_t *)curtex)->palette[PALSTEP*hhh+0];
                m_pBmInfo->bmiColors[hhh].rgbGreen = ((valve_miptexnew_t *)curtex)->palette[PALSTEP*hhh+1];
                m_pBmInfo->bmiColors[hhh].rgbBlue  = ((valve_miptexnew_t *)curtex)->palette[PALSTEP*hhh+2];
                m_pBmInfo->bmiColors[hhh].rgbReserved = 0;
            }
        }
        else
        {
            for (int hhh = 0; hhh < 256; hhh++)
            {
                m_pBmInfo->bmiColors[hhh].rgbRed   = ((sin_miptexnew_t *)curtex)->palette[PALSTEP*hhh+0];
                m_pBmInfo->bmiColors[hhh].rgbGreen = ((sin_miptexnew_t *)curtex)->palette[PALSTEP*hhh+1];
                m_pBmInfo->bmiColors[hhh].rgbBlue  = ((sin_miptexnew_t *)curtex)->palette[PALSTEP*hhh+2];
                m_pBmInfo->bmiColors[hhh].rgbReserved = 0;
            }
        }
    }
    else
    {
        memcpy(m_pBmInfo->bmiColors,set.cTable,256*sizeof(RGBQUAD));
    }
    newTex->dibInfo = m_pBmInfo;
    newTex->truedibInfo = NULL; // tm_pBmInfo;

    if (set.valveBsp)
    {
        delete[] ((valve_miptexnew_t *)curtex)->palette;
    }
    delete curtex;

    sprintf(outstr,"Inserting [%s]",name);
    status->SetText(outstr,true);
    currentHead = toWad->textures->next; // first one

    int done = 0;

    while (currentHead && !done)
    {
        if (currentHead->next)
        {
            if (QCompare(currentHead->bmp,
                         newTex->bmp) > 0)
            {
                newTex->next = currentHead;
                newTex->previous = currentHead->previous;
                newTex->previous->next = newTex;
                newTex->next->previous = newTex;
                toWad->bmpsCount++;
                done = 1;
            }
        }
        else
        {
            // insert before this one...
            newTex->next = currentHead;
            newTex->previous = currentHead->previous;
            newTex->previous->next = newTex;
            newTex->next->previous = newTex;
            toWad->bmpsCount++;
            done = 1;
        }
        currentHead = currentHead->next;
    }

    map *m = map_i[set.curmap];
    SetUpMap(m);
    ChangeToMap(m);

    int idx = GetTextureIndex(m,newTex);
    if (idx >=0 && idx < m->numMapTextures)
    {
        strncpy(m->texName,newTex->bmp,sizeof(m->texName));
        if (set.game_mode == 2)
            strncpy(m->texPath,newTex->basepath,sizeof(m->texPath));
        texList->SetText(m->texName);
        NewCurrentTexture(newTex);
        if (frame->TextureList)
            frame->TextureList->SetSelIndex(idx);

        Select(idx,true);
    }
    RedrawContents();
}

//copy texture to clipboard - copy is 24bit
void TTextureWindow::CopyToClipboard()
{
    if (!set.Map_Read)
        return;
    if (!map_i)
        return;

    map *m = map_i[set.curmap];
    if (!m || !m->mapTextures)
        return;

    texInfo_t *tx = rightClickedTexture;
    if (!tx)
        return;

    int height = tx->h;
    int width = tx->w;

    if (tx->isSkyQ1())
        width*=2;

    int i,j;

    HGLOBAL gmem = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,
                               sizeof(BITMAPINFOHEADER) + height*width*3 + sizeof(int*));

    BYTE *bitmap = (BYTE*)GlobalLock(gmem);

    BITMAPINFOHEADER *bmi = (LPBITMAPINFOHEADER)bitmap;
    memset(bmi,0,sizeof(BITMAPINFOHEADER));
    bmi->biSize = sizeof(BITMAPINFOHEADER);
    bmi->biWidth = width;
    bmi->biHeight = height;
    bmi->biPlanes = 1;
    bmi->biBitCount = 24;

    BYTE *bits = bitmap + sizeof(int*)*0 + sizeof(BITMAPINFOHEADER);	//bmicolors;

    //unsigned char *dest;
    unsigned char *src;
//  dest = (unsigned char *)dib->GetBits();
// FIX xxx for Sin/Valvue use of truebits...
    if (tx->sky)
    {
        src = (unsigned char *)tx->sky->bits;
    }
    else
    {
        src = (unsigned char *)tx->bits;
    }
    if(!set.sinBsp)
    {
        //copy from 8bit image
        for (i = height-1; i >= 0; i--)
        {
            for (j = 0; j < width; j++)
            {

                int d = (i*width + j) * 3;								// dest pixel offset
                int s = PALSTEP * src[((height - 1) - i)*width + j];	// source pal index

                bits[d++] = set.pal[s+2];
                bits[d++] = set.pal[s+1];
                bits[d  ] = set.pal[s+0];
            }
        }
    }
    else
    {
        //copy sin 24bit - untested
        for (i = height-1; i >= 0; i--)
        {
            for (j = 0; j < width; j++)
            {

                int d = (i*width + j) * 3;								// dest pixel offset
                unsigned char *s = &src[3 * (((height - 1) - i)*width + j)];

                bits[d++] = s[2];
                bits[d++] = s[1];
                bits[d  ] = s[0];
            }
        }
    }

    GlobalUnlock(gmem);

    OpenClipboard(hwnd);
    EmptyClipboard();
    SetClipboardData(CF_DIB, (HANDLE)bitmap);
    CloseClipboard();
}

// copy current 3d image portion to clipboard...
void TTextureWindow::CopyImageClipboard()
{
    if (!set.Map_Read)
        return;
    if (!map_i)
        return;

    TMipMapTransferBuffer xfer;
    memset(&xfer,0,sizeof(xfer));

    strcpy(xfer.Name,"NAME");	//not used
    xfer.Left = 0;
    xfer.Top = 0;
    xfer.Width = 64;
    xfer.Height = 64;

    TMipMapDlg dialog(hwnd,&xfer,editWindow->imagebuffer,set.render_width,set.render_height);
    if (dialog.Execute() != IDOK)
        return;

    int width = xfer.Width;
    int height = xfer.Height;
    int x = xfer.Left;
    int y = xfer.Top;

    if (width % 16)		width = 16*(int)((width+15)/16);
    if (height % 16)	height = 16*(int)((height+15)/16);
    if (width<=0 || height<=0)
        return;

    if (x + width >= set.render_width)	x = set.render_width - width;
    if (y + height >= set.render_height)	y = set.render_height - height;
    if (x < 0 || y < 0)
        return;

    HGLOBAL gmem = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, sizeof(BITMAPINFOHEADER) + height*width*3);

    BYTE *bitmap = (BYTE*)GlobalLock(gmem);

    BITMAPINFOHEADER *bmi = (BITMAPINFOHEADER*)bitmap;
    memset(bmi,0,sizeof(BITMAPINFOHEADER));
    bmi->biSize = sizeof(BITMAPINFOHEADER);
    bmi->biWidth = width;
    bmi->biHeight = height;
    bmi->biPlanes = 1;
    bmi->biBitCount = 24;

    BYTE *bits = bitmap + sizeof(BITMAPINFOHEADER);

    if(!set.sinBsp)
    {
        //8bit source
        for(int j=0; j<height; j++)
        {
            for(int i=0; i<width; i++)
            {
                int d = 3*((width*height-1)-(j*width+width-i));						// dest pixel offset
                int s = PALSTEP * editWindow->imagebuffer[(y+j)*set.render_width +x+i];	// source rgb triplet offset
                bits[d+0] = set.pal[s+2];
                bits[d+1] = set.pal[s+1];
                bits[d+2] = set.pal[s+0];
            }
        }
    }
    else
    {
        //24bit source - untested
        for(int j=0; j<height; j++)
        {
            for(int i=0; i<width; i++)
            {
                int d = 3*((width*height-1)-(j*width+width-i));						// dest pixel offset
                unsigned char *s = &editWindow->imagebuffer[3*((y+j)*set.render_width +x+i)];
                bits[d+0] = s[2];
                bits[d+1] = s[1];
                bits[d+2] = s[0];
            }
        }
    }

    GlobalUnlock(gmem);

    OpenClipboard(hwnd);
    EmptyClipboard();
    SetClipboardData(CF_DIB, (HANDLE)bitmap);
    CloseClipboard();
}


void TTextureWindow::PasteFromClipboard()
{
    if (!set.Map_Read || !map_i)
        return;

    map *m = map_i[set.curmap];
    if (!m || !m->mapTextures)
        return;

    // prompt whether to replace current, or to create a new miptex...
    if (!IsClipboardFormatAvailable(CF_DIB))
    {
        MessageBox(hwnd, const_cast<char *> ("Must have a DIB in clipboard..."), const_cast<char *> ("BSP - Paste Texture"), MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    char outstr[128];
    char s[LUMP_NAME_LENGTH_MAX] = "texture";

    if (IDOK != InputDialog(hwnd, const_cast<char *> ("BSP - Paste Texture"), const_cast<char *> ("New Texture Name ? "), s, LUMP_NAME_LENGTH).Execute())
        return;

    if (strlen(s) <= 0)
        return;

    int i,j;

    // Need a function to "delete" all qtexture_t with a name...
    // Need a function to see if a name is already in use

    // for now, just create a new texture...
    OpenClipboard(hwnd);
    HANDLE bitmap = GetClipboardData(CF_DIB);
    BITMAPINFOHEADER *bmi = (LPBITMAPINFOHEADER)bitmap;

    if(bmi->biCompression != BI_RGB)
    {
        MessageBox(hwnd, const_cast<char *> ("Invalid bitmap format (compressed)"), const_cast<char *> ("BSP - Paste Texture"), MB_OK | MB_ICONEXCLAMATION);
        CloseClipboard();
        return;
    }

    int w = bmi->biWidth;
    int h = bmi->biHeight;

    if(h < 0)
    {
        MessageBox(hwnd, const_cast<char *> ("Can't paste, negative bitmap height is unsupported"), const_cast<char *> ("BSP - Paste Texture"), MB_OK | MB_ICONEXCLAMATION);
        CloseClipboard();
        return;
    }

    if ((w % 16) || (h % 16))
    {
        sprintf(outstr, "Bitmap must have dimensions that are multiples of 16 [%i x %i]...", w, h);

        MessageBox(hwnd,outstr,"BSP - Paste Texture",MB_OK | MB_ICONEXCLAMATION);
        CloseClipboard();
        return;
    }

    BYTE* thedib = new BYTE[sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*256+w*h];

    //set bmi
    BITMAPINFOHEADER *bi = (BITMAPINFOHEADER*)thedib;
    bi->biSize = sizeof(BITMAPINFOHEADER);
    bi->biWidth = w;
    bi->biHeight = h;
    bi->biPlanes = 1;
    bi->biBitCount = 8;	//converting to pal
    bi->biCompression = BI_RGB;
    bi->biSizeImage = w*h;
    bi->biXPelsPerMeter = 0;
    bi->biYPelsPerMeter = 0;
    bi->biClrUsed = 256;
    bi->biClrImportant = 256;

    //set dibs palette
    BYTE* temppal = thedib + sizeof(BITMAPINFOHEADER);
    for(i=0; i<256*3; i++)
    {
        *temppal++ = set.pal[i];
        *temppal++ = set.pal[i+1];
        *temppal++ = set.pal[i+2];
        *temppal++ = 0;
    }

    //this code is slow and crappy, but it works and this code isnt speed-critical..
    //set pixels
    int temppxcnt = abs(w*h);
    BYTE* temppx = thedib + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256;
    int oldpalsize = (bmi->biBitCount <= 8 ? (bmi->biClrUsed ? bmi->biClrUsed : (1<<bmi->biBitCount)): 0)  * sizeof(RGBQUAD);  // HAHAHA who let me write c++
    BYTE* oldpx = (BYTE*)bitmap + sizeof(BITMAPINFOHEADER) + oldpalsize;
    BYTE* oldpal = (BYTE*)bitmap + sizeof(BITMAPINFOHEADER);
    for(i=0; i<temppxcnt; i++)
    {
        if(bmi->biBitCount == 4)
        {
            //16 color
            int p = ((*oldpx>>4) & 0xF) * sizeof(RGBQUAD);
            *temppx++ = GetPaletteIndex(256,set.pal, RGB(oldpal[p+2],oldpal[p+1],oldpal[p+0]));
            p = (*oldpx & 0xF) * sizeof(RGBQUAD);
            *temppx++ = GetPaletteIndex(256,set.pal,RGB(oldpal[p+2],oldpal[p+1],oldpal[p+0]));
            oldpx++;
            i++; //two pixels per byte
        }
        else if(bmi->biBitCount == 8)
        {
            //256
            int p = *oldpx * sizeof(RGBQUAD);
            *temppx++ = GetPaletteIndex(256,set.pal,RGB(oldpal[p+2],oldpal[p+1],oldpal[p+0]));
            oldpx++;
        }
        else if(bmi->biBitCount == 24)
        {
            //24bit
            *temppx++ = GetPaletteIndex(256,set.pal,RGB(oldpx[2],oldpx[1],oldpx[0]));
            oldpx += 3;
        }
        else if(bmi->biBitCount == 32)
        {
            //32bit
            *temppx++ = GetPaletteIndex(256,set.pal, *(COLORREF*)oldpx);
            oldpx += 4;
        }
        else
        {
            delete[] thedib;
            CloseClipboard();
            Msgbox(const_cast<char *> ("unknown bmp format! (%d bits)"), bmi->biBitCount);
            return;
        }
    }

    CloseClipboard();

    BYTE *bits = thedib + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256;

//TODO: determine if dib is paletted or not. if paletted, use the pal. if not, convert each pixel to best match
// with that GetPal whatever function. dunno what to do about sin. !!!

    // create a dib, make it paletized
    // compare palettes, barf if != to current palette
    // create mips etc etc.

    int tsize;
    unsigned char *curtex;
    if (set.sinBsp)
    {
        if (set.valveBsp)
        {
            tsize = sizeof(valve_miptexnew_t);
            curtex = reinterpret_cast<unsigned char*>((valve_miptexnew_t *) new valve_miptexnew_t);
            memset(((valve_miptexnew_t *)curtex),0,tsize);
            ((valve_miptexnew_t *)curtex)->palette = new BYTE[256*PALSTEP];
            ((valve_miptexnew_t *)curtex)->palSize = 256;
        }
        else
        {
            tsize = sizeof(sin_miptexnew_t);
            curtex = reinterpret_cast<unsigned char*>((sin_miptexnew_t *) new sin_miptexnew_t);
            memset(((sin_miptexnew_t *)curtex),0,tsize);
        }

        RGBQUAD *palPtr = (RGBQUAD *) (BYTE*)thedib + sizeof(BITMAPINFOHEADER) ;//dib.GetColors();

        if (set.sinBsp)
        {
            if (set.valveBsp)
            {
                for (i = 0; i < 256; i++)
                {
                    ((valve_miptexnew_t *)curtex)->palette[PALSTEP*i + 0] = palPtr[i].rgbRed;
                    ((valve_miptexnew_t *)curtex)->palette[PALSTEP*i + 1] = palPtr[i].rgbGreen;
                    ((valve_miptexnew_t *)curtex)->palette[PALSTEP*i + 2] = palPtr[i].rgbBlue;
                }
            }
            else
            {
                for (i = 0; i < 256; i++)
                {
                    ((sin_miptexnew_t *)curtex)->palette[PALSTEP*i + 0] = palPtr[i].rgbRed;
                    ((sin_miptexnew_t *)curtex)->palette[PALSTEP*i + 1] = palPtr[i].rgbGreen;
                    ((sin_miptexnew_t *)curtex)->palette[PALSTEP*i + 2] = palPtr[i].rgbBlue;
                    ((sin_miptexnew_t *)curtex)->palette[PALSTEP*i + 3] = 255;
                }
            }
        }
    }
    else
    {
        tsize = sizeof(miptex_t);
        curtex = reinterpret_cast<unsigned char*>((miptex_t *) new miptex_t);
        memset(curtex,0,tsize);
    }

    texInfo_t *newTex = new texInfo_t;
    texInfo_t *currentHead;

    char tempNm[LUMP_NAME_LENGTH_MAX];

    strncpy(tempNm,s,LUMP_NAME_LENGTH);
    strncpy(newTex->bmp,tempNm,sizeof(newTex->bmp));
    strupr(newTex->bmp);					// upper case it!

    // error check w and h...
    newTex->w = w;
    newTex->h = h;

    if (set.sinBsp)
    {
        if (set.valveBsp)
        {
            newTex->palSize = 256;
            strncpy(((valve_miptexnew_t *)curtex)->name,s, sizeof(((valve_miptexnew_t *)curtex)->name));
            ((valve_miptexnew_t *)curtex)->width  = w;
            ((valve_miptexnew_t *)curtex)->height = h;
        }
        else
        {
            newTex->palette = new BYTE[256*PALSTEP+3];
            memcpy(newTex->palette,((sin_miptexnew_t *)curtex)->palette,256*PALSTEP);
            strncpy(((sin_miptexnew_t *)curtex)->name,s, sizeof(((sin_miptexnew_t *)curtex)->name));
            ((sin_miptexnew_t *)curtex)->width  = w;
            ((sin_miptexnew_t *)curtex)->height = h;
        }
    }
    else
    {
        strncpy(((miptex_t *)curtex)->name, s, sizeof(((miptex_t *)curtex)->name));
        ((miptex_t *)curtex)->width  = w;
        ((miptex_t *)curtex)->height = h;
    }

    // four offsets...

    int ts = w * h;
    int size = ts + ts/4 + ts/16 + ts/64;

    unsigned char *truebits = new unsigned char[size + w];

    /////////////////
    uintptr_t offsets[4];
    offsets[0] = 0;
    offsets[1] = ts;
    offsets[2] = offsets[1] + ts/4;
    offsets[3] = offsets[2] + ts/16;


    unsigned char *image = (unsigned char *) bits ;//dib.GetBits();
    unsigned char *holdStart = truebits;
    int mipw = w;
    int miph = h;
    for (i = 0; i < miph; i++)
    {
        for (j = 0; j < mipw; j++)
        {
            truebits[i*mipw + j] = image[((miph - 1) - i)*mipw + j];
        }
    }

    d_red = d_green = d_blue = 0;
    for (int miplevel = 1; miplevel < 4; miplevel++)
    {
        int delta = 1 << miplevel;
        truebits = (unsigned char *)(holdStart + offsets[miplevel]);
        for (i = 0; i < miph; i+=delta)
        {
            for (j = 0; j < mipw; j+=delta)
            {
                int count = 0;
                for (int y = 0; y < delta; y++)
                {
                    for (int x = 0; x < delta; x++)
                    {
                        pixdata[count] = holdStart[(y + i) * mipw + x + j];
                        count++;
                    }
                }
                *truebits++ = AveragePixels(count);
            }
        }
    }

    int texStart = tsize;
    if (set.sinBsp)
    {
        if (set.valveBsp)
        {
            for (i = 0; i < 4; i++)
                ((valve_miptexnew_t *)curtex)->offsets[i] = offsets[i] + texStart;
        }
        else
        {
            for (i = 0; i < 4; i++)
                ((sin_miptexnew_t *)curtex)->offsets[i] = offsets[i] + texStart;
        }
    }
    else
    {
        for (i = 0; i < 4; i++)
            ((miptex_t *)curtex)->offsets[i] = offsets[i] + texStart;
    }

    truebits = holdStart;
    newTex->bits = holdStart;
    unsigned char gammatable[256];
    float gamma = set.gamma;
    if (gamma == 1.0f)
    {
        for (i=0 ; i<256 ; i++)
            gammatable[i] = (unsigned char)i;
    }
    else
    {
        for (i=0 ; i<256 ; i++)
        {
            int inf = 255 * (float) pow ( (float)((i+0.5f)/255.5f) , (float) gamma ) + 0.5f;
            inf = min(255, inf);
            inf = max(0, inf);
            gammatable[i] = (unsigned char)inf;
        }
    }
    if (set.sinBsp)
    {
        if (set.valveBsp)
        {
            for (int palc = 0; palc < ((valve_miptexnew_t *)curtex)->palSize*3; palc++)
                ((valve_miptexnew_t *)curtex)->palette[palc] =
                    gammatable[((valve_miptexnew_t *)curtex)->palette[palc]];
            newTex->flat = TranslateColors(((valve_miptexnew_t *)curtex)->palSize,((valve_miptexnew_t *)curtex)->palette,truebits,size);
        }
        else
        {
            for (int palc = 0; palc < 256*3; palc++)
                ((sin_miptexnew_t *)curtex)->palette[palc] = gammatable[((sin_miptexnew_t *)curtex)->palette[palc]];
            newTex->flat = TranslateColors(256,((sin_miptexnew_t *)curtex)->palette,truebits,size);
        }
    }
    else
    {
        newTex->flat = TranslateColors(256,set.pal,truebits,size);
    }

    if (newTex->isSkyQ1())
    {
        // cut down the heights...
        newTex->w /= 2;
        w /= 2;
        newTex->sky = new sky_t;
        newTex->sky->skyRow = 0;
        newTex->sky->skyCol = 0;

        newTex->sky->bits = newTex->bits;
        size = (uintptr_t)w * (uintptr_t)h;
        size = size * 85/64;
        newTex->bits = (unsigned char*)new char[size+3];
        CreateSkyBits(newTex->w,newTex->h,newTex->sky,newTex->bits);
    }
    else if (newTex->isWarp())
    {
        newTex->sky = new sky_t;
        newTex->sky->skyRow = 0;
        newTex->sky->skyCol = 0;

        newTex->sky->bits = newTex->bits;
        size = (uintptr_t)w * (uintptr_t)h;
        size = size * 85/64;
        newTex->bits = (unsigned char*)new char[size+3];
        CreateWaterBits(newTex->w,newTex->h,newTex->sky,newTex->bits);
    }

    unsigned char *thePal;
    if (set.sinBsp)
    {
        if (set.valveBsp)
        {
            thePal = (unsigned char *)((valve_miptexnew_t *)curtex)->palette;
        }
        else
        {
            thePal = (unsigned char *)((sin_miptexnew_t *)curtex)->palette;
        }
    }
    else
    {
        thePal = (unsigned char *)set.pal;
    }

    if (set.glBsp)
        CalcGLBits(newTex,thePal);

    if (set.sinBsp)
    {

        size = (uintptr_t)w * (uintptr_t)h;
        size = size * 85/64;

        //delete [] newTex->truebits;
        free (newTex->truebits);
        newTex->truebits = (unsigned char*)new char[3*size+3];

        unsigned char *tP = (unsigned char *)newTex->truebits;
        unsigned char *sP = (unsigned char *)newTex->bits;

        if (set.valveBsp)
        {
            for (int qq = 0; qq < size; qq++, sP++)
            {
                *tP++ = ((valve_miptexnew_t *)curtex)->palette[PALSTEP*(*sP)+2];
                *tP++ = ((valve_miptexnew_t *)curtex)->palette[PALSTEP*(*sP)+1];
                *tP++ = ((valve_miptexnew_t *)curtex)->palette[PALSTEP*(*sP)+0];
            }
        }
        else
        {
            for (int qq = 0; qq < size; qq++, sP++)
            {
                *tP++ = ((sin_miptexnew_t *)curtex)->palette[PALSTEP*(*sP)+2];
                *tP++ = ((sin_miptexnew_t *)curtex)->palette[PALSTEP*(*sP)+1];
                *tP++ = ((sin_miptexnew_t *)curtex)->palette[PALSTEP*(*sP)+0];
            }
        }
    }

    BITMAPINFO *m_pBmInfo;
    BITMAPINFOHEADER m_pBmInfoHeader;

    memset(&m_pBmInfoHeader,0,sizeof(BITMAPINFOHEADER));
    m_pBmInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
    m_pBmInfoHeader.biWidth = w;
    m_pBmInfoHeader.biHeight = -h;
    m_pBmInfoHeader.biPlanes = 1;
    m_pBmInfoHeader.biBitCount = 8;
    m_pBmInfoHeader.biCompression = BI_RGB;
    m_pBmInfoHeader.biSizeImage = w * h;
    m_pBmInfoHeader.biClrUsed = 256;
    m_pBmInfo = (BITMAPINFO*)new char[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];

    memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));

    if (set.sinBsp)
    {
        if (set.valveBsp)
        {
            for (int hhh = 0; hhh < 256; hhh++)
            {
                m_pBmInfo->bmiColors[hhh].rgbRed   = ((valve_miptexnew_t *)curtex)->palette[PALSTEP*hhh+0];
                m_pBmInfo->bmiColors[hhh].rgbGreen = ((valve_miptexnew_t *)curtex)->palette[PALSTEP*hhh+1];
                m_pBmInfo->bmiColors[hhh].rgbBlue  = ((valve_miptexnew_t *)curtex)->palette[PALSTEP*hhh+2];
                m_pBmInfo->bmiColors[hhh].rgbReserved = 0;
            }
        }
        else
        {
            for (int hhh = 0; hhh < 256; hhh++)
            {
                m_pBmInfo->bmiColors[hhh].rgbRed   = ((sin_miptexnew_t *)curtex)->palette[PALSTEP*hhh+0];
                m_pBmInfo->bmiColors[hhh].rgbGreen = ((sin_miptexnew_t *)curtex)->palette[PALSTEP*hhh+1];
                m_pBmInfo->bmiColors[hhh].rgbBlue  = ((sin_miptexnew_t *)curtex)->palette[PALSTEP*hhh+2];
                m_pBmInfo->bmiColors[hhh].rgbReserved = 0;
            }
        }
    }
    else
    {
        memcpy(m_pBmInfo->bmiColors,set.cTable,256*sizeof(RGBQUAD));
    }
    newTex->dibInfo = m_pBmInfo;
    newTex->truedibInfo = NULL; // tm_pBmInfo;

    if (set.valveBsp)
    {
        delete[] ((valve_miptexnew_t *)curtex)->palette;
    }
    delete curtex;
    delete[] thedib;

    sprintf(outstr,"Inserting [%s]",s);
    status->SetText(outstr,true);
    texWad *toWad = currentWadPtr;      //qqq

    if(!toWad->textures)
    {
        return;
    }

    currentHead = toWad->textures->next; // first one
    int done = 0;
    while (currentHead && !done)
    {
        if (currentHead->next)
        {
            if (QCompare(currentHead->bmp,
                         newTex->bmp) > 0)
            {
                newTex->next = currentHead;
                newTex->previous = currentHead->previous;
                newTex->previous->next = newTex;
                newTex->next->previous = newTex;
                toWad->bmpsCount++;
                done = 1;
            }
        }
        else
        {
            // insert before this one...
            newTex->next = currentHead;
            newTex->previous = currentHead->previous;
            newTex->previous->next = newTex;
            newTex->next->previous = newTex;
            toWad->bmpsCount++;
            done = 1;
        }
        currentHead = currentHead->next;
    }

    SetUpMap(map_i[set.curmap]);
    ChangeToMap(map_i[set.curmap]);

    RedrawContents();
}

// used for building mipmaps. takes "count" pixels from "pixmap" and averages them.
unsigned char TTextureWindow::AveragePixels(int count)
{
    int r,g,b;
    int i;
    int vis;
    unsigned char pix;
    int dr, dg, db;
    int bestdistortion, distortion;
    unsigned char bestcolor;
    unsigned char *pal;
    int fullbright;
    int e;

    unsigned char *lbmpalette = (unsigned char *)set.pal;

    vis = 0;
    r = g = b = 0;
    fullbright = 0;
    for (i=0 ; i<count ; i++)
    {
        pix = pixdata[i];
        if (pix == 255)
        {
            fullbright = 2;
        }
        else if (pix >= 240)
        {
            return pix;
        }
        else
        {
            if (fullbright)
                continue;
        }

        r += lbmpalette[pix*3+0];
        g += lbmpalette[pix*3+1];
        b += lbmpalette[pix*3+2];
        vis++;
    }

    if (!vis || fullbright == 2)
        return 255;

    r /= vis;
    g /= vis;
    b /= vis;

    if (!fullbright)
    {
        r += d_red;
        g += d_green;
        b += d_blue;
    }

//
// find the best color
//
    bestdistortion = r*r + g*g + b*b;
    bestcolor = 0;
    if (fullbright)
    {
        i = 240;
        e = 255;
    }
    else
    {
        i = 0;
        e = 240;
    }

    for ( ; i< e ; i++)
    {
        pix = (unsigned char)i;	//pixdata[i];

        pal = lbmpalette + pix*3;

        dr = r - (int)pal[0];
        dg = g - (int)pal[1];
        db = b - (int)pal[2];

        distortion = dr*dr + dg*dg + db*db;
        if (distortion < bestdistortion)
        {
            if (!distortion)
            {
                d_red = d_green = d_blue = 0;	// no distortion yet
                return pix;		// perfect match
            }

            bestdistortion = distortion;
            bestcolor = pix;
        }
    }

    if (!fullbright)
    {
        // error diffusion
        pal = lbmpalette + bestcolor*3;
        d_red = r - (int)pal[0];
        d_green = g - (int)pal[1];
        d_blue = b - (int)pal[2];
    }

    return bestcolor;
}


void TTextureWindow::GatherPakTextures(char *pakname, texture_hash **hashTable)
{
    // hash them in...
    if (!file_exists(pakname))
    {
        syserror(const_cast<char *> ("Gather Pak Textures, pak file does not exist [%s]..."), pakname);
        return;
    }

    pakheader_t pakheader;

    FILE *f = fopen(pakname,"rb");
    if (!f)
    {
        syserror(const_cast<char *> ("Gather Pak Textures, couldn't open pak file [%s]..."), pakname);
        return;
    }

    fread(&pakheader,sizeof(pakheader_t),1,f);
    if (strnicmp((char *)pakheader.magic,"PACK",4))
    {
        fclose(f);
        syserror(const_cast<char *> ("Gather Pak Textures, [%s] is not a Pakfile..."), pakname);
        return;
    }

    int numentries = pakheader.dirsize / (sizeof(pakentry_t));
    fseek(f,pakheader.diroffset,SEEK_SET);

    pakentry_t  *pakentry = new pakentry_t[numentries];
    char *start;
    int ct;
    pakentry_t *e;
    char dir[64];
    char walName[64];
    char addin[2];
    sprintf(addin,"%c",set.rel_path_separator);

    for (int i=0; i<numentries; i++)
    {
        fread(&pakentry[i],sizeof(pakentry_t),1,f);
        e = &pakentry[i];
        start = (char *)e->filename;
        if (strnicmp(start, "textures",8))
            continue;

        start += 8; //strlen("textures");
        start++;

        strncpy(dir,start,sizeof(dir));
        start = dir;
        ct = 0;
        while (*start && (*start != '\\') && (*start != '/') && (ct++ <= 32))
            start++;

        if (ct >= 32)  // problem, so skip...
            continue;

        *start = '\0';
        // now go forward a bit...
        start++;
        strncpy(walName, start, sizeof(walName));
        strcat(dir, addin);
        // Load it in...
        StripExtension(walName);

        strupr(dir);
        strupr(walName);

        AddToHashPak(walName,dir,e,hashTable);
    }
    fclose(f);
    delete[] pakentry;
}

void TTextureWindow::LoadTexturesInUse(char *basepath, map *m)
{
    if (!m)
        return;

    int i;
    uintptr_t size;
    texInfo_t *newTex;
    texInfo_t *currentHead;
    BITMAPINFOHEADER m_pBmInfoHeader;
    char fname[256];
    unsigned char *lump;
    miptex2_t *qtex;
    char	filename[1024];
    unsigned char *source;
    unsigned	char *dest;
    int width, height, count;

    char shortName[256];
    ExtractFileBase(m->filename,shortName,false);
    sprintf(fname,"Use(%s)",shortName);

    // Quake II .wal file loader...
    currentWadPtr->WadType = QUAKEII_WAD;

    texture_hash **pakTable;
    if (set.pakOk)
    {
        pakTable = new texture_hash *[MAX_HASH];
        memset(pakTable,0,MAX_HASH * sizeof(texture_hash *));
        GatherPakTextures(set.pak_file, pakTable);
    }

    /*FIXME for QII
    if (!CheckWad(fname)) {
    	MessageBox("Error:  Creating a Fake Texture Wad", "BSP Wad Loader", MB_OK);
    	CreateFakeWad(fname);
    	return;
    };
    */

    strncpy(currentWadPtr->currentWad, fname, sizeof(currentWadPtr->currentWad));

    // This would be weird...
    if (currentWadPtr->bmpsCount)
        FreeTextures(currentWadPtr);

    char statstr[80] = "";
    status->SetText("",true);

    int counter = 0;

    currentWadPtr->textures = new texInfo_t; // the head...
    currentHead = currentWadPtr->textures;

    newTex = new texInfo_t;

    newTex->next = NULL;
    newTex->previous = currentHead;
    currentHead->next = newTex;

    char tempNm[LUMP_NAME_LENGTH_MAX];

    // Cycle through the .wal files!
    char walName[128];

    // NO, create a hash, sort all names into it, then
    texture_hash **hashTable;
    hashTable = new texture_hash *[MAX_HASH];
    memset(hashTable,0,MAX_HASH * sizeof(texture_hash *));

    for (Entity *e = m->objects.p_next; e != &m->objects; e = e->p_next)
    {
        if (!e->modifiable)
            continue;
        for (SetBrush *b = e->objects.p_next; b != &e->objects; b = b->p_next)
        {
            if (b->IsInvalid())
                continue;
            // For each face, put the texture into the hash_table...
            for (face_t *fa = b->faces.p_next; fa != &b->faces; fa = fa->p_next)
            {
                AddToHash(fa->texture.texture,fa->texture.basepath,hashTable);
            }
        }
    }

    texture_hash *n;
    int fsize;
    int hashNum;

    bool fromPak;
    pakentry_t *p;
    FILE *pf;
    if (set.pakOk)
    {
        pf = fopen(set.pak_file,"rb");
        if (!pf)
        {
            syserror(const_cast<char *> ("Error opening pakfile [%s] in Texture in Use Scanner..."), set.pak_file);
            Msgbox(const_cast<char *> (  "Error opening pakfile [%s] in Texture in Use Scanner..."), set.pak_file);
            return;
        }
    }

    for (hashNum = 0; hashNum < MAX_HASH; hashNum++)
    {
        for (n = hashTable[hashNum] ; n; n = n->next)
        {
            fromPak = false;
            sprintf(statstr,"Loading %s [%i] ",n->name,counter+1);
            status->SetText(statstr,true);
            sprintf(walName,"%s%s",n->basepath,n->name);

            // Load it in...
            StripExtension(walName);

            // load the file
            sprintf (filename, "%s%s.wal", basepath,walName);

            if (!file_exists(filename))
            {
                if (set.pakOk)  					// how about in pak
                {
                    p = SearchHash(n->name,n->basepath,pakTable);
                    if (!p)
                        continue;
                    fromPak = true;
                }
                else
                {
                    continue;
                }
            }

            if (!fromPak)
            {
                lump = file_get_contents(filename);
                if (!lump)
                {
                    syserror(const_cast<char *> ("load .wal failed: %s\n"), filename);
                    continue;
                }
            }
            else
            {
                fsize = p->size;
                lump = new unsigned char[fsize];
                fseek(pf,p->offset,SEEK_SET);
                fread(lump,p->size,1,pf);  // read in the .wal
            }

            qtex = (miptex2_t *)lump;

            width = qtex->width;
            height = qtex->height;

            newTex = new texInfo_t;
            newTex->def_flags    = qtex->flags;
            newTex->def_value    = qtex->value;
            newTex->def_contents = qtex->contents;

            newTex->texType = TEX_TYPE_Q2;
            strncpy(newTex->basepath, n->basepath, sizeof(newTex->basepath));
            newTex->basepath[strlen(newTex->basepath)-1] = 0;// strip the extra slashes
            strncpy (qtex->name, n->name, sizeof(n->name)-1);
            StripExtension (qtex->name);

            strncpy(tempNm,qtex->name,sizeof(tempNm));

            tempNm[LUMP_NAME_LENGTH-1] = '\0';
            strncpy(newTex->bmp,tempNm, sizeof(newTex->bmp));
            strupr(newTex->bmp);

            newTex->index = counter;

            size = (uintptr_t)width * (uintptr_t)height;
            size = size * 85/64;

            newTex->h = height;
            newTex->w = width;

            dest = (unsigned char *)new char[size+width+1];

            count = size;
            source = (unsigned char *)qtex;
            source += qtex->offsets[0];

            // copy the bits...
            for (i=0 ; i<count ; i++) dest[i] = source[i];

            delete[] lump;

            newTex->bits = dest;
            newTex->flat = TranslateColors(256,
                                           set.pal,dest,size);

            if (newTex->isSkyQ1())
            {
                // cut down the heights...
                newTex->w /= 2;
                width /= 2;

                newTex->sky = new sky_t;
                newTex->sky->skyRow = 0;
                newTex->sky->skyCol = 0;

                newTex->sky->bits = newTex->bits;
                size = (uintptr_t)width * (uintptr_t)height;
                size = size * 85/64;
                newTex->bits = (unsigned char*)new char[size+3];
                CreateSkyBits(newTex->w,newTex->h,newTex->sky,newTex->bits);
            }
            else if (newTex->isWarp())
            {
                newTex->sky = new sky_t;
                newTex->sky->skyRow = 0;
                newTex->sky->skyCol = 0;

                newTex->sky->bits = newTex->bits;
                size = (uintptr_t)width * (uintptr_t)height;
                size = size * 85/64;
                newTex->bits = (unsigned char*)new char[size+3];
                CreateWaterBits(newTex->w,newTex->h,newTex->sky,newTex->bits);
            }

            unsigned char *thePal = (unsigned char *)set.pal;

            if (set.glBsp)
                CalcGLBits(newTex,thePal);

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
            m_pBmInfo = (BITMAPINFO*)new char[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];

            memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
            memcpy(m_pBmInfo->bmiColors,set.cTable,256*sizeof(RGBQUAD));

            newTex->dibInfo = m_pBmInfo;
            newTex->truedibInfo = NULL; // tm_pBmInfo;

            HDC hdc = GetDC(hwnd);
            SetStretchBltMode(hdc, COLORONCOLOR);

            StretchDIBits(hdc,
                          0,0,newTex->w,newTex->h,
                          0,0,newTex->w,newTex->h,
                          newTex->bits, newTex->dibInfo,
                          DIB_RGB_COLORS, SRCCOPY);
            ReleaseDC(hwnd,hdc);

            currentHead = currentWadPtr->textures->next; // first one
            int done = 0;

            while (currentHead && !done)
            {
                if (currentHead->next)
                {
                    if (QCompare(currentHead->bmp,newTex->bmp) > 0)
                    {
                        newTex->next = currentHead;
                        newTex->previous = currentHead->previous;
                        newTex->previous->next = newTex;
                        newTex->next->previous = newTex;
                        done = 1;
                    }
                }
                else
                {
                    // insert before this one...
                    newTex->next = currentHead;
                    newTex->previous = currentHead->previous;
                    newTex->previous->next = newTex;
                    newTex->next->previous = newTex;
                    done = 1;
                }
                currentHead = currentHead->next;
            }
            counter++;

            //n = n->next;
        }
    }

    texture_hash *ch;
    for (i = 0; i < MAX_HASH; i++)
    {
        n = hashTable[i];

        while (n)
        {
            ch = n;
            n = n->next;
            delete ch;
        }

        hashTable[i] = NULL;
    }
    delete[] hashTable;

    // Clean this up, too...
    if (set.pakOk)
    {
        fclose(pf);
        for (i = 0; i < MAX_HASH; i++)
        {
            n = pakTable[i];

            while (n)
            {
                ch = n;
                n = n->next;
                delete ch;
            }
            pakTable[i] = NULL;
        }
        delete[] pakTable;
    }
    currentWadPtr->bmpsCount = counter;

    status->SetText("Done...",true);

    // Is this needed???xxx FIXME
    texList->ClearList();
    if (frame->TextureList)
    {
        frame->TextureList->ClearList();
        delete[] frame->data;
        frame->data = new ListBoxData[currentWadPtr->bmpsCount];
        memset(frame->data,0,currentWadPtr->bmpsCount*sizeof(ListBoxData));
    }

    for (i = 0; i< currentWadPtr->bmpsCount; i++)
    {
        newTex = GetTextureNo(currentWadPtr,i);
        texList->InsertString(newTex->bmp,i);
        if (frame->TextureList)
        {
            frame->data[i].tx = newTex;
            strncpy(frame->data[i].description, newTex->bmp, sizeof(frame->data[i].description));
            frame->TextureList->InsertString((LPSTR)&frame->data[i],i);
        }
    }

    newTex = GetTextureNo(currentWadPtr,0);
    if (newTex)
    {
        texList->SetText(newTex->bmp);
    }
    else
    {
        texList->SetText("");
        if (frame->TextureList)
            frame->TextureList->SetText("");
    }

    if (frame->TextureList)
        frame->TextureList->SetSelIndex(0);

    UpdateScrollbar();

    prevIndex = 0;
    ResetDefaults();
    RedrawContents();
}

void TTextureWindow::ScanNewWad()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    if (!m)
        return;

    struct _finddata_t ffb;
    memset(&ffb,0,sizeof(ffb));

    char dirname[256];
    sprintf(dirname,"%s\\*.wad",set.wad_directory);
    int ff = _findfirst(dirname,&ffb);
    int nomorefiles = ff < 0 ? 1 : 0;

    TScanTransferBuffer x;
    x.Directory.Clear();
    *x.caption = 0;

    int count = 0;
    while (!nomorefiles)
    {
        if (ffb.attrib & _A_SUBDIR)
        {
            nomorefiles = _findnext(ff, &ffb);
            continue;
        }
        x.Directory.Add(ffb.name);
        nomorefiles = _findnext(ff, &ffb);
        count++;
    }
    _findclose(ff);

    if (count <= 0)
    {
        sprintf(dirname,"No .wad files in wad directory [%s]...",set.wad_directory);
        MessageBox(hwnd,dirname, "BSP Scan Wad", MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    TScanDlg dialog(0,&x);
    if (dialog.Execute() == IDOK)
    {
        texWad *Wad;
        Wad = QueryWadLoaded(x.selected);  // scan through list!
        // if found it, just change to it...
        if (!Wad)
        {
            Wad = new texWad;
            int saveGame = set.game_mode;
            set.game_mode = 0; // Act like quake
            int inUse = set.loadInUse;
            set.loadInUse = 0;
            AddWad(Wad,x.selected,x.selected,set.game_mode);
            set.loadInUse = inUse;
            set.game_mode = saveGame;
        }
        m->Wads[m->numWads++] = Wad;

        UnloadTextures();

        SetUpMap(m);
        ChangeToMap(m);

        if (set.game_mode != 2)
        {
            // For quake , don't forget to add to Wad Key!
            MessageBox(hwnd, "You should add the new .wad to the map's 'wad' key...",
                       "BSP - Load New .Wad File", MB_OK | MB_ICONEXCLAMATION);
        }

        sprintf(dirname,"Texture .wad [%s] scanned...",x.selected);
        set.redrawxy = 1;
        set.redrawedit = 1;
        Show_Frame(dirname,true);
        prevIndex = 0;
        ResetDefaults();
        RedrawContents();
    }
}

void TTextureWindow::ScanNewWalDirectory()
{
    const char err_title[] = "BSP Scan Directory";

    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    if (!m)
        return;

    struct _finddata_t ffb;
    memset(&ffb,0,sizeof(ffb));

    char dirname[256];
    sprintf(dirname,"%s\\*.*",set.texture_path);
    int ff = _findfirst(dirname,&ffb);
    int nomorefiles = ff < 0 ? 1 : 0;

    TScanTransferBuffer x;
    x.Directory.Clear();
    *x.caption = 0;

    int count = 0;
    while (!nomorefiles)
    {
        if (!(ffb.attrib & _A_SUBDIR) || (ffb.name[0] == '.'))
        {
            nomorefiles = _findnext(ff, &ffb);
            continue;
        }
        x.Directory.Add(ffb.name);
        nomorefiles = _findnext(ff, &ffb);
        count++;
    }
    _findclose(ff);
    if (count <= 0)
    {
        sprintf(dirname,"No subdirectories in [%s]...",set.texture_path);
        MessageBox(hwnd,dirname, err_title, MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    TScanDlg dialog(0,&x);
    if (dialog.Execute() == IDOK)
    {

        texWad *Wad = QueryWadLoaded(x.selected);  // scan through list!
        // if found it, just change to it...
        if (!Wad)
        {
            Wad = new texWad;
            int inUse = set.loadInUse;
            set.loadInUse = 0;
            AddWad(Wad,x.selected,x.selected,set.game_mode);
            set.loadInUse = inUse;
        }
        m->Wads[m->numWads++] = Wad;

        UnloadTextures();
        SetUpMap(m);
        ChangeToMap(m);

        sprintf(dirname,"Directory [%s] scanned...",x.selected);

        set.redrawxy = 1;
        set.redrawedit = 1;
        Show_Frame(dirname,true);
        prevIndex = 0;
        ResetDefaults();
        RedrawContents();
    }
}

void TTextureWindow::ScanNewTexture()
{
    const char *err_title = "BSP Scan Texture";

    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    if (!m)
        return;

    TWalTransferBuffer x;
    *x.sel_dir = 0;
    *x.sel_wal = 0;
    TWalDlg dialog(hwnd,&x);
    if (dialog.Execute() != IDOK)
        return;

    char checkStr[256];
    ExtractFileBase(m->filename,checkStr,false);
    char fname[256];
    sprintf(fname,"Use(%s)",checkStr);
    // Clear it if it's loaded...
    texWad *Wad;

    if (!QueryWadLoaded(fname))
        ScanInUse(m);

    Wad = QueryWadLoaded(fname);
    if (!Wad)
    {
        MessageBox(hwnd,"Couldn't load used textures?", err_title, MB_ICONEXCLAMATION | MB_OK);
        return;
    }

    ////////////////////////////////////////////////////////
    // Now load and add the miptex...
    int i;
    uintptr_t size;
    BITMAPINFOHEADER m_pBmInfoHeader;
    miptex2_t *qtex;
    unsigned char *source, *dest;
    int width, height, count;
    texInfo_t *newTex;

    // Load it in...
    StripExtension(x.sel_wal);

    // load the file
    Path filename(const_cast<char *> ("%s/%s/%s.wal"), set.texture_path, x.sel_dir, x.sel_wal);

    unsigned char *lump = file_get_contents(filename);

    if (!lump)
    {
        syserror(const_cast<char *> ("load wal failed: %s\n"), (char*) filename);
        Msgbox(const_cast<char *> ("load wal failed: %s\n"), (char*) filename);
        return;
    }

    qtex = (miptex2_t *)lump;

    width = qtex->width;
    height = qtex->height;

    newTex = new texInfo_t;

    newTex->def_flags    = qtex->flags;
    newTex->def_value    = qtex->value;
    newTex->def_contents = qtex->contents;

    newTex->texType = TEX_TYPE_Q2;
    strncpy(newTex->basepath,x.sel_dir, sizeof(newTex->basepath));

    strncpy (qtex->name, x.sel_wal, sizeof(qtex->name)-1);
    StripExtension (qtex->name);

    char tempNm[LUMP_NAME_LENGTH_MAX];
    strncpy(tempNm, qtex->name, sizeof(tempNm));

    tempNm[LUMP_NAME_LENGTH-1] = 0;
    strncpy(newTex->bmp, tempNm, sizeof(newTex->bmp));
    strupr(newTex->bmp);

    newTex->index = 0;

    size = (uintptr_t)width * (uintptr_t)height;
    size = size * 85/64;

    newTex->h = height;
    newTex->w = width;

    dest = new unsigned char[size+width+1];

    count = size;
    source = (unsigned char *)qtex;
    source += qtex->offsets[0];

    // copy the bits...
    for (i=0 ; i<count ; dest[i] = source[i], i++);

    delete[] lump;

    newTex->bits = dest;
    newTex->flat = texWindow->TranslateColors(256,set.pal,dest,size);

    if (newTex->isSkyQ1())
    {
        // cut down the heights...
        newTex->w /= 2;
        width /= 2;

        newTex->sky = new sky_t;
        newTex->sky->skyRow = 0;
        newTex->sky->skyCol = 0;

        newTex->sky->bits = newTex->bits;
        size = (uintptr_t)width * (uintptr_t)height;
        size = size * 85/64;
        newTex->bits = (unsigned char*)new char[size+3];
        CreateSkyBits(newTex->w,newTex->h,newTex->sky,newTex->bits);
    }
    else if (newTex->isWarp())
    {
        newTex->sky = new sky_t;
        newTex->sky->skyRow = 0;
        newTex->sky->skyCol = 0;

        newTex->sky->bits = newTex->bits;
        size = (uintptr_t)width * (uintptr_t)height;
        size = size * 85/64;
        newTex->bits = (unsigned char*)new char[size+3];
        CreateWaterBits(newTex->w,newTex->h,newTex->sky,newTex->bits);
    }

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
    m_pBmInfo = (BITMAPINFO*)new char[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];

    memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
    memcpy(m_pBmInfo->bmiColors,set.cTable,256*sizeof(RGBQUAD));

    newTex->dibInfo = m_pBmInfo;
    newTex->truedibInfo = NULL; // tm_pBmInfo;

    HDC hdc = GetDC(hwnd);
    SetStretchBltMode(hdc, COLORONCOLOR);
    StretchDIBits(hdc,
                  0,0,newTex->w,newTex->h,
                  0,0,newTex->w,newTex->h,
                  newTex->bits, newTex->dibInfo,
                  DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(hwnd,hdc);

    /////////////////////////////////////////////////
    // NOW ADD TO Wad...

    texInfo_t *currentHead;

    currentHead = Wad->textures->next; // first one
    int done = 0;

    while (currentHead && !done)
    {
        if (currentHead->next)
        {
            if (QCompare(currentHead->bmp,newTex->bmp) > 0)
            {
                newTex->next = currentHead;
                newTex->previous = currentHead->previous;
                newTex->previous->next = newTex;
                newTex->next->previous = newTex;
                done = 1;
            };
        }
        else
        {
            // insert before this one...
            newTex->next = currentHead;
            newTex->previous = currentHead->previous;
            newTex->previous->next = newTex;
            newTex->next->previous = newTex;
            done = 1;
        };
        currentHead = currentHead->next;
    }

    // Increment # of textures...
    Wad->bmpsCount++;
    //////////////////////////////////////////////////
    UnloadTextures();

    SetUpMap(m);
    ChangeToMap(m);

    char outstr[256];
    sprintf(outstr,"File [%s\\%s] scanned...",x.sel_dir,x.sel_wal);

    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame(outstr,true);
    prevIndex = 0;
    ResetDefaults();
    RedrawContents();
}

texInfo_t *TTextureWindow::TryLoadTextureWal(map *m, char *walFileName)
{
    if (!set.Map_Read || !m)
        return NULL;

    char dir[256];
    char wal[256];
    SplitBaseQuiet(walFileName,dir,wal);
    if (!dir || !dir[0])
        return NULL;

    if (!wal || !wal[0])
        return NULL;

    char checkStr[256];
    ExtractFileBase(m->filename,checkStr,false);
    char fname[256];
    sprintf(fname,"Use(%s)",checkStr);

    texWad *Wad;
    if (!QueryWadLoaded(fname))
        ScanInUse(m);

    Wad = QueryWadLoaded(fname);
    if (!Wad)
        return NULL;

    ////////////////////////////////////////////////////////
    // Now load and add the miptex...
    int			i;
    uintptr_t size;
    BITMAPINFOHEADER m_pBmInfoHeader;
    unsigned char *lump;
    miptex2_t *qtex;
    char	filename[1024];
    unsigned char *source;
    unsigned	char *dest;
    int width, height, count;
    texInfo_t *newTex;

    // Load it in...
    // shouldn't have any extension...
    StripExtension(wal);

    // load the file
    sprintf (filename,"%s\\%s\\%s.wal", set.texture_path, dir, wal);

    lump = file_get_contents(filename);
    if (!lump)
    {
        syserror(const_cast<char *> ("Load .wal failed: %s\n"), filename);
        return 0;
    }
    qtex = (miptex2_t *)lump;

    width = qtex->width;
    height = qtex->height;

    newTex = new texInfo_t;

    newTex->def_flags    = qtex->flags;
    newTex->def_value    = qtex->value;
    newTex->def_contents = qtex->contents;

    newTex->texType = TEX_TYPE_Q2;
    strncpy(newTex->basepath,dir, sizeof(newTex->basepath));

    strncpy (qtex->name, wal, sizeof(qtex->name)-1);
    StripExtension (qtex->name);

    char tempNm[LUMP_NAME_LENGTH_MAX];
    strncpy(tempNm,qtex->name,sizeof(tempNm));

    tempNm[LUMP_NAME_LENGTH-1] = 0;
    strncpy(newTex->bmp,tempNm, sizeof(newTex->bmp));
    strupr(newTex->bmp);

    newTex->index = 0;

    size = (uintptr_t)width * (uintptr_t)height;
    size = size * 85/64;

    newTex->h = height;
    newTex->w = width;

    dest = new unsigned char[size+width+1];

    count = size;
    source = (unsigned char *)qtex;
    source += qtex->offsets[0];

    // copy the bits...
    for (i=0 ; i<count ; dest[i] = source[i], i++);

    delete[] lump;

    newTex->bits = dest;
    newTex->flat = texWindow->TranslateColors(256,set.pal,dest,size);

    if (newTex->isSkyQ1())
    {
        // cut down the heights...
        newTex->w /= 2;
        width /= 2;

        newTex->sky = new sky_t;
        newTex->sky->skyRow = 0;
        newTex->sky->skyCol = 0;

        newTex->sky->bits = newTex->bits;
        size = ((uintptr_t)width * (uintptr_t)height) * (85/64);
        newTex->bits = (unsigned char*)new char[size+3];
        CreateSkyBits(newTex->w,newTex->h,newTex->sky,newTex->bits);
    }
    else if (newTex->isWarp())
    {
        newTex->sky = new sky_t;
        newTex->sky->skyRow = 0;
        newTex->sky->skyCol = 0;

        newTex->sky->bits = newTex->bits;
        size = (uintptr_t)width * (uintptr_t)height;
        size = size * 85/64;
        newTex->bits = (unsigned char*)new char[size+3];
        CreateWaterBits(newTex->w,newTex->h,newTex->sky,newTex->bits);
    }

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
    m_pBmInfo = (BITMAPINFO*)new char[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];

    memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
    memcpy(m_pBmInfo->bmiColors,set.cTable,256*sizeof(RGBQUAD));

    newTex->dibInfo = m_pBmInfo;
    newTex->truedibInfo = NULL; // tm_pBmInfo;

    HDC hdc = GetDC(hwnd);
    SetStretchBltMode(hdc, COLORONCOLOR);

    StretchDIBits(hdc,
                  0,0,newTex->w,newTex->h,
                  0,0,newTex->w,newTex->h,
                  newTex->bits,
                  newTex->dibInfo,
                  DIB_RGB_COLORS,
                  SRCCOPY);
    ReleaseDC(hwnd,hdc);

    /////////////////////////////////////////////////
    // NOW ADD TO Wad...

    texInfo_t *currentHead = Wad->textures->next; // first one
    int done = 0;

    while (currentHead && !done)
    {
        if (currentHead->next)
        {
            if (QCompare(currentHead->bmp,newTex->bmp) > 0)
            {
                newTex->next = currentHead;
                newTex->previous = currentHead->previous;
                newTex->previous->next = newTex;
                newTex->next->previous = newTex;
                done = 1;
            };
        }
        else
        {
            newTex->next = currentHead;		// insert before this one...
            newTex->previous = currentHead->previous;
            newTex->previous->next = newTex;
            newTex->next->previous = newTex;
            done = 1;
        }
        currentHead = currentHead->next;
    }

    // Increment # of textures...
    Wad->bmpsCount++;
    // add to nummaptextures???
    return newTex;
}

void TTextureWindow::Invalidate()
{
    InvalidateRect(hwnd,0,true);
}
