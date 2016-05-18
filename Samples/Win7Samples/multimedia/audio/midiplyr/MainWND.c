// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*****************************************************************************
*
* MainWnd.C
*
* Message handlers and UI for the main window and associated controls
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
#include <strsafe.h>

#include "debug.h"

#include "MIDIPlyr.H"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif


#define BITMAP_COUNT    6           /* Number of buttons in bitmap */

PRIVATE HWND            ghWndToolbar                    = NULL;
PRIVATE HWND            ghWndStatus                     = NULL;
PRIVATE HWND            ghWndTime                       = NULL;
PRIVATE char            gszAppTitle[80]                 = "";
PRIVATE int             gnSB_TFPaneSize                 = 0;
PRIVATE char            gszOpenName[MAX_FILEPATH]       = "";
PRIVATE char            gszOpenTitle[MAX_FILEPATH]      = "";
PRIVATE char BCODE      gszFilter[]                      =
    "MIDI File (*.MID;*.RMI)\0*.MID;*.RMI\0"
    "All Files (*.*)\0*.*\0";

PRIVATE char BCODE      gszDefExtension[]               = "MID";
PRIVATE BOOL            gbAutoPlay                      = TRUE;
PRIVATE UINT            guDevice                        = 0;

PRIVATE TBBUTTON gatbButton[] =
{
    {0, -1,             TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0,  0, -1},
    {0, IDM_OPEN,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,  0, -1},
    {0, -1,             TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0,  0, -1},
    {2, IDM_PLAY,       0,               TBSTYLE_BUTTON, 0, 0,  0, -1},
    {3, IDM_STOP,       0,               TBSTYLE_BUTTON, 0, 0,  0, -1},
    {4, IDM_PAUSE,      0,               TBSTYLE_BUTTON, 0, 0,  0, -1},
};

#define BUTTON_COUNT (sizeof(gatbButton)/sizeof(gatbButton[0]))

PRIVATE VOID FNLOCAL InitToolbar(HWND hWnd);
PRIVATE VOID FNLOCAL InitStatusBar(HWND hWnd);
PRIVATE VOID FNLOCAL ResizeStatusBar(HWND hWnd);
PRIVATE VOID FNLOCAL SyncUI(HWND hWnd);
PRIVATE VOID FNLOCAL SetOneAction(HWND hWnd, int nMenuID, BOOL fEnable);
PRIVATE VOID FNLOCAL AttemptFileOpen(HWND hWnd);
PRIVATE BOOL FNLOCAL PrerollAndWait(HWND hWnd);

PRIVATE BOOL FNLOCAL MWnd_OnCreate(HWND hWnd, CREATESTRUCT FAR* lpCreateStruct);
PRIVATE VOID FNLOCAL MWnd_OnGetMinMaxInfo(HWND hWnd, MINMAXINFO FAR* lpMinMaxInfo);
PRIVATE VOID FNLOCAL MWnd_OnSize(HWND hWnd, UINT state, int cx, int cy);
PRIVATE VOID FNLOCAL MWnd_OnPaint(HWND hWnd);
PRIVATE VOID FNLOCAL MWnd_OnDropFiles(HWND hWnd, HDROP hDrop);
PRIVATE VOID FNLOCAL MWnd_OnFileOpen(HWND hWnd);
PRIVATE VOID FNLOCAL MWnd_OnCommandToggleChild(HWND hWnd, UINT id);
PRIVATE VOID FNLOCAL MWnd_OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT codeNotify);
PRIVATE VOID FNLOCAL MWnd_OnDestroy(HWND hWnd);

/*****************************************************************************
*
* InitToolbar
*
* Called to create the toolbar
*
* HWND hWnd                 - Application window which toolbar is a child of
*
* Create and show the toolbar window.
*
*****************************************************************************/
PRIVATE VOID FNLOCAL InitToolbar(
    HWND                    hWnd)
{
    ghWndToolbar = CreateToolbarEx(hWnd,
                                   WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
                                   IDC_TOOLBAR,
                                   BITMAP_COUNT,
                                   ghInst,
                                   IDB_TOOLBAR,
                                   gatbButton,
                                   BUTTON_COUNT,
                                   0,  0,
                                   0,  0,
                                   sizeof(TBBUTTON));

    if (ghWndToolbar)
        ShowWindow(ghWndToolbar, SW_RESTORE);
}

