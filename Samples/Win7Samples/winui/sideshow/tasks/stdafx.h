// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#ifndef STRICT
#define STRICT
#endif

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // some CString constructors will be explicit

// turns off ATL's hiding of some common and often safely ignored warning messages
#define _ATL_ALL_WARNINGS

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#include <atlstr.h>

using namespace ATL;

#include <iostream>

//
// Windows header files
//
#include <windows.h>
#include <strsafe.h>
#include <psapi.h>

//
// Include Windows SideShow Platform API
//
#include <WindowsSideShowApi.h>
#include <WindowsSideShow.h>

//
// Project headers
//
#include "BaseContent.h"
#include "BaseEvents.h"
#include "BaseClient.h"

//
// Constants of configuration variables
//
#define TASK_SECTION        L"Tasks"
#define SHOW_WORK_TASKS     L"ShowWorkTasks"
#define SHOW_FAMILY_TASKS   L"ShowFamilyTasks"

//
// Helper to get the lcation of the task file:
//
extern CString GetTaskFile();
//
// Helper to get the lcation of the configuration file
//
extern CString GetConfigFile();
