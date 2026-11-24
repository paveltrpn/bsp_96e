#include "global.h"

// Search for brushes with "mixed face contents".
//
int map::MixFilter(texturedef_t *td)
{
    // "de"select certain ones, like "detail etc."
    //
    if (set.game_mode == 2)
    {
        return td->contents;
    }
    else
    {
        if (!strnicmp(td->texture, "clip", 4))
            return 1;
        if (td->texture[0] == '*')
        {
            if (!strnicmp(td->texture+1,"lava",4))
            {
                return 2;
            }
            else if (!strnicmp(td->texture+1,"slime",5))
            {
                return 3;
            }
            return 4;
        }
        if (!strnicmp(td->texture,"sky",3))
            return 5;
    }
    return 0;
}

int map::MixedFaceSearch()
{
    Entity *e;
    SetBrush *b;
    face_t *f;

    int Flagged = 0;

    makeSelectedPerform(SEL_DESELECT);

    for (e = objects.p_next; e != &objects; e = e->p_next)
    {
        if (!e->modifiable)
            continue;

        for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
        {
            // get face 0 contents...
            if (!b->IsDrawable())
                continue;

            // Must have a first face that is real
            if (b->faces.p_next == &b->faces)
                continue;

            int Face0Contents = MixFilter(&b->faces.p_next->texture);

            for (f = b->faces.p_next; f != &b->faces; f = f->p_next)
            {
                LoopProblem("brush mixed");

                if (MixFilter(&f->texture) == Face0Contents)
                    continue;

                b->setSelected(true);
                Flagged++;
                break;
            }
        }
    }
    return Flagged;
}
int map::TextureSearch(char *name)
{
    Entity *e;
    SetBrush *b;
    face_t *f;

    int Flagged = 0;

    makeSelectedPerform(SEL_DESELECT);

    for (e = objects.p_next; e != &objects; e = e->p_next)
    {
        if (!e->modifiable)
            continue;

        for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
        {
            // get face 0 contents...
            if (!b->IsDrawable())
                continue;

            // Must have a first face that is real
            if (b->faces.p_next == &b->faces)
                continue;

            for (f = b->faces.p_next; f != &b->faces; f = f->p_next)
            {
                LoopProblem("tex search");
                if (strcmpi(f->texture.texture, name))
                    continue;

                b->setSelected(true);
                Flagged++;
                break;
            }
        }
    }
    return Flagged;
}

int map::makeStairs(Entity *currentEntity, SetBrush *seedBrush, int removeSeed, int style, int numsteps, int hgap, int vgap,
                    int down, int xory, float turns, int distfc)
{

    SetBrush *b;
    int i;

    int axis1, axis2;
    vec3_t	mins, maxs;
    vec3_t   cmins,cmaxs;
    float ystep;
    float zstep;
    float zstart;
    texturedef_t td;
    float ang;
    float ctrx, ctry;
    float distx,disty;
    float distfromcenter = (float)distfc;
    float stepdepth;

    float nearfacex, nearfacey;
    float farfacex, farfacey;
//	float vecmag;
//	float vectorx, vectory;
    float leftx, lefty, rightx, righty;

    float zmn, zmx;
    float numrotations = turns;
    float astep;
    int a1, a2;

    dirty = 1;

    b = seedBrush;
    if (!b)
        return 0;

    int grp;
    grp = b->group;
    td = *b->texturedef();
    b->getMins(mins,maxs);

    if (xory == 0)
    {
        axis1 = 1;
        axis2 = 0;
    }
    else
    {
        axis1 = 0;
        axis2 = 1;
    };

    if (numsteps < 2)
        numsteps = 2;

    float brushWidth;
    zstep = ((maxs[2]-mins[2])/(float)(numsteps));

    // gap can be 0 all the way to the step height-1...
    vgap = (int)min((float)(zstep-1.0),(float)vgap);
    vgap = (int)max((float)0.0,(float)vgap);
//	if (down) {
//		zstep = -zstep;
//		zstart = maxs[2];
//	} else {
    zstart = mins[2];
//	};

    switch (style)
    {
        // hgap is the step width
        // vgap is home much space to leave between steps (if possible);
    case 0:
        brushWidth = maxs[axis1]-mins[axis1];
        hgap = (int)min(brushWidth-(float)(numsteps-1),(float)hgap);
        hgap = max(hgap,1); // at least 1...

        ystep = ((brushWidth-hgap)/(float)(numsteps-1));

        for (i = 0; i<numsteps; i++)
        {
            b = new SetBrush();
            b->group = grp;

            cmins[axis2] = (int)(mins[axis2]);
            cmaxs[axis2] = (int)(maxs[axis2]);

            cmins[axis1] = (int)(mins[axis1] + ystep*i);
            cmaxs[axis1] = (int)(mins[axis1] + ystep*i + hgap);

            if (!down)
            {
                cmins[2] = (int)(zstart + zstep*(i));
                cmaxs[2] = (int)(zstart + zstep*(i+1) - vgap);
            }
            else
            {
                cmins[2] = (int)(zstart + zstep*(numsteps-i-1));
                cmaxs[2] = (int)(zstart + zstep*(numsteps-i) - vgap);
            };
            // calc mins, maxs;
            b->initOwner(currentEntity,cmins,cmaxs,&td);
            // add to world and select
            currentEntity->addObject(b);
            b->setSelected(true);
        };
        break;
    case 2:
        brushWidth = maxs[axis1]-mins[axis1];
        hgap = (int)min(brushWidth-(float)(numsteps-1),(float)hgap);
        hgap = max(hgap,1); // at least 1...

        ystep = ((brushWidth-hgap)/(float)(numsteps-1));

        for (i = 0; i<numsteps; i++)
        {
            b = new SetBrush();
            b->group = grp;

            cmins[axis2] = (int)(mins[axis2]);
            cmaxs[axis2] = (int)(maxs[axis2]);

            if (!down)
            {
                cmins[axis1] = (int)(maxs[axis1] - (ystep*(numsteps-i-1) + hgap));
                cmaxs[axis1] = (int)(maxs[axis1]);

                cmins[2] = (int)(zstart + zstep*(i));
                cmaxs[2] = (int)(zstart + zstep*(i+1) - vgap);
            }
            else
            {
                cmins[axis1] = (int)(mins[axis1]);
                cmaxs[axis1] = (int)(mins[axis1] + ystep*i + hgap);

                cmins[2] = (int)(zstart + zstep*(numsteps-i-1));
                cmaxs[2] = (int)(zstart + zstep*(numsteps-i)   - vgap);
            };

            // calc mins, maxs;
            b->initOwner(currentEntity,cmins,cmaxs,&td);
            // add to world and select
            currentEntity->addObject(b);
            b->setSelected(true);
        };
        break;
    case 1:

        // make sure the entries are okay...
        numrotations = max((float)0.1,numrotations);

        ctrx = (int)((maxs[0] + mins[0])/2.0);
        ctry = (int)((maxs[1] + mins[1])/2.0);

        distx = (int)((maxs[0] - mins[0])/2.0); // compute radius...
        disty = (int)((maxs[1] - mins[1])/2.0);

        stepdepth = min(distx,disty);
        stepdepth -= distfromcenter;

        stepdepth = (float)max((float)1.0,stepdepth); // at least 1...

        // ystep = (maxs[axis1]-mins[axis1])/(float)numsteps;

        ang = 0.0;
        astep = (numrotations*2*M_PI)/numsteps;

        a1 = 1;
        a2 = 0;

        for (i = 0; i<numsteps; i++)
        {

            nearfacex = rint(ctrx+distfromcenter*sin(ang));
            nearfacey = rint(ctry+distfromcenter*cos(ang));

            farfacex = rint(ctrx+(distfromcenter+stepdepth)*sin(ang));
            farfacey = rint(ctry+(distfromcenter+stepdepth)*cos(ang));

            //vectorx = nearfacex - ctrx;
            //vectory = nearfacey - ctry;

            // vecmag = sqrt(vectorx*vectorx + vectory*vectory);

            //vectorx /= vecmag;
            //vectory /= vecmag;

            leftx = rint(nearfacex + (hgap/2.0) * sin(ang - M_PI/2.0));
            lefty = rint(nearfacey + (hgap/2.0) * cos(ang - M_PI/2.0));

            rightx = rint(nearfacex + (hgap/2.0) * sin(ang + M_PI/2.0));
            righty = rint(nearfacey + (hgap/2.0) * cos(ang + M_PI/2.0));

            b = new SetBrush();
            b->group = grp;

            if (!down)
            {
                zmn = (int)(zstart + zstep*(i));
                zmx = (int)(zstart + zstep*(i+1) - vgap);
            }
            else
            {
                zmn = (int)(zstart + zstep*(numsteps-i-1));
                zmx = (int)(zstart + zstep*(numsteps-i) - vgap);
            };

            //b->numfaces = 6;
            face_t *f;

            f = new face_t;
            memset(f, 0, sizeof(face_t));

            // nearface
            f->planepts[a1][0] = rightx;
            f->planepts[a1][1] = righty;
            f->planepts[a1][2] = zmx;

            f->planepts[a2][0] = leftx;
            f->planepts[a2][1] = lefty;
            f->planepts[a2][2] = zmx;

            f->planepts[2][0] = leftx;
            f->planepts[2][1] = lefty;
            f->planepts[2][2] = zmn;

            b->addObject(f);

            f = new face_t;
            memset(f, 0, sizeof(face_t));

            // farface
            f->planepts[a1][0] = rint(farfacex + (hgap/2.0) * sin(ang-M_PI/2.0));
            f->planepts[a1][1] = rint(farfacey + (hgap/2.0) * cos(ang-M_PI/2.0));
            f->planepts[a1][2] = zmx;

            f->planepts[a2][0] = rint(farfacex + (hgap/2.0) * sin(ang+M_PI/2.0));
            f->planepts[a2][1] = rint(farfacey + (hgap/2.0) * cos(ang+M_PI/2.0));
            f->planepts[a2][2] = zmx;

            f->planepts[2][0] = rint(farfacex + (hgap/2.0) * sin(ang+M_PI/2.0));
            f->planepts[2][1] = rint(farfacey + (hgap/2.0) * cos(ang+M_PI/2.0));
            f->planepts[2][2] = zmn;

            b->addObject(f);

            f = new face_t;
            memset(f, 0, sizeof(face_t));

            // left face
            f->planepts[a1][0] = leftx;
            f->planepts[a1][1] = lefty;
            f->planepts[a1][2] = zmx;

            f->planepts[a2][0] = rint(farfacex + (hgap/2.0) * sin(ang-M_PI/2.0));
            f->planepts[a2][1] = rint(farfacey + (hgap/2.0) * cos(ang-M_PI/2.0));
            f->planepts[a2][2] = zmx;

            f->planepts[2][0] = rint(farfacex + (hgap/2.0) * sin(ang-M_PI/2.0));
            f->planepts[2][1] = rint(farfacey + (hgap/2.0) * cos(ang-M_PI/2.0));
            f->planepts[2][2] = zmn;

            b->addObject(f);

            f = new face_t;
            memset(f, 0, sizeof(face_t));

            // right face
            f->planepts[a1][0] = rint(farfacex + (hgap/2.0) * sin(ang+M_PI/2.0));
            f->planepts[a1][1] = rint(farfacey + (hgap/2.0) * cos(ang+M_PI/2.0));
            f->planepts[a1][2] = zmx;

            f->planepts[a2][0] = rint(nearfacex + (hgap/2.0) * sin(ang+M_PI/2.0));
            f->planepts[a2][1] = rint(nearfacey + (hgap/2.0) * cos(ang+M_PI/2.0));
            f->planepts[a2][2] = zmx;

            f->planepts[2][0] = rint(nearfacex + (hgap/2.0) * sin(ang+M_PI/2.0));
            f->planepts[2][1] = rint(nearfacey + (hgap/2.0) * cos(ang+M_PI/2.0));
            f->planepts[2][2] = zmn;

            b->addObject(f);

            f = new face_t;
            memset(f, 0, sizeof(face_t));

            // top
            f->planepts[a1][0] = rint(farfacex + (hgap/2.0) * sin(ang-M_PI/2.0));
            f->planepts[a1][1] = rint(farfacey + (hgap/2.0) * cos(ang-M_PI/2.0));
            f->planepts[a1][2] = zmx;

            f->planepts[a2][0] = rint(nearfacex + (hgap/2.0) * sin(ang-M_PI/2.0));
            f->planepts[a2][1] = rint(nearfacey + (hgap/2.0) * cos(ang-M_PI/2.0));
            f->planepts[a2][2] = zmx;

            f->planepts[2][0] = rint(nearfacex + (hgap/2.0) * sin(ang+M_PI/2.0));
            f->planepts[2][1] = rint(nearfacey + (hgap/2.0) * cos(ang+M_PI/2.0));
            f->planepts[2][2] = zmx;

            b->addObject(f);

            f = new face_t;
            memset(f, 0, sizeof(face_t));

            // bottom
            f->planepts[a1][0] = rint(farfacex + (hgap/2.0) * sin(ang+M_PI/2.0));
            f->planepts[a1][1] = rint(farfacey + (hgap/2.0) * cos(ang+M_PI/2.0));
            f->planepts[a1][2] = zmn;

            f->planepts[a2][0] = rint(nearfacex + (hgap/2.0) * sin(ang+M_PI/2.0));
            f->planepts[a2][1] = rint(nearfacey + (hgap/2.0) * cos(ang+M_PI/2.0));
            f->planepts[a2][2] = zmn;

            f->planepts[2][0] = rint(nearfacex + (hgap/2.0) * sin(ang-M_PI/2.0));
            f->planepts[2][1] = rint(nearfacey + (hgap/2.0) * cos(ang-M_PI/2.0));
            f->planepts[2][2] = zmn;

            b->addObject(f);

            b->setTexturedef(&td);
            b->setParent(currentEntity);
            currentEntity->addObject(b);
            b->setSelected(true);
            ang += astep;  // current angle...

        };
        break;
    default:
        break;
    };
    if (removeSeed)
    {
        seedBrush->remove();
        delete seedBrush;
    };

    cleanUpInvalidObjects();

    return (int)zstep;
}

