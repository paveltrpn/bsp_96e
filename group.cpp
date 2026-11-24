// Implementation of Group Class
#include "global.h"

static int group_sel_index = -1;

//group tb commands

void CmGroupNewGroup()
{
    groupWindow->GroupList->MenuNew();
}
void CmGroupNewSpacer()
{
    groupWindow->GroupList->MenuNewSpacer();
}
void CmGroupColor()
{
    groupWindow->GroupList->MenuColor();
}
void CmGroupDelete()
{
    groupWindow->GroupList->MenuDelete();
}
void CmGroupAddSel()
{
    groupWindow->GroupList->MenuMoveto();
}
void CmGroupSelectAll()
{
    groupWindow->GroupList->MenuSelectAll();
}
void CmGroupGoto()
{
    groupWindow->GroupList->MenuGotoGroup();
}
void CmGroupVisible()
{
    groupWindow->GroupList->MenuVisible();
}
void CmGroupRename()
{
    groupWindow->GroupList->MenuRename();
}
void CmGroupUp()
{
    groupWindow->GroupList->MenuMoveGroupUp();
}
void CmGroupDown()
{
    groupWindow->GroupList->MenuMoveGroupDown();
}
void CmGroupSingle()
{
    groupWindow->GroupList->MenuUngroupedOnly();
}

// enabler helpers
int group_ce_idx = -1;
int group_ce_cnt = -1;
void GroupEnableCmd(int id, bool enable)
{
    SendMessage(groupWindow->toolbar->hwnd,TB_ENABLEBUTTON, id, (LPARAM)(WORD)enable);	//update toolbar
}
void UpdateGroupEnablers()
{
    group_ce_idx = group_sel_index;						// get current index
    group_ce_cnt = map_i[set.curmap]->groups->count;	// get count
    Ccmd_UpdateEnablers();								// run update
}
//enablers
void CeGroupNeedSel(int id)
{
    GroupEnableCmd(id, (group_ce_idx >= 0 && group_ce_idx < group_ce_cnt));
}
void CeGroupMoveUp(int id)
{
    GroupEnableCmd(id,  (group_ce_idx >= 2 && group_ce_idx < group_ce_cnt));
}
void CeGroupMoveDown(int id)
{
    GroupEnableCmd(id, (group_ce_idx >= 1 && group_ce_idx < group_ce_cnt - 1));
}

//
//  Group List Box
//
LRESULT TBSPGroupListBox::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    static bool mdown = false;

    switch(msg)
    {
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case GB1:
            MenuUseGroups();
            return 0;
        case GB2:
            MenuVisible();
            return 0;
        case GB3:
            MenuColor();
            return 0;
        case GB4:
            MenuNew();
            return 0;
        case GB5:
            MenuRename();
            return 0;
        case GB6:
            MenuDelete();
            return 0;
        case GB7:
            MenuHide();
            return 0;
        case GB8:
            MenuMoveto();
            return 0;
        case GB9:
            MenuAllVisible();
            return 0;
        case GB91:
            MenuHideAll();
            return 0;
        case GB10:
            MenuUngroupedOnly();
            return 0;
        case GB11:
            MenuSelectAll();
            return 0;
        case GB12:
            MenuExportCurrent();
            return 0;
        case GB13:
            MenuExportVisible();
            return 0;
        case GB15:
            MenuMoveGroupUp();
            return 0;
        case GB16:
            MenuMoveGroupDown();
            return 0;
        case GB17:
            MenuNewSpacer();
            return 0;
        case GB18:
            MenuGotoGroup();
            return 0;
        case GB19:
            set.group_toolbar ^= 1;
            ::ShowWindow(groupWindow->toolbar->hwnd, set.group_toolbar ? SW_SHOW : SW_HIDE);
            groupWindow->Resize();
            return 0;
        }
        break;
    case WM_LBUTTONDOWN:
    {
        MAKEPOINT(pt, lParam);
        SetTempValues(&pt);
        SetCapture(hwnd);
        mdown = true;
    }
    break;
    case WM_LBUTTONUP:
    {
        MAKEPOINT(pt, lParam);
        SetTempValues(&pt);
        ReleaseCapture();
        mdown = false;
    }
    break;
    case WM_MOUSEMOVE:
        if(mdown)
        {
            MAKEPOINT(pt, lParam);
            SetTempValues(&pt);
        }
        break;
    case WM_RBUTTONDOWN:
    {
        MAKEPOINT(pt, lParam);
        EvRButtonDown(&pt);
    }
    return 0;
    case LB_SETCURSEL:
        group_sel_index = wParam;
        UpdateGroupEnablers();
        groupWindow->RedrawAll();
        break;
    }
    return WControl::WndProc(msg,wParam,lParam);
}

TBSPGroupListBox::TBSPGroupListBox(HWND parent, int id, int x, int y, int w, int h)
    : WList(parent, id, x, y, w, h)
{
    attr.dwStyle |= WS_BORDER | LBS_SORT | LBS_NOTIFY | WS_VSCROLL | LBS_OWNERDRAWFIXED;
    memset(tempGroup,0,MAX_GROUP_NAMESIZE);
}


