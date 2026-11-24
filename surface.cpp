// Implementation of Surface Properties Class

#include "global.h"

// ************************
LRESULT WEditSurf::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_SETFOCUS:
        KillAccels = true;
        break;
    case WM_KILLFOCUS:
        KillAccels = false;
        break;
    }
    return WEdit::WndProc(msg,wParam,lParam);
}
WEditSurf::WEditSurf(HWND parent,int id,char *text,int cx,int cy,int width,int height,int textLimit)
    : WEdit(parent,id,text,cx,cy,width,height,textLimit)
{}
// ************************
LRESULT TDownButton::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_LBUTTONDOWN:
        if (surfaceWindow)
        {
            surfaceWindow->MouseDown(dm);
        }
        break;
    case WM_LBUTTONUP:
        if (surfaceWindow) surfaceWindow->MouseUp(dm);
        break;
    case WM_KEYDOWN:
        if(wParam==VK_RETURN && surfaceWindow)
            surfaceWindow->MouseFunc(dm);
    }
    return WButton::WndProc(msg,wParam,lParam);
}

TDownButton::TDownButton(HWND parent, int Id, char* text, int X, int Y, int W, int H, int downMsg)
    : WButton(parent,Id,text,X,Y,W,H), dm(downMsg)
{}

int t_top = 190;
int t_left = 2;
//////////////////////////////////////////////
TSurfaceWindow::TSurfaceWindow(HWND parent, char *title)
    : TCWindow(parent,title)
{
    bw = 14;
    fw = 60;
    cw = 120;

    attr.width = 2 + (bw+fw+2)*5;
    attr.height = 2*bw + 2 + CaptionHeight + 9;

    //set height of window, quake2 has extra features
    if (set.game_mode == 2)
    {
        attr.height += (12*8);
    }
    else
    {
        attr.height += 20;
    }
    attr.x = 0;
    attr.y = 0;

    //SetCaption(title);
    memset(flags,0,S_FLAGS*sizeof(WCheck*));	// zero contents check*s
    contents = NULL;
    flagsStatic = NULL;
    contentsStatic = NULL;
    valStatic = NULL;
    valEdit = NULL;
    applyFace = NULL;
    applyBrush = NULL;
    applyWal = NULL;
    reset = NULL;
    af = NULL;
    df = NULL;
    ac = NULL;
    dc = NULL;
    sf = NULL;
    sc = NULL;
}

//WindowPlacement
HWND TSurfaceWindow::WP_GetHwnd()
{
    return hwnd;
}
const char *TSurfaceWindow::WP_WindowName()
{
    return "Surface";
}

void TSurfaceWindow::SetCaption(const char* /*title*/)
{
    char outstr[80];
    if (texWindow && texWindow->currentTexture)
    {
        texturedef_t *currentTex;
        currentTex = texWindow->currentTexture;
        if (set.game_mode == 2)
        {
            sprintf(outstr,"Surface:  [%s%s]",currentTex->basepath,currentTex->texture);
        }
        else
        {
            sprintf(outstr,"Surface:  [%s]",currentTex->texture);
        }
    }
    else
    {
        strcpy(outstr,"Surface Property Window...");
    }
    TCWindow::SetCaption(outstr);
}
LRESULT TSurfaceWindow::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_CREATE:
        SetupWindow();
        break;

    case WM_TIMER:

        MouseFunc(wParam);
        break;

    case WM_MOUSEWHEEL:

        SetFocusUnderMouse(hwnd);
        break;

    case WM_WINDOWPOSCHANGED:
        ontop->SetCheck(TCWindow::topmost == hwnd);
        break;

    case WM_COMMAND:
        //flags
        if(LOWORD(wParam) >= IDCB_SFLAGBASE && LOWORD(wParam) < IDCB_SFLAGBASE+31)
        {
            GetFlags();
            return 0;
        }
        //contents
        if(LOWORD(wParam) >= IDCB_SCONTENTSBASE && LOWORD(wParam) < IDCB_SCONTENTSBASE+31)
        {
            GetContents();
            return 0;
        }

        //buttons etc
        switch(LOWORD(wParam))
        {
        case IDCB_ADDF:
            AddFlags();
            return 0;
        case IDCB_DELF:
            DelFlags();
            return 0;
        case IDCB_ADDC:
            AddContents();
            return 0;
        case IDCB_DELC:
            DelContents();
            return 0;
        case IDCB_SETF:
            SetFlags();
            return 0;
        case IDCB_SETC:
            SetContents();
            return 0;

        case IDCB_APPLYWAL:
            ApplyWal();
            return 0;
        case IDCB_APPLYFACE:
            ApplyFace();
            return 0;
        case IDCB_APPLYBRUSH:
            ApplyBrush();
            return 0;
        case IDCB_RESET:
            Reset();
            return 0;
        case IDCB_ONTOP:
        {
            ToggleAlwaysOnTop();
            ontop->SetCheck(TCWindow::topmost == hwnd);
            return 0;
        }
        }
        break;
    }
    return TCWindow::WndProc(msg,wParam,lParam);
}