// return 0 if okay.
// return 2 if there is no seed brush or nothing in map selected...
// return 1 if the resulting brush is too invalid (center of arch below bottom of brush...)
int map::makeWedge(Entity *currentEntity, SetBrush *seedBrush, int removeSeed, int axis, bool flip)
{

    if (numSelected() != 1)
    {
        return 2;
    }

    // for rect brush
    face_t *face;
    SetBrush *b;

    vec3_t	mins, maxs;
    texturedef_t td;

    int a1 = 2, a2 = 0, a3 = 1;

    switch (axis)
    {
    case 1:
        a1 = 1;
        a2 = 2;
        a3 = 0;
        break;
    case 2:
        a1 = 0;
        a2 = 1;
        a3 = 2;
    }

    b = seedBrush;
    if (!b)
    {
        return 2;
    }

    dirty = 1;

    int grp = b->group;
    td = *b->texturedef();
    b->getMins(mins,maxs);

    face = new face_t;
    memset(face,0,sizeof(face_t));

    face->planepts[0][a1] = mins[a1];
    face->planepts[0][a2] = mins[a2];
    face->planepts[0][a3] = mins[a3];

    face->planepts[1][a1] = maxs[a1];
    face->planepts[1][a2] = maxs[a2];
    face->planepts[1][a3] = mins[a3];

    face->planepts[2][a1] = maxs[a1];
    face->planepts[2][a2] = maxs[a2];
    face->planepts[2][a3] = maxs[a3];

    if (flip)
    {
        vec3_t temp;
        VectorCopy(face->planepts[0],temp);
        VectorCopy(face->planepts[1],face->planepts[0]);
        VectorCopy(temp,face->planepts[1]);
    }

    b = new SetBrush();
    b->group = grp;
    b->initOwner(currentEntity,mins,maxs,&td);

    // add the "wedge face...
    b->addObject(face);
    // causes a recalculation
    b->setTexturedef(&td);

    b->setParent(currentEntity);
    currentEntity->addObject(b);
    b->setSelected(true);

    if (removeSeed)
    {
        seedBrush->remove();
        delete seedBrush;
    }
    if (b->IsInvalid())
    {
        cleanUpInvalidObjects();
        return 1;
    }
    return 0;
}

// return 0 if okay.
// return 2 if there is no seed brush or nothing in map selected...
// return 1 if the resulting brush is too short (center of arch below bottom of brush...)
int map::makeArch(Entity *currentEntity, SetBrush *seedBrush, int removeSeed,
                  int numsteps, int xory, int width, int rect)
{
// for rect brush
    face_t *facel;
    face_t *facer;
    face_t *facet;
    float leftx, leftz;
    float topx, topz;
    float rightx, rightz;
    face_t *f;

    // SetBrush *pb;  // the post brushes...
    int i;

    vec3_t	mins, maxs;
    vec3_t   cmins,cmaxs;

    float ctrx, ctrz;
    float nearfacex, nearfacez;
    float farfacex, farfacez;
    float nearleftx, nearleftz;
    float farleftx, farleftz;

    int a1, a2, a3;

    if (numSelected() != 1)
    {
        //	MessageBeep(MB_ICONEXCLAMATION);
        //	qprintf ("must have a single brush selected");
        return 2;
    }

    dirty = 1;

    // leave the brush???
    SetBrush *b = seedBrush;
    if (!b)
    {
        return 2;
    }

    int grp = b->group;
    texturedef_t td = *b->texturedef();
    b->getMins(mins,maxs);

    if (numsteps < 3)
        numsteps = 3;

    if (!xory)
    {
        a1 = 0;
        a2 = 1;
        a3 = 1;
    }
    else
    {
        a1 = 1;
        a2 = 0;
        a3 = 0;
    };

    width = min(45,width); // range 1 to 49 %...
    width = max(5,width);

    ctrx = rint((maxs[a1] + mins[a1])/2.0);

    float w = rint(((float)width/100.0)*(maxs[a1] - mins[a1]));

    float radius;
    radius = (maxs[a1] - mins[a1]) - 2*w;
    radius /= 2.0;

    float h = (maxs[2] - mins[2]) - (w + radius); // w - radius...from the top...

    if (h < 1.0)   // not tall enough to even make it...
    {
        return 1;
    };

    ctrz = mins[2]+h;

    float ang = 1.5*M_PI;
    float astep = M_PI/numsteps;

    float saveW = w;

    if (rect)    // make it huge!
    {
        for (i = 0; i < 3; i++)
            if ((maxs[i] - mins[i]) > w)
                w = (maxs[i] - mins[i]);

        w *= 8.0;  // something ridiculous.

        leftx = rint(mins[a1]);
        leftz = 0.0;

        topx = 0.0;
        topz = rint(maxs[2]);

        rightx = rint(maxs[a1]);
        rightz = 0.0;

        facel = new face_t;
        facer = new face_t;
        facet = new face_t;
        memset(facel,0,sizeof(face_t));
        memset(facer,0,sizeof(face_t));
        memset(facet,0,sizeof(face_t));

        f = facel;
        f->planepts[a1][a1] = leftx;
        f->planepts[a1][a2] = mins[a3];
        f->planepts[a1][2]  = leftz +16.0;

        f->planepts[a2][a1] = leftx;
        f->planepts[a2][a2] = mins[a3];
        f->planepts[a2][2]  = leftz + 0.0;

        f->planepts[2][a1] = leftx;
        f->planepts[2][a2] = maxs[a3];;
        f->planepts[2][2]  = leftz + 0.0;

        f = facer;
        f->planepts[a1][a1] = rightx;
        f->planepts[a1][a2] = mins[a3];
        f->planepts[a1][2] = rightz + 0.0;

        f->planepts[a2][a1] = rightx;
        f->planepts[a2][a2] = mins[a3];
        f->planepts[a2][2] = rightz + 16.0;

        f->planepts[2][a1] = rightx;
        f->planepts[2][a2] = maxs[a3];
        f->planepts[2][2] = rightz + 0.0;

        f = facet;
        f->planepts[a1][a1] = topx + 16.0;
        f->planepts[a1][a2] = mins[a3];
        f->planepts[a1][2] = topz;

        f->planepts[a2][a1] = topx + 0.0;
        f->planepts[a2][a2] = mins[a3];
        f->planepts[a2][2] = topz;

        f->planepts[2][a1] = topx + 0.0;
        f->planepts[2][a2] = maxs[a3];
        f->planepts[2][2] = topz;
    }

    for (i = 0; i < numsteps; i++)
    {
        nearfacex = rint(ctrx+radius*sin(ang));
        nearfacez = rint(ctrz+radius*cos(ang));

        farfacex = rint(ctrx+(radius+w)*sin(ang));
        farfacez = rint(ctrz+(radius+w)*cos(ang));

        nearleftx = rint(ctrx+radius*sin(ang+astep));
        nearleftz = rint(ctrz+radius*cos(ang+astep));

        farleftx = rint(ctrx+(radius+w)*sin(ang+astep));
        farleftz = rint(ctrz+(radius+w)*cos(ang+astep));

        b = new SetBrush();
        b->group = grp;
        //b->numfaces = 6;

        f = new face_t;
        memset(f, 0, sizeof(face_t));
        // nearface
        f->planepts[a1][a1] = nearleftx;
        f->planepts[a1][a2] = maxs[a3];
        f->planepts[a1][2] = nearleftz;

        f->planepts[a2][a1] = nearfacex;
        f->planepts[a2][a2] = maxs[a3];
        f->planepts[a2][2] = nearfacez;

        f->planepts[2][a1] = nearfacex;
        f->planepts[2][a2] = mins[a3];
        f->planepts[2][2] = nearfacez;

        b->addObject(f);

        f = new face_t;
        memset(f, 0, sizeof(face_t));

        // farface
        f->planepts[a1][a1] = farleftx;
        f->planepts[a1][a2] = mins[a3];
        f->planepts[a1][2] = farleftz;

        f->planepts[a2][a1] = farfacex;
        f->planepts[a2][a2] = mins[a3];
        f->planepts[a2][2] = farfacez;

        f->planepts[2][a1] = farfacex;
        f->planepts[2][a2] = maxs[a3];
        f->planepts[2][2] = farfacez;

        b->addObject(f);

        f = new face_t;
        memset(f, 0, sizeof(face_t));

        // left face
        f->planepts[a1][a1] = farleftx;
        f->planepts[a1][a2] = maxs[a3];
        f->planepts[a1][2] = farleftz;

        f->planepts[a2][a1] = farfacex;
        f->planepts[a2][a2] = maxs[a3];
        f->planepts[a2][2] = farfacez;

        f->planepts[2][a1] = nearfacex;
        f->planepts[2][a2] = maxs[a3];
        f->planepts[2][2] = nearfacez;

        b->addObject(f);

        f = new face_t;
        memset(f, 0, sizeof(face_t));

        // right face
        f->planepts[a1][a1] = nearleftx;
        f->planepts[a1][a2] = mins[a3];
        f->planepts[a1][2] = nearleftz;

        f->planepts[a2][a1] = nearfacex;
        f->planepts[a2][a2] = mins[a3];
        f->planepts[a2][2] = nearfacez;

        f->planepts[2][a1] = farfacex;
        f->planepts[2][a2] = mins[a3];
        f->planepts[2][2] = farfacez;

        b->addObject(f);

        f = new face_t;
        memset(f, 0, sizeof(face_t));

        // top
        f->planepts[a1][a1] = farleftx;
        f->planepts[a1][a2] = maxs[a3];
        f->planepts[a1][2] = farleftz;

        f->planepts[a2][a1] = nearleftx;
        f->planepts[a2][a2] = maxs[a3];
        f->planepts[a2][2] = nearleftz;

        f->planepts[2][a1] = nearleftx;
        f->planepts[2][a2] = mins[a3];
        f->planepts[2][2] = nearleftz;

        b->addObject(f);

        f = new face_t;
        memset(f, 0, sizeof(face_t));

        // bottom
        f->planepts[a1][a1] = farfacex;
        f->planepts[a1][a2] = mins[a3];
        f->planepts[a1][2] = farfacez;

        f->planepts[a2][a1] = nearfacex;
        f->planepts[a2][a2] = mins[a3];
        f->planepts[a2][2] = nearfacez;

        f->planepts[2][a1] = nearfacex;
        f->planepts[2][a2] = maxs[a3];
        f->planepts[2][2] = nearfacez;

        b->addObject(f);

        if (rect)
        {
            //these need to be sent as new alloc'd pointers, so copy here. orignals must be deleted later
            face_t *rf;
            rf = new face_t;
            memcpy(rf,facel,sizeof(face_t));
            b->addObject(rf);
            rf = new face_t;
            memcpy(rf,facet,sizeof(face_t));
            b->addObject(rf);
            rf = new face_t;
            memcpy(rf,facer,sizeof(face_t));
            b->addObject(rf);
        }

        // causes a recalculation
        b->setTexturedef(&td);
        b->setParent(currentEntity);
        currentEntity->addObject(b);
        b->setSelected(true);
        ang += astep;  // current angle...
    }
    cmins[a1] = mins[a1];
    cmins[a2] = mins[a2];
    cmins[2] = mins[2];
    cmaxs[a1] = mins[a1]+saveW;
    cmaxs[a2] = maxs[a2];
    cmaxs[2] = mins[2]+h;

    b = new SetBrush();
    b->group = grp;
    b->initOwner(currentEntity,cmins,cmaxs,&td);
    currentEntity->addObject(b);
    b->setSelected(true);

    cmins[a1] = maxs[a1]-saveW;
    cmins[a2] = mins[a2];
    cmins[2] = mins[2];
    cmaxs[a1] = maxs[a1];
    cmaxs[a2] = maxs[a2];
    cmaxs[2] = mins[2]+h;

    b = new SetBrush();
    b->group = grp;
    b->initOwner(currentEntity,cmins,cmaxs,&td);
    currentEntity->addObject(b);
    b->setSelected(true);

    if (removeSeed)
    {
        seedBrush->remove();
        delete seedBrush;
    }
    if(rect)
    {
        delete facel;
        delete facet;
        delete facer;
    }
    cleanUpInvalidObjects();

    return 0;
}

