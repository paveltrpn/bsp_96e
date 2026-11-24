#ifndef _SETTINGS_H_
#define _SETTINGS_H_

// BSP Settings

#include "bsphdr.h"

#define FONT_NAME_LEN 64

class BSPSettings
{
public:
    BSPSettings();
    ~BSPSettings();

    // game.cfg settings
    int game;
    char default_mapfile[MAX_PATH];
    char entity_directory[MAX_PATH];
    char entity_file[MAX_PATH];
    char texture_path[MAX_PATH];
    char pak_file[MAX_PATH];
    char wad_directory[MAX_PATH];
    char default_wad[MAX_PATH];
    char map_directory[MAX_PATH];
    char bat_directory[MAX_PATH];
    char palette_file[MAX_PATH];
    char model_file[MAX_PATH];
    int sort_entities;
    int no_models;
    float gamma;
    float world_minmax;
    int fake_colormap;
    char apptitle[MAX_PATH];
    int gl_highres;
    char *gl_highres_path;
    int gl_trans_ents;


    // general
    int draw_old_style;
    int query_wall_width;
    int caption_percent;
    int caption_font_size;
    char caption_font[FONT_NAME_LEN];
    char help_font[FONT_NAME_LEN];
    int help_font_size;
    int nothresholdundo;
    int undo_threshold;
    int bsp_font_size;
    char bsp_font[FONT_NAME_LEN];
    int num_recent;
    char game_directory[MAX_PATH];
    char console_font[FONT_NAME_LEN];
    int console_font_size;

    // GL
    int gl_model_bbox;
    int gl_mode_default;
    int gl_shading;
    int gl_fog;
    int gl_fog_near;
    int gl_fog_far;
    int gl_fog_match_bg;
    COLORREF gl_fog_color;
    COLORREF gl_light_color;
    COLORREF gl_ambient_color;
    int gl_alpha_level;
    int gl_full_res_texture;
    int gl_wire_use_groupcolor;
    char gl_texturemode[64];
    int gl_brightness;
    int gl_selection_wiremode;
    int gl_enable_stencil;
    int gl_skybox;
    int gl_skybox_auto;
    char gl_skybox_path[MAX_PATH];
    char gl_skybox_name[64];

    //groups
    int group_font_size;
    int group_toolbar;
    int group_mode;

    //entity
    int entity_button_size;
    int entity_font_size;
    int entity_window_width;
    int entity_comment_size;
    int entity_keypair_size;
    int entity_inline_help;

    //texture
    int texture_width;
    int texture_height;
    int stretch_textures;
    int show_favorites;
    int show_frequent;
    int tex_numfav;
    int tex_numfrq;
    int height_fav;
    int height_frq;
    int tex_field_width;
    int show_tex_names;
    int texture_alignment;
    int texture_list_width;
    int nosmallstretch;
    int tex_animate;
    int tex_timerdelay;
    int tex_small_height;
    int tex_small_width;
    int tex_animatesky;
    int tex_skyspeed;
    int autocalc_frequents;
    int animate_models;
    int texlist_size;
    float tex_shiftstep;
    float tex_scalestep;
    float tex_rotatestep;


    // cameras
    int show_camera_angle;
    int show_cameras;
    int camera_angle_length;
    int lock_cameras;
    vec3_t lock_camera_offset;
    int camera_size;
    int camera_font_size;


    //colors
    COLORREF color_3dbackground;
    COLORREF color_gridmajor;
    COLORREF color_gridminor;
    COLORREF color_foreground;
    COLORREF color_background;
    COLORREF color_otherclipface;
    COLORREF color_currentclipface;
    COLORREF color_clipborder;
    COLORREF color_connection;
    COLORREF color_selection;
    COLORREF color_otherbrush;
    COLORREF color_curface;
    COLORREF color_hitbrush;
    COLORREF color_gridlabel;
    COLORREF color_eyemarker;
    COLORREF color_activecamera;
    COLORREF color_othercamera;
    COLORREF color_textureselection;
    COLORREF color_lock;
    COLORREF color_texturewindow;
    COLORREF color_3dgrid;
    COLORREF color_selectedtexture;
    COLORREF color_selectoutline;
    COLORREF color_faceoutline;
    COLORREF color_brushoutline;
    COLORREF color_consolefg;
    COLORREF color_consolebg;


