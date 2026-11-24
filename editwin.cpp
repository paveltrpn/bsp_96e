#include "global.h"

bool glUnselectedOnly = false;
map *glMap;

int glLastBoundIndex = -1;
bool glSkyboxNotEmpty;
int knob_eps = 3;

vec3_t edgeNormal;

bool MouseParsed3D = false;
mouseEventType *head3d[MAX_MOUSE_EVENT_TYPES];
char EventTypes3d[MAX_MOUSE_EVENT_TYPES] [32];
int numEventTypes3d = 0;
int curEventType3d = 0;

bool mouse_is_navigating = false;	// true when the mouse is looking/navigating around.

float startA;

int brushCount;

vec3_t matrix[3];

#define	CLIP_RIGHT	 1
#define	CLIP_LEFT	 2
#define	CLIP_TOP	 4
#define	CLIP_BOTTOM	 8
#define	CLIP_FRONT	16

#define NEARZ        1.0f
#define FARZ      4096.0f

SIZE fovsz = {25,100};		//dimensions of fov control rect
//////////////

//TEST
vec3_t start_tex_rotate = {0,0,0};
float start_s=0,start_t=0;

LRESULT TEditWindow::WndProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_CREATE:
        SetupWindow();
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case NUMPORTALS:
            SetNumPortals();
            return 0;
        case STOPPORTALS:
            KillPortals();
            return 0;
        case HELPBUTTON:
            MouseHelp();
            return 0;
        case CONNECTIONBUTTON:
            ToggleConnections();
            return 0;
        case MOUSEBASE+0:
            M1();
            return 0;
        case MOUSEBASE+1:
            M2();
            return 0;
        case MOUSEBASE+2:
            M3();
            return 0;
        case MOUSEBASE+3:
            M4();
            return 0;
        case MOUSEBASE+4:
            M5();
            return 0;
        case MOUSEBASE+5:
            M6();
            return 0;
        case MOUSEBASE+6:
            M7();
            return 0;
        case MOUSEBASE+7:
            M8();
            return 0;
        case MOUSEBASE+8:
            M9();
            return 0;
        case MOUSEBASE+9:
            M10();
            return 0;
        case MODEBASE+0:
            MNormal();
            return 0;
        case MODEBASE+1:
            MSelect();
            return 0;
        case MODEBASE+2:
            MShift();
            return 0;
        case MODEBASE+3:
            MStretch();
            return 0;
        case MODEBASE+4:
            MRotate();
            return 0;
        case MODEBASE+5:
            MApplyToBrush();
            return 0;
        case MODEBASE+6:
            MApplyToFace();
            return 0;
        case MODEBASE+7:
            MLift();
            return 0;
        case MODEBASE+8:
            MLiftApply();
            return 0;
        case MODEBASE+9:
            MDragFace();
            return 0;
        case MODEBASE+10:
            MShearFace();
            return 0;
        case MODEBASE+11:
            MDragBrushes();
            return 0;
        case MODEBASE+12:
            MDragBrushesSide();
            return 0;
        case MODEBASE+13:
            MSetClipper();
            return 0;
        case MODEBASE+14:
            MSetClipperInv();
            return 0;
        case MODEBASE+15:
            MNavigate();
            return 0;
        case MODEBASE+16:
            MEdgeDrag();
            return 0;
        case MODEBASE+17:
            MLook();
            return 0;
        case MODEBASE+18:
            MSelectOne();
            return 0;
        case MODEBASE+19:
            MNavigateStrafe();
            return 0;
        case TIMEREFRESH:
            TimeRefresh();
            return 0;
        case CENTERKNOBS:
            ToggleCenterKnobs();
            return 0;
        case EDITMENU_YAW_CONTROL:
            SetOrientControl(YAW_CONTROL);
            return 0;
        case EDITMENU_PITCH_CONTROL:
            SetOrientControl(PITCH_CONTROL);
            return 0;
        case EDITMENU_FOV_CONTROL:
            SetOrientControl(FOV_CONTROL);
            return 0;
        case EDITMENU_NO_ANGLE_CONTROL:
            SetOrientControl(NO_ANGLE_CONTROL);
            return 0;
        }
        break;

    case WM_MOUSEMOVE:
    {
        if(set.use_crosshair)
            SetCursor(LoadCursor(0,IDC_CROSS));

        MAKEPOINT(pt, lParam);
        EvMouseMove(wParam,&pt);
    }
    break;
    case WM_SIZE:

        RemoveMemDC();
        EditSize.cx = LOWORD(lParam);
        EditSize.cy = HIWORD(lParam);
        EditClip.top = 0;
        EditClip.left = 0;
        EditClip.right = LOWORD(lParam);
        EditClip.bottom = HIWORD(lParam);
        ConstructMemDC(); // new size

        if (set.glBsp)
        {
            wglMakeCurrent(ghDC, ghRC);
            resize();
        }
        RedrawContents();
        break;
    case WM_ERASEBKGND:
        return 0;
    case WM_PAINT:
    {
        HDC hdc = GetDC(hwnd);
        Paint(hdc);
        ReleaseDC(hwnd,hdc);
    }
    return 0;

    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_LBUTTONUP:
    case WM_XBUTTONUP:
    {
        int btn;
        if(msg == WM_LBUTTONUP)
            btn = 0;
        else if(msg == WM_MBUTTONUP)
            btn = 1;
        else if(msg == WM_RBUTTONUP)
            btn = 2;
        else if(msg == WM_XBUTTONUP)
        {
            if(wParam & MK_XBUTTON1)
                btn = 3;
            else
                btn = 4;
        }
        MAKEPOINT(pt, lParam);
        StopMouse(&pt, btn);
        Ccmd_UpdateEnablers();
    }
    break;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDOWN:
    {
        SetFocus();
        int btn;
        if(msg == WM_LBUTTONDOWN)
            btn = 0;
        else if(msg == WM_MBUTTONDOWN)
            btn = 1;
        else if(msg == WM_RBUTTONDOWN)
            btn = 2;
        else if(msg == WM_XBUTTONDOWN)
        {
            if(wParam & MK_XBUTTON1)
                btn = 3;
            else
                btn = 4;
        }

        MAKEPOINT(pt, lParam);
        StartMouse(wParam, &pt, btn);
    }
    break;
    case WM_MOUSEWHEEL:

        if(SetFocusUnderMouse(hwnd))
            return 0;

        if(LOWORD(wParam) & MK_CONTROL)  			// up / down
        {
            if((short)HIWORD(wParam) >= 0)
            {
                CmMoveUp();
            }
            else
            {
                CmMoveDown();
            }
        }
        else
        {
            if((short)HIWORD(wParam) >= 0)  			//forward / back
            {
                CmForward();
            }
            else
            {
                CmBack();
            }
        }
        return 0;

    case WM_NCRBUTTONDOWN:
    {
        POINT pt = {LOWORD(lParam), HIWORD(lParam)};
        if(DoNCRButtonDown(wParam, pt))
            return 0;
    }
    }
    return TCWindow::WndProc(msg,wParam,lParam);
}


