/*++

Copyright (c) 2003  Microsoft Corporation

Module Name: IsapiTools.h

Abstract:

    Master header file for ISAPI tools library

Author:

    ISAPI developer (Microsoft employee), October 2002

--*/

#ifndef _isapitools_h
#define _isapitools_h

#include <windows.h>

//
// ISAPIASSERT macro definition
//

#ifdef _DEBUG
#define ISAPI_ASSERT(expr) {if (!(expr)){IsapiAssert(#expr,__FILE__,__LINE__);}};
#else
#define ISAPI_ASSERT(expr) ((VOID)0)
#endif  // _DEBUG

//
// Global module name
//

extern
CHAR    g_szModuleName[MAX_PATH+1];

//
// Includes
//

#include <IsapiModule.h>
#include <IsapiDebug.h>
#include <IsapiBuffer.h>
#include <IsapiString.h>
#include <IsapiStringW.h>
#include <IsapiRequest.h>
#include <IsapiExtension.h>
#include <IsapiTime.h>

#endif  // _isapitools_h