bool TBSPGroupListBox::SetTempValues(LPPOINT pt)
{
    int top = GetTopIndex();
    int iHeight = GetItemHeight(0);
    int idx = (int)(pt->y/iHeight) + top;
    onPair = false;
    onSpacer = false;
    map *m = map_i[set.curmap];

    *tempGroup = 0;

    if (idx < 0 || idx >= GetCount())
    {
        return false;
    }

    SetSelIndex(idx);
    groupWindow->curindex = idx;

    STRNCPY(tempGroup,m->groups->GetName(idx));

    if (!tempGroup || !*tempGroup)
    {
        return false;
    }
    onPair = true;
    onSpacer = m->groups->IsSpacer(idx);
    return onPair;
}

void TBSPGroupListBox::EvRButtonDown(LPPOINT point)
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    SetTempValues(point);

    char tempStr[256];

    Popup menu;

    // Use Groups...
    if (set.group_mode)
        menu.Check();
    menu.Add("Enable Groups",GB1);
    menu.Add("Hide Group Window", GB7);
    if(set.group_toolbar)
        menu.Add("Hide Toolbar", GB19);
    else
        menu.Add("Show Toolbar", GB19);

    menu.Separator();
    int idx = group_sel_index;
    if (idx >= 1 && idx < m->groups->count)
    {
        if (idx >= 2)
        {
            menu.Add("Move Up", GB15);
            menu.Separator();
        }
        if (idx < m->groups->count-1)
        {
            menu.Add("Move Down", GB16);
            menu.Separator();
        }
    }

    menu.Add("Add New Group...", GB4);
    menu.Add("Add New Spacer...", GB17);
    menu.Separator();

    // Visible...
    if (onPair)
    {
        if (!onSpacer)
        {
            sprintf(tempStr,"Visible [%s]",tempGroup);
            if (m->groups->GetVisible(idx))
                menu.Check();
            menu.Add(tempStr,GB2);
            menu.Separator();
        }

        sprintf(tempStr,"Set Color [%s]...",tempGroup);
        menu.Add(tempStr, GB3);

        sprintf(tempStr,"Rename [%s]...",tempGroup);
        menu.Add(tempStr, GB5);

        sprintf(tempStr,"Delete [%s]",tempGroup);
        menu.Add(tempStr, GB6);

        // Selection..
        if (!onSpacer)
        {
            sprintf(tempStr,"Select All in [%s]",tempGroup);
            menu.Add(tempStr, GB11);

            menu.Separator();

            sprintf(tempStr,"Move Selection to [%s]",tempGroup);
            menu.Add(tempStr, GB8);

            //goto group
            sprintf(tempStr,"Goto [%s] in XY",m->groups->GetName(idx));
            menu.Add(tempStr, GB18);
        }
        menu.Separator();
    }
    // Visibility
    menu.Add("Show All", GB9);
    menu.Add("Hide All", GB91);
    if (onPair)
    {
        if (!onSpacer)
        {
            sprintf(tempStr,"Show only [%s]",m->groups->GetName(idx));
            menu.Add(tempStr, GB10);

            // Export...
            menu.Separator();
            sprintf(tempStr,"Export [%s]",tempGroup);
            menu.Add(tempStr, GB12);
        }
    }
    else
    {
        menu.Separator();
    }

    menu.Add("Export Visible", GB13);
    menu.Show(hwnd);
}

void TBSPGroupListBox::MenuMoveGroupUp()
{
    if (!set.Map_Read)
        return;
    int idx = group_sel_index;
    if (!tempGroup || (idx < 2) || idx >= map_i[set.curmap]->groups->count)
        return;

    map_i[set.curmap]->groups->MoveGroupUp(idx);

    groupWindow->LoadData();
    groupWindow->curindex--;
    groupWindow->GroupList->SetSelIndex(groupWindow->curindex);
    groupWindow->RedrawAll();
}

void TBSPGroupListBox::MenuMoveGroupDown()
{
    if (!set.Map_Read)
        return;
    int idx = group_sel_index;
    if (!tempGroup || idx < 1 || idx >= map_i[set.curmap]->groups->count - 1)
        return;

    map_i[set.curmap]->groups->MoveGroupDown(idx);

    groupWindow->LoadData();
    groupWindow->curindex++;
    groupWindow->GroupList->SetSelIndex(groupWindow->curindex);
    groupWindow->RedrawAll();
}

void TBSPGroupListBox::MenuUseGroups()
{
    CmGroupMode();	//call group mode toggle
}

void TBSPGroupListBox::MenuVisible()
{
    int idx = group_sel_index;
    if (!tempGroup || (idx == -1))
        return;
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];

    char testStr[256];
    int vis = m->groups->GetVisible(idx);

    m->groups->SetVisible(idx, !vis);
    m->UpdateMapVisibility();

    STRNCPY(testStr,m->groups->GetName(idx));

    groupWindow->GroupList->DeleteString(idx);
    group_t *dat = &m->groups->objects[idx];
    groupWindow->GroupList->InsertString((LPSTR)dat,idx);
    groupWindow->GroupList->SetSelIndex(idx);
    groupWindow->curindex = idx;
    groupWindow->RedrawAll();
}

