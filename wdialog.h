#ifndef _WDIALOG_H_
#define _WDIALOG_H_

// dialog


/*

	this dialog class cant inherit from Window classes because they work completely differently.
	but it has to be possible somehow right? modal dialogs wont return a hwnd, let alone allow
	you to process any code while the dialog is up. modeless require IsDialogMessage handling
	to be set up in message pump. can WDialog inherit from Windows somehow? Maybe make a new
	class containing common members and have both classes inherit from that...

*/

class WDialog
{
public:
    WDialog(HWND parent, int dlgId);
    virtual ~WDialog();
    static  WPARAM CALLBACK DialogProcRouter(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

    HWND hwnd, parent;
    int dlgId;

    INT_PTR Execute();
    HWND CreateModeless();
};

//
// InputDialog
//
class InputDialog : public WDialog
{
public:
    InputDialog(HWND parent,char *title,char*prompt,char*buffer,int bufsize);
    INT_PTR DialogProc(UINT msg, WPARAM wParam, LPARAM lParam);

    char *title, *prompt, *buffer;
    int bufsize;
};

#endif	//_WDIALOG_H_