void map::makeLights(int x, int y, int z, int val, char *lightType)
{
    char value[128];
    int i, j, k;

    vec3_t	mins, maxs;
    vec3_t   min;

    if ((numSelected() != 1) || (!lightType) || (!lightType[0]))
    {
        //	qprintf ("must have a single brush selected");
        return;
    }

    dirty = 1;

    // leave the brush???
    SetBrush *b = selectedBrush();
    int grp = b->group;
    b->getMins(mins,maxs);
    b->remove();
    delete b;

    int igap, jgap, kgap;
    int rx, ry, rz;

    if (x <= 1)
    {
        rx = 1;
        igap = 0;
    }
    else
    {
        rx = x;
        igap = (int)((maxs[0] - mins[0])/(rx-1));
    };

    if (y <= 1)
    {
        ry = 1;
        jgap = 0;
    }
    else
    {
        ry = y;
        jgap = (int)((maxs[1] - mins[1])/(ry-1));
    };

    if (z <= 1)
    {
        rz = 1;
        kgap = 0;
    }
    else
    {
        rz = z;
        kgap = (int)((maxs[2] - mins[2])/(rz-1));
    };

    for (i = 0; i < rx; i++)
    {
        for (j = 0; j < ry; j++)
        {
            for (k = 0; k < rz; k++)
            {
                if (rx == 1)
                {
                    min[0] = rint((mins[0]+maxs[0])/2.0);
                }
                else
                {
                    min[0] = mins[0] + i * igap;
                };

                if (ry == 1)
                {
                    min[1] = rint((mins[1]+maxs[1])/2.0);
                }
                else
                {
                    min[1] = mins[1] + j * jgap;
                };

                if (rz == 1)
                {
                    min[2] = rint((mins[2]+maxs[2])/2.0);
                }
                else
                {
                    min[2] = mins[2] + k * kgap;
                };

                sb_newowner = new Entity;
                sb_newowner->setKey(const_cast<char *> ("classname"), lightType);

                sprintf (value, "%i %i %i",
                         (int)min[0],
                         (int)min[1],
                         (int)min[2]);

                sb_newowner->setKey(const_cast<char *> ("origin"), value);
                sb_newowner->createFixedBrush(min);

                sprintf(value,"%i",val);
                sb_newowner->setKey(const_cast<char *> ("light"), value);
                sb_newowner->objects.p_next->group = grp;
                addObject(sb_newowner);
            };
        };
    };
}

void map::rotateBrush(int r, int p, int y)
{
    SetBrush *b;
    Entity *e;
    face_t *f;

    vec3_t p1, p2;

    float fr, fp, fy;

    float X[4][4];
    float Temp[4][4];
    int i, j, k;

    dirty = 1;

    getSelectedCenter();

    sb_ctr[0] = snapToGrid(sb_ctr[0]);
    sb_ctr[1] = snapToGrid(sb_ctr[1]);
    sb_ctr[2] = snapToGrid(sb_ctr[2]);

    fr = ((float)r/180.0)*M_PI;
    fp = ((float)p/180.0)*M_PI;
    fy = ((float)y/180.0)*M_PI;

    vec3_t zero;
    VectorCopy(vec3_origin,zero);
    findQRotate(X,Temp,zero,fy,fp,fr);

    for (e = objects.p_next; e != &objects; e = e->p_next)
    {
        LoopProblem("ROTATEBE");
        if (!e->modifiable)
        {
            // if not modifiable, just translate the origin...
            for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
            {
                LoopProblem("ROTATEBB");
                if (!b->IsDrawable())
                    continue;
                if (!b->IsSelected()) // only rotate selected brushes...
                    continue;

                //b->getMins(mins,maxs);

                for (i = 0; i < 3; i++)
                    p1[i] = b->bctr[i]/*mins[i]+maxs[i])/2.0*/-sb_ctr[i]; // take first point, find dist to center

                transform(p1,X,p2); // translate it...

                for (i = 0; i < 3; i++)
                    sb_translate[i] = rint((p2[i]+sb_ctr[i])-b->bctr[i]/*mins[i]+maxs[i])/2.0*/);  // move everything by the diff!

                b->translate();
            }
        }
        else
        {
            for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
            {
                LoopProblem("ROTATEBB2");
                if (!b->IsDrawable())
                    continue;
                if (!b->IsSelected()) // only rotate selected brushes...
                    continue;

                // cycle through brushes...
                for (f = b->faces.p_next; f != &b->faces; f = f->p_next)
                {
                    LoopProblem("brush rotate3");
                    for (j = 0; j < 3; j++)
                    {

                        for (k = 0; k < 3; k++)
                            p1[k] = f->planepts[j][k]-sb_ctr[k];

                        transform(p1,X,p2);

                        for (k = 0; k < 3; k++)
                            f->planepts[j][k] = rint(p2[k]+sb_ctr[k]);
                    }
                }

                b->calcWindings();

            }
        }
    }
    cleanUpInvalidObjects();
}

typedef struct pointpair_s
{
    int p1;
    int p2;
} pointpair_t;

void map::rotateBrushOrg(int r, int p, int y, vec3_t org)
{
    SetBrush *b;
    Entity *e;
    face_t *f;
    Entity *saveCurrent;

    saveCurrent = current;

    vec3_t mins, maxs;

    vec3_t p1, p2;

    float X[4][4];
    float Temp[4][4];
    int i, j;

    dirty = 1;

    float fr = DEG2RAD(r);
    float fp = DEG2RAD(p);
    float fy = DEG2RAD(y);

    vec3_t zero;
    VectorCopy(vec3_origin,zero);
    findQRotate(X,Temp,zero,fy,fp,fr);

    plane_t *savePlanes = NULL;
    winding_t **saveWindings = NULL;
    pointpair_t *savePoints;
    winding_t *w;
    float ds, dt;
    float ns, nt;
    float o_ang, n_ang, deltaAng;
    bool noLock;
    //   vec3_t xa, ya;
    int pt1, pt2;    // farthest apart points...

    for (e = objects.p_next; e != &objects; e = e->p_next)
    {
        LoopProblem("RBORG");
        if (!e->modifiable)
        {
            // if not modifiable, just translate the origin...
            for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
            {
                LoopProblem("RBORGB");
                if (!b->IsDrawable())
                    continue;
                if (!b->IsSelected()) // only rotate selected brushes...
                    continue;

                b->getMins(mins,maxs);
                // take first point, find dist to center
                p1[0] = (mins[0]+maxs[0])/2.0-org[0];
                p1[1] = (mins[1]+maxs[1])/2.0-org[1];
                p1[2] = (mins[2]+maxs[2])/2.0-org[2];
                // translate it...
                transform(p1,X,p2);
                // move everything by the diff!
                sb_translate[0] = rint((p2[0]+org[0])-(mins[0]+maxs[0])/2.0);
                sb_translate[1] = rint((p2[1]+org[1])-(mins[1]+maxs[1])/2.0);
                sb_translate[2] = rint((p2[2]+org[2])-(mins[2]+maxs[2])/2.0);

                b->translate();
            }
        }
        else
        {
            for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
            {
                LoopProblem("RBORGB2");

                if (!b->IsDrawable())
                    continue;
                if (!b->IsSelected()) // only rotate selected brushes...
                    continue;
                if (b->IsInvalid())
                    continue;
                int numfaces = b->CountFaces();

                if (b->IsLocked() && !set.disable_rotation_texlock)
                {
                    saveWindings = new winding_t *[numfaces];
                    memset(saveWindings,0,numfaces * sizeof(winding_t *));

                    savePlanes = new plane_t[numfaces];
                    memset(savePlanes,0,numfaces * sizeof(plane_t));

                    savePoints = new pointpair_t[numfaces];
                    memset(savePoints,0,numfaces * sizeof(pointpair_t));
                }

                noLock = false;
                i = 0;
                // cycle through brushes and rotate each plane...
                for (f = b->faces.p_next; f != &b->faces; f = f->p_next, i++)
                {
                    LoopProblem("brush lock1");
                    w = f->w;
                    if (!w)
                    {
                        noLock = true;
                        continue;
                    }

                    if (!set.disable_rotation_texlock && b->IsLocked())
                    {
                        saveWindings[i] = CopyWinding(f->w);
                        savePlanes[i] = f->plane;

                        pointpair_t bestPair = { 0, 1 };

                        if (w)
                        {
                            float bestD = -99999.0;

                            for (pt1 = 0; pt1 < w->numpoints; pt1++)
                            {
                                for (pt2 = pt1 + 1; pt2 < w->numpoints; pt2++)
                                {
                                    float dist = 0;

                                    dist += (w->points[pt2][0]-w->points[pt1][0])*(w->points[pt2][0]-w->points[pt1][0]);
                                    dist += (w->points[pt2][1]-w->points[pt1][1])*(w->points[pt2][1]-w->points[pt1][1]);
                                    dist += (w->points[pt2][2]-w->points[pt1][2])*(w->points[pt2][2]-w->points[pt1][2]);

                                    dist = sqrt(dist);

                                    if (dist > bestD)
                                    {
                                        bestD = dist;
                                        bestPair.p1 = pt1;
                                        bestPair.p2 = pt2;
                                    }
                                }
                            }
                        }
                        savePoints[i] = bestPair;
                    }

                    for (j = 0; j < 3; j++)
                    {
                        p1[0] = f->planepts[j][0]-org[0];
                        p1[1] = f->planepts[j][1]-org[1];
                        p1[2] = f->planepts[j][2]-org[2];
                        transform(p1,X,p2);
                        /* NOTE: these rint's are what fuck up brush dimensions.
                        however, they are necessary because Quake coords are saved as integers.
                        without these, what you see while editing and what you see after
                        opening a saved map will differ!
                        */
                        f->planepts[j][0] = rint(p2[0]+org[0]);
                        f->planepts[j][1] = rint(p2[1]+org[1]);
                        f->planepts[j][2] = rint(p2[2]+org[2]);
                    }
                }

                b->calcWindings();

                if (b->IsInvalid())
                    continue;


                if (!set.disable_rotation_texlock && b && b->IsLocked() && !noLock)
                {
                    // now "normalize each face..."  first rotate the texture, then scale,
                    // then finally offsets..
                    // don't forget to recalcwindings and free the "savewindings"

                    // scale warp
                    /*
                    		for (i = 0; i < b->numfaces; i++) {
                    			f = &b->faces[i];
                    	if (!f || !f->w)
                      		continue;

                    	w = f->w;
                    	if (w->numpoints < 2)
                      		continue;

                    	if (saveWindings[i]->numpoints < 2)
                      		continue;

                    	TextureAxisFromPlane(&f->plane, xa, ya);
                    	CrossProduct(xa, ya, texNormal);
                    	VectorNormalize(texNormal);

                    	float dot;
                    	dot = DotProduct(texNormal,f->plane.normal);
                    	if (!dot)
                     		dot = 1.0f;

                    	float adjust = 1/dot;

                    	TextureAxisFromPlane(&savePlanes[i], xa, ya);
                    	CrossProduct(xa, ya, texNormal);
                    	VectorNormalize(texNormal);

                    	dot = DotProduct(texNormal,savePlanes[i].normal);
                    	if (!dot)
                     		dot = 1.0f;

                    	float adjust2 = 1/dot;
                    	// Now the fun part
                    	// compare s, t values for first point...
                    	// shift to fix that
                    	// compute s, t vector to second point
                    	// rotate if necessary
                    	// compare length and scale if necessary

                    	//float oscale, nscale;
                    	float dscale;

                    	//ds = saveWindings[i]->points[1][3] - saveWindings[i]->points[0][3];
                    	//dt = saveWindings[i]->points[1][4] - saveWindings[i]->points[0][4];

                    	// rotation and scale account for ds dt vector...
                    	//ns = w->points[1][3] - w->points[0][3];
                    	//nt = w->points[1][4] - w->points[0][4];

                    	//oscale = ds * ds + dt * dt;
                    	//nscale = ns * ns + nt * nt;

                    	//if (!oscale)
                    	//	oscale = 1.0;

                    	if (!adjust2)
                      		adjust2 = 1.0;

                    	dscale = adjust/adjust2; // nscale/oscale;

                    	b->faces[i].texture.scale[0] /= dscale;  // now we are centered...
                    	b->faces[i].texture.scale[1] /= dscale;
                    };

                    			b->calcWindings();
                    	*/

                    // rotate
                    i = 0;
                    for (f = b->faces.p_next; f != &b->faces; f = f->p_next, i++)
                    {
                        LoopProblem("brush lock2");
                        w = f->w;
                        if (!w)
                            continue;

                        if (w->numpoints < 2)
                            continue;

                        if (saveWindings[i]->numpoints < 2)
                            continue;

                        pt1 = savePoints[i].p1;
                        pt2 = savePoints[i].p2;

                        ds = saveWindings[i]->points[pt2][3] - saveWindings[i]->points[pt1][3];
                        dt = saveWindings[i]->points[pt2][4] - saveWindings[i]->points[pt1][4];

                        o_ang = angle(ds,dt);
                        // rotation and scale account for ds dt vector...
                        ns = w->points[pt2][3] - w->points[pt1][3];
                        nt = w->points[pt2][4] - w->points[pt1][4];

                        n_ang = angle(ns,nt);

                        deltaAng = n_ang - o_ang;
                        //while (deltaAng < 0.0)
                        //	deltaAng += 2*M_PI;

                        deltaAng = RAD2DEG(deltaAng); // convert to a degrees...

                        //				if(f == b->currentFace) {
                        //					Caption("delta: %f  ==  %f  /  %f      ns:%f nt:%f",deltaAng, n_ang, o_ang,  ns, nt);
                        //				}
                        f->texture.rotate -= deltaAng;
//WORKHERE
                        while (f->texture.rotate < 0.0f)
                            f->texture.rotate += 360.0f;

                        while (f->texture.rotate > 360.0f)
                            f->texture.rotate -= 360.0f;


                    }

                    //	b->calcWindings();

                    //	Caption("current face rotate: %4f",b->currentFace->texture.rotate);

                    // scale
                    i = 0;
                    for (f = b->faces.p_next; f != &b->faces; f = f->p_next, i++)
                    {
                        LoopProblem("brush lock3");
                        if (!f->w)
                            continue;

                        w = f->w;
                        if (w->numpoints < 2)
                            continue;

                        if (saveWindings[i]->numpoints < 2)
                            continue;

                        float oscale, nscale, dscale;

                        pt1 = savePoints[i].p1;
                        pt2 = savePoints[i].p2;

                        ds = saveWindings[i]->points[pt2][3] - saveWindings[i]->points[pt1][3];
                        dt = saveWindings[i]->points[pt2][4] - saveWindings[i]->points[pt1][4];

                        // rotation and scale account for ds dt vector...
                        ns = w->points[pt2][3] - w->points[pt1][3];
                        nt = w->points[pt2][4] - w->points[pt1][4];

                        // original distance
                        oscale = sqrt(ds*ds + dt*dt);

                        nscale = sqrt(ns*ns + nt*nt);

                        if (!oscale)
                            oscale = 1.0;

                        dscale = nscale/oscale;
                        if (!dscale)
                            dscale = 1.0;

                        f->texture.scale[0] /= dscale;  // now we are centered...
                        f->texture.scale[1] /= dscale;
                    }
                    //	b->calcWindings();

                    // offsets
                    i = 0;
                    for (f = b->faces.p_next; f != &b->faces; f = f->p_next, i++)
                    {
                        LoopProblem("brush lock4");
                        if (!f->w)
                            continue;

                        w = f->w;
                        if (w->numpoints < 2)
                            continue;

                        if (saveWindings[i]->numpoints < 2)
                            continue;
                        // Now the fun part
                        // compare s, t values for first point...
                        // shift to fix that
                        // compute s, t vector to second point
                        // rotate if necessary
                        // compare length and scale if necessary

                        float orig_s, orig_t;
                        float new_s, new_t;
                        float s_shift, t_shift;

                        pt1 = savePoints[i].p1;
                        pt2 = savePoints[i].p2;

                        // find an average shift for the farthest separated points!
                        orig_s = (saveWindings[i]->points[pt1][3]+saveWindings[i]->points[pt2][3])/2.0;
                        orig_t = (saveWindings[i]->points[pt1][4]+saveWindings[i]->points[pt2][4])/2.0;

                        new_s  = (w->points[pt1][3]+w->points[pt2][3])/2.0;
                        new_t  = (w->points[pt1][4]+w->points[pt2][4])/2.0;

                        s_shift = new_s - orig_s;
                        t_shift = new_t - orig_t;

                        f->texture.shift[0] -= s_shift;  // now we are centered...
                        f->texture.shift[1] -= t_shift;

                    }

                }
                b->calcWindings();

                if (!set.disable_rotation_texlock && b && b->IsLocked())
                {
                    for (i = 0; i < numfaces; i++)
                    {
                        delete [] saveWindings[i];
                        saveWindings[i] = NULL;
                    }
                    delete[] saveWindings;
                    delete[] savePlanes;
                    delete[] savePoints;
                }
            }
        }
    }

    cleanUpInvalidObjects();

    current = saveCurrent;
}

