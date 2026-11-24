#include "global.h"

#define MIN_DELTA (1.0f/16.0f)

vec3_t	select_min, select_max;
bool select_deselect = false;
bool fakebrush = false;
bool clipDraw = false;

face_t *bestface = NULL;
float bestd;

float lightaxis[3] = {1.0f, 0.6f, 0.75f};

Entity *sb_newowner;
map **map_i = NULL;

vec3_t vec3_origin = {0, 0, 0};

void GetAxes(int ViewType, int *ua, int *va, int *na)
{
    switch (ViewType)
    {
    case Type_YZ:
        *ua = 1;
        *va = 2;
        *na = 0;
        break;
    case Type_XZ:
        *ua = 0;
        *va = 2;
        *na = 1;
        break;
        // case Type_XY:
    default:
        *ua = 0;
        *va = 1;
        *na = 2;
        break;
    }
}

void SplitBase(char *path, char *basepath, char *name)
{
    if (!path)
    {
        //basepath = NULL;
        //name = NULL;
        return;
    }

    char *start = path;
    int len = (int) strlen(path);
    int i = 0;
    while (i < len && start && *start != '\\' && *start != '/')
        basepath[i++] = *start++;

    if (i == len)
    {
        if (!loadMessageGiven && (set.game_mode == 2))
        {
            loadMessageGiven = true;
            MessageBox(0,
                       "BSP is currently running in Quake 2 mode,\n"
                       "but the .map file being loaded appears to\n"
                       "be a Quake .map because at least one relative\n"
                       "texture path is missing.  Texture names will\n"
                       "be prefaced with 'BAD\\' as relative path...",
                       "BSP - Open Map",
                       MB_OK | MB_ICONEXCLAMATION);
        }
        sprintf(basepath,"BAD%c",set.rel_path_separator);
        strcpy(name,path);
        return;
    }

    // put in a good separator, too.
    basepath[i++] = set.rel_path_separator;
    start++;
    basepath[i] = '\0';

    //   start++; // skip the double slashes
    i = 0;
    while (start && *start)
        name[i++] = *start++;

    name[i] = '\0';
}

float DistToLine(POINT pt, POINT st, POINT ed)
{
    float distance = TMAX;//BOGUS_RANGE;
    float r, s;
    float len;
    float v1, v2;

    v1 = (float)(ed.x - st.x);
    v2 = (float)(ed.y - st.y);

    len = sqrt(v1*v1 + v2*v2);

    if (len >= 0.0001f)  	// avoid divide by zero...
    {

        r = (float) (st.y-pt.y)*(st.y-ed.y)-(st.x-pt.x)*(ed.x-st.x);
        r = r/(len*len);

        s = (float) (st.y-pt.y)*(ed.x-st.x)-(st.x-pt.x)*(ed.y-st.y);
        s = s/(len*len);

        if (r >= 0.0f && r <= 1.0f)
            distance = fabs(s*len);
    }
    return distance;
}

