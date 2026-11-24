#include "global.h"

bool redraw_interrupted = false;   // User hit Esc during long screen update.

char recent_files[MAX_RECENT][MAX_PATH];


// FIXME FIXME:  Move rendering to separate thread.


/*
=====================
SplitBaseQuiet

Extracts base path from full path name.  No message box on bogus input.
=====================
*/
void SplitBaseQuiet(char *path, char *basepath, char *name)
{
    if (!path)
        return;

    char *start;
    start = path;

    int len = strlen(path);
    int i = 0;

    while (i < len && start && start[0] != '\\' && start[0] != '/')
        basepath[i++] = *start++;

    if(i == len)
    {
        *basepath = 0;
        strcpy(name,path);
        return;
    }

    // put in a good separator, too.
    basepath[i++] = set.rel_path_separator;
    start++;
    basepath[i] = '\0';

//   start++; // skip the wack-wack
    i = 0;
    while (start && start[0])
        name[i++] = *start++;

    name[i] = '\0';
}

/*
=================
ProcessMapFile

Read in a new map file.
=================
*/
bool ProcessMapFile(char *fname)
{
    return map_i[set.curmap]->readMapFile(fname);
}

/*
=================
MergeMapFile

Merge a .mrg file into the current .map
=================
*/
void MergeMapFile(char *fname)
{
    map_i[set.curmap]->mergeMapFile(fname);
}

/*
=================
SaveMapFile

Save file out to disk.
=================
*/
void SaveMapFile(char *fname,int writeWorldKeys)
{
    if (!set.Map_Read)
        return;

    char realName[256];
    STRNCPY(realName,fname);
    StripExtension (realName);
    strcat(realName,".map");
    strcpy(fname,realName);
    map_i[set.curmap]->writeMapFile(fname,writeWorldKeys,false);
}

//=================
// recent files
//=================

void InitRecent()
{
    WProfile pf("BSP", "settings/bsp.ini");
    set.num_recent   = pf.GetInt("num_recent",set.num_recent);
    set.num_recent	  = min(MAX_RECENT, set.num_recent);
    if(set.num_recent <= 0)
        return;

    memset(recent_files,0,sizeof(recent_files));

    //load from bsp.ini into recent_files array
    char fn[MAX_PATH];
    strcpy(pf.section, "Recent");
    for(int i = 0, c = 0 ; c < set.num_recent && i < MAX_RECENT; i++)
    {
        pf.GetString(Str(const_cast<char *> ("r_%d_%d"), set.game_mode, i), fn, sizeof(fn), const_cast<char *> (""));
        if(!*fn)
            continue;
        strncpy(recent_files[i],fn,sizeof(recent_files[i]));
        c++;
    }
}
void SaveRecent()
{
    if(set.num_recent <= 0)
        return;
    //write from recent_files array to bsp.ini
    WProfile pf(const_cast<char *> ("Recent"), const_cast<char *> ("settings/bsp.ini"));
    for(int i = 0; i < MAX_RECENT; i++)
        pf.WriteString(Str(const_cast<char *> ("r_%d_%d"), set.game_mode, i), recent_files[i]);
}
void AddRecent(char *filename)
{
    if(set.num_recent <= 0)
        return;

    int exists = -1;
    //is filename already in recent list?
    for(int i = 0; i<set.num_recent; i++)
    {
        if(!stricmp(recent_files[i], filename))
        {
            exists = i;
            break;
        }
    }

    //how many slots need strings shifted...
    int stop = set.num_recent - 1;
    if(exists >= 0)
    {
        if(exists == 0)
            return;			// already at front

        stop = exists;
    }

    //shift back
    for(int i = stop; i > 0; i--)
    {
        strncpy(recent_files[i], recent_files[i-1], sizeof(recent_files[0]));
    }

    //set first entry
    strncpy(recent_files[0], filename, sizeof(recent_files[0]));

    SaveRecent();
}

//returns number of items added
int BuildRecentMenu(HMENU menu)
{
    //need to determine and cache recent cmd ids
    static int recent_id[MAX_RECENT];
    static bool recent_init = false;

    if(!recent_init)
    {
        recent_init = true;

        for(int i = 0; i < MAX_RECENT; i++)
        {
            ccmd_t *cm = Ccmd_FindCmd(Str(const_cast<char *> ("CM_RECENT%d"), i));
            if(cm) recent_id[i] = cm->id;
        }
    }

    int c = 0;	// add count
    for(int i=0; i < set.num_recent; i++)
    {
        if(!*recent_files[i])
            continue;
        char buf[MAX_PATH+16], *buf_ptr;
        snprintf(buf,sizeof(buf),"&%d %s", c+1, recent_files[i]);

        buf_ptr = buf;

        if(c >= 9)
        {
            buf_ptr++;	// hide mnemonic for numbers 10+
        }
        AppendMenu(menu, MF_STRING, recent_id[i], buf_ptr);

        c++;
    }

    //add a disabled placeholder
    if(!c)
    {
        AppendMenu(menu, MF_STRING | MF_GRAYED, 0, "Recent list empty");
    }
    return c;
}

/*
=================
Show_Frame

Rerender the specified views (set.redrawxy and set.redrawedit)
Display the specified text in the status bar.

THIS IS THE MAIN BSP RENDERING START POINT>
=================
*/
void Show_Frame(const char* str, bool notStatusOnly)
{
    char outstr[128];

    if (*str)
        strncpy(outstr, str, sizeof(outstr));
    else
        strcpy(outstr,"...");
    outstr[sizeof(outstr)-1] = 0;

    // Set the status bar.
    status->SetText(str);

    // Update rest if true passed in.
    if (notStatusOnly && set.Map_Read)
    {
        map *m = map_i[set.curmap];

        // Find rotation matrix
        findQ(set.Q,set.R,m->eye[m->cureye],
              m->angles[m->cureye].roll,
              m->angles[m->cureye].pitch,
              m->angles[m->cureye].yaw);

        if (set.redrawedit)
            editWindow->RedrawContents();

        if (xyWindow && set.redrawxy)
        {
            for (int c = 0 ; c < set.xyViews; c++)
            {
                if(set.redrawxy == 2)
                {
                    xyWindow[c]->RedrawFromBase();
                }
                else
                {
                    xyWindow[c]->RedrawContents();
                }

                if (c == set.curxy)
                {
                    char tmpstr[80];
                    sprintf(tmpstr,"Base: %i Ht. %i",(int)m->minz[xyWindow[c]->NormalAxis],
                            (int)(m->maxz[xyWindow[c]->NormalAxis]-m->minz[xyWindow[c]->NormalAxis]));
                    status->SetText(STATUS_HEIGHT, tmpstr);
                }
            }
        }
        set.redrawedit = 0;
        set.redrawxy = 0;

        // Disply timing info if we were timing.
        //  FIXME, need to put starTime and endTime at right spot.
        if (timeRender)
        {
            status->SetText(STATUS_BOUND, Str(const_cast<char *> ("Time:  %5.5f s."),(float)(endTime-startTime)/CLK_TCK));
        }
    }
    status->DrawStatusBar();
}
void Show_Frame(bool notStatusOnly, const char* str, ...)
{
    if(!str)
    {
        Show_Frame(str, true);
    }
    else
    {
        va_list vargs;
        va_start(vargs, str);
        int c = _vscprintf(str, vargs);
        char *text = new char[c + 1];
        vsprintf(text,str,vargs);
        Show_Frame(text, notStatusOnly);
        delete[] text;
        va_end(vargs);
    }
}

