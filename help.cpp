#include "global.h"

WHelpEdit::WHelpEdit(HWND parent,int id,char *text,int cx,int cy, int width,int height)
    : WEdit(parent,id,text,cx,cy,width,height,0,true)
{}
LRESULT WHelpEdit::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_MOUSEWHEEL:
        if(SetFocusUnderMouse(GetParent(hwnd)))
            return 0;
    }
    return WControl::WndProc(msg,wParam,lParam);
}

THelpWindow::THelpWindow(HWND parent, char *caption)
    : TCWindow(parent,caption)
{
    attr.width = 250;
    attr.height = 320;
    attr.dwStyle |= WS_BORDER;

    helpEdit = 0;

    helpfont = GetFont(set.help_font,-set.help_font_size);
}
THelpWindow::~THelpWindow()
{
    DeleteObject(helpfont);
}
bool THelpWindow::CanClose()
{
    if (!KillAllWindows)
    {
        ShowWindow(SW_HIDE);
    }
    return (0 != KillAllWindows);
}
LRESULT THelpWindow::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_CREATE:
        helpEdit = new WHelpEdit(hwnd,IDE_HELPEDIT,const_cast<char *> (""),5, 5, 32, 32);
        helpEdit->Create();
        helpEdit->SendMessage(WM_SETFONT,(WPARAM)helpfont,0);
        SetIcon(IDI_HELP);
        break;
    case WM_MOUSEWHEEL:

        if(SetFocusUnderMouse(hwnd))
            return 0;

        ::SetFocus(helpEdit->hwnd); //send focus to edit to handle wheel

        return 0;

    case WM_SIZE:
    {
        if(IsIconic() || wParam == SIZE_MINIMIZED)
            break;
        int w = (short)LOWORD(lParam) - 10;
        int h = (short)HIWORD(lParam) - 10;

        if (w > 0 && h > 0)
        {
            MoveWindow(helpEdit->hwnd,5,5,w,h,true);
        }
    }
    break;
    }
    return TCWindow::WndProc(msg,wParam,lParam);
}
