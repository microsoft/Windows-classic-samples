// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off ATL's hiding of some common and often safely ignored warning messages
#define _ATL_ALL_WARNINGS

#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>
#include <atltypes.h>
#include <atlctl.h>
#include <atlhost.h>
#include <atlstr.h>
#include <atlcoll.h>

#include <wingdi.h>

#include <mfidl.h>
#include <mfapi.h>
#include <mftransform.h>

#include "resource.h"

///////////////////////////////////////////////////////////////////////////////
//
#define IFC(val) { hr = (val); if(FAILED(hr)) { goto Cleanup; } }
#define IFC_WIN32(val) if((val) != 0) { hr = HRESULT_FROM_WIN32(GetLastError()); goto Cleanup; }
#define NEXT_ITER_ON_FAIL(val) { hr = (val);  if(FAILED(hr)) { continue; } }
#define CHECK_ALLOC(val) { if( NULL == (val) ) { hr = E_OUTOFMEMORY; goto Cleanup; } }

using namespace ATL;

inline CAtlString LoadAtlString(UINT nID)
{
    CAtlString str;
    (void)str.LoadString(nID);
    return str;
}
