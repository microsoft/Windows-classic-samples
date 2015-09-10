// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef UNICODE
#define UNICODE
#endif

// Windows Header Files:
#include <windows.h>
#include <comdef.h>

// C RunTime Header Files
#include <stdlib.h>
#include <strsafe.h>

#pragma hdrstop

/******************************************************************
 *
 *  Global Variables
 *
 ******************************************************************/

#define TIMERID_NOCOALSCING         (0)
#define TIMERID_COALESCING          (1)

#define TIMERID_UPDATE_SCREEN       (100)

#define TIMER_ELAPSE                ((DWORD)(15.6 * 2))
#define TIMER_TOLERANCE             (80)

#define TIMER_CONTIGUOUS_RUN        (100)

#define TIMER_AUTO_REFRESH_ELAPSE  (5 * 1000)

HWND ghwnd;
HINSTANCE ghInstance;

struct TimerRec {
    LONGLONG lLast;
    LONGLONG lCount;
    LONGLONG lElapsedMin;
    LONGLONG lElapsedMax;
    LONGLONG lSum;
};

TimerRec gTimerRec[2] = { 0, };

LONGLONG glFrequency;


/******************************************************************
 *
 *  Function Prototypes
 *
 ******************************************************************/
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


/******************************************************************
 *
 *  GetPerformanceCounter
 *
 *  Returns the high refolustion time in milliseconds
 *  (resolution of tick counts is a bit too coarse
 *   for the purpose here)
 *
 ******************************************************************/
LONGLONG GetPerformanceCounter()
{
    LARGE_INTEGER t;

    QueryPerformanceCounter(&t);

    return (t.QuadPart * 1000 + 500) / glFrequency;
}


/******************************************************************
 *
 *  Initialize
 *
 *  This method is used to create and display the application
 *  window, and provides a convenient place to create any device
 *  independent resources that will be required.
 *
 ******************************************************************/
BOOL Initialize()
{
    WNDCLASSEX wcex;
    ATOM atom;
    LARGE_INTEGER freq;

    // Prepare the high resolution performance counter.
    SetThreadAffinityMask(GetCurrentThread(), 0);
    if (!QueryPerformanceFrequency(&freq)) {
        return FALSE;
    }
    glFrequency = freq.QuadPart;

    // Initialize the result array.
    gTimerRec[0].lElapsedMin = gTimerRec[1].lElapsedMin = MAXLONGLONG;


    // Register window class
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = WndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = sizeof(LONG_PTR);
    wcex.hInstance     = ghInstance;
    wcex.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName  = NULL;
    wcex.lpszClassName = TEXT("TimerApp");
    wcex.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    atom = RegisterClassEx(&wcex);
    if (atom == 0) {
        return FALSE;
    }

    SetProcessDPIAware();

    // Create & prepare the main window
    ghwnd = CreateWindow(
        (LPCTSTR)atom,
        TEXT("Coalescable Timer Sample"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        600,
        400,
        NULL,
        NULL,
        ghInstance,
        NULL);

    if (ghwnd == NULL) {
        return FALSE;
    }

    ShowWindow(ghwnd, SW_SHOWNORMAL);
    UpdateWindow(ghwnd);

    return TRUE;
}

__inline LONGLONG MakeItLookNormal(_In_ const LONGLONG l)
{
    if (l == MAXLONGLONG) {
        // If the value is initialized but not set yet,
        // give it some reasonable sane value.
        return 0;
    }

    return l;
}

__inline double Average(_In_ const LONGLONG sum, _In_ const LONGLONG count)
{
    if (count == 0) {
        return 0;
    }
    return (double)sum / count;
}

/******************************************************************
 *
 *  OnPaint
 *
 ******************************************************************/
void OnPaint(_In_ HDC hdc, _In_ PRECT prcPaint)
{
    HRESULT hr;
    WCHAR wzText[1024];

    FillRect(hdc, prcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));

    const TimerRec& nocoal = gTimerRec[TIMERID_NOCOALSCING];
    const TimerRec& coal   = gTimerRec[TIMERID_COALESCING];

    hr = StringCchPrintfExW(wzText,
            ARRAYSIZE(wzText),
            NULL,
            NULL,
            STRSAFE_NULL_ON_FAILURE,
            L"Timer non-coalesced  Min = %I64d, Avg = %.1f, Max = %I64d, (%I64d / %I64d)\n\n"
            L"Timer coalesced      Min = %I64d, Avg = %.1f, Max = %I64d, (%I64d / %I64d)\n\n"
            L"[Elapse = %dms, Coaclescing tolerance = %dms]\n\n"
            L"Hit space to turn off the monitor",

            MakeItLookNormal(nocoal.lElapsedMin),
            Average(nocoal.lSum, nocoal.lCount),
            nocoal.lElapsedMax,
            nocoal.lSum,
            nocoal.lCount,

            MakeItLookNormal(coal.lElapsedMin),
            Average(coal.lSum, coal.lCount),
            coal.lElapsedMax,
            coal.lSum,
            coal.lCount,

            TIMER_ELAPSE,
            TIMER_TOLERANCE);

    if (SUCCEEDED(hr)) {
        DrawText(hdc, wzText, -1, prcPaint, DT_TOP | DT_LEFT);
    }
}

