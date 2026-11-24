#ifndef _HELP_H_
#define _HELP_H_

#define IDE_HELPEDIT 98

class WHelpEdit : public WEdit
{
public:
    WHelpEdit(HWND parent,int id,char *text,int cx,int cy, int width,int height);
    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);
};

class THelpWindow : public TCWindow
{
public:
    THelpWindow(HWND parent, char *caption);
    ~THelpWindow();
    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);
    bool CanClose();

    WHelpEdit *helpEdit;
private:
    HFONT helpfont;
};

#endif	//_HELP_H_
