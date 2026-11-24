#include "global.h"

bool loadMessageGiven = false;

typedef struct target_s
{
    char targetName[64];		// NXImage
} target_t;

int ent_count = 0;
char loadName[MAX_PATH];

int snapToGrid(float inp)
{
    return set.gridsize * rint(inp/set.gridsize);
}

void map::cleanUpInvalidObjects()
{
    Entity *e, *e_next;
    SetBrush *b, *b_next;

    for (e = objects.p_next; e != &objects; e = e_next)
    {
        LoopProblem("MCLEANE");
        e_next = e->p_next;

        for (b = e->objects.p_next; b != &e->objects; b = b_next)
        {
            LoopProblem("MCLEANB");
            b_next = b->p_next;

            if (b->IsInvalid())
            {
                e->removeObject(b);
                delete b;
            }
        }

        if (e->invalid)
        {
            if (e != world)
            {
                removeObject(e);
                delete e;
            }
            else
            {
                e->invalid = false;
            }
        }
    }
}

map::map()
{
    count = 0;
    dirty = 0;
    minz[0] = minz[1] = minz[2] = 0.0;
    maxz[0] = maxz[1] = maxz[2] = 80.0;
    oldselection = new Entity;
    current = NULL;
    world = NULL;
    memset(&objects,0,sizeof(Entity));
    objects.p_next = objects.p_prev = &objects;
    memset(filename,0,sizeof(filename));
    groups = new Group;

    //create default cameras
    vec3_t cameraPos = {0,-128,128};
    for (int i=0; i < MAX_CAMERAS; i++)
    {
        VectorCopy(cameraPos/*vec3_origin*/,eye[i]);
        angles[i].roll  = 0.0;
        angles[i].pitch = 0.0;
        angles[i].yaw   = M_PI;
    };
    cureye = 0;
    clipper_i = new Clipper;

    curclippoint = 0;

    favorites = NULL;
    favs = NULL;
    numf = 0;

    bests = NULL;
    mapTextures = NULL;
    Wads = new texWad *[MAX_WADS];
    memset(Wads,0,MAX_WADS*sizeof(texWad *));

    numWads = 0;
    numMapTextures = 0;

    dat = NULL;

    undo = NULL;
    regionSet = false;
    regionBrush = NULL;

    memset(texName,0,sizeof(texName));
    memset(texPath,0,sizeof(texPath));

    mapType = 0;
}

map::~map()
{
    int i;

    if (oldselection)
    {
        oldselection->freeObjects(true);
        delete oldselection;
        oldselection = NULL;
    }
    current = NULL;

    Entity *e, *n;
    for (e = objects.p_next; e != &objects; e = n)
    {
        LoopProblem("MDELETEMAP");
        n = e->p_next;
        e->freeObjects(true);
        removeObject(e);
        delete e;
    }

    count = 0;
    dirty = 0;
    delete groups;
    groups = NULL;

    delete[] bests;
    bests = NULL;

    delete[] favorites;
    favorites = NULL;

    if (favs)
    {
        for (i = 0; i < numf; i++)
            delete[] favs[i];
        delete[] favs;
    }
    favs = 0;

    if (mapTextures)
    {
        for (i = 0; i < numMapTextures; i++)
        {
            delete[] mapTextures[i]->aTextures;
            mapTextures[i]->aTextures = NULL;
            mapTextures[i]->aCount = 0;
        }
        delete[] mapTextures;
    }
    mapTextures = 0;

    numMapTextures = 0;
    numWads = 0;

    delete[] Wads;
    Wads = 0;

    delete clipper_i;
    clipper_i = 0;

    delete regionBrush;
    regionBrush = 0;

    RemoveUndo();

    objects.p_next = objects.p_prev = NULL;
}

void map::RemoveUndo()
{
    int i;
    if (undo)
    {
        switch (undo->undoType)
        {
        case UNDO_WHOLEMAP:
            delete undo->undoMap;
            undo->undoMap = NULL;
            break;
        case UNDO_SINGLEBRUSH:
            undo->singleBrush->freeObjects(true);
            delete undo->singleBrush;
            undo->singleBrush = NULL;
            break;
        case UNDO_MULTIBRUSH:
            for (i = 0; i < undo->numMultiBrush; i++)
            {
                undo->multiBrush[i]->freeObjects(true);
                delete undo->multiBrush[i];
                undo->multiBrush[i] = NULL;
            };
            delete[] undo->multiBrush;
            undo->multiBrush = NULL;
            break;
        case UNDO_GROUP:
            delete undo->undoGroup;
            undo->undoGroup = NULL;
            break;
        };
        delete undo; //!!!
        undo = NULL;
    }
}

// type can be brushes or whole map...
void map::saveForUndo(char *saveStr, int type)
{
    int counter;
    Entity *e, *curEnt;
    SetBrush *b, *sb;

    RemoveUndo();
    if (type == UNDO_NOUNDO)
        return;

    undo = new undo_t;
    memset(undo,0,sizeof(undo_t));

    STRNCPY(undo->description,saveStr);

    int i = numSelectedTotal();
    // qqq use a threshold

    switch (type)
    {
    case UNDO_FORCEWHOLE:
        undo->undoType = UNDO_WHOLEMAP;
        undo->undoMap = undoCopy();
        break;
    case UNDO_BRUSHES:
        if (!i)
        {
            undo->undoType = UNDO_WHOLEMAP;
            undo->undoMap = undoCopy();
        }
        else if (i == 1)
        {
            undo->undoType = UNDO_SINGLEBRUSH;

            b = selectedBrush();
            if (!b || !b->parent)   // qqq!!! should never happen.
            {
                undo->undoType = UNDO_WHOLEMAP;
                undo->undoMap = undoCopy();
                return;
            }

            e = b->parent->undoCopyNoBrushes(); // this is a copy of the parent
            // without any brushes, now just
            // add in the selected brush;

            sb = b->copy();
            sb->setParent(e);                   // point brush at temp ent...
            e->addObject(sb);                   // put it in...
            undo->singleBrush = e;
        }
        else if (i <= set.undo_threshold)
        {
            undo->undoType = UNDO_MULTIBRUSH;
            undo->multiBrush = new Entity *[set.undo_threshold];
            memset(undo->multiBrush,0,set.undo_threshold*sizeof(Entity *));

            counter = 0;
            for (curEnt = objects.p_next; curEnt != &objects; curEnt = curEnt->p_next)
            {
                LoopProblem("MUNDOE");
                if (!curEnt)   // Should not happen
                {
                    undo->undoType = UNDO_WHOLEMAP;
                    undo->undoMap = undoCopy();
                    return;
                }
                e = NULL;

                for (b = curEnt->objects.p_next; b != &curEnt->objects; b = b->p_next)
                {
                    LoopProblem("MUNDOB");
                    if (!b || !b->parent)
                    {
                        undo->undoType = UNDO_WHOLEMAP;
                        undo->undoMap = undoCopy();
                        return;
                    }

                    if (!b->IsSelected())     // skip it...
                        continue;

                    if (!e)
                    {
                        e = b->parent->undoCopyNoBrushes(); // this is a copy of the parent
                        // without any brushes, now just
                        // add in the selected brush;
                        undo->multiBrush[counter++] = e;
                    }
                    sb = b->copy();
                    sb->setParent(e);                   // point brush at temp ent...
                    e->addObject(sb);                   // put it in...
                }
            }
            undo->numMultiBrush = counter;
        }
        else
        {
            if (set.nothresholdundo)
            {
                delete undo;
                undo = NULL;
            }
            else
            {
                undo->undoType = UNDO_WHOLEMAP;
                undo->undoMap  = undoCopy();
            }
        }
        break;
    case UNDO_RENBRUSH:
        if (REN_bestBrush)
        {
            undo->undoType = UNDO_SINGLEBRUSH;

            b = REN_bestBrush;
            if (!b || !b->parent)   // qqq!!! should never happen.
            {
                undo->undoType = UNDO_WHOLEMAP;
                undo->undoMap = undoCopy();
                return;
            }

            e = b->parent->undoCopyNoBrushes(); // this is a copy of the parent
            // without any brushes, now just
            // add in the selected brush;

            sb = b->copy();
            sb->setParent(e);                   // point brush at temp ent...
            e->addObject(sb);                   // put it in...
            undo->singleBrush = e;
        }
        else
        {
            undo->undoType = UNDO_WHOLEMAP;
            undo->undoMap = undoCopy();
        }
        break;
    case UNDO_GROUPS:
        undo->undoType = UNDO_GROUP;
        undo->undoGroup = groups->copy();
        break;
    default:
        undo->undoType = UNDO_WHOLEMAP;
        undo->undoMap = undoCopy();
        break;
    }
}

//UNDO
void map::Undo(map *old, map **newm)
{
    Entity *e;
    Entity *eU;
    SetBrush *b;
    SetBrush *n;
    map *hold;
    int c;
    bool found;

    *newm = NULL;      // assume no new whole map
    if (!old->undo)
        return;

    // figure out what to do...
    switch (old->undo->undoType)
    {
    case UNDO_WHOLEMAP:
        hold = old->undo->undoMap;
        old->undoGetRest(hold);
        for (e = hold->objects.p_next; e != &hold->objects; e = e->p_next)
        {
            LoopProblem("UNDOWHOLEE");
            for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
                b->calcWindings();
        }
        delete old->undo;
        old->undo = NULL;
        *newm = hold;
        break;
    case UNDO_SINGLEBRUSH:
        // confirm that original ent is there
        e = old->undo->singleBrush->undoCameFrom;
        found = old->lookForObject(e);
        // if the original entity is gone, then add the entity back in
        // with the saved brush (b/c it was the only brush...
        if (!found)
        {
            old->undo->singleBrush->undoCameFrom = NULL;
            Entity *e2 = old->undo->singleBrush;
            e2->p_prev = e2->p_next = NULL;
            addObject(e2);
            e2->objects.p_next->calcWindings();
            old->undo->singleBrush = NULL;
        }
        else
        {
            // now look for the brush
            b = old->undo->singleBrush->objects.p_next;
            if (b != &old->undo->singleBrush->objects)
            {
                SetBrush *tBrush;
                bool searchResult = e->lookForBrush(b->undoCameFrom);
                // not there, just add it in
                if (!searchResult)
                {
                    b->undoCameFrom = NULL;
                    b->p_prev = b->p_next = NULL;
                    e->addObject(b);
                    b->setParent(e);
                    b->calcWindings();
                }
                else
                {
                    tBrush = b->undoCameFrom;
                    b->undoCameFrom = NULL;
                    e->removeObject(tBrush);
                    delete tBrush;
                    //b->p_prev = b->p_next = NULL;
                    old->undo->singleBrush->removeObject(b);
                    e->addObject(b);
                    b->setParent(e);
                    b->calcWindings();
                    old->undo->singleBrush->freeObjects(false);
                    delete old->undo->singleBrush;
                    old->undo->singleBrush = NULL;
                }
            }
        }
        delete old->undo;
        old->undo = NULL;
        break;
    case UNDO_MULTIBRUSH:
        // confirm that original ent is there
        for (c = 0; c < old->undo->numMultiBrush; c++)
        {
            e = old->undo->multiBrush[c]->undoCameFrom;
            eU = old->undo->multiBrush[c];

            found = old->lookForObject(e);
            // if the original entity is gone, then add the whole entity back in
            // with the saved brush (b/c it was the only brush...
            if (!found)
            {
                old->undo->multiBrush[c]->undoCameFrom = NULL;
                addObject(old->undo->multiBrush[c]);
                Entity *ent = old->undo->multiBrush[c];

                for (b = ent->objects.p_next; b != &ent->objects; b = b->p_next)
                {
                    b->calcWindings();
                }
                old->undo->multiBrush[c] = NULL;
            }
            else
            {
                for (b = eU->objects.p_next; b != &eU->objects; b = n)
                {
                    n = b->p_next;

                    if (b)
                    {
                        SetBrush *tBrush;
                        bool searchResult = e->lookForBrush(b->undoCameFrom);
                        // not there, just add it in
                        if (searchResult == false)
                        {
                            b->undoCameFrom = NULL;
                            eU->removeObject(b);
                            e->addObject(b);
                            b->setParent(e);
                            b->calcWindings();
                        }
                        else
                        {
                            tBrush = b->undoCameFrom;
                            b->undoCameFrom = NULL;
                            e->removeObject(tBrush);
                            delete tBrush;
                            eU->removeObject(b);
                            e->addObject(b);
                            b->setParent(e);
                            b->calcWindings();
                        }
                    }
                }
            }
            if (old->undo->multiBrush[c])
            {
                old->undo->multiBrush[c]->freeObjects(false);
                delete old->undo->multiBrush[c];
                old->undo->multiBrush[c] = NULL;
            };
        };
        delete[] old->undo->multiBrush;
        old->undo->multiBrush = NULL;
        delete old->undo;
        old->undo = NULL;
        break;
    case UNDO_GROUP:
        delete old->groups;
        old->groups = old->undo->undoGroup;
        delete old->undo;
        old->undo = NULL;
        groupWindow->RedrawReload();
        break;

    };
    // caller needs to delete original ptr and substitute returned object...
}

void map::undoGetRest(map *cm)
{
    STRNCPY(cm->filename,filename);
    STRNCPY(cm->texName,texName);
    STRNCPY(cm->texPath,texPath);
    cm->favorites = favorites;
    cm->favs = favs;
    cm->numf = numf;
    cm->bests = bests;
    cm->mapTextures = mapTextures;
    cm->Wads = Wads;

    cm->numWads = numWads;
    cm->numMapTextures = numMapTextures;

    favs = NULL;
    numf = 0;

    Wads = NULL;
    favorites = NULL;
    bests = NULL;
    mapTextures = NULL;
    numWads = 0;
    numMapTextures = 0;

    dirty = 0;
    oldselection = NULL;
    current = NULL;
    Entity *e;
    Entity *n;
    for (e = objects.p_next; e != &objects; e = n)
    {
        LoopProblem("UNDOGETRESTME");
        n = e->p_next;
        removeObject(e);
        delete e;
    }
    count = 0;
    delete groups;
    delete clipper_i;
    groups = NULL;
    clipper_i = NULL;
}

map *map::undoCopy()
{
    int i;
    map *cm = new map;
    if (!cm)
    {
        syserror(const_cast<char *> ("Couldn't copy map!"));
        return 0;
    }

    delete[] cm->Wads;
    delete cm->clipper_i;
    delete cm->groups;
    cm->Wads = NULL;
    cm->clipper_i = NULL;
    cm->groups = NULL;
// Pointers...
//	cm->oldselection = NULL; // ???
    cm->current = NULL;    // deal with current, if any, when doing copying...

// Stuff that must be copied...
    cm->groups = groups->copy();
    cm->clipper_i = clipper_i->copy();
    Entity *e;
    Entity *n;
    for (e = objects.p_next; e != &objects; e = e->p_next)
    {
        LoopProblem("UNDOCOPYM");
        n = e->undoCopy();
        //e->
        cm->addObject(n);

        if (current && (e == current))
            cm->current = n;

        if (world && (e == world))
            cm->world = n;
    };

    // Numbers...
    cm->count = count;
    cm->dirty = dirty;
    for (i = 0; i < 3; i++)
    {
        cm->minz[i] = minz[i];
        cm->maxz[i] = maxz[i];
    }
    for (i=0; i < MAX_CAMERAS; i++)
    {
        VectorCopy(eye[i],cm->eye[i]);
        cm->angles[i].roll  = angles[i].roll;
        cm->angles[i].pitch = angles[i].pitch;
        cm->angles[i].yaw   = angles[i].yaw;
    }
    cm->cureye = cureye;
    cm->curclippoint = curclippoint;

    cm->undo = NULL;

    cm->regionSet = regionSet;
    if (regionBrush)
        cm->regionBrush = regionBrush->copy();
    else
        cm->regionBrush = NULL;

    cm->mapType = mapType;
    return cm;
}

