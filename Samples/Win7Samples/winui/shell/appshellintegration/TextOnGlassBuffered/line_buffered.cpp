// line.cpp : Defines the entry point for the application.
//

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "stdafx.h"
#include <dwmapi.h>
#include <uxtheme.h>
#include <vssym32.h>
#include "resource.h"

#define MAX_LOADSTRING 100
#define RECT_INCREMENT 20

// Global Variables:
HINSTANCE g_hInst;                                // current instance
wchar_t g_szTitle[MAX_LOADSTRING];              // The title bar text
wchar_t g_szWindowClass[MAX_LOADSTRING];        // the main window class name

// Forward declarations of functions included in this code module:
void MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR /*lpCmdLine*/, int nCmdShow)
{
    SetProcessDPIAware();
    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, g_szTitle, ARRAYSIZE(g_szTitle));
    LoadString(hInstance, IDC_LINE, g_szWindowClass, ARRAYSIZE(g_szWindowClass));
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (InitInstance(hInstance, nCmdShow)) 
    {
        HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LINE));

        // Main message loop:
        MSG message;
        while (GetMessage(&message, NULL, 0, 0)) 
        {
            if (!TranslateAccelerator(message.hwnd, hAccelTable, &message)) 
            {
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
        }
    }
    return 0;
}

//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
void MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex = { sizeof(wcex) };
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LINE));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_LINE);
    wcex.lpszClassName  = g_szWindowClass;

    RegisterClassEx(&wcex);
}

//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    BOOL fPassed = FALSE;
    g_hInst = hInstance; // Store instance handle in our global variable
    
    HWND hWnd = CreateWindow(g_szWindowClass, g_szTitle, WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
    if (hWnd)
    {
        fPassed = TRUE;
        BufferedPaintInit();
        
        ShowWindow(hWnd, nCmdShow);
        UpdateWindow(hWnd);
    }
    
    return fPassed;
}