/*
=================
PopulateMake

Fill in the prefab menu.
=================
*/
void TBSPWindow::PopulateMake()
{
    if (!frame->makeBox)
        return;

    frame->makeBox->ClearList();

    frame->makeBox->InsertString(const_cast<char *> ("Tall Brush"));
    frame->makeBox->InsertString(const_cast<char *> ("Short Brush"));
    frame->makeBox->InsertString(const_cast<char *> ("Brush of Specified Height"));
    frame->makeBox->InsertString(const_cast<char *> ("Brush from Values"));
    frame->makeBox->InsertString(const_cast<char *> ("Lights"));
    frame->makeBox->InsertString(const_cast<char *> ("Stairs"));
    frame->makeBox->InsertString(const_cast<char *> ("Room"));
    frame->makeBox->InsertString(const_cast<char *> ("Extruded Room"));
    frame->makeBox->InsertString(const_cast<char *> ("Arch"));
    frame->makeBox->InsertString(const_cast<char *> ("Tube (Hollow)"));			// was Cylinder
    frame->makeBox->InsertString(const_cast<char *> ("Cylinder (Solid)"));		// was N-Sided Brush
    frame->makeBox->InsertString(const_cast<char *> ("Pyramid"));
    frame->makeBox->InsertString(const_cast<char *> ("Wedge"));

    frame->makeBox->SetSelIndex(0);
}

/*
=================
FindReplaceCallback

FindReplaceDlg callback function
=================
*/
extern FindReplaceDlg *FindReplace;
bool FindReplaceCallback(bool replace, char*findwhat,char*replacewith)
{
    if(replace)
    {
        char fromBase[64];
        char toBase[64];
        char fromTex[64];
        char toTex[64];

        findwhat = strupr(findwhat);
        replacewith = strupr(replacewith);

        if (set.game_mode == 2)
        {
            SplitBaseQuiet(findwhat,fromBase,fromTex);
            SplitBaseQuiet(replacewith,toBase,toTex);

            if ((strlen(fromBase) <= 0) || (strlen(fromTex) <= 0) || (strlen(toBase) <= 0) || (strlen(toTex) <= 0))
            {
                MessageBox(frame->hwnd, "You must include the relative paths, e.g. path1/texname...", "BSP - Replace Texture", MB_OK | MB_ICONEXCLAMATION);
                SetFocus(FindReplace->hwnd);
                return false;
            }
        }


        int retval = MessageBox(frame->hwnd, Str(const_cast<char *> ("Replace %s\nWith %s ? "), findwhat, replacewith), "BSP - Replace Texture", MB_YESNO | MB_ICONQUESTION);
        if (retval == IDYES)
        {
            // Do it!
            map_i[set.curmap]->replaceTexture(findwhat,replacewith);
            set.redrawedit = 1;
            set.redrawxy = 1;
            Show_Frame(Str(const_cast<char *> ("Texture %s replaced with %s..."), findwhat, replacewith), true);
        }

        return false;		//leave dialog open until they hit cancel

    }
    DestroyWindow(FindReplace->hwnd);
    delete FindReplace;
    FindReplace = 0;
    return true;
}
/*
=================
CmReplace

Replace chosen from menu.
=================
*/
void CmReplace()
{
    if (set.Map_Read && !FindReplace)
    {
        FindReplace = new FindReplaceDlg(frame->hwnd, const_cast<char *> (""), const_cast<char *> (""), FindReplaceCallback);
    }
}

/*
=================
CmHelp

=================
*/
void CmHelp()
{
    Bsp_ShellExecute(frame->hwnd, const_cast<char *> ("open"), const_cast<char *> (HELP_FILE), 0, 0, SW_SHOWNORMAL);
}

/*
=================
CmNextMap

Switch to next map.
=================
*/
void CmNextMap()
{
    if (!set.Map_Read)
        return;

    set.curmap++;

    if (set.curmap >= set.nummaps)
        set.curmap = 0;

    map *m = map_i[set.curmap];
    frame->SetCaption(m->filename);
    editWindow->SetCaption(m->filename);

    set.redrawedit = 1;
    set.redrawxy = 1;

    MapChanged();

    Show_Frame("Map switched...",true);
}

/*
=================
CmPreviousMap
=================
*/
void CmPreviousMap()
{
    if (!set.Map_Read)
        return;

    set.curmap--;

    if (set.curmap < 0)
        set.curmap = set.nummaps - 1;

    map *m = map_i[set.curmap];
    frame->SetCaption(m->filename);
    editWindow->SetCaption(m->filename);

    set.redrawedit = 1;
    set.redrawxy = 1;

    MapChanged();

    Show_Frame("Map switched...",true);
}

