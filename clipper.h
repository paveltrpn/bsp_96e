#pragma once

class TXYViewWindow;
class Clipper
{
public:
    int			num;
    vec3_t		pos[3];
    plane_t		plane;
    bool        state;

    Clipper();
    Clipper *copy();
    bool hide();
    void dragFrom(vec3_t location, int pttomove, int , int, int whichXY);
    void DragCallBack(float dx, float dy, int i, int, int);
    void XYClick(vec3_t pt, int, int, int, int);
    bool XYDrag(vec3_t pt, int , int, int);
    bool XYDragWhole(vec3_t pt, int , int, int);
    void carve();
    void split(bool);
    void flipNormal();
    bool getFace(face_t *pl);

    void cameraDrawSelf(HDC);
    void XYDrawSelf(TXYViewWindow *xy, HDC hdc, bool notBlack);
};