void map::scaleBrush(float sx, float sy, float sz)
{
    SetBrush *b;
    Entity *e;
    face_t *f;
    vec3_t mins, maxs;
    vec3_t p1, p2;
    float X[4][4];
    int j, k;

    if (!numSelected())
    {
        //qprintf ("must have a single brush selected");
        return;
    }

    dirty = 1;

    getSelectedCenter();
    sb_ctr[0] = snapToGrid(sb_ctr[0]);
    sb_ctr[1] = snapToGrid(sb_ctr[1]);
    sb_ctr[2] = snapToGrid(sb_ctr[2]);

    scale3(sx, sy, sz, X);

    for (e = objects.p_next; e != &objects; e = e->p_next)
    {
        if (!e->modifiable)
        {
            // if not modifiable, just translate the origin...
            for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
            {
                if (!b->IsDrawable())
                    continue;
                if (!b->IsSelected()) // only rotate selected brushes...
                    continue;

                b->getMins(mins,maxs);

                for (k = 0; k < 3; k++)
                    p1[k] = (mins[k]+maxs[k])/2.0-sb_ctr[k]; // take first point, find dist to center

                transform(p1,X,p2); // translate it...

                for (k = 0; k < 3; k++)
                    sb_translate[k] = rint((p2[k]+sb_ctr[k])-(mins[k]+maxs[k])/2.0);  // move everything by the diff!

                b->translate();
            };
        }
        else
        {
            for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
            {
                if (!b->IsDrawable())
                    continue;
                if (!b->IsSelected()) // only rotate selected brushes...
                    continue;

                for (f = b->faces.p_next; f != &b->faces; f = f->p_next)
                {
                    LoopProblem("brush lock6");
                    for (j = 0; j < 3; j++)
                    {
                        for (k = 0; k < 3; k++)
                            p1[k] = f->planepts[j][k]-sb_ctr[k];

                        transform(p1,X,p2);

                        for (k = 0; k < 3; k++)
                            f->planepts[j][k] = rint(p2[k]+sb_ctr[k]);
                    }
                }

                b->calcWindings();
            }
        }
    }
    cleanUpInvalidObjects();
}

void map::makeRoom()
{
    SetBrush *b;
    vec3_t	mins, maxs;
    vec3_t	cmins[6], cmaxs[6];
    texturedef_t	td;

    if (numSelected() != 1)
    {
        //	qprintf ("must have a single brush selected");
        return;
    }

    int wallthickness = set.gridsize;

    if (set.query_wall_width)
    {
        char s[80];
        sprintf(s,"%i",wallthickness);

        if (InputDialog(client->hwnd, const_cast<char *> ("Set Wall Thickness"), const_cast<char *> ("Enter wall thickness (1-1024)..."), s, sizeof(s)).Execute() != IDOK)
            return;

        wallthickness = atoi(s);
    };

    if ( (wallthickness < 1) || (wallthickness > 1024) )
        return;

    dirty = 1;

    b = selectedBrush();
    int grp;
    grp = b->group;
    td = *b->texturedef();
    b->getMins(mins,maxs);
    b->remove();
    delete b;

    // do all six of them...
    // left...
    cmins[0][0] = mins[0]-wallthickness;
    cmins[0][1] = mins[1];
    cmins[0][2] = mins[2];
    cmaxs[0][0] = mins[0];
    cmaxs[0][1] = maxs[1];
    cmaxs[0][2] = maxs[2];
    // right
    cmins[1][0] = maxs[0];
    cmins[1][1] = mins[1];
    cmins[1][2] = mins[2];
    cmaxs[1][0] = maxs[0]+wallthickness;
    cmaxs[1][1] = maxs[1];
    cmaxs[1][2] = maxs[2];
    // front
    cmins[2][0] = mins[0];
    cmins[2][1] = mins[1]-wallthickness;
    cmins[2][2] = mins[2];
    cmaxs[2][0] = maxs[0];
    cmaxs[2][1] = mins[1];
    cmaxs[2][2] = maxs[2];
    // back;
    cmins[3][0] = mins[0];
    cmins[3][1] = maxs[1];
    cmins[3][2] = mins[2];
    cmaxs[3][0] = maxs[0];
    cmaxs[3][1] = maxs[1]+wallthickness;
    cmaxs[3][2] = maxs[2];
    // top
    cmins[4][0] = mins[0];
    cmins[4][1] = mins[1];
    cmins[4][2] = maxs[2];
    cmaxs[4][0] = maxs[0];
    cmaxs[4][1] = maxs[1];
    cmaxs[4][2] = maxs[2]+wallthickness;
    // bottom
    cmins[5][0] = mins[0];
    cmins[5][1] = mins[1];
    cmins[5][2] = mins[2]-wallthickness;
    cmaxs[5][0] = maxs[0];
    cmaxs[5][1] = maxs[1];
    cmaxs[5][2] = mins[2];

    for (int i = 0; i < 6; i++)
    {
        b = new SetBrush();
        b->group = grp;
        b->initOwner(world,cmins[i],cmaxs[i],&td);
        world->addObject(b);
        b->setSelected(true);
    }
    cleanUpInvalidObjects();
}

void map::extrudeRoom()
{
    SetBrush *b, *old;
    face_t *face;
    vec3_t normal;
    int i, j, k;
    vec3_t	mins, maxs;
    texturedef_t	td;

    if (numSelected() != 1)
    {
        //	qprintf ("must have a single brush selected");
        return;
    }

    int wallthickness = set.gridsize;

    if (set.query_wall_width)
    {
        char s[80];
        sprintf(s,"%i",wallthickness);

        if (InputDialog(client->hwnd, const_cast<char *> ("Set Wall Thickness"), const_cast<char *> ("Enter wall thickness (1-1024)..."), s, sizeof(s)).Execute() != IDOK)
            return;

        wallthickness = atoi(s);
    }

    if (wallthickness < 1 || wallthickness > 1024)
        return;

    dirty = 1;

    old = selectedBrush();
    int grp;
    grp = old->group;
    td = *old->texturedef();
    old->getMins(mins,maxs);
    old->remove();
    old->calcWindings(); // just in case...

    if (!old->IsInvalid())
    {
        for (face = old->faces.p_next; face != &old->faces; face = face->p_next)
        {
            LoopProblem("brush extrude");
            if (!face->w)
                continue;

            b = new SetBrush();
            b->group = grp;
            //b->numfaces = 2 + face->w->numpoints;

            normal[0] = face->plane.normal[0];
            normal[1] = face->plane.normal[1];
            normal[2] = face->plane.normal[2];

            face_t *f = new face_t;
            memset(f, 0, sizeof(face_t));

            VectorCopy(face->w->points[0], f->planepts[0]);
            VectorCopy(face->w->points[1], f->planepts[2]); // Swap order
            VectorCopy(face->w->points[2], f->planepts[1]);

            // close

            b->addObject(f);

            f = new face_t;
            memset(f, 0, sizeof(face_t));

            for (i = 0; i < 3; i++)
            {
                f->planepts[0][i] = rint(face->w->points[0][i]+wallthickness*normal[i]);
                f->planepts[1][i] = rint(face->w->points[1][i]+wallthickness*normal[i]);
                f->planepts[2][i] = rint(face->w->points[2][i]+wallthickness*normal[i]);
            }

            b->addObject(f);

            // K is always 1 behind j;
            k = face->w->numpoints-1;
            for (j = 0; j < face->w->numpoints; j++)
            {
                f = new face_t;
                memset(f, 0, sizeof(face_t));

                for (i = 0; i < 3; i++)
                {
                    f->planepts[0][i] = face->w->points[k][i];
                    f->planepts[1][i] = face->w->points[j][i];
                    f->planepts[2][i] = rint(face->w->points[j][i]+wallthickness*normal[i]);
                }

                k = j;

                b->addObject(f);
            };

            // use windings...
            b->setTexturedef(&td); // calculates the windings...

            b->setParent(world);
            world->addObject(b);
            b->setSelected(true);
        }
    }
    delete old;

    cleanUpInvalidObjects();
}