/*****************************************************************************
*
* InitStatusBar
*
* Called to create the status bar
*
* HWND hWnd                 - Application window which status bar is a child of
*
* Create and show the status window.
*
*****************************************************************************/
PRIVATE VOID FNLOCAL InitStatusBar(
    HWND                    hWnd)
{
    HWND                    hWndDesktop;
    HFONT                   hFontStat;
    HDC                     hDC;
    UINT                    idx;
    SIZE                    size;

    ghWndStatus = CreateStatusWindow(WS_CHILD|SBARS_SIZEGRIP,
                                     "",
                                     hWnd,
                                     IDC_STATBAR);

    if (ghWndStatus)
    {
        hWndDesktop = GetDesktopWindow();
        hFontStat = (HFONT)SendMessage(ghWndStatus, WM_GETFONT, 0, 0L);
        hDC = GetDC(hWndDesktop);

        if (hFontStat != (HFONT)NULL && hDC != (HDC)NULL)
        {
            hFontStat = (HFONT)SelectObject(hDC, hFontStat);

            gnSB_TFPaneSize = 0;
            for (idx = 0; idx < N_TIME_FORMATS; idx++)
            {
                GetTextExtentPoint(hDC,
                                   grgszTimeFormats[idx],
                                   lstrlen(grgszTimeFormats[idx]),
                                   &size);

                gnSB_TFPaneSize = max(gnSB_TFPaneSize, size.cx);
            }

            SelectObject(hDC, hFontStat);

            gnSB_TFPaneSize *= 2;
        }

        if (hDC != (HDC)NULL)
            ReleaseDC(hWndDesktop, hDC);

        ResizeStatusBar(hWnd);
        
        FORWARD_WM_COMMAND(hWnd, gnTimeFormat, 0, 0, SendMessage);
        ShowWindow(ghWndStatus, SW_RESTORE);
    }
}

/*****************************************************************************
*
* ResizeStatusBar
*
* Force the status bar to resize to fit in the main window
*
* HWND hWnd                 - Application window which status bar is a child of
*
* Figure out the pane sizes and send a message to the status bar to set them.
*
*****************************************************************************/
PRIVATE VOID FNLOCAL ResizeStatusBar(
    HWND                    hWnd)
{
    RECT                    rc;
    int                     rnPaneEdge[SB_N_PANES];

    GetClientRect(hWnd, &rc);

    /* SB_SETPARTS expects:
    **  wParam == Number of panes in status bar.
    **  lParam == Pointer to an array of int's indicating the right-hand
    **            coordinate of each pane.
    */
    rnPaneEdge[SB_PANE_STATE] = rc.right - gnSB_TFPaneSize;
    rnPaneEdge[SB_PANE_TFMT]  = -1;

    SendMessage(ghWndStatus,
                SB_SETPARTS,
                SB_N_PANES,
                (DWORD)(INT_PTR)(rnPaneEdge));
}

/*****************************************************************************
*
* SyncUI
*
* Bring all UI elements into sync with the state of the sequencer
*
* HWND hWnd                 - Application main window 
*
* Build a flag word of the actions which are allowed from the current state.
* Set the menu items and toolbar buttons for each action appropriately.
* Show the current state as a string in the status bar.
* Depress the pause button if the sequencer is paused.
* Cause the time window to update.
*
*****************************************************************************/
#define SUI_F_CANPLAY       0x0001
#define SUI_F_CANPAUSE      0x0002
#define SUI_F_CANSTOP       0x0004
#define SUI_F_CANSELDEVICE  0x0008

