#ifndef _COMMANDS_H_
#define _COMMANDS_H_

typedef void (*CM_FUNC) (void);
typedef void (*CE_FUNC) (int);

////////////////////////////////////////////////////////////////////////////////
// new dynamic command system
////////////////////////////////////////////////////////////////////////////////
#define CCMD_HASH_SIZE 500
struct ccmd_group_s;

typedef struct ccmd_s
{
    int id;
    char name[30];
    CM_FUNC cm_func;		//built-in command
    CE_FUNC ce_func;		// ...
    char *user_func;		//point to user script if cm/ce null
    char *menu_text;		//label to use on menus
    char *info_text;		//description for status bar
    int image;				//index for imagelist
    //links
    ccmd_group_s *group;
    ccmd_s *group_next;
    ccmd_s *hash_name_next;
    ccmd_s *hash_id_next;
    ccmd_s *ce_next;
    ccmd_s *next;
} ccmd_t;

typedef struct ccmd_group_s
{
    char name[32];
    ccmd_s *cmd_next;
    ccmd_group_s *next;
} ccmd_group_t;


extern ccmd_t *cmd_head;
extern ccmd_group_t *cmd_groups;		//groups

extern void Ccmd_Initialize();
extern void Ccmd_Execute(char *name);
extern void Ccmd_Execute(int id);
extern void Ccmd_UpdateEnablers();
extern bool Ccmd_LoadCommandDef(char *file);
extern void Ccmd_Unload();
extern ccmd_t *Ccmd_FindCmd(char *name);
extern ccmd_t *Ccmd_FindCmd(int id);
extern void Ccmd_FillCombo(WCombo *combo);
extern int Ccmd_GetId(char *name);
extern char *Ccmd_GetName(int id);
extern char *Ccmd_GetInfoText(int id);
extern int Ccmd_Complete(char *prefix, char *out, int outlen, list<char*> *matches);
extern bool Ccmd_Register(char *name, char *scriptcode, CE_FUNC ce_func);	//script command
extern ccmd_group_t * Ccmd_SetCurrentGroup(char *name);



//===========================================================================

extern bool c_noupdate;						// UpdateEnablers can be skipped if set


//
// Main Commands
//


extern void CmNoop();

extern void CmListSkybox();
extern void CmMakeClicked();
extern void CmClipAutoFlip();
extern void CmClippersStay();
extern void CmClipMode0();
extern void CmClipMode1();
extern void CmLookAtSel();
extern void CmMixed();
extern void CmTexSearch();
extern void CmAddDetail();
extern void CmRemoveDetail();
extern void CmShowPalette();

extern void SelectMap1();
extern void SelectMap2();
extern void SelectMap3();
extern void SelectMap4();
extern void SelectMap5();
extern void SelectMap6();
extern void SelectMap7();
extern void SelectMap8();
extern void SelectMap9();
extern void SelectMap10();

extern void CmRecent0();
extern void CmRecent1();
extern void CmRecent2();
extern void CmRecent3();
extern void CmRecent4();
extern void CmRecent5();
extern void CmRecent6();
extern void CmRecent7();
extern void CmRecent8();
extern void CmRecent9();
extern void CmRecent10();
extern void CmRecent11();
extern void CmRecent12();
extern void CmRecent13();
extern void CmRecent14();
extern void CmRecent15();

extern void CmJumpXY();
extern void CmJump3D();

extern void CmLiftHeights();
extern void CmSetHeights();

extern void SelectMap(int mapnum);

extern void CmTUnload();
extern void CmTLoadWad();
extern void CmTLoadDir();
extern void CmTLoadUsed();
extern void CmTLoadWal();
extern void CmTLoadPak();
extern void CmTLoadPakQ2();

extern void CmResetRun();

extern void CmStopLeaks();
extern void CmStopPortals();

extern void CmImpulse(int f, int s, int v);

extern void CmRunMode();
extern void CmToggleFlymode();
//   extern void CmMouse();