/*
=================
CmSaveAs
=================
*/
void CmSaveAs()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    char filename[256];
    strncpy(filename,m->filename,sizeof(filename));
    filename[sizeof(filename) - 1] = 0;

    int curType = set.game_mode;     // Set default file type.

    OPENFILENAME ofn;
    memset(&ofn,0,sizeof(OPENFILENAME));
    ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
    ofn.hwndOwner = frame->hwnd;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = 256;
    ofn.lpstrInitialDir = set.map_directory;
    ofn.lpstrTitle = "Output MAP File";
    ofn.lpstrDefExt = "map";
    ofn.nFilterIndex = curType + 1;
    ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

    if (set.game_mode == 2)
        ofn.lpstrFilter = "Map Files (*.map)\0*.map\0Hexen II Map Files (*.map)\0*.map\0Quake II Map Files (*.map)\0*.map\0";
    else
        ofn.lpstrFilter = "Map Files (*.map)\0*.map\0Hexen II Map Files (*.map)\0*.map\0";

    // Show dialog, see if user hit OK
    if(GetSaveFileName(&ofn))
    {
        //warn if spaces in filename
        if(set.warn_spaces)
        {
            for (int i = strlen(filename) - 1; i > 0 && filename[i] != '\\' && filename[i] != '/'; i--)
            {
                if(filename[i] == ' ')
                {
                    Msgbox(const_cast<char *> ("Warning! The map filename contains spaces which can cause\n"
                                               "problems when building or loading maps."));
                    break;
                }
            }
        }

        int saveGame = set.game_mode;
        // okay to convert hexen2 to Quake and vice versa
        // not so sure about QII
        if (ofn.nFilterIndex == 1)
        {
            // SaveAs Quake...
            if (set.game_mode == 1) // hexen2
                set.game_mode = 0;
        }
        else if (ofn.nFilterIndex == 2)
        {
            // SaveAs Hexen2...
            if (set.game_mode == 0)
                set.game_mode = 1;
        }
        else if (ofn.nFilterIndex == 3)
        {
            // SaveAs Quake2
            set.game_mode = 2;  // do nothing for now...
        }

        // See if we are changing types.
        if ((ofn.nFilterIndex - 1) != (DWORD)curType)
        {
            m->mapType = ofn.nFilterIndex - 1;
            MessageBox(client->hwnd, "WARNING:  Exporting to a different map type than the original map type may lead to incorrect values for entity spawn flags.",
                       "BSP Save As...", MB_OK);
        }

        // DO IT
        SaveMapFile(filename,true);

        // Restore correct game type.
        // FIXME, make gametype a parameter of SaveMapFile and it's subfuncs.
        set.game_mode = saveGame;

        // Update window titles, etc.
        sprintf(m->filename,filename);
        char outstr[80];
        sprintf(outstr,"File [%s] saved...",filename);
        editWindow->SetCaption(filename);
        frame->SetCaption(filename);

        AddRecent(filename);

        char fname[256];
        ExtractFileBase(filename,fname,true);
        status->SetText(STATUS_MAP, fname);
        status->SizePartToText(STATUS_MAP);

        Show_Frame(outstr,true);
    }
}

/*
=================
CmSave
=================
*/
void CmSave()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    char filename[256];
    strncpy(filename,m->filename,sizeof(filename));
    filename[sizeof(filename)-1] = 0;

    int curType = set.game_mode;     // Set default file type.

    OPENFILENAME ofn;
    memset(&ofn,0,sizeof(OPENFILENAME));
    ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
    ofn.hwndOwner = frame->hwnd;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = 256;
    ofn.lpstrInitialDir = set.map_directory;
    ofn.lpstrTitle = "Output MAP File";
    ofn.lpstrDefExt = "map";
    ofn.nFilterIndex = curType + 1;
    ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;


    if (!strcmp(set.default_mapfile,ofn.lpstrFile))
    {
        if (set.game_mode == 2)
            ofn.lpstrFilter = "Map Files (*.map)\0*.map\0Hexen II Map Files (*.map)\0*.map\0Quake II Map Files (*.map)\0*.map\0";
        else
            ofn.lpstrFilter = "Map Files (*.map)\0*.map\0Hexen II Map Files (*.map)\0*.map\0";

        if(!GetSaveFileName(&ofn))
            return;
    }

    int saveGame = set.game_mode;
    // okay to convert hexen2 to Quake and vice versa
    // not so sure about QII
    if (ofn.nFilterIndex == 1)
    {
        // SaveAs Quake...
        if (set.game_mode == 1) // hexen2
            set.game_mode = 0;
    }
    else if (ofn.nFilterIndex == 2)
    {
        // SaveAs Hexen2...
        if (set.game_mode == 0)
            set.game_mode = 1;
    }
    else if (ofn.nFilterIndex == 3)
    {
        // SaveAs Quake2
        set.game_mode = 2;  // do nothing for now...
    }

    if ((ofn.nFilterIndex - 1) != (DWORD)curType)
    {
        m->mapType = ofn.nFilterIndex - 1;
        char warning[256];

        sprintf(warning,"WARNING:  Exporting to a different map type than the original map type may lead to incorrect values for entity spawn flags.");
        MessageBox(client->hwnd, warning, "BSP Save...", MB_OK);
    }

    SaveMapFile(filename,true);

    // Restore...
    set.game_mode = saveGame;

    STRNCPY(m->filename,filename);
    char outstr[256];
    snprintf(outstr,sizeof(outstr),"File [%s] saved...",filename);
    editWindow->SetCaption(filename);
    frame->SetCaption(filename);

    AddRecent(filename);

    char fname[256];
    ExtractFileBase(filename,fname,true);
    status->SetText(STATUS_MAP,fname);
    status->SizePartToText(STATUS_MAP);
    Show_Frame(outstr,true);
}

/*
=================
CmReload

Reload from .c or .qc files.
=================
*/
void CmReload()
{
    if (!set.Map_Read)
        return;

    kpWindow->reloadEntityClasses();
    Show_Frame("Entity list reloaded...",false);
}

/*
=================
CmMakeEntity

Convert selected world brush(es) into an entity.
=================
*/
void CmMakeEntity()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->makeEntity();
    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Entity created...",true);
}

/*
=================
CmSelectTotal

Selects those brushes that are totally inside the
bounding box of the seed brush.
=================
*/
void CmSelectTotal()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->selectCompletelyInside();
    set.redrawedit = 1;
    set.redrawxy = 1;

    Show_Frame("Selected brushes totally inside...",true);
}

/*
=================
CmSelectPartial

Select those brushes that are inside or partially overlapping
the seed brush.
=================
*/
void CmSelectPartial()
{
    if (!set.Map_Read)
        return;

    char outstr[80];
    sprintf(outstr,"Selected brushes partially inside...");
    map_i[set.curmap]->selectPartiallyInside();
    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame(outstr,true);
}

