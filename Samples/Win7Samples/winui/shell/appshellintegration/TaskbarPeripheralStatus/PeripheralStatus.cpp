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

#define MAX_PROGRESS_IND     50
#define MAX_PROGRESS_NORMAL  50

HINSTANCE g_hInstance = NULL;

ITaskbarList3 *g_pTaskbarList = NULL;   // careful, COM objects should only be accessed from apartment they are created in
UINT_PTR g_nTimerId = 0;
int g_nProgress = 0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static UINT s_uTBBC = WM_NULL;

    if (s_uTBBC == WM_NULL)
    {
        // Compute the value for the TaskbarButtonCreated message
        s_uTBBC = RegisterWindowMessage(L"TaskbarButtonCreated");

        // In case the application is run elevated, allow the
        // TaskbarButtonCreated message through.
        ChangeWindowMessageFilter(s_uTBBC, MSGFLT_ADD);
    }

    if (message == s_uTBBC)
    {
        // Once we get the TaskbarButtonCreated message, we can call methods
        // specific to our window on a TaskbarList instance. Note that it's
        // possible this message can be received multiple times over the lifetime
        // of this window (if explorer terminates and restarts, for example).
        if (!g_pTaskbarList)
        {
            HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_pTaskbarList));
            if (SUCCEEDED(hr))
            {
                hr = g_pTaskbarList->HrInit();
                if (FAILED(hr))
                {
                    g_pTaskbarList->Release();
                    g_pTaskbarList = NULL;
                }
            }
        }
    }
    else switch (message)
    {
        case WM_COMMAND:
        {
            int const wmId = LOWORD(wParam);
            switch (wmId)
            {
                case IDM_OVERLAY1:
                case IDM_OVERLAY2:
                case IDM_OVERLAY_CLEAR:
                    if (g_pTaskbarList)
                    {
                        // Choose which icon to set as the overlay
                        HICON hIcon = NULL; // for IDM_OVERLAY_CLEAR
                        if (wmId == IDM_OVERLAY1)
                        {
                            hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_OVERLAY1));
                        }
                        else if (wmId == IDM_OVERLAY2)
                        {
                            hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_OVERLAY2));
                        }

                        // Set the window's overlay icon, possibly NULL value
                        g_pTaskbarList->SetOverlayIcon(hWnd, hIcon, NULL);

                        if (hIcon)
                        {
                            // need to cleanup the icon as we no longer need it
                            DestroyIcon(hIcon);
                        }
                    }
                    break;

                case IDM_SIMULATEPROGRESS:
                    // If simulated progress isn't underway, start it
                    if (g_pTaskbarList && g_nTimerId == 0)
                    {
                        g_nTimerId = SetTimer(hWnd, 1, 50, NULL);
                        g_nProgress = 0;
                    }
                    break;

                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;

                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        }

        case WM_TIMER:
            g_nProgress++;
            if (g_nProgress == 1)
            {
                // First time through, so we'll set our progress state to
                // be indeterminate - this simulates a background computation
                // to figure out how much progress we'll need.
                g_pTaskbarList->SetProgressState(hWnd, TBPF_INDETERMINATE);
            }
            else if (g_nProgress == MAX_PROGRESS_IND)
            {
                // Now set the progress state to indicate we have some normal
                // progress to show.
                g_pTaskbarList->SetProgressValue(hWnd, 0, MAX_PROGRESS_NORMAL);
                g_pTaskbarList->SetProgressState(hWnd, TBPF_NORMAL);
            }
            else if (g_nProgress > MAX_PROGRESS_IND)
            {
                if (g_nProgress - MAX_PROGRESS_IND <= MAX_PROGRESS_NORMAL)
                {
                    // Now show normal progress to simulate a background operation
                    g_pTaskbarList->SetProgressValue(hWnd, g_nProgress - MAX_PROGRESS_IND, MAX_PROGRESS_NORMAL);
                }
                else
                {
                    // Progress is done, stop the timer and reset progress state
                    KillTimer(hWnd, g_nTimerId);
                    g_nTimerId = 0;
                    g_pTaskbarList->SetProgressState(hWnd, TBPF_NOPROGRESS);

                    MessageBox(hWnd, L"Done!", L"Progress Complete", MB_OK);
                }
            }
            break;

        case WM_DESTROY:
            if (g_nTimerId)
            {
                KillTimer(hWnd, g_nTimerId);
                g_nTimerId = 0;
            }
            if (g_pTaskbarList)
            {
                g_pTaskbarList->Release();
                g_pTaskbarList = NULL;
            }
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    g_hInstance = hInstance;

    HRESULT hrInit = CoInitialize(NULL);    // Initialize COM so we can call CoCreateInstance
    if (SUCCEEDED(hrInit))
    {
        WCHAR const szWindowClass[] = L"PeripheralStatusWnd";   // The main window class name

        WNDCLASSEX wc = { sizeof(wc) };
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP));
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        wc.lpszMenuName = MAKEINTRESOURCE(IDC_PERIPHERALSTATUS);
        wc.lpszClassName = szWindowClass;

        RegisterClassEx(&wc);

        WCHAR szTitle[100];
        LoadString(hInstance, IDS_APP_TITLE, szTitle, ARRAYSIZE(szTitle));

        HWND hWnd = CreateWindowEx(0, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 400, 400, NULL, NULL, hInstance, NULL);
        if (hWnd)
        {
            ShowWindow(hWnd, nCmdShow);
            UpdateWindow(hWnd);

            // Main message loop:
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
