//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) Microsoft Corporation.  All rights reserved.

// DPITutorial.cpp : Defines the entry point for the application.

#include "stdafx.h"
#include "DPITutorial.h"

// Global Variables
CDPI      g_Dpi; // Helper class for scaling
TCHAR     g_szTitle[MAX_LOADSTRING];
TCHAR     g_szWindowClass[MAX_LOADSTRING];
wchar_t   g_pszString[MAX_LOADSTRING];
bool      g_bRescaleOnDpiChanged = true;
UINT      g_nFontHeight = BUTTON_FONT_HEIGHT;
HBITMAP   g_hBmp[4];
HFONT     g_hButtonFont, g_hTextFont;

//
//  WinMain - Checks for S or P in command line, and sets the DPI awareness as specified
//
extern int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    MSG    msg;
    HACCEL hAccelTable;

    // Read command line and set awareness level if specified:
    //        default = DPI unaware (0)
    //   cmd line + S = System DPI aware (1)
    //   cmd line + P = Per-monitor DPI aware (2)
    //
    // To provide a command-line parameter from within Visual Studio, press Alt+F7 to open the project Property Pages dialog
    // then select the Debugging option in the left pane and add the argument to the Command Arguments field.

    if (FindStringOrdinal(FIND_FROMSTART, lpCmdLine, -1, (LPCTSTR) L"p", -1, TRUE) >= 0)
    {
        g_Dpi.SetAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    }
    else if (FindStringOrdinal(FIND_FROMSTART, lpCmdLine, -1, (LPCTSTR) L"s", -1, TRUE) >= 0)
    {
        g_Dpi.SetAwareness(PROCESS_SYSTEM_DPI_AWARE);
    }

    // Initialize global strings
    wsprintf(g_szTitle, L"DPI Awareness Tutorial");
    wsprintf(g_szWindowClass, L"DPITUTORIAL");

    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DPITUTORIAL));

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  MyRegisterClass - registers the window class
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DPITUTORIAL));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) GetStockObject(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDC_DPITUTORIAL);
    wcex.lpszClassName = g_szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   InitInstance:
//     Initializes the DPI
//     Creates the main window
//     Loads the bitmap resources
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND     hWnd, hWndButton;
    HMONITOR hMonitor;
    POINT    pt;
    UINT     dpix = 0, dpiy = 0;
    HRESULT  hr = E_FAIL;

    // Get the DPI for the main monitor, and set the scaling factor
    pt.x = 1;
    pt.y = 1;
    hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    hr = GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpix, &dpiy);

    if (hr != S_OK)
    {
        MessageBox(NULL, (LPCWSTR) L"GetDpiForMonitor failed", (LPCWSTR) L"Notification", MB_OK);
        return FALSE;
    }
    g_Dpi.SetScale(dpix);

    // Create main window and pushbutton window
    hWnd = CreateWindow(g_szWindowClass, g_szTitle, WS_OVERLAPPEDWINDOW, g_Dpi.Scale(100), g_Dpi.Scale(100),
        g_Dpi.Scale(WINDOW_WIDTH), g_Dpi.Scale(WINDOW_HEIGHT), NULL, NULL, hInstance, NULL);
    
    if (!hWnd)
    {
        return FALSE;
    }
    
    hWndButton = CreateWindow(L"BUTTON", L"&Rescale", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, hWnd, (HMENU) IDM_RESCALE_NOW, hInstance, NULL);

    // Load bitmaps for each scaling factor
    g_hBmp[0] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP100));
    g_hBmp[1] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP125));
    g_hBmp[2] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP150));
    g_hBmp[3] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP200));

    // Create fonts for button and window text output
    CreateFonts(hWnd);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

//
//  CreateScaledFont - Create a font close to the specified height
//
HFONT CreateScaledFont(int g_nFontHeight)
{
    LOGFONT lf;
    HFONT   hFont;
    
    memset(&lf, 0, sizeof(lf));
    lf.lfQuality = CLEARTYPE_QUALITY;
    lf.lfHeight = -g_nFontHeight;
    hFont = CreateFontIndirect(&lf);
    return (hFont);
}