PRIVATE VOID FNLOCAL SyncUI(
    HWND                    hWnd)
{
    WORD                    wActionFlags;
    UINT                    uState;
    char                    szState[40];
    BOOL                    fPress;

    wActionFlags = 0;
    uState = SEQ_S_NOFILE;
    
    if (gpSeq != NULL)
    {
        uState = gpSeq->uState;
        switch (uState)
        {
        case SEQ_S_NOFILE:
            wActionFlags = SUI_F_CANSELDEVICE;
            break;

        case SEQ_S_OPENED:
        case SEQ_S_PREROLLED:
            wActionFlags = SUI_F_CANPLAY|SUI_F_CANSELDEVICE;
            break;

        case SEQ_S_PAUSED:
        case SEQ_S_PLAYING:
            wActionFlags = SUI_F_CANPAUSE|SUI_F_CANSTOP;
            break;


        case SEQ_S_PREROLLING:
        case SEQ_S_STOPPING:
//            assert(0);
            wActionFlags = 0;
            break;
        }
    }
    
    fPress = (gpSeq->uState == SEQ_S_PAUSED);
    SendMessage(ghWndToolbar,
                TB_PRESSBUTTON,
                IDM_PAUSE,
                fPress);

    SetOneAction(hWnd, IDM_PLAY,   wActionFlags & SUI_F_CANPLAY);
    SetOneAction(hWnd, IDM_PAUSE,  wActionFlags & SUI_F_CANPAUSE);
    SetOneAction(hWnd, IDM_STOP,   wActionFlags & SUI_F_CANSTOP);

    EnableMenuItem(GetMenu(hWnd),
                   POS_PLAYTHRU,
                   MF_BYPOSITION|((wActionFlags & SUI_F_CANSELDEVICE) ? MF_ENABLED : MF_DISABLED));

    DrawMenuBar(hWnd);

    szState[0] = '\0';
    LoadString(ghInst, IDS_STATES + uState, szState, sizeof(szState));
    SendMessage(ghWndStatus, SB_SETTEXT, SB_PANE_STATE, (LPARAM)(LPSTR)szState);

    InvalidateRect(ghWndTime, NULL, TRUE);
}

/*****************************************************************************
*
* SetOneAction
*
* Update the state of one action in both the toolbar and action menu
*
* HWND hWnd                 - Application main window
* int nMenuID               - Menu ID of action
* BOOL fEnable              - Enable or disable this action
*
*****************************************************************************/
PRIVATE VOID FNLOCAL SetOneAction(
    HWND                hWnd,
    int                 nMenuID,
    BOOL                fEnable)
{
    EnableMenuItem(GetMenu(hWnd),
                   nMenuID,
                   MF_BYCOMMAND|(fEnable ? MF_ENABLED : MF_DISABLED));

    SendMessage(ghWndToolbar,
                TB_ENABLEBUTTON,
                nMenuID,
                (DWORD)fEnable);
}

/*****************************************************************************
*
* AttemptFileOpen
*
* Try to open the given file in the sequencer.
*
* HWND hWnd                 - Application main window
*
* Stop and close the current file.
* Open the new file.
* Preroll the new file.
* Set the title test for the main window.
* Call SyncUI to update available actions.
*
*****************************************************************************/
PRIVATE VOID FNLOCAL AttemptFileOpen(
    HWND                    hWnd)
{
    MMRESULT                mmrc;
    PSTR                    pStrFile    = gszUntitled;
    
    /* Stop, close, etc. if we're still playing
    */
    DPF(1, "AttemptFileOpen: Calling seqStop(); state = %u", gpSeq->uState);
    
    mmrc = seqStop(gpSeq);
	if (mmrc != MMSYSERR_NOERROR)
	{
		Error(hWnd, IDS_STOPFAILED, mmrc);
		return;
	}

    DPF(1, "Calling seqCloseFile(); state = %u", gpSeq->uState);
    seqCloseFile(gpSeq);

    DPF(1, "Calling seqOpenFile(); state = %u", gpSeq->uState);
    /* Open new file
    */

    gpSeq->pstrFile = gszOpenName;
    mmrc = seqOpenFile(gpSeq);
    if (mmrc != MMSYSERR_NOERROR)
    {
        Error(hWnd, IDS_OPENFAILED, mmrc);
        return;
    }

    pStrFile = gszOpenTitle;

    StringCchPrintf(gszAppTitle, ARRAY_SIZE(gszAppTitle), gszAppTitleMask, (LPSTR)pStrFile);
    SetWindowText(hWnd, gszAppTitle);

    SyncUI(hWnd);
}