void TEditWindow::SetOrientControl(int ctrl_mode)
{
    set.angle_control = ctrl_mode;
    set.redrawedit = 1;
    Show_Frame("",true);
}
void b_glSetMatrix(map *m)
{
    float mroll[3][3] =  {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    float mpitch[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    float myaw[3][3] =   {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};

    float  s, c, mtemp1[3][3], mtemp2[3][3];

    float roll = 0;
    float pitch = m->angles[m->cureye].pitch;
    float yaw = m->angles[m->cureye].yaw;

    s = sin(pitch);
    c = cos(pitch);
    mroll[0][0] = c;
    mroll[0][1] = s;
    mroll[1][0] = -s;
    mroll[1][1] = c;

    s = sin(-yaw + 1.5f*M_PI);
    c = cos(-yaw + 1.5f*M_PI);
    mpitch[1][1] = c;
    mpitch[1][2] = s;
    mpitch[2][1] = -s;
    mpitch[2][2] = c;

    s = sin(roll + M_PI/2.0f);
    c = cos(roll + M_PI/2.0f);
    myaw[0][0] = c;
    myaw[0][2] = -s;
    myaw[2][0] = s;
    myaw[2][2] = c;

    MConcat(mpitch, myaw, mtemp1);      // myaw, mtemp1, mtemp2
    MConcat(mroll, mtemp1, mtemp2);
    float SC[4][4], T3[4][4], R[4][4];
    float Q[4][4] = {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
    float T2[4][4] = {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
    // rotate axis 1 by -90 and axis 3 by 90

    for (int i=0 ; i<3 ; i++)
    {
        T2[2][i] = mtemp2[0][i];
        T2[1][i] = mtemp2[1][i];
        T2[0][i] = mtemp2[2][i];
    }

    rot3(1,-M_PI/2.0f,T3);
    mult3(T3, T2, SC);
    rot3(3,M_PI/2.0f,T3);
    mult3(SC, T3, T2);

    scale3(1,1,1,SC);
    mult3(T2,SC,R);


    T2[0][3] = -m->eye[m->cureye][0];
    T2[1][3] = -m->eye[m->cureye][1];
    T2[2][3] = -m->eye[m->cureye][2];

    mult3(T2,SC,Q);

    GLfloat q_m[4][4];
    q_m[0][0] = Q[0][0];
    q_m[1][0] = Q[0][1];
    q_m[2][0] = Q[0][2];
    q_m[3][0] = 0.0f;

    q_m[0][1] = Q[1][0];
    q_m[1][1] = Q[1][1];
    q_m[2][1] = Q[1][2];
    q_m[3][1] = 0.0f;

    q_m[0][2] = -Q[2][0];
    q_m[1][2] = -Q[2][1];
    q_m[2][2] = -Q[2][2];
    q_m[3][2] = 0.0f;

    q_m[0][3] = 0.0f;
    q_m[1][3] = 0.0f;
    q_m[2][3] = 0.0f;
    q_m[3][3] = 1.0f;

    glLoadMatrixf((float *)q_m);

    glTranslatef(-m->eye[m->cureye][0],
                 -m->eye[m->cureye][1],
                 -m->eye[m->cureye][2]);

}

bool TEditWindow::glClickOnEdgeKnob(int clickX, int clickY)
{
    if (!set.Map_Read)
        return false;
    map *m = map_i[set.curmap];
    if (!m || m->numSelected() == 0)
        return false;

    wglMakeCurrent(ghDC, ghRC);

    GLint width, height;
    GLfloat aspect;
    GLint hitIndex;

    RECT rc;
    GetClientRect(hwnd,&rc);
    width = rc.right-rc.left;
    height = rc.bottom-rc.top;
    aspect = (GLfloat)width/(GLfloat)height;

    GLint vp[4];
    GLuint buffer[512];	//selection buffer
    memset(buffer,0,512*sizeof(GLuint));

    glSelectBuffer(512,buffer);
    glGetIntegerv(GL_VIEWPORT,vp);
    // switch to pick mode...

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPickMatrix(clickX, (EditSize.cy - clickY), 5, 5, vp);
    gluPerspective(set.fov, aspect, NEARZ, FARZ);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    b_glSetMatrix(m);

    glRenderMode(GL_SELECT);
    glInitNames();
    glPushName(-1);


    //  number all faces!
    //  render them with names...
    //  do at same time...
    //mapRenderNumber(m);
    SetBrush *b;
    Entity *e;
    int j2,k;
    face_t *f;
    winding_t *w;
    vec3_t ctr;

    int testIndex = 0;

    for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
    {
        if (!e->modifiable)
            continue;

        for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
        {
            if (!b->IsDrawable())
                continue;

            if (!b->IsSelected())
                continue;

            for (f = b->faces.p_next; f != &b->faces; f = f->p_next)
            {
                w = f->w;
                if (!w)
                    continue;

                k = w->numpoints - 1;
                for (j2 = 0; j2 < w->numpoints; j2++)
                {
                    ctr[0] = (w->points[j2][0] + w->points[k][0])/2.0f;
                    ctr[1] = (w->points[j2][1] + w->points[k][1])/2.0f;
                    ctr[2] = (w->points[j2][2] + w->points[k][2])/2.0f;
                    glLoadName((GLint)(testIndex++));
                    glBegin(GL_POINTS);
                    glVertex3f(ctr[0], ctr[1], ctr[2]);
                    glEnd();
                    k = j2;
                }
            }
        }
    }

    glFlush();
    glPopMatrix();
    // look at the buffer
    // get best
    GLint n_hits;
    n_hits = glRenderMode(GL_RENDER);

    testIndex = 0;
    for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
    {
        if (!e->modifiable)
            continue;

        for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
        {
            if (!b->IsDrawable())
                continue;

            if (!b->IsSelected())
                continue;

            for (f = b->faces.p_next; f != &b->faces; f = f->p_next)
            {
                w = f->w;
                if (!w)
                    continue;

                k = w->numpoints - 1;
                for (j2 = 0; j2 < w->numpoints; j2++)
                {

                    ctr[0] = (w->points[j2][0] + w->points[k][0])/2.0f;
                    ctr[1] = (w->points[j2][1] + w->points[k][1])/2.0f;
                    ctr[2] = (w->points[j2][2] + w->points[k][2])/2.0f;

                    for (int counter = 1; counter <= n_hits; counter++)
                    {
                        hitIndex = buffer[counter*4-1];

                        if (hitIndex == testIndex)    // add control points...
                        {
                            int count = m->countSelectedBrushesWithEdge(w->points[j2],w->points[k]);

                            if (count <= 0)
                            {
                                // restore...
                                glMatrixMode(GL_PROJECTION);
                                glPopMatrix();
                                glMatrixMode(GL_MODELVIEW);
                                return false;
                            }
                            multicontrolpoints = 0;

                            if (mcontrolpoints)
                            {
                                delete[] mcontrolpoints;
                                maxmulticontrolpoints = 0;
                                mcontrolpoints = NULL;
                            }

                            // 2 points per face, 2 faces per edge...
                            mcontrolpoints = new float *[5*count];
                            maxmulticontrolpoints = 5*count;

                            Entity *e2;
                            SetBrush *b2;

                            for (e2 = m->objects.p_next; e2 && (e2 != &m->objects); e2 = e2->p_next)
                            {
                                if (!e2->modifiable)
                                    continue;
                                for (b2 = e2->objects.p_next; b2 && (b2 != &e2->objects); b2 = b2->p_next)
                                {
                                    if (!b2->IsDrawable())
                                        continue;
                                    if (!b2->IsSelected())
                                        continue;
                                    b2->getEdgeDragFaces(w->points[j2],w->points[k]);
                                }
                            }

                            if (!multicontrolpoints)
                            {
                                delete[] mcontrolpoints;
                                maxmulticontrolpoints = 0;
                                mcontrolpoints = NULL;
                                // restore...
                                glMatrixMode(GL_PROJECTION);
                                glPopMatrix();
                                glMatrixMode(GL_MODELVIEW);
                                return false;
                            }
                            m->saveForUndo(const_cast<char *> ("Drag Edge (3d)"),UNDO_BRUSHES);
                            VectorSubtract(w->points[j2],w->points[k],edgeNormal);

                            if (VectorLength(edgeNormal) < 0.0001)
                            {
                                // restore...
                                glMatrixMode(GL_PROJECTION);
                                glPopMatrix();
                                glMatrixMode(GL_MODELVIEW);
                                return false;
                            }

                            VectorNormalize(edgeNormal);
                            // restore...
                            glMatrixMode(GL_PROJECTION);
                            glPopMatrix();
                            glMatrixMode(GL_MODELVIEW);
                            return true;
                        }
                    }
                    testIndex++;
                    k = j2;
                }
            }
        }
    }

    // restore...
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    return false;
}

bool TEditWindow::glPointOnEdgeKnob(int clickX, int clickY)
{

    if (!set.Map_Read)
        return false;

    map *m = map_i[set.curmap];
    if (!m)
        return false;

    if (m->numSelected() == 0)
        return false;

    wglMakeCurrent(ghDC, ghRC);

    GLint width, height;
    GLfloat aspect;
    GLint hitIndex;


    RECT rc;
    GetClientRect(hwnd,&rc);
    width = rc.right;
    height = rc.bottom;
    aspect = (GLfloat)width/(GLfloat)height;

    GLint vp[4];
    GLuint buffer[512];		//selection buffer
    memset(buffer,0,512*sizeof(GLuint));

    glSelectBuffer(512,buffer);
    glGetIntegerv(GL_VIEWPORT,vp);
    // switch to pick mode...

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPickMatrix(clickX, (EditSize.cy - clickY), 5, 5, vp);
    gluPerspective(set.fov, aspect, NEARZ, FARZ);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    b_glSetMatrix(m);

    glRenderMode(GL_SELECT);
    glInitNames();
    glPushName(-1);


    //  number all faces!
    //  render them with names...
    //  do at same time...
    //mapRenderNumber(m);
    SetBrush *b;
    Entity *e;
    int j2,k;
    face_t *f;
    winding_t *w;
    vec3_t ctr;
    int i;

    int testIndex = 0;
    for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
    {
        if (!e->modifiable)
            continue;

        for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
        {
            if (!b->IsDrawable())
                continue;

            if (!b->IsSelected())
                continue;

            for (f = b->faces.p_next; f != &b->faces; f = f->p_next)
            {
                w = f->w;
                if (!w)
                    continue;

                k = w->numpoints - 1;
                for (j2 = 0; j2 < w->numpoints; j2++)
                {
                    ctr[0] = (w->points[j2][0] + w->points[k][0])/2.0f;
                    ctr[1] = (w->points[j2][1] + w->points[k][1])/2.0f;
                    ctr[2] = (w->points[j2][2] + w->points[k][2])/2.0f;

                    glLoadName((GLint)(testIndex++));
                    glBegin(GL_POINTS);
                    glVertex3f(ctr[0], ctr[1], ctr[2]);
                    glEnd();
                    k = j2;
                }
            }
        }
    }

    glFlush();
    glPopMatrix();
    // look at the buffer
    // get best
    GLint n_hits;
    GLuint testVal;
    n_hits = glRenderMode(GL_RENDER);

    if (n_hits > 0)
    {
        hitIndex = buffer[3];
        GLuint val = buffer[1];
        for (i = 2; i <= n_hits; i++)
        {
            testVal = buffer[i*4-3];
            if (testVal < val)
            {
                val = testVal;
                hitIndex = buffer[i*4-1];
            }
        }
    }
    else
    {
        hitIndex = 0;
    }

    // restore...
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    return (hitIndex != 0);
}


void TEditWindow::UpdateOtherViews()
{
    if (!set.Map_Read)
        return;
    if (!set.track3dinxy)
        return;

    if (set.track3dincurrentxy)
    {
        xyWindow[set.curxy]->UpdateSelected(true);
    }
    else
    {
        for (int i = 0; i < set.xyViews; i++)
        {
            xyWindow[i]->UpdateSelected(true);
        }
    }
}

// calculates stretchdibits target rectangles for current window
// size and render internal size...
void TEditWindow::CalcBltRects(int scx, int scy, int rx, int ry, RECT *dst, RECT *src)
{
    if (!set.Map_Read)
        return;

    if (!scx || !scy || !rx || !ry)
    {
        src->left = 0;
        src->top = 0;
        src->right = 100;           // Width and Height not "right" side
        src->bottom = 100;

        dst->left = 0;
        dst->top = 0;
        dst->right = 100;            // Width and Height not "right" side
        dst->bottom = 100;
        return;
    }
    src->left = src->top = 0;
    src->right = rx;
    src->bottom = ry;

    dst->left = 0;
    dst->top = 0;
    dst->right = scx;            // Width and Height not "right" side
    dst->bottom = scy;
}

bool TEditWindow::bSetupPixelFormat(HDC hdc)
{
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd,0,sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;

    if (!singleBuffer)
        pfd.dwFlags |= PFD_DOUBLEBUFFER;

    pfd.cColorBits = (BYTE)cColorBits;
    pfd.cDepthBits = (BYTE)cDepthBits;

    if(set.gl_enable_stencil)
        pfd.cStencilBits = 1;

    if(splash)
        splash->SetText(const_cast<char *> ("Calling ChoosePixelFormat"));

    int pixelformat = ChoosePixelFormat(hdc, &pfd);

    if (!pixelformat)
    {
        MessageBox(hwnd,"ChoosePixelFormat failed", "Error", MB_OK);
        return false;
    }

    if (pfd.dwFlags & PFD_NEED_PALETTE)
    {
        MessageBox(hwnd,"Needs palette", "Error", MB_OK);
        return false;
    }

    if(splash)
        splash->SetText(const_cast<char *> ("Calling SetPixelFormat"));
    if (SetPixelFormat(hdc, pixelformat, &pfd) == FALSE)
    {
        MessageBox(hwnd,"SetPixelFormat failed", "Error", MB_OK);
        return false;
    }

    return true;
}

/* OpenGL code */

void TEditWindow::resize()
{
    RECT rect;
    GetClientRect(hwnd, &rect);

    GLfloat width = (float) rect.right;
    GLfloat height = (float) rect.bottom;
    GLfloat aspect;

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    aspect = width / height;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(set.fov, aspect, NEARZ, FARZ);
    glMatrixMode(GL_MODELVIEW);
}

void TEditWindow::initializeGL()
{
    glSetBackgroundColor(set.color_3dbackground);

    glClearDepth(1.0f);

    glDepthFunc(GL_LESS);	//GL_LEQUAL

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glClearStencil(0);
    glEnable(GL_CULL_FACE);
    glDisable(GL_DITHER);

    glColorMaterial ( GL_FRONT, GL_EMISSION ) ;
    glColorMaterial ( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
    glEnable(GL_COLOR_MATERIAL);

    //fog
    if(set.gl_fog)
    {
        glFogOn();
        if(set.gl_fog_match_bg)
            glSetFogColor(set.color_3dbackground);
        else
            glSetFogColor(set.gl_fog_color);
        glSetFogRange((float)set.gl_fog_near, (float)set.gl_fog_far);
    }

    //light
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightAmbient(set.gl_ambient_color);
    glLightColor(set.gl_light_color);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // GL_NICEST
    resize();
}

void TEditWindow::getRenBrush(int clickX, int clickY)
{
    if (!set.Map_Read)
        return;
    if (!texWindow)
        return;
    map *m = map_i[set.curmap];
    if (!m)
        return;

    wglMakeCurrent(ghDC, ghRC);

    GLint width, height;
    GLfloat aspect;

    RECT rc;
    GetClientRect(hwnd,&rc);
    width = rc.right-rc.left;
    height = rc.bottom-rc.top;
    aspect = (GLfloat)width/(GLfloat)height;

    GLint vp[4];
    GLuint buffer[512];	//selection buffer
    memset(buffer,0,512*sizeof(GLuint));

    glSelectBuffer(512,buffer);
    glGetIntegerv(GL_VIEWPORT,vp);
    // switch to pick mode...

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPickMatrix(clickX, (EditSize.cy - clickY), 1, 1, vp);
    gluPerspective(set.fov, aspect, NEARZ, FARZ);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    b_glSetMatrix(m);

    glRenderMode(GL_SELECT);
    glInitNames();
    glPushName(-1);


//  number all faces!
//  render them with names...
//  do at same time...
    mapRenderNumber(m);

    glFlush();
    glPopMatrix();
    // look at the buffer
    // get best
    GLuint testVal;
    GLint hitIndex = 0;
    GLint n_hits = glRenderMode(GL_RENDER);

    if (n_hits > 0)
    {
        hitIndex = buffer[3];
        GLuint val = buffer[1];
        for (int i = 2; i <= n_hits; i++)
        {
            testVal = buffer[i*4-3];
            if (testVal < val)
            {
                val = testVal;
                hitIndex = buffer[i*4-1];
            }
        }
    }

    // restore...
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    if (hitIndex != 0)
    {
        setRenderBrush(m,hitIndex);
    }
}

void TEditWindow::setRenderBrush(map *m, int hitIndex)
{
    Entity *e;
    SetBrush *b;

    for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
    {
        for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
        {
            if (!b->IsDrawable())
                continue;

            if ((hitIndex < (int)b->brushId) ||
                    (hitIndex >= ((int)b->brushId+b->CountFaces())))
                continue;

            // found it...
            REN_bestBrush = b;
            int ofs = (hitIndex - (int)b->brushId);
            face_t *f;
            for (f = b->faces.p_next; f != &b->faces; f = f->p_next)
            {
                LoopProblem("setrendbrush");

                if (ofs-- <= 0)
                {
                    REN_bestFace = f;
                    break;
                }
            }
            return;
        }
    }
}

void TEditWindow::mapRenderNumber(map *m)
{
    int	j;
    Entity *e;
    SetBrush *b;
    winding_t *w;
    float *pt;
    qtexture_t	*q;
    face_t *face;
    int REN_testFaceInt = 1;
    for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
    {
        for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
        {
            if (!b->IsDrawable())
                continue;

            b->brushId = REN_testFaceInt;
            //prevents crash (fills null pointer) but is WRONG: REN_bestFace = &b->faces;
            int i = 0;
            for (face = b->faces.p_next; face != &b->faces; face = face->p_next, i++)
            {
                LoopProblem("brush mrnumber");
                if (!face->qtexture)
                    face->qtexture = texWindow->GetQTexture(face->texture.texture);

                q = face->qtexture;
                if (!q)
                    continue;

                w = face->w;
                if (!w)
                    continue;

                glLoadName((GLint)(REN_testFaceInt++));
                glBegin(GL_TRIANGLE_FAN);
                for (j = 0; j < w->numpoints; j++)
                {
                    pt = w->points[j];
                    glVertex3f(pt[0],pt[1],pt[2]);
                }
                glEnd();
            }
        }
    }
}

extern void beztest();
void TEditWindow::glRenderScene(map *m)
{
    Entity *e;
    SetBrush *b;

    //debug
    //beztest();

    glSkyboxNotEmpty = false;

    //begin poly offset
    if (set.outline != 0 && set.drawmode != Drawmode::wire)
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(0.1f, 0.1f);
    }
    // first the modifiable brushes...

    //textured
    if(set.drawmode == Drawmode::texture)
    {
        glEnable(GL_TEXTURE_2D);
        glLastBoundIndex = -1;

        if(set.render_trans)  	// trans mode
        {
            for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
            {
                if (!e->modifiable)
                    continue;
                for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
                {
                    if (!b->IsDrawable())
                        continue;
                    if (glUnselectedOnly && b->IsSelected())
                        continue;
                    b->glCameraRenderSelfTextureTrans(true);
                }
            }
        }
        else     // reg solid
        {
            for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
            {
                if (!e->modifiable)
                    continue;
                for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
                {
                    if (!b->IsDrawable())
                        continue;
                    if (glUnselectedOnly && b->IsSelected())
                        continue;
                    b->glCameraRenderSelfTexture();
                }
            }
        }
        glDisable(GL_TEXTURE_2D);  // okay we've rendered the textured brushes

        //draw flat mode
    }
    else if(set.drawmode == Drawmode::flat)
    {

        if(set.render_trans)  	// trans mode
        {
            for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
            {
                if (!e->modifiable)
                    continue;
                for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
                {
                    if (!b->IsDrawable())
                        continue;
                    if (glUnselectedOnly && b->IsSelected())
                        continue;
                    b->glCameraRenderSelfFlatTrans(true);
                }
            }
        }
        else     // regular solid
        {
            for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
            {
                if (!e->modifiable)
                    continue;
                for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
                {
                    if (!b->IsDrawable())
                        continue;
                    if (glUnselectedOnly && b->IsSelected())
                        continue;
                    b->glCameraRenderSelfFlat();
                }
            }
        }
        //draw wireframe mode
    }
    else if(set.drawmode == Drawmode::wire)
    {

        for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
        {
            if (!e->modifiable)
                continue;
            for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
            {
                if (!b->IsDrawable())
                    continue;
                if (glUnselectedOnly && b->IsSelected())
                    continue;
                b->glCameraRenderSelfWireframe();
            }
        }
    }


    // now do the unmodifiables... (solid)
    if(!set.gl_trans_ents)
    {
        if(set.texture_models)
        {
            for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
            {
                if (e->modifiable)
                    continue;
                for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
                {
                    if (!b->IsDrawable())
                        continue;
                    if (glUnselectedOnly && b->IsSelected())
                        continue;
                    b->glCameraRenderSelfFixed();
                }
            }
        }
    }

    //end poly offset
    if (set.outline && set.drawmode != Drawmode::wire)
    {
        glPolygonOffset(0, 0);
        glDisable(GL_POLYGON_OFFSET_LINE);
    }

    // render the unselected outlines
    glLineWidth(1.0f);
    if (set.outline)
    {
        if(set.drawmode != Drawmode::wire)
        {
            //render outlines on everything
            for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
            {
                for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
                {
                    if (!b->IsDrawable())
                        continue;
                    if (b->IsSelected())
                        continue;
                    b->glCameraRenderSelfOutline();
                }
            }
        }
        else
        {
            //render outlines on entities only
            for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
            {
                for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
                {
                    if (!b->IsDrawable())
                        continue;
                    if (b->IsSelected())
                        continue;
                    if (e->modifiable)
                        continue;
                    b->glCameraRenderSelfOutline();
                }
            }
        }
    }
    // Now go through and render the selected outlines
    glLineWidth((float)set.selection_thickness);
    for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
    {
        for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
        {
            if (!b->IsDrawable())
                continue;
            if (!b->IsSelected())
                continue;
            if (glUnselectedOnly)
                continue;

            if(set.gl_selection_wiremode == 0)
            {
                //backface cull selection wireframe:
                glDepthFunc(GL_LEQUAL);
                b->glCameraRenderSelfSelected();
                //restore depth mode
                glDepthFunc(GL_LESS);
            }
            else if(set.gl_selection_wiremode == 1)
            {
                //no backface cull:

                //render enter wireframe
                glDepthFunc(GL_ALWAYS);
                b->glCameraRenderSelfSelected();
                //restore depth mode
                glDepthFunc(GL_LESS);
            }
            else if(set.gl_selection_wiremode == 2)
            {

                /*	BUGGY!! :( TODO
                	This code draws the selection wireframe culled, and hidden outlines with the brush
                	outline color. Problem is, the outer edges are drawn twice, creating fuzzy and
                	ugly edges. Need a way to draw ONLY completely hidden lines for this to work.

                	more specifically, hidden outlines get the "GREATER" pixels, while visible outlines
                	get "LEQUAL". there is no way with this drawing method to control pixels
                	against this boundary.....
                */

                //render hidden outlines
                glDepthMask(GL_FALSE);
                glDepthFunc(GL_GREATER);
                b->glCameraRenderSelfOutline();
                glDepthMask(GL_TRUE);

                //render visible outlines
                glDepthFunc(GL_LEQUAL);
                b->glCameraRenderSelfSelected();

                //restore depth mode
                glDepthFunc(GL_LESS);
            }
        }
    }

    //draw current face last for special selection wiremodes
    if(set.gl_selection_wiremode > 0 && REN_curBrush && REN_curBrush->currentFace && REN_curBrush->parent->modifiable)
    {
        glDepthFunc(GL_ALWAYS);
        glShadingOff();
        winding_t *w = REN_curBrush->currentFace->w;
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
        glShadingOn();
        glDepthFunc(GL_LESS);
    }


    //render skybox after regular geometry
    if(glSkyboxNotEmpty)
    {
        glBeginAddToSkybox();
        for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
        {
            if (!e->modifiable)
                continue;
            for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
            {
                if (!b->IsDrawable())
                    continue;
                if (glUnselectedOnly && b->IsSelected())
                    continue;
                b->glCameraRenderSkybox();
            }
        }
        glEndAddToSkybox();
        glRenderSkybox();
    }

    //render entities as transparent
    if(set.gl_trans_ents)
    {
        glEnable(GL_BLEND);
        // now do the unmodifiables...
        if(set.texture_models)
        {
            for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
            {
                if (e->modifiable)
                    continue;
                for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
                {
                    if (!b->IsDrawable())
                        continue;
                    if (glUnselectedOnly && b->IsSelected())
                        continue;
                    b->glCameraRenderSelfFixed();
                }
            }
        }
        glDisable(GL_BLEND);
    }
    // render transparencies on top of skyboxes
    if(set.render_trans)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); //<-ADDITIVE | non-add is nice but req sorting: (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//
        glDepthMask(GL_FALSE);

        int oldshading = set.gl_shading;
        set.gl_shading = 0;

        glDisable(GL_LIGHTING);

        if(set.drawmode == Drawmode::texture)
        {

            glEnable(GL_TEXTURE_2D);
            glLastBoundIndex = -1;

            for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
            {
                if (!e->modifiable)
                    continue;
                for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
                {
                    if (!b->IsDrawable())
                        continue;
                    if (glUnselectedOnly && b->IsSelected())
                        continue;
                    b->glCameraRenderSelfTextureTrans(false);
                }
            }

            glDisable(GL_TEXTURE_2D);  // okay we've rendered the textured brushes
        }
        else if(set.drawmode == Drawmode::flat)
        {

            for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
            {
                if (!e->modifiable)
                    continue;
                for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
                {
                    if (!b->IsDrawable())
                        continue;
                    if (glUnselectedOnly && b->IsSelected())
                        continue;
                    b->glCameraRenderSelfFlatTrans(false);
                }
            }
        }
        set.gl_shading = oldshading;
        glShadingOn();

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }
    glLineWidth(1.0f);
}
void TEditWindow::drawScene()
{
    if (!set.Map_Read)
        return;

    if (!texWindow)
        return;

    map *m = map_i[set.curmap];
    if (!m)
        return;

    wglMakeCurrent(ghDC, ghRC);

    glClearDepth(1.0f);
    glDepthFunc(GL_LESS);	//GL_LEQUAL
    glDisable(GL_BLEND);
    glFrontFace(GL_CW); // default GL_CCW

    if(set.gl_enable_stencil)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    else
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();

    b_glSetMatrix(m);

    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);

    glShadingOff();
    if(set.show_3d_grid)
        glDraw3DGrid();
    if(set.show_world_axes)
        glDrawAxes();
    glShadingOn();

    brushCount = 0; // reset brush count

    // if wire or flat fine, if rendered,
    // then do here in 2 passes, one with texture mapping and one if outline enabled.
    if (glBspRotate)
    {
        glUnselectedOnly = true;
        glRenderScene(glMap);
        glUnselectedOnly = false;
    }
    glRenderScene(m);

    //xxxmap_i[set.curmap]->clipper_i->cameraDrawSelf(glDC);
    ShowLeaks();
    ShowPortals();

    m->clipper_i->cameraDrawSelf(NULL);

    glPopMatrix();

    m->glDrawOrientation();
    m->glBrightness();


    if (singleBuffer)
        glFinish();
    else
        SwapBuffers(ghDC);
}

void TEditWindow::ConstructBuffers(int w, int h)
{
    int size = w*h + w + 1;

    zbuffer = new float[size*sizeof(float)];	//zbuffer1
    t_zbuffer = new float[size*sizeof(float)];	//zbuffer1t

    if (set.sinBsp)
        size *= 3;
    imagebuffer = new unsigned char[size];	//frame buffer1
    t_imagebuffer = new unsigned char[size];	//frame buffer1t
}

void TEditWindow::RemoveBuffers()
{
    delete[] imagebuffer;
    delete[] zbuffer;
    delete[] t_imagebuffer;
    delete[] t_zbuffer;

    imagebuffer = NULL;
    zbuffer = NULL;
    t_imagebuffer = NULL;
    t_zbuffer = NULL;
}

void TEditWindow::StoreBuffers()
{
    if (zbuffer && imagebuffer && t_zbuffer && t_imagebuffer)
    {
        int size = r_width * r_height;
        memcpy(t_zbuffer,zbuffer,size * sizeof(float));
        if (set.sinBsp) size *= 3;
        memcpy(t_imagebuffer,imagebuffer,size);
    }
}

void TEditWindow::RestoreBuffers()
{
    if (t_zbuffer && t_imagebuffer && zbuffer && imagebuffer)
    {
        int size = r_width * r_height;
        memcpy(zbuffer,t_zbuffer,size * sizeof(float));
        if (set.sinBsp) size *= 3;
        memcpy(imagebuffer,t_imagebuffer,size);
    }
}

void TEditWindow::TimeRefresh()
{
    if (IsIconic())
        return;
    if (!set.Map_Read)
        return;
    if (!hmemdc || !hmembitmap)
        return;
    map *m = map_i[set.curmap];
    if(!m)
        return;

    HDC hdc = 0;
    if(!set.glBsp)
    {
        ::GdiFlush();

        hdc = GetDC(hwnd);

        ::SetStretchBltMode(hdc, COLORONCOLOR);
    }

    DWORD startRender, stopRender;
    DWORD startRest, stopRest;

    int frameNumber = 0;
    const int numSteps = 30;

    float renderTime = 0.0f;
    float restTime = 0.0f;

    float yaw = m->angles->yaw; //0.0;
    const float startyaw = m->angles[m->cureye].yaw;
    const float angStep = 2*M_PI/(float)numSteps;

    REN_curBrush = m->selectedBrush();
    int counter;

    for (counter = 0; counter < numSteps; counter++)
    {
        startRest = GetTickCount();

        if (set.glBsp)
        {
            startRender = GetTickCount();
            m->angles[m->cureye].yaw = yaw;
            drawScene();
            stopRender = GetTickCount();
        }
        else
        {
            //
            // draw it
            //
            VectorCopy(m->eye[m->cureye],r_origin);
            vec3_t zero;
            VectorCopy(vec3_origin,zero);
            float Q[4][4];
            float Temp[4][4];

            findQ(Q,Temp,zero,
                  m->angles[m->cureye].roll,
                  m->angles[m->cureye].pitch,
                  yaw
                 );

            // vpn
            r_matrix[0][0] = Q[0][0];
            r_matrix[0][1] = Q[0][1];
            r_matrix[0][2] = Q[0][2];

            r_matrix[1][0] = Q[1][0];
            r_matrix[1][1] = Q[1][1];
            r_matrix[1][2] = Q[1][2];

            r_matrix[2][0] = Q[2][0];
            r_matrix[2][1] = Q[2][1];
            r_matrix[2][2] = Q[2][2];

            r_width = set.render_width; // EditSize.cx;
            r_height = set.render_height; // EditSize.cy;

            r_picbuffer = imagebuffer;
            r_zbuffer = zbuffer;

            r_drawflat = (set.drawmode == Drawmode::flat);
            r_drawwire = (set.drawmode == Drawmode::wire);
            //
            //r_drawwire = true;
            REN_BeginCamera ();
            REN_ClearBuffers ();

            //
            // render the setbrushes
            //
            brushCount = 0;
            startRender = GetTickCount();
            m->makeAllPerform(SEL_CAMERARENDERSELF);
            stopRender = GetTickCount();

            //
            // display the output
            //
            ShowLeaks();
            ShowPortals();

            m_pBmInfoHeader.biWidth = set.render_width;
            m_pBmInfoHeader.biHeight = -set.render_height;
            memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
            if (set.sinBsp)
            {
                m_pBmInfoHeader.biSizeImage = 0;
                memset(m_pBmInfo->bmiColors,0,256*sizeof(RGBQUAD));
            }
            else
            {
                m_pBmInfoHeader.biSizeImage = set.render_width*set.render_height;
                memcpy(m_pBmInfo->bmiColors,set.cTable,256*sizeof(RGBQUAD));
            }

            RECT dest, source;

            CalcBltRects(EditSize.cx, EditSize.cy, set.render_width, set.render_height, &dest, &source);

            if(set.render_auto)
            {
                StretchDIBits(
                    hmemdc,
                    //	dest.left, dest.top, source.right, source.bottom,
                    source.left, source.top, source.right, source.bottom,
                    source.left, source.top, source.right, source.bottom,
                    imagebuffer,
                    m_pBmInfo,
                    DIB_RGB_COLORS,
                    SRCCOPY);

            }
            else
            {
                StretchDIBits(
                    hmemdc,
                    dest.left, dest.top, dest.right, dest.bottom,
                    source.left, source.top, source.right, source.bottom,
                    imagebuffer,
                    m_pBmInfo,
                    DIB_RGB_COLORS,
                    SRCCOPY);
            }

            m->clipper_i->cameraDrawSelf(hmemdc);

            m->DrawOrientation(hmemdc);

            ::BitBlt(hdc,EditClip.left,EditClip.top,EditClip.right-EditClip.left,EditClip.bottom-EditClip.top,hmemdc,0,0,SRCCOPY);
        }

        stopRest = GetTickCount();
        restTime += (stopRest - startRest);///1000;//CLK_TCK;
        renderTime += (stopRender - startRender);///1000;//CLK_TCK;

        frameNumber++;
        yaw+= angStep;
    }

    if(!set.glBsp)
        ::ReleaseDC(hwnd,hdc);

    char outstuff[128];
    float restS = restTime /1000.0f / (float)frameNumber;
    float renderS = renderTime /1000.0f / (float)frameNumber;

    if (!restS)
        restS = 1.0f;
    if (!renderS)
        renderS = 1.0f;

    sprintf(outstuff,"Total %f ms/f %f fps\tRender %f ms/f %f fps",
            1000.0f*restS, 1.0f/restS,
            1000.0f*renderS, 1.0f/renderS);

    m->angles[m->cureye].yaw = startyaw;
    RedrawContents();

    MessageBox(hwnd,outstuff,"Frame Timing",MB_OK);

    RedrawContents();
}

void TEditWindow::SetNumPortals()
{
    if (!set.Map_Read || !set.track_portals)
        return;
    if (numLeakPortals <= 0 || !aLeakPortals)
        return;

    char prompt[80];
    sprintf(prompt,"Maximum number of portals [1-%i]?",numLeakPortals);
    char s[MAX_PATH];

    sprintf(s,"%i",numLeakPortals);

    if (IDOK != InputDialog(hwnd,const_cast<char *> ("BSP - Set Max Portals"),prompt,s,sizeof(s)).Execute())
        return;

    int index = atoi(s);

    if (index < 1)
        return;

    if (index > numLeakPortals)
        index = numLeakPortals;

    maxPortals = index;

    RedrawContents();
}

void TEditWindow::KillPortals()
{
    if (!set.Map_Read)
        return;

    CmStopPortals();
}

void TEditWindow::MNormal()
{
    modal = NO_MOUSE;
}
void TEditWindow::MSelect()
{
    modal = SELECT_CALLBACK;
}
void TEditWindow::MSelectOne()
{
    modal = SELECTONE_CALLBACK;
}
void TEditWindow::MShift()
{
    modal = SHIFT_TEXTURE;
}
void TEditWindow::MStretch()
{
    modal = STRETCH_TEXTURE;
}
void TEditWindow::MRotate()
{
    modal = ROTATE_TEXTURE;
}
void TEditWindow::MApplyToBrush()
{
    modal = TEXTUREBRUSH_CALLBACK;
}
void TEditWindow::MApplyToFace()
{
    modal = TEXTUREFACE_CALLBACK;
}
void TEditWindow::MLift()
{
    modal = LIFT_TEXTURE;
}
void TEditWindow::MLiftApply()
{
    modal = LIFT_APPLY;
}
void TEditWindow::MDragFace()
{
    modal = CLICKFACEDRAG;
}
void TEditWindow::MShearFace()
{
    modal = CLICKFACESHEAR;
}
void TEditWindow::MDragBrushes()
{
    modal = DRAGBRUSHES;
}
void TEditWindow::MDragBrushesSide()
{
    modal = DRAGBRUSHESSIDE;
}
void TEditWindow::MSetClipper()
{
    modal = SETCLIPPER;
}
void TEditWindow::MSetClipperInv()
{
    modal = SETCLIPPERINV;
}
void TEditWindow::MNavigate()
{
    modal = NAVIGATE;
}
void TEditWindow::MNavigateStrafe()
{
    modal = NAVIGATE_STRAFE;
}
void TEditWindow::MEdgeDrag()
{
    modal = EDGEDRAG;
}
void TEditWindow::MLook()
{
    modal = MLOOK;
}
void TEditWindow::NewMouse(int mm)
{
    if ((mm < 0) || (mm >= numEventTypes3d))
        return;

    curEventType3d = mm;
}

void TEditWindow::M1()
{
    NewMouse(0);
}
void TEditWindow::M2()
{
    NewMouse(1);
}
void TEditWindow::M3()
{
    NewMouse(2);
}
void TEditWindow::M4()
{
    NewMouse(3);
}
void TEditWindow::M5()
{
    NewMouse(4);
}
void TEditWindow::M6()
{
    NewMouse(5);
}
void TEditWindow::M7()
{
    NewMouse(6);
}
void TEditWindow::M8()
{
    NewMouse(7);
}
void TEditWindow::M9()
{
    NewMouse(8);
}
void TEditWindow::M10()
{
    NewMouse(9);
}

void TEditWindow::MouseHelp()
{
    BSPHelp(const_cast<char *> ("3D Mouse Settings..."),const_cast<char *> (""));
    BSPHelpAddV(
        const_cast<char *> ("Current 3d Window Mouse Settings\r\n"
                            "%s\r\n"
                            "--------------------------------------------------------------\r\n"),
        EventTypes3d[curEventType3d]);

    mouseEventType *p = head3d[curEventType3d];

    int counter = 1;
    while (p)
    {
        static const char *buttons[] = { "Left", "Middle", "Right", "X1", "X2" };

        char keys[40];
        int fl = 1;
        if ((p->moveFlags & CTRL_FLAG) && (p->moveFlags & SHIFT_FLAG) && (p->moveFlags & ALT_FLAG))
        {
            strcpy(keys,"Ctrl+Alt+Shift");
        }
        else if ((p->moveFlags & SHIFT_FLAG) && (p->moveFlags & CTRL_FLAG))
        {
            strcpy(keys,"Ctrl+Shift");
        }
        else if ((p->moveFlags & CTRL_FLAG) && (p->moveFlags & ALT_FLAG))
        {
            strcpy(keys,"Ctrl+Alt");
        }
        else if ((p->moveFlags & SHIFT_FLAG) && (p->moveFlags & ALT_FLAG))
        {
            strcpy(keys,"Shift+Alt");
        }
        else if (p->moveFlags & SHIFT_FLAG)
        {
            strcpy(keys,"Shift");
        }
        else if (p->moveFlags & CTRL_FLAG)
        {
            strcpy(keys,"Ctrl");
        }
        else if (p->moveFlags & ALT_FLAG)
        {
            strcpy(keys,"Alt");
        }
        else
        {
            fl = 0;
        }

        if (fl)
            BSPHelpAddV(const_cast<char *> ("%i\t%s %s Click:\r\n"),counter,keys,buttons[p->whichMouse]);
        else
            BSPHelpAddV(const_cast<char *> ("%i\t%s Click:\r\n"),counter,buttons[p->whichMouse]);

        for (int i = 0; i < p->count; i++)
        {
            BSPHelpAddV(const_cast<char *> ("\t%s\r\n"),p->moves[i]);
        }
        BSPHelpAdd(const_cast<char *> ("\r\n"));

        p = p->next;
        counter++;
    }
}


// ASSUMES THAT R_ values are set
void TEditWindow::ShowLeaks()
{
    if (!set.Map_Read || !set.track_leaks)
        return;
    if (numLeakPoints <= 0 || !aLeakPoints)
        return;

    if (set.glBsp)
    {
        glLineWidth(3);
        glEnableClientState(GL_VERTEX_ARRAY);
        glColor3ub(255,255,0);
        glVertexPointer(3,GL_FLOAT,0,aLeakPoints);
        glDrawArrays(GL_LINE_STRIP, 0, numLeakPoints);
        glDisableClientState(GL_VERTEX_ARRAY);
        glLineWidth(1);
    }
    else
    {
        winding_t *w = NewWinding(2);
        if(!w) return;

        pixel32_t oldfc = r_flatcolor;
        pixel32_t oldec = r_entitycolor;

        pixel32_t leakColor;
        leakColor.rgb = RGB(255,0,255);
        leakColor.p = GetPaletteIndex(256,set.pal,leakColor.rgb);

        r_drawent = true;
        t_coloroffset = 31*256;
        r_flatcolor = leakColor;
        r_entitycolor = leakColor;

        for (int counter = 1; counter < numLeakPoints; counter++)
        {
            // cycle through aLeakPoints...
            VectorCopy(aLeakPoints[counter-1],w->points[0]);
            VectorCopy(aLeakPoints[counter]  ,w->points[1]);
            REN_DrawLineWinding(w,0);
        }
        delete [] w;
        memcpy(&r_flatcolor,&oldfc,sizeof(pixel32_t));
        memcpy(&r_entitycolor,&oldec,sizeof(pixel32_t));
    }
}

void TEditWindow::glDraw3DGrid()
{

    glColor3ubv((GLubyte*) &set.color_3dgrid);
    if(set.grid_style != GRID_LINES)  			//dots
    {
        glBegin(GL_POINTS);
        GLfloat pt[] = {0,0,0};
        for(int j=64*32; j>=0; j-=64)
        {
            for(int i=64*32; i>=0; i-=64)
            {
                if(set.show_world_axes && (!i || !j))
                    continue;
                pt[0] = (float)i;
                pt[1] = (float)j;
                glVertex3fv(pt);
                pt[0] = (float)-i;
                pt[1] = (float)j;
                glVertex3fv(pt);
                pt[0] = (float)i;
                pt[1] = (float)-j;
                glVertex3fv(pt);
                pt[0] = (float)-i;
                pt[1] = (float)-j;
                glVertex3fv(pt);
            }
        }
        glEnd();
    }
    else  					//lines
    {

        GLfloat startx[] = {-2048,-2048,0};
        GLfloat endx[] =   { 2048,-2048,0};
        GLfloat starty[] = {-2048,-2048,0};
        GLfloat endy[] =   {-2048, 2048,0};

        glBegin(GL_LINES);
        for(int i=0; i<=64; i++)
        {

            //don't draw lines over axes
            if(i!=32 || !set.show_world_axes)
            {
                //x
                glVertex3fv(startx);
                glVertex3fv(endx);
                //y
                glVertex3fv(starty);
                glVertex3fv(endy);
            }

            startx[1] = startx[1] + 64;
            endx[1] = endx[1] + 64;

            starty[0] = starty[0] + 64;
            endy[0] = endy[0] + 64;
        }
        glEnd();
    }
}

void TEditWindow::glDrawAxes()
{
    GLfloat start[] = {0,0,0};

    glBegin(GL_LINES);
    //+x
    glColor3ub(255,0,0);
    glVertex3fv(start);
    glVertex3f(2048.0f, 0.0f, 0.0f);
    //-x
    glColor3ub(128,0,0);
    glVertex3fv(start);
    glVertex3f(-2048.0f, 0.0f, 0.0f);
    //+y
    glColor3ub(0,0,255);
    glVertex3fv(start);
    glVertex3f(0.0f, 2048.0f, 0.0f);
    //-y
    glColor3ub(0,128,128);
    glVertex3fv(start);
    glVertex3f(0.0f, -2048.0f, 0.0f);
    //+z
    glColor3ub(255,255,0);
    glVertex3fv(start);
    glVertex3f(0.0f, 0.0f, 2048.0f);
    //-z
    glColor3ub(155,128,0);
    glVertex3fv(start);
    glVertex3f(0.0f, 0.0f, -2048.0f);
    glEnd();
}

void TEditWindow::DrawAxes()
{
    winding_t *w = NewWinding(2);
    if(!w) return;

    //overrides color
    r_drawent = true;

    pixel32_t color;
    pixel32_t oldfc = r_flatcolor;
    pixel32_t oldec = r_entitycolor;

    t_coloroffset = 31*256;

    vec3_t start = {0,0,0};
    vec3_t vx = {2048,0,0};
    vec3_t vy = {0,2048,0};
    vec3_t vz = {0,0,2048};

    //+x
    color.rgb = RGB(255,0,0);
    color.p = GetPaletteIndex(256,set.pal,color.rgb);

    r_flatcolor = color;
    r_entitycolor = color;

    VectorCopy(start,w->points[0]);
    VectorCopy(vx,w->points[1]);
    REN_DrawLineWinding(w,0);

    //-x
    vx[0] = -vx[0];

    color.rgb = RGB(128,0,0);
    color.p = GetPaletteIndex(256,set.pal,color.rgb);

    r_flatcolor = color;
    r_entitycolor = color;

    VectorCopy(start,w->points[0]);
    VectorCopy(vx,w->points[1]);
    REN_DrawLineWinding(w,0);

    //+y
    color.rgb = RGB(0,0,255);
    color.p = GetPaletteIndex(256,set.pal,color.rgb);

    r_flatcolor = color;
    r_entitycolor = color;

    VectorCopy(start,w->points[0]);
    VectorCopy(vy,w->points[1]);
    REN_DrawLineWinding(w,0);

    //-y
    vy[1] = -vy[1];

    color.rgb = RGB(0,128,128);
    color.p = GetPaletteIndex(256,set.pal,color.rgb);

    r_flatcolor = color;
    r_entitycolor = color;

    VectorCopy(start,w->points[0]);
    VectorCopy(vy,w->points[1]);
    REN_DrawLineWinding(w,0);


    //+z
    color.rgb = RGB(255,255,0);
    color.p = GetPaletteIndex(256,set.pal,color.rgb);

    r_flatcolor = color;
    r_entitycolor = color;

    VectorCopy(start,w->points[0]);
    VectorCopy(vz,w->points[1]);
    REN_DrawLineWinding(w,0);

    //-z
    vz[2] = -vz[2];

    color.rgb = RGB(128,128,0);
    color.p = GetPaletteIndex(256,set.pal,color.rgb);

    r_flatcolor = color;
    r_entitycolor = color;

    VectorCopy(start,w->points[0]);
    VectorCopy(vz,w->points[1]);
    REN_DrawLineWinding(w,0);

    delete [] w;
    r_flatcolor = oldfc;
    r_entitycolor = oldec;

    r_drawent = false;
}

void PlaneFromWinding (winding_t *w, plane_t *plane)
{
    vec3_t		v1, v2;

    // calc plane
    VectorSubtract (w->points[2], w->points[1], v1);
    VectorSubtract (w->points[0], w->points[1], v2);
    CrossProduct (v2, v1, plane->normal);
    VectorNormalize (plane->normal);
    plane->dist = DotProduct(w->points[0], plane->normal);
}

float CalcWindingArea(winding_t *w)
{
    int		i;
    float	add;
    vec3_t	v1, v2, cross;
    float	total = 0.0f;

    if (w->numpoints >= 3)
    {
        for (i=1 ; i<(w->numpoints - 1) ; i++)
        {
            VectorSubtract (w->points[i], w->points[0], v1);
            VectorSubtract (w->points[i+1], w->points[0], v2);
            CrossProduct (v1, v2, cross);
            add = VectorLength (cross);
            total += add*0.5f;
        }
    }
    return total;
}

bool CheckPortalSize(winding_t *w)
{
    int		i,j;
    vec3_t	size;
    vec3_t mins, maxs;
    float v;

    if (w->numpoints < 3)
        return false;

    mins[0] = mins[1] = mins[2] = TMAX;
    maxs[0] = maxs[1] = maxs[2] = -TMAX;

    for (i=0 ; i<w->numpoints ; i++)
    {
        for (j=0 ; j<3 ; j++)
        {
            v = w->points[i][j];  // fix for ceil on neg numbers...

            if (v < mins[j])
                mins[j] = v;
            if (v > maxs[j])
                maxs[j] = v;
        }
    }
    size[0] = (maxs[0] - mins[0]);
    size[1] = (maxs[1] - mins[1]);
    size[2] = (maxs[2] - mins[2]);

    // 32 32 48
    if ( (size[0] >= 32.0f) &&
            (size[1] >= 32.0f) &&
            (size[2] >= 48.0f) )
        return true;

    return false;
}

// ASSUMES THAT R_ values are set
void TEditWindow::ShowPortals()
{
    if (!set.Map_Read || !set.track_portals)
        return;
    if (numLeakPortals <= 0 || !aLeakPortals)
        return;

    maxPortals = min(numLeakPortals,maxPortals);
    maxPortals = max(maxPortals,1);

    int counter;
    int i;
    vec3_t p1, p2;
    float len;
    bool firstPlayerPortalSet = false;
    bool playerCanFit;

    pixel32_t leakColor;
    pixel32_t areaColor;
    pixel32_t exitColor;
    pixel32_t plugColor;

    pixel32_t oldfc;
    pixel32_t oldec;
    oldfc = r_flatcolor;
    oldec = r_entitycolor;

    leakColor.rgb = RGB(255,0,255);
    leakColor.p = GetPaletteIndex(256,set.pal,leakColor.rgb);

    areaColor.rgb = RGB(0,255,0);
    areaColor.p = GetPaletteIndex(256,set.pal,areaColor.rgb);

    exitColor.rgb = RGB(0,255,255);
    exitColor.p = GetPaletteIndex(256,set.pal,exitColor.rgb);

    plugColor.rgb = RGB(255,0,255);
    plugColor.p = GetPaletteIndex(256,set.pal,plugColor.rgb);

    winding_t *w;

    if (set.glBsp)
    {

        w = NewWinding(2);
        if(!w) return;

        w->numpoints = 2;
        VectorCopy(LeakPortalOrigin,w->points[0]);
        VectorCopy(aLeakPortals[0].centerPoint,w->points[1]);

        r_drawent = true;
        t_coloroffset = 31L*256L;
        r_flatcolor = exitColor;
        r_entitycolor = exitColor;

        glColor3ubv((GLubyte*) &exitColor.rgb);
        glBegin(GL_LINES);
        glVertex3f(w->points[0][0],w->points[0][1],w->points[0][2]);
        glVertex3f(w->points[1][0],w->points[1][1],w->points[1][2]);
        glEnd();

        float area;
        r_flatcolor = areaColor;
        r_entitycolor = areaColor;

        int leakNum = -1;

        for (counter = 0; counter < maxPortals; counter++)
        {
            if (counter)
            {
                VectorCopy(aLeakPortals[counter-1].centerPoint,w->points[0]);
                VectorCopy(aLeakPortals[counter].centerPoint,w->points[1]);

                r_drawent = true;
                t_coloroffset = 31L*256L;

                glColor3ubv((GLubyte*) &set.color_faceoutline);
                glBegin(GL_LINES);
                glVertex3fv(w->points[0]);
                glVertex3fv(w->points[1]);
                glEnd();
            }
            //
            playerCanFit = CheckPortalSize(aLeakPortals[counter].w);
            if (!playerCanFit && !firstPlayerPortalSet)
            {
                firstPlayerPortalSet = true;
                glColor3ubv((GLubyte*) &set.color_lock);
                glBegin(GL_LINE_LOOP);
                for (i = 0; i < aLeakPortals[counter].w->numpoints; i++)
                {
                    glVertex3f(aLeakPortals[counter].w->points[i][0],
                               aLeakPortals[counter].w->points[i][1],
                               aLeakPortals[counter].w->points[i][2]);
                }
                glEnd();
                leakNum = counter;

            }
            else
            {
                area = CalcWindingArea(aLeakPortals[counter].w);
                if (area <= 128.0)
                {
                    glColor3ubv((GLubyte*) &areaColor.rgb);
                    glBegin(GL_LINE_LOOP);
                    for (i = 0; i < aLeakPortals[counter].w->numpoints; i++)
                    {
                        glVertex3f(aLeakPortals[counter].w->points[i][0],
                                   aLeakPortals[counter].w->points[i][1],
                                   aLeakPortals[counter].w->points[i][2]);
                    }
                    glEnd();
                }
                else
                {
                    glBegin(GL_LINE_LOOP);
                    glColor3ubv((GLubyte*) &set.color_selectoutline);
                    for (i = 0; i < aLeakPortals[counter].w->numpoints; i++)
                    {
                        glVertex3f(aLeakPortals[counter].w->points[i][0],
                                   aLeakPortals[counter].w->points[i][1],
                                   aLeakPortals[counter].w->points[i][2]);
                    }
                    glEnd();
                }
            }
        }

        for (counter = 0; counter < aLeakNode->numwindings; counter++)
        {
            if (counter == leakNum)
                continue;
            glColor3ubv((GLubyte*) &set.color_lock);
            glBegin(GL_LINE_LOOP);
            for (i = 0; i < aLeakNode->w[counter]->numpoints; i++)
            {
                glVertex3fv(aLeakNode->w[counter]->points[i]);
            }
            glEnd();
        }

        plane_t plane;

        PlaneFromWinding(aLeakPortals[0].w,&plane);

        // for portal going into the "entity" space,
        r_flatcolor = leakColor;
        r_drawent = true;
        t_coloroffset = 31L*256L;
        for (counter = 0; counter < aLeakPortals[0].w->numpoints; counter++)
        {
            // draw the normal from each point.
            len = 10.0;
            VectorCopy(aLeakPortals[0].w->points[counter],p1);

            p2[0] = p1[0] + plane.normal[0]*len;
            p2[1] = p1[1] + plane.normal[1]*len;
            p2[2] = p1[2] + plane.normal[2]*len;

            VectorCopy(p1, w->points[0]);
            VectorCopy(p2, w->points[1]);

            glColor3ubv((GLubyte*) &leakColor.rgb);
            glBegin(GL_LINES);
            glVertex3f(w->points[0][0],w->points[0][1],w->points[0][2]);
            glVertex3f(w->points[1][0],w->points[1][1],w->points[1][2]);
            glEnd();

            // draw a line to the "center"
            VectorCopy(aLeakPortals[0].centerPoint,w->points[1]);
            REN_DrawLineWinding(w,0);

            glColor3ubv((GLubyte*) &leakColor.rgb);
            glBegin(GL_LINES);
            glVertex3f(w->points[0][0],w->points[0][1],w->points[0][2]);
            glVertex3f(w->points[1][0],w->points[1][1],w->points[1][2]);
            glEnd();
        }

        if (firstPlayerPortalSet)
        {

            r_drawent = true;
            t_coloroffset = 31L*256L;
            r_flatcolor = exitColor;
            r_entitycolor = exitColor;

            glColor3ubv((GLubyte*) &exitColor.rgb);
            glBegin(GL_LINE_LOOP);
            for (i = 0; i < aLeakPortals[leakNum].w->numpoints; i++)
            {
                glVertex3f(aLeakPortals[leakNum].w->points[i][0],
                           aLeakPortals[leakNum].w->points[i][1],
                           aLeakPortals[leakNum].w->points[i][2]);
            }
            glEnd();
        }

        if (plugPortal >= 0)
        {
            r_drawent = true;
            t_coloroffset = 31L*256L;
            r_flatcolor = plugColor;
            r_entitycolor = plugColor;

            glColor3ubv((GLubyte*) &plugColor.rgb);
            glBegin(GL_LINE_LOOP);

            winding_t *tw = aLeakPortals[plugPortal].w;
            for (i = 0; i < tw->numpoints; i++)
            {
                glVertex3fv(tw->points[i]);
            }
            glEnd();
        }
    }
    else        // set.glBsp
    {

        w = NewWinding(2);
        if(!w) return;

        VectorCopy(LeakPortalOrigin,w->points[0]);
        VectorCopy(aLeakPortals[0].centerPoint,w->points[1]);

        r_drawent = true;
        t_coloroffset = 31L*256L;
        r_flatcolor = exitColor;
        r_entitycolor = exitColor;

        REN_DrawLineWinding(w,0);
        float area;
        r_flatcolor = areaColor;
        r_entitycolor = areaColor;

        int leakNum = -1;

        for (counter = 0; counter < maxPortals; counter++)
        {
            if (counter)
            {
                VectorCopy(aLeakPortals[counter-1].centerPoint,w->points[0]);
                VectorCopy(aLeakPortals[counter].centerPoint,w->points[1]);

                r_drawent = true;
                t_coloroffset = 31L*256L;

                REN_DrawLineWinding(w,2);
            }
            //
            playerCanFit = CheckPortalSize(aLeakPortals[counter].w);
            if (!playerCanFit && !firstPlayerPortalSet)
            {
                firstPlayerPortalSet = true;
                REN_DrawWinding(aLeakPortals[counter].w,3);
                leakNum = counter;

            }
            else
            {
                area = CalcWindingArea(aLeakPortals[counter].w);
                if (area <= 128.0)
                {
                    REN_DrawWinding(aLeakPortals[counter].w,0);
                }
                else
                {
                    REN_DrawWinding(aLeakPortals[counter].w,1);
                }
            }
        }

        for (counter = 0; counter < aLeakNode->numwindings; counter++)
        {
            if (counter == leakNum)
                continue;
            REN_DrawWinding(aLeakNode->w[counter],3);
        }

        plane_t plane;
        PlaneFromWinding(aLeakPortals[0].w,&plane);

        // for portal going into the "entity" space,
        r_flatcolor = leakColor;
        r_drawent = true;
        t_coloroffset = 31L*256L;
        for (counter = 0; counter < aLeakPortals[0].w->numpoints; counter++)
        {
            // draw the normal from each point.
            len = 10.0;
            VectorCopy(aLeakPortals[0].w->points[counter],p1);

            p2[0] = p1[0] + plane.normal[0]*len;
            p2[1] = p1[1] + plane.normal[1]*len;
            p2[2] = p1[2] + plane.normal[2]*len;

            VectorCopy(p1,w->points[0]);
            VectorCopy(p2,w->points[1]);
            REN_DrawLineWinding(w,0);

            // draw a line to the "center"
            VectorCopy(aLeakPortals[0].centerPoint,w->points[1]);
            REN_DrawLineWinding(w,0);

        }

        if (firstPlayerPortalSet)
        {
            r_drawent = true;
            t_coloroffset = 31L*256L;
            r_flatcolor = exitColor;
            r_entitycolor = exitColor;
            REN_DrawWinding(aLeakPortals[leakNum].w,0);
        }

        if (plugPortal >= 0)
        {
            r_drawent = true;
            t_coloroffset = 31L*256L;
            r_flatcolor = plugColor;
            r_entitycolor = plugColor;
            REN_DrawWinding(aLeakPortals[plugPortal].w,0);
        }
    }  // set.glBsp

    delete [] w;
    r_flatcolor = oldfc;
    r_entitycolor = oldec;
}

void TEditWindow::SetCaption(const char* title)
{
    char outstr[128];
    sprintf(outstr,"[3D - %s]",title);
    Window::SetCaption(outstr);
    TCWindow::DoNCPaint();
}

bool TEditWindow::CanClose()
{
    return (0 != KillAllWindows);
}

TEditWindow::TEditWindow(HWND parent, char *title)
    : TCWindow(parent,0)
{
    imagebuffer = NULL;
    t_imagebuffer = NULL;
    zbuffer = NULL;
    t_zbuffer = NULL;
    attr.width = EditSize.cx;    // Set initial width and height...
    attr.height = EditSize.cy;

    attr.dwStyle |= CS_OWNDC;

    hmemdc = 0;
    hmembitmap = NULL;

    maxPortals = 0;
    numPortals = NULL;
    killPortals = NULL;

    SetCaption(title);

    ParseMouse();

    mousedown = 0;
    modal = 0;
    if(!set.glBsp)
    {
        memset(&m_pBmInfoHeader,0,sizeof(BITMAPINFOHEADER));
        m_pBmInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
        m_pBmInfoHeader.biWidth = (int)1;
        m_pBmInfoHeader.biHeight = (int)1;
        m_pBmInfoHeader.biPlanes = 1;
        m_pBmInfoHeader.biCompression = BI_RGB;

        if (set.sinBsp)
        {
            m_pBmInfoHeader.biBitCount =24;
            m_pBmInfoHeader.biClrUsed = 0;
            m_pBmInfoHeader.biSizeImage = 0;

            m_pBmInfo = (BITMAPINFO*)new char[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];
            memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
            memset(m_pBmInfo->bmiColors,0,256*sizeof(RGBQUAD));
        }
        else
        {
            m_pBmInfoHeader.biBitCount = 8;
            m_pBmInfoHeader.biClrUsed = 256;
            m_pBmInfoHeader.biSizeImage = 1;

            m_pBmInfo = (BITMAPINFO*)new char[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];
            memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
            memset(m_pBmInfo->bmiColors,0,256*sizeof(RGBQUAD));
        }
    }
}

TEditWindow::~TEditWindow()
{
    mouseEventType *cur, *nxt;
    for (int i = 0; i < numEventTypes3d; i++)
    {
        for (cur = head3d[i]; cur; cur = nxt)
        {
            nxt = cur->next;
            delete cur;
        }
        head3d[i] = NULL;
    }

    delete numPortals;
    delete killPortals;
    if (set.glBsp)
    {
        wglMakeCurrent(NULL,NULL);
        if (ghRC)
            wglDeleteContext(ghRC);
        ghRC = 0;
    }
    else
    {
        delete [] m_pBmInfo;
        RemoveMemDC();
        RemoveBuffers();
    }
}

//WindowPlacement
HWND TEditWindow::WP_GetHwnd()
{
    return hwnd;
}
const char *TEditWindow::WP_WindowName()
{
    return "3D";
}

void TEditWindow::SetupWindow()
{
    SetIcon(IDI_THREED);

    HDC hdc = GetDC(hwnd);
    HBRUSH bColor = CreateSolidBrush(set.color_background);
    RECT rc;
    GetWindowRect(hwnd,&rc);
    FillRect(hdc,&rc,bColor);
    DeleteObject(bColor);
    ReleaseDC(hwnd,hdc);

    if(!set.glBsp)
    {
        ConstructMemDC();
        ConstructBuffers(set.render_width, set.render_height);
    }

    numPortals = new WButton(hwnd,NUMPORTALS,const_cast<char *> ("Max. # Portals"),0,1,75,14);
    numPortals->Create();
    numPortals->SendMessage(WM_SETFONT,(WPARAM)set.font9,0);
    ::ShowWindow(numPortals->hwnd,SW_HIDE);
    killPortals = new WButton(hwnd,STOPPORTALS,const_cast<char *> ("Stop Portals"),75,1,75,14);
    killPortals->Create();
    killPortals->SendMessage(WM_SETFONT,(WPARAM)set.font9,0);
    ::ShowWindow(killPortals->hwnd,SW_HIDE);

    if (set.glBsp)
    {
        ghDC = GetDC(hwnd);

        if (!bSetupPixelFormat(ghDC))
            return;

        ghRC = wglCreateContext(ghDC);
        wglMakeCurrent(ghDC, ghRC);

        HFONT hfont = GetFont(0,9,5,0);

        SelectObject(ghDC, hfont);

        if ((font_list = glGenLists (256)) == 0 )
        {
            syserror(const_cast<char *> ("set.glBsp failed to create font display lists..."));
        }

        // create the bitmap display lists
        // we're making images of glyphs 0 thru 255
        if (!wglUseFontBitmaps (ghDC, 0, 255, font_list))
        {
            syserror (const_cast<char *> (__FUNCTION__),const_cast<char *> (": wglUseFontBitmaps failed"));
        }

        // indicate start of glyph display lists
        glListBase(font_list);

        initializeGL();
    }
}

void TEditWindow::ToggleConnections()
{
    set.render_connections ^= 1;
    RedrawContents();
}

void TEditWindow::ToggleCenterKnobs()
{
    set.center_knobs_3d ^= 1;
    RedrawContents();
}

void TEditWindow::Paint(HDC hdc)
{
    ValidateRect(hwnd,0);

    if (IsIconic())
        return;

    if (!set.Map_Read)
    {
        RECT rc;
        GetClientRect(hwnd,&rc);
        HBRUSH bColor = CreateSolidBrush(set.color_background);
        FillRect(hdc,&rc,bColor);
        DeleteObject(bColor);
        return;
    }

    if (set.glBsp)
    {
        drawScene();
    }
    else
    {
        m_pBmInfoHeader.biWidth = (int)set.render_width;
        m_pBmInfoHeader.biHeight = (int)-set.render_height;
        memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
        if (set.sinBsp)
        {
            m_pBmInfoHeader.biSizeImage = 0;
            memset(m_pBmInfo->bmiColors,0,256*sizeof(RGBQUAD));
        }
        else
        {
            m_pBmInfoHeader.biSizeImage = set.render_width*set.render_height;
            memcpy(m_pBmInfo->bmiColors,set.cTable,256*sizeof(RGBQUAD));
        }
        if (imagebuffer && m_pBmInfo)
        {
            SetStretchBltMode(hdc, COLORONCOLOR);

            RECT dest, source;

            CalcBltRects(EditSize.cx, EditSize.cy, (int)set.render_width,(int)set.render_height, &dest, &source);

            StretchDIBits(
                hdc,
                dest.left, dest.top, dest.right, dest.bottom,
                source.left, source.top, source.right, source.bottom,
                imagebuffer,
                m_pBmInfo,
                DIB_RGB_COLORS,
                SRCCOPY);

        }
        map *m = map_i[set.curmap];

        m->clipper_i->cameraDrawSelf(hdc);

        m->DrawOrientation(hdc);
    }
}

void TEditWindow::SyncAutoRender()
{
    if(!set.render_auto)
        sysfatal(const_cast<char *> ("not auto render"));

    int tx = (EditSize.cx+4) & ~3;
    int ty = (EditSize.cy+4) & ~3;

    if(!set.auto_init || set.render_width != tx || set.render_height != ty)
    {
        set.render_width = tx;
        set.render_height = ty;

        //if screen is bigger than buffer, stretch the image (rather than crash)
        if(set.render_width >= AUTO_MAX_W) set.render_width = AUTO_MAX_W;
        if(set.render_height >= AUTO_MAX_H) set.render_height = AUTO_MAX_H;

        RemoveBuffers();
        ConstructBuffers(AUTO_MAX_W, AUTO_MAX_H);	//set max size of buffer
    }
}

void TEditWindow::ClearContents()
{
    if (IsIconic())
        return;
    if (!hmemdc || !hmembitmap)
        return;

    HDC hdc = GetDC(hwnd);
    HBRUSH bColor = CreateSolidBrush(set.color_background);
    FillRect(hmemdc, &EditClip, bColor);
    BitBlt(hdc,EditClip.left,EditClip.top,EditClip.right-EditClip.left,EditClip.bottom-EditClip.top, hmemdc, 0, 0, SRCCOPY);
    DeleteObject(bColor);
    ReleaseDC(hwnd,hdc);
}

void TEditWindow::RemoveMemDC()
{
    if (hmemdc)
        DeleteDC(hmemdc);
    if (hmembitmap)
        DeleteObject(hmembitmap);
    hmemdc = 0;
    hmembitmap = 0;
}

void TEditWindow::ConstructMemDC()
{
    HDC hdc = GetDC(hwnd);
    // allocate the memory dc and the bitmap
    hmemdc = ::CreateCompatibleDC(hdc);
    hmembitmap = CreateCompatibleBitmap(hdc,EditSize.cx, EditSize.cy);

    SelectObject(hmemdc,hmembitmap);
    SetStretchBltMode(hmemdc, COLORONCOLOR);

    RECT rc;
    GetClientRect(hwnd,&rc);
    HBRUSH bColor = CreateSolidBrush(set.color_background);
    FillRect(hmemdc,&rc,bColor);
    DeleteObject(bColor);
    ReleaseDC(hwnd,hdc);
}

void TEditWindow::RedrawBrush(SetBrush *b)
{
    if (!b)
        return;
    if (IsIconic())
        return;
    if (!set.Map_Read)
        return;
    if (!hmemdc || !hmembitmap)
        return;
    map *m = map_i[set.curmap];
    if (!m)
        return;
    if (set.glBsp)
    {
        drawScene();
    }
    else
    {
        VectorCopy(m->eye[m->cureye],r_origin);
        vec3_t zero;
        VectorCopy(vec3_origin,zero);
        float Q[4][4];
        float Temp[4][4];

        findQ(Q,Temp,zero,
              m->angles[m->cureye].roll,
              m->angles[m->cureye].pitch,
              m->angles[m->cureye].yaw);

        // vpn
        r_matrix[0][0] = Q[0][0];
        r_matrix[0][1] = Q[0][1];
        r_matrix[0][2] = Q[0][2];

        r_matrix[1][0] = Q[1][0];
        r_matrix[1][1] = Q[1][1];
        r_matrix[1][2] = Q[1][2];

        r_matrix[2][0] = Q[2][0];
        r_matrix[2][1] = Q[2][1];
        r_matrix[2][2] = Q[2][2];

        r_width = set.render_width; // EditSize.cx;
        r_height = set.render_height; // EditSize.cy;

        r_picbuffer = imagebuffer;
        r_zbuffer = zbuffer;

        r_noz = true;
        REN_BeginCamera ();
        //	REN_ClearBuffers ();

        b->CameraRenderSelf();
        r_noz = false;

        ShowLeaks();
        ShowPortals();

        //
        // display the output
        //
        m_pBmInfoHeader.biWidth = set.render_width;
        m_pBmInfoHeader.biHeight = -set.render_height;
        memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
        if (set.sinBsp)
        {
            m_pBmInfoHeader.biSizeImage = 0;
            memset(m_pBmInfo->bmiColors,0,256*sizeof(RGBQUAD));
        }
        else
        {
            m_pBmInfoHeader.biSizeImage = set.render_width*set.render_height;
            memcpy(m_pBmInfo->bmiColors,set.cTable,256*sizeof(RGBQUAD));
        }
        if(imagebuffer && m_pBmInfo)
        {

            RECT dest, source;

            CalcBltRects(EditSize.cx, EditSize.cy, set.render_width, set.render_height, &dest, &source);

            if(set.render_auto)
            {
                StretchDIBits(
                    hmemdc,
                    //dest.left, dest.top, source.right, source.bottom,
                    source.left, source.top, source.right, source.bottom,
                    source.left, source.top, source.right, source.bottom,
                    imagebuffer,
                    m_pBmInfo,
                    DIB_RGB_COLORS,
                    SRCCOPY);

            }
            else
            {
                StretchDIBits(
                    hmemdc,
                    dest.left, dest.top, dest.right, dest.bottom,
                    source.left, source.top, source.right, source.bottom,
                    imagebuffer,
                    m_pBmInfo,
                    DIB_RGB_COLORS,
                    SRCCOPY);
            }
        }
        m->clipper_i->cameraDrawSelf(hmemdc);

        GdiFlush();

        HDC hdc = GetDC(hwnd);

        SetStretchBltMode(hdc, COLORONCOLOR);

        m->DrawOrientation(hmemdc);

        BitBlt(hdc,EditClip.left,EditClip.top,EditClip.right-EditClip.left,EditClip.bottom-EditClip.top,hmemdc,0,0,SRCCOPY);

        ReleaseDC(hwnd,hdc);
    }
}

// With t_zbuffer and t_imagebuffer usage...
void TEditWindow::RedrawBrushZ()
{
    if (IsIconic())
        return;
    if (!set.Map_Read)
        return;
    if (!hmemdc || !hmembitmap)
        return;
    map *m = map_i[set.curmap];
    if(!m)
        return;

    GdiFlush();

    if (set.glBsp)
    {
        drawScene();
    }
    else
    {
        VectorCopy(m->eye[m->cureye],r_origin);
        vec3_t zero;
        VectorCopy(vec3_origin,zero);
        float Q[4][4];
        float Temp[4][4];

        findQ(Q,Temp,zero,
              m->angles[m->cureye].roll,
              m->angles[m->cureye].pitch,
              m->angles[m->cureye].yaw
             );

        // vpn
        r_matrix[0][0] = Q[0][0];
        r_matrix[0][1] = Q[0][1];
        r_matrix[0][2] = Q[0][2];

        r_matrix[1][0] = Q[1][0];
        r_matrix[1][1] = Q[1][1];
        r_matrix[1][2] = Q[1][2];

        r_matrix[2][0] = Q[2][0];
        r_matrix[2][1] = Q[2][1];
        r_matrix[2][2] = Q[2][2];

        r_width = set.render_width;
        r_height = set.render_height;

        r_picbuffer = imagebuffer;
        r_zbuffer = zbuffer;

        REN_BeginCamera ();
        REN_CopyBuffers (t_zbuffer,zbuffer,t_imagebuffer,imagebuffer);

        m->makeSelectedPerform(SEL_CAMERARENDERSELF);

        ShowLeaks();
        ShowPortals();

        //
        // display the output
        //
        m_pBmInfoHeader.biWidth = (int)set.render_width;
        m_pBmInfoHeader.biHeight = (int)-set.render_height;
        memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
        if (set.sinBsp)
        {
            m_pBmInfoHeader.biSizeImage = 0;
            memset(m_pBmInfo->bmiColors,0,256*sizeof(RGBQUAD));
        }
        else
        {
            m_pBmInfoHeader.biSizeImage = set.render_width*set.render_height;
            memcpy(m_pBmInfo->bmiColors,set.cTable,256*sizeof(RGBQUAD));
        }
        if (imagebuffer && m_pBmInfo)
        {

            RECT dest, source;
            CalcBltRects(EditSize.cx, EditSize.cy, (int)set.render_width,(int)set.render_height, &dest, &source);

            if(set.render_auto)
            {
                StretchDIBits(
                    hmemdc,
                    //	dest.left, dest.top, source.right, source.bottom,
                    source.left, source.top, source.right, source.bottom,
                    source.left, source.top, source.right, source.bottom,
                    imagebuffer,
                    m_pBmInfo,
                    DIB_RGB_COLORS,
                    SRCCOPY);

            }
            else
            {
                StretchDIBits(
                    hmemdc,
                    dest.left, dest.top, dest.right, dest.bottom,
                    source.left, source.top, source.right, source.bottom,
                    imagebuffer,
                    m_pBmInfo,
                    DIB_RGB_COLORS,
                    SRCCOPY);
            }
        }
        m->clipper_i->cameraDrawSelf(hmemdc);

        GdiFlush();

        HDC hdc = GetDC(hwnd);

        SetStretchBltMode(hdc, COLORONCOLOR);

        m->DrawOrientation(hmemdc);

        BitBlt(hdc,EditClip.left,EditClip.top,EditClip.right-EditClip.left,EditClip.bottom-EditClip.top,hmemdc,0,0,SRCCOPY);

        ReleaseDC(hwnd,hdc);
    }
}

void TEditWindow::RedrawContents()
{
    RedrawContentsMap(map_i[set.curmap]);
}
void TEditWindow::RedrawContentsMap(map *m)
{
    if (IsIconic())
        return;
    if (!set.Map_Read)
        return;
    if (!hmemdc || !hmembitmap)
        return;
    if(!m)
        return;

    REN_curBrush = m->selectedBrush();
    SetCaption(m->filename);

    if (set.glBsp)
    {
        drawScene();
    }
    else
    {
        if(set.render_auto)
            SyncAutoRender();

        //
        // draw it
        //
        VectorCopy(m->eye[m->cureye],r_origin);
        vec3_t zero;
        VectorCopy(vec3_origin,zero);
        float Q[4][4];
        float Temp[4][4];

        findQ(Q,Temp,zero,
              m->angles[m->cureye].roll,
              m->angles[m->cureye].pitch,
              m->angles[m->cureye].yaw
             );

        // vpn
        //x
        r_matrix[0][0] = Q[0][0];
        r_matrix[0][1] = Q[0][1];
        r_matrix[0][2] = Q[0][2];
        //y
        r_matrix[1][0] = Q[1][0];
        r_matrix[1][1] = Q[1][1];
        r_matrix[1][2] = Q[1][2];
        //z
        r_matrix[2][0] = Q[2][0];
        r_matrix[2][1] = Q[2][1];
        r_matrix[2][2] = Q[2][2];

        r_width = set.render_width; // EditSize.cx;
        r_height = set.render_height; // EditSize.cy;

        r_picbuffer = t_imagebuffer;
        r_zbuffer = t_zbuffer;

        r_drawflat = (set.drawmode == Drawmode::flat);
        r_drawwire = (set.drawmode == Drawmode::wire);
        //
        //r_drawwire = true;
        REN_BeginCamera ();
        REN_ClearBuffers ();

        r_picbuffer = imagebuffer;
        r_zbuffer = zbuffer;

        REN_ClearBuffers ();

        if(set.show_3d_grid)
            REN_Draw3DGrid();

        if(set.show_world_axes)
            DrawAxes();

        // clear the t_buffers xxx
        //
        // render the setbrushes
        //
        brushCount = 0;

        if (timeRender)
        {
            startTime = GetTickCount();
        }
        Entity   *e;
        SetBrush *b;

        // First draw the unselected brushes...
        for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
        {
            LoopProblem("3dredrawE");
            for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
            {
                LoopProblem("3dredrawB");
                if (!b->IsDrawable())
                    continue;
                if (b->IsSelected())
                    continue;

                b->CameraRenderSelf();
                brushCount++;
            }
        }


        // now draw the selected wire frames...

        StoreBuffers();

        // store the unselected zbuffer etc.
        // now draw the selected...
        for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
        {
            LoopProblem("3drede2");
            for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
            {
                LoopProblem("3dredrb2");
                if (!b->IsDrawable())
                    continue;
                if (!b->IsSelected())
                    continue;

                b->CameraRenderSelf();
                brushCount++;
            }
        }

        if (set.ghosts_3d)
        {
            // switch zbuffers..
            bool	olddraw;
            bool  oldwire;
            clipDraw = true;
            olddraw = r_drawflat;
            oldwire = r_drawwire;
            r_drawflat = false;
            r_drawwire = true;
            r_picbuffer = t_imagebuffer;
            r_zbuffer = t_zbuffer;

            for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
            {
                LoopProblem("3drede3");
                for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
                {
                    LoopProblem("3dredb3");
                    if (!b->IsDrawable())
                        continue;
                    if (!b->IsSelected())
                        continue;

                    b->CameraRenderSelf();
                    brushCount++;
                }
            }
            r_drawflat = olddraw;
            r_drawwire = oldwire;
            clipDraw = false;
            r_picbuffer = imagebuffer;
            r_zbuffer = zbuffer;
        } // set.ghosts_3d


        if (timeRender)
            endTime = GetTickCount();

        //
        // display the output
        //
        if (!redraw_interrupted)
        {
            ShowLeaks();
            ShowPortals();
        }

        if(!set.glBsp)
        {
            m_pBmInfoHeader.biWidth = (int)set.render_width;
            m_pBmInfoHeader.biHeight = (int)-set.render_height;
            memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
            if (set.sinBsp)
            {
                m_pBmInfoHeader.biSizeImage = 0;
                memset(m_pBmInfo->bmiColors,0,256*sizeof(RGBQUAD));
            }
            else
            {
                m_pBmInfoHeader.biSizeImage = set.render_width*set.render_height;
                memcpy(m_pBmInfo->bmiColors,set.cTable,256*sizeof(RGBQUAD));
            }

            if (imagebuffer && m_pBmInfo)
            {
                RECT dest, source;
                CalcBltRects(EditSize.cx, EditSize.cy, (int)set.render_width,(int)set.render_height, &dest, &source);

                if(set.render_auto)
                {
                    StretchDIBits(
                        hmemdc,
                        //	dest.left, dest.top, source.right, source.bottom,
                        source.left, source.top, source.right, source.bottom,
                        source.left, source.top, source.right, source.bottom,
                        imagebuffer,
                        m_pBmInfo,
                        DIB_RGB_COLORS,
                        SRCCOPY);

                }
                else
                {
                    StretchDIBits(
                        hmemdc,
                        dest.left, dest.top, dest.right, dest.bottom,
                        source.left, source.top, source.right, source.bottom,
                        imagebuffer,
                        m_pBmInfo,
                        DIB_RGB_COLORS,
                        SRCCOPY);
                }
            }

            m->clipper_i->cameraDrawSelf(hmemdc);

            GdiFlush();

            HDC hdc = GetDC(hwnd);

            SetStretchBltMode(hdc, COLORONCOLOR);

            m->DrawOrientation(hmemdc);

            if (redraw_interrupted)
            {
                HGDIOBJ old = SelectObject(hmemdc,set.font10);
                SetTextColor(hmemdc,set.color_foreground);
                SetBkColor(hmemdc,set.color_background);
                SetBkMode(hmemdc,TRANSPARENT);

                char intStr[32];
                int l = sprintf(intStr,"Paused at [%i]",brushCount);
                TextOut(hmemdc,0,0,intStr,l);
                SelectObject(hmemdc,old);
            }

            BitBlt(hdc,EditClip.left,EditClip.top,EditClip.right-EditClip.left,EditClip.bottom-EditClip.top,hmemdc,0,0,SRCCOPY);

            ReleaseDC(hwnd,hdc);

        }
    }
}

void TEditWindow::UpdateSelected()
{
    UpdateSelectedMap(map_i[set.curmap]);
}

//FUNCTION FOR GL !!!!!
void TEditWindow::UpdateSelectedMap(map *m)
{
    if (IsIconic())
        return;
    if (!set.Map_Read)
        return;
    if (!hmemdc || !hmembitmap)
        return;
    if (!m)
        return;

    REN_curBrush = m->selectedBrush();
    SetCaption(m->filename);

    findQ(set.Q,set.R,m->eye[m->cureye],
          m->angles[m->cureye].roll,
          m->angles[m->cureye].pitch,
          m->angles[m->cureye].yaw);

    if (set.glBsp)
    {
        if (glBspRotate)
        {
            map *holdMap;
            // draw the original map...
            holdMap = map_i[set.curmap];
            map_i[set.curmap] = m;
            // now draw the "rotate" map if any...
            glMap = holdMap;
            drawScene();
            map_i[set.curmap] = holdMap;
            glMap = NULL;
        }
        else
        {
            drawScene();
        }
    }
    else
    {
        //
        // draw it
        //
        VectorCopy(m->eye[m->cureye],r_origin);
        vec3_t zero;
        VectorCopy(vec3_origin,zero);
        float Q[4][4];
        float Temp[4][4];

        findQ(Q,Temp,zero,
              m->angles[m->cureye].roll,
              m->angles[m->cureye].pitch,
              m->angles[m->cureye].yaw
             );

        // vpn
        r_matrix[0][0] = Q[0][0];
        r_matrix[0][1] = Q[0][1];
        r_matrix[0][2] = Q[0][2];

        r_matrix[1][0] = Q[1][0];
        r_matrix[1][1] = Q[1][1];
        r_matrix[1][2] = Q[1][2];

        r_matrix[2][0] = Q[2][0];
        r_matrix[2][1] = Q[2][1];
        r_matrix[2][2] = Q[2][2];

        r_width = set.render_width; // EditSize.cx;
        r_height = set.render_height; // EditSize.cy;

        r_picbuffer = imagebuffer;
        r_zbuffer = zbuffer;

        r_drawflat = (set.drawmode == Drawmode::flat);
        r_drawwire = (set.drawmode == Drawmode::wire);

        REN_BeginCamera ();
        RestoreBuffers();
        // render the selected brushes

        brushCount = 0;

        Entity   *e;
        SetBrush *b;
        for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
        {
            LoopProblem("3dusme");
            for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
            {
                LoopProblem("3dusmb");
                if (!b->IsDrawable())
                    continue;

                if (!b->IsSelected())
                    continue;

                b->CameraRenderSelf();
                brushCount++;
            }
        }

        //
        // display the output
        //
        m_pBmInfoHeader.biWidth = (int)set.render_width;
        m_pBmInfoHeader.biHeight = (int)-set.render_height;
        memcpy(m_pBmInfo,&m_pBmInfoHeader,sizeof(BITMAPINFOHEADER));
        if (set.sinBsp)
        {
            m_pBmInfoHeader.biSizeImage = 0;
            memset(m_pBmInfo->bmiColors,0,256*sizeof(RGBQUAD));
        }
        else
        {
            m_pBmInfoHeader.biSizeImage = set.render_width*set.render_height;
            memcpy(m_pBmInfo->bmiColors,set.cTable,256*sizeof(RGBQUAD));
        }

        if (imagebuffer && m_pBmInfo)
        {
            RECT dest, source;
            CalcBltRects(EditSize.cx, EditSize.cy, (int)set.render_width,(int)set.render_height, &dest, &source);

            if(set.render_auto)
            {
                StretchDIBits(
                    hmemdc,
                    //	dest.left, dest.top, source.right, source.bottom,
                    source.left, source.top, source.right, source.bottom,
                    source.left, source.top, source.right, source.bottom,
                    imagebuffer,
                    m_pBmInfo,
                    DIB_RGB_COLORS,
                    SRCCOPY);

            }
            else
            {
                StretchDIBits(
                    hmemdc,
                    dest.left, dest.top, dest.right, dest.bottom,
                    source.left, source.top, source.right, source.bottom,
                    imagebuffer,
                    m_pBmInfo,
                    DIB_RGB_COLORS,
                    SRCCOPY);
            }

        }

        m->clipper_i->cameraDrawSelf(hmemdc);

        GdiFlush();

        HDC hdc = GetDC(hwnd);

        SetStretchBltMode(hdc, COLORONCOLOR);

        m->DrawOrientation(hmemdc);

        BitBlt(hdc,EditClip.left,EditClip.top,EditClip.right-EditClip.left,EditClip.bottom-EditClip.top,hmemdc,0,0,SRCCOPY);

        ReleaseDC(hwnd,hdc);
    }
}

void TEditWindow::SetYaw(POINT *point)
{
    POINT cent;

    cent.x = EditSize.cx - 25;
    cent.y = 25;

    int dx = point->x - cent.x;
    int dy = point->y - cent.y;

    float ang = angle((float)dy,(float)dx);

    ang = (float)RAD2DEG(ang);

    while (ang < 0.0f)
        ang += 360.0f;
    while (ang >360.0f)
        ang -= 360.0f;

    float degreesPerStep = (360.0f/(float)set.steps_per_turn);

    ang = degreesPerStep * (int)((ang + degreesPerStep/2.0) / degreesPerStep);

    float radang = DEG2RAD(ang);

    map *m = map_i[set.curmap];
    m->angles[m->cureye].yaw = radang;
    set.redrawedit = 1;
    set.redrawxy = 1;

    char outstr[80];
    while (ang < 0.0)
        ang += 360.0;
    sprintf(outstr,"Yaw set to [%i]...",(int)ang);
    Show_Frame(outstr,true);
}

void TEditWindow::SetPitch(POINT *point)
{
    POINT cent;

    cent.x = EditSize.cx - 25;
    cent.y = 25;

    int dx = point->x - cent.x;
    int dy = point->y - cent.y;

    float ang = angle((float)dy,(float)dx);

    ang = (180.0f*ang)/M_PI;
    while (ang < 0.0f)
        ang += 360.0f;

    float degreesPerStep = (360.0f/(float)set.steps_per_turn);

    ang = degreesPerStep * (int)((ang + degreesPerStep/2.0) / degreesPerStep);

    float radang = DEG2RAD(ang);

    map *m = map_i[set.curmap];
    m->angles[m->cureye].pitch = radang;
    set.redrawedit = 1;
    set.redrawxy = 1;

    char outstr[80];
    sprintf(outstr,"Pitch set to [%i]...",(int)ang);
    Show_Frame(outstr,true);
    return;
}

void TEditWindow::SetFov(POINT *point)
{
    int fov = 90;

    //software render max angle is 180.
    if(!set.glBsp)
    {
        if(point->y <= 15)					// over top of control
            fov = 2;
        else if(point->y >= fovsz.cy+5)		//under control
            fov = 180;
        else
        {
            int p = point->y - 15;
            fov = p * 180 / (fovsz.cy-10);
        }
        fov = min(180, fov);
        fov = max(16, fov);
        set.fov = (float) fov;
        //opengl max angle is 120
    }
    else
    {
        if(point->y <= 15)					// over top of control
            fov = 2;
        else if(point->y >= fovsz.cy+5)		//under control
            fov = 120;
        else
        {
            int p = point->y - 15;
            fov = p * 120 / (fovsz.cy-10);
        }
        fov = min(120, fov);
        fov = max(16, fov);

        set.fov = (float) fov;
        resize();
    }
    set.redrawedit = 1;

    char outstr[80];
    sprintf(outstr,"FOV set to [%i]...",fov);
    Show_Frame(outstr,true);
    return;
}

void TEditWindow::EvMouseMove(UINT modKeys,POINT *curPos)
{
    int	 i,j;
    bool snapEdge;

    if (DoMouseMove())
        return;
    if (!set.Map_Read)
        return;

    map *m = map_i[set.curmap];
    if(!m)
        return;

    if (!mousedown)
    {
        int m_x = (int)( ((float)curPos->x/(float)EditSize.cx)*(float)set.render_width);
        int m_y = (int)( ((float)curPos->y/(float)EditSize.cy)*(float)set.render_height);

        bool onEdge = false;

        if (set.center_knobs_3d)
        {
            if (set.glBsp)
                onEdge = glPointOnEdgeKnob(curPos->x,curPos->y);
            else
                onEdge = mouseOnEdgeKnob(m_x,m_y);
        }

        if (onEdge)
        {
            status->SetPartBmp(STATUS_MODE,9);

        }
        else
        {
            status->SetPartBmp(STATUS_MODE,8);
        }
        status->Redraw();
        return;
    }
    else
    {
        if( mousedown == SELECT_CALLBACK ||
                mousedown == LIFT_TEXTURE ||
                mousedown == LIFT_APPLY ||
                mousedown == TEXTUREBRUSH_CALLBACK ||
                mousedown == TEXTUREFACE_CALLBACK ||
                mousedown == SETCLIPPER ||
                mousedown == SETCLIPPERINV
          )
            return;
    }

    if(mousedown == ANGLE_CONTROL)
    {
        if(set.angle_control == PITCH_CONTROL)
            SetPitch(curPos);
        else if(set.angle_control == YAW_CONTROL)
            SetYaw(curPos);
        else
            SetFov(curPos);
        return;
    }

    if (!REN_bestBrush && (mousedown != NAVIGATE && mousedown != MLOOK && mousedown != NAVIGATE_STRAFE))
    {
        mousedown = 0;
        return;
    }

    newpt.x = curPos->x;
    newpt.y = curPos->y;

    if (set.show_texturedrag && (mousedown == ROTATE_TEXTURE))
    {
        relative.x = (newpt.x - startpt.x);
        relative.y = (newpt.y - startpt.y);
    }
    else if(mousedown == NAVIGATE || mousedown == NAVIGATE_STRAFE)
    {
        relative.x = (newpt.x - startpt.x)/16;
        relative.y = (newpt.y - startpt.y)/4;
    }
    else if(mousedown == NAVIGATE_STRAFE)
    {
        relative.x = (newpt.x - startpt.x)/4;
        relative.y = (newpt.y - startpt.y)/4;
    }
    else if(mousedown == MLOOK)
    {
        relative.x = (newpt.x - startpt.x);///16;
        relative.y = (newpt.y - startpt.y);///4;
    }
    else if (mousedown == EDGEDRAG)
    {
        snapEdge = false;
        for (i = 0; i < 3; i++)
        {
            if (fabs(1.0f - fabs(edgeNormal[i])) < 0.0001)
                snapEdge = true;
        }

        if (!set.edge_use_grid)
            snapEdge = false;

        if (set.gridsize && set.edge_use_grid && snapEdge)
        {
            relative.x = (newpt.x - startpt.x)/set.gridsize;
            relative.y = (newpt.y - startpt.y)/set.gridsize;
        }
        else
        {
            relative.x = (newpt.x - startpt.x);
            relative.y = (newpt.y - startpt.y);
        }
    }
    else
    {
        relative.x = (newpt.x - startpt.x)/4;
        relative.y = (newpt.y - startpt.y)/4;
    }

    delta.x = relative.x - oldrelative.x;
    delta.y = relative.y - oldrelative.y;

    if (!delta.x && !delta.y)	//no change?
        return;

    oldrelative.x = relative.x;
    oldrelative.y = relative.y;

    r_picbuffer = imagebuffer;
    r_zbuffer = zbuffer;

    texturedef_t *td = 0;
    if (mousedown != NAVIGATE && mousedown != MLOOK && mousedown != NAVIGATE_STRAFE)
    {
        td = REN_bestBrush->texturedefForFace(REN_bestFace);
    }

    char outstr[80];
    vec3_t N;
    vec3_t xa,ya;
    int texU, texV;
    float ang, temp;
    plane_t pl;

    *outstr = 0;

    // Throw the cur position stuff here, too...
    switch (mousedown)
    {
    case NAVIGATE:
        // f and b
        m->eye[m->cureye][0] += -delta.y*set.stepsize*cos(m->angles[m->cureye].yaw-M_PI/2.0f);
        m->eye[m->cureye][1] += -delta.y*set.stepsize*sin(m->angles[m->cureye].yaw-M_PI/2.0f);
        m->MoveEyePosition(m->eye[m->cureye],set.lock_cameras);
        // r and l
        ang = m->angles[m->cureye].yaw;
        if (delta.x > 0)
        {
            ang -= set.angstep;
            if (ang < 0.0f)
            {
                ang += 2*M_PI;
            }
        }
        else if (delta.x < 0)
        {
            ang += set.angstep;
            if (ang > 2*M_PI)
            {
                ang -= 2*M_PI;
            }
        }
        m->angles[m->cureye].yaw = ang;

        break;
    case NAVIGATE_STRAFE:
        // f and b
        m->eye[m->cureye][0] += -delta.y * set.stepsize * cos(m->angles[m->cureye].yaw-M_PI/2.0f);
        m->eye[m->cureye][1] += -delta.y * set.stepsize * sin(m->angles[m->cureye].yaw-M_PI/2.0f);

        //strafe l and r
        m->eye[m->cureye][0] += -delta.x * set.stepsize * cos(m->angles[m->cureye].yaw);//-M_PI/2.0);
        m->eye[m->cureye][1] += -delta.x * set.stepsize * sin(m->angles[m->cureye].yaw);//-M_PI/2.0);

        m->MoveEyePosition(m->eye[m->cureye],set.lock_cameras);
        break;

    case MLOOK:
    {
        t_Angles *angles = &m->angles[m->cureye];

        // u and d
        ang = angles->pitch;
        if(delta.y < 0)
        {
            ang += 2*M_PI/set.mlook_vert_step*abs(delta.y); //		72;//set.angstep / 4;
            if (ang > 2*M_PI)
                ang -= 2*M_PI;
        }
        else if (delta.y > 0)
        {
            ang -= 2*M_PI/set.mlook_vert_step*delta.y;//				//72;//set.angstep / 4;
            if (ang < 0.0f)
                ang += 2*M_PI;
        }
        angles->pitch = ang;

        // r and l
        ang = angles->yaw;
        if (delta.x < 0)
        {
            ang += 2*M_PI/set.mlook_horiz_step*abs(delta.x);// set.angstep / 2;
            if (ang < 0.0f)
            {
                ang += 2*M_PI;
            }
        }
        else if (delta.x > 0)
        {
            ang -= 2*M_PI/set.mlook_horiz_step*delta.x;// set.angstep / 2;
            if (ang > 2*M_PI)
            {
                ang -= 2*M_PI;
            }
        }
        angles->yaw = ang;

        break;
    }
    case SHIFT_TEXTURE:

        if (REN_bestBrush && REN_bestBrush->parent->modifiable && REN_bestFace && td)
        {
            float dx,dy;

            //some (appropriate) hacks to have better luck at shifting textures in the expected direction
            //TODO - bug here.does not correctly account for some 45' angles.
            //swapping < with <= fixes the bad angle, but breaks opposite angle and top. ??
            if(REN_bestFace->plane.normal[0] < REN_bestFace->plane.normal[1])
            {
                dx = (float)delta.x * set.tex_shiftstep;
            }
            else
            {
                dx = -(float)delta.x * set.tex_shiftstep;
            }
            dy = -(float)delta.y * set.tex_shiftstep;

            //get texture orientation
            vec3_t tex[3];
            TextureAxisFromPlaneEx(&REN_bestFace->plane, tex);

            //combine texture rot + camera yaw to determine shift angle
            float a = DEG2RAD(td->rotate);

            //special handling for floor/ceiling texture axis
            if(fabs(tex[0][2]) > 0.5f)
            {
                if(tex[0][2] < 0.0f)
                    dy = -dy;			//flip dy for ceiling

                a = a - m->angles[m->cureye].yaw-M_PI;
            }

            //rotate our deltas
            float tx = cos(a) * dx - sin(a) * dy;
            float ty = sin(a) * dx + cos(a) * dy;

            //adjust for negative SX/SY, AFTER applying rotations.
            tx /= td->scale[0];
            ty /= td->scale[1];

            td->shift[0] += tx;
            td->shift[1] += ty;

            REN_bestFace->texture = *td;

            delete REN_bestFace->qtexture;

            REN_bestFace->qtexture = NULL;
            REN_bestBrush->calcWindings();	// in case texture coords changed
            sprintf(outstr,"(S %d, T %d)     ",(int)td->shift[0],(int)td->shift[1]);
        }
        else
        {
            sprintf(outstr,"(S %d, T %d)     ",(int)(td->shift[0]+relative.x),(int)(td->shift[1]-relative.y));
        }
        break;
    case STRETCH_TEXTURE:
        if (REN_bestBrush && REN_bestBrush->parent->modifiable && REN_bestFace && td)
        {

            td->scale[0] += (float)delta.x*set.tex_scalestep;
            td->scale[1] -= (float)delta.y*set.tex_scalestep;

            REN_bestFace->texture = *td;

            delete REN_bestFace->qtexture;

            REN_bestFace->qtexture = NULL;
            REN_bestBrush->calcWindings();	// in case texture coords changed
            sprintf(outstr,"(SX %3.4f, SY %3.4f)     ",td->scale[0],td->scale[1]);
        }
        else
        {
            sprintf(outstr,"(SX %3.4f, SY %3.4f)     ",td->scale[0]+(float)relative.x*set.tex_scalestep,
                    td->scale[1]-(float)relative.y*set.tex_scalestep);
        }
        break;
    case ROTATE_TEXTURE:
        if (REN_bestBrush && REN_bestFace->qtexture && REN_bestBrush->parent->modifiable && REN_bestFace && td)
        {
            //
            // new idea: take x,y point, rotate to a
            //
            float a = DEG2RAD(startA + (float)relative.y*set.tex_rotatestep);

            //get texture orientation so we can use correct U,V from brush->bctr[]
            vec3_t tex[3];
            TextureAxisFromPlaneEx(&REN_bestFace->plane, tex);

            //determine 2d center point
            int va=0, ua=1;

            if(fabs(tex[0][0]) > 0.1)
            {
                va=1;
                ua=2;
            }
            else if(fabs(tex[0][1]) > 0.1)
            {
                va=0;
                ua=2;
            }
            vec2_t ctr;
            ctr[0] = -REN_bestBrush->bctr[va];
            ctr[1] = REN_bestBrush->bctr[ua];

            ctr[0] = ctr[0] - ((int)ctr[0] % REN_bestFace->qtexture->width);
            ctr[1] = ctr[1] - ((int)ctr[1] % REN_bestFace->qtexture->height);

            td->shift[0] = (ctr[0]*cos(a) - ctr[1]*sin(a)) / td->scale[0];
            td->shift[1] = (ctr[1]*cos(a) + ctr[0]*sin(a)) / td->scale[1];

            FixOverflow(td->shift[0], (float)REN_bestFace->qtexture->width);
            FixOverflow(td->shift[1], (float)REN_bestFace->qtexture->height);

            //Caption("S:%d T:%d,, start S:%d T:%d",(int)td->shift[0],(int)td->shift[1], (int)start_s,(int)start_t);
            td->rotate = startA + (float)relative.y*set.tex_rotatestep;

            FixOverflow(td->rotate, 360.0f);

            REN_bestFace->texture = *td;
            delete REN_bestFace->qtexture;
            REN_bestFace->qtexture = NULL;
            REN_bestBrush->calcWindings();	// in case texture coords changed
            sprintf(outstr,"(R %d)     ",(int)td->rotate);
        }
        else
        {
            sprintf(outstr,"(R %d)     ",(int)(td->rotate+RAD2DEG(angle((float)relative.x,(float)-relative.y))));
        }
        break;
    case CLICKFACEDRAG:
        if (REN_bestBrush && REN_bestFace)
        {
            if (abs(delta.x) > abs(delta.y))
                texU = delta.x;
            else
                texU = delta.y;

            VectorCopy(REN_bestFace->plane.normal, N);

            for (i = 0; i < 3; i++)
            {
                if (fabs(fabs(N[i]) - 1.0) < 0.01)
                {
                    break;
                }
            }

            /* removed some stuff not needed here */
            for (i=0 ; i<numcontrolpoints ; i++)
            {
                for (j = 0; j < 3; j++)
                {
                    controlpoints[i][j] += (float)texU * N[j];
                }
            }
            strcpy(outstr, "Dragging face");
            REN_bestBrush->calcWindings();
        }
        break;
    case EDGEDRAG:     // todo: what is wrong with this?
        texU = delta.x;
        texV = delta.y;
        memset(&pl,0,sizeof(plane_t));

        VectorCopy(edgeNormal,pl.normal);
        pl.dist = 0.0;

        TextureAxisFromPlane(&pl,xa, ya);

        if (multicontrolpoints <= 0)
            break;

        snapEdge = false;

        for (i = 0; i < 3; i++)
        {
            if (fabs(1.0f - fabs(edgeNormal[i])) < 0.0001)
                snapEdge = true;
        }

        if (!set.edge_use_grid)
            snapEdge = false;

        // do them all
        for (i=0 ; i < multicontrolpoints ; i++)
        {
            for (j = 0; j < 3; j++)
            {
                temp = texU * xa[j];
                temp+= texV * ya[j];
                if (!snapEdge && (set.edge_use_grid != 2))
                {
                    mcontrolpoints[i][j] += temp;
                }
                else
                {
                    mcontrolpoints[i][j] = (float) snapToGrid(mcontrolpoints[i][j]+temp*set.gridsize);
                }
            }
        }
        m->makeSelectedPerform(SEL_CALCWINDINGS);
        strcpy(outstr,"Dragging Edge (3d)");
        //REN_bestBrush->calcWindings();
        break;
    case CLICKFACESHEAR:
        if (REN_bestBrush && REN_bestFace)
        {
            texU = delta.x;
            texV = delta.y;

            TextureAxisFromPlane(&REN_bestFace->plane, xa, ya);

            for (i=0 ; i<numcontrolpoints ; i++)
            {
                for (j = 0; j < 3; j++)
                {
                    temp = 1.0f * texU * xa[j];
                    temp+= 1.0f * texV * ya[j];
                    controlpoints[i][j] += temp;
                }
            }
            strcpy(outstr,"Shearing face");
            REN_bestBrush->calcWindings();
        }
        break;
    case DRAGBRUSHES:
        if (REN_bestBrush && REN_bestFace)
        {
            VectorCopy(REN_bestFace->plane.normal,N);
            if (abs(delta.x) > abs(delta.y))
                texU = delta.x;
            else
                texU = delta.y;


            sb_translate[0] = (texU * 1.0f * N[0]);
            sb_translate[1] = (texU * 1.0f * N[1]);
            sb_translate[2] = (texU * 1.0f * N[2]);

            m->makeSelectedPerform(SEL_TRANSLATE);
            strcpy(outstr,"Dragging brush(es)");
        }
        break;
    case DRAGBRUSHESSIDE:
        if (REN_bestBrush && REN_bestFace)
        {
            texU = delta.x;
            texV = delta.y;

            TextureAxisFromPlane(&REN_bestFace->plane, xa, ya);

            for (j = 0; j < 3; j++)
            {
                temp = 1.0f * texU * xa[j];
                temp+= 1.0f * texV * ya[j];
                sb_translate[j] = temp;
            }

            m->makeSelectedPerform(SEL_TRANSLATE);
            strcpy(outstr,"Dragging brush(es)");
        };
        break;
    }//end switch
    if (mousedown == NAVIGATE || mousedown == MLOOK || mousedown == NAVIGATE_STRAFE)
    {
        RedrawContents();
    }
    else if (mousedown == EDGEDRAG)
    {
        UpdateSelected();
        UpdateOtherViews();
    }
    else if (set.show_texturedrag && REN_bestBrush)
    {
        if ((mousedown == CLICKFACEDRAG) ||
                (mousedown == CLICKFACESHEAR) ||
                (mousedown == DRAGBRUSHESSIDE) ||
                (mousedown == DRAGBRUSHES))
        {
            UpdateSelected();
            UpdateOtherViews();
        }
        else
        {
            RedrawBrush(REN_bestBrush);
        }
    }
    status->SetText(outstr,true);
}


bool TEditWindow::currentEdgeDragFrom()
{
    SetBrush *b;
    Entity *e;
    int j2,k,c;
    face_t *f;
    winding_t *w;
    vec3_t ctr;
    int px, py;
    float fudge = 0.0f;
    map *m;

    m = map_i[set.curmap];
    if (!m)
        return false;

    if (m->numSelected() == 0)
        return false;

    for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
    {
        LoopProblem("CEDFE");
        if (!e->modifiable)
            continue;

        for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
        {
            LoopProblem("CEDFB");
            if (!b->IsDrawable())
                continue;

            if (!b->IsSelected())
                continue;

            for (f = b->faces.p_next; f != &b->faces; f = f->p_next)
            {
                LoopProblem("brush fcp");
                w = f->w;
                if (!w)
                    continue;

                k = w->numpoints - 1;
                for (j2 = 0; j2 < w->numpoints; j2++)
                {
                    for (c = 0; c < 3; c++)
                    {
                        ctr[c] = (w->points[j2][c] + w->points[k][c])/2.0f;
                    };

                    if (!REN_FindCameraPoint(ctr, fudge, &px, &py))
                        continue;

                    // compare px and py to clickx and clicky
                    // If we found ! edge, then just get all points!
                    if ((px >= (Render::clickX - knob_eps)) &&
                            (px <= (Render::clickX + knob_eps)) &&
                            (py >= (Render::clickY - knob_eps)) &&
                            (py <= (Render::clickY + knob_eps)))
                        // click point is in this edge!
                    {
                        int count;
                        count = m->countSelectedBrushesWithEdge(w->points[j2],w->points[k]);
                        if (count <= 0)
                            return false;

                        multicontrolpoints = 0;
                        if (mcontrolpoints)
                        {
                            delete[] mcontrolpoints;
                            maxmulticontrolpoints = 0;
                            mcontrolpoints = NULL;
                        };
                        // 2 points per face, 2 faces per edge...
                        mcontrolpoints = new float *[5*count];
                        maxmulticontrolpoints = 5*count;

                        Entity *e2;
                        SetBrush *b2;
                        for (e2 = m->objects.p_next; e2 && (e2 != &m->objects); e2 = e2->p_next)
                        {
                            LoopProblem("CEDF2E");
                            if (!e2->modifiable)
                                continue;
                            for (b2 = e2->objects.p_next; b2 && (b2 != &e2->objects); b2 = b2->p_next)
                            {
                                LoopProblem("CEDF2B");
                                if (!b2->IsDrawable())
                                    continue;

                                if (!b2->IsSelected())
                                    continue;

                                b2->getEdgeDragFaces(w->points[j2],w->points[k]);
                            };
                        };

                        if (!multicontrolpoints)
                        {
                            delete[] mcontrolpoints;
                            maxmulticontrolpoints = 0;
                            mcontrolpoints = NULL;
                            return false;
                        };

                        m->saveForUndo(const_cast<char *> ("Drag Edge (3d)"),UNDO_BRUSHES);
                        VectorSubtract(w->points[j2],w->points[k],edgeNormal);
                        if (VectorLength(edgeNormal) < 0.0001)
                            return false;

                        VectorNormalize(edgeNormal);
                        return true;
                    }
                    k = j2;
                }
            }
        };
    };

    return false;
};

bool TEditWindow::mouseOnEdgeKnob(int m_x, int m_y)
{
    SetBrush *b;
    Entity *e;
    int j2,k,c;
    face_t *f;
    winding_t *w;
    vec3_t ctr = {0, 0, 0};
    int px, py;
    float fudge = 0.05f;
    map *m;

    m = map_i[set.curmap];
    if (!m)
        return false;

    if (m->numSelected() == 0)
        return false;

    for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
    {
        LoopProblem("CEMEKE");
        if (!e->modifiable)
            continue;

        for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
        {
            LoopProblem("CEMEKB");
            if (!b->IsDrawable())
                continue;

            if (!b->IsSelected())
                continue;

            for (f = b->faces.p_next; f != &b->faces; f = f->p_next)
            {
                LoopProblem("brush edge knob");
                w = f->w;
                if (!w)
                    continue;

                k = w->numpoints - 1;
                for (j2 = 0; j2 < w->numpoints; j2++)
                {
                    for (c = 0; c < 3; c++)
                    {
                        float a1 = (float) w->points[j2][c];
                        float a2 = (float) w->points[k][c];
                        a1 = (a1 + a2) / 2.f;
                        ctr[c] = a1;
                        //ctr[c] = w->points[j2][c];
                        //ctr[c] += w->points[k][c];
                        //	ctr[c] /= 2.0;
                    };
                    //if (b->pointAlreadyProcessed(ctr, i2))
                    //	continue;

                    if (!REN_FindCameraPoint(ctr, fudge, &px, &py))
                        continue;

                    // compare px and py to clickx and clicky
                    // If we found ! edge, then just get all points!
                    if ((px >= (m_x - knob_eps)) &&
                            (px <= (m_x + knob_eps)) &&
                            (py >= (m_y - knob_eps)) &&
                            (py <= (m_y + knob_eps)))
                        // click point is in this edge!
                    {
                        return true;
                    }
                    k = j2;
                }
            }
        }
    }

    return false;
}

bool TEditWindow::currentFaceDragFrom()
{
    if (!REN_bestBrush)
        return false;

    SetBrush *b = REN_bestBrush;

    controlpoints[0] = b->currentFace->planepts[0];
    controlpoints[1] = b->currentFace->planepts[1];
    controlpoints[2] = b->currentFace->planepts[2];
    numcontrolpoints=3;

    map_i[set.curmap]->saveForUndo(const_cast<char *> ("Drag Current Face (3d)"),UNDO_RENBRUSH);

    return true;
}

bool TEditWindow::shearCurrentFace()
{
    SetBrush *br;

    if (!REN_bestBrush)
        return false;

    br = REN_bestBrush;

    br->getShearPoints(REN_bestFace);
    if (!numcontrolpoints)
        return false;

    map_i[set.curmap]->saveForUndo(const_cast<char *> ("Shear Face (3d)"),UNDO_BRUSHES);

    return true;
}

bool TEditWindow::selectionDragFrom()
{
    map_i[set.curmap]->saveForUndo(NULL,UNDO_NOUNDO);

    return true;
}

bool TEditWindow::selectionDragFromSide()
{
    map_i[set.curmap]->saveForUndo(NULL,UNDO_NOUNDO);

    return true;
}


void TEditWindow::ParseMouse()
{
    if (MouseParsed3D)
        return;

    MouseParsed3D = true;

    char *dat = (char*) file_get_contents(const_cast<char *> ("settings\\bspmou3d.cfg"));
    if (!dat)
    {
        syserror (const_cast<char *> ("Error opening settings\\bspmou3d.cfg"));
        return;
    }

    Tokenizer script(dat);
    if (!script.next(true))
    {
        syserror (const_cast<char *> ("Parsing bspmou3d.cfg: Number of Setups not found"));
        goto error_exit;
    }
    numEventTypes3d = atoi(script.token);
    if (numEventTypes3d <= 0 || numEventTypes3d > 10)
    {
        syserror(const_cast<char *> ("Parsing bspmou3d.cfg: Number of Setups (1 min - 10 max)invalid"));
        goto error_exit;
    }
    memset(head3d,0,numEventTypes3d * sizeof(mouseEventType *));
    memset(EventTypes3d,0,numEventTypes3d * sizeof(char *));

    if (!script.next(true))
    {
        syserror (const_cast<char *> ("Parsing bspmou3d.cfg: Current Setup not found"));
        goto error_exit;
    }
    curEventType3d = atoi(script.token);
    if (curEventType3d <= 0 || curEventType3d > numEventTypes3d)
    {
        syserror (const_cast<char *> ("Parsing bspmou3d.cfg: Current Setup invalid"));
        goto error_exit;
    }

    curEventType3d--; // adjust down...

    for (int i = 0; i < numEventTypes3d; i++)
    {
        mouseEventType *hd = NULL;
        mouseEventType *cur = NULL, *temp;

        if (!script.next(true))
        {
            syserror (const_cast<char *> ("Parsing bspmou3d.cfg: '{' not found"));
            goto error_exit;
        }
        if (!script.next(true))
        {
            syserror (const_cast<char *> ("Parsing bspmou3d.cfg: Current Setup Name not found"));
            goto error_exit;
        }
        strncpy(EventTypes3d[i],script.token,32);

        while(true)
        {
            if (!script.next(true))
                break;  // done...
            if (strcmpi (script.token, "{"))
                break;
            if (!script.next(false))
                break;  // done...

            temp = cur;
            cur = new mouseEventType;
            if (hd)
                temp->next = cur;
            else
                hd = cur;

            cur->count = CountWads(script.token);
            if (cur->count <= 0 || cur->count >= 10)
            {
                syserror(const_cast<char *> ("Parse bspmou3d.cfg:  Invalid number of mouse options [%i] (must be 0-10)"),cur->count);
                goto error_exit;
            }

            cur->moves = new char *[cur->count];
            memset(cur->moves,0,cur->count*sizeof(char *));

            int counter;
            char *start = script.token;
            char *end;
            for (counter = 0; counter < cur->count; counter++)
            {
                cur->moves[counter] = new char[32];
                if (!cur->moves[counter])
                {
                    syserror(const_cast<char *> ("Parse bspmou3d.cfg:  Couldn't allocate moves[%i]..."),counter);
                    goto error_exit;
                }
                memset(cur->moves[counter],0,32*sizeof(char));


                for (end = start; end && *end && *end != ';' && *end != ' ' ; end++);

                strncpy(cur->moves[counter],start,(int)(end-start));
                cur->moves[counter][(int)(end-start)] = '\0';

                start = ++end;
            }

            if (!script.next(false))
                break;  // done...

            if (!strcmpi(script.token,"LEFT"))
                cur->whichMouse = 0;
            else if (!strcmpi(script.token,"MIDDLE"))
                cur->whichMouse = 1;
            else if (!strcmpi(script.token,"RIGHT"))
                cur->whichMouse = 2;
            else if (!strcmpi(script.token,"X1"))
                cur->whichMouse = 3;
            else if (!strcmpi(script.token,"X2"))
                cur->whichMouse = 4;
            else
                syserror(const_cast<char *> ("Mouse bspmou3d.cfg:  Bad mouse type \"%s\" "
                                             "(should be one of: left, middle, right, x1, x2)"), script.token);

            while(true)
            {
                if (!script.next(false))
                    break;  // done...

                if (!strcmpi(script.token,"}"))
                    break;

                if (!strcmpi(script.token,"SHIFT"))
                    cur->moveFlags |= SHIFT_FLAG;
                else if (!strcmpi(script.token,"CTRL"))
                    cur->moveFlags |= CTRL_FLAG;
                else if (!strcmpi(script.token,"ALT"))
                    cur->moveFlags |= ALT_FLAG;
            }
        }
        head3d[i] = hd;
    }
error_exit:
    delete[] dat;
}

int TEditWindow::TranslateCommand(char *in)
{
    if (!strcmpi(in,"SHIFT_TEXTURE"))
        return SHIFT_TEXTURE;
    if (!strcmpi(in,"STRETCH_TEXTURE"))
        return STRETCH_TEXTURE;
    if (!strcmpi(in,"ROTATE_TEXTURE"))
        return ROTATE_TEXTURE;
    if (!strcmpi(in,"SELECT"))
        return SELECT_CALLBACK;
    if (!strcmpi(in,"SELECTONE"))
        return SELECTONE_CALLBACK;
    if (!strcmpi(in,"APPLYCURTEXTOBRUSH"))
        return TEXTUREBRUSH_CALLBACK;
    if (!strcmpi(in,"APPLYCURTEXTOFACE"))
        return TEXTUREFACE_CALLBACK;
    if (!strcmpi(in,"LIFTTEX"))
        return LIFT_TEXTURE;
    if (!strcmpi(in,"LIFT&APPLY"))
        return LIFT_APPLY;
    if (!strcmpi(in,"CLICKFACEDRAG"))
        return CLICKFACEDRAG;
    if (!strcmpi(in,"CLICKFACESHEAR"))
        return CLICKFACESHEAR;
    if (!strcmpi(in,"DRAGBRUSHES"))
        return DRAGBRUSHES;
    if (!strcmpi(in,"DRAGBRUSHESSIDE"))
        return DRAGBRUSHESSIDE;
    if (!strcmpi(in,"SETCLIPPER"))
        return SETCLIPPER;
    if (!strcmpi(in,"SETCLIPPERINV"))
        return SETCLIPPERINV;
    if (!strcmpi(in,"NAVIGATE"))
        return NAVIGATE;
    if (!strcmpi(in,"NAVIGATE_STRAFE"))
        return NAVIGATE_STRAFE;
    if (!strcmpi(in,"EDGEDRAG"))
        return EDGEDRAG;
    if (!strcmpi(in,"MLOOK"))
        return MLOOK;

    return NO_MOUSE;
}

void TEditWindow::StartMouse(UINT modkeys, POINT *point, int whichMouse)
{
    int i, j;
    Entity *e;
    SetBrush *b;
    texturedef_t *td;

    if (!set.Map_Read || mousedown)
        return;

    map *m = map_i[set.curmap];
    if (!m || !m->current)
        return;

    if(set.angle_control != NO_ANGLE_CONTROL)
    {
        ///////////////////////////////////////////////
        // First, figure out if rotation is clicked...
        ///////////////////////////////////////////////
        POINT cent;
        cent.x = EditSize.cx - 25;
        cent.y = 25;

        int dx = point->x - cent.x;
        int dy = point->y - cent.y;

        if (
            (abs(dx) <= 30) && (abs(dy) <= 30) &&
            ((abs(dx) >=2)  || (abs(dy) >= 2)))
        {
            if(whichMouse == 0)
            {
                mousedown = ANGLE_CONTROL;

                if(set.angle_control == PITCH_CONTROL)
                    SetPitch(point);
                else if(set.angle_control == YAW_CONTROL)
                    SetYaw(point);
                else
                    goto fov_ctrl;	// let the fov code handle it

                SetCapture(hwnd);

            }
            else if(whichMouse == 2)
            {
                set.redrawedit = 1;
                if(set.angle_control == FOV_CONTROL)  			// fov to yaw
                {
                    set.angle_control = YAW_CONTROL;
                    Show_Frame("Yaw control",true);
                }
                else if(set.angle_control == YAW_CONTROL)  	// yaw to pitch
                {
                    set.angle_control = PITCH_CONTROL;
                    Show_Frame("Pitch control",true);
                }
                else  										// pitch to fov
                {
                    set.angle_control = FOV_CONTROL;
                    Show_Frame("Fov Control",true);
                }
            }
            return;
        }
        ///////////////////////////////////////////////
        // figure out if fov control is clicked
        //////////////////////////////////////////////
fov_ctrl:
        if(set.angle_control &&
                point->x > (EditSize.cx - fovsz.cx - 5) && point->x < (EditSize.cx - 5) &&
                point->y > 5 && point->y < (fovsz.cy + 5) )
        {
            if(whichMouse == 0)  				//left clicking
            {
                mousedown = ANGLE_CONTROL;
                SetFov(point);
                SetCapture(hwnd);
            }
            else if (whichMouse == 2)  		//right clicking, switch fov to yaw
            {
                set.angle_control = YAW_CONTROL;
                set.redrawedit = 1;
                Show_Frame("Yaw Control",true);	//switch back to yaw control
            }
            return;
        }
        ///////////////////////////////////////////////
        ///////////////////////////////////////////////
    }


    // Now check if there is a mouse handler...
    unsigned char flags = 0;

    if (modkeys & (MK_SHIFT))
        flags |= SHIFT_FLAG;

    if (modkeys & (MK_CONTROL))
        flags |= CTRL_FLAG;

    if(0x8000 & GetKeyState(VK_MENU))
        flags |= ALT_FLAG;

    mouseEventType *p = head3d[curEventType3d];
    while (p)
    {
        if ((p->whichMouse == whichMouse) && (p->moveFlags == flags))
            break;
        p = p->next;
    }
    if (!p)				// no handler found...
        return;


    // found a handler..
    // find what's under the mouse...

    if (!set.glBsp)
    {
        r_picbuffer = imagebuffer;
        r_zbuffer   = zbuffer;
        r_width = set.render_width; // EditSize.cx;
        r_height = set.render_height; // EditSize.cy;

        VectorCopy(m->eye[m->cureye],r_origin);
        vec3_t zero;
        VectorCopy(vec3_origin,zero);



        float Q[4][4];
        float Temp[4][4];

        findQ(Q,Temp,zero,
              m->angles[m->cureye].roll,
              m->angles[m->cureye].pitch,
              m->angles[m->cureye].yaw
             );

        // vpn
        r_matrix[0][0] = Q[0][0];
        r_matrix[0][1] = Q[0][1];
        r_matrix[0][2] = Q[0][2];

        r_matrix[1][0] = Q[1][0];
        r_matrix[1][1] = Q[1][1];
        r_matrix[1][2] = Q[1][2];

        r_matrix[2][0] = Q[2][0];
        r_matrix[2][1] = Q[2][1];
        r_matrix[2][2] = Q[2][2];
    }

    REN_bestBrush = NULL;
    REN_bestDistance = TMAX;		//was BOGUS_RANGE;


    if (set.glBsp)
    {
        Render::clickX = (int)(point->x);
        Render::clickY = (int)(point->y);    // do from bottom of screen?
        getRenBrush(Render::clickX, Render::clickY);
    }
    else
    {
        Render::clickX = (int)( ((float)point->x/(float)EditSize.cx)*(float)set.render_width);
        Render::clickY = (int)( ((float)point->y/(float)EditSize.cy)*(float)set.render_height);

        REN_BeginCamera ();

        m->makeAllPerform(SEL_FINDCLOSEST);
    }

    startpt.x = point->x;
    startpt.y = point->y;
    delta.x = delta.y = 0;
    relative.x = relative.y = 0;
    oldrelative = relative;
    startA = 0.0;

    // parse out the string...
    int action;
    bool handled = false;

    r_width = set.render_width; // EditSize.cx;
    r_height = set.render_height; // EditSize.cy;


    for (i = 0; i < p->count; i++)
    {
        handled = false;
        //set "modal" command for plain left mouse only
        if(!modal || whichMouse || flags)
            action = TranslateCommand(p->moves[i]);
        else
            action = modal;

        switch (action)
        {
        case MLOOK:
        case NAVIGATE_STRAFE:
        case NAVIGATE:
        {
            mouse_is_navigating = true;
            handled = true;
            break;
        }
        case SETCLIPPER:
            if (REN_bestBrush)
            {
                handled = true;
                m->clipper_i->num = 3;
                for (j = 0; j < 3; j++)
                {
                    m->clipper_i->pos[0][j] = REN_bestFace->planepts[0][j];
                    m->clipper_i->pos[1][j] = REN_bestFace->planepts[1][j];
                    m->clipper_i->pos[2][j] = REN_bestFace->planepts[2][j];
                }
            }
            break;
        case SETCLIPPERINV:
            if (REN_bestBrush)
            {
                handled = true;
                m->clipper_i->num = 3;
                for (j = 0; j < 3; j++)
                {
                    m->clipper_i->pos[0][j] = REN_bestFace->planepts[0][j];
                    m->clipper_i->pos[2][j] = REN_bestFace->planepts[1][j];
                    m->clipper_i->pos[1][j] = REN_bestFace->planepts[2][j];
                }
            }
            break;
        case CLICKFACESHEAR:
            if (REN_bestBrush)
                handled = shearCurrentFace();
            break;
        case DRAGBRUSHES:
            if (REN_bestBrush && REN_bestBrush->IsSelected())
                handled = selectionDragFrom();
            break;
        case DRAGBRUSHESSIDE:
            if (REN_bestBrush && REN_bestBrush->IsSelected())
                handled = selectionDragFrom();
            break;
        case CLICKFACEDRAG:
            if (REN_bestBrush)
                handled = currentFaceDragFrom();
            break;
        case EDGEDRAG:
            if (set.center_knobs_3d)
            {
                if (set.glBsp)
                {
                    handled = glClickOnEdgeKnob(Render::clickX, Render::clickY);
                }
                else
                {
                    handled = currentEdgeDragFrom();
                }
            }
            else
            {
                handled = false;
            }
            break;
        case SELECT_CALLBACK:
        case SELECTONE_CALLBACK:
            if (REN_bestBrush)
            {
                handled = true;
                if (REN_bestBrush->parent != m->current)
                {
                    if (!set.multi_select)
                        m->makeSelectedPerform(SEL_DESELECT);
                    m->setCurrentEntity(REN_bestBrush->parent);
                }

                if(action==SELECTONE_CALLBACK)
                {
                    if(m->numSelected() > 0)
                    {
                        bool selstate = REN_bestBrush->IsSelected();
                        m->makeGlobalPerform(SEL_DESELECT);
                        m->setCurrentEntity(m->world);
                        REN_bestBrush->setSelected(selstate);
                    }
                    action=SELECT_CALLBACK;	//regular select from here on...
                }

                if (!REN_bestBrush->IsSelected())
                {
                    if (m->numSelected() == 0)  	// don't grab texture if others are selected
                    {
                        td = REN_bestBrush->texturedefForFace(REN_bestFace);
                        texWindow->setTextureDef(td);
                    }
                    REN_bestBrush->setSelected(true);
                }
                else
                {
                    REN_bestBrush->setSelected(false);
                }
                REN_bestBrush->currentFace = REN_bestFace;
            }
            break;
        case SHIFT_TEXTURE:

            //do not attempt to modify if not modifiable!
            if(!REN_bestBrush || !REN_bestBrush->checkModifiable())
            {
                mousedown = 0;
                return;
            }

            //select new face
            REN_bestBrush->currentFace = REN_bestFace;

            if ((set.drawmode == Drawmode::texture) && (REN_bestBrush))
            {
                m->saveForUndo(const_cast<char *> ("Shift Texture"),UNDO_RENBRUSH);
                handled = true;
            }
            break;
        case STRETCH_TEXTURE:

            //do not attempt to modify if not modifiable!
            if(!REN_bestBrush || !REN_bestBrush->checkModifiable())
            {
                mousedown = 0;
                return;
            }

            //select new face
            REN_bestBrush->currentFace = REN_bestFace;

            if ((set.drawmode == Drawmode::texture) && (REN_bestBrush))
            {
                m->saveForUndo(const_cast<char *> ("Stretch Texture"),UNDO_RENBRUSH);
                handled = true;
            }
            break;
        case ROTATE_TEXTURE:

            //do not attempt to modify an unmodifiable object!
            if(!REN_bestBrush || !REN_bestBrush->checkModifiable())
            {
                mousedown = 0;
                return;
            }

            //select face under mouse
            REN_bestBrush->currentFace = REN_bestFace;

            if (set.drawmode == Drawmode::texture && REN_bestBrush)
            {
                m->saveForUndo(const_cast<char *> ("Rotate Texture"),UNDO_RENBRUSH);
                handled = true;
                td = REN_bestBrush->texturedefForFace(REN_bestFace);
                if (td)
                {
                    startA = td->rotate;

                    start_s = td->shift[0];
                    start_t = td->shift[1];
                    VectorCopy(REN_bestBrush->bctr, start_tex_rotate);
                }
            }
            break;
        case LIFT_TEXTURE:

            if (REN_bestBrush)
            {
                //select new face
                REN_bestBrush->currentFace = REN_bestFace;

                m->saveForUndo(NULL,UNDO_NOUNDO);
                handled = true;
                td = REN_bestBrush->texturedefForFace(REN_bestFace);
                texWindow->setTextureDef(td);
            }
            break;
        case LIFT_APPLY:
            if (REN_bestBrush)
            {
                //select new face
                REN_bestBrush->currentFace = REN_bestFace;

                m->saveForUndo(NULL,UNDO_NOUNDO);
                handled = true;
                td = REN_bestBrush->texturedefForFace(REN_bestFace);
                texWindow->setTextureDef(td);

                for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
                {
                    LoopProblem("CEAPPLYE");
                    if (!e->modifiable)
                        continue;

                    for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
                    {
                        LoopProblem("CEAPPLYB");
                        if (!b->IsDrawable())
                            continue;
                        if (!b->IsSelected())
                            continue;
                        b->takeCurrentTexture();
                    }
                }
            }
            break;
        case TEXTUREBRUSH_CALLBACK:
            if (REN_bestBrush && REN_bestBrush->parent->modifiable)
            {
                //select new face
                REN_bestBrush->currentFace = REN_bestFace;

                m->saveForUndo(const_cast<char *> ("Texture Brush"),UNDO_RENBRUSH);
                REN_bestBrush->takeCurrentTexture();
                handled = true;
            }
            break;
        case TEXTUREFACE_CALLBACK:
            if (REN_bestBrush && REN_bestBrush->parent->modifiable && REN_bestFace)
            {
                m->saveForUndo(const_cast<char *> ("Texture Face"),UNDO_RENBRUSH);

                //select new face
                REN_bestBrush->currentFace = REN_bestFace;

                texturedef_t tempT;
                texWindow->getTextureDef(&tempT);
                REN_bestFace->texture = tempT;
                if (REN_bestFace->qtexture)
                    delete REN_bestFace->qtexture;

                REN_bestFace->qtexture = NULL;
                REN_bestBrush->calcWindings();	// in case texture coords changed
            }
            handled = true;
            break;
        }

        if(handled)
            break;
    }

    if(!handled)
        return;

    mousedown = action;
    SetCapture(hwnd);
}

void TEditWindow::StopMouse(POINT* /*point*/, int /*whichouse*/)
{
    if (!set.Map_Read || !mousedown)
        return;

    ReleaseCapture();

    mouse_is_navigating = false;

    texturedef_t  *td;

    r_picbuffer = imagebuffer;
    r_zbuffer = zbuffer;

    map *m = map_i[set.curmap];
    if(!m)
        return;

    Entity *e;
    SetBrush *b;

    int mdown = mousedown;
    mousedown = NO_MOUSE;

    // finish up
    switch (mdown)
    {
    case NAVIGATE:
        set.redrawxy = 2;
        set.redrawedit = 1;
        Show_Frame("Mouse Navigation...",true);
        return;
    case NAVIGATE_STRAFE:
        set.redrawxy = 2;
        set.redrawedit = 1;
        Show_Frame("Mouse Strafe Navigation...",true);
        return;
    case MLOOK:
        set.redrawxy = 2;
        set.redrawedit = 1;
        Show_Frame("Mouse Look...",true);
        return;
    case CLICKFACESHEAR:
        if (REN_bestBrush)
        {

            REN_bestBrush->removeUnusedFaces();
            if (REN_bestBrush->IsInvalid())
            {
                delete REN_bestBrush;
                REN_bestBrush = NULL;
            }
        }
        set.redrawxy = 1;
        set.redrawedit = 1;
        Show_Frame("Face Sheared...",true);
        return;
    case SETCLIPPER:
        set.redrawxy = 1;
        set.redrawedit = 1;
        Show_Frame("Clipper Lifted...",true);
        return;
    case SETCLIPPERINV:
        set.redrawxy = 1;
        set.redrawedit = 1;
        Show_Frame("Inverted Clipper Lifted...",true);
        return;
    case DRAGBRUSHES:
        set.redrawxy = 1;
        set.redrawedit = 1;
        Show_Frame("Brushes Dragged...",true);
        return;
    case DRAGBRUSHESSIDE:
        set.redrawxy = 1;
        set.redrawedit = 1;
        Show_Frame("Brushes Dragged Sideways...",true);
        return;
    case CLICKFACEDRAG:
        if (REN_bestBrush)
        {
            REN_bestBrush->removeUnusedFaces();
            if (REN_bestBrush->IsInvalid())
            {
                delete REN_bestBrush;
                REN_bestBrush = NULL;
            };
        };
        set.redrawxy = 1;
        set.redrawedit = 1;
        Show_Frame("Face Dragged...",true);
        return;
    case EDGEDRAG:
        for (e = m->objects.p_next; e && (e != &m->objects); e = e->p_next)
        {
            LoopProblem("CEDRAGEE");
            if (!e->modifiable)
                continue;

            for (b = e->objects.p_next; b && (b != &e->objects); b = b->p_next)
            {
                LoopProblem("CEDRAGEB");
                if (!b->IsDrawable())
                    continue;
                if (!b->IsSelected())
                    continue;
                b->removeUnusedFaces();
            }
        }

        m->cleanUpInvalidObjects();

        multicontrolpoints = 0;
        if (mcontrolpoints)
        {
            delete[] mcontrolpoints;
            maxmulticontrolpoints = 0;
            mcontrolpoints = NULL;
        };

        set.redrawxy = 1;
        set.redrawedit = 1;
        Show_Frame("Edge Dragged...",true);
        return;
    case SELECT_CALLBACK:
        //mousedown = NO_MOUSE;
        set.redrawxy = 1;
        set.redrawedit = 1;

        Show_Frame("Select / Deselect",true);
        return;
    case SHIFT_TEXTURE:
        //mousedown = NO_MOUSE;
        if (!REN_bestBrush) // problem...
            return;
        if (!REN_bestBrush->parent->modifiable)
            return;

        td = REN_bestBrush->texturedefForFace(REN_bestFace);
        if (!td)
            return;

        if (!set.show_texturedrag)
        {
            td->shift[0] += (float)relative.x*set.tex_shiftstep;
            td->shift[1] += (float)-relative.y*set.tex_shiftstep;
        };
        break;
    case STRETCH_TEXTURE:
        //mousedown = NO_MOUSE;
        if (!REN_bestBrush) // problem...
            return;
        if (!REN_bestBrush->parent->modifiable)
            return;

        td = REN_bestBrush->texturedefForFace(REN_bestFace);
        if (!td)
            return;

        if (!set.show_texturedrag)
        {
            td->scale[0] += (float)relative.x*set.tex_scalestep;
            td->scale[1] -= (float)relative.y*set.tex_scalestep;
        };
        break;
    case ROTATE_TEXTURE:
        if (!REN_bestBrush || !REN_bestBrush->parent->modifiable)		// problem...
            return;

        td = REN_bestBrush->texturedefForFace(REN_bestFace);
        if (!td)
            return;

        if (!set.show_texturedrag)
        {
            td->rotate += (startA+(float)relative.y*set.tex_rotatestep);

            while (td->rotate >= 360.0)
                td->rotate -= 360.0;
            while (td->rotate < 0.0)
                td->rotate += 360.0;
        }
        break;
    case LIFT_TEXTURE:
        set.redrawxy = 0;
        set.redrawedit = 1;
        Show_Frame("Texture lifted...",true);
        return;
    case LIFT_APPLY:
        set.redrawxy = 0;
        set.redrawedit = 1;
        Show_Frame("Texture lifted and applied to selected brushes...",true);
        return;
    case TEXTUREBRUSH_CALLBACK:
        set.redrawxy = 0;
        set.redrawedit = 1;
        Show_Frame("Current texture applied to brush...",true);
        return;
    case TEXTUREFACE_CALLBACK:
        set.redrawxy = 0;
        set.redrawedit = 1;
        Show_Frame("Current texture applied to face...",true);
        return;
    default:
        return;
    }

    if (td && REN_bestBrush)
    {
        REN_bestFace->texture = *td;
        if (REN_bestFace->qtexture)
        {
            delete REN_bestFace->qtexture;
        }
        REN_bestFace->qtexture = NULL;
        REN_bestBrush->calcWindings();	// in case texture coords changed
    }

    set.redrawxy = 1;
    set.redrawedit = 1;

    Show_Frame("",true);
}

bool TEditWindow::DoNCRButtonDown(UINT /*hitTest*/, POINT screenPt2)
{
    if (IsIconic())
        return false;

    int i;

    Popup ModeMenu;

    // mouse mode

    if (modal == NO_MOUSE)
        ModeMenu.Check();
    ModeMenu.Add("Normal",MODEBASE);

    if (modal == SELECT_CALLBACK)
        ModeMenu.Check();
    ModeMenu.Add("Select",MODEBASE+1);

    if (modal == SELECTONE_CALLBACK)
        ModeMenu.Check();
    ModeMenu.Add("Select One",MODEBASE+18);

    ModeMenu.Separator();

    // texture manipulation

    if (modal == SHIFT_TEXTURE)
        ModeMenu.Check();
    ModeMenu.Add("Shift",MODEBASE+2);

    if (modal == STRETCH_TEXTURE)
        ModeMenu.Check();
    ModeMenu.Add("Stretch",MODEBASE+3);

    if (modal == ROTATE_TEXTURE)
        ModeMenu.Check();
    ModeMenu.Add("Rotate",MODEBASE+4);

    ModeMenu.Separator();

    // single mouse ops

    if (modal == TEXTUREBRUSH_CALLBACK)
        ModeMenu.Check();
    ModeMenu.Add("Apply to Brush",MODEBASE+5);

    if (modal == TEXTUREFACE_CALLBACK)
        ModeMenu.Check();
    ModeMenu.Add("Apply to Face",MODEBASE+6);

    if (modal == LIFT_TEXTURE)
        ModeMenu.Check();
    ModeMenu.Add("Lift",MODEBASE+7);

    if (modal == LIFT_APPLY)
        ModeMenu.Check();
    ModeMenu.Add("Lift and Apply",MODEBASE+8);

    if (modal == CLICKFACEDRAG)
        ModeMenu.Check();
    ModeMenu.Add("Drag Face",MODEBASE+9);

    if (modal == CLICKFACESHEAR)
        ModeMenu.Check();
    ModeMenu.Add("Shear Face",MODEBASE+10);

    if (modal == DRAGBRUSHES)
        ModeMenu.Check();
    ModeMenu.Add("Drag Brushes",MODEBASE+11);

    if (modal == DRAGBRUSHESSIDE)
        ModeMenu.Check();
    ModeMenu.Add("Drag Brushes Sideways",MODEBASE+12);

    if (modal == SETCLIPPER)
        ModeMenu.Check();
    ModeMenu.Add("Set Clipper from Face",MODEBASE+13);

    if (modal == SETCLIPPERINV)
        ModeMenu.Check();
    ModeMenu.Add("Set Inverted Clipper from Face",MODEBASE+14);

    if (modal == NAVIGATE)
        ModeMenu.Check();
    ModeMenu.Add("Move Camera",MODEBASE+15);

    if (modal == NAVIGATE_STRAFE)
        ModeMenu.Check();
    ModeMenu.Add("Strafe Camera",MODEBASE+19);

    if (modal == EDGEDRAG)
        ModeMenu.Check();
    ModeMenu.Add("Edge Face",MODEBASE+16);

    if (modal == MLOOK)
        ModeMenu.Check();
    ModeMenu.Add("Mouse look",MODEBASE+17);


    Popup PopupMenu;
    PopupMenu.Add("Mode",ModeMenu);
    PopupMenu.Separator();
    PopupMenu.Add("Display Settings", HELPBUTTON);
    PopupMenu.Separator();

    for (i = 0; i < numEventTypes3d; i++)
    {
        if (i == curEventType3d)
            PopupMenu.Check();
        PopupMenu.Add(EventTypes3d[i], MOUSEBASE+i);
    }
    PopupMenu.Separator();

    //angel control
    if(set.angle_control == YAW_CONTROL)
        PopupMenu.Check();
    PopupMenu.Add("Yaw Control", EDITMENU_YAW_CONTROL);
    if(set.angle_control == PITCH_CONTROL)
        PopupMenu.Check();
    PopupMenu.Add("Pitch Control", EDITMENU_PITCH_CONTROL);
    if(set.angle_control == FOV_CONTROL)
        PopupMenu.Check();
    PopupMenu.Add("FOV Control", EDITMENU_FOV_CONTROL);
    if(set.angle_control == NO_ANGLE_CONTROL)
        PopupMenu.Check();
    PopupMenu.Add("Hide Control", EDITMENU_NO_ANGLE_CONTROL);

    PopupMenu.Separator();

    if (set.render_connections)
        PopupMenu.Check();
    PopupMenu.Add("Show Connections", CONNECTIONBUTTON);

    if (set.center_knobs_3d)
        PopupMenu.Check();
    PopupMenu.Add("Edge Knobs", CENTERKNOBS);

    PopupMenu.Separator();
    PopupMenu.Add("Time Refresh", TIMEREFRESH);

    PopupMenu.Show(hwnd);
    return true;
}