bool TSurfaceWindow::CanClose()
{
    if (!KillAllWindows)
    {
        ShowWindow(SW_HIDE);
    }
    return (0 != KillAllWindows);
}
void TSurfaceWindow::SetupWindow()
{

    int x = 14;
    int y = bw+1;
    int h = bw;

    sE  = new WEditSurf(hwnd,IDC_EDITS,const_cast<char *> (""),x+2,y,fw-4,h,10);
    sE->attr.dwStyle &= ~WS_TABSTOP;
    sE->Create();
    sE->SetFont(set.font9);
    sl  = new WStatic(hwnd,-1,const_cast<char *> ("S"),x+5,1,fw-4,h);
    sl->Create();
    sl->SetFont(set.font9);

    x += (fw+bw);
    tE  = new WEditSurf(hwnd,IDC_EDITT,const_cast<char *> (""),x+2,y,fw-4,h,10);
    tE->attr.dwStyle &= ~WS_TABSTOP;
    tE->Create();
    tE->SetFont(set.font9);
    tl  = new WStatic(hwnd,-1,const_cast<char *> ("T"),x+5,1,fw-4,h);
    tl->Create();
    tl->SetFont(set.font9);

    x += (fw+bw);
    rE  = new WEditSurf(hwnd,IDC_EDITR,const_cast<char *> (""),x+2,y,fw-4,h,10);
    rE->attr.dwStyle &= ~WS_TABSTOP;
    rE->Create();
    rE->SetFont(set.font9);
    rl  = new WStatic(hwnd,-1,const_cast<char *> ("R"),x+5,1,fw-4,h);
    rl->Create();
    rl->SetFont(set.font9);

    x += (fw+bw);
    sxE = new WEditSurf(hwnd,IDC_EDITSX,const_cast<char *> (""),x+2,y,fw-4,h,12);
    sxE->attr.dwStyle &= ~WS_TABSTOP;
    sxE->Create();
    sxE->SetFont(set.font9);
    sxl  = new WStatic(hwnd,-1,const_cast<char *> ("SX"),x+5,1,fw-4,h);
    sxl->Create();
    sxl->SetFont(set.font9);

    x += (fw+bw);
    syE = new WEditSurf(hwnd,IDC_EDITSY,const_cast<char *> (""),x+2,y,fw-4,h,12);
    syE->attr.dwStyle &= ~WS_TABSTOP;
    syE->Create();
    syE->SetFont(set.font9);
    syl  = new WStatic(hwnd,-1,const_cast<char *> ("SY"),x+5,1,fw-4,h);
    syl->Create();
    syl->SetFont(set.font9);

    int base = ID_SBASE;
    x = 2;

    for (int i = 0; i<2; i++)
    {
        if (i == 0)
        {
            s[i]  = new TDownButton(hwnd,base++,const_cast<char *> ("+"),x,i*bw+1,bw,bw,O_UX);
            s[i]->attr.dwStyle &= ~WS_TABSTOP;
            s[i]->Create();
            s[i]->SetFont(set.font9);
            t[i]  = new TDownButton(hwnd,base++,const_cast<char *> ("+"),x+(fw+bw),i*bw+1,bw,bw,O_UY);
            t[i]->attr.dwStyle &= ~WS_TABSTOP;
            t[i]->Create();
            t[i]->SetFont(set.font9);
            r[i]  = new TDownButton(hwnd,base++,const_cast<char *> ("+"),x+2*(fw+bw),i*bw+1,bw,bw,O_UR);
            r[i]->attr.dwStyle &= ~WS_TABSTOP;
            r[i]->Create();
            r[i]->SetFont(set.font9);
            sx[i] = new TDownButton(hwnd,base++,const_cast<char *> ("+"),x+3*(fw+bw),i*bw+1,bw,bw,O_USX);
            sx[i]->attr.dwStyle &= ~WS_TABSTOP;
            sx[i]->Create();
            sx[i]->SetFont(set.font9);
            sy[i] = new TDownButton(hwnd,base++,const_cast<char *> ("+"),x+4*(fw+bw),i*bw+1,bw,bw,O_USY);
            sy[i]->attr.dwStyle &= ~WS_TABSTOP;
            sy[i]->Create();
            sy[i]->SetFont(set.font9);
        }
        else
        {
            s[i]  = new TDownButton(hwnd,base++,const_cast<char *> ("-"),x,i*bw,bw,bw,O_DX);
            s[i]->attr.dwStyle &= ~WS_TABSTOP;
            s[i]->Create();
            s[i]->SetFont(set.font9);
            t[i]  = new TDownButton(hwnd,base++,const_cast<char *> ("-"),x+(fw+bw),i*bw+1,bw,bw,O_DY);
            t[i]->attr.dwStyle &= ~WS_TABSTOP;
            t[i]->Create();
            t[i]->SetFont(set.font9);
            r[i]  = new TDownButton(hwnd,base++,const_cast<char *> ("-"),x+2*(fw+bw),i*bw+1,bw,bw,O_DR);
            r[i]->attr.dwStyle &= ~WS_TABSTOP;
            r[i]->Create();
            r[i]->SetFont(set.font9);
            sx[i] = new TDownButton(hwnd,base++,const_cast<char *> ("-"),x+3*(fw+bw),i*bw+1,bw,bw,O_DSX);
            sx[i]->attr.dwStyle &= ~WS_TABSTOP;
            sx[i]->Create();
            sx[i]->SetFont(set.font9);
            sy[i] = new TDownButton(hwnd,base++,const_cast<char *> ("-"),x+4*(fw+bw),i*bw+1,bw,bw,O_DSY);
            sy[i]->attr.dwStyle &= ~WS_TABSTOP;
            sy[i]->Create();
            sy[i]->SetFont(set.font9);
        }
    }

    texturedef_t *currentTex;
    currentTex = texWindow->currentTexture;
    sE->SetInt((int)currentTex->shift[0]);
    tE->SetInt((int)currentTex->shift[1]);
    rE->SetInt((int)currentTex->rotate);
    sxE->SetFloat(currentTex->scale[0]);
    syE->SetFloat(currentTex->scale[1]);

    if (set.game_mode == 2)
    {
        CreateFlags();
        CreateContents();
        CreateValue();

        valEdit->SetInt(currentTex->value);
        SetFlags(currentTex->flags);
        SetContents(currentTex->contents);
    }

    SetCaption("");

    int leftSide = 90;
    if (set.game_mode != 2)
        leftSide = 2;

    applyFace = new WButton(hwnd, IDCB_APPLYFACE, const_cast<char *> ("ApplyFace"), leftSide, 30, 60, 16);
    applyFace->Create();
    applyFace->SetFont(set.font9);

    applyBrush = new WButton(hwnd, IDCB_APPLYBRUSH, const_cast<char *> ("ApplyBrush"), leftSide + 60 + 5, 30, 60, 16);
    applyBrush->Create();
    applyBrush->SetFont(set.font9);

    reset = new WButton(hwnd, IDCB_RESET, const_cast<char *> ("Reset"), leftSide + 60 + 60 + 10, 30, 60, 16);
    reset->Create();
    reset->SetFont(set.font9);

    applyWal = new WButton(hwnd, IDCB_APPLYWAL, const_cast<char *> ("Save to .Wal"), leftSide + 195, 30, 60, 16);
    applyWal->Create();
    applyWal->SetFont(set.font9);

    ontop = new WCheck(hwnd, IDCB_ONTOP, const_cast<char *> ("Keep on top"), leftSide + 295, 32, 100, 16);
    ontop->Create();
    ontop->SetFont(set.font9);
    ontop->SetCheck(TCWindow::topmost == hwnd);

    int width = 90;
    leftSide = t_left;
    int height = 15;
    t_top += 3;

    af = new WButton(hwnd, IDCB_ADDF, const_cast<char *> ("Add Flags"), leftSide, t_top, width,height);
    df = new WButton(hwnd, IDCB_DELF, const_cast<char *> ("Remove Flags"), leftSide, t_top+height, width,height);
    sf = new WButton(hwnd, IDCB_SETF, const_cast<char *> ("Set Flags"), leftSide, t_top+2*height, width,height);

    leftSide += 10;

    ac = new WButton(hwnd, IDCB_ADDC, const_cast<char *> ("Add Contents"), leftSide + width, t_top, width,height);
    dc = new WButton(hwnd, IDCB_DELC, const_cast<char *> ("Remove Contents"), leftSide + width, t_top+height, width,height);
    sc = new WButton(hwnd, IDCB_SETC, const_cast<char *> ("Set Contents"), leftSide + width, t_top+2*height, width,height);

    af->attr.dwStyle &= ~WS_TABSTOP;
    df->attr.dwStyle &= ~WS_TABSTOP;
    ac->attr.dwStyle &= ~WS_TABSTOP;
    dc->attr.dwStyle &= ~WS_TABSTOP;
    sf->attr.dwStyle &= ~WS_TABSTOP;
    sc->attr.dwStyle &= ~WS_TABSTOP;

    af->Create();
    df->Create();
    ac->Create();
    dc->Create();
    sf->Create();
    sc->Create();

    af->SetFont(set.font9);
    df->SetFont(set.font9);
    ac->SetFont(set.font9);
    dc->SetFont(set.font9);

    sf->SetFont(set.font9);
    sc->SetFont(set.font9);

    BrowserGame = set.game_mode;
}