    //clippers
    int animate_clip_points;
    int undo_clipper;
    int clipper_show_outline;
    int track_clippers;
    int clipper_autoflip;
    int clipper_mode;
    int clippers_stay;
    int querysplits;


    //editing
    int clone_delta_x;
    int clone_delta_y;
    int clone_delta_disable;
    int snap_back;					//snap vertices back to grid if they are not on it when manip
    int look_at_sel;				// camera will lock onto selection
    int new_brush_depth;
    int extrude_depth;
    int disable_rotation_texlock;
    int edge_use_grid;
    int multi_select;
    float max_face_gap;
    float min_epsilon;
    float max_epsilon;
    float max_normal_delta;
    float max_point_distance;
    float on_plane_epsilon;
    float multi_face_normal_eps;
    float multi_face_distance_eps;
    int vertex_select_mode;
    int vertex_drag_sensitivity;
    float face_drag;
    int select_delta;
    int vertex_use_grid;
    float distance_epsilon;


    //display
    int draw_models;
    int texture_models;
    int gridsize;
    int wire_ents;
    int use_crosshair;
    int outline_use_group;
    int select_delay;
    int drag_brushes;
    float scale;
    int steps_per_turn;
    float stepsize;
    int mlook_vert_step;
    int mlook_horiz_step;
    int xy_drag_move;
    int rotate_step;
    int grid_smallmajor;					//user smaller major grid dots in dot grid mode
    int grid_style;							//0=lines, 1=dots
    int angle_control;
    int show_grid;
    int show_coordinates;
    int show_names;
    int show_hits;
    float fov;
    int flymode;
    int ghosts_3d;
    int ghosts_xy;
    int track3dinxy;
    int trackxyin3d;
    int track3dincurrentxy;
    int warn_spaces;						// warn if spaces in map name
    int zero_crosshair;
    int region_clip_grid;
    int region_ignore;
    int color_wire;
    int cull_wire;
    int selection_thickness;
    int flats_like_texture;
    int center_knobs_xy;
    int center_knobs_3d;
    int center_knobs_use_x;
    float center_knobs_scale;
    int facelighting;
    float lightintensity;
    float ambientintensity;
    int flatlevel;
    int shadeflats;
    int shadetextures;
    int drawmode;
    int render_width;
    int render_height;
    int render_auto;
    int render_trans;	//0=off,1=lines,2=dots



    ////////////////////////////////////////UNSORTED/////////////
    ////////////////
    //////////////////////////////////////////////////////////
    int game_mode;
    int gl_texturemode_value;
    bool gl_skynames_loaded;
    // For 256 Color modes...
    int outlineIndex;
    int selectedIndex;
    int faceIndex;
    int lockIndex;
    //  For RGB Color Modes...

    // Window Positions
    int curCfg;



    int show_texturedrag;
    int track_leaks;
    int track_portals;
    int show_results;
    int region_mode;
    int render_connections;
    int show_connections;
    int show_drag;
    int redrawxy;
    int redrawedit;
    int Map_Read;
    int outline;
    int show_eye;

    // XY View Settings



    int xyViews;
    int curxy;


    // Renderer Settings;
    float ppd;
    float mag;
    bool auto_init;
    int lastw, lasth;
    float angstep;
    float Q[4][4];
    float R[4][4];
    int currentMode;
    // Texture Settings
    char rgborder[4];
    char rgb16order[4];

    int loadInUse;
    // Directories/Filenames
    char gfx_dir[MAX_PATH];
    char loadfile[MAX_PATH];
    char main_dir[MAX_PATH];


    // QII Stuff
    int pakOk;
    // Map Settings
    int filter_detail;
    int filter_hint;
    int filter_unselected;
    int filter_light;
    int filter_entities;
    int filter_world;
    int filter_clip_brushes;
    int filter_water_brushes;
    int filter_path;
    int filter_target;
    int filter_sky;
    int nummaps;
    int curmap;
    int show_3d_grid;
    int show_world_axes;

