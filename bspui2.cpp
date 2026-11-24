#include "global.h"

int 			leakPtType 			= 0;  // 0 = .pts 1 = .lin
vec3_t 			*aLeakPoints 		= NULL; // leak points...
int 			numLeakPoints 		= 0;
leakPortal_t 	*aLeakPortals 		= NULL; // leak portals...
leakNode_t 		*aLeakNode 			= NULL;
vec3_t 			LeakPortalOrigin;
int 			numLeakPortals 	= 0;

int plugPortal = -1;

/*
=================
SegmentOnSegement

Determine whether two line semgents overlap.
=================
*/
bool SegmentOnSegment(float *s1, float *s2, float *t1, float *t2)
{
// test t1->t2 against s1->s2

    vec3_t dirs;
    vec3_t dirt;
    vec3_t flipt;

    VectorSubtract(s2,s1,dirs);   //
    VectorSubtract(t2,t1,dirt);

    VectorNormalize(dirs);
    VectorNormalize(dirt);

    VectorSubtract(vec3_origin,dirt,flipt);  // Flipped unit vector

    // The unit vectors must line up (same dir. or opposite).
    if (!VectorCompare(dirs,flipt) && !VectorCompare(dirs,dirt))
        return false;

    float d1 = DotProduct(s1,dirs);  // dist to start of s
    float d2 = DotProduct(s2,dirs);  // dist to end of s

    float dist1 = DotProduct (t1, dirs) - d1;
    float dist2 = DotProduct (t2, dirs) - d1;

    if (dist1 < 0 || dist2 < 0)
        return false;

    dist1 = DotProduct (t1, dirs) - d2;
    dist2 = DotProduct (t2, dirs) - d2;
    if (dist1 > 0 || dist2 > 0)
        return false;

    return true;
}

/*
=================
CmMixed

Search all brushes for any with
mixed face contents and select those.
=================
*/
void CmMixed()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    if (!m)
        return;

    // loop through them...
    int numMixed = m->MixedFaceSearch();

    set.redrawedit = 1;
    set.redrawxy = 1;
    if (numMixed >= 1)
    {
        MessageBox(client->hwnd, "Brushes with mixed face contents found and selected!", "BSP", MB_OK);
        char outstr[80];
        sprintf(outstr,"[%i] mixed face brushes found and selected...",numMixed);
        Show_Frame(outstr,true);
    }
    else
    {
        Show_Frame("No mixed face contents brushes found...",true);
    }
}
void CmShowPalette()
{
    if(!palettewindow || !palettewindow->IsValid())
    {
        delete palettewindow;
        palettewindow = new PaletteWindow(client->hwnd, "Palette");
        palettewindow->Create(false);
    }
    if (!palettewindow->IsVisible())
        palettewindow->ShowWindow();
    BringWindowToTop(palettewindow->hwnd);
}
/*
=================
CmTexSearch

Search all brushes for texture
=================
*/
void CmTexSearch()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    if (!m)
        return;

    char texture[LUMP_NAME_LENGTH_MAX] = "";
    bool ok = (InputDialog(client->hwnd, const_cast<char *> ("Texture Search"),
                           const_cast<char *> ("Enter name of texture to find"),
                           texture, sizeof(texture)).Execute() == IDOK);

    if(!ok)
        return;

    // loop through them...
    int numMixed = m->TextureSearch(texture);

    set.redrawedit = 1;
    set.redrawxy = 1;
    if (numMixed >= 1)
    {
        //MessageBox(client->hwnd, "Brushes with mixed face contents found and selected!", "BSP", MB_OK);
        char outstr[80];
        sprintf(outstr,"[%i] brushes found with texture and selected...",numMixed);
        Show_Frame(outstr,true);
    }
    else
    {
        Show_Frame("Texture not found...",true);
    }
}
/*
=================
CmSetScale

Sets the scaling for refreshing the 2d views.
=================
*/
void CmSetScale()
{
    if (!set.Map_Read)
        return;

    bool ok;

    char buffer[32];
    sprintf(buffer,"%5.1f",100.0*set.scale);

    ok = (InputDialog(client->hwnd, const_cast<char *> ("Set scale..."), const_cast<char *> ("Enter new scale %%...(1 to 1600)"), buffer, sizeof(buffer)).Execute() == IDOK);
    if (ok)
    {
        if (buffer[0])
        {
            // okay, do it...
            float test;

            test = (float) atof(buffer);
            test = max(1.0f,test);
            test = min(1600.0f,test);

            set.scale = test/100.0f;

            // set.redrawedit = 1;
            set.redrawxy = 1;
            char outstr[80];
            sprintf(outstr,"Scale set to %5.1f...",set.scale);
            Show_Frame(outstr,true);
        }
    }
}

/*
=================
HideAlLWindows

Hide editing windows when no map is open.
=================
*/
void TBSPWindow::HideAllWindows()
{
    for (int i =0 ; i < set.xyViews; i++)
        if (xyWindow[i])
            xyWindow[i]->ShowWindow(SW_HIDE);
    if (texWindow)
        texWindow->ShowWindow(SW_HIDE);
    if (editWindow)
        editWindow->ShowWindow(SW_HIDE);
    if (kpWindow)
        kpWindow->ShowWindow(SW_HIDE);
    if (groupWindow)
        groupWindow->ShowWindow(SW_HIDE);
    if (surfaceWindow)
        surfaceWindow->ShowWindow(SW_HIDE);
    if (wconsole)
        wconsole->ShowWindow(SW_HIDE);
}

void TBSPWindow::SetWindowPositions()
{
    WindowPlacement::LoadPositions(set.curCfg);
}

/*
=================
ShowAllWindows

=================
*/
void TBSPWindow::ShowAllWindows()
{
    if (texWindow)
        texWindow->ShowWindow(SW_SHOW);
    for (int i =0 ; i < set.xyViews; i++)
    {
        if (xyWindow[i])
            xyWindow[i]->ShowWindow(SW_SHOW);
    }
    if (editWindow)
        editWindow->ShowWindow(SW_SHOW);
    if (kpWindow)
        kpWindow->ShowWindow(SW_SHOW);
    if (groupWindow)
        groupWindow->ShowWindow(SW_SHOW);
    // not show surface?
    // not show console?
}

/*
=================
CmNoop.

No Operation
=================
*/
void CmNoop()
{
}

/*
=================
CmToggleConnections.

Show target/targename arrows.
=================
*/
void CmToggleConnections()
{
    set.show_connections ^= 1;
    if (set.Map_Read)
    {
        set.redrawxy = 1;
        Show_Frame("Connections toggled...",true);
    }
}

/*
=================
CmSelAllHit

Select all brushes that were hit by last
selection ray.
=================
*/
void CmSelAllHit()
{
    if (!set.Map_Read)
        return;

    SelectAllHit();

    set.redrawedit = 1;
    set.redrawxy = 1;

    Show_Frame("Selecting all hit brushes...",1);
}

/*
=================
Cm180

Turn around and refresh screen.
=================
*/
void Cm180()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    m->angles[m->cureye].yaw += M_PI;
    if (m->angles[m->cureye].yaw > 2*M_PI)
        m->angles[m->cureye].yaw-=2*M_PI;

    c_noupdate = true;
    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Turning around...",1);
}

/*
=================
CmMoveUp

Move Up
=================
*/
void CmMoveUp()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    m->eye[m->cureye][2] += set.stepsize;

    c_noupdate = true;
    set.redrawedit = 2;
    set.redrawxy = 2;
    m->MoveEyePosition(m->eye[m->cureye],set.lock_cameras);
    m->lookAtSelection();
    Show_Frame("Moving up...",1);
}

void CmMoveDown()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    m->eye[m->cureye][2] -= set.stepsize;

    c_noupdate = true;
    set.redrawedit = 2;
    set.redrawxy = 2;
    m->MoveEyePosition(m->eye[m->cureye],set.lock_cameras);
    m->lookAtSelection();
    Show_Frame("Moving down",1);
}

/*
=================
CmResetRun

Physics.
=================
*/
void CmResetRun()
{
    set.forward_velocity    = 0.0f;
    set.side_velocity       = 0.0f;
    set.vertical_velocity   = 0.0f;
}

// -1 backward, 0 no change, 1 forward...
/*
=================
CmImpulse

Add some velocity.
=================
*/
void CmImpulse(int f, int s, int v)
{
    set.forward_velocity    += f * set.impulse_forward;
    set.side_velocity       += s * set.impulse_side;
    set.vertical_velocity   += v * set.impulse_vertical;

    // adjust vertical for gravity...
    // Upper bounds
    set.forward_velocity  = min(set.forward_velocity,set.max_forward_velocity);
    set.side_velocity     = min(set.side_velocity,set.max_side_velocity);
    set.vertical_velocity = min(set.vertical_velocity,set.max_vertical_velocity);

    // Lower bounds...
    set.forward_velocity  = max(set.forward_velocity,-set.max_forward_velocity);
    set.side_velocity     = max(set.side_velocity,-set.max_side_velocity);
    set.vertical_velocity = max(set.vertical_velocity,-set.max_vertical_velocity);
}

/*
=================
CmRunMode

Turn physics on/off
=================
*/
void CmRunMode()
{
    set.run_mode ^= 1;
    CmResetRun();
    set.redrawedit = 2;
    set.redrawxy = 2;
    Show_Frame("Run mode set...",true);
}
/*
toggle flymode
*/
void CmToggleFlymode()
{
    set.flymode ^= 1;
    Show_Frame("Fly mode toggled...",false);
}
/*
=================
IdleAction

Idle processing.
=================
*/
bool TBSPWindow::IdleAction(long idleCount)
{
    // First idle message after startup is place to load command line specified file.
    /*	todo: autoload
    	if (BSP_AutoLoad)
    	{   // If file exists, then load it and show windows
    		BSP_AutoLoad = false;
    		AutoLoad(set.szAutoLoadFile);
       		return TWindow::IdleAction(idleCount);
    	}*/
    if (!set.Map_Read || !BSP_Active)
        return 0;
    map *m = map_i[set.curmap];

    //
    // RUNMODE MOVEMENT
    //

    // need redraw...
    if ((fabs(set.forward_velocity) < 0.01f) &&
            (fabs(set.side_velocity) < 0.01f) &&
            (fabs(set.vertical_velocity) < 0.01f))
    {
        return 0;
    }

    // move...
    // just forward for now...
    m->eye[m->cureye][0] += set.forward_velocity*cos(m->angles[m->cureye].yaw-M_PI/2.0f);
    m->eye[m->cureye][1] += set.forward_velocity*sin(m->angles[m->cureye].yaw-M_PI/2.0f);

    // and side...
    m->eye[m->cureye][0] += set.side_velocity*cos(m->angles[m->cureye].yaw-M_PI);
    m->eye[m->cureye][1] += set.side_velocity*sin(m->angles[m->cureye].yaw-M_PI);

    // redraw it...
    set.redrawedit = 2;
    set.redrawxy = 2;
    m->MoveEyePosition(m->eye[m->cureye],set.lock_cameras);

    Show_Frame("",true);

    if (set.forward_velocity > 0.01f)
    {
        set.forward_velocity -= set.forward_attenuation;
        if (set.forward_velocity < 0.0f)
            set.forward_velocity = 0.0f;
    }
    else
    {
        set.forward_velocity += set.forward_attenuation;
        if (set.forward_velocity > 0.0f)
            set.forward_velocity = 0.0f;
    }

    if (set.side_velocity > 0.01f)
    {
        set.side_velocity -= set.side_attenuation;
        if (set.side_velocity < 0.0f)
            set.side_velocity = 0.0f;
    }
    else
    {
        set.side_velocity += set.side_attenuation;
        if (set.side_velocity > 0.0f)
            set.side_velocity = 0.0f;
    }

    if (set.vertical_velocity > 0.01f)
    {
        set.vertical_velocity -= set.gravity;
        if (set.vertical_velocity < 0.0f)
            set.vertical_velocity = 0.0f;
    }
    else
    {
        set.vertical_velocity += set.gravity;
        if (set.vertical_velocity > 0.0f)
            set.vertical_velocity = 0.0f;
    }
    return 0;
}