void map::MoveEyePosition(float *newpos, int lockValue)
{
    if (!lockValue)
        return;
    if(!set.Map_Read)
        return;

    set.redrawxy = 1;
    VectorCopy(newpos,xy_eye);
    VectorSubtract(xy_eye, set.lock_camera_offset, xy_eye);
}

SetBrush *map::selectedBrush()
{
    Entity *e;
    SetBrush *b;

//	if(REN_curBrush && REN_curBrush->IsSelected())
//		return REN_curBrush;

    if (!objects.p_next)
        return (SetBrush *)NULL;

    if (objects.p_next == &objects)
        return (SetBrush *)NULL;

    if (!current)
        current = world; // make world current...

    for (b = current->objects.p_next; b != &current->objects; b = b->p_next)
    {
        LoopProblem("SELBRUSH");

        if (!b->IsDrawable())
            continue;
        if (b->IsSelected())
            return b;
    }

    for (e = objects.p_next; e != &objects; e = e->p_next)
    {
        LoopProblem("SELBRUSHE");
        if (e == current)
            continue;

        if (e == world)
            continue;

        for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
        {
            LoopProblem("SELBRUSHEB");
            if (!b->IsDrawable())
                continue;

            if (b->IsSelected())
                return b;
        }
    }
    return (SetBrush*) NULL;
}

void map::removeObject(Entity *o)
{
    if (o == &objects)
        return;

    if (!o->p_next && !o->p_prev)
    {
        //	syserror ("removeObject, object not in a list");
        return; // nothing to do...
    }

    if (!o->p_next || !o->p_prev)
    {
        syserror(const_cast<char *> ("removeObject, object mislinked..."));
        return;
    }

    o->p_next->p_prev = o->p_prev;
    o->p_prev->p_next = o->p_next;
    o->p_next= o->p_prev = NULL;

    count--;

    dirty = 1;

    if (current == o)
        setCurrentEntity(world);
}

// rebuild flag determines whether to calculate the brush windings...
void map::copySelection(map **destinationMap, bool rebuild)
{
    int c, j;
    SetBrush *o;
    SetBrush *b;
    Entity *o2;
    Entity *e;
    Entity *w;
    Entity *saveCurrent;

    target_t *t;

    saveCurrent = current; // the copying can change this...

    delete *destinationMap;

    *destinationMap = new map;

    //grouped objects need to know about themselves... this fixes rotating grouped objects
    delete (*destinationMap)->groups;
    (*destinationMap)->groups = map_i[set.curmap]->groups->copy();

    w = new Entity;
    w->setModifiable(true);
    (*destinationMap)->addObject(w); // add in the world as the first object...
    (*destinationMap)->world = w;

    for (o2 = objects.p_next; o2 != &objects; o2 = o2->p_next)
    {
        if (o2 == world)
        {
            w = world;

            for (o = w->objects.p_next; o != &w->objects; o = o->p_next)
            {
                // don't copy invisible brushes...
                if (!o->IsDrawable())
                    continue;
                if (!o->IsSelected())
                    continue;
                b = o->copy();
                b->setParent((*destinationMap)->world);
                (*destinationMap)->world->addObject(b);

                if (rebuild)
                    b->calcWindings();
            }
        }
        else
        {
            if (o2->objects.p_next == &o2->objects)
                continue;

            if (o2->modifiable)
            {
                // Determine if any brushes are selected and if
                // so, copy entity and only selected brushes
                bool anySelected = false;
                for (o = o2->objects.p_next; o != &o2->objects; o = o->p_next)
                {
                    if (!o->IsDrawable())
                        continue;
                    if (!o->IsSelected())
                        continue;
                    anySelected = true;
                    break;
                }

                if (anySelected)
                {
                    e = o2->undoCopyNoBrushes();
                    e->undoCameFrom = NULL;
                    (*destinationMap)->addObject(e);

                    for (o = o2->objects.p_next; o != &o2->objects; o = o->p_next)
                    {
                        if (!o->IsDrawable())
                            continue;
                        if (!o->IsSelected())
                            continue;
                        b = o->copy();
                        b->setParent(e);
                        e->addObject(b);
                        if (rebuild)
                            b->calcWindings();
                    }
                } // No brushes in selection...
            }
            else     // Not Modifiable
            {
                // For point brushes, just check single brush
                // and copy "whole" ent
                if (!o2->objects.p_next->IsSelected()) // check the first brush for nonworld...
                    continue;

                e = o2->copy();
                (*destinationMap)->addObject(e);

                if (rebuild)
                {
                    for (o = e->objects.p_next; o != &e->objects; o = o->p_next)
                        o->calcWindings();
                }
            } // End Not-Modifiable
        }
    }
    (*destinationMap)->makeGlobalPerform(SEL_SELECT);
    (*destinationMap)->current = (*destinationMap)->world;

    setCurrentEntity(saveCurrent);

    char *targ;
    int found;
    for (o2 = (*destinationMap)->objects.p_next; o2 != &(*destinationMap)->objects; o2 = o2->p_next)
    {
        targ = o2->target;
        if (!targ || !*targ)
            continue;

        found = 0;
        for (w = (*destinationMap)->objects.p_next; !found && (w != &(*destinationMap)->objects); w = w->p_next)
        {
            if (w == o2)
                continue;

            if (!strcmpi(targ,w->targetname))
            {
                found = 1;
                break;
            }
        }

        if (!found)
            o2->removeKeyPair(const_cast<char *> ("target"));
    }

    // now go thru and count number of "target" values left?

    found = 0; // keep count...
    for (o2 = (*destinationMap)->objects.p_next; o2 != &(*destinationMap)->objects; o2 = o2->p_next)
    {
        targ = o2->target;
        if (!targ || !*targ)
            continue;

        found++;
    }
    //todo?	current = saveCurrent;
    if (found)
    {
        t = new target_t[found];
        memset(t,0,found * sizeof(target_t));

        found = 0;
        for (o2 = (*destinationMap)->objects.p_next; o2 != &(*destinationMap)->objects; o2 = o2->p_next)
        {
            targ =  o2->target;
            if (!targ || !*targ)
                continue;

            // find a slot
            c = 0;
            for (j = 0; j < found; j++)
            {
                if (strcmpi(targ,t[j].targetName) > 0)
                {
                    c = j;
                    break;
                }
                if (!strcmpi(targ,t[j].targetName))
                {
                    // already there...
                    c = -1;
                    break;
                }
            }

            if (c == -1)  // duplicate...
                continue;

            for (j = found; j > c; j--)
                t[j] = t[j-1];

            found++;

            STRNCPY(t[c].targetName,targ);
        }

        // one last pass, use the index to set the target number...
        for (o2 = (*destinationMap)->objects.p_next; o2 != &(*destinationMap)->objects; o2 = o2->p_next)
        {
            targ = o2->target;
            if (!targ || !*targ)
                continue;

            for (j = 0; j < found; j++)
            {
                if (!strcmpi(t[j].targetName,targ))
                {
                    o2->setKey(const_cast<char *> ("target"), Str(const_cast<char *> ("t%i"), j));
                    break;
                }
            }
        }
        for (o2 = (*destinationMap)->objects.p_next; o2 != &(*destinationMap)->objects; o2 = o2->p_next)
        {
            targ = o2->targetname;
            if (!targ || !targ[0])
                continue;

            for (j = 0; j < found; j++)
            {
                if (!strcmpi(t[j].targetName,targ))
                {
                    o2->setKey(const_cast<char *> ("targetname"), Str(const_cast<char *> ("t%i"), j));
                    break;
                }
            }
        }
        delete[] t;
    }
}

void map::pasteSelection()
{
    SetBrush *o;
    Entity *o2;
    Entity *src;
    Entity *w;
    Entity *saveCurrent;

    if (!copymap)
        return;

    dirty = 1;

    int newindex = 0;
    if (groupWindow)
    {
        newindex = groupWindow->GroupList->GetSelIndex();

        if (newindex < 0 || newindex >= groupWindow->GroupList->GetCount()) // name field not one of the groups...
            newindex = 0; // put in default if no group is current...

        if (set.group_mode && !groups->GetVisible(newindex))
        {
            int retval;
            retval = MessageBox(0,"Pasting to an invisible group, make group visible?",
                                "BSP - Paste",
                                MB_YESNO | MB_ICONQUESTION);

            if (retval == IDYES)
            {
                groups->SetVisible(newindex,true);
                groupWindow->RedrawReload();
            };
            UpdateMapVisibility();
        };
    };

    saveCurrent = current;

    // deselect all! // this alters the current entity... bad...
    // maybe just go through and set the flags and then set to world?

    makeGlobalPerform(SEL_DESELECT);

    // offset...
    sb_translate[0] = (float) set.clone_delta_x;
    sb_translate[1] = (float) set.clone_delta_y;
    sb_translate[2] = 0;

    char *t;
    int maxt = 0;
    int tval;
    if (objects.p_next != &objects)
    {
        for (o2 = objects.p_next; o2 != &objects; o2 = o2->p_next)
        {
            if (o2 == world)
                continue;

            t = o2->targetname;
            if (!t || t[0] != 't')
                continue;

            tval = atoi (t+1);
            if (tval > maxt)
                maxt = tval;
        }
    };
    maxt++;
    char	name[80];

    for (src = copymap->objects.p_next; src != &copymap->objects; src = src->p_next)
    {
        if (src == copymap->world)
        {
            w = world;

            SetBrush *b;
            for (b = src->objects.p_next; b != &src->objects; b = b->p_next)
            {
                o = b->copy();
                o->setParent(w);
                o->translate();
                o->calcWindings();
                o->setSelected(true);
                o->group = newindex;
                w->addObject(o);
            };
        }
        else
        {
            o2 = src->copy();

            t = o2->targetname;
            if (t && (t[0] == 't'))
            {
                tval = atoi (t+1);
                sprintf(name,"t%i",tval + maxt);
                o2->setKey(const_cast<char *> ("targetname"), name);
            };
            t = o2->target;
            if (t && (t[0] == 't'))
            {
                tval = atoi (t+1);
                sprintf(name,"t%i",tval + maxt);
                o2->setKey(const_cast<char *> ("target"), name);
            };

            for (o = o2->objects.p_next; o != &o2->objects; o = o->p_next)
            {
                o->setSelected(true);
                o->translate();
                o->group = newindex;
                o->calcWindings();
            };
            addObject(o2);
        };
    };

    // set current entity to world...
    setCurrentEntity(saveCurrent);
}

void map::getCenter(vec3_t ctr)
{
    int k;
    int usesel;
    Entity *ent;
    SetBrush *brush;
    vec3_t mins, maxs;

    vec3_t cmin, cmax;

    cmin[0] = TMAX;
    cmin[1] = TMAX;
    cmin[2] = TMAX;
    cmax[0] = -TMAX;
    cmax[1] = -TMAX;
    cmax[2] = -TMAX;

    usesel = 1;
    if (!numSelected())
        usesel = 0;

    for (ent = objects.p_next; ent != &objects; ent = ent->p_next)
    {
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            if (!brush->IsDrawable())
                continue;

            if (usesel && !brush->IsSelected()) // skip it if not selected...
                continue;

            brush->getMins(mins,maxs);

            for (k = 0; k < 3; k++)
            {
                if (mins[k] < cmin[k])
                    cmin[k] = mins[k];
                if (maxs[k] > cmax[k])
                    cmax[k] = maxs[k];
            };
        }
    }

    ctr[0] = (cmin[0] + cmax[0])/2.0f;
    ctr[1] = (cmin[1] + cmax[1])/2.0f;
    ctr[2] = (cmin[2] + cmax[2])/2.0f;
}

void map::makeUnselectedPerform(int sel)
{
    Entity *ent;
    SetBrush *brush;

    dirty = 1;

    for (ent = objects.p_next; ent != &objects; ent = ent->p_next)
    {
        LoopProblem("MUSPE");
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            LoopProblem("MUSPB");
            if (!brush->IsDrawable())
                continue;

            if (brush->IsSelected()) // skip selected ones...
                continue;

            switch(sel)
            {
            case SEL_CALCWINDINGS:
                brush->calcWindings();
                break;
            case SEL_LOCK:
                if (ent->modifiable)
                    brush->setLocked(true);
                break;
            case SEL_UNLOCK:
                if (ent->modifiable)
                    brush->setLocked(false);
                break;
            case SEL_SETFILTERED:
                brush->setFiltered();
                break;
            case SEL_SETREGIONED:
                brush->setRegioned();
                break;
            case SEL_UNSETREGIONED:
                brush->setRegioned(false);
                break;
            case SEL_SETREGIONEDPARTIAL:
                brush->setRegionedPartial();
                break;
            case SEL_SELECT:
                brush->setSelected(true);
                break;
            case SEL_TRANSLATE:
                brush->translate();
                break;
            case SEL_FEETTOFLOOR:
                brush->feetToFloor();
                break;
            case SEL_DESELECT:
                brush->setSelected(false);
                break;
            case SEL_ADDTOBBOX:
                brush->addToBBox();
                break;
            case SEL_TRANSFORM:
                brush->transform();
                break;
            case SEL_FLIPNORMALS:
                brush->flipNormals();
                break;
            case SEL_MOVETOENTITY:
                brush->moveToEntity();
                break;
            case SEL_REMOVE:
                brush->setInvalid(true);
                break;
            case SEL_SELECTCOMPLETE:
                brush->selectComplete();
                break;
            case SEL_SELECTPARTIAL:
                brush->selectPartial();
                break;
            case SEL_CARVEBYCLIPPER:
                brush->carveByClipper();
                break;
            case SEL_SPLITBYCLIPPER:
                brush->splitByClipper();
                break;
            case SEL_CAMERARENDERSELF:
                brush->CameraRenderSelf();
                brushCount++;
                break;
            default:
                syserror(const_cast<char *> ("MUSPerform Unknown method..."));
            };
        }
    }

    if ((sel == SEL_CARVEBYCLIPPER) ||
            (sel == SEL_SPLITBYCLIPPER) ||
            (sel == SEL_REMOVE))
    {
        cleanUpInvalidObjects();
    }
}

void map::getSelectedCenter()
{

    //init addToBBox
    sb_ctr[0] = sb_ctr[1] = sb_ctr[2] = 0;
    BBoxCount = 0;
    //accumulate bctrs
    for (Entity *ent = objects.p_next; ent != &objects; ent = ent->p_next)
    {
        for (SetBrush *brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            if (!brush->IsDrawable())
                continue;
            if (!brush->IsSelected())
                continue;

            brush->addToBBoxCtr();
        }
    }
    //get center
    if(BBoxCount > 1)
        VectorScale(sb_ctr, 1/(float)BBoxCount, sb_ctr);
}

