//------------------------------------------------------------------------------
// File: app.h
//
// Desc: DirectShow sample code
//       - Main header file for VMRPlayer sample
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

typedef TCHAR RECENTFILES[MAX_PATH];

#define MAX_RECENT_FILES    10
#define ID_RECENT_FILE_BASE 500

/* -------------------------------------------------------------------------
** Function prototypes
** -------------------------------------------------------------------------
*/
int
DoMainLoop(
    void
    );

BOOL
InitApplication(
    HINSTANCE hInstance
    );

BOOL
InitInstance(
    HINSTANCE hInstance,
    int nCmdShow
    );

BOOL
LoadWindowPos(
    LPRECT lprc
    );

BOOL
SaveWindowPos(
    HWND hwnd
    );

void
PatB(
    HDC hdc,
    int x,
    int y,
    int dx,
    int dy,
    DWORD rgb
    );

void
GetAdjustedClientRect(
    RECT *prc
    );

BOOL
DrawStats(
    HDC hdc
    );

void
CalcMovieRect(
    LPRECT lprc
    );

LPCTSTR
IdStr(
    int idResource
    );

void
UpdateSystemColors(
    void
    );

void
SetDurationLength(
    REFTIME rt
    );

void
SetCurrentPosition(
    REFTIME rt
    );

TCHAR *
FormatRefTime(
    TCHAR *sz,
    size_t len, 
    REFTIME rt
    );


/* -------------------------------------------------------------------------
** Registry routines
** -------------------------------------------------------------------------
*/
void
ProfileStringOut (
    LPTSTR  szKey,
    LPTSTR  sz
    );

UINT
ProfileStringIn (
    LPTSTR  szKey,
    LPTSTR  szDef,
    LPTSTR  sz,
    DWORD   cb
    );

BOOL
LoadWindowPos(
    LPRECT lprc
    );

BOOL
SaveWindowPos(
    HWND hwnd
    );

HKEY
GetAppKey(
    BOOL fCreate
    );

int
GetRecentFiles(
    int iLastCount,
    int iMenuPosition   // Menu position of start of MRU list
    );

int
SetRecentFiles(
    TCHAR *FileName,    // File name to add
    int iCount,         // Current count of files
    int iMenuPosition   // Menu position of start of MRU list
    );

/* -------------------------------------------------------------------------
** Message crackers
** -------------------------------------------------------------------------
*/
#define HANDLE_WM_USER(hwnd, wParam, lParam, fn) \
    ((fn)(hwnd, wParam, lParam), 0L)

#ifndef HANDLE_WM_NOTIFY
#define HANDLE_WM_NOTIFY(hwnd, wParam, lParam, fn) \
    (fn)((hwnd), (int)(wParam), (NMHDR FAR*)(lParam))
#endif



/* -------------------------------------------------------------------------
** VideoCd window class prototypes
** -------------------------------------------------------------------------
*/
extern "C" LRESULT CALLBACK
VideoCdWndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    );

void
VideoCd_OnClose(
    HWND hwnd
    );

BOOL
VideoCd_OnQueryEndSession(
    HWND hwnd
    );

void
VideoCd_OnDestroy(
    HWND hwnd
    );

void
VideoCd_OnCommand(
    HWND hwnd,
    int id,
    HWND hwndCtl,
    UINT codeNotify
    );

void
VideoCd_OnPaint(
    HWND hwnd
    );

void
VideoCd_OnTimer(
    HWND hwnd,
    UINT id
    );

BOOL
VideoCd_OnCreate(
    HWND hwnd,
    LPCREATESTRUCT lpCreateStruct
    );

void
VideoCd_OnSize(
    HWND hwnd,
    UINT state,
    int cx,
    int cy
    );

void
VideoCd_OnActivate(
    HWND hwnd,
    WPARAM wParam,
    LPARAM lParam
    );

void
VideoCd_OnHScroll(
    HWND hwnd,
    HWND hwndCtl,
    UINT code,
    int pos
    );

