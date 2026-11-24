//command response and command enablers
#include "global.h"

bool c_noupdate = false;

//
// KEYBOARD
//


typedef struct vkey_s
{
    int vkid;
    char *name;
} vkey_t;
#define VKEY_START(name) vkey_t name[] = {
#define VKEY(id,string) {id,string},
#define VKEY_END {0,0} };


VKEY_START(vkeys)
VKEY(VK_LBUTTON, const_cast<char *> ("LButton"))
VKEY(VK_RBUTTON, const_cast<char *> ("RButton"))
VKEY(VK_CANCEL, const_cast<char *> ("Cancel"))
VKEY(VK_MBUTTON, const_cast<char *> ("MButton"))

VKEY(VK_XBUTTON1, const_cast<char *> ("XButton1"))
VKEY(VK_XBUTTON2, const_cast<char *> ("XButton2"))

VKEY(VK_BACK, const_cast<char *> ("Back"))
VKEY(VK_TAB, const_cast<char *> ("Tab"))

VKEY(VK_CLEAR, const_cast<char *> ("Clear"))
VKEY(VK_RETURN, const_cast<char *> ("Return"))

VKEY(VK_SHIFT, const_cast<char *> ("Shift"))
VKEY(VK_CONTROL, const_cast<char *> ("Control"))
VKEY(VK_MENU, const_cast<char *> ("Alt"))
VKEY(VK_PAUSE, const_cast<char *> ("Pause"))
VKEY(VK_CAPITAL, const_cast<char *> ("Capital"))

VKEY(VK_KANA, const_cast<char *> ("Kana"))
VKEY(VK_HANGEUL, const_cast<char *> ("Hangeul"))
VKEY(VK_HANGUL, const_cast<char *> ("Hangul"))
VKEY(VK_JUNJA, const_cast<char *> ("Junja"))
VKEY(VK_FINAL, const_cast<char *> ("Final"))
VKEY(VK_HANJA, const_cast<char *> ("Hanja"))
VKEY(VK_KANJI, const_cast<char *> ("Kanji"))

VKEY(VK_ESCAPE, const_cast<char *> ("Escape"))

VKEY(VK_CONVERT, const_cast<char *> ("Convert"))
VKEY(VK_NONCONVERT, const_cast<char *> ("Nonconvert"))
VKEY(VK_ACCEPT, const_cast<char *> ("Accept"))
VKEY(VK_MODECHANGE, const_cast<char *> ("ModeChange"))

VKEY(VK_SPACE, const_cast<char *> ("Space"))
VKEY(VK_PRIOR, const_cast<char *> ("PageUp"))
VKEY(VK_NEXT, const_cast<char *> ("PageDown"))
VKEY(VK_END, const_cast<char *> ("End"))
VKEY(VK_HOME, const_cast<char *> ("Home"))
VKEY(VK_LEFT, const_cast<char *> ("Left"))
VKEY(VK_UP, const_cast<char *> ("Up"))
VKEY(VK_RIGHT, const_cast<char *> ("Right"))
VKEY(VK_DOWN, const_cast<char *> ("Down"))
VKEY(VK_SELECT, const_cast<char *> ("Select"))
VKEY(VK_PRINT, const_cast<char *> ("Print"))
VKEY(VK_EXECUTE, const_cast<char *> ("Execute"))
VKEY(VK_SNAPSHOT, const_cast<char *> ("Snapshot"))
VKEY(VK_INSERT, const_cast<char *> ("Insert"))
VKEY(VK_DELETE, const_cast<char *> ("Delete"))
VKEY(VK_HELP, const_cast<char *> ("Help"))

/*
* VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
* 0x40 : unassigned
* VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
*/

VKEY(VK_LWIN, const_cast<char *> ("LWin"))
VKEY(VK_RWIN, const_cast<char *> ("RWin"))
VKEY(VK_APPS, const_cast<char *> ("Apps"))

VKEY(VK_SLEEP, const_cast<char *> ("Sleep"))

VKEY(VK_NUMPAD0, const_cast<char *> ("Numpad0"))
VKEY(VK_NUMPAD1, const_cast<char *> ("Numpad1"))
VKEY(VK_NUMPAD2, const_cast<char *> ("Numpad2"))
VKEY(VK_NUMPAD3, const_cast<char *> ("Numpad3"))
VKEY(VK_NUMPAD4, const_cast<char *> ("Numpad4"))
VKEY(VK_NUMPAD5, const_cast<char *> ("Numpad5"))
VKEY(VK_NUMPAD6, const_cast<char *> ("Numpad6"))
VKEY(VK_NUMPAD7, const_cast<char *> ("Numpad7"))
VKEY(VK_NUMPAD8, const_cast<char *> ("Numpad8"))
VKEY(VK_NUMPAD9, const_cast<char *> ("Numpad9"))
VKEY(VK_MULTIPLY, const_cast<char *> ("Multiply"))
VKEY(VK_ADD, const_cast<char *> ("Add"))
VKEY(VK_SEPARATOR, const_cast<char *> ("Separator"))
VKEY(VK_SUBTRACT, const_cast<char *> ("Subtract"))
VKEY(VK_DECIMAL, const_cast<char *> ("Decimal"))
VKEY(VK_DIVIDE, const_cast<char *> ("Divide"))
VKEY(VK_F1, const_cast<char *> ("F1"))
VKEY(VK_F2, const_cast<char *> ("F2"))
VKEY(VK_F3, const_cast<char *> ("F3"))
VKEY(VK_F4, const_cast<char *> ("F4"))
VKEY(VK_F5, const_cast<char *> ("F5"))
VKEY(VK_F6, const_cast<char *> ("F6"))
VKEY(VK_F7, const_cast<char *> ("F7"))
VKEY(VK_F8, const_cast<char *> ("F8"))
VKEY(VK_F9, const_cast<char *> ("F9"))
VKEY(VK_F10, const_cast<char *> ("F10"))
VKEY(VK_F11, const_cast<char *> ("F11"))
VKEY(VK_F12, const_cast<char *> ("F12"))
VKEY(VK_F13, const_cast<char *> ("F13"))
VKEY(VK_F14, const_cast<char *> ("F14"))
VKEY(VK_F15, const_cast<char *> ("F15"))
VKEY(VK_F16, const_cast<char *> ("F16"))
VKEY(VK_F17, const_cast<char *> ("F17"))
VKEY(VK_F18, const_cast<char *> ("F18"))
VKEY(VK_F19, const_cast<char *> ("F19"))
VKEY(VK_F20, const_cast<char *> ("F20"))
VKEY(VK_F21, const_cast<char *> ("F21"))
VKEY(VK_F22, const_cast<char *> ("F22"))
VKEY(VK_F23, const_cast<char *> ("F23"))
VKEY(VK_F24, const_cast<char *> ("F24"))

VKEY(VK_NUMLOCK, const_cast<char *> ("Numlock"))
VKEY(VK_SCROLL, const_cast<char *> ("ScrollLock"))

VKEY(VK_OEM_NEC_EQUAL, const_cast<char *> ("OEM_NEC_EQUAL"))

