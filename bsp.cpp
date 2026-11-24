// BSP Copyright 1996 Yahn W. Bernier

#include "global.h"

int normalExit = 0;
int CaptionHeight;							//height of tiny caption
HFONT g_hCaptionFont;						//font for all tiny captions
int KillAllWindows = 0;                      // During program shutdown, okay to close docs?

map *copymap;                                // Temporary map using during copy operations.

TBSPApp *frame = 0;			                // BSP windows.  Main frame
TBSPWindow *client = 0;						// MDI client.
TTextureWindow *texWindow = 0;				// Texture browser
TXYViewWindow **xyWindow = 0;				// 2D View
TEditWindow *editWindow = 0;				// 3D View
EntityView *kpWindow = 0;					// Entity Browser
TGroupWindow *groupWindow = 0;				// Group Browser
THelpWindow *helpWindow = 0;				// Help Window
TMonsterWindow *monsterWindow = 0;			// Monster browser
TSurfaceWindow *surfaceWindow = 0;			// Surface property window.
BSPConsole *wconsole = 0;
PaletteWindow *palettewindow = 0;

SetBrush **besthits = NULL;
int numhits;

char fName[128];
//todo - global variables are the duck!
namespace Bsp
{
int cur = 0;                            // Cur and Lst flipflop during clipping ops.
int lst = 1;
}
vec3_t *CP[2]; 							// array to hold clipped pts...
vec3_t SPoints[CPT];                    // Point arrays for clipping.
POINT FSPoints[CPT];
vec3_t *tpts;
float *dists;							// Classification of point to plane
float *sides;

RECT EditClip, RenderClip;				// 2d and 3d window sizes and clip rects.
SIZE EditSize, RenderSize;

/*
===============
BSPHelp

Diplays the help window if it is not visible.  Sets the title and caption.
===============
*/
void BSPHelp(char *caption, char *text)
{
    helpWindow->SetCaption(caption);
    helpWindow->helpEdit->SendMessage(WM_SETTEXT,0,0);
    helpWindow->helpEdit->SendMessage(EM_REPLACESEL,0,(LPARAM)text);
    // Bring to foreground and unminimize it.
    if (!helpWindow->IsVisible())
        helpWindow->ShowWindow();
    helpWindow->helpEdit->SendMessage(EM_LINESCROLL,-0xFFFF,-0xFFFF);	//scroll to top
    helpWindow->SetFocus();
}

/*
=================
BSPHelpAdd

Appends text to the help window
=================
*/
void BSPHelpAdd(char *text)
{
    // Must be visible, or just ignore.
    if (!helpWindow || !helpWindow->IsValid())
        return;

    helpWindow->helpEdit->SendMessage(EM_REPLACESEL,0,(LPARAM)text);
    if (!helpWindow->IsVisible())
        helpWindow->ShowWindow();

    helpWindow->helpEdit->SendMessage(EM_LINESCROLL,-0xFFFF,-0xFFFF);	//scroll to top
    helpWindow->SetFocus();
}

void BSPHelpAddV(char *text, ...)
{
    va_list argptr;
    va_start (argptr,text);
    int len = _vscprintf(text, argptr) + 1;
    if(len <= 1) return;
    char *str = new char[len];
    _vsnprintf (str, len, text,argptr);
    va_end (argptr);
    BSPHelpAdd(str);
    delete [] str;
}


/*
=============
MapChanged

This function must be called anytime the current map/document is changed!!!
=============
*/
void MapChanged()
{
    if (!set.Map_Read || !set.nummaps)    // No maps yet.
        return;

    map *m = map_i[set.curmap];

    texWindow->ChangeToMap(m);   // Switch texture browser
    texWindow->SetUpMap(m);

    // Reset surface window caption.
    surfaceWindow->SetCaption("Surface");

    // Clear out and reload the group window.
    if (groupWindow)
    {
        groupWindow->GroupList->ClearList();
        groupWindow->RedrawReload();
    }

    // If the new map is for a different game, switch the entity browser
    if (kpWindow)
    {
        if (kpWindow->BrowserGame != m->mapType)
            // this sets the new Current Entity...
            kpWindow->SwitchMode(m->mapType);
        else
            // Otherwise, just deselect entities.
            kpWindow->newCurrentEntity(false);
    }

    // Update the menu control in the status bar.
    char fname[128];
    // Get map name
    ExtractFileBase(m->filename,fname,true);
    status->SetText(STATUS_MAP, fname);
    status->SizePartToText(STATUS_MAP);

    // If we are displaying leak or portal trails, disable them.
    CmStopLeaks();
    CmStopPortals();

    //  Add items to "Make" dropdown list.
    client->PopulateMake();
}

