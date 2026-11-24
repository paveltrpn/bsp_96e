#ifndef _QTYPE_H_
#define _QTYPE_H_

#define TMAX 32765
#ifndef strcasecmp
#define strcasecmp strcmpi
#endif
#define TYP_MIPTEX 68
#define  LUMPTYPE_QPIC      'B'
// the old miptex format
#define  LUMPTYPE_MIPTEX    'C'
#define  LUMPTYPE_RAW       'D'
#define  LUMPTYPE_COLORMAP2 'E'
#define  LUMPTYPE_QPICNEW     'F'
// the new miptex format
#define  LUMPTYPE_MIPTEXNEW  'G'

typedef float vec_t;
typedef float vec2_t[2];
typedef float vec3_t[3];

///////////////////////////////////////////////
// Quake 2 Model Stuff                       //
///////////////////////////////////////////////
#define IDALIASHEADER  (('2'<<24)+('P'<<16)+('D'<<8)+'I')
#define ALIAS_VERSION  8
#define MAX_TRIANGLES  4096
#define MAX_VERTS		  2048
#define MAX_FRAMES	  512
#define MAX_MD2SKINS	  32
#define MAX_SKINNAME	  64

typedef struct dstvert_s
{
    short	s;
    short	t;
} dstvert_t;

typedef struct dtriangle_s
{
    short	index_xyz[3];
    short	index_st[3];
} dtriangle_t;

typedef struct dtrivertx_s
{
    byte	v[3];	  // scaled byte to fit in frame mins/maxs
    byte	lightnormalindex;
} dtrivertx_t;

typedef struct daliasframe_s
{
    vec3_t scale; // multiply byte verts by this
    vec3_t origin; // then add this
    char		   name[16];	   // frame name from grabbing
    dtrivertx_t	verts[1];	   // variable sized
} daliasframe_t;

// the glcmd format:
// a positive integer starts a tristrip command, followed by that many
// vertex structures.
// a negative integer starts a trifan command, followed by -x vertexes
// a zero indicates the end of the command list.
// a vertex consists of a floating point s, a floating point t,
// and an integer vertex index.

typedef struct dmdlheader_s
{
    int ident;
    int version;
    int skinwidth;
    int skinheight;
    int framesize;		// byte size of each frame
    int num_skins;
    int num_xyz;
    int num_st;			// greater than num_xyz for seams
    int num_tris;
    int num_glcmds;	// dwords in strip/fan command list
    int num_frames;
    int ofs_skins;		// each skin is a MAX_SKINNAME string
    int ofs_st;			// byte offset from start for stverts
    int ofs_tris;		// offset for dtriangles
    int ofs_frames;	// offset for first frame
    int ofs_glcmds;
    int ofs_end;		// end of file
} dmdlheader_t;

typedef struct dmdl_s
{
    char        **skinnames;
    int        num_skins;
    int        num_glcmds;
    int        *glcmds;
    int        numframes;
    daliasframe_t *frames;
    int        framesize;
    int        skinwidth;
    int        skinheight;
    float       s_scale;
    float       t_scale;
    float       skin_screen_offset_x;
    float       skin_screen_offset_y;
    int			gltex;
    int			gl_list;
} dmdl_t;

////////////////////////////////////////////////////
// Quake 1 Model Stuff                            //
////////////////////////////////////////////////////
typedef struct mdlheader_s
{
    int   id;         // 0x4F504449 = "IDPO"
    int   version;    // Version = 3
    vec3_t scale;      // Scale factor, for x,y,z
    vec3_t origin;     // Model origin: point for x=0,y=0,z=0
    float  radius;     // Model radius, maybe useless now.
    vec3_t eye;        // Position of the eyes
    int   numskins ;  // the number of skin textures
    int   skinwidth;  // Width of skin texture
    //  must be multiple of 8
    int   skinheight; // Height of skin texture
    //  must be multiple of 8
    int   numverts;   // Number of vertices
    int   numtris;    // Number of triangles surfaces
    int   numframes;  // Number of frames
    int   sync;       // Sync type
    int   flags;      // duh
    float  size;       // dunno. pixels / triangle ??
} mdlheader_t;

