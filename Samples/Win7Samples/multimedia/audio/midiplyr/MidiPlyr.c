// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*****************************************************************************
*
* MIDIPlyr.C
*
* Initialization code for the polymessage MIDI playback app.
*
*****************************************************************************/

#pragma warning(disable:4756)

#define _INC_SHELLAPI
#include <windows.h>
#undef _INC_SHELLAPI

#include <shellapi.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <commdlg.h>
#include <commctrl.h>
#include <ctype.h>

#include "debug.h"

#include "MIDIPlyr.H"

PUBLIC  char BCODE      gszMWndClass[]      = "MIDIPlyrMWnd";
PUBLIC  char BCODE      gszTWndClass[]      = "MIDIPlyrTWnd";
PUBLIC  HINSTANCE       ghInst              = NULL;
PUBLIC  PSEQ            gpSeq               = NULL;
PUBLIC  char            gszUntitled[80]     = "";
PUBLIC  char            gszAppLongName[80]  = "";
PUBLIC  char            gszAppTitleMask[80] = "";
PUBLIC  char            grgszTimeFormats[N_TIME_FORMATS][CB_TIME_FORMATS] = {0};
PUBLIC  RECT            grcTWnd             = {0, 0, 0, 0};
PUBLIC  int             gnTimeFormat        = 0;

PRIVATE BOOL FNLOCAL InitApp(HINSTANCE hInst);
PRIVATE BOOL FNLOCAL InitInstance(HINSTANCE hInst, int nCmdShow);
PRIVATE VOID FNLOCAL TerminateInstance(VOID);

/*****************************************************************************
*
* WinMain
*
* Called by C startup code.
*
* HANDLE hInst              - Instance handle of this instance
* HANDLE hPrevInst          - Instance handle of previous instance or
*                             NULL if we are the first instance
* LPSTR lpstrCmdLine        - Any command line arguments
* int nCmdShow              - Code for ShowWindow which tells us what state
*                             to initially show the main application window.
*
* Initialize application if first instance.
* Initialize instance.
* Stay in main message processing loop until exit.
*
*****************************************************************************/
int PASCAL WinMain(
    HINSTANCE                  hInst,
    HINSTANCE                  hPrevInst,
    LPSTR                   lpstrCmdLine,
    int                     nCmdShow)
{
    MSG                     msg;

    if (hPrevInst == NULL)
        if (!InitApp(hInst))
            return 0;

    if (!InitInstance(hInst, nCmdShow))
    {
        TerminateInstance();
        return 0;
    }

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    TerminateInstance();

    return (int) msg.wParam;
}

/*****************************************************************************
*
* InitApp
*
* Called for one-time initialization if we are the first app instance.
*
* HANDLE hInst              - Instance handle of this instance
*
* Returns TRUE on success.
*
* Register the window class for the main window and the time window.
*
*****************************************************************************/
BOOL FNLOCAL InitApp(
    HINSTANCE               hInst)
{
    WNDCLASS                wc;

    InitCommonControls();

    /* Don't specify CS_HREDRAW or CS_VREDRAW if you're going to use the
    ** commctrl status or toolbar -- invalidate the (remaining) client
    ** area yourself if you want this behavior. This will allow the child
    ** control redraws to be much more efficient.
    */
    wc.style =          0;
    wc.lpfnWndProc =    MWnd_WndProc;
    wc.cbClsExtra =     0;
    wc.cbWndExtra =     0;
    wc.hInstance =      hInst;
    wc.hIcon =          LoadIcon(hInst, MAKEINTRESOURCE(ID_ICON));
    wc.hCursor =        LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground =  (HBRUSH)(COLOR_BTNFACE+1);
    wc.lpszMenuName =   MAKEINTRESOURCE(ID_MENU);
    wc.lpszClassName =  (LPCSTR)gszMWndClass;

    RegisterClass(&wc);

    wc.style =          CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc =    TWnd_WndProc;
    wc.cbClsExtra =     0;
    wc.cbWndExtra =     0;
    wc.hInstance =      hInst;
    wc.hIcon =          LoadIcon(hInst, MAKEINTRESOURCE(ID_ICON));
    wc.hCursor =        LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground =  (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName =   NULL;
    wc.lpszClassName =  (LPCSTR)gszTWndClass;

    RegisterClass(&wc);

    return TRUE;
}

/*****************************************************************************
*
* InitInstance
*
* Called once per instance of application.
*
* HANDLE hInst              - Instance handle of this instance
* int nCmdShow              - Code for ShowWindow which tells us what state
*                             to initially show the main application window.
* Returns TRUE on success.
*
* Initialize debug library.
* Save the instance handle.
* Load global resource strings.
* Allocate and initialize the global sequencer structure.
* Create the main window.
* Add time formats to the options menu.
* Show the main window.
*
*****************************************************************************/
BOOL FNLOCAL InitInstance(
    HINSTANCE               hInst,
    int                     nCmdShow)
{
    HWND                    hWnd;
    int                     idx;

    DbgInitialize(TRUE);

    ghInst = hInst;

    LoadString(hInst, IDS_APPTITLEMASK, gszAppTitleMask, sizeof(gszAppTitleMask));
    LoadString(hInst, IDS_APPNAME,      gszAppLongName,  sizeof(gszAppLongName));
    LoadString(hInst, IDS_UNTITLED,     gszUntitled,     sizeof(gszUntitled));

    for (idx = 0; idx < N_TIME_FORMATS; idx++)
    {
        *grgszTimeFormats[idx] = '\0';
        LoadString(hInst,
                   IDS_TF_FIRST+idx,
                   grgszTimeFormats[idx],
                   sizeof(grgszTimeFormats[idx]));
    }

    if ((gpSeq = (PSEQ)LocalAlloc(LPTR, sizeof(SEQ))) == NULL)
        return FALSE;

    gpSeq->cBuffer  = C_MIDI_BUFFERS;
    gpSeq->cbBuffer = CB_MIDI_BUFFERS;

    if (seqAllocBuffers(gpSeq) != MMSYSERR_NOERROR)
        return FALSE;

    hWnd = CreateWindow(
        gszMWndClass,
        NULL,
        WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
        CW_USEDEFAULT,CW_USEDEFAULT,
        CW_USEDEFAULT,CW_USEDEFAULT,
        HWND_DESKTOP,
        NULL,
        hInst,
        NULL);

    if (hWnd == (HWND)NULL)
        return FALSE;

    gpSeq->hWnd = hWnd;

    ShowWindow(hWnd, nCmdShow);

    return TRUE;
}

/*****************************************************************************
*
* TerminateInstance
*
* Release any resources for the current instance
*
*****************************************************************************/
VOID FNLOCAL TerminateInstance(
    VOID)
{
    if (gpSeq != NULL)
    {
        seqFreeBuffers(gpSeq);
        LocalFree((HLOCAL)gpSeq);
        gpSeq = NULL;
    }
}