VKEY(VK_OEM_FJ_JISHO, const_cast<char *> ("OEM_FJ_JISHO"))
VKEY(VK_OEM_FJ_MASSHOU, const_cast<char *> ("OEM_FJ_MASSHOU"))
VKEY(VK_OEM_FJ_TOUROKU, const_cast<char *> ("OEM_FJ_TOUROKU"))
VKEY(VK_OEM_FJ_LOYA, const_cast<char *> ("OEM_FJ_LOYA"))
VKEY(VK_OEM_FJ_ROYA, const_cast<char *> ("OEM_FJ_ROYA"))

VKEY(VK_OEM_1, const_cast<char *> ("OEM_1(;)"))
VKEY(VK_OEM_PLUS, const_cast<char *> ("Plus"))
VKEY(VK_OEM_COMMA, const_cast<char *> ("Comma"))
VKEY(VK_OEM_MINUS, const_cast<char *> ("Minus"))
VKEY(VK_OEM_PERIOD, const_cast<char *> ("Period"))
VKEY(VK_OEM_2, const_cast<char *> ("OEM_2(/)"))
VKEY(VK_OEM_3, const_cast<char *> ("OEM_3(`)"))

VKEY(VK_LSHIFT, const_cast<char *> ("LShift"))
VKEY(VK_RSHIFT, const_cast<char *> ("RShift"))
VKEY(VK_LCONTROL, const_cast<char *> ("LControl"))
VKEY(VK_RCONTROL, const_cast<char *> ("RControl"))
VKEY(VK_LMENU, const_cast<char *> ("LMenu"))
VKEY(VK_RMENU, const_cast<char *> ("RMenu"))

VKEY(VK_BROWSER_BACK, const_cast<char *> ("BrowserBack"))
VKEY(VK_BROWSER_FORWARD, const_cast<char *> ("BrowserForward"))
VKEY(VK_BROWSER_REFRESH, const_cast<char *> ("BrowserRefresh"))
VKEY(VK_BROWSER_STOP, const_cast<char *> ("BrowserStop"))
VKEY(VK_BROWSER_SEARCH, const_cast<char *> ("BrowserSearch"))
VKEY(VK_BROWSER_FAVORITES, const_cast<char *> ("BrowserFavorites"))
VKEY(VK_BROWSER_HOME, const_cast<char *> ("BrowserHome"))

VKEY(VK_VOLUME_MUTE, const_cast<char *> ("VolMute"))
VKEY(VK_VOLUME_DOWN, const_cast<char *> ("VolDown"))
VKEY(VK_VOLUME_UP, const_cast<char *> ("VolUp"))
VKEY(VK_MEDIA_NEXT_TRACK, const_cast<char *> ("MediaNext"))
VKEY(VK_MEDIA_PREV_TRACK, const_cast<char *> ("MediaPrev"))
VKEY(VK_MEDIA_STOP, const_cast<char *> ("MediaStop"))
VKEY(VK_MEDIA_PLAY_PAUSE, const_cast<char *> ("MediaPlayPause"))
VKEY(VK_LAUNCH_MAIL, const_cast<char *> ("LaunchMail"))
VKEY(VK_LAUNCH_MEDIA_SELECT, const_cast<char *> ("LaunchMedia"))
VKEY(VK_LAUNCH_APP1, const_cast<char *> ("LaunchApp1"))
VKEY(VK_LAUNCH_APP2, const_cast<char *> ("LaunchApp2"))

VKEY(VK_OEM_4, const_cast<char *> ("OEM_4([)"))
VKEY(VK_OEM_5, const_cast<char *> ("OEM_5(\\)"))
VKEY(VK_OEM_6, const_cast<char *> ("OEM_6(])"))
VKEY(VK_OEM_7, const_cast<char *> ("OEM_7(')"))
VKEY(VK_OEM_8, const_cast<char *> ("OEM_8"))


VKEY(VK_OEM_AX, const_cast<char *> ("OEM_AX"))
VKEY(VK_OEM_102, const_cast<char *> ("OEM_102"))
VKEY(VK_ICO_HELP, const_cast<char *> ("ICO_HELP"))
VKEY(VK_ICO_00, const_cast<char *> ("ICO_00"))

VKEY(VK_PROCESSKEY, const_cast<char *> ("PROCESSKEY"))
VKEY(VK_ICO_CLEAR, const_cast<char *> ("ICO_CLEAR"))
VKEY(VK_PACKET, const_cast<char *> ("PACKET"))

VKEY(VK_OEM_RESET, const_cast<char *> ("OEM_RESET"))
VKEY(VK_OEM_JUMP, const_cast<char *> ("OEM_JUMP"))
VKEY(VK_OEM_PA1, const_cast<char *> ("OEM_PA1"))
VKEY(VK_OEM_PA2, const_cast<char *> ("OEM_PA2"))
VKEY(VK_OEM_PA3, const_cast<char *> ("OEM_PA3"))
VKEY(VK_OEM_WSCTRL, const_cast<char *> ("OEM_WSCTRL"))
VKEY(VK_OEM_CUSEL, const_cast<char *> ("OEM_CUSEL"))
VKEY(VK_OEM_ATTN, const_cast<char *> ("OEM_ATTN"))
VKEY(VK_OEM_FINISH, const_cast<char *> ("OEM_FINISH"))
VKEY(VK_OEM_COPY, const_cast<char *> ("OEM_COPY"))
VKEY(VK_OEM_AUTO, const_cast<char *> ("OEM_AUTO"))
VKEY(VK_OEM_ENLW, const_cast<char *> ("OEM_ENLW"))
VKEY(VK_OEM_BACKTAB, const_cast<char *> ("OEM_BACKTAB"))

VKEY(VK_ATTN, const_cast<char *> ("ATTN"))
VKEY(VK_CRSEL, const_cast<char *> ("CRSEL"))
VKEY(VK_EXSEL, const_cast<char *> ("EXSEL"))
VKEY(VK_EREOF, const_cast<char *> ("EREOF"))
VKEY(VK_PLAY, const_cast<char *> ("PLAY"))
VKEY(VK_ZOOM, const_cast<char *> ("ZOOM"))
VKEY(VK_NONAME, const_cast<char *> ("NONAME"))
VKEY(VK_PA1, const_cast<char *> ("PA1"))
VKEY(VK_OEM_CLEAR, const_cast<char *> ("OEM_CLEAR"))
VKEY_END;

//TODO: do these need to be optimized? or is linear search ok? (where are they used?)
// return string from id
char * GetVKeyName(int id)
{
    for(vkey_t *v=vkeys; v->vkid; v++)		//run while ID is nonzero
        if(v->vkid == id)					//find id
            return v->name;					//return name
    return 0;								//not found
}

// find id from string
int GetVKey(char *vkey)
{
    for(vkey_t *v=vkeys; v->vkid; v++)		//run while ID is nonzero
        if(!strcmpi(v->name,vkey))			//match string
            return v->vkid;					//return id
    return 0;								//not found
}



////////////////////////////////////////////////////////////////////////////////
// new dynamic command system
////////////////////////////////////////////////////////////////////////////////

