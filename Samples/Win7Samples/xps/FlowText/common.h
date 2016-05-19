// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

// The following macros define the minimum required platform.  The minimum required platform
// is the earliest version of Windows, Internet Explorer etc. that has the necessary features to run 
// your application.  The macros work by enabling all features available on platform versions up to and 
// including the version specified.

// Specifies that the minimum required platform is Windows 7
#ifndef _WIN32_WINNT            
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#endif

#include <stdio.h>
#include <windows.h>
#include <StrSafe.h>
#include <shlobj.h>

#include <XpsObjectModel.h>
#include <dwrite.h>
#include <new>