void map::extrudeFace()
{
    face_t *face;
    vec3_t normal;
    int i, j, k;

    if (numSelected() != 1)
        return;

    int wallthickness = !set.extrude_depth ? set.gridsize : set.extrude_depth;

    dirty = 1;

    SetBrush *old = selectedBrush();
    int grp = old->group;
    texturedef_t *td = old->texturedef();

    face = old->currentFace;

    old->deselect();

    if (!old->IsInvalid() && face && face->w)
    {
        SetBrush *b = new SetBrush();
        b->group = grp;
        //b->numfaces = 2 + face->w->numpoints;

        normal[0] = face->plane.normal[0];
        normal[1] = face->plane.normal[1];
        normal[2] = face->plane.normal[2];

        // copy and flip source face

        face_t *f = new face_t;
        memset(f, 0, sizeof(face_t));

        VectorCopy(face->w->points[0], f->planepts[0]);
        VectorCopy(face->w->points[1], f->planepts[2]); // Swap order
        VectorCopy(face->w->points[2], f->planepts[1]);
        //make plane points int
        for(j=0; j<3; j++)
        {
            f->planepts[j][0] = rint(f->planepts[j][0]);
            f->planepts[j][1] = rint(f->planepts[j][1]);
            f->planepts[j][2] = rint(f->planepts[j][2]);
        }

        b->addObject(f);

        // close

        f = new face_t;
        memset(f, 0, sizeof(face_t));

        for (i = 0; i < 3; i++)
        {
            f->planepts[0][i] = rint(face->w->points[0][i]+wallthickness*normal[i]);
            f->planepts[1][i] = rint(face->w->points[1][i]+wallthickness*normal[i]);
            f->planepts[2][i] = rint(face->w->points[2][i]+wallthickness*normal[i]);
        }

        b->addObject(f);
        b->currentFace = f;

        // K is always 1 behind j;
        k = face->w->numpoints-1;
        for (j = 0; j < face->w->numpoints; j++)
        {
            f = new face_t;
            memset(f, 0, sizeof(face_t));

            for (i = 0; i < 3; i++)
            {
                f->planepts[0][i] = rint(face->w->points[k][i]);
                f->planepts[1][i] = rint(face->w->points[j][i]);
                f->planepts[2][i] = rint(face->w->points[j][i]+wallthickness*normal[i]);
            }

            k = j;

            b->addObject(f);
        }

        // use windings...
        b->setTexturedef(td); // calculates the windings...

        b->setParent(world);
        world->addObject(b);
        b->setSelected(true);
    }
    cleanUpInvalidObjects();
}


void map::tallBrush()
{
    SetBrush *b;
    vec3_t	mins, maxs;
    texturedef_t	td;

    if (numSelected() != 1)
        return;

    dirty = 1;

    b = selectedBrush();
    int grp;
    grp = b->group;
    td = *b->texturedef();
    b->getMins(mins,maxs);
    b->remove();
    delete b;

    mins[2] = MAP_MINZ;
    maxs[2] = MAP_MAXZ;

    b = new SetBrush();
    b->group = grp;
    b->initOwner(world,mins,maxs,&td);
    world->addObject(b);
    b->setSelected(true);
}

void map::shortBrush()
{
    SetBrush *b;
    vec3_t	mins, maxs;
    texturedef_t	td;

    if (numSelected() != 1)
        return;

    dirty = 1;

    b = selectedBrush();
    int grp;
    grp = b->group;
    td = *b->texturedef();
    b->getMins(mins,maxs);
    b->remove();
    delete b;

    mins[2] = 0;
    maxs[2] = 16;

    b = new SetBrush();
    b->group = grp;
    b->initOwner(world,mins,maxs,&td);
    world->addObject(b);
    b->setSelected(true);
}

void map::sizeBrush(int height)
{
    SetBrush *b;
    vec3_t	mins, maxs;
    texturedef_t	td;

    if (numSelected() != 1)
        return;

    dirty = 1;

    b = selectedBrush();
    int grp;
    grp = b->group;
    td = *b->texturedef();
    b->getMins(mins,maxs);
    b->remove();
    delete b;

    maxs[2] = mins[2]+height;

    b = new SetBrush();
    b->group = grp;
    b->initOwner(world,mins,maxs,&td);
    world->addObject(b);
    b->setSelected(true);
}

/*
==================
subtractSelection
==================
*/
void map::subtractSelection()
{
    if (numSelected() < 1)
        return;

    SetBrush *o, *o2;
    SetBrush *n, *n2;
    Entity *sellist, *sourcelist;

    sourcelist = new Entity;
    sellist    = new Entity;
    carve_in   = new Entity;
    carve_out  = new Entity;

    dirty = 1;

    for (o = current->objects.p_next; o && (o != &current->objects); o = n)
    {
        n = o->p_next;
        current->removeObject(o);
        if (o->IsSelected())
            sellist->addObject(o);
        else
            sourcelist->addObject(o);
    }

    HDC hdc = GetDC(editWindow->hwnd);
    char outstr[80];
    int len;

    HGDIOBJ oldfont = SelectObject(hdc,set.font12);
    SetTextColor(hdc, set.color_foreground);
    SetBkColor(hdc, set.color_background);

    int c = sellist->count;
    int i = 1;
    for (o = sellist->objects.p_next; o && (o != &sellist->objects); o = n)
    {
        n = o->p_next;
        len = sprintf(outstr,"[Subtracting %i/%i]   ", i++, c);
        TextOut(hdc, 2, 2, outstr, len);
        if (o->IsInvalid())    // skip and delete if invalid?? should never happen...
        {
            sellist->removeObject(o);
            delete o;
            continue;
        }

        o->setCarveVars();

        int c2 = sourcelist->count;
        int j = 0;
        for (o2 = sourcelist->objects.p_next; o2 && (o2 != &sourcelist->objects); o2 = n2)
        {
            n2 = o2->p_next;
            j++;
            if (!(j % 5))
            {
                len = sprintf(outstr,"Cutting [%i/%i]     ",j,c2);
                TextOut(hdc, 2, 22, outstr, len);
            }

            sourcelist->removeObject(o2);

            if (o2->IsInvalid())
            {
                delete o2;
                continue;
            }

            carve(&o2);

            carve_in->freeObjects(true);
        }

        delete sourcelist;  // the individual have been moved/freed
        sourcelist = carve_out;
        carve_out = new Entity;
    }

    SelectObject(hdc,oldfont);
    ReleaseDC(editWindow->hwnd,hdc);

    // add the selection back to the remnants
    current->empty();
    current->appendList(sourcelist);
    current->appendList(sellist);

    // clear them out, but don't delete attached objects, if any...
    sourcelist->empty();
    sellist->empty();
    carve_in->empty();
    carve_out->empty();

    delete sourcelist;
    delete sellist;
    delete carve_in;
    delete carve_out;

    carve_in  = NULL;
    carve_out = NULL;

    if (!current->count)
    {
        removeObject(current);
        delete current;
        current = world;
    }
}

//
// Revolve it...
void map::makeSphere(int /*type*/, int hstrips, int vstrips, int neardist, int fardist, int take, int percent)
{
    int i,j,k,newnov;
    float *c,*s;
    int *index1, *index2;
    vec3_t *verts;
    vec3_t p[4];
    vec3_t q[4];
    vec3_t ctr;
    vec3_t *v;
    texturedef_t td;
    SetBrush *b;
    vec3_t mins, maxs;

    if (numSelected() != 1)
        return;

    float fd = (float)fardist;
    float nd = (float)neardist;

    dirty = 1;

    b = selectedBrush();
    int grp;
    grp = b->group;
    td = *b->texturedef();
    b->getMins(mins,maxs);
    b->remove();
    delete b;

    vec3_t sz;
    float smallest = 9999.0f;
    for (i = 0; i < 3; i++)
    {
        ctr[i] = (int)((mins[i]+maxs[i])/2.0);
        if (take)
        {
            sz[i] = (int)(maxs[i]-mins[i]);
            if (sz[i] < smallest)
                smallest = sz[i];

        }
    }

    if (take)
    {
        fd = (smallest/2.0f);
        nd = ((float)fardist * ((float)percent/100.0));
    }

    float t1 = 2*M_PI/(float)(2*vstrips);

    fd = fd/cos(t1);
    nd = nd/cos(t1);

    if (nd < 1.0)
        nd = 1.0;

    if (fd <= nd)
        fd = nd+1.0f;

    int nhoriz = hstrips;
    int nverts = vstrips+1;

    nhoriz = min(100,nhoriz);
    nhoriz = max(2,nhoriz);

    nverts = min(100,nverts);
    nverts = max(2,nverts);

    float theta = -M_PI*0.5;
    float thetadiff = M_PI/(nverts-1);

    v = new vec3_t[nverts];

    for (i = 0; i < nverts; i++)
    {
        v[i][0] = cos(theta);
        v[i][1] = sin(theta);
        theta += thetadiff;
    };

    theta = 0.0;
    thetadiff = 2*M_PI/nhoriz;

    c = new float[100];
    s = new float[100];
    index1 = new int[101];
    index2 = new int[101];
    verts = new vec3_t[1024];

    for (i = 0; i < nhoriz; i++)
    {
        c[i] = cos(theta);
        s[i] = sin(theta);
        theta += thetadiff;
    }

    newnov = 0;
    // newnov = nov; first new vertex index...

    if (fabs(v[0][0]) < F_EPSILON)
    {
        // create one point at top
        // at 0.0, ctr.y, 0.0
        //
        verts[newnov][0] = 0.0;
        verts[newnov][1] = v[0][1];
        verts[newnov][2] = 0.0;

        for (i = 0; i <= nhoriz; i++)
        {
            index1[i] = newnov;
        };
        newnov++;
    }
    else
    {
        for (i = 0; i < nhoriz; i++)
        {
            verts[newnov][0] = v[0][0]*c[i];
            verts[newnov][1] = v[0][1];
            verts[newnov][2] = -v[0][0]*s[i];
            index1[i] = newnov;
            newnov++;
        }
        index1[nhoriz] = index1[0];
    }

    for (j = 1; j < nverts; j++)
    {
        if (fabs(v[j][0]) < F_EPSILON)
        {
            verts[newnov][0] = 0.0;
            verts[newnov][1] = v[j][1];
            verts[newnov][2] = 0.0;

            for (i = 0; i <= nhoriz; i++)
            {
                index2[i] = newnov;
            }
            newnov++;
        }
        else
        {
            for (i = 0; i < nhoriz; i++)
            {
                verts[newnov][0] = v[j][0]*c[i];
                verts[newnov][1] = v[j][1];
                verts[newnov][2] = -v[j][0]*s[i];
                index2[i] = newnov;
                newnov++;
            }
            index2[nhoriz] = index2[0];
        }
        if (index1[0] != index1[1])
            if (index2[0] == index2[1])
            {
                for (i = 0; i < nhoriz; i++)
                {
                    // three sided facet
                    // create full brush...
                    b = new SetBrush();
                    b->group = grp;

                    for (k = 0; k < 3; k++)
                    {
                        p[0][k] = rint(ctr[k] + (nd*verts[index1[i+1]][k]));
                        p[1][k] = rint(ctr[k] + (nd*verts[index2[i]][k]));
                        p[2][k] = rint(ctr[k] + (nd*verts[index1[i]][k]));

                        q[0][k] = rint(ctr[k] + (fd*verts[index1[i+1]][k]));
                        q[1][k] = rint(ctr[k] + (fd*verts[index2[i]][k]));
                        q[2][k] = rint(ctr[k] + (fd*verts[index1[i]][k]));
                    }

                    // nearface
                    face_t *f;

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));

                    VectorCopy(p[0],f->planepts[0]);
                    VectorCopy(p[1],f->planepts[1]);
                    VectorCopy(p[2],f->planepts[2]);

                    b->addObject(f);

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));

                    // far face...
                    VectorCopy(q[1],f->planepts[0]);
                    VectorCopy(q[0],f->planepts[1]);
                    VectorCopy(q[2],f->planepts[2]);

                    b->addObject(f);

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));

                    // left face
                    VectorCopy(p[2],f->planepts[0]);
                    VectorCopy(p[1],f->planepts[1]);
                    VectorCopy(q[1],f->planepts[2]);

                    b->addObject(f);

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));

                    VectorCopy(p[1],f->planepts[0]);
                    VectorCopy(p[0],f->planepts[1]);
                    VectorCopy(q[0],f->planepts[2]);

                    b->addObject(f);

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));

                    VectorCopy(p[0],f->planepts[0]);
                    VectorCopy(p[2],f->planepts[1]);
                    VectorCopy(q[2],f->planepts[2]);

                    b->addObject(f);

                    b->setTexturedef(&td);

                    b->setParent(world);
                    world->addObject(b);
                    b->setSelected(true);
                };
            }
            else
            {
                for (i = 0; i < nhoriz; i++)
                {
                    // create four sided facets...

                    b = new SetBrush();
                    b->group = grp;

                    for (k = 0; k < 3; k++)
                    {
                        p[0][k] = rint(ctr[k] + (nd*verts[index1[i+1]][k]));
                        p[1][k] = rint(ctr[k] + (nd*verts[index2[i+1]][k]));
                        p[2][k] = rint(ctr[k] + (nd*verts[index2[i]][k]));
                        p[3][k] = rint(ctr[k] + (nd*verts[index1[i]][k]));

                        q[0][k] = rint(ctr[k] + (fd*verts[index1[i+1]][k]));
                        q[1][k] = rint(ctr[k] + (fd*verts[index2[i+1]][k]));
                        q[2][k] = rint(ctr[k] + (fd*verts[index2[i]][k]));
                        q[3][k] = rint(ctr[k] + (fd*verts[index1[i]][k]));
                    };

                    face_t *f;

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));
                    // nearface
                    VectorCopy(p[0],f->planepts[0]);
                    VectorCopy(p[1],f->planepts[1]);
                    VectorCopy(p[2],f->planepts[2]);

                    b->addObject(f);

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));
                    // farface
                    VectorCopy(q[1],f->planepts[0]);
                    VectorCopy(q[0],f->planepts[1]);
                    VectorCopy(q[2],f->planepts[2]);

                    b->addObject(f);

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));
                    VectorCopy(p[1],f->planepts[0]);
                    VectorCopy(p[2],f->planepts[1]);
                    VectorCopy(ctr,f->planepts[2]);

                    b->addObject(f);

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));
                    VectorCopy(p[2],f->planepts[0]);
                    VectorCopy(p[3],f->planepts[1]);
                    VectorCopy(ctr,f->planepts[2]);

                    b->addObject(f);

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));
                    VectorCopy(p[3],f->planepts[0]);
                    VectorCopy(p[0],f->planepts[1]);
                    VectorCopy(ctr,f->planepts[2]);

                    b->addObject(f);

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));
                    VectorCopy(p[0],f->planepts[0]);
                    VectorCopy(p[1],f->planepts[1]);
                    VectorCopy(ctr,f->planepts[2]);

                    b->addObject(f);

                    b->setTexturedef(&td);

                    b->setParent(world);
                    world->addObject(b);
                    b->setSelected(true);
                };
            }
        else if (index2[0] != index2[1])
            for (i = 0; i < nhoriz; i++)
            {
                // three sided facet
                // create full brush...
                b = new SetBrush();
                b->group = grp;

                for (k = 0; k < 3; k++)
                {
                    p[0][k] = rint(ctr[k] + (nd*verts[index2[i+1]][k]));
                    p[1][k] = rint(ctr[k] + (nd*verts[index2[i]][k]));
                    p[2][k] = rint(ctr[k] + (nd*verts[index1[i]][k]));

                    q[0][k] = rint(ctr[k] + (fd*verts[index2[i+1]][k]));
                    q[1][k] = rint(ctr[k] + (fd*verts[index2[i]][k]));
                    q[2][k] = rint(ctr[k] + (fd*verts[index1[i]][k]));
                };

                face_t *f;

                f = new face_t;
                memset(f, 0, sizeof(face_t));

                // nearface
                VectorCopy(p[0],f->planepts[0]);
                VectorCopy(p[1],f->planepts[1]);
                VectorCopy(p[2],f->planepts[2]);

                b->addObject(f);

                f = new face_t;
                memset(f, 0, sizeof(face_t));
                // far face...
                VectorCopy(q[1],f->planepts[0]);
                VectorCopy(q[0],f->planepts[1]);
                VectorCopy(q[2],f->planepts[2]);

                b->addObject(f);

                f = new face_t;
                memset(f, 0, sizeof(face_t));
                // left face
                VectorCopy(p[2],f->planepts[0]);
                VectorCopy(p[1],f->planepts[1]);
                VectorCopy(q[1],f->planepts[2]);

                b->addObject(f);

                f = new face_t;
                memset(f, 0, sizeof(face_t));
                VectorCopy(p[1],f->planepts[0]);
                VectorCopy(p[0],f->planepts[1]);
                VectorCopy(q[0],f->planepts[2]);

                b->addObject(f);

                f = new face_t;
                memset(f, 0, sizeof(face_t));
                VectorCopy(p[0],f->planepts[0]);
                VectorCopy(p[2],f->planepts[1]);
                VectorCopy(q[2],f->planepts[2]);

                b->addObject(f);

                b->setTexturedef(&td);

                b->setParent(world);
                world->addObject(b);
                b->setSelected(true);
            };
        for (i = 0; i <= nhoriz; i++)
            index1[i] = index2[i];
    }

    delete [] v;
    delete [] verts;
    delete [] index2;
    delete [] index1;
    delete [] s;
    delete [] c;

    cleanUpInvalidObjects();
}

