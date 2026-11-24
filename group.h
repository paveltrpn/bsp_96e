#ifndef _GROUP_H_
#define _GROUP_H_

#define MAX_GROUPS 256
#define MAX_GROUP_NAMESIZE 64

////////////////
#define GB1 1
#define GB2 2
#define GB3 3
#define GB4 4
#define GB5 5
#define GB6 6
#define GB7 7
#define GB8 8
#define GB9 9
#define GB91 91
#define GB10 10
#define GB11 11
#define GB12 12
#define GB13 13
#define GB14 14
#define GB15 15
#define GB16 16
#define GB17 17
#define GB18 18
#define GB19 19

//
// Group Commands
//
extern void CmGroupNewGroup();
extern void CmGroupNewSpacer();
extern void CmGroupColor();
extern void CmGroupDelete();
extern void CmGroupAddSel();
extern void CmGroupSelectAll();
extern void CmGroupGoto();
extern void CmGroupVisible();
extern void CmGroupRename();
extern void CmGroupUp();
extern void CmGroupDown();
extern void CmGroupSingle();
//
// Group Enablers
//
extern void CeGroupNeedSel(int id);
extern void CeGroupMoveUp(int id);
extern void CeGroupMoveDown(int id);


// Group 0 default color is color_foreground

typedef struct group_s
{
    char name[MAX_GROUP_NAMESIZE];  // group name
    int visible;    // on or off
    int r, g, b;    // group display color
    int numberofobjects;
    int isSpacer;
} group_t;

class Group;

class TBSPGroupListBox : public WList
{
public:
    TBSPGroupListBox(HWND parent,int id,int cx,int cy, int width,int height);

    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);

    char tempGroup[MAX_GROUP_NAMESIZE];

    bool onPair;
    bool onSpacer;

    void EvRButtonDown(LPPOINT point);
    bool EvEraseBkgnd(HDC);

    void MenuUseGroups();
    void MenuVisible();
    void MenuColor();
    void MenuNew();
    void MenuNewSpacer();
    void MenuRename();
    void MenuDelete();
    void MenuHide();
    void MenuMoveto();
    void MenuAllVisible();
    void MenuHideAll();
    void MenuUngroupedOnly();
    void MenuSelectAll();
    void MenuExportCurrent();
    void MenuExportVisible();
    void MenuShowResults();
    void MenuMoveGroupUp();
    void MenuMoveGroupDown();
    void MenuGotoGroup();

    bool SetTempValues(LPPOINT pt);
};

class Group
{
public:
    Group();							// constructor
    ~Group();

    Group *copy();

    int count;
    group_t objects[MAX_GROUPS];

    void ClearGroups();					// kill them all
    int AddGroup(char *name);			// returns the index
    int AddSpacer(char *name);			// returns the index
    int GetIndex(char *name);			// get index of a group by name
    char *GetName(int index);
    void GetLongName(int index, char *outstr); // with visible [x] box

    void DeleteGroup(int index);		// delete a group, all brushes go t
    // unselected group (0)
    void Rename(int index, char *newname); // sets name of group index to newname

    COLORREF GetGroupColor(int index);
    void SetGroupColor(int index, COLORREF rgb);

    int  GetVisible(int index);			// returns visibility state
    void SetVisible(int index, int yesno);

    int NumGroups();					// returns number of groups
    void CalcNumObjects();				// returns number of ents/brushes with group #
    int GetNumObjects(int index);		// returns # after Calc!! has been run
    void AddSelectedToGroup(int index);

    void SelectGroup(int index);

    void MoveGroupUp(int index);
    void MoveGroupDown(int index);

    bool IsSpacer(int index);
};

class TGroupWindow : public TCWindow, public WindowPlacement
{
public:
    TGroupWindow(HWND parent, char *title);
    ~TGroupWindow();
    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);

    HWND WP_GetHwnd();
    const char *WP_WindowName();

    TBSPGroupListBox *GroupList;
    WToolbar *toolbar;
    WRebar *rebar;

    int curindex;
    int bw, bh;
    int fw;
    int gfSize;
    int groupSize;
    HFONT groupFont;
    HBRUSH backcolor;

    bool CanClose();
    void UseGroup();
    void Paint(HDC hdc);
    void SetupWindow();
    void EvSelDouble();
    void RedrawAll();
    void RedrawReload();
    void EvDrawItem(UINT, LPDRAWITEMSTRUCT);
    void EvMeasureItem(UINT, LPMEASUREITEMSTRUCT);
    void LoadData();
    void Invalidate();
    void Resize();
};


#endif	//_GROUP_H_