/*
=================
CmForward

Forward
=================
*/
void CmForward()
{
    if (!set.Map_Read)
        return;

    if (set.run_mode == Runmode::no_run)
    {
        map *m = map_i[set.curmap];

        if(set.flymode)
        {
            float norm = -sin(m->angles[m->cureye].pitch-M_PI/2.0f);

            m->eye[m->cureye][0] += norm*set.stepsize * cos(m->angles[m->cureye].yaw-M_PI/2.0f);
            m->eye[m->cureye][1] += norm*set.stepsize * sin(m->angles[m->cureye].yaw-M_PI/2.0f);

            m->eye[m->cureye][2] += set.stepsize*cos(m->angles[m->cureye].pitch-M_PI/2.0f);
        }
        else
        {
            m->eye[m->cureye][0] += set.stepsize*cos(m->angles[m->cureye].yaw-M_PI/2.0f);
            m->eye[m->cureye][1] += set.stepsize*sin(m->angles[m->cureye].yaw-M_PI/2.0f);
        }

        //TODO: fix this. :(	if(!mouse_is_navigating)
        set.redrawedit = 2;
        set.redrawxy = 2;
        m->MoveEyePosition(m->eye[m->cureye],set.lock_cameras);
        Show_Frame("Forward...",1);
    }
    else
    {
        CmImpulse(1,0,0);
    }
    c_noupdate = true;
}

void CmBack()
{
    if (!set.Map_Read)
        return;

    if (set.run_mode == Runmode::no_run)
    {
        map *m = map_i[set.curmap];

        if(set.flymode)
        {
            float norm = -sin(m->angles[m->cureye].pitch-M_PI/2.0f);

            m->eye[m->cureye][0] -= norm*set.stepsize * cos(m->angles[m->cureye].yaw-M_PI/2.0f);
            m->eye[m->cureye][1] -= norm*set.stepsize * sin(m->angles[m->cureye].yaw-M_PI/2.0f);

            m->eye[m->cureye][2] -= set.stepsize*cos(m->angles[m->cureye].pitch-M_PI/2.0f);
        }
        else
        {
            m->eye[m->cureye][0] -= set.stepsize*cos(m->angles[m->cureye].yaw-M_PI/2.0f);
            m->eye[m->cureye][1] -= set.stepsize*sin(m->angles[m->cureye].yaw-M_PI/2.0f);
        }

        set.redrawedit = 2;
        set.redrawxy = 2;
        m->MoveEyePosition(m->eye[m->cureye],set.lock_cameras);
        Show_Frame("Backward...",1);
    }
    else
    {
        CmImpulse(-1,0,0);
    }
    c_noupdate = true;
}

void CmMF()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    m->eye[m->cureye][2] -= set.stepsize;


    c_noupdate = true;
    set.redrawedit = 2;
    set.redrawxy = 2;
    m->MoveEyePosition(m->eye[m->cureye],set.lock_cameras);
    m->lookAtSelection();
    Show_Frame("Down",1);
}

void CmMB()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    m->eye[m->cureye][2] += set.stepsize;

    c_noupdate = true;
    set.redrawedit = 2;
    set.redrawxy = 2;
    m->MoveEyePosition(m->eye[m->cureye],set.lock_cameras);
    m->lookAtSelection();
    Show_Frame("Up",1);
}

/*
=================
CmRight

Turn right.
=================
*/
void CmRight()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    m->angles[m->cureye].yaw -= set.angstep;
    if (m->angles[m->cureye].yaw < 0.0)
        m->angles[m->cureye].yaw += 2*M_PI;

    c_noupdate = true;
    set.redrawedit = 2;
    set.redrawxy = 2;
    Show_Frame("Right",1);
}

void CmLeft()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    m->angles[m->cureye].yaw += set.angstep;
    if (m->angles[m->cureye].yaw > 2*M_PI)
        m->angles[m->cureye].yaw -= 2*M_PI;

    c_noupdate = true;
    set.redrawedit = 2;
    set.redrawxy = 2;
    Show_Frame("Left",1);
}

/*
=================
CmPitchDown

Look down.
=================
*/
void CmPitchDown()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    m->angles[m->cureye].pitch -= set.angstep;
    if (m->angles[m->cureye].pitch < 0.0)
        m->angles[m->cureye].pitch += 2*M_PI;

    c_noupdate = true;
    set.redrawedit = 1;
    set.redrawxy = 2;
    Show_Frame("Pitching down...",1);
}

void CmPitchUp()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    m->angles[m->cureye].pitch += set.angstep;
    if (m->angles[m->cureye].pitch > 2*M_PI)
        m->angles[m->cureye].pitch -= 2*M_PI;

    c_noupdate = true;
    set.redrawedit = 1;
    set.redrawxy = 2;
    Show_Frame("Pitching up...",1);
}

/*
=================
CmMagDown

Change magnification
=================
*/
void CmMagDown()
{
    if (!set.Map_Read)
        return;

    set.mag *= 0.9f;
    set.mag = max(0.25f,set.mag);

    set.redrawedit = 1;

    char outstr[80];
    sprintf(outstr,"mag:  %04.2f",set.mag);
    Show_Frame(outstr,1);
}

void CmMagUp()
{
    if (!set.Map_Read)
        return;

    set.mag *= 1.1f;
    set.redrawedit = 1;

    char outstr[80];
    sprintf(outstr,"mag:  %04.2f",set.mag);
    Show_Frame(outstr,1);
}

/*
=================
CmPPDDown

Reduce perspective projection plane distance.
Still used?
=================
*/
void CmPPDDown()
{
    if (!set.Map_Read)
        return;

    char outstr[80];

    if (set.ppd > 0.25)
        set.ppd *= 0.5;
    sprintf(outstr,"ppd: %04.2f",set.ppd);

    set.redrawedit = 1;

    Show_Frame(outstr,1);
}

void CmPPDUp()
{
    if (!set.Map_Read)
        return;

    char outstr[80];
    if (set.ppd < 1024)
        set.ppd *= 2.0;
    sprintf(outstr,"ppd: %04.2f",set.ppd);

    set.redrawedit = 1;

    Show_Frame(outstr,1);
}

/*
=================
CmStepDown

Change movement step size.
=================
*/
void CmStepDown()
{
    if (!set.Map_Read)
        return;

    char outstr[80];
    if (set.stepsize >= 2.0)
        set.stepsize /= 2.0;
    sprintf(outstr,"step: %04.2f",set.stepsize);

    Show_Frame(outstr,0);
}

void CmStepUp()
{
    if (!set.Map_Read)
        return;

    char outstr[80];

    if (set.stepsize < 1024.0)
        set.stepsize *= 2.0;
    sprintf(outstr,"step: %04.2f",set.stepsize);

    Show_Frame(outstr,0);
}

/*
=================
CmClipIn

Change far Z plane distance.
=================
*/
void CmClipIn()
{
    if (!set.Map_Read)
        return;

    char outstr[80];
    set.far_clip_distance += set.stepsize;
    if (set.far_clip_distance >= -2.0)
        set.far_clip_distance = -2.0;
    sprintf(outstr,"zmax: %04.2f",set.far_clip_distance);

    Show_Frame(outstr,0);
}

void CmClipOut()
{
    if (!set.Map_Read)
        return;

    char outstr[80];
    set.far_clip_distance -= set.stepsize;
    if (set.far_clip_distance <= -16384.0)
        set.far_clip_distance = -16384.0;
    sprintf(outstr,"zmax: %04.2f",set.far_clip_distance);

    Show_Frame(outstr,0);
}

void CmLookAtSel()
{
    set.look_at_sel = !set.look_at_sel;
    if(set.look_at_sel && set.Map_Read)
    {
        map_i[set.curmap]->lookAtSelection();
        set.redrawedit = 1;
        set.redrawxy = 2;
    }
    Show_Frame("Look at Selection toggled...",true);
}
/*
=================
CmToggleMode

Cycle between wire/flat/texture mode.
=================
*/
void CmToggleMode()
{
    if (!set.Map_Read)
        return;

    set.redrawedit = 1;
    set.redrawxy = 1;

    switch (set.drawmode)
    {
    case Drawmode::wire:
        set.drawmode = Drawmode::flat;
        Show_Frame("Mode set to Flat",1);
        break;
    case Drawmode::flat:
        set.drawmode = Drawmode::texture;
        Show_Frame("Mode set to Texture",1);
        break;
    default:
        set.drawmode = Drawmode::wire;
        Show_Frame("Mode set to Wire",1);
        break;
    }
}
/*
=================
CmKP2

Keypad 2 key (move selection down)
=================
*/
void CmKP2()
{
    if (!set.Map_Read)
        return;

    int v = 1;
    if (xyWindow[set.curxy])
        v = xyWindow[set.curxy]->VAxis;

    VectorCopy (vec3_origin, sb_translate);
    sb_translate[v] = (float) -set.gridsize;
    map_i[set.curmap]->makeSelectedPerform(SEL_TRANSLATE);

    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Selection moved down...",1);
}

void CmKP4()
{
    if (!set.Map_Read)
        return;

    int u = 0;
    if (xyWindow[set.curxy])
        u = xyWindow[set.curxy]->UAxis;

    VectorCopy (vec3_origin, sb_translate);
    sb_translate[u] = (float) -set.gridsize;
    map_i[set.curmap]->makeSelectedPerform(SEL_TRANSLATE);

    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Selection moved left...",1);
}

void CmKP6()
{
    if (!set.Map_Read)
        return;

    int u = 0;
    if (xyWindow[set.curxy])
        u = xyWindow[set.curxy]->UAxis;

    VectorCopy (vec3_origin, sb_translate);
    sb_translate[u] = (float) set.gridsize;
    map_i[set.curmap]->makeSelectedPerform(SEL_TRANSLATE);

    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Selection moved right...",1);
}

void CmKP8()
{
    if (!set.Map_Read)
        return;

    int v = 1;
    if (xyWindow[set.curxy])
        v = xyWindow[set.curxy]->VAxis;

    VectorCopy (vec3_origin, sb_translate);
    sb_translate[v] = (float) set.gridsize;
    map_i[set.curmap]->makeSelectedPerform(SEL_TRANSLATE);

    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Selection moved up...",1);
}

