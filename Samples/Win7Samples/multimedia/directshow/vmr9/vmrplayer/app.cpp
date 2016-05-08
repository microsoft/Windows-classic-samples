//------------------------------------------------------------------------------
// File: app.cpp
//
// Desc: DirectShow sample code
//       - Main source file for VMRPlayer sample
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "project.h"
#include <strsafe.h>

#include <winuser.h>
#include <initguid.h>
#include <strsafe.h>

/* -------------------------------------------------------------------------
** Global variables that are initialized at run time and then stay constant.
** -------------------------------------------------------------------------
*/
HINSTANCE           hInst=0;
HWND                hwndApp=0;
HWND                g_hwndToolbar=0;
HWND                g_hwndStatusbar=0;
HWND                g_hwndTrackbar=0;
CMovie              *pMovie=0;
double              g_TrackBarScale = 1.0;
BOOL                g_bPlay = FALSE;
BOOL                g_fEnableAppImage=0;
BOOL                g_bSecondFileLoaded = FALSE;

int                 dyToolbar=0, dyStatusbar=0, dyTrackbar=0;
int                 FrameStepCount=0;

FLOAT               g_xPos = 0.25F, g_yPos = 0.25F;
FLOAT               g_xSize = 0.5F, g_ySize = 0.5F;
FLOAT               g_Alpha = 0.5F;

typedef struct
{
    FLOAT               xPos,  yPos;
    FLOAT               xSize, ySize;
    FLOAT               Alpha;
} STRM_PARAM;

const STRM_PARAM strParamInit[1] = {
    {0.0F, 0.0F, 1.0F, 1.0F, 1.0F}
};

STRM_PARAM strParam[2] = {
    {0.0F, 0.0F, 1.0F, 1.0F, 1.0F},
    {0.0F, 0.0F, 1.0F, 1.0F, 0.0F}
};


/* -------------------------------------------------------------------------
** True Globals - these may change during execution of the program.
** -------------------------------------------------------------------------
*/
TCHAR               g_achFileName[MAX_PATH];
OPENFILENAME        ofn;
DWORD               g_State = VCD_NO_CD;
LONG                lMovieOrgX, lMovieOrgY;
int                 g_TimeFormat = IDM_TIME;


/* -------------------------------------------------------------------------
** Constants
** -------------------------------------------------------------------------
*/
#define STRM_A  0
#define STRM_B  1

const TCHAR szClassName[] = TEXT("VMR9Player_CLASS\0");
const TCHAR g_szNULL[]    = TEXT("\0");
const TCHAR g_szEmpty[]   = TEXT("");
const TCHAR g_szMovieX[]  = TEXT("MovieOriginX\0");
const TCHAR g_szMovieY[]  = TEXT("MovieOriginY\0");

const int   dxBitmap        = 16;
const int   dyBitmap        = 15;
const int   dxButtonSep     = 8;
const TCHAR g_chNULL        = TEXT('\0');

#ifdef _WIN64
#define RESERVED    0,0,0,0,0,0  /* BYTE bReserved[6]  // padding for alignment */

#else
#define RESERVED    0,0          /* BYTE bReserved[2]  // padding for alignment */
#endif

const TBBUTTON tbButtons[DEFAULT_TBAR_SIZE] = {
    { IDX_SEPARATOR,    1,                    0,               TBSTYLE_SEP           },
    { IDX_1,            IDM_MOVIE_PLAY,       TBSTATE_ENABLED, TBSTYLE_BUTTON, RESERVED, 0, -1 },
    { IDX_2,            IDM_MOVIE_PAUSE,      TBSTATE_ENABLED, TBSTYLE_BUTTON, RESERVED, 0, -1 },
    { IDX_3,            IDM_MOVIE_STOP,       TBSTATE_ENABLED, TBSTYLE_BUTTON, RESERVED, 0, -1 },
    { IDX_SEPARATOR,    2,                    0,               TBSTYLE_SEP           },
    { IDX_4,            IDM_MOVIE_PREVTRACK,  TBSTATE_ENABLED, TBSTYLE_BUTTON, RESERVED, 0, -1 },
    { IDX_5,            IDM_MOVIE_SKIP_BACK,  TBSTATE_ENABLED, TBSTYLE_BUTTON, RESERVED, 0, -1 },
    { IDX_6,            IDM_MOVIE_SKIP_FORE,  TBSTATE_ENABLED, TBSTYLE_BUTTON, RESERVED, 0, -1 },
    { IDX_SEPARATOR,    3,                    0,               TBSTYLE_SEP           },
    { IDX_12,           IDM_MOVIE_STEP,       TBSTATE_ENABLED, TBSTYLE_BUTTON, RESERVED, 0, -1 }
};

const int CX_MOVIE_DEFAULT    = 352;
const int CY_MOVIE_DEFAULT    = 120;



/******************************Public*Routine******************************\
* WinMain
*
*
* Windows recognizes this function by name as the initial entry point
* for the program.  This function calls the application initialization
* routine, if no other instance of the program is running, and always
* calls the instance initialization routine.  It then executes a message
* retrieval and dispatch loop that is the top-level control structure
* for the remainder of execution.  The loop is terminated when a WM_QUIT
* message is received, at which time this function exits the application
* instance by returning the value passed by PostQuitMessage().
*
* If this function must abort before entering the message loop, it
* returns the conventional value FALSE.
*
\**************************************************************************/
int PASCAL
wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLineOld,
    int nCmdShow
    )
{
    LPTSTR lpCmdLine = lpCmdLineOld;

    HRESULT hres = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if(hres == S_FALSE)
    {
        CoUninitialize();
        return FALSE;
    }

    if(!hPrevInstance)
    {
        if(!InitApplication(hInstance))
        {
            return FALSE;
        }
    }

    /*
    ** Perform initializations that apply to a specific instance
    */
    if(!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    /* Verify that the VMR is present on this system */
    if(!VerifyVMR9())
        return FALSE;

    /* Look for options */
    while(lpCmdLine && (*lpCmdLine == '-' || *lpCmdLine == '/'))
    {
        if ((lpCmdLine[1] == 'P') || (lpCmdLine[1] == 'p'))
        {
            g_bPlay = TRUE;
            lpCmdLine += 2;
        }
        else
        {
            break;
        }

        while(lpCmdLine[0] == ' ')
        {
            lpCmdLine++;
        }
    }

    if(lpCmdLine != NULL && lstrlen(lpCmdLine) > 0)
    {
        ProcessOpen(lpCmdLine, g_bPlay);
        SetPlayButtonsEnableState();
    }

    /*
    ** Acquire and dispatch messages until a WM_QUIT message is received.
    */
    return DoMainLoop();
}


/*****************************Private*Routine******************************\
* DoMainLoop
*
* Process the main message loop
*
\**************************************************************************/
int
DoMainLoop(
    void
    )
{
    MSG       msg;
    HANDLE    ahObjects[1];   // handles that need to be waited on
    const int cObjects = 1;   // no of objects that we are waiting on
    HACCEL    haccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_ACCELERATOR));

    //
    // message loop lasts until we get a WM_QUIT message
    // upon which we shall return from the function
    //

    for(;;)
    {
        if(pMovie != NULL)
        {
            ahObjects[0] = pMovie->GetMovieEventHandle();
        }
        else
        {
            ahObjects[0] = NULL;
        }

        if(ahObjects[0] == NULL)
        {
            WaitMessage();
        }
        else
        {
            //
            // wait for any message sent or posted to this queue
            // or for a graph notification
            //
            DWORD result;

            result = MsgWaitForMultipleObjects(cObjects, ahObjects, FALSE,
                                               INFINITE, QS_ALLINPUT);
            if(result != (WAIT_OBJECT_0 + cObjects))
            {
                if(result == WAIT_OBJECT_0)
                    VideoCd_OnGraphNotify();

                continue;
            }
        }

        //
        // When here, we either have a message or no event handle
        // has been created yet.
        //
        // read all of the messages in this next loop
        // removing each message as we read it
        //

        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_QUIT)
            {
                return (int) msg.wParam;
            }

            if(!TranslateAccelerator(hwndApp, haccel, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

} // DoMainLoop


/*****************************Private*Routine******************************\
* InitApplication(HANDLE)
*
* This function is called at initialization time only if no other
* instances of the application are running.  This function performs
* initialization tasks that can be done once for any number of running
* instances.
*
* In this case, we initialize a window class by filling out a data
* structure of type WNDCLASS and calling the Windows RegisterClass()
* function.  Since all instances of this application use the same window
* class, we only need to do this when the first instance is initialized.
*
\**************************************************************************/
BOOL
InitApplication(
    HINSTANCE hInstance
    )
{
    WNDCLASS  wc;

    /*
    ** Fill in window class structure with parameters that describe the
    ** main window.
    */

    wc.style         = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc   = VideoCdWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)NULL; // (COLOR_BTNFACE + 1);
    wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MAIN_MENU);
    wc.lpszClassName = szClassName;

    OSVERSIONINFO OSVer;
    OSVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    BOOL bRet = GetVersionEx((LPOSVERSIONINFO) &OSVer);
    assert(bRet);

    /*
    ** Register the window class and return success/failure code.
    */
    return RegisterClass(&wc);
}


