// BSP Setttings Implementation...

#include "global.h"

BSPSettings::BSPSettings()
{
    // Get and save .exe directory.
    GetModuleFileName(0, main_dir, sizeof(main_dir));
    char tempstr[MAX_PATH];
    ExtractFileBase(main_dir,tempstr,false);	//todo: this is dumb...
    main_dir[strlen(main_dir) - strlen(tempstr) - 1] = '\0';
    //SetCurrentDirectory(main_dir);
    Path::ChangeDir(set.main_dir);

    // all default values are specified here

    // game.cfg settings
    game = 0;					//game
    *default_mapfile  = 0;		//newmap
    *entity_directory = 0;		//entity_qc_directory
    *entity_file        = 0;	//entities
    *texture_path = 0;			//texture_path
    strcpy(pak_file, "c:\\quake2\\baseq2\\pak0.pak");		//q2_pak_file
    *wad_directory    = 0;		//wad_directory
    *default_wad    = 0;		//default_wad
    *map_directory    = 0;		//map_directory
    *bat_directory    = 0;		//bat_directory
    strcpy(palette_file,"quake.pal");	//palette_file
    strcpy(model_file,"models.ini");	//model_file
    sort_entities = 0;			//sort_entities
    no_models = 0;				//no_models
    gamma = 1.0f;				//gamma
    world_minmax = 4096.0f;		//world_minmax
    fake_colormap = 0;			//fake_colormap
    *apptitle = 0;				//apptitle


    // general
    draw_old_style = 1;
    query_wall_width = 1;
    caption_percent = 58;
    caption_font_size = 9;
    strcpy(caption_font,"small fonts");
    help_font_size = 10;
    strcpy(help_font,"arial");
    nothresholdundo = 1;
    undo_threshold = 100;
    bsp_font_size = 12;
    strcpy(bsp_font,"arial");
    num_recent = 4;
    strcpy(game_directory,main_dir);
    *default_gamedir = 0;
    strcpy(console_font, "terminal");
    console_font_size = 12;

    // GL
    gl_model_bbox = 1;
    gl_mode_default = 1;
    gl_shading = 1;
    gl_fog = 1;
    gl_fog_near = 300;
    gl_fog_far = 1024;
    gl_fog_match_bg = 1;
    gl_fog_color = RGB(200,200,200);
    gl_light_color = RGB(255,255,255);
    gl_ambient_color = RGB(100,100,100);
    gl_alpha_level = 120;
    gl_full_res_texture = 1;
    gl_wire_use_groupcolor = 0;
    strcpy(gl_texturemode, "GL_NEAREST_MIPMAP_NEAREST");
    gl_brightness = 0;
    gl_selection_wiremode = 0;
    gl_enable_stencil = 1;
    gl_skybox = 1;
    gl_skybox_auto = 1;
    strcpy(gl_skybox_path,"settings/gfx/env");
    strcpy(gl_skybox_name,"default");
    gl_highres = 0;
    gl_highres_path = 0;
    gl_trans_ents = 0;

    //groups
    group_font_size = 10;
    group_toolbar = 1;
    group_mode = 1;

    //entity
    entity_button_size = 12;
    entity_font_size = 10;
    entity_window_width = 200;
    entity_comment_size = 55;
    entity_keypair_size = 60;
    entity_inline_help = 0;

    //texture
    texture_width = 44;
    texture_height = 44;
    stretch_textures = 0;
    show_favorites = 1;
    show_frequent = 1;
    tex_numfav = 10;
    tex_numfrq = 10;
    height_fav = 32;
    height_frq = 32;
    tex_field_width = 60;
    show_tex_names = 1;
    texture_alignment = 0;
    texture_list_width = 120;
    nosmallstretch = 1;
    tex_animate = 1;
    tex_timerdelay = 200;
    show_texturedrag = 1;
    tex_small_height = 24;
    tex_small_width = 24;
    tex_animatesky = 1;
    tex_skyspeed = 3;
    autocalc_frequents = 1;
    animate_models = 1;
    texlist_size = 32;
    tex_scalestep = 0.1f;
    tex_rotatestep = 1.0f;
    tex_shiftstep = 1.0f;

    //cameras
    show_camera_angle = 1;
    show_cameras = 1;
    camera_angle_length = 6;
    lock_cameras = 0;
    lock_camera_offset[0] = lock_camera_offset[1] = lock_camera_offset[2] = 0;
    camera_size = 12;
    camera_font_size = 7;

    //colors
    color_foreground = RGB(255,255,255);
    color_background = RGB(0,0,0);
    color_gridmajor = RGB(168,211,255);
    color_gridminor = RGB(46,46,46);
    color_gridlabel = RGB(255,255,0);
    color_currentclipface = RGB(255,255,0);
    color_otherclipface = RGB(0,255,255);
    color_clipborder = RGB(255,0,0);
    color_connection = RGB(128,0,128);
    color_selection = RGB(255,0,0);
    color_otherbrush = RGB(127,127,127);
    color_curface = RGB(255,255,0);
    color_hitbrush = RGB(0,127,255);
    color_eyemarker = RGB(0,255,255);
    color_activecamera = RGB(255,128,0);
    color_othercamera = RGB(128,128,128);
    color_textureselection = RGB(0,255,255);
    color_lock = RGB(0,0,255);
    color_3dbackground = RGB(128,128,128);
    color_texturewindow = RGB(0,0,0);
    color_3dgrid = RGB(152,209,169);
    color_brushoutline = RGB(0,0,0);
    color_faceoutline  = RGB(255,255,0);
    color_selectoutline = RGB(255,63,0);
    color_selectedtexture = RGB(128,145,133);
    color_consolefg = RGB(222,222,222);
    color_consolebg = RGB(34,34,34);

    // clippers
    animate_clip_points = 1;
    undo_clipper = 1;
    clipper_show_outline = 1;
    track_clippers = 1;
    clipper_autoflip = 0;
    clipper_mode = 0;
    clippers_stay = 0;
    querysplits = 1;

    //editing
    clone_delta_x = 0;
    clone_delta_y = 0;
    clone_delta_disable = 0;
    snap_back = 0;
    look_at_sel = 0;
    new_brush_depth = 32;	//0 = old style depth calc, >0 = fixed size always
    extrude_depth = 0;
    disable_rotation_texlock = 0;
    edge_use_grid = 1;
    multi_select = 1;
    max_face_gap = 0.5;
    min_epsilon = 0.000001f;
    max_epsilon = 0.05f;
    max_normal_delta = 0.01f;
    max_point_distance = 0.05f;
    on_plane_epsilon = 0.1f;
    multi_face_normal_eps   = 0.1f;
    multi_face_distance_eps = 2.0f;
    vertex_select_mode = 2;
    vertex_drag_sensitivity = 32;
    face_drag = 0.0;
    select_delta = 4;
    vertex_use_grid = 1;
    distance_epsilon = 1.0f;

    // display
    draw_models = 1;
    texture_models = 1;
    gridsize = 8;
    wire_ents = 0;
    use_crosshair = 1;
    outline_use_group = 1;
    select_delay = 1000;
    drag_brushes = 30;
    scale = 1.0;
    steps_per_turn = 16;
    stepsize = 16;
    mlook_vert_step = 120;
    mlook_horiz_step = 150;
    xy_drag_move = 1;
    rotate_step = 1;
    grid_smallmajor = 1;
    grid_style = 1;
    angle_control = 0;
    show_grid = 1;
    show_names = 0;
    show_hits = 0;
    show_eye = 1;
    show_3d_grid = 0;		//show a grid in 3d view
    show_world_axes = 0;	//show axes in 3d
    fov = 90;				//old bsp's fov is about 57
    flymode = 1;
    ghosts_3d = 1;
    ghosts_xy = 1;
    track3dinxy = 1;
    trackxyin3d = 1;
    track3dincurrentxy = 1;
    warn_spaces = 1;
    far_clip_distance = ORIG_ZMAX;
    near_clip_distance = ORIG_ZMIN;
    zero_crosshair = 2;
    region_clip_grid=1;
    region_ignore = 0;
    color_wire = 1;
    cull_wire = 1;
    selection_thickness = 1;
    flats_like_texture = 1;
    center_knobs_xy = 1;
    center_knobs_3d = 1;
    center_knobs_use_x = 1;
    center_knobs_scale = 1.0f;
    facelighting = 1;
    lightintensity = 32.0f;
    ambientintensity = 16.0f;
    shadeflats = 1;
    shadetextures = 1;
    flatlevel = 32;
    drawmode = 2;
    render_width = 320;
    render_height = 200;
    render_auto = 1;
    render_trans = 0;



    ////////////////// UNSORTED /////////////////////////////

    //todo......................todo
    game_mode = 0;				//copied from game.cfg on startup
    glBsp = 0;
    sinBsp = 0;
    valveBsp = 0;

    region_mode = 1;

    *pal = 0;
    gl_skynames_loaded = false;

    render_connections = 1;
    track_leaks = 0;
    track_portals = 0;
    angstep = M_PI/4.0;

    auto_init = false;
    show_coordinates = 1;
    show_connections = 1;
    ppd = 32.0;
    mag = 4.0;
    nummaps = 0;
    curmap = 0;
    xyViews = 0;
    curxy = 0;
    show_results = 1;
    show_drag = 1;
    autoSaveCounter = 0;
    autoSaveMinutes = 1;

    loadInUse = 1;  // for Quake 2

    //DIRECTORIES

    pakOk = 0;
    curCfg = 0;

    font_ui			= NULL;
    font_ui_bold	= NULL;
    font9			= NULL;
    font10			= NULL;
    font12			= NULL;

    redrawxy = 1;
    redrawedit = 1;
    Map_Read = 0;
    outline = 1;
    drawmode = Drawmode::wire;

    outlineIndex  = 0;
    selectedIndex = 0;
    faceIndex     = 0;
    lockIndex     = 0;

    filter_unselected = 0;
    filter_light = 0;
    filter_entities = 0;
    filter_world = 0;
    filter_clip_brushes = 0;
    filter_water_brushes = 0;
    filter_path = 0;
    filter_target = 0;
    filter_sky = 0;
    filter_detail = 0;
    filter_hint = 0;

    // Stuff for the run mode...
    run_mode = Runmode::no_run;
    gravity = 16.0f;
    forward_velocity    = 0.0f;
    side_velocity       = 0.0f;
    vertical_velocity   = 0.0f;
    forward_attenuation = 16.0f;   // bleed off velocity...
    side_attenuation    = 16.0f;   // bleed off velocity...
    // vertical is part of gravity...

    // impulse values...
    impulse_forward     = 32.0f;
    impulse_side        = 32.0f;
    impulse_vertical    = 32.0f;

    // absolute values...
    max_forward_velocity  = 128.0f;
    max_side_velocity     = 128.0f;
    max_vertical_velocity = 128.0f;

    rel_path_separator = '/';

    set.scx = GetSystemMetrics(SM_CXSCREEN)/2; // save initial width and height...
    set.scy = GetSystemMetrics(SM_CYSCREEN)/2;
};