/*
==================
CmMinus

Move to lower Z value.
==================
*/
void CmMinus()
{
    if (!set.Map_Read)
        return;

    set.redrawedit = 1;
    set.redrawxy = 1;

    int n = 2;

    if (xyWindow[set.curxy])
        n = xyWindow[set.curxy]->NormalAxis;

    VectorCopy (vec3_origin, sb_translate);
    sb_translate[n] = (float) -set.gridsize;
    map_i[set.curmap]->makeSelectedPerform(SEL_TRANSLATE);
    Show_Frame("Selection moved in...",1);
}

/*
==================
CmPlus

Move to higher Z value.
==================
*/
void CmPlus()
{
    if (!set.Map_Read)
        return;

    int n = 2;

    if (xyWindow[set.curxy])
        n = xyWindow[set.curxy]->NormalAxis;

    set.redrawedit = 1;
    set.redrawxy = 1;

    VectorCopy (vec3_origin, sb_translate);
    sb_translate[n] = (float) set.gridsize;
    map_i[set.curmap]->makeSelectedPerform(SEL_TRANSLATE);

    Show_Frame("Selection moved out...",1);
}

/*
==================
CmMoveLeft

Move left
==================
*/
void CmMoveLeft()
{
    if (!set.Map_Read)
        return;

    if (set.run_mode == Runmode::no_run)
    {
        map *m = map_i[set.curmap];
        bool circlestrafe = (set.look_at_sel && m->numSelected() > 0);

        if(circlestrafe)
        {
            m->getSelectedCenter();
            vec3_t v;
            VectorSubtract(m->eye[m->cureye], sb_ctr, v);
            float dist = sqrt(v[0]*v[0] + v[1]*v[1]);		//xy distance from cam to sel

            float stepsize = (2*M_PI/set.stepsize) / 4;	//old stepsize was treated per quadrant

            m->eye[m->cureye][0] = sb_ctr[0] + dist * sin((m->angles[m->cureye].yaw-M_PI) - stepsize);
            m->eye[m->cureye][1] = sb_ctr[1] + dist * -cos((m->angles[m->cureye].yaw-M_PI) - stepsize);

            set.redrawedit = 2;
            set.redrawxy = 2;

            m->MoveEyePosition(m->eye[m->cureye],set.lock_cameras);

            m->lookAtSelection(true);

        }
        else
        {
            m->eye[m->cureye][0] -= set.stepsize*cos(m->angles[m->cureye].yaw-M_PI);
            m->eye[m->cureye][1] -= set.stepsize*sin(m->angles[m->cureye].yaw-M_PI);

            set.redrawedit = 2;
            set.redrawxy = 2;

            m->MoveEyePosition(m->eye[m->cureye],set.lock_cameras);
        }

        Show_Frame("Strafe Left...",1);
    }
    else
    {
        CmImpulse(0,-1,0);
    }
}

/*
==================
CmMoveRight

Move right
==================
*/
void CmMoveRight()
{
    if (!set.Map_Read)
        return;

    if (set.run_mode == Runmode::no_run)
    {
        map *m = map_i[set.curmap];
        bool circlestrafe = (set.look_at_sel && m->numSelected() > 0);

        if(circlestrafe)
        {
            m->getSelectedCenter();
            vec3_t v;
            VectorSubtract(m->eye[m->cureye], sb_ctr, v);
            float dist = sqrt(v[0]*v[0] + v[1]*v[1]);		//xy distance from cam to sel

            float stepsize = (2*M_PI/set.stepsize) / 4;	//old stepsize was treated per quadrant

            m->eye[m->cureye][0] = sb_ctr[0] + dist * sin((m->angles[m->cureye].yaw-M_PI) + stepsize);
            m->eye[m->cureye][1] = sb_ctr[1] + dist * -cos((m->angles[m->cureye].yaw-M_PI) + stepsize);

            set.redrawedit = 2;
            set.redrawxy = 2;

            m->MoveEyePosition(m->eye[m->cureye],set.lock_cameras);

            m->lookAtSelection(true);
        }
        else
        {

            m->eye[m->cureye][0] += set.stepsize*cos(m->angles[m->cureye].yaw-M_PI);
            m->eye[m->cureye][1] += set.stepsize*sin(m->angles[m->cureye].yaw-M_PI);

            set.redrawedit = 2;
            set.redrawxy = 2;

            m->MoveEyePosition(m->eye[m->cureye],set.lock_cameras);
        }
        Show_Frame("Strafe Right...",1);
    }
    else
    {
        CmImpulse(0,1,0);
    }
}

/*
==================
CmGrid1

Set grid size
==================
*/
void Grid1()
{
    set.gridsize = 1;
    set.redrawxy = 1;
    Show_Frame("Grid set to 1",1);
}

void Grid2()
{
    set.gridsize = 2;
    set.redrawxy = 1;
    Show_Frame("Grid set to 2",1);
}

void Grid4()
{
    set.gridsize = 4;
    set.redrawxy = 1;
    Show_Frame("Grid set to 4",1);
}

void Grid8()
{
    set.gridsize = 8;
    set.redrawxy = 1;
    Show_Frame("Grid set to 8",1);
}

void Grid16()
{
    set.gridsize = 16;
    set.redrawxy = 1;
    Show_Frame("Grid set to 16",1);
}

void Grid32()
{
    set.gridsize = 32;
    set.redrawxy = 1;
    Show_Frame("Grid set to 32",1);
}

void Grid64()
{
    set.gridsize = 64;
    set.redrawxy = 1;
    Show_Frame("Grid set to 64",1);
}

void Grid128()
{
    set.gridsize = 128;
    set.redrawxy = 1;
    Show_Frame("Grid set to 128",1);
}

void Grid256()
{
    set.gridsize = 256;
    set.redrawxy = 1;
    Show_Frame("Grid set to 256",1);
}

/*
==================
Scale1_64

Set scale to 1/64
==================
*/
void Scale1_64()
{
    set.scale = 1.0/64.0;

    set.redrawxy = 1;
    Show_Frame("Scale set to 1/64",1);
}

void Scale1_32()
{
    set.scale = 1.0/32.0;

    set.redrawxy = 1;
    Show_Frame("Scale set to 1/32",1);
}

void Scale1_16()
{
    set.scale = 1.0/16.0;

    set.redrawxy = 1;
    Show_Frame("Scale set to 1/16",1);
}

void Scale1_8()
{
    set.scale = 1.0/8.0;

    set.redrawxy = 1;
    Show_Frame("Scale set to 1/8",1);
}

void Scale1_4()
{
    set.scale = 1.0/4.0;

    set.redrawxy = 1;
    Show_Frame("Scale set to 1/4",1);
}

void Scale1_2()
{
    set.scale = 1.0/2.0;

    set.redrawxy = 1;
    Show_Frame("Scale set to 1/2",1);
}

void Scale3_4()
{
    set.scale = 3.0/4.0;

    set.redrawxy = 1;
    Show_Frame("Scale set to 3/4",1);
}

void Scale1()
{
    set.scale = 1.0;

    set.redrawxy = 1;
    Show_Frame("Scale set to 1",1);
}

void Scale2()
{
    set.scale = 2.0;

    set.redrawxy = 1;
    Show_Frame("Scale set to 2",1);
}

void Scale4()
{
    set.scale = 4.0;

    set.redrawxy = 1;
    Show_Frame("Scale set to 4",1);
}

void Scale8()
{
    set.scale = 8.0;

    set.redrawxy = 1;
    Show_Frame("Scale set to 8",1);
}

void Scale16()
{
    set.scale = 16.0;

    set.redrawxy = 1;
    Show_Frame("Scale set to 16",1);
}

/*
==================
ScaleUp

Raise scale
==================
*/
void ScaleUp()
{
    set.scale *= 2.0;
    if (set.scale >= 16.0)
        set.scale = 16.0;
}
void CmScaleUp()
{
    ScaleUp();
    set.redrawxy = 1;
    Show_Frame("Scale increased...",1);
}

/*
==================
ScaleDown


==================
*/
void ScaleDown()
{
    set.scale /= 2.0;
    if (set.scale<= (1.0/64.0))
        set.scale = (1.0/64.0);
}
void CmScaleDown()
{
    ScaleDown();
    set.redrawxy = 1;
    Show_Frame("Scale decreased...",1);
}

/*
==================
GridUp

==================
*/
void GridUp()
{
    if (set.gridsize <=128)
        set.gridsize *= 2;
    set.redrawxy = 1;
    Show_Frame("Grid increased...",1);
}

/*
==================
GridDown

Move to lower Z value.
==================
*/
void GridDown()
{
    if (set.gridsize >= 2)
        set.gridsize /= 2;
    set.redrawxy = 1;
    Show_Frame("Grid decreased...",1);
}

/*
==================
CmSwitchMapView

Change xy, yz, xz for 2d view
==================
*/
void CmSwitchMapView()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    if (xyWindow[set.curxy])
    {
        if (m->clipper_i->num ==2) // so it doesn't try to reset itself...
            m->clipper_i->num = 3;

        switch (xyWindow[set.curxy]->ViewType)
        {
        case Type_XY:
            xyWindow[set.curxy]->ViewType = Type_YZ;
            break;
        case Type_YZ:
            xyWindow[set.curxy]->ViewType = Type_XZ;
            break;
        case Type_XZ:
            xyWindow[set.curxy]->ViewType = Type_XY;
            break;
        }

        GetAxes(xyWindow[set.curxy]->ViewType,
                &xyWindow[set.curxy]->UAxis,
                &xyWindow[set.curxy]->VAxis,
                &xyWindow[set.curxy]->NormalAxis);

        xyWindow[set.curxy]->xy_viewnormal[xyWindow[set.curxy]->UAxis] = 0.0;
        xyWindow[set.curxy]->xy_viewnormal[xyWindow[set.curxy]->VAxis] = 0.0;
        xyWindow[set.curxy]->xy_viewnormal[xyWindow[set.curxy]->NormalAxis] = -1.0;

        set.redrawedit = 1;
        set.redrawxy = 1;
        Show_Frame("Map View Toggled...",true);
    }
}

void W1()
{
    set.curCfg = 0;
    CmWindowPosition();
}
void W2()
{
    set.curCfg = 1;
    CmWindowPosition();
}
void W3()
{
    set.curCfg = 2;
    CmWindowPosition();
}
void W4()
{
    set.curCfg = 3;
    CmWindowPosition();
}