//
//  CreateFonts creates fonts for button and window text output, appropriate for the DPI
//
void CreateFonts(HWND hWnd)
{
    if (g_hButtonFont != NULL)
    {
        DeleteObject(g_hButtonFont);
        g_hButtonFont = NULL;
    }

    g_hButtonFont = CreateScaledFont(g_Dpi.Scale(BUTTON_FONT_HEIGHT));
    if (g_hButtonFont == NULL)
    {
        MessageBox(hWnd, (LPCWSTR) L"Failed to create scaled font for button", (LPCWSTR) L"Notification", MB_OK);
    }

    if (g_hTextFont != NULL)
    {
        DeleteObject(g_hTextFont);
        g_hTextFont = NULL;
    }

    g_hTextFont = CreateScaledFont(g_Dpi.Scale(g_nFontHeight));
    if (g_hTextFont == NULL)
    {
        MessageBox(hWnd, (LPCWSTR) L"Failed to create scaled font for text", (LPCWSTR) L"Notification", MB_OK);
    }

    return;
}

// 
// RenderWindow - Draw the elements of the window: background color, rectangle (frame for bitmap),
//   bitmap (selected from resources based on DPI), and text (using a font based on the DPI)
//   The GetAwareness return value is used as an index into the string & color arrays
//
void RenderWindow(HWND hWnd)
{
    HDC         hdc, hdcMem;
    HBRUSH      hBrush;
    PAINTSTRUCT ps;
    RECT        rcText, rcWin, rcClient;
    POINT       ptOffset;
    LPCWSTR     awareText[3] = { L"DPI Unaware", L"System DPI Aware", L"Per-Monitor DPI Aware" };
    COLORREF    colors[3] = { RGB(220, 120, 120), RGB(250, 250, 210), RGB(150, 250, 150) };
    BITMAP      bmp;
    HBITMAP     hbmpTemp, hbmpScaled;
    HWND        hWndButton;
    UINT        nPad;

    GetWindowRect(hWnd, &rcWin);
    GetClientRect(hWnd, &rcClient);
    nPad = g_Dpi.Scale(PADDING);

    hdc = BeginPaint(hWnd, &ps);
    SetBkMode(hdc, TRANSPARENT);

    // Render client area background
    hBrush = CreateSolidBrush(colors[g_Dpi.GetAwareness()]);
    FillRect(hdc, &rcClient, hBrush);

    // Select the appropriate bitmap for the scaling factor and load it into memory
    if (g_Dpi.GetAwareness() == PROCESS_DPI_UNAWARE)
    {
        hbmpScaled = g_hBmp[0];
    }
    else
    {
        switch (g_Dpi.GetScale())
        {
            case 125:
                hbmpScaled = g_hBmp[1];
                break;

            case 150:
                hbmpScaled = g_hBmp[2];
                break;

            case 200:
                hbmpScaled = g_hBmp[3];
                break;

            default:
                hbmpScaled = g_hBmp[0];
			    break;
	    }
    }
    hdcMem = CreateCompatibleDC(NULL);
    hbmpTemp = SelectBitmap(hdcMem, hbmpScaled);
    GetObject(hbmpScaled, sizeof(bmp), &bmp);

    // Render a rectangle to frame the bitmap in the lower-right of the window
    ptOffset.x = rcClient.right - bmp.bmWidth - nPad;
    ptOffset.y = rcClient.bottom - bmp.bmHeight - nPad;
    Rectangle(hdc, ptOffset.x - nPad / 2, ptOffset.y - nPad / 2, rcClient.right - nPad / 2, rcClient.bottom - nPad / 2);

    // Render bitmap
    BitBlt(hdc, ptOffset.x, ptOffset.y, bmp.bmWidth, bmp.bmHeight, hdcMem, 0, 0, SRCCOPY);
    SelectBitmap(hdcMem, hbmpTemp);
    DeleteDC(hdcMem);

    // Render button
    SelectObject(hdc, g_hButtonFont);
    hWndButton = GetWindow(hWnd, GW_CHILD);
    SetWindowPos(hWndButton, HWND_TOP, rcClient.left + nPad, rcClient.bottom - g_Dpi.Scale(BUTTON_HEIGHT) - nPad,
        g_Dpi.Scale(BUTTON_WIDTH), g_Dpi.Scale(BUTTON_HEIGHT), SWP_SHOWWINDOW);
    SendMessage(hWndButton, WM_SETFONT, (WPARAM) g_hButtonFont, TRUE);
    UpdateWindow(hWndButton);
    EnableWindow(hWndButton, ((g_Dpi.GetAwareness() == PROCESS_PER_MONITOR_DPI_AWARE) && (g_bRescaleOnDpiChanged == false)) ? TRUE : FALSE);

    // Render Text
    SelectObject(hdc, g_hTextFont);

    rcText.left = rcClient.left + nPad;
    rcText.top = rcClient.top + nPad;
    wsprintf(g_pszString, L"%s Window", awareText[g_Dpi.GetAwareness()]);
    DrawText(hdc, g_pszString, -1, &rcText, DT_CALCRECT | DT_LEFT | DT_TOP);
    DrawText(hdc, g_pszString, -1, &rcText, DT_LEFT | DT_TOP);

    rcText.top = rcText.bottom + nPad;
    wsprintf(g_pszString, L"Scaling: %i%%", g_Dpi.GetScale());
    DrawText(hdc, g_pszString, -1, &rcText, DT_CALCRECT | DT_LEFT | DT_TOP);
    DrawText(hdc, g_pszString, -1, &rcText, DT_LEFT | DT_TOP);

    rcText.top = rcText.bottom + nPad;
    wsprintf(g_pszString, L"Position: (%i, %i) to (%i, %i)", rcWin.left, rcWin.top, rcWin.right, rcWin.bottom);
    DrawText(hdc, g_pszString, -1, &rcText, DT_CALCRECT | DT_LEFT | DT_TOP);
    DrawText(hdc, g_pszString, -1, &rcText, DT_LEFT | DT_TOP);

    rcText.top = rcText.bottom + nPad;
    wsprintf(g_pszString, L"Size: %i x %i", RectWidth(rcWin), RectHeight(rcWin));
    DrawText(hdc, g_pszString, -1, &rcText, DT_CALCRECT | DT_LEFT | DT_TOP);
    DrawText(hdc, g_pszString, -1, &rcText, DT_LEFT | DT_TOP);

    rcText.top = rcText.bottom + nPad;
    wsprintf(g_pszString, L"Font height: %i (%i unscaled)", g_Dpi.Scale(g_nFontHeight), g_nFontHeight);
    DrawText(hdc, g_pszString, -1, &rcText, DT_CALCRECT | DT_LEFT | DT_TOP);
    DrawText(hdc, g_pszString, -1, &rcText, DT_LEFT | DT_TOP);

    // Cleanup
    EndPaint(hWnd, &ps);
    DeleteObject(hBrush);
    DeleteDC(hdc);
    return;
}