extern void CmNoRegion();
extern void CmRegionMode();
extern void CmRegion();
extern void CmRegionPartial();
extern void CmModels();
extern void CmSolidModels();
extern void CmGlShading();
extern void CmOutlineUseGroup();
extern void CmOutlineUseFlat();
extern void CmRegionExport();

extern void CmGroupMode();

extern void CmExit();
extern void CmExitFinal();
extern void RecalcFiltered();

extern void CmSave();
extern void CmFaceTexture();
extern void CmBrushTexture();
extern void CmBrushContents(int contents, int removeContents);
extern void CmBrushFlags(int flags, int removeFlags);

extern void CmSelectValue();
//extern void CmHelpWindow();
extern void CmMonsterWindow();
extern void CmBrushWindow();
extern void CmUndo();
extern void CmUndoSave();

extern void CmLock();
extern void CmUnLock();
extern void CmFindBrush();
extern void CmMapInfo();
extern void CmExportSelected();

extern void CmBar1();
extern void CmBar2();
extern void CmToggleGridStyle();
extern void CmToggleKnobs();
extern void CmNextCamera();
extern void CmPrevCamera();

extern void CmSetScale();

extern void CmShowLeaks();
extern void CmShowPortals();
extern void CmLeakTest();
extern void CmMergeBrushes();

extern void CmMergeIn();
extern void CmMergeOut();
extern void CmMergeClipboard();

extern void CmRevert();
extern void CmSelectByTexture();
extern void CmHitUp();
extern void CmHitDown();

extern void CmReplace();
extern void CmNextFace();
extern void CmPrevFace();
extern void CmTT();
extern void CmOmitLights();
extern void CmOmitUnselected();
extern void CmOmitEntities();
extern void CmOmitWorld();
extern void CmOmitWater();
extern void CmOmitClip();
extern void CmOmitSky();
extern void CmOmitTarget();
extern void CmOmitDetail();
extern void CmOmitHint();

extern void CmSaveWindowArrangement();
extern void CmSwitchMapView();
extern void CmCenter();

extern void CmGroup(); // dialog

extern void CmRestoreEntity(); // dialog
extern void CmRestoreTexture(); // dialog
extern void CmRestore3D(); // dialog
extern void CmToggleConsole();	//dialog

extern void OpenMapFile(char *filename);
extern void CmNew();
extern void CmOpen();
extern void CmRot1();
extern void CmRot5();
extern void CmRot10();
extern void CmRot15();
extern void CmRot30();
extern void CmRot45();

extern void CmGrid3D();
extern void CmAxes3D();

extern void CmEditCopy();
extern void CmEditClone();
extern void CmEditPaste();
extern void CmCopyMove();
extern void CmJumpSelXY();
extern void CmDelete();
extern void CmFlipClipper();
extern void CmCarve();
extern void CmSplit();
extern void CmClipCurFace();
extern void CmEditMakeStairs();
extern void CmSelectTotal();
extern void CmSelectPartial();
extern void CmMakeBrush();
extern void CmMakeRoom();
extern void CmMakeArch();
extern void CmMakeWedge();
extern void CmMakeSphere();
extern void CmMakeCylinder();
extern void CmMakeNSided();
extern void CmMakePyramid();
extern void CmMakeLights();
extern void CmRotateBrush();
extern void CmExtrude();
extern void CmExtrudeFace();
extern void CmEditScale();
extern void CmClipMoveUp();
extern void CmClipMoveDown();
extern void CmSnap();
extern void CmSnapPlanes();
extern void CmSnapBack();

extern void CmFlipX();
extern void CmFlipY();
extern void CmFlipZ();
extern void CmRotateX();
extern void CmRotateY();
extern void CmRotateZ();

extern void CmMakeEntity();
extern void CmReload();

extern void CmArrangeIcons();
extern void CmWindowPosition(); // reposition windows...
extern void CmTileChildren(); // reposition windows...
extern void CmCascadeChildren(); // reposition windows...

extern void CmModeWire();

extern void CmModeFlat();

extern void CmModeTexture();