void TSurfaceWindow::ApplyFace()
{
    CmFaceTexture();
}

void TSurfaceWindow::ApplyBrush()
{
    CmBrushTexture();
}

void TSurfaceWindow::SetContents()
{
    if (!set.Map_Read || !*flags)
        return;

    int bits = 0;

    for (int f=0 ; f<32 ; f++)
    {
        if (!contents[f])
            continue;

        int b = (contents[f]->GetCheck() == BST_CHECKED);
        bits |= (b << f);
    }
    texturedef_t *currentTex;
    currentTex = texWindow->currentTexture;
    currentTex->contents = bits;
    CmBrushContents(bits,0);
}

void TSurfaceWindow::AddContents()
{
    if (!set.Map_Read || !*flags)
        return;

    int bits = 0;
    for (int f=0 ; f<32 ; f++)
    {
        if (!contents[f])
            continue;

        int b = (contents[f]->GetCheck() == BST_CHECKED);
        bits |= (b << f);
    }

    texturedef_t *currentTex;
    currentTex = texWindow->currentTexture;
    currentTex->contents |= bits;
    CmBrushContents(bits,1);
}

void TSurfaceWindow::DelContents()
{
    if (!set.Map_Read || !*flags)
        return;

    int bits = 0;
    for (int f=0 ; f<32 ; f++)
    {
        if (!contents[f])
            continue;

        int b = (contents[f]->GetCheck() == BST_CHECKED);
        bits |= (b << f);
    }

    texturedef_t *currentTex;
    currentTex = texWindow->currentTexture;
    currentTex->contents &= ~bits;
    CmBrushContents(bits,-1);
}

