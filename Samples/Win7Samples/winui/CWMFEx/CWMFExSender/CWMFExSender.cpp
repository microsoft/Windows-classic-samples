//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2008  Microsoft Corporation.  All rights reserved.
//
//  CWMFExSender.cpp
//
//          Sending various types of messages to the receiver application.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "..\Include\CWMFExTest.h"


VOID Send(HWND hwnd,
          UINT uMsg,
          WPARAM wParam,
          LPARAM lParam,
          BOOL fExpectedSuccess)
{
    // Attempt to post the message

    BOOL fResult = PostMessage(hwnd, uMsg, wParam, lParam);
    if (fExpectedSuccess) {
        if (!fResult) {
            wprintf(L"\nUnexpected: PostMessage failed with %lu", GetLastError());
        }
    } else {
        if (fResult) {
            wprintf(L"\nUnexpected: PostMessage succeeded");
        }
    }

    // Attempt to send the message

    LRESULT lResult = SendMessage(hwnd, uMsg, wParam, lParam);
    if (fExpectedSuccess) {
        if (CWMFEX_ACK != lResult) {
            wprintf(L"\nUnexpected: SendMessage failed");
        }
    } else {
        if (CWMFEX_ACK == lResult) {
            wprintf(L"\nUnexpected: SendMessage succeeded");
        }
    }
}


int __cdecl wmain (
    int argc, 
    wchar_t *argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    // Find the target window

    HWND hwnd = FindWindow(WNDCLASSNAME, WNDCLASSNAME);
    if (hwnd == NULL) {
        wprintf(L"\nFindWindow failed with %ld", GetLastError());
        return 0;
    }

    // Receiver has taken no action so the message should not make it through
    Send(hwnd, WM_UNDO, 0, 0, FALSE);

    // Notify receiver to allow this message
    // Note that the receiver allows the control message CWMFEX_CONTROL
    SendMessage(hwnd, CWMFEX_CONTROL, (WPARAM)MSGFLT_ALLOW, (LPARAM)WM_UNDO);

    // Receiver has allowed this message, so the message should make it through
    Send(hwnd, WM_UNDO, 0, 0, TRUE);

    // Notify receiver to disallow this message
    SendMessage(hwnd, CWMFEX_CONTROL, (WPARAM)MSGFLT_DISALLOW, (LPARAM)WM_UNDO);

    // Receiver has disallowed this message, so the message should not make it through
    Send(hwnd, WM_UNDO, 0, 0, FALSE);

    // Notify receiver to allow this message
    SendMessage(hwnd, CWMFEX_CONTROL, (WPARAM)MSGFLT_ALLOW, (LPARAM)WM_UNDO);

    // Receiver has allowed this message, so the message should make it through
    Send(hwnd, WM_UNDO, 0, 0, TRUE);

    // Notify receiver to reset its message filter
    SendMessage(hwnd, CWMFEX_CONTROL, (WPARAM)MSGFLT_RESET, (LPARAM)WM_UNDO);

    // Receiver's message filter has been reset, so the message should not make it through
    // Note that, after the rest, the control message CWMFEX_CONTROL, will also not make it through
    Send(hwnd, WM_UNDO, 0, 0, FALSE);

    return 0;
}