/*****************************************************************************
*
* PrerollAndWait
*
* Prerolls the sequencer using the current device ID and file.
*
* HWND hWnd                 - Current window
*
* Just call preroll and loop processing messages until done.
*
*****************************************************************************/
PRIVATE BOOL FNLOCAL PrerollAndWait(
    HWND                    hWnd)
{
    PREROLL                 preroll;
    MMRESULT                mmrc;
    
    preroll.tkBase = 0;
    preroll.tkEnd  = gpSeq->tkLength;

    gpSeq->uDeviceID = guDevice;

    if (MMSYSERR_NOERROR != (mmrc = seqPreroll(gpSeq, &preroll)))
	{
        Error(hWnd, IDS_PREROLLFAILED, mmrc);
        return FALSE;
	}

	return TRUE;
}

/*****************************************************************************
*
* MWnd_OnCreate
*
* Handle WM_CREATE message to main application window.
*
* HWND hWnd                 - Window handle
* CREATESTRUCT FAR* lpCreateStruct
*                           - Pointer to creation parameters for the window.
*
* Returns TRUE on success. Returning FALSE will cause the window to be
* destroyed and the application will exit.
*
* Set the default time format.
* Create the tool and status bars.
* Create the time window as a child of the main app window and show it.
* Set the main window's title to show no document ('Untitled').
* Accept drag/drop files.
* Call SyncUI to update the enable status of the toolbar and menu items.
*
*****************************************************************************/
PRIVATE BOOL FNLOCAL MWnd_OnCreate(
    HWND                    hWnd,
    CREATESTRUCT FAR*       lpCreateStruct)
{
    HMENU                   hMenu;
    HMENU                   hMenuOptions;
    HMENU                   hMenuPlayThru;
    UINT                    cDevs;
    UINT                    idx;
    RECT                    rc;
    MIDIOUTCAPS             moutCaps;

    gnTimeFormat = IDS_TF_FIRST;

    InitToolbar(hWnd);
    InitStatusBar(hWnd);
    
    hMenu = GetMenu(hWnd);
    hMenuOptions = GetSubMenu(hMenu, POS_OPTIONS);
    hMenuPlayThru = GetSubMenu(hMenu, POS_PLAYTHRU);

    AppendMenu(hMenuOptions, MF_SEPARATOR, 0, NULL);
    
    for (idx = 0; idx < N_TIME_FORMATS; idx++)
    {
        AppendMenu(hMenuOptions,
                   MF_ENABLED|MF_STRING,
                   IDS_TF_FIRST + idx,
                   grgszTimeFormats[idx]);
    }

    cDevs = midiOutGetNumDevs();
    if (cDevs)
        AppendMenu(hMenuPlayThru, MF_SEPARATOR, 0, NULL);
    
    for (idx = 0; idx < cDevs; idx++)
    {
        if (midiOutGetDevCaps(idx, &moutCaps, sizeof(moutCaps)) == MMSYSERR_NOERROR)
        {
            AppendMenu(hMenuPlayThru,
                       MF_ENABLED|MF_STRING,
                       IDM_DEVICES + idx,
                       moutCaps.szPname);
        }
    }
    
    CheckMenuItem(hMenu, IDM_TOOLBAR, MF_BYCOMMAND|MF_CHECKED);
    CheckMenuItem(hMenu, IDM_STATUS,  MF_BYCOMMAND|MF_CHECKED);
    CheckMenuItem(hMenu, IDM_AUTOPLAY,MF_BYCOMMAND|MF_CHECKED);
    CheckMenuItem(hMenu, gnTimeFormat,MF_BYCOMMAND|MF_CHECKED);
    CheckMenuItem(hMenu, IDM_DEVICES,  MF_BYCOMMAND|MF_CHECKED);  

    GetClientRect(hWnd, &rc);

    ghWndTime = CreateWindow(
        gszTWndClass,
        NULL,
        WS_CHILD,
        rc.left, rc.top,
        rc.right-rc.left, rc.bottom-rc.top,
        hWnd,
        NULL,
        lpCreateStruct->hInstance,
        NULL);

    ShowWindow(ghWndTime, SW_RESTORE);

    StringCchPrintf(gszAppTitle, ARRAY_SIZE(gszAppTitle), gszAppTitleMask, (LPSTR)gszUntitled);
    SetWindowText(hWnd, gszAppTitle);

    DragAcceptFiles(hWnd, TRUE);

    SyncUI(hWnd);

    return TRUE;
}
                      
