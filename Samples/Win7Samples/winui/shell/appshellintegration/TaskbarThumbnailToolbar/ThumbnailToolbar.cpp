// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#include <windows.h>
#include <strsafe.h>
#include <shobjidl.h>   // For ITaskbarList3
#include "resource.h"

#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

HINSTANCE g_hInstance = NULL;

HRESULT CreateThumbnailToolbar(HWND hWnd)
{
    ITaskbarList3 *pTaskbarList;
    HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pTaskbarList));
    if (SUCCEEDED(hr))
    {
        hr = pTaskbarList->HrInit();
        if (SUCCEEDED(hr))
        {
            // Figure out what bitmap to use for the thumbnail toolbar buttons - we
            // make the decision based on the system's small icon width. This will make
            // us DPI-friendly.
            struct 
            {
                PCWSTR pbmp;
                int cx;
            } 
            const bitmaps[3] =
            {
                { MAKEINTRESOURCE(IDB_BUTTONIMAGES_96),  16 },
                { MAKEINTRESOURCE(IDB_BUTTONIMAGES_120), 20 },
                { MAKEINTRESOURCE(IDB_BUTTONIMAGES_144), 24 }
            };

            int const cxButton = GetSystemMetrics(SM_CXSMICON);

            int iButtons = 0;
            for (int i = 0; i < ARRAYSIZE(bitmaps); i++)
            {
                if (bitmaps[i].cx <= cxButton)
                {
                    iButtons = i;
                }
            }

            HIMAGELIST himl = ImageList_LoadImage(g_hInstance, bitmaps[iButtons].pbmp,
                bitmaps[iButtons].cx, 0, RGB(255,0,255), IMAGE_BITMAP, LR_CREATEDIBSECTION);
            if (himl)
            {
                hr = pTaskbarList->ThumbBarSetImageList(hWnd, himl);
                if (SUCCEEDED(hr))
                {
                    THUMBBUTTON buttons[3] = {};

                    // First button
                    buttons[0].dwMask = THB_BITMAP | THB_TOOLTIP | THB_FLAGS;
                    buttons[0].dwFlags = THBF_ENABLED | THBF_DISMISSONCLICK;
                    buttons[0].iId = IDTB_BUTTON1;
                    buttons[0].iBitmap = 0;
                    StringCchCopy(buttons[0].szTip, ARRAYSIZE(buttons[0].szTip), L"Button 1");

                    // Second button
                    buttons[1].dwMask = THB_BITMAP | THB_TOOLTIP | THB_FLAGS;
                    buttons[1].dwFlags = THBF_ENABLED | THBF_DISMISSONCLICK;
                    buttons[1].iId = IDTB_BUTTON2;
                    buttons[1].iBitmap = 1;
                    StringCchCopy(buttons[1].szTip, ARRAYSIZE(buttons[1].szTip), L"Button 2");

                    // Third button
                    buttons[2].dwMask = THB_BITMAP | THB_TOOLTIP | THB_FLAGS;
                    buttons[2].dwFlags = THBF_ENABLED | THBF_DISMISSONCLICK;
                    buttons[2].iId = IDTB_BUTTON3;
                    buttons[2].iBitmap = 2;
                    StringCchCopy(buttons[2].szTip, ARRAYSIZE(buttons[2].szTip), L"Button 3");

                    // Set the buttons to be the thumbnail toolbar
                    hr = pTaskbarList->ThumbBarAddButtons(hWnd, ARRAYSIZE(buttons), buttons);
                }
                ImageList_Destroy(himl);
            }
        }

        // It's OK to release ITaskbarList3 here; the thumbnail toolbar will remain.
        pTaskbarList->Release();
    }

    return hr;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static UINT s_uTBBC = WM_NULL;

    if (s_uTBBC == WM_NULL)
    {
        // Compute the value for the TaskbarButtonCreated message
        s_uTBBC = RegisterWindowMessage(L"TaskbarButtonCreated");

        // In case the application is run elevated, allow the
        // TaskbarButtonCreated and WM_COMMAND messages through.
        ChangeWindowMessageFilter(s_uTBBC, MSGFLT_ADD);
        ChangeWindowMessageFilter(WM_COMMAND, MSGFLT_ADD);
    }

    if (message == s_uTBBC)
    {
        // Once we get the TaskbarButtonCreated message, we can create
        // our window's thumbnail toolbar.
        CreateThumbnailToolbar(hWnd);
    }
    else switch (message)
    {
        case WM_COMMAND:
        {
            int const wmId = LOWORD(wParam);
            switch (wmId)
            {
                case IDTB_BUTTON1:
                case IDTB_BUTTON2:
                case IDTB_BUTTON3:
                {
                    WCHAR szMsg[100];
                    StringCchPrintf(szMsg, ARRAYSIZE(szMsg), L"Thumbnail toolbar button clicked, ID=%d", wmId);
                    MessageBox(hWnd, szMsg, L"Application", MB_OK);
                    break;
                }

                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;

                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR /* pszCmdLine */, int nCmdShow)
{
    g_hInstance = hInstance;

    SetProcessDPIAware();   // Set ourselves to be a DPI-aware application

    HRESULT hrInit = CoInitialize(NULL);    // Initialize COM so we can call CoCreateInstance
    if (SUCCEEDED(hrInit))
    {
        WCHAR const szWindowClass[] = L"ThumbnailToolbarWnd";	// The main window class name

        WNDCLASSEX wcex = { sizeof(wcex) };
        wcex.lpfnWndProc = WndProc;
        wcex.hInstance = g_hInstance;
        wcex.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_THUMBNAILTOOLBAR));
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        wcex.lpszMenuName = MAKEINTRESOURCE(IDC_THUMBNAILTOOLBAR);
        wcex.lpszClassName = szWindowClass;

        RegisterClassEx(&wcex);

        WCHAR szTitle[100];
        LoadString(g_hInstance, IDS_APP_TITLE, szTitle, ARRAYSIZE(szTitle));

        HWND hWnd = CreateWindowEx(0, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 400, 400, NULL, NULL, g_hInstance, NULL);
        if (hWnd)
        {
            ShowWindow(hWnd, nCmdShow);
            UpdateWindow(hWnd);

            MSG msg;
            while (GetMessage(&msg, NULL, 0, 0))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        CoUninitialize();
    }

    return 0;
}
