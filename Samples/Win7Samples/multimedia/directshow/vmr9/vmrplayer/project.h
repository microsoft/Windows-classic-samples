//------------------------------------------------------------------------------
// File: project.h
//
// Desc: DirectShow sample code
//       - Master header file that includes all other header files used
//         by the project.  This enables precompiled headers during build.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <stdio.h>
#include <assert.h>

#include <commctrl.h>

#include <dshow.h>

#include "app.h"
#include "vcdplyer.h"
#include "resource.h"

// Common files
#include <dshowutil.h>

void InitStreamParams(int i);
