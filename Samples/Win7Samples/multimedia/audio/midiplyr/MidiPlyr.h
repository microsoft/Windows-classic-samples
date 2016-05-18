// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*****************************************************************************
*
* MIDIPlyr.H
*
* Main include file for the polymessage MIDI playback app.
*
*****************************************************************************/

#ifndef _MIDIPLYR_
#define _MIDIPLYR_

#include "seq.h"

#define MAX_FILEPATH        256

/* Number and size of playback buffers to keep around
*/
#define C_MIDI_BUFFERS      4
#define CB_MIDI_BUFFERS     1024


/* Popup menu positions in main menu bar
*/
#define POS_FILE            0
#define POS_ACTIONS         1
#define POS_OPTIONS         2
#define POS_PLAYTHRU        3

/* Status bar pane indices
*/
#define SB_N_PANES          2
#define SB_PANE_STATE       0
#define SB_PANE_TFMT        1

/* Resource ID's
*/
#define ID_ICON             50
#define ID_MENU             51

#define IDM_EXIT            100
#define IDM_ABOUT           101
#define IDM_OPEN            102
#define IDM_PLAY            103
#define IDM_PAUSE           104
#define IDM_STOP            105
#define IDM_TOOLBAR         106
#define IDM_STATUS          107
#define IDM_AUTOPLAY        108

#define IDM_DEVMIN          129
#define IDM_MAPPER          129         /* MUST be IDM_DEVICES - 1 */
#define IDM_DEVICES         130         /* 129 thru 149 */
#define IDM_DEVMAX          149

#define IDM_SYNCUI          150         /* DEBUG */

#define IDB_TOOLBAR         200

#define IDC_TOOLBAR         300
#define IDC_STATBAR         301

#define IDS_APPTITLEMASK    1000
#define IDS_APPNAME         1001
#define IDS_UNTITLED        1002

/* ID's for these must be contiguous !!!
** Note that we also use these as IDM_ items in the Options menu
*/
#define IDS_TF_FIRST        1010
#define IDS_HMS             1010
#define IDS_TICKS           1011
#define IDS_TF_LAST         1011

/* ID's for sequencer state descriptions
** These must also be contigous and in the same order as the SEQ_S
** states in seq.h
*/
#define IDS_STATES          1020
#define IDS_NOFILE          (IDS_STATES + SEQ_S_NOFILE)
#define IDS_OPENED          (IDS_STATES + SEQ_S_OPENED)
#define IDS_PREROLLING      (IDS_STATES + SEQ_S_PREROLLING)
#define IDS_PREROLLED       (IDS_STATES + SEQ_S_PREROLLED)
#define IDS_PLAYING         (IDS_STATES + SEQ_S_PLAYING)
#define IDS_PAUSED          (IDS_STATES + SEQ_S_PAUSED)
#define IDS_STOPPING        (IDS_STATES + SEQ_S_STOPPING)

#define N_TIME_FORMATS      (IDS_TF_LAST - IDS_TF_FIRST + 1)
#define CB_TIME_FORMATS     40

#define IDS_OPENFAILED      1050
#define IDS_PREROLLFAILED   1051
#define IDS_TESTERR         1052
#define IDS_STOPFAILED		1053

/* Globals
*/
extern  HINSTANCE       ghInst;
extern  char BCODE      gszMWndClass[];
extern  char BCODE      gszTWndClass[];
extern  PSEQ            gpSeq;
extern  char            gszUntitled[80];
extern  char            gszAppLongName[80];
extern  char            gszAppTitleMask[80];
extern  char            grgszTimeFormats[N_TIME_FORMATS][CB_TIME_FORMATS];
extern  RECT            grcTWnd;
extern  int             gnTimeFormat;

/* MainWnd.C
*/
VOID FNLOCAL InitToolbar(
    HWND                    hWnd);

VOID FNLOCAL InitToolbar(
    HWND                    hWnd);

LRESULT CALLBACK MWnd_WndProc(
    HWND                    hWnd,
    UINT                    msg,
    WPARAM                  wParam,
    LPARAM                  lParam);

/* TimeWnd.C
*/
LRESULT CALLBACK TWnd_WndProc(
    HWND                    hWnd,
    UINT                    msg,
    WPARAM                  wParam,
    LPARAM                  lParam);

/* UiUtils.C
*/
VOID FNLOCAL MessagePump(
    VOID);

VOID FNLOCAL Error(
     HWND                   hWnd,
     int                    nErrDesc,
     MMRESULT               mmrc);

VOID FNLOCAL EmbossedTextOut(
     HDC                    hDC,
     int                    x,
     int                    y,
     LPSTR                  lpsz,
     UINT                   cb,
     COLORREF               crText,
     COLORREF               crShadow,
     int                    cx,
     int                    cy);   

HFONT FNLOCAL CreateScaledFont(
     HDC                    hDC,
     LPRECT                 lpRect,
     LPSTR                  lpszFormat,
     int                    anPosX[],
     int* nPosY);

#endif