BSPSettings::~BSPSettings()
{
    delete[] gl_highres_path;
    gl_highres_path = 0;
    DeleteObject(font_ui);
    DeleteObject(font_ui_bold);
    DeleteObject(font9);
    DeleteObject(font10);
    DeleteObject(font12);
}


BSPSettings set;		//global config settings

//SETINFO

/*
Syntax:

TYPES:
	color	- color (eg "255 0 0" or "FF7C10")
	int		- any int (eg 123)
	float 	- any float (eg 1.23)
	string  - any string (eg "123")
	vector  - expects format "0.5 0.5 0.5"
	bool	- provides: value 1 "True" value 0 "False"

ATTRIBUTES:
	info	- info text
	value	- declares allowed values
	range	- specify range of allowed values
	min	    - minimum allowed value
	max  	- maximum allowed value

SYNTAX:
	DEF_ITEM:	info [info text]
	DEF_ITEM:	value [value] [description]
	DEF_ITEM:	range [min] [max]
	DEF_ITEMS:	DEF_ITEM (DEF_ITEM (...))
	DEF:		[info text] | { DEF_ITEMS }
	TYPE:		color | int | float | string | vector

	DECL:		TYPE [Setting name] DEF

*/


//parse setinfo.cfg, reloads 'setinfo' member of cset_t
void SetInfo_Parse(char *file)
{
    unsigned char *data = file_get_contents(file);
    if (!data)
    {
        syserror(const_cast<char *> ("SetInfo_Parse: Failed to open %s"), file);
        return;
    }
    Tokenizer script((char*)data);
    while(script.next(true))
    {
        //type
        int type;
        bool type_bool = false;	//special case for int
        if(!stricmp(script.token, "color"))
            type = CSET_COLOR;
        else if (!stricmp(script.token, "int"))
            type = CSET_INT;
        else if (!stricmp(script.token, "bool"))
        {
            type = CSET_INT;
            type_bool = true;
        }
        else if (!stricmp(script.token, "float"))
            type = CSET_FLOAT;
        else if (!stricmp(script.token, "string"))
            type = CSET_STRING;
        else if (!stricmp(script.token, "vector"))
            type = CSET_VECTOR;
        else
        {
            syserror(const_cast<char *> ("SetInfo_Parse: invalid type '%s' in file %s (Line %d)"),script.token,file, script.line);
            goto error_exit;
        }
        //name
        char name[256];
        if(!script.next(true))
        {
            syserror(const_cast<char *> ("SetInfo_Parse: expected 'Setting name' in file %s (Line %d)"),file, script.line);
            goto error_exit;
        }
        strncpy(name,script.token,sizeof(name));
        name[sizeof(name) - 1] = 0;

        //definition
        if(!script.next(true))
        {
            syserror(const_cast<char *> ("SetInfo_Parse: expected 'Setting definition' in file %s (Line %d)"),file, script.line);
            goto error_exit;
        }

        //set up pointers to give to final structure
        char *info = 0;							//info text
        list<setinfo_value*> *values = 0;		//list of allowed values
        //min/max/range allowed for int and float only
        int imin=0,imax=0;
        float fmin=0,fmax=0;
        bool hasmin = false, hasmax = false;
        if(strcmp(script.token,"{"))
        {
            //token is single info string
            //	if(!script.next(true)) {
            //		syserror("SetInfo_Parse: expected 'Setting info' in file %s (Line %d)",file, script.line);
            //		goto error_exit;
            //	}
            //copy info text
            int len = strlen(script.token);
            info = new char[len + 1];
            strcpy(info, script.token);
            //set up bool type
            if(type_bool)
            {
                values = new list<setinfo_value*>();
                //add TRUE
                setinfo_value *v = new setinfo_value;
                v->value = new char[4];
                strcpy(v->value,"1");
                v->info = new char[8];
                strcpy(v->info,"True");
                values->add(v);
                //add FALSE
                v = new setinfo_value;
                v->value = new char[4];
                strcpy(v->value,"0");
                v->info = new char[8];
                strcpy(v->info,"False");
                values->add(v);
            }
        }
        else
        {
            //verbose definition
            while(script.next(true))
            {
                if(!strcmp(script.token,"}"))
                    break;
                else if(!stricmp(script.token, "info"))
                {
                    if(!script.next(true))
                        break;
                    //copy info text
                    int len = strlen(script.token);
                    delete [] info;
                    info = new char[len + 1];
                    strcpy(info, script.token);
                }
                else if(!stricmp(script.token, "range"))
                {
                    hasmin = hasmax = true;
                    if(!script.next(true))
                        break;
                    if(type == CSET_INT)
                    {
                        imin = atoi(script.token);
                    }
                    else if(type == CSET_FLOAT)
                    {
                        fmin = (float) atof(script.token);
                    }
                    if(!script.next(true))
                        break;
                    if(type == CSET_INT)
                    {
                        imax = atoi(script.token);
                    }
                    else if(type == CSET_FLOAT)
                    {
                        fmax = (float) atof(script.token);
                    }
                }
                else if(!stricmp(script.token, "min"))
                {
                    hasmin = true;
                    if(!script.next(true))
                        break;
                    if(type == CSET_INT)
                    {
                        imin = atoi(script.token);
                    }
                    else if(type == CSET_FLOAT)
                    {
                        fmin = (float) atof(script.token);
                    }
                }
                else if(!stricmp(script.token, "max"))
                {
                    hasmax = true;
                    if(!script.next(true))
                        break;
                    if(type == CSET_INT)
                    {
                        imax = atoi(script.token);
                    }
                    else if(type == CSET_FLOAT)
                    {
                        fmax = (float) atof(script.token);
                    }
                }
                else if(!stricmp(script.token, "value"))
                {
                    //value
                    if(!script.next(true))
                        break;
                    int lenval = strlen(script.token);
                    char *strval = new char[lenval+1];
                    strcpy(strval, script.token);
                    //info
                    if(!script.next(true))
                    {
                        //clean up value portion
                        delete [] strval;
                        break;
                    }
                    int leninfo = strlen(script.token);
                    char *strinfo = new char[leninfo+1];
                    strcpy(strinfo, script.token);
                    //create values if not set
                    if(!values)
                        values = new list<setinfo_value*>();
                    //add item
                    setinfo_value *v = new setinfo_value;
                    v->value = strval;
                    v->info = strinfo;
                    values->add(v);
                }
            }
            //script.token should equal "}"
            if(strcmp(script.token,"}"))
            {
                syserror(const_cast<char *> ("SetInfo_Parse: unexpected EOF in file %s (Line %d)"),file, script.line);
                //parsing this item failed, so cleanup and abort
                delete [] info;
                delete values;
                goto error_exit;
            }
        }

        //build item
        setinfo_item *si = new setinfo_item;
        si->fmax = fmax;
        si->fmin = fmin;
        si->hasmax = hasmax;
        si->hasmin = hasmin;
        si->imax = imax;
        si->imin = imin;
        si->info = info;
        si->type = type;
        si->values = values;

        //add item, find cset_t to add to
        cset_t *cs = Cset_FindSet(name);
        if(cs)
        {
            delete cs->setinfo;
            cs->setinfo = si;
        }
        else
        {
            syserror(const_cast<char *> ("SetInfo_Parse: No setting found with name '%s'"), name);
            delete si;
        }

        //loop continues from next token
    }

error_exit:
    delete [] data;
    return;
}

