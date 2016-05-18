
/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples. 
*       Copyright 1993 - 2000 Microsoft Corporation.
*       All rights reserved. 
*       This source code is only intended as a supplement to 
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the 
*       Microsoft samples programs.
\******************************************************************************/

/******************************************************************************\
*
*                             COMMON.H
*
\******************************************************************************/


/******************************************************************************\
*                         SYMBOLIC CONSTANTS
\******************************************************************************/

#define ARC             0x00000001
#define ELLIPSE         0x00000002
#define LINETO          0x00000004
#define PIE             0x00000008
#define PLG_BLT         0x00000010
#define POLYBEZIER      0x00000020
#define POLYGON         0x00000040
#define POLYLINE        0x00000080
#define POLYPOLYGON     0x00000100
#define RECTANGLE       0x00000200
#define ROUNDRECT       0x00000400
#define STRETCH_BLT     0x00000800

#define ALLGRAPHICS     0x00000fff

#define ENUMFONTS       0x40000000
#define DRAWAXIS        0x80000000

#define MAX_GRAPHICS    12

#define BUFSIZE         256

//ERR_MOD_NAME
#define MAIN_MENU_NAME            "MENU"
#define MAIN_CLASS_NAME           "PRINTER"
#define MAIN_WND_TITLE	          IDS_MAINWNDTITLE
#define MAIN_CLASS_STYLE          CS_HREDRAW | CS_VREDRAW
#define MAIN_WND_STYLE            WS_OVERLAPPEDWINDOW



/******************************************************************************\
*                            FUNCTION PROTOTYPES
\******************************************************************************/

#define ErrMsgBox(txt,title) MessageBox (ghwndMain, (LPCTSTR) txt,   \
                                         (LPCTSTR) title,            \
                                         MB_OK | MB_ICONEXCLAMATION);



/******************************************************************************\
*                            FUNCTION PROTOTYPES
\******************************************************************************/

BOOL Paint (HDC, LPRECT);

LRESULT CALLBACK DevCapsXDlgProc           (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK EnumPrintersDlgProc       (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK GetDeviceCapsDlgProc      (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK GetPrinterDriverDlgProc   (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK EnumPrinterDriversDlgProc (HWND, UINT, WPARAM, LPARAM);

LPTSTR   GetStringRes (int id);
LPTSTR   GetStringRes2 (int id);