void map::makeSelectedPerform(int sel)
{
    Entity *ent, *e2;
    SetBrush *brush, *b2;

    dirty = 1;

    for (ent = objects.p_next; ent != &objects; ent = e2)
    {
        e2 = ent->p_next;
        LoopProblem("MSPE");
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = b2)
        {
            b2 = brush->p_next;		//these are in case brush gets deleted... wooo.
            LoopProblem("MSPB");

            if (!brush->IsDrawable())
                continue;

            if (!brush->IsSelected())
                continue;

            switch(sel)
            {
            case SEL_LOCK:
                if (ent->modifiable)
                    brush->setLocked(true);
                break;
            case SEL_UNLOCK:
                if (ent->modifiable)
                    brush->setLocked(false);
                break;
            case SEL_CALCWINDINGS:
                brush->calcWindings();
                break;
            case SEL_SETFILTERED:
                brush->setFiltered();
                break;
            case SEL_SETREGIONED:
                brush->setRegioned();
                break;
            case SEL_UNSETREGIONED:
                brush->setRegioned(false);
                break;
            case SEL_SETREGIONEDPARTIAL:
                brush->setRegionedPartial();
                break;
            case SEL_SELECT:
                brush->setSelected(true);
                break;
            case SEL_TRANSLATE:
                brush->translate();
                break;
            case SEL_FEETTOFLOOR:
                brush->feetToFloor();
                break;
            case SEL_DESELECT:
                brush->setSelected(false);
                break;
            case SEL_ADDTOBBOX:
                brush->addToBBox();
                break;
            case SEL_TRANSFORM:
                brush->transform();
                break;
            case SEL_FLIPNORMALS:
                brush->flipNormals();
                break;
            case SEL_MOVETOENTITY:
                brush->moveToEntity();
                break;
            case SEL_REMOVE:
                brush->setInvalid(true);
                cleanUpInvalidObjects();
                break;
            case SEL_SELECTCOMPLETE:
                brush->selectComplete();
                break;
            case SEL_SELECTPARTIAL:
                brush->selectPartial();
                break;
            case SEL_CARVEBYCLIPPER:
                brush->carveByClipper();
                cleanUpInvalidObjects();
                break;
            case SEL_SPLITBYCLIPPER:
                brush->splitByClipper();
                cleanUpInvalidObjects();
                break;
            case SEL_CAMERARENDERSELF:
                brush->CameraRenderSelf();
                brushCount++;
                break;

            default:
                syserror(const_cast<char *> ("MSPerform Unknown method..."));
            }
        }
    }
}

void map::makeSelectedXYDraw(HDC hdc,int index,int stopAt, Group * /*g*/)
{
    //Entity *saveCurrent;
    int total;
    total = 0;

    Entity *ent;
    SetBrush *brush;

    dirty = 1;

    for (ent = objects.p_next; ent != &objects; ent = ent->p_next)
    {
        LoopProblem("MSXYE");
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            LoopProblem("MSXYB");

            if (!brush->IsDrawable())
                continue;

            if (!brush->IsSelected())
                continue;

            brush->XYDrawSelf(hdc,xy_eye,index);
            total++;
            // if we are to stop, then stop when total hits or exceeds the value...
            if (stopAt > 0 && total >= stopAt)
                //current = saveCurrent;
                return;
        }
    }
}

void map::snapSelected(int mode)
{
    int	k;
    int snapEntities;
    vec3_t mins,maxs,ctr,newctr;

    Entity *ent;
    SetBrush *brush;

    dirty = 1;
    snapEntities = -1;  // don't know...

    for (ent = objects.p_next; ent && (ent != &objects); ent = ent->p_next)
    {
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            if (!brush->IsDrawable())
                continue;
            if (!brush->IsSelected())
                continue;

            // do it...
            // don't snap fixed size brushes, like light...
            if (!brush->parent->modifiable)
            {
                if (snapEntities == -1)    // don't know
                {
                    int retval = MessageBox(0,"Snap Entities to Their Centers?",
                                            "BSP - Snap Selected",MB_YESNO | MB_ICONQUESTION);
                    snapEntities = retval == IDYES ? 1 : 0;
                }
                if (!snapEntities)
                    continue;

                // Calculate the Center
                brush->getMins(mins,maxs);
                for (k = 0; k < 3; k++)
                {
                    ctr[k] = (mins[k] + maxs[k])/2.0f;
                    newctr[k] = (float) snapToGrid(ctr[k]);
                    sb_translate[k] = newctr[k] - ctr[k];
                }

                brush->translate();
            }
            else
            {
                if(mode==SNAP_SELF)
                    brush->snapSelf();
                else if(mode==SNAP_PLANES)
                    brush->snapPlanes();
            }
        }
    }

    cleanUpInvalidObjects();
}

void map::addObject(Entity *ent)
{
    if (ent->p_prev || ent->p_next)
    {
        syserror(const_cast<char *> ("addObject, already in a list"));
        delete ent;
        return;
    }

    ent->owner = this;

    // Link in
    ent->p_next = objects.p_next;
    ent->p_prev = &objects;
    objects.p_next->p_prev = ent;
    objects.p_next = ent;
    count++;

    dirty = 1;
}

// return false if object not in current map
bool map::lookForObject(Entity *e)
{
    if (e && objects.p_next != &objects)
    {
        for (Entity *ent = objects.p_next; ent && (ent != &objects); ent = ent->p_next)
        {
            LoopProblem("MLOOKFORE");
            if (e == ent)
                return true;
        }
    }
    return false;
}

int CountWads(char *path)
{
    int count = 1;
    for (char *cur = path; cur && *cur; cur++)
    {
        if (*cur == ';')
            count++;
    }
    return count;
}

//ben: changed from 2000
#define CHUNKSIZE 0x8000

textureEntry firstEntry;

void map::GatherTextureDirectories()
{
    int	i;
    Entity *ent;
    SetBrush *brush;
    face_t *f;

    memset(&firstEntry,0,sizeof(textureEntry));

    char **EntryList;
    int maxEntries = 64;
    int EntrySize = 64;
    int curEntries = 0;
    EntryList = new char *[maxEntries];
    memset(EntryList,0,maxEntries*sizeof(char *));
    for (i = 0; i < maxEntries; i++)
    {
        EntryList[i] = new char[EntrySize];
        memset(EntryList[i],0,EntrySize * sizeof(char));
    };

    int left, right, current;
    int done, index;
    int testVal;

    for (ent = objects.p_next; ent && (ent != &objects); ent = ent->p_next)
    {
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            for (f = brush->faces.p_next; f != &brush->faces; f = f->p_next)
            {
                LoopProblem("gtd");

                left = 0;        // first
                right = curEntries;   //last+1 // never get's tested...

                done = 0;
                index = -1;

                current = (left+right)/2;

                while (!done)
                {
                    char *tP;
                    tP = EntryList[current];

                    testVal = strcmpi(f->texture.basepath,tP);
                    // check for exact match...
                    if (testVal == 0)
                    {
                        index = current;
                        break;
                    }
                    else if (testVal > 0)
                    {
                        left = current;
                        current = (left+right)/2;
                    }
                    else if (testVal < 0)
                    {
                        right = current;
                        current = (left+right)/2;
                    };
                    if ((right - left) <= 1)
                    {
                        if (!strcmpi(f->texture.basepath,EntryList[left]))
                        {
                            index = left;
                        };
                        done = 1;
                    };
                };

                if (index >= 0) // if found it, then skip
                    continue;

                if (index == -1)   // not found, just insert
                {
                    if (curEntries == 0)   // empty list, put in first slot
                    {
                        strcpy(EntryList[curEntries++],f->texture.basepath);
                    }
                    else
                    {
                        int slot = -1;
                        int w;
                        int tVal;
                        for (w = 0; w < curEntries; w++)
                        {
                            tVal = strcmpi(f->texture.basepath,EntryList[w]);
                            if (tVal < 0)
                            {
                                slot = w;
                                break;
                            };
                        };

                        if (slot == -1)
                            slot = curEntries;  // put at end...

                        for (w = curEntries; w > slot; w--)
                        {
                            strcpy(EntryList[w],  EntryList[w-1]);
                        }
                        strcpy(EntryList[slot],f->texture.basepath);
                        curEntries++;
                    };
                };
            };
        };
    };

    // now fill in structure
    textureEntry *t;
    textureEntry *lastT;
    lastT = NULL;
    for (i = curEntries - 1; i >= 0; i--)
    {
        t = new textureEntry(EntryList[i]);
        t->next = lastT;

        firstEntry.next = t;

        lastT = t;
    }

    for (i = 0; i < maxEntries; i++)
        delete[] EntryList[i];
    delete[] EntryList;
}

extern void Parse_NextLine(char **text);
//
// readMapFile
//
bool map::readMapFile(char *fname)
{
    char outstr[MAX_PATH];
    char *curwad;
    int itemsLeft;
    Entity *newItem;
    Entity *ent;
    SetBrush *brush;
    face_t *f;
    int c,i;
    unsigned char *curpos;
    char tempName[MAX_GROUP_NAMESIZE+10] = "";

    sprintf(outstr,"Loading [%s]",fname);
    status->SetText(outstr);
    status->Redraw();

    strncpy(loadName,fname,sizeof(loadName));
    groups->ClearGroups();

    // open map file
    int size = 0;
    dat = file_get_contents(fname,&size);
    if (dat <= 0)
    {
        MessageBox(client->hwnd,"ReadMapFile:  Couldn't open map...","BSP",MB_ICONEXCLAMATION);
        return false;
    }

    curpos = dat;

    if((c = Parse_BSPGroups((char*)curpos)) >= 0)
    {
        Parse_NextLine((char**)&curpos);
    }

    if (c > 0)
    {
        for (i = 0; i < c; i++)
        {

            groupinfo_t gi;
            if(Parse_BSPGroupInfo((char*)curpos, &gi))
            {
                if (i)   // skip the first one...
                {
                    groups->AddGroup(gi.name);
                    if (groupWindow)
                    {
                        char str[MAX_PATH];
                        groups->GetLongName(i,str);
                        groupWindow->GroupList->InsertString(str,i);
                    }
                }
                else
                {
                    groups->Rename(0, gi.name);	//always has one group...
                }
                groups->SetGroupColor(i,gi.color);

                if(gi.visible == 2)
                {
                    groups->objects[i].isSpacer = 1;
                }
                else
                {
                    groups->SetVisible(i, gi.visible);
                }
                Parse_NextLine((char**)&curpos);
            }
        }
    }
    //favorites
    favs = NULL;
    numf = 0;

    if((c = Parse_BSPFavorites((char*)curpos)) >= 0)
    {
        Parse_NextLine((char**)&curpos);
    }

    if (c > 0)
    {
        c = min(c, set.tex_numfav); // clamp
        numf = c;
        favs = new char *[c];
        memset(favs,0,c*sizeof(char *));

        if(*curpos == '/')
        {
            curpos += 2;	// skip "//"

            for(i = 0; i < c; i++)
            {
                if(!Parse_BSPNextFav((char**)&curpos,tempName,sizeof(tempName)))
                    break;
                favs[i] = new char[64];
                strcpy(favs[i],tempName);
            }
        }
        Parse_NextLine((char**)&curpos);
    }

    //cameras
    if((c = Parse_BSPCameras((char*)curpos)) >= 0)
    {
        Parse_NextLine((char**)&curpos);
    }
    if (c > 0)
    {
        for (i = 0; i < c; i++)
        {
            camerainfo_t ci;

            if(!Parse_BSPCamInfo((char*)curpos, &ci))
                break;

            eye[i][0] = ci.x;
            eye[i][1] = ci.y;
            eye[i][2] = ci.z;
            angles[i].roll  = 0;//DEG2RAD(rr);
            angles[i].pitch = DEG2RAD(ci.pitch);
            angles[i].yaw   = DEG2RAD(ci.yaw);

            Parse_NextLine((char**)&curpos);
        }
    }

    // this sets map type
    if((c = Parse_BSPMapType((char*)curpos)) >= 0)
        Parse_NextLine((char**)&curpos);
    else
        c = set.game_mode;				//default to current gametype

    mapType = min(2,max(0,c));			// bound it...


    //
    // read ents/brushes
    //
    Tokenizer script((char*)dat);
    ent_count = 0;
    numsb = 0;
    itemsLeft = 0;
    bool readWorld = false;
    bool error = false;
    while(true)
    {
        if (ent_count++ == 0)
            sprintf(outstr,"Loading %s, World Entity, [b%ld]...  ",loadName,numsb);
        else
            sprintf(outstr,"Loading %s, [e%ld/b%ld]...  ",loadName,ent_count,numsb);

        status->SetText(outstr);
        status->Redraw();

        newItem = new Entity;
        itemsLeft = newItem->initFromTokens(this,script);
        if (itemsLeft <= 0)
        {
            if(itemsLeft < 0)
                error = true;
            delete newItem;
            break;
        }
        addObject(newItem);
        if (!readWorld)
        {
            readWorld = true;
            world = newItem;
        }
    }
    delete [] dat;
    dat = NULL;

    if(error)
    {
        MessageBox(client->hwnd,"Failed to open map file!","BSP",MB_ICONEXCLAMATION);
        return false;
    }

    *loadName = 0;

    setCurrentEntity(world);

    // load the apropriate texture wad
    curwad = current->valueForQKey(const_cast<char *> ("wad"));
    // FIXME use saveGame?
    if (!curwad || !curwad[0] && (set.game_mode != 2))
    {
        curwad = set.default_wad;
        world->setKey(const_cast<char *> ("wad"), set.default_wad);
    }

    if (curwad && *curwad && (set.game_mode != 2))
    {
        char WadList[MAX_WADS][128];
        int counter;
        int nW = CountWads(curwad);
        char *start = curwad;
        char *end;
        for (counter = 0; counter < nW; counter++)
        {
            // split up curwad...

            for (end = start; end && *end && (*end != ';'); end++);

            strncpy(WadList[counter],start,(int)(end-start));
            WadList[counter][(int)(end-start)] = '\0';

            start = ++end;
            ExtractFileBase(WadList[counter],outstr,false);
            texWad *Wad = texWindow->QueryWadLoaded(outstr);  // scan through list!
            // if found it, just change to it...
            if (!Wad)
            {
                Wad = new texWad;
                texWindow->AddWad(Wad,outstr,WadList[counter],set.game_mode);
            }
            Wads[counter] = Wad;
        }
        numWads = nW;

        texWindow->SetUpMap(this);
    }

    // For QII support...
    if (set.game_mode == 2)
    {
        numWads = 0;
        if (set.loadInUse)
        {
            texWindow->ScanInUse(this);
        }
        else
        {
            GatherTextureDirectories();
            textureEntry *cur;
            textureEntry *old;
            cur = firstEntry.next;
            int SaveRead;
            SaveRead = set.Map_Read;
            set.Map_Read = 0;
            while (cur)
            {
                old = cur;
                ExtractFileBase(cur->dirName,outstr,false);
                cur->dirName[strlen(cur->dirName)-1] = '\0';

                texWad *Wad = texWindow->QueryWadLoaded(outstr);  // scan through list!
                // if found it, just change to it...
                if (!Wad)
                {
                    Wad = new texWad;
                    texWindow->AddWad(Wad,outstr,cur->dirName,set.game_mode);
                }
                Wads[numWads] = Wad;
                numWads++;

                cur = cur->next;
                delete old;
            }

            set.Map_Read = SaveRead;
            firstEntry.next = NULL;
        }
        curwad = current->valueForQKey(const_cast<char *> ("_pakload"));

        if (curwad && curwad[0] && set.pakOk)
        {
            char WadList[MAX_WADS][128];
            int counter;
            int nW = CountWads(curwad);
            char *start = curwad;
            char *end;
            for (counter = 0; counter < nW; counter++)
            {
                // split up curwad...

                for (end = start; end && *end && (*end != ';'); end++);

                strncpy(WadList[counter],start,(int)(end-start));
                WadList[counter][(int)(end-start)] = '\0';

                start = ++end;
                ExtractFileBase(WadList[counter],outstr,false);
                texWad *Wad;
                Wad = texWindow->QueryWadLoaded(outstr);  // scan through list!
                // if found it, just change to it...
                if (!Wad)
                {
                    Wad = new texWad;
                    strncpy(Wad->currentWad,outstr,sizeof(Wad->currentWad)); // may get overwritten anyway...
                    Wad->WadType   = QUAKE_WAD;
                    Wad->bmpsCount = 0;
                    Wad->textures  = NULL;
                    Wad->nextWad 	= texWindow->firstWad;
                    texWindow->firstWad 		= Wad;
                    texWindow->currentWadPtr 	= Wad;
                    texWindow->LoadPakDirectory(outstr,set.pak_file); // load it in...
                }
                Wads[numWads++] = Wad;
            }
        }

        //TODO: this doesnt work! isnt good at loading texture directory...
        curwad = current->valueForQKey(const_cast<char *> ("_dirload"));

        if (curwad && curwad[0])
        {
            char WadList[MAX_WADS][128];
            int counter;
            int nW = CountWads(curwad);
            char *start = curwad;
            char *end;
            for (counter = 0; counter < nW; counter++)
            {
                // split up curwad...
                end = start;
                while (end && *end && (*end != ';'))
                {
                    end++;
                }
                strncpy(WadList[counter],start,(int)(end-start));
                WadList[counter][(int)(end-start)] = '\0';

                start = ++end;
                ExtractFileBase(WadList[counter],outstr,false);
                texWad *Wad;

                char tmp[MAX_PATH],testpath[MAX_PATH];
                sprintf(tmp, "%s\\%s\\",set.texture_path,outstr);
                expand_path(tmp,testpath);
                struct _finddata_t ffb;
                memset(&ffb,0,sizeof(ffb));
                int ff = _findfirst(testpath,&ffb);
                int fn, dirfound=0;
                if(ff != -1)
                {
                    while((fn = _findnext(ff, &ffb)))
                    {
                        if(ffb.attrib & _A_SUBDIR)
                        {
                            dirfound = 1;
                            break;
                        }
                    }
                    _findclose(ff);
                }
                else
                {
                    syserror(const_cast<char *> ("readMapFile: error processing _dirload key"));
                }
                if (dirfound)
                {
                    Wad = texWindow->QueryWadLoaded(outstr);  // scan through list!
                    // if found it, just change to it...
                    if (!Wad)
                    {
                        Wad = new texWad;
                        STRNCPY(Wad->currentWad,outstr); // may get overwritten anyway...
                        Wad->WadType   = QUAKE_WAD;
                        Wad->bmpsCount = 0;
                        Wad->textures  = NULL;
                        Wad->nextWad 	= texWindow->firstWad;
                        texWindow->firstWad 		= Wad;
                        texWindow->currentWadPtr 	= Wad;
                        texWindow->LoadTextureDirectory(set.texture_path,outstr); // load it in...
                    }
                    Wads[numWads++] = Wad;
                }
                else
                {
                    sprintf(testpath,"Path [%s\\] does not exist in [%s\\], skipping...",outstr,set.texture_path);
                    MessageBox(0,testpath,"BSP Load Map",MB_OK);
                }
            }
        }

        texWindow->SetUpMap(this);
        texWindow->ChangeToMap(this);
    }


    bool fixPaths = false;
    if ((set.game_mode == 2) && loadMessageGiven)
    {
        // See if user wants to try and 'convert' the map...
        int retval;
        retval = MessageBox(0, const_cast<char *> ("Load old .wad files and update texture references ? "), const_cast<char *> ("BSP Load Map"), MB_YESNO | MB_ICONEXCLAMATION);
        curwad = current->valueForQKey(const_cast<char *> ("wad"));
        if ((retval == IDYES) && curwad && curwad[0])
        {
            fixPaths = true;
            int saveGame;
            saveGame = set.game_mode;
            set.game_mode = 0;
            char WadList[MAX_WADS][128];
            int counter;
            int nW = CountWads(curwad);
            char *start = curwad;
            char *end;
            for (counter = 0; counter < nW; counter++)
            {
                // split up curwad...
                end = start;
                while (end && *end && (*end != ';'))
                    end++;

                strncpy(WadList[counter],start,(int)(end-start));
                WadList[counter][(int)(end-start)] = '\0';

                start = ++end;
                ExtractFileBase(WadList[counter],outstr,false);
                texWad *Wad;
                Wad = texWindow->QueryWadLoaded(outstr);  // scan through list!
                // if found it, just change to it...
                if (!Wad)
                {
                    Wad = new texWad;
                    texWindow->AddWad(Wad,outstr,WadList[counter],set.game_mode);
                }
                Wads[numWads+counter] = Wad;
            }
            set.game_mode = saveGame;
            numWads += nW;
            texWindow->SetUpMap(this);
        }
    }

    if (set.game_mode == 2)
    {
        texInfo_t *tx;
        status->SetText("Updating default texture values...");
        status->Redraw();

        for (ent = objects.p_next; ent && (ent != &objects); ent = ent->p_next)
        {
            for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
            {
                for (f = brush->faces.p_next; f != &brush->faces; f = f->p_next)
                {
                    LoopProblem("gtd2");
                    tx = texWindow->GetTextureByName(this,f->texture.texture);	// try to load
                    if (!tx)
                        continue;

                    if (fixPaths)
                        sprintf(f->texture.basepath,"%s%c",tx->basepath,set.rel_path_separator);

                    f->dcontents = tx->def_contents;
                    f->dflags    = tx->def_flags;
                    f->dvalue    = tx->def_value;

                    if (!f->defaults)
                        continue;

                    f->texture.contents = f->dcontents;
                    f->texture.flags    = f->dflags;
                    f->texture.value    = f->dvalue;
                }
            }
        }
        texWindow->SetUpMap(this);
    }

    if (groupWindow)
    {
        UpdateMapVisibility();
        groupWindow->RedrawReload();
    }

    dirty = 0;
    return true;
}