//CSET_GROUP

//CSET
#define CSET_HASH_SIZE 200

static cset_t *cset_hash[CSET_HASH_SIZE];
cset_t *cset_all;
cset_group_t *current_group;
cset_group_t *cset_groups;
//Cset_AddGroup
cset_group_t * Cset_AddGroup(char *name)
{
    cset_group_t *g = new cset_group_t;
    strncpy(g->name, name, sizeof(g->name));
    g->count = 0;
    g->head = 0;
    current_group = g;
    ADD_TO_LIST_SORTED(cset_group_t, g, cset_groups, next);
    return g;
}
//Cset_SetCurrentGroup
cset_group_t *Cset_SetCurrentGroup(char *name)
{
    cset_group_t *g = cset_groups;
    for(; g; g=g->next)
        if(!stricmp(g->name,name))
        {
            current_group = g;
            return g;
        }
    return Cset_AddGroup(name);
}
//Cset_HashKey
int Cset_HashKey (char *name)
{
    unsigned char c;
    uintptr_t v = 0;
    while ( (c = *name++) != 0 )
        v += c &~ 32;   // make it case insensitive

    return v % CSET_HASH_SIZE;
}
//Cset_Complete
int Cset_Complete(char *prefix, char *out, int outlen, list<char*> *matches)
{
    if(!out)
        return 0;
    if(!prefix)
        return 0;
    if(!matches)
        return 0;
    *out = 0;

    int plen = strlen(prefix);
    if(plen > outlen)
        return 0;

    int count = 0;
    cset_t *c;
    for(c = cset_all; c; c = c->next)
    {
        if(!_strnicmp(prefix, c->name, plen))
        {
            if(!*out)
            {
                strncpy(out, c->name, outlen);
            }
            else
            {
                for(int i = 0; out[i]; i++)
                {
                    if(out[i] != c->name[i])
                    {
                        out[i] = 0;
                        break;
                    }
                }
            }
            matches->addsorted(c->name);
            ++count;
        }
    }
    return count;
}


void cset_t::setInt(int i)
{
    int *p = (int*) value;
    *p = i;
}
void cset_t::setString(char *str,int maxlen)
{
    char *s = (char*) value;
    strncpy(s,str,maxlen);
}
void cset_t::setFloat(float f)
{
    float *p = (float*) value;
    *p = f;
}
void cset_t::setVector(float *v)
{
    float *p = (float*) value;
    p[0] = v[0];
    p[1] = v[1];
    p[2] = v[2];
}

