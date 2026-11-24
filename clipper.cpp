#include "global.h"

bool splitSelect = true;
static vec3_t oldrelative;

Clipper::Clipper()
{
    num = 0;
    memset(&pos[0],0,sizeof(vec3_t) * 3);
    memset(&plane,0,sizeof(plane_t));
    state = false;
}

Clipper *Clipper::copy()
{
    Clipper *c = new Clipper;
    c->num = num;
    memcpy(&c->pos[0],&pos[0],sizeof(vec3_t) * 3);
    memcpy(&c->plane,&plane,sizeof(plane_t));
    return c;
}

bool Clipper::hide()
{
    int oldnum = num;
    num = 0;
    return (oldnum > 0);
}

void Clipper::flipNormal()
{
    vec3_t temp;
    if (num == 2)
    {
        VectorCopy (pos[0], temp);
        VectorCopy (pos[1], pos[0]);
        VectorCopy (temp, pos[1]);
    }
    else if (num == 3)
    {
        VectorCopy (pos[0], temp);
        VectorCopy (pos[2], pos[0]);
        VectorCopy (temp, pos[2]);
    }
}

bool Clipper::getFace(face_t *f)
{
    if (num < 2)
        return false;

    vec3_t	v1, v2, norm;

    VectorCopy (vec3_origin, plane.normal);
    plane.dist = 0;

    if (num == 2)
    {
        // may need to redo wrt xy view...
        VectorCopy (pos[0], pos[2]);
        if (xyWindow[set.curxy])
        {
            pos[2][xyWindow[set.curxy]->NormalAxis] += 16;
        }
        else
        {
            syserror(const_cast<char *> ("Clipper::getFace: xy is null"));
            pos[2][2] += 16; //todo: this should never happen
        }
    }

    VectorCopy (pos[0], f->planepts[0]);
    VectorCopy (pos[1], f->planepts[1]);
    VectorCopy (pos[2], f->planepts[2]);

    VectorSubtract (pos[2], pos[0], v1);
    VectorSubtract (pos[1], pos[0], v2);

    CrossProduct (v1, v2, norm);

    if ( !norm[0] && !norm[1] && !norm[2] )
        return false;

    VectorNormalize (norm);

    if ( !norm[0] && !norm[1] && !norm[2] )
        return false;

    texWindow->getTextureDef(&f->texture);
    return true;
}

/*
================
XYClick
================
*/
void Clipper::XYClick(vec3_t pt, int UA, int VA, int NA, int /*whichXY*/)
{
    int		i;
    vec3_t	nw;
    vec3_t eyeLevel;

    VectorCopy(xy_eye,eyeLevel);

    nw[UA] = (float) snapToGrid(pt[0]);
    nw[VA] = (float) snapToGrid(pt[1]);
    nw[NA] = (float) snapToGrid(eyeLevel[NA]);

    map *m = map_i[set.curmap];

    // see if a point is already there
    for (i=0 ; i<num ; i++)
    {
        if (nw[UA] == pos[i][UA] && nw[VA] == pos[i][VA])
        {
            if (pos[i][NA] == eyeLevel[NA])
                pos[i][NA] = (float) snapToGrid(eyeLevel[NA]+(m->currentMaxZ(NA)-m->currentMinZ(NA)));
            else
                pos[i][NA] = (float) snapToGrid(eyeLevel[NA]);
            return;
        }
    }

    if(set.clipper_mode == 1)
    {
        if(num >= 2)
            num = 0;
    }
    else
    {
        if(num >= 3)
            num = 0;
    }
    VectorCopy (nw, pos[num]);
    m->curclippoint = num;
    num++;

    /*	does not work: points get all messed up

    	if (set.clipper_mode == 2 && num == 2){
    		vec3_t tmp;
    		VectorCopy(pos[0], tmp);
    		pos[0][UA] = pos[1][UA];
    		pos[0][VA] = pos[1][VA];
    		pos[0][NA] = pos[1][NA] + 32.0;
    		VectorCopy(tmp, pos[2]);
    		num++;
    		m->curclippoint++;
    	}*/

    if(set.clipper_autoflip && num > 1)
    {
        if(num == 2)  	//need a 3rd point
        {
            pos[2][UA] = pos[1][UA];
            pos[2][VA] = pos[1][VA];
            pos[2][NA] = pos[1][NA] + 32.0f;
        }
        vec3_t normal;
        VectorCalcNormal(pos[0],pos[1],pos[2], normal);

        //get direction of plane from center of selection
        vec3_t d;
        d[0] = (pos[0][0] + pos[1][0] + pos[2][0]) / 3.0f;
        d[1] = (pos[0][1] + pos[1][1] + pos[2][1]) / 3.0f;
        d[2] = (pos[0][2] + pos[1][2] + pos[2][2]) / 3.0f;

        m->getSelectedCenter();
        VectorSubtract(d, sb_ctr, d);

        float dot = DotProduct(d, normal);
        if(dot < 0.0)  						//flip if clipper plane is facing brush
        {
            flipNormal();
            m->curclippoint = 0;
        }

    }
    else if(num == 2 && NA == 1)  				// fix XZ problem
    {
        flipNormal();
        m->curclippoint = 0;
    }
}

