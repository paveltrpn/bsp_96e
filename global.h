#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "bsppch.h"

typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;

//these defines are needed early
#define	MAX_KEY		64		// kp
#define	MAX_VALUE	1024	// kp
#define MAXTOKEN	MAX_VALUE
#define MAXCOMTOKEN	1024

#define MAX_MOUSE_EVENT_TYPES	10
#define MAX_EXPORTERS			16

#include "bsphdr.h"
#include "util.h"
#include "tokenizer.h"
#include "qtype.h"
#include "settings.h"
#include "wutil.h"
#include "window.h"
#include "wcontrol.h"
#include "wdialog.h"
#include "dialogs.h"
#include "setbrush.h"
#include "ent.h"
#include "map.h"
#include "eclass.h"
#include "clipper.h"
#include "group.h"
#include "formats.h"
#include "texture.h"
#include "xyview.h"
#include "editwin.h"
#include "keypair.h"
#include "about.h"
#include "bspwin.h"
#include "help.h"
#include "console.h"
#include "monster.h"
#include "surface.h"
#include "commands.h"
#include "menu.h"
#include "res\resource.h"

#pragma warning(disable:4996)

//BSP Window Messages
#define WM_SHORTCUT_CHANGED		WM_USER+1	//(0, 0)

//this is for win98 compat
#define OPENFILENAMESTRUCTSIZE OPENFILENAME_SIZE_VERSION_400 //sizeof(OPENFILENAME);

//util

extern char *RemoveTrailingSlash(char *dir);		//remove trailing slash from a directory
extern int VerifyPak(Path pakname);			// Determines if .pak file is a valid one.
extern void StripExtension (char *path);
extern char *COM_Parse (char *data);
extern "C" unsigned char *file_get_contents(Path filename, int *outlen=0);
extern DWORD expand_path(char *path, char *out, int outsize = MAX_PATH);
extern bool file_exists(char *);
extern bool Bsp_ShellExecute(HWND hwnd,char *op,Path file, char*param, char *start_dir, int show=SW_SHOW);
extern bool SetFocusUnderMouse(HWND hwnd);
extern void CenterWindow(HWND hwnd);
extern char *FixFloatString(char *fstr);		// truncate trailing zeroes, 1.0000 -> 1.0
extern void FixOverflow(float& number, float range);

#define LB_3DCOLORS 1
extern HBITMAP load_bitmap(Path filename, int flags);

#define LoopProblem(x)
#ifndef LoopProblem
extern void LoopProblem(char *);				// For finding infinite loops.
#endif
extern bool isPowerof2(int num);
extern int nextPowerof2(int num);
extern char *stristr(char *src, char *find);
extern int snprintf(char *buffer, size_t count, const char *format, ...);
extern void Msgbox(char *msg, ...);
extern void Caption(char *msg, ...);
extern bool GroupParse(Tokenizer *script);
extern void LogFileReset(Path logfile);
extern "C" void sysprintf(char *text, ...);
extern "C" void syserror(char *text, ...);
__declspec(noreturn)
extern void sysfatal(char *error, ...);
extern void ExtractFileBase (char *path, char *dest, int includeExtension);
extern int rint(float i);

extern char com_token[MAXCOMTOKEN];

typedef struct groupinfo_s
{
    char name[64];
    COLORREF color;
    int visible;
} groupinfo_t;

typedef struct camerainfo_s
{
    float x,y,z;
    int roll,pitch,yaw;
} camerainfo_t;

extern void Parse_NextLine(char **text);
extern void Parse_RTrim(char **text);
extern void Parse_LTrim(char **text);
extern void Parse_Trim(char **text);
extern void Parse_EatWS(char **text);
extern int Parse_Quoted(char **text, char *out, int outlen);
extern COLORREF Parse_Color(char *text);
extern int Parse_BSPGroups(char *text);
extern int Parse_BSPGroupInfo(char *text, groupinfo_t *gi);
extern int Parse_BSPFavorites(char *text);
extern int Parse_BSPNextFav(char **text, char *fav, int favlen);
extern int Parse_BSPCameras(char *text);
extern int Parse_BSPCamInfo(char *text, camerainfo_t *ci);
extern int Parse_BSPMapType(char *text);