bool Cset_RegisterMain(cset_t *cs)
{
    if (Cset_FindSet(cs->name))
    {
        syserror(const_cast<char *> ("Warning: Cset_Register: %s already added!"), cs->name);
        delete cs;
        return false;
    }
    int key = Cset_HashKey(cs->name);
    cs->hash_next = cset_hash[key];
    cset_hash[key] = cs;
    ADD_TO_LIST_SORTED(cset_t, cs, cset_all, next);
    //groups
    cs->group = current_group;
    current_group->count++;
    ADD_TO_LIST_SORTED(cset_t, cs, current_group->head, next_in_group);
    return true;
}
//int
bool Cset_RegisterFn(int* var,char* name,Cset_OnChange cfunc)
{
//   printf("registering int \"%s\" = \"%d\"\n", name, *var);
    cset_t *cs = new cset_t(name, CSET_INT, var, cfunc);
    return Cset_RegisterMain(cs);
}
//float
bool Cset_RegisterFn(float* var,char* name,Cset_OnChange cfunc)
{
    //printf("registering float \"%s\" = \"%f\"\n", name, *var);
    cset_t *cs = new cset_t(name, CSET_FLOAT, var, cfunc);
    return Cset_RegisterMain(cs);
}
//vector
bool Cset_RegisterFn(vec3_t* var,char* name,Cset_OnChange cfunc)
{
    //printf("registering vector \"%s\" = \"%f %f %f\"\n", name, *var[0], *var[1], *var[2]);
    cset_t *cs = new cset_t(name, CSET_VECTOR, var, cfunc);
    return Cset_RegisterMain(cs);
}
//string
bool Cset_RegisterFn(char* *var,char* name,Cset_OnChange cfunc, int flags)
{
    //printf("registering string \"%s\" = \"%s\"\n", name, *var);
    cset_t *cs = new cset_t(name, CSET_STRING, var, cfunc);
    cs->flags = flags;
    return Cset_RegisterMain(cs);
}
//color
bool Cset_RegisterFn(COLORREF* var,char* name,Cset_OnChange cfunc)
{
//   printf("registering color \"%s\" = \"%X\"\n", name, *var);
    cset_t *cs = new cset_t(name, CSET_COLOR, var, cfunc);
    return Cset_RegisterMain(cs);
}

cset_t *Cset_FindSet(char *name)
{
    int key = Cset_HashKey(name);
    cset_t *c;
    for(c = cset_hash[key]; c; c = c->hash_next)
    {
        if(!stricmp(name,c->name))
            return c;
    }
    return 0;
}
bool Cset_SetValue(char *key, char *value)
{
    cset_t *cs = Cset_FindSet(key);
    if(!cs)
        return false;
    if(cs->OnChange)
    {
        cs->OnChange(cs, value);
    }
    else if(cs->type == CSET_INT)
    {
        cs->setInt(atoi(value));
    }
    else if(cs->type == CSET_FLOAT)
    {
        cs->setFloat((float) atof(value));
    }
    else if(cs->type == CSET_VECTOR)
    {
        vec3_t v;
        int c = sscanf(value,"%f %f %f",&v[0],&v[1],&v[2]);
        if(c!=3)
        {
            syserror(const_cast<char *> ("Error setting vector value for %s (bad value: %s)"),key, value);
        }
        else
        {
            cs->setVector(v);
        }
    }
    else if(cs->type == CSET_STRING)
    {
        syserror(const_cast<char *> ("Error: no default OnChange handler available for %s"),key);
    }
    else if(cs->type == CSET_COLOR)
    {
        COLORREF color = Parse_Color(value);
        if(color != -1)
            cs->setInt(color);
    }
    //iterate keys, find and return
    return true;
}

bool Cset_GetValueString(cset_t *c, char *out, int outlen)
{
    bool retval = false;
    if(c)
    {
        char v1[32],v2[32],v3[32];
        COLORREF color;
        switch(c->type)
        {
        case CSET_COLOR:
            color = *(COLORREF*)c->value;
            snprintf(out,outlen,"#%.2X%.2X%.2X",GetRValue(color),GetGValue(color),GetBValue(color));
            retval = true;
            break;
        case CSET_VECTOR:
            snprintf(v1,32,"%f",((float*)c->value)[0]);
            snprintf(v2,32,"%f",((float*)c->value)[1]);
            snprintf(v3,32,"%f",((float*)c->value)[2]);
            snprintf(out,outlen,"%s %s %s",FixFloatString(v1),FixFloatString(v2),FixFloatString(v3));
            retval = true;
            break;
        case CSET_INT:
            snprintf(out,outlen,"%d",*(size_t *)c->value);
            retval = true;
            break;
        case CSET_FLOAT:
            snprintf(out,outlen,"%f",*(float *)c->value);
            FixFloatString(out);
            retval = true;
            break;
        case CSET_STRING:
            if(c->flags & CSET_FLAG_STRPTR)
                snprintf(out,outlen,"%s",*(char **)c->value);
            else
                snprintf(out,outlen,"%s",(char *)c->value);
            retval = true;
            break;
        }
    }

    out[outlen - 1] = 0;
    return retval;
}
//echo value to console
void Cset_PrintValue(cset_t *s)
{
    if(s)
    {
        char val[MAX_VALUE] = "";
        Cset_GetValueString(s, val, sizeof(val));
        sysprintf(const_cast<char *> ("\"%s\" = \"%s\""),(char*) s->name, val);
    }
}
//echo value to file
void Cset_fprintfValue(FILE *f, cset_t *s)
{
    if(f && s)
    {
        char val[MAX_VALUE] = "";
        Cset_GetValueString(s, val, sizeof(val));
        fprintf(f, "%s \"%s\"\n",(char*) s->name, val);
    }
}

// free memory used by Csets
void Cset_Unload()
{
    cset_t *c = cset_all, *tmp;
    for( ; c; c = tmp)
    {
        tmp = c->next;
        delete c;
    }
    cset_group_t *g = cset_groups, *tg;
    for( ; g; g = tg)
    {
        tg = g->next;
        delete g;
    }
    memset(cset_hash,0,sizeof(cset_hash));
}

//load all settings from user
bool Cset_LoadConfig(char *filename)
{
    char *dat = (char*) file_get_contents (filename);
    if(!dat)
    {
        syserror (const_cast<char *> ("Config: couldn't load %s"), filename);
        return false;
    }

    Tokenizer config(dat);
    while(true)
    {
        char key[64], value[512];
        //each line is: "key \"value\"\n"
        if(!config.next(true))
            break;
        strncpy(key,config.token,sizeof(key));
        if(!config.next(true))
            break;

        strncpy(value,config.token,sizeof(value));
        config.skipline();
        if(!Cset_SetValue(key, value))
        {
            syserror(const_cast<char *> ("Warning: unknown key \"%s\" = \"%s\" in file %s (Line %d)"),
                     key, value, filename, config.line);
        }
    }

    delete[] dat;
    return true;
}

//save config file. if game is true, only "Game" group is saved. otherwise, all groups except Game are saved.
bool Cset_SaveConfig(char *filename, bool game)
{
    FILE *f = fopen(filename, "w");
    if(!f)
    {
        syserror(const_cast<char *> ("Save config: Couldn't open %s for writing"), filename);
        return false;
    }


    cset_group_t *g = cset_groups;
    for( ; g; g = g->next)
    {
        if(game)
        {
            //write game only
            if(!stricmp(g->name, "Game"))
            {
                fprintf(f, "\n// %s\n",g->name);
                cset_t *c = g->head;
                for( ; c; c = c->next_in_group)
                {
                    Cset_fprintfValue(f, c);
                }
                break;
            }
        }
        else
        {
            //write all but game
            if(stricmp(g->name, "Game"))
            {
                fprintf(f, "\n// %s\n",g->name);
                cset_t *c = g->head;
                for( ; c; c = c->next_in_group)
                {
                    Cset_fprintfValue(f, c);
                }
            }
        }
    }

    fclose(f);
    return true;
}