/*****************************Private*Routine******************************\
* InitInstance
*
*
* This function is called at initialization time for every instance of
* this application.  This function performs initialization tasks that
* cannot be shared by multiple instances.
*
* In this case, we save the instance handle in a static variable and
* create and display the main program window.
*
\**************************************************************************/
BOOL
InitInstance(
    HINSTANCE hInstance,
    int nCmdShow
    )
{
    HWND    hwnd;
    RECT    rc;
    POINT   pt;

    /*
    ** Save the instance handle in static variable, which will be used in
    ** many subsequence calls from this application to Windows.
    */
    hInst = hInstance;

    if(! LoadWindowPos(&rc))
        rc.left = rc.top = CW_USEDEFAULT;

    /*
    ** Create a main window for this application instance.
    */
    hwnd = CreateWindow(szClassName, IdStr(STR_APP_TITLE),
                        WS_THICKFRAME | WS_POPUP | WS_CAPTION  |
                        WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
                        WS_CLIPCHILDREN,
                        rc.left, rc.top,
                        rc.right - rc.left, rc.bottom - rc.top,
                        NULL, NULL, hInstance, NULL);

    /*
    ** If window could not be created, return "failure"
    */
    if(NULL == hwnd)
    {
        return FALSE;
    }


    hwndApp = hwnd;
    nRecentFiles = GetRecentFiles(nRecentFiles, 3);

    pt.x = lMovieOrgX =  0;
    pt.y = lMovieOrgY =  0;

    // if we fail to get the working area (screen-tray), then assume
    // the screen is 640x480
    //
    if(!SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, FALSE))
    {
        rc.top    = rc.left = 0;
        rc.right  = 640;
        rc.bottom = 480;
    }

    if(!PtInRect(&rc, pt))
    {
        lMovieOrgX = lMovieOrgY = 0L;
    }

    /*
    ** Make the window visible; update its client area; and return "success"
    */
    SetPlayButtonsEnableState();
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    return TRUE;
}

/*****************************Private*Routine******************************\
* GetMoviePosition
*
* Place the movie in the centre of the client window.  We do not stretch the
* the movie yet !
*
\**************************************************************************/
void
GetMoviePosition(
    HWND hwnd,
    long* xPos,
    long* yPos,
    long* pcx,
    long* pcy
    )
{
    RECT rc;
    GetClientRect(hwnd, &rc);

    rc.top += (dyToolbar + dyTrackbar);
    rc.bottom -= dyStatusbar;

    *xPos = rc.left;
    *yPos = rc.top;
    *pcx = rc.right - rc.left;
    *pcy = rc.bottom - rc.top;
}


/*****************************Private*Routine******************************\
* RepositionMovie
*
\**************************************************************************/
void
RepositionMovie(HWND hwnd)
{
    if(pMovie)
    {
        long xPos, yPos, cx, cy;
        GetMoviePosition(hwnd, &xPos, &yPos, &cx, &cy);
        pMovie->PutMoviePosition(xPos, yPos, cx, cy);

        HDC hdcWin = GetDC(NULL);
        pMovie->RepaintVideo(hwnd, hdcWin);
        ReleaseDC(hwnd, hdcWin);
    }
}

/*****************************Private*Routine******************************\
* VideoCd_OnMove
*
\**************************************************************************/
void
VideoCd_OnMove(
    HWND hwnd,
    int x,
    int y
    )
{
    if(pMovie)
    {
        if(pMovie->GetStateMovie() != State_Running)
        {
            RepositionMovie(hwnd);
        }
        else
        {
            long xPos, yPos, cx, cy;

            // Reposition movie but don't invalidate the rect, since
            // the next video frame will handle the redraw.
            GetMoviePosition(hwnd, &xPos, &yPos, &cx, &cy);
            pMovie->PutMoviePosition(xPos, yPos, cx, cy);
        }
    }
}


/******************************Public*Routine******************************\
* VideoCdWndProc
*
\**************************************************************************/
LRESULT CALLBACK
VideoCdWndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch(message)
    {
        HANDLE_MSG(hwnd, WM_CREATE,            VideoCd_OnCreate);
        HANDLE_MSG(hwnd, WM_PAINT,             VideoCd_OnPaint);
        HANDLE_MSG(hwnd, WM_COMMAND,           VideoCd_OnCommand);
        HANDLE_MSG(hwnd, WM_CLOSE,             VideoCd_OnClose);
        HANDLE_MSG(hwnd, WM_QUERYENDSESSION,   VideoCd_OnQueryEndSession);
        HANDLE_MSG(hwnd, WM_DESTROY,           VideoCd_OnDestroy);
        HANDLE_MSG(hwnd, WM_SIZE,              VideoCd_OnSize);
        HANDLE_MSG(hwnd, WM_SYSCOLORCHANGE,    VideoCd_OnSysColorChange);
        HANDLE_MSG(hwnd, WM_MENUSELECT,        VideoCd_OnMenuSelect);
        HANDLE_MSG(hwnd, WM_INITMENUPOPUP,     VideoCd_OnInitMenuPopup);
        HANDLE_MSG(hwnd, WM_HSCROLL,           VideoCd_OnHScroll);
        HANDLE_MSG(hwnd, WM_TIMER,             VideoCd_OnTimer);
        HANDLE_MSG(hwnd, WM_NOTIFY,            VideoCd_OnNotify);
        HANDLE_MSG(hwnd, WM_MOVE,              VideoCd_OnMove);

        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
            VcdPlyerCaptureImage(CAPTURED_IMAGE_NAME);
            break;

        case WM_DISPLAYCHANGE:
            if(pMovie)
                pMovie->DisplayModeChanged();
            break;

        // Note: we do not use HANDLE_MSG here as we want to call
        // DefWindowProc after we have notifed the FilterGraph Resource Manager,
        // otherwise our window will not finish its activation process.

        case WM_ACTIVATE: 
            VideoCd_OnActivate(hwnd, wParam, lParam);

            // IMPORTANT - let this drop through to DefWindowProc

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0L;
}