//bsp
extern void MapChanged();
extern void BSPHelp(char *, char *);
extern void BSPHelpAdd(char *);
extern void BSPHelpAddV(char *, ...);
extern void ConScrollEnd();
extern void ConWrite(char *text);
extern void ConWriteV(char *text, ...);
extern void SetCaptionHeight();

extern TBSPApp *frame;						// BSP windows.  Main frame
extern TBSPWindow *client;					// MDI client.
extern TTextureWindow *texWindow;			// Texture browser
extern TXYViewWindow **xyWindow;			// 2D View
extern TEditWindow *editWindow;				// 3D View
extern EntityView *kpWindow;				// Entity Browser
extern TGroupWindow *groupWindow;			// Group Browser
extern THelpWindow *helpWindow;				// Help Window
extern TMonsterWindow *monsterWindow;		// Monster browser
extern TSurfaceWindow *surfaceWindow;		// Surface property window.
extern BSPConsole *wconsole;				// console window
extern PaletteWindow *palettewindow;		// palette viewer

extern map *copymap;
extern int KillAllWindows;
extern RECT EditClip;
extern RECT RenderClip;
extern SIZE EditSize;
extern SIZE RenderSize;
extern int CaptionHeight;
extern HFONT g_hCaptionFont;

extern int normalExit;
extern bool savetoini;

extern vec3_t *CP[2];							// array to hold clipped pts...
extern vec3_t SPoints[CPT];
extern POINT FSPoints[CPT];
extern vec3_t *tpts;
namespace Bsp
{
extern int cur, lst;
}
//extern vec3_t **pt;
extern SetBrush **besthits;
extern int numhits;


//bsp2
extern TBSPSplashWindow *splash;				// Splash screen
extern bool BSP_Active;							// If BSP is not in foreground, disable
extern bool KillAccels;							// disable KB accelerators
extern int LUMP_NAME_LENGTH;					// Q1 texture name len = 16, Q2 = 32
#define LUMP_NAME_LENGTH_MAX	48				// for automatic memory allocation

extern int PALSTEP;
extern int timeRender;
extern DWORD startTime;
extern DWORD endTime;
extern int cColorBits;
extern int cDepthBits;
extern int cTextureAlpha;
extern int singleBuffer;
extern WRebar *rebar;
extern WToolbar **cbars;
extern WStatus *status;
extern void status_ShowOpenMapsMenu();		// map gadget
extern KBSettings *kbset;
extern WImageList imagelist;

extern HINSTANCE hInstance;					// app hinstance
extern HMENU hMenu;							// main menu handle
extern HACCEL acceltable;					// accelerator table
#define ID_REBAR  102223

//status bar parts
#define STATUS_INFO   0
#define STATUS_MODE   1
#define STATUS_HEIGHT 2
#define STATUS_GROUP  3
#define STATUS_POS    4
#define STATUS_DELTA  5
#define STATUS_BOUND  6
#define STATUS_MAP    7

//texture
extern void UpdateTextureTransFlags(face_t *f);
extern int GetPaletteIndex(int pSize, unsigned char *pal, COLORREF color);
extern int BlendPal(int index1, int index2); //TESTING - blends two palette entries to the nearest pal color
extern TColorMap *texColorMap;
extern bool suppress_gets;
extern int GetTextureModeValue(char *md);


//bspui
extern void SplitBaseQuiet(char *path, char *basepath, char *name);
extern void Show_Frame(const char* str, bool notStatusOnly);
extern void SaveArrangement();
extern bool	ProcessMapFile(char *);
extern void	MergeMapFile(char *);
extern void	SaveMapFile(char *,int);

#define MAX_RECENT 16
extern char recent_files[MAX_RECENT][MAX_PATH];
extern void InitRecent();
extern void SaveRecent();
extern void AddRecent(char *filename);
extern int BuildRecentMenu(HMENU menu);

extern bool redraw_interrupted;				// User hit Esc during long screen update.


//bspui2
extern int plugPortal;
extern int leakPtType;
extern vec3_t *aLeakPoints; // leak points...
extern int numLeakPoints;
extern leakPortal_t *aLeakPortals; // leak portals...
extern int numLeakPortals;
extern vec3_t LeakPortalOrigin;
extern leakNode_t *aLeakNode;

//map
extern int snapToGrid(float);
extern int CountWads(char *);

extern char loadName[MAX_PATH];
extern bool loadMessageGiven;					// True if user has been warned of a loading error once