void Clipper::dragFrom(vec3_t location, int pttomove, int UA, int VA, int whichXY)
{
    MSG msg;
    SIZE XYSize;
    POINT np;

    vec3_t startpt, newpt, relative;

    startpt[0] = location[0];
    startpt[1] = location[1];

    oldrelative[0] = oldrelative[1] = 0.0;

    XYSize.cx = xyWindow[whichXY]->XYSize.cx;
    XYSize.cy = xyWindow[whichXY]->XYSize.cy;

    float u, v;
    SetCapture(xyWindow[whichXY]->hwnd);
    while (1)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            if ((msg.message == WM_LBUTTONUP) ||
                (msg.message == WM_RBUTTONUP) ||
                (msg.message == WM_NCLBUTTONUP) ||
                (msg.message == WM_NCRBUTTONUP))
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
            WaitMessage();

            GetCursorPos(&np);
            ScreenToClient(xyWindow[whichXY]->hwnd,&np);

            u = xy_eye[UA];
            v = xy_eye[VA];

            newpt[0] = ((float)np.x - (float)XYSize.cx/2.0f)/set.scale + u;
            newpt[1] = ((float)XYSize.cy/2.0f - (float)np.y)/set.scale + v;

            relative[0] = newpt[0] - startpt[0];
            relative[1] = newpt[1] - startpt[1];

            if (relative[0] == oldrelative[0] && relative[1] == oldrelative[1])
                continue;

            VectorCopy(relative, oldrelative);

            //TODO TEST
            xyWindow[whichXY]->UpdateSelected(true);
        }
    }
    ReleaseCapture();
}

void Clipper::DragCallBack(float dx, float dy, int i, int UA, int VA)
{
    if (i == -1)
    {
        for (int j=0; j<3; j++)
        {
            pos[j][UA] = pos[j][UA] + dx;
            pos[j][VA] = pos[j][VA] + dy;
        }
    }
    else
    {
        pos[i][UA] = pos[i][UA] + dx;
        pos[i][VA] = pos[i][VA] + dy;
    }
}

/*
================
XYDrag
================
*/
bool Clipper::XYDrag(vec3_t pt,int UAxis, int VAxis, int /*whichXY*/)
{
    float delta;
    float d;
    float pixelWidth;
    float clipRadius = 10.0f;

    pixelWidth = 1.0f/set.scale;  // each pixel is this many world units...
    delta = (clipRadius/2.0f) * pixelWidth; // how many units to allow...
    for (int i=0 ; i < num ; i++)
    {
        d = ((pt[0] - pos[i][UAxis]) * (pt[0] - pos[i][UAxis]) +
             (pt[1] - pos[i][VAxis]) * (pt[1] - pos[i][VAxis]));

        d = sqrt(d);

        if (d > delta)
            continue;

        map_i[set.curmap]->curclippoint = i;
        return true;
    }
    return false;
}