void TBSPGroupListBox::MenuColor()
{
    int idx = group_sel_index;
    if (!tempGroup || (idx == -1))
        return;

    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    COLORREF groupcolor = m->groups->GetGroupColor(idx);

    ColorDialog cd(hwnd, groupcolor, const_cast<char *> ("Choose Group Color"));

    if (cd.Show())
    {
        m->groups->SetGroupColor(idx,cd.result);
    }

    char str[MAX_GROUP_NAMESIZE];
    STRNCPY(str,m->groups->GetName(idx));
    DeleteString(idx);
    group_t *dat = &m->groups->objects[idx];
    InsertString((LPSTR)dat,idx);
    SetSelIndex(idx);
    groupWindow->RedrawAll();
}

void TBSPGroupListBox::MenuNew()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    char buffer[MAX_GROUP_NAMESIZE] = "";

    bool ok = (InputDialog(hwnd, const_cast<char *> ("Create Group") ,const_cast<char *> ("Group Name"), buffer, sizeof(buffer)).Execute() == IDOK);
    if (!ok || !buffer[0])
        return;

    int newindex = m->groups->GetIndex(buffer);
    if (newindex != -1) // name already exists...
        return;

    newindex = m->groups->AddGroup(buffer);
    m->groups->AddSelectedToGroup(newindex);

    if (set.Map_Read)
    {
        if (!set.multi_select)
            m->makeSelectedPerform(SEL_DESELECT);
        m->setCurrentEntity(m->world);
    }

    ColorDialog cd(0, RGB(0, 0, 255), const_cast<char *> ("Choose Group Color"));

    if (cd.Show())
    {
        m->groups->SetGroupColor(newindex,cd.result);
    }

    STRNCPY(buffer,m->groups->GetName(newindex));

    group_t *dat = &m->groups->objects[newindex];
    groupWindow->GroupList->InsertString((LPSTR)dat,newindex);
    groupWindow->curindex = newindex;
    groupWindow->GroupList->SetSelIndex(newindex);
    groupWindow->RedrawAll();
}

void TBSPGroupListBox::MenuNewSpacer()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    char buffer[MAX_GROUP_NAMESIZE];
    memset(buffer,0,MAX_GROUP_NAMESIZE);

    bool ok = (InputDialog(hwnd, const_cast<char *> ("Create Spacer"), const_cast<char *> ("Spacer Title"), buffer, sizeof(buffer)).Execute() == IDOK);
    if (!ok || !buffer[0])
        return;

    int newindex = m->groups->GetIndex(buffer);
    if (newindex != -1) // name already exists...
        return;

    newindex = m->groups->AddSpacer(buffer);

    ColorDialog cd(hwnd, RGB(0, 0, 255), const_cast<char *> ("Choose Spacer Color"));

    if (cd.Show())
    {
        m->groups->SetGroupColor(newindex,cd.result);
    }

    STRNCPY(buffer,m->groups->GetName(newindex));

    group_t *dat  = &m->groups->objects[newindex];
    groupWindow->GroupList->InsertString((LPSTR)dat,newindex);
    groupWindow->curindex = newindex;
    groupWindow->GroupList->SetSelIndex(newindex);
    groupWindow->RedrawAll();
}

void TBSPGroupListBox::MenuDelete()
{
    int idx = group_sel_index;
    if (!tempGroup || (idx == -1))
        return;
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    char prompt[128];
    if (m->groups->IsSpacer(idx))
    {
        sprintf(prompt,"Delete spacer [%s]?",tempGroup);
        if (MessageBox(hwnd, prompt, "BSP - Delete Spacer", MB_OKCANCEL | MB_ICONQUESTION) != IDOK)
            return;
    }
    else
    {
        sprintf(prompt,"Delete group [%s]?",tempGroup);
        if (MessageBox(hwnd, prompt, "BSP - Group Delete", MB_OKCANCEL | MB_ICONQUESTION) != IDOK)
            return;
    }

    // set back to group 0
    groupWindow->curindex = 0;
    m->groups->DeleteGroup(idx);
    groupWindow->LoadData();
    groupWindow->GroupList->SetSelIndex(0);
    groupWindow->RedrawAll();
}

void TBSPGroupListBox::MenuRename()
{
    int idx = group_sel_index;
    if (!tempGroup || (idx == -1))
        return;
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    char testStr[256];
    char buffer[MAX_GROUP_NAMESIZE];
    char prompt[80];
    sprintf(prompt,"Old name:  [%s]",tempGroup);
    STRNCPY(buffer,tempGroup);

    bool ok = (InputDialog(hwnd, const_cast<char *> ("New Name"), prompt, buffer, sizeof(buffer)).Execute() == IDOK);
    if (ok)
    {
        if (buffer[0])
        {
            m->saveForUndo(const_cast<char *> ("Rename Group/Spacer"), UNDO_GROUPS);
            m->groups->Rename(idx,buffer);
        }
    }

    STRNCPY(testStr,m->groups->GetName(idx));
    groupWindow->GroupList->DeleteString(idx);
    group_t *dat = &m->groups->objects[idx];
    groupWindow->GroupList->InsertString((LPSTR)dat, idx);
    groupWindow->GroupList->SetSelIndex(idx);
    groupWindow->curindex = idx;
    groupWindow->RedrawAll();
}