void TSurfaceWindow::SetFlags()
{
    if (!set.Map_Read || !*flags)
        return;

    int bits = 0;
    for (int f=0 ; f < S_FLAGS ; f++)
    {
        if (!flags[f])
            continue;

        int b = (flags[f]->GetCheck() == BST_CHECKED);
        bits |= (b << f);
    }

    texturedef_t *currentTex;
    currentTex = texWindow->currentTexture;
    currentTex->flags = bits;
    CmBrushFlags(bits,0);
}

void TSurfaceWindow::AddFlags()
{
    if (!set.Map_Read || !*flags)
        return;

    int bits = 0;
    for (int f=0 ; f < S_FLAGS ; f++)
    {
        if (!flags[f])
            continue;

        WCheck *cell = flags[f];
        int b = (cell->GetCheck() == BST_CHECKED);
        bits |= (b << f);
    }
    texturedef_t *currentTex;
    currentTex = texWindow->currentTexture;
    currentTex->flags |= bits;
    CmBrushFlags(bits,1);
}

void TSurfaceWindow::DelFlags()
{
    if (!set.Map_Read || !*flags)
        return;

    int bits = 0;
    for (int f=0 ; f < S_FLAGS ; f++)
    {
        if (!flags[f])
            continue;

        int b = (flags[f]->GetCheck() == BST_CHECKED);
        bits |= (b << f);
    }

    texturedef_t *currentTex;
    currentTex = texWindow->currentTexture;
    currentTex->flags &= ~bits;
    CmBrushFlags(bits,-1);
}

