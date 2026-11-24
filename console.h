#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#define IDC_INPUT 100
#define IDC_DISPLAY 101

class BSPConsole;
class WConInput;

typedef struct conhist_s
{
    char *line;
    conhist_s *next;
    conhist_s *prev;
} conhist_t;

class ConsoleHistory
{
public:
    conhist_t *head;	//the list
    conhist_t *tail;	//last item in list
    conhist_t *current;	//current nav position
    int count;
    int maxsize;
    char *activebuf;	//copy of users active buffer, item 'before' head

    ConsoleHistory(int size = 100);
    ~ConsoleHistory();

    bool add(char *line);	//add a line, set current == head
    char *next();			//move to next item, return line
    char *prev(char *active);	//move to prev item, return line. if cur=head, set activebuf
    void clear();			//remove all items and reset
    void trim();			//adjust list to fit within maxsize
    void setsize(int size);	//set max size and trim
};
class WConDisplay : public WEdit
{
public:
    WConDisplay(HWND parent,int id,char *text,int cx,int cy, int width,int height);
    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);
    BSPConsole *con;
};
class WConInput : public WEdit
{
public:
    WConInput(HWND parent,int id,char *text,int cx,int cy, int width,int height);
    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);
    BSPConsole *con;
};

class BSPConsole : public TCWindow, public WindowPlacement
{
public:
    friend class WConInput;
    friend class WConDisplay;
    BSPConsole(HWND parent, char *caption);
    ~BSPConsole();
    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);
    bool CanClose();
    WConDisplay *display;
    WConInput *input;
    ConsoleHistory *history;
    int textwidth;		//number of char columns visible


    //WindowPlacement
    HWND WP_GetHwnd();
    const char *WP_WindowName();

    void WriteV(char *text, va_list args);
    void Writef(char *text, ...);
    void Write(char *text);
    void ScrollEnd();
    void Scroll(int lines);
    void PageUp();
    void PageDown();
    void ClearScreen();
    void DebugCommands(char *cmd);
    void ShowHelp(char *args);
    void ShowHelpItem(char *args);
    void ShowGroups(char *args);
private:
    void process_buffer();
    void display_columns(list<char*>& list);
    void tab_complete();
    HFONT font;
    HBRUSH bgbrush;
};

#endif	//_CONSOLE_H_