//ONCHANGE HANDLERS
void Cset_change_gridsize(cset_t* cs,char *value)
{
    cs->setInt(max(1,atoi(value)));
    if (xyWindow)
        for (int c = 0 ; c < set.xyViews; c++)
            if (xyWindow[c])
                xyWindow[c]->RedrawContents();
}
void Cset_change_gamma(cset_t* cs,char *value)
{
    cs->setFloat(min(3.0f,max(0.1f,(float) atof(value))));
}
//change color - reloads all colors and redraws
void Cset_change_color(cset_t* cs,char *value)
{
    int c = (int)Parse_Color(value);
    if(c == -1)
    {
        syserror(const_cast<char *> ("unknown color format \"%s\""),value);
        return;
    }
    cs->setInt(c);
    if(set.glBsp)
    {
        glSetBackgroundColor(set.color_3dbackground);
        if(set.gl_fog_match_bg)
            glSetFogColor(set.color_3dbackground);
        else
            glSetFogColor(set.gl_fog_color);
    }
    if(client && set.Map_Read)
        RedrawWindow(client->hwnd, 0, 0, RDW_ALLCHILDREN|RDW_INVALIDATE);
}
void Cset_change_gl_color(cset_t* cs,char *value)
{
    Cset_change_color(cs,value);
    glReloadBSPSettings();
}
void Cset_change_camera_angle_length(cset_t* cs,char *value)
{
    cs->setInt(max(1,atoi(value)));
}
void Cset_change_default_mapfile(cset_t* cs,char *value)
{
    cs->setString(value,sizeof(set.default_mapfile));
}
void Cset_change_default_gamedir(cset_t* cs,char *value)
{
    cs->setString(value,sizeof(set.default_gamedir));
}
void Cset_change_font(cset_t* cs,char *value)
{
    cs->setString(value,FONT_NAME_LEN);
}
void Cset_change_max_face_gap(cset_t* cs,char *value)
{
    cs->setFloat(min(10.0f,min(0.0001f,(float) atof(value))));
}
void Cset_change_min_epsilon(cset_t* cs,char *value)
{
    cs->setFloat(max(0.000001f,(float) atof(value)));
}
void Cset_change_max_epsilon(cset_t* cs,char *value)
{
    cs->setFloat(max(0.000001f,(float) atof(value)));
}
void Cset_change_max_normal_delta(cset_t* cs,char *value)
{
    cs->setFloat(min(0.5f,max(0.0001f,(float) atof(value))));
}
void Cset_change_max_point_distance(cset_t* cs,char *value)
{
    cs->setFloat(min(10.0f,max(0.0001f,(float) atof(value))));
}
void Cset_change_on_plane_epsilon(cset_t* cs,char *value)
{
    cs->setFloat(min(10.0f,max(0.0001f,(float) atof(value))));
}
void Cset_change_multi_face_normal_eps(cset_t* cs,char *value)
{
    cs->setFloat(min(3.5f,max(0.0001f,(float) atof(value))));
}
void Cset_change_multi_face_distance_eps(cset_t* cs,char *value)
{
    cs->setFloat(max(0.0001f,(float) atof(value)));
}
void Cset_change_face_drag(cset_t* cs,char *value)
{
    cs->setFloat(min(1.f,max(0.0f,(float) atof(value))));
}
void Cset_change_select_delta(cset_t* cs,char *value)
{
    cs->setInt(max(1,atoi(value)));
}
void Cset_change_clamp01(cset_t* cs,char *value)
{
    cs->setInt(atoi(value) != 0 ? 1 : 0);
}
void Cset_change_angle_control(cset_t* cs, char *value)
{
    if(!stricmp(value, "fov"))
        cs->setInt(FOV_CONTROL);
    else if(!stricmp(value, "yaw"))
        cs->setInt(YAW_CONTROL);
    else if(!stricmp(value, "pitch"))
        cs->setInt(PITCH_CONTROL);
    else
        cs->setInt(max(0,min(3,atoi(value))));	//angle control values 0,1,2,3
    if(editWindow)
        editWindow->RedrawContents();
}
void Cset_change_gl_clamp01(cset_t* cs,char *value)
{
    cs->setInt(atoi(value) != 0 ? 1 : 0);
    glReloadBSPSettings();
}
void Cset_change_gl_clamp012(cset_t* cs,char *value)
{
    cs->setInt(min(2,max(0,atoi(value))));
    glReloadBSPSettings();
}
void Cset_change_clamp1_10(cset_t* cs,char *value)
{
    cs->setInt(min(10,max(1,atoi(value))));
}
void Cset_change_min4(cset_t* cs,char *value)
{
    cs->setInt(max(4,atoi(value)));
}
void Cset_change_min1(cset_t* cs,char *value)
{
    cs->setInt(max(1,atoi(value)));
}
void Cset_change_texture_width_list(cset_t* cs,char *value)
{
    cs->setInt(min(400,max(80,atoi(value))));
}
void Cset_change_texlist_size(cset_t* cs,char *value)
{
    cs->setInt(min(256,max(32,atoi(value))));
}
void Cset_change_scale(cset_t* cs,char *value)
{
    cs->setFloat(min(16.0f,max(1.0f/256.0f,(float) atof(value))));
    if (xyWindow)
        for (int c = 0 ; c < set.xyViews; c++)
            if (xyWindow[c])
                xyWindow[c]->RedrawContents();
}
void Cset_change_stepsize(cset_t* cs,char *value)
{
    cs->setFloat(max(1.0f,(float) atof(value)));
}
void Cset_change_tex_shiftstep(cset_t* cs,char *value)
{
    cs->setFloat(min(128.0f,max(0.0001f,(float) atof(value))));
}
void Cset_change_tex_rotatestep(cset_t* cs,char *value)
{
    cs->setFloat(min(90.0f,max(0.0001f,(float) atof(value))));
}
void Cset_change_tex_scalestep(cset_t* cs,char *value)
{
    cs->setFloat(min(1.0f,max(0.0001f,(float) atof(value))));
}
void Cset_change_fov(cset_t* cs,char *value)
{
    int fov = set.glBsp ? max(15,min(120,atoi(value))) : max(15,min(180,atoi(value)));	//weird clamps
    cs->setFloat((float)fov);
}
void Cset_change_clamp012(cset_t* cs,char *value)
{
    cs->setInt(max(0,min(2,atoi(value))));
}
void Cset_change_render_auto(cset_t* cs,char *value)
{
    set.auto_init = false;		//render settings need reload
    cs->setInt(atoi(value));
}
void Cset_change_clamp0_63(cset_t* cs,char *value)
{
    cs->setInt(min(63,max(0,atoi(value))));
}
void Cset_change_clamp0_63f(cset_t* cs,char *value)
{
    cs->setFloat(min(63.0f,max(0,(float) atof(value))));
}
void Cset_change_gl_brightness(cset_t* cs,char *value)
{
    cs->setInt(min(255,max(0,atoi(value))));
    if(set.glBsp)
        editWindow->RedrawContents();
}
void Cset_change_gl_fog_dist(cset_t* cs,char *value)
{
    cs->setInt(atoi(value));
    glReloadBSPSettings();
}
void Cset_change_gl_alpha_level(cset_t* cs,char *value)
{
    cs->setInt(min(255,max(0,atoi(value))));
    if(set.glBsp)
        editWindow->RedrawContents();
}
void Cset_change_num_recent(cset_t *cs,char *value)
{
    cs->setInt(min(MAX_RECENT,max(0,atoi(value))));
}
void Cset_change_entity_directory(cset_t *cs,char *value)
{
    cs->setString(RemoveTrailingSlash(value),sizeof(set.entity_directory));
}
void Cset_change_entity_file(cset_t *cs,char *value)
{
    cs->setString(value,sizeof(set.entity_file));
}
void Cset_change_texture_path(cset_t *cs,char *value)
{
    cs->setString(RemoveTrailingSlash(value), sizeof(set.texture_path));
}
void Cset_change_pak_file(cset_t *cs,char *value)
{
    cs->setString(value, sizeof(set.pak_file));
}
void Cset_change_wad_directory(cset_t *cs,char *value)
{
    cs->setString(RemoveTrailingSlash(value), sizeof(set.wad_directory));
}
void Cset_change_default_wad(cset_t *cs,char *value)
{
    cs->setString(value, sizeof(set.default_wad));
}
void Cset_change_map_directory(cset_t *cs,char *value)
{
    cs->setString(RemoveTrailingSlash(value),sizeof(set.map_directory));
}
void Cset_change_bat_directory(cset_t *cs, char *value)
{
    cs->setString(RemoveTrailingSlash(value),sizeof(set.bat_directory));
}
void Cset_change_palette_file(cset_t *cs, char *value)
{
    cs->setString(value,sizeof(set.palette_file));
}
void Cset_change_model_file(cset_t *cs, char *value)
{
    cs->setString(value,sizeof(set.model_file));
}
void Cset_change_world_minmax(cset_t *cs, char *value)
{
    cs->setFloat(max(512.0f,(float) atof(value)));
}
void Cset_change_apptitle(cset_t *cs, char *value)
{
    cs->setString(value,sizeof(set.apptitle));
}
void Cset_change_gl_texturemode(cset_t *cs, char *value)
{
    set.gl_texturemode_value = GetTextureModeValue(value);
    cs->setString(value,sizeof(set.gl_texturemode));

    if (!set.Map_Read || !set.glBsp)
        return;

    //rebind textures
    UnloadTextures();
    texWindow->SetUpMap(map_i[set.curmap]);
    editWindow->RedrawContents();
}
void Cset_change_gl_skybox_path(cset_t *cs, char *value)
{
    cs->setString(value,sizeof(set.gl_skybox_path));
    if (!set.gl_skybox || !set.Map_Read || !set.glBsp)
        return;
    glReloadSkybox();
    glReloadBSPSettings();
}
void Cset_change_gl_skybox_name(cset_t *cs, char *value)
{
    cs->setString(value,sizeof(set.gl_skybox_name));
    if (!set.gl_skybox || !set.Map_Read || !set.glBsp)
        return;
    glReloadSkybox();
    glReloadBSPSettings();
}
void Cset_change_gl_skybox(cset_t *cs, char *value)
{
    cs->setInt(atoi(value) != 0 ? 1 : 0);
    if (!set.Map_Read || !set.glBsp)
        return;
    if(!set.gl_skybox)
        glUnloadSkybox();
    else
        glReloadSkybox();
}
void Cset_change_gl_highres_path(cset_t *cs, char *value)
{
    if(!value) return;
    int len = strlen(value) + 1;
    delete[] set.gl_highres_path;
    set.gl_highres_path = new char[len];
    strcpy(set.gl_highres_path, value);

    if (!set.gl_highres || !set.Map_Read || !set.glBsp)
        return;

    //rebind textures
    UnloadTextures();
    texWindow->SetUpMap(map_i[set.curmap]);
    editWindow->RedrawContents();
}
void Cset_change_gl_highres(cset_t* cs,char *value)
{
    cs->setInt(atoi(value) != 0 ? 1 : 0);

    if (!set.Map_Read || !set.glBsp)
        return;

    //rebind textures
    UnloadTextures();
    texWindow->SetUpMap(map_i[set.curmap]);
    editWindow->RedrawContents();
}
void Cset_change_caption_percent(cset_t *cs, char *value)
{
    cs->setInt(atoi(value));
    //todo: need to send wm_size message to all windows for this to work
//	SetCaptionHeight();
//	if(client) InvalidateRect(client->hwnd,0,1);
}