void TSurfaceWindow::ApplyWal()
{
    // prompt then set them...
    // changes will take effect on 'rescan' no
    if (!set.Map_Read)
        return;

    char str[512];

    texturedef_t *currentTex;
    currentTex = texWindow->currentTexture;

    // see if .wal file even exists;
    Path path(const_cast<char *> ("%s\\%s%s.wal"), set.texture_path, currentTex->basepath, currentTex->texture);

    if (!path.Exists())
    {
        sprintf(str,"Couldn't locate [%s]...",(char*)path);
        MessageBox(hwnd,str,"BSP Set .Wal Attributes...",0);
        return;
    }

    FILE *fp = fopen(path,"r+b");
    if (!fp)
    {
        sprintf(str,"Couldn't open [%s]...",(char*)path);
        MessageBox(hwnd, str,"BSP Set .Wal Attributes...",0);
        return;
    }
    miptex2_t data;
    memset(&data,0,sizeof(miptex2_t));
    if (fread(&data,sizeof(miptex2_t),1,fp) != 1)
    {
        fclose(fp);
        sprintf(str,"Couldn't read miptex data from [%s]...",(char*)path);
        MessageBox(hwnd, str,"BSP Set .Wal Attributes...",0);
        return;
    }

    fseek(fp,0,SEEK_SET);
    data.flags    = currentTex->flags;
    data.contents = currentTex->contents;
    data.value    = currentTex->value;

    if (fwrite(&data,sizeof(miptex2_t),1,fp) != 1)
    {
        fclose(fp);
        sprintf(str,"Couldn't write updated miptex data for [%s]...",(char*)path);
        MessageBox(hwnd,str,"BSP Set .Wal Attributes...",0);
        return;
    }

    fclose(fp);

    // now find it in the regular "texture" browser and update the defaults
    // and based on those, load in the sky bits, etc.
}