//
//  WndProc:  Message handler
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT      lRet = 0;
    HWND         hWndButton;
    HMENU        hMenu;
    MENUITEMINFO mii;
    LPRECT       lprcNewScale;
    static RECT  rcNewScale;

    switch (message)
    {
        case WM_CREATE:
            // Initialize window rect
            GetWindowRect(hWnd, &rcNewScale);

            // Initialize menu bar
            hMenu = GetMenu(hWnd);

            // Set the default (selected) state of the menu item
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_STATE;
            mii.fState = (g_bRescaleOnDpiChanged == true) ? MFS_CHECKED : MFS_UNCHECKED;
            SetMenuItemInfo(hMenu, IDM_RESCALE_ON_DPICHANGED, FALSE, &mii);

            // Disable the "Rescale on DPICHANGED" menu item unless the app is DPI Aware
            if (g_Dpi.GetAwareness() == PROCESS_PER_MONITOR_DPI_AWARE)
            {
                break;
            }

            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_STATE;
            mii.fState = MFS_DISABLED;
            SetMenuItemInfo(hMenu, IDM_RESCALE_ON_DPICHANGED, FALSE, &mii);
            break;

        case WM_DPICHANGED:
            // This message tells the program that most of its window is on a monitor with a new DPI.
            // The wParam contains the new DPI, and the lParam contains a rect which defines the window
            // rectangle scaled to the new DPI.

            g_Dpi.SetScale(LOWORD(wParam));  // Set the new DPI, retrieved from the wParam

            if (g_Dpi.GetAwareness() != PROCESS_PER_MONITOR_DPI_AWARE)
            {
                break;
            }

            lprcNewScale = (LPRECT) lParam;  // Get the window rectangle scaled for the new DPI, retrieved from the lParam
            CopyRect(&rcNewScale, lprcNewScale);  // Save the rectangle for the on-demand rescale option (IDM_RESCALE_NOW)

            if (g_bRescaleOnDpiChanged)
            {
                // For the new DPI: resize the window, select new fonts, and re-render window content
                SetWindowPos(hWnd,
                    HWND_TOP,
                    lprcNewScale->left,
                    lprcNewScale->top,
                    RectWidth(*lprcNewScale),
                    RectHeight(*lprcNewScale),
                    SWP_NOZORDER | SWP_NOACTIVATE);
                CreateFonts(hWnd);
                RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
            }
            break;

        case WM_PAINT:
            RenderWindow(hWnd);
            break;

        case WM_EXITSIZEMOVE:
            if (g_bRescaleOnDpiChanged)
            {
                // Refresh the window to display its new position
                RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
            }
            break;

        case WM_COMMAND:
            // Respond to user input from the keyboard or menu options

            switch (LOWORD(wParam))
            {
                case IDM_RESCALE_NOW:  // The "Rescale" pushbutton was clicked
                    if (!g_bRescaleOnDpiChanged)
                    {
                        // For the new DPI: resize the window, select new fonts, and re-render window content
                        SetWindowPos(hWnd,
                            HWND_TOP,
                            0,  // Position origin ignored (per SWP_NOMOVE)
                            0,  // Position origin ignored (per SWP_NOMOVE)
                            RectWidth(rcNewScale),
                            RectHeight(rcNewScale),
                            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
                        CreateFonts(hWnd);
                        RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
                        SetFocus(hWnd);  // Return focus from pushbutton to main window
                    }
                    break;

                case IDM_RESCALE_ON_DPICHANGED:  // The "Rescale on DPICHANGED" menu item was selected
                    g_bRescaleOnDpiChanged = !g_bRescaleOnDpiChanged;

                    hMenu = GetMenu(hWnd);  // Toggle the selected/unselected state of the menu item
                    mii.cbSize = sizeof(MENUITEMINFO);
                    mii.fMask = MIIM_STATE;
                    mii.fState = (g_bRescaleOnDpiChanged) ? MFS_CHECKED : MFS_UNCHECKED;
                    SetMenuItemInfo(hMenu, IDM_RESCALE_ON_DPICHANGED, FALSE, &mii);

                    hWndButton = GetWindow(hWnd, GW_CHILD);  // Toggle the enabled/disabled state of the pushbutton
                    EnableWindow(hWndButton, (g_bRescaleOnDpiChanged == false) ? TRUE : FALSE);
                    RenderWindow(hWnd);
                    break;

                case IDM_FONT_INCREASE:
                    g_nFontHeight += 5;
                    if (g_nFontHeight > WINDOW_HEIGHT / 5)
                    {
                        g_nFontHeight = WINDOW_HEIGHT / 5;
                    }
                    CreateFonts(hWnd);
                    RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
                    break;

                case IDM_FONT_DECREASE:
                    g_nFontHeight -= 5;
                    if (g_nFontHeight < 10)
                    {
                        g_nFontHeight = 10;
                    }
                    CreateFonts(hWnd);
                    RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
                    break;

                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;

                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;

        case WM_DESTROY:
            DeleteObject(g_hButtonFont);
            DeleteObject(g_hTextFont);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return lRet;
}