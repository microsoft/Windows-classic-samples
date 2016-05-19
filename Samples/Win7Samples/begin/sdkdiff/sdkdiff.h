// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __SDKDIFF_H__
#define __SDKDIFF_H__

/*

 * sdkdiff - file and directory comparisons
 *              FOR INTRODUCTORY NOTES ON IMPLEMENTATION SEE SDKDIFF.C
 *
 *      sdkdiff.h - user-interface definitions, and application-wide global
 *                  declarations.
 */

/* application-wide variables -------------------------------------*/

/* this is the section name in the win.ini file to which we
 * write profile info
 */
#define APPNAME szSdkDiff
extern const CHAR szSdkDiff[];

/* Map profile calls to the registry
 */
#include "profile.h"

/* a gmem_init() heap shared by the app. call gmem_get to alloc. */
extern HANDLE hHeap;

/* the instance handle for this app. needed by anyone who uses resources
 * such as dialogs
 */
extern HINSTANCE hInst;

extern HWND hwndClient;
extern HWND hwndRCD;

/* global option flags-------------------------------------------  */

/* which files do we show in outline mode ? all, changed... */
extern int outline_include;

/* which files do we show in expand mode ? all, different, moved... */
extern int expand_include;

/* outline_include is an OR of the following */
#define INCLUDE_SAME            0x01
#define INCLUDE_DIFFER          0x02
#define INCLUDE_LEFTONLY        0x04
#define INCLUDE_RIGHTONLY       0x08

/* expand_include is an OR of the previous (except for INCLUDE_DIFFER)
   AND the following */
#define INCLUDE_MOVEDLEFT       0x10
#define INCLUDE_MOVEDRIGHT      0x20
#define INCLUDE_SIMILARLEFT     0x40
#define INCLUDE_SIMILARRIGHT    0x80

/* bitwise OR of all flags */
#define INCLUDE_ALL             0xFF

/* do we ignore blanks during the line-by-line diff ? */
extern BOOL ignore_blanks;

/* do we show whitespace characters ? */
extern BOOL show_whitespace;

/* which line numbers do we show - left original, right original or none ?*/
extern int line_numbers;

/* what lines do we show in expand mode - all, left only, right only ? */
extern int expand_mode;

/* TRUE if marked compitems are to be excluded from the view */
extern BOOL hide_markedfiles;


// tab expansion width in characters
extern int g_tabwidth;

extern BOOL TrackLeftOnly;
extern BOOL TrackRightOnly;
extern BOOL TrackDifferent;
extern BOOL TrackSame;
extern BOOL TrackReadonly;

/*--- colour scheme ----------------------------------------------  */

/* outline */
extern DWORD rgb_outlinehi;

/* expand view */
extern DWORD rgb_leftfore;
extern DWORD rgb_leftback;
extern DWORD rgb_rightfore;
extern DWORD rgb_rightback;
extern DWORD rgb_mleftfore;
extern DWORD rgb_mleftback;
extern DWORD rgb_mrightfore;
extern DWORD rgb_mrightback;

/* bar window */
extern DWORD rgb_barleft;
extern DWORD rgb_barright;
extern DWORD rgb_barcurrent;

extern BOOL bJapan;  /* TRUE if primary language is Japanese */
extern BOOL bDBCS;   /* TRUE if primary language is Japanese/Korean/Chinese */

/* -- display layout constants---------------------------------------*/

/* percentage of width of window taken by bar display (when visible) */
#define BAR_WIN_WIDTH   10

/* following are horizontal positions within the bar window, expressed
 * in percent of the width of the bar window
 */
#define L_POS_START     10      /* start of left position marker */
#define L_POS_WIDTH     5       /* width of left position marker */
#define R_POS_START     80      /* start of right position marker */
#define R_POS_WIDTH     5       /* width of right position marker */

#define L_UNMATCH_START 30      /* start of left bar for unmatched section */
#define L_UNMATCH_WIDTH 10      /* width of above */
#define R_UNMATCH_START 60      /* start of right bar for unmatch section */
#define R_UNMATCH_WIDTH 10      /* width of right unmatched section marker */
#define L_MATCH_START   30      /* start of left bar for matched section */
#define L_MATCH_WIDTH   10      /* width of left bar for matched section */
#define R_MATCH_START   60      /* start of right bar for matched section */
#define R_MATCH_WIDTH   10      /* width of right bar for matched section */




/* sdkdiff.c functions */

#ifdef trace
void APIENTRY Trace_File(LPSTR msg);     /* dump msg into Sdkdiff.trc */
#endif


/* if you are about to put up a dialog box or in fact process input in any way
   on any thread other than the main thread - or if you MIGHT be on a thread other
   than the main thread, then you must call this function with TRUE before doing
   it and with FALSE immediately afterwards.  Otherwise you will get one of a
   number of flavours of not-very-responsiveness
*/
void sdkdiff_UI(BOOL bAttach);

/* peek the message queue. return TRUE if an abort request is pending */
BOOL Poll(void);                /* true if abort pending */

/* set the text for the 'names' field (central box) on the status bar */
void SetNames(LPSTR names);

/* set the status field (left field) on the status bar */
void SetStatus(LPSTR state);

/* dlg procs */
int FAR PASCAL FindDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
int FAR PASCAL GoToLineDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

/* in bar.c */
BOOL InitBarClass(HINSTANCE hInstance);
void BarDrawPosition(HWND hwndBar, HDC hdcIn, BOOL bErase);

/*-- private messages -- */

/* send this to the main window. return value is the VIEW handle */
#define TM_CURRENTVIEW  WM_USER


/* --- synchronisation ----------------------------------------- */

/*
 * the WIN32 version spawns worker threads to do time-consuming actions.
 * this causes a possible conflict with the UI thread when accessing the
 * BUSY flag.
 *
 * to protect against this we have a critical section. The UI thread
 * will get this before checking/changing the Busy flag,
 * The worker thread will get this before Busy flag* changes.
 *
 */

//CRITICAL_SECTION CSSdkdiff;

extern BOOL __BERR;

#define TRACE_ERROR(msg,flag) ( sdkdiff_UI(TRUE),                      \
                                __BERR = Trace_Error(hwndClient, msg, flag),       \
                                sdkdiff_UI(FALSE),                     \
                                __BERR                                 \
                              )

#endif
