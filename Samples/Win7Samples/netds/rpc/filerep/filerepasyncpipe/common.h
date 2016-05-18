// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*
   

    File Replication Sample
    Server System Service

    FILE: common.h

    PURPOSE: Provides common file replication service declarations.

*/

#pragma once

#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

#ifdef DEBUG1
#define ASSERT(cond) {if ((cond)==0){_tprintf(TEXT("Assertion \"cond\" failed.\n")); *((int *)0)=0;};}
#else
#define ASSERT(cond) {}
#endif

// Macro for printing out error message and exiting the program if an 
// error occured during an RPC library call.
#define EXIT_IF_FAIL(x, string){ \
        ((x) != RPC_S_OK) ? _tprintf(TEXT("%s failed with error %d\n"), \
        TEXT(string), (x)), exit(EXIT_FAILURE) : 0;}

#define RPC_EEINFO_EXIT_IF_FAIL(x, string){ \
        ((x) != RPC_S_OK) ? PrintProcFailureEEInfo(string, x), \
        exit(EXIT_FAILURE) : 0;}

//
// Pipe Routine Pointer Types
//
typedef RPC_STATUS (__stdcall *WINAPI_MY_PIPE_PUSH)(
                                  CHAR *state,
                                  CHAR *buf,
                                  unsigned long ecount);

typedef RPC_STATUS (__stdcall *WINAPI_MY_PIPE_PULL)(CHAR *state,
						  CHAR *buf,
						  unsigned long num,
						  unsigned long * ecount);

// Size for short message strings.
#define MSG_SIZE (1024)

VOID GetEEInfoText(TCHAR *Msg, UINT BuffSize, UINT *MsgSize);

VOID PrintProcFailureEEInfo(LPTSTR ProcName, DWORD ErrCode);

VOID * AutoHeapAlloc(size_t dwBytes);

VOID AutoHeapFree(VOID * lpMem);

#define CriticalSectionOwned(CriticalSection) (ULongToPtr(GetCurrentThreadId()) == (CriticalSection)->OwningThread)

#ifdef _UNICODE
#define RPC_STR RPC_WSTR
#else
#define RPC_STR RPC_CSTR
#endif

// end common.h











