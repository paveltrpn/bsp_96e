#include "global.h"
#include "buildver.h"

/*
=================
TAboutDlg:

About Box
=================
*/

TAboutDlg::TAboutDlg(HWND parent)
    : WDialog(parent,ABOUT_DIALOG)
{
    title = 0;
    thanks = 0;
    desc = 0;
    version = 0;
    build = 0;
}
TAboutDlg::~TAboutDlg()
{
    delete title;
    delete thanks;
    delete desc;
    delete version;
    delete build;
}
INT_PTR TAboutDlg::DialogProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:

        title = new WStatic(hwnd, IDA_ST0);
        thanks = new WStatic(hwnd, IDA_ST6);
        desc = new WStatic(hwnd, IDA_ST7);
        version = new WStatic(hwnd, IDA_ST4);
        build = new WStatic(hwnd, IDA_ST5);

        SetupWindow();
        return TRUE;
    case WM_PAINT:
        EvPaint();
        return FALSE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
        case IDCANCEL:				//handling cancel here lets you close with "x"
            EndDialog(hwnd,0);
            return TRUE;
        }
    case WM_KEYDOWN:
        if(wParam == VK_ESCAPE)
            EndDialog(hwnd,0);
        return TRUE;
    }
    return FALSE;
}

void TAboutDlg::SetupWindow()
{
    CenterWindow(hwnd);

    // title
    title->SetText(const_cast<char *>("About BSP..."));

    //about.txt
    char *text = (char *)file_get_contents(const_cast<char *>("about.txt"));
    desc->SetText(text ? text : const_cast<char *>("Couldn't locate about.txt :<"));
    delete[] text;

    //thanks.txt
    text = (char *)file_get_contents(const_cast<char *>("thanks.txt"));
    thanks->SetText(text ? text : const_cast<char *>("Couldn't locate thanks.txt :<"));
    delete[] text;

    // version info
    version->SetText(Str(const_cast<char *>("Version %d.%d"BSP_REV" (%d.%d)"), BSP_VER_MAJOR, BSP_VER_MINOR, _BUILDNUM, _BUILDREV));
    build->SetText(const_cast<char *> ("Date:  " _BUILDDATE));
}

void TAboutDlg::EvPaint()
{
    HDC hdc = GetDC(hwnd);
    DrawIcon(hdc, 10, 10, LoadIcon(hInstance,MAKEINTRESOURCE(IDI_QUAKE)));
    ReleaseDC(hwnd, hdc);
}