void TSurfaceWindow::Reset()
{
    texWindow->ResetButton();
}

void TSurfaceWindow::GetFlags()
{
    int bits = 0;
    for (int f=0 ; f < S_FLAGS ; f++)
    {
        int b = (flags[f]->GetCheck() == BST_CHECKED);
        bits |= (b << f);
    }

    texturedef_t *currentTex;
    currentTex = texWindow->currentTexture;
    currentTex->flags = bits;

    char temp[64];
    sprintf(temp,"Flags %i Contents %i",currentTex->flags, currentTex->contents);

    status->SetText(temp,true);
}

void TSurfaceWindow::SetFlags(int bits)
{
    for (int f=0 ; f < S_FLAGS ; f++)
    {
        flags[f]->SetCheck(bits & (1 << f));
    }
}

void TSurfaceWindow::GetContents()
{
    int bits = 0;
    for (int f=0 ; f<32 ; f++)
    {
        if (!contents[f])
            continue;

        int b = (contents[f]->GetCheck() == BST_CHECKED);
        bits |= (b << f);
    }

    texturedef_t *currentTex;
    currentTex = texWindow->currentTexture;
    currentTex->contents = bits;

    char temp[64];
    sprintf(temp,"Flags %i Contents %i",currentTex->flags, currentTex->contents);

    status->SetText(temp,true);
}

void TSurfaceWindow::SetContents(int bits)
{

    for (int f=0 ; f<32 ; f++)
    {
        if (!contents[f])
            continue;

        int b = ((bits & (1 << f)) ? BST_CHECKED : BST_UNCHECKED);
        contents[f]->SetCheck(b);
    }
}

void TSurfaceWindow::CreateValue()
{
    int x = 2;
    int y = 30;
    int w = 40;
    int h = 13;

    valStatic = new WStatic(hwnd, -1, const_cast<char *> ("Value"), x, y, 40, h);
    valStatic->Create();
    valStatic->SetFont(set.font9);

    valEdit = new WEditSurf(hwnd, IDCE_SVALUE, const_cast<char *> ("0"), x + 40 + 2, y, w, h);
    valEdit->Create();
    valEdit->SetFont(set.font9);

}

void TSurfaceWindow::CreateFlags()
{
    int i = 0;
    int FlagsX = 2;
    int FlagsY = 60;

    int w = 80;
    int h = 12;
    int x = FlagsX;
    int y = FlagsY;

    flagsStatic = new WStatic(hwnd, -1, const_cast<char *> ("Flags"), FlagsX + 38, y - 15, w, h);
    flagsStatic->Create();
    flagsStatic->SetFont(set.font9);

    flags[0] = new WCheck(hwnd, IDCB_SFLAGBASE + i,const_cast<char *> ("Light"), x, y, w, h);
    y += h + 2;
    i++;
    flags[1] = new WCheck(hwnd, IDCB_SFLAGBASE + i,const_cast<char *> ("Slick"), x, y, w, h);
    y += h + 2;
    i++;
    flags[2] = new WCheck(hwnd, IDCB_SFLAGBASE + i,const_cast<char *> ("Sky"), x, y, w, h);
    y += h + 2;
    i++;
    flags[3] = new WCheck(hwnd, IDCB_SFLAGBASE + i,const_cast<char *> ("Warp"), x, y, w, h);
    y += h + 2;
    i++;
    flags[4] = new WCheck(hwnd, IDCB_SFLAGBASE + i,const_cast<char *> ("Trans33"), x, y, w, h);
    y += h + 2;
    i++;
    flags[5] = new WCheck(hwnd, IDCB_SFLAGBASE + i,const_cast<char *> ("Trans66"), x, y, w, h);
    y += h + 2;
    i++;
    flags[6] = new WCheck(hwnd, IDCB_SFLAGBASE + i,const_cast<char *> ("Flowing"), x, y, w, h);
    y += h + 2;
    i++;
    flags[7] = new WCheck(hwnd, IDCB_SFLAGBASE + i,const_cast<char *> ("Nodraw"), x, y, w, h);
    y += h + 2;
    i++;
    flags[8] = new WCheck(hwnd, IDCB_SFLAGBASE + i,const_cast<char *> ("Hint/Splitter"), x, y, w, h);
    y += h + 2;
    i++;
    flags[9] = new WCheck(hwnd, IDCB_SFLAGBASE + i,const_cast<char *> ("Skip"), x, y, w, h);
    for (i = 0; i < S_FLAGS; i++)
    {
        flags[i]->Create();
        flags[i]->SetFont(set.font9);
    }
}