void WUp()
{
    set.curCfg++;
    if (set.curCfg >= MAX_SAVE_WIN) set.curCfg = 0;
    CmWindowPosition();
}
void WDown()
{
    set.curCfg--;
    if (set.curCfg < 0) set.curCfg = MAX_SAVE_WIN-1;
    CmWindowPosition();
}
void CmRevert()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];
    Entity *e = m->current;
    Entity *w = m->world;

    if (e == w) // already at world!
        return;
    if (!e || !w || !e->modifiable)    // no current entity! or no world or current is not modifiable!
        return;

    char *cl;
    char s[256];

    cl = e->classname;

    if (!cl || !*cl)  // entity should have a classname!
        return;

    sprintf(s,"Revert brushes of entity %s to world?",cl);

    int retval = MessageBox(client->hwnd, s, "BSP - Revert", MB_YESNO | MB_ICONQUESTION);
    if (retval == IDYES)
    {
        SetBrush *b, *n;
        // Empty out the current entity
        // Go backwards...
        for (b = e->objects.p_next; b != &e->objects;)
        {
            LoopProblem("Revert");

            n = b->p_next;
            e->removeObject(b);
            b->setParent(w); // change parent to world and add it!
            b->setSelected(true); // make sure it's selected...
            w->addObject(b);
            b = n;
        }
        // now kill off the current entity...
        if (e->owner && e->p_next && e->p_prev)
            e->owner->removeObject(e);
        delete e;

        // make current entity the world!
        m->setCurrentEntity(w);

        set.redrawedit = 1;
        set.redrawxy = 1;
        Show_Frame("Entity reverted...",true);
    }
}

void CmSelectByTexture()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    char buffer[80];
    if (set.game_mode == 2)
        sprintf(buffer,"%s/%s",m->texPath,m->texName);
    else
        STRNCPY(buffer,m->texName);


    bool ok = (InputDialog(client->hwnd, const_cast<char *> ("Select"), const_cast<char *> ("Select Brushes by Texture:"), buffer, sizeof(buffer)).Execute() == IDOK);
    if (ok)
    {
        if (buffer[0])
        {
            // okay, do it...

            char *u = strupr(buffer);
            m->selectByTexture(u);
            set.redrawedit = 1;
            set.redrawxy = 1;
            char s[MAX_PATH];
            sprintf(s,"Selected brushes having texture %s...",buffer);
            Show_Frame(s,true);
        }
    }
}

void CmHitUp()
{
    if (!set.Map_Read)
        return;

    NextHitBrush();
    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Hit brush cycled up...",true);
}

void CmHitDown()
{
    if (!set.Map_Read)
        return;

    PreviousHitBrush();
    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Hit brush cycled down...",true);
}

void CmMergeIn()
{
    if (!set.Map_Read)
        return;

    char filename[256] = "";

    OPENFILENAME ofn;
    memset(&ofn,0,sizeof(OPENFILENAME));
    ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
    ofn.hwndOwner = frame->hwnd;
    ofn.lpstrFile = filename;
    ofn.lpstrFilter = "Merge Files (*.mrg)\0*.mrg\0";
    ofn.lpstrDefExt = "mrg";
    ofn.nMaxFile = 256;
    ofn.lpstrInitialDir = set.map_directory;
    ofn.lpstrTitle = "Select .MRG File";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    if (GetOpenFileName(&ofn))
    {
        MergeMapFile(filename);

        char outstr[80];
        sprintf(outstr,"File [%s] processed...",filename);
        set.redrawedit = 1;
        set.redrawxy = 1;

        Show_Frame(outstr,true);
    }
}

void CmMergeOut()
{
    if (!set.Map_Read)
        return;

    char filename[256] = "";

    OPENFILENAME ofn;
    memset(&ofn,0,sizeof(OPENFILENAME));
    ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
    ofn.hwndOwner = frame->hwnd;
    ofn.lpstrFile = filename;
    ofn.lpstrFilter = "Merge Files (*.mrg)\0*.mrg\0";
    ofn.lpstrDefExt = "mrg";
    ofn.nMaxFile = 256;
    ofn.lpstrInitialDir = set.map_directory;
    ofn.lpstrTitle = "Name .MRG File";
    ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn))
    {
        map_i[set.curmap]->writeMapFile(filename,true,true); // last 2:  writeWorldKeys and selOnly?

        char outstr[80];
        sprintf(outstr,"Merge file [%s] saved...",filename);
        set.redrawedit = 1;
        set.redrawxy = 1;

        Show_Frame(outstr,true);
    }
}

void CmMergeClipboard()
{
    if (!set.Map_Read)
        return;

    if (!copymap || !copymap->count) // empty clipboard...
        return;

    char filename[256] = "";

    OPENFILENAME ofn;
    memset(&ofn,0,sizeof(OPENFILENAME));
    ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
    ofn.hwndOwner = frame->hwnd;
    ofn.lpstrFile = filename;
    ofn.lpstrFilter = "Merge Files (*.mrg)\0*.mrg\0";
    ofn.lpstrDefExt = "mrg";
    ofn.nMaxFile = 256;
    ofn.lpstrInitialDir = set.map_directory;
    ofn.lpstrTitle = "Name .MRG File";
    ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn))
    {
        copymap->writeMapFile(filename,true,true); // last 2:  writeWorldKeys and selOnly?

        char outstr[80];
        sprintf(outstr,"Clipboard merge file [%s] saved...",filename);
        set.redrawedit = 1;
        set.redrawxy = 1;

        Show_Frame(outstr,true);
    };
}

void CmXUp()
{
    XYZMove(0,(int) set.stepsize,set.currentMode);
    Show_Frame("XUp",true);
}
void CmYUp()
{
    XYZMove(1,(int) set.stepsize,set.currentMode);
    Show_Frame("YUp",true);
}
void CmZUp()
{
    XYZMove(2,(int) set.stepsize,set.currentMode);
    Show_Frame("ZUp",true);
}
void CmXDown()
{
    XYZMove(0,(int) -set.stepsize,set.currentMode);
    Show_Frame("XDown",true);
}
void CmYDown()
{
    XYZMove(1,(int) -set.stepsize,set.currentMode);
    Show_Frame("YDown",true);
}
void CmZDown()
{
    XYZMove(2,(int) -set.stepsize,set.currentMode);
    Show_Frame("ZDown",true);
}

void RenModeFunc(int width, int height)
{
    if(set.glBsp)
        return;
    set.render_auto = false;
    set.render_width = width;
    set.render_height = height;

    editWindow->RemoveBuffers();
    editWindow->ConstructBuffers(set.render_width, set.render_height);

    set.redrawedit = 1;
    char outstr[80];
    sprintf(outstr,"Render Quality set to %d x %d",width,height);
    Show_Frame(outstr,true);
}
void R1()
{
    RenModeFunc(80,60);
}
void R2()
{
    RenModeFunc(100,75);
}
void R3()
{
    RenModeFunc(160,80);
}
void R4()
{
    RenModeFunc(200,150);
}
void R5()
{
    RenModeFunc(320,200);
}
void R6()
{
    RenModeFunc(400,300);
}
void R7()
{
    RenModeFunc(480,360);
}
void R8()
{
    RenModeFunc(640,480);
}
void R9()
{
    if(set.glBsp)
        return;
    set.render_auto = true;
    set.auto_init = false;

    set.redrawedit = 1;

    Show_Frame("Render Quality set to Auto",true);
}


void CmTrans0()
{
    set.render_trans = 0;
    set.redrawedit = 1;
    Show_Frame("Translucency disabled",true);
}
void CmTrans1()
{
    set.render_trans = 1;
    set.redrawedit = 1;
    Show_Frame("Scanline translucency",true);
}
void CmTrans2()
{
    set.render_trans = 2;
    set.redrawedit = 1;
    Show_Frame("Dot translucency",true);
}
void CmTransGL()
{
    set.render_trans = set.render_trans ? 0 : 1;
    set.redrawedit = 1;
    Show_Frame("Translucency toggled...",true);
}

void CmRegion()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    m->makeGlobalPerform(SEL_UNSETREGIONED);
    m->makeRegionBrush();

    if (!set.region_mode)
        set.region_mode = 1;

    m->UpdateMapVisibility();

    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Region Set...",true);
}

void CmRegionPartial()
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    m->makeGlobalPerform(SEL_UNSETREGIONED);
    m->makeRegionBrushPartial();

    if (!set.region_mode)
        set.region_mode = 1;

    m->UpdateMapVisibility();

    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Region Set...",true);
}

void CmJump()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    char s[MAX_PATH];

    vec3_t ep;
    VectorCopy(m->eye[m->cureye], ep);

    sprintf(s,"%i %i %i",(int)ep[0],(int)ep[1],(int)ep[2]);

    if (IDOK != InputDialog(client->hwnd, const_cast<char *> ("BSP - Jump To") ,const_cast<char *> ("Jump where ? "), s, sizeof(s)).Execute())
        return;


    if (sscanf(s,"%f %f %f",&ep[0], &ep[1], &ep[2]) != 3)
        return;

    m->MoveEyePosition(ep, 0);

    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Jumped...",true);
}

void CmJumpXY()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    VectorCopy(xy_eye, m->eye[m->cureye]);

    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Jumped to XY Center...",true);
}

void CmJump3D()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    VectorCopy(m->eye[m->cureye], xy_eye);

    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Jumped to 3D Center...",true);
}

void CmLiftHeights()
{

    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];
    if (!m)
        return;

    if (m->numSelected() != 1)
    {
        Show_Frame("Must only have one selected brush...",false);
        return;
    }

    SetBrush *b = m->selectedBrush();
    if (!b)
        return;

    vec3_t minz, maxz;

    b->getMins(minz, maxz);

    m->setCurrentMinZ(minz);
    m->setCurrentMaxZ(maxz);

    Show_Frame("Height and base lifted",false);
}

void CmSetHeights()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];
    if (!m)
        return;

    char outstr[80];
    int axis;
    float cmin, cmax, cheight;
    axis = xyWindow[set.curxy]->NormalAxis;
    cmin = m->currentMinZ(axis);
    cmax = m->currentMaxZ(axis);
    cheight = cmax - cmin;

    sprintf(client->setxfer.Base,"%f",cmin);
    sprintf(client->setxfer.Height,"%f",cheight);
    // Run dialog...
    TSetHeightsDlg dialog(client->hwnd,&client->setxfer);
    if (dialog.Execute() == IDOK)
    {
        cmin = (float) atof(client->setxfer.Base);
        cheight = (float) atof(client->setxfer.Height);
        cheight = max(1.0f, cheight);

        cmax = cmin + cheight;

        vec3_t minz, maxz;

        minz[0] = minz[1] = minz[2] = cmin;
        maxz[0] = maxz[1] = maxz[2] = cmax;

        m->setCurrentMinZ(minz);
        m->setCurrentMaxZ(maxz);

        set.redrawxy = 1;
        set.redrawedit = 1;
        sprintf(outstr,"New height [%f], new base [%f]",cheight,cmin);
    }
    else
    {
        strcpy(outstr,"Couldn't set height/base...");
    }
    Show_Frame(outstr,false);
}

void CmStopPortals()
{
    if (!set.Map_Read)
        return;

    if (!set.track_portals)
        return;

    set.track_portals = 0;
    ::ShowWindow(editWindow->numPortals->hwnd,SW_HIDE);
    ::ShowWindow(editWindow->killPortals->hwnd,SW_HIDE);

    if (aLeakPortals)
    {
        for (int i = 0; i < numLeakPortals; i++)
        {
            delete [] aLeakPortals[i].w;
            aLeakPortals[i].w = NULL;
        };
        delete[] aLeakPortals;
        aLeakPortals = NULL;
        numLeakPortals = 0;
    };

    if (aLeakNode)
    {
        for (int c = 0; c < aLeakNode->numwindings; c++)
        {
            delete [] aLeakNode->w[c];
            aLeakNode->w[c] = NULL;
        };
        delete[] aLeakNode->w;
        delete aLeakNode;
        aLeakNode = NULL;
    };

    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Portal file turned off...",true);
};