void map::makeCylinder(int sides, int strips, int neardist, int fardist, int take, int percent, int xalign)
{
    SetBrush *b;

    int i,j;

    vec3_t	mins, maxs;
    texturedef_t td;

    float ang;
    float ctrx, ctry;

    float startz;
    float zstep;

    float nearfacex, nearfacey;
    float farfacex, farfacey;
    float nearleftx, nearlefty;
    float farleftx, farlefty;

    float astep;

    if (numSelected() != 1)
        return;

    dirty = 1;

    // leave the brush???
    b = selectedBrush();
    int grp;
    grp = b->group;
    td = *b->texturedef();
    b->getMins(mins,maxs);
    b->remove();
    delete b;

    if (sides<3)
        sides = 3;

    sides = min(32,sides);

    strips = max(1,strips);

    float szx, szy;
    szx = rint(maxs[0] - mins[0]);
    szy = rint(maxs[1] - mins[1]);
    float trad;
    trad = min(szx,szy);
    if (take)
    {
        fardist = (int)(trad/2.0f);
        neardist = (int)((float)fardist * (float)percent/100.0);
    }
    // error checking...
    float nd, fd;
    float t1;
    t1 = 2*M_PI/(float)(2*sides);

    nd = (float)max(2,neardist);
    fd = (float)max(neardist+2,fardist);

    nd = nd/cos(t1);
    fd = fd/cos(t1);

    ctrx = rint((maxs[0] + mins[0])/2.0);
    ctry = rint((maxs[1] + mins[1])/2.0);

    startz = snapToGrid(mins[2]);
    zstep = rint((maxs[2]-mins[2])/(float)strips);

    ang = 2*M_PI;
    astep = ang/sides;

    for (i = 0; i < strips; i++)
    {
        ang = 0.0;
        if (!xalign)
        {
            ang += M_PI/sides;
        }

        for (j = 0; j < sides; j++)
        {

            nearfacex = rint(ctrx+nd*sin(ang));
            nearfacey = rint(ctry+nd*cos(ang));

            farfacex = rint(ctrx+fd*sin(ang));
            farfacey = rint(ctry+fd*cos(ang));

            nearleftx = rint(ctrx+nd*sin(ang+astep));
            nearlefty = rint(ctry+nd*cos(ang+astep));

            farleftx = rint(ctrx+fd*sin(ang+astep));
            farlefty = rint(ctry+fd*cos(ang+astep));

            b = new SetBrush();
            b->group = grp;

            face_t *f;
            f = new face_t;
            memset(f, 0, sizeof(face_t));

            // nearface
            f->planepts[0][0] = nearfacex;
            f->planepts[0][1] = nearfacey;
            f->planepts[0][2] = startz+(i+1)*zstep;

            f->planepts[1][0] = nearleftx;
            f->planepts[1][1] = nearlefty;
            f->planepts[1][2] = startz+(i+1)*zstep;

            f->planepts[2][0] = nearleftx;
            f->planepts[2][1] = nearlefty;
            f->planepts[2][2] = startz+(i)*zstep;

            b->addObject(f);

            f = new face_t;
            memset(f, 0, sizeof(face_t));

            // far
            f->planepts[0][0] = farleftx;
            f->planepts[0][1] = farlefty;
            f->planepts[0][2] = startz+(i+1)*zstep;

            f->planepts[1][0] = farfacex;
            f->planepts[1][1] = farfacey;
            f->planepts[1][2] = startz+(i+1)*zstep;

            f->planepts[2][0] = farfacex;
            f->planepts[2][1] = farfacey;
            f->planepts[2][2] = startz+(i)*zstep;

            b->addObject(f);

            f = new face_t;
            memset(f, 0, sizeof(face_t));

            // top
            f->planepts[0][0] = nearfacex;
            f->planepts[0][1] = nearfacey;
            f->planepts[0][2] = startz+(i+1)*zstep;

            f->planepts[1][0] = farfacex;
            f->planepts[1][1] = farfacey;
            f->planepts[1][2] = startz+(i+1)*zstep;

            f->planepts[2][0] = farleftx;
            f->planepts[2][1] = farlefty;
            f->planepts[2][2] = startz+(i+1)*zstep;

            b->addObject(f);

            f = new face_t;
            memset(f, 0, sizeof(face_t));

            // bottom
            f->planepts[0][0] = farleftx;
            f->planepts[0][1] = farlefty;
            f->planepts[0][2] = startz+(i)*zstep;

            f->planepts[1][0] = farfacex;
            f->planepts[1][1] = farfacey;
            f->planepts[1][2] = startz+(i)*zstep;;

            f->planepts[2][0] = nearfacex;
            f->planepts[2][1] = nearfacey;
            f->planepts[2][2] = startz+(i)*zstep;

            b->addObject(f);

            f = new face_t;
            memset(f, 0, sizeof(face_t));

            // left
            f->planepts[0][0] = farfacex;
            f->planepts[0][1] = farfacey;
            f->planepts[0][2] = startz+(i+1)*zstep;

            f->planepts[1][0] = nearfacex;
            f->planepts[1][1] = nearfacey;
            f->planepts[1][2] = startz+(i+1)*zstep;

            f->planepts[2][0] = nearfacex;
            f->planepts[2][1] = nearfacey;
            f->planepts[2][2] = startz+(i)*zstep;

            b->addObject(f);

            f = new face_t;
            memset(f, 0, sizeof(face_t));

            // right
            f->planepts[0][0] = nearleftx;
            f->planepts[0][1] = nearlefty;
            f->planepts[0][2] = startz+(i+1)*zstep;

            f->planepts[1][0] = farleftx;
            f->planepts[1][1] = farlefty;
            f->planepts[1][2] = startz+(i+1)*zstep;

            f->planepts[2][0] = farleftx;
            f->planepts[2][1] = farlefty;
            f->planepts[2][2] = startz+(i)*zstep;

            b->addObject(f);

            b->setTexturedef(&td);
            b->setParent(world);
            world->addObject(b);
            b->setSelected(true);
            ang += astep;  // current angle...
        }
    }
    cleanUpInvalidObjects();
}

void map::makeNSided(int sides, int radius, int xalign, int take)
{
    SetBrush *b;
    int i;
    vec3_t	mins, maxs;
    texturedef_t td;
    float r;
    float ang;
    float ctrx, ctry;
    float nearfacex, nearfacey;
    float nearleftx, nearlefty;
    float astep;

    if (numSelected() != 1)
        return;

    dirty = 1;

    // leave the brush???
    b = selectedBrush();
    int grp = b->group;
    td = *b->texturedef();
    b->getMins(mins,maxs);
    b->remove();
    delete b;

    if (sides<3)
        sides = 3;

    sides = min(32,sides);

    float szx, szy;
    szx = (maxs[0] - mins[0]);
    szy = (maxs[1] - mins[1]);
    float trad;
    trad = min(szx,szy);
    float t1;
    t1 = 2*M_PI/(float)(2*sides);
    if (take)
    {
        trad /= 2.0f;
        r = (trad / cos(t1));
    }
    else
    {
        ;
        r = ((float)radius / cos(t1));
    }

    r = max(4.0f,r);

    ctrx = rint((maxs[0] + mins[0])/2.0f);
    ctry = rint((maxs[1] + mins[1])/2.0f);

    ang = 2*M_PI;
    astep = ang/sides;

    // add in the top and the bottom...
    b = new SetBrush();
    b->group = grp;

    face_t *f;

    f = new face_t;
    memset(f, 0, sizeof(face_t));

    // top
    f->planepts[0][0] = ctrx;
    f->planepts[0][1] = ctry;
    f->planepts[0][2] = maxs[2];

    f->planepts[1][0] = ctrx;
    f->planepts[1][1] = ctry+16.0;
    f->planepts[1][2] = maxs[2];

    f->planepts[2][0] = ctrx+16.0;
    f->planepts[2][1] = ctry;
    f->planepts[2][2] = maxs[2];

    b->addObject(f);

    f = new face_t;
    memset(f, 0, sizeof(face_t));

    //bottom
    f->planepts[0][0] = ctrx;
    f->planepts[0][1] = ctry;
    f->planepts[0][2] = mins[2];

    f->planepts[1][0] = ctrx+16.0;
    f->planepts[1][1] = ctry;
    f->planepts[1][2] = mins[2];

    f->planepts[2][0] = ctrx;
    f->planepts[2][1] = ctry+16.0;
    f->planepts[2][2] = mins[2];

    b->addObject(f);

    ang = 0.0;
    if (!xalign)
    {
        ang += M_PI/sides;
    }
    for (i = 2; i < (sides+2); i++)
    {

        f = new face_t;
        memset(f, 0, sizeof(face_t));

        nearfacex = rint(ctrx+r*sin(ang));
        nearfacey = rint(ctry+r*cos(ang));

        nearleftx = rint(ctrx+r*sin(ang+astep));
        nearlefty = rint(ctry+r*cos(ang+astep));

        // nearface
        f->planepts[0][0] = nearleftx;
        f->planepts[0][1] = nearlefty;
        f->planepts[0][2] = maxs[2];

        f->planepts[1][0] = nearfacex;
        f->planepts[1][1] = nearfacey;
        f->planepts[1][2] = maxs[2];

        f->planepts[2][0] = nearfacex;
        f->planepts[2][1] = nearfacey;
        f->planepts[2][2] = mins[2];

        b->addObject(f);

        ang += astep;
    }

    b->setTexturedef(&td);
    b->setParent(world);
    world->addObject(b);
    b->setSelected(true);
    cleanUpInvalidObjects();
}

