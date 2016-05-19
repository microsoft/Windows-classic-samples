
/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples. 
*       Copyright 1993 - 2000 Microsoft Corp.
*       All rights reserved. 
*       This source code is only intended as a supplement to 
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the 
*       Microsoft samples programs.
\******************************************************************************/

#define IDM_ABOUT 100

typedef POINTS MPOINT;
#define MPOINT2POINT(mpt,pt)  ((pt).x = (mpt).x, (pt).y = (mpt).y)
#define POINT2MPOINT(pt, mpt) ((mpt).x = (SHORT)(pt).x, (mpt).y = (SHORT)(pt).y)

BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, INT);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
INT sieve(VOID);