void CmStopLeaks()
{
    if (!set.Map_Read)
        return;

    if (!set.track_leaks)
        return;

    set.track_leaks = 0;

    if (aLeakPoints)
    {
        delete[] aLeakPoints;
        aLeakPoints = NULL;
        numLeakPoints = 0;
        leakPtType = 0;
    };

    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Leak file turned off...",true);
};

void CmShowLeaks()
{
    if (!set.Map_Read)
        return;

    if (set.track_leaks || aLeakPoints)
    {
        set.track_leaks = 0;
        delete[] aLeakPoints;
        aLeakPoints = NULL;
        numLeakPoints = 0;
        leakPtType = 0;
    }

    char filename[256] = "";

    OPENFILENAME ofn;
    memset(&ofn,0,sizeof(OPENFILENAME));
    ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
    ofn.hwndOwner = frame->hwnd;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = 256;
    ofn.lpstrInitialDir = set.map_directory;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    ofn.lpstrFilter = "Leak Line Files (*.lin)\0*.lin\0Leak Point Files (*.pts)\0*.pts\0";
    leakPtType = 1;
    if (set.game_mode == 2)
    {
        //	ofn.lpstrFilter = "Leak Line Files (*.lin)\0*.lin\0";
        ofn.lpstrTitle = "Select .lin Line File";
        ofn.lpstrDefExt = "lin";
        //	leakPtType = 1;
    }
    else
    {
        //	ofn.lpstrFilter = "Leak Point Files (*.pts)\0*.pts\0";
        ofn.lpstrTitle = "Select .pts Point File";
        ofn.lpstrDefExt = "pts";
        //	leakPtType = 0;
    }

    if (GetOpenFileName(&ofn))
    {
        FILE *fp;
        vec3_t pt;
        int num;
        numLeakPoints = 0;

        if ((fp = fopen(filename,"rt")) == NULL)
        {
            syserror(const_cast<char *> ("ShowLeaks: Couldn't open file: %s"), filename);
            return;
        }

        // read int the points...
        while (true)
        {
            num = fscanf(fp,"%f %f %f",&pt[0],&pt[1],&pt[2]);

            if (num != 3)
                break;
            numLeakPoints++;
            if (numLeakPoints > 2000)
                break;
        }
        fclose(fp);

        if (!numLeakPoints)
            return;

        // now read them in...
        if(!(fp = fopen(filename,"rt")))
        {
            syserror(const_cast<char *> ("ShowLeaks: Couldn't open file: %s"), filename);
            return;
        }
        aLeakPoints = new vec3_t[numLeakPoints];

        for (int counter = 0; counter < numLeakPoints; counter++)
        {
            num = fscanf(fp,"%f %f %f",&pt[0],&pt[1],&pt[2]);

            if (num != 3)
                break;

            VectorCopy(pt,aLeakPoints[counter]);
        }
        fclose(fp);

        set.track_leaks = 1;
        set.redrawedit = 1;
        set.redrawxy = 1;
        char outstr[80];
        sprintf(outstr,"Leak file [%s] processed...",filename);
        Show_Frame(outstr,true);
        // don't redraw...
    }
}

void CmShowPortals()
{
    if (!set.Map_Read)
        return;

    if (set.track_portals || aLeakPortals)
    {
        for (int i = 0; i < numLeakPortals; i++)
        {
            delete [] aLeakPortals[i].w;
            aLeakPortals[i].w = NULL;
        };
        delete[] aLeakPortals;
        aLeakPortals = NULL;
        numLeakPortals = 0;

        for (int c = 0; c < aLeakNode->numwindings; c++)
        {
            delete [] aLeakNode->w[c];
            aLeakNode->w[c] = NULL;
        };
        delete[] aLeakNode->w;
        delete aLeakNode;
        aLeakNode = NULL;
    }

    char filename[256] = "";

    OPENFILENAME ofn;
    memset(&ofn,0,sizeof(OPENFILENAME));
    ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
    ofn.hwndOwner = frame->hwnd;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = 256;
    ofn.lpstrTitle = "Select .por File";
    ofn.lpstrDefExt = "por";
    ofn.lpstrInitialDir = set.map_directory;
    ofn.lpstrFilter = "Leak Portal Files (*.por)\0*.por\0";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;


    if (!GetOpenFileName(&ofn))
        return;

    FILE *fp;
    float x, y, z;
    int i;
    int num;
    winding_t *w;
    int counter,c;

    numLeakPortals = 0;

    if ((fp = fopen(filename,"rt")) != NULL)
    {
        // read int the points...
        num = fscanf(fp,"%i",&numLeakPortals);
        if (num != 1)
        {
            numLeakPortals = 0;
            return;
        }

        if (numLeakPortals <= 0)
            return;

        editWindow->maxPortals = numLeakPortals;

        num = fscanf(fp,"%f %f %f",&x,&y,&z);
        if (num != 3)
            return;

        LeakPortalOrigin[0] = x;
        LeakPortalOrigin[1] = y;
        LeakPortalOrigin[2] = z;

        aLeakNode = new leakNode_t;
        memset(aLeakNode,0,sizeof(leakNode_t));

        num = fscanf(fp,"%i",&aLeakNode->numwindings);
        if (num != 1)
        {
            aLeakNode->numwindings = 0;
            return;
        }

        if (aLeakNode->numwindings <= 0)
            return;

        aLeakNode->w = new winding_t *[aLeakNode->numwindings];
        memset(aLeakNode->w,0,aLeakNode->numwindings*sizeof(winding_t *));

        for (counter = 0; counter < aLeakNode->numwindings; counter++)
        {
            // read the number of points.....
            num = fscanf(fp,"%i",&i);

            if (num != 1)
                break;

            if (i < 0 || i >= 256)
                break;

            w = aLeakNode->w[counter] = NewWinding(i);

            if(w)
            {
                for (c = 0; c < i; c++)
                {
                    num = fscanf(fp,"%f %f %f",&x,&y,&z);

                    if (num != 3)
                        break;

                    w->points[c][0] = x;
                    w->points[c][1] = y;
                    w->points[c][2] = z;

                }
            }
        }

        // could be bad...
        aLeakPortals = new leakPortal_t[numLeakPortals];
        memset(aLeakPortals,0,numLeakPortals*sizeof(leakPortal_t));

        for (counter = 0; counter < numLeakPortals; counter++)
        {
            // read the center...
            num = fscanf(fp,"%f %f %f",&x,&y,&z);

            if (num != 3)
                break;

            aLeakPortals[counter].centerPoint[0] = x;
            aLeakPortals[counter].centerPoint[1] = y;
            aLeakPortals[counter].centerPoint[2] = z;

            num = fscanf(fp,"%i",&i);

            if (num != 1)
                break;

            if (i < 0 || i >= 256)
                break;

            w = aLeakPortals[counter].w = NewWinding(i);

            if(w)
            {
                for (c = 0; c < i; c++)
                {
                    num = fscanf(fp,"%f %f %f",&x,&y,&z);

                    if (num != 3)
                        break;

                    w->points[c][0] = x;
                    w->points[c][1] = y;
                    w->points[c][2] = z;
                }
            }
        }
        set.track_portals = 1;
        set.redrawedit = 1;
        set.redrawxy = 1;

        char outstr[80];
        sprintf(outstr,"Portal file [%s] processed...",filename);
        Show_Frame(outstr,true);
    }
    fclose(fp);

    // figure out plugPortal value...
    //SegmentOnSegment(s1,s2,t1,t2);

    map *m = map_i[set.curmap];

    if (!m)
        return;

    int j,l,o;
    face_t *f;
    SetBrush *b;
    Entity *e;
    int p1, p2;
    float *t1, *t2;

    char outstr[128];
    vec3_t mins,maxs;

    e = m->world;
    plugPortal = -1;  // index of test portal...
    // only care about world for leak plugging...
    for (i = 0; i < numLeakPortals; i++)
    {
        w = aLeakPortals[i].w;
        if (!w)
            continue;

        // skipWinding = 0;
        p1 = w->numpoints - 1;
        bool foundSeg;
        bool foundAllSegs;

        foundAllSegs = true; // assume yes...
        for (j = 0; j < w->numpoints; j++)
        {
            sprintf(outstr,"Testing portal [%i/%i], segment [%i/%i]...", i,numLeakPortals,j,w->numpoints);
            status->SetText(outstr,true);

            p2 = j;
            foundSeg = false;   // assume no match for p1->p2...

            for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
            {
                if (b->IsInvalid())
                    continue;

                b->getMins(mins,maxs);

                for (l = 0; l < 3; l++)
                {
                    if ((mins[l] - 1.0) > w->points[p1][l])
                        break;
                    if ((mins[l] - 1.0) > w->points[p2][l])
                        break;

                    if ((maxs[l] + 1.0) < w->points[p1][l])
                        break;
                    if ((maxs[l] + 1.0) < w->points[p2][l])
                        break;
                };

                if (l != 3) // bbox failed...
                    continue;

                for (f = b->faces.p_next; f != &b->faces; f = f->p_next)
                {
                    LoopProblem("brush show portals");
                    if (!f->w)
                        continue;

                    for (o = 0; o < f->w->numpoints; o++)
                    {
                        t1 = f->w->points[o];
                        t2 = f->w->points[(o+1) % f->w->numpoints];

                        if (SegmentOnSegment(
                                    t1,t2,
                                    w->points[p1],w->points[p2]))
                        {
                            foundSeg = true;
                            break;
                        };
                    };
                };
            };
            p1 = p2;
            if (!foundSeg)   // if no find any, skip this winding...
            {
                foundAllSegs = false;
                break;
            };
        };
        if (foundAllSegs)
        {
            plugPortal = i;
            break;
        };
    };

    ::ShowWindow(editWindow->numPortals->hwnd,SW_RESTORE);
    ::ShowWindow(editWindow->killPortals->hwnd,SW_RESTORE);

}

void CmFaceTexture()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

//	FIXME cameraWindow->BnSetFaceTexture();
    SetBrush *b = m->selectedBrush();  /// get the first brush...
    if (!b || !b->parent->modifiable) // only for modifiable entities...
        return;

    m->saveForUndo(const_cast<char *> ("Set Face Texture"), UNDO_BRUSHES);

    texturedef_t	td;
    texWindow->getTextureDef(&td);

    if (!b->currentFace)
        b->currentFace = b->faces.p_next;

    if (b->currentFace == &b->faces)
        return;

    face_t *tmp = b->currentFace;	//remember selected face

    b->currentFace->texture = td;
    delete b->currentFace->qtexture;
    b->currentFace->qtexture = NULL;
    b->calcWindings();	// in case texture coords changed

    b->currentFace = tmp;	//keep face selected

    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Face Texture Set...",true);
}

