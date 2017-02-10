//------------------------------------------------------------------------------
//
// File: MFPlayer.h
// Main header for the application
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//------------------------------------------------------------------------------

#pragma once

#define NTDDI_VERSION NTDDI_WIN7
#define _WIN32_WINNT _WIN32_WINNT_WIN7

#include <new>

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfplay.h>
#include <propkey.h>
#include <strsafe.h>
#include <assert.h>
#include <shlwapi.h>

#include "resource.h"

#include "slider.h"
#include "ThemedButton.h"
#include "Player2.h"
#include "MainDialog.h"


inline LONG Width(const RECT& r)
{
    return r.right - r.left;
}

inline LONG Height(const RECT& r)
{
    return r.bottom - r.top;
}

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}


inline HINSTANCE GetInstance()
{
    return (HINSTANCE)GetModuleHandle(NULL); 
}

// Include the v6 common controls in the manifest
#pragma comment(linker, \
    "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' "\
    "processorArchitecture='*' "\
    "publicKeyToken='6595b64144ccf1df' "\
    "language='*'\"")