/*****************************Private*Routine******************************\
* VideoCd_OnCreate
*
\**************************************************************************/
BOOL
VideoCd_OnCreate(
    HWND hwnd,
    LPCREATESTRUCT lpCreateStruct
    )
{
    RECT rc;
    int Pane[2];

    InitCommonControls();

    /*
    ** Create the toolbar and statusbar.
    */
    g_hwndToolbar = CreateToolbarEx(hwnd,
                                    WS_VISIBLE | WS_CHILD |
                                    TBSTYLE_TOOLTIPS | CCS_NODIVIDER,
                                    ID_TOOLBAR, NUMBER_OF_BITMAPS,
                                    hInst, IDR_TOOLBAR, tbButtons,
                                    DEFAULT_TBAR_SIZE, dxBitmap, dyBitmap,
                                    dxBitmap, dyBitmap, sizeof(TBBUTTON));
    if(g_hwndToolbar == NULL)
        return FALSE;

    g_hwndStatusbar = CreateStatusWindow(WS_VISIBLE | WS_CHILD | CCS_BOTTOM,
        TEXT("Example Text"), hwnd, ID_STATUSBAR);

    GetWindowRect(g_hwndToolbar, &rc);
    dyToolbar = rc.bottom - rc.top;

    GetWindowRect(g_hwndStatusbar, &rc);
    dyStatusbar = rc.bottom - rc.top;

    dyTrackbar = 30;

    GetClientRect(hwnd, &rc);
    Pane[0] = (rc.right - rc.left) / 2 ;
    Pane[1] = -1;
    SendMessage(g_hwndStatusbar, SB_SETPARTS, 2, (LPARAM)Pane);


    g_hwndTrackbar = CreateWindowEx(0, TRACKBAR_CLASS, TEXT("Trackbar Control"),
                                    WS_CHILD | WS_VISIBLE |
                                    TBS_AUTOTICKS | TBS_ENABLESELRANGE,
                                    LEFT_MARGIN, dyToolbar - 1,
                                    (rc.right - rc.left) - (2* LEFT_MARGIN),
                                    dyTrackbar, hwnd, (HMENU)ID_TRACKBAR,
                                    hInst, NULL);

    SetDurationLength((REFTIME)0);
    SetCurrentPosition((REFTIME)0);

    SetTimer(hwnd, StatusTimer, 500, NULL);

    if(g_hwndStatusbar == NULL || g_hwndTrackbar == NULL)
    {
        return FALSE;
    }

    // accept filemanager WM_DROPFILES messages
    DragAcceptFiles(hwnd, TRUE);

    return TRUE;
}


/*****************************Private*Routine******************************\
* VideoCd_OnActivate
*
\**************************************************************************/
void
VideoCd_OnActivate(
    HWND hwnd,
    WPARAM wParam,
    LPARAM lParam
    )
{
    if((UINT)LOWORD(wParam))
    {
        // we are being activated - tell the Filter graph (for Sound follows focus)
        if(pMovie)
        {
            pMovie->SetFocus();
        }
    }
}


/*****************************Private*Routine******************************\
* VideoCd_OnHScroll
*
\**************************************************************************/
void
VideoCd_OnHScroll(
    HWND hwnd,
    HWND hwndCtl,
    UINT code,
    int pos
    )
{
    static BOOL fWasPlaying = FALSE;
    static BOOL fBeginScroll = FALSE;

    if(pMovie == NULL)
    {
        return;
    }

    if(hwndCtl == g_hwndTrackbar)
    {
        REFTIME     rtCurrPos;
        REFTIME     rtTrackPos;
        REFTIME     rtDuration;

        pos = (int)SendMessage(g_hwndTrackbar, TBM_GETPOS, 0, 0);
        rtTrackPos = (REFTIME)pos * g_TrackBarScale;

        switch(code)
        {
            case TB_BOTTOM:
                rtDuration = pMovie->GetDuration();
                rtCurrPos = pMovie->GetCurrentPosition();
                VcdPlayerSeekCmd(rtDuration - rtCurrPos);
                SetCurrentPosition(pMovie->GetCurrentPosition());
                break;

            case TB_TOP:
                rtCurrPos = pMovie->GetCurrentPosition();
                VcdPlayerSeekCmd(-rtCurrPos);
                SetCurrentPosition(pMovie->GetCurrentPosition());
                break;

            case TB_LINEDOWN:
                VcdPlayerSeekCmd(10.0);
                SetCurrentPosition(pMovie->GetCurrentPosition());
                break;

            case TB_LINEUP:
                VcdPlayerSeekCmd(-10.0);
                SetCurrentPosition(pMovie->GetCurrentPosition());
                break;

            case TB_ENDTRACK:
                fBeginScroll = FALSE;
                if(fWasPlaying)
                {
                    VcdPlayerPauseCmd();
                    fWasPlaying = FALSE;
                }
                break;

            case TB_THUMBTRACK:
                if(!fBeginScroll)
                {
                    fBeginScroll = TRUE;
                    fWasPlaying = (g_State & VCD_PLAYING);
                    if(fWasPlaying)
                    {
                        VcdPlayerPauseCmd();
                    }
                }
                // Fall through to PAGEUP/PAGEDOWN processing

            case TB_PAGEUP:
            case TB_PAGEDOWN:
                rtCurrPos = pMovie->GetCurrentPosition();
                VcdPlayerSeekCmd(rtTrackPos - rtCurrPos);
                SetCurrentPosition(pMovie->GetCurrentPosition());
                break;
        }
    }
}


