#include "global.h"

menu_t *menus;
static menuitem_t **m_stack[MENU_MAX_SUBMENUS];
static int stack_ptr;
void Menu_Initialize()
{
    menus = 0;
    stack_ptr = 0;
}

//recursive func to destroy menuitem_t
void Menu_DestroyR(menuitem_t *menu)
{
    menuitem_t *m = menu, *tmp;
    while(m)
    {
        if((m->flags & MENU_FLAG_SUBMENU) && m->submenu)
        {
            Menu_DestroyR(m->submenu);
            m->submenu = 0;
        }
        if(m->submenu_text)
        {
            delete [] m->submenu_text;
            m->submenu_text = 0;
        }

        tmp = m->next;
        delete m;
        m = tmp;
        if(m == menu) break;
    }
}
//destroy menuitem_t and all children
void Menu_Destroy(menuitem_t *menu)
{
    Menu_DestroyR(menu);
}
menu_t * Menu_AddMenu(char *name, int type)
{
    menu_t *m = new menu_t;
    strncpy(m->name,name,sizeof(m->name));
    m->hMenu = 0;
    m->menu = 0;
    m->type = type;
    m->next = menus;
    menus = m;
    return m;
}
//destroy every menu and list of menus. should be used in final cleanup only
void Menu_DestroyMenus()
{
    menu_t *m = menus, *tmp;
    for( ; m; m = tmp)
    {
        Menu_Destroy(m->menu);
        tmp = m->next;
        delete m;
    }
}
menuitem_t * Menu_AddMenuItem(ccmd_t *c, menuitem_t **menuitem)
{
    menuitem_t *m = new menuitem_t;
    m->cmd = c;
    m->submenu = 0;
    m->submenu_text = 0;
    m->flags = 0;
    if(!*menuitem)
    {
        (*menuitem) = m;
        (*menuitem)->next = m;
        (*menuitem)->prev = m;
    }
    else
    {
        m->prev = (*menuitem)->prev->prev;
        m->next = (*menuitem);
        (*menuitem)->prev->next = m;
        (*menuitem)->prev = m;
    }
    return m;
}

bool Menu_Push(menuitem_t **m)
{
    if(stack_ptr < MENU_MAX_SUBMENUS)
    {
        m_stack[stack_ptr] = m;
        stack_ptr++;
    }
    else
    {
        syserror(const_cast<char *> ("Menu config: menus nested too deep (%d levels max)"), MENU_MAX_SUBMENUS);
        return false;
    }
    return true;
}
menuitem_t **Menu_Pop()
{
    if(stack_ptr < 1)
        return 0;
    stack_ptr--;
    menuitem_t **m = m_stack[stack_ptr];
    m_stack[stack_ptr] = (menuitem_t**)0;
    return m;
}
menuitem_t **Menu_StackTop()
{
    if(stack_ptr < 1)
        return 0;
    return m_stack[stack_ptr - 1];
}

