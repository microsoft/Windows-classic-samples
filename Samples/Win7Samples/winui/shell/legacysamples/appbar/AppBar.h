// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//


//////////////////////////////////////////////////////////////////////////////
// Constants and Enumerations

#define CX_DEFWIDTH     80
#define CY_DEFHEIGHT    60

#define IDT_AUTOHIDE    1
#define IDT_AUTOUNHIDE  2

#define APPBAR_TOP      0
#define APPBAR_BOTTOM   1
#define APPBAR_LEFT     2
#define APPBAR_RIGHT    3

#define APPBAR_CALLBACK     (WM_USER + 1010)
#define ErrorHandler() ErrorHandlerEx(__LINE__, __FILE__)


//////////////////////////////////////////////////////////////////////////////
// Types

typedef struct tagOPTIONS
{
    BOOL fAutoHide;
    BOOL fOnTop;
    BOOL fHiding;
    UINT uSide;
    DWORD cxWidth;
    DWORD cyHeight;
    RECT rcEdges[4];
} OPTIONS, *POPTIONS;


//////////////////////////////////////////////////////////////////////////////
// Global Variables

extern HINSTANCE g_hInstance;
extern BOOL g_fAppRegistered;
extern RECT g_rcAppBar;


//////////////////////////////////////////////////////////////////////////////
// Prototypes

void AppBar_Size(HWND);
void AppBar_QueryPos(HWND, LPRECT);
void AppBar_QuerySetPos(UINT, LPRECT, PAPPBARDATA, BOOL);
void AppBar_Callback(HWND, UINT, WPARAM, LPARAM);
void AppBar_PosChanged(PAPPBARDATA);
BOOL AppBar_SetAutoHide(HWND hwnd, BOOL fHide);
BOOL AppBar_UnRegister(HWND hwnd);
BOOL AppBar_Register(HWND hwnd);
BOOL AppBar_SetSide(HWND hwnd, UINT uSide);
void AppBar_SetAlwaysOnTop(HWND hwnd, BOOL fOnTop);
void AppBar_Hide(HWND hwnd);
void AppBar_UnHide(HWND hwnd);
void AppBar_SetAutoHideTimer(HWND hwnd);
void AppBar_SetAutoUnhideTimer(HWND hwnd);
POPTIONS GetAppbarData(HWND);
void ErrorHandlerEx(INT, LPSTR);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
void ShowOptions(HWND hwndParent);