    // Misc
    int scx, scy;
    int autoSaveCounter;
    int autoSaveMinutes;
    char szAutoLoadFile[MAX_PATH];

    // Stuff for the run mode...
    int run_mode;
    float gravity;
    float forward_velocity;
    float side_velocity;
    float vertical_velocity;	 // vertical is part of gravity...
    float forward_attenuation;   // bleed off velocity...
    float side_attenuation;      // bleed off velocity...

    // impulse values...
    float impulse_forward;
    float impulse_side;
    float impulse_vertical;

    // absolute values...
    float max_forward_velocity;
    float max_side_velocity;
    float max_vertical_velocity;

    char default_gamedir[MAX_PATH];

    int glBsp;
    int sinBsp;
    int valveBsp;

    // DATA
    float far_clip_distance;
    float near_clip_distance;

    HFONT font_ui,font_ui_bold, font9,font10,font12;

    unsigned char pal[768+4];
    RGBQUAD cTable[256];

    char rel_path_separator;
};

extern BSPSettings set;

//SETINFO
class cset_t;
typedef void (*Cset_OnChange) (cset_t* cset,char *value);

class setinfo_value
{
public:
    setinfo_value() : value(0), info(0) {}
    ~setinfo_value()
    {
        delete [] value;
        delete [] info;
    }
    char *value;
    char *info;
};
class setinfo_item
{
public:
    setinfo_item()
    {
        memset(this,0,sizeof(setinfo_item));
    }
    ~setinfo_item()
    {
        delete [] info;
        delete values;
    }
    int type;
    char *info;						//info text
    list<setinfo_value*> *values;	//list of allowed values
    int imin,imax;					//range for int type
    float fmin,fmax;				//range for float type
    bool hasmin, hasmax;			//range enablers
};

extern void SetInfo_Parse(char *file);

//CSET - user access to settings

enum DataType
{
    CSET_FLOAT,
    CSET_INT,
    CSET_STRING,
    CSET_COLOR, //???  instead of these types, maybe have onchange handler specific to all colors, all paths, etc?
    CSET_VECTOR,
    CSET_PATH //?necessary??
};

#define CSET_FLAG_STRPTR 0x01

//todo: allow side-by-side user variables? aliases?
struct cset_group_s;

class cset_t
{
public:
    cset_t(char *name, DataType type, void *value, Cset_OnChange OnChange)
    {
        this->name = name;
        this->type = type;
        this->value = value;
        this->OnChange = OnChange;
        setinfo = 0;
        flags = 0;
    }
    ~cset_t()
    {
        delete setinfo;
    }
    char *name;
    int namelen;
    DataType type;
    int flags;
    void *value;
    setinfo_item *setinfo;
    Cset_OnChange OnChange;
    cset_group_s *group;	//this group
    cset_t *next_in_group;	//group
    cset_t *hash_next;		//hash
    cset_t *next;			//all
    ///////////////////////////////////////
    void setInt(int i);
    void setFloat(float f);
    void setVector(float *v);
    void setString(char *str,int maxlen);
};

typedef struct cset_group_s
{
    char	name[40];
    int		count;
    cset_t	*head;
    struct cset_group_s *next;
} cset_group_t;

#define Cset_Register(name,onChange) Cset_RegisterFn(&set.name,#name,onChange)
#define Cset_RegisterStr(name,onChange) Cset_RegisterFn((char**)&set.name,#name,onChange,0)
#define Cset_RegisterStrPtr(name,onChange) Cset_RegisterFn((char**)&set.name,#name,onChange,CSET_FLAG_STRPTR)

extern cset_t *cset_all;
extern cset_group_t *cset_groups;

extern bool Cset_LoadConfig(char *filename);
extern bool Cset_SaveConfig(char *filename, bool game);
extern void Cset_Initialize(void);
extern cset_t *Cset_FindSet(char *key);
extern void Cset_Unload();
extern bool Cset_SetValue(char *key, char *value);
extern void Cset_PrintValue(cset_t *s);
extern bool Cset_GetValueString(cset_t *c, char *out, int outlen);

extern int Cset_Complete(char *prefix, char *out, int outlen, list<char*> *matches);

#endif	//_SETTINGS_H_