//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId;
    int wmEvent;
    
    switch (message) 
    {
        case WM_COMMAND:
            wmId    = LOWORD(wParam); 
            wmEvent = HIWORD(wParam); 
            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_ABOUT:
                    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                    break;
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;

        case WM_ERASEBKGND:
            return 1;

        case WM_PAINT:
            // Let's open some theme data, and using BufferedPainting draw different fonts on glass background
            {
                RECT rcClient;
                GetClientRect(hWnd, &rcClient);

                MARGINS marGlassInset = {-1, -1, -1, -1}; // -1 means the whole window
                DwmExtendFrameIntoClientArea(hWnd, &marGlassInset);

                PAINTSTRUCT ps;
                HDC    hdc    = BeginPaint(hWnd, &ps);
                // ControlPanelStyle is declared in AeroStyle.xml
                HTHEME hTheme = OpenThemeData(NULL, L"ControlPanelStyle"); 

                if (hTheme)
                {
                    HDC hdcPaint = NULL;

                    BP_PAINTPARAMS params = { sizeof(BP_PAINTPARAMS) };
                    params.dwFlags        = BPPF_ERASE;

                    HPAINTBUFFER hBufferedPaint = BeginBufferedPaint(hdc, &rcClient, BPBF_TOPDOWNDIB, &params, &hdcPaint);
                    if (hdcPaint)
                    {
                        // Let's start with the simplest GDI default font
                        DTTOPTS DttOpts = {sizeof(DTTOPTS)};
                        DttOpts.dwFlags = DTT_COMPOSITED;
                        DrawText(hdcPaint, L"This is some GDI text in the default font", -1, &rcClient, 0);

                        // Let's look at the same text in the default theme
                        rcClient.top += RECT_INCREMENT;
                        DrawThemeTextEx(hTheme, hdcPaint, 0, 0, L"This is some text in the default font", -1, 0, &rcClient, &DttOpts);

                        // Let's add some text color to add to theming attributes and draw again
                        rcClient.top    += RECT_INCREMENT;
                        DttOpts.dwFlags |= DTT_TEXTCOLOR;
                        DttOpts.crText   = RGB(255, 255, 255);
                        // CPANEL_TASKLINK is declared in VSStyle.h
                        DrawThemeTextEx(hTheme, hdcPaint, CPANEL_TASKLINK, 0, L"This is some text in a themed font", -1, 0, &rcClient, &DttOpts);

                        // Draw text on glass in a selected font
                        DttOpts.dwFlags &= ~DTT_TEXTCOLOR;
                        LOGFONT lgFont;
                        HFONT hFontOld = NULL;
                        if (SUCCEEDED(GetThemeSysFont(hTheme, TMT_CAPTIONFONT, &lgFont)))
                        {
                            HFONT hFont = CreateFontIndirect(&lgFont);
                            hFontOld    = (HFONT) SelectObject(hdcPaint, hFont);
                        }

                        rcClient.top += RECT_INCREMENT;
                        DrawText(hdcPaint, L"This is some GDI text in the selected font", -1, &rcClient, 0);

                        // The same selected font in themed text
                        rcClient.top += RECT_INCREMENT;
                        DrawThemeTextEx(hTheme, hdcPaint, 0, 0, L"This is some text in the selected font", -1, 0, &rcClient, &DttOpts);

                        SelectObject(hdcPaint, hFontOld);
                        rcClient.top += RECT_INCREMENT;
                        DrawText(hdcPaint, L"This is some GDI text in the default font", -1, &rcClient, 0);

                        // Let's add glow to our text attributes
                        DttOpts.dwFlags |= DTT_GLOWSIZE;
                        DttOpts.iGlowSize = 12; // Default value
                        // CompositedWindow::Window is declared in AeroStyle.xml
                        HTHEME hThemeWindow = OpenThemeData(NULL, L"CompositedWindow::Window");
                        if (hThemeWindow != NULL)
                        {
                            GetThemeInt(hThemeWindow, 0, 0, TMT_TEXTGLOWSIZE, &DttOpts.iGlowSize);
                            CloseThemeData(hThemeWindow);
                        }

                        rcClient.top += RECT_INCREMENT;
                        DrawThemeTextEx(hTheme, hdcPaint, 0, 0, L"This is some text with glow in the default font", -1, 0, &rcClient, &DttOpts);

                        // Now some color(theme) and glow
                        DttOpts.dwFlags |= DTT_TEXTCOLOR;
                        DttOpts.crText   = RGB(255, 255, 255);

                        rcClient.top += RECT_INCREMENT;
                        // CPANEL_TASKLINK is declared in VSStyle.h
                        DrawThemeTextEx(hTheme, hdcPaint, CPANEL_TASKLINK, 0, L"This is some text with glow in a themed font", -1, 0, &rcClient, &DttOpts);

                        DttOpts.dwFlags &= ~DTT_TEXTCOLOR;

                        // Find a particular font and draw using this
                        if (SUCCEEDED(GetThemeSysFont(hTheme, TMT_CAPTIONFONT, &lgFont)))
                        {
                            HFONT hFont = CreateFontIndirect(&lgFont);
                            hFontOld    = (HFONT) SelectObject(hdcPaint, hFont);
                        }
                        
                        rcClient.top += RECT_INCREMENT;
                        DrawText(hdcPaint, L"This is some GDI text in the selected font", -1, &rcClient, 0);
                        
                        rcClient.top += RECT_INCREMENT;
                        DrawThemeTextEx(hTheme, hdcPaint, 0, 0, L"This is some text with glow in the selected font", -1, 0, &rcClient, &DttOpts);
                        
                        if (hFontOld)
                        {                        
                            SelectObject(hdcPaint, hFontOld);
                        }
                        EndBufferedPaint(hBufferedPaint, TRUE);
                    }
                    CloseThemeData(hTheme);
                }
                EndPaint(hWnd, &ps);
            }
            break;
        
        case WM_DESTROY:
            PostQuitMessage(0);
            BufferedPaintUnInit();
            break;
        
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM /*lParam*/)
{
    switch (message)
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL)) 
            {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
            break;
    }
    return FALSE;
}