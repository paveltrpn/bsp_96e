#ifndef _BSPPCH_H_
#define _BSPPCH_H_

#pragma warning( disable: 6211 )	//leaking memory due to an exception

// Main precompiled header file for BSP
#define _WIN32_IE 0x0500
#define _WIN32_WINNT 0x0500
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#undef M_PI		// we want floats
#define M_PI 3.14159265358979323846f
#include <fcntl.h>
#include <io.h>
#include <time.h>

#include <windows.h>
#include <commctrl.h>
#include <GL\gl.h> //"\bc5\bsp\gl.h"
#include <GL\glu.h>

#endif	//_BSPPCH_H_
