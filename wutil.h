#ifndef _WUTIL_H_
#define _WUTIL_H_

//
// WProfile
//
class WProfile
{
public:
    char section[MAX_PATH];
    char inifile[MAX_PATH];

    WProfile(const char *section,const char *inifile);
    ~WProfile();

    int GetInt(const char *key,int defvalue = 0);
    bool GetString(const char *key,char *buffer,int size,const char *defvalue = "");
    COLORREF GetColor(const char *key, COLORREF defvalue);
    bool WriteInt(const char *key, int value);
    bool WriteString(const char *key, const char *value);
    bool WriteColor(const char *key, const COLORREF value);
};

//
// GetFont
//
extern HFONT GetFont(const char *fontname,int fontsize,int width=0,int weight=FW_NORMAL);

//
// WPath - manage a filename with path, and provide original name plus ".." expanded name
//
/*
class WPath {
char *sourcepath;
char *fullpath;
int fplen;
char *partdir, *partfile, *partext;
bool dir_only;
public:
	WPath(char *path, bool dir_only);
	~WPath();
	void setPath(char *path, bool dir_only);
	char *sourcePath();
	char *fullPath();
	char *dir();
	char *file();
	char *ext();
protected:
	void clean();
};
*/

class Path
{
public:
    char _path[300];
    bool valid;
    static char _curdir[300];
    Path() : valid(false)
    {
        *_path=0;
    }
    Path(char *text, ...);
    static void ChangeDir(Path p);
    void SetPath(char *text, ...);
    void _setpath(char*text,va_list args);
    bool Exists();
    char *ext();
    operator char*();
    char *operator =(char*rhs);
};

//
// WImageList
//
// this class allows a 'big' and 'small' image list.
#define WIL_BIG_CX 20
#define WIL_BIG_CY 20
#define WIL_SMALL_CX 16
#define WIL_SMALL_CY 15
#define WIL_SMALL 0
#define WIL_BIG   1
#define WIL_HASH_SIZE 300
typedef struct wil_data_s
{
    HIMAGELIST him;
    int index;
    int cmd;
    wil_data_s *hash_next;
    wil_data_s *next;
} wil_data_t;
class WImageList
{
public:
    HIMAGELIST imbig, imsmall;
    WImageList();
    ~WImageList();
    int Add(char *bmp, int cmd_id);
    int GetImage(int cmd_id);
private:
    wil_data_t *head;
    wil_data_t *hash[WIL_HASH_SIZE];
    int hash_key(int cmd_id);
    wil_data_t *find_img(int cmd_id);
    int add_img(HIMAGELIST im,HBITMAP bmp,int cmd_id);
};

//
// ColorDialog
//
class ColorDialog
{
public:
    ColorDialog(HWND owner,COLORREF defcolor,char *title = 0);
    static UINT_PTR * CALLBACK CCHookProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    bool Show();
    CHOOSECOLOR cc;
    COLORREF result;					// shortcut for cc.rgbResult
    char caption[256];
    static COLORREF custColors[16];		// custom colors
};


//
// Popup
//
class Popup
{
public:
    Popup();									// create with window that will receive WM
    ~Popup();									// destroys menu
    // methods
    void Separator();							// add separator
    void Add(const char *text, int id, bool check = false);	// add item
    void Add(const char *text, Popup submenu);	// add submenu
    void Check();								// check next item
    void Gray();								// gray and disable next item
    void Show(HWND hwnd);						// display popup
    // var
    HMENU menu;
    HWND hwnd;
    POINT pt;
    int lastid;
    int flags;
};


//
// WScroll
//
class WScroll
{
public:
    WScroll(HWND owner);
    void Update();
    void Sync();
    bool VScroll(WPARAM wParam, int pagesize);
    SCROLLINFO X, Y;
    HWND hwnd;
};


//
// linked list class to pass data to dialog listboxes.
//
typedef struct ListDataItem
{
    char *str;				// item text
    bool sel;				// selected
    ListDataItem *next;
} t_ListDataItem;

//ListData
class ListData
{
public:
    ListData();
    ~ListData();
    void Add(const char *str, bool sel = false);
    void Clear();
    bool hasNext();
    const char *Next();
    void FillList(HWND listbox);
    void FillCombo(HWND combobox);
    int count;
private:
    void clear_items();
    void reset_ptrs();
    ListDataItem *items;	// list of items
    ListDataItem **tail;	// where to add last item
    ListDataItem **cur;		// for tracking iterator position
};



#define TB_BMP_WIDTH 20
#define TB_BMP_HEIGHT 20
#define TB_BTN_WIDTH 22
#define TB_BTN_HEIGHT 24
#define TB_CTRL_PADDING 4

//
// WReBar
//

class WRebar
{
public:
    WRebar(HWND parent);
    ~WRebar();
    void AddBand(HWND child);
    void Show(int index);
    void Hide(int index);

    HWND hwnd;
};


//
// WToolBar
//
class WToolbar
{
public:
    WToolbar(HWND parent, int bmpcx=TB_BMP_WIDTH, int bmpcy = TB_BMP_HEIGHT,
             int btncx=TB_BTN_WIDTH, int btncy=TB_BTN_HEIGHT);
    ~WToolbar();
    void AddSeparator();
    void AddButton(int imgId, int cmdId);
    void AddControl(HWND hwndchild);