void map::mergeMapFile(char *fname)
{
    char outstr[80];
    unsigned char *dat;
    int itemsLeft;
    Entity *newItem;
    Entity *first = 0;
    Entity *w;
    SetBrush *b;

    sprintf(outstr,"Merging [%s]",fname);
    status->SetText(outstr);
    status->Redraw();

    int size = 0;
    dat = file_get_contents(fname, &size);
    if(!dat)
    {
        syserror(const_cast<char *> ("MergeMapFile:  Couldn't read [%s]..."), fname);
        Msgbox  (const_cast<char *> ("MergeMapFile:  Couldn't read [%s]..."), fname);
        return;
    }

    sprintf(outstr,"Loading %s, [e%ld/b%ld]...  ",loadName,ent_count,numsb);

    Tokenizer script((char*)dat);

    ent_count = 0;

    numsb = 0;

    int firstitem = 1;
    while (1)
    {
        newItem = new Entity;
        wasBad = false;
        itemsLeft = newItem->initFromTokens(this,script);
        if (!itemsLeft)
        {
            delete newItem;
            break;
        }
        if (!firstitem)
        {
            for (b = newItem->objects.p_next; b != &newItem->objects; b = b->p_next)
                b->setSelected(true);
            addObject(newItem);
        }
        else
        {
            if (!wasBad)   // first item was bad? (only when merging...)
            {
                first = newItem;
            }
            else
            {
                delete newItem;
                first = NULL;
            }
            firstitem = 0;
        }
        sprintf(outstr,"Merging %s, [e%ld/b%ld]...  ",loadName,ent_count,numsb);
        status->SetText(outstr,true);
    }
    delete [] dat;

    *loadName = 0;

    // now put the "firstitem" brushes into the "world"
    w = world;
    if (first)
    {
        SetBrush *n;
        for (b = first->objects.p_next; b != &first->objects; b = n)
        {
            n = b->p_next;
            first->removeObject(b);
            b->setParent(w);
            b->setSelected(true);
            w->addObject(b);
        }
        delete first;
    }
    setCurrentEntity(w);
    dirty = 1;
}

int map::numSelected()
{
    int num = 0;

    if (!current)
        current = world;

    if (objects.p_next == &objects)
        return num;

    SetBrush *brush;
    for (brush = current->objects.p_next; brush != &current->objects; brush = brush->p_next)
    {
        if (brush->IsDrawable() && brush->IsSelected())
            num++;
    }
    return num;
}

int map::numSelectedTotal()
{
    int		num;
    Entity *ent;
    SetBrush *brush;

    num = 0;

    if (objects.p_next == &objects)
        return 0;


    for (ent = objects.p_next; ent && (ent != &objects); ent = ent->p_next)
    {
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            if (brush->IsDrawable() && brush->IsSelected())
                num++;
        }
    }
    return num;
}

int map::numBrushes()
{
    int		num;
    Entity   *ent;
    SetBrush *brush;

    num = 0;

    if (objects.p_next == &objects)
        return 0;


    for (ent = objects.p_next; ent != &objects; ent = ent->p_next)
    {
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
            num++;
    };
    return num;
}

void map::newMap()
{
    char outstr[MAX_PATH];
    char *curwad;

    if (groups)
    {
        groups->ClearGroups();
    }

    Entity *ent = new Entity;
    ent->initClass(const_cast<char *> ("worldspawn"));

    curwad = set.default_wad;

    char WadList[MAX_WADS][128];
    int counter;
    int nW = CountWads(curwad);
    char *start = curwad;
    char *end;
    for (counter = 0; counter < nW; counter++)
    {
        // split up curwad...

        for (end = start; end && *end && *end != ';'; end++);

        strncpy(WadList[counter],start,(int)(end-start));
        WadList[counter][(int)(end-start)] = '\0';

        start = ++end;

        ExtractFileBase(WadList[counter],outstr,false);

        texWad *Wad;
        Wad = texWindow->QueryWadLoaded(outstr);  // scan through list!
        // if found it, just change to it...
        if (!Wad)
        {
            Wad = new texWad;
            // FIXME...
            texWindow->AddWad(Wad,outstr,WadList[counter],set.game_mode);
        }
        Wads[counter] = Wad;
    }

    numWads = nW;
    if (ent)
    {
        if (set.game_mode != 2)
            ent->setKey(const_cast<char *> ("wad"), set.default_wad);
        addObject(ent);
        current = NULL;
        world = ent;
        setCurrentEntity(ent);
    }

    if (groupWindow)
    {
        groupWindow->RedrawReload();
    }

    if (texWindow)
    {
        texWindow->SetUpMap(this);
    }
    dirty = 0;
    mapType = set.game_mode;
}

Entity *map::currentEntity()
{
    return current;
}

void map::setCurrentEntity(Entity *cur)
{
    Entity *old;

    dirty = 1;

    old = current;
    current = cur;

    if (current != old)
    {
        kpWindow->newCurrentEntity(false);
    };
}

vec_t map::currentMinZ(int axis)
{
    return minz[axis];
};

vec_t map::currentMaxZ(int axis)
{
    return maxz[axis];
};

// zzz min and max are more complicated than this...
void map::setCurrentMinZ(vec3_t v)
{
    dirty = 1;

    for (int i = 0; i < 3; i++)
    {
        if (v[i] > MAP_MINZ)
            minz[i] = (float) snapToGrid(v[i]);
    }

    if (xyWindow)
    {
        int i = set.curxy;
        char str[80];
        sprintf(str,"Base: %i Ht. %i",(int)minz[xyWindow[i]->NormalAxis],
                (int)(maxz[xyWindow[i]->NormalAxis]-minz[xyWindow[i]->NormalAxis]));
        status->SetText(STATUS_HEIGHT, str, true);
    }

}
void map::setCurrentMaxZ(vec3_t v)
{
    dirty = 1;

    for (int i = 0; i < 3; i++)
    {
        if (v[i] < MAP_MAXZ)
        {
            maxz[i] = (float) snapToGrid(v[i]);
        }
    }

    if (xyWindow)
    {
        int i = set.curxy;
        char str[80];
        sprintf(str,"Base: %i Ht. %i",(int)minz[xyWindow[i]->NormalAxis],
                (int)(maxz[xyWindow[i]->NormalAxis]-minz[xyWindow[i]->NormalAxis]));
        status->SetText(STATUS_HEIGHT, str, true);
    }
}

void map::DrawOrientation(HDC hdc)
{
    if(set.angle_control == NO_ANGLE_CONTROL)
        return;

    char str_ang[40];

    HPEN pen = CreatePen(PS_SOLID,1,set.color_foreground);
    HGDIOBJ oldpen = SelectObject(hdc,pen);
    HBRUSH bColor = CreateSolidBrush(set.color_background);
    HGDIOBJ oldbrush = SelectObject(hdc,bColor);
    HGDIOBJ oldfont = SelectObject(hdc,set.font10);

    if(set.angle_control == FOV_CONTROL)
    {

        RECT rc;
        rc.left = EditSize.cx - fovsz.cx - 5;
        rc.right = rc.left + fovsz.cx;
        rc.top = 5;
        rc.bottom = fovsz.cy + 5;

        FillRect(hdc, &rc, bColor);

        HBRUSH white = CreateSolidBrush(RGB(255,255,255));		// white frame
        FrameRect(hdc, &rc, white);
        DeleteObject(white);

        int fov = (int)set.fov;//RAD2DEG(1/set.fov);
        int len = (int)sprintf(str_ang,"%d",fov);

        SetBkMode(hdc, TRANSPARENT);
        //draw "FOV" title
        SetTextColor(hdc, RGB(99,91,83));		// grayish something
        TextOut(hdc,rc.left+1, rc.top-1, "FOV", 3);

        const int slider_top = rc.top + 9;		// slider start below "FOV"
        const int slider_range = 85;			// pixel height of slide area
        const int pt90 = slider_range / 2;

        //draw triangle
        HPEN bluepen = CreatePen(PS_SOLID,1,RGB(0,0,175));
        SelectObject(hdc,bluepen);
        MoveToEx(hdc, rc.left+3,   rc.bottom-3, 0);
        LineTo(  hdc, rc.left+3+5, slider_top+8);
        LineTo(  hdc, rc.left+3+10,rc.bottom-2);
        SelectObject(hdc,pen);
        DeleteObject(bluepen);

        //line at 15
        MoveToEx(hdc, rc.right - 5, slider_top+5, 0);
        LineTo(hdc, rc.right, slider_top+5);
        //line at 90
        MoveToEx(hdc, rc.right - 5, slider_top + pt90, 0);
        LineTo(hdc, rc.right, slider_top + pt90);
        //line at 180
        MoveToEx(hdc, rc.right - 5, slider_top+slider_range, 0);
        LineTo(hdc, rc.right, slider_top+slider_range);

        fov -= 15;								// make range zero-based
        SetTextColor(hdc, RGB(255,255,0));		// yellow
        TextOut(hdc,rc.left+2, slider_top+(fov*slider_range/180), str_ang, len);
    }
    else
    {
        POINT cent;
        POINT ed,md;
        POINT side1,side2;

        cent.x = EditSize.cx - 25;
        cent.y = 25;

        float ang;
        if(set.angle_control==PITCH_CONTROL)
            ang = angles[cureye].pitch;
        else
            ang = angles[cureye].yaw;

        //	ang -= M_PI/2.0;

        ed.x = (LONG) cent.x + 20 * cos(ang);
        ed.y = (LONG) cent.y - 20 * sin(ang);

        md.x = (LONG) cent.x + 15 * cos(ang);
        md.y = (LONG) cent.y - 15 * sin(ang);

        side1.x = (LONG) cent.x + 12 *cos(ang+0.2);
        side1.y = (LONG) cent.y - 12 *sin(ang+0.2);
        side2.x = (LONG) cent.x + 12 *cos(ang-0.2);
        side2.y = (LONG) cent.y - 12 *sin(ang-0.2);

        Ellipse(hdc,cent.x-20,cent.y-20,cent.x+20,cent.y+20);

        // draw the arrow...
        MoveToEx(hdc,cent.x,cent.y,0);
        LineTo(hdc,md.x,md.y);
        LineTo(hdc,side1.x,side1.y);
        LineTo(hdc,ed.x,ed.y);
        LineTo(hdc,side2.x,side2.y);
        LineTo(hdc,md.x,md.y);

        int a = RAD2DEG(ang);
        while (a >= 360)
            a -= 360;
        while (a < 0)
            a += 360;

        sprintf(str_ang,"%d",a);
        SetBkColor(hdc,set.color_background);

        if(set.angle_control==PITCH_CONTROL)
        {
            SetTextColor(hdc,RGB(0,255,0));		// green
        }
        else
        {
            SetTextColor(hdc,RGB(255,0,0));		// red
        }

        TextOut(hdc,cent.x-5,cent.y-5,str_ang,strlen(str_ang));
    }

    SelectObject(hdc,oldfont);
    SelectObject(hdc,oldbrush);
    DeleteObject(bColor);
    SelectObject(hdc,oldpen);
    DeleteObject(pen);
}