bool Clipper::XYDragWhole(vec3_t pt,int UAxis, int VAxis, int /*whichXY*/)
{
    return XYDrag(pt,UAxis,VAxis,0);
}

//=============================================================================

void Clipper::carve()
{
    map_i[set.curmap]->makeSelectedPerform(SEL_CARVEBYCLIPPER);
    if(!set.clippers_stay)
        num = 0;
}

void Clipper::split(bool selAll)
{
    splitSelect = selAll;
    map_i[set.curmap]->makeSelectedPerform(SEL_SPLITBYCLIPPER);
    if(!set.clippers_stay)
        num = 0;
}

//draw clipper points in 3D
void Clipper::cameraDrawSelf(HDC hdc)
{
    if(num <= 0)
        return;
    map *m = map_i[set.curmap];

    int i;
    if (set.glBsp)
    {

        glShadingOff();

        glDepthMask(GL_FALSE);

        //draw points
        glPointSize(5.0f);
        glBegin(GL_POINTS);
        for (i=0 ; i<num ; i++)
        {
            if (i == m->curclippoint)
            {
                glColor3ubv((GLubyte*) &set.color_currentclipface);
            }
            else
            {
                glColor3ubv((GLubyte*) &set.color_otherclipface);
            }
            glVertex3fv(pos[i]);
        }
        glEnd();
        glPointSize(1.0);

        //connect points
        glColor3ub(0,255,0);
        if(num==3)
        {
            glBegin(GL_LINE_LOOP);
            glVertex3fv(pos[0]);
            glVertex3fv(pos[1]);
            glVertex3fv(pos[2]);
            glEnd();
        }
        else if(num==2)
        {
            glBegin(GL_LINE_STRIP);
            glVertex3fv(pos[0]);
            glVertex3fv(pos[1]);
            glEnd();
        }

        glDepthMask(GL_TRUE);

        glShadingOn();

    }
    else
    {
        int selPoint;
        POINT p1, p2, pt;
        int counter;

        HBRUSH lbColor = CreateSolidBrush(set.color_otherclipface);
        HBRUSH sbColor = CreateSolidBrush(set.color_currentclipface);

        COLORREF color = !state ? set.color_clipborder : set.color_foreground;
        HPEN pen = CreatePen(PS_SOLID,1,color);

        HGDIOBJ oldbrush = SelectObject(hdc,lbColor);
        HGDIOBJ oldpen = SelectObject(hdc,pen);

        HGDIOBJ oldfont = SelectObject(hdc,set.font12);

        selPoint = -1;
        counter = 0;
        for (i = 0; i < num; i++)
        {
            if (i == m->curclippoint)
            {
                selPoint = counter;
            }
            counter++;
        }

        // draw points in 3d
        for (i = 0; i < counter; i++)
        {
            REN_PointToScreen(pos[i],&pt);
            if(pt.x<0 || pt.y<0)
                continue;

            p1.x = pt.x-3;
            p1.y = pt.y-3;
            p2.x = pt.x+3;
            p2.y = pt.y+3;

            Str text(const_cast<char *> ("%i"), i+1);
            SetTextColor(hdc,set.color_foreground);
            SetBkMode(hdc,TRANSPARENT);
            TextOut(hdc,p1.x,p2.y+2,text,text.length);

            if (selPoint != -1 && selPoint == i)
                SelectObject(hdc,sbColor);
            else
                SelectObject(hdc,lbColor);

            Ellipse(hdc,p1.x,p1.y,p2.x,p2.y);
        }
        SelectObject(hdc,oldbrush);
        SelectObject(hdc,oldpen);
        SelectObject(hdc,oldfont);
        DeleteObject(sbColor);
        DeleteObject(lbColor);
        DeleteObject(pen);
    }
}