void TBSPGroupListBox::MenuHide()
{
    groupWindow->ShowWindow(SW_SHOWMINIMIZED);
}

void TBSPGroupListBox::MenuMoveto()
{
    int idx = group_sel_index;
    if (!tempGroup || (idx == -1))
        return;
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    // move the selection...
    m->groups->AddSelectedToGroup(idx);
    if (set.group_mode && !m->groups->GetVisible(idx))
    {
        int retval = MessageBox(hwnd, "Brushes are being moved to an invisible group, make destination group visible?",
                                "BSP - Move To Group", MB_YESNO | MB_ICONQUESTION);

        if (retval == IDYES)
        {
            m->groups->SetVisible(idx,true);
            groupWindow->RedrawReload();
        }
        m->UpdateMapVisibility();
    }
    groupWindow->GroupList->SetSelIndex(idx);
    groupWindow->curindex = idx;
    groupWindow->RedrawAll();
}

void TBSPGroupListBox::MenuHideAll()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    int num = m->groups->NumGroups();

    for (int i = 0; i < num; i++)
        m->groups->SetVisible(i,false);

    m->UpdateMapVisibility();
    groupWindow->LoadData();
    groupWindow->GroupList->SetSelIndex(groupWindow->curindex);
    groupWindow->RedrawAll();
}
void TBSPGroupListBox::MenuAllVisible()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    int num = m->groups->NumGroups();

    for (int i = 0; i < num; i++)
        m->groups->SetVisible(i,true);

    m->UpdateMapVisibility();
    groupWindow->LoadData();
    groupWindow->GroupList->SetSelIndex(groupWindow->curindex);
    groupWindow->RedrawAll();
}

void TBSPGroupListBox::MenuUngroupedOnly()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    int num = m->groups->NumGroups();

    int idx = group_sel_index;
    //hide all groups except selected
    for (int i = 0; i < num; i++)
        m->groups->SetVisible(i, (i == idx));

    m->UpdateMapVisibility();
    groupWindow->LoadData();
    groupWindow->GroupList->SetSelIndex(groupWindow->curindex);
    groupWindow->RedrawAll();
}

void TBSPGroupListBox::MenuSelectAll()
{
    if (!set.Map_Read)
        return;
    int idx = group_sel_index;
    if (!tempGroup || (idx == -1))
        return;
    map *m = map_i[set.curmap];

    if (!set.multi_select)
        m->makeSelectedPerform(SEL_DESELECT);
    m->setCurrentEntity(m->world);
    m->groups->SelectGroup(idx);
    m->UpdateCurrent(); // in case selected are not part of world...
    groupWindow->GroupList->SetSelIndex(idx);
    groupWindow->curindex = idx;
    groupWindow->RedrawAll();
}

void TBSPGroupListBox::MenuExportCurrent()
{
    int idx = group_sel_index;
    if (!tempGroup || (idx == -1))
        return;
    if (!set.Map_Read)
        return;

    // ask whether to export the visibles or just the current...
    int retval = MessageBox(hwnd, "Export current group?", "BSP - Group Export", MB_OKCANCEL | MB_ICONQUESTION);

    if (retval == IDOK)
        map_i[set.curmap]->groupExport(idx); // pass a -1 for visible groups...
}

void TBSPGroupListBox::MenuExportVisible()
{
    if (!set.Map_Read)
        return;

    // ask whether to export the visibles or just the current...
    int retval = MessageBox(hwnd, "Export visible groups?", "BSP - Group Export", MB_OKCANCEL | MB_ICONQUESTION);

    if (retval == IDOK)
        map_i[set.curmap]->groupExport(-1); // pass a -1 for visible groups...
}

void TBSPGroupListBox::MenuShowResults()
{
    set.show_results ^= 1;
    groupWindow->RedrawAll();
}