extern void CmTallBrush();
extern void CmShortBrush();
extern void CmHeightBrush();
extern void CmSelectAll();
extern void CmDeselectAll();
extern void CmSubtract();

extern void CmFileSaveSel();
extern void CmLoadBrushes();
//	extern void CmNew();
//	extern void CmOpen();
extern void CmClose();
extern void CmSaveAs();
extern void CmReset();
extern void CmSetColors();
extern void CmSetGame();
extern void CmMenuRefresh();
extern void CmKBSettings();
extern void CmBspSettings();
extern void CmWriteConfig();
extern void CmWriteGameConfig();
extern void CmShowKeyMappings();
extern void CmNextMap();
extern void CmPreviousMap();
extern void CmAbout();

extern void CmGridMode();

extern void CmModePoint();
extern void CmModeEdge();
extern void CmModePolygon();
extern void CmModeGroup();

extern void XYZMove(int axis,int distance,int currentMode);
extern void CmXUp();
extern void CmYUp();
extern void CmZUp();
extern void CmXDown();
extern void CmYDown();
extern void CmZDown();

extern void CmJump();

extern void CmResetAngles();

extern void CmFloorUp();
extern void CmFloorDown();

extern void CmAutoSave();
extern void CmToggleOutline();
extern void CmToggleNames();
extern void CmToggleConnections();
extern void CmToggleHits();
extern void CmToggleCoordinates();

extern void CmExportQBSP();   // Superfast...
extern void CmExportFull();
extern void CmExportFast();
extern void CmExportNoVis();
extern void CmExportReLight();
extern void CmExportLeakTest();

extern void CmHelp();

extern void RenModeFunc(int width, int height);
extern void R1();
extern void R2();
extern void R3();
extern void R4();
extern void R5();
extern void R6();
extern void R7();
extern void R8();
extern void R9();

extern void CmTrans0();
extern void CmTrans1();
extern void CmTrans2();
extern void CmTransGL();

extern void SelectClipPointFunc(int point);
extern void CmClip1();
extern void CmClip2();
extern void CmClip3();

extern void CmSetBrushDepth();
extern void CmShowFrame();

extern void CmHitFunc(int num);
extern void CmHit0();
extern void CmHit1();
extern void CmHit2();
extern void CmHit3();
extern void CmHit4();
extern void CmHit5();
extern void CmHit6();
extern void CmHit7();
extern void CmHit8();
extern void CmHit9();

extern void CmSelAllHit();

extern void Cm180();

extern void CmMoveUp();
extern void CmMoveDown();

extern void CmForward();
extern void CmBack();

extern void CmMF();
extern void CmMB();

extern void CmRight();
extern void CmLeft();

extern void CmPitchDown();
extern void CmPitchUp();

extern void CmMagDown();
extern void CmMagUp();

extern void CmPPDDown();
extern void CmPPDUp();

extern void CmStepDown();
extern void CmStepUp();

extern void CmClipIn();
extern void CmClipOut();

extern void CmToggleMode();

extern void CmKP2();
extern void CmKP4();
extern void CmKP6();
extern void CmKP8();

extern void CmMinus();
extern void CmPlus();

extern void CmMoveLeft();
extern void CmMoveRight();

extern void ScaleUp();
extern void ScaleDown();
extern void CmScaleUp();
extern void CmScaleDown();

extern void Scale1_64();
extern void Scale1_32();
extern void Scale1_16();
extern void Scale1_8();
extern void Scale1_4();
extern void Scale1_2();
extern void Scale3_4();
extern void Scale1();
extern void Scale2();
extern void Scale4();
extern void Scale8();
extern void Scale16();

extern void GridUp();
extern void GridDown();

extern void Grid1();
extern void Grid2();
extern void Grid4();
extern void Grid8();
extern void Grid16();
extern void Grid32();
extern void Grid64();
extern void Grid128();
extern void Grid256();

// window procedures...
extern void WUp();
extern void WDown();
extern void W1();
extern void W2();
extern void W3();
extern void W4();


//
// Enablers
//