typedef struct skin_s
{
    int   type;            // Group type(1) or regular(0)
    int   numgroupskins;   // only used on group skins
    float  *interval;       // only used on group skins
    unsigned char **skin;   // the skin picture, length = skinwidth*skinheight
} skin_t;                 // numskins pictures

typedef struct stvert_s
{
    int onseam;      // 0 or 1
    int s;           // position, horizontally
    //  in range [0,skinwidth]
    int t;           // position, vertically
    //  in range [0,skinheight]
} stvert_t;

typedef struct itriangle_s
{
    int facesfront;  // boolean
    int vertices[3]; // Index of 3 triangle vertices
    //  in range [0,numverts]
} itriangle_t;

typedef struct trivertx_s
{
    unsigned char x;
    unsigned char y;                    // X,Y,Z coordinate, packed on 0-255
    unsigned char z;
    unsigned char lightnormalindex;     // index of the vertex normal
} trivertx_t;

typedef struct frameinfo_s
{
    trivertx_t min;              //
    trivertx_t max;              //
    char name[16];               // name of frame
} frameinfo_t;

typedef struct frame_s
{
    int type;                    // single frame(0) or group frame(1)
    int numgroupframes;          // used for groups only, duh
    trivertx_t groupmin;          // minimum values of X,Y,Z for group
    trivertx_t groupmax;          // maximum values of X,Y,Z for group
    frameinfo_t *info;            // info on each frame
    float *interval;              // interval crap, dunno what it is
    trivertx_t **data;            // length of this array is numverts*sizeof(trivertx_t)
} frame_t;

typedef struct mdl_s
{
    char        classname[40];
    vec3_t      scale;
    vec3_t      origin;
    int        numskins;
    skin_t      *skins;
    int        numverts;
    stvert_t    *verts;
    int        numtris;
    itriangle_t *triangles;
    int        numframes;
    frame_t     *frames;
    int        skinwidth;
    int        skinheight;
    int        scaledwidth;
    int        scaledheight;
    int		gltex;
    int		gl_list;
} mdl_t;

typedef struct pakheader_s
{
    unsigned char magic[4];  // Name of the new WAD format = "PACK"
    int          diroffset; // Position of WAD directory from start of file
    int          dirsize;   // Number of entries * 0x40 (64 char)
} pakheader_t;

typedef struct pakentry_s
{
    unsigned char filename[0x38]; // Name of the file, Unix style, with extension,
    // 56 chars, padded with '\0'.
    int          offset;         // Position of the entry in PACK file
    int          size;           // Size of the entry in PACK file
} pakentry_t;

typedef struct dentry_s
{
    int offset;
    int size;
} dentry_t;

typedef struct dheader_s
{
    int version;
    dentry_t entities;
    dentry_t planes;
    dentry_t miptex;
    dentry_t vertices;
    dentry_t visilist;
    dentry_t nodes;
    dentry_t surfaces;
    dentry_t lightmaps;
    dentry_t boundnodes;
    dentry_t leaves;
    dentry_t lstsurf;
    dentry_t edges;
    dentry_t lstedges;
    dentry_t hulls;
} dheader_t;

typedef struct boundbox_s
{
    vec3_t minpt;
    vec3_t maxpt;
} boundbox_t;

typedef struct dhull_s
{
    boundbox_t bound;
    int zero[3];
    int node;
    int boundnode;
    int numleafs;
    int firstsurface;
    int numsurfaces;
} dhull_t;

typedef struct vertex_s
{
    float x;
    float y;
    float z;
} vertex_t;

typedef struct edge_s
{
    unsigned short startvertex;
    unsigned short endvertex;
} edge_t;

typedef struct surface_s
{
    unsigned short planenum;
    unsigned short side;
    unsigned char texnum;
    unsigned char sofs;
    unsigned char tofs;
    unsigned char flips;
    int firstedge;
    int numedge;
    unsigned char typelight;
    unsigned char baselight;
    unsigned short unknown1;
    int lightmap;
} surface_t;

typedef struct mipheader_s
{
    int numtex;
    int *offset;
} mipheader_t;

typedef struct miptex_s
{
    char		name[16];
    uintptr_t width, height;
    uintptr_t offsets[4];		// four mip maps stored
} miptex_t;

