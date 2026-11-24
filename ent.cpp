#include "global.h"


bool wasBad;
// if Use_Groups then add new entities to current group?

vec3_t bad_mins = {-8, -8, -8};
vec3_t bad_maxs = { 8,  8,  8};

Entity::Entity()
{
    p_next = NULL;  // prev and next entity
    p_prev = NULL;
    epairs = NULL;
    owner = NULL;
    modifiable = 0;
    count = 0;
    group = 0;
    invalid = false;
    memset(&objects,0,sizeof(SetBrush));
    objects.p_next = objects.p_prev = &objects;
    undoCameFrom = NULL;
    ec = NULL;
    origin[0] = origin[1] = origin[2] = 0.0f;

    //cache
    classname = const_cast<char *> ("");
    targetname = const_cast<char *> ("");
    target = const_cast<char *> ("");
    angle = const_cast<char *> ("");
}

Entity::~Entity()
{
    if (p_prev || p_next)
        syserror(const_cast<char *> ("Deletion of linked entity!"));

    freeObjects(false);
    freeAll();

    objects.p_next = objects.p_prev = NULL;
}

bool Entity::lookForBrush(SetBrush *b)
{
    // no linkages!
    if (!b || !objects.p_next || !objects.p_prev)
        return false;

    for (SetBrush *cur = objects.p_next; cur != &objects; cur = cur->p_next)
    {
        LoopProblem("Elookforbrush");
        if (cur == b)
            return true;
    }
    return false;
}

void Entity::freeObjects(int killtoo)
{
    SetBrush *b, *n;
    for (b = objects.p_next; b && (b != &objects); b = n)
    {
        LoopProblem("Efreeobjects");
        n = b->p_next;
        removeObject(b);
        if (killtoo)
            delete b;
    }

    objects.p_next = objects.p_prev = &objects;

    count = 0;
}

Entity *Entity::copy()
{
    Entity *e = new Entity;
    e->modifiable = modifiable;
    e->group = group;
    e->ec = ec;

    for (SetBrush *src = objects.p_next; src != &objects; src = src->p_next)
    {
        LoopProblem("ECOPY");
        SetBrush *dst = src->copy();
        dst->setParent(e);
        dst->calcWindings();
        e->addObject(dst);
    }

    if (e->count != count)
        syserror(const_cast<char *> ("Copying entity, counts didn't match"));

    for (epair_t *ep = epairs ; ep ; ep=ep->next)
    {
        // don't copy target and targetname fields
        //if (strncmp(ep->key,"target",6))
        e->setKey(ep->key,ep->value);
    }

    VectorCopy(origin,e->origin);

    return e;
}

Entity *Entity::undoCopy()
{
    Entity *e;
    epair_t *ep;
    SetBrush *sb;

    e = new Entity;
    e->modifiable = modifiable;
    e->group = group;

    SetBrush *b;
    for (sb = objects.p_next; sb != &objects; sb = sb->p_next)
    {
        LoopProblem("EUNDOCOPY");
        b = sb->copy();
        b->setParent(e);
        e->addObject(b);
    }

    if (e->count != count)
        syserror(const_cast<char *> ("Copying entity, counts didn't match"));

    for (ep = epairs ; ep ; ep=ep->next)
        e->setKey(ep->key,ep->value);

    VectorCopy(origin,e->origin);

    e->undoCameFrom = this;
    return e;
}

Entity *Entity::undoCopyNoBrushes()
{
    Entity *e;
    epair_t *ep;

    e = new Entity;
    e->modifiable = modifiable;
    e->group = group;

    for (ep = epairs ; ep ; ep=ep->next)
        e->setKey(ep->key,ep->value);

    VectorCopy(origin,e->origin);

    e->undoCameFrom = this;
    return e;
}

void Entity::appendList(Entity *e)
{
    SetBrush *o, *n;

    for (o = e->objects.p_next; o != &e->objects; o = n)
    {
        LoopProblem("EAPPENDLIST");
        n = o->p_next;
        e->removeObject(o);
        if (!o->IsInvalid())
            addObject(o);
        else
            delete o;
    }
}