void CmBrushTexture()
{
    if (!set.Map_Read)
        return;

    Entity *e;
    SetBrush *b;

    map_i[set.curmap]->saveForUndo(const_cast<char *> ("Set All Textures"), UNDO_BRUSHES);

    map *m;
    m = map_i[set.curmap];


    for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
    {
        LoopProblem("BrushTextureE");

        if (!e->modifiable)
            continue;

        for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
        {
            LoopProblem("BrushTextureB");
            if (!b->IsDrawable())
                continue;
            if (!b->IsSelected())
                continue;

            face_t *tmp = b->currentFace;	//remember selected face
            b->takeCurrentTexture();
            b->currentFace = tmp;	//keep face selected
        }
    }


    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Brush Texture(s) Set...",true);
};

void CmBrushContents(int contents, int removeContents)
{
    if (!set.Map_Read)
        return;

    Entity *e;
    SetBrush *b;

    char ti[64];
    if (removeContents == -1)
        strcpy(ti,"Removing contents...");
    else if (removeContents == 0)
        strcpy(ti,"Setting contents...");
    else if (removeContents == 1)
        strcpy(ti,"Adding contents...");
    else
        return;

    map *m = map_i[set.curmap];
    if (!m)
        return;

    m->saveForUndo(ti,UNDO_BRUSHES);

    face_t *f;
    for (e = m->objects.p_next; e != &m->objects; e = e->p_next)
    {
        if (!e->modifiable)
            continue;

        for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
        {
            if (!b->IsDrawable())
                continue;

            if (!b->IsSelected())
                continue;

            for (f = b->faces.p_next; f != &b->faces; f = f->p_next)
            {
                LoopProblem("brush brush contents");
                if (!f)
                    continue;

                if (removeContents == -1)
                    f->texture.contents &= ~contents;
                else if (removeContents == 0)
                    f->texture.contents = contents;
                else if (removeContents == 1)
                    f->texture.contents |= contents;
            }
            b->calcWindings();
        }
    }
    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Brush Flags Added/Removed...",true);
}

void CmBrushFlags(int flags, int removeFlags)
{
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    if (!m)
        return;

    char ti[64];
    if (removeFlags == -1)
        strcpy(ti,"Removing flags...");
    else if (removeFlags == 0)
        strcpy(ti,"Setting flags...");
    else if (removeFlags == 1)
        strcpy(ti,"Adding flags...");
    else
        return;

    m->saveForUndo(ti,UNDO_BRUSHES);

    for (Entity *e = m->objects.p_next; e != &m->objects; e = e->p_next)
    {
        if (!e->modifiable)
            continue;

        for (SetBrush *b = e->objects.p_next; b != &e->objects; b = b->p_next)
        {
            if (!b->IsDrawable())
                continue;

            if (!b->IsSelected())
                continue;

            for (face_t *f = b->faces.p_next; f != &b->faces; f = f->p_next)
            {
                LoopProblem("brush flags");
                if (!f)
                    continue;

                if (removeFlags == -1)
                    f->texture.flags &= ~flags;
                else if (removeFlags == 0)
                    f->texture.flags = flags;
                else if (removeFlags == 1)
                    f->texture.flags |= flags;
            }
            b->calcWindings();
        }
    }
    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Brush Flags Updated...",true);
}

bool CompareFaces(SetBrush *b1, SetBrush *b2, int autoFix)
{
    int i;
    face_t *f1, *f2;
    vec3_t n1, n2;
    float diff,distdiff;
    Entity *saveCurrent;
    int olds1, olds2;

    map *m = map_i[set.curmap];

    // restart here!

    for (f1 = b1->faces.p_next; f1 != &b1->faces; f1 = f1->p_next)
    {
        LoopProblem("brush compare 1");
        for (f2 = b2->faces.p_next; f2 != &b2->faces; f2 = f2->p_next)
        {
            LoopProblem("brush compare 2");
            // see if the planes are in the ball park
            distdiff = fabs(f1->plane.dist + f2->plane.dist);
            if (distdiff >= set.distance_epsilon)
                continue;

            // look for facing planes by negating one of the normals...
            VectorCopy(f1->plane.normal,n1);
            VectorCopy(f2->plane.normal,n2);
            VectorSubtract(vec3_origin,n2,n2);

            diff = 0.0;
            for (i = 0; i < 3; i++)
            {
                diff += (n2[i]-n1[i])*(n2[i]-n1[i]);
            };
            diff = sqrt(diff);

            if (diff <= set.min_epsilon)
                continue;

            if (diff >= set.max_epsilon) // not supposed to be opposites...
                continue;

            int fix;

            if (!autoFix)
            {
                // prompt

                for (i = 0; i < 3; i++)
                    xy_eye[i] = (b1->bctr[i]+b2->bctr[i])/2.0f;

                saveCurrent = m->current;

                m->current = b1->parent;

                olds1 = b1->IsSelected();
                olds2 = b2->IsSelected();

                b1->setSelected(true);
                b2->setSelected(true);

                for (i = 0; i < set.xyViews; i++)
                {
                    xyWindow[i]->RedrawContents();
                }

                char prompt[128];
                sprintf(prompt,"Confirm merging brush faces:\nNormal Epsilon %f\nDistance Epsilon %f",diff,distdiff);
                fix = MessageBox(0,prompt, "BSP - Leak Search", MB_YESNOCANCEL | MB_ICONQUESTION);

                m->current = saveCurrent;

                b1->setSelected(olds1);
                b2->setSelected(olds2);

                if (fix == IDCANCEL)
                    return false;
            }
            else
            {
                fix = IDYES;
            }

            if (fix == IDYES)
            {
                for (int j = 0; j < 3; j++)
                {
                    // copy over the plane definition points...
                    VectorCopy(f1->planepts[j],f2->planepts[2-j]);
                }
                b2->calcWindings();
            }
        }
    }
    return true;
}

void CmLeakTest() // show the leak dialog...
{
    if (!set.Map_Read)
        return;

    int fix;

    if (map_i[set.curmap]->dirty)
    {
        fix = MessageBox(client->hwnd,"Save first? (RECOMMENDED)", "BSP - Leak Search", MB_YESNO | MB_ICONQUESTION);

        if (fix == IDYES)
        {
            CmSave(); // must save first...
        }
    }

    // ask about autoFix or prompting...
    fix = MessageBox(client->hwnd,"Automatically fix brushes, without prompting?", "BSP - Leak Search", MB_YESNOCANCEL | MB_ICONQUESTION);

    if (fix == IDCANCEL)
        return;

    int autoFix = (fix == IDYES);
    // save eye position...
    vec3_t saveEye;
    VectorCopy(xy_eye,saveEye);

    // make sure we hide the clipper, too.
    //CmDeselectAll();
    //CmDeselectAll();

    SetBrush *b;
    SetBrush *b2;
    Entity *e;

    vec3_t mins1, maxs1, mins2, maxs2;

    int skip;

    int done = 0;


    map *m;
    m = map_i[set.curmap];
    m->saveForUndo(const_cast<char *> ("Set All Textures"), UNDO_BRUSHES);

    for (e = m->objects.p_next; !done && e && (e != &m->objects); e = e->p_next)
    {
        LoopProblem("SATE");

        if (!e->modifiable)
            continue;

        for (b = e->objects.p_next; !done && b && (b != &e->objects); b = b->p_next)
        {
            if (!b->IsDrawable())
                continue;

            LoopProblem("SATB");
            b->getMins(mins1,maxs1);
            for (b2 = e->objects.p_next; !done && b2 && (b2 != &e->objects); b2 = b2->p_next)
            {
                if (!b2->IsDrawable())
                    continue;

                if (b == b2)
                    continue;

                b2->getMins(mins2,maxs2);

                skip = 0;
                // bounding boxes overlap?
                for (int j=0 ; j<3 && !skip; j++)
                {
                    if ( ((mins2[j] -1.0) >= (maxs1[j] +1.0)) ||
                            ((maxs2[j] +1.0) <= (mins1[j] -1.0)) )
                    {
                        skip = 1;
                    };
                };

                if (skip)
                    continue;

                // Any faces look suspicious?  Fix if necessary...
                if (!CompareFaces(b,b2,autoFix))
                {
                    done = 1;
                    break;
                };
            };
        };
    };

    // reset eye position...
    VectorCopy(saveEye,xy_eye);

//	map_i[set.curmap]->eye[map_i[set.curmap]->cureye] = saveEye;

    set.redrawxy = 1;
    set.redrawedit = 1;

    if (autoFix)
    {
        Show_Frame("Leaks automatically sealed...",true);
    }
    else
    {
        Show_Frame("Leaks sealed...",true);
    }
}

bool CheckAndMerge(SetBrush *b1, SetBrush *b2, face_t *f1, face_t *f2)
{
    int i, j, k;

    float d;
    face_t *f, *g;
    winding_t *w, *w1, *w2;
    float diff;
    int startPt;
    int test;

    if (!b1 || !b2)
        return false;

    if (b1->IsInvalid() || b2->IsInvalid())
        return false;

    if (!f1 || !f2)
        return false;

    w1 = f1->w;
    w2 = f2->w;

    if (!w1 || !w2)
        return false;

    if (w1->numpoints != w2->numpoints)
        return false;

    // find a matching pair...
    startPt = -1;
    for (i = 0; i < w2->numpoints; i++)
    {
        diff = 0.0;
        for (j = 0; j < 3; j++)
        {
            diff += (w2->points[i][j] - w1->points[0][j])*(w2->points[i][j] - w1->points[0][j]);
        };
        diff = sqrt(diff);

        if (diff < set.max_point_distance)
        {
            startPt = i;
            break;
        }
    }

    if (startPt == -1)
        return false;

    // now compare the rest of the points...
    test = startPt;  // decrement test/ because the faces oppose each other...
    for (i = 0; i < w1->numpoints; i++)
    {
        diff = 0.0f;
        for (j = 0; j < 3; j++)
        {
            diff += (w2->points[test][j] - w1->points[i][j])*(w2->points[test][j] - w1->points[i][j]);
        }
        diff = sqrt(diff);
        if (diff >= set.max_point_distance)
            return false;

        test--;
        if (test < 0)
            test = w2->numpoints - 1;
    }

    // for each pair on points on the winding, find the
    // planes on brush1 and brush2 and compre...
    for (f = b1->faces.p_next; f != &b1->faces; f = f->p_next)
    {
        LoopProblem("brush c&m");
        if (f == f1)
            continue;

        for (g = b2->faces.p_next; g != &b2->faces; g = g->p_next)
        {
            LoopProblem("brush c&m2");
            if (g == f2)
                continue;

            w = g->w;
            if (!w)
                continue;

            for (k = 0; k < w->numpoints; k++)
            {
                d = DotProduct(f->plane.normal,w->points[k]) - f->plane.dist;
                if (d >= set.on_plane_epsilon)
                    return false;
            }
        }
    }

    //////////////////
    // merge them...
    // leave them in 1...
    // remvoe f1 and f2 and then add all together...

    b1->removeObject(f1);
    b2->removeObject(f2);

    //  Remove All Faces from b2, uncache their representation
    //  and add to b1.  Then recalculate windings;
    //  b2 should be an invalid brush, flagged for deletion.

    for (g = b2->faces.p_next; g != &b2->faces;)
    {
        LoopProblem("brush c&m6");
        f = g->p_next;

        b2->removeObject(g);

        if (g->w)
            delete g->w;

        g->w = NULL;

        if (g->qtexture)
            delete g->qtexture;

        g->qtexture = NULL;

        b1->addObject(g);

        g = f;
    }

    b1->calcWindings();
    b1->removeUnusedFaces();

    // Just in case...
    b2->setInvalid(true);
    return true;
}