static ccmd_t *cmd_name_hash[CCMD_HASH_SIZE];	//name lookup
static ccmd_t *cmd_id_hash[CCMD_HASH_SIZE];		//id lookup
ccmd_t *cmd_head;					//all commands
static ccmd_t *cmd_ce_head;			//commands with enablers
ccmd_group_t *cmd_groups;			//groups
ccmd_group_t *cmd_groups_current;	//current group


//returns a new cmd id value
int Ccmd_NextId()
{
    static int next_id = WM_APP;
    return next_id++;
}
//lookup by id
int Ccmd_HashKey(int id)
{
    return id % CCMD_HASH_SIZE;
}
//lookup by name
int Ccmd_HashKey(char *name)
{
    unsigned char c;
    uintptr_t v = 0;
    for ( ; (c = *name++) != 0 ; v += c &~ 32);   // make it case insensitive
    return v % CCMD_HASH_SIZE;
}
ccmd_t *Ccmd_FindCmd(char *name)
{
    int key = Ccmd_HashKey(name);
    ccmd_t *c = cmd_name_hash[key];
    for( ; c; c=c->hash_name_next)
        if(!stricmp(c->name,name))
            return c;
    return 0;
}
ccmd_t *Ccmd_FindCmd(int id)
{
    int key = Ccmd_HashKey(id);
    ccmd_t *c = cmd_id_hash[key];
    for( ; c; c=c->hash_id_next)
        if(c->id == id)
            return c;
    return 0;
}

int Ccmd_GetId(char *name)
{
    ccmd_t *c = Ccmd_FindCmd(name);
    if(c)
        return c->id;
    return -1;
}
char *Ccmd_GetName(int id)
{
    ccmd_t *c = Ccmd_FindCmd(id);
    if(c)
        return c->name;
    return 0;
}
char *Ccmd_GetInfoText(int id)
{
    ccmd_t *c = Ccmd_FindCmd(id);
    if(c)
        return c->info_text;
    return 0;
}
//group add
ccmd_group_t * Ccmd_GroupFind(char *name)
{
    ccmd_group_t *g = cmd_groups;
    for( ; g; g=g->next )
        if(!stricmp(g->name, name))
            return g;
    return 0;
}

ccmd_group_t * Ccmd_SetCurrentGroup(char *name)
{
    ccmd_group_t *g;
    if(g = Ccmd_GroupFind(name))
    {
        cmd_groups_current = g;
        return g;
    }
    g = new ccmd_group_t;
    strncpy(g->name,name,sizeof(g->name));
    g->cmd_next = 0;

    ADD_TO_LIST_SORTED(ccmd_group_t, g, cmd_groups, next);

    // and set current group
    cmd_groups_current = g;
    return g;
}
//get list of commands for a tab completion
int Ccmd_Complete(char *prefix, char *out, int outlen, list<char*> *matches)
{
    if(!out)
        return 0;
    if(!prefix)
        return 0;
    *out = 0;

    int plen = strlen(prefix);
    if(plen > outlen)
        return 0;

    int count = 0;
    ccmd_t *c;
    for(c = cmd_head; c; c = c->next)
    {
        if(!_strnicmp(prefix, c->name, plen))
        {
            if(!*out)
            {
                strncpy(out, c->name, outlen);
            }
            else
            {
                for(int i = 0; out[i]; i++)
                {
                    if(out[i] != c->name[i])
                    {
                        out[i] = 0;
                        break;
                    }
                }
            }

            if(matches)
            {
                matches->addsorted(c->name);
            }
            ++count;
        }
    }
    return count;
}

void Ccmd_Unload()
{
    //commands
    ccmd_t *c = cmd_head, *ctmp;
    for( ; c; c = ctmp)
    {
        //clear fields
        delete[] c->info_text;
        delete[] c->menu_text;
        delete[] c->user_func;
        //unlink
        ctmp = c->next;
        delete c;
    }
    //groups
    ccmd_group_t *g = cmd_groups, *gtmp;
    for(/**/ ; g; g = gtmp)
    {
        gtmp = g->next;
        delete g;
    }
}

bool Ccmd_Register(char *name, CM_FUNC cm_func, CE_FUNC ce_func)
{
    if(Ccmd_FindCmd(name))
        return false;	//already exists
    ccmd_t *c = new ccmd_t;
    c->cm_func = cm_func;
    c->ce_func = ce_func;
    strncpy(c->name,name,sizeof(c->name));
    c->id = Ccmd_NextId();
    c->image = -1;			//images are zero-based
    c->info_text = 0;
    c->menu_text = 0;
    c->user_func = 0;

    //link lists
    ADD_TO_LIST_SORTED(ccmd_t, c, cmd_head, next);
    if(c->ce_func)  		//enabler list
    {
        c->ce_next = cmd_ce_head;
        cmd_ce_head = c;
    }
    //hash id
    int key = Ccmd_HashKey(c->id);
    c->hash_id_next = cmd_id_hash[key];
    cmd_id_hash[key] = c;
    //hash name
    key = Ccmd_HashKey(name);
    c->hash_name_next = cmd_name_hash[key];
    cmd_name_hash[key] = c;
    //group
    c->group = cmd_groups_current;

    ADD_TO_LIST_SORTED(ccmd_t, c, cmd_groups_current->cmd_next, group_next);

    return true;
}

bool Ccmd_Register(char *name, char *scriptcode, CE_FUNC ce_func)
{
    if(Ccmd_FindCmd(name))
        return false;	//already exists
    if(!scriptcode)
        return false;
    ccmd_t *c = new ccmd_t;
    c->cm_func = 0;
    c->ce_func = ce_func;
    strncpy(c->name,name,sizeof(c->name));
    c->id = Ccmd_NextId();
    c->image = -1;			//images are zero-based
    c->info_text = 0;
    c->menu_text = 0;
    c->user_func = new char[strlen(scriptcode)+1];
    strcpy(c->user_func, scriptcode);

    //link lists
    ADD_TO_LIST_SORTED(ccmd_t, c, cmd_head, next);
    if(c->ce_func)  		//enabler list
    {
        c->ce_next = cmd_ce_head;
        cmd_ce_head = c;
    }
    //hash id
    int key = Ccmd_HashKey(c->id);
    c->hash_id_next = cmd_id_hash[key];
    cmd_id_hash[key] = c;
    //hash name
    key = Ccmd_HashKey(name);
    c->hash_name_next = cmd_name_hash[key];
    cmd_name_hash[key] = c;
    //group
    c->group = cmd_groups_current;

    ADD_TO_LIST_SORTED(ccmd_t, c, cmd_groups_current->cmd_next, group_next);

    return true;
}