//sets window caption height
void SetCaptionHeight()
{
    CaptionHeight = (GetSystemMetrics(SM_CYCAPTION) * set.caption_percent) / 100 - GetSystemMetrics(SM_CYBORDER);
}
/*
==============
TBSPWindow

MDI client window.  Handles most of BSP's UI functionality
==============
*/
TBSPWindow::TBSPWindow(HWND parent)
    : MDIClient(parent)
{
    SetCaptionHeight();

    g_hCaptionFont = GetFont(set.caption_font,-set.caption_font_size,0,0);

    memset(ExporterNames, 0, MAX_EXPORTERS*sizeof(char*));	// .bat files inserted into export menu.
    numExporters  = 0;                  // Number of files.
    itemsAdded    = false;              // Flag whether items were added
    holdMenu      = NULL;               // Menu to add to.

    InitRecent();						// load recent files

    set.Map_Read = 0;                  // Set up maps.

    map_i = new map *[MAX_MAPS];        // Fixme, use a linked list

    memset(map_i, 0, MAX_MAPS * sizeof(map *));

    copymap = new map; 					  // Allocate the copy map

    set.curmap     = 0;                // Index of current map.
    set.nummaps    = 0;                // Counter of current maps.

    KillAllWindows  = 0;

    set.curxy      = 0;                // Current 2D View
    set.xyViews    = 0;                // # of 2D Views.

    xyWindow = new TXYViewWindow *[MAX_XY];
    memset(xyWindow, 0, MAX_XY * sizeof(TXYViewWindow *));

    // Allocate points used during clipping operations
    // NOTE:  Some of these may be obsolete now.
    tpts     = new vec3_t[CPT];
    dists    = new float[CPT];
    sides    = new float[CPT];
    CP[0]  = new vec3_t[CPT];
    CP[1]  = new vec3_t[CPT];

    // Allocate transfer buffers to use to communicate between BSP and the various
    //  dialog boxes (make arch, etc.).

    // Make Stairs.
    memset(&stairxfer,0,sizeof(stairxfer));
    stairxfer.HGap = 64;
    stairxfer.VGap    = 0;
    stairxfer.NumTurns = 2.0;
    stairxfer.Dist    = 20;
    stairxfer.NumSteps = 12;
    stairxfer.RegularButton = BST_CHECKED;
    stairxfer.SpiralButton  = BST_UNCHECKED;
    stairxfer.BlockButton   = BST_UNCHECKED;
    stairxfer.GoesDown      = BST_UNCHECKED;
    stairxfer.XAxis         = BST_UNCHECKED;

    // Arch
    memset(&archxfer,0,sizeof(archxfer));
    strcpy(archxfer.Stones  ,"8");
    strcpy(archxfer.width   ,"20");
    archxfer.XAxis = BST_UNCHECKED;

    // Wedge
    memset(&wedgexfer,0,sizeof(wedgexfer));
    strcpy(wedgexfer.Axis  ,"0");
    wedgexfer.flip = BST_UNCHECKED;

    // Lights
    strcpy(lightxfer.NumX    ,"2");
    strcpy(lightxfer.NumY    ,"2");
    strcpy(lightxfer.NumZ    ,"2");
    strcpy(lightxfer.LightVal,"150");
    lightxfer.boxdata.Clear();

    // Rotate Brush
    memset(&rotatexfer,0,sizeof(rotatexfer));
    strcpy(rotatexfer.Roll,"0");
    strcpy(rotatexfer.Pitch,"0");
    strcpy(rotatexfer.Yaw,"0");

    // Copy Brush
    memset(&copyxfer,0,sizeof(copyxfer));
    strcpy(copyxfer.NumX   ,"16");
    strcpy(copyxfer.NumY   ,"16");
    strcpy(copyxfer.NumZ   ,"0");
    strcpy(copyxfer.NumCopies,"1");
    copyxfer.MoveOnly = BST_UNCHECKED;

    // Sphere
    memset(&spherexfer,0,sizeof(spherexfer));
    strcpy(spherexfer.HStrips,"5");
    strcpy(spherexfer.VStrips,"5");
    strcpy(spherexfer.Inner  ,"100");
    strcpy(spherexfer.Outer  ,"200");
    strcpy(spherexfer.InnerPercent, "50");
    spherexfer.TakeSeed = BST_UNCHECKED;

    // Revolve Object
    // 50 % done, but not enabled.  Needs a UI for specifying points and some debugging.
    memset(&revolvexfer,0,sizeof(revolvexfer));
    strcpy(revolvexfer.HStrips,"5");
    strcpy(revolvexfer.Inner  ,"100");
    strcpy(revolvexfer.Outer  ,"200");

    // Merge Brushes
    sprintf(mergexfer.Face      ,"%f", set.max_face_gap);
    sprintf(mergexfer.Normal    ,"%f", set.max_normal_delta);
    sprintf(mergexfer.Point     ,"%f", set.max_point_distance);
    sprintf(mergexfer.Convexity ,"%f", set.on_plane_epsilon);
    mergexfer.SaveFirst      = BST_CHECKED;
    mergexfer.SelectedOnly   = BST_CHECKED;
    mergexfer.TextureCompare = BST_CHECKED;
    mergexfer.MatchingFlags  = BST_CHECKED;

    // Fill in combobox on call b/c it depend on eclass lists
    // Select Entities by ...
    strcpy(selectxfer.Key,"");
    strcpy(selectxfer.Value,"");
    selectxfer.SelectEntity = BST_CHECKED;
    selectxfer.SelectKey    = BST_UNCHECKED;
    selectxfer.SelectValue  = BST_UNCHECKED;

    // Cylindar
    memset(&cylxfer,0,sizeof(cylxfer));
    strcpy(cylxfer.Sides,"6");
    strcpy(cylxfer.Strips,"5");
    strcpy(cylxfer.Inner,"64");
    strcpy(cylxfer.Outer,"128");
    strcpy(cylxfer.InnerPercent, "50");
    cylxfer.TakeSeed = BST_UNCHECKED;

    // N-sided brush
    memset(&nxfer,0,sizeof(nxfer));
    strcpy(nxfer.Sides,"5");
    strcpy(nxfer.Radius,"64");
    nxfer.TakeSeed = BST_UNCHECKED;

    // Scale brush
    memset(&scalexfer,0,sizeof(scalexfer));
    strcpy(scalexfer.ScaleX,"1.0");
    strcpy(scalexfer.ScaleY,"1.0");
    strcpy(scalexfer.ScaleZ,"1.0");

    // Pyramid
    memset(&pyramidxfer,0,sizeof(pyramidxfer));
    strcpy(pyramidxfer.Sides,"5");
    strcpy(pyramidxfer.Radius,"64");
    strcpy(pyramidxfer.Height,"64");
    pyramidxfer.TakeSeed = BST_UNCHECKED;

    // Create x, y, z sized brush
    memset(&createxfer,0,sizeof(createxfer));
    strcpy(createxfer.X,"0");
    strcpy(createxfer.Y,"0");
    strcpy(createxfer.Z,"0");
    strcpy(createxfer.DX,"64");
    strcpy(createxfer.DY,"64");
    strcpy(createxfer.DZ,"64");

    // Set default base level and height for creating brushes in 2d view.
    strcpy(setxfer.Base,"0");
    strcpy(setxfer.Height,"80");

    // Allocate space for hit brushes.
    besthits = new SetBrush *[MAX_HITS];
    memset(besthits,0,MAX_HITS*sizeof(SetBrush *));
    hitSelected = new int[MAX_HITS];
    memset(hitSelected,0,MAX_HITS*sizeof(int));
    numhits = 0;

    set.curCfg = 0;                      // Which window settings is current?
}

/*
=============
~TBSPWindow

Destructor
=============
*/
TBSPWindow::~TBSPWindow()
{
    if (itemsAdded)           // Clean up menus.
    {
        RemoveExporterItemsFromMenu(holdMenu);
        RemoveExportNames();
    }

    KillAllWindows = 1;       // Shutting down for good.
    //////////////////////////
    delete[] CP[0];
    delete[] CP[1];
    delete[] tpts;
    delete[] dists;
    delete[] sides;
    delete[] besthits;
    besthits = NULL;
    delete[] hitSelected;
    hitSelected = NULL;

    DeleteObject(g_hCaptionFont);

    // Go thru .maps and delete all objects...
    if (map_i)
    {
        for (int i = 0; i < MAX_MAPS; i++)
        {
            delete map_i[i];
            map_i[i] = NULL;
        }
        delete[] map_i;
    }

    // Kill the copymap.
    delete copymap;

    delete texWindow;
    texWindow = 0;
    for (int c = 0; c < set.xyViews; c++)
    {
        delete xyWindow[c];
        xyWindow[c] = NULL;
    }
    delete[] xyWindow;
    xyWindow     = 0;
    delete kpWindow;
    kpWindow     = 0;
    delete editWindow;
    editWindow   = 0;
    delete groupWindow;
    groupWindow  = 0;
    delete surfaceWindow;
    surfaceWindow  = 0;
    delete palettewindow;
    palettewindow = 0;
}



/*
TODO: try this to remove mdi scrollbars. TBSPWindow needs to be subclassed first
LRESULT TBSPWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_NCCALCSIZE:
		if(GetWindowLong(hwnd,GWL_STYLE) & (WS_HSCROLL|WS_VSCROLL))
			SetWindowLong(hwnd,GWL_STYLE,GetWindowLong(hwnd,GWL_STYLE) & ~(WS_HSCROLL|WS_VSCROLL));
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
}
*/

/*
=============
CmBar1 & 2

Show/Hide the 1st and 2nd button bars
=============
*/
void CmToggleBar(int n)	//n is zero based index of bar
{
    if(!rebar)
        return;
    if (frame->numbars < 0 || frame->numbars <= n)
        return;
    cbars[n]->visible ^= 1;	//swap visible flag of toolbar
    if(cbars[n]->visible)	//show/hide rebar band based on toolbar flag
        rebar->Show(n);
    else
        rebar->Hide(n);
}
void CmBar1()
{
    CmToggleBar(0);
}
void CmBar2()
{
    CmToggleBar(1);
}

/*
==============
CmToggleGridStyle

grid_style 0 = lines, 1 = dots.
==============
*/
void CmToggleGridStyle()
{
    set.grid_style ^= 1;
    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Grid style toggled",true);
}

/*
===============
CmExit

User clicked close box or File | Exit
===============
*/
void CmExit()
{
    frame->SendMessage(WM_CLOSE,0,0);
}
void CmExitFinal()
{
    // Cycle though maps, query to save if dirty;

    normalExit = 1;          // Not crashing.

    int okay = 1;
    for (int i=0; (i < set.nummaps) && okay; i++)
    {
        if (map_i[i]->dirty)
        {

            char prompt[128];
            sprintf(prompt,"Save changes to map %s?", map_i[i]->filename);

            int retval = MessageBox(client->hwnd, prompt, "BSP - Save Changes?", MB_YESNOCANCEL | MB_ICONQUESTION);

            if (retval == IDYES)
            {
                CmSave();
                okay = 1;
            }
            else if (retval == IDNO)
            {
                okay = 1;
            }
            else       // Cancelled
            {
                okay = 0;
            }
        }
    }

    // If not cancelled, flag that all are ready to close and post a quit message
    if (okay)
    {
        KillAllWindows = 1;
        PostQuitMessage(0);   // Right way to kill a windows app.
    }
}

