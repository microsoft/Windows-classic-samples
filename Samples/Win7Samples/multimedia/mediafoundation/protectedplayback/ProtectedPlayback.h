//////////////////////////////////////////////////////////////////////////
// BasicPlayback.h: Main header for the application.
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

#define WINVER 0x0501       
#define _WIN32_WINNT 0x0501

// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <tchar.h>
#include <commdlg.h> // OpenFile dialog
#include <assert.h>

// Media Foundation headers
#include <mfapi.h>
#include <mfobjects.h>
#include <mfidl.h>
#include <mferror.h>
#include <nserror.h>  // More DRM errors.

// EVR headers
#include <evr.h>

// Safe string functions
#include <strsafe.h>


#define USE_LOGGING

#include "common.h"
using namespace MediaFoundationSamples;

#define CHECK_HR(hr) IF_FAILED_GOTO(hr, done)


#include "resource.h"
#include "webhelper.h"
#include "ContentEnabler.h"
#include "Player.h"
#include "logging.h"