/********************************************************************
 *
 *  TimerHandler
 *
 *  Handles both coalesced and non-coalesced timers, and keeps
 *  the record of the effective elapsed time.
 *  Switches the coalesced modes periodically for comparison.
 *
 *********************************************************************/
void CALLBACK TimerHandler(_In_ HWND hwnd, _In_ UINT_PTR idEvent)
{
    if (idEvent >= ARRAYSIZE(gTimerRec)) {
        return;
    }

    TimerRec& t = gTimerRec[idEvent];

    LONGLONG lTime = GetPerformanceCounter();
    LONGLONG lElapsed = lTime - t.lLast;
    t.lElapsedMin = min(t.lElapsedMin, lElapsed);
    t.lElapsedMax = max(t.lElapsedMax, lElapsed);
    t.lLast = lTime;
    ++t.lCount;
    t.lSum += lElapsed;

    if (t.lCount % TIMER_CONTIGUOUS_RUN == 0) {
        // Now, let's switch the timer types.
        // First, kill the current timer.
        KillTimer(hwnd, idEvent);

        // Reverse the timer id.
        idEvent = !idEvent;

        /*
         * Setting new timer - switching the coalescing mode.
         *
         * Note that the coalesced timers may be fired
         * together with other timers that are readied during the
         * coalescing tolerance.  As such, in an environment
         * that has a lot of short timers may only see a small or no
         * increase in the average time.
         * If that's the case, try running this sample with
         * the minimum number of processes.
         */
        gTimerRec[idEvent].lLast = GetPerformanceCounter();
        SetCoalescableTimer(hwnd, idEvent, TIMER_ELAPSE, NULL,
                            idEvent == TIMERID_NOCOALSCING ? TIMERV_NO_COALESCING : TIMER_TOLERANCE);
    }
}

/******************************************************************
 *
 *  WndProc
 *
 ******************************************************************/
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CREATE:
        // Start with non-coalescable timer.
        // Later we switch to the coalescable timer.
        gTimerRec[TIMERID_NOCOALSCING].lLast = GetPerformanceCounter();
        if (!SetCoalescableTimer(hwnd, TIMERID_NOCOALSCING, TIMER_ELAPSE, NULL, TIMERV_NO_COALESCING)) {
            return -1;
        }

        // Let's update the screen periodically.
        SetTimer(hwnd, TIMERID_UPDATE_SCREEN, TIMER_AUTO_REFRESH_ELAPSE, NULL);
        return 0;

    case WM_TIMER:
        if (wParam < ARRAYSIZE(gTimerRec)) {

            TimerHandler(hwnd, (UINT_PTR)wParam);

        } else if (wParam == TIMERID_UPDATE_SCREEN) {
            // Periodically update the results.
    case WM_MOUSEMOVE:  // Tricky: also update the screen at every mouse move.
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;

    case WM_PAINT:
    case WM_DISPLAYCHANGE:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            OnPaint(hdc, &ps.rcPaint);
            EndPaint(hwnd, &ps);
        }
        return 0;

    case WM_KEYDOWN:
        if (wParam == VK_SPACE) {
            // Space key to power down the monitor.
            DefWindowProc(GetDesktopWindow(), WM_SYSCOMMAND, SC_MONITORPOWER, 2);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 1;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}


/******************************************************************
 *
 *  WinMain
 *
 ******************************************************************/
int CALLBACK WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE /*hPrevInstance*/,
    _In_ LPSTR /*lpCmdLine*/,
    _In_ int /*nCmdShow*/)
{
    MSG msg;

    ghInstance = hInstance;

    if (!Initialize()) {
        return 0;
    }

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