/*****************************Private*Routine******************************\
* VideoCd_OnTimer
*
\**************************************************************************/
void
VideoCd_OnTimer(
    HWND hwnd,
    UINT id
    )
{
    HRESULT hr;
    if(pMovie && pMovie->StatusMovie() == MOVIE_PLAYING)
    {
        switch(id)
        {
            case StatusTimer:
            {
                REFTIME rt = pMovie->GetCurrentPosition();
                SetCurrentPosition(rt);

                if(1)
                {
                    TCHAR   szFmt[64];
                    TCHAR   sz[64];
                    long cx, cy;

                    pMovie->GetNativeMovieSize(&cx, &cy);
                    hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%s"), FormatRefTime(szFmt, NUMELMS(sz),rt));

                    HDC hdc = GetDC(hwndApp);
                    HBITMAP hbmp = CreateCompatibleBitmap(hdc, 128, 128);
                    HBITMAP hbmpVmr = LoadBitmap(hInst, MAKEINTRESOURCE(IDR_VMR));
                    HDC hdcBmp = CreateCompatibleDC(hdc);
                    HDC hdcVMR = CreateCompatibleDC(hdc);
                    ReleaseDC(hwndApp, hdc);

                    HBITMAP hbmpold = (HBITMAP)SelectObject(hdcBmp, hbmp);
                    hbmpVmr = (HBITMAP)SelectObject(hdcVMR, hbmpVmr);
                    BitBlt(hdcBmp, 0, 0, 128, 128, hdcVMR, 0, 0, SRCCOPY);
                    DeleteObject(SelectObject(hdcVMR, hbmpVmr));
                    DeleteDC(hdcVMR);

                    RECT rc;
                    SetRect(&rc, 0, 0, 128, 32);
                    SetBkColor(hdcBmp, RGB(0, 0, 128));
                    SetTextColor(hdcBmp, RGB(255, 255, 255));

                    DrawText(hdcBmp, sz, lstrlen(sz), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                    VMR9AlphaBitmap bmpInfo;
                    ZeroMemory(&bmpInfo, sizeof(bmpInfo));
                    bmpInfo.dwFlags = VMRBITMAP_HDC | VMRBITMAP_SRCCOLORKEY;
                    bmpInfo.hdc = hdcBmp;
                    SetRect(&rc, 0, 0, 128, 128);
                    bmpInfo.rSrc = rc;

                    bmpInfo.rDest.left = g_xPos;
                    bmpInfo.rDest.top = g_yPos;
                    bmpInfo.rDest.right = g_xPos + g_xSize;
                    bmpInfo.rDest.bottom = g_yPos + g_ySize;

                    if(g_fEnableAppImage)
                    {
                        bmpInfo.fAlpha = g_Alpha;
                    }
                    else
                    {
                        bmpInfo.fAlpha = 0.0F;
                    }
                    bmpInfo.clrSrcKey = 0;

                    pMovie->SetAppImage(&bmpInfo);

                    DeleteObject(SelectObject(hdcBmp, hbmpold));
                    DeleteDC(hdcBmp);
                }
            }
            break;
        }
    }
}


/*****************************Private*Routine******************************\
* VideoCd_OnPaint
*
\**************************************************************************/
void
VideoCd_OnPaint(
    HWND hwnd
    )
{
    PAINTSTRUCT ps;
    HDC         hdc;
    RECT        rc1;
    RECT        rc2;

    /*
    ** Draw a frame around the movie playback area.
    */
    GetClientRect(hwnd, &rc2);

    hdc = BeginPaint(hwnd, &ps);

    if(pMovie)
    {
        long xPos, yPos, cx, cy;
        GetMoviePosition(hwnd, &xPos, &yPos, &cx, &cy);
        SetRect(&rc1, xPos, yPos, xPos + cx, yPos + cy);

        HRGN rgnClient = CreateRectRgnIndirect(&rc2);
        HRGN rgnVideo  = CreateRectRgnIndirect(&rc1);
        CombineRgn(rgnClient, rgnClient, rgnVideo, RGN_DIFF);

        HBRUSH hbr = GetSysColorBrush(COLOR_BTNFACE);
        FillRgn(hdc, rgnClient, hbr);
        DeleteObject(hbr);
        DeleteObject(rgnClient);
        DeleteObject(rgnVideo);

        pMovie->RepaintVideo(hwnd, hdc);
    }
    else
    {
        FillRect(hdc, &rc2, (HBRUSH)(COLOR_BTNFACE + 1));
    }

    EndPaint(hwnd, &ps);
}


/*****************************Private*Routine******************************\
* VideoCd_OnCommand
*
\**************************************************************************/
void
VideoCd_OnCommand(
    HWND hwnd,
    int id,
    HWND hwndCtl,
    UINT codeNotify
    )
{
    switch(id)
    {
        case IDM_FILE_OPEN:
            VcdPlayerCloseCmd();
            VcdPlayerOpenCmd(STRM_A);
            break;

        case IDM_FILE_OPEN2:
            VcdPlayerOpenCmd(STRM_B);
            break;

        case IDM_FILE_CLOSE:
            VcdPlayerCloseCmd();
            break;

        case IDM_FILE_EXIT:
            PostMessage(hwnd, WM_CLOSE, 0, 0L);
            break;

        case IDM_MOVIE_PLAY:
            VcdPlayerPlayCmd();
            break;

        case IDM_MOVIE_STOP:
            VcdPlayerStopCmd();
            break;

        case IDM_MOVIE_PAUSE:
            VcdPlayerPauseCmd();
            break;

        case IDM_MOVIE_SKIP_FORE:
            VcdPlayerSeekCmd(1.0);
            break;

        case IDM_MOVIE_SKIP_BACK:
            VcdPlayerSeekCmd(-1.0);
            break;

        case IDM_MOVIE_PREVTRACK:
            if(pMovie)
                VcdPlayerSeekCmd(-pMovie->GetCurrentPosition());
            break;

        case IDM_MOVIE_STEP:
            VcdPlayerStepCmd();
            break;

        case IDM_APP_IMAGE:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_IMAGECTRL), hwnd, (DLGPROC)AppImgDlgProc);
            break;

        case IDM_STREAM_A:
            DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_IMAGECTRL), hwnd,
                          (DLGPROC)TransDlgProc, (LPARAM)STRM_A);
            break;

        case IDM_STREAM_B:
            DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_IMAGECTRL), hwnd,
                          (DLGPROC)TransDlgProc, (LPARAM)STRM_B);
            break;

        case IDM_CAPTURE_IMAGE:
            VcdPlyerCaptureImage(CAPTURED_IMAGE_NAME);
            break;

        case IDM_DISPLAY_CAPTURED_IMAGE:
            VcdPlyerDisplayCapturedImage(CAPTURED_IMAGE_NAME);
            break;

        case IDM_HELP_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX),
                hwnd,  (DLGPROC) AboutDlgProc);
            break;

        default:
            if(id > ID_RECENT_FILE_BASE
                && id <= (ID_RECENT_FILE_BASE + MAX_RECENT_FILES + 1))
            {
                ProcessOpen(aRecentFiles[id - ID_RECENT_FILE_BASE - 1]);
            }
            break;
    }

    SetPlayButtonsEnableState();
}


/******************************Public*Routine******************************\
* VideoCd_OnDestroy
*
\**************************************************************************/
void
VideoCd_OnDestroy(
    HWND hwnd
    )
{
    PostQuitMessage(0);
}


/******************************Public*Routine******************************\
* VideoCd_OnClose
*
\**************************************************************************/
void
VideoCd_OnClose(
    HWND hwnd
    )
{
    // stop accepting dropped filenames
    DragAcceptFiles(hwnd, FALSE);

    VcdPlayerCloseCmd();
    SaveWindowPos(hwnd);
    DestroyWindow(hwnd);
}


/*****************************Private*Routine******************************\
* VideoCd_OnQueryEndSession
*
\**************************************************************************/
BOOL
VideoCd_OnQueryEndSession(
    HWND hwnd
    )
{
    SaveWindowPos(hwnd);
    return TRUE;
}