extern void CeClipAutoFlip(int id);
extern void CeClippersStay(int id);
extern void CeClipMode0(int id);
extern void CeClipMode1(int id);
extern void CeLookAtSel(int id);
extern void CeFileNew(int id);
extern void CeFileOpen(int id);
extern void CeNeedMap(int id);
extern void CeNeedNoMap(int id);
extern void CeNeedSelection(int id);
extern void CePaste(int id);
extern void CeBrushWindow(int id);
extern void CeSnapBack(int id);

extern void CeRunMode(int id);
extern void CeUndo(int id);
extern void CeToggleKnobs(int id);
extern void CeToggleFlymode(int id);
extern void CeModels(int id);
extern void CeSolidModels(int id);
extern void CeGlShading(int id);
extern void CeOutlineUseGroup(int id);
extern void CeOutlineUseFlat(int id);
extern void CeBar1(int id);
extern void CeBar2(int id);
extern void CeRevert(int id);
extern void CeExportSelected(int id);
extern void CeNeedLeaks(int id);
extern void CeNeedPortals(int id);
extern void CeOmitDetail(int id);
extern void CeOmitHint(int id);
extern void CeOmitSky(int id);
extern void CeOmitTarget(int id);
extern void CeOmitLights(int id);
extern void CeOmitUnselected(int id);
extern void CeOmitEntities(int id);
extern void CeOmitWorld(int id);
extern void CeOmitWater(int id);
extern void CeOmitClip(int id);
extern void CeGroupMode(int id);
extern void CeRegionMode(int id);
extern void CeNoRegion(int id);
extern void CeRegionExport(int id);
extern void CeModeWire(int id);
extern void CeModeFlat(int id);
extern void CeModeTexture(int id);
extern void CeGridMode(int id);
extern void CeToggleGridStyle(int id);
extern void CeToggleOutline(int id);
extern void CeToggleNames(int id);
extern void CeToggleConnections(int id);
extern void CeToggleHits(int id);
extern void CeToggleCoordinates(int id);
extern void CeScale1_64(int id);
extern void CeScale1_32(int id);
extern void CeScale1_16(int id);
extern void CeScale1_8(int id);
extern void CeScale1_4(int id);
extern void CeScale1_2(int id);
extern void CeScale3_4(int id);
extern void CeScale1(int id);
extern void CeScale2(int id);
extern void CeScale4(int id);
extern void CeScale8(int id);
extern void CeScale16(int id);
extern void CeGrid1(int id);
extern void CeGrid2(int id);
extern void CeGrid4(int id);
extern void CeGrid8(int id);
extern void CeGrid16(int id);
extern void CeGrid32(int id);
extern void CeGrid64(int id);
extern void CeGrid128(int id);
extern void CeGrid256(int id);
extern void CeW1(int id);
extern void CeW2(int id);
extern void CeW3(int id);
extern void CeW4(int id);
extern void CeRot1(int id);
extern void CeRot5(int id);
extern void CeRot10(int id);
extern void CeRot15(int id);
extern void CeRot30(int id);
extern void CeRot45(int id);
extern void CeGrid3D(int id);
extern void CeAxes3D(int id);
extern void CeTrans0(int id);
extern void CeTrans1(int id);
extern void CeTrans2(int id);
extern void CeTransGL(int id);


// EXPORTERs for .bat files
//
extern void	RemoveExporterItemsFromMenu(HMENU hMenu);
extern int	BuildExporterMenu(HMENU hMenu);
extern int	GetMenuPos(HMENU hMenu, UINT id);
extern void RemoveExportNames();

extern void Exporter(int index);
extern void Exporter0();
extern void Exporter1();
extern void Exporter2();
extern void Exporter3();
extern void Exporter4();
extern void Exporter5();
extern void Exporter6();
extern void Exporter7();
extern void Exporter8();
extern void Exporter9();
extern void Exporter10();
extern void Exporter11();
extern void Exporter12();
extern void Exporter13();
extern void Exporter14();
extern void Exporter15();

#endif	//_COMMANDS_H_