/*
===============
CmClose

Close current map.
===============
*/
void CmClose()
{
    if (!set.Map_Read || !set.nummaps)    // Must be in a map.
        return;

    map *m = map_i[set.curmap];
    int okay = 1;
    if (m->dirty)
    {
        char prompt[128];
        sprintf(prompt,"Save changes to map %s?",m->filename);

        int retval = MessageBox(client->hwnd, prompt, "BSP - Save Changes?", MB_YESNOCANCEL | MB_ICONQUESTION);

        if (retval == IDYES)
        {
            CmSave();
        }
        else if (retval == IDCANCEL)
        {
            okay = 0;
        }
    }

    if (okay)
    {
        set.nummaps--;                       // Kill current map.
        delete map_i[set.curmap];
        if (set.curmap == set.nummaps)
        {
            map_i[set.curmap] = NULL;
        }
        else
        {
            map_i[set.curmap]  = map_i[set.nummaps];
            map_i[set.nummaps] = NULL;
        }

        set.curmap = 0;                     // Switch to first map.
        m = map_i[set.curmap];

        if (!set.nummaps)
        {
            // no maps left...
            status->SetText(STATUS_MAP, "N/A");   // No maps.
            status->SizePartToText(STATUS_MAP);
            set.Map_Read = 0;                      // This little flag controls most things...

            frame->SetCaption("");                                // Set caption
            groupWindow->GroupList->ClearList();                     // Clear groups.
            client->HideAllWindows();                                // Hide windows.
        }
        else
        {
            MapChanged();                                            // Change data.

            frame->SetCaption(m->filename);         // New current map.
            editWindow->SetCaption(m->filename);
            set.redrawedit = 1;                                     // Force redraw of both views.
            set.redrawxy = 1;
            Show_Frame("Map closed...",true);                        // Redraw and set caption.
        }
    }
}

/*
==============
CmAbout

==============
*/
void CmAbout()
{
    TAboutDlg(client->hwnd).Execute();
}

/*
==============
SetupWindow

UI specific setup
==============
*/

void TBSPWindow::SetupWindow()
{
    //LoadWindowPositions();     // Load window positions from .ini.
    set.curCfg = 0;

    set.angstep = 2.f*(float)M_PI/set.steps_per_turn;   // Turning turns you this many degrees per notch.

    entity_classes_i = new EntityClassList;
    // Scan in entities from source files.
    entity_classes_i->initForSourceDirectory(set.entity_directory);

    // Create windows.


    helpWindow = new THelpWindow(hwnd, const_cast<char *> ("BSP Help..."));
    helpWindow->Create(false);

    editWindow	= new TEditWindow    (hwnd, const_cast<char *> ("[?.map]"));
    texWindow	= new TTextureWindow (hwnd, const_cast<char *> ("Textures"));
    kpWindow	= new EntityView     (hwnd, const_cast<char *> ("Entities"));
    wconsole	= new BSPConsole	 (hwnd, const_cast<char *> ("Console"));

    texWindow   ->Create(false);
    kpWindow    ->Create(false);
    editWindow	->Create(false);
    wconsole	->Create(false);

    texWindow   ->SetIcon(IDI_TEXTURE);
    kpWindow    ->SetIcon(IDI_ENTITY);
    editWindow  ->SetIcon(IDI_THREED);
    wconsole	->SetIcon(IDI_CONSOLE);

    char *startuplog = (char*)file_get_contents(const_cast<char *> (BSP_LOG),0);
    if(startuplog)
        sysprintf(startuplog);
    delete [] startuplog;

    // Create 2d views.
    xyWindow[0] = new TXYViewWindow( hwnd, const_cast<char *> ("Map View"), Type_XY);
    xyWindow[0]->index = 0;
    xyWindow[0]->Create(false);
    xyWindow[1] = new TXYViewWindow( hwnd, const_cast<char *> ("Map View"), Type_XZ);
    xyWindow[1]->index = 1;
    xyWindow[1]->Create(false);
    xyWindow[2] = new TXYViewWindow( hwnd, const_cast<char *> ("Map View"), Type_YZ);
    xyWindow[2]->index = 2;
    xyWindow[2]->Create(false);
    set.xyViews = 3;

    xyWindow[0]->SetIcon(IDI_XY);
    xyWindow[1]->SetIcon(IDI_XY);
    xyWindow[2]->SetIcon(IDI_XY);

    groupWindow = new TGroupWindow(hwnd,const_cast<char *> ("Groups"));
    groupWindow->Create(false);
    groupWindow->SetIcon(IDI_GROUP);

    surfaceWindow = new TSurfaceWindow(hwnd, const_cast<char *> ("Surface Property Window..."));
    surfaceWindow->Create(false);
    //surfaceWindow->ShowWindow(SW_HIDE);
    surfaceWindow->SetIcon(IDI_SURFACE);




    // Kill off splash window now that we've created UI windows.
    delete splash;
    splash = NULL;

    SetWindowPositions();//CmWindowPosition();       // Move the windows to the right slots.
    HideAllWindows();         // Hide them.
}

/*
===============
CmMonsterWindow

Show the monster window
===============
*/
void CmMonsterWindow()
{
    if (monsterWindow && monsterWindow->IsValid())
        return;

    delete monsterWindow;

    monsterWindow = new TMonsterWindow(client->hwnd, const_cast<char *> ("Monster Browser"));
    monsterWindow->Create(false);
    monsterWindow->ShowWindow(SW_RESTORE);
    monsterWindow->SetFocus();
}

/*
================
CmBrushWindow

Show the surface property window.  Get updated info on current texture.
================
*/
void CmBrushWindow()
{
    // Toggle off
    if (surfaceWindow->IsWindowVisible())
    {
        surfaceWindow->ShowWindow(SW_HIDE);
        status->SetText("Surface Window Hidden...",true);
    }
    else
    {
        // Show it.
        surfaceWindow->ShowWindow(SW_RESTORE);
        surfaceWindow->SetFocus();
        surfaceWindow->SetCaption("Surface");
        // Repaint small caption.
        surfaceWindow->DoNCPaint();
        texWindow->SetEditBoxes();

        // Status bar.
        status->SetText("Surface Window Shown...",true);
    }
}

/*
===============
CmSaveWindowArrangement

Save current arrangement to disk
===============
*/
void CmSaveWindowArrangement()
{
    SaveArrangement();
}

/*
===============
CmSetColors

Show color dialog and handle changes
===============
*/
void CmSetColors()
{
    BspSettingsDlg(client->hwnd, const_cast<char *> ("Colors")).Execute();

    texWindow->RedrawContents();
    groupWindow->Invalidate();

    // Force views to redraw for new colors.
    set.redrawedit = 1;
    set.redrawxy   = 1;
    Show_Frame("Colors set...",true);
}

/*
=================
CmSetGame

Directory dialog
UNUSED?  This is done in setup.exe and may not be functional right now
==================
*/
void CmSetGame()
{
    BspSettingsDlg(client->hwnd, const_cast<char *> ("Game")).Execute();

    set.redrawedit = 1;
    set.redrawxy = 1;
    texWindow->RedrawContents();
    groupWindow->Invalidate();

    Show_Frame("Game options set...",true);
}

void CmMenuRefresh()
{
    menu_t *menu = Menu_Find(const_cast<char *> (MENU_BSP_MAIN));
    if(menu)
    {
        Menu_Create(menu);
        SetMenu(frame->hwnd, menu->hMenu);
        DrawMenuBar(frame->hwnd);
    }
}
void CmKBSettings()
{
    KeyboardSettingsDlg(client->hwnd).Execute();
    Show_Frame("Keyboard set...",true);
}
void CmBspSettings()
{
    BspSettingsDlg(client->hwnd, 0).Execute();
    // Force views to redraw for new colors.
    set.redrawedit = 1;
    set.redrawxy   = 1;
    texWindow->RedrawContents();
    groupWindow->Invalidate();
    editWindow->RedrawContents();
    Show_Frame("Settings set...",true);
}
void CmWriteConfig()
{
    if(!Cset_SaveConfig(const_cast<char *> ("settings\\bsp.cfg"), false))
        Show_Frame("Writing settings\\bsp.cfg failed!",true);
    else
        Show_Frame("settings\\bsp.cfg written...",true);
}
void CmWriteGameConfig()
{
    char gamesettings[MAX_PATH];
    sprintf(gamesettings,"%s\\game.cfg",set.game_directory);

    if(!Cset_SaveConfig(gamesettings,true))
        Show_Frame("Writing game.cfg failed!",true);
    else
        Show_Frame("game.cfg written...",true);
}
void CmShowKeyMappings()
{
    KBSettings::ShowKeys(acceltable);
    Show_Frame("Keyboard mappings displayed...",true);
}

