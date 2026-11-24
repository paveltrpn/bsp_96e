#pragma once
#include <windows.h>
#include "window.h"
#include "wcontrol.h"

//
// WPropEdit
//
class WPropertyGrid;
class WPropEdit : public WEdit
{
public:
    WPropertyGrid *wprop;
    WPropEdit(HWND parent,int id,char * text,int cx,int cy, int width,int height,WPropertyGrid *wprop);
    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);
};

/*
enum EditType {
	ET_NONE,
	ET_EDIT,
	ET_COMBO,
	ET_COLOR
};

//
// WPropItem
//
class WPropItem {
public:
	WPropItem(char *name, char *data, EditType edit_type = ET_NONE, void *param = 0);
	~WPropItem();
	void set_name(char*);
	void set_display(char*);
	void set_value(char*);

	int idx;				// added index
	EditType edit_type;		// editing controls to display
	void *param;			// extra user data
	bool header;			// header item
	bool disabled;			// grayed out
	bool modified;			// set when value changes
	bool error;				// error state
	COLORREF backcolor;		// background color (auto fg), or -1 for auto bg

	char *name;
	int namelen;
	char *value;
	int valuelen;
	char *display;
	int displaylen;
};
//
// WProperties
//
class WProperties : public WControl {
public:
	WProperties(HWND parent,int id,int cx,int cy, int width,int height);
	~WProperties();

	LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);

	list<WPropItem*> items;
	WPropItem *Add(char *name, char *value, EditType et = ET_NONE, void *param = 0);
	void Clear();

	WEdit *wpedit;
	WFloatingList *wplist;
	DrawButton *button;

	WScroll scroll;
	RECT rect;
	bool get_item_rect(int idx, RECT *rc);
	void redraw();
	void mousedown(POINT pt);
	void mouseup(POINT pt);
	void mousemove(POINT pt);
	void resize(int cx, int cy);
	void resize_scrollbar();
	void scroll_into_view();
	int item_from_point(int Y);
	WPITEM *get_item(int idx);
	void start_edit();
	bool stop_edit();
	EditType edit_type;

	int splitter_pos;
	int item_height;
	int selected_idx;
	int edit_idx;

	bool edit_drag_hack;
	int edit_drag_start;
	int edit_drag_mx;

	bool mdown;
	bool splitdown;
	bool seldown;
	bool onsplit;
};
*/


//
// WPropItem
//
class WPropItem
{
public:
    WPropItem(char *name, char *value = 0, DWORD data = 0);
    ~WPropItem();
    void set_name(char*);
    void set_value(char*);

    bool header;			// header item
    bool disabled;			// grayed out
    bool modified;			// set true when name or value has been changed
    COLORREF backcolor;		// -1 for default
    COLORREF forecolor;		// -1 for default

    char *name;
    int namelen;
    char *value;
    int valuelen;

    DWORD data;				// custom data
    //dropdown list support:
    DWORD list_name;		// these are the actual values to use for lists.
    DWORD list_value;		// set_name/set_value are just for display

    int button_type;		// button type of item
    int max_edit_width;		// limit width of edit - use to show bgcolor
};

typedef listitem<WPropItem*> WPITEM;
#define WP_BWIDTH    13

#define PM_CLICK	WM_USER+1	//left single click, wparam=wpropitem, lparam=part
#define PM_RCLICK	WM_USER+2   //right single click, wparam=wpropitem, lparam=part
#define PM_DBLCLICK	WM_USER+3	//left double click, wparam=wpropitem, lparam=part
#define PM_CHANGE	WM_USER+4	//wparam = wpropitem, lparam = new string. return true to accept
#define PM_POPLIST	WM_USER+5	//wparam = wpropitem, lparam = WList
#define PM_EDITBTN	WM_USER+6	//wparam = wpropitem, lparam = 0
#define PM_LIST_CHANGE WM_USER+7	//wparam = list item id
#define PM_TABBED	WM_USER+8	//wparam = shift held
#define PM_SELCHANGE WM_USER+9	//return button part for default edit button

class WPropertyGrid : public WControl
{
public:
    enum Parts {partNone=0,partLeft,partRight,partHeader,partSplitter,partAny};
    enum ButtonType {btypeNone=0,btypeDrop,btypeList};

    WPropertyGrid(HWND parent,int id,int cx,int cy, int width,int height);
    ~WPropertyGrid();

    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);

    list<WPropItem*> items;
    WPropItem *Add(char *name, char *value = 0, DWORD data = 0);
    void Clear();

    WFloatingList *wplist;
    WPropEdit *wpedit;

    WScroll scroll;
    RECT rect;
    DrawButton *edit_button;
    void Redraw();
    void mousedown(POINT pt, int button, bool doubleclick=false);
    void mouseup(POINT pt, int button);
    void mousemove(POINT pt);
    void Resize(int cx, int cy);
    void ResizeScrollbar();
    void ResizeList();
    void ScrollIntoView();
    void MoveSelUp();
    void MoveSelDown();
    void ScrollUp();
    void ScrollDown();
    int ItemFromPoint(int Y);
    void ClickEditButton();
    WPITEM *GetItem(int idx = -1);
    bool GetItemRect(int idx, RECT *rc);

    void ResizeEdit();
    void ShowEdit(int part);
    bool HideEdit(bool change);
    bool edit_visible;
    int edit_part;

    bool modified;
    int splitter_pos;
    int item_height;
    int selected_idx;
    int tmp_idx;
    bool mdown;
    bool splitdown;
    bool onsplit;
};
