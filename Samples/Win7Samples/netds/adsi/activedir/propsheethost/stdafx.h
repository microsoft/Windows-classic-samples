//***************************************************************************
//    THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
//    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//    PARTICULAR PURPOSE.
//
//    Copyright Microsoft Corporation. All Rights Reserved.
//***************************************************************************

//***************************************************************************
//
//    File:          stdafx.h
//
//    Description:   Include file for standard system include files, or 
//    project specific include files that are used frequently, but are 
//    changed infrequently.
//
//***************************************************************************

#pragma once
#pragma warning(push,3) 

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here
#include <commctrl.h>
#include <ole2.h>
#include <atlbase.h>
#include <shlobj.h>
#include <dsclient.h>
#include <vector>
#include <string>
#include <adsprop.h>
#include "PropSheetHost.h"