typedef struct miptex2_s
{
    char		name[32];
    uintptr_t width, height;
    uintptr_t offsets[4];			// four mip maps stored
    char		animname[32];			// next frame in animation chain
    int			flags;
    int			contents;
    int			value;
} miptex2_t;

typedef struct sin_miptexnew_s
{
    char		name[32];
    unsigned	width, height;
    byte		palette[256*4];
    unsigned	offsets[4];				// four mip maps stored
} sin_miptexnew_t;

typedef struct valve_miptexnew_s
{
    char		name[16];
    unsigned	width, height;
    unsigned	offsets[4];				// four mip maps stored
    WORD		palSize;
    byte		*palette;
} valve_miptexnew_t;

typedef struct wadinfo_s
{
    char		identification[4];		// should be WAD2 or 2DAW
    int			numlumps;
    int			infotableofs;
} wadinfo_t;

typedef struct lumpinfo_s
{
    int			filepos;
    int			disksize;
    int 		size;					// uncompressed
    char		type;
    char		compression;
    char		pad1, pad2;
    char		name[16];				// must be null terminated
} lumpinfo_t;

typedef struct sin_lumpinfo_s
{
    int			filepos;
    int			disksize;
    int 		size;					// uncompressed
    char		type;
    char		compression;
    char		pad1, pad2;
    char		name[32];
} sin_lumpinfo_t;

typedef struct node_s
{
    int planenum;
    unsigned short front;
    unsigned short back;
    boundbox_t box;
} node_t;

typedef struct dleaf_s
{
    int type;
    boundbox_t bound;
    int vislist;
    int firstsurf;
    int numsurf;
    uintptr_t zeroes[3];
    unsigned short zero;
    unsigned short flag;
} dleaf_t;

typedef struct plane_s
{
    vec3_t normal;
    float dist;
    int type;
    int firstsurf;
    int numsurf;
} plane_t;

typedef struct dhullbound_s
{
    uintptr_t planenum;
    short front;
    short back;
} dhullbound_t;

typedef struct pixel32_s
{
    union
    {
        unsigned char chan[4];
        COLORREF rgb;
    };
    unsigned	p;
} pixel32_t;

#define TEX_TYPE_Q1 0
#define TEX_TYPE_Q2 1
typedef struct texturedef_s
{
    char	texture[32];
    float	rotate;
    float	shift[2];
    float	scale[2];

    int value;
    int contents;
    int flags;

    // Quake 2 stuff
    int texType;
    char basepath[32];
} texturedef_t;

typedef struct qtexture_s
{
    char	        name[32];
    int		        width;
    int		        height;
    unsigned char   *data;			// pointer to the image bits (belongs to texInfo_t)
    pixel32_t       flatcolor;
    int             bindIndex;
} qtexture_t;

#define		MAX_FACES		64
typedef float	vec5_t[5];
typedef struct winding_s
{
    int		numpoints;
    vec5_t	points[8];			// variable sized
} winding_t;

typedef struct leakPortal_s
{
    vec3_t centerPoint;
    winding_t *w;
} leakPortal_t;

typedef struct leakNode_s
{
    int numwindings;
    winding_t **w;
} leakNode_t;


#define MAX_POINTS_ON_WINDING	64

typedef struct face_s
{
    face_s *p_next, *p_prev;

    // implicit rep
    vec3_t			planepts[3];
    texturedef_t	texture;

    // cached rep
    plane_t			plane;
    qtexture_t		*qtexture;
    float			light;		// 0 - 1.0
    winding_t		*w;

    // Quake 2
    bool defaults;  // Use defaults if set to YES
    int dcontents;  // Default Values...
    int dflags;
    int dvalue;

    bool transparent;
    bool sky;

} face_t;

// lower bits are stronger, and will eat weaker brushes completely
#define	CONTENTS_SOLID			1		// an eye is never valid in a solid
#define	CONTENTS_WINDOW			2		// translucent, but not watery
#define	CONTENTS_AUX			4
#define	CONTENTS_LAVA			8
#define	CONTENTS_SLIME			16
#define	CONTENTS_WATER			32
#define	CONTENTS_MIST			64
#define	LAST_VISIBLE_CONTENTS	64

