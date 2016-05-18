//////////////////////////////////////////////////////////////////////////
//
// Common include file.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <windowsx.h>

// Media Foundation 
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>

// Shell
#include <shobjidl.h>
#include <shellapi.h> 

// Direct2D
#include <D2d1.h>
#include <D2d1helper.h>

// Misc
#include <strsafe.h>
#include <assert.h>
#include <propvarutil.h>

#include "resource.h"

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}