//
void map::writeMapFile(char *fname, int writeWorldKeys,int selOnly)
{
    if ((objects.p_next == &objects) || (world && (world->objects.p_next == &world->objects)))
    {
        MessageBox(0,"Nothing to save...\nMap must have at least one world brush!",
                   "BSP - Save", MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    FILE *f = fopen (fname,"w");
    while (!f)
    {
        char filename[MAX_PATH] = "";

        OPENFILENAME ofn;
        memset(&ofn,0,sizeof(OPENFILENAME));
        ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
        ofn.hwndOwner = frame->hwnd;
        ofn.lpstrFilter = "Map Files (*.map)\0*.map\0";
        ofn.lpstrDefExt = "map";
        ofn.lpstrFile = filename;
        ofn.nMaxFile = 256;
        ofn.lpstrInitialDir = set.map_directory;
        ofn.lpstrTitle = "Output MAP File";
        ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

        if(!GetSaveFileName(&ofn))
            return;

        f = fopen(filename,"w");
        strncpy(fname,filename,sizeof(fname));
    }

    int i;
    texInfo_t *tx;

    if (writeWorldKeys)
    {
        // write group info...
        fprintf(f,"//BSPGROUPS%04i\n",groups->NumGroups());
        for (i = 0; i < groups->NumGroups(); i++)
        {
            COLORREF groupcolor = groups->GetGroupColor(i);
            fprintf(f,"//BSPGROUPINFO\"%s\" \"%i %i %i\" ",groups->GetName(i),
                    GetRValue(groupcolor), GetGValue(groupcolor), GetBValue(groupcolor));
            if (groups->IsSpacer(i))
                fprintf(f,"\"2\"\n");
            else
                fprintf(f,"\"%i\"\n",groups->GetVisible(i));

        }
        if (favorites && texWindow)   // just in case...
        {
            fprintf(f,"//BSPFAVORITES%04i\n",set.tex_numfav);
            int first = 1;
            for (i = 0; i < set.tex_numfav; i++)
            {
                tx = favorites[i];
                if (tx)
                {
                    if (first)
                    {
                        first = 0;
                        if (set.game_mode == 2)
                            fprintf(f,"//%s/%s",tx->basepath,tx->bmp);
                        else
                            fprintf(f,"//%s",tx->bmp);
                    }
                    else
                    {
                        if (set.game_mode == 2)
                            fprintf(f,";%s/%s",tx->basepath,tx->bmp);
                        else
                            fprintf(f,";%s",tx->bmp);

                    };
                };
            };
            fprintf(f,"\n");
        };
        fprintf(f,"//BSPCAMERAS%04i\n",MAX_CAMERAS);
        for (i = 0; i < MAX_CAMERAS; i++)
        {
            fprintf(f,"//\"%3.1f %3.1f %3.1f %i %i %i\"\n",
                    eye[i][0],eye[i][1],eye[i][2],
                    (int)(180.0*angles[i].roll/M_PI),
                    (int)(180.0*angles[i].pitch/M_PI),
                    (int)(180.0*angles[i].yaw/M_PI)
                   );
        }
        fprintf(f,"//BSPMAPTYPE%04i\n",mapType);
    }

    // now write the real info...
    // Write the "world" first, then the rest of the entities...
    Entity *cur;
    if (!world)
        MessageBox(0,"World mismatch in map save!","BSP Save Map",MB_OK);

    if (world)
        world->writeToFILE(f,writeWorldKeys,selOnly,this);

    for (cur = objects.p_next; cur != &objects; cur = cur->p_next)
    {
        if (cur == world)
            continue;

        cur->writeToFILE(f,writeWorldKeys,selOnly,this);
    }
    fclose (f);

    dirty = 0;
}

void map::groupExportWrite(FILE *f, Entity *e, int func)
{
    epair_t	*ep;
    SetBrush *b;
    int testGroup;

    fprintf (f,"{\n");
    e->setOriginKey();
    // if this is not the world, then write them, if it is the world, only write if
    // writeworldkeys is set
    for (ep = e->epairs ; ep ; ep = ep->next)
        fprintf (f,"\"%s\"\t\"%s\"\n", ep->key, ep->value);

    // fixed size entities don't save out brushes
    if (e->modifiable)
    {
        for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
        {
            if (b->IsInvalid())
                continue;

            if (set.region_mode && b->IsRegioned())
                continue;

            testGroup = b->group;
            if (((func == -1) && groups->GetVisible(testGroup)) ||
                    (testGroup == func))
                b->writeToFILE(f,false);
        };
    }
    fprintf (f,"}\n");
}

void map::groupExport(int func)
{
    Entity *e;
    SetBrush *b;
    FILE	*f;
    int visFlag;
    int testGroup;
    int includes_ips;

    char filename[256] = "";

    OPENFILENAME ofn;
    memset(&ofn,0,sizeof(OPENFILENAME));
    ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
    ofn.hwndOwner = frame->hwnd;
    ofn.lpstrFilter = "Map Files (*.map)\0*.map\0";
    ofn.lpstrDefExt = "map";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = 256;
    ofn.lpstrInitialDir = set.map_directory;
    ofn.lpstrTitle = "Export to Map File";
    ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

    if (!GetSaveFileName(&ofn))
        return;

    f = fopen (filename,"w");
    if (!f)
    {
        syserror (const_cast<char *> ("Couldn't open %s for writing"), filename);
        Msgbox (const_cast<char *> ("Couldn't open %s for writing"), filename);
        return;
    }

    includes_ips = 0;

    if (world)
    {
        visFlag = 0;
        for (b = world->objects.p_next; !visFlag && (b != &world->objects); b = b->p_next)
        {
            testGroup = b->group;

            if (func == -1)
            {
                if (groups->GetVisible(testGroup))
                    visFlag = 1;
                // see if testGroup is visible...
            }
            else
            {
                // see if testGroup is current group?
                if (testGroup == func)
                    visFlag = 1;
            }
        }
    }

    if (world && visFlag)
        groupExportWrite(f,world,func);

    for (e = objects.p_next; e != &objects; e = e->p_next)
    {
        if (e == world)
            continue;

        visFlag = 0;
        for (b = e->objects.p_next; !visFlag && (b != &e->objects); b = b->p_next)
        {
            testGroup = b->group;

            if (func == -1)
            {
                if (groups->GetVisible(testGroup))
                    visFlag = 1;
                // see if testGroup is visible...
            }
            else
            {
                // see if testGroup is current group?
                if (testGroup == func)
                    visFlag = 1;
            }
        }

        if ((e != world) && !visFlag) // nothing visible... and not in world entity, go to next entity...
            continue;

        // check if it is info_player_start?
        if (!strcmp(e->classname,"info_player_start"))
            includes_ips = 1;

        // first, go through all of the brushes in object...
        // if anything in the object is visible, then...
        groupExportWrite(f,e,func);
    }

    if (!includes_ips)
    {
        // find it and write it to file...
        // put a player start in at 0,0,0;
        e = findPlayerStart();
        if (e)
            e->writeToFILE(f,false,false,this);
    }
    fclose (f);
}

void map::writeBox(FILE *f, char *walls, char *sky, char *floor)
{
    SetBrush *b;
    vec3_t cmins[6], cmaxs[6];
    int wallthickness = set.gridsize;
    texturedef_t td[3];
    for (int i = 0; i < 3; i++)
    {
        memset(&td[i],0,sizeof(texturedef_t));
        td[i].scale[0] = 1.0f;
        td[i].scale[1] = 1.0f;
    };

    int c = 0;
    // do all six of them...
    // left...
    if (set.game_mode == 2)
    {
        char basepath[64];
        char wall_texture[64];
        char sky_texture[64];
        char floor_texture[64];

        SplitBaseQuiet(walls,basepath,wall_texture);
        STRNCPY(td[0].texture,wall_texture);
        STRNCPY(td[0].basepath,basepath);

        SplitBaseQuiet(sky,basepath,sky_texture);
        STRNCPY(td[1].texture,sky_texture);
        STRNCPY(td[1].basepath,basepath);

        SplitBaseQuiet(floor,basepath,floor_texture);
        STRNCPY(td[2].texture,floor_texture);
        STRNCPY(td[2].basepath,basepath);
    }
    else
    {
        STRNCPY(td[0].texture,walls);
        STRNCPY(td[1].texture,sky);
        STRNCPY(td[2].texture,floor);
    }

    cmins[0][0] = sb_mins[0]-wallthickness;
    cmins[0][1] = sb_mins[1];
    cmins[0][2] = sb_mins[2];
    cmaxs[0][0] = sb_mins[0];
    cmaxs[0][1] = sb_maxs[1];
    cmaxs[0][2] = sb_maxs[2];
    b = new SetBrush();
    b->group = 0;
    b->setMins(cmins[c],cmaxs[c]);
    b->setTexturedef(&td[0]);
    b->writeToFILE(f,false);
    delete b;
    c++;
    // right
    cmins[1][0] = sb_maxs[0];
    cmins[1][1] = sb_mins[1];
    cmins[1][2] = sb_mins[2];
    cmaxs[1][0] = sb_maxs[0]+wallthickness;
    cmaxs[1][1] = sb_maxs[1];
    cmaxs[1][2] = sb_maxs[2];
    b = new SetBrush();
    b->group = 0;
    b->setMins(cmins[c],cmaxs[c]);
    b->setTexturedef(&td[0]);
    b->writeToFILE(f,false);
    delete b;
    c++;

    // front
    cmins[2][0] = sb_mins[0];
    cmins[2][1] = sb_mins[1]-wallthickness;
    cmins[2][2] = sb_mins[2];
    cmaxs[2][0] = sb_maxs[0];
    cmaxs[2][1] = sb_mins[1];
    cmaxs[2][2] = sb_maxs[2];
    b = new SetBrush();
    b->group = 0;
    b->setMins(cmins[c],cmaxs[c]);
    b->setTexturedef(&td[0]);
    b->writeToFILE(f,false);
    delete b;
    c++;

    // back;
    cmins[3][0] = sb_mins[0];
    cmins[3][1] = sb_maxs[1];
    cmins[3][2] = sb_mins[2];
    cmaxs[3][0] = sb_maxs[0];
    cmaxs[3][1] = sb_maxs[1]+wallthickness;
    cmaxs[3][2] = sb_maxs[2];
    b = new SetBrush();
    b->group = 0;
    b->setMins(cmins[c],cmaxs[c]);
    b->setTexturedef(&td[0]);
    b->writeToFILE(f,false);
    delete b;
    c++;

    // top
    cmins[4][0] = sb_mins[0];
    cmins[4][1] = sb_mins[1];
    cmins[4][2] = sb_maxs[2];
    cmaxs[4][0] = sb_maxs[0];
    cmaxs[4][1] = sb_maxs[1];
    cmaxs[4][2] = sb_maxs[2]+wallthickness;
    b = new SetBrush();
    b->group = 0;
    b->setMins(cmins[c],cmaxs[c]);
    b->setTexturedef(&td[1]);
    b->writeToFILE(f,false);
    delete b;
    c++;

    // bottom
    cmins[5][0] = sb_mins[0];
    cmins[5][1] = sb_mins[1];
    cmins[5][2] = sb_mins[2]-wallthickness;
    cmaxs[5][0] = sb_maxs[0];
    cmaxs[5][1] = sb_maxs[1];
    cmaxs[5][2] = sb_mins[2];
    b = new SetBrush();
    b->group = 0;
    b->setMins(cmins[c],cmaxs[c]);
    b->setTexturedef(&td[2]);
    b->writeToFILE(f,false);
    delete b;
}


int map::getVisibility(Entity *e, int inBox, int mode)
{
    SetBrush *b;

    if (inBox && (e == world))
        return 1;

    for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
    {
        if (mode == MODE_SELECTED)
        {
            if (!b->IsDrawable() || !b->IsSelected())
                continue;
        }
        else if (mode == MODE_REGIONED)
        {
            if (b->IsRegioned())
                continue;
        }
        return 1;
    }
    return 0;
}

void map::writeSelected(FILE *f, Entity *e)
{
    SetBrush *b;
    for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
    {
        if (!b->IsDrawable() || !b->IsSelected())
            continue;
        b->writeToFILE(f,false);
    }
}

Entity *map::findPlayerStart()
{
    Entity *e;
    for (e = objects.p_next; e != &objects; e = e->p_next)
    {
        // if not already set, then skip entity...
        if (strcmp(e->classname,"info_player_start"))
            continue;

        return e;
    }
    return (Entity *)NULL;
}

Entity *map::findWorldSpawn()
{
    Entity *e;
    for (e = objects.p_next; e != &objects; e = e->p_next)
    {
        if (strcmp(e->classname,"worldspawn"))
            continue;
        return e;
    }
    return (Entity *)NULL;
}

// Export selected brushes
void map::selectedExport()
{
    Entity *e;
    FILE	*f;
    epair_t *ep;
    int   i;
    int visFlag;
    int includes_ips;

    char goodname[128];
    char runname[128];

    int inBox;

    if (!numSelected())
    {
        ::MessageBox(0,"Must have at least one brush selected...","BSP Export Selected", MB_OK | MB_ICONEXCLAMATION);
        return;
    }
    char sky[LUMP_NAME_LENGTH_MAX];
    char walls[LUMP_NAME_LENGTH_MAX];
    char floor[LUMP_NAME_LENGTH_MAX];

    client->exportxfer.inBox = BST_CHECKED;
    client->exportxfer.sky.Clear();
    client->exportxfer.walls.Clear();
    client->exportxfer.floor.Clear();
    client->exportxfer.exporter.Clear();
    texInfo_t *tx;

    char tname[256];
    for (i = 0; i< numMapTextures; i++)
    {
        tx = mapTextures[i];
        if (tx)
        {
            int sel = !strnicmp(tx->bmp,"sky",3);

            if (set.game_mode == 2)
                sprintf(tname,"%s/%s",tx->basepath,tx->bmp);
            else
                strncpy(tname,tx->bmp,sizeof(tname));

            client->exportxfer.sky.Add(tname, sel);
            client->exportxfer.walls.Add(tname);
            client->exportxfer.floor.Add(tname);
        }
    }

    //
    struct _finddata_t ffb;
    memset(&ffb,0,sizeof(ffb));
    char dirname[256];
    sprintf(dirname,"%s\\*.bat",set.bat_directory);
    int ff = _findfirst(dirname,&ffb);

    int nomorefiles = ff < 0 ? 1 : 0;
    while (!nomorefiles)
    {
        if (ffb.attrib & _A_SUBDIR)
        {
            nomorefiles = _findnext(ff, &ffb);
            continue;
        }

        strncpy(runname,ffb.name,sizeof(runname));
        runname[sizeof(runname)-1] = 0;
        strlwr(runname);
        toupper((unsigned char)runname[0]);
        client->exportxfer.exporter.Add(runname);
        nomorefiles = _findnext(ff, &ffb);
    }
    _findclose(ff);

    client->exportxfer.exporter.Add("none");
    strcpy(runname,"none");

    TExportDlg dialog(0,&client->exportxfer);
    INT_PTR retval = dialog.Execute();

    STRNCPY(sky,client->exportxfer.sel_sky);
    STRNCPY(walls,client->exportxfer.sel_walls);
    STRNCPY(floor,client->exportxfer.sel_floor);
    inBox = client->exportxfer.inBox;
    STRNCPY(runname,client->exportxfer.sel_exporter);


    if (retval != IDOK)
        return;

    if (!walls || !walls[0])
        strcpy(walls,"?");
    if (!sky || !sky[0])
        strcpy(sky,"?");
    if (!floor || !floor[0])
        strcpy(floor,"?");


    char filename[256] = "";

    OPENFILENAME ofn;
    memset(&ofn,0,sizeof(OPENFILENAME));
    ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
    ofn.hwndOwner = frame->hwnd;
    ofn.lpstrFilter = "Map Files (*.map)\0*.map\0";
    ofn.lpstrDefExt = "map";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = 256;
    ofn.lpstrInitialDir = set.map_directory;
    ofn.lpstrTitle = "Export Selected to Map File";
    ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

    if (!GetSaveFileName(&ofn))
        return;

    f = fopen (filename,"w");
    if (!f)
    {
        ::MessageBox(0,"Couldn't write map file...","BSP Export Selected", MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    // calculate bbox...
    sb_mins[0] = sb_mins[1] = sb_mins[2] = MAX_NUM;
    sb_maxs[0] = sb_maxs[1] = sb_maxs[2] = -MAX_NUM;

    makeSelectedPerform(SEL_ADDTOBBOX);

    // expand it a bit...
    for (i = 0; i < 3; i++)
    {
        sb_mins[i] -= set.gridsize;
        sb_maxs[i] += set.gridsize;
    }

    includes_ips = 0;

    e = world;
    visFlag = getVisibility(e, inBox, MODE_SELECTED);

    // nothing visible... and not in world entity, go to next entity...
    if (visFlag)
    {
        // first, go through alll of the brushes in object...
        // if anything in the object is visible, then...
        fprintf (f,"{\n");
        // if this is the world, then write them, if it is the world, only write if
        // writeworldkeys is set
        for (ep=e->epairs ; ep ; ep=ep->next)
            fprintf (f,"\"%s\"\t\"%s\"\n", ep->key, ep->value);

        // fixed size entities don't save out brushes
        if (inBox)   // at world
            // write out the walls, sky, floor...
            writeBox(f,sky,walls,floor);

        writeSelected(f,e);
        fprintf (f,"}\n");
    }

    for (e = objects.p_next; e != &objects; e = e->p_next)
    {
        if (e == world)
            continue;

        visFlag = getVisibility(e, inBox, MODE_SELECTED);

        if (!visFlag) // nothing visible... and not in world entity, go to next entity...
            continue;

        // check if it is info_player_start?
        if (!strcmp (e->classname,"info_player_start"))
            includes_ips = 1;

        // first, go through alll of the brushes in object...
        // if anything in the object is visible, then...

        fprintf (f,"{\n");
        e->setOriginKey();

        // if this is not the world, then write them, if it is the world, only write if
        // writeworldkeys is set
        for (ep=e->epairs ; ep ; ep=ep->next)
            fprintf (f,"\"%s\"\t\"%s\"\n", ep->key, ep->value);

        // fixed size entities don't save out brushes
        if (e->modifiable)
            writeSelected(f,e);

        fprintf (f,"}\n");
    }

    if (!includes_ips)
    {
        // find it and write it to file...
        // put a player start in at 0,0,0;
        e = findPlayerStart();
        if (e)
            e->writeToFILE(f,false,false,this);
    }
    fclose (f);

    ExtractFileBase(filename,goodname,true);
    DoExport(goodname, runname, const_cast<char *> ("BSP - Export Selected"));
}

// REGION exporting:
void map::regionExport()
{
    Entity *e;
    SetBrush *b;
    FILE	*f;
    epair_t	*ep;
    int   i;
    int visFlag;
    int includes_ips;

    char goodname[128];
    char runname[128];

    char sky[LUMP_NAME_LENGTH_MAX];
    char walls[LUMP_NAME_LENGTH_MAX];
    char floor[LUMP_NAME_LENGTH_MAX];
    int inBox;

    if (!set.region_mode)
    {
        ::MessageBox(0,"Must be in region mode...","BSP Export Region", MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    client->exportxfer.inBox = BST_CHECKED;
    client->exportxfer.sky.Clear();
    client->exportxfer.walls.Clear();
    client->exportxfer.floor.Clear();
    client->exportxfer.exporter.Clear();

    texInfo_t *tx;
    char tname[256];
    for (i = 0; i< numMapTextures; i++)
    {
        tx = mapTextures[i];
        if (tx)
        {
            int sel = !strnicmp(tx->bmp,"sky",3);

            if (set.game_mode == 2)
                sprintf(tname,"%s/%s",tx->basepath,tx->bmp);
            else
                strncpy(tname,tx->bmp,sizeof(tname));

            client->exportxfer.sky.Add(tname, sel);
            client->exportxfer.walls.Add(tname);
            client->exportxfer.floor.Add(tname);
        }
    }

    //
    struct _finddata_t ffb;
    memset(&ffb,0,sizeof(ffb));
    char dirname[256];
    sprintf(dirname,"%s\\*.bat",set.bat_directory);
    int ff = _findfirst(dirname,&ffb);

    int nomorefiles = 0;
    while (!nomorefiles)
    {
        if (ffb.attrib & _A_SUBDIR)
        {
            nomorefiles = _findnext(ff, &ffb);
            continue;
        }

        strncpy(runname,ffb.name,sizeof(runname));
        runname[sizeof(runname)-1] = 0;
        strlwr(runname);
        toupper(runname[0]);
        client->exportxfer.exporter.Add(runname);
        nomorefiles = _findnext(ff, &ffb);
    }
    _findclose(ff);

    client->exportxfer.exporter.Add("none");

    strcpy(runname,"none");

    TExportDlg dialog(0,&client->exportxfer);
    INT_PTR retval = dialog.Execute();

    STRNCPY(sky,client->exportxfer.sel_sky);
    STRNCPY(walls,client->exportxfer.sel_walls);
    STRNCPY(floor,client->exportxfer.sel_floor);
    inBox = client->exportxfer.inBox;
    STRNCPY(runname,client->exportxfer.sel_exporter);


    if (retval != IDOK)
        return;

    if (!walls || !*walls)
        strcpy(walls,"?");
    if (!sky || !*sky)
        strcpy(sky,"?");
    if (!floor || !*floor)
        strcpy(floor,"?");
    char filename[256] = "";

    OPENFILENAME ofn;
    memset(&ofn,0,sizeof(OPENFILENAME));
    ofn.lStructSize = OPENFILENAMESTRUCTSIZE;
    ofn.hwndOwner = frame->hwnd;
    ofn.lpstrFilter = "Map Files (*.map)\0*.map\0";
    ofn.lpstrDefExt = "map";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = 256;
    ofn.lpstrInitialDir = set.map_directory;
    ofn.lpstrTitle = "Export Region to Map File";
    ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

    if (!GetSaveFileName(&ofn))
        return;

    f = fopen (filename,"w");
    if (!f)
    {
        ::MessageBox(0,"Couldn't write map file...","BSP Export Region", MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    // calculate bbox...
    sb_mins[0] = sb_mins[1] = sb_mins[2] = MAX_NUM;
    sb_maxs[0] = sb_maxs[1] = sb_maxs[2] = -MAX_NUM;

    for (e = objects.p_next; e != &objects; e = e->p_next)
    {
        for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
        {
            if (set.group_mode && !groups->GetVisible(b->group))
                continue;

            if (set.region_mode && b->IsRegioned())
                continue;

            b->addToBBox();	//TODO
        }
    }

    // expand it a bit...
    sb_mins[0] -= set.gridsize;
    sb_mins[1] -= set.gridsize;
    sb_mins[2] -= set.gridsize;

    sb_maxs[0] += set.gridsize;
    sb_maxs[1] += set.gridsize;
    sb_maxs[2] += set.gridsize;

    includes_ips = 0;
    e = world;

    visFlag = getVisibility(e, inBox, MODE_REGIONED);

    if (visFlag)   // nothing visible... and not in world entity, go to next entity...
    {
        // first, go through alll of the brushes in object...
        // if anything in the object is visible, then...
        fprintf (f,"{\n");
        e->setOriginKey();
        // if this is not the world, then write them, if it is the world, only write if
        // writeworldkeys is set
        for (ep=e->epairs ; ep ; ep=ep->next)
            fprintf (f,"\"%s\"\t\"%s\"\n", ep->key, ep->value);

        // fixed size entities don't save out brushes
        if (inBox)
            writeBox(f, walls, sky, floor);

        for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
        {
            if (b->IsRegioned())
                continue;
            b->writeToFILE(f,false);
        }
        fprintf (f,"}\n");
    };

    for (e = objects.p_next; e != &objects; e = e->p_next)
    {
        if (e == world)
            continue;

        visFlag = getVisibility(e, inBox, MODE_REGIONED);

        if (!visFlag) // nothing visible... and not in world entity, go to next entity...
            continue;

        // check if it is info_player_start?
        if (!strcmp (e->classname,"info_player_start"))
            includes_ips = 1;

        // first, go through alll of the brushes in object...
        // if anything in the object is visible, then...
        fprintf (f,"{\n");
        e->setOriginKey();
        // if this is not the world, then write them, if it is the world, only write if
        // writeworldkeys is set
        for (ep=e->epairs ; ep ; ep=ep->next)
            fprintf (f,"\"%s\"\t\"%s\"\n", ep->key, ep->value);

        // fixed size entities don't save out brushes
        if (e->modifiable)
        {
            for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
            {
                if (b->IsRegioned())
                    continue;
                b->writeToFILE(f,false);
            };
        }
        fprintf (f,"}\n");
    };

    if (!includes_ips)
    {
        // find it and write it to file...
        // put a player start in at 0,0,0;
        e = findPlayerStart();
        if (e)
            e->writeToFILE(f,false,false,this);
    }
    fclose (f);

    ExtractFileBase(filename,goodname,true);

    DoExport(goodname, runname, const_cast<char *> ("BSP - Export Region"));
}

void map::DoExport(char *goodname, char *runname, char *title)
{
    if (!strcmpi(runname,"none"))
    {
        set.redrawxy = 0;
        set.redrawedit = 0;
        Show_Frame(Str(const_cast<char *> ("Exported information saved to [%s]..."), goodname), true);
        return;
    }

    Path progname(const_cast<char *> ("%s/%s"), set.bat_directory,runname);
    if (!progname.Exists())
    {
        Str str(const_cast<char *> ("Could not find exporter [%s]!\nCheck .bat directory setting..."), progname);
        MessageBox(0,str,title, MB_OK | MB_ICONEXCLAMATION);
        return;
    }
    Bsp_ShellExecute(0, NULL, progname,	goodname, set.map_directory);

    set.redrawxy = 0;
    set.redrawedit = 0;
    Show_Frame(Str(const_cast<char *> ("Spawning [%s] on export file [%s]..."), runname, goodname), true);
}
/*
===================
entityConnect
A command-shift-click on an entity while an entity is selected will
make a target connection from the original entity.
===================
*/
bool map::entityConnect(vec3_t p1, vec3_t p2)
{
    Entity *oldent, *ent;

    oldent = current;
    if (oldent == world)
    {
        //	qprintf ("Must have a non-world entity selected to connect");
        return false;
    }

    selectRay(p1,p2,true);
    ent = current;
    if (ent == oldent)
    {
        //	qprintf ("Must click on a different entity to connect");
        return false;
    }

    if (ent == world)
    {
        //	qprintf ("Must click on a non-world entity to connect");
        return false;
    }
    oldent->setKey(const_cast<char *> ("target"), ent->get_targetname());
    dirty = 1;
    return true;
}


/*
=================
selectRay
If ef is true, any entity brush along the ray will be selected in preference
to intervening world brushes
=================
*/
void map::selectRay(vec3_t p1, vec3_t p2, bool ef)
{
    Entity *ent, *bestent;
    SetBrush *brush, *bestbrush;
    face_t *face, *bestface;
    float	time, besttime;
    texturedef_t	*td;

    ClearHits();

    bestent = NULL;
    bestface = NULL;
    bestbrush = NULL;
    besttime = MAX_NUM;

    // if we will favor ents, then check them first.
    //if (ef) {
    for (ent = objects.p_next; ent && (ent != &objects); ent = ent->p_next)
    {
        if (ent == world)
            continue;

        LoopProblem("SELRAY");
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            LoopProblem("SELRAYB");
            if (!brush->IsDrawable())
                continue;

            brush->hitByRay(p1,p2,&time,&face);
            if (time < 0)
                continue;

            AddHitBrush(brush);

            // in case it turns into a "selection drag"
            brush->setSaveSelected(brush->IsSelected());

            if (time >besttime)
                continue;
            bestent = ent;
            besttime = time;
            bestbrush = brush;
            bestface = face;
        }
    }
    //};

    ent = world;
    // if we favored ents and didn't hit anything OR we didn't check the ents
    // then check just the world...
    if (!ef || !bestbrush)
    {
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            LoopProblem("SELRAYB");
            if (!brush->IsDrawable())
                continue;

            brush->hitByRay(p1,p2,&time,&face);
            if (time < 0)
                continue;

            AddHitBrush(brush);

            // in case it turns into a "selection drag"
            brush->setSaveSelected(brush->IsSelected());
            if (time >besttime)
                continue;

            bestent = ent;
            besttime = time;
            bestbrush = brush;
            bestface = face;
        }
    };

    // Nothing was hit!
    if (besttime == MAX_NUM)
        return;

    dirty = 1;

    // set the best face
    bestbrush->currentFace = bestface;
    if (bestent != current)
    {
        if (!set.multi_select)
            makeGlobalPerform(SEL_DESELECT);
        setCurrentEntity(bestent);
    }

    if (!bestbrush->IsSelected())
    {
        if (numSelected() == 0)  	// don't grab texture if others are selected
        {
            td = bestbrush->texturedefForFace(bestface);
            texWindow->setTextureDef(td);
        };
        bestbrush->setSelected(true);
    }
    else
    {
        bestbrush->setSelected(false);
    }
}

/*
=================
selectKnob
If ef is true, any entity brush along the ray will be selected in preference
to intervening world brushes
=================
*/
void map::selectKnob(vec3_t p1, vec3_t p2, int UAxis, int VAxis, bool ef)
{
    Entity *ent, *bestent;
    SetBrush *brush, *bestbrush;
    face_t *face, *bestface;
    float	time, besttime;
    float d1, d2;
    texturedef_t	*td;
    vec3_t mins, maxs, ctr;

    ClearHits();

    bestent = NULL;
    bestface = NULL;
    bestbrush = NULL;
    besttime = MAX_NUM;      // this will be distance...

    // first check entities if they are favored
//   if (ef) {
    for (ent = objects.p_next; ent && (ent != &objects); ent = ent->p_next)
    {
        if (ent == world)
            continue;
        LoopProblem("SELK");
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            LoopProblem("SELKB");
            if (!brush->IsDrawable())
                continue;

            brush->getMins(mins,maxs);

            for (int k = 0; k < 3; k++)
                ctr[k] = (mins[k] + maxs[k])/2.0f;

            brush->hitByRay(p1,p2,&time,&face);
            if (time < 0)
                continue;

            AddHitBrush(brush);

            // in case it turns into a "selection drag"
            brush->setSaveSelected(brush->IsSelected());

            // Now deal with distance to brush center...
            d1 = p1[UAxis] - ctr[UAxis];
            d2 = p1[VAxis] - ctr[VAxis];

            time = sqrt(d1*d1 + d2*d2);

            if (time >besttime)
                continue;

            bestent = ent;
            besttime = time;
            bestbrush = brush;
            bestface = face;
        }
    }
//	}

    ent = world;
    if (!ef || !bestbrush)
    {
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            LoopProblem("SELKB");
            if (!brush->IsDrawable())
                continue;

            brush->getMins(mins,maxs);

            for (int k = 0; k < 3; k++)
                ctr[k] = (mins[k] + maxs[k])/2.0f;

            brush->hitByRay(p1,p2,&time,&face);
            if (time < 0)
                continue;

            AddHitBrush(brush);

            // in case it turns into a "selection drag"
            brush->setSaveSelected(brush->IsSelected());

            // Now deal with distance to brush center...
            d1 = p1[UAxis] - ctr[UAxis];
            d2 = p1[VAxis] - ctr[VAxis];

            time = sqrt(d1*d1 + d2*d2);

            if (time > besttime)
                continue;

            bestent = ent;
            besttime = time;
            bestbrush = brush;
            bestface = face;
        }
    }

    if (besttime == MAX_NUM)
        return;

    dirty = 1;
    bestbrush->currentFace = bestface;

    if (bestent != current)
    {
        if (!set.multi_select)
            makeGlobalPerform(SEL_DESELECT);
        setCurrentEntity(bestent);
    }

    if (!bestbrush->IsSelected())
    {
        if (numSelected() == 0)
        {
            // don't grab texture if others are selected
            td = bestbrush->texturedefForFace(bestface);
            texWindow->setTextureDef(td);
        }
        bestbrush->setSelected(true);
        //		qprintf ("selected entity %i brush %i face %i", [self indexOf:bestent], [bestent indexOf: bestbrush], bestface);
    }
    else
    {
        bestbrush->setSelected(false);
        //		qprintf ("deselected entity %i brush %i face %i", [self indexOf:bestent], [bestent indexOf: bestbrush], bestface);
    }
}

/*
=================
getMouseObject
Fills in object structure for current position.
=================
*/
void map::getMouseObject(mouseobject_t *obj, vec3_t p1, vec3_t p2, vec3_t dragpoint, int ViewType, int UAxis, int VAxis)
{
    Entity *ent, *bestent;
    SetBrush *brush, *bestbrush;
    face_t *face;
    float	time, besttime;
    float knobd;
    float d1, d2;
    vec3_t mins, maxs, ctr;

    memset(obj,0,sizeof(mouseobject_t));

    bestent = NULL;
    bestbrush = NULL;
    besttime = MAX_NUM;      // this will be distance...

    for (ent = objects.p_next; ent && (ent != &objects); ent = ent->p_next)
    {
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            if (!brush->IsDrawable())
                continue;

            brush->getMins(mins,maxs);

            ctr[0] = (mins[0] + maxs[0])/2.0f;
            ctr[1] = (mins[1] + maxs[1])/2.0f;
            ctr[2] = (mins[2] + maxs[2])/2.0f;

            brush->hitByRay(p1,p2,&time,&face);
            if (time < 0.0f)
                continue;

            // Now deal with distance to brush center...
            d1 = p1[UAxis] - ctr[UAxis];
            d2 = p1[VAxis] - ctr[VAxis];

            knobd = sqrt(d1*d1 + d2*d2);
            if (set.center_knobs_xy)
            {
                time = knobd;
                if (time < 4.0f)
                    obj->CenterKnob = true;
            }

            if (time > besttime)
                continue;

            bestent = ent;
            besttime = time;
            bestbrush = brush;
        }
    }

    if (besttime == MAX_NUM)
    {
        obj->Nothing = true;
        return;
    }

    //clippers?
    if (clipper_i && clipper_i->num)
    {
        float delta;
        float d;
        float pixelWidth;
        float clipRadius = 10.0f;

        pixelWidth = 1.0f/set.scale;  // each pixel is this many world units...
        delta = (clipRadius/2.0f) * pixelWidth; // how many units to allow...
        for (int i=0 ; i<clipper_i->num ; i++)
        {

            d = ((dragpoint[0] - clipper_i->pos[i][UAxis]) * (dragpoint[0] - clipper_i->pos[i][UAxis]) +
                 (dragpoint[1] - clipper_i->pos[i][VAxis]) * (dragpoint[1] - clipper_i->pos[i][VAxis]));

            d = sqrt(d);
            if (d > delta)
                continue;

            obj->Clipper = true;
            obj->Entity = bestent;
            obj->Brush = bestbrush;
            obj->Selected = bestbrush->IsSelected();
            return;
        }
    }

    // Camera?
    if (set.show_cameras)
    {
        float *v;

        float delta;
        float d;
        float pixelWidth;
        pixelWidth = 1.0f/set.scale;  // each pixel is this many world units...
        delta = ((float)set.camera_size/2.0f) * pixelWidth; // how many units to allow...

        map *m = map_i[set.curmap];
        for (int i = 0; i < MAX_CAMERAS; i++)
        {
            v = m->eye[i];

            d = (dragpoint[0] - v[UAxis]) * (dragpoint[0] - v[UAxis]) +
                (dragpoint[1] - v[VAxis]) * (dragpoint[1] - v[VAxis]);
            d = sqrt(d);

            if (d > delta)
                continue;

            obj->Camera = true;
            obj->Entity = bestent;
            obj->Brush = bestbrush;
            obj->Selected = bestbrush->IsSelected();
            return;
        }
    }

    //vertex
    for (ent = objects.p_next; ent && (ent != &objects); ent = ent->p_next)
    {
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            if (!brush->IsDrawable())
                continue;

            if (brush->vertexClicked(dragpoint,ViewType))
            {
                obj->Vertex = true;
                obj->Entity = bestent;
                obj->Brush = bestbrush;
                obj->Selected = bestbrush->IsSelected();
                return;
            }
        }
    }

    obj->Nothing = true;
}

/*
=================
grabRay

only checks the selected brushes
Returns the brush hit, or nil if missed.
=================
*/
SetBrush *map::grabRay(vec3_t p1, vec3_t p2)
{
    Entity *ent;
    SetBrush *brush, *bestbrush;
    face_t *face;
    float	time, besttime;

    // "time" seems to be the depth of a click, ie the "z" of clicking through "xy".

    bestbrush = NULL;
    besttime = TMAX*2;	//this value seems to be the total depth of the entire world, so -TMAX to TMAX would be TMAX*2

    for (ent = objects.p_next; ent && ent != &objects; ent = ent->p_next)
    {
        LoopProblem("GRABE");
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            LoopProblem("GRABB");

            if (!brush->IsDrawable())
                continue;
            if (!brush->IsSelected())
                continue;

            brush->hitByRay(p1,p2,&time,&face);
            if (time < 0 || time > besttime)
                continue;		//select face on current brush

            besttime = time;
            bestbrush = brush;	//selects a new brush
        }
    }

    if (besttime == MAX_NUM)
        return NULL;

    dirty = 1;

    return bestbrush;
}

/*
=================
getTextureRay
=================
*/
SetBrush *map::getTextureRay(vec3_t p1, vec3_t p2)
{
    Entity *ent, *bestent;
    SetBrush *brush, *bestbrush;
    face_t *face, *bestface;
    float	time, besttime;
    texturedef_t	*td;
    vec3_t	mins, maxs;

    bestbrush = NULL;
    bestent = NULL;
    besttime = MAX_NUM;
    bestface = NULL;

    for (ent = objects.p_next; ent && (ent != &objects); ent = ent->p_next)
    {
        LoopProblem("GETTEXRE");
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            LoopProblem("GETTEXRB");
            if (!brush->IsDrawable())
                continue;

            brush->hitByRay(p1,p2,&time,&face);
            if (time < 0 || time >besttime)
                continue;
            bestent = ent;
            bestface = face;
            besttime = time;
            bestbrush = brush;
        }
    }

    if (besttime == MAX_NUM)
        return NULL;

    if (!bestent->modifiable)
    {
        //	qprintf ("can't modify spawned entities");
        return NULL;
    }

    dirty = 1;

    td = bestbrush->texturedefForFace(bestface);
    texWindow->setTextureDef(td);

    bestbrush->getMins(mins,maxs);
    setCurrentMinZ(mins);
    setCurrentMaxZ(maxs);
    return bestbrush;
}