void Entity::empty()
{
    SetBrush *next;
    for (SetBrush *cur = objects.p_next; cur && cur != &objects; cur = next)
    {
        LoopProblem("EEMPTY");
        next = cur->p_next;
        cur->p_next = NULL;
        cur->p_prev = NULL;
    }

    objects.p_next = objects.p_prev = &objects;
    count = 0;
}

void Entity::addObject(SetBrush *b)
{
    if (b->p_prev || b->p_next)
    {
        syserror(const_cast<char *> ("addObject, already in a list"));
        return;
    }

    // Link in
    b->p_next = objects.p_next;
    b->p_prev = &objects;
    objects.p_next->p_prev = b;
    objects.p_next = b;
    count++;
}

void Entity::removeObject(SetBrush *b)
{
    if (b == &objects)
        return;

    if (!b->p_next && !b->p_prev)
    {
        //	syserror ("removeObject, object not in a list");
        return; // nothing to do...
    }
    if (!b->p_next || !b->p_prev)
    {
        syserror(const_cast<char *> ("removeObject, object mislinked..."));
        return;
    }

    b->p_next->p_prev = b->p_prev;
    b->p_prev->p_next = b->p_next;
    b->p_next = b->p_prev = NULL;

    //todo: the code below looks messed
    count--;

    if (count) // still has brushes
        return;
    if (!owner) // owner not set? !
        return;

    // the entity is empty, so remove the entire thing
    if (this == owner->world)
        return;	// never remove the world

    // empty object, remove this entity from world.
    // look for it and remove it if there...
    invalid = true;
    // owner->removeObject(this);
}

SetBrush *Entity::createFixedBrush(vec3_t org)
{
    vec3_t	emins, emaxs;
    float	*v, *v2;
    EntityClass *newClass;
    SetBrush *newBrush;
    texturedef_t td;

    // get class
    newClass = entity_classes_i->classForName(classname);
    if (newClass)
    {
        v = newClass->getmins();
        v2 = newClass->getmaxs();
    }
    else
    {
        v = bad_mins;
        v2 = bad_maxs;
    }

    modifiable = false;
    memset(&td,0,sizeof(td));
    strcpy (td.texture,"entity");

    VectorAdd (org, v, emins);
    VectorAdd (org, v2, emaxs);

    newBrush = new SetBrush();
    newBrush->initOwner(this,emins,emaxs,&td);
    newBrush->setEntityColor(newClass->drawColor());

    addObject(newBrush);
    return newBrush;
}

void Entity::initClass(char *e_classname)
{
    EntityClass *newClass;
    esize_t	esize;
    char	value[80];
    vec3_t	min, max;
    float	*v;

    modifiable = true;

    setKey(const_cast<char *> ("classname"),e_classname);

    // get class
    newClass = entity_classes_i->classForName(classname);
    ec = newClass;
    if (!newClass)
        esize = esize_model;
    else
        esize = newClass->esize;

    // create a brush if needed
    if (esize == esize_fixed)
    {
        v = newClass->mins;
        map_i[set.curmap]->selectedBrush()->getMins(min,max);
        VectorSubtract (min, v, min);

        sprintf (value, "%i %i %i",(int)min[0], (int)min[1], (int)min[2]);
        setKey(const_cast<char *> ("origin"),value);

        createFixedBrush(min);
    }
    else
        modifiable = true;
}


void Entity::freeAll()
{
    epair_t	*n;

    for (epair_t *e=epairs ; e ; e=n)
    {
        n = e->next;
        delete e;
    }
}

void Entity::setModifiable(bool m)
{
    modifiable = m;
}

char *Entity::valueForQKey(char *k)
{
    epair_t	*e;
    static char	ret[MAX_VALUE];

    for (e=epairs ; e ; e=e->next)
        if (!strcmp(k,e->key))
        {
            strcpy (ret, e->value);
            return ret;
        }
    return const_cast<char *> ("");
}