// remaining contents are non-visible, and don't eat brushes
#define	CONTENTS_PLAYERCLIP		0x10000
#define	CONTENTS_MONSTERCLIP	0x20000
// currents can be added to any other contents, and may be mixed
#define	CONTENTS_CURRENT_0		0x40000
#define	CONTENTS_CURRENT_90		0x80000
#define	CONTENTS_CURRENT_180	0x100000
#define	CONTENTS_CURRENT_270	0x200000
#define	CONTENTS_CURRENT_UP		0x400000
#define	CONTENTS_CURRENT_DOWN	0x800000
#define	CONTENTS_ORIGIN			0x1000000	// removed before bsping an entity
#define	CONTENTS_MONSTER		0x2000000	// should never be on a brush, only in game
#define	CONTENTS_DEADMONSTER	0x4000000
#define	CONTENTS_DETAIL			0x8000000	// brushes to be added after vis leafs
#define	CONTENTS_TRANSLUCENT	0x10000000	// auto set if any surface has trans
#define	CONTENTS_LADDER			0x20000000

#define CONTENTS_TRANS_MASK		(0xFFFFFFFF & ~(CONTENTS_SOLID|CONTENTS_LADDER))

#define	SURF_LIGHT		0x1		// value will hold the light strength
#define	SURF_SLICK		0x2		// effects game physics
#define	SURF_SKY		0x4		// don't draw, but add to skybox
#define	SURF_WARP		0x8		// turbulent water warp
#define	SURF_TRANS33	0x10
#define	SURF_TRANS66	0x20
#define	SURF_FLOWING	0x40	// scroll towards angle
#define	SURF_NODRAW		0x80	// don't bother referencing the texture
#define	SURF_HINT		0x100	// for bsp tree
#define	SURF_SKIP		0x200	// for bsp tree

#define SURF_TRANS_MASK			(SURF_TRANS33|SURF_TRANS66|SURF_FLOWING|SURF_NODRAW|SURF_HINT|SURF_SKIP)

#define NO_MOUSE                 0
#define DIRECTION_CALLBACK       1
#define NEW_CALLBACK             2
#define ROTATE_CALLBACK          3
#define SELECT_CALLBACK          4
#define CAMERA_CALLBACK          5
#define ADDCLIPPOINT_CALLBACK    6
#define TEXTUREBRUSH_CALLBACK    7
#define TEXTUREFACE_CALLBACK     8
#define FACEDRAG_CALLBACK        9
#define BRUSHDRAG_CALLBACK       10
#define CURFACEDRAG_CALLBACK     11
#define SHEAR_CALLBACK			 12
#define VERTEX_CALLBACK			 13
#define ENTITY_CONNECT           14
#define CLIPPT_CALLBACK     	 15
#define CLIPWHOLE_CALLBACK       16
#define MOVECAMERA_CALLBACK      17
#define SELECTTOTAL_CALLBACK     18
#define MULTIFACEDRAG_CALLBACK   19
#define MULTIFACEDRAG_CALLBACKSTRICT 20
#define EDGEDRAG_CALLBACK        21
#define SELECTONE_CALLBACK       22
#define KNOB_CALLBACK            72
#define OBJECT_CALLBACK          73

#define SEL_DESELECT 0
#define SEL_ADDTOBBOX 1
#define SEL_TRANSFORM 2
#define SEL_FLIPNORMALS 3
#define SEL_MOVETOENTITY 4
#define SEL_REMOVE 5
#define SEL_SELECTCOMPLETE 6
#define SEL_SELECTPARTIAL 7
#define SEL_CARVEBYCLIPPER 8
#define SEL_TRANSLATE 9
#define SEL_FEETTOFLOOR 10
#define SEL_SELECT 11
#define SEL_CAMERARENDERSELF 12
#define SEL_SPLITBYCLIPPER 13
#define SEL_SETFILTERED 14
#define SEL_FINDCLOSEST 15
#define SEL_SETREGIONED 16
#define SEL_SETREGIONEDPARTIAL 17
#define SEL_CALCWINDINGS 18
#define SEL_UNSETREGIONED 19
#define SEL_SELECTTOTAL 97
#define SEL_LOCK 98
#define SEL_UNLOCK 99

#endif	//_QTYPE_H_