/******************************Public*Routine******************************\
* VideoCd_OnSize
*
\**************************************************************************/
void
VideoCd_OnSize(
    HWND hwnd,
    UINT state,
    int dx,
    int dy
    )
{
    if(IsWindow(g_hwndStatusbar))
    {
        int Pane[2] = {dx/2-8, -1};

        SendMessage(g_hwndStatusbar, WM_SIZE, 0, 0L);
        SendMessage(g_hwndStatusbar, SB_SETPARTS, 2, (LPARAM)Pane);
    }

    if(IsWindow(g_hwndTrackbar))
    {
        SetWindowPos(g_hwndTrackbar, HWND_TOP, LEFT_MARGIN, dyToolbar - 1,
                     dx - (2 * LEFT_MARGIN), dyTrackbar, SWP_NOZORDER);
    }

    if(IsWindow(g_hwndToolbar))
    {
        SendMessage(g_hwndToolbar, WM_SIZE, 0, 0L);
    }

    RepositionMovie(hwnd);
}


/*****************************Private*Routine******************************\
* VideoCd_OnSysColorChange
*
\**************************************************************************/
void
VideoCd_OnSysColorChange(
    HWND hwnd
    )
{
    FORWARD_WM_SYSCOLORCHANGE(g_hwndToolbar, SendMessage);
    FORWARD_WM_SYSCOLORCHANGE(g_hwndStatusbar, SendMessage);
}


/*****************************Private*Routine******************************\
* VideoCd_OnInitMenuPopup
*
\**************************************************************************/
void
VideoCd_OnInitMenuPopup(
    HWND hwnd,
    HMENU hMenu,
    UINT item,
    BOOL fSystemMenu
    )
{
    UINT uFlags;

    switch(item)
    {
        case 0: // File menu
            if(g_State & (VCD_IN_USE | VCD_NO_CD | VCD_DATA_CD_LOADED))
                uFlags = (MF_BYCOMMAND | MF_GRAYED);
            else
                uFlags = (MF_BYCOMMAND | MF_ENABLED);

            EnableMenuItem(hMenu, IDM_FILE_CLOSE, uFlags);

            // Disable the "Open Second Stream" item if already opened
            if (g_bSecondFileLoaded)
                EnableMenuItem(hMenu, IDM_FILE_OPEN2, MF_BYCOMMAND | MF_GRAYED);
            else
                EnableMenuItem(hMenu, IDM_FILE_OPEN2, uFlags);
            break;

        case 1: // Properties menu
            if(g_State & (VCD_IN_USE | VCD_NO_CD | VCD_DATA_CD_LOADED))
                uFlags = (MF_BYCOMMAND | MF_GRAYED);
            else
                uFlags = (MF_BYCOMMAND | MF_ENABLED);

            EnableMenuItem(hMenu, IDM_APP_IMAGE, uFlags);
            EnableMenuItem(hMenu, IDM_STREAM_A, uFlags);
            EnableMenuItem(hMenu, IDM_CAPTURE_IMAGE, uFlags);
            EnableMenuItem(hMenu, IDM_DISPLAY_CAPTURED_IMAGE, uFlags);

            if (!g_bSecondFileLoaded)
                uFlags = (MF_BYCOMMAND | MF_GRAYED);
            else
                uFlags = (MF_BYCOMMAND | MF_ENABLED);

            EnableMenuItem(hMenu, IDM_STREAM_B, uFlags);
            break;

        case 2: // Help menu
            break;
    }
}


/*****************************Private*Routine******************************\
* VideoCd_OnGraphNotify
*
* This is where we get any notifications from the filter graph.
*
\**************************************************************************/
void
VideoCd_OnGraphNotify(
    void
    )
{
    long lEventCode = pMovie->GetMovieEventCode();

    switch(lEventCode)
    {
        case EC_STEP_COMPLETE:
            g_State &= ~VCD_STEPPING;
            SetPlayButtonsEnableState();
            break;

        case EC_COMPLETE:
        case EC_USERABORT:
        case EC_ERRORABORT:
            VcdPlayerStopCmd();
            SetPlayButtonsEnableState();
            break;

        default:
            break;
    }
}


/*****************************Private*Routine******************************\
* VideoCd_OnNotify
*
* This is where we get the text for the little tooltips
*
\**************************************************************************/
LRESULT
VideoCd_OnNotify(
    HWND hwnd,
    int idFrom,
    NMHDR FAR* pnmhdr
    )
{
    switch(pnmhdr->code)
    {
        case TTN_NEEDTEXT:
        {
            LPTOOLTIPTEXT   lpTt;

            lpTt = (LPTOOLTIPTEXT)pnmhdr;
            LoadString(hInst, (UINT) lpTt->hdr.idFrom, lpTt->szText,
                       NUMELMS(lpTt->szText));
        }
        break;
    }

    return 0;
}


/*****************************Private*Routine******************************\
* VideoCd_OnMenuSelect
*
\**************************************************************************/
void
VideoCd_OnMenuSelect(
    HWND hwnd,
    HMENU hmenu,
    int item,
    HMENU hmenuPopup,
    UINT flags
    )
{
    TCHAR szString[STR_MAX_STRING_LEN + 1];
    HRESULT hr;

    /*
    ** Is it time to end the menu help ?
    */
    if((flags == 0xFFFFFFFF) && (hmenu == NULL))
    {
        SendMessage(g_hwndStatusbar, SB_SIMPLE, 0, 0L);
    }

    /*
    ** Do we have a separator, popup or the system menu ?
    */
    else if(flags & MF_POPUP)
    {
        SendMessage(g_hwndStatusbar, SB_SIMPLE, 0, 0L);
    }
    else if(flags & MF_SYSMENU)
    {
        switch(item)
        {
            case SC_RESTORE:
                hr = StringCchCopy(szString, NUMELMS(szString), IdStr(STR_SYSMENU_RESTORE));
                break;

            case SC_MOVE:
                hr = StringCchCopy(szString, NUMELMS(szString), IdStr(STR_SYSMENU_MOVE));
                break;

            case SC_MINIMIZE:
                hr = StringCchCopy(szString, NUMELMS(szString), IdStr(STR_SYSMENU_MINIMIZE));
                break;

            case SC_MAXIMIZE:
                hr = StringCchCopy(szString, NUMELMS(szString), IdStr(STR_SYSMENU_MAXIMIZE));
                break;

            case SC_TASKLIST:
                hr = StringCchCopy(szString, NUMELMS(szString), IdStr(STR_SYSMENU_TASK_LIST));
                break;

            case SC_CLOSE:
                hr = StringCchCopy(szString, NUMELMS(szString), IdStr(STR_SYSMENU_CLOSE));
                break;
        }

        SendMessage(g_hwndStatusbar, SB_SETTEXT, SBT_NOBORDERS|255,
                   (LPARAM)(LPTSTR)szString);
        SendMessage(g_hwndStatusbar, SB_SIMPLE, 1, 0L);
        UpdateWindow(g_hwndStatusbar);
    }

    /*
    ** Hopefully it's one of ours
    */
    else
    {
        if((flags & MF_SEPARATOR))
        {
            szString[0] = g_chNULL;
        }
        else
        {
            hr = StringCchCopy(szString, NUMELMS(szString), IdStr(item + MENU_STRING_BASE));
        }

        SendMessage(g_hwndStatusbar, SB_SETTEXT, SBT_NOBORDERS|255,
                   (LPARAM)(LPTSTR)szString);
        SendMessage(g_hwndStatusbar, SB_SIMPLE, 1, 0L);
        UpdateWindow(g_hwndStatusbar);
    }
}