/*
=================
CmNextFace

Make next face of current brush the selected face.
=================
*/
void CmNextFace()
{
    if (!set.Map_Read)
        return;

    SetBrush *b = map_i[set.curmap]->selectedBrush();  /// get the first brush...

    if (!b)
        return;

    if (!b->currentFace)   // Make first face current if there is none.
        b->currentFace = b->faces.p_next;

    b->currentFace = b->currentFace->p_next;  // Go to next.
    if (b->currentFace == &b->faces)          // Skip sentinal
        b->currentFace = b->currentFace->p_next;

    // Update the texture browser.
    texturedef_t *td;
    td = &b->currentFace->texture;
    texWindow->setTextureDef(td);

    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Next face...",true);
}

/*
=================
CmPrevFace

=================
*/
void CmPrevFace()
{
    if (!set.Map_Read)
        return;

    SetBrush *b = map_i[set.curmap]->selectedBrush();  /// get the first brush...
    if (!b)
        return;

    if (!b->currentFace)
        b->currentFace = b->faces.p_prev;

    b->currentFace = b->currentFace->p_prev;
    if (b->currentFace == &b->faces)
        b->currentFace = b->currentFace->p_prev;

    texturedef_t *td = &b->currentFace->texture;
    texWindow->setTextureDef(td);

    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Previous face...",true);
}


void CmEditClone()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->cloneSelection();
    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Selection cloned...",true);
}

/*
=================
CmResetAngles

Set roll, pitch and yaw angles back to 0
=================
*/
void CmResetAngles()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    t_Angles *angles = &m->angles[m->cureye];

    angles->roll  = 0.0; // M_PI;
    angles->pitch = 0.0; // 1.5*M_PI;
    angles->yaw   = 0.0;
    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Angles reset for current map...",true);
}

/*
=================
CmEditCopy

Copy selection to "copy" map.
=================
*/
void CmEditCopy()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->copySelection(&copymap,false);
    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Selection copied...",true);
}

/*
=================
CmEditPaste

Paste selected brushes into current map.
=================
*/
void CmEditPaste()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->pasteSelection();
    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Selection pasted...",true);
}

