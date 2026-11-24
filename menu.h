#ifndef _MENU_H_
#define _MENU_H_

//flags
#define MENU_FLAG_SOFT                1			//option visible in software mode only
#define MENU_FLAG_GL                  2			//option visible in gl mode only
#define MENU_FLAG_BMP				  4			//menu should draw assoc. bmp
//							   0xF8 reserved for new rendering modes or stuff
#define MENU_FLAG_SUBMENU	      0x100			//menuitem is submenu rather than ccmd_t
#define MENU_FLAG_SEPARATOR	      0x200			//separator
#define MENU_FLAG_RECENT		  0x400			//evaluate to recent list
#define MENU_FLAG_EXPORT		  0x800			//evaluate to export list
#define MENU_FLAG_GAME		     0x1000			//check game value
#define MENU_FLAG_GAME_MASK  0xF0000000			//upper 4 bits == set.game_mode
#define MENU_MAKE_GAME_VAL(v) (((v)&0xF)<<28)	//convert game number to or'able value
#define MENU_GET_GAME_VAL(v) ((int)(((v)>>28)&0xF))
//tokens
#define MENU_TOKEN_SOFT			"SOFT"
#define MENU_TOKEN_GL			"GL"
#define MENU_TOKEN_GAME			"GAME"
#define MENU_TOKEN_BMP			"BMP"
#define MENU_TOKEN_MENU			"MENU"
#define MENU_TOKEN_POPUP		"POPUP"
#define MENU_TOKEN_SUBMENU		"SUBMENU"
#define MENU_TOKEN_ENTER_SCOPE	"{"
#define MENU_TOKEN_LEAVE_SCOPE	"}"
#define MENU_TOKEN_SEPARATOR	"-"
#define MENU_TOKEN_RECENT		"recent_files"
#define MENU_TOKEN_EXPORT		"exporters"
//menu_t types
#define MENU_TYPE_MENU			1
#define MENU_TYPE_POPUP			2
//stuff
#define MENU_MAX_SUBMENUS 8

//built-in menus
#define MENU_BSP_MAIN "bspmenu"

typedef struct menuitem_s
{
    ccmd_t *cmd;
    uintptr_t flags;
    char *submenu_text;
    menuitem_s *submenu;
    menuitem_s *next;
    menuitem_s *prev;
} menuitem_t;

typedef struct menu_s
{
    char name[30];
    HMENU hMenu;
    int type;
    menuitem_t *menu;
    menu_s *next;
} menu_t;
extern void Menu_Initialize();
extern void Menu_Destroy();
extern void Menu_DestroyMenus();
extern void Menu_LoadConfig(char *file);

extern bool Menu_Create(menu_t *menu);
extern menu_t *Menu_Find(char *name);

#endif //_MENU_H_
