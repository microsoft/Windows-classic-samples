// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*****************************************************************************
*
* TimeWnd.C
*
* Message handlers and other code for the time window
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

/* Into gszFormatHMS[]
*/
#define HOUR_INDEX          0
#define MIN_INDEX           3
#define SEC_INDEX           6

#define MIN_SEP_INDEX       2
#define SEC_SEP_INDEX       5

#define TIMER_ID            0
#define TIMER_INTERVAL      100

PRIVATE HFONT           ghFont          = NULL;
PRIVATE char BCODE      gszFormatHMS[]  = "44:44:44";
PRIVATE char BCODE      gszFormatTicks[]= "444444444";

#define MAX_CBFMT (max(sizeof(gszFormatHMS), sizeof(gszFormatTicks)))

PRIVATE BOOL            gbRepaint       = TRUE;
PRIVATE int             gnPosY;
PRIVATE int             ganPosX[MAX_CBFMT];

PRIVATE int             gnTimerOn       = 0;

PRIVATE VOID NEAR PASCAL PaintTime(HDC hDC);

PRIVATE BOOL NEAR PASCAL TWnd_OnCreate(HWND hWnd, CREATESTRUCT FAR* lpCreateStruct);
PRIVATE VOID NEAR PASCAL TWnd_OnSize(HWND hWnd, UINT state, int cx, int cy);
PRIVATE VOID NEAR PASCAL TWnd_OnPaint(HWND hWnd);
PRIVATE VOID NEAR PASCAL TWnd_OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT codeNotify);
PRIVATE VOID NEAR PASCAL TWnd_OnDestroy(HWND hWnd);
PRIVATE VOID NEAR PASCAL TWnd_OnTimer(HWND hWnd, UINT id);

/*****************************************************************************
*
* PaintTime
*
* Paint the current sequencer time into the time window.
*
* HWND hDC                  - DC to paint time into
*
* Repaint the time. Unless gbRepaint is set, just repaint the sections
* of the time that need updating. Clear gbRepaint when done.
*
*****************************************************************************/
static UINT oldh = 0;
static UINT oldm = 0;
static UINT olds = 0;

static char szOldTicks[MAX_CBFMT];

PRIVATE VOID NEAR PASCAL PaintTime(
    HDC                     hDC)           
{
    TICKS                   tkTime;
    DWORD                   msTime;
    int                     ii;
    int                     cb;
    char                    szWork[MAX_CBFMT];
    HFONT                   hFont;
    UINT                    h;
    UINT                    m;
    UINT                    s;
    BOOL                    fRepaintedSec;
    BOOL                    fRepaintedMin;

    if (MMSYSERR_NOERROR != seqTime(gpSeq, &tkTime))
        return;

    SetBkColor(hDC, RGB(0xFF, 0xFF, 0xFF));
    hFont = SelectObject(hDC, ghFont);
    
    if (gnTimeFormat == IDS_TICKS)
    {
        StringCchPrintf(szWork, MAX_CBFMT, "%9lu", (DWORD)tkTime);

        for (ii = 0; ii < 9; ii++)
        {
            if (gbRepaint || szOldTicks[ii] != szWork[ii])
                EmbossedTextOut(
                                hDC,
                                ganPosX[ii],
                                gnPosY,
                                szWork+ii,
                                1,
                                (COLORREF)-1,
                                (COLORREF)-1,
                                3,
                                3);
            szOldTicks[ii] = szWork[ii];
        }
    }
    else
    {
        msTime = seqTicksToMillisecs(gpSeq, tkTime) / 1000L;

        h = (UINT)(msTime / 3600L);    msTime %= 3600L;
        m = (UINT)(msTime / 60L);      msTime %= 60L;
        s = (UINT)msTime;

        cb = lstrlen(gszFormatHMS);

        if (gbRepaint || h != oldh)
        {
            szWork[0] = '0' + (char)(h / 10);
            szWork[1] = '0' + (char)(h % 10);
            EmbossedTextOut(
                            hDC,
                            ganPosX[HOUR_INDEX],
                            gnPosY,
                            szWork,
                            2,
                            (COLORREF)-1,
                            (COLORREF)-1,
                            3,
                            3);
            oldh = h;
        }

        fRepaintedMin = FALSE;
        if (gbRepaint || m != oldm)
        {
            szWork[0] = gszFormatHMS[MIN_SEP_INDEX];
            szWork[1] = '0' + (char)(m / 10);
            szWork[2] = '0' + (char)(m % 10);
            EmbossedTextOut(
                            hDC,
                            ganPosX[MIN_SEP_INDEX],
                            gnPosY,
                            szWork,
                            3,
                            (COLORREF)-1,
                            (COLORREF)-1,
                            3,
                            3);
            oldm = m;
            fRepaintedMin = TRUE;
        }

        fRepaintedSec = FALSE;
        if (gbRepaint || s != olds)
        {
            szWork[0] = gszFormatHMS[SEC_SEP_INDEX];
            szWork[1] = '0' + (char)(s / 10);
            szWork[2] = '0' + (char)(s % 10);
            EmbossedTextOut(
                            hDC,
                            ganPosX[SEC_SEP_INDEX],
                            gnPosY,
                            szWork,
                            3,
                            (COLORREF)-1,
                            (COLORREF)-1,
                            3,
                            3);
            olds = s;
            fRepaintedSec = TRUE;
        }

        /* If we're doing a full repaint, then update the separators
         */
        if (gbRepaint)
            for (ii=0; ii < cb; ii++)
                if (!isdigit(gszFormatHMS[ii]))
                {
                    if (SEC_SEP_INDEX == ii && fRepaintedSec)
                        continue;

                    if (MIN_SEP_INDEX == ii && fRepaintedMin)
                        continue;

                    EmbossedTextOut(
                                    hDC,
                                    ganPosX[ii],
                                    gnPosY,
                                    gszFormatHMS+ii,
                                    1,
                                    (COLORREF)-1,
                                    (COLORREF)-1,
                                    3,
                                    3);
                }
    }
    
    gbRepaint = FALSE;
    SelectObject(hDC, hFont);
}