/*
=================
CmRestoreTexture

Toggle texture window between iconic and regular size.
=================
*/
void CmRestoreTexture()
{
    if (!set.Map_Read)
        return;
    if (!texWindow)
        return;

    if (texWindow->IsIconic() || !texWindow->IsWindowVisible())
    {
        texWindow->ShowWindow(SW_RESTORE);
        texWindow->SetFocus();
        Show_Frame("Texture Window Shown...",false);
    }
    else
    {
        HWND hFw = GetFocus();
        HWND hPw = GetParent(hFw);
        if ((hFw != texWindow->hwnd) && (hPw != texWindow->hwnd))
        {
            texWindow->SetWindowPos(0,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
            texWindow->SetFocus();
            Show_Frame("Texture Window Brought to Front...",false);
        }
        else
        {
            texWindow->ShowWindow(SW_HIDE);
            Show_Frame("Texture Window Hidden...",false);
        }
    }
}

/*
=================
CmRestore3D

=================
*/
void CmRestore3D()
{
    if (!set.Map_Read)
        return;
    if (!editWindow)
        return;

    if (editWindow->IsIconic() || !editWindow->IsWindowVisible())
    {
        editWindow->ShowWindow(SW_RESTORE);
        editWindow->SetFocus();
        Show_Frame("3D Window Shown...",false);
    }
    else
    {
        HWND hFw = GetFocus();
        HWND hPw = GetParent(hFw);
        if ((hFw != editWindow->hwnd) && (hPw != editWindow->hwnd))
        {
            editWindow->SetWindowPos(NULL,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
            editWindow->SetFocus();
            Show_Frame("3D Window Brought to Front...",false);
        }
        else
        {
            editWindow->ShowWindow(SW_HIDE);
            Show_Frame("3D Window Hidden...",false);
        }
    }
}

/*
=================
CmRestoreEntity

=================
*/
void CmRestoreEntity()
{
    if (!set.Map_Read)
        return;

    if (!kpWindow)
        return;

    if (kpWindow->IsIconic() || !kpWindow->IsWindowVisible())
    {
        kpWindow->ShowWindow(SW_RESTORE);
        kpWindow->SetFocus();
        Show_Frame("Entity Window Shown...",false);
    }
    else
    {
        HWND hFw = GetFocus();
        HWND hPw = GetParent(hFw);

        if ((hFw != kpWindow->hwnd) && (hPw != kpWindow->hwnd))
        {
            kpWindow->SetWindowPos(0,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
            kpWindow->SetFocus();
            Show_Frame("Entity Window Brought to Front...",false);
        }
        else
        {
            kpWindow->ShowWindow(SW_HIDE);
            Show_Frame("Entity Window Hidden...",false);
        }
    }
}

/*
=================
CmToggleConsole

=================
*/
void CmToggleConsole()
{
    if (wconsole)
    {
        if (wconsole->IsIconic() || !wconsole->IsWindowVisible())
        {
            wconsole->ShowWindow(SW_RESTORE);
            wconsole->SetFocus();
            ::SetFocus(wconsole->input->hwnd);
        }
        else
        {
            HWND current_focus = GetFocus();
            HWND parent_focus = GetParent(current_focus);

            if ((current_focus != wconsole->hwnd) && (parent_focus != wconsole->hwnd))
            {
                wconsole->SetWindowPos(0,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
                wconsole->SetFocus();
                ::SetFocus(wconsole->input->hwnd);
            }
            else
            {
                wconsole->ShowWindow(SW_HIDE);
                frame->SetFocus();
            }
        }
    }
}
/*
=================
CmRot - rotation step size

=================
*/

void CmRot1()
{
    set.rotate_step = 1;
}
void CmRot5()
{
    set.rotate_step = 5;
}
void CmRot10()
{
    set.rotate_step = 10;
}
void CmRot15()
{
    set.rotate_step = 15;
}
void CmRot30()
{
    set.rotate_step = 30;
}
void CmRot45()
{
    set.rotate_step = 45;
}

void CmGrid3D()
{
    set.show_3d_grid ^= 1;

    set.redrawedit = 1;
    set.redrawxy = 1;

    if(set.show_3d_grid)
        Show_Frame("Show 3D grid...",1);
    else
        Show_Frame("Hide 3D grid...",1);
}

void CmAxes3D()
{
    set.show_world_axes ^= 1;

    set.redrawedit = 1;
    set.redrawxy = 1;

    if(set.show_world_axes)
        Show_Frame("Show 3D axes...",1);
    else
        Show_Frame("Hide 3D axes...",1);
}
/*
=================
CmGroup

=================
*/
void CmGroup()
{
    if (!set.Map_Read)
        return;

    if (groupWindow && groupWindow->IsValid())
    {
        if (!groupWindow->IsVisible())
        {
            groupWindow->ShowWindow(SW_RESTORE);
            groupWindow->SetFocus();
            SetWindowPos(client->hwnd,groupWindow->hwnd,0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
            Show_Frame("Group Window Shown...",false);
        }
        else
        {
            HWND hFw = GetFocus();
            HWND hPw = GetParent(hFw);

            if ((hFw != groupWindow->hwnd) && (hPw != groupWindow->hwnd))
            {
                SetWindowPos(client->hwnd,groupWindow->hwnd,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
                groupWindow->SetFocus();
                Show_Frame("Group Window Brought to Front...",false);
            }
            else
            {
                groupWindow->ShowWindow(SW_HIDE);
                Show_Frame("Group Window Hidden...",false);
            }
        }
    }
}

/*
=================
CmGroupMode

Toggle group mode and
make maps reflect the new mode.
=================
*/
void CmGroupMode()
{
    if (!set.Map_Read)
        return;

    // Toggle mode.
    set.group_mode ^= 1;

    // Update maps.
    map *m;
    for (int i = 0; i < set.nummaps; i++)
    {
        m = map_i[i];
        if (!m)
            continue;
        m->UpdateMapVisibility();
    }

    set.redrawxy = 1;
    set.redrawedit = 1;

    if (set.group_mode)
        Show_Frame("Group mode on...",true);
    else
        Show_Frame("Group mode off...",true);
}

/*
=================
CmRegionMode

Toggle use of regioning
=================
*/
void CmRegionMode()
{
    if (!set.Map_Read)
        return;

    set.region_mode ^= 1;

    for (int i = 0; i < set.nummaps; i++)
    {
        if (map_i[i])
            map_i[i]->UpdateMapVisibility();
    }

    set.redrawxy = 1;
    set.redrawedit = 1;

    if (set.region_mode)
        Show_Frame("Region mode on...",true);
    else
        Show_Frame("Region mode off...",true);
}

/*
=================
CmNoRegion

Disable use of regioning
=================
*/
void CmNoRegion()
{
    if (!set.Map_Read)
        return;

    set.region_mode = 0;

    map *m = map_i[set.curmap];
    if (m->regionBrush)
    {
        delete m->regionBrush;
        m->regionBrush = NULL;
    }

    m->makeGlobalPerform(SEL_UNSETREGIONED);
    m->UpdateMapVisibility();

    set.redrawxy = 1;
    set.redrawedit = 1;

    Show_Frame("Region cleared...",true);
}

/*
=================
CmExtrude

Create a set of brushes by extruding each face
of the seed brush away from the center and flipping
it's normal.
=================
*/
void CmExtrude()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->saveForUndo(const_cast<char *> ("Extrude Room"), UNDO_BRUSHES);
    map_i[set.curmap]->extrudeRoom();
    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Extruding selected brush...",true);
}


void CmExtrudeFace()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->saveForUndo(const_cast<char *> ("Extrude Face"), UNDO_BRUSHES);
    map_i[set.curmap]->extrudeFace();
    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Extruding selected face...",true);
}


/*
=================
CmExportQBSP
=================
*/
void CmExportFunc(char *batname)
{
    if (!set.Map_Read)
        return;

    int retval = MessageBox(client->hwnd, const_cast<char *> ("Okay to save map ? "), const_cast<char *> ("BSP - Export"), MB_YESNO | MB_ICONQUESTION);
    if (retval == IDYES)
    {
        CmSave(); // must save first...

        char goodname[128];
        ExtractFileBase(map_i[set.curmap]->filename,goodname,true);

        Path progname(const_cast<char *> ("%s\\%s"), set.bat_directory,batname);
        Bsp_ShellExecute(0, NULL, progname,	goodname, set.map_directory);
        Show_Frame("Spawning QBSP on current .map file...",true);
    }
}

void CmExportQBSP()
{
    CmExportFunc(const_cast<char *> ("super.bat"));
}
void CmExportFull()
{
    CmExportFunc(const_cast<char *> ("full.bat"));
}
void CmExportFast()
{
    CmExportFunc(const_cast<char *> ("fast.bat"));
}
void CmExportNoVis()
{
    CmExportFunc(const_cast<char *> ("novis.bat"));
}
void CmExportReLight()
{
    CmExportFunc(const_cast<char *> ("relight.bat"));
}
void CmExportLeakTest()
{
    CmExportFunc(const_cast<char *> ("leaktest.bat"));
}

/*
=================
SaveArrangement

Saves window positions
=================
*/
void SaveArrangement()
{
    Str buf(260, const_cast<char *> ("%i"), set.curCfg+1);
    Str prompt(const_cast<char *> ("Which slot to save to (1-%i)"), MAX_SAVE_WIN);
    if (IDOK == (intptr_t)InputDialog(client->hwnd, const_cast<char *> ("Save Window Arrangement"), prompt, buf, buf.length).Execute())
    {
        int slot = atoi(buf) - 1;
        if (slot >= 0 && slot < MAX_SAVE_WIN)
            WindowPlacement::SavePositions(slot);
    }
}

/*
=================
CmFlipX

Flip brushes along x axis
=================
*/
void CmFlipX()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->flip_x();
    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Selection flipped on X axis...",true);
}

/*
=================
CmFlipY

=================
*/
void CmFlipY()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->flip_y();
    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Selection flipped on Y axis...",true);
}

/*
=================
CmFlipZ

=================
*/
void CmFlipZ()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->flip_z();
    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Selection flipped on Z axis...",true);
}

/*
=================
CmRotateX

Rotate on x Axis
=================
*/
void CmRotateX()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->rotate_x();
    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Selection rotated 90 degrees on X axis...",true);
}

/*
=================
CmRotateY

=================
*/
void CmRotateY()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->rotate_y();
    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Selection rotated 90 degrees on Y axis...",true);
}

