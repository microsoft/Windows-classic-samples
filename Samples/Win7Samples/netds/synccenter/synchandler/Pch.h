//////////////////////////////////////////////////////////////////////////////
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Module Name:
//      Pch.h
//
//  Abstract:
//      Include file for standard system include files or project specific
//      include files that are used frequently but are changed infrequently.
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

// Windows header files:
#include <windows.h>
#include <windowsx.h>
#include <SyncMgr.h>            // Sync Center definitions.
#include <shlwapi.h>            // For SHStrDup()
#include <strsafe.h>
#include <shellapi.h>           // For Context menu invocation flags.
#include <shlobj.h>             // For SYNCMGR_OBJECTID GUID values.
#include <assert.h>

// Project header files:
#include "ClassFactory.h"
#include "Dll.h"
#include "Helpers.h"

#define MAX_GUIDSTR_LEN        39