/*
=================
MenuGotoGroup

move XY eye to group center
=================
*/
void TBSPGroupListBox::MenuGotoGroup()
{
    if (!set.Map_Read)
        return;
    int idx = group_sel_index;
    if (!tempGroup || (idx == -1))
        return;

    map *m = map_i[set.curmap];
    vec3_t groupctr = {0, 0, 0};
    int bcnt = 0;
    for (Entity *e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
    {
        for (SetBrush *b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
        {
            if (b->group != idx)
                continue;
            VectorAdd(b->bctr, groupctr, groupctr);
            bcnt++;

        }
    }
    if(!bcnt)
        return;

    VectorScale(groupctr, 1/float(bcnt), groupctr);	// get center average

    VectorCopy(groupctr, xy_eye);	// move xy eye to bctr

    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Moved to group...",true);
}




////////////////
////////////////

Group::Group()    // constructor
{
    count = 1;

    // zero everything out...
    memset(objects,0,sizeof(objects));

    // fill in world data

    strcpy(objects[0].name,"None");
    objects[0].visible = 1;
    objects[0].r = GetRValue(set.color_foreground);
    objects[0].g = GetGValue(set.color_foreground);
    objects[0].b = GetBValue(set.color_foreground);
    objects[0].numberofobjects = 0; // only valid once calculated...
}

Group *Group::copy()    // copy
{
    Group *c = new Group;
    c->count = count;
    // zero everything out...
    memcpy(c->objects,objects,sizeof(objects));
    return c;
}

Group::~Group()
{
//	count = 0;
};

// kill them all
void Group::ClearGroups()
{
    int i;
    SetBrush *b;
    Entity *e;
    map *m = 0;
    if (set.Map_Read)
        m = map_i[set.curmap];

    // never delete group 0...
    for (i = 1; i < count; i++)
    {
        memset(&objects[i],0,sizeof(group_t));
        // now go through map and nix all of the groups

        if (!m) // only if read it in...
            continue;

        for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
        {
            e->group = 0;
            for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
            {
                b->group = 0;
            }
        }
    }
    count = 1;
}

void Group::AddSelectedToGroup(int index)
{
    SetBrush *b;
    Entity *e;

    if (index >= count)
        return;  // bad group #

    if (!set.Map_Read) // only if read it in...
        return;

    map *m = map_i[set.curmap];
    for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
    {
        for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
        {
            if (!b->IsSelected())
                continue;
            b->group = index;
        }
    }
}
// returns the index
int Group::AddGroup(char *name)
{
    if (count >= MAX_GROUPS)
    {
        syserror(const_cast<char *> ("Group:  Too many groups %i"), count);
        return -1;
    }

    STRNCPY(objects[count].name,name);
    objects[count].visible = 1;
    objects[count].r = 0;
    objects[count].g = 0;
    objects[count].b = 0;
    objects[count].numberofobjects = 0;
    count++;

    return (count - 1);
}

// returns the index
int Group::AddSpacer(char *name)
{
    if (count >= MAX_GROUPS)
    {
        syserror(const_cast<char *> ("Group:  Too many groups/spacers %i"), count);
        return -1;
    }

    STRNCPY(objects[count].name,name);
    objects[count].visible = 0;
    objects[count].isSpacer = 1;
    objects[count].r = 0;
    objects[count].g = 0;
    objects[count].b = 0;
    objects[count].numberofobjects = 0;
    count++;

    return (count - 1);
}
// get index of a group by name
// -1 for unfound...
int Group::GetIndex(char *name)
{
    for (int i = 0; i < count; i++)
        if (!strcmp(objects[i].name,name))
            return i;
    return -1;
}

// delete a group, all brushes go to
// unselected group (0)
void Group::DeleteGroup(int index)
{
    if (count <= 1)
        return; // don't delete last one...

    if (index >= count)
        return;  // bad group #

    if (!set.Map_Read) // only if read it in...
        return;

    SetBrush *b;
    Entity *e;
    map *m = map_i[set.curmap];
    // cycle through and move index to None Group
    for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
    {
        for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
        {
            if (b->group != index)
                continue;
            b->group = 0;
        }
    }

    // now go through and close the index gap for every
    // group with an index greater than index
    for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
    {
        for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
        {
            if (b->group > index)
                b->group--;
        }
    }

    // now close up the objects...
    for (int j = index; j < count-1; j++)
        objects[j] = objects[j+1];

    count--;
    memset(&objects[count],0,sizeof(group_t));

}

char *Group::GetName(int index)
{
    if (index < 0 || index >= count)
        return 0;  // invalid
    return objects[index].name;
}


void Group::GetLongName(int index, char *outstr)
{
    if (index < 0 || index >= count)
        *outstr = 0;						// invalid
    else if (objects[index].visible)
        sprintf(outstr,"[V] %s",objects[index].name);
    else
        sprintf(outstr,"[I] %s",objects[index].name);
}

// sets name of group index to newname
void Group::Rename(int index, char *newname)
{
    if (index < 0 || index >= count)
        return;  // invalid

    STRNCPY(objects[index].name,newname);
}

COLORREF Group::GetGroupColor(int index)
{
    if (index < 0 || index >= count)
    {
        return RGB(255,255,255);  // invalid
    }
    return RGB(objects[index].r, objects[index].g, objects[index].b);
}

void Group::SetGroupColor(int index, COLORREF rgb)
{
    if (index < 0 || index >= count)
        return;  // invalid

    objects[index].r = GetRValue(rgb);
    objects[index].g = GetGValue(rgb);
    objects[index].b = GetBValue(rgb);
}

// returns visibility state
int Group::GetVisible(int index)
{
    if (index < 0 || index >= count)
        return false;  // invalid

    if (index < 0) // dummy value, always show it...
        return true;

    return objects[index].visible;
}

// returns visibility state
bool Group::IsSpacer(int index)
{
    if (index < 0 || index >= count)
        return false;  // invalid

    if (index < 0) // dummy value, always show it...
        return false;

    return (0 != objects[index].isSpacer);
}

void Group::SetVisible(int index, int yesno)
{
    if (index < 0 || index >= count)
        return;  // invalid

    objects[index].visible = yesno;
}

// returns number of groups
int Group::NumGroups()
{
    return count;
}

// returns number of ents/brushes with group #
void Group::CalcNumObjects()
{
    int i;
    SetBrush *b;
    Entity *e;

    if (!set.Map_Read)
        return;

    // zero it out...
    for (i = 0; i < count; i++)
        objects[i].numberofobjects = 0;

    map *m = map_i[set.curmap];

    for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
    {
        for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
        {
            if (b->group >=0 && b->group < count)
                objects[b->group].numberofobjects++;
        }
    }
}

int Group::GetNumObjects(int index)
{
    if (index < 0 || index >= count)
        return 0;  // invalid

    return objects[index].numberofobjects;
}

void Group::SelectGroup(int index)
{
    SetBrush *b;
    Entity *e;

    if (index < 0 || index >= count)
        return;  // bad group #
    if (!set.Map_Read) // only if read it in...
        return;

    map *m = map_i[set.curmap];

    for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
    {
        for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
        {
            if (b->group == index)
                b->setSelected(true);
        }
    }
}

void Group::MoveGroupUp(int index)
{
    map_i[set.curmap]->groupSwap(index, index-1);

    group_t temp;
    memcpy(&temp,&objects[index],sizeof(group_t));

    memcpy(&objects[index],&objects[index-1],sizeof(group_t));
    memcpy(&objects[index-1],&temp,sizeof(group_t));
}

void Group::MoveGroupDown(int index)
{
    map_i[set.curmap]->groupSwap(index, index+1);

    group_t temp;
    memcpy(&temp,&objects[index],sizeof(group_t));

    memcpy(&objects[index],&objects[index+1],sizeof(group_t));
    memcpy(&objects[index+1],&temp,sizeof(group_t));
}





////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

LRESULT TGroupWindow::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
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
        switch(HIWORD(wParam))
        {
            //	case LBN_SELCHANGE:
            //	//	EvSelChange();
            //		return 0;
        case LBN_DBLCLK:
            EvSelDouble();
            return 0;
        }

        Ccmd_Execute(LOWORD(wParam));
        UpdateGroupEnablers();

        break;
    case WM_NOTIFY:
    {
        NMHDR *nm = (NMHDR*)lParam;

        //display full info tip on statusbar
        if(nm->code==TBN_HOTITEMCHANGE)
        {
            NMTBHOTITEM *hi = (NMTBHOTITEM*)lParam;

            char *text = Ccmd_GetInfoText(hi->idNew);
            if(text)
                status->SetText(text);
            status->DrawStatusBar();
            break;
        }
        // set tooltip text
        if(nm->code==TTN_GETDISPINFO && nm->idFrom > 0)
        {
            NMTTDISPINFO *tt = (NMTTDISPINFO*)lParam;

            ccmd_t *c = Ccmd_FindCmd(nm->idFrom);
            if(c && c->info_text)
                strncpy(tt->szText, c->info_text, sizeof(tt->szText));
        }
        break;
    }
    case WM_CTLCOLORLISTBOX:
        return (LONG_PTR)(HBRUSH)backcolor;
    case WM_SIZE:
        if (IsIconic() || (wParam == SIZE_MINIMIZED))
            break;
        Resize();
        return 0;
    case WM_PAINT:
    {
        TCWindow::WndProc(msg,wParam,lParam);

        HDC hdc = GetDC(hwnd);
        Paint(hdc);
        ReleaseDC(hwnd, hdc);
    }
    return 0;
    case WM_DRAWITEM:
        EvDrawItem(wParam, (LPDRAWITEMSTRUCT)lParam);
        return 1;
    case WM_MEASUREITEM:
        EvMeasureItem(wParam, (LPMEASUREITEMSTRUCT) lParam);
        return 1;
    }

    return TCWindow::WndProc(msg,wParam,lParam);
}

