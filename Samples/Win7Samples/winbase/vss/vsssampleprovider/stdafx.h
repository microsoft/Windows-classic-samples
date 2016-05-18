/*--

Copyright (C) Microsoft Corporation, 2007

Module Name:

    stdafx.h

Abstract:

    Include file for standard system include files, or project specific include files
    that are used frequently, but are changed infrequently.

Notes:

Revision History:

    10/11/2007  Adding ntverp.h to include the Windows build numbers

--*/

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "resource.h"


#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // some CString constructors will be explicit

// turns off ATL's hiding of some common and often safely ignored warning messages
#define _ATL_ALL_WARNINGS

// ATL
#include <atlbase.h>
#include <atlcom.h>
using namespace ATL;

// STL
#include <new>
#include <string>
#include <vector>
#include <map>

// vss
#include "vss.h"
#include "vsprov.h"
#include "vsadmin.h"

// storage descriptor and identifier definition
#pragma warning(disable : 4201)
#include "winioctl.h"
#pragma warning(default : 4201)

// virtual storage helpers
#include "vstorinterface.h"

// helpers
#include "Utility.h"
#include "EventLogMsgs.h"

// version
#include <ntverp.h>

extern "C" const GUID;