bool MergeBrushes(SetBrush *b1, SetBrush *b2, int reqMatchingTex, int reqMatchingFlags)
{
    face_t *f1, *f2;
//	int i;
    vec3_t n1, n2;
    float diff,distdiff;

    // if we care about textures, compare that they are the same...
    if (reqMatchingTex)
    {
        if (b1->faces.p_next == &b1->faces)
            return false;
        if (b2->faces.p_next == &b2->faces)
            return false;

        char texName[LUMP_NAME_LENGTH_MAX];

        // grab the first face...
        strcpy(texName,b1->faces.p_next->texture.texture);

        //save flags to match if needed
        int contents = b1->faces.p_next->texture.contents;
        int flags = b1->faces.p_next->texture.flags;
        int value = b1->faces.p_next->texture.value;
        // compare all other faces, if any different, abort...

        for (f1 = b1->faces.p_next; (f1 != &b1->faces); f1 = f1->p_next)
        {
            LoopProblem("brush merge1");
            if (strcmpi(texName,f1->texture.texture))
                return false;
            if(reqMatchingFlags)
            {
                if(f1->texture.contents != contents || f1->texture.flags != flags || f1->texture.value != value)
                    return false;
            }
        }
        for (f2 = b2->faces.p_next; (f2 != &b2->faces); f2 = f2->p_next)
        {
            LoopProblem("brush merge2");
            if (strcmpi(texName,f2->texture.texture))
                return false;
            if(reqMatchingFlags)
            {
                if(f1->texture.contents != contents || f1->texture.flags != flags || f1->texture.value != value)
                    return false;
            }
        }


    }

    for (f1 = b1->faces.p_next; f1 != &b1->faces; f1 = f1->p_next)
    {
        LoopProblem("brush m1");
        for (f2 = b2->faces.p_next; f2 != &b2->faces; f2 = f2->p_next)
        {
            LoopProblem("brush m2");
            // see if the planes are in the ball park
            distdiff = fabs(f1->plane.dist + f2->plane.dist);
            if (distdiff >= set.max_face_gap)
                continue;

            // look for facing planes by negating one of the normals...
            VectorCopy(f1->plane.normal,n1);
            VectorCopy(f2->plane.normal,n2);
            VectorSubtract(vec3_origin,n2,n2);

            diff = (n2[0]-n1[0])*(n2[0]-n1[0]) + (n2[1]-n1[1])*(n2[1]-n1[1]) + (n2[2]-n1[2])*(n2[2]-n1[2]);
            diff = sqrt(diff);

            if (diff >= set.max_normal_delta)
                continue;

            //
            if (CheckAndMerge(b1,b2,f1,f2))
                return true;

        }
    }
    return false;
}

void CmMergeBrushes()	 // show the merge brushes dialog...
{
    if (!set.Map_Read)
        return;

    TMergeDlg dialog(client->hwnd,&client->mergexfer);
    if (dialog.Execute() != IDOK)
    {
        return;
    }

    map *m = map_i[set.curmap];

    if (client->mergexfer.SaveFirst == BST_CHECKED)
        CmSave(); // must save first...

    int selOnly = (client->mergexfer.SelectedOnly == BST_CHECKED) ? 1 : 0;
    int reqMatchingTex = (client->mergexfer.TextureCompare == BST_CHECKED) ? 1 : 0;
    int reqMatchingFlags = (client->mergexfer.MatchingFlags == BST_CHECKED) ? 1 : 0;

    set.max_face_gap = (float) atof(client->mergexfer.Face);
    set.max_normal_delta = (float) atof(client->mergexfer.Normal);
    set.max_point_distance = (float) atof(client->mergexfer.Point);
    set.on_plane_epsilon = (float) atof(client->mergexfer.Convexity);

    set.max_face_gap = max((float)(0.0001),set.max_face_gap);
    set.max_face_gap = min((float)(10.0),set.max_face_gap);
    set.max_normal_delta = max((float)(0.0001),set.max_normal_delta);
    set.max_normal_delta = min((float)(0.5),set.max_normal_delta);
    set.max_point_distance = max((float)(0.0001),set.max_point_distance);
    set.max_point_distance = min((float)(10.0),set.max_point_distance);
    set.on_plane_epsilon = max((float)(0.0001),set.on_plane_epsilon);
    set.on_plane_epsilon = min((float)(10.0),set.on_plane_epsilon);

    m->saveForUndo(const_cast<char *> ("Merge Brushes"), UNDO_FORCEWHOLE);
    int prebrushes = m->numBrushes();

    SetBrush *b;
    SetBrush *b2;
    Entity *e;
    vec3_t mins1, maxs1, mins2, maxs2;
    char tempStr[128];

    // now, go through each entity and compare brushes...

    for(int anyChanged = 1, pass = 1; anyChanged; pass++)
    {
        int done = 0;
        int total = 0;
        anyChanged = 0;
        for (e = m->objects.p_next; !done && e && (e != &m->objects); e = e->p_next)
        {
            if (!e->modifiable)
            {
                total++;
                if (!(total % 5))
                {
                    sprintf(tempStr,"Merging:  pass [%i] brush [%i]...",pass,total);
                    status->SetText(tempStr,true);
                }
                continue;
            }

            for (b = e->objects.p_next; !done && (b != &e->objects); b = b->p_next)
            {
                total++;
                if (!(total % 5))
                {
                    sprintf(tempStr,"Merging:  pass [%i] brush [%i]...",pass,total);
                    status->SetText(tempStr,true);
                }

                if (selOnly && !b->IsSelected())
                    continue;
                if (!b->IsDrawable())
                    continue;

                b->getMins(mins1,maxs1);
                for (b2 = e->objects.p_next; !done && (b2 != &e->objects); b2 = b2->p_next)
                {
                    if (b == b2)
                        continue;
                    if (selOnly && !b2->IsSelected())
                        continue;
                    if (!b2->IsDrawable())
                        continue;

                    b2->getMins(mins2,maxs2);

                    bool skip = false;
                    // bounding boxes overlap?
                    for (int j=0 ; j<3 && !skip; j++)
                    {
                        if ( ((mins2[j] -1.0) >= (maxs1[j] +1.0)) ||
                                ((maxs2[j] +1.0) <= (mins1[j] -1.0)) )
                        {
                            skip = true;
                        }
                    }
                    if (skip) continue;

                    // Any faces look suspicious?  Fix if necessary...
                    if (MergeBrushes(b,b2,reqMatchingTex,reqMatchingFlags))
                    {
                        done = 1;
                        anyChanged = 1;
                        break;
                    }
                }
            }
        }
    }

    // remove all "invalid" brushes...
    m->cleanUpInvalidObjects();

    int postbrushes = m->numBrushes();

    sprintf(tempStr,"Merge Results:  Pre-Merge [%i] - Post-Merge[%i] = [%i] Brushes Eliminated",
            prebrushes, postbrushes, prebrushes-postbrushes);

    MessageBox(client->hwnd,tempStr,"BSP Merge Results",MB_OK);

    set.redrawxy = 1;
    set.redrawedit = 1;

    if (selOnly)
    {
        Show_Frame("Selected brushes automatically merged...",true);
    }
    else
    {
        Show_Frame("All brushes merged...",true);
    }
}

void CmNextCamera()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    m->cureye = (m->cureye + 1) % MAX_CAMERAS;

    set.redrawxy = 1;
    set.redrawedit = 1;
    char outstr[80];
    sprintf(outstr,"Camera %i Active...",m->cureye+1);
    Show_Frame(outstr,true);
};

void CmPrevCamera()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    m->cureye = m->cureye - 1;
    if (m->cureye < 0)
    {
        m->cureye = MAX_CAMERAS-1;
    }

    set.redrawxy = 1;
    set.redrawedit = 1;
    char outstr[80];
    sprintf(outstr,"Camera %i Active...",m->cureye+1);
    Show_Frame(outstr,true);
}

void CmFindBrush()
{
    if (!set.Map_Read)
        return;
    map *m = map_i[set.curmap];

    char s[MAX_PATH];
    strcpy(s,"1");

    if (IDOK != InputDialog(client->hwnd, const_cast<char *> ("Find Brush"), const_cast<char *> ("Which brush to find ? "), s, sizeof(s)).Execute())
        return;

    int index = atoi(s);
    if (index <= 0)
        return;

    bool found = false;

    Entity *ent;
    SetBrush *brush;

    for (ent = m->objects.p_next; ent && (ent != &m->objects); ent = ent->p_next)
    {
        for (brush = ent->objects.p_next; brush && (brush != &ent->objects); brush = brush->p_next)
        {
            if (brush->originalIndex == index)
            {
                brush->setSelected(true);
                found = true;
            }
        }
    }
//	set.redrawxy   = 1;
//	set.redrawedit = 1;

    if (found)
        sprintf(s,"Found brush %i...",index);
    else
        sprintf(s,"Couldn't find brush %i...",index);
    Show_Frame(s,true);
}

