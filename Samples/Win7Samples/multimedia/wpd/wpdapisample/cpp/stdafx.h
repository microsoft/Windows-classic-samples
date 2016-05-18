// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER              // Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0600       // Target Windows Vista or later.
#endif

#ifndef _WIN32_WINNT        // Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0600     // Target Windows Vista or later.
#endif

#ifndef _WIN32_WINDOWS      // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0600 //  Target Windows Vista or later.
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
#endif
#include <stdio.h>
#include <tchar.h>


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <specstrings.h>
#include <commdlg.h>
#include <new>

#ifndef IID_PPV_ARGS
#define IID_PPV_ARGS(ppType) __uuidof(**(ppType)), (static_cast<IUnknown *>(*(ppType)),reinterpret_cast<void**>(ppType))
#endif

#include <PortableDeviceApi.h>  // Include this header for Windows Portable Device API interfaces
#include <PortableDevice.h>     // Include this header for Windows Portable Device definitions

#include "CommonFunctions.h"    // Includes common prototypes for functions used across source files