/******************************Public*Routine******************************\
* SetPlayButtonsEnableState
*
* Sets the play buttons enable state to match the state of the current
* cdrom device.  See below...
*
*
*                 VCD Player buttons enable state table
* -------------------------------------------------------------------
* ³E=Enabled D=Disabled      ³ Play ³ Pause ³ Eject ³ Stop  ³ Other ³
* -------------------------------------------------------------------
* ³Disk in use               ³  D   ³  D    ³  D    ³   D   ³   D   ³
* -------------------------------------------------------------------
* ³No video cd or data cdrom ³  D   ³  D    ³  E    ³   D   ³   D   ³
* -------------------------------------------------------------------
* ³Video cd (playing)        ³  D   ³  E    ³  E    ³   E   ³   E   ³
* -------------------------------------------------------------------
* ³Video cd (paused)         ³  E   ³  D    ³  E    ³   E   ³   E   ³
* -------------------------------------------------------------------
* ³Video cd (stopped)        ³  E   ³  D    ³  E    ³   D   ³   E   ³
* -------------------------------------------------------------------
*
*
\**************************************************************************/
void
SetPlayButtonsEnableState(
    void
    )
{
    BOOL    fEnable;
    BOOL    fVideoLoaded;

    /*
    ** Do we have a video cd loaded.
    */
    if(g_State & (VCD_NO_CD | VCD_DATA_CD_LOADED | VCD_IN_USE))
    {
        fVideoLoaded = FALSE;
    }
    else
    {
        fVideoLoaded = TRUE;
    }

    /*
    ** Do the play button
    */
    if(fVideoLoaded
        && ((g_State & VCD_STOPPED) || (g_State & VCD_PAUSED)))
    {
        fEnable = TRUE;
    }
    else
    {
        fEnable = FALSE;
    }
    SendMessage(g_hwndToolbar, TB_ENABLEBUTTON, IDM_MOVIE_PLAY, fEnable);

    /*
    ** Do the stop button
    */
    if(fVideoLoaded
        && ((g_State & VCD_PLAYING) || (g_State & VCD_PAUSED)))
    {
        fEnable = TRUE;
    }
    else
    {
        fEnable = FALSE;
    }
    SendMessage(g_hwndToolbar, TB_ENABLEBUTTON, IDM_MOVIE_STOP, fEnable);

    /*
    ** Do the pause button
    */
    if(fVideoLoaded && (g_State & VCD_PLAYING))
    {
        fEnable = TRUE;
    }
    else
    {
        fEnable = FALSE;
    }
    SendMessage(g_hwndToolbar, TB_ENABLEBUTTON, IDM_MOVIE_PAUSE, fEnable);


    fEnable = FALSE;
    if(fVideoLoaded && pMovie->CanMovieFrameStep())
    {
        if(!(g_State & VCD_STEPPING))
        {
            fEnable = TRUE;
        }
    }
    SendMessage(g_hwndToolbar, TB_ENABLEBUTTON, IDM_MOVIE_STEP, fEnable);

    /*
    ** Do the seeking buttons
    */
    if((g_State & VCD_PAUSED) || (!fVideoLoaded))
        fEnable = FALSE;
    else
        fEnable = TRUE;

    SendMessage(g_hwndToolbar, TB_ENABLEBUTTON,
        IDM_MOVIE_SKIP_FORE, fEnable);

    SendMessage(g_hwndToolbar, TB_ENABLEBUTTON,
        IDM_MOVIE_SKIP_BACK, fEnable);

    SendMessage(g_hwndToolbar, TB_ENABLEBUTTON,
        IDM_MOVIE_PREVTRACK, fEnable);
}


/*****************************Private*Routine******************************\
* GetAdjustedClientRect
*
* Calculate the size of the client rect and then adjusts it to take into
* account the space taken by the toolbar and status bar.
*
\**************************************************************************/
void
GetAdjustedClientRect(
    RECT *prc
    )
{
    RECT rcTool;

    GetClientRect(hwndApp, prc);

    GetWindowRect(g_hwndToolbar, &rcTool);
    prc->top += (rcTool.bottom - rcTool.top);

    GetWindowRect(g_hwndTrackbar, &rcTool);
    prc->top += (rcTool.bottom - rcTool.top);

    GetWindowRect(g_hwndStatusbar, &rcTool);
    prc->bottom -= (rcTool.bottom - rcTool.top);
}


/******************************Public*Routine******************************\
* IdStr
*
* Loads the given string resource ID into the passed storage.
*
\**************************************************************************/
LPCTSTR
IdStr(
    int idResource
    )
{
    static TCHAR chBuffer[ STR_MAX_STRING_LEN ];

    if(LoadString(hInst, idResource, chBuffer, STR_MAX_STRING_LEN) == 0)
        return g_szEmpty;

    return chBuffer;
}


/*****************************Private*Routine******************************\
* SetDurationLength
*
* Updates pane 0 on the status bar
*
\**************************************************************************/
void
SetDurationLength(
    REFTIME rt
    )
{
    TCHAR   szFmt[64];
    TCHAR   sz[64];

    g_TrackBarScale = 1.0;
    while(rt / g_TrackBarScale > 30000)
    {
        g_TrackBarScale *= 10;
    }

    SendMessage(g_hwndTrackbar, TBM_SETRANGE, TRUE,
        MAKELONG(0, (WORD)(rt / g_TrackBarScale)));

    SendMessage(g_hwndTrackbar, TBM_SETTICFREQ, (WPARAM)((int)(rt / g_TrackBarScale) / 9), 0);
    SendMessage(g_hwndTrackbar, TBM_SETPAGESIZE, 0, (LPARAM)((int)(rt / g_TrackBarScale) / 9));

    HRESULT hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("Length: %s\0"), FormatRefTime(szFmt, NUMELMS(szFmt), rt));
    SendMessage(g_hwndStatusbar, SB_SETTEXT, 0, (LPARAM)sz);
}


/*****************************Private*Routine******************************\
* SetCurrentPosition
*
* Updates pane 1 on the status bar
*
\**************************************************************************/
void
SetCurrentPosition(
    REFTIME rt
    )
{
    TCHAR   szFmt[64];
    TCHAR   sz[64];

    SendMessage(g_hwndTrackbar, TBM_SETPOS, TRUE, (LPARAM)(rt / g_TrackBarScale));

    HRESULT hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("Elapsed: %s\0"), FormatRefTime(szFmt, NUMELMS(szFmt),rt));
    SendMessage(g_hwndStatusbar, SB_SETTEXT, 1, (LPARAM)sz);
}


/*****************************Private*Routine******************************\
* FormatRefTime
*
* Formats the given RefTime into the passed in character buffer,
* returns a pointer to the character buffer.
*
\**************************************************************************/
TCHAR *
FormatRefTime(
    TCHAR *sz,
    size_t len,
    REFTIME rt
    )
{
    // If we are not seeking in time then format differently

    HRESULT hr;
    if(pMovie && pMovie->GetTimeFormat() != TIME_FORMAT_MEDIA_TIME)
    {
        hr = StringCchPrintf(sz, len, TEXT("%I64d\0"),(LONGLONG)rt);
        return sz;
    }

    int hrs, mins, secs;

    rt += 0.49;

    hrs  =  (int)rt / 3600;
    mins = ((int)rt % 3600) / 60;
    secs = ((int)rt % 3600) % 60;

    hr = StringCchPrintf(sz, len, TEXT("%02d:%02d:%02d h:m:s\0"),hrs, mins, secs);
    return sz;
}