extern int ent_count;


//setbrush
/*#ifdef BSP_MEMMGR								// If defined, gather a pool of brushes
	extern void InitBrushManager();				//  at startup, otherwise allocation is dynamic
	extern void CloseBrushManager();
#endif*/
extern void VectorAdd (vec3_t va, vec3_t vb, vec3_t out);
extern void VectorSubtract (vec3_t va, vec3_t vb, vec3_t out);
extern void VectorCopy (vec3_t in, vec3_t out);
extern float DotProduct(vec3_t, vec3_t);
extern bool VectorCompare (vec3_t v1, vec3_t v2);
extern float VectorLength(vec3_t v);
extern void VectorNormalize (vec3_t v);
extern void VectorCalcNormal(vec3_t a,vec3_t b,vec3_t c, vec3_t out_normal);
extern void VectorScale (vec3_t v, float sc, vec3_t out);
extern void CrossProduct(vec3_t, vec3_t, vec3_t);
extern void SplitBase(char *path, char *basepath, char *name);
extern float TextureAxisFromPlane(plane_t *pln, float *xv, float *yv);
extern void TextureAxisFromPlaneEx(plane_t *pln, vec3_t out[3]);
extern winding_t *CopyWinding (winding_t *w);
extern winding_t *NewWinding (int points);
extern void carve(SetBrush **in);
extern void GetAxes(int ViewType, int *ua, int *va, int *na);

extern Entity *carve_in, *carve_out;
extern vec3_t vec3_origin;
extern map **map_i;
extern bool clipDraw;
extern int maxmulticontrolpoints;
extern int multicontrolpoints;
extern float **mcontrolpoints;
extern int numsb;
extern float lightaxis[3];

extern vec3_t	region_min, region_max;
extern int		numcontrolpoints;
extern float	*controlpoints[MAX_FACES*3];
extern vec3_t	sb_translate;
extern vec3_t	sb_mins, sb_maxs, sb_ctr;	//TODO: kill sb_mins/maxs
extern int		BBoxCount;
extern vec3_t	sel_x, sel_y, sel_z;
extern vec3_t 	select_min, select_max;
extern float	sb_floor_dir, sb_floor_dist;
extern Entity *sb_newowner;
extern bool select_deselect;

//matrix3
extern float angle(float x, float y);
extern void tran3(float tx, float ty, float tz, float A[][4]);
extern void scale3(float sx, float sy, float sz, float A[][4]);
extern void rot3(int m, float th, float A[][4]);
extern void mult3(float A[][4], float B[][4], float C[][4]);
extern void rotate3(float phi, float *b, float *d, float A[4][4]);
extern void transform(float *v, float A[4][4], float *w);
extern void transform2(float x, float y, float z, float A[4][4], float *w);
extern void findQRotate(float Q[4][4],float R[4][4],float *eye,float roll, float pitch, float yaw);
extern void MConcat(float in1[3][3], float in2[3][3], float out[3][3]);
extern void findQ(float Q[4][4],float R[4][4],float *eye,float roll, float pitch, float yaw);
extern void findQ2(float Q[4][4],float *eye,float *lookAt);
extern void findQ3(float Q[4][4],float *eye,float *lookAt);
extern void invert(float A[4][4],float AInv[4][4],int *sing);
extern int FastClipXY(int npts, LPRECT CR);
extern int Z_Clip(int npts, float zmin, float zmax);

//render
extern void REN_DrawFlatSpanOutline (int y,int selected);
extern void REN_DrawFlatSpanOutlineSin(int y,int selected);
extern void REN_DrawSpanOutline (int y, int selected);
extern void REN_DrawSpanOutlineSin(int y, int selected);
extern void REN_DrawSpanOutlineNoZ (int y, int selected);
extern void REN_DrawSpanOutlineNoZSin(int y, int selected);
extern void REN_CheckDistanceRasterizeFace(winding_t *w);
extern void REN_CheckClosestSpan (int y);
extern void REN_DrawSpanWire(int y, int selected);
extern void REN_DrawSpanWireHoriz(int y, int selected);
extern void REN_LineDraw(int XStart, int YStart, int XEnd, int YEnd, int selected);
extern void REN_LineDrawZ(float *p1, float *p2, int selected);
extern void REN_DrawSpanWireSin(int y, int selected);
extern void REN_DrawSpanWireHorizSin(int y, int selected);
extern void REN_LineDrawSin(int XStart, int YStart, int XEnd, int YEnd, int selected);
extern void REN_LineDrawZSin(float *p1, float *p2, int selected);
extern void REN_DrawWinding(winding_t *inw, int selected);
extern void REN_DrawLineWinding(winding_t *inw, int selected);
extern void REN_RenderWinding(winding_t *inw, int selected);
extern void REN_RasterizeLineWire (winding_t *w, int selected);
extern void REN_CopyBuffers (float *inz, float *outz, unsigned char *inp, unsigned char *outp);
extern void REN_DrawCameraPoint(float *pt,int selected);
extern bool REN_FindCameraPoint(float *pt, float fudge, int *plot_x, int *plot_y);
extern void REN_BeginCamera();
extern void REN_ClearBuffers();