void Menu_LoadConfig(char *file)
{

    char *dat = (char*) file_get_contents(file);
    if(!dat)
    {
        syserror(const_cast<char *> ("Menu config: Couldn't open %s"), file);
        return;
    }

    menuitem_t *current_item = 0;
    memset(m_stack,0,sizeof(m_stack));

    int run = 1;			//
    int level = 0;			//track if more tokens are expected
    int state = 0;			//parser state
    int flags = 0;			//menu flags
    Tokenizer config(dat);
    while(run)
    {
        if(!config.next(true)) break;

        switch(state)
        {
            //new menu
        case 0:			// [MENU] [NAME] (new scope)
            level = 0;
            if(!stricmp(config.token, MENU_TOKEN_MENU))
            {
                if(!config.next(true))
                {
                    run = 0;
                    break;
                }
                Menu_AddMenu(config.token, MENU_TYPE_MENU);	//"menus" head is current menu
                Menu_Push(&menus->menu);
                state = 1; //require submenu at level 1
            }
            else if (!stricmp(config.token, MENU_TOKEN_POPUP))
            {
                if(!config.next(true))
                {
                    run = 0;
                    break;
                }
                Menu_AddMenu(config.token, MENU_TYPE_POPUP);	//"menus" head is current menu
                Menu_Push(&menus->menu);
                state = 7; //add any item
            }
            else
            {
                run = 0;
                break;
            }
            if(!config.next(true))
            {
                run = 0;
                break;
            }
            if(!strcmp(config.token, MENU_TOKEN_ENTER_SCOPE))
            {
                break;
            }
            run = 0;
            break;
        case 1:
            if(stricmp(config.token, MENU_TOKEN_SUBMENU))
            {
                run = 0;
                break;
            }
            if(!config.next(true))
            {
                run = 0;
                break;
            }
            current_item = Menu_AddMenuItem(0,&menus->menu);
            current_item->submenu_text = new char [strlen(config.token) + 1];
            strcpy(current_item->submenu_text, config.token);
            Menu_Push(&current_item->submenu);
            flags = MENU_FLAG_SUBMENU;
            level = 1;
            state = 2;
            break;
        case 2:		// submenu flags
        case 3:		// menuitem flags
            if(!stricmp(config.token,MENU_TOKEN_SOFT))
            {
                flags |= MENU_FLAG_SOFT;
            }
            else if(!stricmp(config.token,MENU_TOKEN_GL))
            {
                flags |= MENU_FLAG_GL;
            }
            else if(!stricmp(config.token,MENU_TOKEN_BMP))
            {
                flags |= MENU_FLAG_BMP;
            }
            else if(!stricmp(config.token,MENU_TOKEN_GAME))
            {
                if(!config.next(true))
                {
                    run = 0;
                    break;
                }
                flags &= ~MENU_FLAG_GAME_MASK;
                flags |= MENU_MAKE_GAME_VAL(atoi(config.token));
                flags |= MENU_FLAG_GAME;
            }
            else
            {
                config.putback();
                state += 3;
            }
            break;
            //	case 4
        case 5:		// submenu scope
            if(stricmp(config.token, MENU_TOKEN_ENTER_SCOPE))
            {
                run = 0;
                break;
            }
            current_item->flags = flags;
            level++;
            state = 7;
            break;
        case 6:		// next item
            current_item->flags = flags;
            //fall
        case 7:		// menuitem
            flags = 0;
            if(!strcmp(config.token, MENU_TOKEN_LEAVE_SCOPE))
            {
                --level;
                if(level == 0)  	//menu scope
                {
                    state = 0;
                }
                else
                    state = 7;
                Menu_Pop();
                break;
            }
            else if (!stricmp(config.token, MENU_TOKEN_SUBMENU))
            {
                // add submenu
                if(!config.next(true))
                {
                    run = 0;
                    break;
                }
                menuitem_t **m = Menu_StackTop();
                current_item = Menu_AddMenuItem(0, m);
                current_item->submenu_text = new char[strlen(config.token) + 1];
                strcpy(current_item->submenu_text, config.token);
                flags |= MENU_FLAG_SUBMENU;
                Menu_Push(&current_item->submenu);
                state = 2; //get flags
                break;
            }
            else if (!stricmp(config.token, MENU_TOKEN_SEPARATOR))
            {
                // add exporter list
                menuitem_t **m = Menu_StackTop();
                current_item = Menu_AddMenuItem(0, m);
                flags |= MENU_FLAG_SEPARATOR;
                state = 3;
                break;
            }
            else if (!stricmp(config.token, MENU_TOKEN_RECENT))
            {
                // add recentlist
                menuitem_t **m = Menu_StackTop();
                current_item = Menu_AddMenuItem(0, m);
                flags |= MENU_FLAG_RECENT;
                state = 3;
                break;
            }
            else if (!stricmp(config.token, MENU_TOKEN_EXPORT))
            {
                // add exporter list
                menuitem_t **m = Menu_StackTop();
                current_item = Menu_AddMenuItem(0, m);
                flags |= MENU_FLAG_EXPORT;
                state = 3;
                break;
            }
            else
            {
                // regular menuitem cmd
                ccmd_t *c = Ccmd_FindCmd(config.token);
                if(!c)
                    syserror(const_cast<char *> ("Menu config: unknown command \"%s\" in file %s (Line: %d)"), config.token, file, config.line);
                menuitem_t **m = Menu_StackTop();
                current_item = Menu_AddMenuItem(c, m);
                state = 3;
                break;
            }
        }
    }
    if(level)
    {
        syserror(const_cast<char *> ("Menu config: "));
    }
}