/*****************************Private*Routine******************************\
* InitStreamParams
*
\**************************************************************************/
void InitStreamParams(int i)
{
    CopyMemory(&strParam[i], strParamInit, sizeof(strParamInit));

    if(i == STRM_B)
    {
        strParam[STRM_B].Alpha = 0.5F;
    }
}


/*****************************Private*Routine******************************\
* UpdateAppImage
*
\**************************************************************************/
void
UpdateAppImage()
{
    VMR9AlphaBitmap bmpInfo;
    ZeroMemory(&bmpInfo, sizeof(bmpInfo));
    bmpInfo.rDest.left   = g_xPos;
    bmpInfo.rDest.top    = g_yPos;
    bmpInfo.rDest.right  = g_xPos + g_xSize;
    bmpInfo.rDest.bottom = g_yPos + g_ySize;
    bmpInfo.dwFlags      = VMRBITMAP_SRCCOLORKEY;
    bmpInfo.clrSrcKey    = 0;

    if(!g_fEnableAppImage)
    {
        bmpInfo.fAlpha = 0.0F;
    }
    else
    {
        bmpInfo.fAlpha = g_Alpha;
    }

    if (pMovie)
        pMovie->UpdateAppImage(&bmpInfo);
}


/*****************************Private*Routine******************************\
* AppImgDlgProc
*
\**************************************************************************/
BOOL CALLBACK
AppImgDlgProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    HWND hwndT;
    int pos;
    TCHAR sz[32];
    HRESULT hr;

    switch(uMsg)
    {
        case WM_INITDIALOG:

            hwndT = GetDlgItem(hwnd, IDC_XPOS_TRK);
            pos = int(1000 * g_xPos);
            SendMessage(hwndT, TBM_SETRANGE, TRUE, MAKELONG(0, (WORD)(1000)));
            SendMessage(hwndT, TBM_SETPOS, TRUE, (LPARAM)(pos));
            hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%.3f"), g_xPos);
            SetWindowText(hwndT, sz);
            SetDlgItemText(hwnd, IDC_XPOS, sz);

            pos = int(1000 * g_yPos);
            hwndT = GetDlgItem(hwnd, IDC_YPOS_TRK);
            SendMessage(hwndT, TBM_SETRANGE, TRUE, MAKELONG(0, (WORD)(1000)));
            SendMessage(hwndT, TBM_SETPOS, TRUE, (LPARAM)(pos));
            hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%.3f"), g_yPos);
            SetWindowText(hwndT, sz);
            SetDlgItemText(hwnd, IDC_YPOS, sz);

            pos = int(1000 * g_xSize);
            hwndT = GetDlgItem(hwnd, IDC_XSIZE_TRK);
            SendMessage(hwndT, TBM_SETRANGE, TRUE, MAKELONG(0, (WORD)(1000)));
            SendMessage(hwndT, TBM_SETPOS, TRUE, (LPARAM)(pos));
            hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%.3f"), g_xSize);
            SetWindowText(hwndT, sz);
            SetDlgItemText(hwnd, IDC_XSIZE, sz);

            pos = int(1000 * g_ySize);
            hwndT = GetDlgItem(hwnd, IDC_YSIZE_TRK);
            SendMessage(hwndT, TBM_SETRANGE, TRUE, MAKELONG(0, (WORD)(1000)));
            SendMessage(hwndT, TBM_SETPOS, TRUE, (LPARAM)(pos));
            hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%.3f"), g_ySize);
            SetDlgItemText(hwnd, IDC_YSIZE, sz);

            pos = int(1000 * g_Alpha);
            hwndT = GetDlgItem(hwnd, IDC_ALPHA_TRK2);
            SendMessage(hwndT, TBM_SETRANGE, TRUE, MAKELONG(0, (WORD)(1000)));
            SendMessage(hwndT, TBM_SETPOS, TRUE, (LPARAM)(pos));
            hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%.3f"), g_Alpha);
            SetDlgItemText(hwnd, IDC_ALPHA, sz);

            Button_SetCheck(GetDlgItem(hwnd, IDC_IMAGE_ENABLE), g_fEnableAppImage);
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    EndDialog(hwnd, 1);
                    break;

                case IDC_IMAGE_ENABLE:
                    g_fEnableAppImage =
                    Button_GetCheck(GetDlgItem(hwnd, IDC_IMAGE_ENABLE));
                    UpdateAppImage();
                    break;
            }
            return TRUE;

        case WM_HSCROLL:
            {
                HWND hwndCtrl = (HWND)lParam;

                if(GetDlgItem(hwnd, IDC_ALPHA_TRK2) == hwndCtrl)
                {
                    pos = (int)SendMessage(hwndCtrl, TBM_GETPOS, 0, 0);
                    g_Alpha = (FLOAT)pos / 1000.0F;
                    UpdateAppImage();
                    hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%.3f"), g_Alpha);
                    SetDlgItemText(hwnd, IDC_ALPHA, sz);
                }
                else if(GetDlgItem(hwnd, IDC_XPOS_TRK) == hwndCtrl)
                {
                        pos = (int)SendMessage(hwndCtrl, TBM_GETPOS, 0, 0);
                        g_xPos = (FLOAT)pos / 1000.0F;
                        UpdateAppImage();
                        hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%.3f"), g_xPos);
                        SetDlgItemText(hwnd, IDC_XPOS, sz);
                }
                else if(GetDlgItem(hwnd, IDC_YPOS_TRK) == hwndCtrl)
                {
                        pos = (int)SendMessage(hwndCtrl, TBM_GETPOS, 0, 0);
                        g_yPos = (FLOAT)pos / 1000.0F;
                        UpdateAppImage();
                        hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%.3f"), g_yPos);
                        SetDlgItemText(hwnd, IDC_YPOS, sz);
                }
                else if(GetDlgItem(hwnd, IDC_XSIZE_TRK) == hwndCtrl)
                {
                        pos = (int)SendMessage(hwndCtrl, TBM_GETPOS, 0, 0);
                        g_xSize = (FLOAT)pos / 1000.0F;
                        UpdateAppImage();
                        hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%.3f"), g_xSize);
                        SetDlgItemText(hwnd, IDC_XSIZE, sz);
                }
                else if(GetDlgItem(hwnd, IDC_YSIZE_TRK) == hwndCtrl)
                {
                        pos = (int)SendMessage(hwndCtrl, TBM_GETPOS, 0, 0);
                        g_ySize = (FLOAT)pos / 1000.0F;
                        UpdateAppImage();
                        hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%.3f"), g_ySize);
                        SetDlgItemText(hwnd, IDC_YSIZE, sz);
                }
            }
            return TRUE;

        default:
            return FALSE;
    }
}


/*****************************Private*Routine******************************\
* UpdatePinAlpha
*
\**************************************************************************/
void
UpdatePinAlpha(int strmID)
{
    STRM_PARAM* p = &strParam[strmID];

    if(pMovie && pMovie->m_pMixControl)
        pMovie->m_pMixControl->SetAlpha(strmID, p->Alpha);
}


/*****************************Private*Routine******************************\
* UpdatePinPos
*
\**************************************************************************/
void
UpdatePinPos(int strmID)
{
    STRM_PARAM* p = &strParam[strmID];
    VMR9NormalizedRect r = {p->xPos, p->yPos, p->xPos + p->xSize, p->yPos + p->ySize};

    if(pMovie && pMovie->m_pMixControl)
        pMovie->m_pMixControl->SetOutputRect(strmID, &r);
}