// read cmddef config file. each line is: [cmd name] [menu name] [info]
// INFO: for each line in this file, this function matches [cmd name] to an existsing
// command, sets [menu text] and [info] if not already specified, and loads an
// associated button bitmap if available.
bool Ccmd_LoadCommandDef(char *file)
{
    char *dat = (char*) file_get_contents(file);
    if(!dat)
        return false;
    Tokenizer config(dat);
    while(true)
    {
        char name[32], menu[48], info[128];
        if(!config.next(true))
            break;
        strncpy(name,config.token,sizeof(name));
        if(!config.next(true))
            break;
        strncpy(menu,config.token,sizeof(menu));
        if(!config.next(true))
            break;
        strncpy(info,config.token,sizeof(info));

        ccmd_t *c = Ccmd_FindCmd(name);
        if(c)
        {
            if(!c->menu_text && *menu)
            {
                c->menu_text = new char[strlen(menu)+1];
                strcpy(c->menu_text, menu);
            }
            if(!c->info_text && *info)
            {
                c->info_text = new char[strlen(info)+1];
                strcpy(c->info_text, info);
            }
            if(c->image == -1)
            {
                c->image = imagelist.Add(Path(const_cast<char *> ("settings/gfx/toolbar/%s.bmp"), name), c->id);
            }
        }
        else
        {
            syserror(const_cast<char *> ("Warning: unknown command \"%s\" in file %s (Line %d)"),
                     name, file, config.line);
        }
        config.skipline();
    }

    delete[] dat;
    return true;
}

//call each enabler function
void Ccmd_UpdateEnablers()
{
    for(ccmd_t *c = cmd_ce_head; c; c = c->ce_next)
        if(c->ce_func)
            c->ce_func(c->id);
}

//call command function from ID
void Ccmd_Execute(int id)
{
    ccmd_t *c = Ccmd_FindCmd(id);
    if(c)
    {
        if(c->cm_func)
            c->cm_func ();
    }
}
//call from by name
void Ccmd_Execute(char *name)
{
    ccmd_t *c = Ccmd_FindCmd(name);
    if(c)
    {
        if(c->cm_func)
            c->cm_func ();
        /*
        		else if(c->user_func)
          			SQ_Exec(c->user_func);
        */
    }
}

// fills a combo with all commands
void Ccmd_FillCombo(WCombo *combo)
{
    for( ccmd_group_t *g = cmd_groups; g; g = g->next )
    {
        char ghdr[64];
        snprintf(ghdr,sizeof(ghdr), "=======  %s  ==========================", g->name);
        combo->InsertString(ghdr);
        for( ccmd_t *c = g->cmd_next; c; c = c->group_next )
        {
            combo->InsertString(c->name);
        }
    }
}



