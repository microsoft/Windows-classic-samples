// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#ifndef STRICT
#define STRICT
#endif
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

#pragma warning(disable : 4100) // unreferenced formal parameter
#include <atlbase.h>
#include <windows.h>
#include <atlcom.h>
#include <assert.h>
#include <stdio.h>
#define RDCAssert assert
#include "DebugHresult.h"