/*
=================
CmRotateZ

=================
*/
void CmRotateZ()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->rotate_z();
    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Selection rotated 90 degrees on Z axis...",true);
}

/*
=================
CmEditMakeStairs

Show the stair dialog.
=================
*/
void CmEditMakeStairs()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];

    if (m->numSelected() != 1)
    {
        Show_Frame("Must only have one selected brush...",false);
        return;
    }

    SetBrush *seed = m->selectedBrush();
    if (!seed || !seed->checkModifiable())
        return;

    Entity *current = m->currentEntity();
    if (!current)
        current = m->world;
    if (!current)
        return; // problem!

    // Run dialog...
    TStairDlg dialog(client->hwnd,&client->stairxfer);
    if (dialog.Execute() != IDOK)
        return;

    int style = STAIR_BLOCK;
    if (client->stairxfer.RegularButton)
        style = STAIR_REGULAR;
    else if (client->stairxfer.SpiralButton)
        style = STAIR_SPIRAL;

    int down     = client->stairxfer.GoesDown;
    int xory     = client->stairxfer.XAxis;
    int steps    = client->stairxfer.NumSteps;
    int hgap     = client->stairxfer.HGap;
    int vgap     = client->stairxfer.VGap;
    float turns  = client->stairxfer.NumTurns;
    int distfc   = client->stairxfer.Dist;

    // error checking...
    steps = max(1,steps);
    hgap = max(1,hgap);
    vgap = max(0,vgap);
    turns = max((float)0.1,turns);
    distfc = max(1,distfc);

    // DO IT...
    vgap = m->makeStairs(current,seed,true,style,steps, hgap,vgap,down,xory,turns,distfc);

    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame(Str(const_cast<char *> ("Made selected brush into stairs, step height %i..."), vgap), true);
}

/*
=================
CmTallBrush

Make a really tall brush.
=================
*/
void CmTallBrush()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->tallBrush();
    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Made selected brush tall...",true);
}

/*
=================
CmHeightBrush

Make brush of specified height.
=================
*/
void CmHeightBrush()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];

    if (m->numSelected() != 1)
    {
        Show_Frame("Must only have one selected brush...",false);
        return;
    }

    // hard code for Z Axis...
    Str buf(200, const_cast<char *> ("%i"),(int)(m->currentMaxZ(2) - m->currentMinZ(2)));
    if (IDOK == (intptr_t)InputDialog(client->hwnd, const_cast<char *> ("Set Brush Height"), const_cast<char *> ("Enter height for brush..."), buf, buf.length).Execute())
    {
        int height = atoi(buf);
        if (height >= 1)
        {
            m->saveForUndo(const_cast<char *> ("Set Brush Height"), UNDO_BRUSHES);

            m->sizeBrush(height);
            set.redrawxy = 1;
            set.redrawedit = 1;
            Show_Frame("Resized height of selected brush...",true);
        }
        else
        {
            Show_Frame("Bad height...",true);
        }
    }
}

/*
=================
CmMakeRoom

Make 6-sided room from brush.
=================
*/
void CmMakeRoom()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->makeRoom();
    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Made selected brush into a room...",true);
}

/*
=================
CmMakeSphere

Makes a sphere.  FIXME, do a solid sphere, too.
=================
*/
void CmMakeSphere()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    if (m->numSelected() != 1)
    {
        Show_Frame("Must only have one selected brush...",false);
        return;
    }
    TSphereDlg dialog(client->hwnd,&client->spherexfer);
    if (dialog.Execute() == IDOK)
    {
        int hs    = atoi(client->spherexfer.HStrips);
        int vs    = atoi(client->spherexfer.VStrips);
        int inner = atoi(client->spherexfer.Inner);
        int outer = atoi(client->spherexfer.Outer);
        int take  = (client->spherexfer.TakeSeed == BST_CHECKED) ? 1:0;
        int percent = min(99,max(1,atoi(client->spherexfer.InnerPercent)));

        hs = max(1,hs);
        hs = min(99,hs);
        vs = max(1,vs);
        vs = min(99,vs);
        inner = max(1,inner);
        outer = max(inner+1,outer); // must be larger...

        m->makeSphere(0,hs,vs,inner,outer,take,percent);
        set.redrawxy = 1;
        set.redrawedit = 1;
        Show_Frame("Made selected brush into a sphere...",true);
    }
    else
    {
        Show_Frame("Couldn't create sphere...",true);
    }
}

/*
=================
CmMakeArch

=================
*/
void CmMakeArch()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    if (m->numSelected() != 1)
    {
        Show_Frame("Must only have one selected brush...",false);
        return;
    }
    SetBrush *seed = m->selectedBrush();
    if (!seed || !seed->checkModifiable())
        return;

    Entity *current = m->currentEntity();
    if (!current)
        current = m->world;

    if (!current)
        return; // problem!

    // Run dialog...
    TArchDlg dialog(client->hwnd,&client->archxfer);
    if (dialog.Execute() == IDOK)
    {
        int xory     = (client->archxfer.XAxis == BST_CHECKED);
        int rect     = (client->archxfer.RectTop == BST_CHECKED);
        int stones   = atoi(client->archxfer.Stones);
        int width    = atoi(client->archxfer.width);

        // error checking
        stones = max(3,stones);
        width = max(1,width);

        // make them...
        m->makeArch(current,seed,true,stones,xory,width,rect);
        set.redrawxy = 1;
        set.redrawedit = 1;
        Show_Frame("Made selected brush into arch...",true);
    }
    else
    {
        Show_Frame("Couldn't create arch",true);
    }
}

/*
=================
CmMakeWedge

=================
*/
void CmMakeWedge()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    if (m->numSelected() != 1)
    {
        Show_Frame("Must only have one selected brush...",false);
        return;
    }

    SetBrush *seed = m->selectedBrush();
    if (!seed || !seed->checkModifiable())
        return;

    Entity *current = m->currentEntity();
    if (!current)
        current = m->world;

    if (!current)
        return; // problem!

    char outstr[80];

    // Run dialog...
    TWedgeDlg dialog(client->hwnd,&client->wedgexfer);
    if (dialog.Execute() == IDOK)
    {
        int axis = atoi(client->wedgexfer.Axis);
        axis = min(axis,2);
        axis = max(axis,0);

        // make them...
        m->makeWedge(current,seed,true,axis,client->wedgexfer.flip);
        set.redrawxy = 1;
        set.redrawedit = 1;
        strcpy(outstr,"Made selected brush into wedge...");
    }
    else
    {
        strcpy(outstr,"Couldn't create wedge");
    }
    Show_Frame(outstr,true);
}

