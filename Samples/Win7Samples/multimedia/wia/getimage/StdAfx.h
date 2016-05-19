/*++

Copyright (c) Microsoft Corporation. All rights reserved.

--*/

#ifndef __STDAFX__
#define __STDAFX__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		
#endif //WIN32_LEAN_AND_MEAN

#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <objbase.h>
#include <atlbase.h>
#include <wia.h>
#include <sti.h>
#include <gdiplus.h>

#define COUNTOF(x) ( sizeof(x) / sizeof(*x) )

#define DEFAULT_STRING_SIZE 256

extern HINSTANCE g_hInstance;

//{{AFX_INSERT_LOCATION}}

#endif //__STDAFX__

