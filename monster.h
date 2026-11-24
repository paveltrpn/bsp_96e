#ifndef _MONSTER_H_
#define _MONSTER_H_

#define IDM_MONSTERLIST 2000
#define IDM_MONSTERLEFT 2001
#define IDM_MONSTERRIGHT 2002
#define IDM_MONSTERDOWN 2003
#define IDM_MONSTERUP 2004
#define IDM_RESETANGLES 2005

class TMonsterWindow : public TCWindow
{
public:
    TMonsterWindow(HWND parent, char *title);
    ~TMonsterWindow();
    LRESULT WndProc(UINT msg,WPARAM wParam,LPARAM lParam);

    void SetAngles();
    // Rendering stuff...

    HDC         hmemdc;        // handle to device context for drawing
    HBITMAP		hmembitmap;  // memory bitmap of drawing
    void ConstructMemDC();
    void RemoveMemDC();
    SIZE lastSz;

    BITMAPINFOHEADER m_pBmInfoHeader;
    BITMAPINFO *m_pBmInfo;

    int rw, rh;
    float yawangle;
    float rollangle;
    float pitchangle;
    BYTE *mimagebuffer;
    float *mzbuffer;
    float distance;
    float s;
    void SetupWindow();

    WCombo *MonsterList;
    WButton *MonsterUp, *MonsterDown, *MonsterLeft, *MonsterRight;
    WButton *bReset;

    void ResetAngles();
    void LeftButton();
    void RightButton();
    void InButton();
    void OutButton();

    void PUp();
    void PDown();
    void RUp();
    void RDown();

    int curframe;
    void ClickMonsterUp();
    void ClickMonsterDown();
    float zoffset;

    HFONT MonsterFont;
    int mfSize;

    RECT MonsterRect;
    POINT startPt;
    POINT lastPt;
    int mode;

    void EvTimer(int timerId);
    void EvLButtonDown(UINT, LPPOINT);
    void EvLButtonUp(UINT, LPPOINT);
    void EvMouseMove(UINT, LPPOINT);
    void EvRButtonDown(UINT modKeys, LPPOINT point);
    void EvRButtonUp(UINT, LPPOINT);
    void EvSize(UINT sizeType, LPSIZE);
    void EvCBNSelChange();
    void RedrawContents();
    //void ClearContents();
};

#endif	//_MONSTER_H_
