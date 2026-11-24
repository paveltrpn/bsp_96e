#ifndef _ABOUT_H_
#define _ABOUT_H_

// About Dialog Box
//

class TAboutDlg : public WDialog
{
public:
    TAboutDlg(HWND parent);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);
    ~TAboutDlg();

    void EvPaint();
    void SetupWindow();

protected:
    WStatic *title, *thanks, *desc, *version, *build;
};

#endif	//_ABOUT_H_
