// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

/******************************************************************
*                                                                 *
*  Global Variables                                               *
*                                                                 *
******************************************************************/

HWND g_hwnd = NULL;
INPUT_MESSAGE_SOURCE g_inputSource;

/******************************************************************
*                                                                 *
*  Function Prototypes                                            *
*                                                                 *
******************************************************************/

HRESULT Initialize(HINSTANCE hInstance);
HRESULT OnRender(HDC hdc, const RECT &rcPaint);
void OnResize(UINT width, UINT height);
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

/******************************************************************
*                                                                 *
*  WinMain                                                        *
*                                                                 *
*  Application entrypoint                                         *
*                                                                 *
******************************************************************/

_Use_decl_annotations_
int
WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    HRESULT hr = E_FAIL;

    ZeroMemory(&g_inputSource, sizeof(g_inputSource));

    if (SUCCEEDED(hr = Initialize(hInstance)))
    {
        MSG msg;

        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return SUCCEEDED(hr);
}

/******************************************************************
*                                                                 *
*  InputSourceApp::Initialize                                      *
*                                                                 *
*  This method is used to create and display the application      *
*  window, and provides a convenient place to create any device   *
*  independent resources that will be required.                   *
*                                                                 *
******************************************************************/

HRESULT Initialize(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;
    ATOM atom;

    // Register window class
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = WndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = sizeof(LONG_PTR);
    wcex.hInstance     = hInstance;
    wcex.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName  = NULL;
    wcex.lpszClassName = TEXT("InputSourceApp");
    wcex.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
    atom = RegisterClassEx(&wcex);

    SetProcessDPIAware();

    // Create window
    g_hwnd = CreateWindow(
        TEXT("InputSourceApp"),
        TEXT("Input Source Identification Sample"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        640,
        480,
        NULL,
        NULL,
        hInstance,
        NULL
        );

    if (g_hwnd)
    {
        ShowWindow(
            g_hwnd,
            SW_SHOWNORMAL
            );

        UpdateWindow(
            g_hwnd
            );
    }

    return g_hwnd ? S_OK : E_FAIL;
}


HRESULT OnRender(HDC hdc, const RECT &rcPaint)
{
    WCHAR wzText[512];

    FillRect(hdc, &rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));

    StringCchCopyW(wzText, ARRAYSIZE(wzText), L"Source: ");

    switch(g_inputSource.deviceType)
    {
        case IMDT_UNAVAILABLE:
            StringCchCatW(wzText, ARRAYSIZE(wzText), L"Unavailable\n");
            break;

        case IMDT_KEYBOARD:
            StringCchCatW(wzText, ARRAYSIZE(wzText), L"Keyboard\n");
            break;

        case IMDT_MOUSE:
            StringCchCatW(wzText, ARRAYSIZE(wzText), L"Mouse\n");
            break;

        case IMDT_TOUCH:
            StringCchCatW(wzText, ARRAYSIZE(wzText), L"Touch\n");
            break;

        case IMDT_PEN:
            StringCchCatW(wzText, ARRAYSIZE(wzText), L"Pen\n");
            break;

    }

    StringCchCatW(wzText, ARRAYSIZE(wzText), L"Origin: ");

    switch(g_inputSource.originId)
    {
        case IMO_UNAVAILABLE:
            StringCchCatW(wzText, ARRAYSIZE(wzText), L"Unavailable\n");
            break;

        case IMO_HARDWARE:
            StringCchCatW(wzText, ARRAYSIZE(wzText), L"Hardware\n");
            break;

        case IMO_INJECTED:
            StringCchCatW(wzText, ARRAYSIZE(wzText), L"Injected\n");
            break;

        case IMO_SYSTEM:
            StringCchCatW(wzText, ARRAYSIZE(wzText), L"System\n");
            break;
    }

    DrawText(hdc, wzText, (int)wcslen(wzText), (LPRECT)&rcPaint, DT_TOP | DT_LEFT);

    return S_OK;
}


/******************************************************************
*                                                                 *
*  WndProc                                        *
*                                                                 *
*  This static method handles our app's window messages           *
*                                                                 *
******************************************************************/

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if ((message >= WM_MOUSEFIRST && message <= WM_MOUSELAST) ||
        (message >= WM_KEYFIRST && message <= WM_KEYLAST) ||
        (message >= WM_TOUCH && message <= WM_POINTERWHEEL))
    {
        GetCurrentInputMessageSource(&g_inputSource);
        InvalidateRect(g_hwnd, NULL, FALSE);
    }

    switch (message)
    {
        case WM_PAINT:
        case WM_DISPLAYCHANGE:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                OnRender(hdc, ps.rcPaint);
                EndPaint(hwnd, &ps);
            }
            return 0;

        case WM_DESTROY:
            {
                PostQuitMessage(0);
            }
            return 1;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