void Entity::getVector(vec3_t v,char *k)
{
    char *c = valueForQKey(k);
    v[0] = v[1] = v[2] = 0;
    sscanf (c, "%f %f %f", &v[0], &v[1], &v[2]);
}

void Entity::setKey(char *k,char *v)
{
    while (*k && *k <= ' ') k++;	// trim left

    if (!*k)
        return;	// don't set NULL values

    for (epair_t *e=epairs ; e ; e=e->next)
        if (!strcmp(k,e->key))
        {
            strncpy(e->value, v, MAX_VALUE);	// overwrite value if key already exists
            return;
        }

    epair_s *e = new epair_s;
    memset (e, 0, sizeof(epair_t));

    strncpy (e->key, k, MAX_KEY);
    strncpy (e->value, v, MAX_VALUE);

    // cache
    if(!strcmp(k,"classname"))
        classname = e->value;
    if(!strcmp(k,"targetname"))
        targetname = e->value;
    if(!strcmp(k,"target"))
        target = e->value;
    if(!strcmp(k,"angle"))
        angle = e->value;

    e->next = epairs;
    epairs = e;
}

int Entity::numPairs()
{
    int i=0;
    for (epair_t *e=epairs ; e ; e=e->next) i++;
    return i;
}

void Entity::removeKeyPair(char *key)
{
    if (!epairs)
        return;

    // cache
    if(!strcmp(key,"classname"))
        classname = const_cast<char *> ("");
    if(!strcmp(key,"targetname"))
        targetname = const_cast<char *> ("");
    if(!strcmp(key,"target"))
        target = const_cast<char *> ("");
    if(!strcmp(key,"angle"))
        angle = const_cast<char *> ("");

    epair_t	*e = epairs;
    if (!strcmp(e->key, key))
    {
        epairs = e->next;
        delete e;
        return;
    }

    for (; e ; e=e->next)
    {
        if (e->next && !strcmp(e->next->key, key))
        {
            epair_t	*e2 = e->next;
            e->next = e2->next;
            delete e2;
            return;
        }
    }
    syserror (const_cast<char *> ("WARNING: removeKeyPair: %s not found\n"), key);
}


/*
=============
targetname

If the entity does not have a "targetname" key, a unique one is generated
=============
*/
char *Entity::get_targetname()
{
    char *t = targetname;
    if (t && *t)
        return t;

    map *m = map_i[set.curmap];

    int maxt = 0;                  // skip world
    if (m->objects.p_next != &m->objects)
    {
        for (Entity *e = m->objects.p_next; e != &m->objects; e = e->p_next)
        {
            LoopProblem("ETNAME");
            if (e == m->world)
                continue;

            t = e->targetname;
            if (!t || *t != 't')
                continue;
            int tval = atoi (t+1);
            if (tval > maxt)
                maxt = tval;
        }
    }

    char	name[80];
    sprintf (name,"t%i",maxt+1);
    setKey(const_cast<char *> ("targetname"),name);
    return targetname;
}

/*
==============================================================================

FILE METHODS

==============================================================================
*/

int	nument;

