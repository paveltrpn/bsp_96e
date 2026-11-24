#ifndef _WCONTROL_H_
#define _WCONTROL_H_

//
// WControl
//
class WControl
{
public:
    HWND hwnd;
    WNDCLASSEX wc;
    WNDATTR attr;
    char caption[256];

protected:
    bool created;

//methods
public:
    //constructors
    WControl(HWND parent,int id,char * text,int cx,int cy, int width,int height);	//createwindow
    WControl(HWND parentdlg, int id, bool subclass = false);	// dialogs
    virtual ~WControl();

    static LRESULT CALLBACK MsgRouter(HWND hwnd, UINT msg,WPARAM wParam,LPARAM lParam);
    virtual LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);
    WNDPROC oldwndproc;

    bool Create();

    LRESULT SendMessage(UINT msg,WPARAM wParam,LPARAM lParam);

    void SetCaption(char *caption);
    void SetFont(HFONT font,int redraw=0);
};



//
// WEdit
//
class WEdit : public WControl
{
public:
    WEdit(HWND parent,int id,char *text,int cx,int cy, int width,int height,int textLimit=0, bool multiline=false);
    WEdit(HWND parentdlg, int id,const char *text=0, int textLimit=0, bool subclass=false);
    int GetText(char *lpString, int textLimit=-1);	// if <0, use MaxLen from constructor
    void SetText(const char *str);
    void SetMaxText(int maxlen);
    void SetInt(int val);
    int GetInt();
    void SetFloat(float val);
    float GetFloat();
    int MaxLen;
};

//
// WStatic
//
class WStatic : public WControl
{
public:
    WStatic(HWND parent,int id,char * text,int cx,int cy, int width,int height);
    WStatic(HWND parentdlg, int id, bool subclass = false);
    void SetText(char *str);
};

//
// WButton
//
class WButton : public WControl
{
public:
    WButton(HWND parent,int id,char * text,int cx,int cy, int width,int height);
    WButton(HWND parentdlg, int id,bool subclass = false);
};
//
// WCheck
//
class WCheck : public WButton
{
public:
    WCheck(HWND parent,int id,char * text,int cx,int cy, int width,int height);
    WCheck(HWND parentdlg, int id,int defValue=BST_UNCHECKED,bool subclass = false);
    int GetCheck();
    void SetCheck(int checkstate);
};

//
// WRadio
//
class WRadio : public WCheck
{
public:
    WRadio(HWND parent,int id,char * text,int cx,int cy, int width,int height);
    WRadio(HWND parentdlg, int id,int defValue=BST_UNCHECKED,bool subclass = false);
};

//
// WList - ListBox
//
class WList : public WControl
{
public:
    WList(HWND parent,int id,int cx,int cy, int width,int height);
    WList(HWND parentdlg, int id, bool subclass = false);
    int AddString(char *str);
    void ClearList();
    void InsertString(char *str, int index = -1);
    int GetString(char *str, int index);
    int GetSelIndex();
    void SetSelIndex(int index);
    void DeleteString(int index);
    int GetCount();
    int GetItemHeight(int index);
    int GetTopIndex();
    DWORD GetItemData(int index);
    bool SetItemData(int index, DWORD data);
};


//
// WCombo - ComboBox
//
class WCombo : public WControl
{
public:
    WCombo(HWND parent,int id,int cx,int cy, int width ,int height, int textLimit=0);
    WCombo(HWND parentdlg, int id, int textLimit=0, bool subclass=false);
    void LimitText(int len);
    int SetEditSel(int start, int end);
    int SetSelString(const char *lpszSelect, int indexStart);
    void SetText(const char *str);
    int ClearList();
    int InsertString(char *str, int index = -1);
    int GetText(char *lpString, int nMaxCount = -1);
    int FindString(char *lpszFind, int index = -1);
    int FindStringExact(char *lpszFind, int index = -1);
    int GetString(char *lpszBuffer, int index);
    int GetSelIndex();
    int SetSelIndex(int index);
    int DeleteString(int index);
    int GetCount();
    int GetItemHeight(int index);
    int GetTopIndex();
    int ShowList();
    int HideList();
    int AddString(char *str);
    DWORD_PTR GetItemData(int index);

    int MaxLen;
};

//
// WListView
//
class WListView : public WControl
{
public:
    WListView(HWND parent,int id,int cx,int cy, int width,int height);

    int AddColumn(const char *text, int width);
    int AddItem(const char *text);
};




#endif	//_WCONTROL_H_


