#include "global.h"


// enabler helpers

void EnableCmd(int id, bool enable)
{
    //update main menu
    EnableMenuItem(GetMenu(frame->hwnd), id, MF_BYCOMMAND | (enable ? MF_ENABLED : MF_DISABLED|MF_GRAYED ));
    //update toolbar
    for(int i=0; i<frame->numbars; i++)
        SendMessage(cbars[i]->hwnd,TB_ENABLEBUTTON, id, (LPARAM)(WORD)enable);
}

void CheckCmd(int id, bool checked)
{
    //update main menu
    CheckMenuItem(GetMenu(frame->hwnd), id, MF_BYCOMMAND | (checked ? MF_CHECKED : MF_UNCHECKED|MF_GRAYED ));
    //update toolbar
    for(int i=0; i<frame->numbars; i++)
        SendMessage(cbars[i]->hwnd,TB_CHECKBUTTON, id, (LPARAM)(WORD)checked);
}
void SetMenuText(int id, char *str)
{
    MENUITEMINFO mii;
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_TYPE;
    mii.fType = MFT_STRING;
    mii.dwTypeData = str;
    mii.cch = strlen(str);
    SetMenuItemInfo(hMenu,id,MF_BYCOMMAND, &mii);
}




// Main Command enablers...

void CeClipAutoFlip(int id)
{
    CheckCmd(id, 0 != set.clipper_autoflip);
}
void CeClippersStay(int id)
{
    CheckCmd(id, 0 != set.clippers_stay);
}
//CmClipMode0
void CeClipMode0(int id)
{
    CheckCmd(id, 0 == set.clipper_mode);
}
//CmClipMode1
void CeClipMode1(int id)
{
    CheckCmd(id, 1 == set.clipper_mode);
}

void CeLookAtSel(int id)
{
    CheckCmd(id, set.look_at_sel);
}
void CeFileNew(int id)
{
    EnableCmd(id, set.nummaps < MAX_MAPS);
}
void CeFileOpen(int id)
{
    EnableCmd(id, set.nummaps < MAX_MAPS);
}
/////////////////////
void CeNeedMap(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
}
void CeNeedNoMap(int id)
{
    EnableCmd(id, 0 == set.Map_Read);
}
void CeNeedSelection(int id)
{
    EnableCmd(id, 0 != set.Map_Read && 0 != map_i[set.curmap]->numSelected());
}
void CePaste(int id)
{
    EnableCmd(id, 0 != set.Map_Read && 0 != copymap);
}
void CeBrushWindow(int id)
{
    EnableCmd(id, true);
    CheckCmd(id, 0 != surfaceWindow);
}
void CeSnapBack(int id)
{
    CheckCmd(id, 0 != set.snap_back);
}
void CeRunMode(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, set.run_mode == Runmode::run);
}
void CeUndo(int id)
{
    if (!set.Map_Read)
    {
        EnableCmd(id, false);
    }
    else
    {
        map *m = map_i[set.curmap];
        if (m && m->undo)
        {
            EnableCmd(id, true);
            char outstr[80];
            sprintf(outstr,"Undo %s...",m->undo->description);
            SetMenuText(id, outstr);
        }
        else
        {
            SetMenuText(id, const_cast<char *> ("Undo..."));
            EnableCmd(id, false);
        }
    }
}
void CeToggleKnobs(int id)
{
    CheckCmd(id, 0 != set.center_knobs_xy);
}
void CeToggleFlymode(int id)
{
    CheckCmd(id, 0 != set.flymode);
}
void CeModels(int id)
{
    CheckCmd(id, 0 != set.draw_models);
}
void CeSolidModels(int id)
{
    CheckCmd(id, 0 != set.texture_models);
}
void CeGlShading(int id)
{
    CheckCmd(id, 0 != set.gl_shading);
}
void CeOutlineUseGroup(int id)
{
    CheckCmd(id, 0 != set.outline_use_group);
}
void CeOutlineUseFlat(int id)
{
    CheckCmd(id, 0 != set.color_wire);
}
void CeBar1(int id)
{
    if (frame->numbars < 1)
    {
        EnableCmd(id, false);
    }
    else
    {
        CheckCmd(id, cbars[0]->visible);
    }
}
void CeBar2(int id)
{
    if (frame->numbars < 2)
    {
        EnableCmd(id, false);
    }
    else
    {
        CheckCmd(id, cbars[1]->visible);
    }
}

void CeRevert(int id)
{
    bool enable = true;
    if (!set.Map_Read)
    {
        enable = false;
    }
    else
    {
        map *m = map_i[set.curmap];
        Entity *e = m->current;
        Entity *w = m->world;

        if (e == w || !e || !w || !e->modifiable)
            enable = false;
        else if (!e->classname || !*e->classname)  // entity should have a classname!
            enable = false;
    }
    EnableCmd(id, enable);
}
void CeExportSelected(int id)
{
    EnableCmd(id, set.Map_Read && map_i[set.curmap]->numSelected());
}


void CeNeedLeaks(int id)
{
    EnableCmd(id, set.Map_Read && set.track_leaks);
}