void CmMapInfo()
{
    if (!set.Map_Read || !map_i || !map_i[set.curmap])
        return;

    int total = 0;
    int totalwithents = 0;
    int world = 0;
    int lights = 0;
    int clip = 0;
    int targetpairs = 0;
    int targetnames = 0;
    int sky = 0;
    int water = 0;
    int regular = 0;
    int emod = 0;
    int efix = 0;
    int detail = 0;
    int hint = 0;
    int selected = 0;
    int unselected = 0;
    int faces = 0;

    char *name;
    map *m;
    Entity *ent;
    SetBrush *brush;

    m = map_i[set.curmap];
    for (ent = m->objects.p_next; ent && (ent != &m->objects); ent = ent->p_next)
    {
        if ((ent != m->world) && ent->modifiable)
        {
            emod++;
        }
        else if ((ent != m->world) && !ent->modifiable)
        {
            efix++;
        }

        name = ent->classname;

        if (!strncmp(name,"light",5))
            lights++;

        name = ent->target;
        if (name && *name)
            targetpairs++;

        name = ent->targetname;
        if (name && *name)
            targetnames++;

        // skip fixed brushes...
        if (!ent->modifiable)
        {
            totalwithents++;
            continue;
        }

        face_t *f;
        for (brush = ent->objects.p_next; brush && (brush != &ent->objects); brush = brush->p_next)
        {
            total++;
            totalwithents++;
            if (ent == m->world)
            {
                world++;
                faces += brush->numfaces;
            }

            if (brush->IsSelected())
                selected++;
            else
                unselected++;

            f = brush->faces.p_next;
            if (f == &brush->faces)
                continue;

            if (f->texture.contents & CONTENTS_DETAIL)
                detail++;

            if (f->texture.flags & SURF_HINT)
                hint++;

            if ((!strcasecmp(f->texture.texture, "clip")) ||
                    (f->texture.contents & (CONTENTS_PLAYERCLIP | CONTENTS_MONSTERCLIP)))
            {
                clip++;
            }
            else if ((f->texture.texture[0] == '*') ||
                     (f->texture.contents & (CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER)))
            {
                water++;
            }
            else if  ((!strnicmp(f->texture.texture, "sky", 3)) ||
                      (f->texture.flags & SURF_SKY))
            {
                sky++;
            }
            else
            {
                regular++;
            }
        }
    }

    BSPHelp(const_cast<char *> ("Map Info..."), const_cast<char *> (""));
    BSPHelpAddV(const_cast<char *> ("Map Info for [%s]\r\n"), m->filename);
    BSPHelpAdd(const_cast<char *> (".................................................\r\n"));
    BSPHelpAddV(const_cast<char *> ("Total Brushes       \t%i\r\n"), totalwithents);
    BSPHelpAddV(const_cast<char *> ("Modifiable          \t%i\r\n"), total);
    BSPHelpAddV(const_cast<char *> ("World Brushes       \t%i\r\n"), world);
    BSPHelpAddV(const_cast<char *> ("Brush Faces         \t%i\r\n"), faces);
    BSPHelpAddV(const_cast<char *> ("Modifiable Entities \t%i\r\n"), emod);
    BSPHelpAddV(const_cast<char *> ("Fixed Size Entities \t%i\r\n"), efix);
    BSPHelpAddV(const_cast<char *> ("Lights              \t%i\r\n"), lights);
    BSPHelpAddV(const_cast<char *> ("Sky Brushes         \t%i\r\n"), sky);
    BSPHelpAddV(const_cast<char *> ("Water Brushes       \t%i\r\n"), water);
    BSPHelpAddV(const_cast<char *> ("Clip Brushes        \t%i\r\n"), clip);
    BSPHelpAddV(const_cast<char *> ("Target Entities     \t%i\r\n"), targetpairs);
    BSPHelpAddV(const_cast<char *> ("Targetnames         \t%i\r\n"), targetnames);
    BSPHelpAddV(const_cast<char *> ("Detail              \t%i\r\n"), detail);
    BSPHelpAddV(const_cast<char *> ("Hint                \t%i\r\n"), hint);
    BSPHelpAddV(const_cast<char *> ("Selected            \t%i\r\n"), selected);
    BSPHelpAddV(const_cast<char *> ("Unselected          \t%i\r\n"), unselected);
    BSPHelpAddV(const_cast<char *> ("Other Brushes       \t%i\r\n"), regular);

    status->SetText(const_cast<char *> ("Map Info Calculated..."), true);
}

void CmExportSelected()
{
    if (!set.Map_Read)
        return;
    map_i[set.curmap]->selectedExport();
}

void CmModels()
{
    set.draw_models ^= 1;
    set.redrawedit = 1;
    Show_Frame("Model Mode Toggled...",true);
}

void CmSolidModels()
{
    set.texture_models ^= 1;
    set.redrawedit = 1;
    Show_Frame("Solid/Wire Models Toggled...",true);
}

void CmGlShading()
{
    set.gl_shading ^= 1;
    set.redrawedit = 1;
    Show_Frame("GL Shading Toggled...",true);
    editWindow->RedrawContents();	//why is this needed??
}
//this ones for GL
void CmOutlineUseGroup()
{
    set.outline_use_group ^= 1;
    set.redrawedit = 1;
    Show_Frame("Outline Mode Toggled...",true);
}
//this is for software ...
void CmOutlineUseFlat()
{
    set.color_wire ^= 1;
    set.redrawedit = 1;
    Show_Frame("Outline Mode Toggled...",true);
}
void CmRegionExport()
{
    if (!set.Map_Read)
        return;
    map_i[set.curmap]->regionExport();
}

void CmAddDetail()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->AddDetail();

    if (texWindow)
    {
        texturedef_t *currentTex;
        currentTex = texWindow->currentTexture;
        currentTex->flags |= CONTENTS_DETAIL;
    }
    if (surfaceWindow->IsWindowVisible())
        surfaceWindow->contents[27]->SetCheck(BST_CHECKED);

    set.redrawxy   = 1;
    set.redrawedit = 1;
    Show_Frame("Detail bit set...",true);
}

void CmRemoveDetail()
{
    if (!set.Map_Read)
        return;

    map_i[set.curmap]->RemoveDetail();

    if (texWindow)
    {
        texturedef_t *currentTex;
        currentTex = texWindow->currentTexture;
        currentTex->flags &= ~CONTENTS_DETAIL;
    }
    if (surfaceWindow->IsWindowVisible())
        surfaceWindow->contents[27]->SetCheck(BST_UNCHECKED);

    set.redrawxy   = 1;
    set.redrawedit = 1;
    Show_Frame("Detail bit removed...",true);
}

void CmMakeClicked()
{
    if (!set.Map_Read)
        return;
    if (!map_i || !map_i[set.curmap])
        return;
    if (!frame->makeBox)
        return;

    char str[40];
    frame->makeBox->GetText(str,40);

    if (!strcmpi(str,"Stairs"))
    {
        CmEditMakeStairs();
    }
    else if (!strcmpi(str,"Tall Brush"))
    {
        CmTallBrush();
    }
    else if (!strcmpi(str,"Short Brush"))
    {
        CmShortBrush();
    }
    else if (!strcmpi(str,"Brush of Specified Height"))
    {
        CmHeightBrush();
    }
    else if (!strcmpi(str,"Brush from Values"))
    {
        CmMakeBrush();
    }
    else if (!strcmpi(str,"Lights"))
    {
        CmMakeLights();
    }
    else if (!strcmpi(str,"Room"))
    {
        CmMakeRoom();
    }
    else if (!strcmpi(str,"Extruded Room"))
    {
        CmExtrude();
    }
    else if (!strcmpi(str,"Arch"))
    {
        CmMakeArch();
//	} else if (!strcmpi(str,"Sphere")) {
//   		CmMakeSphere();
    }
    else if (!strcmpi(str,"Cylinder (Solid)"))
    {
        CmMakeNSided();
    }
    else if (!strcmpi(str,"Tube (Hollow)"))
    {
        CmMakeCylinder();
    }
    else if (!strcmpi(str,"Pyramid"))
    {
        CmMakePyramid();
    }
    else if (!strcmpi(str,"Wedge"))
    {
        CmMakeWedge();
    }
    else
    {
        MessageBox(client->hwnd,"Unknown Make Command!","BSP Make", MB_OK | MB_ICONEXCLAMATION);
    }
}

void Exporter(int e_number)
{
    if (!set.Map_Read)
        return;
    if (e_number < 0 || e_number >= client->numExporters)
        return;
    if (!client->ExporterNames[e_number])
        return;

    char outstr[80] = "";
    char goodname[MAX_PATH];

    ExtractFileBase(map_i[set.curmap]->filename,goodname,true);

    Path progname(const_cast<char *> ("%s\\%s"), set.bat_directory,client->ExporterNames[e_number]);

    int retval = MessageBox(client->hwnd, const_cast<char *> ("Okay to save map ? "), const_cast<char *> ("BSP - Export"), MB_YESNO | MB_ICONQUESTION);

    if (retval == IDYES)
    {
        CmSave(); // must save first...
        Bsp_ShellExecute(0, NULL, progname, goodname, set.map_directory);
        sprintf(outstr,"Spawning %s on current .map file...",client->ExporterNames[e_number]);
    }
    Show_Frame(outstr,true);
}
void Exporter0()
{
    Exporter(0);
}
void Exporter1()
{
    Exporter(1);
}
void Exporter2()
{
    Exporter(2);
}
void Exporter3()
{
    Exporter(3);
}
void Exporter4()
{
    Exporter(4);
}
void Exporter5()
{
    Exporter(5);
}
void Exporter6()
{
    Exporter(6);
}
void Exporter7()
{
    Exporter(7);
}
void Exporter8()
{
    Exporter(8);
}
void Exporter9()
{
    Exporter(9);
}
void Exporter10()
{
    Exporter(10);
}
void Exporter11()
{
    Exporter(11);
}
void Exporter12()
{
    Exporter(12);
}
void Exporter13()
{
    Exporter(13);
}
void Exporter14()
{
    Exporter(14);
}
void Exporter15()
{
    Exporter(15);
}

void RemoveExporterItemsFromMenu(HMENU hMenu)
{
    // remove Exporter items
    if (-1 != GetMenuPos(hMenu, CM_EXPORTER + 0))
        for (int i = CM_EXPORTER; i < (CM_EXPORTER + 16); i++)
            if (-1 != GetMenuPos(hMenu, i))
                ::RemoveMenu(hMenu, i, MF_BYCOMMAND);
}

int BuildExporterMenu(HMENU hMenu)
{
    char buf[MAX_PATH], *buf_ptr;

    //need to determine and cache recent cmd ids
    static int export_id[MAX_RECENT];
    static bool export_init = false;
    if(!export_init)
    {
        export_init = true;
        for(int i = 0; i < MAX_EXPORTERS; i++)
        {
            sprintf(buf, "CM_EXPORTER%d", i);
            ccmd_t *cm = Ccmd_FindCmd(buf);
            if(cm) export_id[i] = cm->id;
        }
    }

    RemoveExportNames();

    struct _finddata_t ffb;
    memset(&ffb,0,sizeof(ffb));
    char dirname[128];
    snprintf(dirname,sizeof(dirname),"%s\\*.bat",set.bat_directory);

    int ff = _findfirst(dirname,&ffb);

    int count = 0;
    for (int files = ff < 0 ? 1 : 0; !files && count < MAX_EXPORTERS; files = _findnext(ff,&ffb))
    {
        if (ffb.attrib & _A_SUBDIR)
            continue;
        int len = strlen(ffb.name);
        client->ExporterNames[count] = new char[len + 1];
        strcpy(client->ExporterNames[count],ffb.name);
        strlwr(client->ExporterNames[count]);
        toupper((unsigned char)client->ExporterNames[count][0]);

        snprintf(buf,sizeof(buf),"&%d %s", count+1, client->ExporterNames[count]);

        buf_ptr = buf;
        if(count >= 9)
            buf_ptr++;	// hide mnemonic for numbers 10+

        AppendMenu(hMenu, MF_STRING, export_id[count], buf_ptr);

        count++;
    }

    if(!count)
        AppendMenu(hMenu, MF_STRING | MF_GRAYED, 0, "Exporter list empty");

    _findclose(ff);
    client->numExporters = count;
    return count;
}

int	GetMenuPos(HMENU hMenu, UINT id)
{
    for (int i = ::GetMenuItemCount(hMenu) - 1; i >= 0; i--)
        if (::GetMenuItemID(hMenu, i) == id)
            return i;
    return -1;
}

void RemoveExportNames()
{
    for (int i = 0; i < 16; i++)
        delete[] client->ExporterNames[i];
    memset(client->ExporterNames, 0, MAX_EXPORTERS*sizeof(char *));
}
