/*++

Copyright (c) 2008  Microsoft Corporation

Title:

    Express Writer sample application

Abstract:

    Precompiled header

--*/


//
// General includes
//
#include <wchar.h>
#include <stdio.h>
#include <malloc.h>

//
// ATL defines and includes
//
#undef _ASSERTE
#define _STR_MERGE(A, B) A##B
#define _STR_MAKE_W(A) _STR_MERGE(L, A)
#define _ASSERTE(x) { if (!(x)) { wprintf(L"ASSERT: %s\n", _STR_MAKE_W(#x)); } }
#include <atlbase.h>
// strsafe.h has to be included after atlbase.h
#include <strsafe.h>

//
// VSS includes
//
#include <vsmgmt.h>
#include <vswriter.h>

//
// Project includes
//
#include "helpers.h"


///////////////////////////////////////////////////////////////////////////////
//
// Error handling macros
//
///////////////////////////////////////////////////////////////////////////////
#define CHECK_HR(hr, wszFormat, ...)                            \
if (!SUCCEEDED((hr)))                                           \
{                                                               \
    wprintf(L"ERROR: [0x%08x] ", hr);                           \
    wprintf(wszFormat, __VA_ARGS__);                            \
    wprintf(L"\n");                                             \
    goto _exit;                                                 \
}                                                               \


#define CHECK_CONDITION(cond, wszFormat, ...)                   \
if (!(cond))                                                    \
{                                                               \
    wprintf(L"ERROR: [%s] ", _STR_MAKE_W(#cond));               \
    wprintf(wszFormat, __VA_ARGS__);                            \
    wprintf(L"\n");                                             \
    goto _exit;                                                 \
}                                                               \