/*****************************************************************************
*
* MWnd_OnGetMinMaxSize
*
* Handle WM_GETMINMAXSIZE message to main application window.
*
* HWND hWnd                 - Window handle
* MINMAXINFO FAR* lpMinMaxInfo
*                           - Pointer to min/max tracking information
*
* This message is sent to a window before resize tracking begins. The
* lpMinMaxInfo structure contains the minimum and maximum x and y values
* the window can be resized to.
*
* We don't allow resizing small enough to cause the status bar and toolbar
* to overlap so they don't try to draw over each other. 
*
*****************************************************************************/
PRIVATE VOID FNLOCAL MWnd_OnGetMinMaxInfo(
    HWND                    hWnd,
    MINMAXINFO FAR*         lpMinMaxInfo)
{
    RECT                    rc;

    GetWindowRect(hWnd, &rc);

    /* Only go small enough that our client area after children is zero.
    */
    lpMinMaxInfo->ptMinTrackSize.y =
        (rc.bottom - rc.top) -
        (grcTWnd.bottom - grcTWnd.top);                                      
}

/*****************************************************************************
*
* MWnd_OnSize
*
* Handle WM_SIZE message to main application window.
*
* HWND hWnd                 - Window handle
* UINT state                - Some SIZE_xxx code indicating what type of
*                             size operation this is.
* int  cx, cy               - New x and y size of the window's client area.
*
* Get the new client area.
* Adjust the client area for the toolbar and status bar if they exist.
* Make sure the client area isn't a negative height and adjust if it is.
* Resize the time window to fit in our new client area.
* Forward the WM_SIZE to the time window so it can resize its font.
*
*****************************************************************************/
PRIVATE VOID FNLOCAL MWnd_OnSize(
    HWND                    hWnd,
    UINT                    state,
    int                     cx,
    int                     cy)
{
    RECT                    rc;
    RECT                    rcClient;

    GetClientRect(hWnd, &rcClient);
    if (ghWndToolbar != NULL)
    {
        /* Cause the toolbar to be aware of the size change
        */
        FORWARD_WM_SIZE(ghWndToolbar, SIZE_RESTORED, 0, 0, SendMessage);
        
        GetWindowRect(ghWndToolbar, &rc);
        rcClient.top += (rc.bottom - rc.top);
    }

    if (ghWndStatus != NULL)
    {
        ResizeStatusBar(hWnd);
        
        /* Cause the status bar to be aware of the size change
        */
        FORWARD_WM_SIZE(ghWndStatus, SIZE_RESTORED, 0, 0, SendMessage);
        
        GetWindowRect(ghWndStatus, &rc);
        rcClient.bottom -= (rc.bottom - rc.top);
    }

    /* Do we need to resize entire window so the tool/status bars
    ** don't overlap? (The only case where this can happen is
    ** on creation of one of the two -- we set minimum tracking so
    ** a user can't manually resize the window to cause this
    ** condition).
    */
    if (rcClient.bottom < rcClient.top)
    {
        GetWindowRect(hWnd, &rc);
        SetWindowPos(hWnd,
                     (HWND)NULL,
                     0, 0,
                     rc.right - rc.left + 1,
                     rc.bottom - rc.top + 1 - rcClient.top - rcClient.bottom,
                     SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
    }

    SetWindowPos(ghWndTime,
                 (HWND)NULL,
                 rcClient.left,
                 rcClient.top,
                 rcClient.right - rcClient.left,
                 rcClient.bottom - rcClient.top,
                 SWP_NOACTIVATE|SWP_NOZORDER);

    FORWARD_WM_SIZE(ghWndTime, SIZE_RESTORED, 0, 0, SendMessage);
}

/*****************************************************************************
*
* MWnd_OnPaint
*
* Handle WM_PAINT message to main application window.
*
* HWND hWnd                 - Window handle
*
* Just do a BeginPaint/EndPaint pair so USER will mark the area
*   as valid. All the real work of painting the time is done
*   by the WM_PAINT handler for the time window.
*
*****************************************************************************/
PRIVATE VOID FNLOCAL MWnd_OnPaint(
    HWND                    hWnd)
{
    PAINTSTRUCT             ps;
    HDC                     hDC;

    hDC = BeginPaint(hWnd, &ps);
    EndPaint(hWnd, &ps);
}

/*****************************************************************************
*
* MWnd_OnDropFiles
*
* Handle WM_DROPFILES message to main application window.
*
* HWND hWnd                 - Window handle
* HDROP hDrop               - Handle to dropped file information
*
* Get the 0th filename and free the drop handle.
* Extract the file title from the full pathname.
* Open the file.
* If we opened successfully, start playback by forwarding a WM_COMMAND
*   of IDM_PLAY to the main window.
*
*****************************************************************************/
PRIVATE VOID FNLOCAL MWnd_OnDropFiles(
    HWND                    hWnd,
    HDROP                   hDrop)
{
    PSTR                    pStr;
    
    /* For multiple selections, we only accept the first file
    */
    DragQueryFile(hDrop, 0, gszOpenName, sizeof(gszOpenName));
    DragFinish(hDrop);

    /* We don't get OpenTitle like we do from GetOpenFileName; need to
    ** figure this out for ourselves
    */
    pStr = gszOpenName + lstrlen(gszOpenName) - 1;

    while (pStr >= gszOpenName && *pStr != '/' && *pStr != '\\' && *pStr != ':')
        pStr--;

    pStr++;
    StringCchCopyA(gszOpenTitle, MAX_FILEPATH, pStr);

    AttemptFileOpen(hWnd);

    if (gbAutoPlay && gpSeq->uState == SEQ_S_OPENED)
        FORWARD_WM_COMMAND(hWnd, IDM_PLAY, (HWND)NULL, 0, SendMessage);
}


/*****************************************************************************
*
* MWnd_OnFileOpen
*
* Handle WM_COMMAND/IDM_OPEN message to main application window.
*
* HWND hWnd                 - Window handle
*
* Fill in the OPENFILENAME struct and call GetOpenFileName.
* If not canceled, try to open the file.
*
*****************************************************************************/
PRIVATE VOID FNLOCAL MWnd_OnFileOpen(
    HWND                    hWnd)
{
    OPENFILENAME            ofn;

    *gszOpenName = '\0';
    
	ofn.lStructSize			= sizeof(OPENFILENAME);
	ofn.hwndOwner			= hWnd;
	ofn.lpstrFilter			= gszFilter;
	ofn.lpstrCustomFilter	= (LPSTR)NULL;
	ofn.nMaxCustFilter		= 0L;
	ofn.nFilterIndex		= 1L;
	ofn.lpstrFile			= gszOpenName;
	ofn.nMaxFile			= MAX_FILEPATH;
	ofn.lpstrFileTitle		= gszOpenTitle;
	ofn.nMaxFileTitle		= MAX_FILEPATH;
	ofn.lpstrTitle			= (LPSTR)NULL;
	ofn.lpstrInitialDir		= (LPSTR)NULL;
	ofn.Flags				= OFN_HIDEREADONLY|OFN_FILEMUSTEXIST;
	ofn.nFileOffset			= 0;
	ofn.nFileExtension		= 0;
	ofn.lpstrDefExt			= gszDefExtension;

    if (!GetOpenFileName(&ofn))
        return;

    AttemptFileOpen(hWnd);
}

/*****************************************************************************
*
* MWnd_OnCommandToggleChild
*
* Handle WM_COMMAND message of toggle tool or status bar to main application
* window.
*
* HWND hWnd                 - Window handle
* UINT id                   - Control id of menu selection; either
*                             IDM_TOOLBAR or IDM_STATUS
*
* Get the current menu item check state.
* Destroy or create the child as needed.
* Send a WM_SIZE to the main window so client area will be recalculated.
* Toggle the menu item check state.
*
*****************************************************************************/
PRIVATE VOID FNLOCAL MWnd_OnCommandToggleChild(
    HWND                    hWnd,                                      
    UINT                    id)
{
    HMENU                   hMenu;
    UINT                    uState;
    HWND*                   phWnd;
    
    phWnd = (id == IDM_TOOLBAR) ? &ghWndToolbar : &ghWndStatus;

    hMenu = GetMenu(hWnd);
    uState = GetMenuState(hMenu, id, MF_BYCOMMAND);
    if (uState & MF_CHECKED)
    {
        DestroyWindow(*phWnd);
        *phWnd = NULL;
    }
    else
    {
        if (id == IDM_TOOLBAR)
            InitToolbar(hWnd);
        else
            InitStatusBar(hWnd);
    }

    SendMessage(hWnd, WM_SIZE, 0, 0L);

    uState ^= MF_CHECKED;
    uState &= MF_CHECKED;
    CheckMenuItem(hMenu, id, MF_BYCOMMAND|uState);

    SyncUI(hWnd);
}
                                          
                            
/*****************************************************************************
*
* MWnd_OnCommand
*
* Handle WM_COMMAND message to main application window.
*
* HWND hWnd                 - Window handle
* int id                    - id of control or menu causing WM_COMMAND
* HWND hwndCtl              - Window handle of child control, if any
* UINT codeNotify           - Notification code if this message is from a
*                             control.
*
* For a press of the toolbar buttons or their menu equivalents, just load
* a resource string and display it in the status bar.
*
* For an exit request, send ourselves a WM_CLOSE message.
*
*****************************************************************************/
PRIVATE VOID FNLOCAL MWnd_OnCommand(
    HWND                    hWnd,
    int                     id,
    HWND                    hWndCtl,
    UINT                    codeNotify)
{
    HMENU                   hMenu;
    int                     nIdxFormat;
    LPSTR                   lpstr;
    
    if (id >= IDS_TF_FIRST && id <= IDS_TF_LAST)
    {
        if (NULL != ghWndStatus)
        {
            nIdxFormat = id - IDS_TF_FIRST;

            lpstr = (LPSTR)(grgszTimeFormats[nIdxFormat]);
            
            SendMessage(ghWndStatus,
                        SB_SETTEXT,
                        SB_PANE_TFMT,
                        (LPARAM)lpstr);

        }

        hMenu = GetMenu(hWnd);

        CheckMenuItem(hMenu, gnTimeFormat, MF_UNCHECKED|MF_BYCOMMAND);
        CheckMenuItem(hMenu, id, MF_CHECKED|MF_BYCOMMAND);

        gnTimeFormat = id;
        
        /* Force time window to update font and repaint entire time string
         */
        
        if(ghWndTime)	// for when WM_COMMAND is called before WM_CREATE
        	FORWARD_WM_SIZE(ghWndTime, SIZE_RESTORED, 0, 0, SendMessage);
    }
    else if (id >= IDM_DEVMIN && id <= IDM_DEVMAX)
    {
        hMenu = GetMenu(hWnd);
                
        CheckMenuItem(hMenu, guDevice + IDM_DEVICES, MF_UNCHECKED|MF_BYCOMMAND);
        guDevice = id - IDM_DEVICES;
        CheckMenuItem(hMenu, guDevice + IDM_DEVICES, MF_CHECKED|MF_BYCOMMAND);
    }
    else switch(id)
    {
        case IDM_OPEN:
            MWnd_OnFileOpen(hWnd);
            break;
        
        case IDM_TOOLBAR:
        case IDM_STATUS:
            MWnd_OnCommandToggleChild(hWnd, id);
            break;

        case IDM_AUTOPLAY:
            gbAutoPlay = !gbAutoPlay;
            CheckMenuItem(GetMenu(hWnd),
                          IDM_AUTOPLAY,
                          MF_BYCOMMAND|(gbAutoPlay ? MF_CHECKED : MF_UNCHECKED));
            break;

        case IDM_PLAY:
            FORWARD_WM_COMMAND(ghWndTime, IDM_PLAY, 0, 0, SendMessage);
        
			if (gpSeq->uState != SEQ_S_OPENED)
				DPF(1, "IDM_PLAY: State %u", gpSeq->uState);
				        	    
            if (PrerollAndWait(hWnd))                   
            	seqStart(gpSeq);

            SyncUI(hWnd);
            break;

        case IDM_STOP:
            FORWARD_WM_COMMAND(ghWndTime, IDM_STOP, 0, 0, SendMessage);

            seqStop(gpSeq);
            SyncUI(hWnd);
            break;

        case IDM_PAUSE:
            if (gpSeq->uState == SEQ_S_PAUSED)
            {
                seqRestart(gpSeq);
            }
            else
            {
                seqPause(gpSeq);
            }

            SyncUI(hWnd);
            break;

        case IDM_SYNCUI:
            SyncUI(hWnd);
            break;
            
        case IDM_EXIT:
            SendMessage(hWnd, WM_CLOSE, 0, 0L);
            break;
    }
}
                                   
/*****************************************************************************
*
* MWnd_OnDestroy
*
* Handle WM_DESTROY message to main application window.
*
* HWND hWnd                 - Window handle
*
* Our main application window has been closed. PostQuitMessage so the main
* message loop will exit and the app can terminate.
*
*****************************************************************************/
PRIVATE VOID FNLOCAL MWnd_OnDestroy(
    HWND                    hWnd)
{
    seqStop(gpSeq);
    PostQuitMessage(0);
}

/*****************************************************************************
*
* MWnd_WndProc
*
* Window procedure for main application window.
*
* HWND hWnd                 - Window handle
* UINT msg                  - Message code
* WPARAM wParam             - Message specific parameter
* LPARAM lParam             - Message specific parameter
*
* Dispatch messages we care about to the appropriate handler, else just
* call DefWindowProc.
*
* Note this use of message cracker macros from windowsx.h. Using these
* macros will shield you from the differences between Win16 and Win32;
* if your app is cross-compilable, you should use these and save yourself
* some headaches!
*
*****************************************************************************/
LRESULT CALLBACK MWnd_WndProc(
    HWND                    hWnd,
    UINT                    msg,
    WPARAM                  wParam,
    LPARAM                  lParam)
{
    switch( msg )
    {
        HANDLE_MSG(hWnd, WM_CREATE,         MWnd_OnCreate);
        HANDLE_MSG(hWnd, WM_GETMINMAXINFO,  MWnd_OnGetMinMaxInfo);
        HANDLE_MSG(hWnd, WM_SIZE,           MWnd_OnSize);
        HANDLE_MSG(hWnd, WM_PAINT,          MWnd_OnPaint);
        HANDLE_MSG(hWnd, WM_DROPFILES,      MWnd_OnDropFiles);
        HANDLE_MSG(hWnd, WM_COMMAND,        MWnd_OnCommand);
        HANDLE_MSG(hWnd, WM_DESTROY,        MWnd_OnDestroy);

        case MMSG_DONE:
            FORWARD_WM_COMMAND(ghWndTime, IDM_STOP, 0, 0, SendMessage);
            
			if (gpSeq->uState != SEQ_S_OPENED)
				DPF(1, "MMSG_DONE and state %u", gpSeq->uState);

            SyncUI(hWnd);
            break;

        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
}
