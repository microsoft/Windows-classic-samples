// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <ole2.h>
#include <uiautomation.h>

#include "AnnotatedTextControl.h"

HINSTANCE thisInstance;

LRESULT CALLBACK MainWndProc(_In_ HWND hwnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    static HWND _hwndControl;

    switch (message) 
    {
    case WM_CREATE:
        {
           // Create the child control...
            _hwndControl = AnnotatedTextControl::Create(hwnd, thisInstance);
            if (_hwndControl == NULL)
            {
                PostQuitMessage(0);
            }
            break;
        }

    case WM_SIZE:
        {
            // When the main window gets resized, resize the child window
            RECT rc;
            GetClientRect( hwnd, & rc );
            SetWindowPos( _hwndControl, NULL, rc.left, rc.top, rc.right, rc.bottom, SWP_NOZORDER | SWP_NOACTIVATE );
            break;
        }

    case WM_SETFOCUS:
        {
            SetFocus(_hwndControl);
            break;
        }

    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

bool RegisterMainWndClass(_In_ HINSTANCE instance)
{
    WNDCLASS wc;
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = MainWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = instance;
    wc.hIcon            = NULL;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = L"UiaDocumentProvider";

    if (!RegisterClass(&wc))
    {
        return false;
    }

    return true;
}

int APIENTRY wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ int)
{
    // Ignore the return value because we want to run the program even in the
    // unlikely event that HeapSetInformation fails.
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    HRESULT hr = CoInitialize(NULL);

    if (SUCCEEDED(hr))
    {
        GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR gdiplusToken;
        if (GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) == Ok)
        {
            if (RegisterMainWndClass(instance))
            {
                thisInstance = instance;
                HWND hwnd = CreateWindow(L"UiaDocumentProvider", L"UI Automation Document Provider",
                    WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN,
                    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, instance, NULL);

                if (hwnd != NULL)
                {
                    // The message loop, it will exit when it gets a WM_QUIT message
                    MSG msg;
                    while (GetMessage(&msg, NULL, 0, 0)) 
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }
            }
            GdiplusShutdown(gdiplusToken);
        }
        CoUninitialize();
    }
    return 0;
}