/*
=================
setTextureRay
=================
*/
void map::setTextureRay(vec3_t p1, vec3_t p2, bool allsides)
{
    Entity *ent, *bestent;
    SetBrush *brush, *bestbrush;
    face_t *face, *bestface;
    float	time, besttime;
    texturedef_t	td;

    bestent = NULL;
    bestface = NULL;
    bestbrush = NULL;
    besttime = MAX_NUM;

    for (ent = objects.p_next; ent && (ent != &objects); ent = ent->p_next)
    {
        LoopProblem("STRE");
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            LoopProblem("STRB");
            if (!brush->IsDrawable())
                continue;

            brush->hitByRay(p1,p2,&time,&face);
            if (time < 0 || time >besttime)
                continue;
            bestent = ent;
            besttime = time;
            bestbrush = brush;
            bestface = face;
        }
    }

    if (besttime == MAX_NUM)
    {
        //	qprintf ("trace missed");
        return;
    }

    if (!bestent->modifiable)
    {
        //	qprintf ("can't modify spawned entities");
        return;
    }

    if (bestbrush->IsRegioned())
    {
        // qprintf ("WANRING: clicked on regioned brush");
        return;
    }

    dirty = 1;

    texWindow->getTextureDef(&td);

    if (allsides)
    {
        bestbrush->setTexturedef(&td);
    }
    else
    {
        bestbrush->setTexturedef(&td,bestface);
    }
}

