// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <iostream>

//
// Windows header files
//
#include <windows.h>
#include <strsafe.h>
#include <shlobj.h>

//
// Include Windows SideShow Platform API
//
#include <WindowsSideShowApi.h>
#include <WindowsSideShow.h>

//
// Project headers
//
#include "resource.h"
#include "BaseContent.h"
#include "BaseEvents.h"
#include "BaseClient.h"

//
// CONTENT_ID ranges used to differentiate content types
//
#define CID_XMLIMAGE_FIRST  1
#define CID_RAWIMAGE_FIRST  10001

