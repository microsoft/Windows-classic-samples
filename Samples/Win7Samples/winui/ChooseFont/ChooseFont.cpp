//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2008  Microsoft Corporation.  All rights reserved.

#include <windows.h>

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define ID_BUTTON   100
#define ID_CHECKBOX 200
#define ID_LABEL    300

HINSTANCE   g_hInst;     
HWND        g_hwndApp;   // Owner window
HWND        g_hwndLabel; // static text window

// Forward declarations
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitDefaultLF(LOGFONT *lf);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR pszCmdLine, int iCmdShow)
{
    g_hInst = hInstance; // Save our hInstance for later

    MSG         msg;
    WCHAR const szWindowName[] = L"ChooseFont Sample";
    WCHAR const szWindowClass[] = L"ChooseFontSampleWClass";

    WNDCLASS    wc = {}; 
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = WndProc;
    wc.hInstance        = hInstance;
    wc.hIcon            = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszClassName    = szWindowClass;

    RegisterClass(&wc);

    g_hwndApp = CreateWindow(szWindowClass, szWindowName,
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        490, 120, NULL, NULL, hInstance, NULL);
    if (g_hwndApp)
    {
        ShowWindow(g_hwndApp, iCmdShow);
        UpdateWindow(g_hwndApp);
        while(GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);    
        }
    }

    return (int)msg.wParam; 
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static LOGFONT lf = {};

    switch(uMsg)
    {
    case WM_CREATE:
        {
            // Create "Choose Font" button
            CreateWindow(L"button",
                L"Choose Font",
                BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
                20, 20,
                100, 20,
                hwnd, (HMENU)ID_BUTTON,
                g_hInst, NULL);

            // Create "Show all fonts?" checkbox
            CreateWindow(L"button",
                L"Show all fonts?",
                BS_AUTOCHECKBOX | WS_CHILD | WS_VISIBLE,
                20, 45,
                120, 20,
                hwnd, (HMENU)ID_CHECKBOX,
                g_hInst, NULL);

            // Create the static label with our sample text 
            g_hwndLabel =  CreateWindow(L"static",
                L"Some words.",
                SS_CENTER | WS_CHILD | WS_VISIBLE,
                150, 10,
                300, 40,
                hwnd, (HMENU)ID_LABEL,
                g_hInst, NULL);
            InitDefaultLF(&lf);
            break;
        }
    case WM_COMMAND:
        {
            if (LOWORD(wParam) == ID_BUTTON)
            {
                CHOOSEFONT cf = { sizeof(cf) };
                cf.hwndOwner = hwnd;
                cf.lpLogFont = &lf;
                if (BST_CHECKED == IsDlgButtonChecked(hwnd, ID_CHECKBOX))
                {
                    // show all fonts (ignore auto-activation)
                    cf.Flags |= CF_INACTIVEFONTS;
                }

                if (ChooseFont(&cf) == TRUE)
                {
                    HFONT hfont = CreateFontIndirect(&lf);
                    if (hfont)
                    {
                        // delete the old font if being used for the control if there is one
                        HFONT hfontOld = (HFONT)SendMessage(g_hwndLabel, WM_GETFONT, 0, 0);
                        if (hfontOld)
                        {
                            DeleteObject(hfontOld);
                        }
                        SendMessage(g_hwndLabel, WM_SETFONT, (WPARAM)hfont,  MAKELPARAM(TRUE, 0));
                    }
                }
            }
            break;
        }
    case WM_DESTROY:
        {
            // cleanup font resoruces created above
            HFONT hfontOld = (HFONT)SendMessage(g_hwndLabel, WM_GETFONT, 0, 0);
            if (hfontOld)
            {
                DeleteObject(hfontOld);
            }
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void InitDefaultLF(LOGFONT *plf)
{
    HDC hdc = GetDC(NULL);
    ZeroMemory(plf, sizeof(*plf));
    plf->lfCharSet = (BYTE) GetTextCharset(hdc);
    plf->lfOutPrecision = OUT_DEFAULT_PRECIS;
    plf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
    plf->lfQuality = DEFAULT_QUALITY;
    plf->lfPitchAndFamily = DEFAULT_PITCH;
    plf->lfWeight = FW_NORMAL;
    plf->lfHeight = -MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 2);

    ReleaseDC(NULL, hdc);
}