void map::makeAllPerform(int sel)
{
    Entity *ent;
    SetBrush *brush;

    dirty = 1;

    for (ent = objects.p_next; ent != &objects; ent = ent->p_next)
    {
        LoopProblem("MAPE");
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            LoopProblem("MAPB");
            if (!brush->IsDrawable())
                continue;

            switch(sel)
            {
            case SEL_FINDCLOSEST:
                brush->ClosestRenderSelf();
                break;
            case SEL_CALCWINDINGS:
                brush->calcWindings();
                break;
            case SEL_LOCK:
                if (ent->modifiable)
                    brush->setLocked(true);
                break;
            case SEL_UNLOCK:
                if (ent->modifiable)
                    brush->setLocked(false);
                break;
            case SEL_SETREGIONED:
                brush->setRegioned();
                break;
            case SEL_UNSETREGIONED:
                brush->setRegioned(false);
                break;
            case SEL_SETREGIONEDPARTIAL:
                brush->setRegionedPartial();
                break;
            case SEL_SETFILTERED:
                brush->setFiltered();
                break;
            case SEL_SELECT:
                brush->setSelected(true);
                break;
            case SEL_TRANSLATE:
                brush->translate();
                break;
            case SEL_FEETTOFLOOR:
                brush->feetToFloor();
                break;
            case SEL_DESELECT:
                brush->setSelected(false);
                break;
            case SEL_ADDTOBBOX:
                brush->addToBBox();
                break;
            case SEL_TRANSFORM:
                brush->transform();
                break;
            case SEL_FLIPNORMALS:
                brush->flipNormals();
                break;
            case SEL_MOVETOENTITY:
                brush->moveToEntity();
                break;
            case SEL_REMOVE:
                brush->setInvalid(true);
                cleanUpInvalidObjects();
                break;
            case SEL_SELECTCOMPLETE:
                brush->selectComplete();
                break;
            case SEL_SELECTPARTIAL:
                brush->selectPartial();
                break;
            case SEL_CARVEBYCLIPPER:
                brush->carveByClipper();
                cleanUpInvalidObjects();
                break;
            case SEL_SPLITBYCLIPPER:
                brush->splitByClipper();
                cleanUpInvalidObjects();
                break;
            case SEL_CAMERARENDERSELF:
                brush->CameraRenderSelf();
                brushCount++;
                break;
            default:
                syserror(const_cast<char *> ("MAPerform Unknown method..."));
            }
        }
    }
}

void map::makeGlobalPerform(int sel)	// in and out of region
{
    Entity *ent;
    SetBrush *brush;

    dirty = 1;

    for (ent = objects.p_next; ent != &objects; ent = ent->p_next)
    {
        LoopProblem("MGPE");
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            LoopProblem("MGPB");
            switch(sel)
            {
            case SEL_CALCWINDINGS:
                brush->calcWindings();
                break;
            case SEL_LOCK:
                if (ent->modifiable)
                    brush->setLocked(true);
                break;
            case SEL_UNLOCK:
                if (ent->modifiable)
                    brush->setLocked(false);
                break;
            case SEL_SETFILTERED:
                brush->setFiltered();
                break;
            case SEL_SETREGIONED:
                brush->setRegioned();
                break;
            case SEL_UNSETREGIONED:
                brush->setRegioned(false);
                break;
            case SEL_SETREGIONEDPARTIAL:
                brush->setRegionedPartial();
                break;
            case SEL_SELECT:
                brush->setSelected(true);
                break;
            case SEL_TRANSLATE:
                brush->translate();
                break;
            case SEL_FEETTOFLOOR:
                brush->feetToFloor();
                break;
            case SEL_DESELECT:
                brush->setSelected(false);
                break;
            case SEL_ADDTOBBOX:
                brush->addToBBox();
                break;
            case SEL_TRANSFORM:
                brush->transform();
                break;
            case SEL_FLIPNORMALS:
                brush->flipNormals();
                break;
            case SEL_MOVETOENTITY:
                brush->moveToEntity();
                break;
            case SEL_REMOVE:
                brush->setInvalid(true);
                cleanUpInvalidObjects();
                break;
            case SEL_SELECTCOMPLETE:
                brush->selectComplete();
                break;
            case SEL_SELECTPARTIAL:
                brush->selectPartial();
                break;
            case SEL_CARVEBYCLIPPER:
                brush->carveByClipper();
                cleanUpInvalidObjects();
                break;
            case SEL_SPLITBYCLIPPER:
                brush->splitByClipper();
                cleanUpInvalidObjects();
                break;
            default:
                syserror(const_cast<char *> ("MGPerform Unknown method..."));
            };
        }
    }
}