void TGroupWindow::Resize()
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    SIZE newSz;
    newSz.cx = rc.right - 10;
    newSz.cy = rc.bottom - 10;

    int lbtop = 5;

    if(set.group_toolbar)
    {
        lbtop = 26;
        int width = newSz.cx + 10;
        MoveWindow(toolbar->hwnd, 1,1, width, lbtop-5, true);
    }

    if (newSz.cx > 0 && newSz.cy > 0)
    {
        MoveWindow(GroupList->hwnd,5,lbtop,newSz.cx,newSz.cy - lbtop + 5,true);
        Invalidate();
    }
}
TGroupWindow::TGroupWindow(HWND parent, char *title)
    : TCWindow(parent,0)
{
    attr.width = 250;
    attr.height = 320;

    SetCaption(title);

    groupFont = GetFont("arial", -set.group_font_size);
    curindex = 0;

    backcolor = CreateSolidBrush(set.color_background);
}

void TGroupWindow::SetupWindow()
{
    GroupList = new TBSPGroupListBox(hwnd, IDC_COMBOBOX1, 5, 5, 0, 0);  // list of groups
    GroupList->attr.dwStyle |= LBS_NOINTEGRALHEIGHT;
    GroupList->Create();
    GroupList->SendMessage(WM_SETFONT,(WPARAM)groupFont,0);

    toolbar = new WToolbar(hwnd, 16,15, 16,15);
    ::SendMessage(toolbar->hwnd,TB_SETIMAGELIST,0,(LPARAM)imagelist.imsmall);


    ////////////////////////////////////////////////////////
    // PARSE GROUPBAR.cfg

    // Open the groupbar.cfg file.
    char *dat = (char*) file_get_contents(const_cast<char *> ("settings\\groupbar.cfg"));	  // File buffer.
    if (!dat)
    {
        syserror(const_cast<char *> ("Group Window: Error opening settings\\groupbar.cfg"));
        return;
    }

    Tokenizer script(dat);			// Tokenize it.

    // Now parse in the definitions.
    while(1)
    {
        if (!script.next(true))			// No more tokens.
            break;						// done...
        if (strcmpi (script.token, "bar"))		// Should start with "bar"
            break;
        if (!script.next(false))			// Should still have function name on same line.
            break;						// now parse text...

        if (!strcmpi(script.token,"Separator"))
        {
            toolbar->AddSeparator();
        }
        else
        {
            ccmd_t *c = Ccmd_FindCmd(script.token);
            if(c)
            {
                toolbar->AddButton(c->image, c->id);
            }
        }
    }
    delete[] dat;
    /////////////////////////////////////////////////////////

    ::ShowWindow(toolbar->hwnd, set.group_toolbar ? SW_SHOW : SW_HIDE);

    LoadData();
}
//WindowPlacement
HWND TGroupWindow::WP_GetHwnd()
{
    return hwnd;
}
const char *TGroupWindow::WP_WindowName()
{
    return "Group";
}

