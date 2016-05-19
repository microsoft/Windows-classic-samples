
//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2008  Microsoft Corporation.  All rights reserved.
//
//  CWMFExReceiver.cpp
//
//          Using the ChangeWindowMessageFilterEx function to demonstrate its capabilities.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "..\Include\CWMFExTest.h"

#define MSGFLTINFO_ERROR    (-1)

DWORD ChangeFilter(HWND hwnd, UINT uMsg, DWORD dwMsgFlt)
{
    BOOL                fSuccess = FALSE;
    DWORD               dwMsgFltInfo = MSGFLTINFO_NONE;
    CHANGEFILTERSTRUCT  ChangeFilterStruct;

    ChangeFilterStruct.cbSize = sizeof(CHANGEFILTERSTRUCT);
    fSuccess = ChangeWindowMessageFilterEx(hwnd, uMsg, dwMsgFlt, &ChangeFilterStruct);
    dwMsgFltInfo = ChangeFilterStruct.ExtStatus;

    if (!fSuccess) {
        wprintf(L"\nChangeWindowMessageFilterEx failed with %lu", GetLastError());
        dwMsgFltInfo = MSGFLTINFO_ERROR;
    }

    return dwMsgFltInfo;
}

LRESULT CALLBACK CWMFExWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg) {
        case CWMFEX_CONTROL:
            return ChangeFilter(hwnd, (UINT)lParam, (DWORD)wParam);
        case WM_CLOSE:
            PostQuitMessage(0);
            break;
        default:
            break;
    }
    DefWindowProc(hwnd, uMsg, wParam, lParam);
    return CWMFEX_ACK;
}

HWND CreateTheWindow(HINSTANCE hInstance)
{
    WNDCLASS    wndclass;
    HWND        hwnd = NULL;

    ZeroMemory(&wndclass, sizeof(WNDCLASS));
    wndclass.lpfnWndProc = CWMFExWindowProc;
    wndclass.hInstance = hInstance;
    wndclass.lpszClassName = WNDCLASSNAME;
    wndclass.hbrBackground = (HBRUSH)COLOR_WINDOW;

    if (0 == RegisterClass(&wndclass)) {
        wprintf(L"\nRegisterClass failed with %lu", GetLastError());
        return NULL;
    }

    hwnd = CreateWindow(WNDCLASSNAME,
                        WNDCLASSNAME,
                        WS_BORDER | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        500,
                        500,
                        NULL,
                        NULL,
                        hInstance,
                        NULL);
    if (hwnd == NULL) {
        wprintf(L"\nCreateWindow failed with %lu", GetLastError());
    }

    return hwnd;
}

int __cdecl wmain (
    int argc, 
    wchar_t *argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    HWND hwnd = NULL;
    MSG msg;

    hwnd = CreateTheWindow(GetModuleHandle(NULL));
    if (hwnd == NULL) {
        return 0;
    }

    // Allow control message CWMFEX_CONTROL so that sender can control this window

    if (MSGFLTINFO_ERROR == ChangeFilter(hwnd, CWMFEX_CONTROL, MSGFLT_ALLOW)) {
        DestroyWindow(hwnd);
        return 0;
    }

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