HMENU Menu_Build(HMENU hmenu,menuitem_t *menu, int level)
{
    menuitem_t *m = menu;
    int menupos = 0;
    while(m)
    {
        bool skip = false;
        if(set.glBsp && m->flags & MENU_FLAG_SOFT && !(m->flags & MENU_FLAG_GL))
            skip = true;
        if(!set.glBsp && m->flags & MENU_FLAG_GL && !(m->flags & MENU_FLAG_SOFT))
            skip = true;
        if(m->flags & MENU_FLAG_GAME && MENU_GET_GAME_VAL(m->flags) != set.game_mode)
            skip = true;

        if(!skip)
        {
            //new menu to insert
            MENUITEMINFO mii;
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_TYPE;
            bool skip_mii_ins = false;

            if(m->flags & MENU_FLAG_SEPARATOR)
            {
                mii.fType = MFT_SEPARATOR;
            }
            else
            {
                mii.fMask = MIIM_TYPE;
                mii.fType = MFT_STRING;
                if(m->cmd)
                {
                    char mtext[100], shortcut[40];
                    kbset->ShowAccelsForCommand(m->cmd->id, shortcut, sizeof(shortcut));
                    if(*shortcut)
                    {
                        char *menu_text = m->cmd->menu_text;
                        if(!menu_text) menu_text = m->cmd->name; //use cmd name as backup
                        snprintf(mtext,sizeof(mtext),"%s\t%s", menu_text, shortcut);
                        mii.dwTypeData = mtext;
                    }
                    else
                    {
                        if(m->cmd->menu_text)
                            mii.dwTypeData = m->cmd->menu_text;
                        else
                            mii.dwTypeData = m->cmd->name; //use cmd name as backup
                    }
                    mii.fMask |= MIIM_ID;
                    mii.wID = m->cmd->id;
                }
                else if(m->flags & MENU_FLAG_SUBMENU)
                {
                    mii.dwTypeData = m->submenu_text;
                    mii.fMask |= MIIM_SUBMENU;
                    mii.hSubMenu = Menu_Build(CreatePopupMenu(),m->submenu, level+1);
                }
                else if(m->flags & MENU_FLAG_RECENT)
                {
                    int c = BuildRecentMenu(hmenu);
                    menupos += c;
                    skip_mii_ins = true;
                }
                else if(m->flags & MENU_FLAG_EXPORT)
                {
                    int c = BuildExporterMenu(hmenu);
                    menupos += c;
                    skip_mii_ins = true;
                }
                else
                {
                    mii.dwTypeData = const_cast<char *> ("(Invalid menu item)");
                }
            }
            if(!skip_mii_ins)
            {
                mii.cch = 0;
                if(mii.dwTypeData)
                    mii.cch = strlen(mii.dwTypeData);
                InsertMenuItem(hmenu,menupos,1,&mii);
                menupos++;
            }
        }
        m = m->next;
        if(m == menu) break;
    }
    return hmenu;
}
//creates HMENU from menu def. destroys old HMENU
bool Menu_Create(menu_t *menu)
{
    if(!menu)
        return false;
    if(!menu->menu)
        return false;
    if(menu->hMenu) DestroyMenu(menu->hMenu);
    if(menu->type == MENU_TYPE_MENU)
        menu->hMenu = CreateMenu();
    else if (menu->type == MENU_TYPE_POPUP)
        menu->hMenu = CreatePopupMenu();
    else
        return false;
    return 0 != Menu_Build(menu->hMenu,menu->menu, 1);
}
//find menu in list.. not designed to be fast if there are a lot of menus....
menu_t *Menu_Find(char *name)
{
    menu_t *m = menus;
    for(; m; m=m->next)
    {
        if(!stricmp(m->name,name))
            return m;
    }
    return 0;
}

