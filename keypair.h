#ifndef _KEYPAIR_H_
#define _KEYPAIR_H_

#define IDTCB_FLAGSBASE 450

//#define IDD_KPENTRY 598
// parse key/values for " and replace with '
//
#define DM_BUTTON 1307
#define TM_BUTTON 1308
#define TA_BUTTON 1309
#define TW_BUTTON 1310

#define GAME_0 1311
#define GAME_1 1312
#define GAME_2 1313

#define IDCB_SKINUP 14075
#define IDCB_SKINDOWN 14076

#define LB1 100
#define LB2 101
#define LB3 102
#define LB4 103
#define LB5 104
#define LB6 105

#define LB7 106

class EntityView;

class TBSPListBox : public WList
{
public:
    TBSPListBox(HWND parent, int Id, int x, int y, int w, int h);
    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);

    void EvRButtonDown(UINT modKeys, LPPOINT point);

    void Clear();
    void Copy();
    void Paste();
    void Cut();
    void Last();
    void Edit();
    void Add();
    bool SetTempValues(LPPOINT pt);
    char saveKey[MAX_KEY];
    char saveValue[MAX_VALUE];
};

class EntityView : public TCWindow, public WindowPlacement
{
public:
    EntityView(HWND parent, char *title);
    ~EntityView();
    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);

    HWND WP_GetHwnd();
    const char *WP_WindowName();

    int BrowserGame;
    int FlagsX, FlagsY;
    void RemoveFlags();
    void SwitchMode(int);
    void Mode0();
    void Mode1();
    void Mode2();

    void SetupWindow();
    void EvCBNSelChange();
    //void EvKPListChange();
    void EvKPListDoubleClick();
    bool CanClose();

    WButton *CreateEntity;

    WCombo *entity_browser_i;
    WEdit *entity_comment_i;
    WButton *helpB;
    TBSPListBox *keypairview_i;
    WStatic *ts3;

    WCheck **flags_i;

    WButton *addp;
    WButton *delp;

    void ToggleModels();
    void ToggleTextures();
    void ToggleAnimate();
    void ToggleWireEnts();

    HFONT kpFont;
    HFONT kpFlagFont;

    void clickHelp();
    // TButtons for angle...
    void add0();
    void add45();
    void add90();
    void add135();
    void add180();
    void add225();
    void add270();
    void add315();
    void addUp();
    void addDown();

    WButton *color;
    void ClickColor();

    WButton *b0;
    WButton *b45;
    WButton *b90;
    WButton *b135;
    WButton *b180;
    WButton *b225;
    WButton *b270;
    WButton *b315;
    WButton *bUp;
    WButton *bDown;

    SIZE bounds;
    int lastSelected;

    int fillMatrix();
    char *spawnName();
    void calcViewSize();
    void drawSelf();
    void ReloadPropertyView();

    void skinUp();
    void skinDown();

    void initEntities();
    void newCurrentEntity(bool);
    void clearInputs();
    void selectClass(char *cl);
    void reloadEntityClasses();
    void selectEntity();
    void doubleClickEntity();
    void addPair();
    void delPair();
    void setAngle(int angle);
    void setFlags();
    void loadEntityComment(EntityClass *obj);
    void FixString(char *in, char *out);
};

#endif	//_KEYPAIR_H_