void
VideoCd_OnUser(
    HWND hwnd,
    WPARAM wParam,
    LPARAM lParam
    );

void
VideoCd_OnSysColorChange(
    HWND hwnd
    );

void
VideoCd_OnMenuSelect(
    HWND hwnd,
    HMENU hmenu,
    int item,
    HMENU hmenuPopup,
    UINT flags
    );

void
VideoCd_OnInitMenuPopup(
    HWND hwnd,
    HMENU hMenu,
    UINT item,
    BOOL fSystemMenu
    );

#ifdef WM_NOTIFY
LRESULT
VideoCd_OnNotify(
    HWND hwnd,
    int idFrom,
    NMHDR FAR* pnmhdr
    );
#endif


void
VideoCd_OnGraphNotify(
    void
    );

void
SetPlayButtonsEnableState(
    void
    );



/* -------------------------------------------------------------------------
** Command processing functions
** -------------------------------------------------------------------------
*/
BOOL
VcdPlyerCaptureImage(
    LPCTSTR szFile
    );

BOOL
VcdPlyerDisplayCapturedImage(
    LPCTSTR szFile
    );

BOOL
VcdPlayerOpenCmd(
    int i
    );

BOOL
VcdPlayerCloseCmd(
    void
    );

BOOL
VcdPlayerPlayCmd(
    void
    );

BOOL
VcdPlayerStopCmd(
    void
    );

BOOL
VcdPlayerRewindCmd(
    void
    );

BOOL
VcdPlayerPauseCmd(
    void
    );

BOOL
VcdPlayerStepCmd(
    void
    );

void
VcdPlayerSeekCmd(
    REFTIME rtSeekBy
    );

void
ProcessOpen(
    TCHAR *achFileName,
    BOOL bPlay = FALSE
    );

int
VcdPlayerChangeTimeFormat(
    int id
    );

BOOL CALLBACK TransDlgProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK AppImgDlgProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK AboutDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL VerifyVMR9(void);

/* -------------------------------------------------------------------------
** Global Variables
** -------------------------------------------------------------------------
*/
extern int              cxMovie;
extern int              cyMovie;
extern HWND             hwndApp;
extern HWND             g_hwndStatusbar;

extern int              xOffset;
extern int              yOffset;
extern TCHAR            g_achFileName[];
extern OPENFILENAME     ofn;
extern DWORD            g_State;
extern int              nRecentFiles;
extern int              g_TimeFormat;
extern LONG             lMovieOrgX, lMovieOrgY;
extern BOOL             g_bSecondFileLoaded;
extern RECENTFILES      aRecentFiles[MAX_RECENT_FILES];

extern FLOAT            g_xPos, g_yPos, g_xSize, g_ySize, g_Alpha;


/* -------------------------------------------------------------------------
** Constants
** -------------------------------------------------------------------------
*/
#define LEFT_MARGIN 0

#define CAPTURED_IMAGE_NAME		TEXT("VMRImage.bmp\0")

/* -------------------------------------------------------------------------
** Video CD Player states
**
**  These are bit flags
** -------------------------------------------------------------------------
*/
#define VCD_PLAYING          0x0001
#define VCD_STOPPED          0x0002
#define VCD_PAUSED           0x0004
#define VCD_SKIP_F           0x0008
#define VCD_SKIP_B           0x0010
#define VCD_FF               0x0020
#define VCD_RW               0x0040
#define VCD_SEEKING          (VCD_FF | VCD_RW)
#define VCD_LOADED           0x0080
#define VCD_NO_CD            0x0100
#define VCD_DATA_CD_LOADED   0x0200
#define VCD_EDITING          0x0400
#define VCD_PAUSED_AND_MOVED 0x0800
#define VCD_PLAY_PENDING     0x1000
#define VCD_WAS_PLAYING      0x2000
#define VCD_IN_USE           0x4000
#define VCD_STEPPING         0x8000

enum {PerformanceTimer = 32, StatusTimer = 33};