/*
=================
CmMakeNSided

=================
*/
void CmMakeNSided()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    if (m->numSelected() != 1)
    {
        Show_Frame("Must only have one selected brush...",false);
        return;
    }

    char *outstr;

    TNDlg dialog(client->hwnd,&client->nxfer);
    if (dialog.Execute() == IDOK)
    {
        int xalign = (client->nxfer.XAlign == BST_CHECKED);
        int sides = atoi(client->nxfer.Sides);
        int radius = atoi(client->nxfer.Radius);
        int take = (client->nxfer.TakeSeed == BST_CHECKED) ? 1:0;

        // error checking...
        sides = max(3,sides);
        radius = max(1,radius);

        // make them...
        m->makeNSided(sides, radius, xalign, take);
        set.redrawxy = 1;
        set.redrawedit = 1;
        outstr = const_cast<char *> ("Made selected brush into N-sided brush...");
    }
    else
    {
        outstr = const_cast<char *> ("Couldn't create N-sided brush...");
    }
    Show_Frame(outstr,true);
}

/*
=================
CmEditScale

=================
*/
void CmEditScale()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    if (!m->numSelected())
    {
        Show_Frame("Must have at least one selected brush...",false);
        return;
    }

    char *outstr;

    // Run dialog...
    TScaleDlg dialog(client->hwnd,&client->scalexfer);
    if (dialog.Execute() == IDOK)
    {
        float sx  = (float) atof(client->scalexfer.ScaleX);
        float sy  = (float) atof(client->scalexfer.ScaleY);
        float sz  = (float) atof(client->scalexfer.ScaleZ);

        if (fabs(sx) < FP_EPSILON) sx = 1.0f;
        if (fabs(sy) < FP_EPSILON) sy = 1.0f;
        if (fabs(sz) < FP_EPSILON) sz = 1.0f;
        // make them...
        m->scaleBrush(sx, sy, sz);
        set.redrawxy = 1;
        set.redrawedit = 1;
        outstr = const_cast<char *> ("Scaled selected brush...");
    }
    else
    {
        outstr = const_cast<char *> ("Couldn't scale brush...");
    }
    Show_Frame(outstr,true);
}

/*
=================
CmMakeBrush

=================
*/
void CmMakeBrush()
{
    if (!set.Map_Read)
        return;

    char outstr[80];

    // Run dialog...
    TCreateDlg dialog(client->hwnd,&client->createxfer);
    if (dialog.Execute() == IDOK)
    {
        vec3_t mn;
        vec3_t mx;

        mn[0]  = (float) atof(client->createxfer.X);
        mn[1]  = (float) atof(client->createxfer.Y);
        mn[2]  = (float) atof(client->createxfer.Z);
        mx[0]  = mn[0] + (float) max(atof(client->createxfer.DX),1.0f);
        mx[1]  = mn[1] + (float) max(atof(client->createxfer.DY),1.0f);
        mx[2]  = mn[2] + (float) max(atof(client->createxfer.DZ),1.0f);

        Entity *owner = map_i[set.curmap]->current;
        texturedef_t td;
        texWindow->getTextureDef(&td);
        int newindex = 0;
        if (groupWindow)
        {
            newindex = groupWindow->GroupList->GetSelIndex();
            if (newindex < 0 || newindex >= groupWindow->GroupList->GetCount()) // name field not one of the groups...
                newindex = 0; // put in default if no group is current...
        }

        SetBrush *b = new SetBrush();
        b->group = newindex;
        b->initOwner(owner,mn,mx,&td);
        if (!b->IsInvalid())
        {
            b->setSelected(true);
            owner->addObject(b);
        }
        else
        {
            delete b;
        }
        set.redrawxy = 1;
        set.redrawedit = 1;
        strcpy(outstr,"Brush added...");
    }
    else
    {
        strcpy(outstr,"Couldn't create brush...");
    }
    Show_Frame(outstr,true);
}

/*
=================
CmMakePyramid

=================
*/
void CmMakePyramid()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    if (m->numSelected() != 1)
    {
        Show_Frame("Must only have one selected brush...",false);
        return;
    }

    char outstr[80];

    // Run dialog...
    TPyramidDlg dialog(client->hwnd,&client->pyramidxfer);
    if (dialog.Execute() == IDOK)
    {
        int xalign = (client->pyramidxfer.XAlign == BST_CHECKED);
        int take  = (client->pyramidxfer.TakeSeed == BST_CHECKED) ? 1:0;

        int sides    = atoi(client->pyramidxfer.Sides);
        int radius   = atoi(client->pyramidxfer.Radius);
        int height   = atoi(client->pyramidxfer.Height);

        // error check...
        sides = max(3,sides);
        radius = max(1,radius);
        height = max(1,height);

        // make them...
        m->makePyramid(sides, radius, height, xalign, take);
        set.redrawxy = 1;
        set.redrawedit = 1;
        strcpy(outstr,"Made selected brush into pyramid...");
    }
    else
    {
        strcpy(outstr,"Couldn't create pyramid...");
    }
    Show_Frame(outstr,true);
}

/*
=================
CmMakeCylinder

=================
*/
void CmMakeCylinder()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    if (m->numSelected() != 1)
    {
        Show_Frame("Must only have one selected brush...",false);
        return;
    }

    char outstr[80];

    // Run dialog...
    TCylDlg dialog(client->hwnd,&client->cylxfer);
    if (dialog.Execute() == IDOK)
    {
        int sides    = atoi(client->cylxfer.Sides);
        int strips   = atoi(client->cylxfer.Strips);
        int inner    = atoi(client->cylxfer.Inner);
        int outer    = atoi(client->cylxfer.Outer);
        int take  = (client->cylxfer.TakeSeed == BST_CHECKED) ? 1:0;
        int xalign  = (client->cylxfer.XAlign == BST_CHECKED) ? 1:0;
        int percent = atoi(client->cylxfer.InnerPercent);
        percent = max(1,percent);
        percent = min(99,percent);

        sides = max(3,sides);
        strips = max(1,strips);
        inner = max(1,inner);
        outer = max(inner+1,outer);

        // make them...
        m->makeCylinder(sides, strips, inner, outer, take, percent, xalign);
        set.redrawxy = 1;
        set.redrawedit = 1;
        strcpy(outstr,"Made selected brush into cylinder...");
    }
    else
    {
        strcpy(outstr,"Couldn't create cylinder");
    }
    Show_Frame(outstr,true);
}

