/*++ BUILD Version: 0001    // Increment this if a change has global effects

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1995 - 2000.  Microsoft Corporation.  All rights reserved.

Header Name:

    appmem.h

Abstract:

    Contains the global definitions for use by the performance DLL and
    the DLL used by the application to allocate and track memory. The
    definitions in this file ARE NOT used by the application programmer

--*/
#ifndef _APPMEM_H_
#define _APPMEM_H_

#define MAX_SIZEOF_INSTANCE_NAME    32

typedef struct _APPMEM_INSTANCE
{
   DWORD   dwOffsetOfNext;     // offset from mem base to next item in list
   DWORD   dwProcessId;        // id of process using this instance
   HANDLE  hProcessHeap;       // handle of process's default heap
   DWORD   dwApplicationBytes; // current count of heap bytes allocated
                               //  by the app mem allocation DLL functions
   DWORD   dwAllocCalls;       // number of memory allocation calls
   DWORD   dwReAllocCalls;     // number of Re-Allocation calls
   DWORD   dwFreeCalls;        // number of memory free calls
   WCHAR   wcszInstanceName[MAX_SIZEOF_INSTANCE_NAME]; // SZ instance name
   DWORD   dwReserved1;        // unused
   DWORD   dwReserved2;        // unused
} APPMEM_INSTANCE, *PAPPMEM_INSTANCE;

#define SHARED_MEMORY_ITEM_COUNT    100
#define SHARED_MEMORY_OBJECT_SIZE   (sizeof(PDWORD) + (sizeof(PAPPMEM_INSTANCE) * 2) + (SHARED_MEMORY_ITEM_COUNT * sizeof(APPMEM_INSTANCE)))
#define SHARED_MEMORY_OBJECT_NAME   (TEXT("APPMEM_PERF_DATA"))

#define SHARED_MEMORY_MUTEX_NAME    (TEXT("APPMEM_PERF_DATA_MUTEX"))
#define SHARED_MEMORY_MUTEX_TIMEOUT ((DWORD)1000L)

typedef struct _APPMEM_DATA_HEADER
{
   DWORD               dwInstanceCount;        // number of entries In Use
   DWORD               dwFirstInUseOffset;     // offset from mem base
   DWORD               dwFirstFreeOffset;      // offset from mem base
} APPMEM_DATA_HEADER, *PAPPMEM_DATA_HEADER;

#define     FIRST_FREE(base)    ((PAPPMEM_INSTANCE)((LPBYTE)(base) + ((PAPPMEM_DATA_HEADER)(base))->dwFirstFreeOffset))
#define     FIRST_INUSE(base)   ((PAPPMEM_INSTANCE)((LPBYTE)(base) + ((PAPPMEM_DATA_HEADER)(base))->dwFirstInUseOffset))
#define     APPMEM_INST(base, offset)   ((PAPPMEM_INSTANCE)((LPBYTE)(base) + (DWORD)(offset)))

LONG
_stdcall
GetSharedMemoryDataHeader (
                          IN  HANDLE              *phAppMemSharedMemory,
                          IN  PAPPMEM_DATA_HEADER *pHeader,
                          IN  BOOL                bReadOnlyAccess
                          );


#endif //_APPMEM_H_
