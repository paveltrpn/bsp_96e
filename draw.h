#pragma once
#include "global.h"

/*
//TEST DRAWING INTERFACE
//ISurface
class IDrawMemBuffer {
public:
	virtual void Create(HDC compat, int width, int height)=0;
	virtual void Destroy()=0;
};
//IPainter
class IPainter {
public:
	//surfaces
	virtual bool SetSurface(ISurface*)=0;
	virtual bool CopySurface(ISurface*)=0;
	virtual bool Resize(int width, int height)=0;
	virtual void Render(HWND, int x, int y, ISurface*)=0;
	//styles
	virtual void SetLineWidth(int size)=0;
	virtual void SetForecolor(COLORREF color)=0;
	virtual void SetBackcolor(COLORREF color)=0;
	virtual void SetBackMode(int mode)=0;
	//fonts - how to generically configure fonts?
	virtual void TextOut(int size, int x, int y, char *text)=0;
	//drawing
	virtual void Ellipse(int x1,int y1,int x2,int y2)=0;
	virtual void Rectangle(int x1,int y1,int x2,int y2)=0;
	virtual void DrawControlPoint(int x, int y, int type)=0;
	virtual void MoveTo(int x, int y)=0;
	virtual void LineTo(int x, int y)=0;
	virtual void SetPixel(int x, int y)=0;
};
*/