/*
=================
CmUndo

Undo last "saved" operation
If you can't figure out the undo system, let me know.  It's not very good.
Dr. Dobbs has a really good article on how to implement it correctly.
Once you understand the workings here, you should do that, it will allow unlimited undo then.
=================
*/
void CmUndo()
{
    if (!set.Map_Read)
        return;

    map *cm;                    // Copy map.
    map *newm;                  // Replacement map.

    cm = map_i[set.curmap];    // Map we are undoing from
    cm->Undo(cm, &newm);        // Perform undo and put results in newm.
    if (newm)                   // If OK, kill old map and replace with newm.
    {
        delete cm;
        map_i[set.curmap] = newm;
    }

    set.redrawedit = 1;
    set.redrawxy = 1;

    groupWindow->Invalidate();  // Force redraw of group window, too.
    Show_Frame("Undo...",true);
}

/*
============
CmUndoSave

Save current state of map so next operation can be "undone" even if I forgot to
code a save for undo func.
============
*/
void CmUndoSave()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->saveForUndo(const_cast<char *> ("Saving for undo..."), -1); // -1 = force whole map

    set.redrawedit = 0;
    set.redrawxy = 0;
    Show_Frame("Saved for undo...",false);
}
//CmClipAutoFlip
void CmClipAutoFlip()
{
    set.clipper_autoflip ^= 1;
    Show_Frame("Clipper auto flip mode toggled...",false);
}
//CmClippersStay
void CmClippersStay()
{
    set.clippers_stay ^= 1;
    Show_Frame("Clippers Stay Visible toggled...",false);
}
//CmClipMode0
void CmClipMode0()
{
    set.clipper_mode = 0;
    Show_Frame("Clipper mode set to Normal...",false);
}
//CmClipMode1
void CmClipMode1()
{
    set.clipper_mode = 1;
    Show_Frame("Clipper mode set to Axial...",false);
}


/*
==============
CmFlipClipper

Flip clip plane orientation
==============
*/
void CmFlipClipper()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];

    m->saveForUndo(NULL,UNDO_NOUNDO);   // Not undoable
    m->clipper_i->flipNormal();         // Flip it
    set.redrawedit = 1;                                 // redraw.
    set.redrawxy = 1;
    Show_Frame("Clipper flipped...",true);
}

/*
================
CmCarve

Perform the clipping operation.
================
*/
void CmCarve()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    if (set.undo_clipper)    // Are we undoing clip operations.
        m->saveForUndo(const_cast<char *> ("Carve"), UNDO_FORCEWHOLE);   // Save entire map.
    else
        m->saveForUndo(NULL, UNDO_NOUNDO);          // No undo.

    // Do it.
    m->clipper_i->carve();
    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Carved...",true);
}

/*
=============
CmSplit

Split brushes along clip plane
=============
*/
void CmSplit()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];

    if (set.undo_clipper)
        m->saveForUndo(const_cast<char *> ("Split"), UNDO_FORCEWHOLE);
    else
        m->saveForUndo(NULL,UNDO_NOUNDO);

    // Are we supposed to ask?
    if (!set.querysplits)
    {
        int retval = MessageBox(client->hwnd, "Leave both brushes selected?", "BSP - Split", MB_YESNO | MB_ICONQUESTION);
        m->clipper_i->split(retval == IDYES);
    }
    else
    {
        m->clipper_i->split(set.querysplits == 1);
    }

    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Split...",true);
}

/*
=============
CmClipCurFace

Create three clip points from current face.
=============
*/
void CmClipCurFace()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];

    if (m->numSelected() < 1)
        return;

    SetBrush *b = m->selectedBrush();

    if (set.undo_clipper)
        m->saveForUndo(const_cast<char *> ("Clip Current Face"), UNDO_FORCEWHOLE);
    else
        m->saveForUndo(NULL,UNDO_NOUNDO);

    VectorCopy(b->currentFace->planepts[0], m->clipper_i->pos[0]);
    VectorCopy(b->currentFace->planepts[1], m->clipper_i->pos[1]);
    VectorCopy(b->currentFace->planepts[2], m->clipper_i->pos[2]);
    m->clipper_i->num = 3;
    m->curclippoint = 0;


    if(set.animate_clip_points)
    {
        //animate clip points in each xy window
        for (int i = 0; i < set.xyViews; i++)
        {
            KillTimer(xyWindow[i]->hwnd,TIMER_CLIP);
            SetTimer(xyWindow[i]->hwnd,TIMER_CLIP,50,0);
        }
    }

    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Clip current face...",true);
}
/*
==============
CmMoveUp

Move current clip point up in z (height) dir.
Move by gridsize amount.
==============
*/
void CmClipMoveUp()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];

    char outstr[80];
    sprintf(outstr,"Clip point %i moved up...",m->curclippoint);
    m->saveForUndo(NULL,UNDO_NOUNDO);
    m->clipper_i->pos[m->curclippoint][2] += set.gridsize;
    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame(outstr,true);
}

void CmClipMoveDown()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];

    char outstr[80];
    sprintf(outstr,"Clip point %i moved down...",m->curclippoint);
    m->saveForUndo(NULL,UNDO_NOUNDO);
    m->clipper_i->pos[m->curclippoint][2] -= set.gridsize;
    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame(outstr,true);
}
/*
=================
CmCenter

Center view on map center point. (2d and 3d)
=================
*/
void CmCenter()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];

    m->saveForUndo(NULL,UNDO_NOUNDO);

    // Get the current center point.
    vec3_t ctr;
    m->getCenter(ctr);

    // Copy to 3d and 2d center positions.
    VectorCopy(ctr, m->eye[m->cureye]);
    VectorCopy(ctr, xy_eye);

    // Redraw.
    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Centering view...",true);
}

/*
=================
CmSubtract

CSG Subtract selected brushes from rest of current entity (generally world)
=================
*/
void CmSubtract()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    m->saveForUndo(const_cast<char *> ("Subtract"), UNDO_FORCEWHOLE);

    // Subtract
    m->subtractSelection();

    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Subtracting selection...",true);
}

/*
==================
CmDelete

Delete selected brushes from map.
==================
*/
void CmDelete()
{
    if (set.Map_Read)
    {
        map *m = map_i[set.curmap];
        m->saveForUndo(const_cast<char *> ("Delete"), UNDO_BRUSHES);

        // Make all selected delete.
        m->makeSelectedPerform(SEL_REMOVE);

        set.redrawedit = 1;
        set.redrawxy = 1;
        Show_Frame("Deleting...",true);
    }
}

/*
=================
CmSnapPlanes

Snap all selected brushes' planes to grid
=================
*/
void CmSnapPlanes()
{
    if (set.Map_Read)
    {
        map *m = map_i[set.curmap];
        m->saveForUndo(const_cast<char *> ("Snap"), UNDO_BRUSHES);

        // Snap them.
        m->snapSelected(SNAP_PLANES);

        set.redrawxy = 1;
        set.redrawedit = 1;
        Show_Frame("Made selected snap to grid...",true);
    }
}

/*
=================
CmSnap

Snap selected brushes (axial sides, or near axial) to grid
=================
*/
void CmSnap()
{
    if (set.Map_Read)
    {
        map *m = map_i[set.curmap];
        m->saveForUndo(const_cast<char *> ("Snap"), UNDO_BRUSHES);

        // Snap them.
        m->snapSelected(SNAP_SELF);

        set.redrawxy = 1;
        set.redrawedit = 1;
        Show_Frame("Made selected snap to grid...",true);
    }
}