float VectorLength(vec3_t v)
{
    return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

void VectorMA (vec3_t va, float sc, vec3_t vb, vec3_t vc)
{
    vc[0] = va[0] + sc*vb[0];
    vc[1] = va[1] + sc*vb[1];
    vc[2] = va[2] + sc*vb[2];
}

bool VectorCompare (vec3_t v1, vec3_t v2)
{
    return (v1[0] == v2[0] && v1[1] == v2[1] && v1[2] == v2[2]);
}

bool VectorIsClose (vec3_t v1, vec3_t v2)
{
    return (fabs(v1[0]-v2[0]) < 0.5 && fabs(v1[1]-v2[1]) < 0.5 && fabs(v1[2]-v2[2]) < 0.5);
}

void CrossProduct (vec3_t v1, vec3_t v2, vec3_t cross)
{
    cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
    cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
    cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

float DotProduct(vec3_t v1, vec3_t v2)
{
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

void VectorSubtract (vec3_t va, vec3_t vb, vec3_t out)
{
    out[0] = va[0]-vb[0];
    out[1] = va[1]-vb[1];
    out[2] = va[2]-vb[2];
}

void VectorAdd (vec3_t va, vec3_t vb, vec3_t out)
{
    out[0] = va[0]+vb[0];
    out[1] = va[1]+vb[1];
    out[2] = va[2]+vb[2];
}

void VectorCopy (vec3_t in, vec3_t out)
{
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
}
void VectorNormalize (vec3_t v)
{
    float length = VectorLength(v);

    v[0] /= length;
    v[1] /= length;
    v[2] /= length;
}

void VectorCalcNormal(vec3_t a,vec3_t b,vec3_t c, vec3_t out_normal)
{
    vec3_t t1, t2;

    t1[0] = a[0] - b[0];
    t1[1] = a[1] - b[1];
    t1[2] = a[2] - b[2];
    t2[0] = c[0] - b[0];
    t2[1] = c[1] - b[1];
    t2[2] = c[2] - b[2];

    CrossProduct(t1,t2, out_normal);
    VectorNormalize (out_normal);
}

void VectorScale (vec3_t v, float sc, vec3_t out)
{
    out[0] = v[0] * sc;
    out[1] = v[1] * sc;
    out[2] = v[2] * sc;
}


/*
==================
textureAxisFromPlane
==================
*/
vec3_t baxis[18] =
{
    { 0, 0, 1}, {1,0,0}, {0,-1, 0},		// floor
    { 0, 0,-1}, {1,0,0}, {0,-1, 0},		// ceiling
    { 1, 0, 0}, {0,1,0}, {0, 0,-1},		// west wall
    {-1, 0, 0}, {0,1,0}, {0, 0,-1},		// east wall
    { 0, 1, 0}, {1,0,0}, {0, 0,-1},		// south wall
    { 0,-1, 0}, {1,0,0}, {0, 0,-1}		// north wall
};


float TextureAxisFromPlane(plane_t *pln, float *xv, float *yv)
{
    float	dot;

    float best = 0;
    int bestaxis = 0;

    if (!set.texture_alignment)
    {
        for (int i=0 ; i<6; i++)
        {
            dot = DotProduct(pln->normal,baxis[i*3]);
            if (dot > best)
            {
                best = dot;
                bestaxis = i;
            }
        }
    }
    else
    {
        for (int i=5 ; i>=0; i--)
        {
            dot = DotProduct(pln->normal,baxis[i*3]);
            if (dot > best)
            {
                best = dot;
                bestaxis = i;
            }
        }
    }

    VectorCopy(baxis[bestaxis*3+1], xv);
    VectorCopy(baxis[bestaxis*3+2], yv);

    return lightaxis[bestaxis>>1];
}

void TextureAxisFromPlaneEx(plane_t *pln, vec3_t out[3])
{
    float best = 0;
    int bestaxis = 0;

    if (!set.texture_alignment)
    {
        for (int i=0 ; i<6; i++)
        {
            float dot = DotProduct(pln->normal,baxis[i*3]);
            if (dot > best)
            {
                best = dot;
                bestaxis = i;
            }
        }
    }
    else
    {
        for (int i=5 ; i>=0; i--)
        {
            float dot = DotProduct(pln->normal,baxis[i*3]);
            if (dot > best)
            {
                best = dot;
                bestaxis = i;
            }
        }
    }

    VectorCopy(baxis[bestaxis*3+0], out[0]);
    VectorCopy(baxis[bestaxis*3+1], out[1]);
    VectorCopy(baxis[bestaxis*3+2], out[2]);
}

/*
=================
CheckFace
Note: this will not catch 0 area polygons
=================
*/
int CheckFace(face_t *f)
{
    int		i, j;
    float	*p1, *p2;
    float	d, edgedist;
    vec3_t	dir, edgenormal;
    winding_t	*w;

    w = f->w;
    if (!w)
    {
        syserror(const_cast<char *> ("CheckFace: no winding"));
        return 0;
    }

    if (w->numpoints < 3)
    {
        syserror(const_cast<char *> ("CheckFace: %i points"), w->numpoints);
        return 0;
    }

    for (i=0 ; i<w->numpoints ; i++)
    {
        p1 = w->points[i];

        if (p1[0] > TMAX || p1[0] < -TMAX ||
                p1[1] > TMAX || p1[1] < -TMAX ||
                p1[2] > TMAX || p1[2] < -TMAX)
        {
            return 0;  //FIXME: if object goes outside of this range it disappears (deletes it)
        }

        j = i+1 == w->numpoints ? 0 : i+1;

        // check the point is on the face plane
        d = DotProduct (p1, f->plane.normal) - f->plane.dist;
        if (d < -ON_EPSILON || d > ON_EPSILON)
        {
            return 0;		// syserror ("CheckFace: point off plane %f",d);
        }
        // check the edge isn't degenerate
        p2 = w->points[j];
        VectorSubtract (p2, p1, dir);

        if (VectorLength (dir) < ON_EPSILON)
        {
            return 0;		// syserror ("CheckFace: degenerate edge");
        }

        CrossProduct (f->plane.normal, dir, edgenormal);
        VectorNormalize (edgenormal);
        edgedist = DotProduct (p1, edgenormal);
        edgedist += ON_EPSILON;

        // all other points must be on front side
        for (j=0 ; j<w->numpoints ; j++)
        {
            if (j == i)
                continue;
            d = DotProduct (w->points[j], edgenormal);
            if (d > edgedist)
            {
                syserror(const_cast<char *> ("CheckFace: non-convex"));
                return 0;
            }
        }
    }
    return 1;
}


/*
=============================================================================

			TURN PLANES INTO GROUPS OF FACES

=============================================================================
*/


/*
==================
NewWinding
==================
*/
winding_t *NewWinding (int points)
{
    if (points > MAX_POINTS_ON_WINDING)
    {
        //todo: allow more than MAX_POINTS_ON_WINDING ?????
        syserror(const_cast<char *> ("NewWinding: MAX_POINTS_ON_WINDING %i (max:%d)"), points, MAX_POINTS_ON_WINDING);
        return 0;
    }

    intptr_t size = (intptr_t)((winding_t *)0)->points[points];

    winding_t *w = (winding_t*) new char[size];
    memset (w, 0, size);

    w->numpoints = points;
    return w;
}


/*
==================
CopyWinding
==================
*/
winding_t *CopyWinding (winding_t *w)
{
    intptr_t size = (intptr_t)((winding_t *)0)->points[w->numpoints];
    winding_t *c = (winding_t *) new char[size];
    memcpy (c, w, size);
    return c;
}


/*
==================
ClipWinding

Clips the winding to the plane, returning the new winding on the positive side
Frees the input winding.
==================
*/
winding_t *ClipWinding (winding_t *in, plane_t *split)
{
    float	dists[MAX_POINTS_ON_WINDING];
    int		sides[MAX_POINTS_ON_WINDING];
    int		counts[3] = { 0, 0, 0 };
    float	dot;
    int		i;
    float	*p1, *p2, *mid;
    winding_t *neww;
    int		maxpts;

// determine sides for each point
    for (i=0 ; i<in->numpoints ; i++)
    {
        dot = DotProduct (in->points[i], split->normal);
        dot -= split->dist;
        dists[i] = dot;
        if (dot > ON_EPSILON)
            sides[i] = SIDE_FRONT;
        else if (dot < -ON_EPSILON)
            sides[i] = SIDE_BACK;
        else
            sides[i] = SIDE_ON;

        counts[sides[i]]++;
    }
    sides[i] = sides[0];
    dists[i] = dists[0];

    if (!counts[1])
        return in;

    if (!counts[0])
    {
        delete [] in;
        return NULL;
    }

    maxpts = in->numpoints+4;	// can't use counts[0]+2 because
    // of fp grouping errors
    neww = NewWinding (maxpts);
    if(!neww)
    {
        delete [] in;
        return 0;
    }
    neww->numpoints = 0;

    for (i=0 ; i < in->numpoints; i++)
    {
        p1 = in->points[i];

        mid = neww->points[neww->numpoints];

        if (sides[i] == SIDE_FRONT || sides[i] == SIDE_ON)
        {
            VectorCopy (p1, mid);
            mid[3] = p1[3];
            mid[4] = p1[4];
            neww->numpoints++;
            if (sides[i] == SIDE_ON)
                continue;
            mid = neww->points[neww->numpoints];
        }

        if (sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
            continue;

        // generate a split point
        if (i == in->numpoints - 1)
            p2 = in->points[0];
        else
            p2 = p1 + 5;

        neww->numpoints++;

        dot = dists[i] / (dists[i]-dists[i+1]);
        //for (j=0 ; j<3 ; j++)
        //{
        // avoid round off error when possible
        //	if (split->normal[j] == 1)
        //		mid[j] = split->dist;
        //	else if (split->normal[j] == -1)
        //		mid[j] = -split->dist;
        mid[0] = p1[0] + dot*(p2[0]-p1[0]);
        mid[1] = p1[1] + dot*(p2[1]-p1[1]);
        mid[2] = p1[2] + dot*(p2[2]-p1[2]);
        //}
        mid[3] = p1[3] + dot*(p2[3]-p1[3]);
        mid[4] = p1[4] + dot*(p2[4]-p1[4]);
    }

    // free the original winding
    delete [] in;

    if (neww->numpoints > maxpts)
        syserror (const_cast<char *> ("ClipWinding: points exceeded estimate"));

    return neww;
}

/*
=================
BasePolyForPlane

There has GOT to be a better way of doing this...
=================
*/
winding_t *BasePolyForPlane (face_t *f)
{
    int		i, x;
    float	max, v;
    vec3_t	org, vright, vup;
    vec3_t	xaxis, yaxis;
    winding_t	*w;
    texturedef_t	*td;
    plane_t		*p;
    float	ang, sinv, cosv;
    float	s, t, ns, nt;

    p = &f->plane;

// find the major axis

    max = -TMAX;
    x = -1;
    for (i=0 ; i<3; i++)
    {
        v = fabs(p->normal[i]);
        if (v > max)
        {
            x = i;
            max = v;
        }
    }
    if (x==-1)
    {
        syserror(const_cast<char *> ("BasePolyForPlane: no axis found"));
        return 0;
    }

    VectorCopy (vec3_origin, vup);
    if (x==0 || x==1)
    {
        vup[2] = 1;
    }
    else if (x==2)
    {
        vup[0] = 1;
    }

    v = DotProduct (vup, p->normal);
    VectorMA (vup, -v, p->normal, vup);
    VectorNormalize (vup);

    VectorScale (p->normal, p->dist, org);

    CrossProduct (vup, p->normal, vright);

    VectorScale (vup, TMAX, vup);   // 8192?
    VectorScale (vright, TMAX, vright);

// project a really big	axis aligned box onto the plane
    w = NewWinding (4);
    if(!w) return 0;

    VectorSubtract (org, vright, w->points[0]);
    VectorAdd (w->points[0], vup, w->points[0]);

    VectorAdd (org, vright, w->points[1]);
    VectorAdd (w->points[1], vup, w->points[1]);

    VectorAdd (org, vright, w->points[2]);
    VectorSubtract (w->points[2], vup, w->points[2]);

    VectorSubtract (org, vright, w->points[3]);
    VectorSubtract (w->points[3], vup, w->points[3]);

// set texture values
    f->light = TextureAxisFromPlane(&f->plane, xaxis, yaxis);

    //if (set.game_mode != 2) { // Only for non-Q2 games...
    // FIXME Should we let this just pass through?  no harm...
    td = &f->texture;

    // rotate axis
    ang = td->rotate / 180 * M_PI;
    sinv = sin(ang);
    cosv = cos(ang);

    if (!td->scale[0])
        td->scale[0] = 1;
    if (!td->scale[1])
        td->scale[1] = 1;

    for (i=0 ; i<4 ; i++)
    {
        s = DotProduct (w->points[i], xaxis);
        t = DotProduct (w->points[i], yaxis);

        ns = cosv * s - sinv * t;
        nt = sinv * s +  cosv * t;

        w->points[i][3] = ns/td->scale[0] + td->shift[0];
        w->points[i][4] = nt/td->scale[1] + td->shift[1];
    }
    //};
    return w;
}

void SetBrush::addObject(face_t *f)
{
//	if (f->p_prev || f->p_next)
//		syserror ("addObject, face already in a list");

    // Link in
    f->p_next = faces.p_next;
    f->p_prev = &faces;
    faces.p_next->p_prev = f;
    faces.p_next = f;

    numfaces++;
}

void SetBrush::removeObject(face_t *f)
{
    if (f == &faces)
        return;

    if (!f->p_next && !f->p_prev)
        return; // nothing to do...
    //	syserror ("removeObject, object not in a list");

    if (!f->p_next || !f->p_prev)
        syserror (const_cast<char *> ("removeObject, face mislinked..."));

    if (currentFace == f)
        currentFace = faces.p_next;

    f->p_next->p_prev = f->p_prev;
    f->p_prev->p_next = f->p_next;
    f->p_next         = f->p_prev = NULL;

    numfaces--;
}

/*
================
BeginTexturingFace
================
*/

// GLOBAL VAR.
//float shift[2];

// For Q2 Support, Set up the shift value based on the
// origin, if any.  No shift for 0,0,0 origin (DP is 0).
//
//void SetBrush::BeginTexturingFace(face_t *f)//, vec3_t origin)
//{
//	vec3_t	pvecs[2];
// get natural texture axis
//	TextureAxisFromPlane(&f->plane, pvecs[0], pvecs[1]);

//	shift[0] = DotProduct (origin, pvecs[0]);
//	shift[1] = DotProduct (origin, pvecs[1]);
//}

// Q2 Support
// Called for each point in the winding, calculate the NAT
// texture coordinates.
//
void SetBrush::EmitTextureCoordinates(float *xyzst, face_t *f)
{
    float	s, t, ns, nt;
    float	ang, sinv, cosv;
    vec3_t	vecs[2];
    texturedef_t *td;

    // get natural texture axis from plane normal
    TextureAxisFromPlane(&f->plane, vecs[0], vecs[1]);

    // point at this face's texture data.
    td = &f->texture;

    ang = DEG2RAD(td->rotate);
    sinv = sin(ang);
    cosv = cos(ang);

    if (!td->scale[0])
        td->scale[0] = 1;
    if (!td->scale[1])
        td->scale[1] = 1;

    // Project against axes
    s = DotProduct(xyzst, vecs[0]);
    t = DotProduct(xyzst, vecs[1]);

    // Deal with rotation
    ns = cosv * s - sinv * t;
    nt = sinv * s +  cosv * t;

    // Scale and offset
    s = ns/td->scale[0] + td->shift[0];
    t = nt/td->scale[1] + td->shift[1];

    // gl scales everything from 0 to 1
    // FIXME Do Scaling here...  or dot it for each face at render time
    // no big deal...
    //s /= q->width;
    //t /= q->height;

    // Remember to add back in the "origin" brush shifting factor.
    xyzst[3] = s;// + shift[0];
    xyzst[4] = t;// + shift[1];
}

SetBrush *SetBrush::copy()
{
    SetBrush *b = new SetBrush();
    b->flags      = flags;
    b->parent     = parent;
    b->group      = group;
    VectorCopy(bmaxs, b->bmaxs);
    VectorCopy(bmins, b->bmins);
    b->entitycolor = entitycolor;

    for (face_t *f = faces.p_next; f != &faces; f = f->p_next)
    {
        LoopProblem("brush copy");
        face_t *f2 = new face_t;
        *f2 = *f;
        f2->w        = NULL;  // clear out the windings!
        f2->qtexture = NULL;  // and the textures!
        f2->p_next = f2->p_prev = NULL;

        b->addObject(f2);
        if (f == currentFace)
            b->currentFace = f2;
    }

    // don't copy brushId because it is automatically generated...
    b->undoCameFrom = this;
    return b;
}

SetBrush::SetBrush()
{
    p_prev = p_next = NULL;
    flags = 0;

    parent = NULL;
    memset(&bmins,0,sizeof(vec3_t));
    memset(&bmaxs,0,sizeof(vec3_t));
    entitycolor = RGB(255,255,0);
    memset(&faces,0,sizeof(face_t));
    faces.p_next = faces.p_prev = &faces;
    numfaces = 0;

    group = 0; // unselected group...
    currentFace = NULL;
    undoCameFrom = NULL;
    next = NULL;
    originalIndex = 0;
}

SetBrush::~SetBrush()
{
    if (p_prev || p_next)
        ::MessageBox(0,"Deletion of linked brush!","BSP",MB_OK | MB_ICONEXCLAMATION);

    RemoveHitBrush(this);
    freeWindings();

    if (!&faces || !faces.p_next)
        return;

    face_t *f, *n;
    for (f = faces.p_next; f != &faces; f = n)
    {
        LoopProblem("brush delete");
        n = f->p_next;
        removeObject(f);
        delete f;
    }
}

int SetBrush::CountFaces()
{
    return numfaces;
}

void SetBrush::removeAllFaces()
{
    freeWindings();
    face_t *f, *n;
    for (f = faces.p_next; f != &faces; f = n)
    {
        LoopProblem("brush removeallfaces");
        n = f->p_next;
        removeObject(f);
        delete f;
    }
    currentFace = 0;
}
/*
===========
calcWindings

recalc the faces and mins / maxs from the planes
If a face has a NULL winding, it is an overconstraining plane and
can be removed.
===========
*/
void SetBrush::calcWindings()
{
    int				i,j, k;
    float			v;
    face_t			*f;
    winding_t		*w;
    plane_t			plane;
    vec3_t			t1, t2, t3;
    bool			useplane[MAX_FACES];

    bmins[0] = bmins[1] = bmins[2] = TMAX;
    bmaxs[0] = bmaxs[1] = bmaxs[2] = -TMAX;
    bctr[0] = bctr[1] = bctr[2] = 0.0f;

    setInvalid(false);

    freeWindings(false);	//TODO!!!!! does this leak? (qtexture)

    float d, d2;

    face_t *o, *curface = 0;
    i = 0;
    for (o = faces.p_next; o != &faces; o = o->p_next, i++ )
    {
        LoopProblem("brush calc");
        f = o;

        // calc a plane from the points
        for (j=0 ; j<3 ; j++)
        {
            t1[j] = f->planepts[0][j] - f->planepts[1][j];
            t2[j] = f->planepts[2][j] - f->planepts[1][j];
            t3[j] = f->planepts[1][j];
        }

        CrossProduct(t1,t2, f->plane.normal);
        if (VectorCompare (f->plane.normal, vec3_origin))
        {
            useplane[i] = false;
            continue;
        }
        VectorNormalize (f->plane.normal);
        f->plane.dist = DotProduct (t3, f->plane.normal);

        // if the plane duplicates another plane, ignore it
        // (assume it is a brush being edited that will be fixed)
        useplane[i] = true;
        face_t *f2;
        j = 0;
        // Start at beginning and go until we get to the current face
        for (f2 = faces.p_next; (f2 != &faces) && (j < i); f2 = f2->p_next, j++)
        {
            LoopProblem("brush calc2");

            d2 = (f->plane.normal[0] - f2->plane.normal[0]);
            d = d2*d2;
            d2 = (f->plane.normal[1] - f2->plane.normal[1]);
            d += d2*d2;
            d2 = (f->plane.normal[2] - f2->plane.normal[2]);
            d = sqrt(d + d2*d2);

            if (d < 0.05f && fabs(f->plane.dist-f2->plane.dist) < 0.05f)
            {
                useplane[i] = false;
                break;
            }
        }
    }

    vec3_t center = {0,0,0};
    int numfaces = 0;
    i = 0;
    for (o = faces.p_next; o != &faces; o = o->p_next)
    {
        LoopProblem("brush calc3");
        if (!useplane[i])
        {
            i++;
            continue;			// duplicate plane
        }

        f = o;

        if(currentFace == f)
            curface = f;

        w = BasePolyForPlane (f);
        face_t *o2;
        for (o2 = faces.p_next; o2 != &faces && w; o2 = o2->p_next)
        {
            LoopProblem("brush calc4");
            if (o == o2)
                continue;

            // flip the plane, because we want to keep the back side
            VectorSubtract (vec3_origin, o2->plane.normal, plane.normal);
            plane.dist = -o2->plane.dist;

            w = ClipWinding (w, &plane);
        }
        f->w = w;
        if (w)
        {
            if (CheckFace(f))
            {
                for (j=0 ; j<w->numpoints ; j++)
                {
                    for (k=0 ; k<3 ; k++)
                    {
                        v = w->points[j][k];  // fix for ceil on neg numbers...
                        if (fabs(v - rint(v)) < FP_EPSILON)
                            v = w->points[j][k] = (float) rint(v);
                        if (v < bmins[k])
                            bmins[k] = v;
                        if (v > bmaxs[k])
                            bmaxs[k] = v;
                    }
                }

                //calc center
                vec3_t polycent = {0, 0, 0};
                for(int n=0; n<w->numpoints; n++)
                {
                    polycent[0] = polycent[0] + w->points[n][0];
                    polycent[1] = polycent[1] + w->points[n][1];
                    polycent[2] = polycent[2] + w->points[n][2];
                }
                VectorScale(polycent, 1.0f/(float)w->numpoints, polycent);
                VectorAdd(polycent, center, center);
                numfaces++;


                //set display transparency flag
                //f->transparent = IsTextureTransparent(&f->texture);
                UpdateTextureTransFlags(f);

                if (set.game_mode == 2)
                {
                    vec3_t org;

                    // make a copy
                    face_t texFace = *f;
                    // don't fuck it up, since we were just borrowing the pointers
                    // and we don't want to delete them or anything when we go out of scope.
                    texFace.w        = NULL;
                    texFace.qtexture = NULL;
                    VectorCopy(vec3_origin,org);
                    if (parent && (parent->origin[0] || parent->origin[1] || parent->origin[2]))
                    {
                        float newdist = f->plane.dist - DotProduct(f->plane.normal, parent->origin);
                        texFace.plane.dist = newdist;
                        VectorCopy(parent->origin,org);
                    }

                    //BeginTexturingFace(&texFace, org);

                    for (j=0 ; j<w->numpoints ; j++)
                        EmitTextureCoordinates(w->points[j],&texFace);
                }

            }
            else
            {
                // bad face
                bmins[0] = TMAX; // get out of the loop and set to invalid...
                setInvalid(true);
                break;
            }					// end bad face
        } // w exists...
        i++;
    }


    /*
    //TEST - THIS DETECTS INVALID PLANE PTS. checks if any decimal is not close to x.0
    for (face_t *o = faces.p_next; o != &faces; o = o->p_next) {
    	for(j=0;j<3;j++)
    	for(i=0;i<3;i++)
    		if( fabs(o->planepts[j][i] - (float)int(o->planepts[j][i])) > 0.01) {
    			//FNHIT();
    		}
    }
    //END TEST
    */

    for (i = 0; i < 3; i++)
    {
        if ((bmins[i] >= TMAX) || (bmaxs[i] <= -TMAX))
        {
            setInvalid(true);
            VectorCopy (vec3_origin, bmins);
            VectorCopy (vec3_origin, bmaxs);
            break;
        }
    }

    // to preserve selection, set new current face only if old current face was not valid
    if(!curface)
        currentFace = faces.p_next;

    if (!IsInvalid())
    {
        //bctr[0] = (bmins[0] + bmaxs[0]) / 2.0f;
        //bctr[1] = (bmins[1] + bmaxs[1]) / 2.0f;
        //bctr[2] = (bmins[2] + bmaxs[2]) / 2.0f;

        bctr[0] = center[0] / (float)numfaces;
        bctr[1] = center[1] / (float)numfaces;
        bctr[2] = center[2] / (float)numfaces;
    }

}

//============================================================================
//		snaps all the plane points to the grid. unfortunately, this
//		method fails to snap winding points at most non-axial plane intersections.
//      HOWEVER, it does a good job of re-aligning odd-shaped brushes.
void SetBrush::snapPlanes()
{
    for (face_t *f = faces.p_next; f != &faces; f = f->p_next)
    {
        winding_t *w = f->w;
        if(!w)
            continue;
        //snap plane to winding points
        VectorCopy(w->points[0], f->planepts[0]);
        VectorCopy(w->points[1], f->planepts[1]);
        VectorCopy(w->points[2], f->planepts[2]);
        //snap all plane points to grid
        for(int i=0; i < 3; i++)
        {
            f->planepts[i][0] = (float) snapToGrid(f->planepts[i][0]);
            f->planepts[i][1] = (float) snapToGrid(f->planepts[i][1]);
            f->planepts[i][2] = (float) snapToGrid(f->planepts[i][2]);
        }

    }
    calcWindings();
}

void SetBrush::snapSelf()
{
    bool changed = false;
    // look for axial faces...
    for (face_t *f = faces.p_next; f != &faces; f = f->p_next)
    {
        LoopProblem("brush snap");
        plane_t *plane = &f->plane;

        // classify the plane...
        // if it's axial, set anyequal to the normal index...
        int anyequal1 = -1;
        for (int j = 0; j < 3; j++)
        {
            if (fabs(fabs(plane->normal[j]) - 1.0) < FP_EPSILON)
            {
                anyequal1 = j;
                break;
            }
        }

        if (anyequal1 < 0)						// not axial, don't do anything...
            continue;

        int dir = -1;							// back facing def...
        // now, first, make it axial?
        if (plane->normal[anyequal1] > 0)  		// forward facing?
        {
            dir = 1;
        }

        int jumpto = -1;
        float best = 99999.0f * dir;
        for (int j = 0; j < 3; j++)
        {
            if (dir > 0)
            {
                if (f->planepts[j][anyequal1] < best)
                {
                    best = f->planepts[j][anyequal1];
                    jumpto = j;
                }
            }
            else
            {
                if (f->planepts[j][anyequal1] > best)
                {
                    best = f->planepts[j][anyequal1];
                    jumpto = j;
                }
            }
        }

        // no best found!!!!
        if (jumpto == -1)
            continue;

        // snap it
        best = (float) snapToGrid(best);

        f->planepts[0][anyequal1] = best;
        f->planepts[1][anyequal1] = best;
        f->planepts[2][anyequal1] = best;

        changed = true;
    }

    if (changed)
        calcWindings();
}

/*
===========
initOwner:::
===========
*/
void SetBrush::initOwner(Entity* own,float *mins,float *maxs,texturedef_t *tex)
{
    parent = own;
    setTexturedef(tex);
    setMins(mins,maxs);

    EntityClass *eclass = entity_classes_i->classForName(parent->classname);
    own->ec = eclass;

    setEntityColor(eclass->drawColor());
}

void SetBrush::setMins(float *mins,float *maxs)
{
    int		i, j;
    vec3_t	pts[4][2];

    freeWindings(false);
    removeAllFaces();

    for (i=0 ; i<3 ; i++)
    {
        if (maxs[i] - mins[i] <= 0)
        {
            VectorCopy (mins, bmins);
            VectorCopy (maxs, bmaxs);
            setInvalid(true);
            // remove();
            return;
        }
    }

    pts[0][0][0] = mins[0];
    pts[0][0][1] = mins[1];

    pts[1][0][0] = mins[0];
    pts[1][0][1] = maxs[1];

    pts[2][0][0] = maxs[0];
    pts[2][0][1] = maxs[1];

    pts[3][0][0] = maxs[0];
    pts[3][0][1] = mins[1];

    for (i=0 ; i<4 ; i++)
    {
        pts[i][0][2] = mins[2];
        pts[i][1][0] = pts[i][0][0];
        pts[i][1][1] = pts[i][0][1];
        pts[i][1][2] = maxs[2];
    }

    face_t *f;
    for (i=0 ; i<4 ; i++)
    {
        j = (i+1)%4;

        f = new face_t;
        memset(f, 0, sizeof(face_t));

        f->planepts[0][0] = pts[j][1][0];
        f->planepts[0][1] = pts[j][1][1];
        f->planepts[0][2] = pts[j][1][2];

        f->planepts[1][0] = pts[i][1][0];
        f->planepts[1][1] = pts[i][1][1];
        f->planepts[1][2] = pts[i][1][2];

        f->planepts[2][0] = pts[i][0][0];
        f->planepts[2][1] = pts[i][0][1];
        f->planepts[2][2] = pts[i][0][2];

        addObject(f);
    }

    f = new face_t;
    memset(f, 0, sizeof(face_t));

    f->planepts[0][0] = pts[0][1][0];
    f->planepts[0][1] = pts[0][1][1];
    f->planepts[0][2] = pts[0][1][2];

    f->planepts[1][0] = pts[1][1][0];
    f->planepts[1][1] = pts[1][1][1];
    f->planepts[1][2] = pts[1][1][2];

    f->planepts[2][0] = pts[2][1][0];
    f->planepts[2][1] = pts[2][1][1];
    f->planepts[2][2] = pts[2][1][2];

    addObject(f);

    f = new face_t;
    memset(f, 0, sizeof(face_t));

    f->planepts[0][0] = pts[2][0][0];
    f->planepts[0][1] = pts[2][0][1];
    f->planepts[0][2] = pts[2][0][2];

    f->planepts[1][0] = pts[1][0][0];
    f->planepts[1][1] = pts[1][0][1];
    f->planepts[1][2] = pts[1][0][2];

    f->planepts[2][0] = pts[0][0][0];
    f->planepts[2][1] = pts[0][0][1];
    f->planepts[2][2] = pts[0][0][2];

    addObject(f);

    texturedef_t td;
    texWindow->getTextureDef(&td);
    setTexturedef(&td);
    //calcWindings();
}

void SetBrush::setParent(Entity *p)
{
    parent = p;
}

void SetBrush::setEntityColor(COLORREF color)
{
    entitycolor = color;
}

void SetBrush::freeWindings(bool free_qtexture)
{
    if (!&faces || !faces.p_next)
        return;

    for (face_t *f = faces.p_next; f != &faces; f = f->p_next)
    {
        LoopProblem("brush free w");
        if (f->w)
            delete [] f->w;
        if(free_qtexture)
        {
            delete f->qtexture;
            f->qtexture = NULL;
        }

        f->w = NULL;
    }
}

/*
===========
initOwner: fromTokens
===========
*/

int numsb;

void SetBrush::initFromTokens(map * /*m*/, Entity *own, Tokenizer &script)
{
    face_t*f;
    int i;

    parent = own;
    map *m = map_i[set.curmap];

    while (true)
    {
        script.getGroups = 1;
        if (!script.next (true))
        {
            script.getGroups = 0;
            break;
        }
        script.getGroups = 0;

        if (!strcmp (script.token, "}") )
            break;

        // parsed in!
        group = script.curGroup;
        group = min(group,(m->groups->NumGroups()-1));
        group = max(group,0);

        setLocked(script.curLock);

        f = new face_t;
        memset(f, 0, sizeof(face_t));

        script.putback();	// a token is already waiting...
        for (i=0 ; i<3 ; i++)
        {
            script.next (true);
            if (strcmp (script.token, "(") )
            {
                setInvalid(true);
                goto badbrush;
            }

            //get plane points
            script.next (false);       // need to alter the writing to find integral stuff! TODO WTF? wtf again?
            f->planepts[i][0] = (float) atoi(script.token);  // save it as a float...
            script.next (false);
            f->planepts[i][1] = (float) atoi(script.token);
            script.next (false);
            f->planepts[i][2] = (float) atoi(script.token);


            script.next (false);
            if (strcmp (script.token, ")") )
            {
                setInvalid(true);
                goto badbrush;
            }
        }

        script.next (false);
        strncpy (f->texture.texture, script.token, LUMP_NAME_LENGTH_MAX);
        f->texture.texture[LUMP_NAME_LENGTH_MAX-1] = 0;
        strupr(f->texture.texture);

        if (set.game_mode == 2)
        {
            // copy the base path...
            f->texture.texType = 1;
            // FIXME
            char realName[32];
            SplitBase(f->texture.texture, f->texture.basepath, realName);
            strncpy(f->texture.texture,realName,sizeof(f->texture.texture));
            f->texture.texture[sizeof(f->texture.texture)-1] = 0;
        }
        script.next (false);
        f->texture.shift[0] = (float) atof(script.token);
        script.next (false);
        f->texture.shift[1] = (float) atof(script.token);
        script.next (false);
        f->texture.rotate = (float) atof(script.token);
        script.next (false);
        f->texture.scale[0] = (float) atof(script.token);
        script.next (false);
        f->texture.scale[1] = (float) atof(script.token);

        f->defaults = true;
        f->texture.contents = f->texture.value = f->texture.flags = 0;
        // skip any hexen stuff..	.
        if (set.game_mode == 1)
        {
            if (script.avail_line())
            {
                script.next(false);
                f->texture.value = atoi(script.token);
            }
        }
        else if (set.game_mode == 2)
        {
            if (script.avail_line())    // if one is used, then no default anymore...
            {
                script.next(false);
                // quake 2 stuff..
                f->texture.contents = atoi(script.token);
                f->defaults = false;
            }
            if (script.avail_line())
            {
                script.next(false);
                // quake 2 stuff..
                f->texture.flags = atoi(script.token);
            }
            if (script.avail_line())
            {
                script.next(false);
                // quake 2 stuff..
                f->texture.value = atoi(script.token);
            }
        }

        // Clear out rest of line...
        script.skipline();

        addObject(f);
    }

    numsb++;

badbrush:
    if (!IsInvalid())
    {
        calcWindings();
        // kill the brush with duplicate planes...
        removeUnusedFaces();
    }
}

/*
===========
writeToFILE
===========
*/
void SetBrush::writeToFILE(FILE *f,int grp)
{
    face_t *fa;
    texturedef_t *td;

    fprintf (f, "{\n");

    if (grp)
    {
        fprintf (f, "//\"%04i\" \"%i\"\n",group,(IsLocked() ? 1:0));  // two slashes and then the "0000" group number...
    }

    char scaleX[32];
    char scaleY[32];

    for (fa = faces.p_next; fa != &faces; fa = fa->p_next)
    {
        fprintf(f,"( %d %d %d ) ( %d %d %d ) ( %d %d %d ) ",
                (int)fa->planepts[0][0], (int)fa->planepts[0][1], (int)fa->planepts[0][2],
                (int)fa->planepts[1][0], (int)fa->planepts[1][1], (int)fa->planepts[1][2],
                (int)fa->planepts[2][0], (int)fa->planepts[2][1], (int)fa->planepts[2][2]
               );

        td = &fa->texture;
        sprintf(scaleX,"%f",td->scale[0]);
        sprintf(scaleY,"%f",td->scale[1]);
        FixFloatString(scaleX);
        FixFloatString(scaleY);

        if (set.game_mode == 1)          // Hexen 2
        {
            fprintf(f,"%s %d %d %d", td->texture, (int)td->shift[0], (int)td->shift[1], (int)td->rotate);
            fprintf(f," %s %s %i", scaleX, scaleY, td->value);
        }
        else if (set.game_mode == 2)     // Quake 2
        {
            char bl[64];
            char tl[64];

            STRNCPY(bl,td->basepath);
            STRNCPY(tl,td->texture);
            strlwr(bl);
            strlwr(tl);
            fprintf (f,"%s%s %d %d %d %s %s", bl, tl, (int)td->shift[0], (int)td->shift[1], (int)td->rotate, scaleX, scaleY);
            if (td->value != fa->dvalue ||
                    td->flags != fa->dflags ||
                    td->contents != fa->dcontents)
            {
                fprintf (f, " %i %i %i", td->contents, td->flags, td->value);
            }
        }
        else
        {
            fprintf (f,"%s %d %d %d %s %s", td->texture, (int)td->shift[0], (int)td->shift[1], (int)td->rotate, scaleX, scaleY);
        }
        fprintf(f,"\n");
    }
    fprintf (f, "}\n");
}



/*
==============================================================================

INTERACTION

==============================================================================
*/

void SetBrush::getMins(vec3_t mins, vec3_t maxs)
{
    VectorCopy (bmins, mins);
    VectorCopy (bmaxs, maxs);
}


void SetBrush::setSaveSelected(bool s)
{
    if (s)
        flags |= B_SAVESEL;
    else
        flags &= ~B_SAVESEL;
}

bool SetBrush::getSaveSelected()
{
    return (flags & B_SELECTED);
}

bool SetBrush::IsOriginBrush()
{
    if (set.game_mode != 2)
        return false;

    // see if origin contents bit is set for first face.
    face_t *f = faces.p_next;
    if (f == &faces)
        return false;

    if (!(f->texture.contents & CONTENTS_ORIGIN))
        return false;

    return true;
}

bool SetBrush::IsDetailBrush()
{
    if (set.game_mode != 2)
        return false;

    // see if origin contents bit is set for first face.
    face_t *f = faces.p_next;
    if (f == &faces)
        return false;

    if (!(f->texture.contents & CONTENTS_DETAIL))
        return false;

    return true;
}

void SetBrush::setSelected(bool s)
{
    if (s)
        flags |= B_SELECTED;
    else
        flags &= ~B_SELECTED;
}

bool SetBrush::IsSelected()
{
    return (flags & B_SELECTED);
}

void SetBrush::setLocked(bool s)
{
    if (s)
        flags |= B_LOCKED;
    else
        flags &= ~B_LOCKED;
}

bool SetBrush::IsLocked()
{
    return (flags & B_LOCKED);
}

void SetBrush::setInvalid(bool s)
{
    if (s)
        flags |= B_INVALID;
    else
        flags &= ~B_INVALID;

    setDrawable();
}

bool SetBrush::IsInvalid()
{
    return (flags & B_INVALID);
}

void SetBrush::setRegioned(bool s)
{
    if (s)
        flags |= B_REGIONED;
    else
        flags &= ~B_REGIONED;

    setDrawable();
}

bool SetBrush::IsRegioned()
{
    return (flags & B_REGIONED);
}

bool SetBrush::IsFiltered()
{
    return (flags & B_FILTERED);
}

// Ignores selection state
void SetBrush::setDrawable()
{
    flags &= ~B_DRAWABLE;  // assume no

    // Don't draw invalid brushes..
    //if (IsInvalid())  // go ahead and leave for now...
    //	return;

    if (IsFiltered())
        return;

    // not drawable if group is hidden
    if (group >= 0 &&
            set.group_mode &&
            parent && parent->owner &&
            parent->owner->groups && !parent->owner->groups->GetVisible(group))
        return;

    if (set.region_mode && IsRegioned())
        return;

    // If get to here it's drawable
    flags |= B_DRAWABLE;
}

bool SetBrush::IsDrawable()
{
    return (flags & B_DRAWABLE);
}

/*
===========
setTexturedef
===========
*/
void SetBrush::setTexturedef(texturedef_t *tex)
{
    if (!tex)
        return;

    for (face_t *f = faces.p_next; f != &faces; f = f->p_next)
    {
        LoopProblem("brush set texture");
        f->texture = *tex;
        delete f->qtexture;
        f->qtexture = NULL;	    // So that it is recalculated.
    }

    calcWindings();	// in case texture coords changed
}

void SetBrush::setTexturedef(texturedef_t *tex, face_t *f)
{
    if (!f)
        return;

    if (!tex)
        return;

    f->texture = *tex;
    if (f->qtexture)
        delete f->qtexture;
    f->qtexture = NULL;	    // So that it is recalculated.
    calcWindings();	       // in case texture coords changed
}

/*
===========
texturedef
===========
*/
texturedef_t *SetBrush::texturedef()
{
    return &faces.p_next->texture;
}

texturedef_t *SetBrush::texturedefForFace(face_t *f)
{
    return &f->texture;
}

/*
===========
removeUnusedFaces
So created veneers don't stay around, sets the invalid flag...
===========
*/
void SetBrush::removeUnusedFaces()
{
    face_t *f, *n;

    for (f = faces.p_next; f != &faces; f = n)
    {
        LoopProblem("brush remove unused");
        n = f->p_next;

        // leave alone
        if (!f->w)
        {
            removeObject(f);
            if (currentFace == f)
                currentFace = n;
            delete f;
        }
    }

    if (CountFaces() < 4)
        setInvalid(true);
}

/*
===========
containsPoint
===========
*/

bool SetBrush::containsPoint(vec3_t pt)
{
    face_t *f;

    for (f = faces.p_next; f != &faces; f = f->p_next)
        if (DotProduct (f->plane.normal, pt) >= f->plane.dist)
            return false;
    return true;
}

/*
===========
clipRay

===========
*/
void SetBrush::clipRay(vec3_t p1, vec3_t p2,
                       vec3_t frontpoint, face_t **f_face,
                       vec3_t backpoint, face_t **b_face, int *fflag, int *bflag)
{
    face_t *frontface = 0, *backface = 0;
    face_t	*f;
    float	d1, d2, m;
    //	float	*start;

    //	start = p1;
    *f_face = NULL;
    *b_face = NULL;
    *fflag = -2;
    *bflag = -2;


    LoopProblem("brush clip ray");
    for (f = faces.p_next; f != &faces; f = f->p_next)
    {
        if (!f->w)
            continue;	// clipped off plane
        LoopProblem("brush clip ray 1 or more");
        d1 = DotProduct (p1, f->plane.normal) - f->plane.dist;
        d2 = DotProduct (p2, f->plane.normal) - f->plane.dist;
        if (d1 >= 0 && d2 >= 0)
        {
            // the entire ray is in front of the polytope
            *fflag = -1;
            *bflag = -1;
            return;
        }
        if (d1 > 0 && d2 < 0)
        {
            // new front plane
            frontface = f;
            m = d1 / (d1-d2);

            frontpoint[0] = p1[0] + m*(p2[0]-p1[0]);
            frontpoint[1] = p1[1] + m*(p2[1]-p1[1]);
            frontpoint[2] = p1[2] + m*(p2[2]-p1[2]);

            p1 = frontpoint;
        }
        if (d1 < 0 && d2 > 0)
        {
            // new back plane
            backface = f;
            m = d1 / (d1-d2);

            backpoint[0] = p1[0] + m*(p2[0]-p1[0]);
            backpoint[1] = p1[1] + m*(p2[1]-p1[1]);
            backpoint[2] = p1[2] + m*(p2[2]-p1[2]);

            p2 = backpoint;
        }
    }

    if(!frontface || !backface)
        return;//Msgbox("frontface or backface is null!");
    *f_face = frontface;
    *b_face = backface;
    *fflag = 0; // OK
    *bflag = 0;
}


/*
===========
hitByRay
===========
*/
void SetBrush::hitByRay(vec3_t p1, vec3_t p2, float *time, face_t **face)
{
    vec3_t	frontpoint, backpoint, dir;
    face_t *frontface, *backface;
    int fflag, bflag;

    if (!IsDrawable())
    {
        *time = -1;
        *face = NULL;
        return;
    }

    clipRay(p1,p2,frontpoint,&frontface,backpoint,&backface,&fflag,&bflag);

    if (fflag == -2 && bflag == -2)
    {
        // entire ray is inside the brush, select first face
        *time = 0;
        *face = NULL;
        return;
    }


    if (fflag < 0)
    {
        // ray started inside the polytope, don't select it
        *time = -1;
        *face = NULL;
        return;
    }

    VectorSubtract (p2, p1, dir);
    VectorNormalize (dir);
    VectorSubtract (frontpoint, p1, frontpoint);
    *time = DotProduct (frontpoint, dir);

//	if (*time < 0)
//		*time=abs(*time);
//		//Error
//		Msgbox("hitByRay: negative t");

    *face = frontface;
}

/*
===========
centerDistToRay
===========
*/
void SetBrush::centerDistToRay(vec3_t p1, vec3_t p2, float *ctrdist)
{
    vec3_t mins,maxs;
    vec3_t dir;
    vec3_t pt;

    // *ctrdist = MAX_NUM;

    // distance from centerpoint to ray...
    getMins(mins,maxs);

    VectorSubtract(p2, p1, dir);
    VectorNormalize(dir);

    // p1 is
    //pt = p1 + (ctr - p1) dot dir times dir

    vec3_t temp;
    VectorSubtract(bctr,p1,temp);
    float temp2 = DotProduct(temp,dir);

    pt[0] = p1[0] + temp2*dir[0];
    pt[1] = p1[1] + temp2*dir[1];
    pt[2] = p1[2] + temp2*dir[2];

    // dist is length of vector from ctr to pt
    VectorSubtract(bctr,pt,temp);

    *ctrdist = VectorLength(temp);
}

/*
==============================================================================

DRAWING ROUTINES

==============================================================================
*/

void SetBrush::drawConnections(HDC hdc, int whichXY)
{
    char *targ = parent->target;
    if (!targ || !*targ)
        return;

    int ViewType = xyWindow[whichXY]->ViewType;
    SIZE XYSize;
    XYSize.cx = xyWindow[whichXY]->XYSize.cx;
    XYSize.cy = xyWindow[whichXY]->XYSize.cy;

    vec3_t	dest, origin;
    vec3_t	mid;
    vec3_t	forward, right;
    vec3_t	min, max, temp;

    int ua, va, na;
    GetAxes(ViewType, &ua, &va, &na);

    float u = xy_eye[ua];
    float v = xy_eye[va];

    float xofs = XYSize.cx/2 + 0.5f;
    float yofs = XYSize.cy/2 + 0.5f;

    origin[ua] = ((bmins[ua] + bmaxs[ua])/2-u)*set.scale + xofs;
    origin[va] = ((bmins[va] + bmaxs[va])/2-v)*set.scale + yofs;
    origin[va] = XYSize.cy - origin[va] - 1.0f;

    HPEN pen = CreatePen(PS_SOLID,1,set.color_connection);
    HGDIOBJ oldpen = SelectObject(hdc,pen);

    map *m = map_i[set.curmap];
    for (Entity *e = m->objects.p_next; e != &m->objects; e = e->p_next)
    {
        if (strcmp (targ, e->targetname))
            continue;

        if (e->objects.p_next == &e->objects) // Never...
            continue;

        e->objects.p_next->getMins(min,max);

        dest[ua] = ((min[ua] + max[ua])/2-u)*set.scale + xofs;
        dest[va] = ((min[va] + max[va])/2-v)*set.scale + yofs;
        dest[va] = XYSize.cy - dest[va] - 1.0f;
        MoveToEx(hdc, (int) origin[ua], (int) origin[va], 0);
        LineTo(hdc, (int) dest[ua], (int) dest[va]);

        forward[ua] = dest[ua] - origin[ua];
        forward[va] = dest[va] - origin[va];
        forward[na] = 0;

        if (!forward[ua] && !forward[va])
            continue;

        VectorNormalize (forward);
        forward[ua] = 8.0f *set.scale*forward[ua];
        forward[va] = 8.0f *set.scale*forward[va];

        right[ua] = forward[va];
        right[va] = -forward[ua];

        mid[ua] = (dest[ua] + origin[ua])/2;
        mid[va] = (dest[va] + origin[va])/2;

        temp[ua] = mid[ua] + right[ua] - forward[ua];
        temp[va] = mid[va] + right[va] - forward[va];

        MoveToEx(hdc, (int) temp[ua], (int) temp[va], 0);
        LineTo(hdc, (int) mid[ua], (int) mid[va]);

        temp[ua] = mid[ua] - right[ua] - forward[ua];
        temp[va] = mid[va] - right[va] - forward[va];

        LineTo(hdc, (int) temp[ua], (int) temp[va]);
    }

    SelectObject(hdc,oldpen);
    DeleteObject(pen);
}


void SetBrush::renderConnections()
{
    Entity *e;
    vec3_t	dest, origin;
    vec3_t	mid;
    vec3_t	forward, right;
    char	*targname;
    vec3_t	min, max, temp;

    char *targ = parent->target;
    if (!targ || !*targ)
        return;

    origin[0] = (bmins[0] + bmaxs[0])/2.0f;
    origin[1] = (bmins[1] + bmaxs[1])/2.0f;
    origin[2] = (bmins[2] + bmaxs[2])/2.0f;

    // save...
    pixel32_t oldfc = r_flatcolor;
    pixel32_t oldec = r_entitycolor;

    pixel32_t cColor;
    cColor.rgb = set.color_connection;
    cColor.p = GetPaletteIndex(256,set.pal, set.color_connection);

    winding_t *w = NewWinding(2);
    if(!w) return;

    int oldoffset = t_coloroffset;
    bool oldent = r_drawent;

    r_drawent = true;
    t_coloroffset = 31*256;
    r_flatcolor = cColor;
    r_entitycolor = cColor;

    glShadingOff();

    glColor3ubv((GLubyte*) &cColor.rgb);

    map *m = map_i[set.curmap];
    for (e = m->objects.p_next; e != &m->objects; e = e->p_next)
    {
        targname = e->targetname;
        if (strcmp (targ, targname))
            continue;

        if (e->objects.p_next == &e->objects)
            continue;

        e->objects.p_next->getMins(min,max);

        dest[0] = (min[0] + max[0])/2.0f;
        dest[1] = (min[1] + max[1])/2.0f;
        dest[2] = (min[2] + max[2])/2.0f;

        VectorCopy(origin,w->points[0]);
        VectorCopy(dest,w->points[1]);
        if (!set.glBsp)
        {
            REN_DrawLineWinding(w,0);
        }
        else
        {
            glBegin(GL_LINES);
            glVertex3fv(w->points[0]);
            glVertex3fv(w->points[1]);
            glEnd();
        }

        forward[0] = dest[0] - origin[0];
        forward[1] = dest[1] - origin[1];
        forward[2] = dest[2] - origin[2];

        if (!forward[0] && !forward[1] && !forward[2])
            continue;

        VectorNormalize (forward);
        forward[0] = 8*forward[0];
        forward[1] = 8*forward[1];

        //forward[2] = 8*forward[2];
        right[0] = forward[1];
        right[1] = -forward[0];
        right[2] = forward[2];

        mid[0] = (dest[0] + origin[0])/2;
        mid[1] = (dest[1] + origin[1])/2;
        mid[2] = (dest[2] + origin[2])/2;

        temp[0] = mid[0] + right[0] - forward[0];
        temp[1] = mid[1] + right[1] - forward[1];
        temp[2] = mid[2] + right[2] - forward[2];

        VectorCopy(temp,w->points[0]);
        VectorCopy(mid,w->points[1]);
        if (!set.glBsp)
        {
            REN_DrawLineWinding(w,0);
        }
        else
        {
            glBegin(GL_LINES);
            glVertex3fv(w->points[0]);
            glVertex3fv(w->points[1]);
            glEnd();
        }

        temp[0] = mid[0] - right[0] - forward[0];
        temp[1] = mid[1] - right[1] - forward[1];
        temp[2] = mid[2] - right[2] - forward[2];

        VectorCopy(mid,w->points[0]);
        VectorCopy(temp,w->points[1]);
        if (!set.glBsp)
        {
            REN_DrawLineWinding(w,0);
        }
        else
        {
            glBegin(GL_LINES);
            glVertex3fv(w->points[0]);
            glVertex3fv(w->points[1]);
            glEnd();
        }
    }

    glShadingOn();

    delete [] w;
    r_flatcolor = oldfc;
    r_entitycolor = oldec;
    t_coloroffset = oldoffset;
    r_drawent = oldent;
}

bool SetBrush::getClippedBrush(SetBrush**major, SetBrush**minor)
{
    *major=*minor=0;

    if (!IsSelected() || fakebrush)
        return false;
    if (!checkModifiable())
        return false;

    //get a new face from current clipper
    face_t	face;
    memset(&face,0,sizeof(face_t));
    if (!(map_i[set.curmap]->clipper_i->getFace(&face)))
        return false;

    //============================================
    //first, get the remaining portion...
    //============================================

    // copy the brush ...
    SetBrush* copybr = copy();
    Entity *saveParent = copybr->parent;
    copybr->parent = NULL;	//todo: why must this be null before adding a face?
    // add a face...
    addFace(copybr,&face);
    copybr->parent = saveParent;
    // if it's valid, then draw it...
    if (!copybr->IsInvalid())
    {
        *major = copybr;
    }
    else
    {
        delete copybr;
    }

    //============================================
    //next, get the clipped portion
    //============================================

    if (set.clipper_show_outline)
    {
        copybr = copy();
        saveParent = copybr->parent;
        copybr->parent = 0;

        vec3_t	temp;
        VectorCopy (face.planepts[0], temp);
        VectorCopy (face.planepts[2], face.planepts[0]);
        VectorCopy (temp, face.planepts[2]);

        addFace(copybr,&face);

        if (copybr)
        {
            copybr->parent = saveParent;

            if (!copybr->IsInvalid())
            {
                *minor = copybr;
            }
            else
            {
                delete copybr;
            }
        }
    }
    return *major || *minor;
}
// md is notBlack and
// cf is viewType double usage
bool SetBrush::fakeBrush(HDC hdc,int mode, float *vp, int /*md*/, int cf, bool /*showCur*/)
{
    if (!IsSelected() || fakebrush)
        return false;

    if (!checkModifiable())
        return false;

    //get a new face from current clipper
    face_t	face;
    memset(&face,0,sizeof(face_t));
    if (!(map_i[set.curmap]->clipper_i->getFace(&face)))
        return false;

    fakebrush = true;

    //============================================
    //first, draw the remaining portion...
    //============================================

    // copy the brush ...
    SetBrush* copybr = copy();
    Entity *saveParent = copybr->parent;
    copybr->parent = NULL;	//todo: why must this be null before adding a face?
    // add a face...
    addFace(copybr,&face);
    copybr->parent = saveParent;
    // if it's valid, then draw it...
    if (!copybr->IsInvalid())
    {
        switch (mode)
        {

        case XYDRAW:
            copybr->XYDrawSelf(hdc,vp,cf);
            break;
        case XYDRAWCUR:
            copybr->XYDrawSelfCur(hdc,vp,cf);
            break;

        case CLOSESTRENDER:
            copybr->ClosestRenderSelf();
            break;
        case CAMERARENDER:
            copybr->CameraRenderSelf();
            break;
        case GLCAMERARENDERSELFTEXTURE:
            copybr->glCameraRenderSelfTexture();
            break;
        case GLCAMERARENDERSELFFIXED:
            copybr->glCameraRenderSelfFixed();
            break;
        case GLCAMERARENDERSELFOUTLINE:
            copybr->glCameraRenderSelfOutline();
            break;
        case GLCAMERARENDERSELFSELECTED:
            copybr->glCameraRenderSelfSelected();
            break;
        case GLCAMERARENDERSELFWIREFRAME:
            copybr->glCameraRenderSelfWireframe();
            break;
        case GLCAMERARENDERSELFFLAT:
            copybr->glCameraRenderSelfFlat();
            break;
        }
    }
    delete copybr;

    //============================================
    //next, draw the clipped portion
    //============================================

    if (set.clipper_show_outline)
    {
        copybr = copy();
        saveParent = copybr->parent;
        copybr->parent = NULL;

        face_t fb;
        vec3_t	temp;

        fb = face;
        VectorCopy (fb.planepts[0], temp);
        VectorCopy (fb.planepts[2], fb.planepts[0]);
        VectorCopy (temp, fb.planepts[2]);

        addFace(copybr,&fb);
        if (copybr)
        {
            copybr->parent = saveParent;
            if (!copybr->IsInvalid())
            {
                clipDraw = true;
                switch (mode)
                {
                case XYDRAW:
                    copybr->XYDrawSelf(hdc,vp,cf);
                    break;
                case XYDRAWCUR:
                    copybr->XYDrawSelfCur(hdc,vp,cf);
                    break;
                case CAMERARENDER:
                {
                    bool olddraw = r_drawflat;
                    bool oldwire = r_drawwire;
                    r_drawflat = false;
                    r_drawwire = true;
                    copybr->CameraRenderSelf();
                    r_drawflat = olddraw;
                    r_drawwire = oldwire;
                }
                break;
                case GLCAMERARENDERSELFTEXTURE:
                case GLCAMERARENDERSELFFIXED:
                case GLCAMERARENDERSELFOUTLINE:
                case GLCAMERARENDERSELFSELECTED:
                case GLCAMERARENDERSELFNOTEXTURE:
                    copybr->glCameraRenderSelfWireframe();
                    break;
                }
                clipDraw = false;
            }
            delete copybr;
        }
    }

    fakebrush = false;
    return true;
}

/* Special Camera Draw:  For Dialog Box Previews pass in the Q and Clip/Min Max values, don't
	check if brush is clipped. */
void SetBrush::CameraDrawSelfSpecial(HDC hdc, float p, COLORREF linecolor, float Q[][4], float EZMin, float EZMax,
                                     LPRECT EClip, LPSIZE ESize)
{
    int counter;
    int npts;

    winding_t *w;

    HPEN linepen = CreatePen(PS_SOLID,1,linecolor);
    HGDIOBJ oldpen = SelectObject(hdc, linepen);

    for (face_t *f = faces.p_next; f != &faces; f = f->p_next)
    {
        LoopProblem("brush cdsp");
        w = f->w;
        if (!w || !(w->numpoints))
            continue;

        Bsp::cur = 0;
        Bsp::lst = 1;

        npts = w->numpoints;

        for (counter = 0; counter<npts; counter++)
        {
            ::transform2(w->points[counter][0],w->points[counter][1],w->points[counter][2], Q, CP[Bsp::cur][counter]);
//			::transform(&TempPt, Q, &CP[cur][counter]);
        }

        npts = Z_Clip(npts,EZMin,EZMax);

        if (npts > 2)    // at least 3 points in front...
        {
            Bsp::cur ^= 1;
            Bsp::lst ^= 1;

            // now loop through and project to screen coords...
            for (counter = 0; counter < npts; counter++)
            {
                float *tp = CP[Bsp::cur][counter];

                tpts[counter][0] =	EClip->left+ (ESize->cx/2-((tp[0]*p)/tp[2]));
                tpts[counter][1] =	EClip->top + (ESize->cy - (ESize->cy/2-((tp[1]*p)/tp[2])));
            }

            npts = FastClipXY(npts,EClip);

            if (npts > 2)   // at least one polygon to draw
            {
                Polyline(hdc,FSPoints,npts);
                MoveToEx(hdc,FSPoints[npts-1].x, FSPoints[npts-1].y, 0);   // close the path...
                LineTo(hdc,FSPoints[0].x, FSPoints[0].y);
            }
        }
    }
    SelectObject(hdc, oldpen);
    DeleteObject(linepen);
}

/* DrawCenterKnob()
*/
//todo, floats could be reduced here... could use setpixelv.. clip pts outside of view.....
void SetBrush::DrawCenterKnob(HDC hdc, int whichXY, LPSIZE XYSize, float u, float v)
{
    int UAxis = xyWindow[whichXY]->UAxis;
    int VAxis = xyWindow[whichXY]->VAxis;

    vec3_t mid;
    mid[0] = ((bctr[UAxis]) - u)*set.scale + (XYSize->cx/2 + 0.5f);
    mid[1] = XYSize->cy - (((bctr[VAxis]) - v)*set.scale + (XYSize->cy/2 + 0.5f));

    const float sc = set.center_knobs_scale;
    if (set.scale >= 0.5f)
    {
        const float sc2 = sc * 2.0f;
        const float sc3 = sc * 3.0f;
        if (set.center_knobs_use_x)
        {
            MoveToEx(hdc, (int) mid[0] - sc2, (int) mid[1] - sc2, 0);
            LineTo(hdc,   (int) mid[0] + sc3, (int) mid[1] + sc3   );
            MoveToEx(hdc, (int) mid[0] - sc2, (int) mid[1] + sc2, 0);
            LineTo(hdc,   (int) mid[0] + sc3, (int) mid[1] - sc3   );
        }
        else
        {
            Rectangle(hdc, (int) mid[0]-sc2, (int) mid[1]-sc2, (int) mid[0]+sc2, (int) mid[1]+sc2);
        }
    }
    else
    {
        if (set.center_knobs_use_x)
        {
            const float sc2 = sc * 2.0f;
            MoveToEx(hdc, (int) mid[0] - sc,  (int) mid[1] - sc, 0);
            LineTo(hdc,   (int) mid[0] + sc2, (int) mid[1] + sc2  );
            MoveToEx(hdc, (int) mid[0] - sc,  (int) mid[1] + sc, 0);
            LineTo(hdc,   (int) mid[0] + sc2, (int) mid[1] - sc2  );
        }
        else
        {
            Rectangle(hdc, (int) mid[0]-sc, (int) mid[1]-sc, (int) mid[0]+sc, (int) mid[1]+sc);
        }
    }
}

/* DrawCenterAngle(floag ang,xxx)
*/

void SetBrush::DrawCenterAngle(HDC hdc, float ang, LPSIZE XYSize, float u, float v)
{
    vec3_t	mid, end, s1, s2, end2;
    float useScale;
    useScale = set.scale;
    mid[0] = (-u+(bmins[0]+bmaxs[0])/2)*set.scale+XYSize->cx/2;
    mid[1] = (-v+(bmins[1]+bmaxs[1])/2)*set.scale+XYSize->cy/2;

    mid[0] = mid[0] - 8*cos(ang)*useScale;
    mid[1] = mid[1] - 8*sin(ang)*useScale;

    end[0] = mid[0] + 20*cos(ang)*useScale;
    end[1] = mid[1] + 20*sin(ang)*useScale;

    end2[0] = mid[0] + 16*cos(ang)*useScale;
    end2[1] = mid[1] + 16*sin(ang)*useScale;

    s1[0] = mid[0] + 12*cos(ang+0.2f)*useScale;
    s1[1] = mid[1] + 12*sin(ang+0.2f)*useScale;

    s2[0] = mid[0] + 12*cos(ang-0.2f)*useScale;
    s2[1] = mid[1] + 12*sin(ang-0.2f)*useScale;

    MoveToEx(hdc,(int) mid[0],(int) XYSize->cy-mid[1],0);
    LineTo(hdc,(int) end2[0],(int) XYSize->cy-end2[1]);
    LineTo(hdc,(int) s1[0],(int) XYSize->cy-s1[1]);
    LineTo(hdc,(int) end[0],(int) XYSize->cy-end[1]);
    LineTo(hdc,(int) s2[0],(int) XYSize->cy-s2[1]);
    LineTo(hdc,(int) end2[0],(int) XYSize->cy-end2[1]);
}

// TBD
void SetBrush::DrawOrigin(HDC hdc, int whichXY, LPSIZE XYSize, float u, float v)
{
    if (!IsOriginBrush())
        return;


    int UAxis = xyWindow[whichXY]->UAxis;
    int VAxis = xyWindow[whichXY]->VAxis;

    vec3_t mid;
    mid[0] = (-u+(bmins[UAxis]+bmaxs[UAxis])/2)*set.scale+XYSize->cx/2;
    mid[1] = XYSize->cy - ((-v+(bmins[VAxis]+bmaxs[VAxis])/2)*set.scale+XYSize->cy/2);

    const float gap = 5.0f;
    SetPixelV(hdc,(int) mid[0] - gap*set.scale,(int) mid[1] - 2*set.scale,RGB(255,0,0));
}

// TBD
void SetBrush::DrawDetail(HDC hdc, int whichXY, LPSIZE XYSize, float u, float v)
{
    if (!IsDetailBrush())
        return;

    int UAxis, VAxis;
    vec3_t mid;

    UAxis = 	xyWindow[whichXY]->UAxis;
    VAxis = 	xyWindow[whichXY]->VAxis;

    mid[0] = (-u+bctr[UAxis])*set.scale+XYSize->cx/2;
    mid[1] = XYSize->cy - ((-v+bctr[VAxis])*set.scale+XYSize->cy/2);

    float gap = 5.0f;
    float sc = set.scale;
    SetPixelV(hdc,(int) mid[0] - gap*sc,(int) mid[1] + 2*sc,RGB(0,255,0));
}

//  For non Gl versions, sets up the HPEN //linepen.
//
HPEN SetBrush::GetBrushPen(Entity *worldent, Entity *currentent)
{
    if (clipDraw)
    {
        return CreatePen(PS_DOT,1,set.color_otherbrush);
    }
    else
    {
        COLORREF color = 0;
        int width = 1;
        if (parent != worldent && worldent == currentent && !IsSelected())
        {
            color = entitycolor;
        }
        else if (IsSelected())
        {
            width = set.selection_thickness;
            if (IsLocked())
            {
                color = set.color_lock;
            }
            else
            {
                color = set.color_selection;
            }
        }
        else if (parent == currentent)
        {
            if (!set.group_mode)
            {
                color = set.color_foreground;
            }
            else
            {
                color = map_i[set.curmap]->groups->GetGroupColor(group);
            }
        }
        else
        {
            color = set.color_otherbrush;
        }
        return CreatePen(PS_SOLID, width, color);
    }
}
// sr sg sb
void SetBrush::glGetBrushColor(Entity *worldent, Entity *currentent, unsigned char *sR, unsigned char *sG, unsigned char *sB)
{
    if (clipDraw)
    {
        *sR = GetRValue(set.color_otherbrush);
        *sG = GetGValue(set.color_otherbrush);
        *sB = GetBValue(set.color_otherbrush);
    }
    else
    {
        if (parent != worldent && worldent == currentent && !IsSelected())
        {
            *sR = GetRValue(entitycolor);
            *sG = GetGValue(entitycolor);
            *sB = GetBValue(entitycolor);
        }
        else if (IsSelected())
        {
            glLineWidth((GLfloat) set.selection_thickness);
            if (IsLocked())
            {
                *sR = GetRValue(set.color_lock);
                *sG = GetGValue(set.color_lock);
                *sB = GetBValue(set.color_lock);
            }
            else
            {
                *sR = GetRValue(set.color_selection);
                *sG = GetGValue(set.color_selection);
                *sB = GetBValue(set.color_selection);
            }
        }
        else if (parent == currentent)
        {
            if (!set.group_mode)
            {
                *sR = GetRValue(set.color_foreground);
                *sG = GetGValue(set.color_foreground);
                *sB = GetBValue(set.color_foreground);
            }
            else
            {
                COLORREF groupcolor = map_i[set.curmap]->groups->GetGroupColor(group);
                *sR = GetRValue(groupcolor);
                *sG = GetGValue(groupcolor);
                *sB = GetBValue(groupcolor);
            }
        }
        else
        {
            *sR = GetRValue(set.color_otherbrush);
            *sG = GetGValue(set.color_otherbrush);
            *sB = GetBValue(set.color_otherbrush);
        }
    }
}

void SetBrush::GetUV(int ViewType, float *vp, float *u, float *v)
{
    switch (ViewType)
    {
    case Type_XY:
        *u = vp[0];
        *v = vp[1];
        break;
    case Type_YZ:
        *u = vp[1];
        *v = vp[2];
        break;
    case Type_XZ:
        *u = vp[0];
        *v = vp[2];
        break;
    };
};
/*
===========
XYDrawSelf (not ShowCur and not Black...)
===========
*/
void SetBrush::XYDrawSelf(HDC hdc, float *vp, int whichXY)
{
    int ViewType;
    RECT XYClip;
    SIZE XYSize;

    ViewType = xyWindow[whichXY]->ViewType;
    CopyRect(&XYClip,&xyWindow[whichXY]->XYClip);
    XYSize.cx = xyWindow[whichXY]->XYSize.cx;
    XYSize.cy = xyWindow[whichXY]->XYSize.cy;

    int ua = xyWindow[whichXY]->UAxis;
    int va = xyWindow[whichXY]->VAxis;
//	int na = xyWindow[whichXY]->NormalAxis;

    int npts, counter;

    winding_t	*w;
    char	*val;
    float	ang;
    Entity *worldent, *currentent;

    if (fakeBrush(hdc,XYDRAW,vp,0,whichXY,false))
        return;

    map *m = map_i[set.curmap];

    worldent = m->world;
    currentent = m->currentEntity();

    // xxx Move to Procedure and only check that parent is not world
    // Need to mark "detail" brushes and "origin" brushes somehow
    bool keybrush = (parent != worldent && (this == parent->objects.p_next));

    HPEN pen = GetBrushPen(worldent, currentent);
    HGDIOBJ oldpen = SelectObject(hdc,pen);

    // Draw any target lines...
    if (keybrush && set.show_connections && !clipDraw)
        drawConnections(hdc,whichXY);	// target line

    Bsp::cur = 0;
    Bsp::lst = 1;

    float u, v;
    GetUV(ViewType, vp, &u, &v);

    for (face_t *f = faces.p_next; f != &faces; f = f->p_next)
    {
        LoopProblem("brush xyds");

        w = f->w;
        if (!w)
            continue;

        // if this is the selected brush, show all of the faces...
        if (DotProduct (f->plane.normal,xyWindow[whichXY]->xy_viewnormal) > -VECTOR_EPSILON)
            continue;

        for (counter = 0; counter < w->numpoints; counter++)
        {
            tpts[counter][0] = (float) ((w->points[counter][ua]-u)*set.scale+XYSize.cx/2 + 0.5);
            tpts[counter][1] = (float) XYSize.cy - (0.5+(w->points[counter][va]-v)*set.scale+XYSize.cy/2);
        }
        npts = FastClipXY(w->numpoints,&XYClip);
        if (npts >= 2)
        {
            Polyline(hdc,FSPoints,npts);
            MoveToEx(hdc,FSPoints[npts-1].x, FSPoints[npts-1].y, 0);
            LineTo(hdc,FSPoints[0].x, FSPoints[0].y);
        }
    }

    // now deal with the current face...
    bool hadAng = false;
    if (!clipDraw && keybrush && (ViewType == Type_XY)) // only good from top down view...
    {
        // angle arrow
        val = parent->angle;
        if (val && val[0])
        {
            ang = DEG2RAD(atof(val));
            if (ang >= 0)	// negative values are up/down flags
            {
                hadAng = true;
                DrawCenterAngle(hdc, ang, &XYSize, u, v);
            }
            // if ang is -1 or -2 deal with up and down
            //
        }
    }

    if (!clipDraw && /*!keybrush &&*/ set.center_knobs_xy && !hadAng)
        DrawCenterKnob(hdc, whichXY, &XYSize, u, v);

    DrawDetail(hdc, whichXY, &XYSize, u, v);

    SelectObject(hdc,oldpen);
    DeleteObject(pen);
}

void SetBrush::XYDrawSelfCur(HDC hdc, float *vp, int whichXY)
{
    RECT XYClip;
    SIZE XYSize;

    int ViewType = xyWindow[whichXY]->ViewType;
    CopyRect(&XYClip, &xyWindow[whichXY]->XYClip);
    XYSize.cx = xyWindow[whichXY]->XYSize.cx;
    XYSize.cy = xyWindow[whichXY]->XYSize.cy;

    int ua = xyWindow[whichXY]->UAxis;
    int va = xyWindow[whichXY]->VAxis;
    //	int na = xyWindow[whichXY]->NormalAxis;

    int npts, counter;

    face_t *f;
    winding_t	*w;
    char	*val;
    float	ang;
    Entity *worldent, *currentent;
    bool	keybrush = false;
    //unsigned char sR, sG, sB;

    if(fakeBrush(hdc,XYDRAWCUR,vp,0,whichXY,false))
        return;
    map *m = map_i[set.curmap];
    worldent = m->world;
    currentent = m->currentEntity();

    if (parent != worldent && (this == parent->objects.p_next))
        keybrush = true;

//debug:
//	HPEN blah = CreatePen(PS_SOLID,1,RGB(0,0,255));
    HPEN pen = GetBrushPen(worldent,currentent);
    HGDIOBJ oldpen = SelectObject(hdc, pen);

    if (keybrush && set.show_connections && !clipDraw)
        drawConnections(hdc,whichXY);	// target line

    Bsp::cur = 0;
    Bsp::lst = 1;

    float u, v;
    GetUV(ViewType, vp, &u, &v);

    for (f = faces.p_next; f != &faces; f = f->p_next)
    {
        LoopProblem("brush xydscur");
        if (f == currentFace && IsSelected() && !clipDraw)
            continue;

        w = f->w;
        if (!w)
            continue;

        /*
        // DEBUG: this draws the planes in XY
        		//BEGIN TEST
        		SelectObject(hdc,blah);
        		for (counter = 0; counter < 3; counter++) {
        			tpts[counter][0] = (int)((f->planepts[counter][ua]-u)*set.scale+XYSize.cx/2 + 0.5);
        			tpts[counter][1] = XYSize.cy - (int)(0.5+(f->planepts[counter][va]-v)*set.scale+XYSize.cy/2);
        		}
        		npts = FastClipXY(3,&XYClip);
        	//	if (npts >= 2) {
        				Polyline(hdc,FSPoints,npts);
        				MoveToEx(hdc,FSPoints[npts-1].x, FSPoints[npts-1].y, 0);
        				LineTo(hdc,FSPoints[0].x, FSPoints[0].y);
        	//	}
        //END TEST
        	*/
        SelectObject(hdc,pen);

        for (counter = 0; counter < w->numpoints; counter++)
        {
            tpts[counter][0] = (float) ((w->points[counter][ua]-u)*set.scale+XYSize.cx/2 + 0.5);
            tpts[counter][1] = (float) XYSize.cy - (0.5+(w->points[counter][va]-v)*set.scale+XYSize.cy/2);
        }
        npts = FastClipXY(w->numpoints,&XYClip);
        if (npts >= 2)
        {
            Polyline(hdc,FSPoints,npts);
            MoveToEx(hdc,FSPoints[npts-1].x, FSPoints[npts-1].y, 0);
            LineTo(hdc,FSPoints[0].x, FSPoints[0].y);
        }
    }
//debug:
//DeleteObject(blah);

    // now deal with the current face...
    f = currentFace;
    if (f && parent->modifiable && !clipDraw && IsSelected() && ((w = f->w) != NULL))
    {

        HPEN selpen = CreatePen(PS_SOLID,1,set.color_curface);
        SelectObject(hdc, selpen);
        for (counter = 0; counter < w->numpoints; counter++)
        {
            tpts[counter][0] = (float) ((w->points[counter][ua]-u)*set.scale+XYSize.cx/2 + 0.5);
            tpts[counter][1] = (float) XYSize.cy - (0.5+(w->points[counter][va]-v)*set.scale+XYSize.cy/2);
        }
        npts = FastClipXY(w->numpoints,&XYClip);
        if (npts >= 2)
        {
            MoveToEx(hdc, FSPoints[npts-1].x, FSPoints[npts-1].y, 0);

            HBRUSH brush = CreateSolidBrush(set.color_background);
            HGDIOBJ oldbrush = SelectObject(hdc, brush);
            Ellipse(hdc, FSPoints[npts-1].x-2, FSPoints[npts-1].y-2, FSPoints[npts-1].x+2, FSPoints[npts-1].y+2);

            for (counter = 0; counter < npts; counter++)
            {
                LineTo(hdc, FSPoints[counter].x, FSPoints[counter].y);
                Ellipse(hdc, FSPoints[counter].x-2,FSPoints[counter].y-2,
                        FSPoints[counter].x+2,FSPoints[counter].y+2);
            }
            SelectObject(hdc,oldbrush);
            DeleteObject(brush);
        }
        SelectObject(hdc,pen);
        DeleteObject(selpen);
    }


    bool hadAng = false;
    if (!clipDraw && keybrush && (ViewType == Type_XY)) // only good from top down view...
    {
        // angle arrow
        val = parent->angle;
        if (val && *val)
        {
            ang = DEG2RAD(atof(val));
            if (ang >= 0)	// negative values are up/down flags
            {
                hadAng = true;
                DrawCenterAngle(hdc, ang, &XYSize, u, v);
            }
        }
    }

    //DrawOrigin(hdc, whichXY, &XYSize, u, v);
    DrawDetail(hdc, whichXY, &XYSize, u, v);

    if (!clipDraw && /*!keybrush &&*/ set.center_knobs_xy && !hadAng)
        DrawCenterKnob(hdc, whichXY, &XYSize, u, v);

    SelectObject(hdc,oldpen);
    DeleteObject(pen);
}

bool SetBrush::pointAlreadyProcessed(vec3_t pt, face_t *max_face)
{
    if (!max_face)
        return false;

    float dot;
    face_t *f;
    winding_t *w;
    for (f = faces.p_next; (f != &faces) && (f != max_face); f = f->p_next)
    {
        LoopProblem("brush point processed");
        w = f->w;
        if (!w)
            continue;

        dot = DotProduct(pt, f->plane.normal);
        if (fabs(dot - f->plane.dist) <= ON_EPSILON)
            return true;
    }
    return false;
}

// Draws the edge handles...
bool show_edge_knobs = true;
void SetBrush::CameraRenderEdges()
{
    if (!set.center_knobs_3d)
        return;

    vec3_t ctr;
    for (face_t *f = faces.p_next; f != &faces; f = f->p_next)
    {
        LoopProblem("brush cre");
        winding_t *w = f->w;
        if (!w)
            continue;

        int k = w->numpoints - 1;
        for (int j = 0; j < w->numpoints; j++)
        {

            ctr[0] = (w->points[j][0] + w->points[k][0])/2.0f;
            ctr[1] = (w->points[j][1] + w->points[k][1])/2.0f;
            ctr[2] = (w->points[j][2] + w->points[k][2])/2.0f;

            if (!pointAlreadyProcessed(ctr, f))
            {
                if (set.sinBsp)
                {
                    REN_DrawCameraHandleSin(ctr, 0, 255, 255);
                }
                else
                {
                    REN_DrawCameraHandle(ctr, RGB(0, 255, 255));
                }
            }
            k = j;
        }
    }
}

void SetBrush::glCameraRenderEdges()
{
    if (!set.center_knobs_3d)
        return;

    glShadingOff();

    glPointSize(4.0f);
    glColor3ub(0,255,255);

    vec3_t ctr;
    for (face_t *f = faces.p_next; f != &faces; f = f->p_next)
    {
        LoopProblem("brush cre");
        winding_t *w = f->w;
        if (!w)
            continue;

        int k = w->numpoints - 1;
        for (int j = 0; j < w->numpoints; j++)
        {

            ctr[0] = (w->points[j][0] + w->points[k][0])/2.0f;
            ctr[1] = (w->points[j][1] + w->points[k][1])/2.0f;
            ctr[2] = (w->points[j][2] + w->points[k][2])/2.0f;

            if (!pointAlreadyProcessed(ctr, f))
            {
                glBegin(GL_POINTS);
                glVertex3fv((GLfloat*) &ctr);
                glEnd();
            }
            k = j;
        }
    }
    glShadingOn();

    glPointSize(1.0f);
}
/*
===========
CameraRenderSelf
===========
*/

void SetBrush::CameraRenderSelf()
{
    map *m = map_i[set.curmap];

    if (fakeBrush(NULL,CAMERARENDER,m->eye[m->cureye],0,0,false))
        return;

    face_t *f;

    Entity *worldent = m->world;

    bool keybrush = false;
    if (parent != worldent && (this == parent->objects.p_next))
        keybrush = true;

    bool olddraw;
    bool oldwire;
    // hack to draw entity boxes as single flat color
    if (!parent->modifiable || clipDraw)
    {
        olddraw = r_drawflat;
        oldwire = r_drawwire;
        if (set.wire_ents)
        {
            r_drawwire = true;
        }
        r_drawflat = true;
        r_drawent = true;

        COLORREF color = entitycolor;
        if (clipDraw)
        {
            color = set.color_otherbrush;
        }
        if (texWindow && set.pal)
        {
            if (set.sinBsp)
            {
                r_entitycolor.rgb = color;
            }
            r_entitycolor.p = GetPaletteIndex(256,set.pal, color);
        }
        else
        {
            if (set.sinBsp)
            {
                r_entitycolor.rgb = RGB(0,255,255);
            }
            r_entitycolor.p = 255;
        }

        EntityClass *ec = parent->ec;
        if (!clipDraw && set.draw_models && ec && (ec->mdl || ec->mdlQ2))
        {
            vec3_t mins, maxs, org;
            parent->objects.p_next->getMins(mins,maxs);
            VectorSubtract(mins, ec->mins, org);

            int ang = 0;
            char *val = parent->angle;
            if (val && *val)
                ang = 360 - atoi(val);

            if (set.texture_models)
            {
                int skinNum = 0;
                if (ec->mdl)
                {
                    if (ec->mdl->numskins > 1)
                        skinNum = ec->curskin;

                    REN_RenderModel(ec->mdl, IsSelected(), 0, 0, ang, (float) ec->curframe, (int) org[0], (int) org[1], org[2], skinNum);
                }
                else
                {
                    if (ec->mdlQ2->num_skins > 1)
                        skinNum = ec->curskin;

                    REN_RenderModelQ2(ec->mdlQ2, ec->skinTextureData, IsSelected(), 0, 0, ang, (float) ec->curframe, (int) org[0], (int) org[1], org[2], skinNum);
                }
            }
            else
            {
                if (ec->mdl)
                    REN_DrawModel(ec->mdl, IsSelected(), 0, 0, ang, (float) ec->curframe, (int) org[0], (int) org[1], org[2]);
                else
                    REN_DrawModelQ2(ec->mdlQ2, IsSelected(), 0, 0, ang, (float) ec->curframe, (int) org[0], (int) org[1], org[2]);
            }
            if (set.animate_models)
            {
                ec->curframe++;
                if (ec->curframe >= ec->maxframe)
                    ec->curframe = 0;
            }
        }
        else
        {
            for (f = faces.p_next; f != &faces; f = f->p_next)
            {
                LoopProblem("brush crs");
                if (clipDraw)
                {
                    REN_DrawCameraFace (f,0);
                }
                else
                {

                    if (!parent->modifiable)
                        REN_DrawCameraFace (f,IsSelected());
                    else if (this == REN_curBrush && (f == currentFace))
                        REN_DrawCameraFace (f,2);  // yellow
                    else if (IsLocked() && IsSelected())
                        REN_DrawCameraFace (f,3);
                    else
                        REN_DrawCameraFace (f,IsSelected());
                }
            }
        }

        r_drawflat = olddraw;
        r_drawwire = oldwire;
        r_drawent = false;
    }
    else
    {
        for (f = faces.p_next; f != &faces; f = f->p_next)
        {
            LoopProblem("brush crs2");
            if (this == REN_curBrush && (f == currentFace))		// currentface?
                REN_DrawCameraFace (f,2);						// yellow
            else if (IsLocked() && IsSelected())				// selected?
                REN_DrawCameraFace (f,3);						// red
            else												//
                REN_DrawCameraFace (f,IsSelected());			//
        }
    }

    if (keybrush && set.render_connections)
        renderConnections();	// target line

    if (!clipDraw && IsSelected() && parent->modifiable && show_edge_knobs)
        CameraRenderEdges();
}

void SetBrush::glCameraRenderSkybox()
{
    map *m = map_i[set.curmap];
    if (fakeBrush(NULL,GLCAMERARENDERSELFTEXTURE,m->eye[m->cureye],0,0,false))
        return;

    bool keybrush = false;
    if (parent != m->world && (this == parent->objects.p_next))
        keybrush = true;

    for (face_t *f = faces.p_next; f != &faces; f = f->p_next)
    {
        if(!f->sky)
            continue;
        winding_t *w = f->w;
        if (!w)
            continue;

        glBegin(GL_TRIANGLE_FAN);
        glNormal3fv(f->plane.normal);
        for (int j = 0; j < w->numpoints; j++)
        {
            glVertex3fv(w->points[j]);
        }
        glEnd();
    }

    if (keybrush && set.render_connections)
    {
        renderConnections();	// target line
    }
}

void SetBrush::glCameraRenderSelfTexture()
{
    map *m = map_i[set.curmap];
    if (fakeBrush(NULL,GLCAMERARENDERSELFTEXTURE,m->eye[m->cureye],0,0,false))
        return;

    bool keybrush = false;
    if (parent != m->world && (this == parent->objects.p_next))
        keybrush = true;

    /*
    //DEBUG - draw planes
    //WORKHERE
    //BEGIN TEST
    glShadingOff();
    glDisable(GL_TEXTURE_2D);
    glColor3ub(0,255,0);
    for (face_t *f = faces.p_next; f != &faces; f = f->p_next) {

    	glBegin(GL_LINE_LOOP);
    		glVertex3fv(f->planepts[0]);
    		glVertex3fv(f->planepts[1]);
    		glVertex3fv(f->planepts[2]);
    	glEnd();
    }
    glEnable(GL_TEXTURE_2D);
    glShadingOn();
    //END TEST
    */

    glColor3ub(255,255,255);
    for (face_t *f = faces.p_next; f != &faces; f = f->p_next)
    {
        if (!f->qtexture)
            f->qtexture = texWindow->GetQTexture(f->texture.texture);

        qtexture_t *q = f->qtexture;
        if (!q)
            continue;

        winding_t *w = f->w;
        if (!w)
            continue;

        if(set.gl_skybox && set.gl_skynames_loaded && f->sky)
        {
            glSkyboxNotEmpty = true;
            continue;
        }
        else if (q->bindIndex != glLastBoundIndex)
        {
            glBindTexture(GL_TEXTURE_2D, q->bindIndex);
            glLastBoundIndex = q->bindIndex;
        }

        glBegin(GL_TRIANGLE_FAN);
        glNormal3fv(f->plane.normal);
        for (int j = 0; j < w->numpoints; j++)
        {
            float u = w->points[j][3];
            float v = w->points[j][4];
            glTexCoord2f(u/q->width, v/q->height);
            glVertex3fv(w->points[j]);
        }
        glEnd();
    }

    if (keybrush && set.render_connections)
    {
        renderConnections();	// target line
    }
}

void SetBrush::glCameraRenderSelfTextureTrans(bool solids)
{
    map *m = map_i[set.curmap];
    if (fakeBrush(NULL,GLCAMERARENDERSELFTEXTURE,m->eye[m->cureye],0,0,false))
        return;

    bool keybrush = false;
    if (parent != m->world && (this == parent->objects.p_next))
        keybrush = true;

    glColor4ub(255,255,255,set.gl_alpha_level);
    for (face_t *f = faces.p_next; f != &faces; f = f->p_next)
    {
        if (!f->qtexture)
            f->qtexture = texWindow->GetQTexture(f->texture.texture);

        qtexture_t *q = f->qtexture;
        if (!q)
            continue;

        if(set.gl_skybox && set.gl_skynames_loaded && f->sky)
        {
            glSkyboxNotEmpty = true;
            continue;
        }

        if(solids == (bool)f->transparent)
            continue;

        winding_t *w = f->w;
        if (!w)
            continue;

        if (q->bindIndex != glLastBoundIndex)
        {
            glBindTexture(GL_TEXTURE_2D, q->bindIndex);
            glLastBoundIndex = q->bindIndex;
        }

        glBegin(GL_TRIANGLE_FAN);
        glNormal3fv(f->plane.normal);
        for (int j = 0; j < w->numpoints; j++)
        {
            float u = w->points[j][3];
            float v = w->points[j][4];
            glTexCoord2f(u/q->width, v/q->height);
            glVertex3fv(w->points[j]);
        }
        glEnd();
    }

    if (keybrush && set.render_connections)
    {
        renderConnections();	// target line
    }
}
//TODO - gl model stuff
extern void glRenderModel(mdl_t *mdl, int selected,
                          float Xrot, float Yrot, float Zrot, int frame, int Xoffs, int Yoffs, float Zoffs, int skinNumber) ;

extern void glRenderModelQ2(dmdl_t *mdl, dskindata_t *skinTex, int selected,
                            float Xrot, float Yrot, float Zrot, int frame, int Xoffs, int Yoffs, float Zoffs, int skinNumber) ;

extern
void glCreateQ1ModelList(mdl_t *mdl);
extern
void glCreateQ2ModelList(dmdl_t *mdl, dskindata_t *skinTex);

//draw entities
void SetBrush::glCameraRenderSelfFixed()
{
    map *m = map_i[set.curmap];
    if (fakeBrush(NULL,GLCAMERARENDERSELFFIXED,m->eye[m->cureye],0,0,false))
        return;

    bool keybrush = false;
    if (parent != m->world && (this == parent->objects.p_next))
        keybrush = true;

    if(set.draw_models && parent->ec && parent->ec->mdl)
    {

        glColor3ub(255,255,255);

        //model angle
        int ang = 0;
        char *val = parent->angle;
        if (val && *val)
            ang = atoi(val);

        //pos
        vec3_t mins, maxs, org;
        parent->objects.p_next->getMins(mins,maxs);
        VectorSubtract(mins, parent->ec->mins, org);

        glEnable(GL_TEXTURE_2D);

        //	glRenderModel(parent->ec->mdl,IsSelected(),0,0, ang, parent->ec->curframe,org[0],org[1],org[2],0);
        if(!parent->ec->mdl->gl_list)
            glCreateQ1ModelList(parent->ec->mdl);

        glPushMatrix();
        glTranslatef(org[0],org[1],org[2]);
        glRotatef((GLfloat) ang, 0, 0, 1);
        glCallList(parent->ec->mdl->gl_list);
        glPopMatrix();

        glDisable(GL_TEXTURE_2D);

    }
    else if (set.draw_models && parent->ec && parent->ec->mdlQ2)
    {

        glColor3ub(255,255,255);

        //model angle
        int ang = 0;
        char *val = parent->angle;
        if (val && *val)
            ang = atoi(val);

        //pos
        vec3_t mins, maxs, org;
        parent->objects.p_next->getMins(mins,maxs);
        VectorSubtract(mins, parent->ec->mins, org);


        //	glShadeModel(GL_FLAT);
        glEnable(GL_TEXTURE_2D);

        if(!parent->ec->mdlQ2->gl_list)
            glCreateQ2ModelList(parent->ec->mdlQ2,parent->ec->skinTextureData);
//		glRenderModelQ2(parent->ec->mdlQ2,parent->ec->skinTextureData, IsSelected(),0,0, ang, parent->ec->curframe,org[0],org[1],org[2],0);

        glPushMatrix();
        glTranslatef(org[0],org[1],org[2]);
        glRotatef((GLfloat) ang, 0, 0, 1);
        glCallList(parent->ec->mdlQ2->gl_list);
        glPopMatrix();

        glDisable(GL_TEXTURE_2D);
        //	glShadeModel(GL_SMOOTH);
    }
    else
    {

        //glColor4ub(255,255,255,set.gl_alpha_level);
        //	glColor3ubv((GLubyte*) &entitycolor);
        ///	glColor4ub(GetRValue(entitycolor),GetGValue(entitycolor),GetBValue(entitycolor),set.gl_alpha_level);
        glColor4ub(GetRValue(entitycolor),GetGValue(entitycolor),GetBValue(entitycolor),set.gl_alpha_level);
        // draw ent boxes...
        for (face_t *f = faces.p_next; f != &faces; f = f->p_next)
        {

            winding_t *w = f->w;
            if (!w)
                continue;

            glBegin(GL_TRIANGLE_FAN);
            glNormal3fv(f->plane.normal);
            glVertex3fv(w->points[0]);
            glVertex3fv(w->points[1]);
            glVertex3fv(w->points[2]);
            glVertex3fv(w->points[3]);
            glEnd();
        }
    }

    if (keybrush && set.render_connections)
    {
        renderConnections();	// target line
    }
}

// render outlines on brushes and entities
void SetBrush::glCameraRenderSelfOutline()
{
    map *m = map_i[set.curmap];
    if (fakeBrush(NULL,GLCAMERARENDERSELFOUTLINE,m->eye[m->cureye],0,0,false))
        return;

    //get outline color
    if(!parent->modifiable)
    {

        // hide bounding box on mdls?
        if(!set.gl_model_bbox && parent->ec && (parent->ec->mdl || parent->ec->mdlQ2))
            return;

        if(!set.texture_models)
        {
            //wireframe ents use entity color
            glColor3ubv((GLubyte*) &parent->ec->color);
        }
        else if(set.outline_use_group)
        {
            //use group color
            COLORREF color = m->groups->GetGroupColor(group);
            glColor3ubv((GLubyte*) &color);
        }
        else
        {
            //use regular outline color
            glColor3ubv((GLubyte*) &set.color_brushoutline);
        }
    }
    else if(!set.outline_use_group)
    {
        //use regular outline color
        glColor3ubv((GLubyte*) &set.color_brushoutline);
    }
    else
    {
        //use group outline
        COLORREF color = m->groups->GetGroupColor(group);
        glColor3ubv((GLubyte*) &color);
    }
    glShadingOff();

    for (face_t *f = faces.p_next; f != &faces; f = f->p_next)
    {

        winding_t *w = f->w;
        if (!w)
            continue;

        glBegin(GL_LINE_LOOP);
        for (int j = 0; j < w->numpoints; j++)
        {
            glVertex3fv(w->points[j]);
        }
        glEnd();
    }
    glShadingOn();
}

//draw selected brush outlines (red) and selected face (yellow)
void SetBrush::glCameraRenderSelfSelected()
{
    map *m = map_i[set.curmap];

    if (fakeBrush(NULL,GLCAMERARENDERSELFSELECTED,m->eye[m->cureye],0,0,false))
        return;

    winding_t *w;
    face_t *f;

    glShadingOff();

    face_t *selface = 0;

    //set edge color
    glColor3ubv((GLubyte*) &set.color_selectoutline);

    //draw selected edges
    for (f = faces.p_next; f != &faces; f = f->p_next)
    {

        w = f->w;
        if (!w)
            continue;

        if ((f == currentFace) && (this == REN_curBrush))
        {
            selface = f;
        }
        else
        {
            glBegin(GL_LINE_LOOP);

            for (int j = 0; j < w->numpoints; j++)
            {
                glVertex3fv(w->points[j]);
            }
            glEnd();
        }
    }

    //draw selected face last for regular wiremodes. current face drawn in main render function for non culled wireframes
    if(set.gl_selection_wiremode == 0)
    {
        if(selface && parent->modifiable)
        {
            w = selface->w;
            if(w)
            {
                glColor3ubv((GLubyte*) &set.color_faceoutline);
                glBegin(GL_LINE_LOOP);
                for (int j = 0; j < w->numpoints; j++)
                {
                    glVertex3fv(w->points[j]);
                }
                glEnd();
            }
        }
    }

    //render edge knobs
    if (parent->modifiable && show_edge_knobs)
    {
        glCameraRenderEdges();
    }

    //restore shading
    glShadingOn();
}


void SetBrush::glCameraRenderSelfFlat()
{
    map *m = map_i[set.curmap];
    if (fakeBrush(NULL,GLCAMERARENDERSELFFLAT,m->eye[m->cureye],0,0,false))
        return;

    bool keybrush = false;
    if (parent != m->world && (this == parent->objects.p_next))
        keybrush = true;

    for (face_t *f = faces.p_next; f != &faces; f = f->p_next)
    {
        if (!f->qtexture)
            f->qtexture = texWindow->GetQTexture(f->texture.texture);

        qtexture_t *q = f->qtexture;
        if (!q)
            continue;

        winding_t *w = f->w;
        if (!w)
            continue;

        if (!parent->modifiable)
        {
            r_flatcolor.rgb = entitycolor;
        }
        else
        {
            r_flatcolor = q->flatcolor;
        }

        //draw flat poly
        glColor3ubv( (GLubyte*)&r_flatcolor.rgb);
        if (w->numpoints == 3)
        {
            //	glBegin(GL_TRIANGLES);
            glBegin(GL_TRIANGLE_FAN);
            glNormal3fv(f->plane.normal);
            glVertex3fv(w->points[0]);
            glVertex3fv(w->points[1]);
            glVertex3fv(w->points[2]);
            glEnd();
        }
        else if (w->numpoints == 4)
        {
            //	glBegin(GL_QUADS);
            glBegin(GL_TRIANGLE_FAN);
            glNormal3fv(f->plane.normal);
            glVertex3fv(w->points[0]);
            glVertex3fv(w->points[1]);
            glVertex3fv(w->points[2]);
            glVertex3fv(w->points[3]);
            glEnd();
        }
        else
        {
            //	glBegin(GL_POLYGON);
            glBegin(GL_TRIANGLE_FAN);
            glNormal3fv(f->plane.normal);
            for (int j = 0; j < w->numpoints; j++)
            {
                glVertex3fv(w->points[j]);
            }
            glEnd();
        }
    }

    if (keybrush && set.render_connections)
    {
        renderConnections();	// target line
    }

    if (!clipDraw && IsSelected() && parent->modifiable && show_edge_knobs)
        glCameraRenderEdges();
}

void SetBrush::glCameraRenderSelfFlatTrans(bool solids)
{
    map *m = map_i[set.curmap];
    if (fakeBrush(NULL,GLCAMERARENDERSELFFLAT,m->eye[m->cureye],0,0,false))
        return;

    bool keybrush = false;
    if (parent != m->world && (this == parent->objects.p_next))
        keybrush = true;

    for (face_t *f = faces.p_next; f != &faces; f = f->p_next)
    {
        if (!f->qtexture)
            f->qtexture = texWindow->GetQTexture(f->texture.texture);

        qtexture_t *q = f->qtexture;
        if (!q)
            continue;
        if(solids == (bool)f->transparent)
            continue;
        winding_t *w = f->w;
        if (!w)
            continue;

        if (!parent->modifiable)
        {
            r_flatcolor.rgb = entitycolor;
        }
        else
        {
            r_flatcolor = q->flatcolor;
        }

        //draw flat poly
        glColor4ub( r_flatcolor.chan[0], r_flatcolor.chan[1], r_flatcolor.chan[2], set.gl_alpha_level);
        if (w->numpoints == 3)
        {
            glBegin(GL_TRIANGLE_FAN);
            glNormal3fv(f->plane.normal);
            glVertex3fv(w->points[0]);
            glVertex3fv(w->points[1]);
            glVertex3fv(w->points[2]);
            glEnd();
        }
        else if (w->numpoints == 4)
        {
            glBegin(GL_TRIANGLE_FAN);
            glNormal3fv(f->plane.normal);
            glVertex3fv(w->points[0]);
            glVertex3fv(w->points[1]);
            glVertex3fv(w->points[2]);
            glVertex3fv(w->points[3]);
            glEnd();
        }
        else
        {
            glBegin(GL_TRIANGLE_FAN);
            glNormal3fv(f->plane.normal);
            for (int j = 0; j < w->numpoints; j++)
            {
                glVertex3fv(w->points[j]);
            }
            glEnd();
        }
    }

    if (keybrush && set.render_connections)
    {
        renderConnections();	// target line
    }

    if (!clipDraw && IsSelected() && parent->modifiable && show_edge_knobs)
        glCameraRenderEdges();
}

void SetBrush::glCameraRenderSelfWireframe()
{
    map *m = map_i[set.curmap];

    if (fakeBrush(NULL,GLCAMERARENDERSELFWIREFRAME,m->eye[m->cureye],0,0,false))
        return;

    Entity *worldent = m->world;

    bool keybrush = false;
    if (parent != worldent && (this == parent->objects.p_next))
        keybrush = true;

    int j;
    winding_t *w;
    face_t *f;

    glShadingOff();

    float dot;
//	face_t *selface = 0;
    for (f = faces.p_next; f != &faces; f = f->p_next)
    {

        // back face cull
        if (set.cull_wire)
        {
            dot = DotProduct (m->eye[set.curmap], f->plane.normal);
            if (dot <= f->plane.dist)
                continue;
        }

        w = f->w;
        if (!w)
            continue;


        if (clipDraw)
        {
            glColor3ubv((GLubyte*) &set.color_brushoutline);
            glBegin(GL_LINE_LOOP);
            for (j = 0; j < w->numpoints; j++)
            {
                glVertex3fv(w->points[j]);
            }
            glEnd();
        }

        if (!f->qtexture)
            f->qtexture = texWindow->GetQTexture(f->texture.texture);

        qtexture_t *q = f->qtexture;
        if (!q)
            continue;


        if (!parent->modifiable)
        {
            r_flatcolor.rgb = entitycolor;
        }
        else
        {
            if (set.group_mode && set.gl_wire_use_groupcolor)
            {
                r_flatcolor.rgb = m->groups->GetGroupColor(group);
            }
            else
            {
                r_flatcolor = q->flatcolor;
            }
        }

        // if it's not an entity and user wants white outlines...
        if (!set.color_wire && parent->modifiable)
        {
            glColor3ubv((GLubyte*) &set.color_brushoutline);
        }
        else
        {
            glColor3ubv((GLubyte*) &r_flatcolor.rgb);
        }
        glBegin(GL_LINE_LOOP);
        for (j = 0; j < w->numpoints; j++)
        {
            glVertex3fv(w->points[j]);
        }
        glEnd();
    }


    if (keybrush && set.render_connections)
        renderConnections();	// target line

    if (!clipDraw && IsSelected() && parent->modifiable && show_edge_knobs)
        glCameraRenderEdges();

    glShadingOn();
}

/*
==============================================================================

SINGLE BRUSH ACTIONS

==============================================================================
*/

face_t	*dragface, *dragface2;
int		numcontrolpoints;
float	*controlpoints[MAX_FACES*3];

int maxmulticontrolpoints = 0;
int multicontrolpoints = 0;
float **mcontrolpoints = NULL;

void SetBrush::addMultiPoint(float *pt)
{
    if (multicontrolpoints >= maxmulticontrolpoints)
        return;

    mcontrolpoints[multicontrolpoints++] = pt;
}

bool SetBrush::checkModifiable()
{
    if (parent && parent->modifiable)
        return true;

    if (!parent) // assume yes
        return true;
    return false;
}

void SetBrush::getZdragface(vec3_t dragpoint, int ViewType)
{
    float d;
    int n, u, v;

    if (!checkModifiable())
        return;

    GetAxes(ViewType, &u, &v, &n);
    numcontrolpoints = 0;
    face_t *f;
    for (f = faces.p_next; f != &faces; f = f->p_next)
    {
        LoopProblem("brush gzdf");
        if (f->plane.normal[n] == 1)
            d = dragpoint[n] - f->plane.dist;
        else if (f->plane.normal[n] == -1)
            d = -f->plane.dist - dragpoint[n];
        else
            continue;

        if (d <= 0)
            continue;

        currentFace = f; // last one...
        controlpoints[numcontrolpoints++] = f->planepts[0];
        controlpoints[numcontrolpoints++] = f->planepts[1];
        controlpoints[numcontrolpoints++] = f->planepts[2];

    }
}

float SetBrush::checkXYmultidragfaces(vec3_t dragpoint, int ViewType, face_t **fa)
{
    float	d;
    int n, u, v;

    *fa = NULL;
    if (!checkModifiable())
        return 99999.0;

    GetAxes(ViewType, &u, &v, &n);

    float bestD = 99999.0;

    face_t *f;
    for (f = faces.p_next; f != &faces; f = f->p_next)
    {
        LoopProblem("brush check xy multi");
        if (!f->w)
            continue;

        // change back to 0.0?  want it to be zero...
        if (fabs(f->plane.normal[n]) > set.face_drag)
            continue;

        d = DotProduct(f->plane.normal, dragpoint) - f->plane.dist;
        if (d <= 0)
            continue;

        if (d > bestD)
            continue;

        bestD = d;
        *fa = f;
    }

    return bestD;
}

void SetBrush::getXYmultidragfaces(face_t *fa, int strict)
{
    if (!checkModifiable())
        return;

    float dist,d2;

    float p1D, p2D;

    face_t *f;
    for (f = faces.p_next; f != &faces; f = f->p_next)
    {
        LoopProblem("brush getxy multi");
        // compare normals..

        d2 = (f->plane.normal[0] - fa->plane.normal[0]);
        dist = d2*d2;
        d2 = (f->plane.normal[1] - fa->plane.normal[1]);
        dist += d2*d2;
        d2 = (f->plane.normal[2] - fa->plane.normal[2]);
        dist += d2*d2;


        if (dist > set.multi_face_normal_eps)   // epsilon...
            continue;
        // change back to 0.0?  want it to be zero...

        if (strict)
        {
            p1D = f->plane.dist;
            p2D = fa->plane.dist;

            if (fabs(p1D - p2D) > set.multi_face_distance_eps)
                continue;
        }
        currentFace = f; // will end up at last one...

        addMultiPoint(f->planepts[0]);
        addMultiPoint(f->planepts[1]);
        addMultiPoint(f->planepts[2]);

        break; // only one face per brush...
    }
}

void SetBrush::getXYdragface(vec3_t dragpoint, int ViewType)
{
    float d;

    numcontrolpoints = 0;

    if (!checkModifiable())
        return;

    int u,v,n;
    GetAxes(ViewType, &u, &v, &n);

    face_t *f;
    for (f = faces.p_next; f != &faces; f = f->p_next)
    {
        LoopProblem("brush xydrag");
        // change back to 0.0?  want it to be zero...
        if (fabs(f->plane.normal[n]) > set.face_drag)
            continue;

        d = DotProduct(f->plane.normal, dragpoint) - f->plane.dist;
        if (d <= 0)
            continue;

        currentFace = f; // will end up at last one...

        controlpoints[numcontrolpoints++] = f->planepts[0];
        controlpoints[numcontrolpoints++] = f->planepts[1];
        controlpoints[numcontrolpoints++] = f->planepts[2];
    }
}

bool SetBrush::vertexClicked(vec3_t dragpoint, int ViewType)
{
    int i;
    float d;

    bestface = NULL;
    float real_delta;
    float pixelWidth;
    pixelWidth = 1.0f/set.scale;  // each pixel is this many world units...
    real_delta = set.select_delta * pixelWidth; // how many units to allow...

    if(!currentFace)
        return false;
    if (!checkModifiable())
        return false;

    int u,v,n;
    GetAxes(ViewType, &u, &v, &n);

    // find the three planes that intersect at the drag point...
    face_t	  *f = currentFace;
    winding_t *w = f->w;

    if (!w)
        return false;

    bestd = 99999.0;

    for (i = 0; i < w->numpoints; i++)
    {
        d = ((w->points[i][u]-dragpoint[u])*(w->points[i][u]-dragpoint[u])+
             (w->points[i][v]-dragpoint[v])*(w->points[i][v]-dragpoint[v]));
        d = sqrt(d);

        // up to 3, or 2 diagonally..
        if (d > real_delta) // don't use
            continue;

        if (d < bestd)
        {
            //	VectorCopy(w->points[i],last_vertex);
            bestd = d;
        }
    }

    // couldn't find a close enough vertex on current face, now check the
    // other faces...

    if (bestd >= 99999.0)
    {
        float bestNormal = -1000.0;
        // loop through other faces...
        bestd = 99999.0;
        for (f = faces.p_next; f != &faces; f = f->p_next)
        {
            LoopProblem("brush vc");
            if (f == currentFace) // skip it...
                continue;

            w = f->w;
            if (!w)
                continue;

            for (i = 0; i < w->numpoints; i++)
            {
                d = ((w->points[i][u]-dragpoint[u])*(w->points[i][u]-dragpoint[u])+
                     (w->points[i][v]-dragpoint[v])*(w->points[i][v]-dragpoint[v]));
                d = sqrt(d);

                // up to 3, or 2 diagonally..
                if (d > real_delta) // don't use
                    continue;

                if (d <= bestd)
                {
                    if (f->plane.normal[n] > bestNormal)
                    {
                        //	VectorCopy(w->points[i],last_vertex);
                        bestNormal = f->plane.normal[n];
                        bestd = d;
                        bestface = f;
                    }
                }
            }
        }
    }

    // couldn't find a close enough vertex...
    if ((bestd >= 99999.0) && (bestface == NULL))
        return false;

    return true;
}

void SetBrush::getXYdragvertex(vec3_t dragpoint, int ViewType)
{
    int		i,j, k;
    int		facectl;
    float	d;
    int		numdragplanes;
    bool	dragplane[MAX_FACES];
    winding_t *w;
    face_t	*f;
    int onplane[MAX_POINTS_ON_WINDING];
    vec3_t vertex;
    vec3_t delta;

    int u,v,n;

    float real_delta;
    float pixelWidth;
    pixelWidth = 1.0f/set.scale;  // each pixel is this many world units...
    real_delta = set.select_delta * pixelWidth; // how many units to allow...

    numcontrolpoints = 0;
    numdragplanes = 0;

    if (!checkModifiable())
        return;

    GetAxes(ViewType, &u, &v, &n);

    // find the three planes that intersect at the drag point...
    f = currentFace;
    w = f->w;

    if (!w)
        return;

    bestd = 99999.0;

    for (i = 0; i < w->numpoints; i++)
    {
        d = ((w->points[i][u]-dragpoint[u])*(w->points[i][u]-dragpoint[u])+
             (w->points[i][v]-dragpoint[v])*(w->points[i][v]-dragpoint[v]));
        d = sqrt(d);

        // up to 3, or 2 diagonally..
        if (d > real_delta) // don't use
            continue;

        if (d < bestd)
        {
            bestd = d;
            // okay, found a suitable vertex...
            VectorCopy(w->points[i],vertex);
        }
    }

    // couldn't find a close enough vertex on current face, now check the
    // other faces...
    bestface = NULL;
    if (bestd >= 99999.0)
    {
        float bestNormal = -1000.0;
        // loop through other faces...
        bestd = 99999.0;
        for (f = faces.p_next; f != &faces; f = f->p_next)
        {
            LoopProblem("brush xydragvertex");
            if (f == currentFace) // skip it...
                continue;

            w = f->w;
            if (!w)
                continue;

            for (i = 0; i < w->numpoints; i++)
            {
                d = ((w->points[i][u]-dragpoint[u])*(w->points[i][u]-dragpoint[u])+
                     (w->points[i][v]-dragpoint[v])*(w->points[i][v]-dragpoint[v]));
                d = sqrt(d);

                // up to 3, or 2 diagonally..
                if (d > real_delta) // don't use
                    continue;

                if (d <= bestd)
                {
                    if (f->plane.normal[n] > bestNormal)
                    {
                        bestNormal = f->plane.normal[n];
                        bestd = d;
                        bestface = f;
                        VectorCopy(w->points[i],vertex);
                    }
                }
            }
        }
        if (bestface)
            currentFace = bestface; // reset current face...
    }

    // couldn't find a close enough vertex...
    if (bestd >= 99999.0 && bestface == NULL)	//yes, &&
        return;

    // now find all faces that share the "vertex"
    i = 0;
    for (f = faces.p_next; f != &faces; f = f->p_next, i++)
    {
        LoopProblem("brush xydv2");
        dragplane[i] = false;
        onplane[i] = -1;

        w = f->w;
        if (!w)
            continue;

        for (j=0 ; j<w->numpoints ; j++)
        {
            delta[0] = w->points[j][0] - vertex[0];
            delta[1] = w->points[j][1] - vertex[1];
            delta[2] = w->points[j][2] - vertex[2];

            d = VectorLength(delta);

            if (d > ON_EPSILON) // don't use
                continue;

            dragplane[i] = true;

            onplane[i] = j; // store which point...

            numdragplanes++;
            break;
        }
    }

    // need to have at least three planes...
    if (numdragplanes < 3)
        return;

    // find faces that just share an edge with a drag plane
    i = 0;
    for (f = faces.p_next; f != &faces; f = f->p_next, i++)
    {
        LoopProblem("brush xydv3");
        w = f->w;
        if (!w)
            continue;
        if (!dragplane[i]) // skip if not a valid plane...
            continue;

        facectl = 0;
        for (j=0 ; j < w->numpoints ; j++)
        {
            if (onplane[i] != j) // skip to proper "control vertex" if any...
                continue;

            bestd = 99999.0;
            for (k = 0; k < 3; k++)
            {
                VectorSubtract(w->points[j],f->planepts[k],delta);

                d = VectorLength(delta);

                if (d < bestd)
                {
                    bestd = d;
                    facectl = k;
                }
            }

            int prev = j-1;
            int prevctl = facectl - 1;

            if (prev < 0)
                prev = w->numpoints-1;
            if (prevctl < 0)
                prevctl = 2;

            if (set.vertex_select_mode == 1)
            {
                // distance the next two points by 4*set.gridsize units or so...
                VectorSubtract(w->points[prev],w->points[j],delta);
                VectorNormalize(delta);
                // that's a unit vector.
                VectorScale(delta,(float) set.vertex_drag_sensitivity*set.gridsize,delta);
                VectorAdd(w->points[j],delta,w->points[prev]);
                // first
            }
            VectorCopy (w->points[prev],f->planepts[prevctl]);

            // second, main control point...
            VectorCopy (w->points[j], f->planepts[facectl]); // insert it into the "plane points..."
            controlpoints[numcontrolpoints++] = f->planepts[facectl];

            if (set.vertex_select_mode == 1)
            {
                VectorSubtract(w->points[(j+1)%w->numpoints],w->points[j],delta);
                VectorNormalize(delta);
                // that's a unit vector.
                VectorScale(delta,(float) set.vertex_drag_sensitivity*set.gridsize,delta);
                VectorAdd(w->points[j],delta,w->points[(j+1)%w->numpoints]);
                // third
            };
            VectorCopy (w->points[(j+1)%w->numpoints],f->planepts[(facectl+1)%3]);
            break;
        }

        for (j=0 ; j<3 ; j++)
        {
            f->planepts[j][0] = (float) rint(f->planepts[j][0]);
            f->planepts[j][1] = (float) rint(f->planepts[j][1]);
            f->planepts[j][2] = (float) rint(f->planepts[j][2]);
        }
    }
}

bool SetBrush::checkEdgeDragFaces(vec3_t p1, vec3_t p2)
{
    int i,j,j2;
    winding_t	*w;
    face_t		*f;

    if (!checkModifiable())
        return false;

    for (f = faces.p_next; f != &faces; f = f->p_next)
    {
        LoopProblem("brush check edge");
        w = f->w;
        if (!w)
            continue;

        j = w->numpoints - 1;
        for (i = 0; i < w->numpoints; i++)
        {
            if (!VectorIsClose(p1,w->points[i]))
                continue;

            // okay, p1 is a match for point i
            // check against point previous and next points...
            if (VectorIsClose(p2,w->points[j]))
                return true;

            j2 = i + 1;
            if (j2 >= w->numpoints)
                j2 = 0;

            if (VectorIsClose(p2,w->points[j2]))
                return true;

            j = i;
        }
    }
    return false;
}

void SetBrush::getEdgeDragFaces(vec3_t v1, vec3_t v2)
{
    int j,k;
    winding_t	*w;
    face_t		*f;

    if (!checkModifiable())
        return;

    int numFF = 0;
    int p1;
    vec3_t delta;

    for (f = faces.p_next; (f != &faces) && (numFF < 2); f = f->p_next)
    {
        LoopProblem("brush get edge");
        w = f->w;
        if (!w)
            continue;

        p1 = w->numpoints - 1;
        for (k = 0; k < w->numpoints; k++)
        {
            // check v1 and v2 against winding p1 and winding k
            if ((VectorIsClose(v1,w->points[k]) && VectorIsClose(v2,w->points[p1])) ||
                    (VectorIsClose(v1,w->points[p1]) && VectorIsClose(v2,w->points[k])))
            {
                // set up the control points...
                // abort as this "face" is done...
                // p1 then k then p1-1 first..
                // planepoints for this face are k, and p1 and the "next face"
                // controlponits are k and p1...move them both...
                int facectl = 0;
                int prev = p1 - 1;

                if (prev < 0)
                    prev = w->numpoints-1;

                if (set.vertex_select_mode == 1)
                {
                    // distance the next two points by 4*set.gridsize units or so...
                    VectorSubtract(w->points[p1],w->points[prev],delta);
                    VectorNormalize(delta);
                    // that's a unit vector.
                    VectorScale(delta,128.0,delta);
                    VectorAdd(w->points[p1],delta,w->points[prev]);
                }
                VectorCopy (w->points[prev],f->planepts[facectl++]);

                // second, add two edge control points for this face...
                VectorCopy (w->points[p1], f->planepts[facectl]); // insert it into the "plane points..."
                addMultiPoint(f->planepts[facectl++]);

                VectorCopy (w->points[k], f->planepts[facectl]); // insert it into the "plane points..."
                addMultiPoint(f->planepts[facectl]);

                // round close ones...
                for (j=0 ; j<3 ; j++)
                {
                    f->planepts[j][0] = (float) rint(f->planepts[j][0]);
                    f->planepts[j][1] = (float) rint(f->planepts[j][1]);
                    f->planepts[j][2] = (float) rint(f->planepts[j][2]);
                }

                numFF++;
                break;
            }
            p1 = k;
        }
    }
}

bool SetBrush::checkXYEdgeDragFaces(vec3_t dragpoint, int ViewType)
{
    float pixelWidth = 1.0f/set.scale;  // each pixel is this many world units...
    float real_delta = set.select_delta * pixelWidth; // how many units to allow...

    if (!checkModifiable())
        return false;

    int u,v,n;
    GetAxes(ViewType, &u, &v, &n);

    float d;

    for (face_t *f = faces.p_next; f != &faces; f = f->p_next)
    {
        LoopProblem("brush check xy edge");
        int numvertices = 0;
        winding_t *w = f->w;
        if (!w)
            continue;

        for (int i = 0; i < w->numpoints; i++)
        {
            d = ((w->points[i][u]-dragpoint[u])*(w->points[i][u]-dragpoint[u])+
                 (w->points[i][v]-dragpoint[v])*(w->points[i][v]-dragpoint[v]));

            d = sqrt(d);

            // up to 3, or 2 diagonally..
            if (d > real_delta) // don't use
                continue;

            numvertices++;
            if (numvertices >= 2)
                return true;
        }
    }
    return false;
}

void SetBrush::getXYEdgeDragFaces(vec3_t dragpoint, int ViewType)
{
    int i,j,k;
    float	d;
    winding_t	*w;
    face_t		*f;
    int u,v,n;
    vec3_t delta;

    float pixelWidth = 1.0f/set.scale;  // each pixel is this many world units...
    float real_delta = set.select_delta * pixelWidth; // how many units to allow...

    if (!checkModifiable())
        return;

    GetAxes(ViewType, &u, &v, &n);

    int numvertices = 0;
    vec3_t v1, v2;

    for (f = faces.p_next; f != &faces; f = f->p_next)
    {
        LoopProblem("brush get xy edge");
        w = f->w;
        if (!w)
            continue;

        numvertices = 0;
        for (i = 0; i < w->numpoints; i++)
        {
            d = ((w->points[i][u]-dragpoint[u])*(w->points[i][u]-dragpoint[u])+
                 (w->points[i][v]-dragpoint[v])*(w->points[i][v]-dragpoint[v]));

            d = sqrt(d);

            // up to 3, or 2 diagonally..
            if (d > real_delta) // don't use
                continue;

            if (numvertices == 0)
            {
                VectorCopy(w->points[i],v1);
            }
            else if (numvertices == 1)
            {
                VectorCopy(w->points[i],v2);
            }
            numvertices++;
            if (numvertices >= 2)
                break;
        }
        if (numvertices == 2)
            break;
    }

    // just in case.
    if (numvertices != 2)
        return;

    // now cycle through faces until we find both faces that "share" the
    // two points...
    // one face is k

    int numFF = 0;
    int p1;
    for (f = faces.p_next; (f != &faces) && (numFF < 2); f = f->p_next)
    {
        LoopProblem("brush get xy 2");
        w = f->w;
        if (!w)
            continue;

        p1 = w->numpoints - 1;
        for (k = 0; k < w->numpoints; k++)
        {
            // check v1 and v2 against winding p1 and winding k
            if ((VectorIsClose(v1,w->points[k]) && VectorIsClose(v2,w->points[p1])) ||
                    (VectorIsClose(v1,w->points[p1]) && VectorIsClose(v2,w->points[k])))
            {
                // set up the control points...
                // abort as this "face" is done...
                // p1 then k then p1-1 first..
                // planepoints for this face are k, and p1 and the "next face"
                // controlponits are k and p1...move them both...

                int facectl = 0;
                int prev = p1 - 1;

                if (prev < 0)
                    prev = w->numpoints-1;

                if (set.vertex_select_mode == 1)
                {
                    // distance the next two points by 4*set.gridsize units or so...
                    VectorSubtract(w->points[p1],w->points[prev],delta);
                    VectorNormalize(delta);
                    // that's a unit vector.
                    VectorScale(delta,128.0,delta);
                    VectorAdd(w->points[p1],delta,w->points[prev]);
                }
                VectorCopy (w->points[prev],f->planepts[facectl++]);

                // second, add two edge control points for this face...
                VectorCopy (w->points[p1], f->planepts[facectl]); // insert it into the "plane points..."
                addMultiPoint(f->planepts[facectl++]);

                VectorCopy (w->points[k], f->planepts[facectl]); // insert it into the "plane points..."
                addMultiPoint(f->planepts[facectl]);

                // round close ones...
                for (j=0 ; j<3 ; j++)
                {
                    f->planepts[j][0] = (float) rint(f->planepts[j][0]);
                    f->planepts[j][1] = (float) rint(f->planepts[j][1]);
                    f->planepts[j][2] = (float) rint(f->planepts[j][2]);
                }

                numFF++;
                break;
            }
            p1 = k;
        }
    }
}

void SetBrush::getXYShearPoints(vec3_t dragpoint, int /*ViewType*/)
{
    int		i,j, k;
    int		facectl;
    float	d;
    int		numdragplanes;
    bool	dragplane[MAX_FACES];
    winding_t	*w;
    face_t		*f;
    bool	onplane[MAX_POINTS_ON_WINDING];

    if (!checkModifiable())
        return;

    numcontrolpoints = 0;
    numdragplanes = 0;
    i = 0;
    for (f = faces.p_next; f != &faces; f = f->p_next, i++)
    {
        LoopProblem("brush get xy shear");
        dragplane[i] = false;
        if (!f->w)
            continue;
//		if (faces[i].plane.normal[2])
//			continue;

        d = DotProduct(f->plane.normal, dragpoint) - f->plane.dist;
        if (d <= -ON_EPSILON)
            continue;

        dragplane[i] = true;
        numdragplanes++;
    }

// find faces that just share an edge with a drag plane
    i = 0;
    for (f = faces.p_next; f != &faces; f = f->p_next, i++)
    {
        LoopProblem("brush get xy shear 2");
        w = f->w;
        if (!w)
            continue;

        if (dragplane[i] && numdragplanes == 1)
        {
            controlpoints[numcontrolpoints++] = f->planepts[0];
            controlpoints[numcontrolpoints++] = f->planepts[1];
            controlpoints[numcontrolpoints++] = f->planepts[2];

            continue;
        }
        if (!dragplane[i] && numdragplanes > 1)
            continue;

        facectl = 0;
        for (j=0 ; j<w->numpoints ; j++)
        {
            onplane[j] = false;
            k = 0;
            for (face_t *fa = faces.p_next; fa != &faces; fa = fa->p_next, k++)
            {
                LoopProblem("brush get xy shear 3");
                if (!dragplane[k])
                    continue;
                if (fa == f)
                    continue;
                d = DotProduct (w->points[j], fa->plane.normal)
                    - fa->plane.dist;
                if (fabs(d) > ON_EPSILON)
                    continue;
                onplane[j] = true;
                facectl++;
                break;
            }
        }
        if (facectl == 0)
            continue;

        // find one or two static points to go with the controlpoints
        // and change the plane points
        k = 0;
        for (j=0 ; j<w->numpoints ; j++)
        {
            if (!onplane[j])
                continue;
            if (facectl >= 2 && !onplane[(j+1)%w->numpoints])
                continue;
            if (facectl == 3 && !onplane[(j+2)%w->numpoints])
                continue;

            VectorCopy (w->points[j], f->planepts[k]);
            controlpoints[numcontrolpoints++] = f->planepts[k];
            k++;

            if (facectl >= 2)
            {
                VectorCopy (w->points[(j+1)%w->numpoints], f->planepts[k]);
                controlpoints[numcontrolpoints++] = f->planepts[k++];
            }
            if (facectl == 3)
            {
                VectorCopy (w->points[(j+2)%w->numpoints], f->planepts[k]);
                controlpoints[numcontrolpoints++] = f->planepts[k++];
            }
            break;
        }

        for ( ; j<w->numpoints && k != 3 ; j++)
            if (!onplane[j])
            {
                VectorCopy (w->points[j], f->planepts[k]);
                k++;
            }

        for (j=0 ; j<w->numpoints && k != 3 ; j++)
            if (!onplane[j])
            {
                VectorCopy (w->points[j], f->planepts[k]);
                k++;
            }

        if (k != 3)
        {
//			syserror ("getXYShearPoints: didn't get three points on plane");
            numcontrolpoints = 0;
            return;
        }

        for (j=0 ; j<3 ; j++)
        {
            f->planepts[j][0] = (float) rint(f->planepts[j][0]);
            f->planepts[j][1] = (float) rint(f->planepts[j][1]);
            f->planepts[j][2] = (float) rint(f->planepts[j][2]);
        }
    }
}

void SetBrush::getShearPoints(face_t *face)
{
    int		i,j, k;
    int		facectl;
    float	d;
    int		numdragplanes;
    bool	dragplane[MAX_FACES];
    winding_t	*w;
    face_t		*f;
    bool	onplane[MAX_POINTS_ON_WINDING];

    if (!checkModifiable())
        return;

    numcontrolpoints = 0;
    i = 0;
    for (f = faces.p_next; f != &faces; f = f->p_next, i++)
    {
        dragplane[i] = false;
        if (face == f)
            dragplane[i] = true;
    }

    numdragplanes = 1;

// find faces that just share an edge with a drag plane
    i = 0;
    for (f = faces.p_next; f != &faces; f = f->p_next, i++)
    {
        LoopProblem("brush gs");
        w = f->w;
        if (!w)
            continue;
        if (dragplane[i] && numdragplanes == 1)
        {
            controlpoints[numcontrolpoints++] = f->planepts[0];
            controlpoints[numcontrolpoints++] = f->planepts[1];
            controlpoints[numcontrolpoints++] = f->planepts[2];

            continue;
        }
        if (!dragplane[i] && numdragplanes > 1)
            continue;

        facectl = 0;
        for (j=0 ; j<w->numpoints ; j++)
        {
            onplane[j] = false;
            k = 0;
            face_t *fa;
            for (fa = faces.p_next; fa != &faces; fa = fa->p_next, k++)
            {
                LoopProblem("brush gs2");
                if (!dragplane[k])
                    continue;
                if (fa == f)
                    continue;
                d = DotProduct (w->points[j], fa->plane.normal)
                    - fa->plane.dist;
                if (fabs(d) > ON_EPSILON)
                    continue;
                onplane[j] = true;
                facectl++;
                break;
            }
        }
        if (facectl == 0)
            continue;

        // find one or two static points to go with the controlpoints
        // and change the plane points
        k = 0;
        for (j=0 ; j<w->numpoints ; j++)
        {
            if (!onplane[j])
                continue;
            if (facectl >= 2 && !onplane[(j+1)%w->numpoints])
                continue;
            if (facectl == 3 && !onplane[(j+2)%w->numpoints])
                continue;

            VectorCopy (w->points[j], f->planepts[k]);
            controlpoints[numcontrolpoints++] = f->planepts[k++];

            if (facectl >= 2)
            {
                VectorCopy (w->points[(j+1)%w->numpoints], f->planepts[k]);
                controlpoints[numcontrolpoints++] = f->planepts[k++];
            }
            if (facectl == 3)
            {
                VectorCopy (w->points[(j+2)%w->numpoints], f->planepts[k]);
                controlpoints[numcontrolpoints++] = f->planepts[k++];
            }
            break;
        }

        for ( ; j<w->numpoints && k != 3 ; j++)
            if (!onplane[j])
            {
                VectorCopy (w->points[j], f->planepts[k]);
                k++;
            }

        for (j=0 ; j<w->numpoints && k != 3 ; j++)
            if (!onplane[j])
            {
                VectorCopy (w->points[j], f->planepts[k]);
                k++;
            }

        if (k != 3)
        {
//			syserror ("getXYShearPoints: didn't get three points on plane");
            numcontrolpoints = 0;
            return;
        }

        for (j=0 ; j<3 ; j++)
        {
            f->planepts[j][0] = (float) rint(f->planepts[j][0]);
            f->planepts[j][1] = (float) rint(f->planepts[j][1]);
            f->planepts[j][2] = (float) rint(f->planepts[j][2]);
        }
    }
}

/*
==============================================================================

MULTIPLE BRUSH ACTIONS

==============================================================================
*/

/*
===========
newRegion
Set the regioned flag based on if the object is containted in region_min/max
===========
*/

// called when view omitting changes...
void SetBrush::setFiltered()
{
    if (CalcFiltered())
        flags |= B_FILTERED;
    else
        flags &= ~B_FILTERED;

    setDrawable();
}

bool SetBrush::CalcFiltered()
{
    char *name;

    // check for entities?
    map *m = map_i[set.curmap];
    if (parent != m->world)
    {
        if (set.filter_entities)
            return true;

        name = parent->classname;

        if ( (set.filter_light && !strnicmp(name,"light",5) )
                || (set.filter_path  && !strnicmp(name,"path",4) ) )
            return true;

        name = parent->target;

        if (set.filter_target && name && name[0])
            return true;

        name = parent->targetname;

        if (set.filter_target && name && name[0])
            return true;

    }
    else if (set.filter_world)
    {
        return true;
    }

    if (set.filter_clip_brushes &&
            (!strnicmp(faces.p_next->texture.texture, "clip", 4) ||
             faces.p_next->texture.contents & (CONTENTS_PLAYERCLIP | CONTENTS_MONSTERCLIP)))
        return true;

    if (set.filter_water_brushes &&
            ((faces.p_next->texture.texture[0] == '*') ||
             (faces.p_next->texture.contents & (CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER))))
        return true;

    if (set.filter_sky &&
            (!strnicmp(faces.p_next->texture.texture,"sky",3) ||
             (faces.p_next->texture.flags & SURF_SKY)))
        return true;

    if (set.filter_detail && (set.game_mode == 2) &&
            (faces.p_next->texture.contents & CONTENTS_DETAIL))
        return true;

    if (set.filter_hint && (set.game_mode == 2) &&
            (faces.p_next->texture.flags & SURF_HINT))
        return true;

    if (set.filter_unselected && !IsSelected())
        return true;

    return false;
}


void SetBrush::newRegion()
{
    vec3_t region_min, region_max;
    map *m = map_i[set.curmap];
    if (!m->regionBrush)
        return;

    m->regionBrush->getMins(region_min, region_max);

    // filter away entities
    if (parent != m->world)
    {
        if (set.filter_entities)
        {
            setRegioned(true);
            return;
        }

        char *name = parent->classname;

        if ( (set.filter_light && !strncmp(name,"light",5) ) || (set.filter_path && !strncmp(name,"path",4) ) )
        {
            setRegioned(true);
            return;
        }
    }
    else if (set.filter_world)
    {
        setRegioned(true);
        return;
    }

    if (set.filter_clip_brushes && !strnicmp(faces.p_next->texture.texture, "clip",4))
    {
        setRegioned(true);
        return;
    }

    if (set.filter_water_brushes && faces.p_next->texture.texture[0] == '*')
    {
        setRegioned(true);
        return;
    }

    for (int i=0 ; i<3 ; i++)
    {
        if (region_min[i] >= bmaxs[i] || region_max[i] <= bmins[i])
        {
            if (IsSelected())
                deselect();
            setRegioned(true);
            return;
        }
    }
    setRegioned(false);
}

void SetBrush::setRegioned()
{
    vec3_t region_min, region_max;

    map *m = map_i[set.curmap];
    if (!m->regionBrush)
        return;

    m->regionBrush->getMins(region_min, region_max);

    for (int i=0 ; i<3 ; i++)
    {
        if (region_min[i] >= bmins[i] || region_max[i] <= bmaxs[i])
        {
            setRegioned(true);
            if (IsSelected())
                deselect();
            return;
        }
    }
    setRegioned(false);
}

void SetBrush::setRegionedPartial()
{
    vec3_t region_min, region_max;

    map *m = map_i[set.curmap];
    if (!m->regionBrush)
        return;

    m->regionBrush->getMins(region_min, region_max);

    for (int i=0 ; i<3 ; i++)
    {
        if (region_min[i] >= bmaxs[i] || region_max[i] <= bmins[i])
        {
            setRegioned(true);
            if (IsSelected())
                deselect();
            return;
        }
    }
    setRegioned(false);
}

void SetBrush::selectPartial()
{
    for (int i=0 ; i<3 ; i++)
        if (select_min[i] >= bmaxs[i] || select_max[i] <= bmins[i])
        {
            if (select_deselect)
            {
                setSelected(false);
            }
            return;
        }
    setSelected(true);
}

void SetBrush::selectComplete()
{
    for (int i=0 ; i<3 ; i++)
        if (select_min[i] > bmins[i] || select_max[i] < bmaxs[i])
        {
            if (select_deselect)
                setSelected(false);
            return;
        }
    setSelected(true);
}


void SetBrush::regionPartial()
{
    for (int i=0 ; i<3 ; i++)
        if (select_min[i] >= bmaxs[i] || select_max[i] <= bmins[i])
            return;
    setSelected(true);
}

void SetBrush::regionComplete()
{
    for (int i=0 ; i<3 ; i++)
        if (select_min[i] > bmins[i] || select_max[i] < bmaxs[i])
            return;
    setSelected(true);
}

void SetBrush::moveToEntity()
{
    // if could span entities, don't "make entity" on any brushes that themselves are entities...
    if (set.multi_select && !parent->modifiable)
        return;

    parent->removeObject(this);
    parent = sb_newowner;

    EntityClass *eclass = entity_classes_i->classForName(parent->classname);
    parent->ec = eclass;
    // hack to allow them to be copied to another map
    //	if (1) // [parent respondsTo:@selector(valueForQKey:)])
    //	{
    setEntityColor(eclass->drawColor());
    //	};
    parent->addObject(this);
    calcWindings();
}

vec3_t	sb_translate;
void SetBrush::translate()
{
    int		j;
    face_t *f;
    vec3_t xa, ya;
    float s, t;
    float ns, nt;
    // if the brush is texLock
    // for each plane, get the texture axes
    // add sb_translate to the s,t values along the texture axis.
    // clamp the s, t values to the texture width/height
    // then calc windings...

    // move the planes
    if (IsLocked())
    {
        for (f = faces.p_next; f != &faces; f = f->p_next)
        {
            LoopProblem("brush translate");
            TextureAxisFromPlane(&f->plane, xa, ya);

            float ang = DEG2RAD(f->texture.rotate);
            float sinv = sin(ang);
            float cosv = cos(ang);

            if (!f->texture.scale[0])
                f->texture.scale[0] = 1;
            if (!f->texture.scale[1])
                f->texture.scale[1] = 1;

            for (j = 0; j < 3; j++)
            {
                s = xa[j] * sb_translate[j];
                t = ya[j] * sb_translate[j];

                ns = cosv*s - sinv*t;
                nt = sinv*s + cosv*t;

                s = ns/f->texture.scale[0];
                t = nt/f->texture.scale[1];

                f->texture.shift[0] -= s;
                f->texture.shift[1] -= t;
            }

            while (f->texture.shift[0] < -512.0)
                f->texture.shift[0] += 512.0;
            while (f->texture.shift[1] < -512.0)
                f->texture.shift[1] += 512.0;
            while (f->texture.shift[0] > 512.0)
                f->texture.shift[0] -= 512.0;
            while (f->texture.shift[1] > 512.0)
                f->texture.shift[1] -= 512.0;

            VectorAdd (f->planepts[0], sb_translate, f->planepts[0]);
            VectorAdd (f->planepts[1], sb_translate, f->planepts[1]);
            VectorAdd (f->planepts[2], sb_translate, f->planepts[2]);
        }
    }
    else
    {
        for (f = faces.p_next; f != &faces; f = f->p_next)
        {
            VectorAdd (f->planepts[0], sb_translate, f->planepts[0]);
            VectorAdd (f->planepts[1], sb_translate, f->planepts[1]);
            VectorAdd (f->planepts[2], sb_translate, f->planepts[2]);
        }
    }
    calcWindings();
}

vec3_t	sb_mins, sb_maxs, sb_ctr;
int BBoxCount;
//addToBBoxCtr: this function merely accumulates bctrs and counts how many are added.
//sb_ctr and BBoxCount must be initialized to zero before looping calls to this func.
//afterwords sb_ctr must be divided by BBoxCount.
void SetBrush::addToBBoxCtr()
{
    if (CountFaces() < 4)
        return;

    VectorAdd(sb_ctr, bctr, sb_ctr);
    BBoxCount++;
}

void SetBrush::addToBBox()
{
    if (CountFaces() < 4)
        return;

    for (int k=0 ; k<3 ; k++)
    {
        if (bmins[k] < sb_mins[k])
            sb_mins[k] = bmins[k];
        if (bmaxs[k] > sb_maxs[k])
            sb_maxs[k] = bmaxs[k];
    }
}

void SetBrush::flushTextures()	// call when texture palette changes
{
    for (face_t *f = faces.p_next; f != &faces; f = f->p_next)
    {
        LoopProblem("brush flush");
        delete f->qtexture;
        f->qtexture = NULL;
    }
    calcWindings();
}

void SetBrush::select()
{
    map_i[set.curmap]->setCurrentEntity(parent);
    setSelected(true);
}

void SetBrush::deselect()
{
    setSelected(false);

    map *m = map_i[set.curmap];
    // the last selected brush determines
    m->setCurrentMinZ(bmins);
    m->setCurrentMaxZ(bmaxs);
}

void SetBrush::remove()
{
// the last selected brush determines
    if (!IsInvalid())
    {
        map *m = map_i[set.curmap];
        m->setCurrentMinZ(bmins);
        m->setCurrentMaxZ(bmaxs);
    }

    if (parent)
        parent->removeObject(this);
}

vec3_t	sel_x, sel_y, sel_z;
//vec3_t	sel_org;

void SetBrush::transform()
{
    vec3_t	old;
    float	*p;

    for (face_t *f = faces.p_next; f != &faces; f = f->p_next)
    {
        LoopProblem("brush transform");
        for (int j=0 ; j<3 ; j++)
        {
            p = f->planepts[j];
            VectorCopy (p, old);
            VectorSubtract (old, sb_ctr, old);
            p[0] = DotProduct (old, sel_x);
            p[1] = DotProduct (old, sel_y);
            p[2] = DotProduct (old, sel_z);
            VectorAdd (p, sb_ctr, p);
        }
    }
    calcWindings();
}

void SetBrush::flipNormals()	// used after an inside-out transform (flip x/y/z)
{
    face_t *f;
    vec3_t	temp;

    for (f = faces.p_next; f != &faces; f = f->p_next)
    {
        LoopProblem("brush flip");
        VectorCopy (f->planepts[0], temp);
        VectorCopy (f->planepts[2], f->planepts[0]);
        VectorCopy (temp, f->planepts[2]);
    }
    calcWindings();
}

void SetBrush::carveByClipper()
{
    face_t face;

    memset(&face,0,sizeof(face_t));

    if (!map_i[set.curmap]->clipper_i->getFace(&face))
        return;

    if (!parent->modifiable)
        return;

    addFace(this,&face);
}

void SetBrush::splitByClipper()
{
    face_t face;
    face_t fb;
    vec3_t	temp;
    SetBrush *b;
    Entity *p;

    memset(&face,0,sizeof(face_t));

    if (!map_i[set.curmap]->clipper_i->getFace(&face))
        return;

    p = parent;

    if (!p->modifiable)
        return;

    b = copy();
    b->parent = NULL;  // for now
    // make a copy...
    // flip face
    fb = face;
    VectorCopy (fb.planepts[0], temp);
    VectorCopy (fb.planepts[2], fb.planepts[0]);
    VectorCopy (temp, fb.planepts[2]);

    // add to copy
    addFace(b,&fb);
    if (b->IsInvalid())
    {
        delete b;
    }
    else
    {
        b->setParent(p); // FIXME, LINK AT END????
        p->addObject(b);
        b->setSelected(splitSelect);
    }

    // do the original...
    addFace(this,&face);
}

void SetBrush::takeCurrentTexture()
{
    texturedef_t	td;
    texWindow->getTextureDef(&td);
    setTexturedef(&td);
}

float	sb_floor_dir, sb_floor_dist;

void SetBrush::feetToFloor()
{
    float	oldz;
    vec3_t	p1, p2;
    face_t *frontface, *backface;
    vec3_t	frontpoint, backpoint;
    int fflag, bflag;
    float	dist;

    map *m = map_i[set.curmap];
    VectorCopy(m->eye[m->cureye],p1);

    VectorCopy (p1, p2);
    oldz = p1[2] - 48;

    p1[2] = 4096;
    p2[2] = -4096;

    clipRay(p1,p2,frontpoint,&frontface,backpoint,&backface, &fflag, &bflag);

    if (frontface == NULL)
        return;

    dist = frontpoint[2] - oldz;

    if (sb_floor_dir == 1)
    {
        if (dist > 0 && dist < sb_floor_dist)
            sb_floor_dist = dist;
    }
    else
    {
        if (dist < 0 && dist > sb_floor_dist)
            sb_floor_dist = dist;
    }
}


/*
===============================================================================

BRUSH SUBTRACTION

===============================================================================
*/

vec3_t	carvemin, carvemax;
//int		numcarvefaces;
face_t	*carvefaces;
Entity *carve_in, *carve_out;

// returns the new brush formed after the addition of the given plane
// nil is returned if it faced all of the original setbrush
void SetBrush::addFace(SetBrush *in, face_t *f)
{
    SetBrush *b = in;
    if (b->CountFaces() >= MAX_FACES)
    {
        syserror(const_cast<char *> ("Brush with too many faces..."));
        return;
    }
    face_t *af = new face_t;
    memset(af, 0, sizeof(face_t));

    *af = *f;
    af->texture = b->faces.p_next->texture;
    af->qtexture = NULL;
    af->w = NULL;

    b->addObject(af);
    b->calcWindings();
    b->removeUnusedFaces();
}

void clipByFace(face_t *fa,SetBrush *in, SetBrush **f, SetBrush **b)
{
    SetBrush	*front, *back;
    face_t	fb;
    vec3_t	temp;

    fb = *fa;
    VectorCopy (fb.planepts[0], temp);
    VectorCopy (fb.planepts[2], fb.planepts[0]);
    VectorCopy (temp, fb.planepts[2]);

    front = in->copy(); // copy this brush...
    back  = in->copy();

    // set up the windings...
    front->calcWindings();
    back->calcWindings();

    in->addFace(back,fa);
    in->addFace(front,&fb);

    if (back->IsInvalid())
    {
        delete back;
        *b = 0;
    }
    else
    {
        *b = back;
    }

    if (front->IsInvalid())
    {
        delete front;
        *f = 0;
    }
    else
    {
        *f = front;
    }
}

void carve(SetBrush **in)
{
    int		i;
    SetBrush *front, *back;
    SetBrush *b;

    //callstocarve++;

    front = back = NULL;

    back = *in;

    // skip invisible groups...
    if (set.group_mode)
    {
        if (!map_i[set.curmap]->groups->GetVisible(back->group))
        {
            carve_out->addObject(back);
            return;
        };
    };

    // skip regioned
    if (set.region_mode && back->IsRegioned())
    {
        carve_out->addObject(back);
        return;
    };
    // check bboxes
    for (i=0 ; i<3 ; i++)
        if (back->bmins[i] >= carvemax[i] || back->bmaxs[i] <= carvemin[i])
        {
            carve_out->addObject(back);
            return;
        }

    // carve by the planes
    face_t *f;
    for (f = carvefaces->p_next; f != carvefaces; f = f->p_next)
    {
        LoopProblem("brush carve");
        // if the input brush is still valid...
        if (back)
        {

            b = back;

            // clip the input brush by the each face and return halves in front an back...
            clipByFace(f,b,&front,&back);

            b->remove();
            delete b;

            *in = back;
        };

        if (front)  // if front brush is still valid
            carve_out->addObject(front); // add it to the output list...

        if (!back)
            return;
    }

    if (back)
        carve_in->addObject(back);
}

/*
==================
setCarveVars
==================
*/
void SetBrush::setCarveVars()
{
    VectorCopy (bmins, carvemin);
    VectorCopy (bmaxs, carvemax);
    carvefaces = &faces;
}

/*
===========
ClosestRenderSelf
===========
*/
void SetBrush::ClosestRenderSelf()
{
    if (!IsDrawable())
        return;

    face_t *f;
    REN_testBrush = this; // remember who we are... could pass it findcf...

    // hack to draw entity boxes as single flat color
    for (f = faces.p_next; f != &faces; f = f->p_next)
    {
        LoopProblem("brush closest");
        REN_testFace = f;
        REN_FindClosestFace (f);
    }
}