void map::makePyramid(int sides, int radius, int height, int xalign, int take)
{
    SetBrush *b;
    int i;
    vec3_t	mins, maxs;
    texturedef_t td;
    float ang;
    float ctrx, ctry;
    float nearfacex, nearfacey;
    float nearleftx, nearlefty;
    float astep;

    if (numSelected() != 1)
        return;

    dirty = 1;

    // leave the brush???
    b = selectedBrush();
    int grp;
    grp = b->group;
    td = *b->texturedef();
    b->getMins(mins,maxs);
    b->remove();
    delete b;

    if (sides<3)
        sides = 3;

    sides = min(32,sides);
    float szx, szy, szz;
    szx = (maxs[0] - mins[0]);
    szy = (maxs[1] - mins[1]);
    szz = (maxs[2] - mins[2]);
    float trad;
    trad = min(szx,szy);
    float rad;

    rad = (float)radius;
    if (take)
    {
        rad = trad/2.0f;
        height = (int)szz;
    }

    rad = max(4.0f,rad);

    float t1;
    t1 = 2*M_PI/(float)(2*sides);
    rad = rad/cos(t1);

    height = max(4, height);

    ctrx = rint((maxs[0] + mins[0])/2.0);
    ctry = rint((maxs[1] + mins[1])/2.0);

    float topz = (int)(mins[2]+height);

    ang = 2*M_PI;
    astep = ang/sides;

    // add in the top and the bottom...
    b = new SetBrush();
    b->group = grp;
    //b->numfaces = (1+sides);

    face_t *f;

    f = new face_t;
    memset(f, 0, sizeof(face_t));

    //bottom
    f->planepts[0][0] = ctrx;
    f->planepts[0][1] = ctry;
    f->planepts[0][2] = mins[2];

    f->planepts[1][0] = ctrx+16.0;
    f->planepts[1][1] = ctry;
    f->planepts[1][2] = mins[2];

    f->planepts[2][0] = ctrx;
    f->planepts[2][1] = ctry+16.0;
    f->planepts[2][2] = mins[2];

    b->addObject(f);

    ang = 0.0;
    if (!xalign)
    {
        ang += M_PI/sides;
    }
    for (i = 1; i < (sides+1); i++)
    {
        nearfacex = rint(ctrx+rad*sin(ang));
        nearfacey = rint(ctry+rad*cos(ang));

        nearleftx = rint(ctrx+rad*sin(ang+astep));
        nearlefty = rint(ctry+rad*cos(ang+astep));

        f = new face_t;
        memset(f, 0, sizeof(face_t));

        // nearface
        f->planepts[0][0] = nearfacex;
        f->planepts[0][1] = nearfacey;
        f->planepts[0][2] = mins[2];

        f->planepts[1][0] = nearleftx;
        f->planepts[1][1] = nearlefty;
        f->planepts[1][2] = mins[2];

        f->planepts[2][0] = ctrx;
        f->planepts[2][1] = ctry;
        f->planepts[2][2] = topz;

        b->addObject(f);

        ang += astep;
    }

    b->setTexturedef(&td);
    b->setParent(world);
    world->addObject(b);
    b->setSelected(true);

    cleanUpInvalidObjects();
}

void map::replaceTexture(char *old_tex, char *new_tex)
{
    char fromBase[64];
    char toBase[64];
    char fromTex[64];
    char toTex[64];

    memset(fromBase,0,64);
    memset(toBase,0,64);
    memset(fromTex,0,64);
    memset(toTex,0,64);

    if (set.game_mode == 2)
    {
        SplitBaseQuiet(old_tex,fromBase,fromTex);
        SplitBaseQuiet(new_tex,toBase  ,  toTex);

        // Remove .wal if some bozo put it in...
        StripExtension(fromTex);
        StripExtension(toTex);
    }
    else
    {
        STRNCPY(fromTex,old_tex);
        STRNCPY(toTex,new_tex);
    }

    Entity   *e;
    SetBrush *b;
    for (e = objects.p_next; e != &objects; e = e->p_next)
    {
        if (!e->modifiable)
            continue;

        // if not modifiable, just translate the origin...
        for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
        {
            if (!b->IsDrawable())
                continue;

            int changed = 0;
            face_t *f;
            for (f = b->faces.p_next; f != &b->faces; f = f->p_next)
            {
                LoopProblem("replace texture");
                if (set.game_mode == 2)
                {
                    if (!strcmpi(f->texture.texture,fromTex) && !strcmpi(f->texture.basepath,fromBase))
                    {
                        changed = 1;
                        STRNCPY(f->texture.texture,toTex);
                        STRNCPY(f->texture.basepath,toBase);
                    }
                }
                else
                {
                    if (!strcmpi(f->texture.texture,fromTex))
                    {
                        changed = 1;
                        STRNCPY(f->texture.texture,toTex);
                    }
                }
                delete f->qtexture;
                f->qtexture = 0;
            }

            if (changed)
            {
                // recache textures...
                b->calcWindings();
            }
        }
    }
}

void map::selectByEntityKeyValue(bool useE, bool useK, bool useV,
                                 char *ent, char *key, char *val)
{
    Entity   *e;
    SetBrush *b;
    char *cl;
    bool selectIt;

    if (!useE && !useK && !useV)  // nothing to look for...
        return;

    for (e = objects.p_next; e != &objects; e = e->p_next)
    {
        // if not modifiable, just translate the origin...
        selectIt = false;
        if (useE)
        {
            cl = e->classname;
            if (cl && cl[0] && !strcmpi(cl,ent))
                selectIt = true;
        }

        if (useK)
        {
            cl = e->valueForQKey(key);
            if (cl && cl[0])
            {
                selectIt = true;
            }
            else
            {
                selectIt = false;
            }
        }

        if (useV)
        {
            if (useK)
            {
                cl = e->valueForQKey(key);
                if (cl && cl[0] && !strcmpi(cl,val))
                {
                    selectIt = true;
                }
                else
                {
                    selectIt = false;
                }
            }
            else
            {
                epair_t *ep;
                for (ep=e->epairs ; ep ; ep=ep->next)
                {
                    if (!strcmpi(ep->value,val))
                    {
                        selectIt = true;
                        break;
                    };
                };
            }
        };

        if (!selectIt)
            continue;

        if (current != e)
            setCurrentEntity(e);

        // Select Everything...
        for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
        {
            if (!b->IsDrawable())
                continue;

            if (b->IsSelected()) // already selected...
                continue;

            b->setSelected(true);
        };
    };
}

void map::selectByTexture(char *buffer)
{
    Entity   *e;
    SetBrush *b;

    char texName[64];
    char texPath[64];

    if (set.game_mode == 2)
    {
        SplitBaseQuiet(buffer,texPath,texName);
        if (!texPath[0] || !texName[0])
        {
            MessageBox(0,"Select by Texture","Empty Texture Name or Path?",MB_OK | MB_ICONEXCLAMATION);
            return;
        }
    }
    else
    {
        STRNCPY(texName,buffer);
        *texPath = 0;
    }

    for (e = objects.p_next; e != &objects; e = e->p_next)
    {
        if (!e->modifiable)
            continue;

        // if not modifiable, just translate the origin...
        for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
        {
            if (!b->IsDrawable())
                continue;

            if (b->IsSelected()) // already selected...
                continue;

            int selectIt = 0;
            face_t *f;
            for (f = b->faces.p_next; f != &b->faces; f = f->p_next)
            {
                LoopProblem("selbytex");
                if (set.game_mode == 2)
                {
                    if (!strcmpi(f->texture.texture,texName) &&
                            !strcmpi(f->texture.basepath,texPath))
                    {
                        selectIt = 1;
                        break;
                    };
                }
                else
                {
                    if (!strcmpi(f->texture.texture,texName))
                    {
                        selectIt = 1;
                        break;
                    };
                }
            }

            if (selectIt)         // recache textures...
            {
                b->setSelected(true);
            };
        };
    };
};

//
// Revolve it...
void map::revolveObject(char *fname, int hstrips, int neardist, int fardist)
{
    int i,j,k,newnov;
    float *c,*s;
    int *index1, *index2;
    vec3_t *verts;
    vec3_t p[4];
    vec3_t q[4];
    vec3_t ctr;
    vec3_t *v;
    face_t *f;

    texturedef_t td;
    SetBrush *b;
    vec3_t mins, maxs;

    if (numSelected() != 1)
    {
        //	qprintf ("must have a single brush selected");
        return;
    }

    dirty = 1;

    b = selectedBrush();
    int grp = b->group;
    td = *b->texturedef();
    b->getMins(mins,maxs);
    b->remove();
    delete b;

    for (i = 0; i < 3; i++)
    {
        ctr[i] = (int)((mins[i]+maxs[i])/2.0);
    };

//	ctr[0] = ctr[1] = ctr[2] = 0.0;
    FILE *fp;

    if ((fp = fopen(fname,"rt")) == NULL)
    {
        return;
    };

    int nhoriz;
    int nverts;
    int okay;

    nhoriz = hstrips;
    okay = fscanf(fp,"%i",&nverts);
    if ((okay != 1) || (nverts < 2) || (nverts >= 100))
    {
        fclose(fp);
        return;
    };

    nhoriz = min(100,nhoriz);
    nhoriz = max(2,nhoriz);

    float theta;
    float thetadiff;

    v = new vec3_t[nverts];

    for (i = 0; i < nverts; i++)
    {
        okay = fscanf(fp,"%f %f",&v[i][0],&v[i][1]);
        if (okay != 2)
        {
            fclose(fp);
            return;
        };
    };

    theta = 0.0;
    thetadiff = 2*M_PI/nhoriz;

    c = new float[100];
    s = new float[100];
    index1 = new int[101];
    index2 = new int[101];
    verts = new vec3_t[1024];

    for (i = 0; i < nhoriz; i++)
    {
        c[i] = cos(theta);
        s[i] = sin(theta);
        theta += thetadiff;
    };

    newnov = 0;
    // newnov = nov; first new vertex index...

    if (fabs(v[0][0]) < F_EPSILON)
    {
        // create one point at top
        // at 0.0, ctr.y, 0.0
        //
        verts[newnov][0] = 0.0;
        verts[newnov][1] = v[0][1];
        verts[newnov][2] = 0.0;

        for (i = 0; i <= nhoriz; i++)
        {
            index1[i] = newnov;
        };
        newnov++;
    }
    else
    {
        for (i = 0; i < nhoriz; i++)
        {
            verts[newnov][0] = v[0][0]*c[i];
            verts[newnov][1] = v[0][1];
            verts[newnov][2] = -v[0][0]*s[i];
            index1[i] = newnov;
            newnov++;
        };
        index1[nhoriz] = index1[0];
    };

    for (j = 1; j < nverts; j++)
    {
        if (fabs(v[j][0]) < F_EPSILON)
        {
            verts[newnov][0] = 0.0;
            verts[newnov][1] = v[j][1];
            verts[newnov][2] = 0.0;

            for (i = 0; i <= nhoriz; i++)
            {
                index2[i] = newnov;
            };
            newnov++;
        }
        else
        {
            for (i = 0; i < nhoriz; i++)
            {
                verts[newnov][0] = v[j][0]*c[i];
                verts[newnov][1] = v[j][1];
                verts[newnov][2] = -v[j][0]*s[i];
                index2[i] = newnov;
                newnov++;
            };
            index2[nhoriz] = index2[0];
        };
        if (index1[0] != index1[1])
            if (index2[0] == index2[1])
            {
                for (i = 0; i < nhoriz; i++)
                {
                    // three sided facet
                    // create full brush...
                    b = new SetBrush();
                    b->group = grp;

                    for (k = 0; k < 3; k++)
                    {
                        p[0][k] = rint(ctr[k] + (neardist*verts[index1[i+1]][k]));
                        p[1][k] = rint(ctr[k] + (neardist*verts[index2[i]][k]));
                        p[2][k] = rint(ctr[k] + (neardist*verts[index1[i]][k]));

                        q[0][k] = rint(ctr[k] + (fardist*verts[index1[i+1]][k]));
                        q[1][k] = rint(ctr[k] + (fardist*verts[index2[i]][k]));
                        q[2][k] = rint(ctr[k] + (fardist*verts[index1[i]][k]));
                    };

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));

                    // nearface
                    VectorCopy(p[0],f->planepts[0]);
                    VectorCopy(p[1],f->planepts[1]);
                    VectorCopy(p[2],f->planepts[2]);

                    b->addObject(f);

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));

                    // far face...
                    VectorCopy(q[1],f->planepts[0]);
                    VectorCopy(q[0],f->planepts[1]);
                    VectorCopy(q[2],f->planepts[2]);

                    b->addObject(f);

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));

                    // left face
                    VectorCopy(p[2],f->planepts[0]);
                    VectorCopy(p[1],f->planepts[1]);
                    VectorCopy(q[1],f->planepts[2]);

                    b->addObject(f);

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));

                    VectorCopy(p[1],f->planepts[0]);
                    VectorCopy(p[0],f->planepts[1]);
                    VectorCopy(q[0],f->planepts[2]);

                    b->addObject(f);

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));

                    VectorCopy(p[0],f->planepts[0]);
                    VectorCopy(p[2],f->planepts[1]);
                    VectorCopy(q[2],f->planepts[2]);

                    b->addObject(f);

                    b->setTexturedef(&td);

                    b->setParent(world);
                    world->addObject(b);
                    b->setSelected(true);
                };
            }
            else
            {
                for (i = 0; i < nhoriz; i++)
                {
                    // create four sided facets...

                    b = new SetBrush();
                    b->group = grp;

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));

                    for (k = 0; k < 3; k++)
                    {
                        p[0][k] = rint(ctr[k] + (neardist*verts[index1[i+1]][k]));
                        p[1][k] = rint(ctr[k] + (neardist*verts[index2[i+1]][k]));
                        p[2][k] = rint(ctr[k] + (neardist*verts[index2[i]][k]));
                        p[3][k] = rint(ctr[k] + (neardist*verts[index1[i]][k]));

                        q[0][k] = rint(ctr[k] + (fardist*verts[index1[i+1]][k]));
                        q[1][k] = rint(ctr[k] + (fardist*verts[index2[i+1]][k]));
                        q[2][k] = rint(ctr[k] + (fardist*verts[index2[i]][k]));
                        q[3][k] = rint(ctr[k] + (fardist*verts[index1[i]][k]));
                    };

                    // nearface
                    VectorCopy(p[0],f->planepts[0]);
                    VectorCopy(p[1],f->planepts[1]);
                    VectorCopy(p[2],f->planepts[2]);

                    b->addObject(f);

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));

                    // farface
                    VectorCopy(q[1],f->planepts[0]);
                    VectorCopy(q[0],f->planepts[1]);
                    VectorCopy(q[2],f->planepts[2]);

                    b->addObject(f);

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));

                    VectorCopy(p[1],f->planepts[0]);
                    VectorCopy(p[2],f->planepts[1]);
                    VectorCopy(ctr,f->planepts[2]);

                    b->addObject(f);

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));

                    VectorCopy(p[2],f->planepts[0]);
                    VectorCopy(p[3],f->planepts[1]);
                    VectorCopy(ctr,f->planepts[2]);

                    b->addObject(f);

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));

                    VectorCopy(p[3],f->planepts[0]);
                    VectorCopy(p[0],f->planepts[1]);
                    VectorCopy(ctr,f->planepts[2]);

                    b->addObject(f);

                    f = new face_t;
                    memset(f, 0, sizeof(face_t));

                    VectorCopy(p[0],f->planepts[0]);
                    VectorCopy(p[1],f->planepts[1]);
                    VectorCopy(ctr,f->planepts[2]);

                    b->addObject(f);

                    b->setTexturedef(&td);

                    b->setParent(world);
                    world->addObject(b);
                    b->setSelected(true);
                };
            }
        else if (index2[0] != index2[1])
            for (i = 0; i < nhoriz; i++)
            {
                // three sided facet
                // create full brush...
                b = new SetBrush();
                b->group = grp;

                for (k = 0; k < 3; k++)
                {
                    p[0][k] = rint(ctr[k] + (neardist*verts[index2[i+1]][k]));
                    p[1][k] = rint(ctr[k] + (neardist*verts[index2[i]][k]));
                    p[2][k] = rint(ctr[k] + (neardist*verts[index1[i]][k]));

                    q[0][k] = rint(ctr[k] + (fardist*verts[index2[i+1]][k]));
                    q[1][k] = rint(ctr[k] + (fardist*verts[index2[i]][k]));
                    q[2][k] = rint(ctr[k] + (fardist*verts[index1[i]][k]));
                };

                f = new face_t;
                memset(f, 0, sizeof(face_t));

                // nearface
                VectorCopy(p[0],f->planepts[0]);
                VectorCopy(p[1],f->planepts[1]);
                VectorCopy(p[2],f->planepts[2]);

                b->addObject(f);

                f = new face_t;
                memset(f, 0, sizeof(face_t));

                // far face...
                VectorCopy(q[1],f->planepts[0]);
                VectorCopy(q[0],f->planepts[1]);
                VectorCopy(q[2],f->planepts[2]);

                b->addObject(f);

                f = new face_t;
                memset(f, 0, sizeof(face_t));

                // left face
                VectorCopy(p[2],f->planepts[0]);
                VectorCopy(p[1],f->planepts[1]);
                VectorCopy(q[1],f->planepts[2]);

                b->addObject(f);

                f = new face_t;
                memset(f, 0, sizeof(face_t));

                VectorCopy(p[1],f->planepts[0]);
                VectorCopy(p[0],f->planepts[1]);
                VectorCopy(q[0],f->planepts[2]);

                b->addObject(f);

                f = new face_t;
                memset(f, 0, sizeof(face_t));

                VectorCopy(p[0],f->planepts[0]);
                VectorCopy(p[2],f->planepts[1]);
                VectorCopy(q[2],f->planepts[2]);

                b->addObject(f);

                b->setTexturedef(&td);

                b->setParent(world);
                world->addObject(b);
                b->setSelected(true);
            };
        for (i = 0; i <= nhoriz; i++)
            index1[i] = index2[i];
    };

    delete [] v;
    delete [] verts;
    delete [] index2;
    delete [] index1;
    delete [] s;
    delete [] c;

    cleanUpInvalidObjects();

};