/*
=================
CmSnapBack

toggles snap_back. when set, dragged edges snap to grid.
=================
*/
void CmSnapBack()
{
    set.snap_back ^= 1;
    Show_Frame("Snap Back toggled...",false);
}
/*
=================
CmCopyMove

Show copy/move dialog and perform copying
=================
*/
void CmCopyMove()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    if (!m->numSelected())
    {
        Show_Frame("At least one brush must be selected to copy from...",false);
        return;
    }

    char outstr[80];

    // Run dialog...
    TCopyDlg dialog(client->hwnd,&client->copyxfer);
    if (dialog.Execute() == IDOK)
    {
        int move = (client->copyxfer.MoveOnly == BST_CHECKED);
        int x    = atoi(client->copyxfer.NumX);
        int y    = atoi(client->copyxfer.NumY);
        int z    = atoi(client->copyxfer.NumZ);
        int copies = atoi(client->copyxfer.NumCopies);
        copies = max(1,copies);

        if (move)
        {
            sb_translate[0] = (float) x;
            sb_translate[1] = (float) y;
            sb_translate[2] = (float) z;
            m->saveForUndo(NULL,UNDO_NOUNDO);

            // Just move brushes
            m->makeSelectedPerform(SEL_TRANSLATE);
        }
        else
        {
            m->saveForUndo(NULL,UNDO_NOUNDO);
            for (int i = 0; i < copies; i++)
            {
                // Copy brushes.
                m->cloneSelection();
                sb_translate[0] = (float) x;
                sb_translate[1] = (float) y;
                sb_translate[2] = (float) z;

                // Move them a bit.  (Moved brush is selected for next clone...)
                m->makeSelectedPerform(SEL_TRANSLATE);
            }
        }

        // move or copy them...
        set.redrawxy = 1;
        set.redrawedit = 1;

        if (move)
        {
            strcpy(outstr,"Moved selected brushes...");
        }
        else
        {
            strcpy(outstr,"Copied and moved selected brushes...");
        }
    }
    else
    {
        strcpy(outstr,"Couldn't create dialog!");
    }
    Show_Frame(outstr,true);
}


/*
=================
CmJumpSelXY

jump selection to XY eye
=================
*/
void CmJumpSelXY()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    if (!m->numSelected())
    {
        Show_Frame("At least one brush must be selected...",false);
        return;
    }

    Entity *ent, *e2;
    SetBrush *brush, *b2;

    vec3_t selctr = {0, 0, 0};
    int bcnt = 0;
    for (ent = m->objects.p_next; ent != &m->objects; ent = e2)
    {
        e2 = ent->p_next;
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = b2)
        {
            b2 = brush->p_next;
            if (!brush->IsSelected())
                continue;
            VectorAdd(brush->bctr, selctr, selctr);
            bcnt++;
        }
    }
    if(!bcnt)
        return;

    VectorScale(selctr, 1/float(bcnt), selctr);	// get center average

    VectorSubtract(xy_eye, selctr, selctr);	// move to xy eye

    sb_translate[0] = (float) snapToGrid(selctr[0]);	//align translation
    sb_translate[1] = (float) snapToGrid(selctr[1]);
    sb_translate[2] = (float) snapToGrid(selctr[2]);

    m->saveForUndo(NULL,UNDO_NOUNDO);

    // move brushes
    m->makeSelectedPerform(SEL_TRANSLATE);

    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Moved selected brushes...",true);
}

