//------------------------------------------------------------------------------
// File: Dbg.h
//
// Desc: DirectShow sample code - Helper file for the PSIParser filter.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <strsafe.h>

// dump a string to debug output
#define Dump(tsz) \
    OutputDebugString(tsz);

#define DumpAndReturnFalse(tsz) \
    {OutputDebugString(tsz);    \
    return false;}              \

// dump a string with a parameter value to debug output
#define Dump1(tsz, arg)                         \
    { TCHAR dbgsup_tszDump[1024];               \
      HRESULT hrT = StringCchPrintf(dbgsup_tszDump, 1024, (tsz), (arg));   \
      OutputDebugString(dbgsup_tszDump); }


#define CHECK_ERROR(tsz,hr)                     \
{   if( !SUCCEEDED(hr)  )                       \
    {                                           \
        OutputDebugString(tsz);                 \
        return hr;                              \
    }                                           \
}

#define RETURN_FALSE_IF_FAILED(tsz,hr)          \
{   if( S_OK != hr)                             \
    {                                           \
        TCHAR dbgsup_tszDump[1024];             \
        HRESULT hrT = StringCchPrintf(dbgsup_tszDump, 1024, (tsz), (hr));  \
        OutputDebugString(dbgsup_tszDump);      \
        return FALSE;                           \
    }                                           \
}

#define CHECK_BADPTR(tsz,ptr)                   \
{                                               \
    if( ptr == 0)                               \
    {                                           \
        OutputDebugString(tsz);                 \
        return E_FAIL;                          \
    }                                           \
}

#define RETURN_FALSE_IF_BADPTR(tsz,ptr)         \
{                                               \
    if( ptr == 0)                               \
    {                                           \
        OutputDebugString(tsz);                 \
        return FALSE;                           \
    }                                           \
}

            