/*****************************************************************************
*
* TWnd_OnCreate
*
* Handle WM_CREATE message to time window.
*
* HWND hWnd                 - Window handle
* CREATESTRUCT FAR* lpCreateStruct
*                           - Pointer to creation parameters for the window.
*
* Returns TRUE on success. Returning FALSE will cause the window to be
* destroyed and the application will exit.
*
* Just return TRUE so the window will be created.
*
*****************************************************************************/
BOOL NEAR PASCAL TWnd_OnCreate(
    HWND                    hWnd,
    CREATESTRUCT FAR*       lpCreateStruct)
{
    return TRUE;
}

/*****************************************************************************
*
* TWnd_OnSize
*
* Handle WM_SIZE message to time window.
*
* HWND hWnd                 - Window handle
* UINT state                - Some SIZE_xxx code indicating what type of
*                             size operation this is.
* int  cx, cy               - New x and y size of the window's client area.
*
* Get the new client rect into grcTWnd.
* Destroy and recreate the font scaled to show the time at the correct size.
* Force the time to fully repaint.
*
*****************************************************************************/
VOID NEAR PASCAL TWnd_OnSize(
    HWND                    hWnd,
    UINT                    state,
    int                     cx,
    int                     cy)
{
    HDC                     hDC;
    
    GetClientRect(hWnd, &grcTWnd);

    if (ghFont != (HFONT)NULL)
        DeleteObject(ghFont);

    hDC = GetDC(hWnd);
    ghFont = CreateScaledFont(hDC,
                              &grcTWnd,
                              (gnTimeFormat == IDS_TICKS) ? gszFormatTicks : gszFormatHMS,
                              ganPosX,
                              &gnPosY);
    ReleaseDC(hWnd, hDC);

    gbRepaint = TRUE;
    InvalidateRect(hWnd, NULL, TRUE);
}