void CeNeedPortals(int id)
{
    EnableCmd(id, set.Map_Read && set.track_portals);
}

void CeOmitDetail(int id)
{
    EnableCmd(id, set.Map_Read && (set.game_mode == 2));
    CheckCmd(id, (set.filter_detail == 1));
}

void CeOmitHint(int id)
{
    EnableCmd(id, set.Map_Read && (set.game_mode == 2));
    CheckCmd(id, (set.filter_hint == 1));
}

void CeOmitSky(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.filter_sky == 1));
}

void CeOmitTarget(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.filter_target == 1));
}

void CeOmitLights(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.filter_light == 1));
}

void CeOmitUnselected(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.filter_unselected == 1));
}

void CeOmitEntities(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.filter_entities == 1));
}

void CeOmitWorld(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.filter_world == 1));
}

void CeOmitWater(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.filter_water_brushes == 1));
}

void CeOmitClip(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.filter_clip_brushes == 1));
}

void CeGroupMode(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, 0 != set.group_mode);
}

void CeRegionMode(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, 0 != set.region_mode);
}

void CeNoRegion(int id)
{
    EnableCmd(id, 0 != set.Map_Read && 0 != map_i[set.curmap]->regionBrush && 0 != set.region_mode);
}

void CeRegionExport(int id)
{
    EnableCmd(id, 0 != set.Map_Read && 0 != set.region_mode);
}

void CeModeWire(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.drawmode == Drawmode::wire));
}

void CeModeFlat(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.drawmode == Drawmode::flat));
}

void CeModeTexture(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.drawmode == Drawmode::texture));
}

void CeGridMode(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, 0 != set.show_grid);
}
void CeToggleGridStyle(int id)
{
    if(set.grid_style != GRID_LINES)
        SetMenuText(id, const_cast<char *> ("Use Grid Lines..."));
    else
        SetMenuText(id, const_cast<char *> ("Use Grid Dots..."));
}

void CeToggleOutline(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, 0 != set.outline);
}

void CeToggleNames(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, 0 != set.show_names);
}

void CeToggleConnections(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, 0 != set.show_connections);
}

void CeToggleHits(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, 0 != set.show_hits);
}

void CeToggleCoordinates(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, 0 != set.show_coordinates);
}

void CeScale1_64(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (fabs(set.scale - 1.0/64.0) < 0.0001));
}

void CeScale1_32(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (fabs(set.scale - 1.0/32.0) < 0.0001));
}

void CeScale1_16(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (fabs(set.scale - 1.0/16.0) < 0.0001));
}

void CeScale1_8(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (fabs(set.scale - 1.0/8.0) < 0.0001));
}

void CeScale1_4(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (fabs(set.scale - 1.0/4.0) < 0.0001));
}

void CeScale1_2(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (fabs(set.scale - 1.0/2.0) < 0.0001));
}

void CeScale3_4(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (fabs(set.scale - 3.0/4.0) < 0.0001));
}

void CeScale1(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (fabs(set.scale - 1.0) < 0.0001));
}

void CeScale2(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (fabs(set.scale - 2.0) < 0.0001));
}

void CeScale4(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (fabs(set.scale - 4.0) < 0.0001));
}

void CeScale8(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (fabs(set.scale - 8.0) < 0.0001));
}

void CeScale16(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (fabs(set.scale - 16.0) < 0.0001));
}

void CeGrid1(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.gridsize == 1));
}

void CeGrid2(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.gridsize == 2));
}

void CeGrid4(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.gridsize == 4));
}

void CeGrid8(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.gridsize == 8));
}

void CeGrid16(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.gridsize == 16));
}

void CeGrid32(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.gridsize == 32));
}

void CeGrid64(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.gridsize == 64));
}

void CeGrid128(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.gridsize == 128));
}

void CeGrid256(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.gridsize == 256));
}

void CeW1(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.curCfg == 0));
}

void CeW2(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.curCfg == 1));
}

void CeW3(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.curCfg == 2));
}

void CeW4(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, (set.curCfg == 3));
}

void CeRot1(int id)
{
    CheckCmd(id, set.rotate_step == 1);
}
void CeRot5(int id)
{
    CheckCmd(id, set.rotate_step == 5);
}
void CeRot10(int id)
{
    CheckCmd(id, set.rotate_step == 10);
}
void CeRot15(int id)
{
    CheckCmd(id, set.rotate_step == 15);
}
void CeRot30(int id)
{
    CheckCmd(id, set.rotate_step == 30);
}
void CeRot45(int id)
{
    CheckCmd(id, set.rotate_step == 45);
}

void CeGrid3D(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, 0 != set.show_3d_grid);
}
void CeAxes3D(int id)
{
    EnableCmd(id, 0 != set.Map_Read);
    CheckCmd(id, 0 != set.show_world_axes);
}

void CeTrans0(int id)
{
    CheckCmd(id, set.render_trans == 0);
}
void CeTrans1(int id)
{
    CheckCmd(id, set.render_trans == 1);
}
void CeTrans2(int id)
{
    CheckCmd(id, set.render_trans == 2);
}
void CeTransGL(int id)
{
    CheckCmd(id, set.render_trans > 0);
}