int Entity::initFromTokens(map *m, Tokenizer &script)
{
    char outstr[80];
    static char key[MAXTOKEN];
    EntityClass *eclass;
    SetBrush *brush;
    char	*spawn;
    vec3_t	emins, emaxs;
    vec3_t	org;
    texturedef_t	td;
    esize_t	esize;
    int saveCur;

    if (!script.next (true))
        return 0;

    if (strcmp(script.token, "{"))
    {
        syserror (const_cast<char *> ("initFromTokens: { not found, %s instead"),script.token);
        return -1;
    }

    while(true)
    {
        script.getGroups = 1;
        if (!script.next (true))
        {
            script.getGroups = 0;
            break;
        }
        script.getGroups = 0;

        saveCur = script.curGroup;

        if (!strcmp (script.token, "}") )
            break;

        if (!strcmp (script.token, "{") )  	// read a brush
        {

            brush = new SetBrush();
            brush->initFromTokens(m,this,script);

            brush->originalIndex = numsb;

            if (!(numsb % 5))
            {
                if (*loadName)
                {
                    snprintf(outstr,sizeof(outstr),"Loading %s, [e%ld/b%ld]...  ",loadName,ent_count,numsb);
                    status->SetText(outstr);
                    status->Redraw();
                }
            }

            if (!brush->IsInvalid())
            {
                addObject(brush);
            }
            else
            {
                delete brush;	// error message, but just kill it...
            }
        }
        else
        {
            // read a key / value pair
            strcpy (key, script.token);
            script.next (false);
            setKey(key,script.token);
        }
    }

    nument++;

    // get class
    spawn = classname;
    eclass = entity_classes_i->classForName(spawn);
    ec = eclass;
    esize = eclass->esize;

    getVector(org,const_cast<char *> ("origin"));
    if (count && (esize != esize_model))
        empty();

    if (!count && (esize == esize_model))
    {
        wasBad = true;
        texWindow->getTextureDef(&td);

        emins[0] = org[0] - 8;
        emins[1] = org[1] - 8;
        emins[2] = org[2] - 8;
        emaxs[0] = org[0] + 8;
        emaxs[1] = org[1] + 8;
        emaxs[2] = org[2] + 8;

        brush = new SetBrush();
        brush->initOwner(this,emins,emaxs,&td);
        addObject(brush);
    }

    // create a brush if needed
    if (esize == esize_fixed)
    {
        brush = createFixedBrush(org);
        brush->group = saveCur;
    }
    else
        modifiable = true;

    // set all the brush colors
    COLORREF color = eclass->drawColor();

    for (brush = objects.p_next; brush != &objects; brush = brush->p_next)
    {
        LoopProblem("EMAKECOLOR");
        brush->setEntityColor(color);
    }
    return 1;
}

void Entity::writeToFILE(FILE *f, int writeWorldKeys, int selOnly, map *thismap)
{
    epair_t	*e;
    SetBrush *b;

    int anySelected = 0;
    if (selOnly)
    {
        for (b = objects.p_next; b != &objects; b = b->p_next)
        {
            if (b->IsSelected())
            {
                anySelected = 1;
                break;
            }
        }
    }

    if (selOnly && !anySelected)
    {
        // write a blank world object...
        if (this == thismap->world)
        {
            fprintf (f,"{\n");
            fprintf (f,"}\n");
        }
        return;
    }

    fprintf (f,"{\n");
    SetBrush *o = objects.p_next;

    // set an origin epair
    if (!modifiable)
    {
        if (writeWorldKeys)
            fprintf (f, "//\"%04i\"\n",o->group);  // two slashes and then the !0000! group number...

        setOriginKey();
    }

    // if this is not the world, then write them, if it is the world, only write if
    // writeworldkeys is set
    if (writeWorldKeys || this != thismap->world)
    {
        for (e=epairs ; e ; e=e->next)
            fprintf (f,"\"%s\"\t\"%s\"\n", e->key, e->value);
    }

    if (modifiable)
    {
        for (b = objects.p_next; b != &objects; b = b->p_next)
        {
            if (!b->IsInvalid() && (!selOnly || b->IsSelected()))
                b->writeToFILE(f,writeWorldKeys);
        }
    }
    fprintf (f,"}\n");
}

// set an origin epair
void Entity::setOriginKey()
{
    vec3_t mins, maxs;
    vec3_t org;
    float *v;
    EntityClass *ec;
    char value[128];

    if (modifiable || objects.p_next == &objects)
        return;

    objects.p_next->getMins(mins,maxs);
    ec = entity_classes_i->classForName(classname);
    if (ec)
        v = ec->mins;
    else
        v = vec3_origin;

    VectorSubtract(mins, v, org);
    sprintf (value, "%i %i %i",(int)org[0], (int)org[1], (int)org[2]);
    setKey(const_cast<char *> ("origin"),value);
}
