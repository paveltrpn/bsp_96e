/* BSPHDR.h Main Header for BSP */
#ifndef	_BSPHDR_H_
#define _BSPHDR_H_

//====================  BSP Defaults    ======================
#define BSP_VER_MAJOR	0            	// Current Version
#define BSP_VER_MINOR	96            	// Current Version
#define BSP_REV         "e"				// version letter, a, b
#define MAX_CAMERAS 	5             	// Number of Cameras
#define MAX_WADS    	64  			// Per .map file

#define SM_REGULAR 		0				// Vertex Selection Modes
#define SM_VERTEX 		1

#define MAX_XY 			3              // Maximum number of 2d views
#define CPT 			256			   // Number of temporary data points
//  in data arrays of points (used
//  in clipping routines).


//=====================  BSP Tolerances ======================
#define ON_EPSILON 		0.1f
#define F_EPSILON 		0.1f
#define epsilon 		0.000001f

//=====================  BSP Make Stairs======================
#define STAIR_REGULAR 	0
#define STAIR_SPIRAL 	1
#define STAIR_BLOCK 	2

#define MAX_HITS 		10   		   // Number of hit brushes to track

//=====================  BSP Screen Dimensions  ==============
#define ORIG_HORIZ 		(400)             // Default window size for BSP Windows
#define ORIG_VERT 		(300)
#define ORIG_XMAX 		((ORIG_HORIZ)-1)
#define ORIG_XMIN 		0
#define ORIG_YMAX 		((ORIG_VERT)-1)
#define ORIG_YMIN 		0
#define ORIG_ZMIN 		-1.0				// Near clipping plane default
#define ORIG_ZMAX 		-8192.0				// Far clipping plane default
#define ORIG_EDIT_H 	(ORIG_HORIZ)		// 3d Preview window default sizes
#define ORIG_EDIT_V 	(ORIG_VERT)			//

#define SELECT_DELTA	8					// Mouse click must be within this
//  distance to select item

#define STEPVAL 		64					// Default movement step size

#define HELP_FILE 		"help\\bsp.chm"			// BSP Help File

#define BSP_LOG			"bsp.log"			// session logfile, relative path
#define MAX_MAPS 		10					// FIXME, remove this limitation
#define MAX_SAVE_WIN 	4					// Number of sets of window positions

//#define BOGUS_RANGE 	30000				//(REPLACED WITH TMAX) A number well outside of the Quake genre +/- 4096.0f range

#define MAX_NUM 		32767				// Maximum 16 Bit Integer

#define MAP_MINZ 		(-4096 + 64) 		// Min/Max for making tall brushes
#define MAP_MAXZ 		( 4096 - 64)		//  slightly adjusted

#define MAX_CLASSES 	8192				// Maximum distinct entity classes

//========================= BSP Enumeration Types ==================
#define XYDRAW							 0
#define XYDRAWCUR						 1
#define CAMERARENDER   					 2
#define GLCAMERARENDERSELFTEXTURE 		 3
#define GLCAMERARENDERSELFFIXED 		 4
#define GLCAMERARENDERSELFOUTLINE 		 5
#define GLCAMERARENDERSELFSELECTED 		 6
#define GLCAMERARENDERSELFNOTEXTURE 	 7
#define CLOSESTRENDER     		  		 8
#define GLCAMERARENDERSELFWIREFRAME 	 9
#define GLCAMERARENDERSELFFLAT			10

#define DRAG_DELTA 		6					// Distance mouse must be moved in
//  preview type dialogs before the
//  drag will cause a roll/pitch/yaw
//  change.


//=======================  BSP Texture Window ======================
#define AREA_TEX 				0              // Mouse is in main texture area
#define AREA_FAV 				1              // Mouse is in favorites area
#define AREA_FRQ 				2              // Mouse is in frequents area

#define O_UX 					0					// Timer callback values for various
#define O_UY 					1              //  derived buttons.  U = UP
#define O_DX 					2              //  D = Down, X = Shift S, Y = Shift T
#define O_DY 					3              //  R = Rotate, SX = Scale S, SY = Scale T
#define O_UR 					4
#define O_DR 					5
#define O_USX 					6
#define O_DSX 					7
#define O_USY 					8
#define O_DSY 					9

//===========================  BSP Preview Dialogs  ==================
#define STAIR_PREV_LEFT 	255         // Stair dialog
#define STAIR_PREV_TOP 		 15
#define STAIR_W 			245
#define STAIR_H 			245

#define ARCH_PREV_LEFT 		 75			// Arch dialog
#define ARCH_PREV_TOP 		125
#define ARCH_W 				400
#define ARCH_H 				220

#define WEDGE_PREV_LEFT 	 75			// Wedge Dialog
#define WEDGE_PREV_TOP 		125
#define WEDGE_W 			400
#define WEDGE_H 			220

#define STEPS 				 72.0f		// Preview dialog steps per turn

#define DRAG_ROTATE 		  1			// Mouse drag rotates object in preview
#define DRAG_DISTANCE 		  2			// Mouse drag (w/ shift key) moves viewpoint closer or farther.

//===========================  BSP Enumerations  ======================
namespace Drawmode
{
enum drawmode
{
    wire,							// Wire Frame Preview
    flat,             				// Flat Shaded
    texture                        // Textured Preview
};
}
namespace Runmode
{
enum runmodes
{
    no_run,
    run
};
}

enum MapViewType  						// 2d Window View Types
{
    Type_XY,
    Type_YZ,
    Type_XZ
};

// DIB Info Header Strucure
typedef struct BmInfo
{
    BITMAPINFOHEADER  Header;
    RGBQUAD aColors[256];  // Assume 256 color palette
} BmInfo;

#endif	//_BSPHDR_H_