/*****************************************************************************
*
* TWnd_OnPaint
*
* Handle WM_PAINT message to time window.
*
* HWND hWnd                 - Window handle
*
* Repaint the 3D inset borders around the edge of the client area.
* Repaint the time.
*
*****************************************************************************/
VOID NEAR PASCAL TWnd_OnPaint( HWND hWnd) { PAINTSTRUCT ps; HDC hDC;
HBRUSH hBrOld; int nWidth; int nHeight;

    RECT                    rc;

    GetClientRect(hWnd, &rc);
    nWidth  = rc.right;
    nHeight = rc.bottom;
    
    hDC = BeginPaint(hWnd, &ps);

    hBrOld = (HBRUSH)SelectObject(hDC, GetStockObject(GRAY_BRUSH));
    PatBlt(hDC, 0, 0, 1, nHeight-1, PATCOPY);
    PatBlt(hDC, 1, 0, nWidth-2, 1, PATCOPY);

    SelectObject(hDC, GetStockObject(BLACK_BRUSH));
    PatBlt(hDC, 1, 1, 1, nHeight-3, PATCOPY);
    PatBlt(hDC, 2, 1, nWidth-4, 1, PATCOPY);

    SelectObject(hDC, GetStockObject(WHITE_BRUSH));
    PatBlt(hDC, rc.right-1, 0, 1, nHeight-1, PATCOPY);
    PatBlt(hDC, 0, rc.bottom-1, nWidth, 1, PATCOPY);

    SelectObject(hDC, GetStockObject(LTGRAY_BRUSH));
    PatBlt(hDC, rc.right-2, 1, 1, nHeight-2, PATCOPY);
    PatBlt(hDC, 1, rc.bottom-2, nWidth-2, 1, PATCOPY);
    
    SelectObject(hDC, hBrOld);

    gbRepaint = TRUE;
    PaintTime(hDC);

    EndPaint(hWnd, &ps);
}

/*****************************************************************************
*
* TWnd_OnCommand
*
* Handle WM_COMMAND message to the time window.
*
* HWND hWnd                 - Window handle
* int id                    - id of control or menu causing WM_COMMAND
* HWND hwndCtl              - Window handle of child control, if any
* UINT codeNotify           - Notification code if this message is from a
*                             control.
*
* We will receive IDM_PLAY and IDM_STOP messages forwarded from the main
* application window whenver playback starts or stops. Use these messages
* to create or kill a timer which will be used to update the time.
* 
*****************************************************************************/
VOID NEAR PASCAL TWnd_OnCommand(
    HWND                    hWnd,
    int                     id,
    HWND                    hWndCtl,
    UINT                    codeNotify)
{
    switch(id)
    {
        case IDM_PLAY:
            if (0 == gnTimerOn)
            {
                gnTimerOn++;
                SetTimer(hWnd, TIMER_ID, TIMER_INTERVAL, 0);
            }

            UpdateWindow(hWnd);
            break;

        case IDM_STOP:
            if (0 == gnTimerOn)
            {
                gnTimerOn--;
                KillTimer(hWnd, TIMER_ID);
            }
            
            UpdateWindow(hWnd);
            break;
    }
}

/*****************************************************************************
*
* TWnd_OnDestroy
*
* Handle WM_DESTROY message to the time window.
*
* HWND hWnd                 - Window handle
*
* If we're being destroyed and the timer is still running, stop it.
* 
*****************************************************************************/
VOID NEAR PASCAL TWnd_OnDestroy(
    HWND                    hWnd)
{
    if (gnTimerOn)
    {
        KillTimer(hWnd, TIMER_ID);
    }
}

/*****************************************************************************
*
* TWnd_OnTimer
*
* Handle WM_TIMER message to the time window.
*
* HWND hWnd                 - Window handle
* UINT id                   - Timer ID
*
* Update the time.
* 
*****************************************************************************/
PRIVATE VOID NEAR PASCAL TWnd_OnTimer(
    HWND                    hWnd,
    UINT                    id)
{
    HDC                     hDC;

    hDC = GetDC(hWnd);
    PaintTime(hDC);
    ReleaseDC(hWnd, hDC);
}

/*****************************************************************************
*
* TWnd_WndProc
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
LRESULT CALLBACK TWnd_WndProc(
    HWND                    hWnd,
    UINT                    msg,
    WPARAM                  wParam,
    LPARAM                  lParam)
{
    switch( msg )
    {
        HANDLE_MSG(hWnd, WM_CREATE,         TWnd_OnCreate);
        HANDLE_MSG(hWnd, WM_SIZE,           TWnd_OnSize);
        HANDLE_MSG(hWnd, WM_PAINT,          TWnd_OnPaint);
        HANDLE_MSG(hWnd, WM_COMMAND,        TWnd_OnCommand);
        HANDLE_MSG(hWnd, WM_DESTROY,        TWnd_OnDestroy);
        HANDLE_MSG(hWnd, WM_TIMER,          TWnd_OnTimer);

        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
}