/*****************************Private*Routine******************************\
* TransDlgProc
*
\**************************************************************************/
BOOL CALLBACK
TransDlgProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    TCHAR sz[32];
    HWND hwndT;
    int pos;
    HRESULT hr;

    static int strmID;
    STRM_PARAM* p = &strParam[strmID];

    switch(uMsg)
    {
        case WM_INITDIALOG:

            strmID = (int)lParam;
            p = &strParam[strmID];

            SetWindowText(hwnd, TEXT("Video Transition Properties"));
            ShowWindow(GetDlgItem(hwnd, IDC_IMAGE_ENABLE), SW_HIDE);

            hwndT = GetDlgItem(hwnd, IDC_XPOS_TRK);
            pos = int(10000 * p->xPos) + 10000;
            SendMessage(hwndT, TBM_SETRANGE, TRUE, MAKELONG(0, (WORD)(20000)));
            SendMessage(hwndT, TBM_SETPOS, TRUE, (LPARAM)(pos));
            hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%.3f"), p->xPos);
            SetWindowText(hwndT, sz);
            SetDlgItemText(hwnd, IDC_XPOS, sz);

            pos = int(10000 * p->yPos) + 10000;
            hwndT = GetDlgItem(hwnd, IDC_YPOS_TRK);
            SendMessage(hwndT, TBM_SETRANGE, TRUE, MAKELONG(0, (WORD)(20000)));
            SendMessage(hwndT, TBM_SETPOS, TRUE, (LPARAM)(pos));
            hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%.3f"), p->yPos);
            SetWindowText(hwndT, sz);
            SetDlgItemText(hwnd, IDC_YPOS, sz);

            pos = int(10000 * p->xSize) + 10000;
            hwndT = GetDlgItem(hwnd, IDC_XSIZE_TRK);
            SendMessage(hwndT, TBM_SETRANGE, TRUE, MAKELONG(0, (WORD)(20000)));
            SendMessage(hwndT, TBM_SETPOS, TRUE, (LPARAM)(pos));
            hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%.3f"), p->xSize);
            SetWindowText(hwndT, sz);
            SetDlgItemText(hwnd, IDC_XSIZE, sz);

            pos = int(10000 * p->ySize) + 10000;
            hwndT = GetDlgItem(hwnd, IDC_YSIZE_TRK);
            SendMessage(hwndT, TBM_SETRANGE, TRUE, MAKELONG(0, (WORD)(20000)));
            SendMessage(hwndT, TBM_SETPOS, TRUE, (LPARAM)(pos));
            hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%.3f"), p->ySize);
            SetWindowText(hwndT, sz);
            SetDlgItemText(hwnd, IDC_YSIZE, sz);

            pos = int(10000 * p->Alpha);
            hwndT = GetDlgItem(hwnd, IDC_ALPHA_TRK2);
            SendMessage(hwndT, TBM_SETRANGE, TRUE, MAKELONG(0, (WORD)(10000)));
            SendMessage(hwndT, TBM_SETPOS, TRUE, (LPARAM)(pos));
            hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%.3f"), p->Alpha);
            SetWindowText(hwndT, sz);
            SetDlgItemText(hwnd, IDC_ALPHA, sz);
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                    EndDialog(hwnd, 1);
                    break;
            }
            return TRUE;

        case WM_HSCROLL:
            {
                HWND hwndCtrl = (HWND)lParam;

                if(GetDlgItem(hwnd, IDC_ALPHA_TRK2) == hwndCtrl)
                {
                    pos = (int)SendMessage(hwndCtrl, TBM_GETPOS, 0, 0);
                    p->Alpha = (FLOAT)pos / 10000.0F;
                    UpdatePinAlpha(strmID);
                    hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%.3f"), p->Alpha);
                    SetDlgItemText(hwnd, IDC_ALPHA, sz);
                }
                else if(GetDlgItem(hwnd, IDC_XPOS_TRK) == hwndCtrl)
                {
                        pos = (int)SendMessage(hwndCtrl, TBM_GETPOS, 0, 0);
                        p->xPos = ((FLOAT)pos - 10000.F) / 10000.0F;
                        UpdatePinPos(strmID);
                        hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%.3f"), p->xPos);
                        SetDlgItemText(hwnd, IDC_XPOS, sz);
                }
                else if(GetDlgItem(hwnd, IDC_YPOS_TRK) == hwndCtrl)
                {
                        pos = (int)SendMessage(hwndCtrl, TBM_GETPOS, 0, 0);
                        p->yPos = ((FLOAT)pos - 10000.F) / 10000.0F;
                        UpdatePinPos(strmID);
                        hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%.3f"), p->yPos);
                        SetDlgItemText(hwnd, IDC_YPOS, sz);
                }
                else if(GetDlgItem(hwnd, IDC_XSIZE_TRK) == hwndCtrl)
                {
                        pos = (int)SendMessage(hwndCtrl, TBM_GETPOS, 0, 0);
                        p->xSize = ((FLOAT)pos - 10000.F) / 10000.0F;
                        UpdatePinPos(strmID);
                        hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%.3f"), p->xSize);
                        SetDlgItemText(hwnd, IDC_XSIZE, sz);
                }
                else if(GetDlgItem(hwnd, IDC_YSIZE_TRK) == hwndCtrl)
                {
                        pos = (int)SendMessage(hwndCtrl, TBM_GETPOS, 0, 0);
                        p->ySize = ((FLOAT)pos - 10000.F) / 10000.0F;
                        UpdatePinPos(strmID);
                        hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("%.3f"), p->ySize);
                        SetDlgItemText(hwnd, IDC_YSIZE, sz);
                }
            }
            return TRUE;

        default:
            return FALSE;
    }
}


LRESULT CALLBACK AboutDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            if(wParam == IDOK)
            {
                EndDialog(hWnd, TRUE);
                return TRUE;
            }
            break;
    }
    return FALSE;
}



//----------------------------------------------------------------------------
//  VerifyVMR9
//
//  Verifies that VMR9 COM objects exist on the system and that the VMR9
//  can be instantiated.
//
//  Returns: FALSE if the VMR9 can't be created
//----------------------------------------------------------------------------

BOOL VerifyVMR9(void)
{
    HRESULT hr;

    // Verify that the VMR exists on this system
    IBaseFilter* pBF = NULL;
    hr = CoCreateInstance(CLSID_VideoMixingRenderer9, NULL,
                          CLSCTX_INPROC,
                          IID_IBaseFilter,
                          (LPVOID *)&pBF);
    if(SUCCEEDED(hr))
    {
        pBF->Release();
        return TRUE;
    }
    else
    {
        MessageBox(NULL,
            TEXT("This application requires the VMR-9.\r\n\r\n")

            TEXT("The VMR-9 is not enabled when viewing through a Remote\r\n")
            TEXT(" Desktop session. You can run VMR-enabled applications only\r\n") 
            TEXT("on your local computer.\r\n\r\n")

            TEXT("\r\nThis sample will now exit."),

            TEXT("Video Mixing Renderer (VMR9) capabilities are required"), MB_OK);

        return FALSE;
    }
}