/*
================
CmMakeLights

Make an array of light entities
================
*/
void CmMakeLights()
{
    if(!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    if (m->numSelected() != 1)
    {
        Show_Frame("Must have only one brush selected...",false);
        return;
    }

    // parse in the light names...
    client->lightxfer.boxdata.Clear();

    for (int i = 0; i < entity_classes_i->numElements; i++)
    {
        char *str = entity_classes_i->objects[i]->name;
        // If it doesn't start with "light..." then skip it.
        if (strnicmp(str,"light",5))
            continue;

        int sel = !strcmpi(str, "light");
        client->lightxfer.boxdata.Add(str, sel);
    }

    // Try and select default "light" entity.
    // NOTE:  This may not work right... because all sort of pass the test if only
    //  the first 5 characters are checked.  Need an "exact" match routine.  There
    //  may be one, I didn't look.
    //  lightxfer.boxdata.SelectString("light"); // try and pick the default...

    char outstr[80];

    // Run dialog...
    TLightDlg dialog(client->hwnd,&client->lightxfer);
    if (dialog.Execute() == IDOK)
    {
        int x = atoi(client->lightxfer.NumX);
        int y = atoi(client->lightxfer.NumY);
        int z = atoi(client->lightxfer.NumZ);
        int val = atoi(client->lightxfer.LightVal);

        m->saveForUndo(NULL,UNDO_NOUNDO);

        // Make them.
        m->makeLights(x,y,z, val, client->lightxfer.selected);

        set.redrawxy = 1;
        set.redrawedit = 1;
        strcpy(outstr,"Made selected brush into lights...");
    }
    else
    {
        strcpy(outstr,"Couldn't create lights");
    }
    Show_Frame(outstr,true);
}

/*
================
CmRotateBrush

Rotate Selected brush by specified angles along specified axes
================
*/
void CmRotateBrush()
{
    char outstr[80];
    if (set.Map_Read)
    {
        map *m = map_i[set.curmap];
        if (!m->numSelected())
        {
            Show_Frame("Must have at least one selected brush...",false);
            return;
        }

        // Run dialog...
        TRotateDlg dialog(client->hwnd,&client->rotatexfer);
        if (dialog.Execute() == IDOK)
        {

            int r = atoi(client->rotatexfer.Roll);
            int p = atoi(client->rotatexfer.Pitch);
            int y = atoi(client->rotatexfer.Yaw);

            m->saveForUndo(NULL,UNDO_NOUNDO);

            // Rotation roll, pitch and yaw.
            m->rotateBrush(r,p,y);
            set.redrawxy = 1;
            set.redrawedit = 1;
            strcpy(outstr,"Rotated Brush(es)...");
        }
        else
        {
            strcpy(outstr,"Couldn't rotate brush...");
        }
        Show_Frame(outstr,true);
    }
}

/*
=================
CmShortBrush

Create a short brush from current brush.
=================
*/
void CmShortBrush()
{
    if (set.Map_Read)
    {
        map *m = map_i[set.curmap];
        m->saveForUndo(NULL,UNDO_NOUNDO);

        // Create it.
        m->shortBrush();

        set.redrawxy = 1;
        set.redrawedit = 1;
        Show_Frame("Made selected brush short...",true);
    }
}
/*
===============
CmTileChildren

Had to write my own because my custom caption MDI children aren't tiled by windoze
===============
*/
void CmTileChildren()
{
    HWND win = ::GetTopWindow(client->hwnd);
    HWND fwin = win;
    //count all visible windows
    int numVis = 0;
    do
    {
        if (::IsWindowVisible(win) && !::IsIconic(win))
        {
            numVis++;
        }
        win = ::GetWindow(win,GW_HWNDNEXT);
    }
    while(win && win != fwin);

    if(!numVis)
        return;

    //create list of all visible windows
    HWND *winlist = new HWND[numVis];

    //reset these...
    win = ::GetTopWindow(client->hwnd);
    fwin = win;

    //loop again and fill winlist. NOTE: these should be sorted so the same order is used
    int i = 0;
    do
    {
        if (::IsWindowVisible(win) && !::IsIconic(win))
        {
            winlist[i++] = win;
        }
        win = ::GetWindow(win,GW_HWNDNEXT);
    }
    while(i < numVis && win && win != fwin);

    // calculate number of rows and cols
    float t = sqrt((float)numVis);   // Get the square root
    int cols = (int) ceil(t);   // Determine # of rows and cols.
    int rows = (int) ceil((float)numVis/(float)cols);

    // Get space we ware tiling within.
    RECT r;
    ::GetClientRect(client->hwnd,&r);

    int clientW = r.right-r.left;
    int clientH = r.bottom-r.top;

    // Determine height and width of each window.
    int winW = clientW / cols;
    int winH = clientH / rows;

    // tile
    int x=0,y=0;
    for(i = 0; i < numVis; i++)
    {
        ::SetWindowPos(winlist[i],HWND_TOP,x*winW,y*winH,winW,winH,0);
        x++;
        if(x>=cols)
        {
            x = 0;
            y++;
        }
    }

    delete [] winlist;
}

/*
================
CmCascadeChildren

================
*/
void CmCascadeChildren()
{
    HWND win = ::GetTopWindow(client->hwnd);
    HWND fwin = win;
    //count all visible windows
    int numVis = 0;
    do
    {
        if (::IsWindowVisible(win) && !::IsIconic(win))
        {
            numVis++;
        }
        win = ::GetWindow(win,GW_HWNDNEXT);
    }
    while(win && win != fwin);

    if(!numVis)
        return;

    //create list of all visible windows
    HWND *winlist = new HWND[numVis];

    //reset these...
    win = ::GetTopWindow(client->hwnd);
    fwin = win;

    //loop again and fill winlist. NOTE: these should be sorted so the same order is used
    int i = 0;
    do
    {
        if (::IsWindowVisible(win) && !::IsIconic(win))
        {
            winlist[i++] = win;
        }
        win = ::GetWindow(win,GW_HWNDNEXT);
    }
    while(i < numVis && win && win != fwin);

    //cascade windows 18px apart
    for(i = 0; i < numVis; i++)
    {
        ::SetWindowPos(winlist[i],HWND_TOP,i*18,i*18,0,0,SWP_NOSIZE);
    }

    delete [] winlist;

    return;
}

/*
================
CmArrangeIcons


================
*/
void CmArrangeIcons()
{
    ArrangeIconicWindows(client->hwnd);
}

/*
================
CmWindowPosition

Reposition windows
================
*/
void CmWindowPosition()
{
    client->SetWindowPositions();
    client->ShowAllWindows();

    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Window positions loaded", true);
}

/*
==============
EvTimer

NOTE:  Should use the timer for autosaving!!!
==============
*/
//void TBSPWindow::EvTimer(uint /*TimerId*/)
//{
/*
FIXME Use a clock_t and measure elapsed time!
if (set.Map_Read)
{
	set.autoSaveCounter++;
	if (set.autoSaveCounter > set.autoSaveMinutes*6)
   {
		// autosave...
		SaveMapFile("c:\\backup.map",true);
		Show_Frame("Autosaving",true);
	}
}
*/
//DefaultProcessing();
//}

/*
================
CmToggleNames

Toggle display of entity names
================
*/
void CmToggleNames()
{
    set.show_names ^= 1;
    if (set.Map_Read)
    {
        set.redrawxy = 1;
        Show_Frame("Names toggled...",true);
    }
}

/*
================
CmToggleHits

Show hit brush #'s and blue outlines?
================
*/
void CmToggleHits()
{
    set.show_hits ^= 1;
    if (set.Map_Read)
    {
        set.redrawedit = 1;
        Show_Frame("Show all hit brushes...",true);
    }
}

/*
=================
CmToggleCoordinates

Show grid coordinates?
=================
*/
void CmToggleCoordinates()
{
    set.show_coordinates ^= 1;
    if (set.Map_Read)
    {
        set.redrawxy = 1;
        Show_Frame("Coordinates toggled...",true);
    }
}

/*
=================
CmToggleOutline

Show 3d window outlines?
=================
*/
void CmToggleOutline()
{
    set.outline ^= 1;
    if (set.Map_Read)
    {
        set.redrawedit = 1;
        Show_Frame("Outline toggled...",true);
    }
}

/*
==================
CmGridMode

Toggle grid display on/off
==================
*/
void CmGridMode()
{
    set.show_grid ^= 1;
    if (set.Map_Read)
    {
        set.redrawxy = 1;
        set.redrawedit = 1;
        Show_Frame("Grid toggled...",true);
    }
}

/*
===============
CmModeWire etc.

Switch display mode
===============
*/
void CmModeWire()
{
    if (!set.Map_Read)
        return;

    set.drawmode = Drawmode::wire;
    set.redrawedit = 1;
    Show_Frame("Mode set to wire...",true);
}

void CmModeFlat()
{
    if (!set.Map_Read)
        return;

    set.drawmode = Drawmode::flat;
    set.redrawedit = 1;
    Show_Frame("Mode set to flat...",true);
}

void CmModeTexture()
{
    if (!set.Map_Read)
        return;

    set.drawmode = Drawmode::texture;
    set.redrawedit = 1;
    Show_Frame("Mode set to texture...",true);
}

/*
=================
CmFloorUp/Down

Move up or down to the next brush level under or above current view spot.
=================
*/
void CmFloorUp()
{
    if (set.Map_Read)
    {
        map *m = map_i[set.curmap];

        sb_floor_dir = 1;
        sb_floor_dist = TMAX;

        // Find distance to next brush.
        m->makeAllPerform(SEL_FEETTOFLOOR);
        if (sb_floor_dist == TMAX)
        {
            status->SetText("Already at top level",true);
            return;
        }

        set.redrawedit = 2;
        set.redrawxy = 2;

        // Move there in both views.
        m->eye[m->cureye][2] += sb_floor_dist;
        m->MoveEyePosition(m->eye[m->cureye],set.lock_cameras);

        Show_Frame("Moved up a floor",true);
    }
}

void CmFloorDown()
{
    if (set.Map_Read)
    {
        map *m = map_i[set.curmap];
        sb_floor_dir = -1;
        sb_floor_dist = -TMAX;
        m->makeAllPerform(SEL_FEETTOFLOOR);
        if (sb_floor_dist == -TMAX)
        {
            status->SetText("Already at bottom level",true);
            return;
        }
        set.redrawedit = 2;
        set.redrawxy = 2;
        m->eye[m->cureye][2] += sb_floor_dist;
        m->MoveEyePosition(m->eye[m->cureye],set.lock_cameras);

        Show_Frame("Moved down a floor",true);
    }
}

/*
==============
CmAutoSave

Set's autosave timer.
FIXME
==============
*/
void CmAutoSave()
{
    if (set.Map_Read)
    {
        // SetTimer(1,1000*30,0); // every ten seconds...
    };
}

/*
===============
CmReset

Redraw views.
===============
*/
void CmReset()
{
    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Reset",true);
}

/*
================
AutoLoad

Load the file specified at the command line.
NOTE:  Does this work?
================
*/
void TBSPWindow::AutoLoad(char *fname)
{
    char fullname[1024];
    sprintf(fullname,"%s\\%s",set.map_directory,fname);

    if (!file_exists(fname) && !file_exists(fullname))
    {
        char prompt[80];
        sprintf(prompt,"Couldn't find [%s]...",fname);
        MessageBox(hwnd, prompt, "BSP - AutoLoad", MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    char *usename = fullname;
    if (file_exists(fname))
        usename = fname;

    map_i[set.nummaps] = new map;

    set.curmap = set.nummaps;
    set.nummaps++;

    map *m = map_i[set.curmap];

    STRNCPY(m->filename,usename);
    frame->SetCaption(m->filename);
    editWindow->SetCaption(m->filename);

    if (!set.Map_Read)
    {
        client->ShowAllWindows();
    }

    set.Map_Read = 1;
    loadMessageGiven = false;
    if(!ProcessMapFile(usename))
    {
        CmClose();
        return;
    }
    char outstr[80];
    sprintf(outstr,"File [%s] processed...",usename);
    set.redrawedit = 1;
    set.redrawxy = 1;

    MapChanged();
    Show_Frame(outstr,true);
}

/*
===============
CmOpen

Open a new .map file
===============
*/

void OpenMapFile(char *filename)
{

    // limit number of maps that can be opened
    if (set.nummaps >= MAX_MAPS)
    {
        MessageBox(client->hwnd, "No more maps available...", "BSP - Cannot Open", MB_OK | MB_ICONEXCLAMATION);
        return;
    }
    // warn if same map is opened twice
    for(int i = 0; i < set.nummaps; i++)
    {
        if(!strcmpi(filename,map_i[i]->filename))
        {
            int retval = MessageBox(client->hwnd,
                                    "This map file is already open. The map may be opened again, but be warned \n"
                                    "that each copy will save to the same file by default.\n\n"
                                    "Open another copy?","BSP Sync Check",MB_YESNO);

            if(retval == IDNO)
                return;

            break;
        }
    }

    map_i[set.nummaps] = new map;

    set.curmap = set.nummaps;
    set.nummaps++;
    map *m = map_i[set.curmap];

    STRNCPY(m->filename,filename);
    frame->SetCaption(m->filename);
    editWindow->SetCaption(m->filename);

    if (!set.Map_Read)
    {
        client->ShowAllWindows();
    }
    set.Map_Read = 1;
    loadMessageGiven = false;
    if(!ProcessMapFile(filename))
    {
        CmClose();
        return;
    }

    char outstr[256];
    snprintf(outstr,sizeof(outstr),"File [%s] processed...",filename);
    set.redrawedit = 1;
    set.redrawxy = 1;

    MapChanged();

    AddRecent(filename);

    // CmAutoSave();
    //autoSaveCounter = 0L;

    Show_Frame(outstr,true);
}
void CmOpen()
{
    if (set.nummaps >= MAX_MAPS)
    {
        MessageBox(client->hwnd, "No more maps available...", "BSP - Cannot Open", MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    char filename[256] = "";

    OPENFILENAME ofn;
    memset(&ofn,0,sizeof(OPENFILENAME));
    ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
    ofn.hwndOwner = frame->hwnd;
    ofn.lpstrFilter = "Map Files (*.map)\0*.map\0";
    ofn.lpstrDefExt = "map";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = 256;
    ofn.lpstrInitialDir = set.map_directory;
    ofn.lpstrTitle = "Select MAP File";
    ofn.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;

    if(GetOpenFileName(&ofn))
    {
        OpenMapFile(filename);
    }
}

/*
===============
CmFileSaveSel

Save selected brushes to .bru file
===============
*/
void CmFileSaveSel()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    int num = m->numSelected();
    if (!num)
        return;

    char filename[256] = "";

    OPENFILENAME ofn;
    memset(&ofn,0,sizeof(OPENFILENAME));
    ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
    ofn.hwndOwner = frame->hwnd;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = 256;
    ofn.lpstrFilter = "Bru Files (*.bru)\0*.bru\0";
    ofn.lpstrDefExt = "bru";
    ofn.lpstrInitialDir = set.map_directory;
    ofn.lpstrTitle = "Output Brush (.bru) File";
    ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

    char outstr[80];

    if(GetSaveFileName(&ofn))
    {
        Entity *e;
        SetBrush *b;
        FILE *f = fopen (filename,"w");
        if (!f)
        {
            sprintf(outstr,"Couldn't write brush file %s",filename);
            Show_Frame(outstr,false);
            return;
        }

        // Output selected brushes that are visible.
        for (e = m->objects.p_next; e != &m->objects; e = e->p_next)
        {
            for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
            {
                if (b->IsDrawable() && b->IsSelected())
                    b->writeToFILE(f,true);
            }
        }
        fclose (f);

        sprintf(outstr,"Wrote selected brushes to file %s...",filename);
        Show_Frame(outstr,false);
    }
    else
    {
        Show_Frame("Canceled...",false);
    }
}

/*
================
CmLoadBrushes

Load .bru file into current world entity of current map
================
*/
void CmLoadBrushes()
{
    if (!set.Map_Read)
        return;

    char filename[256] = "";

    OPENFILENAME ofn;
    memset(&ofn,0,sizeof(OPENFILENAME));
    ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
    ofn.hwndOwner = frame->hwnd;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = 256;
    ofn.lpstrFilter = "Bru Files (*.bru)\0*.bru\0";
    ofn.lpstrDefExt = "bru";
    ofn.lpstrInitialDir = set.map_directory;
    ofn.lpstrTitle = "Input Brush (.bru) File";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    char outstr[80];

    if(GetOpenFileName(&ofn))
    {
        face_t	*f;
        int		i,j;
        unsigned char *dat;
        SetBrush *b;

        map *m = map_i[set.curmap];
        m->saveForUndo(NULL,UNDO_NOUNDO);

        sprintf(outstr,"Loading [%s]",filename);
        status->SetText(outstr,true);


        // Figure out what group to put brushes in.
        int newindex = 0;
        if (groupWindow)
        {
            newindex = groupWindow->GroupList->GetSelIndex();

            if (newindex < 0 || newindex >= groupWindow->GroupList->GetCount()) // name field not one of the groups...
                newindex = 0; // put in default if no group is current...
        }

        dat = file_get_contents(filename);	//new unsigned char[size];
        if(!dat)
        {
            syserror(const_cast<char *> ("Error: ReadBrushFile:  Couldn't read [%s]..."), filename);
            return;
        }

        Tokenizer script((char*)dat);
        do
        {
            if (!script.next(true))   // No more tokens, done.
                break;

            // Create brush
            b = new SetBrush();
            b->group = newindex;
            b->parent = m->world; // add to world.

            while(true)
            {
                // Parse out group #
                script.getGroups = 1;
                if (!script.next(true))
                {
                    script.getGroups = 0;
                    break;
                }
                script.getGroups = 0;

                // If get closing brace, done.
                if (!strcmp (script.token, "}") )
                    break;

                // Parse in next face.
                f = new face_t;
                memset(f, 0, sizeof(face_t));

                // parsed in!
                b->group = newindex;
                b->setLocked(script.curLock);

                // Get face def. points.
                for (i=0 ; i<3 ; i++)
                {
                    if (i != 0)
                        script.next(true);

                    if (strcmp(script.token, "(") )
                    {
                        syserror (const_cast<char *> ("Error: parsing brush file, ( expected"));
                        delete f;
                        delete b;
                        return;
                    }

                    for (j=0 ; j<3 ; j++)
                    {
                        script.next (false);
                        f->planepts[i][j] = (float) atoi(script.token);
                    }

                    script.next (false);

                    if (strcmp (script.token, ")") )
                    {
                        syserror (const_cast<char *> ("Error: parsing brush file, ) expected"));
                        delete f;
                        delete b;
                        return;
                    }
                }

                // Get texture name.
                script.next (false);
                strncpy (f->texture.texture, script.token, sizeof(f->texture.texture));
                f->texture.texture[sizeof(f->texture.texture)-1] = 0;

                // Fix up name.
                for (int l = 0; l < LUMP_NAME_LENGTH; l++)
                    f->texture.texture[l] = (char)toupper(f->texture.texture[l]);

                // If Q2 Map, split out texture base dir.
                if (set.game_mode == 2)
                {
                    // copy the base path...
                    f->texture.texType = TEX_TYPE_Q2;
                    char realName[32];
                    SplitBase(f->texture.texture,
                              f->texture.basepath, realName);
                    sprintf(f->texture.texture,realName);
                }

                // Get shift/rotate/scale values.
                script.next(false);
                f->texture.shift[0] = (float) atof(script.token);
                script.next(false);
                f->texture.shift[1] = (float) atof(script.token);
                script.next(false);
                f->texture.rotate = (float) atof(script.token);
                script.next(false);
                f->texture.scale[0] = (float) atof(script.token);
                script.next(false);
                f->texture.scale[1] = (float) atof(script.token);

                // Assume defaults
                f->defaults = true;
                // And that they are 0
                f->texture.contents = f->texture.value = f->texture.flags = 0;
                // skip any Hexen 2 stuff...
                if (set.game_mode == 1)
                {
                    if (script.avail_line())   // Hexen had -1 on each line, it was not used.
                    {
                        script.next(false);
                        f->texture.value = atoi(script.token);
                    }
                }
                else if (set.game_mode == 2)	// Quake 2
                {
                    if (script.avail_line())
                    {
                        // if one is used, then no default anymore...
                        script.next(false);
                        // quake 2 stuff..
                        f->texture.contents = atoi(script.token);

                        // Note:
                        f->defaults = false;
                    }

                    // quake 2 stuff..
                    if (script.avail_line())
                    {
                        script.next(false);
                        f->texture.flags = atoi(script.token);
                    }
                    if (script.avail_line())
                    {
                        script.next(false);
                        f->texture.value = atoi(script.token);
                    }
                }
                b->addFace(b,f);
                // Clear out rest of line (should never happen)...
                script.skipline();
                //while (TokenAvailable())
                //	GetToken(false);
            }

            // Recalculate visible faces.
            b->calcWindings();

            // Didn't make a valid shape.
            if (b->IsInvalid())
            {
                delete b;
            }
            else
            {
                b->setSelected(true);   // Select and add to world
                m->world->addObject(b);
            }
        }
        while(1);

        // Free file buffer.
        delete[] dat;

        sprintf(outstr,"Read brushes from file %s...",filename);
        set.redrawedit = 1;
        set.redrawxy = 1;
    }
    else
    {
        strcpy(outstr,"Cancelled...");
    }
    Show_Frame(outstr,false);
}

/*
==============
CmNew

New Map
==============
*/
void CmNew()
{
    if (set.nummaps >= MAX_MAPS)
    {
        MessageBox(client->hwnd, "No more maps available...", "BSP - Cannot Open New Map", MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    if (!set.Map_Read)
    {
        client->ShowAllWindows();
    }

    set.Map_Read = 1; // map file read...
    map_i[set.nummaps] = new map;

    set.curmap = set.nummaps;
    set.nummaps++;

    map *m = map_i[set.curmap];
    m->newMap();
    strncpy(m->filename,set.default_mapfile,sizeof(m->filename));

    char outstr[80];
    sprintf(outstr,"Map [%s] created...",set.default_mapfile);
    set.redrawedit = 1;
    set.redrawxy = 1;
    editWindow->SetCaption(set.default_mapfile);
    frame->SetCaption(set.default_mapfile);
    set.autoSaveCounter = 0;
    // CmAutoSave();
    MapChanged();
    Show_Frame(outstr,true);
}

/*
================
CmSelectAll

================
*/
void CmSelectAll()
{
    if (set.Map_Read)
    {
        set.redrawxy = 1;
        set.redrawedit = 1;

        // Do it.
        map_i[set.curmap]->makeAllPerform(SEL_SELECT);

        Show_Frame("Selected all brushes/entities",true);
    }
}

/*
=================
CmDeselectAll

=================
*/
void CmDeselectAll()
{
    if (set.Map_Read)
    {
        map *m = map_i[set.curmap];
        set.redrawxy = 1;
        set.redrawedit = 1;

        if (m->clipper_i->hide())
        {
            // first press hides clipper only
            Show_Frame("Hiding clip points",true);
            return;
        }

        // Second press (or first if no clipper), deselect all.
        m->makeGlobalPerform(SEL_DESELECT);
        // Revert default to world.
        m->setCurrentEntity(m->world);

        Show_Frame("Deselected all brushes/entities",true);
    }
}

/*
============
XYZMove

Move brushes as specified
============
*/
void XYZMove(int axis,int distance,int /*currentMode*/)
{
    // do a translation...
    if (set.Map_Read)
    {
        map *m = map_i[set.curmap];
        // Zero it out.
        VectorCopy (vec3_origin, sb_translate);
        // Set up
        sb_translate[axis] = (float) distance;
        m->saveForUndo(NULL,UNDO_NOUNDO);

        // Do it.
        m->makeSelectedPerform(SEL_TRANSLATE);
        set.redrawedit = 1;
        set.redrawxy = 1;
    }
}

/*
==================
CmLock / Unlock

Set/unset texture locking state for selected brushes
==================
*/
void CmLock()
{
    // handle it...
    if (set.Map_Read)
    {
        set.redrawxy = 1;
        set.redrawedit = 1;
        map_i[set.curmap]->makeSelectedPerform(SEL_LOCK);
        Show_Frame("Brush textures locked...",true);
    }
}

void CmUnLock()
{
// handle it...
    if (set.Map_Read)
    {
        set.redrawxy = 1;
        set.redrawedit = 1;
        map_i[set.curmap]->makeSelectedPerform(SEL_UNLOCK);
        Show_Frame("Brush textures unlocked...",true);
    }
}

/*
=================
CmSelectValue

Select entities by value in keypair.
=================
*/
void CmSelectValue()
{
    if (!set.Map_Read)
        return;

    if (!entity_classes_i || (entity_classes_i->numElements <=0))
        return;

    client->selectxfer.Entity.Clear();
    for (int i = 0; i < entity_classes_i->numElements; i++)
    {
        int sel = !strcmpi(entity_classes_i->objects[i]->name, "info_player_start");
        client->selectxfer.Entity.Add(entity_classes_i->objects[i]->name, sel);
    }

    TSelectDlg dialog(client->hwnd,&client->selectxfer);
    if (dialog.Execute() == IDOK)
    {
        char key[MAX_KEY];
        char val[MAX_VALUE];

        int useE = client->selectxfer.SelectEntity;
        int useK = client->selectxfer.SelectKey;
        int useV = client->selectxfer.SelectValue;
        STRNCPY(key, client->selectxfer.Key);
        STRNCPY(val, client->selectxfer.Value);

        // Do it.
        map_i[set.curmap]->selectByEntityKeyValue(useE,useK,useV,client->selectxfer.selected,key,val);
    }

    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Brushes selected by value...",true);
}

/*
=================
CmToggleKnobs

Show/hide selected 'x' knobs
=================
*/
void CmToggleKnobs()
{
    set.center_knobs_xy ^= 1;

    // Redraw.
    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Knobs toggled on/off...",true);
}

/*
=================
CmTUnload

These are the texture wad functions that show on main menu.  Just pass on to
texture window
FIXME:  Handle by passing command ID and using a switch.
=================
*/
void CmTUnload()
{
    texWindow->UnloadWad();
}
void CmTLoadWad()
{
    texWindow->ScanNewWad();
}
void CmTLoadPak()
{
    texWindow->ScanNewWalFromSelectedPak();
}
void CmTLoadPakQ2()
{
    texWindow->ScanNewWalFromQ2Pak();
}
void CmTLoadDir()
{
    texWindow->ScanNewWalDirectory();
}
void CmTLoadUsed()
{
    texWindow->ScanInUse();
}
void CmTLoadWal()
{
    texWindow->ScanNewTexture();
}

/*
=================
SelectMap

Handle selection of .map from map gadget list.
=================
*/
void SelectMap(int mapnum)
{
    if (!set.Map_Read)
        return;

    //Validate.
    int index = mapnum - 1;
    if (index < 0 || index == set.curmap)  //  no change.
        return;
    if (index >= set.nummaps)
        return;

    set.curmap = index;

    // Set captions.
    map *m = map_i[set.curmap];
    frame->SetCaption(m->filename);
    editWindow->SetCaption(m->filename);

    set.redrawedit = 1;
    set.redrawxy = 1;

    // Change maps.
    MapChanged();

    Show_Frame("Map switched...",true);
}

/*
================
M1 - Mxxx

Handle selection of map by index into map gadget.  There is probably a better
way, e.g. WM_USER messages?
================
*/

void SelectMap1()
{
    SelectMap(1);
}
void SelectMap2()
{
    SelectMap(2);
}
void SelectMap3()
{
    SelectMap(3);
}
void SelectMap4()
{
    SelectMap(4);
}
void SelectMap5()
{
    SelectMap(5);
}
void SelectMap6()
{
    SelectMap(6);
}
void SelectMap7()
{
    SelectMap(7);
}
void SelectMap8()
{
    SelectMap(8);
}
void SelectMap9()
{
    SelectMap(9);
}
void SelectMap10()
{
    SelectMap(10);
}


//
// recent files
//
void OpenRecent(int num)
{
    char fn[MAX_PATH];
    strncpy(fn,recent_files[num],sizeof(fn));
    /*awesome bug alert! without making a copy of the string and passing in a recent_files[] pointer,
    the OpenMapFile will open a map and update the recent_files list, causing a new slot to be saved
    as the most recent file rather than the selected recent file to open. A+++++++ */
    OpenMapFile(fn);
}
void CmRecent0()
{
    OpenRecent(0);
}
void CmRecent1()
{
    OpenRecent(1);
}
void CmRecent2()
{
    OpenRecent(2);
}
void CmRecent3()
{
    OpenRecent(3);
}
void CmRecent4()
{
    OpenRecent(4);
}
void CmRecent5()
{
    OpenRecent(5);
}
void CmRecent6()
{
    OpenRecent(6);
}
void CmRecent7()
{
    OpenRecent(7);
}
void CmRecent8()
{
    OpenRecent(8);
}
void CmRecent9()
{
    OpenRecent(9);
}
void CmRecent10()
{
    OpenRecent(10);
}
void CmRecent11()
{
    OpenRecent(11);
}
void CmRecent12()
{
    OpenRecent(12);
}
void CmRecent13()
{
    OpenRecent(13);
}
void CmRecent14()
{
    OpenRecent(14);
}
void CmRecent15()
{
    OpenRecent(15);
}