void TSurfaceWindow::CreateContents()
{
    int i = 0;
    int ContentsX = 2 + 80 + 20;
    int ContentsY = 60;

    int w = 80;
    int h = 12;
    int x = ContentsX;
    int y = ContentsY;

    contentsStatic = new WStatic(hwnd, -1, const_cast<char *> ("Contents"), ContentsX+82, y - 15, w, h);
    contentsStatic->Create();
    contentsStatic->SetFont(set.font9);

    contents = new WCheck* [32];
    memset(contents,0,32*sizeof(WCheck*));

    contents[0] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Solid"), x, y, w, h);
    y += h + 2;
    i++;
    contents[1] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Window"), x, y, w, h);
    y += h + 2;
    i++;
    contents[2] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Aux"), x, y, w, h);
    y += h + 2;
    i++;
    contents[3] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Lava"), x, y, w, h);
    y += h + 2;
    i++;
    contents[4] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Slime"), x, y, w, h);
    y += h + 2;
    i++;
    contents[5] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Water"), x, y, w, h);
    y += h + 2;
    i++;
    contents[6] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Mist"), x, y, w, h);

    t_top = y + h + 2;
    t_left = x;

    x += w + 2;
    y = ContentsY;
    i = 16;

    contents[16] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Player Clip"), x, y, w, h);
    y += h + 2;
    i++;
    contents[17] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Monster Clip"), x, y, w, h);
    y += h + 2;
    i++;
    contents[18] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Current_0"), x, y, w, h);
    y += h + 2;
    i++;
    contents[19] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Current_90"), x, y, w, h);
    y += h + 2;
    i++;
    contents[20] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Current_180"), x, y, w, h);
    y += h + 2;
    i++;
    contents[21] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Current_270"), x, y, w, h);
    y += h + 2;
    i++;
    contents[22] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Current_Up"), x, y, w, h);

    x += w + 2;
    y = ContentsY;
    i++;

    contents[23] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Current_Down"), x, y, w, h);
    y += h + 2;
    i++;
    contents[24] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Origin"), x, y, w, h);
    y += h + 2;
    i++;
    contents[25] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Monster"), x, y, w, h);
    y += h + 2;
    i++;
    contents[26] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Corpse"), x, y, w, h);
    y += h + 2;
    i++;
    contents[27] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Detail"), x, y, w, h);
    y += h + 2;
    i++;
    contents[28] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Translucent"), x, y, w, h);
    y += h + 2;
    i++;
    contents[29] = new WCheck(hwnd, IDCB_SCONTENTSBASE + i,const_cast<char *> ("Ladder"), x, y, w, h);

    for (i = 0; i < 32; i++)
    {
        if (!contents[i])
            continue;

        contents[i]->attr.dwStyle &= ~WS_TABSTOP;
        contents[i]->Create();
        contents[i]->SetFont(set.font9);
    }
}