bool TGroupWindow::CanClose()
{
    if (!KillAllWindows)
    {
        ShowWindow(SW_MINIMIZE);
    }
    return (0 != KillAllWindows);
}


TGroupWindow::~TGroupWindow()
{
    delete toolbar;
    GroupList->ClearList();
    delete GroupList;
    DeleteObject(groupFont);
    DeleteObject(backcolor);
}

void TGroupWindow::LoadData()
{
    if (!set.Map_Read)
        return;

    GroupList->ClearList();

    map *m = map_i[set.curmap];
    if (!m)
        return;

    int num = m->groups->NumGroups();

    for (int i = 0; i < num; i++)
    {
        group_t *dat;
        dat = &m->groups->objects[i];
        GroupList->InsertString((LPSTR)dat,i);
    }
}


void TGroupWindow::EvDrawItem(UINT, LPDRAWITEMSTRUCT dis)
{
    if (!set.Map_Read)
        return;

    // compute the size of the focus rectangle
    RECT rect;
    rect.top = dis->rcItem.top;
    rect.left = dis->rcItem.left;
    rect.bottom = dis->rcItem.bottom;
    rect.right = dis->rcItem.right;

    group_t *data = (group_t*) dis->itemData;
    if (!data || (intptr_t)data == -1)
        return;

    int textleft = rect.left + 10;

    if (data->isSpacer)
        textleft = rect.left;

    HDC hdc = dis->hDC;

    bool isdark = data->r<100 && data->g<80 && data->b < 128;

    COLORREF bcolor = isdark ? RGB(255,255,255) : RGB(0,0,0);

    const COLORREF COL = RGB(data->r, data->g, data->b);


    if (dis->itemState & ODS_SELECTED)
    {
        SetTextColor(hdc, bcolor);
        HBRUSH colbrush = CreateSolidBrush(COL);
        FillRect(hdc, &dis->rcItem, colbrush);
        DeleteObject(colbrush);
    }
    else
    {
        COLORREF tmp = isdark ? RGB(255,255,255) : COL;
        SetTextColor(hdc, tmp);
        HBRUSH brush = CreateSolidBrush(set.color_background);
        FillRect(hdc, &dis->rcItem, brush);
        DeleteObject(brush);
    }

    HPEN pen = CreatePen(PS_SOLID,1,RGB(128,128,128));
    HGDIOBJ oldpen = SelectObject(hdc,pen);
    MoveToEx(hdc, dis->rcItem.left, dis->rcItem.bottom-1, 0);
    LineTo(hdc, dis->rcItem.right, dis->rcItem.bottom-1);


    if (!data->isSpacer)
    {

        HBRUSH graybrush = CreateSolidBrush(RGB(128,128,128));
        HBRUSH blackbrush = CreateSolidBrush(RGB(0,0,0));
        HBRUSH colbrush = CreateSolidBrush(COL);

        RECT rc;
        // display the icon
        rc.left = dis->rcItem.left + 1;
        rc.top = dis->rcItem.top + 1;
        rc.right = dis->rcItem.left + 10;
        rc.bottom = dis->rcItem.bottom - 2;
        FillRect(hdc, &rc, colbrush);

        rc.left = dis->rcItem.left + 2;
        rc.top = dis->rcItem.top + 2;
        rc.right = dis->rcItem.left + 9;
        rc.bottom = dis->rcItem.bottom - 3;

        FrameRect(hdc, &rc, graybrush);

        if (!data->visible)
        {

            rc.left = dis->rcItem.left + 3;
            rc.top = dis->rcItem.top + 3;
            rc.right = dis->rcItem.left + 8;
            rc.bottom = dis->rcItem.bottom - 4;

            FillRect(hdc, &rc, blackbrush);
        }
        else
        {
            HPEN bpen = CreatePen(PS_SOLID,1,bcolor);
            HGDIOBJ oldbpen = SelectObject(hdc,bpen);
            MoveToEx(hdc, dis->rcItem.left + 3, dis->rcItem.top + 3, 0);
            LineTo(hdc, dis->rcItem.left + 8, dis->rcItem.top + 8);
            MoveToEx(hdc, dis->rcItem.left + 7, dis->rcItem.top + 3, 0);
            LineTo(hdc, dis->rcItem.left + 2, dis->rcItem.top + 8);
            SelectObject(hdc,oldbpen);
            DeleteObject(bpen);
        }

        DeleteObject(graybrush);
        DeleteObject(blackbrush);
        DeleteObject(colbrush);
    }

    // display the text

    SetBkColor(hdc, bcolor);
    SetBkMode(hdc, TRANSPARENT);
    HGDIOBJ oldfont = SelectObject(hdc, groupFont);

    TextOut(hdc, textleft + 2, dis->rcItem.top, data->name, strlen(data->name) );

    if (dis->itemState & ODS_FOCUS)
        DrawFocusRect(hdc, &rect);

    SelectObject(hdc,oldfont);
    SelectObject(hdc,oldpen);
    DeleteObject(pen);
}

