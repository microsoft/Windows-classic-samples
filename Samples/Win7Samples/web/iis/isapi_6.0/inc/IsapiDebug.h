/*++

Copyright (c) 2003  Microsoft Corporation

Module Name: IsapiDebug.h

Abstract:

    Debug functions for ISAPI tools library

Author:

    ISAPI developer (Microsoft employee), October 2002

--*/

#ifndef _isapidebug_h
#define _isapidebug_h

//
// Definitions
//

#define MAX_DEBUG_OUTPUT    2048

//
// Project globals
//

extern
CHAR    g_szModuleName[MAX_PATH+1];

//
// Prototypes
//

VOID
SetModuleName(
    CHAR *  szModuleName
    );

VOID
IsapiAssert(
    CHAR *  szExpr,
    CHAR *  szFile,
    DWORD   dwLine
    );

VOID
WriteDebug(
    CHAR *  szFormat,
    ...
    );

#endif  // _isapidebug_h