void map::UnloadTextures()
{
    Entity *e;
    SetBrush *b;
    face_t *f;

    for (e = objects.p_next; e != &objects; e = e->p_next)
    {
        for (b = e->objects.p_next; b != &e->objects; b = b->p_next)
        {
            for (f = b->faces.p_next; f != &b->faces; f = f->p_next)
            {
                LoopProblem("unload tex");
                if (f->qtexture)
                    delete f->qtexture;
                f->qtexture = NULL;
            }
        }
    }

    if (favorites)
    {
        delete[] favorites;
        favorites = NULL;
    }

    if (bests)
    {
        delete[] bests;
        bests = NULL;
    }

    if (mapTextures)
    {
        for (int i = 0; i < numMapTextures; i++)
        {
            if (mapTextures[i]->aTextures)
            {
                delete[] mapTextures[i]->aTextures;
                mapTextures[i]->aTextures = NULL;
            }
            mapTextures[i]->aCount = 0;
            mapTextures[i]->aCurrent = 0;
            //m->mapTextures[i]->animated = 0;
            if(set.glBsp && mapTextures[i]->bindIndex > 0)
            {
                glDeleteTextures(1, (GLuint*) &mapTextures[i]->bindIndex);
                mapTextures[i]->bindIndex = 0;
            }
        }
        delete[] mapTextures;
        mapTextures = NULL;
    }
    numMapTextures = 0;
}

void map::SetDetail(bool addDetail)
{
    if (!set.Map_Read || (set.game_mode != 2))
        return;

    Entity *ent;
    SetBrush *brush;

    dirty = 1;
    face_t *f;

    for (ent = objects.p_next; ent != &objects; ent = ent->p_next)
    {
        if (!ent->modifiable)
            continue;

        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            if (!brush->IsDrawable())
                continue;
            if (!brush->IsSelected())
                continue;

            for (f = brush->faces.p_next; f != &brush->faces; f = f->p_next)
            {
                LoopProblem("brush add detail");
                if (addDetail)
                    f->texture.contents |= CONTENTS_DETAIL;
                else
                    f->texture.contents &= ~CONTENTS_DETAIL;
            }
            brush->calcWindings();
        }
    }
}

void map::AddDetail()
{
    SetDetail(true);
}
void map::RemoveDetail()
{
    SetDetail(false);
}

//set brightness - 0 is off, >0 is alpha level up to 255 of whiteness to apply
void map::glBrightness()
{
    if(!set.gl_brightness) return;

    float w = EditSize.cx;
    float h = EditSize.cy;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glShadingOff();

    glOrtho(0, w, 0, h, -8000, 8000);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    glColor4ub(255,255,255,set.gl_brightness);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0,0,0);
    glVertex3f(w,0,0);
    glVertex3f(w,h,0);
    glVertex3f(0,h,0);
    glEnd();


    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    //glShadeModel(GL_SMOOTH);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glShadingOn();
}

void map::glDrawOrientation()
{
    if(set.angle_control == NO_ANGLE_CONTROL)
        return;

    char str_ang[40];
    POINT ed,md;
    POINT side1,side2;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glShadingOff();

    vec2_t mins = { 0, 0 }, maxs = { EditSize.cx, EditSize.cy };

    glOrtho(mins[0], maxs[0], mins[1], maxs[1], -8000, 8000);

    if(set.angle_control == FOV_CONTROL)
    {

        RECT rc;
        rc.left = EditSize.cx - fovsz.cx - 5;
        rc.right = rc.left + fovsz.cx;
        rc.top = EditSize.cy;
        rc.bottom = rc.top - (fovsz.cy + 5);

        const int slider_top = rc.top - 9;						// slider start below "FOV"
        const int slider_range = (slider_top-8)-(rc.bottom+2);	// pixel height of slide area
        const int pt90 = (90-15)*slider_range / (120-15);		//calculate pos of 90' between 15-120

        //FOV title
        glColor3ub(99,91,83);
        glRasterPos2i(rc.left+1, rc.top-10);
        glCallLists (3, GL_UNSIGNED_BYTE, "FOV");

        //blue range triangle
        glColor3ub(0,0,175);
        glBegin(GL_TRIANGLES);
        glVertex3i(rc.left+3,   rc.bottom+3,0);
        glVertex3i(rc.left+3+10,slider_top-8,0);
        glVertex3i(rc.left+3+10,rc.bottom+3,0);
        glEnd();

        //print FOV
        float fov = set.fov;

        int len = (int)sprintf(str_ang,"%d",(int)fov);

        fov -= 15;								// make range zero-based
        glColor3ub(255,255,0);					// yellow
        glRasterPos2i(rc.left+2, slider_top-10-(fov*slider_range/105));
        glCallLists (len, GL_UNSIGNED_BYTE, str_ang);

        glColor3ub(0,0,175);

        glBegin(GL_LINES);
        //line at 15
        glVertex3i(rc.right - 6, slider_top-5-5, 0);
        glVertex3i(rc.right, slider_top-5-5, 0);
        //line at 90
        glVertex3i(rc.right - 6, slider_top - pt90-5, 0);
        glVertex3i(rc.right, slider_top - pt90-5, 0);
        //line at 120
        glVertex3i(rc.right - 6, slider_top-slider_range-5, 0);
        glVertex3i(rc.right, slider_top-slider_range-5, 0);
        glEnd();

    }
    else
    {
        POINT cent;
        cent.x = EditSize.cx - 25;
        cent.y = EditSize.cy - 25;   // Adjust for gl

        float ang;
        if(set.angle_control==PITCH_CONTROL)
            ang = angles[cureye].pitch;
        else
            ang = angles[cureye].yaw;

        ed.x = cent.x + 20 * cos(ang);
        ed.y = cent.y + 20 * sin(ang); //

        md.x = cent.x + 15 * cos(ang);
        md.y = cent.y + 15 * sin(ang);   //

        side1.x = cent.x + 12 *cos(ang+0.2);
        side1.y = cent.y + 12 *sin(ang+0.2); //
        side2.x = cent.x + 12 *cos(ang-0.2);
        side2.y = cent.y + 12 *sin(ang-0.2); //


        glColor3ubv((GLubyte*) &set.color_foreground);

        int steps = 36;
        float c_ang = 0.0f;
        float astep = 2.0f*M_PI/(float)steps;
        glBegin(GL_LINE_LOOP);
        vec3_t pts = { 0, 0, 0 };
        for (int i = 0; i < steps; i++)
        {
            pts[0] = cent.x + 20.0f*cos(c_ang);
            pts[1] = cent.y + 20.0f*sin(c_ang);
            glVertex3fv(pts);
            c_ang += astep;
        }
        glEnd();

        glBegin(GL_LINE_STRIP);
        glVertex3i(cent.x,cent.y,0);
        glVertex3i(md.x,md.y,0);
        glVertex3i(side1.x,side1.y,0);
        glVertex3i(ed.x,ed.y,0);
        glVertex3i(side2.x,side2.y,0);
        glVertex3i(md.x,md.y,0);
        glEnd();

        int a = (int)(180.0f*ang/M_PI);
        while (a >= 360.0f)
            a -= 360.0f;
        while (a < 0.0f)
            a += 360.0f;

        sprintf(str_ang,"%d",a);

        // color
        if(set.angle_control==PITCH_CONTROL)
        {
            glColor3ub(0, 255, 0);
        }
        else
        {
            glColor3ub(255, 0, 0);
        }

        glRasterPos2i(cent.x - 10, cent.y - 3);
        glCallLists (strlen(str_ang), GL_UNSIGNED_BYTE, str_ang);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glShadingOn();
}

void map::UpdateGroupVisibility(int grp)
{
    if (!set.Map_Read)
        return;
    if (!groups)
        return;
    if (grp < 0)
        return;
    if (grp >= groups->NumGroups())
        return;

    Entity *ent;
    SetBrush *brush;

    dirty = 1;

    for (ent = objects.p_next; ent != &objects; ent = ent->p_next)
    {
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            if (brush->group != grp)
                continue;

            brush->setDrawable();
        }
    }
}

void map::UpdateMapVisibility()
{
    if (!set.Map_Read)
        return;

    Entity *ent;
    SetBrush *brush;

    dirty = 1;

    for (ent = objects.p_next; ent != &objects; ent = ent->p_next)
    {
        for (brush = ent->objects.p_next; brush != &ent->objects; brush = brush->p_next)
        {
            brush->setDrawable();
        }
    }
}