//draw clipper points in XY/GDI
void Clipper::XYDrawSelf(TXYViewWindow *xy, HDC hdc, bool notBlack)
{
    if (!set.Map_Read)
        return;

    POINT p;

    COLORREF clr_lp, clr_lb, clr_sb;
    if(notBlack)
    {
        clr_lp = state ? set.color_foreground : set.color_clipborder;
        clr_lb = set.color_otherclipface;
        clr_sb = set.color_currentclipface;
    }
    else
    {
        clr_lp = clr_lb = clr_sb = set.color_background;
    }

    int xofs = (int)(xy->XYSize.cx / 2);
    int yofs = (int)(xy->XYSize.cy / 2);

    int axis0, axis1, dummy;
    GetAxes(xy->ViewType, &axis0, &axis1, &dummy);

    float u = xy_eye[axis0];
    float v = xy_eye[axis1];

    HPEN pen;
    HGDIOBJ oldpen;
    //DRAW LINES BETWEEN CLIPPERS
    if(num >= 2)
    {
        pen = CreatePen(PS_SOLID,1,RGB(0,255,0));
        oldpen = SelectObject(hdc,pen);
        POINT sp;
        sp.x = (LONG) (-u + pos[0][axis0])*set.scale+xofs;
        sp.y = (LONG) yofs-(-v + pos[0][axis1])*set.scale;
        MoveToEx(hdc, sp.x, sp.y, 0);

        p.x = (LONG) (-u + pos[1][axis0])*set.scale+xofs;
        p.y = (LONG) yofs-(-v + pos[1][axis1])*set.scale;
        LineTo(hdc, p.x, p.y);

        if(num >= 3)
        {

            p.x = (LONG) (-u + pos[2][axis0])*set.scale+xofs;
            p.y = (LONG) yofs-(-v + pos[2][axis1])*set.scale;
            LineTo(hdc, p.x, p.y);
            LineTo(hdc, sp.x, sp.y);
        }
        SelectObject(hdc,oldpen);
        DeleteObject(pen);
    }
    //END DRAW LINES

    pen = CreatePen(PS_SOLID,1,clr_lp);
    HBRUSH lb = CreateSolidBrush(clr_lb);
    HBRUSH sb = CreateSolidBrush(clr_sb);

    oldpen = SelectObject(hdc,pen);
    HGDIOBJ oldbrush = SelectObject(hdc,lb);
    HGDIOBJ oldfont = SelectObject(hdc,set.font12);

    map *m = map_i[set.curmap];

    for (int i=0 ; i<num ; i++)
    {
        p.x = (LONG) (-u + pos[i][axis0])*set.scale+xofs;
        p.y = (LONG) yofs-(-v + pos[i][axis1])*set.scale;

        if (PtInRect(&xy->XYClip,p))
        {
            if (notBlack)
            {
                SetTextColor(hdc,set.color_foreground);
                SetBkColor(hdc,set.color_background);
            }
            else
            {
                SetTextColor(hdc,set.color_background);
                SetBkColor(hdc,set.color_background);
            }
            Str text(const_cast<char *> ("%i"), i+1);
            TextOut(hdc,p.x-4,p.y+6,text,text.length);

            if (i == m->curclippoint)
            {
                SelectObject(hdc,sb);
            }
            else
            {
                SelectObject(hdc,lb);
            }
            Ellipse(hdc,p.x-2,p.y-1,p.x+4,p.y+5);
        }
    }

    SelectObject(hdc,oldpen);

    SelectObject(hdc,oldbrush);
    SelectObject(hdc,oldfont);
    DeleteObject(pen);
    DeleteObject(lb);
    DeleteObject(sb);
}
