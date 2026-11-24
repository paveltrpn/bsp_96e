#include "global.h"



WDialog::WDialog(HWND parent, int dlgId)
    : parent(parent), dlgId(dlgId)
{}
WDialog::~WDialog()
{}

WPARAM CALLBACK WDialog::DialogProcRouter(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    WDialog *wnd;
    if(msg == WM_INITDIALOG)
    {
        wnd = (WDialog*)lParam;
#ifdef _WIN64
        SetWindowLong(hwndDlg,GWLP_USERDATA,lParam);
#else
        SetWindowLong(hwndDlg,GWL_USERDATA,lParam);
#endif
        if(!wnd)
            syserror(const_cast<char *> ("WDialog failed to initialize"));
        else
            wnd->hwnd = hwndDlg;
    }
    else
    {
#ifdef _WIN64
        wnd = (WDialog *)GetWindowLong(hwndDlg, GWLP_USERDATA);
#else
        wnd = (WDialog *)GetWindowLong(hwndDlg, GWL_USERDATA);
#endif
    }

    //this bypasses the first few window messages until WM_INITDIALOG is hit
    if(!wnd) return 0;

    return (WPARAM)wnd->DialogProc(msg,wParam,lParam);
}
INT_PTR WDialog::Execute()
{
    return DialogBoxParam(hInstance,MAKEINTRESOURCE(dlgId), parent, (DLGPROC)DialogProcRouter, (LPARAM)this);
}
HWND WDialog::CreateModeless()
{
    return CreateDialogParam(hInstance, MAKEINTRESOURCE(dlgId), parent, (DLGPROC)DialogProcRouter, (LPARAM)this);
}

//
// InputDialog
//

InputDialog::InputDialog(HWND parent,char *title,char*prompt,char*buffer,int bufsize)
    : WDialog(parent, IDD_INPUTDIALOG),
      title(title), prompt(prompt), buffer(buffer), bufsize(bufsize)
{
}
INT_PTR InputDialog::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        SetWindowText(hwnd,title);
        SetDlgItemText(hwnd,ID_PROMPT,prompt);
        SetDlgItemText(hwnd,ID_INPUT,buffer);
        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            GetDlgItemText(hwnd,ID_INPUT,buffer,bufsize);
            //fall...
        case IDCANCEL:
            EndDialog(hwnd, LOWORD(wParam));
        }
        return TRUE;
    case WM_KEYDOWN:
        if(wParam == VK_ESCAPE)
            EndDialog(hwnd, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}