void TGroupWindow::EvMeasureItem(UINT, LPMEASUREITEMSTRUCT mis)
{
    int height = set.group_font_size;
    mis->itemHeight = height + 3;
}

void TGroupWindow::EvSelDouble()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    char str[MAX_GROUP_NAMESIZE];
    int index = group_sel_index;

    if (index > m->groups->NumGroups())
        return;

    groupWindow->curindex = index;

    if (m->groups->IsSpacer(index))
        return;

    m->groups->SetVisible(index, !m->groups->GetVisible(index));	//toggle

    m->UpdateMapVisibility();

    STRNCPY(str,m->groups->GetName(index));
    GroupList->DeleteString(index);
    group_t *dat = &m->groups->objects[index];
    groupWindow->GroupList->InsertString((LPSTR)dat,index);
    GroupList->SetSelIndex(index);
    groupWindow->curindex = index;

    RedrawAll();
}

void TGroupWindow::Paint(HDC hdc)
{
    if (IsIconic())
        return;
    if (!set.Map_Read)
        return;

    GdiFlush();

    int index = group_sel_index;
    if (index < 0)
        return;

    POINT TL;
    RECT rc;
    GetWindowRect(GroupList->hwnd, &rc);
    InflateRect(&rc,1,1);

    TL.x = rc.left;
    TL.y = rc.top;
    ScreenToClient(hwnd,&TL);

    COLORREF groupcolor = map_i[set.curmap]->groups->GetGroupColor(index);

    TL.x = rc.left;
    TL.y = rc.top;
    ScreenToClient(hwnd,&TL);

    RECT R;
    R.left = TL.x;
    R.top = TL.y;
    R.right = TL.x + (rc.right-rc.left);
    R.bottom = TL.y + (rc.bottom-rc.top);
    HBRUSH brush = CreateSolidBrush(groupcolor);
    FrameRect(hdc, &R, brush);
    InflateRect(&rc,1,1);
    TL.x = rc.left;
    TL.y = rc.top;
    ScreenToClient(hwnd,&TL);
    R.left = TL.x;
    R.top = TL.y;
    R.right = TL.x + (rc.right-rc.left);
    R.bottom = TL.y + (rc.bottom-rc.top);
    FrameRect(hdc, &R, brush);
    DeleteObject(brush);
}


void TGroupWindow::RedrawAll()
{
    if (!set.Map_Read)
        return;

    GdiFlush();

    int index = group_sel_index;

    if (index >= 0)
    {

        RECT rc;
        POINT TL;
        GetWindowRect(GroupList->hwnd,&rc);
        InflateRect(&rc,1,1);

        TL.x = rc.left;
        TL.y = rc.top;
        ScreenToClient(hwnd, &TL);

        COLORREF groupcolor = map_i[set.curmap]->groups->GetGroupColor(index);

        HDC hdc = GetDC(hwnd);
        HBRUSH brush = CreateSolidBrush(groupcolor);
        RECT R;
        R.left = TL.x;
        R.top = TL.y;
        R.right = TL.x + (rc.right-rc.left);
        R.bottom = TL.y + (rc.bottom-rc.top);
        FrameRect(hdc, &R, brush);
        InflateRect(&rc,1,1);
        TL.x = rc.left;
        TL.y = rc.top;
        ScreenToClient(hwnd, &TL);
        R.left = TL.x;
        R.top = TL.y;
        R.right = TL.x + (rc.right-rc.left);
        R.bottom = TL.y + (rc.bottom-rc.top);
        FrameRect(hdc, &R, brush);

        ReleaseDC(hwnd, hdc);
        DeleteObject(brush);
    }

    ValidateRect(hwnd, 0);

    set.redrawxy = 1;
    set.redrawedit = 1;
    Show_Frame("Updating...",true);
}

void TGroupWindow::RedrawReload()
{
    if (!set.Map_Read)
        return;

    groupWindow->curindex = 0;

    LoadData();
    groupWindow->GroupList->SetSelIndex(0);
    RedrawAll();
}


void TGroupWindow::Invalidate()
{
    InvalidateRect(hwnd,0,true);
}