    HWND hwnd;
    int index;		// zero-based index position of band in rebar
    bool visible;	// parent band visibility
};



//
// WStatus
//
#define SB_HEIGHT 18
#define SB_SPACE   2
#define SB_MAXTEXT 127
class WStatus
{
public:
    typedef struct partinfo_s
    {
        int cx;			//width of part
        int left;		//calculated left coord
        bool ownerdraw;	//dont draw if set
        int bmpid;		//draw bitmap instead of text if >= 0
        char text[SB_MAXTEXT];	//text
    } partinfo_t;

    WStatus(HWND parent, int numParts);
    ~WStatus();
    void DrawStatusBar();
    bool IsVisible();
    void Show(bool visible, bool redraw = false);
    void GetRect(RECT *rc);
    void GetPartRect(int index, RECT *rc);
    void SetFont(HFONT font);
    void SetPart(int index, int width, bool charwidth=false, bool ownerdraw=false);
    void SizePartToText(int index);
    void SetText(int index, const char *text, bool redraw = false);
    void SetText(const char *text, bool redraw = false);	//set first item
    void Redraw();
    void SetPartBmp(int index, int bmpid);
    void SetBmp(HBITMAP bmp, int piece_width);

protected:
    void ConstructMemDC();
    void RemoveMemDC();
    void CheckMemDC();

    HWND parent;
    HDC sdc;
    HBITMAP sbmp;
    HFONT font;
    HBITMAP bmp_strip;
    int bmp_cx;
    RECT sbrc;
    int numParts;
    partinfo_t *parts;
    bool visible;
};


//
// Keyboard Settings
//
typedef struct KBAccel_s
{
    ACCEL key;
    KBAccel_s *next;
} KBAccel_t;

class KBSettings
{
public:
    KBSettings();
    KBSettings(const KBSettings& kbs);
    ~KBSettings();
    void ClearKeys();				// delete KBAccel_t list
    void CopyKeys(const KBSettings& source);	//copy keys from source. keys must be cleared first!!
//	void LoadAccels(HACCEL accel);	// load *keys from an existing table
    HACCEL CreateAccelTable();		// create table from *keys
    void OverwriteAndActivate(KBSettings *dest_kbset);	// copy KBSettings and recreate main accel table
    void SaveToDisk();				// save keyboard settings
    void LoadFromDisk(char *filename);	// load keyboard settings
    static void ShowKeys(HACCEL accel);	// show all key mappings in help window
    static char* AccelToText(ACCEL k, char* buf);	// convert ACCEL to text
    char* AccelToCfgText(ACCEL k, char* buf);	// convert ACCEL to config line
    bool ParseCfgText(char *mod, char *key, char*cmd, ACCEL *ret);	// convert config line to ACCEL
    //nowhere to load defaults from... - void RestoreDefaults(HACCEL defaccel);	// reset keys to just those in resource
    void ShowAccelsForCommand(int cmd, char *out, int outlen);	// return string of all shortcuts used by cmd
    ACCEL *FindAccel(ACCEL k, bool matchkey, bool matchcmd);	// find ACCEL in *keys
    void AddAccel(ACCEL k);			// add key to *keys
    bool AddReplaceAccel(ACCEL *k);	// add key, replace if exists
    bool RemoveAccel(ACCEL k);		// remove by shortcut, ignores cmd

protected:
    KBAccel_t *keys;
};




/*
//
// Registry
//
#define REG_BASEPATH "software\\BSP Quake Editor"
class Registry {
public:
	Registry(const char *path);
	~Registry();
	void ChangePath(const char *path);

	bool GetValue(const char *name, DWORD *value);
	bool GetValue(const char *name, char *value, int maxlen);

	bool SetValue(const char *name, DWORD value);
	bool SetValue(const char *name, const char *value);
private:
	HKEY key;
	char path[256];
};
*/

//
// WindowPlacement
//
#define WP_REGNAME "Window Positions"
class WindowPlacement
{
public:
    WindowPlacement() {};
    virtual const char *WP_WindowName() = 0;
    virtual HWND WP_GetHwnd() = 0;

    //functions per window, per slot
    void WP_GetPos();	//fill wp and zorder
    void WP_SetPos();	//set window position from wp (no zorder here)
    void WP_SavePos(int slot);	//save to config
    void WP_LoadPos(int slot);	//load from config

    WINDOWPLACEMENT WP_wp;
    int WP_zorder;
    bool WP_isvalid;		//set after changing WP_wp

    //all windows, one slot
    static void SavePositions(int slot);	//save slot to file
    static void LoadPositions(int slot);	//load slot from file
};

//
// DrawButton
//

#define DRAWB_HIDE  	0
#define DRAWB_UP		1
#define DRAWB_DOWN		2
#define DRAWB_ELLIPSES	3
class DrawButton
{
public:
    DrawButton(HWND parent, int id, int type, RECT *rc);

    HWND parent;
    int id;
    int type;
    RECT rc;
    bool mdown, pushed, enabled, visible;

    bool mousedown(POINT pt);
    bool mouseup(POINT pt);
    void mousemove(POINT pt);
    void redraw();
    void redraw(HDC hdc);
};


#endif	//_WUTIL_H_