// gets rid of annoying "deprecated conversion from string constant blah blah" warning
#pragma GCC diagnostic ignored "-Wwrite-strings"

//add all settings to Cset list
void Cset_Initialize()
{
    // clear stuff
    memset(cset_hash, 0, sizeof(cset_t*) * CSET_HASH_SIZE);
    cset_all = 0;
    current_group = 0;
    cset_groups = 0;

    // colors
    Cset_SetCurrentGroup(const_cast<char *> ("Colors"));
    Cset_Register(color_foreground,		Cset_change_color );
    Cset_Register(color_background,		Cset_change_color );
    Cset_Register(color_gridmajor,			Cset_change_color );
    Cset_Register(color_gridminor,			Cset_change_color );
    Cset_Register(color_gridlabel,			Cset_change_color );
    Cset_Register(color_currentclipface,	Cset_change_color );
    Cset_Register(color_otherclipface,		Cset_change_color );
    Cset_Register(color_clipborder,		Cset_change_color );
    Cset_Register(color_connection,		Cset_change_color );
    Cset_Register(color_selection,			Cset_change_color );
    Cset_Register(color_otherbrush,		Cset_change_color );
    Cset_Register(color_curface,			Cset_change_color );
    Cset_Register(color_hitbrush,			Cset_change_color );
    Cset_Register(color_eyemarker,			Cset_change_color );
    Cset_Register(color_activecamera,		Cset_change_color );
    Cset_Register(color_othercamera,		Cset_change_color );
    Cset_Register(color_textureselection,	Cset_change_color );
    Cset_Register(color_lock,				Cset_change_color );
    Cset_Register(color_3dbackground,		Cset_change_color );
    Cset_Register(color_texturewindow,		Cset_change_color );
    Cset_Register(color_3dgrid,			Cset_change_color );
    Cset_Register(color_brushoutline,		Cset_change_color );
    Cset_Register(color_faceoutline,		Cset_change_color );
    Cset_Register(color_selectoutline,		Cset_change_color );
    Cset_Register(color_selectedtexture,	Cset_change_color );
    Cset_Register(color_consolefg,			Cset_change_color );
    Cset_Register(color_consolebg,			Cset_change_color );

    // GL
    Cset_SetCurrentGroup(const_cast<char *> ("OpenGL"));
    Cset_Register(gl_model_bbox,			Cset_change_gl_clamp01 );
    Cset_Register(gl_mode_default,			Cset_change_clamp01 );
    Cset_Register(gl_shading,				Cset_change_gl_clamp01 );
    Cset_Register(gl_fog,					Cset_change_gl_clamp01 );
    Cset_Register(gl_fog_near,				Cset_change_gl_fog_dist );
    Cset_Register(gl_fog_far,				Cset_change_gl_fog_dist );
    Cset_Register(gl_fog_match_bg,			Cset_change_gl_clamp01 );
    Cset_Register(gl_fog_color,			Cset_change_gl_color );
    Cset_Register(gl_light_color,			Cset_change_gl_color );
    Cset_Register(gl_ambient_color,		Cset_change_gl_color );
    Cset_Register(gl_alpha_level,			Cset_change_gl_alpha_level );
    Cset_Register(gl_full_res_texture,		Cset_change_clamp01 );
    Cset_Register(gl_wire_use_groupcolor,  Cset_change_gl_clamp01 );
    Cset_RegisterStr(gl_texturemode,		Cset_change_gl_texturemode );
    Cset_Register(gl_brightness,			Cset_change_gl_brightness );
    Cset_Register(gl_selection_wiremode,	Cset_change_gl_clamp012 );
    Cset_Register(gl_enable_stencil,		Cset_change_clamp01 );
    Cset_Register(gl_skybox,				Cset_change_gl_skybox );
    Cset_Register(gl_skybox_auto,			Cset_change_clamp01 );
    Cset_RegisterStr(gl_skybox_path,		Cset_change_gl_skybox_path );
    Cset_RegisterStr(gl_skybox_name,		Cset_change_gl_skybox_name );
    Cset_Register(gl_trans_ents,			Cset_change_gl_clamp01 );

    // groups
    Cset_SetCurrentGroup(const_cast<char *> ("Group Window"));
    Cset_Register(group_font_size,			0 );
    Cset_Register(group_toolbar,			Cset_change_clamp01 );
    Cset_Register(group_mode,				Cset_change_clamp01 );

    // clippers
    Cset_SetCurrentGroup(const_cast<char *> ("Clippers"));
    Cset_Register(animate_clip_points,		Cset_change_clamp01 );
    Cset_Register(undo_clipper,			Cset_change_clamp01 );
    Cset_Register(clipper_show_outline,	Cset_change_clamp01 );
    Cset_Register(track_clippers,			Cset_change_clamp012 );
    Cset_Register(clipper_autoflip,		Cset_change_clamp01 );
    Cset_Register(clipper_mode,			Cset_change_clamp012 );
    Cset_Register(clippers_stay,			Cset_change_clamp01 );
    Cset_Register(querysplits,				Cset_change_clamp012 );

    // cameras
    Cset_SetCurrentGroup(const_cast<char *> ("XY Camera"));
    Cset_Register(show_camera_angle,		Cset_change_clamp01 );
    Cset_Register(show_cameras,			Cset_change_clamp01 );
    Cset_Register(camera_angle_length,		Cset_change_camera_angle_length );
    Cset_Register(lock_cameras,			Cset_change_clamp01 );
    Cset_Register(lock_camera_offset,		0 );
    Cset_Register(camera_size,				0 );
    Cset_Register(camera_font_size,		0 );

    // editing
    Cset_SetCurrentGroup(const_cast<char *> ("Editing"));
    Cset_Register(clone_delta_x,			0 );
    Cset_Register(clone_delta_y,			0 );
    Cset_Register(clone_delta_disable,		Cset_change_clamp01 );
    Cset_Register(snap_back,				Cset_change_clamp01 );
    Cset_Register(look_at_sel,				Cset_change_clamp01 );
    Cset_Register(new_brush_depth,			0 );
    Cset_Register(extrude_depth,			0 );
    Cset_Register(disable_rotation_texlock,Cset_change_clamp01 );
    Cset_Register(edge_use_grid,			Cset_change_clamp012 );
    Cset_Register(multi_select,			Cset_change_clamp01 );
    Cset_Register(max_face_gap,			Cset_change_max_face_gap );
    Cset_Register(min_epsilon,				Cset_change_min_epsilon );
    Cset_Register(max_epsilon,				Cset_change_max_epsilon );
    Cset_Register(max_normal_delta,		Cset_change_max_normal_delta );
    Cset_Register(max_point_distance,		Cset_change_max_point_distance );
    Cset_Register(on_plane_epsilon,		Cset_change_on_plane_epsilon );
    Cset_Register(multi_face_normal_eps,	Cset_change_multi_face_normal_eps );
    Cset_Register(multi_face_distance_eps,	Cset_change_multi_face_distance_eps );
    Cset_Register(vertex_select_mode,		Cset_change_clamp012 );
    Cset_Register(vertex_drag_sensitivity,	0 );
    Cset_Register(face_drag,				Cset_change_face_drag );
    Cset_Register(select_delta,			Cset_change_select_delta );
    Cset_Register(vertex_use_grid,			Cset_change_clamp01 );
    Cset_Register(distance_epsilon,		0 );

    // entity
    Cset_SetCurrentGroup(const_cast<char *> ("Entity"));
    Cset_Register(entity_button_size,		0 );
    Cset_Register(entity_font_size,		0 );
    Cset_Register(entity_window_width,		0 );
    Cset_Register(entity_comment_size,		0 );
    Cset_Register(entity_keypair_size,		0 );
    Cset_Register(entity_inline_help,		Cset_change_clamp01 );

    // general
    Cset_SetCurrentGroup(const_cast<char *> ("Application"));
    Cset_Register(draw_old_style,			Cset_change_clamp01 );
    Cset_Register(query_wall_width,		Cset_change_clamp01 );
    Cset_RegisterStr(default_gamedir,		Cset_change_default_gamedir );
    Cset_Register(caption_percent,			Cset_change_caption_percent );
    Cset_Register(caption_font_size,		Cset_change_min4 );
    Cset_RegisterStr(caption_font,			Cset_change_font );
    Cset_Register(help_font_size,			Cset_change_min4 );
    Cset_RegisterStr(help_font,			Cset_change_font );
    Cset_Register(nothresholdundo,			0 );
    Cset_Register(undo_threshold,			0 );
    Cset_RegisterStr(bsp_font,				Cset_change_font );
    Cset_Register(bsp_font_size,			Cset_change_min4 );
    Cset_Register(num_recent,				Cset_change_num_recent );
    Cset_Register(console_font_size,		Cset_change_min4 );
    Cset_RegisterStr(console_font,			Cset_change_font );

    // TEXTURE
    Cset_SetCurrentGroup(const_cast<char *> ("Texture"));
    Cset_Register(texture_width,			Cset_change_min4 );
    Cset_Register(texture_height,			Cset_change_min4 );
    Cset_Register(stretch_textures,		Cset_change_clamp01 );
    Cset_Register(show_favorites,			Cset_change_clamp01 );
    Cset_Register(show_frequent,			Cset_change_clamp01 );
    Cset_Register(tex_numfav,				Cset_change_clamp1_10 );
    Cset_Register(tex_numfrq,				Cset_change_clamp1_10 );
    Cset_Register(height_fav,				Cset_change_min4 );
    Cset_Register(height_frq,				Cset_change_min4 );
    Cset_Register(show_tex_names,			Cset_change_clamp01 );
    Cset_Register(texture_alignment,		Cset_change_clamp01 );
    Cset_Register(texture_list_width,		Cset_change_texture_width_list );
    Cset_Register(nosmallstretch,			Cset_change_clamp01 );
    Cset_Register(tex_animate,				Cset_change_min4 );
    Cset_Register(tex_timerdelay,			Cset_change_min4 );
    Cset_Register(show_texturedrag,		Cset_change_clamp01 );
    Cset_Register(tex_small_height,		Cset_change_min4 );
    Cset_Register(tex_small_width,			Cset_change_min4 );
    Cset_Register(tex_animatesky,			Cset_change_clamp01 );
    Cset_Register(tex_skyspeed,			Cset_change_min1 );
    Cset_Register(autocalc_frequents,		Cset_change_clamp01 );
    Cset_Register(texlist_size,			Cset_change_texlist_size );
    Cset_Register(tex_shiftstep,			Cset_change_tex_shiftstep );
    Cset_Register(tex_rotatestep,			Cset_change_tex_rotatestep );
    Cset_Register(tex_scalestep,			Cset_change_tex_scalestep );

    // runmode
    Cset_SetCurrentGroup(const_cast<char *> ("Physics"));
    Cset_Register(run_mode,				Cset_change_clamp01 );

    // GAME.CFG
    Cset_SetCurrentGroup(const_cast<char *> ("Game"));
    Cset_Register(game,					0 );
    Cset_RegisterStr(default_mapfile,		Cset_change_default_mapfile );
    Cset_RegisterStr(entity_directory,		Cset_change_entity_directory );
    Cset_RegisterStr(entity_file,			Cset_change_entity_file );
    Cset_RegisterStr(texture_path,			Cset_change_texture_path );
    Cset_RegisterStr(pak_file,				Cset_change_pak_file );
    Cset_RegisterStr(wad_directory,		Cset_change_wad_directory );
    Cset_RegisterStr(default_wad,			Cset_change_default_wad );
    Cset_RegisterStr(map_directory,		Cset_change_map_directory );
    Cset_RegisterStr(bat_directory,		Cset_change_bat_directory );
    Cset_RegisterStr(palette_file,			Cset_change_palette_file );
    Cset_RegisterStr(model_file,			Cset_change_model_file );
    Cset_Register(sort_entities,			Cset_change_clamp01 );
    Cset_Register(no_models,				Cset_change_clamp01 );
    Cset_Register(gamma,					Cset_change_gamma );
    Cset_Register(world_minmax,			Cset_change_world_minmax );
    Cset_Register(fake_colormap,			Cset_change_clamp01 );
    Cset_RegisterStr(apptitle,				Cset_change_apptitle );
    Cset_Register(gl_highres,				Cset_change_gl_highres );
    Cset_RegisterStrPtr(gl_highres_path,	Cset_change_gl_highres_path );

    // DISPLAY
    Cset_SetCurrentGroup(const_cast<char *> ("Display"));
    Cset_Register(animate_models,			Cset_change_clamp01 );
    Cset_Register(draw_models,				Cset_change_clamp01 );
    Cset_Register(texture_models,			Cset_change_clamp01 );
    Cset_Register(gridsize,				Cset_change_gridsize );
    Cset_Register(wire_ents,				Cset_change_clamp01 );
    Cset_Register(use_crosshair,			Cset_change_clamp01 );
    Cset_Register(outline_use_group,		Cset_change_clamp01 );
    Cset_Register(select_delay,			0 );
    Cset_Register(drag_brushes,			0 );
    Cset_Register(scale,					Cset_change_scale );
    Cset_Register(steps_per_turn,			0 );
    Cset_Register(stepsize,				Cset_change_stepsize );
    Cset_Register(mlook_vert_step,			0 );
    Cset_Register(mlook_horiz_step,		0 );
    Cset_Register(xy_drag_move,			Cset_change_clamp01 );
    Cset_Register(rotate_step,				0 );
    Cset_Register(grid_smallmajor,			Cset_change_clamp01 );
    Cset_Register(grid_style,				Cset_change_clamp01 );
    Cset_Register(angle_control,			Cset_change_angle_control );
    Cset_Register(show_grid,				Cset_change_clamp01 );
    Cset_Register(show_coordinates,		Cset_change_clamp01 );
    Cset_Register(show_names,				Cset_change_clamp01 );
    Cset_Register(show_hits,				Cset_change_clamp01 );
    Cset_Register(show_eye,				Cset_change_clamp01 );
    Cset_Register(show_3d_grid,			Cset_change_clamp01 );
    Cset_Register(show_world_axes,			Cset_change_clamp01 );
    Cset_Register(fov,						Cset_change_fov );
    Cset_Register(flymode,					Cset_change_clamp01 );
    Cset_Register(zero_crosshair,			Cset_change_clamp012 );
    Cset_Register(ghosts_3d,				Cset_change_clamp01 );
    Cset_Register(ghosts_xy,				Cset_change_clamp01 );
    Cset_Register(track3dinxy,				Cset_change_clamp01 );
    Cset_Register(trackxyin3d,				Cset_change_clamp01 );
    Cset_Register(track3dincurrentxy,		Cset_change_clamp01 );
    Cset_Register(warn_spaces,				Cset_change_clamp01 );
    Cset_Register(far_clip_distance,		0 );
    Cset_Register(near_clip_distance,		0 );
    Cset_Register(render_width,			Cset_change_min4 );
    Cset_Register(render_height,			Cset_change_min4 );
    Cset_Register(render_auto,				Cset_change_render_auto );
    Cset_Register(region_clip_grid,		Cset_change_clamp01 );
    Cset_Register(region_ignore,			Cset_change_clamp012 );
    Cset_Register(color_wire,				Cset_change_clamp01 );
    Cset_Register(cull_wire,				Cset_change_clamp01 );
    Cset_Register(selection_thickness,		Cset_change_min1 );
    Cset_Register(flats_like_texture,		Cset_change_clamp01 );
    Cset_Register(render_trans,			Cset_change_clamp012 );
    Cset_Register(outline,					Cset_change_clamp01 );
    Cset_Register(center_knobs_xy,			Cset_change_clamp01 );
    Cset_Register(center_knobs_use_x,		Cset_change_clamp01 );
    Cset_Register(center_knobs_scale,		0 );
    Cset_Register(center_knobs_3d,			Cset_change_clamp01 );
    Cset_Register(facelighting,			Cset_change_clamp01 );
    Cset_Register(lightintensity,			Cset_change_clamp0_63f );
    Cset_Register(ambientintensity,		Cset_change_clamp0_63f );
    Cset_Register(shadeflats,				Cset_change_clamp01 );
    Cset_Register(shadetextures,			Cset_change_clamp01 );
    Cset_Register(flatlevel,				Cset_change_clamp0_63 );
    Cset_Register(drawmode,				Cset_change_clamp012 );
}