DockWindow *hacktest;
void CmHack()
{

    hacktest = new DockWindow(client->hwnd, "HACKING TEST");
    hacktest->Create(true);

    hacktest->Dock(xyWindow[0]);
    hacktest->Dock(xyWindow[1]);
    hacktest->Dock(xyWindow[2]);
    hacktest->Dock(editWindow);
    hacktest->PositionWindows();
}
//load built in commands
void Ccmd_Initialize()
{
    //setup mem
    memset(cmd_name_hash,0,sizeof(cmd_name_hash));
    memset(cmd_id_hash,0,sizeof(cmd_id_hash));
    cmd_head = 0;
    cmd_ce_head = 0;
    cmd_groups = 0;
    cmd_groups_current = 0;


    Ccmd_SetCurrentGroup(const_cast<char *> ("HACKING"));
    Ccmd_Register(const_cast<char *> ("CM_HACK"),		CmHack,	0);


    Ccmd_SetCurrentGroup(const_cast<char *> ("Group Window"));
    Ccmd_Register(const_cast<char *> ("CM_GROUP_NEW_GROUP"),		CmGroupNewGroup,	0);
    Ccmd_Register(const_cast<char *> ("CM_GROUP_NEW_SPACER"),	CmGroupNewSpacer,	0);
    Ccmd_Register(const_cast<char *> ("CM_GROUP_COLOR"),			CmGroupColor,		CeGroupNeedSel);
    Ccmd_Register(const_cast<char *> ("CM_GROUP_DELETE"),		CmGroupDelete,		CeGroupNeedSel);
    Ccmd_Register(const_cast<char *> ("CM_GROUP_ADDSEL"),		CmGroupAddSel,		CeGroupNeedSel);
    Ccmd_Register(const_cast<char *> ("CM_GROUP_SELECTALL"),		CmGroupSelectAll,	CeGroupNeedSel);
    Ccmd_Register(const_cast<char *> ("CM_GROUP_GOTO"),			CmGroupGoto,		CeGroupNeedSel);
    Ccmd_Register(const_cast<char *> ("CM_GROUP_VISIBLE"),		CmGroupVisible,		CeGroupNeedSel);
    Ccmd_Register(const_cast<char *> ("CM_GROUP_RENAME"),		CmGroupRename,		CeGroupNeedSel);
    Ccmd_Register(const_cast<char *> ("CM_GROUP_UP"),			CmGroupUp,			CeGroupMoveUp);
    Ccmd_Register(const_cast<char *> ("CM_GROUP_DOWN"),			CmGroupDown,		CeGroupMoveDown);
    Ccmd_Register(const_cast<char *> ("CM_GROUP_SINGLE"),		CmGroupSingle,		CeGroupNeedSel);

    Ccmd_SetCurrentGroup(const_cast<char *> ("Navigation"));
    Ccmd_Register(const_cast<char *> ("CM_MOVEUP"),		CmMoveUp,	0);
    Ccmd_Register(const_cast<char *> ("CM_MOVEDOWN"),	CmMoveDown,	0);
    Ccmd_Register(const_cast<char *> ("CM_FORWARD"),		CmForward,	0);
    Ccmd_Register(const_cast<char *> ("CM_BACK"),		CmBack,	0);
    Ccmd_Register(const_cast<char *> ("CM_RIGHT"),		CmRight,	0);
    Ccmd_Register(const_cast<char *> ("CM_LEFT"),		CmLeft,	0);
    Ccmd_Register(const_cast<char *> ("CM_MOVELEFT"),	CmMoveLeft,	0);
    Ccmd_Register(const_cast<char *> ("CM_MOVERIGHT"),	CmMoveRight,	0);
    Ccmd_Register(const_cast<char *> ("CM_180"),				Cm180, CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_RESET_ANGLES"),	CmResetAngles,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_PITCH_DOWN"),			CmPitchDown,	0);
    Ccmd_Register(const_cast<char *> ("CM_PITCH_UP"),			CmPitchUp,	0);
    Ccmd_Register(const_cast<char *> ("CM_NEXT_CAMERA"),		CmNextCamera,	0);
    Ccmd_Register(const_cast<char *> ("CM_PREV_CAMERA"),		CmPrevCamera,	0);

    Ccmd_SetCurrentGroup(const_cast<char *> ("Physics"));
    Ccmd_Register(const_cast<char *> ("CM_RUNMODE"),		CmRunMode,	CeRunMode);

    Ccmd_SetCurrentGroup(const_cast<char *> ("Editing"));
    Ccmd_Register(const_cast<char *> ("CM_MINUS"),		CmMinus,	0);
    Ccmd_Register(const_cast<char *> ("CM_PLUS"),		CmPlus,	0);
    Ccmd_Register(const_cast<char *> ("CM_EDITCOPY"),		CmEditCopy,	CeNeedSelection);
    Ccmd_Register(const_cast<char *> ("CM_EDITPASTE"),		CmEditPaste,	CePaste);
    Ccmd_Register(const_cast<char *> ("CM_EDITCLONE"),		CmEditClone,	CeNeedSelection);
    Ccmd_Register(const_cast<char *> ("CM_ROT1"),		CmRot1,	CeRot1);
    Ccmd_Register(const_cast<char *> ("CM_ROT5"),		CmRot5,	CeRot5);
    Ccmd_Register(const_cast<char *> ("CM_ROT10"),		CmRot10,	CeRot10);
    Ccmd_Register(const_cast<char *> ("CM_ROT15"),		CmRot15,	CeRot15);
    Ccmd_Register(const_cast<char *> ("CM_ROT30"),		CmRot30,	CeRot30);
    Ccmd_Register(const_cast<char *> ("CM_ROT45"),		CmRot45,	CeRot45);
    Ccmd_Register(const_cast<char *> ("CM_HIT0"),		CmHit0,	0);
    Ccmd_Register(const_cast<char *> ("CM_HIT1"),		CmHit1,	0);
    Ccmd_Register(const_cast<char *> ("CM_HIT2"),		CmHit2,	0);
    Ccmd_Register(const_cast<char *> ("CM_HIT3"),		CmHit3,	0);
    Ccmd_Register(const_cast<char *> ("CM_HIT4"),		CmHit4,	0);
    Ccmd_Register(const_cast<char *> ("CM_HIT5"),		CmHit5,	0);
    Ccmd_Register(const_cast<char *> ("CM_HIT6"),		CmHit6,	0);
    Ccmd_Register(const_cast<char *> ("CM_HIT7"),		CmHit7,	0);
    Ccmd_Register(const_cast<char *> ("CM_HIT8"),		CmHit8,	0);
    Ccmd_Register(const_cast<char *> ("CM_HIT9"),		CmHit9,	0);
    Ccmd_Register(const_cast<char *> ("CM_LIFTHEIGHTS"),		CmLiftHeights,	CeNeedSelection);
    Ccmd_Register(const_cast<char *> ("CM_SETHEIGHTS"),		CmSetHeights,	0);
    Ccmd_Register(const_cast<char *> ("CM_SELECT_VALUE"),		CmSelectValue,	0);
    Ccmd_Register(const_cast<char *> ("CM_MAKE"),		CmMakeClicked,	0);
    Ccmd_Register(const_cast<char *> ("CM_DETAIL"),		CmAddDetail,	CeNeedSelection);
    Ccmd_Register(const_cast<char *> ("CM_NODETAIL"),		CmRemoveDetail,	CeNeedSelection);
    Ccmd_Register(const_cast<char *> ("CM_COPY_MOVE"),		CmCopyMove,	CeNeedSelection);
    Ccmd_Register(const_cast<char *> ("CM_JUMP_SEL_XY"),		CmJumpSelXY,	CeNeedSelection);
    Ccmd_Register(const_cast<char *> ("CM_CENTER"),		CmCenter,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_REGIONEXPORT"),		CmRegionExport,	CeRegionExport);
    Ccmd_Register(const_cast<char *> ("CM_NOREGION"),		CmNoRegion,	CeNoRegion);
    Ccmd_Register(const_cast<char *> ("CM_OMIT_UNSELECTED"),		CmOmitUnselected,	CeOmitUnselected);
    Ccmd_Register(const_cast<char *> ("CM_OMIT_SKY"),		CmOmitSky,	CeOmitSky);
    Ccmd_Register(const_cast<char *> ("CM_OMIT_TARGET"),		CmOmitTarget,	CeOmitTarget);
    Ccmd_Register(const_cast<char *> ("CM_OMIT_LIGHTS"),		CmOmitLights,	CeOmitLights);
    Ccmd_Register(const_cast<char *> ("CM_OMIT_WATER"),		CmOmitWater,	CeOmitWater);
    Ccmd_Register(const_cast<char *> ("CM_OMIT_CLIP"),		CmOmitClip,	CeOmitClip);
    Ccmd_Register(const_cast<char *> ("CM_OMIT_WORLD"),		CmOmitWorld,	CeOmitWorld);
    Ccmd_Register(const_cast<char *> ("CM_OMIT_ENTITIES"),		CmOmitEntities,	CeOmitEntities);
    Ccmd_Register(const_cast<char *> ("CM_OMIT_DETAIL"),		CmOmitDetail,	CeOmitDetail);
    Ccmd_Register(const_cast<char *> ("CM_OMIT_HINT"),		CmOmitHint,	CeOmitHint);
    Ccmd_Register(const_cast<char *> ("CM_FLYMODE"),		CmToggleFlymode,	CeToggleFlymode);
    Ccmd_Register(const_cast<char *> ("CM_LOCK"),		CmLock,	CeNeedSelection);
    Ccmd_Register(const_cast<char *> ("CM_UNLOCK"),		CmUnLock,	CeNeedSelection);
    Ccmd_Register(const_cast<char *> ("CM_JUMP"),		CmJump,	0);
    Ccmd_Register(const_cast<char *> ("CM_JUMPXY"),		CmJumpXY,	0);
    Ccmd_Register(const_cast<char *> ("CM_JUMP3D"),		CmJump3D,	0);
    Ccmd_Register(const_cast<char *> ("CM_MERGE_IN"),		CmMergeIn,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_MERGE_OUT"),		CmMergeOut,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_SNAP_TO_GRID"),		CmSnap,	CeNeedSelection);
    Ccmd_Register(const_cast<char *> ("CM_SNAP_PLANES"),		CmSnapPlanes,	CeNeedSelection);
    Ccmd_Register(const_cast<char *> ("CM_SNAP_BACK"),		CmSnapBack,	CeSnapBack);
    Ccmd_Register(const_cast<char *> ("CM_FLIP_X"),		CmFlipX,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_FLIP_Y"),		CmFlipY,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_FLIP_Z"),		CmFlipZ,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_ROTATE_X"),		CmRotateX,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_ROTATE_Y"),		CmRotateY,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_ROTATE_Z"),		CmRotateZ,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_LOOK_AT_SEL"),		CmLookAtSel,	CeLookAtSel);
    Ccmd_Register(const_cast<char *> ("CM_FLOOR_UP"),		CmFloorUp,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_FLOOR_DOWN"),		CmFloorDown,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_TALL_BRUSH"),		CmTallBrush,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_SHORT_BRUSH"),		CmShortBrush,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_HEIGHT_BRUSH"),		CmHeightBrush,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_EDIT_SCALE"),		CmEditScale,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_SEL_ALL_HIT"),		CmSelAllHit,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_XUP"),		CmXUp,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_YUP"),		CmYUp,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_ZUP"),		CmZUp,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_XDOWN"),		CmXDown,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_YDOWN"),		CmYDown,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_ZDOWN"),		CmZDown,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_SELECT_ALL"),		CmSelectAll,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_DESELECT_ALL"),		CmDeselectAll,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_SET_BRUSH_DEPTH"),		CmSetBrushDepth,	0);
    Ccmd_Register(const_cast<char *> ("CM_SET_SCALE"),		CmSetScale,	0);
    Ccmd_Register(const_cast<char *> ("CM_LEAK_SEARCH"),		CmLeakTest,	0);
    Ccmd_Register(const_cast<char *> ("CM_MERGE_BRUSHES"),		CmMergeBrushes,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_SHOW_LEAKS"),		CmShowLeaks,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_STOP_LEAKS"),		CmStopLeaks,	CeNeedLeaks);
    Ccmd_Register(const_cast<char *> ("CM_SHOW_PORTALS"),		CmShowPortals,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_STOP_PORTALS"),		CmStopPortals,	CeNeedPortals);
    Ccmd_Register(const_cast<char *> ("CM_SELECT_BY_TEXTURE"),		CmSelectByTexture,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_HIT_UP"),		CmHitUp,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_HIT_DOWN"),		CmHitDown,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_REPLACE"),		CmReplace,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_NEXT_FACE"),		CmNextFace,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_PREVIOUS_FACE"),		CmPrevFace,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_TOGGLE_HITS"),		CmToggleHits,	CeToggleHits);
    Ccmd_Register(const_cast<char *> ("CM_SELECT_TOTAL"),		CmSelectTotal,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_SELECT_PARTIAL"),		CmSelectPartial,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_ROTATE_BRUSH"),		CmRotateBrush,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_EDITMAKE_STAIRS"),		CmEditMakeStairs,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_MAKE_ENTITY"),		CmMakeEntity,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_MAKE_ROOM"),		CmMakeRoom,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_MAKE_BRUSH"),		CmMakeBrush,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_EXTRUDE"),		CmExtrude,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_EXTRUDE_FACE"),		CmExtrudeFace,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_MAKE_ARCH"),		CmMakeArch,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_MAKE_WEDGE"),		CmMakeWedge,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_MAKE_SPHERE"),		CmMakeSphere,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_MAKE_CYLINDER"),		CmMakeCylinder,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_MAKE_N_SIDED"),		CmMakeNSided,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_MAKE_LIGHTS"),		CmMakeLights,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_MAKE_PYRAMID"),		CmMakePyramid,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_SUBTRACT"),		CmSubtract,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_DELETE"),		CmDelete,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_GRID_UP"),		GridUp,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_GRID_DOWN"),		GridDown,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_GRID1"),		Grid1,	CeGrid1);
    Ccmd_Register(const_cast<char *> ("CM_GRID2"),		Grid2,	CeGrid2);
    Ccmd_Register(const_cast<char *> ("CM_GRID4"),		Grid4,	CeGrid4);
    Ccmd_Register(const_cast<char *> ("CM_GRID8"),		Grid8,	CeGrid8);
    Ccmd_Register(const_cast<char *> ("CM_GRID16"),		Grid16,	CeGrid16);
    Ccmd_Register(const_cast<char *> ("CM_GRID32"),		Grid32,	CeGrid32);
    Ccmd_Register(const_cast<char *> ("CM_GRID64"),		Grid64,	CeGrid64);
    Ccmd_Register(const_cast<char *> ("CM_GRID128"),		Grid128,	CeGrid128);
    Ccmd_Register(const_cast<char *> ("CM_GRID256"),		Grid256,	CeGrid256);
    Ccmd_Register(const_cast<char *> ("CM_SCALE_UP"),		CmScaleUp,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_SCALE_DOWN"),		CmScaleDown,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_SCALE1_64"),		Scale1_64,	CeScale1_64);
    Ccmd_Register(const_cast<char *> ("CM_SCALE1_32"),		Scale1_32,	CeScale1_32);
    Ccmd_Register(const_cast<char *> ("CM_SCALE1_16"),		Scale1_16,	CeScale1_16);
    Ccmd_Register(const_cast<char *> ("CM_SCALE1_8"),		Scale1_8,	CeScale1_8);
    Ccmd_Register(const_cast<char *> ("CM_SCALE1_4"),		Scale1_4,	CeScale1_4);
    Ccmd_Register(const_cast<char *> ("CM_SCALE1_2"),		Scale1_2,	CeScale1_2);
    Ccmd_Register(const_cast<char *> ("CM_SCALE3_4"),		Scale3_4,	CeScale3_4);
    Ccmd_Register(const_cast<char *> ("CM_SCALE1"),		Scale1,	CeScale1);
    Ccmd_Register(const_cast<char *> ("CM_SCALE2"),		Scale2,	CeScale2);
    Ccmd_Register(const_cast<char *> ("CM_SCALE4"),		Scale4,	CeScale4);
    Ccmd_Register(const_cast<char *> ("CM_SCALE8"),		Scale8,	CeScale8);
    Ccmd_Register(const_cast<char *> ("CM_SCALE16"),		Scale16,	CeScale16);
    Ccmd_Register(const_cast<char *> ("CM_GROUP_MODE"),		CmGroupMode,	CeGroupMode);
    Ccmd_Register(const_cast<char *> ("CM_REGIONMODE"),		CmRegionMode,	CeRegionMode);
    Ccmd_Register(const_cast<char *> ("CM_REGION"),		CmRegion,	CeNeedSelection);
    Ccmd_Register(const_cast<char *> ("CM_REGIONPARTIAL"),		CmRegionPartial,	CeNeedSelection);
    Ccmd_Register(const_cast<char *> ("CM_FINDBRUSH"),		CmFindBrush,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_MIXED"),		CmMixed,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_MODE_POINT"),		CmModePoint,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_MODE_EDGE"),		CmModeEdge,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_MODE_POLYGON"),		CmModePolygon,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_MODE_GROUP"),		CmModeGroup,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_KP_2"),		CmKP2,	0);
    Ccmd_Register(const_cast<char *> ("CM_KP_4"),		CmKP4,	0);
    Ccmd_Register(const_cast<char *> ("CM_KP_6"),		CmKP6,	0);
    Ccmd_Register(const_cast<char *> ("CM_KP_8"),		CmKP8,	0);

    Ccmd_SetCurrentGroup(const_cast<char *> ("Texture"));
    Ccmd_Register(const_cast<char *> ("CM_TLOADWAD"),		CmTLoadWad,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_TLOADWAL"),		CmTLoadWal,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_TLOADDIR"),		CmTLoadDir,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_TLOADUSED"),		CmTLoadUsed,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_TUNLOAD"),		CmTUnload,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_TLOADPAK"),		CmTLoadPak,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_TLOADPAKQ2"),		CmTLoadPakQ2,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_FACE_TEXTURE"),		CmFaceTexture,	0);
    Ccmd_Register(const_cast<char *> ("CM_BRUSH_TEXTURE"),		CmBrushTexture,	0);
    Ccmd_Register(const_cast<char *> ("CM_TEX_SEARCH"),		CmTexSearch,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_SHOW_PALETTE"),		CmShowPalette,	0);

    Ccmd_SetCurrentGroup(const_cast<char *> ("Display"));
    Ccmd_Register(const_cast<char *> ("CM_BAR1"),		CmBar1,	CeBar1);
    Ccmd_Register(const_cast<char *> ("CM_BAR2"),		CmBar2,	CeBar2);
    Ccmd_Register(const_cast<char *> ("CM_SET_GRID"),		CmToggleGridStyle,	CeToggleGridStyle);
    Ccmd_Register(const_cast<char *> ("CM_GRID3D"),		CmGrid3D,	CeGrid3D);
    Ccmd_Register(const_cast<char *> ("CM_AXES3D"),		CmAxes3D,	CeAxes3D);
    Ccmd_Register(const_cast<char *> ("CM_R1"),		R1,	0);
    Ccmd_Register(const_cast<char *> ("CM_R2"),		R2,	0);
    Ccmd_Register(const_cast<char *> ("CM_R3"),		R3,	0);
    Ccmd_Register(const_cast<char *> ("CM_R4"),		R4,	0);
    Ccmd_Register(const_cast<char *> ("CM_R5"),		R5,	0);
    Ccmd_Register(const_cast<char *> ("CM_R6"),		R6,	0);
    Ccmd_Register(const_cast<char *> ("CM_R7"),		R7,	0);
    Ccmd_Register(const_cast<char *> ("CM_R8"),		R8,	0);
    Ccmd_Register(const_cast<char *> ("CM_R9"),		R9,	0);
    Ccmd_Register(const_cast<char *> ("CM_GL_SHADING"),		CmGlShading,	CeGlShading);
    Ccmd_Register(const_cast<char *> ("CM_TOGGLE_MODE"),		CmToggleMode,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_MODELS"),		CmModels,	CeModels);
    Ccmd_Register(const_cast<char *> ("CM_SOLID_MODELS"),		CmSolidModels,	CeSolidModels);
    Ccmd_Register(const_cast<char *> ("CM_TRANS0"),		CmTrans0,	CeTrans0);
    Ccmd_Register(const_cast<char *> ("CM_TRANS1"),		CmTrans1,	CeTrans1);
    Ccmd_Register(const_cast<char *> ("CM_TRANS2"),		CmTrans2,	CeTrans2);
    Ccmd_Register(const_cast<char *> ("CM_TRANSGL"),		CmTransGL,	CeTransGL);
    Ccmd_Register(const_cast<char *> ("CM_CLIP_IN"),		CmClipIn,	0);
    Ccmd_Register(const_cast<char *> ("CM_CLIP_OUT"),	CmClipOut,	0);
    Ccmd_Register(const_cast<char *> ("CM_TOGGLE_KNOBS"),		CmToggleKnobs,	CeToggleKnobs);
    Ccmd_Register(const_cast<char *> ("CM_OUTLINE_USE_GROUP"),	CmOutlineUseGroup,	CeOutlineUseGroup);
    Ccmd_Register(const_cast<char *> ("CM_OUTLINE_USE_FLAT"),	CmOutlineUseFlat,	CeOutlineUseFlat);
    Ccmd_Register(const_cast<char *> ("CM_MODEWIRE"),		CmModeWire,	CeModeWire);
    Ccmd_Register(const_cast<char *> ("CM_MODEFLAT"),		CmModeFlat,	CeModeFlat);
    Ccmd_Register(const_cast<char *> ("CM_MODETEXTURE"),		CmModeTexture,	CeModeTexture);
    Ccmd_Register(const_cast<char *> ("CM_DRAW_CONNECTIONS"),	CmToggleConnections,	CeToggleConnections);
    Ccmd_Register(const_cast<char *> ("CM_TOGGLE_OUTLINE"),		CmToggleOutline,	CeToggleOutline);
    Ccmd_Register(const_cast<char *> ("CM_GRID_MODE"),		CmGridMode,	CeGridMode);
    Ccmd_Register(const_cast<char *> ("CM_TOGGLE_NAMES"),		CmToggleNames,	CeToggleNames);
    Ccmd_Register(const_cast<char *> ("CM_TOGGLE_COORDINATES"),	CmToggleCoordinates,	CeToggleCoordinates);
    Ccmd_Register(const_cast<char *> ("CM_MAG_DOWN"),		CmMagDown,	0);
    Ccmd_Register(const_cast<char *> ("CM_MAG_UP"),			CmMagUp,	0);
    Ccmd_Register(const_cast<char *> ("CM_PPD_DOWN"),		CmPPDDown,	0);
    Ccmd_Register(const_cast<char *> ("CM_PPD_UP"),			CmPPDUp,	0);
    Ccmd_Register(const_cast<char *> ("CM_STEP_DOWN"),		CmStepDown,	0);
    Ccmd_Register(const_cast<char *> ("CM_STEP_UP"),			CmStepUp,	0);

    Ccmd_SetCurrentGroup(const_cast<char *> ("Map"));
    Ccmd_Register(const_cast<char *> ("CM_GROUP"),			CmGroup,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_QBSP"),			CmExportQBSP,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTFULL"),		CmExportFull,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTFAST"),		CmExportFast,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTNO_VIS"),	CmExportNoVis,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTRELIGHT"),	CmExportReLight,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTLEAK_TEST"),	CmExportLeakTest,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTSELECTED"),	CmExportSelected,	CeExportSelected);
    Ccmd_Register(const_cast<char *> ("CM_REVERT"),			CmRevert,	CeRevert);
    Ccmd_Register(const_cast<char *> ("CM_MAPINFO"),			CmMapInfo,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("SelectMap1"),			SelectMap1,		0);
    Ccmd_Register(const_cast<char *> ("SelectMap2"),			SelectMap2,		0);
    Ccmd_Register(const_cast<char *> ("SelectMap3"),			SelectMap3,		0);
    Ccmd_Register(const_cast<char *> ("SelectMap4"),			SelectMap4,		0);
    Ccmd_Register(const_cast<char *> ("SelectMap5"),			SelectMap5,		0);
    Ccmd_Register(const_cast<char *> ("SelectMap6"),			SelectMap6,		0);
    Ccmd_Register(const_cast<char *> ("SelectMap7"),			SelectMap7,		0);
    Ccmd_Register(const_cast<char *> ("SelectMap8"),			SelectMap8,		0);
    Ccmd_Register(const_cast<char *> ("SelectMap9"),			SelectMap9,		0);
    Ccmd_Register(const_cast<char *> ("SelectMap10"),		SelectMap10,	0);
    Ccmd_Register(const_cast<char *> ("CM_LOAD_BRUSHES"),	CmLoadBrushes,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_NEXT_MAP"),		CmNextMap,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_PREVIOUS_MAP"),	CmPreviousMap,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTER0"),		Exporter0,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTER1"),		Exporter1,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTER2"),		Exporter2,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTER3"),		Exporter3,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTER4"),		Exporter4,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTER5"),		Exporter5,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTER6"),		Exporter6,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTER7"),		Exporter7,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTER8"),		Exporter8,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTER9"),		Exporter9,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTER10"),		Exporter10,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTER11"),		Exporter11,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTER12"),		Exporter12,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTER13"),		Exporter13,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTER14"),		Exporter14,	0);
    Ccmd_Register(const_cast<char *> ("CM_EXPORTER15"),		Exporter15,	0);
    Ccmd_Register(const_cast<char *> ("CM_SWITCH_MAP_VIEW"),		CmSwitchMapView,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_MERGE_CLIPBOARD"),		CmMergeClipboard,	CeNeedMap);

    Ccmd_SetCurrentGroup(const_cast<char *> ("Miscellaneous"));
    Ccmd_Register(const_cast<char *> ("CM_NOOP"),	CmNoop,	0);
    Ccmd_Register(const_cast<char *> ("CM_RESET"),		CmReset,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_SHOW_FRAME"),		CmShowFrame,	0);


    Ccmd_SetCurrentGroup(const_cast<char *> ("Application"));
    Ccmd_Register(const_cast<char *> ("CM_MONSTERWINDOW"),		CmMonsterWindow,	0);
    Ccmd_Register(const_cast<char *> ("CM_BRUSHWINDOW"),		CmBrushWindow,	CeBrushWindow);
    Ccmd_Register(const_cast<char *> ("CM_RESTORE_TEXTURE"),		CmRestoreTexture,	0);
    Ccmd_Register(const_cast<char *> ("CM_RESTORE_ENTITY"),		CmRestoreEntity,	0);
    Ccmd_Register(const_cast<char *> ("CM_RESTORE_3D"),		CmRestore3D,	0);
    Ccmd_Register(const_cast<char *> ("CM_TOGGLE_CONSOLE"),	CmToggleConsole,	0);
    Ccmd_Register(const_cast<char *> ("CM_UNDO"),		CmUndo,	CeUndo);
    Ccmd_Register(const_cast<char *> ("CM_UNDOSAVE"),		CmUndoSave,	0);
    Ccmd_Register(const_cast<char *> ("CM_WUP"),		WUp,	0);
    Ccmd_Register(const_cast<char *> ("CM_WDOWN"),	WDown,	0);
    Ccmd_Register(const_cast<char *> ("CM_S1"),		W1,	CeW1);
    Ccmd_Register(const_cast<char *> ("CM_S2"),		W2,	CeW2);
    Ccmd_Register(const_cast<char *> ("CM_S3"),		W3,	CeW3);
    Ccmd_Register(const_cast<char *> ("CM_S4"),		W4,	CeW4);
    Ccmd_Register(const_cast<char *> ("CM_SAVE_WINDOWS"),		CmSaveWindowArrangement,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_LOAD_WINDOWS"),		CmWindowPosition,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_TILECHILDREN"),		CmTileChildren,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_CASCADECHILDREN"),		CmCascadeChildren,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_ARRANGEICONS"),		CmArrangeIcons,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_FILESAVE_SEL"),		CmFileSaveSel,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_SETCOLORS"),			CmSetColors,	0);
    Ccmd_Register(const_cast<char *> ("CM_SETGAME"),				CmSetGame,	0);
    Ccmd_Register(const_cast<char *> ("CM_KBSETTINGS"),			CmKBSettings,	0);
    Ccmd_Register(const_cast<char *> ("CM_BSPSETTINGS"),			CmBspSettings,	0);
    Ccmd_Register(const_cast<char *> ("CM_WRITECFG"),			CmWriteConfig,	0);
    Ccmd_Register(const_cast<char *> ("CM_WRITEGAMECFG"),		CmWriteGameConfig,	0);
    Ccmd_Register(const_cast<char *> ("CM_FILENEW"),				CmNew,	CeFileNew);
    Ccmd_Register(const_cast<char *> ("CM_FILEOPEN"),			CmOpen,	CeFileOpen);
    Ccmd_Register(const_cast<char *> ("CM_FILECLOSE"),			CmClose,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_FILESAVE"),			CmSave,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_FILESAVEAS"),			CmSaveAs,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_EXIT"),				CmExit,	0);
    Ccmd_Register(const_cast<char *> ("CM_RELOAD"),				CmReload,	0);
    Ccmd_Register(const_cast<char *> ("CM_ABOUT"),				CmAbout,	0);
    Ccmd_Register(const_cast<char *> ("CM_HELP"),				CmHelp,	0);
    Ccmd_Register(const_cast<char *> ("CM_RECENT0"),				CmRecent0,	0);
    Ccmd_Register(const_cast<char *> ("CM_RECENT1"),				CmRecent1,	0);
    Ccmd_Register(const_cast<char *> ("CM_RECENT2"),				CmRecent2,	0);
    Ccmd_Register(const_cast<char *> ("CM_RECENT3"),				CmRecent3,	0);
    Ccmd_Register(const_cast<char *> ("CM_RECENT4"),				CmRecent4,	0);
    Ccmd_Register(const_cast<char *> ("CM_RECENT5"),				CmRecent5,	0);
    Ccmd_Register(const_cast<char *> ("CM_RECENT6"),				CmRecent6,	0);
    Ccmd_Register(const_cast<char *> ("CM_RECENT7"),				CmRecent7,	0);
    Ccmd_Register(const_cast<char *> ("CM_RECENT8"),				CmRecent8,	0);
    Ccmd_Register(const_cast<char *> ("CM_RECENT9"),				CmRecent9,	0);
    Ccmd_Register(const_cast<char *> ("CM_RECENT10"),			CmRecent10,	0);
    Ccmd_Register(const_cast<char *> ("CM_RECENT11"),			CmRecent11,	0);
    Ccmd_Register(const_cast<char *> ("CM_RECENT12"),			CmRecent12,	0);
    Ccmd_Register(const_cast<char *> ("CM_RECENT13"),			CmRecent13,	0);
    Ccmd_Register(const_cast<char *> ("CM_RECENT14"),			CmRecent14,	0);
    Ccmd_Register(const_cast<char *> ("CM_RECENT15"),			CmRecent15,	0);
    Ccmd_Register(const_cast<char *> ("CM_SHOWKEYMAPPINGS"),		CmShowKeyMappings,	0);

    Ccmd_SetCurrentGroup(const_cast<char *> ("Clippers"));
    Ccmd_Register(const_cast<char *> ("CM_CLIP_CUR_FACE"),		CmClipCurFace,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_CARVE"),				CmCarve,		CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_CLIPSPLIT"),			CmSplit,		CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_FLIPCLIPPER"),			CmFlipClipper,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_CLIPPER_AUTOFLIP"),	CmClipAutoFlip,	CeClipAutoFlip);
    Ccmd_Register(const_cast<char *> ("CM_CLIPPER_MODE_0"),		CmClipMode0,	CeClipMode0);
    Ccmd_Register(const_cast<char *> ("CM_CLIPPER_MODE_1"),		CmClipMode1,	CeClipMode1);
    Ccmd_Register(const_cast<char *> ("CM_CLIPPERS_STAY"),		CmClippersStay,	CeClippersStay);
    Ccmd_Register(const_cast<char *> ("CM_CLIP1"),				CmClip1,		CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_CLIP2"),				CmClip2,		CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_CLIP3"),				CmClip3,		CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_CLIP_MOVE_UP"),		CmClipMoveUp,	CeNeedMap);
    Ccmd_Register(const_cast<char *> ("CM_CLIP_MOVE_DOWN"),		CmClipMoveDown,	CeNeedMap);


    Ccmd_SetCurrentGroup(const_cast<char *> ("OpenGL"));
    Ccmd_Register(const_cast<char *> ("CM_LIST_SKYBOX"),		CmListSkybox,	0);
}