extern bool REN_PointToScreen(float *pt, POINT *outpt);
extern void REN_Draw3DGrid();
extern void REN_DrawVec(float *pt, COLORREF color);
extern void REN_DrawCameraHandle(float *pt, COLORREF color);
extern void REN_DrawCameraHandleSin(float *pt, unsigned char r, unsigned char g, unsigned char b);
extern void REN_DrawXYFace(face_t *);
extern void REN_DrawCameraFace(face_t *, int);
extern void REN_FindClosestFace(face_t *idpol);
extern void REN_DrawModel(mdl_t *mdl, int selected, float Xrot, float Yrot, float Zrot,
                          int frame, int Xoffs, int Yoffs, float Zoffs);
extern void REN_RenderModel(mdl_t *mdl, int selected, float Xrot, float Yrot, float Zrot,
                            int frame, int Xoffs, int Yoffs, float Zoffs, int skinNumber);
extern void REN_DrawModelQ2(dmdl_t *mdl, int selected, float Xrot, float Yrot, float Zrot,
                            int frame, int Xoffs, int Yoffs, float Zoffs);
extern void REN_RenderModelQ2(dmdl_t *mdl, dskindata_t *skinTex, int selected, float Xrot, float Yrot, float Zrot,
                              int frame, int Xoffs, int Yoffs, float Zoffs, int skinNumber);

extern void XRotatePoint(float *in, float *out, float c, float s);
extern void YRotatePoint(float *in,float *out,float c, float s);
extern void ZRotatePoint(float *in, float *out, float c, float s);

extern vec3_t r_origin, r_matrix[3];
extern int t_coloroffset;
extern int t_width, t_height;
extern unsigned char *t_data;
extern float t_widthadd, t_heightadd;
extern int r_width, r_height;
extern float *r_zbuffer;
extern unsigned char *r_picbuffer;
extern vec5_t rightside, leftside, rightstep,leftstep;
extern face_t *r_face;
extern bool r_drawflat;
extern bool r_drawent;
extern bool r_noz;
extern bool r_drawwire;
extern pixel32_t r_flatcolor;
extern pixel32_t r_entitycolor;
extern plane_t frustum[5];

namespace Render
{
extern int clickX, clickY;
}
extern SetBrush *REN_bestBrush;
extern float REN_bestDistance;
extern SetBrush *REN_testBrush;
extern face_t *REN_bestFace;
extern face_t *REN_testFace;
extern SetBrush *REN_curBrush;

//xyview

extern bool glBspRotate;
extern vec3_t xy_eye;						// XY eye center point.
extern SetBrush *newbrush;
extern int numEventTypes;
extern int curEventType;

//editwin

extern void glSetFogRange(float fognear, float fogfar);
extern void glSetBackgroundColor(COLORREF color);
extern void glSetFogColor(COLORREF color);
extern void glFogOn();
extern void glFogOff();
extern void glShadingOn();
extern void glShadingOff();
extern void glLightAmbient(COLORREF color);
extern void glLightColor(COLORREF color);

extern map *glMap;
extern int glLastBoundIndex;
extern bool glSkyboxNotEmpty;
extern int brushCount;
extern vec3_t matrix[3];

extern char EventTypes3d[MAX_MOUSE_EVENT_TYPES] [32];   // Mouse customizations
extern int numEventTypes3d;
extern int curEventType3d;
extern SIZE fovsz;
extern bool mouse_is_navigating;

//clipper
extern bool splitSelect;


