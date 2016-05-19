// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include <new>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <commdlg.h>

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// ----------------------------------------------------------------------
// These macros are for packaging float values (such as volume levels) into an LPARAM
// so they can be sent by value in a message.  The float is 5 digits of precision
#define FLOAT2LPARAM(fl)        static_cast<LPARAM>(static_cast<int>((fl) * 100000.f))
#define LPARAM2FLOAT(l)         (static_cast<float>(l) / 100000.f)

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

#define WM_APP_SESSION_DUCKED           WM_APP
#define WM_APP_SESSION_UNDUCKED         WM_APP+1
#define WM_APP_GRAPHNOTIFY              WM_APP+2
#define WM_APP_SESSION_VOLUME_CHANGED   WM_APP+3    // wParam = Mute State, lParam = (float) new volume 0.0 - 1.0
