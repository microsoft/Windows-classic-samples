// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//------------------------------------------------------------------------------
// File: DbgSup.h
//
// Desc: DirectShow sample code - DV control/capture example
//       Debug macros and supporting functions for Windows programs
//------------------------------------------------------------------------------

#ifdef _DEBUG

// dump a string to debug output
#define Dump(tsz)   OutputDebugString(tsz  TEXT("\0"))

// dump a string with a parameter value to debug output
#define BUFSIZE 512

TCHAR dbgsup_tszDump[BUFSIZE]={0};
const size_t NUMCHARS = sizeof(dbgsup_tszDump) / sizeof(dbgsup_tszDump[0]);
const int LASTCHAR = NUMCHARS - 1;


#define Dump1(tsz, arg) \
    { (void)StringCchPrintf(dbgsup_tszDump, NUMELMS(dbgsup_tszDump), (tsz  TEXT("\0")), (arg)); \
      OutputDebugString(dbgsup_tszDump); }


#define CHECK_ERROR(tsz,hr)                     \
{   if( S_OK != hr)                             \
    {                                           \
        (void)StringCchPrintf(dbgsup_tszDump, NUMELMS(dbgsup_tszDump), (tsz  TEXT("\0")), (hr));  \
        OutputDebugString(dbgsup_tszDump);      \
        return hr;                              \
    }                                           \
}
    
#ifndef DBGSUPAPI
#define DBGSUPAPI __declspec(dllimport)
#endif

// dump a Windows message to debug output
DBGSUPAPI void DumpMsg(
    UINT msg,
    WPARAM wparam,
    LPARAM lparam);


#include <assert.h>

// assert an expression
#define Assert(exp)     assert(exp)


#else  // _DEBUG not defined

// do nothing in retail version
#define Dump(sz)
#define Dump1(sz, arg)
#define DumpMsg(msg, wp, lp)
#define Assert(exp)

#define CHECK_ERROR(tsz,hr)                     \
{   if( S_OK != hr)                             \
        return hr;                              \
}

#endif  // _DEBUG