void map::sel_identity()
{
    sel_x[0]=1;
    sel_x[1]=0;
    sel_x[2]=0;
    sel_y[0]=0;
    sel_y[1]=1;
    sel_y[2]=0;
    sel_z[0]=0;
    sel_z[1]=0;
    sel_z[2]=1;
}

void map::transformSelection()
{
    if (!current->modifiable)
    {
        //	qprintf ("can't modify spawned entities");
        return;
    }

    dirty = 1;

    // apply transformation to origin of selection
    getSelectedCenter();
    sb_ctr[0] = (float) snapToGrid(sb_ctr[0]);
    sb_ctr[1] = (float) snapToGrid(sb_ctr[1]);
    sb_ctr[2] = (float) snapToGrid(sb_ctr[2]);

    // do it!
    makeSelectedPerform(SEL_TRANSFORM);
}

void map::swapvectors(vec3_t a, vec3_t b)
{
    vec3_t	temp;

    VectorCopy (a, temp);
    VectorCopy (b, a);
    VectorSubtract (vec3_origin, temp, b);
}

void map::rotate_x()
{
    sel_identity();
    swapvectors(sel_y, sel_z);
    transformSelection();
    dirty = 1;
}

void map::rotate_y()
{
    sel_identity();
    swapvectors(sel_x, sel_z);
    transformSelection();
    dirty = 1;
}

void map::rotate_z()
{
    sel_identity();
    swapvectors(sel_x, sel_y);
    transformSelection();
    dirty = 1;
}

void map::flip_x()
{
    sel_identity();
    sel_x[0] = -1;
    transformSelection();
    makeSelectedPerform(SEL_FLIPNORMALS);
    dirty = 1;
}

void map::flip_y()
{
    sel_identity();
    sel_y[1] = -1;
    transformSelection();
    makeSelectedPerform(SEL_FLIPNORMALS);
    dirty = 1;
}

void map::flip_z()
{
    sel_identity();
    sel_z[2] = -1;
    transformSelection();
    makeSelectedPerform(SEL_FLIPNORMALS);
    dirty = 1;
}

bool map::lookAtSelection(bool force)
{

    if(!force)
    {
        if(!set.look_at_sel)
            return false;
        if(numSelected() <= 0)
            return false;
    }

    getSelectedCenter();

    float x = eye[cureye][0] - sb_ctr[0];
    float y = eye[cureye][1] - sb_ctr[1];
    angles[cureye].yaw = angle(x, y);
    angles[cureye].pitch = angle(eye[cureye][2] - sb_ctr[2], sqrt(x*x + y*y));
    return true;
}

void map::cloneSelection()
{
    Entity *o;
    SetBrush *b;
    SetBrush *newBrush;
    Entity *newEntity;
    int someInWorld;

    newEntity = NULL;
    dirty = 1;

    if(set.clone_delta_disable)
    {
        //do not translate clones!
        sb_translate[0] = 0;
        sb_translate[1] = 0;
    }
    else
    {
        if (!set.clone_delta_x)
        {
            sb_translate[0] = (float) set.gridsize;
        }
        else
        {
            sb_translate[0] = (float) set.clone_delta_x;
        }
        if (!set.clone_delta_y)
        {
            sb_translate[1] = (float) set.gridsize;
        }
        else
        {
            sb_translate[1] = (float) set.clone_delta_y;
        }
    }

    sb_translate[2] = 0;

    someInWorld = 0;
    // copy individual brushes in the world entity
    o = world;

    for (b = o->objects.p_next; b != &o->objects; b = b->p_next)
    {
        LoopProblem("CLONEB");
        if (!b->IsDrawable())
            continue;

        if (!b->IsSelected())
            continue;

        // copy the brush, then translate the original
        newBrush = b->copy();
        newBrush->setSelected(true);
        newBrush->translate();
        b->setSelected(false);
        o->addObject(newBrush);
        someInWorld = 1;
    }

    int anySelected;
    // copy entire entities otherwise
    if (objects.p_next != &objects)
    {
        for (o = objects.p_next; o && (o != &objects); o = o->p_next)
        {
            LoopProblem("CLONEE");
            if (o == world)
                continue;
            anySelected = 0;
            SetBrush *brush;
            for (brush = o->objects.p_next; !anySelected && (brush != &o->objects); brush = brush->p_next)
            {
                if (!brush->IsDrawable())
                    continue;
                if (brush->IsSelected())
                    anySelected = 1;
            };

            if (!anySelected)
                continue;

            // if any, copy whole entity...
            newEntity = o->copy();
            addObject(newEntity);

            for (brush = o->objects.p_next; brush != &o->objects; brush = brush->p_next)
                brush->setSelected(false);

            for (brush = newEntity->objects.p_next; brush != &newEntity->objects; brush = brush->p_next)
            {
                brush->translate();
                brush->setSelected(true);
            }
        };
    }

    // set current entity to world...
    if (someInWorld || !newEntity)
    {
        setCurrentEntity(world);
    }
    else
    {
        setCurrentEntity(newEntity);   // last one, to force update of KP view...
    };
}

void map::selectCompleteEntity()
{
    Entity *o;
    SetBrush *b;

    dirty = 1;

    b = selectedBrush();
    if (!b)
        // qprintf ("nothing selected");
        return;

    o = b->parent;
    SetBrush *brush;
    for (brush = o->objects.p_next; brush != &o->objects; brush = brush->p_next)
        brush->setSelected(true);
}

void map::makeEntity()
{
    if (current != world)
    {
        BSPHelp(const_cast<char *> ("Make Entity..."), const_cast<char *> ("Can't make an entity inside another entity"));
        return;
    }

    if (!strcmpi(kpWindow->spawnName(),"worldspawn"))
    {
        BSPHelp(const_cast<char *> ("Make Entity"), const_cast<char *> ("Can't make another world entity!"));
        return;
    }

    int newindex = 0;
    if (groupWindow)
    {
        newindex = groupWindow->GroupList->GetSelIndex();

        if (newindex < 0 || newindex >= groupWindow->GroupList->GetCount()) // name field not one of the groups...
            newindex = 0; // put in default if no group is current...

        if (set.group_mode && !groups->GetVisible(newindex))
        {
            int retval;
            retval = MessageBox(0, const_cast<char *> ("Creating entity in an invisible group, make group visible?"),
                                const_cast<char *> ("BSP - Make Entity"),
                                MB_YESNO | MB_ICONQUESTION);

            if (retval == IDYES)
            {
                groups->SetVisible(newindex,true);
                groupWindow->RedrawReload();
            };
            UpdateMapVisibility();
        };
    };
    EntityClass *ec;
    ec = entity_classes_i->classForName(kpWindow->spawnName());

    int ns;
    SetBrush *b;
    vec3_t mins,maxs;

    ns = numSelected();

    if (ns == 0)
    {
        if (ec && (ec->esize == esize_fixed))
        {
            b = new SetBrush();
            for (int i = 0; i < 3; i++)
            {
                mins[i] = eye[cureye][i];
                maxs[i] = eye[cureye][i]+8.0f;
            };
            b->setMins(mins,maxs);
            b->setParent(current);
            current->addObject(b);
            b->setSelected(true);
        }
        else
        {
            MessageBox(0, "Cannot create variable sized entities without seed brush(es)...",
                       "BSP - Make Entity", MB_OK | MB_ICONEXCLAMATION);
            return;
        };
    };

    sb_newowner = new Entity;
    sb_newowner->initClass(kpWindow->spawnName());

    dirty = 1;

    if (sb_newowner->modifiable)
    {
        makeSelectedPerform(SEL_MOVETOENTITY);
    }
    else  	// throw out seed brush and select entity fixed brush
    {
        makeSelectedPerform(SEL_REMOVE);
        if (sb_newowner->objects.p_next != &sb_newowner->objects)
        {
            sb_newowner->objects.p_next->setSelected(true);
            sb_newowner->objects.p_next->group = newindex;
        };
    }

    addObject(sb_newowner);
    setCurrentEntity(sb_newowner);

    set.redrawedit = 1;
    set.redrawxy = 1;
    Show_Frame("Entity made from brush",true);
}

void map::selbox(int sel)
{
    if (numSelected() != 1)
    {
        //	qprintf ("must have a single brush selected");
        return;
    }

    SetBrush *b = selectedBrush();
    b->getMins(select_min,select_max);
    b->remove();
    delete b;

    dirty = 1;

    makeUnselectedPerform(sel);
}

void map::selectCompletelyInside()
{
    dirty = 1;
    selbox(SEL_SELECTCOMPLETE);
}

void map::selectPartiallyInside()
{
    dirty = 1;
    selbox(SEL_SELECTPARTIAL);
}

void map::makeRegionBrush()
{
    // get the seed..
    SetBrush *b;

    if (numSelected() != 1)
    {
        //	qprintf ("must have a single brush selected");
        return;
    }

    vec3_t rmin, rmax;

    saveForUndo(const_cast<char *> ("Make Region"), UNDO_BRUSHES);

    b = selectedBrush();
    b->getMins(rmin,rmax);
    b->remove();

    if (regionBrush)
        delete regionBrush;

    regionBrush = b;

    if (!set.region_ignore)
    {
        if (MessageBox(0, const_cast<char *> ("Ignore Region Brush Height ? "), const_cast<char *> ("BSP Region"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
        {
            rmin[xyWindow[set.curxy]->NormalAxis] = -set.world_minmax;
            rmax[xyWindow[set.curxy]->NormalAxis] = set.world_minmax;

            b->setMins(rmin,rmax);
        }
    }
    else
    {
        if (set.region_ignore == 1)
        {
            rmin[xyWindow[set.curxy]->NormalAxis] = -set.world_minmax;
            rmax[xyWindow[set.curxy]->NormalAxis] = set.world_minmax;

            b->setMins(rmin,rmax);
        }
    }

    dirty = 1;
    regionSet = true;
    makeGlobalPerform(SEL_SETREGIONED);
}

void map::makeRegionBrushPartial()
{
    if (numSelected() != 1)
    {
        //	qprintf ("must have a single brush selected");
        return;
    }

    vec3_t rmin, rmax;

    saveForUndo(const_cast<char *> ("Make Region Partial"), UNDO_BRUSHES);

    // get the seed..
    SetBrush *b = selectedBrush();
    b->getMins(rmin,rmax);
    b->remove();

    if (regionBrush)
        delete regionBrush;

    regionBrush = b;

    if (0 == set.region_ignore)  	//prompt
    {
        if (MessageBox(0, const_cast<char *> ("Ignore Region Brush Height ? "), const_cast<char *> ("BSP Region"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
        {
            rmin[xyWindow[set.curxy]->NormalAxis] = -set.world_minmax;
            rmax[xyWindow[set.curxy]->NormalAxis] = set.world_minmax;

            b->setMins(rmin,rmax);
        }
    }
    else if (1 == set.region_ignore)    //ignore height
    {
        rmin[xyWindow[set.curxy]->NormalAxis] = -set.world_minmax;
        rmax[xyWindow[set.curxy]->NormalAxis] = set.world_minmax;
        b->setMins(rmin,rmax);
    } //else dont ignore height

    dirty = 1;
    regionSet = true;
    makeGlobalPerform(SEL_SETREGIONEDPARTIAL);
}



void map::GlobalSetKey(char *key, char *val)
{
    int anySelected;
    Entity *o;
    // no target/targetname or classname setting

    if (!key || !*key || !val || !*val)
        return;

    if (!strcmpi(key,"classname"))
    {
        if (MessageBox(0,"Are you sure you want to set 'classname' globally?",
                       "BSP Set Key Pair",MB_YESNO | MB_ICONEXCLAMATION) != IDYES)
            return;
    }
    if (!strnicmp(key,"target",6))
    {
        if (MessageBox(0,"Are you sure you want to set 'target' or 'targetname' globally?",
                       "BSP Set Key Pair",MB_YESNO | MB_ICONEXCLAMATION) != IDYES)
            return;
    }

    if (objects.p_next == &objects)
        return;

    for (o = objects.p_next; o != &objects; o = o->p_next)
    {
        if (o == world)
            continue;

        anySelected = 0;
        SetBrush *b;

        for (b = o->objects.p_next; !anySelected && (b != &o->objects); b = b->p_next)
        {
            if (!b->IsDrawable())
                continue;
            if (b->IsSelected())
                anySelected = 1;
        }

        if (!anySelected)
            continue;

        // if any, copy whole entity...
        o->setKey(key,val);
    }
}

int map::countSelectedBrushesWithEdge(vec3_t p1, vec3_t p2)
{
    Entity *ent;
    SetBrush *brush;

    int brush_count = 0;

    for (ent = objects.p_next; ent && (ent != &objects); ent = ent->p_next)
    {
        LoopProblem("CSBWE");
        if (!ent->modifiable)
            continue;

        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            LoopProblem("CSBWEB");
            if (!brush->IsDrawable())
                continue;

            if (!brush->IsSelected())
                continue;

            // test brush faces, if find one with "both" points, then
            // increment brushCount and go to next brush...
            if (brush->checkEdgeDragFaces(p1,p2))
                brush_count++;
        }
    }

    return brush_count;
}

void map::groupSwap(int g1, int g2)
{
    if (!set.Map_Read) // only if read it in...
        return;

    int count = groups->count;
    if (count <= 1)
        return; // don't delete last one...

    if ((g1 >= count) || (g2 >= count))
        return;  // bad group #

    if ((g1 < 1) || (g2 < 1))
        return;  // bad group #

    int fakeIdx;
    SetBrush *brush;
    Entity *ent;

    fakeIdx = count+1;

    // cycle through and change g1 to fakeIdx
    for (ent = objects.p_next; ent && (ent != &objects); ent = ent->p_next)
    {
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            if (brush->group == g1)
                brush->group = fakeIdx;
        };
    };

    // cycle through and change g2 to g1
    for (ent = objects.p_next; ent && (ent != &objects); ent = ent->p_next)
    {
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            if (brush->group == g2)
                brush->group = g1;
        };
    };

    for (ent = objects.p_next; ent && (ent != &objects); ent = ent->p_next)
    {
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            if (brush->group == fakeIdx)
                brush->group = g2;
        };
    };
}

void map::groupInsert(int pos)
{
    if (!set.Map_Read) // only if read it in...
        return;

    int count = groups->count;
    if (count <= 1)
        return; // don't delete last one...

    if (pos >= count)
        return;  // bad group #

    if (pos < 1)
        return;  // bad group #

    SetBrush *brush;
    Entity *ent;

    // cycle through and add 1 to every group above the pos...
    for (ent = objects.p_next; ent && (ent != &objects); ent = ent->p_next)
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
            if (brush->group >= pos)
                brush->group++;
}

// Determine if any entities have a brush selected and, if so, make the first
//   such entity the current entity.  Called only after drag selecting multiple
//   brushes...
//
void map::UpdateCurrent()
{
    if (!set.Map_Read) // only if read it in...
        return;

    SetBrush *brush;
    Entity *ent;

    // Cycle through and add 1 to every group above the pos...
    for (ent = objects.p_next; ent && (ent != &objects); ent = ent->p_next)
    {
        if (ent == world)
            continue;

        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            if (!brush->IsDrawable())
                continue;

            if (!brush->IsSelected())
                continue;

            // If we get here we found a selected non world brush...
            if (!brush->parent)
                continue;

            setCurrentEntity(brush->parent);
            return;
        };
    };

    // Assume the world, just in case...
    setCurrentEntity(world);
}