TSurfaceWindow::~TSurfaceWindow()
{
    for (int i = 0; i<2; i++)
    {
        delete s[i];
        delete t[i];
        delete r[i];
        delete sx[i];
        delete sy[i];
    }

    delete sE;
    delete tE;
    delete rE;
    delete sxE;
    delete syE;
    delete sl;
    delete tl;
    delete rl;
    delete sxl;
    delete syl;

    if (set.game_mode == 2)
    {
        delete[] contents;
    }

    delete flagsStatic;
    delete contentsStatic;

    delete valEdit;
    delete valStatic;

    delete applyFace;
    delete applyBrush;
    delete reset;
    delete applyWal;
    delete ontop;

    delete af;
    delete ac;
    delete df;
    delete dc;
    delete sc;
    delete sf;
}
void TSurfaceWindow::MouseUp(int operation)
{
    ReleaseCapture();
    KillTimer(surfaceWindow->hwnd,operation);
}
void TSurfaceWindow::MouseDown(int operation)
{
    SetCapture(hwnd);
    SetTimer(hwnd,operation,50,0);
    return;
}
void TSurfaceWindow::MouseFunc(int operation)
{
    switch (operation)
    {
    case O_UX:
        texWindow->currentTexture->shift[0] += set.tex_shiftstep;
        sE->SetInt((int)texWindow->currentTexture->shift[0]);
        break;
    case O_UY:
        texWindow->currentTexture->shift[1] += set.tex_shiftstep;
        tE->SetInt((int)texWindow->currentTexture->shift[1]);
        break;
    case O_DX:
        texWindow->currentTexture->shift[0] -= set.tex_shiftstep;
        sE->SetInt((int)texWindow->currentTexture->shift[0]);
        break;
    case O_DY:
        texWindow->currentTexture->shift[1] -= set.tex_shiftstep;
        tE->SetInt((int)texWindow->currentTexture->shift[1]);
        break;
    case O_UR:
        texWindow->currentTexture->rotate += set.tex_rotatestep;
        while (texWindow->currentTexture->rotate > 360.0)
        {
            texWindow->currentTexture->rotate -= 360.0;
        }
        rE->SetInt((int)texWindow->currentTexture->rotate);
        break;
    case O_DR:
        texWindow->currentTexture->rotate -= set.tex_rotatestep;
        while (texWindow->currentTexture->rotate < 0.0)
        {
            texWindow->currentTexture->rotate += 360.0;
        }
        rE->SetInt((int)texWindow->currentTexture->rotate);
        break;
    case O_USX:
        texWindow->currentTexture->scale[0] += set.tex_scalestep;
        if (fabs(texWindow->currentTexture->scale[0]) < BAD_SCALE)
            texWindow->currentTexture->scale[0] = set.tex_scalestep;
        sxE->SetFloat(texWindow->currentTexture->scale[0]);
        break;
    case O_DSX:
        texWindow->currentTexture->scale[0] -= set.tex_scalestep;
        if (fabs(texWindow->currentTexture->scale[0]) < BAD_SCALE)
            texWindow->currentTexture->scale[0] = -set.tex_scalestep;
        sxE->SetFloat(texWindow->currentTexture->scale[0]);
        break;
    case O_USY:
        texWindow->currentTexture->scale[1] += set.tex_scalestep;
        if (fabs(texWindow->currentTexture->scale[1]) < BAD_SCALE)
            texWindow->currentTexture->scale[1] = set.tex_scalestep;
        syE->SetFloat(texWindow->currentTexture->scale[1]);
        break;
    case O_DSY:
        texWindow->currentTexture->scale[1] -= set.tex_scalestep;
        if (fabs(texWindow->currentTexture->scale[1]) < BAD_SCALE)
            texWindow->currentTexture->scale[1] = -set.tex_scalestep;
        syE->SetFloat(texWindow->currentTexture->scale[1]);
        break;
    }
}