//hits
extern void RemoveHitBrush(SetBrush *b);
extern int GetHitIndex(SetBrush *b);
extern void ClearHits();
extern void NextHitBrush();
extern void HitRestore();
extern void SelectHitBrush(int index);
extern void SelectAllHit();
extern void PreviousHitBrush();
extern void AddHitBrush(SetBrush *b);

extern int *hitSelected;						// Trace results for clicks in 2d view.


//eclass
extern EntityClassList *entity_classes_i;		// Entity browser.


//ent
extern bool wasBad;


//bspresp
extern char * GetVKeyName(int id);
extern int GetVKey(char *vkey);


//glutil
extern void glShadingOn();
extern void glShadingOff();
extern void glAddTexture(int name,int width,int height,GLuint* pic,int bits=0,int wrap=GL_REPEAT);
extern void glReloadBSPSettings();
extern void glBeginAddToSkybox();
extern void glEndAddToSkybox();
extern void glRenderSkybox();
extern void glUnloadSkybox();
extern void glReloadSkybox();

//lua
//extern void LUA_Create();
//extern void LUA_Destroy();
//extern void LUA_Exec(char *lua_code);

//angel
//extern void AS_Create();
//extern void AS_Exec(char *scriptcode);
//extern void AS_Destroy();

//squirrel!
extern void SQ_Create();
extern void SQ_Destroy();
extern void SQ_Exec(char *scriptcode);

//global const

//max dimensions for auto rendering size
const int AUTO_MAX_W = 1280;
const int AUTO_MAX_H = 1024;

#define BAD_SCALE 0.0001

#define DEG2RAD(x) (((float)(x))*M_PI/180.0f)
#define RAD2DEG(x) ((int)(180.0f*((float)(x))/M_PI))

//debug: helps show how many times a function is called
#define FNHIT() do { static int _TEMPVAR=0; Caption("%d",++_TEMPVAR); } while(0)

#define MAKEPOINT(pt, dword) POINT pt = { (short)LOWORD(dword), (short)HIWORD(dword) }

#define SWAPBYTES(x) (((x>>24) & 0xFF) | ((x & 0xFF)<<24) | ((x&0xFF00)<<8) | ((x>>8) & 0xFF00))

//strncpy with automatic size param
//debug: #define STRNCPY(dest,src) do { if(sizeof(dest)<16)sysfatal("bad STRNCPY Line:%i, "__FILE__,__LINE__); strncpy(dest,src,sizeof(dest)); } while(0)
#define STRNCPY(dest,src) strncpy(dest,src,sizeof(dest))

//automatically expand RECT for BitBlt
#define BitBltRect(dst,rc,src,x,y,op) BitBlt(dst,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,src,x,y,op)

//
// ADD_TO_LIST_SORTED
//
//	adds an item to a linked list sorting on T->name. uses
//  compare_strings_int_parts() function to sort numbered items correctly.
//  list T needs to impl "char *name" member, and the supplied "T *next"
//  member.
//
// params
//	T				type of list
//	item			name of item to add
//	head			head pointer of list
//	next_member		name of "T->next" member
#define ADD_TO_LIST_SORTED(T, item, head, next_member)	do {	                        \
	item->next_member = 0;	                                                            \
	T *ATLS_iter = head;	                                                            \
	if(!ATLS_iter) {	/* add first item to list */	                                \
		head = item;	                                                                \
	} else {		/* add item to list with one or more items */	                    \
		T **ATLS_previns = &head;	/* insert point, start at list head */	            \
		for(; ATLS_iter; ATLS_iter = ATLS_iter->next_member) {	                        \
            int compare_strings_int_parts(char *, char *);                              \
			if(compare_strings_int_parts(item->name, ATLS_iter->name) <= 0) {	        \
				item->next_member = ATLS_iter;	/* next item is current iter item */	\
				(*ATLS_previns) = item;		/* previous item points to new item */	    \
				break;	                                                                \
			}	                                                                        \
			if(!ATLS_iter->next_member) {	/* before next... */	                    \
				ATLS_iter->next_member = item;	                                        \
				break;	                                                                \
			}	                                                                        \
			ATLS_previns = &ATLS_iter->next_member;	/* remember insert point */	        \
		}	                                                                            \
	}	                                                                                \
} while(0)


#endif	//_GLOBAL_H_