/*
=================
CmOmitDetail

These just cause a recalculation of brush
visibility based on the current flags.
=================
*/
void CmOmitDetail()
{
    set.filter_detail ^= 1;

    if (!set.Map_Read)
        return;

    RecalcFiltered();

    set.redrawxy   = 1;
    set.redrawedit = 1;
    Show_Frame("Show detail brushes toggled",true);
}

void CmOmitHint()
{
    set.filter_hint ^= 1;

    if (!set.Map_Read)
        return;

    RecalcFiltered();

    set.redrawxy   = 1;
    set.redrawedit = 1;
    Show_Frame("Show hint brushes toggled",true);
}

void CmOmitLights()
{
    set.filter_light ^= 1;

    if (!set.Map_Read)
        return;

    RecalcFiltered();

    set.redrawxy   = 1;
    set.redrawedit = 1;
    Show_Frame("Show lights toggled",true);
}

void CmOmitUnselected()
{
    set.filter_unselected ^= 1;

    if (!set.Map_Read)
        return;

    RecalcFiltered();

    set.redrawxy   = 1;
    set.redrawedit = 1;
    Show_Frame("Show unselected toggled",true);
}

void CmOmitSky()
{
    set.filter_sky ^= 1;

    if (!set.Map_Read)
        return;

    RecalcFiltered();

    set.redrawxy   = 1;
    set.redrawedit = 1;
    Show_Frame("Show sky brushes toggled",true);
}

void CmOmitTarget()
{
    set.filter_target ^= 1;

    if (!set.Map_Read)
        return;

    RecalcFiltered();

    set.redrawxy   = 1;
    set.redrawedit = 1;
    Show_Frame("Show target/targename entities toggled",true);
}

void CmOmitEntities()
{
    set.filter_entities ^= 1;

    if (!set.Map_Read)
        return;

    RecalcFiltered();
    set.redrawxy   = 1;
    set.redrawedit = 1;
    Show_Frame("Show entities toggled",true);
}

void CmOmitWorld()
{
    set.filter_world ^= 1;

    if (!set.Map_Read)
        return;

    RecalcFiltered();

    set.redrawxy   = 1;
    set.redrawedit = 1;
    Show_Frame("Show world toggled",true);
}

void CmOmitWater()
{
    set.filter_water_brushes ^= 1;

    if (!set.Map_Read)
        return;

    RecalcFiltered();

    set.redrawxy   = 1;
    set.redrawedit = 1;
    Show_Frame("Show water toggled",true);
}

void CmOmitClip()
{
    set.filter_clip_brushes ^= 1;

    if (!set.Map_Read)
        return;

    RecalcFiltered();

    set.redrawxy   = 1;
    set.redrawedit = 1;
    Show_Frame("Show clip brushes toggled",true);
}

/*
TODO: this stuff is wack
=================
CmModePolygon

These toggle the current mode.
=================
*/
void CmModePolygon()
{
    char outstr[80];
    sprintf(outstr,"Polygon Mode");
//	set.currentMode = polygon_Mode;
    Show_Frame(outstr,true);
}

void CmModePoint()
{
    char outstr[80];
    sprintf(outstr,"Point Mode");
//	set.currentMode = point_Mode;
    Show_Frame(outstr,true);
}

void CmModeEdge()
{
    char outstr[80];
    sprintf(outstr,"Edge Mode");
    //	set.currentMode = edge_Mode;
    Show_Frame(outstr,true);
}

void CmModeGroup()
{
    char outstr[80];
    sprintf(outstr,"Group Mode");
//	set.currentMode = group_Mode;
    Show_Frame(outstr,true);
}

/*
=================
CmCLip#

Select clip point.
=================
*/

void SelectClipPointFunc(int point)
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->curclippoint = point-1;

    set.redrawedit = 1;
    set.redrawxy = 1;

    char text[40];
    sprintf(text,"Selecting clip point # %d...",point);
    Show_Frame(text, true);
}

void CmClip1()
{
    SelectClipPointFunc(1);
}
void CmClip2()
{
    SelectClipPointFunc(2);
}
void CmClip3()
{
    SelectClipPointFunc(3);
}

/*
=================
CmShowFrame

Force a refresh.
=================
*/
void CmShowFrame()
{
    if (!set.Map_Read)
        return;

    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Redrawing...",1);
}


/*
=================
CmSetBrushDepth

Set newbrush depth
=================
*/
void CmSetBrushDepth()
{
    char s[80];
    sprintf(s,"%i",set.new_brush_depth);

    if (InputDialog(client->hwnd, const_cast<char *> ("Set New Brush Depth"), const_cast<char *> ("Enter brush depth (0=auto)"), s, sizeof(s)).Execute() != IDOK)
        return;

    int val = atoi(s);
    if(val >= 0)
        set.new_brush_depth = val;

}
/*
=================
Cm#

Select hit brush
=================
*/

void CmHitFunc(int num)
{
    if (!set.Map_Read)
        return;

    char outstr[80];
    sprintf(outstr,"Selecting hit brush # %d...",num);

    SelectHitBrush(num);

    set.redrawedit = 1;
    set.redrawxy = 1;

    Show_Frame(outstr,1);
}

void CmHit0()
{
    CmHitFunc(0);
}
void CmHit1()
{
    CmHitFunc(1);
}
void CmHit2()
{
    CmHitFunc(2);
}
void CmHit3()
{
    CmHitFunc(3);
}
void CmHit4()
{
    CmHitFunc(4);
}
void CmHit5()
{
    CmHitFunc(5);
}
void CmHit6()
{
    CmHitFunc(6);
}
void CmHit7()
{
    CmHitFunc(7);
}
void CmHit8()
{
    CmHitFunc(8);
}
void CmHit9()
{
    CmHitFunc(9);
}

/*
=================
RecalcFiltered

Loops through all maps and recalculates whether the brush is visible.
=================
*/
void RecalcFiltered()
{
    if (!set.Map_Read)
        return;

    for (int i=0; i < set.nummaps; i++)
        if (map_i[i])
            map_i[i]->makeGlobalPerform(SEL_SETFILTERED);
}




